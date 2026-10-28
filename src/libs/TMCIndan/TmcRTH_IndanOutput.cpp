// TmcRTH_IndanOutput.cpp : implementation file
//

#include "stdafx.h"
#include <TmcRTH_IndanOutput.h>
#include <TmcRTH_Indan.h>
#include <TmcTtoS.h>
#include <TmcWideband.h>
#include <tmcgrviw.h>	// D-1: types of the input blocks
#include <proc_s.h>		// D-1: save_S_matrix_element
#include <expr.h>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_IndanOutput

CTmcRTH_IndanOutput::CTmcRTH_IndanOutput()
{
	csFileName.Format("");
	csFileNameField.Format("");
	csFileNameField1.Format("");
	csFileNameFieldU.Format("");
	csFileNameField1Mod.Format("");
	csFileNameField1Faz.Format("");
	csFileNameSMatrix.Format("");
	csFileNameQ.Format("");
	csFileNameIx.Format("");
	csFileNameIy.Format("");
	bOutField = FALSE;
	bOutField1 = FALSE;
	bOutFieldU = FALSE;
	bOutTopology = FALSE;
	bOutFieldSinchronization = FALSE;
	bSoundEffect = TRUE;
	bOutSMatrix = FALSE;
	dTmin = 0.0;
	dTmax = 0.0;
	dFreq = 0.0;
	dFreqUnit = 0.0;
	dTimeUnit = 0.0;
	return;
}

CTmcRTH_IndanOutput::~CTmcRTH_IndanOutput()
{
	DeleteData();
	return;
}


//BEGIN_MESSAGE_MAP(CTmcRTH_IndanOutput, CWnd)
	//{{AFX_MSG_MAP(CTmcRTH_IndanOutput)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_IndanOutput message handlers

void CTmcRTH_IndanOutput::DeleteData( void )
{
//{	CString csBuf1;
//	csBuf1.Format("Trace3");
//	AfxMessageBox(csBuf1);		
//};
	cError.Clear();
//{	CString csBuf1;
//	csBuf1.Format("Trace4");
//	AfxMessageBox(csBuf1);		
//};
	cFieldDistr.DeleteData();
	csFileName.Format("");
	csFileNameField.Format("");
	csFileNameField1.Format("");
	csFileNameFieldU.Format("");
	csFileNameField1Mod.Format("");
	csFileNameField1Faz.Format("");
	csFileNameSMatrix.Format("");
	csFileNameQ.Format("");
	csFileNameIx.Format("");
	csFileNameIy.Format("");
	bOutField = FALSE;
	bOutTopology = FALSE;
	bOutSMatrix = FALSE;
	dTmin = 0.0;
	dTmax = 0.0;
	dFreq = 0.0;
	dFreqUnit = 0.0;
	dTimeUnit = 0.0;
	return;
}

void CTmcRTH_IndanOutput::Read( CString &csCh, CTmcLibError &cError1 )
{
	if( cError1.IsError() ) 
	{
		return;
	};
	if( cError.IsError() ) 
	{
		cError1 = cError;
		return;
	};

	if( strncmp( csCh, CTMCRTH_FILEOUT_ID, strlen(CTMCRTH_FILEOUT_ID) ) == 0 )
	{
		ReadFileName( csCh );
		cError1 = cError;
		return;
	};
	if( strncmp( csCh, CTMCRTH_TOPOLOGYOUT_ID, strlen(CTMCRTH_TOPOLOGYOUT_ID) ) == 0 )
	{
		ReadTopology();
		cError1 = cError;
		return;
	};
	if( strncmp( csCh, CTMCRTH_FIELDSOUT_ID, strlen(CTMCRTH_FIELDSOUT_ID) ) == 0 )
	{
		ReadFields();
		cError1 = cError;
		return;
	};
	if( strncmp( csCh, CTMCRTH_SMATRIXOUT_ID, strlen(CTMCRTH_SMATRIXOUT_ID) ) == 0 )
	{
		ReadSmatrix( csCh );
		cError1 = cError;
		return;
	};
	if( strncmp( csCh, CTMCRTH_FLDSDSTROUT_ID, strlen(CTMCRTH_FLDSDSTROUT_ID) ) == 0 )
	{
		ReadFileDistr( csCh, CTMCRTH_FLDSDSTROUT_ID );
		cError1 = cError;
		return;
	};
	if( strncmp( csCh, CTMCRTH_FLDSDSMROUT_ID, strlen(CTMCRTH_FLDSDSMROUT_ID) ) == 0 )
	{
		ReadFileDistrInMemory( csCh );
		cError1 = cError;
		return;
	};

	CString csBuf;
	csBuf.Format( "Unknown keyword {%s}, use {%s} {%s} {%s} {%s} {%s}", csCh,
		CTMCRTH_FILEOUT_ID, CTMCRTH_TOPOLOGYOUT_ID, CTMCRTH_FIELDSOUT_ID, CTMCRTH_SMATRIXOUT_ID, CTMCRTH_FLDSDSTROUT_ID);
	cError.PutErrorMessage(csBuf);
	cError1 = cError;
	return;
}

