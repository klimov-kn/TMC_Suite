// TmcRTHRectNode.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTHRectNode window

#ifndef _CTMCRTHRECTNODE__ 

#define _CTMCRTHRECTNODE__ 

#include <typerth.h>
#include "TmcRTHNodeDiel.h"
#include <TmcLibError.h>
#include <TmcRTH_Indan.h>
#include <TmcRTH_IndanParam.h>

class CTmcRTHRectNode : public CWnd
{
// Construction
public:
	BOOL IsReadData( void );
	BOOL IsRunStep( void );
	BOOL IsError( void );
	void Stop( void );
	CTmcRTH_IndanOutput* GetOutput( void );
	CTmcRTH_IndanTopology& GetTopology( void );
	CTmcRTH_IndanParam& GetParam( void );
	int GetNStepMax( CString csFileName );
	CString GetErrorMessage( void );
	void ReadData( CString csFileName, int nStep1 );
	void ReadData( void );
	void RunStep( void );
	CTmcRTHRectNode();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTHRectNode)
	//}}AFX_VIRTUAL

// Implementation
public:
	void ExportToDirectionalPatter( void );
	BOOL GetTopologyFlag( void );
	BOOL GetSoundEffect( void );
	BOOL GetFieldFlag( void );
	BOOL GetSinchronizationFieldOutput( void );
	void PutTopol( void );
	void OnOffSoundEffects( void );
	void OnOffSinchronizationFieldOutput( void );
	void OnOffFieldOutput( void  );
	virtual ~CTmcRTHRectNode();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcRTHRectNode)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void DistributionIntegrated( int ii );
	void PutSmatrix( void );
	BOOL IsFieldFileRead( void );
	void PutTopology( void );
	void PutStepEndSound( void );
	void RunBlockNode3( void );
	void RunBlockNode2( void );
	void RunBlockNode1( void );
	void RunBlockNode( void );
	void PutField( int ii );
	near void RunScatteringNode1line( _ELEM_VAL_RTH *pr_111, _real *prY_111, int nx_111, int nX_111);
	near void RunScatteringNode1line1( _ELEM_VAL_RTH *pr_111, _real *prY_111, int nx_111, int nX_111);
	near void RunScatteringNode1line2( _ELEM_VAL_RTH *pr_111, _real *prY_111, int nx_111, int nX_111);
	near void RunScatteringNode1line3( _ELEM_VAL_RTH *pr_111, _real *prY_111, int nx_111, int nX_111);
	void OutputnT( void );
	void DeleteOutputFile( void );
	void RunExciteInputs3( void );
	void RunExciteInputs2( void );
	void RunExciteInputs1( void );
	void RunExciteInputs( void );
	void SetDielInNodeList( void );
	void SetDielTopology( void );
	void DeleteDielList( sTmcRTH_DielNodeList ** pcNodeDielList1 );
	void RunScatteringNode( void );
	void RunScatteringNode1( void );
	void RunScatteringNode2( void );
	void RunScatteringNode3( void );
	void RunSaveInputs( void );		// BUG12 portfix
	void RunRestoreInputs( void );	// BUG12 portfix
	CTmcRTH_IndanParam cParam;
	void DeleteDielArray( void );
	void InitDielArray( void );
	BOOL IsDataInit( void );
	volatile BOOL bIsRunStep1Run;
	void RunStep1( void );
	void DeleteNodeArray( void );
	void InitNodeArray( void );
	void RunKernel1T( void );
	double dT;
	double dTCurrent;
	int nTMax;
	void RunKernel( void );
	void InitKernel( void );
	volatile BOOL bIsReadData;
	void ReadOutputSection( void );
	void ReadLinkListSection( void );
	void ReadParamSection( void );
	void ReadTopologySection( void );
	void SetStepNumber( int nStep );
	CTmcRTH_Indan cIndan;
	CTmcLibError cError;
	CTmcRTHNodeDiel * pcNodeDiel; // array for each dielectric
	int nDiel;					// block number ( dielectric number )
	int nTCurrent;				// current number of t time
	int nX;						// number of node for X axies
	int nY;						// number of node for Y axies
	double dDelta;				// global parameter delta = deltaX = deltaY
	double dXmin;				// global parameter Xmin for rectangle
	double dXmax;				// global parameter Xmax for rectangle
	double dYmin;				// global parameter Ymin for rectangle
	double dYmax;				// global parameter Ymax for rectangle
	void DeleteData( void );
	_ELEM_VAL_RTH * prUNode1;			// array voltage on unit with 1 takt 
//	prUNode1[0] - voltage in takt t   and t-4;
//	prUNode1[1] - voltage in takt t-1 and t-5;
//	prUNode1[2] - voltage in takt t-2 and t-6;
//	prUNode1[3] - voltage in takt t-3 and t-7;
	_ELEM_VAL_RTH * prUNode1Temp;		// temp pointer 
//	_ELEM_VAL_RTH * prUNode2;			// array voltage on unit with 2 takt  
	_real * prYNode;					// node admitance array
//	prYNode[0] - value 2/(y1+y2+y3+y4+y5+y+y6*y7/(y6+y7));
//	prYNode[1] - value y5;
//	prYNode[2] - value y;
//	prYNode[3] - value 2*(y6*y7)/(y6+y7);
//	prYNode[4] - value 2*(y7-y6)/(y6+y7);
//	prYNode[5] - value prYNode[3]*prYNode[4]/4;
	_real * prYNodeTemp;				// temp pointer
//	_real * prYMulNode;					// node multiple admitance array
//	  int * pnTypeNode;					// node type array
//	_ELEM_VAL_RTH * prUyNodeTop;		// array falling voltage on unit with a top    V for each node
//	_ELEM_VAL_RTH * prUyNodeBottom;		// array falling voltage on unit with a bottom ^ for each node
//	_ELEM_VAL_RTH * prUxNodeRight;		// array falling voltage on unit at the right	 < for each node
//	_ELEM_VAL_RTH * prUxNodeLeft;		// array falling voltage on unit at the left   > for each node
//	_real * prUNode;			// array falling voltage 1 = V; 2 = ^; 3 = <; 4 = >
//	sTmcRTH_DielNodeList ** pcNodeDielList; // array node dielectric list for each node
	int nNumNode;				// number of all node = nX*nY
};


#endif
/////////////////////////////////////////////////////////////////////////////
