// TmcRTHNodeDiel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTHNodeDiel window

#ifndef _CTMCRTHNODEDIEL__ 

#define _CTMCRTHNODEDIEL__ 

#include <typerth.h>
#include "TmcLibError.h"
#include "TmcRTH_IndanParam.h"
#include "TmcRTH_BolckList.h"
#include "TmcRTH_IndanOutput.h"

#define TMC_RTH_EPSFILE_EXT_	"eps"
#define TMC_RTH_EPSFILE_ID_		"#TamicRTH_planar_DistributionDielectricPermeability_File_V2.00 2000"
#define TMC_RTH_EPSFILE_TYPE_	"#TopologyPrimitiv "
#define TMC_RTH_EPSFILE_Delta_	"#dDelta "
#define TMC_RTH_EPSFILE_XMIN_	"#Xmin "
#define TMC_RTH_EPSFILE_YMIN_	"#Ymin "
#define TMC_RTH_EPSFILE_NX_		"#nX "
#define TMC_RTH_EPSFILE_NY_		"#nY "
#define TMC_RTH_EPSFILE_NPoint_	"#nPoint "
#define TMC_RTH_EPSFILE_NAccur_	"#nAccuracy "
#define TMC_RTH_EPSFILE_NdFrmt_	"#sNodeFormat "
#define TMC_RTH_EPSFILE_VlFrmt_	"#sValueFormat "

struct STMCRTH_NODEDIELONE
{
	_ELEM_VAL_RTH rU;	// falling voltage on dielectric in 0 takt
	_ELEM_VAL_RTH rU1;	// falling voltage on dielectric in 1 takt
	_ELEM_VAL_RTH rU2;	// falling voltage on dielectric in 2 takt
	_ELEM_VAL_RTH rU3;	// falling voltage on dielectric in 3 takt
	_ELEM_VAL_RTH rU_Cur;	// curretn voltage on dielectric
	_ELEM_VAL_RTH rU_Cur1;	// curretn+1 takt voltage on dielectric
	_ELEM_VAL_RTH rU_Cur2;	// curretn+2 takt voltage on dielectric
	_ELEM_VAL_RTH rU_Cur3;	// curretn+3 takt voltage on dielectric
	_real rY;			// admitance for node
	TmcIdx nNodeGlobal;  // number of node in global topology	// INT64
	int nType;			// signum for eps	=  0 for eps < 0;
						//					=  1 for eps > 0;
						//					= -1 for metall;
						//					= -2 for absorber;
						//	is equel members nType in class CTmcRTH_Input 
						//					= -3 for input x left   > 
						//					= -4 for input x right  < 
						//					= -5 for input y top    V 
						//					= -6 for input y bottom ^ 
} ;
typedef struct STMCRTH_NODEDIELONE sTmcRTHNodeDielOne;

struct STMCRTH_DIELNODELIST
{
	STMCRTH_DIELNODELIST * pcDielNodeNextList;
	sTmcRTHNodeDielOne * pcNodeDielOne;
} ;
typedef struct STMCRTH_DIELNODELIST sTmcRTH_DielNodeList;

struct sEpsFastJob;	// EPSFAST

