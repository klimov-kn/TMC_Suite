/*
**  TAMIC_soft(R) 1990,1991 v1.00
**
**  data_const_list.h
**
**  FILE : a:\include\datacnst.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _LONGDATA_H
 #define _LONGDATA_H


struct long_unit_list
          {
	   int         code;
	   _real       unit[3];
	   char  _far *name[3];
           };

void  _far   fortran set_long_unit_code( int nCode );
char  _far * fortran get_long_unit_name( int nCode );
char  _far * fortran get_current_long_unit_name( void );
_real _far   fortran get_current_long_unit( void );

void  _far  fortran set_long_tolerance( _real toler );
_real _far  fortran get_long_tolerance( _real _far *toler );

#endif
