// TmcSMatrix.cpp : implementation file
//

#include "stdafx.h"
#include "TMCGROUT.h"
#include <tmcgrviw.h>
#include <proc_s.h>
#include <s_file.h>
#include <TmcSMatrix.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcSMatrix

CTmcSMatrix::CTmcSMatrix()
{
	csTFileName.Format("");
	csSFileName.Format("");
	csError.Format("No error");
	dTmin = 0.0;
	dTmax = 0.0;
	dFreq = 0.0;
	bIsError = FALSE;
	pcSmatrix = NULL;
	pdTsignal = NULL;
	pdTime = NULL;
	nInput = 0;
	nPoint = 0;
	nLineInSMatrix = -1;
	return;
}

CTmcSMatrix::~CTmcSMatrix()
{
	DeleteData();
	return;
}


/////////////////////////////////////////////////////////////////////////////
// CTmcSMatrix message handlers

void CTmcSMatrix::DeleteData()
{
	csTFileName.Format("");
	csSFileName.Format("");
	csError.Format("No error");
	csWarning.Format("");	// B-3.3
	dTmin = 0.0;
	dTmax = 0.0;
	dFreq = 0.0;
	bIsError = FALSE;
	if( pcSmatrix != NULL ) delete []pcSmatrix;
	pcSmatrix = NULL;
	if( pdTsignal != NULL ) delete []pdTsignal;
	pdTsignal = NULL;
	if( pdTime != NULL ) delete []pdTime;
	pdTime = NULL;
	nInput = 0;
	nPoint = 0;
	nLineInSMatrix = -1;
	return;
}

BOOL CTmcSMatrix::IsError()
{
	return bIsError;
}

void CTmcSMatrix::ReadTFile(CString csTFile, double dTmin1, double dTmax1)
{
	DeleteData();
	
	csTFileName.Format("%s", csTFile);
	dTmin = dTmin1;
	dTmax = dTmax1;
	ReadTFile();

	return;
}

CString & CTmcSMatrix::GetErrorMessage()
{
	return csError;
}

void CTmcSMatrix::Save(CString csSFileName1)
{
	if( IsError() ) return;
	
	csSFileName.Format("%s", csSFileName1);
	Save();

	return;
}

void CTmcSMatrix::Save(CString csSFileName1, double dFreq1)
{
	if( IsError() ) return;
	
	csSFileName.Format("%s", csSFileName1);
	// I-27 (K-6): the point is written under dFreq1, but S is calculated at FREQ of the .t - a warning on mismatch
	/// I-27: частота записи отличаетс€ от частоты расчЄта из `.t` больше чем на 1e-5 (отн.: в .t частота записана %lg Ч 6 знаков) Ч предупреждение.
	if( ( dFreq > 0.0 )&&( fabs( dFreq1 - dFreq ) > 1e-5*dFreq ) )
	{
		CString csW;
		csW.Format( "S of file {%s} is calculated at FREQ = %lg Hz of the .t, but is written under the frequency "
			"%lg Hz (the X value of the row); if X is a frequency, put the FREQ of the task there",
			(LPCSTR)csTFileName, dFreq, dFreq1 );
		if( csWarning.GetLength() > 0 ) csWarning += "; ";
		csWarning += csW;
	};
	dFreq = dFreq1;
	Save();

	return;
}

void CTmcSMatrix::Save()
{
	if( IsError() ) return;
	if( pcSmatrix == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Save: Init error S-matrix pointer is NULL in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( nInput < 1 )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Save: Init error nInput < 1 in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( nLineInSMatrix < 0 )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Save: Init error nLineInSMatrix < 0 in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};

	char szBuf1[TMC_GROUT_MAXSTRING_BUF];
	strcpy( szBuf1, csSFileName );

	if( save_S_matrix_element( nLineInSMatrix + 1, szBuf1, pcSmatrix, dFreq, nInput) != 0 )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Save: error when save to %s from file {%s}", (LPCSTR)( csSFileName ), (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};

	return;
}

