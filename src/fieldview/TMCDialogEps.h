#if !defined(AFX_TMCDIALOGEPS_H__5C245DA4_B155_11D3_A93D_30894DC10000__INCLUDED_)
#define AFX_TMCDIALOGEPS_H__5C245DA4_B155_11D3_A93D_30894DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TMCDialogEps.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTMCDialogEps dialog

class CTMCDialogEps : public CPropertyPage
{
	DECLARE_DYNCREATE(CTMCDialogEps)

// Construction
public:
	CFldViewView * pFieldView;
	COLORREF c20Color[20];
	int nPixel;
	CTMCDialogEps();
	~CTMCDialogEps();

// Dialog Data
	//{{AFX_DATA(CTMCDialogEps)
	enum { IDD = IDD_DIALOGBAREPS };
	int		m_d23Dimension;
	int		m_dBlend;
	BOOL	m_bDrawFlagLine;
	BOOL	m_bDrawFlagSurface;
	BOOL	m_bAutoSizeFlag;
	BOOL	m_DrawSizeFlag;
	double	m_dZAxiesMax;
	double	m_dZAxiesMin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTMCDialogEps)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTMCDialogEps)
	afx_msg void OnPaint();
	afx_msg void OnDefaultTrxyzAngxyz();
	afx_msg void OnColorLevelChange();
	afx_msg void OnAutosizeflagz();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void ChangeColorLevel( int n );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCDIALOGEPS_H__5C245DA4_B155_11D3_A93D_30894DC10000__INCLUDED_)
