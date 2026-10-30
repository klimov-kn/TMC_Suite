// TmcRTH_IndanParam.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_IndanParam window

/// @file TmcRTH_IndanParam.h
/// @brief Параметры расчёта из секции `#PARM` задания: единицы, шаг сетки, область,
///        время, частота; пересчёт координат в номера узлов сетки.
///
/// Часть библиотеки TMCIndan. Объявляет ключевые слова секции `#PARM`,
/// названия единиц измерения с множителями перевода в СИ, класс
/// `CTmcRTH_IndanParam` и функции механического шага по времени.
/// Все величины внутри класса хранятся в СИ (м, с, Гц, радианы).
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#param

#ifndef CTMCRTH_INDANPARAM_H__
#define CTMCRTH_INDANPARAM_H__

#include <Typerth.h>
#include <TmcLibError.h>
#include <TmcIdx.h>	// INT64

/// Ключевое слово единицы длины в `#PARM`: `LONG_UNIT m|cm|mm;` (по умолчанию mm).
/// Единица длины действует на все координаты и размеры задания, читаемые после неё.
#define CTMCRTH_LONGUNIT_ID  "LONG_UNIT "
// Пары CTMCRTH_<ВЕЛИЧИНА>UNIT_<имя> / _U<имя>: имя единицы в задании и множитель
// перевода значения в СИ (м, с, радианы, Гц, Тл, м^-3).
/// Имя единицы длины «метр».
#define CTMCRTH_LONGUNIT_m  "m"
/// Метров в единице «m».
#define CTMCRTH_LONGUNIT_Um  1.
/// Имя единицы длины «сантиметр».
#define CTMCRTH_LONGUNIT_cm "cm"
/// Метров в единице «cm».
#define CTMCRTH_LONGUNIT_Ucm 0.01
/// Имя единицы длины «миллиметр» (по умолчанию).
#define CTMCRTH_LONGUNIT_mm "mm"
/// Метров в единице «mm».
#define CTMCRTH_LONGUNIT_Umm 0.001

/// Ключевое слово единицы времени: `TIME_UNIT s|ms|mks|ns|ps;` (по умолчанию s).
#define CTMCRTH_TIMEUNIT_ID  "TIME_UNIT "
/// Имя единицы времени «секунда» (по умолчанию).
#define CTMCRTH_TIMEUNIT_s  "s"
/// Секунд в единице «s».
#define CTMCRTH_TIMEUNIT_Us  1.
/// Имя единицы времени «миллисекунда».
#define CTMCRTH_TIMEUNIT_ms "ms"
/// Секунд в единице «ms».
#define CTMCRTH_TIMEUNIT_Ums 0.001
/// Имя единицы времени «микросекунда».
#define CTMCRTH_TIMEUNIT_mks "mks"
/// Секунд в единице «mks».
#define CTMCRTH_TIMEUNIT_Umks 0.000001
/// Имя единицы времени «наносекунда».
#define CTMCRTH_TIMEUNIT_ns "ns"
/// Секунд в единице «ns».
#define CTMCRTH_TIMEUNIT_Uns 0.000000001
/// Имя единицы времени «пикосекунда».
#define CTMCRTH_TIMEUNIT_ps "ps"
/// Секунд в единице «ps».
#define CTMCRTH_TIMEUNIT_Ups 0.000000000001

/// Ключевое слово единицы угла: `ANGLE_UNIT radian|gradus;` (по умолчанию radian).
/// Переключает также единицу углов в тригонометрических функциях выражений.
#define CTMCRTH_ANGLEUNIT_ID  "ANGLE_UNIT "
/// Имя единицы угла «радиан» (по умолчанию).
#define CTMCRTH_ANGLEUNIT_r  "radian"
/// Радиан в единице «radian».
#define CTMCRTH_ANGLEUNIT_Ur  1.
/// Имя единицы угла «градус».
#define CTMCRTH_ANGLEUNIT_gr "gradus"
/// Радиан в единице «gradus» (pi/180).
#define CTMCRTH_ANGLEUNIT_Ugr (3.141592653589/180.)

