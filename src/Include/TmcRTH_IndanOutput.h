// TmcRTH_IndanOutput.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_IndanOutput window

/// @file TmcRTH_IndanOutput.h
/// @brief Секция `#OUTPUT` задания: имена выходных файлов, S-матрица, поле, диаграмма
///        направленности.
///
/// Часть библиотеки TMCIndan. Класс `CTmcRTH_IndanOutput` разбирает строки
/// `FILE`, `TOPOLOGY`, `FIELDS`, `SMATRIX`, `FIELD_DISTRIBUTION[_M]`, хранит имена
/// файлов результатов и по окончании шага пересчитывает временные сигналы
/// `.t` в матрицу рассеяния `.s` (`PutSmatrix`).
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#output

#ifndef CTMCRTH_INDANOUTPUT_H__
#define CTMCRTH_INDANOUTPUT_H__

/// `FILE <имя>;` — базовое имя (без точки в имени файла) всех выходных файлов шага.
#define CTMCRTH_FILEOUT_ID      "FILE "
/// `TOPOLOGY` — записывать топологию (файл `.tt`).
#define CTMCRTH_TOPOLOGYOUT_ID  "TOPOLOGY"
/// `FIELDS` — записывать поле (файлы `.ex`, `.ey`, `.hz`).
#define CTMCRTH_FIELDSOUT_ID    "FIELDS"
/// `SMATRIX <файл>; <F>; [<Tmin>; <Tmax>;]` — S-матрица на частоте F по окну Tmin..Tmax.
#define CTMCRTH_SMATRIXOUT_ID   "SMATRIX"
/// `FIELD_DISTRIBUTION <имя>; <F>; <Tmin>; <Tmax>;` — ДН с накоплением квадратур в файлах.
#define CTMCRTH_FLDSDSTROUT_ID  "FIELD_DISTRIBUTION "
/// `FIELD_DISTRIBUTION_M <имя>; <F>; <Tmin>; <Tmax>;` — ДН с накоплением квадратур в памяти.
#define CTMCRTH_FLDSDSMROUT_ID  "FIELD_DISTRIBUTION_M "


#include <TmcLibError.h>
#include <FieldIntegrated.h>

class CTmcRTH_BlockList;	// D-1


