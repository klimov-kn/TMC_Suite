// FldViewView.cpp : implementation of the CFldViewView class
//

#include "stdafx.h"
#include "MainFrm.h"
#include "FldView.h"

#include <io.h>
#include <fcntl.h>
#include "FldViewDoc.h"
#include "FldViewView.h"
#include "TMCDialogPropet.h"
#include <tmcgrviw.h>
#include <TmcLibError.h>
#include "TmcGLText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


unsigned char threeto8[8] = 
{
	0, 0111>>1, 0222>>1, 0333>>1, 0444>>1, 0555>>1, 0666>>1, 0377
};

unsigned char twoto8[4] = 
{
	0, 0x55, 0xaa, 0xff
};

unsigned char oneto8[2] = 
{
	0, 255
};

static int defaultOverride[13] = 
{
	0, 3, 24, 27, 64, 67, 88, 173, 181, 236, 247, 164, 91
};

static PALETTEENTRY defaultPalEntry[20] = 
{
	{ 0,   0,   0,    0 },
	{ 0x80,0,   0,    0 },
	{ 0,   0x80,0,    0 },
	{ 0x80,0x80,0,    0 },
	{ 0,   0,   0x80, 0 },
	{ 0x80,0,   0x80, 0 },
	{ 0,   0x80,0x80, 0 },
	{ 0xC0,0xC0,0xC0, 0 },

	{ 192, 220, 192,  0 },
	{ 166, 202, 240,  0 },
	{ 255, 251, 240,  0 },
	{ 160, 160, 164,  0 },

	{ 0x80,0x80,0x80, 0 },
	{ 0xFF,0,   0,    0 },
	{ 0,   0xFF,0,    0 },
	{ 0xFF,0xFF,0,    0 },
	{ 0,   0,   0xFF, 0 },
	{ 0xFF,0,   0xFF, 0 },
	{ 0,   0xFF,0xFF, 0 },
	{ 0xFF,0xFF,0xFF, 0 }
};


/////////////////////////////////////////////////////////////////////////////
// CFldViewView

IMPLEMENT_DYNCREATE(CFldViewView, CScrollView)

BEGIN_MESSAGE_MAP(CFldViewView, CScrollView)
	//{{AFX_MSG_MAP(CFldViewView)
	ON_MESSAGE( WM_USERAPPLY_PROPPLACE, OnUserApplyPropDialogPlace )
	ON_MESSAGE( WM_USERAPPLY_PROPFIELD, OnUserApplyPropDialogField )
	ON_MESSAGE( WM_USERAPPLY_PROPEPS, OnUserApplyPropDialogEps )
	ON_MESSAGE( WM_USERAPPLY_PROPTOPOL, OnUserApplyPropDialogTopol )
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_ROTATE_LEFT, OnRotateLeft)
	ON_COMMAND(ID_ROTATE_RIGHT, OnRotateRight)
	ON_COMMAND(ID_ROTATE_TOP, OnRotateTop)
	ON_COMMAND(ID_ROTATE_BOTTOM, OnRotateBottom)
	ON_COMMAND(ID_ROTATE_LEFTY, OnRotateLefty)
	ON_COMMAND(ID_ROTATE_RIGHTY, OnRotateRighty)
	ON_COMMAND(ID_TRANSLATE_XT, OnTranslateXt)
	ON_COMMAND(ID_TRANSLATE_XB, OnTranslateXb)
	ON_COMMAND(ID_TRANSLATE_ZB, OnTranslateZb)
	ON_COMMAND(ID_TRANSLATE_ZT, OnTranslateZt)
	ON_COMMAND(ID_TRANSLATE_YB, OnTranslateYb)
	ON_COMMAND(ID_TRANSLATE_YT, OnTranslateYt)
	ON_WM_TIMER()
	ON_COMMAND(ID_VIEW_PARAMETERS, OnViewParameters)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_STRELKLEFT, OnStrelkleft)
	ON_COMMAND(ID_STRELKRIGHT, OnStrelkright)
	ON_COMMAND(ID_STRELKUP, OnStrelkup)
	ON_COMMAND(ID_STRELKDOWN, OnStrelkdown)
	ON_COMMAND(ID_VIEW_ZOOM_P, OnViewZoomP)
	ON_COMMAND(ID_VIEW_ZOOM_M, OnViewZoomM)
	ON_COMMAND(ID_VIEW_ZOOM_PX, OnViewZoomPx)
	ON_COMMAND(ID_VIEW_ZOOM_MX, OnViewZoomMx)
	ON_COMMAND(ID_VIEW_ZOOM_MY, OnViewZoomMy)
	ON_COMMAND(ID_VIEW_ZOOM_PY, OnViewZoomPy)
	ON_COMMAND(ID_VIEW_DEFAULT12, OnViewDefault)
	ON_COMMAND(ID_ROTATEXN, OnRotatexn)
	ON_COMMAND(ID_ROTATEXV, OnRotatexv)
	ON_COMMAND(ID_ROTATEYN, OnRotateyn)
	ON_COMMAND(ID_ROTATEYV, OnRotateyv)
	ON_COMMAND(ID_ROTATEZV, OnRotatezv)
	ON_COMMAND(ID_ROTATEZN, OnRotatezn)
	ON_COMMAND(ID_TRANSLATE_XN, OnTranslateXn)
	ON_COMMAND(ID_TRANSLATE_XV, OnTranslateXv)
	ON_COMMAND(ID_TRANSLATE_YN, OnTranslateYn)
	ON_COMMAND(ID_TRANSLATE_YV, OnTranslateYv)
	ON_COMMAND(ID_TRANSLATE_ZN, OnTranslateZn)
	ON_COMMAND(ID_TRANSLATE_ZV, OnTranslateZv)
	ON_COMMAND(ID_TRANSLATEDATAXN, OnTranslatedataxn)
	ON_COMMAND(ID_TRANSLATEDATAXV, OnTranslatedataxv)
	ON_COMMAND(ID_TRANSLATEDATAYN, OnTranslatedatayn)
	ON_COMMAND(ID_TRANSLATEDATAYV, OnTranslatedatayv)
	ON_COMMAND(ID_DIMENSION23SWITCH, OnDimension23switch)
	ON_COMMAND(ID_FLAGDRAWSURFACE, OnFlagdrawsurface)
	ON_COMMAND(ID_FLAGDRAWLINE, OnFlagdrawline)
	ON_UPDATE_COMMAND_UI(ID_FLAGDRAWLINE, OnUpdateFlagdrawline)
	ON_UPDATE_COMMAND_UI(ID_FLAGDRAWSURFACE, OnUpdateFlagdrawsurface)
	ON_UPDATE_COMMAND_UI(ID_DIMENSION23SWITCH, OnUpdateDimension23switch)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_FLAGDRAWLINEEPS, OnFlagdrawlineeps)
	ON_COMMAND(ID_FLAGDRAWSURFACEEPS, OnFlagdrawsurfaceeps)
	ON_COMMAND(ID_DIMENSION23SWITCHEPS, OnDimension23switcheps)
	ON_UPDATE_COMMAND_UI(ID_DIMENSION23SWITCHEPS, OnUpdateDimension23switcheps)
	ON_UPDATE_COMMAND_UI(ID_FLAGDRAWLINEEPS, OnUpdateFlagdrawlineeps)
	ON_UPDATE_COMMAND_UI(ID_FLAGDRAWSURFACEEPS, OnUpdateFlagdrawsurfaceeps)
	ON_COMMAND(ID_VIEW_PROPORTIONALLY, OnViewProportionally)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROPORTIONALLY, OnUpdateViewProportionally)
	ON_COMMAND(ID_FLAGDRAWLINETOP, OnFlagdrawlinetop)
	ON_UPDATE_COMMAND_UI(ID_FLAGDRAWLINETOP, OnUpdateFlagdrawlinetop)
	ON_COMMAND(ID_FLAGDRAWSURFACETOP, OnFlagdrawsurfacetop)
	ON_UPDATE_COMMAND_UI(ID_FLAGDRAWSURFACETOP, OnUpdateFlagdrawsurfacetop)
	ON_COMMAND(ID_DIMENSION23SWITCHTOP, OnDimension23switchtop)
	ON_UPDATE_COMMAND_UI(ID_DIMENSION23SWITCHTOP, OnUpdateDimension23switchtop)
	ON_COMMAND(ID_VIEW_TOPOLOGY_BOUNDARY_EPS, OnViewTopologyBoundaryEps)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOPOLOGY_BOUNDARY_EPS, OnUpdateViewTopologyBoundaryEps)
	ON_COMMAND(ID_VIEW_TOPOLOGY_BOUNDARY_METAL, OnViewTopologyBoundaryMetal)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOPOLOGY_BOUNDARY_METAL, OnUpdateViewTopologyBoundaryMetal)
	ON_COMMAND(ID_VIEW_TOPOLOGY_BOUNDARY_MAGNETIC, OnViewTopologyBoundaryMagnetic)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOPOLOGY_BOUNDARY_MAGNETIC, OnUpdateViewTopologyBoundaryMagnetic)
	ON_COMMAND(ID_VIEW_TOPOLOGY_BOUNDARY_INPUT, OnViewTopologyBoundaryInput)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOPOLOGY_BOUNDARY_INPUT, OnUpdateViewTopologyBoundaryInput)
	ON_COMMAND(ID_VIEW_TOPOLOGY_BOUNDARY_ABSORBER, OnViewTopologyBoundaryAbsorber)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOPOLOGY_BOUNDARY_ABSORBER, OnUpdateViewTopologyBoundaryAbsorber)
	ON_COMMAND(ID_VIEW_FIELD_SIZE, OnViewFieldSize)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIELD_SIZE, OnUpdateViewFieldSize)
	ON_COMMAND(ID_VIEW_EPS_SIZE, OnViewEpsSize)
	ON_UPDATE_COMMAND_UI(ID_VIEW_EPS_SIZE, OnUpdateViewEpsSize)
	ON_COMMAND(ID_VIEW_TOPOLOGY_DIMENSIONS_ALONGANXAXIES, OnViewTopologyDimensionsAlonganxaxies)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOPOLOGY_DIMENSIONS_ALONGANXAXIES, OnUpdateViewTopologyDimensionsAlonganxaxies)
	ON_COMMAND(ID_VIEW_TOPOLOGY_DIMENSIONS_ALONGANYAXIES, OnViewTopologyDimensionsAlonganyaxies)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOPOLOGY_DIMENSIONS_ALONGANYAXIES, OnUpdateViewTopologyDimensionsAlonganyaxies)
	ON_COMMAND(ID_VIEW_TOPOLOGY_SIZE_BLOCK, OnViewTopologySizeBlock)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOPOLOGY_SIZE_BLOCK, OnUpdateViewTopologySizeBlock)
	ON_COMMAND(ID_VIEW_TOPOLOGY_SIZE_LINKLIST, OnViewTopologySizeLinklist)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOPOLOGY_SIZE_LINKLIST, OnUpdateViewTopologySizeLinklist)
	ON_MESSAGE(WM_COLOR_LEVEL_DIALOG_BYE, OnCloseColorLevelDialog)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_COMMAND(ID_FIELD_L, OnFieldL)
	ON_COMMAND(ID_FIELD_VALUE, OnFieldValue)
	ON_UPDATE_COMMAND_UI(ID_FIELD_VALUE, OnUpdateFieldValue)
	ON_COMMAND(ID_FIELD_MODUL, OnFieldModul)
	ON_UPDATE_COMMAND_UI(ID_FIELD_MODUL, OnUpdateFieldModul)
	END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFldViewView construction/destruction

CFldViewView::CFldViewView()
{
	// TODO: add construction code here
	int i;

	nPropertyPageInd = 0;

	csTplFileName.Format("");
	
	bIsMouseLButonDown = FALSE;
    bBusy = FALSE;
	bDrawFlagAxies = FALSE;
	bProportionally = TRUE;
	dXkoefProp = 1.;
	dYkoefProp = 1.;

	bDrawFieldModulFlag = FALSE;
	bHightColorRezolution = FALSE;
	bDrawFlagAxiesXSize = FALSE;
	bDrawFlagAxiesYSize = FALSE;
	bDrawFlagAxiesZFieldSize = FALSE;
	bDrawFlagAxiesZEpsSize = FALSE;
	bDrawFieldValueFlag = FALSE;
	dDrawFieldValueX = 0.;
	dDrawFieldValueY = 0.;

	for( i = 0; i < 20; i++ )	pscSurfaceColor_Eps[19-i] = RGB( 255-i*10, 255, i*10);

	pcGLText = NULL;
	dDelta = 0.;
	dZtopol = 0.25;
	dZtopolStrelk = 0.02;

	m_pDC = NULL;
	m_pOldPalette = NULL;
	m_play = FALSE;

	SetDefaultValue();

	nX = 0;
	nY = 0;
	nBoundBlockTpl = 0;
	nTopPolygon = 0;
	nTopSize = 0;
	nInputNum = 0;

	pcTopSize = NULL;
	pnTopType = NULL;
	pdTopX1 = NULL;
	pdTopX2 = NULL;
	pdTopX3 = NULL;
	pdTopX4 = NULL;
	pdTopY1 = NULL;
	pdTopY2 = NULL;
	pdTopY3 = NULL;
	pdTopY4 = NULL;
	pdTopXinp = NULL;
	pdTopYinp = NULL;		

	pdSurface  = NULL;
	pdSurface_Eps = NULL;
	pdSurfaceX = NULL;
	pdSurfaceY = NULL;
	pcBoundBlockTpl = NULL;

	scBackgoundColor = RGB( 0, 0, 0);

/*
	pscSurfaceColor[  0 ] = RGB( 25,125, 225);
	pscSurfaceColor[  1 ] = RGB( 50,100, 200);
	pscSurfaceColor[  2 ] = RGB( 75, 75, 175);
	pscSurfaceColor[  3 ] = RGB(100, 55, 150);
	pscSurfaceColor[  4 ] = RGB(125, 25, 125);
	pscSurfaceColor[  5 ] = RGB(150, 25, 125);
	pscSurfaceColor[  6 ] = RGB(200, 25, 100);
	pscSurfaceColor[  7 ] = RGB(225, 50, 100);
	pscSurfaceColor[  8 ] = RGB(250, 75, 100);
	pscSurfaceColor[  9 ] = RGB(225,100, 100);
	pscSurfaceColor[ 10 ] = RGB(200,125,  75);
	pscSurfaceColor[ 11 ] = RGB(175,150, 100);
	pscSurfaceColor[ 12 ] = RGB(150,175, 125);
	pscSurfaceColor[ 13 ] = RGB(125,200, 150);
	pscSurfaceColor[ 14 ] = RGB(100,225, 175);
	pscSurfaceColor[ 15 ] = RGB(100,250, 100);
	pscSurfaceColor[ 16 ] = RGB(125,225, 125);
	pscSurfaceColor[ 17 ] = RGB(225,225,  50);
	pscSurfaceColor[ 18 ] = RGB(255,255,   0);
	pscSurfaceColor[ 19 ] = RGB(255,255, 255);
*/
	for( i = 0; i < 7; i++ )
	{
		pscSurfaceColor[ i ] = RGB( 100, i*35, 255-i*35 );
	};
	for( i = 7; i < 14; i++ )
	{
		pscSurfaceColor[ i ] = RGB( (i-7)*35, 255-(i-7)*35, 150 );
	};
	for( i = 14; i < 18; i++ )
	{
		pscSurfaceColor[ i ] = RGB( 255, (i-14)*83, 100 );
	};
	pscSurfaceColor[ 18 ] = RGB( 255, 255,   0 );
	pscSurfaceColor[ 19 ] = RGB( 255, 255, 255 );
	
	scColorEps = RGB(   0, 230, 230 );
	scColorMet = RGB( 210, 210, 210 );
	scColorMag = RGB( 120, 230, 200 );
	scColorInp = RGB( 255,   0, 255 );
	scColorAbs = RGB( 200, 100, 100 );

	pcColorLevelDialog = NULL;
	pcColorLevelDialog = new CColorLevelDlg( this );

	return;
}

CFldViewView::~CFldViewView()
{
	GL_Close();
	DeleteData();
	if( pcColorLevelDialog != NULL ) delete pcColorLevelDialog;
	pcColorLevelDialog = NULL;
	return;
}

BOOL CFldViewView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CFldViewView drawing

void CFldViewView::OnDraw(CDC* pDC)
{
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
	if( cError.IsError() ) 
	{
		{
			CRect rectClient;
			GetClientRect( &rectClient );
			CClientDC dc(this);
			OnPrepareDC(&dc);
			dc.DPtoLP(&rectClient);  
			rectClient.NormalizeRect();
			pDC->FillSolidRect( &rectClient, scBackgoundColor );
		};
		CString text;
		text.Format( "Error : %s" , cError.GetErrorMessage() );
		VTextOut( pDC, 10, 0,text);
//		pDC->TextOut( 10,00,text);
		return;
	};

	GL_Resize();
	DrawScene();
	return;
}

void CFldViewView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	SetDefaultValue();

	// TODO: calculate the total size of this view
	CSize sizeTotal( (int)(TMC_VIEW_XSIZE*0.01), (int)(TMC_VIEW_YSIZE*0.01) );
	CSize sizePage(sizeTotal.cx/2, sizeTotal.cy/2);
	CSize sizeLine(sizeTotal.cx/50, sizeTotal.cy/50);
	SetScrollSizes(TMC_VIEW_SIZEMODE, sizeTotal, sizePage, sizeLine);

	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pDoc->AddViews__();

	CString csBuffer;
	int i, j, k, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12;
	double r, r1;

// read property page number
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLDNUMPROPPAGE_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d", &i );
		nPropertyPageInd = i;
	};	

// read hight resolution color flag
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLDCOLORRESOL_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d", &i );
		bHightColorRezolution = i;
	};	

// read field modul flag
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLDMODULFL_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d", &i );
		bDrawFieldModulFlag = i;
	};	

// read Color for draw topology block
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLDTOPOLOGYCOLOR_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d %d %d %d %d", &i1, &i2, &i3, &i4, &i5 );
		scColorEps = i1;
		scColorMet = i2;
		scColorMag = i3; 
		scColorInp = i4;
		scColorAbs = i5;
	};	
// read topology parameters draw flag
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLDTOPOLOGY_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d %d %d %d %d %d %d %d %d %d %d %d", &i1, &i2, &i3, &i4, &i5, &i6, &i7, &i8, &i9, &i10, &i11, &i12 );
		bDrawFlagTopology		= i1;
		bDrawLineFlagTopology	= i2;
		bDrawSurfaceFlagTopology= i3; 
		n2D3DFlagTopology		= i4;
		nBlendTopology			= i5;
		bTopologySizeFlag		= i6;
		bTopologySizeFlagLinkList= i7;
		bDrawFlagMagnetic		= i8;
		bDrawFlagMetal			= i9;
		bDrawFlagEps			= i10;
		bDrawFlagInput			= i11;
		bDrawFlagAbsorber		= i12;
	};	
// read draw flag
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_DRAWFLAG_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d %d", &i, &i1);
		bDrawFlag = i;
		bProportionally = i1;
	};
// read draw flag for surface
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_DRFLSURF_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d", &i);
		bDrawSurfaceFlag = i;
	};
// read draw flag for line
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_DRFLLINE_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d", &i);
		bDrawLineFlag = i;
	};
// read X-size flag
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_FLXSIZE_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d", &i);
		bXsizeFlag = i;
	};
// read Y-size flag
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_FLYSIZE_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d", &i);
		bYsizeFlag = i;
	};
// read Z-size flag
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_FLZSIZE_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d", &i);
		bZsizeFlag = i;
	};
// read 2 or 3 dimension for draw
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_FL23D_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d", &i);
		n2D3DFlag = i;
	};
// read blend
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_BLEND_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d", &i);
		nBlend = i;
	};
// read Xmin
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_XMIN_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%lg", &dXmin);
	};
// read Xmax
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_XMAX_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%lg", &dXmax);
	};
// read Ymin
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_YMIN_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%lg", &dYmin);
	};
// read Ymax
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_YMAX_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%lg", &dYmax);
	};
// read Zmin
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_ZMIN_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%lg", &dZmin);
	};
// read Zmax
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_ZMAX_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%lg", &dZmax);
	};
// read XAngle
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_XANGLE_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%lg", &r);
		wAngleX = (GLfloat)r;
	};
// read YAngle
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_YANGLE_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%lg", &r);
		wAngleY = (GLfloat)r;
	};
// read ZAngle
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_ZANGLE_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%lg", &r);
		wAngleZ = (GLfloat)r;
	};
// read XTranslate
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_XTRANS_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%lg", &r);
		wTranslateX = (GLfloat)r;
	};
// read YTranslate
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_YTRANS_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%lg", &r);
		wTranslateY = (GLfloat)r;
	};
// read ZTranslate
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_ZTRANS_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%lg", &r);
		wTranslateZ = (GLfloat)r;
	};
// read DrawFlagAxies
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_DRFLAXIS_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d", &i);
		bDrawFlagAxies = i;
	};
// read Axies Color
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_COLORAXIS_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d %d %d", &i, &j, &k);
		scAxiesColor = RGB( i, j, k );
	};
// read Draw Axies Size
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLDDRFLAXISSZ_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d %d %d %d", &i1, &i2, &i3, &i4);
		bDrawFlagAxiesXSize = i1;
		bDrawFlagAxiesYSize = i2;
		bDrawFlagAxiesZFieldSize = i3;
		bDrawFlagAxiesZEpsSize   = i4;
	};
// read parameter for draw Eps(x,y) data
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLDTOPEPSPAR_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d %d %d %d %d %d %lg %lg", &i1, &i2, &i3, &i4, &i5, &i6, &r, &r1);
		n2D3DFlag_Eps					= i1;
		nBlend_Eps						= i2;
		bDrawLineFlag_Eps				= i3;
		bDrawSurfaceFlag_Eps			= i4;
		bZsizeFlag_Eps					= i5;
		bDrawFlagAxiesZFieldSize_Eps	= i6;
		dZmax_Eps	=  r;
		dZmin_Eps	= r1;
	};
