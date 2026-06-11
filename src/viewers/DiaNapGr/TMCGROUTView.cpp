// TMCGROUTView.cpp : implementation of the CTMCGROUTView class
//

#include "stdafx.h"
#include "TMCGROUT.h"

#include <tmcgrviw.h>
#include "MainFrm.h"
#include "..\\DiaNapr\\c2DArray.h"
#include "TMCGROUTDoc.h"
#include "TMCGROUTView.h"
#include "TMCGROUTDIALOGView.h"
#include "DialogDoc.h"
#include "DialogTtoS.h"
#include "TmcGroutColorGraphDialog.h"
#include "ThreadCalcDirPat.h"

UINT ReadDocFileW_Thread1( LPVOID pParam );


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTView

IMPLEMENT_DYNCREATE(CTMCGROUTView, CScrollView)

BEGIN_MESSAGE_MAP(CTMCGROUTView, CScrollView)
	//{{AFX_MSG_MAP(CTMCGROUTView)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_EDIT_EDIT, OnEditEdit)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_EDIT_GRAPHICSPARAMETERS, OnEditGraphicsparameters)
	ON_COMMAND(ID_VIEW_GRAPHICS, OnViewGraphics)
	ON_COMMAND(ID_EDIT_DOCUMENT, OnEditDocument)
	ON_WM_TIMER()
	ON_COMMAND(ID_EDIT_ADDCHARACTERISTICS, OnEditAddcharacteristics)
	ON_COMMAND(ID_VIEW_RESIZECTRLR, OnViewResizectrlr)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_VIEW_RESIZEWINDOW, OnViewResizewindow)
	ON_COMMAND(ID_CONFIG_EDITOR, OnConfigEditor)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_CONFIG_FONT, OnConfigFont)
	ON_COMMAND(ID_CONFIG_COLORPOINT, OnConfigColorpoint)
	ON_COMMAND(ID_CONFIG_COLOR_AXIS, OnConfigColorAxis)
	ON_COMMAND(ID_CONFIG_COLOR_GRID, OnConfigColorGrid)
	ON_COMMAND(ID_CONFIG_COLOR_BACKGROUND, OnConfigColorBackground)
	ON_COMMAND(ID_EDIT_TOSMATRIX, OnEditTosmatrix)
	ON_COMMAND(ID_VIEW_CHANGEXMAXXMIN_DECRIMENT, OnViewChangexmaxxminDecriment)
	ON_COMMAND(ID_VIEW_CHANGEXMAXXMIN_INCREMENT, OnViewChangexmaxxminIncrement)
	ON_COMMAND(ID_VIEW_CHANGEYMAXYMIN_DECREMENT, OnViewChangeymaxyminDecrement)
	ON_COMMAND(ID_VIEW_CHANGEYMAXYMIN_INCREMENT, OnViewChangeymaxyminIncrement)
	ON_COMMAND(ID_VIEW_AUTOXSIZE, OnViewAutoxsize)
	ON_UPDATE_COMMAND_UI(ID_VIEW_AUTOXSIZE, OnUpdateViewAutoxsize)
	ON_COMMAND(ID_VIEW_AUTOYSIZE, OnViewAutoysize)
	ON_UPDATE_COMMAND_UI(ID_VIEW_AUTOYSIZE, OnUpdateViewAutoysize)
	ON_COMMAND(ID_VIEW_ZOOM_ZOOMP, OnViewZoomZoomp)
	ON_COMMAND(ID_VIEW_ZOOM_ZOOM, OnViewZoomZoom)
	ON_COMMAND(ID_VIEW_ZOOM_ZOOMXP, OnViewZoomZoomxp)
	ON_COMMAND(ID_VIEW_ZOOM_ZOOMYP, OnViewZoomZoomyp)
	ON_COMMAND(ID_VIEW_ZOOM_ZOOMX, OnViewZoomZoomx)
	ON_COMMAND(ID_VIEW_ZOOM_ZOOMY, OnViewZoomZoomy)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_CONFIG_COLOR_GRAPHICS, OnConfigColorGraphics)
	ON_COMMAND(ID_VIEW_TRANSLATE_CHANGEXMAXXMIN_END, OnViewTranslateChangexmaxxminEnd)
	ON_COMMAND(ID_VIEW_TRANSLATE_CHANGEXMAXXMIN_HOME, OnViewTranslateChangexmaxxminHome)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTView construction/destruction



CTMCGROUTView::CTMCGROUTView()
{
	tmcgrwin.psGraph = NULL;
	tmcgrwin.nGraph = 0;
	tmcgrwin.nYType = TMC_GROTS_TYPE_FULL;
	tmcgrwin.nXType	= TMC_GROTS_TYPE_nT;
	tmcgrwin.sGrView.nXmin = 2000;
	tmcgrwin.sGrView.nXmax = TMC_VIEW_XSIZE;
	tmcgrwin.sGrView.nYmin = 2000;
	tmcgrwin.sGrView.nYmax = TMC_VIEW_YSIZE;
	tmcgrwin.sGrWin.Xmin = 1.0;
	tmcgrwin.sGrWin.Xmax = 10.0;
	tmcgrwin.sGrWin.Ymin = 1.0;
	tmcgrwin.sGrWin.Ymax = 1.2;
	tmcgrwin.sGrWin.nAFlagX = TRUE;
	tmcgrwin.sGrWin.nAFlagY = TRUE;
	tmcgrwin.PointDrawFlag = TRUE;

	tmcgrwin1.psGraph = NULL;
	tmcgrwin1.nGraph = 0;
	tmcgrwin1.nYType = TMC_GROTS_TYPE_FULL;
	tmcgrwin1.nXType	= TMC_GROTS_TYPE_nT;
	tmcgrwin1.sGrView.nXmin = 2000;
	tmcgrwin1.sGrView.nXmax = TMC_VIEW_XSIZE;
	tmcgrwin1.sGrView.nYmin = 2000;
	tmcgrwin1.sGrView.nYmax = TMC_VIEW_YSIZE;
	tmcgrwin1.sGrWin.Xmin = 1.0;
	tmcgrwin1.sGrWin.Xmax = 10.0;
	tmcgrwin1.sGrWin.Ymin = 1.0;
	tmcgrwin1.sGrWin.Ymax = 1.2;
	tmcgrwin1.sGrWin.nAFlagX = TRUE;
	tmcgrwin1.sGrWin.nAFlagY = TRUE;
	tmcgrwin1.PointDrawFlag = TRUE;

	bLossPoint = TRUE;
	bIsDataCalculate = FALSE;
	SetRectOutXY();
	MouseXCoord = tmcgrwin.sGrWin.Xmin;
	MouseYCoord = tmcgrwin.sGrWin.Ymin;
	FlagDrawMouseCoord     = FALSE;
	FlagDrawMouseCoordMove = FALSE;
	csXAxiesFormat.Format("%s", "%4.3lg");
	csYAxiesFormat.Format("%s", "%4.3lg");
	dPointSize = 1.0;

	gr_typX = new TMC_GR_TYPE_X[ 7 ];
	gr_typX[0].nType = TMC_GROTS_TYPE_nT;
	strcpy( gr_typX[0].pszXname, "Angle radian" );
	gr_typX[1].nType = TMC_GROTS_TYPE_s;
	strcpy( gr_typX[1].pszXname, "Time sec" );
	gr_typX[2].nType = TMC_GROTS_TYPE_ms;
	strcpy( gr_typX[2].pszXname, "Time msec" );
	gr_typX[3].nType = TMC_GROTS_TYPE_mks;
	strcpy( gr_typX[3].pszXname, "Time mksec" );
	gr_typX[4].nType = TMC_GROTS_TYPE_ns;
	strcpy( gr_typX[4].pszXname, "Time nsec" );
	gr_typX[5].nType = TMC_GROTS_TYPE_ps;
	strcpy( gr_typX[5].pszXname, "Angle degree" );
	gr_typX[6].nType = -1;
	gr_typX[6].pszXname[0] = '\0';

	gr_typY = new TMC_GR_TYPE_Y[ 8 ];
	gr_typY[0].nType = TMC_GROTS_TYPE_FULL;
	strcpy( gr_typY[0].pszYname, "Pattern" );
	gr_typY[1].nType = TMC_GROTS_TYPE_AMPL;
	strcpy( gr_typY[1].pszYname, "Pattern dB" );
	gr_typY[2].nType = TMC_GROTS_TYPE_WSVR;
	strcpy( gr_typY[2].pszYname, "WSVR" );
	gr_typY[3].nType = TMC_GROTS_TYPE_LDB;
	strcpy( gr_typY[3].pszYname, "L dB" );
	gr_typY[4].nType = TMC_GROTS_TYPE_PHAS_R;
	strcpy( gr_typY[4].pszYname, "Pattern" );
	gr_typY[5].nType = TMC_GROTS_TYPE_PHAS_G;
	strcpy( gr_typY[5].pszYname, "Pattern dB"  );
	gr_typY[6].nType = TMC_GROTS_TYPE_PHASD;
	strcpy( gr_typY[6].pszYname, "Delta Phase" );
	gr_typY[7].nType = -1;
	gr_typY[7].pszYname[0] = '\0';

	FlagResizeInit = TRUE;

	// TODO: add construction code here

}

CTMCGROUTView::~CTMCGROUTView()
{
	int i;
	if(tmcgrwin.psGraph != NULL)
	{
		for( i = 0; i < tmcgrwin.nGraph; i++)
		{
			if( tmcgrwin.psGraph[i].pPoint  != NULL ) delete  tmcgrwin.psGraph[i].pPoint;
			if( tmcgrwin.psGraph[i].piPoint != NULL ) delete  tmcgrwin.psGraph[i].piPoint;
		};
		delete tmcgrwin.psGraph;
	};
	if(tmcgrwin1.psGraph != NULL)
	{
		for( i = 0; i < tmcgrwin1.nGraph; i++)
		{
			if( tmcgrwin1.psGraph[i].pPoint  != NULL ) delete  tmcgrwin1.psGraph[i].pPoint;
			if( tmcgrwin1.psGraph[i].piPoint != NULL ) delete  tmcgrwin1.psGraph[i].piPoint;
		};
		delete tmcgrwin1.psGraph;
	};
	delete gr_typX;
	delete gr_typY;
}

BOOL CTMCGROUTView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTView drawing

void CTMCGROUTView::OnDraw(CDC* pDC)
{
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CString text;

	if( FlagResizeInit )
	{
		FlagResizeInit = FALSE;
		OnViewResizectrlr();
	};

	OnDrawBackground( pDC );

	if( !bIsDataCalculate )
	{
		text.Format( "Please wait ... Data calculate");
		VTextOut( pDC, 10,00,text);
		return;
	};

	if( pDoc->error[0] != '\0')
	{
		text.Format( "Error : %s" , pDoc->error);
		VTextOut( pDC, 10,00,text);
		return;
	};
	OnDrawMouseResize( pDC );
	onDrawGrid( pDC );
	onDrowAxies( pDC );
	OnDrawGraph( pDC );
	OnDrawMouseScrol( pDC );
	// TODO: add draw code for native data here
}

void CTMCGROUTView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	CSize sizeTotal( (int)(TMC_VIEW_XSIZE*1.35), (int)(TMC_VIEW_YSIZE*1.05) );
	CSize sizePage(sizeTotal.cx/2, sizeTotal.cy/2);
	CSize sizeLine(sizeTotal.cx/50, sizeTotal.cy/50);
	// TODO: calculate the total size of this view
	SetScrollSizes(TMC_VIEW_SIZEMODE, sizeTotal, sizePage, sizeLine);
	CTMCGROUTDoc* pDoc = GetDocument();		// set view graphics parameters as write document file
	ASSERT_VALID(pDoc);
	pDoc->ReadGraphParametersDefault();
	pDoc->ReadGraphParameters();

	ReadDocFileW();			// read data document 
	tmcgrwin.PointDrawFlag = pDoc->grdoc.PointDrawFlag;
	tmcgrwin.nXType = pDoc->grdoc.nXType;
	tmcgrwin.nYType = pDoc->grdoc.nYType;
	tmcgrwin.sGrWin.Xmin = pDoc->grdoc.Xmin;
	tmcgrwin.sGrWin.Xmax = pDoc->grdoc.Xmax;
	tmcgrwin.sGrWin.Ymin = pDoc->grdoc.Ymin;
	tmcgrwin.sGrWin.Ymax = pDoc->grdoc.Ymax;
	tmcgrwin.sGrWin.nAFlagX = pDoc->grdoc.nAFlagX;
	tmcgrwin.sGrWin.nAFlagY = pDoc->grdoc.nAFlagY;
	bLossPoint = pDoc->bLossPoint;

	{
		int nXX, nYY;
		CString csBuffer;
		double dBuf;
		csBuffer = AfxGetApp()->GetProfileString( TMC_GROTS_DOCFILE_ID, TMC_GROUT_DOCFILE_ID_INIWNDVWPRT );
		if( csBuffer.GetLength() != 0 )
		{
			if( sscanf( csBuffer, "%d %d", &(nXX), &(nYY) ) == 2 )
			{
				tmcgrwin.sGrView.nXmax = nXX;
				tmcgrwin.sGrView.nYmax = nYY;
				FlagResizeInit = FALSE;
			};
		};
		csBuffer = AfxGetApp()->GetProfileString( TMC_GROTS_DOCFILE_ID, TMC_GROTS_DOCFILE_XAXIESFORMAT );
		if( csBuffer.GetLength() != 0 )
		{
			csXAxiesFormat = csBuffer;
		};
		csBuffer = AfxGetApp()->GetProfileString( TMC_GROTS_DOCFILE_ID, TMC_GROTS_DOCFILE_YAXIESFORMAT );
		if( csBuffer.GetLength() != 0 )
		{
			csYAxiesFormat = csBuffer;
		};
		csBuffer = AfxGetApp()->GetProfileString( TMC_GROTS_DOCFILE_ID, TMC_GROTS_DOCFILE_POINTSIZE );
		if( csBuffer.GetLength() != 0 )
		{
			if( sscanf( csBuffer, "%lg", &(dBuf) ) == 1 )
			{
				dPointSize = dBuf;
			};
		};
	};

  	PrepareDoubleGraph();	// copy graph doc->view
	PrepareLogGraph();		// calculation double coord -> logic coord
	SetRectOutXY();	
	
	SetTimer( 1, 3000000, NULL);


	//	InitTmcGrWin(pDoc, &tmcgrwin);

}

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTView printing

