#include "MainFrm.h"
#include "PlanRT_HDoc.h"
#include "PlanRT_HView.h"


#ifndef _TMCRTH_IOFORMAT__H
#define _TMCRTH_IOFORMAT__H

#define MAX_LEN_FORMAT_OUT_FILE			100
#define FORMAT_OUT_FILE_NTFORMAT		"%9d"
#define FORMAT_OUT_FILE_DTCurrentFORMAT "%15.7g"
#define FORMAT_OUT_FILE_NBFORMAT		"%4d"
#define FORMAT_OUT_FILE_DINPFORMAT		"%15.7g"
#define FORMAT_OUT_FILE_DOUTFORMAT		"%15.7g"
#define FORMAT_OUT_FILE_DFLDOUTFORMAT	""

void SetnTFormat( CString csBuf );
char *GetnTFormat( void );
void SetdTcurrentFormat( CString csBuf );
char *GetdTcurrentFormat( void );
void SetnBFormat( CString csBuf );
char *GetnBFormat( void );
void SetdInpFormat( CString csBuf );
char *GetdInpFormat( void );
void SetdOutFormat( CString csBuf );
char *GetdOutFormat( void );
void SetDefaultOutputFormat( void );
char *GetdFieldOutFormat( void );
void SetdFieldOutFormat( CString csBuf );
void SetAutoRunOn( void );
void SetAutoRunOff( void );
BOOL IsAutoRun( void );
void SetDemoRunOn( void );
void SetDemoRunOff( void );
BOOL IsDemoRun( void );
void Set_CommandLine_Flags( char *lpszCmdLine );
void SwitchAutoRunFlag( void );
CMainFrame *GetMainFramePointer__( void );
void SetMainFramePointer__( CMainFrame* HwndMainFrame1 );
void SetBatchRunOn( void );
void SetBatchRunOff( void );
void SwitchBatchRunFlag( void );
BOOL IsBatchRun( void );
void SetTimeReportOn( void );	// TIMING: key /Time - calculation time report file
void SetTimeReportOff( void );
BOOL IsTimeReport( void );
void SetProcessInformation( PROCESS_INFORMATION piProcInfo1 );
PROCESS_INFORMATION GetProcessInformation( void );
void SetPointerClassView( CPlanRT_HView* pcView1 );
void RunOnViewField1( void );

// RUN STATUS: process exit code and console messages for the command line run
// (keys /Ar, /Arb). The exit code is returned by CPlanRT_HApp::ExitInstance.
#define TMC_EXIT_OK				0	// all steps of the .tpl are calculated
#define TMC_EXIT_TPL_ERROR		1	// .tpl not found or error in .tpl (preprocessor, data)
#define TMC_EXIT_CALC_ERROR		2	// error during the calculation of a step
#define TMC_EXIT_ABORTED		3	// stopped by the user / window closed before the end
void SetRunExitCode( int nCode );
int  GetRunExitCode( void );
void SetRunMainWnd( HWND hWnd );
void SetRunClosing( void );
void PostRunClose( void );
void ConsolePrint( const char *pszFormat, ... );
void ConsoleReportResult( void );

#endif