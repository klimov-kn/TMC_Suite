/*
**
**  def_dest.     :  function for includer/definer
**
**  function      :
**                  dest_prep - destroy definer/includer : delete file VR_MAC_FILE_NAME
**  HEDER FILE'S  :
**                  <stdio.h>
**                  <bufm.h>
**
**  LIB           : NO
**
**  OVERLAY       : YES
**
*/

/// @file DEF_DEST.CPP
/// @brief Удаление временного файла макросов `$$vr$$s<PID>.prc` после прохода препроцессора (`dest_prep`).
///
/// Часть модуля «Библиотека prepr».
/// @see https://www.tamic.ru/docs/code/lib-prepr.html#vr-file

#include <stdio.h>
	 
#include <string.h>   // N-29
#include <Typedef.h>

#include "Prepr1.h"



int dest_prep( void )
 {
   char szName[512];
   if( strlen( GetCurrentPath() ) + 1 + strlen( vr_mac_file_name() ) >= sizeof(szName) ) return -1;   // N-29
   sprintf( szName, "%s\\%s", GetCurrentPath(), vr_mac_file_name());   // S-16

   return( remove( szName ) );    /* delete time-file VR_MAC_FILE_NAME */
   }
