// PlanRT_HView.h : interface of the CPlanRT_HView class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef CTMCRTH_CPlanRT_HView_H__
#define CTMCRTH_CPlanRT_HView_H__

#include "TmcSoundEffProp.h"	// Added by ClassView
class CPlanRT_HView : public CScrollView
{
protected: // create from serialization only
	CPlanRT_HView();
	DECLARE_DYNCREATE(CPlanRT_HView)

// Attributes
public:
	CPlanRT_HDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlanRT_HView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	void OnViewField11( void );
	void Stop( void );
	virtual ~CPlanRT_HView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CPlanRT_HView)
	afx_msg void OnEditEdit();
	afx_msg void OnConfigEditor();
	afx_msg void OnConfigColorBackGround();
	afx_msg void OnConfigFormatOutputDataFile();
	afx_msg void OnRunSkipstep();
	afx_msg void OnRunBackstep();
	afx_msg void OnRunRestartall();
	afx_msg void OnRunRestartstep();
	afx_msg void OnRunStartstep();
	afx_msg void OnViewStatistics();
	afx_msg void OnFileSaveAs();
	afx_msg void OnRunStop();
	afx_msg void OnRunAll();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnConfigViewerOutputsignal();
	afx_msg void OnViewOutput();
	afx_msg void OnViewField();
	afx_msg void OnViewField1();
	afx_msg void OnConfigViewerField();
	afx_msg void OnConfigSinchronization();
	afx_msg void OnConfigSound();
	afx_msg void OnConfigSoundMelody();
	afx_msg void OnViewTopology();
	afx_msg void OnUpdateConfigSinchronization(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewField(CCmdUI* pCmdUI);
	afx_msg void OnUpdateConfigSound(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewTopology(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewField1(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRunRun(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRunRestartall(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRunStartstep(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRunRestartstep(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRunSkipstep(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRunBackstep(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRunStop(CCmdUI* pCmdUI);
	afx_msg void OnConfigSmatrix();
	afx_msg void OnConfigViewerSmatrix();
	afx_msg void OnUpdateFileClose(CCmdUI* pCmdUI);
	afx_msg void OnConfigAutorun();
	afx_msg void OnUpdateConfigAutorun(CCmdUI* pCmdUI);
	afx_msg void OnConfigSetup();
	afx_msg void OnViewDirectionalpattern();
	afx_msg void OnUpdateViewDirectionalpattern(CCmdUI* pCmdUI);
	afx_msg void OnConfigViewerDirectionalpattern();
	afx_msg void OnConfigDirectionalpattern();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CTmcSoundEffProp cSoundDialog;
//	CTmcSoundEffProp cSoundDialog("Sound Effects");
	COLORREF scBackgoundColor;
	void OnDrawBackground(CDC* pDC);
	volatile BOOL bIsReadData;
	void ReadData( void );
	void WriteFile( void );
};

#ifndef _DEBUG  // debug version in PlanRT_HView.cpp
inline CPlanRT_HDoc* CPlanRT_HView::GetDocument()
   { return (CPlanRT_HDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
#endif