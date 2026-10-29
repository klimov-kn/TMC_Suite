// PlanRT_HView.cpp : implementation of the CPlanRT_HView class
//

#include "stdafx.h"
#include "MainFrm.h"
#include <tmcgrviw.h>
#include "PlanRT_H.h"
#include <limits.h>
#include <math.h>

#include "PlanRT_HDoc.h"
#include "PlanRT_HView.h"
#include "TmcDialogStatistics.h"
#include "TmcRTH_DialogFormatOutFile.h"
#include "Pl_iofor.h"
#ifdef _WIN64
#include <TmcOcl.h>	// GPU: the list of the video cards for Config -> Compute on GPU
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HView

IMPLEMENT_DYNCREATE(CPlanRT_HView, CScrollView)

BEGIN_MESSAGE_MAP(CPlanRT_HView, CScrollView)
	//{{AFX_MSG_MAP(CPlanRT_HView)
	ON_COMMAND(ID_EDIT_EDIT, OnEditEdit)
	ON_COMMAND(ID_CONFIG_EDITOR, OnConfigEditor)
	ON_COMMAND(ID_CONFIG_COLOR_BACKGROUND, OnConfigColorBackGround)
	ON_COMMAND(ID_CONFIG_FORMAT_OUTPUTDATAFILE, OnConfigFormatOutputDataFile)
	ON_COMMAND(ID_RUN_SKIPSTEP, OnRunSkipstep)
	ON_COMMAND(ID_RUN_BACKSTEP, OnRunBackstep)
	ON_COMMAND(ID_RUN_RESTARTALL, OnRunRestartall)
	ON_COMMAND(ID_RUN_RESTARTSTEP, OnRunRestartstep)
	ON_COMMAND(ID_RUN_STARTSTEP, OnRunStartstep)
	ON_COMMAND(ID_VIEW_STATISTICS, OnViewStatistics)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_RUN_STOP, OnRunStop)
	ON_COMMAND(ID_RUN_RUN, OnRunAll)
	ON_WM_DESTROY()
	ON_COMMAND(ID_CONFIG_VIEWER_OUTPUTSIGNAL, OnConfigViewerOutputsignal)
	ON_COMMAND(ID_VIEW_OUTPUT, OnViewOutput)
	ON_COMMAND(ID_VIEW_FIELD, OnViewField)
	ON_COMMAND(ID_VIEW_FIELD_1, OnViewField1)
	ON_COMMAND(ID_CONFIG_VIEWER_FIELD, OnConfigViewerField)
	ON_COMMAND(ID_CONFIG_SINCHRONIZATION, OnConfigSinchronization)
	ON_COMMAND(ID_CONFIG_SOUND, OnConfigSound)
	ON_COMMAND(ID_CONFIG_SOUND_MELODY, OnConfigSoundMelody)
	ON_COMMAND(ID_VIEW_TOPOLOGY, OnViewTopology)
	ON_UPDATE_COMMAND_UI(ID_CONFIG_SINCHRONIZATION, OnUpdateConfigSinchronization)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIELD, OnUpdateViewField)
	ON_UPDATE_COMMAND_UI(ID_CONFIG_SOUND, OnUpdateConfigSound)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOPOLOGY, OnUpdateViewTopology)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIELD_1, OnUpdateViewField1)
	ON_UPDATE_COMMAND_UI(ID_RUN_RUN, OnUpdateRunRun)
	ON_UPDATE_COMMAND_UI(ID_RUN_RESTARTALL, OnUpdateRunRestartall)
	ON_UPDATE_COMMAND_UI(ID_RUN_STARTSTEP, OnUpdateRunStartstep)
	ON_UPDATE_COMMAND_UI(ID_RUN_RESTARTSTEP, OnUpdateRunRestartstep)
	ON_UPDATE_COMMAND_UI(ID_RUN_SKIPSTEP, OnUpdateRunSkipstep)
	ON_UPDATE_COMMAND_UI(ID_RUN_BACKSTEP, OnUpdateRunBackstep)
	ON_UPDATE_COMMAND_UI(ID_RUN_STOP, OnUpdateRunStop)
	ON_COMMAND(ID_CONFIG_SMATRIX, OnConfigSmatrix)
	ON_COMMAND(ID_CONFIG_VIEWER_SMATRIX, OnConfigViewerSmatrix)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, OnUpdateFileClose)
	ON_COMMAND(ID_CONFIG_AUTORUN, OnConfigAutorun)
	ON_UPDATE_COMMAND_UI(ID_CONFIG_AUTORUN, OnUpdateConfigAutorun)
	ON_COMMAND(ID_CONFIG_GPU, OnConfigGpu)					// GPU
	ON_UPDATE_COMMAND_UI(ID_CONFIG_GPU, OnUpdateConfigGpu)	// GPU
	ON_COMMAND(ID_CONFIG_SETUP, OnConfigSetup)
	ON_COMMAND(ID_VIEW_DIRECTIONALPATTERN, OnViewDirectionalpattern)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DIRECTIONALPATTERN, OnUpdateViewDirectionalpattern)
	ON_COMMAND(ID_CONFIG_VIEWER_DIRECTIONALPATTERN, OnConfigViewerDirectionalpattern)
	ON_COMMAND(ID_CONFIG_DIRECTIONALPATTERN, OnConfigDirectionalpattern)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HView construction/destruction

