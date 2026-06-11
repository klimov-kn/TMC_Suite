/*
**  TAMIC_soft(R) 1993 vR3.10
**
**  Modes flag functions declaration and constants definitions
**
**  FILE : include\3D\modeflag.h
**
*/

#ifndef _MODEFLAG_H
 #define _MODEFLAG_H

#define  M3D_RESET           0
#define  M3D_WAVEG           1

void _far fortran set_3D_mode_flag( int nFlag );
int  _far fortran get_3D_mode_flag( void );

#endif