void CTmcSMatrix::ReadTFile()
{
	if( IsError() ) return;

	if( dTmax <= dTmin )
	{
		PutErrorMessage("Tmax <= Tmin");
		return;
	};

	if( csTFileName.GetLength() == 0 )
	{
		PutErrorMessage("TFileName is NULL");
		return;
	}

	nPoint = CalcNPoint();
	nInput = CalcNInput();
	AllocTArray();
	ReadTArray();

	return;
}

void CTmcSMatrix::PutErrorMessage(char *ch)
{
	bIsError = TRUE;
	csError.Format("%s", ch);
	return;
}

int CTmcSMatrix::CalcNPoint()
{
	if( IsError() ) return 0;

	FILE *fp;
									// calculated number of point
	if( (fp = fopen( csTFileName, "r")) == NULL ) 
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Can't open file {%s}", (LPCSTR)( csTFileName ));	// A-26
		PutErrorMessage(szBuf);
		return 0;
	};

	char ch[TMC_GROUT_MAXSTRING_BUF];
	int i;
	double r;

	for( i = 0; fgets( ch, TMC_GROUT_MAXSTRING_BUF, fp) != NULL; )
	{
		if( ch[0] == ' ' ) 
		{
			r = gets_f1(ch);
			if( (r <= dTmax)&&(r >= dTmin) ) i++;
		};
		if( strncmp( ch, TMC_GROTS_FLOUTFreq_ID, strlen(TMC_GROTS_FLOUTFreq_ID) ) == 0 )
		{
			sscanf( ch + strlen(TMC_GROTS_FLOUTFreq_ID), "%lg", &dFreq);
		};
	};
	
	fclose(fp);

	if( i <= 5 ) 
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "nPoint is %d ( <=5 ) ( Tmin=%lg; Tmax=%lg ) in file {%s}", i, dTmin, dTmax, (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return 0;
	};

	return i;
}

int CTmcSMatrix::CalcNInput()
{
	if( IsError() ) return 0;

	FILE *fp;
									// calculated number of input
	if( (fp = fopen( csTFileName, "r")) == NULL ) 
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Can't open file {%s}", (LPCSTR)( csTFileName ));	// A-26
		PutErrorMessage(szBuf);
		return 0;
	};

	char ch[TMC_GROUT_MAXSTRING_BUF];
	int i;

	for( i = 0; fgets( ch, TMC_GROUT_MAXSTRING_BUF, fp) != NULL; )
	{
		if( ch[0] == ' ' ) 
		{
			i = gets_nNumbersInLine(ch);
			fclose(fp);
			i = (i - 2)/3;
			if( i < 1 )
			{
				char szBuf[TMC_GROUT_MAXSTRING_BUF];
				sprintf( szBuf, "Number of Input is %d ( < 1 ) in file {%s}", i, (LPCSTR)( csTFileName ));
				PutErrorMessage(szBuf);
				return 0;
			};
			return i;
		};
	};
	
	fclose(fp);

	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "nPoint is %d ( <=5 ) ( Tmin=%lg; Tmax=%lg ) in file {%s}", i, dTmin, dTmax, (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return 0;
	};

	return i;
}

double CTmcSMatrix::gets_f1(char *ch)
{
	int i;
	double r1;

	if( IsError() ) return -1.;

	if( sscanf( ch, "%d%lg", &i, &r1) != 2 )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Can't read time in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return -1.;
	};

	return r1;

}

int CTmcSMatrix::gets_nNumbersInLine(char *ch)
{
	int i, ii, j;
	for( i = 0, ii = 0, j = 0; (ch[i] != '\0')&&(ch[i] != '\n'); i++ )
	{
		switch(ch[i])
		{
			case ' ': ;
			case '\t':
				switch( j )
				{
					case 0:
						break;
					case 1:
						j = 0;
						break;
				};
				break;
			default:
				switch( j )
				{
					case 0:
						ii++;
						j = 1;
						break;
					case 1:
						break;
				};
				break;
		};
	};
	return ii;
}

