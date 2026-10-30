/*
 *  TAMIC_soft(R) 1992 vR1.02
 *
 *  processor - processing routine for adding S-files
 *
 *  FILE: ..\sadd.c
 *
 *  OVERLAY: NO
*/

/// @file Sadd.cpp
/// @brief Объединение двух S-файлов в один (`S_add`) и вспомогательные функции проверки и освобождения.
///
/// Часть модуля «Библиотека sfile95». `S_add` объявлена и описана в `proc_s.h`;
/// её вызывает `save_S_matrix_element`, когда добавляется новая частота.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#merge


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include <Typedef.h>
#include <TMCLIMIT.H>
#include <COMPLEX1.H>
#include <TMC_LIB.H>
#include <FRQDATA.H>
#include <S_file.h>
#include <MATRIX.H>
#include <Error1.h>	/* was <error.h>: error.h missing, error codes live in Error1.h */
#include <MAINWNDW.H>
#include <Proc_s.h>

//int  _far _fortran write_S_to_listing( S_INDEX _far *si, int extrapol_flag, _complex _far **sss,  FREQUENCY _far *frq_out, INPUT_DATA _far *id );
// Предварительное объявление; описание — у определения ниже.
int  Compare_S_ind( S_INDEX *si1, int extrapol_flag1, S_INDEX *si2, int extrapol_flag2 );
// Предварительное объявление; определение и описание — unite_s.cpp.
int  Unite_two_S( FREQUENCY *frq_out1, INPUT_DATA *id1, _complex **sss1,
				FREQUENCY *frq_out2, INPUT_DATA *id2, _complex **sss2,
				FREQUENCY *frq_out,  INPUT_DATA **id, _complex ***sss );
// Предварительное объявление; описание — у определения ниже.
void Free_S( INPUT_DATA *id, S_INDEX **si, _real *f,
                           _complex ***sss, int max, int extrapol_flag );
/// @brief Интерполяция S-матрицы (наследие TAMIC). Объявлена, но в исходниках пакета не определена и не вызывается.
/// @param[in] szFile  имя S-файла
/// @return Код завершения.
int Interpolate_S_matrix( char *szFile );


/// @name Тексты сообщений утилиты TAMIC
/// Тексты строки состояния и итоговых сообщений утилиты TAMIC (в сборке не используются).
///@{
/** Текст строки состояния или итогового сообщения утилиты TAMIC (в сборке не используется). */
static char szTrace1[] = "Reading first matrix ...";
static char szTrace2[] = "Reading second matrix ...";
static char szTrace3[] = "Saving data at frequency %10.3g GHz.";
static char szTrace4[] = "Interpolating S-matrix ...";
static char szFin1[] = "S-matrices have been united normally.";
static char szFin2[] = "S-matrix have been read normally.";
///@}

