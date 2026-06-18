// TmcRTHNodeDiel.cpp : implementation file
//

#include "stdafx.h"
#include "PlanRT_H.h"
#include <tmcgrviw.h>
#include "TmcRTHNodeDiel.h"
#include "TmcRTH_Indan.h"
#include "TmcRTHRectNode.h"
#include <math.h>
#include <limits.h>
#include <float.h>
#include <expr.h>
#include "Pl_iofor.h"

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
	pbIsStop = NULL;
	csFileNameEps.Format("");
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
	
	return;
}

CTmcLibError &CTmcRTHNodeDiel::SetBorderXmin( int nX, int nY )
{
	DeleteData();
	
	nNumNode = nY;
	if( nNumNode < 1 )
	{
		cError.PutErrorMessage("nNumNode < 1 for Xmin border");
		return cError;
	};
	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		cError.PutErrorMessage("Not enough memory for Xmin border");
		return cError;
	};
	
	int	i, ii;
//	CString csBuf1;
	for( i = 0, ii = 0; i < nNumNode; i++, ii+=nX )
	{
		pcNodeDielOne[i].nNodeGlobal = ii;
//{
//	csBuf1.Format("{nNodeGlobal=%d;}", ii );
//	AfxMessageBox(csBuf1);
//};
	};

	SetnType();
	return cError;
}

CTmcLibError &CTmcRTHNodeDiel::SetBorderXmax( int nX, int nY )
{
	DeleteData();
	
	nNumNode = nY;
	if( nNumNode < 1 )
	{
		cError.PutErrorMessage("nNumNode < 1 for Xmax border");
		return cError;
	};
	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		cError.PutErrorMessage("Not enough memory for Xmax border");
		return cError;
	};
	
	int	i, ii;

	for( i = 0, ii = nX-1; i < nNumNode; i++, ii+=nX )
	{
		pcNodeDielOne[i].nNodeGlobal = ii;
	};

	SetnType();
	return cError;
}

CTmcLibError &CTmcRTHNodeDiel::SetBorderYmin( int nX )
{
	DeleteData();
	
	nNumNode = nX;
	if( nNumNode < 1 )
	{
		cError.PutErrorMessage("nNumNode < 1 for Ymin border");
		return cError;
	};
	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		cError.PutErrorMessage("Not enough memory for Ymin border");
		return cError;
	};
	
	int	i, ii;

	for( i = 0, ii = 0; i < nNumNode; i++, ii++ )
	{
		pcNodeDielOne[i].nNodeGlobal = ii;
	};

	SetnType();
	return cError;
}

CTmcLibError &CTmcRTHNodeDiel::SetBorderYmax( int nX, int nY )
{
	DeleteData();
	
	nNumNode = nX;
	if( nNumNode < 1 )
	{
		cError.PutErrorMessage("nNumNode < 1 for Ymax border");
		return cError;
	};
	if( (pcNodeDielOne = new sTmcRTHNodeDielOne[nNumNode]) == NULL )
	{
		cError.PutErrorMessage("Not enough memory for Ymax border");
		return cError;
	};
	
	int	i, ii;

	for( i = 0, ii = nX*nY - nX; i < nNumNode; i++, ii++ )
	{
		pcNodeDielOne[i].nNodeGlobal = ii;
	};

	SetnType();
	return cError;
}


CTmcLibError &CTmcRTHNodeDiel::SetBlock( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut, _real *prYNode, volatile BOOL *pbStopFlag, CString csCurrentPath )
{
	DeleteData();
	double r;
	int i;

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
	switch( nType )
	{
		case  CTMCRTH_BLCKNTYPE_METAL    :
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTAT, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == NULL ) 
			{
				cError = SetRectMetal( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTT, strlen(CTMCRTH_INDANBLCK_POLYGSTT) ) == NULL )
			{
				cError = SetPolyMetal( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTAT, strlen(CTMCRTH_INDANBLCK_CIRCSTAT) ) == NULL )
			{
				cError = SetCircMetal( pcNextBlockList, cParam);
				i = 1;
			};
			break;
		case  CTMCRTH_BLCKNTYPE_ABSORBER :
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTAT, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == NULL ) 
			{
				cError = SetRectAbsorber( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTT, strlen(CTMCRTH_INDANBLCK_POLYGSTT) ) == NULL ) 
			{
				cError = SetPolyAbsorber( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTAT, strlen(CTMCRTH_INDANBLCK_CIRCSTAT) ) == NULL )
			{
				cError = SetCircAbsorber( pcNextBlockList, cParam);
				i = 1;
			};
			break;
		case  CTMCRTH_BLCKNTYPE_MAGNETIC :
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTAT, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == NULL ) 
			{
				cError = SetRectMagnetic( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTT, strlen(CTMCRTH_INDANBLCK_POLYGSTT) ) == NULL ) 
			{
				cError = SetPolyMagnetic( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTAT, strlen(CTMCRTH_INDANBLCK_CIRCSTAT) ) == NULL )
			{
				cError = SetCircMagnetic( pcNextBlockList, cParam);
				i = 1;
			};
			break;
		case  CTMCRTH_BLCKNTYPE_EPS      :
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_FILE, strlen(CTMCRTH_INDANBLCK_FILE) ) == NULL ) 
			{
				cError = SetRectFile( pcNextBlockList, cParam, csCurrentPath );
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_FILEN, strlen(CTMCRTH_INDANBLCK_FILEN) ) == NULL ) 
			{
				cError = SetRectFile( pcNextBlockList, cParam, csCurrentPath );
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_FILEB, strlen(CTMCRTH_INDANBLCK_FILEB) ) == NULL ) 
			{
				cError = SetRectFile( pcNextBlockList, cParam, csCurrentPath );
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_FILEY, strlen(CTMCRTH_INDANBLCK_FILEY) ) == NULL ) 
			{
				cError = SetRectFile( pcNextBlockList, cParam, csCurrentPath );
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTAT, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == NULL ) 
			{
				cError = SetRect( pcNextBlockList, cParam, csCurrentPath );
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTATN, strlen(CTMCRTH_INDANBLCK_RECTSTATN) ) == NULL ) 
			{
				cError = SetRect( pcNextBlockList, cParam, csCurrentPath );
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTATB, strlen(CTMCRTH_INDANBLCK_RECTSTATB) ) == NULL ) 
			{
				cError = SetRect( pcNextBlockList, cParam, csCurrentPath );
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTATY, strlen(CTMCRTH_INDANBLCK_RECTSTATY) ) == NULL ) 
			{
				cError = SetRect( pcNextBlockList, cParam, csCurrentPath );
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTMOVE, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == NULL )
			{
				cError = SetRect( pcNextBlockList, cParam, csCurrentPath );
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTAT, strlen(CTMCRTH_INDANBLCK_CIRCSTAT) ) == NULL )
			{
				cError = SetCirc( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTATN, strlen(CTMCRTH_INDANBLCK_CIRCSTATN) ) == NULL )
			{
				cError = SetCirc( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTATB, strlen(CTMCRTH_INDANBLCK_CIRCSTATB) ) == NULL )
			{
				cError = SetCirc( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTATY, strlen(CTMCRTH_INDANBLCK_CIRCSTATY) ) == NULL )
			{
				cError = SetCirc( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCMOVE, strlen(CTMCRTH_INDANBLCK_CIRCMOVE) ) == NULL )
			{
				cError = SetCirc( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTT, strlen(CTMCRTH_INDANBLCK_POLYGSTT) ) == NULL )
			{
				cError = SetPoly( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTTN, strlen(CTMCRTH_INDANBLCK_POLYGSTTN) ) == NULL )
			{
				cError = SetPoly( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTTB, strlen(CTMCRTH_INDANBLCK_POLYGSTTB) ) == NULL )
			{
				cError = SetPoly( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTTY, strlen(CTMCRTH_INDANBLCK_POLYGSTTY) ) == NULL )
			{
				cError = SetPoly( pcNextBlockList, cParam);
				i = 1;
			};
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGMOV, strlen(CTMCRTH_INDANBLCK_POLYGMOV) ) == NULL )
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
		csBuf.Format("Set block %d; points:%d; Type or eps = {%s}", nBlock, nNumNode, csEpsExpr);
		PutTrace(csBuf);
	};
	DeleteDublicateData();
	CalculatedXYCenter( cParam );
	SetnType();
	SetYLineForDiel( cParam, prYNode );
	SetYForInput( cParam );
	pcNextBlockList->SetnMemory( sizeof(CTmcRTHNodeDiel)+(sizeof(sTmcRTHNodeDielOne))*nNumNode );
	DeleteStatDielData();
//	{
//		CString csBuf1;
//		csBuf1.Format("YRR");
//		AfxMessageBox(csBuf1);
//	};
	{
		CString csBuf;
		csBuf.Format(" ");
		PutStatistics(csBuf);
	};
	return cError;
}

