/*
**  TAMIC_soft(R)        25.10.1991 ,1992, 07.07.1999          v2.00
**
**  get_expr_err     : get error code with massage
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
#include <error1.h>

#include <expr.h>

int   get_expr_err( char  *lpszExpr )
 {
  int nTmp = -expr_get_error_cod();

  switch( nTmp )
   {
    case 0: return 0;
    default:if( nTmp > 20 ) nTmp = 21;
            nTmp += 2400;
 //           put_error_messege( nTmp, lpszExpr );
            return -nTmp;
    }
  }
