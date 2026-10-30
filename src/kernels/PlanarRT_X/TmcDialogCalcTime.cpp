// TmcDialogCalcTime.cpp : window "Calculation time" (TIMING)
//

/// @file TmcDialogCalcTime.cpp
/// @brief Окно «Calculation time»: показ, обновление и экспорт отчёта о времени расчёта.
///
/// Часть модуля «Оболочка счётного ядра». Реализация `CTmcDialogCalcTime`.
/// В ядре X код файла совпадает с одноимённым файлом ядра H (`src/kernels/PlanarRT_H`).
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#x-differences
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#statistics
#include "StdAfx.h"
#include "PlanRT_H.h"
#include "TmcDialogCalcTime.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CTmcDialogCalcTime::CTmcDialogCalcTime(CWnd* pParent /*=NULL*/)
	: CDialog(CTmcDialogCalcTime::IDD, pParent)
{
	m_pcTiming = NULL;
}

BEGIN_MESSAGE_MAP(CTmcDialogCalcTime, CDialog)
	ON_BN_CLICKED(IDC_TMCCALCTIMEREFRESH, OnRefresh)
	ON_BN_CLICKED(IDC_TMCCALCTIMEEXPORT, OnExport)
END_MESSAGE_MAP()

BOOL CTmcDialogCalcTime::OnInitDialog()
{
	CDialog::OnInitDialog();
	// fixed-width font: the report is a table
	m_cFont.CreatePointFont( 90, "Courier New" );
	CWnd *pEdit = GetDlgItem( IDC_TMCCALCTIMETEXT );
	if( pEdit != NULL ) pEdit->SetFont( &m_cFont );
	ShowReport();
	// no selection of the whole text, focus on "Close"
	if( pEdit != NULL ) pEdit->SendMessage( EM_SETSEL, 0, 0 );
	CWnd *pOk = GetDlgItem( IDOK );
	if( pOk != NULL ) pOk->SetFocus();
	return FALSE;
}

void CTmcDialogCalcTime::ShowReport( void )
{
	CString csR;
	if( m_pcTiming == NULL ) csR = "No data";
	else csR = m_pcTiming->BuildReport();
	SetDlgItemText( IDC_TMCCALCTIMETEXT, csR );
	return;
}

void CTmcDialogCalcTime::OnRefresh()
{
	ShowReport();
	return;
}

void CTmcDialogCalcTime::OnExport()
{
	if( m_pcTiming == NULL ) return;
	CFileDialog cFileDialog( FALSE, "txt", m_pcTiming->DefaultReportName(),
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,
		"Text files (*.txt)|*.txt|All files (*.*)|*.*||", this );
	if( cFileDialog.DoModal() != IDOK ) return;
	CString csFile = cFileDialog.GetPathName();
	CString csBuf;
	if( m_pcTiming->SaveReport( csFile ) ) csBuf.Format( "Report saved:\n%s", (LPCTSTR)csFile );
	else csBuf.Format( "Can't write file:\n%s", (LPCTSTR)csFile );
	AfxMessageBox( csBuf );
	return;
}
