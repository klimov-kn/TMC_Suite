// TmcCalcTiming.h : precise timing of the calculation phases of the kernels
//                   PlanarRT_H (tmc_rth.exe) and PlanarRT_X (tmc_rtx.exe).
//
// Used ONLY by the kernels. Timer: QueryPerformanceCounter (QPC).
// One QPC read costs ~20-30 ns, a few reads per time step are made, so the
// measurement does not change the calculation speed (much less than 0.1 %).
// The old time outputs (ElapsTime in the status bar, "nT:...;sec;sec/takt")
// are NOT touched by this module.
//
// Header-only: every method is inline (defined in the class).

/// @file TmcCalcTiming.h
/// @brief Замер времени фаз расчёта ядер H и X и текстовый отчёт «Calc. time...» / ключ `/Time`.
///
/// Часть модуля «Расчёт на видеокарте и замер времени». Ядро (`CTmcRTHRectNode`)
/// владеет одним объектом `CTmcCalcTiming` и прибавляет к счётчикам фаз время,
/// измеренное по `QueryPerformanceCounter`; документ ядра отмечает начало и конец
/// шага и «Run all». Отчёт строит `BuildReport`: окно статистики (кнопка «Calc. time...»)
/// показывает его и сохраняет в TXT, а с ключом `/Time` командной строки отчёт после
/// «Run all» сам записывается рядом с .tpl (`DefaultReportName`). Для каждого шага в
/// отчёте есть строка `Compute:` — где шёл цикл по времени (процессор или видеокарта).
/// Все методы встроенные.
/// @see https://www.tamic.ru/docs/code/gpu.html#timing

#ifndef _TMCCALCTIMING_H__
#define _TMCCALCTIMING_H__

#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <vector>

#pragma comment(lib, "psapi.lib")
#pragma warning(push)
#pragma warning(disable: 4996)	// fopen/strcpy as in the rest of the project

/// @brief Возвращает текущее показание счётчика `QueryPerformanceCounter`, тиков.
///
/// Разность двух показаний, делённая на частоту счётчика (`CTmcCalcTiming::qFreq`),
/// даёт время в секундах. Один вызов стоит десятки наносекунд.
/// @return Показание счётчика, тиков QPC.
inline __int64 TmcTimingNow( void )
{
	LARGE_INTEGER t;
	QueryPerformanceCounter( &t );
	return t.QuadPart;
}

// Phase counters (QPC ticks)
/// @brief Номера счётчиков фаз расчёта — индексы массива `TmcTimingStep::q` (тики QPC).
///
/// Группы: A — препроцессор и чтение задания, B — подготовка, C — цикл по времени,
/// D — постобработка, E — прочее. Пояснение к каждой фазе — в хвостовом комментарии.
/// При расчёте на видеокарте блоки METAL/ABSORBER/MAGNETIC считаются вместе с узлами
/// сетки (фаза `TMCT_SCATTER`), счётчик `TMCT_BLOCKNODE` остаётся нулевым.
/// @see https://www.tamic.ru/docs/code/gpu.html#timing
enum
{
	// A. Preprocessor and input
	/// A1: освобождение данных предыдущего шага.
	TMCT_MEMFREE = 0,		// release of the data of the previous step
	/// A2: препроцессор — развёртка макросов .tpl.
	TMCT_PREPR,				// preprocessor: macro expansion of .tpl (prepr.lib)
	/// A3: чтение разделов `#PARM`, `#TOPOLOGY`, `#LINK_LIST`, `#OUTPUT`.
	TMCT_READTPL,			// reading sections #PARM/#TOPOLOGY/#LINK_LIST/#OUTPUT
	// B. Preparation
	/// B1: удаление старых выходных файлов, создание файла сигналов.
	TMCT_OUTFILEINIT,		// delete old output files, create signal file
	/// B2: создание файлов накопления FIELD_DISTRIBUTION.
	TMCT_DISTRINIT,			// FIELD_DISTRIBUTION: creation of accumulation files
	/// B3: выделение и обнуление массивов сетки.
	TMCT_MEMINIT,			// allocation and zeroing of the grid arrays
	/// B4: построение блоков на сетке (eps, металл, поглотители, порты), всего.
	TMCT_BLOCKGEOM,			// blocks on the grid (eps, metal, absorber, ports)
	/// B5: запись файлов распределения eps блоков (часть B4).
	TMCT_EPSFILE,			// writing eps-distribution files of blocks (part of BLOCKGEOM)
	/// B5a: чтение файлов распределения блоками FILE (часть B4).
	TMCT_EPSREAD,			// FILE blocks: reading distribution files (part of BLOCKGEOM)
	/// B6: пересчёт eps в проводимости узлов.
	TMCT_ADMITTANCE,		// eps -> node admittance
	/// B7: запись файла TOPOLOGY.
	TMCT_TOPOLOGYFILE,		// writing TOPOLOGY file
	// C. Time loop
	/// C1 (вместе с C4): возбуждение портов; на видеокарте включает и обмен узлами портов и ожидание карты.
	TMCT_EXCITE,			// port excitation, gross (includes TMCT_SIGNALFILE)
	/// C2: обновление всех узлов сетки; на видеокарте — постановка `Pass` и `Blocks` в очередь.
	TMCT_SCATTER,			// scattering in all grid nodes
	/// C3: узлы блоков (металл, поглотители); на видеокарте не используется (0).
	TMCT_BLOCKNODE,			// block nodes (metal, absorber, ports)
	/// C4: запись nT, t и сигналов портов в файл (входит в C1).
	TMCT_SIGNALFILE,		// writing nT/t and port signals to the output file (inside EXCITE)
	/// C5: запись файла FIELDS (на видеокарте — вместе с копированием сетки с карты).
	TMCT_FIELDFILE,			// writing FIELDS file
	/// C6: накопление FIELD_DISTRIBUTION.
	TMCT_DISTRINTEGR,		// FIELD_DISTRIBUTION accumulation
	/// C7: обновление строки состояния.
	TMCT_STATUS,			// status bar updates
	/// Цикл по времени целиком, по часам.
	TMCT_LOOPWALL,			// time loop, total wall time
	// D. Postprocessor
	/// D1: итог FIELD_DISTRIBUTION — файлы .AMP/.FAZ/.DAT и ДН.
	TMCT_DISTRFINAL,		// FIELD_DISTRIBUTION final: .AMP/.FAZ/.DAT + directional pattern
	/// D2: S-матрица (.s).
	TMCT_SMATRIX,			// S-matrix (.s)
	// E. Other
	/// E1: звуковой сигнал в конце шага.
	TMCT_SOUND,				// sound signal at the end of the step (Config/Sound)
	/// Число счётчиков (размер массива `TmcTimingStep::q`).
	TMCT_COUNT
};

