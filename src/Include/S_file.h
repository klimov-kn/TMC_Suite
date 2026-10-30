/*
**  TAMIC_soft(R) 1991,1992 v1.00
**
**  s_file.
**
**  FILE : ..\include\s_file.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

/// @file S_file.h
/// @brief Структуры, константы и функции двоичного S-файла (матрицы рассеяния, метка `SMT`).
///
/// Часть модуля «Библиотека sfile95». Объявляет заголовок и таблицы смещений
/// S-файла, константы формата, функции низкоуровневого ввода-вывода (`Open_S_file`,
/// `write_S`, `read_S`), записи (`save_S_matrix`) и чтения (`read_S_matrix`)
/// многочастотной матрицы, чтения элемента S-матрицы из файлов HFSS.
/// Часть объявлений осталась от TAMIC: их реализации в сборку Qt6-версии не входят
/// (это отмечено у каждой функции).
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html

#include <Typedef.h>
#include <TMCLIMIT.H>

#ifndef _S_FILE_DEF
 #define _S_FILE_DEF

#ifndef _INPUT_DATA_DEF
 #include <Postpro.h>
#endif

#ifndef _OFFSET_DEF
 /// Признак того, что `_OFFSET` уже определён (защита от повторного определения).
 #define _OFFSET_DEF
/// @brief Запись таблицы смещений: частота и позиция блока этой частоты в S-файле.
///
/// В файле запись хранится не этой структурой, а двумя полями подряд: `double`
/// (8 байт) и `int` (4 байта), см. `update_offset_table`. Структура в коде
/// библиотеки не используется для ввода-вывода.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#offset-table
typedef struct
         {
          /// Частота точки, Гц.
          _real frq;
//          int  lOffset;
          /// Смещение блока данных частоты от начала файла, байт.
          DWORD  lOffset;
          }_OFFSET;
#endif

/// Синоним `_OFFSET` — запись таблицы смещений S-файла.
typedef _OFFSET S_OFFSET;

#ifndef _S_HEADER_DEF
 /// Признак того, что заголовок таблицы смещений уже определён (защита от повторного определения).
 #define _S_HEADER_DEF

#endif

/// @brief Таблица смещений S-файла в памяти.
///
/// В `write_offset_table` (write_s.cpp) используются только первые два поля:
/// они записываются в файл как два `WORD` — длина строки таблицы и число частот.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#offset-table
typedef struct
         {
          /// Длина таблицы, байт: `sizeof(WORD) + (sizeof(_real)+sizeof(int)) * число_частот`.
          WORD           cbSize;              /* offset table size*/
          /// Число уже записанных частот (для основной таблицы при создании файла — общее число точек).
          WORD           cnFrqNum;            /* frequency number*/
          /// Массив записей таблицы; при записи файла не заполняется.
          S_OFFSET *lpOffset;           /* pointer to offset table*/
          }S_OFFSET_TABLE;

/// Режим `Open_S_file`: запись (новый файл на первой частоте, дозапись на остальных).
#define S_WR 0
/// Режим `Open_S_file`: только чтение.
#define S_RD 1
/// Число «паскалевых» строк заголовка при `extrapol_flag` == 0; на каждую дополнительную сетку добавляется 2.
#define S_PS_NUMBER 7
/// Флаг `update_offset_table`: основная таблица смещений (матрицы по частотам).
#define OFF_TBL_MAIN 1
/// Флаг `update_offset_table`: таблица смещений коэффициентов интерполяции.
#define OFF_TBL_APPR 2
/// Метка S-файла в первых 4 байтах (строка «SMT» с завершающим нулём).
#define SMT_LABEL      "SMT"     /*scattering matrix file label*/
/// Версия формата S-файла, записывается текстом «102»; файлы старших версий читаются, младших — нет.
#define SMT_VERSION     102


