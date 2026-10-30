/*
**  TAMIC_soft(R) 1991 v1.00
**
** Macros and decloration for TAMIC_soft standart library.
**
** FILE: a:\include\tmc_lib.h
*/

/// @file TMC_LIB.H
/// @brief Стандартная библиотека TAMIC: память, имена файлов, диски, допуски, геометрия, служебный вывод.
///
/// Используются функции памяти `alloc_mem_err`, `free_mem`, `free_mem_null`
/// (определены в sfile95, s_memf.cpp, — обёртки над `malloc`/`free`) — их вызывают
/// sfile95 и препроцессор. Остальные объявления — наследие DOS-версии: в исходниках
/// пакета определений нет.
/// @see https://www.tamic.ru/docs/code/shared-headers.html#tmc-lib
#ifndef _TMC_LIB_H
 #define _TMC_LIB_H


/// Глубина стека блоков памяти `memory_stack`.
#define MEM_STACK_SIZE   10

/// @brief Выделенный блок памяти: размер и адрес. В коде Qt6-версии не используется.
struct memory_block
            {
             /// Размер блока, байт.
             int size;
	     /// Адрес блока.
	     void *ptr;
             };

/// @brief Стек выделенных блоков памяти. В коде Qt6-версии не используется.
struct memory_stack
            {
             /// Блоки стека.
             struct memory_block mem_blk[MEM_STACK_SIZE];
             /// Вершина стека.
             struct memory_block *sp;
             };


/// @brief Возвращает объём свободной памяти DOS.
/// @return Объём свободной памяти.
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
  int       get_dos_freemem( void );
/// @brief Выделяет память через `malloc`; при нехватке вызывает `put_error_messege(FATAL_MEM_ALLOC_ERROR)`.
/// @param[in] size  размер, байт
/// @return Адрес блока или NULL при нехватке памяти; освобождать `free_mem`.
/// @note Определена в sfile95 (s_memf.cpp). Сообщение об ошибке в Qt6-версии не выводится (заглушка), проверка на NULL обязательна.
void *alloc_mem_err( long size );
/// @brief Освобождает память, выделенную `alloc_mem_err` (вызывает `free`).
/// @param[in] mem_ptr  адрес блока; NULL допустим
/// @return Всегда 0.
/// @note Определена в sfile95 (s_memf.cpp).
int  free_mem( void *mem_ptr );
/// @brief Освобождает память и обнуляет указатель.
/// @param[in,out] ptr  адрес указателя на блок; если `*ptr == NULL`, ничего не делает
/// @return Всегда 0.
/// @note Определена в sfile95 (s_memf.cpp).
int  free_mem_null( void **ptr );
/// @brief Заменяет расширение имени файла.
/// @param[out] new_name  буфер результата
/// @param[in] base_name  исходное имя
/// @param[in] extension  новое расширение без точки
/// @return Указатель на `new_name`.
/// @note Определения в исходниках пакета нет (объявление TAMIC); вызовы есть только в неиспользуемых функциях sfile95.
char *change_file_extension( char *new_name, const char *base_name, const char *extension );

/// @brief Возвращает текущий диск.
/// @return Номер диска.
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
int  get_current_driver( void );
/// @brief Делает диск текущим.
/// @param[in] drive_code  буква диска
/// @return 0 — успех.
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
int  set_default_drive( char drive_code );
/// @brief Возвращает текущую папку диска.
/// @param[out] path  буфер пути
/// @param[in] drive  буква диска
/// @return 0 — успех.
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
int  get_current_dir( char *path, char drive );
/// @brief Проверяет доступность диска.
/// @param[in] driver_code  буква диска
/// @return 0 — диск доступен.
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
int  check_drive( char driver_code );
/// @brief Возвращает полный путь текущей папки диска.
/// @param[out] path_buff  буфер пути
/// @param[in] drive  буква диска
/// @return 0 — успех.
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
int  get_full_path( char *path_buff, char drive );

/// @brief Удаляет пробелы из строки на месте.
/// @param[in,out] string  строка
/// @return Указатель на строку.
/// @note Определения свободной функции в исходниках пакета нет; одноимённый метод есть у `CTmcRTH_Indan` (TMCIndan).
char  * del_blanks( char *string );


/*  real function Angle( x1, x2, x3, y1, y2, y3 ) */
/// @brief Вычисляет угол по трём точкам (подпрограмма TAMIC на Фортране).
/// @param[in] x1  x первой точки
/// @param[in] x2  x второй точки
/// @param[in] x3  x третьей точки
/// @param[in] y1  y первой точки
/// @param[in] y2  y второй точки
/// @param[in] y3  y третьей точки
/// @return Угол, радианы.
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
_real Angle( _real *x1, _real *x2, _real *x3,
			 _real *y1, _real *y2, _real *y3 );
/// @brief Выводит строку в последнюю строку текстового экрана.
/// @param[in] TraceMessage  текст
/// @note Определена в sfile95/outtrace.cpp, который в сборку Qt6-версии не входит.
void  OutTrace( char *TraceMessage);


#endif
