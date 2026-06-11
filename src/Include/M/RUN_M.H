/*
**  TAMIC_soft(R) 1992 v1.00
**
**  header file for run step M program.
**
**  FILE : ..\tamic\run_m.h
**
**  OVERLAY: NO
**
*/
#define _RUN_OLD
 #ifndef _RUN_M_H
 #define _RUN_M_H

 #ifndef _ELEM_VAL
  #ifdef _ELEM_COMPLEX
   #define _ELEM_VAL _complex
   #pragma message("_ELEM_VAL define as _complex . . .")
  #endif
  #ifdef _ELEM_REAL
   #define _ELEM_VAL _real
   #pragma message("_ELEM_VAL define as real . . .")
  #endif
 #endif

  #ifndef _RUN_OLD
  struct tag_ELEM_VALM {
			_ELEM_VAL                  sa;
			WORD                       nstr;
			WORD                       nclmn;
			struct tag_ELEM_VALM _far *lpNext_str;
			struct tag_ELEM_VALM _far *lpNext_clmn;
			} ;
   typedef struct tag_ELEM_VALM _ELEM_VALM;

   typedef struct {
		   int               ndim;
		   _ELEM_VAL  _far  *sad;
		   _ELEM_VALM _far **nstr;
		   _ELEM_VALM _far **nclmn;
		   int        _far  *minst;
		   } ELEMENT_ASM;

  #else
    typedef struct {
		    ELEMENT_BASE _far *eb;
		    int                nd1;
		    int          _far *nsbspr;
		    int          _far *nsbspl;
		    int          _far *indxsr;
		    int          _far *indxsl;
		    _ELEM_VAL    _far *sa;
		    int                nd2;
		    int          _far *nadrsr;
		    int          _far *nadrsl;
		    int          _far *minst;
		    _ELEM_VAL    _far *sad;
		    int                inzsp;
		    int                nd3;
		    _ELEM_VAL    _far *norm_y0;
		    int                nd4;
		    _ELEM_VAL    _far *matrix;
		    int                ndim;
		    int                ifail;
		    int                ndimt;
		    int          _far *nvektr;
		    _ELEM_VAL    _far *avektr;
		    } ELEMENT_ASM;
  #endif

#ifndef _INP_ADRESS
 #include <run.h>
#endif

#ifndef _Y_MATRIX_DEF
 #include <run.h>
#endif


  #ifndef _RUN_OLD
   int _far _fortran run_step( void );
   int _far _fortran kernal( Y_MATRIX _far *lpY);

   int _far _fortran element_assembler( ELEMENT_ASM _far *full_matrix );
   int _far          _fortran full_matrix_init( ELEMENT_ASM _far *full_matrix, ELEMENT_BASE _far *eb);
   int _far          _fortran full_matrix_set_element( ELEMENT_ASM _far *full_matrix, ELEMENT _far *el );
   int _far          _fortran full_matrix_set_inp( ELEMENT_ASM _far *full_matrix, _ELEM_VAL sa, int nclmn, int nstr, int code );
   _ELEM_VALM _far  *_fortran full_matrix_find( ELEMENT_ASM _far *full_matrix, int nclmn, int nstr);
   _ELEM_VAL _far   *_fortran element_matrix( ELEMENT _far *elem );

   int _far          _fortran full_matrix_print( ELEMENT_ASM _far *full_matrix);
   _ELEM_VAL _far    _fortran full_matrix_get_el( int nclmn, int nstr, ELEMENT_ASM _far *full_matrix);
   void _far          fortran _full_matrix_out( _ELEM_VAL _far *m, int ndim);

   _ELEM_VAL  _far  _fortran el_m_0( void );
   _ELEM_VAL  _far  _fortran el_m_add( _ELEM_VAL a1, _ELEM_VAL a2 );
  #else
   int _far _fortran run_step( void );
   int _far _fortran kernal1( Y_MATRIX _far *lpY);

   int       _far _fortran element_assembler1( ELEMENT_ASM _far *full_matrix );
   int       _far _fortran rdcmt1( ELEMENT_ASM _far *full_matrix );
   int       _far _fortran  build_y_matrix_input( Y_MATRIX _far *lpY );
   int       _far _fortran  print_y_matrix_input( Y_MATRIX _far *lpY );
   int       _far  step_out( Y_MATRIX _far *Y_matr );
   int       _far _fortran  free_full_matrix1( ELEMENT_ASM _far *full_matrix );
   int       _far _fortran  init1_full_matrix1( ELEMENT_ASM _far *full_matrix );
   int       _far _fortran  build_cond_y_matrix1( ELEMENT_ASM _far *full_matrix, Y_MATRIX _far *lpY);
   _ELEM_VAL _far get_y( int i, int j, Y_MATRIX _far *lpY , int out_dim);
   int       _far set_y( _ELEM_VAL y_ij, int i, int j, Y_MATRIX _far *lpY , int out_dim);
   int       _far y_matrix_prn( Y_MATRIX _far *lpY  );
   int       _far s_matrix_prn( Y_MATRIX _far *lpY );
  #endif


#endif


