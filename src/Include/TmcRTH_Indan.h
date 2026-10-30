// TmcRTH_Indan.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_Indan window

/// @file TmcRTH_Indan.h
/// @brief Чтение задания ядра (.tpl): препроцессор, поиск шага #STEP и разбор его секций.
///
/// Главный заголовок библиотеки TMCIndan. Объявляет класс `CTmcRTH_Indan` —
/// точку входа ядер H и X в разбор задания — ключевые слова (метки) секций
/// задания и функцию `MainPrepr`, вызывающую препроцессор PREPR.
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html

#ifndef CTMCRTH_INDAN_H__
#define CTMCRTH_INDAN_H__

#include <TmcLibError.h>
#include <TmcRTH_IndanParam.h>
#include <TmcRTH_IndanOutput.h>
#include <TmcRTH_IndanTopology.h>

/// Размер строковых буферов разбора строк секций (символов).
#define CTMCRTH_INDANSBUF 30000
/// Вставка в имя временного файла препроцессора: `<задание>~$Prepr$~<номер процесса>`.
#define CTMCRTH_INDANPREPRFNA "~$Prepr$~"
/// Наибольшая длина строки задания после препроцессора, включая `\n` и нуль (символов).
#define CTMCRTH_INDAN_LINELEN     20000

/// Метка конца задания; строки после неё не читаются. Обязательна.
#define CTMCRTH_INDANMET_EOF         "#EOF"
/// Начало шага расчёта (точки свипа); шаги нумеруются с 1 по порядку в файле.
#define CTMCRTH_INDANMET_STEP	     "#STEP"
/// Конец шага расчёта.
#define CTMCRTH_INDANMET_ENDSTEP     "#END_STEP"
/// Начало секции параметров (обязательна в шаге 1).
#define CTMCRTH_INDANMET_PARAM	     "#PARM"
/// Конец секции параметров.
#define CTMCRTH_INDANMET_ENDPARAM    "#END_PARM"
/// Начало секции топологии (список блоков `BLOCK ... END_B`).
#define CTMCRTH_INDANMET_TOPOLOGY    "#TOPOLOGY"
/// Конец секции топологии.
#define CTMCRTH_INDANMET_ENDTOPOLOGY "#END_TOPOLOGY"
/// Начало секции размещения блоков (строки `T n; X0; Y0;`).
#define CTMCRTH_INDANMET_LINKLIST	 "#LINK_LIST"
/// Конец секции размещения блоков.
#define CTMCRTH_INDANMET_ENDLINK	 "#END_LINK"
/// Начало секции выходных файлов.
#define CTMCRTH_INDANMET_OUTPUT		 "#OUTPUT"
/// Конец секции выходных файлов.
#define CTMCRTH_INDANMET_ENDOUTPUT	 "#END_OUTPUT"

/// Начало описания блока: `BLOCK n;` (n >= 1), далее строка примитива.
#define CTMCRTH_INDANMET_BLOCK	  "BLOCK "
/// Конец описания блока.
#define CTMCRTH_INDANMET_ENDBLOCK "END_B"
/// Строка размещения блока в `#LINK_LIST`: `T n; X0; Y0;` — начало локальных координат
/// блока n помещается в точку (X0, Y0) области.
#define CTMCRTH_INDANMET_LINK	  "T "

/// @brief Запускает препроцессор PREPR: разворачивает макросы и `#include` задания
///        и пишет результат во временный файл.
///
/// @param[in]     csTplFileNamePrepr имя временного файла результата (не длиннее 511 символов)
/// @param[in]     csTplFileName      имя исходного задания .tpl (не длиннее 511 символов)
/// @param[in]     csCurrentPath      папка задания — для поиска подключаемых файлов
/// @param[in,out] cError             ошибка; если уже взведена, препроцессор не запускается;
///                                   при ошибке препроцессора получает его сообщение
/// @see https://www.tamic.ru/docs/code/lib-prepr.html
void MainPrepr( CString csTplFileNamePrepr,	CString csTplFileName, CString csCurrentPath, CTmcLibError &cError );


