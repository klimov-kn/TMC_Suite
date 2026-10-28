/*
**  TAMIC_soft(R) 1990,1991 v1.20
**
** Macros and decloration for run  programms.
**
** FILE: a:\include\run.h
*/

#ifndef _RUN_H
 #define _RUN_H

 #ifndef _ELEM_VAL
  #ifdef _ELEM_COMPLEX
   #define _ELEM_VAL _complex
  #else
   #define _ELEM_VAL _real
  #endif
 #endif

#ifndef _INP_ADRESS
 #define _INP_ADRESS

 typedef struct
         {
          short block;
          short input;
          } INP_ADRESS;

#endif

#ifndef _Y_MATRIX_DEF
 #define _Y_MATRIX_DEF

typedef struct
        {
         short        num;                                       /* number of input*/
         short        norm_ptr[MAX_INPUT_NUM];                   /* number of mode in input*/
         _real      norm_array[MAX_INPUT_NUM][MAX_INPUT_NUM];  /* norm. coeff.*/
         short        flag[MAX_INPUT_NUM];                       /* +1 - no norm., -1 yes norm.*/
         INP_ADRESS ia[MAX_INPUT_NUM];                         /* input adress*/
         } INPUT;

typedef struct
         {
          _ELEM_VAL *matr;               /* pointer to Y-matrix buffer*/
//          float       _far *matr;               /* pointer to Y-matrix buffer*/
          short             norm_flag;          /* norm_flag: 0 - norm. not available*/
                                              /*            1 - available but not norm*/
                                              /*            2 - norm*/
          INPUT           inp;                /* input data*/
          } Y_MATRIX;
#endif
#include <linklist.h>


#ifndef _MODUL_GP
 #define _MODUL_GP

struct modul_GP
	{
         int                   code;        /* entity's code*/
         int                   num;         /* block number*/
         void            *gp;          /* pointer to this entity*/
         struct modul_GP *next;        /* pointer to next entity*/
	 };
typedef struct modul_GP MODUL_GP;
#endif


int  modul( int modul_num, Y_MATRIX *matr, MODUL_GP * ptr );

#endif