CPlanRT_HView::CPlanRT_HView():cSoundDialog("Sound Effects")
{
	// TODO: add construction code here
//	cSoundDialog("Sound Effects");
	bIsReadData = FALSE;
	scBackgoundColor= RGB(   0, 0, 0); // backgound color;
	return;

}

CPlanRT_HView::~CPlanRT_HView()
{
	bIsReadData = FALSE;
	SetPointerClassView( NULL );
	return;
}

BOOL CPlanRT_HView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HView drawing

void CPlanRT_HView::OnDraw(CDC* pDC)
{
	OnDrawBackground( pDC );
	// TODO: add draw code for native data here
}

void CPlanRT_HView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	CSize sizeTotal( (int)(TMC_VIEW_XSIZE), (int)(TMC_VIEW_YSIZE) );
	CSize sizePage(sizeTotal.cx, sizeTotal.cy);
	CSize sizeLine(sizeTotal.cx, sizeTotal.cy);
	
	SetScrollSizes(TMC_VIEW_SIZEMODE, sizeTotal, sizePage, sizeLine);
	
	// TODO: calculate the total size of this view



	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	CString csBuffer;
	int i;
	

	OnConfigSetup();

// read extern editor name
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_EXTERNEDITORNAME_INI );
	if( csBuffer.GetLength() != 0 )	pDoc->SetExternEditorName( csBuffer );
// read extern viewer name
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_EXTERNVIEWERNAME_INI );
	if( csBuffer.GetLength() != 0 )	pDoc->SetExternViewer( csBuffer );
// read extern field viewer name
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_EXTERNFIELDVIEWERNAME_INI );
	if( csBuffer.GetLength() != 0 )	pDoc->SetExternViewerField( csBuffer );
// read extern directional pattern viewer name
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_EXTERNDIRPATVIEWERNAM_INI );
	if( csBuffer.GetLength() != 0 )	pDoc->SetExternViewerDirectionalpattern( csBuffer );
// read extern S-matrix viewer name
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_EXTERNSMVIEWERNAME_INI );
	if( csBuffer.GetLength() != 0 )	pDoc->SetExternViewerSmatrix( csBuffer );
// read color background
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_BACKGROUNDCOLOR_INI );
	if( csBuffer.GetLength() != 0 )
	{
		i = atoi(csBuffer);
		scBackgoundColor = i;
	};
// read format for write data in output file
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFORMATNT_INI );
	if( csBuffer.GetLength() != 0 )	SetnTFormat( csBuffer );
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFORMATDT_INI );
	if( csBuffer.GetLength() != 0 )	SetdTcurrentFormat( csBuffer );
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFORMATNB_INI );
	if( csBuffer.GetLength() != 0 )	SetnBFormat( csBuffer );
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFORMATINP_INI );
	if( csBuffer.GetLength() != 0 )	SetdInpFormat( csBuffer );
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFORMATOUT_INI );
	if( csBuffer.GetLength() != 0 )	SetdOutFormat( csBuffer );
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFORMATOUTFILED_INI );
	if( csBuffer.GetLength() != 0 )	SetdFieldOutFormat( csBuffer );
// read SinchronizationFlag
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFIELDSINCHRFLAG_INI );
	if( csBuffer.GetLength() != 0 )	
	{
		sscanf( csBuffer, "%d", &i);
		pDoc->SetOutSinchrFieldOutFlag( i );
	};
// read Field Output Flag
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFIELDFLAG_INI );
	if( csBuffer.GetLength() != 0 )	
	{
		sscanf( csBuffer, "%d", &i);
		pDoc->SetOutFieldOutFlag( i );
	};
// read SoundEffectFlag and Sound Effects melody
	csBuffer = AfxGetApp()->GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTPUTSOUNDFLAG_INI );
	if( csBuffer.GetLength() != 0 )	
	{
		int pnNote1[12], pnNote2[12], i1, i2, i3;
		if( sscanf( csBuffer, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
								&i1, &i2, &i3,
								&(pnNote1[0]),&(pnNote1[1]),&(pnNote1[2]),&(pnNote1[3]),&(pnNote1[4]),&(pnNote1[5]),&(pnNote1[6]),&(pnNote1[7]),&(pnNote1[8]),&(pnNote1[9]),&(pnNote1[10]),&(pnNote1[11]),
								&(pnNote2[0]),&(pnNote2[1]),&(pnNote2[2]),&(pnNote2[3]),&(pnNote2[4]),&(pnNote2[5]),&(pnNote2[6]),&(pnNote2[7]),&(pnNote2[8]),&(pnNote2[9]),&(pnNote2[10]),&(pnNote2[11]) ) == 27 )
		{
			pDoc->SetSoundFlag( i1 );
			SetMelody1Interval( i2 );
			SetMelody2Interval( i3 );
			SetMelody1( pnNote1 );
			SetMelody2( pnNote2 );
		};
	};
//	read system information and set type OS for sound effects
	ReadSystemType();

	ReadData();

	if( IsDemoRun() ) 
	{
		OnConfigSetup();
		WriteFile();
		pDoc->SetOutFieldOutFlag( TRUE );
	};

	if( IsAutoRun() )
	{
			OnRunAll();
			if( IsDemoRun() ) 
			{
				SetPointerClassView( this );
				pDoc->SetOutSinchrFieldOutFlag( TRUE );
				Sleep(1000);
			};
	};


	WriteFile();
	
	return;
}

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HView printing

