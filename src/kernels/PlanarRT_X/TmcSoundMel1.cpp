// TmcSoundMel1.cpp : implementation file
//

#include "stdafx.h"
#include "PlanRT_H.h"
#include "TmcSoundMel1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcSoundMel1 property page

IMPLEMENT_DYNCREATE(CTmcSoundMel1, CPropertyPage)

CTmcSoundMel1::CTmcSoundMel1() : CPropertyPage(CTmcSoundMel1::IDD)
{
	//{{AFX_DATA_INIT(CTmcSoundMel1)
	m_SoundInterval = 1;
	m_Note1 = 12;
	m_Note2 = 12;
	m_Note3 = 12;
	m_Note4 = 12;
	m_Note5 = 12;
	m_Note6 = 12;
	m_Note7 = 12;
	m_Note8 = 12;
	m_Note9 = 12;
	m_Note10 = 12;
	m_Note11 = 12;
	m_Note12 = 12;
	csMelodyName = _T("");
	//}}AFX_DATA_INIT
}

CTmcSoundMel1::~CTmcSoundMel1()
{
}

void CTmcSoundMel1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTmcSoundMel1)
	DDX_Text(pDX, IDC_TmcSoundEffects_TimeInterval, m_SoundInterval);
	DDV_MinMaxInt(pDX, m_SoundInterval, 1, 60000);
	DDX_Radio(pDX, IDC_TmcSoundEffects_Sound, m_Note1);
	DDX_Radio(pDX, IDC_TmcSoundEffects_Sound98, m_Note2);
	DDX_Radio(pDX, IDC_TmcSoundEffects_Sound111, m_Note3);
	DDX_Radio(pDX, IDC_TmcSoundEffects_Sound124, m_Note4);
	DDX_Radio(pDX, IDC_TmcSoundEffects_Sound137, m_Note5);
	DDX_Radio(pDX, IDC_TmcSoundEffects_Sound150, m_Note6);
	DDX_Radio(pDX, IDC_TmcSoundEffects_Sound163, m_Note7);
	DDX_Radio(pDX, IDC_TmcSoundEffects_Sound176, m_Note8);
	DDX_Radio(pDX, IDC_TmcSoundEffects_Sound189, m_Note9);
	DDX_Radio(pDX, IDC_TmcSoundEffects_Sound202, m_Note10);
	DDX_Radio(pDX, IDC_TmcSoundEffects_Sound215, m_Note11);
	DDX_Radio(pDX, IDC_TmcSoundEffects_Sound228, m_Note12);
	DDX_Text(pDX, IDC_EDIT1, csMelodyName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTmcSoundMel1, CPropertyPage)
	//{{AFX_MSG_MAP(CTmcSoundMel1)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTmcSoundMel1 message handlers
