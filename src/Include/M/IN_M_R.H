/*
**  TAMIC_soft(R) 1992 v1.00
**
**  in_mstep.h
**
**  FILE : ..\include\indan\in_m_r.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _IN_M_R_H
 #define _IN_M_R_H

  #ifndef _RUN_M_H
   #include <m\run_m.h>
  #endif
		  //																							____ 
 typedef struct   //                        ÄÄÄÄÄ´____ÃÄÄÄÄ
   {              //                       1       r       2
    _real  r;     // resister               ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
    _real dx;     // X-size
    _real dy;     // Y-size
    } DATA_R;     //
                  //
 typedef struct   //                        ÄÄÄÄÄ^^^^^^ÄÄÄÄ
   {              //                       1       l       2
    _real rl;      // inductivity           ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
    _real dx;     // X-size
    _real dy;     // Y-size
    } DATA_RL;    //                             
                  //
 typedef struct   //                        ÄÄÄÄÄÄÄ´ÃÄÄÄÄÄÄ
   {              //                       1       c       2
    _real rc;      //                       ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
    _real dx;     // X-size
    _real dy;     // Y-size
    } DATA_RC;    //                             

#ifdef _ELEM_COMPLEX
 int _far _fortran in_M_r( char _far *lpszFilename, ELEMENT _far *lpElement );
#endif
 int _far _fortran in_M_rl( char _far *lpszFilename, ELEMENT _far *lpElement );
 int _far _fortran in_M_rc( char _far *lpszFilename, ELEMENT _far *lpElement );

#ifdef _ELEM_COMPLEX
 int _far _fortran alloc_M_r( ELEMENT _far *lpElement );
#endif
 int _far _fortran alloc_M_rl( ELEMENT _far *lpElement );
 int _far _fortran alloc_M_rc( ELEMENT _far *lpElement );

#ifdef _ELEM_COMPLEX
 _ELEM_VAL _far *_fortran y_r_tmc_m( DATA_R _far *data );
#endif
 _ELEM_VAL _far *_fortran y_rl_tmc_m( DATA_RL _far *data );
 _ELEM_VAL _far *_fortran y_rc_tmc_m( DATA_RC _far *data );

#ifdef _ELEM_COMPLEX
 int _far _fortran y_r_fpData( DATA_EHWF _far *fpData, DATA_R _far *data);
#endif
 int _far _fortran y_rl_fpData( DATA_EHWF _far *fpData, DATA_RL _far *data);
 int _far _fortran y_rc_fpData( DATA_EHWF _far *fpData, DATA_RC _far *data);

#endif
