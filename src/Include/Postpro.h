/*
**  TAMIC_soft(R) 1989,1990,1991,1992 v1.10
**
**  postpro.
**
**  FILE : a:\include\postpro.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#include <typedef.h>
#include <complex1.h>
#include <frqdata.h>
#include <tmcgraph.h>






#define LAST               2





#ifndef _INPUT_DATA_DEF
 #define _INPUT_DATA_DEF
 typedef struct
          {
           INT_1 num;
           INT_1 array[MAX_INPUT_NUM];
           INT_1 dim;
           }INPUT_DATA;
#endif


#ifndef _S_INDEX_DEF
 #define _S_INDEX_DEF
   typedef struct
	   {
	    char *tpl_name;
	    char *S_name;
	    double delta1;
	    double delta2;
	    double delta3;
	    } S_INDEX;
#endif

#ifndef _MARKER
  #define _MARKER

typedef struct
            {
             int type;
             int width;
             int color;
             int size;
             } MARKER;
#endif

#ifndef _CURVE_INDEX
 #define _CURVE_INDEX

typedef struct
            {
             int style;
             int color;
             int width;
             } CURVE;

typedef struct
          {
           int input1;       /* input 1 number in device    */
           int mode1;        /* mode 1 number in this input */
           int input2;       /* input 2 number in device    */
           int mode2;        /* mode 2 number in this input */
           } CURVE_INDEX;
#endif

#ifndef GRAPH_CURVE
struct graph_curve
            {
             int    code;        /* -1 - deleted, 0 - not used, >0 current used*/
             MARKER m;
             CURVE  crv;
             CURVE_INDEX ci;
             INPUT_DATA id;
             char   filename[FILENAME_MAX];
             struct graph_curve *next;
             struct graph_curve *prev;
             };
typedef struct graph_curve GRAPH_CURVE;
#endif


void put_S_table( _complex *lpxS, int nInp_num, int *modnb );

