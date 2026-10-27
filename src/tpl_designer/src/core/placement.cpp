/// @file placement.cpp
/// @brief Реализация проверок размещения (`checkPlacement`) и вспомогательной геометрии.
///
/// Описание — в placement.h.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#placement
#include "placement.h"
#include "lang.h"

#include "expr.h"
#include "physics.h"
#include "sigfile.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace tpl {
namespace {

constexpr double kC = 299.792458; ///< скорость света, мм/нс

/// @brief Возвращает имя фигуры для текста замечания.
/// @param[in] s  фигура.
/// @return Имя или «фигура N».
QString titleOf(const Shape &s)
{
    return s.name.isEmpty() ? tpl::tr("фигура %1", "shape %1").arg(s.id) : s.name;
}

/// @brief Возвращает габарит фигуры.
/// @param[in] s  фигура.
/// @return Габаритный прямоугольник, мм.
RectMm boundsOf(const Shape &s)
{
    RectMm r;
    s.bounds(&r.xLo, &r.xHi, &r.yLo, &r.yHi);
    return r;
}

/// Пересекаются ли прямоугольники ПЛОЩАДЬЮ (касание по стороне — не пересечение).
/// @param[in] a    первый прямоугольник, мм.
/// @param[in] b    второй прямоугольник, мм.
/// @param[in] tol  допуск, мм.
/// @return true — общая площадь есть.
bool overlaps(const RectMm &a, const RectMm &b, double tol = 1e-9)
{
    return a.xLo < b.xHi - tol && b.xLo < a.xHi - tol && a.yLo < b.yHi - tol
           && b.yLo < a.yHi - tol;
}

/// Есть ли в фигуре концентрация электронов (числом, формулой или из файла).
/// @param[in] s  фигура.
/// @return true — задана N (не ноль, формула или файл).
bool hasConcentration(const Shape &s)
{
    const Plasma &pl = s.plasma;
    return pl.hasN && (pl.n != 0.0 || !pl.nFile.trimmed().isEmpty()
                       || !pl.nFormula.trimmed().isEmpty());
}

/// Диэлектрик с ПОЛОЖИТЕЛЬНОЙ добавкой eps (eps > 1) — то, что ядро X не совмещает с
/// плазмой в одном узле. Профиль формулой считаем положительным, если он даёт eps > 1
/// хоть в одной проверенной точке.
/// @param[in] s  фигура.
/// @return true — диэлектрик без плазмы с eps > 1 (у профиля — хоть в одной точке выборки).
bool hasPositiveEpsAdd(const Shape &s)
{
    if (s.plasma.any() || s.material != Material::Dielectric || s.isMoving())
        return false;
    if (!s.hasEpsProfile())
        return s.eps > 1.0;
    if (s.epsProfile.fromFile)
        return false; // что в файле — неизвестно, гадать не будем
    const ExprProgram prog = ExprProgram::compile(s.epsProfile.formula);
    for (const QPointF &pt : epsSamplePoints(s)) {
        const auto v = prog.evalEps(pt.x(), pt.y());
        if (v && *v > 1.0)
            return true;
    }
    return false;
}

/// Сетка точек внутри площади фигуры с шагом не мельче `step` (не больше 60x60 точек).
/// @param[in] s     фигура.
/// @param[in] step  желаемый шаг выборки, мм.
/// @return Точки габаритной сетки, попавшие в площадь фигуры, мм.
QVector<QPointF> areaPoints(const Shape &s, double step)
{
    QVector<QPointF> out;
    const RectMm r = boundsOf(s);
    const int nx = std::clamp(int(std::ceil((r.xHi - r.xLo) / step)), 1, 60);
    const int ny = std::clamp(int(std::ceil((r.yHi - r.yLo) / step)), 1, 60);
    for (int i = 0; i <= nx; ++i) {
        for (int j = 0; j <= ny; ++j) {
            const QPointF pt(r.xLo + (r.xHi - r.xLo) * i / nx, r.yLo + (r.yHi - r.yLo) * j / ny);
            if (shapeCovers(s, pt))
                out << pt;
        }
    }
    return out;
}

/// Отрезок контура с его материалом.
struct Segment {
    ///@{
    /// Начало и конец отрезка, мм.
    QPointF a, b;
    ///@}
    /// Материал стенки на отрезке.
    Material material = Material::Metal;
};

/// Контур фигуры отрезками — то, что ядро рисует у стенки (металл, магнитная стенка,
/// поглотитель). У прямоугольника — только существующие стороны и каждая со своим
/// материалом; у круга — дуги и радиальные стороны сектора.
/// @param[in] s  фигура.
/// @return Отрезки контура (круг — 64 хорды на дугу).
QVector<Segment> outline(const Shape &s)
{
    QVector<Segment> out;
    switch (s.kind) {
    case ShapeKind::Rectangle:
        for (int e = 0; e < 4; ++e) {
            if (s.edges[size_t(e)].present)
                out << Segment{s.edgeFrom(e), s.edgeTo(e), s.edgeMaterial(e)};
        }
        break;
    case ShapeKind::Circle: {
        constexpr double kTwoPi = 2.0 * 3.14159265358979323846;
        const bool full = (s.fiMax - s.fiMin) >= kTwoPi - 1e-9;
        const int n = 64;
        const auto at = [&](double r, double a) {
            return QPointF(s.center.x() + r * std::cos(a), s.center.y() + r * std::sin(a));
        };
        for (double r : {s.rMax, s.rMin}) {
            if (r <= 0.0)
                continue;
            for (int i = 0; i < n; ++i) {
                const double a0 = s.fiMin + (s.fiMax - s.fiMin) * i / n;
                const double a1 = s.fiMin + (s.fiMax - s.fiMin) * (i + 1) / n;
                out << Segment{at(r, a0), at(r, a1), s.material};
            }
        }
        if (!full) {
            out << Segment{at(s.rMin, s.fiMin), at(s.rMax, s.fiMin), s.material};
            out << Segment{at(s.rMin, s.fiMax), at(s.rMax, s.fiMax), s.material};
        }
        break;
    }
    case ShapeKind::Polygon:
    case ShapeKind::Polyline:
        for (int i = 1; i < s.points.size(); ++i)
            out << Segment{s.points.at(i - 1), s.points.at(i), s.material};
        if (s.kind == ShapeKind::Polygon && s.points.size() >= 3)
            out << Segment{s.points.last(), s.points.first(), s.material};
        break;
    }
    return out;
}

/// Точки по отрезкам контура заданного материала, с шагом не крупнее `step`.
/// @param[in] s     фигура (не контурная — пустой результат).
/// @param[in] m     материал отрезков, которые нужны.
/// @param[in] step  наибольший шаг между точками, мм.
/// @return Точки вдоль отрезков материала `m`, мм.
QVector<QPointF> outlinePoints(const Shape &s, Material m, double step)
{
    QVector<QPointF> out;
    if (!s.isContour())
        return out;
    for (const Segment &seg : outline(s)) {
        if (seg.material != m)
            continue;
        const double len = std::hypot(seg.b.x() - seg.a.x(), seg.b.y() - seg.a.y());
        const int n = std::clamp(int(std::ceil(len / step)), 1, 2000);
        for (int i = 0; i <= n; ++i)
            out << seg.a + (seg.b - seg.a) * (double(i) / n);
    }
    return out;
}

/// @brief Проверяет, лежит ли линия внутри области, а не на её краю.
/// @param[in] b         область расчёта.
/// @param[in] x         координата линии, мм.
/// @param[in] vertical  true — вертикальная линия (x = const), false — горизонтальная (y = const).
/// @return true — линия строго внутри области.
bool boxInterior(const Box &b, double x, bool vertical)
{
    // Сторона блока, лежащая на краю области, ничего не делит: за краем узлов нет.
    const double lo = vertical ? b.xMin : b.yMin;
    const double hi = vertical ? b.xMax : b.yMax;
    return x > lo + 1e-9 && x < hi - 1e-9;
}

} // namespace

