/*
**  TAMIC_soft(R) 1991,1992 vR1.02
**
** Macros and declarations for debugging IC program.
**
** FILE: \include\mic\debug_ic.h
*/

#ifndef _DEB_FLAG_H
 #define _DEB_FLAG_H

struct DebugFlags
	{
   int  PrCurLine;  /*   print current lines tables,*/
   int  PrEigGGF;   /*   print eigenvalues of grid Green's function,*/
   int  PrGenSums;  /*   print general sums of GGF eigenvalues,*/
   int  PrRedInv;   /*   print reduced GGF and inverted matrices,*/
   int  CheckInv;   /*   check reduced GGF matrix inversion,*/
   int  PrUnfMul;   /*   print unfolded multiport immitance matrix,*/
  };

/* Declarations of functions for debugging flags */

void _far Set_PrCurLineFlag( int flag );
int  _far Get_PrCurLineFlag( void );

void _far Set_PrEigGGFFlag( int flag );
int  _far Get_PrEigGGFFlag( void );

void _far Set_PrGenSumsFlag( int flag );
int  _far Get_PrGenSumsFlag( void );

void _far Set_PrRedInvFlag( int flag );
int  _far Get_PrRedInvFlag( void );

void _far Set_CheckInvFlag( int flag );
int  _far Get_CheckInvFlag( void );

void _far Set_PrUnfMulFlag( int flag );
int  _far Get_PrUnfMulFlag( void );


#endif
