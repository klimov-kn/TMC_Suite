// TmcRTHNodeDiel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTHNodeDiel window

/// @file TmcRTHNodeDiel.h
/// @brief Блок задания H-ядра: узлы блока на сетке, граничные условия, порты, файлы eps.
///
/// Часть модуля «Счётное ядро H». Объявляет описание узла блока
/// `sTmcRTHNodeDielOne` и класс `CTmcRTHNodeDiel` — один объект на каждый блок
/// задания (`#LINK_LIST`): диэлектрик (RECT/CIRCLE/POLYGON/FILE), металл, поглотитель,
/// магнитная стенка, вход (порт). Сам расчёт во времени ведёт `CTmcRTHRectNode`
/// (`tmcrthrectnode.h`); блок вмешивается в него только через `ExciteInputs*`
/// и `RunBlockNode*`. У ядра X — свой файл с тем же именем и другим узлом (плазма, 4 подтакта).
/// @see https://www.tamic.ru/docs/code/kernel-h.html#blocks
#ifndef _CTMCRTHNODEDIEL__ 

#define _CTMCRTHNODEDIEL__ 

#include <Typerth.h>
#include "TmcLibError.h"
#include "TmcRTH_IndanParam.h"
#include "TmcRTH_BolckList.h"
#include "TmcRTH_IndanOutput.h"

/// Расширение файла распределения eps (`.eps`), добавляется к имени без расширения.
#define TMC_RTH_EPSFILE_EXT_	"eps"
/// Первая строка файла `.eps` (метка формата и версии).
#define TMC_RTH_EPSFILE_ID_		"#TamicRTH_planar_DistributionDielectricPermeability_File_V2.00 2000"
/// Метка строки с типом блока, записавшего файл; читаются только файлы блока RECT_STAT.
#define TMC_RTH_EPSFILE_TYPE_	"#TopologyPrimitiv "
/// Метка строки с шагом сетки, м; при чтении должен совпадать с шагом задания (до 1e-5).
#define TMC_RTH_EPSFILE_Delta_	"#dDelta "
/// Метка строки с левой границей блока в его локальных координатах, м.
#define TMC_RTH_EPSFILE_XMIN_	"#Xmin "
/// Метка строки с нижней границей блока в его локальных координатах, м.
#define TMC_RTH_EPSFILE_YMIN_	"#Ymin "
/// Метка строки с числом узлов блока по X.
#define TMC_RTH_EPSFILE_NX_		"#nX "
/// Метка строки с числом узлов блока по Y.
#define TMC_RTH_EPSFILE_NY_		"#nY "
/// Метка строки с числом записей «значение + номер узла» после заголовка.
#define TMC_RTH_EPSFILE_NPoint_	"#nPoint "
/// Метка строки с размером значения в байтах: 4 (float) или 8 (double).
#define TMC_RTH_EPSFILE_NAccur_	"#nAccuracy "
/// Метка строки формата номера узла (пишется «NULL» — двоичные данные).
#define TMC_RTH_EPSFILE_NdFrmt_	"#sNodeFormat "
/// Метка строки формата значения (пишется «NULL» — двоичные данные).
#define TMC_RTH_EPSFILE_VlFrmt_	"#sValueFormat "

/// @brief Один узел блока: номер узла сетки, его роль в блоке и состояние для граничных условий.
///
/// Массив таких записей (`CTmcRTHNodeDiel::pcNodeDielOne`) строится при подготовке
/// шага и живёт до его конца. Для стационарного диэлектрика массив после
/// занесения eps в `prYNode` удаляется (`DeleteStatDielData`). Сами напряжения
/// сетки хранятся не здесь, а в общем массиве ядра `prUNode1`.
/// @see https://www.tamic.ru/docs/code/kernel-h.html#node-data
struct STMCRTH_NODEDIELONE
{
	/// Порт: падающее напряжение `rY*a(t)` прошлого чётного подтакта (подтакт 0).
	/// Поглотитель и магнитная стенка: напряжение узла `U^(tau-2)`, сохранённое до того,
	/// как его перепишет общий проход рассеяния. Условные единицы сетки.
	_ELEM_VAL_RTH rU;	// falling voltage on dielectric
	/// Порт: напряжение узла в чётной ячейке `prUNode1[2n]`, сохранённое после обновления порта
	/// (восстанавливается перед следующим обновлением — рассеяние могло его переписать).
	_ELEM_VAL_RTH rU_Cur;	// curretn voltage on dielectric
	/// Порт: то же для нечётной ячейки `prUNode1[2n+1]`.
	_ELEM_VAL_RTH rU_Cur1;	// curretn+1 takt voltage on dielectric
	/// Порт: падающее напряжение `rY*a(t)` прошлого нечётного подтакта (подтакт 1).
	_ELEM_VAL_RTH rUo;	// scattering voltage for input
	/// Вес узла; смысл зависит от блока. Диэлектрик — доля четырёх соседних ячеек сетки внутри
	/// блока (1; 0,5 на стороне; 0,25 в углу прямоугольника; сумма по 0,25 у круга и
	/// многоугольника), для FILE — уже умноженное на eps значение из файла. Порт — амплитуда
	/// профиля основной волны в узле. Поглотитель — проводимость согласованной нагрузки `y`
	/// (магнитная стенка — 0). Металл — не используется.
	_real rY;			// admitance for node
	/// Глобальный номер узла сетки `iy*nX + ix` (64 бита). Напряжения узла — `prUNode1[2*n]`,
	/// `prUNode1[2*n+1]`, проводимости — `prYNode[2*n]`, `prYNode[2*n+1]`. -1 при построении —
	/// узел отброшен (повтор или вне области).
	TmcIdx nNodeGlobal;  // number of node in global topology	// INT64
	/// Код узла (`tmcgrviw.h`): `CTMCRTH_BLCKNTYPE_EPS` (0) — диэлектрик, `_METAL` (101) — металл,
	/// `_INPXLEFT`..`_INPYBOT` (102..105) — входы; у поглотителя и магнитной стенки — код
	/// соседства `CTMCRTH_BLCKNTYPE_ABSR_*` (1..16, 21..24). Старое описание справа
	/// (0, 1, -1 … -6) не соответствует текущим значениям констант.
	int nType;			// signum for eps	=  0 for eps < 0;
						//					=  1 for eps > 0;
						//					= -1 for metall;
						//					= -2 for absorber;
						//	is equel members nType in class CTmcRTH_Input 
						//					= -3 for input x left   > 
						//					= -4 for input x right  < 
						//					= -5 for input y top    V 
						//					= -6 for input y bottom ^ 
} ;
/// Короткое имя записи узла блока.
typedef struct STMCRTH_NODEDIELONE sTmcRTHNodeDielOne;

