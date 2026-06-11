// TmcASetColor.cpp : implementation file
//

#include "stdafx.h"
#include "FldView.h"
#include "TmcASetColor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcASetColor dialog


CTmcASetColor::CTmcASetColor(CWnd* pParent /*=NULL*/)
	: CDialog(CTmcASetColor::IDD, pParent)
{
	nPixel = 15;
	pc20Color = NULL;
	//{{AFX_DATA_INIT(CTmcASetColor)
	m_Color1 = -1;
	m_Color2 = -1;
	//}}AFX_DATA_INIT
}


void CTmcASetColor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTmcASetColor)
	DDX_Radio(pDX, IDC_RADIO110, m_Color1);
	DDX_Radio(pDX, IDC_RADIO130, m_Color2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTmcASetColor, CDialog)
	//{{AFX_MSG_MAP(CTmcASetColor)
	ON_BN_CLICKED(ID_ColorLevelAutoSet, OnColorLevelAutoSet)
	ON_BN_CLICKED(ID_ColorLevelAutoSet2, OnColorLevelAutoSet2)
	ON_BN_CLICKED(ID_ColorLevelAutoSet3, OnColorLevelAutoSet3)
	ON_BN_CLICKED(ID_DEFAULT_TRXYZ_ANGXYZ2, OnDefaultOk)
	ON_BN_CLICKED(ID_DEFAULT_TRXYZ_ANGXYZ, OnDefaultTrxyzAngxyz)
	ON_WM_PAINT()
	ON_BN_CLICKED(ID_ColorLevelChange, OnColorLevelChange)
	ON_BN_CLICKED(ID_ColorLevelChange2, OnColorLevelChange2)
	ON_BN_CLICKED(ID_ColorLevelAutoSet4, OnColorLevelAutoSet4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTmcASetColor message handlers

void CTmcASetColor::OnColorLevelAutoSet() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	int n1, n2, n, i;
	int n_r, n_g, n_b;
	int n_dr, n_dg, n_db;
	int n1_r, n1_g, n1_b;
	int n2_r, n2_g, n2_b;
	n1 = m_Color1;
	n2 = m_Color2;
	if( (n1 < 0)||(n1 > 19)||(n2 < 0)||(n2 > 19) ) return;
	if( abs(n1 - n2) < 2 ) return;
	if( n1 > n2 )
	{
		n = n2;
		n2 = n1;
		n1 = n;
	};
	
	n1_r = GetRValue(c20Color[n1]);
	n1_g = GetGValue(c20Color[n1]);
	n1_b = GetBValue(c20Color[n1]);
	
	n2_r = GetRValue(c20Color[n2]);
	n2_g = GetGValue(c20Color[n2]);
	n2_b = GetBValue(c20Color[n2]);

	n_dr = (n2_r - n1_r)/(n2-n1);
	n_dg = (n2_g - n1_g)/(n2-n1);
	n_db = (n2_b - n1_b)/(n2-n1);

	n_r = n1_r + n_dr;
	n_g = n1_g + n_dg;
	n_b = n1_b + n_db;
	
	for( i = n1+1; i < n2; i++, n_r += n_dr, n_g += n_dg, n_b += n_db )
	{
		c20Color[i] = RGB( n_r, n_g, n_b );
	};
	
	RedrawWindow();	
	return;
}

void CTmcASetColor::OnColorLevelAutoSet2() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	int n1, n2, n, i;
	int n_r, n_g, n_b;
	int n_dr, n_dg, n_db;
	int n1_r, n1_g, n1_b;
	int n2_r, n2_g, n2_b;
	n1 = m_Color1;
	n2 = m_Color2;
	if( (n1 < 0)||(n1 > 19)||(n2 < 0)||(n2 > 19) ) return;
	if( abs(n1 - n2) < 2 ) return;
	if( n1 > n2 )
	{
		n = n2;
		n2 = n1;
		n1 = n;
	};
	
	n1_r = GetRValue(c20Color[n1]);
	n1_g = GetGValue(c20Color[n1]);
	n1_b = GetBValue(c20Color[n1]);
	
	n2_r = GetRValue(c20Color[n2]);
	n2_g = GetGValue(c20Color[n2]);
	n2_b = GetBValue(c20Color[n2]);

	n_dr = 2*(256 - n1_r)/(n2-n1);
	n_dg = (n2_g - n1_g)/(n2-n1);
	n_db = (n2_b - n1_b)/(n2-n1);

	n_r = n1_r + n_dr;
	n_g = n1_g + n_dg;
	n_b = n1_b + n_db;
	
	for( i = n1+1; i < (n1+n2)/2; i++, n_r += n_dr, n_g += n_dg, n_b += n_db )
	{
		c20Color[i] = RGB( n_r, n_g, n_b );
	};
	n_dr = 2*(n2_r - 256)/(n2-n1);
	for( ; i < n2; i++, n_r += n_dr, n_g += n_dg, n_b += n_db )
	{
		c20Color[i] = RGB( n_r, n_g, n_b );
	};
	
	RedrawWindow();	
	return;
}

