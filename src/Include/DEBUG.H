/*
**  TAMIC_soft(R) 1990 v1.00     12/01/89 03:54pm
**
** Macros and decloration for Tamic_H programm.
**
** FILE: a:\include\debug.h
*/

struct debug_flags
	{
         int m;
         int a;
         int t;
         int i;
         };

#ifndef _INP_ADRESS
 #include <run.h>
#endif

#ifndef _Y_MATRIX_DEF
 #include <run.h>
#endif

int  _far _fortran GetDebugFlag( DWORD dwFlag );

void _far print_agnv_inp( float _far *a, float _far *b, float _far *m, float _far *e,
                         int nmax, int np1, int nmod1, int nmod, int key1, int key2,
                         double _far *szglm1, double _far *szglm2, double _far *xin, double _far *xin1,
                         float _far *pn, float _far *RKEY6, float _far *DBA, float _far *DDX,
                         int ifail );
void _far print_link_number( int num );
void _far print_matrix( int dim, float _far *matr );
void _far print_matrix_info( char _far *title, Y_MATRIX _far *matr );
void _far print_link_list( LINK_LIST _far *ll_ptr );
void _far print_replace_table_2( int _far *nzam1, int _far *nzam2, INPUT _far *inp_1, INPUT _far *inp_2 );

void _far fortran print_polygon( POLY_LINE_ATTR _far *pla, int block_num );

#ifndef _MODEL_W
 void _far print_wvgd_input_data( MODUL_WVGD _far *wvgd, int block_num );
 void _far print_opstb_input_data( MODUL_STB _far *opstb, int block_num );
 void _far print_trn_input_data( MODUL_TRN _far *trn, int block_num );
#endif