/// @brief Звено односвязного списка «узел сетки -> узлы блоков в нём».
///
/// Используется только функциями `SetBlockInDielList`/`AddList`, которые
/// в Qt6-версии не вызываются.
struct STMCRTH_DIELNODELIST
{
	/// Следующее звено списка; NULL — конец.
	STMCRTH_DIELNODELIST * pcDielNodeNextList;
	/// Узел блока (не владеет).
	sTmcRTHNodeDielOne * pcNodeDielOne;
} ;
/// Короткое имя звена списка узлов.
typedef struct STMCRTH_DIELNODELIST sTmcRTH_DielNodeList;

/// Задание быстрого построения eps-блока (EPSFAST); определено в `tmcrthnodediel.cpp`.
struct sEpsFastJob;	// EPSFAST

/// @brief Блок задания H-ядра: строит свои узлы на сетке и выполняет свои граничные условия.
///
/// Один объект — один блок секции `#LINK_LIST`. Массив объектов создаёт
/// `CTmcRTHRectNode::InitDielArray` (по числу блоков) перед расчётом шага `#STEP`
/// и удаляет `DeleteDielArray` при следующем чтении задания. `SetBlock` строит
/// узлы блока и заносит вклад диэлектрика в общий массив проводимостей; затем
/// на каждом подтакте ядро вызывает `ExciteInputs`/`ExciteInputs1` (порты) и
/// `RunBlockNode`/`RunBlockNode1` (металл, поглотитель, магнитная стенка).
/// Класс наследует `CWnd` только по историческим причинам: окна не создаёт.
/// Не потокобезопасен; все методы вызываются из расчётного потока, кроме
/// `EpsFastRun`, который выполняется в рабочих потоках EPSFAST.
/// @see https://www.tamic.ru/docs/code/kernel-h.html#blocks
class CTmcRTHNodeDiel : public CWnd
{
// Construction						 
public:
	/// @brief Отдаёт модулю расчёта на видеокарте тип блока, число и массив его узлов (только чтение).
	/// @param[out] nType1 тип блока `CTMCRTH_BLCKNTYPE_*`
	/// @param[out] nNum1  число узлов блока (0 — у стационарного диэлектрика после EPSFAST)
	/// @param[out] p1     массив узлов блока; владеет блок
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#gpu
	void GpuView( int &nType1, TmcIdx &nNum1, sTmcRTHNodeDielOne *&p1 ) { nType1 = nType; nNum1 = nNumNode; p1 = pcNodeDielOne; }	// GPU: read access for TmcGpuH.h
	/// @brief Обновляет узлы порта на подтакте 0 (чётные ячейки) и пишет строку «блок, Up, Uo» в `.t`.
	///
	/// Только для блоков-входов (`INPXLEFT`..`INPYBOT`), у прочих ничего не делает.
	/// Шаги: восстанавливает сохранённые напряжения узлов порта (`rU_Cur`, `rU_Cur1`);
	/// обнуляет узел позади порта, если порт стоит не на краю области; вычисляет новое
	/// напряжение каждого узла порта по схеме узла с согласованной нагрузкой `y1`
	/// (`dY1_InputAdmitance`), шлейфом среды порта `dYInput` и источником `rY*a(t)`;
	/// сохраняет результат; вычисляет отражённую волну
	/// `Uo = sum(rY*(U - rU)) / sum(rY^2)` и падающую `Up = a(t)` и выводит их.
	/// Сигнал `a(t) = Amp(t,f)*sin(omega*t + Faza)` или значение из файла `.sig`;
	/// вне окна Tmin..Tmax порта `a = 0`.
	/// @param[in,out] cError1 общий объект ошибки ядра: при уже взведённой ошибке ничего
	///                не делается; ошибка блока (в т. ч. «меньше 3 узлов») копируется сюда
	/// @param[in]  dWT       omega*t, рад (omega = 2*pi*rFreq)
	/// @param[in]  dT        текущее время t, с — по нему проверяется окно Tmin..Tmax порта
	///                       (несмотря на имя, это не шаг по времени)
	/// @param[in,out] pr1    массив напряжений сетки `prUNode1`, 2 значения на узел
	/// @param[in]  nX        число узлов сетки по X
	/// @param[in]  cParam    параметры `#PARM` (единицы, частота, шаг)
	/// @param[in]  dtCurrent текущее время, с — для выражений Amp/Faza и файла `.sig`
	/// @see https://www.tamic.ru/docs/theory/ports.html#transformer
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#ports
	void ExciteInputs(  CTmcLibError &cError1, double dWT, double dT, _ELEM_VAL_RTH *pr1, int nX, CTmcRTH_IndanParam &cParam, double dtCurrent );
	/// @brief То же, что `ExciteInputs`, для подтакта 1: пишет нечётные ячейки, читает чётные.
	/// @param[in,out] cError1 общий объект ошибки ядра
	/// @param[in]  dWT       omega*t, рад
	/// @param[in]  dT        текущее время t, с (окно Tmin..Tmax порта)
	/// @param[in,out] pr1    массив напряжений сетки `prUNode1`
	/// @param[in]  nX        число узлов сетки по X
	/// @param[in]  cParam    параметры `#PARM`
	/// @param[in]  dtCurrent текущее время, с
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#ports
	void ExciteInputs1( CTmcLibError &cError1, double dWT, double dT, _ELEM_VAL_RTH *pr1, int nX, CTmcRTH_IndanParam &cParam, double dtCurrent );
	/// @brief Строит узлы блока задания по его типу и форме и вносит вклад блока в массив проводимостей.
	///
	/// Читает из блока тип, выражения и смещение X0, Y0 и вызывает построитель:
	/// металл — `SetRectMetal`/`SetPolyMetal`/`SetCircMetal`, поглотитель —
	/// `Set*Absorber`, магнитная стенка — `Set*Magnetic`, диэлектрик — `SetRectFile`
	/// (FILE), `SetStatEpsFast` (стационарные RECT/CIRCLE/POLYGON, если включён EPSFAST)
	/// или `SetRect`/`SetCirc`/`SetPoly`, вход — `SetInp*`. Затем проверяет, что узлы
	/// (и соседи узлов порта) лежат в сетке, убирает повторы, задаёт коды узлов,
	/// прибавляет eps диэлектрика к `prYNode[2n]` (`SetYLineForDiel`), строит профиль
	/// порта (`SetYForInput`) и освобождает массив узлов стационарного диэлектрика.
	/// Вызывается один раз на блок при подготовке шага `#STEP`.
	/// @param[in]  pcNextBlockList блок задания (TMCIndan); величины в СИ
	/// @param[in]  cParam          параметры сетки: шаг rDelta (м), область, частота (Гц)
	/// @param[in]  cOut            вывод шага: имя файла `.t`, куда порт пишет сигналы
	/// @param[in,out] prYNode      массив проводимостей ядра, длина 2*nX*nY: в `prYNode[2n]`
	///                             прибавляется вклад блока «вес * значение выражения eps»
	/// @param[in]  pbStopFlag      признак «расчёт идёт» ядра (`bIsReadData`); FALSE — прервать
	/// @param[in]  csCurrentPath   папка задания: в ней ищутся и создаются файлы `.eps` и `.sig`
	/// @return Ссылка на объект ошибки блока; `IsError()` == FALSE — успех.
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#blocks
	CTmcLibError &SetBlock( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut, _real *prYNode, volatile BOOL *pbStopFlag, CString csCurrentPath );
	/// @brief Освобождает узлы блока и данные файла `.sig`, сбрасывает ошибку и поля
	///        в исходное состояние (тип блока — металл).
	void DeleteData( void );
	/// @brief Создаёт пустой блок без узлов.
	CTmcRTHNodeDiel();
	/// @brief Подключает счётчики замера времени (ключ /Time): запись сигналов портов и файлов `.eps`.
	/// @param[in] pqSignalFile счётчик тактов QPC записи сигналов в `.t`; NULL — не считать
	/// @param[in] pqEpsFile    счётчик тактов QPC записи файлов `.eps`; NULL — не считать
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#timing
	void SetTimingCounters( __int64 *pqSignalFile, __int64 *pqEpsFile ) { m_pqSignalFileTicks = pqSignalFile; m_pqEpsFileTicks = pqEpsFile; }	// TIMING
	/// @brief Подключает счётчики чтения файлов распределения (блоки FILE) и числа файлов `.eps`.
	/// @param[in] pqEpsRead    счётчик тактов QPC чтения блоков FILE; NULL — не считать
	/// @param[in] pnEpsWritten счётчик записанных файлов `.eps`; NULL — не считать
	/// @param[in] pnEpsRead    счётчик прочитанных файлов `.eps`; NULL — не считать
	void SetTimingEpsCounters( __int64 *pqEpsRead, int *pnEpsWritten, int *pnEpsRead ) { m_pqEpsReadTicks = pqEpsRead; m_pnEpsWritten = pnEpsWritten; m_pnEpsRead = pnEpsRead; }	// TIMING
	// --- wideband post-processing: port data for the .t header
	/// @brief Сообщает, берётся ли падающая волна порта из файла `.sig`.
	/// @return TRUE — сигнал из файла (`Amp` задан как `[имя]`).
	BOOL IsSignalFromFile( void ) { return bSigFile; }
	/// @brief Сообщает, является ли блок входом (портом).
	/// @return TRUE — тип блока `INPXLEFT`, `INPXRIGHT`, `INPYTOP` или `INPYBOT`.
	BOOL IsInputBlock( void );
	/// @brief Возвращает номер блока из `#LINK_LIST`.
	/// @return Номер блока.
	int GetBlockNumber( void ) { return nBlock; }
	/// @brief Возвращает ширину апертуры порта.
	/// @return (число узлов порта - 1) * Delta, м; для прочих блоков 1.
	double GetWidthWaveg( void ) { return dWidthWaveg; }
	/// @brief Возвращает список частот из строки `#FREQ` файла `.sig`.
	/// @return Частоты через пробел в единицах FREQ_UNIT задания; пусто — списка нет.
	CString GetSigFreqList( void ) { return csSigFreq; }

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTHNodeDiel)
	//}}AFX_VIRTUAL

