// TmcRTH_BolckList.cpp : implementation file
//

#include "stdafx.h"
#include "PlanRT_H.h"
#include <tmcgrviw.h>
#include <TmcRTH_BolckList.h>
#include "TmcRTH_Indan.h"
#include "TmcRTHNodeDiel.h"
#include <expr.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_BolckList

CTmcRTH_BlockList::CTmcRTH_BlockList()
{
	nMemory = 0;
	pcNextBlockList = NULL;
	nBlock = 0;
	dX0 = (_real)(0.0);
	dY0 = (_real)(0.0);
	dXmin = (_real)(0.0);
	dXmax = (_real)(0.0);
	dYmin = (_real)(0.0);
	dYmax = (_real)(0.0);
	csBlock.Format("");
	csEpsExpr.Format("");
	csVx.Format("");
	csVy.Format("");
	csW.Format("");
	nType = CTMCRTH_BLCKNTYPE_METAL;
	dX = NULL;		  
	dY = NULL;		  
	nXY = 0;		  
	return;
}

CTmcRTH_BlockList::~CTmcRTH_BlockList()
{
	DeleteData();
	return;
}


BEGIN_MESSAGE_MAP(CTmcRTH_BlockList, CWnd)
	//{{AFX_MSG_MAP(CTmcRTH_BolckList)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_BolckList message handlers

void CTmcRTH_BlockList::DeleteData( void )
{
	if( pcNextBlockList != NULL ) 
	{
		pcNextBlockList->DeleteData();
		delete []pcNextBlockList;
	};
	pcNextBlockList = NULL;
	if( dX != NULL ) 
	{
		delete []dX;
	};
	dX = NULL;		  
	if( dY != NULL ) 
	{
		delete []dY;
	};
	nMemory = 0;
	dY = NULL;		  
	nXY = 0;		  
	nBlock = 0;
	dX0 = (_real)(0.0);
	dY0 = (_real)(0.0);
	dXmin = (_real)(0.0);
	dXmax = (_real)(0.0);
	dYmin = (_real)(0.0);
	dYmax = (_real)(0.0);
	csBlock.Format("");
	csEpsExpr.Format("");
	csVx.Format("");
	csVy.Format("");
	csW.Format("");
	nType = CTMCRTH_BLCKNTYPE_METAL;
	return;
}

void CTmcRTH_BlockList::Add( CString &csCh, CTmcLibError &cError, CTmcRTH_IndanParam * pcParam )
{
	int i, i1, n, ii;
	CString csBuf;
	char *szBuf = NULL;
	
	szBuf = new char[ csCh.GetLength()+3 ];
	
	if( szBuf == NULL )
	{
		csBuf.Format("Is not enough memory for %s at {%s};", CTMCRTH_INDANMET_BLOCK, csCh);
		cError.PutErrorMessage( csBuf );
		return;
	};
	
	if( cError.IsError() ) 
	{
		if( szBuf != NULL )
		{
			delete []szBuf;
			szBuf = NULL;
		};
		return;
	};

	if( nBlock != 0 )
	{
		if( pcNextBlockList == NULL )
		{
			cError.PutErrorMessage( "BlockList add next" );
			if( szBuf != NULL )
			{
				delete []szBuf;
				szBuf = NULL;
			};
			return;
		};
		if( szBuf != NULL )
		{
			delete []szBuf;
			szBuf = NULL;
		};
		pcNextBlockList->Add( csCh, cError, pcParam );
		return;
	};


	n = csCh.GetLength();

//	if( n >= (10*CTMCRTH_INDAN_LINELEN) )
//	{
//		csBuf.Format("Very long string in %s at {%s};", CTMCRTH_INDANMET_BLOCK, csCh);
//		cError.PutErrorMessage( csBuf );
//		return;
//	}

	for( i = strlen(CTMCRTH_INDANMET_BLOCK), i1 = 0, ii = 0; i < n; i++ )
	{
		switch( csCh[i] )
		{
			case '\n': 
				if( ii == 2 )
				{
					szBuf[i1] = csCh[i];
					i1++;
				};
				if( ii == 1 )
				{
					ii = 2;
					i1 = 0;
				};
			case '\0': ;
				if( ii == 0 )
				{
					csBuf.Format("Missing separator ';' at {%s}, must be %s n;", csCh, CTMCRTH_INDANMET_BLOCK);
					cError.PutErrorMessage( csBuf );
					if( szBuf != NULL )
					{
						delete []szBuf;
						szBuf = NULL;
					};
					return;
				};
				break;
			case ';': ;
				if( ii == 0 )
				{
					ii = 1;
					szBuf[i1] = '\0';
					nBlock = atoi( szBuf );
					if( nBlock < 1 )
					{
						csBuf.Format("%s number at {%s %d;}, must be >= 1", CTMCRTH_INDANMET_BLOCK, CTMCRTH_INDANMET_BLOCK, nBlock);
						cError.PutErrorMessage( csBuf );
						nBlock = 0;
						if( szBuf != NULL )
						{
							delete []szBuf;
							szBuf = NULL;
						};
						return;
					};
					i1 = 0;
					nBlock = -nBlock;
				}
				else
				{
					szBuf[i1] = csCh[i];
					i1++;
				};
				break;
			default: 
					szBuf[i1] = csCh[i];
					i1++;
				break;
		};
	}

	if( ii == 0 )
	{
		csBuf.Format("Missing separator ';' at {%s}, must be %s n;", csCh, CTMCRTH_INDANMET_BLOCK);
		cError.PutErrorMessage( csBuf );
		if( szBuf != NULL )
		{
			delete []szBuf;
			szBuf = NULL;
		};
		return;
	};

	szBuf[i1] = '\0';
	csBlock.Format("%s", szBuf);
	if( csBlock.GetLength() == 0 )
	{
		csBuf.Format("Missing string at %s %d ", CTMCRTH_INDANMET_BLOCK, -nBlock);
		cError.PutErrorMessage( csBuf );
		nBlock = 0;
		if( szBuf != NULL )
		{
			delete []szBuf;
			szBuf = NULL;
		};
		return;
	};

	if( pcNextBlockList != NULL ) delete []pcNextBlockList;
	if( (pcNextBlockList = new CTmcRTH_BlockList[1] )  == NULL )
	{
		csBuf.Format("Memory allocation at %s %d list", CTMCRTH_INDANMET_BLOCK, -nBlock);
		cError.PutErrorMessage( csBuf );
		nBlock = 0;
		if( szBuf != NULL )
		{
			delete []szBuf;
			szBuf = NULL;
		};
		return;
	};
	
	if( szBuf != NULL )
	{
		delete []szBuf;
		szBuf = NULL;
	};
	Read( cError, pcParam );


/*
	n = csCh.GetLength();
	if( n >= (CTMCRTH_INDAN_LINELEN) )
	{
		csBuf.Format("YYYY  Very long string in %s at {%s};", CTMCRTH_INDANMET_BLOCK, csCh);
		cError.PutErrorMessage( csBuf );
		return;
	}
*/

	return;
}

