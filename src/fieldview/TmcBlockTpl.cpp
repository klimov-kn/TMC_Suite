//   TmcBlockTpl . cpp 
//

#include "stdafx.h"
#include <tmcgrviw.h>
#include "FldView.h"
#include "TmcBlockTpl.h"



CTmcBlockTpl::CTmcBlockTpl()
{
	nTCurrent = 0;
	dTCurrent = 0;
	dDelta = 1;
	dXmin = 0;
	dYmin = 0;
	nX = 0;
	nY = 0;
	pdSurface = NULL;
	return;
}

CTmcBlockTpl::~CTmcBlockTpl()
{
	DeleteData();
	return;
}


CTmcLibError & CTmcBlockTpl::GetError()
{
	return cError;
}

void CTmcBlockTpl::SetTplFileName(CString &csTplFileName1)
{
	csTplFileName = csTplFileName1;
	return;
}

CString CTmcBlockTpl::GetTplFileName()
{
	return csTplFileName;
}

void CTmcBlockTpl::Read(CString csTplFileName1, CTmcLibError &cError1)
{
	if( cError1.IsError() ) return;
	DeleteData();
	SetTplFileName(csTplFileName1);
	ReadData();
	cError1 = cError;
	return;
}

void CTmcBlockTpl::DeleteData()
{
	cError.Clear();
	csTplFileName.Format("");
	nTCurrent = 0;
	dTCurrent = 0;
	dDelta = 1;
	dXmin = 0;
	dYmin = 0;
	nX = 0;
	nY = 0;
	if( pdSurface != NULL ) delete []pdSurface;
	pdSurface = NULL;

	CTmcRTH_BlockList::DeleteData();
	return;
}

