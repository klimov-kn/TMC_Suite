// c2DArray.cpp: implementation of the c2DArray class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <io.h>
#include <fcntl.h>
#include <float.h>
#include <string.h>
#include <math.h>
#include <typedef.h>
#include <expr.h>
#include "c2DArray.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

c2DArray::c2DArray()
{
	nMaxTemporary = 400;
	bIsReduceElMagnAnalysis = TRUE;
	bIsInitialData = FALSE;
	dFreq = 0;
	nIzl  = 0;
	iN = 0;
	nPointForNormir = 1000;
	pcIzl = NULL;
	sprintf( szError, "No Error" );
	sprintf( szFileName, "" );
	sprintf( szFileNameOut, "" );
	sprintf( szOneExpression, "" );
	sprintf( szFileNameTemp, "" );	
	bIsError = FALSE;
	nAngle    = 400;
	pdAngle   = NULL;
	pdRequiredDN = NULL;
	pdRealDN  = NULL;
	pdImageDN = NULL;
	IsNormirovka = FALSE;
	dMax = -1.;
	InitAngle();
	dKnd = FLT_MIN;
	dKip = FLT_MIN;
	expr_set_angle( EXPR_RADIAN );
	return;
}

c2DArray::~c2DArray()
{
	if( nIzl < nMaxTemporary )
	{
		if( strlen(szFileNameTemp) >= 1 ) remove(szFileNameTemp);
	}

	if( strlen(szFileNameOut) >= 1 ) remove(szFileNameOut);

	DeleteAngle();
	DeleteData();

	return;
}

void c2DArray::DeleteData()
{
	nPointForNormir = 1000;
	dFreq = 0;
	nIzl  = 0;
	iN = 0;
	if( pcIzl != NULL )
	{
		delete []pcIzl;
		pcIzl = NULL;
	};
	IsNormirovka = FALSE;
	dMax = -1.;
	sprintf( szError, "No Error" );
	sprintf( szFileName, "" );
	sprintf( szFileNameOut, "" );
	sprintf( szOneExpression, "" );
	bIsError = FALSE;
	dKnd = FLT_MIN;
	dKip = FLT_MIN;
	return;
}

BOOL c2DArray::IsError()
{
	return bIsError;
}

char* c2DArray::GetError()
{
	return szError;
}

int c2DArray::GetnAngle()
{
	return nAngle;
}

void c2DArray::ReadData(char *szFileName1)
{
	if( (nIzl > nMaxTemporary)&&bIsInitialData ) return;
	DeleteData();
	sprintf( szFileName, "%s", szFileName1 );
	sprintf( szFileNameTemp, "%s_Temp%d", szFileName, bIsReduceElMagnAnalysis );
	ReadData();
	return;
}

void c2DArray::WriteData(char *szFileName1)
{
	if( (nIzl > nMaxTemporary)&&bIsInitialData ) return;

	sprintf( szFileNameOut, "%s%d", szFileName1, bIsReduceElMagnAnalysis );
	WriteData();
	return;
}

