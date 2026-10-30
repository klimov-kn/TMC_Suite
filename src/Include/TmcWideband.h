/// @file TmcWideband.h
/// @brief Многочастотная обработка файла `.t`: один расчёт с широкополосной падающей волной -> S-матрица на всех частотах (класс `CTmcWideband`).
///
/// Общий заголовок. Реализация `viewers/tmcrtout/tmcwideband.cpp` собирается во вьювер TMCROS
/// (окно `CDialogTtoS`) и в библиотеки ядер `planarrt_h_ttos`, `planarrt_x_ttos`
/// (`CTmcRTH_IndanOutput::PutSmatrix`), поэтому оба пути дают одинаковый `.s`. Класс выбирается,
/// когда `IsWidebandFile` находит в заголовке `.t` строку `#TMC_GROTS_Wideband 1`; иначе
/// работает одночастотный `CTmcTtoS`.
/// @see https://www.tamic.ru/docs/code/tmcros.html#wideband
/// @see https://www.tamic.ru/docs/theory/smatrix.html#multifreq
#if !defined(AFX_TMCWIDEBAND_H__INCLUDED_)
#define AFX_TMCWIDEBAND_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcWideband.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CTmcWideband
//
//  Wideband post-processing of a .t signal file: one kernel run that was
//  excited by a multi-frequency incident wave gives the whole S-matrix
//  sweep at once.
//
//  The class is self-contained on purpose: it does not touch CTmcSMatrix,
//  whose header is shared with TMC_DN.
//
//  What it does:
//    1. reads the header written by the kernel:
//         #TMC_GROTS_Wideband 1
//         #TMC_GROTS_Port <block> <delta,m> <aperture,m> <freq,Hz> <from file>
//       no such header (an old file) -- nothing happens, the caller keeps
//       the usual single-frequency path;
//    2. tells a pulse from a continuous multi-tone by the level of the
//       incident wave at the end of the run, and picks the matching way of
//       resolving frequencies (this must not be mixed up: using the pulse
//       way on a continuous signal drags the transient into the integral
//       and costs about 0.5 dB);
//    3. finds the frequencies that actually carry energy;
//    4. removes the port mismatch, which is what makes the result equal to
//       a frequency sweep instead of merely close to it;
//    5. appends every frequency to the .s file.
//
/////////////////////////////////////////////////////////////////////////////

#include <Typedef.h>

/// Наибольшее число частот одного многочастотного расчёта и наибольшее число портов в
/// заголовке `.t`: `FREQ_MAX_NUM - 1` (предел числа частот файла `.s`).
#define TMC_WIDEBAND_MAXFREQ	99		// FREQ_MAX_NUM - 1, see TMCLIMIT.H

/// @brief Строит S-матрицу на всех частотах широкополосного сигнала по одному файлу `.t`.
///
/// `Process` выполняет шаги по порядку: `ReadHeader` (признак, частоты сигнала, данные
/// портов) -> `ReadSignals` (время, `Up`, `Uo` всех портов) -> `DetectSignalKind` (импульс или
/// непрерывный многотон) -> `ApplyWindow` (окно `Tmin..Tmax`, предупреждения) ->
/// `FindFrequencies` (частоты, несущие энергию) -> `MakeAndSave` (спектры, деление на падающую
/// волну, поправка на рассогласование порта, запись каждой частоты в `.s`). Каждый шаг при
/// уже возникшей ошибке ничего не делает. Ошибка на отдельной частоте не останавливает
/// расчёт: в `.s` на ней пишутся нули, а итоговая ошибка перечисляет такие частоты.
/// Объект создаётся на стеке вызывающим кодом и владеет массивами отсчётов.
/// @note Потоки: объект используется одним потоком (главный поток TMCROS или расчётный
///       поток ядра).
/// @see https://www.tamic.ru/docs/code/tmcros.html#wideband
/// @see https://www.tamic.ru/docs/theory/smatrix.html#multifreq
class CTmcWideband
{
public:
	/// @brief Создаёт пустой объект (массивов нет, ошибки нет).
	CTmcWideband();
	/// @brief Освобождает массивы отсчётов.
	virtual ~CTmcWideband();

	//  TRUE if the kernel marked this .t as wideband
	/// @brief Проверяет, помечен ли файл `.t` ядром как многочастотный.
	///
	/// Просматривает заголовок до первой строки данных и ищет строку `#TMC_GROTS_Wideband n`.
	/// Ядро пишет `1`, если хотя бы у одного порта падающая волна задана файлом `.sig`.
	/// @param[in] csTFile путь к файлу `.t`
	/// @return TRUE — строка есть и `n != 0`; FALSE — нет строки, `n == 0` или файл не открылся.
	static BOOL IsWidebandFile( CString csTFile );