// read Axies Font name
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_FONTNAMEAXIS_INI );
	if( csBuffer.GetLength() != 0 )
	{
	    strcpy( lfAxiesFont.lfFaceName, csBuffer);
	};
// read Axies Font
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_FONTAXIS_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d %d %d %d %d %d %d %d %d %d %d %d %d", &i, &i1, &i2, &i3, &i4, &i5, &i6, &i7, &i8, &i9, &i10, &i11, &i12);
	    lfAxiesFont.lfHeight  = i;
		lfAxiesFont.lfCharSet = (unsigned char)i1;
		lfAxiesFont.lfWeight  = i2;
		lfAxiesFont.lfWidth   = i3;
		lfAxiesFont.lfOutPrecision  = (unsigned char)i4;
		lfAxiesFont.lfClipPrecision = (unsigned char)i5;
		lfAxiesFont.lfItalic    = (unsigned char)i6;
		lfAxiesFont.lfUnderline = (unsigned char)i7;
		lfAxiesFont.lfStrikeOut = (unsigned char)i8;
		lfAxiesFont.lfEscapement = i9;
		lfAxiesFont.lfOrientation = i10;
		lfAxiesFont.lfQuality = (unsigned char)i11;
		lfAxiesFont.lfPitchAndFamily = (unsigned char)i12;
	};
// read Color for draw Z Axies field
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_COLORZFIELD_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
			&( pscSurfaceColor[ 0] ),
			&( pscSurfaceColor[ 1] ),
			&( pscSurfaceColor[ 2] ),
			&( pscSurfaceColor[ 3] ),
			&( pscSurfaceColor[ 4] ),
			&( pscSurfaceColor[ 5] ),
			&( pscSurfaceColor[ 6] ),
			&( pscSurfaceColor[ 7] ),
			&( pscSurfaceColor[ 8] ),
			&( pscSurfaceColor[ 9] ),
			&( pscSurfaceColor[10] ),
			&( pscSurfaceColor[11] ),
			&( pscSurfaceColor[12] ),
			&( pscSurfaceColor[13] ),
			&( pscSurfaceColor[14] ),
			&( pscSurfaceColor[15] ),
			&( pscSurfaceColor[16] ),
			&( pscSurfaceColor[17] ),
			&( pscSurfaceColor[18] ),
			&( pscSurfaceColor[19] ) );
	};
// read Color for draw Z Axies eps
	csBuffer = AfxGetApp()->GetProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_COLORZEPS_INI );
	if( csBuffer.GetLength() != 0 )
	{
		sscanf( csBuffer, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
			&( pscSurfaceColor_Eps[ 0] ),
			&( pscSurfaceColor_Eps[ 1] ),
			&( pscSurfaceColor_Eps[ 2] ),
			&( pscSurfaceColor_Eps[ 3] ),
			&( pscSurfaceColor_Eps[ 4] ),
			&( pscSurfaceColor_Eps[ 5] ),
			&( pscSurfaceColor_Eps[ 6] ),
			&( pscSurfaceColor_Eps[ 7] ),
			&( pscSurfaceColor_Eps[ 8] ),
			&( pscSurfaceColor_Eps[ 9] ),
			&( pscSurfaceColor_Eps[10] ),
			&( pscSurfaceColor_Eps[11] ),
			&( pscSurfaceColor_Eps[12] ),
			&( pscSurfaceColor_Eps[13] ),
			&( pscSurfaceColor_Eps[14] ),
			&( pscSurfaceColor_Eps[15] ),
			&( pscSurfaceColor_Eps[16] ),
			&( pscSurfaceColor_Eps[17] ),
			&( pscSurfaceColor_Eps[18] ),
			&( pscSurfaceColor_Eps[19] ) );
	};



	GL_Init();
//	LastWriteTime1 = pDoc->GetLastWriteFile();
	SetTimer( 1, 50, NULL);
//	if( !pDoc->IsFileRead() ) ReadData();
//	else PrepareData();
	ReadData();

	return;
}

/////////////////////////////////////////////////////////////////////////////
// CFldViewView printing

BOOL CFldViewView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CFldViewView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CFldViewView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CFldViewView diagnostics

#ifdef _DEBUG
void CFldViewView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CFldViewView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CFldViewDoc* CFldViewView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFldViewDoc)));
	return (CFldViewDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFldViewView message handlers

void CFldViewView::GL_Init( void )
{
//	if( !(pDoc->GetDrawFlag()) ) return;
	
	GL_Close();

	pcGLText = new CTmcGLText;
	
	PIXELFORMATDESCRIPTOR pfd;
    int         n;

	GLfloat		fMaxObjSize, fAspect;
	GLfloat		fNearPlane, fFarPlane;

    m_pDC = new CClientDC(this);

    ASSERT(m_pDC != NULL);

    if (!bSetupPixelFormat())
        return;

    n = ::GetPixelFormat(m_pDC->GetSafeHdc());
    ::DescribePixelFormat(m_pDC->GetSafeHdc(), n, sizeof(pfd), &pfd);

    CreateRGBPalette();

	fNearPlane = 3.0f;
	fFarPlane = 7.0f;
	fMaxObjSize = 3.0f;
	m_fRadius = fNearPlane + fMaxObjSize / 2.0f;

//	wTranslateX = (float)(-0.25);
//	wTranslateY = (float)(-0.25);
//	wTranslateZ = -m_fRadius;


    hrc = wglCreateContext(m_pDC->GetSafeHdc());
    wglMakeCurrent(m_pDC->GetSafeHdc(), hrc);
	
	GetClientRect(&m_oldRect);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);

	if (m_oldRect.bottom)
		fAspect = (GLfloat)m_oldRect.right/m_oldRect.bottom;
	else	// don't divide by zero, not that we should ever run into that...
		fAspect = 1.0f;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0f, fAspect, fNearPlane, fFarPlane);
    glMatrixMode(GL_MODELVIEW);
	return;
}

void CFldViewView::GL_Close( void )
{
	wglDeleteContext( hrc );
	if( pcGLText != NULL ) delete pcGLText;
	if(	  m_pDC != NULL ) delete   m_pDC;
	return;
}

unsigned char CFldViewView::ComponentFromIndex(int i, UINT nbits, UINT shift)
{
    unsigned char val;

    val = (unsigned char) (i >> shift);
    switch (nbits) 
	{

    case 1:
        val &= 0x1;
        return oneto8[val];
    case 2:
        val &= 0x3;
        return twoto8[val];
    case 3:
        val &= 0x7;
        return threeto8[val];

    default:
        return 0;
    }

}

void CFldViewView::CreateRGBPalette( void )
{
    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE *pPal;
    int n, i;
 
    n = ::GetPixelFormat(m_pDC->GetSafeHdc());
    ::DescribePixelFormat(m_pDC->GetSafeHdc(), n, sizeof(pfd), &pfd);

    if (pfd.dwFlags & PFD_NEED_PALETTE)
    {
        n = 1 << pfd.cColorBits;
        pPal = (PLOGPALETTE) new char[sizeof(LOGPALETTE) + n * sizeof(PALETTEENTRY)];

        ASSERT(pPal != NULL);

        pPal->palVersion = 0x300;
        pPal->palNumEntries = (unsigned short)(n);
        for (i=0; i<n; i++)
        {
            pPal->palPalEntry[i].peRed =
                    ComponentFromIndex(i, pfd.cRedBits, pfd.cRedShift);
            pPal->palPalEntry[i].peGreen =
                    ComponentFromIndex(i, pfd.cGreenBits, pfd.cGreenShift);
            pPal->palPalEntry[i].peBlue =
                    ComponentFromIndex(i, pfd.cBlueBits, pfd.cBlueShift);
            pPal->palPalEntry[i].peFlags = 0;
        }

        /* fix up the palette to include the default GDI palette */
      if ((pfd.cColorBits == 8)                           &&
            (pfd.cRedBits   == 3) && (pfd.cRedShift   == 0) &&
            (pfd.cGreenBits == 3) && (pfd.cGreenShift == 3) &&
            (pfd.cBlueBits  == 2) && (pfd.cBlueShift  == 6)
           )
        {
			for (i = 1 ; i <= 12 ; i++)
                pPal->palPalEntry[defaultOverride[i]] = defaultPalEntry[i];
        }

        m_cPalette.CreatePalette(pPal);
        delete pPal;

        m_pOldPalette = m_pDC->SelectPalette(&m_cPalette, FALSE);
        m_pDC->RealizePalette();
    }
	
}


void CFldViewView::DrawScene( void )
{
	if( cError.IsError() ) 
	{
//		RedrawWindow();
		return;
	};

	GL_Resize();

    wglMakeCurrent(m_pDC->GetSafeHdc(), hrc);
//	glAlphaFunc( GL_GREATER , 0.5f );
	glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );


	glEnable( GL_POLYGON_STIPPLE );
//	glEnable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();

        glTranslatef( wTranslateX, wTranslateY, wTranslateZ);
        glRotatef(wAngleX, 1.0f, 0.0f, 0.0f);
        glRotatef(wAngleY, 0.0f, 1.0f, 0.0f);
        glRotatef(wAngleZ, 0.0f, 0.0f, 1.0f);

	DrawField();
	DrawEps();
	DrawTopology();
	DrawAxies();
	DrawAxiesSize();
	DrawSize();
	
	glPopMatrix();

    glFinish();

    SwapBuffers(wglGetCurrentDC());

	glDisable( GL_BLEND );
//	glDisable( GL_ALPHA_TEST );
	glDisable( GL_POLYGON_STIPPLE );

	return;
}

GLvoid CFldViewView::GL_Resize( void )
{
    GLfloat aspect;
    GLfloat width, height;

    GetClientRect(&m_oldRect);
	
	width  = (float) (m_oldRect.Width() );
	height = (float)(m_oldRect.Height() );

    glViewport( 0, 0, (int)(width), (int)(height) );

    aspect = (GLfloat) width / height;

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 30.0, aspect, 3.0, 7.0 );
    glMatrixMode( GL_MODELVIEW );
	return;
}


BOOL CFldViewView::bSetupPixelFormat( void )
{
    static PIXELFORMATDESCRIPTOR pfd = 
	{
        sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
        1,                              // version number
        PFD_DRAW_TO_WINDOW |            // support window
          PFD_SUPPORT_OPENGL |          // support OpenGL
          PFD_DOUBLEBUFFER,             // double buffered
        PFD_TYPE_RGBA,                  // RGBA type
        24,                             // 24-bit color depth
        0, 0, 0, 0, 0, 0,               // color bits ignored
        0,                              // no alpha buffer
        0,                              // shift bit ignored
        0,                              // no accumulation buffer
        0, 0, 0, 0,                     // accum bits ignored
        32,                             // 32-bit z-buffer
        0,                              // no stencil buffer
        0,                              // no auxiliary buffer
        PFD_MAIN_PLANE,                 // main layer
        0,                              // reserved
        0, 0, 0                         // layer masks ignored
    };
    int pixelformat;

    if ( (pixelformat = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd)) == 0 )
    {
        MessageBox("ChoosePixelFormat failed");
        return FALSE;
    }

    if (SetPixelFormat(m_pDC->GetSafeHdc(), pixelformat, &pfd) == FALSE)
    {
        MessageBox("SetPixelFormat failed");
        return FALSE;
    }
 
    return TRUE;

}


void CFldViewView::DrawField()
{
	if( cError.IsError() ) return;

	switch( n2D3DFlag )
	{
		case TMCFIELDVIEW_2DDraw:
			DrawFieldSurface2();
			DrawFieldLine2();
			DrawFieldCursore2();
			break;
		case TMCFIELDVIEW_3DDraw:
			DrawFieldSurface();
			DrawFieldLine();
			DrawFieldCursore();
			break;
	};
	
	return;
}

void CFldViewView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if( bDrawLineFlag||bDrawSurfaceFlag )
	{
		dZmax =  -FLT_MAX;
		dZmin =   FLT_MAX;
		bZsizeFlag = TRUE;
	}
	if( bDrawLineFlag_Eps||bDrawSurfaceFlag_Eps )
	{
		dZmax_Eps = -FLT_MAX;
		dZmin_Eps =  FLT_MAX;
		bZsizeFlag_Eps = TRUE;
	}

	if( cError.IsError() )
	{
		bXsizeFlag = TRUE;
		bYsizeFlag = TRUE;
	};
	ReadData();
	DrawScene();	
	WriteIniFile();
	CScrollView::OnLButtonDblClk(nFlags, point);
}

void CFldViewView::OnRotateLeft() 
{
	// TODO: Add your command handler code here
    wAngleY -= 2.0f;
	DrawScene();	
	WriteIniFile();
	return;
}

void CFldViewView::OnRotateRight() 
{
	// TODO: Add your command handler code here
    wAngleY += 2.0f;
	DrawScene();	
	WriteIniFile();
	return;
}

void CFldViewView::OnRotateTop() 
{
	// TODO: Add your command handler code here
    wAngleX -= 2.0f;
	DrawScene();	
	WriteIniFile();
	return;
}

void CFldViewView::OnRotateBottom() 
{
	// TODO: Add your command handler code here
    wAngleX += 2.0f;
	DrawScene();	
	WriteIniFile();
	return;
}

void CFldViewView::OnRotateLefty() 
{
	// TODO: Add your command handler code here
    wAngleZ -= 2.0f;
	DrawScene();	
	WriteIniFile();
	return;
}

void CFldViewView::OnRotateRighty() 
{
	// TODO: Add your command handler code here
    wAngleZ += 2.0f;
	DrawScene();	
	WriteIniFile();
	return;
}

void CFldViewView::OnTranslateXt() 
{
	// TODO: Add your command handler code here
    wTranslateX += 0.04f;
	DrawScene();	
	WriteIniFile();
	return;
}


void CFldViewView::OnTranslateXb() 
{
	// TODO: Add your command handler code here
    wTranslateX -= 0.04f;
	DrawScene();	
	WriteIniFile();
	return;
}

void CFldViewView::OnTranslateZb() 
{
	// TODO: Add your command handler code here
    wTranslateY -= 0.04f;
	DrawScene();	
	WriteIniFile();
	return;
}

void CFldViewView::OnTranslateZt() 
{
	// TODO: Add your command handler code here
    wTranslateY += 0.04f;
	DrawScene();	
	WriteIniFile();
	return;
}

void CFldViewView::OnTranslateYb() 
{
	// TODO: Add your command handler code here
    wTranslateZ -= 0.04f;
	DrawScene();	
	WriteIniFile();
	return;
}

void CFldViewView::OnTranslateYt() 
{
	// TODO: Add your command handler code here
    wTranslateZ += 0.04f;
	DrawScene();	
	WriteIniFile();
	return;
}

void CFldViewView::DeleteData()
{
	nX = 0;
	nY = 0;
	nBoundBlockTpl = 0;
	nTopPolygon = 0;
	nTopSize = 0;
	nInputNum = 0;

	if( pcTopSize != NULL ) delete []pcTopSize;
	pcTopSize = NULL;
	if( pnTopType != NULL ) delete []pnTopType;
	pnTopType = NULL;
	if( pdTopX1 != NULL ) delete []pdTopX1;
	pdTopX1 = NULL;
	if( pdTopX2 != NULL ) delete []pdTopX2;
	pdTopX2 = NULL;
	if( pdTopX3 != NULL ) delete []pdTopX3;
	pdTopX3 = NULL;
	if( pdTopX4 != NULL ) delete []pdTopX4;
	pdTopX4 = NULL;
	if( pdTopY1 != NULL ) delete []pdTopY1;
	pdTopY1 = NULL;
	if( pdTopY2 != NULL ) delete []pdTopY2;
	pdTopY2 = NULL;
	if( pdTopY3 != NULL ) delete []pdTopY3;
	pdTopY3 = NULL;
	if( pdTopY4 != NULL ) delete []pdTopY4;
	pdTopY4 = NULL;
	if( pdTopXinp != NULL ) delete []pdTopXinp;
	pdTopXinp = NULL;
	if( pdTopYinp != NULL ) delete []pdTopYinp;
	pdTopYinp = NULL;

	if( pdSurface != NULL ) delete []pdSurface;
	pdSurface = NULL;

	if( pdSurface_Eps != NULL ) delete []pdSurface_Eps;
	pdSurface_Eps = NULL;

	if( pdSurfaceX != NULL ) delete []pdSurfaceX;
	pdSurfaceX = NULL;

	if( pdSurfaceY != NULL ) delete []pdSurfaceY;
	pdSurfaceY = NULL;
	
	if( pcBoundBlockTpl != NULL ) delete []pcBoundBlockTpl;
	pcBoundBlockTpl = NULL;
	
	cBlockTpl.DeleteData();
	cError.Clear();
}

void CFldViewView::ReadData()
{
	static BOOL bIsDataAlredyRead = FALSE;

	if(bIsDataAlredyRead)
	{
		DeleteData();
		CFldViewDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		pDoc->ReadData();
		cError = pDoc->cError;
//		ReadTpl();
		PrepareData();
		PutStatistics1();
		if( !cError.IsError() ) bIsDataAlredyRead = TRUE;
	}
	else
	{
		CFldViewDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		pDoc->ReadData();
		if( !pDoc->cError.IsError() )
		{
			DeleteData();
			cError = pDoc->cError;
			ReadTpl();
			PrepareData();
			PutStatistics1();
			if( cError.IsError() ) bIsDataAlredyRead = FALSE;
		};
	};

	return;
}

void CFldViewView::PrepareData()
{
	if( cError.IsError() ) return;

	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	int i, j, n, ii, nX1, nY1, jj;
	double *pdBuf, r, dr;
	
	dDelta = pDoc->GetdDelta();

	nX1 = pDoc->GetnX();
	if(bXsizeFlag)
	{
		nX = nX1;
	}
	else
	{
		if( dXmin > dXmax )
		{
			r = dXmax;
			dXmax = dXmin;
			dXmin = r;
		};
		if( dXmin < 0.0 ) dXmin = 0.0;
		if( dXmax > ((nX1-1)*dDelta) ) dXmax = (nX1-1)*dDelta;
		nX = (int)((dXmax-dXmin)/dDelta + 1);
	};
	
	nY1 = pDoc->GetnY();
	if(bYsizeFlag)
	{
		nY = nY1;
	}
	else
	{
		if( dYmin > dYmax )
		{
			r = dYmax;
			dYmax = dYmin;
			dYmin = r;
		};
		if( dYmin < 0.0 ) dYmin = 0.0;
		if( dYmax > ((nY1-1)*dDelta) ) dYmax = (nY1-1)*dDelta;
		nY = (int)((dYmax-dYmin)/dDelta + 1);
	};

	if( nX < 2 )
	{
		cError.PutErrorMessage("number of point along X axies < 2");
		RedrawWindow();
		return;
	};

	if( nY < 2 )
	{
		cError.PutErrorMessage("number of point along Y axies < 2");
		RedrawWindow();
		return;
	};

	if( ( pdBuf = pDoc->GetpdSurface() ) == NULL ) return;

	n = nX*nY;

	if( pdSurface != NULL ) delete []pdSurface;
	pdSurface = NULL;
	if( pdSurfaceX != NULL ) delete []pdSurfaceX;
	pdSurfaceX = NULL;
	if( pdSurfaceY != NULL ) delete []pdSurfaceY;
	pdSurfaceY = NULL;
	
	
	if( ( pdSurface = new double[n] ) == NULL ) 
	{
		cError.PutErrorMessage("allocation memory for double array");
		RedrawWindow();
		return;
	};

	if( ( pdSurfaceX = new double[n] ) == NULL ) 
	{
		CString csBuf;
		csBuf.Format("allocation memory for double array[%d]", nX*nY);
		cError.PutErrorMessage(csBuf);
		RedrawWindow();
		return;
	};

	if( ( pdSurfaceY = new double[n] ) == NULL ) 
	{
		CString csBuf;
		csBuf.Format("allocation memory for double array[%d]", nX*nY);
		cError.PutErrorMessage(csBuf);
		RedrawWindow();
		return;
	};

	if( bXsizeFlag )
	{
		dXmin = 0.0;
		dXmax = (nX-1)*dDelta;
	};

	if( bYsizeFlag )
	{
		dYmin = 0.0;
		dYmax = (nY-1)*dDelta;
	};

	if( bZsizeFlag )
	{
		for( i = 0, ii = 0, jj = (int)(dXmin/dDelta)+((int)(dYmin/dDelta))*nX1; i < nY; i++, jj+=nX1 )
		{
			for( j = 0; j < nX; j++, ii++ )
			{
				if( !bDrawFieldModulFlag )
				{
					if( dZmax < ( pdBuf[jj+j] ) ) dZmax = ( pdBuf[jj+j] );
					if( dZmin > ( pdBuf[jj+j] ) ) dZmin = ( pdBuf[jj+j] );
				}
				else
				{
					if( dZmax < fabs( pdBuf[jj+j] ) ) dZmax = fabs( pdBuf[jj+j] );
				};
			};
		};
		if( bDrawFieldModulFlag )
		{
			if( dZmax < FLT_MIN ) dZmax = FLT_MIN;
			dZmin = -dZmax;
		}
	};

	if( bProportionally )
	{
		if( (dXmax - dXmin) < (dYmax - dYmin) )
		{
			dXkoefProp = (dXmax - dXmin)/(dYmax - dYmin);
			dYkoefProp = 1.;
		}
		else
		{
			dXkoefProp = 1.;
			dYkoefProp = (dYmax - dYmin)/(dXmax - dXmin);
		};
	}
	else
	{
		dXkoefProp = 1.;
		dYkoefProp = 1.;
	};

	for( i = 0, ii = 0, jj = (int)(dXmin/dDelta)+(int)(dYmin/dDelta)*nX1; i < nY; i++, jj+=nX1 )
	{
		for( j = 0; j < nX; j++, ii++ )
		{
//			pdSurface[ii]   = 2*((pdBuf[jj+j] - dZmin)/( dZmax - dZmin)-0.5);
//			pdSurfaceX[ii]  = 2.5*((j*dDelta)/( dXmax - dXmin) - 0.4);
//			pdSurfaceY[ii]  = 2.5*((i*dDelta)/( dYmax - dYmin) - 0.4);
			pdSurface[ii]  = ZtoGLWinCoord( pdBuf[jj+j] );
			pdSurfaceX[ii] = XtoGLWinCoord( j );
			pdSurfaceY[ii] = YtoGLWinCoord( i );
		};
	};

	if( bDrawFieldValueFlag )
	{
		if( dDrawFieldValueX < (dXmin + dDelta) ) dDrawFieldValueX = (dXmin + dDelta);
		if( dDrawFieldValueX > (dXmax - dDelta) ) dDrawFieldValueX = (dXmax - dDelta);
		if( dDrawFieldValueY < (dYmin + dDelta) ) dDrawFieldValueY = (dYmin + dDelta);
		if( dDrawFieldValueY > (dYmax - dDelta) ) dDrawFieldValueY = (dYmax - dDelta);
		nDrawFieldValue1 = GetNumberElement( dDrawFieldValueX - dDelta, dDrawFieldValueY );
		nDrawFieldValue2 = GetNumberElement( dDrawFieldValueX, dDrawFieldValueY + dDelta );
		nDrawFieldValue3 = GetNumberElement( dDrawFieldValueX + dDelta, dDrawFieldValueY );
		nDrawFieldValue4 = GetNumberElement( dDrawFieldValueX, dDrawFieldValueY - dDelta );
		nDrawFieldValue5 = GetNumberElement( dDrawFieldValueX, dDrawFieldValueY );
	};

	PrepareTpl();
	PrepareBoundaryTpl();
	
	for( i = 0, r = dZmin + (dZmax - dZmin)/2., dr = ( dZmax - dZmin )/2/( 20. - 1.); i < 20; i++, r += dr )
	{
		pcColorLevelDialog->pdLevel[ i ] = r;
	};
	pcColorLevelDialog->RedrawWindow();

	return;
}

