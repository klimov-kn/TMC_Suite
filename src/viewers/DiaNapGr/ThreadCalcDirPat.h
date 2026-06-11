#if !defined(AFX_THREADCALCDIRPAT_H__50BE87B7_7475_4BB6_A105_69EC73F63BF8__INCLUDED_)
#define AFX_THREADCALCDIRPAT_H__50BE87B7_7475_4BB6_A105_69EC73F63BF8__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ThreadCalcDirPat.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CThreadCalcDirPat dialog

class CThreadCalcDirPat : public CDialog
{
// Construction
public:
	void * pCView;
	CThreadCalcDirPat(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CThreadCalcDirPat)
	enum { IDD = IDD_READDATA_THREAD };
	CProgressCtrl	m_ProgressAll;
	CProgressCtrl	m_Progress;
	CString	m_csStatistics;
	CString	m_csStatisticsAll;
	int		m_CurrentDirPat;
	int		m_NAllDirPat;
	CString	m_csElapsTime;
	int		m_nEmitters;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CThreadCalcDirPat)
	public:
	virtual INT_PTR DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CThreadCalcDirPat)
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	time_t ltime_current;
	time_t ltime_start;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_THREADCALCDIRPAT_H__50BE87B7_7475_4BB6_A105_69EC73F63BF8__INCLUDED_)
