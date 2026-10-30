// TmcGpuH.h - the time loop of the H-mode kernel on a video card (OpenCL 1.2, double).
//
// On the card: the node update of the whole grid (RunScatteringNode /
// RunScatteringNode1) and the METAL, ABSORBER, MAGNETIC blocks (RunBlockNode /
// RunBlockNode1). The formulas are the same, in the same order, FP_CONTRACT OFF:
// the result is bit-identical to the CPU calculation.
// On the CPU (original code): ports (ExciteInputs) and all file output. Before a
// port is excited the nodes around it are copied from the card and back after it
// (one small transfer each way). FIELDS output and the integration window of the
// directional pattern copy the whole grid from the card.
//
// Switched on by the key /GPU (/GPU:n - card number n) or TMC_GPU=1 (TMC_GPU=n+1).
// Only win64: RTX 50 cards have no 32-bit OpenCL at all.
// Arrays on the card are per component (u0, u1, y0, y1): one OpenCL buffer may
// be at most 1/4 of the card memory, so the interleaved CPU array would not fit.

/// @file TmcGpuH.h
/// @brief Цикл по времени ядра H на видеокарте (OpenCL 1.2, `double`): класс `CTmcGpuH`.
///
/// Часть модуля «Расчёт на видеокарте», включается только в `tmcrthrectnode.cpp`
/// ядра H (один статический объект `g_cGpuH`). На карту переносятся обновление всех
/// внутренних узлов сетки (аналог `RunScatteringNode` / `RunScatteringNode1`) и узлы
/// блоков METAL, ABSORBER, MAGNETIC (аналог `RunBlockNode` / `RunBlockNode1`).
/// Порты, запись сигналов и все выходные файлы остаются на процессоре.
/// Формулы и порядок операций повторяют процессорный код, слияние умножения и
/// сложения запрещено (`FP_CONTRACT OFF`), поэтому результат совпадает с расчётом на
/// процессоре бит в бит.
/// @see https://www.tamic.ru/docs/code/gpu.html#kernels-h
/// @see https://www.tamic.ru/docs/theory/gpu.html#mapping

#ifndef _TMC_GPU_H_H
#define _TMC_GPU_H_H

#include "TmcOcl.h"
#include <algorithm>

// below this number of nodes /GPU stays on the CPU (measured, see research/gpu); TMC_GPU_MIN_NODES overrides
/// Порог числа узлов сетки ядра H для расчёта на карте: при `nX*nY` меньше порога шаг
/// считается на процессоре, даже если карта запрошена (на малой сетке карта проигрывает
/// из-за обмена с портами на каждом подтакте). Переменная окружения `TMC_GPU_MIN_NODES`
/// заменяет значение во время работы (0 — карта всегда).
/// @see https://www.tamic.ru/docs/code/gpu.html#auto
#ifndef TMC_GPU_H_MIN_NODES
#define TMC_GPU_H_MIN_NODES	150000	// RTX 5070 Ti, 22.10.2026: GPU wins from 90..160 thousand nodes (threshold.csv)
#endif

