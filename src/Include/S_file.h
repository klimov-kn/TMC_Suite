/*
**  TAMIC_soft(R) 1991,1992 v1.00
**
**  s_file.
**
**  FILE : ..\include\s_file.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#include <typedef.h>
#include <tmclimit.h>

#ifndef _S_FILE_DEF
 #define _S_FILE_DEF

#ifndef _INPUT_DATA_DEF
 #include <postpro.h>
#endif

#ifndef _OFFSET_DEF
 #define _OFFSET_DEF
typedef struct
         {
          _real frq;
//          int  lOffset;
          DWORD  lOffset;
          }_OFFSET;
#endif

typedef _OFFSET S_OFFSET;

#ifndef _S_HEADER_DEF
 #define _S_HEADER_DEF
typedef struct
         {
          WORD cPs;                 /* "pascal" string counter*/
          WORD cbSize;              /* offset table size*/
          WORD cnFrqNum;            /* frequency number*/
          }S_HEADER;
#endif

typedef struct
         {
          WORD           cbSize;              /* offset table size*/
          WORD           cnFrqNum;            /* frequency number*/
          S_OFFSET _far *lpOffset;           /* pointer to offset table*/
          }S_OFFSET_TABLE;

typedef struct
         {
          _complex _far *lpxC0;   /*pointer at function values*/
          _complex _far *lpxC1;   /*pointer at first  power spline coefficient*/
          _complex _far *lpxC2;   /*pointer at second power spline coefficient*/
          _complex _far *lpxC3;   /*pointer at third  power spline coefficient*/
         }INTERP_COEFF;

#define S_WR 0
#define S_RD 1
#define S_PS_NUMBER 7
#define OFF_TBL_MAIN 1
#define OFF_TBL_APPR 2
#define SMT_LABEL      "SMT"     /*scattering matrix file label*/
#define SMT_VERSION     102

#define S_UNKNOWN        -1
#define S_UNITARY         0
#define S_NOT_UNITARY     1
#define UNITARY_THRESH  (_real)1.0e-12

char _far * _fortran Get_S_file_name( void );
int  _far   _fortran Set_S_file_name( void );
void _far make_S_matrix_name( char _far *S_filename, S_INDEX _far *si );
int  _far save_S_matrix( _complex _far *sss, INPUT_DATA _far *id,
                          char _far *S_filename, S_INDEX _far *si, int extrapol_flag );
int  _far read_S_matrix( char _far *filename, S_INDEX _far **si, int _far *extrapol_flag,
                          _complex _far ***sss, FREQUENCY _far *frq_out, INPUT_DATA _far **id);
int _far _fortran cut_S_matrix( _complex _far ***sss, FREQUENCY _far *frq, INPUT_DATA _far **id );
void _far _fortran set_interp_warning( int nWIFlag );
int  _far _fortran get_interp_warning( void );
int  _far read_S1_matrix( char _far *filename,  _complex _far **sss, INPUT_DATA _far *id, int _far *lpnUnit  );
int  _far copy_S_matrix( void );
void _far _fortran  Free_GRID_Index( S_INDEX _far **si, int extrapol_flag );
void _far _fortran  Free_S_read( _complex _far ***sss, int nFrqNum );
int  _far _fortran  Interp_ODQCH_Cmplx( _real _far *lprX, _complex _far *lpxC0, _complex _far *lpxC1, _complex _far *lpxC2,
                                                          _complex _far *lpxC3, int nDim );
FILE _far * _fortran Get_S_FILE( void );
void   _far _fortran Set_PS_Number( int nNum );
int    _far _fortran Get_PS_Number( void );
int    _far _fortran Open_S_file( char _far *szFile, int nOpenMode );
void   _far _fortran Close_S_file( void );
int    _far _fortran update_offset_table( _real frq, int num, int nTBL_Flag );
int    _far _fortran skip_string( void );
int    _far _fortran write_S( void _far *ptr, size_t size );
int    _far _fortran read_S( void _far *string, size_t size );
void   _far _fortran s_proc( char _far *name );

int _far _fortran read_S_matrix_element_HFSS( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint);
int _far _fortran SMatrixElementHFSS_InpMod( int *in1, int *mod1, int *in2, int *mod2, char *filename );
int _far _fortran SMatrixElementHFSS_ModMax( char *filename, int *pnInp );
int _far _fortran SMatrixElementHFSS_InpMax( char *filename );
int _far _fortran ChooseString( char *pszBuf, char *pszIdentifInput, char *pszIdentifMode, int *pnInp, int *pnMod );
int _far _fortran SMatrixElementHFSS_FreqArray( int *in1, int *mod1, int *in2, int *mod2, char *filename, double **f1, _complex **sss, int *nPoint, int *nMatrixSize);
void _far _fortran SMatrixElementHFSS_FreqArrayCalculation( char *filename, double **f1, int *nPoint, int *nMatrixSize);
void _far _fortran SMatrixElementHFSS_FreqArrayAllocation( double **f1, _complex **sss, int *nPoint);
void _far _fortran SMatrixElementHFSS_FreqArrayRead( int *in1, int *mod1, int *in2, int *mod2, char *filename, double **f1, _complex **sss, int *nPoint, int *nMatrixSize);


#if defined _MODEL_H || defined _MODEL_E || defined _MODEL_W
int    _far _fortran put_S_matrix( float _far *sss, INPUT_DATA _far *id,
                                   char _far *S_filename, S_INDEX _far *si, int extrapol_flag );
#else
int    _far _fortran put_S_matrix( _real _far *sss, INPUT_DATA _far *id,
                                   char _far *S_filename, S_INDEX _far *si, int extrapol_flag );
#endif

#endif
