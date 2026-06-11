/*
 *  TAMIC_soft(R) 1992 vR1.02
 *
 *  Definitions and declarations  for S-files reading utility.
 *
 *  FILE : ..\proc_s.h
 *
*/

#ifndef _PROC_S_D
 #define _PROC_S_D

#define TITLE_STR        "TAMIC S-files control utility.  Version   3.03."
#define COPYRIGHT_TITLE  "Copyright  (C) TAMIC-soft. All rights reserved."
#define LIST_TITLE       "TAMIC_soft(R)        Reading S-files utility. Version 3.03 1991,1993"

#define REAL_IMAG  0
#define ARG_PHASE  1
#define VSWR_LOSS  2

void _far   _fortran get_arg_env( int argc, char _far *argv[], char _far *envp[] );

int  _far   _fortran set_file_name( char _far *name );
char _far * _fortran get_S_file_name( void );

int _far    _fortran processor( void );
int _far put_error_messege( int err_num, ... );
void _far _fortran put_S_table( _complex _far *lpxS, int inpnb, int _far *modnb );
void _far _fortran Set_Table_Mode( int nMode );
void _far _fortran Set_OutTextFlag( int nFlag );
int  _far _fortran Get_OutTextFlag( void );
void _far _fortran Set_OutListFlag( int nFlag );
int  _far _fortran Get_OutListFlag( void );
void   _far OutTrace( char _far *trace );

#define TPL_LINE_BUF_MAX 10000
char  _far * _fortran get_tpl_line( int n );
void _far _fortran tpl_del_bl1( char _far *ch );
void _far _fortran tpl_del_bl2( char _far *ch );
int _far _fortran read_S_matrix_element( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint);
int _far _fortran read_S_matrix_element_TMC( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint);
int _far _fortran read_S_matrix_element_HFSS( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint);
int _far _fortran read_S_matrix_element_CST( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint);
int _far _fortran SMatrixElementCST_InpMod( int *in1, int *mod1, int *in2, int *mod2, char *filename );
int _far _fortran SMatrixElementCST_ModMax( char *filename, int *pnInp );
int _far _fortran SMatrixElementCST_InpMax( char *filename );
int _far _fortran SMatrixElementCST_FreqArray( int *in1, int *mod1, int *in2, int *mod2, char *filename, double **f1, _complex **sss, int *nPoint, int *nMatrixSize);
void _far _fortran SMatrixElementCST_FreqArrayCalculation( char *filename, double **f1, int *nPoint, int *nMatrixSize);
void _far _fortran SMatrixElementCST_FreqArrayAllocation( double **f1, _complex **sss, int *nPoint);
void _far _fortran SMatrixElementCST_FreqArrayRead( int *in1, int *mod1, int *in2, int *mod2, char *filename, double **f1, _complex **sss, int *nPoint, int *nMatrixSize);
int _far _fortran save_S_matrix_element( int in1, char *filename, _complex *sss, double f1, int nInput);
int _far _fortran S_add( char *filename, char *filename1 );

#endif