/// @brief Возвращает имя S-файла, заданное `set_file_name`.
///
/// Реализация (s_db.cpp) в сборку Qt6-версии не входит.
/// @return Указатель на статический буфер с именем файла.
char * Get_S_file_name( void );
/// @brief Записывает S-матрицу одной частоты в S-файл.
///
/// Частота и её номер берутся из глобальной таблицы частот (`get_current_frq_num`,
/// `get_current_dbl_frq`). На точке номер 0 файл создаётся заново: пишется заголовок,
/// строка сетки, имена файлов и две пустые таблицы смещений (основная — на
/// `get_frq_nf()` записей). На остальных точках файл открывается на дозапись.
/// Затем в основную таблицу заносится смещение нового блока и в конец файла
/// дописывается блок: частота, число входов, числа мод, размерность, матрица.
/// @param[in] sss            матрица `dim*dim` комплексных чисел, хранение по строкам
/// @param[in] id             число входов, числа мод на входах, размерность `dim`
/// @param[in] S_filename     имя S-файла
/// @param[in] si             массив из `extrapol_flag+1` индексов сетки (имена .tpl/.s, шаги, м)
/// @param[in] extrapol_flag  число сеток минус один (0..2); обычно 0
/// @return 0 — успех; -1 — ошибка открытия или записи файла.
/// @pre `set_frq_nf`/`set_frequency` и `set_current_frq_num` вызваны до записи.
/// @warning Точки нужно писать по порядку начиная с 0: запись точки 0 стирает файл.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#write
int  save_S_matrix( _complex *sss, INPUT_DATA *id,
                          char *S_filename, S_INDEX *si, int extrapol_flag );
/// @brief Читает из S-файла матрицы рассеяния всех частот.
///
/// Проверяет метку `SMT` и версию, читает строку сетки и имена файлов, затем по
/// основной таблице смещений — блок каждой частоты. Память под результаты
/// выделяется внутри функции; освобождает её вызывающий (`free_mem` для
/// `frq_out->f` и `*id`, `Free_S_read` для `*sss`, `Free_GRID_Index` для `*si`).
/// @param[in]  filename       имя S-файла
/// @param[out] si             массив из `*extrapol_flag+1` индексов сетки (имена, шаги, м)
/// @param[out] extrapol_flag  число сеток минус один (0..2)
/// @param[out] sss            массив из `frq_out->max` указателей на матрицы `dim*dim`
/// @param[out] frq_out        частоты точек (Гц) в `f`, их число в `max`; единицы копируются из глобальной таблицы
/// @param[out] id             массив `INPUT_DATA` на каждую частоту
/// @return 0 — успех; -1 — ошибка чтения или неверные данные в файле;
///         -2 — файл не открылся, это не S-файл, версия младше 102 или нет памяти под индекс сетки;
///         -3 — не хватило памяти под массивы частот.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#read
int  read_S_matrix( char *filename, S_INDEX **si, int *extrapol_flag,
                          _complex ***sss, FREQUENCY *frq_out, INPUT_DATA **id);
