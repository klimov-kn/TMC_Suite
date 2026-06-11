// TmcGroutColorGraphDialog.cpp : implementation file
//

#include "stdafx.h"
#include "TMCGROUT.h"
#include "TmcGroutColorGraphDialog.h"
#include "TmcGrParColTypWid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcGroutColorGraphDialog dialog


CTmcGroutColorGraphDialog::CTmcGroutColorGraphDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CTmcGroutColorGraphDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTmcGroutColorGraphDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	SetDefault();
	return;
}


void CTmcGroutColorGraphDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTmcGroutColorGraphDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTmcGroutColorGraphDialog, CDialog)
	//{{AFX_MSG_MAP(CTmcGroutColorGraphDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(ID_ColorLevelAutoSet2, OnColorLevelAutoSet2)
	ON_BN_CLICKED(ID_ColorLevelAutoSet3, OnColorLevelAutoSet3)
	ON_BN_CLICKED(ID_ColorLevelAutoSet4, OnColorLevelAutoSet4)
	ON_BN_CLICKED(ID_ColorLevelAutoSet5, OnColorLevelAutoSet5)
	ON_BN_CLICKED(ID_ColorLevelAutoSet6, OnColorLevelAutoSet6)
	ON_BN_CLICKED(ID_ColorLevelAutoSet7, OnColorLevelAutoSet7)
	ON_BN_CLICKED(ID_ColorLevelAutoSet8, OnColorLevelAutoSet8)
	ON_BN_CLICKED(ID_ColorLevelAutoSet9, OnColorLevelAutoSet9)
	ON_BN_CLICKED(ID_ColorLevelAutoSet10, OnColorLevelAutoSet10)
	ON_BN_CLICKED(ID_ColorLevelAutoSet11, OnColorLevelAutoSet11)
	ON_BN_CLICKED(ID_ColorLevelAutoSet12, OnColorLevelAutoSet12)
	ON_BN_CLICKED(ID_ColorLevelAutoSet13, OnColorLevelAutoSet13)
	ON_BN_CLICKED(ID_ColorLevelAutoSet14, OnColorLevelAutoSet14)
	ON_BN_CLICKED(ID_ColorLevelAutoSet15, OnColorLevelAutoSet15)
	ON_BN_CLICKED(ID_ColorLevelAutoSet16, OnColorLevelAutoSet16)
	ON_BN_CLICKED(ID_GRAPPARCORDEFAULT, OnGrapparcordefault)
	ON_BN_CLICKED(ID_ColorLevelAutoSet, OnColorLevelAutoSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTmcGroutColorGraphDialog message handlers

void CTmcGroutColorGraphDialog::SetGraphParam(TMC_GR_DOC1 *sGrDoc1)
{
	if( sGrDoc1 == NULL ) return;

	int i;

	for( i = 0; i < 16; i++ )
	{
		sGrDoc[i].nPoint = sGrDoc1[i].nPoint;
		sGrDoc[i].pFreq  = sGrDoc1[i].pFreq;
		sGrDoc[i].pSmatr = sGrDoc1[i].pSmatr;
		sGrDoc[i].pcExpr = sGrDoc1[i].pcExpr;
		sGrDoc[i].nInp1  = sGrDoc1[i].nInp1;
		sGrDoc[i].nMod1  = sGrDoc1[i].nMod1;
		sGrDoc[i].nInp2  = sGrDoc1[i].nInp2;
		sGrDoc[i].nMod2  = sGrDoc1[i].nMod2;
		sGrDoc[i].LastWriteTime.dwLowDateTime   = sGrDoc1[i].LastWriteTime.dwLowDateTime;
		sGrDoc[i].LastWriteTime.dwHighDateTime  = sGrDoc1[i].LastWriteTime.dwHighDateTime;
		sGrDoc[i].OutFlag = sGrDoc1[i].OutFlag;
		
		sGrDoc[i].LineColor = sGrDoc1[i].LineColor;
		sGrDoc[i].piPoint[0].nX = sGrDoc1[i].piPoint[0].nX;
		sGrDoc[i].piPoint[0].nY = sGrDoc1[i].piPoint[0].nY;
		sGrDoc[i].piPoint[1].nX = sGrDoc1[i].piPoint[1].nX;
		sGrDoc[i].piPoint[1].nY = sGrDoc1[i].piPoint[1].nY;
		sGrDoc[i].piPoint[2].nX = sGrDoc1[i].piPoint[2].nX;
		sGrDoc[i].piPoint[2].nY = sGrDoc1[i].piPoint[2].nY;
		sGrDoc[i].piPoint[3].nX = sGrDoc1[i].piPoint[3].nX;
		sGrDoc[i].piPoint[3].nY = sGrDoc1[i].piPoint[3].nY;
		sGrDoc[i].piPoint[4].nX = sGrDoc1[i].piPoint[4].nX;
		sGrDoc[i].piPoint[4].nY = sGrDoc1[i].piPoint[4].nY;
		sGrDoc[i].piPoint[5].nX = sGrDoc1[i].piPoint[5].nX;
		sGrDoc[i].piPoint[5].nY = sGrDoc1[i].piPoint[5].nY;
		sGrDoc[i].piPoint[6].nX = sGrDoc1[i].piPoint[6].nX;
		sGrDoc[i].piPoint[6].nY = sGrDoc1[i].piPoint[6].nY;
		sGrDoc[i].piPoint[7].nX = sGrDoc1[i].piPoint[7].nX;
		sGrDoc[i].piPoint[7].nY = sGrDoc1[i].piPoint[7].nY;
		sGrDoc[i].piPoint[8].nX = sGrDoc1[i].piPoint[8].nX;
		sGrDoc[i].piPoint[8].nY = sGrDoc1[i].piPoint[8].nY;
		sGrDoc[i].piPoint[9].nX = sGrDoc1[i].piPoint[9].nX;
		sGrDoc[i].piPoint[9].nY = sGrDoc1[i].piPoint[9].nY;

		sGrDoc[i].LineType  = sGrDoc1[i].LineType;
		sGrDoc[i].LineWidth = sGrDoc1[i].LineWidth/15;
		sGrDoc[i].PointColor = sGrDoc1[i].PointColor;
		sGrDoc[i].PointType  = sGrDoc1[i].PointType;
		sGrDoc[i].PointWidth = sGrDoc1[i].PointWidth;
	};

	return;
}

void CTmcGroutColorGraphDialog::GetGraphParam(TMC_GR_DOC1 *sGrDoc1)
{
	if( sGrDoc1 == NULL ) return;

	int i;

	for( i = 0; i < 16; i++)
	{
		sGrDoc1[i].nPoint = sGrDoc[i].nPoint;
		sGrDoc1[i].pFreq  = sGrDoc[i].pFreq;
		sGrDoc1[i].pSmatr = sGrDoc[i].pSmatr;
		sGrDoc1[i].pcExpr = sGrDoc[i].pcExpr;
		sGrDoc1[i].nInp1  = sGrDoc[i].nInp1;
		sGrDoc1[i].nMod1  = sGrDoc[i].nMod1;
		sGrDoc1[i].nInp2  = sGrDoc[i].nInp2;
		sGrDoc1[i].nMod2  = sGrDoc[i].nMod2;
		sGrDoc1[i].LastWriteTime.dwLowDateTime   = sGrDoc[i].LastWriteTime.dwLowDateTime;
		sGrDoc1[i].LastWriteTime.dwHighDateTime  = sGrDoc[i].LastWriteTime.dwHighDateTime;
		sGrDoc1[i].OutFlag = sGrDoc[i].OutFlag;
		
		sGrDoc1[i].LineColor = sGrDoc[i].LineColor;
		sGrDoc1[i].piPoint[0].nX = sGrDoc[i].piPoint[0].nX;
		sGrDoc1[i].piPoint[0].nY = sGrDoc[i].piPoint[0].nY;
		sGrDoc1[i].piPoint[1].nX = sGrDoc[i].piPoint[1].nX;
		sGrDoc1[i].piPoint[1].nY = sGrDoc[i].piPoint[1].nY;
		sGrDoc1[i].piPoint[2].nX = sGrDoc[i].piPoint[2].nX;
		sGrDoc1[i].piPoint[2].nY = sGrDoc[i].piPoint[2].nY;
		sGrDoc1[i].piPoint[3].nX = sGrDoc[i].piPoint[3].nX;
		sGrDoc1[i].piPoint[3].nY = sGrDoc[i].piPoint[3].nY;
		sGrDoc1[i].piPoint[4].nX = sGrDoc[i].piPoint[4].nX;
		sGrDoc1[i].piPoint[4].nY = sGrDoc[i].piPoint[4].nY;
		sGrDoc1[i].piPoint[5].nX = sGrDoc[i].piPoint[5].nX;
		sGrDoc1[i].piPoint[5].nY = sGrDoc[i].piPoint[5].nY;
		sGrDoc1[i].piPoint[6].nX = sGrDoc[i].piPoint[6].nX;
		sGrDoc1[i].piPoint[6].nY = sGrDoc[i].piPoint[6].nY;
		sGrDoc1[i].piPoint[7].nX = sGrDoc[i].piPoint[7].nX;
		sGrDoc1[i].piPoint[7].nY = sGrDoc[i].piPoint[7].nY;
		sGrDoc1[i].piPoint[8].nX = sGrDoc[i].piPoint[8].nX;
		sGrDoc1[i].piPoint[8].nY = sGrDoc[i].piPoint[8].nY;
		sGrDoc1[i].piPoint[9].nX = sGrDoc[i].piPoint[9].nX;
		sGrDoc1[i].piPoint[9].nY = sGrDoc[i].piPoint[9].nY;

		sGrDoc1[i].LineType  = sGrDoc[i].LineType;
		sGrDoc1[i].LineWidth = 15*sGrDoc[i].LineWidth;
		sGrDoc1[i].PointColor = sGrDoc[i].PointColor;
		sGrDoc1[i].PointType  = sGrDoc[i].PointType;
		sGrDoc1[i].PointWidth = sGrDoc[i].PointWidth;
	};

	return;
}

void CTmcGroutColorGraphDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	int nPixel = 30;
	int i, ii;

	i = 0;
	CWnd *pWnd = GetDlgItem(IDC_STATICLEVEL1);
	CDC *pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};

	i++;
	pWnd = GetDlgItem(IDC_STATICLEVEL2);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};
	
	i++;
	pWnd = GetDlgItem(IDC_STATICLEVEL3);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};

	i++;
	pWnd = GetDlgItem(IDC_STATICLEVEL4);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};
	
	i++;
	pWnd = GetDlgItem(IDC_STATICLEVEL5);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};
	
	i++;
	pWnd = GetDlgItem(IDC_STATICLEVEL6);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};
	
	i++;
	pWnd = GetDlgItem(IDC_STATICLEVEL7);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};
	
	i++;
	pWnd = GetDlgItem(IDC_STATICLEVEL8);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};
	
	i++;
	pWnd = GetDlgItem(IDC_STATICLEVEL9);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};
	
	i++;
	pWnd = GetDlgItem(IDC_STATICLEVEL10);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};

	i++;
	pWnd = GetDlgItem(IDC_STATICLEVEL11);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};

	i++;
	pWnd = GetDlgItem(IDC_STATICLEVEL12);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};
	
	i++;
	pWnd = GetDlgItem(IDC_STATICLEVEL13);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};

	i++;
	pWnd = GetDlgItem(IDC_STATICLEVEL14);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};
	
	i++;
	pWnd = GetDlgItem(IDC_STATICLEVEL15);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};
	
	i++;
	pWnd = GetDlgItem(IDC_STATICLEVEL16);
	pControlDC = pWnd->GetDC();
	pWnd->Invalidate();
	pWnd->UpdateWindow();
	{
	if( sGrDoc[i].LineType == PS_SOLID )
		{ 
			ii = sGrDoc[i].LineWidth;
		}
		else
		{
			ii = 1;
		};
	CPen newPen( sGrDoc[i].LineType, ii, sGrDoc[i].LineColor);
	CPen *pOldPen = pControlDC->SelectObject(&newPen);	
	pControlDC->MoveTo( 0, 0);	
	pControlDC->LineTo( nPixel, 0);	
	pControlDC->SelectObject(pOldPen);
	pWnd->ReleaseDC( pControlDC );
	};
	
	// Do not call CDialog::OnPaint() for painting messages
	return;
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet2() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 1 );
	return;	
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet3() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 2 );
	return;	
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet4() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 3 );
	return;	
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet5() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 4 );
	return;	
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet6() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 5 );
	return;	
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet7() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 6 );
	return;	
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet8() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 7 );
	return;	
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet9() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 8 );
	return;	
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet10() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 9 );
	return;	
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet11() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 10 );
	return;	
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet12() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 11 );
	return;	
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet13() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 12 );
	return;	
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet14() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 13 );
	return;	
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet15() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 14 );
	return;	
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet16() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 15 );
	return;	
}

