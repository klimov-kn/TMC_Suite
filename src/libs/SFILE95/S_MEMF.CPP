/*
 *  TAMIC_soft(R) 1992 vR1.02
 *
 *  processor - processing routine for reading s-files
 *
 *  FILE: ..\sread.c
 *
 *  OVERLAY: NO
*/

/// @file S_MEMF.CPP
/// @brief Выделение и освобождение памяти для sfile95 и заглушки служебных функций TAMIC.
///
/// Часть модуля «Библиотека sfile95». `alloc_mem_err`, `free_mem`, `free_mem_null`
/// (объявлены в `tmc_lib.h`) — обёртки `malloc`/`free`; ими пользуется и библиотека
/// prepr. `stat_messege` (строка состояния, через макрос `put_trace`) и
/// `get_comp_file_flag` (признак «составного» файла) — заглушки: ничего не делают
/// и возвращают 0.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#memory


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include <Typedef.h>
#include <TMCLIMIT.H>
#include <COMPLEX1.H>
#include <TMC_LIB.H>
#include <FRQDATA.H>
#include <S_file.h>
#include <Error1.h>
#include <Proc_s.h>


#include <malloc.h>

void *  alloc_mem_err( long size )
  {
   void *ptr;
   ptr = (void *)malloc( (size_t)size );
   if( ptr == NULL ) put_error_messege( FATAL_MEM_ALLOC_ERROR );
   return ( ptr );
  }

int  free_mem( void *mem_ptr )
  {
   free( mem_ptr );
   return 0;
  }

int  free_mem_null( void **ptr )
  {
   if( *ptr != NULL )
     {
      free( *ptr );
      *ptr = NULL;
     }
   return 0;
  }

void stat_messege( int num, char *mes ) { return; }

