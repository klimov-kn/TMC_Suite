// FldViewDoc.cpp : implementation of the CFldViewDoc class
//

#include "stdafx.h"
#include "FldView.h"

#include <io.h>
#include <sys/stat.h>
#include <tmcgrviw.h>
#include <TmcLibError.h>
#include "FldViewDoc.h"
#include "FldViewView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFldViewDoc

IMPLEMENT_DYNCREATE(CFldViewDoc, CDocument)

BEGIN_MESSAGE_MAP(CFldViewDoc, CDocument)
	//{{AFX_MSG_MAP(CFldViewDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFldViewDoc construction/destruction

CFldViewDoc::CFldViewDoc()
{
	// TODO: add one-time construction code here
	nX = 0;
	nY = 0;
	nViews = 0;
	nViewsSinchroniz = 0;
	pdSurface = NULL;
	nTCurrent = 0;
	dTCurrent = 0.0;
	dDelta = 0.001;
	dXmin = 0.0;
	dYmin = 0.0;
	nAccuracy = 8;
	pfData = NULL;
	csLongUnit.Format( "m" );
	dLongUnit = 1.;
	csTimeUnit.Format( "s" );
	dTimeUnit = 1.;
    bBusy = FALSE;
	return;
}

CFldViewDoc::~CFldViewDoc()
{
	DeleteData();
	SetFileAttributes( GetPathName(), FILE_ATTRIBUTE_NORMAL );
	return;
}

BOOL CFldViewDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CFldViewDoc serialization

void CFldViewDoc::Serialize(CArchive& ar)
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
// CFldViewDoc diagnostics

#ifdef _DEBUG
void CFldViewDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFldViewDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFldViewDoc commands

void CFldViewDoc::DeleteData()
{
	nX = 0;
	nY = 0;
	if( pdSurface != NULL ) delete []pdSurface;
	pdSurface = NULL;
	cError.Clear();
	nTCurrent = 0;
	dTCurrent = 0.0;
	dDelta = 0.001;
	dXmin = 0.0;
	dYmin = 0.0;
	nAccuracy = 8;
	csLongUnit.Format( "m" );
	dLongUnit = 1.;
	csTimeUnit.Format( "s" );
	dTimeUnit = 1.;
	CloseFileData();

	return;
}

void CFldViewDoc::ReadData()
{
	DeleteData();
	ftLastWriteTime = LastWriteTime( GetPathName() );
	char ch[TMC_GROUT_MAXSTRING_BUF];

	strcpy( ch, GetPathName());
	if( ch[0] == '\0' ) 
	{
		cError.PutErrorMessage("file name is NULL");
		CloseFileData();
		SetSynchronizationFlag();
		return;
	};

	if( (pfData = fopen(GetPathName(), "rb")) == NULL )
	{
		CString csBuf;
		csBuf.Format("can't open data file {%s}", GetPathName());
		cError.PutErrorMessage(csBuf);
		CloseFileData();
		SetSynchronizationFlag();
		return;
	};

	ReadDataParameters( &pfData );
	InitAllocationArray();
	ReadDataArray( &pfData );
	CloseFileData();

//	nViewsSinchroniz = nViews;
//	SetSynchronizationFlag();
	return;
}

