/*
**  TAMIC_soft(R) 1991 v1.00
**
** Macros and decloration for 3D TAMIC graphics programm
**
** FILE: ..\include\tmc3dgr.h
**
*/

#ifndef _TMC_3DGRPH_H
  #define _TMC_3DGRPH_H

#ifndef __3D_POINT
  #define __3D_POINT

typedef struct { double x;
                 double y;
                 double z;
		 } _3D_VECTOR;

typedef _3D_VECTOR  _3D_POINT;

typedef struct
          {
           _3D_POINT   _3D_Max;
           _3D_POINT   _3D_Min;
           } _3D_FRAME;
#endif

void       _far  set_org_vector( _3D_VECTOR _far *vector );
_3D_VECTOR _far *get_org_vector( void );

void    _far set_x_angle( double x );
double  _far get_x_angle( void );

void    _far set_y_angle( double x );
double  _far get_y_angle( void );

void    _far set_z_angle( double x );
double  _far get_z_angle( void );

void _far build_3d_transformation( void );

double    _far x_3d_transformation( double x, double y, double z );
double    _far y_3d_transformation( double x, double y, double z );
double    _far z_3d_transformation( double x, double y, double z );
_3D_POINT _far *_3d_point_transformation( _3D_POINT _far *out, _3D_POINT _far *in );

void _far point_3d_w( _3D_POINT _far *x );
void _far line_3d_w( _3D_POINT _far *x1, _3D_POINT _far *x2 );
void _far outtext_3d_w( _3D_POINT _far *x, char _far *string );

#endif
