// TmcRTH_IndanParam.cpp : implementation file
//

#include "stdafx.h"
#include <limits.h>
#include <float.h>
#include <math.h>
#include <typerth.h>
#include <TmcRTH_Indan.h>
#include <TmcRTH_IndanParam.h>
#include <expr.h>


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
	csBUnit.Format(CTMCRTH_BUNIT_Tl);
	rBUnit = (_real)(CTMCRTH_BUNIT_UTl);
	csNUnit.Format(CTMCRTH_NUNIT_m3);
	rNUnit = (_real)(CTMCRTH_NUNIT_Um3);
	rDelta = (_real)(0.0);
	rFreq = (_real)(0.0);
	rt = (_real)(0.0);
	rXmin = (_real)(0.0);
	rXmax = (_real)(0.0);
	rYmin = (_real)(0.0);
	rYmax = (_real)(0.0);
	rTmin = (_real)(0.0);
	rTmax = (_real)(0.0);
	rHeight = (_real)(0.0);
	rDeltaTMechanical = FLT_MAX;
	nXGrid = 0;	// B-2
	nYGrid = 0;
	rXmaxUser = 0.0;
	rYmaxUser = 0.0;
	nSeenInSection = 0;	// A-13, B-15
	return;
}

CTmcRTH_IndanParam::~CTmcRTH_IndanParam()
{
	DeleteData();
	return;
}


//BEGIN_MESSAGE_MAP(CTmcRTH_IndanParam, CWnd)
	//{{AFX_MSG_MAP(CTmcRTH_IndanParam)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()


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
	csBUnit.Format(CTMCRTH_BUNIT_Tl);
	rBUnit = (_real)(CTMCRTH_BUNIT_UTl);
	csNUnit.Format(CTMCRTH_NUNIT_m3);
	rNUnit = (_real)(CTMCRTH_NUNIT_Um3);
	rDelta = (_real)(0.0);
	rFreq = (_real)(0.0);
	rt = (_real)(0.0);
	rXmin = (_real)(0.0);
	rXmax = (_real)(0.0);
	rYmin = (_real)(0.0);
	rYmax = (_real)(0.0);
	rTmin = (_real)(0.0);
	rTmax = (_real)(0.0);
	rHeight = (_real)(0.0);
	rDeltaTMechanical = FLT_MAX;
	nXGrid = 0;	// B-2
	nYGrid = 0;
	rXmaxUser = 0.0;
	rYmaxUser = 0.0;
	nSeenInSection = 0;	// A-13, B-15
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
	if( strncmp( csCh, CTMCRTH_HEIGHT_ID, strlen(CTMCRTH_HEIGHT_ID) ) == 0 )
	{
		ReadHeight(csCh);
		cError1 = cError;
		return;
	};

	if( strncmp( csCh, CTMCRTH_DELTA_T_ID, strlen(CTMCRTH_DELTA_T_ID) ) == 0 )
	{
		ReadDeltaTMechanical(csCh);
		cError1 = cError;
		return;
	};

	if( strncmp( csCh, CTMCRTH_BUNIT_ID, strlen(CTMCRTH_BUNIT_ID) ) == 0 )
	{
		ReadBUnit(csCh);
		cError1 = cError;
		return;
	};

	if( strncmp( csCh, CTMCRTH_NUNIT_ID, strlen(CTMCRTH_NUNIT_ID) ) == 0 )
	{
		ReadNUnit(csCh);
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
	i = (int)(strlen(CTMCRTH_XMAX_ID));

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
	nSeenInSection |= TMC_PARAM_SEEN_XMAX;	// A-13, B-15
	rXmaxUser = rXmax;	// B-2: the grid is calculated again
	nXGrid = 0;
	
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
	i = (int)(strlen(CTMCRTH_XMIN_ID));

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
	nSeenInSection |= TMC_PARAM_SEEN_XMIN;	// A-13, B-15
	nXGrid = 0;	// B-2: the grid is calculated again
	
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
	i = (int)(strlen(CTMCRTH_YMAX_ID));

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
	nSeenInSection |= TMC_PARAM_SEEN_YMAX;	// A-13, B-15
	rYmaxUser = rYmax;	// B-2: the grid is calculated again
	nYGrid = 0;
	
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
	i = (int)(strlen(CTMCRTH_YMIN_ID));

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
	nSeenInSection |= TMC_PARAM_SEEN_YMIN;	// A-13, B-15
	nYGrid = 0;	// B-2: the grid is calculated again
	
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
	i = (int)(strlen(CTMCRTH_DELTA_ID));

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
	nSeenInSection |= TMC_PARAM_SEEN_DELTA;	// A-13, B-15
	nXGrid = 0;	// B-2: the grid is calculated again
	nYGrid = 0;
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

	i = (int)(strlen(CTMCRTH_TIME_ID));

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
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf1, expr_get_error() );	// N-19
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
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf1, expr_get_error() );	// N-19
		cError.PutErrorMessage(csBuf);
		return;
	};
	rTmax = (_real)(r * rTimeUnit);
	nSeenInSection |= TMC_PARAM_SEEN_TIME;	// A-13, B-15
	
	return;
}

