/// @file meshio.cpp
/// @brief Реализация чтения STL и OBJ (+MTL) и выбора читателя по расширению.
///
/// Описание — в meshio.h.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#mesh-import
#include "meshio.h"
#include "lang.h"

#include "stepio.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QHash>
#include <QRegularExpression>
#include <QtEndian>

#include <cmath>
#include <cstring>

namespace tpl {

namespace {

/// @brief Проверяет, что все три координаты конечны.
/// @param[in] v  точка.
/// @return true — нет inf и NaN.
bool finite(const Vec3 &v) { return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z); }

/// Двоичный STL: 80 байт заголовка, число треугольников, по 50 байт на треугольник.
/// Признак надёжный — размер файла сходится с числом треугольников ровно. Слово
/// `solid` в начале не признак: его пишут в заголовок и двоичного файла многие CAD.
/// @param[in] b  содержимое файла.
/// @return true — файл двоичный STL.
bool looksBinaryStl(const QByteArray &b)
{
    if (b.size() < 84)
        return false;
    const quint32 n = qFromLittleEndian<quint32>(b.constData() + 80);
    return qint64(84) + qint64(n) * 50 == b.size();
}

/// @brief Читает двоичный STL (float, little-endian); нормали пропускаются.
/// @param[in] b     содержимое файла (уже проверено `looksBinaryStl`).
/// @param[in] name  имя единственного тела.
/// @return Результат с одним телом.
MeshReadResult readBinaryStl(const QByteArray &b, const QString &name)
{
    MeshReadResult r;
    const quint32 n = qFromLittleEndian<quint32>(b.constData() + 80);
    Mesh &m = r.mesh;
    m.vertices.reserve(int(n) * 3);
    m.tris.reserve(int(n));
    int skipped = 0;
    for (quint32 i = 0; i < n; ++i) {
        const char *p = b.constData() + 84 + qint64(i) * 50 + 12; // нормаль пропускаем
        Vec3 v[3];
        for (int k = 0; k < 3; ++k) {
            float f[3];
            for (int c = 0; c < 3; ++c) {
                const quint32 bits = qFromLittleEndian<quint32>(p + (k * 3 + c) * 4);
                std::memcpy(&f[c], &bits, 4);
            }
            v[k] = {f[0], f[1], f[2]};
        }
        if (!finite(v[0]) || !finite(v[1]) || !finite(v[2])) {
            ++skipped;
            continue;
        }
        const int base = int(m.vertices.size());
        m.vertices << v[0] << v[1] << v[2];
        m.tris << std::array<int, 3>{base, base + 1, base + 2};
    }
    MeshBody body;
    body.name = name;
    body.triCount = int(m.tris.size());
    m.bodies << body;
    if (skipped > 0)
        r.warnings << tpl::tr("пропущено %1 треугольников с нечисловыми координатами",
                              "skipped %1 triangles with non-numeric coordinates")
                          .arg(skipped);
    return r;
}

/// @brief Читает текстовый STL: каждое `solid <имя>` — отдельное тело с материалом = имя.
/// @param[in] b         содержимое файла.
/// @param[in] fallback  имя тела, если имя после `solid` пусто.
/// @return Результат; грани не из трёх вершин пропускаются.
MeshReadResult readAsciiStl(const QByteArray &b, const QString &fallback)
{
    MeshReadResult r;
    Mesh &m = r.mesh;
    MeshBody cur;
    bool inSolid = false;
    QVector<Vec3> facet;
    int skipped = 0;
    const auto closeBody = [&] {
        cur.triCount = int(m.tris.size()) - cur.firstTri;
        if (cur.triCount > 0)
            m.bodies << cur;
        inSolid = false;
    };
    const QList<QByteArray> lines = b.split('\n');
    for (const QByteArray &raw : lines) {
        const QByteArray l = raw.trimmed();
        if (l.startsWith("solid")) {
            if (inSolid)
                closeBody();
            cur = MeshBody();
            // Имя тела после `solid` — единственное, что текстовый STL знает о
            // материале. SolidWorks пишет туда имя детали, Blender — имя объекта.
            cur.name = QString::fromUtf8(l.mid(5).trimmed());
            if (cur.name.isEmpty())
                cur.name = fallback;
            cur.material = cur.name;
            cur.firstTri = int(m.tris.size());
            inSolid = true;
        } else if (l.startsWith("endsolid")) {
            closeBody();
        } else if (l.startsWith("vertex")) {
            const QList<QByteArray> f = l.simplified().split(' ');
            if (f.size() >= 4) {
                bool a = false, bb = false, c = false;
                const Vec3 v{f.at(1).toDouble(&a), f.at(2).toDouble(&bb), f.at(3).toDouble(&c)};
                if (a && bb && c && finite(v))
                    facet << v;
            }
        } else if (l.startsWith("endloop")) {
            if (facet.size() == 3) {
                if (!inSolid) { // вершины без `solid` — всё равно берём
                    cur = MeshBody();
                    cur.name = fallback;
                    cur.firstTri = int(m.tris.size());
                    inSolid = true;
                }
                const int base = int(m.vertices.size());
                m.vertices << facet.at(0) << facet.at(1) << facet.at(2);
                m.tris << std::array<int, 3>{base, base + 1, base + 2};
            } else {
                ++skipped;
            }
            facet.clear();
        }
    }
    if (inSolid)
        closeBody();
    if (skipped > 0)
        r.warnings << tpl::tr("пропущено %1 граней не из трёх вершин",
                              "skipped %1 faces that do not have three vertices").arg(skipped);
    return r;
}

/// Цвета материалов из .mtl: имя -> RGB. Только для показа.
/// @param[in] path  путь к файлу `.mtl`.
/// @return Имя материала -> цвет 0xRRGGBB по строке `Kd`; пусто, если файл не открылся.
QHash<QString, std::uint32_t> readMtl(const QString &path)
{
    QHash<QString, std::uint32_t> out;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return out;
    QString cur;
    for (const QByteArray &raw : f.readAll().split('\n')) {
        const QByteArray l = raw.trimmed();
        if (l.startsWith("newmtl")) {
            cur = QString::fromUtf8(l.mid(6).trimmed());
        } else if (l.startsWith("Kd") && !cur.isEmpty()) {
            const QList<QByteArray> c = l.simplified().split(' ');
            if (c.size() >= 4) {
                const auto ch = [](const QByteArray &v) {
                    return std::uint32_t(std::clamp(v.toDouble(), 0.0, 1.0) * 255.0 + 0.5);
                };
                out.insert(cur, (ch(c.at(1)) << 16) | (ch(c.at(2)) << 8) | ch(c.at(3)));
            }
        }
    }
    return out;
}

} // namespace

