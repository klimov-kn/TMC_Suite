// TmcBoundaryTpl.cpp : implementation file
//

#include "stdafx.h"
#include <tmcgrviw.h>
#include "FldView.h"
#include "TmcBoundaryTpl.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcBoundaryTpl

CTmcBoundaryTpl::CTmcBoundaryTpl()
{
	pY1 = NULL;
	pY2 = NULL;
	pY3 = NULL;
	pY4 = NULL;
	pX1 = NULL;
	pX2 = NULL;
	pX3 = NULL;
	pX4 = NULL;
	pcTopSize = NULL;

	nTopSize = 0;
	nPoint = 0;
	nType = CTMCRTH_BLCKNTYPE_METAL;
	dX0 = 0.0;
	dY0 = 0.0;
	nInputNum = 0;
	return;
}

CTmcBoundaryTpl::~CTmcBoundaryTpl()
{
	DeleteData();
	return;
}


//BEGIN_MESSAGE_MAP(CTmcBoundaryTpl, CWnd)
	//{{AFX_MSG_MAP(CTmcBoundaryTpl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTmcBoundaryTpl message handlers

void CTmcBoundaryTpl::DeleteData()
{
	if( pY1 != NULL ) delete []pY1;
	pY1 = NULL;
	if( pY2 != NULL ) delete []pY2;
	pY2 = NULL;
	if( pY3 != NULL ) delete []pY3;
	pY3 = NULL;
	if( pY4 != NULL ) delete []pY4;
	pY4 = NULL;

	if( pX1 != NULL ) delete []pX1;
	pX1 = NULL;
	if( pX2 != NULL ) delete []pX2;
	pX2 = NULL;
	if( pX3 != NULL ) delete []pX3;
	pX3 = NULL;
	if( pX4 != NULL ) delete []pX4;
	pX4 = NULL;

	if( pcTopSize != NULL ) delete []pcTopSize;
	pcTopSize = NULL;
	
	nTopSize = 0;
	nPoint = 0;
	nBlock = 0;
	nType = CTMCRTH_BLCKNTYPE_METAL;
	csBlock.Format("");
	csEpsExpr.Format("");
	dX0 = 0.0;
	dY0 = 0.0;
	cTopSizeLinklListX0.DeleteData();
	cTopSizeLinklListY0.DeleteData();
	nInputNum = 0;

	return;
}

void CTmcBoundaryTpl::SetBlockList(CTmcRTH_BlockList *pcBlList, CTmcLibError &cError, double dDelta)
{
	if( cError.IsError() ) return;
	if( pcBlList == NULL )
	{
		cError.PutErrorMessage("pointer for BlockList is NULL");
		return;
	};
	
	DeleteData();

	nType = pcBlList->GetnType();
	nBlock = pcBlList->GetnBlock(); 
	csBlock = pcBlList->GetcsBlock();
	csEpsExpr = pcBlList->GetcsEpsExpr();
	dX0 = pcBlList->GetX0();
	dY0 = pcBlList->GetY0();
	cTopSizeLinklListX0.SetdX1( 0.0 );
	cTopSizeLinklListX0.SetdX2( dX0 );
	cTopSizeLinklListX0.SetdY1( dY0 );
	cTopSizeLinklListX0.SetdY2( dY0 );
	cTopSizeLinklListX0.SetdSize( dX0 );
	cTopSizeLinklListY0.SetdX1( 0.0 );
	cTopSizeLinklListY0.SetdX2( 0.0 );
	cTopSizeLinklListY0.SetdY1( 0.0 );
	cTopSizeLinklListY0.SetdY2( dY0 );
	cTopSizeLinklListY0.SetdSize( dY0 );

	if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTAT, strlen(CTMCRTH_INDANBLCK_RECTSTAT)) == 0 )
	{
		SetRectStat( pcBlList, cError, dDelta );
		TranslateX0Y0();
		return;
	};
	if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTMOVE, strlen(CTMCRTH_INDANBLCK_RECTMOVE)) == 0 )
	{
		SetRectStat( pcBlList, cError, dDelta );
		TranslateX0Y0();
		return;
	};
	if( strncmp( csBlock, CTMCRTH_INDANBLCK_FILE, strlen(CTMCRTH_INDANBLCK_FILE)) == 0 )
	{
		nType = CTMCRTH_BLCKNTYPE_EPS;
		SetRectStat( pcBlList, cError, dDelta );
		TranslateX0Y0();
		return;
	};

	if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTT, strlen(CTMCRTH_INDANBLCK_POLYGSTT)) == 0 )
	{
		SetPolygonStat( pcBlList, cError, dDelta );
		TranslateX0Y0();
		return;
	};
	if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGMOV, strlen(CTMCRTH_INDANBLCK_POLYGMOV)) == 0 )
	{
		SetPolygonStat( pcBlList, cError, dDelta );
		TranslateX0Y0();
		return;
	};

	if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTAT, strlen(CTMCRTH_INDANBLCK_CIRCSTAT)) == 0 )
	{
		SetCircleStat( pcBlList, cError, dDelta );
		TranslateX0Y0();
		return;
	};
	if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCMOVE, strlen(CTMCRTH_INDANBLCK_CIRCMOVE)) == 0 )
	{
		SetCircleStat( pcBlList, cError, dDelta );
		TranslateX0Y0();
		return;
	};

	if( strncmp( csBlock, CTMCRTH_INDANBLCK_INPUT_X, strlen(CTMCRTH_INDANBLCK_INPUT_X)) == 0 )
	{
		SetInputX( pcBlList, cError, dDelta );
		TranslateX0Y0();
		return;
	};
	if( strncmp( csBlock, CTMCRTH_INDANBLCK_INPUT_Y, strlen(CTMCRTH_INDANBLCK_INPUT_Y)) == 0 )
	{
		SetInputY( pcBlList, cError, dDelta );
		TranslateX0Y0();
		return;
	};

	return;
}

