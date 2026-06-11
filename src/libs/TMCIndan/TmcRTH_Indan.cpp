// TmcRTH_Indan.cpp : implementation file
//

#include "stdafx.h"
#include <TmcRTH_Indan.h>
#include <prepr1.h>
#include <conio.h>
#include <tmcrth_indan.h>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_Indan

CTmcRTH_Indan::CTmcRTH_Indan()
{
	cError.Clear();
	nStep = 0;
	pfPrepr = NULL;
	csTplFileName.Format("");
	csTplFileNamePrepr.Format("");
	csCurrentPath.Format("");
	return;
}

CTmcRTH_Indan::~CTmcRTH_Indan()
{
	DeleteData();
}


//BEGIN_MESSAGE_MAP(CTmcRTH_Indan, CWnd)
	//{{AFX_MSG_MAP(CTmcRTH_Indan)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_Indan message handlers

void CTmcRTH_Indan::DeleteData( void )
{
	cOut.DeleteData();
	cParam.DeleteData();
	nStep = 0;
	cError.Clear();
	if( csTplFileNamePrepr.GetLength() != 0 ) remove(csTplFileNamePrepr);
	csTplFileName.Format("");
	csTplFileNamePrepr.Format("");
	csCurrentPath.Format("");
	ClosePreprFile();
	return;
}

void CTmcRTH_Indan::SetTplFileName( CString csFileName )
{
	FILE *pfTpl;
	CString csBuf;

//		{	CString csBuf1;
//			csBuf1.Format("Trace function\nSetTplFileName(%s);", csFileName);
//			AfxMessageBox(csBuf1);		
//		};
	DeleteData();
//		{	CString csBuf1;
//			csBuf1.Format("Trace function\nSetTplFileName(%s);", csFileName);
//			AfxMessageBox(csBuf1);		
//		};

	if( ( pfTpl = fopen( csFileName, "r") ) == NULL )
	{
		csBuf.Format("can't open file %s", csFileName);
		cError.PutErrorMessage(csBuf);
		return;
	};
	fclose(pfTpl);
	csTplFileName = csFileName;
	SetCurrentPath();
	csTplFileNamePrepr.Format("%s%s", csTplFileName, CTMCRTH_INDANPREPRFNA);
	MakePreprFile();
	return;
}

void CTmcRTH_Indan::SetTplFileName( char *pszFileName )
{
	CString csBuffer;
	csBuffer.Format("%s", pszFileName);
	SetTplFileName( csBuffer );
	return;
}

void CTmcRTH_Indan::SetCurrentPath( void )
{
	if( IsError() ) return;

	char szBuf[CTMCRTH_INDANSBUF];
	CString csBuf;
	int n, i;

	if( csTplFileName.GetLength() > CTMCRTH_INDANSBUF )
	{
		csBuf.Format("very long file name %s", csTplFileName);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csTplFileName );
	n = (int)(strlen(szBuf));
	if( n < 1 ) 
	{
		csCurrentPath.Format( "" );
		return;
	};
	for( i = n - 1; i > 1; i--)
	{
		if( szBuf[i] == '\\')
		{
			szBuf[i] = '\0';
			csCurrentPath.Format( "%s", szBuf );
			return;
		};
	};

	csCurrentPath.Format( "" );
	return;
}

BOOL CTmcRTH_Indan::IsError( void )
{
	return cError.IsError();
}

CString CTmcRTH_Indan::GetErrorMessage( void )
{
	return cError.GetErrorMessage();
}

void CTmcRTH_Indan::MakePreprFile( void )
{
	if( IsError() ) return;

//	WriteFileForMaple( "c:\\work\\tpl\\ivtan\\test\\test_maple.dat", "test_maple_spline.dat", 130., 5.0, 40.0, 360.0, 398.0, 86.5);

	PutTrace("Load prepr.");
//	{
//		CString csBuf1;
//		csBuf1.Format("Load prepr.");
//		AfxMessageBox(csBuf1);
//	};
	MainPrepr( csTplFileNamePrepr, csTplFileName, csCurrentPath, cError );
	PutTrace("UnLoad prepr.");
//	{
//		CString csBuf1;
//		csBuf1.Format("UnLoad prepr.");
//		AfxMessageBox(csBuf1);
//	};
	return;
}