CTmcLibError &CTmcRTHNodeDiel::SetRect( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CString csCurrentPath )
{
	SaveFileRect( pcNextBlockList, cParam, csCurrentPath);
	
	if( cError.IsError() ) return cError;
	if( pcNextBlockList == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return cError;
	};

	int nXmin, nXmax, nYmin, nYmax;
	int i, j, nX, nY, ii;
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

	nNumNode = nX*nY;
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

	for( i = 0, ii = 0, x = dXmin; i < nX; i++, x = dXmin + (i*cParam.rDelta) )
	{
//	{
//		CString csBuf1;
//		csBuf1.Format("X = %lg", x);
//		AfxMessageBox(csBuf1);
//	};
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
	int i, j, nX, nY, ii;
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
	if( dFImax > (2*3.141592653589) )
	{
		i = (int)(dFImax/(2*3.141592653589));
		dFImax -= i*(2*3.141592653589);
	};

//	{
//		CString csBuf1;
//		csBuf1.Format("dFImin = %lg; dFImax = %lg", dFImin, dFImax );
//		AfxMessageBox(csBuf1);
//	};

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
				x1 = x - cParam.rDelta;
				y1 = y - cParam.rDelta;
				rRxy1 = x1*x1 + y1*y1;
				rFxy1 = atan2__1( y1, x1);
				if( ( rRxy1 >= dRRmin )&&( rRxy1 <= dRRmax)&&
					( rFxy1 >= dFImin )&&( rFxy1 <= dFImax ))
				{
					pcNodeDielOne[ii].rY += (_real)(0.25);
				};
				x1 = x - cParam.rDelta;
				y1 = y + cParam.rDelta;
				rRxy1 = x1*x1 + y1*y1;
				rFxy1 = atan2__1( y1, x1);
				if( ( rRxy1 >= dRRmin )&&( rRxy1 <= dRRmax)&&
					( rFxy1 >= dFImin )&&( rFxy1 <= dFImax ))
				{
					pcNodeDielOne[ii].rY += (_real)(0.25);
				};
				x1 = x + cParam.rDelta;
				y1 = y + cParam.rDelta;
				rRxy1 = x1*x1 + y1*y1;
				rFxy1 = atan2__1( y1, x1);
				if( ( rRxy1 >= dRRmin )&&( rRxy1 <= dRRmax)&&
					( rFxy1 >= dFImin )&&( rFxy1 <= dFImax ))
				{
					pcNodeDielOne[ii].rY += (_real)(0.25);
				};
				x1 = x + cParam.rDelta;
				y1 = y - cParam.rDelta;
				rRxy1 = x1*x1 + y1*y1;
				rFxy1 = atan2__1( y1, x1);
				if( ( rRxy1 >= dRRmin )&&( rRxy1 <= dRRmax)&&
					( rFxy1 >= dFImin )&&( rFxy1 <= dFImax ))
				{
					pcNodeDielOne[ii].rY += (_real)(0.25);
				};
//	{
//		CString csBuf1;
//		csBuf1.Format("pcNodeDielOne[%d] = %lg;", ii, pcNodeDielOne[ii].rY );
//		AfxMessageBox(csBuf1);
//	};
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
	int i, j, k, nX, nY, ii, k1;
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

//	{
//		CString csBuf1;
//		csBuf1.Format("dXmin = %lg; dXmax = %lg; dYmin = %lg; dYmax = %lg; nNumNode = %d; nX = %d; nY = %d;", dXmin, dXmax, dYmin, dYmax, nNumNode, nX, nY );
//		AfxMessageBox(csBuf1);
//	};

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
					x1 = x - cParam.rDelta;
					y1 = y - cParam.rDelta;
					for( k1 = 2; k1 < nXY; k1++ )
					{
						if( IsPointInTr( pX[0], pY[0], pX[k1-1], pY[k1-1], pX[k1], pY[k1], x1, y1 ) ) 
						{
							pcNodeDielOne[ii].rY += (_real)(0.25);
							k1 = nXY;
						};
					};
					x1 = x - cParam.rDelta;
					y1 = y + cParam.rDelta;
					for( k1 = 2; k1 < nXY; k1++ )
					{
						if( IsPointInTr( pX[0], pY[0], pX[k1-1], pY[k1-1], pX[k1], pY[k1], x1, y1 ) ) 
						{
							pcNodeDielOne[ii].rY += (_real)(0.25);
							k1 = nXY;
						};
					};
					x1 = x + cParam.rDelta;
					y1 = y + cParam.rDelta;
					for( k1 = 2; k1 < nXY; k1++ )
					{
						if( IsPointInTr( pX[0], pY[0], pX[k1-1], pY[k1-1], pX[k1], pY[k1], x1, y1 ) ) 
						{
							pcNodeDielOne[ii].rY += (_real)(0.25);
							k1 = nXY;
						};
					};
					x1 = x + cParam.rDelta;
					y1 = y - cParam.rDelta;
					for( k1 = 2; k1 < nXY; k1++ )
					{
						if( IsPointInTr( pX[0], pY[0], pX[k1-1], pY[k1-1], pX[k1], pY[k1], x1, y1 ) ) 
						{
							pcNodeDielOne[ii].rY += (_real)(0.25);
							k1 = nXY;
						};
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

	int i;

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
	int j, nY;
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
/*
		pcNodeDielOne[j].nNodeGlobal = cParam.CalcnNodeGlobal( (double)cParam.rXmin, (double)y );
*/
		pcNodeDielOne[j].nNodeGlobal = cParam.CalcnNodeGlobal( (double)(pcNextBlockList->GetX0()), (double)y );
		pcNodeDielOne[j].rU  = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[j].rU1 = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[j].rU2 = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[j].rU3 = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[j].rU_Cur = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[j].rU_Cur1 = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[j].rU_Cur2 = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[j].rU_Cur  = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[j].rY = (_real)(0.0);
	};

	csBlock = cOut->csFileName;

	csVx = pcNextBlockList->GetcsVx();
	csVy = pcNextBlockList->GetcsVy();
	csW  = pcNextBlockList->GetcsW();

	CalculateYForInput( cParam.rFreq, cParam.rDelta );

	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == NULL)
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
	int j, nY;
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
/*
		pcNodeDielOne[j].nNodeGlobal = cParam.CalcnNodeGlobal( (double)cParam.rXmax, (double)y );
*/
		pcNodeDielOne[j].nNodeGlobal = cParam.CalcnNodeGlobal( (double)(pcNextBlockList->GetX0()), (double)y );
		pcNodeDielOne[j].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[j].rY = (_real)(0.0);
	};

	csBlock = cOut->csFileName;

	csVx = pcNextBlockList->GetcsVx();
	csVy = pcNextBlockList->GetcsVy();
	csW  = pcNextBlockList->GetcsW();

	CalculateYForInput( cParam.rFreq, cParam.rDelta );


	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == NULL)
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
	int j, nX;
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
/*
		pcNodeDielOne[j].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)cParam.rYmax );
*/
		pcNodeDielOne[j].nNodeGlobal = cParam.CalcnNodeGlobal( (double)x, (double)(pcNextBlockList->GetY0()) );
		pcNodeDielOne[j].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[j].rY = (_real)(0.0);
	};

	csBlock = cOut->csFileName;

	csVx = pcNextBlockList->GetcsVx();
	csVy = pcNextBlockList->GetcsVy();
	csW  = pcNextBlockList->GetcsW();

	CalculateYForInput( cParam.rFreq, cParam.rDelta );

	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == NULL)
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
	int j, nX;
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
/*
		pcNodeDielOne[j].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)cParam.rYmin );
*/
		pcNodeDielOne[j].nNodeGlobal = cParam.CalcnNodeGlobal( (double)x, (double)(pcNextBlockList->GetY0()) );
		pcNodeDielOne[j].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[j].rY = (_real)(0.0);
	};

	csBlock = cOut->csFileName;

	csVx = pcNextBlockList->GetcsVx();
	csVy = pcNextBlockList->GetcsVy();
	csW  = pcNextBlockList->GetcsW();

	CalculateYForInput( cParam.rFreq, cParam.rDelta );

	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == NULL)
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
//	{
//		CString csBuf1;
//		csBuf1.Format("pcNodeDielOne[%d]%d(%lg;%lg) = %lg", i, nNode, x, y, y_line );
//		csBuf1.Format("pcNodeDielOne");
//		AfxMessageBox(csBuf1);
//	};
	if( cError.IsError() ) return;
	if( nNumNode == 0 ) return;
	if( pcNodeDielOne == NULL ) return;
	if( (nType != CTMCRTH_BLCKNTYPE_EPS) )	return;

	CString csBuf;
	char szBuf[CTMCRTH_INDANSBUF];
	char szBuf1[CTMCRTH_INDANSBUF];
	int i, nNode, ii, i_Type;
	double x, y, y_line, dDelta;
	BOOL bIsFile;
	
	if( strncmp( csBlock, CTMCRTH_INDANBLCK_FILE, strlen(CTMCRTH_INDANBLCK_FILE) ) == NULL )
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
	
	csBuf.Format("Set block %d; points:%d; eps = {%s}", nBlock, nNumNode, csEpsExpr);
	PutTrace(csBuf);

	for( i = 0, dDelta = cParam.rDelta/2., ii = 0; i < nNumNode; i++, ii++ )
	{
		if( cError.IsError() ) return;
		if( ii == 10000 )
		{
			csBuf.Format("n:%d", i);
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
//if( fabs(y) < FLT_MIN )
//	{
//		CString csBuf1;
//		csBuf1.Format("pcNodeDielOne[%d]%d(%lg;%lg) = %lg*%lg", i, nNode, x, y, pcNodeDielOne[i].rY, CalcEps( x, y, cParam) );
//		AfxMessageBox(csBuf1);
//	};
			y_line = pcNodeDielOne[i].rY *( CalcEps( x, y, cParam) );
		};
		pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_EPS;

		i_Type = 0;
///////////////////////////////////////////////
//		old type data eps+ write in prYNode[0]
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_FILE, strlen(CTMCRTH_INDANBLCK_FILE) ) == NULL ) 
		{
			i_Type = 0;
		};
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTAT, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == NULL ) 
		{
			i_Type = 0;
		};
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTAT, strlen(CTMCRTH_INDANBLCK_CIRCSTAT) ) == NULL )
		{
			i_Type = 0;
		};
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTT, strlen(CTMCRTH_INDANBLCK_POLYGSTT) ) == NULL )
		{
			i_Type = 0;
		};
///////////////////////////////////////////////
//		new type data N write in prYNode[1]
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_FILEN, strlen(CTMCRTH_INDANBLCK_FILEN) ) == NULL ) 
		{
			i_Type = 1;
		};
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTATN, strlen(CTMCRTH_INDANBLCK_RECTSTATN) ) == NULL ) 
		{
			i_Type = 1;
		};
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTATN, strlen(CTMCRTH_INDANBLCK_CIRCSTATN) ) == NULL )
		{
			i_Type = 1;
		};
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTTN, strlen(CTMCRTH_INDANBLCK_POLYGSTTN) ) == NULL )
		{
			i_Type = 1;
		};
///////////////////////////////////////////////
//		new type data Y write in prYNode[2]
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_FILEY, strlen(CTMCRTH_INDANBLCK_FILEY) ) == NULL ) 
		{
			i_Type = 2;
		};
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTATY, strlen(CTMCRTH_INDANBLCK_RECTSTATY) ) == NULL ) 
		{
			i_Type = 2;
		};
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTATY, strlen(CTMCRTH_INDANBLCK_CIRCSTATY) ) == NULL )
		{
			i_Type = 2;
		};
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTTY, strlen(CTMCRTH_INDANBLCK_POLYGSTTY) ) == NULL )
		{
			i_Type = 2;
		};
///////////////////////////////////////////////
//		new type data B write in prYNode[4]
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_FILEB, strlen(CTMCRTH_INDANBLCK_FILEB) ) == NULL ) 
		{
			i_Type = 4;
		};
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTATB, strlen(CTMCRTH_INDANBLCK_RECTSTATB) ) == NULL ) 
		{
			i_Type = 4;
		};
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCSTATB, strlen(CTMCRTH_INDANBLCK_CIRCSTATB) ) == NULL )
		{
			i_Type = 4;
		};
		if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGSTTB, strlen(CTMCRTH_INDANBLCK_POLYGSTTB) ) == NULL )
		{
			i_Type = 4;
		};
///////////////////////////////////////////////

	prYNode[ 6*nNode + i_Type ] += (_real)(y_line);

//	{
//		CString csBuf1;
//		csBuf1.Format("pcNodeDielOne[%d]%d(%lg;%lg) = %lg", i, nNode, x, y, y_line );
//		AfxMessageBox(csBuf1);
//	};

	};

	return;
}


