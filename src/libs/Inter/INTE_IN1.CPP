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

#define INTER_BLANCK ';'
#define INTER_BRACKET1 '{'
#define INTER_BRACKET2 '}'
#define INTER_BRACKET11 '('
#define INTER_BRACKET12 ')'

int expr_del_Blanks1( char *ch )
{
	int i, i1;
	for( i = 0, i1 = 0;  i >= 0 ; i++  )
	{
		switch( ch[i] )
		{
			case ' ':  
				break;
			case '\t':	
				break;
			case '\n':
				break;
			default:
				ch[i1] = ch[i];
				i1++;
				if( ch[i] == '\0' ) 
				{
					return 0;
				};
		};
	};
	return 0;
}
	   
int  i1nte_atof_1( char *string, double *r )
{
	expr_del_Blanks1( string  );
	*r  =  (double)(inte_atof( string ));
//	printf("\n{%s};r = %lg;\nError = %d", string, *r, expr_get_error_cod());
	return expr_get_error_cod();

}

_real  inte_atof( char  *string )
 {
	_real d;
 // del_blanks( string );
  expr_set_error_cod( 0 );

  d = inte_inter( string, strlen( string ));
  if(expr_get_error_cod() == ERROR_RETURN) 
  {
	  d = inte_get_inter_value();
      expr_set_error_cod( 0 );
  }
  
  return( d );
  }



_real  inte_inter( char   *lex, int n )
 {
  int    i, j, j1;
//  char   ch;
//  int i1;
  double d;

//  printf("\n1lex={%s}, n={%d}", lex, n);
//  getch();
  if( expr_get_error_cod() != 0  ) return 0;
  if( ( n == 1 )&&( lex[0] == INTER_BLANCK ) ) return 0;
  if( n == 0 ) return 0;

  if( ( lex[0] == INTER_BRACKET1) && ( lex[n-1] == INTER_BRACKET2) )   /*for expression in bracket*/
   {
    return( inte_inter( lex+1, n-2) );
    };

  j = 0;
    for( i = 0, j = 0, j1 = 0; i < n; i++)                /* select blanck  */
     {                                               
      switch( lex[i] )                                  
       {                                             
        case INTER_BRACKET1  : j++; break;
        case INTER_BRACKET2  : j--; break;
        case INTER_BRACKET11 : j1++; break;
        case INTER_BRACKET12 : j1--; break;
        case INTER_BLANCK    : if( (j == 0)&&(j1 == 0) )
							     {
								   d = inte_inter( lex, i);
								   if( i != (n-1) ) d =  inte_inter( lex+i+1, n-i-1);
								   return d;
							      };

        };
      };
     if( (j != 0)||(j1 != 0) )                                  /*if no pair bracket*/
      {
       expr_set_error_cod( ERROR_BRACKET );
       return( (_real)0 );
       };

  			                            				/*is string arif expression?*/
  d = expr_inter( lex, n );                             
  if( expr_get_error_cod() == 0 ) return d;             /*is string bad arif expression?*/
  if( expr_get_error_cod() != ERROR_FUNCTION ) return (_real)0;
  expr_set_error_cod( 0 );

    							/*is string operator?*/
  d = inte_oper( lex, n );
  if( (expr_get_error_cod() == 0)||(expr_get_error_cod() == ERROR_RETURN) ) return d;
  expr_set_error_cod( 0 );

  
  expr_set_error_cod( ERROR_FUNCTION );                 /*set error cod*/
  return( (_real)0.0 );
  }

