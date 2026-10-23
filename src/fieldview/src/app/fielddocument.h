// FieldDocument — открытый файл поля (аналог CFldViewDoc Windows-версии) и всё, что к
// нему прилагается: такты, топология <имя>.tt, синхронизация с работающим ядром.
//
// Источники тактов:
//   * один файл поля (.ex — FIELDS, .tf — старое имя, .AMP/.FAZ — FIELD_DISTRIBUTION):
//     один такт; пока файл открыт и включено «Следить за расчётом», каждый новый кадр,
//     записанный ядром, дописывается в историю тактов в памяти (её можно проиграть
//     и сохранить серией .ex.frames);
//   * серия .ex.frames (подряд записанные кадры того же формата, решение M26 мобильной
//     версии) — такты читаются с диска по требованию.
//
// Синхронизация с ядром — как у CFldViewDoc/CFldViewView (OnTimer 50 мс → OnUpdate):
//   ядро после записи кадра ставит файлу атрибут FILE_ATTRIBUTE_NORMAL и (при
//   SYNCHRONIZATION) ждёт, пока атрибут не станет FILE_ATTRIBUTE_ARCHIVE
//   (CTmcRTHRectNode::IsFieldFileRead); FieldView, увидев атрибут ≠ ARCHIVE
//   (IsFileRead), читает кадр и ставит ARCHIVE (SetSynchronizationFlag). При закрытии
//   документа атрибут возвращается в NORMAL (~CFldViewDoc). Кадр читается с запретом
//   записи другим (FILE_SHARE_READ): ядро на это время ждёт (fopen "wb" повторяется
//   через 10 мс) — прочитан всегда законченный кадр.
/// @file fielddocument.h
/// @brief Документ FieldView: открытый файл поля, его такты, топология `.tt` и синхронизация с ядром.
///
/// Часть модуля FieldView (слой `app`). Один документ может показываться в нескольких
/// окнах (`FieldPane`), поэтому передаётся через `std::shared_ptr`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#document
#pragma once

#include "fieldfile.h"
#include "topology.h"

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QTimer>

#include <atomic>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

class QThread;

namespace fieldview {

/// @brief Открытый файл поля: такты (в памяти или на диске), топология, слежение за ядром.
///
/// Создаётся `MainWindow::openFile()`, принадлежит окнам документа через `std::shared_ptr`.
/// Таймер 50 мс (`poll()`) следит за файлом: у одиночного файла — по атрибуту ARCHIVE
/// (протокол синхронизации с ядром), у серии — по размеру и времени изменения. Новые
/// кадры от ядра дописываются в историю тактов в памяти (не больше `setHistoryBudget()`
/// байт). Для серии в фоновом потоке считается размах всех тактов. Все методы — только из
/// потока интерфейса.
/// @see https://www.tamic.ru/docs/code/fieldview.html#sync
class FieldDocument : public QObject
{
    Q_OBJECT
public:
    /// @brief Создаёт пустой документ; таймер опроса настроен на 50 мс, но не запущен.
    /// @param[in] parent  родитель Qt (обычно `nullptr`: временем жизни управляет `shared_ptr`)
    explicit FieldDocument(QObject *parent = nullptr);
    /// @brief Останавливает фоновый подсчёт размаха и возвращает файлу поля атрибут NORMAL.
    ///
    /// Атрибут возвращается, только если файл одиночный, существует и слежение включено
    /// (как при закрытии документа FieldView 4.1). Для ядра это означает «кадр не прочитан».
    ~FieldDocument() override;

    /// @brief Открывает файл поля (или серию) и начинает слежение за ним.
    ///
    /// Открыть файл. false — файл не прочитан (текст ошибки — error()); документ всё равно
    /// остаётся открытым и следит за файлом (ядро может записать его позже).
    ///
    /// Если открыт файл `.tt`, берётся файл поля с тем же именем (`.ex`, `.ey`, `.hz`, `.em`,
    /// `.tf`, их серии `.frames`, `.AMP`, `.FAZ`). Сразу читаются кадр (или оглавление серии)
    /// и топология; одиночному файлу ставится атрибут ARCHIVE.
    /// @param[in] path  путь к файлу поля, серии или топологии
    /// @return `true` — прочитан хотя бы один кадр.
    bool open(const QString &path);
    /// @brief Возвращает полный путь к файлу поля документа.
    /// @return Абсолютный путь.
    QString path() const { return m_path; }
    /// @brief Возвращает имя файла поля без папки.
    /// @return Имя файла.
    QString fileName() const;
    /// @brief Сообщает, открыт ли файл серии `.frames`.
    /// @return `true` — серия; `false` — одиночный файл поля.
    bool isSeriesFile() const { return m_isSeries; }

