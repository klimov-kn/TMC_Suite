// Раздел «Антенны»: излучение в открытое пространство.
//
// Открытое пространство в двумерном ядре — это коробка, обложенная поглотителем по
// краям (как в авторских `LR1_rupor.tpl` и `hyper_sheath_0p4.TPL`). Питающий волновод
// входит в коробку слева; там, где он проходит, поглотителя нет. Диаграмму
// направленности ядро пишет строкой FIELD_DISTRIBUTION_M — её показывает TMC_DN.

/// @file antennas.cpp
/// @brief Раздел каталога «Антенны»: открытый волновод, рупоры, линза, отражатель, решётка.
///
/// Часть модуля «Ядро TPL Designer», каталог образцов. Все рецепты — H-ядро, открытая
/// область с рамкой поглотителя, снимаются S11 и ДН; регистрация — `addAntennas`.
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

/// @brief Возвращает параметр частоты `f0` (1...100 ГГц).
/// @param[in] def  значение по умолчанию, ГГц
/// @return Вещественный параметр.
Param freqParam(double def = 10.0)
{
    return Param::number(QStringLiteral("f0"), tpl::tr("Частота", "Frequency"), tpl::tr("ГГц", "GHz"),
                         def, 1.0, 100.0, 0.5, 2,
                         tpl::tr("Питающий волновод программа подбирает по стандарту сама — "
                                 "тот, в рабочей полосе которого лежит частота",
                                 "The program picks the standard feed waveguide itself: the one "
                                 "whose operating band contains the frequency"));
}

/// Открытая сцена с питающим волноводом слева.
struct Feed {
    /// Сечение питающего волновода.
    WaveguideStd std;
    /// Ширина питающего волновода, мм.
    double a = 0.0;
    double ylo = 0.0, yhi = 0.0;
    /// @var ylo
    /// Нижняя стенка канала питания (внутренняя грань), мм.
    /// @var yhi
    /// Верхняя стенка канала питания (внутренняя грань), мм.
    /// @var fc
    /// Отсечка H10 питающего волновода, ГГц.
    double fc = 0.0;
};

/// Коробка, поглотитель по краям и питающий волновод от левого края до \p xEnd.
/// \p yc — ось волновода. Возвращает раскладку питания.
///
/// Поглотитель толщиной 5 шагов ставится справа, сверху и снизу целиком, слева — выше
/// и ниже волновода (полоски уже 3 шагов не ставятся). Вход H10 — у левого края.
/// @param[in,out] b         сборщик; `b.p.delta` должен быть задан
/// @param[in]     f0        частота, ГГц (по ней выбирается сечение)
/// @param[in]     X         размер области по X, мм
/// @param[in]     Y         размер области по Y, мм
/// @param[in]     yc        ось питающего волновода, мм
/// @param[in]     xEnd      конец питающего волновода по X, мм
/// @param[in]     feedName  имя стенок питающего волновода
/// @return Раскладка питания.
/// @see https://www.tamic.ru/docs/theory/boundaries.html#absorber
Feed openScene(Builder &b, double f0, double X, double Y, double yc, double xEnd,
               const QString &feedName = tpl::tr("питающий волновод", "feed waveguide"))
{
    Feed f;
    f.std = waveguides().value(waveguideForFreq(f0));
    f.a = f.std.a;
    f.fc = f.std.fc10();
    const double d = b.p.delta;
    f.ylo = snap(yc - f.a / 2.0, d);
    f.yhi = f.ylo + f.a;
    b.p.box = {0.0, X, 0.0, Y};

    const double t = 5 * d;
    // Поглотитель: справа, сверху, снизу целиком; слева — выше и ниже волновода.
    b.rect(tpl::tr("поглотитель справа", "absorber right"), Material::Absorber, X - t, X, 0.0, Y);
    b.rect(tpl::tr("поглотитель сверху", "absorber top"), Material::Absorber, t, X - t, Y - t, Y);
    b.rect(tpl::tr("поглотитель снизу", "absorber bottom"), Material::Absorber, t, X - t, 0.0, t);
    // Узкая полоска поглотителя (тоньше 2*DELTA) ядру не нужна и валидатору не нравится.
    if (f.ylo - 3 * d > 3 * d)
        b.rect(tpl::tr("поглотитель слева, низ", "absorber left, bottom"), Material::Absorber, 0.0, t, 0.0,
               f.ylo - 3 * d);
    if (Y - (f.yhi + 3 * d) > 3 * d)
        b.rect(tpl::tr("поглотитель слева, верх", "absorber left, top"), Material::Absorber, 0.0, t,
               f.yhi + 3 * d, Y);
    b.guideX(feedName, 0.0, xEnd, f.ylo, f.yhi);
    b.port(tpl::tr("вход", "port"), PortSide::Left, f.ylo, f.yhi, true);
    return f;
}