void CTmcRTH_IndanOutput::ReadFileName( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	int i;
	CString csBuf;

	strcpy( szBuf, csCh );
	i = (int)(strlen(CTMCRTH_FILEOUT_ID));

	i+= Search_1Param( csBuf, szBuf+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	csFileName.Format("%s", szBuf);
	
	if( strlen( csFileName ) == 0 )
	{
		csBuf.Format("Missing file name in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};
	
	for( i = (int)(strlen(csFileName)); i >= 0; i-- )
	{
		if( ( csFileName[i] == '\\' )||( csFileName[i] == '/' ) ) break;	// N-22: a dot in a folder name is allowed
		if( csFileName[i] == '.' )
		{
			csBuf.Format("Unresolved simbol '.' in filename {%s}", csCh);
			cError.PutErrorMessage(csBuf);
			return;
		};
	};

	csFileName.Format("%s.t", szBuf);
	csFileNameField.Format("%s.ex", szBuf);
	csFileNameField1.Format("%s.ey", szBuf);
	csFileNameFieldU.Format("%s.hz", szBuf);
	csFileNameField1Mod.Format("%s.em", szBuf);
	csFileNameField1Faz.Format("%s.ef", szBuf);
	csFileNameTopology.Format("%s.tt", szBuf);
	csFileNameSMatrix.Format("%s.s", szBuf);
	csFileNameQ.Format("%s.q", szBuf);
	csFileNameIx.Format("%s.ix", szBuf);
	csFileNameIy.Format("%s.iy", szBuf);
	
	return;
}

void CTmcRTH_IndanOutput::ReadTopology( void )
{
	if( cError.IsError() ) return;

	bOutTopology = TRUE;	
	
	return;
}

void CTmcRTH_IndanOutput::ReadFields( void )
{
	if( cError.IsError() ) return;

	bOutField  = TRUE;	
	bOutField1 = TRUE;
	bOutFieldU = TRUE;
	
	return;
}

int CTmcRTH_IndanOutput::Search_1Param( CString &csBuf, char *szBuf )
{
	if( cError.IsError() ) return -1;

	int i, i1, i2, i3;

	for( i = 0, i1 = 0, i2 = 0, i3 = 0; (i < CTMCRTH_INDANSBUF)&&(szBuf[i] != '\0'); i++ )
	{
		switch( szBuf[i] )
		{
			case '{': i1++;
					break;
			case '}': i1--;
					break;
			case '(': i2++;
					break;
			case ')': i2--;
					break;
			case '[': i3++;
					break;
			case ']': i3--;
					break;
			case ';': 
					if( (i1 == 0)&&(i2 == 0)&&(i3 == 0) )
					{
						szBuf[i] = '\0';
						csBuf.Format( "%s", szBuf);
						szBuf[i] = ';';
						i++;
						return i;
					};
					break;
		};
	}

	cError.PutErrorMessage("Missing separator ;");
	return -1;
}

int CTmcRTH_IndanOutput::expr_del_Blanks2( char *ch )
{
	int i, i1;
	for( i = 0, i1 = 0;  i >= 0 ; i++  )
	{
		switch( ch[i] )
		{
			case ' ':  
				break;
			case '\t':	
				break;
			case '\n':
				break;
			default:
				ch[i1] = ch[i];
				i1++;
				if( ch[i] == '\0' ) 
				{
					return 0;
				};
		};
	};
	return 0;
}

void CTmcRTH_IndanOutput::IsCorrectly( CTmcLibError &cError1 )
{
	CString csBuf;

	if( cError1.IsError() ) return;
	
	if( cError.IsError() ) 
	{
		cError1 = cError;
		return;
	}
	
	if( csFileName.GetLength() == 0 )
	{
		csBuf.Format("Section %s output file name is absent", CTMCRTH_INDANMET_OUTPUT );
		cError.PutErrorMessage(csBuf);
		cError1 = cError;
		return;
	};
	
	
	return;
}


//CTmcRTH_IndanOutput& operator=( CTmcRTH_IndanOutput& cOut )

CTmcRTH_IndanOutput& CTmcRTH_IndanOutput::operator=( CTmcRTH_IndanOutput& cOut )
{
	DeleteData();
	bOutField  = cOut.bOutField;
	bOutField1 = cOut.bOutField1;
	bOutFieldU = cOut.bOutFieldU;
	bOutTopology = cOut.bOutTopology;
	bOutFieldSinchronization = cOut.bOutFieldSinchronization;
	bSoundEffect = cOut.bSoundEffect;
	bOutSMatrix = cOut.bOutSMatrix;
	csFileName = cOut.csFileName;
	csFileNameField  = cOut.csFileNameField;
	csFileNameField1 = cOut.csFileNameField1;
	csFileNameFieldU = cOut.csFileNameFieldU;
	csFileNameField1Mod = cOut.csFileNameField1Mod;
	csFileNameField1Faz = cOut.csFileNameField1Faz;
	csFileNameTopology = cOut.csFileNameTopology;
	csFileNameSMatrix = cOut.csFileNameSMatrix;
	csFileNameQ  = cOut.csFileNameQ;
	csFileNameIx = cOut.csFileNameIx;
	csFileNameIy = cOut.csFileNameIy;
	dTmin = cOut.dTmin;
	dTmax = cOut.dTmax;
	dFreq = cOut.dFreq;
	dFreqUnit = cOut.dFreqUnit;
	dTimeUnit = cOut.dTimeUnit;
	cFieldDistr = cOut.cFieldDistr;
	cError = cOut.cError;
	return (*this);
}

void CTmcRTH_IndanOutput::AddPath( CString csPath )
{
	CString csBuf;
	
	if( IsFullPath() ) return;

	csBuf.Format("%s\\%s", csPath, csFileName);
	csFileName = csBuf;
	csBuf.Format("%s\\%s", csPath, csFileNameField);
	csFileNameField = csBuf;
	csBuf.Format("%s\\%s", csPath, csFileNameField1);
	csFileNameField1 = csBuf;
	csBuf.Format("%s\\%s", csPath, csFileNameFieldU);
	csFileNameFieldU = csBuf;
	csBuf.Format("%s\\%s", csPath, csFileNameTopology);
	csFileNameTopology = csBuf;
	csBuf.Format("%s\\%s", csPath, csFileNameSMatrix);
	csFileNameSMatrix = csBuf;
	csBuf.Format("%s\\%s", csPath, csFileNameField1Mod);
	csFileNameField1Mod = csBuf;
	csBuf.Format("%s\\%s", csPath, csFileNameField1Faz);
	csFileNameField1Faz = csBuf;
	csBuf.Format("%s\\%s", csPath, csFileNameQ);
	csFileNameQ = csBuf;
	csBuf.Format("%s\\%s", csPath, csFileNameIx);
	csFileNameIx = csBuf;
	csBuf.Format("%s\\%s", csPath, csFileNameIy);
	csFileNameIy = csBuf;

	cFieldDistr.AddPath( csPath );

	return;
}

BOOL CTmcRTH_IndanOutput::IsFullPath( void )
{
	int i, n;

	n = csFileName.GetLength();
	if( ( n > 0 )&&( ( csFileName[0] == '\\' )||( csFileName[0] == '/' ) ) ) return TRUE;	// N-22: \\server\share, \folder

	for( i = 0; i < n; i++ )
	{
		if( csFileName[i] == ':' ) return TRUE;
	}
	
	return FALSE;
}

void CTmcRTH_IndanOutput::OnOffFieldOutput()
{
	bOutField = !bOutField;
	PutSinchronizFlag( bOutTopology, bOutField, bOutFieldSinchronization);
	return;
}

void CTmcRTH_IndanOutput::OnOffFieldOutput1()
{
	bOutField1 = !bOutField1;
	PutSinchronizFlag( bOutTopology, bOutField1, bOutFieldSinchronization);
	return;
}

void CTmcRTH_IndanOutput::OnOffFieldOutputU()
{
	bOutFieldU = !bOutFieldU;
	PutSinchronizFlag( bOutTopology, bOutFieldU, bOutFieldSinchronization);
	return;
}

void CTmcRTH_IndanOutput::OnOffSinchronizationFieldOutput()
{
	bOutFieldSinchronization = !bOutFieldSinchronization;
	PutSinchronizFlag( bOutTopology, bOutField, bOutFieldSinchronization);
	return;
}

void CTmcRTH_IndanOutput::OnOffSoundEffects()
{
	bSoundEffect = !bSoundEffect;
	if( bSoundEffect )	PutStatistics("Sound on");
	else PutStatistics("Sound off");
	return;
}

void CTmcRTH_IndanOutput::ReadSmatrix( CString &csCh )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	char szBuf1[CTMCRTH_INDANSBUF];
	int i, j, n;
	double r;
	CString csBuf;

	strcpy( szBuf1, csCh );
	i = (int)(strlen(CTMCRTH_SMATRIXOUT_ID));

	i+= Search_1Param( csBuf, szBuf1+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( strlen(csBuf) == 0 )
	{
		csBuf.Format("Missing S matrix file name in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	}
	else
	{
		strcpy( szBuf, csBuf );
		expr_del_Blanks2( szBuf );
		csFileNameSMatrix.Format("%s", szBuf);
		// Есть ли в имени S-файла точка (расширение)? Если нет — ниже добавится ".s".
		// БЫЛО: цикл с csFileNameSMatrix[i] — опечатка индекса (i вместо j) → выход за
		// границу строки → CString::operator[] бросает E_INVALIDARG в совр. MFC (Баг #7).
		n = ( strchr( szBuf, '.' ) != NULL ) ? 1 : 0;
	};

	if( n == 0 ) csFileNameSMatrix.Format("%s.s", szBuf);
	
	i+= Search_1Param( csBuf, szBuf1+i );
	if( cError.IsError() )	// S-10: a missing ';' is not read as the value before
	{
		csBuf.Format("Missing separator ';' in {%s}", (LPCSTR)csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	dFreq = r*dFreqUnit;
	if( !( dFreq > 0.0 ) )	// S-10
	{
		csBuf.Format("In{%s} the frequency must be > 0", (LPCSTR)csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	// B-22: "SMATRIX name; F;" without Tmin; Tmax - the full window 0...Tmax of the task (the multi-frequency run).
	// dTmax < 0 marks it; the window is set when the step is read (CTmcRTH_Indan::ReadOutputSection).
	{
		const char *p = szBuf1 + i;
		while( ( *p == ' ' )||( *p == '\t' )||( *p == '\r' )||( *p == '\n' ) ) p++;
		if( *p == '\0' )
		{
			dTmin = 0.0;
			dTmax = -1.0;
			bOutSMatrix = TRUE;
			return;
		};
	};
	
	i+= Search_1Param( csBuf, szBuf1+i );
	if( cError.IsError() )	// S-10: a missing ';' is not read as the value before
	{
		csBuf.Format("Missing separator ';' in {%s}", (LPCSTR)csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	dTmin = r*dTimeUnit;
	
	i+= Search_1Param( csBuf, szBuf1+i );
	if( cError.IsError() )	// S-10: a missing ';' is not read as the value before
	{
		csBuf.Format("Missing separator ';' in {%s}", (LPCSTR)csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	dTmax = r*dTimeUnit;
	if( !( dTmax > dTmin ) )	// S-10
	{
		csBuf.Format("In{%s} Tmin must be less than Tmax", (LPCSTR)csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	bOutSMatrix = TRUE;
	return;
}

void CTmcRTH_IndanOutput::SetFreqAndTimeUnit(double dFr, double dTim)
{
	dFreqUnit = dFr;
	dTimeUnit = dTim;
	return;
}

void CTmcRTH_IndanOutput::PutSmatrix()
{
	csSMatrixError.Format("");	// A-21
	csSMatrixWarning.Format("");	// B-22
	if( !bOutSMatrix ) return;

//  A run excited by a multi-frequency incident wave carries the whole sweep.
//  The single-frequency TtoS would silently write a wrong .s file, so such a
//  run is converted by the wideband post-processing - the same class TMCROS
//  uses, so both ways give the same file. An old (narrow-band) .t has no
//  wideband header, so IsWidebandFile is FALSE and the former path is kept.
	if( CTmcWideband::IsWidebandFile( csFileName ) )
	{
		CTmcWideband cWide;
		cWide.Process( csFileName, csFileNameSMatrix, dTmin, dTmax );
		csSMatrixWarning = cWide.GetWarningMessage();	// B-22
		if( cWide.IsError() )
		{
			CString csBuf1;
			csBuf1.Format("S-matrix (wideband): %s", (LPCTSTR)cWide.GetErrorMessage() );
			cError.PutErrorMessage( csBuf1 );
			csSMatrixError = csBuf1;	// A-21
		};
		return;
	};

	CTmcTtoS cTtoS;
//	{
//		CString csBuf1;
//		csBuf1.Format("T-FileName = %s; S-FileName = %s; Tmin = %lg; Tmax = %lg; Frequence = %lg;", csFileName, csFileNameSMatrix, dTmin, dTmax, dFreq);
//		AfxMessageBox(csBuf1);
//	};
	cTtoS.TtoS( csFileName, csFileNameSMatrix, dTmin, dTmax, dFreq);
	if( cTtoS.IsError() )	// A-21: ne teryat oshibku
	{
		csSMatrixError.Format("S-matrix: %s", (LPCTSTR)cTtoS.GetErrorMessage() );
	};
//	if( cTtoS.IsError )
//	{
//		CString csBuf1;
//		csBuf1.Format("Error: %s; T-FileName = %s; S-FileName = %s; Tmin = %lg; Tmax = %lg; Frequence = %lg;", cTtoS.GetErrorMessage(), csFileName, csFileNameSMatrix, dTmin, dTmax, dFreq);
//		AfxMessageBox(csBuf1);
//	}
	return;
}

// D-1 (answer of the author 26.10.2026): a step (a point of the sweep) calculated with an error is written into
// the .s as zeros - the whole S-matrix at the frequency of SMATRIX, all columns - so the error is seen at once.
// pcBl - the blocks of the step: the inputs give the number of ports. A port fed from a signal file ([name]) is
// the multi-frequency run: it puts zeros at its bad frequencies itself (CTmcWideband), nothing is written here.
// Returns 1 - zeros are written, 0 - nothing to write, -1 - the .s can not be written.
int CTmcRTH_IndanOutput::PutSmatrixZero( CTmcRTH_BlockList *pcBl )
{
	if( !bOutSMatrix ) return 0;
	if( !( dFreq > 0.0 ) ) return 0;
	if( csFileNameSMatrix.GetLength() == 0 ) return 0;

	int i, nPort = 0, nRet = 1;
	for( ; pcBl != NULL; pcBl = pcBl->GetNext() )
	{
		int nT = pcBl->GetnType();
		if( ( nT < CTMCRTH_BLCKNTYPE_INPXLEFT )||( nT > CTMCRTH_BLCKNTYPE_INPYBOT ) ) continue;
		CString csV = pcBl->GetcsVx();
		if( ( csV.GetLength() > 0 )&&( ((LPCSTR)csV)[0] == '[' ) ) return 0;
		nPort++;
	};
	if( nPort < 1 ) return 0;

	_complex *pcZ = new _complex[nPort];
	for( i = 0; i < nPort; i++ ) { pcZ[i].x = 0.0; pcZ[i].y = 0.0; };
	char szBuf[CTMCRTH_INDANSBUF];
	strncpy( szBuf, (LPCSTR)csFileNameSMatrix, CTMCRTH_INDANSBUF - 1 );
	szBuf[CTMCRTH_INDANSBUF - 1] = '\0';
	for( i = 1; i <= nPort; i++ )
	{
		if( save_S_matrix_element( i, szBuf, pcZ, dFreq, nPort ) != 0 ) { nRet = -1; break; };
	};
	delete []pcZ;
	return nRet;
}

void CTmcRTH_IndanOutput::ReadFileDistr( CString &csCh, char *ch )
{
	if( cError.IsError() ) return;
	
	char szBuf[CTMCRTH_INDANSBUF];
	char szBuf1[CTMCRTH_INDANSBUF];
	int i;
	double r, rFreq, rTmin, rTmax;
	CString csBuf;

	strcpy( szBuf1, csCh );
	i = (int)(strlen(ch));

	i+= Search_1Param( csBuf, szBuf1+i );
	
	if( cError.IsError() ) 
	{
		csBuf.Format("Missing separator ';' in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( strlen(csBuf) == 0 )
	{
		csBuf.Format("Missing fiels distribution file name in {%s}", csCh);
		cError.PutErrorMessage(csBuf);
		return;
	}
	else
	{
		strcpy( szBuf, csBuf );
		expr_del_Blanks2( szBuf );
	};

	cFieldDistr.SetFile(szBuf);

	i+= Search_1Param( csBuf, szBuf1+i );
	if( cError.IsError() )	// S-10: a missing ';' is not read as the value before
	{
		csBuf.Format("Missing separator ';' in {%s}", (LPCSTR)csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	rFreq = r*dFreqUnit;
	if( !( rFreq > 0.0 ) )	// S-10: the window of the pattern is a number of periods of this frequency
	{
		csBuf.Format("In{%s} the frequency must be > 0", (LPCSTR)csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};
	cFieldDistr.SetFreq( rFreq );
 
	i+= Search_1Param( csBuf, szBuf1+i );
	if( cError.IsError() )	// S-10: a missing ';' is not read as the value before
	{
		csBuf.Format("Missing separator ';' in {%s}", (LPCSTR)csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	rTmin = r*dTimeUnit;
	cFieldDistr.SetTmin( rTmin );
	
	i+= Search_1Param( csBuf, szBuf1+i );
	if( cError.IsError() )	// S-10: a missing ';' is not read as the value before
	{
		csBuf.Format("Missing separator ';' in {%s}", (LPCSTR)csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};
	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	rTmax = r*dTimeUnit;
	if( !( rTmax > rTmin ) )	// S-10
	{
		csBuf.Format("In{%s} Tmin must be less than Tmax", (LPCSTR)csCh);
		cError.PutErrorMessage(csBuf);
		return;
	};

	// B-13: a whole number of periods, rounded up with 0.9999 (old: +1 - one more period when the window
	// already was a whole number of periods): rTmax = rTmin + ((int)((rTmax - rTmin)*rFreq+1))/rFreq;
	rTmax = rTmin + ((int)((rTmax - rTmin)*rFreq+0.9999))/rFreq;
	cFieldDistr.SetTmax( rTmax );
	
	return;
}


BOOL CTmcRTH_IndanOutput::bIsFieldDistribution()
{
	return cFieldDistr.bIsFieldDistr();
}

CFieldIntegrated& CTmcRTH_IndanOutput::GetFieldIntegrated()
{
	return cFieldDistr;
}

void CTmcRTH_IndanOutput::ReadFileDistrInMemory( CString &csCh )
{
	if( cError.IsError() ) return;

	ReadFileDistr( csCh, CTMCRTH_FLDSDSMROUT_ID );
	cFieldDistr.SetIntegrateInMemory();
	
	return;
}

