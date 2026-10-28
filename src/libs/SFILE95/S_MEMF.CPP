/*
 *  TAMIC_soft(R) 1992 vR1.02
 *
 *  processor - processing routine for reading s-files
 *
 *  FILE: ..\sread.c
 *
 *  OVERLAY: NO
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include <typedef.h>
#include <tmclimit.h>
#include <complex1.h>
#include <tmc_lib.h>
#include <frqdata.h>
#include <s_file.h>
#include <error1.h>
#include <proc_s.h>


#include <malloc.h>

void *  alloc_mem_err( long size )
  {
   void *ptr;
   ptr = (void *)malloc( (size_t)size );
   if( ptr == NULL ) put_error_messege( FATAL_MEM_ALLOC_ERROR );
   return ( ptr );
  }

int  free_mem( void *mem_ptr )
  {
   free( mem_ptr );
   return 0;
  }

int  free_mem_null( void **ptr )
  {
   if( *ptr != NULL )
     {
      free( *ptr );
      *ptr = NULL;
     }
   return 0;
  }

void stat_messege( int num, char *mes ) { return; }

