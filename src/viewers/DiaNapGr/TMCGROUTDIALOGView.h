// TMCGROUTDIALOGView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTDIALOGView dialog

class CTMCGROUTDIALOGView : public CDialog
{
// Construction
public:
	BOOL m_XTypeGHz1;
	CTMCGROUTDIALOGView(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTMCGROUTDIALOGView)
	enum { IDD = IDD_DIALOG1 };
	BOOL	m_ColorGraphics;
	BOOL	m_ColorBackground;
	BOOL	m_ColorAxies;
	BOOL	m_ColorGrid;
	BOOL	m_Color;
	BOOL	m_Font;
	BOOL	m_aXSFlag;
	BOOL	m_aYSFlag;
	CString	m_szDatFileName;
	double	m_XMax;
	double	m_XMin;
	double	m_YMax;
	double	m_YMin;
	BOOL	m_XTypeHz1;
	BOOL	m_XTypeKHz1;
	BOOL	m_XTypeMHz1;
	BOOL	m_DrawPoinFlag;
	int		m_XUnit;
	int		m_YUnit;
	CString	m_csXFormat;
	CString	m_csYFormat;
	double	m_dPointSize;
	BOOL	m_LossPoint;
	BOOL	m_YNormirovkaFlag;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTMCGROUTDIALOGView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTMCGROUTDIALOGView)
	afx_msg void OnGrparXinghz();
	afx_msg void OnGrparXinhz();
	afx_msg void OnGrparXinkhz();
	afx_msg void OnGrparXinmhz();
	afx_msg void OnOk2();
	afx_msg void OnOk3();
	afx_msg void OnOk4();
	afx_msg void OnOk5();
	afx_msg void OnOk6();
	afx_msg void OnFormatdefaultbutton();
	afx_msg void OnOk7();
	virtual BOOL OnInitDialog();
	afx_msg void OnGrparAxsflag();
	afx_msg void OnGrparAysflag();
	afx_msg void OnGrparPointflag();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
