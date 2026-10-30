/*
 *  TAMIC_soft(R) 1992 vR1.02
 *
 *  Definitions and declarations  for S-files reading utility.
 *
 *  FILE : ..\proc_s.h
 *
*/

/// @file Proc_s.h
/// @brief Внешний интерфейс sfile95 для программ: запись столбца S-матрицы, чтение элемента из .s/HFSS/CST, объединение S-файлов.
///
/// Часть модуля «Библиотека sfile95». Через этот заголовок счётные ядра
/// (библиотека TMCIndan) и вьюверы пишут S-файл по столбцам
/// (`save_S_matrix_element`), а TMCGROUT читает частотную зависимость одного
/// элемента (`read_S_matrix_element`). Остальные объявления — от утилиты TAMIC
/// для работы с S-файлами; их реализации в сборку Qt6-версии не входят
/// (отмечено у каждой функции).
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html

#ifndef _PROC_S_D
 #define _PROC_S_D

/// Заголовок утилиты TAMIC для S-файлов (наследие; в сборке Qt6-версии не используется).
#define TITLE_STR        "TAMIC S-files control utility.  Version   3.03."
/// Строка авторских прав утилиты TAMIC (наследие).
#define COPYRIGHT_TITLE  "Copyright  (C) TAMIC-soft. All rights reserved."

/// Вид текстовой таблицы S-матрицы: действительная и мнимая части (`Set_Table_Mode`).
#define REAL_IMAG  0
/// Вид текстовой таблицы S-матрицы: модуль и фаза.
#define ARG_PHASE  1
/// Вид текстовой таблицы S-матрицы: КСВН и потери.
#define VSWR_LOSS  2


/// @brief Запоминает имя S-файла для утилиты TAMIC. Реализация (s_db.cpp) в сборку Qt6-версии не входит.
/// @param[in] name  имя файла, короче `FILENAME_MAX`
/// @return 0 — успех; -1 — имя слишком длинное.
int  set_file_name( char *name );
/// @brief Возвращает имя S-файла утилиты TAMIC. Определения с таким именем в исходниках нет (в s_db.cpp — `Get_S_file_name`).
/// @return Имя файла.
char * get_S_file_name( void );

/// @brief Главная процедура утилиты TAMIC чтения S-файлов. Реализация (sread.cpp) в сборку Qt6-версии не входит.
/// @return Код завершения (0 — успех).
int processor( void );
/// @brief Выдаёт сообщение об ошибке по коду из `error1.h`/`error2.h`.
///
/// В сборке (errors.cpp) — заглушка: тело закомментировано, сообщений нет,
/// всегда возвращает 0. Библиотека prepr объявляет отдельную функцию
/// `put_error_messege(const char*)` с текстом сообщения.
/// Параметры и возвращаемое значение описаны при объявлении в error1.h.
/// @warning Из-за заглушки ошибки чтения и записи S-файлов не выводятся; узнать о них
///          можно только по кодам возврата функций.
int put_error_messege( int err_num, ... );
/// @brief Задаёт вид текстовой таблицы (`REAL_IMAG`, `ARG_PHASE`, `VSWR_LOSS`). Реализация (stxtable.cpp) в сборку Qt6-версии не входит.
/// @param[in] nMode  вид таблицы
void Set_Table_Mode( int nMode );
/// @brief Задаёт флаг вывода текстовой таблицы утилиты TAMIC. Реализация (s_db.cpp) в сборку Qt6-версии не входит.
/// @param[in] nFlag  0 — не выводить, иначе выводить
void Set_OutTextFlag( int nFlag );
/// @brief Возвращает флаг вывода текстовой таблицы. Реализация (s_db.cpp) в сборку Qt6-версии не входит.
/// @return Значение флага.
int  Get_OutTextFlag( void );
/// @brief Выводит строку состояния в последнюю строку текстового экрана. Реализация (outtrace.cpp) в сборку Qt6-версии не входит.
/// @param[in] trace  текст
void   OutTrace( char *trace );

