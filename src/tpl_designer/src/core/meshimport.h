#pragma once

// Срез 3D-модели -> фигуры задания.
//
// Вставляются ОБЫЧНЫЕ фигуры — те же, что рисует человек: эмиттер, чтение `.tpl`,
// поворот и проверка про импорт не знают ничего. Связи с моделью после вставки нет.

/// @file meshimport.h
/// @brief Превращение среза 3D-модели в обычные фигуры задания: размещение на холсте,
///        контуры стенок, выпуклые куски диэлектрика.
///
/// Часть импорта 3D в ядре TPL Designer; вызывается окном импорта модели.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#mesh-import

#include "meshslice.h"
#include "project.h"

#include <QStringList>

namespace tpl {

/// Что делать с телом модели.
/// @brief Выбор человека для тела модели: вставлять ли и каким материалом.
struct BodyChoice {
    /// true — тело вставляется.
    bool include = true;
    /// Материал фигур тела.
    Material material = Material::Metal;
    double eps = 1.0; ///< для диэлектрика — сама eps, как везде в программе
};

/// Как срез ложится на холст: поворот, зеркало, сдвиг.
/// @brief Преобразование координат среза (u, v) в координаты сцены.
struct ImportPlacement {
    int rot90 = 0;       ///< поворот на 90 градусов * rot90 против часовой
    bool mirror = false; ///< отразить по горизонтали (до поворота)
    QPointF shift;       ///< сдвиг после поворота, мм
};

/// @brief Переводит точку среза на холст: зеркало по горизонтали, поворот на k*90 градусов, сдвиг.
/// @param[in] uv  точка в координатах плоскости среза, мм.
/// @param[in] pl  размещение.
/// @return Точка в координатах сцены, мм.
QPointF placePoint(const QPointF &uv, const ImportPlacement &pl);

/// Сдвиг, который ставит центр габарита среза в центр счётной области.
/// @param[in] r    результат среза.
/// @param[in] pl   размещение (его сдвиг не учитывается).
/// @param[in] box  счётная область.
/// @return Сдвиг, мм; нулевой, если в срезе нет точек.
QPointF centeringShift(const SliceResult &r, ImportPlacement pl, const Box &box);

/// @brief Итог сборки фигур из среза.
struct ImportShapes {
    /// Фигуры для вставки в проект.
    QVector<Shape> shapes;
    QStringList notes; ///< что сделано не один в один: незамкнутые контуры, разбиение
    /// Сколько выпуклых кусков диэлектрика получилось всего.
    int dielectricPieces = 0;
};

/// Собрать фигуры. \p choices — по телу на каждый элемент `mesh.bodies`. Номера
/// фигур — подряд с \p firstId. Срез уже упрощён (`simplifySlice`).
///
/// * Металл, магнитная стенка, поглотитель — контуром: замкнутый срез — замкнутый
///   контур, незамкнутый — ломаная. Ядро рисует их по периметру, выпуклость не нужна.
/// * Диэлектрик — заливкой, а заливку ядро умеет только выпуклую: область режется на
///   выпуклые куски (`convexPieces`). Незамкнутый контур залить нельзя — о нём
///   говорится в notes, и он не вставляется.
/// @param[in] mesh     сетка (имена тел).
/// @param[in] slice    упрощённый срез.
/// @param[in] choices  выбор по каждому телу (индекс = номер тела).
/// @param[in] pl       размещение на холсте.
/// @param[in] firstId  номер первой новой фигуры.
/// @return Фигуры и пояснения; имена тел очищены от символов вне CP1251.
ImportShapes buildImportShapes(const Mesh &mesh, const SliceResult &slice,
                               const QVector<BodyChoice> &choices, const ImportPlacement &pl,
                               int firstId);

/// Строка для описания задания: откуда взята геометрия.
/// @param[in] fileName  имя файла модели.
/// @param[in] plane     плоскость среза.
/// @param[in] offset    координата плоскости по нормали, мм.
/// @return Строка на действующем языке, безопасная для CP1251.
QString sliceProvenance(const QString &fileName, SlicePlane plane, double offset);

} // namespace tpl
