// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

/// @file MainFrm.h
/// @brief Рамка главного окна MFC-версии TMCGROUT; в Qt6-версии — носитель строки состояния для кода вьювера.
///
/// Часть модуля «Вьювер TMCGROUT». Qt-оболочка создаёт объект и делает его `m_pMainWnd`,
/// чтобы `PutTrace`/`PutStatistics` находили строку состояния.
/// @see https://www.tamic.ru/docs/code/tmcgrout.html#qt-shell

/// @brief Рамка окна с панелью инструментов и строкой состояния из двух ячеек.
///
/// В Qt6-версии единственный объект создаёт `MainWindow` (`m_frame`); окно рамки не
/// показывается, используется только `m_wndStatusBar`, текст которой уходит в виджет
/// активного документа.
class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	/// @brief Создаёт рамку; действий нет.
	CMainFrame();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	/// @brief Задаёт стиль окна рамки (развёрнутое окно с заголовком и рамкой).
	/// @param[in,out] cs параметры создания окна
	/// @return Результат базового класса.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	/// Строка состояния: ячейка 0 — координаты под мышью, ячейка 1 — величина по оси Y.
	CStatusBar  m_wndStatusBar;
	/// @brief Уничтожает рамку.
	virtual ~CMainFrame();
#ifdef _DEBUG
	/// @brief Проверка объекта (отладочная сборка MFC).
	virtual void AssertValid() const;
	/// @brief Отладочный вывод (отладочная сборка MFC).
	/// @param[in,out] dc контекст вывода
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	/// Панель инструментов MFC-каркаса (в Qt6-версии панель строит `MainWindow`).
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	/// @brief Создание окна в MFC-каркасе: панель инструментов и строка состояния.
	/// @param[in] lpCreateStruct параметры создания
	/// @return 0 — успех; -1 — ошибка.
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	/// @brief При закрытии нормального (не развёрнутого) окна записывает его положение в настройки (`#INIWND`).
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
