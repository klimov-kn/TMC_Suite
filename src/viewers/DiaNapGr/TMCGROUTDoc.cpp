// TMCGROUTDoc.cpp : implementation of the CTMCGROUTDoc class
//
#include "stdafx.h"
#include <io.h>
#include <fcntl.h>
#include "TMCGROUT.h"

#include "..\\DiaNapr\\c2DArray.h"
#include "TMCGROUTDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTDoc

IMPLEMENT_DYNCREATE(CTMCGROUTDoc, CDocument)

BEGIN_MESSAGE_MAP(CTMCGROUTDoc, CDocument)
	//{{AFX_MSG_MAP(CTMCGROUTDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTDoc construction/destruction

CTMCGROUTDoc::CTMCGROUTDoc()
{
	nGraphCurrent = 0;
	piProcInfo.hProcess = NULL;
	dFreq = 1e+10;
	// TODO: add one-time construction code here
	grdoc.nGraph = 0;
	grdoc.psGraph  = NULL;
	error[0] = '\0';
	grdoc.PointDrawFlag = FALSE;
	grdoc.nXType  = TMC_GROTS_TYPE_nT;
	grdoc.nYType  = TMC_GROTS_TYPE_FULL;
	grdoc.Xmin = 1.0;
	grdoc.Xmax = 10.0;
	grdoc.Ymin = 1.0;
	grdoc.Ymax = 1.2;
	grdoc.nAFlagX = TRUE;
	grdoc.nAFlagY = TRUE;
	grdoc.LastWriteTime.dwLowDateTime   = 0;
	grdoc.LastWriteTime.dwHighDateTime  = 0;
	csEditorName.Format( "winword.exe" );       // font attribute
	strcpy( lfInitial.lfFaceName, "Times New Roman" );
	lfInitial.lfHeight     = 0;
	lfInitial.lfWidth      = 0;
	lfInitial.lfEscapement = 0;
	lfInitial.lfOrientation= 0;
	lfInitial.lfWeight     = 400;
	lfInitial.lfItalic     = FALSE;
	lfInitial.lfUnderline  = FALSE;
	lfInitial.lfStrikeOut  = FALSE;
	lfInitial.lfQuality    = PROOF_QUALITY;
	lfInitial.lfCharSet       = ANSI_CHARSET;
	lfInitial.lfOutPrecision  = OUT_DEFAULT_PRECIS;
	lfInitial.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lfInitial.lfPitchAndFamily= DEFAULT_PITCH | FF_SWISS;
	scTextColor = RGB(   0, 0, 0); // text color 
	scPointColor= RGB( 255, 0, 0); // point color 
	scAxiesColor= RGB(   0, 0, 0); // axies color
	scGridColor = RGB(   0, 0, 0); // grid color 
	scBackgoundColor= RGB(   255, 255, 255); // backgound color;
	bLossPoint = TRUE;
	SetDefaultLineColorWidthType();
	pcDirectionalPattern = NULL;
}

CTMCGROUTDoc::~CTMCGROUTDoc()
{
	piProcInfo.hProcess = NULL;
	DocFileDelData(); // delete old data and free memory
}

BOOL CTMCGROUTDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTDoc serialization

void CTMCGROUTDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTDoc diagnostics

#ifdef _DEBUG
void CTMCGROUTDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTMCGROUTDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTMCGROUTDoc commands




void CTMCGROUTDoc::CreatProc(char * lpszComLine)
{
	char szCurrentDir[TMC_GROUT_MAXSTRING_BUF];
	int i;

	strcpy( szCurrentDir, GetPathName() );
	for( i = strlen(szCurrentDir)-1; i >= 0; i-- )
	{
		if( szCurrentDir[i] == '\\' )
		{
			szCurrentDir[i] = '\0';
			i = -1;
		};
	};

   if( piProcInfo.hProcess != NULL ) 
   {
//	TerminateProcess( piProcInfo.hProcess, 0 );
	piProcInfo.hProcess = NULL;
   }; 
   STARTUPINFO siStartInfo; 
// Set up members of STARTUPINFO structure. 
   ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
   siStartInfo.cb = sizeof(STARTUPINFO); 
// Create the child process. 
   CreateProcess(NULL, 
      lpszComLine,       // command line 
      NULL,          // process security attributes 
      NULL,          // primary thread security attributes 
      TRUE,          // handles are inherited 
      0,             // creation flags 
      NULL,          // use parent's environment 
      szCurrentDir,          // use parent's current directory 
      &siStartInfo,  // STARTUPINFO pointer 
      &piProcInfo);  // receives PROCESS_INFORMATION 
   return ;

}

void CTMCGROUTDoc::RunExeFile( char *lpszNameExe )
{
	char szCommandLine[TMC_GROUT_MAXSTRING_BUF];
	int i;
	CString theString;

	strcpy( szCommandLine, lpszNameExe);
	i = strlen( szCommandLine );

	theString = GetPathName();
	if( strlen(theString) == 0 )
	{
		theString = GetTitle();
	};
	
	szCommandLine[i] = ' ';
	strcpy( szCommandLine + i + 1, theString);

	for( i = 0; ( i < TMC_GROUT_MAXSTRING_BUF )&&( szCommandLine[i] == ' ' ); i++)
	{
		;
	}

	CreatProc( szCommandLine + i );
}


void CTMCGROUTDoc::ReadDocFile(void)
{
	int n, i;

	MakeDocFileName();

	DocFileDelData(); // delete old data and free memory
	
	if( (n = ReadNGraph()) <= 0 )
	{
	 put_error_messege("number of graphic = 0");
	 return;
	};

	if( !DocFileNewData( n ) ) return; //allocation memory for new data and set standart data

	for( i = 0; i < n; i++)		
	{
		ReadGraph( i );
	};

 IsDataModific();
 return;
	
}

void CTMCGROUTDoc::put_error_messege(char *error1)
{
  strcpy( error, error1);
}

int CTMCGROUTDoc::ReadNGraph( void )
{
 FILE *fd;
 int i;
 char ch[TMC_GROUT_MAXSTRING_BUF];

 strcpy( ch, GetPathName());
 if( ch[0] == '\0' ) return -1;
 if( (fd = fopen(GetPathName(), "r")) == NULL ) return -1;

 for(i = 0; fgets(ch, TMC_GROUT_MAXSTRING_BUF, fd) != NULL ; )
 {
	 del_bl2(ch);
	 if( (ch[0] != '\0')&&(ch[0] != '#') ) i++;
 };
 fclose(fd);
 return i;

}

void CTMCGROUTDoc::del_bl2( char *ch)
{
	int i, i1, j, j1;

	for( i = 0, i1 = 0, j = 0, j1 = 0; (i < TMC_GROUT_MAXSTRING_BUF)&&( ch[ i ] != '\0' ) ; i++)
	{
		switch( ch[i] )
		{
		case ' ' : ;
		case '\t' : ;
		case '\n' :
			ch[i] = ' ';
			if( i1 == 0 ) break;
			if( j1 != 0)
			{
				ch[i1] = ch[i];
				i1++;
			}
			else
			{
				if( j == 0 )
				{
				j = 1;
				ch[i1] = ch[i];
				i1++;
			    }; 
			};
			break;
		case '[' :
			ch[i1] = ch[i];
			i1++;
			j1++;
			j = 0;
			break;
		case ']' :
			ch[i1] = ch[i];
			i1++;
			j1--;
			j = 0;
			break;
		case '!' :
			if( i1 == 0 )
			ch[i1] = '\0';
			i1++;
			j = 0;
			return ;
		default :
			ch[i1] = ch[i];
			i1++;
			j = 0;
			break;
		}
		
	};
	if( i1 != 0 ) i1--;
	ch[i1] = '\0';
	return ;
}

