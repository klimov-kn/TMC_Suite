#ifndef _OPTIMIZ_CALCUL
 #define _OPTIMIZ_CALCUL


  typedef struct {
	               int    n;		// array size
    	           char  *buf;		// string data
                 } OPT_INP_BUF;

  typedef struct {
	               int n;						// array size 
	               OPT_INP_BUF *lpsName;		// array name 
	               OPT_INP_BUF *lpsRFor;		// array read format
	               OPT_INP_BUF *lpsWFor;		// array write format
                 } OPT_INP_FORMAT;

  typedef struct {
    	            char              lpszDataFileName[ FILENAME_MAX ];	// file name 
	    			OPT_INP_FORMAT    lpsInpFormat;						// array of input data
					OPT_INP_BUF       lpsMainFun;						// main function string
                 } OPT_INP_DAT1;

  typedef struct {
	               OPT_INP_BUF lpsName;		// name for var
	               OPT_INP_BUF lpsRFor;		// read format for var
	               _real       lprStep;		// step for var
	               _real       lprMinV;		// min value for var
	               _real       lprMaxV;		// max value for var
                 } OPT_BAT_VAR;

  typedef struct {
	               int n;					// array size 
	               OPT_BAT_VAR *lpsVar;		// array of var
    	           char   lpszDataFileName[ FILENAME_MAX ];	// file name 
                 } OPT_BAT_DAT;

  typedef struct {
    	            int              n;			 // number in array lpsInpDat
	    			OPT_INP_DAT1    *lpsInpDat;	 // array of data read for main_fun
					_real            rMfVal;	 // value of main function after calculation
    	            char             lpszBatFileName[ FILENAME_MAX ]; // file name for bat file
	    			OPT_BAT_DAT      lpsBatDat;	 // data for edit data for bat file
    	            int              nIter;		 // max number of iterration for stop calculation
    	            _real            rEps;		 // min value of main_fun for stop calculation
                 } OPT_INP_DAT;

int _far _fortran  optimiz_TargetFunction( char *lpszFileName, _real *rpSignfic);
void _far _fortran get_arg_env( int argc, char _far *argv[], char _far *envp[] );
int _far _fortran processor( void );
void _far title_ys( void );
int _far _fortran title_ys_n( void );
int _far _fortran set_title_ys( char *ch );
int _far _fortran init_input_data_optimiz( void );
void _far _fortran set_input_data_optimiz( OPT_INP_DAT _far *lpsInpDatOpt1 );
OPT_INP_DAT _far * _fortran get_input_data_optimiz( void );
int _far _fortran init_input_data_optimiz_numb( int _far *n );
int _far _fortran init_input_data_optimiz_mf( int n );
int _far _fortran init_input_data_optimiz_fin( void );
int _far _fortran init_input_data_optimiz_fin1( OPT_INP_DAT1     *lpsInpDat );
int _far _fortran init_input_data_optimiz_fin2( OPT_BAT_DAT     *lpsInpDat );
int _far _fortran get_tpl_optim_format( char *ch, OPT_INP_BUF *lpsName, OPT_INP_BUF *lpsRFor, OPT_INP_BUF *lpsWFor );
int _far _fortran get_tpl_optim_format1( char *ch, OPT_INP_BUF *lpsName, int *ii );
int _far _fortran get_tpl_optim_mf_telo( OPT_INP_BUF   *lpsMainFun );
int _far _fortran get_tpl_optim_format_i( OPT_INP_BUF *lpsName, OPT_INP_BUF *lpsRFor, OPT_INP_BUF *lpsWFor );
int _far _fortran calc_main_fun_optimiz( void );
int _far _fortran get_optimiz_numb( void );
OPT_INP_DAT1 _far * _fortran get_input_data1_optimiz( int i );
int _far _fortran calc_optimiz_mf( int i );
int _far _fortran init_input_data_optimiz_mf1( OPT_INP_DAT1 *id1 );
int _far _fortran calc_optimiz_format( OPT_INP_FORMAT *lpsInpFormat );
void _far _fortran set_real_val_opt( _real r );
_real _far _fortran get_real_val_opt( void );
void _far _fortran set_bat_file_name( char *ch );
char _far _fortran *get_bat_file_name( void );
int _far _fortran run_optimiz( void );
int _far _fortran get_tpl_optim_format_ib( OPT_BAT_VAR *lpsVar );
int _far _fortran init_input_data_optimiz_bf( OPT_BAT_DAT *id1 );
int _far _fortran get_tpl_optim_format_b( char *ch, OPT_BAT_VAR *lpsVar );
int _far _fortran get_optimiz_max_N_iter( void );
_real _far _fortran get_optimiz_rEps( void );
int _far _fortran proc_optimiz_1_iter( void );
int _far _fortran get_optimiz_N_var( void );
char _far _fortran *get_datrun_file_name( void );
void _far _fortran  proc_optimiz_save_file( void );
void _far _fortran  proc_optimiz_save_file1( void );
void _far _fortran  proc_optimiz_iter_init( void );
int _far _fortran  proc_optimiz_iter_prizn( void );
int _far _fortran proc_optimiz_1_iter_var( int nVarI, int nStepI );
int _far _fortran proc_optimiz_1_iter_var1( void );
int _far _fortran proc_optimiz_file_zam( char *lpsName, char *lpsRFor, _real rStep, _real rMinV, _real rMaxV);
int _far _fortran proc_optimiz_file_zam1( char *lpsName, char *lpsRFor, FILE *fInp, FILE *fOut, _real *r);
int _far _fortran proc_optimiz_file_zam2( FILE *fInp, FILE *fOut);
int _far _fortran optimiz_kb_break( void );
int _far _fortran optimiz_init( void );
int _far _fortran optimiz_close( void );
int _far _fortran optimiz_proc( void );
int _far _fortran stop_optimiz( void );
int _far _fortran start_optimiz_proc( char *lpszComLine );

int  _far _fortran set_lst_file_name( char _far *name );
int  _far _fortran close_lst_file( void );
char _far * _fortran get_lst_file_name( void );
_real  _far _fortran get_lst_real( char *name, int nFormat, char *format  );
char _far *szGetOutTrace( void );

#define TMCOPT_EVENT      "TmcOptEvent"
#define TMCOPT_EVENTTRACE "TmcOptEventTrace"

#endif
