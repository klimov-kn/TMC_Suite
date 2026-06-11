/*
**  TAMIC_soft(R) 1991 v1.10
**
**  lock.h
**
**  FILE : \include\lock.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _LOCK_H
 #define _LOCK_H

void _far fortran lock_1( char _far *prog_name );
void _far fortran lock_2( void );
void _far fortran lock_3( void );
void _far fortran lock_4( void );
void _far fortran lock_time( void );

#endif
