// app_info.cpp — чем PlanarRT_X отличается от PlanarRT_H.
//
// Подписи взяты из ресурса ../TMC_Suite/src/kernels/PlanarRT_X/PlanRT_H.rc:
// строковая таблица (IDR_MAINFRAME, IDR_PLANRTTYPE), меню Help и диалог
// IDD_ABOUTBOX. Весь остальной интерфейс — общий (../planarrt_common).

/// @file app_info.cpp
/// @brief Сведения программы для общей оболочки ядер: имя, подписи окна и «About», ресурсы и разметка диалогов.
///
/// Часть модуля «Оболочка счётного ядра». У программ PlanarRT_H и PlanarRT_X свои
/// копии файла, различаются только значения.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#build-hx
#include <QtGlobal>

#include "planrt_app.h"

// Разметка диалогов из dialogs/dialog_registry.cpp (порождён rc2ui).
/// @brief Регистрирует разметку диалогов программы и имена их элементов в слое совместимости (однократно).
/// Определена в `dialogs/dialog_registry.cpp`.
void TmcInstallDialogs_planarrt_x();

const TmcPlanRtAppInfo &TmcPlanRtApp()
{
    static const TmcPlanRtAppInfo info = {
        "PlanarRT_X",                                  // раздел настроек
        "Planar RT X-mode analyzer",                   // IDR_MAINFRAME
        "Planar RT X analyzer Files (*.tpl)",          // IDR_PLANRTTYPE
        "&About PlanRT_X...",                          // пункт меню Help
        "Planar RT X analyzer Version 1.0",            // IDD_ABOUTBOX
        "Copyright (C) 2005 Tamic_soft group",         // IDD_ABOUTBOX
        ":/icons/appicon.png"                          // значок окна
    };
    return info;
}

void TmcPlanRtInstallResources()
{
    // Ресурсы собраны в статическую библиотеку. Компоновщик выбрасывает
    // инициализатор, если на него никто не ссылается, поэтому зовём его явно —
    // иначе панель осталась бы без картинок.
    Q_INIT_RESOURCE(planarrt_x);
    TmcInstallDialogs_planarrt_x();
}
