/*
**  TAMIC_soft(R) 1991 v1.00
**
**  Frequency data table declaration
**
**  FILE : a:\include\frqdata.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

/// @file FRQDATA.H
/// @brief Таблица частот: структура `FREQUENCY`, единицы частоты и функции доступа к глобальной таблице.
///
/// Часть модуля «Библиотека sfile95». Глобальная таблица (frqdata.cpp) задаёт
/// частоты, на которых пишется S-файл, номер текущей частоты и единицы для ввода
/// и вывода. Частоты в таблице всегда хранятся в Гц; «текущие единицы» (по
/// умолчанию ГГц) применяются только при задании и чтении через функции с
/// флагом единиц.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#frequency-table

#ifndef _FRQDATA_H
 #define _FRQDATA_H


                                                 /* frequency struct        */
/// @brief Набор частотных точек с текущей точкой и единицами частоты.
///
/// Используется как глобальная таблица библиотеки (одна на процесс, frqdata.cpp)
/// и как результат `read_S_matrix` (тогда `f` выделяет функция, освобождает вызывающий).
/// @note Глобальная таблица не потокобезопасна.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#frequency-table
typedef struct
           {
            /// Номер текущей частоты (с 0).
            int num;                             /* current frequency point */
            /// Число частотных точек; у глобальной таблицы — не больше `FREQ_MAX_NUM`.
            int max;                             /* maximal frequency point */
            /// Массив частот, Гц, длиной `max`.
            _real *f;                       /* frequency point array   */
            /// Код текущих единиц: индекс в `unit`/`item` (`_Hz`..`_GHz`), по умолчанию 3 (ГГц).
            int code;                            /* unit code               */
            /// Множители единиц, Гц на единицу: 1, 1e3, 1e6, 1e9.
            _real unit[4];                       /* unit                    */
            /// Подписи единиц: "_Hz", "KHz", "MHz", "GHz".
            const char *item[4];                  /* unit items strings      */
            }FREQUENCY;

/// Коды единиц частоты — индексы в `FREQUENCY::unit` и `FREQUENCY::item`.
enum freq { _Hz, _KHz, _MHz, _GHz };             /* units numbers           */


 /* return first frequency array element (F0) */

 /* return frequency increement (dF) */

/// @brief Возвращает число частотных точек таблицы.
/// @return `max`.
int   get_frq_nf( void );
 /* return max. frequency point number */

/// @brief Задаёт число частотных точек таблицы.
/// @param[in] num  число точек, 1..`FREQ_MAX_NUM`
/// @return 0 — успех; -1 — число вне диапазона (таблица не меняется).
int   set_frq_nf( int num ); /* == f[index] in Hz !!!*/

/// @brief Записывает частоту точки `num` как есть (в Гц).
/// @param[in] fValue  частота, Гц
/// @param[in] num     номер точки (с 0), без проверки границ
void  set_frq_element( _real fValue, int num );  /* f[num] = fValue*/

/// @brief Заполняет таблицу равномерной сеткой частот `f0 + k*df`, k = 0..nf-1.
/// @param[in] f0  первая частота в текущих единицах
/// @param[in] df  шаг в текущих единицах
/// @param[in] nf  число точек, не больше `FREQ_MAX_NUM` (не проверяется)
/// @return Всегда 0.
int   set_frequency( _real f0, _real df, int nf );

/// @brief Возвращает подпись единиц с кодом `num` ("_Hz", "KHz", "MHz", "GHz").
/// @param[in] num  код единиц 0..3
/// @return Указатель на статическую строку.
const char  * get_frq_item( int num );
/// @brief Возвращает множитель единиц с кодом `num`, Гц на единицу.
/// @param[in] num  код единиц 0..3
/// @return 1, 1e3, 1e6 или 1e9.
_real get_frq_unit( int num );
/// @brief Возвращает код текущих единиц частоты.
/// @return 0..3.
int   get_frq_item_code( void );

/// @brief Возвращает номер текущей частоты.
/// @return Номер (с 0).
int   get_current_frq_num( void );
/// @brief Задаёт номер текущей частоты; от него зависит, создаёт ли `save_S_matrix` файл заново (номер 0).
/// @param[in] number  номер (с 0)
void  set_current_frq_num( int number );
/// @brief Возвращает текущую частоту, Гц, как `double` (для записи в S-файл).
/// @return `f[num]`.
double get_current_dbl_frq( void );

#endif
