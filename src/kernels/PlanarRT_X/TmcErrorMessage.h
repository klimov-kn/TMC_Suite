// TmcErrorMessage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcErrorMessage dialog

class CTmcErrorMessage : public CDialog
{
// Construction
public:
	CTmcErrorMessage(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTmcErrorMessage)
	enum { IDD = IDD_ABOUTBOX };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcErrorMessage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTmcErrorMessage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
