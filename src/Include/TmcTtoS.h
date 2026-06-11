#if !defined(AFX_TMCTTOS_H__F6739481_DF94_11D3_A946_508A4DC10000__INCLUDED_)
#define AFX_TMCTTOS_H__F6739481_DF94_11D3_A946_508A4DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcTtoS.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcTtoS window

#include "TmcSMatrix.h"

class CTmcTtoS
{
// Construction
public:
	CTmcTtoS();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcTtoS)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SaveSmatrixToFileS( void );
	CString & GetErrorMessage( void );
	void MakeSmatrixFromFileT( void );
	BOOL IsError( void );
	void TtoS( CString csTfn, CString csSfn, double dTmin1, double dTmax1,  double dFreq1 );
	virtual ~CTmcTtoS();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcTtoS)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
private:
	CTmcSMatrix cSMatrix;
	void TtoS( void );
	void DeleteData( void );
	CString csSFileName;
	CString csTFileName;
	CString csError;
	double dTmin;
	double dTmax;
	double dFreq;
	BOOL bIsError;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCTTOS_H__F6739481_DF94_11D3_A946_508A4DC10000__INCLUDED_)
