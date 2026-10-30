/// @file TmcSMatrix.h
/// @brief Столбец S-матрицы по сигналам портов из файла `.t` на одной частоте (класс `CTmcSMatrix`).
///
/// Общий заголовок. Реализация `viewers/tmcrtout/tmcsmatrix.cpp` собирается во вьювер TMCROS
/// и в библиотеки ядер `planarrt_h_ttos`, `planarrt_x_ttos`; у вьювера TMC_DN своя копия
/// реализации (`viewers/dianapgr/tmcsmatrix.cpp`). Снаружи класс используется через `CTmcTtoS`.
/// @see https://www.tamic.ru/docs/code/tmcros.html#ttos
#if !defined(AFX_TMCSMATRIX_H__F6739482_DF94_11D3_A946_508A4DC10000__INCLUDED_)
#define AFX_TMCSMATRIX_H__F6739482_DF94_11D3_A946_508A4DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcSMatrix.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcSMatrix window

#include <Typedef.h>

/// @brief Восстанавливает комплексные амплитуды гармонических сигналов портов и строит столбец S-матрицы.
///
/// Порядок работы: `ReadTFile` (отсчёты всех портов в окне времени и частота из заголовка
/// `.t`) -> `MakeSmatrix` (комплексная амплитуда уходящей волны каждого порта, выбор
/// возбуждённого порта, деление на амплитуду его падающей волны, проверка на NaN/Inf) ->
/// `Save` (дописывание столбца в `.s`). Каждый шаг при уже возникшей ошибке ничего не делает,
/// поэтому шаги вызываются подряд, а ошибка проверяется в конце (`IsError`).
/// Амплитуда и фаза восстанавливаются по каждой паре соседних отсчётов в предположении
/// установившегося синуса `A*sin(2*pi*f*t + phi)` и усредняются по всем парам окна.
/// Объект владеет массивами отсчётов и столбца; `DeleteData` и деструктор их освобождают.
/// @note Потоки: объект используется одним потоком.
/// @see https://www.tamic.ru/docs/code/tmcros.html#ttos-harmonic
/// @see https://www.tamic.ru/docs/theory/smatrix.html#s-from-signals
class CTmcSMatrix
{
// Construction
public:
	/// @brief Создаёт пустой объект: массивов нет, ошибки нет, номер возбуждённого входа -1.
	CTmcSMatrix();
	/// @brief Освобождает массивы отсчётов и столбца, сбрасывает имена, окно, частоту и признак ошибки.
	void DeleteData( void );

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcSMatrix)
	//}}AFX_VIRTUAL

// Implementation
public:
	/// @brief Вычисляет столбец S-матрицы по прочитанным отсчётам.
	///
	/// 1. Для каждого входа i — комплексная амплитуда уходящей волны `Uo_i` (`TtoSOneInput`).
	/// 2. Возбуждённый вход n — единственный вход с ненулевой падающей волной
	///    (`SetnLineInSMatrix`); ни одного или больше одного — ошибка.
	/// 3. Тем же способом — комплексная амплитуда падающей волны `Up` входа n; если её нет в
	///    окне — ошибка. При `|Up - 1| >= 1e-3` столбец делится на неё: `S_i = Uo_i / Up`;
	///    при `|Up - 1| < 1e-3` деления нет (разница считается шумом записи `.t`).
	/// 4. Элемент, не являющийся числом (NaN, Inf), — ошибка.
	/// @pre Успешный `ReadTFile`.
	/// @see https://www.tamic.ru/docs/code/tmcros.html#ttos-normalization
	/// @see https://www.tamic.ru/docs/theory/smatrix.html#normalization
	void MakeSmatrix( void );
	/// @brief Дописывает столбец в файл `.s` под заданной частотой.
	///
	/// Столбец с номером возбуждённого входа записывает `save_S_matrix_element`: нет файла —
	/// создаётся, частота уже есть — столбец заменяется, иначе точка добавляется по порядку частот.
	/// @param[in] csSFileName1 путь к файлу `.s`
	/// @param[in] dFreq1       частота точки в `.s`, Гц; заменяет частоту, прочитанную из `.t`
	/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#save-column
	void Save(CString csSFileName1, double dFreq1);
	/// @brief Дописывает столбец в файл `.s` под частотой из заголовка `.t` (частотой расчёта).
	/// @param[in] csSFileName1 путь к файлу `.s`
	void Save( CString csSFileName1 );
	/// @brief Возвращает текст последней ошибки.
	/// @return Ссылка на строку объекта; `No error` — ошибки нет.
	CString & GetErrorMessage( void );
	/// @brief Б-3.3: возвращает предупреждение последнего `MakeSmatrix` (пустая строка — предупреждений нет).
	///
	/// Предупреждение выдаётся, если в окно попали пары отсчётов, у которых оба значения равны 0
	/// (волна до входа ещё не дошла): они входят в среднее как ноль и занижают |S|. Усреднение при этом
	/// не меняется — окно выбирает пользователь (ответ автора 29.10.2026).
	/// @return Ссылка на строку объекта.
	CString & GetWarningMessage( void ) { return csWarning; }	// B-3.3
	/// @brief Сбрасывает объект и читает из `.t` отсчёты всех входов, попавшие в окно времени.
	///
	/// Число входов — по первой строке данных: `(число чисел - 2)/3` (номер такта, время, затем
	/// у каждого входа «номер блока, Up, Uo»). Отсчётов в окне должно быть больше 5. Частота
	/// берётся из последней строки `#TMC_GROTS_Freq` файла.
	/// @param[in] csTFile путь к файлу `.t`
	/// @param[in] dTmin1  начало окна, с
	/// @param[in] dTmax1  конец окна, с; не больше `dTmin1` — ошибка `Tmax <= Tmin`
	/// @see https://www.tamic.ru/docs/code/kernel-h.html#t-file
	void ReadTFile( CString csTFile, double dTmin1, double dTmax1 );
	/// @brief Возвращает признак ошибки.
	/// @return TRUE — была ошибка (текст — `GetErrorMessage`); FALSE — нет.
	BOOL IsError( void );
	/// @brief Освобождает массивы (`DeleteData`).
	virtual ~CTmcSMatrix();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcSMatrix)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
