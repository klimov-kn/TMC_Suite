// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

/// @file MainFrm.h
/// @brief Рамка главного окна (`CMainFrame`) — объявление из Windows-версии, общее с Qt-оболочкой.
///
/// Часть модуля «Оболочка счётного ядра». В Qt-версии заголовок используется как
/// есть, а определена только часть методов — в `src/kernels/qt_common/mainframe.cpp`
/// (конструктор, деструктор, `PreCreateWindow`, `OnCreate`, `OnDestroy`). Коду ядра
/// от рамки нужна строка состояния `m_wndStatusBar` (`PutTrace`, `PutStatistics`) и
/// указатель на рамку для закрытия программы после пакетного расчёта.
/// В ядре X код файла совпадает с одноимённым файлом ядра H (`src/kernels/PlanarRT_H`).
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#x-differences
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#status-bar
#ifndef __RTHMAINFRAME___
#define __RTHMAINFRAME___

/// @brief Рамка главного окна: строка состояния для кода ядра.
///
/// В Qt-версии объект создаёт `MainWindow` и связывает его строку состояния со
/// своей (`m_wndStatusBar.SetHost`); окна рамка не создаёт. Разрушается главным окном.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#status-bar
class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	/// @brief Создаёт рамку (без окон).
	CMainFrame();
	/// Строка состояния: панель 0 — ход расчёта (`PutTrace`), 1 — статистика такта
	/// (`PutStatistics`), 2 — что выводится (`PutSinchronizFlag`). Пишется и из расчётного потока.
	CStatusBar  m_wndStatusBar;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	/// @brief Задаёт стиль окна, как в Windows-версии (рамка, системное меню, развёрнуто).
	/// @param[in,out] cs параметры создания окна.
	/// @return Ответ базового класса.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	/// @brief Разрушает рамку.
	virtual ~CMainFrame();
#ifdef _DEBUG
	/// @brief Проверка целостности (отладочная сборка MFC; в Qt-версии не определена).
	virtual void AssertValid() const;
	/// @brief Отладочный вывод (отладочная сборка MFC; в Qt-версии не определён).
	/// @param[in,out] dc контекст отладочного вывода.
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	/// Панель инструментов Windows-версии; в Qt-версии не используется (панель строит `MainWindow`).
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	/// @brief Создание окна: запоминает рамку (`SetMainFramePointer__`) для закрытия после пакетного расчёта.
	/// @param[in] lpCreateStruct параметры создания (не используются).
	/// @return 0 — успех.
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	/// @brief Показ строки состояния (Windows-версия; в Qt-версии не определён — пункт ведёт главное окно).
	afx_msg void OnViewStatusBar();
	/// @brief Отметка пункта строки состояния (Windows-версия; в Qt-версии не определён).
	/// @param[in,out] pCmdUI состояние пункта меню.
	afx_msg void OnUpdateViewStatusBar(CCmdUI* pCmdUI);
	/// @brief Запрос завершения сеанса Windows (Windows-версия; в Qt-версии не определён).
	/// @return TRUE — сеанс можно завершить.
	afx_msg BOOL OnQueryEndSession();
	/// @brief Разрушение окна: сохраняет положение окна в настройках и снимает указатель на рамку.
	afx_msg void OnDestroy();
	/// @brief Закрытие окна (Windows-версия; в Qt-версии не определён — закрытие ведёт `MainWindow::closeEvent`).
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

/////////////////////////////////////////////////////////////////////////////
