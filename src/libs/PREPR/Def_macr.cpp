/*
**
**  def_macr.     : 5 function for includer/definer
**
**  function      :
**                  def_main_macro - substitute all macros
**                  def_main_macdef - substitute 1 macro
**                  def_subst_mac - substitute macros mac in buffer buf
**                  def_set_formal_param - set formal parameters in macros mac
**                  def_separ_mac_name - separate macros name and formal parameters
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
#include <stdlib.h>   // BUG-004: realloc

#include <typedef.h>
#include <error1.h>

#include "prepr1.h"



#define  DEF_SEPARATOR '@'

/* #define _DEBUG*/

static int  def_main_macdef( BUFFER *buf );

static MACROS *s_pMacMain = NULL;   // BUG-004: one MACROS buffer for all #define

extern char *szError;               // Prep_mn.cpp: the text of the error

static void def_macro_message( const char *pszWhat, const char *pszName )   // S-21
{
  char szMsg[400];
  snprintf( szMsg, sizeof(szMsg), "%s (macro %.200s)", pszWhat, pszName );
  szMsg[sizeof(szMsg)-1] = '\0';
  put_error_messege( szMsg );
}

                /*  substitute all macros*/
int def_main_macro( BUFFER *buf )
 {
  // BUG-004: without recursion and with one MACROS buffer (see the head of the file)
  int  p1, i_err = BUF_NULL, nPos = 0, nMax = 0, k, pLast;
  int *pnPos = NULL, *pnNew;
                /*  the positions of all #define, as the recursion found them*/
  for( p1 = def_macro( buf ); p1 >= 0; p1 = def_macro( buf ) )
   {
    if( nPos >= nMax )
     {
      nMax = nMax * 2 + 256;
      if( (pnNew = (int *)realloc( pnPos, nMax * sizeof(int) )) == NULL )
       {
        free( pnPos );
        put_error_messege( "preprocessor: not enough memory (list of #define)" );
        return -1;
        }
      pnPos = pnNew;
      }
    pnPos[nPos++] = p1;
    buf->inp_p = buf->ilex[p1];
    }
                /*  if not macro define return BUF_NULL*/
  if( nPos == 0 ) return( BUF_NULL );
  pLast = buf->inp_p;

  if( (s_pMacMain = new MACROS[1] ) == NULL )
   {
    free( pnPos );
    put_error_messege( "preprocessor: not enough memory (MACROS)" );
    return -1;
    }
                   /*  substitute the macros from the last one*/
  for( k = nPos - 1; k >= 0; k-- )
   {
    buf->inp_p = pnPos[k];
    i_err = def_main_macdef( buf );
    if( i_err < 0 ) break;
    }

  delete []s_pMacMain;
  s_pMacMain = NULL;
  free( pnPos );
  buf->inp_p = pLast;
  return( i_err );
  }

                   /*  substitute 1 macro define*/
int  def_main_macdef( BUFFER *buf )
 {
   MACROS *mac = s_pMacMain;   // BUG-004: it was new MACROS (~10 MB) for every #define

   int p1, i, n1;


/*
****   separation macros name and formal parameters  *****
*/
   p1 = buf->inp_p;
   if( buf_gets( mac->mac, BUF_STR_N_MAX, buf) != BUF_NULL ) 
   {
	   ;   // BUG-004: MACROS is freed in def_main_macro
	   return -1;
   };
   n1 = (int)( strlen( mac->mac ) );
   // I-09: only the end of line is cut (old: mac->mac[n1-1] = '\0'; - the last character of a #define in the
   // last line of the file without the end of line was lost)
   if( ( n1 > 0 )&&( mac->mac[n1-1] == '\n' ) ) mac->mac[n1-1] = '\0';

   if( (i = def_separ_mac_name( buf, mac)) < 0 ) 
   {
	   ;   // BUG-004: MACROS is freed in def_main_macro
	   return( i );
   };


/*
***** macro define ( separation formal parameter ) *****
*/
    if( def_set_formal_param( buf, mac, i) < 0 ) 
	{
		;   // BUG-004: MACROS is freed in def_main_macro
		return( BUF_ERROR_NO_FORMAL_PARAM );
	};


/*
*****          delete macro define           *****
*/

   buf->inp_p = p1;
   buf_dels( buf, n1);

/*
*****      substitute macros in buffer        ******
*/
	i = def_subst_mac( buf, mac);
	;   // BUG-004: MACROS is freed in def_main_macro
	return( i );
 }