/// @brief Исходный текст программы OpenCL C ядра H (компилируется драйвером в `Begin`).
///
/// Сетка на карте хранится покомпонентно: `u0`, `u1` — две компоненты напряжения узла
/// (на процессоре — `prUNode1[2*n]`, `prUNode1[2*n+1]`), `y0`, `y1` — две проводимости
/// узла (`prYNode[2*n]`, `prYNode[2*n+1]`); `n = y*nX + x`. Функции программы:
/// - `hpass` — обновление внутренних узлов за подтакт: пишет компоненту `d` по
///   компоненте `s` четырёх соседей и самого узла; соответствует
///   `RunScatteringNode1line` (d = u0) и `RunScatteringNode1line1` (d = u1);
///   сетка запусков двумерная, рабочий элемент = узел (x, y), края сетки не трогаются;
/// - `hmetal` — METAL: записанная компонента узлов блока обнуляется;
/// - `habs` — ABSORBER и MAGNETIC: те же формулы по типу узла `typ`, что в
///   `RunBlockNode` / `RunBlockNode1`, с проверкой границ массива как на процессоре;
///   тип -1 — узел ничего не пишет (повторная копия узла в блоке), обновляется только
///   его прошлое значение `ru`;
/// - `hgather` / `hscatter` — упаковка узлов окна вокруг портов в буфер `pk` и обратно.
/// Порядок сложений в каждой формуле совпадает с процессорным, `FP_CONTRACT OFF`.
/// @warning Строка — код, который компилирует драйвер: любое изменение формулы или
/// порядка операций нарушит совпадение с процессором бит в бит.
/// @see https://www.tamic.ru/docs/code/gpu.html#kernels-h
static const char *g_szTmcGpuHSrc =
"#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
"#pragma OPENCL FP_CONTRACT OFF\n"
// RunScatteringNode1line / 1line1: d - written component, s - component read at the neighbours
"__kernel void hpass( __global double *d, __global const double *s,\n"
"	__global const double *y0, __global const double *y1, int nX, int nY )\n"
"{\n"
"	int x = (int)get_global_id(0) + 1;\n"
"	int y = (int)get_global_id(1) + 1;\n"
"	if( x > nX - 2 || y > nY - 2 ) return;\n"
"	long n = (long)y*nX + x;\n"
"	d[n] = y1[n]*( s[n-1] + s[n+1] + s[n-nX] + s[n+nX] + s[n]*y0[n] ) - d[n];\n"
"}\n"
// RunBlockNode / RunBlockNode1, METAL
"__kernel void hmetal( __global double *w, __global const long *idx, long nOff, long nNum )\n"
"{\n"
"	long i = get_global_id(0);\n"
"	if( i >= nNum ) return;\n"
"	w[ idx[nOff + i] ] = 0.0;\n"
"}\n"
// RunBlockNode / RunBlockNode1, ABSORBER and MAGNETIC: w - written component, o - the other one
"__kernel void habs( __global double *w, __global const double *o, __global const double *y0,\n"
"	__global const long *idx, __global const int *typ, __global const double *ry, __global double *ru,\n"
"	long nOff, long nNum, long nX, long nArr )\n"
"{\n"
"	long i = get_global_id(0);\n"
"	if( i >= nNum ) return;\n"
"	i += nOff;\n"
"	long n = idx[i];\n"
"	if( ((2*(n -  1) + 1) > 0)&&((2*(n -  1) + 1) < nArr)&&\n"
"		((2*(n + nX) + 1) > 0)&&((2*(n + nX) + 1) < nArr)&&\n"
"		((2*(n +  1) + 1) > 0)&&((2*(n +  1) + 1) < nArr)&&\n"
"		((2*(n - nX) + 1) > 0)&&((2*(n - nX) + 1) < nArr)&&\n"
"		((2*n + 1) > 0)&&((2*n + 1) < nArr) )\n"
"	{\n"
"		double u1 = o[n - 1], u2 = o[n + nX], u3 = o[n + 1], u4 = o[n - nX];\n"
"		double ut1 = o[n], ut2 = ru[i], y = ry[i], y5 = y0[n];\n"
"		switch( typ[i] )\n"
"		{\n"
"		case 16: w[n] = 0.0; break;\n"
"		case 3:  y5 *= 0.75; w[n] = (2./(3+fabs(y5)+y/2))*(u3+u4+0.5*(u2+u1)+y5*ut1+y/2*ut2) - ut2; break;\n"
"		case 6:  y5 *= 0.75; w[n] = (2./(3+fabs(y5)+y/2))*(u1+u4+0.5*(u2+u3)+y5*ut1+y/2*ut2) - ut2; break;\n"
"		case 7:  y5 *= 0.5;  w[n] = (2./(2+fabs(y5)+y))*(u4+0.5*(u1+u3)+y5*ut1+y*ut2) - ut2; break;\n"
"		case 9:  y5 *= 0.75; w[n] = (2./(3+fabs(y5)+y/2))*(u3+u2+0.5*(u1+u4)+y5*ut1+y/2*ut2) - ut2; break;\n"
"		case 11: y5 *= 0.5;  w[n] = (2./(2+fabs(y5)+y))*(u3+0.5*(u2+u4)+y5*ut1+y*ut2) - ut2; break;\n"
"		case 12: y5 *= 0.75; w[n] = (2./(3+fabs(y5)+y/2))*(u1+u2+0.5*(u4+u3)+y5*ut1+y/2*ut2) - ut2; break;\n"
"		case 13: y5 *= 0.5;  w[n] = (2./(2+fabs(y5)+y))*(u2+0.5*(u1+u3)+y5*ut1+y*ut2) - ut2; break;\n"
"		case 14: y5 *= 0.5;  w[n] = (2./(2+fabs(y5)+y))*(u1+0.5*(u2+u4)+y5*ut1+y*ut2) - ut2; break;\n"
"		case 15: w[n] = 0.0; break;\n"
"		case 21: y5 *= 0.25; w[n] = (2./(1+fabs(y5)+1.5*y))*(0.5*(u1+u2)+y5*ut1+1.5*y*ut2) - ut2; break;\n"
"		case 22: y5 *= 0.25; w[n] = (2./(1+fabs(y5)+1.5*y))*(0.5*(u2+u3)+y5*ut1+1.5*y*ut2) - ut2; break;\n"
"		case 23: y5 *= 0.25; w[n] = (2./(1+fabs(y5)+1.5*y))*(0.5*(u3+u4)+y5*ut1+1.5*y*ut2) - ut2; break;\n"
"		case 24: y5 *= 0.25; w[n] = (2./(1+fabs(y5)+1.5*y))*(0.5*(u1+u4)+y5*ut1+1.5*y*ut2) - ut2; break;\n"
"		default: break;\n"
"		}\n"
"	}\n"
"	ru[i] = o[n];\n"
"}\n"
// nodes around the ports: card <-> CPU
"__kernel void hgather( __global const double *u0, __global const double *u1,\n"
"	__global const long *idx, __global double *pk, long nNum )\n"
"{\n"
"	long i = get_global_id(0);\n"
"	if( i >= nNum ) return;\n"
"	long n = idx[i];\n"
"	pk[2*i] = u0[n]; pk[2*i + 1] = u1[n];\n"
"}\n"
"__kernel void hscatter( __global double *u0, __global double *u1,\n"
"	__global const long *idx, __global const double *pk, long nNum )\n"
"{\n"
"	long i = get_global_id(0);\n"
"	if( i >= nNum ) return;\n"
"	long n = idx[i];\n"
"	u0[n] = pk[2*i]; u1[n] = pk[2*i + 1];\n"
"}\n";

/// @brief Сеанс расчёта ядра H на видеокарте: буферы сетки на карте и запуск ядер OpenCL.
///
/// Один статический объект `g_cGpuH` в `tmcrthrectnode.cpp`. На каждом шаге задания
/// `CTmcRTHRectNode::RunKernel` вызывает `Begin` (решение «карта или процессор» и
/// загрузка сетки); при успехе цикл по времени идёт через `RunKernel1TGpu`, который на
/// каждом подтакте вызывает `PortsDown` -> возбуждение портов на процессоре -> `PortsUp` ->
/// `Pass` -> `Blocks` и при выводе поля `Download`; по окончании шага или при остановке —
/// `End`, возвращающий сетку и состояния поглотителей в массивы процессора.
/// Контекст OpenCL и скомпилированная программа живут между шагами (пересоздаются
/// только при смене карты), буферы сетки — от `Begin` до `End`.
/// Ошибки OpenCL не бросают исключений: методы возвращают `false`, текст — в `csError`.
/// @note Потоки: все методы вызываются из расчётного потока ядра; очередь команд одна
/// и выполняется по порядку, синхронизация — блокирующим чтением в `PortsDown` и `Download`.
/// @see https://www.tamic.ru/docs/code/gpu.html#loop
class CTmcGpuH
{
public:
	/// @brief Создаёт выключенный сеанс без ресурсов OpenCL (OpenCL.dll ещё не загружена).
	CTmcGpuH() : bOn( false ), ctx( NULL ), idDev( NULL ), q( NULL ), prg( NULL ), kPass( NULL ), kMetal( NULL ), kAbs( NULL ),
		kGather( NULL ), kScatter( NULL ), mU0( NULL ), mU1( NULL ), mY0( NULL ), mY1( NULL ), mBIdx( NULL ),
		mBTyp( NULL ), mBRy( NULL ), mBRu( NULL ), mPIdx( NULL ), mPPk( NULL ), nX( 0 ), nY( 0 ), nN( 0 ), nPort( 0 ) {}
	/// @brief Ничего не освобождает: ресурсы карты освобождает завершение процесса
	/// (вызовы OpenCL из деструкторов статических объектов небезопасны).
	~CTmcGpuH() {}	// the end of the process frees the card (no OpenCL calls from static destructors)

