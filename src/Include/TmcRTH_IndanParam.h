// TmcRTH_IndanParam.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_IndanParam window

#ifndef CTMCRTH_INDANPARAM_H__
#define CTMCRTH_INDANPARAM_H__

#include <typerth.h>
#include <TmcLibError.h>

#define CTMCRTH_LONGUNIT_ID  "LONG_UNIT "
#define CTMCRTH_LONGUNIT_m  "m"
#define CTMCRTH_LONGUNIT_Um  1.
#define CTMCRTH_LONGUNIT_cm "cm"
#define CTMCRTH_LONGUNIT_Ucm 0.01
#define CTMCRTH_LONGUNIT_mm "mm"
#define CTMCRTH_LONGUNIT_Umm 0.001

#define CTMCRTH_TIMEUNIT_ID  "TIME_UNIT "
#define CTMCRTH_TIMEUNIT_s  "s"
#define CTMCRTH_TIMEUNIT_Us  1.
#define CTMCRTH_TIMEUNIT_ms "ms"
#define CTMCRTH_TIMEUNIT_Ums 0.001
#define CTMCRTH_TIMEUNIT_mks "mks"
#define CTMCRTH_TIMEUNIT_Umks 0.000001
#define CTMCRTH_TIMEUNIT_ns "ns"
#define CTMCRTH_TIMEUNIT_Uns 0.000000001
#define CTMCRTH_TIMEUNIT_ps "ps"
#define CTMCRTH_TIMEUNIT_Ups 0.000000000001

#define CTMCRTH_ANGLEUNIT_ID  "ANGLE_UNIT "
#define CTMCRTH_ANGLEUNIT_r  "radian"
#define CTMCRTH_ANGLEUNIT_Ur  1.
#define CTMCRTH_ANGLEUNIT_gr "gradus"
#define CTMCRTH_ANGLEUNIT_Ugr (3.141592653589/180.)

#define CTMCRTH_FREQUNIT_ID  "FREQ_UNIT "
#define CTMCRTH_FREQUNIT_Hz  "Hz"
#define CTMCRTH_FREQUNIT_UHz  1.
#define CTMCRTH_FREQUNIT_kHz "kHz"
#define CTMCRTH_FREQUNIT_UkHz 1000.
#define CTMCRTH_FREQUNIT_MHz "MHz"
#define CTMCRTH_FREQUNIT_UMHz 1000000.
#define CTMCRTH_FREQUNIT_GHz "GHz"
#define CTMCRTH_FREQUNIT_UGHz 1000000000.

#define CTMCRTH_BUNIT_ID  "B_UNIT "
#define CTMCRTH_BUNIT_Tl  "Tl"
#define CTMCRTH_BUNIT_UTl  1.
#define CTMCRTH_BUNIT_mTl "mTl"
#define CTMCRTH_BUNIT_UmTl 0.001
#define CTMCRTH_BUNIT_mkTl "mkTl"
#define CTMCRTH_BUNIT_UmkTl 0.000001

#define CTMCRTH_NUNIT_ID  "N_UNIT "
#define CTMCRTH_NUNIT_m3  "m3"
#define CTMCRTH_NUNIT_Um3  1.
#define CTMCRTH_NUNIT_cm3 "cm3"
#define CTMCRTH_NUNIT_Ucm3 0.000001
#define CTMCRTH_NUNIT_mm3 "mm3"
#define CTMCRTH_NUNIT_Umm3 0.000000001

#define CTMCRTH_FREQ_ID   "FREQ "
#define CTMCRTH_DELTA_ID  "DELTA "
#define CTMCRTH_TIME_ID   "TIME "
#define CTMCRTH_XMIN_ID   "X_MIN "
#define CTMCRTH_XMAX_ID   "X_MAX "
#define CTMCRTH_YMIN_ID   "Y_MIN "
#define CTMCRTH_YMAX_ID   "Y_MAX "
#define CTMCRTH_HEIGHT_ID "HEIGHT "
#define CTMCRTH_DELTA_T_ID "DELTA_T_MECHANICAL "

void SetDeltaT( double dDeltaT1 );
double GetDeltaT( void );
BOOL IsDeltaTDefine( void );
void SwitchDeltaT( void );
void SetDeltaT( char *lpszDeltaT );

class CTmcRTH_IndanParam //: public CWnd
{
// Construction
public:
	BOOL IsnInRegion( _real x, _real y );
	BOOL IsnInRegion( int n );
	int CalcnNodeGlobal( double x, double y );
	int CalcnNodeGlobalFM( double x, double y );
	double CalcY( int nNumber );
	double CalcX( int nNumber );
	void GetXYMinMax( double &dXmin, double &dXmax, double &dYmin, double &dYmax );
	double dGetDelta( void );
	double dGetdT( void );
	double dGetTmin( void );
	int nGetTmax( void );
	double dGetHeight( void );
	CTmcLibError cError;
	CString csFreqUnit;
	double rFreqUnit;
	CString csLongUnit;
	double rLongUnit;
	CString csTimeUnit;
	double rTimeUnit;
	CString csAngleUnit;
	double rAngleUnit;
	CString csBUnit;
	double rBUnit;
	CString csNUnit;
	double rNUnit;
	double rDelta;
	double rFreq;
	double rHeight;
	double rt;
	double rXmin;
	double rXmax;
	double rYmin;
	double rYmax;
	double rTmin;
	double rTmax;
	CTmcRTH_IndanParam& operator=( CTmcRTH_IndanParam& cParam );
	double rGetLongUnit( void );
	CString csGetLongUnit( void );
	void IsCorrectly( CTmcLibError &cError1 );
	void Read( CString &csCh, CTmcLibError &cError1 );
	void DeleteData( void );
	CTmcRTH_IndanParam();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTH_IndanParam)
	//}}AFX_VIRTUAL

// Implementation
public:
	int CalcnY( void );
	int CalcnX( void );
	virtual ~CTmcRTH_IndanParam();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcRTH_IndanParam)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
//	DECLARE_MESSAGE_MAP()
private:
	int expr_del_Blanks2( char *ch );
	int Search_1Param( CString &csBuf, char *szBuf );
	void ReadAngleUnit( CString &csCh );
	void ReadTimeUnit( CString &csCh );
	void ReadLongUnit( CString &csCh );
	void ReadFreqUnit( CString &csCh );
	void ReadTime( CString &csCh );
	void ReadFreq( CString &csCh );
	void ReadDelta( CString &csCh );
	void ReadYmax( CString &csCh );
	void ReadYmin( CString &csCh );
	void ReadXmax( CString &csCh );
	void ReadXmin( CString &csCh );
	void ReadHeight( CString &csCh );
	void ReadDeltaTMechanical( CString &csCh );
	void ReadBUnit( CString &csCh );
	void ReadNUnit( CString &csCh );
public:
	// CalcnNodeGlobal return NodeGlobal or -1, if point x,y isn't in region
	int CalcnNodeGlobalNorm(double x, double y);
	double rDeltaTMechanical;
};


#endif
/////////////////////////////////////////////////////////////////////////////
