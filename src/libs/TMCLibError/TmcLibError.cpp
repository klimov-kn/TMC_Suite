// TmcLibError.cpp : implementation file
//

/// @file TmcLibError.cpp
/// @brief Реализация объекта ошибки `CTmcLibError` и двух функций имён временных файлов.
///
/// Единственный исходный файл библиотеки TMCLibError. Описания методов — в tmcliberror.h.
/// @see https://www.tamic.ru/docs/code/lib-tmcliberror.html
#include "STDAFX.H"
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

void CTmcLibError::PutErrorMessage( const char *pszErrorMessage )
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