void CTMCGROUTDoc::ReadGraph( int nGr )
{
 FILE *fd;
 int i;
 char ch[TMC_GROUT_MAXSTRING_BUF];

 strcpy( grdoc.psGraph[nGr].szFileName, GetPathName() );
 for( i = strlen(grdoc.psGraph[nGr].szFileName)-1; i >= 0; i-- )
 {
 	if( grdoc.psGraph[nGr].szFileName[i] == '\\' )
	{
		grdoc.psGraph[nGr].szFileName[i] = '\0';
		i = -1;
	};
 };

 if( (fd = fopen(GetPathName(), "r")) == NULL )
 {
	 sprintf( ch, "when open file %s", GetPathName());
	 put_error_messege(ch);
	 return ;
 };

 for(i = 0; i <= nGr ; )
 {
	 if(fgets(ch, TMC_GROUT_MAXSTRING_BUF, fd) == NULL)
	 {
		 sprintf( ch, "when read line number %d in data file %s", i, GetPathName());
		 put_error_messege(ch);
		 fclose(fd);
		 return;
	 };
	 del_bl2(ch);
	 if( (ch[0] != '\0')&&(ch[0] != '#') ) i++;
 };
 fclose(fd);

if( sscanf( ch,"%s %s %d %d %d %d %d",grdoc.psGraph[nGr].szGrapName,
									  grdoc.psGraph[nGr].szGrapPodp,
									&(grdoc.psGraph[nGr].OutFlag),
									&(grdoc.psGraph[nGr].nInp1),
									&(grdoc.psGraph[nGr].nMod1),
									&(grdoc.psGraph[nGr].nInp2),
									&(grdoc.psGraph[nGr].nMod2) ) != 7 ) 
{
	if( sscanf( ch, "%s %s %d %d %d %d", grdoc.psGraph[nGr].szGrapName,
										  grdoc.psGraph[nGr].szGrapPodp,
										&(grdoc.psGraph[nGr].nInp1),
										&(grdoc.psGraph[nGr].nMod1),
										&(grdoc.psGraph[nGr].nInp2),
										&(grdoc.psGraph[nGr].nMod2) ) != 6 ) 
	{
	
		if( sscanf( ch, "%s %d %d %d %d",  grdoc.psGraph[nGr].szGrapName,
											&(grdoc.psGraph[nGr].nInp1),
											&(grdoc.psGraph[nGr].nMod1),
											&(grdoc.psGraph[nGr].nInp2),
											&(grdoc.psGraph[nGr].nMod2) ) != 5 )
		{
			if( sscanf( ch, "%s",  grdoc.psGraph[nGr].szGrapName) != 1 )
			{
				 sprintf( ch, "when read line number %d in data file %s", nGr, GetPathName());
				 put_error_messege(ch);
				 return ;
			};
			grdoc.psGraph[nGr].nInp1 = 1;
			grdoc.psGraph[nGr].nMod1 = 1;
			grdoc.psGraph[nGr].nInp2 = 1;
			grdoc.psGraph[nGr].nMod2 = 1; 
		};
		strcpy( grdoc.psGraph[nGr].szGrapPodp, grdoc.psGraph[nGr].szGrapName);
	};
	grdoc.psGraph[nGr].OutFlag = TRUE;
};

if( (grdoc.psGraph[nGr].szGrapName[0] == '{')&&(grdoc.psGraph[nGr].szGrapName[strlen(grdoc.psGraph[nGr].szGrapName)-1] == '}') )
{
	ReadGraph1Expession( nGr, grdoc.psGraph[nGr].szGrapName );
}
else
{
	ReadGraph1( nGr,
				grdoc.psGraph[nGr].szGrapName,
				&(grdoc.psGraph[nGr].nPoint),
				&(grdoc.psGraph[nGr].pFreq),
				&(grdoc.psGraph[nGr].pSmatr));
};

return ;
}



void CTMCGROUTDoc::DocFileDelData()	  //delete old data and free memory
{
	int i;
	nGraphCurrent = 0;

	error[0] = '\0';
	if(grdoc.psGraph != NULL)
	{
		for( i = 0; i < grdoc.nGraph; i++)
		{
			if( grdoc.psGraph[i].pFreq  != NULL)	 delete grdoc.psGraph[i].pFreq;
			if( grdoc.psGraph[i].pSmatr != NULL)	 delete grdoc.psGraph[i].pSmatr;
			if( grdoc.psGraph[i].szFileName != NULL) delete grdoc.psGraph[i].szFileName;
			if( grdoc.psGraph[i].szGrapName != NULL) delete grdoc.psGraph[i].szGrapName;
			if( grdoc.psGraph[i].szGrapPodp != NULL) delete grdoc.psGraph[i].szGrapPodp;
			if( grdoc.psGraph[i].pcExpr     != NULL) delete grdoc.psGraph[i].pcExpr;
		};
		delete grdoc.psGraph;
		grdoc.psGraph = NULL;
	}

	if( pcDirectionalPattern != NULL )
	{
		delete []pcDirectionalPattern;
		pcDirectionalPattern = NULL;
	};

	grdoc.nGraph = 0;
}

BOOL CTMCGROUTDoc::DocFileNewData(int n) //allocation memory for new data and set standart data
{
	int i, i1;

	pcDirectionalPattern = new	c2DArray[ n ];

	grdoc.psGraph = new	TMC_GR_DOC1_DN[ n ];
	if( grdoc.psGraph == NULL )
	{
	 put_error_messege("fatal - not memory for graphics array");
	 return FALSE;
	};
	grdoc.nGraph = n;

	for( i = 0, i1 = 0; i < grdoc.nGraph; i++, i1++)
	{
		if( i1 == 16 ) i1 = 0;
		grdoc.psGraph[i].nPoint = sGrDoc[i1].nPoint;
		grdoc.psGraph[i].pFreq  = sGrDoc[i1].pFreq;
		grdoc.psGraph[i].pSmatr = sGrDoc[i1].pSmatr;
		grdoc.psGraph[i].pcExpr = sGrDoc[i1].pcExpr;
		grdoc.psGraph[i].nInp1  = sGrDoc[i1].nInp1;
		grdoc.psGraph[i].nMod1  = sGrDoc[i1].nMod1;
		grdoc.psGraph[i].nInp2  = sGrDoc[i1].nInp2;
		grdoc.psGraph[i].nMod2  = sGrDoc[i1].nMod2;
		grdoc.psGraph[i].szFileName = new char[300];
		grdoc.psGraph[i].szGrapName = new char[300];
		grdoc.psGraph[i].szGrapPodp = new char[300];
		grdoc.psGraph[i].LastWriteTime.dwLowDateTime   = sGrDoc[i1].LastWriteTime.dwLowDateTime;
		grdoc.psGraph[i].LastWriteTime.dwHighDateTime  = sGrDoc[i1].LastWriteTime.dwHighDateTime;
		grdoc.psGraph[i].OutFlag = sGrDoc[i1].OutFlag;
		
		grdoc.psGraph[i].LineColor = sGrDoc[i1].LineColor;
		grdoc.psGraph[i].piPoint[0].nX = sGrDoc[i1].piPoint[0].nX;
		grdoc.psGraph[i].piPoint[0].nY = sGrDoc[i1].piPoint[0].nY;
		grdoc.psGraph[i].piPoint[1].nX = sGrDoc[i1].piPoint[1].nX;
		grdoc.psGraph[i].piPoint[1].nY = sGrDoc[i1].piPoint[1].nY;
		grdoc.psGraph[i].piPoint[2].nX = sGrDoc[i1].piPoint[2].nX;
		grdoc.psGraph[i].piPoint[2].nY = sGrDoc[i1].piPoint[2].nY;
		grdoc.psGraph[i].piPoint[3].nX = sGrDoc[i1].piPoint[3].nX;
		grdoc.psGraph[i].piPoint[3].nY = sGrDoc[i1].piPoint[3].nY;
		grdoc.psGraph[i].piPoint[4].nX = sGrDoc[i1].piPoint[4].nX;
		grdoc.psGraph[i].piPoint[4].nY = sGrDoc[i1].piPoint[4].nY;
		grdoc.psGraph[i].piPoint[5].nX = sGrDoc[i1].piPoint[5].nX;
		grdoc.psGraph[i].piPoint[5].nY = sGrDoc[i1].piPoint[5].nY;
		grdoc.psGraph[i].piPoint[6].nX = sGrDoc[i1].piPoint[6].nX;
		grdoc.psGraph[i].piPoint[6].nY = sGrDoc[i1].piPoint[6].nY;
		grdoc.psGraph[i].piPoint[7].nX = sGrDoc[i1].piPoint[7].nX;
		grdoc.psGraph[i].piPoint[7].nY = sGrDoc[i1].piPoint[7].nY;
		grdoc.psGraph[i].piPoint[8].nX = sGrDoc[i1].piPoint[8].nX;
		grdoc.psGraph[i].piPoint[8].nY = sGrDoc[i1].piPoint[8].nY;
		grdoc.psGraph[i].piPoint[9].nX = sGrDoc[i1].piPoint[9].nX;
		grdoc.psGraph[i].piPoint[9].nY = sGrDoc[i1].piPoint[9].nY;

		grdoc.psGraph[i].LineType  = sGrDoc[i1].LineType;
		grdoc.psGraph[i].LineWidth = sGrDoc[i1].LineWidth;
		grdoc.psGraph[i].PointColor = sGrDoc[i1].PointColor;
		grdoc.psGraph[i].PointType  = sGrDoc[i1].PointType;
		grdoc.psGraph[i].PointWidth = sGrDoc[i1].PointWidth;
	};

	return TRUE;
}