void CTmcRTH_IndanParam::ReadFreqUnit( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	int i;
	CString csBuf;

	if( !CheckUnitBeforeValues( csCh, TMC_PARAM_SEEN_FREQ, CTMCRTH_FREQ_ID ) ) return;	// A-13
	strcpy( szBuf, csCh );
	i = (int)(strlen(CTMCRTH_FREQUNIT_ID));

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

	if( !CheckUnitBeforeValues( csCh, TMC_PARAM_SEEN_XMIN|TMC_PARAM_SEEN_XMAX|TMC_PARAM_SEEN_YMIN|TMC_PARAM_SEEN_YMAX|
		TMC_PARAM_SEEN_DELTA|TMC_PARAM_SEEN_HEIGHT, "X_MIN, X_MAX, Y_MIN, Y_MAX, DELTA, HEIGHT" ) ) return;	// A-13
	strcpy( szBuf, csCh );
	i = (int)(strlen(CTMCRTH_LONGUNIT_ID));

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

	if( !CheckUnitBeforeValues( csCh, TMC_PARAM_SEEN_TIME|TMC_PARAM_SEEN_DELTAT, "TIME, DELTA_T" ) ) return;	// A-13
	strcpy( szBuf, csCh );
	i = (int)(strlen(CTMCRTH_TIMEUNIT_ID));

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
	i = (int)(strlen(CTMCRTH_ANGLEUNIT_ID));

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
	
	// B-15 (author 29.10): every step is an independent task - all of X_MIN, X_MAX, Y_MIN, Y_MAX, DELTA, TIME, FREQ
	// must be in its #PARM section (old: the missing ones were taken from the previous step).
	/// Ѕ-15: в секции `#PARM` каждого шага об€зательны X_MIN, X_MAX, Y_MIN, Y_MAX, DELTA, TIME, FREQ.
	if( ( nSeenInSection & TMC_PARAM_SEEN_REQUIRED ) != TMC_PARAM_SEEN_REQUIRED )
	{
		CString csMiss;
		static const struct { unsigned n; const char *psz; } aReq[] = {
			{ TMC_PARAM_SEEN_XMIN, "X_MIN" }, { TMC_PARAM_SEEN_XMAX, "X_MAX" }, { TMC_PARAM_SEEN_YMIN, "Y_MIN" },
			{ TMC_PARAM_SEEN_YMAX, "Y_MAX" }, { TMC_PARAM_SEEN_DELTA, "DELTA" }, { TMC_PARAM_SEEN_TIME, "TIME" },
			{ TMC_PARAM_SEEN_FREQ, "FREQ" } };
		for( unsigned k = 0; k < sizeof(aReq)/sizeof(aReq[0]); k++ )
			if( !( nSeenInSection & aReq[k].n ) ) { if( csMiss.GetLength() > 0 ) csMiss += ", "; csMiss += aReq[k].psz; };
		csBuf.Format("Section %s: missing %s. Every step is an independent task: write X_MIN, X_MAX, Y_MIN, Y_MAX, "
			"DELTA, TIME, FREQ in the section %s of every step (they are not taken from the previous step)",
			CTMCRTH_INDANMET_PARAM, (LPCSTR)csMiss, CTMCRTH_INDANMET_PARAM);
		cError.PutErrorMessage(csBuf);
		cError1 = cError;
		return;
	};

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
			CTMCRTH_INDANMET_PARAM, rDelta, (C0___/rFreq),	// N-19
			CTMCRTH_DELTA_ID, (C0___/(5.*rFreq))/rLongUnit, csLongUnit);
		cError.PutErrorMessage(csBuf);
		cError1 = cError;
		return;
	};

