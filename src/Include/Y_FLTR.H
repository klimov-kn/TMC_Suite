/*
 *  TAMIC_soft(R) 1998 vR1.01
 *
 *  Declarations and definitions for filter characteristics.
 *
 *
 *  FILE : \source\include\y_fltr.h
 *
*/


#ifndef _Y_FLTR_H
  #define _Y_FLTR_H

//#define __DEBUG
  
  typedef struct {
					_real   l;      // lenght of line      in m
					_real   y;      // admitance of line
					_real alf;      // zatuh               in dB/m
                 } FILT_LINE;
  
  typedef struct {
    	            _real        bF;      // begin freq     in Hz
    	            _real        eF;      // end   freq     in Hz
					int          nF;      // number of freq 
					int          nL;      // number of line
					FILT_LINE  *lpL;      // line parameters
                 } FILT_INP_DAT;

  typedef struct {
					_real       rF;       // freq  in Hz 
					_real       rL;       // zatuh in dB
					_real     rKsv;       // KSV 
					_real      rPh;       // phase S12  in gradus
                 } FILT_OUT_SF;

  typedef struct {
					int             nF;      // number of freq 
					FILT_OUT_SF  *lpsF;      // output data for 1 freq
                 } FILT_OUT_DAT;

  int _far _fortran  filt_c1( char  *lpszInpFile, char  *lpszOutFile  );
  int _far _fortran  filt_inp_dat( char  *lpszInpFile, FILT_INP_DAT *spInpData  );
  int _far _fortran  filt_calc( FILT_INP_DAT *spInpData, FILT_OUT_DAT *spOutData  );
  int _far _fortran  filt_calc_f1( _real rFreq, int nNumLin, FILT_LINE *lpsLinPar, FILT_OUT_SF *lpsOutFreq );
  int _far _fortran  filt_calc_ymatr_init( _real rFreq, int nNumLin, Y_MATR4 *lpsYmatr, FILT_LINE *lpsLinPar );
  int _far _fortran  filt_calc_ymatr_init1( _real rFreq, Y_MATR4 *lpsYmatr, FILT_LINE *lpsLinPar );
  int _far _fortran  filt_calc_ymatr_asem( Y_MATR4 *psYmatrAll, int nNumLin, Y_MATR4 *psYmatr);
  int _far _fortran  filt_calc_ymatr_outp( FILT_OUT_SF *lpsOutFreq, Y_MATR4 *sYmatrAll);
  int _far _fortran  filt_out_dat( char *lpszOutFile, FILT_OUT_DAT *sOutData);
      
#endif
