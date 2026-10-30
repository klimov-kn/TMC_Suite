// TmcSoundEffProp.cpp : implementation file
//

/// @file TmcSoundEffProp.cpp
/// @brief Набор страниц «Sound Effects»: конструкторы и добавление страниц.
///
/// Часть модуля «Оболочка счётного ядра». Реализация `CTmcSoundEffProp`.
/// В ядре X код файла совпадает с одноимённым файлом ядра H (`src/kernels/PlanarRT_H`).
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#x-differences
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#dialogs
#include "StdAfx.h"
#include "PlanRT_H.h"
#include "TmcSoundEffProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcSoundEffProp

IMPLEMENT_DYNAMIC(CTmcSoundEffProp, CPropertySheet)

CTmcSoundEffProp::CTmcSoundEffProp(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CTmcSoundEffProp::CTmcSoundEffProp(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet( pszCaption, pParentWnd, iSelectPage)
{
	AddPage( &cMelody1 );
	AddPage( &cMelody2 );
	cMelody1.csMelodyName.Format("Step finish calculation");
	cMelody2.csMelodyName.Format("All finish calculation");
	return;
}

CTmcSoundEffProp::~CTmcSoundEffProp()
{
}


BEGIN_MESSAGE_MAP(CTmcSoundEffProp, CPropertySheet)
	//{{AFX_MSG_MAP(CTmcSoundEffProp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTmcSoundEffProp message handlers
