/*
**  TAMIC_soft(R) 1989,1991 v1.10
**
**  def_mcci.h
**
**  FILE : ..\include\def_mcci.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

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

		 /*  X-coordinate system */
#define _FRQNS             0
#define _WVLNG             1

		 /*  Y-coordinate system */
#define _KCTU              0
#define _LOSS              1
#define _SMOD              2
#define _FI                3
#define _FI_DGR            4
#define _FI_PLUS           5
#define _FI_DGR_PLUS       6


MARKER      _far * fortran get_default_marker( MARKER _far *m );
CURVE       _far * fortran get_default_curve( CURVE _far *c );

CURVE_INDEX _far * fortran get_default_curve_index( CURVE_INDEX _far *c );
void        _far   fortran set_default_curve_index( int in1, int m1, int in2, int m2 );

void        _far   fortran set_x_scale( int scale );
void        _far   fortran set_y_scale( int scale );
int         _far  _fortran get_y_scale( void );
