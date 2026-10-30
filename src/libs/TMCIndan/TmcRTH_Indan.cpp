// TmcRTH_Indan.cpp : implementation file
//

/// @file TmcRTH_Indan.cpp
/// @brief Чтение задания: запуск препроцессора, построчное чтение временного файла,
///        поиск шага #STEP и секций, проверки окна и частоты `SMATRIX`.
///
/// Часть библиотеки TMCIndan. Реализует `CTmcRTH_Indan` и `MainPrepr`.
/// Каждая секция ищется заново от начала временного файла препроцессора:
/// `FindStep` открывает файл и встаёт на нужный `#STEP`, `FindSection` — на метку
/// секции, `GetLineSection` отдаёт строки до метки конца секции.
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#reading

#include "STDAFX.H"
#include <TmcRTH_Indan.h>
#include <Prepr1.h>
#include <map>		// T-8
#include <set>
#include <string>
#ifndef _WIN32
#include <unistd.h>	// S-16: getpid
#endif
#include <TmcRTH_Indan.h>


/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_Indan

CTmcRTH_Indan::CTmcRTH_Indan()
{
	cError.Clear();
	nStep = 0;
	pfPrepr = NULL;
	csTplFileName.Format("");
	csTplFileNamePrepr.Format("");
	csCurrentPath.Format("");
	return;
}

CTmcRTH_Indan::~CTmcRTH_Indan()
{
	DeleteData();
}


//BEGIN_MESSAGE_MAP(CTmcRTH_Indan, CWnd)
	//{{AFX_MSG_MAP(CTmcRTH_Indan)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_Indan message handlers

void CTmcRTH_Indan::DeleteData( void )
{
	cOut.DeleteData();
	cParam.DeleteData();
	nStep = 0;
	cError.Clear();
	if( csTplFileNamePrepr.GetLength() != 0 ) remove(csTplFileNamePrepr);
	csTplFileName.Format("");
	csTplFileNamePrepr.Format("");
	csCurrentPath.Format("");
	ClosePreprFile();
	return;
}

void CTmcRTH_Indan::SetTplFileName( CString csFileName )
{
	FILE *pfTpl;
	CString csBuf;

//		{	CString csBuf1;
	DeleteData();
//		{	CString csBuf1;

	if( ( pfTpl = fopen( csFileName, "r") ) == NULL )
	{
#ifdef _WIN32
		if( csFileName.GetLength() >= 260 )	// T-7: say why
			csBuf.Format("can't open file %s: the path is %d characters long, at most 259 are supported", (LPCSTR)csFileName, csFileName.GetLength());
		else
#endif
		csBuf.Format("can't open file %s", csFileName);
		cError.PutErrorMessage(csBuf);
		return;
	};
	fclose(pfTpl);
	// Файл открывается: запоминаем путь, папку задания и имя временного файла препроцессора.
	csTplFileName = csFileName;
	SetCurrentPath();
	// S-16: the number of the process in the name - two runs of one .tpl do not share the file
#ifdef _WIN32
	csTplFileNamePrepr.Format("%s%s%lu", (LPCSTR)csTplFileName, CTMCRTH_INDANPREPRFNA, (unsigned long)GetCurrentProcessId());
#else
	csTplFileNamePrepr.Format("%s%s%lu", (LPCSTR)csTplFileName, CTMCRTH_INDANPREPRFNA, (unsigned long)getpid());
#endif
#ifdef _WIN32
	{
		char szFull[1024];
		DWORD dwFull = GetFullPathNameA( csTplFileNamePrepr, sizeof(szFull), szFull, NULL );
		if( ( dwFull == 0 )||( dwFull >= 260 ) )	// S-16: no room for the number - the old name, as before
		{
			csTplFileNamePrepr.Format("%s%s", (LPCSTR)csTplFileName, CTMCRTH_INDANPREPRFNA);
			dwFull = GetFullPathNameA( csTplFileNamePrepr, sizeof(szFull), szFull, NULL );
		};
		if( ( dwFull == 0 )||( dwFull >= 260 ) )	// T-7: the temporary file of the preprocessor must fit too
		{
			csBuf.Format("the path to the task is too long: the temporary file %s needs %lu characters, at most 259 are supported; move the task to a shorter folder",
				(LPCSTR)csTplFileNamePrepr, (unsigned long)dwFull);
			csTplFileNamePrepr.Format("");
			cError.PutErrorMessage(csBuf);
			return;
		};
	};
#endif
	MakePreprFile();
	return;
}

