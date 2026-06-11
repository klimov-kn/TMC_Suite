// TMCGROUTDoc.h : interface of the CTMCGROUTDoc class
//
/////////////////////////////////////////////////////////////////////////////
#include <tmcgrviw.h>
#include <proc_s.h>

typedef struct
	{
	   BOOL		 bIsReduceElMagnAnalysis; //  
	   int		 nPoint;	// number of point
	   double	 dFreq;		// frequence for time array
	   double	*pFreq;		// frequence array
	   _complex *pSmatr;	// point for complex elem Smatrix 
	   CTMCGrExpression *pcExpr;// pointer of expression class 
	   int		 nInp1;		// numer for input 1
	   int		 nMod1;		// moda for input 1
	   int		 nInp2;		// numer for input 2
	   int		 nMod2;		// moda for input 2
	   char		*szFileName;// s matrix FileName with all path
	   char		*szGrapName;// graphics	S FileName
	   char		*szGrapPodp;// graphics	Podp
	   int		 LineColor;	// line color	 how draw line
	   int		 LineType;	// line type
	   int		 LineWidth;	// line width
       TMC_INT_POINT   piPoint[TMC_GROUT_POINT_TYPE];// how draw point
	   int		 PointColor;// point color
	   int		 PointType;	// point type
	   int		 PointWidth;// point width
	   int		 OutFlag;	// Flag for output or not graphics on view
	   FILETIME	 LastWriteTime;	// last time when S-matrix file was written
//	   c2DArray  cDirectionalPattern;
	} TMC_GR_DOC1_DN;

typedef struct
	{
		int			 nGraph; // number of grahics in this document
		TMC_GR_DOC1_DN *psGraph;// pointer for grahics array		  
		BOOL		 nAFlagX;	//if nAFlagX = 0  - X; if = 1 - X1  
		BOOL		 nAFlagY;	//if nAFlagY = 0  - Y; if = 1 - Y1  
		BOOL 		 PointDrawFlag;	//draw or not point in graphics
		int			 nYType;		//graphics type	Y-coord
		int			 nXType;		//graphics type	X-coord
		double		 Xmin;			//
		double		 Xmax;			//
		double		 Ymin;			//
		double		 Ymax;			//
		FILETIME	 LastWriteTime;	// last time when document file was written
	} TMC_GR_DOC_DN;



class CTMCGROUTDoc : public CDocument
{
protected: // create from serialization only
	CTMCGROUTDoc();
	DECLARE_DYNCREATE(CTMCGROUTDoc)

// Attributes
public:
	c2DArray * pcDirectionalPattern;
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
	TMC_GR_DOC_DN grdoc;
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
	int nGraphCurrent;
	void ReadDocFile_1( void );
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
	void ReadGraphCangeAngle( int nGr );
	void SetDefaultLineColorWidthType( void );
	void SetOneReadGraphParameters( char *ch );
	int skip_number( char *ch );
	int skip_blank( char *ch );
	int skip_ntdt_itd( char *ch, int in1 );
	double gets_sss( char *ch, int in1, int mod1);
	double gets_f1( char *ch );
	void read_RT_output_signal( int n_Gr1, int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint, double *dFr);
	void read_RT_output_signal_CangeAngle( int n_Gr1, int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint, double *dFr);
	int CalcNGraphExpession( char* szExpression );
	void ReadGraph1Expession( int nGr, char *szExpression );
	void ReadGraph1( int nGr, char *szFileName, int *nPoint, double **pFreq, _complex **pSmatr );
	void ReadGraph1_CangeAngle( int nGr, char *szFileName, int *nPoint, double **pFreq, _complex **pSmatr );
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