void CTmcASetColor::OnColorLevelAutoSet3() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	int n1, n2, n, i;
	int n_r, n_g, n_b;
	int n_dr, n_dg, n_db;
	int n1_r, n1_g, n1_b;
	int n2_r, n2_g, n2_b;
	n1 = m_Color1;
	n2 = m_Color2;
	if( (n1 < 0)||(n1 > 19)||(n2 < 0)||(n2 > 19) ) return;
	if( abs(n1 - n2) < 2 ) return;
	if( n1 > n2 )
	{
		n = n2;
		n2 = n1;
		n1 = n;
	};
	
	n1_r = GetRValue(c20Color[n1]);
	n1_g = GetGValue(c20Color[n1]);
	n1_b = GetBValue(c20Color[n1]);
	
	n2_r = GetRValue(c20Color[n2]);
	n2_g = GetGValue(c20Color[n2]);
	n2_b = GetBValue(c20Color[n2]);

	n_dr = (n2_r - n1_r)/(n2-n1);
	n_dg = 2*(256 - n1_g)/(n2-n1);
	n_db = (n2_b - n1_b)/(n2-n1);

	n_r = n1_r + n_dr;
	n_g = n1_g + n_dg;
	n_b = n1_b + n_db;
	
	for( i = n1+1; i < (n1+n2)/2; i++, n_r += n_dr, n_g += n_dg, n_b += n_db )
	{
		c20Color[i] = RGB( n_r, n_g, n_b );
	};
	n_dg = 2*(n2_g - 256)/(n2-n1);
	for( ; i < n2; i++, n_r += n_dr, n_g += n_dg, n_b += n_db )
	{
		c20Color[i] = RGB( n_r, n_g, n_b );
	};
	
	RedrawWindow();	
	return;
}

void CTmcASetColor::OnDefaultOk() 
{
	// TODO: Add your control notification handler code here
	if( pc20Color != NULL )
	{
		int i;
		for( i = 0; i < 20; i++ )
		{
			pc20Color[i] = c20Color[i];
		};
	};
	CDialog::OnOK();
	return;
}

void CTmcASetColor::OnDefaultTrxyzAngxyz() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
	return;
}

void CTmcASetColor::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rectClient( 0, 0, 2*nPixel, nPixel);
	int i, ii, nX = 4*nPixel, nY = nPixel/4;

	CWnd *pWnd = GetDlgItem(IDC_STATICLEVEL1);
	CDC *pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[0] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL2);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[1] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL3);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[2] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL4);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[3] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL5);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[4] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL6);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[5] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL7);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[6] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL8);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[7] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL9);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[8] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL10);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[9] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL11);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[10] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL12);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[11] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL13);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[12] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL14);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[13] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL15);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[14] );
	pWnd->ReleaseDC( pControlDC );					 

	pWnd = GetDlgItem(IDC_STATICLEVEL16);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[15] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL17);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[16] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL18);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[17] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL19);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[18] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL20);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[19] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL21);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[0] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL22);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[1] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL23);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[2] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL24);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[3] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL25);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[4] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL26);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[5] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL27);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[6] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL28);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[7] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL29);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[8] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL30);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[9] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL31);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[10] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL32);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[11] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL33);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[12] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL34);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[13] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL35);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[14] );
	pWnd->ReleaseDC( pControlDC );					 

	pWnd = GetDlgItem(IDC_STATICLEVEL36);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[15] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL37);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[16] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL38);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[17] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL39);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[18] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL40);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, c20Color[19] );
	pWnd->ReleaseDC( pControlDC );

	pWnd = GetDlgItem(IDC_STATICLEVEL_COLORALL);
	pControlDC = pWnd->GetDC();
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

int CTmcASetColor::DoModal(COLORREF *c20Color1) 
{
	// TODO: Add your specialized code here and/or call the base class
	if( c20Color1 == NULL )	 return 0;
	pc20Color = c20Color1;
	int i;
	for( i = 0; i < 20 ; i++ )
	{
		c20Color[i] = c20Color1[i];
	};
	m_Color1 = 0;
	m_Color2 = 19;
	return CDialog::DoModal();
}

void CTmcASetColor::OnColorLevelChange() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	ChangeColorLevel( m_Color1 );
	return;
}

void CTmcASetColor::OnColorLevelChange2() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	ChangeColorLevel( m_Color2 );
	return;
}

void CTmcASetColor::ChangeColorLevel(int n)
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

void CTmcASetColor::OnColorLevelAutoSet4() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	int n1, n2, n, i;
	int n_r, n_g, n_b;
	int n_dr, n_dg, n_db;
	int n1_r, n1_g, n1_b;
	int n2_r, n2_g, n2_b;
	n1 = m_Color1;
	n2 = m_Color2;
	if( (n1 < 0)||(n1 > 19)||(n2 < 0)||(n2 > 19) ) return;
	if( abs(n1 - n2) < 2 ) return;
	if( n1 > n2 )
	{
		n = n2;
		n2 = n1;
		n1 = n;
	};
	
	n1_r = GetRValue(c20Color[n1]);
	n1_g = GetGValue(c20Color[n1]);
	n1_b = GetBValue(c20Color[n1]);
	
	n2_r = GetRValue(c20Color[n2]);
	n2_g = GetGValue(c20Color[n2]);
	n2_b = GetBValue(c20Color[n2]);

	n_dr = (n2_r - n1_r)/(n2-n1);
	n_dg = (n2_g - n1_g)/(n2-n1);
	n_db = 2*(256 - n1_b)/(n2-n1);

	n_r = n1_r + n_dr;
	n_g = n1_g + n_dg;
	n_b = n1_b + n_db;
	
	for( i = n1+1; i < (n1+n2)/2; i++, n_r += n_dr, n_g += n_dg, n_b += n_db )
	{
		c20Color[i] = RGB( n_r, n_g, n_b );
	};
	n_db = 2*(n2_b - 256)/(n2-n1);
	for( ; i < n2; i++, n_r += n_dr, n_g += n_dg, n_b += n_db )
	{
		c20Color[i] = RGB( n_r, n_g, n_b );
	};
	
	RedrawWindow();	
	return;
}
