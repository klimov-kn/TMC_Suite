/*
**  TAMIC_soft(R) 1991 v1.00
**
**  tpl_file.h
**
**  FILE : a:\include\tpl_file.c
**
**  LIB  :
**
**  OVERLAY:
**
*/

/// @file TPL_FILE.H
/// @brief Построчное чтение файла задания .tpl в DOS-версии TAMIC.
///
/// Включается препроцессором и sfile95. В Qt6-версии функции не определены
/// (`fget_tpl_line` — только в prepr/dr_buf.cpp вне сборки); вызовы `get_tpl_file_name`
/// есть лишь в закомментированном или неиспользуемом коде. Задание читает TMCIndan.
/// @see https://www.tamic.ru/docs/code/shared-headers.html#legacy-api
#ifndef _TPL_FILE_H
 #define _TPL_FILE_H




/// @brief Возвращает имя файла задания.
/// @return Имя файла.
/// @note Определения в исходниках пакета нет (объявление TAMIC); вызов есть только в неиспользуемой функции sfile95 `make_S_matrix_name`.
char *get_tpl_file_name( void );

/// @brief Читает очередную строку задания.
/// @param[out] str_buff  буфер строки
/// @return Указатель на буфер или NULL в конце файла.
/// @note Определена в prepr/dr_buf.cpp, который в сборку Qt6-версии не входит.
char * fget_tpl_line( char *str_buff );
/// @brief Читает очередную строку задания, пропуская пустые и комментарии.
/// @param[out] str_buff  буфер строки
/// @return Указатель на буфер или NULL.
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
char *          get_tpl_line( char *str_buff );

/// @brief Разбирает параметры строки задания по формату группы.
/// @param[in] code  код группы
/// @param[in] string  строка
/// @param[out] ...  адреса переменных
/// @return Число прочитанных параметров.
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
int  tpl_str_scan( Uint code, char *string, ... );
/// Повторное объявление `tpl_str_scan` (см. выше).
int   tpl_str_scan( Uint code, char *string, ... );
/// @brief Возвращает флаг эхо-вывода файлов `#include`.
/// @return Значение.
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
int   get_include_echo_flag( void );
#endif
