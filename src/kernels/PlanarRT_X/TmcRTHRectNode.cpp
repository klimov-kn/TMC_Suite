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
	DeleteData();
	int i;

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
	ReadParamSection();
	ReadTopologySection();
	ReadLinkListSection();
	ReadOutputSection();

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
	DeleteData();

	ReadParamSection();
	ReadTopologySection();
	ReadLinkListSection();
	ReadOutputSection();

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
	RunStep1();
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

	DeleteOutputFile();

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

	if( ( ((double)(nX))*((double)(nY)) ) > INT_MAX )
	{
		csBuf.Format("Error : very dimension XY-size ( X*Y ) in %s section", CTMCRTH_INDANMET_PARAM);
		cError.PutErrorMessage( csBuf );
		return;
	};
	nNumNode = nX*nY;
	csBuf.Format("nNumNode:(%d);", nNumNode );
	PutStatistics(csBuf);
	if( nNumNode <= 1 )
	{
		nNumNode = 0;
		csBuf.Format("Error : very big X-size and Y-size or delta is very small in %s section", CTMCRTH_INDANMET_PARAM);
		cError.PutErrorMessage( csBuf );
		return;
	};

	cIndan.GetOutput()->GetFieldIntegrated().InitAllFile( nX, nY, dDelta, dXmin, dYmin, cParam, dT );

	cIndan.GetOutput()->GetFieldIntegrated().SetNodeStep( 6 );

	cError = cIndan.GetOutput()->GetFieldIntegrated().GetError();


	InitNodeArray();
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

	for( ; nTCurrent < nTMax;  ii+=4)
	{
		if( IsError() ) return;
		if( !bIsReadData ) 
		{
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
			ii = 0;
		};
		RunKernel1T();
	};

	time( &ltime_current );
	csBuf.Format("nT:%d;%ld;%lg", nTCurrent,
		ltime_current - ltime_start, 
		((double)ltime_current - ltime_start)/nTCurrent);
	PutStatistics(csBuf);
	
	PutSmatrix();
	
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
/* ==== END BUG12 PORTFIX (RectNode) ==== */