class CTmcRTHNodeDiel : public CWnd
{
// Construction						 
public:
	void GpuView( int &nType1, TmcIdx &nNum1, sTmcRTHNodeDielOne *&p1 ) { nType1 = nType; nNum1 = nNumNode; p1 = pcNodeDielOne; }	// GPU: read access for TmcGpuX.h
	void ExciteInputs(  CTmcLibError &cError1, double dWT, double dT, _ELEM_VAL_RTH *pr1, int nX, CTmcRTH_IndanParam &cParam, double dtCurrent );
	void ExciteInputs1( CTmcLibError &cError1, double dWT, double dT, _ELEM_VAL_RTH *pr1, int nX, CTmcRTH_IndanParam &cParam, double dtCurrent );
	void ExciteInputs2( CTmcLibError &cError1, double dWT, double dT, _ELEM_VAL_RTH *pr1, int nX, CTmcRTH_IndanParam &cParam, double dtCurrent );
	void ExciteInputs3( CTmcLibError &cError1, double dWT, double dT, _ELEM_VAL_RTH *pr1, int nX, CTmcRTH_IndanParam &cParam, double dtCurrent );
	/* BUG12 portfix: exclude input-port nodes from scattering */
	void SaveInputNodes( _ELEM_VAL_RTH *pr );
	void RestoreInputNodes( _ELEM_VAL_RTH *pr );
	_ELEM_VAL_RTH *prInputSave;
	CTmcLibError &SetBlock( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut, _real *prYNode, volatile BOOL *pbStopFlag, CString csCurrentPath );
	void DeleteData( void );
	CTmcRTHNodeDiel();
	void SetTimingCounters( __int64 *pqSignalFile, __int64 *pqEpsFile ) { m_pqSignalFileTicks = pqSignalFile; m_pqEpsFileTicks = pqEpsFile; }	// TIMING
	void SetTimingEpsCounters( __int64 *pqEpsRead, int *pnEpsWritten, int *pnEpsRead ) { m_pqEpsReadTicks = pqEpsRead; m_pnEpsWritten = pnEpsWritten; m_pnEpsRead = pnEpsRead; }	// TIMING
	// --- wideband post-processing: port data for the .t header
	BOOL IsSignalFromFile( void ) { return bSigFile; }
	BOOL IsInputBlock( void );
	int GetBlockNumber( void ) { return nBlock; }
	double GetWidthWaveg( void ) { return dWidthWaveg; }
	CString GetSigFreqList( void ) { return csSigFreq; }

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTHNodeDiel)
	//}}AFX_VIRTUAL

// Implementation
public:
	void AddCurrentPath( CString csCurrentPath );
	CTmcLibError & GetError( void );
	void RunBlockNode3( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, TmcIdx nArraySize );
	void RunBlockNode2( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, TmcIdx nArraySize );
	void RunBlockNode1( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, TmcIdx nArraySize );
	void RunBlockNode( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, TmcIdx nArraySize  );
	virtual ~CTmcRTHNodeDiel();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcRTHNodeDiel)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void SetMagneticPolygonType( CTmcRTH_IndanParam &cParam );
	void CalculateYForInput( double dFreq, double dDelta );
	double dFaza(CTmcRTH_IndanParam &cParam, double dtCurrent);
	double dAmplitude(CTmcRTH_IndanParam &cParam, double dtCurrent);
	double dUnormir;
	BOOL IsFullName( void );
	void AddFileEpsExtention( void );
	void SeachEpsFileName( CString csCurrentPath );
	CString csFileNameEps;