/// @brief Замер одного шага (#STEP) задания: размеры, состояние и счётчики фаз.
///
/// Заполняется ядром во время шага (`CTmcCalcTiming::cur`), по окончании копируется в
/// историю. Структура простая (POD): обнуляется `memset` в `ClearStep`.
struct TmcTimingStep
{
	/// Номер шага задания.
	int     nStep;
	/// @name Сетка шага
	///@{
	/// Число узлов сетки по осям X и Y.
	int     nX, nY;
	///@}
	/// Всего узлов сетки.
	double  dNodes;
	/// Число блоков шага.
	int     nBlocks;
	/// Число блоков, записавших файл распределения (.eps).
	int     nEpsWritten;		// blocks that wrote a distribution file: RECT_STAT* [file]expr
	/// Число блоков, прочитавших файл распределения (FILE, FILE_N, FILE_B, FILE_Y).
	int     nEpsRead;			// blocks read from a distribution file: FILE, FILE_N, FILE_B, FILE_Y
	/// Заданное число тактов шага.
	int     nTMax;
	/// Выполнено тактов (меньше `nTMax` при остановке или ошибке).
	int     nTDone;
	/// Шаг по времени, с.
	double  dT;
	/// Шаг сетки, м.
	double  dDelta;
	/// Состояние: 0 — не запускался, 1 — идёт, 2 — завершён, 3 — остановлен, 4 — ошибка.
	int     nState;				// 0 - not run, 1 - running, 2 - finished, 3 - stopped, 4 - error
	/// Счётчики фаз, тиков QPC; индекс — `TMCT_*`.
	__int64 q[TMCT_COUNT];
	/// Время `RunStep` по часам (подготовка + цикл + постобработка), тиков QPC; при продолжении после остановки накапливается.
	__int64 qRunStep;			// RunStep wall time (init + loop + post)
	/// Местное время начала шага.
	SYSTEMTIME stStart;
	/// Где шёл цикл по времени: копия `CTmcGpuH::csInfo` / `CTmcGpuX::csInfo` («CPU», «CPU (причина)» или «GPU n: ...»); пусто — CPU.
	char    szCompute[200];		// GPU: "CPU" or the video card the loop ran on (empty - CPU)
};

/// @brief Хронометраж расчёта ядра: счётчики фаз текущего шага, история шагов и отчёт.
///
/// Один объект на ядро (член `CTmcRTHRectNode`, доступ через `GetTiming()`). Ядро
/// прибавляет время фаз к `cur.q[]` (`Add`, `Counter`); документ вызывает
/// `BeginStep` при чтении шага, `BeginRunStep`/`EndRunStep` вокруг расчёта шага и
/// `BeginRunAll`/`EndRunAll` вокруг «Run all». Завершённые шаги текущего прогона
/// хранятся в истории; `BuildReport` строит по ним текстовый отчёт.
/// Копирование запрещено (внутри критическая секция).
/// @note Потоки: методы начала/конца шага и `BuildReport` защищены критической секцией —
/// отчёт можно строить из потока окна во время расчёта. `Add` и `Counter` не защищены
/// и вызываются только расчётным потоком.
/// @see https://www.tamic.ru/docs/code/gpu.html#timing
class CTmcCalcTiming
{
public:
	/// @brief Читает частоту счётчика QPC, создаёт критическую секцию и обнуляет замеры.
	CTmcCalcTiming()
	{
		LARGE_INTEGER f;
		QueryPerformanceFrequency( &f );
		qFreq = f.QuadPart;
		if( qFreq <= 0 ) qFreq = 1;
		InitializeCriticalSection( &cs );
		ClearStep( cur );
		cur.nStep = 0;
		qRunPrepr = 0;
		qRunStart = 0;
		qRunWall = 0;
		bRunActive = FALSE;
		bRunAll = FALSE;
		dCpuUser = dCpuKernel = -1.;
		memset( &stRunStart, 0, sizeof(stRunStart) );
		bDistrStarted = bDistrFinal = FALSE;
	}
	/// @brief Удаляет критическую секцию.
	~CTmcCalcTiming()
	{
		DeleteCriticalSection( &cs );
	}

