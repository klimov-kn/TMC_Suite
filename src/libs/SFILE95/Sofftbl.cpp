/*
**  TAMIC_soft(R) 1989,1991,1992 vR1.02
**
**  update_offset_table - fill matrices offset table and set position at the end
**
**  FILE : ..\source\tamic\post_pro\sofftbl.c
**
**  LIB  : ooutpXY.lib
**
**  OVERLAY: YES
**
*/

/// @file Sofftbl.cpp
/// @brief Занесение частоты и смещения блока в таблицу смещений S-файла (`update_offset_table`).
///
/// Часть модуля «Библиотека sfile95». Функция объявлена и описана в `s_file.h`.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#offset-table

#include <stdio.h>
#include <string.h>

#ifndef _HIGH_ACCURACY
 #define _HIGH_ACCURACY
#endif

#include <Typedef.h>
#include <TMCLIMIT.H>
#include <TMC_LIB.H>
#include <FRQDATA.H>
#include <Error1.h>
#include <S_file.h>


int update_offset_table( _real frq, int num, int nTBL_Flag )
 {
  int i;
  int nPS_Number = Get_PS_Number();
  long     lOffset;
  long     fPos_t;
  S_OFFSET s_Offset;
//  int n_Offset;
  FILE *sf = Get_S_FILE();
  INT_1 nBuf;

  if( fseek( sf, 0L, SEEK_END ) != 0 ) return -1;     /* position to the file end */
  if( (fPos_t = ftell( sf )) == -1L ) return -1;      /* get end's offset */
    /* skipping TITLE */
  lOffset = (long)(8*sizeof( char ) +  3*sizeof( INT_1 ) );   /* title offset */
  if( fseek( sf, lOffset, SEEK_SET ) != 0 ) return -1;

    /* skipping PS before offset table */
  for( i = ( nTBL_Flag == OFF_TBL_MAIN)?2:1; i < nPS_Number; i++ )
    if( skip_string() != 0 ) return -1;

   /* skipping length of the offset table */
  if( fseek( sf, (long)sizeof(WORD), SEEK_CUR ) != 0 ) return -1;

  // в шапку таблицы пишется число уже записанных частот (номер + 1)
  num++;
  nBuf = num;
  if( write_S( &nBuf, sizeof( INT_1 ) ) != 0 ) return -1; /* write frq num */
  num--;

  // запись номер num: 8 байт частоты + 4 байта смещения
  lOffset =  num*(sizeof(_real)+sizeof(int));  /* S_OFFSET offset */
  s_Offset.frq     = frq;
  s_Offset.lOffset = (int)fPos_t;
  if( fseek( sf, lOffset, SEEK_CUR ) != 0 ) return -1;
//  if( write_S( &s_Offset, sizeof(S_OFFSET) ) != 0 ) return -1; /* write s-matrix offset for the frq value */
  if( write_S( &frq, sizeof(_real) ) != 0 ) return -1; /* write s-matrix offset for the frq value */
  i = (int)fPos_t;
  if( write_S( &i, sizeof(int) ) != 0 ) return -1; /* write s-matrix offset for the frq value */
//  n_Offset = s_Offset;
//  if( write_S( &n_Offset, sizeof(int) ) != 0 ) return -1; /* write s-matrix offset for the frq value */

  if( fseek( sf, 0L, SEEK_END ) != 0 ) return -1;  /* position to the file end */
  return 0;
  }


