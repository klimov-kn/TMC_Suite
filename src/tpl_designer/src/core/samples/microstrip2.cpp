// Раздел «Микрополосковые», вторая половина: узлы, нагрузки и импульсные задачи.
//
// Здесь собрано то, чего разделу не хватало: задания с ПОГЛОТИТЕЛЕМ (согласованная
// нагрузка, рамка вокруг патча), задания, где магнитная стенка работает не стенкой
// канала, а элементом схемы (разомкнутый конец, патч-резонатор), и два задания с
// падающей волной ИЗ ФАЙЛА — импульсная рефлектометрия и линия задержки, где смысл
// целиком во временной картине, а не в частотной точке.
//
// Общие детали планарной модели — в `msplanar.h`. Там же сказано, чего этой моделью
// выразить нельзя: связанных линий, потому что магнитные стенки не пропускают
// краевое поле.

/// @file microstrip2.cpp
/// @brief Раздел каталога «Микрополосковые», вторая половина: изгиб, делитель, нагрузка, разомкнутый конец, рефлектометрия, задержка, тейпер, патч, ПЗФ.
///
/// Часть модуля «Ядро TPL Designer», каталог образцов. Регистрирует рецепты
/// `addMicrostrip2` (вызывается из `addMicrostrip`). Два рецепта используют
/// падающую волну из файла `.sig`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#samples
#include "msplanar.h"

#include "lang.h"
#include "sigfile.h"

#include <algorithm>
#include <cmath>