int  def_subst_mac( BUFFER *buf, MACROS *mac)
 {
   char *mac1;

  if( (mac1 = new char[ BUF_STR_N_MAX ]) == NULL )
  {
	  return -1;
  };


  int p1, n1, j, i_err;


  while( buf->inp_p >= 0)
   {
    p1 = buf->inp_p;

    def_buf_gets_mac( buf, mac1);             /*  read macros name in mac1*/

    if( strcmp( mac1, mac->m[ 0 ]) == 0 )     /*  if mac1 is macros name*/
     {
      n1 = (int)( strlen( mac1 ) );
      if( mac->j1 > 0 ) n1++;


      for( j = 0; j < mac->j1 ; j++)         /*  read fact parameters*/
       {
	if( (i_err = def_buf_gets_param( buf, mac1+n1)) != BUF_NULL )
	{
		def_macro_message( szError, mac->m[ 0 ] );   // S-21
		delete []mac1;
		return( i_err);
	};

	mac->m[ j+1 ] = mac1+n1;
	n1 += (int)( (strlen( mac1+n1) + 1) );
        if( *mac->m[j+1] == '\n')
         {
          def_macro_message( "Fact parameters: a parameter or ';' is missing", mac->m[ 0 ] );   // S-21
		  delete []mac1;
          return( BUF_ERROR_FACT_PARAM);
          }
	};

      buf->inp_p = p1;

      buf_dels( buf, n1);

      for( j = 0; j < mac->i2; j++ )         /*  substitute fact parameters*/
       {
	if( mac->im1[j] >= 0 )
	 if( buf_puts2( mac->m[mac->im1[j]], buf) != BUF_NULL ) 
	 {
		 def_macro_message( szError, mac->m[ 0 ] );   // S-21
		 delete []mac1;
		 return -1;
	 };
        if( buf_puts2( mac->m1[j], buf) != BUF_NULL )
		 {
			 def_macro_message( szError, mac->m[ 0 ] );   // S-21
			 delete []mac1;
			 return -1;
		 };


        if( buf->inp_p < 0 )
         {
		  delete []mac1;
          def_macro_message( "Small buffer (is the macro recursive?)", mac->m[ 0 ] );   // S-21
          return(BUF_EOF );
          }
	};
      buf->inp_p = buf->ilex[ p1 ];
    };
   };
 delete []mac1;
 return( BUF_NULL);
 }
                     /*  set formal parameters in struct. macros*/
int  def_set_formal_param( BUFFER *buf, MACROS *mac, int i)
 {
  int i3, j, n, i1;
  char *ch;
  char  *mac1;

  if( (mac1 = new char[ BUF_STR_N_MAX ]) == NULL )
  {
	  return -1;
  };

  i++;
  mac->i2 = 0;
  i3 = i;
  ch = mac->mac + i;

  def_gets( mac1, ch);            /*  read string with after '@' in mac1*/

  i += (int)( strlen( mac1 ) );


  mac->m1[0] = ch;
  mac->im1[0] = -1;

  for( j = 0; j < mac->j1; j++ )
   {
    if( strcmp( mac1, mac->m[j+1]) == 0 )
     {
      n = (int)( strlen( mac1 ) );
      mac->m1[ mac->i2 ] = ch;
      mac->im1[ mac->i2 ] = j+1;
      mac->i2++;
      if( mac->i2 >= BUF_MASKA_MAX )
       {
		put_error_messege( "Maska num" );
		delete []mac1;
		return -1;
		}
      i -= n;

      j=0;
      while( mac->mac[i3+j+n] != '\0' )
       {
        mac->mac[i3+j] = mac->mac[i3+j+n];
        j++;
        };
      mac->mac[i3+j] = '\0';

      j = 1000;
      }
     else
      {
       mac->m1[0] = ch;
       mac->im1[0] = -1;
      };
    };


  for( i1 = i, mac->i2 = 1; mac->mac[ i ] != '\0'; )
   {
    i3 = i;
    ch = mac->mac + i;
    def_gets( mac1, ch);
    i += (int)( strlen( mac1 ) );

    for( j = 0; j < mac->j1; j++)
     {

      if( strcmp( mac1, mac->m[ j+1 ]) == 0 )
       {
        n = (int)( strlen( mac1 ) );
        mac->m1[mac->i2] = ch+1;
        mac->im1[mac->i2] = j+1;
        mac->i2++;
	if( mac->i2 >= BUF_MASKA_MAX )
	 {
	  put_error_messege( "Maska num" );
	  delete []mac1;
	  return -1;
	  }
        i -= (n-1);

        mac->mac[i3] = '\0';
        j=0;
        while( mac->mac[i3+n+j] != '\0' )
         {
          mac->mac[i3+j+1] = mac->mac[i3+n+j];
          j++;
          };
        mac->mac[i3+j+1] = '\0';

        j = 1000;
        };
      };
    };

  delete []mac1;
  return( i );
  }
                /*  separate macros name and name formal parameter*/
                /*  return i - pointer separator position*/
