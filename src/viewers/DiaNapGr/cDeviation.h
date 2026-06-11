#if !defined(AFX_CDEVIATION_H__EFD8FF16_B805_4DAE_A799_E84714E00F81__INCLUDED_)
#define AFX_CDEVIATION_H__EFD8FF16_B805_4DAE_A799_E84714E00F81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// cDeviation.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// cDeviation dialog

class cDeviation : public CDialog
{
// Construction
public:
	cDeviation(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(cDeviation)
	enum { IDD = IDD_DIALOGMAKEDEVIAT };
	CString	m_csFileFrom;
	CString	m_csFileTo;
	double	m_dXdev;
	double	m_dYdev;
	double	m_dFazaDev;
	double	m_dAmpDev;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cDeviation)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(cDeviation)
	afx_msg void OnButtonsource();
	afx_msg void OnButtondistination();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL MakeDeviation( void );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDEVIATION_H__EFD8FF16_B805_4DAE_A799_E84714E00F81__INCLUDED_)