	// ---------------------------------------------------------------- data
	/// Частота счётчика QPC, тиков в секунду (не меньше 1).
	__int64 qFreq;
	/// Текущий шаг: последний прочитанный или выполняемый.
	TmcTimingStep cur;					// current (last read / running) step
	/// Время препроцессора перед шагами при «Run all» (подсчёт шагов), тиков QPC.
	__int64 qRunPrepr;					// preprocessor run before the steps (Run all)
	/// Показание QPC в начале прогона (запуска шага или «Run all»).
	__int64 qRunStart;
	/// Время всего прогона по часам, тиков QPC.
	__int64 qRunWall;					// wall time of the whole Run
	/// `TRUE` — идёт «Run all» (шаги внутри него не начинают новый прогон).
	volatile BOOL bRunActive;
	/// `TRUE` — последний прогон был «Run all» (следующий одиночный шаг очистит историю).
	BOOL bRunAll;
	/// @name Процессорное время расчётного потока
	///@{
	/// Процессорное время расчётного потока, с: `dCpuUser` — пользовательское,
	/// `dCpuKernel` — системное; -1 — не измерено.
	double dCpuUser, dCpuKernel;		// CPU time of the calculation thread, s
	///@}
	/// Местное время начала прогона.
	SYSTEMTIME stRunStart;
	/// Полное имя файла задания .tpl (для отчёта и имени файла отчёта).
	CString csTplFile;
	/// Название ядра для отчёта, например «PlanarRT_H (tmc_rth.exe)»; «_X» в имени выбирает суффикс X.
	CString csKernel;					// "PlanarRT_H (tmc_rth.exe)"
	/// @name Состояние окна FIELD_DISTRIBUTION
	///@{
	/// Состояние окна FIELD_DISTRIBUTION для `IsDistrFinalCall`: `bDistrStarted` — время
	/// вошло в окно; `bDistrFinal` — итоговый вызов уже был.
	BOOL bDistrStarted, bDistrFinal;	// FIELD_DISTRIBUTION window state
	///@}

	// ------------------------------------------------------------- helpers
	/// @brief Возвращает указатель на счётчик фазы текущего шага.
	/// @param[in] i  номер фазы `TMCT_*`
	/// @return Адрес `cur.q[i]`, тиков QPC.
	__int64 *Counter( int i ) { return &(cur.q[i]); }
	/// @brief Прибавляет к счётчику фазы время от `qBegin` до текущего момента.
	/// @param[in] i       номер фазы `TMCT_*`
	/// @param[in] qBegin  показание `TmcTimingNow` в начале фазы
	void Add( int i, __int64 qBegin ) { cur.q[i] += TmcTimingNow() - qBegin; }
	/// @brief Переводит тики QPC в секунды.
	/// @param[in] q  интервал, тиков QPC
	/// @return Интервал, с.
	double Sec( __int64 q ) const { return ((double)q)/((double)qFreq); }

	/// @brief Обнуляет замер шага, сохраняя номер шага, и ставит время начала = сейчас.
	/// @param[in,out] s  замер шага
	static void ClearStep( TmcTimingStep &s )
	{
		int nStep = s.nStep;
		memset( &s, 0, sizeof(TmcTimingStep) );
		s.nStep = nStep;
		GetLocalTime( &s.stStart );
	}

	// new step is read from .tpl (ReadData)
	/// @brief Начинает замер нового шага (вызывается при чтении шага из .tpl).
	/// @param[in] nStep  номер шага задания
	void BeginStep( int nStep )
	{
		EnterCriticalSection( &cs );
		cur.nStep = nStep;
		ClearStep( cur );
		bDistrStarted = bDistrFinal = FALSE;
		LeaveCriticalSection( &cs );
	}

	// RunStep started
	/// @brief Отмечает начало расчёта шага (`RunStep`).
	///
	/// Вне «Run all» начинает новый прогон: история очищается, если прошлый прогон был
	/// «Run all», если номер шага меньше последнего или тот же шаг запущен заново (не
	/// продолжение после остановки).
	void BeginRunStep( void )
	{
		EnterCriticalSection( &cs );
		if( !bRunActive )
		{
			// single step run (not inside Run all): the run is this step
			if( bRunAll )
			{
				hist.clear();				// previous Run all -> new history
				bRunAll = FALSE;
			}
			else if( !hist.empty() )
			{
				// new pass: step number is less than the last one, or the same
				// step again (not a continuation after Stop)
				const TmcTimingStep &b = hist.back();
				if( ( cur.nStep < b.nStep )||( ( cur.nStep == b.nStep )&&( b.nState != 3 ) ) ) hist.clear();
			}
			qRunPrepr = 0;
			qRunStart = TmcTimingNow();
			GetLocalTime( &stRunStart );
			dCpuUser = dCpuKernel = -1.;
		}
		if( cur.nState == 0 ) GetLocalTime( &cur.stStart );
		cur.nState = 1;
		LeaveCriticalSection( &cs );
	}

