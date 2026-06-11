/*
**  TAMIC_soft(R) 1999 v2.00
**
** Macros and decloration for  TAMIC graphics programm
**
** FILE: ..\include\tmc3dgr.h
**
*/

#ifndef _TMC_GRPH_WIN_H
  #define _TMC_GRPH_WIN_H
#include <typedef.h>
#include <TMCGrExpression.h>

			//type graphics - KSV
#define TMC_GR_TYPE_WSVR 0
			//type graphics - L db
#define TMC_GR_TYPE_L    1
			//type graphics - S modul
#define TMC_GR_TYPE_SM   2
			//type graphics - S phase in radian
#define TMC_GR_TYPE_SFR  3
			//type graphics - S phase in gradus
#define TMC_GR_TYPE_SFD  4

			//type graphics - radio impuls
#define TMC_GROTS_TYPE_FULL  0
			//type graphics - only amplituda
#define TMC_GROTS_TYPE_AMPL  1
			//type graphics - only phase 
#define TMC_GROTS_TYPE_PHAS_R  2
			//type graphics - only phase 
#define TMC_GROTS_TYPE_PHAS_G  3
			//type graphics - only KSV
#define TMC_GROTS_TYPE_WSVR  4
			//type graphics - only L dB
#define TMC_GROTS_TYPE_LDB   5
			//type graphics - only delta phase
#define TMC_GROTS_TYPE_PHASD 6

			//type graphics - X-axies
#define TMC_GR_TYPE_FGHz 0
#define TMC_GR_TYPE_FMHz 1
#define TMC_GR_TYPE_FKHz 2
#define TMC_GR_TYPE_FHz  3
#define TMC_GR_TYPE_F_USER  4

#define TMC_GR_TYPE_PRad    0
#define TMC_GR_TYPE_PGradus 1

#define TMC_GROTS_DOCFILE_ID				"#TMCGROTS"
#define TMC_GROTS_DOCFILE_XAXIESFORMAT		"#TMCGROTS_X_AXIES_FORMAT"
#define TMC_GROTS_DOCFILE_YAXIESFORMAT		"#TMCGROTS_Y_AXIES_FORMAT"
#define TMC_GROTS_DOCFILE_POINTSIZE			"#TMCGROTS_POINTSIZE"
#define TMC_GROTS_DOCFILE_ANGLE_TETA			"#TMCGROTS_ANGLE_TETA"
			//type graphics - X-axies
#define TMC_GROTS_TYPE_nT   0
#define TMC_GROTS_TYPE_s    5
#define TMC_GROTS_TYPE_ms   4
#define TMC_GROTS_TYPE_mks  3
#define TMC_GROTS_TYPE_ns   2
#define TMC_GROTS_TYPE_ps   1

#define TMC_GROTS_FILEOUT_ID		"#TMC_GraphicsOutputSignalFile FormVer2.0 2000 "
#define TMC_GROTS_FLOUTFreq_ID		"#TMC_GROTS_Freq "
#define TMC_GROTS_FLOUTInpNum_ID	"#TMC_GROTS_InpNum "

#define TMC_RTH_FILETOP_ID		"#TMC_TopologyOutputFile FormVer2.0 2000"

#define TMC_VIEW_XSIZE 18000
#define TMC_VIEW_YSIZE 18000
#define TMC_VIEW_SIZEMODE MM_HIMETRIC

#define TMC_GROUT_MAXSTRING_BUF 10000

#define TMC_GROUT_POINT_TYPE   10
#define TMC_GROUT_POINT_TYPE_X 400
#define TMC_GROUT_POINT_TYPE_Y 400

#define TMCFIELDVIEW_2DDraw 0
#define TMCFIELDVIEW_3DDraw 1

#define TMC_GROUT_DOCFILE_ID				"#TMCGROUT"
#define TMC_GROUT_DOCFILE_IDP				"#TMC_PHAZEPORTRATE_VIEWER_AND_SOLVER"
#define TMC_GROUT_DOCFILE_IDPar				"#PhazePortrateParameters"
#define TMC_GROUT_DOCFILE_ID_PointDrawFlag	"#PointDrawFlag"
#define TMC_GROUT_DOCFILE_ID_nXType			"#nXType"
#define TMC_GROUT_DOCFILE_ID_nYType			"#nYType"
#define TMC_GROUT_DOCFILE_ID_Xmin			"#Xmin"
#define TMC_GROUT_DOCFILE_ID_Xmax			"#Xmax"
#define TMC_GROUT_DOCFILE_ID_Ymin			"#Ymin"
#define TMC_GROUT_DOCFILE_ID_Ymax			"#Ymax"
#define TMC_GROUT_DOCFILE_ID_nAFlagX		"#nAFlagX"
#define TMC_GROUT_DOCFILE_ID_nAFlagY		"#nAFlagY"
#define TMC_GROUT_DOCFILE_ID_INIWND			"#INIWND"
#define TMC_GROUT_DOCFILE_ID_INIWNDVWPRT	"#INIWNDVIEWPORT"
#define TMC_GROUT_DOCFILE_ID_dXAxiesUnit	"#dXAxiesUnit"
#define TMC_GROUT_DOCFILE_ID_dXAxiesName	"#dXAxiesName"
#define TMC_GROUT_DOCFILE_ID_nPointForNorm	"#nPointForNormirovka"