/// @brief Сокращает многомодовые матрицы до матриц по одной (первой) моде на входе.
///
/// Реализация (s_cut.cpp) в сборку Qt6-версии не входит.
/// @param[in,out] sss  массив матриц по частотам; заменяется новыми матрицами
/// @param[in]     frq  частоты точек
/// @param[in,out] id   описания входов по частотам; заменяются новыми
/// @return 0 — успех, иначе ошибка.
int cut_S_matrix( _complex ***sss, FREQUENCY *frq, INPUT_DATA **id );
/// @brief Освобождает массив индексов сетки, выделенный `read_S_matrix`, и имена файлов в нём.
/// @param[in,out] si             адрес указателя на массив; после вызова `*si` == NULL
/// @param[in]     extrapol_flag  число сеток минус один — элементов в массиве `extrapol_flag+1`
void Free_GRID_Index( S_INDEX **si, int extrapol_flag );
/// @brief Освобождает матрицы всех частот и массив указателей на них.
/// @param[in,out] sss      адрес массива указателей; после вызова `*sss` == NULL; NULL допустим
/// @param[in]     nFrqNum  число частот (элементов массива)
void Free_S_read( _complex ***sss, int nFrqNum );
/// @brief Возвращает поток открытого S-файла.
/// @return Поток `FILE*` или NULL, если файл не открыт.
/// @note Библиотека держит один открытый S-файл на процесс (статическая переменная); не потокобезопасно.
FILE * Get_S_FILE( void );
/// @brief Запоминает число «паскалевых» строк заголовка открытого S-файла.
/// @param[in] nNum  число строк: `S_PS_NUMBER + 2*extrapol_flag`
void   Set_PS_Number( int nNum );
/// @brief Возвращает число «паскалевых» строк заголовка, заданное `Set_PS_Number`.
/// @return Число строк.
int    Get_PS_Number( void );
/// @brief Открывает S-файл и делает его текущим для `write_S`/`read_S`.
///
/// `S_RD` — режим "rb". `S_WR` — "wb+" (файл создаётся заново), если номер текущей
/// частоты 0, иначе "rb+" (дозапись в существующий).
/// @param[in] szFile     имя файла
/// @param[in] nOpenMode  `S_RD` или `S_WR`
/// @return 0 — успех; -1 — файл не открылся.
/// @warning Ранее открытый файл не закрывается — перед повторным открытием нужен `Close_S_file`.
int    Open_S_file( char *szFile, int nOpenMode );
/// @brief Закрывает текущий S-файл (свой буфер ввода-вывода есть только в сборке без `_NDP2`; `typedef.h` всегда задаёт `_NDP2`).
void   Close_S_file( void );
/// @brief Заносит частоту и смещение блока в таблицу смещений открытого S-файла.
///
/// Смещение — текущая длина файла (блок будет дописан в конец). Обновляет
/// число записанных частот (`num+1`) в шапке таблицы и оставляет позицию в конце файла.
/// @param[in] frq        частота точки, Гц
/// @param[in] num        номер точки (с нуля)
/// @param[in] nTBL_Flag  `OFF_TBL_MAIN` или `OFF_TBL_APPR`
/// @return 0 — успех; -1 — ошибка позиционирования или записи.
/// @warning Смещение хранится как 4-байтовый `int`: файлы длиннее 2 ГБ не поддерживаются.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#offset-table
int    update_offset_table( _real frq, int num, int nTBL_Flag );
/// @brief Пропускает в открытом S-файле одну «паскалеву» строку (длина, текст, разделитель).
/// @return 0 — успех; -1 — ошибка чтения или позиционирования.
int    skip_string( void );
/// @brief Записывает блок байтов в текущий S-файл.
/// @param[in] ptr   данные
/// @param[in] size  длина, байт
/// @return 0 — успех; -1 — ошибка записи.
int    write_S( const void *ptr, size_t size );
/// @brief Читает блок байтов из текущего S-файла.
/// @param[out] string  буфер не короче `size` байт
/// @param[in]  size    длина, байт
/// @return 0 — успех; -1 — ошибка или конец файла.
int    read_S( void *string, size_t size );

