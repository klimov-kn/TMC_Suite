// Global function for PlanRT_H programm
//

/// @file PL_GLFUN.CPP
/// @brief Служебные функции ядра: функции рабочих потоков, вывод в строку состояния,
///        проверка точки в треугольнике, звуковые сигналы.
///
/// Часть модуля «Счётное ядро H» (тот же файл — в ядре X). Функции объявлены
/// в общем заголовке `typerth.h` и описаны там.
/// @see https://www.tamic.ru/docs/code/kernel-h.html#threads
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

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

			  
/// @brief Функция потока: считает текущий шаг документа ядра (если не идёт чтение или расчёт).
/// @param[in] pParam указатель на документ ядра (`CPlanRT_HDoc*`)
/// @return Всегда 0.
/// @see https://www.tamic.ru/docs/code/kernel-h.html#threads
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

/// @brief Функция потока: считает все шаги задания документа ядра подряд.
/// @param[in] pParam указатель на документ ядра (`CPlanRT_HDoc*`)
/// @return Всегда 0.
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

/// @brief Выводит сообщение о ходе работы или ошибке в первую панель строки состояния.
/// @param[in] csTrace текст сообщения
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

/// @brief Показывает в третьей панели строки состояния, что выводится при расчёте.
/// @param[in] bTopologyFlag TRUE — вывод топологии
/// @param[in] bFieldFlag1   TRUE — вывод поля
/// @param[in] bSinchFlag1   TRUE — синхронный вывод поля
void PutSinchronizFlag( BOOL bTopologyFlag, BOOL bFieldFlag1, BOOL bSinchFlag1 )
{
	CMainFrame *pFrame = ( CMainFrame * ) AfxGetApp()->m_pMainWnd;
	CStatusBar *pStatus = &pFrame->m_wndStatusBar;
	CString csTrace, csTrace0, csTrace1, csTrace2, csTrace3;
	
	csTrace1.Format("Output: Signal");


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

/// @brief То же, что `PutTrace(CString)`, для строки C.
/// @param[in] szTrace текст сообщения, завершённый нулём
void PutTrace( const char *szTrace )
{
	CString csBuf;
	csBuf.Format("%s", szTrace);
	PutTrace( csBuf );
	return;
}

/// @brief То же, что `PutStatistics(CString)`, для строки C.
/// @param[in] szTrace текст, завершённый нулём
void PutStatistics( const char *szTrace )
{
	CString csBuf;
	csBuf.Format("%s", szTrace);
	PutStatistics( csBuf );
	return;
}

/// @brief Проверяет, лежит ли точка (x0, y0) внутри треугольника или на его границе
///        (сумма площадей трёх подтреугольников равна площади треугольника с допуском 1e-10).
/// @param[in] x1 X первой вершины
/// @param[in] y1 Y первой вершины
/// @param[in] x2 X второй вершины
/// @param[in] y2 Y второй вершины
/// @param[in] x3 X третьей вершины
/// @param[in] y3 Y третьей вершины
/// @param[in] x0 X проверяемой точки
/// @param[in] y0 Y проверяемой точки
/// @return TRUE — точка в треугольнике.
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

//		CString csBuf1;

	eps = s123*(1.e-10);
	if( (s120 + s230 + s310) > (s123 + eps) ) 
	{
//		CString csBuf1;
		return FALSE;
	};
	
//		CString csBuf1;
	return TRUE;
}

/// @brief Выводит строку статистики (ход счёта, параметры) во вторую панель строки состояния.
/// @param[in] csTrace текст
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


/// @brief Выводит в строку статистики точность расчёта («Model: Float» или «Model: Double»).
void PutModel( void )
{
	CString csTrace1;
	
	if( sizeof(_real) == 4 ) csTrace1.Format("Model: Float");
	if( sizeof(_real) == 8 ) csTrace1.Format("Model: Double");

	PutStatistics( csTrace1 );

	return;
}