void c2DArray::ReadData()
{
	if( bIsError ) return;

	if( strlen( szFileName ) == 0 )
	{
		bIsError = TRUE;
		sprintf( szError, "Input File name is NULL");
		return;
	}

	FILE *fp;
	char szBuf[TMC_C2DARRAY_MAX_STRING];
	char szBuf1[TMC_C2DARRAY_MAX_STRING];
	int i, j;
	double x, y, a, f, dFiMax, dFiMin;

	if( ( fp = fopen( szFileName , "r") ) == NULL ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when open input data file %s", szFileName);
		return;
	};


	if( fgets( szBuf, TMC_C2DARRAY_MAX_STRING, fp ) == NULL ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when read Frequence from input data file %s", szFileName);
		fclose( fp );
		return;
	};
	if( sscanf( szBuf, "Frequence = %lg Ghz;", &dFreq ) != 1 ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when read Frequence from input data file %s", szFileName);
		fclose( fp );
		return;
	};
	dFreq *= 1000000000;

	if( fgets( szBuf, TMC_C2DARRAY_MAX_STRING, fp ) == NULL ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when read Number of emitter from input data file %s", szFileName);
		fclose( fp );
		return;
	};
	if( sscanf( szBuf, "N = %d;", &nIzl ) != 1 ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when read Number of emitter from input data file %s", szFileName);
		fclose( fp );
		return;
	};
	
	if( (nIzl > nMaxTemporary)&&bIsInitialData ) 
	{
		fclose( fp );
		return;
	};

	if( (nIzl > nMaxTemporary) ) 
	{
		nAngle = 1000;
		InitAngle();
		double dAngleMax, dAngleMin, dStep;

		dAngleMax = 3.141592653589;
		dAngleMin = -3.141592653589;

		for( i = 0, dStep = (dAngleMax - dAngleMin)/(nAngle - 1); i < nAngle; i++ )
		{
			pdAngle[ i ] = dAngleMin + i*dStep;
		}
	};

	if( fgets( szBuf, TMC_C2DARRAY_MAX_STRING, fp ) == NULL ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when read Number of emitter from input data file %s", szFileName);
		fclose( fp );
		return;
	};
	if( strlen( szBuf ) < 1 ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when read Expression for Directional pattern of one emitter from input data file %s", szFileName);
		fclose( fp );
		return;
	};
	strcpy( szOneExpression, szBuf );
	
	if( nIzl < 1 )
	{
		bIsError = TRUE;
		sprintf( szError, "Error when read Number of emitter = %d < 1 from input data file %s", nIzl, szFileName);
		fclose( fp );
		return;
	};

	InitData();
	if( bIsError )
	{
		fclose( fp );
		return;
	};

	for( i = 0; i < nIzl; i++ )
	{
		if( fgets( szBuf, TMC_C2DARRAY_MAX_STRING, fp ) == NULL ) 
		{
			bIsError = TRUE;
			sprintf( szError, "Error when read emitter N %d from input data file %s", i, szFileName);
			fclose( fp );
			return;
		};
		if( sscanf( szBuf, "%d   X = %lg mm; Y = %lg mm; Amplitude = %lg; Faza = %lg degree; FiMin = %lg degree; FiMax = %lg degree; %s", &j, &x, &y, &a, &f, &dFiMin, &dFiMax, szBuf1 ) != 8 )
		{
			dFiMin = -180.;
			dFiMax =  180.;
			szBuf1[0] = '\0';

			if( sscanf( szBuf, "%d   X = %lg mm; Y = %lg mm; Amplitude = %lg; Faza = %lg degree;", &j, &x, &y, &a, &f ) != 5 ) 
			{
				bIsError = TRUE;
				sprintf( szError, "Error when read emitter N %d from input data file %s", i, szFileName);
				fclose( fp );
				return;
			};
		};
//		if( (i+1) != j ) 
//		{
//			bIsError = TRUE;
//			sprintf( szError, "Error when read emitter N %d from input data file %s", i, szFileName);
//			fclose( fp );
//			return;
//		};
		pcIzl[i].SetdXcoordinata(x/1000.);
		pcIzl[i].SetdYcoordinata(y/1000.);
		pcIzl[i].SetdAmplituda(a);
		pcIzl[i].SetdFaza(f*3.141592653589/180.);

		if( !bIsReduceElMagnAnalysis )
		{
			dFiMin = -180.;
			dFiMax =  180.;
		};

		pcIzl[i].SetdFimin(dFiMin*3.141592653589/180.);
		pcIzl[i].SetdFimax(dFiMax*3.141592653589/180.);
		pcIzl[i].SetcsEmitterDirectPat( szBuf1 );
	};

	fclose( fp );
	return;
}

