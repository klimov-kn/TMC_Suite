#ifndef _BASE_MIC_H_DEF
#define _BASE_MIC_H_DEF
                       //                               w
typedef struct         // parameters of line         ÃÄÄÄÄÄÄ´
        {              //
         _real e_r;    // diel. pronits              ÉËËËËËË»
         _real h;      // height          Â  ÚÄÄÄÄÄÄÄÐÊÊÊÊÊÊÐÄÄÄÄÄÄÄÄ¿
         _real w;      // shirina        h³  ³ e_r                   ³
	 _real f;      // frequency       ³  ³                       ³
         _real l;      // length          Á  ÔÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¾
         } DATA_EHWF;  //                 Â  ÚÄÄÄÄÄÄÄÒÒÒÒÒÒÒÒÄÄÄÄÄÄÄÄ¿
                       //                 ³  ³       ºººººººº        ³
                       //                 ³  ³       ºººººººº        ³
                       //                l³  ³ e_r   ºººººººº        ³
                       //                 ³  ³       ºººººººº        ³
                       //                 Á  ÀÄÄÄÄÄÄÄÐÐÐÐÐÐÐÐÄÄÄÄÄÄÄÄÙ
typedef struct      // Y- matrix
	{
	 _real y11; // element of Y-matrix
	 _real y12; // ...................
	 _real y21; // ...................
	 _real y22; // ...................
	 } Y_PAR_LIN;

typedef struct         // Y- matrix with loss
	{
	 _complex y11; // element of Y-matrix
	 _complex y12; // ...................
	 _complex y21; // ...................
	 _complex y22; // ...................
	 } Y_PAR_LIN_LOSS;

typedef struct      // Y- matrix
	{
	 _real y11; // element of Y-matrix
	 _real y12; // ...................
	 _real y13; // ...................
	 _real y21; // ...................
	 _real y22; // ...................
	 _real y23; // ...................
	 _real y31; // ...................
	 _real y32; // ...................
	 _real y33; // ...................
	 } Y_PAR_3;

typedef struct               // parameters for paz
        {                    //                   ÃÄÄÄ´
                             //                     a
          DATA_EHWF *line;   //       Â  ÍÍÍÍÍÍÍÍÍ»   ÉÍÍÍÍÍÍÍÍÍÍ   Â
          _real a;           //       ³           º   º            b³
          _real b;           //      w³           ÈÍÍÍ¼             Á
	} DATA_PAZ1;         //       ³             |
                             //       Á  ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
                             //                     |
                             //referens 1 input - > |<-referens 2 input
                             //                     |
                             //                     |
                             //
                             //
                             //
                             //

typedef struct               // parameters for rupt (slot)
        {                    //                     ÃÄÄÄ´
                             //                       s
          DATA_EHWF *line;   //         Â  ÍÍÍÍÍÍÍÍÍ»   ÉÍÍÍÍÍÍÍÍÍÍ
          _real s;           //       w ³           º   º
        } DATA_RUPT;         //         ³           º   º
                             //         Á  ÍÍÍÍÍÍÍÍÍ¼   ÈÍÍÍÍÍÍÍÍÍÍ
                             //                     |   |
                             //referens 1 input - > |   | <- referens 2 input
                             //                     |   |
                             //

int    _far y_line(  DATA_EHWF _far *fpData , Y_PAR_LIN _far *y );        //Y-matrix for microstrip line Model Gupta, Fusko
int    _far y_line_tmc_a(  DATA_EHWF _far *fpData , Y_PAR_LIN _far *y );  //Y-matrix for microstrip line Model Gupta, Fusko
int    _far y_line_g(  DATA_EHWF _far *fpData , Y_PAR_LIN _far *y);       //Y-matrix for microstrip line Model Gardiol
int    _far y_line_g_tmc_a(  DATA_EHWF _far *fpData , Y_PAR_LIN _far *y); //Y-matrix for microstrip line Model Gardiol
int    _far y_user_element_massiv( _real _far *data, _real _far *matrix, int nd4);
int    _far paz_mic(  DATA_PAZ1 _far *fpData , Y_PAR_LIN _far *y);
int    _far paz_mic_tmc_a(  DATA_PAZ1 _far *fpData , Y_PAR_LIN _far *y);
int    _far end_mic(  DATA_EHWF _far *fpData , Y_PAR_LIN _far *y);
int    _far end_mic_tmc_a(  DATA_EHWF _far *fpData , Y_PAR_LIN _far *y);
int    _far t_junction_mic_tmc_a(  DATA_EHWF _far *fpData1 , Y_PAR_3 _far *y);
int    _far t_junction_mic(  DATA_EHWF _far *fpData1 ,DATA_EHWF _far *fpData2 , Y_PAR_3 _far *y);
int    _far coner_mic_tmc_a(  DATA_EHWF _far *fpData , Y_PAR_LIN _far *y);
int    _far coner_mic(  DATA_EHWF _far *fpData, Y_PAR_LIN _far *y);
int    _far step_mic_tmc_a(  DATA_EHWF _far *fpData1 ,  Y_PAR_LIN _far *y);
int    _far step_mic(  DATA_EHWF _far *fpData1 ,DATA_EHWF _far *fpData2 , Y_PAR_LIN _far *y);
int    _far rupt_mic(  DATA_RUPT _far *fpData , Y_PAR_LIN _far *y);
int    _far rupt_mic_tmc_a(  DATA_RUPT _far *fpData ,  Y_PAR_LIN _far *y);
int    _far fortran put_error_messege_lib_el( _real frq );
int    _far fortran set_error_lib_el_flag( void );

