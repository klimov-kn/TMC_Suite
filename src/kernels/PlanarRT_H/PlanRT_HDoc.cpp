// PlanRT_HDoc.cpp : implementation of the CPlanRT_HDoc class
//

#include "stdafx.h"
#include "PlanRT_H.h"
#include <tmcgrviw.h>
#include <afxmt.h>

#include "Pl_iofor.h"
#include "PlanRT_HDoc.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HDoc

IMPLEMENT_DYNCREATE(CPlanRT_HDoc, CDocument)

BEGIN_MESSAGE_MAP(CPlanRT_HDoc, CDocument)
	//{{AFX_MSG_MAP(CPlanRT_HDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HDoc construction/destruction

CPlanRT_HDoc::CPlanRT_HDoc()
{
	// TODO: add one-time construction code here
	csEditorName.Format("winword.exe");
	csExternViewer.Format("winword.exe");
	csExternViewerField.Format("winword.exe");
	nStep = 1;
	bIsReadData = FALSE;
	return;
}

CPlanRT_HDoc::~CPlanRT_HDoc()
{
}

BOOL CPlanRT_HDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HDoc serialization

void CPlanRT_HDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HDoc diagnostics

#ifdef _DEBUG
void CPlanRT_HDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPlanRT_HDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HDoc commands

BOOL CPlanRT_HDoc::RunExeFile( char *lpszNameExe )
{
	char szCommandLine[N_BUF_MAX];
	int i;
	CString theString;

//	strcpy( szCommandLine, lpszNameExe);
	sprintf( szCommandLine, "\"%s\"", lpszNameExe);
	i = strlen( szCommandLine );

	theString = GetPathName();
	if( strlen(theString) == 0 )
	{
		theString = GetTitle();
	};
	
	szCommandLine[i] = ' ';
//	strcpy( szCommandLine + i + 1, theString);
	sprintf( szCommandLine + i + 1, "\"%s\"", theString);

	for( i = 0; ( i < N_BUF_MAX )&&( szCommandLine[i] == ' ' ); i++)
	{
		;
	}

	return CreatProc( szCommandLine + i );
}

BOOL CPlanRT_HDoc::CreatProc(char * lpszComLine)
{
	char szCurrentDir[N_BUF_MAX];
	int i;
	PROCESS_INFORMATION	piProcInfo;
	BOOL ii;

	piProcInfo.hProcess = NULL;

	strcpy( szCurrentDir, GetPathName() );
	for( i = strlen(szCurrentDir)-1; i >= 0; i-- )
	{
		if( szCurrentDir[i] == '\\' )
		{
			szCurrentDir[i] = '\0';
			i = -1;
		};
	};

   if( piProcInfo.hProcess != NULL ) 
   {
//	TerminateProcess( piProcInfo.hProcess, 0 );
	piProcInfo.hProcess = NULL;
   }; 
   STARTUPINFO siStartInfo; 
// Set up members of STARTUPINFO structure. 
   ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
   siStartInfo.cb = sizeof(STARTUPINFO); 
// Create the child process. 
   ii = CreateProcess(NULL, 
      lpszComLine,       // command line 
      NULL,          // process security attributes 
      NULL,          // primary thread security attributes 
      TRUE,          // handles are inherited 
      0,             // creation flags 
      NULL,          // use parent's environment 
      szCurrentDir,          // use parent's current directory 
      &siStartInfo,  // STARTUPINFO pointer 
      &piProcInfo);  // receives PROCESS_INFORMATION 
	if( IsDemoRun() ) 
	{
		SetProcessInformation(piProcInfo);
	};
   return ii;
}



CString CPlanRT_HDoc::GetExternEditorName( void )
{
	return csEditorName;
}

void CPlanRT_HDoc::SetExternEditorName( CString csExternEditorName )
{
	csEditorName = csExternEditorName;
}

BOOL CPlanRT_HDoc::RunExeFile( CString csNameExe )
{
	char szNameExe[N_BUF_MAX];
	strcpy( szNameExe, csNameExe);
	return RunExeFile( szNameExe );
}