/// @brief Задание ядра: препроцессор, поиск шага #STEP и разбор его секций в параметры,
///        топологию и выходные файлы.
///
/// Владеет параметрами (`cParam`), топологией (`cTopology`) и описанием вывода
/// (`cOut`) текущего шага. Порядок работы: `ReadData(имя, шаг)` — задаёт файл
/// (запускает препроцессор во временный файл `<задание>~$Prepr$~<pid>`), проверяет номер
/// шага и читает секции `#PARM` (накопительно по шагам 1..N), `#TOPOLOGY`,
/// `#LINK_LIST`, `#OUTPUT` шага N. Каждая секция ищется заново от начала временного файла.
/// Временный файл удаляется в `DeleteData`/деструкторе. Исключения не бросаются:
/// ошибка запоминается в `cError`, и после неё методы чтения ничего не делают
/// (проверять `IsError()` после каждого вызова).
/// Создаётся ядром (`CTmcRTHRectNode::cIndan`), живёт всё время открытого задания.
/// @note Потоки: не потокобезопасен; вызывается из потока чтения задания ядра.
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#reading
class CTmcRTH_Indan //: public CWnd
{
// Construction
public:
	/// @brief Возвращает число блоков топологии текущего шага.
	/// @return Число блоков (0 — топология не прочитана).
	int GetnBlock( void );
	/// @brief Возвращает границы области расчёта (см. `CTmcRTH_IndanParam::GetXYMinMax`).
	/// @param[out] dXmin левая граница, м
	/// @param[out] dXmax правая граница, м
	/// @param[out] dYmin нижняя граница, м
	/// @param[out] dYmax верхняя граница, м
	void GetXYMinMax( double &dXmin, double &dXmax, double &dYmin, double &dYmax );
	/// @brief Возвращает шаг сетки.
	/// @return Delta, м.
	double dGetDelta( void );
	/// @brief Возвращает шаг по времени.
	/// @return dT, с.
	double dGetdT( void );
	/// @brief Возвращает начало интервала расчёта.
	/// @return Tmin, с.
	double dGetTmin( void );
	/// @brief Возвращает число шагов по времени.
	/// @return `(Tmax - Tmin)/dT`, 0 — интервал пуст.
	int nGetTmax( void );
	/// @brief Возвращает номер текущего шага #STEP.
	/// @return Номер шага (с 1); 0 — не задан.
	int GetnStep( void );
	/// @brief Прочитана ли топология.
	/// @return Всегда TRUE (см. `CTmcRTH_IndanTopology::IsDataRead`).
	BOOL IsDataRead( void );
	/// @brief Возвращает описание выходных файлов текущего шага.
	/// @return Указатель на поле `cOut`; принадлежит объекту.
	CTmcRTH_IndanOutput* GetOutput( void );
	/// @brief Возвращает топологию текущего шага.
	/// @return Ссылка на поле `cTopology`.
	CTmcRTH_IndanTopology& GetTopology( void );
	/// @brief Возвращает параметры текущего шага.
	/// @return Ссылка на поле `cParam`.
	CTmcRTH_IndanParam& GetParam( void );
	/// @brief Читает секцию `#TOPOLOGY` текущего шага: блоки `BLOCK ... END_B`.
	///
	/// Очищает прежний список блоков, разбирает каждый блок, проверяет
	/// повторы номеров и дописывает папку задания к именам файлов блоков `FILE*`.
	/// Ошибка, если секции нет.
	void ReadTopologySection( void );
	/// @brief Читает секцию `#LINK_LIST` текущего шага: размещение блоков `T n; X0; Y0;`.
	///
	/// После чтения проверяет, что размещён каждый блок топологии.
	/// Вызывать после `ReadTopologySection`.
	void ReadLinkListSection( void );
	/// @brief Читает секцию `#OUTPUT` текущего шага и проверяет согласованность `SMATRIX`.
	///
	/// Для одночастотного расчёта требует окно `Tmin; Tmax` у `SMATRIX` и частоту,
	/// равную `FREQ` шага; для расчёта с падающей волной из файла (`[имя]` у входа)
	/// окно по умолчанию — весь интервал `0..Tmax`. Дописывает папку задания к именам файлов.
	/// Вызывать после `ReadParamSection` и `ReadTopologySection`.
	void ReadOutputSection( void );
	/// @brief Читает секции `#PARM` шагов 1..N (N — текущий шаг) и проверяет параметры.
	///
	/// Параметры шага 1 обязательны; в следующих шагах секция `#PARM` может отсутствовать —
	/// тогда действуют значения предыдущих шагов.
	void ReadParamSection( void );
	/// @brief Задаёт номер текущего шага #STEP.
	/// @param[in] nStep1 номер шага, 1..`GetNStepMax()`
	void SetStepNumber( int nStep1 );
	/// @brief Возвращает число шагов #STEP в уже заданном файле.
	///
	/// Заодно проверяет парность `#STEP`/`#END_STEP` и то, что ни одному файлу
	/// `SMATRIX` не задано больше 99 разных частот (предел файла `.s`).
	/// @return Число шагов; 0 при ошибке (текст — `GetErrorMessage`).
	int GetNStepMax( void );
	/// @brief Задаёт файл задания (`SetTplFileName`) и возвращает число шагов #STEP в нём.
	/// @param[in] csFileName путь к заданию .tpl
	/// @return Число шагов; 0 при ошибке.
	int GetNStepMax( CString csFileName );
	/// @brief Возвращает текст ошибки чтения задания.
	/// @return Текст ошибки (ASCII, английский); пусто — ошибки нет.
	CString GetErrorMessage( void );
	/// @brief Есть ли ошибка.
	/// @return TRUE — ошибка была; объект надо пересоздать или вызвать `SetTplFileName` заново.
	BOOL IsError( void );
	/// @brief То же, что `SetTplFileName(CString)`, для строки C.
	/// @param[in] pszFileName путь к заданию .tpl
	void SetTplFileName( char *pszFileName );
	/// @brief Задаёт файл задания и прогоняет его через препроцессор.
	///
	/// Сбрасывает все данные (`DeleteData`), проверяет, что файл открывается,
	/// запоминает папку задания и пишет развёрнутое задание во временный файл
	/// `<задание>~$Prepr$~<номер процесса>` (номер процесса — чтобы два запуска одного
	/// задания не делили файл; при слишком длинном пути — без номера).
	/// @param[in] csFileName путь к заданию .tpl
	/// @warning В Windows путь к заданию и к временному файлу должен быть короче 260 символов,
	///          иначе — ошибка с советом перенести задание в папку с коротким путём.
	void SetTplFileName( CString csFileName );
	/// @brief Создаёт пустой объект (без файла и без ошибки).
	CTmcRTH_Indan();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTH_Indan)
	//}}AFX_VIRTUAL