BOOL CPlanRT_HView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CPlanRT_HView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CPlanRT_HView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HView diagnostics

#ifdef _DEBUG
void CPlanRT_HView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CPlanRT_HView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CPlanRT_HDoc* CPlanRT_HView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPlanRT_HDoc)));
	return (CPlanRT_HDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPlanRT_HView message handlers

void CPlanRT_HView::OnEditEdit() 
{
	// TODO: Add your command handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CString szEditorName;
	szEditorName = pDoc->GetExternEditorName();
	if( !pDoc->RunExeFile( szEditorName ) ) 
	{
		CString csBuf1;
		csBuf1.Format("Please enter a name of the external editor");
		AfxMessageBox( csBuf1 );
		OnConfigEditor();
	};
	return;
}

void CPlanRT_HView::OnConfigEditor() 
{
	// TODO: Add your command handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CFileDialog	cFileDialogEditor(TRUE);
	
	if( cFileDialogEditor.DoModal() == 1 )
	{
		pDoc->SetExternEditorName( cFileDialogEditor.GetPathName() );
		WriteFile();
	};

	return;	
}

void CPlanRT_HView::WriteFile( void )
{
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	char szBuffer[N_BUF_MAX];
// save extern editor name
	strcpy( szBuffer, pDoc->GetExternEditorName() );
	AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_EXTERNEDITORNAME_INI, szBuffer);
// save extern viewer name
	strcpy( szBuffer, pDoc->GetExternViewer() );
	AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_EXTERNVIEWERNAME_INI, szBuffer);
// save extern field viewer name
	strcpy( szBuffer, pDoc->GetExternViewerField() );
	AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_EXTERNFIELDVIEWERNAME_INI, szBuffer);
// save extern directional pattern viewer name
	strcpy( szBuffer, pDoc->GetExternViewerDirectPat() );
	AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_EXTERNDIRPATVIEWERNAM_INI, szBuffer);
// save extern S-matrix viewer name
	strcpy( szBuffer, pDoc->GetExternViewerSmatrix() );
	AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_EXTERNSMVIEWERNAME_INI, szBuffer);
// save color background
	sprintf( szBuffer, "%d", scBackgoundColor);
	AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_BACKGROUNDCOLOR_INI, szBuffer);
// save SinchronizationFlag
	sprintf( szBuffer, "%d", pDoc->GetOutput()->bOutFieldSinchronization);
	AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFIELDSINCHRFLAG_INI, szBuffer);
// save Field Output Flag
	sprintf( szBuffer, "%d", pDoc->GetOutput()->bOutField);
	AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFIELDFLAG_INI, szBuffer);
// read format for write data in output file
	strcpy( szBuffer, GetnTFormat() );
	AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFORMATNT_INI, szBuffer);
	strcpy( szBuffer, GetdTcurrentFormat() );
	AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFORMATDT_INI, szBuffer);
	strcpy( szBuffer, GetnBFormat() );
	AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFORMATNB_INI, szBuffer);
	strcpy( szBuffer, GetdInpFormat() );
	AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFORMATINP_INI, szBuffer);
	strcpy( szBuffer, GetdOutFormat() );
	AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFORMATOUT_INI, szBuffer);
	strcpy( szBuffer, GetdFieldOutFormat() );
	AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTFORMATOUTFILED_INI, szBuffer);
// save SoundEffectFlag and Sound Effects melody
	{
		int pnNote1[12], pnNote2[12];
		GetMelody1( pnNote1 );
		GetMelody2( pnNote2 );
		sprintf( szBuffer, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
								pDoc->GetOutput()->bSoundEffect,
								GetMelody1Interval(), GetMelody2Interval(),
								pnNote1[0],pnNote1[1],pnNote1[2],pnNote1[3],pnNote1[4],pnNote1[5],pnNote1[6],pnNote1[7],pnNote1[8],pnNote1[9],pnNote1[10],pnNote1[11],
								pnNote2[0],pnNote2[1],pnNote2[2],pnNote2[3],pnNote2[4],pnNote2[5],pnNote2[6],pnNote2[7],pnNote2[8],pnNote2[9],pnNote2[10],pnNote2[11]);
		AfxGetApp()->WriteProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_OUTPUTSOUNDFLAG_INI, szBuffer);
	}

	return;
}

void CPlanRT_HView::OnRunSkipstep() 
{
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	if( pDoc->IsRun() ) return;
	
	OnRunStop();
	pDoc->SkipStep();
	ReadData();

	return;
}

void CPlanRT_HView::OnRunBackstep() 
{
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	if( pDoc->IsRun() ) return;
	
	OnRunStop();
	pDoc->BackStep();
	ReadData();

	return;
}

void CPlanRT_HView::OnRunRestartall() 
{
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	OnRunStop();
	Sleep(1000);
	pDoc->SetFirstStep();
	ReadData();

	return;
}

