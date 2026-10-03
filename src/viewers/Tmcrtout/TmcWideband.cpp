// TmcWideband.cpp : implementation file
//
//  Wideband post-processing: builds the whole S-matrix sweep out of one
//  kernel run. See TmcWideband.h for what and why.
//

#include "stdafx.h"
#include <math.h>
#include <float.h>
#include <Tmcgrviw.h>
#include <proc_s.h>
#include <s_file.h>
#include <TmcWideband.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TMC_WB_BUF			1024
#define TMC_WB_PI			3.14159265358979323846
#define TMC_WB_C0			299792458.0
#define TMC_WB_SCANPOINTS	2048		// grid for looking where the energy is
#define TMC_WB_PEAKDB		20.0		// keep lines down to this many dB from the top

/////////////////////////////////////////////////////////////////////////////
CTmcWideband::CTmcWideband()
{
	pdTime = NULL;
	pdUp = NULL;
	pdUo = NULL;
	DeleteData();
}

CTmcWideband::~CTmcWideband()
{
	DeleteData();
}

void CTmcWideband::DeleteData()
{
	if( pdTime != NULL ) { delete []pdTime; pdTime = NULL; };
	if( pdUp != NULL ) { delete []pdUp; pdUp = NULL; };
	if( pdUo != NULL ) { delete []pdUo; pdUo = NULL; };
	csTFileName.Format("");
	csSFileName.Format("");
	csError.Format("No error");
	bIsError = FALSE;
	bWideband = FALSE;
	bContinuous = FALSE;
	nPoint = 0;
	nInput = 0;
	nFreq = 0;
	nPort = 0;
	dFitT1 = 0.0;
	dTmin = 0.0;
	dTmax = 0.0;
	return;
}

BOOL CTmcWideband::IsError()
{
	return bIsError;
}

CString & CTmcWideband::GetErrorMessage()
{
	return csError;
}

void CTmcWideband::PutErrorMessage(char *ch)
{
	bIsError = TRUE;
	csError.Format("%s", ch);
	return;
}

/////////////////////////////////////////////////////////////////////////////
//  Does the kernel say this run was wideband?
BOOL CTmcWideband::IsWidebandFile( CString csTFile )
{
	FILE *fp;
	char szLine[TMC_WB_BUF];
	BOOL bRes = FALSE;

	if( ( fp = fopen( (const char *)csTFile, "r" ) ) == NULL ) return FALSE;

	while( fgets( szLine, TMC_WB_BUF, fp ) != NULL )
	{
		if( szLine[0] == ' ' ) break;			// data started, header is over
		if( strncmp( szLine, TMC_GROTS_FLOUTWideband_ID,
			strlen(TMC_GROTS_FLOUTWideband_ID) ) == 0 )
		{
			int n = 0;
			sscanf( szLine + strlen(TMC_GROTS_FLOUTWideband_ID), "%d", &n );
			bRes = ( n != 0 );
			break;
		};
	};
	fclose( fp );
	return bRes;
}

