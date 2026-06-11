/*
**  TAMIC_soft(R) 1992 v1.00
**
**  in_melem.h
**
**  FILE : ..\include\indan\in_melem.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _IN_MELEM_H
 #define _IN_MELEM_H
 typedef struct
	 {
	  char _far *lpszFilename;
	  } DATA_MELEM;

 int _far _fortran in_M_Element( char _far *lpszFilename, ELEMENT _far *lpElement );
 int _far _fortran alloc_M_Element( ELEMENT _far *lpElement );

#endif