void CTmcRTHNodeDiel::SetnType( void )
{
	if( cError.IsError() ) return;
	if( nNumNode == 0 ) return;
	if( pcNodeDielOne == NULL ) return;
	
	int i;

	{
		CString csBuf;
		csBuf.Format("SetnType");
		PutStatistics(csBuf);
	};
	for( i = 0; i < nNumNode; i++ )
	{
		if( (nType != CTMCRTH_BLCKNTYPE_ABSORBER)&&(nType != CTMCRTH_BLCKNTYPE_MAGNETIC) ) pcNodeDielOne[i].nType = nType;
		pcNodeDielOne[i].rU  = 0.0;
		pcNodeDielOne[i].rU1 = 0.0;
		pcNodeDielOne[i].rU2 = 0.0;
		pcNodeDielOne[i].rU3 = 0.0;
		pcNodeDielOne[i].rU_Cur = 0.0;
		pcNodeDielOne[i].rU_Cur1 = 0.0;
		pcNodeDielOne[i].rU_Cur2 = 0.0;
		pcNodeDielOne[i].rU_Cur3 = 0.0;
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
	int i;

	if( (szBuf == NULL)||(szBuf1 == NULL) )
	{
		csBuf.Format("Not enought memory for SetYLineForDiel");
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csEpsExpr );

	csBuf.Format("Inp:%d", nBlock);
	PutStatistics(csBuf);

	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == NULL)
	{
//		dUnormir = sqrt(1./(dY1_InputAdmitance*(nNumNode-1)*cParam.rFreq/299792458*cParam.dGetDelta()*cParam.dGetDelta()));
//		for( i = 0; i < nNumNode; i++ )
//		{
////			pcNodeDielOne[i].rY = (_real)(1.0);
//			pcNodeDielOne[i].rY = (_real)(dUnormir);
//		};
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
	sprintf( szBuf1, "x=(%lg);y=(%lg);r=(%lg);f=(%lg);", x, y, r, f);
	if( i1nte_atof_1( szBuf1, &r1 ) != 0 )
	{
		csBuf.Format("In{%s}%s", szBuf1, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return (_real)(0.0);
	};
	if( i1nte_atof_1( szBuf, &r1 ) != 0 )
	{
		csBuf.Format("In{%s}%s; r=%lg;f=%lg; x=%lg; y=%lg",
			szBuf, expr_get_error(), r, f, x, y );
		cError.PutErrorMessage(csBuf);
		return (_real)(0.0);
	};
	rr = (_real)r1;
//if( fabs(y) < FLT_MIN )
//	{
//		CString csBuf1;
//		csBuf1.Format("%s; Expr(%s)=%lg", szBuf1, szBuf, r1 );
//		AfxMessageBox(csBuf1);
//	};

	return rr;
}


void CTmcRTHNodeDiel::SetBlockInDielList( sTmcRTH_DielNodeList **  pcNodeDielList, int nNumNode1, CTmcLibError &cError1 )
{
	if( cError1.IsError() ) return;
	if( cError.IsError() )
	{
		cError1 = cError;
		return;
	};
	if( nNumNode1 == 0 ) return;
	if( nNumNode == 0 ) return;
	if( pcNodeDielList == NULL ) return;
	if( pcNodeDielOne == NULL ) return;

	int i;
	CString csBuf;

	for( i = 0; i < nNumNode; i++)
	{
		if( pcNodeDielOne[i].nNodeGlobal >= nNumNode1 )
		{
			csBuf.Format("Set Diel array in NodeList nNodeGlobal = %d > nNodeGlobalMax = %d",
				pcNodeDielOne[i].nNodeGlobal, nNumNode1);
			cError.PutErrorMessage(csBuf);
			cError1 = cError;
			return;
		};
		AddList( &(pcNodeDielOne[i]), &(pcNodeDielList[ pcNodeDielOne[i].nNodeGlobal ]) );
		if( cError.IsError() )
		{
			cError1 = cError;
			return;
		};
	};

	cError1 = cError;
	return;
}

void CTmcRTHNodeDiel::AddList( sTmcRTHNodeDielOne *pcNodeDielOne1, sTmcRTH_DielNodeList **pcNodeDielList1 )
{
	if( cError.IsError() ) return;
	if( pcNodeDielList1 == NULL ) return;
	if( pcNodeDielOne1 == NULL ) return;

//	CString csBuf1;
	CString csBuf;
	sTmcRTH_DielNodeList * pcDielNodeNextList;
//	csBuf1.Format("{%d}", pcNodeDielOne1->nType );
//	AfxMessageBox(csBuf1);
	if( pcNodeDielList1[0] == NULL )
	{
		if( (pcNodeDielList1[0] = new sTmcRTH_DielNodeList[1]) == NULL )
		{
			cError.PutErrorMessage("Not enought memory for NodeDielList");
			return;
		};
		pcNodeDielList1[0]->pcDielNodeNextList = NULL;
		pcNodeDielList1[0]->pcNodeDielOne = pcNodeDielOne1;
		return;
	};
	switch( pcNodeDielList1[0]->pcNodeDielOne->nType )
	{
		case  CTMCRTH_BLCKNTYPE_INPXLEFT  : ;
		case  CTMCRTH_BLCKNTYPE_INPXRIGHT : ;
		case  CTMCRTH_BLCKNTYPE_INPYTOP   : ;
		case  CTMCRTH_BLCKNTYPE_INPYBOT   : ;
			if( (pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPXLEFT )||
				(pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPXRIGHT)||
				(pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPYTOP  )||
				(pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPYBOT  ))
			{
				csBuf.Format("Dublicated inputs in AddList");
				cError.PutErrorMessage(csBuf);
				return;
			};
			AddList( pcNodeDielOne1, &(pcNodeDielList1[0]->pcDielNodeNextList) );
			break;
		case  CTMCRTH_BLCKNTYPE_METAL   : ;
			if( (pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPXLEFT )||
				(pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPXRIGHT)||
				(pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPYTOP  )||
				(pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPYBOT  ))
			{
				pcDielNodeNextList = pcNodeDielList1[0];
				pcNodeDielList1[0] = NULL;
				AddList( pcNodeDielOne1, &(pcNodeDielList1[0]) );
				pcNodeDielList1[0]->pcDielNodeNextList = pcDielNodeNextList;
			}
			else
			{
				AddList( pcNodeDielOne1, &(pcNodeDielList1[0]->pcDielNodeNextList) );
			};
			break;
		case  CTMCRTH_BLCKNTYPE_ABSORBER   : ;
			if( (pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPXLEFT )||
				(pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPXRIGHT)||
				(pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPYTOP  )||
				(pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPYBOT  )||
				(pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_METAL    ))
			{
				pcDielNodeNextList = pcNodeDielList1[0];
				pcNodeDielList1[0] = NULL;
				AddList( pcNodeDielOne1, &(pcNodeDielList1[0]) );
				pcNodeDielList1[0]->pcDielNodeNextList = pcDielNodeNextList;
			}
			else
			{
				AddList( pcNodeDielOne1, &(pcNodeDielList1[0]->pcDielNodeNextList) );
			};
			break;
		case  CTMCRTH_BLCKNTYPE_EPS   : ;
//		case  CTMCRTH_BLCKNTYPE_EPSLE0   : ;
			if( (pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPXLEFT )||
				(pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPXRIGHT)||
				(pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPYTOP  )||
				(pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_INPYBOT  )||
				(pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_METAL    )||
				(pcNodeDielOne1->nType == CTMCRTH_BLCKNTYPE_ABSORBER ))
			{
				pcDielNodeNextList = pcNodeDielList1[0];
				pcNodeDielList1[0] = NULL;
				AddList( pcNodeDielOne1, &(pcNodeDielList1[0]) );
				pcNodeDielList1[0]->pcDielNodeNextList = pcDielNodeNextList;
			}
			else
			{
				AddList( pcNodeDielOne1, &(pcNodeDielList1[0]->pcDielNodeNextList) );
			};
			break;
		default :
			csBuf.Format("Unknow node type %d in AddList", pcNodeDielList1[0]->pcNodeDielOne->nType);
			cError.PutErrorMessage(csBuf);
			return;
	};

	return;
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

	int i, n1, i11 = 0;
	
	CString csBuf;
	double dUp, dUo, a, a1;
	_ELEM_VAL_RTH *u2, *u3, *u4, y1, dBuf;

	a = dAmplitude(cParam, dtCurrent)*sin( dWT + dFaza(cParam, dtCurrent) );
//	y1 = (_ELEM_VAL_RTH)1.0;
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
	
	if( nNumNode < 3 ) return;

	for( i = 0, n1 = nNumNode; i < n1; i++)
	{
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 ] = pcNodeDielOne[i].rU_Cur;
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ] = pcNodeDielOne[i].rU_Cur1;
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2 ] = pcNodeDielOne[i].rU_Cur2;
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ] = pcNodeDielOne[i].rU_Cur3;
	};

	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == NULL)
	{
		i = 0;
		switch( nType )
		{
			case CTMCRTH_BLCKNTYPE_INPXLEFT: 
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6+3;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6+3;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12-3+i11] = 0.0;
	                u3[-12-2+i11] = 0.0;
	                u3[-12-1+i11] = 0.0;
					u3[-12+i11] = 0.0;
				}
				break;
			case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6+3;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6+3;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12-3-i11] = 0.0;
	                u3[12-2-i11] = 0.0;
	                u3[12-1-i11] = 0.0;
					u3[12-i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYTOP:
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6+3;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6+3;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX-3] = 0.0;
	                u3[12*nX-2] = 0.0;
	                u3[12*nX-1] = 0.0;
					u3[12*nX] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYBOT:
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6+3;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6+3;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX-3] = 0.0;
	                u3[-12*nX-2] = 0.0;
	                u3[-12*nX-1] = 0.0;
					u3[-12*nX] = 0.0;
				};
				break;
		};

		pr1[ pcNodeDielOne[i].nNodeGlobal*6 ] = (_ELEM_VAL_RTH)(2./( 1. + y1/2. + fabs(dYInput)/4. )*
			( 0.5*u2[0] + 0.5*u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ]*dYInput/4. + 0.5 * y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2] + 
			pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU2 ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2]);
		pcNodeDielOne[i].rU =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		
		for( i = 1, n1 = nNumNode - 1; i < n1; i++)
		{
			switch( nType )
			{
				case CTMCRTH_BLCKNTYPE_INPXLEFT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6+3;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6+3;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6+3;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-12-3+i11] = 0.0;
	              u3[-12-2+i11] = 0.0;
	              u3[-12-1+i11] = 0.0;
		          u3[-12+i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6+3;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6+3;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6+3;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12-3-i11] = 0.0;
	                u3[12-2-i11] = 0.0;
	                u3[12-1-i11] = 0.0;
					u3[12-i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYTOP:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6+3;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6+3;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6+3;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX-3] = 0.0;
	                u3[12*nX-2] = 0.0;
	                u3[12*nX-1] = 0.0;
					u3[12*nX] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYBOT:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6+3;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6+3;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6+3;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX-3] = 0.0;
	                u3[-12*nX-2] = 0.0;
	                u3[-12*nX-1] = 0.0;
					u3[-12*nX] = 0.0;
				};
					break;
			};
			pr1[ pcNodeDielOne[i].nNodeGlobal*6 ] = (_ELEM_VAL_RTH)(2./( 2. + y1  + fabs(dYInput)/2. )*
				( 0.5*u2[0] + u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ]*dYInput/2. + y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2 ] + 
				pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU2 ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2]);
			pcNodeDielOne[i].rU =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		};
		switch( nType )
		{
			case CTMCRTH_BLCKNTYPE_INPXLEFT: 
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6+3;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6+3;
				u4 = pr1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-12-3+i11] = 0.0;
	              u3[-12-2+i11] = 0.0;
	              u3[-12-1+i11] = 0.0;
		          u3[-12+i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6+3;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6+3;
				u4 = pr1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12-3-i11] = 0.0;
	                u3[12-2-i11] = 0.0;
	                u3[12-1-i11] = 0.0;
					u3[12-i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYTOP:
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6+3;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6+3;
				u4 = pr1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX-3] = 0.0;
	                u3[12*nX-2] = 0.0;
	                u3[12*nX-1] = 0.0;
					u3[12*nX] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYBOT:
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6+3;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6+3;
				u4 = pr1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX-3] = 0.0;
	                u3[-12*nX-2] = 0.0;
	                u3[-12*nX-1] = 0.0;
					u3[-12*nX] = 0.0;
				};
				break;
		};
		pr1[ pcNodeDielOne[i].nNodeGlobal*6 ] = (_ELEM_VAL_RTH)(2./( 1. + y1/2. + fabs(dYInput)/4. )*
			( 0.5*u2[0] + 0.5*u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ]*dYInput/4. + 0.5 * y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2] + 
			pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU2 ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2]);
		pcNodeDielOne[i].rU =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
	}
	else
	{
		for( i = 1, n1 = nNumNode - 1; i < n1; i++)
		{
			switch( nType )
			{
				case CTMCRTH_BLCKNTYPE_INPXLEFT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6+3;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6+3;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6+3;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-12-3+i11] = 0.0;
	              u3[-12-2+i11] = 0.0;
	              u3[-12-1+i11] = 0.0;
		          u3[-12+i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6+3;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6+3;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6+3;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12-3-i11] = 0.0;
	                u3[12-2-i11] = 0.0;
	                u3[12-1-i11] = 0.0;
	                u3[12-i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYTOP:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6+3;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6+3;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6+3;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX-3] = 0.0;
	                u3[12*nX-2] = 0.0;
	                u3[12*nX-1] = 0.0;
					u3[12*nX] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYBOT:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6+3;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6+3;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6+3;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX-3] = 0.0;
	                u3[-12*nX-2] = 0.0;
	                u3[-12*nX-1] = 0.0;
					u3[-12*nX] = 0.0;
				};
					break;
			};
			pr1[ pcNodeDielOne[i].nNodeGlobal*6 ] = (_ELEM_VAL_RTH)(2./( 2. + y1 + fabs(dYInput)/2. )*
				( 0.5*u2[0] + u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ]*dYInput/2. + y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2] + 
				pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU2 ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2]);
			pcNodeDielOne[i].rU =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		};
	};

	for( i = 0, n1 = nNumNode; i < n1; i++)
	{
//		pcNodeDielOne[i].rU_Cur = pr1[ pcNodeDielOne[i].nNodeGlobal*6 ];
//		pcNodeDielOne[i].rU_Cur1 = pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ];
//		pcNodeDielOne[i].rU_Cur2 = pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2 ];
//		pcNodeDielOne[i].rU_Cur3 = pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ];
	};

	for( i = 0, dUp = 0.0, dUo = 0.0; i < nNumNode; i++)
	{
		dUp += pcNodeDielOne[i].rY*pcNodeDielOne[i].rY;
		dUo += pcNodeDielOne[i].rY*( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 0] - pcNodeDielOne[i].rU );
//		pcNodeDielOne[i].rU = (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1* _ELEM_VAL_RTH_MULT);
	};

	dUo /= dUp/_ELEM_VAL_RTH_MULT;
	dUp = a1;
//	dUo /= dUp/_ELEM_VAL_RTH_MULT/dUnormir;
//	dUp = a1/dUnormir;
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

	int i, n1, i11 = 0;
	
	CString csBuf;
	double dUp, dUo, a, a1;
	_ELEM_VAL_RTH *u2, *u3, *u4, y1, dBuf;

	a = dAmplitude(cParam, dtCurrent)*sin( dWT + dFaza(cParam, dtCurrent) );
