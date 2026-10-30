#if !defined(AFX_TMCSOUNDMEL1_H__5D504C21_A7E1_11D3_A93C_B08A4DC10000__INCLUDED_)
#define AFX_TMCSOUNDMEL1_H__5D504C21_A7E1_11D3_A93C_B08A4DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcSoundMel1.h : header file
//

/// @file TmcSoundMel1.h
/// @brief Страница мелодии звукового оповещения: 12 нот и длительность тона.
///
/// Часть модуля «Оболочка счётного ядра». Две такие страницы входят в набор
/// `CTmcSoundEffProp` (Config -> Sound -> Melody). Разметка — `IDD_TMC_SOUND_MELODY1`.
/// В ядре X код файла совпадает с одноимённым файлом ядра H (`src/kernels/PlanarRT_H`).
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#x-differences
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#dialogs
/////////////////////////////////////////////////////////////////////////////
// CTmcSoundMel1 dialog

/// @brief Страница одной мелодии: номер ноты в каждой из 12 позиций и интервал.
///
/// Объекты — члены `CTmcSoundEffProp`; значения заполняет и забирает вид
/// (`OnConfigSoundMelody`). Ноты задаются переключателями (номер выбранного
/// переключателя в ряду): 0…11 — полутоны 262…494 Гц, 12 — конец мелодии; по умолчанию
/// все позиции — 12.
class CTmcSoundMel1 : public CPropertyPage
{
	DECLARE_DYNCREATE(CTmcSoundMel1)

// Construction
public:
	/// Название мелодии в поле страницы («Step finish calculation» / «All finish calculation»).
	CString csMelodyName;
	/// @brief Создаёт страницу: интервал 1, все ноты 12, название пустое.
	CTmcSoundMel1();
	/// @brief Разрушает страницу.
	~CTmcSoundMel1();

// Dialog Data
	//{{AFX_DATA(CTmcSoundMel1)
	/// Номер разметки страницы в ресурсах.
	enum { IDD = IDD_TMC_SOUND_MELODY1 };
	/// Длительность каждого тона мелодии, мс (поле TimeInterval), допустимо 1…60000.
	int		m_SoundInterval;
	/// @name Ноты мелодии 1…12
	/// Номер выбранного переключателя в ряду позиции (`DDX_Radio`): 0…11 — тоны 262…494 Гц
	/// (полутоны от «до» первой октавы), 12 — конец мелодии (по умолчанию).
	///@{
	/// Номер ноты в позиции мелодии: 0…11 — тоны 262…494 Гц, 12 — конец мелодии.
	int		m_Note1;
	int		m_Note2;
	int		m_Note3;
	int		m_Note4;
	int		m_Note5;
	int		m_Note6;
	int		m_Note7;
	int		m_Note8;
	int		m_Note9;
	int		m_Note10;
	int		m_Note11;
	int		m_Note12;
///@}
//	CString	csMelodyName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTmcSoundMel1)
	protected:
	/// @brief Обмен интервала, нот и названия с элементами страницы; проверяет интервал 1…60000.
	/// @param[in,out] pDX контекст обмена DDX.
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTmcSoundMel1)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCSOUNDMEL1_H__5D504C21_A7E1_11D3_A93C_B08A4DC10000__INCLUDED_)
