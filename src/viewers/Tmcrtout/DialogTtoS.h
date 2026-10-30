/// @file DialogTtoS.h
/// @brief Окно Edit > To S-matrix (`IDD_DIALOG3`): пересчёт сигналов `.t` документа в S-матрицу `.s`.
///
/// Часть модуля «Вьювер TMCROS». Открывает `CTMCGROUTView::OnEditTosmatrix`.
/// @see https://www.tamic.ru/docs/code/tmcros.html#dialog-ttos
#if !defined(AFX_DIALOGTTOS_H__5B981521_D4E1_11D3_A945_00A94DC10000__INCLUDED_)
#define AFX_DIALOGTTOS_H__5B981521_D4E1_11D3_A945_00A94DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogTtoS.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogTtoS dialog

#include "TmcTtoS.h"


/// @brief Окно пересчёта «время -> S»: до 20 файлов `.t`, у каждого — признак обработки и частота записи в `.s`.
///
/// Создаётся на стеке обработчиком `OnEditTosmatrix`, который заполняет строки файлами
/// графиков документа, окно времени — текущим диапазоном оси X, частоты — частотами из
/// заголовков `.t`. Каждый отмеченный файл пересчитывает `ConvertOne`: многочастотный
/// (`CTmcWideband::IsWidebandFile`) — `CTmcWideband`, остальные — `CTmcTtoS`. Все результаты
/// дописываются в один файл `m_SFileName`. Пересчёт останавливается на первом файле с ошибкой;
/// текст ошибки виден в поле окна.
/// @note Потоки: только главный поток; пересчёт идёт синхронно, окно на это время не отвечает.
/// @see https://www.tamic.ru/docs/code/tmcros.html#dialog-ttos
class CDialogTtoS : public CDialog
{
// Construction
public:
	/// Множитель перевода единиц оси X во время, с/единицу (1e-12 для пс ... 1 для с и для номера такта); задаёт вид.
	double dTUnit;
	/// @brief Создаёт окно: все строки отмечены и пусты, окно времени 0..0, автозаполнение частот 1e10 Гц с шагом 1e8 Гц.
	/// @param[in] pParent родительское окно; NULL — главное окно
	CDialogTtoS(CWnd* pParent = NULL);   // standard constructor
	//  one signal file -> S-matrix. Wideband runs go the wideband way,
	//  everything else keeps the old single-frequency path.
	/// @brief Пересчитывает один файл `.t` и дописывает результат в `m_SFileName`.
	///
	/// Окно времени — `m_dTmin*dTUnit .. m_dTmax*dTUnit`, с. Многочастотный файл обрабатывает
	/// `CTmcWideband` (все частоты сигнала; `dXValue` не используется, предупреждение запоминается
	/// для показа после OK); иначе `CTmcTtoS` записывает одну точку под частотой `dXValue`.
	/// Текст результата — в `m_csError`, признак ошибки — в `m_bLastError`.
	/// @param[in] csTFile путь к файлу `.t`
	/// @param[in] dXValue частота, под которой точка записывается в `.s`, Гц
	/// @see https://www.tamic.ru/docs/code/tmcros.html#ttos-freq
	void ConvertOne( CString csTFile, double dXValue );
	/// TRUE — последний `ConvertOne` закончился ошибкой (OK тогда не закрывает окно).
	BOOL m_bLastError;
	/// Путь файла документа: по нему строятся имя `.s` по умолчанию и папка для относительных имён `.t`.
	CString	m_DocFileName;

// Dialog Data
	//{{AFX_DATA(CDialogTtoS)
	/// Номер разметки окна в ресурсах.
	enum { IDD = IDD_DIALOG3 };
	/// @name Признак обработки
	///@{
	/// TRUE — пересчитывать файл строки окна (номер — в имени поля); по умолчанию — признак показа графика.
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
	/// Путь к файлу `.s` результата; по умолчанию — путь документа с расширением `.s`; до 300 символов.
	CString	m_SFileName;
	/// @name Файл сигналов
	///@{
	/// Путь к файлу `.t` строки окна (номер — в имени поля); относительное имя дополняется папкой документа в `DoModal`; пусто — строка пропускается.
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
	/// Конец окна времени в единицах оси X (`m_csTimeUnit`).
	double	m_dTmax;
	/// Начало окна времени в единицах оси X.
	double	m_dTmin;
	/// Подпись единиц времени (`ps`, `ns`, `mks`, `ms`, `s`; пусто — ось X в номерах тактов), только для показа.
	CString	m_csTimeUnit;
	/// @name Частота записи
	///@{
	/// Частота, под которой результат строки окна (номер — в имени поля) записывается в `.s`, Гц;
	/// по умолчанию — частота из заголовка `.t`. Для многочастотного файла не используется.
	double	m_dXValue1;
	double	m_dXValue2;
	double	m_dXValue3;
	double	m_dXValue4;
	double	m_dXValue5;
	double	m_dXValue6;
	double	m_dXValue7;
	double	m_dXValue8;
	double	m_dXValue9;
	double	m_dXValue10;
	double	m_dXValue11;
	double	m_dXValue12;
	double	m_dXValue13;
	double	m_dXValue14;
	double	m_dXValue15;
	double	m_dXValue16;
	double	m_dXValue17;
	double	m_dXValue18;
	double	m_dXValue19;
	double	m_dXValue20;
	///@}
	/// Начальная частота автозаполнения столбца частот, Гц (по умолчанию 1e10).
	double	m_dXmin;
	/// Шаг автозаполнения столбца частот, Гц (по умолчанию 1e8).
	double	m_dDx;
	/// Текст результата последнего пересчёта: `No error`, текст ошибки или предупреждение (только для показа).
	CString	m_csError;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogTtoS)
	public:
	/// @brief Перед показом задаёт имя `.s` по умолчанию (путь документа с расширением `.s`) и дополняет относительные имена `.t` папкой документа.
	/// @return `IDOK` или `IDCANCEL`.
	virtual INT_PTR DoModal();
	protected:
	/// @brief Обмен данными полей с элементами окна (DDX), ограничение длины строк 300 символами.
	/// @param[in,out] pDX контекст обмена
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogTtoS)
	/// @brief Кнопка выбора файла `.s` результата (окно выбора в папке документа, фильтр `*.s`).
	afx_msg void OnChangeSfilename();
	/// @brief Кнопка добавления файла `.t`: выбранный путь ставится в первую пустую строку (если пустых нет — не ставится).
	afx_msg void OnAddCharacteristics();
	/// @brief Кнопка пересчёта: `ConvertOne` для каждой отмеченной непустой строки по порядку 1..20, до первой ошибки; окно остаётся открытым.
	afx_msg void OnExportCharacteristics();
	/// @brief Кнопка автозаполнения: частоты строк 1..20 = `m_dXmin + k*m_dDx`, k = 0..19.
	afx_msg void OnExportFillxvalue();
	/// @brief OK: выполняет пересчёт (`OnExportCharacteristics`); при ошибке окно не закрывается, при предупреждении многочастотной обработки показывает его и закрывает окно.
	virtual void OnOK();
	/// @brief Кнопка удаления: сразу удаляет файл `m_SFileName` (без подтверждения). Нужна, потому что пересчёт дописывает точки в существующий `.s`.
	afx_msg void OnChangeSfiledelet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGTTOS_H__5B981521_D4E1_11D3_A945_00A94DC10000__INCLUDED_)