//	y1 = (_ELEM_VAL_RTH)1.0;
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
	
	if( nNumNode < 3 ) return;

	for( i = 0, n1 = nNumNode; i < n1; i++)
	{
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 ] = pcNodeDielOne[i].rU_Cur;
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ] = pcNodeDielOne[i].rU_Cur1;
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2 ] = pcNodeDielOne[i].rU_Cur2;
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ] = pcNodeDielOne[i].rU_Cur3;
	};

	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == NULL)
	{
		i = 0;
		switch( nType )
		{
			case CTMCRTH_BLCKNTYPE_INPXLEFT: 
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-12+i11] = 0.0;
		          u3[-12+1+i11] = 0.0;
		          u3[-12+2+i11] = 0.0;
		          u3[-12+3+i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[12-i11] = 0.0;
		          u3[12+1-i11] = 0.0;
		          u3[12+2-i11] = 0.0;
		          u3[12+3-i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYTOP:
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX] = 0.0;
					u3[12*nX+1] = 0.0;
					u3[12*nX+2] = 0.0;
					u3[12*nX+3] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYBOT:
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX] = 0.0;
					u3[-12*nX+1] = 0.0;
					u3[-12*nX+2] = 0.0;
					u3[-12*nX+3] = 0.0;
				};
				break;
		};
		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ] = (_ELEM_VAL_RTH)(2./( 1. + y1/2. + fabs(dYInput)/4. )*
			( 0.5*u2[0] + 0.5*u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 0]*dYInput/4. + 0.5 * y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3] + 
			pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU3 ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3]);
		pcNodeDielOne[i].rU1 =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		for( i = 1, n1 = nNumNode - 1; i < n1; i++)
		{
			switch( nType )
			{
				case CTMCRTH_BLCKNTYPE_INPXLEFT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-12+i11] = 0.0;
		          u3[-12+1+i11] = 0.0;
		          u3[-12+2+i11] = 0.0;
		          u3[-12+3+i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[12-i11] = 0.0;
		          u3[12+1-i11] = 0.0;
		          u3[12+2-i11] = 0.0;
		          u3[12+3-i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYTOP:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX] = 0.0;
					u3[12*nX+1] = 0.0;
					u3[12*nX+2] = 0.0;
					u3[12*nX+3] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYBOT:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX] = 0.0;
					u3[-12*nX+1] = 0.0;
					u3[-12*nX+2] = 0.0;
					u3[-12*nX+3] = 0.0;
				};
					break;
			};
			pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ] = (_ELEM_VAL_RTH)(2./( 2. + y1 + fabs(dYInput)/2. )*
				( 0.5*u2[0] + u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 0]*dYInput/2. + y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3] + 
				pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU3 ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3]);
			pcNodeDielOne[i].rU1 =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		};
		switch( nType )
		{
			case CTMCRTH_BLCKNTYPE_INPXLEFT: 
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6;
				u4 = pr1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-12+i11] = 0.0;
		          u3[-12+1+i11] = 0.0;
		          u3[-12+2+i11] = 0.0;
		          u3[-12+3+i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6;
				u4 = pr1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[12-i11] = 0.0;
		          u3[12+1-i11] = 0.0;
		          u3[12+2-i11] = 0.0;
		          u3[12+3-i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYTOP:
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6;
				u4 = pr1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX] = 0.0;
					u3[12*nX+1] = 0.0;
					u3[12*nX+2] = 0.0;
					u3[12*nX+3] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYBOT:
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6;
				u4 = pr1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX] = 0.0;
					u3[-12*nX+1] = 0.0;
					u3[-12*nX+2] = 0.0;
					u3[-12*nX+3] = 0.0;
				};
				break;
		};
		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ] = (_ELEM_VAL_RTH)(2./( 1. + y1/2. + fabs(dYInput)/4. )*
			( 0.5*u2[0] + 0.5*u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 0]*dYInput/4. + 0.5 * y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3] + 
			pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU3 ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3]);
		pcNodeDielOne[i].rU1 =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
	}
	else
	{
		for( i = 1, n1 = nNumNode - 1; i < n1; i++)
		{
			switch( nType )
			{
				case CTMCRTH_BLCKNTYPE_INPXLEFT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-12+i11] = 0.0;
		          u3[-12+1+i11] = 0.0;
		          u3[-12+2+i11] = 0.0;
		          u3[-12+3+i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[12-i11] = 0.0;
		          u3[12+1-i11] = 0.0;
		          u3[12+2-i11] = 0.0;
		          u3[12+3-i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYTOP:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX] = 0.0;
					u3[12*nX+1] = 0.0;
					u3[12*nX+2] = 0.0;
					u3[12*nX+3] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYBOT:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX] = 0.0;
					u3[-12*nX+1] = 0.0;
					u3[-12*nX+2] = 0.0;
					u3[-12*nX+3] = 0.0;
				};
					break;
			};
			pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ] = (_ELEM_VAL_RTH)(2./( 2. + y1 + fabs(dYInput)/2. )*
				( 0.5*u2[0] + u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 0]*dYInput/2.  + y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3] + 
				pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU3 ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3]);
			pcNodeDielOne[i].rU1 =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		};
	};

	for( i = 0, n1 = nNumNode; i < n1; i++)
	{
//		pcNodeDielOne[i].rU_Cur = pr1[ pcNodeDielOne[i].nNodeGlobal*6 ];
//		pcNodeDielOne[i].rU_Cur1 = pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ];
//		pcNodeDielOne[i].rU_Cur2 = pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2 ];
//		pcNodeDielOne[i].rU_Cur3 = pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ];
	};

	for( i = 0, dUp = 0.0, dUo = 0.0; i < nNumNode; i++)
	{
		dUp += pcNodeDielOne[i].rY*pcNodeDielOne[i].rY;
		dUo += pcNodeDielOne[i].rY*( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1] - pcNodeDielOne[i].rU1 );
//		pcNodeDielOne[i].rU = (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1* _ELEM_VAL_RTH_MULT);
	};

	dUo /= dUp/_ELEM_VAL_RTH_MULT;
	dUp = a1;
//	dUo /= dUp/_ELEM_VAL_RTH_MULT/dUnormir;
//	dUp = a1/dUnormir;
	OutputUpUo( dUp, dUo , cError1);


	return;
}



void CTmcRTHNodeDiel::ExciteInputs2( CTmcLibError &cError1, double dWT, double dT, _ELEM_VAL_RTH *pr1, int nX, CTmcRTH_IndanParam &cParam, double dtCurrent )
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

	int i, n1, i11 = 0;
	
	CString csBuf;
	double dUp, dUo, a, a1;
	_ELEM_VAL_RTH *u2, *u3, *u4, y1, dBuf;

	a = dAmplitude(cParam, dtCurrent)*sin( dWT + dFaza(cParam, dtCurrent) );
//	y1 = (_ELEM_VAL_RTH)1.0;
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
	
	if( nNumNode < 3 ) return;

	for( i = 0, n1 = nNumNode; i < n1; i++)
	{
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 ] = pcNodeDielOne[i].rU_Cur;
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ] = pcNodeDielOne[i].rU_Cur1;
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2 ] = pcNodeDielOne[i].rU_Cur2;
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ] = pcNodeDielOne[i].rU_Cur3;
	};

	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == NULL)
	{
		i = 0;
		switch( nType )
		{
			case CTMCRTH_BLCKNTYPE_INPXLEFT: 
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 1;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-12+i11] = 0.0;
		          u3[-12+1+i11] = 0.0;
		          u3[-12+2+i11] = 0.0;
		          u3[-12+3+i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 1;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[12-i11] = 0.0;
		          u3[12+1-i11] = 0.0;
		          u3[12+2-i11] = 0.0;
		          u3[12+3-i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYTOP:
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 1;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX] = 0.0;
					u3[12*nX+1] = 0.0;
					u3[12*nX+2] = 0.0;
					u3[12*nX+3] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYBOT:
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 1;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX] = 0.0;
					u3[-12*nX+1] = 0.0;
					u3[-12*nX+2] = 0.0;
					u3[-12*nX+3] = 0.0;
				};
				break;
		};
		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2 ] = (_ELEM_VAL_RTH)(2./( 1. + y1/2. + fabs(dYInput)/4. )*
			( 0.5*u2[0] + 0.5*u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1]*dYInput/4. + 0.5 * y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 0] + 
			pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 0]);
		pcNodeDielOne[i].rU2 =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		for( i = 1, n1 = nNumNode - 1; i < n1; i++)
		{
			switch( nType )
			{
				case CTMCRTH_BLCKNTYPE_INPXLEFT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-12+i11] = 0.0;
		          u3[-12+1+i11] = 0.0;
		          u3[-12+2+i11] = 0.0;
		          u3[-12+3+i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[12-i11] = 0.0;
		          u3[12+1-i11] = 0.0;
		          u3[12+2-i11] = 0.0;
		          u3[12+3-i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYTOP:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX] = 0.0;
					u3[12*nX+1] = 0.0;
					u3[12*nX+2] = 0.0;
					u3[12*nX+3] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYBOT:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX] = 0.0;
					u3[-12*nX+1] = 0.0;
					u3[-12*nX+2] = 0.0;
					u3[-12*nX+3] = 0.0;
				};
					break;
			};
			pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2 ] = (_ELEM_VAL_RTH)(2./( 2. + y1 + fabs(dYInput)/2. )*
				( 0.5*u2[0] + u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ]*dYInput/2. + y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 0] + 
				pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 0]);
			pcNodeDielOne[i].rU2 =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		};
		switch( nType )
		{
			case CTMCRTH_BLCKNTYPE_INPXLEFT: 
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 1;
				u4 = pr1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-12+i11] = 0.0;
		          u3[-12+1+i11] = 0.0;
		          u3[-12+2+i11] = 0.0;
		          u3[-12+3+i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 1;
				u4 = pr1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[12-i11] = 0.0;
		          u3[12+1-i11] = 0.0;
		          u3[12+2-i11] = 0.0;
		          u3[12+3-i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYTOP:
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 1;
				u4 = pr1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX] = 0.0;
					u3[12*nX+1] = 0.0;
					u3[12*nX+2] = 0.0;
					u3[12*nX+3] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYBOT:
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 1;
				u4 = pr1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX] = 0.0;
					u3[-12*nX+1] = 0.0;
					u3[-12*nX+2] = 0.0;
					u3[-12*nX+3] = 0.0;
				};
				break;
		};
		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2 ] = (_ELEM_VAL_RTH)(2./( 1. + y1/2. + fabs(dYInput)/4. )*
			( 0.5*u2[0] + 0.5*u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1]*dYInput/4. + 0.5 * y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 0 ] + 
			pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 0]);
		pcNodeDielOne[i].rU2 =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
	}
	else
	{
		for( i = 1, n1 = nNumNode - 1; i < n1; i++)
		{
			switch( nType )
			{
				case CTMCRTH_BLCKNTYPE_INPXLEFT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-12+i11] = 0.0;
		          u3[-12+1+i11] = 0.0;
		          u3[-12+2+i11] = 0.0;
		          u3[-12+3+i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[12-i11] = 0.0;
		          u3[12+1-i11] = 0.0;
		          u3[12+2-i11] = 0.0;
		          u3[12+3-i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYTOP:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX] = 0.0;
					u3[12*nX+1] = 0.0;
					u3[12*nX+2] = 0.0;
					u3[12*nX+3] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYBOT:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 1;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 1;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX] = 0.0;
					u3[-12*nX+1] = 0.0;
					u3[-12*nX+2] = 0.0;
					u3[-12*nX+3] = 0.0;
				};
					break;
			};
			pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2 ] = (_ELEM_VAL_RTH)(2./( 2. + y1 + fabs(dYInput)/2. )*
				( 0.5*u2[0] + u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ]*dYInput/2.  + y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 0] + 
				pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 0]);
			pcNodeDielOne[i].rU2 =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		};
	};

	for( i = 0, n1 = nNumNode; i < n1; i++)
	{
//		pcNodeDielOne[i].rU_Cur = pr1[ pcNodeDielOne[i].nNodeGlobal*6 ];
//		pcNodeDielOne[i].rU_Cur1 = pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ];
//		pcNodeDielOne[i].rU_Cur2 = pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2 ];
//		pcNodeDielOne[i].rU_Cur3 = pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ];
	};

	for( i = 0, dUp = 0.0, dUo = 0.0; i < nNumNode; i++)
	{
		dUp += pcNodeDielOne[i].rY*pcNodeDielOne[i].rY;
		dUo += pcNodeDielOne[i].rY*( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2] - pcNodeDielOne[i].rU2 );
//		pcNodeDielOne[i].rU = (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1* _ELEM_VAL_RTH_MULT);
	};

	dUo /= dUp/_ELEM_VAL_RTH_MULT;
	dUp = a1;
