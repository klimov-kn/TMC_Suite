// TMCGROUTDIALOGView.cpp : implementation file
//

#include "stdafx.h"
#include "TMCGROUT.h"
#include "TMCGROUTDIALOGView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTDIALOGView dialog


CTMCGROUTDIALOGView::CTMCGROUTDIALOGView(CWnd* pParent /*=NULL*/)
	: CDialog(CTMCGROUTDIALOGView::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTMCGROUTDIALOGView)
	m_ColorGraphics = FALSE;
	m_ColorBackground = FALSE;
	m_ColorAxies = FALSE;
	m_ColorGrid = FALSE;
	m_Color = FALSE;
	m_Font  = FALSE;
	m_aXSFlag = FALSE;
	m_aYSFlag = FALSE;
	m_szDatFileName = _T("");
	m_XMax = 0.0;
	m_XMin = 0.0;
	m_YMax = 0.0;
	m_YMin = 0.0;
	m_XTypeHz1 = FALSE;
	m_XTypeKHz1 = FALSE;
	m_XTypeMHz1 = FALSE;
	m_DrawPoinFlag = FALSE;
	m_XUnit = -1;
	m_YUnit = -1;
	m_csXFormat = _T("");
	m_csYFormat = _T("");
	m_dPointSize = 0.0;
	m_LossPoint = FALSE;
	m_XTypeGHz1 = FALSE;
	m_YNormirovkaFlag = FALSE;
	//}}AFX_DATA_INIT
}


void CTMCGROUTDIALOGView::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTMCGROUTDIALOGView)
	DDX_Check(pDX, IDC_GRPAR_AXSFLAG, m_aXSFlag);
	DDX_Check(pDX, IDC_GRPAR_AYSFLAG, m_aYSFlag);
	DDX_Text(pDX, IDC_GRPAR_DATFILENAME, m_szDatFileName);
	DDX_Text(pDX, IDC_GRPAR_XMAX, m_XMax);
	DDX_Text(pDX, IDC_GRPAR_XMIN, m_XMin);
	DDX_Text(pDX, IDC_GRPAR_YMAX, m_YMax);
	DDX_Text(pDX, IDC_GRPAR_YMIN, m_YMin);
	DDX_Check(pDX, IDC_GRPAR_POINTFLAG, m_DrawPoinFlag);
	DDX_Radio(pDX, IDC_GRPAR_XINGHZ, m_XUnit);
	DDX_Radio(pDX, IDC_GRPAR_YSKSV1, m_YUnit);
	DDX_Text(pDX, IDC_GRPAR_XFORMAT, m_csXFormat);
	DDV_MaxChars(pDX, m_csXFormat, 300);
	DDX_Text(pDX, IDC_GRPAR_YFORMAT, m_csYFormat);
	DDV_MaxChars(pDX, m_csYFormat, 300);
	DDX_Text(pDX, IDC_GRPAR_POINTSIZE, m_dPointSize);
	DDV_MinMaxDouble(pDX, m_dPointSize, 0., 100.);
	DDX_Check(pDX, IDC_GRPAR_POINTFLAG2, m_LossPoint);
	DDX_Check(pDX, IDC_GRPAR_NORMIROVYFLAG3, m_YNormirovkaFlag);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTMCGROUTDIALOGView, CDialog)
	//{{AFX_MSG_MAP(CTMCGROUTDIALOGView)
	ON_BN_CLICKED(IDC_GRPAR_XINGHZ, OnGrparXinghz)
	ON_BN_CLICKED(IDC_GRPAR_XINHZ, OnGrparXinhz)
	ON_BN_CLICKED(IDC_GRPAR_XINKHZ, OnGrparXinkhz)
	ON_BN_CLICKED(IDC_GRPAR_XINMHZ, OnGrparXinmhz)
	ON_BN_CLICKED(IDOK2, OnOk2)
	ON_BN_CLICKED(IDOK3, OnOk3)
	ON_BN_CLICKED(IDOK4, OnOk4)
	ON_BN_CLICKED(IDOK5, OnOk5)
	ON_BN_CLICKED(IDOK6, OnOk6)
	ON_BN_CLICKED(ID_FORMATDEFAULTBUTTON, OnFormatdefaultbutton)
	ON_BN_CLICKED(IDOK7, OnOk7)
	ON_BN_CLICKED(IDC_GRPAR_AXSFLAG, OnGrparAxsflag)
	ON_BN_CLICKED(IDC_GRPAR_AYSFLAG, OnGrparAysflag)
	ON_BN_CLICKED(IDC_GRPAR_POINTFLAG, OnGrparPointflag)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTDIALOGView message handlers

