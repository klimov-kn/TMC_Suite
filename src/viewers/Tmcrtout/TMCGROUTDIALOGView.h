// TMCGROUTDIALOGView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTDIALOGView dialog

/// @file TMCGROUTDIALOGView.h
/// @brief Окно Viewport (`IDD_DIALOG1`): диапазоны и единицы осей, величина по Y, формат подписей, маркеры точек.
///
/// Часть модуля «Вьювер TMCROS». Копия окна TMCGROUT без единиц пользователя оси X; единицы X —
/// время. Заполняет и читает `CTMCGROUTView::OnEditGraphicsparameters`.
/// @see https://www.tamic.ru/docs/code/tmcros.html#dialogs

/// @brief Окно параметров графика. Кнопки шрифта и цветов закрывают окно как OK, взводя свой флажок.
///
/// Номер переключателя единиц X (`m_XUnit`) равен `TMC_GROTS_TYPE_*` оси X (0 номер такта,
/// 1 пс, 2 нс, 3 мкс, 4 мс, 5 с), переключателя величины Y (`m_YUnit`) — `TMC_GROTS_TYPE_*`
/// величины (0 сигнал, 1 огибающая, 2 фаза рад, 3 фаза град, 4 КСВ, 5 L дБ).
class CTMCGROUTDIALOGView : public CDialog
{
// Construction
public:
	/// Признак первого переключателя единиц X (наследие TMCGROUT; на результат не влияет — используется `m_XUnit`).
	BOOL m_XTypeGHz1;
	/// @brief Создаёт окно с пустыми полями.
	/// @param[in] pParent родительское окно; NULL — главное окно
	CTMCGROUTDIALOGView(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTMCGROUTDIALOGView)
	/// Номер разметки окна в ресурсах.
	enum { IDD = IDD_DIALOG1 };
	/// TRUE — окно закрыто кнопкой стилей линий (Config > Color > Graphics).
	BOOL	m_ColorGraphics;
	/// TRUE — окно закрыто кнопкой цвета фона.
	BOOL	m_ColorBackground;
	/// TRUE — окно закрыто кнопкой цвета осей.
	BOOL	m_ColorAxies;
	/// TRUE — окно закрыто кнопкой цвета сетки.
	BOOL	m_ColorGrid;
	/// TRUE — окно закрыто кнопкой цвета точек.
	BOOL	m_Color;
	/// TRUE — окно закрыто кнопкой шрифта.
	BOOL	m_Font;
	/// TRUE — автодиапазон оси X (поля Xmin/Xmax недоступны).
	BOOL	m_aXSFlag;
	/// TRUE — автодиапазон оси Y (поля Ymin/Ymax недоступны).
	BOOL	m_aYSFlag;
	/// Путь файла документа (только для показа).
	CString	m_szDatFileName;
	/// Правая граница оси X, единицы оси.
	double	m_XMax;
	/// Левая граница оси X, единицы оси.
	double	m_XMin;
	/// Верхняя граница оси Y, единицы оси.
	double	m_YMax;
	/// Нижняя граница оси Y, единицы оси.
	double	m_YMin;
	/// @name Признаки единиц X (наследие)
	///@{
	/// Признаки переключателей единиц X (наследие TMCGROUT с именами Гц, кГц, МГц; на результат не влияют).
	BOOL	m_XTypeHz1;
	BOOL	m_XTypeKHz1;
	BOOL	m_XTypeMHz1;
	///@}
	/// TRUE — рисовать маркеры точек.
	BOOL	m_DrawPoinFlag;
	/// Единицы оси X, номер переключателя = `TMC_GROTS_TYPE_*` оси X (0 номер такта, 1 пс, 2 нс, 3 мкс, 4 мс, 5 с).
	int		m_XUnit;
	/// Величина по оси Y, номер переключателя = `TMC_GROTS_TYPE_*` (0 сигнал, 1 огибающая, 2 фаза рад, 3 фаза град, 4 КСВ, 5 L дБ).
	int		m_YUnit;
	/// Формат `printf` подписей сетки X (одно преобразование `e/f/g`), до 300 символов.
	CString	m_csXFormat;
	/// Формат `printf` подписей сетки Y, до 300 символов.
	CString	m_csYFormat;
	/// Масштаб маркеров точек, 0..100.
	double	m_dPointSize;
	/// TRUE — прореживать маркеры (около 20 на график).
	BOOL	m_LossPoint;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTMCGROUTDIALOGView)
	protected:
	/// @brief Обмен полей с элементами окна (DDX); проверка размера маркеров 0..100 и длины строк.
	/// @param[in,out] pDX контекст обмена
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTMCGROUTDIALOGView)
	/// @name Переключатели единиц X
	///@{
	/// @brief Переключатели единиц оси X: взводят один из признаков-наследия `m_XType*1`; выбор единиц передаёт `m_XUnit`.
	afx_msg void OnGrparXinghz();
	afx_msg void OnGrparXinhz();
	afx_msg void OnGrparXinkhz();
	afx_msg void OnGrparXinmhz();
	///@}
	/// @brief Кнопка шрифта: `m_Font` = TRUE, закрыть как OK.
	afx_msg void OnOk2();
	/// @brief Кнопка цвета точек: `m_Color` = TRUE, закрыть как OK.
	afx_msg void OnOk3();
	/// @brief Кнопка цвета сетки: `m_ColorGrid` = TRUE, закрыть как OK.
	afx_msg void OnOk4();
	/// @brief Кнопка цвета осей: `m_ColorAxies` = TRUE, закрыть как OK.
	afx_msg void OnOk5();
	/// @brief Кнопка цвета фона: `m_ColorBackground` = TRUE, закрыть как OK.
	afx_msg void OnOk6();
	/// @brief Кнопка формата по умолчанию: оба формата подписей = `%4.3lg`.
	afx_msg void OnFormatdefaultbutton();
	/// @brief Кнопка стилей линий: `m_ColorGraphics` = TRUE, закрыть как OK.
	afx_msg void OnOk7();
	/// @brief Устанавливает доступность полей диапазонов и маркеров по флажкам.
	/// @return TRUE — фокус ставит каркас.
	virtual BOOL OnInitDialog();
	/// @brief Флажок автодиапазона X: поля Xmin/Xmax доступны только без автодиапазона.
	afx_msg void OnGrparAxsflag();
	/// @brief Флажок автодиапазона Y: поля Ymin/Ymax доступны только без автодиапазона.
	afx_msg void OnGrparAysflag();
	/// @brief Флажок рисования точек: прореживание и размер маркеров доступны только при включённых точках.
	afx_msg void OnGrparPointflag();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