void CFldViewView::DrawFieldSurface()
{
	if( cError.IsError() ) return;
	if( pdSurface  == NULL ) return;
	if( pdSurfaceX == NULL ) return;
	if( pdSurfaceY == NULL ) return;

	if( !bDrawSurfaceFlag ) return;

	int nY1, nX1;
	int i, j, ii;

	nY1 = nY - 1;
	nX1 = nX - 1;

	for( i = 0, ii = 0; i < nY1; i++, ii++ )
	{
		for( j = 0; j < nX1; j++, ii++ )
		{
			glBegin(GL_POLYGON);
			SetColor( pdSurface[ii] );
		    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(pdSurface[ii]));
			SetColor( pdSurface[ii+1] );
		    glVertex3f( (float)(pdSurfaceX[ii+1]), (float)(pdSurfaceY[ii+1]), (float)(pdSurface[ii+1]));
			SetColor( pdSurface[ii+1+nX] );
		    glVertex3f( (float)(pdSurfaceX[ii+1+nX]), (float)(pdSurfaceY[ii+1+nX]), (float)(pdSurface[ii+1+nX]));
			SetColor( pdSurface[ii+nX] );
		    glVertex3f( (float)(pdSurfaceX[ii+nX]), (float)(pdSurfaceY[ii+nX]), (float)(pdSurface[ii+nX]));
		    glEnd();
		};
	};
	return;
}

void CFldViewView::DrawFieldSurface2()
{
	if( cError.IsError() ) return;
	if( pdSurface  == NULL ) return;
	if( pdSurfaceX == NULL ) return;
	if( pdSurfaceY == NULL ) return;

	if( !bDrawSurfaceFlag ) return;

	int nY1, nX1;
	int i, j, ii;

	nY1 = nY - 1;
	nX1 = nX - 1;

	for( i = 0, ii = 0; i < nY1; i++, ii++ )
	{
		for( j = 0; j < nX1; j++, ii++ )
		{
			glBegin(GL_POLYGON);
			SetColor( pdSurface[ii] );
		    glVertex2f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]) );
			SetColor( pdSurface[ii+1] );
		    glVertex2f( (float)(pdSurfaceX[ii+1]), (float)(pdSurfaceY[ii+1]) );
			SetColor( pdSurface[ii+1+nX] );
		    glVertex2f( (float)(pdSurfaceX[ii+1+nX]), (float)(pdSurfaceY[ii+1+nX]) );
			SetColor( pdSurface[ii+nX] );
		    glVertex2f( (float)(pdSurfaceX[ii+nX]), (float)(pdSurfaceY[ii+nX]) );
		    glEnd();
		};
	};
	return;
}

void CFldViewView::DrawFieldLine()
{
	if( cError.IsError() ) return;
	if( pdSurface  == NULL ) return;
	if( pdSurfaceX == NULL ) return;
	if( pdSurfaceY == NULL ) return;

	if( !bDrawLineFlag ) return;

	int nY1, nX1;
	int i, j, ii;

	nY1 = nY - 1;
	nX1 = nX - 1;

	if( bDrawSurfaceFlag )
	{
		glColor4f( 1.0f, 1.0f, 1.0f, (float)(0.8) );
		for( i = 0, ii = 0; i < nY; i++, ii++ )
		{
			glBegin( GL_LINES  );
			for( j = 0; j < nX1; j++, ii++ )
			{
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(pdSurface[ii]));
			    glVertex3f( (float)(pdSurfaceX[ii+1]), (float)(pdSurfaceY[ii+1]), (float)(pdSurface[ii+1]));
			};
		    glEnd();
		};
		for( i = 0; i < nX; i++ )
		{
			glBegin( GL_LINES );
			for( j = 0, ii = i; j < nY1; j++, ii+=nX )
			{
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(pdSurface[ii]));
			    glVertex3f( (float)(pdSurfaceX[ii+nX]), (float)(pdSurfaceY[ii+nX]), (float)(pdSurface[ii+nX]));
			};
		    glEnd();
		};
	}
	else
	{
		for( i = 0, ii = 0; i < nY; i++, ii++ )
		{
			glBegin( GL_LINES  );
			for( j = 0; j < nX1; j++, ii++ )
			{
				SetColor( pdSurface[ii] );
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(pdSurface[ii]));
				SetColor( pdSurface[ii+1] );
			    glVertex3f( (float)(pdSurfaceX[ii+1]), (float)(pdSurfaceY[ii+1]), (float)(pdSurface[ii+1]));
			};
		    glEnd();
		};
		for( i = 0; i < nX; i++ )
		{
			glBegin( GL_LINES );
			for( j = 0, ii = i; j < nY1; j++, ii+=nX )
			{
				SetColor( pdSurface[ii] );
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(pdSurface[ii]));
				SetColor( pdSurface[ii+nX] );
			    glVertex3f( (float)(pdSurfaceX[ii+nX]), (float)(pdSurfaceY[ii+nX]), (float)(pdSurface[ii+nX]));
			};
		    glEnd();
		};
	};
	return;
}

void CFldViewView::DrawFieldLine2()
{
	if( cError.IsError() ) return;
	if( pdSurface  == NULL ) return;
	if( pdSurfaceX == NULL ) return;
	if( pdSurfaceY == NULL ) return;

	if( !bDrawLineFlag ) return;

	int nY1, nX1;
	int i, j, ii;

	nY1 = nY - 1;
	nX1 = nX - 1;

	if( bDrawSurfaceFlag )
	{
		glColor4f( 1.0f, 1.0f, 1.0f, (float)(0.8) );
		for( i = 0, ii = 0; i < nY; i++, ii++ )
		{
			glBegin( GL_LINES  );
			for( j = 0; j < nX1; j++, ii++ )
			{
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(0.01));
			    glVertex3f( (float)(pdSurfaceX[ii+1]), (float)(pdSurfaceY[ii+1]), (float)(0.01));
			};
		    glEnd();
		};
		for( i = 0; i < nX; i++ )
		{
			glBegin( GL_LINES );
			for( j = 0, ii = i; j < nY1; j++, ii+=nX )
			{
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(0.01));
			    glVertex3f( (float)(pdSurfaceX[ii+nX]), (float)(pdSurfaceY[ii+nX]), (float)(0.01));
			};
		    glEnd();
		};
	}
	else
	{
		for( i = 0, ii = 0; i < nY; i++, ii++ )
		{
			glBegin( GL_LINES  );
			for( j = 0; j < nX1; j++, ii++ )
			{
				SetColor( pdSurface[ii] );
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(0.01));
				SetColor( pdSurface[ii+1] );
			    glVertex3f( (float)(pdSurfaceX[ii+1]), (float)(pdSurfaceY[ii+1]), (float)(0.01));
			};
		    glEnd();
		};
		for( i = 0; i < nX; i++ )
		{
			glBegin( GL_LINES );
			for( j = 0, ii = i; j < nY1; j++, ii+=nX )
			{
				SetColor( pdSurface[ii] );
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(0.01));
				SetColor( pdSurface[ii+nX] );
			    glVertex3f( (float)(pdSurfaceX[ii+nX]), (float)(pdSurfaceY[ii+nX]), (float)(0.01));
			};
		    glEnd();
		};
	};
	return;
}


void CFldViewView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if( nIDEvent == 1 )
	{
		CFldViewDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		pDoc->UpdateAllViews(NULL);
	};
	
	CScrollView::OnTimer(nIDEvent);
}


void CFldViewView::SetColor( double z )
{
	int i;
	double n_r, n_g, n_b;
	double n_dr, n_dg, n_db;
	int n1_r, n1_g, n1_b;
	int n2_r, n2_g, n2_b;

	if( z >  1. ) z =  1.0001;
	if( z < -1. ) z = -1.0001;

	if( !bDrawFieldModulFlag )
	{
//		double z1, z2;
//		if( fabs( dZmax ) > fabs( dZmin ) ) z1 = fabs( dZmax );
//		else z1 = fabs( dZmin );
//		if( fabs( dZmax ) < fabs( dZmin ) ) z2 = fabs( dZmax );
//		else z2 = fabs( dZmin );
//		z = (z - dZmin/z1)/(z2/z1 + 1);
		z = (z + 1.)/2.;
	};

//	i = (int)(( fabs( z - (dZmax+dZmin)/2 )/((dZmax-dZmin)/2)*20 -0.5));

	if( bHightColorRezolution )
	{
		n1_r = GetRValue(pscSurfaceColor[0]);
		n1_g = GetGValue(pscSurfaceColor[0]);
		n1_b = GetBValue(pscSurfaceColor[0]);
		
		n2_r = GetRValue(pscSurfaceColor[19]);
		n2_g = GetGValue(pscSurfaceColor[19]);
		n2_b = GetBValue(pscSurfaceColor[19]);

		n_dr = (n2_r - n1_r)/(256.-1);
		n_dg = (n2_g - n1_g)/(256.-1);
		n_db = (n2_b - n1_b)/(256.-1);

		n_r = n1_r + fabs( z )*256*n_dr;
		n_g = n1_g + fabs( z )*256*n_dg;
		n_b = n1_b + fabs( z )*256*n_db;

		glColor4f( ((float)(n_r))/256,
					((float)(n_g))/256,
					((float)(n_b))/256,
					(float)(nBlend/100.) );
	}
	else
	{
		i = (int)(( fabs( z ) )*19 );
		glColor4f( ((float)(GetRValue(pscSurfaceColor[i])))/256,
					((float)(GetGValue(pscSurfaceColor[i])))/256,
					((float)(GetBValue(pscSurfaceColor[i])))/256,
					(float)(nBlend/100.) );
	};

	return;
}

void CFldViewView::SetColor( COLORREF csCol )
{
	glColor4f( ((float)(GetRValue(csCol)))/256,
				((float)(GetGValue(csCol)))/256,
				((float)(GetBValue(csCol)))/256,
				(float)(1.) );
	return;
}

void CFldViewView::SetColor( COLORREF csCol, int nBlend11 )
{
	glColor4f( ((float)(GetRValue(csCol)))/256,
				((float)(GetGValue(csCol)))/256,
				((float)(GetBValue(csCol)))/256,
				(float)(nBlend11/100.) );
	return;
}

void CFldViewView::OnViewParameters() 
{
	// TODO: Add your command handler code here
	CTMCDialogPropet *cDlg = GetPropDialog();
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	int i;

	cDlg->cPlace.pFieldView = this;
	cDlg->cPlace.m_AutoFlagX = bXsizeFlag;
	cDlg->cPlace.m_m_AutoFlagY = bYsizeFlag;
	cDlg->cPlace.m_m_AxiesDrawFlag = bDrawFlagAxies;
	cDlg->cPlace.m_dXAxiesAngle = wAngleX;
	cDlg->cPlace.m_dYAxiesAngle = wAngleY;
	cDlg->cPlace.m_dXAxiesMax = ( dXmax + pDoc->GetdXmin() )/pDoc->GetdLongUnit();
	cDlg->cPlace.m_dYAxiesMax = ( dYmax + pDoc->GetdYmin() )/pDoc->GetdLongUnit();
	cDlg->cPlace.m_dXAxiesMin = ( dXmin + pDoc->GetdXmin() )/pDoc->GetdLongUnit();
	cDlg->cPlace.m_dYAxiesMin = ( dYmin + pDoc->GetdYmin() )/pDoc->GetdLongUnit();
	cDlg->cPlace.m_dXAxiesTrans = wTranslateX;
	cDlg->cPlace.m_dYAxiesTrans = wTranslateY;
	cDlg->cPlace.scColorAxies = scAxiesColor;
	cDlg->cPlace.lfInitial = lfAxiesFont;
	cDlg->cPlace.m_LongUnit = pDoc->GetcsLongUnit();
	cDlg->cPlace.m_TimeUnit = pDoc->GetcsTimeUnit();
	cDlg->cPlace.m_DrawFlafSizeX = bDrawFlagAxiesXSize;
	cDlg->cPlace.m_DrawFlafSizeY = bDrawFlagAxiesYSize;
	cDlg->cPlace.m_Proportionally = bProportionally;

	cDlg->cTopology.pFieldView = this;
	cDlg->cTopology.scColorEps = scColorEps;
	cDlg->cTopology.scColorMet = scColorMet;
	cDlg->cTopology.scColorMag = scColorMag;
	cDlg->cTopology.scColorInp = scColorInp;
	cDlg->cTopology.scColorAbs = scColorAbs;
	cDlg->cTopology.m_Blend    = nBlendTopology;
	cDlg->cTopology.m_DrawFlagEps   = bDrawFlagEps;
	cDlg->cTopology.m_DrawFlagInput = bDrawFlagInput;
	cDlg->cTopology.m_DrawFlagMetal = bDrawFlagMetal;
	cDlg->cTopology.m_DrawFlagLine  = bDrawLineFlagTopology;
	cDlg->cTopology.m_23Dimention   = n2D3DFlagTopology;
	cDlg->cTopology.m_DrawFlagSurface      = bDrawSurfaceFlagTopology;
	cDlg->cTopology.m_DrawFlagMagnetic     = bDrawFlagMagnetic;
	cDlg->cTopology.m_DrawFlagSizeBlock    = bTopologySizeFlag;
	cDlg->cTopology.m_DrawFlagSizeLinkList = bTopologySizeFlagLinkList;
	cDlg->cTopology.m_DrawFlagAbsorber	   = bDrawFlagAbsorber;

	cDlg->cField.pFieldView = this;
	for( i = 0; i < 20; i++ )	cDlg->cField.c20Color[i] = pscSurfaceColor[i];
	cDlg->cField.m_d23Dimension     = n2D3DFlag;
	cDlg->cField.m_dBlend           = nBlend;
	cDlg->cField.m_bDrawFlagLine    = bDrawLineFlag;
	cDlg->cField.m_bDrawFlagSurface = bDrawSurfaceFlag;
	cDlg->cField.m_bAutoSizeFlag    = bZsizeFlag;
	cDlg->cField.m_dZAxiesMax       = dZmax;
	cDlg->cField.m_dZAxiesMin       = dZmin;
	cDlg->cField.m_DrawSizeFlag     = bDrawFlagAxiesZFieldSize;
	cDlg->cField.bHightColorResolution = bHightColorRezolution;
	cDlg->cField.bModulFlag = bDrawFieldModulFlag;

	cDlg->cEps.pFieldView = this;
	for( i = 0; i < 20; i++ )	cDlg->cEps.c20Color[i] = pscSurfaceColor_Eps[i];
	cDlg->cEps.m_d23Dimension     = n2D3DFlag_Eps;
	cDlg->cEps.m_dBlend           = nBlend_Eps;
	cDlg->cEps.m_bDrawFlagLine    = bDrawLineFlag_Eps;
	cDlg->cEps.m_bDrawFlagSurface = bDrawSurfaceFlag_Eps;
	cDlg->cEps.m_bAutoSizeFlag    = bZsizeFlag_Eps;
	cDlg->cEps.m_dZAxiesMax       = dZmax_Eps;
	cDlg->cEps.m_dZAxiesMin       = dZmin_Eps;
	cDlg->cEps.m_DrawSizeFlag     = bDrawFlagAxiesZFieldSize_Eps;

	cDlg->nPropertyPageInd = nPropertyPageInd;
//	cDlg->SetActivePage( nPropertyPageInd );

	if( cDlg->DoModal() == 1  )
	{
		bXsizeFlag = cDlg->cPlace.m_AutoFlagX;
		bYsizeFlag = cDlg->cPlace.m_m_AutoFlagY;
		bDrawFlagAxies = cDlg->cPlace.m_m_AxiesDrawFlag;
		wAngleX = (GLfloat)cDlg->cPlace.m_dXAxiesAngle;
		wAngleY = (GLfloat)cDlg->cPlace.m_dYAxiesAngle;
		dXmax = cDlg->cPlace.m_dXAxiesMax*pDoc->GetdLongUnit() - pDoc->GetdXmin();
		dYmax = cDlg->cPlace.m_dYAxiesMax*pDoc->GetdLongUnit() - pDoc->GetdYmin();
		dXmin = cDlg->cPlace.m_dXAxiesMin*pDoc->GetdLongUnit() - pDoc->GetdXmin();
		dYmin = cDlg->cPlace.m_dYAxiesMin*pDoc->GetdLongUnit() - pDoc->GetdYmin();
		wTranslateX = (GLfloat)cDlg->cPlace.m_dXAxiesTrans;
		wTranslateY = (GLfloat)cDlg->cPlace.m_dYAxiesTrans;
		scAxiesColor = cDlg->cPlace.scColorAxies;
		lfAxiesFont = cDlg->cPlace.lfInitial;
		bDrawFlagAxiesXSize = cDlg->cPlace.m_DrawFlafSizeX;
		bDrawFlagAxiesYSize = cDlg->cPlace.m_DrawFlafSizeY;
		bProportionally = cDlg->cPlace.m_Proportionally;

		scColorEps = cDlg->cTopology.scColorEps;
		scColorMet = cDlg->cTopology.scColorMet;
		scColorMag = cDlg->cTopology.scColorMag;
		scColorInp = cDlg->cTopology.scColorInp;
		scColorAbs = cDlg->cTopology.scColorAbs;
		nBlendTopology = cDlg->cTopology.m_Blend;
		if( nBlendTopology <   0 ) nBlendTopology =   0;
		if( nBlendTopology > 100 ) nBlendTopology = 100;
		bDrawFlagEps   = cDlg->cTopology.m_DrawFlagEps;
		bDrawFlagInput = cDlg->cTopology.m_DrawFlagInput;
		bDrawFlagMetal = cDlg->cTopology.m_DrawFlagMetal;
		bDrawLineFlagTopology = cDlg->cTopology.m_DrawFlagLine;
		n2D3DFlagTopology     = cDlg->cTopology.m_23Dimention;
		bDrawSurfaceFlagTopology  = cDlg->cTopology.m_DrawFlagSurface;
		bDrawFlagMagnetic         = cDlg->cTopology.m_DrawFlagMagnetic;
		bTopologySizeFlag         = cDlg->cTopology.m_DrawFlagSizeBlock;
		bTopologySizeFlagLinkList = cDlg->cTopology.m_DrawFlagSizeLinkList;
		bDrawFlagAbsorber		  = cDlg->cTopology.m_DrawFlagAbsorber;
		
		for( i = 0; i < 20; i++ )	pscSurfaceColor[i] = cDlg->cField.c20Color[i];
		n2D3DFlag        = cDlg->cField.m_d23Dimension;
		nBlend           = cDlg->cField.m_dBlend;
		if( nBlend <   0 ) nBlend =   0;
		if( nBlend > 100 ) nBlend = 100;
		bDrawLineFlag    = cDlg->cField.m_bDrawFlagLine;
		bHightColorRezolution = cDlg->cField.bHightColorResolution;
		bDrawSurfaceFlag = cDlg->cField.m_bDrawFlagSurface;
		bZsizeFlag       = cDlg->cField.m_bAutoSizeFlag;
		dZmax            = cDlg->cField.m_dZAxiesMax;
		dZmin            = cDlg->cField.m_dZAxiesMin;
//		if( fabs(dZmax) < fabs(dZmin) )	dZmax = fabs(dZmin);
//		else dZmax = fabs(dZmax);
//		dZmin = -dZmax;
		if( dZmax < dZmin )	
		{
			double rrrr;
			rrrr = dZmax;
			dZmax = dZmin;
			dZmin = rrrr;
		};
		bDrawFlagAxiesZFieldSize = cDlg->cField.m_DrawSizeFlag;
		bDrawFieldModulFlag = cDlg->cField.bModulFlag;

		for( i = 0; i < 20; i++ )	pscSurfaceColor_Eps[i] = cDlg->cEps.c20Color[i];
		n2D3DFlag_Eps        = cDlg->cEps.m_d23Dimension;
		nBlend_Eps           = cDlg->cEps.m_dBlend;
		if( nBlend_Eps <   0 ) nBlend_Eps =   0;
		if( nBlend_Eps > 100 ) nBlend_Eps = 100;
		bDrawLineFlag_Eps    = cDlg->cEps.m_bDrawFlagLine;
		bDrawSurfaceFlag_Eps = cDlg->cEps.m_bDrawFlagSurface;
		bZsizeFlag_Eps       = cDlg->cEps.m_bAutoSizeFlag;
		dZmax_Eps            = cDlg->cEps.m_dZAxiesMax;
		dZmin_Eps            = cDlg->cEps.m_dZAxiesMin;
		bDrawFlagAxiesZFieldSize_Eps = cDlg->cEps.m_DrawSizeFlag;

		PrepareData();
		DrawScene();
	};

	nPropertyPageInd = cDlg->nPropertyPageInd;
	WriteIniFile();
	return;
}