	/// Итог решения `Begin` для отчёта о времени (строка `Compute:`) и строки состояния:
	/// «GPU n: имя карты (версия OpenCL, driver версия, память GB)» или «CPU (причина)»;
	/// просто «CPU» — карта не запрашивалась. Текст по-английски.
	CString csInfo;		// "GPU 0: <name> ..." or why the CPU is used
	/// Первая ошибка OpenCL во время цикла (например, «GPU node update: CL_OUT_OF_RESOURCES»);
	/// пусто — ошибок не было. Вызывающий код передаёт её в `cError` ядра.
	CString csError;	// error during the loop

	/// @brief Сообщает, находится ли сетка сейчас на карте.
	/// @return `true` — между успешным `Begin` и `End`.
	bool IsOn( void ) { return bOn; }

	// /GPU, /GPU:n (a separate word, not inside "quotes") or TMC_GPU=1..n+1
	// T-13: /CPU (a separate word, not inside "quotes") - the card is not used, whatever /GPU or TMC_GPU say
	/// @brief Проверяет, есть ли в командной строке процесса ключ `/CPU` (или `-CPU`).
	///
	/// Ключ должен быть отдельным словом вне кавычек (слово внутри пути к файлу не
	/// считается), регистр букв не важен. `/CPU` сильнее `/GPU` и переменной `TMC_GPU`.
	/// @return `true` — ключ `/CPU` задан, расчёт только на процессоре.
	/// @see https://www.tamic.ru/docs/code/gpu.html#device-choice
	static bool IsCpuKey( void )
	{
		const char *c = GetCommandLineA();
		bool bQuote = false;
		for( int i = 0; c[i] != '\0'; i++ )
		{
			if( c[i] == '"' ) { bQuote = !bQuote; continue; }
			if( bQuote ) continue;
			if( ( c[i] != '/' )&&( c[i] != '-' ) ) continue;
			if( ( i > 0 )&&( c[i-1] != ' ' )&&( c[i-1] != '\t' ) ) continue;
			if( _strnicmp( c + i + 1, "CPU", 3 ) != 0 ) continue;
			char e = c[i+4];
			if( ( e == '\0' )||( e == ' ' )||( e == '\t' ) ) return true;
		}
		return false;
	}

	/// @brief Определяет, запрошен ли расчёт на карте, и номер карты.
	///
	/// Порядок: ключ `/CPU` -> не запрошен; переменная окружения `TMC_GPU` > 0 ->
	/// запрошен, карта `TMC_GPU - 1`; иначе ключ `/GPU` (карта 0), `/GPU:n` или `/GPU=n`
	/// (карта n; допускается и `-GPU`), отдельным словом вне кавычек. Переменную
	/// `TMC_GPU` ставит оболочка ядра по настройке окна «Compute» до начала расчёта.
	/// @param[out] pnDev  номер карты среди карт с двойной точностью, с 0; 0, если не запрошен
	/// @return `true` — расчёт на карте запрошен (решение ещё зависит от порога и памяти).
	/// @see https://www.tamic.ru/docs/code/gpu.html#device-choice
	static bool IsRequested( int *pnDev )
	{
		*pnDev = 0;
		if( IsCpuKey() ) return false;	// T-13: /CPU is stronger than /GPU
		const char *psz = getenv( "TMC_GPU" );
		if( ( psz != NULL )&&( atoi( psz ) > 0 ) ) { *pnDev = atoi( psz ) - 1; return true; }
		const char *c = GetCommandLineA();
		bool bQuote = false;
		for( int i = 0; c[i] != '\0'; i++ )
		{
			if( c[i] == '"' ) { bQuote = !bQuote; continue; }
			if( bQuote ) continue;
			if( ( c[i] != '/' )&&( c[i] != '-' ) ) continue;
			if( ( i > 0 )&&( c[i-1] != ' ' )&&( c[i-1] != '\t' ) ) continue;
			if( _strnicmp( c + i + 1, "GPU", 3 ) != 0 ) continue;
			char e = c[i+4];
			if( ( e == '\0' )||( e == ' ' )||( e == '\t' ) ) return true;
			if( ( e == ':' )||( e == '=' ) ) { *pnDev = atoi( c + i + 5 ); return true; }
		}
		return false;
	}

