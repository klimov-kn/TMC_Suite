// FieldIntegrated.cpp: implementation of the CFieldIntegrated class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <tmcgrviw.h>
#include <FieldIntegrated.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFieldIntegrated::CFieldIntegrated()
{
	nNodeStep = 2;
	bBorderOnly = FALSE;	// BORDER
	nFreqNum = 0;
	nFreqCur = 0;
	nBX0 = nBX1 = nBY0 = nBY1 = 0;
	nBW = nBHm = 0;
	nBorderSize = 0;
	prSin = NULL;
	prCos = NULL;
	prAmp = NULL;
	prFaz = NULL;

	dT = 0.0;
	bIsIntegretedInMemory = FALSE;
	bIsFieldDistribution = FALSE;
	bIsBeginIntegreted = FALSE;
	dTmin = 0.0;
	dTmax = 0.0;
	dFreq = 0.0;
	nX = 0;
	nY = 0;
	nEmitterInFile = 0;
	dDirPatXmin = -FLT_MAX;
	dDirPatXmax = -FLT_MAX;
	dDirPatYmin = -FLT_MAX;
	dDirPatYmax = -FLT_MAX;
	
	csFileSin.Format("");
	csFileCos.Format("");
	csFileFaza.Format("");
	csFileAmplitude.Format("");
	csFileDirectPattern.Format("");

	cError.Clear();
	return;
}

void CFieldIntegrated::SetNodeStep( int nStep )
{
	if( nStep > 0 ) nNodeStep = nStep;
	return;
}

CFieldIntegrated::~CFieldIntegrated()
{
	DeleteData();
	return;
}

void CFieldIntegrated::DeleteData()
{
	if( prCos != NULL )
	{
		delete []prCos;
		prCos = NULL;
	};
	
	if( prSin != NULL )
	{
		delete []prSin;
		prSin = NULL;
	};

	if( prAmp != NULL )
	{
		delete []prAmp;
		prAmp = NULL;
	};
	
	if( prFaz != NULL )
	{
		delete []prFaz;
		prFaz = NULL;
	};

	bIsIntegretedInMemory = FALSE;
	bIsBeginIntegreted = FALSE;
	bIsFieldDistribution = FALSE;
	dT = 0.0;
	nFreqNum = 0;			// A-41: toni i rezhim granicy ne nasleduyutsya sleduyushchim shagom
	nFreqCur = 0;
	bBorderOnly = FALSE;
	dTmin = 0.0;
	dTmax = 0.0;
	dFreq = 0.0;
	nEmitterInFile = 0;
	nX = 0;
	nY = 0;
	dDirPatXmin = -FLT_MAX;
	dDirPatXmax = -FLT_MAX;
	dDirPatYmin = -FLT_MAX;
	dDirPatYmax = -FLT_MAX;
	
	csFileSin.Format("");
	csFileCos.Format("");
	csFileFaza.Format("");
	csFileAmplitude.Format("");
	csFileDirectPattern.Format("");

	cError.Clear();
	return;
}

void CFieldIntegrated::SetFile(CString csFile_ft, int nX1, int nY1)
{
	SetFile( csFile_ft );
	SetnXnY( nX1, nY1 );
	return;
}

void CFieldIntegrated::SetFile(CString csFile_ft)
{
	DeleteData();
	
	if( strlen(csFile_ft) < 1 )
	{
		cError.PutErrorMessage("Bad file name, lenght < 1");
		return;
	};

	csFileSin  = csFile_ft + ".SIN";
	csFileCos  = csFile_ft + ".COS";
	csFileFaza = csFile_ft + ".FAZ";
	csFileAmplitude = csFile_ft + ".AMP";
	csFileDirectPattern = csFile_ft + ".DAT";

	bIsFieldDistribution = TRUE;

	return;
}

CFieldIntegrated& CFieldIntegrated::operator=( CFieldIntegrated& cFieldIntegr )
{
	TmcIdx i, nNumNode;	// INT64
	DeleteData();

	dT = cFieldIntegr.dT;
	dTmin = cFieldIntegr.dTmin;
	dTmax = cFieldIntegr.dTmax;
	dFreq = cFieldIntegr.dFreq;
	nX = cFieldIntegr.nX;
	nY = cFieldIntegr.nY;

	dDirPatXmin = cFieldIntegr.dDirPatXmin;
	dDirPatXmax = cFieldIntegr.dDirPatXmax;
	dDirPatYmin = cFieldIntegr.dDirPatYmin;
	dDirPatYmax = cFieldIntegr.dDirPatYmax;
	
	csFileSin  = cFieldIntegr.csFileSin;
	csFileCos  = cFieldIntegr.csFileCos;
	csFileFaza = cFieldIntegr.csFileFaza;
	csFileAmplitude = cFieldIntegr.csFileAmplitude;
	csFileDirectPattern = cFieldIntegr.csFileDirectPattern;

	cParam = cFieldIntegr.cParam;
	cError = cFieldIntegr.cError;

	bIsFieldDistribution = cFieldIntegr.bIsFieldDistribution;
	bIsBeginIntegreted   = cFieldIntegr.bIsBeginIntegreted;
	bIsIntegretedInMemory = cFieldIntegr.bIsIntegretedInMemory;

	if( (nX > 1)&&(nY > 1)&&(bIsIntegretedInMemory)&&(cFieldIntegr.prCos != NULL)&&(cFieldIntegr.prSin != NULL) )
	{
		if( prCos != NULL )
		{
			delete []prCos;
			prCos = NULL;
		};
		
		if( prSin != NULL )
		{
			delete []prSin;
			prSin = NULL;
		};

		nNumNode = ((TmcIdx)nX)*nY;	// INT64
		
		if( ( prCos = new _real[ nNumNode ] ) == NULL )
		{
			cError.PutErrorMessage("Error when init array for Cos Field integrate");
			return (*this);
		};
		if( ( prSin = new _real[ nNumNode ] ) == NULL )
		{
			if( prCos != NULL )
			{
				delete []prCos;
				prCos = NULL;
			};
			cError.PutErrorMessage("Error when init array for Sin Field integrate");
			return (*this);
		};
		for( i = 0; i < nNumNode; i++)
		{
			prCos[i] = cFieldIntegr.prCos[i];
			prSin[i] = cFieldIntegr.prSin[i];
		};
	};

	return (*this);
}

void CFieldIntegrated::SetnX(int nX1)
{
	if( nX1 < 1 )
	{
		cError.PutErrorMessage("Bad nX < 1");
		return;
	};
	nX = nX1;
	return;
}

void CFieldIntegrated::SetnY(int nY1)
{
	if( nY1 < 1 )
	{
		cError.PutErrorMessage("Bad nY < 1");
		return;
	};
	nY = nY1;
	return;
}

void CFieldIntegrated::SetnXnY(int nX1, int nY1)
{
	SetnX(nX1);
	SetnY(nY1);

	InitArrayForIntegrate();

	return;
}

void CFieldIntegrated::SetFile(char *szFile_ft)
{
	CString csBuf;
	csBuf.Format("%s", szFile_ft );
	SetFile(csBuf);
	return;
}

void CFieldIntegrated::SetFreq(double dF)
{
	dFreq = dF;
	return;
}


void CFieldIntegrated::SetTmin(double dTmin1)
{
	dTmin = dTmin1;
	return;
}

double CFieldIntegrated::GetTmin()
{
	return dTmin;
}

void CFieldIntegrated::SetTmax(double dTmax1)
{
	dTmax = dTmax1;
	return;
}

double CFieldIntegrated::GetTmax()
{
	return dTmax;
}

BOOL CFieldIntegrated::bIsFieldDistr()
{
	return bIsFieldDistribution;
}

void CFieldIntegrated::InitAllFile( int nX1, int nY1, double dDelta1, double dXmin1, double dYmin1, CTmcRTH_IndanParam& cParam1, double dT1 )
{
	if( IsError() ) return;
	if( !bIsFieldDistribution ) return;

	nTCurrent = 0;
	dTCurrent = 0.0;
	bIsBeginIntegreted = FALSE;
	dT = dT1;
	dDelta = dDelta1;
	dXmin  = dXmin1;
	dYmin  = dYmin1;
	cParam = cParam1;

	SetnXnY( nX1, nY1);

	InitFile( csFileSin );
	InitFile( csFileCos );

	return;
}

// N-24: a file of the field distribution / directional pattern can't be opened - an error, not silence
static void TmcFieldOpenError( CTmcLibError &cErr, const char *pszName, FILE *f1 = NULL, FILE *f2 = NULL, FILE *f3 = NULL )
{
	CString cs;
	if( f1 != NULL ) fclose( f1 );
	if( f2 != NULL ) fclose( f2 );
	if( f3 != NULL ) fclose( f3 );
	cs.Format( "Can't open file %s of FIELD_DISTRIBUTION: the file is used by another program or the folder is read-only", pszName );
	cErr.PutErrorMessage( cs );
	return;
}

