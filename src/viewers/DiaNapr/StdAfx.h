// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

/// @file StdAfx.h
/// @brief Общий заголовок расчётной части ДН (`viewers/dianapr`): подключает `stdio.h`.
///
/// Часть модуля «Вьювер TMC_DN». В Qt6-версии объявления Windows и `CString` модули
/// решётки получают из слоя совместимости (заголовок `tmc_mfc_doc.h` подставляется при сборке).
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#files
#if !defined(AFX_STDAFX_H__AD46D10B_17C4_47FC_B957_B81081DB97E3__INCLUDED_)
#define AFX_STDAFX_H__AD46D10B_17C4_47FC_B957_B81081DB97E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/// Исключает редко используемые части заголовков Windows.
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__AD46D10B_17C4_47FC_B957_B81081DB97E3__INCLUDED_)
