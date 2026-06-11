// DialogTtoS.cpp : implementation file
//

#include "stdafx.h"
#include "TMCGROUT.h"
#include <tmcgrviw.h>
#include "DialogTtoS.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogTtoS dialog


CDialogTtoS::CDialogTtoS(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogTtoS::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogTtoS)
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
	m_SFileName = _T("");
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
	m_dTmax = 0.0;
	m_dTmin = 0.0;
	m_csTimeUnit = _T("");
	m_dXValue1 = 0.0;
	m_dXValue2 = 0.0;
	m_dXValue3 = 0.0;
	m_dXValue4 = 0.0;
	m_dXValue5 = 0.0;
	m_dXValue6 = 0.0;
	m_dXValue7 = 0.0;
	m_dXValue8 = 0.0;
	m_dXValue9 = 0.0;
	m_dXValue10 = 0.0;
	m_dXValue11 = 0.0;
	m_dXValue12 = 0.0;
	m_dXValue13 = 0.0;
	m_dXValue14 = 0.0;
	m_dXValue15 = 0.0;
	m_dXValue16 = 0.0;
	m_dXValue17 = 0.0;
	m_dXValue18 = 0.0;
	m_dXValue19 = 0.0;
	m_dXValue20 = 0.0;
	m_dXmin = 1.e+10;
	m_dDx = 1.e+8;
	m_csError = _T("");
	//}}AFX_DATA_INIT
}


void CDialogTtoS::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogTtoS)
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
	DDX_Text(pDX, IDC_GREDIT_SFILENAME, m_SFileName);
	DDV_MaxChars(pDX, m_SFileName, 300);
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
	DDX_Text(pDX, IDC_GREDIT_TMAX, m_dTmax);
	DDX_Text(pDX, IDC_GREDIT_TMIN, m_dTmin);
	DDX_Text(pDX, IDC_GREDIT_TUNIT, m_csTimeUnit);
	DDV_MaxChars(pDX, m_csTimeUnit, 300);
	DDX_Text(pDX, IDC_GREDIT_XVALUE1, m_dXValue1);
	DDX_Text(pDX, IDC_GREDIT_XVALUE2, m_dXValue2);
	DDX_Text(pDX, IDC_GREDIT_XVALUE3, m_dXValue3);
	DDX_Text(pDX, IDC_GREDIT_XVALUE4, m_dXValue4);
	DDX_Text(pDX, IDC_GREDIT_XVALUE5, m_dXValue5);
	DDX_Text(pDX, IDC_GREDIT_XVALUE6, m_dXValue6);
	DDX_Text(pDX, IDC_GREDIT_XVALUE7, m_dXValue7);
	DDX_Text(pDX, IDC_GREDIT_XVALUE8, m_dXValue8);
	DDX_Text(pDX, IDC_GREDIT_XVALUE9, m_dXValue9);
	DDX_Text(pDX, IDC_GREDIT_XVALUE10, m_dXValue10);
	DDX_Text(pDX, IDC_GREDIT_XVALUE11, m_dXValue11);
	DDX_Text(pDX, IDC_GREDIT_XVALUE12, m_dXValue12);
	DDX_Text(pDX, IDC_GREDIT_XVALUE13, m_dXValue13);
	DDX_Text(pDX, IDC_GREDIT_XVALUE14, m_dXValue14);
	DDX_Text(pDX, IDC_GREDIT_XVALUE15, m_dXValue15);
	DDX_Text(pDX, IDC_GREDIT_XVALUE16, m_dXValue16);
	DDX_Text(pDX, IDC_GREDIT_XVALUE17, m_dXValue17);
	DDX_Text(pDX, IDC_GREDIT_XVALUE18, m_dXValue18);
	DDX_Text(pDX, IDC_GREDIT_XVALUE19, m_dXValue19);
	DDX_Text(pDX, IDC_GREDIT_XVALUE20, m_dXValue20);
	DDX_Text(pDX, IDC_GREDIT_XMIN, m_dXmin);
	DDX_Text(pDX, IDC_GREDIT_DX, m_dDx);
	DDX_Text(pDX, IDC_GREDIT_TTOSERROR, m_csError);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogTtoS, CDialog)
	//{{AFX_MSG_MAP(CDialogTtoS)
	ON_BN_CLICKED(IDC_CHANGE_SFILENAME, OnChangeSfilename)
	ON_BN_CLICKED(IDC_ADD_CHARACTERISTICS, OnAddCharacteristics)
	ON_BN_CLICKED(IDC_EXPORT_CHARACTERISTICS, OnExportCharacteristics)
	ON_BN_CLICKED(IDC_EXPORT_FILLXVALUE, OnExportFillxvalue)
	ON_BN_CLICKED(IDC_CHANGE_SFILEDELET, OnChangeSfiledelet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogTtoS message handlers

INT_PTR CDialogTtoS::DoModal()
{
	// TODO: Add your specialized code here and/or call the base class

	char ch[TMC_GROUT_MAXSTRING_BUF];
	CString csBuf;
	int i;
	strcpy( ch, m_DocFileName );

	for( i = strlen(ch); i > 0; i-- )
	{
		if( ch[i] == '.' )
		{
			ch[i] = '\0';
			i = 0;
		};
	};
	m_SFileName.Format( "%s.s", ch );
	
	for( i = strlen(ch); i > 0; i-- )
	{
		if( ch[i] == '\\' )
		{
			ch[i] = '\0';
			i = 0;
		};
	};
	if(  m_SFileName_1.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_1);
		m_SFileName_1 = csBuf;
	};
	if(  m_SFileName_2.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_2);
		m_SFileName_2 = csBuf;
	};
	if(  m_SFileName_3.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_3);
		m_SFileName_3 = csBuf;
	};
	if(  m_SFileName_4.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_4);
		m_SFileName_4 = csBuf;
	};
	if(  m_SFileName_5.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_5);
		m_SFileName_5 = csBuf;
	};
	if(  m_SFileName_6.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_6);
		m_SFileName_6 = csBuf;
	};
	if(  m_SFileName_7.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_7);
		m_SFileName_7 = csBuf;
	};
	if(  m_SFileName_8.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_8);
		m_SFileName_8 = csBuf;
	};
	if(  m_SFileName_9.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_9);
		m_SFileName_9 = csBuf;
	};
	if(  m_SFileName_10.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_10);
		m_SFileName_10 = csBuf;
	};
	if(  m_SFileName_11.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_11);
		m_SFileName_11 = csBuf;
	};
	if(  m_SFileName_12.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_12);
		m_SFileName_12 = csBuf;
	};
	if(  m_SFileName_13.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_13);
		m_SFileName_13 = csBuf;
	};
	if(  m_SFileName_14.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_14);
		m_SFileName_14 = csBuf;
	};
	if(  m_SFileName_15.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_15);
		m_SFileName_15 = csBuf;
	};
	if(  m_SFileName_16.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_16);
		m_SFileName_16 = csBuf;
	};
	if(  m_SFileName_17.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_17);
		m_SFileName_17 = csBuf;
	};
	if(  m_SFileName_18.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_18);
		m_SFileName_18 = csBuf;
	};
	if(  m_SFileName_19.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_19);
		m_SFileName_19 = csBuf;
	};
	if(  m_SFileName_20.GetLength() != 0 ) 
	{
		csBuf.Format("%s\\%s", ch, m_SFileName_20);
		m_SFileName_20 = csBuf;
	};
	
	return CDialog::DoModal();
}