/// @brief Читает частотную зависимость одного элемента S-матрицы из текстового файла HFSS.
///
/// Номера входов и мод приводятся к допустимым (`SMatrixElementHFSS_InpMod`), затем
/// считаются точки, выделяется память и читаются частоты и элемент
/// S(`in1`,`mod1`; `in2`,`mod2`). Частоты в файле — ГГц, на выходе — Гц.
/// @param[in,out] in1      номер входа строки (с 1); исправляется в пределы 1..число входов
/// @param[in,out] mod1     номер моды строки (с 1); исправляется в пределы
/// @param[in,out] in2      номер входа столбца (с 1); исправляется
/// @param[in,out] mod2     номер моды столбца (с 1); исправляется
/// @param[in]     filename имя файла HFSS
/// @param[out]    sss      массив `*nPoint` значений элемента (выделен `new[]`, освобождает вызывающий)
/// @param[out]    f1       массив `*nPoint` частот, Гц (выделен `new[]`)
/// @param[out]    nPoint   число точек; -1 — файл не прочитан
/// @return 0 — входы и моды найдены (успех чтения — `*nPoint` > 0);
///         -1 — входы или моды в файле не найдены.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#import
int read_S_matrix_element_HFSS( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint);
/// @brief Приводит номера входов и мод к имеющимся в файле HFSS.
///
/// Номера меньше 1 становятся 1, номера входов больше максимального — максимальным,
/// номера мод — не больше числа мод своего входа.
/// @param[in,out] in1       номер входа строки
/// @param[in,out] mod1      номер моды строки
/// @param[in,out] in2       номер входа столбца
/// @param[in,out] mod2      номер моды столбца
/// @param[in]     filename  имя файла HFSS
/// @return 0 — успех; -1 — в файле нет входов или мод.
int SMatrixElementHFSS_InpMod( int *in1, int *mod1, int *in2, int *mod2, char *filename );
/// @brief Возвращает число мод входа `*pnInp` в файле HFSS (номер моды последней строки вида `-> P<вход> M<мода>`).
/// @param[in] filename  имя файла HFSS
/// @param[in] pnInp     номер входа (с 1)
/// @return Число мод; -1 — файл не открылся или вход не найден.
int SMatrixElementHFSS_ModMax( char *filename, int *pnInp );
/// @brief Возвращает число входов в файле HFSS (номер входа последней строки вида `-> P<вход> M<мода>`).
/// @param[in] filename  имя файла HFSS
/// @return Число входов; -1 — файл не открылся или строк нет.
int SMatrixElementHFSS_InpMax( char *filename );
/// @brief Разбирает в строке пару «вход, мода» по двум меткам.
///
/// Ищет в строке метку входа; если она есть, ищет метку моды с начала строки,
/// ставит на её место конец строки и читает целые числа после обеих меток.
/// @param[in,out] pszBuf           строка; портится (в место метки моды пишется '\0')
/// @param[in]     pszIdentifInput  метка входа (HFSS: "-> P")
/// @param[in]     pszIdentifMode   метка моды (HFSS: "M")
/// @param[out]    pnInp            номер входа (не меняется, если меток нет)
/// @param[out]    pnMod            номер моды
/// @return 1 — пара найдена; 0 — нет.
int ChooseString( char *pszBuf, const char *pszIdentifInput, const char *pszIdentifMode, int *pnInp, int *pnMod );
/// @brief Считает точки, выделяет память и читает частоты и элемент из файла HFSS.
///
/// Последовательно вызывает `SMatrixElementHFSS_FreqArrayCalculation`,
/// `..._FreqArrayAllocation` и `..._FreqArrayRead`.
/// @param[in]  in1          номер входа строки
/// @param[in]  mod1         номер моды строки
/// @param[in]  in2          номер входа столбца
/// @param[in]  mod2         номер моды столбца
/// @param[in]  filename     имя файла HFSS
/// @param[out] f1           массив частот, Гц
/// @param[out] sss          массив значений элемента
/// @param[out] nPoint       число точек; -1 — ошибка
/// @param[out] nMatrixSize  размерность матрицы (число пар «вход, мода»)
/// @return Всегда 0; результат — в `*nPoint`.
int SMatrixElementHFSS_FreqArray( int *in1, int *mod1, int *in2, int *mod2, char *filename, double **f1, _complex **sss, int *nPoint, int *nMatrixSize);
/// @brief Считает размерность матрицы и число частот в файле HFSS.
///
/// Размерность — число строк `-> P.. M..` до строки `!END EXCITATION_MAPS`; далее
/// считаются записи «частота + `nMatrixSize^2` пар (Re, Im)».
/// @param[in]  filename     имя файла HFSS
/// @param[in]  f1           не используется
/// @param[out] nPoint       число частот; -1 — ошибка
/// @param[out] nMatrixSize  размерность матрицы; -1 — ошибка
void SMatrixElementHFSS_FreqArrayCalculation( char *filename, double **f1, int *nPoint, int *nMatrixSize);
/// @brief Выделяет массивы частот и значений на `*nPoint` точек (`new[]`).
/// @param[out]    f1      массив частот
/// @param[out]    sss     массив значений
/// @param[in,out] nPoint  число точек; при `*nPoint` < 1 ничего не делается
void SMatrixElementHFSS_FreqArrayAllocation( double **f1, _complex **sss, int *nPoint);
/// @brief Читает частоты (ГГц -> Гц) и значения (Re, Im) выбранного элемента из файла HFSS.
///
/// Индекс элемента в записи частоты: `k = n1*nMatrixSize + n2`, где n1, n2 — порядковые
/// номера пар (`in1`,`mod1`) и (`in2`,`mod2`) в списке `-> P.. M..`.
/// При ошибке массивы освобождаются, указатели обнуляются, `*nPoint` = -1.
/// @param[in]     in1          номер входа строки
/// @param[in]     mod1         номер моды строки
/// @param[in]     in2          номер входа столбца
/// @param[in]     mod2         номер моды столбца
/// @param[in]     filename     имя файла HFSS
/// @param[in,out] f1           массив частот, Гц
/// @param[in,out] sss          массив значений элемента
/// @param[in,out] nPoint       число точек
/// @param[in]     nMatrixSize  размерность матрицы
void SMatrixElementHFSS_FreqArrayRead( int *in1, int *mod1, int *in2, int *mod2, char *filename, double **f1, _complex **sss, int *nPoint, int *nMatrixSize);

#endif
