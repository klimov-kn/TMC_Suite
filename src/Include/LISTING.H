/*
**  TAMIC_soft(R) 1989 v1.00
**
** Macros and declaration for listing output programs.
**
** FILE: \include\listing.h
**
*/

/// @file LISTING.H
/// @brief Листинг — постраничный текстовый отчёт `<имя>.lst` утилит TAMIC: структура и функции вывода.
///
/// Часть модуля «Библиотека sfile95». Функции реализованы в listing.cpp, который
/// в сборку Qt6-версии не входит; в сборке определений нет. Заголовок подключают
/// s_table.cpp (`put_S_table`, в программах не вызывается) и unite_s.cpp.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#legacy

#ifndef _LISTING_H
  #define _LISTING_H



/// Режим `open_listing_file`: начать файл листинга заново ("w"), сбросить счётчики строк и страниц.
#define WR_BEG          1
/// Режим `open_listing_file`: дописывать в конец файла листинга ("a").
#define WR_END          0

/// @brief Открывает файл листинга.
/// @param[in] open_mode_flag  `WR_BEG` — создать заново, `WR_END` — дописывать
/// @return 0 — успех; -1 — файл уже открыт или не открылся.
int  open_listing_file( int open_mode_flag );
/// @brief Возвращает флаг эха листинга. Определения в исходниках пакета нет.
/// @return Значение флага.
int  get_listing_echo_flag( void );

/// @brief Выводит в листинг текст, разбивая его по символам перевода строки.
/// @param[in,out] string  текст; символы '\n' в нём заменяются на '\0'
/// @return Номер последней строки листинга; -1 — файл не открыт.
int  put_listing_string( char *string );
/// @brief Форматирует строку как `printf` и выводит её в листинг.
/// @param[in] format_string  формат; результат не длиннее 254 символов
/// @param[in] ...            аргументы формата
/// @return Как у `put_listing_string`.
int  l_printf( char *format_string, ...);


#endif
