/// @file TmcGrParColTypWid.h
/// @brief Окно стиля одной линии (`IDD_DIALOGGRPARALL`): цвет, тип и толщина.
///
/// Часть модуля «Вьювер TMC_DN»; совпадает с окном TMCGROUT. Открывается из окна 16 стилей
/// (`CTmcGroutColorGraphDialog::ChangeGraphicsPar`).
/// @see https://www.tamic.ru/docs/code/tmcgrout.html#line-style
#if !defined(AFX_TMCGRPARCOLTYPWID_H__DD525C41_40A9_11D4_B56E_D66AE9E2F47C__INCLUDED_)
#define AFX_TMCGRPARCOLTYPWID_H__DD525C41_40A9_11D4_B56E_D66AE9E2F47C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcGrParColTypWid.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcGrParColTypWid dialog

/// @brief Окно стиля линии: 5 типов линии, 5 толщин (только у сплошной линии), выбор цвета.
///
/// Вызывающий заполняет `LineColor`, `LineType`, `LineWidth` до `DoModal` и читает их
/// после OK. У не сплошных линий выбор толщины недоступен и сбрасывается на 1 (GDI рисует
/// такие линии только толщиной 1).
class CTmcGrParColTypWid : public CDialog
{
// Construction
public:
	/// Тип линии — стиль пера GDI (`PS_SOLID`, `PS_DASH`, `PS_DOT`, `PS_DASHDOT`, `PS_DASHDOTDOT`).
	int LineType;
	/// Толщина линии в единицах окна, 1..5 (в документе — умноженная на 15, 0.01 мм).
	int LineWidth;
	/// Цвет линии, `COLORREF`.
	COLORREF LineColor;
	/// @brief Создаёт окно: сплошная линия толщиной 2, бирюзовый цвет.
	/// @param[in] pParent родительское окно; NULL — главное окно
	CTmcGrParColTypWid(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTmcGrParColTypWid)
	/// Номер разметки окна в ресурсах.
	enum { IDD = IDD_DIALOGGRPARALL };
	/// Номер переключателя типа: 0 сплошная, 1 штрих, 2 точки, 3 штрих-точка, 4 штрих-точка-точка.
	int		m_Style;
	/// Номер переключателя толщины: 0..4 — толщина 1..5 (5 — прочие значения, при OK даёт 3).
	int		m_Width;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcGrParColTypWid)
	public:
	/// @brief Переводит `LineType`/`LineWidth` в номера переключателей и показывает окно.
	/// @return `IDOK` или `IDCANCEL`.
	virtual INT_PTR DoModal();
	protected:
	/// @brief Обмен номеров переключателей типа и толщины с окном (DDX).
	/// @param[in,out] pDX контекст обмена
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTmcGrParColTypWid)
	/// @brief Рисует образцы типов и толщин линии текущим цветом.
	afx_msg void OnPaint();
	/// @brief Кнопка цвета: окно выбора цвета, затем перерисовка образцов.
	afx_msg void OnGrParamColor();
	/// @brief Переводит номера переключателей в `LineType`/`LineWidth` и закрывает окно.
	virtual void OnOK();
	/// @name Переключатели типа линии
	///@{
	/// @brief Переключатель типа линии (1 — сплошная, 2..5 — прочие): толщина доступна только у сплошной; у прочих сбрасывается на 1.
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnRadio5();
	///@}
	/// @brief Устанавливает доступность выбора толщины по типу линии.
	/// @return TRUE — фокус ставит каркас.
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCGRPARCOLTYPWID_H__DD525C41_40A9_11D4_B56E_D66AE9E2F47C__INCLUDED_)
