// TmcRTHRectNode.cpp : implementation file
//

/// @file TmcRTHRectNode.cpp
/// @brief Сетка X-ядра: подготовка шага `#STEP`, пересчёт eps, N, B, Y, sigma в коэффициенты
///        узлов, цикл во времени по 4 подтактам, вывод сигналов `.t`, полей и топологии.
///
/// Часть модуля «Счётное ядро X». Реализует `CTmcRTHRectNode` и служебные функции
/// файла `.t` шага (держится открытым всё время шага). Порядок вызовов на одном
/// подтакте: порты -> сохранение узлов портов -> рассеяние во всех внутренних узлах ->
/// восстановление узлов портов -> граничные условия блоков -> поле -> накопление для ДН.
/// @see https://www.tamic.ru/docs/code/kernel-x.html#time-loop
#include "StdAfx.h"
#include "PlanRT_H.h"
#include <Tmcgrviw.h>
#include "TmcRTHRectNode.h"
#include <malloc.h>
#include "Pl_iofor.h"
#include "TmcGpuX.h"	// GPU

/// Сетка на видеокарте на время `RunKernel` (одна на процесс; см. `tmcgpux.h`).
static CTmcGpuX g_cGpuX;	// GPU: the grid on the video card while RunKernel runs

// T-5, T-9: the .t file of the step.
// T-5: the file is kept open while the step runs; other programs may read it, but not
// write it (_SH_DENYWR): a second kernel with the same FILE gets an error instead of
// mixing its lines into the file. The bytes are the same as before (the same text mode
// and order); a line goes to the disk when the next one begins, at Stop and at the end.
// T-9: a busy file is waited for by the clock (20 s), not by 2000 tries of Sleep(10).
#include <errno.h>
#include <chrono>
#include <mutex>
#ifdef _WIN32
#include <share.h>
#endif

/// Сколько ждать занятый файл при открытии, мс.
#define TMC_OUTT_WAIT_MS	20000
/// Сколько файлов `.t` может быть открыто одновременно (таблица `s_aOutT`).
#define TMC_OUTT_MAX		16

/// Таблица открытых файлов `.t`: имя и поток; `fp` == NULL — свободная запись.
/// Запись таблицы открытых файлов `.t`: `csName` — полное имя файла, `fp` — открытый поток (NULL — свободна).
static struct { CString csName; /**< имя файла */ FILE *fp; /**< поток; NULL — запись свободна */ } s_aOutT[TMC_OUTT_MAX];
/// @var s_aOutT
/// Таблица открытых файлов `.t` шага (не больше `TMC_OUTT_MAX` записей).

/// Защищает таблицу `s_aOutT` (расчётный поток и поток интерфейса).
static std::mutex s_mxOutT;

/// @brief Возвращает монотонное время, мс (для ожидания занятого файла).
/// @return Миллисекунды от произвольного начала отсчёта.
static long long TmcNowMs( void )
{
	return (long long)std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now().time_since_epoch() ).count();
}

// T-9: open a file, a busy one is waited for at most 20 s; *pnErr - errno of the last try
/// @brief Открывает файл; если он занят другой программой — повторяет попытки до 20 с.
///
/// Ошибки, при которых ожидание не поможет (нет папки, неверное имя), возвращаются сразу.
/// @param[in]  pszName    имя файла
/// @param[in]  pszMode    режим fopen
/// @param[in]  bDenyWrite TRUE — запретить другим программам запись в файл (только Windows)
/// @param[out] pnErr      errno последней попытки; NULL — не нужен
/// @return Поток файла; NULL — не открыт.
FILE *TmcFopenWait( const char *pszName, const char *pszMode, BOOL bDenyWrite, int *pnErr )
{
	long long t0 = TmcNowMs();
	FILE *fp;
	for( ;; )
	{
		errno = 0;
#ifdef _WIN32
		fp = bDenyWrite ? _fsopen( pszName, pszMode, _SH_DENYWR ) : fopen( pszName, pszMode );
#else
		fp = fopen( pszName, pszMode );
#endif
		if( fp != NULL ) return fp;
		if( pnErr != NULL ) *pnErr = errno;
		if( ( errno != EACCES )&&( errno != EBUSY ) ) return NULL;	// no folder, bad name: waiting does not help
		if( TmcNowMs() - t0 >= TMC_OUTT_WAIT_MS ) return NULL;
		Sleep( 10 );
	};
}

// T-7, T-9: why a file can't be opened
/// @brief Составляет понятный текст ошибки открытия файла (длинный путь, файл занят, нет папки).
/// @param[in] pszName имя файла
/// @param[in] nErr    errno неудачной попытки
/// @return Текст ошибки (по-английски).
CString TmcOpenFailText( const char *pszName, int nErr )
{
	CString cs;
#ifdef _WIN32
	if( strlen( pszName ) >= 260 )
	{
		cs.Format( "Can't open file %s: the path is %d characters long, at most 259 are supported", pszName, (int)strlen( pszName ) );
		return cs;
	};
#endif
	if( ( nErr == EACCES )||( nErr == EBUSY ) )
		cs.Format( "Can't open file %s: the file is used by another program (another calculation with the same FILE?) or is read-only", pszName );
	else if( nErr == ENOENT )
		cs.Format( "Can't open file %s: no such folder", pszName );
	else
		cs.Format( "Can't open file %s: %s", pszName, strerror( nErr ) );
	return cs;
}

/// @brief Закрывает файл `.t` с этим именем, если он открыт в таблице.
/// @param[in] pszName имя файла `.t`
void TmcOutTClose( const char *pszName )
{
	int i;
	std::lock_guard<std::mutex> lk( s_mxOutT );
	for( i = 0; i < TMC_OUTT_MAX; i++ )
	{
		if( ( s_aOutT[i].fp == NULL )||( strcmp( s_aOutT[i].csName, pszName ) != 0 ) ) continue;
		fclose( s_aOutT[i].fp );
		s_aOutT[i].fp = NULL;
		s_aOutT[i].csName = "";
	};
	return;
}

/// @brief Возвращает открытый поток файла `.t` по имени.
/// @param[in] pszName имя файла `.t`
/// @return Поток; NULL — файл не открыт.
FILE *TmcOutTGet( const char *pszName )
{
	int i;
	std::lock_guard<std::mutex> lk( s_mxOutT );
	for( i = 0; i < TMC_OUTT_MAX; i++ )
		if( ( s_aOutT[i].fp != NULL )&&( strcmp( s_aOutT[i].csName, pszName ) == 0 ) ) return s_aOutT[i].fp;
	return NULL;
}

/// @brief Сбрасывает на диск буфер открытого файла `.t` (перед построением матрицы рассеяния).
/// @param[in] pszName имя файла `.t`
void TmcOutTFlush( const char *pszName )
{
	FILE *fp = TmcOutTGet( pszName );
	if( fp != NULL ) fflush( fp );
	return;
}

