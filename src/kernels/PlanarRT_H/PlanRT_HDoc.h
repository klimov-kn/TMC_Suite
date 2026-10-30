// PlanRT_HDoc.h : interface of the CPlanRT_HDoc class
//
/////////////////////////////////////////////////////////////////////////////

/// @file PlanRT_HDoc.h
/// @brief Документ оболочки ядра: задание .tpl, номер шага \#STEP, расчёт в отдельном потоке, внешние программы.
///
/// Часть модуля «Оболочка счётного ядра». Один документ — одно открытое задание.
/// Документ владеет расчётным объектом `CTmcRTHRectNode` (сетка, блоки, вывод)
/// и передаёт ему команды «прочитать шаг», «посчитать шаг», «посчитать всё»,
/// «остановить». Здесь же — имена ключей настроек программы (раздел
/// `PLANRT_H_RAZDEL_INI`) и запуск внешних программ (вьюверы, редактор задания).
/// Файл общий по смыслу для ядер H и X: у X своя копия в каталоге `planarrt_x`.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#document

#ifndef PLANRT_HDOC__H__
#define PLANRT_HDOC__H__

#include "TmcRTHRectNode.h"

/// Размер буферов командной строки, пути и строк настроек, символов (с завершающим нулём).
#define N_BUF_MAX 10000
											
/// @name Ключи настроек программы
/// Раздел и имена ключей в файле настроек ядра (в Windows-сборке слоя совместимости —
/// ini-файл программы, см. `CWinApp::GetProfileString`). Раздел общий для H и X,
/// файл у каждой программы свой (имя — `TmcPlanRtAppInfo::appName`).
/// Значения: имена программ — путь к .exe или имя файла; цвет — десятичное число
/// `COLORREF`; форматы — строки формата `printf`; флаги — `0`/`1`; звук — 27 целых
/// через пробел (флаг, два интервала, две мелодии по 12 нот); положение окна —
/// `left right top bottom`, пиксели.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#settings
///@{
/// Имя раздела или ключа в файле настроек ядра (значение — см. описание группы).
#define PLANRT_H_RAZDEL_INI					"PlanRT_H Config"
#define PLANRT_H_EXTERNEDITORNAME_INI		"ExternEditorName"
#define PLANRT_H_EXTERNVIEWERNAME_INI		"ExternViewerName"
#define PLANRT_H_EXTERNFIELDVIEWERNAME_INI  "ExternFieldViewerName"
#define PLANRT_H_EXTERNDIRPATVIEWERNAM_INI  "ExternDirectPatViewerName"
#define PLANRT_H_EXTERNSMVIEWERNAME_INI		"ExternSmatrixViewerName"
#define PLANRT_H_BACKGROUNDCOLOR_INI		"BackGroundColor"
#define PLANRT_H_OUTFORMATNT_INI			"OutputFormatnT"
#define PLANRT_H_OUTFORMATDT_INI			"OutputFormatdT"
#define PLANRT_H_OUTFORMATNB_INI			"OutputFormatnB"
#define PLANRT_H_OUTFORMATINP_INI			"OutputFormatdInp"
#define PLANRT_H_OUTFORMATOUT_INI			"OutputFormatdOut"
#define PLANRT_H_OUTPUTSOUNDFLAG_INI		"SoundEffectsFlag"
#define PLANRT_H_OUTFORMATOUTFILED_INI		"OutputFormatdOutField"
#define PLANRT_H_OUTFIELDSINCHRFLAG_INI		"OutputSinchrFlagdOutField"
#define PLANRT_H_OUTFIELDFLAG_INI			"OutputFlagdOutField"
#define PLANRT_H_MNWNDSIZEPLACE_INI			"MainFrameSizeAndPlace__1"
///@}

