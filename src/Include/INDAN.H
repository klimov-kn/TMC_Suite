/*
**  TAMIC_soft(R) 1989,1990,1991,1992 v1.00
**
**  indan.h
**
**  FILE : a:\include\indan.c
**
**  LIB  :
**
**  OVERLAY:
**
*/

/// @file INDAN.H
/// @brief Ввод данных задания TAMIC (загрузка шага, атрибуты, вывод, внутренняя база).
///
/// Наследие DOS-версии. Включается препроцессором (defmis.cpp, incpath.cpp), но
/// в Qt6-версии функции не определены (кроме `SetIncludePathName`/`GetIncludePathName`
/// в incpath.cpp, который в сборку не входит). Ввод задания ядер — библиотека TMCIndan.
/// @see https://www.tamic.ru/docs/code/shared-headers.html#legacy-api
#ifndef _INDAN_H
 #define _INDAN_H

/// Режим вывода `set_output_mode`: листинг.
#define LISTING_MODE    0
/// Режим вывода `set_output_mode`: сохранение параметров.
#define SAVE_P_MODE     1


/// @brief Задаёт режим вывода.
/// @param[in] mode  `LISTING_MODE` или `SAVE_P_MODE`
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
void  set_output_mode( int mode );

/// @brief Запоминает путь поиска файлов `#include`.
/// @param[in] ptr  путь
/// @note Определена в prepr/incpath.cpp, который в сборку Qt6-версии не входит.
void  SetIncludePathName( char *ptr );
/// @brief Возвращает путь поиска файлов `#include`.
/// @return Путь.
/// @note Определена в prepr/incpath.cpp, который в сборку Qt6-версии не входит.
char  * GetIncludePathName( void );
#endif
