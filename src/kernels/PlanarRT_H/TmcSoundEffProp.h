#if !defined(AFX_TMCSOUNDEFFPROP_H__5D504C23_A7E1_11D3_A93C_B08A4DC10000__INCLUDED_)
#define AFX_TMCSOUNDEFFPROP_H__5D504C23_A7E1_11D3_A93C_B08A4DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcSoundEffProp.h : header file
//

#include "TmcSoundMel1.h"
/////////////////////////////////////////////////////////////////////////////
// CTmcSoundEffProp

class CTmcSoundEffProp : public CPropertySheet
{
	DECLARE_DYNAMIC(CTmcSoundEffProp)

// Construction
public:
	CTmcSoundEffProp(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
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
	CTmcSoundMel1 cMelody1;
	CTmcSoundMel1 cMelody2;
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
