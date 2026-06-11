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

int  _far   _fortran set_file_name( char _far *name );
char _far * _fortran get_S_file_name( void );

void _far   _fortran Set_OutTextFlag( int nFlag );
int  _far   _fortran Get_OutTextFlag( void );

void _far           ys_proc( void );
void _far           Y_to_norm_Y( char _far *output_name, char _far *input_name );
void _far           S_to_Y( char _far *output_name, _complex _far **sss, FREQUENCY frq_out, INPUT_DATA _far *id, S_INDEX _far  *si, int extrapol_flag);
int _far            _S_to_Y( _complex _far *sm, float _far *ymatr, int dim );
//void _far           _S_to_Y( _complex _far *sss, float _far *ymatr, int dim );

int  _far  _fortran put_S_text_table( _complex   _far **lpxS, 
                                      FREQUENCY  _far  *frq_out, 
                                      INPUT_DATA _far  *id );

int  _far  _fortran write_S_to_listing( S_INDEX    _far  *si,
                                        int               extrapol_flag,
                                        _complex   _far **sss,
                                        FREQUENCY  _far  *frq_out,
                                        INPUT_DATA _far  *id );

int  _far   fortran _mix_S1_and_S2( char _far *szName1, char _far *szName2,
                                    char _far *szTaget );
void _far title_ys( void );
int _far _fortran set_title_ys( char *ch );
int _far _fortran title_ys_n( void );
void _far _fortran set_rSmaxCut( _real s );
_real _far _fortran get_rSmaxCut( void );


//int                *build_index_table( const _OFFSET_TABLE& ot1, const _OFFSET_TABLE& ot2,
//                                       _OFFSET_TABLE& ot3,
//                                       Uint *lpIndexTable, Uint *lpPointNum );

#endif