void CTmcRTH_Indan::SetTplFileName( char *pszFileName )
{
	CString csBuffer;
	csBuffer.Format("%s", pszFileName);
	SetTplFileName( csBuffer );
	return;
}

void CTmcRTH_Indan::SetCurrentPath( void )
{
	if( IsError() ) return;

	char szBuf[CTMCRTH_INDANSBUF];
	CString csBuf;
	int n, i;

	if( csTplFileName.GetLength() > CTMCRTH_INDANSBUF )
	{
		csBuf.Format("very long file name %s", csTplFileName);
		cError.PutErrorMessage(csBuf);
		return;
	};

	// Полный путь к заданию; папка — всё до последнего разделителя пути.
	DWORD dwFull = GetFullPathNameA( csTplFileName, CTMCRTH_INDANSBUF, szBuf, NULL );	// A-16
	if( ( dwFull == 0 )||( dwFull >= CTMCRTH_INDANSBUF ) ) strcpy( szBuf, csTplFileName );
	n = (int)(strlen(szBuf));
	if( n < 1 )
	{
		csCurrentPath.Format( "" );
		return;
	};
	for( i = n - 1; i > 1; i--)
	{
		if( TMC_IS_SEP(szBuf[i]))
		{
			szBuf[i] = '\0';
			csCurrentPath.Format( "%s", szBuf );
			return;
		};
	};

	csCurrentPath.Format( "" );
	return;
}

BOOL CTmcRTH_Indan::IsError( void )
{
	return cError.IsError();
}

CString CTmcRTH_Indan::GetErrorMessage( void )
{
	return cError.GetErrorMessage();
}

void CTmcRTH_Indan::MakePreprFile( void )
{
	if( IsError() ) return;


	PutTrace("Load prepr.");
//		CString csBuf1;
	MainPrepr( csTplFileNamePrepr, csTplFileName, csCurrentPath, cError );
	PutTrace("UnLoad prepr.");
//		CString csBuf1;
	return;
}

int CTmcRTH_Indan::GetNStepMax( CString csFileName )
{
	int i;
	SetTplFileName( csFileName );
	OpenPreprFile();
	i = CalcNStepMax();
	ClosePreprFile();
	return i;
}

int CTmcRTH_Indan::GetNStepMax( void )
{
	int i;
	OpenPreprFile();
	i = CalcNStepMax();
	ClosePreprFile();
	return i;
}

