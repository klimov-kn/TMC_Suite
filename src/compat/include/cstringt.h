// Заглушка <cstringt.h> для сборки под Linux: MFC/win32 здесь нет.
// Даёт минимальный слой совместимости (CString, win-типы) из tmc_mfc.h.
// GUI-классы MFC не эмулируются: интерфейс переписан на Qt, а библиотеки
// используют только строки и базовые типы.
/// @file cstringt.h
/// @brief Заглушка заголовка `<cstringt.h>`: подключает `CString` и базовые типы из `tmc_mfc.h`.
///
/// Часть модуля «Слой совместимости».
/// @see https://www.tamic.ru/docs/code/compat.html#strings
#pragma once
#include "tmc_mfc.h"