//	dUo /= dUp/_ELEM_VAL_RTH_MULT/dUnormir;
//	dUp = a1/dUnormir;
	OutputUpUo( dUp, dUo , cError1);


	return;
}



void CTmcRTHNodeDiel::ExciteInputs3( CTmcLibError &cError1, double dWT, double dT, _ELEM_VAL_RTH *pr1, int nX, CTmcRTH_IndanParam &cParam, double dtCurrent )
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

	int i, n1, i11 = 0;
	
	CString csBuf;
	double dUp, dUo, a, a1;
	_ELEM_VAL_RTH *u2, *u3, *u4, y1, dBuf;

	a = dAmplitude(cParam, dtCurrent)*sin( dWT + dFaza(cParam, dtCurrent) );
//	y1 = (_ELEM_VAL_RTH)1.0;
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
	
	if( nNumNode < 3 ) return;

	for( i = 0, n1 = nNumNode; i < n1; i++)
	{
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 ] = pcNodeDielOne[i].rU_Cur;
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ] = pcNodeDielOne[i].rU_Cur1;
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2 ] = pcNodeDielOne[i].rU_Cur2;
//		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ] = pcNodeDielOne[i].rU_Cur3;
	};

	if( strcmp( csEpsExpr, CTMCRTH_INDANBLCK_TYPEMAG) == NULL)
	{
		i = 0;
		switch( nType )
		{
			case CTMCRTH_BLCKNTYPE_INPXLEFT: 
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 2;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 2;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-12+i11] = 0.0;
		          u3[-12+1+i11] = 0.0;
		          u3[-12+2+i11] = 0.0;
		          u3[-12+3+i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 2;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 2;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[12-i11] = 0.0;
		          u3[12+1-i11] = 0.0;
		          u3[12+2-i11] = 0.0;
		          u3[12+3-i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYTOP:
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 2;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 2;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX] = 0.0;
					u3[12*nX+1] = 0.0;
					u3[12*nX+2] = 0.0;
					u3[12*nX+3] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYBOT:
				u2 = pr1;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 2;
				u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 2;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX] = 0.0;
					u3[-12*nX+1] = 0.0;
					u3[-12*nX+2] = 0.0;
					u3[-12*nX+3] = 0.0;
				};
				break;
		};
		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ] = (_ELEM_VAL_RTH)(2./( 1. + y1/2. + fabs(dYInput)/4. )*
			( 0.5*u2[0] + 0.5*u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2]*dYInput/4. + 0.5 * y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ] + 
			pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU1 ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ]);
		pcNodeDielOne[i].rU3 =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		for( i = 1, n1 = nNumNode - 1; i < n1; i++)
		{
			switch( nType )
			{
				case CTMCRTH_BLCKNTYPE_INPXLEFT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 2;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-12+i11] = 0.0;
		          u3[-12+1+i11] = 0.0;
		          u3[-12+2+i11] = 0.0;
		          u3[-12+3+i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 2;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[12-i11] = 0.0;
		          u3[12+1-i11] = 0.0;
		          u3[12+2-i11] = 0.0;
		          u3[12+3-i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYTOP:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 2;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX] = 0.0;
					u3[12*nX+1] = 0.0;
					u3[12*nX+2] = 0.0;
					u3[12*nX+3] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYBOT:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 2;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX] = 0.0;
					u3[-12*nX+1] = 0.0;
					u3[-12*nX+2] = 0.0;
					u3[-12*nX+3] = 0.0;
				};
					break;
			};
			pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ] = (_ELEM_VAL_RTH)(2./( 2. + y1 + fabs(dYInput)/2. )*
				( 0.5*u2[0] + u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2 ]*dYInput/2. + y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ] + 
				pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU1 ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ]);
			pcNodeDielOne[i].rU3 =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		};
		switch( nType )
		{
			case CTMCRTH_BLCKNTYPE_INPXLEFT: 
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 2;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 2;
				u4 = pr1 + 2;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-12+i11] = 0.0;
		          u3[-12+1+i11] = 0.0;
		          u3[-12+2+i11] = 0.0;
		          u3[-12+3+i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 2;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 2;
				u4 = pr1;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[12-i11] = 0.0;
		          u3[12+1-i11] = 0.0;
		          u3[12+2-i11] = 0.0;
		          u3[12+3-i11] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYTOP:
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 2;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 2;
				u4 = pr1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX] = 0.0;
					u3[12*nX+1] = 0.0;
					u3[12*nX+2] = 0.0;
					u3[12*nX+3] = 0.0;
				};
				break;
			case CTMCRTH_BLCKNTYPE_INPYBOT:
				u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 2;
				u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 2;
				u4 = pr1;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX] = 0.0;
					u3[-12*nX+1] = 0.0;
					u3[-12*nX+2] = 0.0;
					u3[-12*nX+3] = 0.0;
				};
				break;
		};
		pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ] = (_ELEM_VAL_RTH)(2./( 1. + y1/2. + fabs(dYInput)/4. )*
			( 0.5*u2[0] + 0.5*u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2]*dYInput/4. + 0.5 * y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ] + 
			pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU1 ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ]);
		pcNodeDielOne[i].rU3 =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
	}
	else
	{
		for( i = 1, n1 = nNumNode - 1; i < n1; i++)
		{
			switch( nType )
			{
				case CTMCRTH_BLCKNTYPE_INPXLEFT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 2;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rXmin + 0.99999*cParam.dGetDelta() ) )
				{
	              u3[-12+i11] = 0.0;
		          u3[-12+1+i11] = 0.0;
		          u3[-12+2+i11] = 0.0;
		          u3[-12+3+i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPXRIGHT: 
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 2;
//
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rXmax - 0.99999*cParam.dGetDelta() ) )
				{
	              u3[12-i11] = 0.0;
		          u3[12+1-i11] = 0.0;
		          u3[12+2-i11] = 0.0;
		          u3[12+3-i11] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYTOP:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal - nX)*6 + 2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 2;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) < ( cParam.rYmax - 0.99999*cParam.dGetDelta() ) )
				{
	                u3[12*nX] = 0.0;
					u3[12*nX+1] = 0.0;
					u3[12*nX+2] = 0.0;
					u3[12*nX+3] = 0.0;
				};
					break;
				case CTMCRTH_BLCKNTYPE_INPYBOT:
					u2 = pr1 + (pcNodeDielOne[i].nNodeGlobal - 1)*6 + 2;
					u3 = pr1 + (pcNodeDielOne[i].nNodeGlobal + nX)*6 + 2;
					u4 = pr1 + (pcNodeDielOne[i].nNodeGlobal + 1)*6 + 2;
//
				if( cParam.CalcY(pcNodeDielOne[i].nNodeGlobal) > ( cParam.rYmin + 0.99999*cParam.dGetDelta() ) )
				{
	                u3[-12*nX] = 0.0;
					u3[-12*nX+1] = 0.0;
					u3[-12*nX+2] = 0.0;
					u3[-12*nX+3] = 0.0;
				};
					break;
			};
			pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ] = (_ELEM_VAL_RTH)(2./( 2. + y1 + fabs(dYInput)/2. )*
				( 0.5*u2[0] + u3[0] + 0.5*u4[0] + pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2 ]*dYInput/2.  + y1 * ( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1] + 
				pcNodeDielOne[i].rY*a1 - pcNodeDielOne[i].rU1 ) ) - pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1]);
			pcNodeDielOne[i].rU3 =  (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1);
		};
	};

	for( i = 0, n1 = nNumNode; i < n1; i++)
	{
//		pcNodeDielOne[i].rU_Cur = pr1[ pcNodeDielOne[i].nNodeGlobal*6 ];
//		pcNodeDielOne[i].rU_Cur1 = pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 1 ];
//		pcNodeDielOne[i].rU_Cur2 = pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 2 ];
//		pcNodeDielOne[i].rU_Cur3 = pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3 ];
	};

	for( i = 0, dUp = 0.0, dUo = 0.0; i < nNumNode; i++)
	{
		dUp += pcNodeDielOne[i].rY*pcNodeDielOne[i].rY;
		dUo += pcNodeDielOne[i].rY*( pr1[ pcNodeDielOne[i].nNodeGlobal*6 + 3] - pcNodeDielOne[i].rU3 );
//		pcNodeDielOne[i].rU = (_ELEM_VAL_RTH)(pcNodeDielOne[i].rY*a1* _ELEM_VAL_RTH_MULT);
	};

	dUo /= dUp/_ELEM_VAL_RTH_MULT;
	dUp = a1;
//	dUo /= dUp/_ELEM_VAL_RTH_MULT/dUnormir;
//	dUp = a1/dUnormir;
	OutputUpUo( dUp, dUo , cError1);


	return;
}



void CTmcRTHNodeDiel::OutputUpUo( double dUp, double dUo, CTmcLibError &cError1)
{
	FILE *fp;
	char *psForm;

	for( ; (fp = fopen( csBlock, "a")) == NULL ; )
	{
		Sleep(10);
	};

	psForm = GetnBFormat();
	fprintf( fp, psForm, nBlock);
	psForm = GetdInpFormat();
	fprintf( fp, psForm, dUp);
	psForm = GetdOutFormat();

#ifdef _VERSION_PROF___
	fprintf( fp, psForm, dUo);
#endif

#ifdef _VERSION_EDUC___
	fprintf( fp, psForm, dUo);
#endif

#ifdef _VERSION_DEMO___
	fprintf( fp, "   DemoVersion   ");
#endif
	
	fclose( fp );
	return;

}

double CTmcRTHNodeDiel::SetInputAdmitance( double dFreq, double dDelta )
{
	double dY1, dA_111;
//	CString csBuf1;

// for  delta*W->0
//	dY1 = sqrt( 2.*(1. - ( C0___/(dFreq*2.*dWidthWaveg) )*( C0___/(dFreq*2.*dWidthWaveg) ) ) );

	dA_111 = sqrt(2.)*PI___*dDelta*dFreq/C0___;
//	dY1 = 1/(1+2*PI___*PI___/3.*(dDelta*dFreq/C0___)*(dDelta*dFreq/C0___))*acos( 2*cos(dA_111) - cos(PI___*dDelta/dWidthWaveg) )/dA_111;
	dY1 = sin(acos( (2+fabs(dYInput)/2.)*cos(dA_111) - dYInput/2. - cos(PI___/(dWidthWaveg/dDelta)) ))/sin(dA_111);

//	t0 = dDelta*sqrt( MU0___*EPS0___/2. );
//	dY1 = acos( 2*cos(2*PI___*dFreq*t0) - cos(PI___*dDelta/dWidthWaveg) );
//	dWidthWaveg = 2*PI___*dFreq*t0;

//	csBuf1.Format("Freq = %lg; a = %lg; y1 = %lg", dFreq, dWidthWaveg, dY1);
//	AfxMessageBox(csBuf1);

	return dY1;
}

double CTmcRTHNodeDiel::SetAbsorberAdmitance( double dFreq, double dDelta )
{
	double dY1, dA_111;
//	CString csBuf1;

	dA_111 = sqrt(2.)*PI___*dDelta*dFreq/C0___;
	dY1 = sin(acos( (2+fabs(dYInput)/2.)*cos(dA_111) - 1. - dYInput/2. ))/sin(dA_111);

//	csBuf1.Format("y1 = %lg", dY1);
//	AfxMessageBox(csBuf1);

	return dY1;
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
	int i, nX, nY, ii;
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

void CTmcRTHNodeDiel::RunBlockNode( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, int nArraySize )
{
	if( nType == CTMCRTH_BLCKNTYPE_EPS ) return;
	if( cError.IsError() ) return;
	if( pcNodeDielOne == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return;
	};

	int i;
	double u1, u2, u3, u4, ut1, ut2, y5, y;
	_ELEM_VAL_RTH *prU;
	
	switch( nType )
	{
		case  CTMCRTH_BLCKNTYPE_METAL    :
			for( i = 0; i < nNumNode; i++)
				prUNode1[ 6*pcNodeDielOne[i].nNodeGlobal ] = (_ELEM_VAL_RTH)(0.0);
			break;
		case  CTMCRTH_BLCKNTYPE_MAGNETIC :
			;
		case  CTMCRTH_BLCKNTYPE_ABSORBER :
			for( i = 0; i < nNumNode; i++)
			{
				if(
					((6*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 5) < nArraySize)&&
					((6*(pcNodeDielOne[i].nNodeGlobal + nX ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal + nX ) + 5) < nArraySize)&&
					((6*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 5) < nArraySize)&&
					((6*(pcNodeDielOne[i].nNodeGlobal - nX ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal - nX ) + 5) < nArraySize)&&
					((6*pcNodeDielOne[i].nNodeGlobal + 5) > 0)&&
					((6*pcNodeDielOne[i].nNodeGlobal + 5) < nArraySize)
					)
				{
					u1  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 3 ];
					u2  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal + nX ) + 3 ];
					u3  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 3 ];
					u4  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal - nX ) + 3 ];
					ut1 = prUNode1[ 6*pcNodeDielOne[i].nNodeGlobal + 3 ];
					ut2 = pcNodeDielOne[i].rU;
