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
	m_hRunThread = NULL;
	return;
}

CPlanRT_HDoc::~CPlanRT_HDoc()
{
	if( m_hRunThread != NULL )
	{
		::CloseHandle( m_hRunThread );
		m_hRunThread = NULL;
	};
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

	sprintf( szCommandLine, "\"%s\"", lpszNameExe);
	i = (int)( strlen( szCommandLine ) );

	theString = GetPathName();
	if( strlen(theString) == 0 )
	{
		theString = GetTitle();
	};
	
	szCommandLine[i] = ' ';
	sprintf( szCommandLine + i + 1, "\"%s\"", (LPCSTR)( theString ));

	for( i = 0; ( i < N_BUF_MAX )&&( szCommandLine[i] == ' ' ); i++)
	{
		;
	}

	return CreatProc( szCommandLine + i );
}

#ifdef _WIN32
// T-33: the full path of a program given without a folder: next to the kernel, the system
// folder, the Windows folder, "App Paths" of the registry, PATH - but never the current folder
// (that is the task folder, a file put there must not start). FALSE - not found.
static BOOL TmcFindProgram( const char *pszName, char *pszFull, DWORD nMax )
{
	char szDir[MAX_PATH + 1], szName[MAX_PATH + 8];
	const char *pExt;
	DWORD n;
	HKEY hKey;
	int k;

	if( ( strlen( pszName ) == 0 )||( strlen( pszName ) >= MAX_PATH ) ) return FALSE;
	strcpy( szName, pszName );
	pExt = strrchr( szName, '.' );
	if( pExt == NULL ) strcat( szName, ".exe" );
	if( _stricmp( szName, "FLDVIEW.EXE" ) == 0 ) strcpy( szName, "FieldView.exe" );	// the old name of the field viewer
	for( k = 0; k < 3; k++ )
	{
		szDir[0] = '\0';
		if( k == 0 )
		{
			n = GetModuleFileNameA( NULL, szDir, MAX_PATH );
			if( ( n == 0 )||( n >= MAX_PATH ) ) continue;
			char *pS = strrchr( szDir, '\\' );
			if( pS == NULL ) continue;
			pS[0] = '\0';
		};
		if( k == 1 ) { n = GetSystemDirectoryA( szDir, MAX_PATH ); if( ( n == 0 )||( n >= MAX_PATH ) ) continue; };
		if( k == 2 ) { n = GetWindowsDirectoryA( szDir, MAX_PATH ); if( ( n == 0 )||( n >= MAX_PATH ) ) continue; };
		if( strlen( szDir ) + strlen( szName ) + 2 > nMax ) continue;
		sprintf( pszFull, "%s\\%s", szDir, szName );
		if( GetFileAttributesA( pszFull ) != INVALID_FILE_ATTRIBUTES ) return TRUE;
	};
	for( k = 0; k < 2; k++ )
	{
		char szKey[MAX_PATH + 80];
		DWORD dwType, dwSize = nMax;
		sprintf( szKey, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%s", szName );
		if( RegOpenKeyExA( ( k == 0 ) ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey ) != ERROR_SUCCESS ) continue;
		LONG lRes = RegQueryValueExA( hKey, NULL, NULL, &dwType, (LPBYTE)pszFull, &dwSize );
		RegCloseKey( hKey );
		if( ( lRes != ERROR_SUCCESS )||( ( dwType != REG_SZ )&&( dwType != REG_EXPAND_SZ ) )||( dwSize >= nMax ) ) continue;
		pszFull[dwSize] = '\0';
		if( pszFull[0] == '"' )
		{
			memmove( pszFull, pszFull + 1, strlen( pszFull ) );
			char *pQ = strchr( pszFull, '"' );
			if( pQ != NULL ) *pQ = '\0';
		};
		if( GetFileAttributesA( pszFull ) != INVALID_FILE_ATTRIBUTES ) return TRUE;
	};
	{
		const char *pszPath = getenv( "PATH" );
		char *pFile;
		if( ( pszPath != NULL )&&( SearchPathA( pszPath, szName, NULL, nMax, pszFull, &pFile ) > 0 )&&
			( strlen( pszFull ) < nMax ) ) return TRUE;
	};
	return FALSE;
}
#endif

BOOL CPlanRT_HDoc::CreatProc(char * lpszComLine)
{
	char szCurrentDir[N_BUF_MAX];
	int i;
	PROCESS_INFORMATION	piProcInfo;
	BOOL ii;
	BOOL bDirFound = FALSE;	// N-10

	piProcInfo.hProcess = NULL;

	strncpy( szCurrentDir, GetPathName(), N_BUF_MAX - 1 );
	szCurrentDir[N_BUF_MAX - 1] = '\0';
	for( i = (int)strlen(szCurrentDir)-1; i >= 0; i-- )
	{
		if( szCurrentDir[i] == '\\' )
		{
			szCurrentDir[i] = '\0';
			i = -1;
			bDirFound = TRUE;
		};
	};
#ifdef _WIN32
	char szCmdFull[N_BUF_MAX];
	{	// T-33: the program is started by its full path
		const char *p = lpszComLine;
		char szProg[MAX_PATH + 8], szFull[MAX_PATH + 8];
		int n = 0;
		while( ( *p == ' ' )||( *p == '\t' ) ) p++;
		if( *p == '"' )
		{
			for( p++; ( *p != '"' )&&( *p != '\0' )&&( n < MAX_PATH ); p++ ) szProg[n++] = *p;
			if( *p == '"' ) p++;
		}
		else
		{
			for( ; ( *p != ' ' )&&( *p != '\t' )&&( *p != '\0' )&&( n < MAX_PATH ); p++ ) szProg[n++] = *p;
		};
		szProg[n] = '\0';
		if( strpbrk( szProg, "\\/:" ) == NULL )
		{
			if( !TmcFindProgram( szProg, szFull, sizeof(szFull) ) ) return FALSE;
			if( _snprintf( szCmdFull, N_BUF_MAX - 1, "\"%s\"%s", szFull, p ) < 0 ) return FALSE;
			szCmdFull[N_BUF_MAX - 1] = '\0';
			lpszComLine = szCmdFull;
		};
	};
#endif

   if( piProcInfo.hProcess != NULL ) 
   {
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
      FALSE,         // N-10: handles are not inherited (the open .t of the step and others)
      0,             // creation flags
      NULL,          // use parent's environment
      bDirFound ? szCurrentDir : NULL,	// N-10: the task folder, if the path has one
      &siStartInfo,  // STARTUPINFO pointer
      &piProcInfo);  // receives PROCESS_INFORMATION
	if( IsDemoRun() )
	{
		SetProcessInformation(piProcInfo);
	}
#ifdef _WIN32
	else if( ii )	// N-10: the handles are not needed any more
	{
		if( piProcInfo.hThread != NULL ) CloseHandle( piProcInfo.hThread );
		if( piProcInfo.hProcess != NULL ) CloseHandle( piProcInfo.hProcess );
	};
#else
	;
#endif
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
		cRectNode.GetOutput()->PutSmatrixZero( cRectNode.GetTopology().GetBlockList() );	// D-1: zeros in the .s
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

	
	return;
}

void CPlanRT_HDoc::SetRunThreadHandle( HANDLE hThread )
{
	if( m_hRunThread != NULL )
	{
		::CloseHandle( m_hRunThread );
		m_hRunThread = NULL;
	};
	if( hThread != NULL )
	{
		::DuplicateHandle( ::GetCurrentProcess(), hThread,
			::GetCurrentProcess(), &m_hRunThread,
			0, FALSE, DUPLICATE_SAME_ACCESS );
	};
	return;
}

void CPlanRT_HDoc::StopAndWait( void )
{
	SetRunClosing();		// RUN STATUS: the window is closing - no WM_CLOSE from the run thread
	Stop();
	if( m_hRunThread == NULL ) return;
	DWORD dwStart = ::GetTickCount();
	for( ;; )
	{
		DWORD dwRes = ::MsgWaitForMultipleObjects( 1, &m_hRunThread, FALSE, 100, QS_ALLINPUT );
		if( dwRes == WAIT_OBJECT_0 ) break;
		if( dwRes == WAIT_OBJECT_0 + 1 )
		{
			MSG msg;
			while( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
			{
				::TranslateMessage( &msg );
				::DispatchMessage( &msg );
			};
		};
		if( ::GetTickCount() - dwStart > 30000 )	// S-13: the document is not destroyed while its thread works
		{
			PutTrace( "Waiting for the calculation thread to stop..." );
			dwStart = ::GetTickCount();
		};
	};
	::CloseHandle( m_hRunThread );
	m_hRunThread = NULL;
	return;
}

void CPlanRT_HDoc::RunAll( void )
{
	// TIMING: the whole "Run all" is measured; with the key /Time of the command line
	// the report is saved next to the .tpl (without the key - window "Calc. time..." only)
	if( bIsReadData ) return;
	cRectNode.GetTiming().BeginRunAll();
	// RUN STATUS: an exception in the run (first of all "not enough memory": the old code
	// checks new == NULL, but new throws) is not a crash 0xC0000409 any more, but the code 2
	try
	{
		RunAll1();
	}
	catch( CException *e )
	{
		char szErr[512] = "";
		e->GetErrorMessage( szErr, sizeof(szErr) );
		e->Delete();
		ConsolePrint( "ERROR in calculation (step %d): exception: %s", nStep, szErr );
		PutTrace( "Error: exception in calculation" );
		SetRunExitCode( TMC_EXIT_CALC_ERROR );
		bIsReadData = FALSE;
	}
	catch( std::bad_alloc & )
	{
		ConsolePrint( "ERROR in calculation (step %d): not enough memory", nStep );
		PutTrace( "Error: not enough memory" );
		SetRunExitCode( TMC_EXIT_CALC_ERROR );
		bIsReadData = FALSE;
	}
	catch( ... )
	{
		ConsolePrint( "ERROR in calculation (step %d): unknown exception", nStep );
		PutTrace( "Error: exception in calculation" );
		SetRunExitCode( TMC_EXIT_CALC_ERROR );
		bIsReadData = FALSE;
	};
	cRectNode.GetTiming().EndRunAll();
	if( IsTimeReport() ) cRectNode.GetTiming().SaveReport( cRectNode.GetTiming().DefaultReportName() );
	// RUN STATUS: result to the console; in the auto run the window is closed after
	// the successful run (always, not only with the sound on) and in /Arb - after any end
	if( IsAutoRun() )
	{
		ConsoleReportResult();
		if( ( GetRunExitCode() == TMC_EXIT_OK )||IsBatchRun() ) PostRunClose();
	};
	return;
}

void CPlanRT_HDoc::RunAll1( void )
{
	CString csBuf;
	if( bIsReadData ) return;

	bIsReadData = TRUE;

	int i, n;
	int nErrStep = 0;		// D-1: steps calculated with an error
	SetRunWarning( "" );		// the warnings of this run
	CString csErrFirst, csErrSteps;

	__int64 qTmPr = TmcTimingNow();								// TIMING
	n = cRectNode.GetNStepMax( GetPathName() ) + 1;
	cRectNode.GetTiming().qRunPrepr += TmcTimingNow() - qTmPr;		// TIMING

	// RUN STATUS: the preprocessor error or no #STEP in the .tpl - nothing to calculate
	if( cRectNode.IsError() )
	{
		ConsolePrint( "ERROR in .tpl: %s", (LPCTSTR)cRectNode.GetErrorMessage() );
		SetRunExitCode( TMC_EXIT_TPL_ERROR );
		bIsReadData = FALSE;
		return;
	};
	if( n - 1 < nStep )
	{
		ConsolePrint( "ERROR in .tpl: no #STEP to calculate (steps found: %d)", n - 1 );
		SetRunExitCode( TMC_EXIT_TPL_ERROR );
		bIsReadData = FALSE;
		return;
	};
	ConsolePrint( "Steps: %d", n - 1 );

	for( i = nStep; i < n; i++ )
	{
		nStep = i;

		if( !bIsReadData )
		{
			csBuf.Format("User cancel run step %d thread", nStep);
			PutTrace(csBuf);
			ConsolePrint( "STOPPED before step %d of %d", nStep, n - 1 );	// RUN STATUS
			SetRunExitCode( TMC_EXIT_ABORTED );
			bIsReadData = FALSE;
			return;
		};
		cRectNode.ReadData( GetPathName(), nStep );
		if( cRectNode.IsError() )
		{
			ConsolePrint( "ERROR in .tpl (step %d of %d): %s", nStep, n - 1,	// RUN STATUS
				(LPCTSTR)cRectNode.GetErrorMessage() );
			SetRunExitCode( TMC_EXIT_TPL_ERROR );
			bIsReadData = FALSE;
			return;
		};

		csBuf.Format("Start step %d", nStep);
		PutTrace(csBuf);
		ConsolePrint( "Step %d of %d: start", nStep, n - 1 );			// RUN STATUS

		if( !bIsReadData )
		{
			csBuf.Format("User cancel run step %d thread", nStep);
			PutTrace(csBuf);
			ConsolePrint( "STOPPED at step %d of %d", nStep, n - 1 );		// RUN STATUS
			SetRunExitCode( TMC_EXIT_ABORTED );
			bIsReadData = FALSE;
			return;
		};
		cRectNode.RunStep();
		if( cRectNode.IsError() )
		{
			// D-1: the sweep is not broken off - the S-matrix of this step is zeros in the .s, the message goes to
			// the console and the status line, the next step is calculated; the code 2 at the end.
			// Old: ConsolePrint(...); SetRunExitCode( TMC_EXIT_CALC_ERROR ); bIsReadData = FALSE; return;
			int nZero = cRectNode.GetOutput()->PutSmatrixZero( cRectNode.GetTopology().GetBlockList() );
			ConsolePrint( "ERROR in calculation (step %d of %d, %lg %s): %s%s", nStep, n - 1,	// RUN STATUS
				cRectNode.GetParam().rFreq/cRectNode.GetParam().rFreqUnit, (LPCTSTR)cRectNode.GetParam().csFreqUnit,
				(LPCTSTR)cRectNode.GetErrorMessage(),
				( nZero > 0 ) ? " - the S-matrix of this step is zeros in the .s" :
				( ( nZero < 0 ) ? " - zeros could not be written to the .s" : "" ) );
			csBuf.Format( "Error at step %d (%lg %s): %s", nStep,
				cRectNode.GetParam().rFreq/cRectNode.GetParam().rFreqUnit, (LPCTSTR)cRectNode.GetParam().csFreqUnit,
				(LPCTSTR)cRectNode.GetErrorMessage() );
			PutTrace( csBuf );
			if( nErrStep == 0 ) csErrFirst = csBuf;
			if( nErrStep < 10 )
			{
				CString csS;
				csS.Format( "%s%d (%lg %s)", ( nErrStep > 0 ) ? ", " : "", nStep,
					cRectNode.GetParam().rFreq/cRectNode.GetParam().rFreqUnit, (LPCTSTR)cRectNode.GetParam().csFreqUnit );
				csErrSteps += csS;
			};
			nErrStep++;
			if( !bIsReadData )
			{
				ConsolePrint( "STOPPED at step %d of %d", nStep, n - 1 );		// RUN STATUS
				SetRunExitCode( TMC_EXIT_ABORTED );
				return;
			};
			continue;
		};
		if( !bIsReadData )
		{
			csBuf.Format("User cancel run step %d thread", nStep);
			PutTrace(csBuf);
			ConsolePrint( "STOPPED at step %d of %d", nStep, n - 1 );		// RUN STATUS
			SetRunExitCode( TMC_EXIT_ABORTED );
			bIsReadData = FALSE;
			return;
		};

		csBuf.Format("Finish step %d", nStep);
		PutTrace(csBuf);
		ConsolePrint( "Step %d of %d: done", nStep, n - 1 );			// RUN STATUS
	};

	if( nErrStep > 0 )	// D-1: all steps are done, some with an error
	{
		csBuf.Format( "Calculated with errors at %d step(s): %s%s - the S-matrix there is zeros. First: %s",
			nErrStep, (LPCTSTR)csErrSteps, ( nErrStep > 10 ) ? ", ..." : "", (LPCTSTR)csErrFirst );
		ConsolePrint( "%s", (LPCTSTR)csBuf );
		PutTrace( csBuf );
		SetRunExitCode( TMC_EXIT_CALC_ERROR );
		bIsReadData = FALSE;
		return;
	};
	if( GetRunWarning()[0] != '\0' ) PutTrace( GetRunWarning() );	// the warning again at the end (it was overwritten)
	SetRunExitCode( TMC_EXIT_OK );									// RUN STATUS
	bIsReadData = FALSE;

	return;
}

BOOL CPlanRT_HDoc::IsRun( void )
{
	return cRectNode.IsRunStep();
}


BOOL CPlanRT_HDoc::RunExeFile(char *lpszNameExe, char *lpszArgCommLine)
{
	char szCommandLine[N_BUF_MAX];
	int i;
	CString theString;

	sprintf( szCommandLine, "\"%s\"", lpszNameExe);
	i = (int)( strlen( szCommandLine ) );

	theString.Format( "%s", lpszArgCommLine);
	if( strlen(theString) == 0 )
	{
		theString = GetTitle();
	};
	
	szCommandLine[i] = ' ';
	sprintf( szCommandLine + i + 1, "\"%s\"", (LPCSTR)( theString ));

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

//	CString csBuf1;
	Stop();
	
	CDocument::OnCloseDocument();
}

BOOL CPlanRT_HDoc::CanCloseFrame(CFrameWnd* pFrame) 
{
	// TODO: Add your specialized code here and/or call the base class


	if( (IsReadData())||(IsRun()) )
	{
		return FALSE;
	};

	
	return CDocument::CanCloseFrame(pFrame);
}

CTmcRTHRectNode & CPlanRT_HDoc::GetRectNode()
{
	return cRectNode;
}

BOOL CPlanRT_HDoc::RunViewerDirectPattern()
{
	FILE *fd;


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
