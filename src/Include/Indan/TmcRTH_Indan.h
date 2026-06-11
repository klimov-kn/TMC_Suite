// TmcRTH_Indan.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_Indan window

#ifndef CTMCRTH_INDAN_H__
#define CTMCRTH_INDAN_H__

#include "TmcLibError.h"
#include "TmcRTH_IndanParam.h"
#include "TmcRTH_IndanOutput.h"
#include "TmcRTH_IndanTopology.h"

#define CTMCRTH_INDANSBUF 30000
#define CTMCRTH_INDANPREPRFNA "~$Prepr$~"
#define CTMCRTH_INDAN_LINELEN     20000

#define CTMCRTH_INDANMET_PROGRAM     "#TMC_RT_H"
#define CTMCRTH_INDANMET_EOF         "#EOF"
#define CTMCRTH_INDANMET_STEP	     "#STEP"
#define CTMCRTH_INDANMET_ENDSTEP     "#END_STEP"
#define CTMCRTH_INDANMET_PARAM	     "#PARM"
#define CTMCRTH_INDANMET_ENDPARAM    "#END_PARM"
#define CTMCRTH_INDANMET_TOPOLOGY    "#TOPOLOGY"
#define CTMCRTH_INDANMET_ENDTOPOLOGY "#END_TOPOLOGY"
#define CTMCRTH_INDANMET_LINKLIST	 "#LINK_LIST"
#define CTMCRTH_INDANMET_ENDLINK	 "#END_LINK"
#define CTMCRTH_INDANMET_OUTPUT		 "#OUTPUT"
#define CTMCRTH_INDANMET_ENDOUTPUT	 "#END_OUTPUT"

#define CTMCRTH_INDANMET_BLOCK	  "BLOCK "
#define CTMCRTH_INDANMET_ENDBLOCK "END_B"
#define CTMCRTH_INDANMET_LINK	  "T "


class CTmcRTH_Indan : public CWnd
{
// Construction
public:
	int GetnBlock( void );
	void GetXYMinMax( double &dXmin, double &dXmax, double &dYmin, double &dYmax );
	double dGetDelta( void );
	double dGetdT( void );
	double dGetTmin( void );
	int nGetTmax( void );
	int GetnStep( void );
	BOOL IsDataRead( void );
	CTmcRTH_IndanOutput* GetOutput( void );
	CTmcRTH_IndanTopology& GetTopology( void );
	CTmcRTH_IndanParam& GetParam( void );
	void ReadTopologySection( void );
	void ReadLinkListSection( void );
	void ReadOutputSection( void );
	void ReadParamSection( void );
	void SetStepNumber( int nStep1 );
	int GetNStepMax( void );
	int GetNStepMax( CString csFileName );
	CString GetErrorMessage( void );
	BOOL IsError( void );
	void SetTplFileName( char *pszFileName );
	void SetTplFileName( CString csFileName );
	CTmcRTH_Indan();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTH_Indan)
	//}}AFX_VIRTUAL

// Implementation
public:
	void OnOffSoundEffects( void );
	CString GetCurrentPath( void );
	void GetnXnY( int &nX11, int &nY11 );
	void OnOffSinchronizationFieldOutput( void );
	void OnOffFieldOutput( void );
	virtual ~CTmcRTH_Indan();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcRTH_Indan)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CTmcRTH_IndanTopology cTopology;
	int GetBlock( CString &csCh );
	int GetLineSection( CString &csCh );
	CString csEndSection;
	CString csSection;
	int FindSection( void );
	int FindSection( char * pszSection,  char * pszEndSection);
	void FindStep( int nStep1 );
	void ReadParamSection( int nStep1 );
	int nStep;
	void del_blanks( char * szBuf );
	int GetLine( CString &csCh );
	FILE * pfPrepr;
	void ClosePreprFile( void );
	void OpenPreprFile( void );
	int CalcNStepMax( void );
	void MakePreprFile( void );
	void SetCurrentPath( void );
	void DeleteData( void );
	CTmcRTH_IndanParam  cParam;
	CTmcRTH_IndanOutput cOut;
	CTmcLibError cError;
	CString csCurrentPath;
	CString csTplFileName;
	CString csTplFileNamePrepr;
};


#endif
/////////////////////////////////////////////////////////////////////////////