// T-5: open the .t of the step ("w" - the header, "a" - go on after Stop) and keep it open
/// @brief Открывает файл `.t` шага и заносит его в таблицу открытых файлов.
///
/// Файл открывается с запретом записи для других программ: второе ядро с тем же
/// именем файла получит ошибку, а не смешает строки.
/// @param[in]  pszName имя файла `.t`
/// @param[in]  pszMode "w" — новый файл с заголовком, "a" — дозапись после остановки
/// @param[out] csErr   текст ошибки, если файл не открыт
/// @return Поток; NULL — ошибка (текст в `csErr`).
/// @see https://www.tamic.ru/docs/code/kernel-x.html#t-file
FILE *TmcOutTOpen( const char *pszName, const char *pszMode, CString &csErr )
{
	int i, nErr = 0;
	FILE *fp;

	TmcOutTClose( pszName );
	if( ( fp = TmcFopenWait( pszName, pszMode, TRUE, &nErr ) ) == NULL )
	{
		csErr = TmcOpenFailText( pszName, nErr );
		return NULL;
	};
	std::lock_guard<std::mutex> lk( s_mxOutT );
	for( i = 0; i < TMC_OUTT_MAX; i++ )
	{
		if( s_aOutT[i].fp != NULL ) continue;
		s_aOutT[i].fp = fp;
		s_aOutT[i].csName = pszName;
		return fp;
	};
	fclose( fp );
	csErr.Format( "Can't open file %s: too many calculations at once", pszName );
	return NULL;
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcRTHRectNode

CTmcRTHRectNode::CTmcRTHRectNode()
{
	bIsReadData = FALSE;
	nTCurrent = 0;
	nTMax     = 0;
	dTCurrent = 0.0;
	dT        = 0.0;
	dDelta = 0.0;
	nX = 0;
	nY = 0;
	nNumNode = 0;
	
//	prUNode = NULL;			// array falling voltage 1 = V; 2 = ^; 3 = <; 4 = >
	prUNode1   = NULL;					// array voltage on unit with 1 takt 
//	prUNode2   = NULL;					// array voltage on unit with 2 takt  
	prYNode    = NULL;					// node admitance array
//	prYMulNode = NULL;					// node multiple admitance array
//	pnTypeNode = NULL;					// node type array

	nDiel = 0;
	pcNodeDiel = NULL;
	dXmin = 0.0;				// global parameter Xmin for rectangle
	dXmax = 0.0;				// global parameter Xmax for rectangle
	dYmin = 0.0;				// global parameter Ymin for rectangle
	dYmax = 0.0;				// global parameter Ymax for rectangle
	bIsRunStep1Run = FALSE;
	m_cTiming.csKernel = "PlanarRT_X (tmc_rtx.exe)";	// TIMING
	return;
}

CTmcRTHRectNode::~CTmcRTHRectNode()
{
	DeleteData();
}


BEGIN_MESSAGE_MAP(CTmcRTHRectNode, CWnd)
	//{{AFX_MSG_MAP(CTmcRTHRectNode)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTmcRTHRectNode message handlers

void CTmcRTHRectNode::DeleteData( void )
{
	dDelta = 0.0;
	nTCurrent = 0;
	nTMax     = 0;
	dTCurrent = 0.0;
	dT        = 0.0;
	nX = 0;
	nY = 0;
	dXmin = 0.0;				// global parameter Xmin for rectangle
	dXmax = 0.0;				// global parameter Xmax for rectangle
	dYmin = 0.0;				// global parameter Ymin for rectangle
	dYmax = 0.0;				// global parameter Ymax for rectangle
	nNumNode = 0;
	nDiel = 0;
	cError.Clear();
	PutTrace("");
	PutStatistics("");
	
	DeleteNodeArray();
	DeleteDielArray();

	return;
}

void CTmcRTHRectNode::ReadData( CString csFileName, int nStep1 )
{
	CString csBuf;
	__int64 qTm = TmcTimingNow();				// TIMING
	m_cTiming.BeginStep( nStep1 );				// TIMING
	m_cTiming.csTplFile = csFileName;			// TIMING
	DeleteData();
	m_cTiming.Add( TMCT_MEMFREE, qTm );			// TIMING
	int i;

	qTm = TmcTimingNow();						// TIMING
	cIndan.SetTplFileName(csFileName);
	if( cIndan.IsError() )
	{
		csBuf.Format("Error in prepr: %s", cIndan.GetErrorMessage());
		cError.PutErrorMessage( csBuf );
		csBuf.Format("Step:%d;%s", nStep1, cError.GetErrorMessage());
		PutTrace(csBuf);
		bIsReadData = FALSE;
		return;
	};

	i = GetNStepMax( csFileName );
	bIsReadData = TRUE;
	if( cIndan.IsError() )
	{
		csBuf.Format("Error: %s", cIndan.GetErrorMessage());
		cError.PutErrorMessage( csBuf );
		csBuf.Format("Step:%d;%s", nStep1, cError.GetErrorMessage());
		PutTrace(csBuf);
		bIsReadData = FALSE;
		return;
	};
	if( nStep1 > i )
	{
		csBuf.Format("Error: very big step number" );
		cError.PutErrorMessage( csBuf );
		csBuf.Format("Step:%d;%s", nStep1, cError.GetErrorMessage());
		PutTrace(csBuf);
		bIsReadData = FALSE;
		return;
	};
	
	SetStepNumber( nStep1 );
	m_cTiming.Add( TMCT_PREPR, qTm );			// TIMING
	qTm = TmcTimingNow();						// TIMING
	ReadParamSection();
	ReadTopologySection();
	ReadLinkListSection();
	ReadOutputSection();
	m_cTiming.Add( TMCT_READTPL, qTm );			// TIMING

	csBuf.Format("Step:%d;%s", nStep1, cError.GetErrorMessage());
	PutTrace(csBuf);
	PutSinchronizFlag( cIndan.GetOutput()->bOutTopology, cIndan.GetOutput()->bOutField, cIndan.GetOutput()->bOutFieldSinchronization );
	PutModel();

	bIsReadData = FALSE;
	return;
}

void CTmcRTHRectNode::ReadData( void )
{
	bIsReadData = TRUE;
	CString csBuf;
	__int64 qTm = TmcTimingNow();				// TIMING
	DeleteData();
	m_cTiming.Add( TMCT_MEMFREE, qTm );			// TIMING
	qTm = TmcTimingNow();						// TIMING
	ReadParamSection();
	ReadTopologySection();
	ReadLinkListSection();
	ReadOutputSection();
	m_cTiming.Add( TMCT_READTPL, qTm );			// TIMING

	csBuf.Format("Step:%d;%s", cIndan.GetnStep(), cError.GetErrorMessage());
	PutTrace(csBuf);
	PutSinchronizFlag( cIndan.GetOutput()->bOutTopology, cIndan.GetOutput()->bOutField, cIndan.GetOutput()->bOutFieldSinchronization );
	PutModel();
	bIsReadData = FALSE;
	return;
}


void CTmcRTHRectNode::RunStep( void )
{
	if( bIsRunStep1Run ) return;
	bIsRunStep1Run = TRUE;
	__int64 qTmStep = TmcTimingNow();			// TIMING
	m_cTiming.BeginRunStep();					// TIMING
	try	// A-37: isklyuchenie ne dolzhno ostavlyat obekt "v schete"
	{
		RunStep1();
	}
	catch( CException *e )
	{
		char szErr[512] = "";
		e->GetErrorMessage( szErr, sizeof(szErr) );
		e->Delete();
		CString csErr;
		csErr.Format( "Exception in calculation: %s", szErr[0] ? szErr : "not enough memory" );
		cError.PutErrorMessage( csErr );
	}
	catch( std::bad_alloc & )
	{
		cError.PutErrorMessage( "Not enough memory" );
	}
	catch( ... )
	{
		cError.PutErrorMessage( "Exception in calculation" );
	};
	TmcOutTClose( cIndan.GetOutput()->csFileName );	// T-5: the step is over (done, stopped or error) - the .t is closed
	m_cTiming.cur.nTDone = nTCurrent;			// TIMING
	m_cTiming.EndRunStep( ( cError.IsError() || cIndan.IsError() ) ? 4 :
		( ( ( nTMax > 0 )&&( nTCurrent >= nTMax ) ) ? 2 : 3 ), qTmStep );	// TIMING
	bIsRunStep1Run = FALSE;
	bIsReadData = FALSE;
	return;
}

CString CTmcRTHRectNode::GetErrorMessage( void )
{
	return cError.GetErrorMessage();
}

int CTmcRTHRectNode::GetNStepMax( CString csFileName )
{
	bIsReadData = TRUE;
	int i = 0;
	CString csBuf;
	DeleteData();
	
	i = cIndan.GetNStepMax( csFileName );

	if( cIndan.IsError() )
	{
		csBuf.Format("Error : %s", cIndan.GetErrorMessage());
		cError.PutErrorMessage( csBuf );
		bIsReadData = FALSE;
		return i;
	};
	
	bIsReadData = FALSE;
	return i;
}

void CTmcRTHRectNode::SetStepNumber( int nStep )
{
	CString csBuf;

	if( cIndan.IsError() ) return;
	
	cIndan.SetStepNumber( nStep );
	
	if( cIndan.IsError() )
	{
		csBuf.Format("Error : %s", cIndan.GetErrorMessage());
		cError.PutErrorMessage( csBuf );
		return;
	};
	
	return;
}

void CTmcRTHRectNode::ReadLinkListSection( void )
{
	CString csBuf;

	if( cIndan.IsError() ) return;
	
	cIndan.ReadLinkListSection();
	
	if( cIndan.IsError() )
	{
		csBuf.Format("Error : %s", cIndan.GetErrorMessage());
		cError.PutErrorMessage( csBuf );
		return;
	};
	
	return;
}

void CTmcRTHRectNode::ReadParamSection( void )
{
	CString csBuf;

	if( cIndan.IsError() ) return;
	
	cIndan.ReadParamSection();
	
	if( cIndan.IsError() )
	{
		csBuf.Format("Error : %s", cIndan.GetErrorMessage());
		cError.PutErrorMessage( csBuf );
		return;
	};
	cParam = GetParam();
	
	return;
}

void CTmcRTHRectNode::ReadTopologySection( void )
{
	CString csBuf;

	if( cIndan.IsError() ) return;
	
	cIndan.ReadTopologySection();
	
	if( cIndan.IsError() )
	{
		csBuf.Format("Error : %s", cIndan.GetErrorMessage());
		cError.PutErrorMessage( csBuf );
		return;
	};
	
	return;
}

void CTmcRTHRectNode::ReadOutputSection( void )
{
	CString csBuf;

	if( cIndan.IsError() ) return;
	
	cIndan.ReadOutputSection();
	
	if( cIndan.IsError() )
	{
		csBuf.Format("Error : %s", cIndan.GetErrorMessage());
		cError.PutErrorMessage( csBuf );
		return;
	};
	
	return;
}



CTmcRTH_IndanOutput* CTmcRTHRectNode::GetOutput( void )
{
	return cIndan.GetOutput();
}

CTmcRTH_IndanParam& CTmcRTHRectNode::GetParam( void )
{
	return cIndan.GetParam();
}

CTmcRTH_IndanTopology& CTmcRTHRectNode::GetTopology( void )
{
	return cIndan.GetTopology();
}

void CTmcRTHRectNode::Stop( void )
{
	bIsReadData = FALSE;
}

void CTmcRTHRectNode::InitKernel( void )
{
	if( IsError() ) return;
	if( !bIsReadData ) return;
	if( IsDataInit() ) return;

	CString csBuf;
	
	nTCurrent = 0;

	__int64 qTm = TmcTimingNow();				// TIMING
	DeleteOutputFile();
	m_cTiming.Add( TMCT_OUTFILEINIT, qTm );		// TIMING
	if( IsError() ) return;	// T-9: the .t can't be opened - no second wait in PutPortInfo

	// Время: число подтактов, начальный момент Tmin и шаг tau из #PARM.
	nTMax     = cIndan.nGetTmax();
	csBuf.Format("nTmax %d", nTMax);
	PutStatistics(csBuf);
	if( nTMax <= 0 )
	{
		csBuf.Format("Error : very long time interval ( Tmax-Tmin ) in %s section", CTMCRTH_INDANMET_PARAM);
		cError.PutErrorMessage( csBuf );
		return;
	};

	dTCurrent = cIndan.dGetTmin();
	csBuf.Format("dTcurrent %lg", dTCurrent);
	PutStatistics(csBuf);

	dT        = cIndan.dGetdT();
	csBuf.Format("dT %lg", dT );
	PutStatistics(csBuf);
	if( dT <= 0.0 )
	{
		csBuf.Format("Error : time discret dT(%lg) <= 0 ", dT);
		cError.PutErrorMessage( csBuf );
		return;
	};

	dDelta = cIndan.dGetDelta();
	csBuf.Format("dDelta %lg", dDelta );
	PutStatistics(csBuf);
	if( dDelta <= 0.0 )
	{
		csBuf.Format("Error : space discret dDelta(%lg) <= 0 ", dDelta);
		cError.PutErrorMessage( csBuf );
		return;
	};
	
	// Область: не меньше 10 шагов по каждой оси и не больше INT_MAX узлов по стороне.
	cIndan.GetXYMinMax( dXmin, dXmax, dYmin, dYmax );
	csBuf.Format("X:(%lg)-(%lg); Y:(%lg)-(%lg)", dXmin, dXmax, dYmin, dYmax );
	PutStatistics(csBuf);
	if( ( dXmax - dXmin ) < 10*dDelta )
	{
		csBuf.Format("Error : space discret dDelta(%lg) < 10*(X_max-X_min) ", dDelta);
		cError.PutErrorMessage( csBuf );
		return;
	};
	if( ( dYmax - dYmin ) < 10*dDelta )
	{
		csBuf.Format("Error : space discret dDelta(%lg) < 10*(Y_max-Y_min) ", dDelta);
		cError.PutErrorMessage( csBuf );
		return;
	};
	
	if( ( dXmax - dXmin )/dDelta > INT_MAX )
	{
		csBuf.Format("Error : very long X-size ( Xmax-Xmin ) in %s section", CTMCRTH_INDANMET_PARAM);
		cError.PutErrorMessage( csBuf );
		return;
	};
	if( ( dYmax - dYmin )/dDelta > INT_MAX )
	{
		csBuf.Format("Error : very long Y-size ( Ymax-Ymin ) in %s section", CTMCRTH_INDANMET_PARAM);
		cError.PutErrorMessage( csBuf );
		return;
	};

	cIndan.GetnXnY( nX, nY);
	csBuf.Format("nX:(%d); nY:(%d)", nX, nY );
	PutStatistics(csBuf);
	
	if( nX <= 1 )
	{
		csBuf.Format("Error : very long X-size ( Xmax-Xmin ) in %s section", CTMCRTH_INDANMET_PARAM);
		cError.PutErrorMessage( csBuf );
		return;
	};
	if( nY <= 1 )
	{
		csBuf.Format("Error : very long Y-size ( Ymax-Ymin ) in %s section", CTMCRTH_INDANMET_PARAM);
		cError.PutErrorMessage( csBuf );
		return;
	};

	// Общее число узлов: индекс 6n+5 массивов должен помещаться в TmcIdx с запасом.
	if( ( ((double)(nX))*((double)(nY)) ) > ((double)TMC_IDX_MAX)/8. )	// INT64
	{
		csBuf.Format("Error : very dimension XY-size ( X*Y ) in %s section", CTMCRTH_INDANMET_PARAM);
		cError.PutErrorMessage( csBuf );
		return;
	};
	nNumNode = ((TmcIdx)nX)*nY;	// INT64
	m_cTiming.cur.nX = nX; m_cTiming.cur.nY = nY; m_cTiming.cur.dNodes = ((double)nX)*((double)nY);	// TIMING
	m_cTiming.cur.nTMax = nTMax; m_cTiming.cur.dT = dT; m_cTiming.cur.dDelta = dDelta;	// TIMING
	csBuf.Format("nNumNode:(%.0f);", (double)nNumNode );	// INT64
	PutStatistics(csBuf);
	if( nNumNode <= 1 )
	{
		nNumNode = 0;
		csBuf.Format("Error : very big X-size and Y-size or delta is very small in %s section", CTMCRTH_INDANMET_PARAM);
		cError.PutErrorMessage( csBuf );
		return;
	};

	qTm = TmcTimingNow();						// TIMING
	// Накопитель распределения поля для ДН (контур, окно по времени).
	cIndan.GetOutput()->GetFieldIntegrated().InitAllFile( nX, nY, dDelta, dXmin, dYmin, cParam, dT );
	m_cTiming.Add( TMCT_DISTRINIT, qTm );			// TIMING

	// Накопитель ДН читает значение узла с шагом 6 ячеек на узел.
	cIndan.GetOutput()->GetFieldIntegrated().SetNodeStep( 6 );

	if( cIndan.GetOutput()->GetFieldIntegrated().GetError().IsError() )	// T-9: not erase an error set before
		cError = cIndan.GetOutput()->GetFieldIntegrated().GetError();


	// Массивы напряжений и коэффициентов, затем блоки и данные портов в заголовке .t.
	qTm = TmcTimingNow();						// TIMING
	InitNodeArray();
	m_cTiming.Add( TMCT_MEMINIT, qTm );			// TIMING
	if( IsError() ) 
	{
		DeleteNodeArray();
		nNumNode = 0;
		return;
	};
	
	InitDielArray();
	if( IsError() ) 
	{
		DeleteDielArray();
		DeleteNodeArray();
		nNumNode = 0;
		nDiel = 0;
		return;
	};
	PutPortInfo();
	
	return;
}

void CTmcRTHRectNode::RunKernel( void )
{
	if( IsError() ) return;
	if( !bIsReadData ) return;
	
	CString csBuf;
	int ii, nBuf;
    static time_t ltime_start, ltime_current;

	csBuf.Format("Run Step %d", cIndan.GetnStep() );
	PutTrace(csBuf);

	// Новый шаг — отсчёт времени с начала; продолжение после Stop — с прерванного такта.
	if( nTCurrent == 0 )
	{
		ii = 0;
		time( &ltime_start );
	}
	else
	{
		ii =  nTCurrent - ((int)(nTCurrent/100))*100;
		ltime_start = ltime_current - ltime_start;
		time( &ltime_current );
		ltime_start = ltime_current - ltime_start;
	};

	// Сетка переносится на видеокарту, если она выбрана и подходит; иначе счёт на процессоре.
	BOOL bGpu = g_cGpuX.Begin( prUNode1, prYNode, nX, nY, pcNodeDiel, nDiel ) ? TRUE : FALSE;	// GPU
	strncpy( m_cTiming.cur.szCompute, (LPCTSTR)g_cGpuX.csInfo, sizeof(m_cTiming.cur.szCompute) - 1 );	// GPU
	if( g_cGpuX.csInfo != "CPU" ) PutTrace( g_cGpuX.csInfo );	// GPU
	__int64 qTmLoop = TmcTimingNow();					// TIMING
	// Основной цикл: один проход — четыре подтакта (RunKernel1T увеличивает nTCurrent на 4).
	/// З-6 (автор 29.10): при нечётном числе подтактов считается на один подтакт больше заданного TIME — допустимо.
	/// З-3 (автор 29.10): вход внутри области — общий проход переписывает узлы входа, и в кадр поля и накопление
	/// ДН попадает значение после рассеяния; допустимо. Сказаться может, если вход лежит на контуре ДН — проверить.
	for( ; nTCurrent < nTMax;  ii+=4)
	{
		if( IsError() ) { if( bGpu ) g_cGpuX.End( prUNode1, pcNodeDiel ); m_cTiming.Add( TMCT_LOOPWALL, qTmLoop ); m_cTiming.cur.nTDone = nTCurrent; return; }	// TIMING, GPU
		if( !bIsReadData ) 
		{
			if( bGpu ) g_cGpuX.End( prUNode1, pcNodeDiel );	// GPU
			m_cTiming.Add( TMCT_LOOPWALL, qTmLoop );	// TIMING
			m_cTiming.cur.nTDone = nTCurrent;		// TIMING
			time( &ltime_current );
			nBuf = (int)( (dTCurrent/cParam.rTimeUnit)*100 + 0.5 );
			csBuf.Format("nT:%d(%lg%s);%ld;%lg", nTCurrent, nBuf*0.01,
				cParam.csTimeUnit,ltime_current - ltime_start, 
				((double)ltime_current - ltime_start)/nTCurrent);
			PutStatistics(csBuf);
			return;
		};
		// Каждые 100 подтактов — ход счёта и прошедшее время в строку состояния.
		if( ii == 100 )
		{
			__int64 qTmSt = TmcTimingNow();			// TIMING
			time( &ltime_current );
			nBuf = (int)( (dTCurrent/cParam.rTimeUnit)*100 + 0.5 );
			csBuf.Format("nT:%d;Time:%lg%s", nTCurrent, nBuf*0.01, cParam.csTimeUnit );
			PutStatistics(csBuf);
			ii = ltime_current - ltime_start;
			if( ii <= 60 )
			{
				csBuf.Format("Run Step %d  ElapsTime: %2d", cIndan.GetnStep(), ii );
				PutTrace(csBuf);
			}
			else
			{
				if( ii <= 3600 )
				{
					csBuf.Format("Run Step %d  ElapsTime: %2d:%2d", cIndan.GetnStep(), ii/60, ii-(ii/60)*60 );
					PutTrace(csBuf);
				}
				else
				{
					csBuf.Format("Run Step %d  ElapsTime: %2d:%2d:%2d", cIndan.GetnStep(), ii/3600, ii/60-(ii/3600)*60 , ii-(ii/60)*60 );
					PutTrace(csBuf);
				}
			}
			m_cTiming.cur.nTDone = nTCurrent;		// TIMING
			m_cTiming.Add( TMCT_STATUS, qTmSt );	// TIMING
			ii = 0;
		};
		if( bGpu ) RunKernel1TGpu();	// GPU
		else RunKernel1T();
	};
	m_cTiming.Add( TMCT_LOOPWALL, qTmLoop );			// TIMING
	if( bGpu )	// GPU: the grid and absorber states back for PutSmatrix and the next step (one-off, outside the loop time)
	{
		g_cGpuX.End( prUNode1, pcNodeDiel );
		if( ( !g_cGpuX.csError.IsEmpty() )&&( !IsError() ) ) cError.PutErrorMessage( g_cGpuX.csError );
	};
	m_cTiming.cur.nTDone = nTCurrent;					// TIMING

	time( &ltime_current );
	csBuf.Format("nT:%d;%ld;%lg", nTCurrent,
		ltime_current - ltime_start, 
		((double)ltime_current - ltime_start)/nTCurrent);
	PutStatistics(csBuf);
	
	// Окно ДН заканчивается позже конца расчёта — файлы ДН не будут записаны: это ошибка.
	if( ( !IsError() )&&( bIsReadData )&&( nTCurrent >= nTMax )&&	// A-14: faily DN ne zapisany
		( cIndan.GetOutput()->GetFieldIntegrated().bIsFieldDistr() )&&
		( cIndan.GetOutput()->GetFieldIntegrated().IsWindowOpen() ) )
	{
		csBuf.Format("Directional pattern window ends at %lg %s, after the calculation time: DN files are not written",
			cIndan.GetOutput()->GetFieldIntegrated().GetTmax()/cParam.rTimeUnit, (LPCTSTR)cParam.csTimeUnit );
		cError.PutErrorMessage( csBuf );
	};

	// После цикла: матрица рассеяния по сигналам .t (файлы ДН пишутся только по команде меню).
	__int64 qTmPost = TmcTimingNow();			// TIMING
	PutSmatrix();
	m_cTiming.Add( TMCT_SMATRIX, qTmPost );		// TIMING
	
	return;
}

BOOL CTmcRTHRectNode::IsError( void )
{
	return cError.IsError();
}

/* ==== BUG12 PORTFIX (RectNode wrappers) ==== */
void CTmcRTHRectNode::RunSaveInputs( void )
{
    int i;
    for( i = 0; i < nDiel; i++ ) pcNodeDiel[i].SaveInputNodes( prUNode1 );
}
void CTmcRTHRectNode::RunRestoreInputs( void )
{
    int i;
    for( i = 0; i < nDiel; i++ ) pcNodeDiel[i].RestoreInputNodes( prUNode1 );
}
// B-1 (29.10.2026): after the blocks - the nodes of the plane-wave ports once more (the port wins in the nodes
// shared with a MAGNETIC wall, as in the kernel H; old: the wall overwrote the edge nodes of the port)
/// Б-1: после блоков ещё раз восстанавливает узлы портов плоской волны (`RestorePlaneWavePortNodes`).
void CTmcRTHRectNode::RunRestorePlaneWaveInputs( void )
{
    int i;
    for( i = 0; i < nDiel; i++ ) pcNodeDiel[i].RestorePlaneWavePortNodes( prUNode1 );
}
/* ==== END BUG12 PORTFIX (RectNode) ==== */

void CTmcRTHRectNode::RunKernel1T( void )
{
	if( IsError() ) return;
	if( !bIsReadData ) return;
	__int64 qTm0, qTm1;							// TIMING

//   excite and out reflection wave for inputs 
//   calculation reflection wave
	// Подтакт 0: новое значение пишется в ячейку 0 узла (там было U^(tau-4)), U^(tau-1) — ячейка 3.
	// Узлы портов обновляет сам порт: общий проход рассеяния их не меняет (сохранение и
	// восстановление вокруг него).
	qTm0 = TmcTimingNow();								// TIMING
	RunExciteInputs();
	RunSaveInputs();
	qTm1 = TmcTimingNow(); m_cTiming.cur.q[TMCT_EXCITE] += qTm1 - qTm0;		// TIMING
	RunScatteringNode();
	qTm0 = TmcTimingNow(); m_cTiming.cur.q[TMCT_SCATTER] += qTm0 - qTm1;		// TIMING
	RunRestoreInputs();
	RunBlockNode();
	RunRestorePlaneWaveInputs();	// B-1: the plane-wave port wins over the walls
	qTm1 = TmcTimingNow(); m_cTiming.cur.q[TMCT_BLOCKNODE] += qTm1 - qTm0;	// TIMING
	PutField( 0 );
	m_cTiming.cur.q[TMCT_FIELDFILE] += TmcTimingNow() - qTm1;				// TIMING
	DistributionIntegrated( 0 );
	nTCurrent++;
	dTCurrent += dT;

//   excite and out reflection wave for inputs 
//   calculation reflection wave
	// Подтакт 1: пишется ячейка 1, U^(tau-1) — ячейка 0.
	qTm0 = TmcTimingNow();								// TIMING
	RunExciteInputs1();
	RunSaveInputs();
	qTm1 = TmcTimingNow(); m_cTiming.cur.q[TMCT_EXCITE] += qTm1 - qTm0;		// TIMING
	RunScatteringNode1();
	qTm0 = TmcTimingNow(); m_cTiming.cur.q[TMCT_SCATTER] += qTm0 - qTm1;		// TIMING
	RunRestoreInputs();
	RunBlockNode1();
	RunRestorePlaneWaveInputs();	// B-1: the plane-wave port wins over the walls
	qTm1 = TmcTimingNow(); m_cTiming.cur.q[TMCT_BLOCKNODE] += qTm1 - qTm0;	// TIMING
	PutField( 1 );
	m_cTiming.cur.q[TMCT_FIELDFILE] += TmcTimingNow() - qTm1;				// TIMING
	DistributionIntegrated( 1 );
	nTCurrent++;
	dTCurrent += dT;

//   excite and out reflection wave for inputs 
//   calculation reflection wave
	// Подтакт 2: пишется ячейка 2, U^(tau-1) — ячейка 1.
	qTm0 = TmcTimingNow();								// TIMING
	RunExciteInputs2();
	RunSaveInputs();
	qTm1 = TmcTimingNow(); m_cTiming.cur.q[TMCT_EXCITE] += qTm1 - qTm0;		// TIMING
	RunScatteringNode2();
	qTm0 = TmcTimingNow(); m_cTiming.cur.q[TMCT_SCATTER] += qTm0 - qTm1;		// TIMING
	RunRestoreInputs();
	RunBlockNode2();
	RunRestorePlaneWaveInputs();	// B-1: the plane-wave port wins over the walls
	qTm1 = TmcTimingNow(); m_cTiming.cur.q[TMCT_BLOCKNODE] += qTm1 - qTm0;	// TIMING
	PutField( 2 );
	m_cTiming.cur.q[TMCT_FIELDFILE] += TmcTimingNow() - qTm1;				// TIMING
	DistributionIntegrated( 2 );
	nTCurrent++;
	dTCurrent += dT;

//   excite and out reflection wave for inputs 
//   calculation reflection wave
	// Подтакт 3: пишется ячейка 3, U^(tau-1) — ячейка 2.
	qTm0 = TmcTimingNow();								// TIMING
	RunExciteInputs3();
	RunSaveInputs();
	qTm1 = TmcTimingNow(); m_cTiming.cur.q[TMCT_EXCITE] += qTm1 - qTm0;		// TIMING
	RunScatteringNode3();
	qTm0 = TmcTimingNow(); m_cTiming.cur.q[TMCT_SCATTER] += qTm0 - qTm1;		// TIMING
	RunRestoreInputs();
	RunBlockNode3();
	RunRestorePlaneWaveInputs();	// B-1: the plane-wave port wins over the walls
	qTm1 = TmcTimingNow(); m_cTiming.cur.q[TMCT_BLOCKNODE] += qTm1 - qTm0;	// TIMING
	PutField( 3 );
	m_cTiming.cur.q[TMCT_FIELDFILE] += TmcTimingNow() - qTm1;				// TIMING
	DistributionIntegrated( 3 );
	nTCurrent++;
	dTCurrent += dT;

	return;
}

void CTmcRTHRectNode::InitNodeArray( void )
{
	if( IsError() ) return;
	if( !bIsReadData ) return;

	DeleteNodeArray();
	if( nNumNode <= 1 )
	{
		cError.PutErrorMessage("InitNodeArray Number of Node <= 1");
		return;
	};

	TmcIdx i, ii;	// INT64
	void *pTemp;
	char *pTemp1;

	if( (prUNode1Temp	= new _ELEM_VAL_RTH[ 6*nNumNode + 32]) == NULL )
	{			 
		cError.PutErrorMessage("Not enough memory for space array");
		DeleteNodeArray();
		return;
	};
	// Выравнивание начала массива на 32 байта (сдвиг 1..32 байта; запас 32 элемента).
	pTemp  = (void *)prUNode1Temp;
	pTemp1 = (char *)prUNode1Temp;
	ii = (int)(INT_PTR)pTemp;
	i = 32 - (ii - (ii/32)*32);
	pTemp1 += i;
	prUNode1 = (_ELEM_VAL_RTH *) pTemp1;
	//GlobalAlloc		HeapAlloc		 prUNode1Temp
/*	if( (prUNode2	= new _ELEM_VAL_RTH[ nNumNode ]) == NULL )
	{			 
		cError.PutErrorMessage("Not enough memory for space array");
		DeleteNodeArray();
		return;
	};
*/	if( (prYNodeTemp	= new _real[ 6*nNumNode + 32]) == NULL )
	{			 
		cError.PutErrorMessage("Not enough memory for space array");
		DeleteNodeArray();
		return;
	};
	pTemp  = (void *)prYNodeTemp;
	pTemp1 = (char *)prYNodeTemp;
	ii = (int)(INT_PTR)pTemp;
	i = 32 - (ii - (ii/32)*32);
	pTemp1 += i;
	prYNode = (_real *) pTemp1;

//	CString csBuf1;
	// Все 6 + 6 ячеек узла — нули; коэффициенты свободного пространства задаст SetDielInNodeList.
	for( i = 0, ii = 0; i < nNumNode; i++ )	
	{
		prYNode[ii++]  = (_real)(0.0);
		prYNode[ii++]  = (_real)(0.0);
		prYNode[ii++]  = (_real)(0.0);
		prYNode[ii++]  = (_real)(0.0);
		prYNode[ii++]  = (_real)(0.0);
		prYNode[ii++]  = (_real)(0.0);
     };
	for( i = 0, ii = 0; i < nNumNode; i++ )	
	{
		prUNode1[ii++]   = (_ELEM_VAL_RTH)(0.0);
		prUNode1[ii++]   = (_ELEM_VAL_RTH)(0.0);
		prUNode1[ii++]   = (_ELEM_VAL_RTH)(0.0);
		prUNode1[ii++]   = (_ELEM_VAL_RTH)(0.0);
		prUNode1[ii++]   = (_ELEM_VAL_RTH)(0.0);
		prUNode1[ii++]   = (_ELEM_VAL_RTH)(0.0);
     };

	return;
}

void CTmcRTHRectNode::DeleteNodeArray( void )
{
	if( prUNode1 != NULL )
	{
		delete []prUNode1Temp;
		prUNode1 = NULL;
	};
	if( prYNode != NULL )
	{
		delete []prYNodeTemp;
		prYNode = NULL;
	};

	return;
}

void CTmcRTHRectNode::RunStep1( void )
{
	CString csBuf;
	if( bIsReadData ) return;
	if( cIndan.IsError() ) return;
	bIsReadData = TRUE;

	if( !cIndan.IsDataRead() ) ReadData();

	if( cIndan.IsError() )
	{
		csBuf.Format("Error: %s", cIndan.GetErrorMessage());
		cError.PutErrorMessage( csBuf );
		csBuf.Format("Step:%d;%s", cIndan.GetnStep(), cError.GetErrorMessage());
		PutTrace(csBuf);
		return;
	};
	if( !bIsReadData ) 
	{
		csBuf.Format("User cancel run step %d thread", cIndan.GetnStep());
		PutTrace(csBuf);
		return;
	};

	InitKernel();

	if( cError.IsError() )
	{
		csBuf.Format("Step:%d;%s", cIndan.GetnStep(), cError.GetErrorMessage());
		PutTrace(csBuf);
		return;
	};
	if( !bIsReadData ) 
	{
		csBuf.Format("User cancel run step %d thread", cIndan.GetnStep());
		PutTrace(csBuf);
		return;
	};

	RunKernel();

	if( cError.IsError() )
	{
		csBuf.Format("Step:%d;%s", cIndan.GetnStep(), cError.GetErrorMessage());
		PutTrace(csBuf);
		return;
	};
	if( !bIsReadData ) 
	{
		csBuf.Format("User cancel run step %d thread", cIndan.GetnStep());
		PutTrace(csBuf);
		return;
	};

	__int64 qTmSnd = TmcTimingNow();			// TIMING
	PutStepEndSound();
	m_cTiming.Add( TMCT_SOUND, qTmSnd );		// TIMING

	bIsReadData = FALSE;

	return;
}

BOOL CTmcRTHRectNode::IsRunStep( void )
{
	return bIsRunStep1Run;
}

BOOL CTmcRTHRectNode::IsDataInit( void )
{
	if( nNumNode == 0 ) return FALSE;
		else return TRUE;
}

void CTmcRTHRectNode::InitDielArray( void )
{
	if( IsError() ) return;
	if( !bIsReadData ) return;

	CString csBuf;

	DeleteDielArray();
	cParam = cIndan.GetParam();
	
	nDiel = cIndan.GetnBlock();
	csBuf.Format("nBlock:(%d);", nDiel );
	PutStatistics(csBuf);

	if( nNumNode < 1 )
	{
		cError.PutErrorMessage("InitDielArray Number of Block < 1");
		return;
	};

	if( (pcNodeDiel	= new CTmcRTHNodeDiel[ nDiel ]) == NULL )
	{
		cError.PutErrorMessage("Not enough memory for space array");
		DeleteNodeArray();
		return;
	};

	m_cTiming.cur.nBlocks = nDiel;				// TIMING
	__int64 qTm = TmcTimingNow();				// TIMING
	SetDielTopology();
	m_cTiming.Add( TMCT_BLOCKGEOM, qTm );			// TIMING
	qTm = TmcTimingNow();						// TIMING
	SetDielInNodeList();
	// R-1: the absorber load by the medium of its nodes - after the admittances are final, before the GPU copy
	for( int iR1 = 0; ( iR1 < nDiel )&&( !IsError() )&&( pcNodeDiel != NULL ); iR1++ )
		cError = pcNodeDiel[iR1].SetAbsorberLoadByMedium( cParam, prYNode, 6, 1, nNumNode );
	{	// R-1: absorber nodes in a medium without a propagating wave keep the old (vacuum) load - a warning
		TmcIdx nNoWave = 0;
		for( int iR1 = 0; ( iR1 < nDiel )&&( pcNodeDiel != NULL ); iR1++ ) nNoWave += pcNodeDiel[iR1].GetAbsorberNoWave();
		if( nNoWave > 0 )
		{
			CString csW;
			csW.Format("WARNING: %.0f absorber nodes are in a medium where the wave does not propagate on this grid "
				"(a dense plasma eps < 0 or k*Delta too large) - they keep the load of the vacuum; the absorber "
				"does not absorb there, move it out of such a medium", (double)nNoWave );
			ConsolePrint( "%s", (LPCSTR)csW );
			PutTrace( csW );
			SetRunWarning( csW );
		};
	};
	m_cTiming.Add( TMCT_ADMITTANCE, qTm );		// TIMING
	qTm = TmcTimingNow();						// TIMING
	PutTopology();
	m_cTiming.Add( TMCT_TOPOLOGYFILE, qTm );		// TIMING

	return;
}

void CTmcRTHRectNode::DeleteDielArray( void )
{
	int i;
	for( i = 0; i < nDiel; i++  ) pcNodeDiel[i].DeleteData();

	if( pcNodeDiel != NULL )
	{
		delete []pcNodeDiel;
		pcNodeDiel = NULL;
	};
	
}

void CTmcRTHRectNode::SetDielTopology( void )
{
	if( IsError() ) return;
	if( !bIsReadData ) return;
	
	if( nDiel < 1 )
	{
		cError.PutErrorMessage("InitDielArray Number of Block < 1");
		return;
	};
	
	if( pcNodeDiel == NULL )
	{
		cError.PutErrorMessage("InitDielArray isn't initial");
		return;
	};
	
	int i;
	CTmcRTH_BlockList * pcNextBlockList;
	for( i = 0, pcNextBlockList = cIndan.GetTopology().GetBlockList(); i < nDiel; i++)
	{
		pcNodeDiel[i].SetTimingCounters( m_cTiming.Counter( TMCT_SIGNALFILE ), m_cTiming.Counter( TMCT_EPSFILE ) );	// TIMING
		pcNodeDiel[i].SetTimingEpsCounters( m_cTiming.Counter( TMCT_EPSREAD ), &(m_cTiming.cur.nEpsWritten), &(m_cTiming.cur.nEpsRead) );	// TIMING
		cError = pcNodeDiel[i].SetBlock( pcNextBlockList, cParam, GetOutput(), prYNode, &bIsReadData, cIndan.GetCurrentPath() );
		if( IsError() ) return;
		pcNextBlockList = pcNextBlockList->GetNext();
		if( pcNextBlockList == NULL )
		{
			CString csBuf;
			csBuf.Format("Init Block %d to diel", i );
			cError.PutErrorMessage(csBuf);
			return;
		};
	}

//  A-20: spisok tonov signala peredaetsya v FieldIntegrated, kak v H-yadre	// BORDER
	for( i = 0; i < nDiel; i++ )
	{
		CString csFL = pcNodeDiel[i].GetSigFreqList();
		if( csFL.GetLength() < 1 ) continue;
		cIndan.GetOutput()->GetFieldIntegrated().SetFreqList( csFL, cParam.rFreqUnit );
		if( ( cIndan.GetOutput()->GetFieldIntegrated().GetFreqNum() > 1 )&&	// A-5
			( cIndan.GetOutput()->GetFieldIntegrated().bIsFieldDistr() )&&
			( !cIndan.GetOutput()->GetFieldIntegrated().bIsItegratedInMemory() ) )
		{
			cError.PutErrorMessage("Multi-frequency directional pattern needs FIELD_DISTRIBUTION_M (not FIELD_DISTRIBUTION)");
			return;
		};
		if( cIndan.GetOutput()->GetFieldIntegrated().GetFreqNum() > 1 )
			cIndan.GetOutput()->GetFieldIntegrated().InitArrayForIntegrate();
		break;					// one list describes the excitation
	};

	return;
}

BOOL CTmcRTHRectNode::IsReadData( void )
{
	return bIsReadData;
}


void CTmcRTHRectNode::RunScatteringNode( void )
{
	if( IsError() ) return;

	CString csBuf1;
	int i, ny1, nx1, nx14, nx148, ny14, ny148, nx11_10;
	_ELEM_VAL_RTH *pr1;
	_real *prY1;

	ny1   = nY - 2;
	nx1   = nX - 2;
	nx14  = nX*6;
	ny14  = nX*6;
	nx148 = nX*6;
	ny148 = nX*6;
	nx11_10 = nx1;
	// Обходятся только внутренние узлы: строки 1..nY-2, столбцы 1..nX-2.
	// Узлы краёв области не пересчитываются и остаются нулевыми (если их не задаёт блок):
	// на краю области Hz = 0, то есть в X-ядре край — магнитная стенка.
	for(i = 0, 
			pr1 = prUNode1+6+nx14, 
			prY1 = prYNode+6+ny14;
		i < ny1;
		i++,
			pr1     += nx14,
			prY1    += ny14)
	{
		RunScatteringNode1line( pr1, prY1, nx11_10, nx148);
	};


	return;
}

void CTmcRTHRectNode::RunScatteringNode1( void )
{
	if( IsError() ) return;

	CString csBuf1;
	int i, ny1, nx1, nx14, nx148, ny14, ny148, nx11_10;
	_ELEM_VAL_RTH *pr1;
	_real *prY1;

	ny1   = nY - 2;
	nx1   = nX - 2;
	nx14  = nX*6;
	ny14  = nX*6;
	nx148 = nX*6;
	ny148 = nX*6;
	nx11_10 = nx1;
	for(i = 0, 
			pr1 = prUNode1+6+nx14, 
			prY1 = prYNode+6+ny14;
		i < ny1;
		i++,
			pr1     += nx14,
			prY1    += ny14)
	{
		RunScatteringNode1line1( pr1, prY1, nx11_10, nx148);
	};


	return;
}


void CTmcRTHRectNode::RunScatteringNode2( void )
{
	if( IsError() ) return;

	CString csBuf1;
	int i, ny1, nx1, nx14, nx148, ny14, ny148, nx11_10;
	_ELEM_VAL_RTH *pr1;
	_real *prY1;

	ny1   = nY - 2;
	nx1   = nX - 2;
	nx14  = nX*6;
	ny14  = nX*6;
	nx148 = nX*6;
	ny148 = nX*6;
	nx11_10 = nx1;
	for(i = 0, 
			pr1 = prUNode1+6+nx14, 
			prY1 = prYNode+6+ny14;
		i < ny1;
		i++,
			pr1     += nx14,
			prY1    += ny14)
	{
		RunScatteringNode1line2( pr1, prY1, nx11_10, nx148);
	};


	return;
}


void CTmcRTHRectNode::RunScatteringNode3( void )
{
	if( IsError() ) return;

	CString csBuf1;
	int i, ny1, nx1, nx14, nx148, ny14, ny148, nx11_10;
	_ELEM_VAL_RTH *pr1;
	_real *prY1;

	ny1   = nY - 2;
	nx1   = nX - 2;
	nx14  = nX*6;
	ny14  = nX*6;
	nx148 = nX*6;
	ny148 = nX*6;
	nx11_10 = nx1;
	for(i = 0, 
			pr1 = prUNode1+6+nx14, 
			prY1 = prYNode+6+ny14;
		i < ny1;
		i++,
			pr1     += nx14,
			prY1    += ny14)
	{
		RunScatteringNode1line3( pr1, prY1, nx11_10, nx148);
	};


	return;
}





void CTmcRTHRectNode::SetDielInNodeList( void )
{
	if( IsError() ) return;
	if( !bIsReadData ) return;
	
	if( nDiel < 1 )
	{
		cError.PutErrorMessage("InitDielArray Number of Block < 1");
		return;
	};
	
	if( pcNodeDiel == NULL )
	{
		cError.PutErrorMessage("InitDielArray isn't initial");
		return;
	};
	if( (prUNode1 == NULL)||(prYNode == NULL)||(nNumNode == 0) )
	{
		cError.PutErrorMessage("InitDielArray isn't initial");
		return;
	};

	TmcIdx i, ii;	// INT64
	int jj;
	double a, koef, koef1;
	double d_EpsAdd, d_N_ElectronConcentr, d_B_MagneticField, d_Y_Loss;
	double d_e, d_m, d_eps0;
	double d_y5, d_y6, d_y7;
	double koef_N, koef_wp2, koef_WC2;
	double d_wp2, d_WC2;

	// koef1 — поправка omega*tau/2 (проверяется на величину, но в расчёт идёт 1);
	// koef_N = e^2/(m*eps0*omega^2) переводит N в Delta eps, koef_wp2 = e^2/(m*eps0) — N в wp^2,
	// koef_WC2 = e^2/m^2 — B^2 в wc^2.
	koef = (cParam.rDelta*cParam.rFreq*2.*PI___/C0___);
	koef = (koef*koef/2.);
	koef1 = (cParam.rFreq*2.*PI___*dT/2.);

	d_e = ELECTRON_Q___;
	d_m = ELECTRON_M___;
	d_eps0 = EPS0___;

	koef_N = d_e*d_e/(d_m*d_eps0)/(cParam.rFreq*2.*PI___)/(cParam.rFreq*2.*PI___);
	koef_wp2 = d_e*d_e/(d_m*d_eps0);
	koef_WC2 = d_e*d_e/(d_m*d_m);
	if( ( koef1 > ( PI___/3 ) )||( koef1 < ( 0.0 ) ) )
	{
		cError.PutErrorMessage("InitDielArray isn't initial very big Delta*Frequence");
		return;
	};
	if( koef1 < (1.e-15) )
	{
		koef1 = 1.;
	}
	else
	{
		/// З-2 (автор 29.10): поправок на конечную задержку шлейфа ДВЕ и они разные — для шлейфа КЗ (2.45), для шлейфа
		/// ХХ (2.43) книги К. Н. Климова «Численное электродинамическое моделирование неоднородной плазмы» (гл. 2,
		/// стр. 39–40); обе верны каждая для своего случая. Один общий koef1 для обоих шлейфов неверен, поэтому
		/// поправка выключена (koef1 = 1). Включать — только раздельно для КЗ и ХХ.
//		koef1 = koef1/tan( koef1 );found
		koef1 = 1.;
	};

	{
		CString csBuf;
		csBuf.Format("Set node admitance");
		PutTrace(csBuf);
	};
//		CString csBuf1;
////		csBuf1.Format("pcNodeDielOne");

	// Предупреждения (в консоль, строку состояния и итог расчёта), сами значения не меняются.
	{	// B-6: positive epsilon together with the plasma (or with B) in one node - a warning, the code is not changed
		double dEpsN = 0.0, dEpsB = 0.0, dEpsNeg = 0.0;
		TmcIdx k;
		for( k = 0; k < nNumNode; k++ )
		{
			if( !( prYNode[ k*6 ] > 0.0 ) ) continue;
			if( fabs( prYNode[ k*6 + 1 ] ) > DBL_MIN )
			{
				dEpsN += 1.0;
				if( prYNode[ k*6 ] - prYNode[ k*6 + 1 ]*koef_N > 0.0 ) dEpsNeg += 1.0;	// the equivalent N < 0
			}
			else if( fabs( prYNode[ k*6 + 4 ] ) > DBL_MIN ) dEpsB += 1.0;
		};
		if( dEpsN > 0.0 )
		{
			CString csW;
			csW.Format("WARNING: %.0f nodes have a positive addition of epsilon and the plasma N at once - such a node is "
				"calculated as a plasma, the addition of epsilon is right only at FREQ%s; put the dielectric and the plasma "
				"in different nodes", dEpsN, ( dEpsNeg > 0.0 ) ? "; in some of them the addition is larger than the plasma part - "
				"the equivalent concentration is negative and the calculation DIVERGES" : "" );
			ConsolePrint( "%s", (LPCSTR)csW );
			PutTrace( csW );
			SetRunWarning( csW );
		};
		if( dEpsB > 0.0 )
		{
			CString csW;
			csW.Format("WARNING: %.0f nodes have a positive addition of epsilon and the magnetic field B - B is not used "
				"in a dielectric node (only in a plasma)", dEpsB );
			ConsolePrint( "%s", (LPCSTR)csW );
			PutTrace( csW );
			SetRunWarning( csW );
		};
	};
	// Ячейки узла до пересчёта: 0 — Delta eps, 1 — N (м^-3), 2 — Y, 3 — sigma (См/м), 4 — B (Тл).
	for( i = 0, ii = 0, jj = 0; i < nNumNode; i++, ii+=6, jj++ )	
	{
		d_EpsAdd = prYNode[ ii ];
		d_N_ElectronConcentr = prYNode[ ii + 1 ];
		d_Y_Loss = prYNode[ ii + 2 ];
		d_B_MagneticField = prYNode[ ii + 4 ];
		// sigma (См/м) переходит в сеточные потери Y = 2*dT*sigma/eps0 (прибавляется к Y узла).
		/// З-7 (автор 29.10): sigma в *_STAT_S — физическая проводимость, См/м; Y в *_STAT_Y / FILE_Y — уже
		/// нормированная (сеточная) проводимость. Потери действуют и вне плазмы — среда с потерями (Рук. 2).
		/// З-9 (автор 29.10): пересчёт как в узле H верен в смысле дуальной схемы — в X узел хранит Hz, импеданс
		/// дуальной схемы совпадает с адмитансом исходной, затухание плоской волны то же. Модель плазмы — без
		/// столкновений; связь sigma с частотой столкновений — план будущей разработки.
		if( fabs( prYNode[ ii + 3 ] ) > 0.0 )	// B-20: sigma (S/m) of *_STAT_S -> Y = 2*dT*sigma/eps0 (through the grid step)
		{
			prYNode[ ii + 2 ] += (_real)( 2.*dT*prYNode[ ii + 3 ]/EPS0___ );
			d_Y_Loss = prYNode[ ii + 2 ];
			prYNode[ ii + 3 ] = (_real)(0.0);
		};

		// Z-8 (author 29.10): Delta eps and the plasma N that cancel exactly (the sum is 0) - the node is the vacuum
		// (old: the plasma branch with wp^2 = 0 gave y5 = 0/0 at B = 0, NaN in the whole field).
		/// З-8: если добавка диэлектрика и плазма в узле в сумме дали ровно 0, узел — вакуум (ветка диэлектрика).
		if( ( fabs(d_N_ElectronConcentr) > DBL_MIN )&&( d_EpsAdd - d_N_ElectronConcentr*koef_N == 0.0 ) )
		{
			d_EpsAdd = 0.0;
			d_N_ElectronConcentr = 0.0;
		};
		// Узел плазмы: Delta eps < 0 (eps < 1 моделируется эквивалентной плазмой) или задана N.
		if( (d_EpsAdd < 0.0) || ( fabs(d_N_ElectronConcentr) > DBL_MIN ) )
		{
			/// З-10 (автор 29.10): граница «плазма — плазма» и «плазма — диэлектрик» верна потому, что сетка —
			/// соединение многополюсников ячеек: равенство напряжений клемм = непрерывность касательного E, закон
			/// токов = условие для H (книга К. Н. Климова, раздел о граничных условиях).
			// Delta eps и N сводятся к одной эквивалентной концентрации:
			// wp^2 = N*e^2/(m*eps0) - Delta eps*omega^2; wc^2 = (e*B/m)^2.
			if( fabs( d_N_ElectronConcentr ) > DBL_MIN )
			{
				d_EpsAdd -= d_N_ElectronConcentr*koef_N;
			};
			d_N_ElectronConcentr = -d_EpsAdd/koef_N;
			d_wp2 = d_N_ElectronConcentr*koef_wp2;
			d_WC2 = d_B_MagneticField*d_B_MagneticField*koef_WC2;
			// Коэффициенты четырёхслойной схемы узла с подмагниченной плазмой, x = dT^2*(wc^2 + wp^2):
			// Y1 = -dT^2*wp^4/(wc^2 + wp^2), Y3 = 8*dT^2*wc^2*wp^2/((wc^2 + wp^2)*(4 + x)),
			// Y4 = 2*(4 - x)/(4 + x), Y5 = Y3*Y4/4, Y0 = 2/(4 + |Y1| + Y2 + Y3/2).
			d_y5 = dT*dT*d_wp2*d_wp2/(d_WC2+d_wp2);
			d_y6 = 4*d_WC2*d_wp2/((d_WC2+d_wp2)*(d_WC2+d_wp2));
			d_y7 = dT*dT*d_WC2*d_wp2/(d_WC2+d_wp2);
			prYNode[ ii + 1 ] = (_real)( -d_y5 );
			if( fabs(d_y6+d_y7) < DBL_MIN )
			{
				prYNode[ ii + 3 ] = (_real)(0.0);
			}
			else
			{
				prYNode[ ii + 3 ] = (_real)(8*dT*dT*d_WC2*d_wp2/((d_WC2+d_wp2)*(4+dT*dT*(d_WC2+d_wp2))));
			};
			// B-1: the sign by formula (5.49): 2(4 - x)/(4 + x), x = dT^2*(wc^2 + wp^2) (26.10.2026).
			// Old (the pole of the upper hybrid resonance at the Nyquist frequency - B only weakened the plasma);
			// to return it, put this line back instead of the next one (and the old lines in PutTopology, B-1):
			// prYNode[ ii + 4 ] = (_real)(2*(dT*dT*(d_WC2+d_wp2)-4.)/(dT*dT*(d_WC2+d_wp2)+4.));
			prYNode[ ii + 4 ] = (_real)(2*(4.-dT*dT*(d_WC2+d_wp2))/(4.+dT*dT*(d_WC2+d_wp2)));
			prYNode[ ii + 5 ] = (_real)(prYNode[ ii + 3 ]*prYNode[ ii + 4 ]/4.);
			a = (4. + fabs( prYNode[ ii + 1 ] ) + prYNode[ ii + 2 ] + prYNode[ ii + 3 ]/2. );
			prYNode[ ii ] =  (_real)(2./a);
//			{
//				CString csBuf1;
//				csBuf1.Format("SetDielInNodeList:\nd_EpsAdd = %lg;\nd_N_ElectronConcentr = %lg;\nd_wp2 = %lg;\nd_WC2 = %lg;\nd_y5 = %lg;d_y6 = %lg;d_y7 = %lg;",
//					d_EpsAdd, d_N_ElectronConcentr, d_wp2, d_WC2,
//					d_y5, d_y6, d_y7);
//				AfxMessageBox(csBuf1);
//			};
		}
		else
		{
			/// З-11 (автор 29.10): для ХХ (eps > 1) схема для волн E точно совпадает с H — отдельно не выводилась;
			/// усложняется только случай КЗ (eps < 1) и подмагниченная плазма.
			// Узел диэлектрика (Delta eps >= 0, N = 0): шлейф холостого хода Y1 = 4*Delta eps, плазменных
			// слагаемых нет (Y3 = Y5 = 0, Y4 = -2); B такого узла не используется.
			/// З-1 (автор 29.10): Delta eps в пределах 0,004 от 4 сдвигается до 3,996 / 4,004 — защита от расходимости схемы при Delta eps = 4.
			if( fabs( d_EpsAdd - 4.) < 0.004 )
			{
				if( d_EpsAdd < 4. )	d_EpsAdd = (_ELEM_VAL_RTH)3.996;
				else d_EpsAdd = (_ELEM_VAL_RTH)4.004;
			};
			prYNode[ ii + 1 ] = (_real)( 4*d_EpsAdd/koef1 );
			prYNode[ ii + 3 ] = (_real)(0.0);
			prYNode[ ii + 4 ] = (_real)(-2.);
			prYNode[ ii + 5 ] = (_real)(0.0);
			a = (4. + fabs( prYNode[ ii + 1 ] ) + prYNode[ ii + 2 ]);
			prYNode[ ii ] =  (_real)(2./a);
//		CString csBuf1;
////		csBuf1.Format("pcNodeDielOne");
		};
		if( jj == 100000 )
		{
			{
				CString csBuf;
				csBuf.Format("n:%.0f", (double)i);	// INT64
				PutStatistics(csBuf);
			};
			jj = 0;
			if( !bIsReadData ) return;	// S-13: stopped (the window is closing) - do not go through all the nodes
		};
	};

	{
		CString csBuf;
		csBuf.Format(" ");
		PutStatistics(csBuf);
	};
	return;
}

void CTmcRTHRectNode::RunExciteInputs( void )
{
	if( IsError() ) return;
	int i;
	double dWT = dTCurrent*cParam.rFreq*2*3.141592653589;

	OutputnT();
	for( i = 0; i < nDiel; i++)
	{
		pcNodeDiel[i].ExciteInputs( cError, dWT, dTCurrent, prUNode1, nX, cParam, dTCurrent );
		if( IsError() ) return;
	}

	return;
}

void CTmcRTHRectNode::RunExciteInputs1( void )
{
	if( IsError() ) return;
	int i;
	double dWT = (dTCurrent)*cParam.rFreq*2*3.141592653589;

	OutputnT();
	for( i = 0; i < nDiel; i++)
	{
		pcNodeDiel[i].ExciteInputs1( cError, dWT, dTCurrent, prUNode1, nX, cParam, dTCurrent );
		if( IsError() ) return;
	}

	return;
}

void CTmcRTHRectNode::RunExciteInputs2( void )
{
	if( IsError() ) return;
	int i;
	double dWT = (dTCurrent)*cParam.rFreq*2*3.141592653589;

	OutputnT();
	for( i = 0; i < nDiel; i++)
	{
		pcNodeDiel[i].ExciteInputs2( cError, dWT, dTCurrent, prUNode1, nX, cParam, dTCurrent );
		if( IsError() ) return;
	}

	return;
}

void CTmcRTHRectNode::RunExciteInputs3( void )
{
	if( IsError() ) return;
	int i;
	double dWT = (dTCurrent)*cParam.rFreq*2*3.141592653589;

	OutputnT();
	for( i = 0; i < nDiel; i++)
	{
		pcNodeDiel[i].ExciteInputs3( cError, dWT, dTCurrent, prUNode1, nX, cParam, dTCurrent );
		if( IsError() ) return;
	}

	return;
}

void CTmcRTHRectNode::DeleteOutputFile( void )
{
	if( IsError() ) return;

	CTmcRTH_IndanOutput* cOut;
	
	cOut = cIndan.GetOutput();
	
	if( cOut->csFileName.GetLength() == 0 )
	{
		cError.PutErrorMessage("Output file name is NULL");
		return;
	};
	
	if( cOut->csFileNameField.GetLength() == 0 )
	{
		cError.PutErrorMessage("Output field file name is NULL");
		return;
	};
	if( cOut->csFileNameTopology.GetLength() == 0 )
	{
		cError.PutErrorMessage("Output field file name is NULL");
		return;
	};

	FILE *fp;
	CString csOpenErr;	// T-5: the .t is opened first and stays open (write denied to others) until the end of the step
	if( (fp = TmcOutTOpen( cOut->csFileName, "w", csOpenErr )) == NULL )
	{
		CString csOpenMsg;
		csOpenMsg.Format("Can't open output file for write data. %s", (LPCSTR)csOpenErr);
		cError.PutErrorMessage( csOpenMsg );
		return;
	};
	remove(cOut->csFileNameField);
	remove(cOut->csFileNameTopology);
	// Заголовок .t: метка, имя, частота (Гц), число входов -2; дальше строки подтактов.
	fprintf( fp, "%s\n!File name %s\n%s%lg\n%s%d\n!nT\tnBlock\tUp\tUo",
		TMC_GROTS_FILEOUT_ID,
		cOut->csFileName,
		TMC_GROTS_FLOUTFreq_ID, cParam.rFreq,
		TMC_GROTS_FLOUTInpNum_ID, -2);
	fflush( fp );


	return;
}

void CTmcRTHRectNode::OutputnT( void )
{
	if( IsError() ) return;

	CTmcRTH_IndanOutput* cOut;
	
	cOut = cIndan.GetOutput();
	
	if( cOut->csFileName.GetLength() == 0 )
	{
		cError.PutErrorMessage("Output file name is NULL");
		return;
	};

	FILE *fp;
	char *psForm;
	__int64 qTm = TmcTimingNow();				// TIMING

	CString csOpenErr;
	if( ( fp = TmcOutTGet( cOut->csFileName ) ) != NULL ) fflush( fp );	// T-5: the file of the step is open; the line before goes to the disk
	else if( ( fp = TmcOutTOpen( cOut->csFileName, "a", csOpenErr ) ) == NULL )	// after Stop - open it again (T-9: wait 20 s)
	{
		cError.PutErrorMessage( csOpenErr );
		return;
	};
	


	// Строка подтакта: перевод строки, номер такта и время; затем порты дописывают «блок Up Uo».
	fprintf( fp, "\n");
	psForm = GetnTFormat();
	fprintf( fp, psForm, nTCurrent);
	psForm = GetdTcurrentFormat();
	fprintf( fp, psForm, dTCurrent);

	m_cTiming.Add( TMCT_SIGNALFILE, qTm );		// TIMING
	return;
}

/*
near void CTmcRTHRectNode::RunScatteringNode1line( _ELEM_VAL_RTH *pr_111, float *prY_111, int *pnType_111, int nx_111, int nX_111)
{
	__asm
	{
		mov eax, pr_111
		mov ecx, nX_111
		mov edx, nx_111
		mov ebx, prY_111
metka1:
		fld dword ptr [ebx+4]
		fld qword ptr [eax-8]
		fld qword ptr [eax+24]
		fld qword ptr [eax+ecx+8]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+8]
		fld qword ptr [eax+8]
		fld dword ptr [ebx]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax]
		fsub
		fstp qword ptr [eax]

		fld dword ptr [ebx+12]
		fld qword ptr [eax+8]
		fld qword ptr [eax+40]
		fld qword ptr [eax+ecx+24]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+24]
		fld qword ptr [eax+24]
		fld dword ptr [ebx+8]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+16]
		fsub
		fstp qword ptr [eax+16]

		fld dword ptr [ebx+20]
		fld qword ptr [eax+24]
		fld qword ptr [eax+56]
		fld qword ptr [eax+ecx+40]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+40]
		fld qword ptr [eax+40]
		fld dword ptr [ebx+16]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+32]
		fsub
		fstp qword ptr [eax+32]

		fld dword ptr [ebx+28]
		fld qword ptr [eax+40]
		fld qword ptr [eax+72]
		fld qword ptr [eax+ecx+56]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+56]
		fld qword ptr [eax+56]
		fld dword ptr [ebx+24]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+48]
		fsub
		fstp qword ptr [eax+48]

		fld dword ptr [ebx+36]
		fld qword ptr [eax+56]
		fld qword ptr [eax+88]
		fld qword ptr [eax+ecx+72]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+72]
		fld qword ptr [eax+72]
		fld dword ptr [ebx+32]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+64]
		fsub
		fstp qword ptr [eax+64]

		lea eax, [eax + 80]

		fld dword ptr [ebx+44]
		fld qword ptr [eax-8]
		fld qword ptr [eax+24]
		fld qword ptr [eax+ecx+8]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+8]
		fld qword ptr [eax+8]
		fld dword ptr [ebx+40]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax]
		fsub
		fstp qword ptr [eax]

		fld dword ptr [ebx+52]
		fld qword ptr [eax+8]
		fld qword ptr [eax+40]
		fld qword ptr [eax+ecx+24]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+24]
		fld qword ptr [eax+24]
		fld dword ptr [ebx+48]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+16]
		fsub
		fstp qword ptr [eax+16]

		fld dword ptr [ebx+60]
		fld qword ptr [eax+24]
		fld qword ptr [eax+56]
		fld qword ptr [eax+ecx+40]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+40]
		fld qword ptr [eax+40]
		fld dword ptr [ebx+56]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+32]
		fsub
		fstp qword ptr [eax+32]

		fld dword ptr [ebx+68]
		fld qword ptr [eax+40]
		fld qword ptr [eax+72]
		fld qword ptr [eax+ecx+56]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+56]
		fld qword ptr [eax+56]
		fld dword ptr [ebx+64]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+48]
		fsub
		fstp qword ptr [eax+48]


		fld dword ptr [ebx+76]
		fld qword ptr [eax+56]
		fld qword ptr [eax+88]
		fld qword ptr [eax+ecx+72]
		neg ecx
		clc
		fld qword ptr [eax+ecx+72]
		fld qword ptr [eax+72]
		fld dword ptr [ebx+72]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		lea ebx, [ebx + 80]
		fld qword ptr [eax+64]
		fsub
		fstp qword ptr [eax+64]
		
//		lea eax, [eax + 80]


		dec edx
		ja metka1
	}
}


near void CTmcRTHRectNode::RunScatteringNode1line1( _ELEM_VAL_RTH *pr_111, float *prY_111, int *pnType_111, int nx_111, int nX_111)
{
	__asm
	{
		mov eax, pr_111
		mov ecx, nX_111
		mov edx, nx_111
		mov ebx, prY_111
metka1:
		fld dword ptr [ebx+4]
		fld qword ptr [eax-16]
		fld qword ptr [eax+16]
		fld qword ptr [eax+ecx]
		neg ecx
//		clc
		fld qword ptr [eax+ecx]
		fld qword ptr [eax]
		fld dword ptr [ebx]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+8]
		fsub
		fstp qword ptr [eax+8]

		fld dword ptr [ebx+12]
		fld qword ptr [eax]
		fld qword ptr [eax+32]
		fld qword ptr [eax+ecx+16]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+16]
		fld qword ptr [eax+16]
		fld dword ptr [ebx+8]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+24]
		fsub
		fstp qword ptr [eax+24]

		fld dword ptr [ebx+20]
		fld qword ptr [eax+16]
		fld qword ptr [eax+48]
		fld qword ptr [eax+ecx+32]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+32]
		fld qword ptr [eax+32]
		fld dword ptr [ebx+16]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+40]
		fsub
		fstp qword ptr [eax+40]

		fld dword ptr [ebx+28]
		fld qword ptr [eax+32]
		fld qword ptr [eax+64]
		fld qword ptr [eax+ecx+48]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+48]
		fld qword ptr [eax+48]
		fld dword ptr [ebx+24]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+56]
		fsub
		fstp qword ptr [eax+56]

		fld dword ptr [ebx+36]
		fld qword ptr [eax+48]
		fld qword ptr [eax+80]
		fld qword ptr [eax+ecx+64]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+64]
		fld qword ptr [eax+64]
		fld dword ptr [ebx+32]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+72]
		fsub
		fstp qword ptr [eax+72]

		lea eax, [eax + 80]

		fld dword ptr [ebx+44]
		fld qword ptr [eax-16]
		fld qword ptr [eax+16]
		fld qword ptr [eax+ecx]
		neg ecx
//		clc
		fld qword ptr [eax+ecx]
		fld qword ptr [eax]
		fld dword ptr [ebx+40]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+8]
		fsub
		fstp qword ptr [eax+8]

		fld dword ptr [ebx+52]
		fld qword ptr [eax]
		fld qword ptr [eax+32]
		fld qword ptr [eax+ecx+16]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+16]
		fld qword ptr [eax+16]
		fld dword ptr [ebx+48]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+24]
		fsub
		fstp qword ptr [eax+24]

		fld dword ptr [ebx+60]
		fld qword ptr [eax+16]
		fld qword ptr [eax+48]
		fld qword ptr [eax+ecx+32]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+32]
		fld qword ptr [eax+32]
		fld dword ptr [ebx+56]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+40]
		fsub
		fstp qword ptr [eax+40]

		fld dword ptr [ebx+68]
		fld qword ptr [eax+32]
		fld qword ptr [eax+64]
		fld qword ptr [eax+ecx+48]
		neg ecx
//		clc
		fld qword ptr [eax+ecx+48]
		fld qword ptr [eax+48]
		fld dword ptr [ebx+64]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		fld qword ptr [eax+56]
		fsub
		fstp qword ptr [eax+56]


		fld dword ptr [ebx+76]
		fld qword ptr [eax+48]
		fld qword ptr [eax+80]
		fld qword ptr [eax+ecx+64]
		neg ecx
		clc
		fld qword ptr [eax+ecx+64]
		fld qword ptr [eax+64]
		fld dword ptr [ebx+72]
		fmul
		fadd
		fadd
		fadd
		fadd
		fmul
		lea ebx, [ebx + 80]
		fld qword ptr [eax+72]
		fsub
		fstp qword ptr [eax+72]
		
//		lea eax, [eax + 80]


		dec edx
		ja metka1
	}
}
*/

void CTmcRTHRectNode::RunScatteringNode1line( _ELEM_VAL_RTH *pr1, _real *prY1, int nx_10, int nX_111)
{
	int i, ii;
	ii = nx_10;
	_real r;

	// Подтакт 0. Ячейки узла: 3 = U^(tau-1), 2 = U^(tau-2), 1 = U^(tau-3), 0 = U^(tau-4) -> U^tau;
	// 4 = R^(tau-1), 5 = R^(tau-2) -> R^tau. R — сумма соседей, шлейф Y1 и потери Y2.
	for( i = 0; i < ii; i++, pr1+=6, prY1+=6)
	{
//	{ 
//		CString csBuf1;
//		csBuf1.Format(" Node %d\n prY1[ 0 ] = %lg; prY1[ 1 ] = %lg; prY1[ 2 ] = %lg;\n prY1[ 3 ] = %lg; prY1[ 4 ] = %lg; prY1[ 5 ] = %lg;",
//			i, 
//			prY1[ 0 ], prY1[ 1 ], prY1[ 2 ],
//			prY1[ 3 ], prY1[ 4 ], prY1[ 5 ]);
////		csBuf1.Format("pcNodeDielOne");
		r = pr1[-6 + 3]+ pr1[6 + 3]+pr1[-nX_111 + 3]+pr1[nX_111 + 3] + prY1[ 1]*pr1[ 3] + prY1[ 2]*pr1[ 2];
		pr1[ 0] = -pr1[ 0] - 2*pr1[ 2] + prY1[ 4]*(pr1[ 1]+pr1[ 3])
				+ prY1[ 0]*( 
				  r
//				+ pr1[-6 + 3]+ pr1[6 + 3]+pr1[-nX_111 + 3]+pr1[nX_111 + 3] + prY1[ 1]*pr1[ 3] + prY1[ 2]*pr1[ 0]
				+ pr1[  5]
				- prY1[ 4]*(
//				  pr1[-6 + 2]+ pr1[6 + 2]+pr1[-nX_111 + 2]+pr1[nX_111 + 2] + prY1[ 1]*pr1[ 2] + prY1[ 2]*pr1[ 3]
				  pr1[ 4]
					)
				+ prY1[ 3]*pr1[ 2] - prY1[ 5]*(pr1[ 3] + pr1[1] )
				);
		pr1[ 5] = r;
	};
	
	return;
}

void CTmcRTHRectNode::RunScatteringNode1line1( _ELEM_VAL_RTH *pr1, _real *prY1, int nx_10, int nX_111)
{
	int i, ii;
	ii = nx_10;
	_real r;

	// Подтакт 1. Ячейки: 0 = U^(tau-1), 3 = U^(tau-2), 2 = U^(tau-3), 1 -> U^tau;
	// 5 = R^(tau-1), 4 = R^(tau-2) -> R^tau.
	for( i = 0; i < ii; i++, pr1+=6, prY1+=6)
	{
		r = pr1[-6 + 0]+ pr1[6 + 0]+pr1[-nX_111 + 0]+pr1[nX_111 + 0] + prY1[ 1]*pr1[ 0] + prY1[ 2]*pr1[ 3];
		pr1[ 1] = -pr1[ 1] - 2*pr1[ 3] + prY1[ 4]*(pr1[ 2]+pr1[ 0])
				+ prY1[ 0]*( 
				  r
//				+ pr1[-6 + 0]+ pr1[6 + 0]+pr1[-nX_111 + 0]+pr1[nX_111 + 0] + prY1[ 1]*pr1[ 0] + prY1[ 2]*pr1[ 1]
				+ pr1[ 4]
				- prY1[ 4]*(
//				  pr1[-6 + 3]+ pr1[6 + 3]+pr1[-nX_111 + 3]+pr1[nX_111 + 3] + prY1[ 1]*pr1[ 3] + prY1[ 2]*pr1[ 0]
				  pr1[ 5]
					)
				+ prY1[ 3]*pr1[ 3] - prY1[ 5]*(pr1[ 0] + pr1[ 2])
				);
		pr1[ 4] = r;
	};
	
	return;
}

void CTmcRTHRectNode::RunScatteringNode1line2( _ELEM_VAL_RTH *pr1, _real *prY1, int nx_10, int nX_111)
{
	int i, ii;
	ii = nx_10;
	_real r;

	// Подтакт 2. Ячейки: 1 = U^(tau-1), 0 = U^(tau-2), 3 = U^(tau-3), 2 -> U^tau;
	// 4 = R^(tau-1), 5 = R^(tau-2) -> R^tau.
	for( i = 0; i < ii; i++, pr1+=6, prY1+=6)
	{
		r = pr1[-6 + 1]+ pr1[6 + 1]+pr1[-nX_111 + 1]+pr1[nX_111 + 1] + prY1[ 1]*pr1[ 1] + prY1[ 2]*pr1[ 0];
		pr1[ 2] = -pr1[ 2] - 2*pr1[ 0] + prY1[ 4]*(pr1[ 3]+pr1[ 1])
				+ prY1[ 0]*( 
				  r
//				+ pr1[-6 + 1]+ pr1[6 + 1]+pr1[-nX_111 + 1]+pr1[nX_111 + 1] + prY1[ 1]*pr1[ 1] + prY1[ 2]*pr1[ 2]
				+ pr1[ 5]
				- prY1[ 4]*(
//				  pr1[-6 + 0]+ pr1[6 + 0]+pr1[-nX_111 + 0]+pr1[nX_111 + 0] + prY1[ 1]*pr1[ 0] + prY1[ 2]*pr1[ 1]
				  pr1[ 4]
					)
				+ prY1[ 3]*pr1[ 0] - prY1[ 5]*(pr1[ 1] + pr1[ 3])
				);
		pr1[ 5] = r;
	};
	
	return;
}


void CTmcRTHRectNode::RunScatteringNode1line3( _ELEM_VAL_RTH *pr1, _real *prY1, int nx_10, int nX_111)
{
	int i, ii;
	ii = nx_10;
	_real r;

	// Подтакт 3. Ячейки: 2 = U^(tau-1), 1 = U^(tau-2), 0 = U^(tau-3), 3 -> U^tau;
	// 5 = R^(tau-1), 4 = R^(tau-2) -> R^tau.
	for( i = 0; i < ii; i++, pr1+=6, prY1+=6)
	{
		r = pr1[-6 + 2]+ pr1[6 + 2]+pr1[-nX_111 + 2]+pr1[nX_111 + 2] + prY1[ 1]*pr1[ 2] + prY1[ 2]*pr1[ 1];
		pr1[ 3] = -pr1[ 3] - 2*pr1[ 1] + prY1[ 4]*(pr1[ 0]+pr1[ 2])
				+ prY1[ 0]*( 
				  r
//				+ pr1[-6 + 2]+ pr1[6 + 2]+pr1[-nX_111 + 2]+pr1[nX_111 + 2] + prY1[ 1]*pr1[ 2] + prY1[ 2]*pr1[ 3]
				+ pr1[ 4]
				- prY1[ 4]*(
//				  pr1[-6 + 1]+ pr1[6 + 1]+pr1[-nX_111 + 1]+pr1[nX_111 + 1] + prY1[ 1]*pr1[ 1] + prY1[ 2]*pr1[ 2]
				  pr1[ 5]
					)
				+ prY1[ 3]*pr1[ 1] - prY1[ 5]*(pr1[ 2] + pr1[ 0])
				);
		pr1[ 4] = r;
	};
	
	return;
}



void CTmcRTHRectNode::PutField( TmcIdx ii )	// INT64
{
	int i, j;
	FILE *fp;
	CTmcRTH_IndanOutput* cOut;
	
	cOut = cIndan.GetOutput();
	
	if( !cOut->bOutField ) return;

	static BOOL bFieldReaderLost = FALSE;	// A-17: FieldView ne chitaet fail polya
	// Синхронный вывод: ждать (до 500*50 мс), пока программа просмотра прочитает прошлый кадр.
	if( cOut->bOutFieldSinchronization && !IsAutoRun() )
	{
		if( bFieldReaderLost && !IsFieldFileRead() ) bFieldReaderLost = FALSE;	// FieldView snova chitaet
		for( i = 0; (!bFieldReaderLost)&&(IsFieldFileRead())&&(cOut->bOutFieldSinchronization)&&(cOut->bOutField)&&(bIsReadData)&&( i < 500 ) ; i++)
		{
			Sleep(50);
			cOut = cIndan.GetOutput();
		};
		if( i >= 500 ) bFieldReaderLost = TRUE;
	};

	for( i = 0; ( fp = fopen( cOut->csFileNameField , "wb") ) == NULL; i++ ) 
	{
		if( i > 2000  ) return;
		Sleep( 10 );
	};
	
	fprintf( fp, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, nTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, dTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, sizeof(_real),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, cParam.csLongUnit, cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, cParam.csTimeUnit, cParam.rTimeUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nX, nX, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nY, nY);
	// Кадр: строки снизу вверх, в строке слева направо; значение узла — ячейка ii, ii+6, ...
	for( i = 0; i < nY; i++)
	{
		fprintf( fp, "%c%c", 13, 10);
		for( j = 0; j < nX; j++ )
		{
			if( strcmp( GetdFieldOutFormat(), "") == 0 )
			{
				fwrite( &(prUNode1[ii]), sizeof(_real), 1, fp );
			}
			else
			{
				fprintf( fp, GetdFieldOutFormat(), prUNode1[ii]);
			};
			ii+=6;
		};
	};
	
	fclose(fp);
	SetFileAttributes( cOut->csFileNameField, FILE_ATTRIBUTE_NORMAL );

	if( IsDemoRun() ) 
	{
		RunOnViewField1();
	};
	
	return;

}

void CTmcRTHRectNode::OnOffFieldOutput()
{
	cIndan.OnOffFieldOutput();
	return;
}

void CTmcRTHRectNode::OnOffSinchronizationFieldOutput()
{
	cIndan.OnOffSinchronizationFieldOutput();
	return;
}

void CTmcRTHRectNode::RunBlockNode()
{
	if( IsError() ) return;
	int i;
	for( i = 0; i < nDiel; i++)
	{
		pcNodeDiel[i].RunBlockNode( prUNode1, prYNode, nX, 6*nNumNode );
		cError = pcNodeDiel[i].GetError();
		if( IsError() ) return;
	}

	return;
}

void CTmcRTHRectNode::RunBlockNode1()
{
	if( IsError() ) return;
	int i;
	for( i = 0; i < nDiel; i++)
	{
		pcNodeDiel[i].RunBlockNode1( prUNode1, prYNode, nX, 6*nNumNode );
		cError = pcNodeDiel[i].GetError();
		if( IsError() ) return;
	}

	return;
}

void CTmcRTHRectNode::RunBlockNode2()
{
	if( IsError() ) return;
	int i;
	for( i = 0; i < nDiel; i++)
	{
		pcNodeDiel[i].RunBlockNode2( prUNode1, prYNode, nX, 6*nNumNode );
		cError = pcNodeDiel[i].GetError();
		if( IsError() ) return;
	}

	return;
}

void CTmcRTHRectNode::RunBlockNode3()
{
	if( IsError() ) return;
	int i;
	for( i = 0; i < nDiel; i++)
	{
		pcNodeDiel[i].RunBlockNode3( prUNode1, prYNode, nX, 6*nNumNode );
		cError = pcNodeDiel[i].GetError();
		if( IsError() ) return;
	}

	return;
}

void CTmcRTHRectNode::PutStepEndSound()
{
	if( !((cIndan.GetOutput())->bSoundEffect) ) return;
	if( IsBatchRun() ) return;	// N-11: no sound in the batch run /Arb (a synchronous Beep, ~1.3 s a step)
	int n;
	n = cIndan.GetnStep();
	if( n == cIndan.GetNStepMax() )
	{
		BeepAllEnd();
		// RUN STATUS: the auto close after the last step is moved to CPlanRT_HDoc::RunAll
		// (here it worked only with the sound on and closed the window before the end of the run)
		return;
	};
	BeepStepEnd( n );
	return;
}



void CTmcRTHRectNode::OnOffSoundEffects()
{
	cIndan.OnOffSoundEffects();
	return;
}

void CTmcRTHRectNode::PutTopology()
{
	if( IsError() ) return;
	if( !IsDataInit() ) return;
	if( prYNode == NULL ) return;

	int i, j;
	TmcIdx ii;	// INT64
	FILE *fp;
	CTmcRTH_IndanOutput* cOut;
	CString csBuf;
	double dBuf, koef;
	double dAlf, dT2, dy_4, dy_5, dWc2, dwp2, d_e, d_m, d_eps0;
	double koef_N, koef_B;
	
	cOut = cIndan.GetOutput();
	
	if( !cOut->bOutTopology ) return;
	
	GetTopology().GetBlockList()->Save(csBuf);
	koef = (dT*cParam.rFreq*2.*PI___);
	koef = (koef*koef);
	dT2 = dT*dT;
	d_e = ELECTRON_Q___;
	d_m = ELECTRON_M___;
	d_eps0 = EPS0___;
	koef_N = d_eps0*d_m/(d_e*d_e);
	koef_B = d_m/d_e;

	{
		CString csBuf;
		csBuf.Format("Put Topology");
		PutTrace(csBuf);
	};

	for( i = 0; ( fp = fopen( cOut->csFileNameTopology , "wb") ) == NULL; i++ ) 
	{
		if( i > 2000  ) return;
		Sleep( 10 );
	};
	
	fprintf( fp, "%s%c%c%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%d", TMC_RTH_FILETOP_ID, 13, 10,
						csBuf, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, nTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, dTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nX, nX, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nY, nY);
//	Put eps+
	ii = 1;
	for( i = 0; i < nY; i++)
	{
		fprintf( fp, "%c%c", 13, 10);
		for( j = 0; j < nX; j++ )
		{
//			fprintf( fp, GetdFieldOutFormat(), prYNode1[0]);
			// Обратный пересчёт шлейфа Y1 в полную eps: Y1/4 + 1 (диэлектрик) или Y1/(omega*dT)^2 + 1 (плазма).
			dBuf = prYNode[ii];
			if( dBuf >= 0. ) dBuf /= 4.;
			else dBuf /= koef;
			dBuf += 1.; // for uotput full eps
			if( strcmp( GetdFieldOutFormat(), "") == 0 )
			{
				fwrite( &(dBuf), sizeof(double), 1, fp );
			}
			else
			{
				fprintf( fp, GetdFieldOutFormat(), dBuf);
			};
			ii+=6;
		};
	};

//	Put N
	ii = 0;
	for( i = 0; i < nY; i++)
	{
		fprintf( fp, "%c%c", 13, 10);
		for( j = 0; j < nX; j++ )
		{
			dBuf = prYNode[ii+1];
			if( dBuf >= 0. ) 
			{
				dwp2 = 0.0;
			}
			else 
			{
					// B-1: the inverse for the new sign of Y4 = 2(4-x)/(4+x): x = 4(2-Y4)/(2+Y4), wp^2 = sqrt(|Y1|*x)/dT^2.
					// Old (for the old sign):
					//   dy_4 = prYNode[ii+4]; dy_5 = fabs( dBuf ); dAlf = (1.-dy_4); if( fabs(dAlf) < DBL_MIN ) dAlf = DBL_MIN;
					//   dAlf = (1.+dy_4)/dAlf; dwp2 = 2*sqrt(-dy_5*dAlf)/(dT2);
					dy_4 = prYNode[ii+4];
					dy_5 = fabs( dBuf );
					dAlf = 2. + dy_4;
					if( fabs(dAlf) < DBL_MIN ) dAlf = DBL_MIN;
					dAlf = 4.*(2. - dy_4)/dAlf;		// x = dT^2*(wc^2 + wp^2)
					dwp2 = sqrt( dy_5*fabs(dAlf) )/dT2;
//					{
//						CString csBuf1;
//						csBuf1.Format( "dy_4 = %lg;\ndy_5 = %lg;\nAlf = %lg;\ndT2 = %lg;\ndwp2 = %lg;\nkoef_N = %lg;\nN = %lg;\n" ,
//							dy_4, dy_5, dAlf, dT2, dwp2, koef_N, dwp2*koef_N);
//						AfxMessageBox(csBuf1);
//					};
			};
			// N = wp^2*eps0*m/e^2, м^-3 (в узле диэлектрика 0).
			dBuf = dwp2*koef_N;
			if( strcmp( GetdFieldOutFormat(), "") == 0 )
			{
				fwrite( &(dBuf), sizeof(double), 1, fp );
			}
			else
			{
				fprintf( fp, GetdFieldOutFormat(), dBuf);
			};
			ii+=6;
		};
	};

//	Put Y
	ii = 2;
	for( i = 0; i < nY; i++)
	{
		fprintf( fp, "%c%c", 13, 10);
		for( j = 0; j < nX; j++ )
		{
			dBuf = prYNode[ii];
			if( strcmp( GetdFieldOutFormat(), "") == 0 )
			{
				fwrite( &(dBuf), sizeof(double), 1, fp );
			}
			else
			{
				fprintf( fp, GetdFieldOutFormat(), dBuf);
			};
			ii+=6;
		};
	};

//	Put B
	ii = 0;
	for( i = 0; i < nY; i++)
	{
		fprintf( fp, "%c%c", 13, 10);
		for( j = 0; j < nX; j++ )
		{
			dBuf = prYNode[ii+1];
			if( dBuf >= 0. ) dBuf = 0.0;
			else 
			{
				// B-1: the inverse for the new sign: x = 4(2-Y4)/(2+Y4), wp^2 = sqrt(|Y1|*x)/dT^2, wc^2 = x/dT^2 - wp^2.
				// Old (for the old sign):
				dy_4 = prYNode[ii+4];
				dy_5 = fabs( dBuf );
				dAlf = 2. + dy_4;
				if( fabs(dAlf) < DBL_MIN ) dAlf = DBL_MIN;
				dAlf = 4.*(2. - dy_4)/dAlf;		// x = dT^2*(wc^2 + wp^2)
				dwp2 = sqrt( dy_5*fabs(dAlf) )/dT2;
				dWc2 = fabs(dAlf)/dT2 - dwp2;
				if( dWc2 < 0. ) dWc2 = DBL_MIN;
				// B = wc*m/e, Тл.
				dBuf = sqrt( dWc2 )*koef_B;
			};
			if( strcmp( GetdFieldOutFormat(), "") == 0 )
			{
				fwrite( &(dBuf), sizeof(double), 1, fp );
			}
			else
			{
				fprintf( fp, GetdFieldOutFormat(), dBuf);
			};
			ii+=6;
		};
	};

	fclose(fp);

	return;
}

void CTmcRTHRectNode::PutTopol()
{
	PutTopology();
	cIndan.GetOutput()->bOutTopology = !cIndan.GetOutput()->bOutTopology;
	PutSinchronizFlag( cIndan.GetOutput()->bOutTopology, cIndan.GetOutput()->bOutField, cIndan.GetOutput()->bOutFieldSinchronization);
	PutTopology();
	return;
}

BOOL CTmcRTHRectNode::GetSinchronizationFieldOutput()
{
	return cIndan.GetOutput()->bOutFieldSinchronization;
}

BOOL CTmcRTHRectNode::GetFieldFlag()
{
	return cIndan.GetOutput()->bOutField;
}

BOOL CTmcRTHRectNode::GetSoundEffect()
{
	return cIndan.GetOutput()->bSoundEffect;
}

BOOL CTmcRTHRectNode::GetTopologyFlag()
{
	return cIndan.GetOutput()->bOutTopology;
}

BOOL CTmcRTHRectNode::IsFieldFileRead()
{
	CTmcRTH_IndanOutput* cOut;
	cOut = cIndan.GetOutput();
	DWORD dwAttr = GetFileAttributes( cOut->csFileNameField );
	if( dwAttr == 0xFFFFFFFF ) return FALSE;
	if( ( dwAttr & FILE_ATTRIBUTE_ARCHIVE ) == 0 ) return TRUE;	// S-59: only the ARCHIVE bit (compressed / OneDrive files)
	return FALSE;
}

void CTmcRTHRectNode::PutSmatrix()
{
	TmcOutTFlush( cIndan.GetOutput()->csFileName );	// T-5: the S-matrix reads the .t from the disk
	cIndan.GetOutput()->PutSmatrix();
	if( ( !cError.IsError() )&&( cIndan.GetOutput()->csSMatrixError.GetLength() > 0 ) )	// A-21
		cError.PutErrorMessage( cIndan.GetOutput()->csSMatrixError );
	if( cIndan.GetOutput()->csSMatrixWarning.GetLength() > 0 )	// B-22: the response has not died away
	{
		ConsolePrint( "WARNING: %s", (LPCSTR)cIndan.GetOutput()->csSMatrixWarning );
		SetRunWarning( cIndan.GetOutput()->csSMatrixWarning );
		PutTrace( cIndan.GetOutput()->csSMatrixWarning );
	};
	return;
}

void CTmcRTHRectNode::DistributionIntegrated( int ii )
{
	__int64 qTm = TmcTimingNow();				// TIMING
	BOOL bTmFinal = FALSE;						// TIMING
	if( cIndan.GetOutput()->GetFieldIntegrated().bIsFieldDistr() )	// TIMING
		bTmFinal = m_cTiming.IsDistrFinalCall( TRUE, dTCurrent,
			cIndan.GetOutput()->GetFieldIntegrated().GetTmin(),
			cIndan.GetOutput()->GetFieldIntegrated().GetTmax() );	// TIMING
	cIndan.GetOutput()->GetFieldIntegrated().Integrate( dTCurrent, nTCurrent, &(prUNode1[ii]) );
	if( cIndan.GetOutput()->GetFieldIntegrated().GetError().IsError() )	// A-1: ne zatirat oshibku podtakta
		cError = cIndan.GetOutput()->GetFieldIntegrated().GetError();
	m_cTiming.Add( bTmFinal ? TMCT_DISTRFINAL : TMCT_DISTRINTEGR, qTm );	// TIMING
	return;
}

void CTmcRTHRectNode::ExportToDirectionalPatter()
{
	cIndan.GetOutput()->GetFieldIntegrated().ExportToDirectionalPattern(cParam);
	if( cIndan.GetOutput()->GetFieldIntegrated().GetError().IsError() )	// A-1: ne zatirat oshibku rascheta
	{
		cError = cIndan.GetOutput()->GetFieldIntegrated().GetError();
		CString csBuf;
		csBuf.Format("Export to Directional pattern error:%s", cError.GetErrorMessage());
		PutTrace(csBuf);
	};
	return;
}

//  Wideband post-processing needs three numbers per port: grid step,
//  port aperture and the frequency the port was matched at. They are not
//  known when the header is written (blocks are not parsed yet), so the
//  lines are appended right after InitDielArray. OutputnT starts every
//  data line with '\n', therefore the last line here must NOT end with one.
void CTmcRTHRectNode::PutPortInfo( void )
{
	if( IsError() ) return;
	if( pcNodeDiel == NULL ) return;

	CTmcRTH_IndanOutput* cOut = cIndan.GetOutput();
	if( cOut == NULL ) return;
	if( cOut->csFileName.GetLength() == 0 ) return;

	int i, nWide = 0, nPort = 0;

	for( i = 0; i < nDiel; i++ )
	{
		if( !pcNodeDiel[i].IsInputBlock() ) continue;
		nPort++;
		if( pcNodeDiel[i].IsSignalFromFile() ) nWide++;
	};
	if( nPort < 1 ) return;

	FILE *fp;
	CString csOpenErr;
	if( ( fp = TmcOutTGet( cOut->csFileName ) ) == NULL )	// T-5: the file of the step is open
		fp = TmcOutTOpen( cOut->csFileName, "a", csOpenErr );
	if( fp == NULL )
	{
		cError.PutErrorMessage( csOpenErr );
		return;
	};

	fprintf( fp, "\n%s%d", TMC_GROTS_FLOUTWideband_ID, ( nWide > 0 ) ? 1 : 0 );

	//  If the signal file named its frequencies, pass them on: the
	//  post-processing then knows them exactly instead of hunting for
	//  peaks in the spectrum, which is not exact for a multi-tone.
	//  Written in Hz, so that nobody has to guess the units.
	for( i = 0; i < nDiel; i++ )
	{
		CString csL = pcNodeDiel[i].GetSigFreqList();
		if( csL.GetLength() < 1 ) continue;
		{
			char szF[CTMCRTH_INDANSBUF];
			char *pS;
			double dV;
			int nCh;
			strcpy( szF, ( const char * )csL );
			fprintf( fp, "%c%s", 10, TMC_GROTS_FLOUTSigFreq_ID );
			for( pS = szF; sscanf( pS, "%lg%n", &dV, &nCh ) == 1 ; pS += nCh )
			{
				fprintf( fp, " %.10lg", dV*cParam.rFreqUnit );
			};
		};
		break;				// one list is enough: it describes the excitation
	};
	for( i = 0; i < nDiel; i++ )
	{
		if( !pcNodeDiel[i].IsInputBlock() ) continue;
		//  block, grid step [m], aperture [m], matching frequency [Hz], from file
		fprintf( fp, "\n%s%d %.10lg %.10lg %.10lg %d",
			TMC_GROTS_FLOUTPort_ID,
			pcNodeDiel[i].GetBlockNumber(),
			cParam.rDelta,
			pcNodeDiel[i].GetWidthWaveg(),
			cParam.rFreq,
			pcNodeDiel[i].IsSignalFromFile() ? 1 : 0 );
	};
	return;		// T-5: the file stays open
}

// GPU: RunKernel1T with the grid on the video card (TmcGpuX.h). The same calls
// in the same order: ports (ExciteInputs + RunSaveInputs) and file output stay
// here, on the CPU, and see the nodes they touch; the node update, the restore
// of the port nodes (RunRestoreInputs) and METAL/ABSORBER/MAGNETIC blocks run
// on the card.
void CTmcRTHRectNode::RunKernel1TGpu( void )
{
	if( IsError() ) return;
	if( !bIsReadData ) return;
	__int64 qTm0, qTm1;
	int k;
	CTmcRTH_IndanOutput *cOut;
	BOOL bWhole;

	// J-5: /Time on the card - the queue is asynchronous: Pass/Blocks only enqueue, the wait for them is in
	// PortsDown of the next sub-step and is counted as EXCITE; only the sum EXCITE + SCATTER is meaningful.
	/// Время /Time на видеокарте: «возбуждение» включает ожидание ядер прошлого подтакта (очередь OpenCL
	/// асинхронна), поэтому показательна только сумма «возбуждение + рассеяние».
	for( k = 0; k < 4; k++ )
	{
		qTm0 = TmcTimingNow();
		if( !g_cGpuX.PortsDown( prUNode1 ) ) { cError.PutErrorMessage( g_cGpuX.csError ); return; };
		switch( k )
		{
			case 0: RunExciteInputs(); break;
			case 1: RunExciteInputs1(); break;
			case 2: RunExciteInputs2(); break;
			default: RunExciteInputs3(); break;
		};
		RunSaveInputs();
		if( IsError() ) return;
		if( !g_cGpuX.PortsUp( prUNode1 ) ) { cError.PutErrorMessage( g_cGpuX.csError ); return; };
		qTm1 = TmcTimingNow(); m_cTiming.cur.q[TMCT_EXCITE] += qTm1 - qTm0;
		if( ( !g_cGpuX.Pass( k ) )||( !g_cGpuX.Restore() )||( !g_cGpuX.Blocks( k ) )||( !g_cGpuX.RestorePlane() ) ) { cError.PutErrorMessage( g_cGpuX.csError ); return; };	// B-1: RestorePlane
		qTm0 = TmcTimingNow(); m_cTiming.cur.q[TMCT_SCATTER] += qTm0 - qTm1;
//  the whole grid is needed only for FIELDS and inside the directional pattern window
		cOut = cIndan.GetOutput();
		bWhole = cOut->bOutField;
		if( ( cOut->GetFieldIntegrated().bIsFieldDistr() )&&
			( dTCurrent >= cOut->GetFieldIntegrated().GetTmin() )&&
			( dTCurrent <= cOut->GetFieldIntegrated().GetTmax() ) ) bWhole = TRUE;
		if( bWhole && ( !g_cGpuX.Download( prUNode1 ) ) ) { cError.PutErrorMessage( g_cGpuX.csError ); return; };
		PutField( k );
		m_cTiming.cur.q[TMCT_FIELDFILE] += TmcTimingNow() - qTm0;
		DistributionIntegrated( k );
		nTCurrent++;
		dTCurrent += dT;
	};

	return;
}
