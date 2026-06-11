/*
**  TAMIC_soft(R) 1989,1990, 1991, 1992 v1.00
**
**  inptabd.h
**
**  FILE : ..\include\inptabdb.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef __INPTABDB_H
 #define __INPTABDB_H

#ifndef _INP_ADRESS
 #include <run.h>
#endif

typedef struct
	{
	 int  code;
	 void _far *ptr;
	 void _far *next;
	 } OUTPUT_LIST;


OUTPUT_LIST _far * fortran get_out_first_ptr( void );
void        _far   fortran set_out_first_ptr( OUTPUT_LIST _far *ptr );
OUTPUT_LIST _far * fortran get_out_last_ptr( void );
void        _far   fortran set_out_last_ptr( OUTPUT_LIST _far *ptr );

OUTPUT_LIST _far * fortran alloc_out_list( int code );

#endif