/// Ключевое слово единицы частоты: `FREQ_UNIT Hz|kHz|MHz|GHz;` (по умолчанию GHz).
#define CTMCRTH_FREQUNIT_ID  "FREQ_UNIT "
/// Имя единицы частоты «герц».
#define CTMCRTH_FREQUNIT_Hz  "Hz"
/// Герц в единице «Hz».
#define CTMCRTH_FREQUNIT_UHz  1.
/// Имя единицы частоты «килогерц».
#define CTMCRTH_FREQUNIT_kHz "kHz"
/// Герц в единице «kHz».
#define CTMCRTH_FREQUNIT_UkHz 1000.
/// Имя единицы частоты «мегагерц».
#define CTMCRTH_FREQUNIT_MHz "MHz"
/// Герц в единице «MHz».
#define CTMCRTH_FREQUNIT_UMHz 1000000.
/// Имя единицы частоты «гигагерц» (по умолчанию).
#define CTMCRTH_FREQUNIT_GHz "GHz"
/// Герц в единице «GHz».
#define CTMCRTH_FREQUNIT_UGHz 1000000000.

/// Ключевое слово единицы индукции: `B_UNIT Tl|mTl|mkTl;` (по умолчанию Tl).
/// Разбирается, но ядрами не применяется: B в блоках `*_B` всегда в Тл.
#define CTMCRTH_BUNIT_ID  "B_UNIT "
/// Имя единицы индукции «тесла» (по умолчанию).
#define CTMCRTH_BUNIT_Tl  "Tl"
/// Тесла в единице «Tl».
#define CTMCRTH_BUNIT_UTl  1.
/// Имя единицы индукции «миллитесла».
#define CTMCRTH_BUNIT_mTl "mTl"
/// Имя единицы индукции «микротесла».
#define CTMCRTH_BUNIT_mkTl "mkTl"

/// Ключевое слово единицы концентрации: `N_UNIT m3|cm3|mm3;` (по умолчанию m3, т. е. м^-3).
/// Разбирается, но ядрами не применяется: N в блоках `*_N` всегда в м^-3.
#define CTMCRTH_NUNIT_ID  "N_UNIT "
/// Имя единицы концентрации «на кубический метр» (по умолчанию).
#define CTMCRTH_NUNIT_m3  "m3"
/// Множитель единицы «m3».
#define CTMCRTH_NUNIT_Um3  1.
/// Имя единицы концентрации «на кубический сантиметр».
#define CTMCRTH_NUNIT_cm3 "cm3"
/// Имя единицы концентрации «на кубический миллиметр».
#define CTMCRTH_NUNIT_mm3 "mm3"

/// Ключевое слово рабочей частоты: `FREQ <выражение>;` в единицах `FREQ_UNIT`.
#define CTMCRTH_FREQ_ID   "FREQ "
/// Ключевое слово шага сетки: `DELTA <выражение>;` в единицах `LONG_UNIT`.
#define CTMCRTH_DELTA_ID  "DELTA "
/// Ключевое слово интервала расчёта: `TIME <Tmin>; <Tmax>;` в единицах `TIME_UNIT`.
#define CTMCRTH_TIME_ID   "TIME "
/// Левая граница области расчёта: `X_MIN <выражение>;` в единицах `LONG_UNIT`.
#define CTMCRTH_XMIN_ID   "X_MIN "
/// Правая граница области расчёта: `X_MAX <выражение>;`.
#define CTMCRTH_XMAX_ID   "X_MAX "
// A-13, B-15: bits of the parameters read in the #PARM section of the step (CTmcRTH_IndanParam::nSeenInSection)
/// Бит «X_MIN прочитан в секции шага» (А-13, Б-15).
#define TMC_PARAM_SEEN_XMIN   0x001u
/// Бит «X_MAX прочитан».
#define TMC_PARAM_SEEN_XMAX   0x002u
/// Бит «Y_MIN прочитан».
#define TMC_PARAM_SEEN_YMIN   0x004u
/// Бит «Y_MAX прочитан».
#define TMC_PARAM_SEEN_YMAX   0x008u
/// Бит «DELTA прочитан».
#define TMC_PARAM_SEEN_DELTA  0x010u
/// Бит «TIME прочитан».
#define TMC_PARAM_SEEN_TIME   0x020u
/// Бит «FREQ прочитан».
#define TMC_PARAM_SEEN_FREQ   0x040u
/// Бит «HEIGHT прочитан» (необязательный, единица длины).
#define TMC_PARAM_SEEN_HEIGHT 0x080u
/// Бит «DELTA_T прочитан» (необязательный, единица времени).
#define TMC_PARAM_SEEN_DELTAT 0x100u
/// Параметры, обязательные в каждом шаге (Б-15): X_MIN, X_MAX, Y_MIN, Y_MAX, DELTA, TIME, FREQ.
#define TMC_PARAM_SEEN_REQUIRED 0x07Fu
/// Нижняя граница области расчёта: `Y_MIN <выражение>;`.
#define CTMCRTH_YMIN_ID   "Y_MIN "
/// Верхняя граница области расчёта: `Y_MAX <выражение>;`.
#define CTMCRTH_YMAX_ID   "Y_MAX "
/// Высота структуры: `HEIGHT <выражение>;`. Разбирается, на расчёт не влияет.
#define CTMCRTH_HEIGHT_ID "HEIGHT "
/// Механический шаг по времени: `DELTA_T_MECHANICAL <выражение>;` в единицах `TIME_UNIT`.
/// Разбирается и передаётся в `SetDeltaT`; на расчёт поля не влияет.
#define CTMCRTH_DELTA_T_ID "DELTA_T_MECHANICAL "

