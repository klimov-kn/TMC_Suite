/*
**
**  defmis.       : 3 function for includer/definer
**
**  function      :
**                  def_macro - return pointer on string '#define' in buffer
**                  def_include - include file in buffer and delete comment '\\' in buffer
**                  def_sl - delete comment '\\' in buffer
**  HEDER FILE'S  :
**                  <stdio.h>
**                  <stdlib.h>
**                  <string.h>
**                  <bufm.h>
**
**  LIB           : NO
**
**  OVERLAY       : YES
**
*/


#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <typedef.h>
#include <tmc_lib.h>
#include <error1.h>
#include <listing.h>
#include <mainwndw.h>
#include <tpl_file.h>
#include <indan.h>

#include "prepr1.h"

static char  szDefine1[] = "#define ";
static char  szDefine2[] = "#define\t";
                   /*  return pointer on string '#define' in buffer*/
int  def_macro( BUFFER _far *buf )
 {
  int p, p1;

  p = buf->inp_p;

  while( (buf_cmpn2( buf, 8, szDefine1, szDefine2 ) != BUF_NULL )&&( buf->inp_p >= 0) )
   {
    buf->inp_p = buf->ilex[ buf->inp_p ];
    }

  p1 = buf->inp_p;
  buf->inp_p = p;
  return( p1 );
  }

static void  put_inc_line_number( Uint line_number );
                   /*  include file in buffer and delete comment '\\' in buffer*/
static char  szInclude1[] = "#include ";
static char  szInclude2[] = "#include\t";
static char  szMessage1[] = "*** Include file: \"%s\" ***";
static char  szMessage2[] = "  %3d  %s";
static char  szMessage3[] = "*** End Include file: \"%s\" ***";

int _far fortran def_include( BUFFER _far *buf )
 {
  FILE _far *in;
  char       FgetsBuff[BUFSIZ];

  char		*str;
  char		*str1;
  char		*inf;

  if( (str = new char[ BUF_STR_N_MAX ]) == NULL )
  {
	  return -1;
  };
  if( (str1 = new char[ BUF_STR_N_MAX ]) == NULL )
  {
	  delete []str;
	  return -1;
  };
  if( (inf = new char[ BUF_STR_N_MAX ]) == NULL )
  {
	  delete []str1;
	  delete []str;
	  return -1;
  };

//  char  *lpTmp;

  int        p, p1, n1, i;
  int        inc_line;

  p = buf->inp_p;

  buf->inp_p=0;
  def_sl( buf );                                  /*  delete  coment*/

  buf->inp_p = p;

  while(  buf->inp_p >= 0 )                       /*  find include file name*/
   {
    if(buf_cmpn2( buf, 9, szInclude1, szInclude2 ) == BUF_NULL )       /*  if find '#include'*/
     {
      p1 = buf->inp_p;
      if( buf_gets( str, BUF_STR_N_MAX, buf) != BUF_NULL )
	  {
		  delete []inf;
		  delete []str1;
		  delete []str;
		  return -1;
	  };
      n1 = strlen(str);
      buf->inp_p = p1;

      buf_dels( buf, n1);                        /*  delete string with '#include'*/

      str[n1-1] = '\0';

      inc_line = 0;                          /*  include file line counter*/
	  for( i = 9; (i < BUF_STR_N_MAX)&&( str[i] == ' '); i++ )
	  {
		  ;
	  };

      sprintf( inf, "%s\\%s", GetCurrentPath(), str + i);

      if( strlen( inf ) > FILENAME_MAX  )
        {
		  delete []inf;
		  delete []str1;
		  delete []str;
         put_error_messege( "FileName is long" );
         return -1;
        }

      if( (in = fopen( inf, "r")) == NULL)       /*  open including file*/
       {
		  delete []inf;
		  delete []str1;
		  delete []str;
        put_error_messege( "open file" );
		return(DEF_ERROR_INFILE);
	   }                                       /*  read including file in buffer*/

      setbuf( in, FgetsBuff );

      while( fgets( str1, BUF_STR_N_MAX, in) != NULL )
       {
		   DeleteComment(str1);
        /* comments skipping */
/*        if( (*str1 == '!')||(*str1 == '\n')||(*str1 == '\r') ) continue;
        for( lpTmp = str1; *lpTmp != 0; lpTmp++ ) if( *lpTmp != ' ') goto m_Cont;
        continue;

m_Cont: put_inc_line_number( ++inc_line );

		  // comment cutting 
        lpTmp = strrchr( str1, '!' );
        if( lpTmp != NULL )  *lpTmp = 0;
        else
          {
           lpTmp = strrchr( str1, '\n' );
           if( lpTmp != NULL )
            {
             *lpTmp = ' ';
             *(++lpTmp) = 0;
             }
          }


        if( (get_listing_echo_flag() == 1) &&
            (get_include_echo_flag() == 1)    ) l_printf( szMessage2, inc_line, str1);
*/

	if( buf->inp_p_nul < 0 )
	 {
          put_error_messege( "Buffer is small" );
          fclose( in );
		  delete []inf;
		  delete []str1;
		  delete []str;
          return(DEF_ERROR_MEMORY);
          }

        if( buf_puts3( str1, buf) != BUF_NULL )
         {
          fclose( in );
		  delete []inf;
		  delete []str1;
		  delete []str;
          return -1;
          }
       }

      fclose( in );

      buf->inp_p=0;
      def_sl( buf );                              /* delete  coment*/

      buf->inp_p = p1;
      }
    else
     {
      buf->inp_p = buf->ilex[ buf->inp_p ];
      }

    }

  delete []inf;
  delete []str1;
  delete []str;
  buf->inp_p = p;
  return( 0 );
  }


