// TMCGrExpression.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTMCGrExpression window
#ifndef _TMC_GR_EXPRESSION_1
#define _TMC_GR_EXPRESSION_1

#include <typedef.h>

class CTMCGrExpression  
{
// Construction
public:
	double GetExpressionValueK( int nPoint );
	double GetExpressionValueL( int nPoint );
	double GetExpressionValueSM( int nPoint );
	double GetExpressionValueSFR( int nPoint );
	double GetExpressionValueSFD( int nPoint );
	double GetExpressionValueSFR_User( int nPoint );
	double GetExpressionValueSFD_User( int nPoint );
	double GetExpressionValue( int nPoint );
	BOOL IsFreqCorrect( void );
	BOOL IsGraphDifferent( void );
	int * GetNPoint( int nFileName );
	_complex ** GetSMatrArray( int nFileName );
	double ** GetFreqArray( int nFileName );
	char * GetFileName( int nFileName );
	int GetNVar( void );
	char * InitData( void );
	CTMCGrExpression( char *szExpression );

// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual ~CTMCGrExpression();

	// Generated message map functions
protected:

private:
	double GetCPhase_User( int i, int nPoint );
	double GetCPhase( int i, int nPoint );
	BOOL IsError;
	int * pnPoint;
	_complex ** ppcSmatrix;
	double ** ppdFreq;
	void DeleteData( void );
	BOOL Init1Data( void );
	int CalcNGraphExpession( void );
	CString csExpresion;
	int nMaska;
	int nVar;
	char ** pszMaska;
	char ** pszVar;
};

#endif

/////////////////////////////////////////////////////////////////////////////
