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

/// @file DEFMIS.CPP
/// @brief Поиск `#define`, вставка файлов `#include`, обработка переноса строки `\\`.
///
/// Часть модуля «Библиотека prepr». `def_include()` склеивает перенесённые строки
/// (`def_sl`) и вставляет включаемые файлы с проверкой имени (только папка задания и
/// её подпапки) и глубины вложенности; `def_macro()` находит очередное определение.
/// @see https://www.tamic.ru/docs/code/lib-prepr.html#include


#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <Typedef.h>
#include <TMC_LIB.H>
#include <Error1.h>
#include <LISTING.H>
#include <MAINWNDW.H>
#include <TPL_FILE.H>
#include <INDAN.H>

#include "Prepr1.h"

/// Образец начала директивы `#define` с пробелом (8 символов).
static char  szDefine1[] = "#define ";
/// Образец начала директивы `#define` с табуляцией (8 символов).
static char  szDefine2[] = "#define\t";
                   /*  return pointer on string '#define' in buffer*/
int  def_macro( BUFFER *buf )
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

/// @brief Форматирует номер строки включаемого файла; результат никуда не выводится.
/// @param[in] line_number  номер строки
static void  put_inc_line_number( Uint line_number );
                   /*  include file in buffer and delete comment '\\' in buffer*/
/// Образец начала директивы `#include` с пробелом (9 символов).
static char  szInclude1[] = "#include ";
/// Образец начала директивы `#include` с табуляцией (9 символов).
static char  szInclude2[] = "#include\t";
/// Формат строки листинга «начало включаемого файла» (не используется).
static char  szMessage1[] = "*** Include file: \"%s\" ***";
/// Формат строки листинга «номер и текст строки» (используется только в закомментированном коде).
static char  szMessage2[] = "  %3d  %s";
/// Формат строки листинга «конец включаемого файла» (не используется).
static char  szMessage3[] = "*** End Include file: \"%s\" ***";

/// Наибольшая глубина вложенности `#include` (защита от файла, включающего сам себя).
#define DEF_INCLUDE_DEPTH_MAX 64        // S-18: limit of nested #include

/// @brief Проверяет, выходит ли имя файла `#include` за пределы папки задания.
///
/// Запрещены имена с `:` (диск, поток NTFS) и любой элемент пути, состоящий только из
/// точек (не менее двух: `..`, `...`), с учётом пробелов и табуляций.
/// @param[in] pszName  имя файла из директивы (относительный путь, разделители `\` и `/`)
/// @return 1 — имя недопустимо; 0 — допустимо.
// T-3: 1 if the #include name leaves the task folder: a part of the path that is made only of
// dots ("..", "...") or a drive name / stream (':').
static int def_include_name_outside( const char *pszName )
{
  const char *p;
  int nDots = 0, nOther = 0;

  if( strchr( pszName, ':' ) != NULL ) return 1;
  for( p = pszName; ; p++ )
  {
    if( (*p == '\\')||(*p == '/')||(*p == '\0') )
    {
      if( (nDots >= 2)&&(nOther == 0) ) return 1;
      if( *p == '\0' ) return 0;
      nDots = 0;
      nOther = 0;
    }
    else if( *p == '.' ) nDots++;
    else if( (*p != ' ')&&(*p != '\t') ) nOther++;
  }
}

