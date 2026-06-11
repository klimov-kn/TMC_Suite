// cIzl.h: interface for the cIzl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CIZL_H__1A859E7A_D5A1_48CB_A6DC_A4A0A4725C1F__INCLUDED_)
#define AFX_CIZL_H__1A859E7A_D5A1_48CB_A6DC_A4A0A4725C1F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define TMC_IZL_DIRPATTYPE_ALL 0
#define TMC_IZL_DIRPATTYPE_SIN 1
#define TMC_IZL_DIRPATTYPE_COS 2
#define TMC_IZL_DIRPATTYPE_ARB -1

class cIzl  
{
public:
	double GetdAmplituda( double dAngle );
	double dGetDirectionalPatternEmitter( double dAngle );
	BOOL bIsAngleInDiapazon( double dAngle );
	double GetdFimax( void );
	void SetdFimax( double dBuf );
	double GetdFimin( void );
	void SetdFimin( double dBuf );
	void SetcsEmitterDirectPat( char *pszBuf );
	void SetcsEmitterDirectPat( CString& csBuf );
	CString& GetcsEmitterDirectPat( void );
	cIzl& operator=( cIzl& cIzl1 );
	double GetdAmplituda( void );
	double GetdFaza( void );
	double GetdXcoordinata( void );
	double GetdYcoordinata( void );
	void SetdAmplituda( double dR );
	void SetdFaza( double dR );
	void SetdXcoordinata( double dR );
	void SetdYcoordinata( double dR );
	cIzl();
	virtual ~cIzl();

private:
	void dAnglePrived1( double& dAngle );
	void dAnglePrived( double& dAngle );
	double dFimax;
	double dFimin;
	CString csEmitterDirectPat;
	int nTypeEmitterDirectPat;
	double dAmplituda;
	double dFaza;
	double dXcoordinata;
	double dYcoordinata;
};

#endif // !defined(AFX_CIZL_H__1A859E7A_D5A1_48CB_A6DC_A4A0A4725C1F__INCLUDED_)
