// TmcRTHRectNode.cpp : implementation file
//

#include "stdafx.h"
#include "PlanRT_H.h"
#include <tmcgrviw.h>
#include "TmcRTHRectNode.h"
#include <malloc.h>
#include "Pl_iofor.h"

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
//	prUyNodeBottom = NULL;
//	prUyNodeTop  = NULL;
//	prUxNodeRight = NULL;
//	prUxNodeLeft  = NULL;
	prUNode1   = NULL;					// array voltage on unit with 1 takt 
//	prUNode2   = NULL;					// array voltage on unit with 2 takt  
	prYNode    = NULL;					// node admitance array
//	prYMulNode = NULL;					// node multiple admitance array
//	pnTypeNode = NULL;					// node type array

//	pcNodeDielList = NULL;
	nDiel = 0;
	pcNodeDiel = NULL;
	dXmin = 0.0;				// global parameter Xmin for rectangle
	dXmax = 0.0;				// global parameter Xmax for rectangle
	dYmin = 0.0;				// global parameter Ymin for rectangle
	dYmax = 0.0;				// global parameter Ymax for rectangle
	bIsRunStep1Run = FALSE;
	m_cTiming.csKernel = "PlanarRT_H (tmc_rth.exe)";	// TIMING
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
	_heapmin();

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
	RunStep1();
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
	cIndan.GetOutput()->GetFieldIntegrated().InitAllFile( nX, nY, dDelta, dXmin, dYmin, cParam, dT );
	m_cTiming.Add( TMCT_DISTRINIT, qTm );			// TIMING

	cError = cIndan.GetOutput()->GetFieldIntegrated().GetError();


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

	__int64 qTmLoop = TmcTimingNow();					// TIMING
	for( ; nTCurrent < nTMax;  ii+=2)
	{
		if( IsError() ) { m_cTiming.Add( TMCT_LOOPWALL, qTmLoop ); m_cTiming.cur.nTDone = nTCurrent; return; }	// TIMING
		if( !bIsReadData ) 
		{
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
		RunKernel1T();
	};
	m_cTiming.Add( TMCT_LOOPWALL, qTmLoop );			// TIMING
	m_cTiming.cur.nTDone = nTCurrent;					// TIMING

	time( &ltime_current );
	csBuf.Format("nT:%d;%ld;%lg", nTCurrent,
		ltime_current - ltime_start, 
		((double)ltime_current - ltime_start)/nTCurrent);
	PutStatistics(csBuf);
	
	__int64 qTmPost = TmcTimingNow();			// TIMING
	PutSmatrix();

//  the directional pattern is normally built from the menu; in a batch run
//  there is no menu, so it can be asked for with TMC_AUTO_DN=1	// BORDER
	{
		const char *pszDN = getenv( "TMC_AUTO_DN" );
		if( ( pszDN != NULL )&&( *pszDN == '1' ) ) ExportToDirectionalPatter();
	};
	m_cTiming.Add( TMCT_SMATRIX, qTmPost );		// TIMING
	
	return;
}

BOOL CTmcRTHRectNode::IsError( void )
{
	return cError.IsError();
}

