// TmcRTHNodeDiel.cpp : implementation file
//

/// @file TmcRTHNodeDiel.cpp
/// @brief Блоки задания H-ядра: построение узлов (диэлектрик, металл, поглотитель,
///        магнитная стенка, порт), быстрое построение EPSFAST, файлы `.eps` и `.sig`,
///        возбуждение портов и граничные условия блоков на каждом подтакте.
///
/// Часть модуля «Счётное ядро H». Реализует `CTmcRTHNodeDiel` (описание методов —
/// в `tmcrthnodediel.h`). У ядра X — свой файл с тем же именем.
/// @see https://www.tamic.ru/docs/code/kernel-h.html#blocks
#include "StdAfx.h"
#include "PlanRT_H.h"
#include <Tmcgrviw.h>
#include "TmcRTHNodeDiel.h"
#include "TmcRTH_Indan.h"
#include "TmcRTHRectNode.h"
// D-4: the missing neighbour of a corner node of a port is an explicit 0 (old: the node (0,0), pr1[0])
/// Нулевые «соседи» для крайних узлов порта: у первого и последнего узла апертуры
/// нет соседа вдоль порта, и вместо него читается 0.
static _ELEM_VAL_RTH g_rZeroNeighbour[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
// B-16: is the point (x, y) outside the region, more than half a step beyond its border? Such parts of a block
// are dropped (old: they were pressed to the border column).
/// @brief Проверяет, лежит ли точка дальше чем на полшага за границей области (такие части блоков отбрасываются).
/// @param[in] cParam параметры сетки (границы области, шаг)
/// @param[in] x      координата X, м
/// @param[in] y      координата Y, м
/// @return TRUE — точка вне области.
static BOOL TmcB16Out( CTmcRTH_IndanParam &cParam, double x, double y )
{
	return ( x < cParam.rXmin - 0.5*cParam.rDelta )||( x > cParam.rXmax + 0.5*cParam.rDelta )||
		( y < cParam.rYmin - 0.5*cParam.rDelta )||( y > cParam.rYmax + 0.5*cParam.rDelta );
}
// R-2: neighbours of a node along the sides (1 below, 2 right, 3 above, 4 left), in steps Delta.
/// Сдвиги соседей узла по сторонам в шагах сетки: 1 — снизу, 2 — справа, 3 — сверху, 4 — слева
/// (нумерация рисунка автора к Р-2).
static const double g_dR2SideX[4] = { 0., 1., 0., -1. };
/// Сдвиги соседей по Y (в паре с g_dR2SideX).
static const double g_dR2SideY[4] = { -1., 0., 1., 0. };
/// @brief Р-2: вес узла диэлектрика круга или многоугольника по числу соседей по сторонам, попавших в блок.
///
/// Правило автора (29.10.2026), одно для всех примитивов — оно же у прямоугольника (SetRect):
/// 4 соседа — узел внутри, вес 1; 3 — узел на стороне, 1/2 (половина его ячейки в блоке);
/// 2 — угол, 1/4 (так же и два противоположных соседа — блок толщиной в один узел); 1 — вес 1.
/// Других весов нет (3/4 не бывает). 0 соседей автор не называл — принято 1.
/// @param[in] nSide число соседей (x +- Delta, y), (x, y +- Delta) внутри блока, 0..4
/// @return доля проводимостей и шлейфов блока, которую получает узел.
/// @see https://www.tamic.ru/docs/code/kernel-h.html#blocks
static _real TmcR2NodeWeight( int nSide )
{
	if( nSide == 3 ) return (_real)(0.5);
	if( nSide == 2 ) return (_real)(0.25);
	return (_real)(1.0);
}
#include <math.h>
#include <limits.h>
#include <float.h>
#include <EXPR.H>
#include "Pl_iofor.h"
#include <TmcCalcTiming.h>	// TIMING
FILE *TmcOutTGet( const char *pszName );	// T-5, T-9: TmcRTHRectNode.cpp
FILE *TmcFopenWait( const char *pszName, const char *pszMode, BOOL bDenyWrite, int *pnErr );
CString TmcOpenFailText( const char *pszName, int nErr );
#include <thread>		// EPSFAST
#include <atomic>		// EPSFAST
#include <vector>		// EPSFAST

/// @name Форма блока для EPSFAST
///@{
/// Прямоугольник (RECT_STAT*).
#define EPSFAST_RECT		0	// EPSFAST: block shapes
/// Кольцевой сектор (CIRCLE_STAT*).
#define EPSFAST_CIRC		1
/// Многоугольник (POLYGON_STAT*).
#define EPSFAST_POLY		2
///@}
/// @name Вид выражения eps для EPSFAST
///@{
/// В выражении нет x, y, r, f: одно значение на блок.
#define EPSFAST_CONST		0	// EPSFAST: no x, y, r, f in the expression - one value per block
/// Есть x, y, r или f, нет `=`: узлы независимы, счёт в нескольких потоках.
#define EPSFAST_PARALLEL	1	//          x, y, r, f, no '=' - nodes are independent
/// Есть `=`: переменные переходят от узла к узлу, один поток в исходном порядке.
#define EPSFAST_SERIAL		2	//          '=' - variables live from node to node, keep the order
///@}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcRTHNodeDiel

CTmcRTHNodeDiel::CTmcRTHNodeDiel()
{
	cError.Clear();
	dUnormir = 1.;						//normir koef for U falling and scattering
	nBlock   = 0;
	nNumNode = 0;
	pcNodeDielOne = NULL;
	dYInput = 0.;
	nAbsorberNoWave = 0;	// R-1
	dTmin = -INT_MAX;					// T_min for excite input block
	dTmax =  INT_MAX;					// T_max for excite input block
	dXCenter = 0.0;						// x for center block
	dYCenter = 0.0;						// y for center block
	csBlock.Format("");					// block Type
	csEpsExpr.Format("");				// expression for eps
	csW.Format("");						// angular speed
	csVx.Format("");					// linear speed	for block center of rotation x0, y0
	csVy.Format("");					// linear speed	for block center of rotation x0, y0
	nX0 = 0;							// x0 for centre of rotation
	nY0 = 0;							// y0 for centre of rotation
	nTxCurrent = 0;						// counter for x moving
	nTyCurrent = 0;						// counter for y moving
	nTxMove = INT_MAX;					// period for x moving
	nTyMove = INT_MAX;					// period for y moving
	nType = CTMCRTH_BLCKNTYPE_METAL;	//	= -1 for metall;
	dWidthWaveg = 1.0;					// width for input block
	dX0 = 0.0;
	dY0 = 0.0;
	dY1_InputAdmitance = 1.0;
	bSigFile = FALSE;					// incident wave from file
	pdSigT = NULL;
	pdSigV = NULL;
	nSigN = 0;
	csFileNameSig.Format("");
	csSigPath.Format("");
	pbIsStop = NULL;
	csFileNameEps.Format("");
	m_pqSignalFileTicks = NULL;	// TIMING
	m_pqEpsFileTicks = NULL;		// TIMING
	m_pqEpsReadTicks = NULL;		// TIMING
	m_pnEpsWritten = NULL;		// TIMING
	m_pnEpsRead = NULL;			// TIMING
	return;
}

CTmcRTHNodeDiel::~CTmcRTHNodeDiel()
{
	DeleteData();
}


BEGIN_MESSAGE_MAP(CTmcRTHNodeDiel, CWnd)
	//{{AFX_MSG_MAP(CTmcRTHNodeDiel)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTmcRTHNodeDiel message handlers

void CTmcRTHNodeDiel::DeleteData( void )
{
	cError.Clear();
	dUnormir = 1.;						//normir koef for U falling and scattering
	nBlock   = 0;
	nNumNode = 0;
	if( pcNodeDielOne != NULL )
	{
		delete []pcNodeDielOne;
		pcNodeDielOne = NULL;
	};
	dYInput = 0.;
	dTmin = -INT_MAX;					// T_min for excite input block
	dTmax =  INT_MAX;					// T_max for excite input block
	dXCenter = 0.0;						// x for center block
	dYCenter = 0.0;						// y for center block
	csBlock.Format("");					// block Type
	csEpsExpr.Format("");				// expression for eps
	csW.Format("");						// angular speed
	csVx.Format("");					// linear speed	for block center of rotation x0, y0
	csVy.Format("");					// linear speed	for block center of rotation x0, y0
	nX0 = 0;							// x0 for centre of rotation
	nY0 = 0;							// y0 for centre of rotation
	nTxCurrent = 0;						// counter for x moving
	nTyCurrent = 0;						// counter for y moving
	nTxMove = INT_MAX;					// period for x moving
	nTyMove = INT_MAX;					// period for y moving
	nType = CTMCRTH_BLCKNTYPE_METAL;	//	= -1 for metall;
	pbIsStop = NULL;
	csFileNameEps.Format("");
	
	DeleteSignalData();
	return;
}






CTmcLibError &CTmcRTHNodeDiel::SetBlock( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut, _real *prYNode, volatile BOOL *pbStopFlag, CString csCurrentPath )
{
	DeleteData();
	double r;
	int i;
	csSigPath = csCurrentPath;			// remember path for signal file
	BOOL bEpsFast = FALSE;				// EPSFAST

	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};
	nBlock  = pcNextBlockList->GetnBlock();
	csBlock = pcNextBlockList->GetcsBlock();
	csVx    = pcNextBlockList->GetcsVx();
	csVy    = pcNextBlockList->GetcsVy();
	csW     = pcNextBlockList->GetcsW();
	nType   = pcNextBlockList->GetnType();
	csEpsExpr = pcNextBlockList->GetcsEpsExpr();
	dX0 = pcNextBlockList->GetX0();
	dY0 = pcNextBlockList->GetY0();
	pbIsStop = pbStopFlag;
	// Узел сетки, ближайший к точке привязки блока (X0, Y0).
	r = (pcNextBlockList->GetX0() - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big x0 in %s section for block %d", CTMCRTH_INDANMET_LINKLIST, nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX0 = (int)(r);
	r = (pcNextBlockList->GetY0() - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big y0 in %s section for block %d", CTMCRTH_INDANMET_LINKLIST, nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY0 = (int)(r);
	
	i = 0;
	// Выбор построителя по типу блока и ключевому слову примитива.
	switch( nType )
	{
		case  CTMCRTH_BLCKNTYPE_METAL    :
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTAT, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == 0 ) 
			{
				cError = SetRectMetal( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTT, strlen(CTMCRTH_INDANBLCK_POLYGSTT) ) == 0 )
			{
				cError = SetPolyMetal( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTAT, strlen(CTMCRTH_INDANBLCK_CIRCSTAT) ) == 0 )
			{
				cError = SetCircMetal( pcNextBlockList, cParam);
				i = 1;
			};
			break;
		case  CTMCRTH_BLCKNTYPE_ABSORBER :
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTAT, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == 0 ) 
			{
				cError = SetRectAbsorber( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTT, strlen(CTMCRTH_INDANBLCK_POLYGSTT) ) == 0 ) 
			{
				cError = SetPolyAbsorber( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTAT, strlen(CTMCRTH_INDANBLCK_CIRCSTAT) ) == 0 )
			{
				cError = SetCircAbsorber( pcNextBlockList, cParam);
				i = 1;
			};
			break;
		case  CTMCRTH_BLCKNTYPE_MAGNETIC :
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTAT, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == 0 ) 
			{
				cError = SetRectMagnetic( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTT, strlen(CTMCRTH_INDANBLCK_POLYGSTT) ) == 0 ) 
			{
				cError = SetPolyMagnetic( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTAT, strlen(CTMCRTH_INDANBLCK_CIRCSTAT) ) == 0 )
			{
				cError = SetCircMagnetic( pcNextBlockList, cParam);
				i = 1;
			};
			break;
		case  CTMCRTH_BLCKNTYPE_EPS      :
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_FILE, strlen(CTMCRTH_INDANBLCK_FILE) ) == 0 ) 
			{
				{ __int64 qTmF = TmcTimingNow(); cError = SetRectFile( pcNextBlockList, cParam, csCurrentPath ); TimingEpsRead( TmcTimingNow() - qTmF ); }	// TIMING
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTAT, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == 0 ) 
			{
				if( SetStatEpsFast( pcNextBlockList, cParam, csCurrentPath, prYNode, EPSFAST_RECT, 2, 0 ) ) bEpsFast = TRUE;	// EPSFAST
				else cError = SetRect( pcNextBlockList, cParam, csCurrentPath );
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTMOVE, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == 0 )
			{
				cError = SetRect( pcNextBlockList, cParam, csCurrentPath );
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTAT, strlen(CTMCRTH_INDANBLCK_CIRCSTAT) ) == 0 )
			{
				if( SetStatEpsFast( pcNextBlockList, cParam, csCurrentPath, prYNode, EPSFAST_CIRC, 2, 0 ) ) bEpsFast = TRUE;	// EPSFAST
				else cError = SetCirc( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCMOVE, strlen(CTMCRTH_INDANBLCK_CIRCMOVE) ) == 0 )
			{
				cError = SetCirc( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTT, strlen(CTMCRTH_INDANBLCK_POLYGSTT) ) == 0 )
			{
				if( SetStatEpsFast( pcNextBlockList, cParam, csCurrentPath, prYNode, EPSFAST_POLY, 2, 0 ) ) bEpsFast = TRUE;	// EPSFAST
				else cError = SetPoly( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGMOV, strlen(CTMCRTH_INDANBLCK_POLYGMOV) ) == 0 )
			{
				cError = SetPoly( pcNextBlockList, cParam);
				i = 1;
			};
			break;
		case  CTMCRTH_BLCKNTYPE_INPXLEFT :
			cError = SetInpXLeft( pcNextBlockList, cParam, cOut);
			i = 1;
			break;
		case  CTMCRTH_BLCKNTYPE_INPXRIGHT :
			cError = SetInpXRight( pcNextBlockList, cParam, cOut);
			i = 1;
			break;
		case  CTMCRTH_BLCKNTYPE_INPYTOP :
			cError = SetInpYTop( pcNextBlockList, cParam, cOut);
			i = 1;
			break;
		case  CTMCRTH_BLCKNTYPE_INPYBOT :
			cError = SetInpYBot( pcNextBlockList, cParam, cOut);
			i = 1;
			break;
		default:
			{
				CString csBuf;
				csBuf.Format("Unknow type block %d or for Block type %s the non-authorized link %s", nBlock, csBlock, csEpsExpr);
				cError.PutErrorMessage(csBuf);
			};
			break;
	};

	if( i != 1 )
	{
		CString csBuf;
		csBuf.Format("Unknow type block %d or for Block type %s the non-authorized link %s", nBlock, csBlock, csEpsExpr);
		cError.PutErrorMessage(csBuf);
	};


	{
		CString csBuf;
		csBuf.Format("Set block %d; points:%.0f; Type or eps = {%s}", nBlock, (double)nNumNode, csEpsExpr);	// BIGBLOCK
		PutTrace(csBuf);
	};
	// EPSFAST: вклад уже в prYNode, массива узлов нет — блок готов.
	if( bEpsFast )	// EPSFAST: eps is already in prYNode, no node array was built
	{
		CString csBuf;
		csBuf.Format("Set block %d; points:%.0f; Type or eps = {%s}", nBlock, (double)nNumNode, csEpsExpr);
		PutTrace(csBuf);
		pcNextBlockList->SetnMemory( sizeof(CTmcRTHNodeDiel) );
		nNumNode = 0;
		csBuf.Format(" ");
		PutStatistics(csBuf);
		return cError;
	};

	// Все узлы блока (и соседи, которые читает порт) должны лежать в сетке.
	if( ( !cError.IsError() )&&( pcNodeDielOne != NULL )&&( nNumNode > 0 ) )	// A-6: the nodes of the block lie in the grid
	{
		int nXg = cParam.CalcnX();
		TmcIdx nGrid = ((TmcIdx)nXg)*cParam.CalcnY();
		TmcIdx k, g, gBad = -1;
		double dD = 0.99999*cParam.dGetDelta();
		for( k = 0; ( k < nNumNode )&&( gBad < 0 ); k++ )
		{
			g = pcNodeDielOne[k].nNodeGlobal;
			if( ( g < 0 )||( g >= nGrid ) ) { gBad = g; break; };
			switch( nType )	// the neighbours the port reads and the node "behind" it that is set to zero
			{
				case CTMCRTH_BLCKNTYPE_INPYTOP:
					if( ( g - nXg < 0 )||( ( cParam.CalcY(g) < cParam.rYmax - dD )&&( g + nXg >= nGrid ) ) ) gBad = g;
					break;
				case CTMCRTH_BLCKNTYPE_INPYBOT:
					if( ( g + nXg >= nGrid )||( ( cParam.CalcY(g) > cParam.rYmin + dD )&&( g - nXg < 0 ) ) ) gBad = g;
					break;
				case CTMCRTH_BLCKNTYPE_INPXRIGHT:
					if( ( g - 1 < 0 )||( ( cParam.CalcX(g) < cParam.rXmax - dD )&&( g + 1 >= nGrid ) ) ) gBad = g;
					break;
				case CTMCRTH_BLCKNTYPE_INPXLEFT:
					if( ( g + 1 >= nGrid )||( ( cParam.CalcX(g) > cParam.rXmin + dD )&&( g - 1 < 0 ) ) ) gBad = g;
					break;
			};
		};
		if( gBad >= 0 || ( k < nNumNode ) )
		{
			CString csBuf;
			csBuf.Format("Block %d: node %.0f (row %.0f, column %.0f) or its neighbour is outside the grid %d x %d",
				nBlock, (double)g, (double)( g/nXg ), (double)( g%nXg ), nXg, cParam.CalcnY() );
			cError.PutErrorMessage(csBuf);
		};
	};

	// Общая доводка: без повторов, коды узлов, eps -> prYNode, профиль порта,
	// освобождение узлов стационарного диэлектрика.
	DeleteDublicateData();
	CalculatedXYCenter( cParam );
	SetnType();
	SetYLineForDiel( cParam, prYNode );
	SetYForInput( cParam );
	pcNextBlockList->SetnMemory( sizeof(CTmcRTHNodeDiel)+(sizeof(sTmcRTHNodeDielOne))*((TmcIdx)nNumNode) );	// BIGBLOCK
	DeleteStatDielData();
//		CString csBuf1;
	{
		CString csBuf;
		csBuf.Format(" ");
		PutStatistics(csBuf);
	};
	return cError;
}

//////////////////////////////////////////////////////////////////////////////
// EPSFAST: fast build of stationary eps blocks
//          (RECT_STAT, CIRCLE_STAT, POLYGON_STAT and their _N/_B/_Y variants).
//
// prYNode gets the same numbers as from SetRect/SetCirc/SetPoly + SetYLineForDiel,
// but
//  1) an expression without x, y, r, f is evaluated once per block;
//  2) eps goes straight into prYNode - the node array (56 bytes per node) that
//     DeleteStatDielData threw away afterwards is not built at all;
//  3) nodes are visited row by row, i.e. one after another in memory;
//  4) an expression with x, y, r, f is evaluated in several threads, each thread
//     with its own interpreter context (exprint: expr_thread_ctx_begin).
// Anything unusual (repeated grid nodes, geometry errors, no nodes in the block)
// returns FALSE, and the original code then builds the block and gives the
// original messages. Environment: TMC_EPS_FAST=0 - original code for every
// block; TMC_EPS_THREADS=n - number of threads (default: all processors but 1 of 8 - EPSFAST2).
//////////////////////////////////////////////////////////////////////////////

/// Сколько строк описанного прямоугольника рабочий поток берёт за один раз.
#define EPSFAST_CHUNK		8		// rows taken by a thread at once
/// Наибольшее число рабочих потоков EPSFAST (предел WaitForMultipleObjects).
#define EPSFAST_MAXTHREADS	64		// WaitForMultipleObjects limit
/// Порог EPSFAST: если в описанном прямоугольнике блока меньше узлов, считается в одном
/// потоке. Переопределяется переменной окружения `TMC_EPS_SMALL`.
#define EPSFAST_SMALL		1000	// fewer nodes - one thread (EPSFAST2: measured 2026-10-22, threads win from ~1000 nodes)

/// @brief Задание быстрого построения одного eps-блока: геометрия, массивы строк и столбцов,
///        общие счётчики рабочих потоков.
///
/// Создаётся на стеке `SetStatEpsFast`, передаётся всем рабочим потокам; счётчики —
/// атомарные, остальные поля только читаются.
/// @see https://www.tamic.ru/docs/code/kernel-h.html#epsfast
struct sEpsFastJob
{
	/// Параметры сетки (не владеет).
	CTmcRTH_IndanParam *pParam;
	/// Массив проводимостей ядра, куда прибавляется вклад узлов.
	_real *prYNode;
	/// Вклад узла n идёт в `prYNode[nStride*n + nOffset]`.
	int nStride, nOffset;		// eps of node n goes to prYNode[nStride*n + nOffset]
	/// Форма (`EPSFAST_RECT/CIRC/POLY`) и вид выражения (`EPSFAST_CONST/PARALLEL/SERIAL`).
	int nShape, nKind;
	/// Размер описанного прямоугольника блока в узлах.
	int nX, nY;
	/// Выражение eps.
	const char *pszExpr;
	/// Координаты столбцов и строк так, как их строит прежний код, м.
	double *pdX, *pdY;			// node coordinates exactly as the original loops make them
	/// Номер узла сетки = `pnXIdx[i] + pnYRow[j]`.
	TmcIdx *pnXIdx, *pnYRow;	// grid node = pnXIdx[i] + pnYRow[j]
	/// Координаты, подставляемые в выражение eps (относительно X0, Y0), м.
	double *pdEpsX, *pdEpsY;	// coordinates passed to the eps expression
	/// Круг: Rmin^2, Rmax^2 (м^2), FImin, FImax (рад), шаг сетки (м).
	double dRRmin, dRRmax, dFImin, dFImax, dDelta;	// circle
	/// Многоугольник: координаты вершин (локальные), м.
	double *pPX, *pPY;			// polygon
	/// Многоугольник: число вершин.
	int nXY;
	/// Значение выражения без x, y, r, f (считается один раз).
	_real rEpsConst;
	/// Признак «расчёт идёт» ядра; FALSE — потоки прекращают работу.
	volatile BOOL *pbIsStop;
	/// Следующая свободная строка.
	std::atomic<int> nNextRow;
	/// Число обработанных строк (для хода в строке состояния).
	std::atomic<int> nRowsDone;
	/// Не 0 — ошибка выражения в каком-то узле.
	std::atomic<int> nError;
	/// Не 0 — построение прервано пользователем.
	std::atomic<int> nCancel;
	/// Число узлов, вошедших в блок.
	std::atomic<long long> nCount;
};
/// @var sEpsFastJob::nOffset
/// Смещение ячейки eps в записи узла массива `prYNode`.

/// @var sEpsFastJob::nKind
/// Вид выражения: `EPSFAST_CONST`, `EPSFAST_PARALLEL` или `EPSFAST_SERIAL`.

/// @var sEpsFastJob::nY
/// Число строк описанного прямоугольника блока.

/// @var sEpsFastJob::pdY
/// Координаты строк описанного прямоугольника, м.

/// @var sEpsFastJob::pnYRow
/// Номер узла сетки начала строки j (`iy*nX`).

/// @var sEpsFastJob::pdEpsY
/// Координата Y строки, подставляемая в выражение eps (относительно Y0), м.

/// @var sEpsFastJob::dRRmax
/// Круг: квадрат внешнего радиуса Rmax^2, м^2.

/// @var sEpsFastJob::dFImin
/// Круг: начальный угол сектора, рад, [0, 2*pi).

/// @var sEpsFastJob::dFImax
/// Круг: конечный угол сектора, рад.

/// @var sEpsFastJob::dDelta
/// Шаг сетки, м.

/// @var sEpsFastJob::pPY
/// Многоугольник: координаты Y вершин (локальные), м.

int CTmcRTHNodeDiel::EpsFastExprKind( const char *pszExpr )
{
	static const char *aszWord[] = { "return(", "while(", "log10(", "sqrt(", "sinh(", "cosh(", "tanh(",
		"asin(", "acos(", "atan(", "abs(", "exp(", "log(", "sin(", "cos(", "tan(", "if(" };
	CString cs = pszExpr;
	int i;

	if( strchr( pszExpr, '=' ) != NULL ) return EPSFAST_SERIAL;	// variables live from node to node
	for( i = 0; i < (int)(sizeof(aszWord)/sizeof(aszWord[0])); i++ ) cs.Replace( aszWord[i], " " );
	if( cs.FindOneOf( "xyrf" ) >= 0 ) return EPSFAST_PARALLEL;
	return EPSFAST_CONST;
}

unsigned CTmcRTHNodeDiel::EpsFastThreads( void )
{
	unsigned n = std::thread::hardware_concurrency();
	char *psz = getenv( "TMC_EPS_THREADS" );

	if( ( psz != NULL )&&( atoi( psz ) > 0 ) ) n = (unsigned)atoi( psz );
	else if( n > 1 ) n -= ( n + 7 )/8;	// EPSFAST2: leave processors to Windows (1 of 2..8, 2 of 9..16, ...)
	if( n < 1 ) n = 1;
	if( n > EPSFAST_MAXTHREADS ) n = EPSFAST_MAXTHREADS;
	return n;
}

