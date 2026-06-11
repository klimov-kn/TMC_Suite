_real _far fortran inte_fun( char _far *function, int leng_fun);
_real _far fortran inte_oper( char _far *function, int leng_oper);
void _far fortran inte_set_end( int end_code);
void _far fortran inte_set_err( int error_cod );
int _far fortran inte_get_err( void );
void _far fortran inte_del_blank( char _far *function, int *leng_fun);
_real _far fortran inte_f( char _far *function, int leng_fun);
int _far fortran inte_vid_oper( char _far *oper, int leng);
int _far fortran inte_logic_expr( char _far *oper, int leng_oper, _real _far *buf);
int _far fortran inte_vid_oper_for( char _far *oper, int leng);

#define RAZD    ';'
#define END_CODE 1

#define IF       0
#define BREAK    1
#define RETURN   2
#define WHILE    3
#define FOR      4

#define NOT_SEPARATOR -1
#define ABN_OPERATOR  -2
#define ABN_IF        -3
#define ABN_WHILE     -4
#define ABN_FOR       -5

//#define DEBUG 0