private:
	/// @brief Проверяет, возбуждён ли вход: есть ли в окне хотя бы один отсчёт падающей волны `|Up| > 10*DBL_MIN`.
	/// @param[in] n номер входа, 0..`nInput`-1
	/// @return TRUE — вход возбуждён; FALSE — нет или ошибка (признак ошибки взводится).
	BOOL IsInputExcite( int n );
	/// @brief Находит возбуждённый вход и записывает его номер в `nLineInSMatrix`.
	///
	/// Ошибки: `There are no exited inputs` — ни одного, `Not one is exited of inputs` — больше одного.
	void SetnLineInSMatrix( void );
	/// @brief Восстанавливает амплитуду и фазу синуса частоты `dFreq` по двум отсчётам и прибавляет `A*exp(j*phi)` к сумме.
	///
	/// Модель сигнала `u(t) = A*sin(omega*t + phi)`, `omega = 2*pi*dFreq`. Из двух отсчётов
	/// `tan(phi) = -(u2*sin(omega*t1) - u1*sin(omega*t2)) / (u2*cos(omega*t1) - u1*cos(omega*t2))`,
	/// затем `A = u/sin(omega*t + phi)` по отсчёту с большим `|sin|`; при `A < 0` берутся
	/// `-A` и `phi - pi`.
	/// @param[in,out] pscSFor2Point сумма комплексных амплитуд по парам; к ней прибавляется вклад пары
	/// @param[in]     dTsignal1     первый отсчёт сигнала (условные единицы ядра)
	/// @param[in]     dTime1        время первого отсчёта, с
	/// @param[in]     dTsignal2     второй отсчёт
	/// @param[in]     dTime2        время второго отсчёта, с
	/// @return TRUE — пара учтена (в том числе оба отсчёта нулевые: вклад 0); FALSE — пара
	///         вырождена (система не решается), вклад не прибавлен.
	/// @see https://www.tamic.ru/docs/code/tmcros.html#ttos-harmonic
	BOOL TtoSOneInputOnePoint( _complex *pscSFor2Point, double dTsignal1, double dTime1, double dTsignal2, double dTime2);
	/// @brief Комплексная амплитуда уходящей волны `Uo` входа `n`: среднее `TtoSOneInputOnePoint` по всем парам соседних отсчётов окна.
	/// @param[in] n номер входа, 0..`nInput`-1; результат — в `pcSmatrix[n]`
	/// @note Нет ни одной учтённой пары — ошибка `TtoSOne: input n`.
	void TtoSOneInput( int n );
	/// @brief Вызывает `TtoSOneInput` для всех входов.
	void TtoS( void );
	/// Номер возбуждённого входа, 0..`nInput`-1 (столбец S-матрицы); -1 — не определён.
	int nLineInSMatrix;
	/// @brief Выделяет столбец `pcSmatrix` из `nInput` элементов и заполняет его значением `FLT_MAX` (не записано).
	void AllocSArray( void );
	/// @brief Пропускает число (до первого пробела, табуляции или перевода строки).
	/// @param[in] ch строка
	/// @return Число пропущенных символов; 0 — разделителя до конца строки нет.
	int skip_number(char *ch);
	/// @brief Пропускает пробелы, табуляции и переводы строки.
	/// @param[in] ch строка
	/// @return Индекс первого другого символа; 0 — строка до конца пробельная.
	int skip_blank(char *ch);
	/// @brief Читает одно число из строки и записывает его в `**pdTsign` со сдвигом указателя.
	/// @param[in,out] pdTsign указатель на текущую ячейку массива отсчётов; увеличивается на 1
	/// @param[in]     ch      строка, начиная с числа (возможно с начальными пробелами)
	/// @return Число прочитанных символов; 0 — ошибка (признак взведён).
	int readTsignalOne(double **pdTsign, char *ch);
	/// @brief Разбирает строку данных `.t`: пропускает номер такта и время, у каждого входа пропускает номер блока и читает `Up`, `Uo`.
	/// @param[in,out] pdTsign указатель на текущую ячейку `pdTsignal`; сдвигается на `2*nInput`
	/// @param[in]     ch      строка данных
	void readTsignal( double **pdTsign, char *ch );
	/// Путь к файлу `.s` результата.
	CString csSFileName;
	/// @brief Читает время и отсчёты первых `nPoint` строк данных, попавших в окно; строк меньше — ошибка чтения.
	void ReadTArray( void );
	/// @brief Выделяет `pdTsignal` (`nPoint*nInput*2`) и `pdTime` (`nPoint`), заполняет нулями.
	void AllocTArray( void );
	/// @brief Считает числа (слова, разделённые пробелами и табуляциями) в строке.
	/// @param[in] ch строка
	/// @return Число слов до конца строки или перевода строки.
	int gets_nNumbersInLine( char *ch );
	/// @brief Читает время из строки данных (второе число после номера такта).
	/// @param[in] ch строка данных
	/// @return Время, с; -1 — ошибка чтения (признак взведён).
	double gets_f1(char *ch);
	/// @brief Определяет число входов по первой строке данных: `(число чисел - 2)/3`.
	/// @return Число входов; 0 — ошибка (признак взведён).
	int CalcNInput( void );
	/// @brief Считает строки данных с временем в окне `dTmin..dTmax` и читает частоту из строки `#TMC_GROTS_Freq`.
	/// @return Число отсчётов в окне; 0 — ошибка (файл не открылся или отсчётов не больше 5).
	int CalcNPoint( void );
	/// @brief Взводит признак ошибки и запоминает её текст.
	/// @param[in] ch текст ошибки
	void PutErrorMessage( const char *ch );
	/// @brief Читает `.t` по запомненным имени и окну: проверка окна, `CalcNPoint`, `CalcNInput`, `AllocTArray`, `ReadTArray`.
	void ReadTFile( void );
	/// @brief Записывает столбец `nLineInSMatrix` в `csSFileName` с частотой `dFreq` (`save_S_matrix_element`).
	void Save( void );
	/// Столбец S-матрицы: `nInput` комплексных элементов S(i, n), n — возбуждённый вход; владеет объект.
	_complex * pcSmatrix;
	/// Отсчёты сигналов: элемент `[(k*nInput + i)*2]` — падающая волна `Up` входа i в отсчёте k,
	/// `[(k*nInput + i)*2 + 1]` — уходящая волна `Uo`; условные единицы ядра. Владеет объект.
	double * pdTsignal;
	/// Время отсчётов, с; `nPoint` элементов. Владеет объект.
	double * pdTime;
	/// Текст последней ошибки; `No error` — ошибки нет.
	CString csError;
	/// Б-3.3: предупреждение о нулевых парах отсчётов в окне; пустая строка — нет.
	CString csWarning;	// B-3.3
	/// Путь к файлу сигналов `.t`.
	CString csTFileName;
	/// Начало окна времени, с.
	double dTmin;
	/// Конец окна времени, с.
	double dTmax;
	/// Частота, Гц: после `ReadTFile` — частота расчёта из `.t` (по ней восстанавливаются
	/// амплитуды), после `Save(имя, частота)` — частота записи.
	double dFreq;
	/// TRUE — была ошибка.
	BOOL bIsError;
	/// Число входов (портов) в файле `.t`.
	int nInput;
	/// Число отсчётов в окне времени.
	int nPoint;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCSMATRIX_H__F6739482_DF94_11D3_A946_508A4DC10000__INCLUDED_)