/// @brief Возвращает строку задания с номером `n` (наследие TAMIC). Определения в исходниках пакета нет.
/// @param[in] n  номер строки
/// @return Строка.
char  * get_tpl_line( int n );
/// @brief Читает частотную зависимость элемента S(`in1`,`mod1`; `in2`,`mod2`) из файла любого поддерживаемого формата.
///
/// Пробует по очереди: S-файл TMC (`read_S_matrix_element_TMC`), текст HFSS,
/// текст CST; останавливается на первом, давшем `*nPoint` > 0.
/// @param[in,out] in1       номер входа строки (с 1); исправляется в допустимые пределы
/// @param[in,out] mod1      номер моды строки (с 1); исправляется
/// @param[in,out] in2       номер входа столбца (с 1); исправляется
/// @param[in,out] mod2      номер моды столбца (с 1); исправляется
/// @param[in]     filename  имя файла
/// @param[out]    sss       массив `*nPoint` значений элемента (`new[]`, освобождает вызывающий)
/// @param[out]    f1        массив `*nPoint` частот, Гц, для CST — как в файле (`new[]`)
/// @param[out]    nPoint    число точек; <= 0 — файл не прочитан ни в одном формате
/// @return Всегда 0; результат — в `*nPoint`.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#import
int read_S_matrix_element( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint);
/// @brief Читает частотную зависимость одного элемента S-матрицы из S-файла TMC.
///
/// Читает весь файл (`read_S_matrix`), исправляет номера входов (1..число входов
/// первой частоты) и мод (не меньше 1), затем собирает элемент по тем частотам,
/// на которых у обоих входов есть нужные моды. Матрица хранится по строкам, порядок
/// строк и столбцов — по входам, внутри входа — по модам.
/// @param[in,out] in1       номер входа строки (с 1)
/// @param[in,out] mod1      номер моды строки (с 1)
/// @param[in,out] in2       номер входа столбца (с 1)
/// @param[in,out] mod2      номер моды столбца (с 1)
/// @param[in]     filename  имя S-файла
/// @param[out]    sss       массив значений (`new[]`, освобождает вызывающий)
/// @param[out]    f1        массив частот, Гц (`new[]`)
/// @param[out]    nPoint    число точек; -1 — ошибка
/// @return 0 — успех; -1 — файл не прочитан или элемента нет ни на одной частоте.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#import
int read_S_matrix_element_TMC( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint);
/// @cond
// Читает элемент S-матрицы из файла HFSS. Повторное объявление; описание — в `s_file.h`.
int read_S_matrix_element_HFSS( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint);
/// @endcond
/// @brief Читает частотную зависимость одного элемента S-матрицы из текстового файла CST.
///
/// Первая строка файла — заголовок со столбцами `WavePort<вход>):Mode(<мода>`,
/// вторая пропускается, далее по строке на частоту: частота и `nMatrixSize^2` пар
/// «модуль, фаза в градусах». Значение переводится в (Re, Im).
/// @param[in,out] in1       номер входа строки (с 1); исправляется
/// @param[in,out] mod1      номер моды строки (с 1); исправляется
/// @param[in,out] in2       номер входа столбца (с 1); исправляется
/// @param[in,out] mod2      номер моды столбца (с 1); исправляется
/// @param[in]     filename  имя файла CST
/// @param[out]    sss       значения элемента (`new[]`)
/// @param[out]    f1        частоты — как записаны в файле, без пересчёта единиц (`new[]`)
/// @param[out]    nPoint    число точек; -1 — ошибка
/// @return 0 — входы и моды найдены (успех чтения — `*nPoint` > 0); -1 — нет.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#import
int read_S_matrix_element_CST( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint);
/// @brief Приводит номера входов и мод к имеющимся в файле CST (как `SMatrixElementHFSS_InpMod`).
/// @param[in,out] in1       номер входа строки
/// @param[in,out] mod1      номер моды строки
/// @param[in,out] in2       номер входа столбца
/// @param[in,out] mod2      номер моды столбца
/// @param[in]     filename  имя файла CST
/// @return 0 — успех; -1 — в заголовке нет входов или мод.
int SMatrixElementCST_InpMod( int *in1, int *mod1, int *in2, int *mod2, char *filename );
/// @brief Возвращает число мод входа `*pnInp` по заголовку файла CST (номер моды последнего столбца этого входа).
/// @param[in] filename  имя файла CST
/// @param[in] pnInp     номер входа (с 1)
/// @return Число мод; -1 — файл не открылся или вход не найден.
int SMatrixElementCST_ModMax( char *filename, int *pnInp );
/// @brief Возвращает число входов по заголовку файла CST (номер входа последнего столбца `WavePort..`).
/// @param[in] filename  имя файла CST
/// @return Число входов; -1 — файл не открылся или столбцов нет.
int SMatrixElementCST_InpMax( char *filename );
/// @brief Считает точки, выделяет память и читает частоты и элемент из файла CST.
/// @param[in]  in1          номер входа строки
/// @param[in]  mod1         номер моды строки
/// @param[in]  in2          номер входа столбца
/// @param[in]  mod2         номер моды столбца
/// @param[in]  filename     имя файла CST
/// @param[out] f1           массив частот
/// @param[out] sss          массив значений
/// @param[out] nPoint       число точек; -1 — ошибка
/// @param[out] nMatrixSize  размерность матрицы
/// @return Всегда 0; результат — в `*nPoint`.
int SMatrixElementCST_FreqArray( int *in1, int *mod1, int *in2, int *mod2, char *filename, double **f1, _complex **sss, int *nPoint, int *nMatrixSize);
/// @brief Считает размерность матрицы (число столбцов `WavePort..` в заголовке) и число строк данных в файле CST.
///
/// Число точек — число строк файла начиная с третьей.
/// @param[in]  filename     имя файла CST
/// @param[in]  f1           не используется
/// @param[out] nPoint       число точек; -1 — ошибка
/// @param[out] nMatrixSize  размерность матрицы; -1 — ошибка
void SMatrixElementCST_FreqArrayCalculation( char *filename, double **f1, int *nPoint, int *nMatrixSize);
/// @brief Выделяет массивы частот и значений на `*nPoint` точек (`new[]`).
/// @param[out]    f1      массив частот
/// @param[out]    sss     массив значений
/// @param[in,out] nPoint  число точек; при `*nPoint` < 1 ничего не делается
void SMatrixElementCST_FreqArrayAllocation( double **f1, _complex **sss, int *nPoint);
/// @brief Читает частоты и выбранный элемент из файла CST, переводя «модуль, фаза в градусах» в (Re, Im).
///
/// Если пара (`in1`,`mod1`) или (`in2`,`mod2`) в заголовке не найдена, берётся
/// первый столбец заголовка (индекс 0). При ошибке чтения массивы освобождаются,
/// указатели обнуляются, `*nPoint` = -1.
/// @param[in]     in1          номер входа строки
/// @param[in]     mod1         номер моды строки
/// @param[in]     in2          номер входа столбца
/// @param[in]     mod2         номер моды столбца
/// @param[in]     filename     имя файла CST
/// @param[in,out] f1           массив частот
/// @param[in,out] sss          массив значений
/// @param[in,out] nPoint       число точек
/// @param[in]     nMatrixSize  размерность матрицы
void SMatrixElementCST_FreqArrayRead( int *in1, int *mod1, int *in2, int *mod2, char *filename, double **f1, _complex **sss, int *nPoint, int *nMatrixSize);
/// @brief Записывает в S-файл столбец `in1` S-матрицы на частоте `f1`.
///
/// Если файла нет (или он пуст), создаётся новый файл на одну частоту: матрица
/// `nInput x nInput` (по одной моде на входе), незаполненные элементы — `FLT_MAX`.
/// Если частота уже есть в файле, столбец заменяется на месте и файл
/// перезаписывается целиком. Если частоты нет, она записывается во временный
/// файл `<имя>$` и добавляется `S_add` с сортировкой по частоте.
/// @param[in] in1       номер столбца (входа возбуждения), 1..`nInput`
/// @param[in] filename  имя S-файла
/// @param[in] sss       столбец: `nInput` элементов S(i, `in1`), i = 1..`nInput`
/// @param[in] f1        частота, Гц; совпадение с частотой файла — с относительной точностью 1e-15
/// @param[in] nInput    число входов, 1..`MAX_INPUT_NUM`
/// @return 0 — успех; -1 — неверное `nInput`, нет памяти, ошибка записи или структура
///         файла не та (другое число входов или не одна мода на входе; файл не меняется);
///         -2 — существующий непустой файл не читается как S-файл (файл не перезаписывается).
/// @warning Использует глобальную таблицу частот и переводит частоту через ГГц
///          (`set_frequency(f1*1e-9, ...)`): текущие единицы должны оставаться ГГц
///          (код 3, значение по умолчанию).
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#save-column
/// @see https://www.tamic.ru/docs/theory/smatrix.html#s-from-signals
int save_S_matrix_element( int in1, char *filename, _complex *sss, double f1, int nInput);
/// @brief Объединяет два S-файла: частоты обоих, без повторов, по возрастанию — записываются в `filename`.
///
/// Файлы должны иметь одинаковый `extrapol_flag` и (при `extrapol_flag` == 0)
/// одинаковые шаги сетки. Частоты второго файла, совпавшие с частотами первого,
/// заменяют их. Если `filename1` == NULL, первый файл только читается.
/// @param[in] filename   первый S-файл; он же файл результата (перезаписывается)
/// @param[in] filename1  второй S-файл или NULL
/// @return 0 — успех; -1 — ошибка чтения, несовместимые сетки, больше `FREQ_MAX_NUM`-1 частот
///         или ошибка записи; -2, -3 — коды `read_S_matrix`.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#merge
int S_add( char *filename, char *filename1 );

#endif