void CFieldIntegrated::InitFile(CString csFileName)
{
//		CString csBuf1;
	if( IsError() ) return;
	if( !bIsFieldDistribution ) return;
	if( bIsIntegretedInMemory ) return;

	int i, j;
	FILE *fp;
	_real r;

	r = 0.;

	if( strlen(csFileName) < 1 )
	{
		cError.PutErrorMessage("Bad File name for Field Distribution (lenght of file name < 1)");
		return;
	};

	for( i = 0; ( fp = fopen( csFileName , "wb") ) == NULL; i++ )
	{
		if( i > 2000  ) { TmcFieldOpenError( cError, csFileName ); return; };	// N-24
		Sleep( 10 );
	};
	
	fprintf( fp, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, 0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, 0.0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, (int)( sizeof(_real) ),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, (LPCSTR)( cParam.csLongUnit ), cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, (LPCSTR)( cParam.csTimeUnit ), cParam.rTimeUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nX, nX, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nY, nY);
	for( i = 0; i < nY; i++)
	{
		fprintf( fp, "%c%c", 13, 10);
		for( j = 0; j < nX; j++ )
		{
			fwrite( &(r), sizeof(_real), 1, fp );
		};
	};
	
	fclose(fp);
	SetFileAttributes( csFileName, FILE_ATTRIBUTE_NORMAL );

	return;

}

BOOL CFieldIntegrated::IsError()
{
	return cError.IsError();
}

CString CFieldIntegrated::GetErrorMessage()
{
	return cError.GetErrorMessage();
}


CTmcLibError& CFieldIntegrated::GetError()
{
	return cError;
}

void CFieldIntegrated::AddPath(CString csPath)
{
	CString csBuf;
	
	if( IsFullPath() ) return;

	csBuf.Format("%s\\%s", csPath, csFileSin);
	csFileSin = csBuf;
	csBuf.Format("%s\\%s", csPath, csFileCos);
	csFileCos = csBuf;
	csBuf.Format("%s\\%s", csPath, csFileFaza);
	csFileFaza = csBuf;
	csBuf.Format("%s\\%s", csPath, csFileAmplitude);
	csFileAmplitude = csBuf;
	csBuf.Format("%s\\%s", csPath, csFileDirectPattern);
	csFileDirectPattern = csBuf;

	return;
}

BOOL CFieldIntegrated::IsFullPath()
{
	int i, n;

	n = csFileAmplitude.GetLength();
	if( ( n > 0 )&&( ( csFileAmplitude[0] == '\\' )||( csFileAmplitude[0] == '/' ) ) ) return TRUE;	// N-22

	for( i = 0; i < n; i++ )
	{
		if( csFileAmplitude[i] == ':' ) return TRUE;
	}
	
	return FALSE;
}

void CFieldIntegrated::Integrate(double dTCurrent1, int nTCurrent1, _real *prUNode1)
{
	if( IsError() ) return;
	if( !bIsFieldDistribution ) return;

	dTCurrent = dTCurrent1;
	nTCurrent = nTCurrent1;

	if( bIsBeginIntegreted )
	{
		if( dTCurrent >= dTmax )
		{
			bIsBeginIntegreted = FALSE;
			MakeFileAmplitudeAndFaza();
			MakeFileAmplitudeAndFazaFromMemory();
			ExportToDirectionalPattern();
			return;
		};
	}
	else
	{
		if( dTCurrent < dTmin ) return;
		if( dTCurrent > dTmax ) return;
		bIsBeginIntegreted = TRUE;
	};

	Integrate( csFileCos, prUNode1, (_real)(cos(dTCurrent*2*3.141592653589*dFreq)) );
	Integrate( csFileSin, prUNode1, (_real)(sin(dTCurrent*2*3.141592653589*dFreq)) );

	if( nFreqNum > 1 )	// BORDER
	{
//  every tone gets its own pair of quadratures, side by side in the
//  same array: tone k occupies [k*nBorderSize .. (k+1)*nBorderSize).
		int k;
		for( k = 0; k < nFreqNum; k++ )
			IntegrateInMemoryOneFreq( prUNode1,
				(_real)(cos(dTCurrent*2*3.141592653589*dFreqList[k])),
				(_real)(sin(dTCurrent*2*3.141592653589*dFreqList[k])),
				((TmcIdx)k)*nBorderSize );
	}
	else
		IntegrateInMemory( prUNode1, (_real)(cos(dTCurrent*2*3.141592653589*dFreq)), (_real)(sin(dTCurrent*2*3.141592653589*dFreq))  );

	return;

}

void CFieldIntegrated::Integrate(CString csFileName, _real *prUNode1, _real r1)
{
	if( IsError() ) return;
	if( !bIsFieldDistribution ) return;
	if( bIsIntegretedInMemory ) return;

	int i, j;
	TmcIdx ii;	// INT64
	FILE *fp;
	fpos_t fpos;
	_real r;

	if( strlen(csFileName) < 1 )
	{
		cError.PutErrorMessage("Bad File name for Field Distribution (lenght of file name < 1)");
		return;
	};

	for( i = 0; ( fp = fopen( csFileName , "r+b") ) == NULL; i++ )
	{
		if( i > 2000  ) { TmcFieldOpenError( cError, csFileName ); return; };	// N-24
		Sleep( 10 );
	};
	
	fprintf( fp, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, 0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, 0.0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, (int)( sizeof(_real) ),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, (LPCSTR)( cParam.csLongUnit ), cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, (LPCSTR)( cParam.csTimeUnit ), cParam.rTimeUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nX, nX, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nY, nY);
	for( i = 0, ii = 0; i < nY; i++)
	{
		fprintf( fp, "%c%c", 13, 10);
		fflush( NULL );
		for( j = 0; j < nX; j++ )
		{
			fgetpos( fp, &fpos );
			fflush( fp );
			fread( &(r), sizeof(_real), 1, fp );
			fflush( fp );
			r = (_real)(r + prUNode1[ii]*r1*dT);
//		CString csBuf1;
			fsetpos( fp, &fpos );
			fflush( fp );
			fwrite( &(r), sizeof(_real), 1, fp );
			fflush( fp );
			ii += nNodeStep;
		};
	};
	
	fclose(fp);
	SetFileAttributes( csFileName, FILE_ATTRIBUTE_NORMAL );

	return;

}

void CFieldIntegrated::MakeFileAmplitudeAndFaza()
{
	if( IsError() ) return;
	if( !bIsFieldDistribution ) return;
	if( bIsIntegretedInMemory ) return;

	int i, j;
	FILE *fpSin, *fpCos, *fpAmp, *fpFaz;
	_real rSin, rCos, rFaz, rAmp;
	char szBuf[4];

	if( strlen(csFileSin) < 1 )
	{
		cError.PutErrorMessage("Bad File name for Field Distribution (lenght of file name < 1)");
		return;
	};
	if( strlen(csFileCos) < 1 )
	{
		cError.PutErrorMessage("Bad File name for Field Distribution (lenght of file name < 1)");
		return;
	};
	if( strlen(csFileFaza) < 1 )
	{
		cError.PutErrorMessage("Bad File name for Field Distribution (lenght of file name < 1)");
		return;
	};
	if( strlen(csFileAmplitude) < 1 )
	{
		cError.PutErrorMessage("Bad File name for Field Distribution (lenght of file name < 1)");
		return;
	};

	for( i = 0; ( fpSin = fopen( csFileSin , "r+b") ) == NULL; i++ )
	{
		if( i > 2000  ) { TmcFieldOpenError( cError, csFileSin ); return; };	// N-24
		Sleep( 10 );
	};
	for( i = 0; ( fpCos = fopen( csFileCos , "r+b") ) == NULL; i++ )
	{
		if( i > 2000  ) { TmcFieldOpenError( cError, csFileCos, fpSin ); return; };	// N-24
		Sleep( 10 );
	};
	for( i = 0; ( fpFaz = fopen( csFileFaza , "w+b") ) == NULL; i++ )
	{
		if( i > 2000  ) { TmcFieldOpenError( cError, csFileFaza, fpSin, fpCos ); return; };	// N-24
		Sleep( 10 );
	};
	for( i = 0; ( fpAmp = fopen( csFileAmplitude , "w+b") ) == NULL; i++ )
	{
		if( i > 2000  ) { TmcFieldOpenError( cError, csFileAmplitude, fpSin, fpCos, fpFaz ); return; };	// N-24
		Sleep( 10 );
	};

	fprintf( fpSin, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, 0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, 0.0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, (int)( sizeof(_real) ),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, (LPCSTR)( cParam.csLongUnit ), cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, (LPCSTR)( cParam.csTimeUnit ), cParam.rTimeUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nX, nX, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nY, nY);
	fprintf( fpCos, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, 0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, 0.0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, (int)( sizeof(_real) ),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, (LPCSTR)( cParam.csLongUnit ), cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, (LPCSTR)( cParam.csTimeUnit ), cParam.rTimeUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nX, nX, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nY, nY);
	fprintf( fpFaz, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, nTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, dTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, (int)( sizeof(_real) ),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, (LPCSTR)( cParam.csLongUnit ), cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, (LPCSTR)( cParam.csTimeUnit ), cParam.rTimeUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nX, nX, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nY, nY);
	fprintf( fpAmp, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, nTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, dTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, (int)( sizeof(_real) ),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, (LPCSTR)( cParam.csLongUnit ), cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, (LPCSTR)( cParam.csTimeUnit ), cParam.rTimeUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nX, nX, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nY, nY);

	fflush( NULL );
	for( i = 0; i < nY; i++)
	{
		fread( szBuf, 1, 2, fpSin );
		fread( szBuf, 1, 2, fpCos );
		fprintf( fpFaz, "%c%c", 13, 10);
		fprintf( fpAmp, "%c%c", 13, 10);
		for( j = 0; j < nX; j++ )
		{
			fread( &(rSin), sizeof(_real), 1, fpSin );
			fread( &(rCos), sizeof(_real), 1, fpCos );
			rAmp = (_real)(2*sqrt( rSin*rSin + rCos*rCos )/(dTmax-dTmin));
			rFaz = (_real)(atan2( rSin, rCos )/cParam.rAngleUnit);
			fwrite( &(rAmp), sizeof(_real), 1, fpAmp );
			fwrite( &(rFaz), sizeof(_real), 1, fpFaz );
		};
	};
	
	fclose(fpSin);
	fclose(fpCos);
	fclose(fpAmp);
	fclose(fpFaz);
	
	SetFileAttributes( csFileSin, FILE_ATTRIBUTE_NORMAL );
	SetFileAttributes( csFileCos, FILE_ATTRIBUTE_NORMAL );
	SetFileAttributes( csFileFaza, FILE_ATTRIBUTE_NORMAL );
	SetFileAttributes( csFileAmplitude, FILE_ATTRIBUTE_NORMAL );

	return;

}