#define TMC_GROFLD_DOCFILE_ID				"#TMC_GraphicsOutputFieldFile FormVer2.0 2000 "
#define TMC_GROFLD_DOCFILE_ID_nT			"#TMCGROFLD_nT "
#define TMC_GROFLD_DOCFILE_ID_T				"#TMCGROFLD_Time "
#define TMC_GROFLD_DOCFILE_ID_Delta			"#TMCGROFLD_Delta "
#define TMC_GROFLD_DOCFILE_ID_Xmin			"#TMCGROFLD_Xmin "
#define TMC_GROFLD_DOCFILE_ID_Ymin			"#TMCGROFLD_Ymin "
#define TMC_GROFLD_DOCFILE_ID_Accuracy		"#TMCGROFLD_Accuracy "
#define TMC_GROFLD_DOCFILE_ID_LongUnit		"#TMCGROFLD_LongUnit "
#define TMC_GROFLD_DOCFILE_ID_TimeUnit		"#TMCGROFLD_TimeUnit "
#define TMC_GROFLD_DOCFILE_ID_nX			"#TMCGROFLD_nX "
#define TMC_GROFLD_DOCFILE_ID_nY			"#TMCGROFLD_nY "

#define TMC_TOPVIEW_DOCFILE_ID					"#TMCTOPVIEW"
#define TMC_TOPVIEW_DOCFILE_ID_EDITORFLNM		"#EditorFileName"
#define TMC_TOPVIEW_DOCFILE_ID_TPLFILENAME		"#TplFileName"
#define TMC_TOPVIEW_DOCFILE_ID_NSTEP			"#nStep"
#define TMC_TOPVIEW_DOCFILE_ID_NSTEPCTRL		"#nStepControlFlag"
#define TMC_TOPVIEW_DOCFILE_ID_DrawTopFlag		"#DrawTopologyFlag"
#define TMC_TOPVIEW_DOCFILE_ID_nDrawSurfBlend	"#nDrawSurfBlend"
#define TMC_TOPVIEW_DOCFILE_ID_bDrawLineFlag	"#bDrawLineFlag"
#define TMC_TOPVIEW_DOCFILE_ID_bDrawSurfInFlag	"#bDrawSurfInFlag"
#define TMC_TOPVIEW_DOCFILE_ID_bDrawSurfXXFlag	"#bDrawSurfXXFlag"
#define TMC_TOPVIEW_DOCFILE_ID_bDrawSurfKZFlag	"#bDrawSurfKZFlag"
#define TMC_TOPVIEW_DOCFILE_ID_nDrawSurfBlendInp "#nDrawSurfBlendInp"
#define TMC_TOPVIEW_DOCFILE_ID_nDrawSurfBlendKz	"#nDrawSurfBlendKz"
#define TMC_TOPVIEW_DOCFILE_ID_nDrawSurfBlendXx	"#nDrawSurfBlendXx"
#define TMC_TOPVIEW_DOCFILE_ID_bDrawTextInpFlag	"#bDrawTextInpFlag"
#define TMC_TOPVIEW_DOCFILE_ID_bDrawTextSzFlag	"#bDrawTextSzFlag"
#define TMC_TOPVIEW_DOCFILE_ID_sDrawTextFont	"#sDrawTextFont"
#define TMC_TOPVIEW_DOCFILE_ID_sDrawTextColor	"#sDrawTextColor"
#define TMC_TOPVIEW_DOCFILE_ID_sDrawPointColor	"#sDrawPointColor"
#define TMC_TOPVIEW_DOCFILE_ID_sDrawAxiesColor	"#sDrawAxiesColor"
#define TMC_TOPVIEW_DOCFILE_ID_sDrawGridColor	"#sDrawGridColor"
#define TMC_TOPVIEW_DOCFILE_ID_sDrawBckgrdColor	"#sDrawBckgrdColor"

