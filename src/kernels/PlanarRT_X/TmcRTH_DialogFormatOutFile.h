// TmcRTH_DialogFormatOutFile.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_DialogFormatOutFile dialog

class CTmcRTH_DialogFormatOutFile : public CDialog
{
// Construction
public:
	BOOL IsSetDefaultFormat;
	CTmcRTH_DialogFormatOutFile(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTmcRTH_DialogFormatOutFile)
	enum { IDD = IDD_TMC_CONFIG_OUTFFORMAT };
	CString            m_csnT;
	CString            m_csdTcurrent;
	CString            m_csnBlock;
	CString            m_csdInp;
	CString            m_csdOut;
	CString            m_csdOutField;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTH_DialogFormatOutFile)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTmcRTH_DialogFormatOutFile)
		afx_msg void OnSetDefaultFormat();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
