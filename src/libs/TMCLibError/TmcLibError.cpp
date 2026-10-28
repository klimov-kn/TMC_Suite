// TmcLibError.cpp : implementation file
//

#include "stdafx.h"
#include <TmcLibError.h>


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
	bIsError = FALSE;
//		{	CString csBuf1;
	csError.Format(CTMCRTHERROR_0000);
//		{	CString csBuf1;
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