void CTmcWideband::ReadHeader()
{
	FILE *fp;
	char szLine[TMC_WB_BUF];

	if( IsError() ) return;

	if( ( fp = fopen( (const char *)csTFileName, "r" ) ) == NULL )
	{
		PutErrorMessage("Wideband: can not open signal file");
		return;
	};

	nPort = 0;
	while( fgets( szLine, TMC_WB_BUF, fp ) != NULL )
	{
		if( szLine[0] == ' ' ) break;
		if( strncmp( szLine, TMC_GROTS_FLOUTWideband_ID,
			strlen(TMC_GROTS_FLOUTWideband_ID) ) == 0 )
		{
			int n = 0;
			sscanf( szLine + strlen(TMC_GROTS_FLOUTWideband_ID), "%d", &n );
			bWideband = ( n != 0 );
		};
		if( strncmp( szLine, TMC_GROTS_FLOUTSigFreq_ID,
			strlen(TMC_GROTS_FLOUTSigFreq_ID) ) == 0 )
		{
			//  The signal file named its frequencies and the kernel passed
			//  them on. Take them as they are: for a multi-tone this is the
			//  only exact way -- neighbouring tones shift each other peaks,
			//  so looking for maxima in the spectrum misses by megahertz.
			char *pS = szLine + strlen(TMC_GROTS_FLOUTSigFreq_ID);
			double dV;
			int nCh;
			nFreqNamed = 0;
			for( ; ( nFreqNamed < TMC_WIDEBAND_MAXFREQ )&&
				( sscanf( pS, "%lg%n", &dV, &nCh ) == 1 ) ; pS += nCh )
			{
				if( dV > 0.0 )
				{
					dFreq[nFreqNamed] = dV;
					nFreqNamed++;
				};
			};
		};
		if( strncmp( szLine, TMC_GROTS_FLOUTPort_ID,
			strlen(TMC_GROTS_FLOUTPort_ID) ) == 0 )
		{
			int nBl, nFromFile;
			double dD, dA, dF;
			if( ( nPort < TMC_WIDEBAND_MAXFREQ )&&
				( sscanf( szLine + strlen(TMC_GROTS_FLOUTPort_ID), "%d %lg %lg %lg %d",
					&nBl, &dD, &dA, &dF, &nFromFile ) == 5 ) )
			{
				nPortBlock[nPort] = nBl;
				dPortDelta[nPort] = dD;
				dPortAperture[nPort] = dA;
				dPortFreq[nPort] = dF;
				nPort++;
			};
		};
	};
	fclose( fp );

	if( !bWideband ) PutErrorMessage("Wideband: file is not marked as wideband");
	return;
}

/////////////////////////////////////////////////////////////////////////////
//  Reads the data lines. Layout of one line:
//     nT  t  ( block  Up  Uo ) repeated for every port
void CTmcWideband::ReadSignals()
{
	FILE *fp;
	char szLine[TMC_WB_BUF];
	int i, j, n;

	if( IsError() ) return;

	if( ( fp = fopen( (const char *)csTFileName, "r" ) ) == NULL )
	{
		PutErrorMessage("Wideband: can not open signal file");
		return;
	};

	//  first pass: how many lines and how many ports
	nPoint = 0;
	nInput = 0;
	while( fgets( szLine, TMC_WB_BUF, fp ) != NULL )
	{
		if( szLine[0] != ' ' ) continue;
		if( nInput == 0 )
		{
			//  count the numbers in the line: 2 + 3 per port
			char *p = szLine;
			n = 0;
			for( ; *p != '\0' ; )
			{
				for( ; ( *p == ' ' )||( *p == '\t' )||( *p == '\n' ) ; p++ );
				if( *p == '\0' ) break;
				n++;
				for( ; ( *p != ' ' )&&( *p != '\t' )&&( *p != '\n' )&&( *p != '\0' ) ; p++ );
			};
			if( ( n < 5 )||( ( n - 2 )%3 != 0 ) ) continue;
			nInput = ( n - 2 )/3;
		};
		nPoint++;
	};

	if( ( nPoint < 8 )||( nInput < 1 ) )
	{
		PutErrorMessage("Wideband: not enough data in the signal file");
		fclose( fp );
		return;
	};

	pdTime = new double[nPoint];
	pdUp = new double[nPoint*nInput];
	pdUo = new double[nPoint*nInput];
	if( ( pdTime == NULL )||( pdUp == NULL )||( pdUo == NULL ) )
	{
		PutErrorMessage("Wideband: not enough memory for the signals");
		fclose( fp );
		return;
	};

	//  second pass: the values
	rewind( fp );
	i = 0;
	while( ( fgets( szLine, TMC_WB_BUF, fp ) != NULL )&&( i < nPoint ) )
	{
		char *p = szLine;
		double dNT, dT, dBlock, dUp, dUo;

		if( szLine[0] != ' ' ) continue;
		if( sscanf( p, "%lg %lg", &dNT, &dT ) != 2 ) continue;

		//  skip the two leading numbers
		for( n = 0; n < 2; n++ )
		{
			for( ; ( *p == ' ' )||( *p == '\t' ) ; p++ );
			for( ; ( *p != ' ' )&&( *p != '\t' )&&( *p != '\0' ) ; p++ );
		};

		pdTime[i] = dT;
		for( j = 0; j < nInput; j++ )
		{
			if( sscanf( p, "%lg %lg %lg", &dBlock, &dUp, &dUo ) != 3 )
			{
				dUp = 0.0;
				dUo = 0.0;
			};
			pdUp[i*nInput + j] = dUp;
			pdUo[i*nInput + j] = dUo;
			for( n = 0; n < 3; n++ )
			{
				for( ; ( *p == ' ' )||( *p == '\t' ) ; p++ );
				for( ; ( *p != ' ' )&&( *p != '\t' )&&( *p != '\0' ) ; p++ );
			};
		};
		i++;
	};
	fclose( fp );
	nPoint = i;
	return;
}

