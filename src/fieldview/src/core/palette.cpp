// Цветовые шкалы — описание в palette.h.
/// @file palette.cpp
/// @brief Цветовые шкалы: опорные цвета, интерполяция в OKLab, исходные цвета FieldView 4.1.
///
/// Часть модуля FieldView (слой `core`). Описание открытых функций — в `palette.h`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#palettes
#include "palette.h"

#include <algorithm>
#include <cmath>

namespace fieldview {

namespace {

/// @brief Возвращает цвет по записи вида `#rrggbb`.
/// @param[in] s  запись цвета
/// @return Цвет.
QColor hex(const char *s) { return QColor(QString::fromLatin1(s)); }

/// @brief Цвет в пространстве OKLab: светлота L и оси a, b.
struct Lab { double L, a, b; };
/// @var Lab::L
/// Светлота OKLab (0 — чёрный, 1 — белый).
/// @var Lab::a
/// Ось a OKLab (зелёный — красный).
/// @var Lab::b
/// Ось b OKLab (синий — жёлтый).

/// @brief Переводит канал sRGB в линейную яркость.
/// @param[in] c  канал sRGB, 0 … 1
/// @return Линейное значение, 0 … 1.
double toLinear(double c)
{
    return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
}
/// @brief Переводит линейную яркость в канал sRGB (с обрезкой до [0, 1]).
/// @param[in] c  линейное значение
/// @return Канал sRGB, 0 … 1.
double toSrgb(double c)
{
    c = std::clamp(c, 0.0, 1.0);
    return c <= 0.0031308 ? 12.92 * c : 1.055 * std::pow(c, 1.0 / 2.4) - 0.055;
}

/// @brief Переводит цвет sRGB в OKLab.
/// @param[in] c  цвет
/// @return Координаты OKLab.
Lab toOklab(const QColor &c)
{
    const double r = toLinear(c.redF()), g = toLinear(c.greenF()), b = toLinear(c.blueF());
    const double l = std::cbrt(0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b);
    const double m = std::cbrt(0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b);
    const double s = std::cbrt(0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b);
    return {0.2104542553 * l + 0.7936177850 * m - 0.0040720468 * s,
            1.9779984951 * l - 2.4285922050 * m + 0.4505937099 * s,
            0.0259040371 * l + 0.7827717662 * m - 0.8086757660 * s};
}

/// @brief Переводит цвет из OKLab в sRGB (каналы вне диапазона обрезаются).
/// @param[in] x  координаты OKLab
/// @return Цвет.
QColor fromOklab(const Lab &x)
{
    const double l = std::pow(x.L + 0.3963377774 * x.a + 0.2158037573 * x.b, 3);
    const double m = std::pow(x.L - 0.1055613458 * x.a - 0.0638541728 * x.b, 3);
    const double s = std::pow(x.L - 0.0894841775 * x.a - 1.2914855480 * x.b, 3);
    const double r = 4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s;
    const double g = -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s;
    const double b = -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s;
    return QColor::fromRgbF(float(toSrgb(r)), float(toSrgb(g)), float(toSrgb(b)));
}

} // namespace

Colors20 classicFieldColors()
{
    Colors20 c;
    // CFldViewView::CFldViewView — те же формулы.
    for (int i = 0; i < 7; ++i)
        c[size_t(i)] = QColor(100, i * 35, 255 - i * 35);
    for (int i = 7; i < 14; ++i)
        c[size_t(i)] = QColor((i - 7) * 35, 255 - (i - 7) * 35, 150);
    for (int i = 14; i < 18; ++i)
        c[size_t(i)] = QColor(255, std::min(255, (i - 14) * 83), 100);
    c[18] = QColor(255, 255, 0);
    c[19] = QColor(255, 255, 255);
    return c;
}

Colors20 classicEpsColors()
{
    Colors20 c;
    for (int i = 0; i < 20; ++i)
        c[size_t(19 - i)] = QColor(std::max(0, 255 - i * 10), 255, std::min(255, i * 10));
    return c;
}

QVector<QColor> scaleStops(ScaleKind k)
{
    if (k == ScaleKind::Glow) {
        // 0 … max: фон → фиолетовый → циан → янтарный → почти белый.
        return {hex("#0a0e16"), hex("#1b2550"), hex("#3a3384"), hex("#7a55d6"), hex("#b07cff"),
                hex("#34e3c4"), hex("#ffcf5c"), hex("#fff3d6")};
    }
    // −max … 0 … +max
    return {hex("#f1e8ff"), hex("#d7bdff"), hex("#9b6cf0"), hex("#4a3590"), hex("#131a28"),
            hex("#0f4f45"), hex("#1a9a83"), hex("#34e3c4"), hex("#c4f7ee")};
}

QColor scaleColor(ScaleKind k, double t, const Colors20 &custom)
{
    t = std::clamp(t, 0.0, 1.0);
    if (k == ScaleKind::Custom) {
        const double pos = t * 19.0;
        const int i = std::min(int(pos), 18);
        const double f = pos - i;
        const QColor &a = custom[size_t(i)], &b = custom[size_t(i + 1)];
        return QColor::fromRgbF(float(a.redF() + (b.redF() - a.redF()) * f),
                                float(a.greenF() + (b.greenF() - a.greenF()) * f),
                                float(a.blueF() + (b.blueF() - a.blueF()) * f));
    }
    // Опорные цвета стоят равномерно: отрезок i — от t = i / (n - 1) до (i + 1) / (n - 1).
    const QVector<QColor> st = scaleStops(k);
    const double pos = t * (st.size() - 1);
    const int i = std::min(int(pos), int(st.size()) - 2);
    const double f = pos - i;
    const Lab a = toOklab(st[i]), b = toOklab(st[i + 1]);
    return fromOklab({a.L + (b.L - a.L) * f, a.a + (b.a - a.a) * f, a.b + (b.b - a.b) * f});
}

Colors20 sampleColors(ScaleKind k)
{
    Colors20 c;
    const Colors20 classic = classicFieldColors();
    for (int i = 0; i < 20; ++i)
        c[size_t(i)] = scaleColor(k, i / 19.0, classic);
    return c;
}

QImage scaleImage(ScaleKind k, const Colors20 &custom, int width)
{
    QImage img(width, 1, QImage::Format_RGBA8888);
    for (int x = 0; x < width; ++x)
        img.setPixelColor(x, 0, scaleColor(k, double(x) / (width - 1), custom));
    return img;
}

QColor defaultTopologyColor(int slot)
{
    switch (slot) {
    case 1: return hex("#9fb0c8");   // металл — text-dim
    case 2: return hex("#6c7d97");   // магнитная стенка — text-mute
    case 3: return hex("#34e3c4");   // порт — accent
    case 4: return hex("#3a4a6b");   // поглотитель — чуть светлее line, чтобы был виден на фоне
    case 5: return hex("#ffcf5c");   // плазма — accent-3
    default: return hex("#b07cff");  // диэлектрик — accent-2
    }
}

} // namespace fieldview
