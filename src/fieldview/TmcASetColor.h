#if !defined(AFX_TMCASETCOLOR_H__91C77601_CA7C_11D3_A942_708A4DC10000__INCLUDED_)
#define AFX_TMCASETCOLOR_H__91C77601_CA7C_11D3_A942_708A4DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcASetColor.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcASetColor dialog

class CTmcASetColor : public CDialog
{
// Construction
public:
	CTmcASetColor(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTmcASetColor)
	enum { IDD = IDD_DIALOGBARCOLORH };
	int		m_Color1;
	int		m_Color2;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcASetColor)
	public:
	virtual int DoModal(COLORREF *c20Color1);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTmcASetColor)
	afx_msg void OnColorLevelAutoSet();
	afx_msg void OnColorLevelAutoSet2();
	afx_msg void OnColorLevelAutoSet3();
	afx_msg void OnDefaultOk();
	afx_msg void OnDefaultTrxyzAngxyz();
	afx_msg void OnPaint();
	afx_msg void OnColorLevelChange();
	afx_msg void OnColorLevelChange2();
	afx_msg void OnColorLevelAutoSet4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void ChangeColorLevel( int n );
	COLORREF * pc20Color;
	int nPixel;
	COLORREF c20Color[20];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCASETCOLOR_H__91C77601_CA7C_11D3_A942_708A4DC10000__INCLUDED_)
