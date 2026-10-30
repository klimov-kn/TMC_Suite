/*
 *  TAMIC_soft(R) 1992 vR1.02
 *
 *  Unite_two_S - unite two scattering matrices in the new structure
 *
 *  FILE: ..\unite_s.c
 *
 *  OVERLAY: NO
*/

/// @file Unite_s.cpp
/// @brief Объединение частотных точек двух прочитанных S-матриц в один набор, упорядоченный по частоте.
///
/// Часть модуля «Библиотека sfile95». Используется `S_add` (sadd.cpp).
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#merge


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <Typedef.h>
#include <TMCLIMIT.H>
#include <COMPLEX1.H>
#include <TMC_LIB.H>
#include <FRQDATA.H>
#include <LISTING.H>
#include <S_file.h>
#include <Proc_s.h>

/// @brief Частотная точка для сортировки: частота, описание входов и матрица (указатель не копирует данные).
typedef struct
        {
         /// Частота, Гц.
         _real frq;
         /// Описание входов на этой частоте.
         INPUT_DATA id;
         /// Матрица этой частоты (владелец — исходный набор).
	 _complex   *sss;
         }FOR_SORT;

/// @brief Сравнивает две точки `FOR_SORT` по частоте для `qsort` (по возрастанию).
/// @param[in] fs1  первая точка
/// @param[in] fs2  вторая точка
/// @return 1, -1 или 0 — первая частота больше, меньше или равна второй.
int cmpgle( const void *fs1, const void *fs2 );

/// @brief Объединяет частотные точки двух наборов S-матриц без повторов, по возрастанию частоты.
///
/// Частоты первого набора, совпадающие (точно) с какой-либо частотой второго,
/// исключаются — остаётся точка второго набора. Признак исключения — частота 0
/// в `frq_out1->f`, поэтому массив первого набора портится. Матрицы не копируются:
/// в результат попадают указатели на матрицы исходных наборов.
/// @param[in,out] frq_out1  частоты первого набора, Гц; совпавшие обнуляются
/// @param[in]     id1       описания входов первого набора
/// @param[in]     sss1      матрицы первого набора
/// @param[in]     frq_out2  частоты второго набора, Гц
/// @param[in]     id2       описания входов второго набора
/// @param[in]     sss2      матрицы второго набора
/// @param[out]    frq_out   частоты результата (`f` выделяется, освобождает вызывающий `free_mem`); единицы — из глобальной таблицы
/// @param[out]    id        описания входов результата (выделяется, освобождает вызывающий)
/// @param[out]    sss       массив указателей на матрицы (выделяется; сами матрицы — исходных наборов)
/// @return 0 — успех; -1 — точек `FREQ_MAX_NUM` и больше или нет памяти.
/// @warning Точка с частотой ровно 0 Гц в первом наборе будет потеряна.
int  Unite_two_S( FREQUENCY *frq_out1, INPUT_DATA *id1, _complex **sss1,
				FREQUENCY *frq_out2, INPUT_DATA *id2, _complex **sss2,
				FREQUENCY *frq_out,  INPUT_DATA **id, _complex ***sss )
  {
   int i, j;
   int nFr1 = 0, nFr2 = 0;
   FOR_SORT *FSRT;

   /* find equal frequencies */
   for( i = 0; i < frq_out1->max; i++ )
     {
      for( j = 0; j < frq_out2->max; j++ )
        {
         if( frq_out1->f[i] == frq_out2->f[j] )
           {
            frq_out1->f[i] = (_real)0.0;
            break;
           }
        } //end of second frequency loop
     } //end first frequency loop

   /* count unique frequencies in the first structure */
   for( i = 0; i < frq_out1->max; i++ )
     {
      if( frq_out1->f[i] != (_real)0.0 ) nFr1++;
     }

   /* count unique frequencies in the second structure */
   for( i = 0; i < frq_out2->max; i++ )
     {
      if( frq_out2->f[i] != (_real)0.0 ) nFr2++;
     }
   frq_out->max = nFr1 + nFr2;
   if( frq_out->max >= FREQ_MAX_NUM )
     {
      return -1;
     }

  /* allocation memory for sorting according to frequency */
  if( (FSRT = (FOR_SORT *)alloc_mem_err( (long)sizeof(FOR_SORT)*(frq_out->max) ) ) == NULL)
    return -1;

  /* copy data for unique frequencies to struct to be sorted */
   for( i = 0, j = 0; i < frq_out1->max; i++ )
     {
      if( frq_out1->f[i] != (_real)0.0 )
        {
	 FSRT[j].frq = frq_out1->f[i];
	 FSRT[j].id  = id1[i];
	 FSRT[j].sss = sss1[i];
         j++;
        }
     }

   for( i = 0, j = nFr1; i < frq_out2->max; i++ )
     {
      if( frq_out2->f[i] != (_real)0.0 )
        {
	 FSRT[j].frq = frq_out2->f[i];
	 FSRT[j].id  = id2[i];
	 FSRT[j].sss = sss2[i];
         j++;
        }
     }

  /* sorting united frequencies points according to frequencies values */
  qsort( (void *)FSRT, (size_t)frq_out->max, sizeof( FOR_SORT ), &cmpgle );

   /* allocation of memory for frq buff */
   if( (frq_out->f = (_real *)alloc_mem_err( (long)sizeof(_real)*(frq_out->max) ) ) != NULL)
    {
     /* allocation of memory for input data structures */
     if( (*id = (INPUT_DATA *)alloc_mem_err( (long)sizeof(INPUT_DATA)*(frq_out->max) ) ) != NULL )
      {
       /* allocation of memory for pointers at scattering matrices */
       if( ((*sss) = (_complex **)alloc_mem_err( (long)sizeof(void *)*(frq_out->max) ) ) == NULL )
         {
          free_mem( *id );
          free_mem( frq_out->f );
          free_mem( FSRT );
          return -1;
         }
      } /* if id end */
     else
      {
       free_mem( frq_out->f );
       free_mem( FSRT );
       return -1;
      }
    }
   else
    {
     free_mem( FSRT );
     return -1;
    }

   /* copy data to united arrays */
   for( i = 0; i < frq_out->max; i++ )
    {
     frq_out->f[i] = FSRT[i].frq;
     (*id)[i]      = FSRT[i].id;
     (*sss)[i]     = FSRT[i].sss;
    }

    /* copy current frq unit to read struct */
   for( i = 0; i < 4; i++ )
    {
     frq_out->unit[i] =  get_frq_unit(i);
     frq_out->item[i] =  get_frq_item(i);
     }
   frq_out->code = get_frq_item_code();

   free_mem( FSRT );
   return 0;
  } //end of Unite_two_S routine


int cmpgle( const void *fs1, const void *fs2 )
  {
	FOR_SORT *FSRT1 = (FOR_SORT *)fs1;
	FOR_SORT *FSRT2 = (FOR_SORT *)fs2;
   if( FSRT1->frq > FSRT2->frq )       return 1;
   else if( FSRT1->frq < FSRT2->frq )  return -1;
   else                                  return 0;
  }
