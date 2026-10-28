
#define ABS    0
#define EXP    1
#define LOG    2
#define SIN    3
#define COS    4
#define TAN    5
#define SQRT   7
#define SINH   8
#define COSH   9
#define TANH  10
#define ASIN  11
#define ACOS  12
#define ATAN  13
#define LOG10 14

#define EXPR_RADIAN        1
#define EXPR_GRADUS        0

#define FLT_MAX_SIN 65536.


#define ERROR_BRACKET      -17
#define ERROR_FUNCTION     -18
#define ERROR_POW          -16
#define ERROR_DIV_0        -15
#define ERROR_EXPRESSION   -19
#define ERROR_OVERFLOW     -20
#define ERROR_RETURN       -21
#define ERROR_VAR          -22

void    expr_set_angle( int flag );
_real   expr_comp( int cod, char   *oper1, int n1, char  *oper2, int n2 );
void    expr_set_error_cod( int n );
int     expr_get_error_cod( void );
_real   expr_inter( char  *lex, int n );
int     expr_numbern( char *lex, int n);
_real   expr_get_per( char   *lpszName, int nLenghtName );
void    expr_set_per( char   *lpszName, int nLenghtName, _real rValue );
int     expr_is_per( char   *lpszName, int nLenghtName );
char   *expr_get_error( void );
int     i1nte_atof_1( char *string, double *r );
_real   inte_atof( char  *string );
_real   inte_inter( char  *lex, int n );
_real   inte_oper( char   *lex, int n );
int     inte_select_blanck( char   *lex, int n );
void    inte_set_inter_value( _real r );
_real   inte_get_inter_value( void );

// EPSFAST: per thread interpreter context. While a thread has a context bound,
// the interpreter keeps its variables, error code and return value there
// instead of in the common (static) ones, so several threads can evaluate
// expressions at once. begin copies the common variables into the context.
typedef struct {
	_real rPer[256];
	int   nErrorCod;
	_real rInterValue;
} EXPR_THREAD_CTX;
EXPR_THREAD_CTX *expr_get_thread_ctx( void );
void    expr_thread_ctx_begin( EXPR_THREAD_CTX *pCtx );
void    expr_thread_ctx_end( void );
