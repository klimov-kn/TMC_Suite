// TmcGrParColTypWid1.cpp : implementation file
//

#include "stdafx.h"
#include "TMCGROUT.h"
#include "TmcGrParColTypWid1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcGrParColTypWid1 dialog


CTmcGrParColTypWid1::CTmcGrParColTypWid1(CWnd* pParent /*=NULL*/)
	: CDialog(CTmcGrParColTypWid1::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTmcGrParColTypWid1)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTmcGrParColTypWid1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTmcGrParColTypWid1)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTmcGrParColTypWid1, CDialog)
	//{{AFX_MSG_MAP(CTmcGrParColTypWid1)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTmcGrParColTypWid1 message handlers
