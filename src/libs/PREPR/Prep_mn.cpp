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

/// @file Prep_mn.cpp
/// @brief Главный проход препроцессора .tpl: точка входа, чтение задания, запись результата.
///
/// Часть модуля «Библиотека prepr». Содержит `MainPrepr1()` и `main_preproc()`, глобальный
/// связный буфер `buf`, приёмник текста ошибки `szError` с функцией `put_error_messege`,
/// папку задания (`GetCurrentPath`), вырезание комментариев `!` и мелкие функции памяти.
/// @see https://www.tamic.ru/docs/code/lib-prepr.html#pipeline

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>

#include <Typedef.h>
#include <TMCLIMIT.H>
#include <MAINWNDW.H>
#include <TPL_FILE.H>
#include <TMC_LIB.H>
#include <RUN_FLAG.H>
#include <Error1.h>

#include "Prepr1.h"



/// Связный буфер со всем текстом задания на время одного прохода препроцессора.
/// Создаётся в `main_preproc()` (`buf_init`), освобождается `unload_prep_buf()`.
BUFFER buf;
/// Текст последней ошибки препроцессора (английский), `BUF_STR_N_MAX` байт; пустая строка —
/// ошибок нет. Выделяется и освобождается `MainPrepr1()`, пишется `put_error_messege`.
char *szError;
/// Папка задания без завершающей косой черты, `BUF_STR_N_MAX` байт. Выделяется в
/// `MainPrepr1()`, возвращается `GetCurrentPath()`.
char *szPathName;

                                          /*  worker definer/includer*/

                                          /*  worker definer/includer*/

void MainPrepr1( char *csTplFileNamePrepr,	char *csTplFileName, char *csCurrentPath, char *csError )
{
	
	sprintf( csError, "%s", "");

	if( (szError = (char *)alloc_mem_err( sizeof(char)*BUF_STR_N_MAX )) == NULL )
	{
		sprintf( csError, "%s", "Memory allocation");	// S-17: it was printf
		return;
	};
	if( (szPathName = (char *)alloc_mem_err( sizeof(char)*BUF_STR_N_MAX )) == NULL )
	{
		free_mem( szError );
		sprintf( csError, "%s", "Memory allocation");	// S-17: it was printf
		return;
	};

	char *ch;
	char szSourceFileName[512];
	char szTargetFileName[512];
	
	if( (strlen( csTplFileName ) >= sizeof(szSourceFileName))||(strlen( csTplFileNamePrepr ) >= sizeof(szTargetFileName))||
		(strlen( csCurrentPath ) >= BUF_STR_N_MAX) )	// N-29: the names are copied into 512-byte buffers
	{
		sprintf( csError, "the task file name is too long (more than %d characters): %.200s...", (int)sizeof(szSourceFileName) - 1, csTplFileName );
		free_mem( szPathName );
		free_mem( szError );
		return;
	};
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



/// @brief Гарантирует непустой текст ошибки: если `szError` пуст, пишет в него общий текст по коду.
/// @param[in] i_err  код возврата функции препроцессора; 0 — ничего не делается
/// @return `szError`.
// S-17: a function of the preprocessor returned an error code without a text -> generic text
static char *prep_error( int i_err )
{
  if( (i_err != 0)&&(szError[0] == '\0') )
  {
    if( (i_err < 0)&&(i_err >= ERROR_INIT) ) sprintf( szError, "preprocessor error %d (%s)", i_err, def_err_get( i_err ) );
    else sprintf( szError, "preprocessor error %d", i_err );
  }
  return szError;
}

char * main_preproc( char *szTargetFileName, char *szSourceFileName )
 {
  int i_err;
  char *str;
  FILE *pfIn, *pfOut;

  sprintf( szError, "");
  remove( szTargetFileName );   // S-17: the file of the previous run must not be read after an error

  if( (str = new char[ BUF_STR_INLINE_MAX + 1 ]) == NULL )   // N-26: buf_gets writes n characters and 0
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

  if( buf_init( &buf, NMAX) != BUF_NULL )             /*  init buffer*/   // N-27: it was not checked
  {
    buf.lsplex = NULL;
    buf.ilex = NULL;
    fclose( pfIn );
    delete []str;
    sprintf( szError, "preprocessor: not enough memory for the buffer" );
    return szError;
  }
  // Шаг 1: файл задания построчно в буфер; каждая строка — отдельная запись с '\0',
  // комментарий '!' отрезается сразу при чтении.
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
		fclose( pfIn );	// N-27
		delete []str;
		return szError;             /*  put string in buffer*/
	};
    }
  fclose( pfIn );

  buf.inp_p = 0;

  // Шаг 2: склейка переносов и вставка #include; шаг 3: временный файл макросов;
  // шаг 4: проверка повторных #define; шаг 5: подстановка макросов.
  if( ( i_err = def_include( &buf ) ) == 0 )                          /* include file*/
   {
    buf.inp_p = 0;
    if( ( i_err = load_vr_file( &buf ) ) != BUF_NULL) 
	{
		delete []str;
		return prep_error( i_err );	// S-17
	};

    buf.inp_p = 0;
    if( ( i_err = def_err_dec( &buf ) ) != BUF_NULL)
	{
		delete []str;
		return prep_error( i_err ); /*  is redefine in buffer?*/	// S-17
	};

    buf.inp_p=0;                                                      /* macro define*/
    if( ( i_err = def_main_macro( &buf )) != BUF_NULL ) 
	{
		delete []str;
		return prep_error( i_err );	// S-17
	};
    }
  else
  {
	// I-08: an #include error - the result file is not written (old: prep_error( i_err ); and the file was written)
	delete []str;
	return prep_error( i_err );   // S-17: #include error without a text
  };

  // Шаг 6: запись буфера в файл результата (I-08: только без ошибок; раньше — и после ошибки #include).
  buf.inp_p = 0;
  if( (pfOut = fopen( szTargetFileName, "w" )) == NULL ) 
  {
	  sprintf( szError, "file %s isn't open", szTargetFileName );
	  delete []str;
	  return szError;
  }										   
  while( (i_err = buf_gets( str, BUF_STR_N_MAX, &buf )) != BUF_EOF )
  {
     fputs( str, pfOut);
     if( i_err == BUF_ABN ) fputc( buf.lsplex[buf.inp_p], pfOut );  // N-26: buf_gets stopped before this character, the next call skips it
   };
  fclose( pfOut );


  delete []str;
  return szError;
  }


int  unload_prep_buf( void )
 {
  if( buf.lsplex != NULL ) free_mem( buf.lsplex );
  if( buf.ilex != NULL ) free_mem( buf.ilex );
  buf.lsplex = NULL;   // N-27: a second call must not free the memory again
  buf.ilex = NULL;
  return( 0 );
  }

// Определение `put_error_messege( const char * )` (объявлена в error1.h) для препроцессора:
// копирует текст ошибки в `szError`; длина текста должна быть меньше BUF_STR_N_MAX.
// Возвращает 0.
int    put_error_messege( const char *szError1 )
{
	strcpy( szError, szError1 );
	return 0;
}


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
			strcpy( (pszStr + i) , " \n");
			return;
			break;
		case '\0':
			return;
			break;
	  };
  };
  return;
}



