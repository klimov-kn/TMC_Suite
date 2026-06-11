// ThreadCalcDirPat.cpp : implementation file
//

#include "stdafx.h"
#include "tmcgrout.h"


#include <tmcgrviw.h>
#include "MainFrm.h"
#include "..\\DiaNapr\\c2DArray.h"
#include "TMCGROUTDoc.h"
#include "TMCGROUTView.h"

#include "ThreadCalcDirPat.h"

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

	if( pDlg != NULL )
	{
		CTMCGROUTDoc* pDoc = pDlg->GetDocument();
		ASSERT_VALID(pDoc);
		if( (pDoc->pcDirectionalPattern != NULL) )
		{
			if( ((pDoc->grdoc.nGraph == (pDoc->nGraphCurrent + 1))&&(pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ].iN > (pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ].GetnAngle() - 2) ))||(pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ].IsError()) ) 
			{
				EndDialog( 0 );
			}
			else
			{
				m_csStatistics.Format("%#5.3lg%%", ((double)(pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ].iN/10.)) );
				m_csStatisticsAll.Format("%#5.3lg%%", ((double)((pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ].iN/10.+pDoc->nGraphCurrent*100)/pDoc->grdoc.nGraph)) );
				m_CurrentDirPat = pDoc->nGraphCurrent + 1;
				m_NAllDirPat = pDoc->grdoc.nGraph;
				m_nEmitters = pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ].GetnIzl();
//				m_csStatistics.Format("Directional Pattern %d from %d; calculate %#5.3lg%%", pDoc->nGraphCurrent + 1, pDoc->grdoc.nGraph, ((double)(pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ].iN/10.)) );
//				m_csStatisticsAll.Format("Please Wait already calculate %#5.3lg%%", ((double)((pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ].iN/10.+pDoc->nGraphCurrent*100)/pDoc->grdoc.nGraph)) );
				{
					int ii;
					ltime_start = pDlg->ltime_start;
					time( &ltime_current );
					ii = ltime_current - ltime_start;
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
				m_Progress.SetPos( pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ].iN/10 );
				m_ProgressAll.SetPos( (int)(pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ].iN/10.+pDoc->nGraphCurrent*100)/pDoc->grdoc.nGraph  );
			};
		};
	};
	
	CDialog::OnTimer(nIDEvent);
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

//	CTMCGROUTView* pDlg = (CTMCGROUTView *)(pCView);
//	CTMCGROUTDoc* pDoc = pDlg->GetDocument();
//	ASSERT_VALID(pDoc);
//	m_nEmitters = pDoc->pcDirectionalPattern[ pDoc->nGraphCurrent ].GetnIzl();
	
	return CDialog::DoModal();
}
