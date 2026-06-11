// TmcDialogStatistics.cpp : implementation file
//

#include "stdafx.h"
#include "PlanRT_H.h"
#include "TmcDialogStatistics.h"
#include "TmcRTH_DialogBlock.h"
#include "TmcRTHNodeDiel.h"
#include <limits.h>
#include <float.h>
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcDialogStatistics dialog


CTmcDialogStatistics::CTmcDialogStatistics(CWnd* pParent /*=NULL*/)
	: CDialog(CTmcDialogStatistics::IDD, pParent)
{
	pcBlockList = NULL;
	m_cOut = NULL;
	csTolerance = "100 %";
	//{{AFX_DATA_INIT(CTmcDialogStatistics)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTmcDialogStatistics::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTmcDialogStatistics)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Text(pDX, IDC_TMCSTATISTICSNUMMEMORYMB,  m_dMemory);
	DDX_Text(pDX, IDC_TMCSTATISTICSNUMNODE,  m_nNumNode);
	DDX_Text(pDX, IDC_TMCSTATISTICSNSTEP,  m_nStep);
	DDX_Text(pDX, IDC_TMCSTATISTICSNBLOCK,  m_nBlock);
	DDX_Text(pDX, IDC_TMCSTATISTICSERRORMESSAGE,  m_csErrorMessage);
	DDX_Text(pDX, IDC_TMCSTATISTICSTSIZEWL,  dTsizeWL);
	DDX_Text(pDX, IDC_TMCSTATISTICSTSIZEWL1,  dTsizeWL1);
	DDX_Text(pDX, IDC_TMCSTATISTICSTMAX,  dTmax);
	DDX_Text(pDX, IDC_TMCSTATISTICSTMIN,  dTmin);
	DDX_Text(pDX, IDC_TMCSTATISTICSXSIZEWL,  dXsizeWL);
	DDX_Text(pDX, IDC_TMCSTATISTICSXMAX,  dXmax);
	DDX_Text(pDX, IDC_TMCSTATISTICSXMIN,  dXmin);
	DDX_Text(pDX, IDC_TMCSTATISTICSYSIZEWL,  dYsizeWL);
	DDX_Text(pDX, IDC_TMCSTATISTICSYMAX,  dYmax);
	DDX_Text(pDX, IDC_TMCSTATISTICSYMIN,  dYmin);
	DDX_Text(pDX, IDC_TMCSTATISTICST0,    dt);
	DDX_Text(pDX, IDC_TMCSTATISTICST2,    dtT);
	DDX_Text(pDX, IDC_TMCSTATISTICSDELTA, dDelta);
	DDX_Text(pDX, IDC_TMCSTATISTICSFREQ,  dFreq);
	DDX_Text(pDX, IDC_TMCSTATISTICSWAVELENGHT,  dWaveLen);
	DDX_Text(pDX, IDC_TMCSTATISTICSWAVELENGHTDELT,  dWaveLenDelt);
	DDX_Text(pDX, IDC_TMCSTATISTICSANGLEUNIT,  m_cParam.csAngleUnit);
	DDX_Text(pDX, IDC_TMCSTATISTICSFREQUNIT,  m_cParam.csFreqUnit);
	DDX_Text(pDX, IDC_TMCSTATISTICSLONGUNIT,  m_cParam.csLongUnit);
	DDX_Text(pDX, IDC_TMCSTATISTICSTIMEUNIT,  m_cParam.csTimeUnit);
	DDX_Text(pDX, IDC_TMCSTATISTICSFILEOUT,  m_cOut->csFileName);
	DDX_Text(pDX, IDC_TMCSTATISTICSFILEOUT2,  m_cOut->csFileNameField);
	DDX_Text(pDX, IDC_TMCSTATISTICSFILEOUT3,  m_cOut->csFileNameTopology);
	DDX_Text(pDX, IDC_TMCSTATISTICSACCURACY,  csAccuracy);
	DDX_Text(pDX, IDC_TMCSTATISTICSTOLERANCE,  csTolerance);
	DDX_Check(pDX, IDC_TMCSTATISTICSFIELDOUT,  m_cOut->bOutField);
	DDX_Check(pDX, IDC_TMCSTATISTICSTOPOLOGYOUT,  m_cOut->bOutTopology);
	//}}AFX_DATA_MAP
}	
			

void CTmcDialogStatistics::OnViewBlockInformation()
{
	if( m_nBlock <= 0 ) return;
	
	CTmcRTH_BlockList *pcBlockList1 = m_cTopol.GetBlockList();
	if( pcBlockList1 == NULL ) return;

	DeleteData();

	if( (pcBlockList = new CTmcRTH_BlockList[m_nBlock] ) == NULL ) return;
	
	int i;
	
	for( i = 0; i < m_nBlock; i++, pcBlockList1 = pcBlockList1->GetNext() ) 
	{
		if(pcBlockList1 == NULL)
		{
			DeleteData();
			return;
		};
		pcBlockList[i] = *pcBlockList1;
	};

	CTmcRTH_DialogBlock cDlg;

	for( i = 0; ; i = cDlg.i)
	{
		if( i >= m_nBlock) 
		{
			i = m_nBlock - 1;
			cDlg.i = i;
		};
		if( i < 0 )
		{
			DeleteData();
			return;
		};
		cDlg.m_pcParam = &m_cParam;
		cDlg.m_pcBlockList = pcBlockList + i;
		cDlg.PrepareData();
		if( cDlg.DoModal() != 1 )
		{
			DeleteData();
			return;
		};
	};

	DeleteData();
	return;
}


