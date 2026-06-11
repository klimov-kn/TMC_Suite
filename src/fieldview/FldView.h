// FldView.h : main header file for the FLDVIEW application
//

#if !defined(AFX_FLDVIEW_H__9FEC572D_707F_11D3_A93C_E0AB4DC10000__INCLUDED_)
#define AFX_FLDVIEW_H__9FEC572D_707F_11D3_A93C_E0AB4DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CFldViewApp:
// See FldView.cpp for the implementation of this class
//

#define WM_USERAPPLY_PROPPLACE  WM_USER+5
#define WM_USERAPPLY_PROPTOPOL  WM_USER+6
#define WM_USERAPPLY_PROPEPS	WM_USER+7
#define WM_USERAPPLY_PROPFIELD	WM_USER+8

class CFldViewApp : public CWinApp
{
public:
	CFldViewApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFldViewApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CFldViewApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLDVIEW_H__9FEC572D_707F_11D3_A93C_E0AB4DC10000__INCLUDED_)
