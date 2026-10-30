// c2DArray.h: interface for the c2DArray class.
//
//////////////////////////////////////////////////////////////////////

/// @file c2DArray.h
/// @brief Модель плоской антенной решётки `c2DArray`: чтение файла излучателей `.DAT`, расчёт ДН в дальней зоне, КНД и КИП.
///
/// Часть модуля «Вьювер TMC_DN» (расчётная часть, каталог `viewers/dianapr`). Решётка —
/// набор излучателей `cIzl` на плоскости; ДН — когерентная сумма их полей в дальней зоне.
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#array-model
#if !defined(AFX_C2DARRAY_H__15B05DDC_FBD2_4046_87E5_F234F2BA20BF__INCLUDED_)
#define AFX_C2DARRAY_H__15B05DDC_FBD2_4046_87E5_F234F2BA20BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "cIzl.h"
#include "windows.h"

/// Размер буферов имён файлов, текстов ошибок и выражения ДН элемента, символов.
#define TMC_C2DARRAY_MAX_STRING 1000

/// @brief Плоская (2D) решётка излучателей: ДН в дальней зоне, нормировка, КНД, КИП, девиации.
///
/// Один объект — одна решётка из файла `.DAT` (обычно файл эквивалентных излучателей на
/// контуре в ближней зоне, который пишет счётное ядро). Хранит излучатели `pcIzl`, сетку
/// углов графика `pdAngle` и рассчитанную ДН: нормированный модуль `pdRealDN` и фазу
/// `pdImageDN`. Расчёт: `CalculateDiagrNapr(вход, выход)` = `ReadData` + сумма полей на
/// сетке углов + `NormirovkaDiagrNapr` (КНД, КИП, нормировка) + `WriteData`.
///
/// Владение: документ TMC_DN держит массив объектов, по одному на график
/// (`CTMCGROUTDoc::pcDirectionalPattern`, создаётся в `DocFileNewData`, удаляется в
/// `DocFileDelData`); окна девиаций создают временные объекты на стеке.
/// Исключений нет: ошибка ставит флаг `IsError()` и текст `GetError()`, после чего почти
/// все методы ничего не делают до `DeleteData`/`ReadData`.
/// Решётка из больше чем `nMaxTemporary` (400) излучателей считается «большой»: другая сетка
/// углов, вместо КИП — излучённая мощность, ДН хранится во временном файле-кэше.
/// @note Потоки: объект не защищён; в TMC_DN расчёт идёт в потоке `ReadDocFileW_Thread1`,
/// а окно прогресса из главного потока только читает `iN` и `GetnIzl()`.
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#array-model
/// @see https://www.tamic.ru/docs/theory/radiation.html#arrays
class c2DArray  
{
public:
	/// TRUE (по умолчанию) — учитывать секторы излучения FiMin..FiMax излучателей из файла;
	/// FALSE — секторы заменяются на -180..180° (выражение ДН излучателя сохраняется).
	/// Задаёт документ по номеру моды 1 строки графика (1 — TRUE, 0 — FALSE); значение входит
	/// в имена временных файлов (`_Temp0`/`_Temp1`, листинг `...0`/`...1`).
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#sectors
	BOOL bIsReduceElMagnAnalysis;
	/// @brief Сообщает, что решётка «большая» (больше 400 излучателей) и КИП для неё не считается.
	/// @return TRUE — N > `nMaxTemporary`: `GetKip` возвращает излучённую мощность
	///         (интеграл |F|^2 по углу), а не КИП; FALSE — `GetKip` возвращает КИП.
	/// @warning Имя обратно смыслу: TRUE означает, что КИП *не* вычисляется.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#large-arrays
	BOOL IsCalculateKip( void );
	/// Номер текущей точки сетки углов в цикле расчёта ДН (0..`nAngle`); окно прогресса
	/// показывает `iN/10` процентов (рассчитано на 1000 точек большой решётки).
	int iN;
	/// @brief Возвращает нормирующий множитель `dMax`: модуль ДН |F| = `GetdAmplitudeDiagrNapr(i)` * `dMax`.
	/// @return Для N <= 400 — 1.001 * max|F| на мелкой сетке нормировки; для N > 400 — максимум
	///         |F| на сетке графика; -1 — ДН ещё не нормировалась.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#normalization
	double GetdMax( void );
	/// @brief Возвращает частоту решётки, ГГц (строка `Frequence` файла `.DAT`).
	/// @return Частота, ГГц; 0 — не задана.
	double GetdFreqGHz( void );
	/// @brief Считает ДН на сетке углов требуемой ДН и записывает файл сравнения `.dev`.
	///
	/// Формат: строка-заголовок `!Degree ...`, затем по строке на угол: угол (градусы),
	/// требуемая ДН, рассчитанная нормированная ДН (через табуляцию). Сетку углов и требуемую
	/// ДН задаёт `OptimizationInit`, излучатели — `SetIzl`/`CopyIzlSector`. Ошибка — в `IsError()`.
	/// @param[in] pszFileName путь к файлу `.dev`
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#deviation-pattern
	void SaveDeviationDirectionalPattern( char *pszFileName );
	/// @brief Возвращает параметры излучателя `i`.
	/// @param[in]  i           номер излучателя, 0..N-1
	/// @param[out] dXmm        координата X, мм
	/// @param[out] dYmm        координата Y, мм
	/// @param[out] dAmpl       амплитуда (единицы файла `.DAT`)
	/// @param[out] dFazaDegree фаза, градусы
	/// @warning Проверка `i > nIzl` пропускает `i == nIzl` — обращение за конец массива.
	void GetIzl( int i, double *dXmm, double *dYmm, double *dAmpl, double *dFazaDegree);
	/// @brief Записывает решётку в файл формата `.DAT`: частота, N, выражение ДН элемента, излучатели (8 полей).
	///
	/// КНД и точки ДН не пишутся. Выражение пишется как есть (прочитанное `ReadData` уже
	/// содержит перевод строки). Используется окном девиаций распределения.
	/// @param[in] pszFile путь к файлу
	void SaveData( char *pszFile );
	/// @brief Копирует излучателю `i` сектор излучения FiMin..FiMax и выражение ДН излучателя `i` из `cSource`.
	/// @param[in] cSource решётка-источник (та же нумерация излучателей)
	/// @param[in] i       номер излучателя, 0..N-1 в обеих решётках
	void CopyIzlSector( c2DArray &cSource, int i );	// A-31: sektor i diagramma izluchatelya i iz cSource
	/// @brief Создаёт решётку со случайными отклонениями параметров излучателей и записывает её в файл.
	///
	/// Читает `szSourceFile` и каждому излучателю независимо задаёт значение, равномерно
	/// распределённое в пределах: амплитуда A ± A*`dAmplProcent`/100, фаза ± `dFazaDegree`,
	/// координаты ± `dXmm`, ± `dYmm`. Результат — `SaveData(szDistinationFile)`.
	/// @param[in] szSourceFile      исходный файл `.DAT`
	/// @param[in] szDistinationFile файл `.DAT` с отклонениями
	/// @param[in] dXmm              предельное отклонение X, мм
	/// @param[in] dYmm              предельное отклонение Y, мм
	/// @param[in] dAmplProcent      предельное отклонение амплитуды, % от амплитуды
	/// @param[in] dFazaDegree       предельное отклонение фазы, градусы
	/// @note Генератор `rand()` не инициализируется: при каждом запуске программы
	///       последовательность отклонений одна и та же.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#deviation
	void MakeDeviation( char *szSourceFile, char *szDistinationFile, double dXmm, double dYmm, double dAmplProcent, double dFazaDegree );
	/// @brief Задаёт частоту решётки.
	/// @param[in] dFrequenceGHz частота, ГГц (хранится в Гц)
	void SetdFrequence( double dFrequenceGHz );
	/// @brief Задаёт выражение ДН элемента без проверки длины (`strcpy`).
	/// @param[in] ch выражение от угла `f` (радианы); короче `TMC_C2DARRAY_MAX_STRING`
	void SetszOneExpression( char *ch );
	/// @brief Задаёт координаты, амплитуду и фазу излучателя `i`.
	/// @param[in] i            номер излучателя, 0..N-1
	/// @param[in] dXcoord_mm   координата X, мм
	/// @param[in] dYcoord_mm   координата Y, мм
	/// @param[in] dAmplitud    амплитуда
	/// @param[in] dFaza_gradus фаза, градусы
	/// @warning Проверка `i > nIzl` пропускает `i == nIzl` — запись за конец массива.
	void SetIzl( int i, double dXcoord_mm, double dYcoord_mm, double dAmplitud, double dFaza_gradus );
	/// @brief Читает требуемую ДН (файл `.opt`) и готовит решётку из `nIzl1` излучателей на её сетке углов.
	///
	/// Файл — строки «угол_в_градусах значение»; строки, где не прочитались два числа,
	/// пропускаются. Число точек сетки = число таких строк (не меньше 2). Прежние данные
	/// удаляются; излучатели создаются пустыми и заполняются `SetIzl`.
	/// @param[in] szFileRequiDirPat путь к файлу требуемой ДН
	/// @param[in] nIzl1             число излучателей
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#deviation-pattern
	void OptimizationInit( char *szFileRequiDirPat, int nIzl1 );
	/// @brief Читает решётку из файла `.DAT` (прежние данные удаляются).
	///
	/// Формат — `Frequence = <ГГц> Ghz;`, `N = <число>;`, строка выражения ДН элемента, затем
	/// N строк излучателей (8 полей с сектором и ДН излучателя или 5 полей без них). Для
	/// N > 400 сетка углов становится -pi..pi из 1000 точек. Задаёт имена кэша `<файл>_Temp<0|1>`.
	/// @param[in] szFileName1 путь к файлу; не длиннее 800 символов
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#dat-input
	/// @see https://www.tamic.ru/docs/theory/radiation.html#pattern
	void ReadData( char *szFileName1 );
	/// @brief Возвращает выражение ДН элемента (третья строка `.DAT`, вместе с переводом строки).
	/// @return Указатель на внутренний буфер.
	char * GetOneExpression( void );
	/// @brief Возвращает число излучателей N.
	/// @return N; 0 — ошибка.
	int GetnIzl( void );
	/// @brief Возвращает КНД в децибелах: 10*lg(КНД).
	/// @return КНД, дБ; 0 — ошибка.
	double GetKnd_dB( void );
	/// @brief Возвращает КНД двумерной задачи, разы.
	/// @return КНД; `FLT_MIN` — не считался; 0 — ошибка.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#directivity
	/// @see https://www.tamic.ru/docs/theory/radiation.html#directivity
	double GetKnd( void );
	/// @brief Возвращает `GetKip()` в децибелах: 10*lg.
	/// @return Значение, дБ; 0 — ошибка.
	double GetKip_dB( void );
	/// @brief Возвращает КИП (N <= 400) или излучённую мощность (N > 400, см. `IsCalculateKip`).
	/// @return КИП, разы, либо интеграл |F|^2 по углу (условные единицы); `FLT_MIN` — не считался; 0 — ошибка.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#efficiency
	/// @see https://www.tamic.ru/docs/theory/radiation.html#efficiency
	double GetKip( void );
	/// @brief Перестраивает сетку углов графика на [dAngleMin, dAngleMax] (`nAngle` точек), пересчитывает ДН и листинг.
	///
	/// Нормировка не меняется (делится на прежний `dMax`). У большой решётки после первого
	/// расчёта ничего не делает: её сетка остаётся -pi..pi.
	/// @param[in] dAngleMin начало диапазона, радианы
	/// @param[in] dAngleMax конец диапазона, радианы
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#angle-grid
	void ResizeAngle( double dAngleMin, double dAngleMax );
	/// @brief Возвращает нормированный модуль ДН в точке `i` сетки углов.
	/// @param[in] i номер точки, 0..`GetnAngle()`-1
	/// @return |F|/`dMax`; 0 — неверный номер или ошибка.
	double GetdAmplitudeDiagrNapr( int i );
	/// @brief Возвращает угол точки `i` сетки графика.
	/// @param[in] i номер точки, 0..`GetnAngle()`-1
	/// @return Угол, радианы (отсчёт от оси X против часовой стрелки); 0 — неверный номер или ошибка.
	double GetdAngle( int i );
	/// @brief Полный расчёт: читает `.DAT`, считает ДН на сетке углов, нормирует, считает КНД/КИП, пишет листинг.
	///
	/// Листинг — файл `<szOutputFileName>0` или `...1` (по `bIsReduceElMagnAnalysis`): излучатели,
	/// КНД, КИП и точки ДН; удаляется деструктором. Для N >= 400 ДН пишется и в кэш.
	/// @param[in] szInputFileName  путь к файлу излучателей `.DAT`
	/// @param[in] szOutputFileName основа имени листинга
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#calc-flow
	void CalculateDiagrNapr( char *szInputFileName, char *szOutputFileName );
	/// @brief Сообщает, была ли ошибка.
	/// @return TRUE — ошибка (текст — `GetError()`).
	BOOL IsError( void );
	/// @brief Возвращает число точек сетки углов графика (400; для большой решётки 1000; после `OptimizationInit` — число точек требуемой ДН).
	/// @return Число точек.
	int  GetnAngle( void );
	/// @brief Возвращает текст последней ошибки (по-английски); без ошибки — `No Error`.
	/// @return Указатель на внутренний буфер.
	char  *GetError( void );
	/// @brief Удаляет излучатели и сбрасывает частоту, N, выражение, имена файлов, КНД, КИП, нормировку и ошибку.
	void DeleteData( void );
	/// @brief Создаёт пустую решётку: сетка 400 углов 0..pi, предел «малой» решётки 400, углы выражений в радианах.
	c2DArray();
	/// @brief Удаляет листинг и (для N < 400) кэш ДН, освобождает память.
	virtual ~c2DArray();

private:
	/// @brief Возвращает время последней записи файла.
	/// @param[in] szFileName путь к файлу
	/// @return Время записи; нули — файл не открылся.
	FILETIME LastWriteTime( char *szFileName );
	/// @brief Читает ДН из кэша: углы (градусы -> радианы), ненормированный модуль, фазу (градусы -> радианы).
	void ReadDataFromTempFile( void );
	/// @brief Проверяет, можно ли взять ДН из кэша вместо расчёта.
	/// @return TRUE — N >= 400, есть файл `.DAT` и кэш, кэш не старше `.DAT` и подпись в `<кэш>.chk`
	///         совпадает (размер и время `.DAT`, N, частота); FALSE — считать заново.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#cache
	BOOL IsDirectPatSaveInFile( void );
	/// @brief Записывает кэш ДН `<.DAT>_Temp<0|1>` и (для N >= 400) его подпись `.chk`; ставит `bIsInitialData`.
	///
	/// Строка кэша: угол (градусы), ненормированный модуль, нормированный модуль, фаза (градусы).
	/// Папка только для чтения — не ошибка (кэш просто не пишется).
	void WriteTemporaryFile( void );
	/// TRUE — ДН посчитана и сохранена (или сохранение не удалось). У большой решётки после
	/// этого `ReadData`, `CalculateDiagrNapr`, `ResizeAngle`, `WriteData` ничего не делают.
	BOOL bIsInitialData;
	/// @brief Возвращает случайное число, равномерно распределённое на [dMedium - dDelta, dMedium + dDelta].
	/// @param[in] dMedium среднее значение
	/// @param[in] dDelta  полуширина интервала
	/// @return Случайное значение.
	double GetRnd(double dMedium, double dDelta);
	/// Наименьшее число точек мелкой сетки нормировки на 360° (по умолчанию 1000).
	int nPointForNormir;
	/// Выражение ДН элемента — общий множитель всех излучателей (третья строка `.DAT`, с переводом
	/// строки); переменная `f` — угол, радианы. Ядро пишет `1.` (изотропный элемент).
	char szOneExpression[TMC_C2DARRAY_MAX_STRING];
	/// КНД, разы; `FLT_MIN` — не считался.
	double dKnd;
	/// КИП, разы (N <= 400) или излучённая мощность, интеграл |F|^2 по углу (N > 400); `FLT_MIN` — не считался.
	double dKip;
	/// TRUE — КНД, КИП и `dMax` уже посчитаны; повторные расчёты ДН делят на прежний `dMax`.
	BOOL IsNormirovka;
	/// @brief Считает КНД, КИП и нормирующий множитель `dMax` (один раз) и нормирует ДН на сетке графика.
	///
	/// Мелкая сетка на 360°: не меньше `nPointForNormir` точек и не меньше 64*k*Rmax + 1
	/// (k = 2*pi*F/c, Rmax — наибольшее удаление излучателя от начала координат), не больше
	/// 4 000 000, не меньше 16. На ней КНД = 2*pi*max|F|^2 / сумма(|F|^2*dphi); КИП = КНД / КНД0,
	/// где КНД0 — у равноамплитудной синфазной решётки той же геометрии (только N <= 400).
	/// Нулевая или нечисловая ДН — ошибка.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#normalization
	/// @see https://www.tamic.ru/docs/theory/radiation.html#directivity
	/// @see https://www.tamic.ru/docs/theory/radiation.html#efficiency
	void NormirovkaDiagrNapr( void );
	/// @brief Возвращает мнимую часть поля решётки в направлении `dAngle`: сумма A_i(phi)*sin(phi_i + k*r_i*cos(phi - alpha_i)).
	/// @param[in] dAngle направление, радианы
	/// @return Im F (без общей ДН элемента); 0 — ошибка или нет излучателей.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#array-factor
	double CalculateImageDN( double dAngle );
	/// @brief Возвращает действительную часть поля решётки в направлении `dAngle`: сумма A_i(phi)*cos(phi_i + k*r_i*cos(phi - alpha_i)).
	///
	/// A_i(phi) — амплитуда излучателя с его ДН и сектором (`cIzl::GetdAmplituda(phi)`),
	/// phi_i — его фаза, (r_i, alpha_i) — полярные координаты, k = 2*pi*F/c, c = 299792458 м/с.
	/// @param[in] dAngle направление, радианы
	/// @return Re F (без общей ДН элемента); 0 — ошибка или нет излучателей.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#array-factor
	/// @see https://www.tamic.ru/docs/theory/radiation.html#far-field
	double CalculateRealDN( double dAngle );
	/// @brief Как `CalculateImageDN`, но для опорной решётки КИП: амплитуды 1, фазы 0, без ДН и секторов излучателей.
	/// @param[in] dAngle направление, радианы
	/// @return Im F0.
	double CalculateImageDN_ForKip( double dAngle );
	/// @brief Как `CalculateRealDN`, но для опорной решётки КИП: амплитуды 1, фазы 0, без ДН и секторов излучателей.
	/// @param[in] dAngle направление, радианы
	/// @return Re F0.
	double CalculateRealDN_ForKip( double dAngle );
	/// @brief Освобождает массивы сетки углов и ДН; `nAngle` = 0.
	void DeleteAngle( void );
	/// @brief Выделяет массивы на `nAngle` точек и задаёт сетку 0..pi с равным шагом.
	void InitAngle( void );
	/// @brief Считает ДН на сетке углов (или берёт из годного кэша) и вызывает `NormirovkaDiagrNapr`.
	///
	/// В каждой точке F = r1(phi) * (Re + j*Im), где r1 — значение выражения ДН элемента;
	/// `pdRealDN` = |F|, `pdImageDN` = arg F. Ход цикла — в `iN`.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#calc-flow
	void CalculateDiagrNapr( void );
	/// @brief Задаёт имя листинга `<szFileName1><0|1>` и пишет его (`WriteData()`).
	/// @param[in] szFileName1 основа имени; не длиннее 800 символов
	void WriteData( char *szFileName1 );
	/// @brief Выделяет массив из N излучателей и обнуляет ДН; N < 1 или нет памяти — ошибка.
	void InitData( void );
	/// @brief Пишет листинг: частота, N, выражение, излучатели, КНД (разы и дБ), КИП или излучённая мощность, точки ДН (угол, модуль, фаза в градусах); затем кэш.
	///
	/// Папка только для чтения — не ошибка: листинг не пишется, пишется только кэш.
	void WriteData( void );
	/// @brief Читает файл `szFileName` (см. `ReadData(char*)`); проверяет N >= 1, соответствие N размеру файла и конечность чисел.
	void ReadData( void );
	/// Путь кэша ДН: `<.DAT>_Temp<0|1>`; подпись — `<кэш>.chk`.
	char szFileNameTemp[TMC_C2DARRAY_MAX_STRING];
	/// Путь входного файла `.DAT`; пусто — решётка не из файла.
	char szFileName[TMC_C2DARRAY_MAX_STRING];
	/// Путь листинга `<основа><0|1>`; удаляется деструктором.
	char szFileNameOut[TMC_C2DARRAY_MAX_STRING];
	/// Текст последней ошибки; `No Error` — ошибки нет.
	char szError[TMC_C2DARRAY_MAX_STRING];
	/// Нормирующий множитель ДН (см. `GetdMax`); -1 — не посчитан.
	double dMax;
	/// Предел «малой» решётки: 400 излучателей. Больше — КИП не считается, сетка 1000 точек;
	/// не меньше — ДН кэшируется.
	int nMaxTemporary;
	/// Число точек сетки углов графика.
	int nAngle;
	/// Углы сетки графика, радианы; `nAngle` значений.
	double *pdAngle;
	/// Модуль ДН в точках сетки: до нормировки |F|, после — |F|/`dMax`.
	double *pdRealDN;
	/// Требуемая ДН из файла `.opt` (`OptimizationInit`), в точках сетки.
	double *pdRequiredDN;
	/// Фаза ДН arg F в точках сетки, радианы (несмотря на имя — не мнимая часть).
	double *pdImageDN;
	/// Частота, Гц.
	double dFreq;
	/// TRUE — была ошибка (текст в `szError`).
	BOOL bIsError;
	/// Число излучателей N.
	int    nIzl;
	/// Массив из N излучателей; владеет объект.
	cIzl *pcIzl;
};

#endif // !defined(AFX_C2DARRAY_H__15B05DDC_FBD2_4046_87E5_F234F2BA20BF__INCLUDED_)