int CTmcRTH_Indan::CalcNStepMax( void )
{
	if( IsError() ) return 0;
	
	int i, ii;
	CString csCh;
	CString csBuf;

	// T-8: an .s file holds at most FREQ_MAX_NUM-1 = 99 frequencies; count them before the run
	std::map< std::string, std::set<std::string> > mSFreq;
	BOOL bInOutput = FALSE;

	// Один проход по файлу: i — число закрытых шагов, ii — 1 внутри #STEP ... #END_STEP.
	// Попутно внутри #OUTPUT собираются пары «файл SMATRIX -> множество частот»
	// (частота нормализуется через %.12g, к имени без точки дописывается .s).
	for( i = 0, ii = 0; GetLine( csCh ) != EOF;  )
	{
		if( IsError() ) return 0;
		if( strncmp( csCh, CTMCRTH_INDANMET_ENDOUTPUT, strlen(CTMCRTH_INDANMET_ENDOUTPUT) ) == 0 ) bInOutput = FALSE;
		else if( strncmp( csCh, CTMCRTH_INDANMET_OUTPUT, strlen(CTMCRTH_INDANMET_OUTPUT) ) == 0 ) bInOutput = TRUE;
		else if( bInOutput && ( strncmp( csCh, CTMCRTH_SMATRIXOUT_ID, strlen(CTMCRTH_SMATRIXOUT_ID) ) == 0 ) )
		{
			std::string sTok[2];
			const char *p = (const char *)csCh + strlen(CTMCRTH_SMATRIXOUT_ID);
			int nTok = 0, nDepth = 0;
			for( ; ( *p != '\0' )&&( nTok < 2 ); p++ )
			{
				if( ( *p == '(' )||( *p == '[' )||( *p == '{' ) ) nDepth++;
				if( ( *p == ')' )||( *p == ']' )||( *p == '}' ) ) nDepth--;
				if( ( *p == ';' )&&( nDepth == 0 ) ) { nTok++; continue; };
				if( ( *p == ' ' )||( *p == '\t' )||( *p == '\n' )||( *p == '\r' ) ) continue;
				sTok[nTok] += *p;
			};
			if( nTok == 2 )
			{
				char *pEnd;
				double dF = strtod( sTok[1].c_str(), &pEnd );
				if( ( pEnd != sTok[1].c_str() )&&( *pEnd == '\0' ) )
				{
					char szF[64];
					sprintf( szF, "%.12g", dF );
					sTok[1] = szF;
				};
				if( sTok[0].find( '.' ) == std::string::npos ) sTok[0] += ".s";
				mSFreq[ sTok[0] ].insert( sTok[1] );
			};
		};
		if( strncmp( csCh, CTMCRTH_INDANMET_STEP, strlen(CTMCRTH_INDANMET_STEP) ) == 0 )
		{
			switch( ii )
			{
				case  0:	
					ii = 1;
					break;
				case  1:	
					csBuf.Format("Missing %s", CTMCRTH_INDANMET_ENDSTEP);
					cError.PutErrorMessage( csBuf );
					return 0;
			};
		};
		if( strncmp( csCh, CTMCRTH_INDANMET_ENDSTEP, strlen(CTMCRTH_INDANMET_ENDSTEP) ) == 0 )
		{
			switch( ii )
			{
				case  0:	
					csBuf.Format("Missing %s", CTMCRTH_INDANMET_STEP);
					cError.PutErrorMessage( csBuf );
					return 0;
				case  1:	
					ii = 0;
					i++;
					break;
			};
		};
	};
	if( IsError() ) return 0;	// N-18: the error of GetLine is not replaced by "Missing #END_STEP"

	if( ii != 0 )
	{
		csBuf.Format("Missing %s", CTMCRTH_INDANMET_ENDSTEP);
		cError.PutErrorMessage( csBuf );
		return 0;
	};

	{	// T-8
		std::map< std::string, std::set<std::string> >::iterator it;
		for( it = mSFreq.begin(); it != mSFreq.end(); it++ )
		{
			if( (int)it->second.size() < 100 ) continue;
			csBuf.Format("%s %s: %d different frequencies in the task, at most 99 fit in one S-matrix file - split the sweep into several %s files",
				CTMCRTH_SMATRIXOUT_ID, it->first.c_str(), (int)it->second.size(), CTMCRTH_SMATRIXOUT_ID );
			cError.PutErrorMessage( csBuf );
			return 0;
		};
	};

	return i;
}

void CTmcRTH_Indan::OpenPreprFile( void )
{
	if( IsError() ) return;
	if( pfPrepr != NULL ) return;

	PutTrace("Load prepr.");

	CString csBuf;

	if( csTplFileNamePrepr.GetLength() == 0 )
	{
		csBuf.Format("You foget set tpl file name");
		cError.PutErrorMessage( csBuf );
		return;
	};

	if( ( pfPrepr = fopen( csTplFileNamePrepr, "r") ) == NULL )
	{
		csBuf.Format("Can't open prepr file %s", csTplFileNamePrepr);
		cError.PutErrorMessage( csBuf );
		return;
	};
	return;
}

