/*
 *  TAMIC_soft(R) 1990,1991,1992 vR1.01
 *
 *  Declarations and definitions for complex mathematics.
 *
 * Define the macro switch: INLN_COMPLEX,
 * if you wish to use macros expanding to inline code (instead of
 * functions) for the complex conversion functions.
 * Beware of duplicate names!
 *
 *  FILE : \source\include\complex.h
 *
*/

#ifndef _COMPLEX_H
  #define _COMPLEX_H

// #ifndef _COMPLEX_DEFINED
//   #define _COMPLEX_DEFINED
//
//  typedef struct tag_complex { _real x;
//                               _real y; } _complex;/* real and imaginary parts */
//
// #endif

  /* pseudo functions */



#ifdef INLN_COMPLEX	/* then use the following macro versions */

_complex z;
#define   aimag(z)  ( z.y ) /* return float */
#define   ctof(z)   ( z.x ) /* return float */
#define   ftoc(r,i) (( z.x = r, z.y = i ), z)        /* return complex */
#define   conjg(z)  (( z.y = -z.y ), z)              /* return complex */
#define   cneg(z)   ( ( z.x = -z.x, z.y = -z.y ), z) /* return complex */

#else                    /* declare functions */

_real    aimag( _complex );  /* imaginary component of complex number */
_real    ctof( _complex );   /* complex (real component) to float conversion */
_complex ftoc(_real, _real); /* convert floats to complex */
_complex conjg( _complex );  /* complex conjugate */
_complex cneg( _complex );           /* negation of a complex number */

#endif
 /* direct angle (in radians) of complex number */
_real    cphase( _complex ); /* angle (in radians) of complex number from 0 to -2*PI*/
_real    cabsv( _complex );   /* cosine of complex number */


#endif
