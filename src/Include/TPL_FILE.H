/*
**  TAMIC_soft(R) 1991 v1.00
**
**  tpl_file.h
**
**  FILE : a:\include\tpl_file.c
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _TPL_FILE_H
 #define _TPL_FILE_H


#define DEVIDE_CHAR      ';'



Uint _far  get_tpl_line_number( void );
void _far  set_tpl_line_number( Uint line_number );

char _far *get_tpl_file_name( void );
void _far  set_tpl_file_name( char _far *name );

int  _far   _fortran open_tpl_file( FILE_LIST _far *fl );
void _far            close_tpl_file( void );
void _far            fseek_tpl_file( void );

char _far * _fortran fget_tpl_line( char _far *str_buff );
char _far *          get_tpl_line( char _far *str_buff );

int  _far  tpl_str_scan( Uint code, char _far *string, ... );
int  _far  test_unknown_string( char _far *string );
int  _far  test_end_block_dscr( Uint dscr_code, char _far *string );
int  _far  test_wild_block_dscr( Uint dscr_code, char _far *string );

int _far fortran seek_step( int step_num );
char _far * _fortran _fget_tpl_line( char _far *str_buff );

int   _far  str_scan( char devider, char _far *string, char _far *format, ... );
int   _far  vstr_scan( char devider, char _far *string, char _far *format, va_list argptr );
int   _far  tpl_str_scan( Uint code, char _far *string, ... );

char  _far          *bf_get_tpl_line( char _far *str );
void  _far  fortran  set_prepr_echo_flag( int flag );
int   _far  fortran  get_prepr_echo_flag( void );

void  _far  fortran  set_include_echo_flag( int flag );
int   _far  fortran  get_include_echo_flag( void );
#endif