/////////////////////////////////////////////////////////////////////////////
//  A pulse has died away by the end of the run, a multi-tone has not.
//  The two need different ways of resolving frequencies, and mixing them
//  up is a real error, not a nicety.
void CTmcWideband::DetectSignalKind()
{
	int i, j;
	double dMax = 0.0, dTail = 0.0, dR;

	if( IsError() ) return;

	for( i = 0; i < nPoint; i++ )
	{
		for( j = 0; j < nInput; j++ )
		{
			dR = fabs( pdUp[i*nInput + j] );
			if( dR > dMax ) dMax = dR;
			if( i > ( nPoint*3 )/4 )
			{
				if( dR > dTail ) dTail = dR;
			};
		};
	};

	//  tail still above one percent of the peak -- the source is still running
	bContinuous = ( dMax > 0.0 )&&( dTail > 0.01*dMax );
	return;
}

/////////////////////////////////////////////////////////////////////////////
void CTmcWideband::Project( double *pdX, double dF, double dT1, double dT2,
	double *pdRe, double *pdIm )
{
	int i;
	double dRe = 0.0, dIm = 0.0, dW, dT, dDt, dSpan = 0.0;

	dW = 2.0*TMC_WB_PI*dF;
	for( i = 1; i < nPoint; i++ )
	{
		dT = pdTime[i];
		if( ( dT < dT1 )||( dT > dT2 ) ) continue;
		dDt = pdTime[i] - pdTime[i-1];
		dRe += pdX[i]*cos( dW*dT )*dDt;
		dIm -= pdX[i]*sin( dW*dT )*dDt;
		dSpan += dDt;
	};
	if( dSpan <= 0.0 ) { *pdRe = 0.0; *pdIm = 0.0; return; };
	*pdRe = 2.0*dRe/dSpan;
	*pdIm = 2.0*dIm/dSpan;
	return;
}

void CTmcWideband::Integrate( double *pdX, double dF, double *pdRe, double *pdIm )
{
	int i;
	double dRe = 0.0, dIm = 0.0, dW, dT, dDt;

	dW = 2.0*TMC_WB_PI*dF;
	for( i = 1; i < nPoint; i++ )
	{
		dT = pdTime[i];
		dDt = pdTime[i] - pdTime[i-1];
		dRe += pdX[i]*cos( dW*dT )*dDt;
		dIm -= pdX[i]*sin( dW*dT )*dDt;
	};
	*pdRe = dRe;
	*pdIm = dIm;
	return;
}

/////////////////////////////////////////////////////////////////////////////
//  Gauss with partial pivoting; A is n x n row-major, B is the right side.
BOOL CTmcWideband::SolveLinear( double *pdA, double *pdB, int n )
{
	int i, j, k, nBest;
	double dBest, dR;

	for( k = 0; k < n; k++ )
	{
		nBest = k;
		dBest = fabs( pdA[k*n + k] );
		for( i = k + 1; i < n; i++ )
		{
			if( fabs( pdA[i*n + k] ) > dBest )
			{
				dBest = fabs( pdA[i*n + k] );
				nBest = i;
			};
		};
		if( dBest < 1e-300 ) return FALSE;
		if( nBest != k )
		{
			for( j = 0; j < n; j++ )
			{
				dR = pdA[k*n + j]; pdA[k*n + j] = pdA[nBest*n + j]; pdA[nBest*n + j] = dR;
			};
			dR = pdB[k]; pdB[k] = pdB[nBest]; pdB[nBest] = dR;
		};
		for( i = k + 1; i < n; i++ )
		{
			dR = pdA[i*n + k]/pdA[k*n + k];
			if( dR == 0.0 ) continue;
			for( j = k; j < n; j++ ) pdA[i*n + j] -= dR*pdA[k*n + j];
			pdB[i] -= dR*pdB[k];
		};
	};
	for( i = n - 1; i >= 0; i-- )
	{
		dR = pdB[i];
		for( j = i + 1; j < n; j++ ) dR -= pdA[i*n + j]*pdB[j];
		pdB[i] = dR/pdA[i*n + i];
	};
	return TRUE;
}