int  def_separ_mac_name( BUFFER *buf, MACROS *mac)
 {
  int  p1, j, i;
  char ib;


  p1 = buf->inp_p;

  if( p1 < 0 )
   {
    put_error_messege( "Missing separator" );
    return( BUF_ERROR_NO_SEP );
    }

  j = 0;
  mac->j1 = 0;
  ib = 4;
  mac->m[ 0 ] = mac->mac;

  for( i = 8; mac->mac[ i ] != DEF_SEPARATOR; i++)
   {


    switch( ib )
     {
      case 4 :
              switch( mac->mac[ i ] )
               {
                case '\t' : ;
                case ' '  :
                           j--;
                           break;

               default    :
                           ib = 0;
                           mac->mac[i-8+j] = mac->mac[i];
                           break;

                case '@'   : ;
                case  '\0' :
                             put_error_messege( "Missing separator or mac name" );
                             return( BUF_ERROR_NO_SEP);
               };
              break;

      case 0 :
              switch( mac->mac[ i ] )
               {
                case '\t' : ;
                case ' ' : ib = 1;
                           mac->mac[i-8+j] = '\0';
                           mac->j1++;j++;
			   if( mac->j1 >= BUF_PAR_MAX )
			    {
			     put_error_messege( "Number of formal parameters > BUF_PAR_MAX" );
			     return -1;
			     }
                           mac->m[mac->j1] = mac->mac+i+1-8+j;
                           break;

               default    :mac->mac[i-8+j] = mac->mac[i];
                           break;

               case  '\0' :put_error_messege( "Missing separator" );
                           return( BUF_ERROR_NO_SEP);
               };
              break;

      case 1 :
              switch( mac->mac[ i ] )
               {
                case '\t' : ;
                case ' ' : j--;
                           break;

                case ';' : put_error_messege( "Missing formal parameter's" );
                           return( BUF_ERROR_NO_FORMAL_PARAM );

                default  : ib = 2;
                           mac->mac[i-8+j] = mac->mac[i];
                           break;
                case '\0': put_error_messege( "Missing separator" );
                           return( BUF_ERROR_NO_SEP);
                };
               break;

      case 2 :
              switch( mac->mac[ i ] )
               {
                case '\t' : ;
                case ' ' : ib = 3;
                           j--;
                           break;

                case ';' : ib = 1;
                           mac->mac[i-8+j] = '\0';
                           mac->j1++;j++;
			   if( mac->j1 >= BUF_PAR_MAX )
			    {
			     put_error_messege( "Number of formal parameters > BUF_PAR_MAX" );
			     return -1;
			     }
                           mac->m[mac->j1] = mac->mac+i+1-8+j;
                           break;

                default  : mac->mac[i-8+j] = mac->mac[i];
                           break;

                case '\0': put_error_messege( "Missing separator" );
                           return( BUF_ERROR_NO_SEP);
                };
               break;

      case 3 :
              switch( mac->mac[ i ] )
               {
                case '\t' : ;
                case ' ' : j--;
                           break;

                case ';' : ib = 1;
                           mac->mac[i-8+j] = '\0';
                           mac->j1++;j++;
			   if( mac->j1 >= BUF_PAR_MAX )
			    {
			     put_error_messege( "Number of formal parameters > BUF_PAR_MAX" );
			     return -1;
			     }
                           mac->m[mac->j1] = mac->mac+i+1-8+j;
                           break;

                default  : put_error_messege( "Missing formal parameter's" );
                           return( BUF_ERROR_NO_FORMAL_PARAM );

                case '\0': put_error_messege( "Missing separator" );
                           return( BUF_ERROR_NO_SEP);
                };
              break;
       };
    };

  if( ib > 1 )
   {
    put_error_messege( "Missing formal parameter's" );
    return( BUF_ERROR_NO_FORMAL_PARAM );
    }
  mac->mac[ i-8+j ] = '\0';
  mac->j1--;

  return( i );
  }