void CTmcRTH_Indan::ClosePreprFile( void )
{
	if( pfPrepr != NULL ) fclose( pfPrepr );
	pfPrepr = NULL;
	PutTrace("UnLoad prepr.");
	return;
}

int CTmcRTH_Indan::GetLine( CString &csCh )
{
	if( pfPrepr == NULL ) OpenPreprFile();
	if( IsError() ) return EOF;

	// Читаем строки, пока не найдётся значимая: пустые строки и комментарии «!»
	// пропускаются; строка, не поместившаяся в буфер, — ошибка (кроме комментария).
	char szBuf[CTMCRTH_INDAN_LINELEN];
	CString csBuf;

	for( ;; )	// A-2: cikl vmesto rekursii - pustye stroki i kommentarii propuskayutsya
	{
	if( fgets( szBuf, CTMCRTH_INDAN_LINELEN, pfPrepr ) == NULL )
	{
		csBuf.Format("Missing %s", CTMCRTH_INDANMET_EOF);
		cError.PutErrorMessage( csBuf );
		ClosePreprFile();
		return EOF;
	};
	{	// N-18: a longer line is not cut silently
		size_t nLen = strlen( szBuf );
		if( ( nLen == CTMCRTH_INDAN_LINELEN - 1 )&&( szBuf[nLen-1] != '\n' )&&( !feof( pfPrepr ) ) )
		{
			const char *pFirst = szBuf;
			while( ( *pFirst == ' ' )||( *pFirst == '\t' ) ) pFirst++;
			if( *pFirst == '!' )	// a long comment is skipped as a whole
			{
				int c;
				while( ( ( c = fgetc( pfPrepr ) ) != EOF )&&( c != '\n' ) ) ;
				szBuf[0] = '\0';
			}
			else
			{
				szBuf[60] = '\0';
				csBuf.Format("A line of the task is longer than %d characters: {%s...}", CTMCRTH_INDAN_LINELEN - 2, szBuf);
				cError.PutErrorMessage( csBuf );
				ClosePreprFile();
				return EOF;
			};
		};
	};

	del_blanks( szBuf );

	if( (szBuf[0] == '!')||(szBuf[0] == '\n') ) szBuf[0] = '\0';

	if( strlen( szBuf ) != 0 ) break;
	}

	csCh.Format("%s", szBuf);

	if( strncmp( csCh, CTMCRTH_INDANMET_EOF, strlen(CTMCRTH_INDANMET_EOF)) == 0 )
	{
		return EOF;
	};

	return 0;
}

void CTmcRTH_Indan::del_blanks( char * szBuf )
{
	int i, n;
	int ii, i1;
	int nGap = 0;	// N-17

	n = (int)(strlen( szBuf ));

	// i1 — позиция записи; ii = 1 — предыдущий символ значимый (промежуток ещё не записан).
	// nGap — число записанных промежутков: первый всегда становится пробелом.
	for( i = 0, i1 = 0, ii = 0; i < n; i++)
	{
		switch( szBuf[i] )
		{
		case ' ':
		case '\t':
			if( ii == 0) break;
			if( ii == 1 )
			{
			szBuf[ i1 ] = ( nGap == 0 ) ? ' ' : szBuf[ i ];	// N-17: a tab after the keyword is a blank
			nGap++;
			i1++;
			ii = 0;
			};
			break;
		default :
			szBuf[ i1 ] = szBuf[ i ];
			i1++;
			ii = 1;
			break;
		};
	};

	szBuf[ i1 ] = szBuf[ i ];
	return;
}

void CTmcRTH_Indan::SetStepNumber( int nStep1 )
{
	if( IsError() ) return;
	nStep = nStep1;
	return;
}