/// @brief Задаёт механический шаг по времени и взводит признак «задан».
///
/// Значение хранится в статической переменной `tmcrth_indanparam.cpp`
/// (одна на процесс). Вызывается при разборе `DELTA_T_MECHANICAL`.
/// @param[in] dDeltaT1 шаг, с
void SetDeltaT( double dDeltaT1 );
/// @brief Задаёт механический шаг по времени из текста (например, из командной строки).
/// @param[in] lpszDeltaT число в формате `%lg`, с; при ошибке разбора ничего не меняется
void SetDeltaT( char *lpszDeltaT );

/// @brief Параметры расчёта одного шага #STEP: единицы, сетка, область, время, частота.
///
/// Заполняется построчно методом `Read` при чтении секции `#PARM`
/// (`CTmcRTH_Indan::ReadParamSection`), проверяется и выравнивается по сетке
/// методом `IsCorrectly`. Параметры шага N накапливаются из секций `#PARM`
/// шагов 1..N: следующий шаг меняет только то, что в нём записано.
/// Принадлежит `CTmcRTH_Indan` (поле `cParam`); копии лежат в ядре
/// и в `CFieldIntegrated`. Все длины — в м, времена — в с, частота — в Гц.
/// Сетка: узлы `ix = 0..nX-1`, `iy = 0..nY-1`, координата узла
/// `x = rXmin + ix*rDelta`, глобальный номер `iy*nX + ix`.
/// @note Потоки: не потокобезопасен; читается в потоке чтения задания,
///       далее используется расчётным потоком только для чтения.
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#param
/// @see https://www.tamic.ru/docs/theory/method.html#grid
class CTmcRTH_IndanParam //: public CWnd
{
// Construction
public:
	/// @brief Проверяет, попадает ли точка в область расчёта (с допуском в полшага).
	///
	/// Точка дальше чем на `rDelta/2` за границей области — вне области;
	/// иначе проверяется номер ближайшего узла.
	/// @param[in] x координата X, м
	/// @param[in] y координата Y, м
	/// @return TRUE — точка внутри области.
	BOOL IsnInRegion( _real x, _real y );
	/// @brief Проверяет, что глобальный номер узла лежит в `[0, nX*nY)`.
	/// @param[in] n глобальный номер узла
	/// @return TRUE — номер допустим.
	BOOL IsnInRegion( TmcIdx n );	// INT64
	/// @brief Возвращает глобальный номер узла, ближайшего к точке (x, y).
	///
	/// Точка вне области прижимается к границе. Точка ближе полушага к левому
	/// или правому краю относится к крайнему столбцу узлов (`ix = 0` или `nX-1`).
	/// @param[in] x координата X, м
	/// @param[in] y координата Y, м
	/// @return Номер `iy*nX + ix`, всегда в пределах сетки.
	/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#node-index
	TmcIdx CalcnNodeGlobal( double x, double y );	// INT64
	/// @brief Как `CalcnNodeGlobal`, но точки ближе 1,5 шага к левому или правому
	///        краю относятся ко второму столбцу от края (`ix = 1` или `nX-2`).
	///
	/// Используется ядрами при расстановке узлов блоков на сетке.
	/// @param[in] x координата X, м
	/// @param[in] y координата Y, м
	/// @return Глобальный номер узла.
	TmcIdx CalcnNodeGlobalFM( double x, double y );	// INT64
	/// @brief Возвращает координату Y узла по его глобальному номеру.
	/// @param[in] nNumber глобальный номер узла
	/// @return `rYmin + (nNumber / nX)*rDelta`, м.
	double CalcY( TmcIdx nNumber );	// INT64
	/// @brief Возвращает координату X узла по его глобальному номеру.
	/// @param[in] nNumber глобальный номер узла
	/// @return `rXmin + (nNumber mod nX)*rDelta`, м.
	double CalcX( TmcIdx nNumber );	// INT64
	/// @brief Возвращает границы области расчёта (после выравнивания по сетке).
	/// @param[out] dXmin левая граница, м
	/// @param[out] dXmax правая граница (с резервным столбцом), м
	/// @param[out] dYmin нижняя граница, м
	/// @param[out] dYmax верхняя граница (с резервной строкой), м
	void GetXYMinMax( double &dXmin, double &dXmax, double &dYmin, double &dYmax );
	/// @brief Возвращает шаг сетки.
	/// @return `rDelta`, м.
	double dGetDelta( void );
	/// @brief Возвращает шаг по времени.
	/// @return `rt = rDelta/(c*sqrt(2))`, с.
	double dGetdT( void );
	/// @brief Возвращает начало интервала расчёта.
	/// @return `rTmin`, с.
	double dGetTmin( void );
	/// @brief Возвращает число шагов по времени `(rTmax - rTmin)/rt`.
	/// @return Число шагов; 0, если интервал пуст или число не помещается в `int`.
	int nGetTmax( void );
	/// Накопитель ошибок разбора секции `#PARM`; копируется в ошибку вызывающего.
	CTmcLibError cError;
	/// Имя единицы частоты из `FREQ_UNIT` (по умолчанию «GHz»).
	CString csFreqUnit;
	/// Множитель перевода частоты задания в Гц.
	double rFreqUnit;
	/// Имя единицы длины из `LONG_UNIT` (по умолчанию «mm»).
	CString csLongUnit;
	/// Множитель перевода длины задания в м.
	double rLongUnit;
	/// Имя единицы времени из `TIME_UNIT` (по умолчанию «s»).
	CString csTimeUnit;
	/// Множитель перевода времени задания в с.
	double rTimeUnit;
	/// Имя единицы угла из `ANGLE_UNIT` (по умолчанию «radian»).
	CString csAngleUnit;
	/// Множитель перевода угла задания в радианы (1 или pi/180).
	double rAngleUnit;
	/// Имя единицы индукции из `B_UNIT`; при чтении `N_UNIT` сюда же записывается имя единицы N.
	CString csBUnit;
	/// Множитель единицы индукции; при чтении `N_UNIT` сюда же записывается множитель N.
	double rBUnit;
	/// Имя единицы концентрации (всегда «m3»: `ReadNUnit` пишет в `csBUnit`).
	CString csNUnit;
	/// Множитель единицы концентрации (всегда 1).
	double rNUnit;
	/// Шаг сетки Delta, м (одинаков по X и Y).
	double rDelta;
	/// Рабочая частота `FREQ`, Гц.
	double rFreq;
	/// Высота структуры `HEIGHT`, м (на расчёт не влияет).
	double rHeight;
	/// Шаг по времени `dT = rDelta*sqrt(mu0*eps0/2) = rDelta/(c*sqrt(2))`, с; вычисляется при чтении `DELTA`.
	double rt;
	/// Левая граница области, м.
	double rXmin;
	/// Правая граница области после выравнивания по сетке (с резервным столбцом), м.
	double rXmax;
	/// Нижняя граница области, м.
	double rYmin;
	/// Верхняя граница области после выравнивания по сетке (с резервной строкой), м.
	double rYmax;
	/// Начало интервала расчёта, с.
	double rTmin;
	/// Конец интервала расчёта, с.
	double rTmax;
	/// Число узлов по X, вычисленное `IsCorrectly` (0 — ещё не вычислено).
	int nXGrid;			// B-2: nodes on X and Y, calculated once when the borders are read (0 - not yet)
	/// Число узлов по Y, вычисленное `IsCorrectly` (0 — ещё не вычислено).
	int nYGrid;
	/// Значение `X_MAX` как в задании, м (до выравнивания по сетке).
	double rXmaxUser;	// B-2: X_MAX, Y_MAX as written in the task (rXmax, rYmax are moved to the reserve column)
	/// Значение `Y_MAX` как в задании, м (до выравнивания по сетке).
	double rYmaxUser;
	/// @brief А-13, Б-15: какие параметры уже прочитаны в секции `#PARM` текущего шага (биты `TMC_PARAM_SEEN_*`).
	///
	/// Сбрасывается `BeginSection` перед каждой секцией. По нему `IsCorrectly` проверяет, что в шаге заданы
	/// все обязательные параметры (Б-15), а строки единиц — что единица стоит выше своих величин (А-13).
	unsigned nSeenInSection;	// A-13, B-15
	/// @brief А-13, Б-15: начало секции `#PARM` очередного шага — сброс `nSeenInSection`.
	void BeginSection( void ) { nSeenInSection = 0; }
	/// @brief А-13: ошибка, если величина, на которую действует единица, уже прочитана в этой секции.
	/// @param[in] csCh   строка единицы (для сообщения)
	/// @param[in] nMask  биты величин, на которые действует эта единица
	/// @param[in] pszIds названия этих величин (для сообщения)
	/// @return TRUE — порядок верный.
	BOOL CheckUnitBeforeValues( CString &csCh, unsigned nMask, const char *pszIds );
	/// @brief Копирует все параметры, включая ошибку и вычисленную сетку.
	/// @param[in] cParam источник
	/// @return `*this`.
	CTmcRTH_IndanParam& operator=( CTmcRTH_IndanParam& cParam );
	/// @brief Возвращает множитель единицы длины задания.
	/// @return Метров в одной единице `LONG_UNIT`.
	double rGetLongUnit( void );
	/// @brief Возвращает имя единицы длины задания.
	/// @return «m», «cm» или «mm».
	CString csGetLongUnit( void );
	/// @brief Проверяет параметры шага и выравнивает область по сетке.
	///
	/// Требует `Ymin < Ymax`, `Xmin < Xmax`, `Tmin < Tmax`, `Delta > 0`, `Freq > 0`
	/// и `Delta <= lambda/5` (`lambda = c/Freq`). Если сетка ещё не вычислена
	/// (`nXGrid`/`nYGrid` == 0), вычисляет её один раз:
	/// `n = floor((Xmax_задания - Xmin)/Delta + 1e-7) + 1`, `rXmax = Xmin + n*Delta`,
	/// `nXGrid = n + 1` (то же по Y) — правая и верхняя граница переносятся
	/// на резервный столбец/строку узлов.
	/// @param[in,out] cError1 ошибка вызывающего; при нарушении получает текст ошибки
	/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#grid-align
	void IsCorrectly( CTmcLibError &cError1 );
	/// @brief Разбирает одну строку секции `#PARM` и запоминает значение.
	///
	/// Ключевое слово определяется по началу строки; значения — выражения
	/// библиотеки expr (числа, переменные `#define`, функции), завершённые `;`.
	/// Значения сразу переводятся в СИ текущими единицами, поэтому единицы
	/// должны стоять в секции раньше величин.
	/// @param[in]     csCh    строка секции после препроцессора (без лишних пробелов)
	/// @param[in,out] cError1 ошибка вызывающего; при ошибке — текст «Unknown keyword…»,
	///                        «Missing separator ';'…» или ошибка выражения
	void Read( CString &csCh, CTmcLibError &cError1 );
	/// @brief Возвращает все параметры к значениям по умолчанию (GHz, mm, s, radian, Tl, m3; нули).
	void DeleteData( void );
	/// @brief Создаёт параметры со значениями по умолчанию (как `DeleteData`).
	CTmcRTH_IndanParam();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTH_IndanParam)
	//}}AFX_VIRTUAL