void CTmcGroutColorGraphDialog::ChangeGraphicsPar(int n)
{
	if( (n < 0)||(n > 15) ) return;
	
	CTmcGrParColTypWid dlg;

	dlg.LineColor = sGrDoc[n].LineColor;
	dlg.LineWidth = sGrDoc[n].LineWidth;
	dlg.LineType  =  sGrDoc[n].LineType;

	if( dlg.DoModal() == IDOK )
	{
		sGrDoc[n].LineColor = dlg.LineColor;
		sGrDoc[n].LineWidth = dlg.LineWidth;
		sGrDoc[n].LineType  =  dlg.LineType;
	};

	RedrawWindow();	
	return;
}

void CTmcGroutColorGraphDialog::OnGrapparcordefault() 
{
	// TODO: Add your control notification handler code here
	SetDefault();
	RedrawWindow();	
	return;
}

void CTmcGroutColorGraphDialog::SetDefault()
{
	int i;
	for( i = 0; i < 16; i++)
	{
		sGrDoc[i].nPoint = 0;
		sGrDoc[i].pFreq  = NULL;
		sGrDoc[i].pSmatr = NULL;
		sGrDoc[i].pcExpr = NULL;
		sGrDoc[i].nInp1  = 0;
		sGrDoc[i].nMod1  = 0;
		sGrDoc[i].nInp2  = 0;
		sGrDoc[i].nInp2  = 0;
		sGrDoc[i].szFileName = NULL;
		sGrDoc[i].szGrapName = NULL;
		sGrDoc[i].szGrapPodp = NULL;
		sGrDoc[i].LastWriteTime.dwLowDateTime   = 0;
		sGrDoc[i].LastWriteTime.dwHighDateTime  = 0;
		sGrDoc[i].OutFlag = TRUE;
		switch(i)
		{
		case  0:	sGrDoc[i].LineColor = RGB( 0, 0, 255);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[3].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  1:	sGrDoc[i].LineColor = RGB( 0, 255, 0);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[2].nX =  0;
					sGrDoc[i].piPoint[2].nY =  0;
					sGrDoc[i].piPoint[3].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[3].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  2:	sGrDoc[i].LineColor = RGB( 0, 255, 255);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX =  0;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[3].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  3:	sGrDoc[i].LineColor = RGB( 255, 0, 255);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX =  0;
					sGrDoc[i].piPoint[1].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[3].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[3].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[4].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[5].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[6].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[7].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[8].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[9].nY =  TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  4:	sGrDoc[i].LineColor = RGB( 255, 255, 0);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY =  0;
					sGrDoc[i].piPoint[1].nX =  0;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY =  0;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[4].nY =  0;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[5].nY =  0;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[6].nY =  0;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[7].nY =  0;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[8].nY =  0;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[9].nY =  0;
			break;
		case  5:	sGrDoc[i].LineColor = RGB( 0, 0, 128);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY =  0;
					sGrDoc[i].piPoint[1].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  0;
					sGrDoc[i].piPoint[2].nX =  0;
					sGrDoc[i].piPoint[2].nY =  0;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX =  0;
					sGrDoc[i].piPoint[4].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX =  0;
					sGrDoc[i].piPoint[5].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX =  0;
					sGrDoc[i].piPoint[6].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX =  0;
					sGrDoc[i].piPoint[7].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX =  0;
					sGrDoc[i].piPoint[8].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX =  0;
					sGrDoc[i].piPoint[9].nY =  TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  6:	sGrDoc[i].LineColor = RGB( 0, 128, 0);
					sGrDoc[i].piPoint[0].nX =  0;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  0;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX =  0;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX =  0;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX =  0;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX =  0;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX =  0;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX =  0;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  7:	sGrDoc[i].LineColor = RGB( 0, 128, 128);
					sGrDoc[i].piPoint[0].nX =  0;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  0;
					sGrDoc[i].piPoint[2].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX =  0;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX =  0;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX =  0;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX =  0;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX =  0;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX =  0;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  8:	sGrDoc[i].LineColor = RGB( 128, 20, 0);
					sGrDoc[i].piPoint[0].nX =  0;
					sGrDoc[i].piPoint[0].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  0;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX =  0;
					sGrDoc[i].piPoint[4].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX =  0;
					sGrDoc[i].piPoint[5].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX =  0;
					sGrDoc[i].piPoint[6].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX =  0;
					sGrDoc[i].piPoint[7].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX =  0;
					sGrDoc[i].piPoint[8].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX =  0;
					sGrDoc[i].piPoint[9].nY =  TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case  9:	sGrDoc[i].LineColor = RGB( 128, 0, 128);
					sGrDoc[i].piPoint[0].nX =  0;
					sGrDoc[i].piPoint[0].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  0;
					sGrDoc[i].piPoint[2].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX =  0;
					sGrDoc[i].piPoint[4].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX =  0;
					sGrDoc[i].piPoint[5].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX =  0;
					sGrDoc[i].piPoint[6].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX =  0;
					sGrDoc[i].piPoint[7].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX =  0;
					sGrDoc[i].piPoint[8].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX =  0;
					sGrDoc[i].piPoint[9].nY =  TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case 10:	sGrDoc[i].LineColor = RGB( 128, 128, 0);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY =  0;
					sGrDoc[i].piPoint[3].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case 11:	sGrDoc[i].LineColor = RGB( 128, 128, 128);
					sGrDoc[i].piPoint[0].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[1].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[2].nX = -TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[2].nY =  0;
					sGrDoc[i].piPoint[3].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[4].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[5].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[6].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[7].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[8].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/2;
					sGrDoc[i].piPoint[9].nX =  TMC_GROUT_POINT_TYPE_X/2;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/2;
			break;
		case 12:	sGrDoc[i].LineColor = RGB( 0, 0, 192);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[1].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[2].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[3].nX =  TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/4;
			break;
		case 13:	sGrDoc[i].LineColor = RGB( 0, 192, 0);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[0].nY =  0;
					sGrDoc[i].piPoint[1].nX =  0;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[2].nY =  0;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[4].nY =  0;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[5].nY =  0;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[6].nY =  0;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[7].nY =  0;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[8].nY =  0;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[9].nY =  0;
			break;
		case 14:	sGrDoc[i].LineColor = RGB( 0, 192, 192);
					sGrDoc[i].piPoint[0].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[0].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[1].nX =  0;
					sGrDoc[i].piPoint[1].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[2].nX =  TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[2].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[3].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[3].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[4].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[4].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[5].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[5].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[6].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[6].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[7].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[7].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[8].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[8].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[9].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[9].nY = -TMC_GROUT_POINT_TYPE_Y/4;
			break;
		case 15:	sGrDoc[i].LineColor = RGB( 192, 192, 0);
					sGrDoc[i].piPoint[0].nX =  0;
					sGrDoc[i].piPoint[0].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[1].nX =  TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[1].nY =  0;
					sGrDoc[i].piPoint[2].nX = -TMC_GROUT_POINT_TYPE_X/4;
					sGrDoc[i].piPoint[2].nY = -TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[3].nX =  0;
					sGrDoc[i].piPoint[3].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[4].nX =  0;
					sGrDoc[i].piPoint[4].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[5].nX =  0;
					sGrDoc[i].piPoint[5].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[6].nX =  0;
					sGrDoc[i].piPoint[6].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[7].nX =  0;
					sGrDoc[i].piPoint[7].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[8].nX =  0;
					sGrDoc[i].piPoint[8].nY =  TMC_GROUT_POINT_TYPE_Y/4;
					sGrDoc[i].piPoint[9].nX =  0;
					sGrDoc[i].piPoint[9].nY =  TMC_GROUT_POINT_TYPE_Y/4;
			break;
		}
		sGrDoc[i].LineType  = PS_SOLID;
		sGrDoc[i].LineWidth = 50/15;
		sGrDoc[i].PointColor = RGB( 0, 255, 255);
		sGrDoc[i].PointType  = PS_SOLID;
		sGrDoc[i].PointWidth = 10;
	};
	
	return;
}

void CTmcGroutColorGraphDialog::OnColorLevelAutoSet() 
{
	// TODO: Add your control notification handler code here
	ChangeGraphicsPar( 0 );
	return;	
}