static char  szFmt[] = "Incl.line:%3d";

static void  put_inc_line_number( Uint line_number )
 {
  char buff[20];

  sprintf( buff, szFmt, line_number );
//  put_trace( buff );
  }


                   /*  delete comment '\\' in buffer*/
static char  mac[]  = "\\\\";
static char  str1[] = " \n";

int  def_sl( BUFFER _far *buf )
 {
  char *str;
  if( (str = new char[ BUF_STR_N_MAX ]) == NULL )
  {
	  return -1;
  };

  int  p, p1, n1;

  p = buf->inp_p;

  while(  buf->inp_p >= 0 )       /*  loop buffer*/
   {
    if(buf_cmpn( buf, 2, mac) == BUF_NULL )     /*  if find '\\'coment*/
     {
      p1 = buf->inp_p;
      if( buf_gets( str, BUF_STR_N_MAX, buf) != BUF_NULL )
	  {
		  delete []str;
		  return -1;
	  };
      buf->inp_p = p1;
      n1 = strlen(str);
      buf_dels( buf, n1+1);                    /*  delete coment*/
      if( buf_puts2(str1, buf) != BUF_NULL ) 
	  {
		  delete []str;
		  return -1;
	  };
      }
    else
     {
      buf->inp_p = buf->ilex[ buf->inp_p ];
      }

    }

  buf->inp_p = p;
  delete []str;
  return(0);
  }

char  *DefineIncludeName( char _far *str, char _far *str1 )
  {
/*   char _far *szFile, _far *ptr;
   int i;
   FILE _far *fl;

   for( i = 0, szFile = str; i < 8; i++, szFile++ ) *szFile = ' ';
   del_blanks( str );
   if( strcmp( str, VR_MAC_FILE_NAME ) == 0 ) return str;

   strcpy( str1, (char _far *)get_tpl_file_name() );
   ptr = strrchr( str1, '\\' );
   if( strchr( str, '\\' ) == NULL ) *(++ptr) = '\0';
   else                              *(ptr)   = '\0';

   strcat( str1, str );
   if( (fl = fopen( str1, "rb" )) != NULL )
    {
     fclose( fl );
     strcpy( str, str1 );
     return str;
    }

   str1 = make_full_file_name( str1, GetIncludePathName(), str );
   strcpy( str, str1 );
*/   return str;
  }


