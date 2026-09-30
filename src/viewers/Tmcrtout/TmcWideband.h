#if !defined(AFX_TMCWIDEBAND_H__INCLUDED_)
#define AFX_TMCWIDEBAND_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcWideband.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CTmcWideband
//
//  Wideband post-processing of a .t signal file: one kernel run that was
//  excited by a multi-frequency incident wave gives the whole S-matrix
//  sweep at once.
//
//  The class is self-contained on purpose: it does not touch CTmcSMatrix,
//  whose header is shared with TMC_DN.
//
//  What it does:
//    1. reads the header written by the kernel:
//         #TMC_GROTS_Wideband 1
//         #TMC_GROTS_Port <block> <delta,m> <aperture,m> <freq,Hz> <from file>
//       no such header (an old file) -- nothing happens, the caller keeps
//       the usual single-frequency path;
//    2. tells a pulse from a continuous multi-tone by the level of the
//       incident wave at the end of the run, and picks the matching way of
//       resolving frequencies (this must not be mixed up: using the pulse
//       way on a continuous signal drags the transient into the integral
//       and costs about 0.5 dB);
//    3. finds the frequencies that actually carry energy;
//    4. removes the port mismatch, which is what makes the result equal to
//       a frequency sweep instead of merely close to it;
//    5. appends every frequency to the .s file.
//
/////////////////////////////////////////////////////////////////////////////

#include <typedef.h>

#define TMC_WIDEBAND_MAXFREQ	99		// FREQ_MAX_NUM - 1, see TMCLIMIT.H

class CTmcWideband
{
public:
	CTmcWideband();
	virtual ~CTmcWideband();

	//  TRUE if the kernel marked this .t as wideband
	static BOOL IsWidebandFile( CString csTFile );

	//  the whole job: .t in, .s out
	void Process( CString csTFile, CString csSFile, double dTmin1, double dTmax1 );

	BOOL IsError( void );
	CString & GetErrorMessage( void );
	int GetFreqNumber( void ) { return nFreq; }
	BOOL IsContinuous( void ) { return bContinuous; }

	//  service getters, used by the console check wbtest
	int GetPointNumber( void ) { return nPoint; }
	int GetInputNumber( void ) { return nInput; }
	double GetFreqValue( int k ) { return ( ( k >= 0 )&&( k < nFreq ) ) ? dFreq[k] : 0.0; }

private:
	void DeleteData( void );
	void PutErrorMessage( char *ch );

	void ReadHeader( void );
	void ReadSignals( void );
	void DetectSignalKind( void );
	void FindFrequencies( void );
	void MakeAndSave( void );

	//  projection of x(t) on exp(-j*2*pi*f*t) over [t1, t2]
	void Project( double *pdX, double dF, double dT1, double dT2,
		double *pdRe, double *pdIm );
	//  joint least-squares fit of all tones (used for a continuous signal)
	BOOL FitTones( double *pdX, double dT1, double dT2, _complex *pcOut );
	//  integral over the whole run (used for a pulse)
	void Integrate( double *pdX, double dF, double *pdRe, double *pdIm );

	double PortGamma( double dF, int nPort );
	void FixMismatch( _complex *pcCol, int nExcited, double dF );

	static BOOL SolveLinear( double *pdA, double *pdB, int n );

	CString csTFileName;
	CString csSFileName;
	CString csError;
	BOOL bIsError;

	BOOL bWideband;
	BOOL bContinuous;		// TRUE: multi-tone that never stops; FALSE: pulse

	int nPoint;
	int nInput;
	double *pdTime;
	double *pdUp;			// nPoint x nInput
	double *pdUo;			// nPoint x nInput
	double dTmin;
	double dTmax;
	double dFitT1;			// start of the window the frequency search settled on

	int nFreqNamed;			// how many frequencies the signal file named
	int nFreq;
	double dFreq[TMC_WIDEBAND_MAXFREQ];

	//  port data from the header
	int nPort;
	int nPortBlock[TMC_WIDEBAND_MAXFREQ];
	double dPortDelta[TMC_WIDEBAND_MAXFREQ];
	double dPortAperture[TMC_WIDEBAND_MAXFREQ];
	double dPortFreq[TMC_WIDEBAND_MAXFREQ];
};

/////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_TMCWIDEBAND_H__INCLUDED_)