int CTmcRTH_Indan::GetNStepMax( CString csFileName )
{
	int i;
	SetTplFileName( csFileName );
	OpenPreprFile();
	i = CalcNStepMax();
	ClosePreprFile();
	return i;
}

int CTmcRTH_Indan::GetNStepMax( void )
{
	int i;
	OpenPreprFile();
	i = CalcNStepMax();
	ClosePreprFile();
	return i;
}

int CTmcRTH_Indan::CalcNStepMax( void )
{
	if( IsError() ) return 0;
	
	int i, ii;
	CString csCh;
	CString csBuf;

	for( i = 0, ii = 0; GetLine( csCh ) != EOF;  )
	{
		if( IsError() ) return 0;
		if( strncmp( csCh, CTMCRTH_INDANMET_STEP, strlen(CTMCRTH_INDANMET_STEP) ) == 0 )
		{
			switch( ii )
			{
				case  0:	
					ii = 1;
					break;
				case  1:	
					csBuf.Format("Missing %s", CTMCRTH_INDANMET_ENDSTEP);
					cError.PutErrorMessage( csBuf );
					return 0;
			};
		};
		if( strncmp( csCh, CTMCRTH_INDANMET_ENDSTEP, strlen(CTMCRTH_INDANMET_ENDSTEP) ) == 0 )
		{
			switch( ii )
			{
				case  0:	
					csBuf.Format("Missing %s", CTMCRTH_INDANMET_STEP);
					cError.PutErrorMessage( csBuf );
					return 0;
				case  1:	
					ii = 0;
					i++;
					break;
			};
		};
	};

	if( ii != 0 )
	{
		csBuf.Format("Missing %s", CTMCRTH_INDANMET_ENDSTEP);
		cError.PutErrorMessage( csBuf );
		return 0;
	};

	return i;
}

void CTmcRTH_Indan::OpenPreprFile( void )
{
	if( IsError() ) return;
	if( pfPrepr != NULL ) return;

	PutTrace("Load prepr.");

	CString csBuf;

	if( csTplFileNamePrepr.GetLength() == 0 )
	{
		csBuf.Format("You foget set tpl file name");
		cError.PutErrorMessage( csBuf );
		return;
	};

	if( ( pfPrepr = fopen( csTplFileNamePrepr, "r") ) == NULL )
	{
		csBuf.Format("Can't open prepr file %s", csTplFileNamePrepr);
		cError.PutErrorMessage( csBuf );
		return;
	};
	return;
}

void CTmcRTH_Indan::ClosePreprFile( void )
{
	if( pfPrepr != NULL ) fclose( pfPrepr );
	pfPrepr = NULL;
	PutTrace("UnLoad prepr.");
	return;
}

int CTmcRTH_Indan::GetLine( CString &csCh )
{
	if( pfPrepr == NULL ) OpenPreprFile();
	if( IsError() ) return EOF;

	char szBuf[CTMCRTH_INDAN_LINELEN];
	CString csBuf;

	if( fgets( szBuf, CTMCRTH_INDAN_LINELEN, pfPrepr ) == NULL )
	{
		csBuf.Format("Missing %s", CTMCRTH_INDANMET_EOF);
		cError.PutErrorMessage( csBuf );
		ClosePreprFile();
		return EOF;
	};
	
	del_blanks( szBuf );
	
	if( (szBuf[0] == '!')||(szBuf[0] == '\n') ) szBuf[0] = '\0';
	
	if( strlen( szBuf ) == 0 ) return GetLine( csCh );

	csCh.Format("%s", szBuf);

	if( strncmp( csCh, CTMCRTH_INDANMET_EOF, strlen(CTMCRTH_INDANMET_EOF)) == 0 )
	{
		return EOF;
	};

	return 0;
}

