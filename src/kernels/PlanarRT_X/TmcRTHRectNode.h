// TmcRTHRectNode.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTHRectNode window

/// @file TmcRTHRectNode.h
/// @brief Прямоугольная Rt-сетка X-ядра: массивы узлов (6 + 6 ячеек на узел), подготовка
///        шага `#STEP` и цикл во времени по 4 подтактам.
///
/// Часть модуля «Счётное ядро X» (волны E-поляризации, подмагниченная плазма). Класс
/// `CTmcRTHRectNode` — вычислительное сердце ядра: читает шаг задания через TMCIndan,
/// строит сетку и блоки (`CTmcRTHNodeDiel`), пересчитывает eps, N, B, Y, sigma узлов в
/// коэффициенты схемы, ведёт расчёт по подтактам и пишет сигналы портов, поля и
/// топологию. Ветвь того же кода, что ядро H.
/// @see https://www.tamic.ru/docs/code/kernel-x.html
#ifndef _CTMCRTHRECTNODE__ 

#define _CTMCRTHRECTNODE__ 

#include <Typerth.h>
#include "TmcRTHNodeDiel.h"
#include <TmcLibError.h>
#include <TmcRTH_Indan.h>
#include <TmcRTH_IndanParam.h>
#include <TmcCalcTiming.h>	// TIMING: calculation time report

