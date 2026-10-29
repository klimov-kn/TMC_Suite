// TMCGrExpression.cpp : implementation file
//

#include "stdafx.h"
#include "TMCGROUT.h"
#include "TMCGrExpression.h"
#include <complex1.h>
#include <inter.h>
#include <tmcgrviw.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTMCGrExpression

CTMCGrExpression::CTMCGrExpression( char *szExpression )
{
	nVar = 0;
	nMaska = 0;
	pnPoint    = NULL;
	pszMaska   = NULL;
	pszVar     = NULL;
	ppcSmatrix = NULL;
	ppdFreq    = NULL;
	csExpresion.Format( "%s", szExpression );
	IsError = FALSE;
	return;
}

CTMCGrExpression::~CTMCGrExpression()
{
	DeleteData();
}




/////////////////////////////////////////////////////////////////////////////
// CTMCGrExpression message handlers

char * CTMCGrExpression::InitData( void )
{
	int i;
	CalcNGraphExpession();
	if( nVar < 1 ) return "Bad_Nvar_or_Bad_Bracket[]";
	nMaska = nVar + 1;
	if( (pszVar = new char*[nVar]) == NULL )
	{
		DeleteData();
		return "memory_allocation_error";
	};
	for( i = 0; i < nVar; i++ ) pszVar[i] = NULL;

	if( (pnPoint = new int[nVar]) == NULL )
	{
		DeleteData();
		return "memory_allocation_error";
	};
	for( i = 0; i < nVar; i++ ) pnPoint[i] = 0;

	if( (ppdFreq = new double*[nVar]) == NULL )
	{
		DeleteData();
		return "memory_allocation_error";
	};
	for( i = 0; i < nVar; i++ ) ppdFreq[i] = NULL;

	if( (ppcSmatrix = new _complex*[nVar]) == NULL )
	{
		DeleteData();
		return "memory_allocation_error";
	};
	for( i = 0; i < nVar; i++ ) ppcSmatrix[i] = NULL;

	if( (pszMaska = new char*[nMaska]) == NULL )
	{
		DeleteData();
		return "memory_allocation_error";
	};
	for( i = 0; i < nMaska; i++ ) pszMaska[i] = NULL;

	if( !Init1Data() )
	{
		DeleteData();
		return "init_data_error";
	};

	return NULL;
}

int CTMCGrExpression::CalcNGraphExpession( void )
{
	int i, n, i1, n1;
	n1 = csExpresion.GetLength();
	for( i = 0, n = 0, i1 = 0;  i < n1; i++ )
	{
		switch( csExpresion[i] )
		{
			case '[':
				if( i1 == 0 ) n++;
				i1++;
				break;
			case ']':
				if( i1 == 0 ) return 0;
				i1--;
				break;
			default:
				break;
		};
	};
	
	if( i1 != 0 ) 
	{
		n = 0;
	};
	return nVar = n;
}

BOOL CTMCGrExpression::Init1Data( void )
{
	int i, ii, n, i1, i2, iMask, iVar;

	n = csExpresion.GetLength();
	
	for( i = 0, ii = 0, i1 = 0, i2 = 0, iMask = 0, iVar = 0; i < n; i++)
	{
		switch( csExpresion[i] )
		{
			case '[':
				if( ii == 0 )
				{
					i2 = i;
					if( (pszMaska[ iMask ] = new char[ i2 - i1 + 1 ]) == NULL )
					{
						return FALSE;		 
					};
					strncpy( pszMaska[ iMask ], LPCTSTR(csExpresion) + i1, i2 - i1 + 1);
					pszMaska[ iMask ][ i2 - i1 ] = '\0';
					i1 = i + 1;
					iMask++;
				};
				ii++;
				break;
			case ']':
				if( ii == 1 )
				{
					i2 = i;
					if( (pszVar[ iVar ] = new char[ i2 - i1 + 1 ]) == NULL )
					{
						return FALSE;
					};
					strncpy( pszVar[ iVar ], LPCTSTR(csExpresion) + i1, i2 - i1 + 1);
					pszVar[ iVar ][ i2 - i1 ] = '\0';
					i1 = i + 1;
					iVar++;
				};
				ii--;
				break;
			default:
				break;
		};
	};
	
	if( nVar != 0 )
	{
		i2 = i;
		if( (pszMaska[ iMask ] = new char[ i2 - i1 + 1 ]) == NULL )
		{
			return FALSE;		 
		};
		strncpy( pszMaska[ iMask ], LPCTSTR(csExpresion) + i1, i2 - i1 + 1);
		pszMaska[ iMask ][ i2 - i1 ] = '\0';
		i1 = i + 1;
		iMask++;
	};

	return TRUE;
}

