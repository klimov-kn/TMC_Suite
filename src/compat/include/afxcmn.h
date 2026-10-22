// Заглушка <afxcmn.h> для сборки под Linux: MFC здесь нет.
// Общие элементы управления Windows (список с колонками CListCtrl, набор
// картинок CImageList, индикатор CProgressCtrl) объявлены в общем слое
// (mfc_dialog.h), поэтому заголовок подключает его целиком.
/// @file afxcmn.h
/// @brief Заглушка заголовка MFC `<afxcmn.h>` (общие элементы управления): подключает весь слой окон.
///
/// Часть модуля «Слой совместимости». `CListCtrl`, `CImageList`, `CProgressCtrl`
/// объявлены в `mfc_dialog.h`. Макрос `__AFXWIN_H__` определяется так же, как его
/// определял настоящий MFC, — часть исходников проверяет его наличие.
/// @see https://www.tamic.ru/docs/code/compat.html#headers
#ifndef __AFXWIN_H__
  #define __AFXWIN_H__
#endif
#pragma once
#include "afxwin.h"