void CPlanRT_HView::ReadData( void )
{
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CString csBuf;

	if(	pDoc->IsReadData() ) return;
	if(	pDoc->IsRun() ) return;
	if(	bIsReadData ) return;
	
	bIsReadData = TRUE;
	
	pDoc->ReadData();

	csBuf.Format( "Step:%d;%s", pDoc->GetnStep(), pDoc->GetErrorMessage() );
	PutTrace( csBuf );
	bIsReadData = FALSE;
	return;
}

void CPlanRT_HView::OnRunRestartstep() 
{
	OnRunStop();
	ReadData();
	return;
}



void CPlanRT_HView::OnRunStartstep() 
{
	// TODO: Add your command handler code here
	if(	bIsReadData ) return;
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(	pDoc->IsReadData() ) return;
	if(	pDoc->IsRun() ) return;
	CString csBuf;

	bIsReadData = TRUE;

	
	CWinThread* pThread = AfxBeginThread( RunStepGlobal, (void *)(pDoc), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED );
	if( pThread == NULL )
	{
		PutTrace( "Can't open thread for read data" );
		bIsReadData = FALSE;
		return;
	};
	pDoc->SetRunThreadHandle( pThread->m_hThread );
	pThread->ResumeThread();

	bIsReadData = FALSE;
	return;
}

void CPlanRT_HView::OnRunAll() 
{
	// TODO: Add your command handler code here
	if(	bIsReadData ) return;
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(	pDoc->IsReadData() ) return;
	CString csBuf;

	bIsReadData = TRUE;

	
	CWinThread* pThread = AfxBeginThread( RunAllGlobal, (void *)(pDoc), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED );
	if( pThread == NULL )
	{
		PutTrace( "Can't open thread for read data" );
		bIsReadData = FALSE;	// J-2: as in OnRunStep (old: not reset - every next Run was ignored)
		return;
	};
	pDoc->SetRunThreadHandle( pThread->m_hThread );
	pThread->ResumeThread();

	csBuf.Format( "Step:%d;%s", pDoc->GetnStep(), pDoc->GetErrorMessage() );
	PutTrace( csBuf );
	bIsReadData = FALSE;
	return;
}

void CPlanRT_HView::OnViewStatistics() 
{
	CString csBuf;
	// TODO: Add your command handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CTmcDialogStatistics cDlg;

	if( pDoc->IsRun() )	// N-9: the dialog writes into the data of the running step
	{
		AfxMessageBox( "Statistics is shown when the calculation is stopped or finished." );
		return;
	};

	cDlg.m_nStep = pDoc->GetnStep();
	cDlg.m_csErrorMessage = pDoc->GetErrorMessage();
	cDlg.m_cParam = pDoc->GetParam();
	cDlg.m_cTopol = pDoc->GetTopology();
	cDlg.m_cOut = pDoc->GetOutput();
	cDlg.m_pcTiming = &(pDoc->GetRectNode().GetTiming());	// TIMING

	cDlg.PrepareData();

	if( cDlg.DoModal() != 1 ) return;

	if( cDlg.nFlagStep == -1 ) 
	{
		pDoc->BackStep();
		pDoc->ReadData();
		OnViewStatistics();
		csBuf.Format( "Step:%d;%s", pDoc->GetnStep(), pDoc->GetErrorMessage() );
		PutTrace( csBuf );
		return;
	};

	if( cDlg.nFlagStep == 1 ) 
	{
		pDoc->SkipStep();
		pDoc->ReadData();
		OnViewStatistics();
		csBuf.Format( "Step:%d;%s", pDoc->GetnStep(), pDoc->GetErrorMessage() );
		PutTrace( csBuf );
		return;
	};

	return;
}

void CPlanRT_HView::OnFileSaveAs() 
{
	// TODO: Add your command handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CFileDialog dlg( FALSE, "tpl", "*.tpl");

	if( dlg.DoModal() == IDOK )
	{
		FILE *fIn, *fOut;
		int ch;

		if( ( fIn = fopen( pDoc->GetPathName(), "r") ) == NULL )
		{
			return;
		};
		if( ( fOut = fopen( dlg.GetPathName(), "w") ) == NULL )
		{
			fclose( fIn );
			return;
		};
		
		while( (ch = fgetc( fIn )) != EOF )
		{
			if( fputc( ch, fOut) == EOF )
			{
				fclose( fOut );
				fclose( fIn );
				return;
			};
		};
		
		fclose( fOut );
		fclose( fIn );
		pDoc->SetPathName( dlg.GetPathName() );
	};
	return;
}

void CPlanRT_HView::OnRunStop()
{
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pDoc->Stop();
	bIsReadData = FALSE;
	
	return;
}

void CPlanRT_HView::OnDestroy() 
{
	CScrollView::OnDestroy();	
	// TODO: Add your message handler code here	
}

void CPlanRT_HView::OnDrawBackground(CDC* pDC)
{
	CRect rectClient;
	GetClientRect( &rectClient );
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&rectClient);  
	rectClient.NormalizeRect();
	pDC->FillSolidRect( &rectClient, scBackgoundColor );
	return;
}

void CPlanRT_HView::OnConfigColorBackGround() 
{
	// TODO: Add your command handler code here
	CColorDialog dlg;
	
	dlg.m_cc.rgbResult = scBackgoundColor;
	if( dlg.DoModal() == IDOK )
	{
		scBackgoundColor = dlg.GetColor();
		WriteFile();
		RedrawWindow();	
	};

	return;	
}


