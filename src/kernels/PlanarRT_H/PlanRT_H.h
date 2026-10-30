// PlanRT_H.h : main header file for the PLANRT_H application
//

/// @file PlanRT_H.h
/// @brief Объявление класса приложения MFC (`CPlanRT_HApp`) и подключение номеров ресурсов.
///
/// Часть модуля «Оболочка счётного ядра». В Qt-версии файл подключается ради
/// `resource.h`; класс приложения объявлен, но не определён: его реализация
/// (`planrt_h.cpp`) в сборку не входит, роль `InitInstance`/`ExitInstance` выполняет
/// `src/kernels/qt_common/main.cpp`.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#startup
#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HApp:
// See PlanRT_H.cpp for the implementation of this class
//

/// @brief Приложение MFC-версии ядра; в Qt-версии не определено и не создаётся (см. `main.cpp`).
class CPlanRT_HApp : public CWinApp
{
public:
	/// @brief Создаёт приложение (только Windows-версия).
	CPlanRT_HApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlanRT_HApp)
	public:
	/// @brief Запуск MFC-версии: окна, разбор командной строки, открытие задания.
	/// @return TRUE — программа продолжает работу.
	virtual BOOL InitInstance();
	/// @brief Завершение MFC-версии; возвращает код возврата расчёта.
	/// @return Код возврата процесса 0/1/2/3.
	virtual int ExitInstance();		// RUN STATUS: process exit code
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPlanRT_HApp)
	/// @brief Help -> About (MFC-версия).
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
