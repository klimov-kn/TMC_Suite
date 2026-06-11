/*
**  TAMIC_soft(R) 1989,1991,1992
**
**  make_S_matrix_name   - make temporary S-file names
**  copy_S_matrix        - copy temporary file to output file if need
**
**  FILE : ..\tamic\post_pro\wrrd_S.c
**
**  LIB  : ooutpXY.lib
**
**  OVERLAY: YES
**
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <typedef.h>
#include <tmclimit.h>
#include <tmcgraph.h>
#include <run_flag.h>
#include <tmc_lib.h>
#include <indandec.h>
#include <compfile.h>
#include <frqdata.h>
#include <postpro.h>
#include <tpl_file.h>
#include <datacnst.h>

#ifndef _NDP2
  #include "..\file\_file.h"
#else
  #include "_file.h"
#endif

#ifdef _MODEL_IC
#ifndef _NDP2
  #include <mic\inpdata.h>
#else
  #include <mic\inpdata.h>
#endif
#endif


char S_filename[FILENAME_MAX];
static char _far S_filename_buff[FILENAME_MAX];
static int _far file_counter = 0;


void _far make_S_matrix_name( char _far *S_name, S_INDEX _far *si )
 {
  char _far *llist_filename = get_tpl_file_name();
  char _far *ptr, buff[80];

                                /* store file name for control*/
  if( get_current_frq_num() == 0 )
   if( file_counter == 0 )
    {
     strcpy( S_filename_buff, llist_filename );
     file_counter++;
     }
    else
     {
      if( strcmp( S_filename_buff, llist_filename ) == 0 ) file_counter = (file_counter+1)%10;
       else                      /* new name of tpl file*/
        {
         file_counter = 1;
         strcpy( S_filename_buff, llist_filename );
/*         S_filename_buff[0] = 0;*/
         }
      }
                                                  /* make S_matrix file name */
  sprintf( buff, "S%d%d", file_counter, get_tpl_step() );
  if( (ptr = strchr( buff, ' ' )) != NULL ) *ptr = '0';
  change_file_extension( S_filename, llist_filename, buff );

  strcpy( si->tpl_name, llist_filename );
  strcpy( S_name, S_filename );

#if ( defined _MODEL_E )  || ( defined _MODEL_H )
  {
   _real rTmp;
   get_data_const_delta( &rTmp );
   si->delta1 = (double)rTmp;
   si->delta2 = (double)rTmp;
   si->delta3 = 0.0;
  }
#endif
#ifdef _MODEL_IC
  si->delta1    = (double)Get_Mesh_Size_X();
  si->delta2    = (double)Get_Mesh_Size_Y();
  si->delta3    = 0.0;
#endif
#if ( defined _MODEL_M ) || ( defined _MODEL_W )
  si->delta1    = 0.0;
  si->delta2    = 0.0;
  si->delta3    = 0.0;
#endif

  }




int _far copy_S_matrix( void )
 {
  char buff[FILENAME_MAX], tag_name[FILENAME_MAX], _far *ptr;

  if( get_comp_file_flag() == 0 ) return( 0 );

  strcpy( buff, get_comp_file_name() );
  strcpy( tag_name, ( ((ptr = strrchr( buff, '\\')) != NULL)? (ptr + 1):buff) );

  if( (ptr = strrchr( tag_name, '.')) == NULL ) *(ptr = strrchr( tag_name, '\0')) = '.';
  *++ptr = '\0';

  return( copy_file( S_filename, strcat( tag_name, "S" ) ) );
  }