bool patternWindowCloses(const Project &p, double tMin, double tMax)
{
    for (double f : p.freqs) {
        if (f <= 0.0 || tMax <= tMin)
            continue;
        const int periods = int((tMax - tMin) * f + 0.9999);
        if (tMin + periods / f >= p.timeMax - 1e-9)
            return false;
    }
    return true;
}

double patternWindowSafeEnd(const Project &p, double tMin, double want)
{
    if (patternWindowCloses(p, tMin, want))
        return want;
    double fMin = 0.0;
    for (double f : p.freqs) {
        if (f > 0.0)
            fMin = fMin > 0.0 ? std::min(fMin, f) : f;
    }
    if (fMin <= 0.0)
        return want;
    // На период самой низкой частоты раньше конца счёта — и вниз до сотых, чтобы в
    // задании стояло читаемое число.
    double end = std::floor((p.timeMax - 1.0 / fMin - 1e-3) * 100.0) / 100.0;
    end = std::min(end, want);
    for (int i = 0; i < 1000 && end > tMin && !patternWindowCloses(p, tMin, end); ++i)
        end -= 0.01;
    return end;
}

double patternContourMargin(const Project &p)
{
    return 10.0 * p.delta;
}

RectMm plasmaBRect(const Project &p, const Shape &s, bool *widened)
{
    const RectMm base = boundsOf(s);
    RectMm r = base;
    if (widened)
        *widened = false;
    if (p.delta <= 0.0 || s.plasma.bExact)
        return r;

    const double m = kBMarginCells * p.delta;
    // Чужая плазма: блок B, которого раньше она не касалась, не должен на неё залезть —
    // у неё своё поле (или никакого), и наше её молча подмагнитило бы.
    QVector<RectMm> foreign;
    for (const Shape &o : p.shapes) {
        if (o.id == s.id || !hasConcentration(o))
            continue;
        const RectMm ob = boundsOf(o);
        if (!overlaps(base, ob))
            foreign << ob;
    }

    // Формулу B ядро посчитает и в новых узлах. Особенность вне фигуры (у токамака
    // `2*1480/(1480-x)`) дала бы там деление на ноль и ошибку шага — проверяем полосу.
    const QString formula = s.plasma.bFormula.trimmed();
    const ExprProgram prog = formula.isEmpty() ? ExprProgram() : ExprProgram::compile(formula);
    const QPointF o = s.plasma.customOrigin && s.kind != ShapeKind::Circle ? s.plasma.origin
                                                                            : QPointF(0.0, 0.0);
    const auto formulaOk = [&](const RectMm &strip) {
        if (formula.isEmpty())
            return true;
        if (!prog.valid())
            return false;
        for (int i = 0; i <= 8; ++i) {
            for (int j = 0; j <= 8; ++j) {
                const double x = strip.xLo + (strip.xHi - strip.xLo) * i / 8.0 - o.x();
                const double y = strip.yLo + (strip.yHi - strip.yLo) * j / 8.0 - o.y();
                const auto v = prog.evalEps(x, y);
                if (!v || !std::isfinite(*v))
                    return false;
            }
        }
        return true;
    };

    // Каждую сторону — отдельно: упёрлась одна, остальные расширяются всё равно.
    for (int side = 0; side < 4; ++side) {
        RectMm c = r;
        RectMm strip = r;
        switch (side) {
        case 0: c.xLo = std::max(p.box.xMin, r.xLo - m); strip.xHi = r.xLo; strip.xLo = c.xLo; break;
        case 1: c.xHi = std::min(p.box.xMax, r.xHi + m); strip.xLo = r.xHi; strip.xHi = c.xHi; break;
        case 2: c.yLo = std::max(p.box.yMin, r.yLo - m); strip.yHi = r.yLo; strip.yLo = c.yLo; break;
        case 3: c.yHi = std::min(p.box.yMax, r.yHi + m); strip.yLo = r.yHi; strip.yHi = c.yHi; break;
        }
        const bool grows = c.xLo < r.xLo - 1e-12 || c.xHi > r.xHi + 1e-12 || c.yLo < r.yLo - 1e-12
                           || c.yHi > r.yHi + 1e-12;
        if (!grows)
            continue;
        bool blocked = !formulaOk(strip);
        for (const RectMm &f : foreign)
            blocked |= overlaps(c, f);
        if (blocked)
            continue;
        r = c;
        if (widened)
            *widened = true;
    }
    return r;
}