/// @brief Документ оболочки ядра: одно задание .tpl, текущий шаг \#STEP и управление расчётом.
///
/// Создаётся главным окном Qt (`MainWindow::openDocument`) через `TmcCreate()` при
/// открытии задания, удаляется при закрытии программы. Владеет расчётным объектом
/// `cRectNode` (`CTmcRTHRectNode`): параметры сетки, топология блоков, вывод.
///
/// Жизненный цикл шага: `ReadData()` читает шаг `nStep` задания (препроцессор, блоки),
/// `RunStep()` считает его, `RunAll()` проходит все шаги от `nStep` до последнего.
/// `RunStep()` и `RunAll()` выполняются в расчётном потоке (`RunStepGlobal`,
/// `RunAllGlobal` из `pl_glfun.cpp`), остальные методы — в главном потоке.
///
/// Потоки: флаг `bIsReadData` объявлен `volatile` и служит одновременно признаком
/// «идёт чтение или счёт» и флагом продолжения: `Stop()` из главного потока сбрасывает
/// его, и расчётный цикл выходит на ближайшей проверке. Других средств синхронизации нет.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#document
class CPlanRT_HDoc : public CDocument
{
protected: // create from serialization only
	/// @brief Создаёт документ: шаг 1; редактор, просмотр сигналов и поля — «winword.exe» (заменяются при `OnInitialUpdate` вида).
	CPlanRT_HDoc();
	DECLARE_DYNCREATE(CPlanRT_HDoc)

// Attributes
public:
	/// @brief Сообщает, идёт ли сейчас расчёт шага во времени.
	/// @return TRUE — расчётный объект находится в цикле по тактам (`IsRunStep()`).
	BOOL IsRun( void );
	/// @brief Считает все шаги задания от текущего до последнего (команда Run, F5).
	///
	/// Обёртка над `RunAll1()`: замеряет время всего прогона, перехватывает исключения
	/// (нехватка памяти и прочие) и превращает их в код возврата 2, при ключе /Time
	/// сохраняет отчёт о времени рядом с .tpl. В пакетном режиме (/Ar, /Arb) печатает
	/// итог в консоль и закрывает главное окно: при /Ar — только после успешного
	/// расчёта, при /Arb — всегда.
	/// @note Потоки: выполняется в расчётном потоке (`RunAllGlobal`).
	/// @see https://www.tamic.ru/docs/code/kernel-shell.html#run-all
	void RunAll( void );
	/// @brief Просит расчёт остановиться: сбрасывает флаг `bIsReadData` и флаг счёта расчётного объекта.
	///
	/// Не ждёт потока: расчёт выходит из цикла на ближайшей проверке флага.
	/// @note Потоки: вызывается из главного потока.
	void Stop( void );
	/// @brief Запоминает копию дескриптора расчётного потока для последующего ожидания в `StopAndWait`.
	/// @param[in] hThread дескриптор потока (`CWinThread::m_hThread`); NULL — только закрыть прежний.
	void SetRunThreadHandle( HANDLE hThread );
	/// @brief Останавливает расчёт и ждёт завершения расчётного потока, прокачивая события интерфейса.
	///
	/// Отмечает, что окно закрывается (после этого расчётный поток не шлёт главному
	/// окну просьбу закрыться), вызывает `Stop()` и ждёт поток порциями по 100 мс.
	/// Ждёт без ограничения по времени; каждые 30 с пишет в строку состояния, что
	/// ожидает поток. Документ не разрушается, пока его поток работает.
	/// @note Потоки: главный поток; вызывается из `MainWindow::closeEvent`.
	/// @see https://www.tamic.ru/docs/code/kernel-shell.html#stop-close
	void StopAndWait( void );
	/// @brief Считает один текущий шаг `nStep` (команда Run Step, F8).
	///
	/// При ошибке расчёта записывает нули S-матрицы этого шага в файл .s.
	/// @note Потоки: выполняется в расчётном потоке (`RunStepGlobal`).
	void RunStep( void );
	/// @brief Сообщает, занят ли документ чтением данных или счётом.
	/// @return TRUE — идёт чтение шага или расчёт (флаг документа или расчётного объекта).
	BOOL IsReadData( void );
	/// @brief Возвращает описание вывода текущего шага (имена файлов, флаги вывода).
	/// @return Указатель на объект расчётного объекта; не освобождать.
	CTmcRTH_IndanOutput* GetOutput( void );
	/// @brief Возвращает параметры текущего шага (сетка, частота, единицы).
	/// @return Ссылка на объект внутри расчётного объекта.
	CTmcRTH_IndanParam& GetParam( void );
	/// @brief Возвращает топологию текущего шага (список блоков).
	/// @return Ссылка на объект внутри расчётного объекта.
	CTmcRTH_IndanTopology& GetTopology( void );
	/// @brief Возвращает сообщение ядра о последней ошибке («Error:no» — ошибок нет).
	/// @return Копия строки сообщения.
	CString GetErrorMessage( void );
	/// @brief Переходит к предыдущему шагу (не меньше 1); данные шага не читает. Во время чтения не действует.
	void BackStep( void );
	/// @brief Переходит к следующему шагу, если он есть в задании; данные шага не читает.
	///
	/// Число шагов узнаёт препроцессором (`GetNStepMax`). Во время чтения не действует.
	void SkipStep( void );
	/// @brief Делает текущим первый шаг. Во время чтения не действует.
	void SetFirstStep( void  );
	/// @brief Возвращает номер текущего шага \#STEP.
	/// @return Номер шага, начиная с 1.
	int GetnStep( void );
	/// @brief Читает данные текущего шага задания (препроцессор, параметры, блоки) в расчётный объект.
	/// @note Потоки: главный поток. Повторный вход во время чтения игнорируется.
	void ReadData( void );
	/// @brief Запускает внешнюю программу с файлом задания в качестве довода.
	/// @param[in] csNameExe имя или полный путь программы (до `N_BUF_MAX` символов).
	/// @return TRUE — процесс создан; FALSE — программа не найдена или не запустилась.
	BOOL RunExeFile( CString csNameExe );
	/// @brief Задаёт внешний редактор задания (Config -> Editor).
	/// @param[in] csExternEditorName имя или полный путь программы.
	void SetExternEditorName( CString csExternEditorName );
	/// @brief Возвращает имя внешнего редактора задания.
	/// @return Имя или путь программы.
	CString GetExternEditorName( void );
	/// @brief Запускает внешнюю программу с путём задания (или его заголовком, если пути нет) в кавычках.
	/// @param[in] lpszNameExe имя или полный путь программы, строка с завершающим нулём.
	/// @return TRUE — процесс создан; FALSE — программа не найдена или не запустилась.
	BOOL RunExeFile( char *lpszNameExe );

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlanRT_HDoc)
	public:
	/// @brief Обработчик каркаса «новый документ»; дополнительной инициализации нет.
	/// @return TRUE — документ готов; FALSE — базовый класс отказал.
	virtual BOOL OnNewDocument();
	/// @brief Сериализация каркаса MFC; не используется (задание читается расчётным объектом).
	/// @param[in,out] ar архив каркаса.
	virtual void Serialize(CArchive& ar);
	/// @brief Закрытие документа: просит расчёт остановиться (`Stop`) и вызывает базовый класс.
	virtual void OnCloseDocument();
	/// @brief Разрешает или запрещает закрыть окно документа.
	/// @param[in] pFrame рамка окна (не используется).
	/// @return FALSE — идёт чтение данных или расчёт; иначе ответ базового класса (TRUE).
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	//}}AFX_VIRTUAL