//					pcNodeDielOne[i].rU = (_ELEM_VAL_RTH)ut1;
					y = pcNodeDielOne[i].rY;
//					if( y < (1.e-6) ) y = (1.e-6);
					y5 = prYNode[ 6*pcNodeDielOne[i].nNodeGlobal ];
//{		CString csBuf1;
//		csBuf1.Format("y = %lg; y5 = %lg;", y, y5);
//		AfxMessageBox(csBuf1);		
//	};
					prU = prUNode1 + (6*pcNodeDielOne[i].nNodeGlobal + 0);
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
				pcNodeDielOne[i].rU = (_ELEM_VAL_RTH)(prUNode1[ 6*pcNodeDielOne[i].nNodeGlobal + 3 ]);
			};
			break;
		default:
			break;
	};

	return;
}

void CTmcRTHNodeDiel::RunBlockNode1( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, int nArraySize )
{
	if( nType == CTMCRTH_BLCKNTYPE_EPS ) return;
	if( cError.IsError() ) return;
	if( pcNodeDielOne == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return;
	};

	int i;
	double u1, u2, u3, u4, ut1, ut2, y5, y;
	_ELEM_VAL_RTH *prU;
	
	switch( nType )
	{
		case  CTMCRTH_BLCKNTYPE_METAL    :
			for( i = 0; i < nNumNode; i++)
				prUNode1[ 6*pcNodeDielOne[i].nNodeGlobal + 1 ] = (_ELEM_VAL_RTH)(0.0);
			break;
		case  CTMCRTH_BLCKNTYPE_MAGNETIC :
			;
		case  CTMCRTH_BLCKNTYPE_ABSORBER :
			for( i = 0; i < nNumNode; i++)
			{
				if(
					((6*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 5) < nArraySize)&&
					((6*(pcNodeDielOne[i].nNodeGlobal + nX ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal + nX ) + 5) < nArraySize)&&
					((6*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 5) < nArraySize)&&
					((6*(pcNodeDielOne[i].nNodeGlobal - nX ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal - nX ) + 5) < nArraySize)&&
					((6*pcNodeDielOne[i].nNodeGlobal + 5) > 0)&&
					((6*pcNodeDielOne[i].nNodeGlobal + 5) < nArraySize)
					)
				{
					u1  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 0 ];
					u2  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal + nX ) + 0 ];
					u3  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 0 ];
					u4  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal - nX ) + 0 ];
					ut1 = prUNode1[ 6*pcNodeDielOne[i].nNodeGlobal + 0 ];
					ut2 = pcNodeDielOne[i].rU;
//					pcNodeDielOne[i].rU = ut1;
					y = pcNodeDielOne[i].rY;
//					if( y < (1.e-6) ) y = (1.e-6);
					y5 = prYNode[ 6*pcNodeDielOne[i].nNodeGlobal ];
					prU = prUNode1 + (6*pcNodeDielOne[i].nNodeGlobal + 1);
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
				pcNodeDielOne[i].rU = (_ELEM_VAL_RTH)(prUNode1[ 6*pcNodeDielOne[i].nNodeGlobal + 0 ]);
			};
			break;
		default:
			break;
	};

	return;
}




void CTmcRTHNodeDiel::RunBlockNode2( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, int nArraySize )
{
	if( nType == CTMCRTH_BLCKNTYPE_EPS ) return;
	if( cError.IsError() ) return;
	if( pcNodeDielOne == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return;
	};

	int i;
	double u1, u2, u3, u4, ut1, ut2, y5, y;
	_ELEM_VAL_RTH *prU;
	
	switch( nType )
	{
		case  CTMCRTH_BLCKNTYPE_METAL    :
			for( i = 0; i < nNumNode; i++)
				prUNode1[ 6*pcNodeDielOne[i].nNodeGlobal + 2 ] = (_ELEM_VAL_RTH)(0.0);
			break;
		case  CTMCRTH_BLCKNTYPE_MAGNETIC :
			;
		case  CTMCRTH_BLCKNTYPE_ABSORBER :
			for( i = 0; i < nNumNode; i++)
			{
				if(
					((6*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 5) < nArraySize)&&
					((6*(pcNodeDielOne[i].nNodeGlobal + nX ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal + nX ) + 5) < nArraySize)&&
					((6*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 5) < nArraySize)&&
					((6*(pcNodeDielOne[i].nNodeGlobal - nX ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal - nX ) + 5) < nArraySize)&&
					((6*pcNodeDielOne[i].nNodeGlobal + 5) > 0)&&
					((6*pcNodeDielOne[i].nNodeGlobal + 5) < nArraySize)
					)
				{
					u1  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 1 ];
					u2  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal + nX ) + 1 ];
					u3  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 1 ];
					u4  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal - nX ) + 1 ];
					ut1 = prUNode1[ 6*pcNodeDielOne[i].nNodeGlobal + 1 ];
					ut2 = pcNodeDielOne[i].rU;
//					pcNodeDielOne[i].rU = (_ELEM_VAL_RTH)ut1;
					y = pcNodeDielOne[i].rY;
//					if( y < (1.e-6) ) y = (1.e-6);
					y5 = prYNode[ 6*pcNodeDielOne[i].nNodeGlobal ];
//{		CString csBuf1;
//		csBuf1.Format("y = %lg; y5 = %lg;", y, y5);
//		AfxMessageBox(csBuf1);		
//	};
					prU = prUNode1 + (6*pcNodeDielOne[i].nNodeGlobal + 2);
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
				pcNodeDielOne[i].rU = (_ELEM_VAL_RTH)(prUNode1[ 6*pcNodeDielOne[i].nNodeGlobal + 1 ]);
			};
			break;
		default:
			break;
	};

	return;
}

void CTmcRTHNodeDiel::RunBlockNode3( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, int nArraySize )
{
	if( nType == CTMCRTH_BLCKNTYPE_EPS ) return;
	if( cError.IsError() ) return;
	if( pcNodeDielOne == NULL )
	{				
		cError.PutErrorMessage("Init Diel Node pointer is NULL");
		return;
	};

	int i;
	double u1, u2, u3, u4, ut1, ut2, y5, y;
	_ELEM_VAL_RTH *prU;
	
	switch( nType )
	{
		case  CTMCRTH_BLCKNTYPE_METAL    :
			for( i = 0; i < nNumNode; i++)
				prUNode1[ 6*pcNodeDielOne[i].nNodeGlobal + 3 ] = (_ELEM_VAL_RTH)(0.0);
			break;
		case  CTMCRTH_BLCKNTYPE_MAGNETIC :
			;
		case  CTMCRTH_BLCKNTYPE_ABSORBER :
			for( i = 0; i < nNumNode; i++)
			{
				if(
					((6*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 5) < nArraySize)&&
					((6*(pcNodeDielOne[i].nNodeGlobal + nX ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal + nX ) + 5) < nArraySize)&&
					((6*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 5) < nArraySize)&&
					((6*(pcNodeDielOne[i].nNodeGlobal - nX ) + 5) > 0)&&
					((6*(pcNodeDielOne[i].nNodeGlobal - nX ) + 5) < nArraySize)&&
					((6*pcNodeDielOne[i].nNodeGlobal + 5) > 0)&&
					((6*pcNodeDielOne[i].nNodeGlobal + 5) < nArraySize)
					)
				{
					u1  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal -  1 ) + 2 ];
					u2  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal + nX ) + 2 ];
					u3  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal +  1 ) + 2 ];
					u4  = prUNode1[ 6*(pcNodeDielOne[i].nNodeGlobal - nX ) + 2 ];
					ut1 = prUNode1[ 6*pcNodeDielOne[i].nNodeGlobal + 2 ];
					ut2 = pcNodeDielOne[i].rU;
//					pcNodeDielOne[i].rU = ut1;
					y = pcNodeDielOne[i].rY;
//					if( y < (1.e-6) ) y = (1.e-6);
					y5 = prYNode[ 6*pcNodeDielOne[i].nNodeGlobal ];
					prU = prUNode1 + (6*pcNodeDielOne[i].nNodeGlobal + 3);
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
				pcNodeDielOne[i].rU = (_ELEM_VAL_RTH)(prUNode1[ 6*pcNodeDielOne[i].nNodeGlobal + 2 ]);
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
	int i, j, k, nX, nY, ii;
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
	
	nNumNode += abs( nY1 - nY2 ) + abs( nX1 - nX2 );
*/
//	csBuf1.Format("nNumNode = %d;", nNumNode);
//	AfxMessageBox(csBuf1);
		

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
		if( nY < nX )
		{
			for( i = 0, j = 0; (i < nY)&&(j < nX) ; i++ )
			{
//				for( jj = 0; (jj < njj)&&(j < nX); j++, ii++, jj++ )
				for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++, ii++ )
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
				};
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
					ii++;
			};
//				for( jj = 0; (jj < njj)&&(j < nX); j++, ii++, jj++ )
				for(  ; ( (j*nY) < ((i+1)*nX) )&&(j < nX); j++, ii++ )
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
				};
		}
		else
		{
//			for( i = 0, j = 0, njj = nY/(nX+1)+1; (i < nX)&&(j < nY) ; i++ )
			for( i = 0, j = 0; (i < nX)&&(j < nY) ; i++ )
			{
//				for( jj = 0; (jj < njj)&&(j < nY); j++, ii++, jj++ )
				for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++, ii++ )
				{
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
				};
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
					ii++;
			};
//				for( jj = 0; (jj < njj)&&(j < nY); j++, ii++, jj++ )
				for(  ; ( (j*nX) < ((i+1)*nY) )&&(j < nY); j++, ii++ )
				{
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
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
//	csBuf1.Format("nY = %d; nX = %d", nY, nX);
//	AfxMessageBox(csBuf1);		
		if( nY < nX )
		{
			for( i = 0, j = 0, njj = nX/(nY+1)+1; (i < nY)&&(j < nX) ; i++ )
			{
				for( jj = 0; (jj < njj)&&(j < nX); j++, ii++, jj++ )
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
				};
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
					ii++;
			};
				for( jj = 0; (jj < njj)&&(j < nX); j++, ii++, jj++ )
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
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
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
				};
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
					ii++;
			};
				for( jj = 0; (jj < njj)&&(j < nY); j++, ii++, jj++ )
				{
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(0.0);
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
				};
		};
*/
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
	int i, j, k1, k2, nX, nY, ii;
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
		for( i = 0, j = 0, njj = nX/(nY+1)+1; (i < nY)&&(j < nX) ; i++ )
		{
			for( jj = 0; (jj < njj)&&(j < nX); j++, ii++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
			};
			x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(0.0);
			ii++;
			};
			for( jj = 0; (jj < njj)&&(j < nX); j++, ii++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
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
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
			};
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
				ii++;
		};
			for( jj = 0; (jj < njj)&&(j < nY); j++, ii++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
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
		for( i = 0, j = 0, njj = nX/(nY+1)+1; (i < nY)&&(j < nX) ; i++ )
		{
			for( jj = 0; (jj < njj)&&(j < nX); j++, ii++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
			};
			x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(0.0);
			ii++;
			};
			for( jj = 0; (jj < njj)&&(j < nX); j++, ii++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
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
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
			};
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(0.0);
				ii++;
		};
			for( jj = 0; (jj < njj)&&(j < nY); j++, ii++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
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
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(0.0);
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
	int i, nX, nY, ii;
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
	r = (dXmax - cParam.rXmin)/cParam.rDelta - 0.5;
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
	r = (dYmax - cParam.rYmin)/cParam.rDelta - 0.5;
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
		csBuf.Format("Xmax - Xmin for block %d must be >= delta(%lg m) for type ABSORBER", nBlock, cParam.rDelta);
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
		csBuf.Format("Ymax - Ymin for block %d must be >= delta(%lg m) for type ABSORBER", nBlock, cParam.rDelta);
		cError.PutErrorMessage(csBuf);
		return cError;
	};
	nY = nYmax - nYmin + 1;

	nNumNode = 2*nX + 2*(nY-2) + 2*(nX-2)+2*(nY-3);