/// @brief Возвращает строки паспорта о питании и длине волны.
/// @param[in] f   раскладка питания
/// @param[in] f0  частота, ГГц
/// @return Две строки: питающий волновод и lambda в свободном пространстве.
QVector<Row> feedRows(const Feed &f, double f0)
{
    return {
        {tpl::tr("Питание", "Feed"),
         tpl::tr("%1, a = %2 мм, отсечка %3 ГГц",
                 "%1, a = %2 mm, cutoff %3 GHz").arg(f.std.name, num(f.a, 5), num(f.fc, 4))},
        {tpl::tr("Длина волны λ", "Wavelength lambda"), mm(rf::lambda0(f0))},
    };
}

/// @brief Возвращает общий абзац справки раздела: где смотреть S11 и ДН.
/// @return Текст на текущем языке, начинается с пустой строки.
QString helpDn()
{
    return tpl::tr(
        "\n\nЧто смотреть: |S11| питания — в TMCGROUT; диаграмму направленности (файлы "
        ".DAT/.AMP/.FAZ) — в TMC_DN. Картинку поля для FieldView образец не пишет — она "
        "замедляет счёт в разы; включите её в мастере на странице «Вывод данных». "
        "Поглотитель по краям области изображает открытое пространство.",
        "\n\nWhat to look at: |S11| of the feed in TMCGROUT; the radiation pattern (.DAT/.AMP/.FAZ "
        "files) in TMC_DN. The sample does not write the field picture for FieldView: it slows the "
        "run down several times; enable it in the wizard on the \"Output\" page. The absorber along "
        "the edges of the domain represents open space.");
}

// ------------------------------------------------------------ открытый конец

/// @brief Рецепт «Открытый конец волновода» (`ant_open_wg`).
///
/// Простейший излучатель: волновод, обрывающийся в пространство.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `f0` — частота, ГГц (питающий волновод подбирается по стандарту сам)
/// @return Задание, паспорт «теория» и пояснения.
/// @see https://www.tamic.ru/docs/theory/radiation.html#pattern
Build buildOpenWg(const Values &v)
{
    const double f0 = v.value(QStringLiteral("f0"));
    const double lam = rf::lambda0(f0);
    const WaveguideStd w = waveguides().value(waveguideForFreq(f0));
    Builder b(QStringLiteral("openwg"));
    b.p.delta = deltaFitting(w.a, std::min(lam / 20.0, w.a / 16.0));
    const double d = b.p.delta;
    const double stub = snap(1.5 * rf::lambdaGuide(w.a, f0), d);
    const double X = snap(stub + 5.0 * lam, d);
    const double Y = snap(8.0 * lam, d);
    b.p.description = tpl::tr("Излучение из открытого конца волновода %1 на %2 ГГц.",
                              "Radiation from the open end of waveguide %1 at %2 GHz.")
                          .arg(w.name, num(f0, 4));
    const Feed f = openScene(b, f0, X, Y, Y / 2.0, stub);
    b.freq(f0);
    b.outputs(true, true);
    b.time(settleTime(X * 1.5, f0, 0.0));

    QVector<Row> rows = feedRows(f, f0);
    rows << Row{tpl::tr("Раскрыв a/λ", "Aperture a/lambda"), num(f.a / lam)};
    rows << Row{tpl::tr("Ожидаемо", "Expected"),
                tpl::tr("широкая ДН без выраженного лепестка: раскрыв меньше длины волны",
                        "a broad pattern without a pronounced lobe: the aperture is smaller than "
                        "the wavelength")};
    return b.done(rows);
}

// ------------------------------------------------------------ рупор

