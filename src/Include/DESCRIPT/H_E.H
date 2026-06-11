/*
**  TAMIC_soft(R) 1990 v1.00
**
**  descrptr.h
**
**  FILE : a:\include\descrptr.c
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _DSCR_H_E_H
 #define _DSCR_H_E_H

                        /* sections descriptors */
#define M_TITLE      2
#define M_END_TITLE  3
#define M_PARMS      4
#define M_END_PARMS  5
#define M_TOPOLOGY   6
#define M_END_TOP    7
#define M_LINK_LIST  8
#define M_END_LINK   9
#define M_COMENT    13
#define M_END_COM   14

#define M_ERROR     -1
#define M_WORNING   -2


#if !defined _MODEL_W && !defined _MODEL_M
  #define M_WVGD     0
  #define M_STAB     0
#endif

#define M_POLY  (-78)


#define M_FRQS       0
#define M_FRQS_U     1
#define M_FRQU       1
#define M_TOLER      2
#define M_FILE_      3
#define M_DELTA      4
#define M_MU         5
#define M_EPS        6
#define M_LONG_U     7

#if !defined _MODEL_W && !defined _MODEL_M
  #define M_INPUT_1    8
  #define M_LW         9
  #define M_RW        10
  #define M_STAB_     11
  #define M_WVGD_1    12
  #define M_EXT       13
 #else
  #define M_WAVEG      8
#endif

#define M_INP_NUM   14
#define M_BLOCK     15
#define M_END_BLOCK 16

#if !defined _MODEL_W
  #define M_GPGN      17
#endif

#define M_END_GP    18

#if !defined _MODEL_W && !defined _MODEL_M
  #define M_LINE      19
  #define M_GPC1      20
#endif
  #define M_TIGH      21
#if !defined _MODEL_W && !defined _MODEL_M
  #define M_INPUT_2   22
  #define M_INPUT_3   23
  #define M_INPUT_4   24
  #define M_WVGD_2    25
  #define M_WVGD_3    26
  #define M_WVGD_4    27
#endif
  #define M_SIM       29
  #define M_INV       30
  #define M_INP_TABL  31
  #define M_END_TABL  32
#if !defined _MODEL_W && !defined _MODEL_M
  #define M_GPC2      33
  #ifdef _MODEL_H
  #define M_HEIGHT    34
  #endif
  #ifdef _MODEL_E
  #define M_WIDTH     34
  #endif
  #define M_OPSTB_1   35
  #define M_OPSTB_2   36
  #define M_OPSTB_3   37
  #define M_OPSTB_4   38
#endif
  #define M_ACURACY   39
#if !defined _MODEL_W && !defined _MODEL_M
  #define M_SHSTB_1   40
  #define M_SHSTB_2   41
  #define M_SHSTB_3   42
  #define M_SHSTB_4   43
  #define M_RING      44
#endif

#endif