void CTmcBlockTpl::ReadData()
{
	if( cError.IsError() ) return;
	if( strlen(csTplFileName) == 0 )
	{
		cError.PutErrorMessage("Topology file name (*.tt) is NULL");
		return;
	};

	FILE *fp;
	char szBuf[TMC_GROUT_MAXSTRING_BUF];
	
	fp = NULL;
	if( (fp = fopen( csTplFileName, "rb" ) ) == NULL )
	{
		CString csBuf1;
		csBuf1.Format("Can't open topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		return;
	}

	if( fgets( szBuf, TMC_GROUT_MAXSTRING_BUF, fp) == NULL )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	}
	if( strncmp( szBuf, TMC_RTH_FILETOP_ID, strlen(TMC_RTH_FILETOP_ID) ) != 0 )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	if( fgets( szBuf, TMC_GROUT_MAXSTRING_BUF, fp) == NULL )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	}
	if( strncmp( szBuf, TMC_RTH_BLOCKLISTSAVE_BEGIN, strlen(TMC_RTH_BLOCKLISTSAVE_BEGIN) ) != 0 )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	Load( &fp, cError );
	if( (fp == NULL)||(cError.IsError()) )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};

	if( fgets( szBuf, TMC_GROUT_MAXSTRING_BUF, fp ) == NULL )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	if( strncmp( szBuf, TMC_GROFLD_DOCFILE_ID_nT, strlen(TMC_GROFLD_DOCFILE_ID_nT) ) != 0 )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	sscanf( szBuf + strlen(TMC_GROFLD_DOCFILE_ID_nT), "%d",  &(nTCurrent) );

	if( fgets( szBuf, TMC_GROUT_MAXSTRING_BUF, fp ) == NULL )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	if( strncmp( szBuf, TMC_GROFLD_DOCFILE_ID_T, strlen(TMC_GROFLD_DOCFILE_ID_T) ) != 0 )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	sscanf( szBuf + strlen(TMC_GROFLD_DOCFILE_ID_T), "%lg",  &(dTCurrent) );

	if( fgets( szBuf, TMC_GROUT_MAXSTRING_BUF, fp ) == NULL )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	if( strncmp( szBuf, TMC_GROFLD_DOCFILE_ID_Delta, strlen(TMC_GROFLD_DOCFILE_ID_Delta) ) != 0 )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	sscanf( szBuf + strlen(TMC_GROFLD_DOCFILE_ID_Delta), "%lg",  &(dDelta) );

	if( fgets( szBuf, TMC_GROUT_MAXSTRING_BUF, fp ) == NULL )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	if( strncmp( szBuf, TMC_GROFLD_DOCFILE_ID_Xmin, strlen(TMC_GROFLD_DOCFILE_ID_Xmin) ) != 0 )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	sscanf( szBuf + strlen(TMC_GROFLD_DOCFILE_ID_Xmin), "%lg",  &(dXmin) );

	if( fgets( szBuf, TMC_GROUT_MAXSTRING_BUF, fp ) == NULL )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	if( strncmp( szBuf, TMC_GROFLD_DOCFILE_ID_Ymin, strlen(TMC_GROFLD_DOCFILE_ID_Ymin) ) != 0 )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	sscanf( szBuf + strlen(TMC_GROFLD_DOCFILE_ID_Ymin), "%lg",  &(dYmin) );

	if( fgets( szBuf, TMC_GROUT_MAXSTRING_BUF, fp ) == NULL )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	if( strncmp( szBuf, TMC_GROFLD_DOCFILE_ID_nX, strlen(TMC_GROFLD_DOCFILE_ID_nX) ) != 0 )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	sscanf( szBuf + strlen(TMC_GROFLD_DOCFILE_ID_nX), "%d",  &(nX) );

	if( fgets( szBuf, TMC_GROUT_MAXSTRING_BUF, fp ) == NULL )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	if( strncmp( szBuf, TMC_GROFLD_DOCFILE_ID_nY, strlen(TMC_GROFLD_DOCFILE_ID_nY) ) != 0 )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		if( fp != NULL ) fclose( fp );
		fp = NULL;
		return;
	};
	sscanf( szBuf + strlen(TMC_GROFLD_DOCFILE_ID_nY), "%d",  &(nY) );

	InitAllocationArray();

	{
		if( cError.IsError() ) return;
		if( pdSurface == NULL ) return;
	
		double	dBuf;
		int ii, i, j;
		char ch;
	
		for( i = 0, ii = 0; i < nY; i++ )
		{
			for( j = 0; j < nX; j++ )
			{
				if( fread( &(dBuf), sizeof(double), 1, fp ) != 1 )
				{
					CString csBuf1;
					csBuf1.Format("Bad topology file %s", csTplFileName );
					cError.PutErrorMessage( csBuf1 );
					if( fp != NULL ) fclose( fp );
					fp = NULL;
					return;
				};
				pdSurface[ii++] = dBuf;
			};
			fread( &(ch), 1, 1, fp );
			fread( &(ch), 1, 1, fp );
		};
	};

	if( fp != NULL ) fclose( fp );
	fp = NULL;
	return;
}

void CTmcBlockTpl::InitAllocationArray()
{
	if( cError.IsError() ) return;

	int n;

	if( nX < 2 )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s - allocation memory error nX or nY < 2, or nX*nY < 4", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		return;
	};

	if( nY < 2 )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s - allocation memory error nX or nY < 2, or nX*nY < 4", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		return;
	};

	n = nX*nY;
	if( n < 4 )
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s - allocation memory error nX or nY < 2, or nX*nY < 4", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		return;
	};

	if( pdSurface != NULL ) delete []pdSurface;
	pdSurface = NULL;

	if( ( pdSurface = new double[n] ) == NULL ) 
	{
		CString csBuf1;
		csBuf1.Format("Bad topology file %s - allocation memory error nX or nY < 2, or nX*nY < 4", csTplFileName );
		cError.PutErrorMessage( csBuf1 );
		return;
	};


	return;
}

double * CTmcBlockTpl::GetpdSurface()
{
	return pdSurface;
}

int CTmcBlockTpl::GetnX()
{
	return nX;
}
