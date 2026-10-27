#pragma once

// Имя материала из 3D-модели -> материал задания.
//
// В CAD материал называют как угодно: «Медь М1», «Copper», «PEC», «AISI 304»,
// «Rogers RO4003C», «Фторопласт-4». Здесь словарь, который узнаёт типовые имена, а то,
// что не узнал, честно возвращает как неизвестное — выбирает человек.

/// @file materialmap.h
/// @brief Словарь имён материалов из 3D-модели: имя тела -> материал задания и eps.
///
/// Часть импорта 3D в ядре TPL Designer: окно импорта подписывает каждое тело модели
/// предложением материала, человек подтверждает или правит его.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#mesh-import

#include "project.h"

#include <QString>

namespace tpl {

/// @brief Результат распознавания имени материала (предложение, а не решение).
struct MaterialGuess {
    bool known = false; ///< словарь узнал имя
    bool skip = false;  ///< воздух / вакуум: вставлять нечего, это фон
    /// Материал задания; при `known == false` остаётся `Material::Metal` по умолчанию.
    Material material = Material::Metal;
    double eps = 1.0;   ///< для диэлектрика
    QString what;       ///< что узнано: «медь», «FR-4 (eps 4.4)»
};

/// @brief Распознаёт материал по имени тела из CAD (регулярные выражения, первое совпадение).
/// @param[in] name  имя тела или материала из модели, любой регистр, русский или английский.
/// @return Предложение материала; `known == false`, если имя пустое или не узнано.
/// @note Подпись `what` — на действующем языке (`tpl::tr`).
MaterialGuess guessMaterial(const QString &name);

} // namespace tpl
