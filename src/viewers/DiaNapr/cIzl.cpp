// cIzl.cpp: implementation of the cIzl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <typedef.h>
#include <expr.h>
#include "cIzl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cIzl::cIzl()
{
	dAmplituda =  0.0;
	dFaza = 0.0;
	dXcoordinata = 0.0;
	dYcoordinata = 0.0;
	csEmitterDirectPat.Format("");
	nTypeEmitterDirectPat = TMC_IZL_DIRPATTYPE_ALL;
	dFimax = 0.0;
	dFimin = 2*3.141592653589;
	return;
}

cIzl::~cIzl()
{
	return;
}

void cIzl::SetdAmplituda(double dR)
{
	dAmplituda = dR;
	return;
}

void cIzl::SetdFaza(double dR)
{
	dFaza = dR;
	return;
}

void cIzl::SetdXcoordinata(double dR)
{
	dXcoordinata = dR;
	return;
}

void cIzl::SetdYcoordinata(double dR)
{
	dYcoordinata = dR;
	return;
}

double cIzl::GetdAmplituda()
{
	return dAmplituda;
}

double cIzl::GetdFaza()
{
	return dFaza;
}

double cIzl::GetdXcoordinata()
{
	return dXcoordinata;
}

double cIzl::GetdYcoordinata()
{
	return dYcoordinata;
}

cIzl& cIzl::operator=(  cIzl& cIzl1 )
{
	dAmplituda = cIzl1.GetdAmplituda();
	dFaza = cIzl1.GetdFaza();
	dXcoordinata = cIzl1.GetdXcoordinata();
	dYcoordinata = cIzl1.GetdYcoordinata();
	csEmitterDirectPat = cIzl1.GetcsEmitterDirectPat();
	dFimax = cIzl1.GetdFimax();
	dFimin = cIzl1.GetdFimin();
	nTypeEmitterDirectPat = cIzl1.nTypeEmitterDirectPat;

	return (*this);
}

CString& cIzl::GetcsEmitterDirectPat()
{
	return csEmitterDirectPat;
}

void cIzl::SetcsEmitterDirectPat(CString &csBuf)
{
	csEmitterDirectPat = csBuf;
	if( (strcmp( csEmitterDirectPat, "1." ) == 0)||(strlen(csEmitterDirectPat) < 1) )
	{
		nTypeEmitterDirectPat = TMC_IZL_DIRPATTYPE_ALL;
		return;
	};
	if( strcmp( csEmitterDirectPat, "sin(f)" ) == 0 )
	{
		nTypeEmitterDirectPat = TMC_IZL_DIRPATTYPE_SIN;
		return;
	};
	if( strcmp( csEmitterDirectPat, "cos(f)" ) == 0 )
	{
		nTypeEmitterDirectPat = TMC_IZL_DIRPATTYPE_COS;
		return;
	};
	nTypeEmitterDirectPat = TMC_IZL_DIRPATTYPE_ARB;
	return;
}

void cIzl::SetcsEmitterDirectPat(char *pszBuf)
{
	CString csBuf;
	csBuf.Format("%s", pszBuf);
	SetcsEmitterDirectPat( csBuf );
	return;
}

void cIzl::SetdFimin(double dBuf)
{
	dFimin = dBuf;
}

double cIzl::GetdFimin()
{
	return dFimin;
}

void cIzl::SetdFimax(double dBuf)
{
	dFimax = dBuf;
}

double cIzl::GetdFimax()
{
	return dFimax;
}

BOOL cIzl::bIsAngleInDiapazon(double dAngle)
{
	if( strlen(csEmitterDirectPat) < 1 )	return TRUE;

	dAnglePrived( dFimin );
	dAnglePrived( dFimax );
	dAnglePrived( dAngle );
	
//	if( (dAngle >= dFimin)&&(dAngle <= dFimax) )
//	{
//		CString csBuf1;
//		csBuf1.Format("bIsAngleInDiapazon = TRUE; dFimin = %lg; dFimax = %lg; dAngle = %lg", dFimin, dFimax, dAngle);
//		AfxMessageBox(csBuf1);
//	}
	
	if( (dAngle >= dFimin)&&(dAngle <= dFimax) ) return TRUE;
	
	dAnglePrived1( dFimin );
	dAnglePrived1( dFimax );
	dAnglePrived1( dAngle );

//	if( (dAngle >= dFimin)&&(dAngle <= dFimax) )
//	{
//		CString csBuf1;
//		csBuf1.Format("bIsAngleInDiapazon = TRUE; dFimin = %lg; dFimax = %lg; dAngle = %lg", dFimin, dFimax, dAngle);
//		AfxMessageBox(csBuf1);
//	}
	
	if( (dAngle >= dFimin)&&(dAngle <= dFimax) ) return TRUE;

	return FALSE;
}

void cIzl::dAnglePrived(double &dAngle)
{
	double x, y, f;

	x = cos(dAngle);
	y = sin(dAngle);
	f = atan2( y, x);
	dAngle = f;

	return;
}

void cIzl::dAnglePrived1(double &dAngle)
{
	double x, y, f;

	x = cos(dAngle);
	y = sin(dAngle);
	f = atan2( y, x);
	if( f <= 0.0 ) f += 2*3.141592653589;
	dAngle = f;

	return;
}

double cIzl::dGetDirectionalPatternEmitter(double dAngle)
{
	if( strlen(csEmitterDirectPat) < 1 ) return 1.0;
	if( !bIsAngleInDiapazon(dAngle) ) return FLT_MIN;

	double r1;
	char szBuf1[1000];
	CString csBuf1;

	switch( nTypeEmitterDirectPat )
	{
		case TMC_IZL_DIRPATTYPE_ALL:
			return 1.;
			break;
		case TMC_IZL_DIRPATTYPE_SIN:
			return sin(dAngle);
			break;
		case TMC_IZL_DIRPATTYPE_COS:
			return cos(dAngle);
			break;
	};

	sprintf( szBuf1, "f=(%lg);", dAngle);
	if( i1nte_atof_1( szBuf1, &r1 ) != 0 )
	{
//		bIsError = TRUE;
//		sprintf( szError, "Error when calculate expression for one Directional Pattern {%s}", expr_get_error());
		return 0.0;
	};

//	csBuf1.Format("%s", szBuf1);
//	AfxMessageBox(csBuf1);

	sprintf( szBuf1, "(%s);", csEmitterDirectPat);
	if( i1nte_atof_1( szBuf1, &r1 ) != 0 )
	{
//		bIsError = TRUE;
//		sprintf( szError, "Error when calculate expression for one Directional Pattern {%s}", expr_get_error());
		return 0.0;
	};

//	csBuf1.Format("{%s}=%lg", szBuf1, r1);
//	AfxMessageBox(csBuf1);

	return r1;
}

double cIzl::GetdAmplituda(double dAngle)
{
	if( fabs(dAmplituda) < FLT_MIN ) return 0.0;
	return dGetDirectionalPatternEmitter(dAngle)*dAmplituda;
}
