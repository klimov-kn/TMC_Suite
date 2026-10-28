// TmcLibError.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcLibError window

#ifndef CTMCLIBERROR_H__
#define CTMCLIBERROR_H__

#include <cstringt.h>

#define CTMCRTHERROR_0000 "Error:no"
#define CTMCRTHERROR_STRLENMAX 512

class CTmcLibError
{
public:
	void PutErrorMessage( char *pszErrorMessage );
	void PutErrorMessage( CString csErrorMessage );
	CString GetErrorMessage( void );
	BOOL IsError( void );
	void Clear( void );
	CTmcLibError& operator=(CTmcLibError& cError1);
	CTmcLibError();

public:
	virtual ~CTmcLibError();

private:
	CString csError;
	BOOL bIsError;
};

void MakeTempFileNameForBzCalcMn(CString & csTplFileName);
void MakeTempFileNameForBzCalcPl(CString & csTplFileName);

#endif
/////////////////////////////////////////////////////////////////////////////
