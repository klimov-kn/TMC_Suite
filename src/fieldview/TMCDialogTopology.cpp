// TMCDialogTopology.cpp : implementation file
//

#include "stdafx.h"
#include "FldView.h"
#include "FldViewDoc.h"
#include "FldViewView.h"
#include "TMCDialogTopology.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTMCDialogTopology property page

IMPLEMENT_DYNCREATE(CTMCDialogTopology, CPropertyPage)

CTMCDialogTopology::CTMCDialogTopology() : CPropertyPage(CTMCDialogTopology::IDD)
{
	pFieldView = NULL;
	scColorEps = RGB(   0, 230, 230 );
	scColorMet = RGB( 210, 210, 210 );
	scColorMag = RGB( 120, 230, 200 );
	scColorInp = RGB( 255,   0, 255 );
	scColorAbs = RGB( 200, 100, 100 );
	nPixel = 15;
	//{{AFX_DATA_INIT(CTMCDialogTopology)
	m_DrawFlagEps = FALSE;
	m_DrawFlagInput = FALSE;
	m_DrawFlagMagnetic = FALSE;
	m_DrawFlagMetal = FALSE;
	m_DrawFlagSizeBlock = FALSE;
	m_DrawFlagSizeLinkList = FALSE;
	m_DrawFlagSurface = FALSE;
	m_DrawFlagLine = FALSE;
	m_23Dimention = -1;
	m_Blend = 0;
	m_DrawFlagAbsorber = FALSE;
	//}}AFX_DATA_INIT
}

CTMCDialogTopology::~CTMCDialogTopology()
{
}

void CTMCDialogTopology::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTMCDialogTopology)
	DDX_Check(pDX, IDC_DRAWFLAGEPS, m_DrawFlagEps);
	DDX_Check(pDX, IDC_DRAWFLAGInput, m_DrawFlagInput);
	DDX_Check(pDX, IDC_DRAWFLAGMAGNETIC, m_DrawFlagMagnetic);
	DDX_Check(pDX, IDC_DRAWFLAGMETAL, m_DrawFlagMetal);
	DDX_Check(pDX, IDC_DRAWFLAGSIZEBLOCK, m_DrawFlagSizeBlock);
	DDX_Check(pDX, IDC_DRAWFLAGSIZELINKLIST, m_DrawFlagSizeLinkList);
	DDX_Check(pDX, IDC_DRAWFLAGSURFACE, m_DrawFlagSurface);
	DDX_Check(pDX, IDC_DRAWFLAGLINE, m_DrawFlagLine);
	DDX_Radio(pDX, IDC_RADIO9, m_23Dimention);
	DDX_Text(pDX, IDC_BLENDEPS, m_Blend);
	DDX_Check(pDX, IDC_DRAWFLAGAbsorber, m_DrawFlagAbsorber);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTMCDialogTopology, CPropertyPage)
	//{{AFX_MSG_MAP(CTMCDialogTopology)
	ON_WM_PAINT()
	ON_BN_CLICKED(ID_TOPEPSCOLOREPS, OnTopepscoloreps)
	ON_BN_CLICKED(ID_TOPINPCOLORINPUT, OnTopinpcolorinput)
	ON_BN_CLICKED(ID_TOPMAGCOLORMAGNETIC, OnTopmagcolormagnetic)
	ON_BN_CLICKED(ID_TOPMETCOLORMETAL, OnTopmetcolormetal)
	ON_BN_CLICKED(ID_TOPMETCOLORDEFAULT, OnTopmetcolordefault)
	ON_BN_CLICKED(ID_TOPINPCOLORABSORBER, OnTopinpcolorabsorber)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTMCDialogTopology message handlers

void CTMCDialogTopology::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rectClient( 0, 0, 2*nPixel, nPixel);

	CWnd *pWnd = GetDlgItem(IDC_COLORTOPOLOGYEBS);
	CDC *pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, scColorEps );
	pWnd->ReleaseDC( pControlDC );
	
	pWnd = GetDlgItem(IDC_COLORTOPOLOGYMET);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, scColorMet );
	pWnd->ReleaseDC( pControlDC );
	
	pWnd = GetDlgItem(IDC_COLORTOPOLOGYMAG);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, scColorMag );
	pWnd->ReleaseDC( pControlDC );
	
	pWnd = GetDlgItem(IDC_COLORTOPOLOGYINP);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, scColorInp );
	pWnd->ReleaseDC( pControlDC );
	
	pWnd = GetDlgItem(IDC_COLORTOPOLOGYABS);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, scColorAbs );
	pWnd->ReleaseDC( pControlDC );
	
	// Do not call CPropertyPage::OnPaint() for painting messages
}

void CTMCDialogTopology::OnTopepscoloreps() 
{
	// TODO: Add your control notification handler code here
	CColorDialog dlg;
	dlg.m_cc.rgbResult = scColorEps;
	if( dlg.DoModal() == IDOK )
	{
		scColorEps = dlg.GetColor();
		RedrawWindow();	
	};
	return;
}

void CTMCDialogTopology::OnTopinpcolorinput() 
{
	// TODO: Add your control notification handler code here
	CColorDialog dlg;
	dlg.m_cc.rgbResult = scColorInp;
	if( dlg.DoModal() == IDOK )
	{
		scColorInp = dlg.GetColor();
		RedrawWindow();	
	};
	return;
}

void CTMCDialogTopology::OnTopmagcolormagnetic() 
{
	// TODO: Add your control notification handler code here
	CColorDialog dlg;
	dlg.m_cc.rgbResult = scColorMag;
	if( dlg.DoModal() == IDOK )
	{
		scColorMag = dlg.GetColor();
		RedrawWindow();	
	};
	return;
}

void CTMCDialogTopology::OnTopmetcolormetal() 
{
	// TODO: Add your control notification handler code here
	CColorDialog dlg;
	dlg.m_cc.rgbResult = scColorMet;
	if( dlg.DoModal() == IDOK )
	{
		scColorMet = dlg.GetColor();
		RedrawWindow();	
	};
	return;
}

void CTMCDialogTopology::OnTopmetcolordefault() 
{
	// TODO: Add your control notification handler code here
	scColorEps = RGB(   0, 230, 230 );
	scColorMet = RGB( 210, 210, 210 );
	scColorMag = RGB( 120, 230, 200 );
	scColorInp = RGB( 255,   0, 255 );
	scColorAbs = RGB( 200, 100, 100 );
	RedrawWindow();	
	return;
}

BOOL CTMCDialogTopology::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	SetModified( TRUE );
	return CPropertyPage::OnCommand(wParam, lParam);
}

BOOL CTMCDialogTopology::OnApply() 
{
	// TODO: Add your specialized code here and/or call the base class
	if( pFieldView != NULL ) 
	{
		pFieldView->SendMessage( WM_USERAPPLY_PROPTOPOL );
	};
	return CPropertyPage::OnApply();
}

void CTMCDialogTopology::OnTopinpcolorabsorber() 
{
	// TODO: Add your control notification handler code here
	CColorDialog dlg;
	dlg.m_cc.rgbResult = scColorAbs;
	if( dlg.DoModal() == IDOK )
	{
		scColorAbs = dlg.GetColor();
		RedrawWindow();	
	};
	return;
}