/// Рупор: стенки от горловины (xThroat) до раскрыва (xAp), раскрыв A.
///
/// Каждая стенка — ломаная из трёх точек (середина добавлена из-за требования
/// `POLYGON_STAT` к числу строк `L`).
/// @param[in,out] b        сборщик
/// @param[in]     f        раскладка питания (стенки горловины)
/// @param[in]     xThroat  координата горловины по X, мм
/// @param[in]     xAp      координата раскрыва по X, мм
/// @param[in]     A        ширина раскрыва, мм
void hornWalls(Builder &b, const Feed &f, double xThroat, double xAp, double A)
{
    const double yc = (f.ylo + f.yhi) / 2.0;
    b.polyline(tpl::tr("рупор, нижняя стенка", "horn, bottom wall"), Material::Metal,
               {QPointF(xThroat, f.ylo), QPointF((xThroat + xAp) / 2.0, (f.ylo + yc - A / 2) / 2.0),
                QPointF(xAp, yc - A / 2)});
    b.polyline(tpl::tr("рупор, верхняя стенка", "horn, top wall"), Material::Metal,
               {QPointF(xThroat, f.yhi), QPointF((xThroat + xAp) / 2.0, (f.yhi + yc + A / 2) / 2.0),
                QPointF(xAp, yc + A / 2)});
}

/// @brief Рецепт «H-секторальный рупор» (`ant_horn`).
///
/// Оптимальный рупор: длина по заданному раскрыву.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `f0` — частота, ГГц (питающий волновод подбирается по стандарту сам);
///                `ap` — раскрыв, в длинах волны
/// @return Задание, паспорт «теория» и пояснения.
Build buildHorn(const Values &v)
{
    const double f0 = v.value(QStringLiteral("f0"));
    const double lam = rf::lambda0(f0);
    const double A = v.value(QStringLiteral("ap")) * lam;
    const WaveguideStd w = waveguides().value(waveguideForFreq(f0));

    // Оптимальный H-секторальный рупор: A = sqrt(3 lambda R), R — от вершины до раскрыва.
    const double R = A * A / (3.0 * lam);
    const double l = R * (A - w.a) / A; // осевая длина раструба

    Builder b(QStringLiteral("horn"));
    b.p.delta = deltaFitting(w.a, std::min(lam / 20.0, w.a / 16.0));
    const double d = b.p.delta;
    const double lf = snap(rf::lambdaGuide(w.a, f0), d);
    const double xAp = snap(lf + l, d);
    const double X = snap(xAp + 4.0 * lam, d);
    const double Y = snap(A + 4.0 * lam, d);
    b.p.description = tpl::tr("Оптимальный H-секторальный рупор: раскрыв %1 мм на %2 ГГц.",
                              "Optimum H-plane sectoral horn: aperture %1 mm at %2 GHz.")
                          .arg(num(A, 4), num(f0, 4));
    const Feed f = openScene(b, f0, X, Y, Y / 2.0, lf);
    hornWalls(b, f, lf, xAp, A);
    b.freq(f0);
    b.outputs(true, true);
    b.time(settleTime(X * 1.5, f0, 0.0));

    QVector<Row> rows = feedRows(f, f0);
    rows << Row{tpl::tr("Раскрыв A", "Aperture A"), tpl::tr("%1 (%2 λ)", "%1 (%2 lambda)").arg(mm(A), num(A / lam))};
    rows << Row{tpl::tr("Оптимальная длина R = A²/3λ", "Optimum length R = A^2/3lambda"), mm(R)};
    rows << Row{tpl::tr("Длина раструба", "Flare length"), mm(xAp - lf)};
    rows << Row{tpl::tr("Угол раскрыва", "Flare angle"),
                tpl::tr("%1°", "%1 deg").arg(num(2.0 * std::atan((A - w.a) / 2.0 / l) * 180.0 / kPi))};
    rows << Row{tpl::tr("Ширина ДН по уровню -3 дБ", "Pattern width at -3 dB"),
                tpl::tr("≈ %1° (косинусный раскрыв, 68 λ/A)",
                        "approx. %1 deg (cosine aperture, 68 lambda/A)").arg(num(68.0 * lam / A))};
    return b.done(rows, {tpl::tr("«Оптимальный» — при заданном раскрыве даёт наибольший "
                                 "КНД: фазовая ошибка на краю раскрыва около 3/8 λ.",
                                 "\"Optimum\" means the highest directivity for a given aperture: "
                                 "the phase error at the aperture edge is about 3/8 lambda.")});
}

// ------------------------------------------------------------ рупор с линзой