BOOL CTMCGROUTDoc::WriteDocFile()
{
 FILE *fd;
 int i;
 char ch[TMC_GROUT_MAXSTRING_BUF];


 if( (fd = fopen(GetPathName(), "w")) == NULL )
 {
	 sprintf( ch, "when open file for write %s", GetPathName());
	 put_error_messege(ch);
	 return TRUE;
 };

 if(fprintf(fd,"%s\n!Graphics parameters\n", TMC_GROTS_DOCFILE_ID ) <= 0  )
	 {
		 sprintf( ch, "when write in data file %s", GetPathName());
		 put_error_messege(ch);
		 fclose(fd);
		 return TRUE;
	 };

 if(sprintf(ch,"%s %d; %d;\n%s %d\n%s %d\n%s %lg\n%s %lg\n%s %lg\n%s %lg\n%s %d\n%s %d\n%s %s\n%s %s;%ld;%ld;%ld;%ld;%ld;%d;%d;%d;%d;\n%s %d;\n%s %d;\n%s %d;\n%s %d;\n%s %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d;\n",
			TMC_GROUT_DOCFILE_ID_PointDrawFlag , grdoc.PointDrawFlag, bLossPoint,
			TMC_GROUT_DOCFILE_ID_nXType,	 grdoc.nXType,
			TMC_GROUT_DOCFILE_ID_nYType,	 grdoc.nYType,
			TMC_GROUT_DOCFILE_ID_Xmin,	 grdoc.Xmin,
			TMC_GROUT_DOCFILE_ID_Xmax,	 grdoc.Xmax,
			TMC_GROUT_DOCFILE_ID_Ymin,	 grdoc.Ymin,
			TMC_GROUT_DOCFILE_ID_Ymax,	 grdoc.Ymax,
			TMC_GROUT_DOCFILE_ID_nAFlagX,	 grdoc.nAFlagX,
			TMC_GROUT_DOCFILE_ID_nAFlagY,	 grdoc.nAFlagY,
			TMC_TOPVIEW_DOCFILE_ID_EDITORFLNM, csEditorName,
			TMC_TOPVIEW_DOCFILE_ID_sDrawTextFont, lfInitial.lfFaceName,
								lfInitial.lfHeight,
								lfInitial.lfWidth,
								lfInitial.lfEscapement,
								lfInitial.lfOrientation,
								lfInitial.lfWeight,
								lfInitial.lfItalic,
								lfInitial.lfUnderline,
								lfInitial.lfStrikeOut,
								lfInitial.lfQuality,
			TMC_TOPVIEW_DOCFILE_ID_sDrawTextColor, scTextColor,
			TMC_TOPVIEW_DOCFILE_ID_sDrawBckgrdColor, scBackgoundColor,
			TMC_TOPVIEW_DOCFILE_ID_sDrawAxiesColor, scAxiesColor,
			TMC_TOPVIEW_DOCFILE_ID_sDrawGridColor, scGridColor,
			TMC_TOPVIEW_DOCFILE_ID_sDrawPointColor, scPointColor,
			sGrDoc[  0 ].LineColor, sGrDoc[  0 ].LineType, sGrDoc[  0 ].LineWidth,
			sGrDoc[  1 ].LineColor, sGrDoc[  1 ].LineType, sGrDoc[  1 ].LineWidth,
			sGrDoc[  2 ].LineColor, sGrDoc[  2 ].LineType, sGrDoc[  2 ].LineWidth,
			sGrDoc[  3 ].LineColor, sGrDoc[  3 ].LineType, sGrDoc[  3 ].LineWidth,
			sGrDoc[  4 ].LineColor, sGrDoc[  4 ].LineType, sGrDoc[  4 ].LineWidth,
			sGrDoc[  5 ].LineColor, sGrDoc[  5 ].LineType, sGrDoc[  5 ].LineWidth,
			sGrDoc[  6 ].LineColor, sGrDoc[  6 ].LineType, sGrDoc[  6 ].LineWidth,
			sGrDoc[  7 ].LineColor, sGrDoc[  7 ].LineType, sGrDoc[  7 ].LineWidth,
			sGrDoc[  8 ].LineColor, sGrDoc[  8 ].LineType, sGrDoc[  8 ].LineWidth,
			sGrDoc[  9 ].LineColor, sGrDoc[  9 ].LineType, sGrDoc[  9 ].LineWidth,
			sGrDoc[ 10 ].LineColor, sGrDoc[ 10 ].LineType, sGrDoc[ 10 ].LineWidth,
			sGrDoc[ 11 ].LineColor, sGrDoc[ 11 ].LineType, sGrDoc[ 11 ].LineWidth,
			sGrDoc[ 12 ].LineColor, sGrDoc[ 12 ].LineType, sGrDoc[ 12 ].LineWidth,
			sGrDoc[ 13 ].LineColor, sGrDoc[ 13 ].LineType, sGrDoc[ 13 ].LineWidth,
			sGrDoc[ 14 ].LineColor, sGrDoc[ 14 ].LineType, sGrDoc[ 14 ].LineWidth,
			sGrDoc[ 15 ].LineColor, sGrDoc[ 15 ].LineType, sGrDoc[ 15 ].LineWidth ) <= 0  )
	 {
		 sprintf( ch, "when write in data file %s", GetPathName());
		 put_error_messege(ch);
		 fclose(fd);
		 return TRUE;
	 };
 if(fprintf(fd,"%s", ch) <= 0  )
	 {
		 sprintf( ch, "when write in data file %s", GetPathName());
		 put_error_messege(ch);
		 fclose(fd);
		 return TRUE;
	 };
// save graphics default parameters 
 if(sprintf(ch,"%s %d; %d;%s %d%s %d%s %lg%s %lg%s %lg%s %lg%s %d%s %d%s %s%s %s;%ld;%ld;%ld;%ld;%ld;%d;%d;%d;%d;%s %d;%s %d;%s %d;%s %d;%s %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d; %d;",
			TMC_GROUT_DOCFILE_ID_PointDrawFlag , grdoc.PointDrawFlag, bLossPoint,
			TMC_GROUT_DOCFILE_ID_nXType,	 grdoc.nXType,
			TMC_GROUT_DOCFILE_ID_nYType,	 grdoc.nYType,
			TMC_GROUT_DOCFILE_ID_Xmin,	 grdoc.Xmin,
			TMC_GROUT_DOCFILE_ID_Xmax,	 grdoc.Xmax,
			TMC_GROUT_DOCFILE_ID_Ymin,	 grdoc.Ymin,
			TMC_GROUT_DOCFILE_ID_Ymax,	 grdoc.Ymax,
			TMC_GROUT_DOCFILE_ID_nAFlagX,	 grdoc.nAFlagX,
			TMC_GROUT_DOCFILE_ID_nAFlagY,	 grdoc.nAFlagY,
			TMC_TOPVIEW_DOCFILE_ID_EDITORFLNM, csEditorName,
			TMC_TOPVIEW_DOCFILE_ID_sDrawTextFont, lfInitial.lfFaceName,
								lfInitial.lfHeight,
								lfInitial.lfWidth,
								lfInitial.lfEscapement,
								lfInitial.lfOrientation,
								lfInitial.lfWeight,
								lfInitial.lfItalic,
								lfInitial.lfUnderline,
								lfInitial.lfStrikeOut,
								lfInitial.lfQuality,
			TMC_TOPVIEW_DOCFILE_ID_sDrawTextColor, scTextColor,
			TMC_TOPVIEW_DOCFILE_ID_sDrawBckgrdColor, scBackgoundColor,
			TMC_TOPVIEW_DOCFILE_ID_sDrawAxiesColor, scAxiesColor,
			TMC_TOPVIEW_DOCFILE_ID_sDrawGridColor, scGridColor,
			TMC_TOPVIEW_DOCFILE_ID_sDrawPointColor, scPointColor,
			sGrDoc[  0 ].LineColor, sGrDoc[  0 ].LineType, sGrDoc[  0 ].LineWidth,
			sGrDoc[  1 ].LineColor, sGrDoc[  1 ].LineType, sGrDoc[  1 ].LineWidth,
			sGrDoc[  2 ].LineColor, sGrDoc[  2 ].LineType, sGrDoc[  2 ].LineWidth,
			sGrDoc[  3 ].LineColor, sGrDoc[  3 ].LineType, sGrDoc[  3 ].LineWidth,
			sGrDoc[  4 ].LineColor, sGrDoc[  4 ].LineType, sGrDoc[  4 ].LineWidth,
			sGrDoc[  5 ].LineColor, sGrDoc[  5 ].LineType, sGrDoc[  5 ].LineWidth,
			sGrDoc[  6 ].LineColor, sGrDoc[  6 ].LineType, sGrDoc[  6 ].LineWidth,
			sGrDoc[  7 ].LineColor, sGrDoc[  7 ].LineType, sGrDoc[  7 ].LineWidth,
			sGrDoc[  8 ].LineColor, sGrDoc[  8 ].LineType, sGrDoc[  8 ].LineWidth,
			sGrDoc[  9 ].LineColor, sGrDoc[  9 ].LineType, sGrDoc[  9 ].LineWidth,
			sGrDoc[ 10 ].LineColor, sGrDoc[ 10 ].LineType, sGrDoc[ 10 ].LineWidth,
			sGrDoc[ 11 ].LineColor, sGrDoc[ 11 ].LineType, sGrDoc[ 11 ].LineWidth,
			sGrDoc[ 12 ].LineColor, sGrDoc[ 12 ].LineType, sGrDoc[ 12 ].LineWidth,
			sGrDoc[ 13 ].LineColor, sGrDoc[ 13 ].LineType, sGrDoc[ 13 ].LineWidth,
			sGrDoc[ 14 ].LineColor, sGrDoc[ 14 ].LineType, sGrDoc[ 14 ].LineWidth,
			sGrDoc[ 15 ].LineColor, sGrDoc[ 15 ].LineType, sGrDoc[ 15 ].LineWidth ) <= 0  )
	 {
		 sprintf( ch, "when write in data file %s", GetPathName());
		 put_error_messege(ch);
		 fclose(fd);
		 return TRUE;
	 };
	AfxGetApp()->WriteProfileString( TMC_GROTS_DOCFILE_ID, TMC_GROTS_DOCFILE_ID, ch);

 if(fprintf(fd,"!Graphics data\n!file name : %s\n!Data file for draw graphics\n!TFileName GraphicsName DrawFlag Input1 Mod1 Input2 Mod2\n", GetPathName() ) <= 0  )
	 {
		 sprintf( ch, "when write data file %s", GetPathName());
		 put_error_messege(ch);
		 fclose(fd);
		 return TRUE;
	 };

 for( i = 0; i < grdoc.nGraph ; i++)
 {
	 strcpy( ch, grdoc.psGraph[i].szGrapPodp);
	 ch[strlen("Error_")] = '\0';
	 if( (grdoc.psGraph[i].szGrapPodp[0] == '\0')||( strcmp(ch, "Error_") == 0) )
	 {
		 strcpy( grdoc.psGraph[i].szGrapPodp, grdoc.psGraph[i].szGrapName);
	 };

	 del_bl3( grdoc.psGraph[i].szGrapPodp );
	 del_bl3( grdoc.psGraph[i].szGrapName );	 

	 if(fprintf(fd,"%s %s %d %d %d %d %d\n",grdoc.psGraph[i].szGrapName,
									  grdoc.psGraph[i].szGrapPodp,
									  grdoc.psGraph[i].OutFlag,
									  grdoc.psGraph[i].nInp1,
									  grdoc.psGraph[i].nMod1,
									  grdoc.psGraph[i].nInp2,
									  grdoc.psGraph[i].nMod2 ) <= 0  )
	 {
		 sprintf( ch, "when write line number %d in data file %s", i, GetPathName());
		 put_error_messege(ch);
		 fclose(fd);
		 return TRUE;
	 };
 }

 fclose(fd);
 return FALSE;
 }

