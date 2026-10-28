/*
**  TAMIC_soft(R) 1990,1991 v1.00
**
**  tmclimit.h
**
**  FILE : a:\include\tmclimit.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _TMCLIMIT_H
 #define _TMCLIMIT_H




#define FREQ_MAX_NUM      100                    /* maximal frequency number*/


  #define MAX_INPUT_NUM     20


#ifndef MAX_WINDOW_NUM
 #define MAX_WINDOW_NUM    20
#endif

#if defined _MODEL_H || defined MODEL_E
   #define MATRIX_BUFF_SIZE  0x3FFF
 #else
   #define MATRIX_BUFF_SIZE  0x1000
#endif

#define FE_ARRAY_DIM      10000
/*#define FE_ARRAY_DIM	    300 */


#endif