void CTMCGrExpression::DeleteData( void )
{
	int i;
	
	if( pszMaska != NULL ) 
	{
		for( i = 0; i < nMaska; i++)
		{
			if( pszMaska[i] != NULL )
			{
				delete pszMaska[i];
			};
		};
		delete pszMaska;
		pszMaska = NULL;
	};
	
	if( pszVar != NULL ) 
	{
		for( i = 0; i < nVar; i++ )
		{
			if( pszVar[i] != NULL )
			{
				delete pszVar[i];
			};
		};
		delete pszVar;
		pszVar = NULL;
	};
	
	if( ppdFreq != NULL ) 
	{
		for( i = 0; i < nVar; i++ )
		{
			if( ppdFreq[i] != NULL )
			{
				delete ppdFreq[i];
			};
		};
		delete ppdFreq;
		ppdFreq = NULL;
	};
	
	if( ppcSmatrix != NULL ) 
	{
		for( i = 0; i < nVar; i++ )
		{
			if( ppcSmatrix[i] != NULL )
			{
				delete ppcSmatrix[i];
			};
		};
		delete ppcSmatrix;
		ppcSmatrix = NULL;
	};		   	
	
	if( pnPoint != NULL ) 
	{
		delete pnPoint;
		pnPoint = NULL;
	};

	nVar   = 0;
	nMaska = 0;
	IsError = FALSE;

	return;
}

int CTMCGrExpression::GetNVar( void )
{
	return nVar;
}

char * CTMCGrExpression::GetFileName( int nFileName )
{
	if( (pszVar == NULL)||( nFileName >= nVar )||( nFileName < 0 ) ) return NULL;
	return pszVar[ nFileName ];
}

double ** CTMCGrExpression::GetFreqArray( int nFileName )
{
	if( (ppdFreq == NULL)||( nFileName >= nVar )||( nFileName < 0 ) ) return NULL;
	return ppdFreq + nFileName;
}

_complex ** CTMCGrExpression::GetSMatrArray( int nFileName )
{
	if( (ppcSmatrix == NULL)||( nFileName >= nVar )||( nFileName < 0 ) ) return NULL;
	return ppcSmatrix + nFileName;
}

int * CTMCGrExpression::GetNPoint( int nFileName )
{
	if( (pnPoint == NULL)||( nFileName >= nVar )||( nFileName < 0 ) ) return NULL;
	return pnPoint + nFileName;
}

BOOL CTMCGrExpression::IsFreqCorrect( void )
{
	int i, j, n;
	
	n = pnPoint[0];
	
	for( i = 1; i < nVar; i++ )
	{
		if( pnPoint[i] != n ) return FALSE;
		for( j = 0; j < n; j++ )
			if( fabs( (ppdFreq[i][j] - ppdFreq[0][j])/ppdFreq[0][j] ) > (1.e-15) ) return FALSE;
	};
	
	return TRUE;
}

