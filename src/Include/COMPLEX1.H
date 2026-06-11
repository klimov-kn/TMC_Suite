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

#define itocx( i, j ) ftoc((_real)i, (_real)j)
#define ltocx( i, j ) ftoc((_real)i, (_real)j)


#ifdef INLN_COMPLEX	/* then use the following macro versions */

_complex z;
#define   aimag(z)  ( z.y ) /* return float */
#define   ctof(z)   ( z.x ) /* return float */
#define   ftoc(r,i) (( z.x = r, z.y = i ), z)        /* return complex */
#define   conjg(z)  (( z.y = -z.y ), z)              /* return complex */
#define   cneg(z)   ( ( z.x = -z.x, z.y = -z.y ), z) /* return complex */

#else                    /* declare functions */

_real    _far _fortran aimag( _complex );  /* imaginary component of complex number */
_real    _far _fortran ctof( _complex );   /* complex (real component) to float conversion */
_complex _far _fortran ftoc(_real, _real); /* convert floats to complex */
_complex _far _fortran conjg( _complex );  /* complex conjugate */
_complex _far _fortran cneg( _complex );           /* negation of a complex number */

#endif


  /* functions supporting complex expressions */

_complex _far _fortran cadd( _complex, _complex ); /* sum of two complex numbers */
_complex _far _fortran csub( _complex, _complex ); /* difference of two complex numbers */
_complex _far _fortran cmul( _complex, _complex ); /* product of two complex numbers */
_complex _far _fortran cdiv( _complex, _complex ); /* quotient of two complex numbers */
_complex _far _fortran cpow( _complex, _complex ); /* general complex power */
_complex _far _fortran cpowf( _complex, _real );   /* general complex raised to float power */
int      _far _fortran ceq( _complex, _complex );  /* return TRUE if complex numbers equal */

  /* complex library functions */

_real    _far _fortran arg_cpx( _complex ); /* direct angle (in radians) of complex number */
_real    _far _fortran cphase( _complex ); /* angle (in radians) of complex number from 0 to -2*PI*/
_real    _far _fortran cabsv( _complex );   /* complex absolute value */
_real _far _fortran cabsm( _complex z );    /* sqrt from difference of squares */
_real _far _fortran csqsum( _complex z );
_real _far _fortran csqsub( _complex z );
_complex _far _fortran csqrt( _complex );  /* (positive) sq root of complex number */
_complex _far _fortran clog( _complex );   /* complex natural log */
_complex _far _fortran cexp( _complex );   /* complex exponential */
_complex _far _fortran csin( _complex );   /* sine of complex number */
_complex _far _fortran ccos( _complex );   /* cosine of complex number */
_complex _far _fortran ccosh( _complex z );
_complex _far _fortran csinh( _complex z );


#endif
