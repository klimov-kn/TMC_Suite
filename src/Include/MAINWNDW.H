/*
**  TAMIC_soft(R) 1991 v1.00
**
** Macros and declarations for mainwndw program.
**
** FILE: a\include\mainwndw.h
**
*/

/// @file MAINWNDW.H
/// @brief Главное текстовое окно DOS-версии TAMIC: цвета, окно статистики, вывод хода расчёта.
///
/// Включается препроцессором и sfile95. Из функций определена только заглушка
/// `stat_messege` (sfile95, s_memf.cpp); остальные в исходниках пакета отсутствуют.
/// Ход расчёта в Qt6-версии показывает строка состояния окна ядра.
/// @see https://www.tamic.ru/docs/code/shared-headers.html#legacy-api
#ifndef _MAINWNDW_H
  #define _MAINWNDW_H



/// Аргумент `main_window`: без паузы.
#define NO_PAUSE    0
/// Аргумент `main_window`: с паузой.
#define YES_PAUSE   1

/// @brief Рисует главное окно программы.
/// @param[in] pause_flag  `NO_PAUSE` или `YES_PAUSE`
/// @param[in] daun_messge_flag  ненулевое — показать нижнюю строку сообщений
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
void main_window( int pause_flag, int daun_messge_flag );




/* ETIME collides with the ETIME macro from modern <errno.h> (UCRT);
   undef it before the enum so the enumerator compiles. win32/win64 build fix. */
#ifdef ETIME
#undef ETIME
#endif

/// @brief Пункты окна статистики DOS-версии (индекс для `stat_messege`, `put_stat_num`).
enum ST_ITEMS
           {
            /// Имя файла.
            SFILE,
            /// Номер шага.
            STEP,
            /// Частота.
            FREQ,
            /// Номер связи.
            ST_LINK,
            /// Номер блока.
            ST_BLOCK,
            /// Строка трассировки.
            TRACE,
            /// Прошедшее время.
            ETIME,
            /// Оценка оставшегося времени.
            ESTIME,
            /// Свободная память.
            FREEMEM
            };


/// Показывает строку трассировки в окне статистики.
#define put_trace( string)   stat_messege( TRACE, string )

/// @brief Показывает строку в пункте окна статистики.
/// @param[in] stat_item_num  пункт `ST_ITEMS`
/// @param[in] string  текст
/// @note Определена в sfile95 (s_memf.cpp) как пустая заглушка.
void stat_messege( int stat_item_num, char *string);
/// @brief Показывает число в пункте окна статистики.
/// @param[in] stat_item_num  пункт `ST_ITEMS`
/// @param[in] put_num  число
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
void put_stat_num( int stat_item_num, int put_num );

/// @brief Возвращает объём свободной памяти DOS.
/// @return Свободная память (килобайты или параграфы — зависит от варианта DOS).
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
  int       get_dos_freemem( void );

#endif
