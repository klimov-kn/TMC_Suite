#pragma once

// Чтение 3D-моделей: STL (текстовый и двоичный), OBJ (+MTL), STEP (через OpenCASCADE).

/// @file meshio.h
/// @brief Чтение 3D-моделей в `Mesh`: STL (текстовый и двоичный), OBJ с MTL, STEP (stepio.h).
///
/// Часть импорта 3D в ядре TPL Designer.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#mesh-import

#include "mesh3d.h"

#include <QStringList>

namespace tpl {

/// @brief Результат чтения модели: сетка, ошибка, предупреждения, единица файла.
struct MeshReadResult {
    /// Прочитанная сетка (вершины не склеены).
    Mesh mesh;
    QString error;        ///< пусто — прочитано
    QStringList warnings; ///< прочитано, но не всё (пропущенные грани и т. п.)
    /// Единицы, в которых записан файл, если формат их хранит (STEP — да, STL/OBJ — нет).
    /// Миллиметров на единицу файла; 0 — неизвестно, спросить человека.
    double fileUnitMm = 0.0;
    /// @return true — ошибки нет.
    bool ok() const { return error.isEmpty(); }
};

/// Прочитать модель по расширению: .stl, .obj, .step/.stp. Вершины не склеены —
/// это делает вызывающий (`Mesh::weld`) уже после масштаба.
/// @param[in] path  путь к файлу; формат — по расширению (регистр не важен).
/// @return Результат; неизвестное расширение или неоткрывшийся файл — `error`.
MeshReadResult readMeshFile(const QString &path);

/// @brief Читает STL из памяти; двоичный формат узнаётся по размеру 84 + 50 * N байт.
/// @param[in] bytes         содержимое файла.
/// @param[in] fallbackName  имя тела, если `solid` без имени или формат двоичный.
/// @return Результат; треугольники с нечисловыми координатами пропускаются с предупреждением.
MeshReadResult readStl(const QByteArray &bytes, const QString &fallbackName);
/// @brief Читает OBJ из памяти: тело — пара «объект (`o`/`g`) + материал (`usemtl`)».
/// @param[in] bytes         содержимое файла.
/// @param[in] fallbackName  имя тела до первой строки `o`/`g`.
/// @param[in] baseDir       папка файла для поиска `.mtl` (цвета); пусто — цвета не читаются.
/// @return Результат; многоугольные грани разбиваются веером на треугольники.
MeshReadResult readObj(const QByteArray &bytes, const QString &fallbackName,
                       const QString &baseDir = QString());

/// Умеет ли эта сборка читать STEP (собрана ли с OpenCASCADE).
/// @return true — сборка с OpenCASCADE (stepio_occt.cpp), false — без неё (stepio_none.cpp).
bool stepSupported();

/// Фильтр для окна выбора файла — с STEP или без, смотря по сборке.
/// @return Строка фильтра `QFileDialog` на действующем языке.
QString meshFileFilter();

} // namespace tpl