//{	CString csBuf1;
//	csBuf1.Format("nNumNode = %d;", nNumNode);
//	AfxMessageBox(csBuf1);		
//};


//{	CString csBuf1;
//	csBuf1.Format("nXmin = %d; nXmax = %d; nX = %d;", nXmin, nXmax, nX);
//	AfxMessageBox(csBuf1);		
//	csBuf1.Format("nYmin = %d; nYmax = %d; nY = %d;", nYmin, nYmax, nY);
//	AfxMessageBox(csBuf1);		
//	csBuf1.Format("nNumNode = %d;", nNumNode);
//	AfxMessageBox(csBuf1);		
//};

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
//	csBuf1.Format("ii = %d;", ii);
//	AfxMessageBox(csBuf1);		
//};

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
	int i, nX, nY, ii;
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
//	csBuf1.Format("nNumNode = %d;", nNumNode);
//	AfxMessageBox(csBuf1);		
//};


//{	CString csBuf1;
//	csBuf1.Format("nXmin = %d; nXmax = %d; nX = %d;", nXmin, nXmax, nX);
//	AfxMessageBox(csBuf1);		
//	csBuf1.Format("nYmin = %d; nYmax = %d; nY = %d;", nYmin, nYmax, nY);
//	AfxMessageBox(csBuf1);		
//	csBuf1.Format("nNumNode = %d;", nNumNode);
//	AfxMessageBox(csBuf1);		
//};

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
//	csBuf1.Format("ii = %d;", ii);
//	AfxMessageBox(csBuf1);		
//};

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
	int i, j, k, nX, nY, ii, n_x, n_y;
	double r, x, y, x1, y1, dYAdmit;
//	CString csBuf1;
	dYAdmit = SetAbsorberAdmitance( cParam.rFreq, cParam.rDelta );
	n_x = (int)((cParam.rXmax - cParam.rXmin)/cParam.rDelta + 1 );
	n_y = (int)((cParam.rYmax - cParam.rYmin)/cParam.rDelta + 1 );

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
//	csBuf1.Format("nNumNode = %d;", nNumNode);
//	AfxMessageBox(csBuf1);
		

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
		if( nY < nX )
		{
			for( i = 0, j = 0, njj = nX/(nY+1)+1; (i < nY)&&(j < nX) ; i++ )
			{
				for( jj = 0; (jj < njj)&&(j < nX); j++, jj++ )
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
				};
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
			};
				for( jj = 0; (jj < njj)&&(j < nX); j++, jj++ )
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
				};
		}
		else
		{
			for( i = 0, j = 0, njj = nY/(nX+1)+1; (i < nX)&&(j < nY) ; i++ )
			{
				for( jj = 0; (jj < njj)&&(j < nY); j++, jj++ )
				{
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
				};
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
			};
				for( jj = 0; (jj < njj)&&(j < nY); j++, jj++ )
				{
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
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
	SetAbsorberNodeType( cParam );
	return cError;
}


void CTmcRTHNodeDiel::SetAbsorberNodeType( CTmcRTH_IndanParam &cParam )
{
	int i, j, n, nx, nn;

	nx = (int)((cParam.rXmax - cParam.rXmin)/cParam.rDelta + 1 );

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
//		{
//			CString csBuf1;
//			csBuf1.Format("Type = %d; nNode = %d; x = %lg; y = %lg", pcNodeDielOne[i].nType, pcNodeDielOne[i].nNodeGlobal,
//				cParam.CalcX( pcNodeDielOne[i].nNodeGlobal ),
//				cParam.CalcY( pcNodeDielOne[i].nNodeGlobal ) );
//			AfxMessageBox(csBuf1);		
//		};
//	pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
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
			if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTSTAT, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == NULL ) 
			{
				return;
			};
	};

	{
		CString csBuf;
		csBuf.Format("Delete Dublicated Data");
		PutStatistics(csBuf);
	};
	int i, j, n, n1;
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
			pcNodeDielOne1[n].rU1    = pcNodeDielOne[i].rU1;
			pcNodeDielOne1[n].rU2    = pcNodeDielOne[i].rU2;
			pcNodeDielOne1[n].rU3    = pcNodeDielOne[i].rU3;
			pcNodeDielOne1[n].rU_Cur = pcNodeDielOne[i].rU_Cur;
			pcNodeDielOne1[n].rU_Cur1= pcNodeDielOne[i].rU_Cur1;
			pcNodeDielOne1[n].rU_Cur2= pcNodeDielOne[i].rU_Cur2;
			pcNodeDielOne1[n].rU_Cur3= pcNodeDielOne[i].rU_Cur3;
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
	int i, j, k1, k2, nX, nY, ii, n_x, n_y;
	double r, x, y, x1, y1, df, dYAdmit;

	dYAdmit = SetAbsorberAdmitance( cParam.rFreq, cParam.rDelta );
	n_x = (int)((cParam.rXmax - cParam.rXmin)/cParam.rDelta + 1 );
	n_y = (int)((cParam.rYmax - cParam.rYmin)/cParam.rDelta + 1 );

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
	
	k1 = 10*(int)((pcNextBlockList->GetXmin()*( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() ) )/cParam.rDelta + 1 );
	nNumNode += k1;
	
	k2 = 10*(int)((pcNextBlockList->GetXmax()*( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() ) )/cParam.rDelta + 1 );
	nNumNode += k2;
	
	nNumNode *= 4;