void CPlanRT_HDoc::ReadData( void )
{
	if( bIsReadData ) return;
	bIsReadData = TRUE;
	PutTrace("Read Data");
	cRectNode.ReadData( GetPathName(), nStep );
	bIsReadData = FALSE;
	return;
}

int CPlanRT_HDoc::GetnStep( void )
{
	return nStep;
}

void CPlanRT_HDoc::SetFirstStep( void  )
{
	if( bIsReadData ) return;
	nStep = 1;
	return;
}

void CPlanRT_HDoc::SkipStep( void )
{
	if( bIsReadData ) return;
	int i;
	bIsReadData = TRUE;
	i = cRectNode.GetNStepMax( GetPathName() );
	bIsReadData = FALSE;
	if( i > nStep ) nStep++;
	return;
}

void CPlanRT_HDoc::BackStep( void )
{
	if( bIsReadData ) return;
	nStep--;
	if( nStep < 1 ) nStep = 1;
	return;
}

CString CPlanRT_HDoc::GetErrorMessage( void )
{
	return cRectNode.GetErrorMessage();
}


CTmcRTH_IndanParam& CPlanRT_HDoc::GetParam( void )
{
	return cRectNode.GetParam();
}

CTmcRTH_IndanOutput* CPlanRT_HDoc::GetOutput( void )
{
	return cRectNode.GetOutput();
}

CTmcRTH_IndanTopology& CPlanRT_HDoc::GetTopology( void )
{
	return cRectNode.GetTopology();
}

BOOL CPlanRT_HDoc::IsReadData( void )
{
	return (bIsReadData||cRectNode.IsReadData());
}

void CPlanRT_HDoc::RunStep( void )
{
	CString csBuf;
	if( bIsReadData ) return;

	bIsReadData = TRUE;
	
	if( cRectNode.IsError() ) 
	{
		bIsReadData = FALSE;
		return;
	};
	csBuf.Format("Start step %d", nStep);
	PutTrace(csBuf);
	
	cRectNode.RunStep();
	if( cRectNode.IsError() ) 
	{
		bIsReadData = FALSE;
		return;
	};
	
	if( !bIsReadData )
	{
		csBuf.Format("User cancel run step %d thread", nStep);
		PutTrace(csBuf);
		bIsReadData = FALSE;
		return;
	};

	csBuf.Format("Finish step %d", nStep);
	PutTrace(csBuf);

	bIsReadData = FALSE;
	
	return;
}

void CPlanRT_HDoc::Stop( void )
{
//	CEvent cWait;
	bIsReadData = FALSE;
	
	cRectNode.Stop();

//	CloseAndExit();
	
	return;
}

void CPlanRT_HDoc::RunAll( void )
{
	CString csBuf;
	if( bIsReadData ) return;

	bIsReadData = TRUE;
	
	int i, n;
	
	n = cRectNode.GetNStepMax( GetPathName() ) + 1;
	
	for( i = nStep; i < n; i++ )
	{
		nStep = i;
		
		if( !bIsReadData )
		{
			csBuf.Format("User cancel run step %d thread", nStep);
			PutTrace(csBuf);
			bIsReadData = FALSE;
			return;
		};
		cRectNode.ReadData( GetPathName(), nStep );
		if( cRectNode.IsError() ) 
		{
			bIsReadData = FALSE;
			return;
		};
		
		csBuf.Format("Start step %d", nStep);
		PutTrace(csBuf);
		
		if( !bIsReadData )
		{
			csBuf.Format("User cancel run step %d thread", nStep);
			PutTrace(csBuf);
			bIsReadData = FALSE;
			return;
		};
		cRectNode.RunStep();
		if( cRectNode.IsError() ) 
		{
			bIsReadData = FALSE;
			return;
		};
		if( !bIsReadData )
		{
			csBuf.Format("User cancel run step %d thread", nStep);
			PutTrace(csBuf);
			bIsReadData = FALSE;
			return;
		};
		
		csBuf.Format("Finish step %d", nStep);
		PutTrace(csBuf);
	};
	
	bIsReadData = FALSE;
	
	return;
}