// Implementation
public:
	/// @brief Возвращает число узлов сетки по Y.
	/// @return `nYGrid`, если сетка уже вычислена `IsCorrectly`; иначе
	///         `(int)((rYmax - rYmin)/rDelta) + 1`; 0 при `rDelta` ~ 0.
	int CalcnY( void );
	/// @brief Возвращает число узлов сетки по X.
	/// @return `nXGrid`, если сетка уже вычислена `IsCorrectly`; иначе
	///         `(int)((rXmax - rXmin)/rDelta) + 1`; 0 при `rDelta` ~ 0.
	int CalcnX( void );
	/// @brief Уничтожает объект (вызывает `DeleteData`).
	virtual ~CTmcRTH_IndanParam();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcRTH_IndanParam)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
//	DECLARE_MESSAGE_MAP()
private:
	/// @brief Удаляет из строки пробелы, табуляции и переводы строк (на месте).
	/// @param[in,out] ch строка, завершённая нулём
	/// @return Всегда 0.
	int expr_del_Blanks2( char *ch );
	/// @brief Выделяет очередной параметр до `;` верхнего уровня (вне скобок `{}()[]`).
	/// @param[out]    csBuf текст параметра без `;`
	/// @param[in,out] szBuf строка с текущей позиции; временно изменяется и восстанавливается
	/// @return Длина параметра вместе с `;` (сдвиг позиции); -1 и ошибка в `cError`,
	///         если `;` нет.
	int Search_1Param( CString &csBuf, char *szBuf );
	/// @brief Разбирает `ANGLE_UNIT` и переключает единицу углов библиотеки выражений.
	/// @param[in] csCh строка секции
	void ReadAngleUnit( CString &csCh );
	/// @brief Разбирает `TIME_UNIT`.
	/// @param[in] csCh строка секции
	void ReadTimeUnit( CString &csCh );
	/// @brief Разбирает `LONG_UNIT`.
	/// @param[in] csCh строка секции
	void ReadLongUnit( CString &csCh );
	/// @brief Разбирает `FREQ_UNIT`.
	/// @param[in] csCh строка секции
	void ReadFreqUnit( CString &csCh );
	/// @brief Разбирает `TIME <Tmin>; <Tmax>;` в `rTmin`, `rTmax` (с).
	/// @param[in] csCh строка секции
	void ReadTime( CString &csCh );
	/// @brief Разбирает `FREQ` в `rFreq` (Гц).
	/// @param[in] csCh строка секции
	void ReadFreq( CString &csCh );
	/// @brief Разбирает `DELTA` в `rDelta` (м), вычисляет шаг по времени `rt`
	///        и сбрасывает вычисленную сетку.
	/// @param[in] csCh строка секции
	void ReadDelta( CString &csCh );
	/// @brief Разбирает `Y_MAX` в `rYmax`, `rYmaxUser` (м) и сбрасывает `nYGrid`.
	/// @param[in] csCh строка секции
	void ReadYmax( CString &csCh );
	/// @brief Разбирает `Y_MIN` в `rYmin` (м) и сбрасывает `nYGrid`.
	/// @param[in] csCh строка секции
	void ReadYmin( CString &csCh );
	/// @brief Разбирает `X_MAX` в `rXmax`, `rXmaxUser` (м) и сбрасывает `nXGrid`.
	/// @param[in] csCh строка секции
	void ReadXmax( CString &csCh );
	/// @brief Разбирает `X_MIN` в `rXmin` (м) и сбрасывает `nXGrid`.
	/// @param[in] csCh строка секции
	void ReadXmin( CString &csCh );
	/// @brief Разбирает `HEIGHT` в `rHeight` (м).
	/// @param[in] csCh строка секции
	void ReadHeight( CString &csCh );
	/// @brief Разбирает `DELTA_T_MECHANICAL` в `rDeltaTMechanical` (с) и вызывает `SetDeltaT`.
	/// @param[in] csCh строка секции
	void ReadDeltaTMechanical( CString &csCh );
	/// @brief Разбирает `B_UNIT` в `csBUnit`, `rBUnit`.
	/// @param[in] csCh строка секции
	void ReadBUnit( CString &csCh );
	/// @brief Разбирает `N_UNIT`.
	/// @param[in] csCh строка секции
	/// @warning Записывает имя и множитель в `csBUnit`/`rBUnit`, а не в `csNUnit`/`rNUnit`:
	///          `N_UNIT` после `B_UNIT` затирает единицу индукции. Обе величины ядрами
	///          не применяются, поэтому на расчёт это не влияет.
	void ReadNUnit( CString &csCh );
public:
	// CalcnNodeGlobal return NodeGlobal or -1, if point x,y isn't in region
	/// Механический шаг по времени `DELTA_T_MECHANICAL`, с; `FLT_MAX` — не задан.
	double rDeltaTMechanical;
};


#endif
/////////////////////////////////////////////////////////////////////////////
