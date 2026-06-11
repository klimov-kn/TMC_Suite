/*
**  TAMIC_soft(R) 1990,1991 v1.20
**
** Macros and decloration for run  programms.
**
** FILE: a:\include\run.h
*/

#ifndef _RUN_H
 #define _RUN_H

 #ifndef _ELEM_VAL
  #ifdef _ELEM_COMPLEX
   #define _ELEM_VAL _complex
   #pragma message("_ELEM_VAL define as _complex . . .")
  #else
   #define _ELEM_VAL _real
   #pragma message("_ELEM_VAL define as real . . .")
  #endif
 #endif

#ifndef _INP_ADRESS
 #define _INP_ADRESS

 typedef struct
         {
          short block;
          short input;
          } INP_ADRESS;

#endif

#ifndef _Y_MATRIX_DEF
 #define _Y_MATRIX_DEF

typedef struct
        {
         short        num;                                       /* number of input*/
         short        norm_ptr[MAX_INPUT_NUM];                   /* number of mode in input*/
         _real      norm_array[MAX_INPUT_NUM][MAX_INPUT_NUM];  /* norm. coeff.*/
         short        flag[MAX_INPUT_NUM];                       /* +1 - no norm., -1 yes norm.*/
         INP_ADRESS ia[MAX_INPUT_NUM];                         /* input adress*/
         } INPUT;

typedef struct
         {
          _ELEM_VAL _far *matr;               /* pointer to Y-matrix buffer*/
//          float       _far *matr;               /* pointer to Y-matrix buffer*/
          short             norm_flag;          /* norm_flag: 0 - norm. not available*/
                                              /*            1 - available but not norm*/
                                              /*            2 - norm*/
          INPUT           inp;                /* input data*/
          } Y_MATRIX;
#endif
#include <linklist.h>
struct point_trn_adress
         {
          float _far *skx,   _far *sky;
          int   _far *ktpgr, _far *nomuz, maxnuz;
          int   koltr,       _far *nadr1, _far *nadr2, _far *nadr3;
          float _far *epstr, _far *rmutr;
          };

typedef struct
          { _real omga;       /* 2.0*PI*F(i)*/
            _real pi;         /* PI = 3.14....*/
            _real um0;        /* mu0 constant*/
            _real eps0;       /* eps0 constant*/
           }CONST_BLOCK;

#ifndef _MODUL_GP
 #define _MODUL_GP

struct modul_GP
	{
         int                   code;        /* entity's code*/
         int                   num;         /* block number*/
         void            _far *gp;          /* pointer to this entity*/
         struct modul_GP _far *next;        /* pointer to next entity*/
	 };
typedef struct modul_GP MODUL_GP;
#endif

int  _far _fortran run_step( void );
int  _far curnel( Y_MATRIX _far *matr );
int  _far step_out( Y_MATRIX _far *matr );

#ifndef _MODEL_W
int  _far modul( int modul_num, Y_MATRIX _far *matr, MODUL_GP _far * ptr );
int  _far modul_inp( MODUL_GP _far *gp_ptr, float _far *matr, INPUT _far *inp );
int  _far modul_wvgd( MODUL_GP _far *gp_ptr, Y_MATRIX _far *matr );
int  _far modul_opstb( MODUL_GP _far *gp_ptr, float _far *matr, INPUT _far *inp );
int  _far modul_file( MODUL_GP _far *gp_ptr, Y_MATRIX _far *matr );
int  _far trn_modul_fe( struct modul_GP _far *gp_ptr );
int  _far fill_data_trn_fe( struct modul_GP _far *gp,
                           float _far *skx, float _far *sky, float _far *epsgr, float _far *rmugr,
                           int _far *nsbsp, int _far *ktpgr, int _far *inzsp,
                           int _far *ngp, int _far *msinsp, float _far *epsrg, float _far *rmurg, int ngp_max,
                           int _far *kolin, int _far *numod, int in_max);
void _far get_input_length( int ngp, float _far *skx, float _far *sky, int _far *nsbsp,
                           int _far *ktpgr, int _far *msinsp, float _far *rinlong);
void _far get_input_type( int ngp, int _far *nsbsp, int _far *ktpgr, int _far *msinsp,
                         int _far *inp_type);
int  _far init_list( int _far *nsbsp, int list_size);
int  _far ansmbl_modul_fe( float _far *matr, INPUT _far *inp );
void _far fortran Trangl( int _far *ngp, float _far*skx, float _far*sky, int _far*nsbsp,
                         int _far*ktpgr, int _far*nomuz, float _far*epsgr, float _far*rmugr, int _far*nd1,
                         int _far*inzsp, int _far*msinsp, float _far*epsrg, float _far*rmurg, int _far*nd2,
                         float _far*delta, int _far*nstep,
                         int _far*koltr, int _far*nadr1, int _far*nadr2, int _far*nadr3,
                         float _far*epstr, float _far*rmutr, int _far *nd3,
                         int _far *koluzl, int _far *maxnuz, int _far *ifail);
void _far set_trn_data_file_name( int block_num );
int  _far write_trn( float _far *skx, float _far *sky, int _far *nomuz, int _far *ktpgr, int maxnuz,
                    int _far *nadr1, int _far *nadr2, int _far *nadr3, float _far *epstr, float _far *rmutr, int koltr,
                    int _far *numod, int kolin, float _far *rinlng, int _far*inp_type );
int  _far read_trn( struct point_trn_adress _far *pta, int _far *kolin, int _far *numod,
                   int _far *inp_type, float _far *rinlng);