//////////////////////////////////////////////////
//////////////////////////////////////////////////
	// B-2: nX and nY are calculated ONCE, from X_MAX and Y_MAX as written in the task, and kept (nXGrid, nYGrid);
	// a new border or DELTA calculates them again. The border is moved to the reserve column as before.
	// Old (the second division lost the column of X_MAX on some grids; a step without X_MAX moved the border
	// again - the grid grew on every step); to return it, put these lines back instead of the block below:
	//   int nX111, nY111;
	//   nX111 = CalcnX();
	//   nY111 = CalcnY();
	//   rXmax = rXmin + nX111*rDelta;
	//   rYmax = rYmin + nY111*rDelta;
	// and in CalcnX/CalcnY delete the lines "if( nXGrid > 0 ) return nXGrid;" (B-2).
	if( nXGrid <= 0 )
	{
		int nX111 = (int)( ( rXmaxUser - rXmin )/rDelta + 1e-7 ) + 1;
		/// Ѕ-2.2 (автор 29.10): запасной столбец задуман Ч сетка на один узел больше заданной области
		/// (последний узел на шаг дальше X_MAX, то же по Y).
		rXmax = rXmin + nX111*rDelta;
		nXGrid = nX111 + 1;
	};
	if( nYGrid <= 0 )
	{
		int nY111 = (int)( ( rYmaxUser - rYmin )/rDelta + 1e-7 ) + 1;
		rYmax = rYmin + nY111*rDelta;
		nYGrid = nY111 + 1;
	};
//////////////////////////////////////////////////
//////////////////////////////////////////////////

	
	return;
}

// A-13 (author 29.10): a unit after its value is an error (old: the value was silently read in the previous unit)
BOOL CTmcRTH_IndanParam::CheckUnitBeforeValues( CString &csCh, unsigned nMask, const char *pszIds )
{
	if( ( nSeenInSection & nMask ) == 0 ) return TRUE;
	CString csBuf;
	csBuf.Format("{%s}: the unit is written after its value (%s) in the section %s. A unit acts only on the lines "
		"below it - move the line of the unit above the values", (LPCSTR)csCh, pszIds, CTMCRTH_INDANMET_PARAM);
	cError.PutErrorMessage(csBuf);
	return FALSE;
}

void CTmcRTH_IndanParam::ReadFreq( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	double r;
	int i;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = (int)(strlen(CTMCRTH_FREQ_ID));

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
	nSeenInSection |= TMC_PARAM_SEEN_FREQ;	// A-13, B-15
	
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
	cError = cParam.cError;
	csFreqUnit  = cParam.csFreqUnit;
	rFreqUnit   = cParam.rFreqUnit;
	csLongUnit  = cParam.csLongUnit;
	rLongUnit   = cParam.rLongUnit;
	csTimeUnit  = cParam.csTimeUnit;
	rTimeUnit   = cParam.rTimeUnit;
	csAngleUnit = cParam.csAngleUnit;
	rAngleUnit  = cParam.rAngleUnit;
	csBUnit     = cParam.csBUnit;
	rBUnit      = cParam.rBUnit;
	csNUnit     = cParam.csNUnit;
	rNUnit      = cParam.rNUnit;
	rDelta = cParam.rDelta;
	rFreq  = cParam.rFreq;
	rt     = cParam.rt;
	rXmin  = cParam.rXmin;
	rXmax  = cParam.rXmax;
	rYmin  = cParam.rYmin;
	rYmax  = cParam.rYmax;
	rTmin  = cParam.rTmin;
	rTmax  = cParam.rTmax;
	rHeight = cParam.rHeight;
	rDeltaTMechanical = cParam.rDeltaTMechanical;
	nXGrid = cParam.nXGrid;	// B-2
	nYGrid = cParam.nYGrid;
	rXmaxUser = cParam.rXmaxUser;
	rYmaxUser = cParam.rYmaxUser;

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


double CTmcRTH_IndanParam::CalcX( TmcIdx nNumber )	// INT64
{
	double r;
	int nY, nX;

	nX = CalcnX();	// B-2 (old: (int)(( rXmax - rXmin )/rDelta)+1)
	nY = (int)( nNumber - ((TmcIdx)nX)*(nNumber/nX) );	// INT64
	r = rXmin + nY*rDelta;

	return r;
}

double CTmcRTH_IndanParam::CalcY( TmcIdx nNumber )	// INT64
{
	double r;
	int nY, nX;

	nX = CalcnX();	// B-2 (old: (int)(( rXmax - rXmin )/rDelta)+1)
	nY = (int)(nNumber/nX);	// INT64
	r = rYmin + nY*rDelta;

	return r;
}

TmcIdx CTmcRTH_IndanParam::CalcnNodeGlobal( double x, double y )	// INT64
{
	TmcIdx n;	// INT64
	int nX, nY;

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
		n = ((TmcIdx)nX)*((int)(((double)y - (double)rYmin)/(double)rDelta+0.5))+nX-1;
		return n;
	};
	if( x < ((double)rXmin+0.5*(double)rDelta) )
	{
		x = (double)rXmin;
		n = ((TmcIdx)nX)*((int)(((double)y - (double)rYmin)/(double)rDelta+0.5));
		return n;
	};

	if( y > (double)rYmax )	y = (double)rYmax;
	if( y < (double)rYmin )	y = (double)rYmin;


	n = (int)(((double)x - (double)rXmin)/(double)rDelta+0.5) + ((TmcIdx)nX)*((int)(((double)y - (double)rYmin)/(double)rDelta+0.5));


	return n;
}

