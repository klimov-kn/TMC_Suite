// Global function for PlanRT_H programm
//

#include "stdafx.h"
#include "MainFrm.h"
#include <tmcgrviw.h>
#include "PlanRT_H.h"
#include <limits.h>
#include <math.h>
#include <conio.h>

#include "PlanRT_HDoc.h"
#include "PlanRT_HView.h"
#include "TmcDialogStatistics.h"
#include "TmcRTH_DialogFormatOutFile.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

UINT ReadDataGlobal( LPVOID pParam )
//__fastcall *ReadDataGlobal( LPVOID pParam )
{
	CPlanRT_HDoc* pDoc = (CPlanRT_HDoc *)(pParam);
	CString csBuf;

	if(	pDoc->IsReadData() ) return 0;
	pDoc->ReadData();
	
	return 0;
}
			  
UINT RunStepGlobal( LPVOID pParam )
//__fastcall *RunStepGlobal( LPVOID pParam )
{
	CPlanRT_HDoc* pDoc = (CPlanRT_HDoc *)(pParam);
	CString csBuf;

	if(	pDoc->IsReadData() ) return 0;
	if(	pDoc->IsRun() ) return 0;

	pDoc->RunStep();
	
	return 0;
}

UINT RunAllGlobal( LPVOID pParam )
//__fastcall *RunAllGlobal( LPVOID pParam )
{
	CPlanRT_HDoc* pDoc = (CPlanRT_HDoc *)(pParam);
	CString csBuf;

	if(	pDoc->IsReadData() ) return 0;
	if(	pDoc->IsRun() ) return 0;

	pDoc->RunAll();
	
	return 0;
}

void PutTrace( CString csTrace )
{
	CMainFrame *pFrame = ( CMainFrame * ) AfxGetApp()->m_pMainWnd;
	CStatusBar *pStatus = &pFrame->m_wndStatusBar;

	if( pStatus )
	{
		pStatus->SetPaneText( 0, csTrace );
	};

	pStatus->RedrawWindow();

	return;
}

void PutSinchronizFlag( BOOL bTopologyFlag, BOOL bFieldFlag1, BOOL bSinchFlag1 )
{
	CMainFrame *pFrame = ( CMainFrame * ) AfxGetApp()->m_pMainWnd;
	CStatusBar *pStatus = &pFrame->m_wndStatusBar;
	CString csTrace, csTrace0, csTrace1, csTrace2, csTrace3;
	
	csTrace1.Format("Output: Signal");
//	if( sizeof(_real) == 4 ) csTrace1.Format("Float; Output: Signal");
//	if( sizeof(_real) == 8 ) csTrace1.Format("Double; Output: Signal");


	if( bFieldFlag1 ) 
	{
		csTrace2.Format(" + Field");
		if( bSinchFlag1 ) csTrace3.Format(" + Sinchronization");
		if( bTopologyFlag ) csTrace0.Format(" + Topology");
	};

	csTrace = csTrace1 + csTrace0 + csTrace2 + csTrace3;
	
	if( pStatus )
	{

		pStatus->SetPaneText( 2, csTrace );
	};

	return;
}

void PutTrace( char *szTrace )
{
	CString csBuf;
	csBuf.Format("%s", szTrace);
	PutTrace( csBuf );
	return;
}

void PutStatistics( char *szTrace )
{
	CString csBuf;
	csBuf.Format("%s", szTrace);
	PutStatistics( csBuf );
	return;
}

BOOL IsPointInTr( double x1, double y1, double x2, double y2, double x3, double y3, double x0, double y0 )
{
/*
		   x1, y1		  x2, y2
		   ...............
            .           . 
             .     *   .
              . x0,y0 .
               .     .		  return TRUE
                .   .
                 . .
                  .
					x3, y3

          x1, y1		  x2, y2
		   ...............
            .           . 
             .         .
              .       .
      *        .     .		  return FALSE
   x0,y0        .   .
                 . .
                  .
					x3, y3

*/	
	double s120, s230, s310, s123, eps;

	s123 = fabs( (x2-x1)*(y3-y1) - (x3-x1)*(y2-y1) );
	s120 = fabs( (x1-x0)*(y2-y0) - (x2-x0)*(y1-y0) );
	s230 = fabs( (x2-x0)*(y3-y0) - (x3-x0)*(y2-y0) );
	s310 = fabs( (x3-x0)*(y1-y0) - (x1-x0)*(y3-y0) );

//	{
//		CString csBuf1;
//		csBuf1.Format("s120 = %lg; s230 = %lg; s310 = %lg; s123 = %lg;", s120, s230, s310, s123 );
//		AfxMessageBox(csBuf1);
//	};

	eps = s123*(1.e-10);
	if( (s120 + s230 + s310) > (s123 + eps) ) 
	{
//	{
//		CString csBuf1;
//		csBuf1.Format("FALSE x1 = %lg; y1 = %lg; x2 = %lg; y2 = %lg; x3 = %lg; y3 = %lg; x0 = %lg; y0 = %lg;", x1, y1, x2, y2, x3, y3, x0, y0 );
//		AfxMessageBox(csBuf1);
//	};
		return FALSE;
	};
	
//	{
//		CString csBuf1;
//		csBuf1.Format("TRUE x1 = %lg; y1 = %lg; x2 = %lg; y2 = %lg; x3 = %lg; y3 = %lg; x0 = %lg; y0 = %lg;", x1, y1, x2, y2, x3, y3, x0, y0 );
//		AfxMessageBox(csBuf1);
//	};
	return TRUE;
}

