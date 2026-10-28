/*
**  TAMIC_soft(R)        25.10.1991, 07.07.1999          v2.00
**
**  expr_set_error     : set error cod
**  expr_get_error     : get error *string
**  expr_get_error_cod : get error cod
**
**  FILE               :
**                       expr_err.c,
**  HEDER FILE'S       :
**                       <string.h>
**                       <stdio.h>
**                       <stdlib.h>
**                       <math.h>
**                       <float.h>
**                       <typedef.h>
**                       "expr.h"
**
**  LIB               : NO
**
**  OVERLAY           : NO
**
*/


#include <typedef.h>
#include <expr.h>

static char  *char_error[]={
				"all right",
				"overflow in function exp",
				"argument LOG <= 0 (bad)",
				"overflow in function sin (argument > 65536)",
				"overflow in function cos (argument > 65536)",
				"overflow in function tan (argument > 65536 or = pi/2",
				"overflow",
				"argument SQRT < 0 (bad)",
				"overflow in function sinh",
				"overflow in function cosh",
				"overflow in function tanh",
				"argument ASIN > 1 (bad)",
				"argument ACOS > 1 (bad)",
				"overflow",
				"argument LOG10 <=0 (bad)",

				"division by zero in x/y",   // N-30: it was "overflow x/y"
				"x^y: x<0 with a fractional y, or 0^y with y<=0",   // A-10: it was "for x^y x<=0"
				"abnormal bracket",
				"abnormal function",
				"abnormal expression",
				"float overflow",
				"bad return",                // N-30: -21 ERROR_RETURN (the two lines were swapped)
				"bad parameter in ="         // N-30: -22 ERROR_VAR
		     };


char   * fortran expr_get_error( void )
 {
  int n = -expr_get_error_cod();
  if( (n < 0)||(n >= (int)(sizeof(char_error)/sizeof(char_error[0]))) ) return( (char *)"unknown error of expression" );   // N-30
  return( char_error[ n ] );
  }