void _far fortran GnAnsm( float _far *skx, float _far *sky, int _far *nomuz, int _far *ktpgr, int _far *nd1,
                         int _far *na1, int _far *na2, int _far *na3, float _far *epstr, float _far *rmutr, int _far *nd2, int _far *koltr,
                         float _far *sa, int _far *nsbspr, int _far *nsbspl, int _far *indxsr, int _far *indxsl, int _far *nd4,
                         float _far *sad, int _far *nadrsr, int _far *nadrsl, int _far *minst, int _far *nd5, int _far *ndim,
                         int _far *inzsp, int _far *ret_code);
void _far fortran FEMat( float _far *sa, float _far *sad, int _far *nd4, int _far *nd5,
                        int _far *nsbspr, int _far *nsbspl, int _far *nadrsr, int _far *nadrsl,
                        int _far *indxsr, int _far *indxsl, int _far *inzsp,
                        int _far *minst, int _far *ndim,
                        float _far *avektr, int _far *nvektr, int _far *nd6,
                        int _far *kuzbx, int _far *numod, int _far *inp_type, float _far *rinlng, int _far *nd3,
                        float _far *szg, int _far *nd7, int _far *nd9, INPUT _far *inp,
                        float _far *delta, float _far *height, int _far *ret_code);
#endif

int  _far fortran neladr( int _far *out_dim, int _far *i, int _far *j );

#ifndef _MODEL_W


/*             SUBROUTINE TRNSK(AC, B, X1, X2, NL, NMDL, NMAX, NMOD, NMOD1,
                                UM, E, SZG, ND2, NSBX, NUZL, PN, ND4,
                                SZGLM1, SZGLM2, xin, XIN1, nd6, nx1, nx2, nx12, keyl,
                                scob, nd7, KEY1, KEY2, KEY3, KEY5, KEY6, ifail) */
void _far fortran TRNSK( float _far *AC, float _far *B, float _far *X1, float _far *X2,
                        int _far *NL, int _far *NMDL, int _far *NMAX, int _far *NMOD, int _far *NMOD1,
                        float _far *UM, float _far *E, float _far *SZG, int _far *ND2,
                        int _far *NSBX, int _far *NUZL, float _far *nrmrk, int _far *ND4,
                        double _far *SZGLM1, double _far *SZGLM2, double _far *xin, double _far *XIN1, int _far *nd6,
                        int _far *nx1, int _far *nx2, int _far *nx12, int _far *keyl,
                        float _far *scob, int _far *nd7,
                        int _far *KEY1, int _far *KEY2, int _far *KEY3, int _far *KEY5, int _far *KEY6, int _far *ifail);
/*          SUBROUTINE MODNK(AC, B, X1, X2, X3, X4, ND1, NL, NPR, NMDL, NMDP, NMAX,
                             UM, E, SZG, ND2, NSBX, NUZL, ND3, szglm1, szglm2, scob1, nd4,
                             scob2, nd5, nx1, nx2, nx3, nx4, nx12, nx34, keyl, keyp,
                             KEY1, key2, KEY3, KEY5, KEY6, ifail) */
void _far fortran MODNK(float _far *AC, float _far *B, float _far *X1, float _far *X2, float _far *X3, float _far *X4,
                       int _far *ND1, int _far *NL, int _far *NPR, int _far *NMDL, int _far *NMDP, int _far *NMAX,
                       float _far *UM, float _far *E, float _far *SZG, int _far *ND2,
                       int _far *NSBX, int _far *NUZL, int _far *ND3,
                       double _far *szglm1, double _far *szglm2, float _far *scob1, int _far *nd4,
                       float _far *scob2, int _far *nd5,
                       int _far *nx1, int _far *nx2, int _far *nx3, int _far *nx4, int _far *nx12, int _far *nx34,
                       int _far *keyl, int _far *keyp, int _far *KEY1, int _far *key2, int _far *KEY3, int _far *KEY5,
                       int _far *KEY6, int _far *ifail);
void _far fortran STRGG1( float _far *matr1, int _far *dim1, int _far *in2,
                         float _far *matr2, int _far *dim2, int _far *_in2,
                         int _far *dim3, int _far *_in3, int _far *_out3,
                         int _far *KEY1, float _far *fbuff1, float _far *fbuff2, int _far *nd1, int _far *errcode);
void _far fortran DEARNG( float _far *matr1, float _far *matr2, int _far *_in3, int _far *_out3, int _far *flag);

int  _far reduction_1( Y_MATRIX _far *matr, LINK_LIST _far *ll_ptr );
int  _far reduction_2( Y_MATRIX _far *matr_1, Y_MATRIX _far *matr_2, LINK_LIST _far **ll_ptr );
#endif

int  _far save_Y_matrix( Y_MATRIX _far *ym );
int  _far read_Y_matrix( Y_MATRIX _far *ym );
int  _far replace_up( float _far *matr, int n1, int n2, int n3 );
int  _far replace_matrix_up( Y_MATRIX _far *matr, int _far *nzam, INPUT _far *inp_buff, int inp_num );
int  _far replace_doun( float _far *matr, int n1, int n2, int n3 );
int  _far replace_matrix_doun( Y_MATRIX _far *matr, int _far *nzam, INPUT _far *inp_buff, int inp_num );

int  _far oder_input( Y_MATRIX _far *matr );


void _far set_filter_flag( int flag );
int  _far get_filter_flag( void );

#endif
