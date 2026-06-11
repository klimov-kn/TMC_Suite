// TmcRTH_BolckList.h : header file
//

/////////////////////////////////////////////////////////////////////////////

#ifndef CTMCRTH_BLOCKLIST_H__1
#define CTMCRTH_BLOCKLIST_H__1

#define TMC_RTH_BLOCKLISTSAVE_BEGIN "#BlockListBegin"
#define TMC_RTH_BLOCKLISTSAVE_END   "#BlockListEnd"
#define TMC_RTH_BLOCKLISTLOADBUFSIZE  20000

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

#include <TmcLibError.h>
#include <typerth.h>
#include <TmcRTH_IndanParam.h>


class CTmcRTH_BlockList 
{
public:
	friend class CTMCBlList;
	void SetXmin( double r );
	void SetXmax( double r );
	void SetYmin( double r );
	void SetYmax( double r );
	void del_eol( char *szBuf );
	void Load( FILE **fp, CTmcLibError &cError );
	int GetnMemoryAll( void );
	void SetnMemory( int nnn );
	int GetnMemory( void );
	CString GetcsW( void );
	CString GetcsVy( void );
	CString GetcsVx( void );
	double * GetpX( void );
	double * GetpY( void );
	int GetnXY( void );
	CString GetcsEpsExpr( void );
	int GetnType( void );
	void SetnType( int n );
	void SetcsBlock( CString csBlock1 );
	int GetBlockNumber( void );
	CTmcRTH_BlockList * GetNext( void );
	CTmcRTH_BlockList& operator=( CTmcRTH_BlockList& cBlList );
	void SetnBlock( int iBuf );
	void SetX0( double rX );
	void SetY0( double rY );
	int GetnBlock( void );
	CString GetcsBlock( void );
	double GetX0( void );
	double GetY0( void );
	double GetXmin( void );
	double GetXmax( void );
	double GetYmin( void );
	double GetYmax( void );
	void SetLink( int iBuf, double x0, double y0, CTmcLibError &cError );
	CTmcRTH_BlockList * FindBlock( int nBlock1 );
	void IsCorrectly( CTmcLibError &cError );
	void IsCorrectlyLink( CTmcLibError &cError );
	void Add( CString &csCh, CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void Add( CTmcRTH_BlockList &csCh );
	void DeleteData( void );
	CTmcRTH_BlockList();
public:
	BOOL Save1(CString &csBuffer);
	BOOL Save( CString &csBuffer  );
	void AddPathForFile( CString &csPath );
	virtual ~CTmcRTH_BlockList();
private:
	char pszBufForReadTopology[TMC_RTH_BLOCKLISTLOADBUFSIZE];
	BOOL IsFullName( void );
	void AddCurrentPath(CString csCurrentPath);
	void AddFileEpsExtention( void );
	int nMemory;
	void ReadXY( char *szBuf,  CTmcLibError &cError, CTmcRTH_IndanParam * pcParam , double *dX1, double *dY1 );
	int CalcEOL( char *szBuf  );
	int SkipEOL( char *szBuf );
	double *dX;		  // X array for polygon
	double *dY;		  // Y array for polygon
	int nXY;		  // size array for polygon
	int Search_1Param( CString &csBuf, char *szBuf, CTmcLibError &cError );
	int expr_del_Blanks2( char *ch );
	int expr_del_Blanks2( CString &csBuf );
	void ReadFileStat( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadFileStatN( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadFileStatB( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadFileStat_1( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam, int i );
	void ReadRectStat( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadRectStatN( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadRectStatB( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadRectStat_1( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam, int i );
	void ReadCircleStat( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadCircleStatN( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadCircleStatB( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadCircleStat_1( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam, int i );
	void ReadPolygonStat( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadPolygonStatN( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadPolygonStatB( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadPolygonStat_1( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam, int i );
	void ReadRectMove( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadCircleMove( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadPolygonMove( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadInputX( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void ReadInputY( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	void Read( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	double dX0;
	double dY0;
	double dXmin;
	double dXmax;
	double dYmin;
	double dYmax;
	CTmcRTH_BlockList * pcNextBlockList;
	int nBlock;
	int nType;
	CString csBlock;
	CString csEpsExpr;
	CString csMuExpr;
	CString csDzExpr;
	CString csHxExpr;
	CString csHyExpr;
	CString csForceRo;
	CString csForceSigma;
	CString csDzExprOut;
	CString csForceConductivity;
	CString csFileNameMechanicalFreedomDegree;
	CString csVx;	// liner speed X for input is Amplitude from t
	CString csVy;	// liner speed Y for input is Faza from t
	CString csW;	// angular speed for input is Eps
public:
	CString GetcsMuExpr(void);
	CString GetcsDzExpr(void);
	CString GetcsHxExpr(void);
	CString GetcsHyExpr(void);
	CString GetcsFileNameMechanicalFreedomDegree(void);
	CString GetcsForceRo(void);
	CString GetcsForceSigma(void);
	CString GetcsForceConductivity(void);
	CString GetcsDzExprOut(void);
};

#endif


