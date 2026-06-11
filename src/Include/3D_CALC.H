/*
 *  TAMIC_soft(R) 1998 vR1.01
 *
 *  Declarations and definitions for 3d-calculation.
 *
 *
 *  FILE : \source\include\3d_calc.h
 *
*/
#ifndef _TYPDEF_H
 #include <typedef.h>
#endif

#ifndef _3D_CALC_KERN
  #define _3D_CALC_KERN

  typedef struct {
				  _ELEM_VAL  sIx;          // значения X - тока в клемме
				  _ELEM_VAL  sIy;          // значения Y - тока в клемме
				  _ELEM_VAL  sIz;          // значения Z - тока в клемме
                 } CLEMM_I;                // значения тока в клемме

  typedef struct {
				  CLEMM_I  sIe;            // значения электрического тока в клемме
				  CLEMM_I  sIm;            // значения магнитного тока в клемме
                 } BLOCK_CLEMM1;           // данные одной клеммы

  typedef struct {
	              int            nNumClem; // число клемм
				  BLOCK_CLEMM1  *lpsCl1;   // данные одной клеммы
                 } BLOCK_CLEMM;            // пространство клемм

  typedef struct {
				  void        *lpsFunc;    // функция
				  void        *lpsData;    // данные
                 } BLOCK_GRAN_TYPE;        // тип границы

  typedef struct {
	              int           nNumCl;    // число клемм в границе
	              int           *lpnCl;    // ссылки на клеммы в пр-ве клемм
				  BLOCK_CLEMM *lpsBlCl;    // пространство клемм
                 } GRAN_CLEMM;             // список клемм границы

  typedef struct {
	              BLOCK_GRAN_TYPE *lpsGrTyp;// тип границы
				  GRAN_CLEMM      *lpsGrCl; // список клемм границы
                 } BLOCK_GRAN1;             // описание одной границы

  typedef struct {
	              int         nNumGran;    // число границ
				  BLOCK_GRAN1  *lpsGr1;    // описание одной границы
                 } BLOCK_GRAN;             // условия на границах

  typedef struct {
				  void         *lpsData;   // данные блока
				  void  *lpsFunInitBlCl;   // функция инициализации пространства клемм
				  void    *lpsFunInitGr;   // функция инициализации условий на границах
				  void    *lpsFunInitIn;   // функция инициализации условий на входах
				  void        *lpsFunIm;   // функция вычисления Im
				  void        *lpsFunIe;   // функция вычисления Ie
                 } BLOCK_TYPE_EM;          // тип блока

  typedef struct {
				  BLOCK_TYPE_EM *lpsBlTyp; // тип блока
	              BLOCK_CLEMM   *lpsBlCl;  // пространство клемм
				  BLOCK_GRAN  *lpsGr;      // условия на границах
				  BLOCK_GRAN  *lpsIn;      // условия на входах
                 } BLOCK_DATA_EM;          // данные э/м токов в одном блоке

  int init_3d_1b_kern_drv( void );
  int _far _fortran 3d_calc_drv1b( char  *lpszInpFile, char  *lpszOutFile  );
  int _far _fortran 3d_1b_calc( BLOCK_DATA_EM *lpsBlDatEm );
  int _far _fortran 3d_1b_init( BLOCK_DATA_EM *lpsBlDatEm );
  int _far _fortran 3d_1b_calc_proc( BLOCK_DATA_EM *lpsBlDatEm );

#endif