// Implementation
public:
	/// @brief Дописывает к имени файла `.eps` папку задания, если имя не полное.
	/// @param[in] csCurrentPath папка задания
	void AddCurrentPath( CString csCurrentPath );
	/// @brief Возвращает объект ошибки блока.
	/// @return Ссылка на объект ошибки.
	CTmcLibError & GetError( void );
	/// @brief То же, что `RunBlockNode`, для подтакта 1: пишет нечётные ячейки, читает чётные.
	/// @param[in,out] prUNode1  массив напряжений сетки
	/// @param[in]  prYNode   массив проводимостей (`prYNode[2n]` — шлейф узла)
	/// @param[in]  nX        число узлов сетки по X
	/// @param[in]  nArraySize длина массива `prUNode1` (2*nX*nY) — для проверки соседей
	void RunBlockNode1( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, TmcIdx nArraySize );
	/// @brief Выполняет граничное условие блока после рассеяния подтакта 0 (чётные ячейки).
	///
	/// Металл — обнуляет напряжение узлов блока. Поглотитель и магнитная стенка —
	/// пересчитывают узлы контура по коду соседства (полные и половинные линии,
	/// доля шлейфа `y5 = prYNode[2n]`, нагрузка `y = rY`), внутренние узлы (код 1111
	/// и 0000) обнуляют; затем запоминают в `rU` нечётную ячейку для следующего подтакта.
	/// У диэлектрика и входов ничего не делает.
	/// @param[in,out] prUNode1  массив напряжений сетки, 2 значения на узел
	/// @param[in]  prYNode   массив проводимостей
	/// @param[in]  nX        число узлов сетки по X
	/// @param[in]  nArraySize длина массива `prUNode1` (2*nX*nY); узлы, у которых сосед
	///                        выходит за массив, пропускаются
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#absorber
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#absorber
	void RunBlockNode( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, TmcIdx nArraySize  );
	/// @brief Освобождает узлы и данные блока (`DeleteData`).
	virtual ~CTmcRTHNodeDiel();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcRTHNodeDiel)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	/// @brief Уточняет коды угловых узлов магнитного многоугольника и круга у краёв области.
	///
	/// Угол у левого края (x < Xmin + 1,5*Delta) или у правого (x > Xmax - 2,5*Delta)
	/// получает код стороны: стенка продолжается до края области.
	/// @param[in] cParam параметры сетки
	void SetMagneticPolygonType( CTmcRTH_IndanParam &cParam );
	/// @brief Вычисляет нормированную проводимость шлейфа среды порта `dYInput` по выражению W.
	///
	/// W — Delta eps среды порта (eps - 1). Правило то же, что для узлов диэлектрика:
	/// W >= 0 — `dYInput = 4*W` (значения W в пределах 0,004 от 4 сдвигаются до 3,996 или
	/// 4,004), W < 0 — `dYInput = W*(k0*Delta)^2/2`, k0 = 2*pi*f/c. Пустое W — 0.
	/// @param[in] dFreq частота задания, Гц
	/// @param[in] dDelta шаг сетки, м
	/// @see https://www.tamic.ru/docs/theory/ports.html#port-w
	void CalculateYForInput( double dFreq, double dDelta );
	/// @brief Вычисляет начальную фазу сигнала порта по выражению Faza.
	///
	/// В выражение подставляются t (в TIME_UNIT) и f (в FREQ_UNIT задания).
	/// @param[in] cParam    параметры задания (единицы времени, частоты, угла)
	/// @param[in] dtCurrent текущее время, с
	/// @return -Faza*rAngleUnit, рад (минус — задержка); пустое выражение — 0.
	///         При ошибке выражения — 0 и ошибка в `cError`.
	/// @see https://www.tamic.ru/docs/theory/ports.html#signal
	double dFaza(CTmcRTH_IndanParam &cParam, double dtCurrent);
	/// @brief Вычисляет амплитуду сигнала порта по выражению Amp (переменные t, f).
	/// @param[in] cParam    параметры задания (единицы времени и частоты)
	/// @param[in] dtCurrent текущее время, с
	/// @return Амплитуда (условные единицы); пустое выражение — 1; ошибка — 0 и `cError`.
	/// @see https://www.tamic.ru/docs/theory/ports.html#signal
	double dAmplitude(CTmcRTH_IndanParam &cParam, double dtCurrent);
	/// Нормирующий множитель профиля порта: `rY[i] = dUnormir*sin(i*pi/(N-1))`
	/// (порт MAGNETIC — `rY[i] = dUnormir`), `dUnormir = sqrt(1/(y1*sum(profile^2)*Delta^2))`.
	double dUnormir;
	/// @brief Проверяет, задано ли имя файла `.eps` полностью (диск, начальный разделитель) или пусто.
	/// @return TRUE — путь дописывать не нужно.
	BOOL IsFullName( void );
	/// @brief Добавляет к имени файла `.eps` расширение «.eps», если его нет (точка в имени папки не в счёт).
	void AddFileEpsExtention( void );
	/// @brief Выделяет из выражения eps вида `[имя]выражение` имя файла `.eps` для записи.
	///
	/// Имя без скобок попадает в `csFileNameEps` (с расширением и папкой задания),
	/// в `csEpsExpr` остаётся выражение после `]`. Без `[` в начале — ничего не делает;
	/// без `]` — ошибка.
	/// @param[in] csCurrentPath папка задания
	void SeachEpsFileName( CString csCurrentPath );
	/// Полное имя файла `.eps`, куда блок RECT_STAT пишет своё распределение; пусто — не писать.
	CString csFileNameEps;
