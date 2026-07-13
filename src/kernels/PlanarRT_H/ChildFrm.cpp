// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "PlanRT_H.h"

#include "Pl_iofor.h"
#include "ChildFrm.h"
#include "PlanRT_HDoc.h"
#include "PlanRT_HView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here
	
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style = WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
		| FWS_ADDTOTITLE | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE;

	return CMDIChildWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

BOOL CChildFrame::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CMDIChildWnd::DestroyWindow();
}

CDocument* CChildFrame::GetActiveDocument() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CMDIChildWnd::GetActiveDocument();
}

void CChildFrame::OnClose() 
{
	CPlanRT_HDoc* pDoc = (CPlanRT_HDoc*)GetActiveDocument();
	if( pDoc != NULL )
	{
		if( pDoc->IsRun() && !IsBatchRun() )
		{
			if( AfxMessageBox( "Computation in progress. Stop and close?", MB_YESNO | MB_ICONWARNING ) != IDYES )
				return;
		};
		pDoc->StopAndWait();
	}


	CMDIChildWnd::OnClose();

	return;
}