BOOL CTMCGROUTView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CTMCGROUTView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CTMCGROUTView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTView diagnostics

#ifdef _DEBUG
void CTMCGROUTView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CTMCGROUTView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CTMCGROUTDoc* CTMCGROUTView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTMCGROUTDoc)));
	return (CTMCGROUTDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTView message handlers

void CTMCGROUTView::onDrowAxies( CDC *pDC )
{

   CTMCGROUTDoc* pDoc = GetDocument();
   ASSERT_VALID(pDoc);
   CString text;
   int i, j;
   CPen newPen(PS_SOLID, 50, pDoc->scAxiesColor );
   CPen *pOldPen = pDC->SelectObject(&newPen);

   pDC->MoveTo( tmcgrwin.sGrView.nXmin, -tmcgrwin.sGrView.nYmax );
   pDC->LineTo( tmcgrwin.sGrView.nXmax+1200, -tmcgrwin.sGrView.nYmax );
   pDC->LineTo( tmcgrwin.sGrView.nXmax+1200-300, -tmcgrwin.sGrView.nYmax-200 );
   pDC->MoveTo( tmcgrwin.sGrView.nXmax+1200, -tmcgrwin.sGrView.nYmax );
   pDC->LineTo( tmcgrwin.sGrView.nXmax+1200-300, -tmcgrwin.sGrView.nYmax+200 );

   for( i = 0, j = 0; (gr_typX[i].nType != -1) ; i++) 
   {
	   if(tmcgrwin.nXType == gr_typX[i].nType ) j = i;
   };
   text.Format("%s", gr_typX[j].pszXname );
   VTextOut( pDC, tmcgrwin.sGrView.nXmax+1200, -tmcgrwin.sGrView.nYmax+700,text);
	
   pDC->MoveTo( tmcgrwin.sGrView.nXmin, -tmcgrwin.sGrView.nYmax );
   pDC->LineTo( tmcgrwin.sGrView.nXmin, -tmcgrwin.sGrView.nYmin+1000 );
   pDC->LineTo( tmcgrwin.sGrView.nXmin-200, -tmcgrwin.sGrView.nYmin+1000-500 );
   pDC->MoveTo( tmcgrwin.sGrView.nXmin, -tmcgrwin.sGrView.nYmin+1000 );
   pDC->LineTo( tmcgrwin.sGrView.nXmin+200, -tmcgrwin.sGrView.nYmin+1000-500 );

   for( i = 0, j = 0; (gr_typY[i].nType != -1) ; i++) 
   {
	   if(tmcgrwin.nYType == gr_typY[i].nType ) j = i;
   };
   text.Format("%s", gr_typY[j].pszYname );
   VTextOut( pDC, tmcgrwin.sGrView.nXmin+500, -tmcgrwin.sGrView.nYmin+1000,text);
	

   pDC->SelectObject(pOldPen);
}

void CTMCGROUTView::onDrawGrid( CDC *pDC )
{
   CTMCGROUTDoc* pDoc = GetDocument();
   ASSERT_VALID(pDoc);
   int j, i, step;
   int NumX = 10, NumY = 10;
   CString text;
   
   double x, xstep;
   CPen newPen(PS_DOT, 1, pDoc->scGridColor);
   CPen *pOldPen = pDC->SelectObject(&newPen);

   for( j = 0, x = tmcgrwin.sGrWin.Xmin, xstep = (tmcgrwin.sGrWin.Xmax - tmcgrwin.sGrWin.Xmin)/(NumX),i = tmcgrwin.sGrView.nXmin, step = (tmcgrwin.sGrView.nXmax - tmcgrwin.sGrView.nXmin)/(NumX); j <= NumX ; i += step, x += xstep, j++)
   {
	  pDC->MoveTo( i, -tmcgrwin.sGrView.nYmin );
	  pDC->LineTo( i, -tmcgrwin.sGrView.nYmax );
//	  text.Format("%4.3lg", x );
	  text.Format( csXAxiesFormat, x );
	  VTextOut( pDC, i, -tmcgrwin.sGrView.nYmax,text);

   };

   for( j = 0, x = tmcgrwin.sGrWin.Ymax, xstep = (tmcgrwin.sGrWin.Ymax - tmcgrwin.sGrWin.Ymin)/(NumY), i = tmcgrwin.sGrView.nYmin, step = (tmcgrwin.sGrView.nYmax - tmcgrwin.sGrView.nYmin)/(NumY); j <= NumY ; i += step, x -= xstep, j++)
   {
//	  pDC->MoveTo( tmcgrwin.sGrView.nXmin, -i );
	  pDC->MoveTo( 100, -i );
	  pDC->LineTo( tmcgrwin.sGrView.nXmax, -i );
//	  text.Format("%4.3lg", x );
	  text.Format( csYAxiesFormat, x );
//	  VTextOut( pDC, tmcgrwin.sGrView.nXmin - 1000, -i + 500,text);
	  VTextOut( pDC,  100, -i + 500,text);
   };

   pDC->SelectObject(pOldPen);

   /*

	GetClientRect(rectClient);
	text.Format("Error cod = %d", pDoc->nErrorCod );
	VTextOut( pDC, 10,50,text);
	text.Format("x = {%s}; y = {%s}", pDoc->pszNameX, pDoc->pszNameY );
	VTextOut( pDC, 10,100,text);
	if(pDoc->ppPointArray == NULL) return;
	pDC->MoveTo((int)(pDoc->ppPointArray[0].x), (int)(pDoc->ppPointArray[0].y));
	for( i = 1; i < pDoc->nSizePoints; i++)
	{
	pDC->LineTo((int)(pDoc->ppPointArray[i].x), (int)(pDoc->ppPointArray[i].y));
	};
  */
	// TODO: add draw code for native data here




}

void CTMCGROUTView::OnRButtonDown(UINT nFlags, CPoint point) 
{

	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);  

	
	if( (point.x > tmcgrwin.sGrView.nXmin)&&(point.x < tmcgrwin.sGrView.nXmax)&&(-point.y > tmcgrwin.sGrView.nYmin)&&(-point.y < tmcgrwin.sGrView.nYmax) )
	{
		OnEditGraphicsparameters();
	}
	else
	{
		OnEditDocument();
	}

	CScrollView::OnRButtonDown(nFlags, point);
}

void CTMCGROUTView::OnEditEdit() 
{
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	char szEditorName[TMC_GROUT_MAXSTRING_BUF];
	strcpy( szEditorName, pDoc->csEditorName );
	pDoc->RunExeFile( szEditorName );
	
}

void CTMCGROUTView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);  
	MouseResize.left = point.x;
	MouseResize.top  = point.y;
	MouseResize.right  = point.x;
	MouseResize.bottom = point.y;
	
	if( (point.x > tmcgrwin.sGrView.nXmin)&&(point.x < tmcgrwin.sGrView.nXmax)&&(-point.y > tmcgrwin.sGrView.nYmin)&&(-point.y < tmcgrwin.sGrView.nYmax) )
	{
		MouseXCoord = LXCordToDoublX( point.x );
		MouseYCoord = LYCordToDoublY( point.y );
		PutStatistics1();
		PutTrace1();
		FlagDrawMouseCoord     = TRUE;
		FlagDrawMouseCoordMove = TRUE;
		SetCapture();
	}
	else
	{
		FlagDrawMouseCoord = FALSE;
	};
//	IvalidateRectView( rectOutXY );
	CScrollView::OnLButtonDown(nFlags, point);
}


void CTMCGROUTView::SetRectOutXY()
{
	CRect rectOut( tmcgrwin.sGrView.nXmax+1200, -tmcgrwin.sGrView.nYmax+1000, tmcgrwin.sGrView.nXmax+9200, -tmcgrwin.sGrView.nYmax+3000 );
	rectOutXY = rectOut;
}

void CTMCGROUTView::OnDrawMouseScrol(CDC* pDC)
{
/*	int i, j;
	CString text;

	if( FlagDrawMouseCoord )
	{
		for( i = 0, j = 0; (gr_typX[i].nType != -1) ; i++) 
		{
		   if(tmcgrwin.nXType == gr_typX[i].nType ) j = i;
		};
	   text.Format("%s = %lg", gr_typX[j].pszXname, MouseXCoord );
	   VTextOut( pDC,  rectOutXY.TopLeft().x, rectOutXY.BottomRight().y,text);

	   for( i = 0, j = 0; (gr_typY[i].nType != -1) ; i++) 
	   {
		   if(tmcgrwin.nYType == gr_typY[i].nType ) j = i;
	   };
	   text.Format("%s = %lg", gr_typY[j].pszYname, MouseYCoord );
	   VTextOut( pDC, rectOutXY.TopLeft().x, rectOutXY.BottomRight().y - 700,text);
	}
	else
	{
		pDC->FillSolidRect( &rectOutXY, pDC->GetBkColor() );
	};
*/
}


double CTMCGROUTView::LXCordToDoublX( int x )
{
	double x1;
	x1 = tmcgrwin.sGrWin.Xmin + (tmcgrwin.sGrWin.Xmax-tmcgrwin.sGrWin.Xmin)*(x - (double)tmcgrwin.sGrView.nXmin)/(tmcgrwin.sGrView.nXmax-(double)tmcgrwin.sGrView.nXmin);
	if( x1 < tmcgrwin.sGrWin.Xmin )	   x1 = tmcgrwin.sGrWin.Xmin;
	if( x1 > tmcgrwin.sGrWin.Xmax )	   x1 = tmcgrwin.sGrWin.Xmax;
	return x1;
}

double CTMCGROUTView::LYCordToDoublY( int y )
{
	double y1;
	y1 = tmcgrwin.sGrWin.Ymin + (tmcgrwin.sGrWin.Ymax-tmcgrwin.sGrWin.Ymin)*(1+(y + (double)tmcgrwin.sGrView.nYmin)/(tmcgrwin.sGrView.nYmax-(double)tmcgrwin.sGrView.nYmin));
	if( y1 < tmcgrwin.sGrWin.Ymin )	   y1 = tmcgrwin.sGrWin.Ymin;
	if( y1 > tmcgrwin.sGrWin.Ymax )	   y1 = tmcgrwin.sGrWin.Ymax;
	return y1;
}

int CTMCGROUTView::DoublYCordToLY( double y )
{
	int y1;
	y1 = (int)( -tmcgrwin.sGrView.nYmax + (tmcgrwin.sGrView.nYmax-tmcgrwin.sGrView.nYmin)*(y - tmcgrwin.sGrWin.Ymin)/(tmcgrwin.sGrWin.Ymax-tmcgrwin.sGrWin.Ymin) );
	if( y1 > -tmcgrwin.sGrView.nYmin )   y1 = -tmcgrwin.sGrView.nYmin;
	if( y1 < -tmcgrwin.sGrView.nYmax )   y1 = -tmcgrwin.sGrView.nYmax;
	return y1;
}

int CTMCGROUTView::DoublXCordToLX( double x )
{
	int x1;
	x1 = (int)( tmcgrwin.sGrView.nXmin + (tmcgrwin.sGrView.nXmax-tmcgrwin.sGrView.nXmin)*(x - tmcgrwin.sGrWin.Xmin)/(tmcgrwin.sGrWin.Xmax-tmcgrwin.sGrWin.Xmin) );
	if( x1 < tmcgrwin.sGrView.nXmin )   x1 = tmcgrwin.sGrView.nXmin;
	if( x1 > tmcgrwin.sGrView.nXmax )   x1 = tmcgrwin.sGrView.nXmax;
	return x1;
}


