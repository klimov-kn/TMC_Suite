// TmcRTH_IndanOutput.cpp : implementation file
//

#include "stdafx.h"
#include "PlanRT_H.h"
#include "TmcRTH_IndanOutput.h"
#include "TmcRTH_Indan.h"
#include <TmcTtoS.h>
#include <expr.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_IndanOutput

CTmcRTH_IndanOutput::CTmcRTH_IndanOutput()
{
	csFileName.Format("");
	csFileNameField.Format("");
	csFileNameSMatrix.Format("");
	bOutField = FALSE;
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


BEGIN_MESSAGE_MAP(CTmcRTH_IndanOutput, CWnd)
	//{{AFX_MSG_MAP(CTmcRTH_IndanOutput)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_IndanOutput message handlers

void CTmcRTH_IndanOutput::DeleteData( void )
{
	cError.Clear();
	cFieldDistr.DeleteData();
	csFileName.Format("");
	csFileNameSMatrix.Format("");
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
	i = strlen(CTMCRTH_FILEOUT_ID);

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
	
	for( i = strlen(csFileName); i >= 0; i-- )
	{
		if( csFileName[i] == '.' ) 
		{
			csBuf.Format("Unresolved simbol '.' in filename {%s}", csCh);
			cError.PutErrorMessage(csBuf);
			return;
		};
	};

	csFileName.Format("%s.t", szBuf);
	csFileNameField.Format("%s.tf", szBuf);
	csFileNameTopology.Format("%s.tt", szBuf);
	csFileNameSMatrix.Format("%s.s", szBuf);
	
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

	bOutField = TRUE;	
	
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
	bOutField = cOut.bOutField;
	bOutTopology = cOut.bOutTopology;
	bOutFieldSinchronization = cOut.bOutFieldSinchronization;
	bSoundEffect = cOut.bSoundEffect;
	bOutSMatrix = cOut.bOutSMatrix;
	csFileName = cOut.csFileName;
	csFileNameField = cOut.csFileNameField;
	csFileNameTopology = cOut.csFileNameTopology;
	csFileNameSMatrix = cOut.csFileNameSMatrix;
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
	csBuf.Format("%s\\%s", csPath, csFileNameTopology);
	csFileNameTopology = csBuf;
	csBuf.Format("%s\\%s", csPath, csFileNameSMatrix);
	csFileNameSMatrix = csBuf;

	cFieldDistr.AddPath( csPath );

	return;
}

BOOL CTmcRTH_IndanOutput::IsFullPath( void )
{
	int i, n;

	n = csFileName.GetLength();

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
	i = strlen(CTMCRTH_SMATRIXOUT_ID);

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
		for( j = strlen( csFileNameSMatrix ), n = 0; j >= 0; j-- )
		{
			if( csFileNameSMatrix[i] == '.' ) 
			{
				n = 1;
				j = 0;
			};
		};
	};

	if( n == 0 ) csFileNameSMatrix.Format("%s.s", szBuf);
	
	i+= Search_1Param( csBuf, szBuf1+i );
	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	dFreq = r*dFreqUnit;
	
	i+= Search_1Param( csBuf, szBuf1+i );
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
	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	dTmax = r*dTimeUnit;
	
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
	if( !bOutSMatrix ) return;
	CTmcTtoS cTtoS;
//	{
//		CString csBuf1;
//		csBuf1.Format("T-FileName = %s; S-FileName = %s; Tmin = %lg; Tmax = %lg; Frequence = %lg;", csFileName, csFileNameSMatrix, dTmin, dTmax, dFreq);
//		AfxMessageBox(csBuf1);
//	};
	cTtoS.TtoS( csFileName, csFileNameSMatrix, dTmin, dTmax, dFreq);
//	if( cTtoS.IsError )
//	{
//		CString csBuf1;
//		csBuf1.Format("Error: %s; T-FileName = %s; S-FileName = %s; Tmin = %lg; Tmax = %lg; Frequence = %lg;", cTtoS.GetErrorMessage(), csFileName, csFileNameSMatrix, dTmin, dTmax, dFreq);
//		AfxMessageBox(csBuf1);
//	}
	return;
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
	i = strlen(ch);

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
	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	rFreq = r*dFreqUnit;
	cFieldDistr.SetFreq( rFreq );
 
	i+= Search_1Param( csBuf, szBuf1+i );
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
	strcpy( szBuf, csBuf );
	expr_del_Blanks2( szBuf );
	if( i1nte_atof_1( szBuf, &r ) != 0 )
	{
		csBuf.Format("In{%s}{%s}%s", csCh, szBuf, expr_get_error() );
		cError.PutErrorMessage(csBuf);
		return;
	};
	rTmax = r*dTimeUnit;

	rTmax = rTmin + ((int)((rTmax - rTmin)*rFreq+1))/rFreq;
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

