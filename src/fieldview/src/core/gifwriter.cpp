// GifWriter — описание в gifwriter.h.
/// @file gifwriter.cpp
/// @brief Кодировщик анимированного GIF: палитра кадра, LZW, подблоки.
///
/// Часть модуля FieldView (слой `core`). Описание класса — в `gifwriter.h`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#gif
#include "gifwriter.h"

#include <QHash>

#include <algorithm>
#include <array>
#include <vector>

namespace fieldview {

namespace {

/// @brief Дописывает 16-битное число в порядке «младший байт первым».
/// @param[in,out] b  буфер
/// @param[in]     v  число, 0 … 65535
void put16(QByteArray &b, int v)
{
    b.append(char(v & 0xFF));
    b.append(char((v >> 8) & 0xFF));
}

/// @brief Сводит цвет к 16-битному ключу 5-6-5 (красный, зелёный, синий).
/// @param[in] c  цвет
/// @return Ключ 0 … 65535.
inline int key565(QRgb c)
{
    return ((qRed(c) >> 3) << 11) | ((qGreen(c) >> 2) << 5) | (qBlue(c) >> 3);
}

// Поток бит LZW, упакованный в подблоки по 255 байт.
/// @brief Упаковщик кодов LZW переменной длины в поток байт и подблоки GIF по 255 байт.
class BitPacker
{
public:
    /// @brief Создаёт упаковщик, пишущий в буфер.
    /// @param[in] out  выходной буфер (не принадлежит упаковщику)
    explicit BitPacker(QByteArray *out) : m_out(out) {}
    /// @brief Добавляет код заданной длины (младшими битами вперёд).
    /// @param[in] code  код
    /// @param[in] bits  длина кода, бит (9 … 12)
    void put(int code, int bits)
    {
        m_acc |= quint32(code) << m_n;
        m_n += bits;
        while (m_n >= 8) {
            byte(char(m_acc & 0xFF));
            m_acc >>= 8;
            m_n -= 8;
        }
    }
    /// @brief Дописывает неполный байт, последний подблок и пустой подблок-терминатор.
    void finish()
    {
        if (m_n > 0)
            byte(char(m_acc & 0xFF));
        if (!m_block.isEmpty())
            flush();
        m_out->append('\0');   // конец подблоков
    }

private:
    /// @brief Добавляет байт в текущий подблок (полный подблок сразу записывается).
    /// @param[in] c  байт
    void byte(char c)
    {
        m_block.append(c);
        if (m_block.size() == 255)
            flush();
    }
    /// @brief Пишет текущий подблок: байт длины и данные.
    void flush()
    {
        m_out->append(char(m_block.size()));
        m_out->append(m_block);
        m_block.clear();
    }
    /// Выходной буфер.
    QByteArray *m_out;
    /// Текущий подблок (до 255 байт).
    QByteArray m_block;
    /// Накопитель ещё не записанных бит.
    quint32 m_acc = 0;
    /// Число бит в накопителе.
    int m_n = 0;
};

} // namespace

bool GifWriter::open(const QString &path, int width, int height)
{
    m_w = width;
    m_h = height;
    m_file.setFileName(path);
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        m_error = m_file.errorString();
        return false;
    }
    QByteArray b("GIF89a");
    put16(b, m_w);
    put16(b, m_h);
    b.append(char(0x00));   // без общей палитры
    b.append(char(0x00));
    b.append(char(0x00));
    // NETSCAPE2.0: повторять бесконечно.
    b.append("\x21\xFF\x0B" "NETSCAPE2.0" "\x03\x01\x00\x00\x00", 19);
    return m_file.write(b) == b.size();
}