void CDialogTtoS::OnChangeSfilename() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	char ch[TMC_GROUT_MAXSTRING_BUF];
	CString csBuf;
	int i;
	strcpy( ch, m_DocFileName );

	for( i = strlen(ch); i > 0; i-- )
	{
		if( ch[i] == '\\' )
		{
			ch[i] = '\0';
			i = 0;
		};
	};
	csBuf.Format( "%s\\*.s", ch );

	CFileDialog dlg( TRUE, "", csBuf, OFN_ENABLESIZING, "Tamic S-matrix");
	dlg.m_ofn.lpstrTitle = "Change *.s file name";
//	strcpy( dlg.m_ofn.lpstrFile, m_SFileName );
	if( dlg.DoModal() == IDOK )
	{
		m_SFileName.Format( "%s", dlg.GetPathName() );
		UpdateData(FALSE);
	};
	return;
}

void CDialogTtoS::OnAddCharacteristics() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	char ch[TMC_GROUT_MAXSTRING_BUF];
	CString csBuf;
	int i;
	strcpy( ch, m_DocFileName );

	for( i = strlen(ch); i > 0; i-- )
	{
		if( ch[i] == '\\' )
		{
			ch[i] = '\0';
			i = 0;
		};
	};
	csBuf.Format( "%s\\*.t", ch );

	CFileDialog dlg( TRUE, "", csBuf, OFN_ENABLESIZING, "Tamic Rt-Signal");
	dlg.m_ofn.lpstrTitle = "Add *.t characteristics";
	if( dlg.DoModal() == IDOK )
	{
		csBuf.Format( "%s", dlg.GetPathName() );
		if(  m_SFileName_1.GetLength() == 0 ) 
		{
			m_SFileName_1 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_2.GetLength() == 0 ) 
		{
			m_SFileName_2 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_3.GetLength() == 0 ) 
		{
			m_SFileName_3 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_4.GetLength() == 0 ) 
		{
			m_SFileName_4 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_5.GetLength() == 0 ) 
		{
			m_SFileName_5 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_6.GetLength() == 0 ) 
		{
			m_SFileName_6 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_7.GetLength() == 0 ) 
		{
			m_SFileName_7 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_8.GetLength() == 0 ) 
		{
			m_SFileName_8 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_9.GetLength() == 0 ) 
		{
			m_SFileName_9 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_10.GetLength() == 0 ) 
		{
			m_SFileName_10 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_11.GetLength() == 0 ) 
		{
			m_SFileName_11 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_12.GetLength() == 0 ) 
		{
			m_SFileName_12 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_13.GetLength() == 0 ) 
		{
			m_SFileName_13 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_14.GetLength() == 0 ) 
		{
			m_SFileName_14 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_15.GetLength() == 0 ) 
		{
			m_SFileName_15 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_16.GetLength() == 0 ) 
		{
			m_SFileName_16 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_17.GetLength() == 0 ) 
		{
			m_SFileName_17 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_18.GetLength() == 0 ) 
		{
			m_SFileName_18 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_19.GetLength() == 0 ) 
		{
			m_SFileName_19 = csBuf;
			UpdateData(FALSE);
			return;
		};
		if(  m_SFileName_20.GetLength() == 0 ) 
		{
			m_SFileName_20 = csBuf;
			UpdateData(FALSE);
			return;
		};
		UpdateData(FALSE);
	};

	return;
}

