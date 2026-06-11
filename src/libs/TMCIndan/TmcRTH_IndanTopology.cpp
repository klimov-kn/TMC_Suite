// TmcRTH_IndanTopology.cpp : implementation file
//

#include "stdafx.h"
#include <tmcgrviw.h>
#include <TmcRTH_IndanTopology.h>
#include <TmcRTH_Indan.h>
#include <math.h>
#include <expr.h>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_IndanTopology

CTmcRTH_IndanTopology::CTmcRTH_IndanTopology()
{
	pcParam = NULL;
}

CTmcRTH_IndanTopology::~CTmcRTH_IndanTopology()
{
}


//BEGIN_MESSAGE_MAP(CTmcRTH_IndanTopology, CWnd)
	//{{AFX_MSG_MAP(CTmcRTH_IndanTopology)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_IndanTopology message handlers

void CTmcRTH_IndanTopology::DeleteData( void )
{
	pcParam = NULL;
	cBlockList.DeleteData();
	return;
}

void CTmcRTH_IndanTopology::Read( CString &csCh, CTmcLibError &cError )
{
	cBlockList.Add( csCh, cError, pcParam );
	return;
}

void CTmcRTH_IndanTopology::ReadLink( CString &csCh, CTmcLibError &cError )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	char szBuf1[CTMCRTH_INDANSBUF];
	int i;
	CString csBuf;
	int iBuf;
	double rBuf;
	_real  x0, y0;

	if( pcParam == NULL )
	{
		csBuf.Format("section %s isn't init in %s", CTMCRTH_INDANMET_PARAM, CTMCRTH_INDANMET_LINKLIST);
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( strncmp( csCh, CTMCRTH_INDANMET_LINK, strlen(CTMCRTH_INDANMET_LINK) ) != 0 )
	{
		csBuf.Format( "Unknown keyword {%s} in %s section use {%s}", csCh, CTMCRTH_INDANMET_LINKLIST, CTMCRTH_INDANMET_LINK );
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csCh );

	i = (int)(strlen(CTMCRTH_INDANMET_LINK));

	i+= Search_1Param( csBuf, szBuf+i, cError);
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &rBuf ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	iBuf = (int)(rBuf);

	i+= Search_1Param( csBuf, szBuf+i, cError);
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &rBuf ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	x0 = (_real)(rBuf*pcParam->rGetLongUnit());

	i+= Search_1Param( csBuf, szBuf+i, cError);
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &rBuf ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	y0 = (_real)(rBuf*pcParam->rGetLongUnit());

	cBlockList.SetLink( iBuf, x0, y0, cError );
	if( cError.IsError() ) return;

	CTmcRTH_BlockList *pcBl;
	pcBl = cBlockList.FindBlock( iBuf );
	if( pcBl->GetnType() == CTMCRTH_BLCKNTYPE_INPXLEFT )
	{
/*
		if( fabs( (double)(pcParam->rXmin - x0) ) <= (pcParam->rDelta) )
		{
			return;
		};
		if( fabs( (double)(pcParam->rXmax - x0) ) <= (pcParam->rDelta) )
		{
			pcBl->SetnType(CTMCRTH_BLCKNTYPE_INPXRIGHT);
			return;
		};
		csBuf.Format("%s %d %s in %s section x0 must be %lg - for left input or %lg - for right input", 
			CTMCRTH_INDANMET_BLOCK, iBuf, CTMCRTH_INDANBLCK_INPUT_X,
			CTMCRTH_INDANMET_LINKLIST, pcParam->rXmin/pcParam->rGetLongUnit(), 
			pcParam->rXmax/pcParam->rGetLongUnit());
		cError.PutErrorMessage(csBuf);
		return;
*/
		if( fabs( (double)(pcParam->rXmin - x0) ) <= (pcParam->rDelta) )
		{
			return;
		};
		if( fabs( (double)(pcParam->rXmax - x0) ) <= (pcParam->rDelta) )
		{
			pcBl->SetnType(CTMCRTH_BLCKNTYPE_INPXRIGHT);
			return;
		};
		if( (( (double) (x0) ) >= ((double)(pcParam->rXmin)) )&&
			(( (double) (x0) ) <= ((double)((pcParam->rXmin + pcParam->rXmax)/2.)) ) )
		{
			return;
		};
		if( (( (double) (x0) ) <= ((double)(pcParam->rXmax)) )&&
			(( (double) (x0) ) >= ((double)((pcParam->rXmin + pcParam->rXmax)/2.)) ) )
		{
			pcBl->SetnType(CTMCRTH_BLCKNTYPE_INPXRIGHT);
			return;
		};

{
	CString csBuf1;
	csBuf1.Format("Function SetLink\nx0 = %lg;\ny0 = %lg;\nXmin = %lg;\nXmax = %lg;", x0, y0, pcParam->rXmin, pcParam->rXmax );
	AfxMessageBox(csBuf1);
};

		csBuf.Format("%s %d %s in %s section x0 must be %lg - for left input or %lg - for right input", 
			CTMCRTH_INDANMET_BLOCK, iBuf, CTMCRTH_INDANBLCK_INPUT_X,
			CTMCRTH_INDANMET_LINKLIST, pcParam->rXmin/pcParam->rGetLongUnit(), 
			pcParam->rXmax/pcParam->rGetLongUnit());
		cError.PutErrorMessage(csBuf);
		return;
	}

	if( pcBl->GetnType() == CTMCRTH_BLCKNTYPE_INPYTOP )
	{
/*
		if( fabs( (double)(pcParam->rYmin - y0) ) <= (pcParam->rDelta) )
		{
			pcBl->SetnType(CTMCRTH_BLCKNTYPE_INPYBOT);
			return;
		};
		if( fabs( (double)(pcParam->rYmax - y0) ) <= (pcParam->rDelta) )
		{
			return;
		};
*/
		if( fabs( (double)(pcParam->rYmin - y0) ) <= (pcParam->rDelta) )
		{
			pcBl->SetnType(CTMCRTH_BLCKNTYPE_INPYBOT);
			return;
		};
		if( fabs( (double)(pcParam->rYmax - y0) ) <= (pcParam->rDelta) )
		{
			return;
		};
		if( (( (double) (y0) ) >= ((double)(pcParam->rYmin)) )&&
			(( (double) (y0) ) <= ((double)((pcParam->rYmin + pcParam->rYmax)/2.)) ) )
		{
			pcBl->SetnType(CTMCRTH_BLCKNTYPE_INPYBOT);
			return;
		};
		if( (( (double) (y0) ) <= ((double)(pcParam->rYmax)) )&&
			(( (double) (y0) ) >= ((double)((pcParam->rYmin + pcParam->rYmax)/2.)) ) )
		{
			return;
		};
		csBuf.Format("%s %d %s in %s section y0 must be %lg - for left input or %lg - for right input", 
			CTMCRTH_INDANMET_BLOCK, iBuf, CTMCRTH_INDANBLCK_INPUT_Y,
			CTMCRTH_INDANMET_LINKLIST, pcParam->rYmin/pcParam->rGetLongUnit(), 
			pcParam->rYmax/pcParam->rGetLongUnit());
		cError.PutErrorMessage(csBuf);
		return;
	}
	
	
	return;
}

