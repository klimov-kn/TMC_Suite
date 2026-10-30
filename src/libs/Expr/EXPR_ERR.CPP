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

/// @file EXPR_ERR.CPP
/// @brief Хранение кода ошибки вычислителя: `expr_set_error_cod`, `expr_get_error_cod`.
///
/// Код хранится в контексте потока, если он привязан (`expr_thread_ctx_begin`),
/// иначе — в общей статической переменной.
/// @see https://www.tamic.ru/docs/code/lib-expr.html#errors
#include <Typedef.h>

#include <EXPR.H>

/// Общий код ошибки вычислителя (используется, когда контекст потока не привязан).
static int error_cod = 0;

void  expr_set_error_cod( int n )
 {
  EXPR_THREAD_CTX *p = expr_get_thread_ctx();	// EPSFAST
  if( p != NULL ) p->nErrorCod = n;
  else error_cod =  n;
  }
int   expr_get_error_cod( void )
 {
  EXPR_THREAD_CTX *p = expr_get_thread_ctx();	// EPSFAST
  if( p != NULL ) return p->nErrorCod;
  return error_cod;
  }

