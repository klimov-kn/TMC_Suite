/*
**  TAMIC_soft(R) 1989,1990,1991 v1.00
**
** Macros and declarations for file_h program
**
** FILE: a\include\file.h
**
*/

/// @file _FILE.H
/// @brief Константы и объявления файлового меню текстового интерфейса TAMIC (DOS).
///
/// Часть модуля «Библиотека sfile95». Подключается только wrrd_s.cpp ради
/// `copy_file` и `FILENAME_MAX`. Объявленные здесь функции в исходниках пакета не
/// определены — это интерфейс старой DOS-оболочки, в Qt6-версии не используемый.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#legacy

#ifndef __FILE_H
  #define __FILE_H


/// Максимальная длина полного имени файла, если не задана `stdio.h` (DOS: 63).
#ifndef FILENAME_MAX
 #define FILENAME_MAX   63
#endif

/// Длина короткого имени DOS «8.3» с завершающим нулём.
#define FILENAME       13





/// @brief Копирует файл (вызывается из `copy_S_matrix`).
/// Определения в исходниках пакета нет; в сборке Qt6-версии не вызывается.
/// @param[in] source  исходный файл
/// @param[in] target  файл назначения
/// @return 0 — успех, иначе ошибка.
int  copy_file( char *source, char *target );
/// @brief Возвращает текущий диск.
/// Определения в исходниках пакета нет; в сборке Qt6-версии не вызывается.
/// @return Номер диска.
int  get_current_driver( void );
/// @brief Проверяет доступность диска.
/// Определения в исходниках пакета нет; в сборке Qt6-версии не вызывается.
/// @param[in] drive_code  буква диска
/// @return 0 — доступен, иначе код ошибки.
int  check_drive( char drive_code );
/// @brief Делает диск текущим.
/// Определения в исходниках пакета нет; в сборке Qt6-версии не вызывается.
/// @param[in] drive_code  буква диска
/// @return 0 — успех, иначе код ошибки.
int  set_default_drive( char drive_code );
/// @brief Возвращает текущий каталог диска.
/// Определения в исходниках пакета нет; в сборке Qt6-версии не вызывается.
/// @param[out] path  буфер для пути
/// @param[in] drive  буква диска
/// @return 0 — успех, иначе код ошибки.
int  get_current_dir( char *path, char drive);
/// @brief Возвращает полный путь текущего каталога диска.
/// Определения в исходниках пакета нет; в сборке Qt6-версии не вызывается.
/// @param[out] path_buff  буфер для пути
/// @param[in] drive  буква диска
/// @return 0 — успех, иначе код ошибки.
int  get_full_path( char *path_buff, char drive);
/// @brief Очищает окно списка файлов.
/// Определения в исходниках пакета нет; в сборке Qt6-версии не вызывается.
/// @param[in] l2_wndw  окно текстового интерфейса
void clr_file_list( WINDOW *l2_wndw );
/// @brief Рисует список файлов.
/// Определения в исходниках пакета нет; в сборке Qt6-версии не вызывается.
/// @param[in] l2_wndw  окно текстового интерфейса
/// @param[in] fl_l  список файлов
/// @param[in] fl_size  длина списка
/// @param[in] first_item  первый видимый элемент
/// @param[in] curr_item  текущий элемент
/// @param[in] prev  предыдущий элемент
/// @param[in] l4_wndw  окно полосы прокрутки
/// @return Код завершения.
int  drow_file_list( WINDOW *l2_wndw, char **fl_l,
                          int fl_size, int first_item, int curr_item, int prev,
                          WINDOW *l4_wndw);
/// @brief Очищает окно списка файлов.
/// Определения в исходниках пакета нет; в сборке Qt6-версии не вызывается.
/// @param[in] l2_wndw  окно текстового интерфейса
void clr_file_list( WINDOW *l2_wndw );
/// @brief Рисует список файлов.
/// Определения в исходниках пакета нет; в сборке Qt6-версии не вызывается.
/// @param[in] l2_wndw  окно текстового интерфейса
/// @param[in] fl_l  список файлов
/// @param[in] fl_size  длина списка
/// @param[in] first_item  первый видимый элемент
/// @param[in] curr_item  текущий элемент
/// @param[in] prev  предыдущий элемент
/// @param[in] l4_wndw  окно полосы прокрутки
/// @return Код завершения.
int  drow_file_list( WINDOW *l2_wndw, char **fl_l,
                          int fl_size, int first_item, int curr_item, int prev,
                          WINDOW *l4_wndw); 


#endif

