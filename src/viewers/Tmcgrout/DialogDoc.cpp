// DialogDoc.cpp : implementation file
//

#include "stdafx.h"
#include "TMCGROUT.h"
#include "DialogDoc.h"

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
	m_np1_10 = 1;
	m_np1_11 = 1;
	m_np1_12 = 1;
	m_np1_13 = 1;
	m_np1_14 = 1;
	m_np1_15 = 1;
	m_np1_16 = 1;
	m_np1_17 = 1;
	m_np1_18 = 1;
	m_np1_19 = 1;
	m_np1_2 = 1;
	m_np1_20 = 1;
	m_np1_1 = 1;
	m_np1_3 = 1;
	m_np1_4 = 1;
	m_np1_5 = 1;
	m_np1_6 = 1;
	m_np1_7 = 1;
	m_np1_8 = 1;
	m_np1_9 = 1;
	m_np2_1 = 1;
	m_np2_10 = 1;
	m_np2_11 = 1;
	m_np2_12 = 1;
	m_np2_13 = 1;
	m_np2_14 = 1;
	m_np2_15 = 1;
	m_np2_16 = 1;
	m_np2_17 = 1;
	m_np2_18 = 1;
	m_np2_19 = 1;
	m_np2_2 = 1;
	m_np2_20 = 1;
	m_np2_3 = 1;
	m_np2_4 = 1;
	m_np2_5 = 1;
	m_np2_6 = 1;
	m_np2_7 = 1;
	m_np2_8 = 1;
	m_np2_9 = 1;
	m_mod1_1 = 1;
	m_mod1_10 = 1;
	m_mod1_11 = 1;
	m_mod1_12 = 1;
	m_mod1_13 = 1;
	m_mod1_14 = 1;
	m_mod1_15 = 1;
	m_mod1_16 = 1;
	m_mod1_17 = 1;
	m_mod1_18 = 1;
	m_mod1_19 = 1;
	m_mod1_2 = 1;
	m_mod1_20 = 1;
	m_mod1_3 = 1;
	m_mod1_4 = 1;
	m_mod1_5 = 1;
	m_mod1_6 = 1;
	m_mod1_7 = 1;
	m_mod1_8 = 1;
	m_mod1_9 = 1;
	m_mod2_1 = 1;
	m_mod2_10 = 1;
	m_mod2_11 = 1;
	m_mod2_12 = 1;
	m_mod2_13 = 1;
	m_mod2_14 = 1;
	m_mod2_15 = 1;
	m_mod2_16 = 1;
	m_mod2_17 = 1;
	m_mod2_18 = 1;
	m_mod2_19 = 1;
	m_mod2_2 = 1;
	m_mod2_20 = 1;
	m_mod2_3 = 1;
	m_mod2_4 = 1;
	m_mod2_5 = 1;
	m_mod2_6 = 1;
	m_mod2_7 = 1;
	m_mod2_8 = 1;
	m_mod2_9 = 1;
	m_OutGrFlag_1 = TRUE;
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
	m_OutGrFlag_2 = TRUE;
	m_OutGrFlag_20 = TRUE;
	m_OutGrFlag_3 = TRUE;
	m_OutGrFlag_4 = TRUE;
	m_OutGrFlag_5 = TRUE;
	m_OutGrFlag_6 = TRUE;
	m_OutGrFlag_7 = TRUE;
	m_OutGrFlag_8 = TRUE;
	m_OutGrFlag_9 = TRUE;
	m_SFileName_1 = _T("");
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
	m_SFileName_2 = _T("");
	m_SFileName_20 = _T("");
	m_SFileName_3 = _T("");
	m_SFileName_4 = _T("");
	m_SFileName_5 = _T("");
	m_SFileName_6 = _T("");
	m_SFileName_7 = _T("");
	m_SFileName_8 = _T("");
	m_SFileName_9 = _T("");
	m_AddCharacteristicsFlag = FALSE;
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
	DDX_Text(pDX, IDC_GREDIT_INP1_10, m_np1_10);
	DDV_MinMaxInt(pDX, m_np1_10, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_11, m_np1_11);
	DDV_MinMaxInt(pDX, m_np1_11, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_12, m_np1_12);
	DDV_MinMaxInt(pDX, m_np1_12, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_13, m_np1_13);
	DDV_MinMaxInt(pDX, m_np1_13, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_14, m_np1_14);
	DDV_MinMaxInt(pDX, m_np1_14, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_15, m_np1_15);
	DDV_MinMaxInt(pDX, m_np1_15, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_16, m_np1_16);
	DDV_MinMaxInt(pDX, m_np1_16, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_17, m_np1_17);
	DDV_MinMaxInt(pDX, m_np1_17, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_18, m_np1_18);
	DDV_MinMaxInt(pDX, m_np1_18, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_19, m_np1_19);
	DDV_MinMaxInt(pDX, m_np1_19, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_2, m_np1_2);
	DDV_MinMaxInt(pDX, m_np1_2, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_20, m_np1_20);
	DDV_MinMaxInt(pDX, m_np1_20, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_1, m_np1_1);
	DDV_MinMaxInt(pDX, m_np1_1, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_3, m_np1_3);
	DDV_MinMaxInt(pDX, m_np1_3, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_4, m_np1_4);
	DDV_MinMaxInt(pDX, m_np1_4, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_5, m_np1_5);
	DDV_MinMaxInt(pDX, m_np1_5, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_6, m_np1_6);
	DDV_MinMaxInt(pDX, m_np1_6, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_7, m_np1_7);
	DDV_MinMaxInt(pDX, m_np1_7, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_8, m_np1_8);
	DDV_MinMaxInt(pDX, m_np1_8, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP1_9, m_np1_9);
	DDV_MinMaxInt(pDX, m_np1_9, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_1, m_np2_1);
	DDV_MinMaxInt(pDX, m_np2_1, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_10, m_np2_10);
	DDV_MinMaxInt(pDX, m_np2_10, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_11, m_np2_11);
	DDV_MinMaxInt(pDX, m_np2_11, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_12, m_np2_12);
	DDV_MinMaxInt(pDX, m_np2_12, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_13, m_np2_13);
	DDV_MinMaxInt(pDX, m_np2_13, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_14, m_np2_14);
	DDV_MinMaxInt(pDX, m_np2_14, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_15, m_np2_15);
	DDV_MinMaxInt(pDX, m_np2_15, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_16, m_np2_16);
	DDV_MinMaxInt(pDX, m_np2_16, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_17, m_np2_17);
	DDV_MinMaxInt(pDX, m_np2_17, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_18, m_np2_18);
	DDV_MinMaxInt(pDX, m_np2_18, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_19, m_np2_19);
	DDV_MinMaxInt(pDX, m_np2_19, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_2, m_np2_2);
	DDV_MinMaxInt(pDX, m_np2_2, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_20, m_np2_20);
	DDV_MinMaxInt(pDX, m_np2_20, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_3, m_np2_3);
	DDV_MinMaxInt(pDX, m_np2_3, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_4, m_np2_4);
	DDV_MinMaxInt(pDX, m_np2_4, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_5, m_np2_5);
	DDV_MinMaxInt(pDX, m_np2_5, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_6, m_np2_6);
	DDV_MinMaxInt(pDX, m_np2_6, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_7, m_np2_7);
	DDV_MinMaxInt(pDX, m_np2_7, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_8, m_np2_8);
	DDV_MinMaxInt(pDX, m_np2_8, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_INP2_9, m_np2_9);
	DDV_MinMaxInt(pDX, m_np2_9, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_1, m_mod1_1);
	DDV_MinMaxInt(pDX, m_mod1_1, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_10, m_mod1_10);
	DDV_MinMaxInt(pDX, m_mod1_10, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_11, m_mod1_11);
	DDV_MinMaxInt(pDX, m_mod1_11, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_12, m_mod1_12);
	DDV_MinMaxInt(pDX, m_mod1_12, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_13, m_mod1_13);
	DDV_MinMaxInt(pDX, m_mod1_13, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_14, m_mod1_14);
	DDV_MinMaxInt(pDX, m_mod1_14, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_15, m_mod1_15);
	DDV_MinMaxInt(pDX, m_mod1_15, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_16, m_mod1_16);
	DDV_MinMaxInt(pDX, m_mod1_16, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_17, m_mod1_17);
	DDV_MinMaxInt(pDX, m_mod1_17, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_18, m_mod1_18);
	DDX_Text(pDX, IDC_GREDIT_MOD1_19, m_mod1_19);
	DDV_MinMaxInt(pDX, m_mod1_19, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_2, m_mod1_2);
	DDV_MinMaxInt(pDX, m_mod1_2, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_20, m_mod1_20);
	DDV_MinMaxInt(pDX, m_mod1_20, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_3, m_mod1_3);
	DDV_MinMaxInt(pDX, m_mod1_3, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_4, m_mod1_4);
	DDV_MinMaxInt(pDX, m_mod1_4, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_5, m_mod1_5);
	DDV_MinMaxInt(pDX, m_mod1_5, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_6, m_mod1_6);
	DDV_MinMaxInt(pDX, m_mod1_6, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_7, m_mod1_7);
	DDV_MinMaxInt(pDX, m_mod1_7, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_8, m_mod1_8);
	DDV_MinMaxInt(pDX, m_mod1_8, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD1_9, m_mod1_9);
	DDV_MinMaxInt(pDX, m_mod1_9, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_1, m_mod2_1);
	DDV_MinMaxInt(pDX, m_mod2_1, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_10, m_mod2_10);
	DDV_MinMaxInt(pDX, m_mod2_10, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_11, m_mod2_11);
	DDV_MinMaxInt(pDX, m_mod2_11, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_12, m_mod2_12);
	DDV_MinMaxInt(pDX, m_mod2_12, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_13, m_mod2_13);
	DDV_MinMaxInt(pDX, m_mod2_13, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_14, m_mod2_14);
	DDV_MinMaxInt(pDX, m_mod2_14, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_15, m_mod2_15);
	DDV_MinMaxInt(pDX, m_mod2_15, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_16, m_mod2_16);
	DDV_MinMaxInt(pDX, m_mod2_16, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_17, m_mod2_17);
	DDV_MinMaxInt(pDX, m_mod2_17, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_18, m_mod2_18);
	DDV_MinMaxInt(pDX, m_mod2_18, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_19, m_mod2_19);
	DDV_MinMaxInt(pDX, m_mod2_19, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_2, m_mod2_2);
	DDV_MinMaxInt(pDX, m_mod2_2, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_20, m_mod2_20);
	DDV_MinMaxInt(pDX, m_mod2_20, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_3, m_mod2_3);
	DDV_MinMaxInt(pDX, m_mod2_3, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_4, m_mod2_4);
	DDV_MinMaxInt(pDX, m_mod2_4, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_5, m_mod2_5);
	DDV_MinMaxInt(pDX, m_mod2_5, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_6, m_mod2_6);
	DDV_MinMaxInt(pDX, m_mod2_6, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_7, m_mod2_7);
	DDV_MinMaxInt(pDX, m_mod2_7, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_8, m_mod2_8);
	DDV_MinMaxInt(pDX, m_mod2_8, 1, 10000);
	DDX_Text(pDX, IDC_GREDIT_MOD2_9, m_mod2_9);
	DDV_MinMaxInt(pDX, m_mod2_9, 1, 10000);
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
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogDoc, CDialog)
	//{{AFX_MSG_MAP(CDialogDoc)
	ON_BN_CLICKED(IDC_ADD_CHARACTERISTICS, OnAddCharacteristics)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogDoc message handlers

void CDialogDoc::OnAddCharacteristics() 
{
	// TODO: Add your control notification handler code here
	m_AddCharacteristicsFlag = TRUE;
	CDialog::OnOK();
}