#define CTMCRTH_INDANBLCK_RECTSTAT	  "RECT_STAT "
#define CTMCRTH_INDANBLCK_CIRCSTAT	  "CIRCLE_STAT "
#define CTMCRTH_INDANBLCK_POLYGSTT	  "POLYGON_STAT "
#define CTMCRTH_INDANBLCK_RECTSTATN	  "RECT_STAT_N "
#define CTMCRTH_INDANBLCK_CIRCSTATN	  "CIRCLE_STAT_N "
#define CTMCRTH_INDANBLCK_POLYGSTTN	  "POLYGON_STAT_N "
#define CTMCRTH_INDANBLCK_RECTSTATB	  "RECT_STAT_B "
#define CTMCRTH_INDANBLCK_CIRCSTATB	  "CIRCLE_STAT_B "
#define CTMCRTH_INDANBLCK_POLYGSTTB	  "POLYGON_STAT_B "
#define CTMCRTH_INDANBLCK_RECTSTATY	  "RECT_STAT_Y "
#define CTMCRTH_INDANBLCK_CIRCSTATY	  "CIRCLE_STAT_Y "
#define CTMCRTH_INDANBLCK_POLYGSTTY	  "POLYGON_STAT_Y "
#define CTMCRTH_INDANBLCK_POLGLINE	  "L "
#define CTMCRTH_INDANBLCK_RECTMOVE	  "RECT_MOVE "
#define CTMCRTH_INDANBLCK_CIRCMOVE	  "CIRCLE_MOVE "
#define CTMCRTH_INDANBLCK_POLYGMOV	  "POLYGON_MOVE "
#define CTMCRTH_INDANBLCK_INPUT_X	  "INPUT_X "
#define CTMCRTH_INDANBLCK_INPUT_Y	  "INPUT_Y "
#define CTMCRTH_INDANBLCK_FILE		  "FILE "
#define CTMCRTH_INDANBLCK_FILEN		  "FILE_N "
#define CTMCRTH_INDANBLCK_FILEB		  "FILE_B "
#define CTMCRTH_INDANBLCK_FILEY		  "FILE_Y "
#ifdef _MODEL_RTE__
	#define CTMCRTH_INDANBLCK_TYPEMET	  "MAGNETIC"
	#define CTMCRTH_INDANBLCK_TYPEMAG	  "METAL"
#else
	#define CTMCRTH_INDANBLCK_TYPEMET	  "METAL"
	#define CTMCRTH_INDANBLCK_TYPEMAG	  "MAGNETIC"
#endif
#define CTMCRTH_INDANBLCK_TYPEABS	  "ABSORBER"

//#define CTMCRTH_BLCKNTYPE_EPSLE0	   0
//#define CTMCRTH_BLCKNTYPE_EPSGT0	   0
#define CTMCRTH_BLCKNTYPE_EPS		   0
#define CTMCRTH_BLCKNTYPE_MAGNETIC	 107
#define CTMCRTH_BLCKNTYPE_METAL		 101
#define CTMCRTH_BLCKNTYPE_INPXLEFT	 102
#define CTMCRTH_BLCKNTYPE_INPXRIGHT	 103
#define CTMCRTH_BLCKNTYPE_INPYTOP	 104
#define CTMCRTH_BLCKNTYPE_INPYBOT	 105
#define CTMCRTH_BLCKNTYPE_ABSORBER	 100
#define CTMCRTH_BLCKNTYPE_ABSR_0000	  16
#define CTMCRTH_BLCKNTYPE_ABSR_0001	   1
#define CTMCRTH_BLCKNTYPE_ABSR_0010	   2
#define CTMCRTH_BLCKNTYPE_ABSR_0011	   3
#define CTMCRTH_BLCKNTYPE_ABSR_0100	   4
#define CTMCRTH_BLCKNTYPE_ABSR_0101	   5
#define CTMCRTH_BLCKNTYPE_ABSR_0110	   6
#define CTMCRTH_BLCKNTYPE_ABSR_0111	   7
#define CTMCRTH_BLCKNTYPE_ABSR_1000	   8
#define CTMCRTH_BLCKNTYPE_ABSR_1001	   9
#define CTMCRTH_BLCKNTYPE_ABSR_1010	  10
#define CTMCRTH_BLCKNTYPE_ABSR_1011	  11
#define CTMCRTH_BLCKNTYPE_ABSR_1100	  12
#define CTMCRTH_BLCKNTYPE_ABSR_1101	  13
#define CTMCRTH_BLCKNTYPE_ABSR_1110	  14
#define CTMCRTH_BLCKNTYPE_ABSR_1111	  15
#define CTMCRTH_BLCKNTYPE_ABSR_1111_1 21
#define CTMCRTH_BLCKNTYPE_ABSR_1111_2 22
#define CTMCRTH_BLCKNTYPE_ABSR_1111_3 23
#define CTMCRTH_BLCKNTYPE_ABSR_1111_4 24