void CTmcRTH_Indan::ReadParamSection( void )
{
	if( IsError() ) return;

	int i;

	for( i = 1; i <= nStep; i++ )
	{
		ReadParamSection( i );
	};

	return;
}

void CTmcRTH_Indan::ReadParamSection( int nStep1 )
{
	CString csCh;
	CString csBuf;

	if( IsError() ) return;
	// Параметры накапливаются: сбрасываются только перед первым шагом.
	if( nStep1 == 1 ) cParam.DeleteData();
	
	FindStep( nStep1 );
	if( FindSection( CTMCRTH_INDANMET_PARAM, CTMCRTH_INDANMET_ENDPARAM ) != 0 )
	{
		if( nStep1 == 1 )
		{
			csBuf.Format("Missing section %s in %d step", CTMCRTH_INDANMET_PARAM, nStep1);
			cError.PutErrorMessage(csBuf);
			ClosePreprFile();
			return;
		}
		else
		{
			// B-15 (author 29.10): every step is an independent task - a step without #PARAM is an error
			// (old: the step silently repeated the parameters of the previous step).
			csBuf.Format("Missing section %s in %d step. Every step is an independent task: write X_MIN, X_MAX, Y_MIN, "
				"Y_MAX, DELTA, TIME, FREQ in the section %s of every step", CTMCRTH_INDANMET_PARAM, nStep1,
				CTMCRTH_INDANMET_PARAM);
			cError.PutErrorMessage(csBuf);
			ClosePreprFile();
			return;
		};
	};
	cParam.BeginSection();	// A-13, B-15: which parameters this step writes
	for( ; GetLineSection( csCh ) != EOF;  )
	{
		if( IsError() )
		{
			ClosePreprFile();
			return;
		}
		cParam.Read( csCh, cError );
	};
	
	cParam.IsCorrectly( cError );
	ClosePreprFile();
	return;
}

void CTmcRTH_Indan::FindStep( int nStep1 )
{
	CString csCh;
	CString csBuf;
	int i;
	if( IsError() ) return;
	
	ClosePreprFile();
	OpenPreprFile();
	
	for( i = 0; GetLine( csCh ) != EOF;  )
	{
		if( IsError() ) return;
		if( strncmp( csCh, CTMCRTH_INDANMET_STEP, strlen(CTMCRTH_INDANMET_STEP) ) == 0 ) 
		{
			i++;
			if( i == nStep1 ) return;
		};
	};
	csBuf.Format("very big step number %d", nStep1);
	cError.PutErrorMessage( csBuf );
	ClosePreprFile();
	return;
}

int CTmcRTH_Indan::FindSection( const char * pszSection,  const char * pszEndSection)
{
	if( IsError() ) return 0;
	csSection.Format( "%s", pszSection );
	csEndSection.Format( "%s", pszEndSection );
	return FindSection();
}

int CTmcRTH_Indan::FindSection( void )
{
	CString csCh;
	CString csBuf;

	if( IsError() ) return 0;
	
	for( ; GetLine( csCh ) != EOF;  )
	{
		if( IsError() ) return 0;
		if( strncmp( csCh, csSection, csSection.GetLength() ) == 0 ) 
		{
			return 0;
		};
		if( strncmp( csCh, CTMCRTH_INDANMET_ENDSTEP, strlen(CTMCRTH_INDANMET_ENDSTEP) ) == 0 ) 
		{
			ClosePreprFile();
			return -1;
		};
	};
	ClosePreprFile();
	return -1;
}

int CTmcRTH_Indan::GetLineSection( CString &csCh )
{
	int i;
	CString csBuf;

	if( IsError() ) return EOF;
	
	i = GetLine( csCh );
	
	if( IsError() ) return EOF;
	
	if( strncmp( csCh, csEndSection, csEndSection.GetLength() ) == 0 ) 
	{
		return EOF;
	};
	if( strncmp( csCh, CTMCRTH_INDANMET_ENDSTEP, strlen(CTMCRTH_INDANMET_ENDSTEP) ) == 0 ) 
	{
		csBuf.Format("Missing  %s", csEndSection);
		cError.PutErrorMessage( csBuf );
		ClosePreprFile();
		return EOF;
	};
	if( i == EOF )
	{
		csBuf.Format("Missing  %s", csEndSection);
		cError.PutErrorMessage( csBuf );
		ClosePreprFile();
		return EOF;
	};
	
	return i;
}