void CTMCGROUTView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if( pDoc->IsDataModific() )	// if document file or S-matrix is modific
	{
		ReadDocFileW();			// read data document 
		RedrawViewGr();
	};
	

	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);  
	
	if( (point.x > tmcgrwin.sGrView.nXmin)&&(point.x < tmcgrwin.sGrView.nXmax)&&(-point.y > tmcgrwin.sGrView.nYmin)&&(-point.y < tmcgrwin.sGrView.nYmax) )
	{
		OnViewResizewindow();
	}
	else
	{
		OnViewResizectrlr();
		WriteIniWndPar();
	};
	CScrollView::OnLButtonDblClk(nFlags, point);
}

void CTMCGROUTView::ReadDocFileW()
{
	if( bIsDataCalculate ) return;

	bIsDataCalculate = FALSE;

	RedrawWindow();

	CThreadCalcDirPat cDlg;

	cDlg.pCView = (void *)(this);
	time( &ltime_start );

	if( ( AfxBeginThread( ReadDocFileW_Thread1, (void *)(this), THREAD_PRIORITY_NORMAL ) ) == NULL )
	{
		CString csError;
		csError.Format("Error when open thread for optimization All Partitional");
		CTMCGROUTDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		sprintf( pDoc->error, "%s", csError );
		return;
	};
		
	cDlg.DoModal();
	
	return;
}

void CTMCGROUTView::PrepareDoubleGraph()
{
	if( !bIsDataCalculate ) return;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if( tmcgrwin.psGraph == NULL ) return;
	PutStatistics1();

	int i, j, ii, iph;
	double r11, r12, r13, r14, freq, s11;
	double xMin, yMin, xMax, yMax;

	freq = pDoc->dFreq;

	for( i = 0, xMin = yMin = FLT_MAX, xMax = yMax = -FLT_MAX; i < tmcgrwin.nGraph; i++)
	{
	if(pDoc->grdoc.psGraph[i].OutFlag)
	{
		for( j = 0, ii = 0, r11 = 0, r12 = 0, r13 = 0, r14 = 0, iph = 0; j < tmcgrwin.psGraph[i].nPoint; j++)
		{
			switch(tmcgrwin.nXType)
			{
			case TMC_GROTS_TYPE_nT:
				tmcgrwin.psGraph[i].pPoint[j].x = pDoc->grdoc.psGraph[i].pFreq[j];
				break;
			case TMC_GROTS_TYPE_ps:
				tmcgrwin.psGraph[i].pPoint[j].x = pDoc->grdoc.psGraph[i].pFreq[j]*180./3.141592653589;
				;
				break;
			case TMC_GROTS_TYPE_ns:
				tmcgrwin.psGraph[i].pPoint[j].x = pDoc->grdoc.psGraph[i].pFreq[j]*(1.e+9);
				;
				break;
			case TMC_GROTS_TYPE_mks:
				tmcgrwin.psGraph[i].pPoint[j].x = pDoc->grdoc.psGraph[i].pFreq[j]*(1.e+6);
				;
				break;
			case TMC_GROTS_TYPE_ms:
				tmcgrwin.psGraph[i].pPoint[j].x = pDoc->grdoc.psGraph[i].pFreq[j]*(1.e+3);
				;
				break;
			case TMC_GROTS_TYPE_s:
				tmcgrwin.psGraph[i].pPoint[j].x = pDoc->grdoc.psGraph[i].pFreq[j]*(1.);
				;
				break;
			default:
				tmcgrwin.psGraph[i].pPoint[j].x = j;
				break;
			}
			if( xMin > tmcgrwin.psGraph[i].pPoint[j].x ) xMin = tmcgrwin.psGraph[i].pPoint[j].x;
			if( xMax < tmcgrwin.psGraph[i].pPoint[j].x ) xMax = tmcgrwin.psGraph[i].pPoint[j].x;

			switch(tmcgrwin.nYType)
			{
			case TMC_GROTS_TYPE_LDB:
				if(pDoc->grdoc.psGraph[i].pcExpr == NULL )
				{
					tmcgrwin.psGraph[i].pPoint[j].y = r14;
					if( j == 2 )
					{
						r11 = (double)( pDoc->grdoc.psGraph[i].pSmatr[0].x );
						r12 = (double)( pDoc->grdoc.psGraph[i].pSmatr[1].x );
						r13 = (double)( pDoc->grdoc.psGraph[i].pSmatr[2].x );
					};
					if( j > 2 )
					{
						r11 = r12;
						r12 = r13;
						r13 = (double)( pDoc->grdoc.psGraph[i].pSmatr[j].x );
						if( (r12-r11)*(r13-r12) < 0.0 )
						{
							r14 = fabs( r12 );
							ii = 1;
						}
						else
						{
							if( (fabs(r11)+fabs(r12)+fabs(r13)) < 3*DBL_MIN )
							{
								r14 = fabs( r12 );
								ii = 1;
							};
						};
						if( ii == 0 ) r14 = fabs( r12 );
					};
				}
				else
				{
					tmcgrwin.psGraph[i].pPoint[j].y = pDoc->grdoc.psGraph[i].pcExpr->GetExpressionValueL( j );
				};
				break;
			case TMC_GROTS_TYPE_WSVR:
				if(pDoc->grdoc.psGraph[i].pcExpr == NULL )
				{
					s11 = r14;
					if( fabs( s11 - 1.) < 0.0001	)
					{
						s11 = 0.9999;
					}
					tmcgrwin.psGraph[i].pPoint[j].y = (1.+s11)/(1.-s11);
					if( j == 2 )
					{
						r11 = (double)( pDoc->grdoc.psGraph[i].pSmatr[0].x );
						r12 = (double)( pDoc->grdoc.psGraph[i].pSmatr[1].x );
						r13 = (double)( pDoc->grdoc.psGraph[i].pSmatr[2].x );
					};
					if( j > 2 )
					{
						r11 = r12;
						r12 = r13;
						r13 = (double)( pDoc->grdoc.psGraph[i].pSmatr[j].x );
						if( (r12-r11)*(r13-r12) < 0.0 )
						{
							r14 = fabs( r12 );
							ii = 1;
						}
						else
						{
							if( (fabs(r11)+fabs(r12)+fabs(r13)) < 3*DBL_MIN )
							{
								r14 = fabs( r12 );
								ii = 1;
							};
						};
						if( ii == 0 ) r14 = fabs( r12 );
					};
				}
				else
				{
					tmcgrwin.psGraph[i].pPoint[j].y = pDoc->grdoc.psGraph[i].pcExpr->GetExpressionValueL( j );
				};
				break;
			case TMC_GROTS_TYPE_AMPL:
				if(pDoc->grdoc.psGraph[i].pcExpr == NULL )
				{
					r11 = (double)( pDoc->grdoc.psGraph[i].pSmatr[j].x * pDoc->pcDirectionalPattern[i].GetdMax() );
					tmcgrwin.psGraph[i].pPoint[j].y = 20.*log10( r11 );
				}
				else
				{
					tmcgrwin.psGraph[i].pPoint[j].y = pDoc->grdoc.psGraph[i].pcExpr->GetExpressionValueK( j );
				};
				break;
			default:
			case TMC_GROTS_TYPE_FULL:
				if(pDoc->grdoc.psGraph[i].pcExpr == NULL )
				{
					r11 = (double)( pDoc->grdoc.psGraph[i].pSmatr[j].x * pDoc->pcDirectionalPattern[i].GetdMax() );
					tmcgrwin.psGraph[i].pPoint[j].y = r11;
				}
				else
				{
					tmcgrwin.psGraph[i].pPoint[j].y = pDoc->grdoc.psGraph[i].pcExpr->GetExpressionValueK( j );
				};
				break;
			case TMC_GROTS_TYPE_PHAS_R:
				if(pDoc->grdoc.psGraph[i].pcExpr == NULL )
				{
					r11 = (double)( pDoc->grdoc.psGraph[i].pSmatr[j].x );
					tmcgrwin.psGraph[i].pPoint[j].y = r11;
				}
				else
				{
					tmcgrwin.psGraph[i].pPoint[j].y = pDoc->grdoc.psGraph[i].pcExpr->GetExpressionValueK( j );
				};
				break;
			case TMC_GROTS_TYPE_PHAS_G:
				if(pDoc->grdoc.psGraph[i].pcExpr == NULL )
				{
					r11 = (double)( pDoc->grdoc.psGraph[i].pSmatr[j].x );
					tmcgrwin.psGraph[i].pPoint[j].y = 20.*log10( r11 );
				}
				else
				{
					tmcgrwin.psGraph[i].pPoint[j].y = pDoc->grdoc.psGraph[i].pcExpr->GetExpressionValueK( j );
				};
				break;
/*			case TMC_GR_TYPE_SFD:
				if(pDoc->grdoc.psGraph[i].pcExpr == NULL )
				{
					tmcgrwin.psGraph[i].pPoint[j].y = 180./(3.141592653589)*cphase( pDoc->grdoc.psGraph[i].pSmatr[j] );
				}
				else
				{
					tmcgrwin.psGraph[i].pPoint[j].y = pDoc->grdoc.psGraph[i].pcExpr->GetExpressionValueSFD( j );
				};
				break;
*/	
			};

			if( (tmcgrwin.sGrWin.nAFlagX)||( (tmcgrwin.psGraph[i].pPoint[j].x >= tmcgrwin.sGrWin.Xmin)&&(tmcgrwin.psGraph[i].pPoint[j].x <= tmcgrwin.sGrWin.Xmax) ) )
			{
				if( yMin > tmcgrwin.psGraph[i].pPoint[j].y ) yMin = tmcgrwin.psGraph[i].pPoint[j].y;
				if( yMax < tmcgrwin.psGraph[i].pPoint[j].y ) yMax = tmcgrwin.psGraph[i].pPoint[j].y;
			};
			
		};
	};
	};

	if( tmcgrwin.sGrWin.nAFlagX )
	{
	tmcgrwin.sGrWin.Xmin = xMin;
	tmcgrwin.sGrWin.Xmax = xMax;
	};

	if( tmcgrwin.sGrWin.nAFlagY )
	{
	tmcgrwin.sGrWin.Ymin = yMin;
	tmcgrwin.sGrWin.Ymax = yMax;
	}

}

