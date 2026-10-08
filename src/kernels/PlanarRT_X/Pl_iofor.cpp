// Global function for PlanRT_H programm
//

#include "stdafx.h"
#include "MainFrm.h"
#include <tmcgrviw.h>
#include "PlanRT_H.h"
#include <limits.h>
#include <math.h>

#include "PlanRT_HDoc.h"
#include "PlanRT_HView.h"
#include "TmcDialogStatistics.h"
#include "TmcRTH_DialogFormatOutFile.h"
#include "Pl_iofor.h"
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

static char	 szNTFormat[MAX_LEN_FORMAT_OUT_FILE] = {FORMAT_OUT_FILE_NTFORMAT};
static char	 szDTCurrentFormat[MAX_LEN_FORMAT_OUT_FILE] = {FORMAT_OUT_FILE_DTCurrentFORMAT};
static char	 szNBFormat[MAX_LEN_FORMAT_OUT_FILE] = {FORMAT_OUT_FILE_NBFORMAT};
static char	 szDInpFormat[MAX_LEN_FORMAT_OUT_FILE] = {FORMAT_OUT_FILE_DINPFORMAT};
static char	 szDOutFormat[MAX_LEN_FORMAT_OUT_FILE] = {FORMAT_OUT_FILE_DOUTFORMAT};
static char	 szDFieldOutFormat[MAX_LEN_FORMAT_OUT_FILE] = {FORMAT_OUT_FILE_DFLDOUTFORMAT};
static volatile BOOL	 bAutoRunFlag = FALSE;
static volatile BOOL	 bDemoRunFlag = FALSE;
static volatile BOOL	 bBatchRunFlag = FALSE;
static volatile BOOL	 bTimeReportFlag = FALSE;	// TIMING: key /Time
static PROCESS_INFORMATION piProcInfo;
static CMainFrame* HwndMainFrame = NULL;
static CPlanRT_HView* pcView = NULL;

void SetnTFormat( CString csBuf )
{
	if( csBuf.GetLength() > MAX_LEN_FORMAT_OUT_FILE ) return;
	strcpy( szNTFormat, csBuf );
	return;
}

char *GetnTFormat( void )
{
	return szNTFormat;
}

void SetdTcurrentFormat( CString csBuf )
{
	if( csBuf.GetLength() > MAX_LEN_FORMAT_OUT_FILE ) return;
	strcpy( szDTCurrentFormat, csBuf );
	return;
}

char *GetdTcurrentFormat( void )
{
	return szDTCurrentFormat;
}

void SetnBFormat( CString csBuf )
{
	if( csBuf.GetLength() > MAX_LEN_FORMAT_OUT_FILE ) return;
	strcpy( szNBFormat, csBuf );
	return;
}

char *GetnBFormat( void )
{
	return szNBFormat;
}

void SetdInpFormat( CString csBuf )
{
	if( csBuf.GetLength() > MAX_LEN_FORMAT_OUT_FILE ) return;
	strcpy( szDInpFormat, csBuf );
	return;
}

char *GetdInpFormat( void )
{
	return szDInpFormat;
}

void SetdOutFormat( CString csBuf )
{
	if( csBuf.GetLength() > MAX_LEN_FORMAT_OUT_FILE ) return;
	strcpy( szDOutFormat, csBuf );
	return;
}

char *GetdOutFormat( void )
{
	return szDOutFormat;
}

char *GetdFieldOutFormat( void )
{
	return szDFieldOutFormat;
}

void SetdFieldOutFormat( CString csBuf )
{
	if( csBuf.GetLength() > MAX_LEN_FORMAT_OUT_FILE ) return;
	strcpy( szDFieldOutFormat, csBuf );
	return;
}

void SetDefaultOutputFormat( void )
{
	strcpy( szNTFormat, FORMAT_OUT_FILE_NTFORMAT );
	strcpy( szDTCurrentFormat, FORMAT_OUT_FILE_DTCurrentFORMAT );
	strcpy( szNBFormat, FORMAT_OUT_FILE_NBFORMAT );
	strcpy( szDInpFormat, FORMAT_OUT_FILE_DINPFORMAT );
	strcpy( szDOutFormat, FORMAT_OUT_FILE_DOUTFORMAT );
	strcpy( szDFieldOutFormat, FORMAT_OUT_FILE_DFLDOUTFORMAT );
	return;
}

void SetDemoRunOn( void )
{
	bDemoRunFlag = TRUE;
	return;
}

void SetDemoRunOff( void )
{
	bDemoRunFlag = FALSE;
	return;
}

void SetAutoRunOn( void )
{
	bAutoRunFlag = TRUE;
	return;
}