void CTMCGROUTDIALOGView::OnGrparXinghz() 
{
	// TODO: Add your control notification handler code here
	m_XTypeHz1  = FALSE;	
	m_XTypeKHz1 = FALSE;	
	m_XTypeMHz1 = FALSE;	
	m_XTypeGHz1 = FALSE;
	m_XTypeGHz1 = TRUE;
	return;
}

void CTMCGROUTDIALOGView::OnGrparXinhz() 
{
	// TODO: Add your control notification handler code here
	m_XTypeHz1  = FALSE;	
	m_XTypeKHz1 = FALSE;	
	m_XTypeMHz1 = FALSE;	
	m_XTypeGHz1 = FALSE;
	m_XTypeHz1 = TRUE;	
}

void CTMCGROUTDIALOGView::OnGrparXinkhz() 
{
	// TODO: Add your control notification handler code here
	m_XTypeHz1  = FALSE;	
	m_XTypeKHz1 = FALSE;	
	m_XTypeMHz1 = FALSE;	
	m_XTypeGHz1 = FALSE;
	m_XTypeKHz1 = TRUE;	
}

void CTMCGROUTDIALOGView::OnGrparXinmhz() 
{
	// TODO: Add your control notification handler code here
	m_XTypeHz1  = FALSE;	
	m_XTypeKHz1 = FALSE;	
	m_XTypeMHz1 = FALSE;	
	m_XTypeGHz1 = FALSE;
	m_XTypeMHz1 = TRUE;	
}


void CTMCGROUTDIALOGView::OnOk2() 
{
	// TODO: Add your control notification handler code here
	m_Font = TRUE;
	CDialog::OnOK();
}

void CTMCGROUTDIALOGView::OnOk3() 
{
	// TODO: Add your control notification handler code here
	m_Color = TRUE;
	CDialog::OnOK();
}

void CTMCGROUTDIALOGView::OnOk4() 
{
	// TODO: Add your control notification handler code here
	m_ColorGrid = TRUE;
	CDialog::OnOK();
}

void CTMCGROUTDIALOGView::OnOk5() 
{
	// TODO: Add your control notification handler code here
	m_ColorAxies = TRUE;
	CDialog::OnOK();
}

void CTMCGROUTDIALOGView::OnOk6() 
{
	// TODO: Add your control notification handler code here
	m_ColorBackground = TRUE;
	CDialog::OnOK();
}


void CTMCGROUTDIALOGView::OnFormatdefaultbutton() 
{
	// TODO: Add your control notification handler code here
	m_csXFormat.Format("%s", "%4.3lg");
	m_csYFormat.Format("%s", "%4.3lg");
	UpdateData(FALSE);
	return;
}

void CTMCGROUTDIALOGView::OnOk7() 
{
	// TODO: Add your control notification handler code here
	m_ColorGraphics = TRUE;
	CDialog::OnOK();
	return;
}


BOOL CTMCGROUTDIALOGView::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	GetDlgItem( IDC_GRPAR_XMAX )->EnableWindow( !m_aXSFlag );
	GetDlgItem( IDC_GRPAR_XMIN )->EnableWindow( !m_aXSFlag );
	
	GetDlgItem( IDC_GRPAR_YMAX )->EnableWindow( !m_aYSFlag );
	GetDlgItem( IDC_GRPAR_YMIN )->EnableWindow( !m_aYSFlag );

	GetDlgItem( IDC_GRPAR_POINTFLAG2  )->EnableWindow( m_DrawPoinFlag );
	GetDlgItem( IDC_GRPAR_POINTSIZE )->EnableWindow( m_DrawPoinFlag );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTMCGROUTDIALOGView::OnGrparAxsflag() 
{
	// TODO: Add your control notification handler code here
 	UpdateData();


	// Enable/disable edit controls
	GetDlgItem( IDC_GRPAR_XMAX )->EnableWindow( !m_aXSFlag );
	GetDlgItem( IDC_GRPAR_XMIN )->EnableWindow( !m_aXSFlag );

	return;	
}

void CTMCGROUTDIALOGView::OnGrparAysflag() 
{
	// TODO: Add your control notification handler code here
 	UpdateData();

	
	// Enable/disable edit controls
	GetDlgItem( IDC_GRPAR_YMAX )->EnableWindow( !m_aYSFlag );
	GetDlgItem( IDC_GRPAR_YMIN )->EnableWindow( !m_aYSFlag );


	return;	
}

void CTMCGROUTDIALOGView::OnGrparPointflag() 
{
	// TODO: Add your control notification handler code here
 	UpdateData();

	
	// Enable/disable edit controls
	GetDlgItem( IDC_GRPAR_POINTFLAG2  )->EnableWindow( m_DrawPoinFlag );
	GetDlgItem( IDC_GRPAR_POINTSIZE )->EnableWindow( m_DrawPoinFlag );
	
	return;	
}