int def_include( BUFFER *buf )
 {
  FILE *in;
  char       FgetsBuff[BUFSIZ];

  char		*str;
  char		*str1;
  char		*inf;

  if( (str = new char[ BUF_STR_N_MAX + 1 ]) == NULL )	// N-26
  {
	  return -1;
  };
  if( (str1 = new char[ BUF_STR_N_MAX + 1 ]) == NULL )	// N-26
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
  int        nIncDepth = 0;                    // S-18: depth of nested #include
  int        pnIncEnd[DEF_INCLUDE_DEPTH_MAX];  // S-18: node after the inserted text of each open #include
  int        nIncEnd;
  char      *pszName;
  size_t     nPath, nName;
  char       szMsg[640];

  p = buf->inp_p;

  buf->inp_p=0;
  def_sl( buf );                                  /*  delete  coment*/

  buf->inp_p = p;

  // Просмотр по всем узлам текста. Найденная директива удаляется, после её места
  // вставляется текст файла, и просмотр продолжается с начала вставки — так
  // обрабатываются вложенные #include. pnIncEnd хранит узел конца каждой открытой
  // вставки: дойдя до него, глубина вложенности уменьшается.
  while(  buf->inp_p >= 0 )                       /*  find include file name*/
   {
    while( (nIncDepth > 0)&&(buf->inp_p == pnIncEnd[nIncDepth-1]) ) nIncDepth--;  // S-18: the included text is passed
    if(buf_cmpn2( buf, 9, szInclude1, szInclude2 ) == BUF_NULL )       /*  if find '#include'*/
     {
      p1 = buf->inp_p;
      if( buf_gets( str, BUF_STR_N_MAX, buf) != BUF_NULL )
	  {
		  delete []inf;
		  delete []str1;
		  delete []str;
		  put_error_messege( "#include: the line is longer than 500000 characters or the text ends inside it" );  // S-17
		  return -1;
	  };
      n1 = (int)strlen(str);
      buf->inp_p = p1;

      buf_dels( buf, n1);                        /*  delete string with '#include'*/
      nIncEnd = buf->ilex[p1];                   // S-18: the included text is put before this node

      if( (n1 > 0)&&(str[n1-1] == '\n') ) str[n1-1] = '\0';  // N-28: it was str[n1-1] = 0 always

      inc_line = 0;                          /*  include file line counter*/
	  for( i = 9; (i < BUF_STR_N_MAX)&&( str[i] == ' '); i++ )
	  {
		  ;
	  };

      pszName = str + i;
      nPath = strlen( GetCurrentPath() );
      nName = strlen( pszName );

      if( nPath + 1 + nName > FILENAME_MAX  )     // T-2, T-7: the length is checked before the full name is built
        {
         snprintf( szMsg, sizeof(szMsg), "#include: the file name with the task folder is too long (%u characters, at most %d): %.200s%s",
                   (unsigned)(nPath + 1 + nName), (int)FILENAME_MAX, pszName, (nName > 200) ? "..." : "" );
         szMsg[sizeof(szMsg)-1] = '\0';
		  delete []inf;
		  delete []str1;
		  delete []str;
         put_error_messege( szMsg );
         return -1;
        }

      if( def_include_name_outside( pszName ) )   // T-3: only the task folder and its subfolders
        {
         snprintf( szMsg, sizeof(szMsg), "#include: the file must be in the task folder or in its subfolder ('..' and drive names are not allowed): %s", pszName );
         szMsg[sizeof(szMsg)-1] = '\0';
		  delete []inf;
		  delete []str1;
		  delete []str;
         put_error_messege( szMsg );
         return(DEF_ERROR_INFILE);
        }

      if( nIncDepth >= DEF_INCLUDE_DEPTH_MAX )    // S-18: a file that includes itself
        {
         snprintf( szMsg, sizeof(szMsg), "#include: more than %d nested files (does the file include itself?): %s", DEF_INCLUDE_DEPTH_MAX, pszName );
         szMsg[sizeof(szMsg)-1] = '\0';
		  delete []inf;
		  delete []str1;
		  delete []str;
         put_error_messege( szMsg );
         return(DEF_ERROR_INFILE);
        }

      sprintf( inf, "%s\\%s", GetCurrentPath(), pszName );

      if( (in = fopen( inf, "r")) == NULL)       /*  open including file*/
       {
         snprintf( szMsg, sizeof(szMsg), "can't open include file %s", inf );   // S-21: it was "open file"
         szMsg[sizeof(szMsg)-1] = '\0';
		  delete []inf;
		  delete []str1;
		  delete []str;
        put_error_messege( szMsg );
		return(DEF_ERROR_INFILE);
	   }                                       /*  read including file in buffer*/

      setbuf( in, FgetsBuff );

      while( fgets( str1, BUF_STR_N_MAX, in) != NULL )
       {
		   DeleteComment(str1);
        /* comments skipping */

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
      pnIncEnd[nIncDepth++] = nIncEnd;            // S-18: one more open #include

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


/// Формат номера строки включаемого файла для `put_inc_line_number()`.
static char  szFmt[] = "Incl.line:%3d";



                   /*  delete comment '\\' in buffer*/
/// Маркер переноса строки: два обратных слеша.
static char  mac[]  = "\\\\";
/// Текст, который ставится на место маркера переноса и остатка строки: пробел и перевод строки.
static char  str1[] = " \n";

int  def_sl( BUFFER *buf )
 {
  char *str;
  if( (str = new char[ BUF_STR_N_MAX + 1 ]) == NULL )	// N-26
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
      // Удаляются маркер, остаток строки и её завершающий '\0' (n1 + 1 символ), поэтому
      // следующая строка становится продолжением этой же записи.
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