void CPlanRT_HView::OnConfigFormatOutputDataFile() 
{
	CTmcRTH_DialogFormatOutFile cDlg;

	cDlg.m_csnT.Format( "%s", GetnTFormat() );
	cDlg.m_csdTcurrent.Format( "%s", GetdTcurrentFormat() );
	cDlg.m_csnBlock.Format( "%s", GetnBFormat() );
	cDlg.m_csdInp.Format( "%s", GetdInpFormat() );
	cDlg.m_csdOut.Format( "%s", GetdOutFormat() );
	cDlg.m_csdOutField.Format( "%s", GetdFieldOutFormat() );

	if( cDlg.DoModal() == 1 ) 
	{
		SetnTFormat( cDlg.m_csnT );
		SetdTcurrentFormat( cDlg.m_csdTcurrent );
		SetnBFormat( cDlg.m_csnBlock );
		SetdInpFormat( cDlg.m_csdInp );
		SetdOutFormat( cDlg.m_csdOut );
		SetdFieldOutFormat( cDlg.m_csdOutField );
		if( ( strcmp( cDlg.m_csnT, GetnTFormat() ) != 0 )||( strcmp( cDlg.m_csdTcurrent, GetdTcurrentFormat() ) != 0 )||	// T-4
			( strcmp( cDlg.m_csnBlock, GetnBFormat() ) != 0 )||( strcmp( cDlg.m_csdInp, GetdInpFormat() ) != 0 )||
			( strcmp( cDlg.m_csdOut, GetdOutFormat() ) != 0 )||( strcmp( cDlg.m_csdOutField, GetdFieldOutFormat() ) != 0 ) )
			AfxMessageBox( "A format is not valid and is replaced by the default.\nA format must have one conversion: d (i, u, x, o) for a number, g (e, f) for a value, width and precision of at most 2 digits." );
		WriteFile();
		if( cDlg.IsSetDefaultFormat )
		{
			SetDefaultOutputFormat();
			OnConfigFormatOutputDataFile();
		};
		return;
	}


	return;
}

void CPlanRT_HView::OnConfigViewerOutputsignal() 
{
	// TODO: Add your command handler code here	
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CFileDialog	cFileDialog( TRUE, "exe", "*.exe", OFN_ENABLESIZING, "Output Signal Viewer (TMCROS.exe)");
	
	if( cFileDialog.DoModal() == 1 )
	{
		pDoc->SetExternViewer( cFileDialog.GetPathName() );
		WriteFile();
	};

	return;	
}

void CPlanRT_HView::OnViewOutput() 
{
	// TODO: Add your command handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if( !pDoc->RunViewer() ) 
	{
		CString csBuf1;
		csBuf1.Format("Please enter a name of the external output signal viewer");
		AfxMessageBox( csBuf1 );
		OnConfigViewerOutputsignal();
	};
	return;	
}

void CPlanRT_HView::OnViewField() 
{
	// TODO: Add your command handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->OnOffFieldOutput();
	WriteFile();
	return;	
}

void CPlanRT_HView::OnViewField1() 
{
	// TODO: Add your command handler code here	
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if( !pDoc->RunViewerField() ) 
	{
		CString csBuf1;
		csBuf1.Format("Please enter a name of the external field viewer");
		AfxMessageBox( csBuf1 );
		OnConfigViewerField();
	};
	return;	
}

void CPlanRT_HView::OnConfigViewerField() 
{
	// TODO: Add your command handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CFileDialog	cFileDialog( TRUE, "exe", "*.exe", OFN_ENABLESIZING, "Field Viewer (FldView.exe)");
	
	if( cFileDialog.DoModal() == 1 )
	{
		pDoc->SetExternViewerField( cFileDialog.GetPathName() );
		WriteFile();
	};

	return;	
}

void CPlanRT_HView::OnConfigSinchronization() 
{
	// TODO: Add your command handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->OnOffSinchronizationFieldOutput();
	WriteFile();
	return;	
}

void CPlanRT_HView::OnConfigSound() 
{
	// TODO: Add your command handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->OnOffSoundEffects();
	WriteFile();
	return;	
}

