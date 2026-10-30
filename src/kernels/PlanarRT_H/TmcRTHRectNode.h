// TmcRTHRectNode.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTHRectNode window

/// @file TmcRTHRectNode.h
/// @brief Прямоугольная Rt-сетка H-ядра: массивы узлов, подготовка шага `#STEP` и цикл во времени.
///
/// Часть модуля «Счётное ядро H». Класс `CTmcRTHRectNode` — вычислительное
/// сердце ядра: читает шаг задания через TMCIndan, строит сетку и блоки
/// (`CTmcRTHNodeDiel`), ведёт расчёт по подтактам и пишет сигналы портов,
/// поля и топологию. У ядра X — свой файл с тем же именем.
/// @see https://www.tamic.ru/docs/code/kernel-h.html
#ifndef _CTMCRTHRECTNODE__ 

#define _CTMCRTHRECTNODE__ 

#include <Typerth.h>
#include "TmcRTHNodeDiel.h"
#include <TmcLibError.h>
#include <TmcRTH_Indan.h>
#include <TmcRTH_IndanParam.h>
#include <TmcCalcTiming.h>	// TIMING: calculation time report

/// @brief Сетка узлов H-ядра и расчёт одного шага `#STEP` во времени.
///
/// Один объект живёт в документе ядра (`CPlanRT_HDoc::cRectNode`) всё время работы
/// окна. Документ вызывает `ReadData(файл, шаг)`, затем `RunStep()` в рабочем
/// потоке; `Stop()` из потока интерфейса прерывает чтение, построение блоков и
/// цикл во времени (флаг `bIsReadData`). Повторный `RunStep()` после `Stop()`
/// продолжает тот же шаг с прерванного такта. Напряжения узлов —
/// `prUNode1` (2 значения на узел), проводимости — `prYNode` (2 на узел).
/// Класс наследует `CWnd` только по историческим причинам: окна не создаёт.
/// @see https://www.tamic.ru/docs/code/kernel-h.html#lifecycle
class CTmcRTHRectNode : public CWnd
{
// Construction
public:
	/// @brief Сообщает, идёт ли чтение задания или расчёт (флаг `bIsReadData`).
	/// @return TRUE — объект занят.
	BOOL IsReadData( void );
	/// @brief Сообщает, выполняется ли `RunStep` (в т. ч. в другом потоке).
	/// @return TRUE — шаг считается.
	BOOL IsRunStep( void );
	/// @brief Сообщает, взведена ли ошибка ядра.
	/// @return TRUE — есть ошибка.
	BOOL IsError( void );
	/// @brief Просит прервать чтение, построение блоков или расчёт (снимает `bIsReadData`).
	///
	/// Не ждёт остановки: расчётный поток заметит флаг на ближайшей проверке.
	/// @note Потоки: вызывается из потока интерфейса.
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#threads
	void Stop( void );
	/// @brief Возвращает данные секции `#OUTPUT` текущего шага (имена файлов, флаги вывода).
	/// @return Указатель на объект TMCIndan; владеет ядро.
	CTmcRTH_IndanOutput* GetOutput( void );
	/// @brief Возвращает топологию (список блоков) текущего шага.
	/// @return Ссылка на объект TMCIndan.
	CTmcRTH_IndanTopology& GetTopology( void );
	/// @brief Возвращает параметры `#PARM` текущего шага (в СИ).
	/// @return Ссылка на объект TMCIndan.
	CTmcRTH_IndanParam& GetParam( void );
	/// @brief Сбрасывает данные и считает число шагов `#STEP` в задании.
	/// @param[in] csFileName путь к файлу `.tpl`
	/// @return Число шагов; при ошибке — ошибка в `cError`.
	int GetNStepMax( CString csFileName );
	/// @brief Возвращает текст ошибки ядра.
	/// @return Текст (по-английски); пусто — ошибки нет.
	CString GetErrorMessage( void );
	/// @brief Читает шаг задания: препроцессор, число шагов, секции `#PARM`, `#TOPOLOGY`, `#LINK_LIST`, `#OUTPUT`.
	///
	/// Освобождает данные прошлого шага. Сетку не строит — это делает `RunStep`.
	/// @param[in] csFileName путь к файлу `.tpl`
	/// @param[in] nStep1     номер шага, с 1; больше числа шагов — ошибка
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#lifecycle
	void ReadData( CString csFileName, int nStep1 );
	/// @brief Перечитывает секции уже выбранного шага (без препроцессора и поиска шага).
	void ReadData( void );
	/// @brief Считает текущий шаг `#STEP`: подготовка сетки, цикл во времени, матрица рассеяния.
	///
	/// Повторный вход во время расчёта игнорируется. Исключения расчёта (нехватка
	/// памяти и др.) перехватываются и превращаются в ошибку ядра; в конце закрывается
	/// файл `.t` и подводится итог замера времени.
	/// @note Потоки: вызывается из рабочего потока документа.
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#lifecycle
	void RunStep( void );
	/// @brief Создаёт пустую сетку (без массивов).
	CTmcRTHRectNode();
	/// @brief Возвращает накопитель замера времени расчёта (/Time).
	/// @return Ссылка на объект замера.
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#timing
	CTmcCalcTiming & GetTiming( void ) { return m_cTiming; }	// TIMING

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTHRectNode)
	//}}AFX_VIRTUAL

