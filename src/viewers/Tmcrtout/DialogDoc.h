// DialogDoc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogDoc dialog

/// @file DialogDoc.h
/// @brief Окно Characteristics (`IDD_DIALOG2`): список графиков документа TMCROS, 20 строк.
///
/// Часть модуля «Вьювер TMCROS». Копия окна TMCGROUT без полей входа 2 и моды 2.
/// Заполняет и читает `CTMCGROUTView::OnEditDocument`.
/// @see https://www.tamic.ru/docs/code/tmcros.html#dialogs

/// @brief Окно списка графиков: для каждой из 20 строк — файл `.t` или выражение, подпись, признак показа, вход и волна.
///
/// Создаётся на стеке обработчиком `OnEditDocument`; поля заполняются до `DoModal` и
/// читаются после OK. Номер строки окна (1..20) — в имени поля. Пустое имя файла после
/// OK — строка удаляется из списка.
class CDialogDoc : public CDialog
{
// Construction
public:
	/// Не используется (объявлено, нигде не задаётся и не читается).
	double dTUnit;
	/// TRUE — окно закрыто кнопкой добавления файла; вид затем открывает выбор файла.
	BOOL m_AddCharacteristicsFlag;
	/// @brief Создаёт окно с пустыми полями.
	/// @param[in] pParent родительское окно; NULL — главное окно
	CDialogDoc(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDialogDoc)
	/// Номер разметки окна в ресурсах.
	enum { IDD = IDD_DIALOG2 };
	/// Путь файла документа (только для показа).
	CString	m_DocFileName;
	/// @name Подписи графиков
	///@{
	/// Подпись графика в легенде (строка окна с номером из имени поля), до 300 символов.
	CString	m_GraphName1;
	CString	m_GraphName10;
	CString	m_GraphName11;
	CString	m_GraphName12;
	CString	m_GraphName13;
	CString	m_GraphName14;
	CString	m_GraphName15;
	CString	m_GraphName16;
	CString	m_GraphName17;
	CString	m_GraphName18;
	CString	m_GraphName19;
	CString	m_GraphName2;
	CString	m_GraphName20;
	CString	m_GraphName3;
	CString	m_GraphName4;
	CString	m_GraphName5;
	CString	m_GraphName6;
	CString	m_GraphName7;
	CString	m_GraphName8;
	CString	m_GraphName9;
	///@}
	/// @name Номер входа
	///@{
	/// Номер входа (порта) в файле `.t`, с 1 (строка окна с номером из имени поля).
	int		m_np1_10;
	int		m_np1_11;
	int		m_np1_12;
	int		m_np1_13;
	int		m_np1_14;
	int		m_np1_15;
	int		m_np1_16;
	int		m_np1_17;
	int		m_np1_18;
	int		m_np1_19;
	int		m_np1_2;
	int		m_np1_20;
	int		m_np1_1;
	int		m_np1_3;
	int		m_np1_4;
	int		m_np1_5;
	int		m_np1_6;
	int		m_np1_7;
	int		m_np1_8;
	int		m_np1_9;
	///@}
	/// @name Волна
	///@{
	/// Волна: 0 — падающая `Up`, 1 — уходящая `Uo` (строка окна с номером из имени поля).
	int		m_mod1_1;
	int		m_mod1_2;
	int		m_mod1_3;
	int		m_mod1_4;
	int		m_mod1_5;
	int		m_mod1_6;
	int		m_mod1_7;
	int		m_mod1_8;
	int		m_mod1_9;
	int		m_mod1_10;
	int		m_mod1_11;
	int		m_mod1_12;
	int		m_mod1_13;
	int		m_mod1_14;
	int		m_mod1_15;
	int		m_mod1_16;
	int		m_mod1_17;
	int		m_mod1_18;
	int		m_mod1_19;
	int		m_mod1_20;
	///@}
	/// @name Признак показа
	///@{
	/// TRUE — график рисуется (строка окна с номером из имени поля).
	BOOL	m_OutGrFlag_1;
	BOOL	m_OutGrFlag_10;
	BOOL	m_OutGrFlag_11;
	BOOL	m_OutGrFlag_12;
	BOOL	m_OutGrFlag_13;
	BOOL	m_OutGrFlag_15;
	BOOL	m_OutGrFlag_14;
	BOOL	m_OutGrFlag_16;
	BOOL	m_OutGrFlag_17;
	BOOL	m_OutGrFlag_18;
	BOOL	m_OutGrFlag_19;
	BOOL	m_OutGrFlag_2;
	BOOL	m_OutGrFlag_20;
	BOOL	m_OutGrFlag_3;
	BOOL	m_OutGrFlag_4;
	BOOL	m_OutGrFlag_5;
	BOOL	m_OutGrFlag_6;
	BOOL	m_OutGrFlag_7;
	BOOL	m_OutGrFlag_8;
	BOOL	m_OutGrFlag_9;
	///@}
	/// @name Файл или выражение
	///@{
	/// Имя файла `.t` (относительно папки документа) или выражение `{...}`; пусто — строки нет; до 300 символов.
	CString	m_SFileName_1;
	CString	m_SFileName_10;
	CString	m_SFileName_11;
	CString	m_SFileName_12;
	CString	m_SFileName_13;
	CString	m_SFileName_14;
	CString	m_SFileName_15;
	CString	m_SFileName_16;
	CString	m_SFileName_17;
	CString	m_SFileName_18;
	CString	m_SFileName_19;
	CString	m_SFileName_2;
	CString	m_SFileName_20;
	CString	m_SFileName_3;
	CString	m_SFileName_4;
	CString	m_SFileName_5;
	CString	m_SFileName_6;
	CString	m_SFileName_7;
	CString	m_SFileName_8;
	CString	m_SFileName_9;
	///@}
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogDoc)
	public:
	/// @brief Показывает окно (вызов `CDialog::DoModal`).
	/// @return `IDOK` или `IDCANCEL`.
	virtual INT_PTR DoModal();
	protected:
	/// @brief Обмен данными полей с элементами окна (DDX), ограничение длины строк 300 символами.
	/// @param[in,out] pDX контекст обмена
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	/// @brief Уведомления элементов окна; передаёт их базовому классу без изменений.
	/// @param[in]  wParam  номер элемента
	/// @param[in]  lParam  указатель на структуру уведомления
	/// @param[out] pResult результат обработки
	/// @return Результат `CDialog::OnNotify`.
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogDoc)
	/// @brief Кнопка добавления файла: взводит `m_AddCharacteristicsFlag` и закрывает окно как OK.
	afx_msg void OnAddCharacteristics();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
