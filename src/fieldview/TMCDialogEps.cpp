// TMCDialogEps.cpp : implementation file
//

#include "stdafx.h"
#include "FldView.h"
#include "FldViewDoc.h"
#include "FldViewView.h"
#include "TMCDialogEps.h"
#include "TmcASetColor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTMCDialogEps property page

IMPLEMENT_DYNCREATE(CTMCDialogEps, CPropertyPage)

CTMCDialogEps::CTMCDialogEps() : CPropertyPage(CTMCDialogEps::IDD)
{
	int i;
	pFieldView = NULL;
	nPixel = 15;
	for( i = 0; i < 20; i++ )	c20Color[19-i] = RGB( 255-i*10, 255, i*10);
	//{{AFX_DATA_INIT(CTMCDialogEps)
	m_d23Dimension = -1;
	m_dBlend = 0;
	m_bDrawFlagLine = FALSE;
	m_bDrawFlagSurface = FALSE;
	m_bAutoSizeFlag = FALSE;
	m_DrawSizeFlag = FALSE;
	m_dZAxiesMax = 0.0;
	m_dZAxiesMin = 0.0;
	//}}AFX_DATA_INIT
}

CTMCDialogEps::~CTMCDialogEps()
{
}

void CTMCDialogEps::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTMCDialogEps)
	DDX_Radio(pDX, IDC_RADIO1, m_d23Dimension);
	DDX_Text(pDX, IDC_BLEND, m_dBlend);
	DDX_Check(pDX, IDC_DRAWFLAGLINE, m_bDrawFlagLine);
	DDX_Check(pDX, IDC_DRAWFLAGSURFACE, m_bDrawFlagSurface);
	DDX_Check(pDX, IDC_AUTOSIZEFLAGZ, m_bAutoSizeFlag);
	DDX_Check(pDX, IDC_AUTODRAWSIZEFLAGZ, m_DrawSizeFlag);
	DDX_Text(pDX, IDC_ZAXIESMAX, m_dZAxiesMax);
	DDX_Text(pDX, IDC_ZAXIESMIN, m_dZAxiesMin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTMCDialogEps, CPropertyPage)
	//{{AFX_MSG_MAP(CTMCDialogEps)
	ON_WM_PAINT()
	ON_BN_CLICKED(ID_DEFAULT_TRXYZ_ANGXYZ, OnDefaultTrxyzAngxyz)
	ON_BN_CLICKED(ID_ColorLevelChange, OnColorLevelChange)
	ON_BN_CLICKED(IDC_AUTOSIZEFLAGZ, OnAutosizeflagz)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTMCDialogEps message handlers

void CTMCDialogEps::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rectClient( 0, 0, 2*nPixel, nPixel);
	int i, ii, nX = 7*nPixel, nY = (int)(nPixel/2.);

	CWnd *pWnd = GetDlgItem(IDC_STATICLEVEL1);
	CDC *pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	for( i = 0, ii = 0; i < 20 ; i++, ii += nY )
	{
		rectClient.SetRect( 0, ii, nX, ii + nY );
		pControlDC->FillSolidRect( &rectClient, c20Color[19-i] );
	};
	pWnd->ReleaseDC( pControlDC );
	// Do not call CPropertyPage::OnPaint() for painting messages
	return;
}

void CTMCDialogEps::OnDefaultTrxyzAngxyz() 
{
	// TODO: Add your control notification handler code here
	int i;
	for( i = 0; i < 20; i++ )	c20Color[19-i] = RGB( 255-i*10, 255, i*10);
	RedrawWindow();	
	return;
}

void CTMCDialogEps::OnColorLevelChange() 
{
	// TODO: Add your control notification handler code here
	CTmcASetColor dlg;
//	dlg.m_cc.rgbResult = c20Color[ n ];
	dlg.DoModal(c20Color);
	RedrawWindow();	
	return;
}

void CTMCDialogEps::ChangeColorLevel(int n)
{
	if( n < 0 ) return;
	if( n > 19 ) return;

	CColorDialog dlg;
	dlg.m_cc.rgbResult = c20Color[ n ];
	if( dlg.DoModal() == IDOK )
	{
		c20Color[ n ] = dlg.GetColor();
		RedrawWindow();	
	};
	return;
}

BOOL CTMCDialogEps::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	SetModified( TRUE );
	return CPropertyPage::OnCommand(wParam, lParam);
}

BOOL CTMCDialogEps::OnApply() 
{
	// TODO: Add your specialized code here and/or call the base class
	if( pFieldView != NULL ) 
	{
		pFieldView->SendMessage( WM_USERAPPLY_PROPEPS );
	};
	return CPropertyPage::OnApply();
}

void CTMCDialogEps::OnAutosizeflagz() 
{
	// TODO: Add your control notification handler code here
 	UpdateData();

	// Enable/disable edit controls
	GetDlgItem( IDC_ZAXIESMIN  )->EnableWindow( !m_bAutoSizeFlag );
	GetDlgItem( IDC_ZAXIESMAX  )->EnableWindow( !m_bAutoSizeFlag );

	return;	
}

BOOL CTMCDialogEps::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	GetDlgItem( IDC_ZAXIESMIN  )->EnableWindow( !m_bAutoSizeFlag );
	GetDlgItem( IDC_ZAXIESMAX  )->EnableWindow( !m_bAutoSizeFlag );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
