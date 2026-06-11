/*
**
**  def_str.      : 12 function for includer/definer
**
**  function      :
**                  buf_gets - gets string buffer buf -> char _far *str
**                  buf_puts - puts string char _far *str -> buffer buf include NULL('\0') and buf->inp_p is const
**                  buf_puts1 - puts string char _far *str -> buffer buf not include NULL('\0') and buf->inp_p is const
**                  buf_puts2 - puts string char _far *str -> buffer buf not include NULL('\0') and buf->inp_p is var
**                  buf_puts3 - puts string char _far *str -> buffer buf include NULL('\0') and buf->inp_p is var
**                  buf_dels - delete n characters in buffer buf
**                  def_gets - gets formal parameter( A-Z,a-z,1-0,_ ) string ch -> string mac1
**                  def_buf_gets_mac - gets fact macros name in buffer buf
**                  def_buf_gets_param - gets fact parameters in buffer buf
**                  buf_cmpn - compare string str lenght n with string in buffer
**
**  HEDER FILE'S  :
**                  <stdio.h>
**                  <bufm.h>
**
**  LIB           : oindan.lib
**
**  OVERLAY       : YES
**
*/

#include <stdio.h>

#include <typedef.h>
#include <error1.h>

#include "prepr1.h"


int  buf_gets( char _far *str, int n, BUFFER _far *buf)
 {
  int  i, j, j1;

  j1 = buf->inp_p;

  if( buf->inp_p >= 0) buf->inp_p = buf->ilex[buf->inp_p];

  for( i = 0; ( i < n )&&( buf->inp_p >= 0 )&&( BUF_LEX != '\0' ); i++)
   {
    str[i] = BUF_LEX;
    j=buf->inp_p;
    buf->inp_p = buf->ilex[buf->inp_p];
    };
  str[i] = '\0';


  if( buf->inp_p < 0 )
   {
    buf->inp_p = j;
  //  put_error_messege( "Buffer is small" );
    return( BUF_EOF );
    };

  if( BUF_LEX == '\0') return( BUF_NULL );

//  put_error_messege( "Long string" );
  return( BUF_ABN );
  }

int  buf_puts( char _far *str, BUFFER _far *buf)
 {
  int i, j, j1;

  j = buf->ilex[buf->inp_p];
  buf->ilex[buf->inp_p] = buf->inp_p_nul;

  for( i = 0; ( BUF_STR_I != '\0' )&&( buf->inp_p_nul >= 0 ); i++)
   {
    BUF_LEX_NUL = BUF_STR_I;
    buf->inp_p_nul = buf->ilex[buf->inp_p_nul];
    };

  if( buf->inp_p_nul < 0 )
   {
    put_error_messege( "Small buffer" );
    return( BUF_ABN );
    }

  BUF_LEX_NUL = BUF_STR_I;
  j1 = buf->inp_p_nul;
  buf->inp_p_nul = buf->ilex[buf->inp_p_nul];
  buf->ilex[j1] = j;

  return( BUF_NULL );
  }

int  buf_puts1( char _far *str, BUFFER _far *buf )
 {
  int i, j, j1;

  j = buf->ilex[buf->inp_p];
  buf->ilex[buf->inp_p] = buf->inp_p_nul;

  for( i = 0; ( str[i+1] != '\0' )&&( buf->inp_p_nul >= 0 ); i++)
   {
    BUF_LEX_NUL = BUF_STR_I;
    buf->inp_p_nul = buf->ilex[buf->inp_p_nul];
    };
  if( buf->inp_p_nul < 0 )
   {
    put_error_messege( "Small buffer" );
    return( BUF_ABN );
    }

  BUF_LEX_NUL = BUF_STR_I;
  j1 = buf->inp_p_nul;
  buf->inp_p_nul = buf->ilex[buf->inp_p_nul];
  buf->ilex[j1] = j;

  return( BUF_NULL );
  }

int  buf_puts2( char _far *str, BUFFER _far *buf)
 {
  int i, j, j1;

 if( str[0] == '\0' )  return BUF_NULL;

  j=buf->ilex[buf->inp_p];
  buf->ilex[buf->inp_p] = buf->inp_p_nul;

    for( i = 0; ( str[i+1] != '\0' )&&( buf->inp_p_nul >= 0 ); i++)
     {
      BUF_LEX_NUL = BUF_STR_I;
      buf->inp_p_nul = buf->ilex[buf->inp_p_nul];
      };

    if( buf->inp_p_nul < 0 )
     {
      put_error_messege( "Small buffer" );
      return( BUF_ABN );
      }

    BUF_LEX_NUL = BUF_STR_I;
    buf->inp_p=buf->inp_p_nul;

  j1 = buf->inp_p_nul;
  buf->inp_p_nul = buf->ilex[buf->inp_p_nul];
  buf->ilex[j1] = j;

  return( BUF_NULL );
  }