void CPlanRT_HView::OnConfigSoundMelody() 
{
	// TODO: Add your command handler code here
	
	int pnNote1[12], pnNote2[12];
	
	cSoundDialog.cMelody1.m_SoundInterval = GetMelody1Interval();
	GetMelody1( pnNote1 );
	cSoundDialog.cMelody1.m_Note1  = pnNote1[ 0];
	cSoundDialog.cMelody1.m_Note2  = pnNote1[ 1];
	cSoundDialog.cMelody1.m_Note3  = pnNote1[ 2];
	cSoundDialog.cMelody1.m_Note4  = pnNote1[ 3];
	cSoundDialog.cMelody1.m_Note5  = pnNote1[ 4];
	cSoundDialog.cMelody1.m_Note6  = pnNote1[ 5];
	cSoundDialog.cMelody1.m_Note7  = pnNote1[ 6];
	cSoundDialog.cMelody1.m_Note8  = pnNote1[ 7];
	cSoundDialog.cMelody1.m_Note9  = pnNote1[ 8];
	cSoundDialog.cMelody1.m_Note10 = pnNote1[ 9];
	cSoundDialog.cMelody1.m_Note11 = pnNote1[10];
	cSoundDialog.cMelody1.m_Note12 = pnNote1[11];
	cSoundDialog.cMelody2.m_SoundInterval = GetMelody2Interval();
	GetMelody2( pnNote2 );
	cSoundDialog.cMelody2.m_Note1  = pnNote2[ 0];
	cSoundDialog.cMelody2.m_Note2  = pnNote2[ 1];
	cSoundDialog.cMelody2.m_Note3  = pnNote2[ 2];
	cSoundDialog.cMelody2.m_Note4  = pnNote2[ 3];
	cSoundDialog.cMelody2.m_Note5  = pnNote2[ 4];
	cSoundDialog.cMelody2.m_Note6  = pnNote2[ 5];
	cSoundDialog.cMelody2.m_Note7  = pnNote2[ 6];
	cSoundDialog.cMelody2.m_Note8  = pnNote2[ 7];
	cSoundDialog.cMelody2.m_Note9  = pnNote2[ 8];
	cSoundDialog.cMelody2.m_Note10 = pnNote2[ 9];
	cSoundDialog.cMelody2.m_Note11 = pnNote2[10];
	cSoundDialog.cMelody2.m_Note12 = pnNote2[11];

	if( cSoundDialog.DoModal() == IDOK )
	{
		pnNote2[ 0] = cSoundDialog.cMelody2.m_Note1;
		pnNote2[ 1] = cSoundDialog.cMelody2.m_Note2;
		pnNote2[ 2] = cSoundDialog.cMelody2.m_Note3;
		pnNote2[ 3] = cSoundDialog.cMelody2.m_Note4;
		pnNote2[ 4] = cSoundDialog.cMelody2.m_Note5;
		pnNote2[ 5] = cSoundDialog.cMelody2.m_Note6;
		pnNote2[ 6] = cSoundDialog.cMelody2.m_Note7;
		pnNote2[ 7] = cSoundDialog.cMelody2.m_Note8;
		pnNote2[ 8] = cSoundDialog.cMelody2.m_Note9;
		pnNote2[ 9] = cSoundDialog.cMelody2.m_Note10;
		pnNote2[10] = cSoundDialog.cMelody2.m_Note11;
		pnNote2[11] = cSoundDialog.cMelody2.m_Note12;
		SetMelody2( pnNote2 );
		SetMelody2Interval( cSoundDialog.cMelody2.m_SoundInterval );
		pnNote1[ 0] = cSoundDialog.cMelody1.m_Note1;
		pnNote1[ 1] = cSoundDialog.cMelody1.m_Note2;
		pnNote1[ 2] = cSoundDialog.cMelody1.m_Note3;
		pnNote1[ 3] = cSoundDialog.cMelody1.m_Note4;
		pnNote1[ 4] = cSoundDialog.cMelody1.m_Note5;
		pnNote1[ 5] = cSoundDialog.cMelody1.m_Note6;
		pnNote1[ 6] = cSoundDialog.cMelody1.m_Note7;
		pnNote1[ 7] = cSoundDialog.cMelody1.m_Note8;
		pnNote1[ 8] = cSoundDialog.cMelody1.m_Note9;
		pnNote1[ 9] = cSoundDialog.cMelody1.m_Note10;
		pnNote1[10] = cSoundDialog.cMelody1.m_Note11;
		pnNote1[11] = cSoundDialog.cMelody1.m_Note12;
		SetMelody1( pnNote1 );
		SetMelody1Interval( cSoundDialog.cMelody1.m_SoundInterval );
		WriteFile();
	};

	return;	
}

BOOL CPlanRT_HView::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	OnRunStop();

	return CScrollView::DestroyWindow();
}

void CPlanRT_HView::OnViewTopology() 
{
	// TODO: Add your command handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if( pDoc->IsRun() )	// N-9: the arrays of the running step are not read from the window
	{
		AfxMessageBox( "The topology file is written when the calculation is stopped or finished." );
		return;
	};
	pDoc->PutTopology();
	WriteFile();
	return;
}

void CPlanRT_HView::OnUpdateConfigSinchronization(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->SetCheck( pDoc->GetSinchronizationFieldOutput() );
	pCmdUI->Enable(pDoc->GetFieldFlag());
	return;
}

void CPlanRT_HView::OnUpdateViewField(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->SetCheck( pDoc->GetFieldFlag() );
	return;
}

void CPlanRT_HView::OnUpdateConfigSound(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->SetCheck( pDoc->GetSoundEffect() );
	return;
}

void CPlanRT_HView::OnUpdateViewTopology(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->SetCheck( pDoc->GetTopologyFlag() );
	return;
}

void CPlanRT_HView::OnUpdateViewField1(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->Enable(pDoc->GetFieldFlag()&&pDoc->GetTopologyFlag());
	return;
}

void CPlanRT_HView::OnUpdateRunRun(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->Enable( (!pDoc->IsReadData())&&(!pDoc->IsRun()) );
	return;
}

void CPlanRT_HView::OnUpdateRunRestartall(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->Enable( (!pDoc->IsReadData())&&(!pDoc->IsRun()) );
	return;
}