void CTMCGROUTDoc::del_bl3( char *ch )
{
	int i, i1;

	for( i = 0, i1 = 0; (i < TMC_GROUT_MAXSTRING_BUF)&&( ch[ i ] != '\0' ) ; i++)
	{
		switch( ch[i] )
		{
		case ' ' : ;
		case '\t' : ;
		case '\n' :
			break;
		default :
			ch[i1] = ch[i];
			i1++;
			break;
		}
		
	};
	ch[i1] = '\0';
	return ;


}

void CTMCGROUTDoc::MakeDocFileName( void )
{
 FILE *fd;
 char ch[TMC_GROUT_MAXSTRING_BUF];
 int i;
 
						  // if File.dat is NULL
	if( strcmp( GetPathName(), "" ) == 0 )
	{					  // set File.dat as title
		// FALSE: не добавлять во MRU — GetTitle() возвращает заголовок, а не путь;
		// иначе CRecentFileList->Add бросает CInvalidArgException.
		SetPathName( GetTitle(), FALSE );
		if( (fd = fopen(GetPathName(), "w")) == NULL )
		{
			sprintf( ch, "when open file for write %s", GetPathName());
			put_error_messege(ch);
			return ;
		};

	 if(fprintf(fd,"%s\n!Graphics parameters\n", TMC_GROUT_DOCFILE_ID ) <= 0  )
		 {
			 sprintf( ch, "when write data file %s", GetPathName());
			 put_error_messege(ch);
			 fclose(fd);
			 return ;
		 };

		fclose(fd);
		return ;
	};

							// if File.dat is File.s
	strcpy( ch, GetPathName() );
	if( !IsDocFileName( ch ) )	 
	{
		char ch1[TMC_GROUT_MAXSTRING_BUF];
		strcpy( ch1, ch );
		SFileNameToDatFileName( ch1 );
		// FALSE: не добавлять рабочий путь во MRU.
		SetPathName( ch1, FALSE );
		for( i = strlen(ch)-1; i >= 0; i-- )
		{
			if( ch[i] == '\\' )	break;
		};
		i++;

		if( (fd = fopen(GetPathName(), "w")) == NULL )
		{
			sprintf( ch, "when open file for write %s", GetPathName());
			put_error_messege(ch);
			return ;
		};

	 if(fprintf(fd,"%s\n!Graphics parameters\n", TMC_GROTS_DOCFILE_ID ) <= 0  )
		 {
			 sprintf( ch, "when write line number %d in data file %s", i, GetPathName());
			 put_error_messege(ch);
			 fclose(fd);
			 return ;
		 };
  /*
	 if(fprintf(fd,"%s %d\n%s %d\n%s %d\n%s %lg\n%s %lg\n%s %lg\n%s %lg\n%s %d\n%s %d\n",
				TMC_GROUT_DOCFILE_ID_PointDrawFlag , grdoc.PointDrawFlag,
				TMC_GROUT_DOCFILE_ID_nXType,	 grdoc.nXType,
				TMC_GROUT_DOCFILE_ID_nYType,	 grdoc.nYType,
				TMC_GROUT_DOCFILE_ID_Xmin,	 grdoc.Xmin,
				TMC_GROUT_DOCFILE_ID_Xmax,	 grdoc.Xmax,
				TMC_GROUT_DOCFILE_ID_Ymin,	 grdoc.Ymin,
				TMC_GROUT_DOCFILE_ID_Ymax,	 grdoc.Ymax,
				TMC_GROUT_DOCFILE_ID_nAFlagX,	 grdoc.nAFlagX,
				TMC_GROUT_DOCFILE_ID_nAFlagY,	 grdoc.nAFlagY) <= 0  )
		 {
			 sprintf( ch, "when write line number %d in data file %s", i, GetPathName());
			 put_error_messege(ch);
			 fclose(fd);
			 return ;
		 };
	*/
	 if(fprintf(fd,"!Graphics data\n!file name : %s\n!Data file for draw graphics\n!TFileName GraphicsName DrawFlag Input1 Mod1 Input2 Mod2\n", GetPathName() ) <= 0  )
		 {
			 sprintf( ch, "when write line number %d in data file %s", i, GetPathName());
			 put_error_messege(ch);
			 fclose(fd);
			 return ;
		 };

		if(fprintf( fd,"%s %s 1 1 1 1 1\n", ch+i, ch+i ) <= 0  )
		{
			sprintf( ch, "when write in data file %s", GetPathName());
			put_error_messege(ch);
			fclose(fd);
			return ;
		};


		fclose(fd);
		return;
	};


}


void CTMCGROUTDoc::ReadGraphParameters( void )
{

 FILE *fd;
 char ch[TMC_GROUT_MAXSTRING_BUF];

 if( (fd = fopen(GetPathName(), "r")) == NULL ) return ;

 for( ; fgets( ch, TMC_GROUT_MAXSTRING_BUF, fd) != NULL ; )
 {
	 SetOneReadGraphParameters( ch );
 };

 fclose(fd);
 SetGraphColor();
 return ;
 
}

							  // Is ch - data file name for tmcgrout
BOOL CTMCGROUTDoc::IsDocFileName( char *ch)
{
	if( ch[0] == '\0' ) return FALSE;
	
	FILE *fd;
	if( (fd = fopen( ch, "r")) == NULL ) return FALSE;

	char ch1[TMC_GROUT_MAXSTRING_BUF];
	if( fgets( ch1, TMC_GROUT_MAXSTRING_BUF, fd) == NULL )
	{
		fclose(fd);
		return FALSE;
	};
	fclose(fd);
	if( strncmp( ch1, TMC_GROTS_DOCFILE_ID, strlen(TMC_GROTS_DOCFILE_ID) ) == 0 ) return TRUE;
	else
		return FALSE;

}

void CTMCGROUTDoc::SFileNameToDatFileName( char *ch)
{
	int i;

	for( i = strlen(ch); i > 0; )
		if( ch[--i] == '.' ) break;

	if( i == 0 )
	{
		strcpy( ch + strlen(ch), ".dop");
	}
	else
	{
		strcpy( ch + i + 1, "dop");
	};

	FILE *fd;
	if( ( fd = fopen( ch, "r") ) == NULL ) return ;
	fclose( fd );
	
	strcpy( ch + i + 1, "$op" ); 
	return ;
}
										// control LastWriteTime for document and S-matrix file and set there 
										// return TRUE if data was modific. FALSE or not