void CTmcSMatrix::AllocTArray()
{
	if( IsError() ) return;

	if( pdTsignal != NULL ) delete []pdTsignal;
	pdTsignal = NULL;
	if( pdTime != NULL ) delete []pdTime;
	pdTime = NULL;
	
	if( nPoint < 1 ) 
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Init error nPoint < 1 in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( nInput < 1 )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Init error nInput < 1 in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};

	int n, i;

	n = nPoint*nInput*2;
	if( n <= 0 )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "nPoint*nInput is very large in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	}

	if( ( pdTsignal = new double[n] ) == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Memory allocation error for T-SignalArray in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( ( pdTime = new double[nPoint] ) == NULL )
	{
		if( pdTsignal != NULL ) delete []pdTsignal;
		pdTsignal = NULL;
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Memory allocation error for TimeArray in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};

	for( i = 0; i < n; i++ ) pdTsignal[ i ] = 0.0;

	for( i = 0; i < nPoint; i++ ) pdTime[ i ] = 0.0;

	return;
}

void CTmcSMatrix::ReadTArray()
{
	if( IsError() ) return;

	if( nPoint < 1 ) 
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Init error nPoint < 1 in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( nInput < 1 )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Init error nInput < 1 in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( pdTsignal == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Init error pdTsignal == NULL in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( pdTime == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Init error pdTime == NULL in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};

	int i;
	FILE *fp;
	char ch[TMC_GROUT_MAXSTRING_BUF];
	double r, *pdTsign;
									// read data array
	if( (fp = fopen( csTFileName, "r")) == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Error when open file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};

	for( i = 0, pdTsign = pdTsignal; fgets( ch, TMC_GROUT_MAXSTRING_BUF, fp) != NULL; )
	{
		if( IsError() )
		{
			fclose( fp );
			return;
		};
		if( ch[0] == ' ' ) 
		{
			r = gets_f1(ch);
			if( IsError() ) 
			{
				fclose( fp );
				return;
			};
			if( (r <= dTmax)&&(r >= dTmin) )
			{
				pdTime[ i ] = r;
				readTsignal( &pdTsign, ch );
				i++;
				if( i == nPoint )
				{
					fclose( fp );
					return;
				};
			};
		};
	};

	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Error when read file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
	};
	fclose( fp );

	return;
}

void CTmcSMatrix::readTsignal(double **pdTsign, char *ch)
{
	if( IsError() ) return;
	int i, ii;

	ii = skip_blank( ch );
	if( IsError() ) return;
	ii += skip_number( ch+ii);
	if( IsError() ) return;

	ii += skip_blank( ch+ii );
	if( IsError() ) return;
	ii += skip_number( ch+ii);
	if( IsError() ) return;

	for( i = 0; i < nInput; i++)
	{
		ii += skip_blank( ch+ii );
		if( IsError() ) return;
		ii += skip_number( ch+ii);
		if( IsError() ) return;
		ii += readTsignalOne( pdTsign, ch+ii);
		ii += readTsignalOne( pdTsign, ch+ii);
		if( IsError() ) return;
	};
	return;
}

int CTmcSMatrix::readTsignalOne(double **pdTsign, char *ch)
{
	if( IsError() ) return 0;

	int i = skip_blank( ch );
	double r;

	if( IsError() ) return 0;

	if( sscanf( ch + i, "%lg", &r ) != 1 )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Error when read file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return 0;
	};

	i += skip_number( ch + i );
	if( IsError() ) return 0;

	(*pdTsign)[0] = r;
	(*pdTsign)++;

	return i;
}


int CTmcSMatrix::skip_blank(char *ch)
{
	if( IsError() ) return 0;

	int i;
	for( i = 0; ch[i] != NULL; i++ )
	{
		switch( ch[i] )
		{
			case ' ': ;
			case '\t': ;
			case '\n': ;
				break;
			default: return i;
		};
	};
	return 0;

}

