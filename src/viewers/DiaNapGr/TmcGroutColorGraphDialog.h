/// @file TmcGroutColorGraphDialog.h
/// @brief Окно Config > Color > Graphics (`IDD_DIALOGBARCOLORH`): цвет, тип и толщина 16 стилей линий.
///
/// Часть модуля «Вьювер TMC_DN»; совпадает с окном TMCGROUT (другой порядок объявлений).
/// @see https://www.tamic.ru/docs/code/tmcgrout.html#line-style
#if !defined(AFX_TMCGROUTCOLORGRAPHDIALOG_H__2FA3ADE2_407E_11D4_B56E_9A2BF15AD37D__INCLUDED_)
#define AFX_TMCGROUTCOLORGRAPHDIALOG_H__2FA3ADE2_407E_11D4_B56E_9A2BF15AD37D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcGroutColorGraphDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcGroutColorGraphDialog dialog

#include <Tmcgrviw.h>

/// @brief Окно 16 стилей линий: образцы линий и кнопки изменения каждого стиля.
///
/// Работает с копией стилей: `SetGraphParam` до `DoModal`, `GetGraphParam` после OK.
/// Толщина внутри окна хранится делённой на 15 (1..5 — толщины окна стиля линии), в
/// документе — в единицах 0.01 мм. Создаётся на стеке `CTMCGROUTView::OnConfigColorGraphics`.
class CTmcGroutColorGraphDialog : public CDialog
{
// Construction
public:
	/// @brief Копирует стили из окна в массив документа (толщина умножается на 15).
	/// @param[out] sGrDoc1 массив из 16 стилей документа; NULL — ничего не делается
	void GetGraphParam( TMC_GR_DOC1 *sGrDoc1 );
	/// @brief Копирует стили документа в окно (толщина делится на 15).
	/// @param[in] sGrDoc1 массив из 16 стилей документа; NULL — ничего не делается
	void SetGraphParam( TMC_GR_DOC1 *sGrDoc1 );
	/// @brief Создаёт окно со стилями по умолчанию (`SetDefault`).
	/// @param[in] pParent родительское окно; NULL — главное окно
	CTmcGroutColorGraphDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTmcGroutColorGraphDialog)
	/// Номер разметки окна в ресурсах.
	enum { IDD = IDD_DIALOGBARCOLORH };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcGroutColorGraphDialog)
	protected:
	/// @brief Обмен данными с элементами окна; полей для обмена нет.
	/// @param[in,out] pDX контекст обмена
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTmcGroutColorGraphDialog)
	/// @brief Рисует образцы 16 линий в полях `IDC_STATICLEVEL1..16` их цветом, типом и толщиной.
	afx_msg void OnPaint();
	/// @name Кнопки стилей линий
	///@{
	/// @brief Кнопка стиля номер N = 2..16 (`OnColorLevelAutoSetN`): открывает окно стиля этой линии (`ChangeGraphicsPar(N-1)`).
	afx_msg void OnColorLevelAutoSet2();
	afx_msg void OnColorLevelAutoSet3();
	afx_msg void OnColorLevelAutoSet4();
	afx_msg void OnColorLevelAutoSet5();
	afx_msg void OnColorLevelAutoSet6();
	afx_msg void OnColorLevelAutoSet7();
	afx_msg void OnColorLevelAutoSet8();
	afx_msg void OnColorLevelAutoSet9();
	afx_msg void OnColorLevelAutoSet10();
	afx_msg void OnColorLevelAutoSet11();
	afx_msg void OnColorLevelAutoSet12();
	afx_msg void OnColorLevelAutoSet13();
	afx_msg void OnColorLevelAutoSet14();
	afx_msg void OnColorLevelAutoSet15();
	afx_msg void OnColorLevelAutoSet16();
	///@}
	/// @brief Кнопка «по умолчанию»: возвращает стили по умолчанию и перерисовывает окно.
	afx_msg void OnGrapparcordefault();
	/// @brief Кнопка стиля 1: открывает окно стиля первой линии (`ChangeGraphicsPar(0)`).
	afx_msg void OnColorLevelAutoSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	/// @brief Задаёт 16 стилей по умолчанию (те же цвета и маркеры, что у документа; толщина 50/15 = 3).
	void SetDefault( void );
	/// @brief Открывает окно стиля линии `n` (`CTmcGrParColTypWid`) и после OK запоминает цвет, тип и толщину.
	/// @param[in] n номер стиля, 0..15; иначе ничего не делается
	void ChangeGraphicsPar( int n );
	/// Копия 16 стилей линий (толщина — в единицах окна, делённая на 15).
	TMC_GR_DOC1 sGrDoc[16];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCGROUTCOLORGRAPHDIALOG_H__2FA3ADE2_407E_11D4_B56E_9A2BF15AD37D__INCLUDED_)