void c2DArray::WriteData()
{
	if( bIsError ) return;
	if( (nIzl > nMaxTemporary)&&bIsInitialData ) return;

	if( strlen( szFileNameOut ) == 0 )
	{
		bIsError = TRUE;
		sprintf( szError, "Output File name is NULL");
		return;
	}

	FILE *fp;
	int i;
	double x, y, a, f, dFiMax, dFiMin;
	CString csBuf;

	if( ( fp = fopen( szFileNameOut , "w") ) == NULL ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when open output data file %s", szFileNameOut);
		return;
	};


	dFreq /= 1000000000;
	if( fprintf( fp, "Frequence = %lg Ghz;\n", dFreq ) < 0 ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when write Frequence to output data file %s", szFileNameOut);
		fclose( fp );
		return;
	};
	dFreq *= 1000000000;

	if( fprintf( fp, "N = %d;\n", nIzl ) < 0 ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when write Number of emitter to output data file %s", szFileNameOut);
		fclose( fp );
		return;
	};

	if( fprintf( fp, "%s\n", szOneExpression ) < 0 ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when write Expression for one element of array of emitter to output data file %s", szFileNameOut);
		fclose( fp );
		return;
	};

	for( i = 0; i < nIzl; i++ )
	{
		x = 1000 * pcIzl[i].GetdXcoordinata();
		y = 1000 * pcIzl[i].GetdYcoordinata();
		a = pcIzl[i].GetdAmplituda();
		f = pcIzl[i].GetdFaza()*180./3.141592653589;
		dFiMin = pcIzl[i].GetdFimin()*180./3.141592653589;
		dFiMax = pcIzl[i].GetdFimax()*180./3.141592653589;
		csBuf = pcIzl[i].GetcsEmitterDirectPat();
		if(strlen(csBuf) < 1)
		{
			csBuf.Format("1.");
		};
		if( fprintf( fp, "%d   X = %lg mm; Y = %lg mm; Amplitude = %lg; Faza = %lg degree; FiMin = %lg degree; FiMax = %lg degree; %s\n", i+1, x, y, a, f, dFiMin, dFiMax, csBuf ) < 0 ) 
		{
			bIsError = TRUE;
			sprintf( szError, "Error when write emitter N %d to output data file %s", i+1, szFileNameOut);
			fclose( fp );
			return;
		};
	};

	if( IsCalculateKip() )
	{
		if( fprintf( fp, "\nKND = %lg;\nKND = %lg dB;\nEmitted Power = %lg;\n\nDiagrNapr\nAngleDegree\tAmplitue\tFazaDegree\n", GetKnd(), GetKnd_dB(), GetKip() ) < 0 ) 
		{
			bIsError = TRUE;
			sprintf( szError, "Error when write Number of emitter to output data file %s", szFileNameOut);
			fclose( fp );
			return;
		};
	}
	else
	{
		if( fprintf( fp, "\nKND = %lg;\nKND = %lg dB;\nKip = %lg;\nKip = %lg dB;\nDiagrNapr\nAngleDegree\tAmplitue\tFazaDegree\n", GetKnd(), GetKnd_dB(), GetKip(), GetKip_dB() ) < 0 ) 
		{
			bIsError = TRUE;
			sprintf( szError, "Error when write Number of emitter to output data file %s", szFileNameOut);
			fclose( fp );
			return;
		};
	};

	for( i = 0; i < nAngle; i++ )
	{
		a = pdRealDN[i];
		f = pdImageDN[i]*180./3.141592653589;
		if( fprintf( fp, "%lg\t%lg\t%lg\n", pdAngle[i]/3.141592653589*180., a, f ) < 0 ) 
		{
			bIsError = TRUE;
			sprintf( szError, "Error when write emitter N %d to output data file %s", i+1, szFileNameOut);
			fclose( fp );
			return;
		};
	};

	fclose( fp );

	WriteTemporaryFile();

	return;
}


void c2DArray::InitData()
{
	if( bIsError ) return;
	if( nIzl < 1 )
	{
		bIsError = TRUE;
		sprintf( szError, "Error Number of emitter = %d < 1 ", nIzl);
		return;
	};

	if( pcIzl != NULL )
	{
		delete []pcIzl;
		pcIzl = NULL;
	};

	if( ( pcIzl = new cIzl[ nIzl ] ) == NULL )
	{
		bIsError = TRUE;
		sprintf( szError, "Error when allocation memory for emitter array ");
		return;
	};

	if( (nAngle < 2)||( pdRealDN == NULL )||( pdImageDN == NULL ) ) return;
	int i;
	for( i = 0; i < nAngle; i++)
	{
		pdRealDN[ i ] = 0.0;
		pdImageDN[ i ] = 0.0;
	}

	return;
}

void c2DArray::CalculateDiagrNapr(char *szInputFileName, char *szOutputFileName)
{
	ReadData(   szInputFileName );
	CalculateDiagrNapr();
	WriteData( szOutputFileName );
	return;
}

void c2DArray::CalculateDiagrNapr()
{
	if( bIsError ) return;
	if( (nAngle < 2)||( pdRealDN == NULL )||( pdImageDN == NULL ) ) return;
	if( (nIzl > nMaxTemporary)&&bIsInitialData ) return;

	double dReal, dImage, r1;
	char szBuf1[TMC_C2DARRAY_MAX_STRING];

	if( IsDirectPatSaveInFile() )
	{
		ReadDataFromTempFile();
	}
	else
	{		
		if( strcmp("1.", szOneExpression) == 0 )
		{
			for( iN = 0; iN < nAngle; iN++)
			{
				dReal  = CalculateRealDN( pdAngle[ iN ] );
				dImage = CalculateImageDN( pdAngle[ iN ] );
				pdRealDN[ iN ] = sqrt( dReal*dReal + dImage*dImage );
				pdImageDN[ iN ] = atan2( dImage, dReal );
			};
		}
		else
		{
			for( iN = 0; iN < nAngle; iN++)
			{
				sprintf( szBuf1, "f=(%lg);", pdAngle[ iN ]);
				if( i1nte_atof_1( szBuf1, &r1 ) != 0 )
				{
					bIsError = TRUE;
					sprintf( szError, "Error when calculate expression for one Directional Pattern {%s}", expr_get_error());
					return;
				};
				if( i1nte_atof_1( szOneExpression, &r1 ) != 0 )
				{
					bIsError = TRUE;
					sprintf( szError, "Error when calculate expression for one Directional Pattern {%s}", expr_get_error());
					return;
				};
				dReal  = r1*CalculateRealDN( pdAngle[ iN ] );
				dImage = r1*CalculateImageDN( pdAngle[ iN ] );
				pdRealDN[ iN ] = sqrt( dReal*dReal + dImage*dImage );
				pdImageDN[ iN ] = atan2( dImage, dReal );
			};
		};
	};

	NormirovkaDiagrNapr();

	return;
}

