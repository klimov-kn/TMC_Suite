// TmcTopSize.cpp : implementation file
//

#include "stdafx.h"
#include "FldView.h"
#include "TmcTopSize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcTopSize

CTmcTopSize::CTmcTopSize()
{
	dX1 = 0.0;
	dX2 = 0.0;
	dY1 = 0.0;
	dY2 = 0.0;
	dSize = 0.0;
	csSize.Format("");
	return;
}

CTmcTopSize::~CTmcTopSize()
{
	DeleteData();
	return;
}


BEGIN_MESSAGE_MAP(CTmcTopSize, CWnd)
	//{{AFX_MSG_MAP(CTmcTopSize)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTmcTopSize message handlers

void CTmcTopSize::SetdX1(double r)
{
	dX1 = r;
	return;
}

void CTmcTopSize::SetdX2(double r)
{
	dX2 = r;
	return;
}

void CTmcTopSize::SetdY1(double r)
{
	dY1 = r;
	return;
}

void CTmcTopSize::SetdY2(double r)
{
	dY2 = r;
	return;
}


void CTmcTopSize::SetdSize(double r)
{
	dSize = r;
	return;
}

void CTmcTopSize::SetcsSize( CString &csr )
{
	csSize = csr;
	return;
}

double CTmcTopSize::GetdX1()
{
	return dX1;
}

double CTmcTopSize::GetdX2()
{
	return dX2;
}

double CTmcTopSize::GetdY1()
{
	return dY1;
}

double CTmcTopSize::GetdY2()
{
	return dY2;
}

double CTmcTopSize::GetdSize()
{
	return dSize;
}

CString CTmcTopSize::GetcsSize()
{
	return csSize;
}


void CTmcTopSize::DeleteData()
{
	dX1 = 0.0;
	dX2 = 0.0;
	dY1 = 0.0;
	dY2 = 0.0;
	dSize = 0.0;
	csSize.Format("");
	return;
}


void CTmcTopSize::TranslateX0Y0(double dX0, double dY0)
{
	dX1 += dX0;
	dX2 += dX0;
	dY1 += dY0;
	dY2 += dY0;
	return;
}


CTmcTopSize& CTmcTopSize::operator=(CTmcTopSize& cTopSize)
{
	DeleteData();
	dX1 = cTopSize.GetdX1();
	dX2 = cTopSize.GetdX2();
	dY1 = cTopSize.GetdY1();
	dY2 = cTopSize.GetdY2();
	dSize = cTopSize.GetdSize();
	csSize = cTopSize.GetcsSize();
	return (*this);
}