	//  the whole job: .t in, .s out
	/// @brief Выполняет всю обработку: `.t` -> S-матрица на всех найденных частотах -> дописывание в `.s`.
	///
	/// Записывается один столбец (номер — первый порт с ненулевой падающей волной) на каждой
	/// частоте. Ошибка — `IsError`/`GetErrorMessage`, предупреждение — `GetWarningMessage`.
	/// @param[in] csTFile путь к файлу `.t` многочастотного расчёта
	/// @param[in] csSFile путь к файлу `.s` (`save_S_matrix_element` для каждой частоты)
	/// @param[in] dTmin1  начало окна времени, с
	/// @param[in] dTmax1  конец окна времени, с; `dTmax1 <= dTmin1` — окно не применяется, берутся все отсчёты
	/// @see https://www.tamic.ru/docs/code/tmcros.html#wideband
	void Process( CString csTFile, CString csSFile, double dTmin1, double dTmax1 );

	/// @brief Возвращает признак ошибки.
	/// @return TRUE — ошибка (текст — `GetErrorMessage`); FALSE — успех.
	BOOL IsError( void );
	/// @brief Возвращает текст ошибки.
	/// @return Ссылка на строку объекта; `No error` — ошибки нет.
	CString & GetErrorMessage( void );
	/// @brief Возвращает предупреждение об окне времени (незатухший отклик импульса, окно режет импульс).
	/// @return Ссылка на строку объекта; пустая строка — предупреждения нет. S-матрица при этом записана.
	CString & GetWarningMessage( void );	// B-22: "" - no warning
	/// @brief Возвращает число найденных частот.
	/// @return Число частот, 0..`TMC_WIDEBAND_MAXFREQ`.
	int GetFreqNumber( void ) { return nFreq; }
	/// @brief Возвращает вид сигнала, определённый `DetectSignalKind`.
	/// @return TRUE — непрерывный многотон; FALSE — импульс.
	BOOL IsContinuous( void ) { return bContinuous; }

	//  service getters, used by the console check wbtest
	/// @brief Возвращает число отсчётов после применения окна (служебная функция, в Qt6-версии не вызывается).
	/// @return Число отсчётов.
	int GetPointNumber( void ) { return nPoint; }
	/// @brief Возвращает число портов в файле (служебная функция, в Qt6-версии не вызывается).
	/// @return Число портов.
	int GetInputNumber( void ) { return nInput; }
	/// @brief Возвращает частоту номер `k` (служебная функция, в Qt6-версии не вызывается).
	/// @param[in] k номер частоты, 0..`GetFreqNumber()`-1
	/// @return Частота, Гц; 0 — номер вне диапазона.
	double GetFreqValue( int k ) { return ( ( k >= 0 )&&( k < nFreq ) ) ? dFreq[k] : 0.0; }

private:
	/// @brief Освобождает массивы и сбрасывает все поля в начальное состояние.
	void DeleteData( void );
	/// @brief Взводит признак ошибки и запоминает её текст.
	/// @param[in] ch текст ошибки
	void PutErrorMessage( const char *ch );

	/// @brief Читает заголовок `.t`: признак `#TMC_GROTS_Wideband`, список частот `#TMC_GROTS_SigFreq` (Гц),
	///        строки портов `#TMC_GROTS_Port` (блок, шаг сетки, апертура, частота согласования, признак файла).
	///
	/// Нет признака многочастотности — ошибка `file is not marked as wideband`.
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#t-file
	void ReadHeader( void );
	/// @brief Читает строки данных `.t` в `pdTime`, `pdUp`, `pdUo` (два прохода: подсчёт, затем значения).
	///
	/// Число портов — по первой строке с `2 + 3*n` числами. Меньше 8 отсчётов — ошибка.
	void ReadSignals( void );
	/// @brief Определяет вид сигнала по падающим волнам всех портов: непрерывный многотон, если в последней
	///        четверти записи `|Up|` больше 1% своего максимума, иначе импульс.
	/// @note Вызывается до `ApplyWindow`, то есть по всей записи, а не по окну.
	void DetectSignalKind( void );
	/// @brief Оставляет отсчёты окна `dTmin..dTmax` (если `dTmax > dTmin`) и проверяет окно для импульса.
	///
	/// В окне меньше 8 отсчётов — ошибка. Для импульса: если в последних 5% окна сигнал (больший
	/// из `|Up|`, `|Uo|` всех портов) больше 1% пика — предупреждение «отклик не затух»; если окно
	/// отрезало начало записи и в первых 5% окна сигнал больше 1% пика — предупреждение «окно
	/// начинается во время импульса».
	void ApplyWindow( void );		// B-22: the window Tmin...Tmax, the tail of a pulse
	/// @brief Находит частоты расчёта `dFreq[0..nFreq-1]`.
	///
	/// Частоты из заголовка (`#TMC_GROTS_SigFreq`) берутся как есть; для многотона окно
	/// подгоняется под целое число периодов шага частот. Иначе по спектру падающей волны
	/// возбуждённого порта ищутся пики (не ниже -20 дБ от наибольшего) с уточнением по
	/// параболе; если пиков не больше двух — 21 частота равномерно по полосе выше -20 дБ.
	/// Ни одной частоты — ошибка.
	/// @see https://www.tamic.ru/docs/code/tmcros.html#wideband-freqs
	void FindFrequencies( void );
	/// @brief Для каждой частоты строит столбец `S_i = Uo_i(f)/Up(f)`, снимает рассогласование порта и дописывает столбец в `.s`.
	///
	/// Спектры: для многотона — совместная подгонка всех тонов (`FitTones`) на окне
	/// `dFitT1..t_конца`, для импульса — интеграл по всей записи (`Integrate`). Частота, на
	/// которой падающая волна слабее 1e-6 самой сильной (по модулю), вычисление не удалось или
	/// частота ниже отсечки порта, получает нулевой столбец; итоговая ошибка перечисляет до
	/// 10 таких частот.
	/// @see https://www.tamic.ru/docs/code/tmcros.html#wideband-spectrum
	void MakeAndSave( void );