void CFieldIntegrated::SetIntegrateInMemory()
{
	bIsIntegretedInMemory = TRUE;
	return;
}

void CFieldIntegrated::InitArrayForIntegrate()
{
	if( IsError() ) return;
	if( !bIsFieldDistribution ) return;
	if( !bIsIntegretedInMemory ) return;
	if( (nX < 1)||(nY < 1) ) return;

	TmcIdx i, nNumNode;	// INT64

	if( prCos != NULL )
	{
		delete []prCos;
		prCos = NULL;
	};
		
	if( prSin != NULL )
	{
		delete []prSin;
		prSin = NULL;
	};

//  how much memory is free right now - the strips are used when four
//  arrays over the whole area would not fit comfortably	// BORDER
	{
		MEMORYSTATUSEX ms;
		ms.dwLength = sizeof(ms);
		if( GlobalMemoryStatusEx( &ms ) )
			SetBorderModeIfNeeded( (double)ms.ullAvailPhys );
		else
			SetBorderModeIfNeeded( 1.0e9 );
	};

	nNumNode = bBorderOnly ? nBorderSize : ((TmcIdx)nX)*nY;	// BORDER
	if( nFreqNum > 1 ) nNumNode *= nFreqNum;	// a pair of quadratures per tone
		
	if( ( prCos = new _real[ nNumNode ] ) == NULL )
	{
		cError.PutErrorMessage("Error when init array for Cos Field integrate");
		return;
	};
	if( ( prSin = new _real[ nNumNode ] ) == NULL )
	{
		if( prCos != NULL )
		{
			delete []prCos;
			prCos = NULL;
		};
		cError.PutErrorMessage("Error when init array for Sin Field integrate");
		return;
	};
	for( i = 0; i < nNumNode; i++)
	{
		prCos[i] = 0.0;
		prSin[i] = 0.0;
	};

	return;
}


//  ================= border-only accumulation // BORDER =================
//
//  The directional pattern is built from a contour. Keeping cos/sin (and
//  later amplitude/phase) for every node of a huge area is pointless: four
//  arrays times 8 bytes per node is more than the grid itself. In border
//  mode only four strips around the contour are kept, three nodes thick -
//  three because ReadAmplitudaAndFazaX/Y asks for Ez one node to each side.
//
//  Strip layout inside the compact array:
//      [0 .. 3*W)                 bottom strip, rows nBY0 .. nBY0+2
//      [3*W .. 6*W)               top strip,    rows nBY1-2 .. nBY1
//      [6*W .. 6*W+3*Hm)          left strip,   cols nBX0 .. nBX0+2
//      [6*W+3*Hm .. 6*W+6*Hm)     right strip,  cols nBX1-2 .. nBX1
//  where W is the strip width and Hm the height of what is left between
//  the bottom and top strips, so no node is stored twice.


//  The signal file may name its tones; the kernel passes that list on.	// BORDER
//  Values come in the units of the task, so they are scaled to Hz here.
//  An empty list means the usual single frequency.
void CFieldIntegrated::SetFreqList( CString csList, double dFreqUnit )
{
	char szBuf[2048];
	char *pS;
	double dV;
	int nCh;

	nFreqNum = 0;
	if( csList.GetLength() < 1 ) return;
	if( csList.GetLength() >= (int)sizeof(szBuf) ) return;

	strcpy( szBuf, (const char *)csList );
	for( pS = szBuf; sscanf( pS, "%lg%n", &dV, &nCh ) == 1; pS += nCh )
	{
		if( nFreqNum >= TMC_DN_MAXFREQ ) break;
		dFreqList[ nFreqNum ] = dV*dFreqUnit;
		nFreqNum++;
	};

	return;
}
void CFieldIntegrated::BorderGeometry( void )
{
	double x0, x1, y0, y1;

//  the same defaults SetDirectionalPatternParamDefault would apply later
	x0 = ( dDirPatXmin < -0.9*FLT_MAX ) ? ( dXmin + 10*dDelta )          : dDirPatXmin;
	x1 = ( dDirPatXmax < -0.9*FLT_MAX ) ? ( dXmin + (nX-10)*dDelta )     : dDirPatXmax;
	y0 = ( dDirPatYmin < -0.9*FLT_MAX ) ? ( dYmin + 10*dDelta )          : dDirPatYmin;
	y1 = ( dDirPatYmax < -0.9*FLT_MAX ) ? ( dYmin + (nY-10)*dDelta )     : dDirPatYmax;

	nBX0 = (int)( (x0 - dXmin)/dDelta + 0.5 ) - 1;
	nBX1 = (int)( (x1 - dXmin)/dDelta + 0.5 ) + 1;
	nBY0 = (int)( (y0 - dYmin)/dDelta + 0.5 ) - 1;
	nBY1 = (int)( (y1 - dYmin)/dDelta + 0.5 ) + 1;

	if( nBX0 < 0 ) nBX0 = 0;
	if( nBY0 < 0 ) nBY0 = 0;
	if( nBX1 > nX-1 ) nBX1 = nX-1;
	if( nBY1 > nY-1 ) nBY1 = nY-1;

	nBW  = nBX1 - nBX0 + 1;
	nBHm = (nBY1 - 2) - (nBY0 + 3) + 1;
	if( nBHm < 0 ) nBHm = 0;

	nBorderSize = ((TmcIdx)6)*nBW + ((TmcIdx)6)*nBHm;
	return;
}

//  Where this grid node lives in the compact array, or -1 if it is not kept.
TmcIdx CFieldIntegrated::BorderSlot( int ix, int iy )
{
	if( !bBorderOnly ) return ((TmcIdx)iy)*nX + ix;

	if( ( ix < nBX0 )||( ix > nBX1 )||( iy < nBY0 )||( iy > nBY1 ) ) return -1;

	if( iy <= nBY0 + 2 )
		return ((TmcIdx)(iy - nBY0))*nBW + (ix - nBX0);

	if( iy >= nBY1 - 2 )
		return ((TmcIdx)3)*nBW + ((TmcIdx)(iy - (nBY1 - 2)))*nBW + (ix - nBX0);

	if( ix <= nBX0 + 2 )
		return ((TmcIdx)6)*nBW + ((TmcIdx)(iy - (nBY0 + 3)))*3 + (ix - nBX0);

	if( ix >= nBX1 - 2 )
		return ((TmcIdx)6)*nBW + ((TmcIdx)3)*nBHm
			 + ((TmcIdx)(iy - (nBY0 + 3)))*3 + (ix - (nBX1 - 2));

	return -1;						// inside the contour: not needed for the pattern
}

