#if !defined(AFX_TMCDIALOGFIELD_H__5C245DA3_B155_11D3_A93D_30894DC10000__INCLUDED_)
#define AFX_TMCDIALOGFIELD_H__5C245DA3_B155_11D3_A93D_30894DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TMCDialogField.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTMCDialogField dialog

class CTMCDialogField : public CPropertyPage
{
	DECLARE_DYNCREATE(CTMCDialogField)

// Construction
public:
	CFldViewView * pFieldView;
	COLORREF c20Color[20];
//	COLORREF c20ColorBuf[20];
	int nPixel;
	CTMCDialogField();
	~CTMCDialogField();

// Dialog Data
	//{{AFX_DATA(CTMCDialogField)
	enum { IDD = IDD_DIALOGBARFIELD };
	CListCtrl	m_List;
	BOOL	m_bAutoSizeFlag;
	int		m_dBlend;
	BOOL	m_bDrawFlagLine;
	BOOL	m_bDrawFlagSurface;
	double	m_dZAxiesMax;
	double	m_dZAxiesMin;
	int		m_d23Dimension;
	BOOL	m_DrawSizeFlag;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTMCDialogField)
	public:
	virtual void OnOK();
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTMCDialogField)
	afx_msg void OnPaint();
	afx_msg void OnDefaultTrxyzAngxyz();
	afx_msg void OnColorLevelChangeAuto();
	afx_msg void OnAutosizeflagz();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void ChangeColorLevel( int n );
public:
	BOOL bHightColorResolution;
	afx_msg void OnBnClickedAutodrawsizeflagz2();
	BOOL bModulFlag;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCDIALOGFIELD_H__5C245DA3_B155_11D3_A93D_30894DC10000__INCLUDED_)
