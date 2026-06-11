/*
**  TAMIC_soft(R) 1992 vR1.02
**
**  Matrix functions declarations
**
**  FILE : mic\matrix.h
**
*/

#ifndef _MATRIX_H
 #define _MATRIX_H


int _far _fortran Invert_Sym_by_Gauss( _real _huge *hpA, _real _far *lpAux, int _far *lpnAux, int nDim);
int _far _fortran Invert_cSym_by_Gauss( _complex _huge *hpxA, _complex _far *lpxAux, int _far *lpnAux, int nDim);
int _far _fortran Invert_cdSym_by_Gauss( _real _huge *hprA_r, _real _huge *hprA_i, _complex _far *lpxAux, int _far *lpnAux, int nDim);
int _far _fortran Factor_Sym_by_Cholesky( _real _huge *hpA, int nDim, double _far *Det );
int _far _fortran Solve_sym_factored( _real _far *lpV, int nDim, _real _huge *hpLD );
void _far _fortran Change_Matrix_Sign( _real _huge *hpMatrix, int nDim, int nStorage_flag );
void _far _fortran Change_cMatrix_Sign( _complex _far *lpxMatrix, int nDim, int nStorage_flag );
void _far _fortran Cut_the_Bolls( _complex _far *lpxMatrix, int nDim, int nStorage_flag );
void _far _fortran Add_cScalar_to_diag( _complex _far *lpxMatrix, _complex  xScalar, int nDim, int nStorage_flag );
int _far _fortran Copy_cMatrix_to_cBlock( _complex _far *lpxA, _complex _far *lpxB, int nDimA, int nDimB );
int _far _fortran Sub_cMatrix_from_cBlock( _complex _far *lpxA, _complex _far *lpxB, int nDimA, int nDimB );
void _far _fortran Conv_cGen_to_cSym( _complex _far *lpxA, _complex _far *lpxB, int nDim );
void _far _fortran Conv_cSym_to_cGen( _complex _far *lpxA, _complex _far *lpxB, int nDim );

int _far _fortran EigenVV_Real( _real _far *lprA, _real _far *lprB, _real _far *lprX, _real _far *lprXL, int nDim );
int _far _fortran EigenVV_of_Real_Sym( _real _far *lprA, _real _far *lprR, int nDim, int nWhat );

int _far _fortran Mul_cSym_by_cSym_Gen(_complex _far *lpxA, _complex _far *lpxB, _complex _far *lpxC, int nDim);
int _far _fortran Multiply_General( _real _far *lprA, _real _far *lprB, _real _far *lprC, int nN, int nM, int nL );
int _far _fortran Multiply_cGeneral( _complex _far *lpxA, _complex _far *lpxB, _complex _far *lpxC, int nN, int nM, int nL );
void _far _fortran Transpose_N_by_N( _real _far *lprA, int nN );
void _far _fortran Transpose_N_by_N_c( _complex _far *lpxA, int nN );
void _far _fortran Transpose_M_by_L_int( int _far *lpnA, int _far *lpnB, int nM, int nL );
int _far _fortran Mul_2Sym_Err_Norm(_real _huge *hpA, _real _huge *hpB, int nDim, double _far *Norm);
double _far _fortran Vector_Eucl_Norm( double _far *lpVector, WORD wLength, WORD wInc );
void _far _fortran INVGDC( _complex _far *A, int N, _complex _far *D, int _far *L, int _far *M );
void  _far _fortran Hilbert_matrix( _real _far *lpH, int nDim );
int _far _fortran Ext_Precision_Add( double dNumber, double _far *lpAcc);
int _far _fortran Ext_Precision_Mul( double dNum1, double dNum2, double _far *lpAcc);
double _fortran Get_Acc_Value( double _far *lpAcc );

int  _far _fortran Write_matrix(char _far *szTitle, _real _huge *hpMatrix, int nDim, int nS_mode );
void _far _fortran Write_complex_matrix( char _far *szTitle, _complex _huge *hpxMatrix, int nDim, int nS_mode, int nPart );
void _far _fortran Write_vector(char _far *szTitle, _real _far *Vector, WORD wLength, int nNum_flag);

int _far _fortran From_ZY_to_S( _complex _far *lpxS, _complex _far *lpxImmit, int nDim, int nMat_flag);
int _far _fortran From_S_to_Y( _complex _far *lpxS, _complex _far *lpxY, int nDim);
int _far _fortran Check_S_Unitary( _complex _far *lpxS, int nDim, _real _far *rMax );

#define MATRIX_REAL_MIN (1e-1)

#endif