/// @brief Рецепт «Рупор с диэлектрической линзой» (`ant_lens_horn`).
///
/// Короткий рупор и гиперболическая линза — два варианта геометрии (без линзы и с линзой).
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `f0` — частота, ГГц (питающий волновод подбирается по стандарту сам);
///                `ap` — раскрыв, в длинах волны;
///                `eps` — проницаемость линзы
/// @return Задание, паспорт «теория» и пояснения.
Build buildLensHorn(const Values &v)
{
    const double f0 = v.value(QStringLiteral("f0"));
    const double lam = rf::lambda0(f0);
    const double A = v.value(QStringLiteral("ap")) * lam;
    const double eps = v.value(QStringLiteral("eps"));
    const double n = std::sqrt(eps);
    const WaveguideStd w = waveguides().value(waveguideForFreq(f0));

    // Короткий рупор: вдвое короче оптимального — с заметной фазовой ошибкой.
    const double R = A * A / (3.0 * lam) / 2.0;
    const double l = R * (A - w.a) / A;

    Builder b(QStringLiteral("lenshorn"));
    b.p.delta = deltaFitting(w.a, std::min(lam / 20.0, w.a / 16.0));
    const double d = b.p.delta;
    const double lf = snap(rf::lambdaGuide(w.a, f0), d);
    const double xAp = snap(lf + l, d);
    const double xApex = xAp - R;     // вершина раструба = фокус линзы
    const double F = xAp - xApex;

    // Гиперболическая линза: r(t) = F (n-1) / (n cos t - 1), фокус — в вершине рупора.
    // Со стороны рупора — гипербола, снаружи — плоскость.
    QVector<QPointF> hyper;
    double tMax = 0.0;
    {
        const int N = 24;
        // Угол, при котором гипербола поднимается на A/2.
        double lo = 0.0, hi = std::acos(1.0 / n) * 0.999;
        for (int it = 0; it < 60; ++it) {
            const double t = (lo + hi) / 2.0;
            const double r = F * (n - 1.0) / (n * std::cos(t) - 1.0);
            (r * std::sin(t) < A / 2.0 ? lo : hi) = t;
        }
        tMax = lo;
        for (int i = -N; i <= N; ++i) {
            const double t = tMax * i / N;
            const double r = F * (n - 1.0) / (n * std::cos(t) - 1.0);
            hyper << QPointF(xApex + r * std::cos(t), r * std::sin(t));
        }
    }
    const double xEdge = hyper.first().x();
    const double xBack = snap(xEdge + 2 * d, d);
    const double X = snap(xBack + 4.0 * lam, d);
    const double Y = snap(A + 4.0 * lam, d);
    b.p.description = tpl::tr("Укороченный H-рупор с гиперболической линзой eps = %1 "
                              "на %2 ГГц; варианты без линзы и с линзой.",
                              "Shortened H-horn with a hyperbolic lens eps = %1 at %2 GHz; variants "
                              "without and with the lens.")
                          .arg(num(eps), num(f0, 4));
    const Feed f = openScene(b, f0, X, Y, Y / 2.0, lf);
    hornWalls(b, f, lf, xAp, A);

    const double yc = (f.ylo + f.yhi) / 2.0;
    QVector<QPointF> lens;
    for (const QPointF &p : hyper)
        lens << QPointF(p.x(), yc + p.y());
    lens << QPointF(xBack, yc + A / 2.0) << QPointF(xBack, yc - A / 2.0);
    Shape &ls = b.polygon(tpl::tr("линза", "lens"), Material::Dielectric, lens);
    ls.eps = eps;
    const int lensId = ls.id;

    Variant bare;
    bare.name = tpl::tr("без линзы", "without lens");
    bare.tag = QStringLiteral("bare");
    bare.disabledShapes = {lensId};
    Variant withLens;
    withLens.name = tpl::tr("с линзой", "with lens");
    withLens.tag = QStringLiteral("lens");
    b.p.variants << bare << withLens;

    b.freq(f0);
    b.outputs(true, true);
    b.time(settleTime(X * 1.5, f0, 0.0));

    const double phaseErr = A * A / (8.0 * R * lam);
    QVector<Row> rows = feedRows(f, f0);
    rows << Row{tpl::tr("Раскрыв A", "Aperture A"), tpl::tr("%1 (%2 λ)", "%1 (%2 lambda)").arg(mm(A), num(A / lam))};
    rows << Row{tpl::tr("Длина до вершины R",
                        "Length to the apex R"), tpl::tr("%1 (вдвое короче оптимальной)",
                                                              "%1 (half the optimum)").arg(mm(R))};
    rows << Row{tpl::tr("Фазовая ошибка без линзы", "Phase error without the lens"),
                tpl::tr("%1 λ на краю (%2°)",
                        "%1 lambda at the edge (%2 deg)").arg(num(phaseErr), num(phaseErr * 360.0))};
    rows << Row{tpl::tr("Линза", "Lens"),
                tpl::tr("eps = %1 (n = %2), фокус %3, толщина в центре %4",
                        "eps = %1 (n = %2), focus %3, center thickness %4")
                    .arg(num(eps), num(n), mm(F), mm(xBack - xAp))};
    return b.done(rows, {tpl::tr("Одно задание — два варианта: horn_bare и horn_lens. "
                                 "Сравните ДН в TMC_DN: линза выравнивает фазу в раскрыве, "
                                 "лепесток сужается и растёт.",
                                 "One task, two variants: horn_bare and horn_lens. Compare the "
                                 "patterns in TMC_DN: the lens equalizes the phase in the aperture, "
                                 "so the lobe narrows and grows.")});
}

