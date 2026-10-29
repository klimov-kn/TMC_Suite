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




char  arif[] = "=+-*/^";

// A-9: is lex[i] ('+' or '-') the sign of the exponent of a number (1.24e+17, 0.5e-1)?
//      Yes if 'e' follows a mantissa of digits and '.', the mantissa starts the string
//      or follows an operation or a bracket, and a digit follows the sign.
//      "x1e+2" is a name x1e plus 2 - not an exponent.
static int expr_is_exp_sign( char *lex, int i, int n )
 {
  int k, nDigit = 0;
  if( (i < 2)||(i+1 >= n) ) return 0;
  if( lex[i-1] != 'e' ) return 0;
  if( (lex[i+1] < '0')||(lex[i+1] > '9') ) return 0;
  for( k = i-2; (k >= 0)&&( ((lex[k] >= '0')&&(lex[k] <= '9'))||(lex[k] == '.') ); k-- )
    if( lex[k] != '.' ) nDigit++;
  if( nDigit == 0 ) return 0;
  if( k < 0 ) return 1;
  if( strchr( "=+-*/^({", lex[k] ) != NULL ) return 1;
  return 0;
  }

_real  expr_inter( char   *lex, int n )
 {
  int    i, j, i1, j1;
  int    k = -1;   // B-23: position of the leftmost '^' of the level
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
    if( d > REAL_MAX )	// I-03 (old: FLT_MAX)
     {
      expr_set_error_cod( ERROR_OVERFLOW );
      return( (_real)FLT_MAX );
      };
    return((_real) d);
    };

  lex[n] = ch;

  if( n <= 0 )                                          // N-30: "()" was silently 0
   {
    expr_set_error_cod( ERROR_EXPRESSION );
    return( (_real)0 );
    };

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
                           if( ((i1 == 1)||(i1 == 2))&&(i > 0)&&(lex[i-1] != '\0')&&(strchr( "=+-*/^", lex[i-1] ) != NULL) )
                             break;   // S-22: a unary sign after an operation (2^-1, 2*-3) is not a split point
                           if( ((i1 == 1)||(i1 == 2))&&(expr_is_exp_sign( lex, i, n ) != 0) )
                             break;   // A-9: the sign of an exponent (1.24e+17) is not a split point
                           if( (i1 == 5)&&(j == 0)&&(j1 == 0) )
                            {
                             k = i;   // B-23: remember, the scan goes on to the left -> the leftmost '^'
                             break;
                             };
                           if( (j == 0)&&(j1 == 0) ) return( expr_comp( LEXI, lex, i, lex+i+1, n-i-1));
                           };
        };
      };
     if( j != 0 )                                  /*if no pair bracket*/
      {
       expr_set_error_cod( ERROR_BRACKET );
       return( (_real)0 );
       };
     // B-23: a^b^c = a^(b^c). Before, the rightmost '^' was taken: (a^b)^c.
     if( (i1 == 5)&&(k >= 0) ) return( expr_comp( '^', lex, k, lex+k+1, n-k-1));
    };

  if( ( lex[0] == '(') && ( lex[n-1] == ')') )   /*for expression in bracket*/
   {
    return( expr_inter( lex+1, n-2) );
    };

  // I-04: compare a name only if the substring is not shorter than it (old: lex[4..6] = 0 for any n)
  /// I-04: имя функции длины 4, 5, 6 сравнивается, только если подстрока не короче (n >= 4, 5, 6).
  if( n >= 4 )
  {
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
  };

 if( n >= 5 )	// I-04
 {
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
 };

 if( n >= 6 )	// I-04
 {
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
 };


  expr_set_error_cod( ERROR_FUNCTION );                 /*set error cod*/
  return( (_real)0.0 );
  }

int  expr_numbern( char *lex, int n)    /*define number*/
 {                                                  /*expr_numbern=0,*/
  int i, k=0, ie=0, it=0;                           /*if expression is*/
  int nSign = 1;                                    // A-9: 0 - the exponent has no sign (1e18)
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
                 nSign = 1;
                 switch( LEXI )
                  {
                   case '+' : break;
                   case '-' : break;
                   default  : if( (LEXI >= '0')&&(LEXI <= '9')&&(i >= 2)&&
                                  (((lex[i-2] >= '0')&&(lex[i-2] <= '9'))||(lex[i-2] == '.')) )   // A-9: 1e18 = 1e+18; e1 is a name
                               {
                                i--;
                                nSign = 0;
                                break;
                                };
                              return(1);
                   };
                  switch( ie )
                   {
                    case  0 : it = 1;
                              ie = 1;
                              break;
                    case  1 : return(1);
                    };
                   switch( n-k+1-nSign )               // A-9: 1..3 digits of the exponent, with or without a sign
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
   for( i = 0; (i < n)&&((LEXI < '0')||(LEXI > '9')); i++ ) ;   // N-30: "", "-", "." are not numbers
   if( i >= n ) return(1);
   return(0);
  }