// Implementation
public:
	/// @brief Записывает файл диаграммы направленности по накопленному полю (Config -> Directional Pattern).
	/// @see https://www.tamic.ru/docs/theory/radiation.html#pattern
	void ExportToDirectionalPatter( void );
	/// @brief Задаёт программу просмотра диаграммы направленности (TMC_DN).
	/// @param[in] csFileName имя или полный путь программы.
	void SetExternViewerDirectionalpattern( CString csFileName );
	/// @brief Запускает просмотр диаграммы направленности с файлом ДН текущего шага.
	///
	/// Если файла ещё нет, создаёт пустой, чтобы программе было что открыть.
	/// @return TRUE — процесс создан.
	BOOL RunViewerDirectPattern( void );
	/// @brief Возвращает расчётный объект документа.
	/// @return Ссылка на `cRectNode`; живёт, пока жив документ.
	CTmcRTHRectNode & GetRectNode( void );
	/// @brief Возвращает флаг вывода топологии (переключатель Topology).
	/// @return TRUE — топология выводится в файл.
	BOOL GetTopologyFlag( void );
	/// @brief Возвращает флаг звуковых сигналов об окончании шага и расчёта.
	/// @return TRUE — звук включён.
	BOOL GetSoundEffect( void );
	/// @brief Возвращает флаг вывода полей (переключатель Field).
	/// @return TRUE — поля пишутся в файл.
	BOOL GetFieldFlag( void );
	/// @brief Возвращает флаг синхронизации вывода поля с просмотрщиком (переключатель Synchronization).
	/// @return TRUE — синхронизация включена.
	BOOL GetSinchronizationFieldOutput( void );
	/// @brief Записывает файл топологии текущего шага (команда Topology).
	void PutTopology( void );
	/// @brief Задаёт флаг звуковых сигналов (из настроек).
	/// @param[in] i 0 — выключен, иное — включён.
	void SetSoundFlag( int i );
	/// @brief Переключает звуковые сигналы (Config -> Sound -> On/Off).
	void OnOffSoundEffects( void );
	/// @brief Задаёт флаг вывода полей (из настроек).
	/// @param[in] i 0 — выключен, иное — включён.
	void SetOutFieldOutFlag( int i );
	/// @brief Задаёт флаг синхронизации вывода поля (из настроек).
	/// @param[in] i 0 — выключен, иное — включён.
	void SetOutSinchrFieldOutFlag( int i );
	/// @brief Переключает синхронизацию вывода поля с просмотрщиком.
	void OnOffSinchronizationFieldOutput( void );
	/// @brief Запускает просмотрщик поля (FieldView) с файлом поля текущего шага; пустой файл создаёт при отсутствии.
	/// @return TRUE — процесс создан.
	BOOL RunViewerField( void );
	/// @brief Запускает просмотрщик S-матрицы (TMCGROUT) с файлом .s текущего шага; пустой файл создаёт при отсутствии.
	/// @return TRUE — процесс создан.
	BOOL RunViewerSmatrix( void );
	/// @brief Переключает вывод полей в файл (Config -> Field).
	void OnOffFieldOutput( void );
	/// @brief Возвращает программу просмотра сигналов на портах (TMCROS).
	/// @return Имя или путь программы.
	CString GetExternViewer( void );
	/// @brief Возвращает программу просмотра диаграммы направленности (TMC_DN).
	/// @return Имя или путь программы.
	CString GetExternViewerDirectPat( void );
	/// @brief Возвращает программу просмотра поля (FieldView).
	/// @return Имя или путь программы.
	CString GetExternViewerField( void );
	/// @brief Возвращает программу просмотра S-матрицы (TMCGROUT).
	/// @return Имя или путь программы.
	CString GetExternViewerSmatrix( void );
	/// @brief Запускает просмотрщик сигналов на портах (TMCROS) с файлом сигналов .t; пустой файл создаёт при отсутствии.
	/// @return TRUE — процесс создан.
	BOOL RunViewer( void );
	/// @brief Задаёт программу просмотра сигналов на портах.
	/// @param[in] csExternViewer1 имя или полный путь программы.
	void SetExternViewer( CString csExternViewer1 );
	/// @brief Задаёт программу просмотра поля.
	/// @param[in] csExternViewerField1 имя или полный путь программы.
	void SetExternViewerField( CString csExternViewerField1 );
	/// @brief Задаёт программу просмотра S-матрицы.
	/// @param[in] csExternViewerField1 имя или полный путь программы.
	void SetExternViewerSmatrix( CString csExternViewerField1 );
	/// @brief Запускает внешнюю программу с заданным доводом.
	/// @param[in] csNameExe     имя или полный путь программы (до `N_BUF_MAX` символов).
	/// @param[in] csArgCommLine довод — обычно путь к файлу; пустой — заголовок документа.
	/// @return TRUE — процесс создан.
	BOOL RunExeFile( CString csNameExe, CString csArgCommLine);
	/// @brief Запускает внешнюю программу: `"программа" "довод"`, рабочая папка — папка задания.
	/// @param[in] lpszNameExe     имя или полный путь программы.
	/// @param[in] lpszArgCommLine довод; пустая строка — заголовок документа.
	/// @return TRUE — процесс создан.
	/// @warning Длина командной строки не проверяется: имя и довод вместе должны
	///          помещаться в `N_BUF_MAX` символов.
	BOOL RunExeFile( char *lpszNameExe, char *lpszArgCommLine  );
	/// @brief Разрушает документ; закрывает сохранённый дескриптор расчётного потока.
	virtual ~CPlanRT_HDoc();