void CTmcBoundaryTpl::SetRectStat(CTmcRTH_BlockList *pcBlList, CTmcLibError &cError, double dDelta)
{
	if( cError.IsError() ) return;
	if( pcBlList == NULL )
	{
		cError.PutErrorMessage("pointer for BlockList is NULL");
		return;
	};
	nPoint = 0;
	nTopSize = 0;

	if( pY1 != NULL ) delete []pY1;
	pY1 = NULL;
	if( pY2 != NULL ) delete []pY2;
	pY2 = NULL;
	if( pY3 != NULL ) delete []pY3;
	pY3 = NULL;
	if( pY4 != NULL ) delete []pY4;
	pY4 = NULL;

	if( pX1 != NULL ) delete []pX1;
	pX1 = NULL;
	if( pX2 != NULL ) delete []pX2;
	pX2 = NULL;
	if( pX3 != NULL ) delete []pX3;
	pX3 = NULL;
	if( pX4 != NULL ) delete []pX4;
	pX4 = NULL;
	
	if( pcTopSize != NULL ) delete []pcTopSize;
	pcTopSize = NULL;

	nPoint = 4;
	pX1 = new double[ nPoint ];
	pX2 = new double[ nPoint ];
	pX3 = new double[ nPoint ];
	pX4 = new double[ nPoint ];
	pY1 = new double[ nPoint ];
	pY2 = new double[ nPoint ];
	pY3 = new double[ nPoint ];
	pY4 = new double[ nPoint ];
	nTopSize = 2;
	pcTopSize = new CTmcTopSize[nTopSize];

	if(   (pcTopSize == NULL)
		||(pX1 == NULL)||(pX2 == NULL)||(pX3 == NULL)||(pX4 == NULL)
		||(pX1 == NULL)||(pX2 == NULL)||(pX3 == NULL)||(pX4 == NULL))
	{
		DeleteData();
		cError.PutErrorMessage("memory allocation in BlockList Tpl array");
		return;
	};

	pX1[ 0 ] = pcBlList->GetXmin() - dDelta/2;
	pY1[ 0 ] = pcBlList->GetYmin() - dDelta/2;
	pX2[ 0 ] = pcBlList->GetXmin() - dDelta/2;
	pY2[ 0 ] = pcBlList->GetYmax() + dDelta/2;
	pX3[ 0 ] = pcBlList->GetXmin() + dDelta/2;
	pY3[ 0 ] = pcBlList->GetYmax() + dDelta/2;
	pX4[ 0 ] = pcBlList->GetXmin() + dDelta/2;
	pY4[ 0 ] = pcBlList->GetYmin() - dDelta/2;

	pX1[ 1 ] = pcBlList->GetXmin() - dDelta/2;
	pY1[ 1 ] = pcBlList->GetYmax() - dDelta/2;
	pX2[ 1 ] = pcBlList->GetXmin() - dDelta/2;
	pY2[ 1 ] = pcBlList->GetYmax() + dDelta/2;
	pX3[ 1 ] = pcBlList->GetXmax() + dDelta/2;
	pY3[ 1 ] = pcBlList->GetYmax() + dDelta/2;
	pX4[ 1 ] = pcBlList->GetXmax() + dDelta/2;
	pY4[ 1 ] = pcBlList->GetYmax() - dDelta/2;

	pX1[ 2 ] = pcBlList->GetXmax() - dDelta/2;
	pY1[ 2 ] = pcBlList->GetYmin() - dDelta/2;
	pX2[ 2 ] = pcBlList->GetXmax() - dDelta/2;
	pY2[ 2 ] = pcBlList->GetYmax() + dDelta/2;
	pX3[ 2 ] = pcBlList->GetXmax() + dDelta/2;
	pY3[ 2 ] = pcBlList->GetYmax() + dDelta/2;
	pX4[ 2 ] = pcBlList->GetXmax() + dDelta/2;
	pY4[ 2 ] = pcBlList->GetYmin() - dDelta/2;

	pX1[ 3 ] = pcBlList->GetXmin() - dDelta/2;
	pY1[ 3 ] = pcBlList->GetYmin() - dDelta/2;
	pX2[ 3 ] = pcBlList->GetXmin() - dDelta/2;
	pY2[ 3 ] = pcBlList->GetYmin() + dDelta/2;
	pX3[ 3 ] = pcBlList->GetXmax() + dDelta/2;
	pY3[ 3 ] = pcBlList->GetYmin() + dDelta/2;
	pX4[ 3 ] = pcBlList->GetXmax() + dDelta/2;
	pY4[ 3 ] = pcBlList->GetYmin() - dDelta/2;

	pcTopSize[0].SetdX1( pcBlList->GetXmin() );
	pcTopSize[0].SetdX2( pcBlList->GetXmin() );
	pcTopSize[0].SetdY1( pcBlList->GetYmin() );
	pcTopSize[0].SetdY2( pcBlList->GetYmax() );
	pcTopSize[0].SetdSize( pcBlList->GetYmax() - pcBlList->GetYmin() );
	pcTopSize[1].SetdX1( pcBlList->GetXmin() );
	pcTopSize[1].SetdX2( pcBlList->GetXmax() );
	pcTopSize[1].SetdY1( pcBlList->GetYmax() );
	pcTopSize[1].SetdY2( pcBlList->GetYmax() );
	pcTopSize[1].SetdSize( pcBlList->GetXmax() - pcBlList->GetXmin() );

	return;
}

