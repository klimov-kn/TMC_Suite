/// @file mesh3d.cpp
/// @brief Реализация сетки 3D: габарит, склейка вершин по решётке, масштаб, объединение.
///
/// Описание — в mesh3d.h.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#mesh-import
#include "mesh3d.h"

#include <QHash>

#include <algorithm>
#include <cmath>

namespace tpl {

double Bounds3::size() const
{
    if (!valid)
        return 0.0;
    const double dx = hi.x - lo.x, dy = hi.y - lo.y, dz = hi.z - lo.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

Bounds3 Mesh::bounds() const
{
    Bounds3 b;
    for (const Vec3 &v : vertices) {
        if (!b.valid) {
            b.lo = b.hi = v;
            b.valid = true;
            continue;
        }
        b.lo.x = std::min(b.lo.x, v.x);
        b.lo.y = std::min(b.lo.y, v.y);
        b.lo.z = std::min(b.lo.z, v.z);
        b.hi.x = std::max(b.hi.x, v.x);
        b.hi.y = std::max(b.hi.y, v.y);
        b.hi.z = std::max(b.hi.z, v.z);
    }
    return b;
}

namespace {

/// @brief Ячейка решётки склейки: целые номера по x, y, z.
struct Cell {
    /// @name Номера ячейки
    ///@{
    /// Номера ячейки решётки по осям x, y, z.
    qint64 x, y, z;
    ///@}
    /// @brief Сравнивает ячейки.
    /// @param[in] o  другая ячейка.
    /// @return true — все три номера совпадают.
    bool operator==(const Cell &o) const { return x == o.x && y == o.y && z == o.z; }
};

/// @brief Хеш ячейки решётки для `QHash`.
/// @param[in] c     ячейка.
/// @param[in] seed  начальное значение хеша.
/// @return Хеш.
size_t qHash(const Cell &c, size_t seed = 0)
{
    return ::qHash(c.x * 73856093LL ^ c.y * 19349663LL ^ c.z * 83492791LL, seed);
}

} // namespace

void Mesh::weld(double tol)
{
    if (vertices.isEmpty())
        return;
    if (tol <= 0.0)
        tol = std::max(1e-12, bounds().size() * 1e-7);

    // Решётка с шагом tol: кандидаты на склейку лежат в той же или соседней ячейке.
    // Смотрим все 27 соседей, иначе две точки по разные стороны границы ячейки, ближе
    // tol друг к другу, остались бы разными — и контур в этом месте порвался бы.
    QHash<Cell, QVector<int>> grid;
    grid.reserve(vertices.size());
    QVector<int> remap(vertices.size(), -1);
    QVector<Vec3> out;
    out.reserve(vertices.size());
    const double tol2 = tol * tol;
    const auto cellOf = [tol](const Vec3 &v) {
        return Cell{qint64(std::floor(v.x / tol)), qint64(std::floor(v.y / tol)),
                    qint64(std::floor(v.z / tol))};
    };
    for (int i = 0; i < vertices.size(); ++i) {
        const Vec3 &v = vertices.at(i);
        const Cell c = cellOf(v);
        int found = -1;
        for (qint64 dx = -1; dx <= 1 && found < 0; ++dx)
            for (qint64 dy = -1; dy <= 1 && found < 0; ++dy)
                for (qint64 dz = -1; dz <= 1 && found < 0; ++dz) {
                    const auto it = grid.constFind(Cell{c.x + dx, c.y + dy, c.z + dz});
                    if (it == grid.constEnd())
                        continue;
                    for (int j : *it) {
                        const Vec3 &w = out.at(j);
                        const double ex = w.x - v.x, ey = w.y - v.y, ez = w.z - v.z;
                        if (ex * ex + ey * ey + ez * ez <= tol2) {
                            found = j;
                            break;
                        }
                    }
                }
        if (found < 0) {
            found = int(out.size());
            out << v;
            grid[c] << found;
        }
        remap[i] = found;
    }

    // Треугольник, у которого после склейки совпали вершины, площади не имеет и в
    // срезе дал бы отрезок нулевой длины — выбрасываем, сдвигая границы тел.
    QVector<std::array<int, 3>> tri2;
    tri2.reserve(tris.size());
    for (MeshBody &b : bodies) {
        const int from = b.firstTri, to = b.firstTri + b.triCount;
        b.firstTri = int(tri2.size());
        for (int t = from; t < to; ++t) {
            std::array<int, 3> n{remap.at(tris.at(t)[0]), remap.at(tris.at(t)[1]),
                                 remap.at(tris.at(t)[2])};
            if (n[0] == n[1] || n[1] == n[2] || n[0] == n[2])
                continue;
            tri2 << n;
        }
        b.triCount = int(tri2.size()) - b.firstTri;
    }
    vertices = out;
    tris = tri2;
}

void Mesh::scale(double k)
{
    for (Vec3 &v : vertices) {
        v.x *= k;
        v.y *= k;
        v.z *= k;
    }
}


} // namespace tpl