// ------------------------------------------------------------ отражатель

/// @brief Рецепт «Параболический отражатель» (`ant_reflector`).
///
/// Цилиндрическая парабола с облучателем в фокусе.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `f0` — частота, ГГц (питающий волновод подбирается по стандарту сам);
///                `dia` — раскрыв D, в длинах волны;
///                `fd` — отношение F/D
/// @return Задание, паспорт «теория» и пояснения.
Build buildReflector(const Values &v)
{
    const double f0 = v.value(QStringLiteral("f0"));
    const double lam = rf::lambda0(f0);
    const double D = v.value(QStringLiteral("dia")) * lam;
    const double F = v.value(QStringLiteral("fd")) * D;
    const WaveguideStd w = waveguides().value(waveguideForFreq(f0));

    Builder b(QStringLiteral("reflector"));
    b.p.delta = deltaFitting(w.a, std::min(lam / 20.0, w.a / 16.0));
    const double d = b.p.delta;
    const double xFocus = snap(4.0 * lam, d);
    const double xVertex = snap(xFocus + F, d);
    const double X = snap(xVertex + 2.0 * lam, d);
    const double Y = snap(D + 4.0 * lam, d);
    b.p.description = tpl::tr("Параболический цилиндрический отражатель: D = %1 мм, "
                              "F = %2 мм, %3 ГГц.",
                              "Parabolic cylindrical reflector: D = %1 mm, F = %2 mm, %3 GHz.")
                          .arg(num(D, 4), num(F, 4), num(f0, 4));
    const Feed f = openScene(b, f0, X, Y, Y / 2.0, xFocus, tpl::tr("облучатель", "feed"));
    const double yc = (f.ylo + f.yhi) / 2.0;

    QVector<QPointF> par;
    const int N = 40;
    for (int i = 0; i <= N; ++i) {
        const double y = -D / 2.0 + D * i / N;
        par << QPointF(xVertex - y * y / (4.0 * F), yc + y);
    }
    b.polyline(tpl::tr("отражатель", "reflector"), Material::Metal, par);

    b.freq(f0);
    b.outputs(true, true);
    b.time(settleTime(X * 2.0, f0, 0.0));

    const double edge = 2.0 * std::atan(D / 2.0 / (F - D * D / (16.0 * F))) * 180.0 / kPi;
    QVector<Row> rows = feedRows(f, f0);
    rows << Row{tpl::tr("Раскрыв D", "Aperture D"), tpl::tr("%1 (%2 λ)", "%1 (%2 lambda)").arg(mm(D), num(D / lam))};
    rows << Row{tpl::tr("Фокус F (F/D)", "Focus F (F/D)"), QStringLiteral("%1 (%2)").arg(mm(F), num(F / D))};
    rows << Row{tpl::tr("Угол облучения края",
                        "Edge illumination angle"), tpl::tr("%1°", "%1 deg").arg(num(edge))};
    rows << Row{tpl::tr("Ширина ДН по уровню -3 дБ", "Pattern width at -3 dB"),
                tpl::tr("≈ %1° (70 λ/D)", "approx. %1 deg (70 lambda/D)").arg(num(70.0 * lam / D))};
    return b.done(rows, {tpl::tr("Облучатель — открытый волновод в фокусе; его стенки "
                                 "затеняют часть раскрыва, как у настоящей антенны с "
                                 "прямым облучением.",
                                 "The feed is an open waveguide at the focus; its walls shadow part "
                                 "of the aperture, as in a real front-fed antenna.")});
}

// ------------------------------------------------------------ решётка

