// PlanRT_H.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "TmcDocLink.h"
#include <afxadv.h>
#include "PlanRT_H.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "PlanRT_HDoc.h"
#include "PlanRT_HView.h"
#include "Pl_iofor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HApp

BEGIN_MESSAGE_MAP(CPlanRT_HApp, CWinApp)
	//{{AFX_MSG_MAP(CPlanRT_HApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HApp construction

CPlanRT_HApp::CPlanRT_HApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPlanRT_HApp object

CPlanRT_HApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HApp initialization

BOOL CPlanRT_HApp::InitInstance()
{
	// Инициализация OLE/COM — нужна для Shell-вызовов в CRecentFileList (Windows 7+).
	// Без неё CRecentFileList::Add бросает CInvalidArgException при открытии файла (Баг #6).
	AfxOleInit();

	// Хранить настройки/MRU в реестре HKCU, а не в %WINDIR%\*.INI
	// (на современной Windows системный каталог недоступен для записи).
	SetRegistryKey( "PlanarRT_X" );

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
//	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	LoadStdProfileSettings(7);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_PLANRTTYPE,
		RUNTIME_CLASS(CPlanRT_HDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CPlanRT_HView));
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open

	Set_CommandLine_Flags( theApp.m_lpCmdLine );

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
											   // if command line is NULL  - open last dat file
	if( strcmp( cmdInfo.m_strFileName, "" ) == 0 )
	{
		FILE *fd;
		int i;
		for( i = 0; i < m_pRecentFileList->m_nSize; i++)
		{
			if( strcmp( m_pRecentFileList->m_arrNames[i], "" ) != 0)
			{
				if( ( fd = fopen( m_pRecentFileList->m_arrNames[i], "r") ) != NULL )
				{
					fclose( fd );
					cmdInfo.ParseParam( m_pRecentFileList->m_arrNames[i], FALSE, 1);
					i = 10;
				};
			};
		};
	};


	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
					   
	// The main window has been initialized, so show and update it.
	{
		CString csBuffer;
//		CString csBuf1;
		CRect rect;
		WINDOWPLACEMENT sWndPl;
			// read main frame size and place
		csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_MNWNDSIZEPLACE_INI );
//		csBuf1.Format("{%s};", csBuffer );
//		AfxMessageBox(csBuf1);
		if( csBuffer.GetLength() != 0 )
		{
			if( sscanf( csBuffer, "%d %d %d %d", &(rect.left), &(rect.right), &(rect.top), &(rect.bottom)  ) == 4 )
			{
				sWndPl.length  = sizeof( WINDOWPLACEMENT );
				sWndPl.showCmd = SW_NORMAL;
				sWndPl.flags   = WPF_SETMINPOSITION;
				sWndPl.ptMinPosition = CPoint( 0, 0 );
				sWndPl.ptMaxPosition = CPoint( 
					-::GetSystemMetrics( SM_CXBORDER ),
					-::GetSystemMetrics( SM_CYBORDER ) );
				sWndPl.rcNormalPosition = rect;
				pMainFrame->SetWindowPlacement( &sWndPl );
			};
		};
	};

	pMainFrame->ShowWindow(SW_RESTORE);
//	pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);
	pMainFrame->UpdateWindow();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
	virtual BOOL OnInitDialog();
	afx_msg void OnTmcDocLink();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	CFont m_tmcLinkFont;

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_csModel;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CString csModel;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	csModel.Format("Model : %s", GetModel());
	//{{AFX_DATA_INIT(CAboutDlg)
	m_csModel = csModel;
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_RTHTMCABOUTBOXMODEL, m_csModel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_STN_CLICKED(IDC_TMC_DOCLINK, OnTmcDocLink)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	TmcDoc_InitLink(this, IDC_TMC_DOCLINK, m_tmcLinkFont);
	return TRUE;
}

void CAboutDlg::OnTmcDocLink()
{
	TmcDoc_Open(this, _T("kernels\\planar_rt_x.html"));
}

HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	TmcDoc_ColorLink(pDC, pWnd, IDC_TMC_DOCLINK);
	return hbr;
}

BOOL CAboutDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (pWnd != NULL && pWnd->GetDlgCtrlID() == IDC_TMC_DOCLINK)
	{
		::SetCursor(::LoadCursor(NULL, IDC_HAND));
		return TRUE;
	}
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

// App command to run the dialog
void CPlanRT_HApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HApp commands
