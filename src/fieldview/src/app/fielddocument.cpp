// FieldDocument — описание в fielddocument.h.
/// @file fielddocument.cpp
/// @brief Документ FieldView: чтение кадров, история тактов, кэш серии, топология, протокол синхронизации с ядром.
///
/// Часть модуля FieldView (слой `app`). Описание класса — в `fielddocument.h`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#sync
#include "fielddocument.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPointer>
#include <QSaveFile>
#include <QStringList>
#include <QThread>

#include <algorithm>
#include <cmath>
#include <cstring>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace fieldview {

namespace {

/// @brief Читает файл целиком, запрещая другим запись на время чтения.
///
/// Прочитать файл целиком так, чтобы ядро не переписало его во время чтения.
///
/// На Windows файл открывается без `FILE_SHARE_WRITE`: ядро, начавшее запись нового кадра,
/// повторяет попытку открыть файл, поэтому прочитан всегда законченный кадр. Файлы от 2 ГБ
/// и пустые не читаются.
/// @param[in]  path  путь к файлу
/// @param[out] out   содержимое файла
/// @return `false` — файл не открыт, пуст, не меньше 2 ГБ или прочитан не полностью.
bool readLocked(const QString &path, QByteArray *out)
{
#ifdef Q_OS_WIN
    // Без FILE_SHARE_WRITE: fopen(..., "wb") ядра на это время получает отказ и
    // повторяет попытку через 10 мс (TmcRTHRectNode.cpp, PutField).
    const std::wstring w = QDir::toNativeSeparators(path).toStdWString();
    HANDLE h = CreateFileW(w.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    LARGE_INTEGER size;
    bool ok = GetFileSizeEx(h, &size) != 0 && size.QuadPart > 0 && size.QuadPart < (1LL << 31);
    if (ok) {
        out->resize(qsizetype(size.QuadPart));
        DWORD got = 0;
        ok = ReadFile(h, out->data(), DWORD(size.QuadPart), &got, nullptr) != 0
             && qint64(got) == size.QuadPart;
    }
    CloseHandle(h);
    return ok;
#else
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    *out = f.readAll();
    return !out->isEmpty();
#endif
}

#ifdef Q_OS_WIN
/// @brief Возвращает атрибуты файла Windows.
/// @param[in] path  путь к файлу
/// @return Атрибуты или `INVALID_FILE_ATTRIBUTES`.
DWORD fileAttributes(const QString &path)
{
    return GetFileAttributesW(QDir::toNativeSeparators(path).toStdWString().c_str());
}
/// @brief Задаёт атрибуты файла Windows (ошибка не сообщается).
/// @param[in] path  путь к файлу
/// @param[in] a     атрибуты (`FILE_ATTRIBUTE_NORMAL` или `FILE_ATTRIBUTE_ARCHIVE`)
void setFileAttributes(const QString &path, DWORD a)
{
    SetFileAttributesW(QDir::toNativeSeparators(path).toStdWString().c_str(), a);
}
#endif

/// @brief Возвращает путь к файлу топологии для файла поля.
///
/// SetTplFileName: путь без последнего расширения + ".tt" (у серии — без ".frames").
///
/// Берётся имя без последнего расширения (у серии — без «.frames» и расширения кадра) и
/// расширение `.tt` в любом регистре; если такого файла нет — путь с `.tt`.
/// @param[in] fieldPath  путь к файлу поля или серии
/// @return Путь к `.tt`; пусто, если у имени нет расширения.
QString topologyPathFor(const QString &fieldPath)
{
    QString p = fieldPath;
    if (p.endsWith(QLatin1String(".frames"), Qt::CaseInsensitive))
        p.chop(7);
    const int dot = p.lastIndexOf(QLatin1Char('.'));
    const int slash = std::max(p.lastIndexOf(QLatin1Char('/')), p.lastIndexOf(QLatin1Char('\\')));
    if (dot <= slash)
        return QString();
    const QString base = p.left(dot);
    for (const char *ext : {".tt", ".TT", ".Tt", ".tT"}) {
        const QString t = base + QLatin1String(ext);
        if (QFileInfo::exists(t))
            return t;
    }
    return base + QStringLiteral(".tt");
}

/// @brief Возвращает объём значений кадра в памяти.
/// @param[in] f  кадр
/// @return Байт (4 на узел).
qint64 frameBytes(const FieldFrame &f)
{
    return qint64(f.values.size() * sizeof(float));
}

} // namespace

FieldDocument::FieldDocument(QObject *parent) : QObject(parent)
{
    m_timer.setInterval(50);   // SetTimer(1, 50) оригинала
    connect(&m_timer, &QTimer::timeout, this, &FieldDocument::poll);
}

FieldDocument::~FieldDocument()
{
    stopIndexing();
#ifdef Q_OS_WIN
    // ~CFldViewDoc: SetFileAttributes(GetPathName(), FILE_ATTRIBUTE_NORMAL).
    if (!m_path.isEmpty() && !m_isSeries && m_sync && fileAttributes(m_path) != INVALID_FILE_ATTRIBUTES)
        setFileAttributes(m_path, FILE_ATTRIBUTE_NORMAL);
#endif
}

QString FieldDocument::fileName() const
{
    return QFileInfo(m_path).fileName();
}

void FieldDocument::setError(const QString &e)
{
    if (e == m_error)
        return;
    m_error = e;
    Q_EMIT errorChanged();
}

/// @brief Для файла топологии находит файл поля с тем же именем.
///
/// Открыли топологию .tt (двойной щелчок в проводнике) — берём файл поля с тем же
/// именем: такт .ex/.ey/.hz/.em/.tf, серию тактов, амплитуду/фазу. Нет такого — сам .tt.
/// @param[in] path  путь, выбранный пользователем
/// @return Путь к найденному файлу поля или исходный путь.
static QString fieldPathForTopology(const QString &path)
{
    if (!path.endsWith(QLatin1String(".tt"), Qt::CaseInsensitive))
        return path;
    const QString base = path.left(path.size() - 3);
    for (const char *ext : {".ex", ".ey", ".hz", ".em", ".tf", ".ex.frames", ".ey.frames", ".hz.frames",
                            ".em.frames", ".tf.frames", ".AMP", ".FAZ"}) {
        const QString f = base + QLatin1String(ext);
        if (QFileInfo::exists(f))
            return f;
    }
    return path;
}

bool FieldDocument::open(const QString &path)
{
    stopIndexing();
    m_path = QFileInfo(fieldPathForTopology(path)).absoluteFilePath();
    m_isSeries = m_path.endsWith(QLatin1String(".frames"), Qt::CaseInsensitive);
    m_frames.clear();
    m_cache.clear();
    m_lru.clear();
    m_cacheBytes = 0;
    m_memBytes = 0;
    m_exact.clear();
    m_exactIndex = -1;
    m_liveFrames = 0;
    bool ok = false;
    if (m_isSeries) {
        QString err;
        ok = m_series.open(m_path, &err);
        if (ok) {
            for (int i = 0; i < m_series.count(); ++i) {
                Slot s;
                s.header = m_series.entry(i).header;
                s.seriesIndex = i;
                m_frames.push_back(s);
            }
            setError(QString());
        } else {
            setError(err);
        }
        const QFileInfo fi(m_path);
        m_size = fi.size();
        m_stamp = fi.lastModified();
    } else {
        ok = readSingle(false);
#ifdef Q_OS_WIN
        if (m_sync && fileAttributes(m_path) != INVALID_FILE_ATTRIBUTES)
            setFileAttributes(m_path, FILE_ATTRIBUTE_ARCHIVE);   // SetSynchronizationFlag
#endif
    }
    readTopology(true);
    Q_EMIT framesChanged();
    if (m_sync)
        m_timer.start();
    if (m_isSeries)
        startIndexing();
    return ok;
}

void FieldDocument::startIndexing()
{
    // Фоновый проход по всем тактам серии: размах для шкалы «по всем тактам» сразу.
    const int n = m_series.count();
    if (n <= 1)
        return;
    auto cancel = std::make_shared<std::atomic<bool>>(false);
    m_cancel = cancel;
    // Поток работает с копией оглавления и общим флагом отмены; результат возвращается в
    // поток интерфейса через очередь событий и только если документ ещё жив.
    const FieldSeries series = m_series;
    QPointer<FieldDocument> self(this);
    m_indexer = QThread::create([series, cancel, self, n]() {
        std::vector<FieldStats> stats(static_cast<size_t>(n));
        FieldFrame f;
        QString err;
        for (int i = 0; i < n && !cancel->load(); ++i)
            if (series.readFrame(i, &f, &err))
                stats[size_t(i)] = f.stats;
        if (cancel->load())
            return;
        QMetaObject::invokeMethod(self, [self, stats, cancel]() {
            if (!self || cancel->load())
                return;
            for (size_t i = 0; i < stats.size() && i < self->m_frames.size(); ++i) {
                self->m_frames[i].stats = stats[i];
                self->m_frames[i].statsKnown = true;
            }
            Q_EMIT self->statsChanged();
        }, Qt::QueuedConnection);
    });
    m_indexer->start(QThread::LowPriority);
}

void FieldDocument::stopIndexing()
{
    if (m_cancel)
        m_cancel->store(true);
    if (m_indexer) {
        m_indexer->wait();
        delete m_indexer;
        m_indexer = nullptr;
    }
    m_cancel.reset();
}

bool FieldDocument::readSingle(bool live)
{
    QByteArray bytes;
    if (!readLocked(m_path, &bytes)) {
        const QFileInfo fi(m_path);
        if (!fi.exists())
            setError(QStringLiteral("can't open data file {%1}").arg(QDir::toNativeSeparators(m_path)));
        else if (fi.size() >= (1LL << 31))   // readLocked читает файл целиком, не больше 2 ГБ (С-62)
            setError(QStringLiteral("when read data file: file is too large (%1 MB, limit 2047 MB)")
                         .arg(fi.size() >> 20));
        else
            setError(QStringLiteral("when read data file"));
        return false;
    }
    const QFileInfo fi(m_path);
    m_size = fi.size();
    m_stamp = fi.lastModified();
    auto f = std::make_shared<FieldFrame>();
    std::vector<double> exact;
    QString err;
    if (!parseFieldBuffer(bytes, f.get(), &err, &exact)) {
        setError(err);
        return false;
    }
    setError(QString());
    Slot s;
    s.header = f->header;
    s.stats = f->stats;
    s.statsKnown = true;
    s.bytes = frameBytes(*f);
    s.mem = f;

    // Решение: дописать такт в историю или заменить последний.
    bool append = false;
    if (live && m_record && !m_frames.empty()) {
        const FieldHeader &last = m_frames.back().header;
        const bool sameGrid = last.nX == s.header.nX && last.nY == s.header.nY;
        const bool newTact = last.nT != s.header.nT || last.time != s.header.time;
        if (!sameGrid) {
            m_frames.clear();
            m_memBytes = 0;
        } else {
            append = newTact;
        }
    }
    if (!append) {
        if (!m_frames.empty()) {
            m_memBytes -= m_frames.back().bytes;
            m_frames.pop_back();
        }
        if (!m_record || !live)
        {
            // Без истории — единственный такт (как Windows-версия).
            m_frames.clear();
            m_memBytes = 0;
        }
    }
    m_frames.push_back(s);
    m_memBytes += s.bytes;
    m_exact = std::move(exact);
    m_exactIndex = int(m_frames.size()) - 1;
    trimHistory();
    return true;
}

void FieldDocument::trimHistory()
{
    while (m_memBytes > m_budget && m_frames.size() > 1) {
        m_memBytes -= m_frames.front().bytes;
        m_frames.erase(m_frames.begin());
        if (m_exactIndex >= 0)
            --m_exactIndex;
    }
}

void FieldDocument::readTopology(bool force)
{
    const QString tt = topologyPathFor(m_path);
    m_topologyPath = tt;
    const QFileInfo fi(tt);
    if (!fi.exists()) {
        const bool had = m_hasTopology;
        m_hasTopology = false;
        m_topology = TopologyFile();
        m_bounds.clear();
        m_topologyError = QStringLiteral("нет файла топологии %1").arg(fi.fileName());
        if (had || force)
            Q_EMIT topologyChanged();
        return;
    }
    const double delta = m_frames.empty() ? -1.0 : m_frames.front().header.delta;
    if (!force && fi.lastModified() == m_topologyStamp && delta == m_boundsDelta)
        return;
    m_topologyStamp = fi.lastModified();
    TopologyFile t;
    QString err;
    if (!t.read(tt, &err)) {
        m_hasTopology = false;
        m_topology = TopologyFile();
        m_bounds.clear();
        m_topologyError = err;
    } else {
        m_topology = std::move(t);
        m_hasTopology = true;
        m_topologyError.clear();
        // Границы — с шагом сетки файла поля (ReadTpl: dDelta = pDoc->GetdDelta()).
        const double d = delta > 0 ? delta : m_topology.delta();
        m_bounds = buildBoundaries(m_topology.blocks(), d, true);
        m_boundsDelta = delta;
    }
    Q_EMIT topologyChanged();
}

QString FieldDocument::topologyMismatch() const
{
    // А-34: сверка сетки .tt с сеткой файла поля. Оба заголовка пишет ядро одним форматом
    // (%lg), допуски — только на округление записи.
    if (!m_hasTopology || m_frames.empty())
        return QString();
    const FieldHeader &h = m_frames.front().header;
    const TopologyFile &t = m_topology;
    QStringList diff;
    if (t.nX() != h.nX || t.nY() != h.nY)
        diff << QStringLiteral("сетка %1×%2 вместо %3×%4").arg(t.nX()).arg(t.nY()).arg(h.nX).arg(h.nY);
    const double d = std::max(std::fabs(h.delta), std::fabs(t.delta()));
    if (!(std::fabs(t.delta() - h.delta) <= 1e-6 * d))
        diff << QStringLiteral("Delta %1 вместо %2").arg(t.delta(), 0, 'g', 6).arg(h.delta, 0, 'g', 6);
    if (!(std::fabs(t.xMin() - h.xMin) <= 1e-6 * d) || !(std::fabs(t.yMin() - h.yMin) <= 1e-6 * d))
        diff << QStringLiteral("начало сетки (%1; %2) вместо (%3; %4)")
                    .arg(t.xMin(), 0, 'g', 6).arg(t.yMin(), 0, 'g', 6).arg(h.xMin, 0, 'g', 6).arg(h.yMin, 0, 'g', 6);
    if (diff.isEmpty())
        return QString();
    return QStringLiteral("файл топологии %1 от другого расчёта: %2 — не используется")
        .arg(QFileInfo(m_topologyPath).fileName(), diff.join(QStringLiteral(", ")));
}

std::shared_ptr<const FieldFrame> FieldDocument::frame(int i)
{
    if (i < 0 || i >= int(m_frames.size()))
        return nullptr;
    Slot &s = m_frames[size_t(i)];
    if (s.mem)
        return s.mem;
    const auto it = m_cache.find(i);
    if (it != m_cache.end()) {
        m_lru.remove(i);
        m_lru.push_front(i);
        return it->second;
    }
    auto f = std::make_shared<FieldFrame>();
    QString err;
    if (!m_series.readFrame(s.seriesIndex, f.get(), &err)) {
        setError(err);
        return nullptr;
    }
    s.stats = f->stats;
    s.statsKnown = true;
    if (!m_error.isEmpty())
        setError(QString());   // С-59: такт прочитан — прежняя ошибка чтения снята
    m_cache.emplace(i, f);
    m_lru.push_front(i);
    m_cacheBytes += frameBytes(*f);
    // Кэш сверх предела — вытесняются такты, которые дольше всего не запрашивались.
    while (m_cacheBytes > m_cacheBudget && m_lru.size() > 1) {
        const int old = m_lru.back();
        m_lru.pop_back();
        const auto o = m_cache.find(old);
        if (o != m_cache.end()) {
            m_cacheBytes -= frameBytes(*o->second);
            m_cache.erase(o);
        }
    }
    return f;
}

const std::vector<double> *FieldDocument::exactValues(int i) const
{
    return (i == m_exactIndex && !m_exact.empty()) ? &m_exact : nullptr;
}


bool FieldDocument::totalStats(FieldStats *out) const
{
    bool any = false;
    for (const Slot &s : m_frames) {
        if (!s.statsKnown)
            continue;
        if (!any) {
            *out = s.stats;
            any = true;
        } else {
            out->min = std::min(out->min, s.stats.min);
            out->max = std::max(out->max, s.stats.max);
            out->maxAbs = std::max(out->maxAbs, s.stats.maxAbs);
        }
    }
    return any;
}

void FieldDocument::setSyncEnabled(bool on)
{
    if (on == m_sync)
        return;
    m_sync = on;
    if (on)
        m_timer.start();
    else
        m_timer.stop();
    Q_EMIT syncChanged();
}

void FieldDocument::setRecordHistory(bool on)
{
    if (on == m_record)
        return;
    m_record = on;
    Q_EMIT syncChanged();
}

void FieldDocument::reload()
{
    if (m_path.isEmpty())
        return;
    if (m_isSeries) {
        open(m_path);
        return;
    }
    // Тот же такт заменяется, новый дописывается в историю (история не теряется).
    readSingle(true);
    readTopology(true);
    Q_EMIT framesChanged();
}

void FieldDocument::clearHistory()
{
    if (m_isSeries || m_frames.size() <= 1)
        return;
    Slot last = m_frames.back();
    m_frames.clear();
    m_frames.push_back(last);
    m_memBytes = last.bytes;
    m_exactIndex = m_exact.empty() ? -1 : 0;
    Q_EMIT framesChanged();
}

void FieldDocument::poll()
{
    if (m_path.isEmpty() || !m_sync)
        return;
    if (m_errorBackoff > 0) {   // после ошибки — пауза ~1 с (Sleep(1000) в OnUpdate)
        --m_errorBackoff;
        return;
    }
    if (m_isSeries) {
        // Серию может дописывать другая программа — подхватываем новые кадры.
        const QFileInfo fi(m_path);
        if (fi.exists() && (fi.size() != m_size || fi.lastModified() != m_stamp)) {
            m_size = fi.size();
            m_stamp = fi.lastModified();
            FieldSeries s;
            QString err;
            if (s.open(m_path, &err) && s.count() > m_series.count()) {
                const int from = m_series.count();
                m_series = s;
                for (int i = from; i < s.count(); ++i) {
                    Slot sl;
                    sl.header = s.entry(i).header;
                    sl.seriesIndex = i;
                    m_frames.push_back(sl);
                }
                Q_EMIT framesChanged();
                Q_EMIT frameArrived(frameCount() - 1);
            }
        }
        return;
    }
    bool changed = false;
#ifdef Q_OS_WIN
    const DWORD a = fileAttributes(m_path);
    if (a == INVALID_FILE_ATTRIBUTES)
        return;
    // CFldViewDoc::IsFileRead сравнивал атрибуты целиком (== ARCHIVE): у сжатого файла, у
    // файла OneDrive (PINNED/UNPINNED) и т. п. лишние биты не снимаются, и кадр перечитывался
    // каждые 50 мс (С-59). Новый кадр — это снятый бит ARCHIVE (ядро ставит NORMAL).
    changed = (a & FILE_ATTRIBUTE_ARCHIVE) == 0;
#else
    const QFileInfo fi(m_path);
    if (!fi.exists())
        return;
    changed = fi.lastModified() != m_stamp || fi.size() != m_size;
#endif
    if (!changed)
        return;
    const bool ok = readSingle(true);
#ifdef Q_OS_WIN
    setFileAttributes(m_path, FILE_ATTRIBUTE_ARCHIVE);   // SetSynchronizationFlag
#endif
    if (!ok) {
        m_errorBackoff = 20;
        Q_EMIT framesChanged();
        return;
    }
    ++m_liveFrames;
    m_lastLive = QDateTime::currentDateTime();
    readTopology(false);
    Q_EMIT framesChanged();
    Q_EMIT frameArrived(frameCount() - 1);
}

bool FieldDocument::saveSeries(const QString &path, QString *error)
{
    QSaveFile out(path);
    if (!out.open(QIODevice::WriteOnly)) {
        if (error)
            *error = out.errorString();
        return false;
    }
    for (int i = 0; i < frameCount(); ++i) {
        const auto f = frame(i);
        if (!f)
            continue;
        const FieldHeader &h = f->header;
        // Заголовок — как пишет ядро (PutField), значения — одинарная точность.
        QByteArray b;
        b += "#TMC_GraphicsOutputFieldFile FormVer2.0 2000 \r\n";
        b += "#TMCGROFLD_nT " + QByteArray::number(h.nT) + "\r\n";
        b += "#TMCGROFLD_Time " + QByteArray::number(h.time, 'g', 6) + "\r\n";
        b += "#TMCGROFLD_Delta " + QByteArray::number(h.delta, 'g', 6) + "\r\n";
        b += "#TMCGROFLD_Xmin " + QByteArray::number(h.xMin, 'g', 6) + "\r\n";
        b += "#TMCGROFLD_Ymin " + QByteArray::number(h.yMin, 'g', 6) + "\r\n";
        b += "#TMCGROFLD_Accuracy 4\r\n";
        b += "#TMCGROFLD_LongUnit " + h.longUnit.toLatin1() + ' ' + QByteArray::number(h.longUnitValue, 'g', 6) + "\r\n";
        b += "#TMCGROFLD_TimeUnit " + h.timeUnit.toLatin1() + ' ' + QByteArray::number(h.timeUnitValue, 'g', 6) + "\r\n";
        b += "#TMCGROFLD_nX " + QByteArray::number(h.nX) + "\r\n";
        b += "#TMCGROFLD_nY " + QByteArray::number(h.nY);
        for (int y = 0; y < h.nY; ++y) {
            b += "\r\n";
            b.append(reinterpret_cast<const char *>(f->values.data() + size_t(y) * size_t(h.nX)),
                     qsizetype(h.nX) * 4);
        }
        if (out.write(b) != b.size()) {
            if (error)
                *error = out.errorString();
            out.cancelWriting();
            return false;
        }
    }
    if (!out.commit()) {
        if (error)
            *error = out.errorString();
        return false;
    }
    return true;
}

} // namespace fieldview
