// TmcRTH_BolckList.h : header file
//

/////////////////////////////////////////////////////////////////////////////

/// @file TmcRTH_BolckList.h
/// @brief Список блоков топологии задания: разбор примитивов RECT, CIRCLE, POLYGON,
///        FILE, входов INPUT_X/INPUT_Y и плазменных вариантов `_N`, `_B`, `_Y`, `_S`.
///
/// Часть библиотеки TMCIndan. Класс `CTmcRTH_BlockList` хранит один блок
/// задания и указатель на следующий; разбирает текст блока `BLOCK n; ... END_B`,
/// переводит размеры в СИ и определяет тип блока (диэлектрик, металл,
/// магнитная стенка, поглотитель, вход). Здесь же — метки формата файла
/// распределения `.eps` и формата сохранения списка блоков.
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#blocks

#ifndef CTMCRTH_BLOCKLIST_H__1
#define CTMCRTH_BLOCKLIST_H__1

/// Строка-метка начала списка блоков в тексте, созданном `Save`.
#define TMC_RTH_BLOCKLISTSAVE_BEGIN "#BlockListBegin"
/// Строка-метка конца списка блоков (по ней `Load` прекращает чтение).
#define TMC_RTH_BLOCKLISTSAVE_END   "#BlockListEnd"
/// Размер буфера строки при `Load` (символов).
#define TMC_RTH_BLOCKLISTLOADBUFSIZE  20000

/// Расширение файла распределения eps (N, B, Y), дописываемое к имени в блоках `FILE*`.
#define TMC_RTH_EPSFILE_EXT_	"eps"
/// Первая строка (сигнатура) файла распределения `.eps`.
#define TMC_RTH_EPSFILE_ID_		"#TamicRTH_planar_DistributionDielectricPermeability_File_V2.00 2000"
/// Ключ заголовка `.eps`: примитив (величина), записавший файл.
#define TMC_RTH_EPSFILE_TYPE_	"#TopologyPrimitiv "
/// Ключ заголовка `.eps`: шаг сетки.
#define TMC_RTH_EPSFILE_Delta_	"#dDelta "
/// Ключ заголовка `.eps`: левая граница прямоугольника данных.
#define TMC_RTH_EPSFILE_XMIN_	"#Xmin "
/// Ключ заголовка `.eps`: нижняя граница прямоугольника данных.
#define TMC_RTH_EPSFILE_YMIN_	"#Ymin "
/// Ключ заголовка `.eps`: число узлов по X.
#define TMC_RTH_EPSFILE_NX_		"#nX "
/// Ключ заголовка `.eps`: число узлов по Y.
#define TMC_RTH_EPSFILE_NY_		"#nY "
/// Ключ заголовка `.eps`: число записанных узлов.
#define TMC_RTH_EPSFILE_NPoint_	"#nPoint "
/// Ключ заголовка `.eps`: размер вещественного значения, байт.
#define TMC_RTH_EPSFILE_NAccur_	"#nAccuracy "
/// Ключ заголовка `.eps`: формат номера узла.
#define TMC_RTH_EPSFILE_NdFrmt_	"#sNodeFormat "
/// Ключ заголовка `.eps`: формат значения.
#define TMC_RTH_EPSFILE_VlFrmt_	"#sValueFormat "

#include <TmcLibError.h>
#include <Typerth.h>
#include <TmcRTH_IndanParam.h>


