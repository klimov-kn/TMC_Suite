/*
 *  TAMIC_soft(R) 1991,1992 vR1.02
 *
 *  Definitions and declarations for kernel procedures of IC program.
 *
 *  FILE : \include\mic\kernel.h
 *
*/

#ifndef KERNEL_H
 #define KERNEL_H

typedef struct              /* structure for terminals lines parameters */
         {
          int  _far *lpnStart;  /* coordinates of terminals line beginning */
          int  _far *lpnEnd;    /* coordinates of terminals line end       */
          int  _far *lpnType;   /* type of terms. line (simple, inp., lum. or surf. imp.) */
          int  _far *lpnRStart; /* index of terms. line beginning in reduced GGF */
          int  _far *lpnCStart; /* index of term. lines beginning in composed GGF */
          int  nNumber;        /* number of terminals lines */
          int  nDimR;          /* dimension of reduced GGF block */
         } TERM_LINES;

typedef struct              /* structure for terms. lines along X and Y */
         {
          TERM_LINES Along_X;
          TERM_LINES Along_Y;
         } ALL_LINES;

typedef struct
         {
          double _far *cos_x;  /* pointer at cosine array along X axis */
          double _far *sin_x;  /* pointer at sine array along X axis */
          double _far *cos_y;  /* pointer at cosine array along Y axis */
          double _far *sin_y;  /* pointer at sine array along Y axis */
         } CS_ARR;

   /* functions for forming and using current lines arrays */
ALL_LINES _far * _fortran Get_ALL_LINES( void );
void  _far _fortran Free_term_lines( void );
int _far  _fortran Init_terminal_arrays( void );

int   _far _fortran Get_X_lines_num( int nLayer_num );
int   _far _fortran Get_Y_lines_num( int nLayer_num );

int   _far _fortran Get_X_block_dim( int nLayer_num );
int   _far _fortran Get_Y_block_dim( int nLayer_num );
void  _far _fortran Set_GGF_dim( int nDim );
int   _far  _fortran Get_GGF_dim( void );

TERM_LINES _far * _fortran Get_X_lines_arr( int nLayer_num );
TERM_LINES _far * _fortran Get_Y_lines_arr( int nLayer_num );

#define CS_GENERAL 0
#define CS_SHORT   1

   /* functions for working with auxiliary cosine and sine arrays */
int   _far    _fortran   Init_CS_arrays( void );
void  _far    _fortran   Set_CS_arr( CS_ARR _far *lpCS, int nCase );
CS_ARR _far * _fortran   Get_CS_arr( int nCase );
void  _far    _fortran   Free_CS_arrays( void );
void  _far    _fortran   Fill_CS_arrays( void );
double _far * _fortran Get_cos_X( void );
double _far * _fortran Get_cos_Y( void );
double _far * _fortran Get_sin_X( void );
double _far * _fortran Get_sin_Y( void );

#define GGF_IMAGINARY 0
#define GGF_REAL      1

#define BLOCK_XX 1
#define BLOCK_YY 2
#define BLOCK_XY 3

   /* special functions for building Grid Greens Function matrix */
int   _far _fortran Init_GGF_array( int nPart );
void  _far _fortran Free_GGF_array( void );

void  _far    _fortran Set_GGF( _real _huge *hpGGF_Tmp, int nPart );
_real _huge * _fortran Get_GGF( int nPart );
int   _far    _fortran Build_GGF( void );
int   _far    _fortran CalcGeneralSums( void );

int   _far _fortran GGF_Egv( _real _far *lpGGF_Egv, int nBlock_flag );

int _far _fortran GetGeneralSums( _real _far *lpGGF_Egv, _real _far *lpGen_Sums, int nBlock_flag  );
int   _far _fortran Gen_Sums_XX( _real _far *lpGGF_Egv, _real _far *lpGen_Sums );
int   _far _fortran Gen_Sums_YY( _real _far *lpGGF_Egv, _real _far *lpGen_Sums );
int   _far _fortran Gen_Sums_XY( _real _far *lpGGF_Egv, _real _far *lpGen_Sums );

int  _far _fortran GGF_block_XX( _real _far *lpGen_Sums, _real _huge *hpGGF,
            int _far *lpnStart, int _far *lpnEnd, int _far *lpnType, int _far *lpnRStart );
int  _far _fortran GGF_block_YY( _real _far *lpGen_Sums, _real _huge *hpGGF,
            int _far *lpnStart, int _far *lpnEnd, int _far *lpnType, int _far *lpnRStart );
int  _far _fortran GGF_block_XY( _real _far *lpGen_Sums, _real _huge *hpGGF,
            int _far *lpnStart_X, int _far *lpnEnd_X, int _far *lpnType_X, int _far *lpnRStart_X,
            int _far *lpnStart_Y, int _far *lpnEnd_Y, int _far *lpnType_Y, int _far *lpnRStart_Y );

#define GGF_BUFSIZ  512

int _far _fortran Add_react_Count_act( void );
int _far _fortran Condense_GGF( void );
int _far _fortran Elim_Surface_Imp( void );

int   _far _fortran Invert_GGF_Matrix( void );
int   _far _fortran Factor_GGF_Matrix( void );

   /* functions for building immitance matrix of unfolded multiport */
int      _far _fortran Build_Y_unf_mpt( void );
void     _far _fortran Set_Y_unf_dim( int nDim );
int      _far _fortran Get_Y_unf_dim( void );
int      _far _fortran Init_Y_unf_mpt( void );
_complex _huge * _fortran Get_Y_unf_mpt( void );
void     _far _fortran  Free_Y_unf_mpt( void );
int      _far _fortran Unf_mpt_XY( _complex _huge *hpcY_unf );
int      _far _fortran Unf_mpt_Y(  _complex _huge *hpcY_unf );


   /* regular line analysis functions and constants */
       /* number of mesh sizes for cutting and stretching line */
#define LINES_DIFFERENCE  2
#define LF_LONG_LINE   1
#define LF_SHORT_LINE  2
int   _far _fortran Init_lines_matrices( void );
void  _far _fortran Free_lines_matrices( void );
void  _far _fortran Set_line_type( int nLine_type );
int   _far _fortran Get_line_type( void );
void  _far _fortran Set_lines_length( void );
_real _far _fortran Get_line_length( int nLine_flag );
void  _far _fortran Set_line_descr( void );
_complex _far * _fortran Get_line_descr( int nLine_flag );
void  _far _fortran Change_line_length( int );
int   _far _fortran Analyze_line( void );
int _far _fortran Single_line(  double y11a, double y12a, double y11b, double y12b,
                               _complex _far *ro, _complex _far *bll, _complex _far *go, _complex _far *dfa );

int _far _fortran Write_Input_prms( void );
int _far _fortran Sort_Copy_IPRMS( void );
int _far _fortran Make_S_matrix( void );
int _far _fortran Write_LINE_GEOMETRY_PS( void );

#endif

