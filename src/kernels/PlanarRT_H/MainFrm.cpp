// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "PlanRT_H.h"

#include "MainFrm.h"
#include "PlanRT_HDoc.h"
#include "Pl_iofor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_STATUS_BAR, OnViewStatusBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STATUS_BAR, OnUpdateViewStatusBar)
	ON_WM_QUERYENDSESSION()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,           
	ID_SEPARATOR,
//	ID_SEPARATOR,
//	ID_SEPARATOR,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM, ID_MY_STATUS_BAR ) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	SetMainFramePointer__( this );
	
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	CString csBuffer;
//	DWORD dwStyle; 
// read main frame size and place
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_MNWNDSIZEPLACE_INI );
//	if( csBuffer.GetLength() != 0 )
//	{
//		sscanf( csBuffer, "%d%d%d%d%ld", &(cs.cx), &(cs.cy), &(cs.x), &(cs.y), &(dwStyle) );
//		cs.style = dwStyle;
//	}
//	else
//	{
		cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
			| WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE;
//	};

	return CMDIFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnViewStatusBar() 
{
	// TODO: Add your command handler code here
	m_wndStatusBar.ShowWindow( (m_wndStatusBar.GetStyle() & WS_VISIBLE) == 0 );
	RecalcLayout();
	return;	
}

void CMainFrame::OnUpdateViewStatusBar(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( (m_wndStatusBar.GetStyle() & WS_VISIBLE) != 0 );	
	return;	
}

BOOL CMainFrame::OnQueryEndSession() 
{
	if (!CMDIFrameWnd::OnQueryEndSession())
		return FALSE;
	
	// TODO: Add your specialized query end session code here
	
	return TRUE;
}

void CMainFrame::OnDestroy() 
{

	// TODO: Add your message handler code here
/*	char szBuffer[N_BUF_MAX];
	CString csBuf1;
	RECT Rect; 
	DWORD dwStyle = GetStyle(); 
// save main frame size and place
//	sprintf( szBuffer, "%d%d%d%d", cs.cx, cs.cy, cs.x, cs.y );
	csBuf1.Format("cx = %d; cy = %d; x = %d; y = %d;", Rect.right - Rect.left, Rect.bottom - Rect.top, Rect.left, Rect.top );
	AfxMessageBox(csBuf1);
//	sprintf( szBuffer, "%d%d%d%d%ld", Rect.right - Rect.left, -Rect.bottom + Rect.top, Rect.left, Rect.bottom, dwStyle  );
	sprintf( szBuffer, "%d%d%d%d%ld", 200, 300, 200, 300, dwStyle  );
	AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_MNWNDSIZEPLACE_INI, szBuffer);
*/	
	if( IsDemoRun() ) 
	{
		if( GetProcessInformation().hProcess != NULL ) 
		{
			TerminateProcess( GetProcessInformation().hProcess, 0 );
		}; 
	};

	WINDOWPLACEMENT sWndPl;
	sWndPl.length = sizeof( WINDOWPLACEMENT );
	if( GetWindowPlacement( &sWndPl ) )
	{
		if( sWndPl.showCmd == SW_SHOWNORMAL )
		{
			char szBuffer[N_BUF_MAX];
//			CString csBuf1;
			sprintf( szBuffer, "%d %d %d %d", sWndPl.rcNormalPosition.left, sWndPl.rcNormalPosition.right, sWndPl.rcNormalPosition.top, sWndPl.rcNormalPosition.bottom  );
//			csBuf1.Format("{%s};", szBuffer );
//			AfxMessageBox(csBuf1);
			AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_MNWNDSIZEPLACE_INI, szBuffer);
		};
	};

	CMDIFrameWnd::OnDestroy();
	return;
}
