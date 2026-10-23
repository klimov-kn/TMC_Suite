// Чтение файлов поля ядер TAMIC (FIELDS -> <имя>.ex, FIELD_DISTRIBUTION -> .AMP/.FAZ).
//
// Формат и разбор перенесены без изменений из Windows-версии FieldView
// (fldviewdoc.cpp MFC: ReadDataParameters, ReadDataArray):
//
//   #TMC_GraphicsOutputFieldFile FormVer2.0 2000 \r\n
//   #TMCGROFLD_nT <int>\r\n          #TMCGROFLD_Time <double>\r\n
//   #TMCGROFLD_Delta <double>\r\n    #TMCGROFLD_Xmin <double>\r\n
//   #TMCGROFLD_Ymin <double>\r\n     #TMCGROFLD_Accuracy <4|8>\r\n
//   #TMCGROFLD_LongUnit <s> <double>\r\n  #TMCGROFLD_TimeUnit <s> <double>\r\n
//   #TMCGROFLD_nX <int>\r\n          #TMCGROFLD_nY <int>
//   затем nY строк: \r\n + nX значений float/double (порядок байт машины, x86/arm64 — LE).
//
// Строки заголовка сверяются по началу (strncmp), числа берутся как sscanf %d / %lg —
// но без зависимости от локали (на Android Qt ставит локаль системы, а sscanf с
// русской локалью ждёт запятую).
//
// «Серия тактов» <имя>.ex.frames — НЕ формат ядра: это подряд записанные копии файла
// .ex, каждая байт в байт как у ядра (FieldRecorder, решение M25). Один файл .ex —
// серия из одного такта.
/// @file fieldfile.h
/// @brief Чтение файлов поля ядер (кадр поля, амплитуда/фаза) и серий тактов `.ex.frames`.
///
/// Часть модуля FieldView (слой `core`, без графики). Разбирает заголовок файла поля,
/// двоичные значения узлов (4 или 8 байт на значение) и размах значений; серия тактов —
/// подряд записанные кадры того же формата, читаемые с диска по требованию.
/// @see https://www.tamic.ru/docs/code/fieldview.html#field-file
#pragma once

#include <QString>
#include <QVector>

#include <cstdint>
#include <memory>
#include <vector>

