// TMCGROUTView.cpp : implementation of the CTMCGROUTView class
//

#include <tmcgrviw.h>
#include "stdafx.h"
#include "TMCGROUT.h"

#include "MainFrm.h"
#include "TMCGROUTDoc.h"
#include "TMCGROUTView.h"
#include "TMCGROUTDIALOGView.h"
#include "DialogDoc.h"
#include "TmcGroutColorGraphDialog.h"

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
	ON_COMMAND(ID_CONFIG_COLOR_GRAPHICS, OnConfigColorGraphics)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_VIEW_TRANSLATE_HOME, OnViewTranslateHome)
	ON_COMMAND(ID_VIEW_TRANSLATE_END, OnViewTranslateEnd)
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
	tmcgrwin.nYType = TMC_GR_TYPE_WSVR;
	tmcgrwin.nXType	= TMC_GR_TYPE_FGHz;
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
	bLossPoint = TRUE;
	SetRectOutXY();
	MouseXCoord = tmcgrwin.sGrWin.Xmin;
	MouseYCoord = tmcgrwin.sGrWin.Ymin;
	FlagDrawMouseCoord     = FALSE;
	FlagDrawMouseCoordMove = FALSE;
	dXAxiesUserUnit = 0.0;
	csXAxiesFormat.Format("%s", "%4.3lg");
	csYAxiesFormat.Format("%s", "%4.3lg");
	dPointSize = 1.;

	gr_typX = new TMC_GR_TYPE_X[ 6 ];
	gr_typX[0].nType = TMC_GR_TYPE_FGHz;
	strcpy( gr_typX[0].pszXname, "Freq GHz" );
	gr_typX[1].nType = TMC_GR_TYPE_FMHz;
	strcpy( gr_typX[1].pszXname, "Freq MHz" );
	gr_typX[2].nType = TMC_GR_TYPE_FKHz;
	strcpy( gr_typX[2].pszXname, "Freq KHz" );
	gr_typX[3].nType = TMC_GR_TYPE_FHz;
	strcpy( gr_typX[3].pszXname, "Freq Hz" );
	gr_typX[4].nType = TMC_GR_TYPE_F_USER;
	strcpy( gr_typX[4].pszXname, "" );
	gr_typX[5].nType = -1;
	gr_typX[5].pszXname[0] = '\0';

	gr_typY = new TMC_GR_TYPE_Y[ 6 ];
	gr_typY[0].nType = TMC_GR_TYPE_WSVR;
	strcpy( gr_typY[0].pszYname, "VSWR" );
	gr_typY[1].nType = TMC_GR_TYPE_L;
	strcpy( gr_typY[1].pszYname, "L dB" );
	gr_typY[2].nType = TMC_GR_TYPE_SM;
	strcpy( gr_typY[2].pszYname, "|S|" );
	gr_typY[3].nType = TMC_GR_TYPE_SFD;
	strcpy( gr_typY[3].pszYname, "Phase S - grad" );
	gr_typY[4].nType = TMC_GR_TYPE_SFR;
	strcpy( gr_typY[4].pszYname, "Phase S - rad" );
	gr_typY[5].nType = -1;
	gr_typY[5].pszYname[0] = '\0';
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
//	OnDrawMouseScrol( pDC );
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
	dXAxiesUserUnit = pDoc->dXAxiesUserUnit;
	strcpy( gr_typX[4].pszXname, pDoc->csXAxiesUserName );

	{
		int nXX, nYY;
		double dBuf;
		CString csBuffer;
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
	
	SetTimer( 1, 10000, NULL);


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
		FlagDrawMouseCoord     = TRUE;
		FlagDrawMouseCoordMove = TRUE;
		SetCapture();
		PutTrace1();
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
	int i, j;
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

}

