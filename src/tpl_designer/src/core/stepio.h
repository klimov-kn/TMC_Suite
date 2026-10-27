#pragma once

// STEP — родной формат обмена CAD (SolidWorks, Компас-3D, FreeCAD, Fusion, Inventor).
//
// Читается библиотекой OpenCASCADE (LGPL). Если программа собрана без неё, есть
// заглушка (stepio_none.cpp): STL и OBJ работают, STEP честно отвечает «не поддержан».

/// @file stepio.h
/// @brief Чтение STEP через OpenCASCADE (stepio_occt.cpp) или заглушка без неё (stepio_none.cpp).
///
/// Часть импорта 3D в ядре TPL Designer. Какая реализация попадёт в сборку, решает
/// src/core/CMakeLists.txt по наличию OpenCASCADE.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#step-import

#include "meshio.h"

namespace tpl {

/// Прочитать STEP: тела сборки с именами, цветами и материалами, поверхности разбиты
/// на треугольники. Координаты — в миллиметрах (OpenCASCADE пересчитывает единицы
/// файла сам), поэтому \c fileUnitMm = 1.
///
/// \p deflection — допустимое отклонение треугольников от поверхности, мм;
/// 0 — тысячная доля габарита модели.
/// @param[in] path        путь к файлу `.step`/`.stp`.
/// @param[in] deflection  допуск триангуляции, мм; <= 0 — 1e-3 диагонали габарита (не меньше 1e-4).
/// @return Результат: тела сборки (имя, материал, цвет), `fileUnitMm = 1`;
///         в сборке без OpenCASCADE — всегда `error`.
MeshReadResult readStepFile(const QString &path, double deflection = 0.0);

/// Записать тела в STEP — нужно только для тестовых моделей (make_models).
/// Есть лишь в сборке с OpenCASCADE.
/// @return true — сборка с OpenCASCADE.
bool stepWriterAvailable();

} // namespace tpl
