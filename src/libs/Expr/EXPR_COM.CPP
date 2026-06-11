/*
**  TAMIC_soft(R)   25.10.1991, 1992, 07.07.1999    v2.00
**
**  expr_com.     : calculation expression
**                    (string1 length n1) operation cod (string2 length n2)
**
**  FILE          :
**                  expr_com.c,
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

#define RADIAN      (_real)1.0
#define GRADUS      (_real)(PI/180.0)

static _real expr_angle = GRADUS;

void  expr_set_angle( int flag )
 {
  if( flag == EXPR_GRADUS ) expr_angle = GRADUS;
   else expr_angle = RADIAN;
  }



_real  expr_comp( int cod, char   *oper1, int n1, char   *oper2, int n2 )
 {                                             /* oper1-first operand string*/
  _real op1, op2;                              /* n1   -his size*/
   double rl;                                  /* oper2-second operand string*/
                                               /* n2   -his size*/
                                               /* cod  -operation cod*/
  if( expr_get_error_cod() != 0  ) return 0;


  if( n1 == 0 ) op1 = (_real) 0.0;             /* if n1 != 0*/
   else op1=expr_inter( oper1, n1);            /* calculation op1*/
                                               /* op1  -significance oper1*/
  if( cod > LOG10 )
   {
    if( n2 == 0 )
     {
      op2=(_real)0;
      expr_set_error_cod( ERROR_EXPRESSION );
      }                                        /* if n2 != 0*/
     else op2=expr_inter( oper2, n2);          /* calculation op2*/
    };                                         /* op2  -significance oper1*/

  switch( cod)
   {
     case '='  : if( expr_is_per( oper1, n1) != 0 )         /*if cod := op1=op2*/
		  {
		   expr_set_error_cod( ERROR_VAR );
		   return( (_real)0.0 );
		   };
		 expr_set_per( oper1, n1, op2);
		 return(  (_real)(op2) );
     case '^'  : 
//		 printf("\nop1 = %lg; op2 = %lg;", op1, op2);
//		 getch();
		 if( op1 <= (_real)0 )         /*if cod := op1^op2*/
		  {
		   expr_set_error_cod( ERROR_POW );
		   return((_real)0);
		   };
		 if( op2 > (_real)fabs(log(FLT_MAX)/log(op1)) )
		  {
		   expr_set_error_cod( ERROR_OVERFLOW );
		   return((_real)FLT_MAX);
		   };
		 return( (_real)pow( op1, op2) );
     case '+'  :                               /*if cod := op1+op2*/
		 rl= (double)op1+(double)op2;
		 if( rl > FLT_MAX )
		  {
		   expr_set_error_cod( ERROR_OVERFLOW );
		   return( (_real)FLT_MAX );
		   };
		 return( (_real)rl );
     case '-'  :                               /*if cod := op1-op2*/
		 rl = (double)op1-(double)op2;
		 if( rl > FLT_MAX )
		  {
		   expr_set_error_cod( ERROR_OVERFLOW );
		   return( (_real)FLT_MAX );
		   };
		 return( (_real)rl );
     case '*'  :                               /*if cod := op1*op2*/
		 rl = (double)op1*(double)op2;
		 if( rl > FLT_MAX )
		  {
		   expr_set_error_cod( ERROR_OVERFLOW );
		   return( (_real)FLT_MAX );
		   };
		 return( (_real)rl );
     case '/'  : if( op2 == (_real)0 )         /*if cod := op1/op2*/
		  {
		   expr_set_error_cod( ERROR_DIV_0 );
		   return((_real)FLT_MAX);
		   };
		 rl = (double)op1/(double)op2;
		 if( rl > FLT_MAX )
		  {
		   expr_set_error_cod( ERROR_OVERFLOW );
		   return( (_real)FLT_MAX );
		   };
		 return( (_real)rl );
     case EXP  :                              /*if cod := exp(op1)*/
                  if( op1 > (_real)log(FLT_MAX))
                   {
		    expr_set_error_cod( -cod );
                    return( (_real)FLT_MAX );
                    };
                  return( (_real)exp(op1));
     case SIN  :                               /*if cod := sin(op1)*/
                  if( fabs(op1) > FLT_MAX_SIN)
                   {
		    expr_set_error_cod( -cod);
                    return( (_real)0 );
                    };
		  return( (_real)sin( op1*expr_angle ) );
     case COS  :                               /*if cod := cos(op1)*/
                  if( fabs(op1) > FLT_MAX_SIN)
                   {
		    expr_set_error_cod( -cod);
                    return( (_real)0 );
                    };
		  return( (_real)cos( op1*expr_angle ) );
     case TAN  :                               /*if cod := tan(op1)*/
		 if( fabs(op1) > FLT_MAX_SIN)
		  {
		   expr_set_error_cod( -cod);
		   return( (_real)0 );
		   };
		 return( (_real)tan(op1*expr_angle ) );
     case ABS  : return( (_real)fabs(op1));    /*if cod := abs(op1)*/
     case LOG  : if( op1 <= (_real)0.0 )       /*if cod := log(op1)*/
		  {
		   expr_set_error_cod( -cod );
		   return((_real)0);
		   };
		 return( (_real)log(op1) );
     case SQRT : if( op1 < (_real)0)           /*if cod := sqrt(op1)*/
		  {
		   expr_set_error_cod( -cod );
		   return((_real)0);
		   };
		 return( (_real)sqrt(op1) );
     case SINH :                               /*if cod := sinh(op1)*/
		 if( op1 > (_real)log(FLT_MAX))
		  {
		   expr_set_error_cod( -cod );
		   return( (_real)FLT_MAX );
		   };
		 return( (_real)sinh(op1));
     case COSH :                               /*if cod := cosh(op1)*/
		 if( op1 > (_real)log(FLT_MAX))
		  {
		   expr_set_error_cod( -cod );
		   return( (_real)FLT_MAX );
		   };
		 return( (_real)cosh(op1));
     case TANH :                               /*if cod := tanh(op1)*/
		 return( (_real)tanh(op1));
     case ASIN : if( fabs(op1) > (_real)1.)    /*if cod := asin(op1)*/
		  {
		   expr_set_error_cod( -cod );
		   return((_real)0);
		   };
		 return( (_real)asin(op1)/expr_angle );
     case ACOS :                               /*if cod := acos(op1)*/
		 if( fabs(op1) > (_real)1)
		  {
		   expr_set_error_cod( -cod );
		   return((_real)0);
		   };
		 return( (_real)acos(op1)/expr_angle );
     case ATAN :                               /*if cod := atan(op1)*/
		 return( (_real)atan(op1)/expr_angle );
     case LOG10 :                              /*if cod := log10(op1)*/
		  if( op1 <= (_real)0 )
		   {
		    expr_set_error_cod( -cod );
		    return((_real)0);
		    };
		  return( (_real)log10(op1) );

     default    : expr_set_error_cod( -cod );      /*if cod := 0 & set error cod*/
		  return((_real)0);
   };
 }