// --- incident wave from file (wideband excitation) ---------------------
	void SeachSignalFileName( void );
	void ReadSignalFile( void );
	void DeleteSignalData( void );
	double dSignalFromFile( double dtCurrent, CTmcRTH_IndanParam &cParam );
	BOOL bSigFile;
	double *pdSigT;
	double *pdSigV;
	int nSigN;
	CString csFileNameSig;
	CString csSigFreq;		// frequency list named inside the .sig file, if any
	CString csSigPath;
	void DeleteStatDielData( void );
	volatile BOOL * pbIsStop;
	__int64 * m_pqSignalFileTicks;	// TIMING: time of writing port signals (QPC ticks)
	__int64 * m_pqEpsFileTicks;		// TIMING: time of writing eps-distribution files
	__int64 * m_pqEpsReadTicks;		// TIMING: time of FILE blocks (reading distribution files)
	int * m_pnEpsWritten;			// TIMING: number of blocks that wrote a distribution file
	int * m_pnEpsRead;				// TIMING: number of blocks read from a distribution file
	void TimingEpsRead( __int64 qTicks ) { if( m_pqEpsReadTicks != NULL ) *m_pqEpsReadTicks += qTicks; if( ( m_pnEpsRead != NULL )&&( !cError.IsError() ) ) (*m_pnEpsRead)++; }	// TIMING
	void SaveFileRect( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CString csCurrentPath );
	void DeleteDublicateData( void );
	void SetAbsorberNodeType( CTmcRTH_IndanParam &cParam );
	double atan2__1( double y, double x  );
	double dY0;
	double dX0;
	CTmcLibError & SetPolyMagnetic( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	CTmcLibError & SetPolyAbsorber( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	CTmcLibError & SetRectMagnetic( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	CTmcLibError & SetRectAbsorber( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	CTmcLibError & SetPolyMetal(CTmcRTH_BlockList *pcNextBlockList, CTmcRTH_IndanParam &cParam);
	CTmcLibError & SetRectMetal( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	double SetInputAdmitance( double dFreq, double dDelta );
	double SetAbsorberAdmitance( double dFreq, double dDelta );
	double dY1_InputAdmitance;
	double dWidthWaveg;
	void OutputUpUo( double dUp, double dUo, CTmcLibError &cError1);
	_real CalcEps( double x, double y, CTmcRTH_IndanParam &cParam );
	// EPSFAST: fast build of stationary eps blocks (TmcRTHNodeDiel.cpp)
	BOOL SetStatEpsFast( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CString csCurrentPath, _real *prYNode, int nShape, int nStride, int nOffset );
	void EpsFastRun( sEpsFastJob *pJob );
	BOOL EpsFastNode( sEpsFastJob *pJob, int i, int j, _real *prY );
	int  EpsFastEval( const char *pszExpr, double x, double y, CTmcRTH_IndanParam &cParam, char *szBuf, char *szBuf1, _real *pr );
	static int EpsFastExprKind( const char *pszExpr );
	static unsigned EpsFastThreads( void );
	static BOOL EpsFastEnabled( void );
	int  EpsFastTypeOffset( void );
	void SetnType( void );
	void SetYLineForDiel( CTmcRTH_IndanParam &cParam, _real *prYNode );
	void SetYForInput( CTmcRTH_IndanParam &cParam );
	CTmcLibError &SetInpYBot( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut );
	CTmcLibError &SetInpYTop( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut );
	CTmcLibError &SetInpXRight( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut );
	CTmcLibError &SetInpXLeft( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut );
	void CalculatedXYCenter( CTmcRTH_IndanParam &cParam );
	CTmcLibError &SetPoly( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	CTmcLibError &SetCirc( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	CTmcLibError &SetCircMagnetic( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	CTmcLibError &SetCircAbsorber( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	CTmcLibError &SetCircMetal( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	CTmcLibError &SetRect( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CString csCurrentPath );
	CTmcLibError &SetRectFile( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CString csCurrentPath );
	CTmcLibError cError;
	int nBlock;							// block number
	CString csBlock;					// block Type
	TmcIdx nNumNode;					// number of node in dielectric	// BIGBLOCK
	sTmcRTHNodeDielOne * pcNodeDielOne; // array of dielectric node
	double dXCenter;					// x for center block
	double dYCenter;					// y for center block
	double dYInput;						// admittance for input block
	double dTmin;						// T_min for excite input block
	double dTmax;						// T_max for excite input block
	CString csEpsExpr;					// expression for eps
	CString csW;						// angular speed
	CString csVx;						// linear speed	for block center of rotation x0, y0
	CString csVy;						// linear speed	for block center of rotation x0, y0
	int nX0;							// x0 for centre of rotation
	int nY0;							// y0 for centre of rotation
	int nTxCurrent;						// counter for x moving
	int nTyCurrent;						// counter for y moving
	int nTxMove;						// period for x moving
	int nTyMove;						// period for y moving
	int nType;			// signum for eps	=  0 for eps < 0;
						//					=  1 for eps > 0;
						//					= -1 for metall;
						//					= -2 for absorber;
						//	is equel members nType in class CTmcRTH_Input 
						//					= -3 for input x left   > 
						//					= -4 for input x right  < 
						//					= -5 for input y top    V 
						//					= -6 for input y bottom ^ 
};



#endif
/////////////////////////////////////////////////////////////////////////////
