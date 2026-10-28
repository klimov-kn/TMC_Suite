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

#define REAL_IMAG  0
#define ARG_PHASE  1
#define VSWR_LOSS  2


int  set_file_name( char *name );
char * get_S_file_name( void );

int processor( void );
int put_error_messege( int err_num, ... );
void put_S_table( _complex *lpxS, int inpnb, int *modnb );
void Set_OutTextFlag( int nFlag );
int  Get_OutTextFlag( void );
void   OutTrace( char *trace );

char  * get_tpl_line( int n );
int read_S_matrix_element( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint);
int read_S_matrix_element_TMC( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint);
int read_S_matrix_element_HFSS( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint);
int read_S_matrix_element_CST( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint);
int SMatrixElementCST_InpMod( int *in1, int *mod1, int *in2, int *mod2, char *filename );
int SMatrixElementCST_ModMax( char *filename, int *pnInp );
int SMatrixElementCST_InpMax( char *filename );
int SMatrixElementCST_FreqArray( int *in1, int *mod1, int *in2, int *mod2, char *filename, double **f1, _complex **sss, int *nPoint, int *nMatrixSize);
void SMatrixElementCST_FreqArrayCalculation( char *filename, double **f1, int *nPoint, int *nMatrixSize);
void SMatrixElementCST_FreqArrayAllocation( double **f1, _complex **sss, int *nPoint);
void SMatrixElementCST_FreqArrayRead( int *in1, int *mod1, int *in2, int *mod2, char *filename, double **f1, _complex **sss, int *nPoint, int *nMatrixSize);
int save_S_matrix_element( int in1, char *filename, _complex *sss, double f1, int nInput);
int S_add( char *filename, char *filename1 );

#endif
