// FieldIntegrated.cpp: implementation of the CFieldIntegrated class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <tmcgrviw.h>
#include <FieldIntegrated.h>
//#include "TmcRTHRectNode.h"
//#include "Pl_iofor.h"


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
	int i, nNumNode;
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

		nNumNode = nX*nY;
		
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

double CFieldIntegrated::GetFreq()
{
	return dFreq;
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

void CFieldIntegrated::InitFile(CString csFileName)
{
//	{ 
//		CString csBuf1;
//		csBuf1.Format("InitFile = {%s}", csFileName );
//		AfxMessageBox(csBuf1);
//	};
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
		if( i > 2000  ) return;
		Sleep( 10 );
	};
	
	fprintf( fp, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, 0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, 0.0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, sizeof(_real),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, cParam.csLongUnit, cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, cParam.csTimeUnit, cParam.rTimeUnit, 13, 10,
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

	IntegrateInMemory( prUNode1, (_real)(cos(dTCurrent*2*3.141592653589*dFreq)), (_real)(sin(dTCurrent*2*3.141592653589*dFreq))  );

	return;

}

void CFieldIntegrated::Integrate(CString csFileName, _real *prUNode1, _real r1)
{
	if( IsError() ) return;
	if( !bIsFieldDistribution ) return;
	if( bIsIntegretedInMemory ) return;

	int i, j, ii;
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
		if( i > 2000  ) return;
		Sleep( 10 );
	};
	
	fprintf( fp, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, 0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, 0.0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, sizeof(_real),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, cParam.csLongUnit, cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, cParam.csTimeUnit, cParam.rTimeUnit, 13, 10,
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
//	{ 
//		CString csBuf1;
//		csBuf1.Format("Integrate  write; r = %lg; ", r );
//		AfxMessageBox(csBuf1);
//	};
			fsetpos( fp, &fpos );
			fflush( fp );
			fwrite( &(r), sizeof(_real), 1, fp );
			fflush( fp );
			ii += 2;
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
		if( i > 2000  ) return;
		Sleep( 10 );
	};
	for( i = 0; ( fpCos = fopen( csFileCos , "r+b") ) == NULL; i++ ) 
	{
		if( i > 2000  ) return;
		Sleep( 10 );
	};
	for( i = 0; ( fpFaz = fopen( csFileFaza , "w+b") ) == NULL; i++ ) 
	{
		if( i > 2000  ) return;
		Sleep( 10 );
	};
	for( i = 0; ( fpAmp = fopen( csFileAmplitude , "w+b") ) == NULL; i++ ) 
	{
		if( i > 2000  ) return;
		Sleep( 10 );
	};

	fprintf( fpSin, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, 0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, 0.0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, sizeof(_real),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, cParam.csLongUnit, cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, cParam.csTimeUnit, cParam.rTimeUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nX, nX, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nY, nY);
	fprintf( fpCos, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, 0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, 0.0, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, sizeof(_real),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, cParam.csLongUnit, cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, cParam.csTimeUnit, cParam.rTimeUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nX, nX, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nY, nY);
	fprintf( fpFaz, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, nTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, dTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, sizeof(_real),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, cParam.csLongUnit, cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, cParam.csTimeUnit, cParam.rTimeUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nX, nX, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nY, nY);
	fprintf( fpAmp, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, nTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, dTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, sizeof(_real),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, cParam.csLongUnit, cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, cParam.csTimeUnit, cParam.rTimeUnit, 13, 10,
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

	int i, nNumNode;

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

	nNumNode = nX*nY;
		
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

	int i, j, ii, jj;

	for( i = 0, ii = 0, jj = 0; i < nY; i++)
	{
		for( j = 0; j < nX; j++ )
		{
			prCos[jj] = (_real)(prCos[jj] + prUNode1[ii]*rCosCurrent*dT);
			prSin[jj] = (_real)(prSin[jj] + prUNode1[ii]*rSinCurrent*dT);
			jj++;
			ii += 2;
		};
	};
	
	return;
}

void CFieldIntegrated::MakeFileAmplitudeAndFazaFromMemory()
{
	if( IsError() ) return;
	if( !bIsFieldDistribution ) return;
	if( !bIsIntegretedInMemory ) return;
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

	int i, j, jj;
	FILE *fpFaz, *fpAmp;
	_real rSin, rCos, rAmp, rFaz;

	for( i = 0; ( fpFaz = fopen( csFileFaza , "w+b") ) == NULL; i++ ) 
	{
		if( i > 2000  ) return;
		Sleep( 10 );
	};
	for( i = 0; ( fpAmp = fopen( csFileAmplitude , "w+b") ) == NULL; i++ ) 
	{
		if( i > 2000  ) return;
		Sleep( 10 );
	};

	fprintf( fpFaz, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, nTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, dTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, sizeof(_real),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, cParam.csLongUnit, cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, cParam.csTimeUnit, cParam.rTimeUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nX, nX, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nY, nY);
	fprintf( fpAmp, "%s%c%c%s%d%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%lg%c%c%s%d%c%c%s%s %lg%c%c%s%s %lg%c%c%s%d%c%c%s%d", TMC_GROFLD_DOCFILE_ID, 13, 10,
						TMC_GROFLD_DOCFILE_ID_nT, nTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_T, dTCurrent, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Delta, dDelta, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Xmin, dXmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Ymin, dYmin, 13, 10,
						TMC_GROFLD_DOCFILE_ID_Accuracy, sizeof(_real),  13, 10,
						TMC_GROFLD_DOCFILE_ID_LongUnit, cParam.csLongUnit, cParam.rLongUnit, 13, 10,
						TMC_GROFLD_DOCFILE_ID_TimeUnit, cParam.csTimeUnit, cParam.rTimeUnit, 13, 10,
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

	ReadParamFromFile();
	SetDirectionalPatternParamDefault();
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
		||(dTimeUnitA != dTimeUnitA) )
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

void CFieldIntegrated::MakeDirectionalPatternFile()
{
	if( IsError() ) return;
	if( !IsDirectionalPatternDataCorrect() ) return;
	MakeDirectionalPatternFileZagolovok();
	ReadAmplitudaAndFazaEzMInit();
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

	int i, j, nX1, nY1;
	_real rBufA, rBufF;

	aEz = 0.0;
	fEz = 0.0;

	nX1 = (int)((x+0.5*dDelta-dXmin)/dDelta);
	nY1 = (int)((y+0.5*dDelta-dYmin)/dDelta);

	if( (prAmp != NULL)&&(prFaz != NULL) )
	{
		i = nY1*nX+nX1;
		aEz = prAmp[ i ];
		fEz = prFaz[ i ];
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
	fEz = rBufF;

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

//	dA_111 = sqrt(2.)*3.141592653589*dDelta*dFreq/299792458;
//	dW0 = 1./(sqrt((8.854187817e-12)/(2.*(12.566370614e-7))))/(sin(acos( (2+fabs(0.)/2.)*cos(dA_111) - 1. - 0./2. ))/sin(dA_111));
//	dW0 = 1./(sqrt((8.854187817e-12)/((12.566370614e-7))));
//	koef = sqrt(1./(12.))*(dDelta)*sqrt(dW0);
	dDkL = (dDelta*dFreq/299792458);
//	koef = sqrt(134.304966913/(dFreq/299792458))*dDkL/sqrt(dW0);
//	koef = sqrt(dDelta*dW0/3.*0.999996/0.9361811)*dDkL/sqrt(dW0)/sqrt(dFreq/299792458);
	koef = sqrt(dDelta/3.*0.999996/0.9361811)*dDkL/sqrt(dFreq/299792458);

	nEmitterInFile++;
	csBuf.Format("1.");
	if( fprintf( fp, "%d   X = %lg mm; Y = %lg mm; Amplitude = %lg; Faza = %lg degree; FiMin = %lg degree; FiMax = %lg degree; %s\n", nEmitterInFile, x*1000., y*1000., aEz*koef, fEz/3.141592653589*180., dFimin, dFimax, csBuf ) < 0 ) 
	{
		cError.PutErrorMessage("Error when add emitter to output data file for directional pattern");
		fclose(fp);
		return;
	};

	nEmitterInFile++;
	csBuf.Format("cos(f)");
	if( fprintf( fp, "%d   X = %lg mm; Y = %lg mm; Amplitude = %lg; Faza = %lg degree; FiMin = %lg degree; FiMax = %lg degree; %s\n", nEmitterInFile, x*1000., y*1000., aHy*koef, fHy/3.141592653589*180., dFimin, dFimax, csBuf ) < 0 ) 
	{
		cError.PutErrorMessage("Error when add emitter to output data file for directional pattern");
		fclose(fp);
		return;
	};

	nEmitterInFile++;
	csBuf.Format("sin(f)");
	if( fprintf( fp, "%d   X = %lg mm; Y = %lg mm; Amplitude = %lg; Faza = %lg degree; FiMin = %lg degree; FiMax = %lg degree; %s\n", nEmitterInFile, x*1000., y*1000., aHx*koef, fHx/3.141592653589*180., dFimin, dFimax, csBuf ) < 0 ) 
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

	int nNumNode, ii;

	nNumNode = nX*nY;
		
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
