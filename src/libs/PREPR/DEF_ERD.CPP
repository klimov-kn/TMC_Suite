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

/// @file DEF_ERD.CPP
/// @brief Проверка повторных `#define` и запись временного файла макросов.
///
/// Часть модуля «Библиотека prepr». Содержит `def_err_dec()` (поиск повторно
/// определённых имён за один проход с сортировкой), `load_vr_file()` и
/// `vr_mac_file_name()` (временный файл `$$vr$$s<PID>.prc`).
/// @see https://www.tamic.ru/docs/code/lib-prepr.html#redefine

#include <stdio.h>
#include <string.h>
#include <stdlib.h>   // BUG-004: qsort, realloc
#include <process.h>  // S-16: _getpid
/// Текст ошибки: запись `#define` длиннее `BUF_STR_N_MAX` символов или текст кончился внутри
/// неё (например, перенос `\\` в последней строке файла).
// S-17: buf_gets did not read the line of #define up to its end
#define DEF_MSG_LINE (char *)"#define: the line is longer than 500000 characters or the text ends inside it (a '\\\\' at the end of the file?)"

#include <Typedef.h>
#include <Error1.h>

#include "Prepr1.h"


/// @brief Читает запись `#define` в позиции `buf->inp_p` и выделяет из неё имя макроса.
///
/// @param[out]    mac  рабочая структура; имя — `mac->m[0]`
/// @param[in,out] buf  буфер; `inp_p` восстанавливается при успехе
/// @return `BUF_NULL` — успех; `-1` — запись слишком длинная или ошибка в заголовке
///         `#define` (текст ошибки дополняется самой строкой).
static int def_get_mac_name( MACROS *mac, BUFFER *buf);

/// Текст ошибки препроцессора (определён в prep_mn.cpp).
extern char *szError;       // Prep_mn.cpp: the text of the error

/// @brief Имя одного `#define` в списке для поиска повторов.
typedef struct              // BUG-004: name of one #define
{
  /// Смещение имени в общем массиве имён, байт.
  size_t      nOff;         // offset of the name in the common buffer
  /// Порядковый номер `#define` в тексте (с 0).
  int         n;            // number of the #define in the text
  /// Указатель на имя (заполняется после сбора всех имён).
  const char *psz;
} DEF_MAC_NAME;

/// @brief Сравнивает два элемента `DEF_MAC_NAME` для `qsort`: по имени, при равенстве — по номеру.
/// @param[in] a  первый элемент
/// @param[in] b  второй элемент
/// @return <0, 0, >0 — как `strcmp`.
static int def_mac_name_cmp( const void *a, const void *b )   // BUG-004: by name, then by number
{
  const DEF_MAC_NAME *pa = (const DEF_MAC_NAME *)a;
  const DEF_MAC_NAME *pb = (const DEF_MAC_NAME *)b;
  int i = strcmp( pa->psz, pb->psz );
  if( i != 0 ) return i;
  return (pa->n < pb->n) ? -1 : ((pa->n > pb->n) ? 1 : 0);
}

/// @brief Формирует ошибку «`Redefined <имя>`» (имя обрезается до 200 символов).
/// @param[in] pszName  повторно определённое имя
/// @return `BUF_ERROR_REDEC`.
static int def_err_redec( const char *pszName )   // BUG-002: the name can be long
{
  char szBuf[300];
  snprintf( szBuf, sizeof(szBuf), "Redefined %.200s%s", pszName, (strlen( pszName ) > 200) ? "..." : "" );
  szBuf[sizeof(szBuf)-1] = '\0';
  put_error_messege( szBuf );
  return( BUF_ERROR_REDEC );
}

