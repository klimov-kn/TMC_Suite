// DialogDoc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogDoc dialog

/// @file DialogDoc.h
/// @brief Окно Characteristics (`IDD_DIALOG2`) TMC_DN: список ДН документа (20 строк) и кнопки девиаций.
///
/// Часть модуля «Вьювер TMC_DN». Заполняет и читает `CTMCGROUTView::OnEditDocument`.
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#commands

/// @brief Окно списка ДН: для каждой из 20 строк — файл `.DAT` или выражение, подпись, признак показа, признак Reduce.
///
/// Создаётся на стеке обработчиком `OnEditDocument`; поля заполняются до `DoModal` и
/// читаются после OK. Номер строки окна (1..20) — в имени поля. Пустое имя файла после
/// OK — строка удаляется из списка. Кнопки Make Distribution Deviation и Calculate
/// Directional Pattern Deviation открывают окна девиаций поверх этого окна.
class CDialogDoc : public CDialog
{
// Construction
public:
	/// Множитель единиц времени (наследие от TMCROS; в TMC_DN не используется).
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
	/// Имя файла излучателей `.DAT` (относительно папки документа) или выражение `{...}`; пусто — строки нет; до 300 символов.
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
	/// @name Признак Reduce
	///@{
	/// TRUE — учитывать секторы излучения излучателей (мода 1 строки графика = 1); FALSE — мода 1 = 0.
	BOOL	m_NormGrFlag1;
	BOOL	m_NormGrFlag2;
	BOOL	m_NormGrFlag3;
	BOOL	m_NormGrFlag4;
	BOOL	m_NormGrFlag5;
	BOOL	m_NormGrFlag6;
	BOOL	m_NormGrFlag7;
	BOOL	m_NormGrFlag8;
	BOOL	m_NormGrFlag9;
	BOOL	m_NormGrFlag10;
	BOOL	m_NormGrFlag11;
	BOOL	m_NormGrFlag12;
	BOOL	m_NormGrFlag13;
	BOOL	m_NormGrFlag14;
	BOOL	m_NormGrFlag15;
	BOOL	m_NormGrFlag16;
	BOOL	m_NormGrFlag17;
	BOOL	m_NormGrFlag18;
	BOOL	m_NormGrFlag19;
	BOOL	m_NormGrFlag20;
	///@}
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogDoc)
	public:
	protected:
	/// @brief Обмен данными полей с элементами окна (DDX), ограничение длины строк 300 символами.
	/// @param[in,out] pDX контекст обмена
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogDoc)
	/// @brief Кнопка добавления файла: взводит `m_AddCharacteristicsFlag` и закрывает окно как OK.
	afx_msg void OnAddCharacteristics();
	/// @brief Кнопка Make Distribution Deviation: окно девиаций распределения (`cDeviation`), по умолчанию амплитуда и фаза 10.
	afx_msg void OnAddMakedeviation();
	/// @brief Кнопка Calculate Directional Pattern Deviation: окно расчёта ДН с отклонениями (`CDeviatDirPat`).
	afx_msg void OnAddMakedeviation2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
