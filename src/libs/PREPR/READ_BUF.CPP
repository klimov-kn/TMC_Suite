/*
**
**  def_str.      : 12 function for includer/definer
**
**  function      :
**                  read_buf - gets string from buffer
**
**  HEDER FILE'S  :
**                  <stdio.h>
**                  <bufm.h>
**
**  LIB           : oindtX.lib
**
**  OVERLAY       : YES
**
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <tmclimit.h>
#include <typedef.h>
#include <tpl_file.h>
#include <mainwndw.h>
#include <error1.h>
#include <listing.h>
#include <descrptr.h>

#include "prepr1.h"





int _far fortran read_buf( char _far *str, int n, BUFFER _far *buf)
 {
  int  i, j, j1;

  j1 = buf->inp_p;
  if( buf->inp_p >= 0) buf->inp_p = buf->ilex[buf->inp_p];  /*  skeep input pointer*/

  for( i = 0; ( i < n )&&( buf->inp_p >= 0 )&&( BUF_LEX != '\0' )&&( BUF_LEX != '\n' ); i++)
   {
    str[i] = BUF_LEX;
    j = buf->inp_p;
    buf->inp_p = buf->ilex[buf->inp_p];         /*  skeep input pointer*/
    };

  if( buf->lsplex[buf->ilex[buf->inp_p]] == '\0')  buf->inp_p = buf->ilex[buf->inp_p];
  j = buf->inp_p;
  str[i] = '\0';

  if( (BUF_LEX == '\0') || ( BUF_LEX == '\n') )
   {
    if( buf->inp_p < 0 ) buf->inp_p = j1;

    return( BUF_NULL );
    }

  if( buf->inp_p < 0 )
   {
    buf->inp_p = j1;
    return( BUF_EOF );
    };

  put_error_messege( "very long string" );
  return( BUF_ABN );
  }