bool GifWriter::addFrame(const QImage &image, int delayCs)
{
    QImage img = image.convertToFormat(QImage::Format_RGB32);
    if (img.width() != m_w || img.height() != m_h)
        img = img.scaled(m_w, m_h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    // Палитра: 256 самых частых цветов (5-6-5).
    // Гистограмма цветов кадра по ключам 5-6-5; палитра — 256 самых частых ключей.
    std::vector<quint32> hist(65536, 0);
    for (int y = 0; y < m_h; ++y) {
        const QRgb *row = reinterpret_cast<const QRgb *>(img.constScanLine(y));
        for (int x = 0; x < m_w; ++x)
            ++hist[size_t(key565(row[x]))];
    }
    std::vector<int> used;
    for (int k = 0; k < 65536; ++k)
        if (hist[size_t(k)])
            used.push_back(k);
    std::sort(used.begin(), used.end(), [&hist](int a, int b) { return hist[size_t(a)] > hist[size_t(b)]; });
    const size_t nPal = std::min<size_t>(256, used.size());
    std::array<QRgb, 256> pal{};
    for (size_t i = 0; i < nPal; ++i) {
        const int k = used[i];
        pal[i] = qRgb(((k >> 11) & 31) * 255 / 31, ((k >> 5) & 63) * 255 / 63, (k & 31) * 255 / 31);
    }
    // Каждому цвету кадра — ближайший цвет палитры.
    std::vector<quint8> map(65536, 0);
    for (int k : used) {
        const int r = ((k >> 11) & 31) * 255 / 31, g = ((k >> 5) & 63) * 255 / 63, b = (k & 31) * 255 / 31;
        int best = 0, bestD = 1 << 30;
        for (size_t i = 0; i < nPal; ++i) {
            const int dr = qRed(pal[i]) - r, dg = qGreen(pal[i]) - g, db = qBlue(pal[i]) - b;
            const int d = 2 * dr * dr + 4 * dg * dg + 3 * db * db;
            if (d < bestD) {
                bestD = d;
                best = int(i);
                if (d == 0)
                    break;
            }
        }
        map[size_t(k)] = quint8(best);
    }
    QByteArray idx;
    idx.resize(qsizetype(m_w) * m_h);
    char *p = idx.data();
    for (int y = 0; y < m_h; ++y) {
        const QRgb *row = reinterpret_cast<const QRgb *>(img.constScanLine(y));
        for (int x = 0; x < m_w; ++x)
            *p++ = char(map[size_t(key565(row[x]))]);
    }

    QByteArray b;
    // Управление графикой: задержка, без прозрачности.
    b.append("\x21\xF9\x04\x04", 4);
    put16(b, std::max(1, delayCs));
    b.append('\0');
    b.append('\0');
    // Кадр с местной палитрой 256 цветов.
    b.append(',');
    put16(b, 0);
    put16(b, 0);
    put16(b, m_w);
    put16(b, m_h);
    b.append(char(0x87));   // местная палитра, 2^(7+1) = 256 цветов
    for (const QRgb c : pal) {
        b.append(char(qRed(c)));
        b.append(char(qGreen(c)));
        b.append(char(qBlue(c)));
    }
    if (m_file.write(b) != b.size()) {
        m_error = m_file.errorString();
        return false;
    }
    writeLzw(idx);
    return m_file.error() == QFileDevice::NoError;
}

void GifWriter::writeLzw(const QByteArray &indices)
{
    // Классический LZW GIF: начальная длина кода 9 бит (8 бит на пиксель), таблица до 4096.
    const int minCode = 8;
    const int clear = 1 << minCode, eoi = clear + 1;
    QByteArray out;
    out.append(char(minCode));
    BitPacker bits(&out);
    // Словарь: (код префикса << 8) | следующий индекс -> код цепочки. Длина кода растёт
    // с 9 до 12 бит; при заполнении таблицы (4096 кодов) пишется код очистки.
    QHash<quint32, int> dict;
    dict.reserve(4096);
    int next = eoi + 1, size = minCode + 1;
    bits.put(clear, size);
    int prefix = -1;
    for (const char ch : indices) {
        const int c = quint8(ch);
        if (prefix < 0) {
            prefix = c;
            continue;
        }
        const quint32 key = (quint32(prefix) << 8) | quint32(c);
        const auto it = dict.constFind(key);
        if (it != dict.constEnd()) {
            prefix = it.value();
            continue;
        }
        bits.put(prefix, size);
        if (next < 4096) {
            dict.insert(key, next);
            if (next == (1 << size) && size < 12)
                ++size;
            ++next;
        } else {
            bits.put(clear, size);
            dict.clear();
            next = eoi + 1;
            size = minCode + 1;
        }
        prefix = c;
    }
    if (prefix >= 0)
        bits.put(prefix, size);
    bits.put(eoi, size);
    bits.finish();
    m_file.write(out);
}

bool GifWriter::close()
{
    if (!m_file.isOpen())
        return false;
    m_file.write(";", 1);
    const bool ok = m_file.error() == QFileDevice::NoError;
    m_file.close();
    return ok;
}

} // namespace fieldview