void CTmcRTH_BlockList::IsCorrectly( CTmcLibError &cError )
{
	if( cError.IsError() ) return;
	if( pcNextBlockList == NULL ) return;
	if( pcNextBlockList->FindBlock( nBlock ) != NULL )
	{
		CString csBuf;
		csBuf.Format("Dublicated %s %d in %s section", CTMCRTH_INDANMET_BLOCK, -nBlock, CTMCRTH_INDANMET_TOPOLOGY);
		cError.PutErrorMessage( csBuf );
		return;
	};
	if( cError.IsError() ) return;
	pcNextBlockList->IsCorrectly( cError );
	return;
}

void CTmcRTH_BlockList::IsCorrectlyLink( CTmcLibError &cError )
{
	if( cError.IsError() ) return;
	if( nBlock < 0 )
	{
		CString csBuf;
		csBuf.Format("%s %d is absent in %s section", CTMCRTH_INDANMET_BLOCK, -nBlock, CTMCRTH_INDANMET_LINKLIST);
		cError.PutErrorMessage( csBuf );
		return;
	};
	if( pcNextBlockList == NULL ) return;
	pcNextBlockList->IsCorrectlyLink( cError );
	return;
}

CTmcRTH_BlockList * CTmcRTH_BlockList::FindBlock( int nBlock1 )
{
	if( pcNextBlockList == NULL ) return NULL;
	if( nBlock1 == nBlock ) return this;
	if( pcNextBlockList->GetnBlock() == nBlock1 ) return pcNextBlockList;
	return pcNextBlockList->FindBlock( nBlock1 );
}

void CTmcRTH_BlockList::SetLink( int iBuf, _real x0, _real y0, CTmcLibError &cError )
{
	if( cError.IsError() ) return;
	
	CTmcRTH_BlockList *pcBl;
	CString csBuf;
	
	if( iBuf < 1 )
	{
		csBuf.Format("%s %d i in %s section must >= 1", CTMCRTH_INDANMET_BLOCK, iBuf, CTMCRTH_INDANMET_LINKLIST);
		cError.PutErrorMessage( csBuf );
		return;
	};
	if( ((pcBl = FindBlock( -iBuf )) == NULL)&&((pcBl = FindBlock( iBuf )) == NULL) )
	{
		csBuf.Format("%s %d is absent in %s section", CTMCRTH_INDANMET_BLOCK, iBuf, CTMCRTH_INDANMET_TOPOLOGY);
		cError.PutErrorMessage( csBuf );
		return;
	};
	if( ((pcBl = FindBlock( -iBuf )) == NULL) )
	{
		csBuf.Format("dublicated %s %d i in %s section", CTMCRTH_INDANMET_BLOCK, iBuf, CTMCRTH_INDANMET_LINKLIST);
		cError.PutErrorMessage( csBuf );
		return;
	};
	pcBl->SetnBlock(iBuf);
	pcBl->SetX0(x0);
	pcBl->SetY0(y0);
	
	return;
}

void CTmcRTH_BlockList::SetnBlock( int iBuf )
{
	nBlock = iBuf;
	return;
}

void CTmcRTH_BlockList::SetX0( _real rX )
{
	dX0 = rX;
	return;
}

void CTmcRTH_BlockList::SetY0( _real rY )
{
	dY0 = rY;
	return;
}

int CTmcRTH_BlockList::GetnBlock( void )
{
	return nBlock;
}

CString CTmcRTH_BlockList::GetcsBlock( void )
{
	return csBlock;
}

_real CTmcRTH_BlockList::GetX0( void )
{
	return dX0;
}

_real CTmcRTH_BlockList::GetY0( void )
{
	return dY0;
}

_real CTmcRTH_BlockList::GetXmin( void )
{
	return dXmin;
}

_real CTmcRTH_BlockList::GetXmax( void )
{
	return dXmax;
}

_real CTmcRTH_BlockList::GetYmin( void )
{
	return dYmin;
}

_real CTmcRTH_BlockList::GetYmax( void )
{
	return dYmax;
}

CTmcRTH_BlockList& CTmcRTH_BlockList::operator=( CTmcRTH_BlockList& cBlList )
{
	DeleteData();
	Add( cBlList );
	return (*this);
}

void CTmcRTH_BlockList::Add( CTmcRTH_BlockList &cCh )
{
	CTmcRTH_BlockList *pcNext;
	_real *pR;
	int i;

	if( nBlock != 0 )
	{
		if( pcNextBlockList == NULL )
		{
			return;
		};
		pcNextBlockList->Add( cCh );
		return;
	};

	nBlock = cCh.GetnBlock();
	nMemory = cCh.GetnMemory();
	dX0 = cCh.GetX0();
	dY0 = cCh.GetY0();
	csBlock = cCh.GetcsBlock();
	csEpsExpr = cCh.GetcsEpsExpr();
	nType = cCh.GetnType();
	dXmin = cCh.GetXmin();
	dXmax = cCh.GetXmax();
	dYmin = cCh.GetYmin();
	dYmax = cCh.GetYmax();
	nXY = cCh.GetnXY();
	pR = cCh.GetpX();
	csVx = cCh.GetcsVx();
	csVy = cCh.GetcsVy();
	csW  = cCh.GetcsW();
	if( dX != NULL ) delete []dX;
	dX = NULL;		  
	if( (pR != NULL)&&(nXY > 0) )
	{
		if( (dX = new _real[nXY]) != NULL )
		{
//			CString csBuf1;
			for( i = 0; i < nXY; i++ ) 
			{
				dX[i] = pR[i];
//				csBuf1.Format("{%lg;%lg}", dX[i], pR[i] );
//				AfxMessageBox(csBuf1);
			};
		}
		else
		{
			nXY = 0;
			if( pcNextBlockList != NULL ) delete []pcNextBlockList;
			nBlock = 0;
			return;
		};
	};
	pR = cCh.GetpY();
	if( dY != NULL ) delete []dY;
	dY = NULL;		  
	if( (pR != NULL)&&(nXY > 0) )
	{
		if( (dY = new _real[nXY]) != NULL )
		{
//			CString csBuf1;
			for( i = 0; i < nXY; i++ ) 
			{
				dY[i] = pR[i];
//				csBuf1.Format("{%lg;%lg}", dX[i], pR[i] );
//				AfxMessageBox(csBuf1);
			};
		}
		else
		{
			nXY = 0;
			if( dX != NULL ) delete []dX;
			if( pcNextBlockList != NULL ) delete []pcNextBlockList;
			nBlock = 0;
			return;
		};
	};

	if( pcNextBlockList != NULL ) delete []pcNextBlockList;
	if( (pcNextBlockList = new CTmcRTH_BlockList[1] )  == NULL )
	{
		nXY = 0;
		if( dY != NULL ) delete []dY;
		if( dX != NULL ) delete []dX;
		nBlock = 0;
		return;
	};

	if( (pcNext = cCh.GetNext()) == NULL ) return;
	if( pcNext->GetnBlock() == 0 ) return;
	Add( pcNext[0] );
	
	return;
}

