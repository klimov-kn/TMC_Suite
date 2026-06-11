/*
**  TAMIC_soft(R) 1992 v1.00
**
**  in_mfile.h
**
**  FILE : ..\include\indan\in_mline.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _IN_MLINE_H
 #define _IN_MLINE_H

  #ifndef _RUN_M_H
   #include <m\run_m.h>
  #endif
			//                               w
 typedef struct         // parameters of line         ÃÄÄÄÄÄÄ´
	 {              //
	  _real w;      // width
	  _real l;      // length
	  } DATA_LINE;  //                 Â  ÚÄÄÄÄÄÄÄÒÒÒÒÒÒÒÒÄÄÄÄÄÄÄÄ¿
			//                 ³  ³       ºººººººº        ³
			//                 ³  ³       ºººººººº        ³
			//                l³  ³ e_r   ºººººººº        ³
			//                 ³  ³       ºººººººº        ³
			//                 Á  ÀÄÄÄÄÄÄÄĞĞĞĞĞĞĞĞÄÄÄÄÄÄÄÄÙ

 int _far _fortran in_M_Line( char _far *lpszFilename, ELEMENT _far *lpElement );
 int _far _fortran alloc_M_Line( ELEMENT _far *lpElement );

 _ELEM_VAL _far *_fortran y_line_tmc_m( DATA_LINE _far *data );
 int _far _fortran y_line_mtData( _ELEM_VAL _far *mtData, Y_PAR_LIN _far *y);
 int _far _fortran y_line_fpData( DATA_EHWF _far *fpData, DATA_LINE _far *data);

#endif

