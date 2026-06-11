// TMCDialogField.cpp : implementation file
//

#include "stdafx.h"
#include "FldView.h"
#include "FldViewDoc.h"
#include "FldViewView.h"
#include "TMCDialogField.h"
#include "TmcASetColor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTMCDialogField property page

IMPLEMENT_DYNCREATE(CTMCDialogField, CPropertyPage)

CTMCDialogField::CTMCDialogField() : CPropertyPage(CTMCDialogField::IDD)
, bHightColorResolution(FALSE)
, bModulFlag(FALSE)
{
	int i;
	pFieldView = NULL;
	nPixel = 15;
	for( i = 0; i < 20; i++ )	
	{
		c20Color[i] = RGB( 255-i*10, 255, i*10);
//		c20ColorBuf[i] = c20Color[i];
	};
	//{{AFX_DATA_INIT(CTMCDialogField)
	m_bAutoSizeFlag = FALSE;
	m_dBlend = 0;
	m_bDrawFlagLine = FALSE;
	m_bDrawFlagSurface = FALSE;
	m_dZAxiesMax = 0.0;
	m_dZAxiesMin = 0.0;
	m_d23Dimension = -1;
	m_DrawSizeFlag = FALSE;
	//}}AFX_DATA_INIT
}

CTMCDialogField::~CTMCDialogField()
{
}

void CTMCDialogField::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTMCDialogField)
	DDX_Check(pDX, IDC_AUTOSIZEFLAGZ, m_bAutoSizeFlag);
	DDX_Text(pDX, IDC_BLEND, m_dBlend);
	DDV_MinMaxInt(pDX, m_dBlend, 0, 100);
	DDX_Check(pDX, IDC_DRAWFLAGLINE, m_bDrawFlagLine);
	DDX_Check(pDX, IDC_DRAWFLAGSURFACE, m_bDrawFlagSurface);
	DDX_Text(pDX, IDC_ZAXIESMAX, m_dZAxiesMax);
	DDX_Text(pDX, IDC_ZAXIESMIN, m_dZAxiesMin);
	DDX_Radio(pDX, IDC_RADIO1, m_d23Dimension);
	DDX_Check(pDX, IDC_AUTODRAWSIZEFLAGZ, m_DrawSizeFlag);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_AUTODRAWSIZEFLAGZ2, bHightColorResolution);
	DDX_Check(pDX, IDC_AUTODRAWSIZEFLAGZ3, bModulFlag);
}


BEGIN_MESSAGE_MAP(CTMCDialogField, CPropertyPage)
	//{{AFX_MSG_MAP(CTMCDialogField)
	ON_WM_PAINT()
	ON_BN_CLICKED(ID_DEFAULT_TRXYZ_ANGXYZ, OnDefaultTrxyzAngxyz)
	ON_BN_CLICKED(ID_ColorLevelChangeAuto, OnColorLevelChangeAuto)
	ON_BN_CLICKED(IDC_AUTOSIZEFLAGZ, OnAutosizeflagz)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_AUTODRAWSIZEFLAGZ2, OnBnClickedAutodrawsizeflagz2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTMCDialogField message handlers

void CTMCDialogField::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rectClient( 0, 0, 2*nPixel, nPixel);
	int i, ii, nX = 7*nPixel, nY = (int)(nPixel/2.);
	double n_r, n_g, n_b;
	double n_dr, n_dg, n_db;
	int n1_r, n1_g, n1_b;
	int n2_r, n2_g, n2_b;
	COLORREF cBuf;

	CWnd *pWnd = GetDlgItem(IDC_STATICLEVEL1);
	CDC *pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	if( !bHightColorResolution )
	{
		for( i = 0, ii = 0; i < 20 ; i++, ii += nY )
		{
			rectClient.SetRect( 0, ii, nX, ii + nY );
			pControlDC->FillSolidRect( &rectClient, c20Color[19-i] );
		};
	}
	else
	{
		n1_r = GetRValue(c20Color[19]);
		n1_g = GetGValue(c20Color[19]);
		n1_b = GetBValue(c20Color[19]);
		
		n2_r = GetRValue(c20Color[0]);
		n2_g = GetGValue(c20Color[0]);
		n2_b = GetBValue(c20Color[0]);

		n_dr = (n2_r - n1_r)/(20.*nY-1);
		n_dg = (n2_g - n1_g)/(20.*nY-1);
		n_db = (n2_b - n1_b)/(20.*nY-1);

		n_r = n1_r;
		n_g = n1_g;
		n_b = n1_b;
//		n_r = n1_r + n_dr;
//		n_g = n1_g + n_dg;
//		n_b = n1_b + n_db;
	
		for( i = 0, ii = 0; i < 20*nY ; i++, ii++, n_r += n_dr, n_g += n_dg, n_b += n_db )
		{
			rectClient.SetRect( 0, ii, nX, ii + 1 );
			cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
			pControlDC->FillSolidRect( &rectClient, cBuf );
		};
	};

	pWnd->ReleaseDC( pControlDC );
	// Do not call CPropertyPage::OnPaint() for painting messages
	return;
}