// Implementation
public:
	/// @brief Записывает файлы ДН (`.AMP`, `.FAZ`, `.DAT`) по накопленному распределению поля.
	///
	/// Вызывается из меню или в конце шага при `TMC_AUTO_DN=1`.
	/// @see https://www.tamic.ru/docs/theory/radiation.html#pattern
	void ExportToDirectionalPatter( void );
	/// @brief Возвращает флаг вывода топологии.
	/// @return TRUE — топология пишется.
	BOOL GetTopologyFlag( void );
	/// @brief Возвращает флаг звуковых сигналов.
	/// @return TRUE — звук включён.
	BOOL GetSoundEffect( void );
	/// @brief Возвращает флаг вывода поля.
	/// @return TRUE — поле пишется на каждом подтакте.
	BOOL GetFieldFlag( void );
	/// @brief Возвращает флаг синхронного вывода поля (ожидание программы просмотра).
	/// @return TRUE — синхронный вывод.
	BOOL GetSinchronizationFieldOutput( void );
	/// @brief Переключает вывод топологии и записывает файл топологии.
	void PutTopol( void );
	/// @brief Переключает звуковые сигналы.
	void OnOffSoundEffects( void );
	/// @brief Переключает синхронный вывод поля.
	void OnOffSinchronizationFieldOutput( void );
	/// @brief Переключает вывод поля.
	void OnOffFieldOutput( void  );
	/// @brief Освобождает все массивы (`DeleteData`).
	virtual ~CTmcRTHRectNode();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcRTHRectNode)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	/// @brief Передаёт поле подтакта накопителю распределения для ДН (`CFieldIntegrated::Integrate`).
	/// @param[in] ii 0 — чётные ячейки `prUNode1`, 1 — нечётные
	/// @see https://www.tamic.ru/docs/theory/radiation.html#contour
	void DistributionIntegrated( int ii );
	/// @brief Сбрасывает `.t` на диск и строит матрицу рассеяния `.s`; предупреждение
	///        «отклик не затух» выводит в консоль и строку состояния.
	/// @see https://www.tamic.ru/docs/theory/smatrix.html#s-from-signals
	void PutSmatrix( void );
	/// @brief Проверяет, не прочитан ли ещё последний кадр поля программой просмотра.
	///
	/// Ядро снимает у файла атрибут ARCHIVE после записи, FieldView взводит его после чтения.
	/// @return TRUE — файл есть и атрибут ARCHIVE снят (кадр ещё не прочитан).
	BOOL IsFieldFileRead( void );
	/// @brief Пишет файл топологии: заголовок и полную eps каждого узла (double или текст).
	///
	/// eps = Y/4 + 1 при Y >= 0 и eps = Y/k + 1 при Y < 0, k = (k0*Delta)^2/2,
	/// где Y = `prYNode[2n]`.
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#output
	void PutTopology( void );
	/// @brief Подаёт звуковой сигнал конца шага или всего расчёта (не в пакетном режиме).
	void PutStepEndSound( void );
	/// @brief Вызывает `RunBlockNode1` всех блоков (граничные условия подтакта 1).
	void RunBlockNode1( void );
	/// @brief Вызывает `RunBlockNode` всех блоков (граничные условия подтакта 0).
	void RunBlockNode( void );
	/// @brief Пишет кадр поля (напряжения всех узлов) в файл поля.
	///
	/// При синхронном выводе сначала ждёт, пока программа просмотра прочитает прошлый
	/// кадр (до 25 с; если не дождался — дальше не ждёт). Значения — `_real` в двоичном
	/// виде или текст по формату вывода поля.
	/// @param[in] ii 0 — чётные ячейки `prUNode1`, 1 — нечётные
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#output
	void PutField( TmcIdx ii );	// INT64
	/// @brief Рассеяние в узлах одной строки на подтакте 0: новое значение в чётной ячейке.
	///
	/// `U[2n] = prY[2n+1]*(U_left + U_right + U_down + U_up + prY[2n]*U[2n+1]) - U[2n]`,
	/// соседи — нечётные ячейки. Узлы обрабатываются парами.
	/// @param[in,out] pr_111  чётная ячейка первого узла строки (столбец 1)
	/// @param[in]  prY_111 проводимости первого узла строки
	/// @param[in]  nx_111  число узлов строки (nX - 2)
	/// @param[in]  nX_111  расстояние до соседней строки в массиве (2*nX)
	/// @see https://www.tamic.ru/docs/theory/h-mode.html#scattering
	void RunScatteringNode1line( _ELEM_VAL_RTH *pr_111, _real *prY_111, int nx_111, int nX_111);
	/// @brief Рассеяние в узлах одной строки на подтакте 1: новое значение в нечётной ячейке.
	/// @param[in,out] pr_111  чётная ячейка первого узла строки (столбец 1)
	/// @param[in]  prY_111 проводимости первого узла строки
	/// @param[in]  nx_111  число узлов строки (nX - 2)
	/// @param[in]  nX_111  расстояние до соседней строки в массиве (2*nX)
	/// @see https://www.tamic.ru/docs/theory/h-mode.html#scattering
	void RunScatteringNode1line1( _ELEM_VAL_RTH *pr_111, _real *prY_111, int nx_111, int nX_111);
	/// @brief Начинает в `.t` новую строку подтакта: номер такта и время, с.
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#t-file
	void OutputnT( void );
	/// @brief Открывает `.t` шага заново (заголовок, частота) и удаляет старые файлы поля и топологии.
	///
	/// Файл `.t` остаётся открытым до конца шага; другим программам запись запрещена.
	void DeleteOutputFile( void );
	/// @brief Дописывает в заголовок `.t` данные портов для многочастотной обработки.
	///
	/// Признак «сигнал из файла», список частот из `#FREQ` (Гц) и по строке на порт:
	/// блок, шаг сетки (м), апертура (м), частота согласования (Гц), признак файла.
	/// Последняя строка без перевода строки (его ставит `OutputnT`).
	/// @see https://www.tamic.ru/docs/theory/smatrix.html#multifreq
	void PutPortInfo( void );		// wideband: port data for post-processing
	/// @brief Начинает строку `.t` и обновляет все порты на подтакте 1 (`ExciteInputs1`).
	void RunExciteInputs1( void );
	/// @brief Начинает строку `.t` и обновляет все порты на подтакте 0 (`ExciteInputs`).
	void RunExciteInputs( void );
	/// @brief Переводит накопленные Delta eps узлов в проводимости шлейфов.
	///
	/// Для каждого узла: d = `prYNode[2n]`; d >= 0 — Y = 4*d (d в пределах 0,004 от 4
	/// сдвигается до 3,996/4,004), d < 0 — Y = d*(k0*Delta)^2/2; затем
	/// `prYNode[2n] = Y`, `prYNode[2n+1] = 2/(4 + |Y|)`. Ошибка, если
	/// omega*dT/2 > pi/3 (слишком крупный шаг для частоты).
	/// @see https://www.tamic.ru/docs/theory/h-mode.html#dielectric
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#admittance
	void SetDielInNodeList( void );
	/// @brief Строит все блоки (`CTmcRTHNodeDiel::SetBlock`) и передаёт список частот `.sig` накопителю ДН.
	void SetDielTopology( void );
	/// @brief Рассеяние во всех внутренних узлах сетки на подтакте 0 (строки 1..nY-2).
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#scattering
	void RunScatteringNode( void );
	/// @brief Рассеяние во всех внутренних узлах сетки на подтакте 1.
	void RunScatteringNode1( void );
	/// Копия параметров `#PARM` текущего шага (СИ).
	CTmcRTH_IndanParam cParam;
	/// @brief Удаляет массив блоков `pcNodeDiel`.
	void DeleteDielArray( void );
	/// @brief Создаёт блоки, строит их, переводит eps в проводимости и пишет топологию.
	void InitDielArray( void );
	/// @brief Сообщает, построена ли сетка текущего шага.
	/// @return TRUE — `nNumNode` != 0.
	BOOL IsDataInit( void );
	/// TRUE — выполняется `RunStep` (защита от повторного входа).
	volatile BOOL bIsRunStep1Run;
	/// @brief Тело `RunStep`: чтение (если нужно), `InitKernel`, `RunKernel`, звук.
	void RunStep1( void );
	/// @brief Освобождает массивы `prUNode1`, `prYNode`.
	void DeleteNodeArray( void );
	/// @brief Выделяет `prUNode1` и `prYNode` (по 2*nNumNode + 32 элементов, начало выровнено на 32 байта)
	///        и заполняет их значениями свободного пространства: U = 0, Y = 0, 2/(4+Y) = 0,5.
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#node-data
	void InitNodeArray( void );
	/// @brief Выполняет два подтакта расчёта на процессоре (порты, рассеяние, блоки, вывод).
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#time-loop
	void RunKernel1T( void );
	/// @brief Выполняет два подтакта с сеткой на видеокарте (`tmcgpuh.h`); порты и вывод — на процессоре.
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#gpu
	void RunKernel1TGpu( void );	// GPU: RunKernel1T with the grid on the video card (TmcGpuH.h)
	/// Шаг по времени одного подтакта tau = Delta*sqrt(mu0*eps0/2), с.
	double dT;
	/// Текущее время, с (от Tmin задания).
	double dTCurrent;
	/// Число подтактов шага: (Tmax - Tmin)/dT.
	int nTMax;
	/// @brief Цикл во времени до `nTMax` с выводом хода счёта, затем матрица рассеяния и (по `TMC_AUTO_DN`) ДН.
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#time-loop
	void RunKernel( void );
	/// @brief Готовит шаг: проверяет время и область, открывает `.t`, выделяет массивы, строит блоки.
	///
	/// Если сетка уже построена (продолжение после `Stop`) — ничего не делает.
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#init
	void InitKernel( void );
	/// Признак «объект занят чтением или расчётом». Снимается `Stop()`; все циклы ядра
	/// и блоков его проверяют и прерываются.
	volatile BOOL bIsReadData;
	/// @brief Читает секцию `#OUTPUT` текущего шага.
	void ReadOutputSection( void );
	/// @brief Читает секцию `#LINK_LIST` текущего шага.
	void ReadLinkListSection( void );
	/// @brief Читает секцию `#PARM` и копирует параметры в `cParam`.
	void ReadParamSection( void );
	/// @brief Читает секцию `#TOPOLOGY` текущего шага.
	void ReadTopologySection( void );
	/// @brief Выбирает шаг задания по номеру.
	/// @param[in] nStep номер шага, с 1
	void SetStepNumber( int nStep );
	/// Разбор задания (TMCIndan): параметры, блоки, вывод, накопитель ДН.
	CTmcRTH_Indan cIndan;
	/// Ошибка ядра; методы при взведённой ошибке ничего не делают.
	CTmcLibError cError;
	/// Массив блоков шага, длина `nDiel`; владеет ядро.
	CTmcRTHNodeDiel * pcNodeDiel; // array for each dielectric
	/// Число блоков шага.
	int nDiel;					// block number ( dielectric number )
	/// Номер текущего подтакта (0..nTMax).
	int nTCurrent;				// current number of t time
	/// Число узлов сетки по X.
	int nX;						// number of node for X axies
	/// Число узлов сетки по Y.
	int nY;						// number of node for Y axies
	/// Шаг сетки Delta (одинаковый по X и Y), м.
	double dDelta;				// global parameter delta = deltaX = deltaY
	/// Левая граница области, м.
	double dXmin;				// global parameter Xmin for rectangle
	/// Правая граница области, м.
	double dXmax;				// global parameter Xmax for rectangle
	/// Нижняя граница области, м.
	double dYmin;				// global parameter Ymin for rectangle
	/// Верхняя граница области, м.
	double dYmax;				// global parameter Ymax for rectangle
	/// @brief Освобождает массивы и блоки, сбрасывает время, размеры и ошибку.
	void DeleteData( void );
	/// Напряжения узлов: `prUNode1[2n]` — значение чётного подтакта, `prUNode1[2n+1]` —
	/// нечётного (n = iy*nX + ix). Выровнен на 32 байта внутри `prUNode1Temp`.
	_ELEM_VAL_RTH * prUNode1;			// array voltage on unit with 1 takt 
	/// Начало выделенного блока памяти под `prUNode1` (для delete[]).
	_ELEM_VAL_RTH * prUNode1Temp;		// temp pointer 
