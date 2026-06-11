// TmcRTH_IndanParam.cpp : implementation file
//

#include "stdafx.h"
#include "PlanRT_H.h"
#include <limits.h>
#include <float.h>
#include <math.h>
#include <typerth.h>
#include "TmcRTH_Indan.h"
#include "TmcRTH_IndanParam.h"
#include <expr.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_IndanParam

CTmcRTH_IndanParam::CTmcRTH_IndanParam()
{
	expr_set_angle( EXPR_RADIAN );
	csFreqUnit.Format(CTMCRTH_FREQUNIT_GHz);
	rFreqUnit = (_real)(CTMCRTH_FREQUNIT_UGHz);
	csLongUnit.Format(CTMCRTH_LONGUNIT_mm);
	rLongUnit = (_real)(CTMCRTH_LONGUNIT_Umm);
	csTimeUnit.Format(CTMCRTH_TIMEUNIT_s);
	rTimeUnit = (_real)(CTMCRTH_TIMEUNIT_Us);
	csAngleUnit.Format(CTMCRTH_ANGLEUNIT_r);
	rAngleUnit = (_real)(CTMCRTH_ANGLEUNIT_Ur);
	rDelta = (_real)(0.0);
	rFreq = (_real)(0.0);
	rt = (_real)(0.0);
	rXmin = (_real)(0.0);
	rXmax = (_real)(0.0);
	rYmin = (_real)(0.0);
	rYmax = (_real)(0.0);
	rTmin = (_real)(0.0);
	rTmax = (_real)(0.0);
	return;
}

CTmcRTH_IndanParam::~CTmcRTH_IndanParam()
{
	DeleteData();
	return;
}


BEGIN_MESSAGE_MAP(CTmcRTH_IndanParam, CWnd)
	//{{AFX_MSG_MAP(CTmcRTH_IndanParam)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_IndanParam message handlers

void CTmcRTH_IndanParam::DeleteData( void )
{
	expr_set_angle( EXPR_RADIAN );
	cError.Clear();
	csFreqUnit.Format(CTMCRTH_FREQUNIT_GHz);
	rFreqUnit = (_real)(CTMCRTH_FREQUNIT_UGHz);
	csLongUnit.Format(CTMCRTH_LONGUNIT_mm);
	rLongUnit = (_real)(CTMCRTH_LONGUNIT_Umm);
	csTimeUnit.Format(CTMCRTH_TIMEUNIT_s);
	rTimeUnit = (_real)(CTMCRTH_TIMEUNIT_Us);
	csAngleUnit.Format(CTMCRTH_ANGLEUNIT_r);
	rAngleUnit = (_real)(CTMCRTH_ANGLEUNIT_Ur);
	rDelta = (_real)(0.0);
	rFreq = (_real)(0.0);
	rt = (_real)(0.0);
	rXmin = (_real)(0.0);
	rXmax = (_real)(0.0);
	rYmin = (_real)(0.0);
	rYmax = (_real)(0.0);
	rTmin = (_real)(0.0);
	rTmax = (_real)(0.0);
	return;
}

