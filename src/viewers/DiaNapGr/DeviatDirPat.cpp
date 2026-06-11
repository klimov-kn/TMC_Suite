// DeviatDirPat.cpp : implementation file
//

#include "stdafx.h"
#include "tmcgrout.h"
#include "DeviatDirPat.h"
#include "..\\DiaNapr\\c2DArray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDeviatDirPat dialog


CDeviatDirPat::CDeviatDirPat(CWnd* pParent /*=NULL*/)
	: CDialog(CDeviatDirPat::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDeviatDirPat)
	m_csFileDevDirPat = _T("");
	m_csDistrAmplFaza = _T("");
	m_csRequireDirPat = _T("");
	//}}AFX_DATA_INIT
}


void CDeviatDirPat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeviatDirPat)
	DDX_Text(pDX, IDC_DEVIATDIRPATTERN, m_csFileDevDirPat);
	DDX_Text(pDX, IDC_FILEDISTRAMPLFAZA, m_csDistrAmplFaza);
	DDX_Text(pDX, IDC_REQUIREDDIRPAT, m_csRequireDirPat);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDeviatDirPat, CDialog)
	//{{AFX_MSG_MAP(CDeviatDirPat)
	ON_BN_CLICKED(IDC_BUTTONDEVIATDIRPATTERN, OnButtondeviatdirpattern)
	ON_BN_CLICKED(IDC_BUTTONDISTRAMPLFAZA, OnButtondistramplfaza)
	ON_BN_CLICKED(IDC_BUTTONREQURDIRECTPATTERN, OnButtonrequrdirectpattern)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeviatDirPat message handlers

void CDeviatDirPat::OnButtondeviatdirpattern() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CFileDialog dlg( FALSE, "", "*.dev", OFN_ENABLESIZING, "Tamic Deviation Directional Pattern");
	dlg.m_ofn.lpstrTitle = "Save *.dev file with Deviation Directional Pattern";
	if( dlg.DoModal() == IDOK )
	{
		m_csFileDevDirPat = dlg.GetPathName();
		UpdateData(FALSE);
		CalculateDeviationDirectionalPattern();
	};
	return;		
}

void CDeviatDirPat::OnButtondistramplfaza() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CFileDialog dlg( TRUE, "", "*.dat", OFN_ENABLESIZING, "Tamic Directional Pattern");
	dlg.m_ofn.lpstrTitle = "Change *.dat file with Amplitude and Faza distribution";
	if( dlg.DoModal() == IDOK )
	{
		m_csDistrAmplFaza = dlg.GetPathName();
		UpdateData(FALSE);
	};
	return;		
}

void CDeviatDirPat::OnButtonrequrdirectpattern() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CFileDialog dlg( TRUE, "", "*.opt", OFN_ENABLESIZING, "Tamic Required Directional Pattern");
	dlg.m_ofn.lpstrTitle = "Change *.opt file with Required Directional Pattern";
	if( dlg.DoModal() == IDOK )
	{
		m_csRequireDirPat = dlg.GetPathName();
		UpdateData(FALSE);
	};
	return;		
}

void CDeviatDirPat::OnOK() 
{
	// TODO: Add extra validation here
	if( !CalculateDeviationDirectionalPattern() ) return;
	CDialog::OnOK();
}

BOOL CDeviatDirPat::CalculateDeviationDirectionalPattern()
{
	UpdateData(TRUE);

	c2DArray cDn1, cDn2;
	char ch1[1000], ch2[1000], ch3[1000];
	int i, nIzl;
	double x, y, a, f;

	strcpy( ch1, m_csDistrAmplFaza );
	strcpy( ch2, m_csRequireDirPat );
	strcpy( ch3, m_csFileDevDirPat );

	cDn1.ReadData( ch1 );
	if( cDn1.IsError() ) 
	{
		CString csBuf;
		csBuf.Format( "%s", cDn1.GetError() );
		AfxMessageBox( csBuf );
		return FALSE;
	};
	nIzl = cDn1.GetnIzl();
	cDn2.OptimizationInit( ch2, nIzl );
	if( cDn2.IsError() ) 
	{
		CString csBuf;
		csBuf.Format( "%s", cDn2.GetError() );
		AfxMessageBox( csBuf );
		return FALSE;
	};

	for( i = 0; i < nIzl ; i++ )
	{
		cDn1.GetIzl( i, &x, &y, &a, &f );
		if( cDn1.IsError() ) 
		{
			CString csBuf;
			csBuf.Format( "%s", cDn1.GetError() );
			AfxMessageBox( csBuf );
			return FALSE;
		};
		cDn2.SetdFrequence( cDn1.GetdFreqGHz() );
		cDn2.SetszOneExpression( cDn1.GetOneExpression() );
		cDn2.SetIzl( i, x, y, a, f );
		if( cDn2.IsError() ) 
		{
			CString csBuf;
			csBuf.Format( "%s", cDn2.GetError() );
			AfxMessageBox( csBuf );
			return FALSE;
		};
	};

	cDn2.SaveDeviationDirectionalPattern( ch3 );
	if( cDn2.IsError() ) 
	{
		CString csBuf;
		csBuf.Format( "%s", cDn2.GetError() );
		AfxMessageBox( csBuf );
		return FALSE;
	};

	UpdateData(FALSE);

	return TRUE;		
}
