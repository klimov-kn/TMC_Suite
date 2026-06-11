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


#define NX_SC             10
#define NY_SC             10

#define X_MAX_SCALE        5
#define Y_MAX_SCALE        5

#define MAX_GRAPH_NUM     10

#define LEFT_SIDE          0
#define RIGHT_SIDE         1

#define OLD                0
#define NEW                1
#define LAST               2
#define ALL_CURVE          3

#define _LINE              0

#define LINEAR_INTERPOL    0


#define ITEMS_NUM            9
#define ITEMS_MAX_LENGTH     8
#define ITEMS_SECOND_LENGTH 14

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
	    char _far *tpl_name;
	    char _far *S_name;
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
             struct graph_curve _far *next;
             struct graph_curve _far *prev;
             };
typedef struct graph_curve GRAPH_CURVE;
#endif

struct _real_point { _real x, y; };
typedef struct _real_point FLOAT_POINT;



struct curve_data
            {
             int num;                   /* number of point on curve*/
             FLOAT_POINT _far *p;
             };
typedef struct curve_data CURVE_DATA;


struct graph_wnd_config
            {
             int x_scale, y_scale;       /* F, Wl, for x, Kctu, L .. for y*/
             int x_style, y_style;       /* graph net style line or log*/
             int x_type, y_type;         /* graph net line type*/
             int x_width, y_width;       /* graph net width*/
             int x_color, y_color;       /* graph net color*/
             int x_sc_color, y_sc_color; /* graph scale color*/
             int border_color, border_style, border_width;  /* border color*/
             int general_back_color, graph_back_color;      /* graph feald color*/
             WINDOW w;                                      /* graph int window*/
             struct float_window fw;                        /* graph float window*/
             int curve_num;                   /* number of graph curve*/
             GRAPH_CURVE _far *gc;            /* pointer to graph curve list*/
             GRAPH_CURVE _far *zerro;         /* pointer to graph curve zerro list*/
             S_INDEX si[3];                   /* s-matrix index for extrapolation*/
             struct set_char_size _far *ss;   /* scale char size*/
             };
typedef struct graph_wnd_config GRAPH_WND_CONFIG;

void  _far draw_all_curves( GRAPH_WND_CONFIG _far *gwc );
void  _far set_default_graph_wn( _complex _far *sss, INPUT_DATA _far *id, FREQUENCY _far *frq, GRAPH_CURVE _far *gc, GRAPH_WND_CONFIG _far *gwc );
void  _far draw_graph_wn( GRAPH_WND_CONFIG _far *gwc );

void  _far add_curve_to_graph ( CURVE_DATA _far *c_p, GRAPH_WND_CONFIG _far *gwc );

GRAPH_CURVE _far  *add_curve_to_list( char _far *filename, GRAPH_WND_CONFIG _far *gwc,
                                            MARKER _far *mt, CURVE _far *ct,
                                            CURVE_INDEX _far *ci );
void  _far delete_curve( GRAPH_CURVE _far *gc, GRAPH_WND_CONFIG _far *gwc );

void  _far put_marker_to_graph( _complex _far *sss, INPUT_DATA _far *id, FREQUENCY _far *frq,
                               GRAPH_CURVE _far *gc, GRAPH_WND_CONFIG _far *gwc );
void  _far output_graphic( CURVE_DATA _far *c_ptr, GRAPH_WND_CONFIG _far *gwc );
_real _far get_point_x( int num, FREQUENCY _far *frq, int scale );
_real _far get_point_y( _complex _far *sss, INPUT_DATA _far *id, CURVE_INDEX _far *ci, int scale );


void  _far init_float_graph_wn( void );
int   _far corr_vert_region( _real y, struct float_window _far *fw, int scale);
int   _far corr_horiz_region( _real x, struct float_window _far *fw );
void  _far set_vert_region( _real bottom, _real tom, struct float_window _far *fw);
void  _far set_horiz_region( _real left, _real right, struct float_window _far *fw);
void  _far set_marker( _real x, _real y );
void  _far set_curve_type( CURVE _far *crv );

int   _far out_proc( void );
void  _far output_menu( GRAPH_WND_CONFIG _far *gwc );
void  _far put_out_menu_item( WINDOW _far *wn, char _far *item, struct alt_colors _far *fbc);
void  _far make_input_mode_cnfg( char _far *item, CURVE_INDEX _far *im );

int   _far change_y_scale( WINDOW _far *mw, GRAPH_WND_CONFIG _far *gwc );
int   _far file_out_menu( void );

void  _far legend_out( GRAPH_WND_CONFIG _far *gwc );
void  _far init_menu_window_0( WINDOW _far *mw_0 );

void  _far print_first_window( void );

void  _far output_menu_change( GRAPH_WND_CONFIG _far *gwc );
void  _far output_menu_add( GRAPH_WND_CONFIG _far *gwc );
void  _far output_menu_del( GRAPH_WND_CONFIG _far *gwc );
void  _far _draw_lgnd_curve( WINDOW _far *mw, GRAPH_CURVE _far *cv_ptr );

_real _far _fortran _Kctu( _complex _far *sss );
_real _far _fortran _Loss( _complex _far *sss );

#ifndef _MODEL_3D
void _far fortran MtNrm( _real _far *matr, int _far *i, _real _far *norm_array, int _far *j,  int _far *ndimt);
#else
void _far fortran MtNrm( _real _far *matr, _real _far *norm_array, int ndimt );
#endif

void _far _fortran put_S_table( _complex _far *lpxS, int nInp_num, int _far *modnb );