void CTmcRTHRectNode::RunKernel1T( void )
{
	if( IsError() ) return;
	if( !bIsReadData ) return;
	__int64 qTm0, qTm1;							// TIMING

//   excite and out reflection wave for inputs 
//   calculation reflection wave
	qTm0 = TmcTimingNow();								// TIMING
	RunExciteInputs();
	qTm1 = TmcTimingNow(); m_cTiming.cur.q[TMCT_EXCITE] += qTm1 - qTm0;		// TIMING
	RunScatteringNode();
	qTm0 = TmcTimingNow(); m_cTiming.cur.q[TMCT_SCATTER] += qTm0 - qTm1;		// TIMING
	RunBlockNode();
	qTm1 = TmcTimingNow(); m_cTiming.cur.q[TMCT_BLOCKNODE] += qTm1 - qTm0;	// TIMING
	PutField( 0 );
	m_cTiming.cur.q[TMCT_FIELDFILE] += TmcTimingNow() - qTm1;				// TIMING
	DistributionIntegrated( 0 );
	nTCurrent++;
	dTCurrent += dT;

//   excite and out reflection wave for inputs 
//   calculation reflection wave
	qTm0 = TmcTimingNow();								// TIMING
	RunExciteInputs1();
	qTm1 = TmcTimingNow(); m_cTiming.cur.q[TMCT_EXCITE] += qTm1 - qTm0;		// TIMING
	RunScatteringNode1();
	qTm0 = TmcTimingNow(); m_cTiming.cur.q[TMCT_SCATTER] += qTm0 - qTm1;		// TIMING
	RunBlockNode1();
	qTm1 = TmcTimingNow(); m_cTiming.cur.q[TMCT_BLOCKNODE] += qTm1 - qTm0;	// TIMING
	PutField( 1 );
	m_cTiming.cur.q[TMCT_FIELDFILE] += TmcTimingNow() - qTm1;				// TIMING
	DistributionIntegrated( 1 );
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

	if( (prUNode1Temp	= new _ELEM_VAL_RTH[ 2*nNumNode + 32]) == NULL )
	{			 
		cError.PutErrorMessage("Not enough memory for space array");
		DeleteNodeArray();
		return;
	};
	pTemp  = (void *)prUNode1Temp;
	pTemp1 = (char *)prUNode1Temp;
	ii = (int)pTemp;
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
*/	if( (prYNodeTemp	= new _real[ 2*nNumNode + 32]) == NULL )
	{			 
		cError.PutErrorMessage("Not enough memory for space array");
		DeleteNodeArray();
		return;
	};
	pTemp  = (void *)prYNodeTemp;
	pTemp1 = (char *)prYNodeTemp;
	ii = (int)pTemp;
	i = 32 - (ii - (ii/32)*32);
	pTemp1 += i;
	prYNode = (_real *) pTemp1;

//	CString csBuf1;
//	csBuf1.Format("{prUNode1(%p), prYNode(%p)}", prUNode1, prYNode );
//	AfxMessageBox(csBuf1);
/*	if( (prYMulNode	= new _real[ nNumNode ]) == NULL )
	{			 
		cError.PutErrorMessage("Not enough memory for space array");
		DeleteNodeArray();
		return;
	};
*/
/*	if( (pnTypeNode	= new int[ nNumNode ]) == NULL )
	{			 
		cError.PutErrorMessage("Not enough memory for space array");
		DeleteNodeArray();
		return;
	};
*/
/*
	if( (prUyNodeBottom	= new _ELEM_VAL_RTH[ nNumNode ]) == NULL )
	{			 
		cError.PutErrorMessage("Not enough memory for space array");
		DeleteNodeArray();
		return;
	};

	if( (prUyNodeTop	= new _ELEM_VAL_RTH[ nNumNode ]) == NULL )
	{
		cError.PutErrorMessage("Not enough memory for space array");
		DeleteNodeArray();
		return;
	};

	if( (prUxNodeRight	= new _ELEM_VAL_RTH[ nNumNode ]) == NULL )
	{
		cError.PutErrorMessage("Not enough memory for space array");
		DeleteNodeArray();
		return;
	};

	if( (prUxNodeLeft	= new _ELEM_VAL_RTH[ nNumNode ]) == NULL )
	{
		cError.PutErrorMessage("Not enough memory for space array");
		DeleteNodeArray();
		return;
	};
*/
/*	if( (pcNodeDielList	= new sTmcRTH_DielNodeList *[ nNumNode ] ) == NULL ) 
	{
		cError.PutErrorMessage("Not enough memory for space array");
		DeleteNodeArray();
		return;
	};
*/
	for( i = 0, ii = 0; i < nNumNode; i++ )	
	{
//		pcNodeDielList[i] = NULL;
/*
		prUyNodeBottom[i] = (_ELEM_VAL_RTH)(0.0);
		prUyNodeTop[i]    = (_ELEM_VAL_RTH)(0.0);
		prUxNodeRight[i]  = (_ELEM_VAL_RTH)(0.0);
		prUxNodeLeft[i]   = (_ELEM_VAL_RTH)(0.0);
*/
		prUNode1[ii]   = (_ELEM_VAL_RTH)(0.0);
		prYNode[ii++]  = (_real)(0.0);
		prUNode1[ii]   = (_ELEM_VAL_RTH)(0.0);
		prYNode[ii++]  = (_real)(0.5);
//		prUNode1[ii++]   = (_ELEM_VAL_RTH)(0.0);
//		prUNode1[ii++]   = (_ELEM_VAL_RTH)(0.5);
//		prUNode2[i]   = (_ELEM_VAL_RTH)(0.0);
//		prYNode[i]    = (_real)(0.0);
//		prYMulNode[i] = (_real)(0.5);
//		pnTypeNode[i] = CTMCRTH_BLCKNTYPE_EPS;
//		prUNode2[i]   = (_ELEM_VAL_RTH)(0.0);
//		prYNode[i]    = (_real)(0.0);
//		prYMulNode[i] = (_real)(0.5);
//		prUNode1[ii]  = (_ELEM_VAL_RTH)(0.0);
//		prYNode[ii++] = (_real)(0.5);
  };

	return;
}

