/*
**
**  def_str.      : 2 function for includer/definer
**
**  function      :
**                 main_preproc - worker includer/definer
**  HEDER FILE'S  :
**                  <stdio.h>
**                  <stdlib.h>
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
#include <malloc.h>

#include <typedef.h>
#include <tmclimit.h>
#include <mainwndw.h>
#include <tpl_file.h>
#include <tmc_lib.h>
#include <run_flag.h>
#include <error1.h>

#include "prepr1.h"



BUFFER buf;
char *szError;
char *szPathName;

                                          /*  worker definer/includer*/
/*
void MainPrepr( CString csTplFileNamePrepr,	CString csTplFileName, CString csCurrentPath, CTmcLibError &cError )
{
	if( cError.IsError() ) return;
	
	if( (szError = (char *)alloc_mem_err( sizeof(char)*BUF_STR_N_MAX )) == NULL )
	{
		cError.PutErrorMessage("Memory allocation");
		return;
	};
	if( (szPathName = (char *)alloc_mem_err( sizeof(char)*BUF_STR_N_MAX )) == NULL )
	{
		free_mem( szError );
		cError.PutErrorMessage("Memory allocation");
		return;
	};

	char *ch;
	char szSourceFileName[512];
	char szTargetFileName[512];
	
	strcpy( szSourceFileName, csTplFileName);
	strcpy( szTargetFileName, csTplFileNamePrepr);
	strcpy( szPathName, csCurrentPath);
	
	ch = main_preproc( szTargetFileName, szSourceFileName );
	unload_prep_buf();
	dest_prep();

	if( strlen(ch) != 0 ) cError.PutErrorMessage(ch);
	
	free_mem( szPathName );
	free_mem( szError );

	return;
}
*/

                                          /*  worker definer/includer*/

void MainPrepr1( char *csTplFileNamePrepr,	char *csTplFileName, char *csCurrentPath, char *csError )
{
	
	sprintf( csError, "%s", "");

	if( (szError = (char *)alloc_mem_err( sizeof(char)*BUF_STR_N_MAX )) == NULL )
	{
		printf( csError, "%s", "Memory allocation");
		return;
	};
	if( (szPathName = (char *)alloc_mem_err( sizeof(char)*BUF_STR_N_MAX )) == NULL )
	{
		free_mem( szError );
		printf( csError, "%s", "Memory allocation");
		return;
	};

	char *ch;
	char szSourceFileName[512];
	char szTargetFileName[512];
	
	strcpy( szSourceFileName, csTplFileName);
	strcpy( szTargetFileName, csTplFileNamePrepr);
	strcpy( szPathName, csCurrentPath);
	
	ch = main_preproc( szTargetFileName, szSourceFileName );
	unload_prep_buf();
	dest_prep();

	if( strlen(ch) != 0 )
	{
		sprintf( csError, "%s", ch );
	};
	
	free_mem( szPathName );
	free_mem( szError );

	return;
}



char * main_preproc( char *szTargetFileName, char *szSourceFileName )
 {
  int i_err;
  char *str;
  FILE *pfIn, *pfOut;

  sprintf( szError, "");

  if( (str = new char[ BUF_STR_INLINE_MAX ]) == NULL )
  {
	  sprintf( szError, "memory allocation" );
	  return szError;
  };

  if( (pfIn = fopen( szSourceFileName, "r" )) == NULL ) 
  {
	  sprintf( szError, "file %s isn't open", szSourceFileName );
	  delete []str;
	  return szError;
  }

  buf_init( &buf, NMAX);                               /*  init buffer*/
  buf.inp_p = 0;
  while( fgets( str, BUF_STR_INLINE_MAX, pfIn ) != NULL )				  /* read data in buffer */
   {
    if( buf.inp_p < 0 )
     {
	  sprintf( szError, "Init" );
	  fclose( pfIn );
	  delete []str;
      return szError;
      }
	DeleteComment( str );
    if( buf_puts3( str, &buf ) != BUF_NULL ) 
	{
		delete []str;
		return szError;             /*  put string in buffer*/
	};
//    if( strstr( str, str1 ) != NULL ) return 0;
    }
  fclose( pfIn );

  buf.inp_p = 0;

  if( ( i_err = def_include( &buf ) ) == 0 )                          /* include file*/
   {
    buf.inp_p = 0;
    if( ( i_err = load_vr_file( &buf ) ) != BUF_NULL) 
	{
		delete []str;
		return szError;
	};

    buf.inp_p = 0;
    if( ( i_err = def_err_dec( &buf ) ) != BUF_NULL)
	{
		delete []str;
		return szError; /*  is redefine in buffer?*/
	};

    buf.inp_p=0;                                                      /* macro define*/
    if( ( i_err = def_main_macro( &buf )) != BUF_NULL ) 
	{
		delete []str;
		return szError;
	};
    }

  buf.inp_p = 0;
  if( (pfOut = fopen( szTargetFileName, "w" )) == NULL ) 
  {
	  sprintf( szError, "file %s isn't open", szTargetFileName );
	  delete []str;
	  return szError;
  }										   
  while( buf_gets( str, BUF_STR_N_MAX, &buf ) != BUF_EOF )
  {
     fputs( str, pfOut);
   };
  fclose( pfOut );


  delete []str;
  return szError;
  }


int  unload_prep_buf( void )
 {
  if( buf.lsplex != NULL ) free_mem( buf.lsplex );
  if( buf.ilex != NULL ) free_mem( buf.ilex );
  return( 0 );
  }

int    put_error_messege( char *szError1 )
{
	strcpy( szError, szError1 );
	return 0;
}

/*
int  far free_mem( void far *mem_ptr )
{
	free(mem_ptr);
	return 0;
}

void far *alloc_mem_err( long size )
{
	return malloc( size );
}
*/

char *GetCurrentPath( void )
{
	return szPathName;
}


void DeleteComment( char *pszStr )
{
  int i;
  for( i = 0; i < BUF_STR_N_MAX; i++ )
  {
	  switch( pszStr[i] )
	  {
		case '!':
//			pszStr[i] = '\n';
//			pszStr[i+1] = '\0';
			strcpy( (pszStr + i) , " \n");
//			pszStr[i] = 13;
//			pszStr[i+1] = 10;
//			pszStr[i+2] = '\0';
			return;
			break;
		case '\0':
			return;
			break;
	  };
  };
  return;
}


int InitCharArray( char **ch, int n )
{
	if( *ch != NULL )
	{
		free( *ch );
		*ch = NULL;
	};
	if( ((*ch) = ( char *)malloc( n )) == NULL ) return -1;
	return 0;
}

int FreeCharArray( char **ch )
{
	if( *ch != NULL )
	{
		free( *ch );
		*ch = NULL;
	};
	return 0;
}
