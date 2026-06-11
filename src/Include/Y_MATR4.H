/*
 *  TAMIC_soft(R) 1998 vR1.01
 *
 *  Declarations and definitions for Y-matrix 4-pole.
 *
 *
 *  FILE : \source\include\y_matr4.h
 *
*/

#ifndef _Y_MATR4_H
  #define _Y_MATR4_H


  typedef struct {
    	            _ELEM_VAL y11;
	    			_ELEM_VAL y12;
	    			_ELEM_VAL y21;
	    			_ELEM_VAL y22;
                 } Y_MATR4;

  int   _far _fortran y_matr_addpar( Y_MATR4 _far *y1, Y_MATR4 _far *y2  );
  int   _far _fortran y_matr_addposl( Y_MATR4 _far *y1, Y_MATR4 _far *y2  );
  _real _far _fortran y_matr_L( Y_MATR4 _far *y );
  _real _far _fortran y_matr_ksv( Y_MATR4 _far *y );
  _real _far _fortran y_matr_s21_phase( Y_MATR4 _far *y );
  int   _far _fortran y_matr_line( Y_MATR4 _far *lpsYmatr, _real  rFreq, _real rLenght, _real rAlf, _real rY);
  int _far _fortran y_matr_add_stub_xx( Y_MATR4 _far *y1, Y_MATR4 _far *y2  );
  int _far _fortran y_matr_add_stub_kz( Y_MATR4 _far *y1, Y_MATR4 _far *y2  );
  
#endif