namespace fieldview {

/// @brief Заголовок кадра поля: такт, время, сетка, точность, единицы.
///
/// Параметры одного кадра (такта) — заголовок файла поля.
///
/// Заполняется `parseFieldHeader()`; значения по умолчанию остаются, если число в строке
/// заголовка не разобрано (как у `sscanf`).
/// @see https://www.tamic.ru/docs/code/fieldview.html#field-file
struct FieldHeader
{
    /// Номер такта расчёта (`#TMCGROFLD_nT`).
    int nT = 0;              // номер такта
    /// Модельное время кадра, с (`#TMCGROFLD_Time`).
    double time = 0.0;       // модельное время, с
    /// Шаг сетки Delta, м (`#TMCGROFLD_Delta`); один для X и Y.
    double delta = 0.001;    // шаг сетки, м
    /// Координата X узла (0, 0) сетки в координатах задания, м (`#TMCGROFLD_Xmin`).
    double xMin = 0.0;
    /// Координата Y узла (0, 0) сетки в координатах задания, м (`#TMCGROFLD_Ymin`).
    double yMin = 0.0;
    /// Байт на значение узла (`#TMCGROFLD_Accuracy`): 4 — `float`, 8 — `double`; другие отвергаются.
    int accuracy = 8;        // байт на значение: 4 (float) или 8 (double)
    /// Имя единицы длины для подписей (`#TMCGROFLD_LongUnit`, первое слово), например «m», «mm».
    QString longUnit = QStringLiteral("m");
    /// Величина единицы длины в метрах: подписи показывают `x / longUnitValue`.
    double longUnitValue = 1.0;
    /// Имя единицы времени для подписей (`#TMCGROFLD_TimeUnit`, первое слово).
    QString timeUnit = QStringLiteral("s");
    /// Величина единицы времени в секундах: подписи показывают `t / timeUnitValue`.
    double timeUnitValue = 1.0;
    /// Число узлов сетки по X (`#TMCGROFLD_nX`), не меньше 2.
    int nX = 0;
    /// Число узлов сетки по Y (`#TMCGROFLD_nY`), не меньше 2.
    int nY = 0;
};

/// @brief Размах значений кадра: минимум, максимум, максимум модуля.
///
/// Значения кадра и их размах — как в CFldViewView::PrepareData.
///
/// Считается только по конечным числам; `nan`/`inf` подсчитываются отдельно.
struct FieldStats
{
    /// Наименьшее конечное значение кадра (единицы файла поля).
    double min = 0.0;        // минимум значения
    /// Наибольшее конечное значение кадра.
    double max = 0.0;        // максимум значения
    /// Наибольший модуль конечного значения кадра.
    double maxAbs = 0.0;     // максимум модуля
    /// Сколько значений nan/inf пропущено при подсчёте размаха (С-56, С-57): размах — только
    /// по конечным числам, иначе один nan/inf из кадра делает шкалу и рельеф пустыми.
    qint64 nonFinite = 0;
};

/// @brief Кадр поля в памяти: заголовок, значения узлов во `float` и размах.
///
/// Кадр в памяти: значения узлов (nX*nY, строка за строкой, как pdSurface) во float
/// для видеокарты; размах считан по исходным числам (double).
///
/// Узел (j, i) — `values[i * nX + j]`, как в файле ядра.
struct FieldFrame
{
    /// Заголовок кадра.
    FieldHeader header;
    /// Значения узлов, `nX * nY` штук, строка за строкой (для буфера видеокарты).
    std::vector<float> values;
    /// Размах значений кадра.
    FieldStats stats;
};

/// @brief Разбирает текстовый заголовок кадра поля из буфера.
///
/// Разбор заголовка из буфера. Возвращает смещение начала данных или -1 и текст ошибки
/// (тексты ошибок — как в оригинале: "bad data file" …).
///
/// Строки заголовка сверяются по началу в строгом порядке; затем проверяются
/// `nX >= 2`, `nY >= 2`, `accuracy` = 4 или 8.
/// @param[in]  data   начало кадра в памяти
/// @param[in]  size   число доступных байт от `data`
/// @param[out] out    заголовок; не меняется при ошибке; может быть `nullptr`
/// @param[out] error  текст ошибки («bad data file», «bad data file nX = …» и т. п.); может быть `nullptr`
/// @return Смещение первого байта значений от `data` (байт) или -1 при ошибке.
qint64 parseFieldHeader(const char *data, qint64 size, FieldHeader *out, QString *error);

/// @brief Возвращает размер блока значений кадра после заголовка.
///
/// Размер данных кадра после заголовка: nY строк по nX значений, между строками `\r\n`.
/// @param[in] h  заголовок кадра
/// @return `nY * nX * accuracy + (nY - 1) * 2` байт: между строками значений — пара байт CR LF.
qint64 fieldDataBytes(const FieldHeader &h);

/// @brief Декодирует значения узлов кадра и считает их размах.
///
/// Разбор значений (ReadDataArray) и размаха. data указывает на начало данных.
/// exact (необязательно) — те же значения в исходной точности (для показа числа в точке).
///
/// Проверяет, что между строками значений стоят байты CR LF (иначе файл текстовый или
/// повреждён), переводит значения во `float`, размах считает по исходным числам,
/// пропуская `nan`/`inf`.
/// @param[in]  data   первый байт значений (после заголовка)
/// @param[in]  size   число доступных байт от `data`
/// @param[in]  h      заголовок кадра (`nX`, `nY`, `accuracy`)
/// @param[out] out    кадр: заголовок, значения, размах
/// @param[out] error  текст ошибки; может быть `nullptr`
/// @param[out] exact  значения в исходной точности (`double`), `nX * nY`; `nullptr` — не нужны
/// @return `true` — кадр декодирован; `false` — данных меньше `fieldDataBytes(h)` или нет CR LF.
bool decodeFieldValues(const char *data, qint64 size, const FieldHeader &h, FieldFrame *out,
                       QString *error, std::vector<double> *exact = nullptr);

/// @brief Разбирает кадр целиком (заголовок и значения) из буфера.
///
/// Разобрать кадр целиком из буфера (заголовок + данные). Возвращает false при ошибке.
/// @param[in]  bytes  содержимое файла поля (один кадр)
/// @param[out] out    кадр
/// @param[out] error  текст ошибки; может быть `nullptr`
/// @param[out] exact  значения в исходной точности; `nullptr` — не нужны
/// @return `true` — успех.
bool parseFieldBuffer(const QByteArray &bytes, FieldFrame *out, QString *error,
                      std::vector<double> *exact = nullptr);

/// @brief Читает первый (единственный) кадр файла поля с диска.
///
/// Прочитать один файл поля целиком (как CFldViewDoc::ReadData).
/// @param[in]  path   путь к файлу поля
/// @param[out] out    кадр
/// @param[out] error  текст ошибки; может быть `nullptr`
/// @return `true` — кадр прочитан.
bool readFieldFile(const QString &path, FieldFrame *out, QString *error);

/// @brief Серия тактов: один файл поля или склейка кадров; кадры читаются с диска по требованию.
///
/// Серия тактов: один .ex или склейка .ex.frames. Кадры читаются по требованию.
///
/// `open()` один раз проходит файл и запоминает смещения кадров; значения в памяти не
/// держатся. Объект копируемый (копия используется фоновым подсчётом размаха в
/// `FieldDocument`); `readFrame()` константный и каждый раз открывает файл заново,
/// поэтому копии можно читать из разных потоков.
/// @see https://www.tamic.ru/docs/code/fieldview.html#series
class FieldSeries
{
public:
    /// @brief Положение одного кадра серии в файле.
    struct Entry
    {
        /// Смещение начала заголовка кадра от начала файла, байт.
        qint64 offset = 0;       // начало заголовка кадра в файле
        /// Смещение первого байта значений кадра от начала файла, байт.
        qint64 dataOffset = 0;   // начало значений
        /// Заголовок кадра.
        FieldHeader header;
    };

