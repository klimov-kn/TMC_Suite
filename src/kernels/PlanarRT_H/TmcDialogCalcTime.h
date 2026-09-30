// TmcDialogCalcTime.h : window "Calculation time" (TIMING)
//
// Shows the report of CTmcCalcTiming: time of every phase of the calculation
// (preprocessor, preparation, pure calculation, disk writes, postprocessor)
// and saves it to a text file.

#ifndef _TMCDIALOGCALCTIME_H__
#define _TMCDIALOGCALCTIME_H__

#include <TmcCalcTiming.h>

class CTmcDialogCalcTime : public CDialog
{
public:
	CTmcDialogCalcTime(CWnd* pParent = NULL);
	enum { IDD = IDD_TMC_CALCTIME };
	CTmcCalcTiming *m_pcTiming;		// timing of the document (not owned)

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnRefresh();
	afx_msg void OnExport();
	DECLARE_MESSAGE_MAP()

private:
	CFont m_cFont;
	void ShowReport( void );
};

#endif