/// @brief Рецепт «Фазированная решётка» (`ant_array`).
///
/// Линейка волноводных излучателей со сканированием луча.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `f0` — частота, ГГц (питающий волновод подбирается по стандарту сам);
///                `count` — число излучателей;
///                `pitch` — шаг решётки, в длинах волны;
///                `scan` — угол сканирования, градусы
/// @return Задание, паспорт «теория» и пояснения.
/// @see https://www.tamic.ru/docs/theory/radiation.html#arrays
Build buildArray(const Values &v)
{
    const double f0 = v.value(QStringLiteral("f0"));
    const double lam = rf::lambda0(f0);
    const int N = int(v.value(QStringLiteral("count")));
    const double scan = v.value(QStringLiteral("scan"));
    double dRel = v.value(QStringLiteral("pitch"));

    // Элемент — короткий волновод H10: он излучает, только если шире lambda/2.
    const double ae = 0.6 * lam;
    Builder b(QStringLiteral("array"));
    b.p.delta = deltaFitting(ae, lam / 20.0);
    const double d = b.p.delta;
    QStringList notes;
    const double pitchMin = (ae + 3 * d) / lam;
    if (dRel < pitchMin) {
        notes << tpl::tr("Шаг %1 λ меньше ширины излучателя — взят %2 λ.",
                         "Spacing %1 lambda is smaller than the radiator width; %2 lambda is used.")
                     .arg(num(dRel), num(pitchMin));
        dRel = pitchMin;
    }
    const double pitch = snap(dRel * lam, d);
    const double stub = snap(0.75 * lam, d);
    const double span = pitch * (N - 1) + ae;
    const double X = snap(stub + 6.0 * lam, d);
    const double Y = snap(span + 6.0 * lam, d);
    b.p.box = {0.0, X, 0.0, Y};
    b.p.description = tpl::tr("Решётка из %1 излучателей, шаг %2 мм, сканирование на "
                              "%3 град., %4 ГГц.",
                              "Array of %1 radiators, spacing %2 mm, scan to %3 deg., %4 GHz.")
                          .arg(N).arg(num(pitch, 4), num(scan), num(f0, 4));

    const double t = 5 * d;
    b.rect(tpl::tr("поглотитель справа", "absorber right"), Material::Absorber, X - t, X, 0.0, Y);
    b.rect(tpl::tr("поглотитель сверху", "absorber top"), Material::Absorber, t, X - t, Y - t, Y);
    b.rect(tpl::tr("поглотитель снизу", "absorber bottom"), Material::Absorber, t, X - t, 0.0, t);

    // Фазовый сдвиг между соседями: волна уходит под углом scan от оси X.
    const double k = 2.0 * kPi / lam;
    const double dphi = -k * pitch * std::sin(scan * kPi / 180.0);
    const double y0 = snap((Y - span) / 2.0, d);
    for (int i = 0; i < N; ++i) {
        const double lo = y0 + i * pitch;
        const double hi = lo + snap(ae, d);
        b.polyline(tpl::tr("излучатель %1, низ", "radiator %1, bottom").arg(i + 1), Material::Metal,
                   {QPointF(0, lo), QPointF(stub / 2, lo), QPointF(stub, lo)});
        b.polyline(tpl::tr("излучатель %1, верх", "radiator %1, top").arg(i + 1), Material::Metal,
                   {QPointF(0, hi), QPointF(stub / 2, hi), QPointF(stub, hi)});
        Port &p = b.port(tpl::tr("излучатель %1", "radiator %1").arg(i + 1), PortSide::Left, lo, hi, true);
        // Фаза — в радианах (ANGLE_UNIT radian), приведённая в (-pi; pi].
        p.signal.phase = std::remainder(dphi * i, 2.0 * kPi);
    }
    b.freq(f0);
    // Активны все излучатели сразу: .s здесь не матрица рассеяния, его не пишем.
    b.outputs(false, true);
    b.time(settleTime(X * 1.5, f0, 0.0));

    const double grating = 1.0 / (1.0 + std::fabs(std::sin(scan * kPi / 180.0)));
    QVector<Row> rows;
    rows << Row{tpl::tr("Длина волны λ", "Wavelength lambda"), mm(lam)};
    rows << Row{tpl::tr("Излучатели", "Radiators"),
                tpl::tr("%1 шт., ширина %2, шаг %3 (%4 λ)", "%1 pcs., width %2, spacing %3 (%4 lambda)")
                    .arg(N).arg(mm(snap(ae, d)), mm(pitch), num(pitch / lam))};
    rows << Row{tpl::tr("Сдвиг фазы между соседями", "Phase shift between neighbours"),
                tpl::tr("%1°", "%1 deg").arg(num(dphi * 180.0 / kPi))};
    rows << Row{tpl::tr("Ширина луча по уровню -3 дБ", "Beam width at -3 dB"),
                tpl::tr("≈ %1° (50.8 λ / N d cos)", "approx. %1 deg (50.8 lambda / N d cos)")
                    .arg(num(50.8 * lam / (N * pitch * std::cos(scan * kPi / 180.0))))};
    rows << Row{tpl::tr("Без дифракционных лепестков", "No grating lobes"),
                pitch / lam < grating
                    ? tpl::tr("да: d/λ = %1 < %2",
                              "yes: d/lambda = %1 < %2").arg(num(pitch / lam), num(grating))
                    : tpl::tr("НЕТ: d/λ = %1 >= %2 — появится второй луч",
                              "NO: d/lambda = %1 >= %2, a second beam will appear")
                          .arg(num(pitch / lam), num(grating))};
    notes << tpl::tr("Все излучатели активны одновременно, каждый со своей фазой, — "
                     "поэтому S-матрица не пишется: результат смотрят в ДН и в поле.",
                     "All radiators are active at once, each with its own phase, so the S-matrix is "
                     "not written: the result is seen in the radiation pattern and in the field.");
    return b.done(rows, notes);
}

} // namespace