void CTmcRTHRectNode::DeleteNodeArray( void )
{
/*
	// array falling voltage 1 = V; 2 = ^; 3 = <; 4 = >
	if( prUNode != NULL )
	{
		delete []prUNode;
		prUNode = NULL;
	};
*/
/*	if( pcNodeDielList != NULL )
	{
		TmcIdx i;	// BIGBLOCK3
		for( i = 0; i < nNumNode; i++ )
		{
			if( pcNodeDielList[i] != NULL ) 
			{
				DeleteDielList( &(pcNodeDielList[i]) );
			};
		};	
		delete []pcNodeDielList;
		pcNodeDielList = NULL;
	};
*/	
	if( prUNode1 != NULL )
	{
		delete []prUNode1Temp;
		prUNode1 = NULL;
	};
/*
	if( prUNode2 != NULL )
	{
		delete []prUNode2;
		prUNode2 = NULL;
	};
*/
	if( prYNode != NULL )
	{
		delete []prYNodeTemp;
		prYNode = NULL;
	};
/*
	if( prYMulNode != NULL )
	{
		delete []prYMulNode;
		prYMulNode = NULL;
	};
*/
/*	if( pnTypeNode != NULL )
	{
		delete []pnTypeNode;
		pnTypeNode = NULL;
	};
*/

/*
	if( prUyNodeBottom != NULL )
	{
		delete []prUyNodeBottom;
		prUyNodeBottom = NULL;
	};

	if( prUyNodeTop != NULL )
	{
		delete []prUyNodeTop;
		prUyNodeTop = NULL;
	};
	
	if( prUxNodeRight != NULL )
	{
		delete []prUxNodeRight;
		prUxNodeRight = NULL;
	};
	
	if( prUxNodeLeft != NULL )
	{
		delete []prUxNodeLeft;
		prUxNodeLeft = NULL;
	};
*/
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

//  The tone list only exists once the blocks have been read, so it is handed
//  to the integrator here - and the accumulation arrays are laid out again,
//  because a wide-band run needs one pair of quadratures per tone.	// BORDER
	for( i = 0; i < nDiel; i++ )
	{
		CString csFL = pcNodeDiel[i].GetSigFreqList();
		if( csFL.GetLength() < 1 ) continue;
		cIndan.GetOutput()->GetFieldIntegrated().SetFreqList( csFL, cParam.rFreqUnit );
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

//#pragma optimize( "f", on )

void CTmcRTHRectNode::RunScatteringNode( void )
{
	if( IsError() ) return;

	CString csBuf1;
	int i, ny1, nx1, nx14, nx148, nx11_10;
	_ELEM_VAL_RTH *pr1;
	_real *prY1;
//	int *pnType1;

	ny1   = nY - 2;
	nx1   = nX - 2;
	nx14  = nX*2;
	nx148 = nX*2;
	nx11_10 = nx1;
	for(i = 0, 
			pr1 = prUNode1+2+nx14, 
//			pr2 = prUNode2+1+nX, 
//			prY2 = prYMulNode+1+nX, 
//			pnType1 = pnTypeNode+1+nX;
			prY1 = prYNode+2+nx14;
		i < ny1;
		i++,
			pr1     += nx14,
//			pr2     += nX,
//			prY2    += nX,
//			pnType1 += nX)
			prY1    += nx14)
	{
		RunScatteringNode1line( pr1, prY1, nx11_10, nx148);
	};


	return;
}

void CTmcRTHRectNode::RunScatteringNode1( void )
{
	if( IsError() ) return;

	CString csBuf1;
	int i, ny1, nx1, nx14, nx148, nx11_10;
	_ELEM_VAL_RTH *pr1;
	_real *prY1;
//	int *pnType1;

	ny1   = nY - 2;
	nx1   = nX - 2;
	nx14  = nX*2;
	nx148 = nX*2;
	nx11_10 = nx1;
	for(i = 0, 
			pr1 = prUNode1+2+nx14, 
//			pr2 = prUNode2+1+nX, 
//			prY2 = prYMulNode+1+nX, 
//			pnType1 = pnTypeNode+1+nX;
			prY1 = prYNode+2+nx14;
		i < ny1;
		i++,
			pr1     += nx14,
//			pr2     += nX,
//			prY2    += nX,
//			pnType1 += nX)
			prY1    += nx14)
	{
		RunScatteringNode1line1( pr1, prY1, nx11_10, nx148);
	};


	return;
}

//#pragma optimize( "f", off )

void CTmcRTHRectNode::DeleteDielList( sTmcRTH_DielNodeList ** pcNodeDielList1 )
{
	if( pcNodeDielList1 == NULL ) return;
	if( pcNodeDielList1[ 0 ]->pcDielNodeNextList != NULL )
	{
		DeleteDielList( &(pcNodeDielList1[0]->pcDielNodeNextList) );
	};
	delete pcNodeDielList1[0];
	pcNodeDielList1[0] = NULL;

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
	double y_line, a, koef, koef1;

	koef = (cParam.rDelta*cParam.rFreq*2.*PI___/C0___);
	koef = (koef*koef/2.);
	koef1 = (cParam.rFreq*2.*PI___*dT/2.);
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
//		koef1 = koef1/tan( koef1 );found
		koef1 = 1.;
	};

	{
		CString csBuf;
		csBuf.Format("Set node admitance");
		PutTrace(csBuf);
	};
//	{ 
//		CString csBuf1;
//		csBuf1.Format("koef1 = %lg", koef1 );
////		csBuf1.Format("pcNodeDielOne");
//		AfxMessageBox(csBuf1);
//	};

	for( i = 0, ii = 0, jj = 0; i < nNumNode; i++, ii+=2, jj++ )	
	{
		y_line = prYNode[ii];

		if( y_line < 0.0 )
		{
#ifdef _VERSION_EDUC___
			{
				CString csBuf1;
				csBuf1.Format("Error for this version: Educational there is no opportunity to analyze areas with values permittivity by smaller value permittivity for free space. This opportunity is in the version: Professional. For reception of this version you can address in Tamic_soft group under a management Boris Vasilievich Sestroretsky." );
				AfxMessageBox(csBuf1);
				cError.PutErrorMessage(csBuf1);
				return;
			};
#else
			prYNode[ii] = (_real)( y_line*koef/(koef1) );
			a = (4. + fabs( prYNode[ii] ));
			prYNode[ ii + 1 ] =  (_real)(2./a);
#endif
		}
		else
		{
			if( fabs( y_line - 4.) < 0.004 )
			{
				if( y_line < 4. )	y_line = (_ELEM_VAL_RTH)3.996;
				else y_line = (_ELEM_VAL_RTH)4.004;
			};
			prYNode[ii] = (_real)( 4*y_line/koef1 );
			a = (4. + fabs( prYNode[ii] ));
			prYNode[ ii + 1 ] =  (_real)(2./a);
//			if( prYNode[ii] > (FLT_MIN + 2) )
//	{
//		CString csBuf1;
//		csBuf1.Format("eps[%d] = %lg", ii, y_line );
////		csBuf1.Format("pcNodeDielOne");
//		AfxMessageBox(csBuf1);
//	};
		};
		if( jj == 100000 )
		{
			{
				CString csBuf;
				csBuf.Format("n:%.0f", (double)i);	// INT64
				PutStatistics(csBuf);
			};
			jj = 0;
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
	remove(cOut->csFileNameField);
	
	if( cOut->csFileNameTopology.GetLength() == 0 )
	{
		cError.PutErrorMessage("Output field file name is NULL");
		return;
	};
	remove(cOut->csFileNameTopology);

	FILE *fp;
	if( (fp = fopen( cOut->csFileName, "w")) == NULL )
	{
		cError.PutErrorMessage("Can't open output file for write data");
		return;
	};
	fprintf( fp, "%s\n!File name %s\n%s%lg\n%s%d\n!nT\tnBlock\tUp\tUo",
		TMC_GROTS_FILEOUT_ID, 
		cOut->csFileName,
		TMC_GROTS_FLOUTFreq_ID, cParam.rFreq,
		TMC_GROTS_FLOUTInpNum_ID, -2);
	fclose( fp );

//	if( ( fp = fopen( cOut.csFileNameField , "w") ) == NULL ) return;
//	fclose( fp );

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

	for( ; ( fp = fopen( cOut->csFileName , "a") ) == NULL;  ) 
	{
		Sleep( 10 );
	};
	


	fprintf( fp, "\n");
	psForm = GetnTFormat();
	fprintf( fp, psForm, nTCurrent);
	psForm = GetdTcurrentFormat();
	fprintf( fp, psForm, dTCurrent);

	fclose( fp );
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
/*
near void CTmcRTHRectNode::RunScatteringNode1line1( _ELEM_VAL_RTH *pr1, _ELEM_VAL_RTH *pr2, float *prY, int *pnType1, int nx1, int nX11)
{
	int i;
	_ELEM_VAL_RTH ddd, *pr3, *pr4;

	for( i = 0, pr3 = pr2 - nX11, pr4 = pr2 + nX11; i < nx1; i++, pr3++, pr4++ )
	{
//		pr1[0] = 2/(4+prY[0])*( pr1[-1] + pr1[1] + pr1[-nX11] + pr1[nX11] + prY[0]*pr1[0] ) - pr1[0];
		pr1[i] = prY[i]*(pr2[i-1] + pr2[i+1] + pr3[0] + pr4[0] + prY[i]*pr2[i]) - pr1[i];
	};

//	for( i = 0; i < nx1; i++ )
//	{
//		pr1[0] = 2/(4+prY[0])*( pr1[-1] + pr1[1] + pr1[-nX11] + pr1[nX11] + prY[0]*pr1[0] ) - pr1[0];
//		pr1[i] += pr2[i-1];
//	};
}

near void CTmcRTHRectNode::RunScatteringNode1line( _ELEM_VAL_RTH *pr1, _ELEM_VAL_RTH *pr2, float *prY, int *pnType1, int nx1, int nX11)
{
	RunScatteringNode1line1( pr1, pr2, prY, pnType1, nx1, nX11);
//									{
//	csBuf1.Format("{nType(%p)=%d}", pnType1, pnType1[0] );
//	AfxMessageBox(csBuf1);
//									};
		switch( pnType1[i] )
		{
											// if node is metal
			case CTMCRTH_BLCKNTYPE_METAL :
//								rrr = 0.0;
								pr1[0] = 0.0;
//								pr1[1] = 0.0;
								break;
											// if node is absorber
			case CTMCRTH_BLCKNTYPE_ABSORBER :
								break;
//								{
//csBuf1.Format("{r1=%lg;r2=%lg;r3=%lg;r4=%lg}", r1[0], r2[0], r3[0], r4[0] );
//AfxMessageBox(csBuf1);
//								};
											// if dielectric
//			case CTMCRTH_BLCKNTYPE_EPSLE0   : ;
//			case CTMCRTH_BLCKNTYPE_EPSGT0   : ;
			case CTMCRTH_BLCKNTYPE_EPS      : ;
//								rrr = pr1[3]*( pr1[-4+1] + pr1[4+1] + pr1[-nX1+1] + pr1[nX1+1] + pr1[2]*pr1[1] ) - pr1[0];
								pr1[0] = pr1[3]*( pr1[-4+1] + pr1[4+1] + pr1[-nX1+1] + pr1[nX1+1] + pr1[2]*pr1[1] ) - pr1[0];
								break;
											// if one dielectric
			default:
				cError.PutErrorMessage("Kernel: unknow node type %d", pnType1[0]);
				return;
				break;
		};
		pnType1++;
//		pr1[0] = rrr;
		pr1+=4;
		j++;

//		pr1[i] = 2./(4.+prY[i])*( pr2[i-1] + pr2[i+1] + pr2[i-nX11] + pr2[i+nX11] + prY[i]*pr2[i] ) - pr1[i];
//		ddd = 2./(4.+prY[i])*( pr2[i-1] + pr2[i+1]);
}
//		pr1[ 0] = pr1[ 3]*(pr1[-3]+pr1[ 5]+pr1[ 1-nX_111]+pr1[ 1+nX_111]+pr1[ 1]*pr1[ 2])-pr1[ 0];
//		pr1[ 4] = pr1[ 7]*(pr1[ 1]+pr1[ 9]+pr1[ 5-nX_111]+pr1[ 5+nX_111]+pr1[ 5]*pr1[ 6])-pr1[ 4];
//		pr1[ 8] = pr1[11]*(pr1[ 5]+pr1[13]+pr1[ 9-nX_111]+pr1[ 9+nX_111]+pr1[ 9]*pr1[10])-pr1[ 8];
//		pr1[12] = pr1[15]*(pr1[ 9]+pr1[17]+pr1[13-nX_111]+pr1[13+nX_111]+pr1[13]*pr1[14])-pr1[12];
//		pr1[16] = pr1[19]*(pr1[13]+pr1[21]+pr1[17-nX_111]+pr1[17+nX_111]+pr1[17]*pr1[18])-pr1[16];
//		pr1[20] = pr1[23]*(pr1[17]+pr1[25]+pr1[21-nX_111]+pr1[21+nX_111]+pr1[21]*pr1[22])-pr1[20];
//		pr1[24] = pr1[27]*(pr1[21]+pr1[29]+pr1[25-nX_111]+pr1[25+nX_111]+pr1[25]*pr1[26])-pr1[24];
//		pr1[28] = pr1[31]*(pr1[25]+pr1[33]+pr1[29-nX_111]+pr1[29+nX_111]+pr1[29]*pr1[30])-pr1[28];
//		pr1[32] = pr1[35]*(pr1[29]+pr1[37]+pr1[33-nX_111]+pr1[33+nX_111]+pr1[33]*pr1[34])-pr1[32];
//		pr1[36] = pr1[39]*(pr1[33]+pr1[41]+pr1[37-nX_111]+pr1[37+nX_111]+pr1[37]*pr1[38])-pr1[36];
//		
//		pr1[1] = prY2[1]*(pr2[0]+ pr2[2]+pr2[1-nX_111]+pr2[1+nX_111]+pr2[1]*prY1[1])-pr1[1];
//		pr1[2] = prY2[2]*(pr2[1]+ pr2[3]+pr2[2-nX_111]+pr2[2+nX_111]+pr2[2]*prY1[2])-pr1[2];
//		pr1[3] = prY2[3]*(pr2[2]+ pr2[4]+pr2[3-nX_111]+pr2[3+nX_111]+pr2[3]*prY1[3])-pr1[3];
//		pr1[4] = prY2[4]*(pr2[3]+ pr2[5]+pr2[4-nX_111]+pr2[4+nX_111]+pr2[4]*prY1[4])-pr1[4];
//		pr1[5] = prY2[5]*(pr2[4]+ pr2[6]+pr2[5-nX_111]+pr2[5+nX_111]+pr2[5]*prY1[5])-pr1[5];
//		pr1[6] = prY2[6]*(pr2[5]+ pr2[7]+pr2[6-nX_111]+pr2[6+nX_111]+pr2[6]*prY1[6])-pr1[6];
//		pr1[7] = prY2[7]*(pr2[6]+ pr2[8]+pr2[7-nX_111]+pr2[7+nX_111]+pr2[7]*prY1[7])-pr1[7];
//		pr1[8] = prY2[8]*(pr2[7]+ pr2[9]+pr2[8-nX_111]+pr2[8+nX_111]+pr2[8]*prY1[8])-pr1[8];
//		pr1[9] = prY2[9]*(pr2[8]+pr2[10]+pr2[9-nX_111]+pr2[9+nX_111]+pr2[9]*prY1[9])-pr1[9];
//
//		pr1[ 4] = prY1[ 5]*(pr1[-2+ 5]+ pr1[2+ 5]+pr1[-nX_111+ 5]+pr1[nX_111+ 5]+pr1[ 5]*prY1[ 4])-pr1[ 4];
//		pr1[ 6] = prY1[ 7]*(pr1[-2+ 7]+ pr1[2+ 7]+pr1[-nX_111+ 7]+pr1[nX_111+ 7]+pr1[ 7]*prY1[ 6])-pr1[ 6];
//		pr1[ 8] = prY1[ 9]*(pr1[-2+ 9]+ pr1[2+ 9]+pr1[-nX_111+ 9]+pr1[nX_111+ 9]+pr1[ 9]*prY1[ 8])-pr1[ 8];
//		pr1[10] = prY1[11]*(pr1[-2+11]+ pr1[2+11]+pr1[-nX_111+11]+pr1[nX_111+11]+pr1[11]*prY1[10])-pr1[10];
//		pr1[12] = prY1[13]*(pr1[-2+13]+ pr1[2+13]+pr1[-nX_111+13]+pr1[nX_111+13]+pr1[13]*prY1[12])-pr1[12];
//		pr1[14] = prY1[15]*(pr1[-2+15]+ pr1[2+15]+pr1[-nX_111+15]+pr1[nX_111+15]+pr1[15]*prY1[14])-pr1[14];

*/

near void CTmcRTHRectNode::RunScatteringNode1line( _ELEM_VAL_RTH *pr1, _real *prY1, int nx_10, int nX_111)
{
	int i, ii;
	ii = nx_10/2;

	for( i = 0; i < ii; i++, pr1+=4, prY1+=4)
	{

		pr1[ 0] = prY1[ 1]*(pr1[-2+ 1]+ pr1[2+ 1]+pr1[-nX_111+ 1]+pr1[nX_111+ 1]+pr1[ 1]*prY1[ 0])-pr1[ 0];
		pr1[ 2] = prY1[ 3]*(pr1[-2+ 3]+ pr1[2+ 3]+pr1[-nX_111+ 3]+pr1[nX_111+ 3]+pr1[ 3]*prY1[ 2])-pr1[ 2];
	};
	if( (ii*2-nx_10) != 0 )
		pr1[ 0] = prY1[ 1]*(pr1[-2+ 1]+ pr1[2+ 1]+pr1[-nX_111+ 1]+pr1[nX_111+ 1]+pr1[ 1]*prY1[ 0])-pr1[ 0];
	
	return;
}

near void CTmcRTHRectNode::RunScatteringNode1line1( _ELEM_VAL_RTH *pr1, _real *prY1, int nx_10, int nX_111)
{
	int i, ii;
	
	ii = nx_10/2;

	for( i = 0; i < ii; i++, pr1+=4, prY1+=4)
	{
		pr1[1] = prY1[1]*(  pr1[-2]+   pr1[2]+ pr1[-nX_111]+  pr1[nX_111]+pr1[0]*prY1[0])-pr1[1];
		pr1[3] = prY1[3]*(pr1[-2+2]+ pr1[2+2]+pr1[2-nX_111]+pr1[2+nX_111]+pr1[2]*prY1[2])-pr1[3];
	};
	if( (ii*2-nx_10) != 0 )
		pr1[1] = prY1[1]*(pr1[-2]+ pr1[2]+pr1[-nX_111]+pr1[nX_111]+pr1[0]*prY1[0])-pr1[1];
	
	return;
}

void CTmcRTHRectNode::PutField( TmcIdx ii )	// INT64
{
	int i, j;
	FILE *fp;
	CTmcRTH_IndanOutput* cOut;
	
	cOut = cIndan.GetOutput();
	
	if( !cOut->bOutField ) return;

	if( cOut->bOutFieldSinchronization )
	{
		for( i = 0; (IsFieldFileRead())&&(cOut->bOutFieldSinchronization)&&(cOut->bOutField)&&(bIsReadData)&&( i < 500 ) ; i++)	
		{
			Sleep(50);
			cOut = cIndan.GetOutput();
//			if( i == 50) return;
		};
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
	for( i = 0; i < nY; i++)
	{
		fprintf( fp, "%c%c", 13, 10);
		for( j = 0; j < nX; j++ )
		{
//			fprintf( fp, GetdFieldOutFormat(), prUNode1[ii]);
			if( strcmp( GetdFieldOutFormat(), "") == 0 )
			{
				fwrite( &(prUNode1[ii]), sizeof(_real), 1, fp );
			}
			else
			{
				fprintf( fp, GetdFieldOutFormat(), prUNode1[ii]);
			};
			ii+=2;
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
		pcNodeDiel[i].RunBlockNode( prUNode1, prYNode, nX, 2*nNumNode );
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
		pcNodeDiel[i].RunBlockNode1( prUNode1, prYNode, nX, 2*nNumNode );
		cError = pcNodeDiel[i].GetError();
		if( IsError() ) return;
	}

	return;
}

void CTmcRTHRectNode::PutStepEndSound()
{
	if( !((cIndan.GetOutput())->bSoundEffect) ) return;
	int n;
	n = cIndan.GetnStep();
	if( n == cIndan.GetNStepMax() )
	{
		BeepAllEnd();
		if( IsAutoRun() )
		{
			CMainFrame* HwndMainFrame = GetMainFramePointer__();
			HwndMainFrame->PostMessage( WM_CLOSE );
	
		};
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
	
	cOut = cIndan.GetOutput();
	
	if( !cOut->bOutTopology ) return;
	
	GetTopology().GetBlockList()->Save(csBuf);
	koef = (cParam.rDelta*cParam.rFreq*2.*PI___/C0___);
	koef = (koef*koef/2.);

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
	for( i = 0, ii = 0; i < nY; i++)
	{
		fprintf( fp, "%c%c", 13, 10);
		for( j = 0; j < nX; j++ )
		{
//			fprintf( fp, GetdFieldOutFormat(), prYNode1[0]);
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
			ii+=2;
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
	if( GetFileAttributes( cOut->csFileNameField ) == 0xFFFFFFFF ) return FALSE;
	if( GetFileAttributes( cOut->csFileNameField ) != FILE_ATTRIBUTE_ARCHIVE ) return TRUE;
	return FALSE;
}

void CTmcRTHRectNode::PutSmatrix()
{
	cIndan.GetOutput()->PutSmatrix();
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
	cError = cIndan.GetOutput()->GetFieldIntegrated().GetError();
	m_cTiming.Add( bTmFinal ? TMCT_DISTRFINAL : TMCT_DISTRINTEGR, qTm );	// TIMING
	return;
}

void CTmcRTHRectNode::ExportToDirectionalPatter()
{
	cIndan.GetOutput()->GetFieldIntegrated().ExportToDirectionalPattern(cParam);
	cError = cIndan.GetOutput()->GetFieldIntegrated().GetError();
	if( cError.IsError() )
	{
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
	for( ; ( fp = fopen( cOut->csFileName, "a" ) ) == NULL ; )
	{
		Sleep( 10 );
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
	fclose( fp );
	return;
}
