#ifndef _INTER_CALCUL
 #define _INTER_CALCUL

 #include <expr.h>

  typedef struct {
    	            char lpszName[32];
	    			int  nLenght;
                 } INTE_OPER;

int expr_del_Blanks1( char *ch );
_real inte_atof( char *string );
_real inte_inter( char  *lex, int n );
void  inte_set_inter_value( _real r );
_real inte_get_inter_value( void );
_real inte_oper( char  *lex, int n );
int inte_select_blanck( char  *lex, int n );
int i1nte_atof_1( char  *string, _real *r );

#endif