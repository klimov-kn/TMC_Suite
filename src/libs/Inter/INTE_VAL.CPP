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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <conio.h>

#include <typedef.h>
#include <tmc_lib.h>
#include <inter.h>


static _real inter_value = 0;
void  inte_set_inter_value( _real r )
{
	inter_value = r;
}
_real  inte_get_inter_value( void )
{
	return inter_value;
}

