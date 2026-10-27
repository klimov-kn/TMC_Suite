#pragma once

// Простые тела для тестов и тестовых моделей (make_models): срез каждого известен
// заранее, поэтому по ним проверяется вся цепочка «файл -> срез -> фигуры».

/// @file meshgen.h
/// @brief Генератор простых тел (параллелепипед, цилиндр, труба, тор, призма) и запись их в
///        STL/OBJ/MTL — для тестов и тестовых моделей импорта 3D.
///
/// Часть ядра TPL Designer, но программой при работе не используется: нужна тестам и
/// утилите make_models. Все размеры — в единицах, которые затем считаются мм.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#mesh-import

#include "mesh3d.h"

#include <QByteArray>
#include <QPointF>

/// Генератор тестовых сеток и их запись в файлы.
namespace tpl::meshgen {

/// Прямоугольный параллелепипед [x0,x1] x [y0,y1] x [z0,z1].
/// @param[in] name  имя тела (оно же материал).
/// @param[in] lo    нижний угол.
/// @param[in] hi    верхний угол.
/// @return Сетка из одного тела: 8 вершин, 12 треугольников.
Mesh box(const QString &name, Vec3 lo, Vec3 hi);

/// Цилиндр вдоль Z: центр основания \p c, радиус, высота, сегментов по окружности.
/// @param[in] name  имя тела.
/// @param[in] c     центр нижнего основания.
/// @param[in] r     радиус.
/// @param[in] h     высота.
/// @param[in] seg   число сегментов окружности.
/// @return Сетка из одного замкнутого тела.
Mesh cylinder(const QString &name, Vec3 c, double r, double h, int seg = 64);

/// Труба (полый цилиндр) вдоль Z.
/// @param[in] name  имя тела.
/// @param[in] c     центр нижнего основания.
/// @param[in] rIn   внутренний радиус.
/// @param[in] rOut  наружный радиус.
/// @param[in] h     высота.
/// @param[in] seg   число сегментов окружности.
/// @return Сетка из одного замкнутого тела.
Mesh tube(const QString &name, Vec3 c, double rIn, double rOut, double h, int seg = 64);

/// Тор вокруг оси Z: большой радиус R, малый r.
/// @param[in] name  имя тела.
/// @param[in] c     центр тора.
/// @param[in] R     большой радиус.
/// @param[in] r     малый радиус.
/// @param[in] segR  сегментов по большой окружности.
/// @param[in] segr  сегментов по малой окружности.
/// @return Сетка из одного замкнутого тела.
Mesh torus(const QString &name, Vec3 c, double R, double r, int segR = 96, int segr = 32);

/// Призма вдоль Z по многоугольнику в плоскости XY (основание на z0, высота h).
/// Многоугольник может быть невыпуклым — грани основания ставятся веером от центра
/// масс, поэтому для невыпуклого берите звёздный относительно центра.
/// @param[in] name  имя тела.
/// @param[in] poly  многоугольник основания в плоскости XY.
/// @param[in] z0    уровень основания.
/// @param[in] h     высота.
/// @return Сетка из одного замкнутого тела.
Mesh prism(const QString &name, const QVector<QPointF> &poly, double z0, double h);

/// Все тела одним STL: текстовый файл с `solid <имя>` на тело.
/// @param[in] m  сетка.
/// @return Текст STL; материал, отличный от имени, дописывается к имени в скобках.
QByteArray stlAscii(const Mesh &m);
/// Двоичный STL: тела склеены в одно (формат имён не хранит).
/// @param[in] m  сетка.
/// @return Байты двоичного STL (нормали нулевые).
QByteArray stlBinary(const Mesh &m);
/// OBJ: `o <имя>` и `usemtl <материал>` на тело; \p mtlName — имя файла .mtl (пусто — без него).
/// @param[in] m        сетка.
/// @param[in] mtlName  имя файла `.mtl` для строки `mtllib`; пусто — строки нет.
/// @return Текст OBJ.
QByteArray obj(const Mesh &m, const QString &mtlName = QString());
/// MTL к нему: цвета тел.
/// @param[in] m  сетка.
/// @return Текст MTL: `newmtl` и `Kd` для каждого тела с материалом.
QByteArray mtl(const Mesh &m);

} // namespace tpl::meshgen
