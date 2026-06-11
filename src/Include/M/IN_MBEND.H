/*
**  TAMIC_soft(R) 1992 v1.00
**
**  in_mstep.h
**
**  FILE : ..\include\indan\in_mstep.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _IN_MBEND_H
 #define _IN_MBEND_H

  #ifndef _RUN_M_H
   #include <m\run_m.h>
  #endif
			//                               w
 typedef struct         // parameters of line         ÃÄÄÄÄÄÄ´
	 {              //
	  _real w;      // width
	  } DATA_BEND;  //                 Â  ÚÄÄÄÄÄÄÄÒÒÒÒÒÒÒÒÄÄÄÄÄÄÄÄ¿
			//                 ³  ³       ºººººººº        ³
			//                 ³  ³       ºººººººº        ³
			//                l³  ³ e_r   ºººººººº        ³
			//                 ³  ³       ºººººººº        ³
			//                 Á  ÀÄÄÄÄÄÄÄĞĞĞĞĞĞĞĞÄÄÄÄÄÄÄÄÙ
 int _far _fortran in_M_Bend( char _far *lpszFilename, ELEMENT _far *lpElement );
 int _far _fortran alloc_M_Bend( ELEMENT _far *lpElement );

 _ELEM_VAL _far *_fortran y_bend_tmc_m( DATA_BEND _far *data );
 int _far _fortran y_bend_fpData( DATA_EHWF _far *fpData, DATA_BEND _far *data);

#endif
