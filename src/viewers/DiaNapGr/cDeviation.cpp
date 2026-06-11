// cDeviation.cpp : implementation file
//

#include "stdafx.h"
#include "tmcgrout.h"
#include "cDeviation.h"
#include "..\\DiaNapr\\c2DArray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// cDeviation dialog


cDeviation::cDeviation(CWnd* pParent /*=NULL*/)
	: CDialog(cDeviation::IDD, pParent)
{
	//{{AFX_DATA_INIT(cDeviation)
	m_csFileFrom = _T("");
	m_csFileTo = _T("");
	m_dXdev = 0.0;
	m_dYdev = 0.0;
	m_dFazaDev = 0.0;
	m_dAmpDev = 0.0;
	//}}AFX_DATA_INIT
}


void cDeviation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cDeviation)
	DDX_Text(pDX, IDC_EDITSOURCE, m_csFileFrom);
	DDX_Text(pDX, IDC_EDITSOURCE2, m_csFileTo);
	DDX_Text(pDX, IDC_XDEVIATION, m_dXdev);
	DDX_Text(pDX, IDC_YDEVIATION, m_dYdev);
	DDX_Text(pDX, IDC_YDEVIATION3, m_dFazaDev);
	DDX_Text(pDX, IDC_YDEVIATION2, m_dAmpDev);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(cDeviation, CDialog)
	//{{AFX_MSG_MAP(cDeviation)
	ON_BN_CLICKED(IDC_BUTTONSOURCE, OnButtonsource)
	ON_BN_CLICKED(IDC_BUTTONDISTINATION, OnButtondistination)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cDeviation message handlers

void cDeviation::OnButtonsource() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CFileDialog dlg( TRUE, "", "*.dat", OFN_ENABLESIZING, "Tamic Directional Pattern");
	dlg.m_ofn.lpstrTitle = "Change source *.dat file";
	if( dlg.DoModal() == IDOK )
	{
		m_csFileFrom = dlg.GetPathName();
		UpdateData(FALSE);
	};
	return;		
}

void cDeviation::OnButtondistination() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CFileDialog dlg( FALSE, "", "*.dat", OFN_ENABLESIZING, "Tamic Directional Pattern");
	dlg.m_ofn.lpstrTitle = "Change distination *.dat file";
	if( dlg.DoModal() == IDOK )
	{
		m_csFileTo = dlg.GetPathName();
		UpdateData(FALSE);
		MakeDeviation();
	};
	return;		
}

BOOL cDeviation::MakeDeviation()
{
	UpdateData(TRUE);

	BOOL b = TRUE;
	c2DArray cDn;
	char ch1[1000], ch2[1000];

	strcpy( ch1, m_csFileFrom );
	strcpy( ch2, m_csFileTo );

	cDn.MakeDeviation( ch1, ch2, m_dXdev, m_dYdev, m_dAmpDev, m_dFazaDev);
	
	if( cDn.IsError() ) 
	{
		CString csBuf;
		csBuf.Format( "%s", cDn.GetError() );
		AfxMessageBox( csBuf );
		b = FALSE;
	}
	else
	{
		b = TRUE;
	};

	UpdateData(FALSE);

	return b;		
}

void cDeviation::OnOK() 
{
	// TODO: Add extra validation here
	if( !MakeDeviation() ) return;
	CDialog::OnOK();
}
