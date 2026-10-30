/*

 *  TAMIC_soft(R) 1990,1991,1992 vR1.01
 *
 *  Complex library prototype function:
 *
 *  aimag  - return the imaginary component of a complex
 *
 *  FILE : ..\complex\aimag.c
 *
 *  LIB  : cmplx_X.lib
 *
 *  OVERLAY: NO
*/

/// @file CMPLXALL.CPP
/// @brief Реализация функций комплексной арифметики (библиотека `complex`).
///
/// Единственный файл библиотеки `complex`. Все функции объявлены и описаны
/// в complex1.h; здесь — только определения. Функции чистые, без глобального
/// состояния, потокобезопасны.
/// @see https://www.tamic.ru/docs/code/lib-complex.html

#include <stdlib.h>
#include <math.h>

#include <Typedef.h>
#include <COMPLEX1.H>

             /* return the imaginary component of a complex number */

        /* direct angle (in radians) of a complex number */

          /* complex absolute value */

           /* complex absolute value */
_real cabsv( _complex z )
  {
   double s, m, aux;
   m = __max( fabs((double)z.x),fabs((double)z.y) );
   if( m == (double)0. ) return( (_real)(0) );
   aux = z.x/m;
   s = aux*aux;
   aux = z.y/m;
   s = s + aux*aux;
   return( (_real)(m*sqrt(s)) );
  }
             /* retn the sum of two complex numbers */

             /* cosine of complex number */

             /* retn the quotient of two complex numbers */

        /* return TRUE if complex numbers equal */


              /* complex exponential */

              /* complex natural log */

              /* retn the product of two complex numbers */

               /* complex unary minus operation */


                /* return the complex conjugate */

/// Ноль типа `_real` для сравнений в `cphase`.
#define ZERO (_real)0.

_real cphase( _complex z )
  {
   _real fi;

   if( (z.y == ZERO) && (z.x == ZERO) )  return( ZERO );

   if( z.x == ZERO )
     {
      if( z.y > ZERO )  fi = (_real)(-1.5*PI);
      else              fi = (_real)(-0.5*PI);
      return( fi );
     }

   if( z.y == ZERO )
     {
      if( z.x > ZERO )  fi = ZERO;
      else              fi = -PI;
      return( fi );
     }

   // atan даёт угол в (-pi/2, pi/2); по квадранту он переводится в диапазон (-2*pi, 0)
   fi = (_real)atan( z.y/ z.x );
   if( (z.y > ZERO) && (z.x > ZERO) )   fi = fi - (_real)(2.0*PI);
   if( (z.y > ZERO) && (z.x < ZERO) )   fi = fi - PI;
   if( (z.y < ZERO) && (z.x < ZERO) )   fi = fi - PI;

   return( fi );

  } /* end of cphase function */

             /* general complex power */

              /* general complex raised to float power */


              /* sine of complex number */


              /* retn (positive) sq root of complex number */









              /* retn the difference of two complex numbers */


             /* complex (real component) to float conversion */