BOOL CTMCGROUTDoc::IsDataModific( void )
{
	char ch[TMC_GROUT_MAXSTRING_BUF];
	BOOL ret_cod = FALSE;

	strcpy( ch, GetPathName());
    FILETIME LastWriteTime1 = LastWriteTime( ch );
	
	if( CompareFileTime( &(grdoc.LastWriteTime), &LastWriteTime1) != 0 )
	{
		grdoc.LastWriteTime = LastWriteTime1;
		ret_cod = TRUE;
	};

	for( int i = 0; i < grdoc.nGraph; i++)
	{										// if this graphics drawing in window
		if( grdoc.psGraph[i].OutFlag )
		{
			LastWriteTime1 = LastWriteTime( grdoc.psGraph[i].szFileName);
			if( CompareFileTime( &(grdoc.psGraph[i].LastWriteTime), &LastWriteTime1) != 0 )
			{
				grdoc.psGraph[i].LastWriteTime = LastWriteTime1;
				ret_cod = TRUE;
			};
		};
	}

	return ret_cod;
}				 

FILETIME CTMCGROUTDoc::LastWriteTime( char *szFileName )
{
	HANDLE fh;
	FILETIME LastWriteTime1;
	LastWriteTime1.dwLowDateTime   = 0;
	LastWriteTime1.dwHighDateTime  = 0;

	fh =  CreateFile(
    szFileName,				// pointer to name of the file 
    GENERIC_READ,			// access read mode 
    FILE_SHARE_READ,		// share mode 
    NULL,					// pointer to security descriptor 
    OPEN_EXISTING,			// how to create 
    FILE_ATTRIBUTE_NORMAL,	// file attributes 
    NULL					// handle to file with attributes to copy  
   );

	GetFileTime( fh, NULL, NULL, &LastWriteTime1);

	CloseHandle( fh );
	
	return LastWriteTime1;

}

void CTMCGROUTDoc::AddGraphicsInDoc( char *szSFileName )
{
 FILE *fd;
 char ch[TMC_GROUT_MAXSTRING_BUF];
 int i;

 if( (fd = fopen(GetPathName(), "a")) == NULL )
 {
	 sprintf( ch, "when open file for add %s to %s", szSFileName, GetPathName());
	 put_error_messege(ch);
	 return ;
 };

 MakeRelatPathSFile( szSFileName );

 for( i = strlen( szSFileName ); i > -1; i-- )
	 if( szSFileName[i] == '\\' ) break;


 if(fprintf(fd,"\n!New S_Matrix file was add.\n%s %s 1 1 1 1 1\n", szSFileName, szSFileName + i + 1 ) <= 0  )
 {
	 sprintf( ch, "when write line number %s in data file %s", szSFileName, GetPathName());
	 put_error_messege(ch);
	 fclose(fd);
	 return ;
 };

 fclose(fd);
 return ;

}


void CTMCGROUTDoc::MakeRelatPathSFile( char *szSFileName )
{
	int nDoc, nSfile, n;
	int i, iD, iS;
	int i1, i2;
	int j, ii;
	char  ch[TMC_GROUT_MAXSTRING_BUF];
	char ch1[TMC_GROUT_MAXSTRING_BUF];

	strcpy( ch, GetPathName() );
									// calculation number of directory in DocfileName and SfileName
	nDoc   = CalcNDirInPathName( ch );
	nSfile = CalcNDirInPathName( szSFileName );
	if( nDoc > nSfile )
	{
		n = nSfile;
	}
	else
	{
		n = nDoc;
	};
	
	for( i = 0, j = 0; i < n; i++, j += ii)
	{
		iD = SelectDirName( ch + j );
		iS = SelectDirName( szSFileName + j );
		
		if( iD > iS) ii = iS;
		else ii = iD;
		
		if( (iD != iS)||( strncmp( ch + j, szSFileName + j, ii ) != 0 ) )
		{
			if( i == 0 ) return;
			i2 = 0;
			if( nDoc < nSfile )
			{
				for( i1 = 0, i2 = 0; i1 < ( nDoc - i); i1++ )
				{
					ch1[i2++] = '.';
					ch1[i2++] = '.';
					ch1[i2++] = '\\';
				};
			};
			strcpy( ch1 + i2, szSFileName + j );
			strcpy( szSFileName, ch1 );
			return;			
		};
	};
	
	strcpy( ch1, szSFileName + j );
	strcpy( szSFileName, ch1 );
	return;
}

int CTMCGROUTDoc::SelectDirName( char *ch )
{
	int i;
	for( i = 0; (ch[ i ] != '\\')&&( i < TMC_GROUT_MAXSTRING_BUF ) ; i++ ) 
	{
		if( ch[ i ] == '\0' ) return 0;
	};
	return i+1;
}

int CTMCGROUTDoc::CalcNDirInPathName( char  *ch)
{
	int i, j;
	int n;
	n = SelectDirName( ch );
	for( i = n, j = 0; n != 0; j++ )
	{
		n = SelectDirName( ch + i );
		i += n;
	};
	return j;
}

void CTMCGROUTDoc::ReadGraph1( int nGr, char *szFileName, int *nPoint, double **pFreq, _complex **pSmatr )
{
 int i, i1, n;

 strcpy( grdoc.psGraph[nGr].szFileName, GetPathName() );
 for( i = strlen(grdoc.psGraph[nGr].szFileName)-1; i >= 0; i-- )
 {
 	if( grdoc.psGraph[nGr].szFileName[i] == '\\' )
	{
		grdoc.psGraph[nGr].szFileName[i] = '\0';
		i = -1;
	};
 };
 
 n = strlen(grdoc.psGraph[nGr].szFileName);
 grdoc.psGraph[nGr].szFileName[n]   = '\\';
 i1 = strlen( szFileName ); 
 for( i = 0; i < i1; i++)
 {
	 if( (szFileName[i] == ':') )
	 {
		 n = -1;
		 break ;
	 }; 
 };
 strcpy( grdoc.psGraph[nGr].szFileName + n + 1, szFileName);

 nGraphCurrent = nGr;

 read_RT_output_signal( nGr,
						&(grdoc.psGraph[nGr].nInp1),
						&(grdoc.psGraph[nGr].nMod1),
						&(grdoc.psGraph[nGr].nInp2),
						&(grdoc.psGraph[nGr].nMod2),
						grdoc.psGraph[nGr].szFileName,
						pSmatr,
						pFreq,
						nPoint, &(grdoc.psGraph[nGr].dFreq) );

 if( *nPoint <= 0 )
 {
	 sprintf( grdoc.psGraph[nGr].szGrapPodp, "Error_when_read_%s", szFileName );
 };

 return ;
}

void CTMCGROUTDoc::ReadGraph1Expession( int nGr, char *szExpression )
{
	int n, i;
	char      *pszFileName;
	int       *pnPoint;
	double   **ppFreq;
	_complex **ppSmatr;
	if( (grdoc.psGraph[nGr].pcExpr  = new CTMCGrExpression( szExpression ) ) == NULL )
	{
	 put_error_messege("memory allocation error for Expression class");
	 return;
	};
	
	if(  grdoc.psGraph[nGr].pcExpr->InitData() != NULL )
	{
	 sprintf( grdoc.psGraph[nGr].szGrapPodp, "Error_expression_absent_or_bad_file_name_%s", szExpression );
//	 delete grdoc.psGraph[nGr].pcExpr;
//	 grdoc.psGraph[nGr].pcExpr = NULL;
	 return;
	};

	n = grdoc.psGraph[nGr].pcExpr->GetNVar();
	for( i = 0; i < n; i++ )
	{
		pszFileName = grdoc.psGraph[nGr].pcExpr->GetFileName( i );
		pnPoint = grdoc.psGraph[nGr].pcExpr->GetNPoint( i );
		ppFreq  = grdoc.psGraph[nGr].pcExpr->GetFreqArray( i );
		ppSmatr = grdoc.psGraph[nGr].pcExpr->GetSMatrArray( i );
		if( ( pszFileName == NULL )||( pnPoint == NULL )||( ppFreq == NULL )||( ppSmatr == NULL ) )
		{
			 sprintf( grdoc.psGraph[nGr].szGrapPodp, "Error_expression_memory_allocation_error_%s", szExpression );
//			 delete grdoc.psGraph[nGr].pcExpr;
//			 grdoc.psGraph[nGr].pcExpr = NULL;
			 return;
		};
		ReadGraph1( nGr, pszFileName, pnPoint, ppFreq, ppSmatr);
		if( *pnPoint <= 0 ) 
		{
			sprintf( grdoc.psGraph[nGr].szGrapPodp, "Error_when_read_file_%s", pszFileName );
//			delete grdoc.psGraph[nGr].pcExpr;
//			grdoc.psGraph[nGr].pcExpr = NULL;
			return;
		}
	};

	if( !grdoc.psGraph[nGr].pcExpr->IsFreqCorrect() ) 
	{
		sprintf( grdoc.psGraph[nGr].szGrapPodp, "Error_expression_different_freq_array_%s", szExpression );
//		delete grdoc.psGraph[nGr].pcExpr;
//		grdoc.psGraph[nGr].pcExpr = NULL;
		return;
	}

	if( (grdoc.psGraph[nGr].pFreq = new  double[*pnPoint] ) == NULL )
	{
			 sprintf( grdoc.psGraph[nGr].szGrapPodp, "Error_expression_memory_allocation_error_%s", szExpression );
//			 delete grdoc.psGraph[nGr].pcExpr;
//			 grdoc.psGraph[nGr].pcExpr = NULL;
			 return;
	};
	for( i = 0; i < *pnPoint; i++ )
	{
		grdoc.psGraph[nGr].pFreq[i] = (*ppFreq)[i];
	};
	grdoc.psGraph[nGr].nPoint = *pnPoint;
	return;
}



