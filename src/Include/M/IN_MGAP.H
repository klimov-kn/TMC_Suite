/*
**  TAMIC_soft(R) 1992 v1.00
**
**  in_mgap.h
**
**  FILE : ..\include\indan\in_mgap.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _IN_MGAP_H
 #define _IN_MGAP_H

  #ifndef _RUN_M_H
   #include <m\run_m.h>
  #endif
			//                               w
 typedef struct         // parameters of line         ÃÄÄÄÄÄÄ´
	 {              //
	  _real w;      // width
	  _real s;      // lenght gap
	  } DATA_GAP ;  //                    ÚÄÄÄÄÄÄÄÒÒÒÒÒÒÒÒÄÄÄÄÄÄÄÄ¿
			//                    ³       ºººººººº        ³
			//                    ³                       ³
			//                    ³          s            ³
			//                    ³       ºººººººº        ³
			//                    ÀÄÄÄÄÄÄÄĞĞĞĞĞĞĞĞÄÄÄÄÄÄÄÄÙ

 int _far _fortran in_M_Gap( char _far *lpszFilename, ELEMENT _far *lpElement );
 int _far _fortran alloc_M_Gap( ELEMENT _far *lpElement );

 _ELEM_VAL _far *_fortran y_gap_tmc_m( DATA_GAP _far *data );
 int _far _fortran y_gap_fpData( DATA_RUPT _far *fpData, DATA_GAP _far *data);

#endif