void CTmcBoundaryTpl::SetPolygonStat(CTmcRTH_BlockList *pcBlList, CTmcLibError &cError, double dDelta)
{
	if( cError.IsError() ) return;
	if( pcBlList == NULL )
	{
		cError.PutErrorMessage("pointer for BlockList is NULL");
		return;
	};

	int i;
	double *pX, *pY;

	nPoint = 0;
	nTopSize = 0;

	if( pY1 != NULL ) delete []pY1;
	pY1 = NULL;
	if( pY2 != NULL ) delete []pY2;
	pY2 = NULL;
	if( pY3 != NULL ) delete []pY3;
	pY3 = NULL;
	if( pY4 != NULL ) delete []pY4;
	pY4 = NULL;

	if( pX1 != NULL ) delete []pX1;
	pX1 = NULL;
	if( pX2 != NULL ) delete []pX2;
	pX2 = NULL;
	if( pX3 != NULL ) delete []pX3;
	pX3 = NULL;
	if( pX4 != NULL ) delete []pX4;
	pX4 = NULL;

	if( pcTopSize != NULL ) delete []pcTopSize;
	pcTopSize = NULL;

	if( nType == CTMCRTH_BLCKNTYPE_EPS ) nPoint = pcBlList->GetnXY();
	else nPoint = pcBlList->GetnXY()-1;;
	pX1 = new double[ nPoint ];
	pX2 = new double[ nPoint ];
	pX3 = new double[ nPoint ];
	pX4 = new double[ nPoint ];
	pY1 = new double[ nPoint ];
	pY2 = new double[ nPoint ];
	pY3 = new double[ nPoint ];
	pY4 = new double[ nPoint ];
	pX  = pcBlList->GetpX();
	pY  = pcBlList->GetpY();
	nTopSize = pcBlList->GetnXY();
	pcTopSize = new CTmcTopSize[nTopSize];

	if(   (pcTopSize == NULL)||
		  (pX1 == NULL)||(pX2 == NULL)||(pX3 == NULL)||(pX4 == NULL)||
		  (pX1 == NULL)||(pX2 == NULL)||(pX3 == NULL)||(pX4 == NULL)||
		  ( pX == NULL)||( pY == NULL)||( nPoint < 2 ))
	{
		DeleteData();
		cError.PutErrorMessage("memory allocation in BlockList Tpl array");
		return;
	};

	for( i = 0; i < pcBlList->GetnXY()-1; i++ )
	{
		pcTopSize[i].SetdX1( pX[i] );
		pcTopSize[i].SetdX2( pX[i+1] );
		pcTopSize[i].SetdY1( pY[i] );
		pcTopSize[i].SetdY2( pY[i+1] );
		pcTopSize[i].SetdSize( sqrt( (pX[i]-pX[i+1])*(pX[i]-pX[i+1]) + (pY[i]-pY[i+1])*(pY[i]-pY[i+1]) ) );
		if( fabs( pX[i] - pX[i+1] ) < fabs( pY[i] - pY[i+1] ) )
		{
			pX1[ i ] =   pX[i] - dDelta/2;
			pY1[ i ] =   pY[i] - dDelta/2;
			pX2[ i ] = pX[i+1] - dDelta/2;
			pY2[ i ] = pY[i+1] + dDelta/2;
			pX3[ i ] = pX[i+1] + dDelta/2;
			pY3[ i ] = pY[i+1] + dDelta/2;
			pX4[ i ] =   pX[i] + dDelta/2;
			pY4[ i ] =   pY[i] - dDelta/2;
		}
		else
		{
			pX1[ i ] =   pX[i] - dDelta/2;
			pY1[ i ] =   pY[i] - dDelta/2;
			pX2[ i ] =   pX[i] - dDelta/2;
			pY2[ i ] =   pY[i] + dDelta/2;
			pX3[ i ] = pX[i+1] + dDelta/2;
			pY3[ i ] = pY[i+1] + dDelta/2;
			pX4[ i ] = pX[i+1] + dDelta/2;
			pY4[ i ] = pY[i+1] - dDelta/2;
		};
	};
	pcTopSize[i].SetdX1( pX[i] );
	pcTopSize[i].SetdX2( pX[0] );
	pcTopSize[i].SetdY1( pY[i] );
	pcTopSize[i].SetdY2( pY[0] );
	pcTopSize[i].SetdSize( sqrt( (pX[i]-pX[0])*(pX[i]-pX[0]) + (pY[i]-pY[0])*(pY[i]-pY[0]) ) );
	if( nType == CTMCRTH_BLCKNTYPE_EPS )
	{
//		pX1[ i ] = pX[0] - dDelta/2;
//		pY1[ i ] = pY[0] - dDelta/2;
//		pX2[ i ] = pX[0] - dDelta/2;
//		pY2[ i ] = pY[0] + dDelta/2;
//		pX3[ i ] = pX[i] + dDelta/2;
//		pY3[ i ] = pY[i] + dDelta/2;
//		pX4[ i ] = pX[i] + dDelta/2;
//		pY4[ i ] = pY[i] - dDelta/2;
		if( fabs( pX[i] - pX[0] ) < fabs( pY[i] - pY[0] ) )
		{
			pX1[ i ] = pX[i] - dDelta/2;
			pY1[ i ] = pY[i] - dDelta/2;
			pX2[ i ] = pX[0] - dDelta/2;
			pY2[ i ] = pY[0] + dDelta/2;
			pX3[ i ] = pX[0] + dDelta/2;
			pY3[ i ] = pY[0] + dDelta/2;
			pX4[ i ] = pX[i] + dDelta/2;
			pY4[ i ] = pY[i] - dDelta/2;
		}
		else
		{
			pX1[ i ] = pX[i] - dDelta/2;
			pY1[ i ] = pY[i] - dDelta/2;
			pX2[ i ] = pX[i] - dDelta/2;
			pY2[ i ] = pY[i] + dDelta/2;
			pX3[ i ] = pX[0] + dDelta/2;
			pY3[ i ] = pY[0] + dDelta/2;
			pX4[ i ] = pX[0] + dDelta/2;
			pY4[ i ] = pY[0] - dDelta/2;
		};
	};

	return;
}