void CFldViewView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	OnViewParameters();	
	CScrollView::OnRButtonDown(nFlags, point);
}

void CFldViewView::WriteIniFile()
{
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	char szBuffer[1024];
	double r;
	int i, j, k, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12;

// save hight resolution color flag
	sprintf( szBuffer, "%d", bHightColorRezolution);
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLDCOLORRESOL_INI, szBuffer);

// save field modul flag
	sprintf( szBuffer, "%d", bDrawFieldModulFlag);
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLDMODULFL_INI, szBuffer);

// save property page number
	sprintf( szBuffer, "%d", nPropertyPageInd);
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLDNUMPROPPAGE_INI, szBuffer);

// save topology parameters draw flag
	sprintf( szBuffer, "%d %d %d %d %d %d %d %d %d %d %d %d", 
		bDrawFlagTopology, 
		bDrawLineFlagTopology,
		bDrawSurfaceFlagTopology, 
		n2D3DFlagTopology,
		nBlendTopology,
		bTopologySizeFlag, 
		bTopologySizeFlagLinkList,
		bDrawFlagMagnetic,
		bDrawFlagMetal,
		bDrawFlagEps,
		bDrawFlagInput,
		bDrawFlagAbsorber);
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLDTOPOLOGY_INI, szBuffer);

// save draw flag
	sprintf( szBuffer, "%d %d", bDrawFlag, bProportionally);
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_DRAWFLAG_INI, szBuffer);

// save draw flag for surface
	sprintf( szBuffer, "%d", bDrawSurfaceFlag );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_DRFLSURF_INI, szBuffer);

// save draw flag for line
	sprintf( szBuffer, "%d", bDrawLineFlag );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_DRFLLINE_INI, szBuffer);

// save X-Size flag
	sprintf( szBuffer, "%d", bXsizeFlag );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_FLXSIZE_INI, szBuffer);

// save Y-Size flag
	sprintf( szBuffer, "%d", bYsizeFlag );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_FLYSIZE_INI, szBuffer);

// save Z-Size flag
	sprintf( szBuffer, "%d", bZsizeFlag );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_FLZSIZE_INI, szBuffer);

// save 2 or 3 dimension for draw
	sprintf( szBuffer, "%d", n2D3DFlag );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_FL23D_INI, szBuffer);

// save blend
	sprintf( szBuffer, "%d", nBlend );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_BLEND_INI, szBuffer);

// save Xmin
	sprintf( szBuffer, "%lg", dXmin );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_XMIN_INI, szBuffer);

// save Xmax
	sprintf( szBuffer, "%lg", dXmax );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_XMAX_INI, szBuffer);

// save Ymin
	sprintf( szBuffer, "%lg", dYmin );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_YMIN_INI, szBuffer);

// save Ymax
	sprintf( szBuffer, "%lg", dYmax );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_YMAX_INI, szBuffer);

// save Zmin
	sprintf( szBuffer, "%lg", dZmin );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_ZMIN_INI, szBuffer);

// save Zmax
	sprintf( szBuffer, "%lg", dZmax );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_ZMAX_INI, szBuffer);

// save XAngle
	r = wAngleX;
	sprintf( szBuffer, "%lg", r );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_XANGLE_INI, szBuffer);

// save ZAngle
	r = wAngleY;
	sprintf( szBuffer, "%lg", r );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_YANGLE_INI, szBuffer);

// save ZAngle
	r = wAngleZ;
	sprintf( szBuffer, "%lg", r );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_ZANGLE_INI, szBuffer);

// save XTranslate
	r = wTranslateX;
	sprintf( szBuffer, "%lg", r );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_XTRANS_INI, szBuffer);

// save YTranslate
	r = wTranslateY;
	sprintf( szBuffer, "%lg", r );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_YTRANS_INI, szBuffer);

// save ZTranslate
	r = wTranslateZ;
	sprintf( szBuffer, "%lg", r );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_ZTRANS_INI, szBuffer);
	
// save DrawFlagAxies
	sprintf( szBuffer, "%d", bDrawFlagAxies );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_DRFLAXIS_INI, szBuffer);

// save Axies Color
	i = GetRValue( scAxiesColor );
	j = GetGValue( scAxiesColor );
	k = GetBValue( scAxiesColor );
	sprintf( szBuffer, "%d %d %d", i, j, k );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_COLORAXIS_INI, szBuffer);

// save Draw Axies Size
	i1 = bDrawFlagAxiesXSize;
	i2 = bDrawFlagAxiesYSize;
	i3 = bDrawFlagAxiesZFieldSize;
	i4 = bDrawFlagAxiesZEpsSize;
	sprintf( szBuffer, "%d %d %d %d", i1, i2, i3, i4 );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLDDRFLAXISSZ_INI, szBuffer);

// save Axies Font name
	sprintf( szBuffer, "%s", lfAxiesFont.lfFaceName );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_FONTNAMEAXIS_INI, szBuffer);

// save Axies Font
    i  = lfAxiesFont.lfHeight;
	i1 = lfAxiesFont.lfCharSet;
	i2 = lfAxiesFont.lfWeight;
	i3 = lfAxiesFont.lfWidth;
	i4 = lfAxiesFont.lfOutPrecision;
	i5 = lfAxiesFont.lfClipPrecision;
	i6 = lfAxiesFont.lfItalic;
	i7 = lfAxiesFont.lfUnderline;
	i8 = lfAxiesFont.lfStrikeOut;
	i9 = lfAxiesFont.lfEscapement;
	i10= lfAxiesFont.lfOrientation;
	i11= lfAxiesFont.lfQuality;
	i12= lfAxiesFont.lfPitchAndFamily;
	
	sprintf( szBuffer, "%d %d %d %d %d %d %d %d %d %d %d %d %d", i, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12);
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_FONTAXIS_INI, szBuffer);

// save Color for draw Z Axies field
	sprintf( szBuffer, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
			pscSurfaceColor[ 0],
			pscSurfaceColor[ 1],
			pscSurfaceColor[ 2],
			pscSurfaceColor[ 3],
			pscSurfaceColor[ 4],
			pscSurfaceColor[ 5],
			pscSurfaceColor[ 6],
			pscSurfaceColor[ 7],
			pscSurfaceColor[ 8],
			pscSurfaceColor[ 9],
			pscSurfaceColor[10],
			pscSurfaceColor[11],
			pscSurfaceColor[12],
			pscSurfaceColor[13],
			pscSurfaceColor[14],
			pscSurfaceColor[15],
			pscSurfaceColor[16],
			pscSurfaceColor[17],
			pscSurfaceColor[18],
			pscSurfaceColor[19] );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_COLORZFIELD_INI, szBuffer);

// save Color for draw Z Axies eps
	sprintf( szBuffer, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
			pscSurfaceColor_Eps[ 0],
			pscSurfaceColor_Eps[ 1],
			pscSurfaceColor_Eps[ 2],
			pscSurfaceColor_Eps[ 3],
			pscSurfaceColor_Eps[ 4],
			pscSurfaceColor_Eps[ 5],
			pscSurfaceColor_Eps[ 6],
			pscSurfaceColor_Eps[ 7],
			pscSurfaceColor_Eps[ 8],
			pscSurfaceColor_Eps[ 9],
			pscSurfaceColor_Eps[10],
			pscSurfaceColor_Eps[11],
			pscSurfaceColor_Eps[12],
			pscSurfaceColor_Eps[13],
			pscSurfaceColor_Eps[14],
			pscSurfaceColor_Eps[15],
			pscSurfaceColor_Eps[16],
			pscSurfaceColor_Eps[17],
			pscSurfaceColor_Eps[18],
			pscSurfaceColor_Eps[19] );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLD_COLORZEPS_INI, szBuffer);

// save Color for draw topology block
	sprintf( szBuffer, "%d %d %d %d %d", 
			scColorEps,
			scColorMet,
			scColorMag,
			scColorInp,
			scColorAbs );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLDTOPOLOGYCOLOR_INI, szBuffer);

// save parameter for draw Eps(x,y) data
	sprintf( szBuffer, "%d %d %d %d %d %d %lg %lg", 
			n2D3DFlag_Eps,
			nBlend_Eps,
			bDrawLineFlag_Eps,
			bDrawSurfaceFlag_Eps,
			bZsizeFlag_Eps,
			bDrawFlagAxiesZFieldSize_Eps,
			dZmax_Eps,
			dZmin_Eps );
	AfxGetApp()->WriteProfileString( TMC_GROFLD_RAZDEL_INI, TMC_GROFLDTOPEPSPAR_INI, szBuffer);

	return;
}

void CFldViewView::SetDefaultValue()
{
	bHightColorRezolution	= FALSE;
	bDrawFlag				= TRUE;
	bDrawSurfaceFlag		= FALSE;
	bDrawSurfaceFlag_Eps	= FALSE;
	bDrawLineFlag			= TRUE;
	bDrawLineFlag_Eps		= FALSE;
	bDrawFlagAxies = FALSE;
	bProportionally = TRUE;

	bDrawFlagTopology			= FALSE;
	bDrawSurfaceFlagTopology	= TRUE;
	bDrawLineFlagTopology		= TRUE;

	bDrawFlagAxiesXSize			= FALSE;
	bDrawFlagAxiesYSize			= FALSE;
	bDrawFlagAxiesZFieldSize	= FALSE;
	bDrawFlagAxiesZFieldSize_Eps= FALSE;
	bDrawFlagAxiesZEpsSize		= FALSE;
	
	scAxiesColor = RGB( 255, 255, 0 );
    lfAxiesFont.lfHeight = 12 * GetDeviceCaps( wglGetCurrentDC(), LOGPIXELSY) / 72;
    lfAxiesFont.lfCharSet = SHIFTJIS_CHARSET;
    lfAxiesFont.lfWeight = 400;
	lfAxiesFont.lfWidth = lfAxiesFont.lfHeight/3;
    lfAxiesFont.lfOutPrecision = OUT_STROKE_PRECIS;
    lfAxiesFont.lfClipPrecision = CLIP_CHARACTER_PRECIS;
    strcpy( lfAxiesFont.lfFaceName, "TimesET");
    lfAxiesFont.lfItalic = FALSE;
    lfAxiesFont.lfUnderline = FALSE;
    lfAxiesFont.lfStrikeOut = FALSE;

	dDelta = 0.;
	dZtopol = 0.25;

	wTranslateX = (float)(-0.25);
	wTranslateY = (float)(-0.25);
	wTranslateZ = (float)(-4.50);

    wAngleY = 0.0f;
    wAngleX = 0.0f;
    wAngleZ = 0.0f;

	nBlend = 100;
	nBlendTopology	= 100;
	nBlend_Eps		=  80;

	dXmin		= -1.;
	dXmax		=  1.;
	dYmin		= -1.;
	dYmax		=  1.;
	dZmin		= -1.;
	dZmax		=  0.0;
	dZmax_Eps	= -1.;
	dZmin_Eps	= 0.0;
	
	bXsizeFlag		= TRUE;
	bYsizeFlag		= TRUE;
	bZsizeFlag		= TRUE;
	bZsizeFlag_Eps	= TRUE;
	
	bTopologySizeFlag = FALSE;
	bTopologySizeFlagLinkList = FALSE;

	bDrawFlagMagnetic = TRUE;
	bDrawFlagMetal    = TRUE;
	bDrawFlagEps      = TRUE;
	bDrawFlagInput    = TRUE;
	bDrawFlagAbsorber = TRUE;

	n2D3DFlag = TMCFIELDVIEW_2DDraw;
	n2D3DFlagTopology = TMCFIELDVIEW_2DDraw;
	n2D3DFlag_Eps	  = TMCFIELDVIEW_2DDraw;

	return;
}

void CFldViewView::OnStrelkleft() 
{
	// TODO: Add your command handler code here
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	double dDelta = pDoc->GetdDelta();

	if( bDrawFieldValueFlag )
	{
		dDrawFieldValueX -= dDelta;
		PrepareData();
		DrawScene();
		WriteIniFile();
		PutTrace1();
		return;
	};

	if( (dXmin - dDelta) < 0.0 )
	{
		dXmin = 0.0;
		PrepareData();
		DrawScene();
		WriteIniFile();
		return;
	};
	dXmin -= dDelta;
	dXmax -= dDelta;
	PrepareData();
	DrawScene();
	WriteIniFile();
	PutTrace1();
	return;
}

void CFldViewView::OnStrelkright() 
{
	// TODO: Add your command handler code here
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	double dDelta = pDoc->GetdDelta();

	if( bDrawFieldValueFlag )
	{
		dDrawFieldValueX += dDelta;
		PrepareData();
		DrawScene();
		WriteIniFile();
		PutTrace1();
		return;
	};

	if( ( dDelta*(pDoc->GetnX()-2) ) < dXmax )
	{
		dXmax = dDelta*(pDoc->GetnX()-1);
		PrepareData();
		DrawScene();
		WriteIniFile();
		return;
	};
	dXmin += dDelta;
	dXmax += dDelta;
	PrepareData();
	DrawScene();
	WriteIniFile();
	PutTrace1();
	return;
}

void CFldViewView::OnStrelkup() 
{
	// TODO: Add your command handler code here
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	double dDelta = pDoc->GetdDelta();

	if( bDrawFieldValueFlag )
	{
		dDrawFieldValueY += dDelta;
		PrepareData();
		DrawScene();
		WriteIniFile();
		PutTrace1();
		return;
	};

	if( ( dDelta*(pDoc->GetnY()-2) ) < dYmax )
	{
		dYmax = dDelta*(pDoc->GetnY()-1);
		PrepareData();
		DrawScene();
		WriteIniFile();
		return;
	};
	dYmin += dDelta;
	dYmax += dDelta;
	PrepareData();
	DrawScene();
	WriteIniFile();
	PutTrace1();
	return;
}

void CFldViewView::OnStrelkdown() 
{
	// TODO: Add your command handler code here
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	double dDelta = pDoc->GetdDelta();

	if( bDrawFieldValueFlag )
	{
		dDrawFieldValueY -= dDelta;
		PrepareData();
		DrawScene();
		WriteIniFile();
		PutTrace1();
		return;
	};

	if( (dYmin - dDelta) < 0.0 ) 
	{
		dYmin = 0.0;
		PrepareData();
		DrawScene();
		WriteIniFile();
		return;
	};
	dYmin -= dDelta;
	dYmax -= dDelta;
	PrepareData();
	DrawScene();
	WriteIniFile();
	PutTrace1();
	return;
}

void CFldViewView::OnViewZoomP() 
{
	// TODO: Add your command handler code here
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	double dX0, dY0, dX1, dY1;

	bXsizeFlag = FALSE;
	bYsizeFlag = FALSE;
	
	dX0 = (dXmax + dXmin)/2;
	dY0 = (dYmax + dYmin)/2;
	
	dX1 = (dXmax - dXmin)/4;
	if( dX1 < 2*pDoc->GetdDelta() ) dX1 = pDoc->GetdDelta();

	dY1 = (dYmax - dYmin)/4;
	if( dY1 < 2*pDoc->GetdDelta() ) dY1 = pDoc->GetdDelta();

	dXmin = dX0 - dX1;
	dXmax = dX0 + dX1;

	dYmin = dY0 - dY1;
	dYmax = dY0 + dY1;
	
	PrepareData();
	DrawScene();
	WriteIniFile();
	PutTrace1();
	return;
}

