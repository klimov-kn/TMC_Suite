/*
**  TAMIC_soft(R) 1992 vR1.02
**
**  Modes flag functions declaration and constants definitions
**
**  FILE : include\mic\modeflag.h
**
*/

#ifndef _MODEFLAG_H
 #define _MODEFLAG_H

#define  ICM_RESET           0
#define  ICM_SUMS            1
#define  ICM_UNFOLDED        2
#define  ICM_LINE            4
#define  ICM_DISCONTINUITY   8
#define  ICM_SUMS_READ      16
#define  ICM_UNFOLDED_READ  32
#define  ICM_SECOND_LINE    64
#define  ICM_THERD_LINE    128

void _far fortran set_IC_mode_flag( int nFlag );
int  _far fortran get_IC_mode_flag( void );

#endif
