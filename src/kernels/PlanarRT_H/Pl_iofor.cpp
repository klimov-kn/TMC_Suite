// Global function for PlanRT_H programm
//

/// @file Pl_iofor.cpp
/// @brief Форматы чисел выходных файлов, режимы запуска ядра, код завершения и консольный вывод.
///
/// Часть модуля «Счётное ядро H» (тот же файл — в ядре X). Описание функций —
/// в `pl_iofor.h`. Состояние хранится в статических переменных процесса.
/// @see https://www.tamic.ru/docs/code/kernel-h.html#flags
#include "StdAfx.h"
#include "MainFrm.h"
#include <Tmcgrviw.h>
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

/// @name Текущие форматы вывода (по умолчанию — `FORMAT_OUT_FILE_*`)
///@{
/// Формат номера такта.
static char	 szNTFormat[MAX_LEN_FORMAT_OUT_FILE] = {FORMAT_OUT_FILE_NTFORMAT};
/// Формат времени подтакта.
static char	 szDTCurrentFormat[MAX_LEN_FORMAT_OUT_FILE] = {FORMAT_OUT_FILE_DTCurrentFORMAT};
/// Формат номера блока.
static char	 szNBFormat[MAX_LEN_FORMAT_OUT_FILE] = {FORMAT_OUT_FILE_NBFORMAT};
/// Формат падающей волны.
static char	 szDInpFormat[MAX_LEN_FORMAT_OUT_FILE] = {FORMAT_OUT_FILE_DINPFORMAT};
/// Формат отражённой волны.
static char	 szDOutFormat[MAX_LEN_FORMAT_OUT_FILE] = {FORMAT_OUT_FILE_DOUTFORMAT};
/// Формат значений поля и топологии (пусто — двоичный вывод).
static char	 szDFieldOutFormat[MAX_LEN_FORMAT_OUT_FILE] = {FORMAT_OUT_FILE_DFLDOUTFORMAT};
///@}
/// Режим автозапуска (/Ar, /Arb, /d).
static volatile BOOL	 bAutoRunFlag = FALSE;
/// Демонстрационный режим (/d).
static volatile BOOL	 bDemoRunFlag = FALSE;
/// Пакетный режим (/Arb, /d).
static volatile BOOL	 bBatchRunFlag = FALSE;
/// Отчёт о времени расчёта (/Time).
static volatile BOOL	 bTimeReportFlag = FALSE;	// TIMING: key /Time
/// Сведения о запущенном внешнем процессе.
static PROCESS_INFORMATION piProcInfo;
/// Главное окно (не владеет).
static CMainFrame* HwndMainFrame = NULL;
/// Вид документа ядра для демонстрационного режима (не владеет).
static CPlanRT_HView* pcView = NULL;

// T-4, S-15: the format from the settings goes straight to fprintf. It is accepted only
// with exactly one conversion of the needed kind ('i' - integer: d i u x X o; 'f' - double:
// e E f F g G, optional 'l'), flags -+ #0, width and precision of at most 2 digits, other
// text without '%' (except "%%"), shorter than the buffer. Otherwise the default is used.
/// @brief Проверяет строку формата вывода перед передачей в fprintf.
/// @param[in] psz      строка формата
/// @param[in] cKind    'i' — нужно целое преобразование, 'f' — вещественное
/// @param[in] bEmptyOk TRUE — пустая строка допустима (двоичный вывод поля)
/// @return TRUE — формат допустим.
static BOOL IsGoodOutFormat( const char *psz, char cKind, BOOL bEmptyOk )
{
	const char *p;
	int nConv = 0, nD;

	if( psz == NULL ) return FALSE;
	if( strlen( psz ) >= MAX_LEN_FORMAT_OUT_FILE ) return FALSE;
	if( psz[0] == '\0' ) return bEmptyOk;
	for( p = psz; *p != '\0'; p++ )
	{
		if( *p != '%' ) continue;
		p++;
		if( *p == '%' ) continue;
		while( ( *p == '-' )||( *p == '+' )||( *p == ' ' )||( *p == '#' )||( *p == '0' ) ) p++;
		for( nD = 0; ( *p >= '0' )&&( *p <= '9' ); p++ ) if( ++nD > 2 ) return FALSE;
		if( *p == '.' )
		{
			p++;
			for( nD = 0; ( *p >= '0' )&&( *p <= '9' ); p++ ) if( ++nD > 2 ) return FALSE;
		};
		if( cKind == 'i' )
		{
			if( ( *p == '\0' )||( strchr( "diuxXo", *p ) == NULL ) ) return FALSE;
		}
		else
		{
			if( *p == 'l' ) p++;
			if( ( *p == '\0' )||( strchr( "eEfFgG", *p ) == NULL ) ) return FALSE;
		};
		nConv++;
	};
	return ( nConv == 1 ) ? TRUE : FALSE;
}

void SetnTFormat( CString csBuf )
{
	if( !IsGoodOutFormat( csBuf, 'i', FALSE ) ) { strcpy( szNTFormat, FORMAT_OUT_FILE_NTFORMAT ); return; }	// T-4
	strcpy( szNTFormat, csBuf );
	return;
}

char *GetnTFormat( void )
{
	return szNTFormat;
}