void CTmcRTH_IndanParam::Read( CString &csCh, CTmcLibError &cError1 )
{
	if( cError1.IsError() ) 
	{
		return;
	};
	if( cError.IsError() ) 
	{
		cError1 = cError;
		return;
	};

	if( strncmp( csCh, CTMCRTH_LONGUNIT_ID, strlen(CTMCRTH_LONGUNIT_ID) ) == 0 )
	{
		ReadLongUnit(csCh);
		cError1 = cError;
		return;
	};
	if( strncmp( csCh, CTMCRTH_TIMEUNIT_ID, strlen(CTMCRTH_TIMEUNIT_ID) ) == 0 )
	{
		ReadTimeUnit(csCh);
		cError1 = cError;
		return;
	};
	if( strncmp( csCh, CTMCRTH_ANGLEUNIT_ID, strlen(CTMCRTH_ANGLEUNIT_ID) ) == 0 )
	{
		ReadAngleUnit(csCh);
		cError1 = cError;
		return;
	};
	if( strncmp( csCh, CTMCRTH_FREQUNIT_ID, strlen(CTMCRTH_FREQUNIT_ID) ) == 0 )
	{
		ReadFreqUnit(csCh);
		cError1 = cError;
		return;
	};
	if( strncmp( csCh, CTMCRTH_DELTA_ID, strlen(CTMCRTH_DELTA_ID) ) == 0 )
	{
		ReadDelta(csCh);
		cError1 = cError;
		return;
	};
	if( strncmp( csCh, CTMCRTH_FREQ_ID, strlen(CTMCRTH_FREQ_ID) ) == 0 )
	{
		ReadFreq(csCh);
		cError1 = cError;
		return;
	};
	if( strncmp( csCh, CTMCRTH_TIME_ID, strlen(CTMCRTH_TIME_ID) ) == 0 )
	{
		ReadTime(csCh);
		cError1 = cError;
		return;
	};
	if( strncmp( csCh, CTMCRTH_XMIN_ID, strlen(CTMCRTH_XMIN_ID) ) == 0 )
	{
		ReadXmin(csCh);
		cError1 = cError;
		return;
	};
	if( strncmp( csCh, CTMCRTH_XMAX_ID, strlen(CTMCRTH_XMAX_ID) ) == 0 )
	{
		ReadXmax(csCh);
		cError1 = cError;
		return;
	};
	if( strncmp( csCh, CTMCRTH_YMIN_ID, strlen(CTMCRTH_YMIN_ID) ) == 0 )
	{
		ReadYmin(csCh);
		cError1 = cError;
		return;
	};
	if( strncmp( csCh, CTMCRTH_YMAX_ID, strlen(CTMCRTH_YMAX_ID) ) == 0 )
	{
		ReadYmax(csCh);
		cError1 = cError;
		return;
	};

	CString csBuf;
	csBuf.Format( "Unknown keyword {%s}", csCh );
	cError.PutErrorMessage(csBuf);
	cError1 = cError;
	return;
}


