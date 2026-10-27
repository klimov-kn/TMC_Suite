/// @file project.cpp
/// @brief Реализация помощников модели задания: единицы, стороны прямоугольника,
///        покрытие точки фигурой, правила портов, свип и счёт шагов.
///
/// Часть ядра редактора заданий. Функции чистые (без состояния) и не зависят
/// от интерфейса; описание каждой — в `project.h`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#model
#include "project.h"
#include "lang.h"

#include "expr.h"

#include <algorithm>
#include <cmath>

namespace tpl {

double toMillimeters(double value, LengthUnit from)
{
    switch (from) {
    case LengthUnit::Millimeter: return value;
    case LengthUnit::Centimeter: return value * 10.0;
    case LengthUnit::Meter:      return value * 1000.0;
    }
    return value;
}

QString unitSuffix(LengthUnit u)
{
    switch (u) {
    case LengthUnit::Millimeter: return tpl::tr("мм", "mm");
    case LengthUnit::Centimeter: return tpl::tr("см", "cm");
    case LengthUnit::Meter:      return tpl::tr("м", "m");
    }
    return {};
}

bool isContourMaterial(Material m)
{
    // Все три материала ядро рисует периметром; заливка — только у выражения eps.
    return m == Material::Metal || m == Material::Magnetic || m == Material::Absorber;
}

QString materialName(Material m)
{
    switch (m) {
    case Material::Metal:      return tpl::tr("металл", "metal");
    case Material::Magnetic:   return tpl::tr("магнитная стенка", "magnetic wall");
    case Material::Absorber:   return tpl::tr("поглотитель", "absorber");
    case Material::Dielectric: return tpl::tr("диэлектрик", "dielectric");
    }
    return {};
}

bool Shape::isContour() const
{
    // Плазменная область — всегда ЗАЛИВКА: ядро задаёт её выражением (nType == EPS),
    // материала у такого блока не бывает вовсе. То же и у профиля eps.
    return !plasma.any() && !hasEpsProfile() && isContourMaterial(material);
}

bool Shape::hasEpsProfile() const
{
    return epsProfile.active();
}

QPointF Shape::epsOrigin() const
{
    // У окружности выбора нет: центра в блоке не существует, им служит сам анкер.
    // Значит и координаты формулы у неё всегда от центра — задать иначе нечем.
    if (kind != ShapeKind::Circle && epsProfile.customOrigin)
        return epsProfile.origin;

    double xLo = 0, xHi = 0, yLo = 0, yHi = 0;
    bounds(&xLo, &xHi, &yLo, &yHi);
    return {(xLo + xHi) / 2.0, (yLo + yHi) / 2.0};
}

/// Точки внутри фигуры, в которых имеет смысл проверить формулу профиля.
///
/// Координаты — ЛОКАЛЬНЫЕ, от центра: ровно те, что ядро подставит в выражение.
/// Область берётся настоящая, а не описанный прямоугольник: у сектора углы коробки
/// лежат вне фигуры, ядро их не считает, и жаловаться на них — обманывать пользователя.
/// @param[in] s  фигура
/// @return Точки в локальных координатах относительно `Shape::epsOrigin()`, мм.
QVector<QPointF> epsSamplePoints(const Shape &s)
{
    constexpr int kSteps = 24; // сетка 25x25 — достаточно, чтобы поймать особенность
    QVector<QPointF> out;
    const QPointF o = s.epsOrigin();

    if (s.kind == ShapeKind::Circle) {
        // Идём по радиусу и углу — так же, как обходит узлы `SetCirc`.
        for (int i = 0; i <= kSteps; ++i) {
            const double r = s.rMin + (s.rMax - s.rMin) * i / kSteps;
            for (int j = 0; j <= kSteps; ++j) {
                const double a = s.fiMin + (s.fiMax - s.fiMin) * j / kSteps;
                out << QPointF(r * std::cos(a), r * std::sin(a));
            }
        }
        return out;
    }

    double xLo = 0, xHi = 0, yLo = 0, yHi = 0;
    s.bounds(&xLo, &xHi, &yLo, &yHi);

    if (s.kind == ShapeKind::Rectangle) {
        for (int i = 0; i <= kSteps; ++i) {
            for (int j = 0; j <= kSteps; ++j) {
                out << QPointF(xLo + (xHi - xLo) * i / kSteps - o.x(),
                               yLo + (yHi - yLo) * j / kSteps - o.y());
            }
        }
        return out;
    }

    // Полигон: сетка по коробке, но берём только точки внутри контура. Проверка
    // своя (луч вправо, считаем пересечения) — QPolygonF живёт в QtGui, а ядро
    // генератора от виджетов не зависит и зависеть не должно.
    const auto inside = [&s](const QPointF &pt) {
        const int n = s.points.size();
        if (n < 3)
            return true;
        bool in = false;
        for (int i = 0, j = n - 1; i < n; j = i++) {
            const QPointF &a = s.points.at(i);
            const QPointF &b = s.points.at(j);
            if ((a.y() > pt.y()) == (b.y() > pt.y()))
                continue;
            const double xCross = (b.x() - a.x()) * (pt.y() - a.y()) / (b.y() - a.y()) + a.x();
            if (pt.x() < xCross)
                in = !in;
        }
        return in;
    };
    for (int i = 0; i <= kSteps; ++i) {
        for (int j = 0; j <= kSteps; ++j) {
            const QPointF pt(xLo + (xHi - xLo) * i / kSteps, yLo + (yHi - yLo) * j / kSteps);
            if (!inside(pt))
                continue;
            out << pt - o;
        }
    }
    // Вырожденный случай — ломаная нулевой площади: внутрь не попало ничего, но
    // проверить формулу всё равно надо. Берём сами вершины.
    if (out.isEmpty()) {
        for (const QPointF &pt : s.points)
            out << pt - o;
    }
    return out;
}

QString plasmaFieldText(const QString &formula, double number)
{
    return formula.trimmed().isEmpty() ? QString::number(number, 'g', 6) : formula.trimmed();
}

void setPlasmaField(const QString &text, double *number, QString *formula)
{
    // Одно поле на два случая: пользователь пишет либо число, либо выражение от
    // координат. Различаем разбором, а не отдельной галочкой, — в самом языке на
    // этом месте тоже один параметр, и ядро точно так же смотрит, число там или нет.
    bool ok = false;
    const double v = text.trimmed().toDouble(&ok);
    if (ok) {
        *number = v;
        formula->clear();
    } else {
        *number = 0.0;
        *formula = text.trimmed();
    }
}

/// @brief Имя файла профиля eps фигуры без крайних пробелов (описание — project.h).
/// @param[in] s  фигура
/// @return Имя без расширения; пусто — файла не просили.
QString epsProfileFile(const Shape &s)
{
    return s.epsProfile.file.trimmed();
}

/// @brief Имя, которое предложить в поле файла профиля eps (описание — project.h).
/// @param[in] s  фигура
/// @return Имя вида `eps<id>` без точки.
QString epsProfileSuggestedFile(const Shape &s)
{
    // Имя должно быть коротким и без точки: ядро дописывает расширение само,
    // а вторая точка в имени отменила бы это (`AddFileEpsExtention` ищет любую точку).
    return QStringLiteral("eps%1").arg(s.id);
}

bool Shape::hasUniformEdges() const
{
    if (!isContour())
        return true; // у заливки сторон нет: выражение задаёт площадь целиком
    for (const EdgeSpec &e : edges) {
        if (!e.present || e.custom)
            return false;
    }
    return true;
}

Material Shape::edgeMaterial(int edge) const
{
    if (edge < 0 || edge >= 4)
        return material;
    const EdgeSpec &e = edges[size_t(edge)];
    return e.custom ? e.material : material;
}

QPointF Shape::edgeFrom(int edge) const
{
    switch (edge) {
    case EdgeBottom: return {xMin, yMin};
    case EdgeRight:  return {xMax, yMin};
    case EdgeTop:    return {xMax, yMax};
    case EdgeLeft:   return {xMin, yMax};
    }
    return {};
}

QPointF Shape::edgeTo(int edge) const
{
    switch (edge) {
    case EdgeBottom: return {xMax, yMin};
    case EdgeRight:  return {xMax, yMax};
    case EdgeTop:    return {xMin, yMax};
    case EdgeLeft:   return {xMin, yMin};
    }
    return {};
}

void Shape::bounds(double *xLo, double *xHi, double *yLo, double *yHi) const
{
    double a = 0, b = 0, c = 0, d = 0;
    switch (kind) {
    case ShapeKind::Rectangle:
        a = std::min(xMin, xMax); b = std::max(xMin, xMax);
        c = std::min(yMin, yMax); d = std::max(yMin, yMax);
        break;
    case ShapeKind::Circle:
        a = center.x() - rMax; b = center.x() + rMax;
        c = center.y() - rMax; d = center.y() + rMax;
        break;
    case ShapeKind::Polygon:
    case ShapeKind::Polyline:
        if (points.isEmpty())
            break;
        a = b = points.first().x();
        c = d = points.first().y();
        for (const QPointF &p : points) {
            a = std::min(a, p.x()); b = std::max(b, p.x());
            c = std::min(c, p.y()); d = std::max(d, p.y());
        }
        break;
    }
    if (xLo) *xLo = a;
    if (xHi) *xHi = b;
    if (yLo) *yLo = c;
    if (yHi) *yHi = d;
}

bool portAlongY(PortSide s)
{
    return s == PortSide::Left || s == PortSide::Right;
}

bool portHasEps(const Port &p)
{
    // Сравнение с единицей точное и намеренное: eps ставит человек, а не расчёт.
    // Ровно 1 означает «вакуум, ничего не задавать» — параметр не пишем, пусть
    // работает умолчание ядра.
    return p.eps != 1.0;
}

double portPlane(const Port &p, const Box &b)
{
    if (p.plane.has_value())
        return *p.plane;
    switch (p.side) {
    case PortSide::Left:   return b.xMin;
    case PortSide::Right:  return b.xMax;
    case PortSide::Bottom: return b.yMin;
    case PortSide::Top:    return b.yMax;
    }
    return b.xMin;
}

PortSide directionForPlane(bool alongY, double plane, const Box &b)
{
    // Ровно правило ядра: ближе одного шага к границе — эта граница, иначе половина,
    // в которую попала координата. Шаг здесь не нужен: середина решает то же самое.
    if (alongY)
        return plane <= (b.xMin + b.xMax) / 2.0 ? PortSide::Left : PortSide::Right;
    return plane <= (b.yMin + b.yMax) / 2.0 ? PortSide::Bottom : PortSide::Top;
}

double signedArea(const QVector<QPointF> &pts)
{
    if (pts.size() < 3)
        return 0.0;
    double s = 0.0;
    for (int i = 0; i < pts.size(); ++i) {
        const QPointF &a = pts.at(i);
        const QPointF &b = pts.at((i + 1) % pts.size());
        s += a.x() * b.y() - b.x() * a.y();
    }
    return s / 2.0;
}

bool isConvex(const QVector<QPointF> &pts)
{
    if (pts.size() < 4)
        return true; // треугольник выпуклый всегда, а двум точкам выпуклость не про них
    bool pos = false, neg = false;
    for (int i = 0; i < pts.size(); ++i) {
        const QPointF &a = pts.at(i);
        const QPointF &b = pts.at((i + 1) % pts.size());
        const QPointF &c = pts.at((i + 2) % pts.size());
        const double cross = (b.x() - a.x()) * (c.y() - b.y()) - (b.y() - a.y()) * (c.x() - b.x());
        if (cross > 1e-9) pos = true;
        if (cross < -1e-9) neg = true;
        if (pos && neg)
            return false;
    }
    return true;
}

int Project::nextFreeId() const
{
    int id = 0;
    for (const Shape &s : shapes)
        id = std::max(id, s.id);
    for (const Port &p : ports)
        id = std::max(id, p.id);
    return id + 1; // номера произвольные, требование только >= 1 и уникальность
}

int Project::portOrdinal(int id) const
{
    for (int i = 0; i < ports.size(); ++i) {
        if (ports.at(i).id == id)
            return i + 1;
    }
    return 0;
}

int Project::stepCount() const
{
    if (sweep.active())
        return int(sweep.angles.size()) + (sweep.referenceStep ? 1 : 0);
    const int v = std::max(1, int(variants.size()));
    const int e = std::max(1, int(exciters.size()));
    const int f = std::max(1, int(freqs.size()));
    return v * e * f;
}

int Project::chunkCount() const
{
    // Конвейер делит только сетку частот. Угловой свип делить нечем: точку `.s`
    // подписывает угол, и частота там всего одна.
    if (!chunking.enabled || chunking.freqsPerTask < 1 || freqs.size() <= chunking.freqsPerTask)
        return 1;
    return (int(freqs.size()) + chunking.freqsPerTask - 1) / chunking.freqsPerTask;
}

/// @brief Ось поворота углового свипа (описание — project.h).
/// @param[in] p  задание
/// @return Ось поворота в координатах сцены, мм.
QPointF sweepPivot(const Project &p)
{
    if (!p.sweep.pivotAtCenter)
        return p.sweep.pivot;
    bool any = false;
    double xLo = 0, xHi = 0, yLo = 0, yHi = 0;
    for (const Shape &s : p.shapes) {
        if (!p.sweep.shapeIds.contains(s.id))
            continue;
        double a = 0, b = 0, c = 0, d = 0;
        s.bounds(&a, &b, &c, &d);
        xLo = any ? std::min(xLo, a) : a;
        xHi = any ? std::max(xHi, b) : b;
        yLo = any ? std::min(yLo, c) : c;
        yHi = any ? std::max(yHi, d) : d;
        any = true;
    }
    return any ? QPointF((xLo + xHi) / 2.0, (yLo + yHi) / 2.0) : p.sweep.pivot;
}

double sweepLabel(double angleDeg)
{
    double a = std::fmod(angleDeg, 360.0);
    if (a <= 1e-9)
        a += 360.0; // ноль ядро теряет: точка с подписью 0 в .s не попадает
    return a;
}

QString sweepFileTag(double angleDeg)
{
    QString s = QString::asprintf("%.6g", std::fabs(angleDeg));
    s.replace(QLatin1Char('.'), QLatin1Char('p'));
    return (angleDeg < 0.0 ? QStringLiteral("am") : QStringLiteral("a")) + s;
}

QString freqFileTag(double ghz)
{
    // Точку в имени файла ядро приняло бы за расширение, поэтому «p» вместо неё —
    // так же, как в метке угла.
    QString s = QString::asprintf("%.6g", ghz);
    s.replace(QLatin1Char('.'), QLatin1Char('p'));
    return QStringLiteral("f") + s;
}

QPointF rotatePoint(const QPointF &p, const QPointF &pivot, double angleDeg)
{
    const double a = angleDeg * 3.14159265358979323846 / 180.0;
    const double c = std::cos(a), s = std::sin(a);
    const double dx = p.x() - pivot.x(), dy = p.y() - pivot.y();
    return {pivot.x() + dx * c - dy * s, pivot.y() + dx * s + dy * c};
}

/// @brief Копия фигуры, повёрнутая вокруг оси (описание — project.h).
/// @param[in] s         исходная фигура
/// @param[in] pivot     ось поворота, мм
/// @param[in] angleDeg  угол, градусы, против часовой стрелки
/// @return Повёрнутая копия фигуры.
Shape rotatedShape(const Shape &s, const QPointF &pivot, double angleDeg)
{
    Shape r = s;
    switch (s.kind) {
    case ShapeKind::Circle: {
        // У окружности поворачиваются центр и раскрыв сектора; радиусы те же.
        const double a = angleDeg * 3.14159265358979323846 / 180.0;
        r.center = rotatePoint(s.center, pivot, angleDeg);
        const bool full = (s.fiMax - s.fiMin) >= 2 * 3.14159265358979323846 - 1e-9;
        if (!full) {
            r.fiMin = s.fiMin + a;
            r.fiMax = s.fiMax + a;
        }
        break;
    }
    case ShapeKind::Rectangle: {
        // Повёрнутый прямоугольник — это уже полигон: у RECT_STAT стороны идут по осям.
        r.kind = ShapeKind::Polygon;
        r.points.clear();
        for (int e = 0; e < 4; ++e)
            r.points << rotatePoint(s.edgeFrom(e), pivot, angleDeg);
        break;
    }
    case ShapeKind::Polygon:
    case ShapeKind::Polyline:
        for (QPointF &pt : r.points)
            pt = rotatePoint(pt, pivot, angleDeg);
        break;
    }
    if (s.plasma.customOrigin)
        r.plasma.origin = rotatePoint(s.plasma.origin, pivot, angleDeg);
    return r;
}

QVector<Port> Project::activePorts() const
{
    QVector<Port> out;
    for (const Port &p : ports) {
        if (p.active)
            out << p;
    }
    return out;
}

namespace {

/// @brief Возвращает расстояние от точки до отрезка.
/// @param[in] p  точка, мм
/// @param[in] a  начало отрезка, мм
/// @param[in] b  конец отрезка, мм
/// @return Расстояние до ближайшей точки отрезка, мм (для вырожденного отрезка — до `a`).
double distToSegment(const QPointF &p, const QPointF &a, const QPointF &b)
{
    const double dx = b.x() - a.x(), dy = b.y() - a.y();
    const double len2 = dx * dx + dy * dy;
    double t = 0.0;
    if (len2 > 0.0)
        t = std::clamp(((p.x() - a.x()) * dx + (p.y() - a.y()) * dy) / len2, 0.0, 1.0);
    return std::hypot(p.x() - (a.x() + t * dx), p.y() - (a.y() + t * dy));
}

} // namespace

/// @brief Лежит ли точка в площади фигуры или в пределах допуска от неё (описание — project.h).
/// @param[in] s    фигура
/// @param[in] pt   точка сцены, мм
/// @param[in] tol  допуск, мм
/// @return true — точка в фигуре или в пределах `tol`.
bool shapeCovers(const Shape &s, const QPointF &pt, double tol)
{
    switch (s.kind) {
    case ShapeKind::Rectangle:
        return pt.x() >= s.xMin - tol && pt.x() <= s.xMax + tol && pt.y() >= s.yMin - tol
               && pt.y() <= s.yMax + tol;
    case ShapeKind::Circle: {
        const double dx = pt.x() - s.center.x(), dy = pt.y() - s.center.y();
        const double r = std::hypot(dx, dy);
        if (r < s.rMin - tol || r > s.rMax + tol)
            return false;
        constexpr double kTwoPi = 2.0 * 3.14159265358979323846;
        if (s.fiMax - s.fiMin >= kTwoPi - 1e-9 || r <= tol)
            return true;
        // Угол точки, приведённый к началу сектора. Допуск по дуге — tol/r радиан.
        double a = std::atan2(dy, dx) - s.fiMin;
        a = std::fmod(a, kTwoPi);
        if (a < 0.0)
            a += kTwoPi;
        const double slack = r > 0.0 ? tol / r : 0.0;
        return a <= (s.fiMax - s.fiMin) + slack || a >= kTwoPi - slack;
    }
    case ShapeKind::Polygon: {
        const int n = s.points.size();
        if (n < 3)
            break;
        bool in = false;
        for (int i = 0, j = n - 1; i < n; j = i++) {
            const QPointF &a = s.points.at(i);
            const QPointF &b = s.points.at(j);
            if ((a.y() > pt.y()) != (b.y() > pt.y())) {
                const double xc = (b.x() - a.x()) * (pt.y() - a.y()) / (b.y() - a.y()) + a.x();
                if (pt.x() < xc)
                    in = !in;
            }
        }
        if (in)
            return true;
        for (int i = 0, j = n - 1; i < n; j = i++) {
            if (distToSegment(pt, s.points.at(j), s.points.at(i)) <= tol)
                return true;
        }
        return false;
    }
    case ShapeKind::Polyline:
        break;
    }
    // Ломаная (и вырожденный полигон): площади нет, только близость к звеньям.
    for (int i = 1; i < s.points.size(); ++i) {
        if (distToSegment(pt, s.points.at(i - 1), s.points.at(i)) <= tol)
            return true;
    }
    return s.points.size() == 1 && std::hypot(pt.x() - s.points.first().x(),
                                              pt.y() - s.points.first().y()) <= tol;
}

/// @brief eps среды в середине апертуры порта (описание — project.h).
/// @param[in] p        задание
/// @param[in] port     порт
/// @param[in] variant  вариант геометрии; nullptr — все фигуры
/// @return eps среды (не добавка); 1 — вакуум.
double portMediumEps(const Project &p, const Port &port, const Variant *variant)
{
    // Середина апертуры в плоскости входа: «блок, в котором стоит вход».
    const double plane = portPlane(port, p.box);
    const double mid = (port.from + port.to) / 2.0;
    const QPointF at = portAlongY(port.side) ? QPointF(plane, mid) : QPointF(mid, plane);

    double eps = 1.0;
    for (const Shape &s : p.shapes) {
        if (variant && !variant->isEnabled(s.id))
            continue;
        if (s.material != Material::Dielectric || s.plasma.any() || s.isMoving())
            continue;
        if (s.kind == ShapeKind::Polyline || !shapeCovers(s, at, 1e-9))
            continue;
        if (s.hasEpsProfile()) {
            // Профиль формулой — считаем его в этой точке тем же вычислителем, что и
            // проверка. Из файла одного числа нет: такую фигуру пропускаем.
            if (s.epsProfile.fromFile)
                continue;
            const QPointF local = at - s.epsOrigin();
            const auto v = ExprProgram::compile(s.epsProfile.formula).evalEps(local.x(), local.y());
            if (v && std::isfinite(*v))
                eps = *v;
            continue;
        }
        eps = s.eps; // из нескольких берём нарисованную позже: так её показывает и холст
    }
    return eps;
}

/// @brief eps, которую эмиттер запишет входу (описание — project.h).
/// @param[in] p        задание
/// @param[in] port     порт
/// @param[in] variant  вариант геометрии; nullptr — все фигуры
/// @return eps входа (сама eps).
double portEps(const Project &p, const Port &port, const Variant *variant)
{
    return port.epsAuto ? portMediumEps(p, port, variant) : port.eps;
}

} // namespace tpl