void SetdTcurrentFormat( CString csBuf )
{
	if( !IsGoodOutFormat( csBuf, 'f', FALSE ) ) { strcpy( szDTCurrentFormat, FORMAT_OUT_FILE_DTCurrentFORMAT ); return; }	// T-4
	strcpy( szDTCurrentFormat, csBuf );
	return;
}

char *GetdTcurrentFormat( void )
{
	return szDTCurrentFormat;
}

void SetnBFormat( CString csBuf )
{
	if( !IsGoodOutFormat( csBuf, 'i', FALSE ) ) { strcpy( szNBFormat, FORMAT_OUT_FILE_NBFORMAT ); return; }	// T-4
	strcpy( szNBFormat, csBuf );
	return;
}

char *GetnBFormat( void )
{
	return szNBFormat;
}

void SetdInpFormat( CString csBuf )
{
	if( !IsGoodOutFormat( csBuf, 'f', FALSE ) ) { strcpy( szDInpFormat, FORMAT_OUT_FILE_DINPFORMAT ); return; }	// T-4
	strcpy( szDInpFormat, csBuf );
	return;
}

char *GetdInpFormat( void )
{
	return szDInpFormat;
}

void SetdOutFormat( CString csBuf )
{
	if( !IsGoodOutFormat( csBuf, 'f', FALSE ) ) { strcpy( szDOutFormat, FORMAT_OUT_FILE_DOUTFORMAT ); return; }	// T-4
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
	if( !IsGoodOutFormat( csBuf, 'f', TRUE ) ) { strcpy( szDFieldOutFormat, FORMAT_OUT_FILE_DFLDOUTFORMAT ); return; }	// T-4
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

/// @brief Ищет в командной строке ключ /Time (или -Time) отдельным словом вне кавычек.
/// @param[in] lpszCmdLine командная строка
/// @return TRUE — ключ есть.
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
/// Код завершения процесса (`TMC_EXIT_*`).
static volatile int	 nRunExitCode = TMC_EXIT_OK;
/// Главное окно для закрытия после автозапуска.
static HWND		 hRunMainWnd = NULL;
/// TRUE — окно уже закрывается, повторный WM_CLOSE не посылать.
static volatile BOOL	 bRunClosing = FALSE;
/// TRUE — консоль уже искали (`ConsoleInit`).
static BOOL		 bConsoleInit = FALSE;
/// Куда писать консольные сообщения; NULL — некуда.
static HANDLE		 hConsoleOut = NULL;
/// TRUE — вывод в настоящую консоль (пишется UTF-16), иначе в файл или канал.
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

// The warnings of a run are shown in the status line only for a moment (the next progress message
// overwrites them) - the last one is kept and shown again at the end of the run.
/// Последнее предупреждение расчёта.
static char szRunWarning[1024] = "";

void SetRunWarning( const char *pszText )
{
	if( pszText == NULL ) pszText = "";
	strncpy( szRunWarning, pszText, sizeof(szRunWarning) - 1 );
	szRunWarning[sizeof(szRunWarning) - 1] = '\0';
	return;
}

const char *GetRunWarning( void )
{
	return szRunWarning;
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

/// @brief Находит, куда писать консольные сообщения: перенаправленный stdout (файл, канал)
///        или консоль родительского процесса.
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
	
	// A-15: klyuch - otdelnoe slovo vne kavychek: /Ar, /Arb, /d (takzhe -Ar, \Ar ...)
	BOOL bQuote = FALSE;
	const char *k;
	for( i = 0; lpszCmdLine[i] != '\0'; i++ )
	{
		if( lpszCmdLine[i] == '"' )
		{
			bQuote = !bQuote;
			continue;
		};
		if( bQuote ) continue;
		if( ( lpszCmdLine[i] != '/' )&&( lpszCmdLine[i] != '-' )&&( !TMC_IS_SEP(lpszCmdLine[i]) ) ) continue;
		if( ( i > 0 )&&( lpszCmdLine[i-1] != ' ' )&&( lpszCmdLine[i-1] != '\t' ) ) continue;
		k = lpszCmdLine + i + 1;
		// T-6: the keys are not case sensitive (/arb = /Arb), as /Time and /GPU
		if( ( _strnicmp( k, "Ar", 2 ) == 0 )&&( ( k[2] == '\0' )||( k[2] == ' ' )||( k[2] == '\t' ) ) )
		{
			SetAutoRunOn();
		}
		else if( ( _strnicmp( k, "Arb", 3 ) == 0 )&&( ( k[3] == '\0' )||( k[3] == ' ' )||( k[3] == '\t' ) ) )
		{
			SetAutoRunOn();
			SetBatchRunOn();
		}
		else if( ( ( k[0] == 'd' )||( k[0] == 'D' ) )&&( ( k[1] == '\0' )||( k[1] == ' ' )||( k[1] == '\t' ) ) )
		{
			SetAutoRunOn();
			SetBatchRunOn();
			SetDemoRunOn();
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



BOOL IsBatchRun( void )
{
	return bBatchRunFlag;
}

void SetProcessInformation( PROCESS_INFORMATION piProcInfo1 )
{
	piProcInfo = piProcInfo1;
	return;
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