void CTmcRTHRectNode::RunKernel1T( void )
{
	if( IsError() ) return;
	if( !bIsReadData ) return;

//   excite and out reflection wave for inputs 
//   calculation reflection wave
	RunExciteInputs();
	RunSaveInputs();
	RunScatteringNode();
	RunRestoreInputs();
	RunBlockNode();
	PutField( 0 );
	DistributionIntegrated( 0 );
	nTCurrent++;
	dTCurrent += dT;

//   excite and out reflection wave for inputs 
//   calculation reflection wave
	RunExciteInputs1();
	RunSaveInputs();
	RunScatteringNode1();
	RunRestoreInputs();
	RunBlockNode1();
	PutField( 1 );
	DistributionIntegrated( 1 );
	nTCurrent++;
	dTCurrent += dT;

//   excite and out reflection wave for inputs 
//   calculation reflection wave
	RunExciteInputs2();
	RunSaveInputs();
	RunScatteringNode2();
	RunRestoreInputs();
	RunBlockNode2();
	PutField( 2 );
	DistributionIntegrated( 2 );
	nTCurrent++;
	dTCurrent += dT;

//   excite and out reflection wave for inputs 
//   calculation reflection wave
	RunExciteInputs3();
	RunSaveInputs();
	RunScatteringNode3();
	RunRestoreInputs();
	RunBlockNode3();
	PutField( 3 );
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

	int i, ii;
	void *pTemp;
	char *pTemp1;

	if( (prUNode1Temp	= new _ELEM_VAL_RTH[ 6*nNumNode + 32]) == NULL )
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
*/	if( (prYNodeTemp	= new _real[ 6*nNumNode + 32]) == NULL )
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

	PutStepEndSound();

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

	SetDielTopology();
	SetDielInNodeList();
	PutTopology();

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

	int i, ii, jj;
	double a, koef, koef1;
	double d_EpsAdd, d_N_ElectronConcentr, d_B_MagneticField, d_Y_Loss;
	double d_e, d_m, d_eps0;
	double d_y5, d_y6, d_y7;
	double koef_N, koef_wp2, koef_WC2;
	double d_wp2, d_WC2;

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

	for( i = 0, ii = 0, jj = 0; i < nNumNode; i++, ii+=6, jj++ )	
	{
//		y_line = prYNode[ii];
		d_EpsAdd = prYNode[ ii ];
		d_N_ElectronConcentr = prYNode[ ii + 1 ];
		d_Y_Loss = prYNode[ ii + 2 ];
		d_B_MagneticField = prYNode[ ii + 4 ];

		if( (d_EpsAdd < 0.0) || ( fabs(d_N_ElectronConcentr) > DBL_MIN ) )
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
			if( fabs( d_N_ElectronConcentr ) > DBL_MIN )
			{
				d_EpsAdd -= d_N_ElectronConcentr*koef_N;
			};
			d_N_ElectronConcentr = -d_EpsAdd/koef_N;
			d_wp2 = d_N_ElectronConcentr*koef_wp2;
			d_WC2 = d_B_MagneticField*d_B_MagneticField*koef_WC2;
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
//				prYNode[ ii + 3 ] = (_real)(2*d_y6*d_y7/(d_y6+d_y7));
				prYNode[ ii + 3 ] = (_real)(8*dT*dT*d_WC2*d_wp2/((d_WC2+d_wp2)*(4+dT*dT*(d_WC2+d_wp2))));
			};
			prYNode[ ii + 4 ] = (_real)(2*(dT*dT*(d_WC2+d_wp2)-4.)/(dT*dT*(d_WC2+d_wp2)+4.));
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
#endif
		}
		else
		{
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
				csBuf.Format("n:%d", i);
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
	ii = nx_10;
	register _real r;

	for( i = 0; i < ii; i++, pr1+=6, prY1+=6)
	{
//	{ 
//		CString csBuf1;
//		csBuf1.Format(" Node %d\n prY1[ 0 ] = %lg; prY1[ 1 ] = %lg; prY1[ 2 ] = %lg;\n prY1[ 3 ] = %lg; prY1[ 4 ] = %lg; prY1[ 5 ] = %lg;",
//			i, 
//			prY1[ 0 ], prY1[ 1 ], prY1[ 2 ],
//			prY1[ 3 ], prY1[ 4 ], prY1[ 5 ]);
////		csBuf1.Format("pcNodeDielOne");
//		AfxMessageBox(csBuf1);
//	};
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

near void CTmcRTHRectNode::RunScatteringNode1line1( _ELEM_VAL_RTH *pr1, _real *prY1, int nx_10, int nX_111)
{
	int i, ii;
	ii = nx_10;
	register _real r;

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

near void CTmcRTHRectNode::RunScatteringNode1line2( _ELEM_VAL_RTH *pr1, _real *prY1, int nx_10, int nX_111)
{
	int i, ii;
	ii = nx_10;
	register _real r;

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


near void CTmcRTHRectNode::RunScatteringNode1line3( _ELEM_VAL_RTH *pr1, _real *prY1, int nx_10, int nX_111)
{
	int i, ii;
	ii = nx_10;
	register _real r;

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



void CTmcRTHRectNode::PutField( int ii )
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

	int i, j, ii;
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
//			fprintf( fp, GetdFieldOutFormat(), prYNode1[0]);
			dBuf = prYNode[ii+1];
			if( dBuf >= 0. ) 
			{
				dwp2 = 0.0;
			}
			else 
			{
					dy_4 = prYNode[ii+4];
					dy_5 = fabs( dBuf );
					dAlf = (1.-dy_4);
					if( fabs(dAlf) < DBL_MIN ) dAlf = DBL_MIN;
					dAlf = (1.+dy_4)/dAlf;
					dwp2 = 2*sqrt(-dy_5*dAlf)/(dT2);
//					{
//						CString csBuf1;
//						csBuf1.Format( "dy_4 = %lg;\ndy_5 = %lg;\nAlf = %lg;\ndT2 = %lg;\ndwp2 = %lg;\nkoef_N = %lg;\nN = %lg;\n" ,
//							dy_4, dy_5, dAlf, dT2, dwp2, koef_N, dwp2*koef_N);
//						AfxMessageBox(csBuf1);
//					};
			};
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
//			fprintf( fp, GetdFieldOutFormat(), prYNode1[0]);
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
//			fprintf( fp, GetdFieldOutFormat(), prYNode1[0]);
			dBuf = prYNode[ii+1];
			if( dBuf >= 0. ) dBuf = 0.0;
			else 
			{
				dy_4 = prYNode[ii+4];
				dy_5 = fabs( dBuf );
				dAlf = (1.-dy_4);
				if( fabs(dAlf) < DBL_MIN ) dAlf = DBL_MIN;
				dAlf = (1.+dy_4)/dAlf;
				dwp2 = 2*sqrt(-dy_5*dAlf)/(dT2);
				dWc2 = 2*prYNode[ii+3]*(dAlf+1)*(dAlf)/(dwp2*dT2*dT2);
//				dWc2 = (4*dAlf-2*dwp2*dT2)/dT2;
				if( dWc2 < 0. ) dWc2 = DBL_MIN;
				dBuf = sqrt( dWc2 )*koef_B;
			};
//			dBuf = prYNode[ii+3];
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
	cIndan.GetOutput()->GetFieldIntegrated().Integrate( dTCurrent, nTCurrent, &(prUNode1[ii]) );
	cError = cIndan.GetOutput()->GetFieldIntegrated().GetError();
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