void CTMCGROUTView::PrepareDoubleGraph()
{
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if( tmcgrwin.psGraph == NULL ) return;
	
	int i, j;
	double r11;
	double xMin, yMin, xMax, yMax;

	PutStatistics1();

	for( i = 0, xMin = yMin = FLT_MAX, xMax = yMax = -FLT_MAX; i < tmcgrwin.nGraph; i++)
	{
	if(pDoc->grdoc.psGraph[i].OutFlag)
	{
		for( j = 0; j < tmcgrwin.psGraph[i].nPoint; j++)
		{
			switch(tmcgrwin.nXType)
			{
			case TMC_GR_TYPE_FGHz:
				tmcgrwin.psGraph[i].pPoint[j].x = pDoc->grdoc.psGraph[i].pFreq[j]/(1.e+9);
				break;
			case TMC_GR_TYPE_FMHz:
				tmcgrwin.psGraph[i].pPoint[j].x = pDoc->grdoc.psGraph[i].pFreq[j]/(1.e+6);
				;
				break;
			case TMC_GR_TYPE_FKHz:
				tmcgrwin.psGraph[i].pPoint[j].x = pDoc->grdoc.psGraph[i].pFreq[j]/(1.e+3);
				;
				break;
			case TMC_GR_TYPE_FHz:
				tmcgrwin.psGraph[i].pPoint[j].x = pDoc->grdoc.psGraph[i].pFreq[j]/(1.);
				;
				break;
			case TMC_GR_TYPE_F_USER:
				tmcgrwin.psGraph[i].pPoint[j].x = pDoc->grdoc.psGraph[i].pFreq[j]*(dXAxiesUserUnit);
				;
				break;
			default:
				tmcgrwin.psGraph[i].pPoint[j].x = pDoc->grdoc.psGraph[i].pFreq[j]/(1.e+9);
				;
				break;
			}
			if( xMin > tmcgrwin.psGraph[i].pPoint[j].x ) xMin = tmcgrwin.psGraph[i].pPoint[j].x;
			if( xMax < tmcgrwin.psGraph[i].pPoint[j].x ) xMax = tmcgrwin.psGraph[i].pPoint[j].x;

			switch(tmcgrwin.nYType)
			{
			case TMC_GR_TYPE_L:
				if(pDoc->grdoc.psGraph[i].pcExpr == NULL )
				{
					tmcgrwin.psGraph[i].pPoint[j].y = -20.*log10( cabsv( pDoc->grdoc.psGraph[i].pSmatr[j] ) );
				}
				else
				{
					if( pDoc->grdoc.psGraph[i].pcExpr->IsGraphDifferent() )
					{
//						tmcgrwin.psGraph[i].pPoint[j].y = -20.*log10( cabsv( pDoc->grdoc.psGraph[i].pSmatr[j] ) );
					}
					else
					{
						tmcgrwin.psGraph[i].pPoint[j].y = pDoc->grdoc.psGraph[i].pcExpr->GetExpressionValueL( j );
					};
				};
				break;
			default:
			case TMC_GR_TYPE_WSVR:
				if(pDoc->grdoc.psGraph[i].pcExpr == NULL )
				{
					r11 = (double)(cabsv( pDoc->grdoc.psGraph[i].pSmatr[j] ));
					if( fabs( r11 - 1.) < 0.0001	)
					{
						r11 = 0.9999;
					}
					tmcgrwin.psGraph[i].pPoint[j].y = (1.+r11)/(1.-r11 );
				}
				else
				{
					tmcgrwin.psGraph[i].pPoint[j].y = pDoc->grdoc.psGraph[i].pcExpr->GetExpressionValueK( j );
				};
				break;
			case TMC_GR_TYPE_SM:
				if(pDoc->grdoc.psGraph[i].pcExpr == NULL )
				{
					tmcgrwin.psGraph[i].pPoint[j].y = cabsv( pDoc->grdoc.psGraph[i].pSmatr[j] );
				}
				else
				{
					tmcgrwin.psGraph[i].pPoint[j].y = pDoc->grdoc.psGraph[i].pcExpr->GetExpressionValueSM( j );
				};
				break;
			case TMC_GR_TYPE_SFR:
				if(pDoc->grdoc.psGraph[i].pcExpr == NULL )
				{
					tmcgrwin.psGraph[i].pPoint[j].y = cphase( pDoc->grdoc.psGraph[i].pSmatr[j] );
				}
				else
				{
					if( tmcgrwin.nXType == TMC_GR_TYPE_F_USER ) tmcgrwin.psGraph[i].pPoint[j].y = pDoc->grdoc.psGraph[i].pcExpr->GetExpressionValueSFR_User( j );
					else tmcgrwin.psGraph[i].pPoint[j].y = pDoc->grdoc.psGraph[i].pcExpr->GetExpressionValueSFR( j );
				};
				break;
			case TMC_GR_TYPE_SFD:
				if(pDoc->grdoc.psGraph[i].pcExpr == NULL )
				{
					tmcgrwin.psGraph[i].pPoint[j].y = 180./(3.141592653589)*cphase( pDoc->grdoc.psGraph[i].pSmatr[j] );
				}
				else
				{
					if( tmcgrwin.nXType == TMC_GR_TYPE_F_USER ) tmcgrwin.psGraph[i].pPoint[j].y = pDoc->grdoc.psGraph[i].pcExpr->GetExpressionValueSFD_User( j );
					else tmcgrwin.psGraph[i].pPoint[j].y = pDoc->grdoc.psGraph[i].pcExpr->GetExpressionValueSFD( j );
				};
				break;
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
			tmcgrwin.psGraph[i].nPointDraw++;

			if( x < tmcgrwin.sGrWin.Xmin )
			{
				tmcgrwin.psGraph[i].nPointDraw--;
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
			};
			if( x > tmcgrwin.sGrWin.Xmax )
			{
				tmcgrwin.psGraph[i].nPointDraw--;
				if( j > 0 )
				{
					tmcgrwin.psGraph[i].piPoint[j].nX = tmcgrwin.psGraph[i].piPoint[j-1].nX;
					tmcgrwin.psGraph[i].piPoint[j].nY = tmcgrwin.psGraph[i].piPoint[j-1].nY;
				};

			};

		};
	};
	};

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

	for( i = 0; i < tmcgrwin.nGraph; i++)
	{	
	  OnDrawGraph1( pDC, i);	   // draw line
	};
	for( i = 0; i < tmcgrwin.nGraph; i++)
	{	
	  OnDrawGraph2( pDC, i);	   // draw point
	};
	for( i = 0; i < tmcgrwin.nGraph; i++)
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
	if( tmcgrwin.psGraph == NULL ) return;
	if( tmcgrwin.psGraph[i].nPoint <= 0 ) return;
		
		for( j = 0, pDC->MoveTo(tmcgrwin.psGraph[i].piPoint[0].nX, tmcgrwin.psGraph[i].piPoint[0].nY); j < tmcgrwin.psGraph[i].nPoint; j++)
		{
			OnDrawLine( pDC, i, tmcgrwin.psGraph[i].piPoint[j].nX, tmcgrwin.psGraph[i].piPoint[j].nY );
		};

}

void CTMCGROUTView::OnDrawGraph2(CDC *pDC, int i)
{
	int  j, ii;
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(!pDoc->grdoc.psGraph[i].OutFlag) return;

	if( tmcgrwin.psGraph == NULL ) return;
	if( tmcgrwin.psGraph[i].nPoint <= 0 ) return;
	
	if( bLossPoint )
	{
		if( tmcgrwin.psGraph[i].nPointDraw < 0 ) tmcgrwin.psGraph[i].nPointDraw = 0;
		ii = tmcgrwin.psGraph[i].nPointDraw/20 + 1;
	}
	else
	{
		ii = 1;
	};

	for( j = 0; j < tmcgrwin.psGraph[i].nPoint; j += ii)
	{
		OnDrawPoint( pDC,  i,  tmcgrwin.psGraph[i].piPoint[j].nX, tmcgrwin.psGraph[i].piPoint[j].nY );
	};

}

void CTMCGROUTView::OnDrawPoint( CDC *pDC, int i, int nX, int nY )
{
	int j1;
	if( tmcgrwin.psGraph == NULL ) return;
	if(  !tmcgrwin.PointDrawFlag ) return;
	
	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CPen newPen( pDoc->grdoc.psGraph[i].PointType, pDoc->grdoc.psGraph[i].PointWidth, (COLORREF) pDoc->scPointColor );
	CPen *pOldPen = pDC->SelectObject(&newPen);

	for( j1 = 0, pDC->MoveTo(nX + (int)(dPointSize*pDoc->grdoc.psGraph[i].piPoint[0].nX), nY + (int)(dPointSize*pDoc->grdoc.psGraph[i].piPoint[0].nY) ); j1 < TMC_GROUT_POINT_TYPE; j1++)
				pDC->LineTo(nX + (int)(dPointSize*pDoc->grdoc.psGraph[i].piPoint[j1].nX), nY + (int)(dPointSize*pDoc->grdoc.psGraph[i].piPoint[j1].nY) );

   pDC->SelectObject(pOldPen);


}

void CTMCGROUTView::OnDrawGraph3(CDC *pDC, int i)
{
	int nX, nY, nX1, nY1;
	CString text;

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

   text.Format("- %s - %d %d %d %d"  ,	pDoc->grdoc.psGraph[i].szGrapPodp,
										pDoc->grdoc.psGraph[i].nInp1,
										pDoc->grdoc.psGraph[i].nMod1,
										pDoc->grdoc.psGraph[i].nInp2,
										pDoc->grdoc.psGraph[i].nMod2);

   VTextOut( pDC,  nX1+400, nY1,text);

}

void CTMCGROUTView::OnDrawLine( CDC *pDC, int i, int nX, int nY )
{
	if( tmcgrwin.psGraph == NULL ) return;
	
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
	dlg.m_dXUnit = dXAxiesUserUnit;
	dlg.m_csXUserName.Format("%s", gr_typX[4].pszXname);
	dlg.m_csXFormat = csXAxiesFormat;
	dlg.m_csYFormat = csYAxiesFormat;
	
	dlg.m_XUnit = tmcgrwin.nXType;
	dlg.m_YUnit = tmcgrwin.nYType;
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
	dXAxiesUserUnit = dlg.m_dXUnit;
	strcpy( gr_typX[4].pszXname, dlg.m_csXUserName );
	
	pDoc->dXAxiesUserUnit = dXAxiesUserUnit;
	pDoc->csXAxiesUserName.Format("%s", dlg.m_csXUserName);
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
				dlg.m_np1_1 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_1 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_1 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_1 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_1 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case  1:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_2.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName2.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_2 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_2 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_2 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_2 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_2 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case  2:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_3.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName3.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_3 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_3 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_3 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_3 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_3 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case  3:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_4.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName4.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_4 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_4 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_4 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_4 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_4 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case  4:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_5.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName5.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_5 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_5 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_5 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_5 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_5 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case  5:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_6.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName6.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_6 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_6 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_6 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_6 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_6 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case  6:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_7.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName7.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_7 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_7 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_7 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_7 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_7 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case  7:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_8.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName8.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_8 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_8 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_8 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_8 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_8 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case  8:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_9.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName9.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_9 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_9 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_9 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_9 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_9 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case  9:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_10.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName10.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_10 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_10 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_10 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_10 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_10 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case  10:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_11.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName11.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_11 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_11 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_11 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_11 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_11 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case  11:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_12.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName12.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_12 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_12 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_12 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_12 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_12 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case 12:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_13.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName13.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_13 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_13 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_13 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_13 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_13 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case 13:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_14.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName14.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_14 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_14 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_14 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_14 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_14 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case 14:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_15.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName15.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_15 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_15 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_15 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_15 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_15 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case 15:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_16.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName16.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_16 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_16 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_16 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_16 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_16 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case 16:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_17.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName17.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_17 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_17 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_17 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_17 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_17 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case 17:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_18.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName18.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_18 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_18 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_18 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_18 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_18 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case 18:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_19.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName19.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_19 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_19 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_19 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_19 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_19 = pDoc->grdoc.psGraph[i].OutFlag;
			}
			break;
		case 19:
			if( i <  pDoc->grdoc.nGraph)
			{
				dlg.m_SFileName_20.Format("%s", pDoc->grdoc.psGraph[i].szGrapName );
				dlg.m_GraphName20.Format("%s", pDoc->grdoc.psGraph[i].szGrapPodp );
				dlg.m_np1_20 = pDoc->grdoc.psGraph[i].nInp1;
				dlg.m_np2_20 = pDoc->grdoc.psGraph[i].nInp2;
				dlg.m_mod1_20 = pDoc->grdoc.psGraph[i].nMod1;
				dlg.m_mod2_20 = pDoc->grdoc.psGraph[i].nMod2;
				dlg.m_OutGrFlag_20 = pDoc->grdoc.psGraph[i].OutFlag;
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
		if( pDoc->DocFileNewData( n ) )          //allocation memory for new data and set standart data
		{
			i = 0;
			if(  dlg.m_SFileName_1.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_1);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName1 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_1;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_1;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_1;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_1;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_1;
				i++;
			}
			if(  dlg.m_SFileName_2.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_2);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName2 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_2;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_2;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_2;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_2;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_2;
				i++;
			}
			if(  dlg.m_SFileName_3.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_3);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName3 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_3;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_3;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_3;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_3;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_3;
				i++;
			}
			if(  dlg.m_SFileName_4.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_4);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName4 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_4;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_4;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_4;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_4;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_4;
				i++;
			}
			if(  dlg.m_SFileName_5.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_5);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName5 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_5;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_5;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_5;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_5;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_5;
				i++;
			}
			if(  dlg.m_SFileName_6.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_6);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName6 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_6;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_6;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_6;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_6;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_6;
				i++;
			}
			if(  dlg.m_SFileName_7.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_7);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName7 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_7;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_7;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_7;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_7;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_7;
				i++;
			}
			if(  dlg.m_SFileName_8.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_8);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName8 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_8;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_8;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_8;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_8;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_8;
				i++;
			}
			if(  dlg.m_SFileName_9.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_9);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName9 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_9;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_9;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_9;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_9;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_9;
				i++;
			}
			if(  dlg.m_SFileName_10.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_10);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName10 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_10;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_10;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_10;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_10;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_10;
				i++;
			}
			if(  dlg.m_SFileName_11.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_11);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName11 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_11;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_11;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_11;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_11;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_11;
				i++;
			}
			if(  dlg.m_SFileName_12.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_12);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName12 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_12;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_12;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_12;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_12;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_12;
				i++;
			}
			if(  dlg.m_SFileName_13.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_13);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName13 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_13;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_13;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_13;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_13;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_13;
				i++;
			}
			if(  dlg.m_SFileName_14.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_14);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName14 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_14;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_14;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_14;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_14;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_14;
				i++;
			}
			if(  dlg.m_SFileName_15.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_15);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName15 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_15;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_15;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_15;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_15;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_15;
				i++;
			}
			if(  dlg.m_SFileName_16.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_16);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName16 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_16;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_16;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_16;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_16;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_16;
				i++;
			}
			if(  dlg.m_SFileName_17.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_17);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName17 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_17;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_17;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_17;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_17;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_17;
				i++;
			}
			if(  dlg.m_SFileName_18.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_18);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName18 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_18;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_18;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_18;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_18;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_18;
				i++;
			}
			if(  dlg.m_SFileName_19.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_19);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName19 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_19;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_19;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_19;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_19;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_19;
				i++;
			}
			if(  dlg.m_SFileName_20.GetLength() != 0 )
			{
				strcpy( pDoc->grdoc.psGraph[i].szGrapName,  dlg.m_SFileName_20);
				strcpy( pDoc->grdoc.psGraph[i].szGrapPodp, dlg.m_GraphName20 );
				pDoc->grdoc.psGraph[i].nInp1 = dlg.m_np1_20;
				pDoc->grdoc.psGraph[i].nInp2 = dlg.m_np2_20;
				pDoc->grdoc.psGraph[i].nMod1 = dlg.m_mod1_20;
				pDoc->grdoc.psGraph[i].nMod2 = dlg.m_mod2_20;
				pDoc->grdoc.psGraph[i].OutFlag = dlg.m_OutGrFlag_20;
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
	csBuf.Format( "%s\\*.s;*.s2p;*.s4p;*.s6p;*.s8p;*.tab", ch );

	CFileDialog dlg( TRUE, "", csBuf, OFN_ENABLESIZING, "Tamic S-matrix");
	dlg.m_ofn.lpstrTitle = "Add *.s characteristics";
//	strcpy( dlg.m_ofn.lpstrFile, m_SFileName );
	if( dlg.DoModal() == IDOK )
	{
		strcpy( ch, dlg.GetPathName() );
		pDoc->AddGraphicsInDoc( ch );
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
//	IvalidateRectView( rectOutXY );
	PutTrace1();

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
	CFileDialog dlg( FALSE, "soc", "*.soc");
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
//	rectClient.NormalizeRect();
//	pDC->FillSolidRect( &rectClient, pDoc->scBackgoundColor );
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
	csBuf.Format("%s", gr_typY[j].pszYname );
	
	PutStatistics( csBuf );
	
	return;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
//void PutStatistics111( char *szTrace )
//{
//	CString csBuf;
//	csBuf.Format("%s", szTrace);
//	AfxMessageBox(csBuf);		
//	return;
//}
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

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

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
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
		ReadDocFileW();
		pDoc->WriteDocFile();
		RedrawViewGr();	
	};

	return;
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

void CTMCGROUTView::OnViewTranslateHome() 
{
	// TODO: Add your command handler code here
	if( tmcgrwin.sGrWin.nAFlagX ) return;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	double dXMin, dDeltaX;
	int i, j;

	dDeltaX = tmcgrwin.sGrWin.Xmax - tmcgrwin.sGrWin.Xmin;
	dXMin = 0;

	j = 0;

	for( i = 0; i < pDoc->grdoc.nGraph; i++ )
	{
		if( pDoc->grdoc.psGraph[i].OutFlag )
		{
			if( j == 0 )
			{
				j = 1;
				dXMin = pDoc->grdoc.psGraph[i].pFreq[0];
			}
			else
			{
				if( pDoc->grdoc.psGraph[i].pFreq[0] < dXMin )
				{
					dXMin = pDoc->grdoc.psGraph[i].pFreq[0];
				};
			};
		};
	};
	if( j != 0 )
	{
		tmcgrwin.sGrWin.Xmin = dXMin*GetXAxiesUnit();
		tmcgrwin.sGrWin.Xmax = tmcgrwin.sGrWin.Xmin + dDeltaX;
	};

	if( j == 0 ) return;

	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	pDoc->WriteDocFile();

	RedrawViewGr();	
	
	return;
}

void CTMCGROUTView::OnViewTranslateEnd() 
{
	// TODO: Add your command handler code here
	if( tmcgrwin.sGrWin.nAFlagX ) return;

	CTMCGROUTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	double dXMax, dDeltaX;
	int i, j;

	dDeltaX = tmcgrwin.sGrWin.Xmax - tmcgrwin.sGrWin.Xmin;
	dXMax = 0;

	j = 0;

	for( i = 0; i < pDoc->grdoc.nGraph; i++ )
	{
		if( pDoc->grdoc.psGraph[i].OutFlag )
		{
			if( j == 0 )
			{
				j = 1;
				dXMax = pDoc->grdoc.psGraph[i].pFreq[ pDoc->grdoc.psGraph[i].nPoint-1 ];
			}
			else
			{
				if( pDoc->grdoc.psGraph[i].pFreq[  pDoc->grdoc.psGraph[i].nPoint-1  ] > dXMax )
				{
					dXMax = pDoc->grdoc.psGraph[i].pFreq[  pDoc->grdoc.psGraph[i].nPoint-1  ];
				};
			};
		};
	};

	if( j == 0 ) return;

	if( j != 0 )
	{
		tmcgrwin.sGrWin.Xmax = dXMax*GetXAxiesUnit();
		tmcgrwin.sGrWin.Xmin = tmcgrwin.sGrWin.Xmax - dDeltaX;
	};

	pDoc->grdoc.nAFlagX = tmcgrwin.sGrWin.nAFlagX;
	pDoc->grdoc.Xmin = tmcgrwin.sGrWin.Xmin;
	pDoc->grdoc.Xmax = tmcgrwin.sGrWin.Xmax;
	pDoc->WriteDocFile();

	RedrawViewGr();	
	
	return;
}

double CTMCGROUTView::GetXAxiesUnit()
{
	double x;

	switch(tmcgrwin.nXType)
	{
	case TMC_GR_TYPE_FGHz:
		x = (1.e-9);
		break;
	case TMC_GR_TYPE_FMHz:
		x = (1.e-6);
		break;
	case TMC_GR_TYPE_FKHz:
		x = (1.e-3);
		break;
	case TMC_GR_TYPE_FHz:
		x = (1.);
		break;
	case TMC_GR_TYPE_F_USER:
		x = (dXAxiesUserUnit);
		break;
	default:
		x = (1.e-9);
		break;
	}
	return x;
}