int  _far zv_v_tr( _real y1, _real y2, _real y3,
		    _real *y12, _real *y23, _real *y13) ;
int _far  tr_v_zv( _real y12, _real y23, _real y13,
		      _real *y1, _real *y2,  _real *y3);
 _real _far l_db_4( Y_PAR_LIN _far *y);

int _far  y_obed_posl( Y_PAR_LIN _far *y1, Y_PAR_LIN _far *y2);
int _far  y_obed_parr( Y_PAR_LIN _far *y1, Y_PAR_LIN _far *y2);

int _far  y_obed_posl_loss( Y_PAR_LIN_LOSS _far *y1, Y_PAR_LIN_LOSS _far *y2);
int _far  y_obed_parr_loss( Y_PAR_LIN_LOSS _far *y1, Y_PAR_LIN_LOSS _far *y2);
int  _far zv_v_tr_loss( _complex y1, _complex y2, _complex y3,
		    _complex *y12, _complex *y23, _complex *y13);
int _far  tr_v_zv_loss( _complex y12, _complex y23, _complex y13,
		     _complex *y1, _complex *y2, _complex *y3 );
 _real _far  l_db_4_loss( Y_PAR_LIN_LOSS _far *y);
int _far y_line_loss(  _real y0, _complex bl, Y_PAR_LIN_LOSS _far *y);


 _real _far beta(  DATA_EHWF _far *fpData );     //effective permittivity for microstrip line Model Gupta, Fusko
 _real _far beta_dis(  DATA_EHWF _far *fpData ); //effective permittivity for microstrip line Model Gupta, Fusko
 _real _far beta_g(  DATA_EHWF _far *fpData );   //effective permittivity for microstrip line Model Gardiol
 _real _far z0(  DATA_EHWF _far *fpData );
 _real _far z0_dis(  DATA_EHWF _far *fpData );
 _real _far e_r_ef_dis(  DATA_EHWF _far *fpData );

int _far _fortran get_pakeg( _real _far *er, _real _far *h );
_real _far _fortran get_er( char _far *str );
_real _far _fortran get_mu( char _far *str );


#define  C0  2.9979e+8



#ifndef _ELBASE_M_H
 #include <m\elbase_m.h>
#endif


int _far fortran element_assembler11( ELEMENT_BASE _far *eb,
				    _ELEM_VAL _far *sa, int _far *nsbspr, int _far *nsbspl,
				    int _far *indxsr, int _far *indxsl, int _far *nd1,
				    _ELEM_VAL _far *sad, int _far *nadrsr, int _far *nadrsl,
				    int _far *minst, int _far *nd2,
				    int _far *inzsp,
				    _ELEM_VAL _far *norm_y0, int _far *nd3,
				    _ELEM_VAL _far *matrix, int _far *nd4 );

void _far fortran RdcMt11( _ELEM_VAL _far *sa, _ELEM_VAL _far *sad, int _far *nd1, int _far *nd2,
			 int _far *nsbspr, int _far *nsbspl, int _far *nadrsr, int _far *nadrsl,
			 int _far *indxsr, int _far *indxsl, int _far *nuladr,
			 int _far *minst, int _far *ndim, int _far *ndimt,
			 _ELEM_VAL _far *avektr, int _far *nvektr, int _far *nd3,
			 int _far *ifail);

int _far element_matrix11( ELEMENT _far *elem, _real _far *matrix, int nd4,
		    _real _far *norm_y0, int nd3);
int _far element_matrix_c( ELEMENT _far *elem, _complex _far *matrix, int nd4,
		    _complex _far *norm_y0, int nd3);

_real _far mod( _complex s);
_real _far faz( _complex s);
int _far _fortran From_ZY_to_S( _complex _far *lpS, _complex _far *lpImmit, int nDim, int nMat_flag);


#define  ERROR_E_R        -1
#define  ERROR_H          -2
#define  ERROR_W          -3
#define  ERROR_F          -4
#define  ERROR_PAZ_A      -5
#define  ERROR_PAZ_B      -6
#define  ERROR_W2_W1_STEP -7
#define  ERROR_W_K_H      -8
#define  ERROR_S_K_W      -9

#define  WARRING_W         1
#define  WARRING_E_R       2
#define  WARRING_F         3
#define  WARRING_PAZ_A     4
#define  WARRING_PAZ_B     5
#define  WARRING_E_R_STEP  6
#define  WARRING_WKH       7

#endif


