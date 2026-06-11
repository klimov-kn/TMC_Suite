
#ifndef TMCINDANPREPR_H__
#define TMCINDANPREPR_H__

//#include <windows.h>
//#include <TmcLibError.h>


#define NMAX             30000000
//#define NMAX             8000000

#define BUF_STR_I        str[i]
#define BUF_LEX          buf->lsplex[ buf->inp_p ]
#define BUF_LEX_NUL      buf->lsplex[ buf->inp_p_nul]

#define BUF_STR_INLINE_MAX 500000
#define BUF_STR_N_MAX    500000
#define BUF_MASKA_MAX    800000
#define BUF_PAR_MAX      60

#define VR_MAC_FILE_NAME "$$vr$$s.prc"
#define VR_MAC_INCLUDE   "#include $$vr$$s.prc"

typedef struct
 {
  char       *lsplex;                   /*  buffer*/
  int        *ilex;                     /*  elements pointer*/
  int         inp_p;                    /*  pointer on begin*/
  int         inp_p_nul;                /*  pointer on begin null*/
  }BUFFER;

typedef struct
 {
  char        mac[ BUF_STR_N_MAX ];       /*   buffer's string*/
  char       *m[ BUF_PAR_MAX ];           /*   *m[0]-macros name*/
  char       *m1[ BUF_MASKA_MAX ];        /*   *m1[i]-for maska*/
  int         im1[ BUF_MASKA_MAX ];       /*   im1   -for maska*/
  int         j1;                         /*   number formal parameters in name*/
  int         i2;                         /*   number formal parameters in maska*/
  } MACROS;


#define BUF_NULL                    0
#define BUF_EOF                    -1
#define BUF_ABN                    -2
#define DEF_ER_BRACKET             -3
#define BUF_ERROR_MACR             -4
#define BUF_ERROR_FACT_PARAM       -5
#define BUF_ERROR_NO_SEP           -6
#define DEF_ERROR_INFILE           -7
#define DEF_ERROR_MEMORY           -8
#define BUF_ERROR_NO_FORMAL_PARAM  -9
#define BUF_ERROR_REDEC           -10
#define ERROR_INIT                -11

int  buf_gets( char  *str, int n, BUFFER  *buf);
int  read_buf( char  *str, int n, BUFFER  *buf);
int  buf_puts( char  *str,  BUFFER *buf);
int  buf_puts1( char  *str, BUFFER *buf );
int  buf_puts2( char  *str, BUFFER *buf);
int  buf_puts3( char  *str, BUFFER *buf);
int  buf_dels( BUFFER  *buf, int n);
int  buf_init( BUFFER  *buf, int n);

void def_gets( char *mac1, char  *ch);
int  def_buf_gets_mac( BUFFER  *buf, char  *mac1);
int  def_buf_gets_param( BUFFER *buf, char *mac1);
int  buf_cmpn( BUFFER  *buf, int n, char  *mac);
int  buf_cmpn2( BUFFER *buf, int n, char *mac1, char  *mac2 );
int  def_main_macro( BUFFER  *buf );
int  def_macro( BUFFER  *buf );
int  def_include( BUFFER  *buf );
int  def_sl( BUFFER  *buf );

char * def_err_get( int i_err );
int    unload_prep_buf( void );
int    load_preproc_buff( void );
char * main_preproc( char *szTargetFileName, char *szSourceFileName );
int   load_vr_file( BUFFER  *buf );
int   dest_prep( void );

int  def_subst_mac( BUFFER  *buf, MACROS  *mac);
int  def_set_formal_param( BUFFER  *buf, MACROS  *mac, int i);
int  def_separ_mac_name( BUFFER  *buf, MACROS  *mac);
int  def_err_dec( BUFFER  *buf );

char  *DefineIncludeName( char  *str, char  *str1 );
char *GetCurrentPath( void );
void DeleteComment( char *pszStr );
int InitCharArray( char **ch, int n );
int FreeCharArray( char **ch );
void MainPrepr1( char *csTplFileNamePrepr,	char *csTplFileName, char *csCurrentPath, char *csError );

#endif