void addAntennas(QVector<Sample> *out)
{
    const QString sec = QStringLiteral("antenna");
    {
        Sample s;
        s.id = QStringLiteral("ant_open_wg");
        s.section = sec;
        s.title = tpl::tr("Открытый конец волновода", "Open-ended waveguide");
        s.summary = tpl::tr("Простейший излучатель: волновод, обрывающийся в пространство",
                            "The simplest radiator: a waveguide ending in open space");
        s.standard = tpl::tr("питание по EIA RS-261 / IEC 60153-2", "feed per EIA RS-261 / IEC 60153-2");
        s.help = tpl::tr(
                     "Волновод, открытый в свободное пространство. Раскрыв меньше длины волны, "
                     "поэтому излучение широкое, а часть мощности отражается обратно — это видно "
                     "по |S11|. С этого образца удобно начинать разговор об антеннах: дальше "
                     "раскрыв расширяют рупором.",
                     "A waveguide open to free space. The aperture is smaller than the wavelength, "
                     "so the radiation is broad and part of the power is reflected back, as |S11| "
                     "shows. This sample is a convenient start for antennas: next the aperture is "
                     "widened by a horn.")
                 + helpDn();
        s.params = {freqParam()};
        s.build = buildOpenWg;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ant_horn");
        s.section = sec;
        s.title = tpl::tr("H-секторальный рупор", "H-plane sectoral horn");
        s.summary = tpl::tr("Оптимальный рупор: длина по заданному раскрыву",
                            "Optimum horn: length for a given aperture");
        s.standard = tpl::tr("оптимальный рупор, A = sqrt(3 λ R)", "optimum horn, A = sqrt(3 lambda R)");
        s.help = tpl::tr(
                     "Рупор, расширяющий волновод в H-плоскости. Длина подобрана так, чтобы при "
                     "заданном раскрыве КНД был наибольшим: длиннее — лучше фаза, но больше "
                     "габарит; короче — фаза на краю раскрыва «разъезжается», и лепесток "
                     "расплывается. В паспорте — оптимальная длина и оценка ширины ДН.",
                     "A horn flaring the waveguide in the H-plane. The length is chosen to give the "
                     "highest directivity for the given aperture: longer means better phase but a "
                     "larger size; shorter means the phase at the aperture edge \"drifts apart\" "
                     "and the lobe spreads. The datasheet gives the optimum length and an estimate "
                     "of the pattern width.")
                 + helpDn();
        s.params = {freqParam(),
                    Param::number(QStringLiteral("ap"), tpl::tr("Раскрыв", "Aperture"),
                                  tpl::tr("λ", "lambda"), 4.0, 1.5, 10.0, 0.5, 1)};
        s.build = buildHorn;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ant_lens_horn");
        s.section = sec;
        s.title = tpl::tr("Рупор с диэлектрической линзой", "Horn with a dielectric lens");
        s.summary = tpl::tr("Короткий рупор и гиперболическая линза — два варианта",
                            "Short horn and a hyperbolic lens: two variants");
        s.standard = tpl::tr("гиперболическая линза, фокус в вершине рупора",
                             "hyperbolic lens, focus at the horn apex");
        s.help = tpl::tr(
                     "Короткий рупор с большой фазовой ошибкой в раскрыве и гиперболическая "
                     "диэлектрическая линза, которая эту ошибку исправляет. Задание считает оба "
                     "варианта сразу — без линзы и с линзой, — в разные файлы. Сравнение ДН "
                     "показывает, зачем линза нужна.",
                     "A short horn with a large phase error in the aperture and a hyperbolic "
                     "dielectric lens that corrects this error. The task computes both variants at "
                     "once, without and with the lens, into separate files. Comparing the patterns "
                     "shows why the lens is needed.")
                 + helpDn();
        s.params = {freqParam(),
                    Param::number(QStringLiteral("ap"), tpl::tr("Раскрыв", "Aperture"),
                                  tpl::tr("λ", "lambda"), 5.0, 2.0, 10.0, 0.5, 1),
                    Param::number(QStringLiteral("eps"), tpl::tr("eps линзы", "Lens eps"), {}, 2.5, 1.5,
                                  10.0, 0.1, 2, tpl::tr("2.1 — фторопласт, 2.5 — полистирол",
                                                        "2.1 = PTFE, 2.5 = polystyrene"))};
        s.build = buildLensHorn;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ant_reflector");
        s.section = sec;
        s.title = tpl::tr("Параболический отражатель", "Parabolic reflector");
        s.summary = tpl::tr("Цилиндрическая парабола с облучателем в фокусе",
                            "Cylindrical parabola with a feed at the focus");
        s.standard = tpl::tr("зеркальная антенна с прямым облучением", "front-fed reflector antenna");
        s.help = tpl::tr(
                     "Параболический цилиндр и открытый волновод в его фокусе. Отражатель "
                     "превращает расходящуюся волну облучателя в плоскую — луч получается узким, "
                     "около 70 λ/D. Облучатель стоит перед зеркалом и затеняет его середину: "
                     "это видно по боковым лепесткам.",
                     "A parabolic cylinder with an open waveguide at its focus. The reflector turns "
                     "the diverging wave of the feed into a plane one, so the beam is narrow, about "
                     "70 lambda/D. The feed stands in front of the mirror and shadows its middle: "
                     "this shows in the side lobes.")
                 + helpDn();
        s.params = {freqParam(),
                    Param::number(QStringLiteral("dia"), tpl::tr("Раскрыв D", "Aperture D"),
                                  tpl::tr("λ", "lambda"), 10.0, 4.0, 20.0, 1.0, 1),
                    Param::number(QStringLiteral("fd"), tpl::tr("Отношение F/D", "Ratio F/D"), {}, 0.4,
                                  0.25, 1.0, 0.05, 2)};
        s.build = buildReflector;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("ant_array");
        s.section = sec;
        s.title = tpl::tr("Фазированная решётка", "Phased array");
        s.summary = tpl::tr("Линейка волноводных излучателей со сканированием луча",
                            "Linear array of waveguide radiators with beam scanning");
        s.standard = tpl::tr("линейная ФАР, равномерное возбуждение",
                             "linear phased array, uniform excitation");
        s.help = tpl::tr(
                     "Линейка открытых волноводов, возбуждаемых одновременно с линейным "
                     "набегом фазы. Набег поворачивает луч на заданный угол. Если шаг решётки "
                     "велик, появляется второй — дифракционный — луч: паспорт предупреждает об "
                     "этом заранее.\n\nФазы входов программа считает сама и пишет седьмым "
                     "параметром каждого входа.",
                     "A line of open waveguides excited simultaneously with a linear phase "
                     "progression. The progression steers the beam to the given angle. If the array "
                     "spacing is large, a second (grating) beam appears: the datasheet warns about "
                     "it in advance.\n\nThe program computes the port phases itself and writes them "
                     "as the seventh parameter of each port.")
                 + helpDn();
        s.params = {freqParam(),
                    Param::integer(QStringLiteral("count"), tpl::tr("Излучателей", "Radiators"), 6, 2, 16),
                    Param::number(QStringLiteral("pitch"), tpl::tr("Шаг решётки", "Array spacing"),
                                  tpl::tr("λ", "lambda"), 0.7, 0.5, 2.0, 0.05, 2),
                    Param::number(QStringLiteral("scan"), tpl::tr("Угол сканирования", "Scan angle"),
                                  tpl::tr("град.", "deg."), 20.0, -60.0, 60.0, 5.0, 1)};
        s.build = buildArray;
        *out << s;
    }
}

} // namespace tpl::samples
