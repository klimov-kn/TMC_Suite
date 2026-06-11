// TMCGROUTView.h : interface of the CTMCGROUTView class
//
/////////////////////////////////////////////////////////////////////////////
#include <tmcgrviw.h>
#include <complex1.h>

void PutTrace( CString csTrace );
void PutTrace( char *szTrace );
void PutStatistics( CString csTrace );
void PutStatistics( char *szTrace );

class CTMCGROUTView : public CScrollView
{
protected: // create from serialization only
	CTMCGROUTView();
	DECLARE_DYNCREATE(CTMCGROUTView)

// Attributes
public:
	CTMCGROUTDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTMCGROUTView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTMCGROUTView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTMCGROUTView)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnEditEdit();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnEditGraphicsparameters();
	afx_msg void OnViewGraphics();
	afx_msg void OnEditDocument();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEditAddcharacteristics();
	afx_msg void OnViewResizectrlr();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnViewResizewindow();
	afx_msg void OnConfigEditor();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnConfigFont();
	afx_msg void OnConfigColorpoint();
	afx_msg void OnConfigColorAxis();
	afx_msg void OnConfigColorGrid();
	afx_msg void OnConfigColorBackground();
	afx_msg void OnEditTosmatrix();
	afx_msg void OnViewChangexmaxxminDecriment();
	afx_msg void OnViewChangexmaxxminIncrement();
	afx_msg void OnViewChangeymaxyminDecrement();
	afx_msg void OnViewChangeymaxyminIncrement();
	afx_msg void OnViewAutoxsize();
	afx_msg void OnUpdateViewAutoxsize(CCmdUI* pCmdUI);
	afx_msg void OnViewAutoysize();
	afx_msg void OnUpdateViewAutoysize(CCmdUI* pCmdUI);
	afx_msg void OnViewZoomZoomp();
	afx_msg void OnViewZoomZoom();
	afx_msg void OnViewZoomZoomxp();
	afx_msg void OnViewZoomZoomyp();
	afx_msg void OnViewZoomZoomx();
	afx_msg void OnViewZoomZoomy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnConfigColorGraphics();
	afx_msg void OnViewTranslateChangexmaxxminEnd();
	afx_msg void OnViewTranslateChangexmaxxminHome();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	double GetXAxiesUnit( void );
	BOOL bLossPoint;
	double dPointSize;
	CString csXAxiesFormat;
	CString csYAxiesFormat;
	void PutTrace1( void );
	void PutStatistics1( void );
	void WriteIniWndPar( void );
	void OnDrawBackground(CDC* pDC);
	void VTextOut( CDC *pDC, int nX, int nY, CString cText);
	void IvalidateRectView( CRect  r1);
	CRect MouseResize;
	void OnDrawMouseResize( CDC *pDC );
	BOOL FlagDrawMouseCoordMove;
	BOOL FlagResizeInit;
	BOOL FlagDrawMouseCoord;
	void OnDrawLine( CDC *pDC, int i, int nX, int nY );
	void OnDrawGraph3(CDC *pDC, int i);
	void OnDrawPoint( CDC *pDC, int i, int nX, int nY );
	void OnDrawGraph2(CDC *pDC, int i);
	void OnDrawGraph1(CDC *pDC, int i);
	void OnDrawGraph(CDC *pDC);
	void PrepareLogGraph();
	void RedrawViewGr();
	void PrepareDoubleGraph();
	void ReadDocFileW();
	double LXCordToDoublX( int x );
	double LYCordToDoublY( int y );
    int DoublYCordToLY( double y );
	int DoublXCordToLX( double x );
	double MouseYCoord;
	double MouseXCoord;
	void OnDrawMouseScrol(CDC* pDC);
	void SetRectOutXY();
	CRect rectOutXY;
	void onDrawGrid( CDC *pDC );
	void onDrowAxies( CDC *pDC );
	TMC_GR_WINDOW tmcgrwin;
	TMC_GR_WINDOW tmcgrwin1;
	TMC_GR_TYPE_X * gr_typX;
	TMC_GR_TYPE_Y * gr_typY;
};

#ifndef _DEBUG  // debug version in TMCGROUTView.cpp
inline CTMCGROUTDoc* CTMCGROUTView::GetDocument()
   { return (CTMCGROUTDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