	// RunStep finished: nState 2 - finished, 3 - stopped, 4 - error
	/// @brief Отмечает конец расчёта шага и заносит его в историю.
	///
	/// Продолжение остановленного шага заменяет его прежнюю запись. Вне «Run all»
	/// заодно фиксирует время прогона и процессорное время потока.
	/// @param[in] nState         итог: 2 — завершён, 3 — остановлен пользователем, 4 — ошибка
	/// @param[in] qRunStepBegin  показание `TmcTimingNow` в начале `RunStep`
	void EndRunStep( int nState, __int64 qRunStepBegin )
	{
		EnterCriticalSection( &cs );
		cur.qRunStep += TmcTimingNow() - qRunStepBegin;	// += : continuation after Stop
		cur.nState = nState;
		// continuation of a stopped step replaces its record
		if( !hist.empty() && ( hist.back().nStep == cur.nStep )&&( hist.back().nState == 3 ) ) hist.back() = cur;
		else hist.push_back( cur );
		if( !bRunActive )
		{
			qRunWall = TmcTimingNow() - qRunStart;
			ReadThreadCpu();
		}
		LeaveCriticalSection( &cs );
	}

	// Run all started / finished (Doc::RunAll)
	/// @brief Отмечает начало «Run all»: очищает историю и запускает часы прогона.
	void BeginRunAll( void )
	{
		EnterCriticalSection( &cs );
		hist.clear();
		qRunPrepr = 0;
		qRunWall = 0;
		qRunStart = TmcTimingNow();
		GetLocalTime( &stRunStart );
		dCpuUser = dCpuKernel = -1.;
		bRunAll = TRUE;
		bRunActive = TRUE;
		LeaveCriticalSection( &cs );
	}
	/// @brief Отмечает конец «Run all»: время прогона по часам и процессорное время потока.
	/// @note Вызывается из расчётного потока: процессорное время берётся у текущего потока.
	void EndRunAll( void )
	{
		EnterCriticalSection( &cs );
		qRunWall = TmcTimingNow() - qRunStart;
		bRunActive = FALSE;
		ReadThreadCpu();
		LeaveCriticalSection( &cs );
	}

	// FIELD_DISTRIBUTION: is the next Integrate() call the final one
	// (the same test as in CFieldIntegrated::Integrate)
	/// @brief Определяет, будет ли очередной вызов накопления FIELD_DISTRIBUTION последним (итоговым).
	///
	/// Нужен, чтобы время итоговой записи .AMP/.FAZ/.DAT отнести к фазе D1, а не C6.
	/// Повторяет условие `CFieldIntegrated::Integrate`: окно открылось, когда время вошло
	/// в [dTmin, dTmax]; итог — первый вызов с dTCur >= dTmax после этого.
	/// @param[in] bIsDistr  в задании есть FIELD_DISTRIBUTION
	/// @param[in] dTCur     текущее время расчёта, с
	/// @param[in] dTmin     начало окна накопления, с
	/// @param[in] dTmax     конец окна накопления, с
	/// @return `TRUE` — этот вызов итоговый (возвращается один раз за шаг).
	BOOL IsDistrFinalCall( BOOL bIsDistr, double dTCur, double dTmin, double dTmax )
	{
		if( !bIsDistr ) return FALSE;
		if( !bDistrStarted )
		{
			if( ( dTCur >= dTmin )&&( dTCur <= dTmax ) ) bDistrStarted = TRUE;
			return FALSE;
		}
		if( !bDistrFinal && ( dTCur >= dTmax ) )
		{
			bDistrFinal = TRUE;
			return TRUE;
		}
		return FALSE;
	}