    /// @brief Открывает файл и строит список кадров серии.
    ///
    /// Кадры идут подряд; после кадра допускается до двух байт CR/LF. Хвост, не похожий на
    /// кадр, и недописанный последний кадр отбрасываются. Кадры с сеткой, отличной от
    /// первого, в серию не попадают.
    /// @param[in]  path   путь к `.ex`, `.tf`, `.AMP`, `.FAZ` или `.ex.frames`
    /// @param[out] error  текст ошибки, если не прочитан ни один кадр; может быть `nullptr`
    /// @return `true` — найден хотя бы один кадр.
    bool open(const QString &path, QString *error);
    /// @brief Забывает файл и список кадров.
    void close();

    /// @brief Возвращает путь к файлу серии (пусто — серия не открыта).
    /// @return Путь к файлу.
    QString path() const { return m_path; }
    /// @brief Возвращает число кадров серии.
    /// @return Число кадров.
    int count() const { return int(m_entries.size()); }
    /// @brief Возвращает положение кадра `i` в файле.
    /// @param[in] i  номер кадра, 0 … `count()` - 1 (не проверяется)
    /// @return Ссылка на описание кадра.
    const Entry &entry(int i) const { return m_entries[size_t(i)]; }
    /// @brief Возвращает число узлов по X первого кадра (0 — серия пуста).
    ///
    /// Сетка одна на всю серию (кадры с другой сеткой отбрасываются при открытии).
    /// @return Число узлов по X.
    int nX() const { return m_entries.empty() ? 0 : m_entries.front().header.nX; }
    /// @brief Возвращает число узлов по Y первого кадра (0 — серия пуста).
    /// @return Число узлов по Y.
    int nY() const { return m_entries.empty() ? 0 : m_entries.front().header.nY; }

    /// @brief Читает кадр серии с диска.
    ///
    /// Прочитать кадр i с диска.
    /// @param[in]  i      номер кадра, 0 … `count()` - 1
    /// @param[out] out    кадр
    /// @param[out] error  текст ошибки («no frame N», «can't open data file …»); может быть `nullptr`
    /// @param[out] exact  значения в исходной точности; `nullptr` — не нужны
    /// @return `true` — кадр прочитан.
    bool readFrame(int i, FieldFrame *out, QString *error, std::vector<double> *exact = nullptr) const;

private:
    /// Путь к файлу серии.
    QString m_path;
    /// Кадры серии в порядке следования в файле.
    std::vector<Entry> m_entries;
};

} // namespace fieldview
