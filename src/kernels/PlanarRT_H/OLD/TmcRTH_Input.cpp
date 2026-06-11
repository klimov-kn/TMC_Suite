// TmcRTH_Input.cpp : implementation file
//

#include "stdafx.h"
#include "PlanRT_H.h"
#include "TmcRTH_Input.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_Input

CTmcRTH_Input::CTmcRTH_Input()
{
	pcInputNode = NULL;
	nType = 0;
	nNode = 0;
	rX = (_real)(0.0);
	rY = (_real)(0.0);
	return;
}

CTmcRTH_Input::~CTmcRTH_Input()
{
	DeleteData();
	return;
}


BEGIN_MESSAGE_MAP(CTmcRTH_Input, CWnd)
	//{{AFX_MSG_MAP(CTmcRTH_Input)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_Input message handlers

void CTmcRTH_Input::DeleteData( void )
{
	if( pcInputNode != NULL ) delete []pcInputNode;
	pcInputNode = NULL;
	nType = 0;
	nNode = 0;
	rX = (_real)(0.0);
	rY = (_real)(0.0);
	return;
}
