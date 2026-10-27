// Раздел «Среды, плазма, ЭПР»: неоднородная eps, плазма и рассеяние на телах.

/// @file media.cpp
/// @brief Раздел каталога «Среды, плазма, ЭПР»: диэлектрический слой, профиль eps, файл .eps, линза, плазма, рассеяние.
///
/// Часть модуля «Ядро TPL Designer», каталог образцов. Рецепты плазмы — X-ядро,
/// остальные — H-ядро; рецепты ЭПР используют варианты геометрии и угловой свип.
/// Регистрация — `addMedia`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#samples
#include "emitter.h"
#include "epsfile.h"
#include "kit.h"
#include "lang.h"
#include "rf.h"
#include "sample.h"
#include "standards.h"

#include "physics.h"

#include <QByteArray>

#include <cmath>
#include <cstring>

namespace tpl::samples {

namespace {

using namespace kit;

/// Число pi.
constexpr double kPi = rf::kPi;

/// Канал плоской волны: две магнитные стенки ТОЧНО по краям апертуры. Между ними
/// однородная волна Ez распространяется как плоская волна при нормальном падении.
/// @param[in,out] b    сборщик
/// @param[in]     L    длина канала по X (от 0), мм
/// @param[in]     ylo  нижняя стенка, мм
/// @param[in]     yhi  верхняя стенка, мм
/// @see https://www.tamic.ru/docs/theory/boundaries.html#open
void temChannel(Builder &b, double L, double ylo, double yhi)
{
    b.polyline(tpl::tr("канал, низ", "channel, bottom"), Material::Magnetic,
               {QPointF(0, ylo), QPointF(L / 2, ylo), QPointF(L, ylo)});
    b.polyline(tpl::tr("канал, верх", "channel, top"), Material::Magnetic,
               {QPointF(0, yhi), QPointF(L / 2, yhi), QPointF(L, yhi)});
}

/// @brief Возвращает абзац справки о канале плоской волны с магнитными стенками.
/// @return Текст на текущем языке, начинается с пустой строки.
QString helpTem()
{
    return tpl::tr(
        "\n\nПлоская волна при нормальном падении задана каналом с магнитными стенками: в нём "
        "однородное поле распространяется ровно так, как плоская волна в свободном "
        "пространстве. У входа плоской волны в ядре есть остаточное отражение около -25 дБ.",
        "\n\nA plane wave at normal incidence is modelled by a channel with magnetic walls: a "
        "uniform field propagates in it exactly as a plane wave in free space. The plane-wave port "
        "of the kernel has a residual reflection of about -25 dB.");
}

/// @brief Возвращает абзац справки об открытой области и опорном варианте без объекта.
/// @return Текст на текущем языке, начинается с пустой строки.
QString helpOpen()
{
    return tpl::tr(
        "\n\nОткрытое пространство — коробка с поглотителем по краям, плоская волна приходит "
        "слева. Первый вариант считает ту же сцену без объекта: его отклик вычитают, чтобы "
        "отделить рассеяние от отражений самой области.",
        "\n\nOpen space is a box with an absorber along the edges; the plane wave comes from the "
        "left. The first variant computes the same scene without the object: its response is "
        "subtracted to separate the scattering from the reflections of the domain itself.");
}

// ------------------------------------------------------------ слой

/// @brief Рецепт «Плоская волна на диэлектрическом слое» (`med_slab`).
///
/// Нормальное падение: точная формула Френеля для проверки.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `eps` — проницаемость слоя;
///                `t` — толщина, мм;
///                `flo`, `fhi` — границы полосы, ГГц
/// @return Задание, паспорт «теория» и пояснения.
Build buildSlab(const Values &v)
{
    const double eps = v.value(QStringLiteral("eps"));
    const double t = v.value(QStringLiteral("t"));
    const double fLo = v.value(QStringLiteral("flo")), fHi = v.value(QStringLiteral("fhi"));

    Builder b(QStringLiteral("slab"));
    const double lamMin = rf::lambda0(fHi) / std::sqrt(eps);
    b.p.delta = chooseDelta(lamMin, t, 20.0, 6.0);
    const double d = b.p.delta;
    const double W = snap(rf::lambda0(fHi) / 2.0, d);
    const double lead = snap(rf::lambda0(fLo), d);
    const double tt = snap(t, d);
    const double L = 2 * lead + tt;
    const double ylo = 3 * d, yhi = ylo + W;
    b.p.box = {0.0, L, 0.0, yhi + 3 * d};
    b.p.description = tpl::tr("Плоская волна на диэлектрическом слое: eps = %1, толщина %2 мм.",
                              "Plane wave on a dielectric layer: eps = %1, thickness %2 mm.")
                          .arg(num(eps), num(tt));
    temChannel(b, L, ylo, yhi);
    b.dielectric(tpl::tr("слой", "layer"), eps, lead, lead + tt, ylo, yhi);
    b.port(tpl::tr("вход 1", "port 1"), PortSide::Left, ylo, yhi, true, PortMode::Plane);
    b.port(tpl::tr("вход 2", "port 2"), PortSide::Right, ylo, yhi, false, PortMode::Plane);
    b.freqs(fLo, fHi, 33);
    b.outputs(true);
    b.time(settleTime(L + tt * std::sqrt(eps) * 4, fLo, 0.0));

    QVector<Row> rows;
    rows << Row{tpl::tr("Слой", "Layer"), tpl::tr("eps = %1, толщина %2",
                                                "eps = %1, thickness %2").arg(num(eps), mm(tt))};
    const double fT = kLightSpeed / (2.0 * tt * std::sqrt(eps));
    rows << Row{tpl::tr("Слой прозрачен на", "Layer is transparent at"),
                tpl::tr("%1 ГГц и кратных (толщина = k λ/2 в слое)",
                        "%1 GHz and multiples (thickness = k lambda/2 in the layer)").arg(num(fT, 4))};
    const double f0 = (fLo + fHi) / 2.0;
    for (double f : {fLo, f0, fHi}) {
        const double r = std::abs(rf::slabReflection(0.0, eps, tt, f));
        rows << Row{tpl::tr("Точно, %1 ГГц", "Exact, %1 GHz").arg(num(f, 4)),
                    QStringLiteral("|S11| = %1, |S21| = %2").arg(db(r), db(std::sqrt(1 - r * r)))};
    }
    rows << Row{tpl::tr("Одна граница (полупространство)", "Single boundary (half-space)"),
                tpl::tr("|Г| = %1", "|Gamma| = %1").arg(num((std::sqrt(eps) - 1) / (std::sqrt(eps) + 1)))};
    return b.done(rows);
}

// ------------------------------------------------------------ градиентный переход

/// @brief Рецепт «Плавный переход: eps формулой» (`med_grad`).
///
/// Профиль eps от координат против резкой границы — два варианта.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `eps` — проницаемость за переходом;
///                `len` — длина перехода, в длинах волны;
///                `f0` — частота, ГГц
/// @return Задание, паспорт «теория» и пояснения.
Build buildGrad(const Values &v)
{
    const double epsMax = v.value(QStringLiteral("eps"));
    const double lenLam = v.value(QStringLiteral("len"));
    const double f0 = v.value(QStringLiteral("f0"));
    const double lam = rf::lambda0(f0);

    Builder b(QStringLiteral("grad"));
    b.p.delta = chooseDelta(lam / std::sqrt(epsMax), lam, 20.0, 4.0);
    const double d = b.p.delta;
    const double W = snap(lam / 2.0, d);
    const double lead = snap(lam, d);
    const double T = snap(lenLam * lam, d);
    const double tail = snap(lam, d);
    const double L = lead + T + tail;
    const double ylo = 3 * d, yhi = ylo + W;
    b.p.box = {0.0, L, 0.0, yhi + 3 * d};
    b.p.description = tpl::tr("Плавный переход из вакуума в диэлектрик eps = %1: профиль "
                              "eps формулой против резкой границы.",
                              "Smooth transition from vacuum to dielectric eps = %1: an eps profile "
                              "given by a formula versus a sharp boundary.")
                          .arg(num(epsMax));
    temChannel(b, L, ylo, yhi);
    // Резкая граница: сразу eps_max. Плавный переход: eps растёт линейно по длине T.
    const int abrupt = b.dielectric(tpl::tr("резкая граница",
                                            "sharp boundary"), epsMax, lead, lead + T, ylo, yhi).id;
    Shape &g = b.dielectric(tpl::tr("плавный профиль", "smooth profile"), epsMax, lead, lead + T, ylo, yhi);
    g.epsProfile.enabled = true;
    // x — от центра прямоугольника: -T/2 ... T/2.
    g.epsProfile.formula = QStringLiteral("1+(%1)*(x+%2)/%3")
                               .arg(formatNumber(epsMax - 1.0), formatNumber(T / 2.0),
                                    formatNumber(T));
    g.epsProfile.file = QStringLiteral("grad");
    const int graded = g.id;
    b.dielectric(tpl::tr("диэлектрик за переходом",
                         "dielectric beyond the transition"), epsMax, lead + T, L, ylo, yhi);
    b.port(tpl::tr("вход 1 (вакуум)", "port 1 (vacuum)"), PortSide::Left, ylo, yhi, true, PortMode::Plane);
    Port &p2 = b.port(tpl::tr("вход 2 (в диэлектрике)",
                              "port 2 (in the dielectric)"), PortSide::Right, ylo, yhi, false,
                      PortMode::Plane);
    p2.eps = epsMax;
    p2.epsAuto = false; // eps в сечении задана рецептом, а не фигурой под входом

    Variant va;
    va.name = tpl::tr("резкая граница", "sharp boundary");
    va.tag = QStringLiteral("step");
    va.disabledShapes = {graded};
    Variant vb;
    vb.name = tpl::tr("плавный профиль", "smooth profile");
    vb.tag = QStringLiteral("taper");
    vb.disabledShapes = {abrupt};
    b.p.variants << va << vb;

    b.freqs(std::round(f0 * 0.5 * 100) / 100, std::round(f0 * 2.0 * 100) / 100, 16);
    b.outputs(true);
    b.time(settleTime(L * std::sqrt(epsMax) * 2, f0 * 0.5, 0.0));

    QVector<Row> rows;
    rows << Row{tpl::tr("Переход", "Transition"),
                tpl::tr("eps от 1 до %1 на длине %2 (%3 λ на %4 ГГц)",
                        "eps from 1 to %1 over a length of %2 (%3 lambda at %4 GHz)")
                    .arg(num(epsMax), mm(T), num(T / lam), num(f0, 3))};
    rows << Row{tpl::tr("Резкая граница, точно", "Sharp boundary, exact"),
                tpl::tr("|S11| = %1 на всех частотах", "|S11| = %1 at all frequencies")
                    .arg(db((std::sqrt(epsMax) - 1) / (std::sqrt(epsMax) + 1)))};
    rows << Row{tpl::tr("Плавный профиль", "Smooth profile"),
                tpl::tr("отражение падает с ростом частоты: переход длиннее волны",
                        "the reflection falls as the frequency grows: the transition is longer than "
                        "the wavelength")};
    rows << Row{tpl::tr("Файл профиля", "Profile file"),
                tpl::tr("grad.eps пишется ядром на первом шаге; готовый файл "
                        "подключают блоком FILE — см. образец «Среда из файла .eps»",
                        "the kernel writes grad.eps at the first step; a ready file is connected by "
                        "a FILE block, see the sample \"Medium from an .eps file\"")};
    return b.done(rows, {tpl::tr("Один профиль задан формулой от x: так в ядро уходит "
                                 "любая неоднородная eps. Градиент виден и на холсте.",
                                 "One profile is given by a formula of x: this is how any "
                                 "inhomogeneous eps is passed to the kernel. The gradient is also "
                                 "visible on the canvas.")});
}

// ------------------------------------------------------------ среда из файла

/// @brief Рецепт «Среда из готового файла .eps» (`med_eps_file`).
///
/// Распределение eps, посчитанное заранее, подключается блоком FILE; файл кладётся
/// рядом с заданием через `Build::companions`.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `eps` — проницаемость в центре;
///                `f0` — частота, ГГц
/// @return Задание, паспорт «теория» и пояснения.
Build buildEpsFile(const Values &v)
{
    const double epsMax = v.value(QStringLiteral("eps"));
    const double f0 = v.value(QStringLiteral("f0"));
    const double lam = rf::lambda0(f0);

    Builder b(QStringLiteral("epsfile"));
    b.p.delta = chooseDelta(lam / std::sqrt(epsMax), lam, 20.0, 4.0);
    const double d = b.p.delta;
    const double R = snap(1.5 * lam, d);      // полуширина «линзы»
    const double X = snap(2 * R + 5 * lam, d);
    const double Y = snap(2 * R + 4 * lam, d);
    b.p.box = {0.0, X, 0.0, Y};
    b.p.description = tpl::tr("Среда из готового файла lens.eps: гауссов бугор eps до %1.",
                              "Medium from the ready file lens.eps: a Gaussian bump of eps up to "
                              "%1.")
                          .arg(num(epsMax));
    b.absorberFrame(false, true, true, true);
    const QPointF c(snap(X / 2.0, d), snap(Y / 2.0, d));
    Shape &s = b.rect(tpl::tr("распределение из файла",
                              "distribution from file"), Material::Dielectric, c.x() - R,
                      c.x() + R, c.y() - R, c.y() + R);
    s.epsProfile.enabled = true;
    s.epsProfile.fromFile = true;
    s.epsProfile.file = QStringLiteral("lens");
    const double t = 5 * d;
    b.port(tpl::tr("плоская волна", "plane wave"), PortSide::Left, t + d, Y - t - d, true, PortMode::Plane);
    b.freq(f0);
    b.outputs(true);
    b.time(settleTime(X * 2, f0, 0.0));

    const double w = R / 1.5;
    Build out = b.done(
        {Row{tpl::tr("Файл", "File"), tpl::tr("lens.eps рядом с заданием, сетка %1 мм",
                                             "lens.eps next to the task, grid %1 mm").arg(num(d))},
         Row{tpl::tr("Распределение", "Distribution"),
             tpl::tr("eps = 1 + %1 exp(-(x²+y²)/%2²), квадрат %3",
                     "eps = 1 + %1 exp(-(x^2+y^2)/%2^2), square %3").arg(num(epsMax - 1), mm(w), mm(2 * R))}},
        {tpl::tr("Файл программа пишет сама в формате ядра; координаты блока ядро не читает "
                 "— распределение ставится на место анкером.",
                 "The program writes the file itself in the kernel format; the kernel does not read "
                 "the block coordinates: the distribution is put in place by the anchor."),
         tpl::tr("Блок FILE читается только ядром, собранным после 6 сентября 2026 "
                 "(исправление бага #19). Старые сборки останавливаются на «Missing "
                 "separator ;».",
                 "The FILE block is read only by a kernel built after 6 September 2026 (fix of bug "
                 "#19). Older builds stop at \"Missing separator ;\".")});
    out.companions.insert(QStringLiteral("lens.eps"),
                          epsFileBytes(d, 2 * R, 2 * R, [&](double x, double y) {
                              return (epsMax - 1.0) * std::exp(-(x * x + y * y) / (w * w));
                          }));
    return out;
}

// ------------------------------------------------------------ Люнеберг

/// @brief Рецепт «Линза Люнеберга» (`med_luneburg`).
///
/// Градиентная линза: eps = 2 - (r/R)^2 фокусирует плоскую волну.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `r` — радиус, в длинах волны;
///                `f0` — частота, ГГц
/// @return Задание, паспорт «теория» и пояснения.
Build buildLuneburg(const Values &v)
{
    const double f0 = v.value(QStringLiteral("f0"));
    const double lam = rf::lambda0(f0);
    const double R = v.value(QStringLiteral("r")) * lam;

    Builder b(QStringLiteral("luneburg"));
    b.p.delta = chooseDelta(lam / std::sqrt(2.0), lam, 20.0, 4.0);
    const double d = b.p.delta;
    const double X = snap(2 * R + 5 * lam, d);
    const double Y = snap(2 * R + 4 * lam, d);
    b.p.box = {0.0, X, 0.0, Y};
    b.p.description = tpl::tr("Линза Люнеберга радиусом %1 мм: eps = 2 - (r/R)^2.",
                              "Luneburg lens of radius %1 mm: eps = 2 - (r/R)^2.")
                          .arg(num(R, 4));
    b.absorberFrame(false, true, true, true);
    const QPointF c(snap(2 * lam + R, d), snap(Y / 2.0, d));
    Shape &s = b.circle(tpl::tr("линза", "lens"), Material::Dielectric, c, R);
    s.epsProfile.enabled = true;
    // r — расстояние от анкера, а анкер круга — его центр.
    s.epsProfile.formula = QStringLiteral("2-(r/%1)*(r/%1)").arg(formatNumber(R));
    const double t = 5 * d;
    b.port(tpl::tr("плоская волна", "plane wave"), PortSide::Left, t + d, Y - t - d, true, PortMode::Plane);
    b.freq(f0);
    b.outputs(true, true);
    b.time(settleTime(X * 2, f0, 0.0));

    return b.done({Row{tpl::tr("Радиус", "Radius"), tpl::tr("%1 (%2 λ)",
                                                         "%1 (%2 lambda)").arg(mm(R), num(R / lam))},
                   Row{tpl::tr("Профиль", "Profile"), tpl::tr("eps = 2 в центре, 1 на краю",
                                                          "eps = 2 at the center, 1 at the edge")},
                   Row{tpl::tr("Фокус", "Focus"),
                       tpl::tr("на противоположном краю: x = %1 мм, y = %2 мм",
                               "on the opposite edge: x = %1 mm, y = %2 mm")
                           .arg(num(c.x() + R, 4), num(c.y(), 4))}},
                  {tpl::tr("Чтобы увидеть фокусировку, включите выгрузку поля на первом шаге "
                           "(мастер, «Вывод данных») и откройте его в FieldView: плоская волна "
                           "собирается в точку на тыльной стороне линзы.",
                           "To see the focusing, enable the field export at the first step (wizard, "
                           "\"Output\") and open it in FieldView: the plane wave converges to a "
                           "point on the back side of the lens.")});
}

// ------------------------------------------------------------ плазма

/// @brief Возвращает параметр выбора сечения `std`.
/// @param[in] def  номер сечения по умолчанию в `waveguides()`
/// @return Параметр-выбор.
Param stdParam(int def)
{
    return Param::choice(QStringLiteral("std"), tpl::tr("Сечение волновода", "Waveguide cross-section"),
                         waveguideChoices(), def);
}

/// @brief Ищет сечение по началу имени («WR-284»).
/// @param[in] name  начало `WaveguideStd::name`
/// @return Номер в `waveguides()`; не нашлось — `defaultWaveguide()`.
int stdIndex(const char *name)
{
    const QVector<WaveguideStd> &t = waveguides();
    for (int i = 0; i < t.size(); ++i) {
        if (t.at(i).name.startsWith(QLatin1String(name)))
            return i;
    }
    return defaultWaveguide();
}

/// Волновод X-моды с плазменным слоем поперёк — постановка лабораторной работы ЛР2.
struct PlasmaGuide {
    /// Сечение волновода.
    WaveguideStd std;
    double d = 0.0, ylo = 0.0, yhi = 0.0, L = 0.0, x0 = 0.0, x1 = 0.0;
    /// @var d
    /// Шаг сетки, мм.
    /// @var ylo
    /// Нижняя стенка канала (внутренняя грань), мм.
    /// @var yhi
    /// Верхняя стенка канала (внутренняя грань), мм.
    /// @var L
    /// Длина области по X, мм.
    /// @var x0
    /// Левая граница слоя плазмы по X, мм.
    /// @var x1
    /// Правая граница слоя плазмы по X, мм.
};

/// @brief Строит сцену X-ядра: волновод вдоль X и место под слой плазмы поперёк.
///
/// Шаг — a/40, подводы по обе стороны слоя — две длины волны H10 на нижней границе
/// полосы; толщина слоя не меньше двух шагов. Ставит `b.p.kernel = X`, коробку и стенки.
/// @param[in,out] b       сборщик
/// @param[in]     stdIdx  номер сечения в `waveguides()`
/// @param[in]     thick   толщина слоя, мм (округляется к сетке)
/// @return Раскладка сцены.
PlasmaGuide plasmaGuide(Builder &b, int stdIdx, double thick)
{
    PlasmaGuide g;
    g.std = waveguides().value(stdIdx);
    b.p.kernel = Kernel::X;
    b.p.delta = std::round(g.std.a / 40.0 * 1e6) / 1e6;
    g.d = b.p.delta;
    g.ylo = 3 * g.d;
    g.yhi = g.ylo + g.std.a;
    const double lead = snap(2.0 * rf::lambdaGuide(g.std.a, g.std.fLo), g.d);
    const double t = std::max(2 * g.d, snap(thick, g.d));
    g.x0 = lead;
    g.x1 = lead + t;
    g.L = g.x1 + lead;
    b.p.box = {0.0, g.L, 0.0, g.std.a + 6 * g.d};
    b.guideX(tpl::tr("волновод", "waveguide"), 0.0, g.L, g.ylo, g.yhi);
    return g;
}

/// @brief Рецепт «Плазменный слой в волноводе» (`pl_slab`), X-ядро.
///
/// Изотропная плазма: отсечка sqrt(fp^2 + fc^2), точная теория.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `std` — номер сечения в `waveguides()`;
///                `fp` — плазменная частота, ГГц;
///                `t` — толщина слоя, мм
/// @return Задание, паспорт «теория» и пояснения.
/// @see https://www.tamic.ru/docs/theory/plasma.html#plasma-frequency
Build buildPlasmaSlab(const Values &v)
{
    const double fp = v.value(QStringLiteral("fp"));
    const double thick = v.value(QStringLiteral("t"));
    Builder b(QStringLiteral("plslab"));
    const PlasmaGuide g = plasmaGuide(b, int(v.value(QStringLiteral("std"))), thick);
    b.p.description = tpl::tr("Плазменный слой в волноводе %1: fp = %2 ГГц (X-мода, "
                              "ядро tmc_rtx).",
                              "Plasma layer in waveguide %1: fp = %2 GHz (X-mode, tmc_rtx kernel).")
                          .arg(g.std.name, num(fp, 4));
    Shape &s = b.rect(tpl::tr("плазма", "plasma"), Material::Dielectric, g.x0, g.x1, g.ylo, g.yhi);
    s.plasma.hasN = true;
    s.plasma.n = concentrationForFreqGHz(fp);
    b.port(tpl::tr("вход 1", "port 1"), PortSide::Left, g.ylo, g.yhi, true);
    b.port(tpl::tr("вход 2", "port 2"), PortSide::Right, g.ylo, g.yhi, false);
    b.freqs(g.std.fLo, g.std.fHi, 21);
    b.outputs(true);
    b.time(settleTime(g.L, g.std.fLo, g.std.fc10()));

    const double t = g.x1 - g.x0;
    QVector<Row> rows;
    rows << Row{tpl::tr("Волновод", "Waveguide"), tpl::tr("%1, a = %2 мм, отсечка H10 %3 ГГц",
                                                    "%1, a = %2 mm, H10 cutoff %3 GHz")
                                                .arg(g.std.name, num(g.std.a, 5), num(g.std.fc10(), 4))};
    rows << Row{tpl::tr("Плазма", "Plasma"), tpl::tr("fp = %1 ГГц, N = %2 м⁻³, слой %3",
                                                  "fp = %1 GHz, N = %2 m^-3, layer %3")
                                              .arg(num(fp, 4), num(s.plasma.n, 4), mm(t))};
    rows << Row{tpl::tr("Отсечка волновода с плазмой", "Cutoff of the waveguide with plasma"),
                tpl::tr("√(fp² + fc²) = %1 ГГц",
                        "sqrt(fp^2 + fc^2) = %1 GHz").arg(num(std::hypot(fp, g.std.fc10()), 4))};
    for (double f : {g.std.fLo, (g.std.fLo + g.std.fHi) / 2, g.std.fHi}) {
        const double eps = 1.0 - fp * fp / (f * f);
        const double r = std::abs(rf::slabReflection(g.std.a, eps, t, f));
        rows << Row{tpl::tr("Теория, %1 ГГц", "Theory, %1 GHz").arg(num(f, 4)),
                    QStringLiteral("eps = %1, |S11| = %2").arg(num(eps), db(r))};
    }
    return b.done(rows, {tpl::tr("Концентрация пишется в СИ один раз: на частоту её "
                                 "пересчитывает само ядро на каждом шаге.",
                                 "The density is written in SI once: the kernel itself converts it "
                                 "to the frequency at each step.")});
}

/// @brief Рецепт «Замагниченная плазма» (`pl_magnetized`), X-ядро.
///
/// Слой с N и B: полосы непрозрачности X-моды.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `std` — номер сечения в `waveguides()`;
///                `fp` — плазменная частота, ГГц;
///                `fce` — циклотронная частота, ГГц;
///                `t` — толщина слоя, мм
/// @return Задание, паспорт «теория» и пояснения.
/// @see https://www.tamic.ru/docs/theory/plasma.html#cutoffs
Build buildMagnetized(const Values &v)
{
    const double fp = v.value(QStringLiteral("fp"));
    const double fce = v.value(QStringLiteral("fce"));
    const double thick = v.value(QStringLiteral("t"));
    Builder b(QStringLiteral("plmag"));
    const PlasmaGuide g = plasmaGuide(b, int(v.value(QStringLiteral("std"))), thick);
    b.p.description = tpl::tr("Замагниченный плазменный слой в волноводе %1: fp = %2 ГГц, "
                              "fce = %3 ГГц.",
                              "Magnetized plasma layer in waveguide %1: fp = %2 GHz, fce = %3 GHz.")
                          .arg(g.std.name, num(fp, 4), num(fce, 4));
    Shape &s = b.rect(tpl::tr("плазма", "plasma"), Material::Dielectric, g.x0, g.x1, g.ylo, g.yhi);
    s.plasma.hasN = true;
    s.plasma.n = concentrationForFreqGHz(fp);
    s.plasma.hasB = true;
    s.plasma.b = fieldForFreqGHz(fce);
    // Краевой слой: на границе блока ядро ополовинивает N, и у слоя N/2 своя
    // верхнегибридная частота — sqrt(fp^2/2 + fce^2) при B шире плазмы и
    // sqrt(fp^2/2 + fce^2/4) при B ровно по плазме. Слой в полосе резонирует: при
    // умолчаниях (fp = 3, fce = 1.5 ГГц) широкий B дал на 2.6 ГГц |S|^2 = 1.13, ровный —
    // 1.0006 (прогон ядром X от 26.10.2026). Берём тот вариант, чей слой вне полосы.
    const auto inBand = [&](double f) { return f >= g.std.fLo * 0.98 && f <= g.std.fHi * 1.02; };
    s.plasma.bExact = inBand(std::sqrt(fp * fp / 2.0 + fce * fce))
                      && !inBand(std::sqrt(fp * fp / 2.0 + fce * fce / 4.0));
    b.port(tpl::tr("вход 1", "port 1"), PortSide::Left, g.ylo, g.yhi, true);
    b.port(tpl::tr("вход 2", "port 2"), PortSide::Right, g.ylo, g.yhi, false);
    b.freqs(g.std.fLo, g.std.fHi, 21);
    b.outputs(true);
    b.time(settleTime(g.L, g.std.fLo, g.std.fc10(), 5.0));

    const PlasmaBands pb = plasmaBands(s.plasma.n, s.plasma.b);
    QVector<Row> rows;
    rows << Row{tpl::tr("Волновод",
                        "Waveguide"), tpl::tr("%1, a = %2 мм",
                                                    "%1, a = %2 mm").arg(g.std.name, num(g.std.a, 5))};
    rows << Row{tpl::tr("Плазма", "Plasma"), tpl::tr("N = %1 м⁻³, B = %2 Тл, слой %3",
                                                  "N = %1 m^-3, B = %2 T, layer %3")
                                              .arg(num(s.plasma.n, 4), num(s.plasma.b, 4), mm(g.x1 - g.x0))};
    rows << Row{QStringLiteral("fp / fce"), tpl::tr("%1 / %2 ГГц",
                                                    "%1 / %2 GHz").arg(num(pb.fp, 4), num(pb.fce, 4))};
    rows << Row{tpl::tr("Отсечки X-моды fL / fR", "X-mode cutoffs fL / fR"),
                tpl::tr("%1 / %2 ГГц", "%1 / %2 GHz").arg(num(pb.fL, 4), num(pb.fR, 4))};
    rows << Row{tpl::tr("Верхний гибридный fUH", "Upper hybrid fUH"), ghz(pb.fUH)};
    rows << Row{tpl::tr("Непрозрачно для X-моды", "Opaque for the X-mode"),
                tpl::tr("f < %1 ГГц и %2 < f < %3 ГГц",
                        "f < %1 GHz and %2 < f < %3 GHz").arg(num(pb.fL, 4), num(pb.fUH, 4), num(pb.fR, 4))};
    QStringList notes{tpl::tr("Подмагничивание действует только там, где есть "
                              "концентрация: поэтому N и B заданы одной фигуре.",
                              "Magnetization acts only where there is density: therefore N and B "
                              "are assigned to one shape.")};
    if (s.plasma.bExact) {
        notes << tpl::tr("Блок B здесь ровно по плазме, а не шире: краевой слой N/2 "
                         "с полным B резонировал бы на %1 ГГц, внутри полосы.",
                         "The B block here matches the plasma exactly rather than being wider: an "
                         "edge layer N/2 with full B would resonate at %1 GHz, inside the band.")
                     .arg(num(std::sqrt(fp * fp / 2.0 + fce * fce), 4));
    }
    return b.done(rows, notes);
}

/// @brief Рецепт «Плазменный столб с профилем» (`pl_column`), X-ядро.
///
/// Концентрация задана формулой от координат.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `std` — номер сечения в `waveguides()`;
///                `fp` — плазменная частота в центре, ГГц;
///                `r` — радиус, доля a
/// @return Задание, паспорт «теория» и пояснения.
/// @see https://www.tamic.ru/docs/theory/plasma.html#units
Build buildColumn(const Values &v)
{
    const double fp0 = v.value(QStringLiteral("fp"));
    const double rRel = v.value(QStringLiteral("r"));
    Builder b(QStringLiteral("plcol"));
    const PlasmaGuide g = plasmaGuide(b, int(v.value(QStringLiteral("std"))), 0.0);
    const double R = std::max(2 * g.d, rRel * g.std.a);
    const QPointF c((g.x0 + g.x1) / 2.0, (g.ylo + g.yhi) / 2.0);
    b.p.description = tpl::tr("Плазменный столб с параболическим профилем в волноводе %1: "
                              "fp в центре %2 ГГц.",
                              "Plasma column with a parabolic profile in waveguide %1: fp at the "
                              "center %2 GHz.")
                          .arg(g.std.name, num(fp0, 4));
    Shape &s = b.circle(tpl::tr("плазменный столб", "plasma column"), Material::Dielectric, c, R);
    s.plasma.hasN = true;
    const double n0 = concentrationForFreqGHz(fp0);
    s.plasma.n = n0;
    // Профиль от центра: N0 * (1 - (r/Rp)^2), r — расстояние от анкера, то есть от центра.
    //
    // ⚠️ Rp чуть больше радиуса столба: на самом краю концентрация обязана остаться
    // ПОЛОЖИТЕЛЬНОЙ. При N = 0 в узле ядро останавливает шаг сообщением «abnormal
    // function» (проверено прогоном 2026-09-23: r = R давало ровно ноль).
    const double rp = R * 1.1;
    s.plasma.nFormula = QStringLiteral("%1*(1-(r/%2)*(r/%2))").arg(formatNumber(n0), formatNumber(rp));
    b.port(tpl::tr("вход 1", "port 1"), PortSide::Left, g.ylo, g.yhi, true);
    b.port(tpl::tr("вход 2", "port 2"), PortSide::Right, g.ylo, g.yhi, false);
    b.freqs(g.std.fLo, g.std.fHi, 21);
    b.outputs(true);
    b.time(settleTime(g.L, g.std.fLo, g.std.fc10(), 5.0));
    return b.done({Row{tpl::tr("Столб",
                               "Column"), tpl::tr("радиус %1 (%2 a)",
                                                        "radius %1 (%2 a)").arg(mm(R), num(R / g.std.a))},
                   Row{tpl::tr("Профиль", "Profile"),
                       tpl::tr("N = N0 (1 - (r/1.1R)²), N0 = %1 м⁻³; на краю N = 0.17 N0 — "
                               "нулевую концентрацию ядро не принимает",
                               "N = N0 (1 - (r/1.1R)^2), N0 = %1 m^-3; at the edge N = 0.17 N0: the "
                               "kernel does not accept zero density")
                           .arg(num(n0, 4))},
                   Row{tpl::tr("fp в центре", "fp at the center"), ghz(fp0)},
                   Row{tpl::tr("Ожидаемо", "Expected"),
                       tpl::tr("столб отражает сильнее на частотах ниже fp: там его середина "
                               "непрозрачна",
                               "the column reflects more strongly at frequencies below fp: there "
                               "its middle is opaque")}},
                  {tpl::tr("Концентрация задана формулой от координат — ядро считает её в "
                           "каждом узле. Так же заданы профили плотности в авторских "
                           "токамаковых заданиях.",
                           "The density is given by a formula of the coordinates: the kernel "
                           "computes it at each node. Density profiles in the author's tokamak "
                           "tasks are given the same way.")});
}

// ------------------------------------------------------------ ЭПР

/// Открытая сцена рассеяния: коробка с поглотителем сверху, снизу и справа, плоская
/// волна слева почти на всю высоту.
/// @param[in,out] b  сборщик; `b.p.delta` должен быть задан
/// @param[in]     X  размер области по X, мм
/// @param[in]     Y  размер области по Y, мм
void scatterScene(Builder &b, double X, double Y)
{
    const double d = b.p.delta;
    b.p.box = {0.0, X, 0.0, Y};
    b.absorberFrame(false, true, true, true);
    const double t = 5 * d;
    b.port(tpl::tr("плоская волна", "plane wave"), PortSide::Left, t + d, Y - t - d, true, PortMode::Plane);
}

/// @brief Рецепт «ЭПР металлического цилиндра» (`rcs_cylinder`).
///
/// Эталон: точное решение рядом Ми; варианты «без объекта» и «цилиндр».
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `r` — радиус, мм;
///                `flo`, `fhi` — границы полосы, ГГц
/// @return Задание, паспорт «теория» и пояснения.
Build buildCylinder(const Values &v)
{
    const double a = v.value(QStringLiteral("r"));
    const double fLo = v.value(QStringLiteral("flo")), fHi = v.value(QStringLiteral("fhi"));
    const double lamMax = rf::lambda0(fLo), lamMin = rf::lambda0(fHi);

    Builder b(QStringLiteral("cyl"));
    b.p.delta = chooseDelta(lamMin, a, 20.0, 8.0);
    const double d = b.p.delta;
    const double X = snap(2 * a + 3 * lamMax, d);
    const double Y = snap(2 * a + 3 * lamMax, d);
    b.p.description = tpl::tr("Рассеяние плоской волны на металлическом цилиндре радиусом "
                              "%1 мм.",
                              "Scattering of a plane wave by a metal cylinder of radius %1 mm.").arg(num(a, 4));
    scatterScene(b, X, Y);
    const int cyl = b.circle(tpl::tr("цилиндр", "cylinder"), Material::Metal,
                             QPointF(snap(X / 2, d), snap(Y / 2, d)), a).id;
    Variant empty;
    empty.name = tpl::tr("без объекта", "no object");
    empty.tag = QStringLiteral("ref");
    empty.disabledShapes = {cyl};
    Variant body;
    body.name = tpl::tr("цилиндр", "cylinder");
    body.tag = QStringLiteral("body");
    b.p.variants << empty << body;
    b.freqs(fLo, fHi, 13);
    b.outputs(true);
    b.time(settleTime(X * 2, fLo, 0.0));

    QVector<Row> rows;
    rows << Row{tpl::tr("Цилиндр", "Cylinder"), tpl::tr("радиус %1, ka = %2 ... %3", "radius %1, ka = %2 ... %3")
                                               .arg(mm(a), num(2 * kPi * a / lamMax), num(2 * kPi * a / lamMin))};
    for (double f : {fLo, (fLo + fHi) / 2, fHi}) {
        const double s = rf::cylinderEchoWidth(a, f, true);
        rows << Row{tpl::tr("Ряд Ми (E вдоль оси), %1 ГГц",
                            "Mie series (E along the axis), %1 GHz").arg(num(f, 4)),
                    tpl::tr("σ = %1 мм (%2 дБ·м)",
                            "sigma = %1 mm (%2 dB*m)").arg(num(s, 4), num(10 * std::log10(s / 1000.0), 3))};
    }
    rows << Row{tpl::tr("Оценка по S11", "Estimate from S11"),
                tpl::tr("σ ≈ 2π W² |S11 - S11 опоры|² / λ, W — апертура входа",
                        "sigma approx. 2 pi W^2 |S11 - S11 ref|^2 / lambda, W = port aperture")};
    return b.done(rows, {tpl::tr("Погонная ЭПР — рассеяние бесконечно длинного цилиндра "
                                 "(задача двумерная). Оценка по S11 — порядок величины, "
                                 "а не калибровка: область конечна, поглотитель "
                                 "отражает на уровне -30 дБ.",
                                 "Echo width per unit length: scattering by an infinitely long "
                                 "cylinder (the problem is two-dimensional). The estimate from S11 "
                                 "gives the order of magnitude, not a calibration: the domain is "
                                 "finite, and the absorber reflects at about -30 dB.")});
}

/// Сцена ЭПР с поворотом объекта. Углы — от \p a0 до \p a1 с шагом \p step.
///
/// Включает `Project::sweep` с опорным шагом без объекта; углы округляются до 0.001°.
/// @param[in,out] b      сборщик
/// @param[in]     ids    номера поворачиваемых фигур
/// @param[in]     pivot  ось поворота, мм
/// @param[in]     a0     начальный угол, градусы
/// @param[in]     a1     конечный угол, градусы (включительно)
/// @param[in]     step   шаг по углу, градусы
void rotateObject(Builder &b, const QVector<int> &ids, const QPointF &pivot, double a0, double a1,
                  double step)
{
    b.p.sweep.enabled = true;
    b.p.sweep.shapeIds = ids;
    b.p.sweep.pivot = pivot;
    b.p.sweep.referenceStep = true;
    for (double a = a0; a <= a1 + 1e-9; a += step)
        b.p.sweep.angles << std::round(a * 1000.0) / 1000.0;
}

/// @brief Рецепт «ЭПР полосы: поворот по шагам» (`rcs_strip`).
///
/// Угловой свип: объект поворачивается, источник стоит.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `w` — ширина полосы, в длинах волны;
///                `f0` — частота, ГГц;
///                `step` — шаг по углу, градусы
/// @return Задание, паспорт «теория» и пояснения.
Build buildStrip(const Values &v)
{
    const double f0 = v.value(QStringLiteral("f0"));
    const double lam = rf::lambda0(f0);
    const double w = v.value(QStringLiteral("w")) * lam;
    const double step = v.value(QStringLiteral("step"));

    Builder b(QStringLiteral("strip"));
    b.p.delta = chooseDelta(lam, w, 30.0, 8.0);
    const double d = b.p.delta;
    const double X = snap(w + 4 * lam, d);
    const double Y = snap(w + 4 * lam, d);
    b.p.description = tpl::tr("ЭПР металлической полосы шириной %1 мм на %2 ГГц: поворот "
                              "по шагам.",
                              "RCS of a metal strip of width %1 mm at %2 GHz: rotation by steps.").arg(num(w, 4), num(f0, 4));
    scatterScene(b, X, Y);
    const QPointF c(snap(X / 2, d), snap(Y / 2, d));
    const int strip = b.polyline(tpl::tr("полоса", "strip"), Material::Metal,
                                 {QPointF(c.x(), c.y() - w / 2), QPointF(c.x(), c.y() + w / 2)}).id;
    rotateObject(b, {strip}, c, 0.0, 180.0, step);
    b.freq(f0);
    b.outputs(true);
    b.time(settleTime(X * 2, f0, 0.0));

    QVector<Row> rows;
    rows << Row{tpl::tr("Полоса", "Strip"), tpl::tr("ширина %1 (%2 λ)",
                                                  "width %1 (%2 lambda)").arg(mm(w), num(w / lam))};
    rows << Row{tpl::tr("Углы", "Angles"), tpl::tr("0...180° с шагом %1°, шагов %2",
                                                "0...180 deg in steps of %1 deg, %2 steps")
                                            .arg(num(step)).arg(b.p.stepCount())};
    for (double rot : {0.0, 10.0, 30.0, 60.0}) {
        const double s = rf::stripEchoWidth(w, f0, 90.0 - rot);
        rows << Row{tpl::tr("Физ. оптика, поворот %1°", "Phys. optics, rotation %1 deg").arg(num(rot)),
                    tpl::tr("σ = %1 мм (%2 дБ·м)",
                            "sigma = %1 mm (%2 dB*m)").arg(num(s, 4), num(10 * std::log10(std::max(s, 1e-9) / 1000.0), 3))};
    }
    return b.done(rows, {tpl::tr("При повороте 0° полоса стоит поперёк волны — зеркальное "
                                 "отражение, максимум ЭПР k w². Точка .s подписана углом: "
                                 "в TMCGROUT по горизонтали будет угол, а не частота.",
                                 "At 0 deg rotation the strip stands across the wave: specular "
                                 "reflection, maximum RCS k w^2. The .s point is labelled by the "
                                 "angle: in TMCGROUT the horizontal axis will show the angle, not "
                                 "the frequency.")});
}

/// @brief Рецепт «ЭПР профиля крыла» (`rcs_airfoil`).
///
/// Профиль NACA, угловой свип 0...180°.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `c` — хорда, в длинах волны;
///                `t` — толщина, % хорды;
///                `f0` — частота, ГГц;
///                `step` — шаг по углу, градусы
/// @return Задание, паспорт «теория» и пояснения.
Build buildAirfoil(const Values &v)
{
    const double f0 = v.value(QStringLiteral("f0"));
    const double lam = rf::lambda0(f0);
    const double chord = v.value(QStringLiteral("c")) * lam;
    const double thick = v.value(QStringLiteral("t")) / 100.0;
    const double step = v.value(QStringLiteral("step"));

    Builder b(QStringLiteral("airfoil"));
    b.p.delta = chooseDelta(lam, chord * thick, 30.0, 6.0);
    const double d = b.p.delta;
    const double X = snap(chord + 4 * lam, d);
    const double Y = snap(chord + 4 * lam, d);
    b.p.description = tpl::tr("ЭПР профиля NACA 00%1 с хордой %2 мм на %3 ГГц: поворот по "
                              "шагам.",
                              "RCS of airfoil NACA 00%1 with chord %2 mm at %3 GHz: rotation by "
                              "steps.").arg(int(std::lround(thick * 100))).arg(num(chord, 4), num(f0, 4));
    scatterScene(b, X, Y);
    const QPointF c(snap(X / 2, d), snap(Y / 2, d));
    // NACA 4-digit, симметричный: y = 5 t c [0.2969 √x - 0.1260 x - 0.3516 x² + 0.2843 x³ - 0.1036 x⁴]
    // (последний коэффициент — вариант с замкнутой задней кромкой). Носок — навстречу волне.
    QVector<QPointF> upper, lower;
    const int N = 24;
    for (int i = 0; i <= N; ++i) {
        const double beta = kPi * i / N;
        const double xr = (1.0 - std::cos(beta)) / 2.0; // сгущение к кромкам
        const double yt = 5.0 * thick * (0.2969 * std::sqrt(xr) - 0.1260 * xr - 0.3516 * xr * xr
                                         + 0.2843 * xr * xr * xr - 0.1036 * xr * xr * xr * xr);
        upper << QPointF(c.x() - chord / 2 + xr * chord, c.y() + yt * chord);
        lower << QPointF(c.x() - chord / 2 + xr * chord, c.y() - yt * chord);
    }
    QVector<QPointF> contour = upper;
    for (int i = lower.size() - 2; i >= 1; --i)
        contour << lower.at(i);
    const int body = b.polygon(tpl::tr("профиль", "airfoil"), Material::Metal, contour).id;
    rotateObject(b, {body}, c, 0.0, 180.0, step);
    b.freq(f0);
    b.outputs(true);
    b.time(settleTime(X * 2, f0, 0.0));

    return b.done({Row{tpl::tr("Профиль", "Airfoil"), tpl::tr("NACA 00%1, хорда %2 (%3 λ)",
                                                          "NACA 00%1, chord %2 (%3 lambda)")
                                                     .arg(int(std::lround(thick * 100))).arg(mm(chord), num(chord / lam))},
                   Row{tpl::tr("Углы", "Angles"), tpl::tr("0...180° с шагом %1°, шагов %2",
                                                       "0...180 deg in steps of %1 deg, %2 steps")
                                                   .arg(num(step)).arg(b.p.stepCount())},
                   Row{tpl::tr("Ожидаемо", "Expected"),
                       tpl::tr("максимум ЭПР при 90° (волна на плоскую сторону), минимум — "
                               "с носка и с кромки",
                               "maximum RCS at 90 deg (wave onto the flat side), minimum from the "
                               "nose and from the trailing edge")}},
                  {tpl::tr("Точной формулы для профиля нет — результат даёт ядро. "
                           "На сетке повёрнутый профиль — «лесенка», поэтому шаг сетки "
                           "λ/30.",
                           "There is no exact formula for an airfoil; the kernel gives the result. "
                           "On the grid a rotated airfoil is a \"staircase\", so the grid step is "
                           "lambda/30.")});
}

/// @brief Рецепт «Цилиндр в плазменной оболочке» (`rcs_coated`), X-ядро.
///
/// Как плазма меняет рассеяние: три варианта сразу.
/// @param[in] v  значения карточки (полный набор после `make()`):
///                `r` — радиус цилиндра, мм;
///                `t` — толщина оболочки, мм;
///                `fp` — плазменная частота оболочки, ГГц;
///                `flo`, `fhi` — границы полосы, ГГц
/// @return Задание, паспорт «теория» и пояснения.
Build buildCoated(const Values &v)
{
    const double a = v.value(QStringLiteral("r"));
    const double t = v.value(QStringLiteral("t"));
    const double fp = v.value(QStringLiteral("fp"));
    const double fLo = v.value(QStringLiteral("flo")), fHi = v.value(QStringLiteral("fhi"));
    const double lamMax = rf::lambda0(fLo), lamMin = rf::lambda0(fHi);

    Builder b(QStringLiteral("coated"));
    b.p.kernel = Kernel::X;
    b.p.delta = chooseDelta(lamMin, t, 20.0, 4.0);
    const double d = b.p.delta;
    const double X = snap(2 * (a + t) + 3 * lamMax, d);
    const double Y = X;
    b.p.description = tpl::tr("Металлический цилиндр в плазменной оболочке: радиус %1 мм, "
                              "оболочка %2 мм, fp = %3 ГГц (X-мода).",
                              "Metal cylinder in a plasma sheath: radius %1 mm, sheath %2 mm, fp = "
                              "%3 GHz (X-mode).")
                          .arg(num(a, 4), num(t, 4), num(fp, 4));
    scatterScene(b, X, Y);
    const QPointF c(snap(X / 2, d), snap(Y / 2, d));
    const int cyl = b.circle(tpl::tr("цилиндр", "cylinder"), Material::Metal, c, a).id;
    Shape &sh = b.circle(tpl::tr("плазменная оболочка", "plasma sheath"), Material::Dielectric, c, a + t);
    sh.rMin = a;
    sh.plasma.hasN = true;
    sh.plasma.n = concentrationForFreqGHz(fp);
    const int sheath = sh.id;
    Variant empty;
    empty.name = tpl::tr("без объекта", "no object");
    empty.tag = QStringLiteral("ref");
    empty.disabledShapes = {cyl, sheath};
    Variant bare;
    bare.name = tpl::tr("металл", "metal");
    bare.tag = QStringLiteral("metal");
    bare.disabledShapes = {sheath};
    Variant coated;
    coated.name = tpl::tr("металл + плазма", "metal + plasma");
    coated.tag = QStringLiteral("plasma");
    b.p.variants << empty << bare << coated;
    b.freqs(fLo, fHi, 9);
    b.outputs(true);
    b.time(settleTime(X * 2, fLo, 0.0));

    QVector<Row> rows;
    rows << Row{tpl::tr("Цилиндр / оболочка",
                        "Cylinder / sheath"), tpl::tr("радиус %1, оболочка %2",
                                                              "radius %1, sheath %2").arg(mm(a), mm(t))};
    rows << Row{tpl::tr("Плазма",
                        "Plasma"), tpl::tr("fp = %1 ГГц, N = %2 м⁻³",
                                                  "fp = %1 GHz, N = %2 m^-3").arg(num(fp, 4), num(sh.plasma.n, 4))};
    for (double f : {fLo, fHi}) {
        const double s = rf::cylinderEchoWidth(a, f, false);
        rows << Row{tpl::tr("Голый цилиндр, ряд Ми (E ⟂ оси), %1 ГГц",
                            "Bare cylinder, Mie series (E perpendicular to the axis), %1 GHz").arg(num(f, 4)),
                    tpl::tr("σ = %1 мм", "sigma = %1 mm").arg(num(s, 4))};
    }
    rows << Row{tpl::tr("Ожидаемо", "Expected"),
                tpl::tr("разница вариантов «металл» и «металл + плазма» — вклад оболочки. "
                        "Для тонкой оболочки при E в плоскости (X-мода) она невелика: "
                        "прогон ядра при умолчаниях даёт расхождение меньше 1 дБ — "
                        "утолщайте оболочку или поднимайте fp",
                        "the difference between the \"metal\" and \"metal + plasma\" variants is "
                        "the contribution of the sheath. For a thin sheath with E in the plane "
                        "(X-mode) it is small: a kernel run at the defaults gives a difference "
                        "below 1 dB; make the sheath thicker or raise fp")};
    return b.done(rows, {tpl::tr("Три варианта одним заданием: без объекта (опора), голый "
                                 "металл и металл в плазме — так видно, что меняет "
                                 "оболочка. X-мода: поле E лежит в плоскости рисунка.",
                                 "Three variants in one task: no object (reference), bare metal and "
                                 "metal in plasma; this shows what the sheath changes. X-mode: the "
                                 "E field lies in the plane of the drawing.")});
}

} // namespace

void addMedia(QVector<Sample> *out)
{
    const QString sec = QStringLiteral("media");
    {
        Sample s;
        s.id = QStringLiteral("med_slab");
        s.section = sec;
        s.title = tpl::tr("Плоская волна на диэлектрическом слое", "Plane wave on a dielectric layer");
        s.summary = tpl::tr("Нормальное падение: точная формула Френеля для проверки",
                            "Normal incidence: the exact Fresnel formula for checking");
        s.standard = tpl::tr("формулы Френеля, слой в однородной среде",
                             "Fresnel formulas, layer in a homogeneous medium");
        s.help = tpl::tr(
                     "Плоская волна падает нормально на диэлектрический слой. Отражения от двух "
                     "граней интерферируют: на частотах, где толщина кратна половине длины волны "
                     "в слое, слой становится прозрачным. Формула точная — паспорт даёт |S11| и "
                     "|S21| на трёх частотах.",
                     "A plane wave is normally incident on a dielectric layer. Reflections from the "
                     "two faces interfere: at frequencies where the thickness is a multiple of half "
                     "the wavelength in the layer, the layer becomes transparent. The formula is "
                     "exact; the datasheet gives |S11| and |S21| at three frequencies.")
                 + helpTem();
        s.params = {Param::number(QStringLiteral("eps"), tpl::tr("Проницаемость eps",
                                                                 "Permittivity eps"), {}, 4.0,
                                  1.1, 30.0, 0.1, 2),
                    Param::number(QStringLiteral("t"), tpl::tr("Толщина", "Thickness"), tpl::tr("мм", "mm"),
                                  10.0, 0.5, 200.0, 0.5, 2),
                    Param::number(QStringLiteral("flo"), tpl::tr("Частота от",
                                                                 "Frequency from"), tpl::tr("ГГц", "GHz"),
                                  2.0, 0.3, 50.0, 0.5, 2),
                    Param::number(QStringLiteral("fhi"), tpl::tr("Частота до",
                                                                 "Frequency to"), tpl::tr("ГГц", "GHz"),
                                  12.0, 0.5, 100.0, 0.5, 2)};
        s.build = buildSlab;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("med_grad");
        s.section = sec;
        s.title = tpl::tr("Плавный переход: eps формулой", "Smooth transition: eps by formula");
        s.summary = tpl::tr("Профиль eps от координат против резкой границы",
                            "An eps profile of the coordinates versus a sharp boundary");
        s.standard = tpl::tr("неоднородная среда, профиль формулой",
                             "inhomogeneous medium, profile by formula");
        s.help = tpl::tr(
                     "Переход из вакуума в плотный диэлектрик. Резкая граница отражает на всех "
                     "частотах одинаково, а плавный — линейный по длине — профиль тем лучше, чем "
                     "длиннее переход по сравнению с волной. Профиль задан формулой от x: так в "
                     "ядро уходит любая неоднородная eps, а на холсте она видна градиентом.\n\n"
                     "Ядро на первом шаге сохранит посчитанный профиль в файл grad.eps — этот же "
                     "файл можно потом подключить блоком FILE.",
                     "A transition from vacuum to a dense dielectric. A sharp boundary reflects "
                     "equally at all frequencies, while a smooth profile, linear along the length, "
                     "is the better the longer the transition is compared with the wavelength. The "
                     "profile is given by a formula of x: this is how any inhomogeneous eps is "
                     "passed to the kernel, and on the canvas it is shown as a gradient.\n\nAt the "
                     "first step the kernel saves the computed profile to the file grad.eps; the "
                     "same file can later be connected by a FILE block.")
                 + helpTem();
        s.params = {Param::number(QStringLiteral("eps"), tpl::tr("eps за переходом",
                                                                 "eps beyond the transition"), {}, 6.0,
                                  1.5, 20.0, 0.5, 1),
                    Param::number(QStringLiteral("len"), tpl::tr("Длина перехода", "Transition length"),
                                  tpl::tr("λ", "lambda"), 1.0, 0.2, 5.0, 0.1, 1),
                    Param::number(QStringLiteral("f0"), tpl::tr("Частота f0",
                                                                "Frequency f0"), tpl::tr("ГГц", "GHz"),
                                  5.0, 0.5, 50.0, 0.5, 2)};
        s.build = buildGrad;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("med_eps_file");
        s.section = sec;
        s.title = tpl::tr("Среда из готового файла .eps", "Medium from an .eps file");
        s.summary = tpl::tr("Распределение eps, посчитанное заранее, подключается блоком FILE",
                            "A precomputed eps distribution is connected by a FILE block");
        s.standard = tpl::tr("формат распределения eps ядра TMC (V2.00)",
                             "eps distribution format of the TMC kernel (V2.00)");
        s.help = tpl::tr(
                     "Распределение eps не обязательно задавать формулой: его можно посчитать "
                     "заранее — своей программой или самим ядром — и подключить готовым файлом. "
                     "Здесь программа сама пишет файл lens.eps (гауссов бугор проницаемости) "
                     "рядом с заданием, а задание читает его блоком FILE.\n\nКоординаты блока "
                     "ядро не читает — сетка лежит в самом файле, а на место распределение "
                     "ставит анкер. Шаг сетки в файле обязан совпадать с DELTA задания.\n\n"
                     "Внимание: блок FILE читает только ядро, собранное после 6 сентября 2026 "
                     "(исправление бага #19).",
                     "An eps distribution does not have to be given by a formula: it can be "
                     "computed in advance, by your own program or by the kernel itself, and "
                     "connected as a ready file. Here the program itself writes the file lens.eps "
                     "(a Gaussian bump of permittivity) next to the task, and the task reads it "
                     "with a FILE block.\n\nThe kernel does not read the block coordinates: the "
                     "grid is stored in the file itself, and the anchor puts the distribution in "
                     "place. The grid step in the file must match the DELTA of the task.\n\nNote: "
                     "the FILE block is read only by a kernel built after 6 September 2026 (fix of "
                     "bug #19).")
                 + helpOpen();
        s.params = {Param::number(QStringLiteral("eps"), tpl::tr("eps в центре",
                                                                 "eps at the center"), {}, 3.0, 1.2,
                                  10.0, 0.1, 2),
                    Param::number(QStringLiteral("f0"), tpl::tr("Частота",
                                                                "Frequency"), tpl::tr("ГГц", "GHz"),
                                  10.0, 1.0, 50.0, 0.5, 2)};
        s.build = buildEpsFile;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("med_luneburg");
        s.section = sec;
        s.title = tpl::tr("Линза Люнеберга", "Luneburg lens");
        s.summary = tpl::tr("Градиентная линза: eps = 2 - (r/R)² фокусирует плоскую волну",
                            "Gradient lens: eps = 2 - (r/R)^2 focuses a plane wave");
        s.standard = tpl::tr("линза Люнеберга", "Luneburg lens");
        s.help = tpl::tr(
                     "Круглая линза с проницаемостью, плавно спадающей от 2 в центре до 1 на "
                     "краю. Плоская волна с любого направления собирается в точку на "
                     "противоположном краю линзы. Профиль задан формулой от расстояния r до "
                     "центра — на холсте он виден градиентом, а поле первого шага в FieldView (выгрузку "
                     "поля включите на странице «Вывод данных») "
                     "показывает фокусировку.",
                     "A round lens with permittivity smoothly falling from 2 at the center to 1 at "
                     "the edge. A plane wave from any direction converges to a point on the "
                     "opposite edge of the lens. The profile is given by a formula of the distance "
                     "r to the center: on the canvas it is shown as a gradient, and the first-step "
                     "field in FieldView (enable the field export on the \"Output\" page) shows the "
                     "focusing.")
                 + helpOpen();
        s.params = {Param::number(QStringLiteral("r"), tpl::tr("Радиус", "Radius"), tpl::tr("λ", "lambda"),
                                  2.5, 1.0, 6.0, 0.5, 1),
                    Param::number(QStringLiteral("f0"), tpl::tr("Частота",
                                                                "Frequency"), tpl::tr("ГГц", "GHz"),
                                  10.0, 1.0, 50.0, 0.5, 2)};
        s.build = buildLuneburg;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("pl_slab");
        s.section = sec;
        s.title = tpl::tr("Плазменный слой в волноводе", "Plasma layer in a waveguide");
        s.summary = tpl::tr("Изотропная плазма: отсечка √(fp² + fc²), точная теория",
                            "Isotropic plasma: cutoff sqrt(fp^2 + fc^2), exact theory");
        s.standard = tpl::tr("холодная бесстолкновительная плазма", "cold collisionless plasma");
        s.kernel = Kernel::X;
        s.help = tpl::tr(
                     "Слой плазмы поперёк волновода (ядро tmc_rtx, X-мода). Проницаемость "
                     "плазмы eps = 1 - fp²/f² отрицательна ниже плазменной частоты — там слой "
                     "отражает. Волновод с плазмой пропускает волну выше √(fp² + fc²). "
                     "Концентрация N пересчитывается из fp и пишется в СИ; на каждую частоту её "
                     "переводит само ядро.",
                     "A plasma layer across the waveguide (tmc_rtx kernel, X-mode). The plasma "
                     "permittivity eps = 1 - fp^2/f^2 is negative below the plasma frequency, where "
                     "the layer reflects. A waveguide with plasma passes the wave above sqrt(fp^2 + "
                     "fc^2). The density N is computed from fp and written in SI; the kernel itself "
                     "converts it for each frequency.");
        s.params = {stdParam(stdIndex("WR-284")),
                    Param::number(QStringLiteral("fp"), tpl::tr("Плазменная частота fp",
                                                                "Plasma frequency fp"),
                                  tpl::tr("ГГц", "GHz"), 3.0, 0.1, 100.0, 0.1, 2),
                    Param::number(QStringLiteral("t"), tpl::tr("Толщина слоя",
                                                               "Layer thickness"), tpl::tr("мм", "mm"),
                                  20.0, 1.0, 500.0, 1.0, 1)};
        s.build = buildPlasmaSlab;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("pl_magnetized");
        s.section = sec;
        s.title = tpl::tr("Замагниченная плазма", "Magnetized plasma");
        s.summary = tpl::tr("Слой с N и B: полосы непрозрачности X-моды",
                            "Layer with N and B: X-mode opacity bands");
        s.standard = tpl::tr("холодная замагниченная плазма, X-мода", "cold magnetized plasma, X-mode");
        s.kernel = Kernel::X;
        s.help = tpl::tr(
                     "Та же постановка, что в лабораторной работе ЛР2: слой плазмы в волноводе, "
                     "подмагниченный полем B, перпендикулярным плоскости. У X-моды две полосы "
                     "непрозрачности: ниже fL и между fUH и fR. Все характерные частоты — в "
                     "паспорте.\n\nПодмагничивание действует только в области с концентрацией, "
                     "поэтому N и B заданы одной и той же фигуре.",
                     "The same setup as in laboratory work Lab 2: a plasma layer in a waveguide, "
                     "magnetized by a field B perpendicular to the plane. The X-mode has two "
                     "opacity bands: below fL and between fUH and fR. All characteristic "
                     "frequencies are in the datasheet.\n\nMagnetization acts only in the region "
                     "with density, so N and B are assigned to the same shape.");
        s.params = {stdParam(stdIndex("WR-284")),
                    Param::number(QStringLiteral("fp"), tpl::tr("Плазменная частота fp",
                                                                "Plasma frequency fp"),
                                  tpl::tr("ГГц", "GHz"), 3.0, 0.1, 100.0, 0.1, 2),
                    Param::number(QStringLiteral("fce"), tpl::tr("Циклотронная частота fce",
                                                                 "Cyclotron frequency fce"),
                                  tpl::tr("ГГц", "GHz"), 1.5, 0.1, 100.0, 0.1, 2),
                    Param::number(QStringLiteral("t"), tpl::tr("Толщина слоя",
                                                               "Layer thickness"), tpl::tr("мм", "mm"),
                                  20.0, 1.0, 500.0, 1.0, 1)};
        s.build = buildMagnetized;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("pl_column");
        s.section = sec;
        s.title = tpl::tr("Плазменный столб с профилем", "Plasma column with a profile");
        s.summary = tpl::tr("Концентрация задана формулой от координат",
                            "Density given by a formula of the coordinates");
        s.standard = tpl::tr("параболический профиль плотности", "parabolic density profile");
        s.kernel = Kernel::X;
        s.help = tpl::tr(
                     "Плазменный столб в волноводе с плотностью, спадающей к краю по параболе. "
                     "Концентрация задана не числом, а формулой от расстояния до центра — ядро "
                     "считает её в каждом узле. Так заданы профили плотности в авторских "
                     "заданиях по токамакам.",
                     "A plasma column in a waveguide with density falling parabolically towards the "
                     "edge. The density is given not by a number but by a formula of the distance "
                     "to the center: the kernel computes it at each node. Density profiles in the "
                     "author's tokamak tasks are given this way.");
        s.params = {stdParam(stdIndex("WR-284")),
                    Param::number(QStringLiteral("fp"), tpl::tr("fp в центре",
                                                                "fp at the center"), tpl::tr("ГГц", "GHz"),
                                  4.0, 0.1, 100.0, 0.1, 2),
                    Param::number(QStringLiteral("r"), tpl::tr("Радиус, доля a",
                                                               "Radius, fraction of a"), {}, 0.2, 0.05,
                                  0.45, 0.05, 2)};
        s.build = buildColumn;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("rcs_cylinder");
        s.section = sec;
        s.title = tpl::tr("ЭПР металлического цилиндра", "RCS of a metal cylinder");
        s.summary = tpl::tr("Эталон: точное решение рядом Ми", "Reference: exact Mie series solution");
        s.standard = tpl::tr("ряд Ми для идеально проводящего цилиндра",
                             "Mie series for a perfectly conducting cylinder");
        s.help = tpl::tr(
                     "Плоская волна рассеивается на металлическом цилиндре. Для цилиндра есть "
                     "точное решение — ряд Ми, — поэтому это эталон, на котором проверяют саму "
                     "постановку расчёта ЭПР. Поворачивать круглое тело бессмысленно, поэтому "
                     "здесь свип по частоте: вариант без объекта (опора) и с цилиндром.\n\n"
                     "ЭПР двумерная — погонная, в дБ относительно метра.",
                     "A plane wave is scattered by a metal cylinder. The cylinder has an exact "
                     "solution, the Mie series, so this is the reference used to check the RCS "
                     "setup itself. Rotating a round body makes no sense, so here a frequency sweep "
                     "is used: a variant without the object (reference) and with the "
                     "cylinder.\n\nThe RCS is two-dimensional: per unit length, in dB relative to "
                     "one metre.")
                 + helpOpen();
        s.params = {Param::number(QStringLiteral("r"), tpl::tr("Радиус", "Radius"), tpl::tr("мм", "mm"),
                                  20.0, 2.0, 500.0, 1.0, 1),
                    Param::number(QStringLiteral("flo"), tpl::tr("Частота от",
                                                                 "Frequency from"), tpl::tr("ГГц", "GHz"),
                                  2.0, 0.3, 50.0, 0.5, 2),
                    Param::number(QStringLiteral("fhi"), tpl::tr("Частота до",
                                                                 "Frequency to"), tpl::tr("ГГц", "GHz"),
                                  8.0, 0.5, 100.0, 0.5, 2)};
        s.build = buildCylinder;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("rcs_strip");
        s.section = sec;
        s.title = tpl::tr("ЭПР полосы: поворот по шагам", "RCS of a strip: rotation by steps");
        s.summary = tpl::tr("Угловой свип: объект поворачивается, источник стоит",
                            "Angle sweep: the object rotates, the source stays");
        s.standard = tpl::tr("физическая оптика", "physical optics");
        s.help = tpl::tr(
                     "Моностатическая ЭПР металлической полосы в зависимости от угла. Плоская "
                     "волна приходит всегда слева, а полоса на каждом шаге повёрнута на следующий "
                     "угол — повернуть сам источник язык не позволяет, а для рассеяния важен "
                     "только угол между волной и телом.\n\nШаг описан ОДИН раз макросом с углом в "
                     "параметре; первым идёт шаг без объекта (опора). Точка .s подписана углом, "
                     "поэтому TMCGROUT покажет зависимость от угла. Угол 0 подписан как 360: "
                     "точку с подписью 0 ядро теряет.",
                     "Monostatic RCS of a metal strip versus angle. The plane wave always comes "
                     "from the left, and at each step the strip is rotated to the next angle: the "
                     "language does not allow rotating the source itself, and for scattering only "
                     "the angle between the wave and the body matters.\n\nThe step is described "
                     "ONCE by a macro with the angle as a parameter; the first step is without the "
                     "object (reference). The .s point is labelled by the angle, so TMCGROUT shows "
                     "the dependence on the angle. Angle 0 is labelled as 360: the kernel loses a "
                     "point labelled 0.")
                 + helpOpen();
        s.params = {Param::number(QStringLiteral("w"), tpl::tr("Ширина полосы",
                                                               "Strip width"), tpl::tr("λ",
                                                                                                "lambda"),
                                  2.0, 0.5, 10.0, 0.5, 1),
                    Param::number(QStringLiteral("f0"), tpl::tr("Частота",
                                                                "Frequency"), tpl::tr("ГГц", "GHz"),
                                  5.0, 0.5, 50.0, 0.5, 2),
                    // От 2°: 0...180 при шаге 2 — 91 угол, а больше 99 точек .s не примет.
                    Param::number(QStringLiteral("step"), tpl::tr("Шаг по углу", "Angle step"),
                                  tpl::tr("град.", "deg."), 5.0, 2.0, 45.0, 1.0, 0)};
        s.build = buildStrip;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("rcs_airfoil");
        s.section = sec;
        s.title = tpl::tr("ЭПР профиля крыла", "RCS of a wing airfoil");
        s.summary = tpl::tr("Профиль NACA, угловой свип 0...180°", "NACA airfoil, angle sweep 0...180 deg");
        s.standard = tpl::tr("NACA 4-digit (симметричный профиль)", "NACA 4-digit (symmetric airfoil)");
        s.help = tpl::tr(
                     "Сечение крыла (профиль NACA 00xx) из металла, повёрнутое по шагам "
                     "относительно падающей волны. Точной формулы нет — это как раз та задача, "
                     "ради которой ЭПР считают численно. Профиль задан контуром из вершин и "
                     "поворачивается целиком.",
                     "A metal wing section (NACA 00xx airfoil) rotated by steps relative to the "
                     "incident wave. There is no exact formula: this is exactly the kind of problem "
                     "for which RCS is computed numerically. The airfoil is given by a polygon of "
                     "vertices and rotates as a whole.")
                 + helpOpen();
        s.params = {Param::number(QStringLiteral("c"), tpl::tr("Хорда", "Chord"), tpl::tr("λ", "lambda"),
                                  3.0, 1.0, 10.0, 0.5, 1),
                    Param::number(QStringLiteral("t"), tpl::tr("Толщина",
                                                               "Thickness"), tpl::tr("% хорды",
                                                                                          "% of chord"),
                                  12.0, 6.0, 24.0, 1.0, 0),
                    Param::number(QStringLiteral("f0"), tpl::tr("Частота",
                                                                "Frequency"), tpl::tr("ГГц", "GHz"),
                                  5.0, 0.5, 50.0, 0.5, 2),
                    // От 2°: 0...180 при шаге 2 — 91 угол, а больше 99 точек .s не примет.
                    Param::number(QStringLiteral("step"), tpl::tr("Шаг по углу", "Angle step"),
                                  tpl::tr("град.", "deg."), 5.0, 2.0, 45.0, 1.0, 0)};
        s.build = buildAirfoil;
        *out << s;
    }
    {
        Sample s;
        s.id = QStringLiteral("rcs_coated");
        s.section = sec;
        s.title = tpl::tr("Цилиндр в плазменной оболочке", "Cylinder in a plasma sheath");
        s.summary = tpl::tr("Как плазма меняет рассеяние: три варианта сразу",
                            "How plasma changes scattering: three variants at once");
        s.standard = tpl::tr("X-мода, холодная плазма", "X-mode, cold plasma");
        s.kernel = Kernel::X;
        s.help = tpl::tr(
                     "Металлический цилиндр, окружённый слоем плазмы, — упрощённая модель тела в "
                     "плазменной оболочке (как у гиперзвукового аппарата). Одно задание считает "
                     "три варианта: пустую область (опору), голый металл и металл в плазме.",
                     "A metal cylinder surrounded by a plasma layer: a simplified model of a body "
                     "in a plasma sheath (as for a hypersonic vehicle). One task computes three "
                     "variants: an empty domain (reference), bare metal and metal in plasma.")
                 + helpOpen();
        s.params = {Param::number(QStringLiteral("r"), tpl::tr("Радиус цилиндра", "Cylinder radius"),
                                  tpl::tr("мм", "mm"), 20.0, 2.0, 500.0, 1.0, 1),
                    Param::number(QStringLiteral("t"), tpl::tr("Толщина оболочки", "Sheath thickness"),
                                  tpl::tr("мм", "mm"), 10.0, 1.0, 200.0, 1.0, 1),
                    Param::number(QStringLiteral("fp"), tpl::tr("fp оболочки",
                                                                "Sheath fp"), tpl::tr("ГГц", "GHz"),
                                  4.0, 0.1, 100.0, 0.1, 2),
                    Param::number(QStringLiteral("flo"), tpl::tr("Частота от",
                                                                 "Frequency from"), tpl::tr("ГГц", "GHz"),
                                  2.0, 0.3, 50.0, 0.5, 2),
                    Param::number(QStringLiteral("fhi"), tpl::tr("Частота до",
                                                                 "Frequency to"), tpl::tr("ГГц", "GHz"),
                                  8.0, 0.5, 100.0, 0.5, 2)};
        s.build = buildCoated;
        *out << s;
    }
}

} // namespace tpl::samples