/// @brief Выходные файлы шага #STEP и пересчёт сигналов в S-матрицу.
///
/// Принадлежит `CTmcRTH_Indan` (поле `cOut`); заполняется методом `Read`
/// по строкам секции `#OUTPUT`. Перед разбором `CTmcRTH_Indan` передаёт единицы
/// частоты и времени (`SetFreqAndTimeUnit`). Содержит интегратор поля
/// `CFieldIntegrated` для диаграммы направленности.
/// Все времена — в с, частоты — в Гц.
/// @note Потоки: заполняется в потоке чтения задания; `PutSmatrix` и интегратор
///       вызываются расчётным потоком ядра.
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#output
class CTmcRTH_IndanOutput //: public CWnd
{
// Construction
public:
	/// @brief Приписывает папку задания ко всем именам выходных файлов, если имя `.t`
	///        не содержит полного пути.
	/// @param[in] csPath папка задания
	void AddPath( CString csPath );
	/// @brief Копирует все имена, флаги, окно S-матрицы и интегратор поля.
	/// @param[in] cOut источник
	/// @return `*this`.
	CTmcRTH_IndanOutput& operator=( CTmcRTH_IndanOutput& cOut );
	/// Начало окна сигналов для S-матрицы, с.
	double dTmin;
	/// Конец окна сигналов для S-матрицы, с; -1 — окно не задано (`SMATRIX имя; F;`),
	/// его задаёт `CTmcRTH_Indan::ReadOutputSection`.
	double dTmax;
	/// Частота S-матрицы из `SMATRIX`, Гц.
	double dFreq;
	/// Множитель единицы частоты задания (Гц в единице `FREQ_UNIT`).
	double dFreqUnit;
	/// Множитель единицы времени задания (с в единице `TIME_UNIT`).
	double dTimeUnit;
	/// TRUE — в секции есть `SMATRIX`: по окончании шага писать `.s`.
	BOOL bOutSMatrix;
	/// TRUE — выводить поле `.ex` (включается `FIELDS`, переключается из интерфейса).
	BOOL bOutField;
	/// TRUE — выводить поле `.ey`.
	BOOL bOutField1;
	/// TRUE — выводить поле `.hz`.
	BOOL bOutFieldU;
	/// TRUE — записывать топологию `.tt` (`TOPOLOGY`).
	BOOL bOutTopology;
	/// TRUE — синхронный вывод поля (переключается из интерфейса ядра).
	BOOL bOutFieldSinchronization;
	/// Файл временных сигналов на входах `<имя>.t`.
	CString csFileName;
	/// Файл поля `<имя>.ex`.
	CString csFileNameField;
	/// Файл поля `<имя>.ey`.
	CString csFileNameField1;
	/// Файл `<имя>.em`.
	CString csFileNameField1Mod;
	/// Файл `<имя>.ef`.
	CString csFileNameField1Faz;
	/// Файл поля `<имя>.hz`.
	CString csFileNameFieldU;
	/// Файл топологии `<имя>.tt`.
	CString csFileNameTopology;
	/// Файл S-матрицы: имя из `SMATRIX` (без расширения — с `.s`); `FILE` задаёт `<имя>.s`.
	CString csFileNameSMatrix;
	/// Файл `<имя>.q`.
	CString csFileNameQ;
	/// Файл `<имя>.ix`.
	CString csFileNameIx;
	/// Файл `<имя>.iy`.
	CString csFileNameIy;
	/// @brief Проверяет, что в секции задано имя `FILE`.
	/// @param[in,out] cError1 ошибка «`Section #OUTPUT output file name is absent`»
	void IsCorrectly( CTmcLibError &cError1 );
	/// @brief Разбирает одну строку секции `#OUTPUT`.
	/// @param[in]     csCh    строка секции
	/// @param[in,out] cError1 ошибка; неизвестное слово — «Unknown keyword…»
	void Read( CString &csCh, CTmcLibError &cError1 );
	/// @brief Сбрасывает имена файлов, флаги вывода S-матрицы/поля/топологии, окно и интегратор.
	/// @note Флаги `bOutField1`, `bOutFieldU`, `bOutFieldSinchronization` и `bSoundEffect`
	///       не сбрасываются — это переключатели интерфейса.
	void DeleteData( void );
	/// @brief Создаёт пустое описание вывода (звук включён).
	CTmcRTH_IndanOutput();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTH_IndanOutput)
	//}}AFX_VIRTUAL

// Implementation
public:
	/// @brief Возвращает интегратор поля для диаграммы направленности.
	/// @return Ссылка на поле `cFieldDistr`; принадлежит объекту.
	CFieldIntegrated& GetFieldIntegrated( void );
	/// @brief Пересчитывает временные сигналы `.t` в S-матрицу `.s` (если задан `SMATRIX`).
	///
	/// Файл `.t` с многочастотным заголовком (падающая волна из файла) обрабатывается
	/// классом `CTmcWideband` — все частоты сигнала за один проход; иначе — `CTmcTtoS`
	/// на одной частоте `dFreq` по окну `dTmin..dTmax`. Результат дописывается
	/// в `csFileNameSMatrix` как одна частотная точка.
	/// Ошибка сохраняется в `csSMatrixError`, предупреждение — в `csSMatrixWarning`.
	/// @see https://www.tamic.ru/docs/theory/smatrix.html#s-from-signals
	/// @see https://www.tamic.ru/docs/theory/smatrix.html#multifreq
	void PutSmatrix( void );
	/// @brief Записывает в `.s` нулевую S-матрицу на частоте `SMATRIX` для шага,
	///        посчитанного с ошибкой (чтобы ошибочная точка свипа была видна сразу).
	///
	/// Число портов — число входов в списке блоков. При падающей волне из файла
	/// (`[имя]` у входа) ничего не пишет: нули на плохих частотах ставит `CTmcWideband`.
	/// @param[in] pcBl голова списка блоков шага
	/// @return 1 — нули записаны; 0 — писать не нужно (нет `SMATRIX`, нет входов,
	///         многочастотный расчёт); -1 — файл `.s` записать не удалось.
	int PutSmatrixZero( CTmcRTH_BlockList *pcBl );	// D-1: zeros at the frequency of SMATRIX (1 - written, 0 - not needed, -1 - error)
	/// Ошибка последнего `PutSmatrix` (пусто — ошибки нет).
	CString csSMatrixError;		// A-21: oshibka poslednego PutSmatrix ("" - net oshibki)
	/// Предупреждение последнего `PutSmatrix` (пусто — нет).
	CString csSMatrixWarning;	// B-22: preduprezhdenie poslednego PutSmatrix ("" - net)
	/// @brief Задаёт единицы частоты и времени задания для перевода значений секции в СИ.
	/// @param[in] dFr  Гц в единице `FREQ_UNIT`
	/// @param[in] dTim с в единице `TIME_UNIT`
	void SetFreqAndTimeUnit( double dFr, double dTim );
	/// @brief Переключает звуковые сигналы и пишет «Sound on/off» в строку статистики.
	void OnOffSoundEffects( void );
	/// TRUE — звуковые сигналы окончания счёта включены.
	BOOL bSoundEffect;
	/// @brief Переключает синхронный вывод поля и обновляет строку состояния.
	void OnOffSinchronizationFieldOutput( void );
	/// @brief Переключает вывод поля `.ex` и обновляет строку состояния.
	void OnOffFieldOutput( void );
	/// @brief Уничтожает объект (вызывает `DeleteData`).
	virtual ~CTmcRTH_IndanOutput();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcRTH_IndanOutput)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