double CTMCGrExpression::GetExpressionValue( int nPoint )
{
	char ch[TMC_GROUT_MAXSTRING_BUF];
	double d = 0.0;	// S-48

	if( (IsError)||(pnPoint == NULL)||(ppcSmatrix == NULL)||(ppdFreq == NULL) ) return 0;
	if( (pnPoint[0] <= nPoint)||( nPoint < 0 ) ) return 0;	// I-14 (old: pnPoint[0] < nPoint - one past the array)

	snprintf( ch, TMC_GROUT_MAXSTRING_BUF, "{f=(%.17g)}", ppdFreq[0][nPoint] );	// S-48, A-28: bylo %lf (1e-12 -> 0)
	if( i1nte_atof_1( ch, &d ) != 0 ) d = 0.0;	// S-48: oshibka - tolko v etoi tochke (ne zalipaet)
	return d;
}

double CTMCGrExpression::GetExpressionValueSFD( int nPoint )
{
	int i, n;
	char ch[TMC_GROUT_MAXSTRING_BUF];
	double d = 0.0;	// S-48

	if( (IsError)||(pnPoint == NULL)||(ppcSmatrix == NULL)||(ppdFreq == NULL) ) return 0;
	if( (pnPoint[0] <= nPoint)||( nPoint < 0 ) ) return 0;	// I-14 (old: pnPoint[0] < nPoint - one past the array)
	GetExpressionValue( nPoint );
	
	strcpy( ch, pszMaska[0] );
	n = strlen( ch );
	for( i = 0; i < nVar; i++, n = strlen( ch ) )
	{
		d = 180./(3.141592653589)*GetCPhase( i, nPoint);
		snprintf( ch+n, TMC_GROUT_MAXSTRING_BUF-n, "(%.17g)%s", d, pszMaska[i+1] );	// S-48: bylo %lf
	};

	if( i1nte_atof_1( ch, &d ) != 0 ) d = 0.0;	// S-48: oshibka - tolko v etoi tochke (ne zalipaet)
	return d;
}

double CTMCGrExpression::GetExpressionValueSFR( int nPoint )
{
	int i, n;
	char ch[TMC_GROUT_MAXSTRING_BUF];
	double d = 0.0;	// S-48

	if( (IsError)||(pnPoint == NULL)||(ppcSmatrix == NULL)||(ppdFreq == NULL) ) return 0;
	if( (pnPoint[0] <= nPoint)||( nPoint < 0 ) ) return 0;	// I-14 (old: pnPoint[0] < nPoint - one past the array)
	GetExpressionValue( nPoint );
	
	strcpy( ch, pszMaska[0] );
	n = strlen( ch );
	for( i = 0; i < nVar; i++, n = strlen( ch ) )
	{
		d = GetCPhase( i, nPoint);
		snprintf( ch+n, TMC_GROUT_MAXSTRING_BUF-n, "(%.17g)%s", d, pszMaska[i+1] );	// S-48: bylo %lf
	};

	if( i1nte_atof_1( ch, &d ) != 0 ) d = 0.0;	// S-48: oshibka - tolko v etoi tochke (ne zalipaet)
	return d;
}

double CTMCGrExpression::GetExpressionValueSM( int nPoint )
{
	int i, n;
	char ch[TMC_GROUT_MAXSTRING_BUF];
	double d = 0.0;	// S-48

	if( (IsError)||(pnPoint == NULL)||(ppcSmatrix == NULL)||(ppdFreq == NULL) ) return 0;
	if( (pnPoint[0] <= nPoint)||( nPoint < 0 ) ) return 0;	// I-14 (old: pnPoint[0] < nPoint - one past the array)
	GetExpressionValue( nPoint );
	
	strcpy( ch, pszMaska[0] );
	n = strlen( ch );
	for( i = 0; i < nVar; i++, n = strlen( ch ) )
	{
		d = cabsv( ppcSmatrix[i][nPoint] );
		snprintf( ch+n, TMC_GROUT_MAXSTRING_BUF-n, "(%.17g)%s", d, pszMaska[i+1] );	// S-48: bylo %lf
	};

	if( i1nte_atof_1( ch, &d ) != 0 ) d = 0.0;	// S-48: oshibka - tolko v etoi tochke (ne zalipaet)
	return d;
}