// The same as CalcEps, but without cError, so it can run in any thread.
int CTmcRTHNodeDiel::EpsFastEval( const char *pszExpr, double x, double y, CTmcRTH_IndanParam &cParam, char *szBuf, char *szBuf1, _real *pr )
{
	double r1, r, f;

	strcpy( szBuf, pszExpr );

	x /=cParam.rLongUnit;
	y /=cParam.rLongUnit;
	r = sqrt( x*x + y*y );
	f = atan2__1( y, x)/cParam.rAngleUnit;
	// B-11, old (6 digits):
	expr_set_per( (char *)"x", 1, (_real)x );	// B-11: the full precision, without a string
	expr_set_per( (char *)"y", 1, (_real)y );
	expr_set_per( (char *)"r", 1, (_real)r );
	expr_set_per( (char *)"f", 1, (_real)f );
	if( i1nte_atof_1( szBuf, &r1 ) != 0 ) return -1;
	*pr = (_real)r1;
	return 0;
}

// Is node (i, j) of the block's bounding box in the block, and its weight
// (the original SetRect/SetCirc/SetPoly rY).
BOOL CTmcRTHNodeDiel::EpsFastNode( sEpsFastJob *pJob, int i, int j, _real *prY )
{
	double x, y, x1, y1, rRxy, rFxy, rRxy1, rFxy1, d;
	double *pX, *pY;
	int k, k1, nXY;
	int nSideFast;	// R-2: side neighbours of the node in the polygon
	BOOL bIn;

	switch( pJob->nShape )
	{
		case EPSFAST_RECT:
			*prY = (_real)(1.0);
			if( i == 0 )               *prY *= (_real)(0.5);
			if( i == (pJob->nX - 1) )  *prY *= (_real)(0.5);
			if( j == 0 )               *prY *= (_real)(0.5);
			if( j == (pJob->nY - 1) )  *prY *= (_real)(0.5);
			return TRUE;

		case EPSFAST_CIRC:
			x = pJob->pdX[i];
			y = pJob->pdY[j];
			d = pJob->dDelta;
			rRxy = x*x + y*y;
			rFxy = atan2__1( y, x);
			if( !( ( rRxy >= pJob->dRRmin )&&( rRxy <= pJob->dRRmax)&&
				( rFxy >= pJob->dFImin )&&( rFxy <= pJob->dFImax )&&
				pJob->pParam->IsnInRegion( pJob->pnXIdx[i] + pJob->pnYRow[j] ) ) ) return FALSE;
			// R-2 (author 29.10): the same weight as in SetCirc - by the side neighbours (old: diagonal points,
			// under #if 0 below).
			for( k = 0, k1 = 0; k < 4; k++ )
			{
				x1 = x + g_dR2SideX[k]*d;
				y1 = y + g_dR2SideY[k]*d;
				rRxy1 = x1*x1 + y1*y1;
				rFxy1 = atan2__1( y1, x1);
				if( ( rRxy1 >= pJob->dRRmin )&&( rRxy1 <= pJob->dRRmax)&&
					( rFxy1 >= pJob->dFImin )&&( rFxy1 <= pJob->dFImax )) k1++;
			};
			*prY = TmcR2NodeWeight( k1 );
			return TRUE;

		case EPSFAST_POLY:
			x = pJob->pdX[i];
			y = pJob->pdY[j];
			d = pJob->dDelta;
			pX = pJob->pPX;
			pY = pJob->pPY;
			nXY = pJob->nXY;
			for( k = 2, bIn = FALSE; k < nXY; k++ )
			{
				if( IsPointInTr( pX[0], pY[0], pX[k-1], pY[k-1], pX[k], pY[k], x, y )&&
					pJob->pParam->IsnInRegion( pJob->pnXIdx[i] + pJob->pnYRow[j] ) )
				{
					bIn = TRUE;
					break;
				};
			};
			if( !bIn ) return FALSE;
			// R-2 (author 29.10): the same weight as in SetPoly - by the side neighbours (old: diagonal points,
			// under #if 0 below).
			for( k = 0, nSideFast = 0; k < 4; k++ )
			{
				x1 = x + g_dR2SideX[k]*d;
				y1 = y + g_dR2SideY[k]*d;
				for( k1 = 2; k1 < nXY; k1++ )
					if( IsPointInTr( pX[0], pY[0], pX[k1-1], pY[k1-1], pX[k1], pY[k1], x1, y1 ) ) { nSideFast++; break; };
			};
			*prY = TmcR2NodeWeight( nSideFast );
			return TRUE;
	};
	return FALSE;
}

// One thread: takes rows until none are left. Grid nodes of the block are all
// different (checked before), so threads never write the same prYNode element.
void CTmcRTHNodeDiel::EpsFastRun( sEpsFastJob *pJob )
{
	EXPR_THREAD_CTX sCtx;
	char *szBuf = NULL, *szBuf1 = NULL;
	BOOL bExpr = ( pJob->nKind != EPSFAST_CONST );
	long long nCount = 0;
	int i, j, j0, j1;
	_real rY, rEps = pJob->rEpsConst;
	TmcIdx nRow;
	double y_line;

	if( bExpr )
	{
		szBuf  = new char[CTMCRTH_INDANSBUF];
		szBuf1 = new char[CTMCRTH_INDANSBUF];
		expr_thread_ctx_begin( &sCtx );
	};

	while( ( pJob->nError == 0 )&&( pJob->nCancel == 0 ) )
	{
		j0 = pJob->nNextRow.fetch_add( EPSFAST_CHUNK );
		if( j0 >= pJob->nY ) break;
		j1 = j0 + EPSFAST_CHUNK;
		if( j1 > pJob->nY ) j1 = pJob->nY;
		for( j = j0; ( j < j1 )&&( pJob->nError == 0 ); j++ )
		{
			if( !pJob->pbIsStop[0] )
			{
				pJob->nCancel = 1;
				break;
			};
			nRow = pJob->pnYRow[j];
			for( i = 0; i < pJob->nX; i++ )
			{
				if( !EpsFastNode( pJob, i, j, &rY ) ) continue;
				if( bExpr )
				{
					if( EpsFastEval( pJob->pszExpr, pJob->pdEpsX[i], pJob->pdEpsY[j], *(pJob->pParam), szBuf, szBuf1, &rEps ) != 0 )
					{
						pJob->nError = 1;
						break;
					};
				};
				y_line = rY *( rEps );
				pJob->prYNode[ pJob->nStride*(pJob->pnXIdx[i] + nRow) + pJob->nOffset ] += (_real)(y_line);
				nCount++;
			};
			pJob->nRowsDone++;
		};
	};

	if( bExpr )
	{
		expr_thread_ctx_end();
		delete []szBuf;
		delete []szBuf1;
	};
	pJob->nCount += nCount;
}

