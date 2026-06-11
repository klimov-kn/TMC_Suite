#ifndef _TYPDEF_H
  #define _TYPDEF_H

//#include <windows.h>

#ifndef _NDP2
 #define _NDP2
#endif

#ifndef _NDP2
 #define _NDP2
#endif

#ifndef _HIGH_ACCURACY
  #define _HIGH_ACCURACY
#endif

#ifndef _ELEM_VAL
 #define _ELEM_VAL _real
 #ifndef _ELEM_COMPLEX
  #define _ELEM_COMPLEX
 #endif
 #pragma message("_ELEM_VAL define as _complex . . .")
#endif

#if (defined _NDP2)||(defined _DOS386)||(defined DOS386)
 #define _near
 #define  near

 #define _far
 #define  far

 #define _huge
 #define  huge

 #define _fortran
 #define  fortran

 #define _pascal
//  #define  pascal

 #define _BASE_CODE

 #define _cdecl
#endif

#ifdef _NDP2
 #define _static_ static
#else
 #define _static_
#endif

#include <float.h>

#if (defined _MODEL_IC)||(defined _MODEL_M)
#ifndef _HIGH_ACCURACY
  #define _HIGH_ACCURACY
#endif
#endif

typedef unsigned char Uchar;
typedef unsigned char BYTE;

typedef unsigned int   Uint;
typedef unsigned short WORD;

#if (defined _MODEL_H)||(defined _MODEL_E)
 typedef short          INT;
#endif

typedef unsigned long Ulong;
typedef unsigned long DWORD;

#ifndef FILENAME
 #define FILENAME           13
#endif
#ifndef FILENAME_MAX
 #define FILENAME_MAX      63
#endif

typedef struct { char name[FILENAME];
                 char path[FILENAME_MAX];
                 char drive;
                 } FILE_LIST;

#ifndef _HIGH_ACCURACY
  #define REAL_DIG  FLT_DIG    /* # of decimal digits of precision */
  #define REAL_MAX_10_EXP  FLT_MAX_10_EXP    /* max decimal exponent */
  #define REAL_MIN_10_EXP  FLT_MIN_10_EXP    /* max decimal exponent */
  #define REAL_MAX  FLT_MAX    /* max value */
  #define REAL_MIN  FLT_MIN    /* min positive value */
  #define REAL_EPSILON FLT_EPSILON /* smalest such that 1 + REAL_EPSILON != 1 */
  typedef float _real;
  #pragma message("FLOAT storage as 4 bytes . . .")
#ifndef _NDP2
  #pragma message("FLOAT storage as 4 bytes . . .")
#endif
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
  #pragma message("FLOAT storage as 8 bytes . . .")
#ifndef _NDP2
  #pragma message("FLOAT storage as 8 bytes . . .")
#endif
#endif

// typedef _real _far * r_array;

#ifndef __cplusplus
 #ifndef __COMPLEX_DEFINED
   #define __COMPLEX_DEFINED

  typedef struct tag_complex { _real x;
                               _real y; }_complex;/* real and imaginary parts */

 // typedef _complex _far * c_array;
 #endif
#else
 #ifndef __COMPLEX_DEFINED_TMC
   #define __COMPLEX_DEFINED_TMC

	#include <math.h>

#endif
#endif

#ifndef _NDP2
#ifndef DOS386
 #define _BASE_CODE    _based( _segname("_CODE") )
#else
 #define fputs(a,b)  fprintf(b,"%s",a)
#endif
#endif

#define MU_NUL               (_real)12.56637e-7
#define EPSILON_NUL          (_real)8.85418757e-12
#define LIGHT_VEL            (_real)2.99792458e8

#ifndef PI
 #define PI                   (_real)3.141592653589793
#endif

int _far _fortran isnumber( char );
#define isnumber(_c) (isdigit( _c )||( _c == '+' )||( _c == '-' )||( _c == '.' ))

int _far _fortran rsign( _real );
#define rsign( rn )  ( (rn >= 0)?1:-1 )

#define INT_1 short

#endif