void CTmcRTH_Indan::del_blanks( char * szBuf )
{
	int i, n;
	int ii, i1;
	
	n = (int)(strlen( szBuf ));
	
	for( i = 0, i1 = 0, ii = 0; i < n; i++)
	{
		switch( szBuf[i] )
		{
		case ' ':
		case '\t':
			if( ii == 0) break;
			if( ii == 1 )
			{
			szBuf[ i1 ] = szBuf[ i ];
			i1++;
			ii = 0;
			};
			break;
		default :
			szBuf[ i1 ] = szBuf[ i ];
			i1++;
			ii = 1;
			break;
		};
	};

	szBuf[ i1 ] = szBuf[ i ];
	return;
}

void CTmcRTH_Indan::SetStepNumber( int nStep1 )
{
	if( IsError() ) return;
	nStep = nStep1;
	return;
}

void CTmcRTH_Indan::ReadParamSection( void )
{
	if( IsError() ) return;

	int i;

	for( i = 1; i <= nStep; i++ )
	{
		ReadParamSection( i );
	};

	return;
}

void CTmcRTH_Indan::ReadParamSection( int nStep1 )
{
	CString csCh;
	CString csBuf;

	if( IsError() ) return;
	if( nStep1 == 1 ) cParam.DeleteData();
	
	FindStep( nStep1 );
	if( FindSection( CTMCRTH_INDANMET_PARAM, CTMCRTH_INDANMET_ENDPARAM ) != 0 )
	{
		if( nStep1 == 1 )
		{
			csBuf.Format("Missing section %s in %d step", CTMCRTH_INDANMET_PARAM, nStep1);
			cError.PutErrorMessage(csBuf);
			ClosePreprFile();
			return;
		}
		else
		{
			ClosePreprFile();
			return;
		};
	};
	for( ; GetLineSection( csCh ) != EOF;  )
	{
		if( IsError() ) 
		{
			ClosePreprFile();
			return;
		}
		cParam.Read( csCh, cError );
	};
	
	cParam.IsCorrectly( cError );
	ClosePreprFile();
	return;
}

void CTmcRTH_Indan::FindStep( int nStep1 )
{
	CString csCh;
	CString csBuf;
	int i;
	if( IsError() ) return;
	
	ClosePreprFile();
	OpenPreprFile();
	
	for( i = 0; GetLine( csCh ) != EOF;  )
	{
		if( IsError() ) return;
		if( strncmp( csCh, CTMCRTH_INDANMET_STEP, strlen(CTMCRTH_INDANMET_STEP) ) == 0 ) 
		{
			i++;
			if( i == nStep1 ) return;
		};
	};
	csBuf.Format("very big step number %d", nStep1);
	cError.PutErrorMessage( csBuf );
	ClosePreprFile();
	return;
}

int CTmcRTH_Indan::FindSection( char * pszSection,  char * pszEndSection)
{
	if( IsError() ) return 0;
	csSection.Format( "%s", pszSection );
	csEndSection.Format( "%s", pszEndSection );
	return FindSection();
}

int CTmcRTH_Indan::FindSection( void )
{
	CString csCh;
	CString csBuf;

	if( IsError() ) return 0;
	
	for( ; GetLine( csCh ) != EOF;  )
	{
		if( IsError() ) return 0;
		if( strncmp( csCh, csSection, csSection.GetLength() ) == 0 ) 
		{
			return 0;
		};
		if( strncmp( csCh, CTMCRTH_INDANMET_ENDSTEP, strlen(CTMCRTH_INDANMET_ENDSTEP) ) == 0 ) 
		{
			ClosePreprFile();
			return -1;
		};
	};
	ClosePreprFile();
	return -1;
}

int CTmcRTH_Indan::GetLineSection( CString &csCh )
{
	int i;
	CString csBuf;

	if( IsError() ) return EOF;
	
	i = GetLine( csCh );
	
	if( IsError() ) return EOF;
	
	if( strncmp( csCh, csEndSection, csEndSection.GetLength() ) == 0 ) 
	{
		return EOF;
	};
	if( strncmp( csCh, CTMCRTH_INDANMET_ENDSTEP, strlen(CTMCRTH_INDANMET_ENDSTEP) ) == 0 ) 
	{
		csBuf.Format("Missing  %s", csEndSection);
		cError.PutErrorMessage( csBuf );
		ClosePreprFile();
		return EOF;
	};
	if( i == EOF )
	{
		csBuf.Format("Missing  %s", csEndSection);
		cError.PutErrorMessage( csBuf );
		ClosePreprFile();
		return EOF;
	};
	
	return i;
}