#ifdef _DEBUG
	/// @brief Проверка целостности объекта (отладочная сборка MFC).
	virtual void AssertValid() const;
	/// @brief Отладочный вывод объекта (отладочная сборка MFC).
	/// @param[in,out] dc контекст отладочного вывода.
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CPlanRT_HDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	/// Программа просмотра диаграммы направленности (TMC_DN): имя или путь.
	CString csExternViewerDirectPattern;
	/// Программа просмотра S-матрицы (TMCGROUT): имя или путь.
	CString csExternViewerSmatrix;
	/// Программа просмотра поля (FieldView): имя или путь.
	CString csExternViewerField;
	/// Программа просмотра сигналов на портах (TMCROS): имя или путь.
	CString csExternViewer;
	/// Признак «идёт чтение или счёт» и одновременно флаг продолжения расчёта:
	/// TRUE ставит начало операции, FALSE — её конец или `Stop()` из главного потока.
	volatile BOOL bIsReadData;
	/// Копия дескриптора расчётного потока (для `StopAndWait`); NULL — поток не запускался.
	HANDLE m_hRunThread;
	/// Номер текущего шага \#STEP задания, от 1.
	int nStep;
	/// Расчётный объект: параметры, топология, сетка и вывод текущего шага.
	CTmcRTHRectNode cRectNode;
	/// Внешний редактор задания: имя или путь программы.
	CString csEditorName;
	/// @brief Создаёт процесс по готовой командной строке, рабочая папка — папка задания.
	///
	/// В Windows программу, заданную без папки, ищет по полному пути (`TmcFindProgram`):
	/// рядом с ядром, в системных папках, в «App Paths» реестра, в PATH, но не в текущей
	/// папке. Дескрипторы не наследуются. В демонстрационном режиме (/d) сведения о
	/// процессе запоминаются, иначе дескрипторы сразу закрываются.
	/// @param[in] lpszComLine командная строка: `"программа" "довод"`.
	/// @return TRUE — процесс создан; FALSE — программа не найдена или не запустилась.
	BOOL CreatProc(char * lpszComLine);
	/// @brief Цикл по шагам задания без замера времени и перехвата исключений (тело `RunAll`).
	///
	/// Для каждого шага: читает данные, считает, при ошибке шага пишет нули S-матрицы
	/// в .s и продолжает со следующего шага; проверяет флаг остановки до и после
	/// каждого этапа. Ставит код возврата 0/1/2/3 и текст итога в строку состояния и
	/// в консоль.
	/// @see https://www.tamic.ru/docs/code/kernel-shell.html#run-all
	void RunAll1( void );	// TIMING: RunAll without time measurement wrapper
};



#endif
/////////////////////////////////////////////////////////////////////////////