// --- incident wave from file (wideband excitation) ---------------------
	/// @brief Если Amp порта задано как `[имя]`, находит и читает файл падающей волны `.sig`.
	///
	/// Без точки в имени добавляется «.sig», без пути — папка задания. Иначе снимает
	/// признак `bSigFile`.
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#signal-file
	void SeachSignalFileName( void );
	/// @brief Читает файл `.sig`: две колонки «время (TIME_UNIT) значение».
	///
	/// Строки, начинающиеся с `!` или `#`, пропускаются; строка `#FREQ f1 f2 ...`
	/// задаёт список частот (FREQ_UNIT). Ошибки: файл не открывается, меньше двух
	/// точек, время не возрастает. Если у порта задана ненулевая фаза — выводит
	/// предупреждение: с файлом фаза не используется.
	/// @see https://www.tamic.ru/docs/theory/ports.html#incident-file
	void ReadSignalFile( void );
	/// @brief Освобождает таблицу сигнала `.sig` и снимает признак `bSigFile`.
	void DeleteSignalData( void );
	/// @brief Возвращает значение падающей волны из файла `.sig` в момент времени (линейная интерполяция).
	/// @param[in] dtCurrent текущее время, с (переводится в TIME_UNIT задания)
	/// @param[in] cParam    параметры задания
	/// @return Значение сигнала; вне диапазона времени файла — 0.
	double dSignalFromFile( double dtCurrent, CTmcRTH_IndanParam &cParam );
	/// TRUE — падающая волна порта берётся из файла `.sig`, а не из Amp*sin(omega*t + Faza).
	BOOL bSigFile;
	/// Моменты времени таблицы `.sig` в TIME_UNIT задания, возрастают; длина `nSigN`.
	double *pdSigT;
	/// Значения сигнала таблицы `.sig`; длина `nSigN`.
	double *pdSigV;
	/// Число точек таблицы `.sig` (не меньше 2, если файл прочитан).
	int nSigN;
	/// Полное имя файла `.sig`.
	CString csFileNameSig;
	/// Список частот из строки `#FREQ` файла `.sig` (FREQ_UNIT); пусто — нет.
	CString csSigFreq;		// frequency list named inside the .sig file, if any
	/// Папка задания для поиска файла `.sig`.
	CString csSigPath;
	/// @brief Освобождает массив узлов стационарного диэлектрика после занесения eps в `prYNode`.
	///
	/// Для блоков *_MOVE и не диэлектриков ничего не делает.
	void DeleteStatDielData( void );
	/// Признак «расчёт идёт» ядра (`CTmcRTHRectNode::bIsReadData`); `*pbIsStop` == FALSE — прервать построение.
	volatile BOOL * pbIsStop;
	/// Счётчик /Time: такты QPC записи сигналов портов; NULL — не считать.
	__int64 * m_pqSignalFileTicks;	// TIMING: time of writing port signals (QPC ticks)
	/// Счётчик /Time: такты QPC записи файлов `.eps`; NULL — не считать.
	__int64 * m_pqEpsFileTicks;		// TIMING: time of writing eps-distribution files
	/// Счётчик /Time: такты QPC чтения блоков FILE; NULL — не считать.
	__int64 * m_pqEpsReadTicks;		// TIMING: time of FILE blocks (reading distribution files)
	/// Счётчик /Time: число записанных файлов `.eps`; NULL — не считать.
	int * m_pnEpsWritten;			// TIMING: number of blocks that wrote a distribution file
	/// Счётчик /Time: число прочитанных файлов `.eps`; NULL — не считать.
	int * m_pnEpsRead;				// TIMING: number of blocks read from a distribution file
	/// @brief Прибавляет время чтения блока FILE и (при успехе) единицу к числу прочитанных файлов.
	/// @param[in] qTicks длительность чтения, такты QPC
	void TimingEpsRead( __int64 qTicks ) { if( m_pqEpsReadTicks != NULL ) *m_pqEpsReadTicks += qTicks; if( ( m_pnEpsRead != NULL )&&( !cError.IsError() ) ) (*m_pnEpsRead)++; }	// TIMING
	/// @brief Записывает распределение eps блока RECT_STAT в файл `.eps`, если в выражении задано `[имя]`.
	///
	/// Заголовок — строки с метками `TMC_RTH_EPSFILE_*` (CR LF), затем для каждого узла
	/// блока (строки снизу вверх, в строке слева направо) — значение «вес * eps» типа `_real`
	/// и 4-байтовый локальный номер узла. Координаты — локальные координаты блока.
	/// При остановке файл удаляется.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @param[in] csCurrentPath   папка задания
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#eps-file
	void SaveFileRect( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CString csCurrentPath );
	/// @brief Удаляет повторяющиеся узлы блока (каждый узел сетки — один раз).
	///
	/// Сложность O(N^2) по числу узлов блока. Для диэлектрика и прямоугольного металла
	/// ничего не делает.
	void DeleteDublicateData( void );
	/// @brief Назначает узлам поглотителя или магнитной стенки код соседства `ABSR_*`.
	///
	/// Бит кода взводится, если соседний узел тоже принадлежит блоку: 0001 — левый
	/// (n-1), 0010 — верхний (n+nX), 0100 — правый (n+1), 1000 — нижний (n-nX).
	/// Нет соседей — код 0000 (16). Узел 1111 без одного из диагональных соседей
	/// получает код внутреннего угла 1111_1..1111_4. Сложность O(N^2).
	/// @param[in] cParam параметры сетки (nX)
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#absorber
	void SetAbsorberNodeType( CTmcRTH_IndanParam &cParam );
	/// @brief Возвращает полярный угол точки в диапазоне [0, 2*pi).
	/// @param[in] y координата Y
	/// @param[in] x координата X
	/// @return Угол, рад.
	double atan2__1( double y, double x  );
	/// Смещение блока по Y из `#LINK_LIST` (Y0), м.
	double dY0;
	/// Смещение блока по X из `#LINK_LIST` (X0), м.
	double dX0;
	/// @brief Строит магнитную стенку по контуру многоугольника: полоса 2x2 узла вдоль ломаной.
	///
	/// Нагрузка `y` = 0; коды узлов — `SetAbsorberNodeType` и `SetMagneticPolygonType`.
	/// Части вне области отбрасываются.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#open
	CTmcLibError & SetPolyMagnetic( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит поглотитель по контуру многоугольника: полоса 2x2 узла вдоль ломаной.
	///
	/// Нагрузка узлов `y = SetAbsorberAdmitance(...)`; точки ближе 1–2 шагов к краю
	/// области прижимаются внутрь, части вне области отбрасываются; коды — `SetAbsorberNodeType`.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#absorber
	CTmcLibError & SetPolyAbsorber( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит магнитную стенку по прямоугольнику: внешний контур и кольцо внутри (код 0000).
	///
	/// Нагрузка `y` = 0. Узлы ставятся `CalcnNodeGlobalFM`; у края области к коду угла
	/// и стороны добавляется бит стороны края.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#open
	CTmcLibError & SetRectMagnetic( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит поглотитель по прямоугольнику: внешний контур с кодами сторон и углов
	///        и кольцо узлов внутри с кодом 0000 (обнуляются).
	///
	/// Нагрузка узлов `y = SetAbsorberAdmitance(...)`. Прямоугольник не подходит к краям
	/// области ближе 1 шага слева/снизу и 2 шагов справа/сверху; размер — не меньше 2 шагов.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#absorber
	CTmcLibError & SetRectAbsorber( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит металл по контуру многоугольника: ступенчатая ломаная из узлов (4-связная).
	///
	/// Узлы вне области отбрасываются. Металл — только контур: внутренние узлы
	/// отделены им от остальной сетки.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#staircase
	CTmcLibError & SetPolyMetal(CTmcRTH_BlockList *pcNextBlockList, CTmcRTH_IndanParam &cParam);
	/// @brief Строит металл по контуру прямоугольника (4 стороны, части вне области обрезаются).
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#metal
	CTmcLibError & SetRectMetal( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Вычисляет проводимость согласованной нагрузки порта `y1` для основной волны.
	///
	/// `a = sqrt(2)*pi*Delta*f/c` (omega*tau), `arg = (2 + |yW|/2)*cos(a) - yW/2 - cos(pi*Delta/b)`,
	/// `y1 = sin(acos(arg))/sin(a)`, где yW = `dYInput`, b = `dWidthWaveg`. Если |arg| > 1
	/// (частота ниже отсечки порта) — ошибка в `cError`.
	/// @param[in] dFreq  частота задания, Гц
	/// @param[in] dDelta шаг сетки, м
	/// @return Нормированная проводимость нагрузки.
	/// @see https://www.tamic.ru/docs/theory/ports.html#transformer
	double SetInputAdmitance( double dFreq, double dDelta );
	/// @brief Вычисляет проводимость нагрузки поглотителя (и порта MAGNETIC) для плоской волны.
	///
	/// Как `SetInputAdmitance`, но без поперечной вариации: `arg = (2 + |yW|/2)*cos(a) - 1 - yW/2`.
	/// У блоков поглотителя `dYInput` = 0 (свободное пространство).
	/// @param[in] dFreq  частота задания, Гц
	/// @param[in] dDelta шаг сетки, м
	/// @return Нормированная проводимость нагрузки.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#absorber
	double SetAbsorberAdmitance( double dFreq, double dDelta );
	/// @brief Р-1: пересчитывает нагрузку узлов поглотителя по среде, в которой стоит каждый узел.
	///
	/// По (7.54) диссертации набег alpha берётся из дисперсии той среды, где стоит граница: для узла с
	/// шлейфом y (полный шлейф узла после `SetDielInNodeList`) нагрузка
	/// `y0 = sin(acos((2 + |y|/2)*cos(a) - 1 - y/2))/sin(a)`, a = sqrt(2)*pi*Delta*f/c — так же, как у
	/// входа по W (Д-5). Узлы в вакууме (y = 0) не трогаются: нагрузка та же, что раньше. Вызывается один
	/// раз после сборки проводимостей, до передачи узлов на видеокарту. Только для блоков ABSORBER.
	/// @param[in] cParam   параметры шага (частота, шаг сетки)
	/// @param[in] prYNode  проводимости узлов сетки
	/// @param[in] nStride  число ячеек prYNode на узел (2 в H, 6 в X)
	/// @param[in] nSlot    ячейка шлейфа узла (0 в H, 1 в X)
	/// @param[in] nNodes   число узлов сетки (границы массива)
	/// Узел, в среде которого волна на этой сетке не распространяется (|arg| > 1: плотная плазма eps < 0),
	/// сохраняет прежнюю нагрузку и считается в `nAbsorberNoWave` (ядро выдаёт предупреждение).
	/// @return Ошибка блока (как у прочих методов; сам пересчёт ошибок не выдаёт).
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#absorber
public:	// R-1: called by CTmcRTHRectNode
	CTmcLibError &SetAbsorberLoadByMedium( CTmcRTH_IndanParam &cParam, _real *prYNode, int nStride, int nSlot, TmcIdx nNodes );
	/// @brief Р-1: число узлов поглотителя, у которых в их среде волна не распространяется (оставлена прежняя нагрузка).
	/// @return Число таких узлов после `SetAbsorberLoadByMedium` (0 — у прочих блоков).
	TmcIdx GetAbsorberNoWave( void ) { return nAbsorberNoWave; }
	/// Р-1: узлы поглотителя без распространяющейся волны в их среде (считает `SetAbsorberLoadByMedium`).
	TmcIdx nAbsorberNoWave;
private:
	/// Нормированная проводимость согласованной нагрузки порта `y1`.
	double dY1_InputAdmitance;
	/// Ширина апертуры порта b, м.
	double dWidthWaveg;
	/// @brief Пишет в `.t` номер блока, падающую `Up` и отражённую `Uo` волны порта.
	///
	/// Файл шага держит открытым ядро (`TmcOutTGet`); если он закрыт — открывается
	/// на дозапись (ожидание занятого файла до 20 с). В демо-варианте вместо Uo — надпись.
	/// @param[in] dUp падающая волна (амплитуда сигнала a(t))
	/// @param[in] dUo отражённая волна (проекция на профиль порта)
	/// @param[in,out] cError1 объект ошибки ядра (ошибка открытия файла)
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#t-file
	void OutputUpUo( double dUp, double dUo, CTmcLibError &cError1);
	/// @brief Вычисляет выражение eps блока в точке.
	///
	/// В выражение подставляются x, y (в LONG_UNIT, относительно X0, Y0 блока),
	/// r = sqrt(x^2 + y^2) и f (полярный угол в ANGLE_UNIT). Значение выражения — Delta eps
	/// (eps - 1) для H-ядра.
	/// @param[in] x      координата узла относительно X0 блока, м
	/// @param[in] y      координата узла относительно Y0 блока, м
	/// @param[in] cParam параметры задания (единицы)
	/// @return Значение выражения; при ошибке — 0 и ошибка в `cError`.
	/// @see https://www.tamic.ru/docs/theory/h-mode.html#dielectric
	_real CalcEps( double x, double y, CTmcRTH_IndanParam &cParam );
	// EPSFAST: fast build of stationary eps blocks (TmcRTHNodeDiel.cpp)
	/// @brief Быстро строит стационарный eps-блок (EPSFAST) прямо в `prYNode`, без массива узлов.
	///
	/// Тот же результат, что `SetRect`/`SetCirc`/`SetPoly` + `SetYLineForDiel`, бит в бит:
	/// выражение без x, y, r, f считается один раз; с ними — в нескольких потоках
	/// (если узлов в описанном прямоугольнике не меньше порога, по умолчанию 1000);
	/// с `=` — в одном потоке в исходном порядке узлов. При любом нестандартном случае
	/// (повтор узлов сетки, нет узлов) возвращает FALSE, и блок строится прежним кодом.
	/// @param[in]  pcNextBlockList блок задания
	/// @param[in]  cParam          параметры сетки
	/// @param[in]  csCurrentPath   папка задания (файл `.eps` у RECT_STAT)
	/// @param[in,out] prYNode      массив, куда прибавляется вклад узлов
	/// @param[in]  nShape          форма: `EPSFAST_RECT`, `EPSFAST_CIRC`, `EPSFAST_POLY`
	/// @param[in]  nStride         шаг массива на узел (2 в H-ядре)
	/// @param[in]  nOffset         смещение ячейки eps в записи узла (0)
	/// @return TRUE — блок построен (или причина ошибки в `cError`); FALSE — ничего не сделано.
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#epsfast
	BOOL SetStatEpsFast( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CString csCurrentPath, _real *prYNode, int nShape, int nStride, int nOffset );
	/// @brief Тело рабочего потока EPSFAST: берёт строки по `EPSFAST_CHUNK`, пока они есть.
	///
	/// Узлы блока на сетке различны (проверено заранее), поэтому потоки не пишут
	/// в одну ячейку. У каждого потока свой контекст интерпретатора выражений.
	/// @param[in,out] pJob задание; общие счётчики — атомарные
	void EpsFastRun( sEpsFastJob *pJob );
	/// @brief Проверяет, входит ли узел (i, j) описанного прямоугольника в блок, и вычисляет его вес.
	/// @param[in]  pJob задание
	/// @param[in]  i    номер столбца описанного прямоугольника
	/// @param[in]  j    номер строки описанного прямоугольника
	/// @param[out] prY  вес узла (как `rY` у `SetRect`/`SetCirc`/`SetPoly`)
	/// @return TRUE — узел принадлежит блоку.
	BOOL EpsFastNode( sEpsFastJob *pJob, int i, int j, _real *prY );
	/// @brief Вычисляет выражение eps в точке без `cError` (для рабочих потоков), как `CalcEps`.
	/// @param[in]  pszExpr выражение
	/// @param[in]  x       координата относительно X0 блока, м
	/// @param[in]  y       координата относительно Y0 блока, м
	/// @param[in]  cParam  параметры задания
	/// @param[out] szBuf   рабочий буфер `CTMCRTH_INDANSBUF` символов
	/// @param[out] szBuf1  второй рабочий буфер (не используется)
	/// @param[out] pr      значение выражения
	/// @return 0 — успех, -1 — ошибка выражения.
	int  EpsFastEval( const char *pszExpr, double x, double y, CTmcRTH_IndanParam &cParam, char *szBuf, char *szBuf1, _real *pr );
	/// @brief Классифицирует выражение eps для EPSFAST.
	/// @param[in] pszExpr выражение
	/// @return `EPSFAST_SERIAL` — есть `=`; `EPSFAST_PARALLEL` — есть x, y, r или f
	///         (после удаления имён функций); иначе `EPSFAST_CONST`.
	static int EpsFastExprKind( const char *pszExpr );
	/// @brief Возвращает число потоков EPSFAST.
	/// @return `TMC_EPS_THREADS`, если задана; иначе число процессоров минус 1 на каждые 8
	///         (1..64).
	static unsigned EpsFastThreads( void );
	/// @brief Проверяет, включён ли EPSFAST.
	/// @return FALSE, если переменная окружения `TMC_EPS_FAST` начинается с «0».
	static BOOL EpsFastEnabled( void );
	/// @brief Записывает тип блока в узлы (кроме поглотителя и магнитной стенки) и обнуляет их напряжения.
	void SetnType( void );
	/// @brief Прибавляет к `prYNode[2n]` узлов диэлектрика «вес * значение выражения eps».
	///
	/// Для FILE берёт готовое значение `rY` из файла. Выражение без x, y, r, f при
	/// включённом EPSFAST считается один раз. Пересчёт в проводимость шлейфа делает
	/// потом `CTmcRTHRectNode::SetDielInNodeList`.
	/// @param[in] cParam параметры задания
	/// @param[in,out] prYNode массив проводимостей ядра
	/// @see https://www.tamic.ru/docs/theory/h-mode.html#dielectric
	void SetYLineForDiel( CTmcRTH_IndanParam &cParam, _real *prYNode );
	/// @brief Вычисляет профиль порта `rY`: sin(i*pi/(N-1)) (стенки — металл) или постоянный
	///        (порт MAGNETIC), нормированный множителем `dUnormir`.
	/// @param[in] cParam параметры сетки (шаг)
	/// @see https://www.tamic.ru/docs/theory/ports.html#modes
	void SetYForInput( CTmcRTH_IndanParam &cParam );
	/// @brief Строит нижний вход (волна идёт вверх): строка узлов на высоте Y0, апертура X0+Xmin..X0+Xmax.
	///
	/// Апертура не подходит к краям области ближе 2 шагов. Задаёт окно Tmin..Tmax,
	/// выражения Amp, Faza, W, проводимости `dYInput` и `y1`, читает файл `.sig`.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @param[in] cOut            вывод шага (имя файла `.t`)
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#ports
	CTmcLibError &SetInpYBot( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut );
	/// @brief Строит верхний вход (волна идёт вниз): строка узлов на высоте Y0. См. `SetInpYBot`.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @param[in] cOut            вывод шага (имя файла `.t`)
	/// @return Ссылка на объект ошибки блока.
	CTmcLibError &SetInpYTop( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut );
	/// @brief Строит правый вход (волна идёт влево): столбец узлов на X0, апертура Y0+Xmin..Y0+Xmax.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @param[in] cOut            вывод шага (имя файла `.t`)
	/// @return Ссылка на объект ошибки блока.
	CTmcLibError &SetInpXRight( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut );
	/// @brief Строит левый вход (волна идёт вправо): столбец узлов на X0, апертура Y0+Xmin..Y0+Xmax.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @param[in] cOut            вывод шага (имя файла `.t`)
	/// @return Ссылка на объект ошибки блока.
	CTmcLibError &SetInpXLeft( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut );
	/// @brief Вычисляет средние координаты узлов блока `dXCenter`, `dYCenter` (дальше не используются).
	/// @param[in] cParam параметры сетки
	void CalculatedXYCenter( CTmcRTH_IndanParam &cParam );
	/// @brief Строит узлы диэлектрического многоугольника (прежний путь без EPSFAST).
	///
	/// Узел внутри, если попадает в один из треугольников веера от первой вершины;
	/// вес — по 0,25 за каждую диагональную соседнюю точку внутри.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	CTmcLibError &SetPoly( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит узлы диэлектрического кольцевого сектора (Rmin..Rmax, FImin..FImax).
	///
	/// Вес — по 0,25 за каждую диагональную соседнюю точку внутри сектора.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	CTmcLibError &SetCirc( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит магнитную стенку по контуру кольцевого сектора (два радиуса и две дуги, полоса 2x2).
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#open
	CTmcLibError &SetCircMagnetic( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит поглотитель по контуру кольцевого сектора (два радиуса и две дуги, полоса 2x2).
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#absorber
	CTmcLibError &SetCircAbsorber( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит металл по контуру кольцевого сектора: два радиальных отрезка и две дуги.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#staircase
	CTmcLibError &SetCircMetal( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит узлы диэлектрического прямоугольника (прежний путь без EPSFAST) и пишет `.eps`, если задано.
	///
	/// Вес узла: 1 внутри, 0,5 на стороне, 0,25 в углу. Части вне области обрезаются.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @param[in] csCurrentPath   папка задания (для файла `.eps`)
	/// @return Ссылка на объект ошибки блока.
	CTmcLibError &SetRect( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CString csCurrentPath );
	/// @brief Строит блок FILE: читает распределение из файла `.eps` и ставит узлы на сетку.
	///
	/// Ищет файл как задан, затем в папке задания и с расширением «.eps». Проверяет
	/// метки заголовка, тип RECT_STAT, совпадение шага, точность 4 или 8 байт.
	/// Узлы, не попавшие в область, пропускаются. Значение из файла (вес * eps)
	/// становится `rY` узла.
	/// @param[in] pcNextBlockList блок задания; его Xmin..Ymax переписываются по файлу
	/// @param[in] cParam          параметры сетки
	/// @param[in] csCurrentPath   папка задания
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#eps-file
	CTmcLibError &SetRectFile( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CString csCurrentPath );
	/// Ошибка блока; методы при взведённой ошибке ничего не делают.
	CTmcLibError cError;
	/// Номер блока из `#LINK_LIST`.
	int nBlock;							// block number
	/// Ключевое слово примитива (RECT_STAT, FILE …); у входа после построения — имя файла `.t`.
	CString csBlock;					// block Type
	/// Число узлов блока (длина `pcNodeDielOne`); 0 у стационарного диэлектрика после построения.
	TmcIdx nNumNode;					// number of node in dielectric	// BIGBLOCK
	/// Массив узлов блока; владеет блок, NULL — нет узлов.
	sTmcRTHNodeDielOne * pcNodeDielOne; // array of dielectric node
	/// Средняя координата X узлов блока, м (вычисляется, не используется).
	double dXCenter;					// x for center block
	/// Средняя координата Y узлов блока, м (вычисляется, не используется).
	double dYCenter;					// y for center block
	/// Порт: нормированная проводимость шлейфа среды порта по W (0 — свободное пространство).
	double dYInput;						// admittance for input block
	/// Порт: начало окна возбуждения Tmin, с; вне окна сигнал равен 0.
	double dTmin;						// T_min for excite input block
	/// Порт: конец окна возбуждения Tmax, с.
	double dTmax;						// T_max for excite input block
	/// Выражение eps диэлектрика; у FILE — имя файла; у металла и др. — слово типа
	/// (у порта — METAL или MAGNETIC: тип боковых стенок).
	CString csEpsExpr;					// expression for eps
	/// У порта — выражение W (Delta eps среды порта); у прочих блоков не используется.
	CString csW;						// angular speed
	/// У порта — выражение амплитуды Amp(t, f) или `[имя]` файла `.sig`.
	CString csVx;						// linear speed	for block center of rotation x0, y0
	/// У порта — выражение фазы Faza(t, f) в ANGLE_UNIT.
	CString csVy;						// linear speed	for block center of rotation x0, y0
	/// Номер столбца сетки, ближайшего к X0 блока.
	int nX0;							// x0 for centre of rotation
	/// Номер строки сетки, ближайшей к Y0 блока.
	int nY0;							// y0 for centre of rotation
	/// Не используется (наследие подвижных блоков).
	int nTxCurrent;						// counter for x moving
	/// Не используется (наследие подвижных блоков).
	int nTyCurrent;						// counter for y moving
	/// Не используется (наследие подвижных блоков).
	int nTxMove;						// period for x moving
	/// Не используется (наследие подвижных блоков).
	int nTyMove;						// period for y moving
	/// Тип блока `CTMCRTH_BLCKNTYPE_*` (`tmcgrviw.h`): EPS 0, ABSORBER 100, METAL 101,
	/// INPXLEFT..INPYBOT 102..105, MAGNETIC 107. Старое описание справа устарело.
	int nType;			// signum for eps	=  0 for eps < 0;
						//					=  1 for eps > 0;
						//					= -1 for metall;
						//					= -2 for absorber;
						//	is equel members nType in class CTmcRTH_Input 
						//					= -3 for input x left   > 
						//					= -4 for input x right  < 
						//					= -5 for input y top    V 
						//					= -6 for input y bottom ^ 
};



#endif
/////////////////////////////////////////////////////////////////////////////