// TRUE  - the block is built (or cError holds the reason it could not be);
// FALSE - nothing was done, build the block the original way.
BOOL CTmcRTHNodeDiel::SetStatEpsFast( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CString csCurrentPath, _real *prYNode, int nShape, int nStride, int nOffset )
{
	if( cError.IsError() ) return FALSE;
	if( ( pcNextBlockList == NULL )||( prYNode == NULL ) ) return FALSE;
	{
		if( !EpsFastEnabled() ) return FALSE;
	};

	int nXmin, nXmax, nYmin, nYmax;
	int i, j, nX, nY;
	double dXmin, dXmax, dYmin, dYmax, r;
	double dRmin = 0, dRmax = 0, dFImin = 0, dFImax = 0;
	double *pX = NULL, *pY = NULL;
	int nXY = 0;

	////////////////////////// geometry: the same numbers as the original code
	switch( nShape )
	{
		case EPSFAST_RECT:
			dXmin = pcNextBlockList->GetXmin() + pcNextBlockList->GetX0();
			if( dXmin < cParam.rXmin ) dXmin = cParam.rXmin;
			dXmax = pcNextBlockList->GetXmax() + pcNextBlockList->GetX0();
			if( dXmax > cParam.rXmax ) dXmax = cParam.rXmax;
			dYmin = pcNextBlockList->GetYmin() + pcNextBlockList->GetY0();
			if( dYmin < cParam.rYmin ) dYmin = cParam.rYmin;
			dYmax = pcNextBlockList->GetYmax() + pcNextBlockList->GetY0();
			if( dYmax > cParam.rYmax ) dYmax = cParam.rYmax;
			break;

		case EPSFAST_CIRC:
			dRmin = pcNextBlockList->GetXmin();
			dRmax = pcNextBlockList->GetXmax();
			dFImin = pcNextBlockList->GetYmin();
			if( dFImin < 0 )
			{
				i = (int)(dFImin/(2*3.141592653589) + 1);
				dFImin += i*(2*3.141592653589);
			};
			if( dFImin >= (2*3.141592653589) )
			{
				i = (int)(dFImin/(2*3.141592653589));
				dFImin -= i*(2*3.141592653589);
			};
			dFImax = pcNextBlockList->GetYmax();
			if( dFImax <= 0 )
			{
				i = (int)(dFImax/(2*3.141592653589) + 1);
				dFImax += i*(2*3.141592653589);
			};
			// B-19: FImax up to 2*pi + 1e-12 is a full circle (full pi, the same constant as in the parser).
			// Old (the exact 2*pi became ~1e-12, an empty block or a ray):
			//   if( dFImax > (2*3.141592653589) )
			//   { i = (int)(dFImax/(2*3.141592653589)); dFImax -= i*(2*3.141592653589); };
			if( dFImax > (2*3.14159265358979323846 + 1e-12) )
			{
				i = (int)(dFImax/(2*3.14159265358979323846));
				dFImax -= i*(2*3.14159265358979323846);
			};
			dXmin = - pcNextBlockList->GetXmax() + pcNextBlockList->GetX0();
			if( dXmin < cParam.rXmin ) dXmin = cParam.rXmin;
			dXmax = pcNextBlockList->GetXmax() + pcNextBlockList->GetX0();
			if( dXmax > cParam.rXmax ) dXmax = cParam.rXmax;
			dYmin = - pcNextBlockList->GetXmax() + pcNextBlockList->GetY0();
			if( dYmin < cParam.rYmin ) dYmin = cParam.rYmin;
			dYmax = pcNextBlockList->GetXmax() + pcNextBlockList->GetY0();
			if( dYmax > cParam.rYmax ) dYmax = cParam.rYmax;
			break;

		case EPSFAST_POLY:
			if( ( pX = pcNextBlockList->GetpX() ) == NULL ) return FALSE;
			if( ( pY = pcNextBlockList->GetpY() ) == NULL ) return FALSE;
			if( ( nXY = pcNextBlockList->GetnXY() ) < 3 ) return FALSE;
			for( i = 0, dXmin = pX[0], dXmax = pX[0], dYmin = pY[0], dYmax = pY[0]; i < nXY; i++ )
			{
				if( pX[i] > dXmax ) dXmax = pX[i];
				if( pX[i] < dXmin ) dXmin = pX[i];
				if( pY[i] > dYmax ) dYmax = pY[i];
				if( pY[i] < dYmin ) dYmin = pY[i];
			}
			dXmin += pcNextBlockList->GetX0();
			if( dXmin < cParam.rXmin ) dXmin = cParam.rXmin;
			dXmax += pcNextBlockList->GetX0();
			if( dXmax > cParam.rXmax ) dXmax = cParam.rXmax;
			dYmin += pcNextBlockList->GetY0();
			if( dYmin < cParam.rYmin ) dYmin = cParam.rYmin;
			dYmax += pcNextBlockList->GetY0();
			if( dYmax > cParam.rYmax ) dYmax = cParam.rYmax;
			break;

		default:
			return FALSE;
	};

	r = (dXmin - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) return FALSE;
	nXmin = (int)(r);
	r = (dXmax - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) return FALSE;
	nXmax = (int)(r);
	r = (dYmin - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) return FALSE;
	nYmin = (int)(r);
	r = (dYmax - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) return FALSE;
	nYmax = (int)(r);
	if( nXmax < nXmin ) return FALSE;
	nX = nXmax - nXmin + 1;
	if( nYmax < nYmin ) return FALSE;
	nY = nYmax - nYmin + 1;
	if( ( nShape == EPSFAST_RECT )&&( ((double)nX)*((double)nY) > ((double)TMC_IDX_MAX)/64. ) ) return FALSE;

	////////////////////////// coordinates and grid nodes of the rows and columns
	std::vector<double> vdX( nX ), vdY( nY ), vdEpsX( nX ), vdEpsY( nY );
	std::vector<TmcIdx> vnXIdx( nX ), vnYRow( nY );
	double x, y;

	if( nShape == EPSFAST_RECT )
	{
		for( i = 0; i < nX; i++ ) vdX[i] = dXmin + (i*cParam.rDelta);
		for( j = 0; j < nY; j++ ) vdY[j] = dYmin + (j*cParam.rDelta);
		for( i = 0; i < nX; i++ ) vnXIdx[i] = cParam.CalcnNodeGlobal( (_real)vdX[i], (_real)cParam.rYmin );
		for( j = 0; j < nY; j++ ) vnYRow[j] = cParam.CalcnNodeGlobal( (_real)cParam.rXmin, (_real)vdY[j] );
	}
	else
	{
		for( i = 0, x = dXmin - pcNextBlockList->GetX0(); i < nX; i++, x+=cParam.rDelta ) vdX[i] = x;
		for( j = 0, y = dYmin - pcNextBlockList->GetY0(); j < nY; j++, y+=cParam.rDelta ) vdY[j] = y;
		for( i = 0; i < nX; i++ ) vnXIdx[i] = cParam.CalcnNodeGlobal( (_real)(vdX[i]+pcNextBlockList->GetX0()), (_real)cParam.rYmin );
		for( j = 0; j < nY; j++ ) vnYRow[j] = cParam.CalcnNodeGlobal( (_real)cParam.rXmin, (_real)(vdY[j]+pcNextBlockList->GetY0()) );
	};
	for( i = 1; i < nX; i++ ) if( vnXIdx[i] <= vnXIdx[i-1] ) return FALSE;	// a grid node would get eps twice
	for( j = 1; j < nY; j++ ) if( vnYRow[j] <= vnYRow[j-1] ) return FALSE;
	for( i = 0; i < nX; i++ ) vdEpsX[i] = cParam.CalcX( vnXIdx[i] ) - dX0;
	for( j = 0; j < nY; j++ ) vdEpsY[j] = cParam.CalcY( vnYRow[j] ) - dY0;

	////////////////////////// the job
	sEpsFastJob sJob;
	CString csExpr = csEpsExpr;
	_real rY;
	int iFirst = -1, jFirst = -1, iLast = -1, jLast = -1;

	sJob.pParam  = &cParam;
	sJob.prYNode = prYNode;
	sJob.nStride = nStride;
	sJob.nOffset = nOffset;
	sJob.nShape  = nShape;
	sJob.nKind   = EpsFastExprKind( csExpr );
	sJob.nX = nX;
	sJob.nY = nY;
	sJob.pszExpr = (LPCTSTR)csExpr;
	sJob.pdX = &vdX[0];
	sJob.pdY = &vdY[0];
	sJob.pnXIdx = &vnXIdx[0];
	sJob.pnYRow = &vnYRow[0];
	sJob.pdEpsX = &vdEpsX[0];
	sJob.pdEpsY = &vdEpsY[0];
	sJob.dRRmin = dRmin*dRmin;
	sJob.dRRmax = dRmax*dRmax;
	sJob.dFImin = dFImin;
	sJob.dFImax = dFImax;
	sJob.dDelta = cParam.rDelta;
	sJob.pPX = pX;
	sJob.pPY = pY;
	sJob.nXY = nXY;
	sJob.rEpsConst = (_real)(0.0);
	sJob.pbIsStop = pbIsStop;
	sJob.nNextRow  = 0;
	sJob.nRowsDone = 0;
	sJob.nError  = 0;
	sJob.nCancel = 0;
	sJob.nCount  = 0;

	// first and last node in the order of the original code (x outer, y inner)
	for( i = 0; ( i < nX )&&( iFirst < 0 ); i++ )
		for( j = 0; j < nY; j++ )
			if( EpsFastNode( &sJob, i, j, &rY ) ) { iFirst = i; jFirst = j; break; };
	if( iFirst < 0 ) return FALSE;		// no nodes: the original code reports it
	for( i = nX-1; ( i >= 0 )&&( iLast < 0 ); i-- )
		for( j = nY-1; j >= 0; j-- )
			if( EpsFastNode( &sJob, i, j, &rY ) ) { iLast = i; jLast = j; break; };

	////////////////////////// from here on the block is ours
	if( nShape == EPSFAST_RECT )
	{
		__int64 qTm = TmcTimingNow();	// TIMING
		SaveFileRect( pcNextBlockList, cParam, csCurrentPath);
		if( m_pqEpsFileTicks != NULL ) *m_pqEpsFileTicks += TmcTimingNow() - qTm;	// TIMING
		if( ( m_pnEpsWritten != NULL )&&( strlen(csFileNameEps) != 0 )&&( !cError.IsError() ) ) (*m_pnEpsWritten)++;	// TIMING
		if( cError.IsError() ) return TRUE;
		csExpr = csEpsExpr;					// EPSFAST2: SaveFileRect took "[name]" off the expression
		sJob.nKind   = EpsFastExprKind( csExpr );
		sJob.pszExpr = (LPCTSTR)csExpr;
	};

	CString csBuf;
	double y_line;
	TmcIdx nCount = 0;

	csBuf.Format("Set block %d; eps = {%s}", nBlock, csEpsExpr);
	PutTrace(csBuf);

	if( sJob.nKind == EPSFAST_SERIAL )
	{
		// '=' in the expression: one thread, nodes in the original order
		DWORD dwLast = GetTickCount();
		for( i = 0; ( i < nX )&&( !cError.IsError() ); i++ )
		{
			if( !pbIsStop[0] )
			{
				csBuf.Format("User cancel calculated eps array thread in Block %d", nBlock);
				cError.PutErrorMessage(csBuf);
				return TRUE;
			};
			for( j = 0; j < nY; j++ )
			{
				if( !EpsFastNode( &sJob, i, j, &rY ) ) continue;
				y_line = rY *( CalcEps( vdEpsX[i], vdEpsY[j], cParam) );
				prYNode[ nStride*(vnXIdx[i] + vnYRow[j]) + nOffset ] += (_real)(y_line);
				nCount++;
				if( cError.IsError() ) break;
			};
			if( GetTickCount() - dwLast > 250 )
			{
				dwLast = GetTickCount();
				csBuf.Format("n:%.0f", (double)nCount);
				PutStatistics(csBuf);
			};
		};
		nNumNode = nCount;
		return TRUE;
	};

	if( sJob.nKind == EPSFAST_CONST )
	{
		sJob.rEpsConst = CalcEps( vdEpsX[iFirst], vdEpsY[jFirst], cParam );
		if( cError.IsError() ) return TRUE;
	};

	unsigned nThreads = EpsFastThreads();
	{
		char *pszSmall = getenv( "TMC_EPS_SMALL" );	// EPSFAST2: nodes of a block below which one thread is used
		double dSmall = ( pszSmall != NULL ) ? atof( pszSmall ) : (double)EPSFAST_SMALL;
		if( ((double)nX)*((double)nY) < dSmall ) nThreads = 1;
	};

	if( nThreads == 1 )
	{
		EpsFastRun( &sJob );
	}
	else
	{
		std::vector<std::thread> vThreads;
		std::vector<HANDLE> vHandles;
		unsigned n;

		for( n = 0; n < nThreads; n++ ) vThreads.push_back( std::thread( &CTmcRTHNodeDiel::EpsFastRun, this, &sJob ) );
		for( n = 0; n < nThreads; n++ ) vHandles.push_back( (HANDLE)vThreads[n].native_handle() );
		for( n = 0; n < nThreads; n++ ) SetThreadPriority( vHandles[n], THREAD_PRIORITY_BELOW_NORMAL );	// EPSFAST2: Windows stays responsive
		while( WaitForMultipleObjects( nThreads, &vHandles[0], TRUE, 250 ) == WAIT_TIMEOUT )
		{
			csBuf.Format("n:%.0f", ((double)sJob.nRowsDone)*nX );
			PutStatistics(csBuf);
		};
		for( n = 0; n < nThreads; n++ ) vThreads[n].join();
	};

	if( sJob.nCancel != 0 )
	{
		csBuf.Format("User cancel calculated eps array thread in Block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return TRUE;
	};
	if( sJob.nError != 0 )
	{
		// the original code stops at the first bad node in its order: find it
		// and let CalcEps give the original message
		for( i = 0; ( i < nX )&&( !cError.IsError() ); i++ )
			for( j = 0; j < nY; j++ )
			{
				if( !EpsFastNode( &sJob, i, j, &rY ) ) continue;
				CalcEps( vdEpsX[i], vdEpsY[j], cParam );
				if( cError.IsError() ) break;
			};
		if( !cError.IsError() )
		{
			csBuf.Format("Error in eps expression {%s} for block %d", csEpsExpr, nBlock);
			cError.PutErrorMessage(csBuf);
		};
		return TRUE;
	};

	// the interpreter keeps x, y, r, f of the last node, as after the original code
	CalcEps( vdEpsX[iLast], vdEpsY[jLast], cParam );

	nNumNode = (TmcIdx)sJob.nCount;
	return TRUE;
}

BOOL CTmcRTHNodeDiel::EpsFastEnabled( void )	// EPSFAST: TMC_EPS_FAST=0 - original code
{
	char *psz = getenv( "TMC_EPS_FAST" );
	return !( ( psz != NULL )&&( psz[0] == '0' ) );
}

CTmcLibError &CTmcRTHNodeDiel::SetRect( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CString csCurrentPath )
{
	__int64 qTm = TmcTimingNow();	// TIMING
	SaveFileRect( pcNextBlockList, cParam, csCurrentPath);
	if( m_pqEpsFileTicks != NULL ) *m_pqEpsFileTicks += TmcTimingNow() - qTm;	// TIMING
	if( ( m_pnEpsWritten != NULL )&&( strlen(csFileNameEps) != 0 )&&( !cError.IsError() ) ) (*m_pnEpsWritten)++;	// TIMING
	
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	int nXmin, nXmax, nYmin, nYmax;
	int i, j, nX, nY;
	TmcIdx ii;	// BIGBLOCK3
	double dXmin, dXmax, dYmin, dYmax, r, x, y;

	dXmin = pcNextBlockList->GetXmin() + pcNextBlockList->GetX0();
	if( dXmin < cParam.rXmin ) dXmin = cParam.rXmin;
	r = (dXmin - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmin = (int)(r);

	dXmax = pcNextBlockList->GetXmax() + pcNextBlockList->GetX0();
	if( dXmax > cParam.rXmax ) dXmax = cParam.rXmax;
	r = (dXmax - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmax = (int)(r);
	
	dYmin = pcNextBlockList->GetYmin() + pcNextBlockList->GetY0();
	if( dYmin < cParam.rYmin ) dYmin = cParam.rYmin;
	r = (dYmin - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmin = (int)(r);

	dYmax = pcNextBlockList->GetYmax() + pcNextBlockList->GetY0();
	if( dYmax > cParam.rYmax ) dYmax = cParam.rYmax;
	r = (dYmax - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmax = (int)(r);
	
	if( nXmax < nXmin )
	{
		CString csBuf;
		csBuf.Format("Xmax < Xmin for block %d, or the rectangular is behind borders of a box, i.e. it is necessary to change X_min and X_max of a rectangular or displacement on an axis X in section #LINK_LIST", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX = nXmax - nXmin + 1;

	if( nYmax < nYmin )
	{
		CString csBuf;
		csBuf.Format("Ymax < Ymin for block %d, or the rectangular is behind borders of a box, i.e. it is necessary to change Y_min and Y_max of a rectangular or displacement on an axis Y in section #LINK_LIST", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY = nYmax - nYmin + 1;

	if( ((double)nX)*((double)nY) > ((double)TMC_IDX_MAX)/64. )	// BIGBLOCK
	{
		CString csBuf;
		csBuf.Format("Block %d is too big: %.0f nodes", nBlock, ((double)nX)*((double)nY));
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nNumNode = ((TmcIdx)nX)*nY;	// BIGBLOCK
	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};

	// Вес узла — доля четырёх соседних ячеек внутри прямоугольника: 1, 0,5 на стороне, 0,25 в углу.
	for( i = 0, ii = 0, x = dXmin; i < nX; i++, x = dXmin + (i*cParam.rDelta) )
	{
//		CString csBuf1;
		for( j = 0, y = dYmin; j < nY; j++, y = dYmin + (j*cParam.rDelta) )
		{
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(1.0);
			if( i == 0 )        pcNodeDielOne[ii].rY *= (_real)(0.5);
			if( i == (nX - 1) ) pcNodeDielOne[ii].rY *= (_real)(0.5);
			if( j == 0 )        pcNodeDielOne[ii].rY *= (_real)(0.5);
			if( j == (nY - 1) ) pcNodeDielOne[ii].rY *= (_real)(0.5);
			ii++;
			if( !pbIsStop[0] )
			{
				CString csBuf;
				csBuf.Format("User cancel calculated eps array thread in Block %d", nBlock);
				cError.PutErrorMessage(csBuf);
				return cError;
			};
		};
	};

	return cError;
}

CTmcLibError &CTmcRTHNodeDiel::SetCirc( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam )
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		CString csBuf;
		csBuf.Format("Init Diel Node pointer is NULL");
		cError.PutErrorMessage(csBuf);
		return cError;
	};

	int nXmin, nXmax, nYmin, nYmax;
	int i, j, nX, nY;
	TmcIdx ii;	// BIGBLOCK3
	double dXmin, dXmax, dYmin, dYmax, r, x, y, rRxy, rFxy, x1, y1, rRxy1, rFxy1;
	double dRmin, dRmax, dFImin, dFImax, dRRmin, dRRmax;

	dRmin = pcNextBlockList->GetXmin();
	dRmax = pcNextBlockList->GetXmax();

	dFImin = pcNextBlockList->GetYmin();
	if( dFImin < 0 )
	{
		i = (int)(dFImin/(2*3.141592653589) + 1);
		dFImin += i*(2*3.141592653589);
	};
	if( dFImin >= (2*3.141592653589) )
	{
		i = (int)(dFImin/(2*3.141592653589));
		dFImin -= i*(2*3.141592653589);
	};

	dFImax = pcNextBlockList->GetYmax();
	if( dFImax <= 0 )
	{
		i = (int)(dFImax/(2*3.141592653589) + 1);
		dFImax += i*(2*3.141592653589);
	};
	// B-19: FImax up to 2*pi + 1e-12 is a full circle (full pi, the same constant as in the parser).
	// Old (the exact 2*pi became ~1e-12, an empty block or a ray):
	//   if( dFImax > (2*3.141592653589) )
	//   { i = (int)(dFImax/(2*3.141592653589)); dFImax -= i*(2*3.141592653589); };
	if( dFImax > (2*3.14159265358979323846 + 1e-12) )
	{
		i = (int)(dFImax/(2*3.14159265358979323846));
		dFImax -= i*(2*3.14159265358979323846);
	};

//		CString csBuf1;

	dXmin = - pcNextBlockList->GetXmax() + pcNextBlockList->GetX0();
	if( dXmin < cParam.rXmin ) dXmin = cParam.rXmin;
	r = (dXmin - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Rmax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmin = (int)(r);

	dXmax = pcNextBlockList->GetXmax() + pcNextBlockList->GetX0();
	if( dXmax > cParam.rXmax ) dXmax = cParam.rXmax;
	r = (dXmax - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Rmax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmax = (int)(r);
	
	dYmin = - pcNextBlockList->GetXmax() + pcNextBlockList->GetY0();
	if( dYmin < cParam.rYmin ) dYmin = cParam.rYmin;
	r = (dYmin - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Rmax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmin = (int)(r);

	dYmax = pcNextBlockList->GetXmax() + pcNextBlockList->GetY0();
	if( dYmax > cParam.rYmax ) dYmax = cParam.rYmax;
	r = (dYmax - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Rmax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmax = (int)(r);
	
	if( nXmax < nXmin )
	{
		CString csBuf;
		csBuf.Format("Xmax < Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX = nXmax - nXmin + 1;

	if( nYmax < nYmin )
	{
		CString csBuf;
		csBuf.Format("Ymax < Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY = nYmax - nYmin + 1;
							// calculated nNumNode
	dRRmin = dRmin*dRmin;
	dRRmax = dRmax*dRmax;
	for( i = 0, nNumNode = 0, x = dXmin - pcNextBlockList->GetX0(); i < nX; i++, x+=cParam.rDelta )
	{
		for( j = 0, y = dYmin - pcNextBlockList->GetY0(); j < nY; j++, y+=cParam.rDelta )
		{
			rRxy = x*x + y*y;
			rFxy = atan2__1( y, x);
			if( ( rRxy >= dRRmin )&&( rRxy <= dRRmax)&&
				( rFxy >= dFImin )&&( rFxy <= dFImax )&&
				cParam.IsnInRegion( (_real)(x+pcNextBlockList->GetX0()), (_real)(y+pcNextBlockList->GetY0()) )) nNumNode++;
		};
	};

	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};

	
							// allocation memory
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};

	dRRmin = dRmin*dRmin;
	dRRmax = dRmax*dRmax;
	for( i = 0, ii = 0, x = dXmin - pcNextBlockList->GetX0(); i < nX; i++, x+=cParam.rDelta )
	{
		for( j = 0, y = dYmin - pcNextBlockList->GetY0(); j < nY; j++, y+=cParam.rDelta )
		{
			rRxy = x*x + y*y;
			rFxy = atan2__1( y, x);
			if( ( rRxy >= dRRmin )&&( rRxy <= dRRmax)&&
				( rFxy >= dFImin )&&( rFxy <= dFImax )&&
				cParam.IsnInRegion( (_real)(x+pcNextBlockList->GetX0()), (_real)(y+pcNextBlockList->GetY0()) )) 
			{
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x+pcNextBlockList->GetX0()), (_real)(y+pcNextBlockList->GetY0()) );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
				// R-2 (author 29.10): weight by the side neighbours in the sector (old: 0.25 for each diagonal point
				// (x +- Delta, y +- Delta) - the circle lost half a step of radius; the old code is below under #if 0).
				/// Р-2: вес узла — по числу соседей по сторонам, попавших в сектор (TmcR2NodeWeight).
				{
					int kS, nSide;
					for( kS = 0, nSide = 0; kS < 4; kS++ )
					{
						x1 = x + g_dR2SideX[kS]*cParam.rDelta;
						y1 = y + g_dR2SideY[kS]*cParam.rDelta;
						rRxy1 = x1*x1 + y1*y1;
						rFxy1 = atan2__1( y1, x1);
						if( ( rRxy1 >= dRRmin )&&( rRxy1 <= dRRmax)&&
							( rFxy1 >= dFImin )&&( rFxy1 <= dFImax )) nSide++;
					};
					pcNodeDielOne[ii].rY = TmcR2NodeWeight( nSide );
				};
//		CString csBuf1;
				ii++;
			};
		};
	};
	

	return cError;
}

CTmcLibError &CTmcRTHNodeDiel::SetPoly( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam )
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	double *pX, *pY;
	int nXY;
	int nXmin, nXmax, nYmin, nYmax;
	int i, j, k, nX, nY, k1;
	TmcIdx ii;	// BIGBLOCK3
	double dXmin, dXmax, dYmin, dYmax, r, x, y, x1, y1;

	if( ( pX = pcNextBlockList->GetpX() ) == NULL )
	{				
		cError.PutErrorMessage("Init pointer pX is NULL for block ");
		return cError;
	};

	if( ( pY = pcNextBlockList->GetpY() ) == NULL )
	{				
		cError.PutErrorMessage("Init pointer pY is NULL for block ");
		return cError;
	};

	if( ( nXY = pcNextBlockList->GetnXY() ) < 3 )
	{				
		cError.PutErrorMessage("Init nXY < 3 for block ");
		return cError;
	};

	for( i = 0, dXmin = pX[0], dXmax = pX[0], dYmin = pY[0], dYmax = pY[0]; i < nXY; i++ )
	{
		if( pX[i] > dXmax ) dXmax = pX[i];
		if( pX[i] < dXmin ) dXmin = pX[i];
		if( pY[i] > dYmax ) dYmax = pY[i];
		if( pY[i] < dYmin ) dYmin = pY[i];
	}


	dXmin += pcNextBlockList->GetX0();
	if( dXmin < cParam.rXmin ) dXmin = cParam.rXmin;
	r = (dXmin - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmin = (int)(r);

	dXmax += pcNextBlockList->GetX0();
	if( dXmax > cParam.rXmax ) dXmax = cParam.rXmax;
	r = (dXmax - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmax = (int)(r);
	
	dYmin += pcNextBlockList->GetY0();
	if( dYmin < cParam.rYmin ) dYmin = cParam.rYmin;
	r = (dYmin - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmin = (int)(r);

	dYmax += pcNextBlockList->GetY0();
	if( dYmax > cParam.rYmax ) dYmax = cParam.rYmax;
	r = (dYmax - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmax = (int)(r);
	
	if( nXmax < nXmin )
	{
		CString csBuf;
		csBuf.Format("Xmax < Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX = nXmax - nXmin + 1;

	if( nYmax < nYmin )
	{
		CString csBuf;
		csBuf.Format("Ymax < Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY = nYmax - nYmin + 1;
							// calculated nNumNode
	for( i = 0, nNumNode = 0, x = dXmin - pcNextBlockList->GetX0(); i < nX; i++, x+=cParam.rDelta )
	{
		for( j = 0, y = dYmin - pcNextBlockList->GetY0(); j < nY; j++, y+=cParam.rDelta )
		{
			for( k = 2; k < nXY; k++ )
			{
				if( IsPointInTr( pX[0], pY[0], pX[k-1], pY[k-1], pX[k], pY[k], x, y )&&cParam.IsnInRegion( (_real)(x+pcNextBlockList->GetX0()), (_real)(y+pcNextBlockList->GetY0()) ) ) 
				{
					nNumNode++;
					k = nXY;
				};
			};
		};
	};

//		CString csBuf1;
//		csBuf1.Format("dXmin = %lg; dXmax = %lg; dYmin = %lg; dYmax = %lg; nNumNode = %d; nX = %d; nY = %d;", dXmin, dXmax, dYmin, dYmax, nNumNode, nX, nY );

	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	
							// allocation memory
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};


	for( i = 0, ii = 0, x = dXmin - pcNextBlockList->GetX0(); i < nX; i++, x+=cParam.rDelta )
	{
		for( j = 0, y = dYmin - pcNextBlockList->GetY0(); j < nY; j++, y+=cParam.rDelta )
		{
			for( k = 2; k < nXY; k++ )
			{
				if( IsPointInTr( pX[0], pY[0], pX[k-1], pY[k-1], pX[k], pY[k], x, y )&&cParam.IsnInRegion( (_real)(x+pcNextBlockList->GetX0()), (_real)(y+pcNextBlockList->GetY0()) ) ) 
				{
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x+pcNextBlockList->GetX0()), (_real)(y+pcNextBlockList->GetY0()) );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
					// R-2 (author 29.10): weight by the side neighbours in the polygon (old: 0.25 for each diagonal
					// point inside the polygon; the old code is below under #if 0).
					/// Р-2: вес узла — по числу соседей по сторонам внутри многоугольника (TmcR2NodeWeight).
					{
						int kS, nSide;
						for( kS = 0, nSide = 0; kS < 4; kS++ )
						{
							x1 = x + g_dR2SideX[kS]*cParam.rDelta;
							y1 = y + g_dR2SideY[kS]*cParam.rDelta;
							for( k1 = 2; k1 < nXY; k1++ )
							{
								if( IsPointInTr( pX[0], pY[0], pX[k1-1], pY[k1-1], pX[k1], pY[k1], x1, y1 ) )
								{
									nSide++;
									k1 = nXY;
								};
							};
						};
						pcNodeDielOne[ii].rY = TmcR2NodeWeight( nSide );
					};
					ii++;
					k = nXY;
				};
			};
		};
	};
	

	return cError;
}

void CTmcRTHNodeDiel::CalculatedXYCenter( CTmcRTH_IndanParam &cParam )
{
	if( cError.IsError() ) return;
	if( nNumNode == 0 ) return;
	if( pcNodeDielOne == NULL ) return;

	TmcIdx i;	// BIGBLOCK3

	{
		CString csBuf;
		csBuf.Format("CalculatedXYCenter");
		PutStatistics(csBuf);
	};
	for( i = 0, dXCenter = 0.0, dYCenter = 0.0; i < nNumNode; i++ )
	{
		dXCenter += (cParam.CalcX( pcNodeDielOne[i].nNodeGlobal ) - nX0*cParam.rDelta );
		dYCenter += (cParam.CalcY( pcNodeDielOne[i].nNodeGlobal ) - nY0*cParam.rDelta );
	};

	dXCenter /= nNumNode;
	dYCenter /= nNumNode;
	
	return;
}

CTmcLibError &CTmcRTHNodeDiel::SetInpXLeft( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut )
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	int nYmin, nYmax;
	int nY;
	TmcIdx j;	// BIGBLOCK3
	double dYmin, dYmax, r, y;

	// Апертура порта: Xmin..Xmax блока — пределы по Y относительно Y0, не ближе 2 шагов к краю.
	// Ymin..Ymax блока — окно возбуждения Tmin..Tmax.
	dYmin = pcNextBlockList->GetXmin() + pcNextBlockList->GetY0();
	if( dYmin < (cParam.rYmin + 2*cParam.rDelta) ) dYmin = cParam.rYmin + 2*cParam.rDelta;
	r = (dYmin - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmin = (int)(r);

	dYmax = pcNextBlockList->GetXmax() + pcNextBlockList->GetY0();
	if( dYmax > (cParam.rYmax - 2*cParam.rDelta) ) dYmax = cParam.rYmax - 2*cParam.rDelta;
	r = (dYmax - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmax = (int)(r);

	dWidthWaveg = (nYmax-nYmin)*cParam.rDelta;
	
	dTmin = pcNextBlockList->GetYmin();		// T_min for excite input block
	dTmax = pcNextBlockList->GetYmax();		// T_max for excite input block

	if( nYmax < nYmin )
	{
		CString csBuf;
		csBuf.Format("Ymax < Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY = nYmax - nYmin + 1;

	nNumNode = nY;
	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};

	for( j = 0, y = dYmin; j < nY; j++, y+=cParam.rDelta )
	{
		pcNodeDielOne[j].nNodeGlobal = cParam.CalcnNodeGlobal( (double)(pcNextBlockList->GetX0()), (double)y );
		pcNodeDielOne[j].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[j].rY = (_real)(0.0);
	};

	csBlock = cOut->csFileName;

	csVx = pcNextBlockList->GetcsVx();
	csVy = pcNextBlockList->GetcsVy();
	csW  = pcNextBlockList->GetcsW();

	// Шлейф среды порта по W, файл .sig, проводимость согласованной нагрузки:
	// для порта MAGNETIC — как у поглотителя (плоская волна), иначе — для основной волны H10.
	CalculateYForInput( cParam.rFreq, cParam.rDelta );
	SeachSignalFileName();

	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == 0)
	{
		dY1_InputAdmitance = SetAbsorberAdmitance( cParam.rFreq, cParam.rDelta );
	}
	else
	{
		dY1_InputAdmitance = SetInputAdmitance( cParam.rFreq, cParam.rDelta );
	};


	return cError;
}

CTmcLibError &CTmcRTHNodeDiel::SetInpXRight( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut )
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	int nYmin, nYmax;
	int nY;
	TmcIdx j;	// BIGBLOCK3
	double dYmin, dYmax, r, y;

	dYmin = pcNextBlockList->GetXmin() + pcNextBlockList->GetY0();
	if( dYmin < (cParam.rYmin + 2*cParam.rDelta) ) dYmin = cParam.rYmin + 2*cParam.rDelta;
	r = (dYmin - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmin = (int)(r);

	dYmax = pcNextBlockList->GetXmax() + pcNextBlockList->GetY0();
	if( dYmax > (cParam.rYmax - 2*cParam.rDelta) ) dYmax = cParam.rYmax - 2*cParam.rDelta;
	r = (dYmax - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmax = (int)(r);

	dWidthWaveg = (nYmax-nYmin)*cParam.rDelta;
	
	dTmin = pcNextBlockList->GetYmin();		// T_min for excite input block
	dTmax = pcNextBlockList->GetYmax();		// T_max for excite input block

	if( nYmax < nYmin )
	{
		CString csBuf;
		csBuf.Format("Ymax < Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY = nYmax - nYmin + 1;

	nNumNode = nY;
	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};

	for( j = 0, y = dYmin; j < nY; j++, y+=cParam.rDelta )
	{
		pcNodeDielOne[j].nNodeGlobal = cParam.CalcnNodeGlobal( (double)(pcNextBlockList->GetX0()), (double)y );
		pcNodeDielOne[j].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[j].rY = (_real)(0.0);
	};

	csBlock = cOut->csFileName;

	csVx = pcNextBlockList->GetcsVx();
	csVy = pcNextBlockList->GetcsVy();
	csW  = pcNextBlockList->GetcsW();

	CalculateYForInput( cParam.rFreq, cParam.rDelta );
	SeachSignalFileName();


	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == 0)
	{
		dY1_InputAdmitance = SetAbsorberAdmitance( cParam.rFreq, cParam.rDelta );
	}
	else
	{
		dY1_InputAdmitance = SetInputAdmitance( cParam.rFreq, cParam.rDelta );
	};

	return cError;
}

CTmcLibError &CTmcRTHNodeDiel::SetInpYTop( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut )
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	int nXmin, nXmax;
	int nX;
	TmcIdx j;	// BIGBLOCK3
	double dXmin, dXmax, r, x;

	dXmin = pcNextBlockList->GetXmin() + pcNextBlockList->GetX0();
	if( dXmin < (cParam.rXmin + 2*cParam.rDelta) ) dXmin = cParam.rXmin + 2*cParam.rDelta;
	r = (dXmin - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmin = (int)(r);

	dXmax = pcNextBlockList->GetXmax() + pcNextBlockList->GetX0();
	if( dXmax > (cParam.rXmax - 2*cParam.rDelta) ) dXmax = cParam.rXmax - 2*cParam.rDelta;
	r = (dXmax - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmax = (int)(r);
	dWidthWaveg = (nXmax-nXmin)*cParam.rDelta;
	
	dTmin = pcNextBlockList->GetYmin();		// T_min for excite input block
	dTmax = pcNextBlockList->GetYmax();		// T_max for excite input block

	if( nXmax < nXmin )
	{
		CString csBuf;
		csBuf.Format("Xmax < Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX = nXmax - nXmin + 1;

	nNumNode = nX;
	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};

	for( j = 0, x = dXmin; j < nX; j++, x+=cParam.rDelta )
	{
		pcNodeDielOne[j].nNodeGlobal = cParam.CalcnNodeGlobal( (double)x, (double)(pcNextBlockList->GetY0()) );
		pcNodeDielOne[j].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[j].rY = (_real)(0.0);
	};

	csBlock = cOut->csFileName;

	csVx = pcNextBlockList->GetcsVx();
	csVy = pcNextBlockList->GetcsVy();
	csW  = pcNextBlockList->GetcsW();

	CalculateYForInput( cParam.rFreq, cParam.rDelta );
	SeachSignalFileName();

	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == 0)
	{
		dY1_InputAdmitance = SetAbsorberAdmitance( cParam.rFreq, cParam.rDelta );
	}
	else
	{
		dY1_InputAdmitance = SetInputAdmitance( cParam.rFreq, cParam.rDelta );
	};

	return cError;
}

CTmcLibError &CTmcRTHNodeDiel::SetInpYBot( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut )
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	int nXmin, nXmax;
	int nX;
	TmcIdx j;	// BIGBLOCK3
	double dXmin, dXmax, r, x;

	dXmin = pcNextBlockList->GetXmin() + pcNextBlockList->GetX0();
	if( dXmin < (cParam.rXmin + 2*cParam.rDelta) ) dXmin = cParam.rXmin + 2*cParam.rDelta;
	r = (dXmin - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmin = (int)(r);

	dXmax = pcNextBlockList->GetXmax() + pcNextBlockList->GetX0();
	if( dXmax > (cParam.rXmax - 2*cParam.rDelta) ) dXmax = cParam.rXmax - 2*cParam.rDelta;
	r = (dXmax - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmax = (int)(r);
	dWidthWaveg = (nXmax-nXmin)*cParam.rDelta;
	
	dTmin = pcNextBlockList->GetYmin();		// T_min for excite input block
	dTmax = pcNextBlockList->GetYmax();		// T_max for excite input block

	if( nXmax < nXmin )
	{
		CString csBuf;
		csBuf.Format("Xmax < Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX = nXmax - nXmin + 1;

	nNumNode = nX;
	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};

	for( j = 0, x = dXmin; j < nX; j++, x+=cParam.rDelta )
	{
		pcNodeDielOne[j].nNodeGlobal = cParam.CalcnNodeGlobal( (double)x, (double)(pcNextBlockList->GetY0()) );
		pcNodeDielOne[j].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[j].rY = (_real)(0.0);
	};

	csBlock = cOut->csFileName;

	csVx = pcNextBlockList->GetcsVx();
	csVy = pcNextBlockList->GetcsVy();
	csW  = pcNextBlockList->GetcsW();

	CalculateYForInput( cParam.rFreq, cParam.rDelta );
	SeachSignalFileName();

	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == 0)
	{
		dY1_InputAdmitance = SetAbsorberAdmitance( cParam.rFreq, cParam.rDelta );
	}
	else
	{
		dY1_InputAdmitance = SetInputAdmitance( cParam.rFreq, cParam.rDelta );
	};

	return cError;
}

void CTmcRTHNodeDiel::SetYLineForDiel( CTmcRTH_IndanParam &cParam, _real *prYNode )
{
//		CString csBuf1;
	if( cError.IsError() ) return;
	if( nNumNode == 0 ) return;
	if( pcNodeDielOne == NULL ) return;
	if( (nType != CTMCRTH_BLCKNTYPE_EPS) )	return;

	CString csBuf;
	char szBuf[CTMCRTH_INDANSBUF];
	char szBuf1[CTMCRTH_INDANSBUF];
	TmcIdx i;	// BIGBLOCK3
	TmcIdx nNode, ii;	// INT64
	double x, y, y_line, dDelta;
	BOOL bIsFile;
	
	if( strncmp( csBlock, CTMCRTH_INDANBLCK_FILE, strlen(CTMCRTH_INDANBLCK_FILE) ) == 0 )
	{
		bIsFile = TRUE;
	}
	else
	{
		bIsFile = FALSE;
	};
	
	if( (szBuf == NULL)||(szBuf1 == NULL) )
	{
		csBuf.Format("Not enought memory for SetYLineForDiel");
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csEpsExpr );
	BOOL bEpsConst = ( !bIsFile )&&EpsFastEnabled()&&( EpsFastExprKind( csEpsExpr ) == EPSFAST_CONST );	// EPSFAST
	_real rEpsConst = (_real)(0.0);	// EPSFAST
	
	csBuf.Format("Set block %d; points:%.0f; eps = {%s}", nBlock, (double)nNumNode, csEpsExpr);
	PutTrace(csBuf);

	// Вклады блоков складываются: узел на стыке двух блоков получает сумму долей.
	for( i = 0, dDelta = cParam.rDelta/2., ii = 0; i < nNumNode; i++, ii++ )
	{
		if( cError.IsError() ) return;
		if( ii == 10000 )
		{
			csBuf.Format("n:%.0f", (double)i);
			PutStatistics(csBuf);
			ii = 0;
		};
		if( !pbIsStop[0] )
		{
			csBuf.Format("User cancel calculated eps array thread in Block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return;
		};
		
		nNode = pcNodeDielOne[i].nNodeGlobal;
		if( bIsFile )
		{
			y_line = pcNodeDielOne[i].rY;
		}
		else
		{
			x = cParam.CalcX( nNode ) - dX0;
			y = cParam.CalcY( nNode ) - dY0;
//		CString csBuf1;
			if( ( !bEpsConst )||( i == 0 ) ) rEpsConst = CalcEps( x, y, cParam);	// EPSFAST: constant - once
			y_line = pcNodeDielOne[i].rY *( rEpsConst );
		};
		pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_EPS;
		prYNode[ 2*nNode ] += (_real)(y_line);
//		CString csBuf1;

	};
	if( bEpsConst && ( nNumNode > 1 ) )	// EPSFAST: the interpreter keeps the last node, as before
		CalcEps( cParam.CalcX( pcNodeDielOne[nNumNode-1].nNodeGlobal ) - dX0, cParam.CalcY( pcNodeDielOne[nNumNode-1].nNodeGlobal ) - dY0, cParam );

	return;
}


void CTmcRTHNodeDiel::SetnType( void )
{
	if( cError.IsError() ) return;
	if( nNumNode == 0 ) return;
	if( pcNodeDielOne == NULL ) return;
	
	TmcIdx i;	// BIGBLOCK3

	{
		CString csBuf;
		csBuf.Format("SetnType");
		PutStatistics(csBuf);
	};
	for( i = 0; i < nNumNode; i++ )
	{
		if( (nType != CTMCRTH_BLCKNTYPE_ABSORBER)&&(nType != CTMCRTH_BLCKNTYPE_MAGNETIC) ) pcNodeDielOne[i].nType = nType;
		pcNodeDielOne[i].rU  = 0.0;
		pcNodeDielOne[i].rUo = 0.0;
		pcNodeDielOne[i].rU_Cur = 0.0;
		pcNodeDielOne[i].rU_Cur1 = 0.0;
	};

	
	return;
}

void CTmcRTHNodeDiel::SetYForInput( CTmcRTH_IndanParam &cParam )
{
	if( cError.IsError() ) return;
	if( nNumNode == 0 ) return;
	if( pcNodeDielOne == NULL ) return;
	if( (nType != CTMCRTH_BLCKNTYPE_INPXLEFT)&&
		(nType != CTMCRTH_BLCKNTYPE_INPXRIGHT)&&
		(nType != CTMCRTH_BLCKNTYPE_INPYTOP)&&
		(nType != CTMCRTH_BLCKNTYPE_INPYBOT) )	return;

	CString csBuf;
	char szBuf[CTMCRTH_INDANSBUF];
	char szBuf1[CTMCRTH_INDANSBUF];
	TmcIdx i;	// BIGBLOCK3

	if( (szBuf == NULL)||(szBuf1 == NULL) )
	{
		csBuf.Format("Not enought memory for SetYLineForDiel");
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csEpsExpr );

	csBuf.Format("Inp:%d", nBlock);
	PutStatistics(csBuf);

	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == 0)
	{
////			pcNodeDielOne[i].rY = (_real)(1.0);
		// Порт MAGNETIC (стенки — магнитные): постоянный профиль.
		for( i = 0, dUnormir = 0.0; i < nNumNode; i++ )
		{
			dUnormir += 1.*1.;
		};
		
		dUnormir = sqrt(1./(dY1_InputAdmitance*dUnormir*cParam.dGetDelta()*cParam.dGetDelta()));
		
		for( i = 0; i < nNumNode; i++ )
		{
			pcNodeDielOne[i].rY = (_real)(dUnormir*1.);
		};
	}
	else
	{
		// Стенки порта — металл: профиль sin(pi*i/(N-1)), крайние узлы на стенках равны 0.
		for( i = 0, dUnormir = 0.0; i < nNumNode; i++ )
		{
			dUnormir += sin(i*PI___/(nNumNode-1))*sin(i*PI___/(nNumNode-1));
		};
		
		dUnormir = sqrt(1./(dY1_InputAdmitance*dUnormir*cParam.dGetDelta()*cParam.dGetDelta()));
		
		for( i = 0; i < nNumNode; i++ )
		{
			pcNodeDielOne[i].rY = (_real)(dUnormir*sin(i*PI___/(nNumNode-1)));
		};
	};


	return;
}

_real CTmcRTHNodeDiel::CalcEps( double x, double y, CTmcRTH_IndanParam &cParam )
{
	if( cError.IsError() ) return (_real)(0.0);

	CString csBuf;
	char szBuf[CTMCRTH_INDANSBUF];
	char szBuf1[CTMCRTH_INDANSBUF];
	_real rr = (_real)0.0;
	double r1, r, f;
	double dDelta = cParam.rDelta/2.;

	if( (szBuf == NULL)||(szBuf1 == NULL) )
	{
		cError.PutErrorMessage("Not enought memory for CalcEps");
		return (_real)(0.0);
	};

	strcpy( szBuf, csEpsExpr );

	x /=cParam.rLongUnit;
	y /=cParam.rLongUnit;
	r = sqrt( x*x + y*y );
	f = atan2__1( y, x)/cParam.rAngleUnit;
	// B-11, old (6 digits):
	expr_set_per( (char *)"x", 1, (_real)x );	// B-11: the full precision, without a string
	expr_set_per( (char *)"y", 1, (_real)y );
	expr_set_per( (char *)"r", 1, (_real)r );
	expr_set_per( (char *)"f", 1, (_real)f );
	szBuf1[0] = '\0';
	if( i1nte_atof_1( szBuf, &r1 ) != 0 )
	{
		csBuf.Format("In{%s}%s; r=%lg;f=%lg; x=%lg; y=%lg",
			szBuf, expr_get_error(), r, f, x, y );
		cError.PutErrorMessage(csBuf);
		return (_real)(0.0);
	};
	rr = (_real)r1;
//		CString csBuf1;

	return rr;
}





void CTmcRTHNodeDiel::ExciteInputs( CTmcLibError &cError1, double dWT, double dT, _ELEM_VAL_RTH *pr1, int nX, CTmcRTH_IndanParam &cParam, double dtCurrent )
{
	if( cError1.IsError() ) return;
	if( cError.IsError() )
	{
		cError1 = cError;
		return;
	};
	if( nNumNode == 0 ) return;
	if( pcNodeDielOne == NULL ) return;

	if( (nType != CTMCRTH_BLCKNTYPE_INPXLEFT )&&
		(nType != CTMCRTH_BLCKNTYPE_INPXRIGHT)&&
		(nType != CTMCRTH_BLCKNTYPE_INPYTOP  )&&
		(nType != CTMCRTH_BLCKNTYPE_INPYBOT  )) return;

	TmcIdx i, n1;	// BIGBLOCK
	int i11 = 0;
	
	CString csBuf;
	double dUp, dUo, a, a1;
	_ELEM_VAL_RTH *u2, *u3, *u4, y1, dBuf;

	// Падающая волна: из файла .sig или Amp(t,f)*sin(omega*t + Faza); вне окна Tmin..Tmax — 0.
	a = bSigFile ? dSignalFromFile( dtCurrent, cParam ) : dAmplitude(cParam, dtCurrent)*sin( dWT + dFaza(cParam, dtCurrent) );
	y1 = (_ELEM_VAL_RTH)dY1_InputAdmitance;
//	y1 = (_ELEM_VAL_RTH)( 1 + 
//		( (4*cos(dY1_InputAdmitance)-8*cos(dWidthWaveg))*cos(dWT) + sin(dY1_InputAdmitance)*sin(dWT) )/
//		( (-cos(dY1_InputAdmitance)+cos(dWidthWaveg))*cos(dWT) + sin(dY1_InputAdmitance)*sin(dWT) )
//		);
	if( (dT >= dTmin)&&(dT <= dTmax) ) a1 = a;
	else a1 = 0.0;
	
	dUp = 0.0;
	dUo = 0.0;
	i = 0;
	dBuf = (_ELEM_VAL_RTH)0.0;
	
	if( nNumNode < 3 )	// A-36: ne molchat
	{
		CString csFew;
		csFew.Format("Port block %d has %d nodes, need at least 3", nBlock, (int)nNumNode);
		cError1.PutErrorMessage( csFew );
		return;
	};

	// Восстановить напряжения узлов порта: общий проход рассеяния мог их переписать.
	for( i = 0, n1 = nNumNode; i < n1; i++)
	{
		pr1[ pcNodeDielOne[i].nNodeGlobal*2 ] = pcNodeDielOne[i].rU_Cur;
		pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1 ] = pcNodeDielOne[i].rU_Cur1;
	};

	// Порт MAGNETIC: считаются все узлы, крайние — с половинными линиями (магнитные стенки).
	// u2, u4 — соседи вдоль апертуры, u3 — сосед внутрь области; узел позади порта обнуляется,
	// если порт стоит не на краю области.
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == 0)
	{
		i = 0;
		switch( nType )
		{
			case CTMCRTH_BLCKNTYPE_INPXLEFT: 
				u2 = g_rZeroNeighbour;	// D-4 (old: u2 = pr1; - the node (0,0))
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2+1;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2+1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-4-1+i11] = 0.0;
					u3[-4+i11] = 0.0;
				}
				break;
			case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
				u2 = g_rZeroNeighbour;	// D-4 (old: u2 = pr1; - the node (0,0))
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2+1;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2+1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[4-1-i11] = 0.0;
					u3[4-i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYTOP:
				u2 = g_rZeroNeighbour;	// D-4 (old: u2 = pr1; - the node (0,0))
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2+1;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2+1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[4*nX-1] = 0.0;
					u3[4*nX] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYBOT:
				u2 = g_rZeroNeighbour;	// D-4 (old: u2 = pr1; - the node (0,0))
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2+1;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2+1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-4*nX-1] = 0.0;
					u3[-4*nX] = 0.0;
				};
				break;
		};

		// Крайний узел: половина ячейки — линии, шлейф и нагрузка вдвое меньше.
		pr1[ pcNodeDielOne[i].nNodeGlobal*2 ] = (_ELEM_VAL_RTH)(2./( 1. + y1/2. + fabs(dYInput)/4. )*
			( 0.5*u2[0] + 0.5*u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1 ]*dYInput/4. + 0.5 * y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*2 ] + 
			pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*2 ]);
		pcNodeDielOne[i].rU =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		
		// Внутренние узлы апертуры: линия внутрь области полная, вдоль апертуры — половинные
		// (узел на границе), нагрузка y1 и источник y1*(U + rY*a - rU).
		for( i = 1, n1 = nNumNode - 1; i < n1; i++)
		{
			switch( nType )
			{
				case CTMCRTH_BLCKNTYPE_INPXLEFT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2+1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2+1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2+1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-4-1+i11] = 0.0;
		          u3[-4+i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2+1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2+1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2+1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[4-1-i11] = 0.0;
					u3[4-i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYTOP:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2+1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2+1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2+1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[4*nX-1] = 0.0;
					u3[4*nX] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYBOT:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2+1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2+1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2+1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-4*nX-1] = 0.0;
					u3[-4*nX] = 0.0;
				};
					break;
			};
			pr1[ pcNodeDielOne[i].nNodeGlobal*2 ] = (_ELEM_VAL_RTH)(2./( 2. + y1  + fabs(dYInput)/2. )*
				( 0.5*u2[0] + u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1 ]*dYInput/2. + y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*2 ] + 
				pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*2 ]);
			pcNodeDielOne[i].rU =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		};
		switch( nType )
		{
			case CTMCRTH_BLCKNTYPE_INPXLEFT: 
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2+1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2+1;
				u4 = g_rZeroNeighbour;	// D-4 (old: u4 = pr1; - the node (0,0))
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-4-1+i11] = 0.0;
		          u3[-4+i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2+1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2+1;
				u4 = g_rZeroNeighbour;	// D-4 (old: u4 = pr1; - the node (0,0))
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[4-1-i11] = 0.0;
					u3[4-i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYTOP:
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2+1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2+1;
				u4 = g_rZeroNeighbour;	// D-4 (old: u4 = pr1; - the node (0,0))
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[4*nX-1] = 0.0;
					u3[4*nX] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYBOT:
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2+1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2+1;
				u4 = g_rZeroNeighbour;	// D-4 (old: u4 = pr1; - the node (0,0))
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-4*nX-1] = 0.0;
					u3[-4*nX] = 0.0;
				};
				break;
		};
		pr1[ pcNodeDielOne[i].nNodeGlobal*2 ] = (_ELEM_VAL_RTH)(2./( 1. + y1/2. + fabs(dYInput)/4. )*
			( 0.5*u2[0] + 0.5*u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1 ]*dYInput/4. + 0.5 * y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*2 ] + 
			pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*2 ]);
		pcNodeDielOne[i].rU =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
	}
	else
	{
		// Стенки порта — металл: крайние узлы (на стенках) не пересчитываются.
		for( i = 1, n1 = nNumNode - 1; i < n1; i++)
		{
			switch( nType )
			{
				case CTMCRTH_BLCKNTYPE_INPXLEFT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2+1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2+1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2+1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-4-1+i11] = 0.0;
		          u3[-4+i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2+1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2+1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2+1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[4-1-i11] = 0.0;
	                u3[4-i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYTOP:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2+1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2+1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2+1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[4*nX-1] = 0.0;
					u3[4*nX] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYBOT:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2+1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2+1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2+1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-4*nX-1] = 0.0;
					u3[-4*nX] = 0.0;
				};
					break;
			};
			pr1[ pcNodeDielOne[i].nNodeGlobal*2 ] = (_ELEM_VAL_RTH)(2./( 2. + y1 + fabs(dYInput)/2. )*
				( 0.5*u2[0] + u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1 ]*dYInput/2. + y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*2 ] + 
				pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*2 ]);
			pcNodeDielOne[i].rU =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		};
	};

	// Запомнить новые напряжения узлов порта.
	for( i = 0, n1 = nNumNode; i < n1; i++)
	{
		pcNodeDielOne[i].rU_Cur = pr1[ pcNodeDielOne[i].nNodeGlobal*2 ];
		pcNodeDielOne[i].rU_Cur1 = pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1 ];
	};

	// Отражённая волна — проекция (U - падающая) на профиль порта: Uo = sum(rY*(U - rU))/sum(rY^2).
	for( i = 0, dUp = 0.0, dUo = 0.0; i < nNumNode; i++)
	{
		dUp += pcNodeDielOne[i].rY*pcNodeDielOne[i].rY;
		dUo += pcNodeDielOne[i].rY*( pr1[ pcNodeDielOne[i].nNodeGlobal*2 ] - pcNodeDielOne[i].rU );
	};

	dUo /= dUp/_ELEM_VAL_RTH_MULT;
	dUp = a1;
	OutputUpUo( dUp, dUo , cError1);


	return;
}

void CTmcRTHNodeDiel::ExciteInputs1( CTmcLibError &cError1, double dWT, double dT, _ELEM_VAL_RTH *pr1, int nX, CTmcRTH_IndanParam &cParam, double dtCurrent )
{
	if( cError1.IsError() ) return;
	if( cError.IsError() )
	{
		cError1 = cError;
		return;
	};
	if( nNumNode == 0 ) return;
	if( pcNodeDielOne == NULL ) return;

	if( (nType != CTMCRTH_BLCKNTYPE_INPXLEFT )&&
		(nType != CTMCRTH_BLCKNTYPE_INPXRIGHT)&&
		(nType != CTMCRTH_BLCKNTYPE_INPYTOP  )&&
		(nType != CTMCRTH_BLCKNTYPE_INPYBOT  )) return;

	TmcIdx i, n1;	// BIGBLOCK
	int i11 = 0;
	
	CString csBuf;
	double dUp, dUo, a, a1;
	_ELEM_VAL_RTH *u2, *u3, *u4, y1, dBuf;

	a = bSigFile ? dSignalFromFile( dtCurrent, cParam ) : dAmplitude(cParam, dtCurrent)*sin( dWT + dFaza(cParam, dtCurrent) );
	y1 = (_ELEM_VAL_RTH)dY1_InputAdmitance;
//	y1 = sqrt(2.)*sqrt(1-(dWT/dT)/(2*dWidthWaveg*C0___)*(dWT/dT)/(2*dWidthWaveg*C0___));
//	y1 = (_ELEM_VAL_RTH)( 1 + 
//		( (4*cos(dY1_InputAdmitance)-8*cos(dWidthWaveg))*cos(dWT) + sin(dY1_InputAdmitance)*sin(dWT) )/
//		( (-cos(dY1_InputAdmitance)+cos(dWidthWaveg))*cos(dWT) + sin(dY1_InputAdmitance)*sin(dWT) )
//		);

	if( (dT >= dTmin)&&(dT <= dTmax) ) a1 = a;
	else a1 = 0.0;
	
	dUp = 0.0;
	dUo = 0.0;
	i = 0;
	dBuf = (_ELEM_VAL_RTH)0.0;
	
	if( nNumNode < 3 )	// A-36: ne molchat
	{
		CString csFew;
		csFew.Format("Port block %d has %d nodes, need at least 3", nBlock, (int)nNumNode);
		cError1.PutErrorMessage( csFew );
		return;
	};

	for( i = 0, n1 = nNumNode; i < n1; i++)
	{
		pr1[ pcNodeDielOne[i].nNodeGlobal*2 ] = pcNodeDielOne[i].rU_Cur;
		pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1 ] = pcNodeDielOne[i].rU_Cur1;
	};

	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == 0)
	{
		i = 0;
		switch( nType )
		{
			case CTMCRTH_BLCKNTYPE_INPXLEFT: 
				u2 = g_rZeroNeighbour;	// D-4 (old: u2 = pr1; - the node (0,0))
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-4+i11] = 0.0;
		          u3[-4+1+i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
				u2 = g_rZeroNeighbour;	// D-4 (old: u2 = pr1; - the node (0,0))
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[4-i11] = 0.0;
		          u3[4+1-i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYTOP:
				u2 = g_rZeroNeighbour;	// D-4 (old: u2 = pr1; - the node (0,0))
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[4*nX] = 0.0;
					u3[4*nX+1] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYBOT:
				u2 = g_rZeroNeighbour;	// D-4 (old: u2 = pr1; - the node (0,0))
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-4*nX] = 0.0;
					u3[-4*nX+1] = 0.0;
				};
				break;
		};
		pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1 ] = (_ELEM_VAL_RTH)(2./( 1. + y1/2. + fabs(dYInput)/4. )*
			( 0.5*u2[0] + 0.5*u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*2 ]*dYInput/4. + 0.5 * y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1] + 
			pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rUo ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1]);
		pcNodeDielOne[i].rUo =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		for( i = 1, n1 = nNumNode - 1; i < n1; i++)
		{
			switch( nType )
			{
				case CTMCRTH_BLCKNTYPE_INPXLEFT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-4+i11] = 0.0;
		          u3[-4+1+i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[4-i11] = 0.0;
		          u3[4+1-i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYTOP:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[4*nX] = 0.0;
					u3[4*nX+1] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYBOT:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-4*nX] = 0.0;
					u3[-4*nX+1] = 0.0;
				};
					break;
			};
			pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1 ] = (_ELEM_VAL_RTH)(2./( 2. + y1 + fabs(dYInput)/2. )*
				( 0.5*u2[0] + u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*2 ]*dYInput/2. + y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1] + 
				pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rUo ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1]);
			pcNodeDielOne[i].rUo =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		};
		switch( nType )
		{
			case CTMCRTH_BLCKNTYPE_INPXLEFT: 
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2;
				u4 = g_rZeroNeighbour;	// D-4 (old: u4 = pr1; - the node (0,0))
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-4+i11] = 0.0;
		          u3[-4+1+i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2;
				u4 = g_rZeroNeighbour;	// D-4 (old: u4 = pr1; - the node (0,0))
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[4-i11] = 0.0;
		          u3[4+1-i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYTOP:
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2;
				u4 = g_rZeroNeighbour;	// D-4 (old: u4 = pr1; - the node (0,0))
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[4*nX] = 0.0;
					u3[4*nX+1] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYBOT:
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2;
				u4 = g_rZeroNeighbour;	// D-4 (old: u4 = pr1; - the node (0,0))
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-4*nX] = 0.0;
					u3[-4*nX+1] = 0.0;
				};
				break;
		};
		pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1 ] = (_ELEM_VAL_RTH)(2./( 1. + y1/2. + fabs(dYInput)/4. )*
			( 0.5*u2[0] + 0.5*u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*2 ]*dYInput/4. + 0.5 * y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1] + 
			pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rUo ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1]);
		pcNodeDielOne[i].rUo =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
	}
	else
	{
		for( i = 1, n1 = nNumNode - 1; i < n1; i++)
		{
			switch( nType )
			{
				case CTMCRTH_BLCKNTYPE_INPXLEFT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-4+i11] = 0.0;
		          u3[-4+1+i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[4-i11] = 0.0;
		          u3[4+1-i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYTOP:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[4*nX] = 0.0;
					u3[4*nX+1] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYBOT:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*2;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-4*nX] = 0.0;
					u3[-4*nX+1] = 0.0;
				};
					break;
			};
			pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1 ] = (_ELEM_VAL_RTH)(2./( 2. + y1 + fabs(dYInput)/2. )*
				( 0.5*u2[0] + u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*2 ]*dYInput/2.  + y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1] + 
				pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rUo ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1]);
			pcNodeDielOne[i].rUo =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		};
	};

	for( i = 0, n1 = nNumNode; i < n1; i++)
	{
		pcNodeDielOne[i].rU_Cur = pr1[ pcNodeDielOne[i].nNodeGlobal*2 ];
		pcNodeDielOne[i].rU_Cur1 = pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1 ];
	};

	for( i = 0, dUp = 0.0, dUo = 0.0; i < nNumNode; i++)
	{
		dUp += pcNodeDielOne[i].rY*pcNodeDielOne[i].rY;
		dUo += pcNodeDielOne[i].rY*( pr1[ pcNodeDielOne[i].nNodeGlobal*2 + 1] - pcNodeDielOne[i].rUo );
	};

	dUo /= dUp/_ELEM_VAL_RTH_MULT;
	dUp = a1;
	OutputUpUo( dUp, dUo , cError1);


	return;
}