int def_err_dec( BUFFER *buf )
 {
  // BUG-002, BUG-004: one pass over the #define with one MACROS buffer (see the head of the file)
  MACROS       *mac;
  char         *pszNames = NULL, *pszNew;
  DEF_MAC_NAME *pList = NULL, *pListNew;
  size_t        nUsed = 0, nSize = 0, nLen, nList = 0, nMax = 0, i, j;
  int           i_err = BUF_NULL, bBad = 0, nRedec = -1;
  const char   *pszRedec = NULL;

  if( (mac = new MACROS[1] ) == NULL )
  {
    put_error_messege( "preprocessor: not enough memory (MACROS)" );
    return -1;
  };
                           /*  loop buffer : read the names of all macros*/
  for( buf->inp_p = 0; ( buf->inp_p = def_macro( buf )) >= 0; buf->inp_p = buf->ilex[ buf->inp_p ] )
   {
    if( ( i_err = def_get_mac_name( mac, buf ) ) != BUF_NULL )
     {
      bBad = 1;            // the first bad #define: its number is nList
      break;
      }
    nLen = strlen( mac->m[0] ) + 1;
    if( nUsed + nLen > nSize )
     {
      nSize = (nUsed + nLen) * 2 + 4096;
      if( (pszNew = (char *)realloc( pszNames, nSize )) == NULL ) { i_err = -1; bBad = 2; break; }
      pszNames = pszNew;
      }
    if( nList >= nMax )
     {
      nMax = nMax * 2 + 256;
      if( (pListNew = (DEF_MAC_NAME *)realloc( pList, nMax * sizeof(DEF_MAC_NAME) )) == NULL ) { i_err = -1; bBad = 2; break; }
      pList = pListNew;
      }
    memcpy( pszNames + nUsed, mac->m[0], nLen );
    pList[nList].nOff = nUsed;
    pList[nList].n = (int)nList;
    nUsed += nLen;
    nList++;
    }
  delete []mac;

  if( bBad == 2 )
   {
    put_error_messege( "preprocessor: not enough memory (names of #define)" );
    free( pszNames );
    free( pList );
    return( i_err );
    }
  for( i = 0; i < nList; i++ ) pList[i].psz = pszNames + pList[i].nOff;

  if( bBad )
   {                       // as before: only the first #define was compared with the next ones
    for( i = 1; i < nList; i++ )
      if( strcmp( pList[0].psz, pList[i].psz ) == 0 ) { pszRedec = pList[0].psz; break; }
    }
  else if( nList > 1 )
   {                       // the first #define (in the text) whose name is repeated later
    qsort( pList, nList, sizeof(DEF_MAC_NAME), def_mac_name_cmp );
    for( i = 0; i + 1 < nList; i = j )
     {
      for( j = i + 1; (j < nList)&&(strcmp( pList[i].psz, pList[j].psz ) == 0); j++ ) ;
      if( (j > i + 1)&&((nRedec < 0)||(pList[i].n < nRedec)) )
       {
        nRedec = pList[i].n;
        pszRedec = pList[i].psz;
        }
      }
    }

  if( pszRedec != NULL ) i_err = def_err_redec( pszRedec );
  free( pszNames );
  free( pList );
  return( i_err );
  }


/// Имя временного файла макросов текущего процесса; до первого `vr_mac_file_name()` —
/// базовое `VR_MAC_FILE_NAME`.
static char name_vr_mac_file[64] = VR_MAC_FILE_NAME;
const char *vr_mac_file_name( void )   // S-16: the name with the process number
{
  if( strcmp( name_vr_mac_file, VR_MAC_FILE_NAME ) == 0 )
    snprintf( name_vr_mac_file, sizeof(name_vr_mac_file), "$$vr$$s%lu.prc", (unsigned long)_getpid() );
  return name_vr_mac_file;
}
/// Образец начала директивы для `load_vr_file()` (только с пробелом, 8 символов).
static char szDefine[] = "#define ";

                     /*  load preprocessor time file ( save macro define )*/
