/// @file STDAFX.H
/// @brief Общие системные заголовки библиотеки TMCLibError: `<afx.h>`, `<afxwin.h>`.
///
/// В Qt6-версии эти имена разрешаются слоем совместимости `compat/` (MFC поверх Qt),
/// который даёт `CString`, `BOOL` и др.
/// @see https://www.tamic.ru/docs/code/lib-tmcliberror.html
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

/// Исключает редко используемые части заголовков Windows (ускоряет компиляцию).
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// TODO: reference additional headers your program requires here

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components

