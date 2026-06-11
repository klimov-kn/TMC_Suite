/*
**  TAMIC_soft(R)        25.10.1991,1992,07.07.1999       v2.00
**
**  expr_set_error     : set error cod
**  expr_get_error     : get error *string
**  expr_get_error_cod : get error cod
**
**  FILE               :
**                       expr_err.c,
**  HEDER FILE'S       :
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

static int error_cod = 0;

void  expr_set_error_cod( int n ) {  error_cod =  n; }
int   expr_get_error_cod( void )
 {
  return error_cod;
  }