void CFldViewDoc::ReadDataParameters(FILE **fp)
{
	if( *fp == NULL ) return;
	if( cError.IsError() ) return;
	char ch[TMC_GROUT_MAXSTRING_BUF], ch1[TMC_GROUT_MAXSTRING_BUF];

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, *fp ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID, strlen(TMC_GROFLD_DOCFILE_ID) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, *fp ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_nT, strlen(TMC_GROFLD_DOCFILE_ID_nT) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_nT), "%d",  &(nTCurrent) );

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, *fp ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_T, strlen(TMC_GROFLD_DOCFILE_ID_T) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_T), "%lg",  &(dTCurrent) );

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, *fp ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_Delta, strlen(TMC_GROFLD_DOCFILE_ID_Delta) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_Delta), "%lg",  &(dDelta) );

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, *fp ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_Xmin, strlen(TMC_GROFLD_DOCFILE_ID_Xmin) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_Xmin), "%lg",  &(dXmin) );

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, *fp ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_Ymin, strlen(TMC_GROFLD_DOCFILE_ID_Ymin) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_Ymin), "%lg",  &(dYmin) );

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, *fp ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_Accuracy, strlen(TMC_GROFLD_DOCFILE_ID_Accuracy) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_Accuracy), "%d",  &(nAccuracy) );

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, *fp ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_LongUnit, strlen(TMC_GROFLD_DOCFILE_ID_LongUnit) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_LongUnit), "%s %lg",  ch1,  &(dLongUnit) );
	csLongUnit.Format( "%s",  ch1);

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, *fp ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_TimeUnit, strlen(TMC_GROFLD_DOCFILE_ID_TimeUnit) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_TimeUnit), "%s %lg",  ch1,  &(dTimeUnit) );
	csTimeUnit.Format( "%s",  ch1);

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, *fp ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_nX, strlen(TMC_GROFLD_DOCFILE_ID_nX) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_nX), "%d",  &(nX) );

	if( fgets( ch, TMC_GROUT_MAXSTRING_BUF, *fp ) == NULL )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	if( strncmp( ch, TMC_GROFLD_DOCFILE_ID_nY, strlen(TMC_GROFLD_DOCFILE_ID_nY) ) != 0 )
	{
		cError.PutErrorMessage("bad data file");
		fclose( *fp );
		*fp = NULL;
		return;
	};
	sscanf( ch + strlen(TMC_GROFLD_DOCFILE_ID_nY), "%d",  &(nY) );

	return;
}

