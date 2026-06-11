// TmcRTH_DialogBlock.cpp : implementation file
//

#include "stdafx.h"
#include "PlanRT_H.h"
#include <tmcgrviw.h>
#include "TmcRTH_DialogBlock.h"
#include "TmcRTH_Indan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_DialogBlock dialog


CTmcRTH_DialogBlock::CTmcRTH_DialogBlock(CWnd* pParent /*=NULL*/)
	: CDialog(CTmcRTH_DialogBlock::IDD, pParent)
{
	m_pcParam = NULL;
	m_pcBlockList = NULL;
	i = 0;
	//{{AFX_DATA_INIT(CTmcRTH_DialogBlock)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTmcRTH_DialogBlock::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTmcRTH_DialogBlock)
	DDX_Text(pDX, IDC_TMCBLOCKNUMBER,  m_nBlock);
	DDX_Text(pDX, IDC_TMCBLOCKNPOINTXY,  m_nXY);
	DDX_Text(pDX, IDC_TMCBLOCKMEMORY1,  m_nMemory);
	DDX_Text(pDX, IDC_TMCBLOCKDX1,  dX1);
	DDX_Text(pDX, IDC_TMCBLOCKDX2,  dX2);
	DDX_Text(pDX, IDC_TMCBLOCKDX3,  dX3);
	DDX_Text(pDX, IDC_TMCBLOCKDX4,  dX4);
	DDX_Text(pDX, IDC_TMCBLOCKDX5,  dX5);
	DDX_Text(pDX, IDC_TMCBLOCKDX6,  dX6);
	DDX_Text(pDX, IDC_TMCBLOCKDX7,  dX7);
	DDX_Text(pDX, IDC_TMCBLOCKDX8,  dX8);
	DDX_Text(pDX, IDC_TMCBLOCKDX9,  dX9);
	DDX_Text(pDX, IDC_TMCBLOCKDX10,  dX10);
	DDX_Text(pDX, IDC_TMCBLOCKDY1,  dY1);
	DDX_Text(pDX, IDC_TMCBLOCKDY2,  dY2);
	DDX_Text(pDX, IDC_TMCBLOCKDY3,  dY3);
	DDX_Text(pDX, IDC_TMCBLOCKDY4,  dY4);
	DDX_Text(pDX, IDC_TMCBLOCKDY5,  dY5);
	DDX_Text(pDX, IDC_TMCBLOCKDY6,  dY6);
	DDX_Text(pDX, IDC_TMCBLOCKDY7,  dY7);
	DDX_Text(pDX, IDC_TMCBLOCKDY8,  dY8);
	DDX_Text(pDX, IDC_TMCBLOCKDY9,  dY9);
	DDX_Text(pDX, IDC_TMCBLOCKDY10,  dY10);
	DDX_Text(pDX, IDC_TMCBLOCKNTYPE,  m_nType);
	DDX_Text(pDX, IDC_TMCBLOCKSTRING,  m_csString);
	DDX_Text(pDX, IDC_TMCBLOCKVXSPEED,  m_csVx);
	DDX_Text(pDX, IDC_TMCBLOCKVYSPEED,  m_csVy);
	DDX_Text(pDX, IDC_TMCBLOCKWSPEED,  m_csW);
	DDX_Text(pDX, IDC_TMCBLOCKEXPREPS,  m_csEpsExpr);
	DDX_Text(pDX, IDC_TMCBLOCKX0,  dX0);
	DDX_Text(pDX, IDC_TMCBLOCKY0,  dY0);
	DDX_Text(pDX, IDC_TMCBLOCKXMINBTEXT,  csXminText);
	DDX_Text(pDX, IDC_TMCBLOCKXMAXBTEXT,  csXmaxText);
	DDX_Text(pDX, IDC_TMCBLOCKYMINBTEXT,  csYminText);
	DDX_Text(pDX, IDC_TMCBLOCKYMAXBTEXT,  csYmaxText);
	DDX_Text(pDX, IDC_TMCBLOCKXMINB,  dXmin);
	DDX_Text(pDX, IDC_TMCBLOCKXMAXB,  dXmax);
	DDX_Text(pDX, IDC_TMCBLOCKYMINB,  dYmin);
	DDX_Text(pDX, IDC_TMCBLOCKYMAXB,  dYmax);
	DDX_Text(pDX, IDC_TMCBLOCKLONGUNIT,  m_csLongUnit);
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTmcRTH_DialogBlock, CDialog)
	//{{AFX_MSG_MAP(CTmcRTH_DialogBlock)
	ON_BN_CLICKED(ID_TMCBLOCKBUTTONNEXT2, OnCancel1)
	ON_BN_CLICKED(ID_TMCBLOCKBUTTONNEXT, OnTmcblockbuttonnext)
	ON_BN_CLICKED(ID_TMCBLOCKBUTTONPREV, OnTmcblockbuttonprev)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_DialogBlock message handlers

