// commandupdate.h — обновление состояния команд (ON_UPDATE_COMMAND_UI).
//
// В MFC каркас перед показом меню и панели опрашивал обработчики обновления:
// они говорят, доступен ли пункт (`pCmdUI->Enable`) и нажат ли он
// (`pCmdUI->SetCheck`). Благодаря этому кнопки-переключатели показывают
// включённый режим, а неприменимые пункты сереют.
//
// Здесь то же самое делает одна функция: она проходит по действиям главного
// окна, для каждого ищет обработчик в карте сообщений вида (или документа) и
// переносит его решение в QAction. Главное окно зовёт её перед показом меню
// (сигнал `QMenu::aboutToShow`) и после каждой выполненной команды.
//
// Ни одной формулы здесь нет: только состояние элементов интерфейса.

/// @file commandupdate.h
/// @brief Перенос состояния команд из обработчиков `ON_UPDATE_COMMAND_UI` в действия Qt (`QAction`).
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_gui_common`. Только поток интерфейса.
/// @see https://www.tamic.ru/docs/code/gui-common.html#command-update
#pragma once

#include <QMap>

#include "mfc_dialog.h"   // UINT, CObject, MfcMessageMap, CCmdUI

QT_BEGIN_NAMESPACE
class QAction;
QT_END_NAMESPACE

/// @brief Обновление состояния команд меню и панелей.
namespace tmccmd {

/// Обновить состояние действий по обработчикам ON_UPDATE_COMMAND_UI цели.
///
/// \param actions  таблица «идентификатор команды -> действие» главного окна;
///                 действия без обработчика остаются как были;
/// \param target   объект с картой сообщений (вид активного окна, рамка или
///                 документ). Ноль допустим — тогда ничего не меняется.
void updateActions(const QMap<UINT, QAction *> &actions, CObject *target);

/// Обновить одно действие. Возвращает false, если обработчика для этой команды
/// у цели нет.
/// @param[in] commandId  номер команды
/// @param[in] action     действие; NULL — ничего
/// @param[in] target     объект с картой сообщений; NULL — ничего
/// @return true — обработчик найден и его решение перенесено в действие.
bool updateAction(UINT commandId, QAction *action, CObject *target);

} // namespace tmccmd