void CTMCGROUTDoc::SetGraphColor( void )
{
	int i;
	for( i = 0; i < grdoc.nGraph; i++)
	{
		grdoc.psGraph[i].PointColor = scPointColor;
	};
	return;
}



void CTMCGROUTDoc::read_RT_output_signal( int n_Gr1, int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint, double *dFr)
{
	int i, n;
	char ch[TMC_GROUT_MAXSTRING_BUF];
//	c2DArray  cDirectionalPattern;
	double dX_min1, dX_max1;
	
	*nPoint = 0;
	
	if(  *in1 < 1 )  *in1 = 1;
//	if( *mod1 < 0 ) *mod1 = 0;
	if( *mod1 == 0 )
	{
		pcDirectionalPattern[ n_Gr1 ].bIsReduceElMagnAnalysis = FALSE;
	}
	else
	{
		pcDirectionalPattern[ n_Gr1 ].bIsReduceElMagnAnalysis = TRUE;
	};
	if(  *in2 < 1 )  *in2 = 1;
	if(  *in2 > 1 )  *in2 = 1;
	if( *mod2 < 0 ) *mod2 = 0;

	if( *mod1 > 1 ) *mod1 = 1;
	if( *mod2 > 1 ) *mod2 = 1;

	sprintf( ch, "%s~~tempDia", filename);
	pcDirectionalPattern[ n_Gr1 ].CalculateDiagrNapr( filename, ch );
	if( pcDirectionalPattern[ n_Gr1 ].IsError() ) return;

	switch(grdoc.nXType)
	{
		default:
			;
		case TMC_GROTS_TYPE_nT:
			dX_min1 = grdoc.Xmin;
			dX_max1 = grdoc.Xmax;
			break;
		case TMC_GROTS_TYPE_ps:
			dX_min1 = grdoc.Xmin*3.141592653589/180.;
			dX_max1 = grdoc.Xmax*3.141592653589/180.;
			break;
	};
	pcDirectionalPattern[ n_Gr1 ].ResizeAngle( dX_min1, dX_max1);
	if( pcDirectionalPattern[ n_Gr1 ].IsError() ) return;

								// calculated number of point
	n = pcDirectionalPattern[ n_Gr1 ].GetnAngle();
	if( n < 1 ) return;

									// allocation memory for data array
	if( *sss != NULL ) 
	{
		delete []*sss;
		*sss = NULL;
	};
	if( *f1 != NULL ) 
	{
		delete []*f1;
		*f1 = NULL;
	};
	if( ((*sss) = new _complex[ n ]) == NULL ) return;
	if( ((*f1) = new double[ n ]) == NULL ) return;

									// read data array
	for( i = 0; i < n; i++)
	{
			 (*f1)[i] = pcDirectionalPattern[ n_Gr1 ].GetdAngle(i);
			(*sss)[i].x = pcDirectionalPattern[ n_Gr1 ].GetdAmplitudeDiagrNapr(i);
	};
	*nPoint = n;

	dFreq = pcDirectionalPattern[ 0 ].GetdFreqGHz()*1000000000;

	return;
}

double CTMCGROUTDoc::gets_f1(char *ch)
{
	int i;
	double r1;

	sscanf( ch, "%d%lg", &i, &r1);

	return r1;

}

double CTMCGROUTDoc::gets_sss(char *ch, int in1, int mod1)
{
	int i, i1;
	double r1, r2;

	i = skip_ntdt_itd( ch, in1 );
	
	if( i == 0 ) return 0.0;

	sscanf( ch + i, "%d%lg%lg", &i1, &r1, &r2);

	if( mod1 == 0 ) return r1;
	if( mod1 == 1 ) return r2;

	return 0.0;
}

int CTMCGROUTDoc::skip_ntdt_itd(char *ch, int in1)
{
	int i, i1;
	
	i = 0;
	i += skip_blank(ch+i);
	i += skip_number(ch+i);
	i += skip_blank(ch+i);
	i += skip_number(ch+i);
	in1--;
	
	for( i1 = 0; i1 < in1; i1++ )
	{
		i += skip_blank(ch+i);
		i += skip_number(ch+i);
		i += skip_blank(ch+i);
		i += skip_number(ch+i);
		i += skip_blank(ch+i);
		i += skip_number(ch+i);
	}

	return i;
}

int CTMCGROUTDoc::skip_blank(char *ch)
{
	int i;
	for( i = 0; ch[i] != NULL; i++ )
	{
		switch( ch[i] )
		{
			case ' ': ;
			case '\t': ;
			case '\n': ;
				break;
			default: return i;
		};
	};
	return 0;
}

int CTMCGROUTDoc::skip_number(char *ch)
{
	int i;
	for( i = 0; ch[i] != NULL; i++ )
	{
		switch( ch[i] )
		{
			case ' ': ;
			case '\t': ;
			case '\n': ;
				return i;
			default: break;
		};
	};
	return 0;
}

