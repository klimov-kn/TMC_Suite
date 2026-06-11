#if !defined(AFX_TMCTOPSIZE_H__20C16661_C436_11D3_A940_508A4DC10000__INCLUDED_)
#define AFX_TMCTOPSIZE_H__20C16661_C436_11D3_A940_508A4DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcTopSize.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcTopSize window

class CTmcTopSize : public CWnd
{
// Construction
public:
	CTmcTopSize();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcTopSize)
	//}}AFX_VIRTUAL

// Implementation
public:
	CTmcTopSize& operator=(CTmcTopSize& cTopSize);
	void TranslateX0Y0( double dX0, double dY0 );
	void DeleteData( void );
	double GetdX1( void );
	double GetdX2( void );
	double GetdY1( void );
	double GetdY2( void );
	double GetdSize( void );
	CString GetcsSize( void );
	void SetcsSize( CString &csr );
	void SetdSize( double r );
	void SetdX1( double r );
	void SetdX2( double r );
	void SetdY1( double r );
	void SetdY2( double r );
	virtual ~CTmcTopSize();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcTopSize)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	double  dX1;
	double  dX2;
	double  dY1;
	double  dY2;
	double  dSize;
	CString csSize;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCTOPSIZE_H__20C16661_C436_11D3_A940_508A4DC10000__INCLUDED_)
