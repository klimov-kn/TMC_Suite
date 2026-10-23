// Значки команд FieldView.
//
// Значок ищется в ресурсах по имени команды (= ID команды Windows-версии, например
// ID_FILE_OPEN): ":/toolbar/<ID>.svg", затем ".png". Файлы берутся при сборке из
// research/gpu/qt6/resources/icons/toolbar/fieldview/ (их рисует оформитель пакета).
// Если файла нет — рисуется временный простой значок в цветах оформления.
/// @file icons.h
/// @brief Значки команд и значок программы FieldView.
///
/// Часть модуля FieldView (слой `ui`).
/// @see https://www.tamic.ru/docs/code/fieldview.html#dialogs
#pragma once

#include <QIcon>
#include <QString>

namespace fieldview {

/// @brief Возвращает значок команды по её идентификатору.
///
/// Ищет в ресурсах `:/toolbar/<id>.svg`, затем `.png`; если файла нет, рисует простой
/// значок 24 x 24 (масштаб 1; 1,5; 2) с отдельным видом для недоступной команды.
/// Значки кэшируются.
/// @param[in] id  идентификатор команды («ID_FILE_OPEN», «ID_FLAGDRAWLINE» …)
/// @return Значок.
QIcon commandIcon(const QString &id);
/// @brief Возвращает значок программы.
///
/// Значок программы (окно, About).
/// @return Значок из ресурса `:/app/fieldview.png` или нарисованный (волны над сеткой), 16 … 256 точек.
QIcon appIcon();

} // namespace fieldview
