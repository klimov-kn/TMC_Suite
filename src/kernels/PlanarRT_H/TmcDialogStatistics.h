// TmcDialogStatistics.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcDialogStatistics dialog

#include "TmcRTH_IndanParam.h"
#include "TmcRTH_IndanTopology.h"
#include "TmcRTH_IndanOutput.h"


class CTmcDialogStatistics : public CDialog
{
// Construction
public:
	CString csAccuracy;
	int nFlagStep;
	void PrepareData( void );
	CTmcDialogStatistics(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTmcDialogStatistics)
	enum { IDD = IDD_TMC_STATISTICS };
	int		           m_nStep;
	int		           m_nNumNode;
	double		       m_dMemory;
	int		           m_nBlock;
	CString            m_csErrorMessage;
	CTmcRTH_IndanParam m_cParam;
	CTmcRTH_IndanTopology m_cTopol;
	CTmcRTH_IndanOutput* m_cOut;
	double dDelta;
	double dFreq;
	double dt;
	double dtT;
	double dTsizeWL;
	double dTsizeWL1;
	double dXsizeWL;
	double dXmin;
	double dXmax;
	double dYsizeWL;
	double dYmin;
	double dYmax;
	double dTmin;
	double dTmax;
	double dWaveLen;
	double dWaveLenDelt;
	CString csTolerance;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcDialogStatistics)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTmcDialogStatistics)
	afx_msg void OnViewBlockInformation();
	afx_msg void OnStepPrev();;
	afx_msg void OnStepNext();
	afx_msg void OnCancel1();
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void DeleteData( void );
	CTmcRTH_BlockList * pcBlockList;
};
