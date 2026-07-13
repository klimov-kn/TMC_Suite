// PlanRT_HDoc.h : interface of the CPlanRT_HDoc class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef PLANRT_HDOC__H__
#define PLANRT_HDOC__H__

#include "TmcRTHRectNode.h"

#define N_BUF_MAX 10000
											
#define PLANRT_H_RAZDEL_INI					"PlanRT_H Config"
#define PLANRT_H_EXTERNEDITORNAME_INI		"ExternEditorName"
#define PLANRT_H_EXTERNVIEWERNAME_INI		"ExternViewerName"
#define PLANRT_H_EXTERNFIELDVIEWERNAME_INI  "ExternFieldViewerName"
#define PLANRT_H_EXTERNDIRPATVIEWERNAM_INI  "ExternDirectPatViewerName"
#define PLANRT_H_EXTERNSMVIEWERNAME_INI		"ExternSmatrixViewerName"
#define PLANRT_H_BACKGROUNDCOLOR_INI		"BackGroundColor"
#define PLANRT_H_OUTFORMATNT_INI			"OutputFormatnT"
#define PLANRT_H_OUTFORMATDT_INI			"OutputFormatdT"
#define PLANRT_H_OUTFORMATNB_INI			"OutputFormatnB"
#define PLANRT_H_OUTFORMATINP_INI			"OutputFormatdInp"
#define PLANRT_H_OUTFORMATOUT_INI			"OutputFormatdOut"
#define PLANRT_H_OUTPUTSOUNDFLAG_INI		"SoundEffectsFlag"
#define PLANRT_H_OUTFORMATOUTFILED_INI		"OutputFormatdOutField"
#define PLANRT_H_OUTFIELDSINCHRFLAG_INI		"OutputSinchrFlagdOutField"
#define PLANRT_H_OUTFIELDFLAG_INI			"OutputFlagdOutField"
#define PLANRT_H_MNWNDSIZEPLACE_INI			"MainFrameSizeAndPlace__1"

class CPlanRT_HDoc : public CDocument
{
protected: // create from serialization only
	CPlanRT_HDoc();
	DECLARE_DYNCREATE(CPlanRT_HDoc)

// Attributes
public:
	void CloseAndExit( void );
	BOOL IsRun( void );
	void RunAll( void );
	void Stop( void );
	void SetRunThreadHandle( HANDLE hThread );
	void StopAndWait( void );
	void RunStep( void );
	BOOL IsReadData( void );
	CTmcRTH_IndanOutput* GetOutput( void );
	CTmcRTH_IndanParam& GetParam( void );
	CTmcRTH_IndanTopology& GetTopology( void );
	CString GetErrorMessage( void );
	void BackStep( void );
	void SkipStep( void );
	void SetFirstStep( void  );
	int GetnStep( void );
	void ReadData( void );
	BOOL RunExeFile( CString csNameExe );
	void SetExternEditorName( CString csExternEditorName );
	CString GetExternEditorName( void );
	BOOL RunExeFile( char *lpszNameExe );

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlanRT_HDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void OnCloseDocument();
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	//}}AFX_VIRTUAL

// Implementation
public:
	void ExportToDirectionalPatter( void );
	void SetExternViewerDirectionalpattern( CString csFileName );
	BOOL RunViewerDirectPattern( void );
	CTmcRTHRectNode & GetRectNode( void );
	BOOL GetTopologyFlag( void );
	BOOL GetSoundEffect( void );
	BOOL GetFieldFlag( void );
	BOOL GetSinchronizationFieldOutput( void );
	void PutTopology( void );
	void SetSoundFlag( int i );
	void OnOffSoundEffects( void );
	void SetOutFieldOutFlag( int i );
	void SetOutSinchrFieldOutFlag( int i );
	void OnOffSinchronizationFieldOutput( void );
	BOOL RunViewerField( void );
	BOOL RunViewerSmatrix( void );
	void OnOffFieldOutput( void );
	CString GetExternViewer( void );
	CString GetExternViewerDirectPat( void );
	CString GetExternViewerField( void );
	CString GetExternViewerSmatrix( void );
	BOOL RunViewer( void );
	void SetExternViewer( CString csExternViewer1 );
	void SetExternViewerField( CString csExternViewerField1 );
	void SetExternViewerSmatrix( CString csExternViewerField1 );
	BOOL RunExeFile( CString csNameExe, CString csArgCommLine);
	BOOL RunExeFile( char *lpszNameExe, char *lpszArgCommLine  );
	virtual ~CPlanRT_HDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CPlanRT_HDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CString csExternViewerDirectPattern;
	CString csExternViewerSmatrix;
	CString csExternViewerField;
	CString csExternViewer;
	volatile BOOL bIsReadData;
	HANDLE m_hRunThread;
	int nStep;
	CTmcRTHRectNode cRectNode;
	CString csEditorName;
	BOOL CreatProc(char * lpszComLine);
};



#endif
/////////////////////////////////////////////////////////////////////////////
