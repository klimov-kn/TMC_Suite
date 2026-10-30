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

/// @file INTE_OPE.CPP
/// @brief Операторы интерпретатора `return`, `if`, `while`: `inte_oper`, `inte_select_blanck`.
/// @see https://www.tamic.ru/docs/code/lib-inter.html#operators
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include <Typedef.h>
#include <TMC_LIB.H>
#include <INTER.H>

/// Таблица операторов: 0 — `return(`, 1 — `if(`, 2 — `while(`.
INTE_OPER lpsOper[]=
                {
					{"return(", 7},
					{"if(", 3},
					{"while(", 6},
	             };

_real  inte_oper( char   *lex, int n )
 {
  int    i, j;
//  char   ch;
//  int i1;
  double d;


  // return(e): значение e запоминается, код ERROR_RETURN прерывает разбор до inte_atof
  i = 0;
  if( strncmp( lpsOper[i].lpszName, lex, lpsOper[i].nLenght ) == 0)
  {
	  d = inte_inter( lex + lpsOper[i].nLenght, n - lpsOper[i].nLenght - 1 );
	  if( expr_get_error_cod() == 0 )
	  {
		  expr_set_error_cod( ERROR_RETURN );
		  inte_set_inter_value( d );
		  return d;	  
	  }
	  return 0.0;	  
  }


  // if(c)S: S выполняется, если c >= 0; иначе результат — значение c
  i = 1;
  if( strncmp( lpsOper[i].lpszName, lex, lpsOper[i].nLenght ) == 0)
  {
	  j = inte_select_blanck( lex + lpsOper[i].nLenght, n - lpsOper[i].nLenght ) + lpsOper[i].nLenght;
	  d = inte_inter( lex + lpsOper[i].nLenght, j - lpsOper[i].nLenght );
	  if( expr_get_error_cod() != 0 ) return 0.;
	  if( d < 0.0 ) return d;
	  d = inte_inter( lex + j + 1, n - j - 1 );
	  if( expr_get_error_cod() != 0 ) return 0.;
	  return d;	  
  }

  // while(c)S: S выполняется, пока c >= 0 (не более 10 000 000 раз)
  i = 2;
  if( strncmp( lpsOper[i].lpszName, lex, lpsOper[i].nLenght ) == 0)
  {
	  j = inte_select_blanck( lex + lpsOper[i].nLenght, n - lpsOper[i].nLenght ) + lpsOper[i].nLenght;
	  d = inte_inter( lex + lpsOper[i].nLenght, j - lpsOper[i].nLenght );
	  if( expr_get_error_cod() != 0 ) return 0.;
	  if( d < 0.0 ) return d;
	  long nIter = 0;   // N-30: while(1) hung the kernel
	  while( d >= 0.0 )
	  {
		  if( ++nIter > 10000000L ) { expr_set_error_cod( ERROR_EXPRESSION ); return 0.; }
		  d = inte_inter( lex + j + 1, n - j - 1 );
		  if( expr_get_error_cod() != 0 ) return 0.;
		  d = inte_inter( lex + lpsOper[i].nLenght, j - lpsOper[i].nLenght );
		  if( expr_get_error_cod() != 0 ) return 0.;
	  }
	  return d;	  
  }


  expr_set_error_cod( ERROR_FUNCTION );                 /*set error cod*/
  return( (_real)0.0 );
  }

int  inte_select_blanck( char   *lex, int n )
 {
  int    i, j, j1;

  for( i = 0, j = 0, j1 = 0; i < n; i++)
  {
	  switch(lex[i])
	  {
	  case ')': if( (j == 0)&&(j1 == 0) ) return i;
		  else j++; break;
	  case '(': j--; break;
	  case '{': j1++; break;
	  case '}': j1--; break;
	  }
  }
  return i;
  }

