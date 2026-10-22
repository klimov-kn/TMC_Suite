// FieldIntegrated.h: interface for the CFieldIntegrated class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FIELDINTEGRATED_H__C0B2DF6A_836F_4331_9C70_3DB935CE0EA1__INCLUDED_)
#define AFX_FIELDINTEGRATED_H__C0B2DF6A_836F_4331_9C70_3DB935CE0EA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <TmcLibError.h>
#include <TmcRTH_IndanParam.h>


class CFieldIntegrated  
{
public:
	void ExportToDirectionalPattern( CTmcRTH_IndanParam &cParam1 );
	CString GetcsDirectPatternFile( void );
	BOOL bIsItegratedInMemory( void );
	void SetIntegrateInMemory( void );
	void Integrate( double dTCurrent1, int nTCurrent1, _real *prUNode1 );
	void SetNodeStep( int nStep );
	BOOL IsFullPath( void );
	void AddPath( CString csPath );
	CTmcLibError& GetError( void );
	CString GetErrorMessage( void );
	BOOL IsError( void );
	void InitAllFile( int nX1, int nY1, double dDelta1, double dXmin1, double dYmin1, CTmcRTH_IndanParam& cParam1, double dT1  );
	BOOL bIsFieldDistr( void );
	BOOL IsWindowOpen( void ) { return bIsBeginIntegreted; }	// A-14: okno DN nachato, no ne zakoncheno
	void SetTmin( double dTmin1 );
	double GetTmin( void );
	void SetTmax( double dTmax1 );
	double GetTmax( void );
	double GetFreq( void );
	void SetFreq( double dF );
	void SetFile(char* szFile_ft);
	void SetnXnY( int nX1, int nY1 );
	void SetnY( int nY1 );
	void SetnX( int nX1 );
	CFieldIntegrated& operator=( CFieldIntegrated& cFieldIntegr );
	void SetFile( CString csFile_ft, int nX1, int nY1 );
	void SetFile( CString csFile_ft );
	void DeleteData( void );
	CFieldIntegrated();
	virtual ~CFieldIntegrated();

private:
	void ReadAmplitudaAndFazaEzMInit( void );
	void ExportToDirectionalPattern( void );
	void AddDirectionalPatternFileOneEmitter(double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy, double &dFimin, double &dFimax);
	void ReadAmplitudaAndFazaEz(double x, double y, double &aEz, double &fEz);
	void AddDirectionalPatternFileYmax( double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy );
	void AddDirectionalPatternFileYmin( double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy );
	void AddDirectionalPatternFileXmax( double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy );
	void AddDirectionalPatternFileXmin( double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy );
	void ReadAmplitudaAndFazaY( double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy );
	void ReadAmplitudaAndFazaX( double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy );
	void AddDirectionalPatternFileYmax( void );
	void AddDirectionalPatternFileYmin( void );
	void AddDirectionalPatternFileXmax( void );
	void AddDirectionalPatternFileXmin( void );
	int GetNumberOfEmittersAlongY( void );
	int GetNumberOfEmittersAlongX( void );
	void MakeDirectionalPatternFileZagolovok( void );
	BOOL IsDirectionalPatternDataCorrect( void );
	void MakeDirectionalPatternFile( void );
	void SetDirectionalPatternParamDefault( void );
	void ReadParamFrom1File( FILE *pfDataA, int &nTCurrentA, int &nAccuracyA, int &nXA, int &nYA, double &dTCurrentA, double &dDeltaA, double &dXminA, double &dYminA, double &dLongUnitA, double &dTimeUnitA, CString &csLongUnitA, CString &csTimeUnitA );
	void ReadParamFromFile( void );
	void MakeFileAmplitudeAndFazaFromMemory( void );
	void IntegrateInMemory( _real* prUNode1, _real rCosCurrent, _real rSinCurrent);
	_real* prSin;
	_real* prCos;
	BOOL bIsIntegretedInMemory;
	void MakeFileAmplitudeAndFaza( void );
	BOOL bIsBeginIntegreted;
	void Integrate(CString csFileName, _real *prUNode1, _real r1);
	int nTCurrent;
	double dT;
	double dTCurrent;
	_real *prAmp;
	_real *prFaz;
	double dDelta;
	double dDirPatXmin;
	double dDirPatXmax;
	double dDirPatYmin;
	double dDirPatYmax;
	double dXmin;
	double dYmin;
	CTmcRTH_IndanParam cParam;
	void InitFile(CString csFileName );
	BOOL bIsFieldDistribution;
	int nEmitterInFile;
	int nY;
	int nX;

//  ---- border-only accumulation // BORDER ----------------------------
//  A directional pattern is built from a contour, not from the whole area,
//  so on a huge grid there is no reason to keep four arrays per node.
//  In border mode only four strips (3 nodes thick) around the contour are
//  kept; BorderSlot() maps a grid node to its place inside them, or -1.
public:
	BOOL IsBorderOnly( void ) { return bBorderOnly; }
	TmcIdx GetBorderSize( void ) { return nBorderSize; }
	void   SetBorderModeIfNeeded( double dFreeBytes );
	void   MakeAmpFazFromMemory( void );

//  ---- several frequencies at once // BORDER ----------------------
//  A wide-band run carries many tones; the pattern is wanted for each
//  of them. One pair of quadratures per frequency is kept, which is
//  affordable exactly because only the border strips are stored.
	void   SetFreqList( CString csList, double dFreqUnit );
	void   InitArrayForIntegrate( void );
	int    GetFreqNum( void ) { return nFreqNum; }
#define TMC_DN_MAXFREQ 99
	double dFreqList[TMC_DN_MAXFREQ];
	int    nFreqNum;
	int    nFreqCur;				// tone whose pattern is being built
	void   IntegrateInMemoryOneFreq( _real *prUNode1, _real rCosCurrent,
			_real rSinCurrent, TmcIdx iBase );
	TmcIdx BorderSlot( int ix, int iy );
private:
	void   BorderGeometry( void );
	BOOL   bBorderOnly;
	int    nBX0, nBX1, nBY0, nBY1;	// contour rectangle widened by one node
	int    nBW;						// width of the horizontal strips
	int    nBHm;					// height of the vertical strips (middle part)
	TmcIdx nBorderSize;				// how many nodes the four strips hold

	int    nNodeStep;
	CTmcLibError cError;
	double dFreq;
	double dTmin;
	double dTmax;
	CString csFileDirectPattern;
	CString csFileSin;
	CString csFileCos;
	CString csFileFaza;
	CString csFileAmplitude;
};

#endif // !defined(AFX_FIELDINTEGRATED_H__C0B2DF6A_836F_4331_9C70_3DB935CE0EA1__INCLUDED_)