	// upload the step to the card; false - the loop stays on the CPU (csInfo says why)
	/// @brief Решает, считать ли шаг на карте, и при согласии загружает на неё сетку и блоки.
	///
	/// Последовательно проверяет: запрос (`IsRequested`), 64-битную сборку, порог числа
	/// узлов (`TMC_GPU_H_MIN_NODES` или `TMC_GPU_MIN_NODES`), наличие OpenCL.dll, карты с
	/// двойной точностью и карты с запрошенным номером; затем собирает блоки METAL,
	/// ABSORBER, MAGNETIC в общий список узлов, окна узлов вокруг портов и проверяет память
	/// карты. Любой отказ оставляет шаг на процессоре: метод возвращает `false`, причина —
	/// в `csInfo`, расчёт процессором даёт тот же результат.
	/// Блок с узлом вне сетки, блок без массива узлов и узел поглотителя недопустимого
	/// типа (процессор на нём остановится с ошибкой) тоже оставляют шаг на процессоре —
	/// чтобы ошибку выдал процессорный код.
	/// @param[in] prU    массив напряжений сетки процессора, 2 значения на узел, длина 2*nX*nY
	/// @param[in] prY    массив проводимостей узлов процессора, 2 значения на узел
	/// @param[in] nX1    число узлов сетки по X
	/// @param[in] nY1    число узлов сетки по Y
	/// @param[in] pDiel  массив блоков шага (`CTmcRTHNodeDiel`), читается через `GpuView`
	/// @param[in] nDiel  число блоков в `pDiel`
	/// @return `true` — сетка на карте, цикл идёт через `RunKernel1TGpu`; `false` — шаг на процессоре.
	/// @note Ошибка памяти при создании буферов тоже даёт `false` (не ошибку расчёта).
	/// @see https://www.tamic.ru/docs/code/gpu.html#auto
	bool Begin( _ELEM_VAL_RTH *prU, _real *prY, int nX1, int nY1, CTmcRTHNodeDiel *pDiel, int nDiel )
	{
		Release();
		csError = "";
		int nDev = 0;
		if( !IsRequested( &nDev ) ) { csInfo = IsCpuKey() ? "CPU (key /CPU)" : "CPU"; return false; }	// T-13
#ifndef _WIN64
		csInfo = "CPU (GPU needs the win64 build)";
		return false;
#else
		nX = nX1; nY = nY1; nN = (TmcIdx)nX*nY;
		// small grids: the card waits for the ports every pass and loses to the CPU
		{
			TmcIdx nMin = TMC_GPU_H_MIN_NODES;
			const char *pszMin = getenv( "TMC_GPU_MIN_NODES" );
			if( pszMin != NULL ) nMin = (TmcIdx)_atoi64( pszMin );
			if( nN < nMin )
			{
				csInfo.Format( "CPU (grid of %I64d nodes is below the GPU threshold %I64d; TMC_GPU_MIN_NODES=0 forces the card)", (__int64)nN, (__int64)nMin );
				return false;
			}
		}
		// выбор карты: только карты с double, номер nDev - среди них (как в окне Compute)
		if( !ocl.Load() ) { csInfo = "CPU (GPU requested, but OpenCL.dll was not found)"; return false; }
		std::vector<TmcOclDevice> v = TmcOclListGpus( ocl ), vd;
		for( size_t k = 0; k < v.size(); k++ ) if( v[k].bFP64 ) vd.push_back( v[k] );
		if( vd.empty() ) { csInfo = "CPU (GPU requested, but no card with double precision)"; return false; }
		if( ( nDev < 0 )||( nDev >= (int)vd.size() ) )	// T-12: not the card 0 silently
		{
			csInfo.Format( "CPU (GPU card %d not found: %d card(s) with double precision, numbered from 0)", nDev, (int)vd.size() );
			return false;
		}
		dev = vd[nDev];

		// blocks run on the card; anything the CPU code would stop on stays on the CPU
		// Сбор блоков: узлы METAL/ABSORBER/MAGNETIC всех блоков подряд в vIdx (сегмент
		// на блок, в порядке блоков задания), порты - в окна vPort; блоки EPS не нужны
		// (их проводимости уже в prY).
		std::vector<cl_long> vIdx, vPort;
		std::vector<cl_int> vTyp;
		std::vector<double> vRy, vRu;
		vSeg.clear();
		for( int b = 0; b < nDiel; b++ )
		{
			int nT; TmcIdx nNum; sTmcRTHNodeDielOne *p;
			pDiel[b].GpuView( nT, nNum, p );
			if( nT == CTMCRTH_BLCKNTYPE_EPS ) continue;
			if( p == NULL ) { csInfo = "CPU (a block has no nodes)"; return false; }
			if( ( nT == CTMCRTH_BLCKNTYPE_INPXLEFT )||( nT == CTMCRTH_BLCKNTYPE_INPXRIGHT )||
				( nT == CTMCRTH_BLCKNTYPE_INPYTOP )||( nT == CTMCRTH_BLCKNTYPE_INPYBOT ) )
			{
				for( TmcIdx i = 0; i < nNum; i++ ) AddWindow( vPort, p[i].nNodeGlobal );
				continue;
			}
			if( ( nT != CTMCRTH_BLCKNTYPE_METAL )&&( nT != CTMCRTH_BLCKNTYPE_ABSORBER )&&( nT != CTMCRTH_BLCKNTYPE_MAGNETIC ) ) continue;
			if( nNum <= 0 ) continue;
			Seg s;
			s.nBlock = b; s.bMetal = ( nT == CTMCRTH_BLCKNTYPE_METAL ); s.nOff = (TmcIdx)vIdx.size(); s.nNum = nNum;
			std::vector< std::pair<cl_long, TmcIdx> > vChk;
			for( TmcIdx i = 0; i < nNum; i++ )
			{
				TmcIdx n = p[i].nNodeGlobal;
				if( ( n < 0 )||( n >= nN ) ) { csInfo = "CPU (a block node is outside the grid)"; return false; }
				vIdx.push_back( n ); vTyp.push_back( p[i].nType ); vRy.push_back( p[i].rY ); vRu.push_back( p[i].rU );
				vChk.push_back( std::make_pair( (cl_long)n, i ) );
				if( !s.bMetal && IsBadAbsorber( p[i].nType ) && InBounds( n ) )
				{ csInfo = "CPU (bad absorber node type - the CPU reports it)"; return false; }
			}
			// a node twice in a block: every copy reads the same values (the other component),
			// the CPU keeps what the LAST copy wrote; earlier copies only update their rU
			std::sort( vChk.begin(), vChk.end() );
			for( size_t k = 0; k + 1 < vChk.size(); k++ )
				if( vChk[k].first == vChk[k + 1].first ) vTyp[ s.nOff + vChk[k].second ] = -1;
			vSeg.push_back( s );
		}
		if( !vPort.empty() ) AddWindow( vPort, 0 );	// ExciteInputs reads node 0 at the ends of a port
		std::sort( vPort.begin(), vPort.end() );
		vPort.erase( std::unique( vPort.begin(), vPort.end() ), vPort.end() );
		nPort = (TmcIdx)vPort.size();

		// memory
		// 32 байта на узел (u0, u1, y0, y1 по 8 байт) + узлы блоков + окна портов;
		// один буфер компоненты (8*nN) не больше nMaxAlloc, всё вместе - не больше 92 % памяти
		double dNeed = 32.0*nN + 28.0*vIdx.size() + 24.0*nPort;	// J-4: 28 bytes per block node (old: 20)
		if( 8.0*nN > (double)dev.nMaxAlloc || dNeed > 0.92*dev.nGlobalMem )
		{
			csInfo.Format( "CPU (the grid needs %.2f GB, card %s has %.2f GB)", dNeed/1073741824.0, dev.sName.c_str(), dev.nGlobalMem/1073741824.0 );
			return false;
		}

		cl_int e = CL_SUCCESS;
		if( ( ctx == NULL )||( idDev != dev.idDevice ) )	// the context and the compiled kernels live between the steps
		{
			ReleaseAll();
			cl_context_properties pr[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)dev.idPlatform, 0 };
			ctx = ocl.clCreateContext( pr, 1, &dev.idDevice, NULL, NULL, &e );
			if( e == CL_SUCCESS ) q = ocl.clCreateCommandQueue( ctx, dev.idDevice, 0, &e );
			if( e == CL_SUCCESS ) prg = ocl.clCreateProgramWithSource( ctx, 1, &g_szTmcGpuHSrc, NULL, &e );
			if( e == CL_SUCCESS ) e = ocl.clBuildProgram( prg, 1, &dev.idDevice, "", NULL, NULL );
			if( e != CL_SUCCESS ) { csInfo.Format( "CPU (OpenCL start on %s failed: %s)", dev.sName.c_str(), TmcOclErr( e ) ); ReleaseAll(); return false; }
			// J-4: every clCreateKernel is checked (old: the result was not checked, a NULL kernel ran on)
			cl_int eK[5];
			kPass = ocl.clCreateKernel( prg, "hpass", &eK[0] );
			kMetal = ocl.clCreateKernel( prg, "hmetal", &eK[1] );
			kAbs = ocl.clCreateKernel( prg, "habs", &eK[2] );
			kGather = ocl.clCreateKernel( prg, "hgather", &eK[3] );
			kScatter = ocl.clCreateKernel( prg, "hscatter", &eK[4] );
			for( int iK = 0; iK < 5; iK++ ) if( eK[iK] != CL_SUCCESS ) { e = eK[iK]; break; }
			if( e != CL_SUCCESS ) { csInfo.Format( "CPU (OpenCL start on %s failed: %s)", dev.sName.c_str(), TmcOclErr( e ) ); ReleaseAll(); return false; }
			idDev = dev.idDevice;
		}