TmcIdx CTmcRTH_IndanParam::CalcnNodeGlobalFM( double x, double y )	// INT64
{
	TmcIdx n;	// INT64
	int nX, nY;

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
		n = ((TmcIdx)nX)*((int)(((double)y - (double)rYmin)/(double)rDelta+0.5))+nX-2;
		return n;
	};
	if( x < ((double)rXmin+1.5*(double)rDelta) )
	{
		x = (double)rXmin;
		n = ((TmcIdx)nX)*((int)(((double)y - (double)rYmin)/(double)rDelta+0.5))+1;
		return n;
	};

	if( y > (double)rYmax )	y = (double)rYmax;
	if( y < (double)rYmin )	y = (double)rYmin;


	n = (int)(((double)x - (double)rXmin)/(double)rDelta+0.5) + ((TmcIdx)nX)*((int)(((double)y - (double)rYmin)/(double)rDelta+0.5));


	return n;
}

BOOL CTmcRTH_IndanParam::IsnInRegion( TmcIdx n )	// INT64
{
	int nX, nY;
	nX = CalcnX();	// B-2 (old: (int)(( rXmax - rXmin )/rDelta)+1, the same for nY)
	nY = CalcnY();

	if( (n < 0)||( n >= ((TmcIdx)nX)*nY ) ) return FALSE;	// INT64

	return TRUE;
}

BOOL CTmcRTH_IndanParam::IsnInRegion( _real x, _real y )
{
	// B-16: a point more than half a step beyond the border is outside (old: CalcnNodeGlobal pressed it to the
	// border first, so the check was always TRUE)
	if( ( x < rXmin - 0.5*rDelta )||( x > rXmax + 0.5*rDelta )||( y < rYmin - 0.5*rDelta )||( y > rYmax + 0.5*rDelta ) )
		return FALSE;
	return IsnInRegion( CalcnNodeGlobal( x, y ) );
}


int CTmcRTH_IndanParam::CalcnX()
{
	int nX;
	if( nXGrid > 0 ) return nXGrid;	// B-2: calculated once
	if( fabs(rDelta) < FLT_MIN ) return 0;
	nX = (int)(( rXmax - rXmin )/rDelta)+1;
	return nX;
}

int CTmcRTH_IndanParam::CalcnY()
{
	int nY;
	if( nYGrid > 0 ) return nYGrid;	// B-2: calculated once
	if( fabs(rDelta) < FLT_MIN ) return 0;
	nY = (int)(( rYmax - rYmin )/rDelta)+1;
	return nY;
}


void CTmcRTH_IndanParam::ReadHeight( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	double r;
	int i;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = (int)(strlen(CTMCRTH_HEIGHT_ID));

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
	rHeight = (_real)(r * rLongUnit);
	nSeenInSection |= TMC_PARAM_SEEN_HEIGHT;	// A-13
	
	return;
}