int  buf_puts3( char _far *str, BUFFER _far *buf)
 {
  int i, j, j1;

  j=buf->ilex[buf->inp_p];
  buf->ilex[buf->inp_p] = buf->inp_p_nul;

  for( i = 0; ( str[i] != '\0' )&&( buf->inp_p_nul >= 0 ); i++)
   {
    BUF_LEX_NUL = BUF_STR_I;
    buf->inp_p_nul = buf->ilex[buf->inp_p_nul];
    };

  if( buf->inp_p_nul < 0 )
   {
    put_error_messege( "Small buffer" );
    return( BUF_ABN );
    }

  BUF_LEX_NUL = BUF_STR_I;
  buf->inp_p=buf->inp_p_nul;
  j1 = buf->inp_p_nul;
  buf->inp_p_nul = buf->ilex[buf->inp_p_nul];
  buf->ilex[j1] = j;

  return( BUF_NULL );
  }

int  buf_dels( BUFFER _far *buf, int n)
 {
  int i, j, j1;

  j = buf->inp_p;
  j1 = buf->inp_p_nul;
  buf->inp_p_nul = buf->ilex[j];

  for( i = 0; ( i < n )&&( buf->inp_p >= 0); i++) buf->inp_p = buf->ilex[buf->inp_p];

  buf->ilex[j] = buf->ilex[buf->inp_p];
  buf->ilex[buf->inp_p] = j1;
  buf->inp_p = j;

  return( BUF_NULL );
  }




void  def_gets( char _far *mac1, char _far *ch)
 {
  register int i = 0;

  while( ( (ch[i] <= 'z')&&(ch[i] >= 'a') )||
         ( (ch[i] <= 'Z')&&(ch[i] >= 'A') )||
         ( (ch[i] <= '9')&&(ch[i] >= '0') )||
           (ch[i] == '_') )
   {
    mac1[i] = ch[i];
    i++;
    };

  if( i == 0)
   {
    mac1[i] = ch[i];
    i++;
    };

  mac1[i] = '\0';
  }

int   def_buf_gets_mac( BUFFER _far *buf, char _far *mac1)
 {
  int i;
  char ch;

  for( i = 0; i >= 0; i++)
   {
    if( buf->inp_p < 0)
     {
      mac1[i] = '\0';
      return( BUF_EOF );
      };

    buf->inp_p = buf->ilex[buf->inp_p];
    ch = buf->lsplex[buf->inp_p];
    if( ( (ch <= 'z')&&(ch >= 'a') )||
        ( (ch <= 'Z')&&(ch >= 'A') )||
        ( (ch <= '9')&&(ch >= '0') )||
          (ch == '_') )
     {
      mac1[i] = ch;
      }
    else
     {
      if( i == 0 )
       {
        mac1[i] = ch;
	i++;
	};
      mac1[i] = '\0';
      return( 0 );
      };
    };
  return( 0 );
  }

int   def_buf_gets_param( BUFFER _far *buf, char _far *mac1)
 {
  int  jb = 0, i;
  char ch;

  for( i = 0; i >= 0; i++)
   {
    if( buf->inp_p < 0 )
     {
      put_error_messege( "Small buffer" );
      return( BUF_EOF );
      }

    buf->inp_p = buf->ilex[buf->inp_p];
    ch = buf->lsplex[buf->inp_p];
    switch( ch )
     {
      case '(' : jb++;
                 break;

      case ')' : jb--;
                 break;

      case ';' : if( jb == 0 )
		  {
                   if( (mac1[0] == '(')&&(mac1[i-1] == ')') )
                         {
                          mac1[i-1] = ' ';
                          mac1[0] = ' ';
                         }
                   mac1[i] = '\0';

		   return( BUF_NULL );
                   }
		  break;
      case '\0' : put_error_messege( "Fact parameter's" );
                  return( BUF_ERROR_FACT_PARAM );
      }
    mac1[i] = ch;
    }
  return 0;
  }

int   buf_cmpn( BUFFER _far *buf, int n, char _far *mac)
 {
  int p, i;

  p = buf->inp_p;

  for( i = 0; i < n; i++)
   {
    if( p < 0 ) return( BUF_EOF );
    p = buf->ilex[p];

    if( buf->lsplex[p] != mac[i])  return( BUF_EOF );
    }

  return( BUF_NULL );
  }

int   buf_cmpn2( BUFFER _far *buf, int n, char _far *mac1, char _far *mac2 )
 {
  int p, i;

  p = buf->inp_p;

  for( i = 0; i < n; i++)
   {
    if( p < 0 ) return( BUF_EOF );
    p = buf->ilex[p];
    if( ((buf->lsplex[p] != mac1[i])&&(buf->lsplex[p] != mac2[i])) )  return( BUF_EOF );
    }

  return( BUF_NULL );
  }