		// буферы шага; сетка процессора (2 значения на узел подряд) раскладывается по компонентам
		mU0 = Buf( 8*nN ); mU1 = Buf( 8*nN ); mY0 = Buf( 8*nN ); mY1 = Buf( 8*nN );
		if( !vIdx.empty() )
		{
			mBIdx = Buf( 8*vIdx.size(), &vIdx[0] ); mBTyp = Buf( 4*vTyp.size(), &vTyp[0] );
			mBRy = Buf( 8*vRy.size(), &vRy[0] ); mBRu = Buf( 8*vRu.size(), &vRu[0] );
		}
		if( nPort > 0 ) { mPIdx = Buf( 8*nPort, &vPort[0] ); mPPk = Buf( 16*nPort ); vPk.resize( 2*nPort ); vPortIdx = vPort; }
		if( csError.IsEmpty() ) Upload( prU, mU0, mU1 );
		if( csError.IsEmpty() ) Upload( prY, mY0, mY1 );
		if( csError.IsEmpty() ) { e = ocl.clFinish( q ); if( e != CL_SUCCESS ) csError.Format( "upload: %s", TmcOclErr( e ) ); }
		if( !csError.IsEmpty() )
		{
			csInfo.Format( "CPU (memory on %s: %s)", dev.sName.c_str(), (LPCTSTR)csError );
			csError = "";
			Release();
			return false;
		}
		csInfo.Format( "GPU %d: %s (%s, driver %s, %.1f GB)", nDev, dev.sName.c_str(), dev.sVersion.c_str(), dev.sDriver.c_str(), dev.nGlobalMem/1073741824.0 );
		bOn = true;
		return true;
#endif
	}

	// grid and absorber states back into the CPU arrays, card released
	/// @brief Возвращает сетку и прошлые значения узлов поглотителей в массивы процессора и освобождает буферы.
	///
	/// Вызывается по окончании цикла шага, при остановке пользователем и при ошибке.
	/// После `End` процессорные массивы такие же, какими их оставил бы расчёт на
	/// процессоре: по ним считаются S-матрица, следующий шаг или продолжение после «Stop».
	/// Без успешного `Begin` ничего не делает. Контекст и программа OpenCL сохраняются.
	/// @param[out] prU    массив напряжений сетки процессора, 2 значения на узел
	/// @param[in,out] pDiel  блоки шага (те же, что в `Begin`): в узлы ABSORBER/MAGNETIC пишется `rU`
	/// @note Ошибка чтения записывается в `csError`; вызывающий код проверяет её после `End`.
	void End( _ELEM_VAL_RTH *prU, CTmcRTHNodeDiel *pDiel )
	{
		if( !bOn ) return;
		if( csError.IsEmpty() ) Download( prU );
		if( csError.IsEmpty() && ( mBRu != NULL ) )
		{
			std::vector<double> vRu( Total() );
			cl_int e = ocl.clEnqueueReadBuffer( q, mBRu, CL_TRUE, 0, 8*vRu.size(), &vRu[0], 0, NULL, NULL );
			if( e == CL_SUCCESS )
				for( size_t k = 0; k < vSeg.size(); k++ )
				{
					int nT; TmcIdx nNum; sTmcRTHNodeDielOne *p;
					pDiel[ vSeg[k].nBlock ].GpuView( nT, nNum, p );
					if( !vSeg[k].bMetal ) for( TmcIdx i = 0; i < nNum; i++ ) p[i].rU = vRu[ vSeg[k].nOff + i ];
				}
			else csError.Format( "read of absorber states: %s", TmcOclErr( e ) );
		}
		Release();
	}

	// node update of the grid: nHalf 0 - RunScatteringNode, 1 - RunScatteringNode1
	/// @brief Ставит в очередь обновление всех внутренних узлов сетки за один подтакт (ядро `hpass`).
	///
	/// Подтакт 0 пишет компоненту u0 по u1 соседей, подтакт 1 — u1 по u0. Узлы
	/// независимы: ни один рабочий элемент не читает то, что пишет другой. Группа
	/// 64 x 4 узла, глобальный размер округлён вверх до кратного группе; лишние
	/// элементы за краем сетки сразу выходят.
	/// @param[in] nHalf  номер подтакта: 0 — как `RunScatteringNode`, 1 — как `RunScatteringNode1`
	/// @return `true` — команда поставлена в очередь; `false` — ошибка, текст в `csError`.
	/// @note Не ждёт окончания счёта на карте.
	/// @see https://www.tamic.ru/docs/theory/gpu.html#mapping
	bool Pass( int nHalf )
	{
		cl_mem d = nHalf ? mU1 : mU0, s = nHalf ? mU0 : mU1;
		Arg( kPass, 0, d ); Arg( kPass, 1, s ); Arg( kPass, 2, mY0 ); Arg( kPass, 3, mY1 );
		ocl.clSetKernelArg( kPass, 4, sizeof(int), &nX ); ocl.clSetKernelArg( kPass, 5, sizeof(int), &nY );
		size_t lc[2] = { 64, 4 };
		size_t gl[2] = { ( (size_t)( nX - 2 ) + 63 )/64*64, ( (size_t)( nY - 2 ) + 3 )/4*4 };
		return Check( ocl.clEnqueueNDRangeKernel( q, kPass, 2, NULL, gl, lc, 0, NULL, NULL ), "node update" );
	}

	// METAL / ABSORBER / MAGNETIC blocks in the CPU order: nHalf 0 - RunBlockNode, 1 - RunBlockNode1
	/// @brief Ставит в очередь обработку узлов блоков METAL, ABSORBER, MAGNETIC за подтакт.
	///
	/// Один запуск (`hmetal` или `habs`) на блок, в порядке блоков задания: очередь
	/// выполняется по порядку, поэтому узел, входящий в два блока, получает значение
	/// последнего блока — как на процессоре. Пишется та же компонента, что в `Pass`.
	/// @param[in] nHalf  номер подтакта: 0 — как `RunBlockNode`, 1 — как `RunBlockNode1`
	/// @return `true` — все запуски поставлены в очередь; `false` — ошибка, текст в `csError`.
	/// @see https://www.tamic.ru/docs/code/gpu.html#blocks
	bool Blocks( int nHalf )
	{
		cl_mem w = nHalf ? mU1 : mU0, o = nHalf ? mU0 : mU1;
		cl_long nXl = nX, nArr = 2*(cl_long)nN;
		for( size_t k = 0; k < vSeg.size(); k++ )
		{
			cl_long nOff = vSeg[k].nOff, nNum = vSeg[k].nNum;
			size_t lc = 64, gl = ( (size_t)nNum + 63 )/64*64;
			cl_int e;
			if( vSeg[k].bMetal )
			{
				Arg( kMetal, 0, w ); Arg( kMetal, 1, mBIdx );
				ocl.clSetKernelArg( kMetal, 2, 8, &nOff ); ocl.clSetKernelArg( kMetal, 3, 8, &nNum );
				e = ocl.clEnqueueNDRangeKernel( q, kMetal, 1, NULL, &gl, &lc, 0, NULL, NULL );
			}
			else
			{
				Arg( kAbs, 0, w ); Arg( kAbs, 1, o ); Arg( kAbs, 2, mY0 ); Arg( kAbs, 3, mBIdx );
				Arg( kAbs, 4, mBTyp ); Arg( kAbs, 5, mBRy ); Arg( kAbs, 6, mBRu );
				ocl.clSetKernelArg( kAbs, 7, 8, &nOff ); ocl.clSetKernelArg( kAbs, 8, 8, &nNum );
				ocl.clSetKernelArg( kAbs, 9, 8, &nXl ); ocl.clSetKernelArg( kAbs, 10, 8, &nArr );
				e = ocl.clEnqueueNDRangeKernel( q, kAbs, 1, NULL, &gl, &lc, 0, NULL, NULL );
			}
			if( !Check( e, "blocks" ) ) return false;
		}
		return true;
	}

	// nodes around the ports: card -> CPU array (before ExciteInputs)
	/// @brief Копирует узлы окон вокруг портов с карты в массив процессора (перед возбуждением портов).
	///
	/// Окно — порт и все узлы в квадрате +-4 узла вокруг него, плюс окно узла 0. Узлы
	/// собираются на карте в буфер `mPPk` (`hgather`) и читаются одним блокирующим
	/// чтением; это чтение заодно дожидается окончания всех ранее поставленных команд
	/// (очередь по порядку) — точка синхронизации подтакта.
	/// @param[in,out] prU  массив напряжений сетки процессора; меняются только узлы окон
	/// @return `true` — успех или портов нет; `false` — ошибка, текст в `csError`.
	/// @see https://www.tamic.ru/docs/code/gpu.html#ports
	bool PortsDown( _ELEM_VAL_RTH *prU )
	{
		if( nPort == 0 ) return true;
		cl_long n = nPort;
		Arg( kGather, 0, mU0 ); Arg( kGather, 1, mU1 ); Arg( kGather, 2, mPIdx ); Arg( kGather, 3, mPPk );
		ocl.clSetKernelArg( kGather, 4, 8, &n );
		size_t lc = 64, gl = ( (size_t)n + 63 )/64*64;
		if( !Check( ocl.clEnqueueNDRangeKernel( q, kGather, 1, NULL, &gl, &lc, 0, NULL, NULL ), "ports" ) ) return false;
		if( !Check( ocl.clEnqueueReadBuffer( q, mPPk, CL_TRUE, 0, 16*(size_t)n, &vPk[0], 0, NULL, NULL ), "ports read" ) ) return false;
		for( TmcIdx i = 0; i < nPort; i++ ) { prU[ 2*vPortIdx[i] ] = vPk[2*i]; prU[ 2*vPortIdx[i] + 1 ] = vPk[2*i + 1]; }
		return true;
	}

	// ... and back to the card (after ExciteInputs)
	/// @brief Возвращает узлы окон вокруг портов из массива процессора на карту (после возбуждения портов).
	///
	/// Запись в `mPPk` неблокирующая; рабочий буфер `vPk` не меняется до следующего
	/// `PortsDown`, который стоит в очереди позже, так что запись успевает завершиться.
	/// @param[in] prU  массив напряжений сетки процессора после `RunExciteInputs`
	/// @return `true` — успех или портов нет; `false` — ошибка, текст в `csError`.
	/// @see https://www.tamic.ru/docs/code/gpu.html#ports
	bool PortsUp( _ELEM_VAL_RTH *prU )
	{
		if( nPort == 0 ) return true;
		for( TmcIdx i = 0; i < nPort; i++ ) { vPk[2*i] = prU[ 2*vPortIdx[i] ]; vPk[2*i + 1] = prU[ 2*vPortIdx[i] + 1 ]; }
		cl_long n = nPort;
		if( !Check( ocl.clEnqueueWriteBuffer( q, mPPk, CL_FALSE, 0, 16*(size_t)n, &vPk[0], 0, NULL, NULL ), "ports write" ) ) return false;
		Arg( kScatter, 0, mU0 ); Arg( kScatter, 1, mU1 ); Arg( kScatter, 2, mPIdx ); Arg( kScatter, 3, mPPk );
		ocl.clSetKernelArg( kScatter, 4, 8, &n );
		size_t lc = 64, gl = ( (size_t)n + 63 )/64*64;
		return Check( ocl.clEnqueueNDRangeKernel( q, kScatter, 1, NULL, &gl, &lc, 0, NULL, NULL ), "ports" );
	}

	// whole grid card -> CPU array (FIELDS, directional pattern window)
	/// @brief Копирует всю сетку с карты в массив процессора (вывод FIELDS, окно ДН, конец шага).
	///
	/// Читает кусками по 4M узлов и чередует компоненты в формат процессора. Дорогая
	/// операция: задания с выводом поля или накоплением ДН ускоряются на карте слабо.
	/// @param[out] prU  массив напряжений сетки процессора, 2 значения на узел
	/// @return `true` — успех; `false` — ошибка, текст в `csError`.
	bool Download( _ELEM_VAL_RTH *prU )
	{
		const TmcIdx nChunk = 4*1048576;
		std::vector<double> a( (size_t)__min( nChunk, nN ) ), b( a.size() );
		for( TmcIdx c = 0; c < nN; c += nChunk )
		{
			TmcIdx k, n = __min( nChunk, nN - c );
			if( !Check( ocl.clEnqueueReadBuffer( q, mU0, CL_FALSE, 8*(size_t)c, 8*(size_t)n, &a[0], 0, NULL, NULL ), "download" ) ) return false;
			if( !Check( ocl.clEnqueueReadBuffer( q, mU1, CL_TRUE, 8*(size_t)c, 8*(size_t)n, &b[0], 0, NULL, NULL ), "download" ) ) return false;
			for( k = 0; k < n; k++ ) { prU[ 2*(c + k) ] = a[k]; prU[ 2*(c + k) + 1 ] = b[k]; }
		}
		return true;
	}

	/// @brief Возвращает имя выбранной карты.
	/// @return Имя карты после `Begin`, дошедшего до выбора карты; иначе пустая строка.
	CString DeviceName( void ) { return CString( dev.sName.c_str() ); }

