#if !defined(AFX_DEVIATDIRPAT_H__22FC9C4F_6D48_43E2_A247_D13473AE581D__INCLUDED_)
#define AFX_DEVIATDIRPAT_H__22FC9C4F_6D48_43E2_A247_D13473AE581D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DeviatDirPat.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDeviatDirPat dialog

class CDeviatDirPat : public CDialog
{
// Construction
public:
	CDeviatDirPat(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDeviatDirPat)
	enum { IDD = IDD_DIALOGCALCDEVIAT };
	CString	m_csFileDevDirPat;
	CString	m_csDistrAmplFaza;
	CString	m_csRequireDirPat;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeviatDirPat)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDeviatDirPat)
	afx_msg void OnButtondeviatdirpattern();
	afx_msg void OnButtondistramplfaza();
	afx_msg void OnButtonrequrdirectpattern();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL CalculateDeviationDirectionalPattern( void );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVIATDIRPAT_H__22FC9C4F_6D48_43E2_A247_D13473AE581D__INCLUDED_)