int CTmcRTH_Indan::GetBlock( CString &csCh )
{
	int i;
	CString csBuf;
	CString csBuf1;
	
	csCh.Format("");

	if( IsError() ) return EOF;
	
	i = GetLineSection( csBuf );
	
	if( IsError() ) return EOF;

	if( i == EOF ) return i;
	
	if( strncmp( csBuf, CTMCRTH_INDANMET_BLOCK, strlen(CTMCRTH_INDANMET_BLOCK) ) != 0 ) 
	{
		csBuf1.Format("{%s} must {%s} in %s section",
								csBuf,
								CTMCRTH_INDANMET_BLOCK,
								CTMCRTH_INDANMET_TOPOLOGY );
		cError.PutErrorMessage( csBuf1 );
		ClosePreprFile();
		return EOF;
	};
	
	csCh += csBuf;
	while( (i = GetLineSection( csBuf )) != EOF )
	{
//		{
//			FILE *fp;
//			fp = fopen( "debug2.txt", "a" );
//			fprintf( fp, "\nAdd Block {%s}", csCh);
//			fclose(fp);
//		};
		if( strncmp( csBuf, CTMCRTH_INDANMET_ENDBLOCK, strlen(CTMCRTH_INDANMET_ENDBLOCK) ) == 0 ) 
		{
//			csCh += csBuf;
			return 0;
		};
		if( strncmp( csBuf, CTMCRTH_INDANMET_BLOCK, strlen(CTMCRTH_INDANMET_BLOCK) ) == 0 ) 
		{
			csBuf1.Format("Missing  %s", CTMCRTH_INDANMET_ENDBLOCK);
			cError.PutErrorMessage( csBuf1 );
			ClosePreprFile();
			return EOF;
		};
		csCh += csBuf;
	}

	csBuf1.Format("Missing  %s", CTMCRTH_INDANMET_ENDBLOCK);
	cError.PutErrorMessage( csBuf1 );
	ClosePreprFile();
	return EOF;
}

void CTmcRTH_Indan::ReadOutputSection( void )
{
	CString csCh;
	CString csBuf;

	if( IsError() ) return;
	
	cOut.DeleteData();

	cOut.SetFreqAndTimeUnit( cParam.rFreqUnit, cParam.rTimeUnit );
	
	FindStep( nStep );
	if( FindSection( CTMCRTH_INDANMET_OUTPUT, CTMCRTH_INDANMET_ENDOUTPUT ) != 0 )
	{
		csBuf.Format("Missing section %s in %d step", CTMCRTH_INDANMET_OUTPUT, nStep);
		cError.PutErrorMessage(csBuf);
		ClosePreprFile();
		return;
	};
	for( ; GetLineSection( csCh ) != EOF;  )
	{
		if( IsError() ) 
		{
			ClosePreprFile();
			return;
		}
		cOut.Read( csCh, cError );
	};
	
	cOut.IsCorrectly( cError );
	cOut.AddPath( csCurrentPath );
	
	ClosePreprFile();
	return;
}

void CTmcRTH_Indan::ReadLinkListSection( void )
{
	CString csCh;
	CString csBuf;

	if( IsError() ) return;
	
	cTopology.SetParam( &cParam );

	FindStep( nStep );
	if( FindSection( CTMCRTH_INDANMET_LINKLIST, CTMCRTH_INDANMET_ENDLINK ) != 0 )
	{
		csBuf.Format("Missing section %s in %d step", CTMCRTH_INDANMET_LINKLIST, nStep);
		cError.PutErrorMessage(csBuf);
		ClosePreprFile();
		return;
	};
	for( ; GetLineSection( csCh ) != EOF;  )
	{
		if( IsError() ) 
		{
			ClosePreprFile();
			return;
		}
		cTopology.ReadLink( csCh, cError );
	};
	
	cTopology.IsCorrectlyLink( cError );
	ClosePreprFile();
	return;
}

