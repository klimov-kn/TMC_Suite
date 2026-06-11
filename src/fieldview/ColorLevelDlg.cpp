// ColorLevelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FldView.h"
#include "FldViewDoc.h"
#include "FldViewView.h"
#include "ColorLevelDlg.h"


// CColorLevelDlg dialog

IMPLEMENT_DYNAMIC(CColorLevelDlg, CDialog)
CColorLevelDlg::CColorLevelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CColorLevelDlg::IDD, pParent)
	, pcFldView(NULL)
	, pbHightColorRezolution(NULL)
{
	int i;
	for( i = 0; i < 20; i++)
	{
		pdLevel[ i ] = i;
	};
}

CColorLevelDlg::CColorLevelDlg(CView *pFldView)
{
	int i;
	for( i = 0; i < 20; i++)
	{
		pdLevel[ i ] = i;
	};
	pcFldView = (void *)pFldView;
	pscSurfaceColor = ((CFldViewView *)(pcFldView))->GetpscSurfaceColor();
	pbHightColorRezolution = ((CFldViewView *)(pcFldView))->GetbHightColorRezolution();
	return;
}

CColorLevelDlg::~CColorLevelDlg()
{
}

void CColorLevelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1,  pdLevel[19]);
	DDX_Text(pDX, IDC_EDIT2,  pdLevel[18]);
	DDX_Text(pDX, IDC_EDIT3,  pdLevel[17]);
	DDX_Text(pDX, IDC_EDIT4,  pdLevel[16]);
	DDX_Text(pDX, IDC_EDIT5,  pdLevel[15]);
	DDX_Text(pDX, IDC_EDIT6,  pdLevel[14]);
	DDX_Text(pDX, IDC_EDIT7,  pdLevel[13]);
	DDX_Text(pDX, IDC_EDIT8,  pdLevel[12]);
	DDX_Text(pDX, IDC_EDIT9,  pdLevel[11]);
	DDX_Text(pDX, IDC_EDIT10, pdLevel[10]);
	DDX_Text(pDX, IDC_EDIT11, pdLevel[ 9]);
	DDX_Text(pDX, IDC_EDIT12, pdLevel[ 8]);
	DDX_Text(pDX, IDC_EDIT13, pdLevel[ 7]);
	DDX_Text(pDX, IDC_EDIT14, pdLevel[ 6]);
	DDX_Text(pDX, IDC_EDIT15, pdLevel[ 5]);
	DDX_Text(pDX, IDC_EDIT16, pdLevel[ 4]);
	DDX_Text(pDX, IDC_EDIT17, pdLevel[ 3]);
	DDX_Text(pDX, IDC_EDIT18, pdLevel[ 2]);
	DDX_Text(pDX, IDC_EDIT19, pdLevel[ 1]);
	DDX_Text(pDX, IDC_EDIT20, pdLevel[ 0]);
}


BEGIN_MESSAGE_MAP(CColorLevelDlg, CDialog)
	ON_BN_CLICKED(ID_DEFAULT_TRXYZ_ANGXYZ2, OnBnClickedDefaultTrxyzAngxyz2)
	ON_BN_CLICKED(ID_DEFAULT_TRXYZ_ANGXYZ, OnBnClickedDefaultTrxyzAngxyz)
	ON_WM_CLOSE()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CColorLevelDlg message handlers

void CColorLevelDlg::OnBnClickedDefaultTrxyzAngxyz2()
{
	// TODO: Add your control notification handler code here
	if( pcFldView != NULL )
	{
		UpdateData( TRUE );
		((CFldViewView *)pcFldView)->PostMessage( WM_COLOR_LEVEL_DIALOG_BYE, IDOK );
	}
	else
	{
		CDialog::OnOK();
	};
	return;
}

void CColorLevelDlg::OnBnClickedDefaultTrxyzAngxyz()
{
	// TODO: Add your control notification handler code here
	if( pcFldView != NULL )
	{
		((CFldViewView *)pcFldView)->PostMessage( WM_COLOR_LEVEL_DIALOG_BYE, IDCANCEL );
	}
	else
	{
		CDialog::OnCancel();
	};
	return;
}

void CColorLevelDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
//	nPropertyPageInd = GetActiveIndex();
	if( pcFldView != NULL )
	{
		((CFldViewView *)pcFldView)->PostMessage( WM_COLOR_LEVEL_DIALOG_BYE, IDCANCEL );
	}
	else
	{
		CDialog::OnClose();
	};
	return;
}

BOOL CColorLevelDlg::Create(void)
{
	return CDialog::Create( CColorLevelDlg::IDD );
}


void CColorLevelDlg::OnPaint() 
{
	UpdateData( FALSE );
	CPaintDC dc(this); // device context for painting
	
	int nPixel;

	nPixel = 15;

	CRect rectClient( 0, 0, 2*nPixel, nPixel);

	if( !pbHightColorRezolution[0] )
	{
		CWnd *pWnd = GetDlgItem(IDC_STATICLEVEL1);
		CDC *pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[19] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL2);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[18] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL3);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[17] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL4);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[16] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL5);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[15] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL6);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[14] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL7);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[13] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL8);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[12] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL9);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[11] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL10);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[10] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL11);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[9] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL12);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[8] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL13);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[7] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL14);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[6] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL15);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[5] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL16);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[4] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL17);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[3] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL18);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[2] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL19);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[1] );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL20);
		pControlDC = pWnd->GetDC();
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, pscSurfaceColor[0] );
		pWnd->ReleaseDC( pControlDC );
	}
	else
	{
		double n_r, n_g, n_b;
		double n_dr, n_dg, n_db;
		int n1_r, n1_g, n1_b;
		int n2_r, n2_g, n2_b;
		COLORREF cBuf;

		n1_r = GetRValue(pscSurfaceColor[19]);
		n1_g = GetGValue(pscSurfaceColor[19]);
		n1_b = GetBValue(pscSurfaceColor[19]);
		
		n2_r = GetRValue(pscSurfaceColor[0]);
		n2_g = GetGValue(pscSurfaceColor[0]);
		n2_b = GetBValue(pscSurfaceColor[0]);

		n_dr = (n2_r - n1_r)/(20.-1);
		n_dg = (n2_g - n1_g)/(20.-1);
		n_db = (n2_b - n1_b)/(20.-1);

		n_r = n1_r;
		n_g = n1_g;
		n_b = n1_b;
	
		CWnd *pWnd = GetDlgItem(IDC_STATICLEVEL1);
		CDC *pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL2);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL3);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL4);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL5);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL6);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL7);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL8);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL9);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL10);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL11);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL12);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL13);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL14);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL15);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL16);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL17);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL18);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL19);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

		pWnd = GetDlgItem(IDC_STATICLEVEL20);
		pControlDC = pWnd->GetDC();
		cBuf = RGB( (int)(n_r), (int)(n_g), (int)(n_b) );
		n_r += n_dr;
		n_g += n_dg;
		n_b += n_db;
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		pControlDC->SelectStockObject( BLACK_BRUSH );
		pControlDC->FillSolidRect( &rectClient, cBuf );
		pWnd->ReleaseDC( pControlDC );

	};

	// Do not call CPropertyPage::OnPaint() for painting messages
	return;
}