void CTmcBoundaryTpl::SetInputX(CTmcRTH_BlockList *pcBlList, CTmcLibError &cError, double dDelta)
{
	if( cError.IsError() ) return;
	if( pcBlList == NULL )
	{
		cError.PutErrorMessage("pointer for BlockList is NULL");
		return;
	};

	nPoint = 0;
	nTopSize = 0;

	if( pY1 != NULL ) delete []pY1;
	pY1 = NULL;
	if( pY2 != NULL ) delete []pY2;
	pY2 = NULL;
	if( pY3 != NULL ) delete []pY3;
	pY3 = NULL;
	if( pY4 != NULL ) delete []pY4;
	pY4 = NULL;

	if( pX1 != NULL ) delete []pX1;
	pX1 = NULL;
	if( pX2 != NULL ) delete []pX2;
	pX2 = NULL;
	if( pX3 != NULL ) delete []pX3;
	pX3 = NULL;
	if( pX4 != NULL ) delete []pX4;
	pX4 = NULL;

	if( pcTopSize != NULL ) delete []pcTopSize;
	pcTopSize = NULL;

	nPoint = 1;
	pX1 = new double[ nPoint ];
	pX2 = new double[ nPoint ];
	pX3 = new double[ nPoint ];
	pX4 = new double[ nPoint ];
	pY1 = new double[ nPoint ];
	pY2 = new double[ nPoint ];
	pY3 = new double[ nPoint ];
	pY4 = new double[ nPoint ];
	nTopSize = 1;
	pcTopSize = new CTmcTopSize[nTopSize];
	
	if(   (pcTopSize == NULL)||
		  (pX1 == NULL)||(pX2 == NULL)||(pX3 == NULL)||(pX4 == NULL)||
		  (pX1 == NULL)||(pX2 == NULL)||(pX3 == NULL)||(pX4 == NULL) )
	{
		DeleteData();
		cError.PutErrorMessage("memory allocation in BlockList Tpl array");
		return;
	};

	pX1[ 0 ] = - dDelta/2;
	pY1[ 0 ] = pcBlList->GetXmin() - dDelta/2;
	pX2[ 0 ] = - dDelta/2;
	pY2[ 0 ] = pcBlList->GetXmax() + dDelta/2;
	pX3[ 0 ] =   dDelta/2;
	pY3[ 0 ] = pcBlList->GetXmax() + dDelta/2;
	pX4[ 0 ] =   dDelta/2;
	pY4[ 0 ] = pcBlList->GetXmin() - dDelta/2;

	pcTopSize[0].SetdX1( 0.0 );
	pcTopSize[0].SetdX2( 0.0 );
	pcTopSize[0].SetdY1( pcBlList->GetXmin() );
	pcTopSize[0].SetdY2( pcBlList->GetXmax() );
	pcTopSize[0].SetdSize( pcBlList->GetXmax() - pcBlList->GetXmin() );

	return;
}

