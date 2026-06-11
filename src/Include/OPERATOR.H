/*
**  TAMIC_soft(R) 1990 v1.00
**
** Macros and decloration for Tamic_H topology programms
**
** FILE: a:\include\topology.h>
**
*/

#ifndef _OPERATOR_H
 #define _OPERATOR_H

 #ifndef _GP_POLYGON_DEF
  #define _GP_POLYGON_DEF
 struct poly_line_attr
	 {
	  _real  x, y;                       // point coordinates
	  int    bound;                      // boundary type: input if < 0, internal XX - 0, metal > 0
	  int    mode;                       // mode number if input
	  _real  e, m;                       // boundary E,M
	  struct poly_line_attr _far *next;  // pointer to next line
	  };
 typedef struct poly_line_attr POLY_LINE_ATTR;

 struct gp_polygon
	  {
	   int   code;
	   _real e, m;                        // E, M
	   int   num;                         // polygon number
	   POLY_LINE_ATTR _far *pla;          // pointer to line structure
	   struct gp_polygon _far *next;      // pointer to next polygon
	   Uint lw_num;                       // left window number for INPUT, WAVEG ...
	   Uint rw_num;                       // right window number for INPUT, WAVEG ...
	   };

 typedef struct gp_polygon GP_POLYGON;
 #endif
 #ifndef _FLOAT_POINTTYPE
  #define _FLOAT_POINTTYPE
  struct float_pointtype
             {
              float x;
              float y;
              };
 #endif




#ifndef _OPERATOR_DEF
 #define _OPERATOR_DEF
struct matrix
        {
	 _real a11, a12, a21, a22;
         };


typedef struct
        {
	 _real x, y;
	 } VECTOR;

struct operator1
        {
	 int                   block_num;   // block number to be tramsformed
	 int                   x_flag;      // x & y sim. transform flag
	 int                   y_flag;
	 _real                 phi;         // angle for turne block
	 VECTOR                bp;          // base point for rotation
	 struct matrix         m;           // transform matrix of angle phi
	 VECTOR                v;           // shift block vector
	 struct operator1 _far *next;        // pointer to next operator structure
         };
typedef struct operator1 OPERATOR1;
#endif

typedef struct
	 {
	  VECTOR  v0;
	  VECTOR  v1;         // start & end input coordinates
	  int     parm;       // mesh points or modes for this input
	  }INPUT_DATA;


OPERATOR1 _far * fortran get_operator_first_ptr( void );
void     _far   fortran set_operator_first_ptr( OPERATOR1 _far *ptr );
OPERATOR1 _far * fortran get_operator_last_ptr( void );
void     _far   fortran set_operator_last_ptr( OPERATOR1 _far *ptr );

int      _far   fortran _polygon_transformation( GP_POLYGON _far *gp_pol,
						 OPERATOR1 _far *oper_first,
						 int block_num );

int      _far   fortran polygon_transformation( struct float_window _far *wf,
						GP_POLYGON _far *gp_pol,
						OPERATOR1   _far *oper_first,
						int block_num, int transf_flag );
int      _far   fortran invers_polygon_transformation( GP_POLYGON _far *gp_pol,
						       OPERATOR1 _far *oper_first,
						       int block_num );
void     _far   fortran make_transformation( _real _far *x, _real _far *y, OPERATOR1 _far *oper );

void     _far   fortran transformation( OPERATOR1   _far *lpOper_1,
					OPERATOR1   _far *lpOper_2,
					INPUT_DATA _far *id1,
					INPUT_DATA _far *id2      );

int      _far   fortran make_operator( LINK_LIST  _far *ll,
                                       INPUT_DATA _far *id1,
                                       INPUT_DATA _far *id2 );
#endif