void CFldViewView::OnViewZoomM() 
{
	// TODO: Add your command handler code here
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	double dX0, dY0, dX1, dY1;

	bXsizeFlag = FALSE;
	bYsizeFlag = FALSE;
	
	dX0 = (dXmax + dXmin)/2;
	dY0 = (dYmax + dYmin)/2;
	
	dX1 = (dXmax - dXmin);
	if( 2*dX1 > pDoc->GetnX()*pDoc->GetdDelta() ) bXsizeFlag = TRUE;

	dY1 = (dYmax - dYmin);
	if( 2*dY1 > pDoc->GetnY()*pDoc->GetdDelta() ) bYsizeFlag = TRUE;

	dXmin = dX0 - dX1;
	dXmax = dX0 + dX1;

	dYmin = dY0 - dY1;
	dYmax = dY0 + dY1;
	
	PrepareData();
	DrawScene();
	WriteIniFile();
	PutTrace1();
	return;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

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

void CFldViewView::PutTrace1()
{
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CString csBuf;
	double dLongUnit = pDoc->GetdLongUnit();
	CString csLongUnit = pDoc->GetcsLongUnit();

	if( bDrawFieldValueFlag )
	{
		csBuf.Format("X = %lg %s; Y = %lg %s; Field = %lg;", 
			( dDrawFieldValueX + pDoc->GetdXmin() )/dLongUnit, csLongUnit,
			( dDrawFieldValueY + pDoc->GetdYmin() )/dLongUnit, csLongUnit,
			pDoc->GetdZ( dDrawFieldValueX + pDoc->GetdXmin() , dDrawFieldValueY + pDoc->GetdYmin() ) );
	}
	else
	{
		csBuf.Format("Xmin = %lg %s; Xmax = %lg %s; Ymin = %lg %s; Ymax = %lg %s; Delta = %lg %s;", 
			(dXmin + pDoc->GetdXmin())/dLongUnit, csLongUnit,
			(dXmax + pDoc->GetdXmin())/dLongUnit, csLongUnit,
			(dYmin + pDoc->GetdYmin())/dLongUnit, csLongUnit,
			(dYmax + pDoc->GetdYmin())/dLongUnit, csLongUnit,
			(pDoc->GetdDelta())/dLongUnit, csLongUnit );
	};
	
	PutTrace( csBuf );
	
	return;
}

CTMCDialogPropet * GetPropDialog( void )
{
	static CTMCDialogPropet cDlg_Global("Parameters");
	return &cDlg_Global;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

void CFldViewView::PutStatistics1()
{
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CString csBuf;
	
	csBuf.Format("nT = %d ; t = %lg %s;", 
		 pDoc->GetnTCurrent(), pDoc->GetdTCurrent()/pDoc->GetdTimeUnit(), pDoc->GetcsTimeUnit() );
	
	PutStatistics( csBuf );
	
	return;
}

void CFldViewView::OnViewZoomPx() 
{
	// TODO: Add your command handler code here
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	double dX0, dX1;

	bXsizeFlag = FALSE;

	dX0 = (dXmax + dXmin)/2;
	
	dX1 = (dXmax - dXmin)/4;
	if( dX1 < 2*pDoc->GetdDelta() ) dX1 = pDoc->GetdDelta();

	dXmin = dX0 - dX1;
	dXmax = dX0 + dX1;
	
	PrepareData();
	DrawScene();
	WriteIniFile();
	PutTrace1();
	return;
}

void CFldViewView::OnViewZoomMx() 
{
	// TODO: Add your command handler code here
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	double dX0, dX1;

	bXsizeFlag = FALSE;
	
	dX0 = (dXmax + dXmin)/2;
	
	dX1 = (dXmax - dXmin);
	if( 2*dX1 > pDoc->GetnX()*pDoc->GetdDelta() ) bXsizeFlag = TRUE;

	dXmin = dX0 - dX1;
	dXmax = dX0 + dX1;

	PrepareData();
	DrawScene();
	WriteIniFile();
	PutTrace1();
	return;
}

void CFldViewView::OnViewZoomMy() 
{
	// TODO: Add your command handler code here
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	double dY0, dY1;

	bYsizeFlag = FALSE;
	
	dY0 = (dYmax + dYmin)/2;
	
	dY1 = (dYmax - dYmin);
	if( 2*dY1 > pDoc->GetnY()*pDoc->GetdDelta() ) bYsizeFlag = TRUE;

	dYmin = dY0 - dY1;
	dYmax = dY0 + dY1;
	
	PrepareData();
	DrawScene();
	WriteIniFile();
	PutTrace1();
	return;
}

void CFldViewView::OnViewZoomPy() 
{
	// TODO: Add your command handler code here
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	double dY0, dY1;

	bYsizeFlag = FALSE;
	
	dY0 = (dYmax + dYmin)/2;
	
	dY1 = (dYmax - dYmin)/4;
	if( dY1 < 2*pDoc->GetdDelta() ) dY1 = pDoc->GetdDelta();

	dYmin = dY0 - dY1;
	dYmax = dY0 + dY1;
	
	PrepareData();
	DrawScene();
	WriteIniFile();
	PutTrace1();
	return;
}

void CFldViewView::OnViewDefault() 
{
	// TODO: Add your command handler code here
	SetDefaultValue();
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnRotatexn() 
{
	// TODO: Add your command handler code here
	OnRotateTop();
	return;
}

void CFldViewView::OnRotatexv() 
{
	// TODO: Add your command handler code here
	OnRotateBottom();
	return;
}

void CFldViewView::OnRotateyn() 
{
	// TODO: Add your command handler code here
	OnRotateRight();
	return;
}

void CFldViewView::OnRotateyv() 
{
	// TODO: Add your command handler code here
	OnRotateLeft();
	return;
}

void CFldViewView::OnRotatezv() 
{
	// TODO: Add your command handler code here
	OnRotateLefty();
	return;
}

void CFldViewView::OnRotatezn() 
{
	// TODO: Add your command handler code here
	OnRotateRighty();
	return;
}

void CFldViewView::OnTranslateXn() 
{
	// TODO: Add your command handler code here
	OnTranslateXb();	
	return;
}

void CFldViewView::OnTranslateXv() 
{
	// TODO: Add your command handler code here
	OnTranslateXt();	
	return;
}

void CFldViewView::OnTranslateYn() 
{
	// TODO: Add your command handler code here
	OnTranslateZb();	
	return;
}

void CFldViewView::OnTranslateYv() 
{
	// TODO: Add your command handler code here
	OnTranslateZt();	
	return;
}

void CFldViewView::OnTranslateZn() 
{
	// TODO: Add your command handler code here
	OnTranslateYt();	
	return;
}

void CFldViewView::OnTranslateZv() 
{
	// TODO: Add your command handler code here
	OnTranslateYb();	
	return;
}

void CFldViewView::OnTranslatedataxn() 
{
	// TODO: Add your command handler code here
	OnStrelkleft();
	return;
}

void CFldViewView::OnTranslatedataxv() 
{
	// TODO: Add your command handler code here
	OnStrelkright();
	return;
}

void CFldViewView::OnTranslatedatayn() 
{
	// TODO: Add your command handler code here
	OnStrelkdown();
	return;
}

void CFldViewView::OnTranslatedatayv() 
{
	// TODO: Add your command handler code here
	OnStrelkup();
	return;
}

void CFldViewView::OnDimension23switch() 
{
	// TODO: Add your command handler code here
	n2D3DFlag = !n2D3DFlag;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnFlagdrawsurface() 
{
	// TODO: Add your command handler code here
	bDrawSurfaceFlag = !bDrawSurfaceFlag;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnFlagdrawline() 
{
	// TODO: Add your command handler code here
	bDrawLineFlag = !bDrawLineFlag;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateFlagdrawline(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawLineFlag );	
	return;
}

void CFldViewView::OnUpdateFlagdrawsurface(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawSurfaceFlag );	
	return;
}

void CFldViewView::OnUpdateDimension23switch(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( n2D3DFlag );	
	pCmdUI->Enable(bDrawSurfaceFlag||bDrawLineFlag);
	return;
}

double CFldViewView::XtoGLWinCoord( double x )
{
	return dXkoefProp*2.5*((x - dXmin)/( dXmax - dXmin) - 0.4);
}

double CFldViewView::YtoGLWinCoord( double y )
{
	return dYkoefProp*2.5*((y - dYmin)/( dYmax - dYmin) - 0.4);
}

double CFldViewView::XtoGLWinCoord( int j )
{
	return dXkoefProp*2.5*((j*dDelta)/( dXmax - dXmin) - 0.4);
}

double CFldViewView::YtoGLWinCoord( int i )
{
	return dYkoefProp*2.5*((i*dDelta)/( dYmax - dYmin) - 0.4);
}

double CFldViewView::ZtoGLWinCoord(double dZcurrent)
{
	return 2*((dZcurrent - dZmin)/( dZmax - dZmin)-0.5);
}

double CFldViewView::Z_EpstoGLWinCoord(double dZcurrent)
{
	return 2*((dZcurrent - dZmin_Eps)/( dZmax_Eps - dZmin_Eps)-0.5);
}

void CFldViewView::DrawAxies()
{
	if( cError.IsError() ) return;
	if( !bDrawFlagAxies ) return;

	CTmcGLText glText1;
	
	SetColor( scAxiesColor );
	// Y - axies
	glBegin( GL_LINES  );
    glVertex3f( (float)(XtoGLWinCoord( (dXmax + dXmin)/2 )), (float)(YtoGLWinCoord( (dYmax + dYmin)/2 )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( (dXmax + dXmin)/2 )), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( 0. )));
    glEnd();
	glBegin(GL_POLYGON);
    glVertex3f( (float)(XtoGLWinCoord( (dXmax + dXmin)/2-0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmax-0.04*(dYmax - dYmin) )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( (dXmax + dXmin)/2 )), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( (dXmax + dXmin)/2+0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmax-0.04*(dYmax - dYmin) )), (float)(ZtoGLWinCoord( 0. )));
    glEnd();
	// X - axies
	glBegin( GL_LINES  );
    glVertex3f( (float)(XtoGLWinCoord( (dXmax + dXmin)/2 )), (float)(YtoGLWinCoord( (dYmax + dYmin)/2 )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmax )), (float)(YtoGLWinCoord( (dYmax + dYmin)/2 )), (float)(ZtoGLWinCoord( 0. )));
    glEnd();
	glBegin(GL_POLYGON);
    glVertex3f( (float)(XtoGLWinCoord( dXmax - 0.04*(dXmax - dXmin) )), (float)(YtoGLWinCoord( (dYmax + dYmin)/2 - 0.02*(dYmax - dYmin) )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmax )), (float)(YtoGLWinCoord( (dYmax + dYmin)/2 )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmax - 0.04*(dXmax - dXmin) )), (float)(YtoGLWinCoord( (dYmax + dYmin)/2 + 0.02*(dYmax - dYmin) )), (float)(ZtoGLWinCoord( 0. )));
    glEnd();
	// Z - axies
	glBegin( GL_LINES  );
    glVertex3f( (float)(XtoGLWinCoord( (dXmax + dXmin)/2 )), (float)(YtoGLWinCoord( (dYmax + dYmin)/2 )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( (dXmax + dXmin)/2 )), (float)(YtoGLWinCoord( (dYmax + dYmin)/2 )), (float)(ZtoGLWinCoord( dZmax )));
    glEnd();
	glBegin(GL_POLYGON);
    glVertex3f( (float)(XtoGLWinCoord( (dXmax + dXmin)/2 - 0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( (dYmax + dYmin)/2  )), (float)(ZtoGLWinCoord( (1. - 0.04)*dZmax )));
    glVertex3f( (float)(XtoGLWinCoord( (dXmax + dXmin)/2 )), (float)(YtoGLWinCoord( (dYmax + dYmin)/2 )), (float)(ZtoGLWinCoord( dZmax )));
    glVertex3f( (float)(XtoGLWinCoord( (dXmax + dXmin)/2 + 0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( (dYmax + dYmin)/2  )), (float)(ZtoGLWinCoord( (1. - 0.04)*dZmax )));
    glEnd();

	glText1.SetColor( scAxiesColor );
	glText1.SetFont( &lfAxiesFont );
	glText1.DrawString( "X", XtoGLWinCoord( dXmax ), YtoGLWinCoord( (dYmax + dYmin)/2 ), ZtoGLWinCoord( 0. ) );
	glText1.DrawString( "Y", XtoGLWinCoord( (dXmax + dXmin)/2 ), YtoGLWinCoord( dYmax ), ZtoGLWinCoord( 0. ) );
	glText1.DrawString( "Z", XtoGLWinCoord( (dXmax + dXmin)/2 ), YtoGLWinCoord( (dYmax + dYmin)/2 ), ZtoGLWinCoord( dZmax ) );
	
	return;
}

void CFldViewView::DrawAxiesSize()
{
	DrawAxiesSizeX();
	DrawAxiesSizeY();
	DrawAxiesSizeZField();
	DrawAxiesSizeZEps();
	return;
}

void CFldViewView::DrawAxiesSizeX()
{
	if( cError.IsError() ) return;
	if( !bDrawFlagAxiesXSize ) return;
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CTmcGLText glText1;
	CString csBuf;
	
	SetColor( scAxiesColor );
	// X - axies
	glBegin( GL_LINES  );
    glVertex3f( (float)(XtoGLWinCoord( dXmin )), (float)(YtoGLWinCoord( dYmin - (dYmax - dYmin)*0.04 )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmax )), (float)(YtoGLWinCoord( dYmin - (dYmax - dYmin)*0.04 )), (float)(ZtoGLWinCoord( 0. )));
    glEnd();
	glBegin(GL_POLYGON);
    glVertex3f( (float)(XtoGLWinCoord( dXmax - 0.04*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmin - (dYmax - dYmin)*0.04 - 0.02*(dYmax - dYmin) )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmax )), (float)(YtoGLWinCoord( dYmin - (dYmax - dYmin)*0.04 )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmax - 0.04*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmin - (dYmax - dYmin)*0.04 + 0.02*(dYmax - dYmin) )), (float)(ZtoGLWinCoord( 0. )));
    glEnd();
	glBegin(GL_POLYGON);
    glVertex3f( (float)(XtoGLWinCoord( dXmin + 0.04*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmin - (dYmax - dYmin)*0.04 - 0.02*(dYmax - dYmin) )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin )), (float)(YtoGLWinCoord( dYmin - (dYmax - dYmin)*0.04 )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin + 0.04*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmin - (dYmax - dYmin)*0.04 + 0.02*(dYmax - dYmin) )), (float)(ZtoGLWinCoord( 0. )));
    glEnd();
	glBegin( GL_LINES  );
    glVertex3f( (float)(XtoGLWinCoord( dXmin )), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin )), (float)(YtoGLWinCoord( dYmin - 2*(dYmax - dYmin)*0.04 )), (float)(ZtoGLWinCoord( 0. )));
    glEnd();
	glBegin( GL_LINES  );
    glVertex3f( (float)(XtoGLWinCoord( dXmax )), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmax )), (float)(YtoGLWinCoord( dYmin - 2*(dYmax - dYmin)*0.04 )), (float)(ZtoGLWinCoord( 0. )));
    glEnd();

	glText1.SetColor( scAxiesColor );
	glText1.SetFont( &lfAxiesFont );
	csBuf.Format( "%lg %s", (dXmax - dXmin)/pDoc->GetdLongUnit(), pDoc->GetcsLongUnit() );
	glText1.DrawString( csBuf, XtoGLWinCoord( (dXmax + dXmin)/2 ), YtoGLWinCoord( dYmin - (dYmax - dYmin)*0.04 ), ZtoGLWinCoord( 0. ) );
	
	return;
}

void CFldViewView::DrawAxiesSizeY()
{
	if( cError.IsError() ) return;
	if( !bDrawFlagAxiesYSize ) return;
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CTmcGLText glText1;
	CString csBuf;
	
	SetColor( scAxiesColor );
	// Y - axies
	glBegin( GL_LINES  );
    glVertex3f( (float)(XtoGLWinCoord( dXmin - (dXmax - dXmin)*0.04 )), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin - (dXmax - dXmin)*0.04 )), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( 0. )));
    glEnd();
	glBegin(GL_POLYGON);
    glVertex3f( (float)(XtoGLWinCoord( dXmin - (dXmax - dXmin)*0.04-0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmax-0.04*(dYmax - dYmin) )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin - (dXmax - dXmin)*0.04 )), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin - (dXmax - dXmin)*0.04+0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmax-0.04*(dYmax - dYmin) )), (float)(ZtoGLWinCoord( 0. )));
    glEnd();
	glBegin(GL_POLYGON);
    glVertex3f( (float)(XtoGLWinCoord( dXmin - (dXmax - dXmin)*0.04-0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmin+0.04*(dYmax - dYmin) )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin - (dXmax - dXmin)*0.04 )), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin - (dXmax - dXmin)*0.04+0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmin+0.04*(dYmax - dYmin) )), (float)(ZtoGLWinCoord( 0. )));
    glEnd();
	glBegin( GL_LINES  );
    glVertex3f( (float)(XtoGLWinCoord( dXmin )), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin - 2*(dXmax - dXmin)*0.04 )), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( 0. )));
    glEnd();
	glBegin( GL_LINES  );
    glVertex3f( (float)(XtoGLWinCoord( dXmin )), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( 0. )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin - 2*(dXmax - dXmin)*0.04 )), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( 0. )));
    glEnd();

	glText1.SetColor( scAxiesColor );
	glText1.SetFont( &lfAxiesFont );
	csBuf.Format( "%lg %s", (dYmax - dYmin)/pDoc->GetdLongUnit(), pDoc->GetcsLongUnit() );
	glText1.DrawString( csBuf, XtoGLWinCoord( dXmin - (dXmax - dXmin)*0.04 ), YtoGLWinCoord( (dYmax + dYmin)/2 ), ZtoGLWinCoord( 0. ) );
	
	return;
}

void CFldViewView::DrawAxiesSizeZField()
{
	if( cError.IsError() ) return;
	if( !bDrawFlagAxiesZFieldSize ) return;

	CTmcGLText glText1;
	CString csBuf;
	
	SetColor( scAxiesColor );
	// Z - axies
	glBegin( GL_LINES  );
    glVertex3f( (float)(XtoGLWinCoord( dXmin )), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( -dZmax )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin )), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( dZmax )));
    glEnd();
	glBegin(GL_POLYGON);
    glVertex3f( (float)(XtoGLWinCoord( dXmin - 0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( (1. - 0.04)*dZmax )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin )), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( dZmax )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin + 0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( (1. - 0.04)*dZmax )));
    glEnd();
	glBegin(GL_POLYGON);
    glVertex3f( (float)(XtoGLWinCoord( dXmin - 0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( -(1. - 0.04)*dZmax )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin )), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( -dZmax )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin + 0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( -(1. - 0.04)*dZmax )));
    glEnd();

	glText1.SetColor( scAxiesColor );
	glText1.SetFont( &lfAxiesFont );

	if( bDrawFlagAxiesZFieldSize_Eps )	csBuf.Format( "Field %lg", dZmin );
	else csBuf.Format( "%lg", dZmin );
	glText1.DrawString( csBuf, (float)(XtoGLWinCoord( dXmin ) ), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( dZmin )) );
	
	if( bDrawFlagAxiesZFieldSize_Eps )	csBuf.Format( "Field %lg", dZmax );
	else csBuf.Format( "%lg", dZmax );
	glText1.DrawString( csBuf, (float)(XtoGLWinCoord( dXmin ) ), (float)(YtoGLWinCoord( dYmin )), (float)(ZtoGLWinCoord( dZmax )) );
	
	return;
}

void CFldViewView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	bIsMouseLButonDown = TRUE;
	cMousePoint = point;
	PutTrace1();
	CScrollView::OnLButtonDown(nFlags, point);
}

void CFldViewView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	bIsMouseLButonDown = FALSE;
	CScrollView::OnLButtonUp(nFlags, point);
}

void CFldViewView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if( bIsMouseLButonDown )
	{
		if( abs(point.x - cMousePoint.x) > 0.01*cMousePoint.x )
		{
			if( point.x < cMousePoint.x ) OnRotateLefty();
			else OnRotateRighty();
		};
		if( abs(point.y - cMousePoint.y) > 0.01*cMousePoint.y )
		{
			if( point.y < cMousePoint.y ) OnRotatexn();
			else OnRotatexv();
		};
		cMousePoint = point;
	};
	CScrollView::OnMouseMove(nFlags, point);
}

void CFldViewView::OnDestroy() 
{
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pDoc->DelViews__();

	CScrollView::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

LRESULT CFldViewView::OnUserApplyPropDialogPlace( WPARAM wParam, LPARAM lParam)
{
	CTMCDialogPropet *cDlg = GetPropDialog();
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	bXsizeFlag = cDlg->cPlace.m_AutoFlagX;
	bYsizeFlag = cDlg->cPlace.m_m_AutoFlagY;
	bDrawFlagAxies = cDlg->cPlace.m_m_AxiesDrawFlag;
	wAngleX = (GLfloat)cDlg->cPlace.m_dXAxiesAngle;
	wAngleY = (GLfloat)cDlg->cPlace.m_dYAxiesAngle;
	dXmax = cDlg->cPlace.m_dXAxiesMax*pDoc->GetdLongUnit() - pDoc->GetdXmin();
	dYmax = cDlg->cPlace.m_dYAxiesMax*pDoc->GetdLongUnit() - pDoc->GetdYmin();
	dXmin = cDlg->cPlace.m_dXAxiesMin*pDoc->GetdLongUnit() - pDoc->GetdXmin();
	dYmin = cDlg->cPlace.m_dYAxiesMin*pDoc->GetdLongUnit() - pDoc->GetdYmin();
	wTranslateX = (GLfloat)cDlg->cPlace.m_dXAxiesTrans;
	wTranslateY = (GLfloat)cDlg->cPlace.m_dYAxiesTrans;
	scAxiesColor = cDlg->cPlace.scColorAxies;
	lfAxiesFont = cDlg->cPlace.lfInitial;
	bDrawFlagAxiesXSize = cDlg->cPlace.m_DrawFlafSizeX;
	bDrawFlagAxiesYSize = cDlg->cPlace.m_DrawFlafSizeY;
	bProportionally = cDlg->cPlace.m_Proportionally;

	PrepareData();
	DrawScene();
	WriteIniFile();

	return 0;
}

LRESULT CFldViewView::OnUserApplyPropDialogTopol( WPARAM wParam, LPARAM lParam)
{
	CTMCDialogPropet *cDlg = GetPropDialog();
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	scColorEps = cDlg->cTopology.scColorEps;
	scColorMet = cDlg->cTopology.scColorMet;
	scColorMag = cDlg->cTopology.scColorMag;
	scColorInp = cDlg->cTopology.scColorInp;
	scColorAbs = cDlg->cTopology.scColorAbs;
	nBlendTopology = cDlg->cTopology.m_Blend;
	if( nBlendTopology <   0 ) nBlendTopology =   0;
	if( nBlendTopology > 100 ) nBlendTopology = 100;
	bDrawFlagEps   = cDlg->cTopology.m_DrawFlagEps;
	bDrawFlagInput = cDlg->cTopology.m_DrawFlagInput;
	bDrawFlagMetal = cDlg->cTopology.m_DrawFlagMetal;
	bDrawLineFlagTopology = cDlg->cTopology.m_DrawFlagLine;
	n2D3DFlagTopology     = cDlg->cTopology.m_23Dimention;
	bDrawSurfaceFlagTopology  = cDlg->cTopology.m_DrawFlagSurface;
	bDrawFlagMagnetic         = cDlg->cTopology.m_DrawFlagMagnetic;
	bTopologySizeFlag         = cDlg->cTopology.m_DrawFlagSizeBlock;
	bTopologySizeFlagLinkList = cDlg->cTopology.m_DrawFlagSizeLinkList;
	bDrawFlagAbsorber		  = cDlg->cTopology.m_DrawFlagAbsorber;

	PrepareData();
	DrawScene();
	WriteIniFile();

	return 0;
}

LRESULT CFldViewView::OnUserApplyPropDialogField( WPARAM wParam, LPARAM lParam)
{
	CTMCDialogPropet *cDlg = GetPropDialog();
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	int i;
		
	for( i = 0; i < 20; i++ )	pscSurfaceColor[i] = cDlg->cField.c20Color[i];
	n2D3DFlag        = cDlg->cField.m_d23Dimension;
	nBlend           = cDlg->cField.m_dBlend;
	if( nBlend <   0 ) nBlend =   0;
	if( nBlend > 100 ) nBlend = 100;
	bHightColorRezolution = cDlg->cField.bHightColorResolution;
	bDrawLineFlag    = cDlg->cField.m_bDrawFlagLine;
	bDrawSurfaceFlag = cDlg->cField.m_bDrawFlagSurface;
	bZsizeFlag       = cDlg->cField.m_bAutoSizeFlag;
	dZmax            = cDlg->cField.m_dZAxiesMax;
	dZmin            = cDlg->cField.m_dZAxiesMin;
	if( bDrawFieldModulFlag )
	{
		if( fabs(dZmax) < fabs(dZmin) )	dZmax = fabs(dZmin);
		else dZmax = fabs(dZmax);
		dZmin = -dZmax;
	}
	bDrawFlagAxiesZFieldSize = cDlg->cField.m_DrawSizeFlag;
	bDrawFieldModulFlag = cDlg->cField.bModulFlag;

	PrepareData();
	DrawScene();
	WriteIniFile();

	return 0;
}

LRESULT CFldViewView::OnUserApplyPropDialogEps( WPARAM wParam, LPARAM lParam)
{
	CTMCDialogPropet *cDlg = GetPropDialog();
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	int i;

	for( i = 0; i < 20; i++ )	pscSurfaceColor_Eps[i] = cDlg->cEps.c20Color[i];
	n2D3DFlag_Eps        = cDlg->cEps.m_d23Dimension;
	nBlend_Eps           = cDlg->cEps.m_dBlend;
	if( nBlend_Eps <   0 ) nBlend_Eps =   0;
	if( nBlend_Eps > 100 ) nBlend_Eps = 100;
	bDrawLineFlag_Eps    = cDlg->cEps.m_bDrawFlagLine;
	bDrawSurfaceFlag_Eps = cDlg->cEps.m_bDrawFlagSurface;
	bZsizeFlag_Eps       = cDlg->cEps.m_bAutoSizeFlag;
	dZmax_Eps            = cDlg->cEps.m_dZAxiesMax;
	dZmin_Eps            = cDlg->cEps.m_dZAxiesMin;
	bDrawFlagAxiesZFieldSize_Eps = cDlg->cEps.m_DrawSizeFlag;

	PrepareData();
	DrawScene();
	WriteIniFile();

	return 0;
}


void CFldViewView::SetTplFileName()
{
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	int i;
	char szBuf[10000];
	
	strcpy( szBuf, pDoc->GetPathName() );
	
	for( i = strlen( szBuf ); szBuf[i] != '.'; i-- ) 
	{
		if( i <= 0 ) 
		{
			cError.PutErrorMessage("can't transform *.tf to *.tt file name");
			RedrawWindow();
			return;
		};
	};

	szBuf[i] = '\0';
	csTplFileName.Format( "%s.tt", szBuf );

	return;
}

void CFldViewView::ReadTpl()
{
	if( cError.IsError() ) return;
	if( (!bDrawLineFlagTopology)&&(!bDrawSurfaceFlagTopology)&&(!bDrawSurfaceFlag_Eps)&&(!bDrawLineFlag_Eps) ) return;
	SetTplFileName();
	if( cError.IsError() ) return;
	cBlockTpl.Read( csTplFileName, cError);
	if( cError.IsError() ) return;

	nBoundBlockTpl = 0;
	if( pcBoundBlockTpl != NULL ) delete []pcBoundBlockTpl;
	pcBoundBlockTpl = NULL;

	if( (!bDrawLineFlagTopology)&&(!bDrawSurfaceFlagTopology) ) return;

	nBoundBlockTpl = cBlockTpl.GetBlockNumber();
	if( nBoundBlockTpl < 1 )
	{
		cError.PutErrorMessage("Block Numbers < 1");
		RedrawWindow();
		return;
	};
	if( (pcBoundBlockTpl = new CTmcBoundaryTpl[nBoundBlockTpl]) == NULL )
	{
		nBoundBlockTpl = 0;
		cError.PutErrorMessage("memory allocation error for boundary topology array");
		RedrawWindow();
		return;
	};

	int i, j, nType;
	CTmcRTH_BlockList *pcBlList;
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	dDelta = pDoc->GetdDelta();

	for( i = 0, j = 0, pcBlList = (CTmcRTH_BlockList *)(&cBlockTpl); i < nBoundBlockTpl; i++ )
	{
		pcBoundBlockTpl[i].SetBlockList( pcBlList, cError, dDelta);
		if( cError.IsError() ) 
		{
			RedrawWindow();
			return;
		};
		nType = pcBoundBlockTpl[i].GetnType();
		if( (nType == CTMCRTH_BLCKNTYPE_INPXLEFT)||(nType == CTMCRTH_BLCKNTYPE_INPXRIGHT)||(nType == CTMCRTH_BLCKNTYPE_INPYTOP)||(nType == CTMCRTH_BLCKNTYPE_INPYBOT) )
		{
			j++;
			pcBoundBlockTpl[i].SetnInputNum( j );
		};
		pcBlList = pcBlList->GetNext();
	}

	return;
}

void CFldViewView::PrepareTpl()
{
	if( cError.IsError() ) return;

	if( pdSurface_Eps != NULL ) delete []pdSurface_Eps;
	pdSurface_Eps = NULL;

	if( (!bDrawSurfaceFlag_Eps)&&(!bDrawLineFlag_Eps) ) return;

	int i, j, n, ii, nX1, jj;
	double *pdBuf, r;
	
	if( nX < 2 )
	{
		cError.PutErrorMessage("number of point along X axies < 2");
		RedrawWindow();
		return;
	};

	if( nY < 2 )
	{
		cError.PutErrorMessage("number of point along Y axies < 2");
		RedrawWindow();
		return;
	};

	if( ( pdBuf = cBlockTpl.GetpdSurface() ) == NULL ) return;

	n = nX*nY;
	
	if( ( pdSurface_Eps = new double[n] ) == NULL ) 
	{
		cError.PutErrorMessage("allocation memory for double array");
		RedrawWindow();
		return;
	};

	if( fabs(dZmax_Eps - dZmin_Eps) < FLT_MIN ) 
	{
		r = dZmax_Eps;
		dZmax_Eps = r + FLT_MIN;
		dZmin_Eps = r - FLT_MIN;
	};

	nX1 = cBlockTpl.GetnX();
	if( bZsizeFlag_Eps )
	{
		for( i = 0, ii = 0, jj = (int)(dXmin/dDelta)+((int)(dYmin/dDelta))*nX1; i < nY; i++, jj+=nX1 )
		{
			for( j = 0; j < nX; j++, ii++ )
			{
				if( dZmax_Eps < pdBuf[jj+j] ) dZmax_Eps = pdBuf[jj+j];
				if( dZmin_Eps > pdBuf[jj+j] ) dZmin_Eps = pdBuf[jj+j];
			};
		};
	};

	for( i = 0, ii = 0, jj = (int)(dXmin/dDelta)+(int)(dYmin/dDelta)*nX1; i < nY; i++, jj+=nX1 )
	{
		for( j = 0; j < nX; j++, ii++ )
		{
			pdSurface_Eps[ii]  = Z_EpstoGLWinCoord( pdBuf[jj+j] );
		};
	};

	if( dZmax_Eps < dZmin_Eps ) 
	{
		r = dZmax_Eps;
		dZmax_Eps = dZmin_Eps;
		dZmin_Eps = r;
	};

	return;
}

void CFldViewView::DrawEps()
{
	if( cError.IsError() ) return;

	switch( n2D3DFlag_Eps )
	{
		case TMCFIELDVIEW_2DDraw:
			DrawEpsSurface2();
			DrawEpsdLine2();
			break;
		case TMCFIELDVIEW_3DDraw:
			DrawEpsSurface();
			DrawEpsdLine();
			break;
	};
	
	return;
}

void CFldViewView::DrawEpsSurface2()
{
	if( cError.IsError() ) return;
	if( pdSurface_Eps  == NULL ) return;
	if( pdSurfaceX == NULL ) return;
	if( pdSurfaceY == NULL ) return;

	if( !bDrawSurfaceFlag_Eps ) return;

	int nY1, nX1;
	int i, j, ii;

	nY1 = nY - 1;
	nX1 = nX - 1;

	for( i = 0, ii = 0; i < nY1; i++, ii++ )
	{
		for( j = 0; j < nX1; j++, ii++ )
		{
			glBegin(GL_POLYGON);
			SetColor_Eps( pdSurface_Eps[ii] );
		    glVertex2f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]) );
			SetColor_Eps( pdSurface_Eps[ii+1] );
		    glVertex2f( (float)(pdSurfaceX[ii+1]), (float)(pdSurfaceY[ii+1]) );
			SetColor_Eps( pdSurface_Eps[ii+1+nX] );
		    glVertex2f( (float)(pdSurfaceX[ii+1+nX]), (float)(pdSurfaceY[ii+1+nX]) );
			SetColor_Eps( pdSurface_Eps[ii+nX] );
		    glVertex2f( (float)(pdSurfaceX[ii+nX]), (float)(pdSurfaceY[ii+nX]) );
		    glEnd();
		};
	};
	return;
}

