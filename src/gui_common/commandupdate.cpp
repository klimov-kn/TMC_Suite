// Обновление состояния команд: карта сообщений -> действия Qt
// (см. commandupdate.h).

/// @file commandupdate.cpp
/// @brief Реализация обновления состояния команд по карте сообщений.
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_gui_common`.
/// @see https://www.tamic.ru/docs/code/gui-common.html#command-update
#include "commandupdate.h"

#include <QAction>

namespace {

/// Перенести решение обработчика в действие.
///
/// Меняются только те свойства, которых обработчик коснулся: в MFC пункт, о
/// котором обработчик ничего не сказал, тоже остаётся прежним.
/// @param[in]     ui      состояние, записанное обработчиком
/// @param[in,out] action  действие Qt (делается отмечаемым, если обработчик задал отметку)
void applyCmdUi(const CCmdUI &ui, QAction *action)
{
    if (ui.EnableWasSet())
        action->setEnabled(ui.Enabled() != FALSE);

    if (ui.CheckWasSet()) {
        // Отметку показывает только «отмечаемое» действие. Windows-версия
        // отличает состояния 0/1 (снята/стоит); третье состояние MFC (2 —
        // «неопределённое») в TMC не встречается, поэтому считаем его нажатым.
        if (!action->isCheckable())
            action->setCheckable(true);
        action->setChecked(ui.Check() != 0);
    }
}

} // namespace

namespace tmccmd {

bool updateAction(UINT commandId, QAction *action, CObject *target)
{
    if (!action || !target)
        return false;

    MfcMessageMap map;
    target->TmcBuildMessageMap(map);

    CCmdUI ui;
    ui.m_nID = commandId;
    ui.m_pQtAction = action;
    if (!map.callUpdate(commandId, &ui))
        return false;

    applyCmdUi(ui, action);
    return true;
}

void updateActions(const QMap<UINT, QAction *> &actions, CObject *target)
{
    if (!target || actions.isEmpty())
        return;

    // Карта строится один раз на весь проход: она одинакова для всех команд.
    MfcMessageMap map;
    target->TmcBuildMessageMap(map);
    if (map.updateHandlers().empty())
        return;

    for (QMap<UINT, QAction *>::const_iterator it = actions.constBegin();
         it != actions.constEnd(); ++it) {
        QAction *action = it.value();
        if (!action)
            continue;

        CCmdUI ui;
        ui.m_nID = it.key();
        ui.m_pQtAction = action;
        if (!map.callUpdate(it.key(), &ui))
            continue;      // обработчика нет — пункт остаётся как был

        applyCmdUi(ui, action);
    }
}

} // namespace tmccmd