//  Decide whether the whole area still fits. Four arrays of _real per node
//  are needed (cos, sin, amplitude, phase); if they would eat more than a
//  quarter of what is free, switch to the strips.
void CFieldIntegrated::SetBorderModeIfNeeded( double dFreeBytes )
{
	double dWhole;

	bBorderOnly = FALSE;

//  a way to exercise the strips on a small task, for checking // BORDER
	{
		const char *psz = getenv( "TMC_FORCE_BORDER" );
		if( ( psz != NULL )&&( *psz == '1' ) )
		{
			bBorderOnly = TRUE;
			BorderGeometry();
			return;
		};
	};

//  a wide-band run is always done on the strips: one pair of quadratures
//  per tone over the whole area would be out of the question	// BORDER
	if( nFreqNum > 1 )
	{
		bBorderOnly = TRUE;
		BorderGeometry();
		return;
	};

	if( (nX < 32)||(nY < 32) ) return;		// too small to bother

	dWhole = 4.0*sizeof(_real)*((double)nX)*((double)nY);
	if( dWhole > 0.25*dFreeBytes ) bBorderOnly = TRUE;

	if( bBorderOnly ) BorderGeometry();
	return;
}

//  Same accumulation as IntegrateInMemory, but into one slice of the array.	// BORDER
//  Used when the run carries several tones; iBase picks the slice.
void CFieldIntegrated::IntegrateInMemoryOneFreq( _real *prUNode1,
		_real rCosCurrent, _real rSinCurrent, TmcIdx iBase )
{
	if( IsError() ) return;
	if( !bIsFieldDistribution ) return;
	if( !bIsIntegretedInMemory ) return;
	if( (prCos == NULL)||(prSin == NULL) ) return;
	if( !bBorderOnly ) return;		// several tones are kept on the strips only

	int i, j, jlo, jhi, nPart;
	TmcIdx ii, jj;

	for( i = nBY0; i <= nBY1; i++ )
	{
		for( nPart = 0; nPart < 2; nPart++ )
		{
			if( ( i <= nBY0 + 2 )||( i >= nBY1 - 2 ) )
			{
				if( nPart > 0 ) break;
				jlo = nBX0;  jhi = nBX1;
			}
			else if( nPart == 0 ) { jlo = nBX0;      jhi = nBX0 + 2; }
			else                  { jlo = nBX1 - 2;  jhi = nBX1;     };

			for( j = jlo; j <= jhi; j++ )
			{
				jj = BorderSlot( j, i );
				if( jj < 0 ) continue;
				jj += iBase;
				ii = ( ((TmcIdx)i)*nX + j )*nNodeStep;
				prCos[jj] = (_real)(prCos[jj] + prUNode1[ii]*rCosCurrent*dT);
				prSin[jj] = (_real)(prSin[jj] + prUNode1[ii]*rSinCurrent*dT);
			};
		};
	};

	return;
}
void CFieldIntegrated::IntegrateInMemory(_real *prUNode1, _real rCosCurrent, _real rSinCurrent)
{
	if( IsError() ) return;
	if( !bIsFieldDistribution ) return;
	if( !bIsIntegretedInMemory ) return;
	if( (prCos == NULL)||(prSin == NULL) )
	{
		cError.PutErrorMessage("Isn't initial prSin or prCos Array for Field Integrate");
		return;
	};

	int i, j;
	TmcIdx ii, jj;	// INT64

	if( bBorderOnly )	// BORDER
	{
//  only the four strips are kept, so only they are accumulated:
//  full rows inside the bottom and top strips, three columns on each
//  side in between. Nothing else is touched.
		int jlo, jhi, nPart;

		for( i = nBY0; i <= nBY1; i++ )
		{
			for( nPart = 0; nPart < 2; nPart++ )
			{
				if( ( i <= nBY0 + 2 )||( i >= nBY1 - 2 ) )
				{
					if( nPart > 0 ) break;		// whole row at once
					jlo = nBX0;  jhi = nBX1;
				}
				else if( nPart == 0 )
				{
					jlo = nBX0;  jhi = nBX0 + 2;	// left column strip
				}
				else
				{
					jlo = nBX1 - 2;  jhi = nBX1;	// right column strip
				};

				for( j = jlo; j <= jhi; j++ )
				{
					jj = BorderSlot( j, i );
					if( jj < 0 ) continue;
					ii = ( ((TmcIdx)i)*nX + j )*nNodeStep;
					prCos[jj] = (_real)(prCos[jj] + prUNode1[ii]*rCosCurrent*dT);
					prSin[jj] = (_real)(prSin[jj] + prUNode1[ii]*rSinCurrent*dT);
				};
			};
		};
		return;
	};

	for( i = 0, ii = 0, jj = 0; i < nY; i++)
	{
		for( j = 0; j < nX; j++ )
		{
			prCos[jj] = (_real)(prCos[jj] + prUNode1[ii]*rCosCurrent*dT);
			prSin[jj] = (_real)(prSin[jj] + prUNode1[ii]*rSinCurrent*dT);
			jj++;
			ii += nNodeStep;
		};
	};
	
	return;
}

void CFieldIntegrated::MakeFileAmplitudeAndFazaFromMemory()
{
	if( IsError() ) return;
	if( !bIsFieldDistribution ) return;
	if( !bIsIntegretedInMemory ) return;
//  on the strips the arrays hold nBorderSize nodes, not nX*nY: the whole-area
//  files cannot be made from them (and are not needed for the pattern)
	if( bBorderOnly ) return;	// BORDER
	if( (prCos == NULL)||(prSin == NULL) )
	{
		cError.PutErrorMessage("Isn't initial prSin or prCos Array for Field Integrate");
		return;
	};
	if( strlen(csFileFaza) < 1 )
	{
		cError.PutErrorMessage("Bad File name for Field Distribution (lenght of file name < 1)");
		return;
	};
	if( strlen(csFileAmplitude) < 1 )
	{
		cError.PutErrorMessage("Bad File name for Field Distribution (lenght of file name < 1)");
		return;
	};

	int i, j;
	TmcIdx jj;	// INT64
	FILE *fpFaz, *fpAmp;
	_real rSin, rCos, rAmp, rFaz;

	for( i = 0; ( fpFaz = fopen( csFileFaza , "w+b") ) == NULL; i++ )
	{
		if( i > 2000  ) { TmcFieldOpenError( cError, csFileFaza ); return; };	// N-24
		Sleep( 10 );
	};
	for( i = 0; ( fpAmp = fopen( csFileAmplitude , "w+b") ) == NULL; i++ )
	{
		if( i > 2000  ) { TmcFieldOpenError( cError, csFileAmplitude, fpFaz ); return; };	// N-24
		Sleep( 10 );
	};

	fprintf( fpFaz, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, nTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, dTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, (int)( sizeof(_real) ),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, (LPCSTR)( cParam.csLongUnit ), cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, (LPCSTR)( cParam.csTimeUnit ), cParam.rTimeUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nX, nX, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nY, nY);
	fprintf( fpAmp, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, nTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, dTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, (int)( sizeof(_real) ),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, (LPCSTR)( cParam.csLongUnit ), cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, (LPCSTR)( cParam.csTimeUnit ), cParam.rTimeUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nX, nX, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nY, nY);


	for( i = 0, jj = 0; i < nY; i++)
	{
		fprintf( fpFaz, "%c%c", 13, 10);
		fprintf( fpAmp, "%c%c", 13, 10);
		for( j = 0; j < nX; j++ )
		{
			rSin = prSin[jj];
			rCos = prCos[jj];
			rAmp = (_real)(2*sqrt( rSin*rSin + rCos*rCos )/(dTmax-dTmin));
			rFaz = (_real)(atan2( rSin, rCos )/cParam.rAngleUnit);
			fwrite( &(rAmp), sizeof(_real), 1, fpAmp );
			fwrite( &(rFaz), sizeof(_real), 1, fpFaz );
			jj++;
		};
	};
	
	fclose(fpAmp);
	fclose(fpFaz);
	
	SetFileAttributes( csFileFaza, FILE_ATTRIBUTE_NORMAL );
	SetFileAttributes( csFileAmplitude, FILE_ATTRIBUTE_NORMAL );

	return;

}

BOOL CFieldIntegrated::bIsItegratedInMemory()
{
	return bIsIntegretedInMemory;
}

void CFieldIntegrated::ExportToDirectionalPattern( CTmcRTH_IndanParam &cParam1 )
{
	if( IsError() ) return;

	cParam = cParam1;
	ExportToDirectionalPattern();

	return;
}