void CTmcBoundaryTpl::SetInputY(CTmcRTH_BlockList *pcBlList, CTmcLibError &cError, double dDelta)
{
	if( cError.IsError() ) return;
	if( pcBlList == NULL )
	{
		cError.PutErrorMessage("pointer for BlockList is NULL");
		return;
	};

	nPoint = 0;
	nTopSize = 0;

	if( pY1 != NULL ) delete []pY1;
	pY1 = NULL;
	if( pY2 != NULL ) delete []pY2;
	pY2 = NULL;
	if( pY3 != NULL ) delete []pY3;
	pY3 = NULL;
	if( pY4 != NULL ) delete []pY4;
	pY4 = NULL;

	if( pX1 != NULL ) delete []pX1;
	pX1 = NULL;
	if( pX2 != NULL ) delete []pX2;
	pX2 = NULL;
	if( pX3 != NULL ) delete []pX3;
	pX3 = NULL;
	if( pX4 != NULL ) delete []pX4;
	pX4 = NULL;

	if( pcTopSize != NULL ) delete []pcTopSize;
	pcTopSize = NULL;

	nPoint = 1;
	pX1 = new double[ nPoint ];
	pX2 = new double[ nPoint ];
	pX3 = new double[ nPoint ];
	pX4 = new double[ nPoint ];
	pY1 = new double[ nPoint ];
	pY2 = new double[ nPoint ];
	pY3 = new double[ nPoint ];
	pY4 = new double[ nPoint ];
	nTopSize = 1;
	pcTopSize = new CTmcTopSize[nTopSize];
	
	if(   (pcTopSize == NULL)||
		  (pX1 == NULL)||(pX2 == NULL)||(pX3 == NULL)||(pX4 == NULL)||
		  (pX1 == NULL)||(pX2 == NULL)||(pX3 == NULL)||(pX4 == NULL) )
	{
		DeleteData();
		cError.PutErrorMessage("memory allocation in BlockList Tpl array");
		return;
	};

	pX1[ 0 ] = pcBlList->GetXmin() - dDelta/2;
	pY1[ 0 ] = - dDelta/2;
	pX2[ 0 ] = pcBlList->GetXmin() - dDelta/2;
	pY2[ 0 ] = dDelta/2;
	pX3[ 0 ] = pcBlList->GetXmax() + dDelta/2;
	pY3[ 0 ] = dDelta/2;
	pX4[ 0 ] = pcBlList->GetXmax() + dDelta/2;
	pY4[ 0 ] = - dDelta/2;

	pcTopSize[0].SetdX1( pcBlList->GetXmin() );
	pcTopSize[0].SetdX2( pcBlList->GetXmax() );
	pcTopSize[0].SetdY1( 0.0 );
	pcTopSize[0].SetdY2( 0.0 );
	pcTopSize[0].SetdSize( pcBlList->GetXmax() - pcBlList->GetXmin() );

	return;
}