void CTMCGROUTView::PrepareLogGraph()
{
	int i, j, j1;
	double x, y;

	if( !bIsDataCalculate ) return;
	if( tmcgrwin.psGraph == NULL ) return;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);


	for( i = 0; i < tmcgrwin.nGraph; i++)
	{
		if(pDoc->grdoc.psGraph[i].OutFlag)
		{
			for( j = 0, tmcgrwin.psGraph[i].nPointDraw = 0; j < tmcgrwin.psGraph[i].nPoint; j++)
			{
				x = tmcgrwin.psGraph[i].pPoint[j].x;
				tmcgrwin.psGraph[i].piPoint[j].nX = DoublXCordToLX( x );
				y = tmcgrwin.psGraph[i].pPoint[j].y;
				tmcgrwin.psGraph[i].piPoint[j].nY = DoublYCordToLY( y );
//				tmcgrwin.psGraph[i].nPointDraw++;
	
				if( x < tmcgrwin.sGrWin.Xmin )
				{
//					tmcgrwin.psGraph[i].nPointDraw--;
					if( j > 0 )
					{
						tmcgrwin.psGraph[i].piPoint[j].nX = tmcgrwin.psGraph[i].piPoint[j-1].nX;
						tmcgrwin.psGraph[i].piPoint[j].nY = tmcgrwin.psGraph[i].piPoint[j-1].nY;
					}
					else
					{
						for( j1 = 0; j1 < tmcgrwin.psGraph[i].nPoint; j1++)
						{
							if( tmcgrwin.psGraph[i].pPoint[j1].x >= tmcgrwin.sGrWin.Xmin )
							{
								tmcgrwin.psGraph[i].piPoint[j].nX = DoublXCordToLX( tmcgrwin.psGraph[i].pPoint[j1].x );
								tmcgrwin.psGraph[i].piPoint[j].nY = DoublYCordToLY( tmcgrwin.psGraph[i].pPoint[j1].y );
								break;
							};
						};
					};
				}
				else
				{
					if( x > tmcgrwin.sGrWin.Xmax )
					{
//						tmcgrwin.psGraph[i].nPointDraw--;
						if( j > 0 )
						{
							tmcgrwin.psGraph[i].piPoint[j].nX = tmcgrwin.psGraph[i].piPoint[j-1].nX;
							tmcgrwin.psGraph[i].piPoint[j].nY = tmcgrwin.psGraph[i].piPoint[j-1].nY;
						};
					}
					else
					{
						if( tmcgrwin.psGraph[i].nPointDraw != 0 )
						{
							if( j < (tmcgrwin.psGraph[i].nPoint-1) )
							{
								if( tmcgrwin.psGraph[i].pPoint[j+1].x < tmcgrwin.sGrWin.Xmax )
								{
//									if( ((tmcgrwin.psGraph[i].piPoint[j].nY-tmcgrwin.psGraph[i].piPoint[j-1].nY) == 0) && ((tmcgrwin.psGraph[i].piPoint[j].nY-tmcgrwin.psGraph[i].piPoint[j+1].nY) == 0) )
									if( ((tmcgrwin.psGraph[i].piPoint[j].nY-tmcgrwin.psGraph[i].piPoint[j-1].nY) == 0) && ((tmcgrwin.psGraph[i].piPoint[j].nY-DoublYCordToLY( tmcgrwin.psGraph[i].pPoint[j+1].y )) == 0) )
									{
//										tmcgrwin.psGraph[i].nPointDraw--;
										;
									}
									else
									{
										tmcgrwin.psGraph[i].nPointDraw++;
									};
								}
								else
								{
									tmcgrwin.psGraph[i].nPointDraw++;
								};
							}
							else
							{
								tmcgrwin.psGraph[i].nPointDraw++;
							};
						}
						else
						{
							tmcgrwin.psGraph[i].nPointDraw++;
						};
					};
				};
			};
		};
	};

	if(tmcgrwin1.psGraph != NULL)
	{
		for( i = 0; i < tmcgrwin1.nGraph; i++)
		{
			if( tmcgrwin1.psGraph[i].pPoint  != NULL ) delete  tmcgrwin1.psGraph[i].pPoint;
			if( tmcgrwin1.psGraph[i].piPoint != NULL ) delete  tmcgrwin1.psGraph[i].piPoint;
		};
		delete tmcgrwin1.psGraph;
	};
	tmcgrwin1.nGraph = tmcgrwin.nGraph;
	tmcgrwin1.psGraph = new TMC_GR_VIEW[tmcgrwin1.nGraph];
	for( i = 0; i < tmcgrwin1.nGraph; i++)
	{
		tmcgrwin1.psGraph[i].nPoint = tmcgrwin.psGraph[i].nPointDraw;
		tmcgrwin1.psGraph[i].nPointDraw = tmcgrwin.psGraph[i].nPointDraw;
		if( tmcgrwin1.psGraph[i].nPoint <= 0 )
		{
			tmcgrwin1.psGraph[i].pPoint  = NULL;
			tmcgrwin1.psGraph[i].piPoint = NULL;
		}
		else
		{
			tmcgrwin1.psGraph[i].pPoint  = new     TMC_POINT[tmcgrwin1.psGraph[i].nPoint];
			tmcgrwin1.psGraph[i].piPoint = new TMC_INT_POINT[tmcgrwin1.psGraph[i].nPoint];
		};
	};



	for( i = 0; i < tmcgrwin.nGraph; i++)
	{
		if(pDoc->grdoc.psGraph[i].OutFlag)
		{
//			for( j = 0, j1 = 0; (j < tmcgrwin.psGraph[i].nPoint)&&(j1 < tmcgrwin1.psGraph[i].nPoint); j++)
			for( j = 0, j1 = 0; j < tmcgrwin.psGraph[i].nPoint; j++)
			{
				if( tmcgrwin.psGraph[i].pPoint[j].x < tmcgrwin.sGrWin.Xmin )
				{
					;
				}
				else
				{
					if( tmcgrwin.psGraph[i].pPoint[j].x > tmcgrwin.sGrWin.Xmax )
					{
						;
					}
					else
					{
						if( j1 == 0 )
						{
							tmcgrwin1.psGraph[i].piPoint[j1].nX = tmcgrwin.psGraph[i].piPoint[j].nX;
							tmcgrwin1.psGraph[i].piPoint[j1].nY = tmcgrwin.psGraph[i].piPoint[j].nY;
							j1++;
						}
						else
						{
							if( j < (tmcgrwin.psGraph[i].nPoint-1) )
							{
								if( tmcgrwin.psGraph[i].pPoint[j+1].x < tmcgrwin.sGrWin.Xmax )
								{
									if( ((tmcgrwin.psGraph[i].piPoint[j].nY-tmcgrwin.psGraph[i].piPoint[j-1].nY) == 0) && ((tmcgrwin.psGraph[i].piPoint[j].nY-tmcgrwin.psGraph[i].piPoint[j+1].nY) == 0) )
									{
										;
									}
									else
									{
										tmcgrwin1.psGraph[i].piPoint[j1].nX = tmcgrwin.psGraph[i].piPoint[j].nX;
										tmcgrwin1.psGraph[i].piPoint[j1].nY = tmcgrwin.psGraph[i].piPoint[j].nY;
										j1++;
									};
								}
								else
								{
									tmcgrwin1.psGraph[i].piPoint[j1].nX = tmcgrwin.psGraph[i].piPoint[j].nX;
									tmcgrwin1.psGraph[i].piPoint[j1].nY = tmcgrwin.psGraph[i].piPoint[j].nY;
									j1++;
								};
							}
							else
							{
								tmcgrwin1.psGraph[i].piPoint[j1].nX = tmcgrwin.psGraph[i].piPoint[j].nX;
								tmcgrwin1.psGraph[i].piPoint[j1].nY = tmcgrwin.psGraph[i].piPoint[j].nY;
								j1++;
							};
						};
					};
				};
			};
		};
	};

	return;

}

void CTMCGROUTView::RedrawViewGr()
{
  	PrepareDoubleGraph();	// copy graph doc->view
	PrepareLogGraph();		// calculation double coord -> logic coord

	RedrawWindow();

}

void CTMCGROUTView::OnDrawGraph(CDC *pDC)
{
	int i;
	if( tmcgrwin.psGraph == NULL ) return;

	for( i = 0; i < tmcgrwin1.nGraph; i++)
	{	
	  OnDrawGraph1( pDC, i);	   // draw line
	};
	for( i = 0; i < tmcgrwin1.nGraph; i++)
	{	
	  OnDrawGraph2( pDC, i);	   // draw point
	};
	for( i = 0; i < tmcgrwin1.nGraph; i++)
	{	
	  OnDrawGraph3( pDC, i);	   // draw point
	};
}

void CTMCGROUTView::OnDrawGraph1(CDC *pDC, int i)
{
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(!pDoc->grdoc.psGraph[i].OutFlag) return;

	int  j;
	if( tmcgrwin1.psGraph == NULL ) return;
	if( tmcgrwin1.psGraph[i].nPoint <= 0 ) return;
		
		for( j = 0, pDC->MoveTo(tmcgrwin1.psGraph[i].piPoint[0].nX, tmcgrwin1.psGraph[i].piPoint[0].nY); j < tmcgrwin1.psGraph[i].nPoint; j++)
		{
			OnDrawLine( pDC, i, tmcgrwin1.psGraph[i].piPoint[j].nX, tmcgrwin1.psGraph[i].piPoint[j].nY );
		};

}

void CTMCGROUTView::OnDrawGraph2(CDC *pDC, int i)
{
	int  j, ii;
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(!pDoc->grdoc.psGraph[i].OutFlag) return;

	if( tmcgrwin1.psGraph == NULL ) return;
	if( tmcgrwin1.psGraph[i].nPoint <= 0 ) return;
	
	if( bLossPoint )
	{
		if( tmcgrwin1.psGraph[i].nPointDraw < 0 ) tmcgrwin1.psGraph[i].nPointDraw = 0;
		ii = tmcgrwin1.psGraph[i].nPointDraw/20 + 1;
	}
	else
	{
		ii = 1;
	};

	for( j = 0; j < tmcgrwin1.psGraph[i].nPoint; j += ii)
	{
		OnDrawPoint( pDC,  i,  tmcgrwin1.psGraph[i].piPoint[j].nX, tmcgrwin1.psGraph[i].piPoint[j].nY );
	};

}

void CTMCGROUTView::OnDrawPoint( CDC *pDC, int i, int nX, int nY )
{
	int j1;
	if( tmcgrwin1.psGraph == NULL ) return;
	if(  !tmcgrwin.PointDrawFlag ) return;
	
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CPen newPen( pDoc->grdoc.psGraph[i].PointType, pDoc->grdoc.psGraph[i].PointWidth, (COLORREF) pDoc->scPointColor);
	CPen *pOldPen = pDC->SelectObject(&newPen);

	for( j1 = 0, pDC->MoveTo(nX + (int)(dPointSize*pDoc->grdoc.psGraph[i].piPoint[0].nX), nY + (int)(dPointSize*pDoc->grdoc.psGraph[i].piPoint[0].nY) ); j1 < TMC_GROUT_POINT_TYPE; j1++)
				pDC->LineTo(nX + (int)(dPointSize*pDoc->grdoc.psGraph[i].piPoint[j1].nX), nY + (int)(dPointSize*pDoc->grdoc.psGraph[i].piPoint[j1].nY) );

   pDC->SelectObject(pOldPen);


}

void CTMCGROUTView::OnDrawGraph3(CDC *pDC, int i)
{
	int nX, nY, nX1, nY1;
	CString text, text1;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(!pDoc->grdoc.psGraph[i].OutFlag) return;

	nX  = (int)(tmcgrwin.sGrView.nXmax*1.10);
	nX1 = (int)(tmcgrwin.sGrView.nXmax*1.15);
	nY  = (int)(-(i+1)*700);
	nY1 = (int)(-(i+0.5)*700);

		     pDC->MoveTo( nX, nY);
	OnDrawLine(  pDC, i, nX1, nY);
	OnDrawPoint( pDC, i,  nX, nY );
	OnDrawPoint( pDC, i, nX1, nY );

	if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
	{
	   text.Format("- %s"  ,	pDoc->grdoc.psGraph[i].szGrapPodp);
	}
	else
	{
	   text.Format("-%s;"  ,	pDoc->grdoc.psGraph[i].szGrapPodp);
	};

	if( pDoc->pcDirectionalPattern != NULL )
	{
		if( pDoc->pcDirectionalPattern[i].IsCalculateKip() )
		{
		   text1.Format(" Knd =%7.4lgdB; Emitted Power =%5.2lg;", pDoc->pcDirectionalPattern[i].GetKnd_dB(), pDoc->pcDirectionalPattern[i].GetKip());
		}
		else
		{
		   text1.Format(" Knd =%7.4lgdB; Kip =%5.2lg;", pDoc->pcDirectionalPattern[i].GetKnd_dB(), pDoc->pcDirectionalPattern[i].GetKip());
		};
	}
	else
	{
	   text1.Format(" ");
	};

	text += text1;

   VTextOut( pDC,  nX1+400, nY1,text);

}

void CTMCGROUTView::OnDrawLine( CDC *pDC, int i, int nX, int nY )
{
	if( tmcgrwin1.psGraph == NULL ) return;
	
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	int ii;

	if( pDoc->grdoc.psGraph[i].LineType == PS_SOLID ) 
	{
		ii = pDoc->grdoc.psGraph[i].LineWidth;
	}
	else 
	{
		ii = 1;
	};

	CPen newPen( pDoc->grdoc.psGraph[i].LineType, ii, (COLORREF) pDoc->grdoc.psGraph[i].LineColor);
	CPen *pOldPen = pDC->SelectObject(&newPen);
	
	pDC->LineTo( nX, nY);
	
	pDC->SelectObject(pOldPen);

}

