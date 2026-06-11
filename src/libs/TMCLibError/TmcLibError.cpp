// TmcLibError.cpp : implementation file
//

#include "stdafx.h"
//#include "PlanRT_H.h"
#include <TmcLibError.h>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcLibError

CTmcLibError::CTmcLibError()
{
	bIsError = FALSE;
	csError.Format(CTMCRTHERROR_0000);
	return;
}

CTmcLibError::~CTmcLibError()
{
	return;
}


/////////////////////////////////////////////////////////////////////////////
// CTmcLibError message handlers

void CTmcLibError::Clear( void )
{								  
//		{	CString csBuf1;
//			csBuf1.Format("Trace0");
//			AfxMessageBox(csBuf1);		
//		};
	bIsError = FALSE;
//		{	CString csBuf1;
//			csBuf1.Format("Trace2");
//			AfxMessageBox(csBuf1);		
//		};
	csError.Format(CTMCRTHERROR_0000);
//		{	CString csBuf1;
//			csBuf1.Format("Trace1");
//			AfxMessageBox(csBuf1);		
//		};
	return;
}

BOOL CTmcLibError::IsError( void )
{
	return bIsError;
}

void CTmcLibError::PutErrorMessage( CString csErrorMessage )
{
	bIsError = TRUE;
	csError = csErrorMessage;
	return;
}

void CTmcLibError::PutErrorMessage( char *pszErrorMessage )
{
	bIsError = TRUE;
	csError.Format( "%s", pszErrorMessage );
	return;
}

/*
void CTmcLibError::PutErrorMessage( char *pszErrorMessage, ... )
{
	bIsError = TRUE;

	va_list argptr;
	char szBuf[CTMCRTHERROR_STRLENMAX];
	
	if( (szBuf == NULL)||((&argptr) == NULL) )
	{
		csError.Format( "Memory allocation error for error message" );
		return;
	};
//	vsprintf( char *buffer, const char *format, va_list argptr );
//	csError.Format( "%s", pszErrorMessage );
	vsprintf( szBuf, pszErrorMessage, argptr );
	csError.Format( "%s", szBuf );
	return;
}
*/

CString CTmcLibError::GetErrorMessage( void )
{
	return csError;
}

CTmcLibError& CTmcLibError::operator=( CTmcLibError& cError1 )
{
	bIsError = cError1.IsError();
	csError.Format( "%s", cError1.GetErrorMessage() );
	return (*this);
}

void MakeTempFileNameForBzCalcMn(CString & csTplFileName)
{
	csTplFileName += "_for_Bz_Mn";
	return;
}

void MakeTempFileNameForBzCalcPl(CString & csTplFileName)
{
	csTplFileName += "_for_Bz_Pl";
	return;
}