void CFldViewDoc::InitAllocationArray()
{
	if( cError.IsError() ) return;

	int n;

	if( nX < 2 )
	{
		CString csBuf;
		csBuf.Format("bad data file nX = %d < 2", nX);
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( nY < 2 )
	{
		CString csBuf;
		csBuf.Format("bad data file nY = %d < 2", nY);
		cError.PutErrorMessage(csBuf);
		return;
	};

	n = nX*nY;
	if( n < 4 )
	{
		CString csBuf;
		csBuf.Format("bad data file nX*nY = %d < 4", n);
		cError.PutErrorMessage(csBuf);
		return;
	};

	if( pdSurface != NULL ) delete []pdSurface;
	pdSurface = NULL;

	if( ( pdSurface = new double[n] ) == NULL ) 
	{
		CString csBuf;
		csBuf.Format("allocation memory for double array[%d]", n);
		cError.PutErrorMessage(csBuf);
		return;
	};


	return;
}

void CFldViewDoc::ReadDataArray(FILE **fp)
{
	if( *fp == NULL ) return;
	if( cError.IsError() ) return;
	if( pdSurface == NULL ) return;

	double	dBuf;
	float	fBuf;
	int ii, i, j;
	char ch;

	switch( nAccuracy )
	{
		case 4:
			for( i = 0, ii = 0; i < nY; i++ )
			{
				for( j = 0; j < nX; j++ )
				{
					if( fread( &(fBuf), nAccuracy, 1, *fp ) != 1 )
					{
						cError.PutErrorMessage("when read data file");
						fclose( *fp );
						*fp = NULL;
						return;
					};
					pdSurface[ii++] = fBuf;
				};
				fread( &(ch), 1, 1, *fp );
				fread( &(ch), 1, 1, *fp );
			};
			break;
		case 8:
			for( i = 0, ii = 0; i < nY; i++ )
			{
				for( j = 0; j < nX; j++ )
				{
					if( fread( &(dBuf), nAccuracy, 1, *fp ) != 1 )
					{
						cError.PutErrorMessage("when read data file");
						fclose( *fp );
						*fp = NULL;
						return;
					};
					pdSurface[ii++] = dBuf;
				};
				fread( &(ch), 1, 1, *fp );
				fread( &(ch), 1, 1, *fp );
			};
			break;
		default:
			{
				CString csBuf;
				csBuf.Format("abnormal accuracy [%d], but must be 4 or 8", nAccuracy);
				cError.PutErrorMessage(csBuf);
				return;
			};
			break;
	};

	return;
}

int CFldViewDoc::GetnX()
{
	return nX;
}

int CFldViewDoc::GetnY()
{
	return nY;
}

int CFldViewDoc::GetnTCurrent()
{
	return nTCurrent;
}

double CFldViewDoc::GetdTCurrent()
{
	return dTCurrent;
}

double CFldViewDoc::GetdDelta()
{
	return dDelta;
}

double *CFldViewDoc::GetpdSurface()
{
	return pdSurface;
}

void CFldViewDoc::CloseFileData()
{
	if( pfData != NULL ) 
	{
		fclose(pfData);
		pfData = NULL;
	};
}

void CFldViewDoc::SetSynchronizationFlag()
{
//	_chmod( GetPathName(), _S_IREAD );
	nViewsSinchroniz--;
//	CString csBuf;
//	csBuf.Format("nViewsSinchroniz = %d; nViews = %d", nViewsSinchroniz, nViews);
//	PutTrace(csBuf);
	if( nViewsSinchroniz < 0 ) nViewsSinchroniz = 0;
	if( nViewsSinchroniz == 0 )	
	{
		SetFileAttributes( GetPathName(), FILE_ATTRIBUTE_ARCHIVE );
		nViewsSinchroniz = nViews;
	};
	return;
}

BOOL CFldViewDoc::IsFileRead()
{
	if( GetFileAttributes( GetPathName() ) == 0xFFFFFFFF ) FALSE;
	if( GetFileAttributes( GetPathName() ) == FILE_ATTRIBUTE_ARCHIVE )	return TRUE;
	return FALSE;
}

double CFldViewDoc::GetdXmin()
{
	return dXmin;
}

double CFldViewDoc::GetdYmin()
{
	return dYmin;
}

double CFldViewDoc::GetdLongUnit()
{
	return dLongUnit;
}

CString CFldViewDoc::GetcsLongUnit()
{
	return csLongUnit;
}

double CFldViewDoc::GetdTimeUnit()
{
	return dTimeUnit;
}

CString CFldViewDoc::GetcsTimeUnit()
{
	return csTimeUnit;
}


void CFldViewDoc::AddViews__()
{
	nViews++;
//	if( nViews != 1 ) nViewsSinchroniz = nViews-1;
//	else nViewsSinchroniz = nViews;
	nViewsSinchroniz = nViews;
	return;

}

void CFldViewDoc::DelViews__()
{
	nViews--;
	nViewsSinchroniz = nViews;
	return;
}

FILETIME CFldViewDoc::GetLastWriteFile()
{
	return ftLastWriteTime;
}

FILETIME CFldViewDoc::LastWriteTime(char *szFileName)
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

FILETIME CFldViewDoc::LastWriteTime(CString csFileName)
{
	HANDLE fh;
	FILETIME LastWriteTime1;
	LastWriteTime1.dwLowDateTime   = 0;
	LastWriteTime1.dwHighDateTime  = 0;

	fh =  CreateFile(
    csFileName,				// pointer to name of the file 
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

void CFldViewDoc::SetBusyOn()
{
    bBusy = TRUE;
	return;
}

void CFldViewDoc::SetBusyOff()
{
    bBusy = FALSE;
	return;
}

BOOL CFldViewDoc::IsBusy()
{
    return bBusy;
}

double CFldViewDoc::GetdZ(double dX, double dY)
{
	if( cError.IsError() )	return 0.;
	if( pdSurface == NULL )	return 0.;

	return pdSurface[ GetNumberElement( dX, dY ) ];
}

int CFldViewDoc::GetNumberElement(double dX, double dY)
{
	if( cError.IsError() )	return 0;
	if( pdSurface == NULL )	return 0;
	if( dDelta < FLT_MIN )	return 0;

	int n;

	n = ((int)((dX - dXmin)/dDelta)) + nX*((int)((dY - dYmin)/dDelta));
	
	if( n > (nX*nY - 1) ) n =  nX*nY - 1;

	return n;
}