void CTmcRTH_IndanParam::ReadDeltaTMechanical( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	double r;
	int i;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = (int)(strlen(CTMCRTH_DELTA_T_ID));

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
	rDeltaTMechanical = (_real)(r * rTimeUnit);
	nSeenInSection |= TMC_PARAM_SEEN_DELTAT;	// A-13
	SetDeltaT( rDeltaTMechanical );
	
	return;
}



// CalcnNodeGlobal return NodeGlobal or -1, if point x,y isn't in region

void CTmcRTH_IndanParam::ReadBUnit( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	int i;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = (int)(strlen(CTMCRTH_BUNIT_ID));

	i+= Search_1Param( csBuf, szBuf+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );

	if( strcmp( szBuf, CTMCRTH_BUNIT_Tl ) == 0 )
	{
		csBUnit.Format( "%s", szBuf);
		rBUnit = (_real)(CTMCRTH_BUNIT_UTl);
		return;
	};
	// J-6: the multipliers of B_UNIT are not applied in the calculation (B is always in Tl) - not Tl is an error
	// (old: mTl and mkTl were accepted and the values were silently taken as Tl)
	if( ( strcmp( szBuf, CTMCRTH_BUNIT_mTl ) == 0 )||( strcmp( szBuf, CTMCRTH_BUNIT_mkTl ) == 0 ) )
	{
		csBuf.Format("{%s}: the kernel calculates B only in Tl (B_UNIT %s;) - write B in Tl and B_UNIT %s;",
			(LPCSTR)csCh, CTMCRTH_BUNIT_Tl, CTMCRTH_BUNIT_Tl);
		cError.PutErrorMessage(csBuf);
		return;
	};

	csBuf.Format("{%s}use following %s: %s, %s, %s", csCh, CTMCRTH_BUNIT_ID,
		CTMCRTH_BUNIT_Tl, CTMCRTH_BUNIT_mTl, CTMCRTH_BUNIT_mkTl);
	cError.PutErrorMessage(csBuf);
	return;
}

void CTmcRTH_IndanParam::ReadNUnit( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	int i;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = (int)(strlen(CTMCRTH_NUNIT_ID));

	i+= Search_1Param( csBuf, szBuf+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );

	// Bug #16: the fields of N (old: csBUnit/rBUnit - a copy of ReadBUnit)
	if( strcmp( szBuf, CTMCRTH_NUNIT_m3 ) == 0 )
	{
		csNUnit.Format( "%s", szBuf);
		rNUnit = (_real)(CTMCRTH_NUNIT_Um3);
		return;
	};
	// J-6: the multipliers of N_UNIT are not applied in the calculation (N is always in m^-3) - not m3 is an error
	// (old: cm3 and mm3 were accepted and the values were silently taken as m^-3)
	if( ( strcmp( szBuf, CTMCRTH_NUNIT_cm3 ) == 0 )||( strcmp( szBuf, CTMCRTH_NUNIT_mm3 ) == 0 ) )
	{
		csBuf.Format("{%s}: the kernel calculates N only in m^-3 (N_UNIT %s;) - write N in m^-3 and N_UNIT %s;",
			(LPCSTR)csCh, CTMCRTH_NUNIT_m3, CTMCRTH_NUNIT_m3);
		cError.PutErrorMessage(csBuf);
		return;
	};

	csBuf.Format("{%s}use following %s: %s, %s, %s", csCh, CTMCRTH_NUNIT_ID,
		CTMCRTH_NUNIT_m3, CTMCRTH_NUNIT_cm3, CTMCRTH_NUNIT_mm3);
	cError.PutErrorMessage(csBuf);
	return;
}



static double dDeltaT = 100000000;
static BOOL bDeltaTFlag = FALSE;

void SetDeltaT( double dDeltaT1 )
{
	dDeltaT = dDeltaT1;
	bDeltaTFlag = TRUE;
	return;
}




void SetDeltaT( char *lpszDeltaT )
{
	double dDeltaT1;
	if( sscanf( lpszDeltaT, "%lg", &dDeltaT1 ) != 1 )
	{
//		CString csBuf;
		return;
	}
	else
	{
		dDeltaT = dDeltaT1;
		bDeltaTFlag = TRUE;
//		CString csBuf;
	};
	return;
}