void CTmcBoundaryTpl::SetCircleStat(CTmcRTH_BlockList *pcBlList, CTmcLibError &cError, double dDelta)
{
	if( cError.IsError() ) return;
	if( pcBlList == NULL )
	{
		cError.PutErrorMessage("pointer for BlockList is NULL");
		return;
	};

	int i, n;
	double f, df, x1, x2, y1, y2;

	nPoint = 0;
	nTopSize = 0;

	if( pY1 != NULL ) delete []pY1;
	pY1 = NULL;
	if( pY2 != NULL ) delete []pY2;
	pY2 = NULL;
	if( pY3 != NULL ) delete []pY3;
	pY3 = NULL;
	if( pY4 != NULL ) delete []pY4;
	pY4 = NULL;

	if( pX1 != NULL ) delete []pX1;
	pX1 = NULL;
	if( pX2 != NULL ) delete []pX2;
	pX2 = NULL;
	if( pX3 != NULL ) delete []pX3;
	pX3 = NULL;
	if( pX4 != NULL ) delete []pX4;
	pX4 = NULL;

	if( pcTopSize != NULL ) delete []pcTopSize;
	pcTopSize = NULL;

//	if( fabs( 2*PI - (pcBlList->GetYmax() - pcBlList->GetYmin()) )*pcBlList->GetXmax() <= dDelta ) 
	if( fabs( pcBlList->GetXmin() ) <= dDelta ) 
	{
		nPoint = (int)( pcBlList->GetXmax()*(pcBlList->GetYmax() - pcBlList->GetYmin())/dDelta + 0.5) + 1;
		nTopSize = 1;
		pcTopSize = new CTmcTopSize[nTopSize];
		if( pcTopSize == NULL )
		{
			DeleteData();
			cError.PutErrorMessage("memory allocation in BlockList Tpl array");
			return;
		};
		pcTopSize[0].SetdX1( 0.0 );
		pcTopSize[0].SetdX2( 0.0 );
		pcTopSize[0].SetdY1( 0.0 );
		pcTopSize[0].SetdY2( pcBlList->GetXmax() );
		pcTopSize[0].SetdSize( pcBlList->GetXmax() );
	}
	else
	{
		nPoint = 2 + 
			(int)( pcBlList->GetXmin()*(pcBlList->GetYmax() - pcBlList->GetYmin())/dDelta + 0.5) + 1 +
			(int)( pcBlList->GetXmax()*(pcBlList->GetYmax() - pcBlList->GetYmin())/dDelta + 0.5);
		nTopSize = 2;
		pcTopSize = new CTmcTopSize[nTopSize];
		if( pcTopSize == NULL )
		{
			DeleteData();
			cError.PutErrorMessage("memory allocation in BlockList Tpl array");
			return;
		};
		pcTopSize[0].SetdX1( 0.0 );
		pcTopSize[0].SetdX2( 0.0 );
		pcTopSize[0].SetdY1( 0.0 );
		pcTopSize[0].SetdY2( pcBlList->GetXmax() );
		pcTopSize[0].SetdSize( pcBlList->GetXmax() );
		pcTopSize[1].SetdX1( 0.0 );
		pcTopSize[1].SetdX2( 0.0 );
		pcTopSize[1].SetdY1( - pcBlList->GetXmin() );
		pcTopSize[1].SetdY2( 0.0 );
		pcTopSize[1].SetdSize( pcBlList->GetXmin() );
	};
	if( nPoint == 0 )
	{
		cError.PutErrorMessage("error data for circle Block, nPoint = 0");
		return;
	};

	pX1 = new double[ nPoint ];
	pX2 = new double[ nPoint ];
	pX3 = new double[ nPoint ];
	pX4 = new double[ nPoint ];
	pY1 = new double[ nPoint ];
	pY2 = new double[ nPoint ];
	pY3 = new double[ nPoint ];
	pY4 = new double[ nPoint ];

	if(   (pX1 == NULL)||(pX2 == NULL)||(pX3 == NULL)||(pX4 == NULL)||
		  (pX1 == NULL)||(pX2 == NULL)||(pX3 == NULL)||(pX4 == NULL))
	{
		DeleteData();
		cError.PutErrorMessage("memory allocation in BlockList Tpl array");
		return;
	};


	n = (int)( pcBlList->GetXmax()*(pcBlList->GetYmax() - pcBlList->GetYmin())/dDelta + 0.5) + 1;
	for( i = 0, df = (pcBlList->GetYmax() - pcBlList->GetYmin())/(n-1), f = pcBlList->GetYmin(); i < n; i++, f += df )
	{
		x1 = pcBlList->GetXmax()*cos(f);
		y1 = pcBlList->GetXmax()*sin(f);
		x2 = pcBlList->GetXmax()*cos(f+df);
		y2 = pcBlList->GetXmax()*sin(f+df);
		if( fabs( x1 - x2 ) < fabs( y1 - y2 ) )
		{
//			pX1[ i ] = x1 - dDelta/2;
//			pY1[ i ] = y1 - dDelta/2;
//			pX2[ i ] = x1 - dDelta/2;
//			pY2[ i ] = y1 + dDelta/2;
//			pX3[ i ] = x2 + dDelta/2;
//			pY3[ i ] = y2 + dDelta/2;
//			pX4[ i ] = x2 + dDelta/2;
//			pY4[ i ] = y2 - dDelta/2;
			pX1[ i ] = x1;
			pY1[ i ] = y1;
			pX2[ i ] = x1;
			pY2[ i ] = y2;
			pX3[ i ] = x2;
			pY3[ i ] = y2;
			pX4[ i ] = x2;
			pY4[ i ] = y1;
		}
		else
		{
//			pX1[ i ] = x1 - dDelta/2;
//			pY1[ i ] = y1 - dDelta/2;
//			pX2[ i ] = x2 - dDelta/2;
//			pY2[ i ] = y2 + dDelta/2;
//			pX3[ i ] = x2 + dDelta/2;
//			pY3[ i ] = y2 + dDelta/2;
//			pX4[ i ] = x1 + dDelta/2;
//			pY4[ i ] = y1 - dDelta/2;
			pX1[ i ] = x1;
			pY1[ i ] = y1;
			pX2[ i ] = x1;
			pY2[ i ] = y2;
			pX3[ i ] = x2;
			pY3[ i ] = y2;
			pX4[ i ] = x2;
			pY4[ i ] = y1;
		};
//		pX1[ i ] = x1 - dDelta/2;
//		pY1[ i ] = y1 - dDelta/2;
//		pX2[ i ] = x2 - dDelta/2;
//		pY2[ i ] = y2 + dDelta/2;
//		pX3[ i ] = x2 + dDelta/2;
//		pY3[ i ] = y2 + dDelta/2;
//		pX4[ i ] = x1 + dDelta/2;
//		pY4[ i ] = y1 - dDelta/2;
	};

//	if( fabs( 2*PI - (pcBlList->GetYmax() - pcBlList->GetYmin()) )*pcBlList->GetXmax() <= dDelta ) return;
	if( fabs( pcBlList->GetXmin() ) <= dDelta ) return;
/*
	x1 = pcBlList->GetXmin()*cos(pcBlList->GetYmin());
	y1 = pcBlList->GetXmin()*sin(pcBlList->GetYmin());
	x2 = pcBlList->GetXmax()*cos(pcBlList->GetYmin());
	y2 = pcBlList->GetXmax()*sin(pcBlList->GetYmin());
	pX1[ i ] = x1 - dDelta/2;
	pY1[ i ] = y1 - dDelta/2;
	pX2[ i ] = x2 - dDelta/2;
	pY2[ i ] = y2 + dDelta/2;
	pX3[ i ] = x2 + dDelta/2;
	pY3[ i ] = y2 + dDelta/2;
	pX4[ i ] = x1 + dDelta/2;
	pY4[ i ] = y1 - dDelta/2;
	i++;

	x1 = pcBlList->GetXmin()*cos(pcBlList->GetYmax());
	y1 = pcBlList->GetXmin()*sin(pcBlList->GetYmax());
	x2 = pcBlList->GetXmax()*cos(pcBlList->GetYmax());
	y2 = pcBlList->GetXmax()*sin(pcBlList->GetYmax());
	pX1[ i ] = x1 - dDelta/2;
	pY1[ i ] = y1 - dDelta/2;
	pX2[ i ] = x2 - dDelta/2;
	pY2[ i ] = y2 + dDelta/2;
	pX3[ i ] = x2 + dDelta/2;
	pY3[ i ] = y2 + dDelta/2;
	pX4[ i ] = x1 + dDelta/2;
	pY4[ i ] = y1 - dDelta/2;
	i++;
*/
	x1 = pcBlList->GetXmax()*cos(pcBlList->GetYmin());
	y1 = pcBlList->GetXmax()*sin(pcBlList->GetYmin());
	x2 = pcBlList->GetXmin()*cos(pcBlList->GetYmin());
	y2 = pcBlList->GetXmin()*sin(pcBlList->GetYmin());
	if( fabs( x1 - x2 ) < fabs( y1 - y2 ) )
	{
		pX1[ i ] = x1 - dDelta/2;
		pY1[ i ] = y1 - dDelta/2;
		pX2[ i ] = x2 - dDelta/2;
		pY2[ i ] = y2 + dDelta/2;
		pX3[ i ] = x2 + dDelta/2;
		pY3[ i ] = y2 + dDelta/2;
		pX4[ i ] = x1 + dDelta/2;
		pY4[ i ] = y1 - dDelta/2;
	}
	else
	{
		pX1[ i ] = x1 - dDelta/2;
		pY1[ i ] = y1 - dDelta/2;
		pX2[ i ] = x1 - dDelta/2;
		pY2[ i ] = y1 + dDelta/2;
		pX3[ i ] = x2 + dDelta/2;
		pY3[ i ] = y2 + dDelta/2;
		pX4[ i ] = x2 + dDelta/2;
		pY4[ i ] = y2 - dDelta/2;
	};
	i++;

	x1 = pcBlList->GetXmin()*cos(pcBlList->GetYmax());
	y1 = pcBlList->GetXmin()*sin(pcBlList->GetYmax());
	x2 = pcBlList->GetXmax()*cos(pcBlList->GetYmax());
	y2 = pcBlList->GetXmax()*sin(pcBlList->GetYmax());
	if( fabs( x1 - x2 ) < fabs( y1 - y2 ) )
	{
		pX1[ i ] = x1 - dDelta/2;
		pY1[ i ] = y1 - dDelta/2;
		pX2[ i ] = x2 - dDelta/2;
		pY2[ i ] = y2 + dDelta/2;
		pX3[ i ] = x2 + dDelta/2;
		pY3[ i ] = y2 + dDelta/2;
		pX4[ i ] = x1 + dDelta/2;
		pY4[ i ] = y1 - dDelta/2;
	}
	else
	{
		pX1[ i ] = x1 - dDelta/2;
		pY1[ i ] = y1 - dDelta/2;
		pX2[ i ] = x1 - dDelta/2;
		pY2[ i ] = y1 + dDelta/2;
		pX3[ i ] = x2 + dDelta/2;
		pY3[ i ] = y2 + dDelta/2;
		pX4[ i ] = x2 + dDelta/2;
		pY4[ i ] = y2 - dDelta/2;
	};
	i++;

	n = (int)( pcBlList->GetXmin()*(pcBlList->GetYmax() - pcBlList->GetYmin())/dDelta + 0.5);
	if( n < 1 ) return;
	if( n < 2 ) 
	{
		f = pcBlList->GetYmin();
		df = (pcBlList->GetYmax() - pcBlList->GetYmin());
		x1 = pcBlList->GetXmin()*cos(f);
		y1 = pcBlList->GetXmin()*sin(f);
		x2 = pcBlList->GetXmin()*cos(f+df);
		y2 = pcBlList->GetXmin()*sin(f+df);
		pX1[ i ] = x1;
		pY1[ i ] = y1;
		pX2[ i ] = x1;
		pY2[ i ] = y2;
		pX3[ i ] = x2;
		pY3[ i ] = y2;
		pX4[ i ] = x2;
		pY4[ i ] = y1;
		return;
	};
	for( df = (pcBlList->GetYmax() - pcBlList->GetYmin())/(n-1), f = pcBlList->GetYmin(); i < nPoint; i++, f += df )
	{
		x1 = pcBlList->GetXmin()*cos(f);
		y1 = pcBlList->GetXmin()*sin(f);
		x2 = pcBlList->GetXmin()*cos(f+df);
		y2 = pcBlList->GetXmin()*sin(f+df);
		pX1[ i ] = x1;
		pY1[ i ] = y1;
		pX2[ i ] = x1;
		pY2[ i ] = y2;
		pX3[ i ] = x2;
		pY3[ i ] = y2;
		pX4[ i ] = x2;
		pY4[ i ] = y1;
	};

	return;
}


