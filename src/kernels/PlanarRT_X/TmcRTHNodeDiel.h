// TmcRTHNodeDiel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTHNodeDiel window

/// @file TmcRTHNodeDiel.h
/// @brief Блок задания X-ядра: узлы блока на сетке, блоки плазмы, граничные условия, порты, файлы eps.
///
/// Часть модуля «Счётное ядро X» (волны E-поляризации, подмагниченная плазма). Объявляет
/// описание узла блока `sTmcRTHNodeDielOne` и класс `CTmcRTHNodeDiel` — один объект на
/// каждый блок задания (`#LINK_LIST`): диэлектрик и плазма (RECT/CIRCLE/POLYGON_STAT и их
/// варианты `_N`, `_B`, `_Y`, `_S`, FILE, FILE_N/_B/_Y), металл, поглотитель, магнитная
/// стенка, вход (порт). Сам расчёт во времени ведёт `CTmcRTHRectNode` (`tmcrthrectnode.h`);
/// блок вмешивается в него через `ExciteInputs*`, `SaveInputNodes`/`RestoreInputNodes`
/// и `RunBlockNode*` — по одному варианту на каждый из 4 подтактов.
/// Ветвь того же кода, что ядро H; отличия — шаг массивов узла 6 вместо 2, 4 подтакта,
/// блоки плазмы.
/// @see https://www.tamic.ru/docs/code/kernel-x.html#blocks
#ifndef _CTMCRTHNODEDIEL__ 

#define _CTMCRTHNODEDIEL__ 

#include <Typerth.h>
#include "TmcLibError.h"
#include "TmcRTH_IndanParam.h"
#include "TmcRTH_BolckList.h"
#include "TmcRTH_IndanOutput.h"

/// Расширение файла распределения (`.eps`), добавляется к имени без расширения.
#define TMC_RTH_EPSFILE_EXT_	"eps"
/// Первая строка файла `.eps` (метка формата и версии).
#define TMC_RTH_EPSFILE_ID_		"#TamicRTH_planar_DistributionDielectricPermeability_File_V2.00 2000"
/// Метка строки с типом блока, записавшего файл (RECT_STAT, RECT_STAT_N, _B, _Y …).
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