void CTMCGROUTView::OnEditGraphicsparameters() 
{
	// TODO: Add your command handler code here
	CTMCGROUTDIALOGView dlg;
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	int ret;
	
	dlg.m_XMin = tmcgrwin.sGrWin.Xmin;
	dlg.m_XMax = tmcgrwin.sGrWin.Xmax;
	dlg.m_YMin = tmcgrwin.sGrWin.Ymin;
	dlg.m_YMax = tmcgrwin.sGrWin.Ymax;
	dlg.m_aXSFlag = tmcgrwin.sGrWin.nAFlagX;
	dlg.m_aYSFlag = tmcgrwin.sGrWin.nAFlagY;
	dlg.m_DrawPoinFlag  = tmcgrwin.PointDrawFlag;
	dlg.m_szDatFileName = pDoc->GetPathName();
	
	dlg.m_XUnit = tmcgrwin.nXType;
	dlg.m_YUnit = tmcgrwin.nYType;
	dlg.m_YNormirovkaFlag = FALSE;
	if( dlg.m_YUnit >= TMC_GROTS_TYPE_PHAS_R ) 
	{
		dlg.m_YNormirovkaFlag = TRUE;
		dlg.m_YUnit = dlg.m_YUnit - TMC_GROTS_TYPE_PHAS_R;
	};
	dlg.m_csXFormat = csXAxiesFormat;
	dlg.m_csYFormat = csYAxiesFormat;
	dlg.m_dPointSize = dPointSize;
	dlg.m_LossPoint = bLossPoint;

	if( (ret = dlg.DoModal()) == 1)
	{
	tmcgrwin.sGrWin.Xmin =	dlg.m_XMin;
	tmcgrwin.sGrWin.Xmax = dlg.m_XMax;
	tmcgrwin.sGrWin.Ymin = dlg.m_YMin;
	tmcgrwin.sGrWin.Ymax = dlg.m_YMax;
	tmcgrwin.sGrWin.nAFlagX = dlg.m_aXSFlag;
	tmcgrwin.sGrWin.nAFlagY = dlg.m_aYSFlag;
	tmcgrwin.PointDrawFlag = dlg.m_DrawPoinFlag;
	csXAxiesFormat = dlg.m_csXFormat;
	csYAxiesFormat = dlg.m_csYFormat;
	dPointSize = dlg.m_dPointSize;
	bLossPoint = dlg.m_LossPoint;
	AfxGetApp()->WriteProfileString( TMC_GROTS_DOCFILE_ID, TMC_GROTS_DOCFILE_XAXIESFORMAT, csXAxiesFormat);
	AfxGetApp()->WriteProfileString( TMC_GROTS_DOCFILE_ID, TMC_GROTS_DOCFILE_YAXIESFORMAT, csYAxiesFormat);
	{
		CString csBuffer;
		csBuffer.Format("%lg", dPointSize);
		AfxGetApp()->WriteProfileString( TMC_GROTS_DOCFILE_ID, TMC_GROTS_DOCFILE_POINTSIZE, csBuffer);
	};

	tmcgrwin.nXType = dlg.m_XUnit;
	tmcgrwin.nYType = dlg.m_YUnit;
	if( dlg.m_YNormirovkaFlag ) 
	{
		tmcgrwin.nYType = dlg.m_YUnit + TMC_GROTS_TYPE_PHAS_R;
	};
	
	pDoc->grdoc.PointDrawFlag = tmcgrwin.PointDrawFlag;
	pDoc->grdoc.nXType  = tmcgrwin.nXType;
	pDoc->grdoc.nYType  = tmcgrwin.nYType;
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	pDoc->grdoc.Ymin = tmcgrwin.sGrWin.Ymin;
	pDoc->grdoc.Ymax = tmcgrwin.sGrWin.Ymax;
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.nAFlagY = tmcgrwin.sGrWin.nAFlagY;
	pDoc->bLossPoint = bLossPoint;
	pDoc->WriteDocFile();

	RedrawViewGr();	

	if( dlg.m_Font )
	{
		OnConfigFont();
		OnEditGraphicsparameters();
		return;
	};

	if( dlg.m_Color )
	{
		OnConfigColorpoint();
		OnEditGraphicsparameters();
		return;
	};

	if( dlg.m_ColorGrid )
	{
		OnConfigColorGrid();
		OnEditGraphicsparameters();
		return;
	};

	if( dlg.m_ColorAxies )
	{
		OnConfigColorAxis();
		OnEditGraphicsparameters();
		return;
	};

	if( dlg.m_ColorBackground )
	{
		OnConfigColorBackground();
		OnEditGraphicsparameters();
		return;
	};

	if( dlg.m_ColorGraphics )
	{
		OnConfigColorGraphics();
		OnEditGraphicsparameters();
		return;
	};

	};

	// TODO: Add your message handler code here and/or call default
	
}

void CTMCGROUTView::OnViewGraphics() 
{
	// TODO: Add your command handler code here
	bIsDataCalculate = FALSE;
	ReadDocFileW();			// read data document 
	RedrawViewGr();	
}

void CTMCGROUTView::OnEditDocument() 
{
	// TODO: Add your command handler code here
	int i, n;
	CDialogDoc dlg;
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	dlg.m_DocFileName = pDoc->GetPathName();


	for( i = 0; i < 20; i++)
	{
		switch(i)
		{
		case  0:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_1.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName1.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_1 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag1 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag1 = TRUE;
				};
			}
			break;
		case  1:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_2.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName2.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_2 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag2 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag2 = TRUE;
				};
			}
			break;
		case  2:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_3.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName3.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_3 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag3 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag3 = TRUE;
				};
			}
			break;
		case  3:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_4.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName4.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_4 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag4 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag4 = TRUE;
				};
			}
			break;
		case  4:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_5.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName5.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_5 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag5 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag5 = TRUE;
				};
			}
			break;
		case  5:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_6.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName6.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_6 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag6 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag6 = TRUE;
				};
			}
			break;
		case  6:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_7.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName7.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_7 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag7 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag7 = TRUE;
				};
			}
			break;
		case  7:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_8.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName8.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_8 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag8 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag8 = TRUE;
				};
			}
			break;
		case  8:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_9.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName9.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_9 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag9 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag9 = TRUE;
				};
			}
			break;
		case  9:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_10.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName10.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_10 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag10 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag10 = TRUE;
				};
			}
			break;
		case  10:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_11.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName11.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_11 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag11 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag11 = TRUE;
				};
			}
			break;
		case  11:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_12.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName12.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_12 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag12 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag12 = TRUE;
				};
			}
			break;
		case 12:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_13.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName13.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_13 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag13 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag13 = TRUE;
				};
			}
			break;
		case 13:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_14.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName14.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_14 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag14 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag14 = TRUE;
				};
			}
			break;
		case 14:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_15.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName15.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_15 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag15 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag15 = TRUE;
				};
			}
			break;
		case 15:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_16.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName16.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_16 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag16 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag16 = TRUE;
				};
			}
			break;
		case 16:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_17.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName17.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_17 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag17 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag17 = TRUE;
				};
			}
			break;
		case 17:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_18.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName18.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_18 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag18 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag18 = TRUE;
				};
			}
			break;
		case 18:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_19.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName19.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_19 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag19 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag19 = TRUE;
				};
			}
			break;
		case 19:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_20.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName20.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_OutGrFlag_20 = pDoc->grdoc.psGraph[i].OutFlag;
				if( pDoc->grdoc.psGraph[i].nMod1 == 0 )
				{
					dlg.m_NormGrFlag20 = FALSE;
				}
				else
				{
					dlg.m_NormGrFlag20 = TRUE;
				};
			}
			break;
		}
	};	
	
	int ret = dlg.DoModal();

	if( ret == 1)
	{

		i = 0;								  // calculation number of graphics
		if(  dlg.m_SFileName_1.GetLength() != 0 ) i++;
		if(  dlg.m_SFileName_2.GetLength() != 0 ) i++;
		if(  dlg.m_SFileName_3.GetLength() != 0 ) i++;
		if(  dlg.m_SFileName_4.GetLength() != 0 ) i++;
		if(  dlg.m_SFileName_5.GetLength() != 0 ) i++;
		if(  dlg.m_SFileName_6.GetLength() != 0 ) i++;
		if(  dlg.m_SFileName_7.GetLength() != 0 ) i++;
		if(  dlg.m_SFileName_8.GetLength() != 0 ) i++;
		if(  dlg.m_SFileName_9.GetLength() != 0 ) i++;
		if( dlg.m_SFileName_10.GetLength() != 0 ) i++;
		if( dlg.m_SFileName_11.GetLength() != 0 ) i++;
		if( dlg.m_SFileName_12.GetLength() != 0 ) i++;
		if( dlg.m_SFileName_13.GetLength() != 0 ) i++;
		if( dlg.m_SFileName_14.GetLength() != 0 ) i++;
		if( dlg.m_SFileName_15.GetLength() != 0 ) i++;
		if( dlg.m_SFileName_16.GetLength() != 0 ) i++;
		if( dlg.m_SFileName_17.GetLength() != 0 ) i++;
		if( dlg.m_SFileName_18.GetLength() != 0 ) i++;
		if( dlg.m_SFileName_19.GetLength() != 0 ) i++;
		if( dlg.m_SFileName_20.GetLength() != 0 ) i++;
		n = i;
		pDoc->DocFileDelData();					 // delete old data and free memory
		bIsDataCalculate = FALSE;
		if( pDoc->DocFileNewData( n ) )          //allocation memory for new data and set standart data
		{
			i = 0;
			if(  dlg.m_SFileName_1.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_1);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName1 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_1;
				if( dlg.m_NormGrFlag1 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_2.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_2);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName2 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_2;
				if( dlg.m_NormGrFlag2 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_3.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_3);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName3 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_3;
				if( dlg.m_NormGrFlag3 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_4.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_4);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName4 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_4;
				if( dlg.m_NormGrFlag4 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_5.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_5);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName5 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_5;
				if( dlg.m_NormGrFlag5 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_6.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_6);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName6 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_6;
				if( dlg.m_NormGrFlag6 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_7.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_7);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName7 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_7;
				if( dlg.m_NormGrFlag7 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_8.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_8);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName8 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_8;
				if( dlg.m_NormGrFlag8 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_9.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_9);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName9 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_9;
				if( dlg.m_NormGrFlag9 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_10.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_10);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName10 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_10;
				if( dlg.m_NormGrFlag10 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_11.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_11);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName11 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_11;
				if( dlg.m_NormGrFlag11 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_12.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_12);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName12 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_12;
				if( dlg.m_NormGrFlag12 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_13.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_13);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName13 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_13;
				if( dlg.m_NormGrFlag13 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_14.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_14);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName14 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_14;
				if( dlg.m_NormGrFlag14 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_15.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_15);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName15 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_15;
				if( dlg.m_NormGrFlag15 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_16.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_16);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName16 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_16;
				if( dlg.m_NormGrFlag16 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_17.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_17);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName17 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_17;
				if( dlg.m_NormGrFlag17 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_18.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_18);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName18 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_18;
				if( dlg.m_NormGrFlag18 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_19.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_19);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName19 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_19;
				if( dlg.m_NormGrFlag19 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			if(  dlg.m_SFileName_20.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_20);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName20 );
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_20;
				if( dlg.m_NormGrFlag20 )
				{
					pDoc->grdoc.psGraph[i].nMod1 = 1;
				}
				else
				{
					pDoc->grdoc.psGraph[i].nMod1 = 0;
				};
				i++;
			}
			pDoc->grdoc.PointDrawFlag = tmcgrwin.PointDrawFlag;
			pDoc->grdoc.nXType  = tmcgrwin.nXType;
			pDoc->grdoc.nYType  = tmcgrwin.nYType;
			pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
			pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
			pDoc->grdoc.Ymin = tmcgrwin.sGrWin.Ymin;
			pDoc->grdoc.Ymax = tmcgrwin.sGrWin.Ymax;
			pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
			pDoc->grdoc.nAFlagY = tmcgrwin.sGrWin.nAFlagY;

			pDoc->WriteDocFile();
			ReadDocFileW();						// read data document 
		};
		RedrawViewGr();
	};

	if( dlg.m_AddCharacteristicsFlag )
	{
		OnEditAddcharacteristics();
	};
	
}

void CTMCGROUTView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	  
	if( nIDEvent == 1 )
	{
		CTMCGROUTDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		if( pDoc->IsDataModific() )	// if document file or S-matrix is modific
		{
			ReadDocFileW();			// read data document 
			RedrawViewGr();
		};
	};

	CScrollView::OnTimer(nIDEvent);
}

void CTMCGROUTView::OnEditAddcharacteristics() 
{
	// TODO: Add your command handler code here
	char ch[TMC_GROUT_MAXSTRING_BUF];
	CString csBuf;
	int i;
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	strcpy( ch, pDoc->GetPathName() );

	for( i = strlen(ch); i > 0; i-- )
	{
		if( ch[i] == '\\' )
		{
			ch[i] = '\0';
			i = 0;
		};
	};
	csBuf.Format( "%s\\*.dat", ch );

	CFileDialog dlg( TRUE, "", csBuf, OFN_ENABLESIZING, "Tamic Directional Pattern");
	dlg.m_ofn.lpstrTitle = "Add *.dat characteristics";
//	strcpy( dlg.m_ofn.lpstrFile, m_SFileName );
	if( dlg.DoModal() == IDOK )
	{
		strcpy( ch, dlg.GetPathName() );
		pDoc->AddGraphicsInDoc( ch );
		bIsDataCalculate = FALSE;
		ReadDocFileW();			// read data document 
		OnEditDocument();
	};
	
}

