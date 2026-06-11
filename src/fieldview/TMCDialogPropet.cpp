// TMCDialogPropet.cpp : implementation file
//

#include "stdafx.h"
#include "FldView.h"
#include "FldViewDoc.h"
#include "FldViewView.h"
#include "TMCDialogPropet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTMCDialogPropet

IMPLEMENT_DYNAMIC(CTMCDialogPropet, CPropertySheet)

CTMCDialogPropet::CTMCDialogPropet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CTMCDialogPropet::CTMCDialogPropet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	AddPage( &cPlace );
	AddPage( &cTopology );
	AddPage( &cEps );
	AddPage( &cField );
	nPropertyPageInd = 0;
}

CTMCDialogPropet::~CTMCDialogPropet()
{
}


BEGIN_MESSAGE_MAP(CTMCDialogPropet, CPropertySheet)
	//{{AFX_MSG_MAP(CTMCDialogPropet)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTMCDialogPropet message handlers

BOOL CTMCDialogPropet::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	return bResult;
}

INT_PTR CTMCDialogPropet::DoModal()
{
	// TODO: Add your specialized code here and/or call the base class
	SetActivePage( nPropertyPageInd );
	return CPropertySheet::DoModal();
}

void CTMCDialogPropet::DoDataExchange(CDataExchange* pDX) 
{
	// TODO: Add your specialized code here and/or call the base class
//	nPropertyPageInd = GetActiveIndex();
	CPropertySheet::DoDataExchange(pDX);
}

void CTMCDialogPropet::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
//	nPropertyPageInd = GetActiveIndex();
	CPropertySheet::OnClose();
}

void CTMCDialogPropet::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	nPropertyPageInd = GetActiveIndex();
	// Do not call CPropertySheet::OnPaint() for painting messages
}
