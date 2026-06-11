#if !defined(AFX_TMCGRPARCOLTYPWID_H__A9118743_3FD1_11D4_B56E_F711396E8A7D__INCLUDED_)
#define AFX_TMCGRPARCOLTYPWID_H__A9118743_3FD1_11D4_B56E_F711396E8A7D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcGrParColTypWid.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcGrParColTypWid dialog

class CTmcGrParColTypWid : public CDialog
{
// Construction
public:
	int LineType;
	int LineWidth;
	COLORREF LineColor;
	CTmcGrParColTypWid(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTmcGrParColTypWid)
	enum { IDD = IDD_DIALOGGRPARALL };
	int		m_Style;
	int		m_Width;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcGrParColTypWid)
	public:
	virtual INT_PTR DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTmcGrParColTypWid)
	afx_msg void OnPaint();
	afx_msg void OnGrParamColor();
	virtual void OnOK();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnRadio5();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCGRPARCOLTYPWID_H__A9118743_3FD1_11D4_B56E_F711396E8A7D__INCLUDED_)
