/// @file meshslice.cpp
/// @brief Реализация среза сетки: отрезки по рёбрам, сшивка в цепочки, вложенность, Дуглас — Пейкер.
///
/// Описание — в meshslice.h.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#mesh-import
#include "meshslice.h"

#include <QHash>

#include <algorithm>
#include <cmath>

namespace tpl {

QString slicePlaneName(SlicePlane p)
{
    switch (p) {
    case SlicePlane::XY: return QStringLiteral("XY");
    case SlicePlane::YZ: return QStringLiteral("YZ");
    case SlicePlane::XZ: return QStringLiteral("XZ");
    }
    return QString();
}

PlaneAxes planeAxes(SlicePlane p)
{
    switch (p) {
    case SlicePlane::XY: return {0, 1, 2};
    case SlicePlane::YZ: return {1, 2, 0};
    case SlicePlane::XZ: return {0, 2, 1};
    }
    return {};
}

double axisValue(const Vec3 &p, int axis)
{
    return axis == 0 ? p.x : axis == 1 ? p.y : p.z;
}

QString normalAxisName(SlicePlane p)
{
    static const char *names[] = {"X", "Y", "Z"};
    return QString::fromLatin1(names[planeAxes(p).normal]);
}

namespace {

/// @brief Составляет ключ ребра по номерам вершин (порядок вершин не важен).
/// @param[in] a  номер первой вершины.
/// @param[in] b  номер второй вершины.
/// @return 64-битный ключ (меньший номер — в старших 32 битах).
quint64 edgeKey(int a, int b)
{
    if (a > b)
        std::swap(a, b);
    return (quint64(quint32(a)) << 32) | quint32(b);
}

/// Отрезки одного тела -> цепочки. Узел цепочки — ребро сетки, которое пересекла
/// плоскость; у ребра замкнутой поверхности ровно два треугольника, поэтому у узла
/// ровно два соседа, и цепочки обходятся без единого сравнения расстояний.
/// @param[in] segs   отрезки среза: пары ключей рёбер.
/// @param[in] where  точка пересечения плоскости с каждым ребром, мм в координатах (u, v).
/// @return Контуры: сначала открытые (от концов), затем замкнутые.
QVector<SliceLoop> chain(const QVector<std::array<quint64, 2>> &segs,
                         const QHash<quint64, QPointF> &where)
{
    QHash<quint64, QVector<int>> at; // ребро -> отрезки, которые на нём кончаются
    at.reserve(segs.size() * 2);
    for (int i = 0; i < segs.size(); ++i) {
        at[segs.at(i)[0]] << i;
        at[segs.at(i)[1]] << i;
    }
    QVector<bool> used(segs.size(), false);
    QVector<SliceLoop> out;

    const auto walk = [&](int first, quint64 from) {
        SliceLoop loop;
        quint64 node = from;
        int seg = first;
        loop.pts << where.value(node);
        while (seg >= 0 && !used.at(seg)) {
            used[seg] = true;
            const quint64 next = segs.at(seg)[0] == node ? segs.at(seg)[1] : segs.at(seg)[0];
            node = next;
            loop.pts << where.value(node);
            seg = -1;
            for (int cand : at.value(node)) {
                if (!used.at(cand)) {
                    seg = cand;
                    break;
                }
            }
        }
        // Вернулись в начальное ребро — контур замкнут; последнюю точку не дублируем.
        loop.closed = (node == from) && loop.pts.size() > 3;
        if (loop.closed)
            loop.pts.removeLast();
        return loop;
    };

    // Сначала открытые цепочки — от концов (ребро, на котором кончается один отрезок):
    // начни обход с середины, и открытая цепочка распалась бы на две.
    for (auto it = at.constBegin(); it != at.constEnd(); ++it) {
        if (it.value().size() != 1 || used.at(it.value().first()))
            continue;
        out << walk(it.value().first(), it.key());
    }
    for (int i = 0; i < segs.size(); ++i) {
        if (!used.at(i))
            out << walk(i, segs.at(i)[0]);
    }
    return out;
}

/// @brief Вычисляет знаковую площадь многоугольника (формула Гаусса).
/// @param[in] p  вершины.
/// @return Площадь, мм^2; знак — направление обхода.
double polygonArea(const QVector<QPointF> &p)
{
    double a = 0.0;
    for (int i = 0, n = int(p.size()); i < n; ++i) {
        const QPointF &u = p.at(i), &w = p.at((i + 1) % n);
        a += u.x() * w.y() - w.x() * u.y();
    }
    return a / 2.0;
}

/// @brief Вычисляет расстояние от точки до отрезка.
/// @param[in] p  точка.
/// @param[in] a  начало отрезка.
/// @param[in] b  конец отрезка.
/// @return Расстояние, мм.
double distToSegment(const QPointF &p, const QPointF &a, const QPointF &b)
{
    const double dx = b.x() - a.x(), dy = b.y() - a.y();
    const double len2 = dx * dx + dy * dy;
    double t = len2 > 0 ? ((p.x() - a.x()) * dx + (p.y() - a.y()) * dy) / len2 : 0.0;
    t = std::clamp(t, 0.0, 1.0);
    const double ex = a.x() + t * dx - p.x(), ey = a.y() + t * dy - p.y();
    return std::sqrt(ex * ex + ey * ey);
}

/// @brief Рекурсивно отмечает точки, которые нужно сохранить (алгоритм Дугласа — Пейкера).
/// @param[in]     p    точки ломаной.
/// @param[in]     i0   номер начальной опорной точки.
/// @param[in]     i1   номер конечной опорной точки.
/// @param[in]     tol  допуск отклонения, мм.
/// @param[in,out] keep флаги сохранения точек.
void douglasPeucker(const QVector<QPointF> &p, int i0, int i1, double tol, QVector<bool> *keep)
{
    if (i1 <= i0 + 1)
        return;
    double best = -1.0;
    int at = -1;
    for (int i = i0 + 1; i < i1; ++i) {
        const double d = distToSegment(p.at(i), p.at(i0), p.at(i1));
        if (d > best) {
            best = d;
            at = i;
        }
    }
    if (best > tol) {
        (*keep)[at] = true;
        douglasPeucker(p, i0, at, tol, keep);
        douglasPeucker(p, at, i1, tol, keep);
    }
}

} // namespace

bool pointInPolygon(const QPointF &p, const QVector<QPointF> &poly)
{
    bool in = false;
    for (int i = 0, j = int(poly.size()) - 1; i < poly.size(); j = i++) {
        const QPointF &a = poly.at(i), &b = poly.at(j);
        if ((a.y() > p.y()) != (b.y() > p.y())
            && p.x() < (b.x() - a.x()) * (p.y() - a.y()) / (b.y() - a.y()) + a.x())
            in = !in;
    }
    return in;
}

SliceResult sliceMesh(const Mesh &mesh, SlicePlane plane, double offset)
{
    SliceResult res;
    const PlaneAxes ax = planeAxes(plane);
    // Расстояния вершин до плоскости — один раз на сетку, а не на каждый треугольник.
    QVector<double> d(mesh.vertices.size());
    for (int i = 0; i < mesh.vertices.size(); ++i)
        d[i] = axisValue(mesh.vertices.at(i), ax.normal) - offset;

    for (int b = 0; b < mesh.bodies.size(); ++b) {
        const MeshBody &body = mesh.bodies.at(b);
        QVector<std::array<quint64, 2>> segs;
        QHash<quint64, QPointF> where;
        for (int t = body.firstTri; t < body.firstTri + body.triCount; ++t) {
            const auto &tri = mesh.tris.at(t);
            // «Выше или на плоскости» против «ниже»: ноль считается выше.
            const bool up[3] = {d.at(tri[0]) >= 0, d.at(tri[1]) >= 0, d.at(tri[2]) >= 0};
            if (up[0] == up[1] && up[1] == up[2])
                continue;
            std::array<quint64, 2> seg{};
            int n = 0;
            for (int e = 0; e < 3; ++e) {
                const int a = tri[e], c = tri[(e + 1) % 3];
                if (up[e] == up[(e + 1) % 3])
                    continue;
                const quint64 key = edgeKey(a, c);
                if (!where.contains(key)) {
                    // Точку считаем от вершин в ПОРЯДКЕ ключа: соседний треугольник
                    // проходит то же ребро в обратную сторону и обязан получить ту же
                    // точку до последнего бита.
                    const int lo = std::min(a, c), hi = std::max(a, c);
                    const double dl = d.at(lo), dh = d.at(hi);
                    const double s = dl / (dl - dh);
                    const Vec3 &P = mesh.vertices.at(lo), &Q = mesh.vertices.at(hi);
                    const double u = axisValue(P, ax.u) + s * (axisValue(Q, ax.u) - axisValue(P, ax.u));
                    const double v = axisValue(P, ax.v) + s * (axisValue(Q, ax.v) - axisValue(P, ax.v));
                    where.insert(key, QPointF(u, v));
                }
                if (n < 2)
                    seg[n++] = key;
            }
            if (n == 2 && seg[0] != seg[1])
                segs << seg;
        }
        if (segs.isEmpty())
            continue;

        SliceBody sb;
        sb.body = b;
        sb.loops = chain(segs, where);
        // Вложенность: сколько других замкнутых контуров того же тела содержат точку
        // этого. Срезы не пересекаются (сетка — поверхность тела), поэтому одной точки
        // достаточно.
        for (int i = 0; i < sb.loops.size(); ++i) {
            SliceLoop &L = sb.loops[i];
            if (!L.closed) {
                ++res.openLoops;
                continue;
            }
            for (int j = 0; j < sb.loops.size(); ++j) {
                if (j != i && sb.loops.at(j).closed && pointInPolygon(L.pts.first(), sb.loops.at(j).pts))
                    ++L.depth;
            }
        }
        res.bodies << sb;
    }
    return res;
}

QVector<QPointF> simplifyPolyline(const QVector<QPointF> &in, double tol, bool closed)
{
    // Сначала — повторы подряд: на вершине, лежащей ровно в плоскости, несколько рёбер
    // дают одну и ту же точку.
    QVector<QPointF> p;
    p.reserve(in.size());
    const double eps = std::max(1e-12, tol * 1e-6);
    for (const QPointF &q : in) {
        if (p.isEmpty() || std::hypot(q.x() - p.last().x(), q.y() - p.last().y()) > eps)
            p << q;
    }
    if (closed && p.size() > 1
        && std::hypot(p.first().x() - p.last().x(), p.first().y() - p.last().y()) <= eps)
        p.removeLast();
    if (tol <= 0.0 || p.size() < 3)
        return p;

    if (!closed) {
        QVector<bool> keep(p.size(), false);
        keep.first() = keep.last() = true;
        douglasPeucker(p, 0, int(p.size()) - 1, tol, &keep);
        QVector<QPointF> out;
        for (int i = 0; i < p.size(); ++i)
            if (keep.at(i))
                out << p.at(i);
        return out;
    }

    // Замкнутый: опорные точки — первая и самая далёкая от неё; дальше две половины.
    int far = 0;
    double best = -1.0;
    for (int i = 1; i < p.size(); ++i) {
        const double dd = std::hypot(p.at(i).x() - p.first().x(), p.at(i).y() - p.first().y());
        if (dd > best) {
            best = dd;
            far = i;
        }
    }
    QVector<QPointF> ring = p;
    ring << p.first();
    QVector<bool> keep(ring.size(), false);
    keep[0] = keep[far] = keep.last() = true;
    douglasPeucker(ring, 0, far, tol, &keep);
    douglasPeucker(ring, far, int(ring.size()) - 1, tol, &keep);
    QVector<QPointF> out;
    for (int i = 0; i + 1 < ring.size(); ++i)
        if (keep.at(i))
            out << ring.at(i);
    // Контур, который упрощение свело к отрезку, — это щель тоньше допуска. Оставляем
    // его как был: выбросить значило бы потерять стенку молча.
    if (out.size() < 3 || std::fabs(polygonArea(out)) <= 0.0)
        return p;
    return out;
}

void simplifySlice(SliceResult *r, double tol)
{
    for (SliceBody &b : r->bodies)
        for (SliceLoop &l : b.loops)
            l.pts = simplifyPolyline(l.pts, tol, l.closed);
}

} // namespace tpl
