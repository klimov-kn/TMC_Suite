#if !defined(AFX_TMCGROUTCOLORGRAPHDIALOG_H__5C17A5A1_3FAE_11D4_B56E_F711396E8A7D__INCLUDED_)
#define AFX_TMCGROUTCOLORGRAPHDIALOG_H__5C17A5A1_3FAE_11D4_B56E_F711396E8A7D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcGroutColorGraphDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcGroutColorGraphDialog dialog

#include <tmcgrviw.h>

class CTmcGroutColorGraphDialog : public CDialog
{
// Construction
public:
	void GetGraphParam( TMC_GR_DOC1 *sGrDoc1 );
	void SetGraphParam( TMC_GR_DOC1 *sGrDoc1 );
	CTmcGroutColorGraphDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTmcGroutColorGraphDialog)
	enum { IDD = IDD_DIALOGBARCOLORH };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcGroutColorGraphDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTmcGroutColorGraphDialog)
	afx_msg void OnPaint();
	afx_msg void OnColorLevelAutoSet();
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
	afx_msg void OnGrapparcordefault();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void SetDefault( void );
	void ChangeGraphicsPar( int n );
	TMC_GR_DOC1 sGrDoc[16];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCGROUTCOLORGRAPHDIALOG_H__5C17A5A1_3FAE_11D4_B56E_F711396E8A7D__INCLUDED_)
