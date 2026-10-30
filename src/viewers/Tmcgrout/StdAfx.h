// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

/// @file StdAfx.h
/// @brief Общий заголовок вьювера TMCGROUT: подключает заголовки MFC (в Qt6-версии — слоя совместимости).
///
/// Часть модуля «Вьювер TMCGROUT».
/// @see https://www.tamic.ru/docs/code/compat.html#headers

/// Исключает редко используемые части заголовков Windows.
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC OLE automation (нужно для AfxOleInit на совр. Windows)
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT




