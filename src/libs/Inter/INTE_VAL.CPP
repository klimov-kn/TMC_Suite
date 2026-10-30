/*
**  TAMIC_soft(R) 18.10.1991, 1992, 07.07.1999 v2.00
**
**  expr_inter.   : calculation string length n
**
**  FILE          :
**                  expr_int.c,
**  HEDER FILE'S  :
**                  <string.h>
**                  <stdio.h>
**                  <stdlib.h>
**                  <math.h>
**                  <typedef.h>
**                  "expr.h"
**
**  LIB           : NO
**
**  OVERLAY       : NO
**
*/

/// @file INTE_VAL.CPP
/// @brief Хранение значения оператора `return`: `inte_set_inter_value`, `inte_get_inter_value`.
/// @see https://www.tamic.ru/docs/code/lib-inter.html#operators
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include <Typedef.h>
#include <TMC_LIB.H>
#include <INTER.H>


/// Общее значение последнего `return(...)` (используется без контекста потока).
static _real inter_value = 0;
void  inte_set_inter_value( _real r )
{
	EXPR_THREAD_CTX *p = expr_get_thread_ctx();	// EPSFAST
	if( p != NULL ) p->rInterValue = r;
	else inter_value = r;
}
_real  inte_get_inter_value( void )
{
	EXPR_THREAD_CTX *p = expr_get_thread_ctx();	// EPSFAST
	if( p != NULL ) return p->rInterValue;
	return inter_value;
}

