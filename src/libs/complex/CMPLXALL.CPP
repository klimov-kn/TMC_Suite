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

#include <stdlib.h>
#include <math.h>

#include <typedef.h>
#include <complex1.h>

             /* return the imaginary component of a complex number */
  _real _far _fortran aimag( _complex z )
    {
     return( (_real)(z.y) );
    }

        /* direct angle (in radians) of a complex number */
_real _far _fortran arg_cpx( _complex z )
  {
   return( (_real)asin( z.y/cabsv(z) ) );
  }

          /* complex absolute value */
_real _far _fortran cabsm( _complex z )
  {
   double s, m, aux;
   m = __max( fabs((double)z.x),fabs((double)z.y) );
   if( m == (double)0. ) return( (_real)(0) );
   aux = z.x/m;
   s = aux*aux;
   aux = z.y/m;
   s = s - aux*aux;
   return( (_real)(m*sqrt(s)) );
  }

           /* complex absolute value */
_real _far _fortran cabsv( _complex z )
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
_complex _far _fortran cadd( _complex l, _complex r )
  {
   _complex z;
   z.x = l.x + r.x;
   z.y = l.y + r.y;
   return( z );
  }

             /* cosine of complex number */
_complex _far _fortran ccos( _complex z )
  {
   _complex zc;

   zc.x = (_real)(cos(z.x)*cosh(z.y));
   zc.y = (_real)(-sin(z.x)*sinh(z.y));
   return( zc );
  }

             /* retn the quotient of two complex numbers */
_complex _far _fortran cdiv( _complex l, _complex r  )
  {
   _complex z;
   double den;

   if( r.x == (_real)0 && r.y == (_real)0. )
     {
      if( l.x == (_real)0. && l.y == (_real)0. )
        {
         z.x = (_real)0; z.y = (_real)0;
         return( z );
        }
      z.x = (_real)REAL_MAX/sqrt(2.); z.y = (_real)REAL_MAX/sqrt(2.);
      return( z );
     }
   den = (double)r.x*r.x + (double)r.y*r.y;

   z.x = (_real)( (double)l.x*r.x + (double)l.y*r.y )/den;
   z.y = (_real)( (double)r.x*l.y - (double)l.x*r.y )/den;
   return( z );
  }

        /* return TRUE if complex numbers equal */
int _far _fortran ceq( _complex l, _complex r )
  {
   if( l.x == r.x  &&  l.y == r.y )
     return( 1 );
   return( 0 );
  }


              /* complex exponential */
_complex _far _fortran cexp( _complex z )
  {
   _real exp_re;

   if( z.x == (_real)0 )
     exp_re = (_real)1.;
   else
     exp_re = (_real)exp(z.x);

   if( z.y == (_real)0 )  /* real only complex number */
     {
      z.x = exp_re;
      return( z );
     }
   z.x = exp_re*cos(z.y);
   z.y = exp_re*sin(z.y);
   return( z );
  }

              /* complex natural log */
_complex _far _fortran clog( _complex z )
  {
   _complex l;

   l.x = (_real)log( cabsv(z) );
   l.y = arg_cpx(z);  /* the direct angle */
   return( l );
  }

              /* retn the product of two complex numbers */
_complex _far _fortran cmul( _complex l, _complex r )
  {
   _complex z;
   z.x = l.x*r.x - l.y*r.y;
   z.y = l.x*r.y + l.y*r.x;
   return( z );
  }

               /* complex unary minus operation */
  _complex _far _fortran cneg( _complex z )
    {
     z.x = -z.x; z.y = -z.y;
     return( z );
    }


                /* return the complex conjugate */
  _complex _far _fortran conjg( _complex z )
    {
     z.y = -z.y;
     return( z );
    }

#define ZERO (_real)0.

_real _far _fortran cphase( _complex z )
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

   fi = (_real)atan( z.y/ z.x );
   if( (z.y > ZERO) && (z.x > ZERO) )   fi = fi - (_real)(2.0*PI);
   if( (z.y > ZERO) && (z.x < ZERO) )   fi = fi - PI;
   if( (z.y < ZERO) && (z.x < ZERO) )   fi = fi - PI;

   return( fi );

  } /* end of cphase function */

             /* general complex power */
_complex _far _fortran cpow( _complex l, _complex r  )
  {
   if( l.x == (_real)0 && l.y == (_real)0 )
		return( l );	/* undefined */

   return( cexp( cmul(r,clog(l)) ) );
  }

              /* general complex raised to float power */
_complex _far _fortran cpowf( _complex l, _real r )
  {
   _complex z;

   if( l.x == (_real)0 && l.y == (_real)0 )
     return( l );  /* undefined */

   z = clog( z );
   z.x = r*z.x;
   z.y = r*z.y;
   return( cexp(z) );
 }


              /* sine of complex number */
_complex _far _fortran csin( _complex z )
  {
   _complex zs;

   zs.x = (_real)(sin( z.x )*cosh( z.y ));
   zs.y = (_real)(cos( z.x )*sinh( z.y ));
   return( zs );
  }


              /* retn (positive) sq root of complex number */
_complex _far _fortran csqrt( _complex z )
  {
   _real sqrtm;    /* sq rt of complex magnitude */
   _real theta_2;  /* the complex direct angle/2. (radians) */

   sqrtm = (_real)sqrt( cabsv(z) );
   theta_2 = (_real)(arg_cpx( z )/2.);
   z.x = sqrtm*cos( theta_2 );
   z.y = sqrtm*sin( theta_2 );
   return( z );
  }



_real _far _fortran csqsub( _complex z )
  {
   double s, m, aux;
   m = __max( fabs((double)z.x),fabs((double)z.y) );
   if( m == (double)0. ) return( (_real)(0) );
   aux = z.x/m;
   s = aux*aux;
   aux = z.y/m;
   s = s - aux*aux;
   return( (_real)(m*m*s) );
  }



_real _far _fortran csqsum( _complex z )
  {
   return( (_real)((z.x)*(z.x) + (z.y)*(z.y)) );
  }



              /* retn the difference of two complex numbers */
_complex _far _fortran csub( _complex l, _complex r )
  {
   _complex z;
   z.x = l.x - r.x;
   z.y = l.y - r.y;
   return( z );
  }


             /* complex (real component) to float conversion */
  _real _far _fortran ctof( _complex z )
    {
     return( (_real)(z.x) );
    }


_complex _far _fortran ccosh( _complex z )
  {
   _complex zc;

   zc.x = (_real)(cos(z.y)*cosh(z.x));
   zc.y = (_real)(sin(z.y)*sinh(z.x));
   return( zc );
  }

_complex _far _fortran csinh( _complex z )
  {
   _complex zs;

   zs.x = (_real)(sinh( z.x )*cos( z.y ));
   zs.y = (_real)(cosh( z.x )*sin( z.y ));
   return( zs );
  }