/// @brief Возвращает строку «точность + вариант поставки», например «Double Professional».
/// @return Текст для окна «О программе».
CString GetModel( void )
{
	CString csTrace1;
	CString csTrace2;
	csTrace2.Format("Professional");
	
	if( sizeof(_real) == 4 ) csTrace1.Format("Float %s", csTrace2);
	if( sizeof(_real) == 8 ) csTrace1.Format("Double %s", csTrace2);
	
	return csTrace1;
}

/// TRUE — звуковые сигналы разрешены (по типу ОС, `ReadSystemType`).
static BOOL bIsSound = TRUE;
/// TRUE — ОС семейства NT: тон подаётся функцией Beep.
static BOOL bIsSysten_WNT = FALSE;
/// TRUE — ОС семейства Windows 95 (историческая ветка).
static BOOL bIsSysten_W95 = TRUE;
/// Мелодии: [0] — конец шага, [1] — конец расчёта. Отрицательный элемент — длительность
/// следующих тонов, мс; положительный — частота тона, Гц; 0 — конец.
static int play_dat[][20]= {{  -20, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 00, 00, 00, 00, 00, 00},
							{ -100, 494, 466, 440, 415, 392, 370, 349, 330, 311, 294, 277, 262, 000, 00, 00, 00, 00, 00, 00}};

/// @brief Задаёт длительность нот мелодии конца шага.
/// @param[in] i длительность, мс (1..60000)
void SetMelody1Interval( int i )
{
	if( i < 1 ) i = 1;
	if( i > 60000 ) i = 60000;
	play_dat[0][0] = -i;
	return;
}
/// @brief Возвращает длительность нот мелодии конца шага.
/// @return Длительность, мс.
int GetMelody1Interval( void )
{
	return -play_dat[0][0];
}
/// @brief Задаёт ноты мелодии конца шага.
/// @param[in] i 12 номеров нот: 0..11 — полутоны 262..494 Гц, 12 — конец мелодии
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
/// @brief Возвращает номера нот мелодии конца шага.
/// @param[out] i 12 номеров нот (0..11, 12 — нет ноты)
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

/// @brief Задаёт длительность нот мелодии конца расчёта.
/// @param[in] i длительность, мс (1..60000)
void SetMelody2Interval( int i )
{
	if( i < 1 ) i = 1;
	if( i > 60000 ) i = 60000;
	play_dat[1][0] = -i;
	return;
}
/// @brief Возвращает длительность нот мелодии конца расчёта.
/// @return Длительность, мс.
int GetMelody2Interval( void )
{
	return -play_dat[1][0];
}
/// @brief Задаёт ноты мелодии конца расчёта.
/// @param[in] i 12 номеров нот: 0..11 — полутоны 262..494 Гц, 12 — конец мелодии
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
/// @brief Возвращает номера нот мелодии конца расчёта.
/// @param[out] i 12 номеров нот (0..11, 12 — нет ноты)
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


/// @brief Определяет семейство ОС и по нему разрешает звук.
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

/// @brief Проигрывает мелодию конца шага.
/// @param[in] i не используется
void BeepStepEnd( int i )
{
	i = 0;
	if( !bIsSound ) return;
	s_alarm( 0 );
	return;
}

/// @brief Проигрывает мелодию конца всего расчёта.
void BeepAllEnd( void )
{
	if( !bIsSound ) return;
	s_alarm( 1 );
	return;
}


/// @brief Проигрывает мелодию по номеру.
/// @param[in] code 0 — конец шага, 1 — конец расчёта
void s_alarm( int code )
 {
  if( !bIsSound ) return;
  s_play( &play_dat[code][0] );
  }


/// @brief Проигрывает мелодию из массива `play_dat` (синхронно).
/// @param[in] ptr массив: отрицательное — длительность, мс; положительное — частота, Гц; 0 — конец
void s_play( int *ptr )
 {
  if( !bIsSound ) return;
  int wait = 0;

  while( *ptr )
   {
    if( *ptr < 0 ) wait = -*ptr;
     else s_tone( *ptr, wait );

    ptr++;
    }
  }

/// @brief Подаёт один тон функцией Beep (синхронно).
/// @param[in] freq частота, Гц; 0 — ничего
/// @param[in] time длительность, мс
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