void SetAutoRunOff( void )
{
	bAutoRunFlag = FALSE;
	return;
}

void SwitchAutoRunFlag( void )
{
	bAutoRunFlag = !bAutoRunFlag;
	return;
}

BOOL IsAutoRun( void )
{
	return bAutoRunFlag;
}

BOOL IsDemoRun( void )
{
	return bDemoRunFlag;
}


// TIMING: key /Time (or -Time) of the command line: at the end of "Run all"
// the calculation time report <tpl>.timing_<H|X>_<win32|win64>.txt is written
// next to the .tpl (the same text as Statistics -> "Calc. time..." -> "Export to TXT").
// The key is a separate word: it is not searched inside "quoted" file names
// and inside words (a path like C:\Time\a.tpl or my-time.tpl is not a key).
void SetTimeReportOn( void )
{
	bTimeReportFlag = TRUE;
	return;
}

void SetTimeReportOff( void )
{
	bTimeReportFlag = FALSE;
	return;
}

BOOL IsTimeReport( void )
{
	return bTimeReportFlag;
}

static BOOL IsKeyTimeInCommandLine( const char *lpszCmdLine )
{
	int i;
	BOOL bQuote = FALSE;
	char c;

	for( i = 0; lpszCmdLine[i] != '\0'; i++ )
	{
		if( lpszCmdLine[i] == '"' )
		{
			bQuote = !bQuote;
			continue;
		};
		if( bQuote ) continue;
		if( ( lpszCmdLine[i] != '/' )&&( lpszCmdLine[i] != '-' ) ) continue;
		if( ( i > 0 )&&( lpszCmdLine[i-1] != ' ' )&&( lpszCmdLine[i-1] != '\t' ) ) continue;
		if( _strnicmp( lpszCmdLine + i + 1, "Time", 4 ) != 0 ) continue;
		c = lpszCmdLine[i+5];
		if( ( c == '\0' )||( c == ' ' )||( c == '\t' ) ) return TRUE;
	};
	return FALSE;
}

// RUN STATUS: exit code of the process and console output for /Ar, /Arb.
// Console messages are written only in the auto run mode: to the redirected
// stdout (file or pipe: "> log.txt", "| more", bash) or to the console of the
// parent process (cmd, bat). Started not from a console - nothing is written.
static volatile int	 nRunExitCode = TMC_EXIT_OK;
static HWND		 hRunMainWnd = NULL;
static volatile BOOL	 bRunClosing = FALSE;
static BOOL		 bConsoleInit = FALSE;
static HANDLE		 hConsoleOut = NULL;
static BOOL		 bConsoleIsChar = FALSE;

void SetRunExitCode( int nCode )
{
	nRunExitCode = nCode;
	return;
}

int GetRunExitCode( void )
{
	return nRunExitCode;
}

void SetRunMainWnd( HWND hWnd )
{
	hRunMainWnd = hWnd;
	return;
}

// the window is being closed (CPlanRT_HDoc::StopAndWait from OnClose): the run thread
// must not post one more WM_CLOSE - the nested OnClose destroyed the document
// while the first OnClose was still waiting for the thread
void SetRunClosing( void )
{
	bRunClosing = TRUE;
	return;
}

// close the main window after the auto run (by HWND: safe if it is already closed)
void PostRunClose( void )
{
	if( bRunClosing ) return;
	if( ( hRunMainWnd != NULL )&&( ::IsWindow( hRunMainWnd ) ) )
		::PostMessage( hRunMainWnd, WM_CLOSE, 0, 0 );
	return;
}

