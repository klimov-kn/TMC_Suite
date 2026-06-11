/*
**  TAMIC_soft(R) 1990,1991 v1.00
**
**  data_const_list.h
**
**  FILE : a:\include\datacnst.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _DATACNST_H
 #define _DATACNST_H


struct data_const_list
         {
          int num_step;
          _real delta;
          _real eps, mu;
          _real height;
          _real acuracy;
          };

typedef struct data_const_list DATA_CONST_LIST;

void  _far  set_data_const_delta( _real delta );
_real _far  get_data_const_delta( _real _far *delta );
_real _far  get_data_const_delta_nrm( _real _far *delta );

_real _far fortran GetGlobalEpsilon( void );
_real _far fortran GetGlobalMu( void );

void  _far  set_data_const_height_width( _real height );
_real _far  get_data_const_height_width( _real _far *height );
_real _far  get_data_const_height_width_nrm( _real _far *height );

void  _far  set_data_const_acuracy( _real acuracy );
_real _far  get_data_const_acuracy( _real _far *acuracy );

void _far fortran get_dx( _real _far *dx );
void _far fortran set_dx( _real _far *dx );

void _far fortran set_dz( _real _far *dz );
void _far fortran get_dz( _real _far *dz );

#endif
