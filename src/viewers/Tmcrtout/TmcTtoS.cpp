// TmcTtoS.cpp : implementation file
//

#include "stdafx.h"
#include "TMCGROUT.h"
#include <tmcgrviw.h>
#include <TmcTtoS.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcTtoS

CTmcTtoS::CTmcTtoS()
{
	csSFileName.Format("");
	csTFileName.Format("");
	csError.Format("No error");
	dTmin = 0.0;
	dTmax = 0.0;
	dFreq = 0.0;
	bIsError = FALSE;
	return;
}

CTmcTtoS::~CTmcTtoS()
{
	DeleteData();
	return;
}



/////////////////////////////////////////////////////////////////////////////
// CTmcTtoS message handlers

void CTmcTtoS::DeleteData()
{
	
	csSFileName.Format("");
	csTFileName.Format("");
	csError.Format("No error");
	dTmin = 0.0;
	dTmax = 0.0;
	dFreq = 0.0;
	bIsError = FALSE;
	cSMatrix.DeleteData();

	return;
}

void CTmcTtoS::TtoS(CString csTfn, CString csSfn, double dTmin1, double dTmax1, double dFreq1)
{
	DeleteData();
	csSFileName.Format("%s", csSfn);
	csTFileName.Format("%s", csTfn);
	dTmin = dTmin1;
	dTmax = dTmax1;
	dFreq = dFreq1;
	TtoS();
	return;
}

void CTmcTtoS::TtoS()
{
	if( IsError() ) return;

	MakeSmatrixFromFileT();
	SaveSmatrixToFileS();

	return;
}

BOOL CTmcTtoS::IsError()
{
	return bIsError;
}


void CTmcTtoS::MakeSmatrixFromFileT()
{
	if( IsError() ) return;
	cSMatrix.ReadTFile( csTFileName, dTmin, dTmax );
	if( cSMatrix.IsError() )
	{
		bIsError = TRUE;
		csError.Format( "%s", cSMatrix.GetErrorMessage() );
	};
	cSMatrix.MakeSmatrix();
	if( cSMatrix.IsError() )
	{
		bIsError = TRUE;
		csError.Format( "%s", cSMatrix.GetErrorMessage() );
	};
	return;
}

CString & CTmcTtoS::GetErrorMessage()
{
	return csError;
}

void CTmcTtoS::SaveSmatrixToFileS()
{
	if( IsError() ) return;
	cSMatrix.Save( csSFileName, dFreq );
	if( cSMatrix.IsError() )
	{
		bIsError = TRUE;
		csError.Format( "%s", cSMatrix.GetErrorMessage() );
	};
	return;
}