double steadyStateTime(const Project &p)
{
    // Возбуждение, которое вообще может выйти на установившийся режим, — непрерывная
    // несущая (формулой) или окно с несущей. Гаусс и сигнал из файла — это импульс
    // или многотон, у них своя обработка и своё предупреждение ядра (Б-22).
    double start = -1.0;
    for (const Port &q : p.ports) {
        const bool active = p.exciters.isEmpty() ? q.active : p.exciters.contains(q.id);
        if (!active || q.signal.source != SignalSource::Formula
            || q.signal.shape == SignalShape::Gaussian)
            continue;
        Port on = q;
        on.active = true;
        start = std::max(start, portWindow(on, p.timeMax).tMin);
    }
    if (start < 0.0)
        return 0.0;

    // Самая «медленная» среда: волна в диэлектрике идёт в sqrt(eps) раз медленнее.
    double eps = 1.0;
    for (const Shape &s : p.shapes) {
        if (!s.plasma.any() && s.material == Material::Dielectric && !s.hasEpsProfile())
            eps = std::max(eps, s.eps);
    }
    const double diag = std::hypot(p.box.width(), p.box.height());
    // Один проход по диагонали — нижняя граница: раньше поле просто не дошло до всех
    // уголков области. В резонаторе установление дольше, но сколько — генератор не
    // знает, а пугать замечанием каждое рабочее задание нельзя.
    return start + diag * std::sqrt(eps) / kC;
}

