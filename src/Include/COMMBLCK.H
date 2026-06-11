/*
**  TAMIC_soft(R) 1991 v1.20
**
** Macros and decloration for fortran compatability.
**
** FILE: a:\include\commblk.h
*/

#ifndef _COMMBLCK_H
 #define _COMMBLCK_H


typedef struct
          { _real omga;       // 2.0*PI*F(i)
            _real pi;         // PI = 3.14....
            _real um0;        // mu0 constant
            _real eps0;       // eps0 constant
	   }_CONST_BLOCK;

typedef struct
          { _real dx;         // dx - step
            _real dy;         // dy - step
            _real dz;         // dz - step
           }DXYZ_BLOCK;

#endif