int CTmcSMatrix::skip_number(char *ch)
{
	if( IsError() ) return 0;

	int i;
	for( i = 0; ch[i] != NULL; i++ )
	{
		switch( ch[i] )
		{
			case ' ': ;
			case '\t': ;
			case '\n': ;
				return i;
			default: break;
		};
	};
	return 0;

}

void CTmcSMatrix::MakeSmatrix()
{
	if( IsError() ) return;

	AllocSArray();
	TtoS();
	SetnLineInSMatrix();

	// B-3.1: S = Uo/Up - divide the column by the incident wave of the excited input (the same two-point
	// estimate as for Uo, column 2*n of the .t). Old: S = Uo (true only for Amp = 1, Faza = 0); to return it,
	// delete this block. |Up - 1| < 1e-3 is the noise of the 7 digits of the .t - not divided.
	if( !IsError() )
	{
		int i, ii, n = nLineInSMatrix;
		_complex scUp;
		for( i = 1, ii = 0, scUp.x = 0.0, scUp.y = 0.0; i < nPoint; i++ )
		{
			if( TtoSOneInputOnePoint( &scUp, pdTsignal[2*n+(i-1)*nInput*2], pdTime[i-1], pdTsignal[2*n+i*nInput*2], pdTime[i]) ) ii++;
		};
		if( ii > 0 )
		{
			scUp.x /= ii;
			scUp.y /= ii;
		};
		double dUp2 = scUp.x*scUp.x + scUp.y*scUp.y;
		if( ( ii == 0 )||( !( dUp2 > 1e-24 ) ) )
		{
			char szBuf[TMC_GROUT_MAXSTRING_BUF];
			sprintf( szBuf, "No incident wave (Up) of the excited input %d in the window Tmin=%lg; Tmax=%lg in file {%s}",
				n + 1, dTmin, dTmax, (LPCSTR)csTFileName );
			PutErrorMessage(szBuf);
			return;
		};
		if( ( scUp.x - 1.0 )*( scUp.x - 1.0 ) + scUp.y*scUp.y >= 1e-6 )
		{
			for( i = 0; i < nInput; i++ )
			{
				double dRe = pcSmatrix[i].x, dIm = pcSmatrix[i].y;
				pcSmatrix[i].x = ( dRe*scUp.x + dIm*scUp.y )/dUp2;
				pcSmatrix[i].y = ( dIm*scUp.x - dRe*scUp.y )/dUp2;
			};
		};
	};

	// B-3.3 (author 29.10): pairs of zero samples in the window (the wave has not come yet) - a warning only,
	// the average is not changed (they enter it as 0, see TtoSOneInputOnePoint).
	/// Ѕ-3.3: считаютс€ пары соседних отсчЄтов, у которых оба значени€ равны 0, в столбцах Up и Uo
	/// каждого входа; если такие есть Ч предупреждение с входом, где их больше всего.
	if( !IsError() )
	{
		int n, i, k, nZero, nMax = 0, nInMax = 0, nInWith = 0;
		for( n = 0; n < nInput; n++ )
		{
			for( k = 0, nZero = 0; k < 2; k++ )	// k = 0 - Up, 1 - Uo
				for( i = 1; i < nPoint; i++ )
					if( ( fabs( pdTsignal[k+2*n+(i-1)*nInput*2] ) < 10*DBL_MIN )&&( fabs( pdTsignal[k+2*n+i*nInput*2] ) < 10*DBL_MIN ) )
					{
						// Up of an input that is not excited is 0 everywhere - not counted
						if( ( k == 0 )&&( n != nLineInSMatrix ) ) continue;
						nZero++;
					};
			if( nZero > 0 ) nInWith++;
			if( nZero > nMax ) { nMax = nZero; nInMax = n; };
		};
		if( nMax > 0 )
			csWarning.Format( "the SMATRIX window Tmin=%lg; Tmax=%lg of file {%s} contains pairs of zero samples "
				"(the wave has not come yet): %d of %d pairs at input %d (inputs with such pairs: %d). They enter the "
				"average as 0 and decrease |S|; if this is not meant, move Tmin of SMATRIX to the steady state",
				dTmin, dTmax, (LPCSTR)csTFileName, nMax, nPoint - 1, nInMax + 1, nInWith );
	};

	// D-1: S that is not a number (the field diverged) is an error - not NaN in the .s
	if( !IsError() )
	{
		int i;
		for( i = 0; i < nInput; i++ )
		{
			if( ( fabs( (double)pcSmatrix[i].x ) <= DBL_MAX )&&( fabs( (double)pcSmatrix[i].y ) <= DBL_MAX ) ) continue;
			char szBuf[TMC_GROUT_MAXSTRING_BUF];
			sprintf( szBuf, "S of input %d is not a number (NaN or Inf: the field of the calculation diverged) in file {%s}",
				i + 1, (LPCSTR)csTFileName );
			PutErrorMessage(szBuf);
			return;
		};
	};

	return;
}

