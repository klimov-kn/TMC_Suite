/*
**  TAMIC_soft(R) 1991,1992 vR1.02
**
**  Miscellaneous definitions and declarations for IC-program.
**
** FILE: \include\mic\misc_ic.h
**
*/

#ifndef _MISC_IC_H
 #define _MISC_IC_H

/* Enumeration of notes and frequencies     */
enum NOTES
   {
    A0 = 440,
    C1 = 523, D1 = 587, D1d = 622, E1 = 659, F1 = 699, G1 = 784, A1 = 880, B1 = 988,
    EIGHTH = 62, QUARTER = 125, QUARTER_POINT = 187, HALF = 250, HALF_POINT = 375, WHOLE = 500,
    END = 0
   };

#define MIN_TRACE_DIM 30

void _far pauseex( void);
void _far _fortran Signal( int nFreq, int nDuration );

#endif
