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

				"overflow x/y",
				"for x^y x<0",
				"abnormal bracket",
				"abnormal function",
				"abnormal expression",
				"float overflow",
				"bad parameter in =",
				"bad return"
		     };


char   * fortran expr_get_error( void )
 {
  return( char_error[ -expr_get_error_cod() ] );
  }

