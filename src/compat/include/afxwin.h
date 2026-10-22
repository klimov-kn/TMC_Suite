// Заглушка <afxwin.h> для сборки под Linux: MFC здесь нет.
// Даёт слой совместимости: строки и типы (tmc_mfc.h), документ и
// настройки (tmc_mfc_doc.h), рисование и окна поверх Qt (mfc_view.h),
// диалоги и карты сообщений (mfc_dialog.h), потоки (mfc_thread.h).
/// @file afxwin.h
/// @brief Заглушка главного заголовка MFC `<afxwin.h>`: подключает весь слой окон, потоков и WGL.
///
/// Часть модуля «Слой совместимости». Цепочка включений: `mfc_thread.h` ->
/// `mfc_dialog.h` -> `mfc_view.h` -> `mfc_gdi.h` / `tmc_mfc_doc.h` -> `tmc_mfc.h`;
/// @see https://www.tamic.ru/docs/code/compat.html#headers
#ifndef __AFXWIN_H__
  #define __AFXWIN_H__
#endif
#pragma once
#include "mfc_thread.h"