/// @brief Один узел блока X-ядра: номер узла сетки, его роль в блоке и состояние порта
///        или граничного условия.
///
/// Массив таких записей (`CTmcRTHNodeDiel::pcNodeDielOne`) строится при подготовке
/// шага и живёт до его конца. Для стационарного диэлектрика и плазмы массив после
/// занесения значений в `prYNode` удаляется (`DeleteStatDielData`). Сами напряжения
/// сетки хранятся не здесь, а в общем массиве ядра `prUNode1` (6 ячеек на узел).
/// @see https://www.tamic.ru/docs/code/kernel-x.html#node-data
struct STMCRTH_NODEDIELONE
{
	/// Порт: падающее напряжение `rY*a(t)` подтакта 0; его вычитает `ExciteInputs2`
	/// (через два подтакта). Поглотитель и магнитная стенка: напряжение узла `U^(tau-2)`
	/// для следующего подтакта (обновляется в конце каждого `RunBlockNode*`).
	/// Условные единицы сетки.
	_ELEM_VAL_RTH rU;	// falling voltage on dielectric in 0 takt
	/// Порт: падающее напряжение `rY*a(t)` подтакта 1; его вычитает `ExciteInputs3`.
	_ELEM_VAL_RTH rU1;	// falling voltage on dielectric in 1 takt
	/// Порт: падающее напряжение `rY*a(t)` подтакта 2; его вычитает `ExciteInputs` следующего цикла.
	_ELEM_VAL_RTH rU2;	// falling voltage on dielectric in 2 takt
	/// Порт: падающее напряжение `rY*a(t)` подтакта 3; его вычитает `ExciteInputs1` следующего цикла.
	_ELEM_VAL_RTH rU3;	// falling voltage on dielectric in 3 takt
	/// Не используется в расчёте (обнуляется при построении). Узлы порта от общего прохода
	/// рассеяния защищают `SaveInputNodes`/`RestoreInputNodes` (массив `prInputSave`).
	_ELEM_VAL_RTH rU_Cur;	// curretn voltage on dielectric
	/// Не используется в расчёте (см. `rU_Cur`).
	_ELEM_VAL_RTH rU_Cur1;	// curretn+1 takt voltage on dielectric
	/// Не используется в расчёте (см. `rU_Cur`).
	_ELEM_VAL_RTH rU_Cur2;	// curretn+2 takt voltage on dielectric
	/// Не используется в расчёте (см. `rU_Cur`).
	_ELEM_VAL_RTH rU_Cur3;	// curretn+3 takt voltage on dielectric
	/// Вес узла; смысл зависит от блока. Диэлектрик и плазма — доля четырёх соседних ячеек
	/// сетки внутри блока (1; 0,5 на стороне; 0,25 в углу прямоугольника; сумма по 0,25 у
	/// круга и многоугольника), для FILE, FILE_N/_B/_Y — уже умноженное на вес значение из
	/// файла. Порт — амплитуда профиля основной волны в узле. Поглотитель — проводимость
	/// согласованной нагрузки `y` (магнитная стенка — 0). Металл — не используется.
	_real rY;			// admitance for node
	/// Глобальный номер узла сетки `iy*nX + ix` (64 бита). Напряжения узла —
	/// `prUNode1[6*n + 0..5]`, коэффициенты — `prYNode[6*n + 0..5]`. -1 при построении —
	/// узел отброшен (повтор или вне области).
	TmcIdx nNodeGlobal;  // number of node in global topology	// INT64
	/// Код узла (`tmcgrviw.h`): `CTMCRTH_BLCKNTYPE_EPS` (0) — диэлектрик или плазма, `_METAL`
	/// (101) — металл, `_INPXLEFT`..`_INPYBOT` (102..105) — входы; у поглотителя и магнитной
	/// стенки — код соседства `CTMCRTH_BLCKNTYPE_ABSR_*` (1..16, 21..24). Старое описание
	/// справа (0, 1, -1 … -6) не соответствует текущим значениям констант.
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

/// Задание быстрого построения блока eps/плазмы (EPSFAST); определено в `tmcrthnodediel.cpp`.
struct sEpsFastJob;	// EPSFAST

/// @brief Блок задания X-ядра: строит свои узлы на сетке и выполняет свои граничные условия.
///
/// Один объект — один блок секции `#LINK_LIST`. Массив объектов создаёт
/// `CTmcRTHRectNode::InitDielArray` (по числу блоков) перед расчётом шага `#STEP`
/// и удаляет `DeleteDielArray` при следующем чтении задания. `SetBlock` строит
/// узлы блока и заносит вклад диэлектрика или плазмы (Delta eps, N, B, Y, sigma) в свою
/// ячейку общего массива `prYNode`; затем на каждом из 4 подтактов ядро вызывает
/// `ExciteInputs`..`ExciteInputs3` (порты), `SaveInputNodes`/`RestoreInputNodes`
/// (защита узлов порта от общего прохода рассеяния) и `RunBlockNode`..`RunBlockNode3`
/// (металл, поглотитель, магнитная стенка). Узел X-ядра — компонента Hz, поэтому
/// блок METAL физически — магнитная стенка, а MAGNETIC — металл.
/// Класс наследует `CWnd` только по историческим причинам: окна не создаёт.
/// Не потокобезопасен; все методы вызываются из расчётного потока, кроме
/// `EpsFastRun`, который выполняется в рабочих потоках EPSFAST.
/// @see https://www.tamic.ru/docs/code/kernel-x.html#blocks
class CTmcRTHNodeDiel : public CWnd
{
// Construction						 
public:
	/// @brief Отдаёт модулю расчёта на видеокарте тип блока, число и массив его узлов (только чтение).
	/// @param[out] nType1 тип блока `CTMCRTH_BLCKNTYPE_*`
	/// @param[out] nNum1  число узлов блока (0 — у стационарного диэлектрика и плазмы после EPSFAST)
	/// @param[out] p1     массив узлов блока; владеет блок
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#gpu
	void GpuView( int &nType1, TmcIdx &nNum1, sTmcRTHNodeDielOne *&p1 ) { nType1 = nType; nNum1 = nNumNode; p1 = pcNodeDielOne; }	// GPU: read access for TmcGpuX.h
	/// @brief Б-1: блок — порт плоской волны (`INPUT_X`/`INPUT_Y MAGNETIC`), который сам пересчитывает все свои узлы.
	/// @return TRUE — порт MAGNETIC; FALSE — волноводный порт или не порт.
	BOOL IsPlaneWavePort( void );
	/// @brief Б-1: ещё раз восстанавливает узлы порта плоской волны после блоков METAL/ABSORBER/MAGNETIC.
	///
	/// В общем узле стенки и края порта остаётся значение порта (как в ядре H, где порт и стенка пишут
	/// разные ячейки подтакта). Значения — сохранённые `SaveInputNodes` после возбуждения порта.
	/// @param[in,out] pr  массив напряжений сетки (6 ячеек на узел)
	void RestorePlaneWavePortNodes( _ELEM_VAL_RTH *pr );
	/// @brief Б-8.2: узел поглотителя или стенки MAGNETIC в подмагниченной плазме — по (7.55) диссертации.
	///
	/// Четырёхпараметрическая схема (7.15) с весами линий узла границы, приведёнными к объёмному узлу,
	/// полными шлейфами y5, y6, y7 и нагрузкой (4*w/Lw)*y0. Только при Y3 != 0 (поле B в узле); иначе узел
	/// считает прежний код (без поля схемы совпадают).
	/// @param[in,out] prUNode1 напряжения сетки (6 ячеек на узел)
	/// @param[in]     prYNode  коэффициенты узлов
	/// @param[in]     nX       узлов по X
	/// @param[in]     i        номер узла блока
	/// @param[in]     k        подтакт 0..3 (пишется ячейка k, читается (k+3)%4)
	/// @return TRUE — узел посчитан здесь; FALSE — узел без поля или тип не по (7.55).
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#absorber
	BOOL RunWallNodePlasma( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, TmcIdx i, int k );
	/// @brief Б-8.2: есть ли в блоке ABSORBER/MAGNETIC узлы подмагниченной плазмы (тогда расчёт — на процессоре).
	/// @param[in] prYNode коэффициенты узлов сетки (6 на узел; ячейка 3 = Y3, не 0 — в узле поле B)
	/// @return TRUE — есть хотя бы один такой узел; FALSE — нет или блок другого типа.
	BOOL HasMagnetizedWallNodes( _real *prYNode );
	/// Б-8.2: история узлов (7.55) обнулена (первый подтакт шага).
	BOOL m_bB82Init;
	/// @brief Обновляет узлы порта на подтакте 0 (ячейки `prUNode1[6n + 0]`) и пишет строку
	///        «блок, Up, Uo» в `.t`.
	///
	/// Только для блоков-входов (`INPXLEFT`..`INPYBOT`), у прочих ничего не делает.
	/// Шаги: обнуляет четыре ячейки напряжений узла позади порта, если порт стоит не на
	/// краю области; вычисляет новое напряжение каждого узла порта по двухслойной схеме
	/// узла с согласованной нагрузкой `y1` (`dY1_InputAdmitance`), шлейфом среды порта
	/// `dYInput` и источником `rY*a(t)`: соседи и шлейф берутся из ячейки 3 (`U^(tau-1)`),
	/// `U^(tau-2)` — из ячейки 2, вычитается падающее напряжение `rU2` подтакта 2; запоминает
	/// падающее напряжение в `rU`; вычисляет отражённую волну
	/// `Uo = sum(rY*(U - rU)) / sum(rY^2)` и падающую `Up = a(t)` и выводит их.
	/// Сигнал `a(t) = Amp(t,f)*sin(omega*t + Faza)` или значение из файла `.sig`;
	/// вне окна Tmin..Tmax порта `a = 0`.
	/// @param[in,out] cError1 общий объект ошибки ядра: при уже взведённой ошибке ничего
	///                не делается; ошибка блока (в т. ч. «меньше 3 узлов») копируется сюда
	/// @param[in]  dWT       omega*t, рад (omega = 2*pi*rFreq)
	/// @param[in]  dT        текущее время t, с — по нему проверяется окно Tmin..Tmax порта
	///                       (несмотря на имя, это не шаг по времени)
	/// @param[in,out] pr1    массив напряжений сетки `prUNode1`, 6 ячеек на узел
	/// @param[in]  nX        число узлов сетки по X
	/// @param[in]  cParam    параметры `#PARM` (единицы, частота, шаг)
	/// @param[in]  dtCurrent текущее время, с — для выражений Amp/Faza и файла `.sig`
	/// @see https://www.tamic.ru/docs/theory/ports.html#transformer
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#ports
	void ExciteInputs(  CTmcLibError &cError1, double dWT, double dT, _ELEM_VAL_RTH *pr1, int nX, CTmcRTH_IndanParam &cParam, double dtCurrent );
	/// @brief То же, что `ExciteInputs`, для подтакта 1: пишет ячейку 1, соседи — из ячейки 0,
	///        `U^(tau-2)` — из ячейки 3, вычитается `rU3`; падающее напряжение запоминается в `rU1`.
	/// @param[in,out] cError1 общий объект ошибки ядра
	/// @param[in]  dWT       omega*t, рад
	/// @param[in]  dT        текущее время t, с (окно Tmin..Tmax порта)
	/// @param[in,out] pr1    массив напряжений сетки `prUNode1`, 6 ячеек на узел
	/// @param[in]  nX        число узлов сетки по X
	/// @param[in]  cParam    параметры `#PARM`
	/// @param[in]  dtCurrent текущее время, с
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#ports
	void ExciteInputs1( CTmcLibError &cError1, double dWT, double dT, _ELEM_VAL_RTH *pr1, int nX, CTmcRTH_IndanParam &cParam, double dtCurrent );
	/// @brief То же, что `ExciteInputs`, для подтакта 2: пишет ячейку 2, соседи — из ячейки 1,
	///        `U^(tau-2)` — из ячейки 0, вычитается `rU`; падающее напряжение запоминается в `rU2`.
	/// @param[in,out] cError1 общий объект ошибки ядра
	/// @param[in]  dWT       omega*t, рад
	/// @param[in]  dT        текущее время t, с (окно Tmin..Tmax порта)
	/// @param[in,out] pr1    массив напряжений сетки `prUNode1`, 6 ячеек на узел
	/// @param[in]  nX        число узлов сетки по X
	/// @param[in]  cParam    параметры `#PARM`
	/// @param[in]  dtCurrent текущее время, с
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#ports
	void ExciteInputs2( CTmcLibError &cError1, double dWT, double dT, _ELEM_VAL_RTH *pr1, int nX, CTmcRTH_IndanParam &cParam, double dtCurrent );
	/// @brief То же, что `ExciteInputs`, для подтакта 3: пишет ячейку 3, соседи — из ячейки 2,
	///        `U^(tau-2)` — из ячейки 1, вычитается `rU1`; падающее напряжение запоминается в `rU3`.
	/// @param[in,out] cError1 общий объект ошибки ядра
	/// @param[in]  dWT       omega*t, рад
	/// @param[in]  dT        текущее время t, с (окно Tmin..Tmax порта)
	/// @param[in,out] pr1    массив напряжений сетки `prUNode1`, 6 ячеек на узел
	/// @param[in]  nX        число узлов сетки по X
	/// @param[in]  cParam    параметры `#PARM`
	/// @param[in]  dtCurrent текущее время, с
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#ports
	void ExciteInputs3( CTmcLibError &cError1, double dWT, double dT, _ELEM_VAL_RTH *pr1, int nX, CTmcRTH_IndanParam &cParam, double dtCurrent );
	/* BUG12 portfix: exclude input-port nodes from scattering */
	/// @brief Запоминает все 6 ячеек напряжений узлов порта сразу после их обновления (`ExciteInputs*`).
	///
	/// Только для блоков-входов. Буфер `prInputSave` (6 значений на узел порта) выделяется
	/// при первом вызове. Вместе с `RestoreInputNodes` исключает узлы порта из общего
	/// прохода рассеяния: порт пересчитывает их сам.
	/// @param[in] pr массив напряжений сетки `prUNode1`, 6 ячеек на узел
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#port-nodes
	void SaveInputNodes( _ELEM_VAL_RTH *pr );
	/// @brief Возвращает узлам порта значения, сохранённые `SaveInputNodes` (после общего прохода рассеяния).
	///
	/// Только для блоков-входов и только если буфер `prInputSave` уже есть.
	/// @param[in,out] pr массив напряжений сетки `prUNode1`, 6 ячеек на узел
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#port-nodes
	void RestoreInputNodes( _ELEM_VAL_RTH *pr );
	/// Сохранённые ячейки узлов порта: 6 значений на узел, длина 6*`nNumNode`; NULL — ещё
	/// не выделен. Владеет блок, освобождает `DeleteData`.
	_ELEM_VAL_RTH *prInputSave;
	/// @brief Строит узлы блока задания по его типу и форме и вносит вклад блока в массив `prYNode`.
	///
	/// Читает из блока тип, выражения и смещение X0, Y0 и вызывает построитель:
	/// металл — `SetRectMetal`/`SetPolyMetal`/`SetCircMetal`, поглотитель —
	/// `Set*Absorber`, магнитная стенка — `Set*Magnetic`, диэлектрик и плазма —
	/// `SetRectFile` (FILE, FILE_N, FILE_B, FILE_Y), `SetStatEpsFast` (стационарные
	/// RECT/CIRCLE/POLYGON_STAT и варианты `_N`, `_B`, `_Y`, `_S`, если включён EPSFAST)
	/// или `SetRect`/`SetCirc`/`SetPoly`, вход — `SetInp*`. Затем проверяет, что узлы
	/// (и соседи узлов порта) лежат в сетке, убирает повторы, задаёт коды узлов,
	/// прибавляет значение выражения к своей ячейке `prYNode[6n + k]` (`SetYLineForDiel`;
	/// k по `EpsFastTypeOffset`: 0 — Delta eps, 1 — N, 2 — Y, 3 — sigma, 4 — B), строит
	/// профиль порта (`SetYForInput`) и освобождает массив узлов стационарного блока.
	/// Вызывается один раз на блок при подготовке шага `#STEP`.
	/// @param[in]  pcNextBlockList блок задания (TMCIndan); величины в СИ
	/// @param[in]  cParam          параметры сетки: шаг rDelta (м), область, частота (Гц)
	/// @param[in]  cOut            вывод шага: имя файла `.t`, куда порт пишет сигналы
	/// @param[in,out] prYNode      массив коэффициентов ядра, 6 ячеек на узел: в ячейку блока
	///                             прибавляется «вес * значение выражения»
	/// @param[in]  pbStopFlag      признак «расчёт идёт» ядра (`bIsReadData`); FALSE — прервать
	/// @param[in]  csCurrentPath   папка задания: в ней ищутся и создаются файлы `.eps` и `.sig`
	/// @return Ссылка на объект ошибки блока; `IsError()` == FALSE — успех.
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#plasma-blocks
	CTmcLibError &SetBlock( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut, _real *prYNode, volatile BOOL *pbStopFlag, CString csCurrentPath );
	/// @brief Освобождает узлы блока, буфер узлов порта и данные файла `.sig`, сбрасывает
	///        ошибку и поля в исходное состояние (тип блока — металл).
	void DeleteData( void );
	/// @brief Создаёт пустой блок без узлов.
	CTmcRTHNodeDiel();
	/// @brief Подключает счётчики замера времени (ключ /Time): запись сигналов портов и файлов `.eps`.
	/// @param[in] pqSignalFile счётчик тактов QPC записи сигналов в `.t`; NULL — не считать
	/// @param[in] pqEpsFile    счётчик тактов QPC записи файлов `.eps`; NULL — не считать
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#timing
	void SetTimingCounters( __int64 *pqSignalFile, __int64 *pqEpsFile ) { m_pqSignalFileTicks = pqSignalFile; m_pqEpsFileTicks = pqEpsFile; }	// TIMING
	/// @brief Подключает счётчики чтения файлов распределения (блоки FILE*) и числа файлов `.eps`.
	/// @param[in] pqEpsRead    счётчик тактов QPC чтения блоков FILE*; NULL — не считать
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
	/// @brief То же, что `RunBlockNode`, для подтакта 3: пишет ячейку 3, соседи и шлейф — из ячейки 2;
	///        в `rU` запоминается ячейка 2.
	/// @param[in,out] prUNode1  массив напряжений сетки, 6 ячеек на узел
	/// @param[in]  prYNode   массив коэффициентов (`prYNode[6n + 1]` — шлейф узла)
	/// @param[in]  nX        число узлов сетки по X
	/// @param[in]  nArraySize длина массива `prUNode1` (6*nX*nY) — для проверки соседей
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#absorber
	void RunBlockNode3( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, TmcIdx nArraySize );
	/// @brief То же, что `RunBlockNode`, для подтакта 2: пишет ячейку 2, соседи и шлейф — из ячейки 1;
	///        в `rU` запоминается ячейка 1.
	/// @param[in,out] prUNode1  массив напряжений сетки, 6 ячеек на узел
	/// @param[in]  prYNode   массив коэффициентов (`prYNode[6n + 1]` — шлейф узла)
	/// @param[in]  nX        число узлов сетки по X
	/// @param[in]  nArraySize длина массива `prUNode1` (6*nX*nY) — для проверки соседей
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#absorber
	void RunBlockNode2( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, TmcIdx nArraySize );
	/// @brief То же, что `RunBlockNode`, для подтакта 1: пишет ячейку 1, соседи и шлейф — из ячейки 0;
	///        в `rU` запоминается ячейка 0.
	/// @param[in,out] prUNode1  массив напряжений сетки, 6 ячеек на узел
	/// @param[in]  prYNode   массив коэффициентов (`prYNode[6n + 1]` — шлейф узла)
	/// @param[in]  nX        число узлов сетки по X
	/// @param[in]  nArraySize длина массива `prUNode1` (6*nX*nY) — для проверки соседей
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#absorber
	void RunBlockNode1( _ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, TmcIdx nArraySize );
	/// @brief Выполняет граничное условие блока после рассеяния подтакта 0 (ячейка `prUNode1[6n + 0]`).
	///
	/// Металл — обнуляет напряжение узлов блока (в X-ядре это Hz = 0, то есть физически
	/// магнитная стенка). Поглотитель и магнитная стенка — пересчитывают узлы контура по
	/// коду соседства по двухслойной схеме, как в H-ядре (полные и половинные линии, доля
	/// шлейфа `y5 = prYNode[6n + 1]`, нагрузка `y = rY`, `U^(tau-1)` — ячейка 3,
	/// `U^(tau-2)` — `rU`), внутренние узлы (код 1111 и 0000) обнуляют; затем запоминают
	/// в `rU` ячейку 3 для следующего подтакта. Ячейки плазмы узла (y6, y7) не учитываются.
	/// У диэлектрика и входов ничего не делает.
	/// @param[in,out] prUNode1  массив напряжений сетки, 6 ячеек на узел
	/// @param[in]  prYNode   массив коэффициентов
	/// @param[in]  nX        число узлов сетки по X
	/// @param[in]  nArraySize длина массива `prUNode1` (6*nX*nY); узлы, у которых сосед
	///                        выходит за массив, пропускаются
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#absorber-x
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#duality-x
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#absorber
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
	/// W — Delta eps среды в сечении входа (eps - 1). Правило то же, что для узлов H-ядра:
	/// W >= 0 — `dYInput = 4*W` (значения W в пределах 0,004 от 4 сдвигаются до 3,996 или
	/// 4,004), W < 0 — `dYInput = W*(k0*Delta)^2/2`, k0 = 2*pi*f/c; это совпадает с
	/// коэффициентом y5 узла эквивалентной плазмы X-ядра при B = 0. Пустое W — 0.
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
	/// @brief Выделяет из выражения блока вида `[имя]выражение` имя файла `.eps` для записи.
	///
	/// Имя без скобок попадает в `csFileNameEps` (с расширением и папкой задания),
	/// в `csEpsExpr` остаётся выражение после `]`. Без `[` в начале — ничего не делает;
	/// без `]` — ошибка.
	/// @param[in] csCurrentPath папка задания
	void SeachEpsFileName( CString csCurrentPath );
	/// Полное имя файла `.eps`, куда блок RECT_STAT* пишет своё распределение; пусто — не писать.
	CString csFileNameEps;
// --- incident wave from file (wideband excitation) ---------------------
	/// @brief Если Amp порта задано как `[имя]`, находит и читает файл падающей волны `.sig`.
	///
	/// Без точки в имени добавляется «.sig», без пути — папка задания. Иначе снимает
	/// признак `bSigFile`.
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#signal-file
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
	/// @brief Освобождает массив узлов стационарного диэлектрика или плазмы после занесения
	///        значений в `prYNode`.
	///
	/// Для блоков *_MOVE и не диэлектриков ничего не делает.
	void DeleteStatDielData( void );
	/// Признак «расчёт идёт» ядра (`CTmcRTHRectNode::bIsReadData`); `*pbIsStop` == FALSE — прервать построение.
	volatile BOOL * pbIsStop;
	/// Счётчик /Time: такты QPC записи сигналов портов; NULL — не считать.
	__int64 * m_pqSignalFileTicks;	// TIMING: time of writing port signals (QPC ticks)
	/// Счётчик /Time: такты QPC записи файлов `.eps`; NULL — не считать.
	__int64 * m_pqEpsFileTicks;		// TIMING: time of writing eps-distribution files
	/// Счётчик /Time: такты QPC чтения блоков FILE*; NULL — не считать.
	__int64 * m_pqEpsReadTicks;		// TIMING: time of FILE blocks (reading distribution files)
	/// Счётчик /Time: число записанных файлов `.eps`; NULL — не считать.
	int * m_pnEpsWritten;			// TIMING: number of blocks that wrote a distribution file
	/// Счётчик /Time: число прочитанных файлов `.eps`; NULL — не считать.
	int * m_pnEpsRead;				// TIMING: number of blocks read from a distribution file
	/// @brief Прибавляет время чтения блока FILE* и (при успехе) единицу к числу прочитанных файлов.
	/// @param[in] qTicks длительность чтения, такты QPC
	void TimingEpsRead( __int64 qTicks ) { if( m_pqEpsReadTicks != NULL ) *m_pqEpsReadTicks += qTicks; if( ( m_pnEpsRead != NULL )&&( !cError.IsError() ) ) (*m_pnEpsRead)++; }	// TIMING
	/// @brief Записывает распределение блока RECT_STAT* в файл `.eps`, если в выражении задано `[имя]`.
	///
	/// Заголовок — строки с метками `TMC_RTH_EPSFILE_*` (CR LF), в строке типа — ключевое
	/// слово блока (RECT_STAT, RECT_STAT_N, _B, _Y …), затем для каждого узла блока (строки
	/// снизу вверх, в строке слева направо) — значение «вес * значение выражения» типа `_real`
	/// и 4-байтовый локальный номер узла. Координаты — локальные координаты блока.
	/// При остановке файл удаляется.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @param[in] csCurrentPath   папка задания
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#eps-file
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
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#absorber
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
	/// Нагрузка `y` = 0 (холостой ход); в X-ядре такая граница физически — металл.
	/// Коды узлов — `SetAbsorberNodeType` и `SetMagneticPolygonType`. Части вне области
	/// отбрасываются.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#duality-x
	CTmcLibError & SetPolyMagnetic( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит поглотитель по контуру многоугольника: полоса 2x2 узла вдоль ломаной.
	///
	/// Нагрузка узлов `y = SetAbsorberAdmitance(...)`; точки ближе 1–2 шагов к краю
	/// области прижимаются внутрь, части вне области отбрасываются; коды — `SetAbsorberNodeType`.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#absorber-x
	CTmcLibError & SetPolyAbsorber( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит магнитную стенку по прямоугольнику: внешний контур и кольцо внутри (код 0000).
	///
	/// Нагрузка `y` = 0; в X-ядре такая граница физически — металл. Узлы ставятся
	/// `CalcnNodeGlobalFM`; у края области к коду угла и стороны добавляется бит стороны края.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#duality-x
	CTmcLibError & SetRectMagnetic( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит поглотитель по прямоугольнику: внешний контур с кодами сторон и углов
	///        и кольцо узлов внутри с кодом 0000 (обнуляются).
	///
	/// Нагрузка узлов `y = SetAbsorberAdmitance(...)`. Прямоугольник не подходит к краям
	/// области ближе 1 шага слева/снизу и 2 шагов справа/сверху; размер — не меньше 2 шагов.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#absorber-x
	CTmcLibError & SetRectAbsorber( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит металл по контуру многоугольника: ступенчатая ломаная из узлов (4-связная).
	///
	/// Узлы вне области отбрасываются. Блок METAL обнуляет узлы контура (в X-ядре Hz = 0 —
	/// физически магнитная стенка); внутренние узлы отделены им от остальной сетки.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#staircase
	CTmcLibError & SetPolyMetal(CTmcRTH_BlockList *pcNextBlockList, CTmcRTH_IndanParam &cParam);
	/// @brief Строит блок METAL по контуру прямоугольника (4 стороны, части вне области обрезаются).
	///
	/// В X-ядре узлы контура получают Hz = 0 — физически магнитная стенка.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#duality-x
	CTmcLibError & SetRectMetal( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Вычисляет проводимость согласованной нагрузки порта `y1` для основной волны.
	///
	/// `a = sqrt(2)*pi*Delta*f/c` (omega*tau), `arg = (2 + |yW|/2)*cos(a) - yW/2 - cos(pi*Delta/b)`,
	/// `y1 = sin(acos(arg))/sin(a)`, где yW = `dYInput`, b = `dWidthWaveg`. Если |arg| > 1
	/// (частота ниже отсечки порта) — ошибка в `cError`. Формула та же, что в H-ядре.
	/// @param[in] dFreq  частота задания, Гц
	/// @param[in] dDelta шаг сетки, м
	/// @return Нормированная проводимость нагрузки.
	/// @see https://www.tamic.ru/docs/theory/ports.html#transformer
	double SetInputAdmitance( double dFreq, double dDelta );
	/// @brief Вычисляет проводимость нагрузки поглотителя (и порта MAGNETIC) для плоской волны,
	///        падающей по нормали.
	///
	/// Как `SetInputAdmitance`, но без поперечной вариации: `arg = (2 + |yW|/2)*cos(a) - 1 - yW/2`.
	/// У блоков поглотителя `dYInput` = 0 (свободное пространство): плазма под поглотителем
	/// и угол падения не учитываются.
	/// @param[in] dFreq  частота задания, Гц
	/// @param[in] dDelta шаг сетки, м
	/// @return Нормированная проводимость нагрузки.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#absorber-x
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
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#t-file
	void OutputUpUo( double dUp, double dUo, CTmcLibError &cError1);
	/// @brief Вычисляет выражение блока в точке.
	///
	/// В выражение подставляются x, y (в LONG_UNIT, относительно X0, Y0 блока),
	/// r = sqrt(x^2 + y^2) и f (полярный угол в ANGLE_UNIT). Смысл значения зависит от
	/// блока: Delta eps (eps - 1) у RECT/CIRCLE/POLYGON_STAT, концентрация N (м^-3) у `_N`,
	/// индукция B (Тл) у `_B`, сеточные потери Y (безразмерные) у `_Y`, sigma (См/м) у `_S`.
	/// @param[in] x      координата узла относительно X0 блока, м
	/// @param[in] y      координата узла относительно Y0 блока, м
	/// @param[in] cParam параметры задания (единицы)
	/// @return Значение выражения; при ошибке — 0 и ошибка в `cError`.
	/// @see https://www.tamic.ru/docs/theory/plasma.html#units
	_real CalcEps( double x, double y, CTmcRTH_IndanParam &cParam );
	// EPSFAST: fast build of stationary eps blocks (TmcRTHNodeDiel.cpp)
	/// @brief Быстро строит стационарный блок (EPSFAST) прямо в `prYNode`, без массива узлов.
	///
	/// Тот же результат, что `SetRect`/`SetCirc`/`SetPoly` + `SetYLineForDiel`, бит в бит:
	/// выражение без x, y, r, f считается один раз; с ними — в нескольких потоках
	/// (если узлов в описанном прямоугольнике не меньше порога, по умолчанию 1000);
	/// с `=` — в одном потоке в исходном порядке узлов. При любом нестандартном случае
	/// (повтор узлов сетки, нет узлов) возвращает FALSE, и блок строится прежним кодом.
	/// @param[in]  pcNextBlockList блок задания
	/// @param[in]  cParam          параметры сетки
	/// @param[in]  csCurrentPath   папка задания (файл `.eps` у RECT_STAT*)
	/// @param[in,out] prYNode      массив, куда прибавляется вклад узлов
	/// @param[in]  nShape          форма: `EPSFAST_RECT`, `EPSFAST_CIRC`, `EPSFAST_POLY`
	/// @param[in]  nStride         шаг массива на узел (6 в X-ядре)
	/// @param[in]  nOffset         ячейка величины в записи узла (`EpsFastTypeOffset`: 0..4)
	/// @return TRUE — блок построен (или причина ошибки в `cError`); FALSE — ничего не сделано.
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#epsfast
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
	/// @brief Вычисляет выражение блока в точке без `cError` (для рабочих потоков), как `CalcEps`.
	/// @param[in]  pszExpr выражение
	/// @param[in]  x       координата относительно X0 блока, м
	/// @param[in]  y       координата относительно Y0 блока, м
	/// @param[in]  cParam  параметры задания
	/// @param[out] szBuf   рабочий буфер `CTMCRTH_INDANSBUF` символов
	/// @param[out] szBuf1  второй рабочий буфер (не используется)
	/// @param[out] pr      значение выражения
	/// @return 0 — успех, -1 — ошибка выражения.
	int  EpsFastEval( const char *pszExpr, double x, double y, CTmcRTH_IndanParam &cParam, char *szBuf, char *szBuf1, _real *pr );
	/// @brief Классифицирует выражение блока для EPSFAST.
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
	/// @brief Возвращает ячейку записи узла `prYNode[6n + k]`, куда блок заносит своё значение.
	///
	/// Определяется по ключевому слову блока один раз на блок (используется и EPSFAST,
	/// и `SetYLineForDiel`).
	/// @return 0 — Delta eps (FILE, RECT/CIRCLE/POLYGON_STAT); 1 — концентрация N, м^-3
	///         (`_N`, FILE_N); 2 — сеточные потери Y (`_Y`, FILE_Y); 3 — sigma, См/м (`_S`);
	///         4 — индукция B, Тл (`_B`, FILE_B).
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#plasma-blocks
	int  EpsFastTypeOffset( void );
	/// @brief Записывает тип блока в узлы (кроме поглотителя и магнитной стенки) и обнуляет
	///        их состояние (`rU`..`rU3`, `rU_Cur`..`rU_Cur3`).
	void SetnType( void );
	/// @brief Прибавляет к ячейке блока `prYNode[6n + k]` узлов «вес * значение выражения».
	///
	/// Ячейка k — `EpsFastTypeOffset` (Delta eps, N, Y, sigma или B). Для FILE, FILE_N/_B/_Y
	/// берёт готовое значение `rY` из файла. Выражение без x, y, r, f при включённом EPSFAST
	/// считается один раз. Перевод в коэффициенты узла делает потом
	/// `CTmcRTHRectNode::SetDielInNodeList`.
	/// @param[in] cParam параметры задания
	/// @param[in,out] prYNode массив коэффициентов ядра, 6 ячеек на узел
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#plasma-blocks
	void SetYLineForDiel( CTmcRTH_IndanParam &cParam, _real *prYNode );
	/// @brief Вычисляет профиль порта `rY`: sin(i*pi/(N-1)) (стенки — блоки METAL, Hz = 0 на
	///        краях) или постоянный (порт MAGNETIC), нормированный множителем `dUnormir`.
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
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#ports
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
	/// @brief Строит узлы многоугольника диэлектрика или плазмы (прежний путь без EPSFAST).
	///
	/// Узел внутри, если попадает в один из треугольников веера от первой вершины;
	/// вес — по 0,25 за каждую диагональную соседнюю точку внутри.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	CTmcLibError &SetPoly( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит узлы кольцевого сектора диэлектрика или плазмы (Rmin..Rmax, FImin..FImax).
	///
	/// Вес — по 0,25 за каждую диагональную соседнюю точку внутри сектора.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	CTmcLibError &SetCirc( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит магнитную стенку по контуру кольцевого сектора (два радиуса и две дуги, полоса 2x2).
	///
	/// В X-ядре такая граница физически — металл.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#duality-x
	CTmcLibError &SetCircMagnetic( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит поглотитель по контуру кольцевого сектора (два радиуса и две дуги, полоса 2x2).
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#absorber-x
	CTmcLibError &SetCircAbsorber( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит блок METAL по контуру кольцевого сектора: два радиальных отрезка и две дуги.
	///
	/// В X-ядре узлы контура получают Hz = 0 — физически магнитная стенка.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/theory/boundaries.html#staircase
	CTmcLibError &SetCircMetal( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam );
	/// @brief Строит узлы прямоугольника диэлектрика или плазмы (прежний путь без EPSFAST)
	///        и пишет `.eps`, если задано.
	///
	/// Вес узла: 1 внутри, 0,5 на стороне, 0,25 в углу. Части вне области обрезаются.
	/// @param[in] pcNextBlockList блок задания
	/// @param[in] cParam          параметры сетки
	/// @param[in] csCurrentPath   папка задания (для файла `.eps`)
	/// @return Ссылка на объект ошибки блока.
	CTmcLibError &SetRect( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CString csCurrentPath );
	/// @brief Строит блок FILE, FILE_N, FILE_B или FILE_Y: читает распределение из файла `.eps`
	///        и ставит узлы на сетку.
	///
	/// Ищет файл как задан, затем в папке задания и с расширением «.eps». Проверяет
	/// метки заголовка, тип (FILE — RECT_STAT; FILE_N/_B/_Y — RECT_STAT или RECT_STAT_N/_B/_Y
	/// соответственно, иначе ошибка с причиной), совпадение шага, точность 4 или 8 байт.
	/// Узлы, не попавшие в область, пропускаются. Значение из файла (вес * величина)
	/// становится `rY` узла и попадает в ячейку N, B или Y по типу блока.
	/// @param[in] pcNextBlockList блок задания; его Xmin..Ymax переписываются по файлу
	/// @param[in] cParam          параметры сетки
	/// @param[in] csCurrentPath   папка задания
	/// @return Ссылка на объект ошибки блока.
	/// @see https://www.tamic.ru/docs/code/kernel-x.html#eps-file
	CTmcLibError &SetRectFile( CTmcRTH_BlockList * pcNextBlockList, CTmcRTH_IndanParam &cParam, CString csCurrentPath );
	/// Ошибка блока; методы при взведённой ошибке ничего не делают.
	CTmcLibError cError;
	/// Номер блока из `#LINK_LIST`.
	int nBlock;							// block number
	/// Ключевое слово примитива (RECT_STAT_N, FILE …); у входа после построения — имя файла `.t`.
	CString csBlock;					// block Type
	/// Число узлов блока (длина `pcNodeDielOne`); 0 у стационарного диэлектрика и плазмы
	/// после построения.
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
	/// Выражение блока (Delta eps, N, B, Y или sigma); у FILE* — имя файла; у металла и др. —
	/// слово типа (у порта — METAL или MAGNETIC: тип боковых стенок).
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
	/// Тип блока `CTMCRTH_BLCKNTYPE_*` (`tmcgrviw.h`): EPS 0 (диэлектрик и плазма), ABSORBER 100,
	/// METAL 101, INPXLEFT..INPYBOT 102..105, MAGNETIC 107. Старое описание справа устарело.
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