void c2DArray::InitAngle()
{
	if( bIsError ) return;
	if( nAngle < 2 )
	{
		bIsError = TRUE;
		sprintf( szError, "Error Number of angle = %d < 2 ", nAngle);
		return;
	};
	if( pdRequiredDN != NULL )
	{
		delete []pdRequiredDN;
		pdRequiredDN = NULL;
	};
	if( pdRealDN != NULL )
	{
		delete []pdRealDN;
		pdRealDN = NULL;
	};

	if( ( pdRequiredDN = new double[ nAngle ] ) == NULL )
	{
		bIsError = TRUE;
		sprintf( szError, "Error when allocation memory for angle array ");
		return;
	};

	if( ( pdRealDN = new double[ nAngle ] ) == NULL )
	{
		bIsError = TRUE;
		sprintf( szError, "Error when allocation memory for angle array ");
		return;
	};

	if( pdImageDN != NULL )
	{
		delete []pdImageDN;
		pdImageDN = NULL;
	};
	if( ( pdImageDN = new double[ nAngle ] ) == NULL )
	{
		bIsError = TRUE;
		sprintf( szError, "Error when allocation memory for angle array ");
		return;
	};

	if( pdAngle != NULL )
	{
		delete []pdAngle;
		pdAngle = NULL;
	};
	if( ( pdAngle = new double[ nAngle ] ) == NULL )
	{
		bIsError = TRUE;
		sprintf( szError, "Error when allocation memory for angle array ");
		return;
	};

	int i;
	double dStep;

	for( i = 0, dStep = 3.141592653589/(nAngle - 1); i < nAngle; i++ )
	{
		pdAngle[ i ] = i*dStep;
	}

	return;
}

void c2DArray::DeleteAngle()
{
	nAngle  = 0;
	if( pdAngle != NULL )
	{
		delete []pdAngle;
		pdAngle = NULL;
	};	
	if( pdRequiredDN != NULL )
	{
		delete []pdRequiredDN;
		pdRequiredDN = NULL;
	};
	if( pdRealDN != NULL )
	{
		delete []pdRealDN;
		pdRealDN = NULL;
	};
	if( pdImageDN != NULL )
	{
		delete []pdImageDN;
		pdImageDN = NULL;
	};
}

double c2DArray::CalculateRealDN(double dAngle)
{
	if( bIsError ) return 0.;
	if( (nIzl < 1)||( pcIzl == NULL ) ) return 0.;

	int i;
	double d, a, f, x, y, c, alf1;

	for( i = 0, d = 0.0; i < nIzl; i++)
	{
		a = pcIzl[i].GetdAmplituda( dAngle );
		f = pcIzl[i].GetdFaza();
		x = pcIzl[i].GetdXcoordinata();
		y = pcIzl[i].GetdYcoordinata();
		c = sqrt( x*x +y*y );
		alf1 = atan2( y, x );
		d += a*cos( f + 2*3.141592653589*dFreq*c*cos( dAngle - alf1 )/299792458. );
	}

	return d;
}

double c2DArray::CalculateImageDN(double dAngle)
{
	if( bIsError ) return 0.;
	if( (nIzl < 1)||( pcIzl == NULL ) ) return 0.;

	int i;
	double d, a, f, x, y, c, alf1;

	for( i = 0, d = 0.0; i < nIzl; i++)
	{
		a = pcIzl[i].GetdAmplituda( dAngle );
		f = pcIzl[i].GetdFaza();
		x = pcIzl[i].GetdXcoordinata();
		y = pcIzl[i].GetdYcoordinata();
		c = sqrt( x*x +y*y );
		alf1 = atan2( y, x );
		d += a*sin( f + 2*3.141592653589*dFreq*c*cos( dAngle - alf1 )/299792458. );
	}

	return d;
}

double c2DArray::CalculateRealDN_ForKip(double dAngle)
{
	if( bIsError ) return 0.;
	if( (nIzl < 1)||( pcIzl == NULL ) ) return 0.;

	int i;
	double d, a, f, x, y, c, alf1;

	for( i = 0, d = 0.0; i < nIzl; i++)
	{
		a = 1.;
		f = 0.;
		x = pcIzl[i].GetdXcoordinata();
		y = pcIzl[i].GetdYcoordinata();
		c = sqrt( x*x +y*y );
		alf1 = atan2( y, x );
		d += a*cos( f + 2*3.141592653589*dFreq*c*cos( dAngle - alf1 )/299792458. );
	}

	return d;
}

