// TmcRTH_IndanTopology.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_IndanTopology window

#ifndef CTMCRTH_INDANTOPOLOGY_H__
#define CTMCRTH_INDANTOPOLOGY_H__

#include <TmcRTH_BolckList.h>
#include <TmcRTH_IndanParam.h>

class CTmcRTH_IndanTopology //: public CWnd
{
// Construction
public:
	BOOL IsDataRead( void );
	int GetBlockNumber( void );
	CTmcRTH_IndanTopology& operator=( CTmcRTH_IndanTopology& cTopol );
	CTmcRTH_IndanParam * GetpParam( void );
	CTmcRTH_BlockList * GetBlockList( void );
	void SetParam( CTmcRTH_IndanParam * pcParam1 );
	void IsCorrectly( CTmcLibError &cError );
	void IsCorrectlyLink( CTmcLibError &cError );
	void ReadLink( CString &csCh, CTmcLibError &cError );
	void Read( CString &csCh, CTmcLibError &cError );
	void DeleteData( void );
	CTmcRTH_IndanTopology();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTH_IndanTopology)
	//}}AFX_VIRTUAL

// Implementation
public:
	void AddPathForFile( CString &csCurrentPath );
	virtual ~CTmcRTH_IndanTopology();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcRTH_IndanTopology)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
//	DECLARE_MESSAGE_MAP()
private:
	CTmcRTH_IndanParam * pcParam;
	CTmcRTH_BlockList cBlockList;
	int expr_del_Blanks2( char *ch );
	int Search_1Param( CString &csBuf, char *szBuf, CTmcLibError &cError );
};


#endif
/////////////////////////////////////////////////////////////////////////////