int CTmcRTH_Indan::GetBlock( CString &csCh )
{
	int i;
	CString csBuf;
	CString csBuf1;
	
	csCh.Format("");

	if( IsError() ) return EOF;
	
	i = GetLineSection( csBuf );
	
	if( IsError() ) return EOF;

	if( i == EOF ) return i;
	
	if( strncmp( csBuf, CTMCRTH_INDANMET_BLOCK, strlen(CTMCRTH_INDANMET_BLOCK) ) != 0 ) 
	{
		csBuf1.Format("{%s} must {%s} in %s section",
								csBuf,
								CTMCRTH_INDANMET_BLOCK,
								CTMCRTH_INDANMET_TOPOLOGY );
		cError.PutErrorMessage( csBuf1 );
		ClosePreprFile();
		return EOF;
	};
	
	csCh += csBuf;
	while( (i = GetLineSection( csBuf )) != EOF )
	{
//			FILE *fp;
		if( strncmp( csBuf, CTMCRTH_INDANMET_ENDBLOCK, strlen(CTMCRTH_INDANMET_ENDBLOCK) ) == 0 ) 
		{
			return 0;
		};
		if( strncmp( csBuf, CTMCRTH_INDANMET_BLOCK, strlen(CTMCRTH_INDANMET_BLOCK) ) == 0 ) 
		{
			csBuf1.Format("Missing  %s", CTMCRTH_INDANMET_ENDBLOCK);
			cError.PutErrorMessage( csBuf1 );
			ClosePreprFile();
			return EOF;
		};
		csCh += csBuf;
	}

	csBuf1.Format("Missing  %s", CTMCRTH_INDANMET_ENDBLOCK);
	cError.PutErrorMessage( csBuf1 );
	ClosePreprFile();
	return EOF;
}