void CFldViewView::DrawEpsdLine2()
{
	if( cError.IsError() ) return;
	if( pdSurface_Eps  == NULL ) return;
	if( pdSurfaceX == NULL ) return;
	if( pdSurfaceY == NULL ) return;

	if( !bDrawLineFlag_Eps ) return;

	int nY1, nX1;
	int i, j, ii;

	nY1 = nY - 1;
	nX1 = nX - 1;

	if( bDrawSurfaceFlag_Eps )
	{
		glColor4f( 1.0f, 1.0f, 1.0f, (float)(0.8) );
		for( i = 0, ii = 0; i < nY; i++, ii++ )
		{
			glBegin( GL_LINES  );
			for( j = 0; j < nX1; j++, ii++ )
			{
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(0.01));
			    glVertex3f( (float)(pdSurfaceX[ii+1]), (float)(pdSurfaceY[ii+1]), (float)(0.01));
			};
		    glEnd();
		};
		for( i = 0; i < nX; i++ )
		{
			glBegin( GL_LINES );
			for( j = 0, ii = i; j < nY1; j++, ii+=nX )
			{
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(0.01));
			    glVertex3f( (float)(pdSurfaceX[ii+nX]), (float)(pdSurfaceY[ii+nX]), (float)(0.01));
			};
		    glEnd();
		};
	}
	else
	{
		for( i = 0, ii = 0; i < nY; i++, ii++ )
		{
			glBegin( GL_LINES  );
			for( j = 0; j < nX1; j++, ii++ )
			{
				SetColor_Eps( pdSurface_Eps[ii] );
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(0.01));
				SetColor_Eps( pdSurface_Eps[ii+1] );
			    glVertex3f( (float)(pdSurfaceX[ii+1]), (float)(pdSurfaceY[ii+1]), (float)(0.01));
			};
		    glEnd();
		};
		for( i = 0; i < nX; i++ )
		{
			glBegin( GL_LINES );
			for( j = 0, ii = i; j < nY1; j++, ii+=nX )
			{
				SetColor_Eps( pdSurface_Eps[ii] );
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(0.01));
				SetColor_Eps( pdSurface_Eps[ii+nX] );
			    glVertex3f( (float)(pdSurfaceX[ii+nX]), (float)(pdSurfaceY[ii+nX]), (float)(0.01));
			};
		    glEnd();
		};
	};
	return;
}

void CFldViewView::DrawEpsSurface()
{
	if( cError.IsError() ) return;
	if( pdSurface_Eps  == NULL ) return;
	if( pdSurfaceX == NULL ) return;
	if( pdSurfaceY == NULL ) return;

	if( !bDrawSurfaceFlag_Eps ) return;

	int nY1, nX1;
	int i, j, ii;

	nY1 = nY - 1;
	nX1 = nX - 1;

	for( i = 0, ii = 0; i < nY1; i++, ii++ )
	{
		for( j = 0; j < nX1; j++, ii++ )
		{
			glBegin(GL_POLYGON);
			SetColor_Eps( pdSurface_Eps[ii] );
		    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(pdSurface_Eps[ii]));
			SetColor_Eps( pdSurface_Eps[ii+1] );
		    glVertex3f( (float)(pdSurfaceX[ii+1]), (float)(pdSurfaceY[ii+1]), (float)(pdSurface_Eps[ii+1]));
			SetColor_Eps( pdSurface_Eps[ii+1+nX] );
		    glVertex3f( (float)(pdSurfaceX[ii+1+nX]), (float)(pdSurfaceY[ii+1+nX]), (float)(pdSurface_Eps[ii+1+nX]));
			SetColor_Eps( pdSurface_Eps[ii+nX] );
		    glVertex3f( (float)(pdSurfaceX[ii+nX]), (float)(pdSurfaceY[ii+nX]), (float)(pdSurface_Eps[ii+nX]));
		    glEnd();
		};
	};
	return;
}

void CFldViewView::DrawEpsdLine()
{
	if( cError.IsError() ) return;
	if( pdSurface_Eps  == NULL ) return;
	if( pdSurfaceX == NULL ) return;
	if( pdSurfaceY == NULL ) return;

	if( !bDrawLineFlag_Eps ) return;

	int nY1, nX1;
	int i, j, ii;

	nY1 = nY - 1;
	nX1 = nX - 1;

	if( bDrawSurfaceFlag_Eps )
	{
		glColor4f( 1.0f, 1.0f, 1.0f, (float)(0.8) );
		for( i = 0, ii = 0; i < nY; i++, ii++ )
		{
			glBegin( GL_LINES  );
			for( j = 0; j < nX1; j++, ii++ )
			{
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(pdSurface_Eps[ii]));
			    glVertex3f( (float)(pdSurfaceX[ii+1]), (float)(pdSurfaceY[ii+1]), (float)(pdSurface_Eps[ii+1]));
			};
		    glEnd();
		};
		for( i = 0; i < nX; i++ )
		{
			glBegin( GL_LINES );
			for( j = 0, ii = i; j < nY1; j++, ii+=nX )
			{
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(pdSurface_Eps[ii]));
			    glVertex3f( (float)(pdSurfaceX[ii+nX]), (float)(pdSurfaceY[ii+nX]), (float)(pdSurface_Eps[ii+nX]));
			};
		    glEnd();
		};
	}
	else
	{
		for( i = 0, ii = 0; i < nY; i++, ii++ )
		{
			glBegin( GL_LINES  );
			for( j = 0; j < nX1; j++, ii++ )
			{
				SetColor_Eps( pdSurface_Eps[ii] );
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(pdSurface_Eps[ii]));
				SetColor_Eps( pdSurface_Eps[ii+1] );
			    glVertex3f( (float)(pdSurfaceX[ii+1]), (float)(pdSurfaceY[ii+1]), (float)(pdSurface_Eps[ii+1]));
			};
		    glEnd();
		};
		for( i = 0; i < nX; i++ )
		{
			glBegin( GL_LINES );
			for( j = 0, ii = i; j < nY1; j++, ii+=nX )
			{
				SetColor_Eps( pdSurface_Eps[ii] );
			    glVertex3f( (float)(pdSurfaceX[ii]), (float)(pdSurfaceY[ii]), (float)(pdSurface_Eps[ii]));
				SetColor_Eps( pdSurface_Eps[ii+nX] );
			    glVertex3f( (float)(pdSurfaceX[ii+nX]), (float)(pdSurfaceY[ii+nX]), (float)(pdSurface_Eps[ii+nX]));
			};
		    glEnd();
		};
	};
	return;
}

void CFldViewView::SetColor_Eps( double z )
{
	int i;

	if( z >  1. ) z =  1.0001;
	if( z < -1. ) z = -1.0001;

//	i = (int)(( fabs( z - (dZmax+dZmin)/2 )/((dZmax-dZmin)/2)*20 -0.5));
	i = (int)( 10 + z*10 );

	glColor4f( ((float)(GetRValue(pscSurfaceColor_Eps[i])))/256,
				((float)(GetGValue(pscSurfaceColor_Eps[i])))/256,
				((float)(GetBValue(pscSurfaceColor_Eps[i])))/256,
				(float)(nBlend_Eps/100.) );

	return;
}


void CFldViewView::OnFlagdrawlineeps() 
{
	// TODO: Add your command handler code here
	bDrawLineFlag_Eps = !bDrawLineFlag_Eps;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnFlagdrawsurfaceeps() 
{
	// TODO: Add your command handler code here
	bDrawSurfaceFlag_Eps = !bDrawSurfaceFlag_Eps;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnDimension23switcheps() 
{
	// TODO: Add your command handler code here
	n2D3DFlag_Eps = !n2D3DFlag_Eps;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateDimension23switcheps(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( n2D3DFlag_Eps );	
	pCmdUI->Enable(bDrawSurfaceFlag_Eps||bDrawLineFlag_Eps);
	return;
}

void CFldViewView::OnUpdateFlagdrawlineeps(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawLineFlag_Eps );	
	return;
}

void CFldViewView::OnUpdateFlagdrawsurfaceeps(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawSurfaceFlag_Eps );	
	return;
}

void CFldViewView::DrawAxiesSizeZEps()
{
	if( cError.IsError() ) return;
	if( !bDrawFlagAxiesZFieldSize_Eps ) return;

	CTmcGLText glText1;
	CString csBuf;
	
	SetColor( scAxiesColor );
	// Z - axies
	glBegin( GL_LINES  );
    glVertex3f( (float)(XtoGLWinCoord( dXmin )), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( -dZmax )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin )), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( dZmax )));
    glEnd();
	glBegin(GL_POLYGON);
    glVertex3f( (float)(XtoGLWinCoord( dXmin - 0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( (1. - 0.04)*dZmax )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin )), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( dZmax )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin + 0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( (1. - 0.04)*dZmax )));
    glEnd();
	glBegin(GL_POLYGON);
    glVertex3f( (float)(XtoGLWinCoord( dXmin - 0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( -(1. - 0.04)*dZmax )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin )), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( -dZmax )));
    glVertex3f( (float)(XtoGLWinCoord( dXmin + 0.02*(dXmax - dXmin) )), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( -(1. - 0.04)*dZmax )));
    glEnd();

	glText1.SetColor( scAxiesColor );
	glText1.SetFont( &lfAxiesFont );

	if( bDrawFlagAxiesZFieldSize )	csBuf.Format( "Eps %lg", dZmin_Eps );
	else csBuf.Format( "%lg", dZmin_Eps );
	glText1.DrawString( csBuf, (float)(XtoGLWinCoord( dXmin ) ), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( dZmin )) );
	
	if( bDrawFlagAxiesZFieldSize )	csBuf.Format( "Eps %lg", dZmax_Eps );
	else csBuf.Format( "%lg", dZmax_Eps );
	glText1.DrawString( csBuf, (float)(XtoGLWinCoord( dXmin ) ), (float)(YtoGLWinCoord( dYmax )), (float)(ZtoGLWinCoord( dZmax )) );
	
	return;
}

void CFldViewView::PrepareBoundaryTpl()
{
	if( cError.IsError() ) return;
	if( (!bDrawLineFlagTopology)&&(!bDrawSurfaceFlagTopology) ) return;
	if( pcBoundBlockTpl == NULL ) return;

	CalcnTopPolygon();
	if( pnTopType != NULL ) delete []pnTopType;
	pnTopType = NULL;
	if( pdTopX1 != NULL ) delete []pdTopX1;
	pdTopX1 = NULL;
	if( pdTopX2 != NULL ) delete []pdTopX2;
	pdTopX2 = NULL;
	if( pdTopX3 != NULL ) delete []pdTopX3;
	pdTopX3 = NULL;
	if( pdTopX4 != NULL ) delete []pdTopX4;
	pdTopX4 = NULL;
	if( pdTopY1 != NULL ) delete []pdTopY1;
	pdTopY1 = NULL;
	if( pdTopY2 != NULL ) delete []pdTopY2;
	pdTopY2 = NULL;
	if( pdTopY3 != NULL ) delete []pdTopY3;
	pdTopY3 = NULL;
	if( pdTopY4 != NULL ) delete []pdTopY4;
	pdTopY4 = NULL;
	if( pdTopXinp != NULL ) delete []pdTopXinp;
	pdTopXinp = NULL;
	if( pdTopYinp != NULL ) delete []pdTopYinp;
	pdTopYinp = NULL;


	if( nInputNum > 0 )
	{
		pdTopXinp = new double[nInputNum];
		pdTopYinp = new double[nInputNum];
		if( (pdTopXinp == NULL)||(pdTopXinp == NULL) )
		{
			if( pdTopXinp != NULL ) delete []pdTopXinp;
			pdTopXinp = NULL;
			if( pdTopYinp != NULL ) delete []pdTopYinp;
			pdTopYinp = NULL;
			nInputNum = 0;
			cError.PutErrorMessage("memory allocation error for topology array");
			RedrawWindow();
			return;
		};
	};

	if( nTopPolygon > 0 )
	{
		pnTopType = new int[nTopPolygon];
		pdTopX1 = new double[nTopPolygon];
		pdTopX2 = new double[nTopPolygon];
		pdTopX3 = new double[nTopPolygon];
		pdTopX4 = new double[nTopPolygon];
		pdTopY1 = new double[nTopPolygon];
		pdTopY2 = new double[nTopPolygon];
		pdTopY3 = new double[nTopPolygon];
		pdTopY4 = new double[nTopPolygon];
	}
	else
	{
		if( pdTopXinp != NULL ) delete []pdTopXinp;
		pdTopXinp = NULL;
		if( pdTopYinp != NULL ) delete []pdTopYinp;
		pdTopYinp = NULL;
		nInputNum = 0;
		return;
	};

	if( (pdTopX1 == NULL)||(pdTopX2 == NULL)||(pdTopX3 == NULL)||(pdTopX4 == NULL)||
		(pdTopY1 == NULL)||(pdTopY2 == NULL)||(pdTopY3 == NULL)||(pdTopY4 == NULL)||
		(pnTopType == NULL))
	{
		cError.PutErrorMessage("memory allocation error for topology array");
		RedrawWindow();
		return;
	};

	int i, j, n, ii, jj, nType; 
	double *pX1, *pX2, *pX3, *pX4, *pY1, *pY2, *pY3, *pY4;
	double dXmin_1, dXmax_1, dYmin_1, dYmax_1;
	
	dXmin_1 = dXmin - 1.1*dDelta;
	dXmax_1 = dXmax + 1.1*dDelta;
	dYmin_1 = dYmin - 1.1*dDelta;
	dYmax_1 = dYmax + 1.1*dDelta;
	
	for( i = 0, jj = 0, ii = 0; i < nBoundBlockTpl; i++ )
	{
		pX1 = pcBoundBlockTpl[i].GetpdX1();
		pX2 = pcBoundBlockTpl[i].GetpdX2();
		pX3 = pcBoundBlockTpl[i].GetpdX3();
		pX4 = pcBoundBlockTpl[i].GetpdX4();
		pY1 = pcBoundBlockTpl[i].GetpdY1();
		pY2 = pcBoundBlockTpl[i].GetpdY2();
		pY3 = pcBoundBlockTpl[i].GetpdY3();
		pY4 = pcBoundBlockTpl[i].GetpdY4();
		n = pcBoundBlockTpl[i].GetnPoint();
		nType = pcBoundBlockTpl[i].GetnType();
		if( ((nType == CTMCRTH_BLCKNTYPE_EPS)&&(bDrawFlagEps))||
			(((nType == CTMCRTH_BLCKNTYPE_INPXLEFT)||(nType == CTMCRTH_BLCKNTYPE_INPXRIGHT)||(nType == CTMCRTH_BLCKNTYPE_INPYTOP)||(nType == CTMCRTH_BLCKNTYPE_INPYBOT))&&(bDrawFlagInput))||
			((nType == CTMCRTH_BLCKNTYPE_ABSORBER)&&(bDrawFlagAbsorber))||
			((nType == CTMCRTH_BLCKNTYPE_METAL)&&(bDrawFlagMetal))||
			((nType == CTMCRTH_BLCKNTYPE_MAGNETIC)&&(bDrawFlagMagnetic)) )
		{
			if( (pX1 == NULL)||(pX2 == NULL)||(pX3 == NULL)||(pX4 == NULL)||(pY1 == NULL)||(pY2 == NULL)||(pY3 == NULL)||(pY4 == NULL) )
			{
				cError.PutErrorMessage("pointer topology polygon is NULL");
				RedrawWindow();
				return;
			};
			if( (nType == CTMCRTH_BLCKNTYPE_INPXLEFT)||(nType == CTMCRTH_BLCKNTYPE_INPXRIGHT)||(nType == CTMCRTH_BLCKNTYPE_INPYTOP)||(nType == CTMCRTH_BLCKNTYPE_INPYBOT) )
			{
				pdTopXinp[ jj ] = XtoGLWinCoord( (pX1[0]+pX2[0]+pX3[0]+pX4[0])/4. );
				pdTopYinp[ jj ] = YtoGLWinCoord( (pY1[0]+pY2[0]+pY3[0]+pY4[0])/4. );
				jj++;
			};
			for( j = 0; j < n; j++ )
			{
				if( ((pX1[j] < dXmax_1)&&(pX1[j] > dXmin_1)&&(pY1[j] < dYmax_1)&&(pY1[j] > dYmin_1))||
					((pX2[j] < dXmax_1)&&(pX2[j] > dXmin_1)&&(pY2[j] < dYmax_1)&&(pY2[j] > dYmin_1))||
					((pX3[j] < dXmax_1)&&(pX3[j] > dXmin_1)&&(pY3[j] < dYmax_1)&&(pY3[j] > dYmin_1))||
					((pX4[j] < dXmax_1)&&(pX4[j] > dXmin_1)&&(pY4[j] < dYmax_1)&&(pY4[j] > dYmin_1)))
				{
					pnTopType[ii] = pcBoundBlockTpl[i].GetnType();
					pdTopX1[ii] = XtoGLWinCoord(pX1[j]);
					pdTopX2[ii] = XtoGLWinCoord(pX2[j]);
					pdTopX3[ii] = XtoGLWinCoord(pX3[j]);
					pdTopX4[ii] = XtoGLWinCoord(pX4[j]);
					pdTopY1[ii] = YtoGLWinCoord(pY1[j]);
					pdTopY2[ii] = YtoGLWinCoord(pY2[j]);
					pdTopY3[ii] = YtoGLWinCoord(pY3[j]);
					pdTopY4[ii] = YtoGLWinCoord(pY4[j]);
					ii++;
				};
			};
		};
	}

	PrepareBoundTplSize();

	return;
}

