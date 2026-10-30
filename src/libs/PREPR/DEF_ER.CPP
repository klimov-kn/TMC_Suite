/*
**
**  defer.       :  function for includer/definer
**
**  function      :
**                  def_err_get - return pointer on string with description error
**  HEDER FILE'S  :
**                  <bufm.h>
**
**  LIB           : NO
**
**  OVERLAY       : YES
**
*/

/// @file DEF_ER.CPP
/// @brief Таблица текстов для кодов возврата препроцессора (`def_err_get`).
///
/// Часть модуля «Библиотека prepr». Индекс таблицы — модуль кода (0..11).
/// @see https://www.tamic.ru/docs/code/lib-prepr.html#errors


#include "Prepr1.h"

const char  *def_err_get( int i_err )
 {
  static const char  *error[]={
			      "all right",
			      "very small memory",
			      "error in buffer",
			      "error in bracket",
			      "error in macro define",
			      "error in fact parameters",
			      "not separator or error in formal param",
			      "error when open include file ",
			      "not memory",
			      "error in formal parameters",
			      "redefine macros",
			      "error not enought memory"
			      };
  return( error[-i_err]);     /* return error cod */
  }
