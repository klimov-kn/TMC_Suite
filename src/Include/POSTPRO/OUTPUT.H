/*
**  TAMIC_soft(R) 1989.1991 v1.00
**
**  output.
**
**  FILE : ...\output.h
**
**  LIB  :
**
**  OVERLAY:
**
*/


int  _far get_marker_type( int num );

void _far init_marker_type( void );
void _far init_marker_type_add( void );

GRAPH_CURVE _far  *finde_marker_in_list( int num, GRAPH_WND_CONFIG _far *gwc );

void _far drow_curves_add( void );
void _far drow_curves_chng( void );
void _far drow_curves_del( void );

int _far corr_vert_region( _real y, FLOAT_WINDOW _far *fw, int scale );
int far corr_horiz_region( _real x, FLOAT_WINDOW _far *fw );
void _far set_vert_region( _real bottom, _real top, FLOAT_WINDOW _far *fw );