void PutStatistics( CString csTrace )
{
	CString csBuf;
	CMainFrame *pFrame = ( CMainFrame * ) AfxGetApp()->m_pMainWnd;
	CStatusBar *pStatus = &pFrame->m_wndStatusBar;


	if( pStatus )
	{
		pStatus->SetPaneText( 1, csTrace );
	};

	pStatus->RedrawWindow();

	return;
}


void PutModel( void )
{
	CString csTrace1;
	
	if( sizeof(_real) == 4 ) csTrace1.Format("Model: Float");
	if( sizeof(_real) == 8 ) csTrace1.Format("Model: Double");

	PutStatistics( csTrace1 );

	return;
}

CString GetModel( void )
{
	CString csTrace1;
	CString csTrace2;
#ifdef _VERSION_PROF___
	csTrace2.Format("Professional");
#endif
#ifdef _VERSION_DEMO___
	csTrace2.Format("Demonstration");
#endif
#ifdef _VERSION_EDUC___
	csTrace2.Format("Educational");
#endif
	
	if( sizeof(_real) == 4 ) csTrace1.Format("Float %s", csTrace2);
	if( sizeof(_real) == 8 ) csTrace1.Format("Double %s", csTrace2);
	
	return csTrace1;
}

static BOOL bIsSound = TRUE;
static BOOL bIsSysten_WNT = FALSE;
static BOOL bIsSysten_W95 = TRUE;
static int play_dat[][20]= {{  -20, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 00, 00, 00, 00, 00, 00},
							{ -100, 494, 466, 440, 415, 392, 370, 349, 330, 311, 294, 277, 262, 000, 00, 00, 00, 00, 00, 00}};

void SetMelody1Interval( int i )
{
	if( i < 1 ) i = 1;
	if( i > 60000 ) i = 60000;
	play_dat[0][0] = -i;
	return;
}
int GetMelody1Interval( void )
{
	return -play_dat[0][0];
}
void SetMelody1( int *i )
{
	if( i == NULL ) return;
	int j;
	for( j = 0; j < 12; j++ )
	{
		switch( i[j] )
		{
			case 0:
				play_dat[0][j+1] = 262;
				break;
			case 1:
				play_dat[0][j+1] = 277;
				break;
			case 2:
				play_dat[0][j+1] = 294;
				break;
			case 3:
				play_dat[0][j+1] = 311;
				break;
			case 4:
				play_dat[0][j+1] = 330;
				break;
			case 5:
				play_dat[0][j+1] = 349;
				break;
			case 6:
				play_dat[0][j+1] = 370;
				break;
			case 7:
				play_dat[0][j+1] = 392;
				break;
			case 8:
				play_dat[0][j+1] = 415;
				break;
			case 9:
				play_dat[0][j+1] = 440;
				break;
			case 10:
				play_dat[0][j+1] = 466;
				break;
			case 11:
				play_dat[0][j+1] = 494;
				break;
			case 12:
			default:
				play_dat[0][j+1] = 0;
				return;
				break;
		};
	};
	play_dat[0][j+1] = 0;
	return;
}
void GetMelody1( int *i )
{
	if( i == NULL ) return;
	int j;
	for( j = 0; j < 12; j++ )
	{
		switch( play_dat[0][j+1] )
		{
			case 262:
				i[j] = 0;
				break;
			case 277:
				i[j] = 1;
				break;
			case 294:
				i[j] = 2;
				break;
			case 311:
				i[j] = 3;
				break;
			case 330:
				i[j] = 4;
				break;
			case 349:
				i[j] = 5;
				break;
			case 370:
				i[j] = 6;
				break;
			case 392:
				i[j] = 7;
				break;
			case 415:
				i[j] = 8;
				break;
			case 440:
				i[j] = 9;
				break;
			case 466:
				i[j] = 10;
				break;
			case 494:
				i[j] = 11;
				break;
			case 12:
			default:
				i[j] = 12;
				break;
		};
	};
	return;
}