    /// @brief Возвращает текст последней ошибки чтения.
    /// @return Текст ошибки; пусто — последнее чтение удалось.
    QString error() const { return m_error; }
    /// @brief Сообщает, есть ли хотя бы один такт.
    /// @return `true` — тактов больше нуля.
    bool hasData() const { return !m_frames.empty(); }

    /// @brief Возвращает число тактов (в истории или в серии).
    /// @return Число тактов.
    int frameCount() const { return int(m_frames.size()); }
    /// @brief Возвращает заголовок такта без чтения значений.
    /// @param[in] i  номер такта, 0 … `frameCount()` - 1 (не проверяется)
    /// @return Ссылка на заголовок.
    const FieldHeader &header(int i) const { return m_frames[size_t(i)].header; }
    /// @brief Возвращает такт со значениями.
    ///
    /// Кадр i (из памяти или с диска, с кэшем). nullptr — ошибка чтения.
    ///
    /// Такт истории берётся из памяти; такт серии читается с диска и кладётся в кэш (не
    /// больше 256 МБ, вытесняются давно не нужные).
    /// @param[in] i  номер такта
    /// @return Кадр; `nullptr` — номер вне диапазона или ошибка чтения (текст — `error()`).
    std::shared_ptr<const FieldFrame> frame(int i);
    /// @brief Возвращает значения такта в исходной точности файла (`double`).
    ///
    /// Значения в исходной точности для кадра i, если они есть (последний прочитанный кадр).
    /// @param[in] i  номер такта
    /// @return Значения `nX * nY` или `nullptr`, если такт — не последний прочитанный из одиночного файла.
    const std::vector<double> *exactValues(int i) const;
    /// @brief Объединяет размах всех тактов с известным размахом.
    /// @param[out] s  общий минимум, максимум и максимум модуля
    /// @return `false` — размах не известен ни для одного такта.
    bool totalStats(FieldStats *s) const;
    /// @brief Возвращает число узлов по X первого такта.
    /// @return Число узлов; 0 — тактов нет.
    int nX() const { return m_frames.empty() ? 0 : m_frames.front().header.nX; }
    /// @brief Возвращает число узлов по Y первого такта.
    /// @return Число узлов; 0 — тактов нет.
    int nY() const { return m_frames.empty() ? 0 : m_frames.front().header.nY; }

    // Топология.
    /// @brief Сообщает, прочитан ли файл топологии `.tt`.
    /// @return `true` — топология прочитана.
    bool hasTopology() const { return m_hasTopology; }
    /// @brief Проверяет, что топология прочитана и её сетка совпадает с сеткой файла поля.
    ///
    /// .tt от того же расчёта, что и файл поля: совпадают nX, nY, Delta, Xmin, Ymin (А-34).
    /// Иначе .tt не используется: ни слой ε, ни контуры блоков.
    /// @return `true` — слой eps и контуры блоков можно показывать.
    bool topologyMatchesGrid() const { return m_hasTopology && topologyMismatch().isEmpty(); }
    /// @brief Описывает расхождение сетки `.tt` и сетки файла поля.
    ///
    /// Чем сетка .tt отличается от сетки файла поля (пусто — совпадает или нет данных).
    ///
    /// Сравниваются nX, nY, Delta, Xmin, Ymin; допуск по длинам — 1e-6 * Delta (округление записи).
    /// @return Текст для пользователя или пустая строка.
    QString topologyMismatch() const;
    /// @brief Возвращает прочитанную топологию.
    /// @return Ссылка на топологию (пустую, если файла нет).
    const TopologyFile &topology() const { return m_topology; }
    /// @brief Возвращает границы блоков, построенные с шагом сетки файла поля.
    /// @return Ссылка на границы.
    const std::vector<BlockBoundary> &boundaries() const { return m_bounds; }
    /// @brief Возвращает путь к файлу топологии (существующему или ожидаемому).
    /// @return Путь к `.tt`.
    QString topologyPath() const { return m_topologyPath; }
    /// @brief Возвращает причину, по которой топология не прочитана.
    /// @return Текст; пусто — топология прочитана.
    QString topologyError() const { return m_topologyError; }