double c2DArray::CalculateImageDN_ForKip(double dAngle)
{
	if( bIsError ) return 0.;
	if( (nIzl < 1)||( pcIzl == NULL ) ) return 0.;

	int i;
	double d, a, f, x, y, c, alf1;

	for( i = 0, d = 0.0; i < nIzl; i++)
	{
		a = 1.;
		f = 0.;
		x = pcIzl[i].GetdXcoordinata();
		y = pcIzl[i].GetdYcoordinata();
		c = sqrt( x*x +y*y );
		alf1 = atan2( y, x );
		d += a*sin( f + 2*3.141592653589*dFreq*c*cos( dAngle - alf1 )/299792458. );
	}

	return d;
}

void c2DArray::NormirovkaDiagrNapr()
{
	if( bIsError ) return;
	if( (nAngle < 2)||( pdRealDN == NULL )||( pdImageDN == NULL ) ) return;

	int i;
	double x, y, r, r1, dAng, dStep;
	double xKip, yKip, rKip, dMaxKip;
	char szBuf1[TMC_C2DARRAY_MAX_STRING];
	
	if( !IsNormirovka )
	{
		IsNormirovka = TRUE;
		if( IsCalculateKip() )
		{
			for( i = 0, dKip = dKnd = dMax = 0.0, dStep = 2*3.141592653589/(nAngle-1); i < nAngle; i++)
			{
				if( pdRealDN[ i ] > dMax ) dMax = pdRealDN[ i ];
			};
			for( i = 0; i < nAngle; i++)
			{
				dKnd += pdRealDN[ i ]*pdRealDN[ i ]*dStep;
//				dKip += pdRealDN[ i ]*dStep;
				pdRealDN[ i ] /= dMax;
			};
//			dKip = dKnd/2/(2*3.141592653589);
			dKip = dKnd;
			dKnd = (2*3.141592653589)*dMax*dMax/dKnd;
			return;
		};

		for( i = 0, dAng = 0.0, dStep = 2*3.141592653589/(nPointForNormir-1), dMax = 0.0, dMaxKip = 0.0, dKnd = 0.0, dKip = 0.0; i < nPointForNormir; i++, dAng += dStep)
		{
			sprintf( szBuf1, "f=(%lg);", dAng);
			if( i1nte_atof_1( szBuf1, &r1 ) != 0 )
			{
				bIsError = TRUE;
				sprintf( szError, "Error when calculate expression for one Directional Pattern {%s}", expr_get_error());
				return;
			};
			if( i1nte_atof_1( szOneExpression, &r1 ) != 0 )
			{
				bIsError = TRUE;
				sprintf( szError, "Error when calculate expression for one Directional Pattern {%s}", expr_get_error());
				return;
			};
			x = r1*CalculateRealDN( dAng );
			y = r1*CalculateImageDN( dAng );
			r = x*x + y*y;
			dKnd += dStep*r;
			if( r > dMax ) dMax = r;
			
			xKip = CalculateRealDN_ForKip( dAng );
			yKip = CalculateImageDN_ForKip( dAng );
			rKip = xKip*xKip + yKip*yKip;
			dKip += dStep*rKip;
			if( rKip > dMaxKip ) dMaxKip = rKip;
		};

		dKnd = 2*3.141592653589*dMax*dMax/dKnd;
		
		dKip = 2*3.141592653589*dMaxKip*dMaxKip/dKip;
		dKip = dKnd/dKip;
		
		dMax = sqrt( dMax );
		dMax *= 1.001;

		if( dMax < DBL_MIN ) return;
	};

	for( i = 0; i < nAngle; i++)
	{
		pdRealDN[ i ] /= dMax;
	}

	return;
}

double c2DArray::GetdAngle(int i)
{
	if( bIsError ) return 0.0;
	if( (nAngle < 2)||(nAngle <= i)||( pdRealDN == NULL )||( pdImageDN == NULL ) ) return 0.0;
	return pdAngle[ i ];
}

double c2DArray::GetdAmplitudeDiagrNapr(int i)
{
	if( bIsError ) return 0.0;
	if( (nAngle < 2)||(nAngle <= i)||( pdRealDN == NULL )||( pdImageDN == NULL ) ) return 0.0;
	return pdRealDN[ i ];
}

void c2DArray::ResizeAngle(double dAngleMin, double dAngleMax)
{
	if( bIsError ) return;
	if( (nAngle < 2)||( pdAngle == NULL )||( pdRealDN == NULL )||( pdImageDN == NULL ) ) return;
	if( nAngle < 2 )
	{
		bIsError = TRUE;
		sprintf( szError, "Error Number of angle = %d < 2 ", nAngle);
		return;
	};
	
	if( (nIzl > nMaxTemporary)&&bIsInitialData ) return;

	int i;
	double dStep;

	for( i = 0, dStep = (dAngleMax - dAngleMin)/(nAngle - 1); i < nAngle; i++ )
	{
		pdAngle[ i ] = dAngleMin + i*dStep;
	}

	CalculateDiagrNapr();
	WriteData();

	return;
}

