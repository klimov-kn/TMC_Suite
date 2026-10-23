// Разбор текстовых строк заголовков так же, как в Windows-версии (fgets + sscanf),
// но без зависимости от локали: числа читаются как в локали "C".
/// @file textscan.h
/// @brief Разбор строк текстовых заголовков (поле, топология) как `fgets` + `sscanf`, без зависимости от локали.
///
/// Часть модуля FieldView (слой `core`). Числа читаются по правилам локали «C» (точка —
/// десятичный разделитель) независимо от системной локали.
/// @see https://www.tamic.ru/docs/code/fieldview.html#field-file
#pragma once

#include <QByteArray>
#include <QString>

namespace fieldview {

/// @brief Последовательное чтение строк из буфера в памяти, как `fgets`.
///
/// fgets по буферу: строка до '\n' включительно, не длиннее TMC_GROUT_MAXSTRING_BUF-1.
///
/// Не владеет буфером: `data` должен жить дольше объекта. Строка длиннее 9999 байт
/// режется на части, как у `fgets` с буфером `TMC_GROUT_MAXSTRING_BUF`.
class LineReader
{
public:
    /// @brief Создаёт чтение с начала буфера.
    /// @param[in] data  начало буфера (не копируется)
    /// @param[in] size  длина буфера, байт
    LineReader(const char *data, qint64 size) : m_data(data), m_size(size) {}
    /// @brief Читает очередную строку.
    /// @param[out] line  строка вместе с завершающим `'\n'` (если он есть в пределах длины)
    /// @return `false` — буфер кончился, `line` не изменена.
    bool next(QByteArray *line)
    {
        if (m_pos >= m_size)
            return false;
        constexpr qint64 kMax = 10000 - 1;   // TMC_GROUT_MAXSTRING_BUF - 1
        qint64 end = m_pos;
        while (end < m_size && end - m_pos < kMax) {
            if (m_data[end++] == '\n')
                break;
        }
        *line = QByteArray(m_data + m_pos, int(end - m_pos));
        m_pos = end;
        return true;
    }
    /// @brief Возвращает смещение начала ещё не прочитанной части буфера, байт.
    /// @return Смещение, байт.
    qint64 position() const { return m_pos; }

private:
    /// Начало буфера (чужая память).
    const char *m_data;
    /// Длина буфера, байт.
    qint64 m_size;
    /// Смещение следующего непрочитанного байта.
    qint64 m_pos = 0;
};

/// @brief Проверяет, является ли байт пробельным символом в смысле `isspace` локали «C».
/// @param[in] c  байт
/// @return `true` для пробела, `\t`, `\r`, `\n`, `\v`, `\f`.
inline bool isSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f';
}

/// @brief Читает вещественное число с позиции `from`, как `sscanf("%lg")`.
///
/// Число в начале строки с позиции from, как sscanf("%lg"): при неудаче — fallback
/// (sscanf в этом случае переменную не меняет). *next — позиция после числа.
///
/// Пропускает пробелы, принимает знак, цифры, дробную часть, порядок `e`/`E`, а также
/// `inf` и `nan`.
/// @param[in]  s         строка
/// @param[in]  from      позиция начала разбора, байт
/// @param[in]  fallback  значение при неудаче
/// @param[out] next      позиция после числа; `nullptr` — не нужна
/// @return Прочитанное число или `fallback`.
inline double scanDouble(const QByteArray &s, int from, double fallback, int *next = nullptr)
{
    int i = from;
    while (i < s.size() && isSpace(s[i]))
        ++i;
    const int start = i;
    if (i < s.size() && (s[i] == '+' || s[i] == '-'))
        ++i;
    bool digits = false;
    while (i < s.size() && s[i] >= '0' && s[i] <= '9') {
        ++i;
        digits = true;
    }
    if (i < s.size() && s[i] == '.') {
        ++i;
        while (i < s.size() && s[i] >= '0' && s[i] <= '9') {
            ++i;
            digits = true;
        }
    }
    if (digits && i < s.size() && (s[i] == 'e' || s[i] == 'E')) {
        int k = i + 1;
        if (k < s.size() && (s[k] == '+' || s[k] == '-'))
            ++k;
        if (k < s.size() && s[k] >= '0' && s[k] <= '9') {
            while (k < s.size() && s[k] >= '0' && s[k] <= '9')
                ++k;
            i = k;
        }
    }
    if (next)
        *next = i;
    if (!digits) {
        // inf / nan, как у strtod
        const QByteArray w = s.mid(start, 4).toLower();
        bool ok = false;
        if (w.startsWith("inf") || w.startsWith("-inf") || w.startsWith("nan")) {
            const double v = s.mid(start).trimmed().toDouble(&ok);
            if (ok)
                return v;
        }
        return fallback;
    }
    bool ok = false;
    const double v = s.mid(start, i - start).toDouble(&ok);
    return ok ? v : fallback;
}

/// @brief Читает целое с позиции `from`, как `sscanf("%d")`.
///
/// Целое как sscanf("%d").
/// @param[in] s         строка
/// @param[in] from      позиция начала разбора, байт
/// @param[in] fallback  значение при неудаче
/// @return Прочитанное целое или `fallback`.
inline int scanInt(const QByteArray &s, int from, int fallback)
{
    int i = from;
    while (i < s.size() && isSpace(s[i]))
        ++i;
    const int start = i;
    if (i < s.size() && (s[i] == '+' || s[i] == '-'))
        ++i;
    const int digitsFrom = i;
    while (i < s.size() && s[i] >= '0' && s[i] <= '9')
        ++i;
    if (i == digitsFrom)
        return fallback;
    bool ok = false;
    const int v = s.mid(start, i - start).toInt(&ok);
    return ok ? v : fallback;
}

/// @brief Читает слово и число за ним, как `sscanf("%s %lg")`.
///
/// "%s %lg": слово до пробела и число за ним.
/// @param[in]     s      строка
/// @param[in]     from   позиция начала разбора, байт
/// @param[in,out] word   слово (Latin-1); не меняется, если слова нет
/// @param[in,out] value  число; не меняется, если число не разобрано
inline void scanWordDouble(const QByteArray &s, int from, QString *word, double *value)
{
    int i = from;
    while (i < s.size() && isSpace(s[i]))
        ++i;
    const int start = i;
    while (i < s.size() && !isSpace(s[i]))
        ++i;
    if (i > start)
        *word = QString::fromLatin1(s.mid(start, i - start));
    *value = scanDouble(s, i, *value);
}

} // namespace fieldview