void CFieldIntegrated::ExportToDirectionalPattern()
{
	if( IsError() ) return;

	if( !bBorderOnly ) ReadParamFromFile();	// BORDER
	SetDirectionalPatternParamDefault();

	if( nFreqNum > 1 )	// BORDER
	{
//  one pattern per tone: the file name gets the frequency in MHz so that
//  the whole set can be opened in TMC_DN side by side.
		CString csBase = csFileDirectPattern;
		CString csOne;
		double dFreqSave = dFreq;
		int k, nCut;

		nCut = csBase.ReverseFind( '.' );
		if( nCut > 0 ) csBase = csBase.Left( nCut );

		int nDig, k2;	// N-21: as many digits after the point as the names need to differ
		for( nDig = 0; nDig < 6; nDig++ )
		{
			BOOL bSame = FALSE;
			for( k = 0; ( k < nFreqNum )&&( !bSame ); k++ )
				for( k2 = k + 1; ( k2 < nFreqNum )&&( !bSame ); k2++ )
				{
					CString cs1, cs2;
					cs1.Format( "%.*lf", nDig, dFreqList[k]/1.0e6 );
					cs2.Format( "%.*lf", nDig, dFreqList[k2]/1.0e6 );
					if( strcmp( cs1, cs2 ) == 0 ) bSame = TRUE;
				};
			if( !bSame ) break;
		};

		for( k = 0; k < nFreqNum; k++ )
		{
			nFreqCur = k;
			dFreq    = dFreqList[k];
			csOne.Format( "%s_%.*lfMHz.DAT", (const char *)csBase, nDig, dFreqList[k]/1.0e6 );
			csFileDirectPattern = csOne;
			MakeDirectionalPatternFile();
			if( IsError() ) break;
		};

		nFreqCur = 0;
		dFreq = dFreqSave;
		csFileDirectPattern = csBase + ".DAT";
		return;
	};

	MakeDirectionalPatternFile();

	return;
}

CString CFieldIntegrated::GetcsDirectPatternFile()
{
	return csFileDirectPattern;
}

void CFieldIntegrated::ReadParamFromFile()
{
	if( IsError() ) return;

	if( strlen(csFileFaza) < 1 )
	{
		cError.PutErrorMessage("Bad File name for Field Distribution (lenght of file name < 1)");
		return;
	};
	if( strlen(csFileAmplitude) < 1 )
	{
		cError.PutErrorMessage("Bad File name for Field Distribution (lenght of file name < 1)");
		return;
	};

	FILE *pfDataF;
	int nTCurrentF, nAccuracyF, nXF, nYF;
	double dTCurrentF, dDeltaF, dXminF, dYminF, dLongUnitF, dTimeUnitF;
	CString csLongUnitF, csTimeUnitF;

	FILE *pfDataA;
	int nTCurrentA, nAccuracyA, nXA, nYA;
	double dTCurrentA, dDeltaA, dXminA, dYminA, dLongUnitA, dTimeUnitA;
	CString csLongUnitA, csTimeUnitA;

	if( (pfDataA = fopen(csFileAmplitude, "rb")) == NULL )
	{
		CString csBuf;
		csBuf.Format("can't open data file {%s}", csFileAmplitude);
		cError.PutErrorMessage(csBuf);
		return;
	};
	ReadParamFrom1File( pfDataA, nTCurrentA, nAccuracyA, nXA, nYA, dTCurrentA, dDeltaA, dXminA, dYminA, dLongUnitA, dTimeUnitA, csLongUnitA, csTimeUnitA );
	fclose( pfDataA );
	
	if( (pfDataF = fopen(csFileFaza, "rb")) == NULL )
	{
		CString csBuf;
		csBuf.Format("can't open data file {%s}", csFileFaza);
		cError.PutErrorMessage(csBuf);
		return;
	};
	ReadParamFrom1File( pfDataF, nTCurrentF, nAccuracyF, nXF, nYF, dTCurrentF, dDeltaF, dXminF, dYminF, dLongUnitF, dTimeUnitF, csLongUnitF, csTimeUnitF );
	fclose( pfDataF );

	if(   (nTCurrentA != nTCurrentF)
		||(nAccuracyA != nAccuracyF) 
		||(nXA != nXF) 
		||(nYA != nYF) 
		||(dTCurrentA != dTCurrentF) 
		||(dDeltaA != dDeltaF) 
		||(dXminA != dXminF) 
		||(dYminA != dYminF) 
		||(dLongUnitA != dLongUnitF) 
		||(dTimeUnitA != dTimeUnitF) )	// I-02: the typo dTimeUnitA != dTimeUnitA (always false)
	{
		CString csBuf;
		csBuf.Format("Data in file {%s} and data in file {%s} is different ", csFileAmplitude, csFileFaza);
		cError.PutErrorMessage(csBuf);
		return;
	};
	if( (nAccuracyA != sizeof(_real)) )
	{
		CString csBuf;
		csBuf.Format("Accuracy data in file {%s} = %d and program accuracy = %d is different ", csFileAmplitude, nAccuracyA, sizeof(_real) );
		cError.PutErrorMessage(csBuf);
		return;
	};
	
	nTCurrent = nTCurrentA;
	dTCurrent = dTCurrentA;
	dDelta = dDeltaA;
	dXmin = dXminA;
	dYmin = dYminA;
	cParam.csLongUnit = csLongUnitA;
	cParam.csTimeUnit = csTimeUnitA;
	nY = nYA;
	nX = nXA;

	return;
}

void CFieldIntegrated::ReadParamFrom1File(FILE *pfDataA, int &nTCurrentA, int &nAccuracyA, int &nXA, int &nYA, double &dTCurrentA, double &dDeltaA, double &dXminA, double &dYminA, double &dLongUnitA, double &dTimeUnitA, CString &csLongUnitA, CString &csTimeUnitA)
{
	if( IsError() ) return;
	if( pfDataA == NULL ) return;

	char ch[TMC_GROUT_MAXSTRING_BUF], ch1[TMC_GROUT_MAXSTRING_BUF];

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, pfDataA ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID, strlen(TMC_GROFLD_DOCFILE_ID) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, pfDataA ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_nT, strlen(TMC_GROFLD_DOCFILE_ID_nT) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_nT), "%d",  &(nTCurrentA) );

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, pfDataA ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_T, strlen(TMC_GROFLD_DOCFILE_ID_T) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_T), "%lg",  &(dTCurrentA) );

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, pfDataA ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_Delta, strlen(TMC_GROFLD_DOCFILE_ID_Delta) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_Delta), "%lg",  &(dDeltaA) );

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, pfDataA ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_Xmin, strlen(TMC_GROFLD_DOCFILE_ID_Xmin) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_Xmin), "%lg",  &(dXminA) );

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, pfDataA ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_Ymin, strlen(TMC_GROFLD_DOCFILE_ID_Ymin) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_Ymin), "%lg",  &(dYminA) );

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, pfDataA ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_Accuracy, strlen(TMC_GROFLD_DOCFILE_ID_Accuracy) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_Accuracy), "%d",  &(nAccuracyA) );

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, pfDataA ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_LongUnit, strlen(TMC_GROFLD_DOCFILE_ID_LongUnit) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_LongUnit), "%s %lg",  ch1,  &(dLongUnitA) );
	csLongUnitA.Format( "%s",  ch1);

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, pfDataA ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_TimeUnit, strlen(TMC_GROFLD_DOCFILE_ID_TimeUnit) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_TimeUnit), "%s %lg",  ch1,  &(dTimeUnitA) );
	csTimeUnitA.Format( "%s",  ch1);

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, pfDataA ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_nX, strlen(TMC_GROFLD_DOCFILE_ID_nX) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_nX), "%d",  &(nXA) );

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, pfDataA ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_nY, strlen(TMC_GROFLD_DOCFILE_ID_nY) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_nY), "%d",  &(nYA) );

	return;
}

void CFieldIntegrated::SetDirectionalPatternParamDefault()
{
	if( IsError() ) return;

	if( dDirPatXmin < -0.9*FLT_MAX )
	{
		dDirPatXmin = dXmin + 10*dDelta;
	};

	if( dDirPatXmax < -0.9*FLT_MAX )
	{
		/// Б-14 (автор 29.10): контур несимметричен — 10 шагов от X_MIN/Y_MIN и 8 от X_MAX/Y_MAX (nX включает
		/// запасной столбец, Б-2.2). На ДН это не влияет: контур — решётка, каждая сторона которой излучает в свою
		/// полуплоскость. Оставлено как есть, ключ контура не вводится.
		dDirPatXmax = dXmin + (nX-10)*dDelta;
	};

	if( dDirPatYmin < -0.9*FLT_MAX )
	{
		dDirPatYmin = dYmin + 10*dDelta;
	};

	if( dDirPatYmax < -0.9*FLT_MAX )
	{
		dDirPatYmax = dYmin + (nY-10)*dDelta;
	};

	return;
}