void CPlanRT_HView::OnUpdateRunStartstep(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->Enable( (!pDoc->IsReadData())&&(!pDoc->IsRun()) );
	return;
}

void CPlanRT_HView::OnUpdateRunRestartstep(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->Enable( (!pDoc->IsReadData())&&(!pDoc->IsRun()) );
	return;
}

void CPlanRT_HView::OnUpdateRunSkipstep(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->Enable( (!pDoc->IsReadData())&&(!pDoc->IsRun()) );
	return;
}

void CPlanRT_HView::OnUpdateRunBackstep(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->Enable( (!pDoc->IsReadData())&&(!pDoc->IsRun()) );
	return;
}

void CPlanRT_HView::OnUpdateRunStop(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->Enable( (pDoc->IsReadData())||(pDoc->IsRun()) );
	return;
}



void CPlanRT_HView::OnConfigSmatrix() 
{
	// TODO: Add your command handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if( !pDoc->RunViewerSmatrix() ) 
	{
		CString csBuf1;
		csBuf1.Format("Please enter a name of the external S-matrix viewer");
		AfxMessageBox( csBuf1 );
		OnConfigViewerSmatrix();
	};
	return;	
}

void CPlanRT_HView::OnConfigViewerSmatrix() 
{
	// TODO: Add your command handler code here
	// set s-matrix viewer name
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CFileDialog	cFileDialog( TRUE, "exe", "*.exe", OFN_ENABLESIZING, "S-matrix Viewer (tmcgrout.exe)");
	
	if( cFileDialog.DoModal() == 1 )
	{
		pDoc->SetExternViewerSmatrix( cFileDialog.GetPathName() );
		WriteFile();
	};

	return;	
}

void CPlanRT_HView::OnUpdateFileClose(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->Enable( (!pDoc->IsReadData())&&(!pDoc->IsRun()) );
	return;
}


void CPlanRT_HView::OnConfigAutorun() 
{
	// TODO: Add your command handler code here
	SwitchAutoRunFlag();
	return;
}

void CPlanRT_HView::OnUpdateConfigAutorun(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( IsAutoRun() );
	return;
}

#ifdef _WIN64
// GPU: the video card the kernel will take (the same list and order as TmcGpuH.h: only cards with double);
// FALSE - no card, csText says why
static BOOL TmcGpuCardText( CString &csText )
{
	TmcOcl ocl;
	if( !ocl.Load() )
	{
		csText = "OpenCL.dll was not found in the system folder - install the driver of the video card.\n"
			"The calculation stays on the CPU.";
		return FALSE;
	};
	std::vector<TmcOclDevice> v = TmcOclListGpus( ocl ), vd;
	for( size_t k = 0; k < v.size(); k++ ) if( v[k].bFP64 ) vd.push_back( v[k] );
	FreeLibrary( ocl.hLib );
	int nDev = GetGpuSettingCard();
	if( vd.empty() )
	{
		csText = v.empty() ? "OpenCL has no video card (no OpenCL driver of a video card is installed).\n"
			: "No video card with double precision (cl_khr_fp64) - the calculation needs it.\n";
		csText += "The calculation stays on the CPU.";
		return FALSE;
	};
	if( nDev >= (int)vd.size() )
	{
		csText.Format( "TMC_GPU asks for the card %d, but there are %d card(s) with double precision, numbered from 0.\n"
			"The calculation stays on the CPU.", nDev, (int)vd.size() );
		return FALSE;
	};
	csText.Format( "The calculation goes to the video card GPU %d: %s\n(%s, driver %s, memory %I64u MB).\n\n"
		"Small grids stay on the CPU - it is faster there. The status line shows where a step is calculated.\n"
		"The keys /GPU, /GPU:n and /CPU of the command line are stronger than this switch.",
		nDev, vd[nDev].sName.c_str(), vd[nDev].sPlatform.c_str(), vd[nDev].sDriver.c_str(),
		(unsigned __int64)( vd[nDev].nGlobalMem/( 1024*1024 ) ) );
	return TRUE;
}
#endif

void CPlanRT_HView::OnConfigGpu()	// GPU: Config -> Compute on GPU
{
	BOOL bOn = !IsGpuSettingOn();
#ifdef _WIN64
	if( bOn )
	{
		CString csText;
		if( !TmcGpuCardText( csText ) )
		{
			AfxMessageBox( csText, MB_ICONWARNING );	// no card: the switch stays off
			return;
		};	// the card is found: only the check mark (the status line shows where a step is calculated)
	};
#else
	return;	// win32: OpenCL calculation needs the win64 build
#endif
	SetGpuSetting( bOn );
	AfxGetApp()->WriteProfileInt( PLANRT_H_RAZDEL_INI, PLANRT_H_COMPUTEGPU_INI, bOn ? 1 : 0 );
	return;
}

void CPlanRT_HView::OnUpdateConfigGpu(CCmdUI* pCmdUI)
{
#ifdef _WIN64
	CPlanRT_HDoc* pDoc = GetDocument();
	// the command line decides (/GPU, /CPU); during a run the switch is not changed
	pCmdUI->Enable( !IsGpuKeyInCommandLine() && !( ( pDoc != NULL )&&pDoc->IsRun() ) );
#else
	pCmdUI->Enable( FALSE );
#endif
	pCmdUI->SetCheck( IsGpuSettingOn() );
	return;
}

