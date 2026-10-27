// Раздел «Микрополосковые»: планарная волноводная модель.
//
// Общие детали модели (Line, Board, стенки, порты, подсказки) живут в msplanar.h —
// их делят между собой оба файла рецептов раздела. Пояснение, что это за модель и
// чего ею выразить нельзя, тоже там.

/// @file microstrip.cpp
/// @brief Раздел каталога «Микрополосковые», первая половина: линия, шлейф, ФНЧ, шлейфовый мост, трансформатор.
///
/// Часть модуля «Ядро TPL Designer», каталог образцов. Планарная волноводная модель
/// (`msplanar.h`), H-ядро, входы плоской волны; регистрация — `addMicrostrip`, она же
/// вызывает `addMicrostrip2`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#samples
#include "msplanar.h"

#include "lang.h"
#include "physics.h"

#include <cmath>

namespace tpl::samples {

namespace {

using namespace msp;

// ------------------------------------------------------------ линия

/// @brief Рецепт «Микрополосковая линия» (`ms_line`).
///
/// Синтез ширины по волновому сопротивлению.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `sub`, `h` — подложка и толщина, мм (см. `msp::boardParams`);
///                `z0` — волновое сопротивление, Ом;
///                `len` — длина, в длинах волны в линии;
///                `f0` — частота, ГГц
/// @return Задание, паспорт «теория» и пояснения.
Build buildLine(const Values &v)
{
    const Board bd = board(v);
    const double f0 = v.value(QStringLiteral("f0"));
    const Line l = bd.line(v.value(QStringLiteral("z0")));

    Builder b(QStringLiteral("msline"));
    b.p.delta = deltaFitting(l.weff, gridFor(l, f0));
    const double d = b.p.delta;
    const double L = snap(v.value(QStringLiteral("len")) * l.lambdaG(f0), d);
    const double ylo = 3 * d, yhi = ylo + l.weff;
    b.p.box = {0.0, L, 0.0, yhi + 3 * d};
    b.p.description = tpl::tr("Микрополосковая линия %1 Ом на %2 (планарная модель).",
                              "Microstrip line %1 Ohm on %2 (planar model).")
                          .arg(num(l.z0), bd.sub.name);
    b.polyline(tpl::tr("край полоски, низ", "strip edge, bottom"), Material::Magnetic,
               {QPointF(0, ylo), QPointF(L / 2, ylo), QPointF(L, ylo)});
    b.polyline(tpl::tr("край полоски, верх", "strip edge, top"), Material::Magnetic,
               {QPointF(0, yhi), QPointF(L / 2, yhi), QPointF(L, yhi)});
    fill(b, tpl::tr("линия", "line"), l, 0.0, L, ylo, yhi);
    msPort(b, tpl::tr("вход 1", "port 1"), PortSide::Left, ylo, yhi, true, l);
    msPort(b, tpl::tr("вход 2", "port 2"), PortSide::Right, ylo, yhi, false, l);
    // Вход плоской волны тем точнее, чем шире полоска относительно волны, поэтому свип
    // не опускается ниже 0.6 f0 (проверено прогоном: на 0.33 f0 остаток входа -10 дБ).
    b.freqs(std::round(f0 * 0.6 * 100) / 100, std::round(f0 * 1.6 * 100) / 100, 21);
    b.outputs(true);
    b.time(settleTime(L * std::sqrt(l.epsEff), f0, 0.0));

    QVector<Row> rows = boardRows(bd);
    rows << lineRow(tpl::tr("Линия %1 Ом", "Line %1 Ohm").arg(num(l.z0)), l, f0);
    rows << Row{tpl::tr("Проверка синтеза", "Synthesis check"),
                tpl::tr("Z0(w) = %1 Ом", "Z0(w) = %1 Ohm").arg(num(rf::msZ0(l.w / bd.h, bd.sub.eps), 4))};
    rows << Row{tpl::tr("Длина", "Length"), tpl::tr("%1 (%2 λв)",
                                                 "%1 (%2 lambda_g)").arg(mm(L), num(L / l.lambdaG(f0)))};
    rows << Row{tpl::tr("Ожидаемо", "Expected"),
                tpl::tr("|S21| около 0 дБ, |S11| на уровне остатка входа (около -25 дБ)",
                        "|S21| about 0 dB, |S11| at the level of the port residual (about -25 dB)")};
    return b.done(rows);
}

// ------------------------------------------------------------ шлейф

/// @brief Рецепт «Режекторный шлейф lambda/4» (`ms_stub`).
///
/// Разомкнутый шлейф вырезает частоту из полосы.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `sub`, `h` — подложка и толщина, мм (см. `msp::boardParams`);
///                `z0` — сопротивление линии, Ом;
///                `corr` — поправка длин на соединения, %;
///                `f0` — частота режекции, ГГц
/// @return Задание, паспорт «теория» и пояснения.
Build buildStub(const Values &v)
{
    const Board bd = board(v);
    const double f0 = v.value(QStringLiteral("f0"));
    const Line l = bd.line(v.value(QStringLiteral("z0")));

    Builder b(QStringLiteral("msstub"));
    b.p.delta = deltaFitting(l.weff, gridFor(l, f0));
    const double d = b.p.delta;
    const double lg = l.lambdaG(f0);
    const double lead = snap(lg, d);
    const double xs = lead + snap(l.weff / 2.0, d); // ось шлейфа
    const double L = 2 * xs;
    const double ylo = 3 * d, yhi = ylo + l.weff;
    // Длина шлейфа — λв/4 от оси основной линии (с поправкой на тройник), то есть от
    // её края на W_eff/2 меньше.
    const double corr = v.value(QStringLiteral("corr")) / 100.0;
    const double stubLen = snap(lg / 4.0 * (1.0 + corr) - l.weff / 2.0, d);
    const double ytop = yhi + stubLen;
    const double sx0 = xs - l.weff / 2.0, sx1 = xs + l.weff / 2.0;
    b.p.box = {0.0, L, 0.0, ytop + 3 * d};
    b.p.description = tpl::tr("Разомкнутый шлейф lambda/4 на %1 ГГц, линия %2 Ом, %3.",
                              "Open-circuited stub lambda/4 at %1 GHz, line %2 Ohm, %3.")
                          .arg(num(f0, 3), num(l.z0), bd.sub.name);

    b.polyline(tpl::tr("край полоски, низ", "strip edge, bottom"), Material::Magnetic,
               {QPointF(0, ylo), QPointF(xs, ylo), QPointF(L, ylo)});
    b.polyline(tpl::tr("край полоски и шлейфа", "strip and stub edge"), Material::Magnetic,
               {QPointF(0, yhi), QPointF(sx0, yhi), QPointF(sx0, ytop), QPointF(sx1, ytop),
                QPointF(sx1, yhi), QPointF(L, yhi)});
    fill(b, tpl::tr("линия", "line"), l, 0.0, L, ylo, yhi);
    fill(b, tpl::tr("шлейф", "stub"), l, sx0, sx1, yhi, ytop);
    msPort(b, tpl::tr("вход 1", "port 1"), PortSide::Left, ylo, yhi, true, l);
    msPort(b, tpl::tr("вход 2", "port 2"), PortSide::Right, ylo, yhi, false, l);
    b.freqs(std::round(f0 * 0.5 * 100) / 100, std::round(f0 * 1.5 * 100) / 100, 41);
    b.outputs(true);
    b.time(settleTime((L + 2 * stubLen) * std::sqrt(l.epsEff), f0, 0.0));

    QVector<Row> rows = boardRows(bd);
    rows << lineRow(tpl::tr("Линия и шлейф %1 Ом", "Line and stub %1 Ohm").arg(num(l.z0)), l, f0);
    rows << Row{tpl::tr("Шлейф", "Stub"),
                tpl::tr("%1 от края линии (λв/4 от оси, поправка %2 %)",
                        "%1 from the line edge (lambda_g/4 from the axis, correction %2 %)")
                    .arg(mm(stubLen), num(corr * 100.0))};
    rows << Row{tpl::tr("Ожидаемо", "Expected"),
                tpl::tr("провал |S21| на %1 ГГц: разомкнутый λ/4 даёт короткое замыкание",
                        "|S21| dip at %1 GHz: an open lambda/4 gives a short circuit")
                    .arg(num(f0, 3))};
    return b.done(rows, {tpl::tr("Конец шлейфа — магнитная стенка: в планарной модели это "
                                 "и есть холостой ход.",
                                 "The end of the stub is a magnetic wall: in the planar model this "
                                 "is exactly an open circuit."),
                         tpl::tr("Без поправки (0 %) провал ядра приходится примерно на "
                                 "1.2 f0: широкий тройник укорачивает шлейф. Поправка 17 % "
                                 "подобрана прогоном ядра для умолчаний.",
                                 "Without correction (0 %) the kernel's dip is at about 1.2 f0: the "
                                 "wide tee shortens the stub. The 17 % correction was tuned by a "
                                 "kernel run for the defaults.")});
}

// ------------------------------------------------------------ ФНЧ

/// @brief Рецепт «ФНЧ на ступенях импеданса» (`ms_lpf`).
///
/// Фильтр нижних частот из чередующихся широких и узких отрезков.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `sub`, `h` — подложка и толщина, мм (см. `msp::boardParams`);
///                `fc` — частота среза, ГГц;
///                `order` — порядок;
///                `type` — прототип: 0 — Баттерворт, 1 — Чебышёв 0.1 дБ;
///                `zh` — сопротивление высокоомного отрезка, Ом;
///                `zl` — сопротивление низкоомного отрезка, Ом;
///                `corr` — поправка длин на соединения, %
/// @return Задание, паспорт «теория» и пояснения.
Build buildLpf(const Values &v)
{
    const Board bd = board(v);
    const double fc = v.value(QStringLiteral("fc"));
    const int n = int(v.value(QStringLiteral("order")));
    const bool cheb = v.value(QStringLiteral("type")) > 0.5;
    const Line l0 = bd.line(50.0);
    const Line lh = bd.line(v.value(QStringLiteral("zh")));
    const Line ll = bd.line(v.value(QStringLiteral("zl")));
    const QVector<double> g = cheb ? rf::chebyshev(n, 0.1) : rf::butterworth(n);
    const double corr = v.value(QStringLiteral("corr")) / 100.0;

    Builder b(QStringLiteral("mslpf"));
    b.p.delta = chooseDelta(rf::lambda0(2 * fc) / std::sqrt(ll.epsEff), lh.weff, 20.0, 8.0);
    const double d = b.p.delta;
    const double wMax = std::max({l0.weff, lh.weff, ll.weff});
    const double yc = snap(3 * d + wMax / 2.0, d);

    // Участки: питание, C-L-C-..., питание. Нечётные элементы — ёмкости (широкие).
    struct Part { Line line; double len; QString name; };
    QVector<Part> parts;
    const double feed = snap(0.5 * l0.lambdaG(fc), d);
    parts << Part{l0, feed, tpl::tr("питание 1", "feed 1")};
    QVector<Row> lens;
    for (int k = 1; k <= n; ++k) {
        const bool cap = (k % 2 == 1);
        const Line &ln = cap ? ll : lh;
        const double bl = cap ? g[k] * ll.z0 / 50.0 : g[k] * 50.0 / lh.z0; // электрическая длина, рад
        const double len = std::max(d, snap(bl * ln.lambdaG(fc) / (2 * kPi) * (1.0 + corr), d));
        parts << Part{ln, len, QStringLiteral("%1 %2").arg(cap ? tpl::tr("ёмкость", "capacitance")
                                                               : tpl::tr("индуктивность",
                                                                         "inductance")).arg(k)};
        lens << Row{tpl::tr("Элемент %1 (%2 Ом)", "Element %1 (%2 Ohm)").arg(k).arg(num(ln.z0)),
                    tpl::tr("g = %1, длина %2, W_eff %3", "g = %1, length %2, W_eff %3")
                        .arg(num(g[k], 4), mm(len), mm(ln.weff))};
    }
    parts << Part{l0, feed, tpl::tr("питание 2", "feed 2")};

    QVector<QPointF> lower, upper;
    double x = 0.0;
    for (int i = 0; i < parts.size(); ++i) {
        const Part &p = parts.at(i);
        const double h = p.line.weff / 2.0;
        lower << QPointF(x, yc - h) << QPointF(x + p.len, yc - h);
        upper << QPointF(x, yc + h) << QPointF(x + p.len, yc + h);
        fill(b, p.name, p.line, x, x + p.len, yc - h, yc + h);
        x += p.len;
    }
    const double L = x;
    b.p.box = {0.0, L, 0.0, 2 * yc};
    b.p.description = tpl::tr("ФНЧ на ступенях импеданса: %1, n = %2, fc = %3 ГГц, %4.",
                              "Stepped-impedance LPF: %1, n = %2, fc = %3 GHz, %4.")
                          .arg(cheb ? tpl::tr("Чебышёв 0.1 дБ",
                                              "Chebyshev 0.1 dB") : tpl::tr("Баттерворт", "Butterworth"))
                          .arg(n)
                          .arg(num(fc, 3), bd.sub.name);
    b.polyline(tpl::tr("край полоски, низ", "strip edge, bottom"), Material::Magnetic, lower);
    b.polyline(tpl::tr("край полоски, верх", "strip edge, top"), Material::Magnetic, upper);
    msPort(b, tpl::tr("вход 1", "port 1"), PortSide::Left, yc - l0.weff / 2, yc + l0.weff / 2, true, l0);
    msPort(b, tpl::tr("вход 2", "port 2"), PortSide::Right, yc - l0.weff / 2, yc + l0.weff / 2, false, l0);
    b.freqs(std::round(fc * 0.2 * 100) / 100, std::round(fc * 2.0 * 100) / 100, 37);
    b.outputs(true);
    b.time(settleTime(L * std::sqrt(ll.epsEff), fc * 0.2, 0.0, 3.0 * n / (kPi * fc)));

    QVector<Row> rows = boardRows(bd);
    rows << lineRow(tpl::tr("Питание 50 Ом", "Feed 50 Ohm"), l0, fc);
    rows << Row{tpl::tr("Прототип", "Prototype"),
                cheb ? tpl::tr("Чебышёв, пульсации 0.1 дБ, n = %1", "Chebyshev, ripple 0.1 dB, n = %1").arg(n)
                     : tpl::tr("Баттерворт (максимально плоский), n = %1",
                               "Butterworth (maximally flat), n = %1").arg(n)};
    rows += lens;
    rows << Row{tpl::tr("Ожидаемо", "Expected"),
                cheb ? tpl::tr("|S21| >= -0.1 дБ до %1 ГГц, дальше быстрый спад",
                               "|S21| >= -0.1 dB up to %1 GHz, then a fast roll-off").arg(num(fc, 3))
                     : tpl::tr("|S21| = -3 дБ на %1 ГГц", "|S21| = -3 dB at %1 GHz").arg(num(fc, 3))};
    rows << Row{tpl::tr("Поправка длин", "Length correction"), QStringLiteral("%1 %").arg(num(corr * 100.0))};
    return b.done(rows, {tpl::tr("Высокоомные отрезки работают как индуктивности, "
                                 "низкоомные — как ёмкости (приближение коротких "
                                 "отрезков, Поузар).",
                                 "High-impedance segments act as inductances, low-impedance ones as "
                                 "capacitances (short-segment approximation, Pozar)."),
                         tpl::tr("Ступеньки ширины добавляют ёмкость, а широкие отрезки не "
                                 "«короткие»: без поправки (0 %) срез ядра примерно на 22 % "
                                 "ниже расчётного. Поправка -22 % подобрана прогоном ядра.",
                                 "Width steps add capacitance, and the wide segments are not "
                                 "\"short\": without correction (0 %) the kernel's cutoff is about "
                                 "22 % below the design. The -22 % correction was tuned by a kernel "
                                 "run.")});
}

// ------------------------------------------------------------ шлейфовый ответвитель

/// @brief Рецепт «Шлейфовый ответвитель 3 дБ» (`ms_branchline`).
///
/// Квадратурный мост: деление пополам со сдвигом 90°.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `sub`, `h` — подложка и толщина, мм (см. `msp::boardParams`);
///                `f0` — рабочая частота, ГГц;
///                `corr` — поправка длин на соединения, %
/// @return Задание, паспорт «теория» и пояснения.
Build buildBranchline(const Values &v)
{
    const Board bd = board(v);
    const double f0 = v.value(QStringLiteral("f0"));
    const Line l50 = bd.line(50.0);
    const Line l35 = bd.line(50.0 / std::sqrt(2.0));

    Builder b(QStringLiteral("branch"));
    b.p.delta = gridFor(l50, f0);
    const double d = b.p.delta;
    const double hv = l50.weff / 2.0, hb = l35.weff / 2.0, hf = l50.weff / 2.0;
    // Между осями плеч — λв/4 с поправкой на соединения: у широких полосок модели
    // квадраты соединений электрически короче своих размеров.
    const double corr = v.value(QStringLiteral("corr")) / 100.0;
    const double Lh = snap(l35.lambdaG(f0) / 4.0 * (1.0 + corr), d); // между осями вертикальных плеч
    const double Lv = snap(l50.lambdaG(f0) / 4.0 * (1.0 + corr), d); // между осями горизонтальных плеч
    const double feed = snap(0.5 * l50.lambdaG(f0), d);
    const double xl = snap(feed + hv, d), xr = xl + Lh;
    const double yb = snap(3 * d + hb, d), yt = yb + Lv;
    const double X = xr + hv + feed;
    const double Y = yt + hb + 3 * d;
    b.p.box = {0.0, X, 0.0, Y};
    b.p.description = tpl::tr("Шлейфовый направленный ответвитель 3 дБ на %1 ГГц, %2.",
                              "Branch-line 3 dB directional coupler at %1 GHz, %2.")
                          .arg(num(f0, 3), bd.sub.name);

    // Магнитные стенки — граница объединения плеч и питающих линий. У входов (x = 0 и
    // x = X) стенок нет: там апертуры.
    b.polyline(tpl::tr("край, низ", "edge, bottom"), Material::Magnetic,
               {QPointF(0, yb - hf), QPointF(xl - hv, yb - hf), QPointF(xl - hv, yb - hb),
                QPointF(xr + hv, yb - hb), QPointF(xr + hv, yb - hf), QPointF(X, yb - hf)});
    b.polyline(tpl::tr("край, справа", "edge, right"), Material::Magnetic,
               {QPointF(X, yb + hf), QPointF(xr + hv, yb + hf), QPointF(xr + hv, yt - hf),
                QPointF(X, yt - hf)});
    b.polyline(tpl::tr("край, верх", "edge, top"), Material::Magnetic,
               {QPointF(X, yt + hf), QPointF(xr + hv, yt + hf), QPointF(xr + hv, yt + hb),
                QPointF(xl - hv, yt + hb), QPointF(xl - hv, yt + hf), QPointF(0, yt + hf)});
    b.polyline(tpl::tr("край, слева", "edge, left"), Material::Magnetic,
               {QPointF(0, yt - hf), QPointF(xl - hv, yt - hf), QPointF(xl - hv, yb + hf),
                QPointF(0, yb + hf)});
    b.rect(tpl::tr("окно в середине",
                   "window in the middle"), Material::Magnetic, xl + hv, xr - hv, yb + hb, yt - hb);

    fill(b, tpl::tr("питание 1", "feed 1"), l50, 0.0, xl - hv, yb - hf, yb + hf);
    fill(b, tpl::tr("питание 4", "feed 4"), l50, 0.0, xl - hv, yt - hf, yt + hf);
    fill(b, tpl::tr("питание 2", "feed 2"), l50, xr + hv, X, yb - hf, yb + hf);
    fill(b, tpl::tr("питание 3", "feed 3"), l50, xr + hv, X, yt - hf, yt + hf);
    fill(b, tpl::tr("плечо 35 Ом, низ", "arm 35 Ohm, bottom"), l35, xl - hv, xr + hv, yb - hb, yb + hb);
    fill(b, tpl::tr("плечо 35 Ом, верх", "arm 35 Ohm, top"), l35, xl - hv, xr + hv, yt - hb, yt + hb);
    fill(b, tpl::tr("плечо 50 Ом, лево", "arm 50 Ohm, left"), l50, xl - hv, xl + hv, yb + hb, yt - hb);
    fill(b, tpl::tr("плечо 50 Ом, право", "arm 50 Ohm, right"), l50, xr - hv, xr + hv, yb + hb, yt - hb);

    msPort(b, tpl::tr("вход 1", "port 1"), PortSide::Left, yb - hf, yb + hf, true, l50);
    msPort(b, tpl::tr("вход 2 (прямой)", "port 2 (through)"), PortSide::Right, yb - hf, yb + hf, false, l50);
    msPort(b, tpl::tr("вход 3 (связанный)",
                      "port 3 (coupled)"), PortSide::Right, yt - hf, yt + hf, false, l50);
    msPort(b, tpl::tr("вход 4 (развязанный)",
                      "port 4 (isolated)"), PortSide::Left, yt - hf, yt + hf, false, l50);
    b.freqs(std::round(f0 * 0.6 * 100) / 100, std::round(f0 * 1.4 * 100) / 100, 21);
    b.outputs(true);
    b.time(settleTime((X + 2 * Lv) * std::sqrt(l35.epsEff), f0, 0.0, 5.0));

    QVector<Row> rows = boardRows(bd);
    rows << lineRow(tpl::tr("Плечи 50 Ом", "Arms 50 Ohm"), l50, f0);
    rows << lineRow(tpl::tr("Плечи 35.4 Ом (Z0/√2)", "Arms 35.4 Ohm (Z0/sqrt(2))"), l35, f0);
    rows << Row{tpl::tr("Кольцо между осями",
                        "Ring between axes"), QStringLiteral("%1 x %2").arg(mm(Lh), mm(Lv))};
    rows << Row{tpl::tr("Ожидаемо на f0", "Expected at f0"),
                tpl::tr("|S21| = |S31| = -3 дБ, сдвиг фаз 90°, |S11| и |S41| — минимум",
                        "|S21| = |S31| = -3 dB, phase shift 90 deg, |S11| and |S41| at minimum")};
    rows << Row{tpl::tr("Поправка длин", "Length correction"), QStringLiteral("%1 %").arg(num(corr * 100.0))};
    return b.done(rows, {tpl::tr("Возбуждается вход 1 — в .s столбец S11, S21, S31, S41: "
                                 "для ответвителя этого достаточно, остальное даёт "
                                 "симметрия.",
                                 "Port 1 is excited, so the .s file holds the column S11, S21, S31, "
                                 "S41: this is enough for a coupler, symmetry gives the rest."),
                         tpl::tr("Без поправки (0 %) центр рабочей полосы ядра выше f0 "
                                 "примерно на 18 %: соединения широких плеч электрически "
                                 "короче. Поправка 18 % подобрана прогоном ядра.",
                                 "Without correction (0 %) the center of the kernel's operating "
                                 "band is about 18 % above f0: the junctions of the wide arms are "
                                 "electrically shorter. The 18 % correction was tuned by a kernel "
                                 "run.")});
}

// ------------------------------------------------------------ трансформатор

/// @brief Рецепт «Четвертьволновый трансформатор» (`ms_qwt`).
///
/// Согласование двух сопротивлений отрезком lambda/4.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `sub`, `h` — подложка и толщина, мм (см. `msp::boardParams`);
///                `z1`, `z2` — согласуемые сопротивления, Ом;
///                `f0` — рабочая частота, ГГц;
///                `corr` — поправка длин на соединения, %
/// @return Задание, паспорт «теория» и пояснения.
Build buildQwt(const Values &v)
{
    const Board bd = board(v);
    const double f0 = v.value(QStringLiteral("f0"));
    const double z1 = v.value(QStringLiteral("z1")), z2 = v.value(QStringLiteral("z2"));
    const Line l1 = bd.line(z1), l2 = bd.line(z2), lt = bd.line(std::sqrt(z1 * z2));

    Builder b(QStringLiteral("msqwt"));
    const Line &narrow = (l1.weff < l2.weff) ? l1 : l2;
    b.p.delta = gridFor(narrow, f0);
    const double d = b.p.delta;
    const double wMax = std::max({l1.weff, l2.weff, lt.weff});
    const double yc = snap(3 * d + wMax / 2.0, d);
    const double feed1 = snap(l1.lambdaG(f0), d), feed2 = snap(l2.lambdaG(f0), d);
    // Поправка длины — для подстройки, а не для лечения ошибки: при 0 расчёт идёт по
    // осям, как в учебнике, и это ПРАВИЛЬНО. Прогон при умолчаниях даёт на f0
    // |S11| = -22.7 дБ, то есть ровно уровень остаточного отражения входа плоской
    // волны (-25 дБ) — глубже планарная модель не покажет при любом измельчении сетки.
    //
    // ⚠️ Глобальный минимум |S11| при этом приходится не на f0, а на соседнюю точку
    // свипа: кривая изрезана рябью от переотражений между ступеньками и подводящими
    // линиями длиной λв. Судить о настройке по положению глобального минимума здесь
    // нельзя — смотреть надо уровень НА f0.
    const double corr = v.value(QStringLiteral("corr")) / 100.0;
    const double qt = snap(lt.lambdaG(f0) / 4.0 * (1.0 + corr), d);
    const double x1 = feed1, x2 = feed1 + qt, L = x2 + feed2;
    b.p.box = {0.0, L, 0.0, 2 * yc};
    b.p.description = tpl::tr("Четвертьволновый трансформатор %1 -> %2 Ом на %3 ГГц, %4.",
                              "Quarter-wave transformer %1 -> %2 Ohm at %3 GHz, %4.")
                          .arg(num(z1), num(z2), num(f0, 3), bd.sub.name);
    const auto edge = [&](double s) {
        return QVector<QPointF>{QPointF(0, yc + s * l1.weff / 2), QPointF(x1, yc + s * l1.weff / 2),
                                QPointF(x1, yc + s * lt.weff / 2), QPointF(x2, yc + s * lt.weff / 2),
                                QPointF(x2, yc + s * l2.weff / 2), QPointF(L, yc + s * l2.weff / 2)};
    };
    b.polyline(tpl::tr("край полоски, низ", "strip edge, bottom"), Material::Magnetic, edge(-1.0));
    b.polyline(tpl::tr("край полоски, верх", "strip edge, top"), Material::Magnetic, edge(+1.0));
    fill(b, tpl::tr("линия %1 Ом",
                    "line %1 Ohm").arg(num(z1)), l1, 0, x1, yc - l1.weff / 2, yc + l1.weff / 2);
    fill(b, tpl::tr("трансформатор", "transformer"), lt, x1, x2, yc - lt.weff / 2, yc + lt.weff / 2);
    fill(b, tpl::tr("линия %1 Ом",
                    "line %1 Ohm").arg(num(z2)), l2, x2, L, yc - l2.weff / 2, yc + l2.weff / 2);
    msPort(b, tpl::tr("вход 1", "port 1"), PortSide::Left, yc - l1.weff / 2, yc + l1.weff / 2, true, l1);
    msPort(b, tpl::tr("вход 2", "port 2"), PortSide::Right, yc - l2.weff / 2, yc + l2.weff / 2, false, l2);
    b.freqs(std::round(f0 * 0.4 * 100) / 100, std::round(f0 * 1.6 * 100) / 100, 25);
    b.outputs(true);
    b.time(settleTime(L * std::sqrt(lt.epsEff), f0 * 0.4, 0.0));

    // Точная линия передачи: Zin трансформатора, нагруженного на Z2.
    const auto gamma = [&](double f) {
        const double th = kPi / 2.0 * f / f0;
        const rf::cplx j(0.0, 1.0);
        const double zt = lt.z0;
        const rf::cplx zin = zt * (z2 + j * zt * std::tan(th)) / (zt + j * z2 * std::tan(th));
        return std::abs((zin - z1) / (zin + z1));
    };
    const double gm = 0.1;
    const double arg = gm / std::sqrt(1 - gm * gm) * 2.0 * std::sqrt(z1 * z2) / std::fabs(z2 - z1);
    const double bw = arg < 1.0 ? 2.0 - 4.0 / kPi * std::acos(arg) : 2.0;

    QVector<Row> rows = boardRows(bd);
    rows << lineRow(tpl::tr("Линия %1 Ом", "Line %1 Ohm").arg(num(z1)), l1, f0);
    rows << lineRow(tpl::tr("Трансформатор %1 Ом", "Transformer %1 Ohm").arg(num(lt.z0)), lt, f0);
    rows << lineRow(tpl::tr("Линия %1 Ом", "Line %1 Ohm").arg(num(z2)), l2, f0);
    rows << Row{tpl::tr("Длина трансформатора", "Transformer length"),
                tpl::tr("%1 (поправка %2 %)", "%1 (correction %2 %)").arg(mm(qt), num(corr * 100.0))};
    for (double k : {0.5, 1.0, 1.5})
        rows << Row{tpl::tr("Теория ЛП, %1 ГГц", "Transmission-line theory, %1 GHz").arg(num(k * f0, 3)),
                    QStringLiteral("|S11| = %1").arg(db(gamma(k * f0)))};
    rows << Row{tpl::tr("Полоса по |S11| < -20 дБ", "Band for |S11| < -20 dB"),
                tpl::tr("%1 % от f0", "%1 % of f0").arg(num(bw * 100.0))};
    rows << Row{tpl::tr("Без трансформатора", "Without transformer"),
                QStringLiteral("|S11| = %1").arg(db(std::fabs(z2 - z1) / (z2 + z1)))};
    return b.done(rows);
}

} // namespace

void addMicrostrip(QVector<Sample> *out)
{
    const QString sec = QStringLiteral("microstrip");
    const QString ref = tpl::tr("Хаммерстад-Йенсен; планарная модель Весселя-Кольхоффа",
                                "Hammerstad-Jensen; Wessel-Kohlhoff planar model");
    {
        Sample s;
        s.id = QStringLiteral("ms_line");
        s.section = sec;
        s.title = tpl::tr("Микрополосковая линия", "Microstrip line");
        s.summary = tpl::tr("Синтез ширины по волновому сопротивлению",
                            "Width synthesis from the characteristic impedance");
        s.standard = ref;
        s.help = tpl::tr(
                     "Отрезок микрополосковой линии заданного сопротивления. Ширина полоски "
                     "считается по формулам Хаммерстада, затем полоска заменяется планарной "
                     "моделью. Образец показывает и саму модель, и то, с какой точностью "
                     "ядро считает вход плоской волны.",
                     "A section of microstrip line with a given impedance. The strip width is "
                     "computed by the Hammerstad formulas, then the strip is replaced by the planar "
                     "model. The sample shows both the model itself and how accurately the kernel "
                     "computes the plane-wave port.")
                 + helpPlanar();
        s.params = boardParams();
        s.params << Param::number(QStringLiteral("z0"), tpl::tr("Волновое сопротивление",
                                                                "Characteristic impedance"),
                                  tpl::tr("Ом", "Ohm"), 50.0, 20.0, 120.0, 5.0, 1)
                 << freqParam(3.0)
                 << Param::number(QStringLiteral("len"), tpl::tr("Длина", "Length"), tpl::tr("λв", "lambda_g"),
                                  2.0, 0.5, 10.0, 0.5, 1);
        s.build = buildLine;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ms_stub");
        s.section = sec;
        s.title = tpl::tr("Режекторный шлейф λ/4", "Band-stop stub lambda/4");
        s.summary = tpl::tr("Разомкнутый шлейф вырезает частоту из полосы",
                            "An open stub cuts a frequency out of the band");
        s.standard = ref;
        s.help = tpl::tr(
                     "Разомкнутый на конце шлейф длиной в четверть волны. На своей частоте он "
                     "превращает холостой ход в короткое замыкание в точке подключения — и "
                     "линия перестаёт пропускать. Простейший режекторный фильтр.",
                     "A quarter-wave stub open at its end. At its frequency it turns the open "
                     "circuit into a short circuit at the junction, and the line stops "
                     "transmitting. The simplest band-stop filter.")
                 + helpPlanar();
        s.params = boardParams();
        s.params << Param::number(QStringLiteral("z0"), tpl::tr("Сопротивление линии", "Line impedance"),
                                  tpl::tr("Ом", "Ohm"), 50.0, 20.0, 120.0, 5.0, 1)
                 << freqParam(3.0) << corrParam(17.0);
        s.build = buildStub;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ms_lpf");
        s.section = sec;
        s.title = tpl::tr("ФНЧ на ступенях импеданса", "Stepped-impedance LPF");
        s.summary = tpl::tr("Фильтр нижних частот из чередующихся широких и узких отрезков",
                            "Low-pass filter of alternating wide and narrow segments");
        s.standard = tpl::tr("Поузар, «Microwave Engineering», гл. 8; ",
                             "Pozar, \"Microwave Engineering\", ch. 8; ") + ref;
        s.help = tpl::tr(
                     "Фильтр нижних частот из коротких отрезков линии: узкие (высокоомные) "
                     "работают как последовательные индуктивности, широкие (низкоомные) — как "
                     "ёмкости на землю. Длины считаются по прототипу Баттерворта или Чебышёва.",
                     "A low-pass filter of short line segments: the narrow (high-impedance) ones "
                     "act as series inductances, the wide (low-impedance) ones as shunt "
                     "capacitances to ground. The lengths are computed from a Butterworth or "
                     "Chebyshev prototype.")
                 + helpPlanar();
        s.params = boardParams();
        s.params << Param::number(QStringLiteral("fc"), tpl::tr("Частота среза", "Cutoff frequency"),
                                  tpl::tr("ГГц", "GHz"), 2.0, 0.3, 20.0, 0.1, 2)
                 << Param::integer(QStringLiteral("order"), tpl::tr("Порядок", "Order"), 5, 1, 9)
                 << Param::choice(QStringLiteral("type"), tpl::tr("Прототип", "Prototype"),
                                  {tpl::tr("Баттерворт", "Butterworth"), tpl::tr("Чебышёв 0.1 дБ",
                                                                         "Chebyshev 0.1 dB")}, 0)
                 << Param::number(QStringLiteral("zh"), tpl::tr("Высокоомный отрезок",
                                                                "High-impedance segment"),
                                  tpl::tr("Ом", "Ohm"), 100.0, 70.0, 150.0, 5.0, 1)
                 << Param::number(QStringLiteral("zl"), tpl::tr("Низкоомный отрезок",
                                                                "Low-impedance segment"),
                                  tpl::tr("Ом", "Ohm"), 20.0, 10.0, 35.0, 1.0, 1)
                 << corrParam(-22.0);
        s.build = buildLpf;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ms_branchline");
        s.section = sec;
        s.title = tpl::tr("Шлейфовый ответвитель 3 дБ", "Branch-line 3 dB coupler");
        s.summary = tpl::tr("Квадратурный мост: деление пополам со сдвигом 90°",
                            "Quadrature hybrid: equal split with a 90 deg shift");
        s.standard = ref;
        s.help = tpl::tr(
                     "Кольцо из четырёх четвертьволновых отрезков: два по Z0/√2 и два по Z0. "
                     "Мощность со входа 1 делится поровну между входами 2 и 3 со сдвигом фаз "
                     "90°, вход 4 развязан. Ширина рабочей полосы — около 10-15%.",
                     "A ring of four quarter-wave segments: two of Z0/sqrt(2) and two of Z0. The "
                     "power from port 1 is split equally between ports 2 and 3 with a 90 deg phase "
                     "shift, port 4 is isolated. The operating bandwidth is about 10-15%.")
                 + helpPlanar();
        s.params = boardParams();
        s.params << freqParam(3.0) << corrParam(18.0);
        s.build = buildBranchline;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ms_qwt");
        s.section = sec;
        s.title = tpl::tr("Четвертьволновый трансформатор", "Quarter-wave transformer");
        s.summary = tpl::tr("Согласование двух сопротивлений отрезком λ/4",
                            "Matching two impedances with a lambda/4 segment");
        s.standard = ref;
        s.help = tpl::tr(
                     "Отрезок линии в четверть волны с сопротивлением √(Z1·Z2) согласует две "
                     "линии разного сопротивления на центральной частоте. Для этой схемы есть "
                     "точная формула теории линий — в паспорте |S11| на трёх частотах и полоса "
                     "согласования.",
                     "A quarter-wave line segment with impedance sqrt(Z1*Z2) matches two lines of "
                     "different impedance at the center frequency. There is an exact "
                     "transmission-line formula for this circuit: the datasheet gives |S11| at "
                     "three frequencies and the matching bandwidth.")
                 + helpPlanar();
        s.params = boardParams();
        s.params << freqParam(3.0)
                 << Param::number(QStringLiteral("z1"), QStringLiteral("Z1"), tpl::tr("Ом", "Ohm"),
                                  50.0, 20.0, 150.0, 5.0, 1)
                 << Param::number(QStringLiteral("z2"), QStringLiteral("Z2"), tpl::tr("Ом", "Ohm"),
                                  100.0, 20.0, 150.0, 5.0, 1)
                 << corrParam(0.0);
        s.build = buildQwt;
        *out << s;
    }
    addMicrostrip2(out);
}

} // namespace tpl::samples
