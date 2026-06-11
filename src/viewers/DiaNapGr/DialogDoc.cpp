// DialogDoc.cpp : implementation file
//

#include "stdafx.h"
#include "TMCGROUT.h"
#include <tmcgrviw.h>
#include "DialogDoc.h"
#include "cDeviation.h"
#include "DeviatDirPat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogDoc dialog


CDialogDoc::CDialogDoc(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogDoc::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogDoc)
	m_DocFileName = _T("");
	m_GraphName1 = _T("");
	m_GraphName10 = _T("");
	m_GraphName11 = _T("");
	m_GraphName12 = _T("");
	m_GraphName13 = _T("");
	m_GraphName14 = _T("");
	m_GraphName15 = _T("");
	m_GraphName16 = _T("");
	m_GraphName17 = _T("");
	m_GraphName18 = _T("");
	m_GraphName19 = _T("");
	m_GraphName2 = _T("");
	m_GraphName20 = _T("");
	m_GraphName3 = _T("");
	m_GraphName4 = _T("");
	m_GraphName5 = _T("");
	m_GraphName6 = _T("");
	m_GraphName7 = _T("");
	m_GraphName8 = _T("");
	m_GraphName9 = _T("");
	m_OutGrFlag_1  = TRUE;
	m_OutGrFlag_10 = TRUE;
	m_OutGrFlag_11 = TRUE;
	m_OutGrFlag_12 = TRUE;
	m_OutGrFlag_13 = TRUE;
	m_OutGrFlag_15 = TRUE;
	m_OutGrFlag_14 = TRUE;
	m_OutGrFlag_16 = TRUE;
	m_OutGrFlag_17 = TRUE;
	m_OutGrFlag_18 = TRUE;
	m_OutGrFlag_19 = TRUE;
	m_OutGrFlag_2  = TRUE;
	m_OutGrFlag_20 = TRUE;
	m_OutGrFlag_3  = TRUE;
	m_OutGrFlag_4  = TRUE;
	m_OutGrFlag_5  = TRUE;
	m_OutGrFlag_6  = TRUE;
	m_OutGrFlag_7  = TRUE;
	m_OutGrFlag_8  = TRUE;
	m_OutGrFlag_9  = TRUE;
	m_SFileName_1  = _T("");
	m_SFileName_10 = _T("");
	m_SFileName_11 = _T("");
	m_SFileName_12 = _T("");
	m_SFileName_13 = _T("");
	m_SFileName_14 = _T("");
	m_SFileName_15 = _T("");
	m_SFileName_16 = _T("");
	m_SFileName_17 = _T("");
	m_SFileName_18 = _T("");
	m_SFileName_19 = _T("");
	m_SFileName_2  = _T("");
	m_SFileName_20 = _T("");
	m_SFileName_3  = _T("");
	m_SFileName_4  = _T("");
	m_SFileName_5  = _T("");
	m_SFileName_6  = _T("");
	m_SFileName_7  = _T("");
	m_SFileName_8  = _T("");
	m_SFileName_9  = _T("");
	m_AddCharacteristicsFlag = FALSE;
	m_NormGrFlag1  = TRUE;
	m_NormGrFlag2  = TRUE;
	m_NormGrFlag3  = TRUE;
	m_NormGrFlag4  = TRUE;
	m_NormGrFlag5  = TRUE;
	m_NormGrFlag6  = TRUE;
	m_NormGrFlag7  = TRUE;
	m_NormGrFlag8  = TRUE;
	m_NormGrFlag9  = TRUE;
	m_NormGrFlag10 = TRUE;
	m_NormGrFlag11 = TRUE;
	m_NormGrFlag12 = TRUE;
	m_NormGrFlag13 = TRUE;
	m_NormGrFlag14 = TRUE;
	m_NormGrFlag15 = TRUE;
	m_NormGrFlag16 = TRUE;
	m_NormGrFlag17 = TRUE;
	m_NormGrFlag18 = TRUE;
	m_NormGrFlag19 = TRUE;
	m_NormGrFlag20 = TRUE;
	//}}AFX_DATA_INIT
}


