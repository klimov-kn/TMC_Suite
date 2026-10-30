// TmcRTH_DialogFormatOutFile.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_DialogFormatOutFile dialog

/// @file TmcRTH_DialogFormatOutFile.h
/// @brief Окно форматов чисел в выходных файлах ядра (Config -> Format -> OutputDataFile).
///
/// Часть модуля «Оболочка счётного ядра». Форматы хранит `pl_iofor.cpp`
/// (`SetnTFormat` …), окно лишь показывает и возвращает строки. Разметка —
/// `IDD_TMC_CONFIG_OUTFFORMAT`.
/// В ядре X код файла совпадает с одноимённым файлом ядра H (`src/kernels/PlanarRT_H`).
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#x-differences
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#settings

/// @brief Окно форматов `printf` для чисел в выходных файлах.
///
/// Создаётся на стеке видом (`OnConfigFormatOutputDataFile`), поля заполняет вид.
/// Допустимый формат — одно преобразование: `d`, `i`, `u`, `x`, `o` для целого,
/// `g`, `e`, `f` для вещественного, ширина и точность не длиннее 2 цифр; проверку
/// выполняют функции `Set*Format`.
class CTmcRTH_DialogFormatOutFile : public CDialog
{
// Construction
public:
	/// TRUE — нажата кнопка «Set default» (вид восстанавливает форматы по умолчанию).
	BOOL IsSetDefaultFormat;
	/// @brief Создаёт окно; «Set default» не нажата.
	/// @param[in] pParent родительское окно; NULL — главное окно программы.
	CTmcRTH_DialogFormatOutFile(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTmcRTH_DialogFormatOutFile)
	/// Номер разметки окна в ресурсах.
	enum { IDD = IDD_TMC_CONFIG_OUTFFORMAT };
	/// Формат номера такта nT (целое; по умолчанию «%9d»).
	CString            m_csnT;
	/// Формат текущего времени (вещественное; по умолчанию «%15.7g»).
	CString            m_csdTcurrent;
	/// Формат номера блока (целое; по умолчанию «%4d»).
	CString            m_csnBlock;
	/// Формат входного сигнала порта (вещественное; по умолчанию «%15.7g»).
	CString            m_csdInp;
	/// Формат выходного сигнала порта (вещественное; по умолчанию «%15.7g»).
	CString            m_csdOut;
	/// Формат значений поля (по умолчанию пустая строка).
	CString            m_csdOutField;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTH_DialogFormatOutFile)
	protected:
	/// @brief Обмен строк форматов с полями окна.
	/// @param[in,out] pDX контекст обмена DDX.
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTmcRTH_DialogFormatOutFile)
		/// @brief Кнопка «Set default»: отметить сброс форматов и закрыть окно с результатом OK.
		/// @note Кнопка имеет номер `IDOKDefaultFormat` = 2, совпадающий с IDCANCEL.
		afx_msg void OnSetDefaultFormat();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
