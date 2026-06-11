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

#include <typedef.h>
#include <error1.h>

#include "prepr1.h"



#define  DEF_SEPARATOR '@'

/* #define _DEBUG*/

static int  _near fortran def_main_macdef( BUFFER _far *buf );

                /*  substitute all macros*/
int def_main_macro( BUFFER _far *buf )
 {
  int p, p1, i_err;

  p1 = buf->inp_p;
                /*  if not macro define return BUF_NULL*/
  if( ( p1 =  def_macro( buf ) ) < 0 )  return( BUF_NULL );

  buf->inp_p = buf->ilex[p1];  /*  if there is macro define call def_main_macro*/
  if( (i_err = def_main_macro( buf )) < 0) return( i_err);

  p = buf->inp_p;
  buf->inp_p = p1;
                   /*  substitute 1 macro*/
  i_err = def_main_macdef( buf );

  buf->inp_p = p;
  return( i_err );

  }

                   /*  substitute 1 macro define*/
int  def_main_macdef( BUFFER _far *buf )
 {
   MACROS *mac;
   
   if( (mac = new MACROS[1] ) == NULL )
   {
	   return -1;
   };

   int p1, i, n1;
#ifdef __DEBUG
   int ind;
#endif

#ifdef __DEBUG
 printf("\n function def_main_macdef");
#endif

/*
****   separation macros name and formal parameters  *****
*/
   p1 = buf->inp_p;
   if( buf_gets( mac->mac, BUF_STR_N_MAX, buf) != BUF_NULL ) 
   {
	   delete []mac;
	   return -1;
   };
   n1 = strlen( mac->mac );
   mac->mac[n1-1] = '\0';

   if( (i = def_separ_mac_name( buf, mac)) < 0 ) 
   {
	   delete []mac;
	   return( i );
   };

#ifdef __DEBUG
printf("\n after function def_separ_mac_name");
for( ind = 0; ind <= mac.j1; ind++ )
 {
  printf("\n number formal par = %d ; maska = {%s}", ind, mac.m[ind] );
  getch();
  }
#endif

/*
***** macro define ( separation formal parameter ) *****
*/
    if( def_set_formal_param( buf, mac, i) < 0 ) 
	{
		delete []mac;
		return( BUF_ERROR_NO_FORMAL_PARAM );
	};

#ifdef __DEBUG
printf("\n after function def_set_formal_param");
for( ind = 0; ind < mac.i2; ind++ )
 {
  printf("\n number of formal parameter : %d\n", mac.im1[ind]);
  if( mac.im1[ind] >= 0 )
   printf("{%s}", mac.m[mac.im1[ind]]);
  printf("{%s}", mac.m1[ind]);
  getch();
  }
#endif

/*
*****          delete macro define           *****
*/

   buf->inp_p = p1;
   buf_dels( buf, n1);

/*
*****      substitute macros in buffer        ******
*/
	i = def_subst_mac( buf, mac);
	delete []mac;
	return( i );
 }



int  def_subst_mac( BUFFER _far *buf, MACROS _far *mac)
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
      n1 = strlen( mac1 );
      if( mac->j1 > 0 ) n1++;


      for( j = 0; j < mac->j1 ; j++)         /*  read fact parameters*/
       {
	if( (i_err = def_buf_gets_param( buf, mac1+n1)) != BUF_NULL )
	{
		delete []mac1;
		return( i_err);
	};

	mac->m[ j+1 ] = mac1+n1;
	n1 += (strlen( mac1+n1) + 1);
        if( *mac->m[j+1] == '\n')
         {
          put_error_messege( "Fact parameter's" );
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
		 delete []mac1;
		 return -1;
	 };
        if( buf_puts2( mac->m1[j], buf) != BUF_NULL )
		 {
			 delete []mac1;
			 return -1;
		 };

#ifdef __DEBUG
printf("\n after function def_subst_mac\n");
  if( mac->im1[j] >= 0 )
   printf("{%s}", mac->m[mac->im1[j]]);
  printf("{%s}", mac->m1[j]);
  getch();
#endif

        if( buf->inp_p < 0 )
         {
		  delete []mac1;
          put_error_messege( "Small buffer" );
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
int  def_set_formal_param( BUFFER _far *buf, MACROS _far *mac, int i)
 {
  int i3, j, n, i1;
  char _far *ch;
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

  i += strlen( mac1 );


  mac->m1[0] = ch;
  mac->im1[0] = -1;
#ifdef __DEBUG
  printf("\n in function def_set_formal_param\n");
  printf(" mac->i2 = 0; number formal param = %d; maska string = {%s}", mac->im1[0], mac->m1[0]);
  getch();
#endif

  for( j = 0; j < mac->j1; j++ )
   {
    if( strcmp( mac1, mac->m[j+1]) == 0 )
     {
      n = strlen( mac1 );
      mac->m1[ mac->i2 ] = ch;
      mac->im1[ mac->i2 ] = j+1;
#ifdef __DEBUG
  printf("\n in function def_set_formal_param for IF\n");
  printf(" mac->i2 = %d; number formal param = %d; maska string = {%s}", mac->i2, mac->im1[mac->i2], mac->m1[mac->i2]);
  getch();
#endif
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
#ifdef __DEBUG
  printf("\n in function def_set_formal_param for ELSE\n");
  printf(" mac->i2 = 0; number formal param = %d; maska string = {%s}", mac->im1[0], mac->m1[0]);
  getch();
#endif
      };
    };


  for( i1 = i, mac->i2 = 1; mac->mac[ i ] != '\0'; )
   {
    i3 = i;
    ch = mac->mac + i;
    def_gets( mac1, ch);
    i += strlen( mac1 );

    for( j = 0; j < mac->j1; j++)
     {

      if( strcmp( mac1, mac->m[ j+1 ]) == 0 )
       {
        n = strlen( mac1 );
        mac->m1[mac->i2] = ch+1;
        mac->im1[mac->i2] = j+1;
#ifdef __DEBUG
  printf("\n in function def_set_formal_param\n");
  printf(" mac->i2 = %d; number formal param = %d; maska string = {%s}", mac->i2, mac->im1[mac->i2], mac->m1[mac->i2]);
  getch();
#endif
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
int  def_separ_mac_name( BUFFER _far *buf, MACROS _far *mac)
 {
  int  p1, j, i;
  char ib;

#ifdef __DEBUG
  printf("\n function def_separ_mac_name");
#endif

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

#ifdef __DEBUG
  printf("\n ib = %d; mac->mac[%d] = mac->mac[%d] = %c", ib, i-8+j, i, mac->mac[i]);
#endif

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