#define TMC_DNOUT_DOCFILE_ID		"#TMCDNOUT"

#define TMC_TMCE_ID		"#TMC_E"
#define TMC_TMCH_ID		"#TMC_H"
#define TMC_TMCW_ID		"#TMC_W"
#define TMC_TMCM_ID		"#TMC_M"
#define TMC_TMCI_ID		"#TMC_I"

#define TMC_STEP_ID		"#STEP"
#define TMC_END_STEP_ID	"#END_STEP"

#define TMC_EOF_ID		"#EOF"

#define TMC_BLOCK_ID		"BLOCK"	 
#define TMC_END_BLOCK_ID	"END_B"
#define TMC_END_GP_ID		"END_GP"

#define TMC_TOPOLOGY_ID		"#TOPOLOGY"
#define TMC_END_TOPOLOGY_ID	"#END_TOPOLOGY"

#define TMC_LINK_LIST_ID	"#LINK_LIST"
#define TMC_LINK_LIST_DSC	"T "
#define TMC_END_LINK_ID		"#END_LINK"

#define TMC_OUTPUT_ID		"#OUTPUT"
#define TMC_END_OUTPUT_ID	"#END_OUTPUT"

#define TMC_COMMENT_ID		"#COMMENT"
#define TMC_END_COMMENT_ID	"#END_COMMENT"
				   // surface type for kz or xx
#define TMC_SURF_MET 0
#define TMC_SURF_MUG 1
				   // input type 
#define TMC_INPUT_E 0


typedef struct
	{ 
		 double x;				
		 double y;
	} TMC_POINT;

typedef struct
	{ 
		 int nX;
		 int nY;
	} TMC_INT_POINT;

typedef struct
	{
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
	} TMC_GR_DOC1;

typedef struct
	{
		int			 nGraph; // number of grahics in this document
		TMC_GR_DOC1 *psGraph;// pointer for grahics array		  
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
	} TMC_GR_DOC;

typedef struct
	{
	   int nPoint;			// number of point
	   int		 nPointDraw;// number of drawing point 
           TMC_POINT       *pPoint;	//   _real point in graphics
           TMC_INT_POINT   *piPoint;// logical point in graphics
	} TMC_GR_VIEW;

typedef struct
	{ 
		 int nXmin;
		 int nXmax;
		 int nYmin;
		 int nYmax;
	} TMC_GR_VIEWPORT;

typedef struct
	{ 
		double Xmin;
		double Xmax;
		double Ymin;
		double Ymax;
//		double X1min;
//		double X1max;
//		double Y1min;
//		double Y1max;
		BOOL	nAFlagX;	//if nAFlagX = 0  - X; if = 1 - X1  
		BOOL	nAFlagY;	//if nAFlagY = 0  - Y; if = 1 - Y1  
	} TMC_GR_WINDOW_D;

typedef struct
	{ 
		TMC_GR_WINDOW_D   sGrWin;//structure _real window
		TMC_GR_VIEWPORT sGrView;//structure int viewport
		int			 nYType;		//graphics type	Y-coord
		int			 nXType;		//graphics type	X-coord
		int 		 nGraph;		//number of graphics
		BOOL 		 PointDrawFlag;	//draw or not point in graphics
		TMC_GR_VIEW		 *psGraph;	//pointer for graphics array structure
	} TMC_GR_WINDOW;


typedef struct
	{ 
		int				 nType;		//graphics type
		char 	 pszYname[100];		//Y name of graphics
	} TMC_GR_TYPE_Y;

typedef struct
	{ 
		int				 nType;		//graphics type
		char 	 pszXname[100];		//Y name of graphics
	} TMC_GR_TYPE_X;

void RunExeFileG( char *lpszNameExe, char *lpszArgv );
void CreatProcG(char * lpszComLine, char *lpszDirectory);
void MakeRelatPathFile( char *szFileName, char *szWorkDir );
void MakeRelatPathFile( char *szFileName, CString szWorkDir );
int SelectDirName( char *ch );
int CalcNDirInPathName( char  *ch);
void MakeFullPathFile( char *szFileName, char *szWorkDir );
void MakeFullPathFile( char *szFileName, CString szWorkDir );

#endif