//	_ELEM_VAL_RTH * prUNode2;			// array voltage on unit with 2 takt  
	/// Проводимости узлов: `prYNode[2n]` — шлейф Y (со знаком), `prYNode[2n+1]` — 2/(4 + |Y|).
	/// До `SetDielInNodeList` в `prYNode[2n]` накапливается Delta eps узла.
	_real * prYNode;					// node admitance array
	/// Начало выделенного блока памяти под `prYNode` (для delete[]).
	_real * prYNodeTemp;				// temp pointer
//	_real * prYMulNode;					// node multiple admitance array
//	  int * pnTypeNode;					// node type array
//	_ELEM_VAL_RTH * prUyNodeTop;		// array falling voltage on unit with a top    V for each node
//	_ELEM_VAL_RTH * prUyNodeBottom;		// array falling voltage on unit with a bottom ^ for each node
//	_ELEM_VAL_RTH * prUxNodeRight;		// array falling voltage on unit at the right	 < for each node
//	_ELEM_VAL_RTH * prUxNodeLeft;		// array falling voltage on unit at the left   > for each node
//	_real * prUNode;			// array falling voltage 1 = V; 2 = ^; 3 = <; 4 = >
//	sTmcRTH_DielNodeList ** pcNodeDielList; // array node dielectric list for each node
	/// Число узлов сетки nX*nY (64 бита); 0 — сетка не построена.
	TmcIdx nNumNode;			// number of all node = nX*nY	// INT64
	/// Замер времени этапов расчёта (/Time).
	CTmcCalcTiming m_cTiming;	// TIMING: calculation time report
};


#endif
/////////////////////////////////////////////////////////////////////////////