//  Amplitude and phase straight from the accumulated quadratures // BORDER
//  In border mode there are no .AMP/.FAZ files to read back: the strips
//  live in memory and the pattern is built from them directly. The formula
//  is the same one MakeFileAmplitudeAndFazaFromMemory writes to disk.
void CFieldIntegrated::MakeAmpFazFromMemory( void )
{
	if( IsError() ) return;
	if( (prCos == NULL)||(prSin == NULL) )
	{
		cError.PutErrorMessage("Isn't initial prSin or prCos Array for Field Integrate");
		return;
	};

	TmcIdx n = bBorderOnly ? nBorderSize : ((TmcIdx)nX)*nY;
	TmcIdx iBase = ((TmcIdx)nFreqCur)*n;	// which tone we are building now
	TmcIdx i;
	_real rSin, rCos;

	if( prAmp != NULL ) { delete []prAmp; prAmp = NULL; };
	if( prFaz != NULL ) { delete []prFaz; prFaz = NULL; };

	if( ( prAmp = new _real[ n ] ) == NULL )
	{
		cError.PutErrorMessage("Error when init amplitude array for directional pattern");
		return;
	};
	if( ( prFaz = new _real[ n ] ) == NULL )
	{
		delete []prAmp;
		prAmp = NULL;
		cError.PutErrorMessage("Error when init phase array for directional pattern");
		return;
	};

	for( i = 0; i < n; i++ )
	{
		rSin = prSin[i + iBase];
		rCos = prCos[i + iBase];
		prAmp[i] = (_real)(2*sqrt( rSin*rSin + rCos*rCos )/(dTmax-dTmin));
		prFaz[i] = (_real)(atan2( rSin, rCos )/cParam.rAngleUnit);
	};

	return;
}
void CFieldIntegrated::MakeDirectionalPatternFile()
{
	if( IsError() ) return;
	if( !IsDirectionalPatternDataCorrect() ) return;
	MakeDirectionalPatternFileZagolovok();
	if( bBorderOnly ) MakeAmpFazFromMemory();	// BORDER
	else              ReadAmplitudaAndFazaEzMInit();
	AddDirectionalPatternFileXmin();
	AddDirectionalPatternFileXmax();
	AddDirectionalPatternFileYmin();
	AddDirectionalPatternFileYmax();
}

BOOL CFieldIntegrated::IsDirectionalPatternDataCorrect()
{
	if( IsError() ) return FALSE;

	if( dDirPatXmin < (dXmin+9*dDelta) )
	{
		cError.PutErrorMessage("Isn't correct data for make directional pattern data file: dDirPatXmin < dXmin+9*delta ");
		return FALSE;
	};
	if( dDirPatXmax > (dXmin+nX*dDelta-9*dDelta) )
	{
		cError.PutErrorMessage("Isn't correct data for make directional pattern data file: dDirPatXmax > dXmax-9*delta ");
		return FALSE;
	};
	if( dDirPatYmin < (dYmin+9*dDelta) )
	{
		cError.PutErrorMessage("Isn't correct data for make directional pattern data file: dDirPatYmin < dYmin+9*delta ");
		return FALSE;
	};
	if( dDirPatYmax > (dYmin+nY*dDelta-9*dDelta) )
	{
		cError.PutErrorMessage("Isn't correct data for make directional pattern data file: dDirPatYmax > dYmax-9*delta ");
		return FALSE;
	};
	if( dFreq < FLT_MIN )
	{
		CString csBuf;
		csBuf.Format("Isn't correct data for make directional pattern data file: Frequency (%lg) < %lg", dFreq, FLT_MIN);
		cError.PutErrorMessage(csBuf);
		return FALSE;
	};
	if( dDelta < FLT_MIN )
	{
		CString csBuf;
		csBuf.Format("Isn't correct data for make directional pattern data file: Delta (%lg) < %lg", dDelta, FLT_MIN);
		cError.PutErrorMessage(csBuf);
		return FALSE;
	};

	return TRUE;
}

void CFieldIntegrated::MakeDirectionalPatternFileZagolovok()
{
	if( IsError() ) return;
	if( strlen( csFileDirectPattern ) < 1 )
	{
		cError.PutErrorMessage("Output File name is NULL for Directional pattern");
		return;
	}

	FILE *fp;
	double f;
	CString csBuf;
	int nIzlX, nIzlY;

	nEmitterInFile = 0;

	if( ( fp = fopen( csFileDirectPattern , "w") ) == NULL ) 
	{
		cError.PutErrorMessage("Error when open output data file for directional pattern");
		return;
	};


	f = dFreq/1000000000.;
	if( fprintf( fp, "Frequence = %lg Ghz;\n", f ) < 0 ) 
	{
		cError.PutErrorMessage("Error when write output data file for directional pattern");
		fclose( fp );
		return;
	};

	nIzlX = (2*3*GetNumberOfEmittersAlongX() );
	if( nIzlX < 1 ) 
	{
		cError.PutErrorMessage("Error when write output data file for directional pattern N of emitters along X axies < 1");
		fclose( fp );
		return;
	};
	nIzlY = (2*3*GetNumberOfEmittersAlongY());
	if( nIzlY < 1 ) 
	{
		cError.PutErrorMessage("Error when write output data file for directional pattern N of emitters along Y axies < 1");
		fclose( fp );
		return;
	};

	if( fprintf( fp, "N = %d;\n", nIzlX+nIzlY ) < 0 ) 
	{
		cError.PutErrorMessage("Error when write output data file for directional pattern");
		fclose( fp );
		return;
	};

	if( fprintf( fp, "%s\n", "1." ) < 0 ) 
	{
		cError.PutErrorMessage("Error when write output data file for directional pattern");
		fclose( fp );
		return;
	};

	fclose( fp );
	return;
}

int CFieldIntegrated::GetNumberOfEmittersAlongX()
{
	if( IsError() ) return 0;
	if( dDelta < FLT_MIN )
	{
		CString csBuf;
		csBuf.Format("Isn't correct data for make directional pattern data file: Delta (%lg) < %lg", dDelta, FLT_MIN);
		cError.PutErrorMessage(csBuf);
		return 0;
	};

	return (int)( (dDirPatXmax - dDirPatXmin)/dDelta);
}

int CFieldIntegrated::GetNumberOfEmittersAlongY()
{
	if( IsError() ) return 0;
	if( dDelta < FLT_MIN )
	{
		CString csBuf;
		csBuf.Format("Isn't correct data for make directional pattern data file: Delta (%lg) < %lg", dDelta, FLT_MIN);
		cError.PutErrorMessage(csBuf);
		return 0;
	};

	return (int)( (dDirPatYmax - dDirPatYmin)/dDelta );
}

void CFieldIntegrated::AddDirectionalPatternFileXmin()
{
	if( IsError() ) return;

	int i;
	int nIzl = GetNumberOfEmittersAlongY();
	double x, y, aEz, fEz, aHx, fHx, aHy, fHy;

	for( i = 0, x = dDirPatXmin, y = dDirPatYmin + dDelta/2.; i < nIzl; i++, y += dDelta )
	{
		ReadAmplitudaAndFazaX( x, y, aEz, fEz, aHx, fHx, aHy, fHy );
		if( cos( fHy - fEz ) >= 0.0 )
		{
			aEz = 0.0;
			aHy = 0.0;
		};
		fEz *= -1.0;
		fHy *= -1.0;
		AddDirectionalPatternFileXmin( x, y, aEz, fEz, aHx, fHx, aHy, fHy );
	};
	
	return;
}

void CFieldIntegrated::AddDirectionalPatternFileXmax()
{
	if( IsError() ) return;

	int i;
	int nIzl = GetNumberOfEmittersAlongY();
	double x, y, aEz, fEz, aHx, fHx, aHy, fHy;

	for( i = 0, x = dDirPatXmax, y = dDirPatYmin + dDelta/2.; i < nIzl; i++, y += dDelta )
	{
		ReadAmplitudaAndFazaX( x, y, aEz, fEz, aHx, fHx, aHy, fHy );
		if( cos( fHy - fEz ) <= 0.0 )
		{
			aEz = 0.0;
			aHy = 0.0;
		};
		fEz *= -1.0;
		fHy *= -1.0;
		AddDirectionalPatternFileXmax( x, y, aEz, fEz, aHx, fHx, aHy, fHy );
	};
	
	return;
}

void CFieldIntegrated::AddDirectionalPatternFileYmin()
{
	if( IsError() ) return;

	int i;
	int nIzl = GetNumberOfEmittersAlongX();
	double x, y, aEz, fEz, aHx, fHx, aHy, fHy;

	for( i = 0, x = dDirPatXmin + dDelta/2., y = dDirPatYmin; i < nIzl; i++, x += dDelta )
	{
		ReadAmplitudaAndFazaY( x, y, aEz, fEz, aHx, fHx, aHy, fHy );
		if( cos( fHx - fEz ) >= 0.0 )
		{
			aEz = 0.0;
			aHx = 0.0;
		};
		fEz *= -1.0;
		fHx *= -1.0;
		AddDirectionalPatternFileYmin( x, y, aEz, fEz, aHx, fHx, aHy, fHy );
	};
	
	return;
}

void CFieldIntegrated::AddDirectionalPatternFileYmax()
{
	if( IsError() ) return;

	int i;
	int nIzl = GetNumberOfEmittersAlongX();
	double x, y, aEz, fEz, aHx, fHx, aHy, fHy;

	for( i = 0, x = dDirPatXmin + dDelta/2., y = dDirPatYmax; i < nIzl; i++, x += dDelta )
	{
		ReadAmplitudaAndFazaY( x, y, aEz, fEz, aHx, fHx, aHy, fHy );
		if( cos( fHx - fEz ) <= 0.0 )
		{
			aEz = 0.0;
			aHx = 0.0;
		};
		fEz *= -1.0;
		fHx *= -1.0;
		AddDirectionalPatternFileYmax( x, y, aEz, fEz, aHx, fHx, aHy, fHy );
	};
	
	return;
}

