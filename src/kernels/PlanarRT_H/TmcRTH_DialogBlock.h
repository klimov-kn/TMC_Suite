// TmcRTH_DialogBlock.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_DialogBlock dialog

#include "TmcRTH_IndanParam.h"
#include "TmcRTH_IndanTopology.h"


class CTmcRTH_DialogBlock : public CDialog
{
// Construction
public:
	void PrepareData( void );
	CTmcRTH_DialogBlock(CWnd* pParent = NULL);   // standard constructor
	CTmcRTH_IndanParam * m_pcParam;
	CTmcRTH_BlockList  * m_pcBlockList;
	int i;
	CString dX[10];
	CString dY[10];

// Dialog Data
	//{{AFX_DATA(CTmcRTH_DialogBlock)
	enum { IDD = IDD_TMC_RTH_TOPBLOCK };
	int		           m_nBlock;
	int		           m_nType;
	double	           m_nMemory;
	int		           m_nXY;
	CString            m_csString;
	CString            m_csVx;
	CString            m_csVy;
	CString            m_csW;
	CString            m_csEpsExpr;
	double dX0;
	double dY0;
	CString dX1;
	CString dY1;
	CString dX2;
	CString dY2;
	CString dX3;
	CString dY3;
	CString dX4;
	CString dY4;
	CString dX5;
	CString dY5;
	CString dX6;
	CString dY6;
	CString dX7;
	CString dY7;
	CString dX8;
	CString dY8;
	CString dX9;
	CString dY9;
	CString dX10;
	CString dY10;
	CString csXminText;
	CString csXmaxText;
	CString csYminText;
	CString csYmaxText;
	double dXmin;
	double dXmax;
	double dYmin;
	double dYmax;
	CString            m_csLongUnit;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTH_DialogBlock)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTmcRTH_DialogBlock)
	virtual void OnOK();
	afx_msg void OnCancel1();
	afx_msg void OnTmcblockbuttonnext();
	afx_msg void OnTmcblockbuttonprev();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