	//  projection of x(t) on exp(-j*2*pi*f*t) over [t1, t2]
	/// @brief Комплексная амплитуда составляющей частоты `dF` на отрезке времени (проекция на `exp(-j*2*pi*f*t)`).
	///
	/// `X = 2/T * sum x(t_i)*exp(-j*omega*t_i)*(t_i - t_(i-1))` по отсчётам `dT1 <= t_i <= dT2`,
	/// `T` — сумма шагов. Для `x = A*cos(omega*t + phi)` на целом числе периодов `X = A*exp(j*phi)`.
	/// @param[in]  pdX  отсчёты сигнала, `nPoint` элементов (шаг 1)
	/// @param[in]  dF   частота, Гц
	/// @param[in]  dT1  начало отрезка, с
	/// @param[in]  dT2  конец отрезка, с
	/// @param[out] pdRe действительная часть
	/// @param[out] pdIm мнимая часть; при пустом отрезке обе части 0
	void Project( double *pdX, double dF, double dT1, double dT2,
		double *pdRe, double *pdIm );
	//  joint least-squares fit of all tones (used for a continuous signal)
	/// @brief Совместная подгонка всех тонов `dFreq[]` методом наименьших квадратов на отрезке времени.
	///
	/// Модель `x(t) = sum_k ( a_k*cos(omega_k*t) - b_k*sin(omega_k*t) )`; результат `c_k = a_k + j*b_k`,
	/// то есть `x(t) = Re sum_k c_k*exp(j*omega_k*t)`. Нормальные уравнения размера `2*nFreq`
	/// решает `SolveLinear`.
	/// @param[in]  pdX   отсчёты сигнала, `nPoint` элементов
	/// @param[in]  dT1   начало отрезка, с
	/// @param[in]  dT2   конец отрезка, с
	/// @param[out] pcOut комплексные амплитуды тонов, `nFreq` элементов
	/// @return TRUE — успех; FALSE — нет памяти или система вырождена (тоны ближе `1/T` или повторяются).
	BOOL FitTones( double *pdX, double dT1, double dT2, _complex *pcOut );
	//  integral over the whole run (used for a pulse)
	/// @brief Спектр сигнала на частоте `dF` по всей записи: `X = sum x(t_i)*exp(-j*omega*t_i)*(t_i - t_(i-1))` (условные единицы ядра * с).
	/// @param[in]  pdX  отсчёты сигнала, `nPoint` элементов
	/// @param[in]  dF   частота, Гц
	/// @param[out] pdRe действительная часть
	/// @param[out] pdIm мнимая часть
	void Integrate( double *pdX, double dF, double *pdRe, double *pdIm );