void CTmcRTH_Indan::ReadTopologySection( void )
{
	CString csCh;
	CString csBuf;

	if( IsError() ) return;
	cTopology.DeleteData();
	
	cTopology.SetParam( &cParam );
	
	FindStep( nStep );
	if( FindSection( CTMCRTH_INDANMET_TOPOLOGY, CTMCRTH_INDANMET_ENDTOPOLOGY ) != 0 )
	{
		csBuf.Format("Missing section %s in %d step", CTMCRTH_INDANMET_TOPOLOGY, nStep);
		cError.PutErrorMessage(csBuf);
		ClosePreprFile();
		return;
	};
	for( ; GetBlock( csCh ) != EOF;  )
	{
		if( IsError() ) 
		{
			ClosePreprFile();
			return;
		}
		cTopology.Read( csCh, cError );
	};
	
	cTopology.IsCorrectly( cError );
	ClosePreprFile();
	cTopology.AddPathForFile( csCurrentPath );
	return;
}



CTmcRTH_IndanOutput* CTmcRTH_Indan::GetOutput( void )
{
	return &cOut;
}

CTmcRTH_IndanParam& CTmcRTH_Indan::GetParam( void )
{
	return cParam;
}

CTmcRTH_IndanTopology& CTmcRTH_Indan::GetTopology( void )
{
	return cTopology;
}

BOOL CTmcRTH_Indan::IsDataRead( void )
{
	return cTopology.IsDataRead();
}

int CTmcRTH_Indan::GetnStep( void )
{
	return nStep;
}


int CTmcRTH_Indan::nGetTmax( void )
{
	return cParam.nGetTmax();
}

double CTmcRTH_Indan::dGetTmin( void )
{
	return cParam.dGetTmin();
}

double CTmcRTH_Indan::dGetdT( void )
{
	return cParam.dGetdT();
}

double CTmcRTH_Indan::dGetDelta( void )
{
	return cParam.dGetDelta();
}

void CTmcRTH_Indan::GetXYMinMax( double &dXmin, double &dXmax, double &dYmin, double &dYmax )
{
	cParam.GetXYMinMax( dXmin, dXmax, dYmin, dYmax );
	return;
}

int CTmcRTH_Indan::GetnBlock( void )
{
	return cTopology.GetBlockNumber();
}

void CTmcRTH_Indan::OnOffFieldOutput()
{
	cOut.OnOffFieldOutput();
	return;
}

void CTmcRTH_Indan::OnOffFieldOutput1()
{
	cOut.OnOffFieldOutput1();
	return;
}

void CTmcRTH_Indan::OnOffFieldOutputU()
{
	cOut.OnOffFieldOutputU();
	return;
}

void CTmcRTH_Indan::OnOffSinchronizationFieldOutput()
{
	cOut.OnOffSinchronizationFieldOutput();
	return;
}

void CTmcRTH_Indan::GetnXnY(int &nX11, int &nY11)
{
	nX11 = cParam.CalcnX();
	nY11 = cParam.CalcnY();
	return;
}

CString CTmcRTH_Indan::GetCurrentPath()
{
	return csCurrentPath;
}

void CTmcRTH_Indan::OnOffSoundEffects()
{
	cOut.OnOffSoundEffects();
	return;
}

void CTmcRTH_Indan::ReadData(CString csFileName, int nStepNum )
{
	if( strlen( csFileName ) < 1 )
	{
		CString csBuf;
		csBuf.Format("bad file name (lenght of file name < 1)");
		cError.PutErrorMessage( csBuf );
		return;
	}
	SetTplFileName( csFileName );
	if( cError.IsError() )
	{
		return;
	}
	if( (nStepNum < 1)||(nStepNum > GetNStepMax() ) )
	{
		CString csBuf;
		csBuf.Format("bad step number %d (in file %s step number must be from 1 to %d) ", nStepNum, csFileName, GetNStepMax());
		cError.PutErrorMessage( csBuf );
		return;
	};
	SetStepNumber( nStepNum );
	ReadParamSection();
	ReadTopologySection();
	ReadLinkListSection();
	ReadOutputSection();

	return;
}


