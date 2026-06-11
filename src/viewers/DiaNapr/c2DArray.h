// c2DArray.h: interface for the c2DArray class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_C2DARRAY_H__15B05DDC_FBD2_4046_87E5_F234F2BA20BF__INCLUDED_)
#define AFX_C2DARRAY_H__15B05DDC_FBD2_4046_87E5_F234F2BA20BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "cIzl.h"
#include "windows.h"

#define TMC_C2DARRAY_MAX_STRING 1000

class c2DArray  
{
public:
	BOOL bIsReduceElMagnAnalysis;
	BOOL IsCalculateKip( void );
	int iN;
	double GetdMax( void );
	double GetdFreqGHz( void );
	void SaveDeviationDirectionalPattern( char *pszFileName );
	void GetIzl( int i, double *dXmm, double *dYmm, double *dAmpl, double *dFazaDegree);
	void SaveData( char *pszFile );
	void MakeDeviation( char *szSourceFile, char *szDistinationFile, double dXmm, double dYmm, double dAmplProcent, double dFazaDegree );
	void SetNPointForNormirovka( int nPoint );
	void SetdFrequence( double dFrequenceGHz );
	void SetszOneExpression( char *ch );
	double GetOptimPurposeFunction( void );
	void SetIzl( int i, double dXcoord_mm, double dYcoord_mm, double dAmplitud, double dFaza_gradus );
	void OptimizationInit( char *szFileRequiDirPat, int nIzl1 );
	double GetImageIzl( int i );
	double GetRealIzl( int i );
	void ReadData( char *szFileName1 );
	char * GetOneExpression( void );
	void SetOneExpression( char *ch );
	int GetnIzl( void );
	double GetKnd_dB( void );
	double GetKnd( void );
	double GetKip_dB( void );
	double GetKip( void );
	void ResizeAngle( double dAngleMin, double dAngleMax );
	double GetdAmplitudeDiagrNapr( int i );
	double GetdAngle( int i );
	void CalculateDiagrNapr( char *szInputFileName, char *szOutputFileName );
	BOOL IsError( void );
	int  GetnAngle( void );
	char  *GetError( void );
	void DeleteData( void );
	c2DArray();
	virtual ~c2DArray();

private:
	FILETIME LastWriteTime( char *szFileName );
	void ReadDataFromTempFile( void );
	BOOL IsDirectPatSaveInFile( void );
	void WriteTemporaryFile( void );
	BOOL bIsInitialData;
	double GetRnd(double dMedium, double dDelta);
	int nPointForNormir;
	char szOneExpression[TMC_C2DARRAY_MAX_STRING];
	double dKnd;
	double dKip;
	BOOL IsNormirovka;
	void NormirovkaDiagrNapr( void );
	double CalculateImageDN( double dAngle );
	double CalculateRealDN( double dAngle );
	double CalculateImageDN_ForKip( double dAngle );
	double CalculateRealDN_ForKip( double dAngle );
	void DeleteAngle( void );
	void InitAngle( void );
	void CalculateDiagrNapr( void );
	void WriteData( char *szFileName1 );
	void InitData( void );
	void WriteData( void );
	void ReadData( void );
	char szFileNameTemp[TMC_C2DARRAY_MAX_STRING];
	char szFileName[TMC_C2DARRAY_MAX_STRING];
	char szFileNameOut[TMC_C2DARRAY_MAX_STRING];
	char szError[TMC_C2DARRAY_MAX_STRING];
	double dMax;
	int nMaxTemporary;
	int nAngle;
	double *pdAngle;
	double *pdRealDN;
	double *pdRequiredDN;
	double *pdImageDN;
	double dFreq;
	BOOL bIsError;
	int    nIzl;
	cIzl *pcIzl;
};

#endif // !defined(AFX_C2DARRAY_H__15B05DDC_FBD2_4046_87E5_F234F2BA20BF__INCLUDED_)
