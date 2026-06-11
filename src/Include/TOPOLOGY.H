/*
**  TAMIC_soft(R) 1990 v1.00
**
** Macros and decloration for Tamic_H topology programms
**
** FILE: a:\include\topology.h>
**
*/

#ifndef _TOPOLOGY_H
 #define _TOPOLOGY_H

#ifndef _FLOAT_POINTTYPE
 #define _FLOAT_POINTTYPE
  struct float_pointtype
             {
              float x;
              float y;
              };
#endif



struct topology_data
        {
         struct float_pointtype max;
         struct float_pointtype min;
         int dflt_ntm;
         int current_ntm;
         };

#ifndef _MODEL_W
#ifndef _MODEL_M

struct bound_type
        {
         int color;
         int thickness;
         int style;
         };

#ifndef _OPERATOR_DEF
 #define _OPERATOR_DEF
struct matrix
        {
         float a11, a12, a21, a22;
         };


struct vector
        {
         float x, y;
         };
typedef struct vector VECTOR;

struct operator1
        {
         int block_num;              /* block number to be tramsformed*/
         int x_flag, y_flag;         /* x & y sim. transform flag*/
         float phi;                  /* angle for turne block*/
         VECTOR bp;                  /* base point for rotation*/
         struct matrix m;            /* transform matrix of angle phi*/
         VECTOR v;                   /* shift block vector*/
         struct operator1 _far *next; /* pointer to next operator structure*/
         };
typedef struct operator1 OPERATOR1;
#endif
struct top_input_data
        {
         VECTOR v0, v1;              /* start & end input coordinates*/
         int parm;                   /* mesh points or modes for this input*/
         };

#ifndef _MODUL_GP
 #define _MODUL_GP

struct modul_GP
	{
         int                   code;        /* entity's code*/
         int                   num;         /* block number*/
         void            _far *gp;          /* pointer to this entity*/
         struct modul_GP _far *next;        /* pointer to next entity*/
	 };
typedef struct modul_GP MODUL_GP;
#endif

void _far init_topology_data( void );
int  _far complex_topolody( void );

int  _far simple_topology( void );
void      set_default_topology_ntm( void );
int  _far get_block_topology_size( struct float_pointtype _far *max,
                                  struct float_pointtype _far *min );
int  _far get_max_gp_size( struct gp_polygon _far *gp_ptr, struct float_pointtype _far *max,
                                                         struct float_pointtype _far *min );
int  _far drow_all_block_topology( MODUL_GP _far *gp );
void _far Drtrn( float x1, float x2, float x3, float y1, float y2, float y3 );
void _far set_trn_vp( void );
void _far set_deflt_vp( void );

int  _far out_twp_file( void );
 #endif
 #endif

int  _far put_topology( void );
#endif
