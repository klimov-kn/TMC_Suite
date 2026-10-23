// Чтение файлов поля — перенос CFldViewDoc::ReadDataParameters / ReadDataArray
// (fldviewdoc.cpp MFC) и цикла размаха из CFldViewView::PrepareData.
// Подробности формата — fieldfile.h.
/// @file fieldfile.cpp
/// @brief Разбор заголовка и значений файла поля, оглавление и чтение серии тактов.
///
/// Часть модуля FieldView (слой `core`). Описание функций — в `fieldfile.h`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#field-file
#include "fieldfile.h"
#include "textscan.h"

#include <QFile>

#include <cmath>
#include <cstring>

namespace fieldview {

namespace {

// Строки-признаки из src/Include/tmcgrviw.h (TMC_GROFLD_DOCFILE_ID…).
/// @name Строки-признаки заголовка файла поля
///@{
/// Начала строк заголовка в порядке записи ядром; строка сверяется по началу.
constexpr const char kId[]        = "#TMC_GraphicsOutputFieldFile FormVer2.0 2000 ";
constexpr const char kIdNT[]      = "#TMCGROFLD_nT ";
constexpr const char kIdT[]       = "#TMCGROFLD_Time ";
constexpr const char kIdDelta[]   = "#TMCGROFLD_Delta ";
constexpr const char kIdXmin[]    = "#TMCGROFLD_Xmin ";
constexpr const char kIdYmin[]    = "#TMCGROFLD_Ymin ";
constexpr const char kIdAcc[]     = "#TMCGROFLD_Accuracy ";
constexpr const char kIdLong[]    = "#TMCGROFLD_LongUnit ";
constexpr const char kIdTime[]    = "#TMCGROFLD_TimeUnit ";
constexpr const char kIdNX[]      = "#TMCGROFLD_nX ";
constexpr const char kIdNY[]      = "#TMCGROFLD_nY ";
///@}

/// @brief Проверяет, начинается ли буфер с признака файла поля.
///
/// Начало файла поля — признак первой строки.
/// @param[in] data  начало буфера
/// @param[in] size  длина буфера, байт
/// @return `true` — первые байты совпадают с `kId`.
bool startsWithId(const char *data, qint64 size)
{
    const qint64 n = qint64(sizeof(kId) - 1);
    return size >= n && std::memcmp(data, kId, size_t(n)) == 0;
}

} // namespace

qint64 parseFieldHeader(const char *data, qint64 size, FieldHeader *out, QString *error)
{
    LineReader r(data, size);
    // Значения по умолчанию остаются, если число в строке не разобрано (как у sscanf).
    FieldHeader h;
    const auto bad = [error]() {
        if (error)
            *error = QStringLiteral("bad data file");
        return qint64(-1);
    };
    QByteArray line;
    // Порядок строк и проверки — как в ReadDataParameters.
    if (!r.next(&line) || !line.startsWith(kId))
        return bad();
    if (!r.next(&line) || !line.startsWith(kIdNT))
        return bad();
    h.nT = scanInt(line, sizeof(kIdNT) - 1, h.nT);
    if (!r.next(&line) || !line.startsWith(kIdT))
        return bad();
    h.time = scanDouble(line, sizeof(kIdT) - 1, h.time);
    if (!r.next(&line) || !line.startsWith(kIdDelta))
        return bad();
    h.delta = scanDouble(line, sizeof(kIdDelta) - 1, h.delta);
    if (!r.next(&line) || !line.startsWith(kIdXmin))
        return bad();
    h.xMin = scanDouble(line, sizeof(kIdXmin) - 1, h.xMin);
    if (!r.next(&line) || !line.startsWith(kIdYmin))
        return bad();
    h.yMin = scanDouble(line, sizeof(kIdYmin) - 1, h.yMin);
    if (!r.next(&line) || !line.startsWith(kIdAcc))
        return bad();
    h.accuracy = scanInt(line, sizeof(kIdAcc) - 1, h.accuracy);
    if (!r.next(&line) || !line.startsWith(kIdLong))
        return bad();
    scanWordDouble(line, sizeof(kIdLong) - 1, &h.longUnit, &h.longUnitValue);
    if (!r.next(&line) || !line.startsWith(kIdTime))
        return bad();
    scanWordDouble(line, sizeof(kIdTime) - 1, &h.timeUnit, &h.timeUnitValue);
    if (!r.next(&line) || !line.startsWith(kIdNX))
        return bad();
    h.nX = scanInt(line, sizeof(kIdNX) - 1, h.nX);
    if (!r.next(&line) || !line.startsWith(kIdNY))
        return bad();
    h.nY = scanInt(line, sizeof(kIdNY) - 1, h.nY);

    // InitAllocationArray: те же проверки и тексты.
    if (h.nX < 2) {
        if (error)
            *error = QStringLiteral("bad data file nX = %1 < 2").arg(h.nX);
        return -1;
    }
    if (h.nY < 2) {
        if (error)
            *error = QStringLiteral("bad data file nY = %1 < 2").arg(h.nY);
        return -1;
    }
    if (h.accuracy != 4 && h.accuracy != 8) {
        if (error)
            *error = QStringLiteral("abnormal accuracy [%1], but must be 4 or 8").arg(h.accuracy);
        return -1;
    }
    if (out)
        *out = h;
    return r.position();
}

qint64 fieldDataBytes(const FieldHeader &h)
{
    // Строка nY заголовка заканчивается \r\n первой строки данных; дальше nY строк по
    // nX значений, между ними \r\n (ядро пишет \r\n ПЕРЕД каждой строкой).
    return qint64(h.nY) * qint64(h.nX) * qint64(h.accuracy) + qint64(h.nY - 1) * 2;
}

bool decodeFieldValues(const char *data, qint64 size, const FieldHeader &h, FieldFrame *out,
                       QString *error, std::vector<double> *exact)
{
    if (size < fieldDataBytes(h)) {
        if (error)
            *error = QStringLiteral("when read data file");
        return false;
    }
    // С-62: ядро пишет \r\n перед каждой строкой значений. Если между строками их нет —
    // файл не двоичный (текстовый вывод поля) или повреждён: не читать его как числа.
    for (int i = 1; i < h.nY; ++i) {
        const char *sep = data + qint64(i) * qint64(h.nX) * qint64(h.accuracy) + qint64(i - 1) * 2;
        if (sep[0] != '\r' || sep[1] != '\n') {
            if (error)
                *error = QStringLiteral("bad data file: no CR LF after data row %1 of %2 (text format or damaged file)")
                             .arg(i).arg(h.nY);
            return false;
        }
    }
    const size_t n = size_t(h.nX) * size_t(h.nY);   // size_t, а не int (С-62)
    out->header = h;
    out->values.resize(n);
    if (exact)
        exact->resize(n);
    double vmin = 0.0, vmax = 0.0, amax = 0.0;
    bool first = true;
    qint64 nonFinite = 0;
    const char *p = data;
    size_t ii = 0;
    for (int i = 0; i < h.nY; ++i) {
        for (int j = 0; j < h.nX; ++j) {
            double v;
            if (h.accuracy == 4) {
                float f;
                std::memcpy(&f, p, 4);
                v = f;
                p += 4;
            } else {
                std::memcpy(&v, p, 8);
                p += 8;
            }
            if (exact)
                (*exact)[ii] = v;
            out->values[ii++] = float(v);
            // Размах — по исходным числам, как в PrepareData (там же fabs для «модуля»);
            // nan/inf в размах не входят (С-56, С-57), значение в кадре остаётся как есть.
            if (!std::isfinite(v)) {
                ++nonFinite;
            } else if (first) {
                vmin = vmax = v;
                amax = std::fabs(v);
                first = false;
            } else {
                if (vmax < v)
                    vmax = v;
                if (vmin > v)
                    vmin = v;
                if (amax < std::fabs(v))
                    amax = std::fabs(v);
            }
        }
        p += 2;   // fread(&ch,1,1) дважды: \r\n перед следующей строкой
    }
    out->stats.min = vmin;
    out->stats.max = vmax;
    out->stats.maxAbs = amax;
    out->stats.nonFinite = nonFinite;
    return true;
}

bool parseFieldBuffer(const QByteArray &bytes, FieldFrame *out, QString *error,
                      std::vector<double> *exact)
{
    FieldHeader h;
    const qint64 start = parseFieldHeader(bytes.constData(), bytes.size(), &h, error);
    if (start < 0)
        return false;
    return decodeFieldValues(bytes.constData() + start, bytes.size() - start, h, out, error, exact);
}

bool readFieldFile(const QString &path, FieldFrame *out, QString *error)
{
    FieldSeries s;
    if (!s.open(path, error))
        return false;
    // Один файл .ex: берётся первый (единственный) кадр.
    return s.readFrame(0, out, error);
}

bool FieldSeries::open(const QString &path, QString *error)
{
    close();
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error)
            *error = QStringLiteral("can't open data file {%1}").arg(path);
        return false;
    }
    // Проход по файлу: заголовок кадра читается окном 4 КБ, значения пропускаются по
    // размеру из заголовка — сами значения здесь не читаются.
    const qint64 total = f.size();
    qint64 pos = 0;
    // Заголовок не длиннее нескольких сотен байт; читаем окно с запасом.
    constexpr qint64 kHeaderWindow = 4096;
    while (pos < total) {
        if (!f.seek(pos))
            break;
        const QByteArray head = f.read(qMin(kHeaderWindow, total - pos));
        if (!startsWithId(head.constData(), head.size())) {
            if (m_entries.empty()) {
                if (error)
                    *error = QStringLiteral("bad data file");
                return false;
            }
            break;   // хвост, не похожий на кадр — конец серии
        }
        Entry e;
        QString err;
        const qint64 dataStart = parseFieldHeader(head.constData(), head.size(), &e.header, &err);
        if (dataStart < 0) {
            if (m_entries.empty()) {
                if (error)
                    *error = err;
                return false;
            }
            break;
        }
        e.offset = pos;
        e.dataOffset = pos + dataStart;
        const qint64 end = e.dataOffset + fieldDataBytes(e.header);
        if (end > total) {
            // Последний кадр недописан (запись ещё шла) — без него.
            if (m_entries.empty()) {
                if (error)
                    *error = QStringLiteral("when read data file");
                return false;
            }
            break;
        }
        // Кадр с другой сеткой пропускается, но разбор серии продолжается за ним.
        if (m_entries.empty() || (e.header.nX == nX() && e.header.nY == nY()))
            m_entries.push_back(e);
        pos = end;
        // Между кадрами серии ничего нет; одиночный .ex может кончаться \r\n — допускаем.
        while (pos < total && pos < end + 2) {
            char c = 0;
            f.seek(pos);
            if (f.read(&c, 1) != 1 || (c != '\r' && c != '\n'))
                break;
            ++pos;
        }
    }
    m_path = path;
    return true;
}

void FieldSeries::close()
{
    m_path.clear();
    m_entries.clear();
}

bool FieldSeries::readFrame(int i, FieldFrame *out, QString *error, std::vector<double> *exact) const
{
    if (i < 0 || i >= count()) {
        if (error)
            *error = QStringLiteral("no frame %1").arg(i);
        return false;
    }
    const Entry &e = m_entries[size_t(i)];
    QFile f(m_path);
    if (!f.open(QIODevice::ReadOnly) || !f.seek(e.dataOffset)) {
        if (error)
            *error = QStringLiteral("can't open data file {%1}").arg(m_path);
        return false;
    }
    const qint64 n = fieldDataBytes(e.header);
    const QByteArray data = f.read(n);
    return decodeFieldValues(data.constData(), data.size(), e.header, out, error, exact);
}

} // namespace fieldview
