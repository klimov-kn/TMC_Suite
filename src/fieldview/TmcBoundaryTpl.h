#if !defined(AFX_TMCBOUNDARYTPL_H__E9DBCCC1_C1D4_11D3_A940_908A4DC10000__INCLUDED_)
#define AFX_TMCBOUNDARYTPL_H__E9DBCCC1_C1D4_11D3_A940_908A4DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcBoundaryTpl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcBoundaryTpl window
#include "TmcBlockTpl.h"
#include "TmcTopSize.h"


class CTmcBoundaryTpl //: public CWnd
{
// Construction
public:
	CTmcBoundaryTpl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcBoundaryTpl)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetnInputNum( int n );
	int GetnInputNum( void );
	CTmcTopSize& GetcTopSizeY0( void );
	CTmcTopSize& GetcTopSizeX0( void );
	CTmcTopSize * GetpcTopSize( void );
	int GetnTopSize( void );
	double * GetpdX1( void );
	double * GetpdX2( void );
	double * GetpdX3( void );
	double * GetpdX4( void );
	double * GetpdY1( void );
	double * GetpdY2( void );
	double * GetpdY3( void );
	double * GetpdY4( void );
	CString GetcsBlock( void );
	CString GetcsEpsExpr( void );
	double GetdX0( void );
	double GetdY0( void );
	int GetnBlock( void );
	int GetnPoint( void );
	int GetnType( void );
	void SetBlockList(CTmcRTH_BlockList *pcBlList, CTmcLibError &cError, double dDelta);
	virtual ~CTmcBoundaryTpl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcBoundaryTpl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
//	DECLARE_MESSAGE_MAP()
private:
	int nInputNum;
	int nTopSize;
	CTmcTopSize cTopSizeLinklListX0;
	CTmcTopSize cTopSizeLinklListY0;
	CTmcTopSize * pcTopSize;
	void TranslateX0Y0( void );
	void SetCircleStat(CTmcRTH_BlockList *pcBlList, CTmcLibError &cError, double dDelta);
	void SetInputY(CTmcRTH_BlockList *pcBlList, CTmcLibError &cError, double dDelta);
	void SetInputX(CTmcRTH_BlockList *pcBlList, CTmcLibError &cError, double dDelta);
	void SetRectStat(CTmcRTH_BlockList *pcBlList, CTmcLibError &cError, double dDelta);
	void SetPolygonStat(CTmcRTH_BlockList *pcBlList, CTmcLibError &cError, double dDelta);
	double dX0;
	double dY0;
	int nBlock;
	CString csBlock;
	CString csEpsExpr;
	void DeleteData( void );
	double * pY1;
	double * pY2;
	double * pY3;
	double * pY4;
	double * pX1;
	double * pX2;
	double * pX3;
	double * pX4;
	int nPoint;
	int nType;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCBOUNDARYTPL_H__E9DBCCC1_C1D4_11D3_A940_908A4DC10000__INCLUDED_)
