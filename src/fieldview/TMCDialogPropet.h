#if !defined(AFX_TMCDIALOGPROPET_H__5C245DA5_B155_11D3_A93D_30894DC10000__INCLUDED_)
#define AFX_TMCDIALOGPROPET_H__5C245DA5_B155_11D3_A93D_30894DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TMCDialogPropet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTMCDialogPropet
#include "TMCDialogEps.h"
#include "TMCDialogField.h"
#include "TMCDialogPlace.h"
#include "TMCDialogTopology.h"

class CTMCDialogPropet : public CPropertySheet
{
	DECLARE_DYNAMIC(CTMCDialogPropet)

// Construction
public:
	CTMCDialogPropet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CTMCDialogPropet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTMCDialogPropet)
	public:
	virtual BOOL OnInitDialog();
	virtual INT_PTR DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// Implementation
public:
	int nPropertyPageInd;
	CImageList m_ImageList1;
	CTMCDialogTopology cTopology;
	CTMCDialogPlace cPlace;
	CTMCDialogField cField;
	CTMCDialogEps cEps;
	virtual ~CTMCDialogPropet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTMCDialogPropet)
	afx_msg void OnClose();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


CTMCDialogPropet * GetPropDialog( void );
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCDIALOGPROPET_H__5C245DA5_B155_11D3_A93D_30894DC10000__INCLUDED_)
