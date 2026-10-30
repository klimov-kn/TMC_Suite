// TMCGROUTDoc.h : interface of the CTMCGROUTDoc class
//
/////////////////////////////////////////////////////////////////////////////
/// @file TMCGROUTDoc.h
/// @brief Документ TMCGROUT: файл документа `.soc` со списком графиков, чтение данных из файлов `.s`.
///
/// Часть модуля «Вьювер TMCGROUT». Копии класса с отличиями есть у TMCROS
/// (`viewers/tmcrtout`) и TMC_DN (`viewers/dianapgr`).
/// @see https://www.tamic.ru/docs/code/tmcgrout.html#document
#include <Tmcgrviw.h>
#include <Proc_s.h>

/// @brief Документ вьювера: список графиков S-параметров, параметры окна графика, цвета и шрифт.
///
/// Один объект — одно окно документа. В Qt6-версии создаётся главным окном
/// (`CTMCGROUTDoc::TmcCreate` в `MainWindow::createDocumentWindow`) и удаляется им же.
/// Путь документа (`GetPathName`) может указывать на файл документа `.soc` (первая
/// строка `#TMCGROUT`) или на файл данных `.s`: тогда рядом создаётся документ `.soc`
/// (или `.$OC`, если `.soc` уже есть) с одним графиком.
/// Жизненный цикл чтения: `ReadGraphParametersDefault` (настройки программы) ->
/// `ReadGraphParameters` (строки `#...` файла документа) -> `ReadDocFile` (список
/// графиков и их данные). Запись — `WriteDocFile`. Ошибки не выдаются окном: текст
/// ошибки документа лежит в `error`, ошибки отдельных графиков — в их подписях.
/// @note Потоки: только главный поток.
/// @see https://www.tamic.ru/docs/code/tmcgrout.html#document
class CTMCGROUTDoc : public CDocument
{
protected: // create from serialization only
	/// @brief Задаёт параметры по умолчанию: КСВ по частоте в ГГц, автодиапазон, белый фон, шрифт Times New Roman, редактор winword.exe, 16 стилей линий.
	CTMCGROUTDoc();
	DECLARE_DYNCREATE(CTMCGROUTDoc)

// Attributes
public:
	/// TRUE — прореживать маркеры точек (около 20 маркеров на график); строка `#PointDrawFlag`, второе число.
	BOOL bLossPoint;
	/// @brief Читает параметры по умолчанию из настроек программы (раздел и ключ `#TMCGROTS`).
	///
	/// Строка настроек — те же строки `#...`, что в файле документа, но разделённые байтом
	/// 0x01 вместо перевода строки; каждая разбирается `SetOneReadGraphParameters`.
	/// Вызывается до `ReadGraphParameters`, чтобы файл документа мог переопределить значения.
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#settings
	void ReadGraphParametersDefault( void );
	/// @brief Присваивает всем графикам документа общий цвет маркеров точек `scPointColor`.
	void SetGraphColor( void );
	/// Множитель единиц пользователя оси X: значение оси = частота (Гц) * `dXAxiesUserUnit`; строка `#dXAxiesUnit`.
	double  dXAxiesUserUnit;
	/// Подпись оси X в единицах пользователя; строка `#dXAxiesName`.
	CString csXAxiesUserName;
	/// Цвет фона, `COLORREF`; строка `#sDrawBckgrdColor`.
	COLORREF scBackgoundColor;
	/// Цвет сетки; строка `#sDrawGridColor`.
	COLORREF scGridColor;
	/// Цвет осей; строка `#sDrawAxiesColor`.
	COLORREF scAxiesColor;
	/// Цвет текста (подписи, легенда); строка `#sDrawTextColor`.
	COLORREF scTextColor;
	/// Цвет маркеров точек всех графиков; строка `#sDrawPointColor`, первое число.
	COLORREF scPointColor;
	/// Шрифт подписей; высота `lfHeight` — в точках экрана, при выводе умножается на 20
	/// (вид работает в единицах 0.01 мм); строка `#sDrawTextFont`.
	LOGFONT lfInitial;
	/// Внешний редактор файла документа (команда Edit > Editor); строка `#EditorFileName`.
	CString csEditorName;
	/// @brief Дописывает в конец файла документа график для файла `.s` (вход и мода 1/1 -> 1/1).
	///
	/// Путь переводится в относительный от папки документа (`MakeRelatPathSFile`);
	/// подпись — имя файла без папки. Данные не перечитываются: это делает вызывающий.
	/// @param[in,out] szSFileName полный путь к файлу; на выходе — относительный путь
	void AddGraphicsInDoc( char *szSFileName );
	/// @brief Проверяет, изменились ли файл документа или файлы `.s` показываемых графиков, и запоминает новые времена записи.
	/// @return TRUE — хотя бы один файл изменился с прошлой проверки (данные нужно перечитать).
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#auto-refresh
	BOOL IsDataModific( void );
	/// @brief Читает строки `#...` файла документа (параметры осей, цвета, шрифт, стили линий).
	///
	/// Каждая строка разбирается `SetOneReadGraphParameters`; затем всем графикам
	/// присваивается цвет точек. Файл не открылся — параметры не меняются.
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#doc-file
	void ReadGraphParameters( void );
	/// @brief Записывает файл документа и строку параметров по умолчанию в настройки программы.
	///
	/// Пишет метку `#TMCGROUT`, строки параметров, затем строки графиков
	/// `имя подпись показ вход1 мода1 вход2 мода2`. Пустая подпись или подпись `Error_...`
	/// заменяется именем; из имени и подписи удаляются пробелы.
	/// @return FALSE — успех; TRUE — ошибка (текст в `error`).
	/// @warning Код возврата обратный обычному: TRUE означает ошибку.
	BOOL WriteDocFile();
	/// Текст ошибки документа; пустая строка — ошибки нет. Непустой — вид вместо графиков выводит `Error : <текст>`.
	char error[300];
	/// @brief Читает список графиков из файла документа и данные каждого графика.
	///
	/// Готовит имя документа (`MakeDocFileName`), освобождает прежние данные, считает строки
	/// графиков, выделяет массив (`DocFileNewData`) и читает каждый график (`ReadGraph`).
	/// Запоминает времена записи файлов (`IsDataModific`). Ошибка — в `error`.
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#read-s
	void ReadDocFile(void);
	/// Список графиков и параметры окна графика (оси, диапазоны, признаки).
	TMC_GR_DOC grdoc;
	/// @brief Открывает файл документа во внешней программе (редакторе).
	///
	/// Командная строка: имя программы (в кавычках, если это существующий путь) и путь
	/// документа в кавычках; рабочая папка — папка документа.
	/// @param[in] lpszNameExe имя или путь программы; начальные пробелы пропускаются
	void RunExeFile( char *lpszNameExe );
	/// @brief Выделяет массив из `n` графиков и задаёт им стиль по умолчанию (стиль i % 16 из `sGrDoc`).
	/// @param[in] n число графиков, больше 0
	/// @return TRUE — успех; FALSE — нет памяти (текст в `error`).
	BOOL DocFileNewData(int n);
	/// @brief Освобождает массив графиков и все их данные (частоты, S, имена, выражения); очищает `error`.
	void DocFileDelData();
	/// @brief Пересчитывает относительные пути графиков после «Save As» в новую папку документа.
	///
	/// Каждое имя без `:` и без начального `\\` считается путём относительно старой папки,
	/// дополняется до полного и снова переводится в относительный — уже от нового пути
	/// документа. Выражения (`{...}`) не меняются.
	/// @param[in] szOldDocPath прежний полный путь файла документа
	void RebaseGraphPaths( const char *szOldDocPath );	// S-44: imena grafikov - otnositelno novoi papki

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTMCGROUTDoc)
	public:
	/// @brief Подготовка нового документа каркасом MFC; дополнительных действий нет.
	/// @return TRUE — успех.
	virtual BOOL OnNewDocument();
	/// @brief Сериализация каркаса MFC; не используется (документ пишет файл сам).
	/// @param[in,out] ar архив
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	/// Стили 16 линий по умолчанию (цвет, тип, толщина, форма маркера); строка
	/// `#sDrawPointColor`; меняются в окне Config > Color > Graphics.
	TMC_GR_DOC1 sGrDoc[16];
	/// @brief Освобождает данные графиков.
	virtual ~CTMCGROUTDoc();
