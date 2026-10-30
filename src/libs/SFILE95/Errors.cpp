/*
 *  TAMIC_soft(R) 1992 vR1.02
 *
 *  put_error_messege - dummy error processing routine.
 *
 *  FILE: ..\errors.c
 *
 *  OVERLAY: NO
*/

/// @file Errors.cpp
/// @brief Заглушка `put_error_messege(int, ...)` и тексты сообщений об ошибках S-файлов.
///
/// Часть модуля «Библиотека sfile95». Тело функции закомментировано: в сборке
/// Qt6-версии сообщения не выводятся, функция всегда возвращает 0. Об ошибках
/// вызывающий узнаёт только по кодам возврата.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#pitfalls


#include <stdio.h>
#include <stdarg.h>

#include <Typedef.h>
#include <Error1.h>
#include "ERROR2.H"


/// Текст ошибки `FATAL_MEM_ALLOC_ERROR`: не хватает памяти.
char msg1[] = "Fatal ERROR.\nNot enough memory to run program.\nRemove resident programs and restart system. ";
/// Текст ошибки `ERROR_OPEN_LISTING`: не открывается файл листинга (%s — имя).
char msg2[] = "Error !\nCan not open listing output file:\n%s!";
/// Текст предупреждения `SMT_FRQ_MEMORY`: памяти хватило не на все частоты (%d — сколько прочитано).
char msg3[] = "Warning !\nNot enough memory for all frequency points!\nOnly %d frequencies processed!";
/// Текст ошибки `ERROR_OPEN_S`: не открывается S-файл (%s — имя).
char msg4[] = "Error !\nCan't open S-matrix data file\n%s!";
/// Текст ошибки `ERROR_READ_S`: ошибка чтения S-файла (%s — имя).
char msg5[] = "Error while reading from S-file:\n%s!";
/// Текст ошибки `SMT_LABEL_ERROR`: файл не является S-файлом (%s — имя).
char msg6[] = "Error !\nThis is not scattering matrix file:\n%s!";
/// Текст ошибки `SMT_VERSION_ERROR`: версия S-файла младше 102 (%s — имя).
char msg7[] = "Error !\nToo old scattering matrix file:\n%s!";
/// Текст ошибки `DIFF_INPUT_NUM`: разное число входов.
char msg8[] = "Error !\nDifferent number of inputs! See listing.";
/// Текст ошибки `DIFF_MODE_NUM`: разное число мод.
char msg9[] = "Error !\nDifferent number of modes! See listing.";
/// Текст ошибки `ERROR_OPEN_S_T`: не открывается файл текстовой таблицы (%s — имя).
char msg10[] = "Error !\nCan't open file for text table\n%s!";

int  put_error_messege( int err_num, ... )
  {
/*   va_list  argptr;                      // Argument list pointer
//   char _far *msg;
   char buff[512];

   va_start( argptr, err_num );       // Initialize va_ functions

   switch ( err_num )
     {
      case FATAL_MEM_ALLOC_ERROR:
                                 vsprintf( buff, msg1, argptr );
                              break;
      case ERROR_OPEN_LISTING:
                                 vsprintf( buff, msg2, argptr );
                              break;
      case SMT_FRQ_MEMORY:
                                 vsprintf( buff, msg3, argptr );
                              break;
      case ERROR_OPEN_S:
                                 vsprintf( buff, msg4, argptr );
                              break;
      case ERROR_READ_S:
                                 vsprintf( buff, msg5, argptr );
                              break;
      case SMT_LABEL_ERROR:
                                 vsprintf( buff, msg6, argptr );
                              break;
      case SMT_VERSION_ERROR:
                                 vsprintf( buff, msg7, argptr );
                              break;
      case DIFF_INPUT_NUM:
                                 vsprintf( buff, msg8, argptr );
                              break;
      case DIFF_MODE_NUM:
                                 vsprintf( buff, msg9, argptr );
                              break;
      case ERROR_OPEN_S_T:
                                 vsprintf( buff, msg10, argptr );
                              break;
      default:
                              va_end( argptr );
                              return -1;
                              break;
     }
   va_end( argptr );                     // Close va_ functions

*/   return 0;
  }
