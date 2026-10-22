// Заглушка <afxext.h> для сборки под Linux: MFC здесь нет.
// Расширения MFC (панели инструментов, рамки) объявлены в общем слое, поэтому
// заголовок просто подключает его целиком — как это делал afxext.h, тянувший
// за собой afxwin.h.
/// @file afxext.h
/// @brief Заглушка заголовка MFC `<afxext.h>` (панели, рамки): подключает весь слой окон.
///
/// Часть модуля «Слой совместимости». Классы расширений (`CToolBar`, `CStatusBar`,
/// `CFrameWnd`) объявлены в общих заголовках слоя.
/// @see https://www.tamic.ru/docs/code/compat.html#headers
#ifndef __AFXWIN_H__
  #define __AFXWIN_H__
#endif
#pragma once
#include "afxwin.h"
