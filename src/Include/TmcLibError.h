// TmcLibError.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcLibError window

#ifndef CTMCLIBERROR_H__
#define CTMCLIBERROR_H__

#include <cstringt.h>
//#include <windows.h>

#define CTMCRTHERROR_0000 "Error:no"
#define CTMCRTHERROR_0001 "Error:"
#define CTMCRTHERROR_0002 "Error:"
#define CTMCRTHERROR_0003 "Error:"
#define CTMCRTHERROR_0004 "Error:"
#define CTMCRTHERROR_0005 "Error:"
#define CTMCRTHERROR_0006 "Error:"
#define CTMCRTHERROR_0007 "Error:"
#define CTMCRTHERROR_0008 "Error:"
#define CTMCRTHERROR_0009 "Error:"
#define CTMCRTHERROR_0010 "Error:"
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