//  Joint fit of all tones at once. Needed for a continuous multi-tone:
//  the tones are not orthogonal on an arbitrary window, so fitting them
//  one by one would leak energy between neighbours.
BOOL CTmcWideband::FitTones( double *pdX, double dT1, double dT2, _complex *pcOut )
{
	int i, j, k, m, n2;
	double *pdA, *pdB, *pdBas;
	double dT, dW;
	BOOL bRes;

	m = nFreq;
	n2 = 2*m;
	pdA = new double[n2*n2];
	pdB = new double[n2];
	pdBas = new double[n2];
	if( ( pdA == NULL )||( pdB == NULL )||( pdBas == NULL ) )
	{
		if( pdA != NULL ) delete []pdA;
		if( pdB != NULL ) delete []pdB;
		if( pdBas != NULL ) delete []pdBas;
		return FALSE;
	};

	for( i = 0; i < n2*n2; i++ ) pdA[i] = 0.0;
	for( i = 0; i < n2; i++ ) pdB[i] = 0.0;

	for( i = 0; i < nPoint; i++ )
	{
		dT = pdTime[i];
		if( ( dT < dT1 )||( dT > dT2 ) ) continue;
		for( k = 0; k < m; k++ )
		{
			dW = 2.0*TMC_WB_PI*dFreq[k]*dT;
			pdBas[2*k] = cos( dW );
			pdBas[2*k + 1] = -sin( dW );
		};
		for( j = 0; j < n2; j++ )
		{
			pdB[j] += pdBas[j]*pdX[i];
			for( k = j; k < n2; k++ ) pdA[j*n2 + k] += pdBas[j]*pdBas[k];
		};
	};
	for( j = 0; j < n2; j++ )
		for( k = 0; k < j; k++ ) pdA[j*n2 + k] = pdA[k*n2 + j];

	bRes = SolveLinear( pdA, pdB, n2 );
	if( bRes )
	{
		for( k = 0; k < m; k++ )
		{
			pcOut[k].x = pdB[2*k];
			pcOut[k].y = pdB[2*k + 1];
		};
	};

	delete []pdA;
	delete []pdB;
	delete []pdBas;
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////
//  Where does the incident wave actually carry energy? Asking for S at a
//  frequency the signal does not contain would be dividing by noise.
void CTmcWideband::FindFrequencies()
{
	int i, k, nExc = -1;
	double dFs, dFmax, dStep, dRe, dIm;
	double *pdLev, dTop = 0.0;
	double dT1, dT2;

	if( IsError() ) return;

	if( nFreqNamed > 0 )
	{
		//  the frequencies came with the file; nothing to look for
		nFreq = nFreqNamed;
		if( bContinuous )
		{
			//  window: a whole number of periods of the tone spacing, so the
			//  tones stay orthogonal and do not leak into each other
			double dT2n = pdTime[nPoint-1];
			double dT1n = dT2n - 0.5*( dT2n - pdTime[0] );
			if( nFreq >= 2 )
			{
				double dSp = ( dFreq[nFreq-1] - dFreq[0] )/( nFreq - 1 );
				if( dSp > 0.0 )
				{
					double dW = floor( ( dT2n - dT1n )*dSp + 0.5 )/dSp;
					if( ( dW > 0.0 )&&( dW <= ( dT2n - pdTime[0] ) ) ) dT1n = dT2n - dW;
				};
			};
			dFitT1 = dT1n;
		};
		return;
	};

	//  the excited port is the one with a non-zero incident wave
	for( k = 0; k < nInput; k++ )
	{
		for( i = 0; i < nPoint; i++ )
		{
			if( fabs( pdUp[i*nInput + k] ) > 1e-12 ) { nExc = k; break; };
		};
		if( nExc >= 0 ) break;
	};
	if( nExc < 0 )
	{
		PutErrorMessage("Wideband: no excited port in the signal file");
		return;
	};

	if( pdTime[nPoint-1] <= pdTime[0] )
	{
		PutErrorMessage("Wideband: bad time column in the signal file");
		return;
	};
	dFs = ( nPoint - 1 )/( pdTime[nPoint-1] - pdTime[0] );	// sampling rate
	dFmax = 0.25*dFs;										// well below Nyquist
	//  Scanning the whole band up to Nyquist would make the grid far too
	//  coarse (tens of MHz), and the found lines would be off. The useful
	//  band always sits around the frequency the port was matched at, so
	//  the scan is narrowed to it -- the grid gets an order finer.
	if( ( nPort > 0 )&&( dPortFreq[0] > 0.0 ) )
	{
		double dTop = 3.0*dPortFreq[0];
		if( dTop < dFmax ) dFmax = dTop;
	};

	pdLev = new double[TMC_WB_SCANPOINTS];
	if( pdLev == NULL )
	{
		PutErrorMessage("Wideband: not enough memory for the spectrum scan");
		return;
	};

	dT1 = pdTime[0];
	dT2 = pdTime[nPoint-1];
	if( bContinuous ) dT1 = dT2 - 0.5*( dT2 - pdTime[0] );	// settled part only
	dFitT1 = dT1;

	dStep = dFmax/TMC_WB_SCANPOINTS;
	double *pdX = pdUp + nExc;
	//  the incident wave of the excited port, taken with a stride
	double *pdCol = new double[nPoint];
	if( pdCol == NULL )
	{
		delete []pdLev;
		PutErrorMessage("Wideband: not enough memory for the spectrum scan");
		return;
	};
	for( i = 0; i < nPoint; i++ ) pdCol[i] = pdUp[i*nInput + nExc];

	for( k = 0; k < TMC_WB_SCANPOINTS; k++ )
	{
		double dF = ( k + 1 )*dStep;
		if( bContinuous ) Project( pdCol, dF, dT1, dT2, &dRe, &dIm );
		else Integrate( pdCol, dF, &dRe, &dIm );
		pdLev[k] = sqrt( dRe*dRe + dIm*dIm );
		if( pdLev[k] > dTop ) dTop = pdLev[k];
	};

	nFreq = 0;
	if( dTop > 0.0 )
	{
		double dCut = dTop*pow( 10.0, -TMC_WB_PEAKDB/20.0 );
		//  a multi-tone shows sharp peaks; a pulse fills a band. Peaks first.
		//  Two lines closer than 1/T are not distinguishable at all, so a
		//  bump narrower than that is ripple, not a tone. Look for a maximum
		//  over a window of that width instead of comparing to neighbours.
		int nWin = ( int )( 1.0/( ( dT2 - dT1 )*dStep ) + 0.5 );
		if( nWin < 3 ) nWin = 3;
		for( k = 1; ( k < TMC_WB_SCANPOINTS - 1 )&&( nFreq < TMC_WIDEBAND_MAXFREQ ) ; k++ )
		{
			int nA = k - nWin, nB = k + nWin, q;
			BOOL bTop = TRUE;
			if( nA < 0 ) nA = 0;
			if( nB > TMC_WB_SCANPOINTS - 1 ) nB = TMC_WB_SCANPOINTS - 1;
			for( q = nA; q <= nB; q++ )
			{
				if( q == k ) continue;
				if( pdLev[q] > pdLev[k] ) { bTop = FALSE; break; };
			};
			if( bTop&&( pdLev[k] > dCut )&&( pdLev[k] >= pdLev[k-1] )&&( pdLev[k] > pdLev[k+1] ) )
			{
				//  The peak almost never falls exactly on a grid point, and
				//  taking the grid point itself would put S at a slightly
				//  wrong frequency. A parabola through the three samples
				//  gives the true position.
				double dA1 = pdLev[k-1], dA2 = pdLev[k], dA3 = pdLev[k+1];
				double dDen = dA1 - 2.0*dA2 + dA3;
				double dShift = 0.0;
				if( fabs( dDen ) > 1e-300 ) dShift = 0.5*( dA1 - dA3 )/dDen;
				if( ( dShift < -1.0 )||( dShift > 1.0 ) ) dShift = 0.0;
				dFreq[nFreq] = ( k + 1 + dShift )*dStep;
				nFreq++;
			};
		};
		//  a pulse gives one broad hump: cover the band evenly instead
		if( nFreq <= 2 )
		{
			int kLo = -1, kHi = -1;
			for( k = 0; k < TMC_WB_SCANPOINTS; k++ )
			{
				if( pdLev[k] > dCut )
				{
					if( kLo < 0 ) kLo = k;
					kHi = k;
				};
			};
			nFreq = 0;
			if( ( kLo >= 0 )&&( kHi > kLo ) )
			{
				int nWant = 21;
				for( k = 0; k < nWant; k++ )
				{
					dFreq[nFreq] = ( kLo + 1 + ( double )( kHi - kLo )*k/( nWant - 1 ) )*dStep;
					nFreq++;
				};
			};
		};
	};

	//  A continuous multi-tone leaks between neighbouring lines unless the
	//  window holds a whole number of periods of their spacing. The leakage
	//  pulls the peaks apart -- the found grid comes out stretched by about
	//  a percent, which is enough to spoil the joint fit. So: estimate the
	//  spacing from the rough peaks, trim the window to a whole number of
	//  its periods, and look again.
	if( bContinuous&&( nFreq >= 2 ) )
	{
		int q;
		double dSum = 0.0, dSpacing, dSpan, dWhole;

		for( q = 1; q < nFreq; q++ ) dSum += dFreq[q] - dFreq[q-1];
		dSpacing = dSum/( nFreq - 1 );

		if( dSpacing > 0.0 )
		{
			dSpan = dT2 - dT1;
			dWhole = floor( dSpan*dSpacing + 0.5 )/dSpacing;
			if( ( dWhole > 0.0 )&&( dWhole <= dSpan*1.0001 ) )
			{
				dT1 = dT2 - dWhole;
				dFitT1 = dT1;
				nFreq = 0;
				for( k = 0; k < TMC_WB_SCANPOINTS; k++ )
				{
					double dF = ( k + 1 )*dStep;
					Project( pdCol, dF, dT1, dT2, &dRe, &dIm );
					pdLev[k] = sqrt( dRe*dRe + dIm*dIm );
				};
				dTop = 0.0;
				for( k = 0; k < TMC_WB_SCANPOINTS; k++ )
					if( pdLev[k] > dTop ) dTop = pdLev[k];
				{
					double dCut2 = dTop*pow( 10.0, -TMC_WB_PEAKDB/20.0 );
					int nWin2 = ( int )( 1.0/( ( dT2 - dT1 )*dStep ) + 0.5 );
					if( nWin2 < 3 ) nWin2 = 3;
					for( k = 1; ( k < TMC_WB_SCANPOINTS - 1 )&&
						( nFreq < TMC_WIDEBAND_MAXFREQ ) ; k++ )
					{
						int nA = k - nWin2, nB = k + nWin2, q2;
						BOOL bTop2 = TRUE;
						if( nA < 0 ) nA = 0;
						if( nB > TMC_WB_SCANPOINTS - 1 ) nB = TMC_WB_SCANPOINTS - 1;
						for( q2 = nA; q2 <= nB; q2++ )
						{
							if( q2 == k ) continue;
							if( pdLev[q2] > pdLev[k] ) { bTop2 = FALSE; break; };
						};
						if( bTop2&&( pdLev[k] > dCut2 )&&( pdLev[k] >= pdLev[k-1] )&&
							( pdLev[k] > pdLev[k+1] ) )
						{
							double dA1 = pdLev[k-1], dA2 = pdLev[k], dA3 = pdLev[k+1];
							double dDen2 = dA1 - 2.0*dA2 + dA3;
							double dSh = 0.0;
							if( fabs( dDen2 ) > 1e-300 ) dSh = 0.5*( dA1 - dA3 )/dDen2;
							if( ( dSh < -1.0 )||( dSh > 1.0 ) ) dSh = 0.0;
							dFreq[nFreq] = ( k + 1 + dSh )*dStep;
							nFreq++;
						};
					};
				};
			};
		};
	};

	delete []pdCol;
	delete []pdLev;

	if( nFreq < 1 ) PutErrorMessage("Wideband: no frequency carries energy in this file");
	return;
}

/////////////////////////////////////////////////////////////////////////////
//  Port mismatch. The kernel sets the port admittance once, from FREQ, so
//  at every other frequency the port itself reflects. The same formula as
//  in CTmcRTHNodeDiel::SetInputAdmitance.
double CTmcWideband::PortGamma( double dF, int nPort1 )
{
	double dA, dY1, dY0, dDelta, dAper, dF0;

	if( ( nPort1 < 0 )||( nPort1 >= nPort ) ) return 0.0;
	dDelta = dPortDelta[nPort1];
	dAper = dPortAperture[nPort1];
	dF0 = dPortFreq[nPort1];
	if( ( dDelta <= 0.0 )||( dAper <= 0.0 )||( dF0 <= 0.0 ) ) return 0.0;

	dA = sqrt( 2.0 )*TMC_WB_PI*dDelta*dF/TMC_WB_C0;
	if( fabs( sin( dA ) ) < 1e-30 ) return 0.0;
	dY1 = sin( acos( 2.0*cos( dA ) - cos( TMC_WB_PI*dDelta/dAper ) ) )/sin( dA );

	dA = sqrt( 2.0 )*TMC_WB_PI*dDelta*dF0/TMC_WB_C0;
	if( fabs( sin( dA ) ) < 1e-30 ) return 0.0;
	dY0 = sin( acos( 2.0*cos( dA ) - cos( TMC_WB_PI*dDelta/dAper ) ) )/sin( dA );

	if( dY1 + dY0 == 0.0 ) return 0.0;
	return ( dY1 - dY0 )/( dY1 + dY0 );
}

//  Removing the port mismatch.
//
//  One run gives one column of the matrix, and BOTH ports are mismatched,
//  so fixing the reflection of the excited one is not enough. For a
//  SYMMETRIC two-port (S22 = S11, S12 = S21) the task splits into two
//  independent one-port ones through the even and odd modes:
//
//      Ge = S11 + S21,   Go = S11 - S21
//      G' = ( G + Gamma )/( 1 + Gamma*G )      for each mode
//      S11 = ( Ge' + Go' )/2,   S21 = ( Ge' - Go' )/2
//
//  This is the form that was checked against a frequency sweep and matched
//  it to 0.002 dB. Scaling the transmitted wave instead does NOT work.
//
//  A structure that is not symmetric needs the full matrix, that is both
//  excitations, and the correction then applies to the assembled matrix.
//  With a single column the symmetric case is what can be done honestly;
//  for anything else only the reflection of the excited port is fixed.
static void TmcWbBilinear( double dRe, double dIm, double dG,
	double *pdOutRe, double *pdOutIm )
{
	//  ( m + G )/( 1 + G*m ), G real
	double dNumRe = dRe + dG;
	double dNumIm = dIm;
	double dDenRe = 1.0 + dG*dRe;
	double dDenIm = dG*dIm;
	double dDen = dDenRe*dDenRe + dDenIm*dDenIm;

	if( dDen < 1e-300 ) { *pdOutRe = dRe; *pdOutIm = dIm; return; };
	*pdOutRe = ( dNumRe*dDenRe + dNumIm*dDenIm )/dDen;
	*pdOutIm = ( dNumIm*dDenRe - dNumRe*dDenIm )/dDen;
	return;
}

void CTmcWideband::FixMismatch( _complex *pcCol, int nExcited, double dF )
{
	double dG = PortGamma( dF, nExcited );
	if( dG == 0.0 ) return;

	if( nInput != 2 )
	{
		double dR, dI;
		TmcWbBilinear( pcCol[nExcited].x, pcCol[nExcited].y, dG, &dR, &dI );
		pcCol[nExcited].x = ( _real )dR;
		pcCol[nExcited].y = ( _real )dI;
		return;
	};

	{
		int nOther = 1 - nExcited;
		double dSrR = pcCol[nExcited].x, dSrI = pcCol[nExcited].y;
		double dStR = pcCol[nOther].x, dStI = pcCol[nOther].y;
		double dGeR, dGeI, dGoR, dGoI;

		TmcWbBilinear( dSrR + dStR, dSrI + dStI, dG, &dGeR, &dGeI );
		TmcWbBilinear( dSrR - dStR, dSrI - dStI, dG, &dGoR, &dGoI );

		pcCol[nExcited].x = ( _real )( 0.5*( dGeR + dGoR ) );
		pcCol[nExcited].y = ( _real )( 0.5*( dGeI + dGoI ) );
		pcCol[nOther].x = ( _real )( 0.5*( dGeR - dGoR ) );
		pcCol[nOther].y = ( _real )( 0.5*( dGeI - dGoI ) );
	};
	return;
}

/////////////////////////////////////////////////////////////////////////////
void CTmcWideband::MakeAndSave()
{
	int i, k, nExc = -1;
	_complex *pcCol, *pcInc, *pcRef;
	double dT1, dT2;
	char szBuf[TMC_WB_BUF];

	if( IsError() ) return;

	for( k = 0; k < nInput; k++ )
	{
		for( i = 0; i < nPoint; i++ )
			if( fabs( pdUp[i*nInput + k] ) > 1e-12 ) { nExc = k; break; };
		if( nExc >= 0 ) break;
	};
	if( nExc < 0 ) { PutErrorMessage("Wideband: no excited port"); return; };

	pcCol = new _complex[nInput];
	pcInc = new _complex[nFreq];
	pcRef = new _complex[nFreq];
	double *pdCol = new double[nPoint];
	if( ( pcCol == NULL )||( pcInc == NULL )||( pcRef == NULL )||( pdCol == NULL ) )
	{
		if( pcCol != NULL ) delete []pcCol;
		if( pcInc != NULL ) delete []pcInc;
		if( pcRef != NULL ) delete []pcRef;
		if( pdCol != NULL ) delete []pdCol;
		PutErrorMessage("Wideband: not enough memory");
		return;
	};

	dT2 = pdTime[nPoint-1];
	//  the same window that the frequency search settled on
	dT1 = ( bContinuous ) ? dFitT1 : pdTime[0];

	//  incident wave of the excited port, once for all frequencies
	for( i = 0; i < nPoint; i++ ) pdCol[i] = pdUp[i*nInput + nExc];
	if( bContinuous )
	{
		if( !FitTones( pdCol, dT1, dT2, pcInc ) )
		{
			PutErrorMessage("Wideband: can not resolve the tones of the incident wave");
			delete []pcCol; delete []pcInc; delete []pcRef; delete []pdCol;
			return;
		};
	}
	else
	{
		for( k = 0; k < nFreq; k++ )
		{
			double dRe, dIm;
			Integrate( pdCol, dFreq[k], &dRe, &dIm );
			pcInc[k].x = ( _real )dRe;
			pcInc[k].y = ( _real )dIm;
		};
	};

	//  one frequency at a time: build the column and append it to the .s
	for( k = 0; k < nFreq; k++ )
	{
		for( i = 0; i < nInput; i++ ) { pcCol[i].x = 0.0; pcCol[i].y = 0.0; };

		for( i = 0; i < nInput; i++ )
		{
			int j;
			double dRe, dIm, dDen;
			for( j = 0; j < nPoint; j++ ) pdCol[j] = pdUo[j*nInput + i];
			if( bContinuous )
			{
				if( !FitTones( pdCol, dT1, dT2, pcRef ) ) continue;
				dRe = pcRef[k].x;
				dIm = pcRef[k].y;
			}
			else
			{
				Integrate( pdCol, dFreq[k], &dRe, &dIm );
			};
			//  divide by the incident wave
			dDen = pcInc[k].x*pcInc[k].x + pcInc[k].y*pcInc[k].y;
			if( dDen < 1e-300 ) continue;
			pcCol[i].x = ( _real )( ( dRe*pcInc[k].x + dIm*pcInc[k].y )/dDen );
			pcCol[i].y = ( _real )( ( dIm*pcInc[k].x - dRe*pcInc[k].y )/dDen );
		};

FixMismatch( pcCol, nExc, dFreq[k] );

		strcpy( szBuf, ( const char * )csSFileName );
		if( save_S_matrix_element( nExc + 1, szBuf, pcCol, dFreq[k], nInput ) != 0 )
		{
			PutErrorMessage("Wideband: can not write the S-matrix file");
			break;
		};
	};

	delete []pcCol;
	delete []pcInc;
	delete []pcRef;
	delete []pdCol;
	return;
}

/////////////////////////////////////////////////////////////////////////////
void CTmcWideband::Process( CString csTFile, CString csSFile,
	double dTmin1, double dTmax1 )
{
	DeleteData();
	csTFileName.Format("%s", csTFile);
	csSFileName.Format("%s", csSFile);
	dTmin = dTmin1;
	dTmax = dTmax1;

	ReadHeader();
	ReadSignals();
	DetectSignalKind();
	FindFrequencies();
	MakeAndSave();
	return;
}
