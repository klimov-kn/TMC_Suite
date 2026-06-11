// FldViewView.h : interface of the CFldViewView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLDVIEWVIEW_H__9FEC5737_707F_11D3_A93C_E0AB4DC10000__INCLUDED_)
#define AFX_FLDVIEWVIEW_H__9FEC5737_707F_11D3_A93C_E0AB4DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <io.h>
#include <fcntl.h>
#include <tmcgrviw.h>
#include "TmcGLText.h"
#include <TmcLibError.h>
#include "TmcBlockTpl.h"
#include "TmcBoundaryTpl.h"
#include "colorleveldlg.h"

#define TMC_GROFLD_RAZDEL_INI		"#TMC_GraphicsOutputFieldFile"
#define TMC_GROFLD_DRAWFLAG_INI		"#TMC_GraphOutFld_DrFl"
#define TMC_GROFLD_DRFLSURF_INI		"#TMC_GraphOutFld_DrFlSurface"
#define TMC_GROFLD_DRFLLINE_INI		"#TMC_GraphOutFld_DrFlLine"
#define TMC_GROFLD_DRFLAXIS_INI		"#TMC_GraphOutFld_DrFlAxies"
#define TMC_GROFLD_FLXSIZE_INI		"#TMC_GraphOutFld_FlXSize"
#define TMC_GROFLD_FLYSIZE_INI		"#TMC_GraphOutFld_FlYSize"
#define TMC_GROFLD_FLZSIZE_INI		"#TMC_GraphOutFld_FlZSize"
#define TMC_GROFLD_FL23D_INI		"#TMC_GraphOutFld_Fl23D"
#define TMC_GROFLD_BLEND_INI		"#TMC_GraphOutFld_Blend"
#define TMC_GROFLD_XMIN_INI			"#TMC_GraphOutFld_Xmin"
#define TMC_GROFLD_XMAX_INI			"#TMC_GraphOutFld_Xmax"
#define TMC_GROFLD_YMIN_INI			"#TMC_GraphOutFld_Ymin"
#define TMC_GROFLD_YMAX_INI			"#TMC_GraphOutFld_Ymax"
#define TMC_GROFLD_ZMIN_INI			"#TMC_GraphOutFld_Zmin"
#define TMC_GROFLD_ZMAX_INI			"#TMC_GraphOutFld_Zmax"
#define TMC_GROFLD_XANGLE_INI		"#TMC_GraphOutFld_XAngle"
#define TMC_GROFLD_YANGLE_INI		"#TMC_GraphOutFld_YAngle"
#define TMC_GROFLD_ZANGLE_INI		"#TMC_GraphOutFld_ZAngle"
#define TMC_GROFLD_XTRANS_INI		"#TMC_GraphOutFld_XTranslate"
#define TMC_GROFLD_YTRANS_INI		"#TMC_GraphOutFld_YTranslate"
#define TMC_GROFLD_ZTRANS_INI		"#TMC_GraphOutFld_ZTranslate"
#define TMC_GROFLD_INIWND			"#INIWND"
#define TMC_GROFLD_COLORAXIS_INI	"#TMC_GraphOutFld_ColorAxies"
#define TMC_GROFLD_FONTAXIS_INI		"#TMC_GraphOutFld_FontAxies"
#define TMC_GROFLD_FONTNAMEAXIS_INI	"#TMC_GraphOutFld_FontNameAxies"
#define TMC_GROFLD_COLORZFIELD_INI	"#TMC_GraphOutFld_ColorZField"
#define TMC_GROFLD_COLORZEPS_INI	"#TMC_GraphOutFld_ColorZEps"
#define TMC_GROFLDDRFLAXISSZ_INI	"#TMC_GraphOutFld_Dr_FlAxiesSize"
#define TMC_GROFLDTOPOLOGY_INI		"#TMC_GraphOutFld_Topology"
#define TMC_GROFLDTOPOLOGYCOLOR_INI	"#TMC_GraphOutFld_TopologyColor_EpsMetMagInp"
#define TMC_GROFLDNUMPROPPAGE_INI	"#TMC_GraphOutFld_PropertyPageNumber"
#define TMC_GROFLDCOLORRESOL_INI	"#TMC_GraphOutFld_ColorResolution"
#define TMC_GROFLDTOPEPSPAR_INI		"#TMC_GraphOutFld_EpsParam"
#define TMC_GROFLDMODULFL_INI		"#TMC_GraphOutFld_DrawFieldModulFlag"