void checkPlacement(const Project &p, Diagnostics *d)
{
    QStringList &e = d->errors;
    QStringList &w = d->warnings;
    if (p.delta <= 0.0 || p.box.width() <= 0.0 || p.box.height() <= 0.0)
        return;
    const double half = p.delta / 2.0;
    const auto nice = [](double v) { return QString::number(v, 'g', 6); };

    // --- фигуры за границей области (Б-16) ---
    //
    // С 26.10.2026 ядро не прижимает вылезшие точки к краю, а отбрасывает часть блока,
    // лежащую дальше полушага за границей. Целиком вне области — отдельное замечание
    // в validate(); здесь — только частично.
    for (const Shape &s : p.shapes) {
        const RectMm r = boundsOf(s);
        const bool outside = r.xHi < p.box.xMin || r.xLo > p.box.xMax || r.yHi < p.box.yMin
                             || r.yLo > p.box.yMax;
        if (outside || p.sweep.rotates(s.id))
            continue;
        if (r.xLo < p.box.xMin - half || r.xHi > p.box.xMax + half || r.yLo < p.box.yMin - half
            || r.yHi > p.box.yMax + half) {
            w << tpl::tr("часть «%1» выходит за счётную область — ядро отбросит её: "
                         "точки дальше полушага за границей в расчёт не попадают",
                         "part of '%1' lies outside the computational domain - the kernel will "
                         "discard it: points more than half a step beyond the boundary do not "
                         "enter the computation")
                     .arg(titleOf(s));
        }
    }

    // --- толщина магнитной стенки и поглотителя у круга и контура (Б-26, Б-4) ---
    //
    // У прямоугольника это проверяет validate(). Стенка обязана иметь толщину в ячейку
    // (2x2 узла): тип узла ядро выводит из числа и расположения соседей.
    for (const Shape &s : p.shapes) {
        if (!s.isContour()
            || (s.material != Material::Magnetic && s.material != Material::Absorber))
            continue;
        const RectMm r = boundsOf(s);
        bool thin = false;
        if (s.kind == ShapeKind::Circle)
            thin = 2.0 * s.rMax <= 2.0 * p.delta;
        else if (s.kind == ShapeKind::Polygon)
            thin = (r.xHi - r.xLo) <= 2.0 * p.delta && (r.yHi - r.yLo) <= 2.0 * p.delta;
        if (thin) {
            w << tpl::tr("«%1» меньше ячейки сетки: у магнитной стенки и поглотителя "
                         "размер должен быть больше 2*DELTA (%2 мм), иначе ядро не "
                         "соберёт из узлов сплошную стенку",
                         "'%1' is smaller than a grid cell: a magnetic wall or absorber must be "
                         "larger than 2*DELTA (%2 mm), otherwise the kernel cannot assemble a "
                         "continuous wall from the nodes")
                     .arg(titleOf(s), nice(2.0 * p.delta));
        }
    }

    // --- плазма ---
    if (p.kernel == Kernel::X) {
        // Краевой слой плазмы. В узлах на границе блока ядро ополовинивает N (в углу
        // делит на четыре), а B там — полное у широкого блока и половинное у
        // совпадающего. У такого слоя своя верхнегибридная частота, и если она в полосе
        // расчёта, тонкий слой резонирует: в образце «Подмагниченная плазма» (fp = 3,
        // fce = 1.5 ГГц, широкий B) это 2.598 ГГц, и на 2.6 ГГц прогон дал
        // |S11|^2 + |S21|^2 = 1.13 — энергию из ниоткуда.
        double fLo = 0.0, fHi = 0.0;
        for (double f : p.freqs) {
            fLo = fLo > 0.0 ? std::min(fLo, f) : f;
            fHi = std::max(fHi, f);
        }
        const auto inBand = [&](double f) {
            return fHi > 0.0 && f >= fLo * 0.98 && f <= fHi * 1.02;
        };
        /// Частоты краевого слоя для N и B числами: {широкий B, ровный B}; {0, 0} — не число.
        const auto edgeFreqs = [](const Plasma &pl) {
            if (!pl.hasN || !pl.hasB || !pl.nFormula.trimmed().isEmpty()
                || !pl.bFormula.trimmed().isEmpty() || pl.hasFile() || pl.n <= 0.0)
                return std::pair<double, double>(0.0, 0.0);
            const double fp2 = std::pow(plasmaFreqGHz(pl.n), 2);
            const double fce2 = std::pow(cyclotronFreqGHz(pl.b), 2);
            return std::pair<double, double>(std::sqrt(fp2 / 2.0 + fce2),
                                             std::sqrt(fp2 / 2.0 + fce2 / 4.0));
        };

        for (const Shape &s : p.shapes) {
            if (!s.plasma.hasB || !s.plasma.bFile.trimmed().isEmpty())
                continue;
            // Граница блока B, проходящая по плазме, даёт там B/2 (Б-21). Свою плазму
            // эмиттер обходит сам, расширяя блок; сюда попадает то, что расширить не
            // вышло (упёрлось в чужую плазму или в особенность формулы), и чужая
            // плазма, которую режет отдельно нарисованный блок B.
            const RectMm b = plasmaBRect(p, s);
            for (const Shape &o : p.shapes) {
                if (!hasConcentration(o))
                    continue;
                const RectMm n = boundsOf(o);
                if (!overlaps(b, n))
                    continue;
                const auto cuts = [&](double x, bool vertical) {
                    if (!boxInterior(p.box, x, vertical))
                        return false;
                    return vertical ? (x >= n.xLo - half && x <= n.xHi + half)
                                    : (x >= n.yLo - half && x <= n.yHi + half);
                };
                if (cuts(b.xLo, true) || cuts(b.xHi, true) || cuts(b.yLo, false)
                    || cuts(b.yHi, false)) {
                    // Своя плазма с B ровно по ней и широким вариантом хуже: так и говорим,
                    // а не советуем то, что испортит расчёт.
                    const auto [fWide, fExact] = edgeFreqs(s.plasma);
                    if (o.id == s.id && s.plasma.bExact && fWide > 0.0 && inBand(fWide)
                        && !inBand(fExact)) {
                        w << tpl::tr("блок B у «%1» задан ровно по плазме: на её краю ядро "
                                     "ополовинивает и N, и B. Здесь это оправдано — при B "
                                     "шире плазмы краевой слой резонировал бы на %2 ГГц, "
                                     "внутри полосы расчёта, а так он на %3 ГГц, вне её",
                                     "the B block of '%1' is set exactly over the plasma: at its "
                                     "edge the kernel halves both N and B. Here this is justified "
                                     "- with B wider than the plasma the edge layer would "
                                     "resonate at %2 GHz, inside the computation band, whereas "
                                     "now it is at %3 GHz, outside it")
                                 .arg(titleOf(s), nice(fWide), nice(fExact));
                        break;
                    }
                    w << tpl::tr("граница подмагничивания B у «%1» совпадает с границей "
                                 "плазмы «%2» или проходит по ней. На границе блока ядро "
                                 "ополовинивает B, и на краю плазмы получится ложный "
                                 "резонансный слой. Задайте B шире плазмы — вне плазмы "
                                 "оно ни на что не влияет",
                                 "the boundary of the magnetization B of '%1' coincides with the "
                                 "boundary of plasma '%2' or runs along it. At a block boundary "
                                 "the kernel halves B, and a false resonant layer appears at the "
                                 "plasma edge. Make B wider than the plasma - outside the plasma "
                                 "it has no effect")
                             .arg(titleOf(s), titleOf(o));
                    break;
                }
            }
        }

        // Ядро такое задание считает, поэтому — предупреждение с числами и тем, что поможет.
        for (const Shape &s : p.shapes) {
            const auto [fWide, fExact] = edgeFreqs(s.plasma);
            if (fWide <= 0.0)
                continue;
            bool wide = false;
            plasmaBRect(p, s, &wide);
            const double fEdge = wide ? fWide : fExact;
            const double fOther = wide ? fExact : fWide;
            if (!inBand(fEdge))
                continue;
            QString cure = wide ? tpl::tr("отметьте «B ровно по плазме»",
                                          "check \"B exactly over the plasma\"")
                                : tpl::tr("снимите «B ровно по плазме»",
                                          "uncheck \"B exactly over the plasma\"");
            cure = inBand(fOther)
                       ? tpl::tr("сдвиньте полосу или сгладьте край плазмы профилем N",
                                 "shift the band or smooth the plasma edge with an N profile")
                       : tpl::tr("%1 — краевой слой уйдёт на %2 ГГц, вне полосы",
                                 "%1 - the edge layer will move to %2 GHz, outside the band")
                             .arg(cure, nice(fOther));
            w << tpl::tr("у «%1» краевые узлы плазмы (на границе блока ядро "
                         "ополовинивает N) с %2 B резонируют на %3 ГГц — это внутри "
                         "полосы расчёта %4...%5 ГГц. На частотах около резонанса счёт "
                         "может дать |S| > 1. Что поможет: %6",
                         "the edge plasma nodes of '%1' (at a block boundary the kernel halves N) "
                         "resonate with %2 B at %3 GHz - this is inside the computation band "
                         "%4...%5 GHz. At frequencies near the resonance the computation may give "
                         "|S| > 1. What helps: %6")
                     .arg(titleOf(s),
                          wide ? tpl::tr("полным", "full") : tpl::tr("половинным", "half"),
                          nice(fEdge), nice(fLo), nice(fHi), cure);
        }

        // Б-6: в узле либо положительная eps, либо плазма. Совмещение верно только на
        // FREQ, а при добавке больше плазменной части расчёт этих узлов расходится.
        // B в диэлектрике без плазмы ядро не использует. Ядро говорит то же самое при
        // счёте, но узнать это до прогона дешевле.
        for (const Shape &dsh : p.shapes) {
            if (!hasPositiveEpsAdd(dsh))
                continue;
            const QVector<QPointF> pts = areaPoints(dsh, p.delta);
            for (const Shape &o : p.shapes) {
                if (o.id == dsh.id || !o.plasma.any())
                    continue;
                bool withN = false, withB = false;
                for (const QPointF &pt : pts) {
                    if (!shapeCovers(o, pt))
                        continue;
                    if (hasConcentration(o))
                        withN = true;
                    else if (o.plasma.hasB)
                        withB = true;
                }
                if (withN) {
                    w << tpl::tr("плазма «%1» и диэлектрик «%2» (eps = %3) перекрываются. "
                                 "В одном узле ядро X держит либо положительную eps, "
                                 "либо плазму: добавка eps там верна только на частоте "
                                 "FREQ, а если она больше плазменной части, расчёт этих "
                                 "узлов расходится. Разнесите их по разным областям",
                                 "plasma '%1' and dielectric '%2' (eps = %3) overlap. In one node "
                                 "kernel X holds either a positive eps or plasma: the eps "
                                 "increment there is correct only at frequency FREQ, and if it "
                                 "exceeds the plasma part, the computation of these nodes "
                                 "diverges. Separate them into different regions")
                             .arg(titleOf(o), titleOf(dsh), nice(dsh.eps));
                } else if (withB) {
                    w << tpl::tr("подмагничивание «%1» заходит в диэлектрик «%2» без "
                                 "плазмы — в узлах с положительной eps ядро поле B не "
                                 "использует",
                                 "magnetization '%1' extends into dielectric '%2' without plasma "
                                 "- in nodes with positive eps the kernel does not use the B "
                                 "field")
                             .arg(titleOf(o), titleOf(dsh));
                }
            }
        }

        // Подмагниченная плазма у MAGNETIC и любая плазма в поглотителе (диссертация
        // К. Н. Климова 2007). Узел X-схемы — это H_z, схема дуальна физической (3.29),
        // поэтому MAGNETIC в X-ядре физически металл для волн E, и для плазмы в поле B
        // на нём нужно особое граничное условие (3.33), которого в ядре нет. Поглощающий
        // узел соответствует (7.53)-(7.55) только без шлейфов плазмы.
        // Б-8.2 (29.10.2026): если все входы — плоская волна, условие (3.33) не нужно
        // (поле вдоль стенки однородно), и замечание о стенке не пишем.
        bool planeWaveOnly = !p.ports.isEmpty();
        for (const Port &q : p.ports)
            planeWaveOnly &= (q.mode == PortMode::Plane);
        for (const Shape &pl : p.shapes) {
            if (!hasConcentration(pl))
                continue;
            bool magnetized = pl.plasma.hasB;
            for (const Shape &o : p.shapes) {
                if (o.id != pl.id && o.plasma.hasB && overlaps(boundsOf(o), boundsOf(pl)))
                    magnetized = true;
            }
            for (const Shape &wall : p.shapes) {
                if (wall.id == pl.id || !wall.isContour())
                    continue;
                if (magnetized && !planeWaveOnly) {
                    bool touch = false;
                    for (const QPointF &pt : outlinePoints(wall, Material::Magnetic, half))
                        touch |= shapeCovers(pl, pt, p.delta);
                    if (touch) {
                        // С ядер 29.10.2026 узлы стенки в подмагниченной плазме считаются по
                        // (7.55) с шлейфами y6, y7 (Б-8.2): плоская волна вдоль стенки — точно.
                        // Особого условия (3.33) для поля, меняющегося поперёк стенки, в ядре
                        // по-прежнему нет — предупреждение остаётся, мягче.
                        w << tpl::tr("подмагниченная плазма «%1» касается магнитной стенки "
                                     "«%2». В X-ядре MAGNETIC для волн E — это металл; для "
                                     "плазмы в поле B на нём нужно особое граничное условие "
                                     "(3.33 диссертации), которого в ядре нет: плоская волна "
                                     "вдоль стенки считается верно, а поле, меняющееся поперёк "
                                     "стенки, — неточно. Если это не плоская волна, отодвиньте "
                                     "плазму хотя бы на шаг сетки",
                                     "magnetized plasma '%1' touches magnetic wall '%2'. In the X "
                                     "kernel MAGNETIC is metal for E waves; plasma in a B field "
                                     "needs a special boundary condition there (3.33 of the "
                                     "dissertation) that the kernel lacks: a plane wave along the "
                                     "wall is computed correctly, a field varying across the wall is "
                                     "not. Unless it is a plane wave, move the plasma at least one "
                                     "grid step away")
                                 .arg(titleOf(pl), titleOf(wall));
                    }
                }
                // Р-1, Б-8.2 (ответы автора 29.10.2026): с ядер 29.10 нагрузка поглотителя
                // считается по среде каждого его узла — диэлектрик, плазма, подмагниченная
                // плазма ((7.54), (7.55)). Не согласовать её ядро может только там, где волна
                // вообще не распространяется: плотная плазма, eps < 0 на частоте FREQ. Там
                // узел остаётся с вакуумной нагрузкой (ядро пишет WARNING) — об этом и говорим.
                // N формулой или из файла здесь не оцениваем — молчим (ядро скажет само).
                bool inAbsorber = false;
                for (const QPointF &pt : outlinePoints(wall, Material::Absorber, half))
                    inAbsorber |= shapeCovers(pl, pt, half);
                if (inAbsorber && pl.plasma.hasN && pl.plasma.n > 0.0 && pl.plasma.nFile.trimmed().isEmpty()
                    && pl.plasma.nFormula.trimmed().isEmpty() && !p.freqs.isEmpty()) {
                    double fMin = p.freqs.first();
                    for (double f : p.freqs)
                        fMin = std::min(fMin, f);
                    const double epsPl = 1.0 - std::pow(plasmaFreqGHz(pl.plasma.n) / fMin, 2);
                    if (epsPl < 0.0) {
                        w << tpl::tr("поглотитель «%2» стоит в плотной плазме «%1» (eps = %3 "
                                     "на %4 ГГц): волна в ней не распространяется, и согласовать "
                                     "поглотитель нечем — ядро оставит в этих узлах нагрузку "
                                     "вакуума, и там он не поглощает. Вынесите поглотитель из "
                                     "плотной плазмы",
                                     "absorber '%2' lies in the dense plasma '%1' (eps = %3 at %4 "
                                     "GHz): no wave propagates there, so the absorber cannot be "
                                     "matched - the kernel keeps the vacuum load in these nodes and "
                                     "they do not absorb. Move the absorber out of the dense plasma")
                                 .arg(titleOf(pl), titleOf(wall), nice(epsPl), nice(fMin));
                    }
                }
            }
        }

        // Добавку eps, отличающуюся от 4 меньше чем на 0.004, ядро X сдвигает до 3.996
        // или 4.004 — так задумано, без этого алгоритм расходится (tpl-guide 5.1).
        for (const Shape &s : p.shapes) {
            if (s.plasma.any() || s.material != Material::Dielectric || s.hasEpsProfile())
                continue;
            const double add = s.eps - 1.0;
            if (std::fabs(add - 4.0) < 0.004 && std::fabs(add - 4.0) > 0.0) {
                w << tpl::tr("«%1»: добавку eps-1 = %2 ядро X сдвинет до %3 — около 4 "
                             "алгоритм иначе расходится",
                             "'%1': kernel X will shift the eps-1 increment = %2 to %3 - near 4 "
                             "the algorithm otherwise diverges")
                         .arg(titleOf(s), nice(add), add < 4.0 ? QStringLiteral("3.996")
                                                               : QStringLiteral("4.004"));
            } else if (add == 4.0) {
                w << tpl::tr("«%1»: eps = 5 (добавка 4) ядро X считает как добавку 3.996 "
                             "или 4.004 — ровно 4 алгоритм не держит",
                             "'%1': kernel X treats eps = 5 (increment 4) as increment 3.996 or "
                             "4.004 - the algorithm cannot handle exactly 4")
                         .arg(titleOf(s));
            }
        }
    }

    // --- окна съёма ---
    const double settle = steadyStateTime(p);
    const bool anyWide = hasFileSignal(p);

    // Б-22: в одночастотном режиме окно SMATRIX — с установившегося режима.
    if (p.output.sMatrix && !anyWide && settle > 0.0 && p.output.sMatrixTMin < settle) {
        w << tpl::tr("окно S-матрицы начинается на %1 нс, а поле в области "
                     "устанавливается не раньше %2 нс (волна проходит область по диагонали): "
                     "в окно попадёт переходный процесс. Начните окно позже",
                     "the S-matrix window starts at %1 ns, but the field in the domain settles no "
                     "earlier than %2 ns (the wave crosses the domain along the diagonal): the "
                     "transient will fall into the window. Start the window later")
                 .arg(nice(p.output.sMatrixTMin), nice(settle));
    }

    if (p.output.directionalPattern) {
        const double t0 = p.output.patternTMin, t1 = p.output.patternTMax;
        // Б-13: ядро округляет окно ДН вверх до целого числа периодов,
        // (int)((Tmax-Tmin)*f + 0.9999). Окно, кончающееся не раньше TIME, ядро не
        // закрывает — файлов ДН нет, код 2. Ошибка: такой расчёт ничего не даст.
        for (double f : p.freqs) {
            if (f <= 0.0 || t1 <= t0)
                continue;
            const int periods = int((t1 - t0) * f + 0.9999);
            const double end = t0 + periods / f;
            if (end >= p.timeMax - 1e-9) {
                const double safe = patternWindowSafeEnd(p, t0, t1);
                e << tpl::tr("окно диаграммы направленности %1...%2 нс ядро дотягивает до "
                             "целого числа периодов частоты %3 ГГц — до %4 нс, а счёт "
                             "кончается на %5 нс. Такое окно не закроется, и файлов ДН "
                             "не будет. Кончите окно на %6 нс или раньше, либо "
                             "увеличьте время счёта",
                             "the kernel extends the radiation pattern window %1...%2 ns to a "
                             "whole number of periods of frequency %3 GHz - up to %4 ns, but the "
                             "run ends at %5 ns. Such a window will not close, and there will be "
                             "no radiation pattern files. End the window at %6 ns or earlier, or "
                             "increase the run time")
                         .arg(nice(t0), nice(t1), nice(f), nice(end), nice(p.timeMax),
                              nice(safe));
                break;
            }
        }
        if (settle > 0.0 && t0 < settle) {
            w << tpl::tr("окно диаграммы направленности начинается на %1 нс, а поле "
                         "устанавливается не раньше %2 нс: в накопление попадёт "
                         "переходный процесс. ДН снимают в установившемся режиме — "
                         "начните окно позже",
                         "the radiation pattern window starts at %1 ns, but the field settles no "
                         "earlier than %2 ns: the transient will enter the accumulation. The "
                         "radiation pattern is taken in the steady state - start the window later")
                     .arg(nice(t0), nice(settle));
        }

        // Б-14: контур ДН стоит ровно в 10 шагах от края области, и снаружи него должно
        // быть свободное пространство. Поглотитель — снаружи контура (тоньше отступа),
        // плазма — внутри, вход контур не пересекает.
        const double mg = patternContourMargin(p);
        const RectMm c{p.box.xMin + mg, p.box.xMax - mg, p.box.yMin + mg, p.box.yMax - mg};
        if (c.xHi > c.xLo && c.yHi > c.yLo) {
            const auto inside = [&](const QPointF &pt) {
                return pt.x() > c.xLo + half && pt.x() < c.xHi - half && pt.y() > c.yLo + half
                       && pt.y() < c.yHi - half;
            };
            const auto outside = [&](const QPointF &pt) {
                return pt.x() < c.xLo - half || pt.x() > c.xHi + half || pt.y() < c.yLo - half
                       || pt.y() > c.yHi + half;
            };
            for (const Shape &s : p.shapes) {
                if (s.isContour() && s.material == Material::Absorber) {
                    bool in = false, out = false;
                    for (const QPointF &pt : outlinePoints(s, Material::Absorber, half)) {
                        in |= inside(pt);
                        out |= outside(pt);
                    }
                    if (in && out) {
                        w << tpl::tr("поглотитель «%1» пересекает контур диаграммы "
                                     "направленности (%2 мм = 10 шагов сетки от края "
                                     "области). Снаружи контура должно быть свободное "
                                     "пространство: поглотитель делают тоньше отступа",
                                     "absorber '%1' crosses the radiation pattern contour (%2 mm "
                                     "= 10 grid steps from the domain edge). Outside the contour "
                                     "there must be free space: make the absorber thinner than "
                                     "the margin")
                                 .arg(titleOf(s), nice(mg));
                    }
                }
                if (hasConcentration(s)) {
                    const RectMm r = boundsOf(s);
                    if (r.xLo < c.xLo || r.xHi > c.xHi || r.yLo < c.yLo || r.yHi > c.yHi) {
                        w << tpl::tr("плазма «%1» ближе %2 мм (10 шагов сетки) к краю "
                                     "области — заходит за контур диаграммы "
                                     "направленности, и ДН посчитается неверно. "
                                     "Отодвиньте плазму от края или увеличьте область",
                                     "plasma '%1' is closer than %2 mm (10 grid steps) to the "
                                     "domain edge - it extends beyond the radiation pattern "
                                     "contour, and the pattern will be computed incorrectly. Move "
                                     "the plasma away from the edge or enlarge the domain")
                                 .arg(titleOf(s), nice(mg));
                    }
                }
            }
            for (const Port &q : p.ports) {
                const double plane = portPlane(q, p.box);
                const bool alongY = portAlongY(q.side);
                bool in = false, out = false;
                for (int i = 0; i <= 40; ++i) {
                    const double t = q.from + (q.to - q.from) * i / 40.0;
                    const QPointF pt = alongY ? QPointF(plane, t) : QPointF(t, plane);
                    in |= inside(pt);
                    out |= outside(pt);
                }
                // Вход, лежащий вдоль самой линии контура, режет его по всей апертуре.
                const bool along = alongY ? (std::fabs(plane - c.xLo) <= half
                                             || std::fabs(plane - c.xHi) <= half)
                                          : (std::fabs(plane - c.yLo) <= half
                                             || std::fabs(plane - c.yHi) <= half);
                const double aLo = alongY ? c.yLo : c.xLo, aHi = alongY ? c.yHi : c.xHi;
                const bool onLine = along && q.to > aLo && q.from < aHi;
                if ((in && out) || onLine) {
                    w << tpl::tr("вход «%1» пересекает контур диаграммы направленности "
                                 "(%2 мм от края области): поле входа попадёт в ДН как "
                                 "излучение. Поставьте вход целиком внутри контура или "
                                 "целиком снаружи",
                                 "port '%1' crosses the radiation pattern contour (%2 mm from the "
                                 "domain edge): the port field will enter the pattern as "
                                 "radiation. Place the port entirely inside the contour or "
                                 "entirely outside")
                             .arg(q.name, nice(mg));
                }
            }
        }
    }

    // --- магнитная стенка на апертуре входа плоской волны (Б-1, ответ автора 29.10.2026) ---
    //
    // Вход плоской волны согласован (|S11| пустого канала около 1e-7 в H и X), когда
    // стенки MAGNETIC стоят по образцу TEST025: верхняя от a/2 до a/2+DELTA, нижняя от
    // -a/2-2*DELTA до -a/2-DELTA. Стенка, заходящая внутрь апертуры у плоскости входа,
    // закрывает часть входа и рассогласует его.
    for (const Port &q : p.ports) {
        if (q.mode != PortMode::Plane)
            continue;
        const double plane = portPlane(q, p.box);
        const bool alongY = portAlongY(q.side);
        const double lo = std::min(q.from, q.to), hi = std::max(q.from, q.to);
        for (const Shape &wall : p.shapes) {
            bool on = false;
            for (const QPointF &pt : outlinePoints(wall, Material::Magnetic, half)) {
                const double across = alongY ? pt.x() : pt.y();
                const double t = alongY ? pt.y() : pt.x();
                on |= std::fabs(across - plane) <= p.delta && t > lo + half && t < hi - half;
            }
            if (on) {
                w << tpl::tr("магнитная стенка «%2» заходит на апертуру входа плоской волны "
                             "«%1» (%3...%4 мм) и закрывает часть входа. Стенки канала ставят "
                             "у краёв апертуры, не заходя на неё (образец TEST025: верхняя от "
                             "a/2 до a/2+DELTA, нижняя от -a/2-2*DELTA до -a/2-DELTA) — тогда "
                             "вход согласован",
                             "magnetic wall '%2' extends onto the aperture of plane-wave port "
                             "'%1' (%3...%4 mm) and covers part of the port. Channel walls go at "
                             "the aperture edges without entering it (sample TEST025: upper from "
                             "a/2 to a/2+DELTA, lower from -a/2-2*DELTA to -a/2-DELTA) - then the "
                             "port is matched")
                         .arg(q.name, titleOf(wall), nice(lo), nice(hi));
            }
        }
    }

    // --- многочастотный режим: порты разной ширины (Б-22, ответ автора 29.10.2026) ---
    //
    // Поправка на рассогласование порта при сигнале из файла выведена для одинаковых
    // портов и симметричного устройства. Порты разной ширины (или разной моды) —
    // ответ приближённый; точный — свипом по частотам.
    bool wideband = false;
    for (const Port &q : p.ports)
        wideband |= portUsesSignalFile(p, q);
    if (wideband && p.ports.size() >= 2) {
        const Port &a = p.ports.first();
        const double wa = std::fabs(a.to - a.from);
        for (int i = 1; i < p.ports.size(); ++i) {
            const Port &b = p.ports.at(i);
            const double wb = std::fabs(b.to - b.from);
            if (std::fabs(wa - wb) > half || a.mode != b.mode) {
                w << tpl::tr("многочастотный расчёт: входы «%1» (%2 мм) и «%3» (%4 мм) "
                             "разные. Поправка порта в этом режиме верна для одинаковых входов "
                             "и симметричного устройства — здесь ответ приближённый; для "
                             "точного считайте свипом по частотам",
                             "wideband computation: ports '%1' (%2 mm) and '%3' (%4 mm) differ. "
                             "The port correction in this mode holds for identical ports and a "
                             "symmetric device - here the answer is approximate; for an exact "
                             "one use a frequency sweep")
                         .arg(a.name, nice(wa), b.name, nice(wb));
                break;
            }
        }
    }
}

} // namespace tpl
