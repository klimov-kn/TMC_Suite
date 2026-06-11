/*
**  TAMIC_soft(R) 1992 v1.00
**
**  in_mfile.h
**
**  FILE : ..\include\indan\in_moend.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _IN_MOEND_H
 #define _IN_MOEND_H

  #ifndef _RUN_M_H
   #include <m\run_m.h>
  #endif
			       //                               w
 typedef struct                // parameters of line         ÃÄÄÄÄÄÄ´
	 {                     //
	  _real w;             // width
	  _real l;             // length
	  } DATA_OPEN_END;     //                 Â  ÚÄÄÄÄÄÄÄÒÒÒÒÒÒÒÒÄÄÄÄÄÄÄÄ¿
			       //                 ³  ³       ºººººººº        ³
			       //                 ³  ³       ºººººººº        ³
			       //                l³  ³ e_r   ºººººººº        ³
			       //                 ³  ³       ºººººººº        ³
			       //                 Á  ÀÄÄÄÄÄÄÄĞĞĞĞĞĞĞĞÄÄÄÄÄÄÄÄÙ


 int _far _fortran in_M_Open_End( char _far *lpszFilename, ELEMENT _far *lpElement );
 int _far _fortran alloc_M_Open_End( ELEMENT _far *lpElement );
 _ELEM_VAL _far *_fortran y_oend_tmc_m( DATA_OPEN_END _far *data );
 int _far _fortran y_oend_fpData( DATA_EHWF _far *fpData, DATA_OPEN_END _far *data);

#endif