double c2DArray::GetKnd()
{
	if( bIsError ) return 0.0;
	return dKnd;
}

double c2DArray::GetKnd_dB()
{
	if( bIsError ) return 0.0;
	return 10.*log10(dKnd);
}

double c2DArray::GetKip()
{
	if( bIsError ) return 0.0;
	return dKip;
}

double c2DArray::GetKip_dB()
{
	if( bIsError ) return 0.0;
	return 10.*log10(dKip);
}

int c2DArray::GetnIzl()
{
	if( bIsError ) return 0;
	return nIzl;
}

void c2DArray::SetOneExpression(char *ch)
{
	if( bIsError ) return;
	if( strlen(ch) >= TMC_C2DARRAY_MAX_STRING )
	{
		bIsError = TRUE;
		sprintf( szError, "Error Lenght of expression >= %d", TMC_C2DARRAY_MAX_STRING);
		return;
	};

	strcpy( szOneExpression, ch );

	return;
}

char * c2DArray::GetOneExpression()
{
	return szOneExpression;
}

double c2DArray::GetRealIzl(int i)
{
	if( bIsError ) return 0.;
	if( i < 0 ) return 0.;
	if( nIzl <= i ) return 0.;
	if( (nIzl < 1)||( pcIzl == NULL ) ) return 0.;

	double a, f;
	a = pcIzl[i].GetdAmplituda();
	f = pcIzl[i].GetdFaza();
	return a*cos(f);
}

double c2DArray::GetImageIzl(int i)
{
	if( bIsError ) return 0.;
	if( i < 0 ) return 0.;
	if( nIzl <= i ) return 0.;
	if( (nIzl < 1)||( pcIzl == NULL ) ) return 0.;

	double a, f;
	a = pcIzl[i].GetdAmplituda();
	f = pcIzl[i].GetdFaza();
	return a*sin(f);
}

void c2DArray::OptimizationInit(char *szFileRequiDirPat, int nIzl1)
{
	if( strlen(szFileRequiDirPat) < 1 )
	{
		bIsError = TRUE;
		sprintf( szError, "Error: File name for required directional pattern is bad {%s}", szFileRequiDirPat);
		return;
	};

	FILE *fp;
	char szBuf[TMC_C2DARRAY_MAX_STRING];
	int i;
	double f, d;

	if( ( fp = fopen( szFileRequiDirPat , "r") ) == NULL ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when open optimization file %s", szFileRequiDirPat);
		return;
	};

	for( i = 0; fgets( szBuf, TMC_C2DARRAY_MAX_STRING, fp ) != NULL ; )
	{
		if( sscanf( szBuf, "%lg%lg", &f, &d) == 2 ) i++;
	};

	fclose(fp);

	if( i < 2 ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error in optimization file %s. Number Angles points  = %d < 2", szFileRequiDirPat, i);
		return;
	};

	DeleteData();
	DeleteAngle();
	nAngle = i;
	nIzl = nIzl1;
	InitAngle();
	InitData();
	if( bIsError ) return;

	if( ( fp = fopen( szFileRequiDirPat , "r") ) == NULL ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when open optimization file %s", szFileRequiDirPat);
		return;
	};

	for( i = 0; i < nAngle ; )
	{
		if( fgets( szBuf, TMC_C2DARRAY_MAX_STRING, fp ) == NULL )
		{
			bIsError = TRUE;
			sprintf( szError, "Error when read optimization file %s", szFileRequiDirPat);
			fclose(fp);
			return;
		};
		if( sscanf( szBuf, "%lg%lg", &f, &d) == 2 )
		{
			pdAngle[ i ] = f*3.141592653589/180.;
			pdRequiredDN[ i ] = d;
			i++;
		};
	};

	fclose(fp);
	return;
}

void c2DArray::SetIzl(int i, double dXcoord_mm, double dYcoord_mm, double dAmplitud, double dFaza_gradus)
{
	if( bIsError ) return;
	if( (i < 0)||(i > nIzl) )
	{
		bIsError = TRUE;
		sprintf( szError, "Error when set emitter %d", i);
		return;
	}

	pcIzl[i].SetdXcoordinata(dXcoord_mm/1000.);
	pcIzl[i].SetdYcoordinata(dYcoord_mm/1000.);
	pcIzl[i].SetdAmplituda(dAmplitud);
	pcIzl[i].SetdFaza(dFaza_gradus*3.141592653589/180.);

	return;
}

double c2DArray::GetOptimPurposeFunction()
{
	if( bIsError ) return 0.;

	double d;
	int i;

	IsNormirovka = FALSE;
	CalculateDiagrNapr();
	if( bIsError ) return 0.;

	for( i = 0, d = 0.; i < nAngle; i++)
	{
		d += fabs( pdRealDN[i] - pdRequiredDN[i] );
	};

	return d;
}