int CTmcBoundaryTpl::GetnType()
{
	return nType;
}

int CTmcBoundaryTpl::GetnPoint()
{
	return nPoint;
}

int CTmcBoundaryTpl::GetnBlock()
{
	return nBlock;
}

double CTmcBoundaryTpl::GetdY0()
{
	return dY0;
}

double CTmcBoundaryTpl::GetdX0()
{
	return dX0;
}

CString CTmcBoundaryTpl::GetcsEpsExpr()
{
	return csEpsExpr;
}

CString CTmcBoundaryTpl::GetcsBlock()
{
	return csBlock;
}

double * CTmcBoundaryTpl::GetpdX1()
{
	return pX1;
}

double * CTmcBoundaryTpl::GetpdX2()
{
	return pX2;
}

double * CTmcBoundaryTpl::GetpdX3()
{
	return pX3;
}

double * CTmcBoundaryTpl::GetpdX4()
{
	return pX4;
}

double * CTmcBoundaryTpl::GetpdY1()
{
	return pY1;
}

double * CTmcBoundaryTpl::GetpdY2()
{
	return pY2;
}

double * CTmcBoundaryTpl::GetpdY3()
{
	return pY3;
}

double * CTmcBoundaryTpl::GetpdY4()
{
	return pY4;
}