	/// @brief Коэффициент отражения порта на частоте `dF`, вызванный тем, что ядро согласует порт только на частоте задания.
	///
	/// `Gamma = (Y(f) - Y(f0))/(Y(f) + Y(f0))`, где `Y(f) = sin(acos(2*cos(a) - cos(pi*Delta/b)))/sin(a)`,
	/// `a = sqrt(2)*pi*Delta*f/c`, `Delta` — шаг сетки, `b` — апертура, `f0` — частота согласования
	/// (данные строки `#TMC_GROTS_Port`). Частота ниже отсечки порта — ошибка.
	/// @param[in] dF    частота, Гц
	/// @param[in] nPort номер порта в порядке строк `#TMC_GROTS_Port`, 0..`nPort`-1
	/// @return Коэффициент отражения (действительный); 0 — номер вне диапазона, данных порта нет или ошибка.
	/// @see https://www.tamic.ru/docs/code/tmcros.html#mismatch
	/// @note Б-22 (1), ответ автора 29.10.2026: Y(f) — без W порта и без магнитных стенок, которые
	///       учитывает нагрузка порта в ядре; ограничение оставлено и описано (свойство многочастотного режима).
	/// @see https://www.tamic.ru/docs/theory/ports.html#mismatch
	double PortGamma( double dF, int nPort );
	/// @brief Снимает рассогласование порта со столбца S на частоте `dF`.
	///
	/// Двухпортовая структура считается симметричной (`S22 = S11`, `S12 = S21`), и оба порта
	/// получают `Gamma` возбуждённого: чётная и нечётная волны `G = S11 +- S21` пересчитываются
	/// `G' = (G + Gamma)/(1 + Gamma*G)`, затем `S11 = (Ge' + Go')/2`, `S21 = (Ge' - Go')/2`. При
	/// другом числе портов исправляется только `S_nn` возбуждённого порта той же формулой.
	/// @param[in,out] pcCol    столбец S, `nInput` элементов
	/// @param[in]     nExcited номер возбуждённого порта, 0..`nInput`-1
	/// @param[in]     dF       частота, Гц
	/// @note Б-22 (1), ответ автора 29.10.2026: поправка точна только для одинаковых портов и симметричного
	///       устройства (S11 = S22); у портов разной ширины или несимметричной структуры она приближённая.
	///       Оставлено как есть; предупреждение о таких заданиях — в TPL Designer; в будущем — многоканальный вход.
	/// @see https://www.tamic.ru/docs/code/tmcros.html#mismatch
	void FixMismatch( _complex *pcCol, int nExcited, double dF );

	/// @brief Решает систему `A*x = B` методом Гаусса с выбором главного элемента по столбцу.
	/// @param[in,out] pdA матрица `n x n` по строкам; портится
	/// @param[in,out] pdB правая часть; на выходе — решение
	/// @param[in]     n   размер системы
	/// @return TRUE — решено; FALSE — система вырождена (главный элемент меньше 1e-300 или меньше 1e-6 наибольшего диагонального).
	static BOOL SolveLinear( double *pdA, double *pdB, int n );

	/// Путь к файлу `.t`.
	CString csTFileName;
	/// Путь к файлу `.s`.
	CString csSFileName;
	/// Текст ошибки; `No error` — ошибки нет.
	CString csError;
	/// TRUE — ошибка.
	BOOL bIsError;
	/// Предупреждение об окне времени; пустая строка — нет.
	CString csWarning;		// B-22

	/// TRUE — в заголовке есть `#TMC_GROTS_Wideband` с ненулевым значением.
	BOOL bWideband;
	/// TRUE — непрерывный многотон; FALSE — импульс.
	BOOL bContinuous;		// TRUE: multi-tone that never stops; FALSE: pulse

	/// Число отсчётов (после `ApplyWindow` — в окне).
	int nPoint;
	/// Число портов в строках данных.
	int nInput;
	/// Время отсчётов, с; `nPoint` элементов.
	double *pdTime;
	/// Падающая волна: элемент `[k*nInput + i]` — отсчёт k порта i (условные единицы ядра).
	double *pdUp;			// nPoint x nInput
	/// Уходящая волна, та же раскладка, что у `pdUp`.
	double *pdUo;			// nPoint x nInput
	/// Начало окна времени, с.
	double dTmin;
	/// Конец окна времени, с; `dTmax <= dTmin` — окно не задано.
	double dTmax;
	/// Начало отрезка подгонки тонов многотона, с (конец — последний отсчёт).
	double dFitT1;			// start of the window the frequency search settled on

	/// Число частот из строки `#TMC_GROTS_SigFreq`; 0 — частоты ищутся по спектру.
	int nFreqNamed;			// how many frequencies the signal file named
	/// Число частот расчёта.
	int nFreq;
	/// Частоты расчёта, Гц.
	double dFreq[TMC_WIDEBAND_MAXFREQ];

	//  port data from the header
	/// Число строк `#TMC_GROTS_Port` (портов с данными для поправки).
	int nPort;
	/// Номер блока порта (из задания).
	int nPortBlock[TMC_WIDEBAND_MAXFREQ];
	/// Шаг сетки, м.
	double dPortDelta[TMC_WIDEBAND_MAXFREQ];
	/// Ширина апертуры порта, м.
	double dPortAperture[TMC_WIDEBAND_MAXFREQ];
	/// Частота, на которой ядро согласовало порт (FREQ шага), Гц.
	double dPortFreq[TMC_WIDEBAND_MAXFREQ];
};

/////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_TMCWIDEBAND_H__INCLUDED_)
