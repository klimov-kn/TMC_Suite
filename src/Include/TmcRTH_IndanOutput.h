// TmcRTH_IndanOutput.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_IndanOutput window

#ifndef CTMCRTH_INDANOUTPUT_H__
#define CTMCRTH_INDANOUTPUT_H__

#define CTMCRTH_FILEOUT_ID      "FILE "
#define CTMCRTH_TOPOLOGYOUT_ID  "TOPOLOGY"
#define CTMCRTH_FIELDSOUT_ID    "FIELDS"
#define CTMCRTH_SMATRIXOUT_ID   "SMATRIX"
#define CTMCRTH_FLDSDSTROUT_ID  "FIELD_DISTRIBUTION "
#define CTMCRTH_FLDSDSMROUT_ID  "FIELD_DISTRIBUTION_M "


#include <TmcLibError.h>
#include <FieldIntegrated.h>


class CTmcRTH_IndanOutput //: public CWnd
{
// Construction
public:
	void AddPath( CString csPath );
	CTmcRTH_IndanOutput& operator=( CTmcRTH_IndanOutput& cOut );
	double dTmin;
	double dTmax;
	double dFreq;
	double dFreqUnit;
	double dTimeUnit;
	BOOL bOutSMatrix;
	BOOL bOutField;
	BOOL bOutField1;
	BOOL bOutFieldU;
	BOOL bOutTopology;
	BOOL bOutFieldSinchronization;
	CString csFileName;
	CString csFileNameField;
	CString csFileNameField1;
	CString csFileNameField1Mod;
	CString csFileNameField1Faz;
	CString csFileNameFieldU;
	CString csFileNameTopology;
	CString csFileNameSMatrix;
	CString csFileNameQ;
	CString csFileNameIx;
	CString csFileNameIy;
	void IsCorrectly( CTmcLibError &cError1 );
	void Read( CString &csCh, CTmcLibError &cError1 );
	void DeleteData( void );
	CTmcRTH_IndanOutput();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTH_IndanOutput)
	//}}AFX_VIRTUAL

// Implementation
public:
	CFieldIntegrated& GetFieldIntegrated( void );
	BOOL bIsFieldDistribution( void );
	void PutSmatrix( void );
	void SetFreqAndTimeUnit( double dFr, double dTim );
	void OnOffSoundEffects( void );
	BOOL bSoundEffect;
	void OnOffSinchronizationFieldOutput( void );
	void OnOffFieldOutput( void );
	void OnOffFieldOutput1( void );
	void OnOffFieldOutputU( void );
	virtual ~CTmcRTH_IndanOutput();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcRTH_IndanOutput)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
//	DECLARE_MESSAGE_MAP()
private:
	void ReadSmatrix( CString &csCh );
	BOOL IsFullPath( void );
	int expr_del_Blanks2( char *ch );
	int Search_1Param( CString &csBuf, char *szBuf );
	void ReadFileDistr( CString &csCh, char *ch );
	void ReadFileDistrInMemory( CString &csCh );
	void ReadFileName( CString &csCh );
	void ReadTopology( void );
	void ReadFields( void );
	CTmcLibError cError;
	CFieldIntegrated cFieldDistr;
};


#endif

/////////////////////////////////////////////////////////////////////////////
