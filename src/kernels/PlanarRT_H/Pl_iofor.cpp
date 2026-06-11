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


void Set_CommandLine_Flags( char *lpszCmdLine )
{
	int i;
	
	SetAutoRunOff();
	SetDemoRunOff();
	
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