void CTmcBoundaryTpl::TranslateX0Y0()
{
	if( nPoint < 1 ) return;
	if(   (pX1 == NULL)||(pX2 == NULL)||(pX3 == NULL)||(pX4 == NULL)||
		  (pX1 == NULL)||(pX2 == NULL)||(pX3 == NULL)||(pX4 == NULL)) return;
	
	int i;
	
	for( i = 0; i < nPoint; i++ )
	{
		
		pX1[i] += dX0;
		pX2[i] += dX0;
		pX3[i] += dX0;
		pX4[i] += dX0;

		pY1[i] += dY0;
		pY2[i] += dY0;
		pY3[i] += dY0;
		pY4[i] += dY0;

	};

	if( nTopSize < 1 ) return;
	if( pcTopSize == NULL ) return;
	
	for( i = 0; i < nTopSize; i++ )
	{
		pcTopSize[i].TranslateX0Y0( dX0, dY0 );
	};

	return;
}

int CTmcBoundaryTpl::GetnTopSize()
{
	return nTopSize;
}

CTmcTopSize * CTmcBoundaryTpl::GetpcTopSize()
{
	return pcTopSize;
}

CTmcTopSize& CTmcBoundaryTpl::GetcTopSizeX0()
{
	return cTopSizeLinklListX0;
}

CTmcTopSize& CTmcBoundaryTpl::GetcTopSizeY0()
{
	return cTopSizeLinklListY0;
}

int CTmcBoundaryTpl::GetnInputNum()
{
	return nInputNum;
}

void CTmcBoundaryTpl::SetnInputNum(int n)
{
	nInputNum = n;
	return;
}