void CTMCDialogField::OnDefaultTrxyzAngxyz() 
{
	// TODO: Add your control notification handler code here
/*
	c20Color[  0 ] = RGB( 25,125, 225);
	c20Color[  1 ] = RGB( 50,100, 200);
	c20Color[  2 ] = RGB( 75, 75, 175);
	c20Color[  3 ] = RGB(100, 55, 150);
	c20Color[  4 ] = RGB(125, 25, 125);
	c20Color[  5 ] = RGB(150, 25, 125);
	c20Color[  6 ] = RGB(200, 25, 100);
	c20Color[  7 ] = RGB(225, 50, 100);
	c20Color[  8 ] = RGB(250, 75, 100);
	c20Color[  9 ] = RGB(225,100, 100);
	c20Color[ 10 ] = RGB(200,125,  75);
	c20Color[ 11 ] = RGB(175,150, 100);
	c20Color[ 12 ] = RGB(150,175, 125);
	c20Color[ 13 ] = RGB(125,200, 150);
	c20Color[ 14 ] = RGB(100,225, 175);
	c20Color[ 15 ] = RGB(100,250, 100);
	c20Color[ 16 ] = RGB(125,225, 125);
	c20Color[ 17 ] = RGB(225,225,  50);
	c20Color[ 18 ] = RGB(255,255,   0);
	c20Color[ 19 ] = RGB(255,255, 255);
*/
	int i;
	for( i = 0; i < 7; i++ )
	{
		c20Color[ i ] = RGB( 100, i*35, 255-i*35 );
	};
	for( i = 7; i < 14; i++ )
	{
		c20Color[ i ] = RGB( (i-7)*35, 255-(i-7)*35, 150 );
	};
	for( i = 14; i < 18; i++ )
	{
		c20Color[ i ] = RGB( 255, (i-14)*83, 100 );
	};
	c20Color[ 18 ] = RGB( 255, 255,   0 );
	c20Color[ 19 ] = RGB( 255, 255, 255 );
	RedrawWindow();	
	return;
}

void CTMCDialogField::OnOK() 
{
	// TODO: Add your specialized code here and/or call the base class
	CPropertyPage::OnOK();
}

BOOL CTMCDialogField::OnApply() 
{
	// TODO: Add your specialized code here and/or call the base class
	if( pFieldView != NULL ) 
	{
		pFieldView->SendMessage( WM_USERAPPLY_PROPFIELD );
	};
	return CPropertyPage::OnApply();
}


void CTMCDialogField::ChangeColorLevel(int n)
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


BOOL CTMCDialogField::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	SetModified( TRUE );
	return CPropertyPage::OnCommand(wParam, lParam);
}

void CTMCDialogField::OnColorLevelChangeAuto() 
{
	// TODO: Add your control notification handler code here
	CTmcASetColor dlg;
//	dlg.m_cc.rgbResult = c20Color[ n ];
	dlg.DoModal(c20Color);
	RedrawWindow();	
	return;
}

void CTMCDialogField::OnAutosizeflagz() 
{
	// TODO: Add your control notification handler code here
 	UpdateData();

	// Enable/disable edit controls
	GetDlgItem( IDC_ZAXIESMIN  )->EnableWindow( !m_bAutoSizeFlag );
	GetDlgItem( IDC_ZAXIESMAX  )->EnableWindow( !m_bAutoSizeFlag );

	return;	
}

BOOL CTMCDialogField::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here

	GetDlgItem( IDC_ZAXIESMIN  )->EnableWindow( !m_bAutoSizeFlag );
	GetDlgItem( IDC_ZAXIESMAX  )->EnableWindow( !m_bAutoSizeFlag );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTMCDialogField::OnBnClickedAutodrawsizeflagz2()
{
	// TODO: Add your control notification handler code here
 	UpdateData();

/*
	int i;
	COLORREF cBuf;

	if( bHightColorResolution )
	{
		for( i = 0; i < 20; i++ )	
		{
			cBuf = c20Color[i];
			c20Color[i] = c20ColorBuf[i];
			c20ColorBuf[i] = cBuf;
		};
	}
	else
	{
		for( i = 0; i < 20; i++ )	
		{
			cBuf = c20ColorBuf[i];
			c20ColorBuf[i] = c20Color[i];
			c20Color[i] = cBuf;
		};
	};

*/
	RedrawWindow();	

	return;
}
