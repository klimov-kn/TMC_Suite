/// @file cp1251.cpp
/// @brief Реализация кодека CP1251 (таблица 0x80...0xFF) и чтения/записи файлов задания.
///
/// Системные кодировщики не используются: таблица своя, чтобы файл задания был
/// одинаковым на всех системах. Описание функций — в cp1251.h.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#encoding
#include "cp1251.h"

#include <QFile>
#include <QSaveFile>
#include <QStringConverter>

namespace tpl {
namespace {

/// Таблица CP1251 для байтов 0x80…0xFF. Ниже 0x80 кодировка совпадает с ASCII.
///
/// ⚠️ Таблица, а не системный кодировщик — решение 2026-09-06. Раньше здесь звался
/// `WideCharToMultiByte(1251, …)` из `windows.h`, то есть генератор физически не
/// собирался нигде, кроме Windows. Но дело даже не в сборке: задание `.tpl`, сделанное
/// на macOS, обязано быть тем же файлом байт в байт, что и сделанное на Windows, иначе
/// «одно и то же задание» начинает зависеть от того, за какой машиной сидели. Системная
/// таблица такой гарантии не даёт (её содержимое — дело системы), собственная даёт.
///
/// 0x98 в CP1251 не определён: Windows отдаёт его как U+FFFD, так же поступаем и мы.
const char16_t kHighTable[128] = {
    0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021, // 80
    0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F, // 88
    0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, // 90
    0xFFFD, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F, // 98
    0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7, // A0
    0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407, // A8
    0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7, // B0
    0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457, // B8
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, // C0
    0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F, // C8
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, // D0
    0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F, // D8
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, // E0
    0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F, // E8
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, // F0
    0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F, // F8
};

/// Обратный ход: символ -> байт CP1251, или -1, если такого символа в ней нет.
///
/// «Похожий» символ не подставляется НИКОГДА (у Windows это делал флаг
/// WC_NO_BEST_FIT_CHARS): иначе '→' молча уехало бы в '>', а '№' в 'N', и
/// пользователь получил бы в задании не тот текст, который написал.
/// @brief Находит байт CP1251 для символа UTF-16.
/// @param[in] u  кодовая единица UTF-16 (одиночная, не суррогатная пара).
/// @return Байт 0...255 или -1, если символа в CP1251 нет (в том числе U+FFFD).
int cp1251ByteFor(char16_t u)
{
    if (u < 0x80)
        return int(u);
    // Непрерывный кусок А…я — почти весь русский текст попадает сюда.
    if (u >= 0x0410 && u <= 0x044F)
        return 0xC0 + int(u - 0x0410);
    if (u == 0xFFFD) // «символ не разобрался» — своего байта у него нет
        return -1;
    for (int i = 0; i < 128; ++i) {
        if (kHighTable[i] == u)
            return 0x80 + i;
    }
    return -1;
}

/// @name Проверка суррогатных половин UTF-16
///@{
/// @brief Проверяет, является ли кодовая единица старшей (D800...DBFF) или младшей (DC00...DFFF) половиной пары.
/// @param[in] u  кодовая единица UTF-16.
/// @return true — `u` относится к соответствующему диапазону.
bool isHighSurrogate(char16_t u) { return u >= 0xD800 && u <= 0xDBFF; }
bool isLowSurrogate(char16_t u)  { return u >= 0xDC00 && u <= 0xDFFF; }
///@}

} // namespace

QByteArray toCp1251(const QString &s, bool *lossy)
{
    if (lossy)
        *lossy = false;
    if (s.isEmpty())
        return {};

    QByteArray out;
    out.reserve(s.size());
    bool lost = false;

    for (int i = 0; i < s.size(); ++i) {
        const char16_t u = s.at(i).unicode();

        // Символ вне BMP приходит парой суррогатов. Заменяем ПАРУ одним '?',
        // а не каждую половину: иначе один эмодзи давал бы два вопроса.
        if (isHighSurrogate(u) && i + 1 < s.size() && isLowSurrogate(s.at(i + 1).unicode())) {
            out += '?';
            lost = true;
            ++i;
            continue;
        }

        const int b = cp1251ByteFor(u);
        if (b < 0) {
            out += '?';
            lost = true;
        } else {
            out += char(static_cast<unsigned char>(b));
        }
    }

    if (lossy)
        *lossy = lost;

    return out;
}

QString fromCp1251(const QByteArray &bytes)
{
    if (bytes.isEmpty())
        return {};

    QString out;
    out.reserve(bytes.size());
    for (char c : bytes) {
        const unsigned char b = static_cast<unsigned char>(c);
        out += (b < 0x80) ? QChar(char16_t(b)) : QChar(kHighTable[b - 0x80]);
    }
    return out;
}


QString decodeTplBytes(const QByteArray &bytes, bool *wasUtf8)
{
    const auto setFlag = [wasUtf8](bool v) { if (wasUtf8) *wasUtf8 = v; };

    if (bytes.startsWith("\xEF\xBB\xBF")) { // BOM — сомнений нет
        setFlag(true);
        return QString::fromUtf8(bytes.mid(3));
    }

    bool hasHighByte = false;
    for (char c : bytes) {
        if (static_cast<unsigned char>(c) >= 0x80) {
            hasHighByte = true;
            break;
        }
    }
    if (!hasHighByte) { // чистый ASCII — обе кодировки дают одно и то же
        setFlag(false);
        return QString::fromLatin1(bytes);
    }

    // Русский текст в CP1251 почти никогда не проходит проверку UTF-8: последо-
    // вательность вида C2 EE (Волновод) — невалидный UTF-8. Проверкой и пользуемся.
    QStringDecoder utf8(QStringDecoder::Utf8, QStringDecoder::Flag::Stateless);
    const QString asUtf8 = utf8.decode(bytes);
    if (!utf8.hasError()) {
        setFlag(true);
        return asUtf8;
    }

    setFlag(false);
    return fromCp1251(bytes);
}

bool readTplFileText(const QString &path, QString *text, QString *error, bool *wasUtf8)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) { // бинарно: QTextStream решил бы за нас, что это UTF-8
        if (error)
            *error = QStringLiteral("не удалось открыть файл: %1").arg(f.errorString());
        return false;
    }

    QString s = decodeTplBytes(f.readAll(), wasUtf8);
    s.replace(QLatin1String("\r\n"), QLatin1String("\n"));
    s.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    if (text)
        *text = s;
    return true;
}

bool writeCp1251File(const QString &path, const QString &text, QString *error, bool *lossy)
{
    QString normalized = text;
    normalized.replace(QLatin1String("\r\n"), QLatin1String("\n"));
    normalized.replace(QLatin1Char('\n'), QLatin1String("\r\n"));

    const QByteArray bytes = toCp1251(normalized, lossy);
    if (bytes.isEmpty() && !normalized.isEmpty()) {
        if (error)
            *error = QStringLiteral("не удалось перекодировать текст в CP1251");
        return false;
    }

    // QSaveFile — чтобы при сбое записи не осталось обрезанного .tpl:
    // ядро прочитает такой файл и упадёт уже на счёте.
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly)) { // бинарно: QTextStream перекодировал бы в UTF-8
        if (error)
            *error = QStringLiteral("не удалось открыть файл: %1").arg(f.errorString());
        return false;
    }

    if (f.write(bytes) != bytes.size()) {
        if (error)
            *error = QStringLiteral("файл записан не полностью: %1").arg(f.errorString());
        return false;
    }

    if (!f.commit()) {
        if (error)
            *error = QStringLiteral("не удалось сохранить файл: %1").arg(f.errorString());
        return false;
    }

    return true;
}

} // namespace tpl
