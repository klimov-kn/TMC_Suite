/// @file standards.cpp
/// @brief Таблицы сечений волноводов (EIA/IEC, ГОСТ 20900-75) и подложек для образцов.
///
/// Часть модуля «Ядро TPL Designer», каталог образцов. Описание открытых функций —
/// в `standards.h`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#samples
#include "standards.h"

#include "lang.h"
#include "physics.h"

#include <cmath>

namespace tpl::samples {

double WaveguideStd::fc10() const
{
    return kLightSpeed / (2.0 * a);
}

QString WaveguideStd::label() const
{
    return tpl::tr("%1 — %2x%3 мм, %4-%5 ГГц%6", "%1 - %2x%3 mm, %4-%5 GHz%6")
        .arg(name, QString::number(a, 'g', 5), QString::number(b, 'g', 5),
             QString::number(fLo, 'g', 4), QString::number(fHi, 'g', 4),
             bandFromStandard ? QString() : tpl::tr(" (расч.)", " (calc.)"));
}

namespace {

/// Международная серия: EIA RS-261 (WR) и её аналог IEC 60153-2 (R). Размеры и рабочие
/// диапазоны — из стандарта.
/// @param[in] wr  обозначение EIA («WR-90»)
/// @param[in] r   обозначение IEC («R100»)
/// @param[in] a   широкая стенка, мм
/// @param[in] b   узкая стенка, мм
/// @param[in] lo  нижняя граница рабочего диапазона, ГГц
/// @param[in] hi  верхняя граница рабочего диапазона, ГГц
/// @return Строка таблицы с `bandFromStandard` = true.
WaveguideStd eia(const char *wr, const char *r, double a, double b, double lo, double hi)
{
    WaveguideStd w;
    w.name = QStringLiteral("%1 / %2").arg(QLatin1String(wr), QLatin1String(r));
    w.family = QStringLiteral("EIA / IEC");
    w.a = a;
    w.b = b;
    w.fLo = lo;
    w.fHi = hi;
    w.bandFromStandard = true;
    return w;
}

/// Отечественная серия ГОСТ 20900-75 «Трубы волноводные медные и латунные
/// прямоугольные». Стандарт задаёт только внутреннее сечение; рабочий диапазон считаем
/// по обычному правилу (1.25...1.9)*fc10: ниже — велики потери и дисперсия у отсечки,
/// выше — рядом отсечка H20. Поэтому он и помечен «расч.».
/// @param[in] a  широкая стенка, мм
/// @param[in] b  узкая стенка, мм
/// @return Строка таблицы с расчётным диапазоном (округлён до 0.01 ГГц).
WaveguideStd gost(double a, double b)
{
    WaveguideStd w;
    w.name = tpl::tr("ГОСТ %1x%2", "GOST %1x%2").arg(QString::number(a, 'g', 4),
                                                     QString::number(b, 'g', 4));
    w.family = tpl::tr("ГОСТ 20900-75", "GOST 20900-75");
    w.a = a;
    w.b = b;
    const double fc = kLightSpeed / (2.0 * a);
    // Два знака — чтобы в списке не стояло 8.14777 ГГц.
    w.fLo = std::round(1.25 * fc * 100.0) / 100.0;
    w.fHi = std::round(1.9 * fc * 100.0) / 100.0;
    w.bandFromStandard = false;
    return w;
}

/// @brief Строит таблицу сечений на текущем языке: 22 сечения EIA/IEC и 24 ГОСТ.
/// @return Таблица сечений.
QVector<WaveguideStd> makeWaveguides()
{
    return [] {
        QVector<WaveguideStd> t;
        t << eia("WR-650", "R14", 165.10, 82.55, 1.12, 1.70)
          << eia("WR-510", "R18", 129.54, 64.77, 1.45, 2.20)
          << eia("WR-430", "R22", 109.22, 54.61, 1.70, 2.60)
          << eia("WR-340", "R26", 86.36, 43.18, 2.20, 3.30)
          << eia("WR-284", "R32", 72.14, 34.04, 2.60, 3.95)
          << eia("WR-229", "R40", 58.17, 29.08, 3.30, 4.90)
          << eia("WR-187", "R48", 47.55, 22.15, 3.95, 5.85)
          << eia("WR-159", "R58", 40.39, 20.19, 4.90, 7.05)
          << eia("WR-137", "R70", 34.85, 15.80, 5.85, 8.20)
          << eia("WR-112", "R84", 28.50, 12.62, 7.05, 10.00)
          << eia("WR-90", "R100", 22.86, 10.16, 8.20, 12.40)
          << eia("WR-75", "R120", 19.05, 9.525, 10.00, 15.00)
          << eia("WR-62", "R140", 15.80, 7.90, 12.40, 18.00)
          << eia("WR-51", "R180", 12.95, 6.48, 15.00, 22.00)
          << eia("WR-42", "R220", 10.67, 4.32, 18.00, 26.50)
          << eia("WR-34", "R260", 8.636, 4.318, 22.00, 33.00)
          << eia("WR-28", "R320", 7.112, 3.556, 26.50, 40.00)
          << eia("WR-22", "R400", 5.690, 2.845, 33.00, 50.00)
          << eia("WR-19", "R500", 4.775, 2.388, 40.00, 60.00)
          << eia("WR-15", "R620", 3.759, 1.880, 50.00, 75.00)
          << eia("WR-12", "R740", 3.099, 1.549, 60.00, 90.00)
          << eia("WR-10", "R900", 2.540, 1.270, 75.00, 110.00);
        // Основная серия ГОСТ 20900-75 (без сечений пониженной высоты и без помеченных
        // в стандарте «не применять в новых разработках»).
        const double g[][2] = {{292, 146}, {248, 124}, {220, 110}, {180, 90}, {160, 80},
                               {130, 65},  {110, 55},  {90, 45},   {72, 34},  {58, 25},
                               {48, 24},   {40, 20},   {35, 15},   {28.5, 12.6}, {23, 10},
                               {19, 9.5},  {16, 8},    {13, 6.5},  {11, 5.5}, {9, 4.5},
                               {7.2, 3.4}, {5.2, 2.6}, {3.6, 1.8}, {2.4, 1.2}};
        for (const auto &s : g)
            t << gost(s[0], s[1]);
        return t;
    }();
}

} // namespace

// Имена ГОСТ-сечений зависят от языка (lang.h): таблица своя на каждый язык, чтобы
// переключение не «замораживало» надписи. Порядок и числа в обеих одинаковые —
// номер строки остаётся значением параметра образца.
const QVector<WaveguideStd> &waveguides()
{
    if (tpl::english()) {
        static const QVector<WaveguideStd> en = makeWaveguides();
        return en;
    }
    static const QVector<WaveguideStd> ru = makeWaveguides();
    return ru;
}

QStringList waveguideChoices()
{
    QStringList out;
    for (const WaveguideStd &w : waveguides())
        out << w.label();
    return out;
}

int defaultWaveguide()
{
    const QVector<WaveguideStd> &t = waveguides();
    for (int i = 0; i < t.size(); ++i) {
        if (t.at(i).name.startsWith(QLatin1String("WR-90 ")))
            return i;
    }
    return 0;
}

int waveguideForFreq(double fGHz)
{
    // Лучше всего — стандарт, в рабочей полосе которого частота лежит. Не нашлось
    // (частота между полосами или за краем таблицы) — любой, у которого она заведомо
    // выше отсечки H10 и ниже H20, ближайший по середине полосы. Молча вернуть
    // волновод, запредельный на этой частоте, нельзя: вход H10 в нём не возбудится.
    const QVector<WaveguideStd> &t = waveguides();
    int best = -1, fallback = -1;
    double bestDist = 1e300, fallbackDist = 1e300;
    for (int i = 0; i < t.size(); ++i) {
        if (!t.at(i).bandFromStandard)
            continue;
        const double mid = (t.at(i).fLo + t.at(i).fHi) / 2.0;
        const double dist = std::fabs(std::log(fGHz / mid));
        if (fGHz >= t.at(i).fLo && fGHz <= t.at(i).fHi && dist < bestDist) {
            best = i;
            bestDist = dist;
        }
        const double fc = t.at(i).fc10();
        if (fGHz > 1.1 * fc && fGHz < 1.95 * fc && dist < fallbackDist) {
            fallback = i;
            fallbackDist = dist;
        }
    }
    if (best >= 0)
        return best;
    return fallback >= 0 ? fallback : defaultWaveguide();
}

namespace {

/// @brief Строит таблицу подложек на текущем языке.
/// @return Таблица подложек (eps, стандартная толщина в мм, источник числа).
QVector<Substrate> makeSubstrates()
{
    // Справочные значения производителей и отраслевых ТУ. Проницаемость — на частотах
    // единиц гигагерц; толщина — самая ходовая из стандартного ряда.
    return {
        {QStringLiteral("FR-4"), 4.4, 1.5, tpl::tr("стеклотекстолит, eps 4.3-4.7", "glass-epoxy laminate, eps 4.3-4.7")},
        {QStringLiteral("Rogers RO4003C"), 3.38, 0.508, tpl::tr("данные Rogers Corp.", "Rogers Corp. data")},
        {QStringLiteral("Rogers RO4350B"), 3.48, 0.762, tpl::tr("данные Rogers Corp.", "Rogers Corp. data")},
        {QStringLiteral("Rogers RT/duroid 5880"), 2.20, 0.787, tpl::tr("данные Rogers Corp.", "Rogers Corp. data")},
        {tpl::tr("ФАФ-4Д", "FAF-4D"), 2.5, 1.0, tpl::tr("фольгированный фторопласт", "foil-clad PTFE")},
        {tpl::tr("ФЛАН-2.8", "FLAN-2.8"), 2.8, 1.0, tpl::tr("фольгированный армированный", "foil-clad reinforced")},
        {tpl::tr("ФЛАН-5", "FLAN-5"), 5.0, 1.0, tpl::tr("фольгированный армированный", "foil-clad reinforced")},
        {tpl::tr("ФЛАН-10", "FLAN-10"), 10.0, 1.0, tpl::tr("фольгированный армированный", "foil-clad reinforced")},
        {tpl::tr("Поликор (Al2O3)", "Polikor (Al2O3)"), 9.8, 1.0, tpl::tr("керамика ВК-100", "VK-100 ceramic")},
    };
}

} // namespace

// Названия и пояснения зависят от языка — таблица своя на каждый язык (см. waveguides()).
const QVector<Substrate> &substrates()
{
    if (tpl::english()) {
        static const QVector<Substrate> en = makeSubstrates();
        return en;
    }
    static const QVector<Substrate> ru = makeSubstrates();
    return ru;
}

QStringList substrateChoices()
{
    QStringList out;
    for (const Substrate &s : substrates())
        out << tpl::tr("%1 — eps %2, h %3 мм", "%1 - eps %2, h %3 mm")
                   .arg(s.name, QString::number(s.eps), QString::number(s.h));
    return out;
}

} // namespace tpl::samples