void CFldViewView::OnViewProportionally() 
{
	// TODO: Add your command handler code here
	bProportionally = !bProportionally;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateViewProportionally(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bProportionally );	
	return;
}
			  // calculation number of polygon in topology geometry
void CFldViewView::CalcnTopPolygon()
{
	if( cError.IsError() ) return;
	if( (!bDrawLineFlagTopology)&&(!bDrawSurfaceFlagTopology) ) return;
	if( pcBoundBlockTpl == NULL ) return;

	int i, j, n, nType; 
	double *pX1, *pX2, *pX3, *pX4, *pY1, *pY2, *pY3, *pY4;
	double dXmin_1, dXmax_1, dYmin_1, dYmax_1;
	
	dXmin_1 = dXmin - 1.1*dDelta;
	dXmax_1 = dXmax + 1.1*dDelta;
	dYmin_1 = dYmin - 1.1*dDelta;
	dYmax_1 = dYmax + 1.1*dDelta;
	
	for( i = 0, nInputNum = 0, nTopPolygon = 0; i < nBoundBlockTpl; i++ )
	{
		pX1 = pcBoundBlockTpl[i].GetpdX1();
		pX2 = pcBoundBlockTpl[i].GetpdX2();
		pX3 = pcBoundBlockTpl[i].GetpdX3();
		pX4 = pcBoundBlockTpl[i].GetpdX4();
		pY1 = pcBoundBlockTpl[i].GetpdY1();
		pY2 = pcBoundBlockTpl[i].GetpdY2();
		pY3 = pcBoundBlockTpl[i].GetpdY3();
		pY4 = pcBoundBlockTpl[i].GetpdY4();
		n = pcBoundBlockTpl[i].GetnPoint();
		nType = pcBoundBlockTpl[i].GetnType();
		if( ((nType == CTMCRTH_BLCKNTYPE_EPS)&&(bDrawFlagEps))||
			(((nType == CTMCRTH_BLCKNTYPE_INPXLEFT)||(nType == CTMCRTH_BLCKNTYPE_INPXRIGHT)||(nType == CTMCRTH_BLCKNTYPE_INPYTOP)||(nType == CTMCRTH_BLCKNTYPE_INPYBOT))&&(bDrawFlagInput))||
			((nType == CTMCRTH_BLCKNTYPE_ABSORBER)&&(bDrawFlagAbsorber))||
			((nType == CTMCRTH_BLCKNTYPE_METAL)&&(bDrawFlagMetal))||
			((nType == CTMCRTH_BLCKNTYPE_MAGNETIC)&&(bDrawFlagMagnetic)) )
		{
			if( (nType == CTMCRTH_BLCKNTYPE_INPXLEFT)||(nType == CTMCRTH_BLCKNTYPE_INPXRIGHT)||(nType == CTMCRTH_BLCKNTYPE_INPYTOP)||(nType == CTMCRTH_BLCKNTYPE_INPYBOT) ) nInputNum++;
			if( (pX1 == NULL)||(pX2 == NULL)||(pX3 == NULL)||(pX4 == NULL)||(pY1 == NULL)||(pY2 == NULL)||(pY3 == NULL)||(pY4 == NULL) )
			{
				cError.PutErrorMessage("pointer topology polygon is NULL");
				RedrawWindow();
				return;
			};
			for( j = 0; j < n; j++ )
			{
				if( ((pX1[j] < dXmax_1)&&(pX1[j] > dXmin_1)&&(pY1[j] < dYmax_1)&&(pY1[j] > dYmin_1))||
					((pX2[j] < dXmax_1)&&(pX2[j] > dXmin_1)&&(pY2[j] < dYmax_1)&&(pY2[j] > dYmin_1))||
					((pX3[j] < dXmax_1)&&(pX3[j] > dXmin_1)&&(pY3[j] < dYmax_1)&&(pY3[j] > dYmin_1))||
					((pX4[j] < dXmax_1)&&(pX4[j] > dXmin_1)&&(pY4[j] < dYmax_1)&&(pY4[j] > dYmin_1)))
					nTopPolygon++;
			};
		};
	}

	return;
}

void CFldViewView::DrawTopology()
{
	if( cError.IsError() ) return;

	switch( n2D3DFlagTopology )
	{
		case TMCFIELDVIEW_2DDraw:
			DrawTopSurface2();
			DrawTopdLine2();
			DrawTopInputNum2();
			break;
		case TMCFIELDVIEW_3DDraw:
			DrawTopSurface();
			DrawTopdLine();
			DrawTopInputNum();
			break;
	};
	
	return;
}

void CFldViewView::DrawTopdLine2()
{
	if( cError.IsError() ) return;
	if( !bDrawLineFlagTopology ) return;
	if( (pdTopX1 == NULL)||(pdTopX2 == NULL)||(pdTopX3 == NULL)||(pdTopX4 == NULL)||
		(pdTopY1 == NULL)||(pdTopY2 == NULL)||(pdTopY3 == NULL)||(pdTopY4 == NULL)||
		(pnTopType == NULL)) return;

	int i;

	if( bDrawSurfaceFlagTopology )
	{
		glColor4f( 1.0f, 1.0f, 1.0f, (float)(0.8) );
		for( i = 0; i < nTopPolygon; i++ )
		{
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(0.03));
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(0.03));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(0.03));
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(0.03));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(0.03));
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(0.03));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(0.03));
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(0.03));
		    glEnd();
		};
	}
	else
	{
		for( i = 0; i < nTopPolygon; i++ )
		{
			SetColornType( pnTopType[i] );
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(0.03));
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(0.03));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(0.03));
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(0.03));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(0.03));
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(0.03));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(0.03));
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(0.03));
		    glEnd();
		};
	};
	return;
}

void CFldViewView::SetColornType(int nType)
{
	switch(nType)
	{
		case CTMCRTH_BLCKNTYPE_EPS:
			SetColor(scColorEps, nBlendTopology);
			break;
		case CTMCRTH_BLCKNTYPE_MAGNETIC:
			SetColor(scColorMag, nBlendTopology);
			break;
		case CTMCRTH_BLCKNTYPE_METAL:
			SetColor(scColorMet, nBlendTopology);
			break;
		case CTMCRTH_BLCKNTYPE_ABSORBER:
			SetColor(scColorAbs, nBlendTopology);
			break;
		case CTMCRTH_BLCKNTYPE_INPXLEFT:
		case CTMCRTH_BLCKNTYPE_INPXRIGHT:
		case CTMCRTH_BLCKNTYPE_INPYTOP:
		case CTMCRTH_BLCKNTYPE_INPYBOT:
			SetColor(scColorInp, nBlendTopology);
			break;
		default:
			SetColor(scColorEps, nBlendTopology);
			break;
	};
	return;
}

void CFldViewView::DrawTopSurface2()
{
	if( cError.IsError() ) return;
	if( !bDrawSurfaceFlagTopology ) return;
	if( (pdTopX1 == NULL)||(pdTopX2 == NULL)||(pdTopX3 == NULL)||(pdTopX4 == NULL)||
		(pdTopY1 == NULL)||(pdTopY2 == NULL)||(pdTopY3 == NULL)||(pdTopY4 == NULL)||
		(pnTopType == NULL)) return;

	int i;

	for( i = 0; i < nTopPolygon; i++ )
	{
		SetColornType( pnTopType[i] );
		glBegin( GL_POLYGON  );
		    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(0.02));
		    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(0.02));
		    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(0.02));
		    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(0.02));
	    glEnd();
	};

	return;
}

void CFldViewView::DrawTopdLine()
{
	if( cError.IsError() ) return;
	if( !bDrawLineFlagTopology ) return;
	if( (pdTopX1 == NULL)||(pdTopX2 == NULL)||(pdTopX3 == NULL)||(pdTopX4 == NULL)||
		(pdTopY1 == NULL)||(pdTopY2 == NULL)||(pdTopY3 == NULL)||(pdTopY4 == NULL)||
		(pnTopType == NULL)) return;

	int i;

	if( bDrawSurfaceFlagTopology )
	{
		glColor4f( 1.0f, 1.0f, 1.0f, (float)(0.8) );
		for( i = 0; i < nTopPolygon; i++ )
		{
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(dZtopol));
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(dZtopol));
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(dZtopol));
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(dZtopol));
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(dZtopol));
		    glEnd();
		};
		for( i = 0; i < nTopPolygon; i++ )
		{
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(-dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(-dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(-dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(-dZtopol));
		    glEnd();
		};
		for( i = 0; i < nTopPolygon; i++ )
		{
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)( dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)( dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)( dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)( dZtopol));
		    glEnd();
		};
	}
	else
	{
		for( i = 0; i < nTopPolygon; i++ )
		{
			SetColornType( pnTopType[i] );
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(dZtopol));
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(dZtopol));
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(dZtopol));
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(dZtopol));
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(dZtopol));
		    glEnd();
		};
		for( i = 0; i < nTopPolygon; i++ )
		{
			SetColornType( pnTopType[i] );
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(-dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(-dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(-dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(-dZtopol));
		    glEnd();
		};
		for( i = 0; i < nTopPolygon; i++ )
		{
			SetColornType( pnTopType[i] );
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)( dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)( dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)( dZtopol));
		    glEnd();
			glBegin( GL_LINES  );
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(-dZtopol));
			    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)( dZtopol));
		    glEnd();
		};
	};
	return;
}

void CFldViewView::DrawTopSurface()
{
	if( cError.IsError() ) return;
	if( !bDrawSurfaceFlagTopology ) return;
	if( (pdTopX1 == NULL)||(pdTopX2 == NULL)||(pdTopX3 == NULL)||(pdTopX4 == NULL)||
		(pdTopY1 == NULL)||(pdTopY2 == NULL)||(pdTopY3 == NULL)||(pdTopY4 == NULL)||
		(pnTopType == NULL)) return;

	int i;

	for( i = 0; i < nTopPolygon; i++ )
	{
		SetColornType( pnTopType[i] );
		glBegin( GL_POLYGON  );
		    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(dZtopol));
		    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(dZtopol));
		    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(dZtopol));
		    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(dZtopol));
	    glEnd();
		glBegin( GL_POLYGON  );
		    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(-dZtopol));
		    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(-dZtopol));
		    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(-dZtopol));
		    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(-dZtopol));
	    glEnd();
		glBegin( GL_POLYGON  );
		    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(-dZtopol));
		    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)( dZtopol));
		    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)( dZtopol));
		    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(-dZtopol));
	    glEnd();
		glBegin( GL_POLYGON  );
		    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)(-dZtopol));
		    glVertex3f( (float)(pdTopX2[i]), (float)(pdTopY2[i]), (float)( dZtopol));
		    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)( dZtopol));
		    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(-dZtopol));
	    glEnd();
		glBegin( GL_POLYGON  );
		    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)(-dZtopol));
		    glVertex3f( (float)(pdTopX3[i]), (float)(pdTopY3[i]), (float)( dZtopol));
		    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)( dZtopol));
		    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(-dZtopol));
	    glEnd();
		glBegin( GL_POLYGON  );
		    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)(-dZtopol));
		    glVertex3f( (float)(pdTopX4[i]), (float)(pdTopY4[i]), (float)( dZtopol));
		    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)( dZtopol));
		    glVertex3f( (float)(pdTopX1[i]), (float)(pdTopY1[i]), (float)(-dZtopol));
	    glEnd();
	};

	return;
}

void CFldViewView::OnFlagdrawlinetop() 
{
	// TODO: Add your command handler code here
	bDrawLineFlagTopology = !bDrawLineFlagTopology;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateFlagdrawlinetop(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawLineFlagTopology );	
	return;
}

void CFldViewView::OnFlagdrawsurfacetop() 
{
	// TODO: Add your command handler code here
	bDrawSurfaceFlagTopology = !bDrawSurfaceFlagTopology;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateFlagdrawsurfacetop(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawSurfaceFlagTopology );	
	return;
}

void CFldViewView::OnDimension23switchtop() 
{
	// TODO: Add your command handler code here
	n2D3DFlagTopology = !n2D3DFlagTopology;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateDimension23switchtop(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( n2D3DFlagTopology );	
	pCmdUI->Enable(bDrawSurfaceFlagTopology||bDrawLineFlagTopology);
	return;
}

void CFldViewView::OnViewTopologyBoundaryEps() 
{
	// TODO: Add your command handler code here
	bDrawFlagEps = !bDrawFlagEps;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateViewTopologyBoundaryEps(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawFlagEps );	
	pCmdUI->Enable(bDrawSurfaceFlagTopology||bDrawLineFlagTopology);
	return;
}

void CFldViewView::OnViewTopologyBoundaryMetal() 
{
	// TODO: Add your command handler code here
	bDrawFlagMetal = !bDrawFlagMetal;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateViewTopologyBoundaryMetal(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawFlagMetal );	
	pCmdUI->Enable(bDrawSurfaceFlagTopology||bDrawLineFlagTopology);
	return;
}

void CFldViewView::OnViewTopologyBoundaryMagnetic() 
{
	// TODO: Add your command handler code here
	bDrawFlagMagnetic = !bDrawFlagMagnetic;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateViewTopologyBoundaryMagnetic(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawFlagMagnetic );	
	pCmdUI->Enable(bDrawSurfaceFlagTopology||bDrawLineFlagTopology);
	return;
}

void CFldViewView::OnViewTopologyBoundaryInput() 
{
	// TODO: Add your command handler code here
	bDrawFlagInput = !bDrawFlagInput;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateViewTopologyBoundaryInput(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawFlagInput );	
	pCmdUI->Enable(bDrawSurfaceFlagTopology||bDrawLineFlagTopology);
	return;
}

void CFldViewView::OnViewTopologyBoundaryAbsorber() 
{
	// TODO: Add your command handler code here
	bDrawFlagAbsorber = !bDrawFlagAbsorber;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateViewTopologyBoundaryAbsorber(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawFlagAbsorber );	
	pCmdUI->Enable(bDrawSurfaceFlagTopology||bDrawLineFlagTopology);
	return;
}

void CFldViewView::PrepareBoundTplSize()
{
	nTopSize = 0;

	if( pcTopSize != NULL ) delete []pcTopSize;
	pcTopSize = NULL;

	if( cError.IsError() ) return;
	if( (!bDrawLineFlagTopology)&&(!bDrawSurfaceFlagTopology) ) return;
	if( (!bTopologySizeFlagLinkList)&&(!bTopologySizeFlag) ) return;
	if( pcBoundBlockTpl == NULL ) return;
	if( nTopPolygon < 1 ) return;
	if( nBoundBlockTpl < 1 ) return;

	int i, j, n, ii; 
	double dX1, dX2, dY1, dY2, dSize;
	double dXmin_1, dXmax_1, dYmin_1, dYmax_1;
	CTmcTopSize cTopSize1, cTopSize2, *pcTopSize1;
	
	dXmin_1 = dXmin - 1.1*dDelta;
	dXmax_1 = dXmax + 1.1*dDelta;
	dYmin_1 = dYmin - 1.1*dDelta;
	dYmax_1 = dYmax + 1.1*dDelta;

		// calculation nTopsize
	for( i = 0, ii = 0; i < nBoundBlockTpl; i++ )
	{
		if( bTopologySizeFlagLinkList )
		{
			cTopSize1 = pcBoundBlockTpl[i].GetcTopSizeX0();
			dX1   = cTopSize1.GetdX1();
			dX2   = cTopSize1.GetdX2();
			dY1   = cTopSize1.GetdY1();
			dY2   = cTopSize1.GetdY2();
			dSize = cTopSize1.GetdSize();
			if( ((dX1 < dXmax_1)&&(dX1 > dXmin_1)&&(dY1 < dYmax_1)&&(dY1 > dYmin_1))&&
				((dX2 < dXmax_1)&&(dX2 > dXmin_1)&&(dY2 < dYmax_1)&&(dY2 > dYmin_1))&&
				(dSize > 2*dDelta))
				{
					ii++;
				};
			cTopSize2 = pcBoundBlockTpl[i].GetcTopSizeY0();
			dX1   = cTopSize2.GetdX1();
			dX2   = cTopSize2.GetdX2();
			dY1   = cTopSize2.GetdY1();
			dY2   = cTopSize2.GetdY2();
			dSize = cTopSize2.GetdSize();
			if( ((dX1 < dXmax_1)&&(dX1 > dXmin_1)&&(dY1 < dYmax_1)&&(dY1 > dYmin_1))&&
				((dX2 < dXmax_1)&&(dX2 > dXmin_1)&&(dY2 < dYmax_1)&&(dY2 > dYmin_1))&&
				(dSize > 2*dDelta))
				{
					ii++;
				};
		};
		if( bTopologySizeFlag )
		{
			pcTopSize1 = pcBoundBlockTpl[i].GetpcTopSize();
			n = pcBoundBlockTpl[i].GetnTopSize();
			if( (pcTopSize1 == NULL)||( n < 1 ) )
			{
				cError.PutErrorMessage("pointer size of topology is NULL");
				RedrawWindow();
				return;
			};
			for( j = 0; j < n; j++ )
			{
				dX1   = pcTopSize1[j].GetdX1();
				dX2   = pcTopSize1[j].GetdX2();
				dY1   = pcTopSize1[j].GetdY1();
				dY2   = pcTopSize1[j].GetdY2();
				dSize = pcTopSize1[j].GetdSize();
				if( ((dX1 < dXmax_1)&&(dX1 > dXmin_1)&&(dY1 < dYmax_1)&&(dY1 > dYmin_1))&&
					((dX2 < dXmax_1)&&(dX2 > dXmin_1)&&(dY2 < dYmax_1)&&(dY2 > dYmin_1))&&
					(dSize > 2*dDelta))
					{
						ii++;
					};
			};
		};
	};

	if( ii < 1 ) return;

	nTopSize = ii;
	pcTopSize = new CTmcTopSize[nTopSize];
	if( pcTopSize == NULL )
	{
		nTopSize = 0;
		cError.PutErrorMessage("memory allocation error for topology size array");
		RedrawWindow();
		return;
	};

	for( i = 0, ii = 0; i < nBoundBlockTpl; i++ )
	{
		if( bTopologySizeFlagLinkList )
		{
			cTopSize1 = pcBoundBlockTpl[i].GetcTopSizeX0();
			dX1   = cTopSize1.GetdX1();
			dX2   = cTopSize1.GetdX2();
			dY1   = cTopSize1.GetdY1();
			dY2   = cTopSize1.GetdY2();
			dSize = cTopSize1.GetdSize();
			if( ((dX1 < dXmax_1)&&(dX1 > dXmin_1)&&(dY1 < dYmax_1)&&(dY1 > dYmin_1))&&
				((dX2 < dXmax_1)&&(dX2 > dXmin_1)&&(dY2 < dYmax_1)&&(dY2 > dYmin_1))&&
				(dSize > 2*dDelta))
				{
					pcTopSize[ii] = cTopSize1;
					ii++;
				};
			cTopSize2 = pcBoundBlockTpl[i].GetcTopSizeY0();
			dX1   = cTopSize2.GetdX1();
			dX2   = cTopSize2.GetdX2();
			dY1   = cTopSize2.GetdY1();
			dY2   = cTopSize2.GetdY2();
			dSize = cTopSize2.GetdSize();
			if( ((dX1 < dXmax_1)&&(dX1 > dXmin_1)&&(dY1 < dYmax_1)&&(dY1 > dYmin_1))&&
				((dX2 < dXmax_1)&&(dX2 > dXmin_1)&&(dY2 < dYmax_1)&&(dY2 > dYmin_1))&&
				(dSize > 2*dDelta))
				{
					pcTopSize[ii] = cTopSize2;
					ii++;
				};
		};
		if( bTopologySizeFlag )
		{
			pcTopSize1 = pcBoundBlockTpl[i].GetpcTopSize();
			n = pcBoundBlockTpl[i].GetnTopSize();
			if( (pcTopSize1 == NULL)||( n < 1 ) )
			{
				cError.PutErrorMessage("pointer size of topology is NULL");
				RedrawWindow();
				return;
			};
			for( j = 0; j < n; j++ )
			{
				dX1   = pcTopSize1[j].GetdX1();
				dX2   = pcTopSize1[j].GetdX2();
				dY1   = pcTopSize1[j].GetdY1();
				dY2   = pcTopSize1[j].GetdY2();
				dSize = pcTopSize1[j].GetdSize();
				if( ((dX1 < dXmax_1)&&(dX1 > dXmin_1)&&(dY1 < dYmax_1)&&(dY1 > dYmin_1))&&
					((dX2 < dXmax_1)&&(dX2 > dXmin_1)&&(dY2 < dYmax_1)&&(dY2 > dYmin_1))&&
					(dSize > 2*dDelta))
					{
						pcTopSize[ii] = pcTopSize1[j];
						ii++;
					};
			};
		};
	};
	
	PrepareBoundTplSizeToGLCoord();
	return;
}

void CFldViewView::PrepareBoundTplSizeToGLCoord()
{
	if( cError.IsError() ) return;
	if( (!bDrawLineFlagTopology)&&(!bDrawSurfaceFlagTopology) ) return;
	if( (!bTopologySizeFlagLinkList)&&(!bTopologySizeFlag) ) return;
	if( nTopSize < 1 ) return;
	if( pcTopSize == NULL ) return;

	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	int i; 
	double dX1, dX2, dY1, dY2, dSize;

	for( i = 0; i < nTopSize; i++ )
	{
		dX1   = pcTopSize[i].GetdX1();
		dX2   = pcTopSize[i].GetdX2();
		dY1   = pcTopSize[i].GetdY1();
		dY2   = pcTopSize[i].GetdY2();
		dSize = pcTopSize[i].GetdSize();
		pcTopSize[i].SetdX1( XtoGLWinCoord( dX1 ) );
		pcTopSize[i].SetdX2( XtoGLWinCoord( dX2 ) );
		pcTopSize[i].SetdY1( YtoGLWinCoord( dY1 ) );
		pcTopSize[i].SetdY2( YtoGLWinCoord( dY2 ) );
		pcTopSize[i].SetdSize( dSize/pDoc->GetdLongUnit() );
	};

	return;
}

void CFldViewView::DrawSize()
{
	if( cError.IsError() ) return;

	switch( n2D3DFlagTopology )
	{
		case TMCFIELDVIEW_2DDraw:
			DrawSize2();
			break;
		case TMCFIELDVIEW_3DDraw:
			DrawSize3();
			break;
	};
	
	return;
}

void CFldViewView::DrawSize2()
{
	if( cError.IsError() ) return;
	if( (!bDrawLineFlagTopology)&&(!bDrawSurfaceFlagTopology) ) return;
	if( (!bTopologySizeFlagLinkList)&&(!bTopologySizeFlag) ) return;
	if( nTopSize < 1 ) return;
	if( pcTopSize == NULL ) return;

	int i; 
	double dX1, dX2, dY1, dY2, dZ1, dSize, r, rX, rY;
	CTmcGLText glText1;
	CString csBuf;

	SetColor( scAxiesColor );
	dZ1 = dZtopol;
	for( i = 0; i < nTopSize; i++ )
	{
		dX1   = pcTopSize[i].GetdX1();
		dX2   = pcTopSize[i].GetdX2();
		dY1   = pcTopSize[i].GetdY1();
		dY2   = pcTopSize[i].GetdY2();
		dSize = pcTopSize[i].GetdSize();
		rX = dX2 - dX1;
		rY = dY2 - dY1;
		r = sqrt( rX*rX + rY*rY );
		if( r < FLT_MIN ) r = FLT_MIN;
		rX = 2*rX*dZtopolStrelk/r;
		rY = 2*rY*dZtopolStrelk/r;
		glBegin( GL_LINES  );
		glVertex3f( (float)( dX1 ), (float)( dY1 ), (float)(ZtoGLWinCoord( 0. )));
		glVertex3f( (float)( dX1 ), (float)( dY1 ), (float)( dZ1 + 3*dZtopolStrelk ));
		glEnd();
		glBegin( GL_LINES  );
		glVertex3f( (float)( dX2 ), (float)( dY2 ), (float)(ZtoGLWinCoord( 0. )));
		glVertex3f( (float)( dX2 ), (float)( dY2 ), (float)( dZ1 + 3*dZtopolStrelk ));
		glEnd();
		glBegin( GL_LINES  );
		glVertex3f( (float)( dX1 ), (float)( dY1 ), (float)( dZ1 ));
		glVertex3f( (float)( dX2 ), (float)( dY2 ), (float)( dZ1 ));
		glEnd();
		glBegin(GL_POLYGON);
		glVertex3f( (float)( dX1 ), (float)( dY1 ), (float)( dZ1 ));
		glVertex3f( (float)( dX1 + rX ), (float)( dY1 + rY ), (float)( dZ1 + dZtopolStrelk ));
		glVertex3f( (float)( dX1 + rX ), (float)( dY1 + rY ), (float)( dZ1 - dZtopolStrelk ));
		glEnd();
		glBegin(GL_POLYGON);
		glVertex3f( (float)( dX2 ), (float)( dY2 ), (float)( dZ1 ));
		glVertex3f( (float)( dX2 - rX ), (float)( dY2 - rY ), (float)( dZ1 + dZtopolStrelk ));
		glVertex3f( (float)( dX2 - rX ), (float)( dY2 - rY ), (float)( dZ1 - dZtopolStrelk ));
		glEnd();
		glText1.SetColor( scAxiesColor );
		glText1.SetFont( &lfAxiesFont );
		csBuf.Format( "%lg", dSize );
		glText1.DrawString( csBuf, (dX1 + dX2)/2., (dY1 + dY2)/2., dZ1 );
	};

	return;
}

void CFldViewView::DrawSize3()
{
	if( cError.IsError() ) return;
	if( (!bDrawLineFlagTopology)&&(!bDrawSurfaceFlagTopology) ) return;
	if( (!bTopologySizeFlagLinkList)&&(!bTopologySizeFlag) ) return;
	if( nTopSize < 1 ) return;
	if( pcTopSize == NULL ) return;

	int i; 
	double dX1, dX2, dY1, dY2, dZ1, dSize, r, rX, rY;
	CTmcGLText glText1;
	CString csBuf;

	SetColor( scAxiesColor );
	dZ1 = 2*dZtopol;
	for( i = 0; i < nTopSize; i++ )
	{
		dX1   = pcTopSize[i].GetdX1();
		dX2   = pcTopSize[i].GetdX2();
		dY1   = pcTopSize[i].GetdY1();
		dY2   = pcTopSize[i].GetdY2();
		dSize = pcTopSize[i].GetdSize();
		rX = dX2 - dX1;
		rY = dY2 - dY1;
		r = sqrt( rX*rX + rY*rY );
		if( r < FLT_MIN ) r = FLT_MIN;
		rX = 2*rX*dZtopolStrelk/r;
		rY = 2*rY*dZtopolStrelk/r;
		glBegin( GL_LINES  );
		glVertex3f( (float)( dX1 ), (float)( dY1 ), (float)(ZtoGLWinCoord( 0. )));
		glVertex3f( (float)( dX1 ), (float)( dY1 ), (float)( dZ1 + 3*dZtopolStrelk ));
		glEnd();
		glBegin( GL_LINES  );
		glVertex3f( (float)( dX2 ), (float)( dY2 ), (float)(ZtoGLWinCoord( 0. )));
		glVertex3f( (float)( dX2 ), (float)( dY2 ), (float)( dZ1 + 3*dZtopolStrelk ));
		glEnd();
		glBegin( GL_LINES  );
		glVertex3f( (float)( dX1 ), (float)( dY1 ), (float)( dZ1 ));
		glVertex3f( (float)( dX2 ), (float)( dY2 ), (float)( dZ1 ));
		glEnd();
		glBegin(GL_POLYGON);
		glVertex3f( (float)( dX1 ), (float)( dY1 ), (float)( dZ1 ));
		glVertex3f( (float)( dX1 + rX ), (float)( dY1 + rY ), (float)( dZ1 + dZtopolStrelk ));
		glVertex3f( (float)( dX1 + rX ), (float)( dY1 + rY ), (float)( dZ1 - dZtopolStrelk ));
		glEnd();
		glBegin(GL_POLYGON);
		glVertex3f( (float)( dX2 ), (float)( dY2 ), (float)( dZ1 ));
		glVertex3f( (float)( dX2 - rX ), (float)( dY2 - rY ), (float)( dZ1 + dZtopolStrelk ));
		glVertex3f( (float)( dX2 - rX ), (float)( dY2 - rY ), (float)( dZ1 - dZtopolStrelk ));
		glEnd();
		glText1.SetColor( scAxiesColor );
		glText1.SetFont( &lfAxiesFont );
		csBuf.Format( "%lg", dSize );
		glText1.DrawString( csBuf, (dX1 + dX2)/2., (dY1 + dY2)/2., dZ1 );
	};

	return;
}

void CFldViewView::OnViewFieldSize() 
{
	// TODO: Add your command handler code here
	bDrawFlagAxiesZFieldSize = !bDrawFlagAxiesZFieldSize;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateViewFieldSize(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawFlagAxiesZFieldSize );	
	return;
}



void CFldViewView::OnViewEpsSize() 
{
	// TODO: Add your command handler code here
	bDrawFlagAxiesZFieldSize_Eps = !bDrawFlagAxiesZFieldSize_Eps;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateViewEpsSize(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawFlagAxiesZFieldSize_Eps );	
	return;
}

void CFldViewView::OnViewTopologyDimensionsAlonganxaxies() 
{
	// TODO: Add your command handler code here
	bDrawFlagAxiesXSize = !bDrawFlagAxiesXSize;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateViewTopologyDimensionsAlonganxaxies(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawFlagAxiesXSize );	
	return;
}

void CFldViewView::OnViewTopologyDimensionsAlonganyaxies() 
{
	// TODO: Add your command handler code here
	bDrawFlagAxiesYSize = !bDrawFlagAxiesYSize;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateViewTopologyDimensionsAlonganyaxies(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawFlagAxiesYSize );	
	return;
}

void CFldViewView::OnViewTopologySizeBlock() 
{
	// TODO: Add your command handler code here
	bTopologySizeFlag = !bTopologySizeFlag;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateViewTopologySizeBlock(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bTopologySizeFlag );	
	pCmdUI->Enable(bDrawSurfaceFlagTopology||bDrawLineFlagTopology);
	return;
}

void CFldViewView::OnViewTopologySizeLinklist() 
{
	// TODO: Add your command handler code here
	bTopologySizeFlagLinkList = !bTopologySizeFlagLinkList;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateViewTopologySizeLinklist(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bTopologySizeFlagLinkList );
	pCmdUI->Enable(bDrawSurfaceFlagTopology||bDrawLineFlagTopology);
	return;
}

void CFldViewView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if( !pDoc->IsFileRead() )
	{
		ReadData();
		if( !cError.IsError() ) 
		{
			DrawScene();
			pDoc->SetSynchronizationFlag();
		}
		else 
		{
			RedrawWindow();
			pDoc->SetSynchronizationFlag();
			Sleep(1000);
		};
	}
	return;
}

