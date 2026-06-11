// TmcErrorMessage.cpp : implementation file
//

#include "stdafx.h"
#include "PlanRT_H.h"
#include "TmcErrorMessage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcErrorMessage dialog


CTmcErrorMessage::CTmcErrorMessage(CWnd* pParent /*=NULL*/)
	: CDialog(CTmcErrorMessage::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTmcErrorMessage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTmcErrorMessage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTmcErrorMessage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTmcErrorMessage, CDialog)
	//{{AFX_MSG_MAP(CTmcErrorMessage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTmcErrorMessage message handlers
