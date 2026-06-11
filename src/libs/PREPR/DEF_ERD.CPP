/*
**
**  def_erd.      : 3 function for includer/definer
**
**  function      :
**                  def_get_mac_name - get macros name in buffer
**                  def_err_dec - tested buffer to redefine macros name
**                  load_vr_file - load macros in time file
**  HEDER FILE'S  :
**                  <stdio.h>
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

#include <typedef.h>
#include <error1.h>

#include "prepr1.h"


static int _near fortran def_get_mac_name( char _far *name, BUFFER _far *buf);


int _far fortran def_err_dec( BUFFER _far *buf )
 {
  char name1[512];
  char name2[512];
  char szBuf[512];

  int p, i_err;
                           /*  loop buffer : read 1 name of macros*/
  for( buf->inp_p = 0; ( buf->inp_p = def_macro( buf )) >= 0; )
   {

    p = buf->inp_p;

    if( ( i_err = def_get_mac_name( name1, buf ) ) != BUF_NULL ) return( i_err );
                           /*  loop buffer : read 2 name of macros*/
    for( buf->inp_p = buf->ilex[buf->inp_p]; (buf->inp_p) >= 0;  )
     {
      buf->inp_p = def_macro( buf );

      if( buf->inp_p < 0 ) break;
      if( ( i_err = def_get_mac_name( name2, buf ) ) != BUF_NULL ) return( i_err );
      if( strcmp( name1, name2) == 0 )
       {
		sprintf( szBuf, "Redefined %s", name1);
        put_error_messege( szBuf );
        return( BUF_ERROR_REDEC ); /*  comparable name 1 and name 2*/
        }
      buf->inp_p = buf->ilex[ buf->inp_p ] ;
      }

     buf->inp_p = buf->ilex[ p ];
    }
  return( BUF_NULL );
  }


static char name_vr_mac_file[] = VR_MAC_FILE_NAME;
static char szDefine[] = "#define ";

                     /*  load preprocessor time file ( save macro define )*/
int _far fortran load_vr_file( BUFFER _far *buf )
 {
  FILE _far *vr_mac;
  char       FgetsBuff[BUFSIZ];
  char       *str;

  int p;
  int i;

  if( (str = new char[ BUF_STR_N_MAX ]) == NULL )
  {
	  return -1;
  };

  char _far *str1;
                      /*  open time file*/
  if( (vr_mac = fopen( name_vr_mac_file, "w")) == NULL)
   {
	sprintf( str, "Can't open file", (char _far *)name_vr_mac_file);
    put_error_messege( str );
	delete []str;
    return( -11 );
    }
  setbuf( vr_mac, FgetsBuff );

  p = buf->inp_p;
  buf->inp_p = 0;
                    /*  loop BUFFER buf*/
  while( buf->inp_p >= 0 )
   {                                /*  if string is macros*/
    if( buf_cmpn( buf, 8, szDefine) == BUF_NULL )
     {                              /*  gets this string in buffer*/
      if( buf_gets( str, BUF_STR_N_MAX, buf) != BUF_NULL )
        {
         fclose( vr_mac );
		 delete []str;
         return -1;
        }
                    /*  puts string in time file*/
      for( i = 0, str1 = str; str[ i ] != '\0'; i++)
       {
	if( (str[ i ] == '\n')&&(str[ i+1 ] != '\0' ) )
	 {
	  str[ i ] = '\0';
	  fprintf( vr_mac, "%s\\\\ \n", str1);
	  str1 = str + i + 1;
	  }
	}
      fprintf( vr_mac, "%s\n", str1);
      }
    else
     {
      buf->inp_p = buf->ilex[ buf->inp_p ];
      }
    }

 fclose( vr_mac );
 delete []str;
 return BUF_NULL;
 }

                           /*  get macros name BUFFER _far *buf -> char _far name*/
static int _near fortran def_get_mac_name( char _far *name, BUFFER _far *buf)
 {
  MACROS *mac;
   
  if( (mac = new MACROS[1] ) == NULL )
  {
   return -1;
  };

  int p;

  p = buf->inp_p;
  if( buf_gets( mac->mac, BUF_STR_N_MAX, buf) != BUF_NULL ) return -1;  /*  get string BUFFER buf -> char mac.mac*/
  if(  def_separ_mac_name( buf, mac) < 0 ) return -1;
  buf->inp_p = p;
  strcpy( name, mac->m[0]);

  delete []mac;

  return( BUF_NULL );
  }                        /*  tested buffer to redefined macros name*/
