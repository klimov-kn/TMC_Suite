// TMCGROUTDoc.h : interface of the CTMCGROUTDoc class
//
/////////////////////////////////////////////////////////////////////////////
/// @file TMCGROUTDoc.h
/// @brief Документ TMC_DN: файл документа `.dop` со списком ДН, расчёт ДН каждой решётки из файлов `.DAT`.
///
/// Часть модуля «Вьювер TMC_DN». Копия документа TMCGROUT с отличиями: строка графика
/// ссылается на файл излучателей `.DAT`, «данные графика» — ДН, рассчитанная объектом
/// `c2DArray`; ось X — угол.
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#document
#include <Tmcgrviw.h>
#include <Proc_s.h>

/// @brief Один график документа TMC_DN: файл решётки `.DAT`, рассчитанная ДН, номера мод и стиль линии.
///
/// Копия `TMC_GR_DOC1` (общий `tmcgrviw.h`) с полями `bIsReduceElMagnAnalysis` и `dFreq`.
/// Имена полей унаследованы от графика S-параметров: «частоты» здесь — углы, «элемент S» — ДН.
/// Элементы массива `grdoc.psGraph` создаёт `DocFileNewData`, освобождает `DocFileDelData`.
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#doc-file
typedef struct
	{
	   /// Не используется (признак секторов хранится в решётке `c2DArray::bIsReduceElMagnAnalysis`).
	   BOOL		 bIsReduceElMagnAnalysis; //  
	   /// Число точек ДН (точек сетки углов); 0 — ДН не посчитана (ошибка).
	   int		 nPoint;	// number of point
	   /// Не заполняется при расчёте ДН (наследие TMCROS; читается только окном TtoS, недоступным из меню).
	   double	 dFreq;		// frequence for time array
	   /// Углы точек ДН, радианы; `nPoint` значений; владеет документ.
	   double	*pFreq;		// frequence array
	   /// ДН в точках: `.x` — нормированная амплитуда |F|/`dMax`, `.y` не задаётся; владеет документ.
	   _complex *pSmatr;	// point for complex elem Smatrix 
	   /// Кривая-выражение, если имя графика в `{}`; иначе NULL; владеет документ.
	   CTMCGrExpression *pcExpr;// pointer of expression class 
	   /// Поле «вход 1» строки графика; при чтении приводится к 1.
	   int		 nInp1;		// numer for input 1
	   /// Поле «мода 1»: 1 — учитывать секторы излучателей (Reduce), 0 — нет; больше 1 приводится к 1.
	   int		 nMod1;		// moda for input 1
	   /// Поле «вход 2»; при чтении приводится к 1.
	   int		 nInp2;		// numer for input 2
	   /// Поле «мода 2»; при чтении приводится к 0..1; на расчёт не влияет.
	   int		 nMod2;		// moda for input 2
	   /// Полный путь файла `.DAT`; буфер `TMC_GROUT_MAXSTRING_BUF` байт.
	   char		*szFileName;// s matrix FileName with all path
	   /// Имя файла `.DAT` (относительно папки документа) или выражение `{...}` из строки документа.
	   char		*szGrapName;// graphics	S FileName
	   /// Подпись ДН в легенде; при ошибке — `Error_...` с причиной.
	   char		*szGrapPodp;// graphics	Podp
	   /// Цвет линии, `COLORREF`.
	   int		 LineColor;	// line color	 how draw line
	   /// Тип линии — стиль пера GDI (`PS_SOLID` …).
	   int		 LineType;	// line type
	   /// Толщина линии, 0.01 мм.
	   int		 LineWidth;	// line width
       /// Форма маркера: вершины ломаной, 0.01 мм относительно точки.
       TMC_INT_POINT   piPoint[TMC_GROUT_POINT_TYPE];// how draw point
	   /// Цвет маркера, `COLORREF`.
	   int		 PointColor;// point color
	   /// Тип линии маркера — стиль пера GDI.
	   int		 PointType;	// point type
	   /// Толщина линии маркера, 0.01 мм.
	   int		 PointWidth;// point width
	   /// TRUE — ДН рисуется; FALSE — скрыта (и не проверяется на изменение файла).
	   int		 OutFlag;	// Flag for output or not graphics on view
	   /// Время последней записи файла `.DAT` при прошлой проверке (для автообновления).
	   FILETIME	 LastWriteTime;	// last time when S-matrix file was written
//	   c2DArray  cDirectionalPattern;
	} TMC_GR_DOC1_DN;

