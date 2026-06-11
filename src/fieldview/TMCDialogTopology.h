#if !defined(AFX_TMCDIALOGTOPOLOGY_H__5C245DA1_B155_11D3_A93D_30894DC10000__INCLUDED_)
#define AFX_TMCDIALOGTOPOLOGY_H__5C245DA1_B155_11D3_A93D_30894DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TMCDialogTopology.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTMCDialogTopology dialog

class CTMCDialogTopology : public CPropertyPage
{
	DECLARE_DYNCREATE(CTMCDialogTopology)

// Construction
public:
	CFldViewView * pFieldView;
	COLORREF scColorEps;
	COLORREF scColorMet;
	COLORREF scColorMag;
	COLORREF scColorInp;
	COLORREF scColorAbs;
	CTMCDialogTopology();
	~CTMCDialogTopology();

// Dialog Data
	//{{AFX_DATA(CTMCDialogTopology)
	enum { IDD = IDD_DIALOGBARTOPOLOGY };
	BOOL	m_DrawFlagEps;
	BOOL	m_DrawFlagInput;
	BOOL	m_DrawFlagMagnetic;
	BOOL	m_DrawFlagMetal;
	BOOL	m_DrawFlagSizeBlock;
	BOOL	m_DrawFlagSizeLinkList;
	BOOL	m_DrawFlagSurface;
	BOOL	m_DrawFlagLine;
	int		m_23Dimention;
	int		m_Blend;
	BOOL	m_DrawFlagAbsorber;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTMCDialogTopology)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTMCDialogTopology)
	afx_msg void OnPaint();
	afx_msg void OnTopepscoloreps();
	afx_msg void OnTopinpcolorinput();
	afx_msg void OnTopmagcolormagnetic();
	afx_msg void OnTopmetcolormetal();
	afx_msg void OnTopmetcolordefault();
	afx_msg void OnTopinpcolorabsorber();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int nPixel;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCDIALOGTOPOLOGY_H__5C245DA1_B155_11D3_A93D_30894DC10000__INCLUDED_)
