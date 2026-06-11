/*
**  TAMIC_soft(R) 1991,1992 vR1.02
**
**  Limits for IC program.
**
**  FILE : \include\mic\limit_ic.h
**
*/

#ifndef _LIMIT_IC_H
 #define _LIMIT_IC_H

#define LM_TO_DD_MIN  20  /*min value of wavelength to discr. dist. ratio*/

#define DD_BOTTOM  0.0         /*min value of discretisation distances( m. )*/
#define DD_TOP     10.0f       /*max value of discretisation distances( m. )*/

#define HEIGHT_BOTTOM  0.0f       /*min value of media layer thickness( m. )*/
#define HEIGHT_TOP     1.0e3f     /*max value of media layer thickness( m. )*/

#define EPSILON_BOTTOM  1.0f    /*min value of relative dielectric constant*/
#define EPSILON_TOP     1.0e3f  /*max value of relative dielectric constant*/

#define MU_BOTTOM  1.0f    /*min value of relative permeability*/
#define MU_TOP     1.0f    /*max value of relative permeability*/

#define IMPED_BOTTOM 1.0e-10f /*min absolute value of lumped elem. impedance( Ohm )*/

#endif