/// @brief Список ДН документа TMC_DN и параметры окна графика (копия `TMC_GR_DOC` с графиками `TMC_GR_DOC1_DN`).
///
/// Поле `CTMCGROUTDoc::grdoc`; записывается в файл документа `.dop` и читается из него.
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#doc-file
typedef struct
	{
		/// Число графиков в `psGraph`.
		int			 nGraph; // number of grahics in this document
		/// Массив графиков; владеет документ.
		TMC_GR_DOC1_DN *psGraph;// pointer for grahics array		  
		/// TRUE — автодиапазон оси X.
		BOOL		 nAFlagX;	//if nAFlagX = 0  - X; if = 1 - X1  
		/// TRUE — автодиапазон оси Y.
		BOOL		 nAFlagY;	//if nAFlagY = 0  - Y; if = 1 - Y1  
		/// TRUE — рисовать маркеры точек.
		BOOL 		 PointDrawFlag;	//draw or not point in graphics
		/// Величина по оси Y: `TMC_GROTS_TYPE_FULL` (0) ДН, `_AMPL` (1) ДН в дБ, `_PHAS_R` (2) нормированная ДН, `_PHAS_G` (3) нормированная ДН в дБ.
		int			 nYType;		//graphics type	Y-coord
		/// Ось X: `TMC_GROTS_TYPE_nT` (0) — угол в радианах, `TMC_GROTS_TYPE_ps` (1) — в градусах.
		int			 nXType;		//graphics type	X-coord
		/// Левая граница оси X, единицы оси (радианы или градусы); задаёт и диапазон расчёта ДН малой решётки.
		double		 Xmin;			//
		/// Правая граница оси X, единицы оси.
		double		 Xmax;			//
		/// Нижняя граница оси Y, единицы оси.
		double		 Ymin;			//
		/// Верхняя граница оси Y, единицы оси.
		double		 Ymax;			//
		/// Время последней записи файла документа при прошлой проверке.
		FILETIME	 LastWriteTime;	// last time when document file was written
	} TMC_GR_DOC_DN;