double CTMCGrExpression::GetExpressionValueL( int nPoint )
{
	int i, n;
	char ch[TMC_GROUT_MAXSTRING_BUF];
	double d = 0.0;	// S-48

	if( (IsError)||(pnPoint == NULL)||(ppcSmatrix == NULL)||(ppdFreq == NULL) ) return 0;
	if( (pnPoint[0] <= nPoint)||( nPoint < 0 ) ) return 0;	// I-14 (old: pnPoint[0] < nPoint - one past the array)
	GetExpressionValue( nPoint );
	
	strcpy( ch, pszMaska[0] );
	n = strlen( ch );
	for( i = 0; i < nVar; i++, n = strlen( ch ) )
	{
		d = -20.*log10( cabsv( ppcSmatrix[i][nPoint] ) );
		snprintf( ch+n, TMC_GROUT_MAXSTRING_BUF-n, "(%.17g)%s", d, pszMaska[i+1] );	// S-48: bylo %lf
	};

	if( i1nte_atof_1( ch, &d ) != 0 ) d = 0.0;	// S-48: oshibka - tolko v etoi tochke (ne zalipaet)
	return d;
}

double CTMCGrExpression::GetExpressionValueK( int nPoint )
{
	int i, n;
	char ch[TMC_GROUT_MAXSTRING_BUF];
	double d = 0.0, r11;	// S-48

	if( (IsError)||(pnPoint == NULL)||(ppcSmatrix == NULL)||(ppdFreq == NULL) ) return 0;
	if( (pnPoint[0] <= nPoint)||( nPoint < 0 ) ) return 0;	// I-14 (old: pnPoint[0] < nPoint - one past the array)
	GetExpressionValue( nPoint );
	
	strcpy( ch, pszMaska[0] );
	n = strlen( ch );
	for( i = 0; i < nVar; i++, n = strlen( ch ) )
	{
		r11 = (double)(cabsv( ppcSmatrix[i][nPoint] ));
		if( fabs( r11 - 1.) < 0.0001	)
		{
			r11 = 0.9999;
		}
		d = (1. + r11)/(1. - r11 );
		snprintf( ch+n, TMC_GROUT_MAXSTRING_BUF-n, "(%.17g)%s", d, pszMaska[i+1] );	// S-48: bylo %lf
	};

	if( i1nte_atof_1( ch, &d ) != 0 ) d = 0.0;	// S-48: oshibka - tolko v etoi tochke (ne zalipaet)
	return d;
}

int g_nTmcPhaseUnwrap = 0;	// D-8: 0 - monotonically decreasing (default), 1 - standard (threshold pi)

double CTMCGrExpression::GetCPhase( int i, int nPoint )
{
	if( (IsError)||(pnPoint == NULL)||(ppcSmatrix == NULL)||(ppdFreq == NULL) ) return 0;
	if( (pnPoint[0] <= nPoint)||( nPoint < 0 ) ) return 0;	// I-14 (old: pnPoint[0] < nPoint - one past the array)

	int j, n;
	double d, d1;
	
	for( j = 1, n = 0, d1 = cphase( ppcSmatrix[i][0] ); j <= nPoint; j++, d1 = d )
	{
		d = cphase( ppcSmatrix[i][j] );
		if( g_nTmcPhaseUnwrap == 1 )	// D-8: the standard unwrapping, the threshold pi
		{
			if( d - d1 > 3.141592653589 ) n++;
			else if( d - d1 < -3.141592653589 ) n--;
		}
		else if( d > d1 ) n++;		// the phase decreases monotonically (as before)
	};

	return d1-2*3.141592653589*n;
}
