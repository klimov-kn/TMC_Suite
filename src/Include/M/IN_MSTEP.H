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

#ifndef _IN_MSTEP_H
 #define _IN_MSTEP_H

  #ifndef _RUN_M_H
   #include <m\run_m.h>
  #endif
		       //                               w
typedef struct         // parameters of line         ÃÄÄÄÄÄÄ´
	{              //
	 _real w1;     // width 1-st line
	 _real w2;     // width 2-nd line
	 } DATA_STEP;  //                 Â  ÚÄÄÄÄÄÄÄÒÒÒÒÒÒÒÒÄÄÄÄÄÄÄÄ¿
		       //                 ³  ³       ºººººººº        ³
		       //                 ³  ³       ºººººººº        ³
		       //                l³  ³ e_r   ºººººººº        ³
		       //                 ³  ³       ºººººººº        ³
		       //                 Á  ÀÄÄÄÄÄÄÄĞĞĞĞĞĞĞĞÄÄÄÄÄÄÄÄÙ

 int _far _fortran in_M_Step( char _far *lpszFilename, ELEMENT _far *lpElement );
 int _far _fortran alloc_M_Step( ELEMENT _far *lpElement );

 _ELEM_VAL _far *_fortran y_step_tmc_m( DATA_STEP _far *data );
 int _far _fortran y_step_fpData( DATA_EHWF _far *fpData1, DATA_EHWF _far *fpData2, DATA_STEP _far *data);

#endif
