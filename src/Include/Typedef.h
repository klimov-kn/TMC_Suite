#ifndef _TYPDEF_H
  #define _TYPDEF_H

#ifndef _HIGH_ACCURACY
  #define _HIGH_ACCURACY
#endif

#ifndef _ELEM_VAL
 #define _ELEM_VAL _real
 #ifndef _ELEM_COMPLEX
  #define _ELEM_COMPLEX
 #endif
#endif

#include <float.h>


typedef unsigned int   Uint;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#ifndef FILENAME
 #define FILENAME           13
#endif
#ifndef FILENAME_MAX
 #define FILENAME_MAX      63
#endif


#ifndef _HIGH_ACCURACY
  #define REAL_DIG  FLT_DIG    /* # of decimal digits of precision */
  #define REAL_MAX_10_EXP  FLT_MAX_10_EXP    /* max decimal exponent */
  #define REAL_MIN_10_EXP  FLT_MIN_10_EXP    /* max decimal exponent */
  #define REAL_MAX  FLT_MAX    /* max value */
  #define REAL_MIN  FLT_MIN    /* min positive value */
  #define REAL_EPSILON FLT_EPSILON /* smalest such that 1 + REAL_EPSILON != 1 */
  typedef float _real;
 #else
  #define REAL_DIG  DBL_DIG    /* # of decimal digits of precision */
  #define REAL_MAX_10_EXP  DBL_MAX_10_EXP    /* max decimal exponent */
  #define REAL_MIN_10_EXP  DBL_MIN_10_EXP    /* max decimal exponent */
  #define REAL_MAX  DBL_MAX    /* max value */
  #define REAL_MIN  DBL_MIN    /* min positive value */
  #define REAL_EPSILON DBL_EPSILON /* smalest such that 1 + REAL_EPSILON != 1 */
  #ifndef _real
    typedef double _real;
  #endif
#endif

#ifndef __cplusplus
 #ifndef __COMPLEX_DEFINED
   #define __COMPLEX_DEFINED

  typedef struct tag_complex { _real x;
                               _real y; }_complex;/* real and imaginary parts */
 #endif
#else
 #ifndef __COMPLEX_DEFINED_TMC
   #define __COMPLEX_DEFINED_TMC

	#include <math.h>

#endif
#endif



#ifndef PI
 #define PI                   (_real)3.141592653589793
#endif



#define INT_1 short

#endif
