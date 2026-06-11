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

#include <typedef.h>
#include <tmc_lib.h>
#include <expr.h>

#define   LEXI *(lex+i)

static char   szABS[]   = "abs(" ;
static char   szEXP[]   = "exp(" ;
static char   szLOG[]   = "log(" ;
static char   szSIN[]   = "sin(" ;
static char   szCOS[]   = "cos(" ;
static char   szTAN[]   = "tan(" ;
static char   sz1111[]  = "1111" ;
static char   szSQRT[]  = "sqrt(";
static char   szSINH[]  = "sinh(";
static char   szCOSH[]  = "cosh(";
static char   szTANH[]  = "tanh(";
static char   szASIN[]  = "asin(";
static char   szACOS[]  = "acos(";
static char   szATAN[]  = "atan(";
static char   szLOG10[] = "log10(";

static char   *fun[15] = {
                  szABS,
                  szEXP,
                  szLOG,
                  szSIN,
                  szCOS,
                  szTAN,
                  sz1111,
                  szSQRT,
                  szSINH,
                  szCOSH,
                  szTANH,
                  szASIN,
                  szACOS,
                  szATAN,
                  szLOG10 };


_real  expr_atof( char  *string )
 {
 // del_blanks( string );
  expr_set_error_cod( 0 );
  return( expr_inter( string, strlen( string )) );
  }


char  arif[] = "=+-*/^";

_real  expr_inter( char   *lex, int n )
 {
  int    i, j, i1, j1;
  char   ch;
  double d;

  if( expr_get_error_cod() != 0  ) return 0;

  ch = lex[n];
  lex[n] = 0;

  j = expr_numbern( lex, n );

  if( j == 0 )                                          /*is string number?*/
   {
    d =  atof( lex );
    lex[n] = ch;
    if( d > FLT_MAX )
     {
      expr_set_error_cod( ERROR_OVERFLOW );
      return( (_real)FLT_MAX );
      };
    return((_real) d);
    };

  lex[n] = ch;

if( expr_is_per( lex, n) == 0 )                         /*is string var?*/
 {
	return( (_real)expr_get_per( lex, n) );
 }

  j = 0; j1 = 0;
  for( i1 = 0; i1 < 6; i1++)                         /*is string 'a=b' or*/
   {                                                 /*          'a+b' or*/
    for( i = n-1; i >= 0; i--)                       /*          'a-b' or*/
     {                                               /*          'a*b' or*/
      switch( LEXI)                                  /*          'a/b' or*/
       {                                             /*          'a^b'   */
        case '('       : j++;
                         break;
        case ')'       : j--;
                         break;
        case '{'       : j1++;
                         break;
        case '}'       : j1--;
                         break;
        default        : if( arif[i1] == LEXI )
                          {
                           if( (j == 0)&&(j1 == 0) ) return( expr_comp( LEXI, lex, i, lex+i+1, n-i-1));
                           };
        };
      };
     if( j != 0 )                                  /*if no pair bracket*/
      {
       expr_set_error_cod( ERROR_BRACKET );
       return( (_real)0 );
       };
    };

  if( ( lex[0] == '(') && ( lex[n-1] == ')') )   /*for expression in bracket*/
   {
    return( expr_inter( lex+1, n-2) );
    };

  ch = lex[4];
  lex[4] = '\0';

  for( i = 0; i < 6; i++)                           /*define function*/
   {
    if( strcmp( fun[ i ], lex) == 0 )
     {
      lex[4] = ch;
      return( expr_comp(i, lex+3+(int)(i/7), n-3-(int)(i/7), lex, 0));
      };
    };

  lex[4] = ch;

 ch=*(lex+5);
 *(lex+5)='\0';

 for( i = 7; i < 14; i++)                           /*define function*/
  {
    if( strcmp( fun[ i ], lex) == 0 )
      {
       lex[5] = ch;
       return( expr_comp(i, lex+3+(int)(i/7), n-3-(int)(i/7), lex, 0));
       };
  };
  *(lex+5)=ch;

 ch=*(lex+6);
 *(lex+6)='\0';

 for( i = 14; i < 15; i++)                           /*define function*/
  {
    if( strcmp( fun[ i ], lex) == 0 )
      {
       lex[6] = ch;
       return( expr_comp(i, lex+3+(int)(i/7), n-3-(int)(i/7), lex, 0));
       };
  };
  *(lex+6)=ch;


  expr_set_error_cod( ERROR_FUNCTION );                 /*set error cod*/
  return( (_real)0.0 );
  }

int  expr_numbern( char *lex, int n)    /*define number*/
 {                                                  /*expr_numbern=0,*/
  int i, k=0, ie=0, it=0;                           /*if expression is*/
                                                    /*numbern,else*/
  if( n > 40 ) return( 1 );                         /*expr_numbern=1*/
  
  i = 0;
  if( LEXI == '-' )	
  {
	  i = 1;
	  k = 1;
  };
  
  for( ; i < n; i++ )
   {

    switch( LEXI )
     {
      case '.' : switch( it )
                  {
                   case 0 : it = 1;
                            break;
                   case 1 : return(1);
                   };
      case '0' : ;
      case '1' : ;
      case '2' : ;
      case '3' : ;
      case '4' : ;
      case '5' : ;
      case '6' : ;
      case '7' : ;
      case '8' : ;
      case '9' : k++; break;
      case 'e' : i++;
                 switch( LEXI )
                  {
                   case '+' : break;
                   case '-' : break;
                   default  : return(1);
                   };
                  switch( ie )
                   {
                    case  0 : it = 1;
                              ie = 1;
                              break;
                    case  1 : return(1);
                    };
                   switch( n-k )
                    {
                     case  3 : ;
                     case  4 : ;
                     case  5 : break;
                     default : return(1);
                     };
                    break;
      default : return(1);
     };
   };
   return(0);
  }