BEGIN_MESSAGE_MAP(CTmcDialogStatistics, CDialog)
	//{{AFX_MSG_MAP(CTmcDialogStatistics)
		// NOTE: the ClassWizard will add message map macros here
	ON_BN_CLICKED(IDC_TMCSTATISTICSVIEWBLOCKLIST, OnCancel1)
	ON_BN_CLICKED(IDC_TMCSTATISTICSPREVSTEP, OnStepPrev)
	ON_BN_CLICKED(IDC_TMCSTATISTICSNEXTSTEP, OnStepNext)
	ON_BN_CLICKED(IDC_TMCSTATISTICSVIEWBLOCKLIST2, OnViewBlockInformation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTmcDialogStatistics message handlers

void CTmcDialogStatistics::PrepareData( void )
{
	double dwt0, dTolerance;
	int i;
	csAccuracy = GetModel();
	dTolerance = 100.;
	nFlagStep = 0;
	dXsizeWL = 0.0;
	dYsizeWL = 0.0;
	dTsizeWL = 0.0;
	dTsizeWL1 = 0.0;
	m_nNumNode = 0;
	m_dMemory = 0.0;
	m_nBlock = m_cTopol.GetBlockNumber();
	dDelta   = m_cParam.rDelta/m_cParam.rLongUnit;
	dFreq    = m_cParam.rFreq/m_cParam.rFreqUnit;

	CTmcRTH_BlockList *pcBlockList1 = m_cTopol.GetBlockList();
	
	if( pcBlockList1 != NULL )
	{
		m_dMemory = pcBlockList1->GetnMemoryAll();
	};

	if( (fabs( (double)(m_cParam.rFreq) ) < (1.e-38))|| (fabs( (double)(m_cParam.rLongUnit) ) < (1.e-38)) )
	{
		dWaveLen = 0;
	}
	else
	{
		dWaveLen = (C0___/m_cParam.rFreq)/m_cParam.rLongUnit;
		dXsizeWL = ((int)(( m_cParam.rXmax - m_cParam.rXmin )*1000/(C0___/m_cParam.rFreq)+0.5))/1000.;
		dYsizeWL = ((int)(( m_cParam.rYmax - m_cParam.rYmin )*1000/(C0___/m_cParam.rFreq)+0.5))/1000.;
	};
	if( fabs( (double)(dDelta) ) < (1.e-38) )
	{
		dWaveLenDelt = 0;
	}
	else
	{
		int nnn = 1;
		dWaveLenDelt = dWaveLen/dDelta;
		m_nNumNode = ((int)(( m_cParam.rXmax - m_cParam.rXmin )/m_cParam.rDelta)+1)*((int)(( m_cParam.rYmax - m_cParam.rYmin )/m_cParam.rDelta)+1);
		if(m_cOut != NULL)
		{
			if(m_cOut->GetFieldIntegrated().bIsItegratedInMemory()) nnn = 3;
		};
		m_dMemory = ((int)((( m_dMemory + m_nBlock*sizeof(CTmcRTHNodeDiel)+m_nNumNode*nnn*(2.*sizeof(_real)+2.*sizeof(_real) ))*1000.)/(1024.*1024.)+0.5))/1000.;
	};
	dt       = m_cParam.rt/m_cParam.rTimeUnit;
	if( fabs( (double)(m_cParam.rt*m_cParam.rFreq) ) < (1.e-38) )
	{
		dtT = 0;
	}
	else
	{
		dtT = 1./(m_cParam.rt*m_cParam.rFreq);
		dTsizeWL = (int)(( m_cParam.rTmax - m_cParam.rTmin )/(m_cParam.rt)+0.5);
		dTsizeWL1 = (int)(( m_cParam.rTmax - m_cParam.rTmin )*(m_cParam.rFreq)+0.5);
	};

	dwt0 = fabs(m_cParam.rt*m_cParam.rFreq*2*PI___);
	if( dwt0 < FLT_MIN ) 
	{
		dTolerance = 0;
	}
	else
	{
		dTolerance = acos( 2*cos(dwt0) - 1.)/(sqrt(2.)*dwt0)-1.;
		i = (int)(dTolerance*1000000);
		dTolerance = i/(10000.) ;
	};
	csTolerance.Format("%lg ", dTolerance);
	csTolerance += "%";
	dXmin    = m_cParam.rXmin/m_cParam.rLongUnit;
	dXmax    = m_cParam.rXmax/m_cParam.rLongUnit;
	dYmin    = m_cParam.rYmin/m_cParam.rLongUnit;
	dYmax    = m_cParam.rYmax/m_cParam.rLongUnit;
	dTmin    = m_cParam.rTmin/m_cParam.rTimeUnit;
	dTmax    = m_cParam.rTmax/m_cParam.rTimeUnit;
	return;
}

void CTmcDialogStatistics::DeleteData( void )
{
	if( pcBlockList != NULL )
	{
		delete []pcBlockList;
		pcBlockList = NULL;
	};
	return;
}

void CTmcDialogStatistics::OnStepNext( void )
{
	nFlagStep = 1;
	CDialog::OnOK();
	return;
}

void CTmcDialogStatistics::OnStepPrev( void )
{
	nFlagStep = -1;
	CDialog::OnOK();
	return;
}

void CTmcDialogStatistics::OnCancel1( void )
{
	nFlagStep = -1;
	CDialog::OnCancel();
	return;
}
