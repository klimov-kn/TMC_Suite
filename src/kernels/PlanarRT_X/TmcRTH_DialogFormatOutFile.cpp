// TmcRTH_DialogFormatOutFile.cpp : implementation file
//

#include "stdafx.h"
#include "PlanRT_H.h"
#include "TmcRTH_DialogFormatOutFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_DialogFormatOutFile dialog


CTmcRTH_DialogFormatOutFile::CTmcRTH_DialogFormatOutFile(CWnd* pParent /*=NULL*/)
	: CDialog(CTmcRTH_DialogFormatOutFile::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTmcRTH_DialogFormatOutFile)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	IsSetDefaultFormat = FALSE;
}


void CTmcRTH_DialogFormatOutFile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTmcRTH_DialogFormatOutFile)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Text(pDX, IDC_TMCRTHFORMUOTF_NT,  m_csnT);
	DDX_Text(pDX, IDC_TMCRTHFORMUOTF_DT,  m_csdTcurrent);
	DDX_Text(pDX, IDC_TMCRTHFORMUOTF_NBL,  m_csnBlock);
	DDX_Text(pDX, IDC_TMCRTHFORMUOTF_INP,  m_csdInp);
	DDX_Text(pDX, IDC_TMCRTHFORMUOTF_OUT,  m_csdOut);
	DDX_Text(pDX, IDC_TMCRTHFORMUOTFIELD,  m_csdOutField);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTmcRTH_DialogFormatOutFile, CDialog)
	//{{AFX_MSG_MAP(CTmcRTH_DialogFormatOutFile)
	ON_BN_CLICKED(IDOKDefaultFormat, OnSetDefaultFormat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_DialogFormatOutFile message handlers

void CTmcRTH_DialogFormatOutFile::OnSetDefaultFormat() 
{
	// TODO: Add your control notification handler code here
	IsSetDefaultFormat = TRUE;
	CDialog::OnOK();
	return;
}
