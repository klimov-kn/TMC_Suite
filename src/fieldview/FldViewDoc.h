// FldViewDoc.h : interface of the CFldViewDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLDVIEWDOC_H__9FEC5735_707F_11D3_A93C_E0AB4DC10000__INCLUDED_)
#define AFX_FLDVIEWDOC_H__9FEC5735_707F_11D3_A93C_E0AB4DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "gl\gl.h"
#include "gl\glu.h"
#include <TmcLibError.h>


class CFldViewDoc : public CDocument
{
protected: // create from serialization only
	CFldViewDoc();
	DECLARE_DYNCREATE(CFldViewDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFldViewDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetBusyOn( void );
	void SetBusyOff( void );
	BOOL IsBusy( void );
	FILETIME GetLastWriteFile( void );
	void DelViews__( void );
	void AddViews__( void );
	double GetdLongUnit( void );
	CString GetcsLongUnit( void );
	double GetdTimeUnit( void );
	CString GetcsTimeUnit( void );
	double GetdYmin( void );
	double GetdXmin( void );
	BOOL IsFileRead( void );
	void SetSynchronizationFlag( void );
	double *GetpdSurface( void );
	double GetdDelta( void );
	double GetdTCurrent( void );
	int GetnTCurrent( void );
	int GetnY( void );
	int GetnX( void );
	CTmcLibError cError;
	void ReadData( void );
	virtual ~CFldViewDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFldViewDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL volatile bBusy;
	FILETIME LastWriteTime(CString csFileName);
	FILETIME LastWriteTime(char *szFileName);
	FILETIME ftLastWriteTime;
	int volatile nViews;
	int volatile nViewsSinchroniz;
	CString csLongUnit;
	double  dLongUnit;
	CString csTimeUnit;
	double  dTimeUnit;
	void CloseFileData( void );
	void ReadDataArray( FILE **fp );
	void InitAllocationArray( void );
	void ReadDataParameters( FILE **fp );
	void DeleteData( void );
	int nX;
	int nY;
	int nTCurrent;
	double dTCurrent;
	double dDelta;
	double dXmin;
	double dYmin;
	int nAccuracy;
	double *pdSurface;
	FILE *pfData;
public:
	double GetdZ(double dX, double dY);
	int GetNumberElement(double dX, double dY);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLDVIEWDOC_H__9FEC5735_707F_11D3_A93C_E0AB4DC10000__INCLUDED_)
