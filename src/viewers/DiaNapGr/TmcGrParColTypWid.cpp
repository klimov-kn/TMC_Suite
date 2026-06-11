// TmcGrParColTypWid.cpp : implementation file
//

#include "stdafx.h"
#include "tmcgrout.h"
#include "TmcGrParColTypWid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcGrParColTypWid dialog


CTmcGrParColTypWid::CTmcGrParColTypWid(CWnd* pParent /*=NULL*/)
	: CDialog(CTmcGrParColTypWid::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTmcGrParColTypWid)
	m_Style = -1;
	m_Width = -1;
	//}}AFX_DATA_INIT
	LineType = PS_SOLID;
	LineWidth = 2;
	LineColor = RGB( 0, 255, 255);
}


void CTmcGrParColTypWid::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTmcGrParColTypWid)
	DDX_Radio(pDX, IDC_RADIO1, m_Style);
	DDX_Radio(pDX, IDC_RADIO6, m_Width);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTmcGrParColTypWid, CDialog)
	//{{AFX_MSG_MAP(CTmcGrParColTypWid)
	ON_WM_PAINT()
	ON_BN_CLICKED(ID_GrParam_Color, OnGrParamColor)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_BN_CLICKED(IDC_RADIO4, OnRadio4)
	ON_BN_CLICKED(IDC_RADIO5, OnRadio5)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTmcGrParColTypWid message handlers

void CTmcGrParColTypWid::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	int nPixel = 30;

	CWnd *pWnd = GetDlgItem(IDC_STATICLEVEL2);
	CDC *pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	CPen newPen( PS_SOLID, LineWidth, LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};

	pWnd = GetDlgItem(IDC_STATICLEVEL3);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	CPen newPen( PS_DASH, 1, LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};

	pWnd = GetDlgItem(IDC_STATICLEVEL4);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	CPen newPen( PS_DOT, 1, LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};

	pWnd = GetDlgItem(IDC_STATICLEVEL5);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	CPen newPen( PS_DASHDOT, 1, LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};

	pWnd = GetDlgItem(IDC_STATICLEVEL6);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	CPen newPen( PS_DASHDOTDOT, 1, LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};

	pWnd = GetDlgItem(IDC_STATICLEVEL7);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	CPen newPen( PS_SOLID, 1, LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};

	pWnd = GetDlgItem(IDC_STATICLEVEL8);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	CPen newPen( PS_SOLID, 2, LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};

	pWnd = GetDlgItem(IDC_STATICLEVEL9);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	CPen newPen( PS_SOLID, 3, LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};

	pWnd = GetDlgItem(IDC_STATICLEVEL10);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	CPen newPen( PS_SOLID, 4, LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};

	pWnd = GetDlgItem(IDC_STATICLEVEL11);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	CPen newPen( PS_SOLID, 5, LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};


	return;
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CTmcGrParColTypWid::OnGrParamColor() 
{
	// TODO: Add your control notification handler code here
	CColorDialog dlg;
	dlg.m_cc.rgbResult = LineColor;
	if( dlg.DoModal() == IDOK )
	{
		LineColor = dlg.GetColor();
		RedrawWindow();	
	};
	return;
}


INT_PTR CTmcGrParColTypWid::DoModal()
{
	// TODO: Add your specialized code here and/or call the base class
	switch( LineType )
	{
		case PS_SOLID:
			m_Style = 0;
			break;
		case PS_DASH:
			m_Style = 1;
			break;
		case PS_DOT:
			m_Style = 2;
			break;
		case PS_DASHDOT:
			m_Style = 3;
			break;
		case PS_DASHDOTDOT:
			m_Style = 4;
			break;
		default:
			m_Style = 0;
			break;
	};
	switch( LineWidth )
	{
		case 1:
			m_Width= 0;
			break;
		case 2:
			m_Width = 1;
			break;
		case 3:
			m_Width = 2;
			break;
		case 4:
			m_Width = 3;
			break;
		case 5:
			m_Width = 4;
			break;
		default:
			m_Width = 5;
			break;
	};
	return CDialog::DoModal();
}

void CTmcGrParColTypWid::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData();
	switch( m_Style )
	{
		case 0:
			LineType = PS_SOLID;
			break;
		case 1:
			LineType = PS_DASH;
			break;
		case 2:
			LineType = PS_DOT;
			break;
		case 3:
			LineType = PS_DASHDOT;
			break;
		case 4:
			LineType = PS_DASHDOTDOT;
			break;
		default:
			LineType = PS_SOLID;
			break;
	};
	switch( m_Width )
	{
		case 0:
			LineWidth = 1;
			break;
		case 1:
			LineWidth = 2;
			break;
		case 2:
			LineWidth = 3;
			break;
		case 3:
			LineWidth = 4;
			break;
		case 4:
			LineWidth = 5;
			break;
		default:
			LineWidth = 50/15;
			break;
	};
	
	CDialog::OnOK();
}

void CTmcGrParColTypWid::OnRadio1() 
{
	// TODO: Add your control notification handler code here
 	UpdateData();

	// Enable/disable edit controls
	GetDlgItem( IDC_RADIO7  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO8  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO9  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO10 )->EnableWindow( m_Style == 0 );

	return;	
}

void CTmcGrParColTypWid::OnRadio2() 
{
	// TODO: Add your control notification handler code here
 	UpdateData();

	// Enable/disable edit controls
	GetDlgItem( IDC_RADIO7  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO8  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO9  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO10 )->EnableWindow( m_Style == 0 );

	m_Width = 0;
 	UpdateData(FALSE);

	return;	
}

void CTmcGrParColTypWid::OnRadio3() 
{
	// TODO: Add your control notification handler code here
 	UpdateData();

	// Enable/disable edit controls
	GetDlgItem( IDC_RADIO7  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO8  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO9  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO10 )->EnableWindow( m_Style == 0 );

	m_Width = 0;
 	UpdateData(FALSE);

	return;	
}

void CTmcGrParColTypWid::OnRadio4() 
{
	// TODO: Add your control notification handler code here
 	UpdateData();

	// Enable/disable edit controls
	GetDlgItem( IDC_RADIO7  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO8  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO9  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO10 )->EnableWindow( m_Style == 0 );

	m_Width = 0;
 	UpdateData(FALSE);

	return;	
}

void CTmcGrParColTypWid::OnRadio5() 
{
	// TODO: Add your control notification handler code here
 	UpdateData();

	// Enable/disable edit controls
	GetDlgItem( IDC_RADIO7  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO8  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO9  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO10 )->EnableWindow( m_Style == 0 );

	m_Width = 0;
 	UpdateData(FALSE);

	return;	
}

BOOL CTmcGrParColTypWid::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	GetDlgItem( IDC_RADIO7  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO8  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO9  )->EnableWindow( m_Style == 0 );
	GetDlgItem( IDC_RADIO10 )->EnableWindow( m_Style == 0 );

	if( m_Style != 0 ) m_Width = 0;
 	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

