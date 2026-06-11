// DialogDoc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogDoc dialog

class CDialogDoc : public CDialog
{
// Construction
public:
	double dTUnit;
	BOOL m_AddCharacteristicsFlag;
	CDialogDoc(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDialogDoc)
	enum { IDD = IDD_DIALOG2 };
	CString	m_DocFileName;
	CString	m_GraphName1;
	CString	m_GraphName10;
	CString	m_GraphName11;
	CString	m_GraphName12;
	CString	m_GraphName13;
	CString	m_GraphName14;
	CString	m_GraphName15;
	CString	m_GraphName16;
	CString	m_GraphName17;
	CString	m_GraphName18;
	CString	m_GraphName19;
	CString	m_GraphName2;
	CString	m_GraphName20;
	CString	m_GraphName3;
	CString	m_GraphName4;
	CString	m_GraphName5;
	CString	m_GraphName6;
	CString	m_GraphName7;
	CString	m_GraphName8;
	CString	m_GraphName9;
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
	BOOL	m_NormGrFlag1;
	BOOL	m_NormGrFlag2;
	BOOL	m_NormGrFlag3;
	BOOL	m_NormGrFlag4;
	BOOL	m_NormGrFlag5;
	BOOL	m_NormGrFlag6;
	BOOL	m_NormGrFlag7;
	BOOL	m_NormGrFlag8;
	BOOL	m_NormGrFlag9;
	BOOL	m_NormGrFlag10;
	BOOL	m_NormGrFlag11;
	BOOL	m_NormGrFlag12;
	BOOL	m_NormGrFlag13;
	BOOL	m_NormGrFlag14;
	BOOL	m_NormGrFlag15;
	BOOL	m_NormGrFlag16;
	BOOL	m_NormGrFlag17;
	BOOL	m_NormGrFlag18;
	BOOL	m_NormGrFlag19;
	BOOL	m_NormGrFlag20;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogDoc)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogDoc)
	afx_msg void OnAddCharacteristics();
	afx_msg void OnAddMakedeviation();
	afx_msg void OnAddMakedeviation2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
