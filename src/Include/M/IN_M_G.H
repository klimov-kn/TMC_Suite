/*
**  TAMIC_soft(R) 1992 v1.00
**
**  in_mstep.h
**
**  FILE : ..\include\indan\in_m_g.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _IN_M_G_H
 #define _IN_M_G_H

  #ifndef _RUN_M_H
   #include <m\run_m.h>
  #endif

 typedef struct   //                          ÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄ
   {              //                                ÚÁ¿     
    _real g;      // resister                 1     ³g³     2
    _real dx;     // X-size                         ÀÂÙ
    _real dy;     // Y-size                   ÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄ
    } DATA_G;     //

		  //
		  //                         ÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄ
 typedef struct   //                                >                         
   {              //                         1      <l     2                 
    _real gl;     // inductivity                    >
    _real dx;     // X-size                  ÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄ
    _real dy;     // Y-size
     } DATA_GL;   //
		  //                         ÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄ
 typedef struct   //                                ³                                       
   {              //                         1     ÍØÍ c   2                                 
    _real gc;     //                                ³
    _real dx;     // X-size                  ÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄ
    _real dy;     // Y-size
    } DATA_GC;    //

#ifdef _ELEM_COMPLEX
 int _far _fortran in_M_g( char _far *lpszFilename, ELEMENT _far *lpElement );
#endif
 int _far _fortran in_M_gl( char _far *lpszFilename, ELEMENT _far *lpElement );
 int _far _fortran in_M_gc( char _far *lpszFilename, ELEMENT _far *lpElement );

#ifdef _ELEM_COMPLEX
 int _far _fortran alloc_M_g( ELEMENT _far *lpElement );
#endif
 int _far _fortran alloc_M_gl( ELEMENT _far *lpElement );
 int _far _fortran alloc_M_gc( ELEMENT _far *lpElement );

#ifdef _ELEM_COMPLEX
 _ELEM_VAL _far *_fortran y_g_tmc_m( DATA_G _far *data );
#endif
 _ELEM_VAL _far *_fortran y_gl_tmc_m( DATA_GL _far *data );
 _ELEM_VAL _far *_fortran y_gc_tmc_m( DATA_GC _far *data );

#ifdef _ELEM_COMPLEX
 int _far _fortran y_g_fpData( DATA_EHWF _far *fpData, DATA_G _far *data);
#endif
 int _far _fortran y_gl_fpData( DATA_EHWF _far *fpData, DATA_GL _far *data);
 int _far _fortran y_gc_fpData( DATA_EHWF _far *fpData, DATA_GC _far *data);

#endif
