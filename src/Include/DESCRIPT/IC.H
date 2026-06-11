/*
**  TAMIC_soft(R) 1991,1992 vR1.02
**
**  Language descriptors definitions for IC program.
**
**  FILE : include\descript\ic.h
**
*/

#ifndef _DSCR_IC_H
 #define _DSCR_IC_H

                        /* sections descriptors */
#define M_TITLE                    2
#define M_END_TITLE                3
#define M_PARMS                    4
#define M_END_PARMS                5
#define M_SUMS                     6
#define M_UNFOLDED                27
#define M_LINE                     8
#define M_DISCONTINUITY            9
#define M_COMENT                  13
#define M_END_COM                 14
#define M_TMC_I                   15
#ifndef M_ATTRIBUTES
 #define M_ATTRIBUTES              16
#endif
#ifndef M_END_ATTRIBUTES
 #define M_END_ATTRIBUTES          17
#endif
#define M_PACKAGE                 18
#define M_END_PACKAGE             19
#define M_TOPOLOGY                20
#define M_END_TOPOLOGY            21
#define M_ELEMENT_GEOMETRY        22
#define M_END_ELEMENT_GEOMETRY    23
#define M_DB                      24
#define M_END_DB                  25


#ifndef M_ERROR
 #define M_ERROR     -1
#endif
#ifndef M_WORNING
 #define M_WORNING   -2
#endif

#define M_POLY  (-78)


#define M_FRQS       0
#define M_FRQS_U     1
#define M_FRQU       1
#define M_F_UNIT     1
#define M_TOLER      2
#define M_DELTA      4
#define M_LONG_U     7

                     /* package */
#define M_TOP_W      8
#define M_BOTTOM_W   9
#define M_MEDIA     10
#define M_LAYER     11
#define M_REGION    12
#define M_SIDEWALLS 13

                       /* topology descriptor data */
#define M_TLAYER    15
#define M_END_LAYER 16
#define M_L         19
#define M_RECT      21
#define M_END_GP    18
#define M_POLYGON   17
#define M_CIRCLE1   20
#define M_CIRCLE2   33
#define M_RING      44
#define M_INPUT     22
#define M_LUMPED    23
#define M_W         24


                       /* output */
#define M_INP_TABLE 31
#define M_END_TABLE 32

                        /* attributes section */

#define M_INP_NUM   14



#endif