    // Синхронизация с ядром и история тактов.
    /// @brief Сообщает, включено ли слежение за файлом («Следить за расчётом»).
    /// @return `true` — таймер опроса работает.
    bool syncEnabled() const { return m_sync; }
    /// @brief Включает или выключает слежение за файлом.
    /// @param[in] on  `true` — запустить опрос раз в 50 мс
    void setSyncEnabled(bool on);
    /// @brief Сообщает, запоминаются ли кадры от ядра («Запоминать такты»).
    /// @return `true` — каждый новый такт дописывается в историю.
    bool recordHistory() const { return m_record; }
    /// @brief Включает или выключает запоминание тактов.
    /// @param[in] on  `false` — новый кадр заменяет единственный такт
    void setRecordHistory(bool on);
    /// @brief Задаёт предельный объём истории тактов в памяти.
    ///
    /// Объём истории тактов в памяти, байт (старые такты отбрасываются).
    /// @param[in] bytes  байт (по умолчанию 1 ГБ); сверх него отбрасываются самые старые такты
    void setHistoryBudget(qint64 bytes) { m_budget = bytes; }
    /// @brief Возвращает объём значений тактов истории в памяти.
    /// @return Байт (4 байта на узел такта).
    qint64 historyBytes() const { return m_memBytes; }
    /// @brief Возвращает число кадров, прочитанных при слежении с момента открытия.
    ///
    /// Сколько кадров пришло от ядра с момента открытия.
    /// @return Число кадров.
    int liveFramesRead() const { return m_liveFrames; }
    /// @brief Возвращает время прихода последнего кадра от ядра.
    /// @return Местное время; недействительное — кадров не было.
    QDateTime lastLiveFrame() const { return m_lastLive; }
    /// @brief Перечитывает файл поля и топологию.
    ///
    /// Перечитать файл (двойной щелчок по виду, как OnLButtonDblClk).
    void reload();
    /// @brief Удаляет из истории все такты, кроме последнего (у серии ничего не делает).
    ///
    /// Удалить историю, оставить последний такт.
    void clearHistory();
    /// @brief Записывает все такты файлом серии.
    ///
    /// Сохранить все такты серией .ex.frames (кадры в формате ядра, одинарная точность).
    ///
    /// Каждый кадр пишется заголовком формата ядра и значениями `float` (`Accuracy 4`);
    /// запись атомарная (`QSaveFile`). Такты, которые не удалось прочитать, пропускаются.
    /// @param[in]  path   путь к файлу серии
    /// @param[out] error  текст ошибки; может быть `nullptr`
    /// @return `true` — файл записан.
    bool saveSeries(const QString &path, QString *error);

    /// @brief Проверяет файл один раз и при новом кадре читает его.
    ///
    /// Один опрос файла (зовётся таймером; открыт для проверок).
    ///
    /// Одиночный файл: снят бит ARCHIVE — кадр новый; кадр читается, бит ставится снова.
    /// После ошибки чтения следующие 20 опросов (~1 с) пропускаются. Серия: при изменении
    /// размера или времени файла добавляются новые кадры.
    /// @see https://www.tamic.ru/docs/code/fieldview.html#sync
    void poll();

Q_SIGNALS:
    /// @brief Сигнал: изменились число тактов или их содержимое.
    void framesChanged();        // число тактов или содержимое изменилось
    /// @brief Сигнал: при слежении пришёл новый кадр.
    /// @param index  номер нового такта
    void frameArrived(int index);// новый кадр от ядра
    /// @brief Сигнал: топология прочитана заново, пропала или появилась.
    void topologyChanged();
    /// @brief Сигнал: изменился текст ошибки чтения.
    void errorChanged();
    /// @brief Сигнал: изменились флаги слежения или запоминания тактов.
    void syncChanged();
    /// @brief Сигнал: фоновый подсчёт размаха тактов серии закончен.
    void statsChanged();         // размах тактов серии посчитан в фоне

private:
    /// @brief Такт документа: заголовок, где лежат значения, известный размах.
    struct Slot
    {
        /// Заголовок такта.
        FieldHeader header;
        /// Такт в памяти (история одиночного файла) или `nullptr` (такт серии на диске).
        std::shared_ptr<const FieldFrame> mem;   // такт в памяти (история) или nullptr
        /// Номер кадра в серии; -1 — такт не из серии.
        int seriesIndex = -1;                    // номер кадра в серии .frames
        /// Размах такта (действителен при `statsKnown`).
        FieldStats stats;
        /// Размах такта известен.
        bool statsKnown = false;
        /// Объём значений такта в памяти, байт (0 у такта серии).
        qint64 bytes = 0;
    };

