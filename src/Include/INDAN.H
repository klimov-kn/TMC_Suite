/*
**  TAMIC_soft(R) 1989,1990,1991,1992 v1.00
**
**  indan.h
**
**  FILE : a:\include\indan.c
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _INDAN_H
 #define _INDAN_H

#define LISTING_MODE    0
#define SAVE_P_MODE     1


int   _far fortran load_step( FILE_LIST _far *fl, int step_num );

int   _far fortran in_data( void );
void  _far fortran LoadStepStartStatOutput( void );
void  _far fortran LoadStepEndStatOutput( void );
void  _far fortran SeekStepErrorHandler( void );

int   _far fortran in_attributes( void );
int   _far fortran in_output( void );

int   _far s_printf( char _far *format, ... );

int   _far fortran corr_all_data( void );

void  _far fortran set_output_mode( int mode );

void  _far fortran set_data_base_out_flag( int value );
int   _far fortran get_data_base_out_flag( void );

void  _far fortran set_DB_out_mode_flag( int value );
int   _far fortran get_DB_out_mode_flag( void );

void  _far fortran output_data_base( void );

void  _far   _fortran SetIncludePathName( char _far *ptr );
char  _far * _fortran GetIncludePathName( void );

void  _far _fortran SetInternalDataBaseSize( DWORD dwSize );
DWORD _far _fortran GetInternalDataBaseSize( void );
#endif