void CTmcRTH_IndanTopology::IsCorrectly( CTmcLibError &cError )
{
	cBlockList.IsCorrectly( cError );
	return;
}

void CTmcRTH_IndanTopology::IsCorrectlyLink( CTmcLibError &cError )
{
	cBlockList.IsCorrectlyLink( cError );
	return;
}

int CTmcRTH_IndanTopology::Search_1Param( CString &csBuf, char *szBuf, CTmcLibError &cError )
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

int CTmcRTH_IndanTopology::expr_del_Blanks2( char *ch )
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

void CTmcRTH_IndanTopology::SetParam( CTmcRTH_IndanParam * pcParam1 )
{
	pcParam = pcParam1;
	return;
}

CTmcRTH_BlockList * CTmcRTH_IndanTopology::GetBlockList( void )
{
	return &cBlockList;
}

CTmcRTH_IndanParam * CTmcRTH_IndanTopology::GetpParam( void )
{
	return pcParam;
}

CTmcRTH_IndanTopology& CTmcRTH_IndanTopology::operator=( CTmcRTH_IndanTopology& cTopol )
{
	pcParam = cTopol.GetpParam();
	cBlockList = *cTopol.GetBlockList();
	return (*this);
}

int CTmcRTH_IndanTopology::GetBlockNumber( void )
{
	return cBlockList.GetBlockNumber();
}

BOOL CTmcRTH_IndanTopology::IsDataRead( void )
{
	if( GetBlockNumber() >= 0 ) return TRUE;
	return FALSE;
}

void CTmcRTH_IndanTopology::AddPathForFile(CString &csCurrentPath)
{
	cBlockList.AddPathForFile( csCurrentPath );
	return;
}