void CTmcRTHNodeDiel::OutputUpUo( double dUp, double dUo, CTmcLibError &cError1)
{
	FILE *fp;
	char *psForm;
	__int64 qTm = TmcTimingNow();	// TIMING

	BOOL bHeld = TRUE;
	if( ( fp = TmcOutTGet( csBlock ) ) == NULL )	// T-5: the .t of the step is kept open by the kernel
	{
		int nErr = 0;
		bHeld = FALSE;
		if( ( fp = TmcFopenWait( csBlock, "a", FALSE, &nErr ) ) == NULL )	// T-9: 20 s by the clock
		{
			cError1.PutErrorMessage( TmcOpenFailText( csBlock, nErr ) );
			return;
		};
	};

	psForm = GetnBFormat();
	fprintf( fp, psForm, nBlock);
	psForm = GetdInpFormat();
	fprintf( fp, psForm, dUp);
	psForm = GetdOutFormat();

	fprintf( fp, psForm, dUo);



	if( !bHeld ) fclose( fp );
	if( m_pqSignalFileTicks != NULL ) *m_pqSignalFileTicks += TmcTimingNow() - qTm;	// TIMING
	return;

}

double CTmcRTHNodeDiel::SetInputAdmitance( double dFreq, double dDelta )
{
	double dY1, dA_111;
//	CString csBuf1;

// for  delta*W->0
//	dY1 = sqrt( 2.*(1. - ( C0___/(dFreq*2.*dWidthWaveg) )*( C0___/(dFreq*2.*dWidthWaveg) ) ) );

	dA_111 = sqrt(2.)*PI___*dDelta*dFreq/C0___;
	double dArgCutoff = (2+fabs(dYInput)/2.)*cos(dA_111) - dYInput/2. - cos(PI___/(dWidthWaveg/dDelta));	// A-7
	if( !( dArgCutoff >= -1. && dArgCutoff <= 1. ) )
	{
		CString csCutoff;
		csCutoff.Format("Frequency %lg is below cutoff of port (width %lg)", dFreq, dWidthWaveg);
		cError.PutErrorMessage( csCutoff );
	};
	dY1 = sin(acos( dArgCutoff ))/sin(dA_111);



	return dY1;
}

double CTmcRTHNodeDiel::SetAbsorberAdmitance( double dFreq, double dDelta )
{
	double dY1, dA_111;
//	CString csBuf1;

	dA_111 = sqrt(2.)*PI___*dDelta*dFreq/C0___;
	dY1 = sin(acos( (2+fabs(dYInput)/2.)*cos(dA_111) - 1. - dYInput/2. ))/sin(dA_111);


	return dY1;
}


// R-1 (author 29.10): the absorber load by the medium of its nodes, (7.54) (old: always the vacuum, dYInput = 0)
CTmcLibError &CTmcRTHNodeDiel::SetAbsorberLoadByMedium( CTmcRTH_IndanParam &cParam, _real *prYNode, int nStride, int nSlot, TmcIdx nNodes )
{
	if( cError.IsError() ) return cError;
	if( nType != CTMCRTH_BLCKNTYPE_ABSORBER ) return cError;
	if( ( pcNodeDielOne == NULL )||( prYNode == NULL ) ) return cError;

	TmcIdx i, n;
	double y, dArg, dA_111, dKeep = dYInput;
	dA_111 = sqrt(2.)*PI___*cParam.rDelta*cParam.rFreq/C0___;
	nAbsorberNoWave = 0;
	for( i = 0; i < nNumNode; i++ )
	{
		n = pcNodeDielOne[i].nNodeGlobal;
		if( ( n < 0 )||( n >= nNodes ) ) continue;	// B-16: dropped nodes
		y = prYNode[ n*nStride + nSlot ];
		if( y == 0.0 ) continue;				// vacuum: the load is the same as before
		dArg = (2+fabs(y)/2.)*cos(dA_111) - 1. - y/2.;
		if( !( dArg >= -1. && dArg <= 1. ) )
		{
			nAbsorberNoWave++;	// no propagating wave in this medium - the old (vacuum) load, a warning
			continue;
		};
		dYInput = y;
		pcNodeDielOne[i].rY = (_real)SetAbsorberAdmitance( cParam.rFreq, cParam.rDelta );
	};
	dYInput = dKeep;
	return cError;
}