void CPlanRT_HView::OnConfigSetup() 
{

	// TODO: Add your command handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CString csBuffer, csBuffer1;
	char szBuffer[N_BUF_MAX] = "";
	char szBuffer1[N_BUF_MAX] = "";

	char szCurrentDir[N_BUF_MAX];
	int i;
	strcpy( szCurrentDir, pDoc->GetPathName() );
	for( i = strlen(szCurrentDir)-1; i >= 0; i-- )
	{
		if( szCurrentDir[i] == '\\' )
		{
			szCurrentDir[i] = '\0';
			i = -1;
		};
	};

// read extern editor name
	strcpy( szBuffer1, pDoc->GetExternEditorName() );	// S-12: the editor, not the signal viewer
	GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_EXTERNEDITORNAME_INI, szBuffer1, szBuffer, N_BUF_MAX);
	csBuffer = szBuffer;
	if( csBuffer.GetLength() != 0 )
	{
		pDoc->SetExternEditorName( csBuffer );
	}
	else
	{
		csBuffer = "NOTEPAD.EXE";	// T-33: started from the system folder (CreatProc)
		pDoc->SetExternEditorName( csBuffer );
	};

// read extern viewer name
	strcpy( szBuffer1, pDoc->GetExternViewer() );
	GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_EXTERNVIEWERNAME_INI, szBuffer1, szBuffer, N_BUF_MAX);
	csBuffer = szBuffer;
	if( csBuffer.GetLength() != 0 )
	{
		pDoc->SetExternViewer( csBuffer );
	}
	else
	{
		csBuffer = "TMCROS.EXE";	// S-12: the program of the package, found next to the kernel (not "task folder" + name)
		pDoc->SetExternViewer( csBuffer );
	};

// read extern field viewer name
	strcpy( szBuffer1, pDoc->GetExternViewerField() );
	GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_EXTERNFIELDVIEWERNAME_INI, szBuffer1, szBuffer, N_BUF_MAX);
	csBuffer = szBuffer;
	if( csBuffer.GetLength() != 0 )	
	{
		pDoc->SetExternViewerField( csBuffer );
	}
	else
	{
		csBuffer = "FLDVIEW.EXE";	// S-12: the program of the package, found next to the kernel (not "task folder" + name)
		pDoc->SetExternViewerField( csBuffer );
	};


// read extern S-matrix viewer name
	strcpy( szBuffer1, pDoc->GetExternViewerSmatrix() );
	GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_EXTERNSMVIEWERNAME_INI, szBuffer1, szBuffer, N_BUF_MAX);
	csBuffer = szBuffer;
	if( csBuffer.GetLength() != 0 )	
	{
		pDoc->SetExternViewerSmatrix( csBuffer );
	}
	else
	{
		csBuffer = "TMCGROUT.EXE";	// S-12: the program of the package, found next to the kernel (not "task folder" + name)
		pDoc->SetExternViewerSmatrix( csBuffer );
	};


// read extern directional pattern viewer name
	strcpy( szBuffer1, pDoc->GetExternViewerDirectPat() );	// S-12: the directional pattern viewer, not the S-matrix one
	GetProfileString( PLANRT_H_RAZDEL_INI, PLANRT_H_EXTERNDIRPATVIEWERNAM_INI, szBuffer1, szBuffer, N_BUF_MAX);
	csBuffer = szBuffer;
	if( csBuffer.GetLength() != 0 )	
	{
		pDoc->SetExternViewerDirectionalpattern( csBuffer );
	}
	else
	{
		csBuffer = "TMC_DN.EXE";	// S-12: the program of the package, found next to the kernel (not "task folder" + name)
		pDoc->SetExternViewerDirectionalpattern( csBuffer );
	};

	return;
}


void CPlanRT_HView::OnViewField11()
{
	static BOOL bIsRunViewField = FALSE;
	if( bIsRunViewField ) return;
	bIsRunViewField = TRUE;
	OnViewField1();
	return;
}

void CPlanRT_HView::OnViewDirectionalpattern() 
{
	// TODO: Add your command handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if( !pDoc->RunViewerDirectPattern() ) 
	{
		CString csBuf1;
		csBuf1.Format("Please enter a name of the external directional pattern viewer");
		AfxMessageBox( csBuf1 );
		OnConfigViewerDirectionalpattern();
	};
	return;	
}

void CPlanRT_HView::OnUpdateViewDirectionalpattern(CCmdUI* pCmdUI) 
{
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->Enable( ((pDoc->GetOutput())->GetFieldIntegrated()).bIsFieldDistr() );
	return;
}

void CPlanRT_HView::OnConfigViewerDirectionalpattern() 
{
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CFileDialog	cFileDialog( TRUE, "exe", "*.exe", OFN_ENABLESIZING, "Directional Pattern Viewer (TMC_DN.exe)");
	
	if( cFileDialog.DoModal() == 1 )
	{
		pDoc->SetExternViewerDirectionalpattern( cFileDialog.GetPathName() );
		WriteFile();
	};

	return;	
}

void CPlanRT_HView::OnConfigDirectionalpattern() 
{
	// TODO: Add your command handler code here
	// TODO: Add your command handler code here
	CPlanRT_HDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->ExportToDirectionalPatter();
	return;	
}
