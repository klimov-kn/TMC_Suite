/*
**  TAMIC_soft(R)   25.10.1991, 1998, 07.07.1999    v2.00
**
**  expr_get_per
**  expr_set_per
**  expr_is_per
**
**
**
**  FILE          :
**                  expr_per.c,
**  HEDER FILE'S  :
**                  <string.h>
**                  <stdio.h>
**                  <stdlib.h>
**                  <math.h>
**                  <float.h>
**                  <typedef.h>
**                  "expr.h"
**
**  LIB           : NO
**
**  OVERLAY       : NO
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

#include <typedef.h>
#include <expr.h>


static _real rExpr_per[256];

_real  expr_get_per( char   *lpszName, int nLenghtName )
{
	int i;
    i = lpszName[0];
	return ( rExpr_per[i] );
}

void  expr_set_per( char   *lpszName, int nLenghtName, _real rValue )
{
	int i;
    i = lpszName[0];
    rExpr_per[i] = rValue;
}

int  expr_is_per( char   *lpszName, int nLenghtName )
{
	char ch;

	if( nLenghtName != 1 ) return -1;
	ch = lpszName[0];
	if( ((ch >= 'A')&&(ch <= 'Z')) || ((ch >= 'a')&&(ch <= 'z')) ) return 0;

	return -1;
}