CTmcRTH_BlockList * CTmcRTH_BlockList::GetNext( void )
{
	return pcNextBlockList; 
}

int CTmcRTH_BlockList::GetBlockNumber( void )
{
	if( nBlock == 0 ) return 0;
	if( pcNextBlockList == NULL ) return 0;
	return 1+pcNextBlockList->GetBlockNumber();
}

void CTmcRTH_BlockList::SetcsBlock( CString csBlock1 )
{
	csBlock = csBlock1;
}

int CTmcRTH_BlockList::GetnType( void )
{
	return nType;
}

void CTmcRTH_BlockList::SetnType( int n )
{
	nType = n;
	return;
}

void CTmcRTH_BlockList::Read( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam )
{
	CString csBuf;

	if( cError.IsError() ) return;
	if( pcParam == NULL )
	{
		csBuf.Format("pointer for parametrs is NULL");
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( strncmp( csBlock, (CTMCRTH_INDANBLCK_FILE), strlen(CTMCRTH_INDANBLCK_FILE) ) == 0 )
	{
		ReadFileStat( cError, pcParam );
		return;
	};

	if( strncmp( csBlock, (CTMCRTH_INDANBLCK_RECTSTAT), strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == 0 )
	{
		ReadRectStat( cError, pcParam );
		return;
	};

	if( strncmp( csBlock, (CTMCRTH_INDANBLCK_CIRCSTAT), strlen(CTMCRTH_INDANBLCK_CIRCSTAT) ) == 0 )
	{
		ReadCircleStat( cError, pcParam );
		return;
	};

	if( strncmp( csBlock, (CTMCRTH_INDANBLCK_POLYGSTT), strlen(CTMCRTH_INDANBLCK_POLYGSTT) ) == 0 )
	{
		ReadPolygonStat( cError, pcParam );
		return;
	};

	if( strncmp( csBlock, (CTMCRTH_INDANBLCK_RECTMOVE), strlen(CTMCRTH_INDANBLCK_RECTMOVE) ) == 0 )
	{
		ReadRectMove( cError, pcParam );
		return;
	};

	if( strncmp( csBlock, (CTMCRTH_INDANBLCK_CIRCMOVE), strlen(CTMCRTH_INDANBLCK_CIRCMOVE) ) == 0 )
	{
		ReadCircleMove( cError, pcParam );
		return;
	};

	if( strncmp( csBlock, (CTMCRTH_INDANBLCK_POLYGMOV), strlen(CTMCRTH_INDANBLCK_POLYGMOV) ) == 0 )
	{
		ReadPolygonMove( cError, pcParam );
		return;
	};

	if( strncmp( csBlock, (CTMCRTH_INDANBLCK_INPUT_X), strlen(CTMCRTH_INDANBLCK_INPUT_X) ) == 0 )
	{
		ReadInputX( cError, pcParam );
		return;
	};

	if( strncmp( csBlock, (CTMCRTH_INDANBLCK_INPUT_Y), strlen(CTMCRTH_INDANBLCK_INPUT_Y) ) == 0 )
	{
		ReadInputY( cError, pcParam );
		return;
	};

	csBuf.Format( "Unknown keyword {%s} in %s %d; use following keywords {%s,%s,%s,%s,%s,%s,%s,%s,%s}",
					csBlock, CTMCRTH_INDANMET_BLOCK, -nBlock,
					CTMCRTH_INDANBLCK_RECTSTAT,
					CTMCRTH_INDANBLCK_CIRCSTAT,
					CTMCRTH_INDANBLCK_POLYGSTT,
					CTMCRTH_INDANBLCK_RECTMOVE,
					CTMCRTH_INDANBLCK_CIRCMOVE,
					CTMCRTH_INDANBLCK_POLYGMOV,
					CTMCRTH_INDANBLCK_INPUT_X,
					CTMCRTH_INDANBLCK_INPUT_Y,
					CTMCRTH_INDANBLCK_FILE);
	cError.PutErrorMessage(csBuf);

	return;
}


void CTmcRTH_BlockList::ReadRectStat( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam )
{
	CString csBuf;

	if( cError.IsError() ) return;
	if( pcParam == NULL )
	{
		csBuf.Format("pointer for parametrs is NULL");
		cError.PutErrorMessage(csBuf);
		return;
	};


	char szBuf[CTMCRTH_INDAN_LINELEN];
	char szBuf1[CTMCRTH_INDAN_LINELEN];
	double r;
	int i, n;

	strcpy( szBuf, csBlock );
	i = strlen( CTMCRTH_INDANBLCK_RECTSTAT );
	n = strlen( szBuf );
	expr_del_Blanks2( szBuf+i );
	csBlock.Format("%s", CTMCRTH_INDANBLCK_RECTSTAT);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) return;
	csEpsExpr.Format("%s", csBuf);
	nType = CTMCRTH_BLCKNTYPE_EPS;
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMET) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_METAL;
	};
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEABS) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_ABSORBER;
	};
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_MAGNETIC;
	};
	
	
	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dXmin = (_real)(r*pcParam->rLongUnit);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dXmax = (_real)(r*pcParam->rLongUnit);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dYmin = (_real)(r*pcParam->rLongUnit);
	
	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dYmax = (_real)(r*pcParam->rLongUnit);

	if( dXmin >= dXmax )
	{
		csBuf.Format("In{%s%d;} Xmin(%lg) >= Xmax(%lg)",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dXmin/pcParam->rLongUnit, dXmax/pcParam->rLongUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( dYmin >= dYmax )
	{
		csBuf.Format("In{%s%d;} Ymin(%lg) >= Ymax(%lg)",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dYmin/pcParam->rLongUnit, dYmax/pcParam->rLongUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	return;
}

void CTmcRTH_BlockList::ReadCircleStat( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam )
{
	CString csBuf;
	if( cError.IsError() ) return;
	if( pcParam == NULL )
	{
		csBuf.Format("pointer for parametrs is NULL");
		cError.PutErrorMessage(csBuf);
		return;
	};

	char szBuf[CTMCRTH_INDAN_LINELEN];
	char szBuf1[CTMCRTH_INDAN_LINELEN];
	double r;
	int i, n;

	strcpy( szBuf, csBlock );
	i = strlen( CTMCRTH_INDANBLCK_CIRCSTAT );
	n = strlen( szBuf );
	expr_del_Blanks2( szBuf+i );
	csBlock.Format("%s", CTMCRTH_INDANBLCK_CIRCSTAT);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) return;
	csEpsExpr.Format("%s", csBuf);
	nType = CTMCRTH_BLCKNTYPE_EPS;
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMET) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_METAL;
	};
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEABS) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_ABSORBER;
	};
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_MAGNETIC;
	};
	
	
	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dXmin = (_real)(r*pcParam->rLongUnit);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dXmax = (_real)(r*pcParam->rLongUnit);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dYmin = (_real)(r*pcParam->rAngleUnit);
	
	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dYmax = (_real)(r*pcParam->rAngleUnit);

	if( dXmin >= dXmax )
	{
		csBuf.Format("In{%s%d;} Rmin(%lg) >= Rmax(%lg)",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dXmin/pcParam->rLongUnit, dXmax/pcParam->rLongUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( dXmin < 0.0 )
	{
		csBuf.Format("In{%s%d;} Rmin(%lg) < 0.0",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dXmin/pcParam->rLongUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( dYmax <= dYmin  )
	{
		csBuf.Format("In{%s%d;} FImin(%lg) >= FImax(%lg)",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dYmin/pcParam->rAngleUnit, dYmax/pcParam->rAngleUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( dYmin < 0.0  )
	{
		csBuf.Format("In{%s%d;} FImin(%lg) < 0",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dYmin/pcParam->rAngleUnit );
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( dYmax > 2*3.14159265359  )
	{
		csBuf.Format("In{%s%d;} FImax(%lg) > 2*PI",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dYmax/pcParam->rAngleUnit );
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( ( dYmax - dYmin ) > 2*3.14159265359 )
	{
		csBuf.Format("In{%s%d;} FImax(%lg) - FImin(%lg) > 2*PI",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dYmin/pcParam->rAngleUnit, dYmax/pcParam->rAngleUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	return;
}

void CTmcRTH_BlockList::ReadPolygonStat( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam )
{
	CString csBuf;
//	CString csBuf1;

	if( cError.IsError() ) return;
	if( pcParam == NULL )
	{
		csBuf.Format("pointer for parametrs is NULL");
		cError.PutErrorMessage(csBuf);
		return;
	};


	int i, n, i1, n1;
	char *szBuf = NULL;

	szBuf = new char[ csBlock.GetLength()+3 ];
	
	if( szBuf == NULL )
	{
		csBuf.Format("Is not enough memory for %s %d at {%s};", CTMCRTH_INDANMET_BLOCK, nBlock, csBlock);
		cError.PutErrorMessage( csBuf );
		return;
	};

	strcpy( szBuf, csBlock );
	i = strlen( CTMCRTH_INDANBLCK_POLYGSTT );
	n = strlen( szBuf );
	csBlock.Format("%s", CTMCRTH_INDANBLCK_POLYGSTT);
	
	if( cError.IsError() ) 
	{
		if( szBuf != NULL )
		{
			delete []szBuf;
			szBuf = NULL;
		};
		return;
	};

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		if( szBuf != NULL )
		{
			delete []szBuf;
			szBuf = NULL;
		};
		return;
	};
	expr_del_Blanks2( csBuf );
	csEpsExpr.Format("%s", csBuf);
	nType = CTMCRTH_BLCKNTYPE_EPS;
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMET) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_METAL;
	};
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEABS) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_ABSORBER;
	};
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_MAGNETIC;
	};
	

	i1 = SkipEOL( szBuf+i );
	if( i1 <= 0 ) 
	{
		csBuf.Format("Missing lines %s x; y; for %s in {%s}",
			CTMCRTH_INDANBLCK_POLGLINE, CTMCRTH_INDANBLCK_POLYGSTT, szBuf);
		cError.PutErrorMessage(csBuf);
		if( szBuf != NULL )
		{
			delete []szBuf;
			szBuf = NULL;
		};
		return;
	};
	i += i1;
	n1 = CalcEOL( szBuf+i );
	if( n1 < 3 ) 
	{
		csBuf.Format("Number's of lines %s x; y; < 3 for %s in {%s}",
			CTMCRTH_INDANBLCK_POLGLINE, CTMCRTH_INDANBLCK_POLYGSTT, szBuf);
		cError.PutErrorMessage(csBuf);
		if( szBuf != NULL )
		{
			delete []szBuf;
			szBuf = NULL;
		};
		return;
	};
	nXY = n1;
	if( dX != NULL ) delete []dX;
	dX = NULL;
	if( (dX = new _real[n1]) == NULL )
	{
		csBuf.Format("In{%s%d;} memory allocation error for dX array",
			CTMCRTH_INDANMET_BLOCK,	-nBlock);
		cError.PutErrorMessage(csBuf);
		if( szBuf != NULL )
		{
			delete []szBuf;
			szBuf = NULL;
		};
		return;
	};
	
	if( dY != NULL ) delete []dY;
	dY = NULL;
	if( (dY = new _real[n1]) == NULL )
	{
		csBuf.Format("In{%s%d;} memory allocation error for dY array",
			CTMCRTH_INDANMET_BLOCK,	-nBlock);
		cError.PutErrorMessage(csBuf);
		if( dX != NULL ) delete []dX;
		dX = NULL;
		if( szBuf != NULL )
		{
			delete []szBuf;
			szBuf = NULL;
		};
		return;
	};
	
	for( i1 = 0; i1 < nXY; i1++ )
	{
//		CString csBuf1;
//		csBuf1.Format("{%s}", szBuf+i );
//		AfxMessageBox(csBuf1);
		ReadXY( szBuf+i,  cError, pcParam , dX+i1, dY+i1 );
//		csBuf1.Format("{%lg;%lg}", dX[i1], dY[i1] );
//		AfxMessageBox(csBuf1);
//		if(i1 > 0) csBuf1.Format("{%lg;%lg}", dX[i1-1], dY[i1-1] );
//		AfxMessageBox(csBuf1);
		if( cError.IsError() )
		{
			if( szBuf != NULL )
			{
				delete []szBuf;
				szBuf = NULL;
			};
			return;
		};
		if( i1 < nXY ) i += SkipEOL( szBuf+i  );
	};

/*	{
	
		int i;
		_real *pX, *pY;
		CString csBuf1;
	
		pX = GetpX();
		pY = GetpY();
		
		if( pX == NULL ) return;
		if( pY == NULL ) return;
		
		for( i = 0; i < GetnXY(); i++ )
		{
			csBuf1.Format("{%lg;%lg}", dX[i], dY[i] );
			AfxMessageBox(csBuf1);
			csBuf1.Format("{%lg;%lg}", pX[i], pY[i] );
			AfxMessageBox(csBuf1);
		};	

	}
*/	
	
	if( szBuf != NULL )
	{
		delete []szBuf;
		szBuf = NULL;
	};
	return;
}

void CTmcRTH_BlockList::ReadRectMove( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam )
{
	CString csBuf;

	if( cError.IsError() ) return;
	if( pcParam == NULL )
	{
		csBuf.Format("pointer for parametrs is NULL");
		cError.PutErrorMessage(csBuf);
		return;
	};


	char szBuf[CTMCRTH_INDAN_LINELEN];
	char szBuf1[CTMCRTH_INDAN_LINELEN];
	double r;
	int i, n;

	strcpy( szBuf, csBlock );
	i = strlen( CTMCRTH_INDANBLCK_RECTMOVE );
	n = strlen( szBuf );
	expr_del_Blanks2( szBuf+i );
	csBlock.Format("%s", CTMCRTH_INDANBLCK_RECTMOVE);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) return;
	csEpsExpr.Format("%s", csBuf);
	nType = CTMCRTH_BLCKNTYPE_EPS;
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMET) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_METAL;
	};
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEABS) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_ABSORBER;
	};
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_MAGNETIC;
	};
	
	
	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dXmin = (_real)(r*pcParam->rLongUnit);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dXmax = (_real)(r*pcParam->rLongUnit);