CString CTmcRTH_Indan::GetTopologyFileName(void)
{
	return csTplFileName;
}

// Write file csFileName for calculating in Maple 9.0
// after executing Maple 9.0 data was write in file csFileNameData
// dHeight   - height stator and rolik in mm
// dRolikR1  - internal radius of rolik in mm
// dRolikR2  - external radius of rolik in mm
// dStatorR1 - internal radius of stator in mm
// dStatorR2 - external radius of stator in mm
// dMagnMomP -  magnetic moment

CTmcLibError CTmcRTH_Indan::WriteFileForMaple(CString csFileName, CString csFileNameData, double dHeight, double dRolikR1, double dRolikR2, double dStatorR1, double dStatorR2, double dMagnMomP)
{
	CTmcLibError cError1;
	FILE *fp;

	if( strlen(csFileName) < 1 )
	{
		cError1.PutErrorMessage("lenght of file name for Maple < 1");
		return cError1;
	}

	if( strlen(csFileNameData) < 1 )
	{
		cError1.PutErrorMessage("lenght of file name for spline data < 1");
		return cError1;
	}
	
	if( ( fp = fopen( csFileName , "w") ) == NULL )
	{
		CString csBuf;
		csBuf.Format("can't open file {%s} for write geometry data for Maple", csFileName );
		cError1.PutErrorMessage( csBuf );
		return cError1;
	};

	if( fprintf( fp, "\nFile name for spline \"%s\"\nHeight stator and rolik %lg mm\nInternal radius of rolik %lg mm\nExternal radius of rolik %lg mm\nInternal radius of stator %lg mm\nInternal radius of stator %lg mm\nMagnetic moment %lg \n",
		csFileNameData, dHeight, dRolikR1, dRolikR2, dStatorR1, dStatorR2, dMagnMomP ) < 1 )
	{
		CString csBuf;
		csBuf.Format("can't  write geometry data for Maple in file {%s}", csFileName );
		cError1.PutErrorMessage( csBuf );
		fclose( fp );
		return cError1;
	};

//	{
//		CString csBuf1;
//		csBuf1.Format( "Write file {%s} for Maple", csFileName );
//		AfxMessageBox(csBuf1);
//	};

	fclose( fp );
	return cError1;
}

CTmcLibError CTmcRTH_Indan::WriteFileForMaple(char *szFileName, char *szFileNameData, double dHeight, double dRolikR1, double dRolikR2, double dStatorR1, double dStatorR2, double dMagnMomP)
{
	CString csBuf, csBuf1;

	csBuf.Format("%s", szFileName);
	csBuf1.Format("%s", szFileNameData);

	return WriteFileForMaple( csBuf, csBuf1, dHeight, dRolikR1, dRolikR2, dStatorR1, dStatorR2, dMagnMomP);
}




void MainPrepr( CString csTplFileNamePrepr,	CString csTplFileName, CString csCurrentPath, CTmcLibError &cError )
{
	if( cError.IsError() ) return;

	char *szError, *szPathName;
	
	if( (szError = new char[ sizeof(char)*BUF_STR_N_MAX ]) == NULL )
	{
		cError.PutErrorMessage("Memory allocation");
		return;
	};
	if( (szPathName = new char[ sizeof(char)*BUF_STR_N_MAX ]) == NULL )
	{
		delete  []szError;
		cError.PutErrorMessage("Memory allocation");
		return;
	};

//	char *ch;
	char szSourceFileName[512];
	char szTargetFileName[512];
	
	strcpy( szSourceFileName, csTplFileName);
	strcpy( szTargetFileName, csTplFileNamePrepr);
	strcpy( szPathName, csCurrentPath);


	MainPrepr1( szTargetFileName,	szSourceFileName, szPathName, szError );

	if( strlen(szError) != 0 ) cError.PutErrorMessage(szError);
	
	delete []szPathName;
	delete []szError;

	return;
}
