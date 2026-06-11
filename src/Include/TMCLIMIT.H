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

#ifdef TMC_TWP
  #define DATA_BASE_SIZE    0x7FFF
#else
  #define DATA_BASE_SIZE    0x1FFF
#endif


#define MAX_STRING_LEN    512
#define DAMP_LENGTH      2048
#define SCREEN_STRING_LEN 60

#define FREQ_MAX_NUM      100                    /* maximal frequency number*/

#define MEDEA_TABLE_SIZE   12
#define TOLER      (_real)1.0e-3

#ifndef _MODEL_3D
  #define MAX_INPUT_NUM     20
 #else
  #define MAX_INPUT_NUM      6
  #define MAX_INPUT_NODE   400
#endif

#ifdef _MODEL_M
 #define MAX_ELEMENT_NUM    1024
 #define MAX_MODE_NUM       400
// #define FE_ARRAY_DIM      10000
#endif

#ifndef MAX_WINDOW_NUM
 #define MAX_WINDOW_NUM    20
#endif

#if defined _MODEL_H || defined MODEL_E
   #define MATRIX_BUFF_SIZE  0x3FFF
 #else
   #define MATRIX_BUFF_SIZE  0x1000
#endif

#define MAX_AGNV_NUM      200
#define GP_NUM_MAX        100
#define DATA_LINK_SIZE    2000
#define DATA_TRN_SIZE     3000
#define FE_ARRAY_DIM      10000
/*#define FE_ARRAY_DIM	    300 */

#define MAX_GPC1_POLYGON  500

#endif
