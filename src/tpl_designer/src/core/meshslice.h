#pragma once

// Срез 3D-модели плоскостью: из треугольной сетки — контуры на плоскости холста.

/// @file meshslice.h
/// @brief Срез треугольной сетки плоскостью XY/YZ/XZ в контуры на холсте и их упрощение.
///
/// Часть импорта 3D в ядре TPL Designer: шаг между чтением модели (meshio.h) и
/// превращением контуров в фигуры (`buildImportShapes`, meshimport.h).
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#mesh-import

#include "mesh3d.h"

#include <QPointF>
#include <QVector>

namespace tpl {

/// Плоскость среза. Название — две оси, которые ложатся на холст.
/// @brief Плоскость среза: XY (нормаль z), YZ (нормаль x), XZ (нормаль y).
enum class SlicePlane { XY, YZ, XZ };

/// @param[in] p  плоскость.
/// @return «XY», «YZ» или «XZ».
QString slicePlaneName(SlicePlane p);

/// Какая ось модели — нормаль плоскости, какие две идут в X и Y холста
/// (0 — x, 1 — y, 2 — z). XY: (x, y), нормаль z; YZ: (y, z), нормаль x; XZ: (x, z), нормаль y.
/// @brief Номера осей модели для плоскости среза.
struct PlaneAxes {
    /// @name Оси плоскости
    ///@{
    /// Номера осей модели: u -> X холста, v -> Y холста, normal — нормаль (0 — x, 1 — y, 2 — z).
    int u = 0, v = 1, normal = 2;
    ///@}
};
/// @param[in] p  плоскость.
/// @return Номера осей u, v и нормали.
PlaneAxes planeAxes(SlicePlane p);
/// @param[in] p     точка.
/// @param[in] axis  номер оси: 0 — x, 1 — y, иначе z.
/// @return Координата точки по оси.
double axisValue(const Vec3 &p, int axis);
/// Название нормали: «Z» для XY и т. д. — подпись ползунка.
/// @param[in] p  плоскость.
/// @return Имя оси нормали «X», «Y» или «Z».
QString normalAxisName(SlicePlane p);

/// Один контур среза.
/// @brief Контур среза одного тела.
struct SliceLoop {
    QVector<QPointF> pts; ///< в координатах плоскости (u, v), мм
    bool closed = false;  ///< замкнулся ли
    /// Вложенность среди замкнутых контуров ТОГО ЖЕ тела: 0 — внешний, 1 — дырка в нём,
    /// 2 — остров в дырке… Нужна для заливки диэлектрика (чётно-нечётное правило).
    int depth = 0;
};

/// @brief Контуры среза одного тела модели.
struct SliceBody {
    int body = 0; ///< номер тела в Mesh::bodies
    /// Контуры тела (замкнутые и открытые).
    QVector<SliceLoop> loops;
};

/// @brief Итог среза сетки плоскостью.
struct SliceResult {
    QVector<SliceBody> bodies; ///< только тела, которые плоскость задела
    int openLoops = 0;         ///< сколько контуров не замкнулось (сетка «не герметична»)
};

/// Разрезать сетку плоскостью «координата нормали = offset».
///
/// Сетка должна быть склеена (`Mesh::weld`): отрезки сшиваются по общим рёбрам.
/// Вершина ровно на плоскости считается лежащей «выше» неё — так у отрезков не
/// бывает вырожденных случаев, а грань, лежащая в самой плоскости, не даёт ничего
/// (её контур дадут соседние боковые грани).
/// @param[in] mesh    склеенная сетка, мм.
/// @param[in] plane   плоскость среза.
/// @param[in] offset  координата плоскости по нормали, мм.
/// @return Контуры по телам с глубиной вложенности и числом незамкнутых контуров.
SliceResult sliceMesh(const Mesh &mesh, SlicePlane plane, double offset);

/// Упростить ломаную: убрать точки, отклоняющиеся от прямой меньше \p tol (Дуглас — Пейкер).
/// @param[in] pts     точки ломаной, мм.
/// @param[in] tol     допуск отклонения, мм; <= 0 — убираются только повторы подряд.
/// @param[in] closed  true — контур замкнут (первая точка не повторяется в конце).
/// @return Упрощённая ломаная; замкнутый контур, сжавшийся в отрезок, возвращается без упрощения.
QVector<QPointF> simplifyPolyline(const QVector<QPointF> &pts, double tol, bool closed);

/// Упростить все контуры среза (замкнутым — не меньше трёх точек).
/// @param[in,out] r    результат среза; контуры заменяются упрощёнными.
/// @param[in]     tol  допуск, мм.
void simplifySlice(SliceResult *r, double tol);

/// Точка внутри многоугольника (чётно-нечётное правило).
/// @param[in] p     точка.
/// @param[in] poly  вершины многоугольника (замыкание неявное).
/// @return true — точка внутри.
bool pointInPolygon(const QPointF &p, const QVector<QPointF> &poly);

} // namespace tpl