void CDialogTtoS::OnExportFillxvalue() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	
	double dx = m_dXmin;
	
	m_dXValue1 = dx;
	dx += m_dDx;
	m_dXValue2 = dx;
	dx += m_dDx;
	m_dXValue3 = dx;
	dx += m_dDx;
	m_dXValue4 = dx;
	dx += m_dDx;
	m_dXValue5 = dx;
	dx += m_dDx;
	m_dXValue6 = dx;
	dx += m_dDx;
	m_dXValue7 = dx;
	dx += m_dDx;
	m_dXValue8 = dx;
	dx += m_dDx;
	m_dXValue9 = dx;
	dx += m_dDx;
	m_dXValue10 = dx;
	dx += m_dDx;
	m_dXValue11 = dx;
	dx += m_dDx;
	m_dXValue12 = dx;
	dx += m_dDx;
	m_dXValue13 = dx;
	dx += m_dDx;
	m_dXValue14 = dx;
	dx += m_dDx;
	m_dXValue15 = dx;
	dx += m_dDx;
	m_dXValue16 = dx;
	dx += m_dDx;
	m_dXValue17 = dx;
	dx += m_dDx;
	m_dXValue18 = dx;
	dx += m_dDx;
	m_dXValue19 = dx;
	dx += m_dDx;
	m_dXValue20 = dx;
	dx += m_dDx;

	UpdateData(FALSE);
	return;
}

void CDialogTtoS::OnExportCharacteristics() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	CTmcTtoS cTtoS;

	if( (m_OutGrFlag_1)&&(m_SFileName_1.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_1, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue1 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_2)&&(m_SFileName_2.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_2, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue2 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_3)&&(m_SFileName_3.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_3, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue3 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_4)&&(m_SFileName_4.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_4, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue4 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_5)&&(m_SFileName_5.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_5, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue5 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_6)&&(m_SFileName_6.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_6, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue6 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_7)&&(m_SFileName_7.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_7, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue7 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_8)&&(m_SFileName_8.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_8, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue8 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_9)&&(m_SFileName_9.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_9, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue9 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_10)&&(m_SFileName_10.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_10, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue10 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_11)&&(m_SFileName_11.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_11, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue11 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_12)&&(m_SFileName_12.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_12, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue12 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_13)&&(m_SFileName_13.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_13, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue13 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_14)&&(m_SFileName_14.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_14, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue14 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_15)&&(m_SFileName_15.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_15, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue15 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_16)&&(m_SFileName_16.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_16, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue16 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_17)&&(m_SFileName_17.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_17, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue17 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_18)&&(m_SFileName_18.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_18, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue18 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_19)&&(m_SFileName_19.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_19, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue19 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	if( (m_OutGrFlag_20)&&(m_SFileName_20.GetLength() != 0) )
	{
		cTtoS.TtoS( m_SFileName_20, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue20 );
		m_csError = cTtoS.GetErrorMessage();
		UpdateData(FALSE);
		if( cTtoS.IsError() ) return;
	};

	return;
}


void CDialogTtoS::OnOK() 
{
	// TODO: Add extra validation here

	OnExportCharacteristics();
	
	CDialog::OnOK();
}

void CDialogTtoS::OnChangeSfiledelet() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	
	remove(m_SFileName);

	UpdateData(FALSE);
	
	return;
}
