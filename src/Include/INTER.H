#ifndef _INTER_CALCUL
 #define _INTER_CALCUL

 #include <expr.h>

  typedef struct {
    	            char lpszName[32];
	    			int  nLenght;
                 } INTE_OPER;

int expr_del_Blanks1( char *ch );
_real _far fortran inte_atof( char _far *string );
_real _far fortran inte_inter( char  _far *lex, int n );
void  _far fortran inte_set_inter_value( _real r );
_real _far fortran inte_get_inter_value( void );
_real _far fortran inte_oper( char  _far *lex, int n );
int _far fortran inte_select_blanck( char  _far *lex, int n );
int _far fortran i1nte_atof_1( char  *string, _real *r );

#endif