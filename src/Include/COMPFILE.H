/*
**  TAMIC_soft(R) 1990,1991 v1.00
**
**  compfile.h
**
**  FILE : ..\include\compfile.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _COMPFILE_H
 #define _COMPFILE_H


void  _far   fortran set_comp_file_name( char _far *name );
char  _far * fortran get_comp_file_name( void );

void  _far   fortran set_comp_file_flag( int flag );
int   _far   fortran get_comp_file_flag( void );

#endif
