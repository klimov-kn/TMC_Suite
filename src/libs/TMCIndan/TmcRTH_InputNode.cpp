// TmcRTH_InputNode.cpp : implementation file
//

#include "stdafx.h"
#include <TmcRTH_InputNode.h>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_InputNode

CTmcRTH_InputNode::CTmcRTH_InputNode()
{
	rUf = (_real)(0.0);
	rUs = (_real)(0.0);
	rX = (_real)(0.0);
	rY = (_real)(0.0);
	rT = (_real)(0.0);
	return;
}

CTmcRTH_InputNode::~CTmcRTH_InputNode()
{
	return;
}


//BEGIN_MESSAGE_MAP(CTmcRTH_InputNode, CWnd)
	//{{AFX_MSG_MAP(CTmcRTH_InputNode)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_InputNode message handlers
