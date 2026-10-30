// TMCGROUT.h : main header file for the TMCGROUT application
//

/// @file TMCGROUT.h
/// @brief Класс приложения MFC-версии TMCGROUT и подключение номеров ресурсов.
///
/// Часть модуля «Вьювер TMCGROUT». В Qt6-версии класс не используется (его реализация
/// `tmcgrout.cpp` в сборку не входит, приложение — `CWinApp` слоя совместимости,
/// точка входа — `src/viewers/Tmcgrout/qt/main.cpp`); заголовок включается ради `resource.h`.
/// @see https://www.tamic.ru/docs/code/tmcgrout.html#files
#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTApp:
// See TMCGROUT.cpp for the implementation of this class
//

/// @brief Приложение MFC-версии TMCGROUT. В Qt6-версии не создаётся.
class CTMCGROUTApp : public CWinApp
{
public:
	/// @brief Создаёт приложение.
	CTMCGROUTApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTMCGROUTApp)
	public:
	/// @brief Запуск MFC-приложения: шаблон документа, главное окно, разбор командной строки.
	/// @return TRUE — приложение запущено.
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTMCGROUTApp)
	/// @brief Команда Help > About.
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
