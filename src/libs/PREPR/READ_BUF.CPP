/*
**
**  def_str.      : 12 function for includer/definer
**
**  function      :
**                  read_buf - gets string from buffer
**
**  HEDER FILE'S  :
**                  <stdio.h>
**                  <bufm.h>
**
**  LIB           : oindtX.lib
**
**  OVERLAY       : YES
**
*/

/// @file READ_BUF.CPP
/// @brief Чтение одной строки из связного буфера препроцессора (`read_buf`).
///
/// Часть модуля «Библиотека prepr». Функция входит в сборку, но в коде пакета не вызывается.
/// @see https://www.tamic.ru/docs/code/lib-prepr.html#buffer

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <TMCLIMIT.H>
#include <Typedef.h>
#include <TPL_FILE.H>
#include <MAINWNDW.H>
#include <Error1.h>
#include <LISTING.H>
#include <DESCRPTR.H>

#include "Prepr1.h"