MeshReadResult readStl(const QByteArray &bytes, const QString &fallbackName)
{
    MeshReadResult r = looksBinaryStl(bytes) ? readBinaryStl(bytes, fallbackName)
                                             : readAsciiStl(bytes, fallbackName);
    if (r.ok() && r.mesh.isEmpty())
        r.error = tpl::tr("в файле нет ни одного треугольника", "the file contains no triangles");
    return r;
}

MeshReadResult readObj(const QByteArray &bytes, const QString &fallbackName, const QString &baseDir)
{
    MeshReadResult r;
    Mesh &m = r.mesh;
    QVector<Vec3> pos;
    QHash<QString, std::uint32_t> colors;
    QString object = fallbackName, material;
    MeshBody cur;
    cur.name = fallbackName;
    bool open = false;
    int skipped = 0;

    // Тело — пара «объект + материал»: в OBJ одна деталь может нести несколько
    // материалов (`usemtl` посреди `o`), и у каждого свой контур в срезе.
    const auto startBody = [&] {
        if (open) {
            cur.triCount = int(m.tris.size()) - cur.firstTri;
            if (cur.triCount > 0)
                m.bodies << cur;
        }
        cur = MeshBody();
        cur.name = material.isEmpty() || material == object
                       ? object
                       : QStringLiteral("%1 / %2").arg(object, material);
        cur.material = material.isEmpty() ? object : material;
        if (colors.contains(material)) {
            cur.hasColor = true;
            cur.rgb = colors.value(material);
        }
        cur.firstTri = int(m.tris.size());
        open = true;
    };

    // Номер вершины в OBJ: с единицы, отрицательный — от конца уже прочитанных.
    const auto index = [&](const QByteArray &tok, bool *ok) {
        const int slash = tok.indexOf('/');
        const int v = (slash < 0 ? tok : tok.left(slash)).toInt(ok);
        if (!*ok)
            return -1;
        const int i = v > 0 ? v - 1 : int(pos.size()) + v;
        *ok = i >= 0 && i < pos.size();
        return i;
    };

    QHash<int, int> used; // номер в OBJ -> номер в сетке
    for (const QByteArray &raw : bytes.split('\n')) {
        const QByteArray l = raw.trimmed();
        if (l.isEmpty() || l.startsWith('#'))
            continue;
        const QList<QByteArray> f = l.simplified().split(' ');
        const QByteArray &key = f.at(0);
        if (key == "v" && f.size() >= 4) {
            pos << Vec3{f.at(1).toDouble(), f.at(2).toDouble(), f.at(3).toDouble()};
        } else if (key == "mtllib" && f.size() >= 2 && !baseDir.isEmpty()) {
            colors.insert(readMtl(QDir(baseDir).filePath(QString::fromUtf8(l.mid(6).trimmed()))));
        } else if ((key == "o" || key == "g") && f.size() >= 2) {
            object = QString::fromUtf8(l.mid(1).trimmed());
            if (open)
                startBody();
        } else if (key == "usemtl" && f.size() >= 2) {
            material = QString::fromUtf8(l.mid(6).trimmed());
            startBody();
        } else if (key == "f" && f.size() >= 4) {
            if (!open)
                startBody();
            QVector<int> poly;
            bool good = true;
            for (int i = 1; i < f.size() && good; ++i) {
                const int oi = index(f.at(i), &good);
                if (!good)
                    break;
                auto it = used.constFind(oi);
                if (it == used.constEnd()) {
                    it = used.insert(oi, int(m.vertices.size()));
                    m.vertices << pos.at(oi);
                }
                poly << *it;
            }
            if (!good || poly.size() < 3) {
                ++skipped;
                continue;
            }
            // Многоугольник грани — веером. Грани OBJ выпуклые (так их пишут все
            // экспортёры), и веер покрывает их точно.
            for (int i = 2; i < poly.size(); ++i)
                m.tris << std::array<int, 3>{poly.at(0), poly.at(i - 1), poly.at(i)};
        }
    }
    if (open) {
        cur.triCount = int(m.tris.size()) - cur.firstTri;
        if (cur.triCount > 0)
            m.bodies << cur;
    }
    if (skipped > 0)
        r.warnings << tpl::tr("пропущено %1 граней со ссылками мимо вершин",
                              "skipped %1 faces referring to missing vertices").arg(skipped);
    if (m.isEmpty())
        r.error = tpl::tr("в файле нет ни одной грани (строк f)",
                          "the file contains no faces (f lines)");
    return r;
}