CTmcLibError & CTmcRTHNodeDiel::SetRectMetal(CTmcRTH_BlockList *pcNextBlockList, CTmcRTH_IndanParam &cParam)
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	int nXmin, nXmax, nYmin, nYmax;
	int i, nX, nY;
	TmcIdx ii;	// BIGBLOCK3
	double dXmin, dXmax, dYmin, dYmax, r, x, y;

	dXmin = pcNextBlockList->GetXmin() + pcNextBlockList->GetX0();
	if( dXmin < cParam.rXmin ) dXmin = cParam.rXmin;
	r = (dXmin - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmin = (int)(r);

	dXmax = pcNextBlockList->GetXmax() + pcNextBlockList->GetX0();
	if( dXmax > cParam.rXmax ) dXmax = cParam.rXmax;
	r = (dXmax - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmax = (int)(r);
	
	dYmin = pcNextBlockList->GetYmin() + pcNextBlockList->GetY0();
	if( dYmin < cParam.rYmin ) dYmin = cParam.rYmin;
	r = (dYmin - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmin = (int)(r);

	dYmax = pcNextBlockList->GetYmax() + pcNextBlockList->GetY0();
	if( dYmax > cParam.rYmax ) dYmax = cParam.rYmax;
	r = (dYmax - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmax = (int)(r);
	
	if( nXmax < nXmin )
	{
		CString csBuf;
		csBuf.Format("Xmax < Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX = nXmax - nXmin + 1;

	if( nYmax < nYmin )
	{
		CString csBuf;
		csBuf.Format("Ymax < Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY = nYmax - nYmin + 1;

	nNumNode = 2*nX + 2*(nY-1);
	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};

	for( i = 0, ii = 0, x = dXmin, y = dYmin; i < nX; i++, x+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(1.0);
		ii++;
	};

	for( i = 0, x = dXmin, y = dYmax; i < nX; i++, x+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(1.0);
		ii++;
	};

	for( i = 0, x = dXmin, y = dYmin; i < (nY - 1); i++, y+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(1.0);
		ii++;
	};

	for( i = 0, x = dXmax, y = dYmin; i < (nY - 1); i++, y+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(1.0);
		ii++;
	};

	return cError;
}

void CTmcRTHNodeDiel::RunBlockNode( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, TmcIdx nArraySize )
{
	if( nType == CTMCRTH_BLCKNTYPE_EPS ) return;
	if( cError.IsError() ) return;
	if( pcNodeDielOne == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return;
	};

	TmcIdx i;	// BIGBLOCK3
	double u1, u2, u3, u4, ut1, ut2, y5, y;
	_ELEM_VAL_RTH *prU;
	
	switch( nType )
	{
		case  CTMCRTH_BLCKNTYPE_METAL    :
			// Металл: напряжение узлов блока равно 0 (условие КЗ).
			for( i = 0; i < nNumNode; i++)
				prUNode1[ 2*pcNodeDielOne[i].nNodeGlobal ] = (_ELEM_VAL_RTH)(0.0);
			break;
		case  CTMCRTH_BLCKNTYPE_MAGNETIC :
			;
		case  CTMCRTH_BLCKNTYPE_ABSORBER :
			for( i = 0; i < nNumNode; i++)
			{
				// Узлы, у которых сосед выходит за массив, пропускаются.
				if(
					((2*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 1) > 0)&&
					((2*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 1) < nArraySize)&&
					((2*(pcNodeDielOne[i].nNodeGlobal + nX ) + 1) > 0)&&
					((2*(pcNodeDielOne[i].nNodeGlobal + nX ) + 1) < nArraySize)&&
					((2*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 1) > 0)&&
					((2*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 1) < nArraySize)&&
					((2*(pcNodeDielOne[i].nNodeGlobal - nX ) + 1) > 0)&&
					((2*(pcNodeDielOne[i].nNodeGlobal - nX ) + 1) < nArraySize)&&
					((2*pcNodeDielOne[i].nNodeGlobal + 1) > 0)&&
					((2*pcNodeDielOne[i].nNodeGlobal + 1) < nArraySize)
					)
				{
					u1  = prUNode1[ 2*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 1 ];
					u2  = prUNode1[ 2*(pcNodeDielOne[i].nNodeGlobal + nX ) + 1 ];
					u3  = prUNode1[ 2*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 1 ];
					u4  = prUNode1[ 2*(pcNodeDielOne[i].nNodeGlobal - nX ) + 1 ];
					ut1 = prUNode1[ 2*pcNodeDielOne[i].nNodeGlobal + 1 ];
					ut2 = pcNodeDielOne[i].rU;
					y = pcNodeDielOne[i].rY;
					y5 = prYNode[ 2*pcNodeDielOne[i].nNodeGlobal ];
//{		CString csBuf1;
					prU = prUNode1 + (2*pcNodeDielOne[i].nNodeGlobal);
					/// З-12 (автор 29.10): веса линий, доли шлейфа и нагрузки угловых узлов — по правилу ассемблирования
					/// общей схемы из схем элементарных объёмов (то же правило, что веса узлов в Р-2).
					// Схема узла с нагрузкой y и долей шлейфа y5 по коду соседства: ребро (y5*0,5, нагрузка y),
					// внешний угол (y5*0,75, y/2), внутренний угол 1111_k (y5*0,25, 1,5*y); 0000 и 1111 — 0;
					// коды с одним соседом или только с двумя противоположными — ошибка «Bad Absorber».
					switch( pcNodeDielOne[i].nType )
					{
						case CTMCRTH_BLCKNTYPE_ABSR_0000:
							prU[0] = (_ELEM_VAL_RTH)(0.0);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_0001:
							cError.PutErrorMessage("Bad Absorber or Magnetic Type 0001");
							return;
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_0010:
							cError.PutErrorMessage("Bad Absorber or Magnetic Type 0010");
							return;
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_0011:
							y5 *= 0.75;
							prU[0] = (_ELEM_VAL_RTH)((2./(3+fabs(y5)+y/2))*(u3+u4+0.5*(u2+u1)+y5*ut1+y/2*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_0100:
							cError.PutErrorMessage("Bad Absorber or Magnetic Type 0100");
							return;
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_0101:
							cError.PutErrorMessage("Bad Absorber or Magnetic Type 0101");
							return;
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_0110:
							y5 *= 0.75;
							prU[0] = (_ELEM_VAL_RTH)((2./(3+fabs(y5)+y/2))*(u1+u4+0.5*(u2+u3)+y5*ut1+y/2*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_0111:
							y5 *= 0.5;
							prU[0] = (_ELEM_VAL_RTH)((2./(2+fabs(y5)+y))*(u4+0.5*(u1+u3)+y5*ut1+y*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1000:
							cError.PutErrorMessage("Bad Absorber or Magnetic Type 1000");
							return;
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1001:
							y5 *= 0.75;
							prU[0] = (_ELEM_VAL_RTH)((2./(3+fabs(y5)+y/2))*(u3+u2+0.5*(u1+u4)+y5*ut1+y/2*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1010:
							cError.PutErrorMessage("Bad Absorber or Magnetic Type 1010");
							return;
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1011:
							y5 *= 0.5;
							prU[0] = (_ELEM_VAL_RTH)((2./(2+fabs(y5)+y))*(u3+0.5*(u2+u4)+y5*ut1+y*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1100:
							y5 *= 0.75;
							prU[0] = (_ELEM_VAL_RTH)((2./(3+fabs(y5)+y/2))*(u1+u2+0.5*(u4+u3)+y5*ut1+y/2*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1101:
							y5 *= 0.5;
							prU[0] = (_ELEM_VAL_RTH)((2./(2+fabs(y5)+y))*(u2+0.5*(u1+u3)+y5*ut1+y*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1110:
							y5 *= 0.5;
							prU[0] = (_ELEM_VAL_RTH)((2./(2+fabs(y5)+y))*(u1+0.5*(u2+u4)+y5*ut1+y*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1111:
							prU[0] = (_ELEM_VAL_RTH)(0.0);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1111_1:
							y5 *= 0.25;
							prU[0] = (_ELEM_VAL_RTH)((2./(1+fabs(y5)+1.5*y))*(0.5*(u1+u2)+y5*ut1+1.5*y*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1111_2:
							y5 *= 0.25;
							prU[0] = (_ELEM_VAL_RTH)((2./(1+fabs(y5)+1.5*y))*(0.5*(u2+u3)+y5*ut1+1.5*y*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1111_3:
							y5 *= 0.25;
							prU[0] = (_ELEM_VAL_RTH)((2./(1+fabs(y5)+1.5*y))*(0.5*(u3+u4)+y5*ut1+1.5*y*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1111_4:
							y5 *= 0.25;
							prU[0] = (_ELEM_VAL_RTH)((2./(1+fabs(y5)+1.5*y))*(0.5*(u1+u4)+y5*ut1+1.5*y*ut2) - ut2);
							break;
						};
				};
			};
			// Сохранить значение другой ячейки до того, как её перепишет следующий подтакт.
			for( i = 0; i < nNumNode; i++)
			{
				pcNodeDielOne[i].rU = (_ELEM_VAL_RTH)(prUNode1[ 2*pcNodeDielOne[i].nNodeGlobal + 1 ]);
			};
			break;
		default:
			break;
	};

	return;
}

void CTmcRTHNodeDiel::RunBlockNode1( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, TmcIdx nArraySize )
{
	if( nType == CTMCRTH_BLCKNTYPE_EPS ) return;
	if( cError.IsError() ) return;
	if( pcNodeDielOne == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return;
	};

	TmcIdx i;	// BIGBLOCK3
	double u1, u2, u3, u4, ut1, ut2, y5, y;
	_ELEM_VAL_RTH *prU;
	
	switch( nType )
	{
		case  CTMCRTH_BLCKNTYPE_METAL    :
			for( i = 0; i < nNumNode; i++)
				prUNode1[ 2*pcNodeDielOne[i].nNodeGlobal + 1 ] = (_ELEM_VAL_RTH)(0.0);
			break;
		case  CTMCRTH_BLCKNTYPE_MAGNETIC :
			;
		case  CTMCRTH_BLCKNTYPE_ABSORBER :
			for( i = 0; i < nNumNode; i++)
			{
				if(
					((2*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 1) > 0)&&
					((2*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 1) < nArraySize)&&
					((2*(pcNodeDielOne[i].nNodeGlobal + nX ) + 1) > 0)&&
					((2*(pcNodeDielOne[i].nNodeGlobal + nX ) + 1) < nArraySize)&&
					((2*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 1) > 0)&&
					((2*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 1) < nArraySize)&&
					((2*(pcNodeDielOne[i].nNodeGlobal - nX ) + 1) > 0)&&
					((2*(pcNodeDielOne[i].nNodeGlobal - nX ) + 1) < nArraySize)&&
					((2*pcNodeDielOne[i].nNodeGlobal + 1) > 0)&&
					((2*pcNodeDielOne[i].nNodeGlobal + 1) < nArraySize)
					)
				{
					u1  = prUNode1[ 2*(pcNodeDielOne[i].nNodeGlobal -  1 ) ];
					u2  = prUNode1[ 2*(pcNodeDielOne[i].nNodeGlobal + nX ) ];
					u3  = prUNode1[ 2*(pcNodeDielOne[i].nNodeGlobal +  1 ) ];
					u4  = prUNode1[ 2*(pcNodeDielOne[i].nNodeGlobal - nX ) ];
					ut1 = prUNode1[ 2*pcNodeDielOne[i].nNodeGlobal ];
					ut2 = pcNodeDielOne[i].rU;
					y = pcNodeDielOne[i].rY;
					y5 = prYNode[ 2*pcNodeDielOne[i].nNodeGlobal ];
					prU = prUNode1 + (2*pcNodeDielOne[i].nNodeGlobal+1);
					switch( pcNodeDielOne[i].nType )
					{
						case CTMCRTH_BLCKNTYPE_ABSR_0000:
							prU[0] = (_ELEM_VAL_RTH)(0.0);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_0001:
							cError.PutErrorMessage("Bad Absorber or Magnetic Type 0001");
							return;
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_0010:
							cError.PutErrorMessage("Bad Absorber or Magnetic Type 0010");
							return;
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_0011:
							y5 *= 0.75;
							prU[0] = (_ELEM_VAL_RTH)((2./(3+fabs(y5)+y/2))*(u3+u4+0.5*(u2+u1)+y5*ut1+y/2*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_0100:
							cError.PutErrorMessage("Bad Absorber or Magnetic Type 0100");
							return;
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_0101:
							cError.PutErrorMessage("Bad Absorber or Magnetic Type 0101");
							return;
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_0110:
							y5 *= 0.75;
							prU[0] = (_ELEM_VAL_RTH)((2./(3+fabs(y5)+y/2))*(u1+u4+0.5*(u2+u3)+y5*ut1+y/2*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_0111:
							y5 *= 0.5;
							prU[0] = (_ELEM_VAL_RTH)((2./(2+fabs(y5)+y))*(u4+0.5*(u1+u3)+y5*ut1+y*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1000:
							cError.PutErrorMessage("Bad Absorber or Magnetic Type 1000");
							return;
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1001:
							y5 *= 0.75;
							prU[0] = (_ELEM_VAL_RTH)((2./(3+fabs(y5)+y/2))*(u3+u2+0.5*(u1+u4)+y5*ut1+y/2*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1010:
							cError.PutErrorMessage("Bad Absorber or Magnetic Type 1010");
							return;
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1011:
							y5 *= 0.5;
							prU[0] = (_ELEM_VAL_RTH)((2./(2+fabs(y5)+y))*(u3+0.5*(u2+u4)+y5*ut1+y*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1100:
							y5 *= 0.75;
							prU[0] = (_ELEM_VAL_RTH)((2./(3+fabs(y5)+y/2))*(u1+u2+0.5*(u4+u3)+y5*ut1+y/2*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1101:
							y5 *= 0.5;
							prU[0] = (_ELEM_VAL_RTH)((2./(2+fabs(y5)+y))*(u2+0.5*(u1+u3)+y5*ut1+y*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1110:
							y5 *= 0.5;
							prU[0] = (_ELEM_VAL_RTH)((2./(2+fabs(y5)+y))*(u1+0.5*(u2+u4)+y5*ut1+y*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1111:
							prU[0] = (_ELEM_VAL_RTH)(0.0);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1111_1:
							y5 *= 0.25;
							prU[0] = (_ELEM_VAL_RTH)((2./(1+fabs(y5)+1.5*y))*(0.5*(u1+u2)+y5*ut1+1.5*y*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1111_2:
							y5 *= 0.25;
							prU[0] = (_ELEM_VAL_RTH)((2./(1+fabs(y5)+1.5*y))*(0.5*(u2+u3)+y5*ut1+1.5*y*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1111_3:
							y5 *= 0.25;
							prU[0] = (_ELEM_VAL_RTH)((2./(1+fabs(y5)+1.5*y))*(0.5*(u3+u4)+y5*ut1+1.5*y*ut2) - ut2);
							break;
						case CTMCRTH_BLCKNTYPE_ABSR_1111_4:
							y5 *= 0.25;
							prU[0] = (_ELEM_VAL_RTH)((2./(1+fabs(y5)+1.5*y))*(0.5*(u1+u4)+y5*ut1+1.5*y*ut2) - ut2);
							break;
					};
				};
			};
			for( i = 0; i < nNumNode; i++)
			{
				pcNodeDielOne[i].rU = (_ELEM_VAL_RTH)(prUNode1[ 2*pcNodeDielOne[i].nNodeGlobal ]);
			};
			break;
		default:
			break;
	};

	return;
}


CTmcLibError & CTmcRTHNodeDiel::SetPolyMetal(CTmcRTH_BlockList *pcNextBlockList, CTmcRTH_IndanParam &cParam)
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	double *pX, *pY;
	int nXY, i_step, j_step;
//	int njj, jj;
	int nX1, nX2, nY1, nY2;
	int j, k, nX, nY;
	TmcIdx i, ii;	// BIGBLOCK3
	double r, x, y, x1, y1;
	CString csBuf1;

	if( ( pX = pcNextBlockList->GetpX() ) == NULL )
	{				
		cError.PutErrorMessage("Init pointer pX is NULL for block ");
		return cError;
	};

	if( ( pY = pcNextBlockList->GetpY() ) == NULL )
	{				
		cError.PutErrorMessage("Init pointer pY is NULL for block ");
		return cError;
	};

	if( ( nXY = pcNextBlockList->GetnXY() ) < 3 )
	{				
		cError.PutErrorMessage("Init nXY < 3 for block ");
		return cError;
	};

	r = pX[0] + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pY[0] + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);
	
							// calculated nNumNode
	for( k = 1, nNumNode = 0; k < nXY; k++ )
	{
		r = pX[k] + pcNextBlockList->GetX0();
		r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Xmin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nX2 = (int)(r);	
		
		r = pY[k] + pcNextBlockList->GetY0();
		r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Ymin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nY2 = (int)(r);		
		
		nNumNode += abs( nY1 - nY2 ) + abs( nX1 - nX2 );
		
		nX1 = nX2;
		nY1 = nY2;	
	};
		

	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	
							// allocation memory
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};
	for( i = 0; i < nNumNode; i++ ) pcNodeDielOne[i].nNodeGlobal = 1;

	r = pX[0] + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pY[0] + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);
	
							// set Node
	for( k = 1, ii = 0; k < nXY; k++ )
	{
		r = pX[k] + pcNextBlockList->GetX0();
		r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Xmin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nX2 = (int)(r);	
		
		r = pY[k] + pcNextBlockList->GetY0();
		r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Ymin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nY2 = (int)(r);

		nY = abs( nY1 - nY2 );
		y1 = pY[k-1];
		if( nY1 < nY2 ) 
		{
			i_step = 1;
		}
		else 
		{
			i_step = -1;
		};

		nX = abs( nX1 - nX2 );
		x1 = pX[k-1];
		if( nX1 < nX2 )
		{
			j_step = 1;
		}
		else 
		{
			j_step = -1;
		};

//	csBuf1.Format("nY = %d; nX = %d", nY, nX);
//	AfxMessageBox(csBuf1);		
		// Ступенчатая аппроксимация отрезка: по оси с большим числом шагов идём каждый шаг,
		// по другой — равномерно; соседние узлы связаны по стороне (без диагоналей).
		if( nY < nX )
		{
			for( i = 0, j = 0; (i < nY)&&(j < nX) ; i++ )
			{
				for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++, ii++ )
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
				};
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
					ii++;
			};
				for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++, ii++ )
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
				};
		}
		else
		{
			for( i = 0, j = 0; (i < nX)&&(j < nY) ; i++ )
			{
				for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++, ii++ )
				{
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
				};
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
					ii++;
			};
				for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++, ii++ )
				{
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
				};
		};

		r = pX[k] + pcNextBlockList->GetX0();
		r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Xmin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nX1 = (int)(r);	
		
		r = pY[k] + pcNextBlockList->GetY0();
		r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Ymin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nY1 = (int)(r);
	};

/*		
	r = pX[0] + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX2 = (int)(r);	
	
	r = pY[0] + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY2 = (int)(r);
	nY = abs( nY1 - nY2 );
	y1 = pY[k-1];
	if( nY1 < nY2 ) 
	{
		i_step = 1;
	}
	else 
	{
		i_step = -1;
	};
	nX = abs( nX1 - nX2 );
	x1 = pX[k-1];
	if( nX1 < nX2 )
	{
		j_step = 1;
	}
	else 
	{
		j_step = -1;
	};
		if( nY < nX )
		{
			for( i = 0, j = 0, njj = nX/(nY+1)+1; (i < nY)&&(j < nX) ; i++ )
			{
				for( jj = 0; (jj < njj)&&(j < nX); j++, ii++, jj++ )
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
				};
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
					ii++;
			};
				for( jj = 0; (jj < njj)&&(j < nX); j++, ii++, jj++ )
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
				};
		}
		else
		{
			for( i = 0, j = 0, njj = nY/(nX+1)+1; (i < nX)&&(j < nY) ; i++ )
			{
				for( jj = 0; (jj < njj)&&(j < nY); j++, ii++, jj++ )
				{
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
				};
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
					ii++;
			};
				for( jj = 0; (jj < njj)&&(j < nY); j++, ii++, jj++ )
				{
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
				};
		};
*/
	{	// B-16: the nodes of the parts outside the region (marked -1) are dropped
		TmcIdx a, b;
		for( a = 0, b = 0; a < nNumNode; a++ )	// after DeleteDublicateData nNumNode is the number of nodes
		{
			if( pcNodeDielOne[a].nNodeGlobal < 0 ) continue;
			if( a != b ) pcNodeDielOne[b] = pcNodeDielOne[a];
			b++;
		};
		nNumNode = b;
	};
	return cError;
}

CTmcLibError & CTmcRTHNodeDiel::SetCircMetal(CTmcRTH_BlockList *pcNextBlockList, CTmcRTH_IndanParam &cParam)
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	int i_step, j_step, njj, jj;
	int nX1, nX2, nY1, nY2;
	int j, k1, k2, nX, nY;
	TmcIdx i, ii;	// BIGBLOCK3
	double r, x, y, x1, y1, df;

							// calculated nNumNode
	nNumNode = 0;
	r = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);

	r = pcNextBlockList->GetXmax()*cos( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX2 = (int)(r);	
	
	r = pcNextBlockList->GetXmax()*sin( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY2 = (int)(r);		
		
	nNumNode += abs( nY1 - nY2 ) + abs( nX1 - nX2 );
	
	r = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);

	r = pcNextBlockList->GetXmax()*cos( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX2 = (int)(r);	
	
	r = pcNextBlockList->GetXmax()*sin( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY2 = (int)(r);		
		
	nNumNode += abs( nY1 - nY2 ) + abs( nX1 - nX2 );
	
	k1 = 2*(int)((pcNextBlockList->GetXmin()*( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() ) )/cParam.rDelta + 1 );
	nNumNode += k1;
	
	k2 = 2*(int)((pcNextBlockList->GetXmax()*( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() ) )/cParam.rDelta + 1 );
	nNumNode += k2;
	
							// calculated nNumNode

	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	
							// allocation memory
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};
	for( i = 0; i < nNumNode; i++ ) pcNodeDielOne[i].nNodeGlobal = 1;

							// set Node
	ii = 0;
	r = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);

	r = pcNextBlockList->GetXmax()*cos( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX2 = (int)(r);	
	
	r = pcNextBlockList->GetXmax()*sin( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY2 = (int)(r);		
		
	nY = abs( nY1 - nY2 );
	y1 = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmin() );
	if( nY1 < nY2 ) 
	{
		i_step = 1;
	}
	else 
	{
		i_step = -1;
	};

	nX = abs( nX1 - nX2 );
	x1 = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmin() );
	if( nX1 < nX2 )
	{
		j_step = 1;
	}
	else 
	{
		j_step = -1;
	};

	if( nY < nX )
	{
		for( i = 0, j = 0; (i < nY)&&(j < nX) ; i++ )	// B-4 (old: njj = nX/(nY+1)+1 - the staircase stopped halfway)
		{
			for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++, ii++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
			};
			x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(0.0);
			ii++;
			};
			for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++, ii++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
			};
	}
	else
	{
		for( i = 0, j = 0; (i < nX)&&(j < nY) ; i++ )	// B-4 (old: njj = nY/(nX+1)+1)
		{
			for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++, ii++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
			};
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
				ii++;
		};
			for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++, ii++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
			};
	};


	r = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);

	r = pcNextBlockList->GetXmax()*cos( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX2 = (int)(r);	
	
	r = pcNextBlockList->GetXmax()*sin( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY2 = (int)(r);		

	nY = abs( nY1 - nY2 );
	y1 = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmax() );
	if( nY1 < nY2 ) 
	{
		i_step = 1;
	}
	else 
	{
		i_step = -1;
	};

	nX = abs( nX1 - nX2 );
	x1 = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmax() );
	if( nX1 < nX2 )
	{
		j_step = 1;
	}
	else 
	{
		j_step = -1;
	};

	if( nY < nX )
	{
		for( i = 0, j = 0; (i < nY)&&(j < nX) ; i++ )	// B-4 (old: njj = nX/(nY+1)+1 - the staircase stopped halfway)
		{
			for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++, ii++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
			};
			x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(0.0);
			ii++;
			};
			for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++, ii++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
			};
	}
	else
	{
		for( i = 0, j = 0; (i < nX)&&(j < nY) ; i++ )	// B-4 (old: njj = nY/(nX+1)+1)
		{
			for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++, ii++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
			};
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
				ii++;
		};
			for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++, ii++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
			};
	};

	if( k1 > 2 ) df = ( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() )/( k1 - 2 );
	else df = 0.0;
	for( i = 0, r = pcNextBlockList->GetYmin(); i < k1; i++, ii++, r += df )
	{
		x = pcNextBlockList->GetXmin()*cos(r) + pcNextBlockList->GetX0();
		y = pcNextBlockList->GetXmin()*sin(r) + pcNextBlockList->GetY0();
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(0.0);
	};

	if( k2 > 2 ) df = ( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() )/( k2 - 2 );
	else df = 0.0;
	for( i = 0, r = pcNextBlockList->GetYmin(); i < k2; i++, ii++, r += df )
	{
		x = pcNextBlockList->GetXmax()*cos(r) + pcNextBlockList->GetX0();
		y = pcNextBlockList->GetXmax()*sin(r) + pcNextBlockList->GetY0();
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		if( TmcB16Out( cParam, x, y ) ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(0.0);
	};

	{	// B-16: the nodes of the parts outside the region (marked -1) are dropped
		TmcIdx a, b;
		for( a = 0, b = 0; a < nNumNode; a++ )	// after DeleteDublicateData nNumNode is the number of nodes
		{
			if( pcNodeDielOne[a].nNodeGlobal < 0 ) continue;
			if( a != b ) pcNodeDielOne[b] = pcNodeDielOne[a];
			b++;
		};
		nNumNode = b;
	};
	return cError;
}

CTmcLibError & CTmcRTHNodeDiel::SetRectAbsorber(CTmcRTH_BlockList *pcNextBlockList, CTmcRTH_IndanParam &cParam)
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	int nXmin, nXmax, nYmin, nYmax;
	int nX, nY;
	TmcIdx i, ii;	// BIGBLOCK3
	double dXmin, dXmax, dYmin, dYmax, r, x, y, dYAdmit;

	dYAdmit = SetAbsorberAdmitance( cParam.rFreq, cParam.rDelta );

	dXmin = pcNextBlockList->GetXmin() + pcNextBlockList->GetX0();
	if( dXmin <= (cParam.rXmin + cParam.rDelta) ) dXmin = cParam.rXmin + cParam.rDelta;
	r = (dXmin - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmin = (int)(r);

	dXmax = pcNextBlockList->GetXmax() + pcNextBlockList->GetX0();
	if( dXmax >= (cParam.rXmax - 2*cParam.rDelta) ) dXmax = cParam.rXmax - 2*cParam.rDelta;
	// B-10 (26.10.2026): +0.5 - the ring of the absorber is closed (the border on a node keeps its node).
	// Was: r = ... - 0.5; then - 0.499999 (the same index for a border on a node - two corners without a node).
	// To return the old variant: r = (dXmax - cParam.rXmin)/cParam.rDelta - 0.5;
	r = (dXmax - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmax = (int)(r);
	
	dYmin = pcNextBlockList->GetYmin() + pcNextBlockList->GetY0();
	if( dYmin <= (cParam.rYmin + cParam.rDelta) ) dYmin = cParam.rYmin + cParam.rDelta;
	r = (dYmin - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmin = (int)(r);

	dYmax = pcNextBlockList->GetYmax() + pcNextBlockList->GetY0();
	if( dYmax >= (cParam.rYmax - 2*cParam.rDelta) ) dYmax = cParam.rYmax - 2*cParam.rDelta;
	// B-10 (26.10.2026): +0.5 (was - 0.5, then - 0.499999; to return: r = (dYmax - cParam.rYmin)/cParam.rDelta - 0.5;)
	r = (dYmax - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmax = (int)(r);
	
	if( nXmax <= nXmin )
	{
		CString csBuf;
		csBuf.Format("Xmax <= Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	if( (nXmax - nXmin) < 2 )
	{
		CString csBuf;
		csBuf.Format("Xmax - Xmin for block %d must be >= 2*delta(%lg m) for type ABSORBER", nBlock, 2*cParam.rDelta);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX = nXmax - nXmin + 1;

	if( nYmax <= nYmin )
	{
		CString csBuf;
		csBuf.Format("Ymax <= Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	if( (nYmax - nYmin) < 2 )
	{
		CString csBuf;
		csBuf.Format("Ymax - Ymin for block %d must be >= 2*delta(%lg m) for type ABSORBER", nBlock, 2*cParam.rDelta);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY = nYmax - nYmin + 1;

	nNumNode = 2*nX + 2*(nY-2) + 2*(nX-2)+2*(nY-3);

//{	CString csBuf1;


//{	CString csBuf1;

	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};

	for( i = 0; i < nNumNode; i++ )
	{
		pcNodeDielOne[i].nNodeGlobal = 0;
	};

	// Кольцо узлов на шаг внутри контура — код 0000 (напряжение 0).
	for( i = 0, ii = 0, x = dXmin + cParam.rDelta, y = dYmin + cParam.rDelta; i < (nX-2); i++, x+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};

	for( i = 0, x = dXmin + cParam.rDelta, y = dYmax - cParam.rDelta; i < (nX-2); i++, x+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};

	for( i = 0, x = dXmin + cParam.rDelta, y = dYmin + cParam.rDelta; i < (nY - 3); i++, y+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};

	for( i = 0, x = dXmax - cParam.rDelta, y = dYmin + cParam.rDelta; i < (nY - 3); i++, y+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};

	{
		// Внешний контур: нижняя сторона с углами, затем верхняя, левая и правая стороны.
		x = dXmin;
		y = dYmin;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0110;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
		x+=cParam.rDelta;
		for( i = 1; i < (nX-1); i++, x+=cParam.rDelta )
		{
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0111;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			ii++;
		};
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0011;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};
		
	{
		x = dXmin;
		y = dYmax;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_1100;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
		x+=cParam.rDelta;
		for( i = 1; i < (nX-1); i++, x+=cParam.rDelta )
		{
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_1101;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			ii++;
		};
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_1001;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};

	for( i = 0, x = dXmin, y = dYmin+cParam.rDelta; i < (nY - 2); i++, y+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_1110;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};

	for( i = 0, x = dXmax, y = dYmin+cParam.rDelta; i < (nY - 2); i++, y+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_1011;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};

//{	CString csBuf1;

	return cError;
}


CTmcLibError & CTmcRTHNodeDiel::SetRectMagnetic(CTmcRTH_BlockList *pcNextBlockList, CTmcRTH_IndanParam &cParam)
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	int nXmin, nXmax, nYmin, nYmax;
	int nX, nY;
	TmcIdx i, ii;	// BIGBLOCK3
	double dXmin, dXmax, dYmin, dYmax, r, x, y, dYAdmit;

	dYAdmit = 0.0;

	dXmin = pcNextBlockList->GetXmin() + pcNextBlockList->GetX0();
	if( dXmin < (cParam.rXmin + cParam.rDelta) ) dXmin = cParam.rXmin + cParam.rDelta;
	r = (dXmin - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmin = (int)(r);

	dXmax = pcNextBlockList->GetXmax() + pcNextBlockList->GetX0();
	if( dXmax > (cParam.rXmax - cParam.rDelta) ) dXmax = cParam.rXmax - cParam.rDelta;
	r = (dXmax - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nXmax = (int)(r);
	
	dYmin = pcNextBlockList->GetYmin() + pcNextBlockList->GetY0();
	if( dYmin < (cParam.rYmin + cParam.rDelta) ) dYmin = cParam.rYmin + cParam.rDelta;
	/// З-4 (автор 29.10): нижняя граница округляется вниз (-0,5), к верхней добавляется шаг — края сдвигаются в
	/// противоположные стороны, и область стенки получается симметричной. Так и задумано, не ошибка.
	r = (dYmin - cParam.rYmin)/cParam.rDelta - 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmin = (int)(r);

	dYmax = pcNextBlockList->GetYmax() + pcNextBlockList->GetY0() + cParam.rDelta;
	if( dYmax > (cParam.rYmax - cParam.rDelta) ) dYmax = cParam.rYmax - cParam.rDelta;
	r = (dYmax - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nYmax = (int)(r);
	
	if( nXmax < nXmin )
	{
		CString csBuf;
		csBuf.Format("Xmax < Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	if( (nXmax - nXmin) < 2 )
	{
		CString csBuf;
		csBuf.Format("Xmax - Xmin for block %d must be >= delta(%lg m) for type %s", nBlock, cParam.rDelta, CTMCRTH_INDANBLCK_TYPEMAG);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX = nXmax - nXmin + 1;

	if( nYmax < nYmin )
	{
		CString csBuf;
		csBuf.Format("Ymax < Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	if( (nYmax - nYmin) < 2 )
	{
		CString csBuf;
		csBuf.Format("Ymax - Ymin for block %d must be >= delta(%lg m) for type %s", nBlock, cParam.rDelta, CTMCRTH_INDANBLCK_TYPEMAG);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY = nYmax - nYmin + 1;

	nNumNode = 2*nX + 2*(nY-2) + 2*(nX-2)+2*(nY-3);

//{	CString csBuf1;


//{	CString csBuf1;

	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};

	for( i = 0; i < nNumNode; i++ )
	{
		pcNodeDielOne[i].nNodeGlobal = 0;
	};

	for( i = 0, ii = 0, x = dXmin + cParam.rDelta, y = dYmin + cParam.rDelta; i < (nX-2); i++, x+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobalFM( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};

	for( i = 0, x = dXmin + cParam.rDelta, y = dYmax - cParam.rDelta; i < (nX-2); i++, x+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobalFM( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};

	for( i = 0, x = dXmin + cParam.rDelta, y = dYmin + cParam.rDelta; i < (nY - 3); i++, y+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobalFM( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};

	for( i = 0, x = dXmax - cParam.rDelta, y = dYmin + cParam.rDelta; i < (nY - 3); i++, y+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobalFM( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};

	{
		x = dXmin;
		y = dYmin;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobalFM( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0110;
		if( x < (cParam.rXmin + 1.5*cParam.rDelta) ) pcNodeDielOne[ii].nType += CTMCRTH_BLCKNTYPE_ABSR_0001;
		if( y < (cParam.rYmin + 1.5*cParam.rDelta) ) pcNodeDielOne[ii].nType += CTMCRTH_BLCKNTYPE_ABSR_1000;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
		x+=cParam.rDelta;
		for( i = 1; i < (nX-1); i++, x+=cParam.rDelta )
		{
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobalFM( (_real)x, (_real)y );
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0111;
			if( y < (cParam.rYmin + 1.5*cParam.rDelta) ) pcNodeDielOne[ii].nType += CTMCRTH_BLCKNTYPE_ABSR_1000;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			ii++;
		};
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobalFM( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0011;
		if( x > (cParam.rXmax - 1.5*cParam.rDelta) ) pcNodeDielOne[ii].nType += CTMCRTH_BLCKNTYPE_ABSR_0100;
		if( y < (cParam.rYmin + 1.5*cParam.rDelta) ) pcNodeDielOne[ii].nType += CTMCRTH_BLCKNTYPE_ABSR_1000;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};
		
	{
		x = dXmin;
		y = dYmax;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobalFM( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_1100;
		if( x < (cParam.rXmin + 1.5*cParam.rDelta) ) pcNodeDielOne[ii].nType += CTMCRTH_BLCKNTYPE_ABSR_0001;
		if( y > (cParam.rYmax - 1.5*cParam.rDelta) ) pcNodeDielOne[ii].nType += CTMCRTH_BLCKNTYPE_ABSR_0010;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
		x+=cParam.rDelta;
		for( i = 1; i < (nX-1); i++, x+=cParam.rDelta )
		{
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobalFM( (_real)x, (_real)y );
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_1101;
			if( y > (cParam.rYmax - 1.5*cParam.rDelta) ) pcNodeDielOne[ii].nType += CTMCRTH_BLCKNTYPE_ABSR_0010;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			ii++;
		};
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobalFM( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_1001;
		if( x > (cParam.rXmax - 1.5*cParam.rDelta) ) pcNodeDielOne[ii].nType += CTMCRTH_BLCKNTYPE_ABSR_0100;
		if( y > (cParam.rYmax - 1.5*cParam.rDelta) ) pcNodeDielOne[ii].nType += CTMCRTH_BLCKNTYPE_ABSR_0010;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};

	for( i = 0, x = dXmin, y = dYmin+cParam.rDelta; i < (nY - 2); i++, y+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobalFM( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_1110;
		if( x < (cParam.rXmin + 1.5*cParam.rDelta) ) pcNodeDielOne[ii].nType += CTMCRTH_BLCKNTYPE_ABSR_0001;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};

	for( i = 0, x = dXmax, y = dYmin+cParam.rDelta; i < (nY - 2); i++, y+=cParam.rDelta )
	{
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobalFM( (_real)x, (_real)y );
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_1011;
		if( x > (cParam.rXmax - 1.5*cParam.rDelta) ) pcNodeDielOne[ii].nType += CTMCRTH_BLCKNTYPE_ABSR_0100;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		ii++;
	};

//{	CString csBuf1;

	return cError;
}



double CTmcRTHNodeDiel::atan2__1(double y, double x)
{
	double f;
	f = atan2( y, x );
	if( f < 0.0 ) f += 2*PI___;
	return f;
}


CTmcLibError & CTmcRTHNodeDiel::SetPolyAbsorber(CTmcRTH_BlockList *pcNextBlockList, CTmcRTH_IndanParam &cParam)
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	double *pX, *pY;
	int nXY, i_step, j_step, njj, jj;
	int nX1, nX2, nY1, nY2;
	int j, k, nX, nY, n_x, n_y;
	TmcIdx i, ii;	// BIGBLOCK3
	double r, x, y, x1, y1, dYAdmit;
	BOOL bB16Out = FALSE;	// B-16
//	CString csBuf1;
	dYAdmit = SetAbsorberAdmitance( cParam.rFreq, cParam.rDelta );
	n_x = cParam.CalcnX();	// B-2 (old: (int)((cParam.rXmax - cParam.rXmin)/cParam.rDelta + 1 ))
	n_y = cParam.CalcnY();	// B-2 (old: (int)((cParam.rYmax - cParam.rYmin)/cParam.rDelta + 1 ))

	if( ( pX = pcNextBlockList->GetpX() ) == NULL )
	{				
		cError.PutErrorMessage("Init pointer pX is NULL for block ");
		return cError;
	};

	if( ( pY = pcNextBlockList->GetpY() ) == NULL )
	{				
		cError.PutErrorMessage("Init pointer pY is NULL for block ");
		return cError;
	};

	if( ( nXY = pcNextBlockList->GetnXY() ) < 3 )
	{				
		cError.PutErrorMessage("Init nXY < 3 for block ");
		return cError;
	};

	r = pX[0] + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pY[0] + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);
	
							// calculated nNumNode
	for( k = 1, nNumNode = 0; k < nXY; k++ )
	{
		r = pX[k] + pcNextBlockList->GetX0();
		r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Xmin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nX2 = (int)(r);	
		
		r = pY[k] + pcNextBlockList->GetY0();
		r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Ymin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nY2 = (int)(r);		
		
		nNumNode += abs( nY1 - nY2 ) + abs( nX1 - nX2 );
		
		nX1 = nX2;
		nY1 = nY2;	
	};

	nNumNode *=4;
		

	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	
							// allocation memory
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};
	
	for( i = 0; i < nNumNode; i++ ) pcNodeDielOne[i].nNodeGlobal = 1;

	r = pX[0] + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pY[0] + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);
	
							// set Node
	for( k = 1, ii = 0; k < nXY; k++ )
	{
		r = pX[k] + pcNextBlockList->GetX0();
		r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Xmin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nX2 = (int)(r);	
		
		r = pY[k] + pcNextBlockList->GetY0();
		r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Ymin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nY2 = (int)(r);

		nY = abs( nY1 - nY2 );
		y1 = pY[k-1];
		if( nY1 < nY2 ) 
		{
			i_step = 1;
		}
		else 
		{
			i_step = -1;
		};

		nX = abs( nX1 - nX2 );
		x1 = pX[k-1];
		if( nX1 < nX2 )
		{
			j_step = 1;
		}
		else 
		{
			j_step = -1;
		};

		if( nY < nX )
		{
			for( i = 0, j = 0; (i < nY)&&(j < nX) ; i++ )	// B-4 (old: njj = nX/(nY+1)+1 - the staircase stopped halfway)
			{
				for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++ )	// B-4 (old: jj < njj)
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
				};
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

			};
				for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++ )	// B-4 (old: jj < njj)
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

				};
		}
		else
		{
			for( i = 0, j = 0; (i < nX)&&(j < nY) ; i++ )	// B-4 (old: njj = nY/(nX+1)+1)
			{
				for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++ )	// B-4 (old: jj < njj)
				{
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

				};
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

			};
				for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++ )	// B-4 (old: jj < njj)
				{
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					
				};
		};

		r = pX[k] + pcNextBlockList->GetX0();
		r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Xmin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nX1 = (int)(r);	
		
		r = pY[k] + pcNextBlockList->GetY0();
		r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Ymin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nY1 = (int)(r);
	};

	DeleteDublicateData();
	{	// B-16: the nodes of the parts outside the region (marked -1) are dropped
		TmcIdx a, b;
		for( a = 0, b = 0; a < nNumNode; a++ )	// after DeleteDublicateData nNumNode is the number of nodes
		{
			if( pcNodeDielOne[a].nNodeGlobal < 0 ) continue;
			if( a != b ) pcNodeDielOne[b] = pcNodeDielOne[a];
			b++;
		};
		nNumNode = b;
	};
	SetAbsorberNodeType( cParam );
	return cError;
}


void CTmcRTHNodeDiel::SetAbsorberNodeType( CTmcRTH_IndanParam &cParam )
{
	TmcIdx i, j;	// BIGBLOCK
	int nx, nn;
	TmcIdx n;	// INT64

	nx = cParam.CalcnX();	// B-2 (old: (int)((cParam.rXmax - cParam.rXmin)/cParam.rDelta + 1 ))

	// Перебор всех пар узлов блока: O(N^2).
	for( i = 0; i < nNumNode; i++ ) 
	{
		pcNodeDielOne[i].nType = 0;
	
		for( j = 0, n = pcNodeDielOne[i].nNodeGlobal - 1; j < nNumNode; j++ )
		{
			if( pcNodeDielOne[j].nNodeGlobal == n )
			{
				pcNodeDielOne[i].nType += CTMCRTH_BLCKNTYPE_ABSR_0001;
				j = nNumNode;
			};
		};
		for( j = 0, n = pcNodeDielOne[i].nNodeGlobal + nx; j < nNumNode; j++ )
		{
			if( pcNodeDielOne[j].nNodeGlobal == n )
			{
				pcNodeDielOne[i].nType += CTMCRTH_BLCKNTYPE_ABSR_0010;
				j = nNumNode;
			};
		};
		for( j = 0, n = pcNodeDielOne[i].nNodeGlobal + 1; j < nNumNode; j++ )
		{
			if( pcNodeDielOne[j].nNodeGlobal == n )
			{
				pcNodeDielOne[i].nType += CTMCRTH_BLCKNTYPE_ABSR_0100;
				j = nNumNode;
			};
		};
		for( j = 0, n = pcNodeDielOne[i].nNodeGlobal - nx; j < nNumNode; j++ )
		{
			if( pcNodeDielOne[j].nNodeGlobal == n )
			{
				pcNodeDielOne[i].nType += CTMCRTH_BLCKNTYPE_ABSR_1000;
				j = nNumNode;
			};
		};
	
		if( pcNodeDielOne[i].nType == 0 ) pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		
		if( pcNodeDielOne[i].nType == CTMCRTH_BLCKNTYPE_ABSR_1111 )
		{
			for( j = 0, nn = 0, n = pcNodeDielOne[i].nNodeGlobal + nx - 1; j < nNumNode; j++ )
			{
				if( pcNodeDielOne[j].nNodeGlobal == n )
				{
					nn = 1;
					j = nNumNode;
				};
			};
			if( nn == 0 ) pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_ABSR_1111_1;
			
			for( j = 0, nn = 0, n = pcNodeDielOne[i].nNodeGlobal + nx + 1; j < nNumNode; j++ )
			{
				if( pcNodeDielOne[j].nNodeGlobal == n )
				{
					nn = 1;
					j = nNumNode;
				};
			};
			if( nn == 0 ) pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_ABSR_1111_2;
			
			for( j = 0, nn = 0, n = pcNodeDielOne[i].nNodeGlobal - nx + 1; j < nNumNode; j++ )
			{
				if( pcNodeDielOne[j].nNodeGlobal == n )
				{
					nn = 1;
					j = nNumNode;
				};
			};
			if( nn == 0 ) pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_ABSR_1111_3;
			
			for( j = 0, nn = 0, n = pcNodeDielOne[i].nNodeGlobal - nx - 1; j < nNumNode; j++ )
			{
				if( pcNodeDielOne[j].nNodeGlobal == n )
				{
					nn = 1;
					j = nNumNode;
				};
			};
			if( nn == 0 ) pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_ABSR_1111_4;
		};
//			CString csBuf1;
//			csBuf1.Format("Type = %d; nNode = %d; x = %lg; y = %lg", pcNodeDielOne[i].nType, pcNodeDielOne[i].nNodeGlobal,
//				cParam.CalcX( pcNodeDielOne[i].nNodeGlobal ),
	};
	return;
}

void CTmcRTHNodeDiel::DeleteDublicateData()
{
	if( cError.IsError() ) return;
	if( nNumNode == 0 ) return;
	if( pcNodeDielOne == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return;
	};
	
	if( nType == CTMCRTH_BLCKNTYPE_EPS ) return;
	if( nType == CTMCRTH_BLCKNTYPE_METAL )
	{
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTAT, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == 0 ) 
			{
				return;
			};
	};

	{
		CString csBuf;
		csBuf.Format("Delete Dublicated Data");
		PutStatistics(csBuf);
	};
	TmcIdx i, j, n;	// BIGBLOCK3
	TmcIdx n1;	// INT64
	sTmcRTHNodeDielOne *pcNodeDielOne1;
	
	for( i = 0; i < nNumNode; i++ )
	{
		if( ( n1 = pcNodeDielOne[i].nNodeGlobal ) != -1 )
		{
			if( !pbIsStop[0] )
			{
				CString csBuf;
				csBuf.Format("User cancel delete dublicated data thread in Block %d", nBlock);
				cError.PutErrorMessage(csBuf);
				return;
			};
			for( j = i+1; j < nNumNode; j++ )
			{
				if( pcNodeDielOne[j].nNodeGlobal == n1 )
				{
					pcNodeDielOne[j].nNodeGlobal = -1;
				};
			};
		};
	};

	for( i = 0, n = 0; i < nNumNode; i++ )
	{
		if( pcNodeDielOne[i].nNodeGlobal != -1 )
		{
			n++;
		};
	};

	if( n == 0 ) return;

							// allocation memory
	if( (pcNodeDielOne1 = new sTmcRTHNodeDielOne[n]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block DeleteDublicatedFunction");
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
		pcNodeDielOne = NULL;
		return;
	};

	for( i = 0, n = 0; i < nNumNode; i++ )
	{
		if( pcNodeDielOne[i].nNodeGlobal != -1 )
		{
			pcNodeDielOne1[n].nNodeGlobal = pcNodeDielOne[i].nNodeGlobal;
			pcNodeDielOne1[n].nType = pcNodeDielOne[i].nType;
			pcNodeDielOne1[n].rU    = pcNodeDielOne[i].rU;
			pcNodeDielOne1[n].rUo   = pcNodeDielOne[i].rUo;
			pcNodeDielOne1[n].rY    = pcNodeDielOne[i].rY;
			n++;
		};
	};
	
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = pcNodeDielOne1;
	nNumNode = n;

	return;
}

CTmcLibError & CTmcRTHNodeDiel::GetError()
{
	return cError;
}


CTmcLibError & CTmcRTHNodeDiel::SetCircAbsorber(CTmcRTH_BlockList *pcNextBlockList, CTmcRTH_IndanParam &cParam)
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	int i_step, j_step, njj, jj;
	int nX1, nX2, nY1, nY2;
	int j, k1, k2, nX, nY, n_x, n_y;
	TmcIdx i, ii;	// BIGBLOCK3
	double r, x, y, x1, y1, df, dYAdmit;
	BOOL bB16Out = FALSE;	// B-16

	dYAdmit = SetAbsorberAdmitance( cParam.rFreq, cParam.rDelta );
	n_x = cParam.CalcnX();	// B-2 (old: (int)((cParam.rXmax - cParam.rXmin)/cParam.rDelta + 1 ))
	n_y = cParam.CalcnY();	// B-2 (old: (int)((cParam.rYmax - cParam.rYmin)/cParam.rDelta + 1 ))

							// calculated nNumNode
	nNumNode = 0;
	r = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);

	r = pcNextBlockList->GetXmax()*cos( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX2 = (int)(r);	
	
	r = pcNextBlockList->GetXmax()*sin( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY2 = (int)(r);		
		
	nNumNode += abs( nY1 - nY2 ) + abs( nX1 - nX2 );
	
	r = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);

	r = pcNextBlockList->GetXmax()*cos( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX2 = (int)(r);	
	
	r = pcNextBlockList->GetXmax()*sin( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY2 = (int)(r);		
		
	nNumNode += abs( nY1 - nY2 ) + abs( nX1 - nX2 );
	
	k1 = 10*(int)((pcNextBlockList->GetXmin()*( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() ) )/cParam.rDelta + 1 );
	nNumNode += k1;
	
	k2 = 10*(int)((pcNextBlockList->GetXmax()*( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() ) )/cParam.rDelta + 1 );
	nNumNode += k2;
	
	nNumNode *= 4;
//		CString csBuf1;
	
	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	
							// allocation memory
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};
	for( i = 0; i < nNumNode; i++ ) 
	{
		pcNodeDielOne[i].nNodeGlobal = n_x + 1;
		pcNodeDielOne[i].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[i].rY = (_real)(dYAdmit);
		pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
	};

							// set Node
	ii = 0;
	r = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);

	r = pcNextBlockList->GetXmax()*cos( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX2 = (int)(r);	
	
	r = pcNextBlockList->GetXmax()*sin( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY2 = (int)(r);		
		
	nY = abs( nY1 - nY2 );
	y1 = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmin() );
	if( nY1 < nY2 ) 
	{
		i_step = 1;
	}
	else 
	{
		i_step = -1;
	};

	nX = abs( nX1 - nX2 );
	x1 = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmin() );
	if( nX1 < nX2 )
	{
		j_step = 1;
	}
	else 
	{
		j_step = -1;
	};

	if( nY < nX )
	{
		for( i = 0, j = 0; (i < nY)&&(j < nX) ; i++ )	// B-4 (old: njj = nX/(nY+1)+1 - the staircase stopped halfway)
		{
			for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
			bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			};
			for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
	}
	else
	{
		for( i = 0, j = 0; (i < nX)&&(j < nY) ; i++ )	// B-4 (old: njj = nY/(nX+1)+1)
		{
			for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
			bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
		};
			for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
	};


	r = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);

	r = pcNextBlockList->GetXmax()*cos( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX2 = (int)(r);	
	
	r = pcNextBlockList->GetXmax()*sin( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY2 = (int)(r);		

	nY = abs( nY1 - nY2 );
	y1 = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmax() );
	if( nY1 < nY2 ) 
	{
		i_step = 1;
	}
	else 
	{
		i_step = -1;
	};

	nX = abs( nX1 - nX2 );
	x1 = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmax() );
	if( nX1 < nX2 )
	{
		j_step = 1;
	}
	else 
	{
		j_step = -1;
	};

	if( nY < nX )
	{
		for( i = 0, j = 0; (i < nY)&&(j < nX) ; i++ )	// B-4 (old: njj = nX/(nY+1)+1 - the staircase stopped halfway)
		{
			for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
			bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			};
			for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
	}
	else
	{
		for( i = 0, j = 0; (i < nX)&&(j < nY) ; i++ )	// B-4 (old: njj = nY/(nX+1)+1)
		{
			for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
			bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
		};
			for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
	};

	if( k1 > 2 ) df = ( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() )/( k1 - 2 );
	else df = 0.0;
	for( i = 0, r = pcNextBlockList->GetYmin(); i < k1; i++, r += df )
	{
		x = pcNextBlockList->GetXmin()*cos(r) + pcNextBlockList->GetX0();
		y = pcNextBlockList->GetXmin()*sin(r) + pcNextBlockList->GetY0();
		bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
		if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
		if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
		if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
		if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
	};

//		CString csBuf1;
	if( k2 > 2 ) df = ( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() )/( k2 - 2 );
	else df = 0.0;
	for( i = 0, r = pcNextBlockList->GetYmin(); i < k2; i++, r += df )
	{
		x = pcNextBlockList->GetXmax()*cos(r) + pcNextBlockList->GetX0();
		y = pcNextBlockList->GetXmax()*sin(r) + pcNextBlockList->GetY0();
		bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
		if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
		if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
		if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
		if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
	};

//		CString csBuf1;
	DeleteDublicateData();
	{	// B-16: the nodes of the parts outside the region (marked -1) are dropped
		TmcIdx a, b;
		for( a = 0, b = 0; a < nNumNode; a++ )	// after DeleteDublicateData nNumNode is the number of nodes
		{
			if( pcNodeDielOne[a].nNodeGlobal < 0 ) continue;
			if( a != b ) pcNodeDielOne[b] = pcNodeDielOne[a];
			b++;
		};
		nNumNode = b;
	};
	SetAbsorberNodeType( cParam );

//		CString csBuf1;

	return cError;
}



CTmcLibError & CTmcRTHNodeDiel::SetCircMagnetic(CTmcRTH_BlockList *pcNextBlockList, CTmcRTH_IndanParam &cParam)
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	int i_step, j_step, njj, jj;
	int nX1, nX2, nY1, nY2;
	int j, k1, k2, nX, nY, n_x, n_y;
	TmcIdx i, ii;	// BIGBLOCK3
	double r, x, y, x1, y1, df, dYAdmit;
	BOOL bB16Out = FALSE;	// B-16

	dYAdmit = 0.0;
	n_x = cParam.CalcnX();	// B-2 (old: (int)((cParam.rXmax - cParam.rXmin)/cParam.rDelta + 1 ))
	n_y = cParam.CalcnY();	// B-2 (old: (int)((cParam.rYmax - cParam.rYmin)/cParam.rDelta + 1 ))

							// calculated nNumNode
	nNumNode = 0;
	r = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);

	r = pcNextBlockList->GetXmax()*cos( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX2 = (int)(r);	
	
	r = pcNextBlockList->GetXmax()*sin( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY2 = (int)(r);		
		
	nNumNode += abs( nY1 - nY2 ) + abs( nX1 - nX2 );
	
	r = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);

	r = pcNextBlockList->GetXmax()*cos( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX2 = (int)(r);	
	
	r = pcNextBlockList->GetXmax()*sin( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY2 = (int)(r);		
		
	nNumNode += abs( nY1 - nY2 ) + abs( nX1 - nX2 );
	
	k1 = 10*(int)((pcNextBlockList->GetXmin()*( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() ) )/cParam.rDelta + 1 );
	nNumNode += k1;
	
	k2 = 10*(int)((pcNextBlockList->GetXmax()*( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() ) )/cParam.rDelta + 1 );
	nNumNode += k2;
	
	nNumNode *= 4;
//		CString csBuf1;
	
	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	
							// allocation memory
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};
	for( i = 0; i < nNumNode; i++ ) 
	{
		pcNodeDielOne[i].nNodeGlobal = n_x + 1;
		pcNodeDielOne[i].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[i].rY = (_real)(dYAdmit);
		pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
	};

							// set Node
	ii = 0;
	r = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);

	r = pcNextBlockList->GetXmax()*cos( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX2 = (int)(r);	
	
	r = pcNextBlockList->GetXmax()*sin( pcNextBlockList->GetYmin() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY2 = (int)(r);		
		
	nY = abs( nY1 - nY2 );
	y1 = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmin() );
	if( nY1 < nY2 ) 
	{
		i_step = 1;
	}
	else 
	{
		i_step = -1;
	};

	nX = abs( nX1 - nX2 );
	x1 = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmin() );
	if( nX1 < nX2 )
	{
		j_step = 1;
	}
	else 
	{
		j_step = -1;
	};

	if( nY < nX )
	{
		for( i = 0, j = 0; (i < nY)&&(j < nX) ; i++ )	// B-4 (old: njj = nX/(nY+1)+1 - the staircase stopped halfway)
		{
			for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
			bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			};
			for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
	}
	else
	{
		for( i = 0, j = 0; (i < nX)&&(j < nY) ; i++ )	// B-4 (old: njj = nY/(nX+1)+1)
		{
			for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
			bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
		};
			for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
	};


	r = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);

	r = pcNextBlockList->GetXmax()*cos( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX2 = (int)(r);	
	
	r = pcNextBlockList->GetXmax()*sin( pcNextBlockList->GetYmax() ) + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;	// B-26: rounded as in the polygon (old: no + 0.5)
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY2 = (int)(r);		

	nY = abs( nY1 - nY2 );
	y1 = pcNextBlockList->GetXmin()*sin( pcNextBlockList->GetYmax() );
	if( nY1 < nY2 ) 
	{
		i_step = 1;
	}
	else 
	{
		i_step = -1;
	};

	nX = abs( nX1 - nX2 );
	x1 = pcNextBlockList->GetXmin()*cos( pcNextBlockList->GetYmax() );
	if( nX1 < nX2 )
	{
		j_step = 1;
	}
	else 
	{
		j_step = -1;
	};

	if( nY < nX )
	{
		for( i = 0, j = 0; (i < nY)&&(j < nX) ; i++ )	// B-4 (old: njj = nX/(nY+1)+1 - the staircase stopped halfway)
		{
			for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
			bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			};
			for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
	}
	else
	{
		for( i = 0, j = 0; (i < nX)&&(j < nY) ; i++ )	// B-4 (old: njj = nY/(nX+1)+1)
		{
			for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
			bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
		};
			for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++ )	// B-4 (old: jj < njj)
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
	};

	if( k1 > 2 ) df = ( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() )/( k1 - 2 );
	else df = 0.0;
	for( i = 0, r = pcNextBlockList->GetYmin(); i < k1; i++, r += df )
	{
		x = pcNextBlockList->GetXmin()*cos(r) + pcNextBlockList->GetX0();
		y = pcNextBlockList->GetXmin()*sin(r) + pcNextBlockList->GetY0();
		bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
		if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
		if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
		if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
		if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
	};

//		CString csBuf1;
	if( k2 > 2 ) df = ( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() )/( k2 - 2 );
	else df = 0.0;
	for( i = 0, r = pcNextBlockList->GetYmin(); i < k2; i++, r += df )
	{
		x = pcNextBlockList->GetXmax()*cos(r) + pcNextBlockList->GetX0();
		y = pcNextBlockList->GetXmax()*sin(r) + pcNextBlockList->GetY0();
		bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
		if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
		if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
		if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
		if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
		if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
	};

//		CString csBuf1;
	DeleteDublicateData();
	{	// B-16: the nodes of the parts outside the region (marked -1) are dropped
		TmcIdx a, b;
		for( a = 0, b = 0; a < nNumNode; a++ )	// after DeleteDublicateData nNumNode is the number of nodes
		{
			if( pcNodeDielOne[a].nNodeGlobal < 0 ) continue;
			if( a != b ) pcNodeDielOne[b] = pcNodeDielOne[a];
			b++;
		};
		nNumNode = b;
	};
	SetAbsorberNodeType( cParam );
	SetMagneticPolygonType( cParam );	// B-26: the edge of the region as for the magnetic polygon (old: not called)

//		CString csBuf1;

	return cError;
}




CTmcLibError & CTmcRTHNodeDiel::SetPolyMagnetic(CTmcRTH_BlockList *pcNextBlockList, CTmcRTH_IndanParam &cParam)
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	double *pX, *pY;
	int nXY, i_step, j_step, njj, jj;
	int nX1, nX2, nY1, nY2;
	int j, k, nX, nY, n_x, n_y;
	TmcIdx i, ii;	// BIGBLOCK3
	double r, x, y, x1, y1, dYAdmit;
	BOOL bB16Out = FALSE;	// B-16
//	CString csBuf1;
	dYAdmit = 0.0;
	n_x = cParam.CalcnX();	// B-2 (old: (int)((cParam.rXmax - cParam.rXmin)/cParam.rDelta + 1 ))
	n_y = cParam.CalcnY();	// B-2 (old: (int)((cParam.rYmax - cParam.rYmin)/cParam.rDelta + 1 ))

	if( ( pX = pcNextBlockList->GetpX() ) == NULL )
	{				
		cError.PutErrorMessage("Init pointer pX is NULL for block ");
		return cError;
	};

	if( ( pY = pcNextBlockList->GetpY() ) == NULL )
	{				
		cError.PutErrorMessage("Init pointer pY is NULL for block ");
		return cError;
	};

	if( ( nXY = pcNextBlockList->GetnXY() ) < 3 )
	{				
		cError.PutErrorMessage("Init nXY < 3 for block ");
		return cError;
	};

	r = pX[0] + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pY[0] + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);
	
							// calculated nNumNode
	for( k = 1, nNumNode = 0; k < nXY; k++ )
	{
		r = pX[k] + pcNextBlockList->GetX0();
		r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Xmin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nX2 = (int)(r);	
		
		r = pY[k] + pcNextBlockList->GetY0();
		r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Ymin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nY2 = (int)(r);		
		
		nNumNode += abs( nY1 - nY2 ) + abs( nX1 - nX2 );
		
		nX1 = nX2;
		nY1 = nY2;	
	};

	nNumNode *=4;
		

	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	
							// allocation memory
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		return cError;
	};
	
	for( i = 0; i < nNumNode; i++ ) pcNodeDielOne[i].nNodeGlobal = 1;

	r = pX[0] + pcNextBlockList->GetX0();
	r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nX1 = (int)(r);

	r = pY[0] + pcNextBlockList->GetY0();
	r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY1 = (int)(r);
	
							// set Node
	for( k = 1, ii = 0; k < nXY; k++ )
	{
		r = pX[k] + pcNextBlockList->GetX0();
		r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Xmin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nX2 = (int)(r);	
		
		r = pY[k] + pcNextBlockList->GetY0();
		r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Ymin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nY2 = (int)(r);

		nY = abs( nY1 - nY2 );
		y1 = pY[k-1];
		if( nY1 < nY2 ) 
		{
			i_step = 1;
		}
		else 
		{
			i_step = -1;
		};

		nX = abs( nX1 - nX2 );
		x1 = pX[k-1];
		if( nX1 < nX2 )
		{
			j_step = 1;
		}
		else 
		{
			j_step = -1;
		};

		if( nY < nX )
		{
			for( i = 0, j = 0; (i < nY)&&(j < nX) ; i++ )	// B-4 (old: njj = nX/(nY+1)+1 - the staircase stopped halfway)
			{
				for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++ )	// B-4 (old: jj < njj)
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
				};
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

			};
				for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++ )	// B-4 (old: jj < njj)
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

				};
		}
		else
		{
			for( i = 0, j = 0; (i < nX)&&(j < nY) ; i++ )	// B-4 (old: njj = nY/(nX+1)+1)
			{
				for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++ )	// B-4 (old: jj < njj)
				{
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

				};
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

			};
				for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++ )	// B-4 (old: jj < njj)
				{
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					bB16Out = TmcB16Out( cParam, x, y );	// B-16: before the clamp to the border
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					if( bB16Out ) pcNodeDielOne[ii].nNodeGlobal = -1;	// B-16: outside - dropped
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					
				};
		};

		r = pX[k] + pcNextBlockList->GetX0();
		r = (r - cParam.rXmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Xmin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nX1 = (int)(r);	
		
		r = pY[k] + pcNextBlockList->GetY0();
		r = (r - cParam.rYmin)/cParam.rDelta + 0.5;
		if( fabs(r) >= INT_MAX ) 
		{
			CString csBuf;
			csBuf.Format("Very big Ymin for block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			return cError;
		};
		nY1 = (int)(r);
	};

	DeleteDublicateData();
	{	// B-16: the nodes of the parts outside the region (marked -1) are dropped
		TmcIdx a, b;
		for( a = 0, b = 0; a < nNumNode; a++ )	// after DeleteDublicateData nNumNode is the number of nodes
		{
			if( pcNodeDielOne[a].nNodeGlobal < 0 ) continue;
			if( a != b ) pcNodeDielOne[b] = pcNodeDielOne[a];
			b++;
		};
		nNumNode = b;
	};
	SetAbsorberNodeType( cParam );
	SetMagneticPolygonType( cParam );
	return cError;
}



void CTmcRTHNodeDiel::SaveFileRect( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CString csCurrentPath )
{
	if( cError.IsError() ) return ;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return;
	};
	
	SeachEpsFileName( csCurrentPath );
	if( strlen(csFileNameEps) == 0 ) return;

	int nXmin, nXmax, nYmin, nYmax;
	int i, j, nX, nY, ii, jj;
	double dXmin, dXmax, dYmin, dYmax, r, x, y;
	sTmcRTHNodeDielOne pcNodeDielOne1;
	int nNumNode1;
	CString csBuf;
	FILE *fp;

	csBuf.Format("Creation of a file %s", csFileNameEps);
	PutTrace(csBuf);

	dXmin = pcNextBlockList->GetXmin();
	r = (dXmin - dXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return;
	};
	nXmin = (int)(r);

	dXmax = pcNextBlockList->GetXmax();
	r = (dXmax - dXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return;
	};
	nXmax = (int)(r);
	
	dYmin = pcNextBlockList->GetYmin();
	r = (dYmin - dYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return;
	};
	nYmin = (int)(r);

	dYmax = pcNextBlockList->GetYmax();
	r = (dYmax - dYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return;
	};
	nYmax = (int)(r);
	
	if( nXmax < nXmin )
	{
		CString csBuf;
		csBuf.Format("Xmax < Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return;
	};
	nX = nXmax - nXmin + 1;

	if( nYmax < nYmin )
	{
		CString csBuf;
		csBuf.Format("Ymax < Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return;
	};
	nY = nYmax - nYmin + 1;

	if( ((double)nX)*((double)nY) > INT_MAX )	// INT64
	{
		csBuf.Format("Block %d is too big for eps file: %.0f nodes", nBlock, ((double)nX)*((double)nY));
		cError.PutErrorMessage(csBuf);
		return;
	};
	nNumNode1 = nX*nY;
	if( nNumNode1 <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return;
	};
	
	int nOpenErr = 0;	// T-9: a busy file is waited for 20 s by the clock
	if( ( fp = TmcFopenWait( csFileNameEps, "wb", FALSE, &nOpenErr ) ) == NULL )
	{
		cError.PutErrorMessage( TmcOpenFailText( csFileNameEps, nOpenErr ) );
		return;
	};
	
	fprintf( fp, "%s%c%c%s%s%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%d%c%c%s%d%c%c%s%d%c%c%s%s%c%c%s%s%c%c", TMC_RTH_EPSFILE_ID_, 13, 10,
						TMC_RTH_EPSFILE_TYPE_, csBlock, 13, 10,
						TMC_RTH_EPSFILE_Delta_, cParam.rDelta, 13, 10,
						TMC_RTH_EPSFILE_XMIN_, dXmin, 13, 10,
						TMC_RTH_EPSFILE_YMIN_, dYmin, 13, 10,
						TMC_RTH_EPSFILE_NX_, nX, 13, 10,
						TMC_RTH_EPSFILE_NY_, nY,  13, 10,
						TMC_RTH_EPSFILE_NPoint_, nNumNode1, 13, 10,
						TMC_RTH_EPSFILE_NAccur_, sizeof(_real), 13, 10,
						TMC_RTH_EPSFILE_NdFrmt_, "NULL", 13, 10,
						TMC_RTH_EPSFILE_VlFrmt_, "NULL", 13, 10);
////			fprintf( fp, GetdFieldOutFormat(), prUNode1[ii]);
	
	csBuf.Format("%d point in file %s", nNumNode1, csFileNameEps);
	PutTrace(csBuf);

	// Записи узлов: строки снизу вверх; номер узла — локальный (j + i*nX).
	for( i = 0, jj = 0, ii = 0, y = dYmin; i < nY; i++, y+=cParam.rDelta )
	{
		for( j = 0, x = dXmin; j < nX; j++, x+=cParam.rDelta )
		{
			pcNodeDielOne1.nNodeGlobal = ii;
			pcNodeDielOne1.rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne1.rY = (_real)(1.0);
			if( i == 0 )        pcNodeDielOne1.rY *= (_real)(0.5);
			if( i == (nY - 1) ) pcNodeDielOne1.rY *= (_real)(0.5);
			if( j == 0 )        pcNodeDielOne1.rY *= (_real)(0.5);
			if( j == (nX - 1) ) pcNodeDielOne1.rY *= (_real)(0.5);
			pcNodeDielOne1.rY *= CalcEps( x, y, cParam);
			fwrite( &(pcNodeDielOne1.rY), sizeof(_real), 1, fp );
			TmcWriteIdx32( &(pcNodeDielOne1.nNodeGlobal), fp );	// INT64
			if( !pbIsStop[0] )
			{
				CString csBuf;
				csBuf.Format("User cancel calculated eps array thread in Block %d", nBlock);
				cError.PutErrorMessage(csBuf);
				fclose(fp);
				remove(csFileNameEps);
				return;
			};
			if( jj == 10000 )
			{
				csBuf.Format("Save n:%d", ii);
				PutStatistics(csBuf);
				jj = 0;
			};
			ii++;
			jj++;
		};
	};

	fclose(fp);
	csBuf.Format(" ");
	PutTrace(csBuf);
	return;
}

void CTmcRTHNodeDiel::DeleteStatDielData()
{
	
	if( nType != CTMCRTH_BLCKNTYPE_EPS ) return;
	if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTMOVE, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == 0 )
	{
		return;
	};
	if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCMOVE, strlen(CTMCRTH_INDANBLCK_CIRCMOVE) ) == 0 )
	{
		return;
	};
	if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGMOV, strlen(CTMCRTH_INDANBLCK_POLYGMOV) ) == 0 )
	{
		return;
	};

	{
		CString csBuf;
		csBuf.Format("Delete Stationary Data");
		PutStatistics(csBuf);
	};
	nNumNode = 0;
	if( pcNodeDielOne != NULL )
	{
		delete []pcNodeDielOne;
		pcNodeDielOne = NULL;
	};
	return;
}

void CTmcRTHNodeDiel::SeachEpsFileName( CString csCurrentPath )
{
	char szBuf[CTMCRTH_INDANSBUF];
	int i, n;
	
	strcpy( szBuf, csEpsExpr);
	n = strlen(szBuf);
	
	if( szBuf[0] != '[' ) return;
	
	for( i = 1; i < n; i++)
	{
		switch( szBuf[i] )
		{
			case ']':
				szBuf[i] = '\0';
				csFileNameEps.Format("%s", szBuf+1);
				csEpsExpr.Format("%s", szBuf+i+1);
				AddFileEpsExtention();
				AddCurrentPath( csCurrentPath );
				return;
				break;
			case '\0':
				{
					CString csBuf;
					csBuf.Format("missing ] or bad [ in expression {%s} for block %d", csEpsExpr, nBlock);
					cError.PutErrorMessage(csBuf);
					return;
				};
				break;
		};
	};

	{
		CString csBuf;
		csBuf.Format("missing ] or bad [ in expression {%s} for block %d", csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		return;
	};
	return;
}


void CTmcRTHNodeDiel::AddFileEpsExtention()
{
	char szBuf[CTMCRTH_INDANSBUF];
	int i, n;
	int nDot = -1;	// N-22

	strcpy( szBuf, csFileNameEps);
	n = strlen(szBuf);

	if( n == 0 ) return;

	for( i = 0; i < n; i++)
	{
		switch( szBuf[i] )
		{
			case '.':
				nDot = i;
				break;
			case '\\':
			case '/':
				nDot = -1;	// N-22: a dot in a folder name is not an extension
				break;
		};
	};
	if( nDot >= 0 ) return;

	csFileNameEps += ".";
	csFileNameEps += TMC_RTH_EPSFILE_EXT_;

	return;
}

void CTmcRTHNodeDiel::AddCurrentPath(CString csCurrentPath)
{
	CString csBuf;
	
	if( IsFullName() ) return;

	csBuf.Format("%s\\%s", csCurrentPath, csFileNameEps);
	csFileNameEps = csBuf;

	return;
}

BOOL CTmcRTHNodeDiel::IsFullName()
{
	int i, n;

	n = csFileNameEps.GetLength();
	if( n == 0 ) return TRUE;
	if( ( csFileNameEps[0] == '\\' )||( csFileNameEps[0] == '/' ) ) return TRUE;	// N-22
	for( i = 0; i < n; i++ )
	{
		if( csFileNameEps[i] == ':' ) return TRUE;
	}
	
	return FALSE;
}

CTmcLibError &CTmcRTHNodeDiel::SetRectFile( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CString csCurrentPath )
{
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

//	int nXmin, nXmax, nYmin, nYmax;
	int j, nX, nY, nPoint, nNumNode1, nAccur, jj;
	TmcIdx i, ii;	// BIGBLOCK3
	double dXmin, dXmin1, dXmax, dYmin, dYmin1, dYmax, r, x, y;
	char ch[ CTMCRTH_INDANSBUF ];
	sTmcRTHNodeDielOne *pcNodeDielOne1;
	FILE *pfData;
	CString csBufTr;

	csBufTr.Format("Block %d read file %s", nBlock, csEpsExpr);
	PutTrace(csBufTr);

	pfData = fopen( csEpsExpr, "rb" );
	if( pfData == NULL )	// N-5: as RECT_STAT [name] writes it - in the task folder and with .eps
	{
		CString csTry, csName = csEpsExpr;
		int nSep = -1, nDot = -1, k;
		for( k = 0; k < csName.GetLength(); k++ )
		{
			if( ( csName[k] == '\\' )||( csName[k] == '/' ) ) nSep = k;
			if( csName[k] == '.' ) nDot = k;
		};
		BOOL bFull = ( csName.GetLength() > 1 )&&( ( csName[1] == ':' )||( csName[0] == '\\' )||( csName[0] == '/' ) );
		if( ( !bFull )&&( csCurrentPath.GetLength() > 0 ) )
		{
			csTry.Format( "%s\\%s", (LPCSTR)csCurrentPath, (LPCSTR)csName );
			pfData = fopen( csTry, "rb" );
		};
		if( ( pfData == NULL )&&( nDot <= nSep ) )
		{
			csTry.Format( "%s.eps", (LPCSTR)csName );
			pfData = fopen( csTry, "rb" );
			if( ( pfData == NULL )&&( !bFull )&&( csCurrentPath.GetLength() > 0 ) )
			{
				csTry.Format( "%s\\%s.eps", (LPCSTR)csCurrentPath, (LPCSTR)csName );
				pfData = fopen( csTry, "rb" );
			};
		};
	};
	if( pfData == NULL )
	{
		CString csBuf;
		csBuf.Format("Can't open diel.data file {%s} for block %d", csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		return cError;
	};

	if( fgets( ch, CTMCRTH_INDANSBUF, pfData ) == NULL )
	{
		CString csBuf;
		csBuf.Format("Error when read or bad diel.data file {%s} for block %d", csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	if( strncmp( ch, TMC_RTH_EPSFILE_ID_, strlen(TMC_RTH_EPSFILE_ID_) ) != 0 )
	{
		CString csBuf;
		csBuf.Format("bad {%s} in diel.data file {%s} for block %d", TMC_RTH_EPSFILE_ID_, csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};

	if( fgets( ch, CTMCRTH_INDANSBUF, pfData ) == NULL )
	{
		CString csBuf;
		csBuf.Format("Error when read or bad diel.data file {%s} for block %d", csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	if( strncmp( ch, TMC_RTH_EPSFILE_TYPE_, strlen(TMC_RTH_EPSFILE_TYPE_) ) != 0 )
	{
		CString csBuf;
		csBuf.Format("bad {%s} in diel.data file {%s} for block %d", TMC_RTH_EPSFILE_TYPE_, csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	if( strncmp( ch + strlen(TMC_RTH_EPSFILE_TYPE_), CTMCRTH_INDANBLCK_RECTSTAT, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) != 0 )
	{
		CString csBuf;
		csBuf.Format("bad type {%s} in diel.data file {%s} for block %d", CTMCRTH_INDANBLCK_RECTSTAT, csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};

	if( fgets( ch, CTMCRTH_INDANSBUF, pfData ) == NULL )
	{
		CString csBuf;
		csBuf.Format("Error when read or bad diel.data file {%s} for block %d", csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	if( strncmp( ch, TMC_RTH_EPSFILE_Delta_, strlen(TMC_RTH_EPSFILE_Delta_) ) != 0 )
	{
		CString csBuf;
		csBuf.Format("bad {%s} in diel.data file {%s} for block %d", TMC_RTH_EPSFILE_Delta_, csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	sscanf( ch+strlen(TMC_RTH_EPSFILE_Delta_), "%lg", &r );
	if( fabs((r - cParam.rDelta)/cParam.rDelta) > (1.e-5) )
	{
		CString csBuf;
		csBuf.Format("bad {#PARAM delta = %lg} != {delta = %lg} in diel.data file {%s} for block %d", cParam.rDelta, r, (LPCSTR)csEpsExpr, nBlock);	// S-4
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};

	if( fgets( ch, CTMCRTH_INDANSBUF, pfData ) == NULL )
	{
		CString csBuf;
		csBuf.Format("Error when read or bad diel.data file {%s} for block %d", csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	if( strncmp( ch, TMC_RTH_EPSFILE_XMIN_, strlen(TMC_RTH_EPSFILE_XMIN_) ) != 0 )
	{
		CString csBuf;
		csBuf.Format("bad {%s} in diel.data file {%s} for block %d", TMC_RTH_EPSFILE_XMIN_, csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	sscanf( ch+strlen(TMC_RTH_EPSFILE_XMIN_), "%lg", &r );
	pcNextBlockList->SetXmin( r );
	dXmin1 = r;
	dXmin = r + pcNextBlockList->GetX0();
	if( dXmin < cParam.rXmin ) dXmin = cParam.rXmin;
	r = (dXmin - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};

	if( fgets( ch, CTMCRTH_INDANSBUF, pfData ) == NULL )
	{
		CString csBuf;
		csBuf.Format("Error when read or bad diel.data file {%s} for block %d", csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	if( strncmp( ch, TMC_RTH_EPSFILE_YMIN_, strlen(TMC_RTH_EPSFILE_YMIN_) ) != 0 )
	{
		CString csBuf;
		csBuf.Format("bad {%s} in diel.data file {%s} for block %d", TMC_RTH_EPSFILE_YMIN_, csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	sscanf( ch+strlen(TMC_RTH_EPSFILE_YMIN_), "%lg", &r );
	pcNextBlockList->SetYmin( r );
	dYmin1 = r;
	dYmin = r + pcNextBlockList->GetY0();
	if( dYmin < cParam.rYmin ) dYmin = cParam.rYmin;
	r = (dYmin - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};

	if( fgets( ch, CTMCRTH_INDANSBUF, pfData ) == NULL )
	{
		CString csBuf;
		csBuf.Format("Error when read or bad diel.data file {%s} for block %d", csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	if( strncmp( ch, TMC_RTH_EPSFILE_NX_, strlen(TMC_RTH_EPSFILE_NX_) ) != 0 )
	{
		CString csBuf;
		csBuf.Format("bad {%s} in diel.data file {%s} for block %d", TMC_RTH_EPSFILE_NX_, csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	sscanf( ch+strlen(TMC_RTH_EPSFILE_NX_), "%d", &nX );
	if( nX < 1 ) 
	{
		CString csBuf;
		csBuf.Format("nX = %d < 1 for block %d", nX, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	pcNextBlockList->SetXmax( dXmin1 + (nX-1)*cParam.rDelta );

	if( fgets( ch, CTMCRTH_INDANSBUF, pfData ) == NULL )
	{
		CString csBuf;
		csBuf.Format("Error when read or bad diel.data file {%s} for block %d", csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	if( strncmp( ch, TMC_RTH_EPSFILE_NY_, strlen(TMC_RTH_EPSFILE_NY_) ) != 0 )
	{
		CString csBuf;
		csBuf.Format("bad {%s} in diel.data file {%s} for block %d", TMC_RTH_EPSFILE_NY_, csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	sscanf( ch+strlen(TMC_RTH_EPSFILE_NY_), "%d", &nY );
	if( nY < 1 ) 
	{
		CString csBuf;
		csBuf.Format("nY = %d < 1 for block %d", nY, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	pcNextBlockList->SetYmax( dYmin1 + (nY-1)*cParam.rDelta );
//	CString csBuf1;

	if( fgets( ch, CTMCRTH_INDANSBUF, pfData ) == NULL )
	{
		CString csBuf;
		csBuf.Format("Error when read or bad diel.data file {%s} for block %d", csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	if( strncmp( ch, TMC_RTH_EPSFILE_NPoint_, strlen(TMC_RTH_EPSFILE_NPoint_) ) != 0 )
	{
		CString csBuf;
		csBuf.Format("bad {%s} in diel.data file {%s} for block %d", TMC_RTH_EPSFILE_NPoint_, csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	sscanf( ch+strlen(TMC_RTH_EPSFILE_NPoint_), "%d", &nPoint );
	if( nPoint < 1 ) 
	{
		CString csBuf;
		csBuf.Format("nPoint = %d < 1 for block %d", nPoint, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};

	if( fgets( ch, CTMCRTH_INDANSBUF, pfData ) == NULL )
	{
		CString csBuf;
		csBuf.Format("Error when read or bad diel.data file {%s} for block %d", csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	if( strncmp( ch, TMC_RTH_EPSFILE_NAccur_, strlen(TMC_RTH_EPSFILE_NAccur_) ) != 0 )
	{
		CString csBuf;
		csBuf.Format("bad {%s} in diel.data file {%s} for block %d", TMC_RTH_EPSFILE_NAccur_, csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	sscanf( ch+strlen(TMC_RTH_EPSFILE_NAccur_), "%d", &nAccur );
	if( (nAccur != 4)&&(nAccur != 8) ) 
	{
		CString csBuf;
		csBuf.Format("nAccuracy = %d != 4 and != 8 for block %d", nAccur, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};

	if( fgets( ch, CTMCRTH_INDANSBUF, pfData ) == NULL )
	{
		CString csBuf;
		csBuf.Format("Error when read or bad diel.data file {%s} for block %d", csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	if( fgets( ch, CTMCRTH_INDANSBUF, pfData ) == NULL )
	{
		CString csBuf;
		csBuf.Format("Error when read or bad diel.data file {%s} for block %d", csEpsExpr, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};

	dXmax = dXmin + cParam.rDelta*nX;
	if( dXmax > cParam.rXmax ) dXmax = cParam.rXmax;
	r = (dXmax - cParam.rXmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Xmax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	

	dYmax = dYmin + cParam.rDelta*nY;
	if( dYmax > cParam.rYmax ) dYmax = cParam.rYmax;
	r = (dYmax - cParam.rYmin)/cParam.rDelta + 0.5;
	if( fabs(r) >= INT_MAX ) 
	{
		CString csBuf;
		csBuf.Format("Very big Ymax for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	
	if( dXmax < dXmin )
	{
		CString csBuf;
		csBuf.Format("Xmax < Xmin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};

	if( dYmax < dYmin )
	{
		CString csBuf;
		csBuf.Format("Ymax < Ymin for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};	 

	nNumNode1 = nPoint;
	if( nNumNode1 <= 0)
	{
		CString csBuf;
		csBuf.Format("nPoint <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( pfData );
		return cError;
	};
	
	if( (pcNodeDielOne1 = new sTmcRTHNodeDielOne[nNumNode1]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		fclose( pfData );
		return cError;
	};

	csBufTr.Format("%d point in file %s", nPoint, csEpsExpr);
	PutTrace(csBufTr);
	
	// Значения файла приводятся к _real; номер узла — 4 байта.
	if( nAccur == sizeof( _real ) )
	{
		for( i = 0, jj = 0; i < nPoint; i++ )
		{
			if( fread( &(pcNodeDielOne1[i].rY), nAccur, 1, pfData ) != 1 )
			{
				CString csBuf;
				csBuf.Format("bad data array in diel.data file {%s} for block %d", (LPCSTR)csEpsExpr, nBlock);	// S-4
				cError.PutErrorMessage(csBuf);
				fclose( pfData );
				delete []pcNodeDielOne1;	// S-3
				return cError;
			};
			if( TmcReadIdx32( &(pcNodeDielOne1[i].nNodeGlobal), pfData ) != 1 )
			{
				CString csBuf;
				csBuf.Format("bad data node array in diel.data file {%s} for block %d", (LPCSTR)csEpsExpr, nBlock);	// S-4
				cError.PutErrorMessage(csBuf);
				fclose( pfData );
				delete []pcNodeDielOne1;	// S-3
				return cError;
			};
			if( !pbIsStop[0] )
			{
				CString csBuf;
				csBuf.Format("User cancel calculated eps array thread in Block %d", nBlock);
				cError.PutErrorMessage(csBuf);
				fclose( pfData );
				delete []pcNodeDielOne1;	// S-3
				return cError;
			};
			if( jj == 10000 )
			{
				csBufTr.Format("Read n:%.0f", (double)i);
				PutStatistics(csBufTr);
				jj = 0;
			};
			jj++;
		};
	}
	else
	{
		switch( nAccur )
		{
			case 4:
				{
					float fBuf;
					for( i = 0, jj = 0; i < nPoint; i++ )
					{
						if( fread( &(fBuf), nAccur, 1, pfData ) != 1 )
						{
							CString csBuf;
							csBuf.Format("bad data array in diel.data file {%s} for block %d", (LPCSTR)csEpsExpr, nBlock);	// S-4
							cError.PutErrorMessage(csBuf);
							fclose( pfData );
							delete []pcNodeDielOne1;	// S-3
							return cError;
						};
						pcNodeDielOne1[i].rY = fBuf;
						if( TmcReadIdx32( &(pcNodeDielOne1[i].nNodeGlobal), pfData ) != 1 )
						{
							CString csBuf;
							csBuf.Format("bad data node array in diel.data file {%s} for block %d", (LPCSTR)csEpsExpr, nBlock);	// S-4
							cError.PutErrorMessage(csBuf);
							fclose( pfData );
							delete []pcNodeDielOne1;	// S-3
							return cError;
						};
						if( !pbIsStop[0] )
						{
							CString csBuf;
							csBuf.Format("User cancel calculated eps array thread in Block %d", nBlock);
							cError.PutErrorMessage(csBuf);
							fclose( pfData );
							delete []pcNodeDielOne1;	// S-3
							return cError;
						};
						if( jj == 10000 )
						{
							csBufTr.Format("Read n:%.0f", (double)i);
							PutStatistics(csBufTr);
							jj = 0;
						};
						jj++;
					};
				};
				break;
			case 8:
				{
					double dBuf;
					for( i = 0, jj = 0; i < nPoint; i++ )
					{
						if( fread( &(dBuf), nAccur, 1, pfData ) != 1 )
						{
							CString csBuf;
							csBuf.Format("bad data array in diel.data file {%s} for block %d", (LPCSTR)csEpsExpr, nBlock);	// S-4
							cError.PutErrorMessage(csBuf);
							fclose( pfData );
							delete []pcNodeDielOne1;	// S-3
							return cError;
						};
						pcNodeDielOne1[i].rY = (_real)(dBuf);
						if( TmcReadIdx32( &(pcNodeDielOne1[i].nNodeGlobal), pfData ) != 1 )
						{
							CString csBuf;
							csBuf.Format("bad data node array in diel.data file {%s} for block %d", (LPCSTR)csEpsExpr, nBlock);	// S-4
							cError.PutErrorMessage(csBuf);
							fclose( pfData );
							delete []pcNodeDielOne1;	// S-3
							return cError;
						};
						if( !pbIsStop[0] )
						{
							CString csBuf;
							csBuf.Format("User cancel calculated eps array thread in Block %d", nBlock);
							cError.PutErrorMessage(csBuf);
							fclose( pfData );
							delete []pcNodeDielOne1;	// S-3
							return cError;
						};
						if( jj == 10000 )
						{
							csBufTr.Format("Read n:%.0f", (double)i);
							PutStatistics(csBufTr);
							jj = 0;
						};
						jj++;
					};
				};
				break;
			default :
				{
					CString csBuf;
					csBuf.Format("nAccuracy = %d != 4 and != 8 for block %d", nAccur, nBlock);
					cError.PutErrorMessage(csBuf);
					delete []pcNodeDielOne1;	// S-3
					fclose( pfData );
					return cError;
				};
				break;
		};
	};

	fclose( pfData );

	// Два прохода: подсчёт узлов, попавших в область, затем их размещение на сетке.
	for( i = 0, ii = 0, jj = 0; i < nPoint; i++ )
	{
		j = (int)pcNodeDielOne1[i].nNodeGlobal;	// INT64: local number from .eps file (4 bytes)
		x = dXmin1 + pcNextBlockList->GetX0() + ( j - ((int)((j)/nX))*nX )*cParam.rDelta;
		y = dYmin1 + pcNextBlockList->GetY0() + ((int)((j)/nX))*cParam.rDelta;
		if( (x <= dXmax)&&(x >= dXmin)&&(y <= dYmax)&&(y >= dYmin) ) ii++;
		if( !pbIsStop[0] )
		{
			CString csBuf;
			csBuf.Format("User cancel calculated eps array thread in Block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			delete []pcNodeDielOne1;	// S-3
			return cError;
		};
		if( jj == 10000 )
		{
			csBufTr.Format("Load n:%.0f %.0f", (double)ii, (double)i);
			PutStatistics(csBufTr);
			jj = 0;
		};
		jj++;
	};

	if( ii < 1 )
	{
		CString csBuf;
		csBuf.Format("nNumNode < 1 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		delete []pcNodeDielOne1;	// S-3
		return cError;
	}

	nNumNode = ii;
	
	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		delete []pcNodeDielOne1;	// S-3
		return cError;
	};
	
	if( pcNodeDielOne != NULL ) delete []pcNodeDielOne;
	pcNodeDielOne = NULL;

	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		CString csBuf;
		csBuf.Format("Not enought memory for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		nNumNode = 0;
		delete []pcNodeDielOne1;	// S-3
		return cError;
	};

	for( i = 0, ii = 0, jj = 0; i < nPoint; i++ )
	{
		j = (int)pcNodeDielOne1[i].nNodeGlobal;	// INT64: local number from .eps file (4 bytes)
		x = dXmin1 + pcNextBlockList->GetX0() + ( j - ((int)((j)/nX))*nX )*cParam.rDelta;
		y = dYmin1 + pcNextBlockList->GetY0() + ((int)((j)/nX))*cParam.rDelta;
		if( (x <= dXmax)&&(x >= dXmin)&&(y <= dYmax)&&(y >= dYmin) )
		{
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = pcNodeDielOne1[i].rY;
			ii++;
		};
		if( !pbIsStop[0] )
		{
			CString csBuf;
			csBuf.Format("User cancel calculated eps array thread in Block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			delete []pcNodeDielOne1;	// S-3
			return cError;
		};
		if( jj == 10000 )
		{
			csBufTr.Format("Load n:%.0f %.0f", (double)ii, (double)i);
			PutStatistics(csBufTr);
			jj = 0;
		};
		jj++;
	};
	delete []pcNodeDielOne1;	// S-3: the temporary array of the block FILE is not needed any more

	csBufTr.Format(" ");
	PutTrace(csBufTr);

	return cError;
}



double CTmcRTHNodeDiel::dAmplitude(CTmcRTH_IndanParam &cParam, double dtCurrent)
{
	double r1, f, t;
	char szBuf[CTMCRTH_INDANSBUF];
	char szBuf1[CTMCRTH_INDANSBUF];
	CString csBuf;

	r1 = 1.;

	if( strlen( csVx ) < 1 ) return r1;

	strcpy( szBuf, csVx);

	f = cParam.rFreq/cParam.rFreqUnit;
	t = dtCurrent/cParam.rTimeUnit;

	// B-11, old (6 digits):
	expr_set_per( (char *)"t", 1, (_real)t );	// B-11: the full precision, without a string
	expr_set_per( (char *)"f", 1, (_real)f );
	szBuf1[0] = '\0';
	if( i1nte_atof_1( szBuf, &r1 ) != 0 )
	{
		csBuf.Format("In{%s}%s; t=(%lg);f=(%lg);",
			szBuf, expr_get_error(), t, f );
		cError.PutErrorMessage(csBuf);
		return (double)(0.0);
	};

	return r1;
}

double CTmcRTHNodeDiel::dFaza(CTmcRTH_IndanParam &cParam, double dtCurrent)
{
	double r1, f, t;
	char szBuf[CTMCRTH_INDANSBUF];
	char szBuf1[CTMCRTH_INDANSBUF];
	CString csBuf;

	r1 = 0.;

	if( strlen( csVy ) < 1 ) return r1;

	strcpy( szBuf, csVy);

	f = cParam.rFreq/cParam.rFreqUnit;
	t = dtCurrent/cParam.rTimeUnit;

	// B-11, old (6 digits):
	expr_set_per( (char *)"t", 1, (_real)t );	// B-11: the full precision, without a string
	expr_set_per( (char *)"f", 1, (_real)f );
	szBuf1[0] = '\0';
	if( i1nte_atof_1( szBuf, &r1 ) != 0 )
	{
		csBuf.Format("In{%s}%s; t=(%lg);f=(%lg);",
			szBuf, expr_get_error(), t, f );
		cError.PutErrorMessage(csBuf);
		return (double)(0.0);
	};

	// B-5.1: degrees -> radians is a multiplication by rAngleUnit = pi/180 (old: return -r1/cParam.rAngleUnit;)
	// B-5.2: the minus is the delay - meant
	return -r1*cParam.rAngleUnit;
}

void CTmcRTHNodeDiel::CalculateYForInput(double dFreq, double dDelta)
{
	if( cError.IsError() ) return;

	dYInput = 0.;

	if( strlen( csW ) < 1 )
	{
		return;
	};

	double y_line, koef, r1;
	char szBuf[CTMCRTH_INDANSBUF];
	CString csBuf;

	koef = (dDelta*dFreq*2.*PI___/C0___);
	koef = (koef*koef/2.);
//	koef = 1.;

	strcpy( szBuf, csW);
	if( i1nte_atof_1( szBuf, &r1 ) != 0 )
	{
		csBuf.Format("In{%s}%s;",
			szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};

	y_line = r1;

	if( y_line < 0.0 )
	{
		dYInput = (double)( y_line*koef );
	}
	else
	{
		/// З-1 (автор 29.10): W в пределах 0,004 от 4 сдвигается до 3,996 / 4,004 — защита от расходимости схемы при Delta eps = 4.
		if( fabs( y_line - 4.) < 0.004 )
		{
			if( y_line < 4. )	y_line = (_ELEM_VAL_RTH)3.996;
			else y_line = (_ELEM_VAL_RTH)4.004;
		};
		dYInput = (double)( 4*y_line );
	};


//	csBuf.Format("In{%s}%lg;",
//		szBuf, dYInput );
//	AfxMessageBox(csBuf);

	return;
}

/// З-5 (автор 29.10): пороги у левого (1,5 шага) и правого (2,5 шага) краёв разные потому же, что в З-4:
/// края сдвигаются в противоположные стороны (правый край области — на запасном столбце, Б-2.2).
void CTmcRTHNodeDiel::SetMagneticPolygonType( CTmcRTH_IndanParam &cParam )
{
	TmcIdx i;	// BIGBLOCK3
//	CString csBuf;

	for( i = 0; i < nNumNode; i++ ) 
	{
		switch( pcNodeDielOne[i].nType )
		{
			case CTMCRTH_BLCKNTYPE_ABSR_0110:
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < (cParam.rXmin + 1.5*cParam.rDelta) )
				{
					pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_ABSR_0111;
//						csBuf.Format("x = %lg; set type CTMCRTH_BLCKNTYPE_ABSR_1101",
				};
				break;
			case CTMCRTH_BLCKNTYPE_ABSR_1100:
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < (cParam.rXmin + 1.5*cParam.rDelta) )
				{
					pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_ABSR_1101;
//						csBuf.Format("x = %lg; set type CTMCRTH_BLCKNTYPE_ABSR_0111",
				};
				break;
			case CTMCRTH_BLCKNTYPE_ABSR_1001:
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > (cParam.rXmax - 2.5*cParam.rDelta) )
				{
					pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_ABSR_1101;
//						csBuf.Format("x = %lg; set type CTMCRTH_BLCKNTYPE_ABSR_1101",
				};
				break;
			case CTMCRTH_BLCKNTYPE_ABSR_0011:
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > (cParam.rXmax - 2.5*cParam.rDelta) )
				{
					pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_ABSR_0111;
//						csBuf.Format("x = %lg; set type CTMCRTH_BLCKNTYPE_ABSR_0111",
				};
				break;
			default:
				break;
		};
	};
	return;
}


// =====================================================================
//  Incident wave from file. Added 2026-09-30.
//  The port takes a(t) from a text file instead of Amp*sin(wt+Faza).
//  Syntax in .tpl:  INPUT_X ; ylo; yhi; Tmin; Tmax; [name]; Faza;
//  File <name>.sig: two columns, time in TIME_UNIT and value of a.
//  Outside the file range the signal is zero; the Tmin..Tmax window of
//  the port still applies, so a longer file is cut and a shorter one is
//  padded with zeros.
// =====================================================================

void CTmcRTHNodeDiel::DeleteSignalData( void )
{
	if( pdSigT != NULL )
	{
		delete []pdSigT;
		pdSigT = NULL;
	};
	if( pdSigV != NULL )
	{
		delete []pdSigV;
		pdSigV = NULL;
	};
	nSigN = 0;
	bSigFile = FALSE;
	csFileNameSig.Format("");
	csSigFreq.Format("");
	return;
}

void CTmcRTHNodeDiel::SeachSignalFileName( void )
{
	char szBuf[CTMCRTH_INDANSBUF];
	int i, n;

	DeleteSignalData();

	if( cError.IsError() ) return;
	if( strlen( csVx ) < 1 ) return;

	strcpy( szBuf, csVx );
	n = strlen( szBuf );
	if( szBuf[0] != '[' ) return;

	for( i = 1; i < n; i++ )
	{
		if( szBuf[i] == ']' )
		{
			CString csBuf;
			szBuf[i] = '\0';
			csFileNameSig.Format("%s", szBuf+1);
			if( strchr( (const char *)csFileNameSig, '.' ) == NULL )
			{
				csBuf.Format("%s.sig", csFileNameSig);
				csFileNameSig = csBuf;
			};
			if( ( strlen( csSigPath ) > 0 )&&
				( strchr( (const char *)csFileNameSig, '\\' ) == NULL )&&
				( strchr( (const char *)csFileNameSig, ':' ) == NULL ) )
			{
				csBuf.Format("%s\\%s", csSigPath, csFileNameSig);
				csFileNameSig = csBuf;
			};
			ReadSignalFile();
			return;
		};
	};

	{
		CString csBuf;
		csBuf.Format("missing ] or bad [ in signal expression {%s} for block %d", csVx, nBlock);
		cError.PutErrorMessage(csBuf);
	};
	return;
}

void CTmcRTHNodeDiel::ReadSignalFile( void )
{
	FILE *fp;
	char szLine[CTMCRTH_INDANSBUF];
	int i, n;
	double t, v;

	if( cError.IsError() ) return;

	if( ( fp = fopen( (const char *)csFileNameSig, "r" ) ) == NULL )
	{
		CString csBuf;
		csBuf.Format("can not open signal file {%s} for block %d", csFileNameSig, nBlock);
		cError.PutErrorMessage(csBuf);
		return;
	};

	//  first pass: count the data lines and pick up the frequency list.
	//  A multi-tone signal is made by somebody who already knows its
	//  frequencies, and looking for them afterwards in the spectrum is
	//  NOT exact: neighbouring tones pull each other peaks aside. So the
	//  file may simply name them:  #FREQ 3.0 3.2 3.4 ...  (in FREQ_UNIT).
	csSigFreq.Format("");
	for( n = 0; fgets( szLine, CTMCRTH_INDANSBUF, fp ) != NULL ; )
	{
		if( strncmp( szLine, "#FREQ", 5 ) == 0 )
		{
			int nL = strlen( szLine );
			for( ; ( nL > 0 )&&( ( szLine[nL-1] == 10 )||( szLine[nL-1] == 13 ) ) ; nL-- );
			szLine[nL] = 0;
			csSigFreq.Format("%s", szLine + 5);
			continue;
		};
		if( ( szLine[0] == '!' )||( szLine[0] == '#' ) ) continue;
		if( sscanf( szLine, "%lg %lg", &t, &v ) == 2 ) n++;
	};

	if( n < 2 )
	{
		CString csBuf;
		csBuf.Format("signal file {%s} for block %d has less than 2 points", csFileNameSig, nBlock);
		cError.PutErrorMessage(csBuf);
		fclose( fp );
		return;
	};

	if( ( ( pdSigT = new double[n] ) == NULL )||( ( pdSigV = new double[n] ) == NULL ) )
	{
		CString csBuf;
		csBuf.Format("not enough memory for signal file {%s}", csFileNameSig);
		cError.PutErrorMessage(csBuf);
		DeleteSignalData();
		fclose( fp );
		return;
	};

	//  second pass: read the values
	rewind( fp );
	int nLineNo = 0;	// N-15: the number of the line in the file, not of the point
	for( i = 0; ( i < n )&&( fgets( szLine, CTMCRTH_INDANSBUF, fp ) != NULL ) ; )
	{
		nLineNo++;
		if( ( szLine[0] == '!' )||( szLine[0] == '#' ) ) continue;
		if( sscanf( szLine, "%lg %lg", &t, &v ) != 2 ) continue;
		if( ( i > 0 )&&( t <= pdSigT[i-1] ) )
		{
			CString csBuf;
			csBuf.Format("time is not increasing in signal file {%s} at line %d", csFileNameSig, nLineNo);
			cError.PutErrorMessage(csBuf);
			DeleteSignalData();
			fclose( fp );
			return;
		};
		pdSigT[i] = t;
		pdSigV[i] = v;
		i++;
	};
	fclose( fp );

	nSigN = i;
	bSigFile = TRUE;
	{	// N-15: the phase of the port is not used with a signal file - say so
		char *pEnd;
		const char *pF = csVy;
		while( ( *pF == ' ' )||( *pF == '\t' ) ) pF++;
		double dF = strtod( pF, &pEnd );
		while( ( *pEnd == ' ' )||( *pEnd == '\t' ) ) pEnd++;
		if( ( *pF != '\0' )&&( ( *pEnd != '\0' )||( dF != 0.0 ) ) )
		{
			CString csBuf;
			csBuf.Format("WARNING: block %d: the phase {%s} is not used, the signal comes from the file {%s}", nBlock, (LPCSTR)csVy, (LPCSTR)csFileNameSig);
			PutTrace( csBuf );
			ConsolePrint( "%s", (LPCSTR)csBuf );
		};
	};
	return;
}

double CTmcRTHNodeDiel::dSignalFromFile( double dtCurrent, CTmcRTH_IndanParam &cParam )
{
	int nLo, nHi, nMid;
	double t, dT;

	if( !bSigFile ) return 0.0;
	if( nSigN < 2 ) return 0.0;

	//  file keeps time in TIME_UNIT of the task
	t = dtCurrent/cParam.rTimeUnit;

	if( t < pdSigT[0] ) return 0.0;
	if( t > pdSigT[nSigN-1] ) return 0.0;

	//  binary search for the interval
	nLo = 0;
	nHi = nSigN - 1;
	while( nHi - nLo > 1 )
	{
		nMid = ( nLo + nHi )/2;
		if( pdSigT[nMid] > t ) nHi = nMid;
		else nLo = nMid;
	};

	dT = pdSigT[nHi] - pdSigT[nLo];
	if( dT <= 0.0 ) return pdSigV[nLo];

	return pdSigV[nLo] + ( pdSigV[nHi] - pdSigV[nLo] )*( t - pdSigT[nLo] )/dT;
}

BOOL CTmcRTHNodeDiel::IsInputBlock( void )
{
	return ( nType == CTMCRTH_BLCKNTYPE_INPXLEFT )||
		( nType == CTMCRTH_BLCKNTYPE_INPXRIGHT )||
		( nType == CTMCRTH_BLCKNTYPE_INPYTOP )||
		( nType == CTMCRTH_BLCKNTYPE_INPYBOT );
}
