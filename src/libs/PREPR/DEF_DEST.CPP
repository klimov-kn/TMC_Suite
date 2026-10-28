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

#include <stdio.h>
	 
#include <string.h>   // N-29
#include <typedef.h>

#include "prepr1.h"



int dest_prep( void )
 {
   char szName[512];
   if( strlen( GetCurrentPath() ) + 1 + strlen( vr_mac_file_name() ) >= sizeof(szName) ) return -1;   // N-29
   sprintf( szName, "%s\\%s", GetCurrentPath(), vr_mac_file_name());   // S-16

   return( remove( szName ) );    /* delete time-file VR_MAC_FILE_NAME */
   }
