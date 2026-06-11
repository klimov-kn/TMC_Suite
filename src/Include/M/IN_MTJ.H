/*
**  TAMIC_soft(R) 1992 v1.00
**
**  in_mtj.h
**
**  FILE : ..\include\indan\in_mtj.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _IN_MTJ_H
 #define _IN_MTJ_H

  #ifndef _RUN_M_H
   #include <m\run_m.h>
  #endif

		       //                               w
typedef struct         // parameters of line         ÃÄÄÄÄÄÄ´
	{              //
	 _real w1;     // width 1-st line
	 _real w2;     // width 2-nd line
	 } DATA_TJ  ;  //                 Â  ÚÄÄÄÄÄÄÄÒÒÒÒÒÒÒÒÄÄÄÄÄÄÄÄ¿
		       //                 ³  ³       ºººººººº        ³
		       //                 ³  ³       ºººººººº        ³
		       //                l³  ³ e_r   ºººººººº        ³
		       //                 ³  ³       ºººººººº        ³
		       //                 Á  ÀÄÄÄÄÄÄÄĞĞĞĞĞĞĞĞÄÄÄÄÄÄÄÄÙ
 int _far _fortran in_M_T_J( char _far *lpszFilename, ELEMENT _far *lpElement );
 int _far _fortran alloc_M_T_J( ELEMENT _far *lpElement );

 _ELEM_VAL _far *_fortran y_tj_tmc_m( DATA_TJ _far *data );
 int _far _fortran y_tj_fpData( DATA_EHWF _far *fpData1, DATA_EHWF _far *fpData2, DATA_TJ _far *data);
 int _far _fortran y_tj_mtData( _ELEM_VAL _far *mtData, Y_PAR_3 _far *y);

#endif