//	{
//		CString csBuf1;
//		csBuf1.Format("nNumNode = %d;", nNumNode);
//		AfxMessageBox(csBuf1);
//	};
	
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
		for( i = 0, j = 0, njj = nX/(nY+1)+1; (i < nY)&&(j < nX) ; i++ )
		{
			for( jj = 0; (jj < njj)&&(j < nX); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			};
			for( jj = 0; (jj < njj)&&(j < nX); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
	}
	else
	{
		for( i = 0, j = 0, njj = nY/(nX+1)+1; (i < nX)&&(j < nY) ; i++ )
		{
			for( jj = 0; (jj < njj)&&(j < nY); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
		};
			for( jj = 0; (jj < njj)&&(j < nY); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
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
		for( i = 0, j = 0, njj = nX/(nY+1)+1; (i < nY)&&(j < nX) ; i++ )
		{
			for( jj = 0; (jj < njj)&&(j < nX); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			};
			for( jj = 0; (jj < njj)&&(j < nX); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
	}
	else
	{
		for( i = 0, j = 0, njj = nY/(nX+1)+1; (i < nX)&&(j < nY) ; i++ )
		{
			for( jj = 0; (jj < njj)&&(j < nY); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
		};
			for( jj = 0; (jj < njj)&&(j < nY); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
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
		if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
		if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
		if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
		if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
	};

//	{
//		CString csBuf1;
//		csBuf1.Format("nNumNode = %d, ii = %d, k2 = %d;", nNumNode, ii, k2);
//		AfxMessageBox(csBuf1);
//	};
	if( k2 > 2 ) df = ( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() )/( k2 - 2 );
	else df = 0.0;
	for( i = 0, r = pcNextBlockList->GetYmin(); i < k2; i++, r += df )
	{
		x = pcNextBlockList->GetXmax()*cos(r) + pcNextBlockList->GetX0();
		y = pcNextBlockList->GetXmax()*sin(r) + pcNextBlockList->GetY0();
		if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
		if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
		if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
		if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
	};

//	{
//		CString csBuf1;
//		csBuf1.Format("nNumNode = %d, ii = %d, k2 = %d;", nNumNode, ii, k2);
//		AfxMessageBox(csBuf1);
//	};
	DeleteDublicateData();
	SetAbsorberNodeType( cParam );

//	{
//		CString csBuf1;
//		csBuf1.Format("After dublicated nNumNode = %d, ii = %d, k2 = %d;", nNumNode, ii, k2);
//		AfxMessageBox(csBuf1);
//	};

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
	int i, j, k1, k2, nX, nY, ii, n_x, n_y;
	double r, x, y, x1, y1, df, dYAdmit;

	dYAdmit = 0.0;
	n_x = (int)((cParam.rXmax - cParam.rXmin)/cParam.rDelta + 1 );
	n_y = (int)((cParam.rYmax - cParam.rYmin)/cParam.rDelta + 1 );

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
	
	k1 = 10*(int)((pcNextBlockList->GetXmin()*( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() ) )/cParam.rDelta + 1 );
	nNumNode += k1;
	
	k2 = 10*(int)((pcNextBlockList->GetXmax()*( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() ) )/cParam.rDelta + 1 );
	nNumNode += k2;
	
	nNumNode *= 4;
//	{
//		CString csBuf1;
//		csBuf1.Format("nNumNode = %d;", nNumNode);
//		AfxMessageBox(csBuf1);
//	};
	
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
		for( i = 0, j = 0, njj = nX/(nY+1)+1; (i < nY)&&(j < nX) ; i++ )
		{
			for( jj = 0; (jj < njj)&&(j < nX); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			};
			for( jj = 0; (jj < njj)&&(j < nX); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
	}
	else
	{
		for( i = 0, j = 0, njj = nY/(nX+1)+1; (i < nX)&&(j < nY) ; i++ )
		{
			for( jj = 0; (jj < njj)&&(j < nY); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
		};
			for( jj = 0; (jj < njj)&&(j < nY); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
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
		for( i = 0, j = 0, njj = nX/(nY+1)+1; (i < nY)&&(j < nX) ; i++ )
		{
			for( jj = 0; (jj < njj)&&(j < nX); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			};
			for( jj = 0; (jj < njj)&&(j < nX); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
	}
	else
	{
		for( i = 0, j = 0, njj = nY/(nX+1)+1; (i < nX)&&(j < nY) ; i++ )
		{
			for( jj = 0; (jj < njj)&&(j < nY); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
			};
			x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
			y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
			if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
			if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
			if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
			if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
			pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
			pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
			pcNodeDielOne[ii].rY = (_real)(dYAdmit);
			pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
			ii++;
		};
			for( jj = 0; (jj < njj)&&(j < nY); j++, jj++ )
			{
				x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
				y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
				if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
				if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
				if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
				if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
				pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
				pcNodeDielOne[ii].rY = (_real)(dYAdmit);
				pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
				ii++;
				pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
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
		if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
		if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
		if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
		if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
	};

//	{
//		CString csBuf1;
//		csBuf1.Format("nNumNode = %d, ii = %d, k2 = %d;", nNumNode, ii, k2);
//		AfxMessageBox(csBuf1);
//	};
	if( k2 > 2 ) df = ( pcNextBlockList->GetYmax() - pcNextBlockList->GetYmin() )/( k2 - 2 );
	else df = 0.0;
	for( i = 0, r = pcNextBlockList->GetYmin(); i < k2; i++, r += df )
	{
		x = pcNextBlockList->GetXmax()*cos(r) + pcNextBlockList->GetX0();
		y = pcNextBlockList->GetXmax()*sin(r) + pcNextBlockList->GetY0();
		if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
		if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
		if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
		if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
		pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
		pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
		pcNodeDielOne[ii].rY = (_real)(dYAdmit);
		pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
		ii++;
	};

//	{
//		CString csBuf1;
//		csBuf1.Format("nNumNode = %d, ii = %d, k2 = %d;", nNumNode, ii, k2);
//		AfxMessageBox(csBuf1);
//	};
	DeleteDublicateData();
	SetAbsorberNodeType( cParam );

//	{
//		CString csBuf1;
//		csBuf1.Format("After dublicated nNumNode = %d, ii = %d, k2 = %d;", nNumNode, ii, k2);
//		AfxMessageBox(csBuf1);
//	};

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
	int i, j, k, nX, nY, ii, n_x, n_y;
	double r, x, y, x1, y1, dYAdmit;
//	CString csBuf1;
	dYAdmit = 0.0;
	n_x = (int)((cParam.rXmax - cParam.rXmin)/cParam.rDelta + 1 );
	n_y = (int)((cParam.rYmax - cParam.rYmin)/cParam.rDelta + 1 );

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
//	csBuf1.Format("nNumNode = %d;", nNumNode);
//	AfxMessageBox(csBuf1);
		

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
		if( nY < nX )
		{
			for( i = 0, j = 0, njj = nX/(nY+1)+1; (i < nY)&&(j < nX) ; i++ )
			{
				for( jj = 0; (jj < njj)&&(j < nX); j++, jj++ )
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
				};
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
			};
				for( jj = 0; (jj < njj)&&(j < nX); j++, jj++ )
				{
					x = x1 + pcNextBlockList->GetX0() + j*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + i*i_step*cParam.rDelta;
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
				};
		}
		else
		{
			for( i = 0, j = 0, njj = nY/(nX+1)+1; (i < nX)&&(j < nY) ; i++ )
			{
				for( jj = 0; (jj < njj)&&(j < nY); j++, jj++ )
				{
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
				};
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;

//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
			};
				for( jj = 0; (jj < njj)&&(j < nY); j++, jj++ )
				{
					x = x1 + pcNextBlockList->GetX0() + i*j_step*cParam.rDelta;
					y = y1 + pcNextBlockList->GetY0() + j*i_step*cParam.rDelta;
					if( x >= (cParam.rXmax-2*cParam.rDelta) ) x = (cParam.rXmax-cParam.rDelta*2.01);
					if( y >= (cParam.rYmax-2*cParam.rDelta) ) y = (cParam.rYmax-cParam.rDelta*2.01);
					if( x <= (cParam.rXmin+cParam.rDelta) ) x = (cParam.rXmin+cParam.rDelta*1.01);
					if( y <= (cParam.rYmin+cParam.rDelta) ) y = (cParam.rYmin+cParam.rDelta*1.01);
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)x, (_real)y );
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					pcNodeDielOne[ii].nNodeGlobal = cParam.CalcnNodeGlobal( (_real)(x), (_real)(y) )+n_x+1;
					pcNodeDielOne[ii].rU = (_ELEM_VAL_RTH)(0.0);
					pcNodeDielOne[ii].rY = (_real)(dYAdmit);
					pcNodeDielOne[ii].nType = CTMCRTH_BLCKNTYPE_ABSR_0000;
					ii++;
					
//	csBuf1.Format("x = %lg; y = %lg; nNodeGlobal[%d] = %d", x, y, ii, pcNodeDielOne[ii].nNodeGlobal);
//	AfxMessageBox(csBuf1);		
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

	nNumNode1 = nX*nY;
	if( nNumNode1 <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		return;
	};
	
	for( i = 0; ( fp = fopen( csFileNameEps , "wb") ) == NULL; i++ ) 
	{
		Sleep( 10 );
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
//	for( i = 0; i < nY; i++)
//	{
//		fprintf( fp, "%c%c", 13, 10);
//		for( j = 0; j < nX; j++ )
//		{
////			fprintf( fp, GetdFieldOutFormat(), prUNode1[ii]);
//			if( strcmp( GetdFieldOutFormat(), "") == 0 )
//			{
//				fwrite( &(prUNode1[ii]), sizeof(_real), 1, fp );
//			}
//			else
//			{
//				fprintf( fp, GetdFieldOutFormat(), prUNode1[ii]);
//			};
//			ii+=2;
//		};
//	};
	
	csBuf.Format("%d point in file %s", nNumNode1, csFileNameEps);
	PutTrace(csBuf);

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
			fwrite( &(pcNodeDielOne1.nNodeGlobal), sizeof(int), 1, fp );
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
	if( strncmp( csBlock, CTMCRTH_INDANBLCK_RECTMOVE, strlen(CTMCRTH_INDANBLCK_RECTSTAT) ) == NULL )
	{
		return;
	};
	if( strncmp( csBlock, CTMCRTH_INDANBLCK_CIRCMOVE, strlen(CTMCRTH_INDANBLCK_CIRCMOVE) ) == NULL )
	{
		return;
	};
	if( strncmp( csBlock, CTMCRTH_INDANBLCK_POLYGMOV, strlen(CTMCRTH_INDANBLCK_POLYGMOV) ) == NULL )
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
	
	strcpy( szBuf, csFileNameEps);
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
	int i, j, nX, nY, ii, nPoint, nNumNode1, nAccur, jj;
	double dXmin, dXmin1, dXmax, dYmin, dYmin1, dYmax, r, x, y;
	char ch[ CTMCRTH_INDANSBUF ];
	sTmcRTHNodeDielOne *pcNodeDielOne1;
	FILE *pfData;
	CString csBufTr;

	csBufTr.Format("Block %d read file %s", nBlock, csEpsExpr);
	PutTrace(csBufTr);

	if( (pfData = fopen( csEpsExpr, "rb")) == NULL )
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
		csBuf.Format("bad {#PARAM delta = %lg} != {delta = %lg} in diel.data file {%s} for block %d", cParam.rDelta, r, TMC_RTH_EPSFILE_Delta_, csEpsExpr, nBlock);
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
//{
//	CString csBuf1;
//	csBuf1.Format("{ Ymin = %lg; Ymax = %lg; nY = %d }", dYmin1, dYmin1 + (nY-1)*cParam.rDelta, nY );
//	AfxMessageBox(csBuf1);
//};

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
	
	if( nAccur == sizeof( _real ) )
	{
		for( i = 0, jj = 0; i < nPoint; i++ )
		{
			if( fread( &(pcNodeDielOne1[i].rY), nAccur, 1, pfData ) != 1 )
			{
				CString csBuf;
				csBuf.Format("bad data array in diel.data file {%s} for block %d", TMC_RTH_EPSFILE_Delta_, csEpsExpr, nBlock);
				cError.PutErrorMessage(csBuf);
				fclose( pfData );
				delete pcNodeDielOne1;
				return cError;
			};
			if( fread( &(pcNodeDielOne1[i].nNodeGlobal), sizeof(int), 1, pfData ) != 1 )
			{
				CString csBuf;
				csBuf.Format("bad data node array in diel.data file {%s} for block %d", TMC_RTH_EPSFILE_Delta_, csEpsExpr, nBlock);
				cError.PutErrorMessage(csBuf);
				fclose( pfData );
				delete pcNodeDielOne1;
				return cError;
			};
			if( !pbIsStop[0] )
			{
				CString csBuf;
				csBuf.Format("User cancel calculated eps array thread in Block %d", nBlock);
				cError.PutErrorMessage(csBuf);
				fclose( pfData );
				delete pcNodeDielOne1;
				return cError;
			};
			if( jj == 10000 )
			{
				csBufTr.Format("Read n:%d", i);
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
							csBuf.Format("bad data array in diel.data file {%s} for block %d", TMC_RTH_EPSFILE_Delta_, csEpsExpr, nBlock);
							cError.PutErrorMessage(csBuf);
							fclose( pfData );
							delete pcNodeDielOne1;
							return cError;
						};
						pcNodeDielOne1[i].rY = fBuf;
						if( fread( &(pcNodeDielOne1[i].nNodeGlobal), sizeof(int), 1, pfData ) != 1 )
						{
							CString csBuf;
							csBuf.Format("bad data node array in diel.data file {%s} for block %d", TMC_RTH_EPSFILE_Delta_, csEpsExpr, nBlock);
							cError.PutErrorMessage(csBuf);
							fclose( pfData );
							delete pcNodeDielOne1;
							return cError;
						};
						if( !pbIsStop[0] )
						{
							CString csBuf;
							csBuf.Format("User cancel calculated eps array thread in Block %d", nBlock);
							cError.PutErrorMessage(csBuf);
							fclose( pfData );
							delete pcNodeDielOne1;
							return cError;
						};
						if( jj == 10000 )
						{
							csBufTr.Format("Read n:%d", i);
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
							csBuf.Format("bad data array in diel.data file {%s} for block %d", TMC_RTH_EPSFILE_Delta_, csEpsExpr, nBlock);
							cError.PutErrorMessage(csBuf);
							fclose( pfData );
							delete pcNodeDielOne1;
							return cError;
						};
						pcNodeDielOne1[i].rY = (_real)(dBuf);
						if( fread( &(pcNodeDielOne1[i].nNodeGlobal), sizeof(int), 1, pfData ) != 1 )
						{
							CString csBuf;
							csBuf.Format("bad data node array in diel.data file {%s} for block %d", TMC_RTH_EPSFILE_Delta_, csEpsExpr, nBlock);
							cError.PutErrorMessage(csBuf);
							fclose( pfData );
							delete pcNodeDielOne1;
							return cError;
						};
						if( !pbIsStop[0] )
						{
							CString csBuf;
							csBuf.Format("User cancel calculated eps array thread in Block %d", nBlock);
							cError.PutErrorMessage(csBuf);
							fclose( pfData );
							delete pcNodeDielOne1;
							return cError;
						};
						if( jj == 10000 )
						{
							csBufTr.Format("Read n:%d", i);
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
					delete pcNodeDielOne1;
					fclose( pfData );
					return cError;
				};
				break;
		};
	};

	fclose( pfData );

	for( i = 0, ii = 0, jj = 0; i < nPoint; i++ )
	{
		j = pcNodeDielOne1[i].nNodeGlobal;
		x = dXmin1 + pcNextBlockList->GetX0() + ( j - ((int)((j)/nX))*nX )*cParam.rDelta;
		y = dYmin1 + pcNextBlockList->GetY0() + ((int)((j)/nX))*cParam.rDelta;
		if( (x <= dXmax)&&(x >= dXmin)&&(y <= dYmax)&&(y >= dYmin) ) ii++;
		if( !pbIsStop[0] )
		{
			CString csBuf;
			csBuf.Format("User cancel calculated eps array thread in Block %d", nBlock);
			cError.PutErrorMessage(csBuf);
			delete pcNodeDielOne1;
			return cError;
		};
		if( jj == 10000 )
		{
			csBufTr.Format("Load n:%d %d", ii, i);
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
		delete pcNodeDielOne1;
		return cError;
	}

	nNumNode = ii;
	
	if( nNumNode <= 0)
	{
		CString csBuf;
		csBuf.Format("nNumNode <= 0 for block %d", nBlock);
		cError.PutErrorMessage(csBuf);
		delete pcNodeDielOne1;
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
		delete pcNodeDielOne1;
		return cError;
	};

	for( i = 0, ii = 0, jj = 0; i < nPoint; i++ )
	{
		j = pcNodeDielOne1[i].nNodeGlobal;
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
			delete pcNodeDielOne1;
			return cError;
		};
		if( jj == 10000 )
		{
			csBufTr.Format("Load n:%d %d", ii, i);
			PutStatistics(csBufTr);
			jj = 0;
		};
		jj++;
	};

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

	sprintf( szBuf1, "t=(%lg);f=(%lg);", t, f);
	if( i1nte_atof_1( szBuf1, &r1 ) != 0 )
	{
		csBuf.Format("In{%s}%s", szBuf1, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return (double)(0.0);
	};
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

	sprintf( szBuf1, "t=(%lg);f=(%lg);", t, f);
	if( i1nte_atof_1( szBuf1, &r1 ) != 0 )
	{
		csBuf.Format("In{%s}%s", szBuf1, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return (double)(0.0);
	};
	if( i1nte_atof_1( szBuf, &r1 ) != 0 )
	{
		csBuf.Format("In{%s}%s; t=(%lg);f=(%lg);",
			szBuf, expr_get_error(), t, f );
		cError.PutErrorMessage(csBuf);
		return (double)(0.0);
	};

	return -r1/cParam.rAngleUnit;
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

void CTmcRTHNodeDiel::SetMagneticPolygonType( CTmcRTH_IndanParam &cParam )
{
	int i;
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
//						cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) );
//						AfxMessageBox(csBuf);
				};
				break;
			case CTMCRTH_BLCKNTYPE_ABSR_1100:
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) < (cParam.rXmin + 1.5*cParam.rDelta) )
				{
					pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_ABSR_1101;
//						csBuf.Format("x = %lg; set type CTMCRTH_BLCKNTYPE_ABSR_0111",
//						cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) );
//						AfxMessageBox(csBuf);
				};
				break;
			case CTMCRTH_BLCKNTYPE_ABSR_1001:
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > (cParam.rXmax - 2.5*cParam.rDelta) )
				{
					pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_ABSR_1101;
//						csBuf.Format("x = %lg; set type CTMCRTH_BLCKNTYPE_ABSR_1101",
//						cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) );
//						AfxMessageBox(csBuf);
				};
				break;
			case CTMCRTH_BLCKNTYPE_ABSR_0011:
				if( cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) > (cParam.rXmax - 2.5*cParam.rDelta) )
				{
					pcNodeDielOne[i].nType = CTMCRTH_BLCKNTYPE_ABSR_0111;
//						csBuf.Format("x = %lg; set type CTMCRTH_BLCKNTYPE_ABSR_0111",
//						cParam.CalcX(pcNodeDielOne[i].nNodeGlobal) );
//						AfxMessageBox(csBuf);
				};
				break;
			default:
				break;
		};
	};
	return;
}
