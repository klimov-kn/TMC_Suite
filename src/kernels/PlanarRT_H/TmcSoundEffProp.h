#if !defined(AFX_TMCSOUNDEFFPROP_H__5D504C23_A7E1_11D3_A93C_B08A4DC10000__INCLUDED_)
#define AFX_TMCSOUNDEFFPROP_H__5D504C23_A7E1_11D3_A93C_B08A4DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcSoundEffProp.h : header file
//

/// @file TmcSoundEffProp.h
/// @brief Набор страниц «Sound Effects»: мелодии окончания шага и окончания всего расчёта.
///
/// Часть модуля «Оболочка счётного ядра». В слое совместимости набор страниц
/// показывается окном с закладками. Член вида `CPlanRT_HView::cSoundDialog`.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#dialogs
#include "TmcSoundMel1.h"
/////////////////////////////////////////////////////////////////////////////
// CTmcSoundEffProp

/// @brief Набор из двух страниц мелодий: «Step finish calculation» и «All finish calculation».
///
/// Создаётся вместе с видом (заголовок «Sound Effects»), показывается `DoModal()`
/// из `CPlanRT_HView::OnConfigSoundMelody`.
class CTmcSoundEffProp : public CPropertySheet
{
	DECLARE_DYNAMIC(CTmcSoundEffProp)

// Construction
public:
	/// @brief Создаёт набор с заголовком из строкового ресурса; страницы НЕ добавляет.
	/// @param[in] nIDCaption  номер строки заголовка.
	/// @param[in] pParentWnd  родительское окно; NULL — главное окно.
	/// @param[in] iSelectPage номер страницы, открытой первой.
	CTmcSoundEffProp(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	/// @brief Создаёт набор с заголовком-строкой и добавляет обе страницы мелодий с их названиями.
	/// @param[in] pszCaption  заголовок окна.
	/// @param[in] pParentWnd  родительское окно; NULL — главное окно.
	/// @param[in] iSelectPage номер страницы, открытой первой.
	CTmcSoundEffProp(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcSoundEffProp)
	//}}AFX_VIRTUAL

// Implementation
public:
	/// Мелодия окончания шага расчёта.
	CTmcSoundMel1 cMelody1;
	/// Мелодия окончания всего расчёта.
	CTmcSoundMel1 cMelody2;
	/// @brief Разрушает набор страниц.
	virtual ~CTmcSoundEffProp();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcSoundEffProp)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCSOUNDEFFPROP_H__5D504C23_A7E1_11D3_A93C_B08A4DC10000__INCLUDED_)
