/*
**  TAMIC_soft(R) 1991,1992 vR1.02
**
**  Definitions and declarations for input data routines.
**
**  FILE : \include\mic\inpdata.h
**
*/

#ifndef _INPDATA_H
 #define _INPDATA_H

typedef struct
  {
   int nBack;      /* back wall type*/
   int nRight;     /* right wall type*/
   int nLeft;      /* left wall type*/
  } IWALLS;        /* input and sidewalls relation structure*/


typedef struct
  {
                 /* total number of current regions in input */
   int nWin_num;

                 /* length from input sidewall to opposite one in mesh sizes */
   int nLength_in_mesh;

                 /*numbers of terminals along each  window */
   int nAlong_term_num;

                 /* matrix of layer numbers for each current region */
   int _far *lpnFloor_of_window;

                 /*matrix of numbers of terminals across each region */
   int _far *lpnAcross_term_num;

              /*matrix of numbers of mesh sizes from nearest sidewall to edge */
   int _far *lpnNear_Shift;

                 /*matrix of numbers of mesh sizes from far sidewall to edge */
   int _far *lpnFar_Shift;

   IWALLS IWalls; /* input and sidewalls relation structure*/

   } IPOS; /*structure for definition of input positions at the grid*/


#ifndef _IPRMS_STRUCT
 #define _IPRMS_STRUCT

typedef struct
  {
                /* transformation matrix( from modes space to terminals one ) */
   _complex _far *lpxTransMatrix;

               /* matrix of characteristic impedances of modes */
   _complex _far *lpxCharImp;

              /* matrix of compensation admittances of modes */
   _complex _far *lpxCompAdmitt;

              /* matrix of compensation phases of modes */
   _complex _far *lpxCompPhase;

              /* matrix of propagation constants of modes */
   _complex _far *lpxPropConst;
  } IPRMS; /*structure for difinition of input matching parameters*/
#endif

typedef struct
 {
  _complex xImp_X;      /* impedance along X axis*/
  _complex xImp_Y;      /* impedance along Y axis*/
  } SURIMP;             /* impedances corresponding to surface impedances*/


int         _far _fortran Set_nSize_X( void );
int         _far _fortran Set_nSize_Y( void );
int         _far _fortran Get_nSize_X( void );
int         _far _fortran Get_nSize_Y( void );
void        _far _fortran Set_new_Size_X( int L1 );
void        _far _fortran Set_new_Size_Y( int M1 );

_real       _far _fortran Get_Mesh_Size_X( void );
_real       _far _fortran Get_Mesh_Size_Y( void );

void       _far _fortran Set_input_num( int nInp_num );
void       _far _fortran Set_X_input_num( int nInp_X_num );
void       _far _fortran Set_Y_input_num( int nInp_Y_num );
void       _far _fortran Set_total_reg_num( int nTot_reg_num );

int        _far _fortran Get_input_num( void );
int        _far _fortran Get_X_input_num( void );
int        _far _fortran Get_Y_input_num( void );
int        _far _fortran Get_total_reg_num( void );

IPRMS _far * _fortran Get_inp_values( int nInpNum );

int        _far * _fortran Get_inp_pos_ind( void );
IPOS       _far * _fortran Get_inp_pos( void );
IPRMS      _far * _fortran Get_inp_prms( int nfrq ); /*only for allocation*/

void       _far   _fortran Set_inp_pos_ind( int _far *lpnIPI );
void       _far   _fortran Set_inp_pos( IPOS _far *lpipsIP );
void       _far   _fortran Set_inp_prms( int nfrq, IPRMS _far *lpiprIPRM );

int  _far _fortran Init_all_input_arr( void );
int _far  _fortran Init_IPRMS_root( void );
void _far _fortran Free_all_input_arr( void );
void _far _fortran Free_preliminary_arr( void );
void _far _fortran Free_IPOS( void );
void _far _fortran Free_IPRMS( IPRMS _far *lpIPRMS );
void _far _fortran Free_IPRMS_root( void );


void       _far _fortran Set_lumped_num( int nLum_num );
void       _far _fortran Set_X_lumped_num( int nLum_num );
void       _far _fortran Set_Y_lumped_num( int nLum_num );
int        _far _fortran Get_lumped_num( void );
int        _far _fortran Get_X_lumped_num( void );
int        _far _fortran Get_Y_lumped_num( void );
int        _far _fortran Init_lumped_root( void );
void       _far _fortran Free_lumped_root( void );
_complex   _far _fortran Get_lumped_admittance( int nLumNum );
void       _far _fortran Free_lumped_arr( void );
void       _far _fortran Set_lumped_array( int nfrq, _complex _far *lpxPtr );
_complex   _far * _fortran Get_lumped_array( int nfrq );

void _far _fortran Set_SurImp_num( int nNum );
int  _far _fortran Get_SurImp_num( void );
void _far _fortran Set_TermImp_num( int nNum );
int  _far _fortran Get_TermImp_num( void );
_complex   _far   _fortran Get_sur_imp( int nObj_num, int nCurDir );
int        _far   _fortran Init_surface_root( void );
void       _far   _fortran Free_surface_root( void );
void       _far   _fortran Free_surface_arr( void );
void       _far   _fortran Set_objects_array( int _far *lpnPtr );
int        _far * _fortran Get_objects_array( void );
void       _far   _fortran Set_surface_array( int nfrq, SURIMP _far *lpsPtr );
SURIMP     _far * _fortran Get_surface_array( int nfrq );

int _far _fortran  make_name_with_path( char _far *szFile, char _far *szExt );

#endif