static void ConsoleInit( void )
{
	HANDLE h;
	DWORD dwType, dwMode;

	bConsoleInit = TRUE;
	h = ::GetStdHandle( STD_OUTPUT_HANDLE );
	dwType = ( ( h != NULL )&&( h != INVALID_HANDLE_VALUE ) ) ? ::GetFileType( h ) : FILE_TYPE_UNKNOWN;
	if( ( dwType == FILE_TYPE_DISK )||( dwType == FILE_TYPE_PIPE ) )
	{
		hConsoleOut = h;
		return;
	};
	if( !::AttachConsole( ATTACH_PARENT_PROCESS ) ) return;
	h = ::CreateFileA( "CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL );
	if( h == INVALID_HANDLE_VALUE ) return;
	hConsoleOut = h;
	bConsoleIsChar = ::GetConsoleMode( h, &dwMode );
	return;
}

void ConsolePrint( const char *pszFormat, ... )
{
	char szBuf[4096];
	int n, nPrefix;
	va_list args;
	DWORD dwWritten;

	if( !IsAutoRun() ) return;
	if( !bConsoleInit ) ConsoleInit();
	if( hConsoleOut == NULL ) return;

	nPrefix = _snprintf( szBuf, sizeof(szBuf) - 3, "[%s] ", AfxGetApp()->m_pszExeName );
	if( ( nPrefix < 0 )||( nPrefix >= (int)sizeof(szBuf) - 3 ) ) nPrefix = 0;
	va_start( args, pszFormat );
	n = _vsnprintf( szBuf + nPrefix, sizeof(szBuf) - 3 - nPrefix, pszFormat, args );
	va_end( args );
	if( ( n < 0 )||( n >= (int)sizeof(szBuf) - 3 - nPrefix ) ) n = (int)strlen( szBuf + nPrefix );
	n += nPrefix;
	szBuf[n++] = '\r';
	szBuf[n++] = '\n';
	szBuf[n] = '\0';

	if( bConsoleIsChar )
	{
		WCHAR wszBuf[4096];
		int nW = ::MultiByteToWideChar( CP_ACP, 0, szBuf, n, wszBuf, 4096 );
		if( nW > 0 ) ::WriteConsoleW( hConsoleOut, wszBuf, nW, &dwWritten, NULL );
	}
	else
	{
		::WriteFile( hConsoleOut, szBuf, n, &dwWritten, NULL );
	};
	return;
}

void ConsoleReportResult( void )
{
	switch( GetRunExitCode() )
	{
		case TMC_EXIT_OK:
			ConsolePrint( "RESULT %d: OK - all steps are calculated", TMC_EXIT_OK );
			break;
		case TMC_EXIT_TPL_ERROR:
			ConsolePrint( "RESULT %d: ERROR in .tpl file - calculation is not done", TMC_EXIT_TPL_ERROR );
			break;
		case TMC_EXIT_CALC_ERROR:
			ConsolePrint( "RESULT %d: ERROR during calculation", TMC_EXIT_CALC_ERROR );
			break;
		default:
			ConsolePrint( "RESULT %d: STOPPED - calculation is not finished", GetRunExitCode() );
			break;
	};
	return;
}

void Set_CommandLine_Flags( char *lpszCmdLine )
{
	int i;

	SetAutoRunOff();
	SetDemoRunOff();
	SetTimeReportOff();											// TIMING
	if( IsKeyTimeInCommandLine( lpszCmdLine ) ) SetTimeReportOn();	// TIMING: key /Time
	
	for( i = 0; lpszCmdLine[i] != '\0'; i++ )
	{
		switch( lpszCmdLine[i] )
		{
			case '-': ;
			case '/': ;
			case '\\': ;
				i--;
			case ' ': ;
				switch( lpszCmdLine[i+1] )
				{
					case '-': ;
					case '/': ;
					case '\\': ;
						switch( lpszCmdLine[i+2] )
						{
							case 'A':
								switch( lpszCmdLine[i+3] )
								{
									case 'r':
										SetAutoRunOn();
										switch( lpszCmdLine[i+4] )
										{
											case 'b':
												SetBatchRunOn();
												break;
											default:
												break;
										};
										break;
									default:
										break;
								};
								break;
							case 'd':
								SetAutoRunOn();
								SetBatchRunOn();
								SetDemoRunOn();
								break;
							default:
								break;
						};
						i++;
						break;
					default:
						break;
				};
				break;
			default:
				break;
		};
	};
	// RUN STATUS: in the auto run the code "stopped" until the run is finished
	SetRunExitCode( IsAutoRun() ? TMC_EXIT_ABORTED : TMC_EXIT_OK );
	return;
};

CMainFrame* GetMainFramePointer__( void )
{
	return HwndMainFrame;
};

void SetMainFramePointer__( CMainFrame* HwndMainFrame1 )
{
	HwndMainFrame = HwndMainFrame1;
	return;
};

void SetBatchRunOn( void )
{
	bBatchRunFlag = TRUE;
	return;
}

void SetBatchRunOff( void )
{
	bBatchRunFlag = FALSE;
	return;
}

void SwitchBatchRunFlag( void )
{
	bBatchRunFlag = !bBatchRunFlag;
	return;
}

BOOL IsBatchRun( void )
{
	return bBatchRunFlag;
}

void SetProcessInformation( PROCESS_INFORMATION piProcInfo1 )
{
	piProcInfo = piProcInfo1;
	return;
}

PROCESS_INFORMATION GetProcessInformation( void )
{
	return piProcInfo;
}

void SetPointerClassView( CPlanRT_HView* pcView1 )
{
	pcView = pcView1;
	return;
}

void RunOnViewField1( void )
{
	if( pcView == NULL ) return;
	pcView->OnViewField11();
	return;	
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