MeshReadResult readMeshFile(const QString &path)
{
    const QFileInfo fi(path);
    const QString ext = fi.suffix().toLower();
    const QString name = fi.completeBaseName();
    if (ext == QLatin1String("step") || ext == QLatin1String("stp"))
        return readStepFile(path);

    MeshReadResult r;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        r.error = tpl::tr("файл не открылся: %1",
                          "the file could not be opened: %1").arg(f.errorString());
        return r;
    }
    const QByteArray bytes = f.readAll();
    if (ext == QLatin1String("stl"))
        return readStl(bytes, name);
    if (ext == QLatin1String("obj"))
        return readObj(bytes, name, fi.absolutePath());
    r.error = tpl::tr("формат .%1 не поддерживается: нужен STL, OBJ%2",
                      "format .%1 is not supported: STL, OBJ%2 is required")
                  .arg(ext, stepSupported() ? tpl::tr(" или STEP", " or STEP") : QString());
    return r;
}

QString meshFileFilter()
{
    return stepSupported()
               ? tpl::tr("3D-модели (*.stl *.obj *.step *.stp);;STL (*.stl);;OBJ (*.obj);;"
                         "STEP (*.step *.stp);;Все файлы (*)",
                         "3D models (*.stl *.obj *.step *.stp);;STL (*.stl);;OBJ (*.obj);;STEP "
                         "(*.step *.stp);;All files (*)")
               : tpl::tr("3D-модели (*.stl *.obj);;STL (*.stl);;OBJ (*.obj);;Все файлы (*)",
                         "3D models (*.stl *.obj);;STL (*.stl);;OBJ (*.obj);;All files (*)");
}

} // namespace tpl