private:
	/// @brief Сегмент общего списка узлов блоков: узлы одного блока METAL/ABSORBER/MAGNETIC.
	///
	/// Поля: `nBlock` — номер блока в `pDiel`; `bMetal` — блок METAL (иначе ABSORBER или
	/// MAGNETIC); `nOff` — смещение первого узла блока в буферах `mBIdx`, `mBTyp`,
	/// `mBRy`, `mBRu`; `nNum` — число узлов блока.
	struct Seg { int nBlock; bool bMetal; TmcIdx nOff, nNum; };

	/// @var CTmcGpuH::Seg::nBlock
	/// Номер блока в массиве `pDiel`.

	/// @var CTmcGpuH::Seg::bMetal
	/// `true` — блок METAL; `false` — ABSORBER или MAGNETIC.

	/// @var CTmcGpuH::Seg::nOff
	/// Смещение первого узла блока в буферах `mBIdx`, `mBTyp`, `mBRy`, `mBRu`.

	/// @var CTmcGpuH::Seg::nNum
	/// Число узлов блока.

	/// Таблица функций OpenCL этого сеанса.
	TmcOcl ocl;
	/// Выбранная карта (заполняется в `Begin`).
	TmcOclDevice dev;
	/// `true` — сетка на карте (между успешным `Begin` и `End`/`Release`).
	bool bOn;
	/// Контекст OpenCL на выбранной карте; живёт между шагами.
	cl_context ctx;
	/// Карта, для которой создан `ctx`; при смене карты контекст пересоздаётся.
	cl_device_id idDev;	// the device ctx was made for
	/// Очередь команд (выполнение по порядку, без профилирования).
	cl_command_queue q;
	/// Скомпилированная программа `g_szTmcGpuHSrc`.
	cl_program prg;
	/// @name Ядра программы
	///@{
	/// Ядра программы: `kPass` — `hpass`, `kMetal` — `hmetal`, `kAbs` — `habs`,
	/// `kGather` — `hgather`, `kScatter` — `hscatter`.
	cl_kernel kPass, kMetal, kAbs, kGather, kScatter;
	///@}
	/// @name Буферы на карте
	///@{
	/// Буферы на карте (NULL — не созданы): `mU0`, `mU1` — компоненты напряжения узлов,
	/// `mY0`, `mY1` — проводимости узлов (по nN значений `double`); `mBIdx` — номера узлов
	/// всех блоков (`cl_long`), `mBTyp` — типы узлов (`cl_int`, -1 — повторная копия),
	/// `mBRy` — проводимость узла поглотителя `rY`, `mBRu` — прошлое значение `rU`;
	/// `mPIdx` — номера узлов окон портов, `mPPk` — упаковка их значений (2 на узел).
	cl_mem mU0, mU1, mY0, mY1, mBIdx, mBTyp, mBRy, mBRu, mPIdx, mPPk;
	///@}
	/// @name Размеры сетки
	///@{
	/// Размеры, узлов: `nX`, `nY` — сетка по осям.
	int nX, nY;
	/// Размеры, узлов: `nN` = nX*nY — всего; `nPort` — число узлов во всех окнах портов.
	TmcIdx nN, nPort;
	///@}
	/// Сегменты блоков в порядке блоков задания.
	std::vector<Seg> vSeg;
	/// Номера узлов окон портов (по возрастанию, без повторов), копия `mPIdx` на процессоре.
	std::vector<cl_long> vPortIdx;
	/// Рабочий буфер обмена узлами портов, 2 значения на узел.
	std::vector<double> vPk;

	/// @brief Возвращает общее число узлов во всех сегментах блоков.
	/// @return Число узлов (длина буферов `mBIdx` ... `mBRu`); 0 — блоков нет.
	TmcIdx Total( void ) { return vSeg.empty() ? 0 : vSeg.back().nOff + vSeg.back().nNum; }

	/// @brief Проверяет, недопустим ли тип узла поглотителя (на процессоре такой узел — ошибка «Bad Absorber»).
	/// @param[in] t  тип узла (`CTMCRTH_BLCKNTYPE_ABSR_*`): 1, 2, 4, 5, 8, 10 — недопустимы
	/// @return `true` — тип недопустим.
	static bool IsBadAbsorber( int t ) { return ( t == 1 )||( t == 2 )||( t == 4 )||( t == 5 )||( t == 8 )||( t == 10 ); }

	/// @brief Повторяет проверку границ массива из `RunBlockNode` для узла `n` и четырёх соседей.
	/// @param[in] n  глобальный номер узла
	/// @return `true` — процессор обработал бы узел (все индексы внутри массива 2*nN).
	bool InBounds( TmcIdx n )
	{
		TmcIdx a = 2*nN;
		TmcIdx k[5] = { 2*(n - 1) + 1, 2*(n + nX) + 1, 2*(n + 1) + 1, 2*(n - nX) + 1, 2*n + 1 };
		for( int i = 0; i < 5; i++ ) if( ( k[i] <= 0 )||( k[i] >= a ) ) return false;
		return true;
	}

	// a port node and every node within 4 nodes of it (what ExciteInputs may touch)
	/// @brief Добавляет в список узел `n` и все узлы квадрата +-4 узла вокруг него (в пределах сетки).
	/// @param[in,out] v  список номеров узлов (повторы допускаются, убираются позже)
	/// @param[in]     n  глобальный номер узла порта
	void AddWindow( std::vector<cl_long> &v, TmcIdx n )
	{
		int x0 = (int)( n % nX ), y0 = (int)( n / nX );
		for( int y = y0 - 4; y <= y0 + 4; y++ )
			for( int x = x0 - 4; x <= x0 + 4; x++ )
				if( ( x >= 0 )&&( x < nX )&&( y >= 0 )&&( y < nY ) ) v.push_back( (cl_long)y*nX + x );
	}

	/// @brief Создаёт буфер на карте, при необходимости заполняя его данными процессора.
	/// @param[in] nBytes  размер, байт (0 заменяется на 8 — OpenCL не создаёт пустых буферов)
	/// @param[in] pInit   данные для копирования в буфер; `NULL` — без заполнения
	/// @return Буфер; `NULL` — ошибка, текст записан в `csError`.
	cl_mem Buf( size_t nBytes, const void *pInit = NULL )
	{
		cl_int e;
		cl_mem m = ocl.clCreateBuffer( ctx, CL_MEM_READ_WRITE | ( pInit ? CL_MEM_COPY_HOST_PTR : 0 ), nBytes ? nBytes : 8, (void *)pInit, &e );
		if( e != CL_SUCCESS ) { if( csError.IsEmpty() ) csError.Format( "buffer of %.2f GB: %s", nBytes/1073741824.0, TmcOclErr( e ) ); return NULL; }	// J-4: the first error is kept (as in X)
		return m;
	}

	/// @brief Задаёт аргумент-буфер ядра OpenCL (код возврата не проверяется).
	/// @param[in] k  ядро
	/// @param[in] i  номер аргумента, с 0
	/// @param[in] m  буфер
	void Arg( cl_kernel k, cl_uint i, cl_mem m ) { ocl.clSetKernelArg( k, i, sizeof(cl_mem), &m ); }

	/// @brief Проверяет код OpenCL и запоминает первую ошибку в `csError` в виде «GPU что: код».
	/// @param[in] e       код возврата функции OpenCL
	/// @param[in] szWhat  что делалось (для текста ошибки)
	/// @return `true` — `CL_SUCCESS`.
	bool Check( cl_int e, const char *szWhat )
	{
		if( e == CL_SUCCESS ) return true;
		if( csError.IsEmpty() ) csError.Format( "GPU %s: %s", szWhat, TmcOclErr( e ) );
		return false;
	}

	/// @brief Раскладывает массив процессора (2 значения на узел подряд) в два буфера карты.
	///
	/// Пишет кусками по 4M узлов блокирующими записями. При ошибке прекращает работу,
	/// текст — в `csError`.
	/// @param[in] pr  массив процессора длины 2*nN (`prUNode1` или `prYNode`)
	/// @param[in] m0  буфер для значений с чётными индексами (компонента 0)
	/// @param[in] m1  буфер для значений с нечётными индексами (компонента 1)
	void Upload( const double *pr, cl_mem m0, cl_mem m1 )
	{
		const TmcIdx nChunk = 4*1048576;
		std::vector<double> a( (size_t)__min( nChunk, nN ) ), b( a.size() );
		for( TmcIdx c = 0; c < nN; c += nChunk )
		{
			TmcIdx k, n = __min( nChunk, nN - c );
			for( k = 0; k < n; k++ ) { a[k] = pr[ 2*(c + k) ]; b[k] = pr[ 2*(c + k) + 1 ]; }
			if( !Check( ocl.clEnqueueWriteBuffer( q, m0, CL_TRUE, 8*(size_t)c, 8*(size_t)n, &a[0], 0, NULL, NULL ), "upload" ) ) return;
			if( !Check( ocl.clEnqueueWriteBuffer( q, m1, CL_TRUE, 8*(size_t)c, 8*(size_t)n, &b[0], 0, NULL, NULL ), "upload" ) ) return;
		}
	}

	/// @brief Дожидается конца очереди и освобождает буферы шага; контекст и программа остаются.
	void Release( void )
	{
		cl_mem *ms[] = { &mU0, &mU1, &mY0, &mY1, &mBIdx, &mBTyp, &mBRy, &mBRu, &mPIdx, &mPPk };
		if( q != NULL ) ocl.clFinish( q );
		for( int i = 0; i < 10; i++ ) if( *ms[i] != NULL ) { ocl.clReleaseMemObject( *ms[i] ); *ms[i] = NULL; }
		bOn = false;	// nPort/nRest are set by Begin (ReleaseAll inside Begin must not clear them)
	}

	/// @brief Освобождает всё: буферы, ядра, программу, очередь и контекст (перед сменой карты или после сбоя запуска).
	void ReleaseAll( void )
	{
		cl_kernel *ks[] = { &kPass, &kMetal, &kAbs, &kGather, &kScatter };
		Release();
		for( int i = 0; i < 5; i++ ) if( *ks[i] != NULL ) { ocl.clReleaseKernel( *ks[i] ); *ks[i] = NULL; }
		if( prg != NULL ) { ocl.clReleaseProgram( prg ); prg = NULL; }
		if( q != NULL ) { ocl.clReleaseCommandQueue( q ); q = NULL; }
		if( ctx != NULL ) { ocl.clReleaseContext( ctx ); ctx = NULL; }
		idDev = NULL;
	}
};

#endif