    /// @brief Читает одиночный файл поля и кладёт кадр в список тактов.
    ///
    /// При `live` и включённом запоминании новый такт (другие nT или время) дописывается,
    /// тот же — заменяется; при другой сетке история сбрасывается. Без запоминания остаётся
    /// один такт.
    /// @param[in] live  `true` — кадр пришёл при слежении (или перечитывании)
    /// @return `false` — файл не прочитан (текст — `error()`).
    bool readSingle(bool live);
    /// @brief Запускает фоновый поток подсчёта размаха всех тактов серии (больше одного такта).
    void startIndexing();
    /// @brief Отменяет фоновый подсчёт и ждёт завершения потока.
    void stopIndexing();
    /// @brief Читает файл топологии рядом с файлом поля и строит границы блоков.
    /// @param[in] force  `true` — читать всегда; `false` — только если изменились время файла или шаг сетки
    void readTopology(bool force);
    /// @brief Запоминает текст ошибки и при изменении выдаёт `errorChanged()`.
    /// @param[in] e  текст ошибки; пусто — ошибки нет
    void setError(const QString &e);
    /// @brief Отбрасывает самые старые такты, пока объём истории больше предела (последний остаётся).
    void trimHistory();

    /// Абсолютный путь к файлу поля.
    QString m_path;
    /// Открыт файл серии `.frames`.
    bool m_isSeries = false;
    /// Оглавление серии (у одиночного файла не используется).
    FieldSeries m_series;
    /// Такты документа.
    std::vector<Slot> m_frames;
    /// Значения последнего прочитанного одиночного кадра в исходной точности.
    std::vector<double> m_exact;
    /// Номер такта, к которому относятся `m_exact`; -1 — нет.
    int m_exactIndex = -1;
    /// Текст последней ошибки чтения.
    QString m_error;

    /// Номера тактов серии в кэше, недавно нужные — в начале.
    std::list<int> m_lru;
    /// Кэш прочитанных тактов серии.
    std::unordered_map<int, std::shared_ptr<const FieldFrame>> m_cache;
    /// Объём кэша, байт.
    qint64 m_cacheBytes = 0;
    /// Предел объёма кэша, байт (256 МБ).
    qint64 m_cacheBudget = 256LL * 1024 * 1024;

    /// Прочитанная топология.
    TopologyFile m_topology;
    /// Топология прочитана.
    bool m_hasTopology = false;
    /// Границы блоков.
    std::vector<BlockBoundary> m_bounds;
    /// Путь к файлу топологии.
    QString m_topologyPath;
    /// Причина отсутствия топологии.
    QString m_topologyError;
    /// Время изменения прочитанного `.tt`.
    QDateTime m_topologyStamp;
    /// Шаг сетки, с которым построены границы, м; -1 — не строились.
    double m_boundsDelta = -1.0;

    /// Слежение включено.
    bool m_sync = true;
    /// Запоминание тактов включено.
    bool m_record = true;
    /// Предел объёма истории, байт (1 ГБ).
    qint64 m_budget = 1024LL * 1024 * 1024;
    /// Объём истории, байт.
    qint64 m_memBytes = 0;
    /// Кадров от ядра с момента открытия.
    int m_liveFrames = 0;
    /// Время последнего кадра от ядра.
    QDateTime m_lastLive;
    /// Время изменения файла при последнем чтении (слежение серии и систем без атрибута ARCHIVE).
    QDateTime m_stamp;       // для систем без атрибута ARCHIVE
    /// Размер файла при последнем чтении, байт.
    qint64 m_size = -1;
    /// Сколько опросов ещё пропустить после ошибки чтения.
    int m_errorBackoff = 0;
    /// Таймер опроса файла (50 мс).
    QTimer m_timer;
    /// Поток фонового подсчёта размаха серии (владеет документ).
    QThread *m_indexer = nullptr;
    /// Флаг отмены фонового подсчёта (общий с потоком).
    std::shared_ptr<std::atomic<bool>> m_cancel;
};

} // namespace fieldview