void c2DArray::SetszOneExpression(char *ch)
{
	if( bIsError ) return;
	strcpy( szOneExpression, ch );
	return;
}

void c2DArray::SetdFrequence( double dFrequenceGHz )
{
	dFreq = dFrequenceGHz*1000000000.;
	return;
}

void c2DArray::SetNPointForNormirovka(int nPoint)
{
	if( nPoint <= 0 ) return;
	nPointForNormir = nPoint;
	return;
}

void c2DArray::MakeDeviation(char *szSourceFile, char *szDistinationFile, double dXmm, double dYmm, double dAmplProcent, double dFazaDegree)
{
	ReadData( szSourceFile );
	if( bIsError ) return;

	int i;
	double a, f, x, y;

	for( i = 0; i < nIzl; i++)
	{
		a = pcIzl[i].GetdAmplituda();
		pcIzl[i].SetdAmplituda( GetRnd( a, a*dAmplProcent/100.) );
		
		f = pcIzl[i].GetdFaza();
		pcIzl[i].SetdFaza(GetRnd( f, dFazaDegree*3.141592653589/180.));
		
		x = pcIzl[i].GetdXcoordinata();
		pcIzl[i].SetdXcoordinata(GetRnd( x, dXmm/1000.));
		
		y = pcIzl[i].GetdYcoordinata();
		pcIzl[i].SetdYcoordinata(GetRnd( y, dYmm/1000.));
	}

	SaveData( szDistinationFile );
	return;
}

double c2DArray::GetRnd(double dMedium, double dDelta)
{

//   srand( (unsigned)time( NULL ) );

   double d;

   d = RAND_MAX;

   d = 2.*rand()/d;

   d = dMedium + ( d - 1. )*dDelta;

   return d;
}

void c2DArray::SaveData(char *pszFile)
{
	if( bIsError ) return;
	if( strlen( pszFile ) == 0 )
	{
		bIsError = TRUE;
		sprintf( szError, "Output File name is NULL");
		return;
	}

	FILE *fp;
	int i;
	double x, y, a, f;

	if( ( fp = fopen( pszFile , "w") ) == NULL ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when open output data file %s", pszFile);
		return;
	};


	dFreq /= 1000000000;
	if( fprintf( fp, "Frequence = %lg Ghz;\n", dFreq ) < 0 ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when write Frequence to output data file %s", pszFile);
		fclose( fp );
		return;
	};
	dFreq *= 1000000000;

	if( fprintf( fp, "N = %d;\n", nIzl ) < 0 ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when write Number of emitter to output data file %s", pszFile);
		fclose( fp );
		return;
	};

	if( fprintf( fp, "%s", szOneExpression ) < 0 ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when write Expression for one element of array of emitter to output data file %s", pszFile);
		fclose( fp );
		return;
	};

	for( i = 0; i < nIzl; i++ )
	{
		x = 1000 * pcIzl[i].GetdXcoordinata();
		y = 1000 * pcIzl[i].GetdYcoordinata();
		a = pcIzl[i].GetdAmplituda();
		f = pcIzl[i].GetdFaza()*180./3.141592653589;
		if( fprintf( fp, "%d   X = %lg mm; Y = %lg mm; Amplitude = %lg; Faza = %lg degree;\n", i+1, x, y, a, f ) < 0 ) 
		{
			bIsError = TRUE;
			sprintf( szError, "Error when write emitter N %d to output data file %s", i+1, pszFile);
			fclose( fp );
			return;
		};
	};

	fclose( fp );
	return;

}


void c2DArray::GetIzl(int i, double *dXmm, double *dYmm, double *dAmpl, double *dFazaDegree)
{
	if( bIsError ) return;
	if( (i < 0)||(i > nIzl) )
	{
		bIsError = TRUE;
		sprintf( szError, "Error when get emitter %d", i);
		return;
	}

	*dXmm = pcIzl[i].GetdXcoordinata()*1000;
	*dYmm = pcIzl[i].GetdYcoordinata()*1000;
	*dAmpl = pcIzl[i].GetdAmplituda();
	*dFazaDegree = pcIzl[i].GetdFaza()*180./3.141592653589;

	return;
}