#ifdef _DEBUG
	/// @brief Проверка объекта (отладочная сборка MFC).
	virtual void AssertValid() const;
	/// @brief Отладочный вывод (отладочная сборка MFC).
	/// @param[in,out] dc контекст вывода
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTMCGROUTDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	/// @brief Заполняет `sGrDoc` стилями по умолчанию: 16 цветов, сплошная линия толщиной 0.5 мм, свой маркер у каждого стиля.
	void SetDefaultLineColorWidthType( void );
	/// @brief Разбирает одну строку параметров (`#метка значение`) и записывает значение в поле документа.
	///
	/// Неизвестные метки и строки без `#` пропускаются. Для `#sDrawPointColor` ожидается
	/// 49 чисел (цвет точек и 16 троек «цвет; тип; толщина»); если их меньше, берётся только
	/// цвет точек. Строка шрифта без `;` пропускается.
	/// @param[in,out] ch строка; сжимается `del_bl2` и временно меняется при разборе шрифта
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#doc-file
	void SetOneReadGraphParameters(char *ch);
	/// @brief Объявлен, но не определён и не вызывается (подсчёт файлов выражения выполняет `CTMCGrExpression`).
	/// @param[in] szExpression выражение
	/// @return Число файлов в выражении.
	int CalcNGraphExpession( char* szExpression );
	/// @brief Читает кривую-выражение: создаёт `CTMCGrExpression`, читает все её файлы, копирует частоты.
	///
	/// При ошибке подпись графика получает текст `Error_expression_...` или
	/// `Error_when_read_file_...`, число точек остаётся 0.
	/// @param[in] nGr          номер графика в `grdoc.psGraph`
	/// @param[in] szExpression строка выражения вместе с `{` и `}`
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#expressions
	void ReadGraph1Expession( int nGr, char *szExpression );
	/// @brief Читает частотную зависимость одного элемента S из файла через `read_S_matrix_element`.
	///
	/// Имя без `:` и без начального `\\` считается путём относительно папки документа.
	/// Префикс `*вход1;мода1;вход2;мода2*` в имени задаёт элемент S; без него берутся номера
	/// из строки графика. После чтения номера входов и мод графика заменяются фактическими.
	/// @param[in]  nGr        номер графика (в него пишутся полный путь и номера входов)
	/// @param[in]  szFileName имя файла из документа, возможно с префиксом элемента
	/// @param[out] nPoint     число прочитанных частот; 0 или меньше — ошибка (подпись `Error_when_read_...`)
	/// @param[out] pFreq      массив частот, Гц; выделяет sfile95
	/// @param[out] pSmatr     массив комплексных значений элемента S; выделяет sfile95
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#read-s
	void ReadGraph1( int nGr, char *szFileName, int *nPoint, double **pFreq, _complex **pSmatr );
	/// @brief Возвращает число папок в пути (число элементов, оканчивающихся разделителем).
	/// @param[in] ch путь
	/// @return Число папок.
	int CalcNDirInPathName( char  *ch);
	/// @brief Возвращает длину первого элемента пути вместе с разделителем `\` или `/`.
	/// @param[in] ch путь (или его остаток)
	/// @return Число символов до разделителя включительно; 0 — разделителя нет.
	int SelectDirName( char *ch );
	/// @brief Переводит полный путь файла в путь относительно папки документа (с `..` при необходимости).
	///
	/// Сравнивает папки пути документа и файла (на Windows без учёта регистра, `\` и `/`
	/// равны). Если различается уже первый элемент пути (другой диск), путь не меняется.
	/// @param[in,out] szSFileName полный путь; на выходе — относительный
	void MakeRelatPathSFile( char *szSFileName );
	/// @brief Возвращает время последней записи файла.
	/// @param[in] szFileName путь к файлу
	/// @return Время записи; нули — файл не открылся.
	FILETIME LastWriteTime( char *szFileName );
	/// @brief Заменяет расширение имени файла на `.soc`; если такой файл уже есть — на `.$OC`.
	/// @param[in,out] lpszDatFN путь к файлу данных; буфер должен вмещать на 4 символа больше
	void SFileNameToDatFileName( char *lpszDatFN);
	/// @brief Проверяет, является ли файл документом TMCGROUT (первая строка начинается с `#TMCGROUT`).
	/// @param[in] ch путь к файлу
	/// @return TRUE — файл документа; FALSE — другой файл, пустой путь или файл не открылся.
	BOOL IsDocFileName( char *ch);
	/// @brief Готовит файл документа перед чтением.
	///
	/// Пустой путь (новый документ) — путь = заголовок окна, создаётся файл с одной меткой.
	/// Путь на несуществующий файл — ошибка `file not found`. Путь на файл, который не
	/// документ (например `.s`), — путь меняется на `.soc`/`.$OC` и создаётся документ с
	/// одним графиком для этого файла. Документ `.soc` не трогается.
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#document
	void MakeDocFileName( void );
	/// @brief Удаляет из строки все пробелы, табуляции и переводы строки.
	/// @param[in,out] ch строка
	void del_bl3( char *ch );
	/// @brief Читает строку графика номер `nGr` из файла документа и его данные.
	///
	/// Строка — 7, 6, 5 или 1 поле: `имя подпись показ вход1 мода1 вход2 мода2`; без подписи
	/// подпись = имя, без номеров — 1, без признака показа — показывать. Имя в `{}` —
	/// кривая-выражение (`ReadGraph1Expession`), иначе файл (`ReadGraph1`).
	/// @param[in] nGr номер графика, 0..`grdoc.nGraph`-1
	void ReadGraph( int nGr );
	/// @brief Сжимает пробельные символы вне квадратных скобок до одного пробела и отрезает комментарий.
	///
	/// Комментарий — `!` в начале строки или после пробела вне скобок; `!` внутри имени
	/// остаётся. Внутри `[...]` пробелы сохраняются. Конечный пробел удаляется.
	/// @param[in,out] ch строка
	void del_bl2( char *);
	/// @brief Считает строки графиков в файле документа (непустые строки не с `#` после удаления комментариев).
	/// @return Число графиков; -1 — путь пуст или файл не открылся.
	int ReadNGraph( void );
	/// @brief Записывает текст ошибки документа в `error` (с обрезкой до размера буфера).
	/// @param[in] error текст ошибки
	void put_error_messege(const char *error);
	/// Сведения о последнем запущенном внешнем процессе; описатели сразу закрываются.
	PROCESS_INFORMATION piProcInfo;
	/// @brief Запускает процесс по командной строке, рабочая папка — папка документа; не ждёт завершения.
	/// @param[in] lpszNameExe полная командная строка
	void CreatProc(char *lpszNameExe);
	/// @brief Отделяет от имени файла префикс `*вход1;мода1;вход2;мода2*`.
	/// @param[out] csFileName имя файла без префикса
	/// @param[in]  szFileName имя из документа; временно меняется при разборе
	/// @param[out] inp1 номер входа 1; -1 — префикса нет или он неверен
	/// @param[out] mod1 номер моды входа 1; -1 — нет
	/// @param[out] inp2 номер входа 2; -1 — нет
	/// @param[out] mod2 номер моды входа 2; -1 — нет
	void ReadInputModeFromFileName(CString &csFileName, char *szFileName, int & inp1, int & mod1, int & inp2, int & mod2);
};

/////////////////////////////////////////////////////////////////////////////
