// ThreadCalcDirPat.cpp : implementation file
//

#include "stdafx.h"
#include "tmcgrout.h"


#include <tmcgrviw.h>
#include "MainFrm.h"
#include "..\\DiaNapr\\c2DArray.h"
#include "TMCGROUTDoc.h"
#include "TMCGROUTView.h"

#include "threadcalcdirpat.h"

// I-33: the percent of the progress is 100*iN/nAngle (old: iN/10 - right only for 1000 angles)
/// @brief I-33: дол€ посчитанных углов текущей ƒЌ, проценты.
/// @param[in] a  рассчитываема€ диаграмма
/// @return 100*iN/nAngle; 0 Ч сетка углов не задана.
static double TmcDnPercent( c2DArray &a )
{
	int n = a.GetnAngle();
	return ( n > 0 ) ? 100.*a.iN/n : 0.;
}

extern volatile LONG g_lTmcDnCalcRunning;	// A-30: tmcgroutview.cpp

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CThreadCalcDirPat dialog


CThreadCalcDirPat::CThreadCalcDirPat(CWnd* pParent /*=NULL*/)
	: CDialog(CThreadCalcDirPat::IDD, pParent)
{
	//{{AFX_DATA_INIT(CThreadCalcDirPat)
	m_csStatistics = _T("");
	m_csStatisticsAll = _T("");
	m_CurrentDirPat = 1;
	m_NAllDirPat = 1;
	m_csElapsTime = _T("");
	m_nEmitters = 0;
	//}}AFX_DATA_INIT
	pCView = NULL;
}


void CThreadCalcDirPat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CThreadCalcDirPat)
	DDX_Control(pDX, IDC_PROGRESS2, m_ProgressAll);
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
	DDX_Text(pDX, IDC_EDIT1, m_csStatistics);
	DDX_Text(pDX, IDC_EDIT2, m_csStatisticsAll);
	DDX_Text(pDX, IDC_EDIT_CURDIRPAT, m_CurrentDirPat);
	DDX_Text(pDX, IDC_EDIT_NUMDIRPAT, m_NAllDirPat);
	DDX_Text(pDX, IDC_EDIT_TIMEELAPS, m_csElapsTime);
	DDX_Text(pDX, IDC_EDIT_NUMBEMITTERS, m_nEmitters);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CThreadCalcDirPat, CDialog)
	//{{AFX_MSG_MAP(CThreadCalcDirPat)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CThreadCalcDirPat message handlers

void CThreadCalcDirPat::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	CTMCGROUTView* pDlg = (CTMCGROUTView *)(pCView);

	if( g_lTmcDnCalcRunning == 0 )	// A-30, S-55: okno zakryvaetsya, kogda potok rascheta zavershilsya
	{
		EndDialog( 0 );
		CDialog::OnTimer(nIDEvent);
		return;
	};

	if( pDlg != NULL )
	{
		CTMCGROUTDoc* pDoc = pDlg->GetDocument();
		ASSERT_VALID(pDoc);
		if( (pDoc->pcDirectionalPattern != NULL) )
		{
			if( ( pDoc->grdoc.nGraph <= 0 )||( pDoc->nGraphCurrent < 0 )||( pDoc->nGraphCurrent >= pDoc->grdoc.nGraph ) )	// S-55: delenie na 0; A-30: zakrytie - tolko po koncu potoka
			{
				;
			}
			else
			{
				// I-33: проценты Ч 100*iN/nAngle дл€ любой сетки углов (было iN/10 Ч только дл€ 1000 углов;
				// у сетки из 400 углов полоса доходила до 40%). ¬рем€ нормировки ( Ќƒ) в ходе не отражаетс€.
				m_csStatistics.Format("%#5.3lg%%", TmcDnPercent( pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ] ) );
				m_csStatisticsAll.Format("%#5.3lg%%", ( TmcDnPercent( pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ] )+pDoc->nGraphCurrent*100 )/pDoc->grdoc.nGraph );
				m_CurrentDirPat = pDoc->nGraphCurrent + 1;
				m_NAllDirPat = pDoc->grdoc.nGraph;
				m_nEmitters = pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ].GetnIzl();
				{
					int ii;
					ltime_start = pDlg->ltime_start;
					time( &ltime_current );
					ii = (int)( ltime_current - ltime_start );
					if( ii <= 60 )
					{
						m_csElapsTime.Format("%2d", ii );
					}
					else
					{
						if( ii <= 3600 )
						{
							m_csElapsTime.Format("%2d:%2d", ii/60, ii-(ii/60)*60 );
						}
						else
						{
							m_csElapsTime.Format("%2d:%2d:%2d", ii/3600, ii/60-(ii/3600)*60 , ii-(ii/60)*60 );
						}
					}
				};
				
				UpdateData(FALSE);
				m_Progress.SetPos( (int)TmcDnPercent( pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ] ) );	// I-33
				m_ProgressAll.SetPos( (int)( TmcDnPercent( pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ] )+pDoc->nGraphCurrent*100 )/pDoc->grdoc.nGraph  );
			};
		};
	};
	
	CDialog::OnTimer(nIDEvent);
}

void CThreadCalcDirPat::OnCancel()	// A-30: "Hide" ne zakryvaet okno, poka idet raschet
{
	if( g_lTmcDnCalcRunning != 0 ) return;
	CDialog::OnCancel();
}

void CThreadCalcDirPat::OnClose()	// A-30: krestik - tozhe
{
	if( g_lTmcDnCalcRunning != 0 ) return;
	CDialog::OnClose();
}

BOOL CThreadCalcDirPat::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	SetTimer( 1, 100, NULL);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

INT_PTR CThreadCalcDirPat::DoModal()
{
	// TODO: Add your specialized code here and/or call the base class

	time( &ltime_start );

	
	return CDialog::DoModal();
}