//	DECLARE_MESSAGE_MAP()
private:
	/// @brief Разбирает `SMATRIX <файл>; <F>; [<Tmin>; <Tmax>;]`.
	///
	/// F — в единицах `FREQ_UNIT`, > 0; Tmin < Tmax — в единицах `TIME_UNIT`.
	/// Без окна `dTmax` = -1 (окно задаётся при чтении шага).
	/// @param[in] csCh строка секции
	void ReadSmatrix( CString &csCh );
	/// @brief Содержит ли имя `.t` полный путь (начало `\` или `/`, или символ `:`).
	/// @return TRUE — путь полный.
	BOOL IsFullPath( void );
	/// @brief Удаляет из строки пробелы, табуляции и переводы строк (на месте).
	/// @param[in,out] ch строка, завершённая нулём
	/// @return Всегда 0.
	int expr_del_Blanks2( char *ch );
	/// @brief Выделяет очередной параметр до `;` верхнего уровня (вне скобок `{}()[]`).
	/// @param[out]    csBuf текст параметра без `;`
	/// @param[in,out] szBuf строка с текущей позиции; временно изменяется и восстанавливается
	/// @return Сдвиг позиции за `;`; -1 и ошибка в `cError`, если `;` нет.
	int Search_1Param( CString &csBuf, char *szBuf );
	/// @brief Разбирает `FIELD_DISTRIBUTION[_M] <имя>; <F>; <Tmin>; <Tmax>;` и настраивает интегратор.
	///
	/// Окно Tmax округляется вверх до целого числа периодов частоты F:
	/// `Tmax = Tmin + ceil((Tmax - Tmin)*F)/F` (с допуском 1e-4 периода).
	/// @param[in] csCh строка секции
	/// @param[in] ch   ключевое слово, с которого начинается строка (его длина пропускается)
	/// @see https://www.tamic.ru/docs/theory/radiation.html#pattern
	void ReadFileDistr( CString &csCh, const char *ch );
	/// @brief Разбирает `FIELD_DISTRIBUTION_M` и включает накопление квадратур в памяти.
	/// @param[in] csCh строка секции
	void ReadFileDistrInMemory( CString &csCh );
	/// @brief Разбирает `FILE <имя>;` и строит имена всех выходных файлов `<имя>.<расширение>`.
	/// @param[in] csCh строка секции; точка в имени файла (не в папке) — ошибка
	void ReadFileName( CString &csCh );
	/// @brief Обрабатывает `TOPOLOGY`: включает запись топологии.
	void ReadTopology( void );
	/// @brief Обрабатывает `FIELDS`: включает вывод полей `.ex`, `.ey`, `.hz`.
	void ReadFields( void );
	/// Ошибка разбора секции `#OUTPUT`.
	CTmcLibError cError;
	/// Интегратор поля для диаграммы направленности (`FIELD_DISTRIBUTION[_M]`).
	CFieldIntegrated cFieldDistr;
};


#endif

/////////////////////////////////////////////////////////////////////////////
