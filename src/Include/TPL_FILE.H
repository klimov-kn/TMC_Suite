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



char *          get_tpl_line( char *str_buff );

int  tpl_str_scan( Uint code, char *string, ... );
int   tpl_str_scan( Uint code, char *string, ... );
int   get_include_echo_flag( void );
#endif
