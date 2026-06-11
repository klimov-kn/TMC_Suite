/*
**  TAMIC_soft(R) 1992 v1.00
**
**  in_mpaz.h
**
**  FILE : ..\include\indan\in_mpaz.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _IN_MPAZ_H
 #define _IN_MPAZ_H

  #ifndef _RUN_M_H
   #include <m\run_m.h>
  #endif

typedef struct               // parameters for paz
	{                    //                   รฤฤฤด
			     //                     a
	  _real w;           //       ย  อออออออออป   ษออออออออออ   ย
	  _real a;           //       ณ           บ   บ            bณ
	  _real b;           //      wณ           ศอออผ             ม
	} DATA_PAZ;          //       ณ             |
			     //       ม  ออออออออออออออออออออออออ
			     //                     |
			     //referens 1 input - > |<-referens 2 input
			     //                     |
			     //                     |
			     //
			     //
			     //
			     //
 int _far _fortran in_M_Paz( char _far *lpszFilename, ELEMENT _far *lpElement );
 int _far _fortran alloc_M_Paz( ELEMENT _far *lpElement );

 _ELEM_VAL _far *_fortran y_paz_tmc_m( DATA_PAZ _far *data );
 int _far _fortran y_paz_fpData( DATA_PAZ1 _far *fpData, DATA_PAZ _far *data);

#endif