/// @brief Сетка узлов X-ядра и расчёт одного шага `#STEP` во времени.
///
/// Один объект живёт в документе ядра (`CPlanRT_HDoc::cRectNode`) всё время работы
/// окна. Документ вызывает `ReadData(файл, шаг)`, затем `RunStep()` в рабочем
/// потоке; `Stop()` из потока интерфейса прерывает чтение, построение блоков и
/// цикл во времени (флаг `bIsReadData`). Повторный `RunStep()` после `Stop()`
/// продолжает тот же шаг с прерванного такта. Напряжения узлов — `prUNode1`
/// (6 ячеек на узел: 4 временных слоя и 2 вспомогательные суммы), коэффициенты —
/// `prYNode` (6 на узел). Узел X-ядра — компонента Hz поля.
/// Класс наследует `CWnd` только по историческим причинам: окна не создаёт.
/// @see https://www.tamic.ru/docs/code/kernel-x.html#node-data
/// @see https://www.tamic.ru/docs/theory/accuracy.html#accuracy-x
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
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#threads
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
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#lifecycle
	void ReadData( CString csFileName, int nStep1 );
	/// @brief Перечитывает секции уже выбранного шага (без препроцессора и поиска шага).
	void ReadData( void );
	/// @brief Считает текущий шаг `#STEP`: подготовка сетки, цикл во времени, матрица рассеяния.
	///
	/// Повторный вход во время расчёта игнорируется. Исключения расчёта (нехватка
	/// памяти и др.) перехватываются и превращаются в ошибку ядра; в конце закрывается
	/// файл `.t` и подводится итог замера времени.
	/// @note Потоки: вызывается из рабочего потока документа.
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#lifecycle
	void RunStep( void );
	/// @brief Создаёт пустую сетку (без массивов); имя ядра для отчёта /Time — «PlanarRT_X (tmc_rtx.exe)».
	CTmcRTHRectNode();
	/// @brief Возвращает накопитель замера времени расчёта (/Time).
	/// @return Ссылка на объект замера.
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#timing
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
	/// Вызывается из меню. В отличие от H-ядра, X-ядро не выполняет запись ДН
	/// автоматически по переменной `TMC_AUTO_DN`.
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
	///
	/// Накопитель знает шаг 6 ячеек на узел (`SetNodeStep(6)` в `InitKernel`).
	/// @param[in] ii номер подтакта 0..3 — ячейка `prUNode1[6n + ii]`, в которой новое значение
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
	/// @brief Пишет файл топологии X-ядра: заголовок и четыре таблицы nY x nX — eps, N, Y, B.
	///
	/// Восстанавливает величины из коэффициентов узла (Yk = `prYNode[6n + k]`):
	/// eps = Y1/4 + 1 при Y1 >= 0 и eps = Y1/(omega*dT)^2 + 1 при Y1 < 0;
	/// в узле плазмы (Y1 < 0) x = 4*(2 - Y4)/(2 + Y4), wp^2 = sqrt(|Y1|*x)/dT^2,
	/// N = wp^2*eps0*m/e^2 (м^-3), wc^2 = x/dT^2 - wp^2, B = wc*m/e (Тл); в узле
	/// диэлектрика N = 0, B = 0; Y = Y2 как есть (вместе с пересчитанной sigma). Значения —
	/// `double` в двоичном виде или текст по формату вывода поля.
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#topology
	void PutTopology( void );
	/// @brief Подаёт звуковой сигнал конца шага или всего расчёта (не в пакетном режиме).
	void PutStepEndSound( void );
	/// @brief Вызывает `RunBlockNode3` всех блоков (граничные условия подтакта 3).
	void RunBlockNode3( void );
	/// @brief Вызывает `RunBlockNode2` всех блоков (граничные условия подтакта 2).
	void RunBlockNode2( void );
	/// @brief Вызывает `RunBlockNode1` всех блоков (граничные условия подтакта 1).
	void RunBlockNode1( void );
	/// @brief Вызывает `RunBlockNode` всех блоков (граничные условия подтакта 0).
	void RunBlockNode( void );
	/// @brief Пишет кадр поля (Hz всех узлов) в файл поля.
	///
	/// При синхронном выводе сначала ждёт, пока программа просмотра прочитает прошлый
	/// кадр (до 25 с; если не дождался — дальше не ждёт). Значения — `_real` в двоичном
	/// виде или текст по формату вывода поля.
	/// @param[in] ii номер подтакта 0..3 — пишется ячейка `prUNode1[6n + ii]`
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#output
	void PutField( TmcIdx ii );	// INT64
	/// @brief Пересчитывает узлы одной строки на подтакте 0 по четырёхслойной схеме узла с плазмой.
	///
	/// Новое значение пишется в ячейку 0 (там было `U^(tau-4)`); `U^(tau-1)`, `U^(tau-2)`,
	/// `U^(tau-3)` — ячейки 3, 2, 1. С Yk = `prY[6n + k]`, Uk = `U^(tau-k)`:
	/// `R = сумма соседей U1 + Y1*U1 + Y2*U2`,
	/// `U = -U4 - 2*U2 + Y4*(U1 + U3) + Y0*(R + R2 - Y4*R1 + Y3*U2 - Y5*(U1 + U3))`,
	/// где R1, R2 — суммы R прошлых подтактов (ячейки 4 и 5); новое R пишется в ячейку 5.
	/// @param[in,out] pr_111  ячейка 0 первого узла строки (столбец 1)
	/// @param[in]  prY_111 коэффициенты первого узла строки
	/// @param[in]  nx_111  число узлов строки (nX - 2)
	/// @param[in]  nX_111  расстояние до соседней строки в массиве (6*nX)
	/// @see https://www.tamic.ru/docs/theory/x-mode.html#coefficients
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#scattering
	void RunScatteringNode1line( _ELEM_VAL_RTH *pr_111, _real *prY_111, int nx_111, int nX_111);
	/// @brief То же, что `RunScatteringNode1line`, для подтакта 1: пишет ячейку 1;
	///        `U^(tau-1..tau-3)` — ячейки 0, 3, 2; новое R — в ячейку 4.
	/// @param[in,out] pr_111  ячейка 0 первого узла строки (столбец 1)
	/// @param[in]  prY_111 коэффициенты первого узла строки
	/// @param[in]  nx_111  число узлов строки (nX - 2)
	/// @param[in]  nX_111  расстояние до соседней строки в массиве (6*nX)
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#scattering
	void RunScatteringNode1line1( _ELEM_VAL_RTH *pr_111, _real *prY_111, int nx_111, int nX_111);
	/// @brief То же, что `RunScatteringNode1line`, для подтакта 2: пишет ячейку 2;
	///        `U^(tau-1..tau-3)` — ячейки 1, 0, 3; новое R — в ячейку 5.
	/// @param[in,out] pr_111  ячейка 0 первого узла строки (столбец 1)
	/// @param[in]  prY_111 коэффициенты первого узла строки
	/// @param[in]  nx_111  число узлов строки (nX - 2)
	/// @param[in]  nX_111  расстояние до соседней строки в массиве (6*nX)
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#scattering
	void RunScatteringNode1line2( _ELEM_VAL_RTH *pr_111, _real *prY_111, int nx_111, int nX_111);
	/// @brief То же, что `RunScatteringNode1line`, для подтакта 3: пишет ячейку 3;
	///        `U^(tau-1..tau-3)` — ячейки 2, 1, 0; новое R — в ячейку 4.
	/// @param[in,out] pr_111  ячейка 0 первого узла строки (столбец 1)
	/// @param[in]  prY_111 коэффициенты первого узла строки
	/// @param[in]  nx_111  число узлов строки (nX - 2)
	/// @param[in]  nX_111  расстояние до соседней строки в массиве (6*nX)
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#scattering
	void RunScatteringNode1line3( _ELEM_VAL_RTH *pr_111, _real *prY_111, int nx_111, int nX_111);
	/// @brief Начинает в `.t` новую строку подтакта: номер такта и время, с.
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#t-file
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
	/// @brief Начинает строку `.t` и обновляет все порты на подтакте 3 (`ExciteInputs3`).
	void RunExciteInputs3( void );
	/// @brief Начинает строку `.t` и обновляет все порты на подтакте 2 (`ExciteInputs2`).
	void RunExciteInputs2( void );
	/// @brief Начинает строку `.t` и обновляет все порты на подтакте 1 (`ExciteInputs1`).
	void RunExciteInputs1( void );
	/// @brief Начинает строку `.t` и обновляет все порты на подтакте 0 (`ExciteInputs`).
	void RunExciteInputs( void );
	/// @brief Пересчитывает накопленные Delta eps, N, Y, sigma, B узлов в коэффициенты схемы узла.
	///
	/// До вызова в `prYNode[6n + 0..4]` лежат Delta eps, N (м^-3), Y, sigma (См/м), B (Тл).
	/// sigma != 0 добавляет к Y величину 2*dT*sigma/eps0. Узел плазмы (Delta eps < 0 или N != 0):
	/// эквивалентная плазменная частота wp^2 = N*e^2/(m*eps0) - Delta eps*omega^2,
	/// wc^2 = (e*B/m)^2, x = dT^2*(wc^2 + wp^2); Y1 = -dT^2*wp^4/(wc^2 + wp^2),
	/// Y3 = 8*dT^2*wc^2*wp^2/((wc^2 + wp^2)*(4 + x)), Y4 = 2*(4 - x)/(4 + x), Y5 = Y3*Y4/4,
	/// Y0 = 2/(4 + |Y1| + Y2 + Y3/2). Узел диэлектрика: Y1 = 4*Delta eps (около 4 — сдвиг до
	/// 3,996/4,004), Y3 = 0, Y4 = -2, Y5 = 0, Y0 = 2/(4 + |Y1| + Y2). Перед пересчётом
	/// предупреждает об узлах, где Delta eps > 0 сочетается с N или с B. Ошибка, если
	/// omega*dT/2 > pi/3; в учебной версии узлы с eps < 1 запрещены.
	/// @see https://www.tamic.ru/docs/theory/x-mode.html#coefficients
	/// @see https://www.tamic.ru/docs/theory/plasma.html#eps-below-one
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#coefficients
	void SetDielInNodeList( void );
	/// @brief Строит все блоки (`CTmcRTHNodeDiel::SetBlock`) и передаёт список частот `.sig` накопителю ДН.
	void SetDielTopology( void );
	/// @brief Пересчитывает все внутренние узлы сетки на подтакте 0 (строки 1..nY-2).
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#scattering
	void RunScatteringNode( void );
	/// @brief Пересчитывает все внутренние узлы сетки на подтакте 1.
	void RunScatteringNode1( void );
	/// @brief Пересчитывает все внутренние узлы сетки на подтакте 2.
	void RunScatteringNode2( void );
	/// @brief Пересчитывает все внутренние узлы сетки на подтакте 3.
	void RunScatteringNode3( void );
	/// @brief Вызывает `SaveInputNodes` всех блоков: запоминает узлы портов после их обновления.
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#port-nodes
	void RunSaveInputs( void );		// BUG12 portfix
	/// @brief Вызывает `RestoreInputNodes` всех блоков: возвращает узлам портов значения,
	///        которые переписал общий проход рассеяния.
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#port-nodes
	void RunRestoreInputs( void );	// BUG12 portfix
	/// @brief Б-1: после блоков ещё раз восстанавливает узлы портов плоской волны (в общем узле со стенкой —
	///        значение порта, как в ядре H).
	void RunRestorePlaneWaveInputs( void );	// B-1
	/// Копия параметров `#PARM` текущего шага (СИ).
	CTmcRTH_IndanParam cParam;
	/// @brief Удаляет массив блоков `pcNodeDiel`.
	void DeleteDielArray( void );
	/// @brief Создаёт блоки, строит их, пересчитывает значения узлов в коэффициенты и пишет топологию.
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
	/// @brief Выделяет `prUNode1` и `prYNode` (по 6*nNumNode + 32 элементов, начало выровнено
	///        на 32 байта) и обнуляет все ячейки; коэффициенты свободного пространства
	///        задаёт потом `SetDielInNodeList`.
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#node-data
	void InitNodeArray( void );
	/// @brief Выполняет четыре подтакта расчёта на процессоре; на каждом — порты, сохранение
	///        узлов портов, рассеяние, восстановление узлов портов, блоки, вывод.
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#subcycles
	void RunKernel1T( void );
	/// @brief Выполняет четыре подтакта с сеткой на видеокарте (`tmcgpux.h`); порты и вывод — на процессоре.
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#gpu
	void RunKernel1TGpu( void );	// GPU: RunKernel1T with the grid on the video card (TmcGpuX.h)
	/// Шаг по времени одного подтакта tau = Delta*sqrt(mu0*eps0/2), с.
	double dT;
	/// Текущее время, с (от Tmin задания).
	double dTCurrent;
	/// Число подтактов шага: (Tmax - Tmin)/dT.
	int nTMax;
	/// @brief Цикл во времени до `nTMax` (по 4 подтакта за проход) с выводом хода счёта, затем
	///        матрица рассеяния.
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#time-loop
	void RunKernel( void );
	/// @brief Готовит шаг: проверяет время и область, открывает `.t`, выделяет массивы, строит блоки.
	///
	/// Если сетка уже построена (продолжение после `Stop`) — ничего не делает.
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#init
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
	/// Напряжения узлов (значения Hz), 6 ячеек на узел n = iy*nX + ix: ячейки 0..3 — четыре
	/// последних временных слоя по кругу (на подтакте k новое значение пишется в ячейку k),
	/// ячейки 4, 5 — суммы R двух прошлых подтактов. Выровнен на 32 байта внутри `prUNode1Temp`.
	/// Старое описание ниже (t, t-4 …) говорит о том же круговом хранении.
	_ELEM_VAL_RTH * prUNode1;			// array voltage on unit with 1 takt 
//	prUNode1[0] - voltage in takt t   and t-4;
//	prUNode1[1] - voltage in takt t-1 and t-5;
//	prUNode1[2] - voltage in takt t-2 and t-6;
//	prUNode1[3] - voltage in takt t-3 and t-7;
	/// Начало выделенного блока памяти под `prUNode1` (для delete[]).
	_ELEM_VAL_RTH * prUNode1Temp;		// temp pointer 
//	_ELEM_VAL_RTH * prUNode2;			// array voltage on unit with 2 takt  
	/// Коэффициенты узлов, 6 на узел: Y0 = 2/(4 + |Y1| + Y2 + Y3/2), Y1 — шлейф (y5 со знаком:
	/// > 0 диэлектрик, < 0 плазма), Y2 — потери y, Y3 = 2*y6*y7/(y6 + y7), Y4 = 2*(y7 - y6)/(y6 + y7),
	/// Y5 = Y3*Y4/4. До `SetDielInNodeList` в ячейках 0..4 накапливаются Delta eps, N (м^-3),
	/// Y, sigma (См/м), B (Тл). Старое описание ниже — то же, в обозначениях теории.
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