// Implementation
public:
	/// @brief Включает/выключает звуковые сигналы окончания счёта (см. `CTmcRTH_IndanOutput`).
	void OnOffSoundEffects( void );
	/// @brief Возвращает папку задания (полный путь без завершающего разделителя).
	/// @return Папка; пусто, если путь не содержит разделителя.
	CString GetCurrentPath( void );
	/// @brief Возвращает число узлов сетки.
	/// @param[out] nX11 число узлов по X
	/// @param[out] nY11 число узлов по Y
	void GetnXnY( int &nX11, int &nY11 );
	/// @brief Переключает синхронный вывод поля (см. `CTmcRTH_IndanOutput`).
	void OnOffSinchronizationFieldOutput( void );
	/// @brief Переключает вывод поля в файл `.ex` (см. `CTmcRTH_IndanOutput`).
	void OnOffFieldOutput( void );
	/// @brief Уничтожает объект; удаляет временный файл препроцессора.
	virtual ~CTmcRTH_Indan();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcRTH_Indan)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
//	DECLARE_MESSAGE_MAP()
private:
	/// Топология (список блоков) текущего шага.
	CTmcRTH_IndanTopology cTopology;
	/// @brief Собирает текст одного блока `BLOCK n; ... END_B` секции `#TOPOLOGY`.
	///
	/// Строки блока склеиваются без разделителей (каждая строка уже оканчивается `\n`);
	/// `END_B` в результат не входит.
	/// @param[out] csCh текст блока, начиная с `BLOCK `
	/// @return 0 — блок прочитан; `EOF` — конец секции или ошибка (строка не `BLOCK`,
	///         нет `END_B`).
	int GetBlock( CString &csCh );
	/// @brief Читает следующую строку внутри секции.
	/// @param[out] csCh строка
	/// @return 0 — строка прочитана; `EOF` — встречен конец секции `csEndSection`
	///         или ошибка (`#END_STEP` или конец файла раньше конца секции).
	int GetLineSection( CString &csCh );
	/// Метка конца искомой секции (например, `#END_PARM`).
	CString csEndSection;
	/// Метка начала искомой секции (например, `#PARM`).
	CString csSection;
	/// @brief Ищет начало секции `csSection` от текущей позиции до `#END_STEP`.
	/// @return 0 — найдена (позиция — после метки); -1 — нет в шаге (файл закрывается).
	int FindSection( void );
	/// @brief Запоминает метки секции и ищет её начало (см. `FindSection()`).
	/// @param[in] pszSection    метка начала секции
	/// @param[in] pszEndSection метка конца секции
	/// @return 0 — найдена; -1 — нет в шаге.
	int FindSection( const char * pszSection,  const char * pszEndSection);
	/// @brief Открывает временный файл заново и встаёт после метки `#STEP` с номером `nStep1`.
	/// @param[in] nStep1 номер шага (с 1); больше числа шагов — ошибка «very big step number».
	void FindStep( int nStep1 );
	/// @brief Читает секцию `#PARM` одного шага в `cParam` и проверяет параметры.
	/// @param[in] nStep1 номер шага; при 1 параметры предварительно сбрасываются
	///                   и отсутствие секции — ошибка.
	void ReadParamSection( int nStep1 );
	/// Номер текущего шага #STEP (с 1; 0 — не задан).
	int nStep;
	/// @brief Сжимает пробелы и табуляции строки: убирает ведущие, серию заменяет одним символом.
	///
	/// Первый промежуток после ключевого слова всегда становится пробелом
	/// (ключевые слова сравниваются вместе с пробелом: `"BLOCK "`, `"FREQ "`).
	/// @param[in,out] szBuf строка, завершённая нулём
	void del_blanks( char * szBuf );
	/// @brief Читает следующую значимую строку временного файла.
	///
	/// Пропускает пустые строки и комментарии (`!` в начале строки), сжимает
	/// пробелы. Строка длиннее `CTMCRTH_INDAN_LINELEN - 2` — ошибка (длинный
	/// комментарий пропускается целиком).
	/// @param[out] csCh строка (с завершающим `\n`, если он был)
	/// @return 0 — строка прочитана; `EOF` — встречен `#EOF` (без ошибки)
	///         или конец файла без `#EOF` (ошибка «`Missing #EOF`»).
	int GetLine( CString &csCh );
	/// Открытый временный файл препроцессора (`NULL` — закрыт).
	FILE * pfPrepr;
	/// @brief Закрывает временный файл препроцессора (если открыт).
	void ClosePreprFile( void );
	/// @brief Открывает временный файл препроцессора на чтение (если ещё не открыт).
	void OpenPreprFile( void );
	/// @brief Считает шаги #STEP, проверяет парность меток и число частот `SMATRIX`.
	/// @return Число шагов; 0 при ошибке.
	int CalcNStepMax( void );
	/// @brief Запускает препроцессор: задание -> временный файл (`MainPrepr`).
	void MakePreprFile( void );
	/// @brief Определяет папку задания по полному пути `csTplFileName`.
	void SetCurrentPath( void );
	/// @brief Сбрасывает параметры, вывод, номер шага и ошибку; удаляет временный файл.
	/// @note Список блоков `cTopology` здесь не очищается — это делает `ReadTopologySection`.
	void DeleteData( void );
	/// Параметры (секции `#PARM` шагов 1..N).
	CTmcRTH_IndanParam  cParam;
	/// Описание выходных файлов (секция `#OUTPUT` шага N).
	CTmcRTH_IndanOutput cOut;
	/// Ошибка чтения задания (после неё чтение не продолжается).
	CTmcLibError cError;
	/// Папка задания (полный путь).
	CString csCurrentPath;
	/// Путь к заданию .tpl, как его передали.
	CString csTplFileName;
	/// Путь к временному файлу препроцессора (пусто — не создан).
	CString csTplFileNamePrepr;
public:
};


#endif
/////////////////////////////////////////////////////////////////////////////