namespace tpl::samples {

namespace {

using namespace msp;

// ------------------------------------------------------------ изгиб 90 градусов

/// @brief Рецепт «Изгиб 90° со скосом» (`ms_bend`).
///
/// Поворот линии: прямой угол против скошенного.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `sub`, `h` — подложка и толщина, мм (см. `msp::boardParams`);
///                `z0` — сопротивление линии, Ом;
///                `miter` — скос, % ширины;
///                `f0` — рабочая частота, ГГц
/// @return Задание, паспорт «теория» и пояснения.
Build buildBend(const Values &v)
{
    const Board bd = board(v);
    const double f0 = v.value(QStringLiteral("f0"));
    const Line l = bd.line(v.value(QStringLiteral("z0")));
    const double miter = v.value(QStringLiteral("miter")) / 100.0;

    Builder b(QStringLiteral("msbend"));
    b.p.delta = deltaFitting(l.weff, gridFor(l, f0));
    const double d = b.p.delta;
    const double w = l.weff;
    const double feed = snap(l.lambdaG(f0), d);

    // Канал приходит слева вдоль X и уходит вверх вдоль Y. Внешний угол — (xr, yb),
    // внутренний — (xl, yb+w): скос режет именно внешний.
    const double yb = 3 * d;
    const double xl = feed;
    const double xr = xl + w;
    const double yTop = yb + w + feed;
    const double cut = snap(miter * w, d);
    b.p.box = {0.0, xr + 3 * d, 0.0, yTop};
    b.p.description = tpl::tr("Изгиб микрополосковой линии %1 Ом на 90 градусов, скос "
                              "%2 процентов ширины, %3.",
                              "Microstrip line bend %1 Ohm by 90 degrees, mitre %2 percent of "
                              "width, %3.")
                          .arg(num(l.z0), num(miter * 100.0, 3), bd.sub.name);

    QVector<QPointF> outer{QPointF(0, yb)};
    if (cut > 0.0)
        outer << QPointF(xr - cut, yb) << QPointF(xr, yb + cut);
    else
        outer << QPointF(xr, yb);
    outer << QPointF(xr, yTop);
    b.polyline(tpl::tr("внешняя стенка со скосом", "outer wall with mitre"), Material::Magnetic, outer);
    b.polyline(tpl::tr("внутренняя стенка", "inner wall"), Material::Magnetic,
               {QPointF(0, yb + w), QPointF(xl, yb + w), QPointF(xl, yTop)});

    fill(b, tpl::tr("плечо по X", "arm along X"), l, 0.0, xr, yb, yb + w);
    fill(b, tpl::tr("плечо по Y", "arm along Y"), l, xl, xr, yb + w, yTop);

    msPort(b, tpl::tr("вход 1", "port 1"), PortSide::Left, yb, yb + w, true, l);
    msPort(b, tpl::tr("вход 2", "port 2"), PortSide::Top, xl, xr, false, l);
    b.freqs(std::round(f0 * 0.6 * 100) / 100, std::round(f0 * 1.6 * 100) / 100, 21);
    b.outputs(true);
    b.time(settleTime((xr + yTop) * std::sqrt(l.epsEff), f0, 0.0));

    QVector<Row> rows = boardRows(bd);
    rows << lineRow(tpl::tr("Линия %1 Ом", "Line %1 Ohm").arg(num(l.z0)), l, f0);
    rows << Row{tpl::tr("Скос", "Mitre"),
                cut > 0.0 ? tpl::tr("%1 % ширины канала, то есть %2", "%1 % of the channel width, i.e. %2")
                                .arg(num(miter * 100.0, 3), mm(cut))
                          : tpl::tr("нет — прямой угол", "none, square corner")};
    rows << Row{tpl::tr("Ожидаемо", "Expected"),
                tpl::tr("|S21| около 0 дБ; без скоса |S11| заметно хуже — лишняя медь "
                        "в углу добавляет ёмкость",
                        "|S21| about 0 dB; without the mitre |S11| is noticeably worse: the extra "
                        "copper in the corner adds capacitance")};
    return b.done(rows, {tpl::tr("Выход стоит на ВЕРХНЕЙ границе области: в языке это "
                                 "INPUT_Y, и апертура у него задаётся по X, а не по Y.",
                                 "The output is on the TOP boundary of the domain: in the language "
                                 "this is INPUT_Y, and its aperture is given along X, not Y."),
                         tpl::tr("Диэлектрик залит двумя прямоугольниками, поэтому за "
                                 "скосом остаётся треугольник заполнения. Полю он не "
                                 "достаётся: магнитная стенка проходит по скосу.",
                                 "The dielectric is filled with two rectangles, so a triangle of "
                                 "filling remains behind the mitre. The field does not reach it: "
                                 "the magnetic wall runs along the mitre.")});
}

// ------------------------------------------------------------ Т-образный делитель

/// @brief Рецепт «Т-образный делитель мощности» (`ms_tee`).
///
/// Деление пополам на два плеча двойного импеданса.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `sub`, `h` — подложка и толщина, мм (см. `msp::boardParams`);
///                `z0` — сопротивление входа, Ом;
///                `f0` — рабочая частота, ГГц
/// @return Задание, паспорт «теория» и пояснения.
Build buildTee(const Values &v)
{
    const Board bd = board(v);
    const double f0 = v.value(QStringLiteral("f0"));
    const double z0 = v.value(QStringLiteral("z0"));
    const Line lin = bd.line(z0);
    const Line arm = bd.line(std::min(120.0, 2.0 * z0)); // два плеча в параллель дают Z0

    Builder b(QStringLiteral("mstee"));
    b.p.delta = deltaFitting(arm.weff, gridFor(arm, f0));
    const double d = b.p.delta;
    const double w0 = lin.weff, w1 = arm.weff;
    const double yb = 3 * d;
    const double xj = snap(lin.lambdaG(f0), d);
    const double X = xj + snap(arm.lambdaG(f0), d);
    const double Y = yb + w0 + snap(arm.lambdaG(f0), d);
    b.p.box = {0.0, X, 0.0, Y};
    b.p.description = tpl::tr("Т-образный делитель мощности: %1 Ом -> два плеча по %2 Ом, "
                              "%3 ГГц, %4.",
                              "T-junction power divider: %1 Ohm -> two arms of %2 Ohm, %3 GHz, %4.")
                          .arg(num(z0), num(arm.z0), num(f0, 3), bd.sub.name);

    // Низ идёт насквозь: у входного канала и у правого плеча он общий.
    b.polyline(tpl::tr("стенка снизу", "bottom wall"), Material::Magnetic,
               seg(QPointF(0, yb), QPointF(X, yb)));
    // Верх входа переходит в левую стенку верхнего плеча.
    b.polyline(tpl::tr("стенка сверху и слева от плеча", "wall top and left of the arm"), Material::Magnetic,
               {QPointF(0, yb + w0), QPointF(xj - w1, yb + w0), QPointF(xj - w1, Y)});
    // Правая стенка верхнего плеча переходит в верх правого плеча.
    b.polyline(tpl::tr("стенка справа от плеча и сверху",
                       "wall right of the arm and top"), Material::Magnetic,
               {QPointF(xj, Y), QPointF(xj, yb + w1), QPointF(X, yb + w1)});

    fill(b, tpl::tr("вход %1 Ом", "input %1 Ohm").arg(num(z0)), lin, 0.0, xj, yb, yb + w0);
    fill(b, tpl::tr("плечо вверх", "arm up"), arm, xj - w1, xj, yb + w0, Y);
    fill(b, tpl::tr("плечо вправо", "arm right"), arm, xj, X, yb, yb + w1);

    msPort(b, tpl::tr("вход 1", "port 1"), PortSide::Left, yb, yb + w0, true, lin);
    msPort(b, tpl::tr("вход 2 (вправо)", "port 2 (right)"), PortSide::Right, yb, yb + w1, false, arm);
    msPort(b, tpl::tr("вход 3 (вверх)", "port 3 (up)"), PortSide::Top, xj - w1, xj, false, arm);
    b.freqs(std::round(f0 * 0.6 * 100) / 100, std::round(f0 * 1.4 * 100) / 100, 21);
    b.outputs(true);
    b.time(settleTime((X + Y) * std::sqrt(arm.epsEff), f0, 0.0, 5.0));

    QVector<Row> rows = boardRows(bd);
    rows << lineRow(tpl::tr("Вход %1 Ом", "Input %1 Ohm").arg(num(z0)), lin, f0);
    rows << lineRow(tpl::tr("Плечи %1 Ом", "Arms %1 Ohm").arg(num(arm.z0)), arm, f0);
    rows << Row{tpl::tr("Согласование на постоянном токе", "DC matching"),
                tpl::tr("два плеча по %1 Ом в параллель = %2 Ом (вход %3 Ом)",
                        "two arms of %1 Ohm in parallel = %2 Ohm (input %3 Ohm)")
                    .arg(num(arm.z0), num(arm.z0 / 2.0), num(z0))};
    rows << Row{tpl::tr("Ожидаемо", "Expected"),
                tpl::tr("|S21| = |S31| = -3 дБ, |S11| мал на f0",
                        "|S21| = |S31| = -3 dB, |S11| small at f0")};
    return b.done(rows, {tpl::tr("Развязки между плечами у такого делителя НЕТ — это не "
                                 "Вилкинсон: поглощающий резистор между плечами в "
                                 "планарной модели выразить нечем.",
                                 "This divider has NO isolation between the arms; it is not a "
                                 "Wilkinson: the absorbing resistor between the arms cannot be "
                                 "expressed in the planar model."),
                         tpl::tr("Реактивность самого соединения учебниковой формулой не "
                                 "описывается — на неё и смотрят по |S11| ядра.",
                                 "The reactance of the junction itself is not described by a "
                                 "textbook formula; it is what the kernel's |S11| shows.")});
}

// ------------------------------------------------------------ нагрузка на поглотителе

/// @brief Рецепт «Поглотитель в канале: сколько отражает» (`ms_load`).
///
/// Измеряет отражение контура ABSORBER в планарном канале.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `sub`, `h` — подложка и толщина, мм (см. `msp::boardParams`);
///                `z0` — сопротивление линии, Ом;
///                `cells` — число слоёв поглотителя;
///                `f0` — рабочая частота, ГГц
/// @return Задание, паспорт «теория» и пояснения.
/// @see https://www.tamic.ru/docs/theory/boundaries.html#absorber
Build buildMsLoad(const Values &v)
{
    const Board bd = board(v);
    const double f0 = v.value(QStringLiteral("f0"));
    const Line l = bd.line(v.value(QStringLiteral("z0")));
    const int cells = int(v.value(QStringLiteral("cells")));

    Builder b(QStringLiteral("msload"));
    b.p.delta = deltaFitting(l.weff, gridFor(l, f0));
    const double d = b.p.delta;
    const double w = l.weff;
    const double yb = 3 * d, yh = yb + w;
    const double X = snap(2.0 * l.lambdaG(f0), d);
    b.p.box = {0.0, X, 0.0, yh + 3 * d};
    b.p.description = tpl::tr("Нагрузка линии %1 Ом на поглотителе: %2 слоёв поперёк "
                              "канала у края области, %3.",
                              "Load of a %1 Ohm line on an absorber: %2 layers across the channel "
                              "at the domain edge, %3.")
                          .arg(num(l.z0)).arg(cells).arg(bd.sub.name);

    b.polyline(tpl::tr("край полоски, низ", "strip edge, bottom"), Material::Magnetic,
               seg(QPointF(0, yb), QPointF(X, yb)));
    b.polyline(tpl::tr("край полоски, верх", "strip edge, top"), Material::Magnetic,
               seg(QPointF(0, yh), QPointF(X, yh)));
    fill(b, tpl::tr("линия", "line"), l, 0.0, X, yb, yh);

    // 🔑 Поглотитель стоит у САМОГО КРАЯ области и растёт внутрь — так же, как рамка
    // поглотителя в авторских заданиях.
    //
    // ⚠️ ABSORBER в языке — КОНТУР, а не заливка: поглощающего объёма не существует.
    // Проверено прогонами (2026-10-01), и обе проверки важны:
    //   * поглотитель посередине области, с каналом за ним, даёт то же самое — часть
    //     волны проходит, отражается от границы коробки (края НЕ поглощают) и
    //     возвращается;
    //   * число слоёв на отражение НЕ влияет: один слой и пять слоёв дали одно и то
    //     же. Стопка контуров не становится толстым поглотителем.
    // Поэтому параметр «слоёв» здесь — повод убедиться в этом самому, а не способ
    // улучшить согласование.
    for (int i = 0; i < cells; ++i) {
        const double x = X - i * d;
        b.polyline(tpl::tr("поглотитель, слой %1", "absorber, layer %1").arg(i + 1), Material::Absorber,
                   seg(QPointF(x, yb), QPointF(x, yh)));
    }

    msPort(b, tpl::tr("вход 1", "port 1"), PortSide::Left, yb, yh, true, l);
    b.freqs(std::round(f0 * 0.5 * 100) / 100, std::round(f0 * 1.5 * 100) / 100, 21);
    b.outputs(true);
    b.time(settleTime(2.0 * X * std::sqrt(l.epsEff), f0, 0.0));

    QVector<Row> rows = boardRows(bd);
    rows << lineRow(tpl::tr("Линия %1 Ом", "Line %1 Ohm").arg(num(l.z0)), l, f0);
    rows << Row{tpl::tr("Поглотитель", "Absorber"),
                tpl::tr("%1 слоёв по %2 у края области (x от %3 до %4)",
                        "%1 layers of %2 at the domain edge (x from %3 to %4)")
                    .arg(cells).arg(mm(d), mm(X - (cells - 1) * d), mm(X))};
    rows << Row{tpl::tr("Измерено ядром (умолчания)", "Measured by the kernel (defaults)"),
                tpl::tr("|S11| от -2 до -7 дБ по полосе — столько отражает ABSORBER "
                        "в планарном канале",
                        "|S11| from -2 to -7 dB over the band: this is how much ABSORBER reflects "
                        "in a planar channel")};
    rows << Row{tpl::tr("Для сравнения", "For comparison"),
                tpl::tr("тот же плоский поглотитель в МЕТАЛЛИЧЕСКОМ волноводе (образец "
                        "«Нагрузка волновода») даёт -12...-22 дБ",
                        "the same flat absorber in a METAL waveguide (sample \"Waveguide "
                        "termination\") gives -12...-22 dB")};
    rows << Row{tpl::tr("Разомкнутый конец", "Open end"),
                tpl::tr("около 0 дБ — отражается всё (образец «Разомкнутый конец линии»)",
                        "about 0 dB, everything is reflected (sample \"Open end of a line\")")};
    return b.done(rows, {tpl::tr("Порт ОДИН: матрица вырождается в одно число S11, и "
                                 "именно оно здесь измеряется.",
                                 "ONE port: the matrix degenerates into a single number S11, and "
                                 "that is what is measured here."),
                         tpl::tr("Задание не рекламирует поглотитель, а ИЗМЕРЯЕТ его. "
                                 "Вывод из прогонов: в канале с магнитными стенками "
                                 "ABSORBER отражает заметно больше, чем в металлическом "
                                 "волноводе. Это свойство граничного условия ядра, и его "
                                 "надо знать всем, кто ставит рамку поглотителя вокруг "
                                 "планарной структуры.",
                                 "The task does not advertise the absorber, it MEASURES it. "
                                 "Conclusion from the runs: in a channel with magnetic walls "
                                 "ABSORBER reflects noticeably more than in a metal waveguide. This "
                                 "is a property of the kernel's boundary condition, and anyone who "
                                 "puts an absorber frame around a planar structure should know it."),
                         tpl::tr("Число слоёв проверено: один и пять дают одно и то же. "
                                 "ABSORBER — это условие на контуре, а не толщина "
                                 "поглощающей среды, и стопка контуров толстым "
                                 "поглотителем не становится.",
                                 "The number of layers has been checked: one and five give the same "
                                 "result. ABSORBER is a condition on a contour, not the thickness "
                                 "of an absorbing medium, and a stack of contours does not become a "
                                 "thick absorber.")});
}

// ------------------------------------------------------------ разомкнутый конец

/// @brief Рецепт «Разомкнутый конец линии» (`ms_openend`).
///
/// Магнитная стенка как холостой ход; эталон для входа.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `sub`, `h` — подложка и толщина, мм (см. `msp::boardParams`);
///                `z0` — сопротивление линии, Ом;
///                `len` — длина, в длинах волны;
///                `f0` — рабочая частота, ГГц
/// @return Задание, паспорт «теория» и пояснения.
/// @see https://www.tamic.ru/docs/theory/boundaries.html#open
Build buildOpenEnd(const Values &v)
{
    const Board bd = board(v);
    const double f0 = v.value(QStringLiteral("f0"));
    const Line l = bd.line(v.value(QStringLiteral("z0")));

    Builder b(QStringLiteral("msopen"));
    b.p.delta = deltaFitting(l.weff, gridFor(l, f0));
    const double d = b.p.delta;
    const double w = l.weff;
    const double yb = 3 * d, yh = yb + w;
    const double L = snap(v.value(QStringLiteral("len")) * l.lambdaG(f0), d);
    b.p.box = {0.0, L + 3 * d, 0.0, yh + 3 * d};
    b.p.description = tpl::tr("Разомкнутый конец линии %1 Ом: магнитная стенка поперёк "
                              "канала, %2.",
                              "Open end of a %1 Ohm line: a magnetic wall across the channel, %2.")
                          .arg(num(l.z0), bd.sub.name);

    b.polyline(tpl::tr("край полоски, низ", "strip edge, bottom"), Material::Magnetic,
               seg(QPointF(0, yb), QPointF(L, yb)));
    b.polyline(tpl::tr("край полоски, верх", "strip edge, top"), Material::Magnetic,
               seg(QPointF(0, yh), QPointF(L, yh)));
    b.polyline(tpl::tr("разомкнутый конец", "open end"), Material::Magnetic,
               seg(QPointF(L, yb), QPointF(L, yh)));
    fill(b, tpl::tr("линия", "line"), l, 0.0, L, yb, yh);
    msPort(b, tpl::tr("вход 1", "port 1"), PortSide::Left, yb, yh, true, l);
    b.freqs(std::round(f0 * 0.5 * 100) / 100, std::round(f0 * 1.5 * 100) / 100, 21);
    b.outputs(true);
    b.time(settleTime(2.0 * L * std::sqrt(l.epsEff), f0, 0.0));

    // Удлинение разомкнутого конца по Хаммерстаду: насколько линия «кажется» длиннее
    // из-за краевого поля. Планарная модель этого НЕ содержит, и в том весь смысл.
    const double u = l.w / bd.h;
    const double dl = 0.412 * bd.h * (l.epsEff + 0.3) / (l.epsEff - 0.258) * (u + 0.262)
                      / (u + 0.813);

    QVector<Row> rows = boardRows(bd);
    rows << lineRow(tpl::tr("Линия %1 Ом", "Line %1 Ohm").arg(num(l.z0)), l, f0);
    rows << Row{tpl::tr("Длина до конца", "Length to the end"),
                tpl::tr("%1 (%2 длины волны в линии)",
                        "%1 (%2 wavelengths in the line)").arg(mm(L), num(L / l.lambdaG(f0)))};
    rows << Row{tpl::tr("Фаза S11 по теории линий", "S11 phase by transmission-line theory"),
                tpl::tr("%1 град. на f0 (идеальный холостой ход)", "%1 deg. at f0 (ideal open circuit)")
                    .arg(num(std::fmod(-720.0 * L / l.lambdaG(f0), 360.0), 4))};
    rows << Row{tpl::tr("Удлинение края (Хаммерстад)", "Edge extension (Hammerstad)"),
                tpl::tr("%1 — столько НЕ учитывает планарная модель",
                        "%1: this is what the planar model does NOT take into account").arg(mm(dl))};
    rows << Row{tpl::tr("Ожидаемо", "Expected"),
                tpl::tr("|S11| около 0 дБ на всех частотах: магнитная стенка отражает "
                        "полностью, излучения с края в двумерной модели нет",
                        "|S11| about 0 dB at all frequencies: the magnetic wall reflects "
                        "completely, there is no radiation from the edge in the 2D model")};
    return b.done(rows, {tpl::tr("Это образец-ЭТАЛОН. Отражать здесь должно ровно "
                                 "единицу, поэтому всё, что в |S11| не дотягивает до "
                                 "нуля дБ, — остаток самого входа плоской волны.",
                                 "This is a REFERENCE sample. The reflection here must be exactly "
                                 "unity, so whatever |S11| falls short of zero dB is the residual "
                                 "of the plane-wave port itself."),
                         tpl::tr("Настоящий разомкнутый конец излучает и «удлиняется» на "
                                 "величину из паспорта. Сравнивая фазу с теорией линий, "
                                 "эту поправку надо добавлять руками.",
                                 "A real open end radiates and \"lengthens\" by the amount given in "
                                 "the datasheet. When comparing the phase with transmission-line "
                                 "theory, this correction has to be added by hand.")});
}

// ------------------------------------------------------------ импульсная рефлектометрия

/// @brief Рецепт «Импульсная рефлектометрия ступеньки» (`ms_tdr`).
///
/// Моноцикл из файла `.sig`: отражение видно во времени.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `sub`, `h` — подложка и толщина, мм (см. `msp::boardParams`);
///                `z1`, `z2` — сопротивления участков, Ом;
///                `len1`, `len2` — длины участков, в длинах волны;
///                `f0` — частота, ГГц
/// @return Задание, паспорт «теория» и пояснения.
/// @see https://www.tamic.ru/docs/theory/ports.html#incident-file
Build buildTdr(const Values &v)
{
    const Board bd = board(v);
    const double f0 = v.value(QStringLiteral("f0"));
    const double z1 = v.value(QStringLiteral("z1")), z2 = v.value(QStringLiteral("z2"));
    const Line l1 = bd.line(z1), l2 = bd.line(z2);

    Builder b(QStringLiteral("mstdr"));
    const Line &narrow = (l1.weff < l2.weff) ? l1 : l2;
    b.p.delta = gridFor(narrow, f0);
    const double d = b.p.delta;
    const double wMax = std::max(l1.weff, l2.weff);
    const double yc = snap(3 * d + wMax / 2.0, d);
    const double L1 = snap(v.value(QStringLiteral("len1")) * l1.lambdaG(f0), d);
    const double L2 = snap(v.value(QStringLiteral("len2")) * l2.lambdaG(f0), d);
    const double X = L1 + L2;
    b.p.box = {0.0, X, 0.0, 2 * yc};
    b.p.description = tpl::tr("Импульсная рефлектометрия ступеньки импеданса %1 -> %2 Ом: "
                              "падающая волна из файла, %3.",
                              "Pulse reflectometry of an impedance step %1 -> %2 Ohm: incident wave "
                              "from a file, %3.")
                          .arg(num(z1), num(z2), bd.sub.name);

    const auto edge = [&](double s) {
        return QVector<QPointF>{QPointF(0, yc + s * l1.weff / 2), QPointF(L1, yc + s * l1.weff / 2),
                                QPointF(L1, yc + s * l2.weff / 2), QPointF(X, yc + s * l2.weff / 2)};
    };
    b.polyline(tpl::tr("край полоски, низ", "strip edge, bottom"), Material::Magnetic, edge(-1.0));
    b.polyline(tpl::tr("край полоски, верх", "strip edge, top"), Material::Magnetic, edge(+1.0));
    fill(b, tpl::tr("линия %1 Ом", "line %1 Ohm").arg(num(z1)), l1, 0.0, L1, yc - l1.weff / 2,
         yc + l1.weff / 2);
    fill(b, tpl::tr("линия %1 Ом", "line %1 Ohm").arg(num(z2)), l2, L1, X, yc - l2.weff / 2,
         yc + l2.weff / 2);

    msPort(b, tpl::tr("вход 1", "port 1"), PortSide::Left, yc - l1.weff / 2, yc + l1.weff / 2, true,
           l1);
    msPort(b, tpl::tr("вход 2", "port 2"), PortSide::Right, yc - l2.weff / 2, yc + l2.weff / 2, false,
           l2);
    // ⚠️ Сигнал ставим ПОСЛЕ того, как добавлены ВСЕ порты, и по индексу, а не по
    // ссылке: `Builder::port` дописывает в вектор, вектор переселяется, и ссылка,
    // взятая раньше, указывает в освобождённую память. Поймано тем, что файл .sig
    // не появлялся рядом с заданием вовсе.
    Port &in = b.p.ports[0];

    // 🔑 Падающая волна — МОНОЦИКЛ из файла. Формулой её не задать: ядро умеет только
    // несущую под огибающей, а у моноцикла несущей нет вовсе. Нулевое среднее важно:
    // постоянная составляющая в линии не распространяется и только портила бы картину.
    in.signal.source = SignalSource::File;
    in.signal.wave.kind = Waveform::Monocycle;
    in.signal.wave.amplitude = 1.0;
    in.signal.wave.start = 0.0;
    // Длительность берём от времени пробега первого участка: импульс должен быть
    // КОРОЧЕ пробега, иначе отражение от ступеньки сольётся с падающим.
    const double travel1 = L1 * std::sqrt(l1.epsEff) / 299.792458; // нс
    in.signal.wave.width = std::max(2.0 * d / 299.792458, travel1 / 4.0);

    b.p.freqs = {suggestedFreq(in.signal.wave)};
    // SMATRIX не включаем: встроенный пересчёт одночастотный, а смысл задания — во
    // ВРЕМЕННОЙ картине. Результат смотрят по файлу сигналов .t.
    b.outputs(false);
    b.time(6.0 * travel1 + 4.0 * L2 * std::sqrt(l2.epsEff) / 299.792458 + 1.0);

    const double gamma = (z2 - z1) / (z2 + z1);
    QVector<Row> rows = boardRows(bd);
    rows << lineRow(tpl::tr("Участок 1, %1 Ом", "Section 1, %1 Ohm").arg(num(z1)), l1, f0);
    rows << lineRow(tpl::tr("Участок 2, %1 Ом", "Section 2, %1 Ohm").arg(num(z2)), l2, f0);
    rows << Row{tpl::tr("Отражение от ступеньки", "Reflection from the step"),
                tpl::tr("Г = %1 (%2)", "Gamma = %1 (%2)").arg(num(gamma, 4), db(std::fabs(gamma)))};
    rows << Row{tpl::tr("Время до ступеньки и обратно", "Round-trip time to the step"),
                tpl::tr("%1 нс (участок %2, eps_eff = %3)", "%1 ns (section %2, eps_eff = %3)")
                    .arg(num(2.0 * travel1, 4), mm(L1), num(l1.epsEff))};
    rows << Row{tpl::tr("Длительность импульса", "Pulse duration"),
                tpl::tr("tau = %1 нс, полоса примерно до %2 ГГц", "tau = %1 ns, band up to about %2 GHz")
                    .arg(num(in.signal.wave.width, 4), num(waveBand(in.signal.wave).hi, 4))};
    rows << Row{tpl::tr("Что смотреть", "What to look at"),
                tpl::tr("в файле %1.t: падающий импульс, затем отражённый через %2 нс",
                        "in the file %1.t: the incident pulse, then the reflected one after %2 ns")
                    .arg(b.p.output.signalFile, num(2.0 * travel1, 4))};
    return b.done(rows, {tpl::tr("Падающую волну пишет сам генератор в файл %1.sig рядом "
                                 "с заданием — править его руками не нужно.",
                                 "The generator itself writes the incident wave to the file %1.sig "
                                 "next to the task; there is no need to edit it by hand.")
                             .arg(signalFileName(b.p, b.p.ports.at(0))),
                         tpl::tr("Матрица рассеяния здесь не снимается: встроенный "
                                 "пересчёт SMATRIX рассчитан на одну частоту, а импульс "
                                 "широкополосный. Полосу по этому же файлу .t строит "
                                 "постобработка.",
                                 "The scattering matrix is not taken here: the built-in SMATRIX "
                                 "conversion is designed for a single frequency, while the pulse is "
                                 "broadband. Post-processing builds the band from the same .t file."),
                         tpl::tr("Знак отражения виден прямо на графике: при переходе на "
                                 "БОЛЬШЕЕ сопротивление отражённый импульс той же "
                                 "полярности, на меньшее — обратной.",
                                 "The sign of the reflection is seen directly on the plot: for a "
                                 "step to a HIGHER impedance the reflected pulse has the same "
                                 "polarity, to a lower one the opposite polarity.")});
}

// ------------------------------------------------------------ линия задержки

/// @brief Рецепт «Линия задержки «змейкой»» (`ms_delay`).
///
/// Видеоимпульс из файла `.sig` приходит позже: задержка во времени.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `sub`, `h` — подложка и толщина, мм (см. `msp::boardParams`);
///                `z0` — сопротивление линии, Ом;
///                `runs` — число прогонов змейки;
///                `run` — длина прогона, в длинах волны;
///                `f0` — частота, ГГц
/// @return Задание, паспорт «теория» и пояснения.
/// @see https://www.tamic.ru/docs/theory/ports.html#incident-file
Build buildDelay(const Values &v)
{
    const Board bd = board(v);
    const double f0 = v.value(QStringLiteral("f0"));
    const Line l = bd.line(v.value(QStringLiteral("z0")));
    const int n = int(v.value(QStringLiteral("runs")));

    Builder b(QStringLiteral("msdelay"));
    b.p.delta = deltaFitting(l.weff, gridFor(l, f0));
    const double d = b.p.delta;
    const double w = l.weff;
    const double m0 = 3 * d;
    const double Lr = snap(v.value(QStringLiteral("run")) * l.lambdaG(f0), d);

    // Прогоны идут вдоль X и уложены друг над другом с шагом 2*w: между ними остаётся
    // полоса той же ширины, и колено получается квадратным.
    const auto yOf = [&](int i) { return m0 + i * 2.0 * w; };
    // Колено между прогоном i и i+1 стоит справа при чётном i и слева при нечётном.
    const auto turnRight = [](int i) { return i % 2 == 0; };

    const double x0 = 0.0, x1 = Lr;
    // ⚠️ Координаты колен кладём на УЗЛЫ сетки. Ширина канала кратна шагу по
    // построению (deltaFitting), но «x1 - w» в double даёт число с мусором в
    // последних битах, и при круговороте «открыл -> сохранил» задание отличалось от
    // исходного десятым знаком: эмиттер пишет локальные координаты, а читатель
    // прибавляет анкер, и разряда перестаёт хватать.
    const double xRightIn = snap(x1 - w, d);
    const double xLeftIn = snap(x0 + w, d);
    const double yTop = yOf(n - 1) + w + 3 * d;
    b.p.box = {x0, x1, 0.0, yTop};
    b.p.description = tpl::tr("Линия задержки «змейкой»: %1 прогонов по %2, линия %3 Ом, "
                              "%4. Падающая волна — видеоимпульс из файла.",
                              "Meander delay line: %1 runs of %2, line %3 Ohm, %4. The incident "
                              "wave is a video pulse from a file.")
                          .arg(n).arg(mm(Lr), num(l.z0), bd.sub.name);

    // Заливка: прогоны и колена.
    for (int i = 0; i < n; ++i)
        fill(b, tpl::tr("прогон %1", "run %1").arg(i + 1), l, x0, x1, yOf(i), yOf(i) + w);
    for (int i = 0; i + 1 < n; ++i) {
        const double cx1 = turnRight(i) ? x1 : xLeftIn;
        const double cx0 = turnRight(i) ? xRightIn : x0;
        fill(b, tpl::tr("колено %1", "bend %1").arg(i + 1), l, cx0, cx1, yOf(i) + w, yOf(i + 1));
    }

    // Горизонтальные стенки: каждая с «окном» там, где к прогону подходит колено.
    const auto hWall = [&](const QString &name, double y, bool hole, bool holeRight) {
        if (!hole) {
            b.polyline(name, Material::Magnetic, seg(QPointF(x0, y), QPointF(x1, y)));
        } else if (holeRight) {
            b.polyline(name, Material::Magnetic, seg(QPointF(x0, y), QPointF(xRightIn, y)));
        } else {
            b.polyline(name, Material::Magnetic, seg(QPointF(xLeftIn, y), QPointF(x1, y)));
        }
    };
    for (int i = 0; i < n; ++i) {
        hWall(tpl::tr("прогон %1: стенка снизу", "run %1: bottom wall").arg(i + 1), yOf(i), i > 0,
              turnRight(i - 1));
        hWall(tpl::tr("прогон %1: стенка сверху", "run %1: top wall").arg(i + 1), yOf(i) + w, i + 1 < n,
              turnRight(i));
    }
    // Стенки колен — ОБЕ, и внутренняя, и внешняя.
    //
    // ⚠️ Внешнюю нельзя заменить границей области: края коробки магнитными НЕ
    // являются (см. правила входа плоской волны), и на повороте волна уходила бы
    // в край вместо того, чтобы завернуть. Стенка ставится ровно по краю области —
    // блок на границе законен, так же стоят стенки волноводных образцов.
    for (int i = 0; i + 1 < n; ++i) {
        const double cIn = turnRight(i) ? xRightIn : xLeftIn;
        const double cOut = turnRight(i) ? x1 : x0;
        b.polyline(tpl::tr("колено %1: внутренняя стенка",
                           "bend %1: inner wall").arg(i + 1), Material::Magnetic,
                   seg(QPointF(cIn, yOf(i) + w), QPointF(cIn, yOf(i + 1))));
        b.polyline(tpl::tr("колено %1: внешняя стенка", "bend %1: outer wall").arg(i + 1), Material::Magnetic,
                   seg(QPointF(cOut, yOf(i) + w), QPointF(cOut, yOf(i + 1))));
    }
    // Закрываем свободные торцы прогонов — все, кроме двух портов.
    const bool outRight = !turnRight(n - 2);
    for (int i = 0; i < n; ++i) {
        const bool leftIsPort = (i == 0) || (i == n - 1 && !outRight);
        const bool rightIsPort = (i == n - 1 && outRight);
        // Торец занят коленом? Колено стоит не в торце, а сверху/снизу, так что торец
        // всегда либо порт, либо стенка.
        if (!leftIsPort) {
            b.polyline(tpl::tr("прогон %1: торец слева", "run %1: left end").arg(i + 1), Material::Magnetic,
                       seg(QPointF(x0, yOf(i)), QPointF(x0, yOf(i) + w)));
        }
        if (!rightIsPort) {
            b.polyline(tpl::tr("прогон %1: торец справа", "run %1: right end").arg(i + 1), Material::Magnetic,
                       seg(QPointF(x1, yOf(i)), QPointF(x1, yOf(i) + w)));
        }
    }

    msPort(b, tpl::tr("вход 1", "port 1"), PortSide::Left, yOf(0), yOf(0) + w, true, l);
    msPort(b, tpl::tr("вход 2", "port 2"), outRight ? PortSide::Right : PortSide::Left,
           yOf(n - 1), yOf(n - 1) + w, false, l);
    // Ссылку берём ПОСЛЕ всех портов: см. пояснение в рецепте рефлектометрии.
    Port &in = b.p.ports[0];

    // Видеоимпульс: гауссова шапка без несущей. Формулой её не задать — у ядра всегда
    // есть множитель sin(2*pi*FREQ*t), то есть несущая.
    const double path = n * Lr + (n - 1) * w;
    const double travel = path * std::sqrt(l.epsEff) / 299.792458; // нс
    in.signal.source = SignalSource::File;
    in.signal.wave.kind = Waveform::GaussVideo;
    in.signal.wave.amplitude = 1.0;
    in.signal.wave.start = 0.0;
    in.signal.wave.width = std::max(3.0 * d / 299.792458, travel / 8.0);
    b.p.freqs = {suggestedFreq(in.signal.wave)};
    b.outputs(false);
    b.time(3.0 * travel + 1.0);

    QVector<Row> rows = boardRows(bd);
    rows << lineRow(tpl::tr("Линия %1 Ом", "Line %1 Ohm").arg(num(l.z0)), l, f0);
    rows << Row{tpl::tr("Развёрнутая длина", "Unfolded length"),
                tpl::tr("%1 при габарите %2 x %3",
                        "%1 with overall size %2 x %3").arg(mm(path), mm(Lr), mm(yTop))};
    rows << Row{tpl::tr("Задержка по теории", "Theoretical delay"),
                tpl::tr("%1 нс (eps_eff = %2)", "%1 ns (eps_eff = %2)").arg(num(travel, 4), num(l.epsEff))};
    rows << Row{tpl::tr("Длительность импульса", "Pulse duration"), tpl::tr("tau = %1 нс", "tau = %1 ns")
                    .arg(num(in.signal.wave.width, 4))};
    rows << Row{tpl::tr("Что смотреть", "What to look at"),
                tpl::tr("в файле %1.t импульс на входе 2 приходит на %2 нс позже, чем на "
                        "входе 1",
                        "in the file %1.t the pulse at port 2 arrives %2 ns later than at port 1").arg(b.p.output.signalFile, num(travel, 4))};
    return b.done(rows, {tpl::tr("Смысл задания — ВРЕМЕННАЯ картина, поэтому матрица "
                                 "рассеяния не снимается. Результат читают по файлу "
                                 "сигналов .t.",
                                 "The point of the task is the TIME picture, so the scattering "
                                 "matrix is not taken. The result is read from the signal file .t."),
                         tpl::tr("Колена прибавляют задержку и чуть портят форму импульса: "
                                 "сравните приход с теоретической задержкой из паспорта.",
                                 "The bends add delay and slightly distort the pulse shape: compare "
                                 "the arrival with the theoretical delay from the datasheet."),
                         tpl::tr("Соседние прогоны друг с другом НЕ связаны: стенки канала "
                                 "магнитные, и поле за них не выходит. У настоящей "
                                 "змейки связь между витками есть, и двумерная модель её "
                                 "не воспроизводит.",
                                 "Adjacent runs are NOT coupled to each other: the channel walls "
                                 "are magnetic, and the field does not leave them. A real meander "
                                 "has coupling between turns, and the 2D model does not reproduce "
                                 "it.")});
}

// ------------------------------------------------------------ плавный переход

/// @brief Рецепт «Плавный переход (тейпер)» (`ms_taper`).
///
/// Экспоненциальный переход шире по полосе, чем lambda/4.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `sub`, `h` — подложка и толщина, мм (см. `msp::boardParams`);
///                `z1`, `z2` — согласуемые сопротивления, Ом;
///                `len` — длина перехода, в длинах волны;
///                `steps` — число ступеней;
///                `f0` — рабочая частота, ГГц
/// @return Задание, паспорт «теория» и пояснения.
Build buildTaper(const Values &v)
{
    const Board bd = board(v);
    const double f0 = v.value(QStringLiteral("f0"));
    const double z1 = v.value(QStringLiteral("z1")), z2 = v.value(QStringLiteral("z2"));
    const int steps = int(v.value(QStringLiteral("steps")));
    const Line l1 = bd.line(z1), l2 = bd.line(z2);

    Builder b(QStringLiteral("mstaper"));
    const Line &narrow = (l1.weff < l2.weff) ? l1 : l2;
    b.p.delta = gridFor(narrow, f0);
    const double d = b.p.delta;
    const double wMax = std::max(l1.weff, l2.weff);
    const double yc = snap(3 * d + wMax / 2.0, d);
    const double feed = snap(0.5 * l1.lambdaG(f0), d);
    const double L = snap(v.value(QStringLiteral("len")) * l1.lambdaG(f0), d);
    const double X = feed + L + feed;
    b.p.box = {0.0, X, 0.0, 2 * yc};
    b.p.description = tpl::tr("Экспоненциальный переход %1 -> %2 Ом длиной %3, %4.",
                              "Exponential taper %1 -> %2 Ohm, length %3, %4.")
                          .arg(num(z1), num(z2), mm(L), bd.sub.name);

    // Z(x) = Z1*exp(x/L*ln(Z2/Z1)) — классический экспоненциальный тейпер.
    const double lnr = std::log(z2 / z1);
    QVector<QPointF> lower, upper;
    lower << QPointF(0, yc - l1.weff / 2);
    upper << QPointF(0, yc + l1.weff / 2);
    fill(b, tpl::tr("питание %1 Ом", "feed %1 Ohm").arg(num(z1)), l1, 0.0, feed, yc - l1.weff / 2,
         yc + l1.weff / 2);
    lower << QPointF(feed, yc - l1.weff / 2);
    upper << QPointF(feed, yc + l1.weff / 2);

    for (int i = 0; i < steps; ++i) {
        const double u0 = double(i) / steps, u1 = double(i + 1) / steps;
        const double zc = z1 * std::exp(lnr * (u0 + u1) / 2.0); // импеданс середины шага
        const Line ls = bd.line(zc);
        const double xa = snap(feed + L * u0, d), xb = snap(feed + L * u1, d);
        if (xb <= xa)
            continue;
        fill(b, tpl::tr("шаг %1 (%2 Ом)", "step %1 (%2 Ohm)").arg(i + 1).arg(num(zc)), ls, xa, xb,
             yc - ls.weff / 2, yc + ls.weff / 2);
        lower << QPointF(xa, yc - ls.weff / 2) << QPointF(xb, yc - ls.weff / 2);
        upper << QPointF(xa, yc + ls.weff / 2) << QPointF(xb, yc + ls.weff / 2);
    }

    lower << QPointF(feed + L, yc - l2.weff / 2) << QPointF(X, yc - l2.weff / 2);
    upper << QPointF(feed + L, yc + l2.weff / 2) << QPointF(X, yc + l2.weff / 2);
    fill(b, tpl::tr("питание %1 Ом", "feed %1 Ohm").arg(num(z2)), l2, feed + L, X, yc - l2.weff / 2,
         yc + l2.weff / 2);

    b.polyline(tpl::tr("край полоски, низ", "strip edge, bottom"), Material::Magnetic, lower);
    b.polyline(tpl::tr("край полоски, верх", "strip edge, top"), Material::Magnetic, upper);
    msPort(b, tpl::tr("вход 1", "port 1"), PortSide::Left, yc - l1.weff / 2, yc + l1.weff / 2, true,
           l1);
    msPort(b, tpl::tr("вход 2", "port 2"), PortSide::Right, yc - l2.weff / 2, yc + l2.weff / 2, false,
           l2);
    b.freqs(std::round(f0 * 0.3 * 100) / 100, std::round(f0 * 2.0 * 100) / 100, 35);
    b.outputs(true);
    b.time(settleTime(X * std::sqrt(l1.epsEff), f0 * 0.3, 0.0));

    // |Г| экспоненциального тейпера: ln(Z2/Z1)/2 * sin(bL)/(bL).
    const auto gamma = [&](double f) {
        const double bl = 2.0 * kPi * L / (rf::lambda0(f) / std::sqrt(l1.epsEff));
        const double sinc = bl > 1e-9 ? std::sin(bl) / bl : 1.0;
        return std::fabs(lnr / 2.0 * sinc);
    };

    QVector<Row> rows = boardRows(bd);
    rows << lineRow(tpl::tr("Линия %1 Ом", "Line %1 Ohm").arg(num(z1)), l1, f0);
    rows << lineRow(tpl::tr("Линия %1 Ом", "Line %1 Ohm").arg(num(z2)), l2, f0);
    rows << Row{tpl::tr("Длина перехода", "Taper length"),
                tpl::tr("%1 (%2 длины волны), шагов %3", "%1 (%2 wavelengths), %3 steps")
                    .arg(mm(L), num(L / l1.lambdaG(f0)))
                    .arg(steps)};
    for (double k : {0.5, 1.0, 2.0})
        rows << Row{tpl::tr("Теория тейпера, %1 ГГц", "Taper theory, %1 GHz").arg(num(k * f0, 3)),
                    QStringLiteral("|S11| = %1").arg(db(gamma(k * f0)))};
    rows << Row{tpl::tr("Без перехода", "Without taper"),
                QStringLiteral("|S11| = %1").arg(db(std::fabs(z2 - z1) / (z2 + z1)))};
    return b.done(rows, {tpl::tr("Экспоненциальный переход широкополоснее "
                                 "четвертьволнового трансформатора: отражение падает как "
                                 "sin(bL)/(bL), то есть монотонно с длиной.",
                                 "An exponential taper is more broadband than a quarter-wave "
                                 "transformer: the reflection falls as sin(bL)/(bL), i.e. "
                                 "monotonically with length."),
                         tpl::tr("Переход набран ступеньками: в планарной модели с ростом "
                                 "Z0 меняется и ширина канала, и eps_eff, поэтому каждый "
                                 "шаг — своя линия со своим заполнением.",
                                 "The taper is built of steps: in the planar model both the channel "
                                 "width and eps_eff change as Z0 grows, so each step is a separate "
                                 "line with its own filling.")});
}

// ------------------------------------------------------------ патч-антенна

/// @brief Рецепт «Патч-антенна: полость и излучение» (`ms_patch`).
///
/// Края патча — магнитные стенки или открытые щели; рамка поглотителя.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `sub`, `h` — подложка и толщина, мм (см. `msp::boardParams`);
///                `z0` — сопротивление питающей линии, Ом;
///                `fr` — резонансная частота, ГГц;
///                `edges` — края патча: 0 — магнитные стенки (резонатор), 1 — открытые (излучение);
///                `air` — воздух вокруг, в длинах волны;
///                `frame` — число слоёв поглотителя в рамке
/// @return Задание, паспорт «теория» и пояснения.
Build buildPatch(const Values &v)
{
    const Board bd = board(v);
    const double fr = v.value(QStringLiteral("fr"));
    const Line feedLine = bd.line(v.value(QStringLiteral("z0")));

    // Ширина патча по обычной рекомендации, проницаемость — по этой ширине.
    const double c = 299.792458; // мм*ГГц
    const double Wp = c / (2.0 * fr) * std::sqrt(2.0 / (bd.sub.eps + 1.0));
    const double epsEffPatch = rf::msEpsEff(Wp / bd.h, bd.sub.eps);
    const double Lp = c / (2.0 * fr * std::sqrt(epsEffPatch));

    Builder b(QStringLiteral("mspatch"));
    b.p.delta = chooseDelta(c / fr / std::sqrt(epsEffPatch), std::min(feedLine.weff, Lp), 24.0,
                            12.0);
    const double d = b.p.delta;
    const int frame = int(v.value(QStringLiteral("frame")));
    const double air = snap(v.value(QStringLiteral("air")) * c / fr, d); // воздух вокруг патча
    const double t = frame * d;

    const double feed = snap(0.5 * feedLine.lambdaG(fr), d);
    const double xp = snap(t + air + feed, d);
    const double xq = snap(xp + Lp, d);
    const double X = snap(xq + air + t, d);
    const double yc = snap((t + air + Wp / 2.0 + air + t) / 2.0, d);
    const double Y = snap(2.0 * yc, d);
    const double yp0 = snap(yc - Wp / 2.0, d), yp1 = snap(yc + Wp / 2.0, d);
    const double wf = feedLine.weff;
    const double yf0 = snap(yc - wf / 2.0, d), yf1 = snap(yc + wf / 2.0, d);

    b.p.box = {0.0, X, 0.0, Y};
    // В описании — размеры, которые РЕАЛЬНО легли на сетку, а не расчётные: человек
    // читает описание и сверяет его с холстом, и расхождение в сотых только путает.
    const double LpGrid = xq - xp, WpGrid = yp1 - yp0;
    b.p.description = tpl::tr("Патч-антенна %1 x %2 на %3 ГГц, модель резонатора с "
                              "излучающими щелями, %4.",
                              "Patch antenna %1 x %2 at %3 GHz, cavity model with radiating slots, "
                              "%4.")
                          .arg(mm(LpGrid), mm(WpGrid), num(fr, 3), bd.sub.name);

    // Патч: заливка eps_eff и магнитные стенки по двум краям вдоль резонанса.
    fill(b, tpl::tr("патч", "patch"), Line{v.value(QStringLiteral("z0")), 0.0, epsEffPatch, Wp}, xp,
         xq, yp0, yp1);
    b.polyline(tpl::tr("край патча, низ", "patch edge, bottom"), Material::Magnetic,
               seg(QPointF(xp, yp0), QPointF(xq, yp0)));
    b.polyline(tpl::tr("край патча, верх", "patch edge, top"), Material::Magnetic,
               seg(QPointF(xp, yp1), QPointF(xq, yp1)));

    // 🔑 Два ЧЕСТНЫХ случая, и выбирает их человек.
    //
    // «Резонатор»: края x тоже магнитные — тогда вдоль x стоит полуволновый резонанс,
    // и его частоту даёт формула. Это и есть классическая модель резонатора патча
    // (Баланис): сперва находят частоту в замкнутой полости, а излучение добавляют
    // потом как поправку.
    //
    // «Излучение»: края x открыты в воздух, и резонатора вдоль x НЕТ ВОВСЕ.
    // Проверено прогоном: при открытых краях провалы |S11| оказались на 2.475 и
    // 3.825 ГГц, тогда как полуволновый резонанс патча — 2.98 ГГц. Провалы ставит
    // линия питания со щелью, а не полость. Паспорт, обещавший бы здесь резонанс,
    // просто врал бы — что и выяснилось числами ядра.
    const bool closed = v.value(QStringLiteral("edges")) < 0.5;
    if (closed) {
        // Левый край — двумя кусками: в середине щель, через неё входит линия.
        b.polyline(tpl::tr("край патча, слева снизу", "patch edge, left bottom"), Material::Magnetic,
                   seg(QPointF(xp, yp0), QPointF(xp, yf0)));
        b.polyline(tpl::tr("край патча, слева сверху", "patch edge, left top"), Material::Magnetic,
                   seg(QPointF(xp, yf1), QPointF(xp, yp1)));
        b.polyline(tpl::tr("край патча, справа", "patch edge, right"), Material::Magnetic,
                   seg(QPointF(xq, yp0), QPointF(xq, yp1)));
    }

    // Питающая линия от левой границы до щели патча.
    fill(b, tpl::tr("питающая линия", "feed line"), feedLine, 0.0, xp, yf0, yf1);
    b.polyline(tpl::tr("край линии, низ", "line edge, bottom"), Material::Magnetic,
               seg(QPointF(0, yf0), QPointF(xp, yf0)));
    b.polyline(tpl::tr("край линии, верх", "line edge, top"), Material::Magnetic,
               seg(QPointF(0, yf1), QPointF(xp, yf1)));

    // Рамка поглотителя — это «свободное пространство»: без неё область была бы
    // закрытой коробкой, и излучённое вернулось бы обратно эхом.
    // ⚠️ ABSORBER — контур, а не заливка: слои ставим по одному на шаг сетки.
    for (int i = 0; i < frame; ++i) {
        const double o = i * d;
        b.polyline(tpl::tr("поглотитель справа, слой %1",
                           "absorber right, layer %1").arg(i + 1), Material::Absorber,
                   seg(QPointF(X - o, 0.0), QPointF(X - o, Y)));
        b.polyline(tpl::tr("поглотитель снизу, слой %1",
                           "absorber bottom, layer %1").arg(i + 1), Material::Absorber,
                   seg(QPointF(0.0, o), QPointF(X, o)));
        b.polyline(tpl::tr("поглотитель сверху, слой %1",
                           "absorber top, layer %1").arg(i + 1), Material::Absorber,
                   seg(QPointF(0.0, Y - o), QPointF(X, Y - o)));
        // Слева — только выше и ниже апертуры питающей линии: сам вход закрывать нечем.
        b.polyline(tpl::tr("поглотитель слева снизу, слой %1", "absorber left bottom, layer %1").arg(i + 1),
                   Material::Absorber, seg(QPointF(o, 0.0), QPointF(o, yf0)));
        b.polyline(tpl::tr("поглотитель слева сверху, слой %1", "absorber left top, layer %1").arg(i + 1),
                   Material::Absorber, seg(QPointF(o, yf1), QPointF(o, Y)));
    }

    msPort(b, tpl::tr("вход 1", "port 1"), PortSide::Left, yf0, yf1, true, feedLine);
    b.freqs(std::round(fr * 0.7 * 100) / 100, std::round(fr * 1.3 * 100) / 100, 25);
    b.outputs(true);
    b.time(settleTime(4.0 * X * std::sqrt(epsEffPatch), fr * 0.7, 0.0, 5.0));

    QVector<Row> rows = boardRows(bd);
    rows << Row{tpl::tr("Ширина патча", "Patch width"),
                tpl::tr("%1 (по формуле c/(2*fr)*sqrt(2/(eps+1)))",
                        "%1 (by the formula c/(2*fr)*sqrt(2/(eps+1)))").arg(mm(Wp))};
    rows << Row{tpl::tr("Проницаемость патча",
                        "Patch permittivity"), tpl::tr("eps_eff = %1 при W/h = %2",
                                                               "eps_eff = %1 at W/h = %2")
                    .arg(num(epsEffPatch), num(Wp / bd.h))};
    rows << Row{tpl::tr("Длина патча", "Patch length"),
                tpl::tr("%1 = c/(2*fr*sqrt(eps_eff)) — БЕЗ поправки на края",
                        "%1 = c/(2*fr*sqrt(eps_eff)), WITHOUT the edge correction").arg(mm(Lp))};
    rows << Row{tpl::tr("Легло на сетку", "Snapped to the grid"),
                tpl::tr("%1 x %2 при шаге %3 — резонанс считается по ЭТИМ размерам",
                        "%1 x %2 with step %3: the resonance is computed from THESE dimensions")
                    .arg(mm(LpGrid), mm(WpGrid), mm(d))};
    rows << Row{tpl::tr("Резонанс по размерам сетки", "Resonance from the grid dimensions"),
                tpl::tr("%1 ГГц", "%1 GHz")
                    .arg(num(c / (2.0 * LpGrid * std::sqrt(epsEffPatch)), 4))};
    rows << lineRow(tpl::tr("Питающая линия %1 Ом", "Feed line %1 Ohm").arg(num(feedLine.z0)), feedLine, fr);
    const double frGrid = c / (2.0 * LpGrid * std::sqrt(epsEffPatch));
    rows << Row{tpl::tr("Края патча", "Patch edges"),
                closed ? tpl::tr("магнитные стенки — замкнутая полость, резонатор",
                                 "magnetic walls: a closed cavity, a resonator")
                       : tpl::tr("открыты в воздух — излучающие щели", "open to the air: radiating slots")};
    rows << Row{tpl::tr("Ожидаемо", "Expected"),
                closed ? tpl::tr("узкий провал |S11| — полуволновый резонанс полости. "
                                 "Формула по размерам сетки даёт %1 ГГц, а ядро на "
                                 "умолчаниях показало -12.8 дБ на 3.30 ГГц, то есть на "
                                 "11 процентов выше: щель под питание в торцевой стенке "
                                 "замыкает полость не до конца",
                                 "a narrow |S11| dip, the half-wave resonance of the cavity. The "
                                 "formula from the grid dimensions gives %1 GHz, while the kernel "
                                 "at the defaults showed -12.8 dB at 3.30 GHz, i.e. 11 percent "
                                 "higher: the feed slot in the end wall does not close the cavity "
                                 "completely")
                             .arg(num(frGrid, 4))
                       : tpl::tr("провалы |S11| ставит линия питания со щелью, а НЕ "
                                 "полость: резонатора вдоль патча при открытых краях нет. "
                                 "Проверено прогоном — провалы вышли на 2.475 и 3.825 ГГц "
                                 "при расчётном резонансе %1 ГГц",
                                 "the |S11| dips are set by the feed line with the slot, NOT by the "
                                 "cavity: with open edges there is no resonator along the patch. "
                                 "Checked by a run: the dips came out at 2.475 and 3.825 GHz with "
                                 "the calculated resonance at %1 GHz").arg(num(frGrid, 4))};

    QStringList notes;
    if (closed) {
        notes << tpl::tr("Все четыре края патча — магнитные стенки, в левом оставлена "
                         "щель под линию питания. Так и считают частоту резонатора патча "
                         "(Баланис): излучение добавляют потом поправкой, а двумерная "
                         "модель даёт саму частоту.",
                         "All four edges of the patch are magnetic walls, with a slot left in the "
                         "left one for the feed line. This is how the patch resonator frequency is "
                         "computed (Balanis): radiation is added later as a correction, and the 2D "
                         "model gives the frequency itself.");
        notes << tpl::tr("Рамка поглотителя вокруг области здесь страховка: из замкнутой "
                         "полости наружу ничего не идёт. Переключите края на открытые — и "
                         "рамка станет свободным пространством, в которое патч излучает.",
                         "The absorber frame around the domain is a safeguard here: nothing leaves "
                         "the closed cavity. Switch the edges to open, and the frame becomes the "
                         "free space into which the patch radiates.");
    } else {
        notes << tpl::tr("Края патча открыты — это излучающие щели, и рамка поглотителя "
                         "вокруг области играет свободное пространство. ⚠ Играет "
                         "неточно: в канале с магнитными стенками ABSORBER отражает "
                         "около -2...-7 дБ (измерено образцом «Поглотитель в канале»), "
                         "так что часть излучённого возвращается эхом.",
                         "The patch edges are open: they are radiating slots, and the absorber "
                         "frame around the domain plays the role of free space. (!) It does so "
                         "imprecisely: in a channel with magnetic walls ABSORBER reflects about "
                         "-2...-7 dB (measured by the sample \"Absorber in the channel\"), so part "
                         "of the radiated power returns as an echo.");
        notes << tpl::tr("Резонанса вдоль патча здесь НЕТ: полость разомкнута. Провалы "
                         "|S11| — это согласование линии питания с излучающей щелью, и "
                         "их положение зависит от длины линии. Чтобы увидеть именно "
                         "резонанс, переключите края на магнитные стенки.",
                         "There is NO resonance along the patch here: the cavity is open. The |S11| "
                         "dips are the matching of the feed line to the radiating slot, and their "
                         "position depends on the line length. To see the resonance itself, switch "
                         "the edges to magnetic walls.");
    }
    notes << tpl::tr("⚠ Два разных сдвига частоты, и путать их нельзя. У НАСТОЯЩЕГО "
                     "патча края «удлиняют» резонатор (около 2 x 0.4 h), и его резонанс "
                     "ниже формулы идеальной полости — этого двумерная модель не знает "
                     "вовсе. А в самой модели резонанс вышел ВЫШЕ формулы (3.30 против "
                     "2.98 ГГц на умолчаниях): так действует щель под питание. Оба числа "
                     "измерены, а не выведены.",
                     "(!) There are two different frequency shifts, and they must not be confused. "
                     "In a REAL patch the edges \"lengthen\" the resonator (by about 2 x 0.4 h), "
                     "and its resonance is below the ideal-cavity formula; the 2D model knows "
                     "nothing about this. In the model itself the resonance came out ABOVE the "
                     "formula (3.30 versus 2.98 GHz at the defaults): this is the effect of the "
                     "feed slot. Both numbers were measured, not derived.");
    notes << tpl::tr("Питание торцом, а не вставкой: согласование не идеальное, и глубина "
                     "провала тут не показатель.",
                     "End feed rather than inset feed: the matching is not ideal, and the dip depth "
                     "is not an indicator here.");
    return b.done(rows, notes);
}

// ------------------------------------------------------------ ПЗФ на шлейфах

/// @brief Рецепт «ПЗФ на разомкнутых шлейфах» (`ms_bsf`).
///
/// Полоса заграждения; считается свипом или одним проходом с гребёнкой тонов из файла.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `sub`, `h` — подложка и толщина, мм (см. `msp::boardParams`);
///                `z0` — сопротивление линии, Ом;
///                `zs` — сопротивление шлейфов, Ом;
///                `stubs` — число шлейфов;
///                `mode` — способ расчёта: 0 — свип по частотам (41 шаг), 1 — один проход, гребёнка тонов;
///                `corr` — поправка длин на соединения, %;
///                `f0` — рабочая частота, ГГц
/// @return Задание, паспорт «теория» и пояснения.
/// @see https://www.tamic.ru/docs/theory/smatrix.html#multifreq
Build buildBsf(const Values &v)
{
    const Board bd = board(v);
    const double f0 = v.value(QStringLiteral("f0"));
    const Line l = bd.line(v.value(QStringLiteral("z0")));
    const Line st = bd.line(v.value(QStringLiteral("zs")));
    const int n = int(v.value(QStringLiteral("stubs")));
    const bool wideband = v.value(QStringLiteral("mode")) > 0.5;
    const double corr = v.value(QStringLiteral("corr")) / 100.0;

    Builder b(QStringLiteral("msbsf"));
    b.p.delta = deltaFitting(st.weff, gridFor(st, f0));
    const double d = b.p.delta;
    const double lg = l.lambdaG(f0);
    const double w = l.weff, ws = st.weff;
    const double yb = 3 * d, yh = yb + w;
    const double pitch = snap(lg / 4.0 * (1.0 + corr), d); // между осями шлейфов
    const double lead = snap(lg * 0.75, d);
    const double stubLen = snap(lg / 4.0 * (1.0 + corr) - w / 2.0, d);
    const double ytop = yh + stubLen;

    QVector<double> axes;
    for (int i = 0; i < n; ++i)
        axes << snap(lead + i * pitch, d);
    const double X = snap(axes.last() + lead, d);
    b.p.box = {0.0, X, 0.0, ytop + 3 * d};
    b.p.description = tpl::tr("Полосно-заграждающий фильтр на %1 разомкнутых шлейфах "
                              "lambda/4, %2 ГГц, линия %3 Ом, шлейфы %4 Ом, %5.",
                              "Band-stop filter on %1 open stubs lambda/4, %2 GHz, line %3 Ohm, "
                              "stubs %4 Ohm, %5.")
                          .arg(n).arg(num(f0, 3), num(l.z0), num(st.z0), bd.sub.name);

    b.polyline(tpl::tr("край полоски, низ", "strip edge, bottom"), Material::Magnetic,
               seg(QPointF(0, yb), QPointF(X, yb)));
    // Верхний край с «гребёнкой» шлейфов: одна ломаная на всю линию.
    QVector<QPointF> top{QPointF(0, yh)};
    for (double a : axes) {
        top << QPointF(a - ws / 2.0, yh) << QPointF(a - ws / 2.0, ytop)
            << QPointF(a + ws / 2.0, ytop) << QPointF(a + ws / 2.0, yh);
    }
    top << QPointF(X, yh);
    b.polyline(tpl::tr("край полоски и шлейфов", "strip and stub edge"), Material::Magnetic, top);

    fill(b, tpl::tr("линия", "line"), l, 0.0, X, yb, yh);
    for (int i = 0; i < n; ++i) {
        fill(b, tpl::tr("шлейф %1", "stub %1").arg(i + 1), st, axes.at(i) - ws / 2.0,
             axes.at(i) + ws / 2.0, yh, ytop);
    }

    msPort(b, tpl::tr("вход 1", "port 1"), PortSide::Left, yb, yh, true, l);
    msPort(b, tpl::tr("вход 2", "port 2"), PortSide::Right, yb, yh, false, l);
    // Ссылку берём ПОСЛЕ всех портов: см. пояснение в рецепте рефлектометрии.
    Port &in = b.p.ports[0];

    const double fLo = std::round(f0 * 0.5 * 100) / 100;
    const double fHi = std::round(f0 * 1.5 * 100) / 100;
    if (wideband) {
        // 🔑 Весь диапазон за ОДИН проход: гребёнка тонов в файле падающей волны.
        // Вместо сорока одного прогона — один, а матрицу по полосе строит
        // постобработка по файлу сигналов .t.
        in.signal.source = SignalSource::File;
        in.signal.wave.kind = Waveform::MultiTone;
        in.signal.wave.f1 = fLo;
        in.signal.wave.f2 = fHi;
        in.signal.wave.tones = 41;
        in.signal.wave.amplitude = 1.0;
        in.signal.wave.start = 0.0;
        b.p.freqs = {suggestedFreq(in.signal.wave)};
        b.outputs(false);
        in.signal.wave.duration = 0.0; // поставим после b.time(), когда известен счёт
    } else {
        b.freqs(fLo, fHi, 41);
        b.outputs(true);
    }
    b.time(settleTime((X + 2 * stubLen) * std::sqrt(l.epsEff), f0 * 0.5, 0.0, 5.0));
    if (wideband) {
        // Посылка тянется на весь счёт: многотон непрерывный, и постобработка
        // раскладывает его по частотам на УСТАНОВИВШЕМСЯ участке.
        b.p.ports[0].signal.wave.duration = b.p.timeMax;
    }

    QVector<Row> rows = boardRows(bd);
    rows << lineRow(tpl::tr("Линия %1 Ом", "Line %1 Ohm").arg(num(l.z0)), l, f0);
    rows << lineRow(tpl::tr("Шлейфы %1 Ом", "Stubs %1 Ohm").arg(num(st.z0)), st, f0);
    rows << Row{tpl::tr("Шлейфов", "Stubs"),
                tpl::tr("%1, между осями %2, длина от края линии %3",
                        "%1, spacing between axes %2, length from the line edge %3")
                    .arg(n).arg(mm(pitch), mm(stubLen))};
    rows << Row{tpl::tr("Ожидаемо", "Expected"),
                tpl::tr("провал |S21| на %1 ГГц тем глубже и шире, чем больше шлейфов; "
                        "у %2 шлейфов полоса заграждения заметно шире, чем у одного",
                        "the |S21| dip at %1 GHz is deeper and wider the more stubs there are; with "
                        "%2 stubs the stopband is noticeably wider than with one")
                    .arg(num(f0, 3)).arg(n)};
    rows << Row{tpl::tr("Способ расчёта", "Calculation method"),
                wideband ? tpl::tr("ОДИН проход: гребёнка из %1 тонов %2...%3 ГГц в файле "
                                   "падающей волны",
                                   "ONE pass: a comb of %1 tones %2...%3 GHz in the incident-wave "
                                   "file")
                               .arg(41).arg(num(fLo, 3), num(fHi, 3))
                         : tpl::tr("свип: 41 шаг, на каждом своя FREQ",
                                   "sweep: 41 steps, each with its own FREQ")};
    // Числа замерены на умолчаниях (2026-10-01, i5-9600KF): свип 393 с, широкополосный
    // проход 13.6 с. Разница — в число точек свипа, как и обещает метод.
    rows << Row{tpl::tr("Время счёта (замерено)", "Run time (measured)"),
                wideband ? tpl::tr("13.6 с — один проход; свип по тем же 41 точкам шёл "
                                   "393 с, то есть в 29 раз дольше",
                                   "13.6 s for one pass; a sweep over the same 41 points took 393 "
                                   "s, i.e. 29 times longer")
                         : tpl::tr("393 с на 41 шаг; тем же заданием через гребёнку тонов "
                                   "вышло 13.6 с — в 29 раз быстрее",
                                   "393 s for 41 steps; the same task with a tone comb took 13.6 s, "
                                   "i.e. 29 times faster")};
    rows << Row{tpl::tr("Поправка длин", "Length correction"), QStringLiteral("%1 %").arg(num(corr * 100.0))};

    QStringList notes{tpl::tr("Шлейфы работают по самому каналу, а не через краевое поле, "
                              "поэтому планарная модель их выражает. Фильтр на СВЯЗАННЫХ "
                              "резонаторах в ней не получится: магнитные стенки не "
                              "пропускают поле между полосками.",
                              "The stubs act through the channel itself, not through the fringing "
                              "field, so the planar model can express them. A filter on COUPLED "
                              "resonators cannot be built in it: the magnetic walls do not let the "
                              "field pass between the strips.")};
    if (wideband) {
        notes << tpl::tr("Широкополосный проход считается столько же, сколько один шаг "
                         "свипа, — выигрыш во столько раз, сколько в свипе точек. "
                         "Матрица рассеяния не снимается: встроенный SMATRIX "
                         "одночастотный, полосу строит постобработка по файлу .t.",
                         "A broadband pass takes as long as one sweep step, so the gain equals the "
                         "number of points in the sweep. The scattering matrix is not taken: the "
                         "built-in SMATRIX is single-frequency; post-processing builds the band "
                         "from the .t file.");
        notes << tpl::tr("FREQ стоит в ЦЕНТРЕ полосы: по ней ядро согласует порт.",
                         "FREQ is at the CENTER of the band: the kernel matches the port at this "
                         "frequency.");
    } else {
        notes << tpl::tr("Тот же диапазон можно посчитать за ОДИН проход — переключите "
                         "способ расчёта на гребёнку тонов.",
                         "The same range can be computed in ONE pass: switch the calculation method "
                         "to the tone comb.");
    }
    return b.done(rows, notes);
}

} // namespace

void addMicrostrip2(QVector<Sample> *out)
{
    const QString sec = QStringLiteral("microstrip");
    const QString ref = tpl::tr("Хаммерстад-Йенсен; планарная модель Весселя-Кольхоффа",
                                "Hammerstad-Jensen; Wessel-Kohlhoff planar model");
    {
        Sample s;
        s.id = QStringLiteral("ms_bend");
        s.section = sec;
        s.title = tpl::tr("Изгиб 90° со скосом", "90 deg bend with mitre");
        s.summary = tpl::tr("Поворот линии: прямой угол против скошенного",
                            "Line bend: square corner versus mitred");
        s.standard = ref;
        s.help = tpl::tr(
                     "Прямоугольный поворот микрополосковой линии. В углу собирается лишняя "
                     "медь, и она работает как ёмкость — отражение растёт. Лечат это скосом: "
                     "внешний угол срезают под 45°. Поставьте скос 0 и сравните |S11|.",
                     "A right-angle bend of a microstrip line. Extra copper gathers in the corner "
                     "and acts as a capacitance, so the reflection grows. The cure is a mitre: the "
                     "outer corner is cut at 45 deg. Set the mitre to 0 and compare |S11|.")
                 + helpPlanar();
        s.params = boardParams();
        s.params << z0Param(tpl::tr("Сопротивление линии", "Line impedance"), 50.0) << freqParam(3.0)
                 << Param::number(QStringLiteral("miter"), tpl::tr("Скос", "Mitre"),
                                  tpl::tr("% ширины", "% of width"), 57.0, 0.0, 90.0, 1.0, 0,
                                  tpl::tr("Доля ширины канала, срезаемая у внешнего угла. "
                                          "0 — прямой угол; около 57 % считается "
                                          "оптимумом",
                                          "Fraction of the channel width cut off at the outer "
                                          "corner. 0 = square corner; about 57 % is considered "
                                          "optimal"));
        s.build = buildBend;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ms_tee");
        s.section = sec;
        s.title = tpl::tr("Т-образный делитель мощности", "T-junction power divider");
        s.summary = tpl::tr("Деление пополам на два плеча двойного импеданса",
                            "Equal split into two arms of double impedance");
        s.standard = ref;
        s.help = tpl::tr(
                     "Простейший делитель: линия Z0 расходится на два плеча по 2·Z0, и в "
                     "параллель они снова дают Z0. Мощность делится поровну, но развязки между "
                     "плечами нет — это не Вилкинсон. Хорошая задача, чтобы увидеть, сколько "
                     "портит само соединение.",
                     "The simplest divider: a Z0 line branches into two arms of 2*Z0 each, which in "
                     "parallel again give Z0. The power is split equally, but there is no isolation "
                     "between the arms; it is not a Wilkinson. A good task to see how much the "
                     "junction itself spoils.")
                 + helpPlanar();
        s.params = boardParams();
        s.params << z0Param(tpl::tr("Сопротивление входа", "Input impedance"), 50.0) << freqParam(3.0);
        s.build = buildTee;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ms_load");
        s.section = sec;
        s.title = tpl::tr("Поглотитель в канале: сколько отражает",
                          "Absorber in the channel: how much it reflects");
        s.summary = tpl::tr("Измерение ABSORBER вместо веры в него",
                            "Measuring ABSORBER instead of trusting it");
        s.standard = ref;
        s.help = tpl::tr(
                     "Планарная линия, закрытая поглотителем у края области. ABSORBER ставит "
                     "на узлах контура условие безотражательного прохода — так в двумерной "
                     "модели делают согласованную нагрузку и рамку вокруг излучающей "
                     "структуры.\n\n"
                     "Задание его ИЗМЕРЯЕТ. Прогон на умолчаниях дал |S11| от -2 до "
                     "-7 дБ: в канале с магнитными стенками поглотитель отражает заметно "
                     "больше, чем в металлическом волноводе, где тот же контур даёт "
                     "-12...-22 дБ. Это надо знать всем, кто ставит рамку поглотителя вокруг "
                     "планарной структуры.\n\n"
                     "⚠ ABSORBER — КОНТУР, а не заливка: поглощающего объёма не существует. "
                     "Число слоёв проверено прогоном и на отражение почти не влияет: стопка "
                     "контуров толстым поглотителем не становится.",
                     "A planar line closed by an absorber at the domain edge. ABSORBER imposes a "
                     "non-reflecting condition on the contour nodes: this is how a matched load and "
                     "a frame around a radiating structure are made in a 2D model.\n\nThe task "
                     "MEASURES it. A run at the defaults gave |S11| from -2 to -7 dB: in a channel "
                     "with magnetic walls the absorber reflects noticeably more than in a metal "
                     "waveguide, where the same contour gives -12...-22 dB. Anyone who puts an "
                     "absorber frame around a planar structure should know this.\n\n(!) ABSORBER is "
                     "a CONTOUR, not a fill: an absorbing volume does not exist. The number of "
                     "layers has been checked by a run and has almost no effect on the reflection: "
                     "a stack of contours does not become a thick absorber.")
                 + helpPlanar();
        s.params = boardParams();
        s.params << z0Param(tpl::tr("Сопротивление линии", "Line impedance"), 50.0) << freqParam(3.0)
                 << Param::integer(QStringLiteral("cells"), tpl::tr("Слоёв поглотителя", "Absorber layers"),
                                   1, 1, 20,
                                   tpl::tr("Проверено прогоном: на отражение почти не "
                                           "влияет. ABSORBER — одно граничное условие на "
                                           "контуре, а не слой поглощающей среды",
                                           "Checked by a run: it has almost no effect on the "
                                           "reflection. ABSORBER is one boundary condition on a "
                                           "contour, not a layer of absorbing medium"));
        s.build = buildMsLoad;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ms_openend");
        s.section = sec;
        s.title = tpl::tr("Разомкнутый конец линии", "Open end of a line");
        s.summary = tpl::tr("Магнитная стенка как холостой ход; эталон для входа",
                            "A magnetic wall as an open circuit; a reference for the port");
        s.standard = ref;
        s.help = tpl::tr(
                     "Линия, оборванная магнитной стенкой. В планарной модели это и есть "
                     "холостой ход: отражается всё, фаза считается длиной линии. Задание "
                     "полезно дважды — как эталон (|S11| обязан быть 0 дБ, и отклонение "
                     "показывает остаток самого входа) и как напоминание, что настоящий "
                     "открытый конец излучает и «удлиняет» линию.",
                     "A line terminated by a magnetic wall. In the planar model this is exactly an "
                     "open circuit: everything is reflected, and the phase is set by the line "
                     "length. The task is useful twice: as a reference (|S11| must be 0 dB, and the "
                     "deviation shows the residual of the port itself) and as a reminder that a "
                     "real open end radiates and \"lengthens\" the line.")
                 + helpPlanar();
        s.params = boardParams();
        s.params << z0Param(tpl::tr("Сопротивление линии", "Line impedance"), 50.0) << freqParam(3.0)
                 << Param::number(QStringLiteral("len"), tpl::tr("Длина", "Length"),
                                  tpl::tr("длин волны", "wavelengths"), 1.0, 0.25, 5.0, 0.25, 2);
        s.build = buildOpenEnd;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ms_tdr");
        s.section = sec;
        s.title = tpl::tr("Импульсная рефлектометрия ступеньки", "Pulse reflectometry of a step");
        s.summary = tpl::tr("Моноцикл из файла: отражение видно во времени",
                            "Monocycle from a file: the reflection is seen in time");
        s.standard = ref;
        s.help = tpl::tr(
                     "Две линии разного сопротивления встык. На вход подаётся короткий "
                     "импульс-моноцикл, и в файле сигналов видно сначала падающий импульс, "
                     "потом отражённый от ступеньки — через время двойного пробега. Так "
                     "устроена импульсная рефлектометрия: по задержке находят, ГДЕ "
                     "неоднородность, по амплитуде — какая она.\n\n"
                     "Форму такой волны выражением задать нельзя: ядро умеет только несущую "
                     "под огибающей, а у моноцикла несущей нет. Поэтому падающую волну пишет "
                     "генератор — в файл .sig рядом с заданием.",
                     "Two lines of different impedance joined end to end. A short monocycle pulse "
                     "is applied to the input, and the signal file first shows the incident pulse, "
                     "then the one reflected from the step after the round-trip time. This is how "
                     "pulse reflectometry works: the delay tells WHERE the discontinuity is, the "
                     "amplitude tells what it is.\n\nThe shape of such a wave cannot be given by an "
                     "expression: the kernel only knows a carrier under an envelope, and a "
                     "monocycle has no carrier. Therefore the generator writes the incident wave to "
                     "a .sig file next to the task.")
                 + helpPlanar();
        s.params = boardParams();
        s.params << freqParam(3.0)
                 << Param::number(QStringLiteral("z1"), QStringLiteral("Z1"), tpl::tr("Ом", "Ohm"),
                                  50.0, 20.0, 120.0, 5.0, 1)
                 << Param::number(QStringLiteral("z2"), QStringLiteral("Z2"), tpl::tr("Ом", "Ohm"),
                                  100.0, 20.0, 120.0, 5.0, 1)
                 << Param::number(QStringLiteral("len1"), tpl::tr("Длина участка 1", "Length of section 1"),
                                  tpl::tr("длин волны", "wavelengths"), 3.0, 1.0, 10.0, 0.5, 1)
                 << Param::number(QStringLiteral("len2"), tpl::tr("Длина участка 2", "Length of section 2"),
                                  tpl::tr("длин волны", "wavelengths"), 3.0, 1.0, 10.0, 0.5, 1);
        s.build = buildTdr;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ms_delay");
        s.section = sec;
        s.title = tpl::tr("Линия задержки «змейкой»", "Meander delay line");
        s.summary = tpl::tr("Видеоимпульс приходит позже: задержка во времени",
                            "The video pulse arrives later: a delay in time");
        s.standard = ref;
        s.help = tpl::tr(
                     "Линия, свёрнутая змейкой, чтобы уложить большую длину в маленький "
                     "габарит. На вход подаётся видеоимпульс, и на втором порту он приходит "
                     "позже ровно на время пробега. Задание показывает и саму задержку, и то, "
                     "как колена портят форму импульса.\n\n"
                     "Видеоимпульс (гауссова шапка без несущей) выражением тоже не задать — "
                     "в формуле ядра всегда есть множитель sin(2·pi·FREQ·t). Файл пишет "
                     "генератор.",
                     "A line folded into a meander to fit a long length into a small size. A video "
                     "pulse is applied to the input, and it arrives at the second port later by "
                     "exactly the propagation time. The task shows both the delay itself and how "
                     "the bends distort the pulse shape.\n\nA video pulse (a Gaussian bump without "
                     "a carrier) cannot be given by an expression either: the kernel formula always "
                     "contains the factor sin(2*pi*FREQ*t). The generator writes the file.")
                 + helpPlanar();
        s.params = boardParams();
        s.params << z0Param(tpl::tr("Сопротивление линии", "Line impedance"), 50.0) << freqParam(3.0)
                 // Нечётное число прогонов выводит сигнал на ПРОТИВОПОЛОЖНУЮ сторону —
                 // так картинка читается сама собой: вошло слева, вышло справа.
                 << Param::integer(QStringLiteral("runs"), tpl::tr("Прогонов", "Runs"), 3, 2, 8)
                 << Param::number(QStringLiteral("run"), tpl::tr("Длина прогона", "Run length"),
                                  tpl::tr("длин волны", "wavelengths"), 2.0, 0.5, 8.0, 0.5, 1);
        s.build = buildDelay;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ms_taper");
        s.section = sec;
        s.title = tpl::tr("Плавный переход (тейпер)", "Smooth transition (taper)");
        s.summary = tpl::tr("Экспоненциальный переход шире по полосе, чем λ/4",
                            "An exponential taper is more broadband than lambda/4");
        s.standard = tpl::tr("Поузар, «Microwave Engineering», гл. 5; ",
                             "Pozar, \"Microwave Engineering\", ch. 5; ") + ref;
        s.help = tpl::tr(
                     "Согласование двух сопротивлений плавным переходом: Z(x) растёт "
                     "экспоненциально по длине. У такого перехода отражение падает как "
                     "sin(βL)/(βL) — монотонно с длиной и без резонансов, в отличие от "
                     "четвертьволнового трансформатора. Сравните с ним на тех же Z1 и Z2.",
                     "Matching two impedances with a smooth transition: Z(x) grows exponentially "
                     "along the length. The reflection of such a taper falls as sin(beta L)/(beta "
                     "L), monotonically with length and without resonances, unlike a quarter-wave "
                     "transformer. Compare it with one at the same Z1 and Z2.")
                 + helpPlanar();
        s.params = boardParams();
        s.params << freqParam(3.0)
                 << Param::number(QStringLiteral("z1"), QStringLiteral("Z1"), tpl::tr("Ом", "Ohm"),
                                  50.0, 20.0, 120.0, 5.0, 1)
                 << Param::number(QStringLiteral("z2"), QStringLiteral("Z2"), tpl::tr("Ом", "Ohm"),
                                  100.0, 20.0, 120.0, 5.0, 1)
                 << Param::number(QStringLiteral("len"), tpl::tr("Длина перехода", "Taper length"),
                                  tpl::tr("длин волны", "wavelengths"), 1.0, 0.25, 4.0, 0.25, 2)
                 << Param::integer(QStringLiteral("steps"), tpl::tr("Ступеней", "Steps"), 12, 4, 40);
        s.build = buildTaper;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ms_patch");
        s.section = sec;
        s.title = tpl::tr("Патч-антенна: полость и излучение", "Patch antenna: cavity and radiation");
        s.summary = tpl::tr("Магнитные стенки или открытые щели; рамка поглотителя",
                            "Magnetic walls or open slots; absorber frame");
        s.standard = tpl::tr("Баланис, модель резонатора патча; ", "Balanis, patch cavity model; ") + ref;
        s.help = tpl::tr(
                     "Прямоугольная патч-антенна в двумерной модели, питаемая торцом. "
                     "Переключатель краёв даёт ДВЕ разные задачи, и обе честные.\n\n"
                     "«Магнитные стенки» — замкнутая полость: вдоль патча стоит полуволновый "
                     "резонанс, и его частоту даёт формула. Так её и считают (модель "
                     "резонатора патча), а излучение добавляют потом поправкой.\n\n"
                     "«Открытые края» — щели излучают в воздух, и рамка поглотителя вокруг "
                     "области играет свободное пространство. ⚠ Резонатора вдоль патча при "
                     "этом НЕТ: провалы |S11| ставит линия питания со щелью. Проверено "
                     "прогоном — провалы вышли на 2.475 и 3.825 ГГц при расчётном резонансе "
                     "2.98 ГГц.\n\n"
                     "В замкнутой полости резонанс есть: ядро на умолчаниях дало узкий провал "
                     "-12.8 дБ на 3.30 ГГц против расчётных 2.98 — на 11 % ВЫШЕ формулы. "
                     "Разницу вносит щель под питание в торцевой стенке: полость замкнута не "
                     "до конца.\n\n"
                     "⚠ Длина патча взята по формуле БЕЗ поправки на удлинение краёв. У "
                     "настоящего патча эта поправка опускает резонанс, а двумерная модель её "
                     "не содержит: сравнивать её числа с реальной антенной напрямую нельзя.",
                     "A rectangular patch antenna in a 2D model, end-fed. The edge switch gives TWO "
                     "different tasks, and both are honest.\n\n\"Magnetic walls\" is a closed "
                     "cavity: a half-wave resonance stands along the patch, and its frequency is "
                     "given by a formula. That is how it is computed (the patch cavity model), and "
                     "radiation is added later as a correction.\n\n\"Open edges\": the slots "
                     "radiate into the air, and the absorber frame around the domain plays the role "
                     "of free space. (!) There is NO resonator along the patch in this case: the "
                     "|S11| dips are set by the feed line with the slot. Checked by a run: the dips "
                     "came out at 2.475 and 3.825 GHz with the calculated resonance at 2.98 "
                     "GHz.\n\nIn the closed cavity the resonance exists: the kernel at the defaults "
                     "gave a narrow dip of -12.8 dB at 3.30 GHz versus the calculated 2.98, i.e. 11 "
                     "% ABOVE the formula. The difference comes from the feed slot in the end wall: "
                     "the cavity is not completely closed.\n\n(!) The patch length is taken from "
                     "the formula WITHOUT the edge-extension correction. In a real patch this "
                     "correction lowers the resonance, but the 2D model does not contain it: its "
                     "numbers cannot be compared directly with a real antenna.")
                 + helpPlanar();
        s.params = boardParams();
        s.params << Param::number(QStringLiteral("fr"), tpl::tr("Резонансная частота", "Resonant frequency"),
                                  tpl::tr("ГГц", "GHz"), 3.0, 0.5, 15.0, 0.1, 2)
                 << Param::choice(QStringLiteral("edges"), tpl::tr("Края патча", "Patch edges"),
                                  {tpl::tr("Магнитные стенки (резонатор)", "Magnetic walls (resonator)"),
                                   tpl::tr("Открытые (излучение)", "Open (radiation)")},
                                  0)
                 << z0Param(tpl::tr("Сопротивление питающей линии", "Feed line impedance"), 50.0)
                 << Param::number(QStringLiteral("air"), tpl::tr("Воздух вокруг", "Air around"),
                                  tpl::tr("длин волны", "wavelengths"), 0.3, 0.1, 1.0, 0.05, 2)
                 << Param::integer(QStringLiteral("frame"),
                                   tpl::tr("Слоёв поглотителя в рамке",
                                           "Absorber layers in the frame"), 5, 1, 20);
        s.build = buildPatch;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ms_bsf");
        s.section = sec;
        s.title = tpl::tr("ПЗФ на разомкнутых шлейфах", "Band-stop filter on open stubs");
        s.summary = tpl::tr("Полоса заграждения; считается свипом или одним проходом",
                            "Stopband; computed by a sweep or in one pass");
        s.standard = ref;
        s.help = tpl::tr(
                     "Несколько разомкнутых шлейфов λ/4, расставленных через λ/4 по линии. "
                     "Каждый в одиночку даёт узкий провал (см. «Режекторный шлейф»), а вместе "
                     "они образуют полосу заграждения: чем больше шлейфов, тем шире и глубже.\n\n"
                     "Этим же заданием удобно сравнить два способа расчёта. Свип считает 41 "
                     "шаг, по одному на частоту. Гребёнка тонов кладёт все 41 частоту в один "
                     "файл падающей волны и считается ОДИН раз — ровно столько же, сколько "
                     "один шаг свипа.",
                     "Several open lambda/4 stubs placed lambda/4 apart along the line. Each one "
                     "alone gives a narrow dip (see \"Band-stop stub\"), and together they form a "
                     "stopband: the more stubs, the wider and deeper it is.\n\nThe same task is "
                     "convenient for comparing two calculation methods. The sweep computes 41 "
                     "steps, one per frequency. The tone comb puts all 41 frequencies into one "
                     "incident-wave file and is computed ONCE, taking exactly as long as one sweep "
                     "step.")
                 + helpPlanar();
        s.params = boardParams();
        s.params << z0Param(tpl::tr("Сопротивление линии", "Line impedance"), 50.0) << freqParam(3.0)
                 << Param::integer(QStringLiteral("stubs"), tpl::tr("Шлейфов", "Stubs"), 3, 1, 7)
                 << Param::number(QStringLiteral("zs"), tpl::tr("Сопротивление шлейфов", "Stub impedance"),
                                  tpl::tr("Ом", "Ohm"), 70.0, 20.0, 120.0, 5.0, 1)
                 << Param::choice(QStringLiteral("mode"), tpl::tr("Способ расчёта", "Calculation method"),
                                  {tpl::tr("Свип по частотам (41 шаг)", "Frequency sweep (41 steps)"),
                                   tpl::tr("Один проход: гребёнка тонов", "One pass: tone comb")},
                                  0)
                 << corrParam(17.0);
        s.build = buildBsf;
        *out << s;
    }
}

} // namespace tpl::samples