/// @brief Блок топологии задания (элемент односвязного списка блоков).
///
/// Список устроен так: голова — поле `cBlockList` топологии; элемент с
/// `nBlock != 0` хранит блок и указатель `pcNextBlockList` на следующий,
/// последний элемент — пустой (`nBlock == 0`, `pcNextBlockList == NULL`).
/// После разбора `#TOPOLOGY` номер блока хранится со знаком минус
/// («ещё не размещён»); `SetLink` при разборе `#LINK_LIST` делает его положительным.
///
/// Смысл геометрических полей зависит от примитива:
/// - RECT_*: `dXmin..dXmax`, `dYmin..dYmax` — прямоугольник, м;
/// - CIRCLE_*: `dXmin`/`dXmax` — радиусы Rmin/Rmax, м; `dYmin`/`dYmax` — углы
///   FImin/FImax, рад (0..2*pi);
/// - POLYGON_*: вершины в `dX[]`, `dY[]` (`nXY` штук, не меньше 3), м;
/// - INPUT_X: `dXmin`/`dXmax` — пределы входа по Y, м; INPUT_Y — по X, м;
///   у обоих `dYmin`/`dYmax` — время импульса Tmin/Tmax, с;
/// - FILE*: в `csEpsExpr` — имя файла `.eps`.
/// Все координаты — локальные, относительно точки привязки (`dX0`, `dY0`).
///
/// Создаётся `CTmcRTH_IndanTopology`; ядро читает список через `GetNext()`.
/// @note Потоки: не потокобезопасен.
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#blocks
class CTmcRTH_BlockList
{
public:
	friend class CTMCBlList;
	/// @brief Задаёт `dXmin`.
	/// @param[in] r значение, м (или Rmin для круга)
	void SetXmin( double r );
	/// @brief Задаёт `dXmax`.
	/// @param[in] r значение, м (или Rmax для круга)
	void SetXmax( double r );
	/// @brief Задаёт `dYmin`.
	/// @param[in] r значение, м (угол, рад, для круга; время, с, для входа)
	void SetYmin( double r );
	/// @brief Задаёт `dYmax`.
	/// @param[in] r значение, м (угол, рад, для круга; время, с, для входа)
	void SetYmax( double r );
	/// @brief Возвращает сумму счётчиков памяти `nMemory` этого и всех следующих блоков.
	/// @return Сумма (единицы задаёт ядро, заполняющее `SetnMemory`).
	int GetnMemoryAll( void );
	/// @brief Задаёт счётчик памяти блока (заполняется ядром для окна статистики).
	/// @param[in] nnn значение
	void SetnMemory( int nnn );
	/// @brief Возвращает счётчик памяти блока.
	/// @return Значение `nMemory`.
	int GetnMemory( void );
	/// @brief Возвращает 8-й параметр входа W (добавка eps в сечении входа) или
	///        угловую скорость подвижного блока.
	/// @return Текст выражения; пусто — не задан.
	CString GetcsW( void );
	/// @brief Возвращает фазу сигнала входа (выражение от t, f) или скорость Vy подвижного блока.
	/// @return Текст выражения; пусто — не задан.
	CString GetcsVy( void );
	/// @brief Возвращает амплитуду сигнала входа (выражение от t, f или `[файл]`)
	///        или скорость Vx подвижного блока.
	/// @return Текст выражения; начинается с `[` — падающая волна из файла .sig.
	CString GetcsVx( void );
	/// @brief Возвращает массив X-координат вершин многоугольника.
	/// @return Указатель на `nXY` значений, м; `NULL` — не многоугольник. Принадлежит блоку.
	double * GetpX( void );
	/// @brief Возвращает массив Y-координат вершин многоугольника.
	/// @return Указатель на `nXY` значений, м; `NULL` — не многоугольник. Принадлежит блоку.
	double * GetpY( void );
	/// @brief Возвращает число вершин многоугольника.
	/// @return `nXY`; 0 — не многоугольник.
	int GetnXY( void );
	/// @brief Возвращает первый параметр блока: выражение eps (N, B, Y, sigma для
	///        плазменных блоков), слово METAL/MAGNETIC/ABSORBER или имя файла `FILE*`.
	/// @return Текст параметра.
	CString GetcsEpsExpr( void );
	/// @brief Возвращает числовой тип блока `CTMCRTH_BLCKNTYPE_*` (`tmcgrviw.h`).
	/// @return 0 — диэлектрик/плазма; 100 — поглотитель; 101 — металл; 102..105 — входы
	///         (слева, справа, сверху, снизу); 107 — магнитная стенка.
	int GetnType( void );
	/// @brief Задаёт числовой тип блока (используется для уточнения стороны входа).
	/// @param[in] n код `CTMCRTH_BLCKNTYPE_*`
	void SetnType( int n );
	/// @brief Возвращает число блоков от этого элемента до конца списка.
	/// @return Число непустых элементов.
	int GetBlockNumber( void );
	/// @brief Возвращает следующий элемент списка.
	/// @return Указатель; у последнего (пустого) элемента — `NULL`.
	CTmcRTH_BlockList * GetNext( void );
	/// @brief Заменяет список копией другого списка (глубокое копирование).
	/// @param[in] cBlList голова копируемого списка
	/// @return `*this`.
	CTmcRTH_BlockList& operator=( CTmcRTH_BlockList& cBlList );
	/// @brief Задаёт номер блока.
	/// @param[in] iBuf номер (положительный — размещён, отрицательный — не размещён)
	void SetnBlock( int iBuf );
	/// @brief Задаёт X точки привязки блока.
	/// @param[in] rX координата, м
	void SetX0( double rX );
	/// @brief Задаёт Y точки привязки блока.
	/// @param[in] rY координата, м
	void SetY0( double rY );
	/// @brief Возвращает номер блока.
	/// @return Номер (отрицательный — блок ещё не размещён в `#LINK_LIST`; 0 — пустой элемент).
	int GetnBlock( void );
	/// @brief Возвращает имя примитива блока.
	/// @return Например, `"RECT_STAT_N "`, `"INPUT_X "`, `"FILE_B "`; до разбора — весь текст блока.
	CString GetcsBlock( void );
	/// @brief Возвращает X точки привязки (из `#LINK_LIST`).
	/// @return X0, м.
	double GetX0( void );
	/// @brief Возвращает Y точки привязки (из `#LINK_LIST`).
	/// @return Y0, м.
	double GetY0( void );
	/// @brief Возвращает `dXmin` (смысл зависит от примитива, см. описание класса).
	/// @return Значение в СИ.
	double GetXmin( void );
	/// @brief Возвращает `dXmax` (смысл зависит от примитива).
	/// @return Значение в СИ.
	double GetXmax( void );
	/// @brief Возвращает `dYmin` (смысл зависит от примитива).
	/// @return Значение в СИ.
	double GetYmin( void );
	/// @brief Возвращает `dYmax` (смысл зависит от примитива).
	/// @return Значение в СИ.
	double GetYmax( void );
	/// @brief Размещает блок `iBuf`: делает номер положительным и задаёт точку привязки.
	/// @param[in]     iBuf   номер блока (>= 1)
	/// @param[in]     x0     X привязки, м
	/// @param[in]     y0     Y привязки, м
	/// @param[in,out] cError ошибка: номер < 1, блока нет, блок уже размещён
	void SetLink( int iBuf, double x0, double y0, CTmcLibError &cError );
	/// @brief Ищет блок с номером `nBlock1` (со знаком) от этого элемента до конца списка.
	/// @param[in] nBlock1 номер блока (отрицательный — ещё не размещённый)
	/// @return Указатель на блок; `NULL` — не найден.
	CTmcRTH_BlockList * FindBlock( int nBlock1 );
	/// @brief Проверяет, что номера блоков не повторяются.
	/// @param[in,out] cError ошибка «`Dublicated BLOCK n in #TOPOLOGY section`»
	void IsCorrectly( CTmcLibError &cError );
	/// @brief Проверяет, что все блоки размещены (номер положителен).
	/// @param[in,out] cError ошибка «BLOCK n is absent in #LINK_LIST section»
	void IsCorrectlyLink( CTmcLibError &cError );
	/// @brief Разбирает текст блока и записывает его в первый свободный элемент списка.
	///
	/// Текст — строки `BLOCK n;\n<ПРИМИТИВ> <параметры через ;>\n[L x; y;\n ...]`.
	/// Номер n >= 1 сохраняется со знаком минус; после записи создаётся новый
	/// пустой последний элемент. Разбор примитива — `Read`.
	/// @param[in]     csCh    текст блока из `CTmcRTH_Indan::GetBlock`
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага (единицы); `NULL` — ошибка
	/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#blocks
	void Add( CString &csCh, CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Дописывает в конец списка копию блока `csCh` и всех следующих за ним.
	/// @param[in] csCh первый копируемый блок
	void Add( CTmcRTH_BlockList &csCh );
	/// @brief Освобождает весь хвост списка и вершины; возвращает элемент к пустому состоянию.
	void DeleteData( void );
	/// @brief Создаёт пустой элемент списка (тип по умолчанию — металл).
	CTmcRTH_BlockList();
public:
	/// @brief Дописывает к тексту блоки от этого элемента до конца списка (формат см. `Load`).
	/// @param[in,out] csBuffer текст, к которому дописываются блоки
	/// @return TRUE, если дошли до пустого последнего элемента.
	BOOL Save1(CString &csBuffer);
	/// @brief Записывает список блоков в текст между метками `#BlockListBegin`/`#BlockListEnd`.
	///
	/// Сохраняются тип, первый параметр, Vx, Vy, W, геометрия, номер, тип и вершины;
	/// выражения mu, Dz, Hx, Hy и силовые параметры не сохраняются.
	/// @param[out] csBuffer текст списка (прежнее содержимое заменяется)
	/// @return Результат `Save1`.
	BOOL Save( CString &csBuffer  );
	/// @brief Для блоков `FILE`, `FILE_N`, `FILE_B`, `FILE_Y` дописывает к имени файла
	///        расширение `.eps` (если его нет) и папку задания (если путь не полный).
	/// @param[in] csPath папка задания
	void AddPathForFile( CString &csPath );
	/// @brief Уничтожает элемент и весь хвост списка.
	virtual ~CTmcRTH_BlockList();
private:
	/// Буфер строки для `Load`.
	char pszBufForReadTopology[TMC_RTH_BLOCKLISTLOADBUFSIZE];
	/// @brief Полный ли путь в имени файла блока (`csEpsExpr`).
	/// @return TRUE — путь начинается с `\` или `/`, содержит `:`, или имя пустое.
	BOOL IsFullName( void );
	/// @brief Приписывает папку задания к имени файла блока, если путь не полный.
	/// @param[in] csCurrentPath папка задания
	void AddCurrentPath(CString csCurrentPath);
	/// @brief Дописывает `.eps` к имени файла блока, если в имени файла (не папки) нет точки.
	void AddFileEpsExtention( void );
	/// Счётчик памяти блока, заполняемый ядром (`SetnMemory`) для окна статистики.
	int nMemory;
	/// @brief Разбирает строку вершины многоугольника `L x; y;`.
	/// @param[in]     szBuf   строка с текущей позиции
	/// @param[in,out] cError  ошибка: не `L`, нет `;`, неверное выражение
	/// @param[in]     pcParam параметры (единица длины)
	/// @param[out]    dX1     X вершины, м
	/// @param[out]    dY1     Y вершины, м
	void ReadXY( char *szBuf,  CTmcLibError &cError, CTmcRTH_IndanParam * pcParam , double *dX1, double *dY1 );
	/// @brief Считает переводы строк до конца текста (число строк вершин многоугольника).
	/// @param[in] szBuf текст
	/// @return Число символов `\n`.
	int CalcEOL( char *szBuf  );
	/// @brief Находит начало следующей строки.
	/// @param[in] szBuf текст
	/// @return Смещение символа после первого `\n`; 0 — `\n` нет.
	int SkipEOL( char *szBuf );
	/// Массив X-координат вершин многоугольника, м (`NULL` — не многоугольник).
	double *dX;		  // X array for polygon
	/// Массив Y-координат вершин многоугольника, м.
	double *dY;		  // Y array for polygon
	/// Число вершин многоугольника.
	int nXY;		  // size array for polygon
	/// @brief Выделяет очередной параметр до `;` верхнего уровня (вне скобок `{}()[]`).
	/// @param[out]    csBuf  текст параметра без `;`
	/// @param[in,out] szBuf  строка с текущей позиции; временно изменяется и восстанавливается
	/// @param[in,out] cError ошибка «Missing separator ;», если `;` нет
	/// @return Сдвиг позиции за `;`; -1 при ошибке.
	int Search_1Param( CString &csBuf, char *szBuf, CTmcLibError &cError );
	/// @brief Удаляет из строки пробелы, табуляции и переводы строк (на месте).
	/// @param[in,out] ch строка, завершённая нулём
	/// @return Всегда 0.
	int expr_del_Blanks2( char *ch );
	/// @brief То же для `CString` (не длиннее `CTMCRTH_INDAN_LINELEN - 1` символов).
	/// @param[in,out] csBuf строка
	/// @return Всегда 0.
	int expr_del_Blanks2( CString &csBuf );
	/// @brief Разбирает примитив `FILE <имя>;` — распределение eps из файла `.eps`.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	void ReadFileStat( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Разбирает `FILE_N <имя>;` — концентрация N (м^-3) из файла (X-ядро).
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	void ReadFileStatN( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Разбирает `FILE_B <имя>;` и `FILE_Y <имя>;` (метки одной длины) — поле B (Тл)
	///        или проводимость Y из файла (X-ядро).
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	void ReadFileStatB( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Общая часть разбора `FILE*`: сохраняет метку примитива и имя файла в `csEpsExpr`.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	/// @param[in]     i       длина метки примитива (смещение первого параметра)
	void ReadFileStat_1( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam, int i );
	/// @brief Разбирает `RECT_STAT eps|METAL|MAGNETIC|ABSORBER; Xmin; Xmax; Ymin; Ymax; [...]`.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	void ReadRectStat( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Разбирает `RECT_STAT_N N; ...` — концентрация электронов N, м^-3 (только выражение).
	/// @param[in,out] cError  ошибка; METAL/MAGNETIC/ABSORBER недопустимы
	/// @param[in]     pcParam параметры шага
	/// @see https://www.tamic.ru/docs/theory/plasma.html#plasma-frequency
	void ReadRectStatN( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Разбирает `RECT_STAT_B B; ...` — индукция подмагничивающего поля B, Тл.
	/// @param[in,out] cError  ошибка; METAL/MAGNETIC/ABSORBER недопустимы
	/// @param[in]     pcParam параметры шага
	/// @see https://www.tamic.ru/docs/theory/plasma.html#cyclotron
	void ReadRectStatB( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Разбирает `RECT_STAT_Y Y; ...` — потери (проводимость Y); также используется
	///        для `RECT_STAT_S` (sigma, См/м).
	/// @param[in,out] cError  ошибка; METAL/MAGNETIC/ABSORBER недопустимы
	/// @param[in]     pcParam параметры шага
	/// @see https://www.tamic.ru/docs/theory/plasma.html#collisions
	void ReadRectStatY( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Общая часть разбора прямоугольника.
	///
	/// Параметры: значение или METAL/MAGNETIC/ABSORBER (определяет `nType`);
	/// Xmin; Xmax; Ymin; Ymax (единицы `LONG_UNIT`, обязательны, min < max);
	/// далее необязательные mu; Dz; Hx; Hy; Ro; Sigma — сохраняются как текст.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	/// @param[in]     i       длина метки примитива
	void ReadRectStat_1( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam, int i );
	/// @brief Разбирает `CIRCLE_STAT значение; Rmin; Rmax; FImin; FImax; [...]` — кольцевой сектор.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	void ReadCircleStat( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Разбирает `CIRCLE_STAT_N` — N, м^-3, в кольцевом секторе.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	void ReadCircleStatN( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Разбирает `CIRCLE_STAT_B` — B, Тл, в кольцевом секторе.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	void ReadCircleStatB( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Разбирает `CIRCLE_STAT_Y` (а также `CIRCLE_STAT_S`) — потери в кольцевом секторе.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	void ReadCircleStatY( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );   // A-11
	/// @brief Общая часть разбора кольцевого сектора.
	///
	/// Параметры: значение или METAL/MAGNETIC/ABSORBER; Rmin; Rmax (`LONG_UNIT`,
	/// 0 <= Rmin < Rmax); FImin; FImax (`ANGLE_UNIT`, 0 <= FImin < FImax <= 2*pi);
	/// далее необязательные mu; Dz; Hx; Hy; Ro; Sigma; Dz внешний; проводимость.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	/// @param[in]     i       длина метки примитива
	void ReadCircleStat_1( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam, int i );
	/// @brief Разбирает `POLYGON_STAT значение;` и строки вершин `L x; y;` (не меньше трёх).
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	void ReadPolygonStat( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Разбирает `POLYGON_STAT_N` — N, м^-3, в многоугольнике.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	void ReadPolygonStatN( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Разбирает `POLYGON_STAT_B` — B, Тл, в многоугольнике.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	void ReadPolygonStatB( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Разбирает `POLYGON_STAT_Y` (а также `POLYGON_STAT_S`) — потери в многоугольнике.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	void ReadPolygonStatY( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );   // A-11
	/// @brief Общая часть разбора многоугольника: значение, затем строки `L x; y;` (`LONG_UNIT`).
	/// @param[in,out] cError  ошибка: меньше трёх вершин, неверная строка вершины
	/// @param[in]     pcParam параметры шага
	/// @param[in]     i       длина метки примитива
	void ReadPolygonStat_1( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam, int i );
	/// @brief Разбирает подвижный прямоугольник `RECT_MOVE`: как `RECT_STAT`, затем
	///        необязательные mu; Dz; Hx; Hy; Vx; Vy; W; файл степеней свободы; Ro; Sigma.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	void ReadRectMove( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Разбирает подвижный кольцевой сектор `CIRCLE_MOVE` (как `CIRCLE_STAT` плюс
	///        Vx; Vy; W; файл степеней свободы; Ro; Sigma; Dz внешний; проводимость).
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	void ReadCircleMove( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Разбирает подвижный многоугольник `POLYGON_MOVE значение; Vx; Vy; W;` и вершины `L x; y;`.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	void ReadPolygonMove( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Разбирает вход `INPUT_X [METAL|MAGNETIC]; Ymin; Ymax; Tmin; Tmax; [Amp]; [Faza]; [W];`.
	///
	/// Вход на левой или правой границе (сторона уточняется в `#LINK_LIST`).
	/// Пусто или METAL — волна H10 волновода, MAGNETIC — плоская волна.
	/// Ymin/Ymax — `LONG_UNIT`, Tmin/Tmax — `TIME_UNIT` (время радиоимпульса).
	/// Amp — выражение от t, f или `[файл]` (падающая волна из файла .sig);
	/// Faza — выражение от t, f; W — добавка eps в сечении входа, eps = 1 + W > 0,
	/// без переменных t и f. Если задан Amp, обязателен и Faza.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	/// @see https://www.tamic.ru/docs/theory/ports.html#signal
	/// @see https://www.tamic.ru/docs/theory/ports.html#port-w
	void ReadInputX( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Разбирает вход `INPUT_Y [METAL|MAGNETIC]; Xmin; Xmax; Tmin; Tmax; [Amp]; [Faza]; [W];`
	///        на нижней или верхней границе (параметры — как у `ReadInputX`).
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага
	/// @see https://www.tamic.ru/docs/theory/ports.html#signal
	void ReadInputY( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// @brief Определяет примитив по началу `csBlock` и вызывает его разборщик.
	///
	/// Метки сравниваются целиком (с завершающим пробелом), поэтому
	/// `RECT_STAT_N` не принимается за `RECT_STAT`. `*_STAT_S` разбираются
	/// как `*_STAT_Y`, но сохраняют свою метку. Неизвестный примитив — ошибка
	/// со списком допустимых.
	/// @param[in,out] cError  ошибка разбора
	/// @param[in]     pcParam параметры шага; `NULL` — ошибка
	void Read( CTmcLibError &cError, CTmcRTH_IndanParam * pcParam );
	/// X точки привязки блока (из `#LINK_LIST`), м.
	double dX0;
	/// Y точки привязки блока (из `#LINK_LIST`), м.
	double dY0;
	/// Xmin прямоугольника, м; Rmin круга, м; Ymin входа INPUT_X (Xmin INPUT_Y), м.
	double dXmin;
	/// Xmax прямоугольника, м; Rmax круга, м; Ymax входа INPUT_X (Xmax INPUT_Y), м.
	double dXmax;
	/// Ymin прямоугольника, м; FImin круга, рад; Tmin входа, с.
	double dYmin;
	/// Ymax прямоугольника, м; FImax круга, рад; Tmax входа, с.
	double dYmax;
	/// Следующий элемент списка (владеет им; `NULL` у последнего пустого элемента).
	CTmcRTH_BlockList * pcNextBlockList;
	/// Номер блока: < 0 — прочитан, но не размещён; > 0 — размещён; 0 — пустой элемент.
	int nBlock;
	/// Числовой тип блока `CTMCRTH_BLCKNTYPE_*` (см. `GetnType`).
	int nType;
	/// Имя примитива (`"RECT_STAT "`, `"INPUT_X "` …); до разбора — весь текст блока.
	CString csBlock;
	/// Первый параметр: выражение eps (или N, B, Y, sigma), METAL/MAGNETIC/ABSORBER,
	/// имя файла для `FILE*`; у входов — тип моды. Может начинаться с `[имя]` —
	/// признак записи распределения в файл `.eps` (обрабатывается ядром).
	CString csEpsExpr;
	/// Необязательное выражение mu (текст; ядрами не используется).
	CString csMuExpr;
	/// Необязательное выражение Dz (текст; ядрами не используется).
	CString csDzExpr;
	/// Необязательное выражение Hx (текст; ядрами не используется).
	CString csHxExpr;
	/// Необязательное выражение Hy (текст; ядрами не используется).
	CString csHyExpr;
	/// Необязательный параметр Ro силовой модели (текст; ядрами не используется).
	CString csForceRo;
	/// Необязательный параметр Sigma силовой модели (текст; ядрами не используется).
	CString csForceSigma;
	/// Необязательное выражение внешнего Dz круга (текст; ядрами не используется).
	CString csDzExprOut;
	/// Необязательная проводимость силовой модели круга (текст; ядрами не используется).
	CString csForceConductivity;
	/// Имя файла механических степеней свободы подвижного блока (ядрами не используется).
	CString csFileNameMechanicalFreedomDegree;
	/// Вход: амплитуда сигнала (выражение от t, f или `[файл .sig]`); подвижный блок: скорость Vx.
	CString csVx;	// liner speed X for input is Amplitude from t
	/// Вход: фаза сигнала (выражение от t, f, в `ANGLE_UNIT`); подвижный блок: скорость Vy.
	CString csVy;	// liner speed Y for input is Faza from t
	/// Вход: W — добавка eps в сечении входа (eps = 1 + W); подвижный блок: угловая скорость.
	CString csW;	// angular speed for input is Eps
public:
	/// @brief Возвращает выражение mu.
	/// @return Текст; пусто — не задано.
	CString GetcsMuExpr(void);
	/// @brief Возвращает выражение Dz.
	/// @return Текст; пусто — не задано.
	CString GetcsDzExpr(void);
	/// @brief Возвращает выражение Hx.
	/// @return Текст; пусто — не задано.
	CString GetcsHxExpr(void);
	/// @brief Возвращает выражение Hy.
	/// @return Текст; пусто — не задано.
	CString GetcsHyExpr(void);
	/// @brief Возвращает имя файла механических степеней свободы.
	/// @return Текст; пусто — не задано.
	CString GetcsFileNameMechanicalFreedomDegree(void);
	/// @brief Возвращает параметр Ro силовой модели.
	/// @return Текст; пусто — не задано.
	CString GetcsForceRo(void);
	/// @brief Возвращает параметр Sigma силовой модели.
	/// @return Текст; пусто — не задано.
	CString GetcsForceSigma(void);
	/// @brief Возвращает проводимость силовой модели.
	/// @return Текст; пусто — не задано.
	CString GetcsForceConductivity(void);
	/// @brief Возвращает выражение внешнего Dz.
	/// @return Текст; пусто — не задано.
	CString GetcsDzExprOut(void);
};

#endif