void CTmcRTH_DialogBlock::PrepareData( void )
{
	int i;
	double *pX, *pY;
	m_nBlock = 0;
	m_csString.Format("");
	m_csVx.Format("");
	m_csVy.Format("");
	m_csW.Format("");
	m_csEpsExpr.Format("");
	csXminText.Format("");
	csXmaxText.Format("");
	csYminText.Format("");
	csYmaxText.Format("");
	dX0 = 0.0;
	dY0 = 0.0;
	dXmin = 0.0;
	for( i = 0; i < 10; i++ )
	{
		dX[i].Format("");
		dY[i].Format("");
	};
	dX1 = dX[0];
	dX2 = dX[1];
	dX3 = dX[2];
	dX4 = dX[3];
	dX5 = dX[4];
	dX6 = dX[5];
	dX7 = dX[6];
	dX8 = dX[7];
	dX9 = dX[8];
	dX10 = dX[9];
	dY1 = dY[0];
	dY2 = dY[1];
	dY3 = dY[2];
	dY4 = dY[3];
	dY5 = dY[4];
	dY6 = dY[5];
	dY7 = dY[6];
	dY8 = dY[7];
	dY9 = dY[8];
	dY10 = dY[9];
	dXmax = 0.0;
	dYmin = 0.0;
	dYmax = 0.0;
	m_nXY = 0;
	m_nType = 0;
	m_csLongUnit.Format("");
	if( m_pcParam == NULL ) return;
	if( m_pcBlockList == NULL ) return;
	m_csLongUnit = m_pcParam->csGetLongUnit();
	m_nBlock = m_pcBlockList->GetnBlock();
	m_nXY = m_pcBlockList->GetnXY();
	m_nType = m_pcBlockList->GetnType();
	m_csString = m_pcBlockList->GetcsBlock();
	m_csVx = m_pcBlockList->GetcsVx();
	m_csVy = m_pcBlockList->GetcsVy();
	m_csW = m_pcBlockList->GetcsW();
	m_nMemory = ((int)((m_pcBlockList->GetnMemory()*1000.)/1024./1024.+0.5))/1000.;
	m_csEpsExpr = m_pcBlockList->GetcsEpsExpr();
	dX0 = m_pcBlockList->GetX0()/m_pcParam->rGetLongUnit();
	dY0 = m_pcBlockList->GetY0()/m_pcParam->rGetLongUnit();
	dXmin = m_pcBlockList->GetXmin()/m_pcParam->rGetLongUnit();
	dXmax = m_pcBlockList->GetXmax()/m_pcParam->rGetLongUnit();
	if( (strcmp( m_csString, CTMCRTH_INDANBLCK_RECTSTAT ) == 0)||
		(strcmp( m_csString, CTMCRTH_INDANBLCK_RECTMOVE ) == 0)||
		(strcmp( m_csString, CTMCRTH_INDANBLCK_FILE ) == 0) )
	{
		dYmin = m_pcBlockList->GetYmin()/m_pcParam->rGetLongUnit();
		dYmax = m_pcBlockList->GetYmax()/m_pcParam->rGetLongUnit();
		csXminText.Format("Xmin");
		csXmaxText.Format("Xmax");
		csYminText.Format("Ymin");
		csYmaxText.Format("Ymax");
	};

	if( (strcmp( m_csString, CTMCRTH_INDANBLCK_CIRCSTAT ) == 0)||
		(strcmp( m_csString, CTMCRTH_INDANBLCK_CIRCMOVE ) == 0) )
	{
		dYmin = m_pcBlockList->GetYmin()/m_pcParam->rAngleUnit;
		dYmax = m_pcBlockList->GetYmax()/m_pcParam->rAngleUnit;
		csXminText.Format("Rmin");
		csXmaxText.Format("Rmax");
		csYminText.Format("FImin");
		csYmaxText.Format("FImax");
	};
	if( (strcmp( m_csString, CTMCRTH_INDANBLCK_INPUT_X ) == 0)||
		(strcmp( m_csString, CTMCRTH_INDANBLCK_INPUT_Y ) == 0) )
	{
		dYmin = m_pcBlockList->GetYmin()/m_pcParam->rTimeUnit;
		dYmax = m_pcBlockList->GetYmax()/m_pcParam->rTimeUnit;
		if( strcmp( m_csString, CTMCRTH_INDANBLCK_INPUT_X ) == 0 )
		{
			csXminText.Format("Ymin");
			csXmaxText.Format("Ymax");
		}
		else
		{
			csXminText.Format("Xmin");
			csXmaxText.Format("Xmax");
		};
		csYminText.Format("Tmin");
		csYmaxText.Format("Tmax");
	};
	pX = m_pcBlockList->GetpX();
	pY = m_pcBlockList->GetpY();
	if( pX == NULL ) return;
	if( pY == NULL ) return;
	for( i = 0; i < m_nXY; i++ )
	{
//		CString csBuf1;
		dX[i].Format("%lg", pX[i]/m_pcParam->rGetLongUnit());
		dY[i].Format("%lg", pY[i]/m_pcParam->rGetLongUnit());
//		csBuf1.Format("{%lg;%lg}", pX[i], pY[i] );
//		AfxMessageBox(csBuf1);
	};
	dX1 = dX[0];
	dX2 = dX[1];
	dX3 = dX[2];
	dX4 = dX[3];
	dX5 = dX[4];
	dX6 = dX[5];
	dX7 = dX[6];
	dX8 = dX[7];
	dX9 = dX[8];
	dX10 = dX[9];
	dY1 = dY[0];
	dY2 = dY[1];
	dY3 = dY[2];
	dY4 = dY[3];
	dY5 = dY[4];
	dY6 = dY[5];
	dY7 = dY[6];
	dY8 = dY[7];
	dY9 = dY[8];
	dY10 = dY[9];
	return;
}

void CTmcRTH_DialogBlock::OnOK() 
{
	// TODO: Add extra validation here
	i = -1;
	CDialog::OnOK();
	return;
}

void CTmcRTH_DialogBlock::OnCancel1() 
{
	// TODO: Add extra validation here
	CDialog::OnCancel();
	return;
}

void CTmcRTH_DialogBlock::OnTmcblockbuttonnext() 
{
	// TODO: Add your control notification handler code here
	i++;
	CDialog::OnOK();
	return;
}

void CTmcRTH_DialogBlock::OnTmcblockbuttonprev() 
{
	// TODO: Add your control notification handler code here
	if( i <= 0 ) i = 1;
	i--;
	CDialog::OnOK();
	return;
}
