/*
 *  TAMIC_soft(R) 1991,1992 vR1.02
 *
 *  Definitions and declarations for post processing procedures of IC program.
 *
 *  FILE : ..\tamic\post_pro\ic\post_pro.h
 *
*/

#define NORM_COEFF   ((_real)1.0e-9)

int  _far _fortran save_S( _complex _far *smatr,   int nInp_num, int _far *modnb );
int _far _fortran Interpolate_S_matrix( char _far *szFile );
void _far _fortran put_S_table( _complex _far *lpxS, int nInp_num, int _far *modnb );