void CTMCGROUTDoc::SetOneReadGraphParameters(char *ch)
{
	 int i;
	 int nCol; 
	 del_bl2(ch);
	 if( ch[0] == '#' ) 
	 {
		 if( strncmp( ch, TMC_GROTS_DOCFILE_ID, strlen(TMC_GROTS_DOCFILE_ID) ) == 0 )    ;
		 if( strncmp( ch, TMC_GROUT_DOCFILE_ID_PointDrawFlag, strlen(TMC_GROUT_DOCFILE_ID_PointDrawFlag) ) == 0 )
		 {
			 sscanf( ch + strlen(TMC_GROUT_DOCFILE_ID_PointDrawFlag), "%d;%d;",  &(grdoc.PointDrawFlag), &(bLossPoint) );
		 };
		 if( strncmp( ch, TMC_GROUT_DOCFILE_ID_nXType, strlen(TMC_GROUT_DOCFILE_ID_nXType) ) == 0 )
		 {
			 sscanf( ch + strlen(TMC_GROUT_DOCFILE_ID_nXType), "%d",  &(grdoc.nXType) );
		 };
		 if( strncmp( ch, TMC_GROUT_DOCFILE_ID_nYType, strlen(TMC_GROUT_DOCFILE_ID_nYType) ) == 0 )
		 {
			 sscanf( ch + strlen(TMC_GROUT_DOCFILE_ID_nYType), "%d",  &(grdoc.nYType) );
		 };
		 if( strncmp( ch, TMC_GROUT_DOCFILE_ID_Xmin, strlen(TMC_GROUT_DOCFILE_ID_Xmin) ) == 0 )
		 {
			 sscanf( ch + strlen(TMC_GROUT_DOCFILE_ID_Xmin), "%lg",  &(grdoc.Xmin) );
		 };
		 if( strncmp( ch, TMC_GROUT_DOCFILE_ID_Xmax, strlen(TMC_GROUT_DOCFILE_ID_Xmax) ) == 0 )
		 {
			 sscanf( ch + strlen(TMC_GROUT_DOCFILE_ID_Xmax), "%lg",  &(grdoc.Xmax) );
		 };
		 if( strncmp( ch, TMC_GROUT_DOCFILE_ID_Ymin, strlen(TMC_GROUT_DOCFILE_ID_Ymin) ) == 0 )
		 {
			 sscanf( ch + strlen(TMC_GROUT_DOCFILE_ID_Ymin), "%lg",  &(grdoc.Ymin) );
		 };
		 if( strncmp( ch, TMC_GROUT_DOCFILE_ID_Ymax, strlen(TMC_GROUT_DOCFILE_ID_Ymax) ) == 0 )
		 {
			 sscanf( ch + strlen(TMC_GROUT_DOCFILE_ID_Ymax), "%lg",  &(grdoc.Ymax) );
		 };
		 if( strncmp( ch, TMC_GROUT_DOCFILE_ID_nAFlagX, strlen(TMC_GROUT_DOCFILE_ID_nAFlagX) ) == 0 )
		 {
			 sscanf( ch + strlen(TMC_GROUT_DOCFILE_ID_nAFlagX), "%d",  &(grdoc.nAFlagX) );
		 };
		 if( strncmp( ch, TMC_GROUT_DOCFILE_ID_nAFlagY, strlen(TMC_GROUT_DOCFILE_ID_nAFlagY) ) == 0 )
		 {
			 sscanf( ch + strlen(TMC_GROUT_DOCFILE_ID_nAFlagY), "%d",  &(grdoc.nAFlagY) );
		 };
		 if( strncmp( ch, TMC_TOPVIEW_DOCFILE_ID_EDITORFLNM, strlen(TMC_TOPVIEW_DOCFILE_ID_EDITORFLNM) ) == 0 )
		 {
			 csEditorName.Format( "%s", ch + strlen(TMC_TOPVIEW_DOCFILE_ID_EDITORFLNM) );
		 };
		 if( strncmp( ch, TMC_TOPVIEW_DOCFILE_ID_sDrawPointColor, strlen(TMC_TOPVIEW_DOCFILE_ID_sDrawPointColor) ) == 0 )
		 {
			 TMC_GR_DOC1 sGrDoc1[16];
			 if(sscanf( ch + strlen(TMC_TOPVIEW_DOCFILE_ID_sDrawPointColor), "%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;",  &(nCol), 
				&( sGrDoc1[  0 ].LineColor ), &( sGrDoc1[  0 ].LineType ), &( sGrDoc1[  0 ].LineWidth ),
				&( sGrDoc1[  1 ].LineColor ), &( sGrDoc1[  1 ].LineType ), &( sGrDoc1[  1 ].LineWidth ),
				&( sGrDoc1[  2 ].LineColor ), &( sGrDoc1[  2 ].LineType ), &( sGrDoc1[  2 ].LineWidth ),
				&( sGrDoc1[  3 ].LineColor ), &( sGrDoc1[  3 ].LineType ), &( sGrDoc1[  3 ].LineWidth ),
				&( sGrDoc1[  4 ].LineColor ), &( sGrDoc1[  4 ].LineType ), &( sGrDoc1[  4 ].LineWidth ),
				&( sGrDoc1[  5 ].LineColor ), &( sGrDoc1[  5 ].LineType ), &( sGrDoc1[  5 ].LineWidth ),
				&( sGrDoc1[  6 ].LineColor ), &( sGrDoc1[  6 ].LineType ), &( sGrDoc1[  6 ].LineWidth ),
				&( sGrDoc1[  7 ].LineColor ), &( sGrDoc1[  7 ].LineType ), &( sGrDoc1[  7 ].LineWidth ),
				&( sGrDoc1[  8 ].LineColor ), &( sGrDoc1[  8 ].LineType ), &( sGrDoc1[  8 ].LineWidth ),
				&( sGrDoc1[  9 ].LineColor ), &( sGrDoc1[  9 ].LineType ), &( sGrDoc1[  9 ].LineWidth ),
				&( sGrDoc1[ 10 ].LineColor ), &( sGrDoc1[ 10 ].LineType ), &( sGrDoc1[ 10 ].LineWidth ),
				&( sGrDoc1[ 11 ].LineColor ), &( sGrDoc1[ 11 ].LineType ), &( sGrDoc1[ 11 ].LineWidth ),
				&( sGrDoc1[ 12 ].LineColor ), &( sGrDoc1[ 12 ].LineType ), &( sGrDoc1[ 12 ].LineWidth ),
				&( sGrDoc1[ 13 ].LineColor ), &( sGrDoc1[ 13 ].LineType ), &( sGrDoc1[ 13 ].LineWidth ),
				&( sGrDoc1[ 14 ].LineColor ), &( sGrDoc1[ 14 ].LineType ), &( sGrDoc1[ 14 ].LineWidth ),
				&( sGrDoc1[ 15 ].LineColor ), &( sGrDoc1[ 15 ].LineType ), &( sGrDoc1[ 15 ].LineWidth ) ) != 49 )
			 {
//					SetDefaultLineColorWidthType();	
					sscanf( ch + strlen(TMC_TOPVIEW_DOCFILE_ID_sDrawPointColor), "%d",  &(nCol) );
					scPointColor = nCol;
					for( i = 0; i < 16; i++ )
					{
						sGrDoc[ i ].PointColor = scPointColor;
					};
			 }
			 else
			 {
					scPointColor = nCol;
					for( i = 0; i < 16; i++ )
					{
						sGrDoc[ i ].LineColor  = sGrDoc1[ i ].LineColor;
						sGrDoc[ i ].LineType   = sGrDoc1[ i ].LineType;
						sGrDoc[ i ].LineWidth  = sGrDoc1[ i ].LineWidth;
						sGrDoc[ i ].PointColor = scPointColor;
					};
			 };
		 };
		 if( strncmp( ch, TMC_TOPVIEW_DOCFILE_ID_sDrawTextColor, strlen(TMC_TOPVIEW_DOCFILE_ID_sDrawTextColor) ) == 0 )
		 {
			 sscanf( ch + strlen(TMC_TOPVIEW_DOCFILE_ID_sDrawTextColor), "%d",  &(nCol) );
			 scTextColor = nCol;
		 };
		 if( strncmp( ch, TMC_TOPVIEW_DOCFILE_ID_sDrawGridColor, strlen(TMC_TOPVIEW_DOCFILE_ID_sDrawGridColor) ) == 0 )
		 {
			 sscanf( ch + strlen(TMC_TOPVIEW_DOCFILE_ID_sDrawGridColor), "%d",  &(nCol) );
			 scGridColor = nCol;
		 };
		 if( strncmp( ch, TMC_TOPVIEW_DOCFILE_ID_sDrawAxiesColor, strlen(TMC_TOPVIEW_DOCFILE_ID_sDrawAxiesColor) ) == 0 )
		 {
			 sscanf( ch + strlen(TMC_TOPVIEW_DOCFILE_ID_sDrawAxiesColor), "%d",  &(nCol) );
			 scAxiesColor = nCol;
		 };
		 if( strncmp( ch, TMC_TOPVIEW_DOCFILE_ID_sDrawBckgrdColor, strlen(TMC_TOPVIEW_DOCFILE_ID_sDrawBckgrdColor) ) == 0 )
		 {
			 sscanf( ch + strlen(TMC_TOPVIEW_DOCFILE_ID_sDrawBckgrdColor), "%d",  &(nCol) );
			 scBackgoundColor = nCol;
		 };
		 if( strncmp( ch, TMC_TOPVIEW_DOCFILE_ID_sDrawTextFont, strlen(TMC_TOPVIEW_DOCFILE_ID_sDrawTextFont) ) == 0 )
		 {
			 int iii1, iii2, iii3, iii4;
			 for( i = strlen(TMC_TOPVIEW_DOCFILE_ID_sDrawTextFont); (i < TMC_GROUT_MAXSTRING_BUF)&&(ch[i] != ';'); i++ )
			 {
				 ;
			 }
			 ch[i] = '\0';
			 strcpy( lfInitial.lfFaceName, ch + 1 + strlen(TMC_TOPVIEW_DOCFILE_ID_sDrawTextFont));
			 ch[i] = ';';
			 sscanf( ch + i + 1, "%ld;%ld;%ld;%ld;%ld;%d;%d;%d;%d;%d;", 
										&( lfInitial.lfHeight ),
										&( lfInitial.lfWidth ),
										&( lfInitial.lfEscapement ),
										&( lfInitial.lfOrientation ),
										&( lfInitial.lfWeight ),
										&( iii1 ),
										&( iii2 ),
										&( iii3 ),
										&( iii4 ) );
			 lfInitial.lfItalic    = iii1;
			 lfInitial.lfUnderline = iii2;
			 lfInitial.lfStrikeOut = iii3;
			 lfInitial.lfQuality   = iii4;
		 };
	 };
}

void CTMCGROUTDoc::ReadGraphParametersDefault()
{

// read default graphics parameters
 int i, i1;
 char  ch[TMC_GROUT_MAXSTRING_BUF];
 char ch1[TMC_GROUT_MAXSTRING_BUF];
 char ch2;
 CString csBuffer;

 csBuffer = AfxGetApp()->GetProfileString( TMC_GROTS_DOCFILE_ID, TMC_GROTS_DOCFILE_ID );
 if( csBuffer.GetLength() <= 0 )	return;
 strcpy( ch1, csBuffer);

 for( i = 0, i1 = 0; ch1[i] != '\0' ; i++)
 {
	 if( ch1[i] == '')
	 {
		 ch1[i] = '\n';
		 ch2 = ch1[i+1];
		 ch1[i+1] = '\0';
		 strcpy( ch, ch1+i1);
		 SetOneReadGraphParameters( ch );
		 ch1[i]   = '';
		 ch1[i+1] = ch2;
		 i1 = i+1;
	 };
 };

 return ;
 
}