void CTmcSMatrix::AllocSArray()
{
	if( IsError() ) return;

	if( pcSmatrix != NULL ) delete []pcSmatrix;
	pcSmatrix = NULL;

	nLineInSMatrix = -1;
	
	if( nInput < 1 )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Init error nInput < 1 in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};

	if( ( pcSmatrix = new _complex[nInput] ) == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "Memory allocation error for TimeArray in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};

	int i;

	for( i = 0; i < nInput; i++ ) 
	{
		pcSmatrix[ i ].x = FLT_MAX;
		pcSmatrix[ i ].y = FLT_MAX;
	};

	return;
}

void CTmcSMatrix::TtoS()
{
	if( IsError() ) return;
	if( pcSmatrix == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "TtoS: Init error S-matrix pointer is NULL in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( nPoint < 1 ) 
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "TtoS: Init error nPoint < 1 in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( nInput < 1 )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "TtoS: Init error nInput < 1 in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( pdTsignal == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "TtoS: Init error pdTsignal == NULL in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( pdTime == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "TtoS: Init error pdTime == NULL in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};

	int i;

	for( i = 0; i < nInput; i++ )
	{
		TtoSOneInput( i );
		if( IsError() ) return;
	};

	return;
}

void CTmcSMatrix::TtoSOneInput(int n)
{
	if( IsError() ) return;
	if( pcSmatrix == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "TtoSOne: Init error S-matrix pointer is NULL in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( nPoint < 2 ) 
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "TtoSOne: Init error nPoint < 2 in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( nInput < n )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "TtoSOne: Init error nInput < %d in file {%s}", n, (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( pdTsignal == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "TtoSOne: Init error pdTsignal == NULL in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( pdTime == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "TtoSOne: Init error pdTime == NULL in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};

	int i, ii;
	_complex scSFor2Point;

	for( i = 1, ii = 0, scSFor2Point.x = 0.0, scSFor2Point.y = 0.0; i < nPoint; i++ )
	{
		if( TtoSOneInputOnePoint( &scSFor2Point, pdTsignal[1+2*n+(i-1)*nInput*2], pdTime[i-1], pdTsignal[1+2*n+i*nInput*2], pdTime[i]) ) ii++;
	};

	if( ii == 0 )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "TtoSOne: input %d in file {%s}", n, (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};

	pcSmatrix[n].x = scSFor2Point.x/ii;
	pcSmatrix[n].y = scSFor2Point.y/ii;

	return;
}

BOOL CTmcSMatrix::TtoSOneInputOnePoint(_complex *pscSFor2Point, double dTsignal1, double dTime1, double dTsignal2, double dTime2)
{
	if( (fabs( dTsignal1 ) < 10*DBL_MIN)&&(fabs( dTsignal2 ) < 10*DBL_MIN) )
	{
		pscSFor2Point->x += 0.0;
		pscSFor2Point->y += 0.0;
		return TRUE;
	};

	double dFi, dwt1, dwt2, dAmpl, dsin1, dsin2, dcos1;

	dwt1 = 2*3.141592653589*dFreq*dTime1;
	dwt2 = 2*3.141592653589*dFreq*dTime2;
	dsin1 = dTsignal2*sin( dwt1 ) - dTsignal1*sin( dwt2 );
	dcos1 = dTsignal2*cos( dwt1 ) - dTsignal1*cos( dwt2 );

	if( (fabs( dsin1 ) < 10*DBL_MIN)&&(fabs( dcos1 ) < 10*DBL_MIN) )
	{
		return FALSE;
	};

	if( fabs( dsin1 ) < fabs( dcos1 ) )
	{
		dFi = atan(-dsin1/dcos1);
	}
	else
	{
		if( fabs(dcos1) < FLT_MIN ) return FALSE;
		dFi = atan(-dsin1/dcos1);
	};

	dsin1 = sin( dwt1 + dFi );
	dsin2 = sin( dwt2 + dFi );

	if( (fabs( dsin1 ) < 10*DBL_MIN)&&(fabs( dsin2 ) < 10*DBL_MIN) )
	{
		return FALSE;
	};

	if( fabs( dsin2 ) < fabs( dsin1 ) )
	{
		dAmpl = dTsignal1/dsin1;
	}
	else
	{
		dAmpl = dTsignal2/dsin2;
	};

	if( dAmpl < 0. )
	{
		dAmpl = -dAmpl;
		dFi -= 3.141592653589;
	};

	pscSFor2Point->x += dAmpl*cos( dFi );
	pscSFor2Point->y += dAmpl*sin( dFi );

	return TRUE;
}

void CTmcSMatrix::SetnLineInSMatrix()
{
	if( IsError() ) return;
	if( pcSmatrix == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "SetnLineInSMatrix: Init error S-matrix pointer is NULL in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( nPoint < 1 ) 
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "SetnLineInSMatrix: Init error nPoint < 1 in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( nInput < 1 )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "SetnLineInSMatrix: Init error nInput < 1 in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( pdTsignal == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "SetnLineInSMatrix: Init error pdTsignal == NULL in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};
	if( pdTime == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "SetnLineInSMatrix: Init error pdTime == NULL in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};

	int i;

	nLineInSMatrix = -1;

	for( i = 0; i < nInput; i++ )
	{
		if( IsInputExcite( i ) )
		{
			if( nLineInSMatrix != -1 )
			{
				char szBuf[TMC_GROUT_MAXSTRING_BUF];
				sprintf( szBuf, "Not one is exited of inputs in file {%s}", (LPCSTR)( csTFileName ));
				PutErrorMessage(szBuf);
				return;
			};
			nLineInSMatrix = i;
		};
		if( IsError() ) return;
	};

	if( nLineInSMatrix == -1 )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "There are no exited inputs in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return;
	};

	return;
}

BOOL CTmcSMatrix::IsInputExcite(int n)
{
	if( IsError() ) return FALSE;
	if( nPoint < 1 ) 
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "IsInputExcite: Init error nPoint < 1 in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return FALSE;
	};
	if( nInput < n )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "IsInputExcite: Init error nInput < %d in file {%s}", n, (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return FALSE;
	};
	if( pdTsignal == NULL )
	{
		char szBuf[TMC_GROUT_MAXSTRING_BUF];
		sprintf( szBuf, "IsInputExcite: Init error pdTsignal == NULL in file {%s}", (LPCSTR)( csTFileName ));
		PutErrorMessage(szBuf);
		return FALSE;
	};

	int i;

	for( i = 0; i < nPoint; i++ )
	{
		if( fabs( pdTsignal[2*n+(i)*nInput*2] ) > 10*DBL_MIN ) return TRUE;
	};

	return FALSE;
}


