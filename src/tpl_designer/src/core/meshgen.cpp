/// @file meshgen.cpp
/// @brief Реализация генератора тестовых тел и их записи в STL/OBJ/MTL.
///
/// Описание — в meshgen.h.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#mesh-import
#include "meshgen.h"

#include <QPointF>
#include <QtEndian>

#include <cmath>
#include <cstring>

namespace tpl::meshgen {

namespace {

/// Число pi.
constexpr double kPi = 3.14159265358979323846;

/// @brief Собирает сетку из одного тела.
/// @param[in] name  имя и материал тела.
/// @param[in] v     вершины.
/// @param[in] t     треугольники.
/// @return Сетка.
Mesh single(const QString &name, const QVector<Vec3> &v, const QVector<std::array<int, 3>> &t)
{
    Mesh m;
    m.vertices = v;
    m.tris = t;
    MeshBody b;
    b.name = name;
    b.material = name;
    b.triCount = int(t.size());
    m.bodies << b;
    return m;
}

/// @brief Добавляет четырёхугольник a-b-c-d двумя треугольниками.
/// @param[in,out] t  список треугольников.
/// @param[in]     a  номер вершины 1.
/// @param[in]     b  номер вершины 2.
/// @param[in]     c  номер вершины 3.
/// @param[in]     d  номер вершины 4.
void quad(QVector<std::array<int, 3>> *t, int a, int b, int c, int d)
{
    *t << std::array<int, 3>{a, b, c} << std::array<int, 3>{a, c, d};
}

} // namespace

Mesh box(const QString &name, Vec3 lo, Vec3 hi)
{
    QVector<Vec3> v;
    for (int i = 0; i < 8; ++i)
        v << Vec3{(i & 1) ? hi.x : lo.x, (i & 2) ? hi.y : lo.y, (i & 4) ? hi.z : lo.z};
    QVector<std::array<int, 3>> t;
    quad(&t, 0, 2, 3, 1); // низ
    quad(&t, 4, 5, 7, 6); // верх
    quad(&t, 0, 1, 5, 4);
    quad(&t, 1, 3, 7, 5);
    quad(&t, 3, 2, 6, 7);
    quad(&t, 2, 0, 4, 6);
    return single(name, v, t);
}

Mesh tube(const QString &name, Vec3 c, double rIn, double rOut, double h, int seg)
{
    QVector<Vec3> v;
    QVector<std::array<int, 3>> t;
    for (int i = 0; i < seg; ++i) {
        const double a = 2 * kPi * i / seg;
        for (double r : {rOut, rIn})
            for (double z : {c.z, c.z + h})
                v << Vec3{c.x + r * std::cos(a), c.y + r * std::sin(a), z};
    }
    // На шаг: внешний низ, внешний верх, внутренний низ, внутренний верх.
    for (int i = 0; i < seg; ++i) {
        const int a = 4 * i, b = 4 * ((i + 1) % seg);
        quad(&t, a, b, b + 1, a + 1);         // наружная стенка
        quad(&t, a + 2, a + 3, b + 3, b + 2); // внутренняя
        quad(&t, a, a + 2, b + 2, b);         // низ
        quad(&t, a + 1, b + 1, b + 3, a + 3); // верх
    }
    return single(name, v, t);
}

Mesh cylinder(const QString &name, Vec3 c, double r, double h, int seg)
{
    QVector<Vec3> v;
    QVector<std::array<int, 3>> t;
    v << Vec3{c.x, c.y, c.z} << Vec3{c.x, c.y, c.z + h};
    for (int i = 0; i < seg; ++i) {
        const double a = 2 * kPi * i / seg;
        v << Vec3{c.x + r * std::cos(a), c.y + r * std::sin(a), c.z}
          << Vec3{c.x + r * std::cos(a), c.y + r * std::sin(a), c.z + h};
    }
    for (int i = 0; i < seg; ++i) {
        const int a = 2 + 2 * i, b = 2 + 2 * ((i + 1) % seg);
        quad(&t, a, b, b + 1, a + 1);
        t << std::array<int, 3>{0, b, a} << std::array<int, 3>{1, a + 1, b + 1};
    }
    return single(name, v, t);
}

Mesh torus(const QString &name, Vec3 c, double R, double r, int segR, int segr)
{
    QVector<Vec3> v;
    QVector<std::array<int, 3>> t;
    for (int i = 0; i < segR; ++i) {
        const double u = 2 * kPi * i / segR;
        for (int j = 0; j < segr; ++j) {
            const double w = 2 * kPi * j / segr;
            const double rr = R + r * std::cos(w);
            v << Vec3{c.x + rr * std::cos(u), c.y + rr * std::sin(u), c.z + r * std::sin(w)};
        }
    }
    for (int i = 0; i < segR; ++i)
        for (int j = 0; j < segr; ++j) {
            const int a = i * segr + j, b = ((i + 1) % segR) * segr + j;
            const int a1 = i * segr + (j + 1) % segr, b1 = ((i + 1) % segR) * segr + (j + 1) % segr;
            quad(&t, a, b, b1, a1);
        }
    return single(name, v, t);
}

Mesh prism(const QString &name, const QVector<QPointF> &poly, double z0, double h)
{
    QVector<Vec3> v;
    QVector<std::array<int, 3>> t;
    const int n = int(poly.size());
    QPointF mid;
    for (const QPointF &p : poly)
        mid += p / n;
    v << Vec3{mid.x(), mid.y(), z0} << Vec3{mid.x(), mid.y(), z0 + h};
    for (const QPointF &p : poly)
        v << Vec3{p.x(), p.y(), z0} << Vec3{p.x(), p.y(), z0 + h};
    for (int i = 0; i < n; ++i) {
        const int a = 2 + 2 * i, b = 2 + 2 * ((i + 1) % n);
        quad(&t, a, b, b + 1, a + 1);
        t << std::array<int, 3>{0, b, a} << std::array<int, 3>{1, a + 1, b + 1};
    }
    return single(name, v, t);
}

QByteArray stlAscii(const Mesh &m)
{
    QByteArray out;
    for (const MeshBody &b : m.bodies) {
        // Материала в STL нет — CAD пишет его в имя тела («Вставка (PTFE)»), так делаем и
        // мы: по имени чтение его и узнаёт.
        const QByteArray name = (b.material.isEmpty() || b.material == b.name
                                     ? b.name
                                     : QStringLiteral("%1 (%2)").arg(b.name, b.material))
                                    .toUtf8();
        out += "solid " + name + "\n";
        for (int i = b.firstTri; i < b.firstTri + b.triCount; ++i) {
            out += "  facet normal 0 0 0\n    outer loop\n";
            for (int k = 0; k < 3; ++k) {
                const Vec3 &p = m.vertices.at(m.tris.at(i)[k]);
                out += QByteArray("      vertex ") + QByteArray::number(p.x, 'g', 12) + ' '
                       + QByteArray::number(p.y, 'g', 12) + ' ' + QByteArray::number(p.z, 'g', 12)
                       + '\n';
            }
            out += "    endloop\n  endfacet\n";
        }
        out += "endsolid " + name + "\n";
    }
    return out;
}

QByteArray stlBinary(const Mesh &m)
{
    QByteArray out(80, ' ');
    out.replace(0, 22, "TPL Designer test mesh");
    char n[4];
    qToLittleEndian<quint32>(quint32(m.tris.size()), n);
    out.append(n, 4);
    for (const auto &tri : m.tris) {
        char rec[50] = {};
        for (int k = 0; k < 3; ++k) {
            const Vec3 &p = m.vertices.at(tri[k]);
            const float f[3] = {float(p.x), float(p.y), float(p.z)};
            for (int c = 0; c < 3; ++c) {
                quint32 bits;
                std::memcpy(&bits, &f[c], 4);
                qToLittleEndian<quint32>(bits, rec + 12 + (k * 3 + c) * 4);
            }
        }
        out.append(rec, 50);
    }
    return out;
}

QByteArray obj(const Mesh &m, const QString &mtlName)
{
    QByteArray out = "# TPL Designer test mesh\n";
    if (!mtlName.isEmpty())
        out += "mtllib " + mtlName.toUtf8() + "\n";
    for (const Vec3 &p : m.vertices)
        out += "v " + QByteArray::number(p.x, 'g', 12) + ' ' + QByteArray::number(p.y, 'g', 12) + ' '
               + QByteArray::number(p.z, 'g', 12) + '\n';
    for (const MeshBody &b : m.bodies) {
        out += "o " + b.name.toUtf8() + "\n";
        if (!b.material.isEmpty())
            out += "usemtl " + b.material.toUtf8() + "\n";
        for (int i = b.firstTri; i < b.firstTri + b.triCount; ++i) {
            const auto &t = m.tris.at(i);
            out += "f " + QByteArray::number(t[0] + 1) + ' ' + QByteArray::number(t[1] + 1) + ' '
                   + QByteArray::number(t[2] + 1) + '\n';
        }
    }
    return out;
}

QByteArray mtl(const Mesh &m)
{
    QByteArray out;
    for (const MeshBody &b : m.bodies) {
        if (b.material.isEmpty())
            continue;
        out += "newmtl " + b.material.toUtf8() + "\n";
        const auto ch = [&](int s) { return QByteArray::number(((b.rgb >> s) & 0xFF) / 255.0, 'f', 3); };
        out += "Kd " + ch(16) + ' ' + ch(8) + ' ' + ch(0) + "\n\n";
    }
    return out;
}

} // namespace tpl::meshgen