int S_add( char *filename, char *filename1 )
  {
   int ret_code = 0, i;

   int extrapol_flag;
   char *szFile = filename;
   _complex   **sss;
   INPUT_DATA *id;
   S_INDEX    *si;
   FREQUENCY  frq_out;

   int extrapol_flag1;
   char *szFile1 = filename;
   _complex   **sss1;
   INPUT_DATA *id1;
   S_INDEX    *si1;
   FREQUENCY  frq_out1;

   int extrapol_flag2;
   char *szFile2 = filename1;
   _complex   **sss2;
   INPUT_DATA *id2;
   S_INDEX    *si2;
   FREQUENCY  frq_out2;

   // результат пишется в первый файл: szFile и szFile1 указывают на одно имя
   /* reading first scattering matrix */
   ret_code = read_S_matrix( szFile1, &si1, &extrapol_flag1, &sss1,  &frq_out1, &id1 );

   if( szFile2 != NULL )
     {
      /* reading second scattering matrix */
      if( ret_code == 0 ) ret_code = read_S_matrix( szFile2, &si2, &extrapol_flag2, &sss2,  &frq_out2, &id2 );

      if( ret_code == 0 )
       {
        /* check is it possible to write both scattering matrices in one file */
        ret_code = Compare_S_ind( si1, extrapol_flag1, si2, extrapol_flag2 );

        /* unite both scattering matrix in one structure */
        if( ret_code == 0 ) ret_code = Unite_two_S( &frq_out1, id1, sss1,
                                                    &frq_out2, id2, sss2,
                                                    &frq_out, &id, &sss );
        if( ret_code == 0 )
         {
          extrapol_flag = extrapol_flag1;
          si = si1;

          /* rename first file if there is not third parameter in the string */
          if( strcmp( szFile1, szFile ) == 0 )
            {
            }

          /* write united matrix to the file */
          // глобальная таблица частот получает объединённые частоты; точка 0 пересоздаёт файл
          set_frq_nf( frq_out.max );
          for( i = 0; ( i < frq_out.max )&&( ret_code == 0 ); i++ )
            {
             set_current_frq_num( i );
             set_frq_element( frq_out.f[i], i );
             if( ret_code == 0 ) ret_code = save_S_matrix( sss[i], &id[i], szFile, si, extrapol_flag );	// A-3: id[i]
            }

          /* write united matrix to the listing */
          free_mem( frq_out.f );
          free_mem( id );
          free_mem( sss );
         }
        Free_S( id2, &si2, frq_out2.f, &sss2, frq_out2.max, extrapol_flag2 );
		Free_S( id1, &si1, frq_out1.f, &sss1, frq_out1.max, extrapol_flag1 );
       }
      else
       {
		Free_S( id1, &si1, frq_out1.f, &sss1, frq_out1.max, extrapol_flag1 );
       }
     }
   else
     {
      if( ret_code == 0 )
       {
		Free_S( id1, &si1, frq_out1.f, &sss1, frq_out1.max, extrapol_flag1 );
       }
     }

   return ret_code;
  }

/// @brief Освобождает все результаты `read_S_matrix` для одного файла.
/// @param[in]     id             массив описаний входов (`free_mem`)
/// @param[in,out] si             индексы сетки и имена (`Free_GRID_Index`); после вызова NULL
/// @param[in]     f              массив частот (`free_mem`)
/// @param[in,out] sss            матрицы (`Free_S_read`); после вызова NULL
/// @param[in]     max            число частот
/// @param[in]     extrapol_flag  число сеток минус один
void Free_S( INPUT_DATA *id, S_INDEX **si, _real *f,
                           _complex ***sss, int max, int extrapol_flag )
  {
   free_mem( id );
   free_mem( f );
   Free_S_read( sss, max );
   Free_GRID_Index( si, extrapol_flag );
  }

/// @brief Проверяет, можно ли записать матрицы двух S-файлов в один файл.
///
/// Требуется одинаковое число сеток; при одной сетке (`extrapol_flag` == 0) — ещё и
/// точное равенство шагов `delta1..3`. При нескольких сетках шаги не сравниваются.
/// @param[in] si1             индексы сетки первого файла
/// @param[in] extrapol_flag1  число сеток первого файла минус один
/// @param[in] si2             индексы сетки второго файла
/// @param[in] extrapol_flag2  число сеток второго файла минус один
/// @return 0 — совместимы; -1 — нет.
int Compare_S_ind( S_INDEX *si1, int extrapol_flag1, S_INDEX *si2, int extrapol_flag2 )
 {
  if( extrapol_flag1 != extrapol_flag2 )
    {
     return -1;
    }
  else
    {
     if( extrapol_flag1 != 0 ) return 0;
     else
       {
        if( ( si1->delta1 != si2->delta1 )||
            ( si1->delta2 != si2->delta2 )||
            ( si1->delta3 != si2->delta3 ) )
           {
            return -1;
           }
       }
    }
  return 0;
 }