void CTMCGROUTView::OnViewResizectrlr() 
{
	// TODO: Add your command handler code here
	CRect m_oldRect;
	CPoint point;
    GetClientRect(&m_oldRect);

	CClientDC dc(this);
	OnPrepareDC(&dc);
	point.x = m_oldRect.right;
	point.y = m_oldRect.bottom;
	
	dc.DPtoLP(&point);  
	
	tmcgrwin.sGrView.nXmax =  (int)(0.7*point.x);
	tmcgrwin.sGrView.nYmax = -(int)(0.9*point.y);

	SetRectOutXY();
	RedrawViewGr();
}

void CTMCGROUTView::OnDrawMouseResize( CDC *pDC )
{
    if( !FlagDrawMouseCoordMove ) return;

    CPen newPen( PS_DASHDOTDOT, PS_COSMETIC, (COLORREF) RGB( 0, 200, 200 ) );
	CPen *pOldPen = pDC->SelectObject(&newPen);
	pDC->SelectStockObject( GRAY_BRUSH );
	pDC->Rectangle( MouseResize.left, MouseResize.top, MouseResize.right, MouseResize.bottom );
    pDC->SelectObject(pOldPen);

}

void CTMCGROUTView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	ReleaseCapture();
	FlagDrawMouseCoordMove = FALSE;

	IvalidateRectView( MouseResize );
	
	double rr;
	double Xmin, Xmax;
	double Ymin, Ymax;

	Xmin = LXCordToDoublX( MouseResize.left );
	Xmax = LXCordToDoublX( MouseResize.right);
	if( Xmax < Xmin )
	{
		rr = Xmax;
		Xmax = Xmin;
		Xmin = rr;
	};

	Ymin = LYCordToDoublY( MouseResize.top);
	Ymax = LYCordToDoublY( MouseResize.bottom);
	if( Ymax < Ymin )
	{
		rr = Ymax;
		Ymax = Ymin;
		Ymin = rr;
	};

	if( ((Ymax-Ymin) < 0.1*(tmcgrwin.sGrWin.Ymax-tmcgrwin.sGrWin.Ymin))&&
		((Xmax-Xmin) < 0.1*(tmcgrwin.sGrWin.Xmax-tmcgrwin.sGrWin.Xmin)) )
		return ;

	tmcgrwin.sGrWin.nAFlagY = TRUE;
//	tmcgrwin.sGrWin.nAFlagX = TRUE;	

	if( (Ymax-Ymin) >= 0.1*(tmcgrwin.sGrWin.Ymax-tmcgrwin.sGrWin.Ymin) )
	{
		tmcgrwin.sGrWin.Ymin = Ymin;
		tmcgrwin.sGrWin.Ymax = Ymax;
		tmcgrwin.sGrWin.nAFlagY = FALSE;
	};

	if((Xmax-Xmin) >= 0.1*(tmcgrwin.sGrWin.Xmax-tmcgrwin.sGrWin.Xmin) )
	{
	tmcgrwin.sGrWin.Xmin = Xmin;
	tmcgrwin.sGrWin.Xmax = Xmax;
	tmcgrwin.sGrWin.nAFlagX = FALSE;
	};

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	pDoc->grdoc.Ymin = tmcgrwin.sGrWin.Ymin;
	pDoc->grdoc.Ymax = tmcgrwin.sGrWin.Ymax;
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.nAFlagY = tmcgrwin.sGrWin.nAFlagY;
	pDoc->WriteDocFile();
	ReadDocFileW_1();

//	ReadDocFileW();						// read data document 
	RedrawViewGr();	

	CScrollView::OnLButtonUp(nFlags, point);
}

void CTMCGROUTView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if( !FlagDrawMouseCoordMove ) return;

	IvalidateRectView( MouseResize );

	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);  
	
	MouseResize.right  = point.x;
	if( MouseResize.right > tmcgrwin.sGrView.nXmax )  MouseResize.right = tmcgrwin.sGrView.nXmax;
	if( MouseResize.right < tmcgrwin.sGrView.nXmin )  MouseResize.right = tmcgrwin.sGrView.nXmin;
	MouseResize.bottom = point.y;
	if( MouseResize.bottom < -tmcgrwin.sGrView.nYmax )  MouseResize.bottom =-tmcgrwin.sGrView.nYmax;
	if( MouseResize.bottom > -tmcgrwin.sGrView.nYmin )  MouseResize.bottom =-tmcgrwin.sGrView.nYmin;

	MouseXCoord = LXCordToDoublX( point.x );
	MouseYCoord = LYCordToDoublY( point.y );
	PutStatistics1();
	PutTrace1();
//	IvalidateRectView( rectOutXY );

	CScrollView::OnMouseMove(nFlags, point);
}

void CTMCGROUTView::OnViewResizewindow() 
{
	// TODO: Add your command handler code here
		tmcgrwin.sGrWin.nAFlagX = TRUE;
		tmcgrwin.sGrWin.nAFlagY = TRUE;

		CTMCGROUTDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
		if( tmcgrwin.sGrWin.nAFlagX )
		{
			switch(tmcgrwin.nXType)
			{
				default:
					;
				case TMC_GROTS_TYPE_nT:
					tmcgrwin.sGrWin.Xmin = -3.141592653589;
					tmcgrwin.sGrWin.Xmax =  3.141592653589;
					break;
				case TMC_GROTS_TYPE_ps:
					tmcgrwin.sGrWin.Xmin = -180;
					tmcgrwin.sGrWin.Xmax =  180;
					break;
			};
		};
		pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
		pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
		ReadDocFileW_1();

		pDoc->grdoc.nAFlagY = tmcgrwin.sGrWin.nAFlagY;
		pDoc->WriteDocFile();
		RedrawViewGr();	
	
}

void CTMCGROUTView::OnConfigEditor() 
{
	// TODO: Add your command handler code here
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CFileDialog dlg( TRUE, "exe", "*.exe");
	if( dlg.DoModal() == IDOK )
	{
		pDoc->csEditorName = dlg.GetPathName();
		pDoc->WriteDocFile();
	};

}

void CTMCGROUTView::IvalidateRectView( CRect  r1 )
{
	CClientDC dc(this);
	OnPrepareDC(&dc);

	dc.LPtoDP( r1 );
	r1.NormalizeRect();
	InvalidateRect( r1 );
}

void CTMCGROUTView::OnFileSave() 
{
	// TODO: Add your command handler code here
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->WriteDocFile();
	RedrawViewGr();	
}

void CTMCGROUTView::OnFileSaveAs() 
{
	// TODO: Add your command handler code here
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CFileDialog dlg( FALSE, "tos", "*.tos");
	if( dlg.DoModal() == IDOK )
	{
		pDoc->SetPathName( dlg.GetPathName() );
		pDoc->WriteDocFile();
		RedrawViewGr();	
	};
}

void CTMCGROUTView::OnConfigFont() 
{
	// TODO: Add your command handler code here
/*
CFontDialog( LOGFONT lplfInitial = NULL, DWORD dwFlags = CF_EFFECTS | CF_SCREENFONTS, CDC* pdcPrinter = NULL, CWnd* pParentWnd = NULL )
*/
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CFontDialog dlg( &(pDoc->lfInitial), CF_EFFECTS | CF_SCREENFONTS);
	
	dlg.m_cf.rgbColors = pDoc->scTextColor;

	if( dlg.DoModal() == IDOK )
	{
		pDoc->scTextColor = dlg.GetColor();
		pDoc->WriteDocFile();
		RedrawViewGr();	
	};
}



void CTMCGROUTView::VTextOut( CDC *pDC, int nX, int nY, CString cText)
{
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
    CFont      fontText;
	COLORREF cOldColor;

	if( pDoc->lfInitial.lfHeight == 0 )
	{
		TEXTMETRIC Metrics;
		pDC->GetTextMetrics( &Metrics );
		pDoc->lfInitial.lfHeight = Metrics.tmHeight/20;
	};
	
	(pDoc->lfInitial.lfHeight)*=20;
	if( fontText.CreateFontIndirect( &(pDoc->lfInitial) ) )
	{
	    CFont* pOldFont = (CFont*) pDC->SelectObject(&fontText);
		cOldColor = pDC->GetTextColor();
		pDC->SetTextColor( pDoc->scTextColor );
		pDC->TextOut( nX, nY, cText);
	    pDC->SelectObject(pOldFont);
		pDC->SetTextColor( cOldColor );
	}
	else
	{
		pDC->TextOut( nX, nY, cText);
	}
	
	(pDoc->lfInitial.lfHeight)/=20;
	return;
}

void CTMCGROUTView::OnConfigColorpoint() 
{
	// TODO: Add your command handler code here
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CColorDialog dlg;
	
	dlg.m_cc.rgbResult = pDoc->scPointColor;

	if( dlg.DoModal() == IDOK )
	{
		pDoc->scPointColor = dlg.GetColor();
		pDoc->SetGraphColor();
		pDoc->WriteDocFile();
		RedrawViewGr();	
	};
}

void CTMCGROUTView::OnConfigColorAxis() 
{
	// TODO: Add your command handler code here
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CColorDialog dlg;
	
	dlg.m_cc.rgbResult = pDoc->scAxiesColor;

	if( dlg.DoModal() == IDOK )
	{
		pDoc->scAxiesColor = dlg.GetColor();
		pDoc->WriteDocFile();
		RedrawViewGr();	
	};
}

void CTMCGROUTView::OnConfigColorGrid() 
{
	// TODO: Add your command handler code here
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CColorDialog dlg;
	
	dlg.m_cc.rgbResult = pDoc->scGridColor;

	if( dlg.DoModal() == IDOK )
	{
		pDoc->scGridColor = dlg.GetColor();
		pDoc->WriteDocFile();
		RedrawViewGr();	
	};
}

void CTMCGROUTView::OnConfigColorBackground() 
{
	// TODO: Add your command handler code here
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CColorDialog dlg;
	
	dlg.m_cc.rgbResult = pDoc->scBackgoundColor;

	if( dlg.DoModal() == IDOK )
	{
		pDoc->scBackgoundColor = dlg.GetColor();
		pDoc->WriteDocFile();
		RedrawViewGr();	
	};
}

void CTMCGROUTView::OnDrawBackground(CDC* pDC)
{
//	CTMCGROUTDoc* pDoc = GetDocument();
//	ASSERT_VALID(pDoc);
//	CRect rectClient;
//	GetClientRect( &rectClient );
//	CClientDC dc(this);
//	OnPrepareDC(&dc);
//	dc.DPtoLP(&rectClient);  
//	rectClient.left = 0;
//	rectClient.top = 0;
//	rectClient.right  = (int)(tmcgrwin.sGrView.nXmax/0.7);
//	rectClient.bottom = -(int)(tmcgrwin.sGrView.nYmax/0.9);
//	rectClient.right  = (int)(TMC_VIEW_XSIZE*1.35);
//	rectClient.bottom = -(int)(TMC_VIEW_YSIZE*1.05);
//	rectClient.NormalizeRect();
//	pDC->FillSolidRect( &rectClient, pDoc->scBackgoundColor );
	return;
}


void CTMCGROUTView::WriteIniWndPar()
{
	char szBuffer[300];
	sprintf( szBuffer, "%d %d", tmcgrwin.sGrView.nXmax, tmcgrwin.sGrView.nYmax );
	AfxGetApp()->WriteProfileString( TMC_GROTS_DOCFILE_ID, TMC_GROUT_DOCFILE_ID_INIWNDVWPRT, szBuffer);
	AfxGetApp()->WriteProfileString( TMC_GROTS_DOCFILE_ID, TMC_GROTS_DOCFILE_XAXIESFORMAT, csXAxiesFormat);
	AfxGetApp()->WriteProfileString( TMC_GROTS_DOCFILE_ID, TMC_GROTS_DOCFILE_YAXIESFORMAT, csYAxiesFormat);
	{
		CString csBuffer;
		csBuffer.Format("%lg", dPointSize);
		AfxGetApp()->WriteProfileString( TMC_GROTS_DOCFILE_ID, TMC_GROTS_DOCFILE_POINTSIZE, csBuffer);
	};
	PutStatistics1();
	return;
}


void PutTrace( CString csTrace )
{
	CMainFrame *pFrame = ( CMainFrame * ) AfxGetApp()->m_pMainWnd;
	CStatusBar *pStatus = &pFrame->m_wndStatusBar;

	if( pStatus )
	{
		pStatus->SetPaneText( 0, csTrace );
	};

	return;
}

void PutTrace( char *szTrace )
{
	CString csBuf;
	csBuf.Format("%s", szTrace);
	PutTrace( csBuf );
	return;
}

void PutStatistics( CString csTrace )
{
	CMainFrame *pFrame = ( CMainFrame * ) AfxGetApp()->m_pMainWnd;
	CStatusBar *pStatus = &pFrame->m_wndStatusBar;

	if( pStatus )
	{
		pStatus->SetPaneText( 1, csTrace );
	};

	return;
}

void PutStatistics( char *szTrace )
{
	CString csBuf;
	csBuf.Format("%s", szTrace);
	PutStatistics( csBuf );
	return;
}

