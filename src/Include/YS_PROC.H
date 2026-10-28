/*
**  TAMIC_soft(R) 1991 v1.00
**
** Macros and decloration for ys_proc programm
**
** FILE: a\tmc_norm\ys_proc.h
**
*/

#include <frqdata.h>
#include <s_file.h>

#ifndef _YS_PROC_H
  #define _YS_PROC_H

#define REAL_IMAG  0
#define ARG_PHASE  1
#define VSWR_LOSS  2

int  set_file_name( char *name );
char * get_S_file_name( void );

void Set_OutTextFlag( int nFlag );
int  Get_OutTextFlag( void );

void ys_proc( void );
int _S_to_Y( _complex *sm, float *ymatr, int dim );

int  put_S_text_table( _complex   **lpxS, 
                                      FREQUENCY  *frq_out, 
                                      INPUT_DATA *id );

int  write_S_to_listing( S_INDEX    *si,
                                        int               extrapol_flag,
                                        _complex   **sss,
                                        FREQUENCY  *frq_out,
                                        INPUT_DATA *id );


//int                *build_index_table( const _OFFSET_TABLE& ot1, const _OFFSET_TABLE& ot2,
//                                       _OFFSET_TABLE& ot3,
//                                       Uint *lpIndexTable, Uint *lpPointNum );

#endif