BOOL CPlanRT_HDoc::IsRun( void )
{
	return cRectNode.IsRunStep();
//	return (cRectNode.IsRunStep()||bIsReadData);
}

void CPlanRT_HDoc::CloseAndExit( void )
{
	for( ; cRectNode.IsRunStep(); ) ;
	for( ; IsReadData(); ) ;
	return;
}

BOOL CPlanRT_HDoc::RunExeFile(char *lpszNameExe, char *lpszArgCommLine)
{
	char szCommandLine[N_BUF_MAX];
	int i;
	CString theString;

//	strcpy( szCommandLine, lpszNameExe);
	sprintf( szCommandLine, "\"%s\"", lpszNameExe);
	i = strlen( szCommandLine );

	theString.Format( "%s", lpszArgCommLine);
	if( strlen(theString) == 0 )
	{
		theString = GetTitle();
	};
	
	szCommandLine[i] = ' ';
//	strcpy( szCommandLine + i + 1, theString);
	sprintf( szCommandLine + i + 1, "\"%s\"", theString);

	for( i = 0; ( i < N_BUF_MAX )&&( szCommandLine[i] == ' ' ); i++)
	{
		;
	}

	return CreatProc( szCommandLine + i );
}

BOOL CPlanRT_HDoc::RunExeFile(CString csNameExe, CString csArgCommLine)
{
	char szNameExe[N_BUF_MAX];
	char szArgCommLine[N_BUF_MAX];
	strcpy( szNameExe, csNameExe);
	strcpy( szArgCommLine, csArgCommLine);
	return RunExeFile( szNameExe, szArgCommLine );
}

void CPlanRT_HDoc::SetExternViewer(CString csExternViewer1)
{
	csExternViewer = csExternViewer1;
	return;
}

void CPlanRT_HDoc::SetExternViewerField(CString csExternViewerField1)
{
	csExternViewerField = csExternViewerField1;
	return;
}

void CPlanRT_HDoc::SetExternViewerSmatrix(CString csExternViewerField1)
{
	csExternViewerSmatrix = csExternViewerField1;
	return;
}

BOOL CPlanRT_HDoc::RunViewer()
{
	FILE *fd;
	if( ( fd = fopen( (cRectNode.GetOutput())->csFileName, "r") ) == NULL )
	{
		if( ( fd = fopen( (cRectNode.GetOutput())->csFileName, "w") ) != NULL )
			fclose( fd );
	}
	else
	{
		fclose( fd );
	};
	return RunExeFile( csExternViewer, (cRectNode.GetOutput())->csFileName);
}

CString CPlanRT_HDoc::GetExternViewer()
{
	return csExternViewer;
}

CString CPlanRT_HDoc::GetExternViewerField()
{
	return csExternViewerField;
}

CString CPlanRT_HDoc::GetExternViewerSmatrix()
{
	return csExternViewerSmatrix;
}

void CPlanRT_HDoc::OnOffFieldOutput()
{
	cRectNode.OnOffFieldOutput();
	return;
}

BOOL CPlanRT_HDoc::RunViewerField()
{
	FILE *fd;
	if( ( fd = fopen( (cRectNode.GetOutput())->csFileNameField, "r") ) == NULL )
	{
		if( ( fd = fopen( (cRectNode.GetOutput())->csFileNameField, "w") ) != NULL )
			fclose( fd );
	}
	else
	{
		fclose( fd );
	};
	return RunExeFile( csExternViewerField, (cRectNode.GetOutput())->csFileNameField);
}