void CTMCGROUTDoc::SetDefaultLineColorWidthType()
{
	
	int i;

	for( i = 0; i < 16; i++)
	{
		sGrDoc[i].nPoint = 0;
		sGrDoc[i].pFreq  = NULL;
		sGrDoc[i].pSmatr = NULL;
		sGrDoc[i].pcExpr = NULL;
		sGrDoc[i].nInp1  = 0;
		sGrDoc[i].nMod1  = 0;
		sGrDoc[i].nInp2  = 0;
		sGrDoc[i].nInp2  = 0;
		sGrDoc[i].szFileName = NULL;
		sGrDoc[i].szGrapName = NULL;
		sGrDoc[i].szGrapPodp = NULL;
		sGrDoc[i].LastWriteTime.dwLowDateTime   = 0;
		sGrDoc[i].LastWriteTime.dwHighDateTime  = 0;
		sGrDoc[i].OutFlag = TRUE;
		switch(i)
		{
		case  0:	sGrDoc[i].LineColor = RGB( 0, 0, 255);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[3].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  1:	sGrDoc[i].LineColor = RGB( 0, 255, 0);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[2].nX =  0;
					sGrDoc[i].piPoint[2].nY =  0;
					sGrDoc[i].piPoint[3].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[3].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  2:	sGrDoc[i].LineColor = RGB( 0, 255, 255);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX =  0;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[3].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  3:	sGrDoc[i].LineColor = RGB( 255, 0, 255);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX =  0;
					sGrDoc[i].piPoint[1].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[3].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[3].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[4].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[5].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[6].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[7].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[8].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[9].nY =  TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  4:	sGrDoc[i].LineColor = RGB( 255, 255, 0);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY =  0;
					sGrDoc[i].piPoint[1].nX =  0;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY =  0;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[4].nY =  0;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[5].nY =  0;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[6].nY =  0;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[7].nY =  0;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[8].nY =  0;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[9].nY =  0;
			break;
		case  5:	sGrDoc[i].LineColor = RGB( 0, 0, 128);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY =  0;
					sGrDoc[i].piPoint[1].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  0;
					sGrDoc[i].piPoint[2].nX =  0;
					sGrDoc[i].piPoint[2].nY =  0;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX =  0;
					sGrDoc[i].piPoint[4].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX =  0;
					sGrDoc[i].piPoint[5].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX =  0;
					sGrDoc[i].piPoint[6].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX =  0;
					sGrDoc[i].piPoint[7].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX =  0;
					sGrDoc[i].piPoint[8].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX =  0;
					sGrDoc[i].piPoint[9].nY =  TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  6:	sGrDoc[i].LineColor = RGB( 0, 128, 0);
					sGrDoc[i].piPoint[0].nX =  0;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  0;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX =  0;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX =  0;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX =  0;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX =  0;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX =  0;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX =  0;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  7:	sGrDoc[i].LineColor = RGB( 0, 128, 128);
					sGrDoc[i].piPoint[0].nX =  0;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  0;
					sGrDoc[i].piPoint[2].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX =  0;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX =  0;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX =  0;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX =  0;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX =  0;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX =  0;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  8:	sGrDoc[i].LineColor = RGB( 128, 20, 0);
					sGrDoc[i].piPoint[0].nX =  0;
					sGrDoc[i].piPoint[0].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  0;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX =  0;
					sGrDoc[i].piPoint[4].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX =  0;
					sGrDoc[i].piPoint[5].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX =  0;
					sGrDoc[i].piPoint[6].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX =  0;
					sGrDoc[i].piPoint[7].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX =  0;
					sGrDoc[i].piPoint[8].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX =  0;
					sGrDoc[i].piPoint[9].nY =  TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  9:	sGrDoc[i].LineColor = RGB( 128, 0, 128);
					sGrDoc[i].piPoint[0].nX =  0;
					sGrDoc[i].piPoint[0].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  0;
					sGrDoc[i].piPoint[2].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX =  0;
					sGrDoc[i].piPoint[4].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX =  0;
					sGrDoc[i].piPoint[5].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX =  0;
					sGrDoc[i].piPoint[6].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX =  0;
					sGrDoc[i].piPoint[7].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX =  0;
					sGrDoc[i].piPoint[8].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX =  0;
					sGrDoc[i].piPoint[9].nY =  TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case 10:	sGrDoc[i].LineColor = RGB( 128, 128, 0);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY =  0;
					sGrDoc[i].piPoint[3].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case 11:	sGrDoc[i].LineColor = RGB( 128, 128, 128);
					sGrDoc[i].piPoint[0].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[2].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY =  0;
					sGrDoc[i].piPoint[3].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case 12:	sGrDoc[i].LineColor = RGB( 0, 0, 192);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[1].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[2].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[3].nX =  TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/4;
			break;
		case 13:	sGrDoc[i].LineColor = RGB( 0, 192, 0);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[0].nY =  0;
					sGrDoc[i].piPoint[1].nX =  0;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[2].nY =  0;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[4].nY =  0;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[5].nY =  0;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[6].nY =  0;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[7].nY =  0;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[8].nY =  0;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[9].nY =  0;
			break;
		case 14:	sGrDoc[i].LineColor = RGB( 0, 192, 192);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[1].nX =  0;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[2].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[3].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/4;
			break;
		case 15:	sGrDoc[i].LineColor = RGB( 192, 192, 0);
					sGrDoc[i].piPoint[0].nX =  0;
					sGrDoc[i].piPoint[0].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[1].nX =  TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[1].nY =  0;
					sGrDoc[i].piPoint[2].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[2].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[4].nX =  0;
					sGrDoc[i].piPoint[4].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[5].nX =  0;
					sGrDoc[i].piPoint[5].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[6].nX =  0;
					sGrDoc[i].piPoint[6].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[7].nX =  0;
					sGrDoc[i].piPoint[7].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[8].nX =  0;
					sGrDoc[i].piPoint[8].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[9].nX =  0;
					sGrDoc[i].piPoint[9].nY =  TMC_GROUT_POINT_TYPE_Y/4;
			break;
		}
		sGrDoc[i].LineType  = PS_SOLID;
		sGrDoc[i].LineWidth = 50;
		sGrDoc[i].PointColor = scPointColor;
		sGrDoc[i].PointType  = PS_SOLID;
		sGrDoc[i].PointWidth = 10;
	};
	
	return;
}

void CTMCGROUTDoc::ReadDocFile_1()
{
	int i;
	for( i = 0; i < grdoc.nGraph; i++)		
	{
		ReadGraphCangeAngle( i );
	};
}

void CTMCGROUTDoc::ReadGraphCangeAngle(int nGr)
{
	ReadGraph1_CangeAngle( nGr,
				grdoc.psGraph[nGr].szGrapName,
				&(grdoc.psGraph[nGr].nPoint),
				&(grdoc.psGraph[nGr].pFreq),
				&(grdoc.psGraph[nGr].pSmatr));

	return;
}

void CTMCGROUTDoc::ReadGraph1_CangeAngle( int nGr, char *szFileName, int *nPoint, double **pFreq, _complex **pSmatr )
{

	read_RT_output_signal_CangeAngle( nGr,
						&(grdoc.psGraph[nGr].nInp1),
						&(grdoc.psGraph[nGr].nMod1),
						&(grdoc.psGraph[nGr].nInp2),
						&(grdoc.psGraph[nGr].nMod2),
						grdoc.psGraph[nGr].szFileName,
						pSmatr,
						pFreq,
						nPoint, &(grdoc.psGraph[nGr].dFreq) );

 if( *nPoint <= 0 )
 {
	 sprintf( grdoc.psGraph[nGr].szGrapPodp, "Error_when_read_%s", szFileName );
 };

 return ;
}

void CTMCGROUTDoc::read_RT_output_signal_CangeAngle( int n_Gr1, int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint, double *dFr)
{
	int i, n;
//	char ch[TMC_GROUT_MAXSTRING_BUF];
//	c2DArray  cDirectionalPattern;
	double dX_min1, dX_max1;
	
	*nPoint = 0;
	
	if(  *in1 < 1 )  *in1 = 1;
//	if( *mod1 < 0 ) *mod1 = 0;
	if( *mod1 == 0 )
	{
		pcDirectionalPattern[ n_Gr1 ].bIsReduceElMagnAnalysis = FALSE;
	}
	else
	{
		pcDirectionalPattern[ n_Gr1 ].bIsReduceElMagnAnalysis = TRUE;
	};
	if(  *in2 < 1 )  *in2 = 1;
	if(  *in2 > 1 )  *in2 = 1;
	if( *mod2 < 0 ) *mod2 = 0;

	if( *mod1 > 1 ) *mod1 = 1;
	if( *mod2 > 1 ) *mod2 = 1;

	if( pcDirectionalPattern[ n_Gr1 ].IsError() ) return;

	switch(grdoc.nXType)
	{
		default:
			;
		case TMC_GROTS_TYPE_nT:
			dX_min1 = grdoc.Xmin;
			dX_max1 = grdoc.Xmax;
			break;
		case TMC_GROTS_TYPE_ps:
			dX_min1 = grdoc.Xmin*3.141592653589/180.;
			dX_max1 = grdoc.Xmax*3.141592653589/180.;
			break;
	};
	pcDirectionalPattern[ n_Gr1 ].ResizeAngle( dX_min1, dX_max1);
	if( pcDirectionalPattern[ n_Gr1 ].IsError() ) return;

								// calculated number of point
	n = pcDirectionalPattern[ n_Gr1 ].GetnAngle();
	if( n < 1 ) return;

	if( *sss == NULL ) return;
	if( *f1 == NULL ) return;
									// read data array
	for( i = 0; i < n; i++)
	{
			 (*f1)[i] = pcDirectionalPattern[ n_Gr1 ].GetdAngle(i);
			(*sss)[i].x = pcDirectionalPattern[ n_Gr1 ].GetdAmplitudeDiagrNapr(i);
	};
	*nPoint = n;

	return;
}