	// ------------------------------------------------------------- report
	/// @brief Строит текстовый отчёт о времени расчёта (окно «Calc. time...», файл `/Time`).
	///
	/// Шапка: ядро, платформа, дата сборки, файл .tpl, компьютер, процессор, память,
	/// частота таймера, пиковая память процесса. Затем по каждому шагу истории (и по
	/// выполняемому шагу) — таблица фаз A–E в формате h:mm:ss.mmm, секунды и доля шага,
	/// строка `Compute:` (процессор или видеокарта), «чистое время счёта» C1+C2+C3 и
	/// скорость в нс на узел за такт. В конце — сводка по всем шагам и пояснения.
	/// Текст по-английски, концы строк CRLF.
	/// @return Текст отчёта.
	/// @note Можно вызывать во время расчёта из потока окна: данные копируются под
	/// критической секцией.
	/// @see https://www.tamic.ru/docs/code/gpu.html#time-report
	CString BuildReport( void )
	{
		CString r, s;
		std::vector<TmcTimingStep> h;
		TmcTimingStep c;
		EnterCriticalSection( &cs );
		h = hist;
		c = cur;
		__int64 qRP = qRunPrepr, qRW = qRunWall, qRS = qRunStart;
		BOOL bAct = bRunActive, bAll = bRunAll;
		double dU = dCpuUser, dK = dCpuKernel;
		SYSTEMTIME stR = stRunStart;
		LeaveCriticalSection( &cs );
		if( bAct ) qRW = TmcTimingNow() - qRS;
		// the running step (or a step only read) is shown too
		if( ( c.nState == 1 )||( h.empty() ) ) h.push_back( c );

		SYSTEMTIME st;
		GetLocalTime( &st );
		const char *line = "==============================================================================\r\n";
		const char *line1 = "------------------------------------------------------------------------------\r\n";

		r += line;
		r += " TMC Suite - calculation time report\r\n";
		r += line;
		s.Format( "Kernel             : %s, platform %s, build %s\r\n", (LPCTSTR)csKernel,
#ifdef _WIN64
			"win64",
#else
			"win32",
#endif
			__DATE__ ); r += s;
		s.Format( "Project (.tpl)     : %s\r\n", (LPCTSTR)csTplFile ); r += s;
		s.Format( "Report created     : %04d-%02d-%02d %02d:%02d:%02d\r\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond ); r += s;
		if( stR.wYear != 0 )
		{
			s.Format( "Run started        : %04d-%02d-%02d %02d:%02d:%02d (%s)%s\r\n", stR.wYear, stR.wMonth, stR.wDay, stR.wHour, stR.wMinute, stR.wSecond,
				bAll ? "Run all" : "Run step", bAct ? "  - STILL RUNNING" : "" );
			r += s;
		}
		r += MachineInfo();
		s.Format( "Timer              : QueryPerformanceCounter, %I64d Hz (resolution %.3f us)\r\n", qFreq, 1.e6/qFreq ); r += s;
		{
			PROCESS_MEMORY_COUNTERS pmc;
			memset( &pmc, 0, sizeof(pmc) );
			pmc.cb = sizeof(pmc);
			if( GetProcessMemoryInfo( GetCurrentProcess(), &pmc, sizeof(pmc) ) )
			{
				s.Format( "Peak memory        : %.1f MB (peak working set of the process)\r\n", pmc.PeakWorkingSetSize/1048576. ); r += s;
			}
		}
		r += "\r\n";

		size_t k;
		for( k = 0; k < h.size(); k++ ) r += StepReport( h[k], line1 );

		// summary for all steps
		r += line1;
		r += " SUMMARY FOR ALL STEPS (h:mm:ss.mmm)\r\n";
		r += line1;
		r += " Step  Prepr+input  Preparation    Time loop   Pure calc.   Disk write    Postproc.        Sound   Step total  State\r\n";
		double t[8];
		memset( t, 0, sizeof(t) );
		for( k = 0; k < h.size(); k++ )
		{
			double a[8];
			Group( h[k], a );
			s.Format( " %4d %12s %12s %12s %12s %12s %12s %12s %12s  %s\r\n", h[k].nStep, (LPCTSTR)Hms( a[0] ), (LPCTSTR)Hms( a[1] ), (LPCTSTR)Hms( a[2] ),
				(LPCTSTR)Hms( a[3] ), (LPCTSTR)Hms( a[4] ), (LPCTSTR)Hms( a[5] ), (LPCTSTR)Hms( a[7] ), (LPCTSTR)Hms( a[6] ), StateName( h[k].nState ) );
			r += s;
			int j;
			for( j = 0; j < 8; j++ ) t[j] += a[j];
		}
		s.Format( " ALL  %12s %12s %12s %12s %12s %12s %12s %12s\r\n", (LPCTSTR)Hms( t[0] ), (LPCTSTR)Hms( t[1] ), (LPCTSTR)Hms( t[2] ),
			(LPCTSTR)Hms( t[3] ), (LPCTSTR)Hms( t[4] ), (LPCTSTR)Hms( t[5] ), (LPCTSTR)Hms( t[7] ), (LPCTSTR)Hms( t[6] ) ); r += s;
		r += "\r\n";
		s.Format( " Preprocessor before the steps (Run all, count of steps) : %s\r\n", (LPCTSTR)SecStr( Sec( qRP ) ) ); r += s;
		s.Format( " PURE CALCULATION TIME, all steps                          : %s\r\n", (LPCTSTR)SecStr( t[3] ) ); r += s;
		s.Format( " Total of all phases, all steps                            : %s\r\n", (LPCTSTR)SecStr( t[6] + Sec( qRP ) ) ); r += s;
		if( qRW > 0 )
		{
			s.Format( " TOTAL CALCULATION TIME (wall, from Run to the end)       : %s%s\r\n", (LPCTSTR)SecStr( Sec( qRW ) ), bAct ? " (running)" : "" ); r += s;
		}
		if( dU >= 0. )
		{
			s.Format( " CPU time of the calculation thread (reference, ~16 ms accuracy):\r\n   user %s, system %s\r\n",
				(LPCTSTR)SecStr( dU ), (LPCTSTR)SecStr( dK ) ); r += s;
			if( qRW > 0 )
			{
				s.Format( "   CPU / wall = %.1f %% (the rest - waiting: disk, swap, sound signal, window messages)\r\n", 100.*(dU + dK)/Sec( qRW ) ); r += s;
			}
		}
		r += "\r\n";
		r += " Notes:\r\n";
		r += "  Pure calculation = C1 + C2 + C3 (port excitation, scattering in the grid,\r\n";
		r += "  block nodes) without disk writes, preprocessor, preparation, FIELDS,\r\n";
		r += "  FIELD_DISTRIBUTION and postprocessor.\r\n";
		r += "  Disk write = B1 + B2 + B5 + B7 + C4 + C5 (FIELD_DISTRIBUTION is shown in C6).\r\n";
		r += "  B5 and B5a are parts of B4; both are zero when the project does not use\r\n";
		r += "  distribution files (.eps: eps or plasma N/B/Y).\r\n";
		r += "  The old 'ElapsTime' in the status bar is measured as before (whole seconds).\r\n";
		r += line;
		return r;
	}

	// write the report to a text file (TRUE - OK)
	/// @brief Записывает отчёт `BuildReport` в текстовый файл (двоичный режим, CRLF из текста).
	/// @param[in] lpszFile  полное имя файла; существующий файл перезаписывается
	/// @return `TRUE` — записан; `FALSE` — файл не открылся.
	BOOL SaveReport( LPCTSTR lpszFile )
	{
		CString csR = BuildReport();
		FILE *fp = fopen( lpszFile, "wb" );
		if( fp == NULL ) return FALSE;
		fwrite( (LPCTSTR)csR, 1, csR.GetLength(), fp );
		fclose( fp );
		return TRUE;
	}

	// default file name for the report: <tpl without extension>.timing_<H|X>_<win32|win64>.txt
	/// @brief Возвращает имя файла отчёта рядом с заданием: `<tpl без расширения>.timing_<H|X>_<win32|win64>.txt`.
	/// @return Полное имя файла (буква ядра — по наличию «_X» в `csKernel`).
	CString DefaultReportName( void )
	{
		CString csName = csTplFile;
		int i = csName.ReverseFind( '.' );
		int j = csName.ReverseFind( '\\' );
		if( ( i > 0 )&&( i > j ) ) csName = csName.Left( i );
		csName += ".timing_";
		csName += ( csKernel.Find( "_X" ) >= 0 ) ? "X_" : "H_";
#ifdef _WIN64
		csName += "win64.txt";
#else
		csName += "win32.txt";
#endif
		return csName;
	}

private:
	/// Критическая секция для начала/конца шага и построения отчёта.
	CRITICAL_SECTION cs;
	/// Завершённые (и остановленные) шаги текущего прогона.
	std::vector<TmcTimingStep> hist;	// finished steps of the current run

	/// @brief Копирование запрещено (объявлен, не определён).
	CTmcCalcTiming( const CTmcCalcTiming & );
	/// @brief Присваивание запрещено (объявлен, не определён).
	/// @return Не используется.
	CTmcCalcTiming & operator=( const CTmcCalcTiming & );

	/// @brief Записывает процессорное время текущего потока в `dCpuUser`, `dCpuKernel` (с, точность ~16 мс).
	void ReadThreadCpu( void )
	{
		FILETIME ftC, ftE, ftK, ftU;
		if( GetThreadTimes( GetCurrentThread(), &ftC, &ftE, &ftK, &ftU ) )
		{
			ULARGE_INTEGER u, k;
			u.LowPart = ftU.dwLowDateTime; u.HighPart = ftU.dwHighDateTime;
			k.LowPart = ftK.dwLowDateTime; k.HighPart = ftK.dwHighDateTime;
			dCpuUser   = u.QuadPart*1.e-7;
			dCpuKernel = k.QuadPart*1.e-7;
		}
	}

	/// @brief Возвращает название состояния шага для отчёта.
	/// @param[in] n  состояние `TmcTimingStep::nState`
	/// @return «running», «finished», «stopped by user», «error» или «not run».
	static const char *StateName( int n )
	{
		switch( n )
		{
			case 1: return "running";
			case 2: return "finished";
			case 3: return "stopped by user";
			case 4: return "error";
		}
		return "not run";
	}

	// groups: 0 prepr+input, 1 preparation, 2 time loop, 3 pure calc, 4 disk, 5 post, 6 total
	/// @brief Сводит счётчики шага в группы отчёта, с.
	///
	/// a[0] — A (препроцессор и чтение), a[1] — B (подготовка), a[2] — цикл по часам,
	/// a[3] — чистый счёт C1+C2+C3, a[4] — запись на диск B1+B2+B5+B7+C4+C5,
	/// a[5] — D (постобработка), a[7] — E (звук), a[6] — всего A+B+C+D+E.
	/// @param[in]  c  замер шага
	/// @param[out] a  массив из 8 элементов, с
	void Group( const TmcTimingStep &c, double *a )
	{
		const __int64 *q = c.q;
		a[0] = Sec( q[TMCT_MEMFREE] + q[TMCT_PREPR] + q[TMCT_READTPL] );
		a[1] = Sec( q[TMCT_OUTFILEINIT] + q[TMCT_DISTRINIT] + q[TMCT_MEMINIT] + q[TMCT_BLOCKGEOM] +
				q[TMCT_ADMITTANCE] + q[TMCT_TOPOLOGYFILE] );
		a[2] = Sec( q[TMCT_LOOPWALL] );
		a[3] = Sec( q[TMCT_EXCITE] - q[TMCT_SIGNALFILE] + q[TMCT_SCATTER] + q[TMCT_BLOCKNODE] );
		a[4] = Sec( q[TMCT_OUTFILEINIT] + q[TMCT_DISTRINIT] + q[TMCT_EPSFILE] + q[TMCT_TOPOLOGYFILE] +
				q[TMCT_SIGNALFILE] + q[TMCT_FIELDFILE] );
		a[5] = Sec( q[TMCT_DISTRFINAL] + q[TMCT_SMATRIX] );
		a[7] = Sec( q[TMCT_SOUND] );
		a[6] = a[0] + a[1] + a[2] + a[5] + a[7];
	}

	/// @brief Форматирует строку таблицы фаз: название, h:mm:ss.mmm, секунды и доля шага.
	/// @param[in] szName  название фазы
	/// @param[in] d       время фазы, с
	/// @param[in] dTot    время всего шага, с; <= 0 — доля не печатается
	/// @return Строка с CRLF.
	CString Row( const char *szName, double d, double dTot )
	{
		CString s;
		if( dTot > 0. ) s.Format( "   %-50s %15s %14.6f  %6.2f %%\r\n", szName, (LPCTSTR)Hms( d ), d, 100.*d/dTot );
		else            s.Format( "   %-50s %15s %14.6f\r\n", szName, (LPCTSTR)Hms( d ), d );
		return s;
	}

	/// @brief Строит раздел отчёта одного шага: сетка, такты, строка `Compute:`, таблица фаз, итоги и скорость.
	///
	/// Скорость = чистое время счёта / (узлов * выполненных тактов), нс на узел за такт.
	/// @param[in] c      замер шага
	/// @param[in] line1  строка-разделитель
	/// @return Текст раздела.
	CString StepReport( const TmcTimingStep &c, const char *line1 )
	{
		CString r, s;
		const __int64 *q = c.q;
		double a[8];
		Group( c, a );
		double dTot = a[6];
		double dLoopParts = Sec( q[TMCT_EXCITE] + q[TMCT_SCATTER] + q[TMCT_BLOCKNODE] + q[TMCT_FIELDFILE] +
			q[TMCT_DISTRINTEGR] + q[TMCT_STATUS] );

		r += line1;
		s.Format( " STEP %d  (%s), started %02d:%02d:%02d\r\n", c.nStep, StateName( c.nState ),
			c.stStart.wHour, c.stStart.wMinute, c.stStart.wSecond ); r += s;
		r += line1;
		s.Format( " Grid: nX = %d, nY = %d, nodes = %.0f; blocks = %d\r\n", c.nX, c.nY, c.dNodes, c.nBlocks ); r += s;
		s.Format( " Time steps: %d of %d done; dt = %.6g s; delta = %.6g m\r\n", c.nTDone, c.nTMax, c.dT, c.dDelta ); r += s;
		s.Format( " Compute: %s\r\n", c.szCompute[0] ? c.szCompute : "CPU" ); r += s;	// GPU
		// distribution files (.eps: eps, plasma N/B/Y) are optional
		if( ( c.nEpsWritten == 0 )&&( c.nEpsRead == 0 ) ) r += " Distribution files (.eps): not used\r\n";
		else
		{
			s.Format( " Distribution files (.eps): written by %d block(s), read by %d block(s)\r\n", c.nEpsWritten, c.nEpsRead ); r += s;
		}
		r += "\r\n";
		s.Format( "   %-50s %15s %14s  %8s\r\n", "Phase", "h:mm:ss.mmm", "seconds", "% of step" ); r += s;
		r += " A. PREPROCESSOR AND INPUT\r\n";
		r += Row( "A1 Release of data of the previous step", Sec( q[TMCT_MEMFREE] ), dTot );
		r += Row( "A2 Preprocessor: macro expansion of .tpl", Sec( q[TMCT_PREPR] ), dTot );
		r += Row( "A3 Reading #PARM/#TOPOLOGY/#LINK_LIST/#OUTPUT", Sec( q[TMCT_READTPL] ), dTot );
		r += " B. PREPARATION FOR THE CALCULATION\r\n";
		r += Row( "B1 Output files: delete old, create signal file", Sec( q[TMCT_OUTFILEINIT] ), dTot );
		r += Row( "B2 FIELD_DISTRIBUTION: create files", Sec( q[TMCT_DISTRINIT] ), dTot );
		r += Row( "B3 Grid memory: allocation and zeroing", Sec( q[TMCT_MEMINIT] ), dTot );
		r += Row( "B4 Blocks on the grid (eps, metal, ports), total", Sec( q[TMCT_BLOCKGEOM] ), dTot );
		r += Row( "B5   incl. writing eps-distribution file(s)", Sec( q[TMCT_EPSFILE] ), dTot );
		r += Row( "B5a  incl. FILE blocks (distribution from file)", Sec( q[TMCT_EPSREAD] ), dTot );
		r += Row( "B6 eps -> node admittance", Sec( q[TMCT_ADMITTANCE] ), dTot );
		r += Row( "B7 Writing TOPOLOGY file", Sec( q[TMCT_TOPOLOGYFILE] ), dTot );
		r += " C. TIME LOOP\r\n";
		r += Row( "C1 Port excitation (calculation)", Sec( q[TMCT_EXCITE] - q[TMCT_SIGNALFILE] ), dTot );
		r += Row( "C2 Scattering in the grid nodes (calculation)", Sec( q[TMCT_SCATTER] ), dTot );
		r += Row( "C3 Block nodes: metal/absorber/ports (calc.)", Sec( q[TMCT_BLOCKNODE] ), dTot );
		r += Row( "C4 Writing nT and port signals to file (disk)", Sec( q[TMCT_SIGNALFILE] ), dTot );
		r += Row( "C5 Writing FIELDS file (disk)", Sec( q[TMCT_FIELDFILE] ), dTot );
		r += Row( "C6 FIELD_DISTRIBUTION accumulation", Sec( q[TMCT_DISTRINTEGR] ), dTot );
		r += Row( "C7 Status bar updates", Sec( q[TMCT_STATUS] ), dTot );
		r += Row( "C8 Loop overhead (not attributed)", Sec( q[TMCT_LOOPWALL] ) - dLoopParts, dTot );
		r += Row( "   Time loop, total (wall)", Sec( q[TMCT_LOOPWALL] ), dTot );
		r += " D. POSTPROCESSOR\r\n";
		r += Row( "D1 FIELD_DISTRIBUTION: .AMP/.FAZ/.DAT, dir. pattern", Sec( q[TMCT_DISTRFINAL] ), dTot );
		r += Row( "D2 S-matrix (.s)", Sec( q[TMCT_SMATRIX] ), dTot );
		r += " E. OTHER\r\n";
		r += Row( "E1 Sound signal at the end of the step", Sec( q[TMCT_SOUND] ), dTot );
		r += "\r\n";
		s.Format( " PURE CALCULATION TIME (C1+C2+C3) ...... %s\r\n", (LPCTSTR)SecStr( a[3] ) ); r += s;
		s.Format( " Disk writes (B1+B2+B5+B7+C4+C5) ....... %s\r\n", (LPCTSTR)SecStr( a[4] ) ); r += s;
		s.Format( " TOTAL STEP TIME (A+B+C+D+E) ........... %s\r\n", (LPCTSTR)SecStr( a[6] ) ); r += s;
		s.Format( " RunStep wall time (B+C+D+E, check) .... %s; not attributed %.6f s\r\n", (LPCTSTR)SecStr( Sec( c.qRunStep ) ),
			Sec( c.qRunStep ) - ( a[1] + a[2] + a[5] + a[7] ) ); r += s;
		if( ( c.nTDone > 0 )&&( c.dNodes > 0. )&&( a[3] > 0. ) )
		{
			double dN = c.dNodes*c.nTDone;
			s.Format( " Speed: %.3f ns per node per time step; %.2f million node-updates per second\r\n",
				1.e9*a[3]/dN, dN/a[3]*1.e-6 ); r += s;
		}
		if( dTot > 0. )
		{
			s.Format( " Share of pure calculation in the step: %.2f %%\r\n", 100.*a[3]/dTot ); r += s;
		}
		r += "\r\n";
		return r;
	}

	// h:mm:ss.mmm
	/// @brief Форматирует время в виде h:mm:ss.mmm (с округлением до мс, знак минус сохраняется).
	/// @param[in] d  время, с
	/// @return Строка.
	static CString Hms( double d )
	{
		CString s;
		BOOL bNeg = ( d < 0. );
		if( bNeg ) d = -d;
		__int64 ms = (__int64)( d*1000. + 0.5 );
		s.Format( "%s%I64d:%02d:%02d.%03d", bNeg ? "-" : "", ms/3600000, (int)((ms/60000)%60), (int)((ms/1000)%60), (int)(ms%1000) );
		return s;
	}

	/// @brief Форматирует время как «h:mm:ss.mmm  (N.NNNNNN s)».
	/// @param[in] d  время, с
	/// @return Строка.
	static CString SecStr( double d )
	{
		CString s;
		s.Format( "%s  (%.6f s)", (LPCTSTR)Hms( d ), d );
		return s;
	}

	/// @brief Возвращает строки отчёта о компьютере: имя, процессор (из реестра), число логических процессоров, память.
	/// @return Текст с CRLF.
	static CString MachineInfo( void )
	{
		CString r, s;
		char szName[256];
		DWORD n = sizeof(szName);
		if( !GetComputerNameA( szName, &n ) ) strcpy( szName, "?" );
		char szCpu[256] = "?";
		HKEY hKey;
		if( RegOpenKeyExA( HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
		{
			DWORD dwType = 0, dwSize = sizeof(szCpu) - 1;
			if( RegQueryValueExA( hKey, "ProcessorNameString", NULL, &dwType, (LPBYTE)szCpu, &dwSize ) != ERROR_SUCCESS ) strcpy( szCpu, "?" );
			szCpu[sizeof(szCpu) - 1] = '\0';
			RegCloseKey( hKey );
		}
		SYSTEM_INFO si;
		GetSystemInfo( &si );
		MEMORYSTATUSEX ms;
		memset( &ms, 0, sizeof(ms) );
		ms.dwLength = sizeof(ms);
		GlobalMemoryStatusEx( &ms );
		s.Format( "Computer           : %s\r\n", szName ); r += s;
		s.Format( "CPU                : %s; logical processors: %u\r\n", szCpu, (unsigned)si.dwNumberOfProcessors ); r += s;
		s.Format( "RAM                : %.1f GB installed, %.1f GB free at report time\r\n",
			ms.ullTotalPhys/1073741824., ms.ullAvailPhys/1073741824. ); r += s;
		return r;
	}
};

#pragma warning(pop)

#endif