//	csBuf1.Format();
//	AfxMessageBox(csBuf1);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dYmin = (_real)(r*pcParam->rLongUnit);
	
	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dYmax = (_real)(r*pcParam->rLongUnit);

	if( dXmin >= dXmax )
	{
		csBuf.Format("In{%s%d;} Xmin(%lg) >= Xmax(%lg)",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dXmin/pcParam->rLongUnit, dXmax/pcParam->rLongUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( dYmin >= dYmax )
	{
		csBuf.Format("In{%s%d;} Ymin(%lg) >= Ymax(%lg)",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dYmin/pcParam->rLongUnit, dYmax/pcParam->rLongUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	csVx.Format("%s", szBuf1);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	csVy.Format("%s", szBuf1);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	csW.Format("%s", szBuf1);

	return;
}

void CTmcRTH_BlockList::ReadCircleMove( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam )
{
	CString csBuf;
	if( cError.IsError() ) return;
	if( pcParam == NULL )
	{
		csBuf.Format("pointer for parametrs is NULL");
		cError.PutErrorMessage(csBuf);
		return;
	};

	char szBuf[CTMCRTH_INDAN_LINELEN];
	char szBuf1[CTMCRTH_INDAN_LINELEN];
	double r;
	int i, n;

	strcpy( szBuf, csBlock );
	i = strlen( CTMCRTH_INDANBLCK_CIRCSTAT );
	n = strlen( szBuf );
	expr_del_Blanks2( szBuf+i );
	csBlock.Format("%s", CTMCRTH_INDANBLCK_CIRCSTAT);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) return;
	csEpsExpr.Format("%s", csBuf);
	nType = CTMCRTH_BLCKNTYPE_EPS;
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMET) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_METAL;
	};
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEABS) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_ABSORBER;
	};
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_MAGNETIC;
	};
	
	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dXmin = (_real)(r*pcParam->rLongUnit);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dXmax = (_real)(r*pcParam->rLongUnit);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dYmin = (_real)(r*pcParam->rAngleUnit);
	
	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dYmax = (_real)(r*pcParam->rAngleUnit);

	if( dXmin >= dXmax )
	{
		csBuf.Format("In{%s%d;} Rmin(%lg) >= Rmax(%lg)",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dXmin/pcParam->rLongUnit, dXmax/pcParam->rLongUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( dXmin < 0.0 )
	{
		csBuf.Format("In{%s%d;} Rmin(%lg) < 0.0",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dXmin/pcParam->rLongUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( dYmin < 0.0  )
	{
		csBuf.Format("In{%s%d;} FImin(%lg) < 0",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dYmin/pcParam->rAngleUnit );
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( dYmax > 2*3.14159265359  )
	{
		csBuf.Format("In{%s%d;} FImax(%lg) > 2*PI",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dYmax/pcParam->rAngleUnit );
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( dYmax <= dYmin  )
	{
		csBuf.Format("In{%s%d;} FImin(%lg) >= FImax(%lg)",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dYmin/pcParam->rAngleUnit, dYmax/pcParam->rAngleUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( ( dYmax - dYmin ) > 2*3.14159265359 )
	{
		csBuf.Format("In{%s%d;} FImax(%lg) - FImin(%lg) > 2*PI",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dYmin/pcParam->rAngleUnit, dYmax/pcParam->rAngleUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	csVx.Format("%s", szBuf1);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	csVy.Format("%s", szBuf1);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	csW.Format("%s", szBuf1);

	return;
}

void CTmcRTH_BlockList::ReadPolygonMove( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam )
{
	CString csBuf;
//	CString csBuf1;

	if( cError.IsError() ) return;
	if( pcParam == NULL )
	{
		csBuf.Format("pointer for parametrs is NULL");
		cError.PutErrorMessage(csBuf);
		return;
	};


	char szBuf[CTMCRTH_INDAN_LINELEN];
	char szBuf1[CTMCRTH_INDAN_LINELEN];
	int i, n, i1, n1;

	strcpy( szBuf, csBlock );
	i = strlen( CTMCRTH_INDANBLCK_POLYGMOV );
	n = strlen( szBuf );
	csBlock.Format("%s", CTMCRTH_INDANBLCK_POLYGMOV);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) return;
	expr_del_Blanks2( csBuf );
	csEpsExpr.Format("%s", csBuf);
	nType = CTMCRTH_BLCKNTYPE_EPS;
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMET) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_METAL;
	};
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEABS) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_ABSORBER;
	};
	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == NULL )
	{
		nType = CTMCRTH_BLCKNTYPE_MAGNETIC;
	};
	


	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	csVx.Format("%s", szBuf1);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	csVy.Format("%s", szBuf1);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	csW.Format("%s", szBuf1);
	
	i1 = SkipEOL( szBuf+i );
	if( i1 <= 0 ) 
	{
		csBuf.Format("Missing lines %s x; y; for %s in {%s}",
			CTMCRTH_INDANBLCK_POLGLINE, CTMCRTH_INDANBLCK_POLYGSTT, szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	i += i1;
	n1 = CalcEOL( szBuf+i );
	if( n1 < 3 ) 
	{
		csBuf.Format("Number's of lines %s x; y; < 3 for %s in {%s}",
			CTMCRTH_INDANBLCK_POLGLINE, CTMCRTH_INDANBLCK_POLYGSTT, szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	nXY = n1;
	if( dX != NULL ) delete []dX;
	dX = NULL;
	if( (dX = new _real[n1]) == NULL )
	{
		csBuf.Format("In{%s%d;} memory allocation error for dX array",
			CTMCRTH_INDANMET_BLOCK,	-nBlock);
		cError.PutErrorMessage(csBuf);
		return;
	};
	
	if( dY != NULL ) delete []dY;
	dY = NULL;
	if( (dY = new _real[n1]) == NULL )
	{
		csBuf.Format("In{%s%d;} memory allocation error for dY array",
			CTMCRTH_INDANMET_BLOCK,	-nBlock);
		cError.PutErrorMessage(csBuf);
		if( dX != NULL ) delete []dX;
		dX = NULL;
		return;
	};
	
	for( i1 = 0; i1 < nXY; i1++ )
	{
//		CString csBuf1;
//		csBuf1.Format("{%s}", szBuf+i );
//		AfxMessageBox(csBuf1);
		ReadXY( szBuf+i,  cError, pcParam , dX+i1, dY+i1 );
//		csBuf1.Format("{%lg;%lg}", dX[i1], dY[i1] );
//		AfxMessageBox(csBuf1);
//		if(i1 > 0) csBuf1.Format("{%lg;%lg}", dX[i1-1], dY[i1-1] );
//		AfxMessageBox(csBuf1);
		if( cError.IsError() ) return;
		if( i1 < nXY ) i += SkipEOL( szBuf+i  );
	};

/*	{
	
		int i;
		_real *pX, *pY;
		CString csBuf1;
	
		pX = GetpX();
		pY = GetpY();
		
		if( pX == NULL ) return;
		if( pY == NULL ) return;
		
		for( i = 0; i < GetnXY(); i++ )
		{
			csBuf1.Format("{%lg;%lg}", dX[i], dY[i] );
			AfxMessageBox(csBuf1);
			csBuf1.Format("{%lg;%lg}", pX[i], pY[i] );
			AfxMessageBox(csBuf1);
		};	

	}
*/	return;
}

void CTmcRTH_BlockList::ReadInputX( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam )
{
	CString csBuf;

	if( cError.IsError() ) return;
	if( pcParam == NULL )
	{
		csBuf.Format("pointer for parametrs is NULL");
		cError.PutErrorMessage(csBuf);
		return;
	};


	char szBuf[CTMCRTH_INDAN_LINELEN];
	char szBuf1[CTMCRTH_INDAN_LINELEN];
	double r;
	int i, n;

	strcpy( szBuf, csBlock );
	i = strlen( CTMCRTH_INDANBLCK_INPUT_X );
	n = strlen( szBuf );
	expr_del_Blanks2( szBuf+i );
	csBlock.Format("%s", CTMCRTH_INDANBLCK_INPUT_X);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) return;
	csEpsExpr.Format("%s", csBuf);
	nType = CTMCRTH_BLCKNTYPE_INPXLEFT;

	if( (strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMET) != NULL)&&
		(strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) != NULL)&&
		(strlen( csEpsExpr ) != 0))
	{
		csBuf.Format("%s%d; first parameters in %s must be %s, %s or NULL", 
			CTMCRTH_INDANMET_BLOCK,	nBlock,
			csEpsExpr,
			CTMCRTH_INDANBLCK_TYPEMET,
			CTMCRTH_INDANBLCK_TYPEMAG);
		cError.PutErrorMessage(csBuf);
		return;
	};
	
	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dXmin = (_real)(r*pcParam->rLongUnit);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dXmax = (_real)(r*pcParam->rLongUnit);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dYmin = (_real)(r*pcParam->rTimeUnit);
	
	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dYmax = (_real)(r*pcParam->rTimeUnit);

	if( dXmin >= dXmax )
	{
		csBuf.Format("In{%s%d;} Ymin(%lg) >= Ymax(%lg)",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dXmin/pcParam->rLongUnit, dXmax/pcParam->rLongUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( dYmin >= dYmax )
	{
		csBuf.Format("In{%s%d;} Tmin(%lg) >= Tmax(%lg)",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dYmin/pcParam->rLongUnit, dYmax/pcParam->rLongUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		cError.Clear();
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	csVx.Format("%s", szBuf1);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	csVy.Format("%s", szBuf1);


	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		cError.Clear();
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	csW.Format("%s", szBuf1);

	return;
}

void CTmcRTH_BlockList::ReadInputY( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam )
{
	CString csBuf;

	if( cError.IsError() ) return;
	if( pcParam == NULL )
	{
		csBuf.Format("pointer for parametrs is NULL");
		cError.PutErrorMessage(csBuf);
		return;
	};


	char szBuf[CTMCRTH_INDAN_LINELEN];
	char szBuf1[CTMCRTH_INDAN_LINELEN];
	double r;
	int i, n;

	strcpy( szBuf, csBlock );
	i = strlen( CTMCRTH_INDANBLCK_INPUT_Y );
	n = strlen( szBuf );
	expr_del_Blanks2( szBuf+i );
	csBlock.Format("%s", CTMCRTH_INDANBLCK_INPUT_Y);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) return;
	csEpsExpr.Format("%s", csBuf);
	nType = CTMCRTH_BLCKNTYPE_INPYTOP;

	if( (strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMET) != NULL)&&
		(strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) != NULL)&&
		(strlen( csEpsExpr ) != 0))
	{
		csBuf.Format("%s%d; first parameters in %s must be %s, %s or NULL", 
			CTMCRTH_INDANMET_BLOCK,	nBlock,
			csEpsExpr,
			CTMCRTH_INDANBLCK_TYPEMET,
			CTMCRTH_INDANBLCK_TYPEMAG);
		cError.PutErrorMessage(csBuf);
		return;
	};
	
	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dXmin = (_real)(r*pcParam->rLongUnit);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dXmax = (_real)(r*pcParam->rLongUnit);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dYmin = (_real)(r*pcParam->rTimeUnit);
	
	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	dYmax = (_real)(r*pcParam->rTimeUnit);

	if( dXmin >= dXmax )
	{
		csBuf.Format("In{%s%d;} Xmin(%lg) >= Xmax(%lg)",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dXmin/pcParam->rLongUnit, dXmax/pcParam->rLongUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( dYmin >= dYmax )
	{
		csBuf.Format("In{%s%d;} Tmin(%lg) >= Tmax(%lg)",
			CTMCRTH_INDANMET_BLOCK,	-nBlock,
			dYmin/pcParam->rLongUnit, dYmax/pcParam->rLongUnit);
		cError.PutErrorMessage(csBuf);
		return;
	};

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		cError.Clear();
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	csVx.Format("%s", szBuf1);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", szBuf);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	csVy.Format("%s", szBuf1);


	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) 
	{
		cError.Clear();
		return;
	};
	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	csW.Format("%s", szBuf1);


	return;
}

int CTmcRTH_BlockList::expr_del_Blanks2( char *ch )
{
	int i, i1;
	for( i = 0, i1 = 0;  i >= 0 ; i++  )
	{
		switch( ch[i] )
		{
			case ' ':  
				break;
			case '\t':	
				break;
			case '\n':
				break;
			default:
				ch[i1] = ch[i];
				i1++;
				if( ch[i] == '\0' ) 
				{
					return 0;
				};
		};
	};
	return 0;
}

int CTmcRTH_BlockList::expr_del_Blanks2( CString &csBuf )
{
	int i;
	char ch[CTMCRTH_INDAN_LINELEN];

	strcpy( ch, csBuf );
	i = expr_del_Blanks2( ch );
	csBuf.Format("%s", ch);

	return i;
}

int CTmcRTH_BlockList::Search_1Param( CString &csBuf, char *szBuf, CTmcLibError &cError )
{
	if( cError.IsError() ) return -1;

	int i, i1, i2, i3;

	for( i = 0, i1 = 0, i2 = 0, i3 = 0; (i < CTMCRTH_INDANSBUF)&&(szBuf[i] != '\0'); i++ )
	{
		switch( szBuf[i] )
		{
			case '{': i1++;
					break;
			case '}': i1--;
					break;
			case '(': i2++;
					break;
			case ')': i2--;
					break;
			case '[': i3++;
					break;
			case ']': i3--;
					break;
			case ';': 
					if( (i1 == 0)&&(i2 == 0)&&(i3 == 0) )
					{
						szBuf[i] = '\0';
						csBuf.Format( "%s", szBuf);
						szBuf[i] = ';';
						i++;
						return i;
					};
					break;
		};
	}

	cError.PutErrorMessage("Missing separator ;");
	return -1;
}


CString CTmcRTH_BlockList::GetcsEpsExpr( void )
{
	return csEpsExpr;
}

int CTmcRTH_BlockList::GetnXY( void )
{
	return nXY;
}

_real * CTmcRTH_BlockList::GetpX( void )
{
	return dX;
}

_real * CTmcRTH_BlockList::GetpY( void )
{
	return dY;
}

int CTmcRTH_BlockList::CalcEOL( char *szBuf  )
{
	int i, ii;
	for( i = 0, ii = 0; szBuf[i] != '\0'; i++ )
	{
		if( szBuf[i] == '\n' ) ii++;
	};
	return ii;
}

int CTmcRTH_BlockList::SkipEOL( char *szBuf )
{
	int i;
	for( i = 0; szBuf[i] != '\0'; i++ )
	{
		if( szBuf[i] == '\n' ) return i+1;
	};
	return 0;
}

void CTmcRTH_BlockList::ReadXY( char *szBuf,  CTmcLibError &cError, CTmcRTH_IndanParam * pcParam , _real *dX1, _real *dY1 )
{
	CString csBuf;
//	CString csBuf1;
	int i;					 
	char szBuf1[CTMCRTH_INDAN_LINELEN];
	double r;

	if( cError.IsError() ) return;
	if( pcParam == NULL )
	{
		csBuf.Format("pointer for parametrs is NULL");
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( strncmp( szBuf, CTMCRTH_INDANBLCK_POLGLINE, strlen(CTMCRTH_INDANBLCK_POLGLINE) ) != 0 )
	{
		csBuf.Format("Unknow keyword {%s} for %s, use %s x; y;", szBuf,
				CTMCRTH_INDANBLCK_POLYGSTT, CTMCRTH_INDANBLCK_POLGLINE);
		cError.PutErrorMessage(csBuf);
		return;
	};
	
	i = strlen(CTMCRTH_INDANBLCK_POLGLINE);

//	{
//		CString csBuf1;
//		csBuf1.Format("{%s}", szBuf+i);
//		AfxMessageBox(csBuf1);
//	};
	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) return;
//	AfxMessageBox(csBuf);
	strcpy( szBuf1, csBuf );
//	csBuf1.Format("{%s}", szBuf1);
//	AfxMessageBox(csBuf1);
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	*dX1 = (_real)(r*pcParam->rLongUnit);
//	csBuf1.Format("{%lg}", dX1);
//	AfxMessageBox(csBuf1);

//	csBuf1.Format("{%s}", szBuf+i);
//	AfxMessageBox(csBuf1);
	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) return;
//	AfxMessageBox(csBuf);
	strcpy( szBuf1, csBuf );
//	csBuf1.Format("{%s}", szBuf1);
//	AfxMessageBox(csBuf1);
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}%s{%s}", szBuf, expr_get_error(), szBuf1);
		cError.PutErrorMessage(csBuf);
		return;
	};
	*dY1 = (_real)(r*pcParam->rLongUnit);
//	csBuf1.Format("{%lg}", dY1);
//	AfxMessageBox(csBuf1);

	return;
}

CString CTmcRTH_BlockList::GetcsVx( void )
{
	return csVx;
}

CString CTmcRTH_BlockList::GetcsVy( void )
{
	return csVy;
}

CString CTmcRTH_BlockList::GetcsW( void )
{
	return csW;
}

int CTmcRTH_BlockList::GetnMemory( void )
{
	return nMemory;
}

void CTmcRTH_BlockList::SetnMemory( int nnn )
{
	nMemory = nnn;
}

int CTmcRTH_BlockList::GetnMemoryAll( void )
{
	if( pcNextBlockList == NULL ) return GetnMemory();
	return GetnMemory() + pcNextBlockList->GetnMemoryAll();
}

void CTmcRTH_BlockList::ReadFileStat( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam )
{
	CString csBuf;

	if( cError.IsError() ) return;
	if( pcParam == NULL )
	{
		csBuf.Format("pointer for parametrs is NULL");
		cError.PutErrorMessage(csBuf);
		return;
	};

	char szBuf[CTMCRTH_INDAN_LINELEN];
	int i, n;

	strcpy( szBuf, csBlock );
	i = strlen( CTMCRTH_INDANBLCK_FILE );
	n = strlen( szBuf );
	expr_del_Blanks2( szBuf+i );
	csBlock.Format("%s", CTMCRTH_INDANBLCK_FILE);

	i += Search_1Param( csBuf, szBuf+i, cError );
	if( cError.IsError() ) return;
	csEpsExpr.Format("%s", csBuf);
	nType = CTMCRTH_BLCKNTYPE_EPS;

	return;
}

void CTmcRTH_BlockList::AddPathForFile( CString &csPath )
{
	if( pcNextBlockList != NULL ) pcNextBlockList->AddPathForFile( csPath );

	if( strncmp( csBlock, (CTMCRTH_INDANBLCK_FILE), strlen(CTMCRTH_INDANBLCK_FILE) ) == 0 )
	{
		AddFileEpsExtention();
		AddCurrentPath( csPath );
		return;
	};

	return;
}

void CTmcRTH_BlockList::AddFileEpsExtention()
{
	char szBuf[CTMCRTH_INDANSBUF];
	int i, n;
	
	strcpy( szBuf, csEpsExpr);
	n = strlen(szBuf);
	
	if( n == 0 ) return;
	
	for( i = 0; i < n; i++)
	{
		switch( szBuf[i] )
		{
			case '.':
				return;
				break;
		};
	};

	csEpsExpr += ".";
	csEpsExpr += TMC_RTH_EPSFILE_EXT_;

	return;
}

void CTmcRTH_BlockList::AddCurrentPath(CString csCurrentPath)
{
	CString csBuf;
	
	if( IsFullName() ) return;

	csBuf.Format("%s\\%s", csCurrentPath, csEpsExpr);
	csEpsExpr = csBuf;

	return;
}

BOOL CTmcRTH_BlockList::IsFullName()
{
	int i, n;

	n = csEpsExpr.GetLength();
	if( n == 0 ) return TRUE;
	for( i = 0; i < n; i++ )
	{
		if( csEpsExpr[i] == ':' ) return TRUE;
	}
	
	return FALSE;
}

BOOL CTmcRTH_BlockList::Save(CString &csBuffer)
{
	CString csBuf;
	BOOL bBuf;
	csBuffer.Format("%s\n", TMC_RTH_BLOCKLISTSAVE_BEGIN);
	bBuf = Save1( csBuffer );
	csBuf.Format("%s", TMC_RTH_BLOCKLISTSAVE_END);
	csBuffer += csBuf;
	return bBuf;
}

BOOL CTmcRTH_BlockList::Save1(CString &csBuffer)
{
	CString csBuf;
	BOOL bBuf = FALSE;
	int i;

	if( nBlock == 0 ) return TRUE;
	csBuf.Format("%s\n%s\n%s\n%s\n%s\n%lg %lg %lg %lg %lg %lg %d %d %d %d\n", 
					csBlock, 
					csEpsExpr,
					csVx,
					csVy,
					csW,
					dX0, dXmax, dXmin, dY0, dYmax, dYmin, nBlock, nMemory, nType, nXY);
	csBuffer += csBuf;
	
	for( i = 0; i < nXY; i++ )
	{
		csBuf.Format("%lg %lg\n", dX[i], dY[i]);
		csBuffer += csBuf;
	};
	
	if( pcNextBlockList != NULL ) bBuf = pcNextBlockList->Save1( csBuffer );

	return bBuf;
}

void CTmcRTH_BlockList::Load( FILE **fp, CTmcLibError &cError )
{

	if( cError.IsError() ) return;

	if( *fp == NULL )
	{
		cError.PutErrorMessage("FILE pointer is NULL when read block_list");
		return;
	};
	if(nBlock == 0)
	{
		{
			int i;
			if( fgets( pszBufForReadTopology, TMC_RTH_BLOCKLISTLOADBUFSIZE, *fp) == NULL )
			{
				cError.PutErrorMessage( "bad topology file (can't read block list)" );
				nBlock = 0;
				fclose(*fp);
				*fp = NULL;
				return;
			};
			if( strncmp( pszBufForReadTopology, TMC_RTH_BLOCKLISTSAVE_END, strlen( TMC_RTH_BLOCKLISTSAVE_END ) ) == 0 )
			{
				return;
			};
			del_eol( pszBufForReadTopology );
			csBlock.Format("%s", pszBufForReadTopology);
			if( fgets( pszBufForReadTopology, TMC_RTH_BLOCKLISTLOADBUFSIZE, *fp) == NULL )
			{
				cError.PutErrorMessage( "bad topology file (can't read block list)" );
				nBlock = 0;
				fclose(*fp);
				*fp = NULL;
				return;
			};
			del_eol( pszBufForReadTopology );
			csEpsExpr.Format("%s", pszBufForReadTopology);
			if( fgets( pszBufForReadTopology, TMC_RTH_BLOCKLISTLOADBUFSIZE, *fp) == NULL )
			{
				cError.PutErrorMessage( "bad topology file (can't read block list)" );
				nBlock = 0;
				fclose(*fp);
				*fp = NULL;
				return;
			};
			del_eol( pszBufForReadTopology );
			csVx.Format("%s", pszBufForReadTopology);
			if( fgets( pszBufForReadTopology, TMC_RTH_BLOCKLISTLOADBUFSIZE, *fp) == NULL )
			{
				cError.PutErrorMessage( "bad topology file (can't read block list)" );
				nBlock = 0;
				fclose(*fp);
				*fp = NULL;
				return;
			};
			del_eol( pszBufForReadTopology );
			csVy.Format("%s", pszBufForReadTopology);
			if( fgets( pszBufForReadTopology, TMC_RTH_BLOCKLISTLOADBUFSIZE, *fp) == NULL )
			{
				cError.PutErrorMessage( "bad topology file (can't read block list)" );
				nBlock = 0;
				fclose(*fp);
				*fp = NULL;
				return;
			};
			del_eol( pszBufForReadTopology );
			csW.Format("%s", pszBufForReadTopology);
			if( fgets( pszBufForReadTopology, TMC_RTH_BLOCKLISTLOADBUFSIZE, *fp) == NULL )
			{
				cError.PutErrorMessage( "bad topology file (can't read block list)" );
				nBlock = 0;
				fclose(*fp);
				*fp = NULL;
				return;
			};
			if( sscanf( pszBufForReadTopology, "%lg %lg %lg %lg %lg %lg %d %d %d %d", &dX0, &dXmax, &dXmin, &dY0, &dYmax, &dYmin, &nBlock, &nMemory, &nType, &nXY) != 10 )
			{
				cError.PutErrorMessage( "bad topology file (can't read block list)" );
				nBlock = 0;
				fclose(*fp);
				*fp = NULL;
				return;
			};
			if( nXY > 0 )
			{
				if( dX != NULL ) delete []dX;
				dX = NULL;		  
				if(	(dX = new _real[nXY]) == NULL )
				{
					cError.PutErrorMessage( "Memory allocation at block list" );
					nBlock = 0;
					fclose(*fp);
					*fp = NULL;
					return;
				};
				if( dY != NULL ) delete []dY;
				dY = NULL;		  
				if( (dY = new _real[nXY]) == NULL )
				{
					cError.PutErrorMessage( "Memory allocation at block list" );
					nBlock = 0;
					fclose(*fp);
					*fp = NULL;
					return;
				};
				for( i = 0; i < nXY; i++ )
				{
					if( fgets( pszBufForReadTopology, TMC_RTH_BLOCKLISTLOADBUFSIZE, *fp) == NULL )
					{
						cError.PutErrorMessage( "bad topology file (can't read block list)" );
						nBlock = 0;
						fclose(*fp);
						*fp = NULL;
						return;
					};
					if( sscanf( pszBufForReadTopology, "%lg %lg", &(dX[i]), &(dY[i]) ) != 2 )
					{
						cError.PutErrorMessage( "bad topology file (can't read block list)" );
						nBlock = 0;
						fclose(*fp);
						*fp = NULL;
						return;
					};
				};
			};
		};
		if( pcNextBlockList != NULL ) delete []pcNextBlockList;
		if( (pcNextBlockList = new CTmcRTH_BlockList[1] )  == NULL )
		{
			cError.PutErrorMessage( "Memory allocation at block list" );
			nBlock = 0;
			fclose(*fp);
			*fp = NULL;
			return;
		};
		pcNextBlockList->Load( fp, cError );
	}
	else
	{
		if( pcNextBlockList == NULL )
		{
			if( (pcNextBlockList = new CTmcRTH_BlockList[1] )  == NULL )
			{
				cError.PutErrorMessage( "Memory allocation at block list" );
				nBlock = 0;
				fclose(*fp);
				*fp = NULL;
				return;
			};
		};
		pcNextBlockList->Load( fp, cError );
	};
	return;
}

void CTmcRTH_BlockList::del_eol( char *szBuf )
{
	int i;
	for( i = 0; szBuf[i] != '\0'; i++)
	{
		switch( szBuf[i] )
		{
			case 10:
				szBuf[i] = '\0';
				return;
			case 13:
				szBuf[i] = '\0';
				return;
		};
	}
	return;
};


void CTmcRTH_BlockList::SetXmin( double r )
{
	dXmin = (_real)r;
	return;
};

void CTmcRTH_BlockList::SetXmax( double r )
{
	dXmax = (_real)r;
	return;
};

void CTmcRTH_BlockList::SetYmin( double r )
{
	dYmin = (_real)r;
	return;
};

void CTmcRTH_BlockList::SetYmax( double r )
{
	dYmax = (_real)r;
	return;
};

