#if !defined(AFX_DIALOGTTOS_H__5B981521_D4E1_11D3_A945_00A94DC10000__INCLUDED_)
#define AFX_DIALOGTTOS_H__5B981521_D4E1_11D3_A945_00A94DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogTtoS.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogTtoS dialog

#include "TmcTtoS.h"


class CDialogTtoS : public CDialog
{
// Construction
public:
	double dTUnit;
	CDialogTtoS(CWnd* pParent = NULL);   // standard constructor
	CString	m_DocFileName;

// Dialog Data
	//{{AFX_DATA(CDialogTtoS)
	enum { IDD = IDD_DIALOG3 };
	BOOL	m_OutGrFlag_1;
	BOOL	m_OutGrFlag_10;
	BOOL	m_OutGrFlag_11;
	BOOL	m_OutGrFlag_12;
	BOOL	m_OutGrFlag_13;
	BOOL	m_OutGrFlag_15;
	BOOL	m_OutGrFlag_14;
	BOOL	m_OutGrFlag_16;
	BOOL	m_OutGrFlag_17;
	BOOL	m_OutGrFlag_18;
	BOOL	m_OutGrFlag_19;
	BOOL	m_OutGrFlag_2;
	BOOL	m_OutGrFlag_20;
	BOOL	m_OutGrFlag_3;
	BOOL	m_OutGrFlag_4;
	BOOL	m_OutGrFlag_5;
	BOOL	m_OutGrFlag_6;
	BOOL	m_OutGrFlag_7;
	BOOL	m_OutGrFlag_8;
	BOOL	m_OutGrFlag_9;
	CString	m_SFileName;
	CString	m_SFileName_1;
	CString	m_SFileName_10;
	CString	m_SFileName_11;
	CString	m_SFileName_12;
	CString	m_SFileName_13;
	CString	m_SFileName_14;
	CString	m_SFileName_15;
	CString	m_SFileName_16;
	CString	m_SFileName_17;
	CString	m_SFileName_18;
	CString	m_SFileName_19;
	CString	m_SFileName_2;
	CString	m_SFileName_20;
	CString	m_SFileName_3;
	CString	m_SFileName_4;
	CString	m_SFileName_5;
	CString	m_SFileName_6;
	CString	m_SFileName_7;
	CString	m_SFileName_8;
	CString	m_SFileName_9;
	double	m_dTmax;
	double	m_dTmin;
	CString	m_csTimeUnit;
	double	m_dXValue1;
	double	m_dXValue2;
	double	m_dXValue3;
	double	m_dXValue4;
	double	m_dXValue5;
	double	m_dXValue6;
	double	m_dXValue7;
	double	m_dXValue8;
	double	m_dXValue9;
	double	m_dXValue10;
	double	m_dXValue11;
	double	m_dXValue12;
	double	m_dXValue13;
	double	m_dXValue14;
	double	m_dXValue15;
	double	m_dXValue16;
	double	m_dXValue17;
	double	m_dXValue18;
	double	m_dXValue19;
	double	m_dXValue20;
	double	m_dXmin;
	double	m_dDx;
	CString	m_csError;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogTtoS)
	public:
	virtual INT_PTR DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogTtoS)
	afx_msg void OnChangeSfilename();
	afx_msg void OnAddCharacteristics();
	afx_msg void OnExportCharacteristics();
	afx_msg void OnExportFillxvalue();
	virtual void OnOK();
	afx_msg void OnChangeSfiledelet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGTTOS_H__5B981521_D4E1_11D3_A945_00A94DC10000__INCLUDED_)