void CTMCGROUTView::PutTrace1()
{
	CString csBuf, text, text1;
	int i, j;

	for( i = 0, j = 0; (gr_typX[i].nType != -1) ; i++) 
	{
		if(tmcgrwin.nXType == gr_typX[i].nType ) j = i;
	};
//	text.Format("%s=%lg", gr_typX[j].pszXname, MouseXCoord );
	text.Format("%s=%lg", gr_typX[j].pszXname, MouseXCoord );

	for( i = 0, j = 0; (gr_typY[i].nType != -1) ; i++) 
	{
		if(tmcgrwin.nYType == gr_typY[i].nType ) j = i;
	};
//	text1.Format("%s=%lg", gr_typY[j].pszYname, MouseYCoord );
	text1.Format("%s=%lg", gr_typY[j].pszYname, MouseYCoord );
	
	csBuf.Format("%s; %s", text, text1);

	PutTrace( csBuf );
	
	return;
}

void CTMCGROUTView::PutStatistics1()
{
	CString csBuf;
	int i, j;
	
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	for( i = 0, j = 0; (gr_typY[i].nType != -1) ; i++) 
	{
		if(tmcgrwin.nYType == gr_typY[i].nType ) j = i;
	};
	csBuf.Format("%s; Freq=%lg GHz", gr_typY[j].pszYname, pDoc->dFreq/(1.e+9) );
	
	PutStatistics( csBuf );
	
	return;
}


void CTMCGROUTView::OnEditTosmatrix() 
{
	// TODO: Add your command handler code here
	CDialogTtoS dlg;
	int i;
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	dlg.m_DocFileName = pDoc->GetPathName();

	dlg.m_dTmin = tmcgrwin.sGrWin.Xmin;
	dlg.m_dTmax = tmcgrwin.sGrWin.Xmax;

	switch(tmcgrwin.nXType)
	{
		case TMC_GROTS_TYPE_ps:
			dlg.dTUnit = (1.e-12);
			dlg.m_csTimeUnit.Format( "%s", "ps");
			break;
		case TMC_GROTS_TYPE_ns:
			dlg.dTUnit = (1.e-9);
			dlg.m_csTimeUnit.Format( "%s", "ns");
			break;
		case TMC_GROTS_TYPE_mks:
			dlg.dTUnit = (1.e-6);
			dlg.m_csTimeUnit.Format( "%s", "mks");
			break;
		case TMC_GROTS_TYPE_ms:
			dlg.dTUnit = (1.e-3);
			dlg.m_csTimeUnit.Format( "%s", "ms");
			break;
		case TMC_GROTS_TYPE_s:
			dlg.dTUnit = (1.);
			dlg.m_csTimeUnit.Format( "%s", "s");
			break;
		default:
			dlg.dTUnit = (1.);
			dlg.m_dTmin = 0.;
			dlg.m_dTmax = 1.;
			break;
	}


	for( i = 0; i < 20; i++)
	{
		switch(i)
		{
		case  0:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_1.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_1 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue1 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case  1:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_2.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_2 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue2 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case  2:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_3.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_3 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue3 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case  3:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_4.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_4 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue4 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case  4:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_5.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_5 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue5 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case  5:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_6.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_6 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue6 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case  6:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_7.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_7 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue7 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case  7:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_8.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_8 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue8 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case  8:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_9.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_9 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue9 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case  9:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_10.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_10 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue10 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case  10:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_11.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_11 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue11 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case  11:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_12.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_12 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue12 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case 12:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_13.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_13 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue13 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case 13:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_14.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_14 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue14 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case 14:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_15.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_15 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue15 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case 15:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_16.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_16 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue16 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case 16:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_17.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_17 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue17 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case 17:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_18.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_18 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue18 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case 18:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_19.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_19 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue19 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		case 19:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_20.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_OutGrFlag_20 = pDoc->grdoc.psGraph[i].OutFlag;
				dlg.m_dXValue20 = pDoc->grdoc.psGraph[i].dFreq;
			}
			break;
		}
	};	

	int ret = dlg.DoModal();

	return;
}


void CTMCGROUTView::OnViewChangexmaxxminDecriment() 
{
	// TODO: Add your command handler code here
	double Xmin, Xmax;

	Xmin = tmcgrwin.sGrWin.Xmin - 0.1*(tmcgrwin.sGrWin.Xmax-tmcgrwin.sGrWin.Xmin);
	Xmax = tmcgrwin.sGrWin.Xmax - 0.1*(tmcgrwin.sGrWin.Xmax-tmcgrwin.sGrWin.Xmin);

	tmcgrwin.sGrWin.Xmin = Xmin;
	tmcgrwin.sGrWin.Xmax = Xmax;
	tmcgrwin.sGrWin.nAFlagX = FALSE;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	pDoc->grdoc.Ymin = tmcgrwin.sGrWin.Ymin;
	pDoc->grdoc.Ymax = tmcgrwin.sGrWin.Ymax;
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.nAFlagY = tmcgrwin.sGrWin.nAFlagY;
	pDoc->WriteDocFile();

	ReadDocFileW_1();

	RedrawViewGr();	
	
	return;
}

void CTMCGROUTView::OnViewChangexmaxxminIncrement() 
{
	// TODO: Add your command handler code here
	double Xmin, Xmax;

	Xmin = tmcgrwin.sGrWin.Xmin + 0.1*(tmcgrwin.sGrWin.Xmax-tmcgrwin.sGrWin.Xmin);
	Xmax = tmcgrwin.sGrWin.Xmax + 0.1*(tmcgrwin.sGrWin.Xmax-tmcgrwin.sGrWin.Xmin);

	tmcgrwin.sGrWin.Xmin = Xmin;
	tmcgrwin.sGrWin.Xmax = Xmax;
	tmcgrwin.sGrWin.nAFlagX = FALSE;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	pDoc->grdoc.Ymin = tmcgrwin.sGrWin.Ymin;
	pDoc->grdoc.Ymax = tmcgrwin.sGrWin.Ymax;
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.nAFlagY = tmcgrwin.sGrWin.nAFlagY;
	pDoc->WriteDocFile();

	ReadDocFileW_1();

	RedrawViewGr();	
	
	return;
}

void CTMCGROUTView::OnViewChangeymaxyminDecrement() 
{
	// TODO: Add your command handler code here
	double Ymin, Ymax;

	Ymin = tmcgrwin.sGrWin.Ymin - 0.1*(tmcgrwin.sGrWin.Ymax-tmcgrwin.sGrWin.Ymin);
	Ymax = tmcgrwin.sGrWin.Ymax - 0.1*(tmcgrwin.sGrWin.Ymax-tmcgrwin.sGrWin.Ymin);

	tmcgrwin.sGrWin.Ymin = Ymin;
	tmcgrwin.sGrWin.Ymax = Ymax;
	tmcgrwin.sGrWin.nAFlagY = FALSE;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	pDoc->grdoc.Ymin = tmcgrwin.sGrWin.Ymin;
	pDoc->grdoc.Ymax = tmcgrwin.sGrWin.Ymax;
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.nAFlagY = tmcgrwin.sGrWin.nAFlagY;
	pDoc->WriteDocFile();

	RedrawViewGr();	
	
	return;
}

void CTMCGROUTView::OnViewChangeymaxyminIncrement() 
{
	// TODO: Add your command handler code here
	double Ymin, Ymax;

	Ymin = tmcgrwin.sGrWin.Ymin + 0.1*(tmcgrwin.sGrWin.Ymax-tmcgrwin.sGrWin.Ymin);
	Ymax = tmcgrwin.sGrWin.Ymax + 0.1*(tmcgrwin.sGrWin.Ymax-tmcgrwin.sGrWin.Ymin);

	tmcgrwin.sGrWin.Ymin = Ymin;
	tmcgrwin.sGrWin.Ymax = Ymax;
	tmcgrwin.sGrWin.nAFlagY = FALSE;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	pDoc->grdoc.Ymin = tmcgrwin.sGrWin.Ymin;
	pDoc->grdoc.Ymax = tmcgrwin.sGrWin.Ymax;
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.nAFlagY = tmcgrwin.sGrWin.nAFlagY;
	pDoc->WriteDocFile();

	RedrawViewGr();	
	
	return;
}

void CTMCGROUTView::OnViewAutoxsize() 
{
	// TODO: Add your command handler code here
	tmcgrwin.sGrWin.nAFlagX = !tmcgrwin.sGrWin.nAFlagX;

	if( tmcgrwin.sGrWin.nAFlagX )
	{
		switch(tmcgrwin.nXType)
		{
			default:
				;
			case TMC_GROTS_TYPE_nT:
				tmcgrwin.sGrWin.Xmin = -3.141592653589;
				tmcgrwin.sGrWin.Xmax =  3.141592653589;
				break;
			case TMC_GROTS_TYPE_ps:
				tmcgrwin.sGrWin.Xmin = -180;
				tmcgrwin.sGrWin.Xmax =  180;
				break;
		};
	};

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	ReadDocFileW_1();
	pDoc->WriteDocFile();

	RedrawViewGr();	
	
	return;
}

void CTMCGROUTView::OnUpdateViewAutoxsize(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( tmcgrwin.sGrWin.nAFlagX );	
	return;
}

void CTMCGROUTView::OnViewAutoysize() 
{
	// TODO: Add your command handler code here
	tmcgrwin.sGrWin.nAFlagY = !tmcgrwin.sGrWin.nAFlagY;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.nAFlagY = tmcgrwin.sGrWin.nAFlagY;
	pDoc->WriteDocFile();

	RedrawViewGr();	
	
	return;
}

void CTMCGROUTView::OnUpdateViewAutoysize(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( tmcgrwin.sGrWin.nAFlagY );	
	return;
}

void CTMCGROUTView::OnViewZoomZoom() 
{
	// TODO: Add your command handler code here
	double Xmin, Xmax, Ymin, Ymax;

	Xmin = 0.5*(tmcgrwin.sGrWin.Xmin + tmcgrwin.sGrWin.Xmax) - 1.0*(tmcgrwin.sGrWin.Xmax - tmcgrwin.sGrWin.Xmin);
	Xmax = 0.5*(tmcgrwin.sGrWin.Xmin + tmcgrwin.sGrWin.Xmax) + 1.0*(tmcgrwin.sGrWin.Xmax - tmcgrwin.sGrWin.Xmin);
	Ymin = 0.5*(tmcgrwin.sGrWin.Ymin + tmcgrwin.sGrWin.Ymax) - 1.0*(tmcgrwin.sGrWin.Ymax - tmcgrwin.sGrWin.Ymin);
	Ymax = 0.5*(tmcgrwin.sGrWin.Ymin + tmcgrwin.sGrWin.Ymax) + 1.0*(tmcgrwin.sGrWin.Ymax - tmcgrwin.sGrWin.Ymin);

	tmcgrwin.sGrWin.Xmin = Xmin;
	tmcgrwin.sGrWin.Xmax = Xmax;
	tmcgrwin.sGrWin.Ymin = Ymin;
	tmcgrwin.sGrWin.Ymax = Ymax;

	tmcgrwin.sGrWin.nAFlagX = FALSE;
	tmcgrwin.sGrWin.nAFlagY = FALSE;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	pDoc->grdoc.Ymin = tmcgrwin.sGrWin.Ymin;
	pDoc->grdoc.Ymax = tmcgrwin.sGrWin.Ymax;
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.nAFlagY = tmcgrwin.sGrWin.nAFlagY;
	pDoc->WriteDocFile();

	RedrawViewGr();	
	
	return;
}

void CTMCGROUTView::OnViewZoomZoomp() 
{
	// TODO: Add your command handler code here
	double Xmin, Xmax, Ymin, Ymax;

	Xmin = 0.5*(tmcgrwin.sGrWin.Xmin + tmcgrwin.sGrWin.Xmax) - 0.25*(tmcgrwin.sGrWin.Xmax - tmcgrwin.sGrWin.Xmin);
	Xmax = 0.5*(tmcgrwin.sGrWin.Xmin + tmcgrwin.sGrWin.Xmax) + 0.25*(tmcgrwin.sGrWin.Xmax - tmcgrwin.sGrWin.Xmin);
	Ymin = 0.5*(tmcgrwin.sGrWin.Ymin + tmcgrwin.sGrWin.Ymax) - 0.25*(tmcgrwin.sGrWin.Ymax - tmcgrwin.sGrWin.Ymin);
	Ymax = 0.5*(tmcgrwin.sGrWin.Ymin + tmcgrwin.sGrWin.Ymax) + 0.25*(tmcgrwin.sGrWin.Ymax - tmcgrwin.sGrWin.Ymin);

	tmcgrwin.sGrWin.Xmin = Xmin;
	tmcgrwin.sGrWin.Xmax = Xmax;
	tmcgrwin.sGrWin.Ymin = Ymin;
	tmcgrwin.sGrWin.Ymax = Ymax;

	tmcgrwin.sGrWin.nAFlagX = FALSE;
	tmcgrwin.sGrWin.nAFlagY = FALSE;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	pDoc->grdoc.Ymin = tmcgrwin.sGrWin.Ymin;
	pDoc->grdoc.Ymax = tmcgrwin.sGrWin.Ymax;
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.nAFlagY = tmcgrwin.sGrWin.nAFlagY;
	pDoc->WriteDocFile();

	RedrawViewGr();	
	
	return;
}