void CTmcRTH_IndanParam::ReadXmax( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	int i;
	double r;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = strlen(CTMCRTH_XMAX_ID);

	i+= Search_1Param( csBuf, szBuf+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	rXmax = (_real)(r * rLongUnit);
	
	return;
}
void CTmcRTH_IndanParam::ReadXmin( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	int i;
	double r;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = strlen(CTMCRTH_XMIN_ID);

	i+= Search_1Param( csBuf, szBuf+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	rXmin = (_real)(r * rLongUnit);
	
	return;
}

void CTmcRTH_IndanParam::ReadYmax( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	double r;
	int i;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = strlen(CTMCRTH_YMAX_ID);

	i+= Search_1Param( csBuf, szBuf+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	rYmax = (_real)(r * rLongUnit);
	
	return;
}

void CTmcRTH_IndanParam::ReadYmin( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	double r;
	int i;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = strlen(CTMCRTH_YMIN_ID);

	i+= Search_1Param( csBuf, szBuf+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	rYmin = (_real)(r * rLongUnit);
	
	return;
}

int CTmcRTH_IndanParam::Search_1Param( CString &csBuf, char *szBuf )
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

int CTmcRTH_IndanParam::expr_del_Blanks2( char *ch )
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

void CTmcRTH_IndanParam::ReadDelta( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	double r;
	int i;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = strlen(CTMCRTH_DELTA_ID);

	i+= Search_1Param( csBuf, szBuf+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	rDelta = (_real)(r * rLongUnit);
	rt = (_real)(rDelta*sqrt(0.5*MU0___*EPS0___));
	
	return;
}

void CTmcRTH_IndanParam::ReadTime( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	char szBuf1[CTMCRTH_INDANSBUF];
	double r;
	int i;
	CString csBuf;

	strcpy( szBuf, csCh );

	i = strlen(CTMCRTH_TIME_ID);

	i+= Search_1Param( csBuf, szBuf+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	rTmin = (_real)(r * rTimeUnit);

	i+= Search_1Param( csBuf, szBuf+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf1, csBuf );
	expr_del_Blanks2( szBuf1 );
	if( i1nte_atof_1( szBuf1, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	rTmax = (_real)(r * rTimeUnit);
	
	return;
}

void CTmcRTH_IndanParam::ReadFreqUnit( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	int i;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = strlen(CTMCRTH_FREQUNIT_ID);

	i+= Search_1Param( csBuf, szBuf+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );

	if( strcmp( szBuf, CTMCRTH_FREQUNIT_GHz ) == 0 )
	{
		csFreqUnit.Format( "%s", szBuf);
		rFreqUnit = (_real)(CTMCRTH_FREQUNIT_UGHz);
		return;
	};
	if( strcmp( szBuf, CTMCRTH_FREQUNIT_MHz ) == 0 )
	{
		csFreqUnit.Format( "%s", szBuf);
		rFreqUnit = (_real)(CTMCRTH_FREQUNIT_UMHz);
		return;
	};
	if( strcmp( szBuf, CTMCRTH_FREQUNIT_kHz ) == 0 )
	{
		csFreqUnit.Format( "%s", szBuf);
		rFreqUnit = (_real)(CTMCRTH_FREQUNIT_UkHz);
		return;
	};
	if( strcmp( szBuf, CTMCRTH_FREQUNIT_Hz ) == 0 )
	{
		csFreqUnit.Format( "%s", szBuf);
		rFreqUnit = (_real)(CTMCRTH_FREQUNIT_UHz);
		return;
	};

	csBuf.Format("{%s}use following %s: %s, %s, %s, %s", csCh, CTMCRTH_FREQUNIT_ID,
		CTMCRTH_FREQUNIT_GHz, CTMCRTH_FREQUNIT_MHz, CTMCRTH_FREQUNIT_kHz, CTMCRTH_FREQUNIT_Hz);
	cError.PutErrorMessage(csBuf);
	return;
}

void CTmcRTH_IndanParam::ReadLongUnit( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	int i;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = strlen(CTMCRTH_LONGUNIT_ID);

	i+= Search_1Param( csBuf, szBuf+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );

	if( strcmp( szBuf, CTMCRTH_LONGUNIT_mm ) == 0 )
	{
		csLongUnit.Format( "%s", szBuf);
		rLongUnit = (_real)(CTMCRTH_LONGUNIT_Umm);
		return;
	};
	if( strcmp( szBuf, CTMCRTH_LONGUNIT_m ) == 0 )
	{
		csLongUnit.Format( "%s", szBuf);
		rLongUnit = (_real)(CTMCRTH_LONGUNIT_Um);
		return;
	};
	if( strcmp( szBuf, CTMCRTH_LONGUNIT_cm ) == 0 )
	{
		csLongUnit.Format( "%s", szBuf);
		rLongUnit = (_real)(CTMCRTH_LONGUNIT_Ucm);
		return;
	};

	csBuf.Format("{%s}use following %s: %s, %s, %s", csCh, CTMCRTH_LONGUNIT_ID,
		CTMCRTH_LONGUNIT_m, CTMCRTH_LONGUNIT_cm, CTMCRTH_LONGUNIT_mm);
	cError.PutErrorMessage(csBuf);
	return;
	
}

void CTmcRTH_IndanParam::ReadTimeUnit( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	int i;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = strlen(CTMCRTH_TIMEUNIT_ID);

	i+= Search_1Param( csBuf, szBuf+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );

	if( strcmp( szBuf, CTMCRTH_TIMEUNIT_s ) == 0 )
	{
		csTimeUnit.Format( "%s", szBuf);
		rTimeUnit = (_real)(CTMCRTH_TIMEUNIT_Us);
		return;
	};
	if( strcmp( szBuf, CTMCRTH_TIMEUNIT_ms ) == 0 )
	{
		csTimeUnit.Format( "%s", szBuf);
		rTimeUnit = (_real)(CTMCRTH_TIMEUNIT_Ums);
		return;
	};
	if( strcmp( szBuf, CTMCRTH_TIMEUNIT_mks ) == 0 )
	{
		csTimeUnit.Format( "%s", szBuf);
		rTimeUnit = (_real)(CTMCRTH_TIMEUNIT_Umks);
		return;
	}; 
	if( strcmp( szBuf, CTMCRTH_TIMEUNIT_ns ) == 0 )
	{
		csTimeUnit.Format( "%s", szBuf);
		rTimeUnit = (_real)(CTMCRTH_TIMEUNIT_Uns);
		return;
	};
	if( strcmp( szBuf, CTMCRTH_TIMEUNIT_ps ) == 0 )
	{
		csTimeUnit.Format( "%s", szBuf);
		rTimeUnit = (_real)(CTMCRTH_TIMEUNIT_Ups);
		return;
	};

	csBuf.Format("{%s}use following %s: %s, %s, %s, %s, %s", csCh, CTMCRTH_TIMEUNIT_ID,
		CTMCRTH_TIMEUNIT_s, CTMCRTH_TIMEUNIT_ms, 
		CTMCRTH_TIMEUNIT_mks, CTMCRTH_TIMEUNIT_ns, 
		CTMCRTH_TIMEUNIT_ps);
	cError.PutErrorMessage(csBuf);
	return;
	
}

void CTmcRTH_IndanParam::ReadAngleUnit( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	int i;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = strlen(CTMCRTH_ANGLEUNIT_ID);

	i+= Search_1Param( csBuf, szBuf+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );

	if( strcmp( szBuf, CTMCRTH_ANGLEUNIT_r ) == 0 )
	{
		csAngleUnit.Format( "%s", szBuf);
		rAngleUnit = (_real)(CTMCRTH_ANGLEUNIT_Ur);
		expr_set_angle( EXPR_RADIAN );
		return;
	};
	if( strcmp( szBuf, CTMCRTH_ANGLEUNIT_gr ) == 0 )
	{
		csAngleUnit.Format( "%s", szBuf);
		rAngleUnit = (_real)(CTMCRTH_ANGLEUNIT_Ugr);
		expr_set_angle( EXPR_GRADUS );
		return;
	};

	csBuf.Format("{%s}use following %s: %s, %s", csCh, CTMCRTH_ANGLEUNIT_ID,
		CTMCRTH_ANGLEUNIT_r, CTMCRTH_ANGLEUNIT_gr);
	cError.PutErrorMessage(csBuf);
	return;
	
}


void CTmcRTH_IndanParam::IsCorrectly( CTmcLibError &cError1 )
{
	CString csBuf;

	if( cError1.IsError() ) return;
	
	if( cError.IsError() ) 
	{
		cError1 = cError;
		return;
	}
	
	if( rYmin >= rYmax )
	{
		csBuf.Format("Section %s Ymin(%lg) >= Ymax(%lg)", CTMCRTH_INDANMET_PARAM, rYmin, rYmax);
		cError.PutErrorMessage(csBuf);
		cError1 = cError;
		return;
	};
	
	if( rXmin >= rXmax )
	{
		csBuf.Format("Section %s Xmin(%lg) >= Xmax(%lg)", CTMCRTH_INDANMET_PARAM, rXmin, rXmax);
		cError.PutErrorMessage(csBuf);
		cError1 = cError;
		return;
	};

	if( rTmin >= rTmax )
	{
		csBuf.Format("Section %s Tmin(%lg) >= Tmax(%lg)", CTMCRTH_INDANMET_PARAM, rTmin, rTmax);
		cError.PutErrorMessage(csBuf);
		cError1 = cError;
		return;
	};
	
	if( rDelta <= 0. )
	{
		csBuf.Format("Section %s Delta(%lg) <= 0", CTMCRTH_INDANMET_PARAM, rDelta );
		cError.PutErrorMessage(csBuf);
		cError1 = cError;
		return;
	};
	
	if( rFreq <= 0. )
	{
		csBuf.Format("Section %s Freq(%lg) <= 0", CTMCRTH_INDANMET_PARAM, rFreq );
		cError.PutErrorMessage(csBuf);
		cError1 = cError;
		return;
	};
	
	if( rDelta > (C0___/(5.*rFreq)) )
	{
		csBuf.Format("Section %s Delta(%lg) > WaveLenght(%lg)/5; make please parameter %s less than %lg %s",
			CTMCRTH_INDANMET_PARAM, rDelta, (300000000./rFreq),
			CTMCRTH_DELTA_ID, (C0___/(5.*rFreq))/rLongUnit, csLongUnit);
		cError.PutErrorMessage(csBuf);
		cError1 = cError;
		return;
	};

//////////////////////////////////////////////////
//////////////////////////////////////////////////
	int nX111, nY111;
	nX111 = CalcnX();
	nY111 = CalcnY();
	rXmax = rXmin + nX111*rDelta;
	rYmax = rYmin + nY111*rDelta;
//////////////////////////////////////////////////
//////////////////////////////////////////////////

	
	return;
}

void CTmcRTH_IndanParam::ReadFreq( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	double r;
	int i;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = strlen(CTMCRTH_FREQ_ID);

	i+= Search_1Param( csBuf, szBuf+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	rFreq = (_real)(r * rFreqUnit);
	
	return;
}

double CTmcRTH_IndanParam::rGetLongUnit( void )
{
	return rLongUnit;
}

CString CTmcRTH_IndanParam::csGetLongUnit( void )
{
	return csLongUnit;
}



CTmcRTH_IndanParam& CTmcRTH_IndanParam::operator=( CTmcRTH_IndanParam& cParam )
{
	cError = cError;
	csFreqUnit  = cParam.csFreqUnit;
	rFreqUnit   = cParam.rFreqUnit;
	csLongUnit  = cParam.csLongUnit;
	rLongUnit   = cParam.rLongUnit;
	csTimeUnit  = cParam.csTimeUnit;
	rTimeUnit   = cParam.rTimeUnit;
	csAngleUnit = cParam.csAngleUnit;
	rAngleUnit  = cParam.rAngleUnit;
	rDelta = cParam.rDelta;
	rFreq  = cParam.rFreq;
	rt     = cParam.rt;
	rXmin  = cParam.rXmin;
	rXmax  = cParam.rXmax;
	rYmin  = cParam.rYmin;
	rYmax  = cParam.rYmax;
	rTmin  = cParam.rTmin;
	rTmax  = cParam.rTmax;
	return (*this);
}

int CTmcRTH_IndanParam::nGetTmax( void )
{
	int i;
	double r;

	r = (rTmax - rTmin)/rt;

	if( r > INT_MAX ) return 0;

	i = (int)(r);

	if( i <= 0 ) return 0;
	return i;
}

double CTmcRTH_IndanParam::dGetTmin( void )
{
	double r;
	r = (double)(rTmin);
	return r;
}

double CTmcRTH_IndanParam::dGetdT( void )
{
	double r;
	r = (double)(rt);
	return r;
}

double CTmcRTH_IndanParam::dGetDelta( void )
{
	double r;
	r = (double)(rDelta);
	return r;
}

void CTmcRTH_IndanParam::GetXYMinMax( double &dXmin, double &dXmax, double &dYmin, double &dYmax )
{
	double r;
	r = (double)(rXmin);
	dXmin = r;
	r = (double)(rXmax);
	dXmax = r;
	r = (double)(rYmin);
	dYmin = r;
	r = (double)(rYmax);
	dYmax = r;
	return;
}


double CTmcRTH_IndanParam::CalcX( int nNumber )
{
	double r;
	int nY, nX;

	nX = (int)(( rXmax - rXmin )/rDelta)+1;
	nY = nNumber - nX*((int)(nNumber/nX));
	r = rXmin + nY*rDelta;

	return r;
}

double CTmcRTH_IndanParam::CalcY( int nNumber )
{
	double r;
	int nY, nX;

	nX = (int)(( rXmax - rXmin )/rDelta)+1;
	nY = nNumber/nX;
	r = rYmin + nY*rDelta;

	return r;
}

int CTmcRTH_IndanParam::CalcnNodeGlobal( double x, double y )
{
	int n, nX, nY;

	n = 0;
	nX = CalcnX();
	nY = CalcnY();

	if( x > (double)rXmax )	x = (double)rXmax;
	if( x < (double)rXmin )	x = (double)rXmin;
	if( y > (double)rYmax )	y = (double)rYmax;
	if( y < (double)rYmin )	y = (double)rYmin;
	
	if( x >= ((double)rXmax-0.5*(double)rDelta) )
	{
		x = (double)rXmax;
		n = nX*((int)(((double)y - (double)rYmin)/(double)rDelta+0.5))+nX-1;
		return n;
	};
	if( x < ((double)rXmin+0.5*(double)rDelta) )
	{
		x = (double)rXmin;
		n = nX*((int)(((double)y - (double)rYmin)/(double)rDelta+0.5));
		return n;
	};

	if( y > (double)rYmax )	y = (double)rYmax;
	if( y < (double)rYmin )	y = (double)rYmin;


	n = (int)(((double)x - (double)rXmin)/(double)rDelta+0.5) + nX*((int)(((double)y - (double)rYmin)/(double)rDelta+0.5));

//	if( n < 0 ) n = 0;
//	if( n >= nX*nY ) n = nX*nY-1;

	return n;
}

int CTmcRTH_IndanParam::CalcnNodeGlobalFM( double x, double y )
{
	int n, nX, nY;

	n = 0;
	nX = CalcnX();
	nY = CalcnY();

	if( x > (double)rXmax )	x = (double)rXmax;
	if( x < (double)rXmin )	x = (double)rXmin;
	if( y > (double)rYmax )	y = (double)rYmax;
	if( y < (double)rYmin )	y = (double)rYmin;
	
	if( x >= ((double)rXmax-1.5*(double)rDelta) )
	{
		x = (double)rXmax;
		n = nX*((int)(((double)y - (double)rYmin)/(double)rDelta+0.5))+nX-2;
		return n;
	};
	if( x < ((double)rXmin+1.5*(double)rDelta) )
	{
		x = (double)rXmin;
		n = nX*((int)(((double)y - (double)rYmin)/(double)rDelta+0.5))+1;
		return n;
	};

	if( y > (double)rYmax )	y = (double)rYmax;
	if( y < (double)rYmin )	y = (double)rYmin;


	n = (int)(((double)x - (double)rXmin)/(double)rDelta+0.5) + nX*((int)(((double)y - (double)rYmin)/(double)rDelta+0.5));

//	if( n < 0 ) n = 0;
//	if( n >= nX*nY ) n = nX*nY-1;

	return n;
}

BOOL CTmcRTH_IndanParam::IsnInRegion( int n )
{
	int nX, nY;
	nX = (int)(( rXmax - rXmin )/rDelta)+1;
	nY = (int)(( rYmax - rYmin )/rDelta)+1;

	if( (n < 0)||( n >= nX*nY ) ) return FALSE;

	return TRUE;
}

BOOL CTmcRTH_IndanParam::IsnInRegion( _real x, _real y )
{
	return IsnInRegion( CalcnNodeGlobal( x, y ) );
}


int CTmcRTH_IndanParam::CalcnX()
{
	int nX;
	if( fabs(rDelta) < FLT_MIN ) return 0;
	nX = (int)(( rXmax - rXmin )/rDelta)+1;
	return nX;
}

int CTmcRTH_IndanParam::CalcnY()
{
	int nY;
	if( fabs(rDelta) < FLT_MIN ) return 0;
	nY = (int)(( rYmax - rYmin )/rDelta)+1;
	return nY;
}