int load_vr_file( BUFFER *buf )
 {
  FILE *vr_mac;
  char       FgetsBuff[BUFSIZ];
  char       *str;

  int p;
  int i;

  if( (str = new char[ BUF_STR_N_MAX + 1 ]) == NULL )   // N-26
  {
	  put_error_messege( "preprocessor: not enough memory" );   // S-17
	  return -1;
  };

  char *str1;
                      /*  open time file*/
  char szVrName[512];	/* A-19: v papke zadaniya, kak v #include i dest_prep */
  vr_mac_file_name();   // S-16
  if( strlen( GetCurrentPath() ) + 1 + strlen( name_vr_mac_file ) >= sizeof(szVrName) )   // N-29, T-7: it was cut
   {
    snprintf( str, BUF_STR_N_MAX, "the task folder name is too long (%u characters, at most %u): %.200s...",
              (unsigned)strlen( GetCurrentPath() ), (unsigned)(sizeof(szVrName) - 2 - strlen( name_vr_mac_file )), GetCurrentPath() );
    put_error_messege( str );
    delete []str;
    return( -11 );
    }
  if( GetCurrentPath()[0] != '\0' ) _snprintf( szVrName, sizeof(szVrName), "%s\\%s", GetCurrentPath(), name_vr_mac_file );
  else                              _snprintf( szVrName, sizeof(szVrName), "%s", name_vr_mac_file );
  szVrName[sizeof(szVrName)-1] = '\0';
  // I-07: the file is not written - nobody reads it (old: fopen "w" below; no right to write -> error -11)
  /// I-07: временный файл макросов не пишется: его никто не читает; без права записи в папку задания
  /// препроцессор останавливался. Проверка строк `#define` (DEF_MSG_LINE) ниже сохранена.
  vr_mac = NULL;

  p = buf->inp_p;
  buf->inp_p = 0;
                    /*  loop BUFFER buf*/
  while( buf->inp_p >= 0 )
   {                                /*  if string is macros*/
    if( buf_cmpn( buf, 8, szDefine) == BUF_NULL )
     {                              /*  gets this string in buffer*/
      if( buf_gets( str, BUF_STR_N_MAX, buf) != BUF_NULL )
        {
         put_error_messege( DEF_MSG_LINE );   // S-17
         if( vr_mac != NULL ) fclose( vr_mac );	// I-07
		 delete []str;
         return -1;
        }
                    /*  puts string in time file*/
      // Внутренние переводы строки многострочного определения снова записываются
      // с переносом: строка, два обратных слеша, пробел.
      for( i = 0, str1 = str; str[ i ] != '\0'; i++)
       {
	if( (str[ i ] == '\n')&&(str[ i+1 ] != '\0' ) )
	 {
	  str[ i ] = '\0';
	  if( vr_mac != NULL ) fprintf( vr_mac, "%s\\\\ \n", str1);	// I-07
	  str1 = str + i + 1;
	  }
	}
      if( vr_mac != NULL ) fprintf( vr_mac, "%s\n", str1);	// I-07
      }
    else
     {
      buf->inp_p = buf->ilex[ buf->inp_p ];
      }
    }

 if( vr_mac != NULL ) fclose( vr_mac );	// I-07
 delete []str;
 return BUF_NULL;
 }

                           /*  get macros name BUFFER _far *buf -> char _far name*/
static int def_get_mac_name( MACROS *mac, BUFFER *buf)
 {                         // BUG-004: the buffer MACROS is given by the caller; the name is mac->m[0]
  int p;
  char szLine[160];
  char szMsg[600];

  p = buf->inp_p;
  if( buf_gets( mac->mac, BUF_STR_N_MAX, buf) != BUF_NULL )   /*  get string BUFFER buf -> char mac.mac*/
  {
    put_error_messege( DEF_MSG_LINE );   // S-17
    return -1;
  }
  snprintf( szLine, sizeof(szLine), "%s", mac->mac );          // S-21: the line for the message
  szLine[sizeof(szLine)-1] = '\0';
  if( strchr( szLine, '\n' ) != NULL ) *strchr( szLine, '\n' ) = '\0';
  if(  def_separ_mac_name( buf, mac) < 0 )
  {
    snprintf( szMsg, sizeof(szMsg), "%s in: %s", szError, szLine );   // S-21
    szMsg[sizeof(szMsg)-1] = '\0';
    put_error_messege( szMsg );
    return -1;
  }
  buf->inp_p = p;

  return( BUF_NULL );
  }                        /*  tested buffer to redefined macros name*/
