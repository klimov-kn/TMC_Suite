#if !defined(AFX_TMCDIALOGPLACE_H__5C245DA2_B155_11D3_A93D_30894DC10000__INCLUDED_)
#define AFX_TMCDIALOGPLACE_H__5C245DA2_B155_11D3_A93D_30894DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TMCDialogPlace.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTMCDialogPlace dialog

class CTMCDialogPlace : public CPropertyPage
{
	DECLARE_DYNCREATE(CTMCDialogPlace)

// Construction
public:
	CFldViewView * pFieldView;
	LOGFONT lfInitial;
	int nPixel;
	COLORREF scColorAxies;
	CTMCDialogPlace();
	~CTMCDialogPlace();

// Dialog Data
	//{{AFX_DATA(CTMCDialogPlace)
	enum { IDD = IDD_DIALOGBARPLACE };
	BOOL	m_AutoFlagX;
	BOOL	m_m_AutoFlagY;
	BOOL	m_m_AxiesDrawFlag;
	double	m_dXAxiesAngle;
	double	m_dYAxiesAngle;
	double	m_dXAxiesMax;
	double	m_dYAxiesMax;
	double	m_dXAxiesMin;
	double	m_dYAxiesMin;
	double	m_dXAxiesTrans;
	double	m_dYAxiesTrans;
	CString	m_TimeUnit;
	CString	m_LongUnit;
	BOOL	m_DrawFlafSizeX;
	BOOL	m_DrawFlafSizeY;
	BOOL	m_Proportionally;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTMCDialogPlace)
	protected:
	virtual BOOL OnApply();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTMCDialogPlace)
	afx_msg void OnPlaceaxiescolor();
	afx_msg void OnPlaceaxiesfont();
	afx_msg void OnPaint();
	afx_msg void OnAutosizeflagx();
	afx_msg void OnAutosizeflagy();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCDIALOGPLACE_H__5C245DA2_B155_11D3_A93D_30894DC10000__INCLUDED_)
