/// @file convexsplit.cpp
/// @brief Реализация разбиения области на выпуклые куски методом вертикальных полос.
///
/// Описание — в convexsplit.h.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#geometry
#include "convexsplit.h"

#include <algorithm>
#include <cmath>

namespace tpl {

double polyArea(const QVector<QPointF> &p)
{
    double a = 0.0;
    for (int i = 0, n = int(p.size()); i < n; ++i) {
        const QPointF &u = p.at(i), &w = p.at((i + 1) % n);
        a += u.x() * w.y() - w.x() * u.y();
    }
    return a / 2.0;
}

QVector<QPointF> padTriangle(const QVector<QPointF> &p)
{
    if (p.size() != 3)
        return p;
    // Середина самой длинной стороны: так новые углы самые тупые, и ни одна проверка
    // выпуклости не споткнётся о почти вырожденный угол.
    int best = 0;
    double len = -1.0;
    for (int i = 0; i < 3; ++i) {
        const QPointF d = p.at((i + 1) % 3) - p.at(i);
        const double l = d.x() * d.x() + d.y() * d.y();
        if (l > len) {
            len = l;
            best = i;
        }
    }
    QVector<QPointF> out;
    for (int i = 0; i < 3; ++i) {
        out << p.at(i);
        if (i == best)
            out << (p.at(i) + p.at((i + 1) % 3)) / 2.0;
    }
    return out;
}

namespace {

/// @brief Невертикальное ребро контура с упорядоченными по x концами.
struct Edge {
    /// @name Концы ребра
    ///@{
    /// Левый (`a`) и правый (`b`) концы ребра.
    QPointF a, b; // a.x < b.x
    ///@}
    /// @brief Вычисляет ординату ребра на абсциссе.
    /// @param[in] x  абсцисса внутри [a.x; b.x].
    /// @return Ордината точки ребра.
    double yAt(double x) const
    {
        const double t = (x - a.x()) / (b.x() - a.x());
        return a.y() + t * (b.y() - a.y());
    }
};

/// Выпуклый кусок, растущий слева направо: нижняя и верхняя цепочки (x растёт).
struct Piece {
    /// @name Цепочки куска
    ///@{
    /// Нижняя и верхняя цепочки вершин куска (x растёт).
    QVector<QPointF> lower, upper;
    ///@}
    /// @return Абсцисса правого края куска.
    double rightX() const { return lower.last().x(); }
};

/// @brief Вычисляет векторное произведение (a - o) x (b - o).
/// @param[in] o  общая точка.
/// @param[in] a  конец первого вектора.
/// @param[in] b  конец второго вектора.
/// @return Положительно — поворот влево, отрицательно — вправо.
double cross(const QPointF &o, const QPointF &a, const QPointF &b)
{
    return (a.x() - o.x()) * (b.y() - o.y()) - (a.y() - o.y()) * (b.x() - o.x());
}

/// Добавить точку в цепочку. Нижняя цепочка выпуклого куска поворачивает только
/// влево (cross >= 0), верхняя — только вправо. Точка на продолжении прямой заменяет
/// предыдущую: лишняя вершина ядру не нужна. false — выпуклость нарушилась бы.
/// @param[in,out] chain  цепочка вершин.
/// @param[in]     p      новая точка.
/// @param[in]     lower  true — нижняя цепочка, false — верхняя.
/// @param[in]     eps    допуск коллинеарности.
/// @return true — точка добавлена.
bool extend(QVector<QPointF> *chain, const QPointF &p, bool lower, double eps)
{
    if (chain->size() >= 2) {
        const double c = cross(chain->at(chain->size() - 2), chain->last(), p);
        if (lower ? c < -eps : c > eps)
            return false;
        if (std::fabs(c) <= eps)
            chain->removeLast();
    }
    chain->append(p);
    return true;
}

} // namespace

QVector<QVector<QPointF>> convexPieces(const QVector<QVector<QPointF>> &loops)
{
    // Шаг 1: невертикальные рёбра всех контуров и абсциссы всех вершин (границы полос).
    QVector<Edge> edges;
    QVector<double> xs;
    double span = 0.0;
    for (const auto &loop : loops) {
        for (int i = 0, n = int(loop.size()); i < n; ++i) {
            QPointF a = loop.at(i), b = loop.at((i + 1) % n);
            xs << a.x();
            span = std::max({span, std::fabs(a.x()), std::fabs(a.y())});
            if (a.x() == b.x())
                continue; // вертикальное ребро полосу не пересекает
            if (a.x() > b.x())
                std::swap(a, b);
            edges << Edge{a, b};
        }
    }
    std::sort(xs.begin(), xs.end());
    xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
    const double eps = std::max(1e-12, span * 1e-10);

    QVector<QVector<QPointF>> out;
    QVector<Piece> open; // куски, у которых правый край лежит на текущей x

    const auto flush = [&](const Piece &p) {
        QVector<QPointF> poly = p.lower;
        for (int i = int(p.upper.size()) - 1; i >= 0; --i)
            poly << p.upper.at(i);
        // Треугольник: у трапеции нулевой высоты на одном конце верх и низ сходятся.
        QVector<QPointF> clean;
        for (const QPointF &q : poly)
            if (clean.isEmpty() || std::hypot(q.x() - clean.last().x(), q.y() - clean.last().y()) > eps)
                clean << q;
        if (clean.size() > 2
            && std::hypot(clean.first().x() - clean.last().x(), clean.first().y() - clean.last().y()) <= eps)
            clean.removeLast();
        if (clean.size() >= 3 && std::fabs(polyArea(clean)) > eps * eps)
            out << clean;
    };

    // Шаг 2: в каждой полосе пары рёбер (снизу вверх, по чётно-нечётному правилу) дают
    // трапеции; трапеция сливается с куском слева, если край совпадает и выпуклость
    // сохраняется, иначе кусок слева закрывается и начинается новый.
    for (int s = 0; s + 1 < xs.size(); ++s) {
        const double x0 = xs.at(s), x1 = xs.at(s + 1);
        if (x1 - x0 <= eps)
            continue;
        const double xm = (x0 + x1) / 2.0;
        // Рёбра, проходящие через полосу насквозь (вершин внутри полосы нет: границы
        // полос стоят на всех вершинах), упорядоченные снизу вверх.
        QVector<const Edge *> cut;
        for (const Edge &e : edges)
            if (e.a.x() <= x0 + eps && e.b.x() >= x1 - eps)
                cut << &e;
        std::sort(cut.begin(), cut.end(),
                  [xm](const Edge *p, const Edge *q) { return p->yAt(xm) < q->yAt(xm); });

        QVector<Piece> next;
        for (int k = 0; k + 1 < cut.size(); k += 2) {
            const Edge &lo = *cut.at(k), &hi = *cut.at(k + 1);
            const QPointF bl(x0, lo.yAt(x0)), br(x1, lo.yAt(x1));
            const QPointF tl(x0, hi.yAt(x0)), tr(x1, hi.yAt(x1));
            // Слить с куском слева, если их общий край совпадает целиком и
            // объединение остаётся выпуклым.
            bool merged = false;
            for (int i = 0; i < open.size(); ++i) {
                Piece &p = open[i];
                if (std::fabs(p.rightX() - x0) > eps)
                    continue;
                if (std::fabs(p.lower.last().y() - bl.y()) > eps
                    || std::fabs(p.upper.last().y() - tl.y()) > eps)
                    continue;
                Piece trial = p;
                if (extend(&trial.lower, br, true, eps) && extend(&trial.upper, tr, false, eps)) {
                    next << trial;
                    open.removeAt(i);
                    merged = true;
                }
                break;
            }
            if (!merged) {
                Piece fresh;
                fresh.lower = {bl, br};
                fresh.upper = {tl, tr};
                next << fresh;
            }
        }
        // Куски, которым продолжения не нашлось, закончились.
        for (const Piece &p : open)
            flush(p);
        open = next;
    }
    for (const Piece &p : open)
        flush(p);
    return out;
}

} // namespace tpl