void PutTrace( CString csTrace );
void PutTrace( char *szTrace );
void PutStatistics( CString csTrace );
void PutStatistics( char *szTrace );

class CFldViewView : public CScrollView
{
protected: // create from serialization only
	CFldViewView();
	DECLARE_DYNCREATE(CFldViewView)

// Attributes
public:
	CFldViewDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFldViewView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFldViewView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFldViewView)
	afx_msg LRESULT OnUserApplyPropDialogPlace( WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserApplyPropDialogField( WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserApplyPropDialogEps( WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserApplyPropDialogTopol( WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCloseColorLevelDialog( WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRotateLeft();
	afx_msg void OnRotateRight();
	afx_msg void OnRotateTop();
	afx_msg void OnRotateBottom();
	afx_msg void OnRotateLefty();
	afx_msg void OnRotateRighty();
	afx_msg void OnTranslateXt();
	afx_msg void OnTranslateXb();
	afx_msg void OnTranslateZb();
	afx_msg void OnTranslateZt();
	afx_msg void OnTranslateYb();
	afx_msg void OnTranslateYt();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnViewParameters();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnStrelkleft();
	afx_msg void OnStrelkright();
	afx_msg void OnStrelkup();
	afx_msg void OnStrelkdown();
	afx_msg void OnViewZoomP();
	afx_msg void OnViewZoomM();
	afx_msg void OnViewZoomPx();
	afx_msg void OnViewZoomMx();
	afx_msg void OnViewZoomMy();
	afx_msg void OnViewZoomPy();
	afx_msg void OnViewDefault();
	afx_msg void OnRotatexn();
	afx_msg void OnRotatexv();
	afx_msg void OnRotateyn();
	afx_msg void OnRotateyv();
	afx_msg void OnRotatezv();
	afx_msg void OnRotatezn();
	afx_msg void OnTranslateXn();
	afx_msg void OnTranslateXv();
	afx_msg void OnTranslateYn();
	afx_msg void OnTranslateYv();
	afx_msg void OnTranslateZn();
	afx_msg void OnTranslateZv();
	afx_msg void OnTranslatedataxn();
	afx_msg void OnTranslatedataxv();
	afx_msg void OnTranslatedatayn();
	afx_msg void OnTranslatedatayv();
	afx_msg void OnDimension23switch();
	afx_msg void OnFlagdrawsurface();
	afx_msg void OnFlagdrawline();
	afx_msg void OnUpdateFlagdrawline(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFlagdrawsurface(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDimension23switch(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnFlagdrawlineeps();
	afx_msg void OnFlagdrawsurfaceeps();
	afx_msg void OnDimension23switcheps();
	afx_msg void OnUpdateDimension23switcheps(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFlagdrawlineeps(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFlagdrawsurfaceeps(CCmdUI* pCmdUI);
	afx_msg void OnViewProportionally();
	afx_msg void OnUpdateViewProportionally(CCmdUI* pCmdUI);
	afx_msg void OnFlagdrawlinetop();
	afx_msg void OnUpdateFlagdrawlinetop(CCmdUI* pCmdUI);
	afx_msg void OnFlagdrawsurfacetop();
	afx_msg void OnUpdateFlagdrawsurfacetop(CCmdUI* pCmdUI);
	afx_msg void OnDimension23switchtop();
	afx_msg void OnUpdateDimension23switchtop(CCmdUI* pCmdUI);
	afx_msg void OnViewTopologyBoundaryEps();
	afx_msg void OnUpdateViewTopologyBoundaryEps(CCmdUI* pCmdUI);
	afx_msg void OnViewTopologyBoundaryMetal();
	afx_msg void OnUpdateViewTopologyBoundaryMetal(CCmdUI* pCmdUI);
	afx_msg void OnViewTopologyBoundaryMagnetic();
	afx_msg void OnUpdateViewTopologyBoundaryMagnetic(CCmdUI* pCmdUI);
	afx_msg void OnViewTopologyBoundaryInput();
	afx_msg void OnUpdateViewTopologyBoundaryInput(CCmdUI* pCmdUI);
	afx_msg void OnViewTopologyBoundaryAbsorber();
	afx_msg void OnUpdateViewTopologyBoundaryAbsorber(CCmdUI* pCmdUI);
	afx_msg void OnViewFieldSize();
	afx_msg void OnUpdateViewFieldSize(CCmdUI* pCmdUI);
	afx_msg void OnViewEpsSize();
	afx_msg void OnUpdateViewEpsSize(CCmdUI* pCmdUI);
	afx_msg void OnViewTopologyDimensionsAlonganxaxies();
	afx_msg void OnUpdateViewTopologyDimensionsAlonganxaxies(CCmdUI* pCmdUI);
	afx_msg void OnViewTopologyDimensionsAlonganyaxies();
	afx_msg void OnUpdateViewTopologyDimensionsAlonganyaxies(CCmdUI* pCmdUI);
	afx_msg void OnViewTopologySizeBlock();
	afx_msg void OnUpdateViewTopologySizeBlock(CCmdUI* pCmdUI);
	afx_msg void OnViewTopologySizeLinklist();
	afx_msg void OnUpdateViewTopologySizeLinklist(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	COLORREF scBackgoundColor;
	void VTextOut( CDC *pDC, int nX, int nY, CString cText);
	void DrawTopInputNum( void );
	void DrawTopInputNum2( void );
	int nInputNum;
	void DrawSize3( void );
	void DrawSize2( void );
	void DrawSize( void );
	void PrepareBoundTplSizeToGLCoord( void );
	CTmcTopSize * pcTopSize;
	int nTopSize;
	double dZtopol;
	double dZtopolStrelk;
	void PrepareBoundTplSize( void );
	void DrawTopSurface( void );
	void DrawAxiesSizeZField( void );
	void DrawAxies( void );
	void DrawTopdLine( void );
	void DrawTopSurface2( void );
	void SetColornType( int  nType );
	void DrawTopdLine2( void );
	void DrawTopology( void );
	void CalcnTopPolygon( void );
	int nTopPolygon;
	BOOL bProportionally;
	void PrepareBoundaryTpl( void );
	int nBoundBlockTpl;
	CTmcBoundaryTpl * pcBoundBlockTpl;
	void DrawAxiesSizeZEps( void );
	void DrawEpsdLine( void );
	void DrawEpsSurface( void );
	void DrawEpsdLine2( void );
	void DrawEpsSurface2( void );
	void DrawEps( void );
	void PrepareTpl( void );
	void ReadTpl( void );
	CTmcBlockTpl cBlockTpl;
	void SetTplFileName( void );
	CString csTplFileName;
	CPoint cMousePoint;
	BOOL bIsMouseLButonDown;
	void DrawAxiesSizeY( void );
	void DrawAxiesSizeX( void );
	void DrawAxiesSize( void );
	LOGFONT lfAxiesFont;
	double dDelta;
	double XtoGLWinCoord( double x );
	double YtoGLWinCoord( double y );
	double XtoGLWinCoord( int j );
	double YtoGLWinCoord( int i );
	double Z_EpstoGLWinCoord( double dZcurrent );
	double ZtoGLWinCoord( double dZcurrent );
	void PutStatistics1( void );
	void PutTrace1( void );
//	FILETIME LastWriteTime1;
	void SetDefaultValue( void );
	void WriteIniFile( void );
    BOOL volatile bBusy;
	COLORREF pscSurfaceColor[20];
	COLORREF pscSurfaceColor_Eps[20];
	COLORREF scAxiesColor;
	COLORREF scColorEps;
	COLORREF scColorMet;
	COLORREF scColorMag;
	COLORREF scColorInp;
	COLORREF scColorAbs;
	void SetColor_Eps( double z );
	void SetColor( double z );
	void SetColor_Cursor( double z );	
	void SetColor( COLORREF csCol );
	void SetColor( COLORREF csCol, int nBlend11 );
	int nBlend;
	int nBlendTopology;
	int nBlend_Eps;
	int n2D3DFlag;
	int n2D3DFlagTopology;
	int n2D3DFlag_Eps;
	void DrawFieldCursore( void );
	void DrawFieldLine( void );
	void DrawFieldSurface( void );
	void DrawFieldCursore2( void );
	void DrawFieldLine2( void );
	void DrawFieldSurface2( void );
	void PrepareData( void );
	void ReadData( void );
	CTmcLibError cError;
	void DeleteData( void );
	int nX;
	int nY;
	int *pnTopType;
	double  *pdTopXinp;
	double  *pdTopYinp;
	double *pdTopX1;
	double *pdTopX2;
	double *pdTopX3;
	double *pdTopX4;
	double *pdTopY1;
	double *pdTopY2;
	double *pdTopY3;
	double *pdTopY4;
	double *pdSurface;
	double *pdSurface_Eps;
	double *pdSurfaceX;
	double *pdSurfaceY;
	double	dXkoefProp;
	double	dYkoefProp;
	double	dXmin;
	double	dXmax;
	double	dYmin;
	double	dYmax;
	double	dZmin;
	double	dZmin_Eps;
	double	dZmax;
	double	dZmax_Eps;
	double	dDrawFieldValueX;
	double	dDrawFieldValueY;
	int		nDrawFieldValue1;
	int		nDrawFieldValue2;
	int		nDrawFieldValue3;
	int		nDrawFieldValue4;
	int		nDrawFieldValue5;
	BOOL	bXsizeFlag;
	BOOL	bYsizeFlag;
	BOOL	bZsizeFlag;
	BOOL	bZsizeFlag_Eps;
	void DrawField( void );
	BOOL	bDrawFieldModulFlag;
	BOOL	bDrawFieldValueFlag;
	BOOL	bHightColorRezolution;
	BOOL	bDrawFlagAxiesXSize;
	BOOL	bDrawFlagAxiesYSize;
	BOOL	bDrawFlagAxiesZFieldSize;
	BOOL	bDrawFlagAxiesZEpsSize;
	BOOL	bDrawFlagAxiesZFieldSize_Eps;
	BOOL	bDrawFlagAxies;
	BOOL	bDrawFlag;
	BOOL	bDrawSurfaceFlag;
	BOOL	bDrawSurfaceFlag_Eps;
	BOOL	bDrawLineFlag;
	BOOL	bDrawLineFlag_Eps;
	BOOL	bDrawFlagTopology;
	BOOL	bDrawSurfaceFlagTopology;
	BOOL	bDrawLineFlagTopology;
	BOOL	bTopologySizeFlag;
	BOOL	bTopologySizeFlagLinkList;
	BOOL	bDrawFlagMagnetic;
	BOOL	bDrawFlagMetal;
	BOOL	bDrawFlagEps;
	BOOL	bDrawFlagInput;
	BOOL	bDrawFlagAbsorber;
	GLfloat	wAngleY;
    GLfloat	wAngleX;
    GLfloat	wAngleZ;
	GLfloat	wTranslateX;
	GLfloat	wTranslateY;
	GLfloat	wTranslateZ;
	HGLRC	hrc;		
	BOOL	m_play;
	float	m_fRadius;
	CRect	m_oldRect;
	CPalette	*m_pOldPalette;
	CPalette	 m_cPalette;
	CClientDC	*m_pDC;
	void GL_Init( void );
	GLvoid GL_Resize( void );
	BOOL bSetupPixelFormat( void );
	unsigned char ComponentFromIndex(int i, UINT nbits, UINT shift);
	void CreateRGBPalette( void );
	void DrawScene(void);
	void GL_Close( void );
	CTmcGLText *pcGLText;
	int nPropertyPageInd;
	CColorLevelDlg *pcColorLevelDialog;
public:
	afx_msg void OnFieldL();
	COLORREF * GetpscSurfaceColor(void);
	BOOL * GetbHightColorRezolution(void);
	afx_msg void OnFieldValue();
	afx_msg void OnUpdateFieldValue(CCmdUI *pCmdUI);
private:
	int GetNumberElement(double dX, double dY);
public:
	afx_msg void OnFieldModul();
	afx_msg void OnUpdateFieldModul(CCmdUI *pCmdUI);
};

#ifndef _DEBUG  // debug version in FldViewView.cpp
inline CFldViewDoc* CFldViewView::GetDocument()
   { return (CFldViewDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLDVIEWVIEW_H__9FEC5737_707F_11D3_A93C_E0AB4DC10000__INCLUDED_)
