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

#include <stdio.h>
#include <string.h>

#ifndef _HIGH_ACCURACY
 #define _HIGH_ACCURACY
#endif

#include <typedef.h>
#include <tmclimit.h>
#include <tmc_lib.h>
#include <frqdata.h>
#include <error1.h>
#include <s_file.h>


int _far _fortran update_offset_table( _real frq, int num, int nTBL_Flag )
 {
  int i;
  int nPS_Number = Get_PS_Number();
  long     lOffset;
  fpos_t   fPos_t;
  S_OFFSET s_Offset;
//  int n_Offset;
  FILE _far *sf = Get_S_FILE();
  INT_1 nBuf;

  if( fseek( sf, 0L, SEEK_END ) != 0 ) return -1;     /* position to the file end */
  if( fgetpos( sf,  &fPos_t ) != 0 ) return -1;       /* get end's offset */
    /* skipping TITLE */
  lOffset = (fpos_t)(8*sizeof( char ) +  3*sizeof( INT_1 ) );   /* title offset */
  if( fseek( sf, lOffset, SEEK_SET ) != 0 ) return -1;

    /* skipping PS before offset table */
  for( i = ( nTBL_Flag == OFF_TBL_MAIN)?2:1; i < nPS_Number; i++ )
    if( skip_string() != 0 ) return -1;

   /* skipping length of the offset table */
  if( fseek( sf, (long)sizeof(WORD), SEEK_CUR ) != 0 ) return -1;

  num++;
  nBuf = num;
//  if( write_S( &num, sizeof( WORD ) ) != 0 ) return -1; /* write frq num */
  if( write_S( &nBuf, sizeof( INT_1 ) ) != 0 ) return -1; /* write frq num */
  num--;

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