void c2DArray::SaveDeviationDirectionalPattern(char *pszFileName)
{
	CalculateDiagrNapr();
	if( bIsError ) return;

	if( strlen( pszFileName ) == 0 )
	{
		bIsError = TRUE;
		sprintf( szError, "Output File name is NULL");
		return;
	}

	if( (pdRealDN == NULL)||(pdImageDN == NULL)||(pdRequiredDN == NULL) )
	{
		bIsError = TRUE;
		sprintf( szError, "Output File name is NULL");
		return;
	}

	FILE *fp;
	int i;

	if( ( fp = fopen( pszFileName , "w") ) == NULL ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when open output data file %s", pszFileName);
		return;
	};

	if( fprintf( fp, "!Degree\tAmplitudeDirectPatternRequired\tAmplitudeDirectPattern\n") < 0 ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when write emitter N 0 to output data file %s", pszFileName);
		fclose( fp );
		return;
	};

	for( i = 0; i < nAngle; i++ )
	{
		if( fprintf( fp, "%lg\t%lg\t%lg\n", pdAngle[i]/3.141592653589*180., pdRequiredDN[i], pdRealDN[i] ) < 0 ) 
		{
			bIsError = TRUE;
			sprintf( szError, "Error when write emitter N %d to output data file %s", i+1, pszFileName);
			fclose( fp );
			return;
		};
	};

	fclose( fp );
	return;
}

double c2DArray::GetdFreqGHz()
{
	return dFreq/1000000000.;
}

void c2DArray::WriteTemporaryFile()
{
	if( bIsError ) return;
	if( (nIzl > nMaxTemporary)&&bIsInitialData ) return;

	FILE *fp;
	int i;
	double a, f;
	CString csBuf;

	if( ( fp = fopen( szFileNameTemp , "w") ) == NULL ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when open temporary data file %s", szFileNameTemp);
		return;
	};

	for( i = 0; i < nAngle; i++ )
	{
		a = pdRealDN[i];
		f = pdImageDN[i]*180./3.141592653589;
		if( fprintf( fp, "%lg\t%lg\t%lg\t%lg\n", pdAngle[i]/3.141592653589*180., a*dMax, a, f ) < 0 ) 
		{
			bIsError = TRUE;
			sprintf( szError, "Error when write emitter N %d to output data file %s", i+1, szFileNameOut);
			fclose( fp );
			return;
		};
	};

	fclose( fp );

	bIsInitialData = TRUE;
	
	return;
}

double c2DArray::GetdMax()
{
	return dMax;
}

BOOL c2DArray::IsCalculateKip()
{
	return (nIzl > nMaxTemporary);
}

BOOL c2DArray::IsDirectPatSaveInFile()
{
	if( nIzl < nMaxTemporary ) return FALSE;

	FILE *fp;
	if( ( fp = fopen( szFileName , "r") ) == NULL ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when open input data file %s", szFileName);
		return FALSE;
	};
	fclose( fp );
	if( ( fp = fopen( szFileNameTemp , "r") ) == NULL ) 
	{
		return FALSE;
	};
	fclose( fp );
	
	char szBuf[TMC_C2DARRAY_MAX_STRING];
    FILETIME LastWriteTimeInput, LastWriteTimeTemp;

	sprintf( szBuf, szFileName );
	LastWriteTimeInput = LastWriteTime( szBuf );
	sprintf( szBuf, szFileNameTemp );
	LastWriteTimeTemp = LastWriteTime( szBuf );

	if( CompareFileTime( &LastWriteTimeInput, &LastWriteTimeTemp) <= 0 )
	{
		return TRUE;
	};
	return FALSE;
}

void c2DArray::ReadDataFromTempFile()
{
	if( bIsError ) return;

	FILE *fp;
	double a, f, an, ugol;
	CString csBuf;

	if( ( fp = fopen( szFileNameTemp , "r") ) == NULL ) 
	{
		bIsError = TRUE;
		sprintf( szError, "Error when open output data file %s", szFileNameOut);
		return;
	};

	for( iN = 0; iN < nAngle; iN++ )
	{
		if( fscanf( fp, "%lg\t%lg\t%lg\t%lg\n", &ugol, &a, &an, &f ) < 0 ) 
		{
			bIsError = TRUE;
			sprintf( szError, "Error when read emitter N %d from temporary data file %s", iN+1, szFileNameTemp);
			fclose( fp );
			return;
		};
		pdAngle[iN] = ugol*3.141592653589/180.;
		pdRealDN[iN] = a;
		pdImageDN[iN] = f*3.141592653589/180.;
	};

	fclose( fp );

	return;

}

FILETIME c2DArray::LastWriteTime(char *szFileName1)
{
	HANDLE fh;
	FILETIME LastWriteTime1;
	LastWriteTime1.dwLowDateTime   = 0;
	LastWriteTime1.dwHighDateTime  = 0;

	fh =  CreateFile(
    szFileName1,				// pointer to name of the file 
    GENERIC_READ,			// access read mode 
    FILE_SHARE_READ,		// share mode 
    NULL,					// pointer to security descriptor 
    OPEN_EXISTING,			// how to create 
    FILE_ATTRIBUTE_NORMAL,	// file attributes 
    NULL					// handle to file with attributes to copy  
   );

	GetFileTime( fh, NULL, NULL, &LastWriteTime1);

	CloseHandle( fh );
	
	return LastWriteTime1;

}
