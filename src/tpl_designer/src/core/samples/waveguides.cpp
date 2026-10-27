// Раздел «Волноводы и узлы»: стандартные сечения и неоднородности в H-плоскости.
//
// Общее у всех рецептов раздела: волновод идёт вдоль X, стенки — металл, вход H10
// слева и справа (или сверху у изгиба и тройника). Шаг сетки берётся так, чтобы в
// широкую стенку укладывалось целое число ячеек: тогда стенки лежат в узлах, и
// ширина волновода по сетке совпадает с номиналом стандарта.

/// @file waveguides.cpp
/// @brief Раздел каталога «Волноводы и узлы»: рецепты отрезка, диафрагмы, штыря, ступеньки, изгиба, тройника, фильтра, окна, нагрузки.
///
/// Часть модуля «Ядро TPL Designer», каталог образцов. Все рецепты — H-ядро,
/// металлические стенки, входы H10; регистрация — `addWaveguides`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#samples
#include "kit.h"
#include "rf.h"
#include "sample.h"
#include "standards.h"

#include "lang.h"
#include "physics.h"

#include <cmath>

namespace tpl::samples {

namespace {

using namespace kit;

/// Число pi.
constexpr double kPi = rf::kPi;

/// Раскладка волноводной сцены по сечению стандарта.
struct Guide {
    /// Строка таблицы стандартных сечений.
    WaveguideStd std;
    double a = 0.0;    ///< ширина по сетке
    double d = 0.0;    ///< шаг сетки
    double ylo = 0.0;  ///< нижняя стенка
    double yhi = 0.0;  ///< верхняя
    double fc = 0.0;   ///< отсечка H10
    double f0 = 0.0;   ///< середина рабочей полосы
};

/// \p cells — сколько ячеек в широкой стенке. 40 хватает, чтобы отсечка по сетке
/// разошлась с формулой меньше чем на процент.
///
/// Шаг сетки `a/cells` (6 знаков), нижняя стенка канала на высоте 3 шага, частота f0 —
/// середина рабочей полосы, округлённая до 0.01 ГГц.
/// @param[in] stdIndex  номер сечения в `waveguides()` (прижимается к границам таблицы)
/// @param[in] cells     число ячеек сетки на широкую стенку
/// @return Раскладка сцены, мм и ГГц.
/// @see https://www.tamic.ru/docs/theory/accuracy.html#grid-rules
Guide layout(int stdIndex, int cells = 40)
{
    Guide g;
    const QVector<WaveguideStd> &t = waveguides();
    g.std = t.value(std::clamp(stdIndex, 0, int(t.size()) - 1));
    g.a = g.std.a;
    g.d = std::round(g.a / cells * 1e6) / 1e6;
    g.ylo = 3 * g.d;
    g.yhi = g.ylo + g.a;
    g.fc = kLightSpeed / (2 * g.a);
    g.f0 = std::round((g.std.fLo + g.std.fHi) / 2.0 * 100.0) / 100.0;
    return g;
}

/// @brief Возвращает параметр выбора сечения `std` (умолчание — WR-90).
/// @return Параметр-выбор со списком `waveguideChoices()`.
Param stdParam()
{
    return Param::choice(QStringLiteral("std"), tpl::tr("Сечение волновода", "Waveguide cross-section"),
                         waveguideChoices(), defaultWaveguide(),
                         tpl::tr("EIA/IEC — размеры и полоса по стандарту; ГОСТ 20900-75 — "
                                 "сечение по стандарту, полоса расчётная 1.25-1.9 fc. "
                                 "Ядро двумерное: в расчёт идёт только широкая стенка a.",
                                 "EIA/IEC: dimensions and band from the standard; GOST 20900-75: "
                                 "cross-section from the standard, band calculated as 1.25-1.9 fc. "
                                 "The kernel is two-dimensional: only the broad wall a enters the "
                                 "calculation."));
}

/// @brief Возвращает параметр числа частот свипа `n` (2...99 — предел одного `.s`).
/// @param[in] def  значение по умолчанию
/// @return Целочисленный параметр.
Param pointsParam(int def = 21)
{
    return Param::integer(QStringLiteral("n"), tpl::tr("Частот в свипе",
                                                       "Frequencies in the sweep"), def, 2, 99,
                          tpl::tr("Больше 99 точек в один .s не помещается — предел пакета",
                                  "More than 99 points do not fit into one .s file: a package limit"));
}

/// Общие строки паспорта волновода.
/// @param[in] g  раскладка сцены
/// @return Стандарт, сечение a x b, рабочая полоса, отсечки H10/H20.
QVector<Row> guideRows(const Guide &g)
{
    return {
        {tpl::tr("Стандарт", "Standard"), g.std.name + QStringLiteral(" (") + g.std.family
                                         + QStringLiteral(")")},
        {tpl::tr("Сечение a x b", "Cross-section a x b"),
         tpl::tr("%1 x %2 мм", "%1 x %2 mm").arg(num(g.std.a, 5), num(g.std.b, 5))},
        {tpl::tr("Рабочая полоса", "Operating band"),
         tpl::tr("%1 - %2 ГГц%3", "%1 - %2 GHz%3")
             .arg(num(g.std.fLo, 4), num(g.std.fHi, 4),
                  g.std.bandFromStandard ? QString() : tpl::tr(" (расчётная)", " (calculated)"))},
        {tpl::tr("Отсечка H10 / H20", "Cutoff H10 / H20"),
         tpl::tr("%1 / %2 ГГц", "%1 / %2 GHz").arg(num(g.fc, 4), num(2 * g.fc, 4))},
    };
}

/// @brief Возвращает длину волны H10 в волноводе текстом для паспорта.
/// @param[in] a    ширина волновода, мм
/// @param[in] f    частота, ГГц
/// @param[in] eps  проницаемость заполнения
/// @return «N мм» или «ниже отсечки».
QString lgText(double a, double f, double eps = 1.0)
{
    const double lg = rf::lambdaGuide(a, f, eps);
    return lg > 0.0 ? mm(lg) : tpl::tr("ниже отсечки", "below cutoff");
}

/// Стандартная шапка описания задания (идёт в .tpl комментарием, поэтому без
/// греческих букв и стрелок — их нет в CP1251).
/// @param[in] what  что за устройство (первое предложение)
/// @param[in] g     раскладка сцены
/// @return Строка вида «\p what. Волновод WR-90 / R100, a = 22.86 мм, H10.».
QString describe(const QString &what, const Guide &g)
{
    return tpl::tr("%1. Волновод %2, a = %3 мм, H10.", "%1. Waveguide %2, a = %3 mm, H10.")
        .arg(what, g.std.name, num(g.std.a, 5));
}

// ------------------------------------------------------------ отрезок волновода

/// @brief Рецепт «Отрезок прямоугольного волновода» (`wg_section`).
///
/// Стандартное сечение, свип по рабочей полосе.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `std` — номер сечения в `waveguides()`;
///                `len` — длина отрезка, в длинах волны в волноводе;
///                `eps` — проницаемость заполнения;
///                `n` — число частот свипа по рабочей полосе (2...99)
/// @return Задание, паспорт «теория» и пояснения.
Build buildSection(const Values &v)
{
    Guide g = layout(int(v.value(QStringLiteral("std"))));
    const double eps = v.value(QStringLiteral("eps"));
    const double lenLg = v.value(QStringLiteral("len"));
    const int n = int(v.value(QStringLiteral("n")));

    // Заполнение опускает отсечку в sqrt(eps) раз — вместе с ней уезжает и полоса.
    const double fcEff = g.fc / std::sqrt(eps);
    const double lo = eps > 1.0 ? std::round(1.25 * fcEff * 100) / 100 : g.std.fLo;
    const double hi = eps > 1.0 ? std::round(1.9 * fcEff * 100) / 100 : g.std.fHi;
    const double f0 = (lo + hi) / 2.0;

    Builder b(QStringLiteral("wg"));
    b.p.delta = g.d;
    const double lg0 = rf::lambdaGuide(g.a, f0, eps);
    const double L = snap(lenLg * lg0, g.d);
    b.p.box = {0.0, L, 0.0, g.a + 6 * g.d};
    b.p.description = describe(tpl::tr("Отрезок волновода", "Waveguide section"), g);

    b.guideX(tpl::tr("волновод", "waveguide"), 0.0, L, g.ylo, g.yhi);
    if (eps > 1.0)
        b.dielectric(tpl::tr("заполнение", "filling"), eps, 0.0, L, g.ylo, g.yhi);
    Port &p1 = b.port(tpl::tr("вход 1", "port 1"), PortSide::Left, g.ylo, g.yhi, true);
    Port &p2 = b.port(tpl::tr("вход 2", "port 2"), PortSide::Right, g.ylo, g.yhi, false);
    p1.eps = eps;
    p2.eps = eps;
    p1.epsAuto = p2.epsAuto = false; // заполнение задано рецептом
    b.freqs(lo, hi, n);
    b.outputs(true);
    b.time(settleTime(L, lo, fcEff));

    QVector<Row> t = guideRows(g);
    if (eps > 1.0) {
        t << Row{tpl::tr("С заполнением eps = %1", "Filled with eps = %1").arg(num(eps)),
                 tpl::tr("отсечка %1 ГГц, полоса %2 - %3 ГГц", "cutoff %1 GHz, band %2 - %3 GHz")
                     .arg(num(fcEff, 4), num(lo, 4), num(hi, 4))};
    }
    t << Row{tpl::tr("λв на %1 / %2 / %3 ГГц",
                     "lambda_g at %1 / %2 / %3 GHz").arg(num(lo, 4), num(f0, 4), num(hi, 4)),
             QStringLiteral("%1 / %2 / %3")
                 .arg(lgText(g.a, lo, eps), lgText(g.a, f0, eps), lgText(g.a, hi, eps))};
    t << Row{tpl::tr("Длина отрезка", "Section length"),
             tpl::tr("%1 (%2 λв на %3 ГГц)",
                     "%1 (%2 lambda_g at %3 GHz)").arg(mm(L), num(L / lg0), num(f0, 4))};
    t << Row{tpl::tr("Ожидаемо", "Expected"), tpl::tr("|S11| = 0, |S21| = 1 во всей полосе",
                                                 "|S11| = 0, |S21| = 1 over the whole band")};
    return b.done(t, {tpl::tr("Стенки — металл толщиной в шаг сетки, как в авторских "
                              "заданиях. Шаг сетки a/40 = %1 мм.",
                              "The walls are metal one grid step thick, as in the author's tasks. "
                              "Grid step a/40 = %1 mm.").arg(num(g.d, 4))});
}

// ------------------------------------------------------------ диафрагма

/// @brief Рецепт «Индуктивная диафрагма» (`wg_iris`).
///
/// Окно в поперечной стенке — шунтирующая индуктивность.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `std` — номер сечения в `waveguides()`;
///                `d` — ширина окна, доля a;
///                `sym` — симметричная диафрагма (флажок);
///                `t` — толщина, доля a;
///                `n` — число частот свипа по рабочей полосе (2...99)
/// @return Задание, паспорт «теория» и пояснения.
Build buildIris(const Values &v)
{
    Guide g = layout(int(v.value(QStringLiteral("std"))));
    const double ratio = v.value(QStringLiteral("d"));
    const bool sym = v.value(QStringLiteral("sym")) > 0.5;
    const double tRel = v.value(QStringLiteral("t"));

    Builder b(QStringLiteral("iris"));
    b.p.delta = g.d;
    const double lg0 = rf::lambdaGuide(g.a, g.f0);
    const double L = snap(4.0 * lg0, g.d);
    const double xc = snap(L / 2.0, g.d);
    const double t = std::max(2 * g.d, snap(tRel * g.a, g.d));
    // Окно — по узлам сетки: у симметричной диафрагмы каждая «щека» — целое число ячеек.
    const double cheek = sym ? snap((1.0 - ratio) * g.a / 2.0, g.d) : snap((1.0 - ratio) * g.a, g.d);
    const double dWin = sym ? g.a - 2 * cheek : g.a - cheek;
    b.p.box = {0.0, L, 0.0, g.a + 6 * g.d};
    b.p.description = describe(sym ? tpl::tr("Симметричная индуктивная диафрагма", "Symmetric inductive iris")
                                   : tpl::tr("Несимметричная индуктивная диафрагма",
                                             "Asymmetric inductive iris"),
                               g);

    b.guideX(tpl::tr("волновод", "waveguide"), 0.0, L, g.ylo, g.yhi);
    b.rect(tpl::tr("диафрагма, нижняя щека",
                   "iris, lower jaw"), Material::Metal, xc - t / 2, xc + t / 2, g.ylo,
           g.ylo + cheek);
    if (sym)
        b.rect(tpl::tr("диафрагма, верхняя щека", "iris, upper jaw"), Material::Metal, xc - t / 2, xc + t / 2,
               g.yhi - cheek, g.yhi);
    b.port(tpl::tr("вход 1", "port 1"), PortSide::Left, g.ylo, g.yhi, true);
    b.port(tpl::tr("вход 2", "port 2"), PortSide::Right, g.ylo, g.yhi, false);
    b.freqs(g.std.fLo, g.std.fHi, int(v.value(QStringLiteral("n"))));
    b.outputs(true);
    b.time(settleTime(L, g.std.fLo, g.fc));

    QVector<Row> rows = guideRows(g);
    rows << Row{tpl::tr("Окно d (по сетке)", "Window d (on the grid)"),
                QStringLiteral("%1 (d/a = %2)").arg(mm(dWin), num(dWin / g.a))};
    rows << Row{tpl::tr("Толщина", "Thickness"), mm(t)};
    if (sym) {
        for (double f : {g.std.fLo, g.f0, g.std.fHi}) {
            const double bb = rf::irisSusceptance(g.a, dWin, f);
            rows << Row{tpl::tr("Тонкая диафрагма, %1 ГГц", "Thin iris, %1 GHz").arg(num(f, 4)),
                        QStringLiteral("B/Y0 = %1, |S11| = %2")
                            .arg(num(bb), db(rf::shuntReflection(bb)))};
        }
    }
    QStringList notes;
    notes << tpl::tr("Теория — приближение бесконечно тонкой диафрагмы (Маркувиц); "
                     "толщина делает диафрагму «сильнее», |S11| ядра будет выше.",
                     "Theory: approximation of an infinitely thin iris (Marcuvitz); the thickness "
                     "makes the iris \"stronger\", so the kernel's |S11| will be higher.");
    if (!sym)
        notes << tpl::tr("Для несимметричной диафрагмы простой формулы нет — смотрите "
                         "результат ядра.",
                         "There is no simple formula for an asymmetric iris; see the kernel result.");
    return b.done(rows, notes);
}

// ------------------------------------------------------------ штырь

/// @brief Рецепт «Индуктивный штырь» (`wg_post`).
///
/// Металлический стержень поперёк волновода.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `std` — номер сечения в `waveguides()`;
///                `dia` — диаметр, доля a;
///                `off` — смещение от оси, доля a;
///                `n` — число частот свипа по рабочей полосе (2...99)
/// @return Задание, паспорт «теория» и пояснения.
Build buildPost(const Values &v)
{
    Guide g = layout(int(v.value(QStringLiteral("std"))), 60);
    const double dRel = v.value(QStringLiteral("dia"));
    const double off = v.value(QStringLiteral("off"));

    Builder b(QStringLiteral("post"));
    b.p.delta = g.d;
    const double lg0 = rf::lambdaGuide(g.a, g.f0);
    const double L = snap(4.0 * lg0, g.d);
    const double dia = std::max(2 * g.d, dRel * g.a);
    const QPointF c(snap(L / 2.0, g.d), (g.ylo + g.yhi) / 2.0 + off * g.a);
    b.p.box = {0.0, L, 0.0, g.a + 6 * g.d};
    b.p.description = describe(tpl::tr("Индуктивный штырь", "Inductive post"), g);

    b.guideX(tpl::tr("волновод", "waveguide"), 0.0, L, g.ylo, g.yhi);
    b.circle(tpl::tr("штырь", "post"), Material::Metal, c, dia / 2.0);
    b.port(tpl::tr("вход 1", "port 1"), PortSide::Left, g.ylo, g.yhi, true);
    b.port(tpl::tr("вход 2", "port 2"), PortSide::Right, g.ylo, g.yhi, false);
    b.freqs(g.std.fLo, g.std.fHi, int(v.value(QStringLiteral("n"))));
    b.outputs(true);
    b.time(settleTime(L, g.std.fLo, g.fc));

    QVector<Row> rows = guideRows(g);
    rows << Row{tpl::tr("Диаметр штыря",
                        "Post diameter"), QStringLiteral("%1 (d/a = %2)").arg(mm(dia), num(dia / g.a))};
    rows << Row{tpl::tr("Смещение от оси", "Offset from the axis"), mm(off * g.a)};
    if (std::fabs(off) < 1e-9) {
        for (double f : {g.std.fLo, g.f0, g.std.fHi}) {
            const double x = rf::postReactance(g.a, dia, f);
            rows << Row{tpl::tr("Тонкий штырь, %1 ГГц", "Thin post, %1 GHz").arg(num(f, 4)),
                        QStringLiteral("X/Z0 = %1, |S11| = %2")
                            .arg(num(x), db(1.0 / std::sqrt(1.0 + 4.0 * x * x)))};
        }
    }
    return b.done(rows,
                  {tpl::tr("Теория — тонкий штырь в центре (Маркувиц); у толстого штыря и "
                           "при смещении отражение другое — его и показывает ядро.",
                           "Theory: a thin post on the axis (Marcuvitz); a thick or offset post "
                           "reflects differently, and that is what the kernel shows."),
                   tpl::tr("Шаг сетки мельче обычного, a/60: круг на сетке — «лесенка».",
                           "The grid step is finer than usual, a/60: a circle on the grid is a "
                           "\"staircase\".")});
}

// ------------------------------------------------------------ ступенька

/// @brief Рецепт «Ступенька по ширине» (`wg_step`).
///
/// Стык двух волноводов разной ширины.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `std` — номер сечения в `waveguides()`;
///                `ratio` — отношение ширин a2/a1
/// @return Задание, паспорт «теория» и пояснения.
Build buildStep(const Values &v)
{
    Guide g = layout(int(v.value(QStringLiteral("std"))));
    const double ratio = v.value(QStringLiteral("ratio"));

    Builder b(QStringLiteral("step"));
    b.p.delta = g.d;
    const double a2 = g.a - 2.0 * snap((g.a - ratio * g.a) / 2.0, g.d);
    const double fc2 = kLightSpeed / (2 * a2);
    // Полоса — там, где обе половины одномодовые. Стык симметричный, поэтому H20 он
    // не возбуждает; следующая — H30.
    const double lo = std::max(g.std.fLo, std::round(1.1 * fc2 * 100) / 100);
    const double hi = std::min(g.std.fHi, std::round(0.95 * 3.0 * fc2 * 100) / 100);
    const double lg0 = rf::lambdaGuide(g.a, (lo + hi) / 2.0);
    const double half = snap(2.0 * lg0, g.d);
    const double L = 2 * half;
    const double wMax = std::max(g.a, a2);
    const double yc = 3 * g.d + wMax / 2.0;
    const double y1lo = yc - g.a / 2, y1hi = yc + g.a / 2;
    const double y2lo = yc - a2 / 2, y2hi = yc + a2 / 2;
    b.p.box = {0.0, L, 0.0, wMax + 6 * g.d};
    b.p.description = describe(tpl::tr("Ступенька по ширине волновода", "Step in waveguide width"), g);

    b.polyline(tpl::tr("нижняя стенка", "bottom wall"), Material::Metal,
               {QPointF(0, y1lo), QPointF(half, y1lo), QPointF(half, y2lo), QPointF(L, y2lo)});
    b.polyline(tpl::tr("верхняя стенка", "top wall"), Material::Metal,
               {QPointF(0, y1hi), QPointF(half, y1hi), QPointF(half, y2hi), QPointF(L, y2hi)});
    const int id1 = b.port(tpl::tr("вход 1 (a1)", "port 1 (a1)"), PortSide::Left, y1lo, y1hi, true).id;
    const int id2 = b.port(tpl::tr("вход 2 (a2)", "port 2 (a2)"), PortSide::Right, y2lo, y2hi, false).id;
    b.p.exciters = {id1, id2};
    b.freqs(lo, hi, 21);
    b.outputs(true);
    b.time(settleTime(L, lo, std::max(g.fc, fc2)));

    QVector<Row> rows = guideRows(g);
    rows << Row{tpl::tr("Ширины a1 -> a2",
                        "Widths a1 -> a2"), tpl::tr("%1 -> %2 мм",
                                                           "%1 -> %2 mm").arg(num(g.a, 5), num(a2, 5))};
    rows << Row{tpl::tr("Отсечки H10",
                        "Cutoffs H10"), tpl::tr("%1 / %2 ГГц",
                                                       "%1 / %2 GHz").arg(num(g.fc, 4), num(fc2, 4))};
    rows << Row{tpl::tr("Полоса расчёта", "Calculation band"),
                tpl::tr("%1 - %2 ГГц (обе половины одномодовые)",
                        "%1 - %2 GHz (both halves single-mode)").arg(num(lo, 4), num(hi, 4))};
    return b.done(rows, {tpl::tr("Возбуждаются оба входа по очереди — в .s полная матрица "
                                 "2x2. Входы разной ширины, поэтому S11 и S22 разные.",
                                 "Both ports are excited in turn, so the .s file holds the full 2x2 "
                                 "matrix. The ports have different widths, so S11 and S22 differ.")});
}

// ------------------------------------------------------------ изгиб

/// @brief Рецепт «H-изгиб 90°» (`wg_bend`).
///
/// Поворот волновода со срезанным наружным углом.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `std` — номер сечения в `waveguides()`;
///                `cut` — срез угла, доля a;
///                `n` — число частот свипа по рабочей полосе (2...99)
/// @return Задание, паспорт «теория» и пояснения.
Build buildBend(const Values &v)
{
    Guide g = layout(int(v.value(QStringLiteral("std"))));
    const double cut = v.value(QStringLiteral("cut"));

    Builder b(QStringLiteral("bend"));
    b.p.delta = g.d;
    const double lg0 = rf::lambdaGuide(g.a, g.f0);
    const double arm = snap(2.0 * lg0, g.d);
    const double xc = arm;                 // начало угла
    const double ytop = g.yhi + arm;       // конец вертикального плеча
    const double m = snap(cut * g.a, g.d); // срез наружного угла по каждой стороне
    b.p.box = {0.0, xc + g.a + 3 * g.d, 0.0, ytop};
    b.p.description = describe(tpl::tr("H-изгиб 90 град. со срезанным углом",
                                       "H-bend 90 deg. with a mitred corner"), g);

    QVector<QPointF> outer;
    outer << QPointF(0, g.ylo);
    if (m > 0.0)
        outer << QPointF(xc + g.a - m, g.ylo) << QPointF(xc + g.a, g.ylo + m);
    else
        outer << QPointF(xc + g.a, g.ylo);
    outer << QPointF(xc + g.a, ytop);
    b.polyline(tpl::tr("наружная стенка", "outer wall"), Material::Metal, outer);
    b.polyline(tpl::tr("внутренняя стенка", "inner wall"), Material::Metal,
               {QPointF(0, g.yhi), QPointF(xc, g.yhi), QPointF(xc, ytop)});
    b.port(tpl::tr("вход 1", "port 1"), PortSide::Left, g.ylo, g.yhi, true);
    b.port(tpl::tr("вход 2", "port 2"), PortSide::Top, xc, xc + g.a, false);
    b.freqs(g.std.fLo, g.std.fHi, int(v.value(QStringLiteral("n"))));
    b.outputs(true);
    b.time(settleTime(2 * arm + g.a, g.std.fLo, g.fc));

    QVector<Row> rows = guideRows(g);
    rows << Row{tpl::tr("Срез угла", "Corner mitre"),
                m > 0.0 ? tpl::tr("%1 по каждой стенке (%2 a)",
                                  "%1 along each wall (%2 a)").arg(mm(m), num(m / g.a))
                        : tpl::tr("нет — прямой угол", "none, square corner")};
    rows << Row{tpl::tr("Плечи", "Arms"), tpl::tr("по %1 (2 λв)", "%1 each (2 lambda_g)").arg(mm(arm))};
    return b.done(rows, {tpl::tr("Формулы для срезанного изгиба нет — его подбирают. "
                                 "Поменяйте долю среза и сравните |S11|: прямой угол "
                                 "отражает заметно сильнее.",
                                 "There is no formula for a mitred bend; it is tuned. Change the "
                                 "mitre fraction and compare |S11|: a square corner reflects "
                                 "noticeably more.")});
}

// ------------------------------------------------------------ тройник

/// @brief Рецепт «H-плоскостной тройник» (`wg_tee`).
///
/// Разветвление на три плеча, матрица 3x3.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `std` — номер сечения в `waveguides()`;
///                `n` — число частот свипа по рабочей полосе (2...99)
/// @return Задание, паспорт «теория» и пояснения.
Build buildTee(const Values &v)
{
    Guide g = layout(int(v.value(QStringLiteral("std"))));

    Builder b(QStringLiteral("tee"));
    b.p.delta = g.d;
    const double lg0 = rf::lambdaGuide(g.a, g.f0);
    const double arm = snap(2.0 * lg0, g.d);
    const double L = 2 * arm + g.a;
    const double xl = arm, xr = arm + g.a;
    const double ytop = g.yhi + arm;
    b.p.box = {0.0, L, 0.0, ytop};
    b.p.description = describe(tpl::tr("H-плоскостной тройник", "H-plane tee"), g);

    b.polyline(tpl::tr("нижняя стенка", "bottom wall"), Material::Metal,
               {QPointF(0, g.ylo), QPointF(L / 2, g.ylo), QPointF(L, g.ylo)});
    b.polyline(tpl::tr("стенка слева от ответвления", "wall left of the branch"), Material::Metal,
               {QPointF(0, g.yhi), QPointF(xl, g.yhi), QPointF(xl, ytop)});
    b.polyline(tpl::tr("стенка справа от ответвления", "wall right of the branch"), Material::Metal,
               {QPointF(xr, ytop), QPointF(xr, g.yhi), QPointF(L, g.yhi)});
    const Port &p1 = b.port(tpl::tr("вход 1", "port 1"), PortSide::Left, g.ylo, g.yhi, true);
    const int id1 = p1.id;
    const int id2 = b.port(tpl::tr("вход 2", "port 2"), PortSide::Right, g.ylo, g.yhi, false).id;
    const int id3 = b.port(tpl::tr("вход 3 (ответвление)",
                                   "port 3 (branch)"), PortSide::Top, xl, xr, false).id;
    b.p.exciters = {id1, id2, id3};
    b.freqs(g.std.fLo, g.std.fHi, int(v.value(QStringLiteral("n"))));
    b.outputs(true);
    b.time(settleTime(L + arm, g.std.fLo, g.fc));

    QVector<Row> rows = guideRows(g);
    rows << Row{tpl::tr("Плечи", "Arms"), tpl::tr("по %1", "%1 each").arg(mm(arm))};
    rows << Row{tpl::tr("Шагов расчёта", "Calculation steps"),
                tpl::tr("%1 = 3 возбуждения x %2 частот", "%1 = 3 excitations x %2 frequencies")
                    .arg(b.p.stepCount()).arg(b.p.freqs.size())};
    return b.done(rows, {tpl::tr("Все три входа возбуждаются по очереди — в .s полная "
                                 "матрица 3x3. Идеально согласовать все три плеча "
                                 "тройника без потерь нельзя — это видно по S11, S22, S33.",
                                 "All three ports are excited in turn, so the .s file holds the "
                                 "full 3x3 matrix. All three arms of a lossless tee cannot be "
                                 "matched at once; this shows in S11, S22, S33.")});
}

// ------------------------------------------------------------ фильтр

/// @brief Рецепт «Фильтр на индуктивных диафрагмах» (`wg_filter`).
///
/// Полосовой фильтр Чебышёва на диафрагмах.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `std` — номер сечения в `waveguides()`;
///                `f0` — центральная частота, ГГц;
///                `fbw` — полоса пропускания, %;
///                `order` — порядок;
///                `ripple` — пульсации, дБ;
///                `n` — число частот свипа по рабочей полосе (2...99)
/// @return Задание, паспорт «теория» и пояснения.
Build buildFilter(const Values &v)
{
    Guide g = layout(int(v.value(QStringLiteral("std"))), 48);
    const int n = int(v.value(QStringLiteral("order")));
    const double fbw = v.value(QStringLiteral("fbw")) / 100.0;
    const double ripple = v.value(QStringLiteral("ripple"));
    double f0 = v.value(QStringLiteral("f0"));
    QStringList notes;
    // Центр полосы обязан лежать в рабочей полосе выбранного сечения.
    const double fMin = g.std.fLo / (1.0 - fbw);
    const double fMax = g.std.fHi / (1.0 + fbw);
    if (f0 < fMin || f0 > fMax) {
        const double fixed = std::clamp(f0, fMin, fMax);
        notes << tpl::tr("Частота %1 ГГц вне полосы сечения — взята %2 ГГц.",
                         "Frequency %1 GHz is outside the band of the cross-section; %2 GHz is "
                         "used.")
                     .arg(num(f0, 4), num(fixed, 4));
        f0 = std::round(fixed * 100) / 100;
    }

    // --- расчёт по Маттею (фильтр на связанных резонаторах с индуктивными окнами) ---
    const double f1 = f0 * (1.0 - fbw / 2.0), f2 = f0 * (1.0 + fbw / 2.0);
    const double lg0 = rf::lambdaGuide(g.a, f0);
    const double w = (rf::lambdaGuide(g.a, f1) - rf::lambdaGuide(g.a, f2)) / lg0;
    const QVector<double> gp = rf::chebyshev(n, ripple);
    QVector<double> K(n + 1), X(n + 1), dWin(n + 1);
    for (int j = 0; j <= n; ++j) {
        if (j == 0)
            K[j] = std::sqrt(kPi * w / (2.0 * gp[0] * gp[1]));
        else if (j == n)
            K[j] = std::sqrt(kPi * w / (2.0 * gp[n] * gp[n + 1]));
        else
            K[j] = kPi * w / (2.0 * std::sqrt(gp[j] * gp[j + 1]));
        X[j] = K[j] / (1.0 - K[j] * K[j]);
        dWin[j] = rf::irisWindowForReactance(g.a, X[j], f0);
    }
    QVector<double> len(n + 1);
    for (int j = 1; j <= n; ++j) {
        const double theta = kPi - 0.5 * (std::atan(2.0 * X[j - 1]) + std::atan(2.0 * X[j]));
        len[j] = theta * lg0 / (2.0 * kPi);
    }

    Builder b(QStringLiteral("filter"));
    b.p.delta = g.d;
    const double t = 2 * g.d;
    const double lead = snap(1.5 * lg0, g.d);
    QVector<double> xs;
    double x = lead;
    xs << x;
    for (int j = 1; j <= n; ++j) {
        x += len[j];
        xs << x;
    }
    const double L = snap(x + lead, g.d);
    b.p.box = {0.0, L, 0.0, g.a + 6 * g.d};
    b.p.description = tpl::tr("Фильтр Чебышёва %1-го порядка на индуктивных диафрагмах: "
                              "f0 = %2 ГГц, полоса %3 %. Волновод %4.",
                              "Chebyshev filter of order %1 on inductive irises: f0 = %2 GHz, "
                              "bandwidth %3 %. Waveguide %4.")
                          .arg(n).arg(num(f0, 4), num(fbw * 100.0), g.std.name);

    b.guideX(tpl::tr("волновод", "waveguide"), 0.0, L, g.ylo, g.yhi);
    for (int j = 0; j <= n; ++j) {
        const double cheek = (g.a - dWin[j]) / 2.0;
        b.rect(tpl::tr("диафрагма %1, низ", "iris %1, bottom").arg(j + 1), Material::Metal, xs[j] - t / 2,
               xs[j] + t / 2, g.ylo, g.ylo + cheek);
        b.rect(tpl::tr("диафрагма %1, верх", "iris %1, top").arg(j + 1), Material::Metal, xs[j] - t / 2,
               xs[j] + t / 2, g.yhi - cheek, g.yhi);
    }
    b.port(tpl::tr("вход 1", "port 1"), PortSide::Left, g.ylo, g.yhi, true);
    b.port(tpl::tr("вход 2", "port 2"), PortSide::Right, g.ylo, g.yhi, false);
    const double bw = fbw * f0;
    const double lo = std::max(g.std.fLo, f0 - 2.0 * bw);
    const double hi = std::min(g.std.fHi, f0 + 2.0 * bw);
    b.freqs(lo, hi, int(v.value(QStringLiteral("n"))));
    b.outputs(true);
    b.time(settleTime(L, lo, g.fc, 5.0 * n / (kPi * bw)));

    QVector<Row> rows = guideRows(g);
    rows << Row{tpl::tr("Полоса пропускания", "Passband"),
                tpl::tr("%1 - %2 ГГц, пульсации %3 дБ",
                        "%1 - %2 GHz, ripple %3 dB").arg(num(f1, 4), num(f2, 4), num(ripple))};
    QStringList gs;
    for (int j = 1; j <= n; ++j)
        gs << num(gp[j], 4);
    rows << Row{tpl::tr("Прототип g1...g%1", "Prototype g1...g%1").arg(n), gs.join(QStringLiteral(", "))};
    for (int j = 0; j <= n; ++j)
        rows << Row{tpl::tr("Окно %1", "Window %1").arg(j + 1),
                    QStringLiteral("%1 (X/Z0 = %2)").arg(mm(dWin[j]), num(X[j]))};
    for (int j = 1; j <= n; ++j)
        rows << Row{tpl::tr("Резонатор %1", "Resonator %1").arg(j), mm(len[j])};
    notes << tpl::tr("Расчёт по Маттею в приближении бесконечно тонких диафрагм. У "
                     "диафрагм толщиной %1 на сетке полоса ядра уходит ВВЕРХ примерно на "
                     "3 %, а согласование в полосе хуже расчётного (проверено прогоном: "
                     "для умолчаний полоса 10.05-10.55 ГГц вместо 9.75-10.25). Дальше "
                     "фильтр настраивают — удлиняют резонаторы и подбирают окна; это и "
                     "есть учебная задача.",
                     "Design after Matthaei in the approximation of infinitely thin irises. With "
                     "irises %1 thick on the grid, the kernel's passband shifts UP by about 3 %, "
                     "and the in-band match is worse than designed (checked by a run: for the "
                     "defaults the band is 10.05-10.55 GHz instead of 9.75-10.25). Then the filter "
                     "is tuned: the resonators are lengthened and the windows adjusted; that is the "
                     "training task.")
                 .arg(mm(t));
    return b.done(rows, notes);
}

// ------------------------------------------------------------ полуволновое окно

/// @brief Рецепт «Диэлектрическое окно lambda/2» (`wg_window`).
///
/// Полуволновая пластина: точная формула для проверки ядра.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `std` — номер сечения в `waveguides()`;
///                `eps` — проницаемость пластины;
///                `thick` — толщина, доля полуволны в пластине;
///                `n` — число частот свипа по рабочей полосе (2...99)
/// @return Задание, паспорт «теория» и пояснения.
Build buildWindow(const Values &v)
{
    Guide g = layout(int(v.value(QStringLiteral("std"))));
    const double eps = v.value(QStringLiteral("eps"));
    const double k = v.value(QStringLiteral("thick"));

    Builder b(QStringLiteral("window"));
    b.p.delta = g.d;
    const double lgEps = rf::lambdaGuide(g.a, g.f0, eps);
    const double t = std::max(g.d, snap(k * lgEps / 2.0, g.d));
    const double lg0 = rf::lambdaGuide(g.a, g.f0);
    const double L = snap(4.0 * lg0 + t, g.d);
    const double x0 = snap((L - t) / 2.0, g.d);
    b.p.box = {0.0, L, 0.0, g.a + 6 * g.d};
    b.p.description = describe(tpl::tr("Диэлектрическое окно (полуволновая пластина)",
                                       "Dielectric window (half-wave plate)"), g);

    b.guideX(tpl::tr("волновод", "waveguide"), 0.0, L, g.ylo, g.yhi);
    b.dielectric(tpl::tr("пластина", "plate"), eps, x0, x0 + t, g.ylo, g.yhi);
    b.port(tpl::tr("вход 1", "port 1"), PortSide::Left, g.ylo, g.yhi, true);
    b.port(tpl::tr("вход 2", "port 2"), PortSide::Right, g.ylo, g.yhi, false);
    b.freqs(g.std.fLo, g.std.fHi, int(v.value(QStringLiteral("n"))));
    b.outputs(true);
    b.time(settleTime(L, g.std.fLo, g.fc));

    QVector<Row> rows = guideRows(g);
    rows << Row{tpl::tr("Пластина", "Plate"),
                tpl::tr("eps = %1, толщина %2 (%3 λв/2 в диэлектрике на %4 ГГц)",
                        "eps = %1, thickness %2 (%3 lambda_g/2 in the dielectric at %4 GHz)")
                    .arg(num(eps), mm(t), num(2 * t / lgEps), num(g.f0, 4))};
    for (double f : {g.std.fLo, g.f0, g.std.fHi}) {
        const double r = std::abs(rf::slabReflection(g.a, eps, t, f));
        rows << Row{tpl::tr("Точно, %1 ГГц", "Exact, %1 GHz").arg(num(f, 4)),
                    QStringLiteral("|S11| = %1 (%2)").arg(num(r), db(r))};
    }
    return b.done(rows, {tpl::tr("Слой толщиной в полволны прозрачен: отражения от двух "
                                 "его граней гасят друг друга. Формула точная — хорошая "
                                 "проверка ядра.",
                                 "A layer half a wavelength thick is transparent: reflections from "
                                 "its two faces cancel each other. The formula is exact, a good "
                                 "check of the kernel.")});
}

// ------------------------------------------------------------ нагрузка

/// @brief Рецепт «Нагрузка волновода» (`wg_load`).
///
/// Три варианта геометрии: короткое замыкание, плоский поглотитель и клин.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `std` — номер сечения в `waveguides()`;
///                `wedge` — длина клина, в длинах волны в волноводе;
///                `n` — число частот свипа по рабочей полосе (2...99)
/// @return Задание, паспорт «теория» и пояснения.
/// @see https://www.tamic.ru/docs/theory/boundaries.html#absorber
Build buildLoad(const Values &v)
{
    Guide g = layout(int(v.value(QStringLiteral("std"))));
    const double wedge = v.value(QStringLiteral("wedge"));

    Builder b(QStringLiteral("load"));
    b.p.delta = g.d;
    const double lg0 = rf::lambdaGuide(g.a, g.f0);
    const double lead = snap(2.0 * lg0, g.d);
    const double lw = snap(wedge * lg0, g.d);
    const double xEnd = lead + lw;
    const double L = xEnd + 3 * g.d;
    const double yc = (g.ylo + g.yhi) / 2.0;
    b.p.box = {0.0, L, 0.0, g.a + 6 * g.d};
    b.p.description = describe(tpl::tr("Нагрузка волновода: короткое замыкание, "
                                       "плоский поглотитель и клин",
                                       "Waveguide termination: short circuit, flat absorber and "
                                       "wedge"),
                               g);

    b.guideX(tpl::tr("волновод", "waveguide"), 0.0, xEnd, g.ylo, g.yhi);
    const QVector<QPointF> across{QPointF(xEnd, g.ylo), QPointF(xEnd, yc), QPointF(xEnd, g.yhi)};
    const int shortId = b.polyline(tpl::tr("короткозамыкатель", "short"), Material::Metal, across).id;
    const int flatId = b.polyline(tpl::tr("поглотитель в торце",
                                          "end absorber"), Material::Absorber, across).id;
    const int wedgeId = b.polygon(tpl::tr("клин поглотителя", "absorber wedge"), Material::Absorber,
                                  {QPointF(xEnd, g.ylo), QPointF(xEnd, g.yhi), QPointF(lead, yc)})
                            .id;
    b.port(tpl::tr("вход", "port"), PortSide::Left, g.ylo, g.yhi, true);

    // Три варианта в одном задании: короткое замыкание (|S11| = 1 — проверка
    // постановки), плоский поглотитель в торце и клин перед короткозамыкателем.
    Variant sc;
    sc.name = tpl::tr("короткое замыкание", "short circuit");
    sc.tag = QStringLiteral("short");
    sc.disabledShapes = {flatId, wedgeId};
    Variant flat;
    flat.name = tpl::tr("плоский поглотитель", "flat absorber");
    flat.tag = QStringLiteral("flat");
    flat.disabledShapes = {shortId, wedgeId};
    Variant wedgeV;
    wedgeV.name = tpl::tr("клин", "wedge");
    wedgeV.tag = QStringLiteral("wedge");
    wedgeV.disabledShapes = {flatId};
    b.p.variants << sc << flat << wedgeV;

    b.freqs(g.std.fLo, g.std.fHi, int(v.value(QStringLiteral("n"))));
    b.outputs(true);
    b.time(settleTime(L, g.std.fLo, g.fc));

    QVector<Row> rows = guideRows(g);
    rows << Row{tpl::tr("Клин", "Wedge"), tpl::tr("длина %1 (%2 λв)",
                                                "length %1 (%2 lambda_g)").arg(mm(lw), num(lw / lg0))};
    // Поглотитель ядра согласован для НОРМАЛЬНОГО падения. Волна H10 — две плоские
    // волны под углом к оси, у которого cos = λ/λв, и от плоского поглотителя они
    // отражаются как от сопротивления, согласованного не под тем углом.
    for (double f : {g.std.fLo, g.f0, g.std.fHi}) {
        const double c = rf::lambda0(f) / rf::lambdaGuide(g.a, f);
        rows << Row{tpl::tr("Плоский поглотитель, %1 ГГц", "Flat absorber, %1 GHz").arg(num(f, 4)),
                    tpl::tr("|S11| = (1 - λ/λв)/(1 + λ/λв) = %1",
                            "|S11| = (1 - lambda/lambda_g)/(1 + lambda/lambda_g) = %1").arg(db((1 - c) / (1 + c)))};
    }
    rows << Row{tpl::tr("Короткое замыкание", "Short circuit"), tpl::tr("|S11| = 1 (0 дБ)", "|S11| = 1 (0 dB)")};
    return b.done(rows, {tpl::tr("Варианты считаются одним заданием: load_short.s, "
                                 "load_flat.s и load_wedge.s. Поглотитель ядра согласован "
                                 "для нормального падения, а волна H10 бежит к торцу "
                                 "наклонными плоскими волнами — отсюда формула для "
                                 "плоского поглотителя. Для клина формулы нет.",
                                 "The variants are computed as one task: load_short.s, load_flat.s "
                                 "and load_wedge.s. The kernel's absorber is matched for normal "
                                 "incidence, while the H10 wave travels to the end as oblique plane "
                                 "waves; hence the formula for the flat absorber. There is no "
                                 "formula for the wedge.")});
}

/// @brief Возвращает общий абзац справки раздела: как считать и где смотреть результат.
/// @return Текст на текущем языке, начинается с пустой строки.
QString helpCommon()
{
    return tpl::tr(
        "\n\nКак считать: сохраните задание и запустите ядро tmc_rth. Перед повторным "
        "прогоном удалите старый .s — ядро дописывает его, а не перезаписывает. "
        "Частотные характеристики открываются в TMCGROUT, сигналы — в TMCROS.",
        "\n\nHow to compute: save the task and run the tmc_rth kernel. Delete the old .s file "
        "before a repeated run: the kernel appends to it rather than overwriting it. Frequency "
        "responses open in TMCGROUT, signals in TMCROS.");
}

} // namespace

void addWaveguides(QVector<Sample> *out)
{
    const QString sec = QStringLiteral("waveguide");
    const QString stdRef = tpl::tr("EIA RS-261, IEC 60153-2, ГОСТ 20900-75",
                                   "EIA RS-261, IEC 60153-2, GOST 20900-75");

    {
        Sample s;
        s.id = QStringLiteral("wg_section");
        s.section = sec;
        s.title = tpl::tr("Отрезок прямоугольного волновода", "Rectangular waveguide section");
        s.summary = tpl::tr("Стандартное сечение, свип по рабочей полосе",
                            "Standard cross-section, sweep over the operating band");
        s.standard = stdRef;
        s.help = tpl::tr(
                     "Отрезок регулярного волновода с волной H10 на входе и выходе. Это «нулевой» "
                     "образец: на нём проверяют постановку — |S11| должен быть около нуля, а |S21| "
                     "— единица во всей полосе. Фаза S21 показывает длину волны в волноводе: чем "
                     "ближе к отсечке, тем длиннее λв.\n\nЗаполнение диэлектриком опускает отсечку "
                     "в sqrt(eps) раз, и программа сама сдвигает полосу свипа. Проницаемость "
                     "в сечении входа ставится той же — иначе вход окажется рассогласован.",
                     "A section of a regular waveguide with the H10 wave at the input and output. "
                     "This is the \"zero\" sample used to check the setup: |S11| must be close to "
                     "zero and |S21| close to one over the whole band. The phase of S21 shows the "
                     "guide wavelength: the closer to cutoff, the longer lambda_g.\n\nA dielectric "
                     "filling lowers the cutoff by a factor of sqrt(eps), and the program shifts "
                     "the sweep band itself. The permittivity in the port cross-section is set to "
                     "the same value; otherwise the port would be mismatched.")
                 + helpCommon();
        s.params = {stdParam(),
                    Param::number(QStringLiteral("len"), tpl::tr("Длина", "Length"),
                                  tpl::tr("λв", "lambda_g"), 3.0, 0.5, 20.0, 0.5, 1,
                                  tpl::tr("В длинах волны в волноводе на середине полосы",
                                          "In guide wavelengths at the middle of the band")),
                    Param::number(QStringLiteral("eps"), tpl::tr("Заполнение, eps", "Filling, eps"), {},
                                  1.0, 1.0, 12.0, 0.1, 2,
                                  tpl::tr("1 — пустой волновод", "1 = empty waveguide")),
                    pointsParam()};
        s.build = buildSection;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("wg_iris");
        s.section = sec;
        s.title = tpl::tr("Индуктивная диафрагма", "Inductive iris");
        s.summary = tpl::tr("Окно в поперечной стенке — шунтирующая индуктивность",
                            "A window in a transverse wall: a shunt inductance");
        s.standard = stdRef;
        s.help = tpl::tr(
                     "Тонкая металлическая стенка поперёк волновода с окном по ширине. Для волны "
                     "H10 это шунтирующая индуктивность: чем уже окно, тем сильнее отражение. "
                     "Диафрагмы — основной элемент волноводных фильтров и согласующих устройств.\n\n"
                     "В паспорте — нормированная реактивная проводимость B/Y0 по формуле "
                     "Маркувица для бесконечно тонкой диафрагмы. Ядро считает диафрагму реальной "
                     "толщины, поэтому отражение выйдет несколько сильнее.",
                     "A thin metal wall across the waveguide with a window along the width. For the "
                     "H10 wave it is a shunt inductance: the narrower the window, the stronger the "
                     "reflection. Irises are the main element of waveguide filters and matching "
                     "devices.\n\nThe datasheet gives the normalized susceptance B/Y0 by the "
                     "Marcuvitz formula for an infinitely thin iris. The kernel computes an iris of "
                     "real thickness, so the reflection will be somewhat stronger.")
                 + helpCommon();
        s.params = {stdParam(),
                    Param::number(QStringLiteral("d"), tpl::tr("Окно, доля a",
                                                               "Window, fraction of a"), {}, 0.5,
                                  0.2, 0.95, 0.05, 2),
                    Param::flag(QStringLiteral("sym"), tpl::tr("Симметричная", "Symmetric"), true),
                    Param::number(QStringLiteral("t"), tpl::tr("Толщина, доля a",
                                                               "Thickness, fraction of a"), {}, 0.05,
                                  0.05, 0.5, 0.01, 2),
                    pointsParam()};
        s.build = buildIris;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("wg_post");
        s.section = sec;
        s.title = tpl::tr("Индуктивный штырь", "Inductive post");
        s.summary = tpl::tr("Металлический стержень поперёк волновода", "A metal rod across the waveguide");
        s.standard = stdRef;
        s.help = tpl::tr(
                     "Металлический штырь, перекрывающий волновод по высоте. В H-плоскости — "
                     "круг. Действует как шунтирующая индуктивность; смещение от оси ослабляет "
                     "связь с волной H10. Штыри применяют в фильтрах и для настройки.\n\n"
                     "Теория — формула Маркувица для тонкого центрального штыря.",
                     "A metal post spanning the full height of the waveguide. In the H-plane it is "
                     "a circle. It acts as a shunt inductance; an offset from the axis weakens the "
                     "coupling to the H10 wave. Posts are used in filters and for "
                     "tuning.\n\nTheory: the Marcuvitz formula for a thin post on the axis.")
                 + helpCommon();
        s.params = {stdParam(),
                    Param::number(QStringLiteral("dia"), tpl::tr("Диаметр, доля a",
                                                                 "Diameter, fraction of a"), {}, 0.1,
                                  0.04, 0.4, 0.01, 2),
                    Param::number(QStringLiteral("off"), tpl::tr("Смещение от оси, доля a",
                                                                 "Offset from the axis, fraction of a"),
                                  {}, 0.0, -0.3, 0.3, 0.05, 2),
                    pointsParam()};
        s.build = buildPost;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("wg_step");
        s.section = sec;
        s.title = tpl::tr("Ступенька по ширине", "Step in width");
        s.summary = tpl::tr("Стык двух волноводов разной ширины",
                            "Junction of two waveguides of different width");
        s.standard = stdRef;
        s.help = tpl::tr(
                     "Симметричный стык двух волноводов разной ширины. Отсечки у половин разные, "
                     "поэтому свип идёт только там, где обе половины одномодовые: программа "
                     "вычисляет эту полосу сама. Оба входа возбуждаются по очереди — получается "
                     "полная матрица 2x2.",
                     "A symmetric junction of two waveguides of different width. The halves have "
                     "different cutoffs, so the sweep covers only the band where both halves are "
                     "single-mode; the program computes this band itself. Both ports are excited in "
                     "turn, giving the full 2x2 matrix.")
                 + helpCommon();
        s.params = {stdParam(),
                    Param::number(QStringLiteral("ratio"), tpl::tr("Отношение a2/a1", "Ratio a2/a1"), {},
                                  0.85, 0.7, 1.4, 0.05, 2)};
        s.build = buildStep;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("wg_bend");
        s.section = sec;
        s.title = tpl::tr("H-изгиб 90°", "H-bend 90 deg.");
        s.summary = tpl::tr("Поворот волновода со срезанным наружным углом",
                            "Waveguide bend with a mitred outer corner");
        s.standard = stdRef;
        s.help = tpl::tr(
                     "Поворот волновода на 90 градусов в H-плоскости. Прямой угол заметно отражает; "
                     "срез наружного угла уменьшает отражение. Оптимальный срез подбирают — "
                     "меняйте долю среза и сравнивайте |S11|.\n\nВторой вход стоит на верхней "
                     "границе области: вход вдоль оси X (INPUT_Y) программа выбирает сама.",
                     "A 90-degree bend of the waveguide in the H-plane. A square corner reflects "
                     "noticeably; mitring the outer corner reduces the reflection. The optimal "
                     "mitre is found by tuning: change the mitre fraction and compare |S11|.\n\nThe "
                     "second port is on the top boundary of the domain: the program chooses the "
                     "port along the X axis (INPUT_Y) itself.")
                 + helpCommon();
        s.params = {stdParam(),
                    Param::number(QStringLiteral("cut"), tpl::tr("Срез угла, доля a",
                                                                 "Corner mitre, fraction of a"), {},
                                  0.6, 0.0, 0.95, 0.05, 2),
                    pointsParam()};
        s.build = buildBend;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("wg_tee");
        s.section = sec;
        s.title = tpl::tr("H-плоскостной тройник", "H-plane tee");
        s.summary = tpl::tr("Разветвление на три плеча, матрица 3x3", "Three-arm junction, 3x3 matrix");
        s.standard = stdRef;
        s.help = tpl::tr(
                     "Ответвление в H-плоскости. Все три входа возбуждаются по очереди в ОДНОМ "
                     "задании — ядро заполняет полную матрицу рассеяния 3x3. Без согласующих "
                     "элементов тройник отражает: три плеча без потерь одновременно согласовать "
                     "нельзя.",
                     "A branch in the H-plane. All three ports are excited in turn in ONE task, and "
                     "the kernel fills the full 3x3 scattering matrix. Without matching elements "
                     "the tee reflects: three lossless arms cannot be matched simultaneously.")
                 + helpCommon();
        s.params = {stdParam(), pointsParam(11)};
        s.build = buildTee;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("wg_filter");
        s.section = sec;
        s.title = tpl::tr("Фильтр на индуктивных диафрагмах", "Inductive iris filter");
        s.summary = tpl::tr("Полосовой фильтр Чебышёва, расчёт по Маттею",
                            "Chebyshev bandpass filter, Matthaei design");
        s.standard = tpl::tr("Маттей, Янг, Джонс; ", "Matthaei, Young, Jones; ") + stdRef;
        s.help = tpl::tr(
                     "Полосовой фильтр из цепочки полуволновых резонаторов, связанных "
                     "индуктивными диафрагмами. Размеры считаются по классической методике "
                     "Маттея: прототип Чебышёва -> инверторы сопротивления -> реактивности "
                     "диафрагм -> ширины окон и длины резонаторов. Весь расчёт — в паспорте.\n\n"
                     "Методика предполагает бесконечно тонкие диафрагмы; ядро считает реальные, "
                     "поэтому полоса немного сдвинется — ровно так выглядит настройка реального "
                     "фильтра.",
                     "A bandpass filter made of a chain of half-wave resonators coupled by "
                     "inductive irises. The dimensions are computed by the classical Matthaei "
                     "method: Chebyshev prototype -> impedance inverters -> iris reactances -> "
                     "window widths and resonator lengths. The whole calculation is in the "
                     "datasheet.\n\nThe method assumes infinitely thin irises; the kernel computes "
                     "real ones, so the band will shift slightly. This is exactly what tuning a "
                     "real filter looks like.")
                 + helpCommon();
        s.params = {stdParam(),
                    Param::number(QStringLiteral("f0"), tpl::tr("Центральная частота", "Center frequency"),
                                  tpl::tr("ГГц", "GHz"), 10.0, 0.5, 110.0, 0.1, 3),
                    Param::number(QStringLiteral("fbw"), tpl::tr("Полоса пропускания", "Passband"),
                                  QStringLiteral("%"), 5.0, 1.0, 15.0, 0.5, 1),
                    Param::integer(QStringLiteral("order"), tpl::tr("Порядок", "Order"), 3, 1, 7),
                    Param::number(QStringLiteral("ripple"), tpl::tr("Пульсации", "Ripple"),
                                  tpl::tr("дБ", "dB"), 0.1, 0.01, 1.0, 0.05, 2),
                    pointsParam(31)};
        s.build = buildFilter;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("wg_window");
        s.section = sec;
        s.title = tpl::tr("Диэлектрическое окно λ/2", "Dielectric window lambda/2");
        s.summary = tpl::tr("Полуволновая пластина: точная формула для проверки ядра",
                            "Half-wave plate: an exact formula to check the kernel");
        s.standard = stdRef;
        s.help = tpl::tr(
                     "Диэлектрическая пластина поперёк волновода — так делают герметичные окна. "
                     "При толщине в половину длины волны (в диэлектрике, с учётом волновода) "
                     "отражения от двух граней гасят друг друга, и окно прозрачно на центральной "
                     "частоте.\n\nФормула однородного слоя точная, поэтому паспорт даёт |S11| на "
                     "трёх частотах — сравните с тем, что посчитает ядро.",
                     "A dielectric plate across the waveguide: this is how hermetic windows are "
                     "made. When its thickness is half a wavelength (in the dielectric, taking the "
                     "waveguide into account), reflections from the two faces cancel, and the "
                     "window is transparent at the center frequency.\n\nThe formula for a "
                     "homogeneous layer is exact, so the datasheet gives |S11| at three "
                     "frequencies: compare them with what the kernel computes.")
                 + helpCommon();
        s.params = {stdParam(),
                    Param::number(QStringLiteral("eps"), tpl::tr("Проницаемость eps", "Permittivity eps"), {},
                                  2.1, 1.1, 20.0, 0.1, 2,
                                  tpl::tr("2.1 — фторопласт, 4 — кварц, 9.8 — поликор",
                                          "2.1 = PTFE, 4 = quartz, 9.8 = polycor")),
                    Param::number(QStringLiteral("thick"), tpl::tr("Толщина, доля λв/2",
                                                                   "Thickness, fraction of lambda_g/2"), {},
                                  1.0, 0.1, 3.0, 0.1, 2),
                    pointsParam()};
        s.build = buildWindow;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("wg_load");
        s.section = sec;
        s.title = tpl::tr("Нагрузка волновода", "Waveguide termination");
        s.summary = tpl::tr("Короткое замыкание, плоский поглотитель и клин — три варианта",
                            "Short circuit, flat absorber and wedge: three variants");
        s.standard = stdRef;
        s.help = tpl::tr(
                     "Чем закончить волновод. Задание считает три варианта сразу: короткое "
                     "замыкание (полное отражение, |S11| = 1 — проверка постановки), плоский "
                     "поглотитель в торце и клин поглотителя перед короткозамыкателем.\n\n"
                     "Поглотитель ядра согласован для нормального падения. Волна H10 — это две "
                     "плоские волны, бегущие под углом к оси, поэтому плоский поглотитель "
                     "частично отражает: |S11| = (1 - λ/λв)/(1 + λ/λв), и тем сильнее, чем "
                     "ближе частота к отсечке. Эту формулу ядро и проверяет.",
                     "How to terminate a waveguide. The task computes three variants at once: a "
                     "short circuit (total reflection, |S11| = 1, a check of the setup), a flat "
                     "absorber at the end and an absorber wedge in front of the short.\n\nThe "
                     "kernel's absorber is matched for normal incidence. The H10 wave is two plane "
                     "waves travelling at an angle to the axis, so a flat absorber partially "
                     "reflects: |S11| = (1 - lambda/lambda_g)/(1 + lambda/lambda_g), the more "
                     "strongly the closer the frequency is to cutoff. This is the formula the "
                     "kernel checks.")
                 + helpCommon();
        s.params = {stdParam(),
                    Param::number(QStringLiteral("wedge"), tpl::tr("Длина клина", "Wedge length"),
                                  tpl::tr("λв", "lambda_g"), 3.0, 0.3, 8.0, 0.1, 1),
                    pointsParam(11)};
        s.build = buildLoad;
        *out << s;
    }
}

} // namespace tpl::samples
