#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <typedef.h>
#include <expr.h>

void _far main( void )
 {
  _real rl;
  char _far *error;
  char ch[800];

  printf("\nTAMIC\tcalculator version 1.0 (25.10.91)\n");

  gets( ch );
  while( ch[0] != '0' )
   {
    rl= expr_atof(ch);
    error=expr_get_error();
    printf("\n%s=%g;\n%s\n", ch, rl, error );
    gets( ch );
    };
  }