BOOL CPlanRT_HDoc::RunViewerSmatrix()
{
	FILE *fd;
	if( ( fd = fopen( (cRectNode.GetOutput())->csFileNameSMatrix, "r") ) == NULL )
	{
		if( ( fd = fopen( (cRectNode.GetOutput())->csFileNameSMatrix, "w") ) != NULL )
			fclose( fd );
	}
	else
	{
		fclose( fd );
	};
	return RunExeFile( csExternViewerSmatrix, (cRectNode.GetOutput())->csFileNameSMatrix);
}

void CPlanRT_HDoc::OnOffSinchronizationFieldOutput()
{
	cRectNode.OnOffSinchronizationFieldOutput();
	return;
}

void CPlanRT_HDoc::SetOutSinchrFieldOutFlag(int i)
{
	cRectNode.GetOutput()->bOutFieldSinchronization = i;
	return;
}

void CPlanRT_HDoc::SetOutFieldOutFlag(int i)
{
	cRectNode.GetOutput()->bOutField = i;
	return;
}

void CPlanRT_HDoc::OnOffSoundEffects()
{
	cRectNode.OnOffSoundEffects();
	return;
}

void CPlanRT_HDoc::SetSoundFlag(int i)
{
	cRectNode.GetOutput()->bSoundEffect = i;
	return;
}

void CPlanRT_HDoc::PutTopology()
{
	cRectNode.PutTopol();
	return;
}

BOOL CPlanRT_HDoc::GetSinchronizationFieldOutput()
{
	return cRectNode.GetSinchronizationFieldOutput();
}

BOOL CPlanRT_HDoc::GetFieldFlag()
{
	return cRectNode.GetFieldFlag();
}

BOOL CPlanRT_HDoc::GetSoundEffect()
{
	return cRectNode.GetSoundEffect();
}

BOOL CPlanRT_HDoc::GetTopologyFlag()
{
	return cRectNode.GetTopologyFlag();
}

void CPlanRT_HDoc::OnCloseDocument() 
{
	// TODO: Add your specialized code here and/or call the base class

//{
//	CString csBuf1;
//	csBuf1.Format("It is impossible to close the document %s, while the process of calculations proceeds. Please stop calculations.", GetPathName() );
//	AfxMessageBox(csBuf1);
//};
	Stop();
//	CloseAndExit();
	
	CDocument::OnCloseDocument();
}

BOOL CPlanRT_HDoc::CanCloseFrame(CFrameWnd* pFrame) 
{
	// TODO: Add your specialized code here and/or call the base class

//	Stop();
//	CloseAndExit();

	if( (IsReadData())||(IsRun()) )
	{
		return FALSE;
	};

//	if( bIsReadData != TRUE ) return FALSE;
	
	return CDocument::CanCloseFrame(pFrame);
}

CTmcRTHRectNode & CPlanRT_HDoc::GetRectNode()
{
	return cRectNode;
}

BOOL CPlanRT_HDoc::RunViewerDirectPattern()
{
	FILE *fd;

//	((cRectNode.GetOutput())->GetFieldIntegrated()).ExportToDirectionalPattern(cRectNode.GetParam());

	if( ( fd = fopen( ((cRectNode.GetOutput())->GetFieldIntegrated()).GetcsDirectPatternFile(), "r") ) == NULL )
	{
		if( ( fd = fopen( ((cRectNode.GetOutput())->GetFieldIntegrated()).GetcsDirectPatternFile(), "w") ) != NULL )
			fclose( fd );
	}
	else
	{
		fclose( fd );
	};
	return RunExeFile( csExternViewerDirectPattern, ((cRectNode.GetOutput())->GetFieldIntegrated()).GetcsDirectPatternFile());
}

void CPlanRT_HDoc::SetExternViewerDirectionalpattern(CString csFileName)
{
	csExternViewerDirectPattern = csFileName;
	return;
}

CString CPlanRT_HDoc::GetExternViewerDirectPat()
{
	return csExternViewerDirectPattern;
}


void CPlanRT_HDoc::ExportToDirectionalPatter()
{
	cRectNode.ExportToDirectionalPatter();
	return;
}
