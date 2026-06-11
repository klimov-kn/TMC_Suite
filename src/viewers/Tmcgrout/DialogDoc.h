// DialogDoc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogDoc dialog

class CDialogDoc : public CDialog
{
// Construction
public:
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
	int		m_np1_10;
	int		m_np1_11;
	int		m_np1_12;
	int		m_np1_13;
	int		m_np1_14;
	int		m_np1_15;
	int		m_np1_16;
	int		m_np1_17;
	int		m_np1_18;
	int		m_np1_19;
	int		m_np1_2;
	int		m_np1_20;
	int		m_np1_1;
	int		m_np1_3;
	int		m_np1_4;
	int		m_np1_5;
	int		m_np1_6;
	int		m_np1_7;
	int		m_np1_8;
	int		m_np1_9;
	int		m_np2_1;
	int		m_np2_10;
	int		m_np2_11;
	int		m_np2_12;
	int		m_np2_13;
	int		m_np2_14;
	int		m_np2_15;
	int		m_np2_16;
	int		m_np2_17;
	int		m_np2_18;
	int		m_np2_19;
	int		m_np2_2;
	int		m_np2_20;
	int		m_np2_3;
	int		m_np2_4;
	int		m_np2_5;
	int		m_np2_6;
	int		m_np2_7;
	int		m_np2_8;
	int		m_np2_9;
	int		m_mod1_1;
	int		m_mod1_10;
	int		m_mod1_11;
	int		m_mod1_12;
	int		m_mod1_13;
	int		m_mod1_14;
	int		m_mod1_15;
	int		m_mod1_16;
	int		m_mod1_17;
	int		m_mod1_18;
	int		m_mod1_19;
	int		m_mod1_2;
	int		m_mod1_20;
	int		m_mod1_3;
	int		m_mod1_4;
	int		m_mod1_5;
	int		m_mod1_6;
	int		m_mod1_7;
	int		m_mod1_8;
	int		m_mod1_9;
	int		m_mod2_1;
	int		m_mod2_10;
	int		m_mod2_11;
	int		m_mod2_12;
	int		m_mod2_13;
	int		m_mod2_14;
	int		m_mod2_15;
	int		m_mod2_16;
	int		m_mod2_17;
	int		m_mod2_18;
	int		m_mod2_19;
	int		m_mod2_2;
	int		m_mod2_20;
	int		m_mod2_3;
	int		m_mod2_4;
	int		m_mod2_5;
	int		m_mod2_6;
	int		m_mod2_7;
	int		m_mod2_8;
	int		m_mod2_9;
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
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogDoc)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogDoc)
	afx_msg void OnAddCharacteristics();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
