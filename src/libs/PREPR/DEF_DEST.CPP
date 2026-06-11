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
	 
#include <typedef.h>

#include "prepr1.h"



int _far fortran dest_prep( void )
 {
   char szName[512];
   sprintf( szName, "%s\\%s", GetCurrentPath(), VR_MAC_FILE_NAME);

   return( remove( szName ) );    /* delete time-file VR_MAC_FILE_NAME */
   }
