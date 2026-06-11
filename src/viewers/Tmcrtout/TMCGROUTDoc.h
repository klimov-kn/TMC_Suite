// TMCGROUTDoc.h : interface of the CTMCGROUTDoc class
//
/////////////////////////////////////////////////////////////////////////////
#include <tmcgrviw.h>
#include <proc_s.h>

class CTMCGROUTDoc : public CDocument
{
protected: // create from serialization only
	CTMCGROUTDoc();
	DECLARE_DYNCREATE(CTMCGROUTDoc)

// Attributes
public:
	BOOL bLossPoint;
	void SetGraphColor( void );
	COLORREF scBackgoundColor;
	COLORREF scGridColor;
	COLORREF scAxiesColor;
	COLORREF scTextColor;
	COLORREF scPointColor;
	LOGFONT lfInitial;
	CString csEditorName;
	void AddGraphicsInDoc( char *szSFileName );
	BOOL IsDataModific( void );
	void ReadGraphParameters( void );
	BOOL WriteDocFile();
	char error[300];
	void ReadDocFile(void);
	TMC_GR_DOC grdoc;
	void RunExeFile( char *lpszNameExe );
	BOOL DocFileNewData(int n);
	void DocFileDelData();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTMCGROUTDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	TMC_GR_DOC1 sGrDoc[16];
	double dFreq;
	void ReadGraphParametersDefault( void );
	virtual ~CTMCGROUTDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTMCGROUTDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void SetDefaultLineColorWidthType( void );
	void SetOneReadGraphParameters( char *ch );
	int skip_number( char *ch );
	int skip_blank( char *ch );
	int skip_ntdt_itd( char *ch, int in1 );
	double gets_sss( char *ch, int in1, int mod1);
	double gets_f1( char *ch );
	void read_RT_output_signal(int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint, double *dFr);
	int CalcNGraphExpession( char* szExpression );
	void ReadGraph1Expession( int nGr, char *szExpression );
	void ReadGraph1( int nGr, char *szFileName, int *nPoint, double **pFreq, _complex **pSmatr );
	int CalcNDirInPathName( char  *ch);
	int SelectDirName( char *ch );
	void MakeRelatPathSFile( char *szSFileName );
	FILETIME LastWriteTime( char *szFileName );
	void SFileNameToDatFileName( char *lpszDatFN);
	BOOL IsDocFileName( char *ch);
	void MakeDocFileName( void );
	void del_bl3( char *ch );
	void ReadGraph( int nGr );
	void del_bl2( char *);
	int ReadNGraph( void );
	void put_error_messege(char *error);
	PROCESS_INFORMATION piProcInfo;
	void CreatProc(char *lpszNameExe);
};

/////////////////////////////////////////////////////////////////////////////