void CTmcRTH_Indan::ReadOutputSection( void )
{
	CString csCh;
	CString csBuf;

	if( IsError() ) return;
	
	// Значения секции #OUTPUT переводятся в СИ единицами, прочитанными в #PARM.
	cOut.DeleteData();

	cOut.SetFreqAndTimeUnit( cParam.rFreqUnit, cParam.rTimeUnit );
	
	FindStep( nStep );
	if( FindSection( CTMCRTH_INDANMET_OUTPUT, CTMCRTH_INDANMET_ENDOUTPUT ) != 0 )
	{
		csBuf.Format("Missing section %s in %d step", CTMCRTH_INDANMET_OUTPUT, nStep);
		cError.PutErrorMessage(csBuf);
		ClosePreprFile();
		return;
	};
	for( ; GetLineSection( csCh ) != EOF;  )
	{
		if( IsError() ) 
		{
			ClosePreprFile();
			return;
		}
		cOut.Read( csCh, cError );
	};
	
	cOut.IsCorrectly( cError );
	cOut.AddPath( csCurrentPath );

	// B-3.2: with one frequency S is calculated at FREQ (TtoS reads it from the .t), the frequency of SMATRIX
	// is only written into the .s - they must be equal. A port fed from a signal file ([name]) is the
	// multi-frequency run (wideband post-processing), not checked.
	if( ( !cError.IsError() )&&( cOut.bOutSMatrix ) )
	{
		BOOL bWide = FALSE;
		CTmcRTH_BlockList *pcBl;
		for( pcBl = cTopology.GetBlockList(); pcBl != NULL; pcBl = pcBl->GetNext() )
		{
			CString csV = pcBl->GetcsVx();
			if( ( csV.GetLength() > 0 )&&( ((LPCSTR)csV)[0] == '[' ) ) bWide = TRUE;
		};
		if( cOut.dTmax < 0.0 )	// B-22: SMATRIX without Tmin; Tmax
		{
			if( bWide )
			{
				cOut.dTmin = 0.0;			// the full window of the multi-frequency run
				cOut.dTmax = cParam.rTmax;
			}
			else
			{
				csBuf.Format("%s: with one frequency the window is required - %s name; %lg; Tmin; Tmax; with Tmin in the steady state",
					CTMCRTH_SMATRIXOUT_ID, CTMCRTH_SMATRIXOUT_ID, cOut.dFreq/cParam.rFreqUnit );
				cError.PutErrorMessage(csBuf);
			};
		};
		if( ( !cError.IsError() )&&( !bWide )&&( fabs( cOut.dFreq - cParam.rFreq ) > 1e-9*fabs( cParam.rFreq ) ) )
		{
			csBuf.Format("%s: the frequency %lg %s differs from FREQ %lg %s of step %d. With one frequency the S-matrix"
				" is calculated at FREQ - set the frequency in %s equal to FREQ (%s name; %lg; Tmin; Tmax;)",
				CTMCRTH_SMATRIXOUT_ID, cOut.dFreq/cParam.rFreqUnit, (LPCSTR)cParam.csFreqUnit,
				cParam.rFreq/cParam.rFreqUnit, (LPCSTR)cParam.csFreqUnit, nStep,
				CTMCRTH_SMATRIXOUT_ID, CTMCRTH_SMATRIXOUT_ID, cParam.rFreq/cParam.rFreqUnit );
			cError.PutErrorMessage(csBuf);
		};
	};
	
	ClosePreprFile();
	return;
}

void CTmcRTH_Indan::ReadLinkListSection( void )
{
	CString csCh;
	CString csBuf;

	if( IsError() ) return;
	
	cTopology.SetParam( &cParam );

	FindStep( nStep );
	if( FindSection( CTMCRTH_INDANMET_LINKLIST, CTMCRTH_INDANMET_ENDLINK ) != 0 )
	{
		csBuf.Format("Missing section %s in %d step", CTMCRTH_INDANMET_LINKLIST, nStep);
		cError.PutErrorMessage(csBuf);
		ClosePreprFile();
		return;
	};
	for( ; GetLineSection( csCh ) != EOF;  )
	{
		if( IsError() ) 
		{
			ClosePreprFile();
			return;
		}
		cTopology.ReadLink( csCh, cError );
	};
	
	cTopology.IsCorrectlyLink( cError );
	ClosePreprFile();
	return;
}

void CTmcRTH_Indan::ReadTopologySection( void )
{
	CString csCh;
	CString csBuf;

	if( IsError() ) return;
	// Список блоков строится заново для каждого шага.
	cTopology.DeleteData();
	
	cTopology.SetParam( &cParam );
	
	FindStep( nStep );
	if( FindSection( CTMCRTH_INDANMET_TOPOLOGY, CTMCRTH_INDANMET_ENDTOPOLOGY ) != 0 )
	{
		csBuf.Format("Missing section %s in %d step", CTMCRTH_INDANMET_TOPOLOGY, nStep);
		cError.PutErrorMessage(csBuf);
		ClosePreprFile();
		return;
	};
	for( ; GetBlock( csCh ) != EOF;  )
	{
		if( IsError() ) 
		{
			ClosePreprFile();
			return;
		}
		cTopology.Read( csCh, cError );
	};
	
	cTopology.IsCorrectly( cError );
	ClosePreprFile();
	cTopology.AddPathForFile( csCurrentPath );
	return;
}



CTmcRTH_IndanOutput* CTmcRTH_Indan::GetOutput( void )
{
	return &cOut;
}

CTmcRTH_IndanParam& CTmcRTH_Indan::GetParam( void )
{
	return cParam;
}