void CFldViewView::DrawTopInputNum2()
{
	if( cError.IsError() ) return;
	if( !bDrawFlagInput ) return;
	if( nInputNum < 1 ) return;
	if( (pdTopXinp == NULL)||(pdTopYinp == NULL) ) return;

	int i;
	CTmcGLText glText1;
	CString csBuf;

	SetColor( scAxiesColor );
	for( i = 0; i < nInputNum; i++ )
	{
		glText1.SetColor( scAxiesColor );
		glText1.SetFont( &lfAxiesFont );
		csBuf.Format( "Inp %d", i+1 );
		glText1.DrawString( csBuf, pdTopXinp[i], pdTopYinp[i], 0.02 );
	};

	return;
}

void CFldViewView::DrawTopInputNum()
{
	if( cError.IsError() ) return;
	if( !bDrawFlagInput ) return;
	if( nInputNum < 1 ) return;
	if( (pdTopXinp == NULL)||(pdTopYinp == NULL) ) return;

	int i;
	CTmcGLText glText1;
	CString csBuf;

	SetColor( scAxiesColor );
	for( i = 0; i < nInputNum; i++ )
	{
		glText1.SetColor( scAxiesColor );
		glText1.SetFont( &lfAxiesFont );
		csBuf.Format( "Inp %d", i+1 );
		glText1.DrawString( csBuf, pdTopXinp[i], pdTopYinp[i], dZtopol );
	};

	return;
}

void CFldViewView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	// TODO: Add your specialized code here and/or call the base class
	if( pDC->IsPrinting() ) 
	{
		pDC->SetBkColor( scBackgoundColor );
	}
	else 
	{
		pDC->SetBkColor( scBackgoundColor );
	};

	CScrollView::OnPrepareDC(pDC, pInfo);
}


void CFldViewView::VTextOut( CDC *pDC, int nX, int nY, CString cText)
{
	CFldViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
    CFont      fontText;
	COLORREF cOldColor;

	if( lfAxiesFont.lfHeight == 0 )
	{
		TEXTMETRIC Metrics;
		pDC->GetTextMetrics( &Metrics );
		lfAxiesFont.lfHeight = Metrics.tmHeight/20;
	};
	
	(lfAxiesFont.lfHeight)*=20;
	if( fontText.CreateFontIndirect( &(lfAxiesFont) ) )
	{
	    CFont* pOldFont = (CFont*) pDC->SelectObject(&fontText);
		cOldColor = pDC->GetTextColor();
		pDC->SetTextColor( scAxiesColor );
		pDC->TextOut( nX, nY, cText);
	    pDC->SelectObject(pOldFont);
		pDC->SetTextColor( cOldColor );
	}
	else
	{
		pDC->TextOut( nX, nY, cText);
	}
	
	(lfAxiesFont.lfHeight)/=20;
	return;
}

void CFldViewView::OnFieldL()
{
	// TODO: Add your command handler code here
	if(pcColorLevelDialog->GetSafeHwnd() == 0)
	{
		pcColorLevelDialog->Create();
	};
	return;
}

LRESULT CFldViewView::OnCloseColorLevelDialog( WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your command handler code here
	pcColorLevelDialog->DestroyWindow();
	return 0L;
}




COLORREF * CFldViewView::GetpscSurfaceColor(void)
{
	return pscSurfaceColor;
}

BOOL * CFldViewView::GetbHightColorRezolution(void)
{
	return &bHightColorRezolution;
}

void CFldViewView::OnFieldValue()
{
	// TODO: Add your command handler code here
	bDrawFieldValueFlag = !bDrawFieldValueFlag;
	return;
}

void CFldViewView::OnUpdateFieldValue(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawFieldValueFlag );	
//	pCmdUI->Enable(bDrawSurfaceFlag||bDrawLineFlag);
	return;
}

int CFldViewView::GetNumberElement(double dX, double dY)
{
	if( cError.IsError() )	return 0;
	if( pdSurface == NULL )	return 0;
	if( dDelta < FLT_MIN )	return 0;

	return ((int)((dX - dXmin)/dDelta)) + nX*((int)((dY - dYmin)/dDelta));
}

void CFldViewView::DrawFieldCursore()
{
	if( cError.IsError() ) return;
	if( pdSurface  == NULL ) return;
	if( pdSurfaceX == NULL ) return;
	if( pdSurfaceY == NULL ) return;

	if( !bDrawFieldValueFlag ) return;

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue1 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue1]), (float)(pdSurface[nDrawFieldValue1]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue2] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(pdSurface[nDrawFieldValue2]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(pdSurface[nDrawFieldValue5]+0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue1 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue1]), (float)(pdSurface[nDrawFieldValue1]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue2] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(pdSurface[nDrawFieldValue2]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(pdSurface[nDrawFieldValue5]-0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue1 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue1]), (float)(pdSurface[nDrawFieldValue1]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue2] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(pdSurface[nDrawFieldValue2]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue2] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(pdSurface[nDrawFieldValue2]-0.01));
	SetColor_Cursor( pdSurface[ nDrawFieldValue1 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue1]), (float)(pdSurface[nDrawFieldValue1]-0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue2 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(pdSurface[nDrawFieldValue2]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue3] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(pdSurface[nDrawFieldValue3]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(pdSurface[nDrawFieldValue5]+0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue2 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(pdSurface[nDrawFieldValue2]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue3] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(pdSurface[nDrawFieldValue3]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(pdSurface[nDrawFieldValue5]-0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue2 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(pdSurface[nDrawFieldValue2]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue3] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(pdSurface[nDrawFieldValue3]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue3] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(pdSurface[nDrawFieldValue3]-0.01));
	SetColor_Cursor( pdSurface[ nDrawFieldValue2 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(pdSurface[nDrawFieldValue2]-0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue3 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(pdSurface[nDrawFieldValue3]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue4] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(pdSurface[nDrawFieldValue4]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(pdSurface[nDrawFieldValue5]+0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue3 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(pdSurface[nDrawFieldValue3]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue4] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(pdSurface[nDrawFieldValue4]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(pdSurface[nDrawFieldValue5]-0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue3 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(pdSurface[nDrawFieldValue3]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue4] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(pdSurface[nDrawFieldValue4]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue4] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(pdSurface[nDrawFieldValue4]-0.01));
	SetColor_Cursor( pdSurface[ nDrawFieldValue3 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(pdSurface[nDrawFieldValue3]-0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue4 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(pdSurface[nDrawFieldValue3]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue1] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(pdSurface[nDrawFieldValue4]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(pdSurface[nDrawFieldValue5]+0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue4 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(pdSurface[nDrawFieldValue4]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue1] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue1]), (float)(pdSurface[nDrawFieldValue1]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(pdSurface[nDrawFieldValue5]-0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue4 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(pdSurface[nDrawFieldValue4]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue1] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue1]), (float)(pdSurface[nDrawFieldValue1]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue1] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue1]), (float)(pdSurface[nDrawFieldValue1]-0.01));
	SetColor_Cursor( pdSurface[ nDrawFieldValue4 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(pdSurface[nDrawFieldValue4]-0.01));
    glEnd();

	return;
}

void CFldViewView::SetColor_Cursor( double z )
{
	int i;
	double n_r, n_g, n_b;
	double n_dr, n_dg, n_db;
	int n1_r, n1_g, n1_b;
	int n2_r, n2_g, n2_b;

	if( z >  1. ) z =  1.0001;
	if( z < -1. ) z = -1.0001;

//	i = (int)(( fabs( z - (dZmax+dZmin)/2 )/((dZmax-dZmin)/2)*20 -0.5));

	if( bHightColorRezolution )
	{
		n1_r = GetRValue(pscSurfaceColor[0]);
		n1_g = GetGValue(pscSurfaceColor[0]);
		n1_b = GetBValue(pscSurfaceColor[0]);
		
		n2_r = GetRValue(pscSurfaceColor[19]);
		n2_g = GetGValue(pscSurfaceColor[19]);
		n2_b = GetBValue(pscSurfaceColor[19]);

		n_dr = (n2_r - n1_r)/(256.-1);
		n_dg = (n2_g - n1_g)/(256.-1);
		n_db = (n2_b - n1_b)/(256.-1);

		n_r = n1_r + fabs( z )*256*n_dr;
		n_g = n1_g + fabs( z )*256*n_dg;
		n_b = n1_b + fabs( z )*256*n_db;

		glColor4f( 1-((float)(n_r))/256,
					1-((float)(n_g))/256,
					1-((float)(n_b))/256,
					(float)(nBlend/100.) );
	}
	else
	{
		i = (int)(( fabs( z ) )*19 );
		glColor4f( 1-((float)(GetRValue(pscSurfaceColor[i])))/256,
					1-((float)(GetGValue(pscSurfaceColor[i])))/256,
					1-((float)(GetBValue(pscSurfaceColor[i])))/256,
					(float)(nBlend/100.) );
	};

	return;
}


void CFldViewView::DrawFieldCursore2()
{
	if( cError.IsError() ) return;
	if( pdSurface  == NULL ) return;
	if( pdSurfaceX == NULL ) return;
	if( pdSurfaceY == NULL ) return;

	if( !bDrawFieldValueFlag ) return;

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue1 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue1]), (float)(0*pdSurface[nDrawFieldValue1]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue2] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(0*pdSurface[nDrawFieldValue2]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(0*pdSurface[nDrawFieldValue5]+0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue1 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue1]), (float)(0*pdSurface[nDrawFieldValue1]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue2] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(0*pdSurface[nDrawFieldValue2]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(0*pdSurface[nDrawFieldValue5]-0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue1 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue1]), (float)(0*pdSurface[nDrawFieldValue1]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue2] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(0*pdSurface[nDrawFieldValue2]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue2] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(0*pdSurface[nDrawFieldValue2]-0.01));
	SetColor_Cursor( pdSurface[ nDrawFieldValue1 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue1]), (float)(0*pdSurface[nDrawFieldValue1]-0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue2 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(0*pdSurface[nDrawFieldValue2]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue3] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(0*pdSurface[nDrawFieldValue3]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(0*pdSurface[nDrawFieldValue5]+0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue2 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(0*pdSurface[nDrawFieldValue2]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue3] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(0*pdSurface[nDrawFieldValue3]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(0*pdSurface[nDrawFieldValue5]-0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue2 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(0*pdSurface[nDrawFieldValue2]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue3] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(0*pdSurface[nDrawFieldValue3]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue3] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(0*pdSurface[nDrawFieldValue3]-0.01));
	SetColor_Cursor( pdSurface[ nDrawFieldValue2 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue2]), (float)(pdSurfaceY[nDrawFieldValue2]), (float)(0*pdSurface[nDrawFieldValue2]-0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue3 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(0*pdSurface[nDrawFieldValue3]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue4] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(0*pdSurface[nDrawFieldValue4]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(0*pdSurface[nDrawFieldValue5]+0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue3 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(0*pdSurface[nDrawFieldValue3]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue4] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(0*pdSurface[nDrawFieldValue4]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(0*pdSurface[nDrawFieldValue5]-0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue3 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(0*pdSurface[nDrawFieldValue3]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue4] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(0*pdSurface[nDrawFieldValue4]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue4] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(0*pdSurface[nDrawFieldValue4]-0.01));
	SetColor_Cursor( pdSurface[ nDrawFieldValue3 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue3]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(0*pdSurface[nDrawFieldValue3]-0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue4 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue3]), (float)(0*pdSurface[nDrawFieldValue3]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue1] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(0*pdSurface[nDrawFieldValue4]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(0*pdSurface[nDrawFieldValue5]+0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue4 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(0*pdSurface[nDrawFieldValue4]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue1] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue1]), (float)(0*pdSurface[nDrawFieldValue1]-0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue5] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue5]), (float)(pdSurfaceY[nDrawFieldValue5]), (float)(0*pdSurface[nDrawFieldValue5]-0.01));
    glEnd();

	glBegin(GL_POLYGON);
	SetColor_Cursor( pdSurface[ nDrawFieldValue4 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(0*pdSurface[nDrawFieldValue4]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue1] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue1]), (float)(0*pdSurface[nDrawFieldValue1]+0.01));
	SetColor_Cursor( pdSurface[nDrawFieldValue1] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue1]), (float)(pdSurfaceY[nDrawFieldValue1]), (float)(0*pdSurface[nDrawFieldValue1]-0.01));
	SetColor_Cursor( pdSurface[ nDrawFieldValue4 ] );
    glVertex3f( (float)(pdSurfaceX[nDrawFieldValue4]), (float)(pdSurfaceY[nDrawFieldValue4]), (float)(0*pdSurface[nDrawFieldValue4]-0.01));
    glEnd();

	return;
}

void CFldViewView::OnFieldModul()
{
	// TODO: Add your command handler code here
	bDrawFieldModulFlag = !bDrawFieldModulFlag;
	PrepareData();
	DrawScene();
	WriteIniFile();
	return;
}

void CFldViewView::OnUpdateFieldModul(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( bDrawFieldModulFlag );	
	return;
}