void CDialogDoc::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogDoc)
	DDX_Text(pDX, IDC_GREDIT_DOCFILENAME, m_DocFileName);
	DDV_MaxChars(pDX, m_DocFileName, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME1, m_GraphName1);
	DDV_MaxChars(pDX, m_GraphName1, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME10, m_GraphName10);
	DDV_MaxChars(pDX, m_GraphName10, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME11, m_GraphName11);
	DDV_MaxChars(pDX, m_GraphName11, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME12, m_GraphName12);
	DDV_MaxChars(pDX, m_GraphName12, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME13, m_GraphName13);
	DDV_MaxChars(pDX, m_GraphName13, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME14, m_GraphName14);
	DDV_MaxChars(pDX, m_GraphName14, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME15, m_GraphName15);
	DDV_MaxChars(pDX, m_GraphName15, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME16, m_GraphName16);
	DDV_MaxChars(pDX, m_GraphName16, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME17, m_GraphName17);
	DDV_MaxChars(pDX, m_GraphName17, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME18, m_GraphName18);
	DDV_MaxChars(pDX, m_GraphName18, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME19, m_GraphName19);
	DDV_MaxChars(pDX, m_GraphName19, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME2, m_GraphName2);
	DDV_MaxChars(pDX, m_GraphName2, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME20, m_GraphName20);
	DDV_MaxChars(pDX, m_GraphName20, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME3, m_GraphName3);
	DDV_MaxChars(pDX, m_GraphName3, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME4, m_GraphName4);
	DDV_MaxChars(pDX, m_GraphName4, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME5, m_GraphName5);
	DDV_MaxChars(pDX, m_GraphName5, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME6, m_GraphName6);
	DDV_MaxChars(pDX, m_GraphName6, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME7, m_GraphName7);
	DDV_MaxChars(pDX, m_GraphName7, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME8, m_GraphName8);
	DDV_MaxChars(pDX, m_GraphName8, 300);
	DDX_Text(pDX, IDC_GREDIT_GRAPHNAME9, m_GraphName9);
	DDV_MaxChars(pDX, m_GraphName9, 300);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG1, m_OutGrFlag_1);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG10, m_OutGrFlag_10);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG11, m_OutGrFlag_11);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG12, m_OutGrFlag_12);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG13, m_OutGrFlag_13);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG15, m_OutGrFlag_15);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG14, m_OutGrFlag_14);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG16, m_OutGrFlag_16);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG17, m_OutGrFlag_17);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG18, m_OutGrFlag_18);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG19, m_OutGrFlag_19);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG2, m_OutGrFlag_2);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG20, m_OutGrFlag_20);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG3, m_OutGrFlag_3);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG4, m_OutGrFlag_4);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG5, m_OutGrFlag_5);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG6, m_OutGrFlag_6);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG7, m_OutGrFlag_7);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG8, m_OutGrFlag_8);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG9, m_OutGrFlag_9);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME1, m_SFileName_1);
	DDV_MaxChars(pDX, m_SFileName_1, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME10, m_SFileName_10);
	DDV_MaxChars(pDX, m_SFileName_10, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME11, m_SFileName_11);
	DDV_MaxChars(pDX, m_SFileName_11, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME12, m_SFileName_12);
	DDV_MaxChars(pDX, m_SFileName_12, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME13, m_SFileName_13);
	DDV_MaxChars(pDX, m_SFileName_13, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME14, m_SFileName_14);
	DDV_MaxChars(pDX, m_SFileName_14, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME15, m_SFileName_15);
	DDV_MaxChars(pDX, m_SFileName_15, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME16, m_SFileName_16);
	DDV_MaxChars(pDX, m_SFileName_16, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME17, m_SFileName_17);
	DDV_MaxChars(pDX, m_SFileName_17, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME18, m_SFileName_18);
	DDV_MaxChars(pDX, m_SFileName_18, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME19, m_SFileName_19);
	DDV_MaxChars(pDX, m_SFileName_19, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME2, m_SFileName_2);
	DDV_MaxChars(pDX, m_SFileName_2, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME20, m_SFileName_20);
	DDV_MaxChars(pDX, m_SFileName_20, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME3, m_SFileName_3);
	DDV_MaxChars(pDX, m_SFileName_3, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME4, m_SFileName_4);
	DDV_MaxChars(pDX, m_SFileName_4, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME5, m_SFileName_5);
	DDV_MaxChars(pDX, m_SFileName_5, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME6, m_SFileName_6);
	DDV_MaxChars(pDX, m_SFileName_6, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME7, m_SFileName_7);
	DDV_MaxChars(pDX, m_SFileName_7, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME8, m_SFileName_8);
	DDV_MaxChars(pDX, m_SFileName_8, 300);
	DDX_Text(pDX, IDC_GREDIT_SFILENAME9, m_SFileName_9);
	DDV_MaxChars(pDX, m_SFileName_9, 300);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG21, m_NormGrFlag1);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG22, m_NormGrFlag2);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG23, m_NormGrFlag3);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG24, m_NormGrFlag4);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG25, m_NormGrFlag5);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG26, m_NormGrFlag6);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG27, m_NormGrFlag7);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG28, m_NormGrFlag8);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG29, m_NormGrFlag9);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG30, m_NormGrFlag10);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG31, m_NormGrFlag11);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG32, m_NormGrFlag12);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG33, m_NormGrFlag13);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG34, m_NormGrFlag14);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG35, m_NormGrFlag15);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG36, m_NormGrFlag16);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG37, m_NormGrFlag17);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG38, m_NormGrFlag18);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG39, m_NormGrFlag19);
	DDX_Check(pDX, IDC_GREDIT_OUTFLAG40, m_NormGrFlag20);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogDoc, CDialog)
	//{{AFX_MSG_MAP(CDialogDoc)
	ON_BN_CLICKED(IDC_ADD_CHARACTERISTICS, OnAddCharacteristics)
	ON_BN_CLICKED(IDC_ADD_MAKEDEVIATION, OnAddMakedeviation)
	ON_BN_CLICKED(IDC_ADD_MAKEDEVIATION2, OnAddMakedeviation2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogDoc message handlers

void CDialogDoc::OnAddCharacteristics() 
{
	// TODO: Add your control notification handler code here
	m_AddCharacteristicsFlag = TRUE;
	CDialog::OnOK();
	return;
}


//DEL int CDialogDoc::DoModal() 
//DEL {
//DEL 	// TODO: Add your specialized code here and/or call the base class
//DEL 	
//DEL 	return CDialog::DoModal();
//DEL }

//DEL BOOL CDialogDoc::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
//DEL {
//DEL 	// TODO: Add your specialized code here and/or call the base class
//DEL 	
//DEL 	return CDialog::OnNotify(wParam, lParam, pResult);
//DEL }

void CDialogDoc::OnAddMakedeviation() 
{
	// TODO: Add your control notification handler code here
	cDeviation dlg;
//	char ch[1000];

//	GetCurrentDirectory( 1000, ch );

	dlg.m_dAmpDev  = 10.;
	dlg.m_dFazaDev = 10.;
	dlg.m_dXdev = 0.0;
	dlg.m_dYdev = 0.0;
	
	dlg.DoModal();

	return;
}

void CDialogDoc::OnAddMakedeviation2() 
{
	// TODO: Add your control notification handler code here

	CDeviatDirPat dlg;

	dlg.DoModal();

	return;
}