CTmcRTH_IndanTopology& CTmcRTH_Indan::GetTopology( void )
{
	return cTopology;
}

BOOL CTmcRTH_Indan::IsDataRead( void )
{
	return cTopology.IsDataRead();
}

int CTmcRTH_Indan::GetnStep( void )
{
	return nStep;
}


int CTmcRTH_Indan::nGetTmax( void )
{
	return cParam.nGetTmax();
}

double CTmcRTH_Indan::dGetTmin( void )
{
	return cParam.dGetTmin();
}

double CTmcRTH_Indan::dGetdT( void )
{
	return cParam.dGetdT();
}

double CTmcRTH_Indan::dGetDelta( void )
{
	return cParam.dGetDelta();
}

void CTmcRTH_Indan::GetXYMinMax( double &dXmin, double &dXmax, double &dYmin, double &dYmax )
{
	cParam.GetXYMinMax( dXmin, dXmax, dYmin, dYmax );
	return;
}

int CTmcRTH_Indan::GetnBlock( void )
{
	return cTopology.GetBlockNumber();
}

void CTmcRTH_Indan::OnOffFieldOutput()
{
	cOut.OnOffFieldOutput();
	return;
}



void CTmcRTH_Indan::OnOffSinchronizationFieldOutput()
{
	cOut.OnOffSinchronizationFieldOutput();
	return;
}

void CTmcRTH_Indan::GetnXnY(int &nX11, int &nY11)
{
	nX11 = cParam.CalcnX();
	nY11 = cParam.CalcnY();
	return;
}

CString CTmcRTH_Indan::GetCurrentPath()
{
	return csCurrentPath;
}

void CTmcRTH_Indan::OnOffSoundEffects()
{
	cOut.OnOffSoundEffects();
	return;
}




// Write file csFileName for calculating in Maple 9.0
// after executing Maple 9.0 data was write in file csFileNameData
// dHeight   - height stator and rolik in mm
// dRolikR1  - internal radius of rolik in mm
// dRolikR2  - external radius of rolik in mm
// dStatorR1 - internal radius of stator in mm
// dStatorR2 - external radius of stator in mm
// dMagnMomP -  magnetic moment






void MainPrepr( CString csTplFileNamePrepr,	CString csTplFileName, CString csCurrentPath, CTmcLibError &cError )
{
	if( cError.IsError() ) return;

	char *szError, *szPathName;
	
	if( (szError = new char[ sizeof(char)*BUF_STR_N_MAX ]) == NULL )
	{
		cError.PutErrorMessage("Memory allocation");
		return;
	};
	if( (szPathName = new char[ sizeof(char)*BUF_STR_N_MAX ]) == NULL )
	{
		delete  []szError;
		cError.PutErrorMessage("Memory allocation");
		return;
	};

//	char *ch;
	char szSourceFileName[512];
	char szTargetFileName[512];

	// J-1: the paths must fit the buffers (old: strcpy without a check wrote past the 512 bytes on the stack)
	if( ( strlen( csTplFileName ) >= sizeof(szSourceFileName) )||( strlen( csTplFileNamePrepr ) >= sizeof(szTargetFileName) )||
		( strlen( csCurrentPath ) >= BUF_STR_N_MAX ) )
	{
		delete []szPathName;
		delete []szError;
		cError.PutErrorMessage("The path of the task is too long for the preprocessor (more than 511 characters): "
			"move the task to a shorter folder");
		return;
	};
	
	// Препроцессор принимает строки C; буферы имён — 512 символов.
	strcpy( szSourceFileName, csTplFileName);
	strcpy( szTargetFileName, csTplFileNamePrepr);
	strcpy( szPathName, csCurrentPath);


	MainPrepr1( szTargetFileName,	szSourceFileName, szPathName, szError );

	if( strlen(szError) != 0 ) cError.PutErrorMessage(szError);
	
	delete []szPathName;
	delete []szError;

	return;
}