void CFieldIntegrated::ReadAmplitudaAndFazaX(double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy)
{
	if( IsError() ) return;

	double aEz1, fEz1, aEz2, fEz2, aEz3, fEz3, aEz4, fEz4, koef;

	koef = 299792458./(2*3.141592653589*dFreq)/dDelta;

	ReadAmplitudaAndFazaEz( x, y - dDelta/2, aEz1, fEz1);
	ReadAmplitudaAndFazaEz( x, y + dDelta/2, aEz2, fEz2);
	aEz = 0.5*sqrt( (aEz1*cos(fEz1)+aEz2*cos(fEz2))*(aEz1*cos(fEz1)+aEz2*cos(fEz2)) + (aEz1*sin(fEz1)+aEz2*sin(fEz2))*(aEz1*sin(fEz1)+aEz2*sin(fEz2)) );
	fEz = atan2( (aEz1*sin(fEz1)+aEz2*sin(fEz2)), (aEz1*cos(fEz1)+aEz2*cos(fEz2)) );
	ReadAmplitudaAndFazaEz( x - dDelta, y - dDelta/2, aEz1, fEz1);
	ReadAmplitudaAndFazaEz( x - dDelta, y + dDelta/2, aEz2, fEz2);
	ReadAmplitudaAndFazaEz( x + dDelta, y + dDelta/2, aEz3, fEz3);
	ReadAmplitudaAndFazaEz( x + dDelta, y - dDelta/2, aEz4, fEz4);
	aHy = koef*0.25*sqrt( (aEz1*cos(fEz1)+aEz2*cos(fEz2)-aEz3*cos(fEz3)-aEz4*cos(fEz4))*(aEz1*cos(fEz1)+aEz2*cos(fEz2)-aEz3*cos(fEz3)-aEz4*cos(fEz4)) + (aEz1*sin(fEz1)+aEz2*sin(fEz2)-aEz3*sin(fEz3)-aEz4*sin(fEz4))*(aEz1*sin(fEz1)+aEz2*sin(fEz2)-aEz3*sin(fEz3)-aEz4*sin(fEz4)) );
	fHy = atan2( (aEz1*sin(fEz1)+aEz2*sin(fEz2)-aEz3*sin(fEz3)-aEz4*sin(fEz4)), (aEz1*cos(fEz1)+aEz2*cos(fEz2)-aEz3*cos(fEz3)-aEz4*cos(fEz4)) ) + 3.141592653589/2.;
	aHx = 0.0;
	fHx = 0.0;

	return;
}

void CFieldIntegrated::ReadAmplitudaAndFazaY(double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy)
{
	if( IsError() ) return;

	double aEz1, fEz1, aEz2, fEz2, aEz3, fEz3, aEz4, fEz4, koef;

	koef = 299792458./(2*3.141592653589*dFreq)/dDelta;

	ReadAmplitudaAndFazaEz( x - dDelta/2, y, aEz1, fEz1);
	ReadAmplitudaAndFazaEz( x + dDelta/2, y, aEz2, fEz2);
	aEz = 0.5*sqrt( (aEz1*cos(fEz1)+aEz2*cos(fEz2))*(aEz1*cos(fEz1)+aEz2*cos(fEz2)) + (aEz1*sin(fEz1)+aEz2*sin(fEz2))*(aEz1*sin(fEz1)+aEz2*sin(fEz2)) );
	fEz = atan2( (aEz1*sin(fEz1)+aEz2*sin(fEz2)), (aEz1*cos(fEz1)+aEz2*cos(fEz2)) );
	ReadAmplitudaAndFazaEz( x - dDelta/2, y - dDelta, aEz1, fEz1);
	ReadAmplitudaAndFazaEz( x - dDelta/2, y + dDelta, aEz4, fEz4);
	ReadAmplitudaAndFazaEz( x + dDelta/2, y + dDelta, aEz3, fEz3);
	ReadAmplitudaAndFazaEz( x + dDelta/2, y - dDelta, aEz2, fEz2);
	aHx = koef*0.25*sqrt( (aEz1*cos(fEz1)+aEz2*cos(fEz2)-aEz3*cos(fEz3)-aEz4*cos(fEz4))*(aEz1*cos(fEz1)+aEz2*cos(fEz2)-aEz3*cos(fEz3)-aEz4*cos(fEz4)) + (aEz1*sin(fEz1)+aEz2*sin(fEz2)-aEz3*sin(fEz3)-aEz4*sin(fEz4))*(aEz1*sin(fEz1)+aEz2*sin(fEz2)-aEz3*sin(fEz3)-aEz4*sin(fEz4)) );
	fHx = atan2( (aEz1*sin(fEz1)+aEz2*sin(fEz2)-aEz3*sin(fEz3)-aEz4*sin(fEz4)), (aEz1*cos(fEz1)+aEz2*cos(fEz2)-aEz3*cos(fEz3)-aEz4*cos(fEz4)) ) +  3.141592653589/2.;
	aHy = 0.0;
	fHy = 0.0;

	return;
}

void CFieldIntegrated::AddDirectionalPatternFileYmin(double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy)
{
	if( IsError() ) return;
	double dFimin = 180.001;
	double dFimax = 359.999;
	AddDirectionalPatternFileOneEmitter( x, y, aEz, fEz, aHx, fHx, aHy, fHy, dFimin, dFimax);
	return;
}

void CFieldIntegrated::AddDirectionalPatternFileYmax(double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy)
{
	if( IsError() ) return;
	double dFimin = 0.001;
	double dFimax = 179.999;
	AddDirectionalPatternFileOneEmitter( x, y, aEz, fEz, aHx, fHx, aHy, fHy, dFimin, dFimax);
	return;
}

void CFieldIntegrated::AddDirectionalPatternFileXmin(double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy)
{
	if( IsError() ) return;
	double dFimin = 90.001;
	double dFimax = 269.999;
	AddDirectionalPatternFileOneEmitter( x, y, aEz, fEz, aHx, fHx, aHy, fHy, dFimin, dFimax);
	return;
}

void CFieldIntegrated::AddDirectionalPatternFileXmax(double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy)
{
	if( IsError() ) return;
	double dFimin = -90.001;
	double dFimax =  89.999;
	AddDirectionalPatternFileOneEmitter( x, y, aEz, fEz, aHx, fHx, aHy, fHy, dFimin, dFimax);
	return;
}