/// @brief Документ TMC_DN: список ДН (решёток), их рассчитанные ДН, параметры окна графика, цвета и шрифт.
///
/// Один объект — одно окно документа. В Qt6-версии создаётся главным окном
/// (`CTMCGROUTDoc::TmcCreate` в `MainWindow::openDocument`) и удаляется им же.
/// Путь документа (`GetPathName`) может указывать на файл документа `.dop` (первая
/// строка `#TMCGROTS`) или на файл излучателей `.DAT`: тогда рядом создаётся документ
/// `.dop` (или `.$op`, если `.dop` уже есть) с одной ДН.
/// Жизненный цикл чтения: `ReadGraphParametersDefault` (настройки программы) ->
/// `ReadGraphParameters` (строки `#...` файла документа) -> `ReadDocFile` (список
/// графиков и расчёт ДН каждого). Для каждого графика создаётся решётка
/// `pcDirectionalPattern[i]`; её ДН на сетке углов копируется в `pFreq` (углы) и
/// `pSmatr[].x` (нормированная амплитуда). Запись — `WriteDocFile`. Ошибки не выдаются
/// окном: текст ошибки документа — в `error`, ошибки отдельных ДН — в их подписях.
/// @note Потоки: `ReadDocFile` выполняется в потоке расчёта (`ReadDocFileW_Thread1`), прочие
/// методы — в главном потоке; вид не трогает документ, пока идёт расчёт (`g_lTmcDnCalcRunning`).
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#document
class CTMCGROUTDoc : public CDocument
{
protected: // create from serialization only
	/// @brief Задаёт параметры по умолчанию: ДН (линейно) по углу в радианах, без маркеров, автодиапазон, частота 10 ГГц, белый фон, шрифт Times New Roman, редактор winword.exe, 16 стилей линий.
	CTMCGROUTDoc();
	DECLARE_DYNCREATE(CTMCGROUTDoc)

// Attributes
public:
	/// Решётки излучателей, по одной на график (`grdoc.nGraph` штук); создаёт `DocFileNewData`,
	/// удаляет `DocFileDelData`; NULL — графиков нет.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#array-model
	c2DArray * pcDirectionalPattern;
	/// TRUE — прореживать маркеры точек (около 20 маркеров на график); строка `#PointDrawFlag`, второе число.
	BOOL bLossPoint;
	/// @brief Присваивает всем графикам документа общий цвет маркеров точек `scPointColor`.
	void SetGraphColor( void );
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
	/// @brief Дописывает в конец файла документа ДН для файла `.DAT` (строка `имя имя 1 1 1 1 1`, секторы учитываются).
	///
	/// Путь переводится в относительный от папки документа (`MakeRelatPathSFile`);
	/// подпись — имя файла без папки. Данные не перечитываются: это делает вызывающий.
	/// @param[in,out] szSFileName полный путь к файлу; на выходе — относительный путь
	void AddGraphicsInDoc( char *szSFileName );
	/// @brief Проверяет, изменились ли файл документа или файлы `.DAT` показываемых графиков, и запоминает новые времена записи.
	/// @return TRUE — хотя бы один файл изменился с прошлой проверки (данные нужно перечитать).
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#auto-refresh
	BOOL IsDataModific( void );
	/// @brief Читает строки `#...` файла документа (параметры осей, цвета, шрифт, стили линий).
	///
	/// Каждая строка разбирается `SetOneReadGraphParameters`; затем всем графикам
	/// присваивается цвет точек. Файл не открылся — параметры не меняются.
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#doc-file
	void ReadGraphParameters( void );
	/// @brief Записывает файл документа и строку параметров по умолчанию в настройки программы.
	///
	/// Пишет метку `#TMCGROTS`, строки параметров (без единиц пользователя оси X), затем строки
	/// графиков `имя подпись показ вход1 мода1 вход2 мода2` (мода1: 1 — секторы учитываются,
	/// 0 — нет). Пустая подпись или подпись `Error_...` заменяется именем; из имени и подписи
	/// удаляются пробелы.
	/// @return FALSE — успех; TRUE — ошибка (текст в `error`).
	/// @warning Код возврата обратный обычному: TRUE означает ошибку.
	BOOL WriteDocFile();
	/// Текст ошибки документа; пустая строка — ошибки нет. Непустой — вид вместо графиков выводит `Error : <текст>`.
	char error[300];
	/// @brief Читает список графиков из файла документа и считает ДН каждого.
	///
	/// Готовит имя документа (`MakeDocFileName`), освобождает прежние данные, считает строки
	/// графиков, выделяет массивы графиков и решёток (`DocFileNewData`) и читает каждый график
	/// (`ReadGraph` -> `ReadGraph1` -> `read_RT_output_signal`, полный расчёт ДН). Запоминает
	/// времена записи файлов (`IsDataModific`). Ошибка — в `error`.
	/// @note Потоки: в TMC_DN вызывается из потока расчёта `ReadDocFileW_Thread1`.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#calc-flow
	void ReadDocFile(void);
	/// Список ДН и параметры окна графика (строки файла документа).
	TMC_GR_DOC_DN grdoc;
	/// @brief Открывает файл документа во внешней программе (редакторе).
	///
	/// Командная строка: имя программы (в кавычках, если это существующий путь) и путь
	/// документа в кавычках; рабочая папка — папка документа.
	/// @param[in] lpszNameExe имя или путь программы; начальные пробелы пропускаются
	void RunExeFile( char *lpszNameExe );
	/// @brief Выделяет массив из `n` графиков и `n` решёток `c2DArray`, задаёт графикам стиль по умолчанию (стиль i % 16 из `sGrDoc`).
	/// @param[in] n число графиков, больше 0
	/// @return TRUE — успех; FALSE — нет памяти (текст в `error`).
	BOOL DocFileNewData(int n);
	/// @brief Освобождает массив графиков и все их данные (углы, ДН, имена, выражения) и массив решёток; очищает `error`, `nGraphCurrent` = 0.
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
	/// Номер графика, ДН которого считается сейчас (для окна прогресса); 0 — после очистки.
	int nGraphCurrent;
	/// @brief Пересчитывает ДН всех графиков на новый диапазон оси X без перечитывания файлов (`ReadGraphCangeAngle`).
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#angle-grid
	void ReadDocFile_1( void );
	/// Стили 16 линий по умолчанию (цвет, тип, толщина, форма маркера); строка
	/// `#sDrawPointColor`; меняются в окне Config > Color > Graphics.
	TMC_GR_DOC1 sGrDoc[16];
	/// Частота ДН для строки состояния, Гц: частота первой решётки документа (по умолчанию 10 ГГц).
	double dFreq;
	/// @brief Читает параметры по умолчанию из настроек программы (раздел и ключ `#TMCGROTS`).
	///
	/// Строка настроек — те же строки `#...`, что в файле документа, но разделённые байтом
	/// 0x01 вместо перевода строки; каждая разбирается `SetOneReadGraphParameters`.
	/// Вызывается до `ReadGraphParameters`, чтобы файл документа мог переопределить значения.
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#settings
	void ReadGraphParametersDefault( void );
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
	/// @brief Пересчитывает ДН графика `nGr` на текущий диапазон оси X (`ReadGraph1_CangeAngle`).
	/// @param[in] nGr номер графика
	void ReadGraphCangeAngle( int nGr );
	/// @brief Заполняет `sGrDoc` стилями по умолчанию: 16 цветов, сплошная линия толщиной 0.5 мм, свой маркер у каждого стиля.
	void SetDefaultLineColorWidthType( void );
	/// @brief Разбирает одну строку параметров (`#метка значение`) и записывает значение в поля документа.
	///
	/// Комментарий и строки без `#` игнорируются. Метки — как у TMCGROUT, кроме единиц
	/// пользователя оси X (`#dXAxiesUnit`, `#dXAxiesName` в TMC_DN не читаются).
	/// @param[in,out] ch строка; очищается `del_bl2`
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#doc-file
	void SetOneReadGraphParameters( char *ch );
	/// @brief Возвращает длину первого слова строки (до пробела, табуляции или перевода строки); наследие TMCROS, в TMC_DN не вызывается.
	/// @param[in] ch строка
	/// @return Длина слова; 0 — разделителя нет до конца строки.
	int skip_number( char *ch );
	/// @brief Возвращает число начальных пробельных символов строки; наследие TMCROS.
	/// @param[in] ch строка
	/// @return Число пробельных символов; 0 — строка из одних пробелов.
	int skip_blank( char *ch );
	/// @brief Возвращает смещение колонки входа `in1` в строке сигналов `.t`; наследие TMCROS, в TMC_DN не вызывается.
	/// @param[in] ch  строка данных
	/// @param[in] in1 номер входа, с 1
	/// @return Смещение в символах.
	int skip_ntdt_itd( char *ch, int in1 );
	/// @brief Возвращает значение сигнала входа `in1` (мода 0 — первое, 1 — второе число) из строки `.t`; наследие TMCROS, в TMC_DN не вызывается.
	/// @param[in] ch   строка данных
	/// @param[in] in1  номер входа, с 1
	/// @param[in] mod1 0 или 1 — какое из двух чисел колонки
	/// @return Значение; 0 — нет колонки.
	double gets_sss( char *ch, int in1, int mod1);
	/// @brief Возвращает второе число строки (время отсчёта `.t`); наследие TMCROS, в TMC_DN не вызывается.
	/// @param[in] ch строка данных
	/// @return Значение.
	double gets_f1( char *ch );
	/// @brief Полностью считает ДН решётки графика `n_Gr1` из файла `.DAT` и выдаёт её на текущем диапазоне оси X.
	///
	/// Нормирует номера: `in1`, `in2` -> 1, `mod1`, `mod2` -> 0..1; `mod1` = 0 выключает секторы
	/// (`bIsReduceElMagnAnalysis`). Затем `c2DArray::CalculateDiagrNapr(файл, файл~~tempDia)`,
	/// перестройка сетки на диапазон оси X (`ResizeAngle`, градусы переводятся в радианы),
	/// выделение массивов и копирование углов и нормированной амплитуды. Частота первой
	/// решётки пишется в `dFreq` документа.
	/// @param[in]     n_Gr1    номер графика и решётки
	/// @param[in,out] in1      номер входа 1 (приводится к 1)
	/// @param[in,out] mod1     мода 1: 0 — без секторов, иначе 1
	/// @param[in,out] in2      номер входа 2 (приводится к 1)
	/// @param[in,out] mod2     мода 2 (приводится к 0..1)
	/// @param[in]     filename полный путь файла `.DAT`
	/// @param[in,out] sss      массив ДН (прежний освобождается, новый выделяется): `.x` — нормированная амплитуда
	/// @param[in,out] f1       массив углов, радианы (прежний освобождается, новый выделяется)
	/// @param[out]    nPoint   число точек; 0 — ошибка (текст — у решётки)
	/// @param[out]    dFr      не заполняется
	/// @warning Мнимая часть `(*sss)[i].y` не задаётся.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#calc-flow
	void read_RT_output_signal( int n_Gr1, int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint, double *dFr);
	/// @brief Как `read_RT_output_signal`, но без чтения файла: только `ResizeAngle` и копирование в уже выделенные массивы.
	///
	/// Массивы `*sss`, `*f1` должны существовать и вмещать `GetnAngle()` точек (длина сетки не
	/// меняется). У большой решётки `ResizeAngle` ничего не делает — копируется прежняя ДН.
	/// @param[in]     n_Gr1    номер графика и решётки
	/// @param[in,out] in1      номер входа 1 (приводится к 1)
	/// @param[in,out] mod1     мода 1: 0 — без секторов, иначе 1
	/// @param[in,out] in2      номер входа 2 (приводится к 1)
	/// @param[in,out] mod2     мода 2 (приводится к 0..1)
	/// @param[in]     filename не используется
	/// @param[in,out] sss      массив ДН графика; NULL — ничего не делается
	/// @param[in,out] f1       массив углов графика, радианы; NULL — ничего не делается
	/// @param[out]    nPoint   число точек
	/// @param[out]    dFr      не заполняется
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#angle-grid
	void read_RT_output_signal_CangeAngle( int n_Gr1, int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint, double *dFr);
	/// @brief Объявлен, но не определён и не вызывается (подсчёт файлов выражения выполняет `CTMCGrExpression`).
	/// @param[in] szExpression выражение
	/// @return Число файлов в выражении.
	int CalcNGraphExpession( char* szExpression );
	/// @brief Читает кривую-выражение: создаёт `CTMCGrExpression`, считает ДН всех её файлов `.DAT` (`ReadGraph1`), копирует углы.
	///
	/// При ошибке подпись графика получает текст `Error_expression_...` или
	/// `Error_when_read_file_...`, число точек остаётся 0.
	/// @param[in] nGr          номер графика в `grdoc.psGraph`
	/// @param[in] szExpression строка выражения вместе с `{` и `}`
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#expressions
	void ReadGraph1Expession( int nGr, char *szExpression );
	/// @brief Считает ДН решётки из файла `.DAT` графика `nGr` (`read_RT_output_signal`) и кладёт её в массивы графика.
	///
	/// Имя без `:` и без начального `\\` считается путём относительно папки документа; полный
	/// путь пишется в `szFileName` графика. Ставит `nGraphCurrent` = `nGr` (для окна прогресса).
	/// При ошибке подпись графика — `Error_when_read_<имя>:_<текст ошибки решётки>`.
	/// @param[in]  nGr        номер графика и решётки
	/// @param[in]  szFileName имя файла `.DAT` из документа
	/// @param[out] nPoint     число точек ДН; 0 — ошибка
	/// @param[out] pFreq      массив углов, радианы; выделяется здесь
	/// @param[out] pSmatr     массив, в `.x` — нормированная амплитуда ДН; выделяется здесь
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#calc-flow
	void ReadGraph1( int nGr, char *szFileName, int *nPoint, double **pFreq, _complex **pSmatr );
	/// @brief Пересчитывает ДН графика `nGr` на новый диапазон оси X (`read_RT_output_signal_CangeAngle`); при ошибке — подпись `Error_when_read_...`.
	/// @param[in]     nGr        номер графика
	/// @param[in]     szFileName имя файла для подписи ошибки
	/// @param[out]    nPoint     число точек
	/// @param[in,out] pFreq      массив углов графика, радианы
	/// @param[in,out] pSmatr     массив ДН графика
	void ReadGraph1_CangeAngle( int nGr, char *szFileName, int *nPoint, double **pFreq, _complex **pSmatr );
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
	/// @brief Заменяет расширение имени файла на `.dop`; если такой файл уже есть — на `.$op`.
	/// @param[in,out] lpszDatFN путь к файлу данных; буфер должен вмещать на 4 символа больше
	void SFileNameToDatFileName( char *lpszDatFN);
	/// @brief Проверяет, является ли файл документом (первая строка начинается с `#TMCGROTS`, как у документов TMCROS).
	/// @param[in] ch путь к файлу
	/// @return TRUE — файл документа; FALSE — другой файл, пустой путь или файл не открылся.
	BOOL IsDocFileName( char *ch);
	/// @brief Готовит файл документа перед чтением.
	///
	/// Пустой путь (новый документ) — путь = заголовок окна, создаётся файл с одной меткой.
	/// Путь на несуществующий файл — ошибка `file not found`. Путь на файл, который не
	/// документ (например `.DAT`), — путь меняется на `.dop`/`.$op` и создаётся документ с
	/// одной ДН для этого файла (`имя имя 1 1 1 1 1`). Документ `.dop` не трогается.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#document
	void MakeDocFileName( void );
	/// @brief Удаляет из строки все пробелы, табуляции и переводы строки.
	/// @param[in,out] ch строка
	void del_bl3( char *ch );
	/// @brief Читает строку графика номер `nGr` из файла документа и его данные.
	///
	/// Строка — 7, 6, 5 или 1 поле: `имя подпись показ вход1 мода1 вход2 мода2`; без подписи
	/// подпись = имя, без номеров — 1, без признака показа — показывать. Имя в `{}` —
	/// кривая-выражение (`ReadGraph1Expession`), иначе файл излучателей (`ReadGraph1`).
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
};

/////////////////////////////////////////////////////////////////////////////
