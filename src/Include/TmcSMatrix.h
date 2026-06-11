#if !defined(AFX_TMCSMATRIX_H__F6739482_DF94_11D3_A946_508A4DC10000__INCLUDED_)
#define AFX_TMCSMATRIX_H__F6739482_DF94_11D3_A946_508A4DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcSMatrix.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcSMatrix window

#include <typedef.h>

class CTmcSMatrix
{
// Construction
public:
	CTmcSMatrix();
	void DeleteData( void );

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcSMatrix)
	//}}AFX_VIRTUAL

// Implementation
public:
	void MakeSmatrix( void );
	void Save(CString csSFileName1, double dFreq1);
	void Save( CString csSFileName1 );
	CString & GetErrorMessage( void );
	void ReadTFile( CString csTFile, double dTmin1, double dTmax1 );
	BOOL IsError( void );
	virtual ~CTmcSMatrix();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcSMatrix)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
private:
	BOOL IsInputExcite( int n );
	void SetnLineInSMatrix( void );
	BOOL TtoSOneInputOnePoint( _complex *pscSFor2Point, double dTsignal1, double dTime1, double dTsignal2, double dTime2);
	void TtoSOneInput( int n );
	void TtoS( void );
	int nLineInSMatrix;
	void AllocSArray( void );
	int skip_number(char *ch);
	int skip_blank(char *ch);
	int readTsignalOne(double **pdTsign, char *ch);
	void readTsignal( double **pdTsign, char *ch );
	CString csSFileName;
	void ReadTArray( void );
	void AllocTArray( void );
	int gets_nNumbersInLine( char *ch );
	double gets_f1(char *ch);
	int CalcNInput( void );
	int CalcNPoint( void );
	void PutErrorMessage( char *ch );
	void ReadTFile( void );
	void Save( void );
	_complex * pcSmatrix;
	double * pdTsignal;
	double * pdTime;
	CString csError;
	CString csTFileName;
	double dTmin;
	double dTmax;
	double dFreq;
	BOOL bIsError;
	int nInput;
	int nPoint;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCSMATRIX_H__F6739482_DF94_11D3_A946_508A4DC10000__INCLUDED_)