void CTMCGROUTView::OnViewZoomZoomxp() 
{
	// TODO: Add your command handler code here
	double Xmin, Xmax;

	Xmin = 0.5*(tmcgrwin.sGrWin.Xmin + tmcgrwin.sGrWin.Xmax) - 0.25*(tmcgrwin.sGrWin.Xmax - tmcgrwin.sGrWin.Xmin);
	Xmax = 0.5*(tmcgrwin.sGrWin.Xmin + tmcgrwin.sGrWin.Xmax) + 0.25*(tmcgrwin.sGrWin.Xmax - tmcgrwin.sGrWin.Xmin);

	tmcgrwin.sGrWin.Xmin = Xmin;
	tmcgrwin.sGrWin.Xmax = Xmax;

	tmcgrwin.sGrWin.nAFlagX = FALSE;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	pDoc->grdoc.Ymin = tmcgrwin.sGrWin.Ymin;
	pDoc->grdoc.Ymax = tmcgrwin.sGrWin.Ymax;
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.nAFlagY = tmcgrwin.sGrWin.nAFlagY;
	pDoc->WriteDocFile();
	ReadDocFileW_1();

	RedrawViewGr();	
	
	return;
}

void CTMCGROUTView::OnViewZoomZoomyp() 
{
	// TODO: Add your command handler code here
	double Ymin, Ymax;

	Ymin = 0.5*(tmcgrwin.sGrWin.Ymin + tmcgrwin.sGrWin.Ymax) - 0.25*(tmcgrwin.sGrWin.Ymax - tmcgrwin.sGrWin.Ymin);
	Ymax = 0.5*(tmcgrwin.sGrWin.Ymin + tmcgrwin.sGrWin.Ymax) + 0.25*(tmcgrwin.sGrWin.Ymax - tmcgrwin.sGrWin.Ymin);

	tmcgrwin.sGrWin.Ymin = Ymin;
	tmcgrwin.sGrWin.Ymax = Ymax;

	tmcgrwin.sGrWin.nAFlagY = FALSE;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	pDoc->grdoc.Ymin = tmcgrwin.sGrWin.Ymin;
	pDoc->grdoc.Ymax = tmcgrwin.sGrWin.Ymax;
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.nAFlagY = tmcgrwin.sGrWin.nAFlagY;
	pDoc->WriteDocFile();

	RedrawViewGr();	
	
	return;
}

void CTMCGROUTView::OnViewZoomZoomx() 
{
	// TODO: Add your command handler code here
	double Xmin, Xmax;

	Xmin = 0.5*(tmcgrwin.sGrWin.Xmin + tmcgrwin.sGrWin.Xmax) - 1.0*(tmcgrwin.sGrWin.Xmax - tmcgrwin.sGrWin.Xmin);
	Xmax = 0.5*(tmcgrwin.sGrWin.Xmin + tmcgrwin.sGrWin.Xmax) + 1.0*(tmcgrwin.sGrWin.Xmax - tmcgrwin.sGrWin.Xmin);

	tmcgrwin.sGrWin.Xmin = Xmin;
	tmcgrwin.sGrWin.Xmax = Xmax;

	tmcgrwin.sGrWin.nAFlagX = FALSE;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	pDoc->grdoc.Ymin = tmcgrwin.sGrWin.Ymin;
	pDoc->grdoc.Ymax = tmcgrwin.sGrWin.Ymax;
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.nAFlagY = tmcgrwin.sGrWin.nAFlagY;
	pDoc->WriteDocFile();
	ReadDocFileW_1();

	RedrawViewGr();	
	
	return;
}

void CTMCGROUTView::OnViewZoomZoomy() 
{
	// TODO: Add your command handler code here
	double Ymin, Ymax;

	Ymin = 0.5*(tmcgrwin.sGrWin.Ymin + tmcgrwin.sGrWin.Ymax) - 1.0*(tmcgrwin.sGrWin.Ymax - tmcgrwin.sGrWin.Ymin);
	Ymax = 0.5*(tmcgrwin.sGrWin.Ymin + tmcgrwin.sGrWin.Ymax) + 1.0*(tmcgrwin.sGrWin.Ymax - tmcgrwin.sGrWin.Ymin);

	tmcgrwin.sGrWin.Ymin = Ymin;
	tmcgrwin.sGrWin.Ymax = Ymax;

	tmcgrwin.sGrWin.nAFlagY = FALSE;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	pDoc->grdoc.Ymin = tmcgrwin.sGrWin.Ymin;
	pDoc->grdoc.Ymax = tmcgrwin.sGrWin.Ymax;
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.nAFlagY = tmcgrwin.sGrWin.nAFlagY;
	pDoc->WriteDocFile();

	RedrawViewGr();	
	
	return;
}

BOOL CTMCGROUTView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// Set brush to desired background color
	CBrush backBrush( pDoc->scBackgoundColor );
	// Save old brush
	CBrush* pOldBrush = pDC->SelectObject(&backBrush);
	CRect rect;
	pDC->GetClipBox(&rect);     // Erase the area needed
	pDC->PatBlt( rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
	pDC->SelectObject(pOldBrush);
	return TRUE;
//	return CScrollView::OnEraseBkgnd(pDC);
}

void CTMCGROUTView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	// TODO: Add your specialized code here and/or call the base class
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if( pDC->IsPrinting() ) 
	{
//		pDC->SetBkColor( pDoc->scBackgoundColor );
	}
	else 
	{
		pDC->SetBkColor( pDoc->scBackgoundColor );
	};
	CScrollView::OnPrepareDC(pDC, pInfo);
}

void CTMCGROUTView::OnConfigColorGraphics() 
{
	// TODO: Add your command handler code here
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CTmcGroutColorGraphDialog dlg;
	
	dlg.SetGraphParam( pDoc->sGrDoc );

	if( dlg.DoModal() == IDOK )
	{
		dlg.GetGraphParam( pDoc->sGrDoc );
		pDoc->WriteDocFile();
//		pDoc->ReadGraphParametersDefault();
//		pDoc->ReadGraphParameters();
		bIsDataCalculate = FALSE;
		ReadDocFileW();
		RedrawViewGr();	
	};

	return;
}

void CTMCGROUTView::OnViewTranslateChangexmaxxminEnd() 
{
	// TODO: Add your command handler code here
	if( tmcgrwin.sGrWin.nAFlagX ) return;


	double d;

	d = tmcgrwin.sGrWin.Xmax - tmcgrwin.sGrWin.Xmin;

	switch(tmcgrwin.nXType)
	{
		default:
			;
		case TMC_GROTS_TYPE_nT:
			tmcgrwin.sGrWin.Xmax = 3.141592653589;
			break;
		case TMC_GROTS_TYPE_ps:
			tmcgrwin.sGrWin.Xmax = 180.;
			break;
	};

	tmcgrwin.sGrWin.Xmin = tmcgrwin.sGrWin.Xmax - d;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	ReadDocFileW_1();
	pDoc->WriteDocFile();

	RedrawViewGr();	
	
	return;
	
}

void CTMCGROUTView::OnViewTranslateChangexmaxxminHome() 
{
	// TODO: Add your command handler code here
	if( tmcgrwin.sGrWin.nAFlagX ) return;

	double d;

	d = tmcgrwin.sGrWin.Xmax - tmcgrwin.sGrWin.Xmin;

	switch(tmcgrwin.nXType)
	{
		default:
			;
		case TMC_GROTS_TYPE_nT:
			tmcgrwin.sGrWin.Xmin = 0;
			break;
		case TMC_GROTS_TYPE_ps:
			tmcgrwin.sGrWin.Xmin = 0;
			break;
	};

	tmcgrwin.sGrWin.Xmax = tmcgrwin.sGrWin.Xmin + d;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	ReadDocFileW_1();
	pDoc->WriteDocFile();

	RedrawViewGr();	
	
	return;
	
}

double CTMCGROUTView::GetXAxiesUnit()
{
	double x;

	switch(tmcgrwin.nXType)
	{
	case TMC_GROTS_TYPE_nT:
		x = (1.);
		break;
	case TMC_GROTS_TYPE_ps:
		x = (1.e+12);
		break;
	case TMC_GROTS_TYPE_ns:
		x = (1.e+9);
		break;
	case TMC_GROTS_TYPE_mks:
		x = (1.e+6);
		break;
	case TMC_GROTS_TYPE_ms:
		x = (1.e+3);
		break;
	case TMC_GROTS_TYPE_s:
		x = (1.);
		break;
	default:
		x = (1.);
		break;
	};

	return x;
}

void CTMCGROUTView::ReadDocFileW_1()
{
	int i;
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	pDoc->ReadDocFile_1();	// read doc file
	
	if(tmcgrwin.psGraph != NULL)
	{
		for( i = 0; i < tmcgrwin.nGraph; i++)
		{
			if( tmcgrwin.psGraph[i].pPoint  != NULL ) delete  tmcgrwin.psGraph[i].pPoint;
			if( tmcgrwin.psGraph[i].piPoint != NULL ) delete  tmcgrwin.psGraph[i].piPoint;
		};
		delete tmcgrwin.psGraph;
	};
	tmcgrwin.nGraph = pDoc->grdoc.nGraph;
	tmcgrwin.psGraph = new TMC_GR_VIEW[tmcgrwin.nGraph];

	for( i = 0; i < tmcgrwin.nGraph; i++)
	{
		tmcgrwin.psGraph[i].nPoint = pDoc->grdoc.psGraph[i].nPoint;
		if( tmcgrwin.psGraph[i].nPoint <= 0 )
		{
			tmcgrwin.psGraph[i].pPoint  = NULL;
			tmcgrwin.psGraph[i].piPoint = NULL;
		}
		else
		{
			tmcgrwin.psGraph[i].pPoint  = new     TMC_POINT[tmcgrwin.psGraph[i].nPoint];
			tmcgrwin.psGraph[i].piPoint = new TMC_INT_POINT[tmcgrwin.psGraph[i].nPoint];
		};
	};

}

void CTMCGROUTView::ReadDocFileW_Thread()
{
	int i;
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	pDoc->ReadDocFile();	// read doc file
	
	if(tmcgrwin.psGraph != NULL)
	{
		for( i = 0; i < tmcgrwin.nGraph; i++)
		{
			if( tmcgrwin.psGraph[i].pPoint  != NULL ) delete  tmcgrwin.psGraph[i].pPoint;
			if( tmcgrwin.psGraph[i].piPoint != NULL ) delete  tmcgrwin.psGraph[i].piPoint;
		};
		delete tmcgrwin.psGraph;
	};
	tmcgrwin.nGraph = pDoc->grdoc.nGraph;
	tmcgrwin.psGraph = new TMC_GR_VIEW[tmcgrwin.nGraph];

	for( i = 0; i < tmcgrwin.nGraph; i++)
	{
		tmcgrwin.psGraph[i].nPoint = pDoc->grdoc.psGraph[i].nPoint;
		if( tmcgrwin.psGraph[i].nPoint <= 0 )
		{
			tmcgrwin.psGraph[i].pPoint  = NULL;
			tmcgrwin.psGraph[i].piPoint = NULL;
		}
		else
		{
			tmcgrwin.psGraph[i].pPoint  = new     TMC_POINT[tmcgrwin.psGraph[i].nPoint];
			tmcgrwin.psGraph[i].piPoint = new TMC_INT_POINT[tmcgrwin.psGraph[i].nPoint];
		};
	};

	bIsDataCalculate = TRUE;
  	PrepareDoubleGraph();	// copy graph doc->view
	PrepareLogGraph();		// calculation double coord -> logic coord
	SetRectOutXY();	
	RedrawWindow();
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////


UINT ReadDocFileW_Thread1( LPVOID pParam )
{
	CTMCGROUTView* pDlg = (CTMCGROUTView *)(pParam);

	pDlg->ReadDocFileW_Thread();
	
	return 0;
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

void CTMCGROUTView::OnDestroy() 
{
	CScrollView::OnDestroy();


	for( ; !bIsDataCalculate; )
	{
		CThreadCalcDirPat cDlg;
		cDlg.pCView = (void *)(this);
		cDlg.DoModal();
		Sleep(1000);
	};

	
	// TODO: Add your message handler code here
	
}

BOOL CTMCGROUTView::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class

	return CScrollView::DestroyWindow();
}
