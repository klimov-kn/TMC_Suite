// TMCDialogPlace.cpp : implementation file
//

#include "stdafx.h"
#include "FldView.h"
#include "FldViewDoc.h"
#include "FldViewView.h"
#include "TMCDialogPlace.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTMCDialogPlace property page

IMPLEMENT_DYNCREATE(CTMCDialogPlace, CPropertyPage)

CTMCDialogPlace::CTMCDialogPlace() : CPropertyPage(CTMCDialogPlace::IDD)
{
	pFieldView = NULL;
	scColorAxies = RGB( 255, 255, 255);
	nPixel = 20;
    lfInitial.lfHeight = 12 * GetDeviceCaps( wglGetCurrentDC(), LOGPIXELSY) / 72;
    lfInitial.lfCharSet = SHIFTJIS_CHARSET;
    lfInitial.lfWeight = 400;
    lfInitial.lfOutPrecision = OUT_STROKE_PRECIS;
    lfInitial.lfClipPrecision = CLIP_CHARACTER_PRECIS;
    lstrcpy(lfInitial.lfFaceName, "TimesET");
    lfInitial.lfItalic = FALSE;
    lfInitial.lfUnderline = FALSE;
    lfInitial.lfStrikeOut = FALSE;
	m_AutoFlagX = FALSE;
	m_m_AutoFlagY = FALSE;
	m_m_AxiesDrawFlag = FALSE;
	m_dXAxiesAngle = 0.0;
	m_dYAxiesAngle = 0.0;
	m_dXAxiesMax = 0.0;
	m_dYAxiesMax = 0.0;
	m_dXAxiesMin = 0.0;
	m_dYAxiesMin = 0.0;
	m_dXAxiesTrans = 0.0;
	m_dYAxiesTrans = 0.0;
	m_DrawFlafSizeX = FALSE;
	m_DrawFlafSizeY = FALSE;
	m_Proportionally = FALSE;
	//}}AFX_DATA_INIT
}

CTMCDialogPlace::~CTMCDialogPlace()
{
}

void CTMCDialogPlace::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTMCDialogPlace)
	DDX_Check(pDX, IDC_AUTOSIZEFLAGX, m_AutoFlagX);
	DDX_Check(pDX, IDC_AUTOSIZEFLAGY, m_m_AutoFlagY);
	DDX_Check(pDX, IDC_DRAWAXIESFLAG, m_m_AxiesDrawFlag);
	DDX_Text(pDX, IDC_XAXIESANGLE, m_dXAxiesAngle);
	DDX_Text(pDX, IDC_YAXIESANGLE, m_dYAxiesAngle);
	DDX_Text(pDX, IDC_XAXIESMAX, m_dXAxiesMax);
	DDX_Text(pDX, IDC_YAXIESMAX, m_dYAxiesMax);
	DDX_Text(pDX, IDC_XAXIESMIN, m_dXAxiesMin);
	DDX_Text(pDX, IDC_YAXIESMIN, m_dYAxiesMin);
	DDX_Text(pDX, IDC_XAXIESTRANS, m_dXAxiesTrans);
	DDX_Text(pDX, IDC_YAXIESTRANS, m_dYAxiesTrans);
	DDX_Text(pDX, IDC_TIMEUNIT, m_TimeUnit);
	DDX_Text(pDX, IDC_LONGUNIT, m_LongUnit);
	DDX_Check(pDX, IDC_AUTODRAWSIZEFLAGX, m_DrawFlafSizeX);
	DDX_Check(pDX, IDC_AUTODRAWSIZEFLAGY, m_DrawFlafSizeY);
	DDX_Check(pDX, IDC_FLGPROPORTIONALLY, m_Proportionally);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTMCDialogPlace, CPropertyPage)
	//{{AFX_MSG_MAP(CTMCDialogPlace)
	ON_BN_CLICKED(ID_PLACEAXIESCOLOR, OnPlaceaxiescolor)
	ON_BN_CLICKED(ID_PLACEAXIESFONT, OnPlaceaxiesfont)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_AUTOSIZEFLAGX, OnAutosizeflagx)
	ON_BN_CLICKED(IDC_AUTOSIZEFLAGY, OnAutosizeflagy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTMCDialogPlace message handlers

void CTMCDialogPlace::OnPlaceaxiescolor() 
{
	// TODO: Add your control notification handler code here
	CColorDialog dlg;
	
	dlg.m_cc.rgbResult = scColorAxies;

	if( dlg.DoModal() == IDOK )
	{
		scColorAxies = dlg.GetColor();
		RedrawWindow();	
	};
	return;
}

void CTMCDialogPlace::OnPlaceaxiesfont() 
{
	// TODO: Add your control notification handler code here
	CFontDialog dlg( &lfInitial, CF_SCREENFONTS );
//	CFontDialog dlg( &lfInitial, CF_EFFECTS | CF_SCREENFONTS);
	
	dlg.m_cf.rgbColors = scColorAxies;

	if( dlg.DoModal() == IDOK )
	{
//		scColorAxies = dlg.GetColor();
		RedrawWindow();	
	};
	return;
}

void CTMCDialogPlace::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rectClient( 0, 0, nPixel, nPixel);

	CWnd *pWnd = GetDlgItem(IDC_STATICAXIESCOLORIM);
	CDC *pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pControlDC->SelectStockObject( BLACK_BRUSH );
	pControlDC->FillSolidRect( &rectClient, scColorAxies );
	pWnd->ReleaseDC( pControlDC );

	// Do not call CPropertyPage::OnPaint() for painting messages
	return;
}

BOOL CTMCDialogPlace::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	SetModified( TRUE );
	return CPropertyPage::OnCommand(wParam, lParam);
}

BOOL CTMCDialogPlace::OnApply() 
{
	// TODO: Add your specialized code here and/or call the base class
	if( pFieldView != NULL ) 
	{
		pFieldView->SendMessage( WM_USERAPPLY_PROPPLACE );
	};
	return CPropertyPage::OnApply();
}

void CTMCDialogPlace::OnAutosizeflagx() 
{
	// TODO: Add your control notification handler code here
 	UpdateData();

	// Enable/disable edit controls
	GetDlgItem( IDC_XAXIESMIN  )->EnableWindow( !m_AutoFlagX );
	GetDlgItem( IDC_XAXIESMAX  )->EnableWindow( !m_AutoFlagX );

	return;	
}

void CTMCDialogPlace::OnAutosizeflagy() 
{
	// TODO: Add your control notification handler code here
 	UpdateData();

	// Enable/disable edit controls
	GetDlgItem( IDC_YAXIESMIN  )->EnableWindow( !m_m_AutoFlagY );
	GetDlgItem( IDC_YAXIESMAX  )->EnableWindow( !m_m_AutoFlagY );

	return;	
}

BOOL CTMCDialogPlace::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	GetDlgItem( IDC_XAXIESMIN  )->EnableWindow( !m_AutoFlagX );
	GetDlgItem( IDC_XAXIESMAX  )->EnableWindow( !m_AutoFlagX );
	GetDlgItem( IDC_YAXIESMIN  )->EnableWindow( !m_m_AutoFlagY );
	GetDlgItem( IDC_YAXIESMAX  )->EnableWindow( !m_m_AutoFlagY );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