void SetMelody2Interval( int i )
{
	if( i < 1 ) i = 1;
	if( i > 60000 ) i = 60000;
	play_dat[1][0] = -i;
	return;
}
int GetMelody2Interval( void )
{
	return -play_dat[1][0];
}
void SetMelody2( int *i )
{
	if( i == NULL ) return;
	int j;
	for( j = 0; j < 12; j++ )
	{
		switch( i[j] )
		{
			case 0:
				play_dat[1][j+1] = 262;
				break;
			case 1:
				play_dat[1][j+1] = 277;
				break;
			case 2:
				play_dat[1][j+1] = 294;
				break;
			case 3:
				play_dat[1][j+1] = 311;
				break;
			case 4:
				play_dat[1][j+1] = 330;
				break;
			case 5:
				play_dat[1][j+1] = 349;
				break;
			case 6:
				play_dat[1][j+1] = 370;
				break;
			case 7:
				play_dat[1][j+1] = 392;
				break;
			case 8:
				play_dat[1][j+1] = 415;
				break;
			case 9:
				play_dat[1][j+1] = 440;
				break;
			case 10:
				play_dat[1][j+1] = 466;
				break;
			case 11:
				play_dat[1][j+1] = 494;
				break;
			case 12:
			default:
				play_dat[1][j+1] = 0;
				return;
				break;
		};
	};
	play_dat[1][j+1] = 0;
	return;
}
void GetMelody2( int *i )
{
	if( i == NULL ) return;
	int j;
	for( j = 0; j < 12; j++ )
	{
		switch( play_dat[1][j+1] )
		{
			case 262:
				i[j] = 0;
				break;
			case 277:
				i[j] = 1;
				break;
			case 294:
				i[j] = 2;
				break;
			case 311:
				i[j] = 3;
				break;
			case 330:
				i[j] = 4;
				break;
			case 349:
				i[j] = 5;
				break;
			case 370:
				i[j] = 6;
				break;
			case 392:
				i[j] = 7;
				break;
			case 415:
				i[j] = 8;
				break;
			case 440:
				i[j] = 9;
				break;
			case 466:
				i[j] = 10;
				break;
			case 494:
				i[j] = 11;
				break;
			case 12:
			default:
				i[j] = 12;
				break;
		};
	};
	return;
}


void ReadSystemType( void )
{
	OSVERSIONINFO VersionInformation;

	VersionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if( GetVersionEx( &VersionInformation ) )
	{
		switch( VersionInformation.dwPlatformId )
		{
			case VER_PLATFORM_WIN32_WINDOWS:
				bIsSound = TRUE;
				bIsSysten_WNT = FALSE;
				bIsSysten_W95 = TRUE;
				break;
			case VER_PLATFORM_WIN32_NT:
				bIsSound = TRUE;
				bIsSysten_WNT = TRUE;
				bIsSysten_W95 = FALSE;
				break;
			default:
				bIsSound = FALSE;
				bIsSysten_WNT = FALSE;
				bIsSysten_W95 = FALSE;
				break;
		};
	}
	else
	{
		bIsSound = FALSE;
		bIsSysten_WNT = FALSE;
		bIsSysten_W95 = FALSE;
	}
	return;
}

void BeepStepEnd( int i )
{
	i = 0;
	if( !bIsSound ) return;
	s_alarm( 0 );
	return;
}

void BeepAllEnd( void )
{
	if( !bIsSound ) return;
	s_alarm( 1 );
	return;
}


void s_alarm( int code )
 {
  if( !bIsSound ) return;
  s_play( &play_dat[code][0] );
  }


void s_play( int far *ptr )
 {
  if( !bIsSound ) return;
  register int wait = 0;

  while( *ptr )
   {
    if( *ptr < 0 ) wait = -*ptr;
     else s_tone( *ptr, wait );

    ptr++;
    }
  }

void s_tone( int freq, int time )
 {
  if( !bIsSound ) return;

  int port_buff;
  unsigned int divisor;

  if( freq != 0 )
   {
    if( bIsSysten_WNT )
	{
		Beep( freq, time);
		return;
	};
    if( bIsSysten_W95 )
	{
		// Историческая ветка Windows 95: звук через прямой доступ к портам в/в
		// (таймер 8253, порт 0x42/0x43, порт динамика 0x61) интринсиками _outp/_inp.
		// Современный MSVC удалил _inp/_outp из <conio.h>, а на x64 прямой доступ
		// к портам невозможен (привилегированные инструкции). На любой ОС, способной
		// запустить эту сборку (Windows 7+), система — NT, и сюда управление не доходит
		// (ветка bIsSysten_WNT выше делает Beep и return). Оставлен резервный Beep.
		(void)divisor; (void)port_buff;
		Beep( freq, time );
		return;
	};
  };
  return;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