void CFieldIntegrated::ReadAmplitudaAndFazaEz(double x, double y, double &aEz, double &fEz)
{
	if( IsError() ) return;

	TmcIdx i;	// INT64
	int j, nX1, nY1;
	_real rBufA, rBufF;

	aEz = 0.0;
	fEz = 0.0;

	nX1 = (int)((x+0.5*dDelta-dXmin)/dDelta);
	nY1 = (int)((y+0.5*dDelta-dYmin)/dDelta);

	if( (prAmp != NULL)&&(prFaz != NULL) )
	{
		TmcIdx iSlot = BorderSlot( nX1, nY1 );	// BORDER
		if( iSlot < 0 )
		{
//  outside the strips the field was not accumulated; for the pattern
//  such a point is never asked, so zero is the honest answer.
			aEz = 0.0;
			fEz = 0.0;
			return;
		};
		aEz = prAmp[ iSlot ];
		// I-01: the phase is stored in ANGLE_UNIT - back to radians (old: fEz = prFaz[ iSlot ]; - wrong for gradus)
		/// I-01: фаза в массиве — в единицах ANGLE_UNIT (atan2/rAngleUnit), дальше нужна в радианах.
		fEz = prFaz[ iSlot ]*cParam.rAngleUnit;
		return;		
	};


	FILE *pfDataF;
	int nTCurrentF, nAccuracyF, nXF, nYF;
	double dTCurrentF, dDeltaF, dXminF, dYminF, dLongUnitF, dTimeUnitF;
	CString csLongUnitF, csTimeUnitF;

	FILE *pfDataA;
	int nTCurrentA, nAccuracyA, nXA, nYA;
	double dTCurrentA, dDeltaA, dXminA, dYminA, dLongUnitA, dTimeUnitA;
	CString csLongUnitA, csTimeUnitA;

	char ch;

	if( (pfDataA = fopen(csFileAmplitude, "rb")) == NULL )
	{
		CString csBuf;
		csBuf.Format("can't open data file {%s}", csFileAmplitude);
		cError.PutErrorMessage(csBuf);
		return;
	};
	ReadParamFrom1File( pfDataA, nTCurrentA, nAccuracyA, nXA, nYA, dTCurrentA, dDeltaA, dXminA, dYminA, dLongUnitA, dTimeUnitA, csLongUnitA, csTimeUnitA );
	
	if( (pfDataF = fopen(csFileFaza, "rb")) == NULL )
	{
		CString csBuf;
		csBuf.Format("can't open data file {%s}", csFileFaza);
		cError.PutErrorMessage(csBuf);
		fclose( pfDataA );
		return;
	};
	ReadParamFrom1File( pfDataF, nTCurrentF, nAccuracyF, nXF, nYF, dTCurrentF, dDeltaF, dXminF, dYminF, dLongUnitF, dTimeUnitF, csLongUnitF, csTimeUnitF );

	if( IsError() ) 
	{
		fclose( pfDataF );
		fclose( pfDataA );
		return;
	};

	for( i = 0; i < nY1; i++ )
	{
		for( j = 0; j < nX; j++ )
		{
			if( fread( &(rBufF), sizeof(_real), 1, pfDataF ) != 1 )
			{
				cError.PutErrorMessage("when read Faza data file");
				fclose( pfDataF );
				fclose( pfDataA );
				return;
			};
			if( fread( &(rBufA), sizeof(_real), 1, pfDataA ) != 1 )
			{
				cError.PutErrorMessage("when read Amplit. data file");
				fclose( pfDataF );
				fclose( pfDataA );
				return;
			};
		};
		fread( &(ch), 1, 1, pfDataF );
		fread( &(ch), 1, 1, pfDataF );
		fread( &(ch), 1, 1, pfDataA );
		fread( &(ch), 1, 1, pfDataA );
	};

	for( j = 0; j < nX1; j++ )
	{
		if( fread( &(rBufF), sizeof(_real), 1, pfDataF ) != 1 )
		{
			cError.PutErrorMessage("when read Faza data file");
			fclose( pfDataF );
			fclose( pfDataA );
			return;
		};
		if( fread( &(rBufA), sizeof(_real), 1, pfDataA ) != 1 )
		{
			cError.PutErrorMessage("when read Amplit. data file");
			fclose( pfDataF );
			fclose( pfDataA );
			return;
		};
	};

	aEz = rBufA;
	// I-01: the .FAZ file is in ANGLE_UNIT - back to radians (old: fEz = rBufF;)
	fEz = rBufF*cParam.rAngleUnit;

	fclose( pfDataF );
	fclose( pfDataA );
	return;
}

void CFieldIntegrated::AddDirectionalPatternFileOneEmitter(double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy, double &dFimin, double &dFimax)
{
	if( IsError() ) return;

	FILE *fp;
	CString csBuf;
	double koef;

	if( ( fp = fopen( csFileDirectPattern , "a") ) == NULL ) 
	{
		cError.PutErrorMessage("Error when add emitter to output data file for directional pattern");
		return;
	};

//	koef = 0.5*(dDelta*299792458/dFreq)*sqrt(sqrt((12.566370614e-7)/(8.854187817e-12)));
//	double dW0, dA_111, dDkL;
	double dDkL;
//	CString csBuf1;

	dDkL = (dDelta*dFreq/299792458);
	/// З-13 (автор 29.10): калибровка СЕТОЧНОЙ функции Грина двумерной задачи. «/3» — точка контура даёт три
	/// излучателя, образующих элемент Гюйгенса: электрический вибратор (Ez, ДН «1.») и магнитный ток (Hx, Hy,
	/// ДН «sin(f)», «cos(f)»); на каждой стороне контура элемент свой и смотрит наружу, ориентация (cos/sin) —
	/// по типу границы (З-14).
	koef = sqrt(dDelta/3.*0.999996/0.9361811)*dDkL/sqrt(dFreq/299792458);

	nEmitterInFile++;
	csBuf.Format("1.");
	if( fprintf( fp, "%d   X = %lg mm; Y = %lg mm; Amplitude = %lg; Faza = %lg degree; FiMin = %lg degree; FiMax = %lg degree; %s\n", nEmitterInFile, x*1000., y*1000., aEz*koef, fEz/3.141592653589*180., dFimin, dFimax, (LPCSTR)( csBuf ) ) < 0 ) 
	{
		cError.PutErrorMessage("Error when add emitter to output data file for directional pattern");
		fclose(fp);
		return;
	};

	nEmitterInFile++;
	csBuf.Format("cos(f)");
	if( fprintf( fp, "%d   X = %lg mm; Y = %lg mm; Amplitude = %lg; Faza = %lg degree; FiMin = %lg degree; FiMax = %lg degree; %s\n", nEmitterInFile, x*1000., y*1000., aHy*koef, fHy/3.141592653589*180., dFimin, dFimax, (LPCSTR)( csBuf ) ) < 0 ) 
	{
		cError.PutErrorMessage("Error when add emitter to output data file for directional pattern");
		fclose(fp);
		return;
	};

	nEmitterInFile++;
	csBuf.Format("sin(f)");
	if( fprintf( fp, "%d   X = %lg mm; Y = %lg mm; Amplitude = %lg; Faza = %lg degree; FiMin = %lg degree; FiMax = %lg degree; %s\n", nEmitterInFile, x*1000., y*1000., aHx*koef, fHx/3.141592653589*180., dFimin, dFimax, (LPCSTR)( csBuf ) ) < 0 ) 
	{
		cError.PutErrorMessage("Error when add emitter to output data file for directional pattern");
		fclose(fp);
		return;
	};

	fclose(fp);
	return;
}

void CFieldIntegrated::ReadAmplitudaAndFazaEzMInit()
{
	if( IsError() ) return;

	if( prAmp != NULL )
	{
		delete []prAmp;
		prAmp = NULL;
	};
	
	if( prFaz != NULL )
	{
		delete []prFaz;
		prFaz = NULL;
	};

	int i, j;

	FILE *pfDataF;
	int nTCurrentF, nAccuracyF, nXF, nYF;
	double dTCurrentF, dDeltaF, dXminF, dYminF, dLongUnitF, dTimeUnitF;
	CString csLongUnitF, csTimeUnitF;

	FILE *pfDataA;
	int nTCurrentA, nAccuracyA, nXA, nYA;
	double dTCurrentA, dDeltaA, dXminA, dYminA, dLongUnitA, dTimeUnitA;
	CString csLongUnitA, csTimeUnitA;

	char ch;

	if( (pfDataA = fopen(csFileAmplitude, "rb")) == NULL )
	{
		CString csBuf;
		csBuf.Format("can't open data file {%s}", csFileAmplitude);
		cError.PutErrorMessage(csBuf);
		return;
	};
	ReadParamFrom1File( pfDataA, nTCurrentA, nAccuracyA, nXA, nYA, dTCurrentA, dDeltaA, dXminA, dYminA, dLongUnitA, dTimeUnitA, csLongUnitA, csTimeUnitA );
	
	if( (pfDataF = fopen(csFileFaza, "rb")) == NULL )
	{
		CString csBuf;
		csBuf.Format("can't open data file {%s}", csFileFaza);
		cError.PutErrorMessage(csBuf);
		fclose( pfDataA );
		return;
	};
	ReadParamFrom1File( pfDataF, nTCurrentF, nAccuracyF, nXF, nYF, dTCurrentF, dDeltaF, dXminF, dYminF, dLongUnitF, dTimeUnitF, csLongUnitF, csTimeUnitF );

	if( IsError() ) 
	{
		fclose( pfDataF );
		fclose( pfDataA );
		return;
	};

	TmcIdx nNumNode, ii;	// INT64

	nNumNode = ((TmcIdx)nX)*nY;	// INT64
		
	if( ( prAmp = new _real[ nNumNode ] ) == NULL )
	{
		fclose( pfDataF );
		fclose( pfDataA );
		return;
	};
	if( ( prFaz = new _real[ nNumNode ] ) == NULL )
	{
		if( prAmp != NULL )
		{
			delete []prAmp;
			prAmp = NULL;
		};
		fclose( pfDataF );
		fclose( pfDataA );
		return;
	};

	for( i = 0, ii = 0; i < nY; i++ )
	{
		for( j = 0; j < nX; j++ )
		{
			if( fread( (prFaz+ii), sizeof(_real), 1, pfDataF ) != 1 )
			{
				cError.PutErrorMessage("when read Faza data file");
				fclose( pfDataF );
				fclose( pfDataA );
				return;
			};
			if( fread( (prAmp+ii), sizeof(_real), 1, pfDataA ) != 1 )
			{
				cError.PutErrorMessage("when read Amplit. data file");
				fclose( pfDataF );
				fclose( pfDataA );
				return;
			};
			ii++;
		};
		fread( &(ch), 1, 1, pfDataF );
		fread( &(ch), 1, 1, pfDataF );
		fread( &(ch), 1, 1, pfDataA );
		fread( &(ch), 1, 1, pfDataA );
	};

	fclose( pfDataF );
	fclose( pfDataA );
	return;

}
