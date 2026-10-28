/*
**  TAMIC_soft(R) 1989,1991,1992 vR1.02
**
**  Get_S_FILE    - return pointer at S-file structure
**  Set_PS_Number - set number of "pascal" strings in the S-file
**  Get_PS_Number - return number of "pascal" strings int the S-file
**  Open_S_file   - open scattering matrix file( S-file )
**  Close_S_file  - close S-file
**  skip_string         - skip "pascal" string in the S-file
**  write_S             - write item to S-file
**  read_S              - read item from S-file
**
**  FILE : ..\source\tamic\post_pro\S_file.c
**
**  LIB  : ooutpXY.lib
**
**  OVERLAY: YES
**
*/

#include <stdio.h>
#include <string.h>


#include <typedef.h>
#include <tmclimit.h>
#include <tmc_lib.h>
#include <frqdata.h>
#include <error1.h>
#include <s_file.h>


static FILE *sf = NULL;
static int nPS_Number;


FILE   * Get_S_FILE( void ) { return( sf ); }

void   Set_PS_Number( int nNum ) { nPS_Number = nNum; }
int    Get_PS_Number( void ) { return nPS_Number; }

int Open_S_file( char *szFile, int nOpenMode )
 {
  if( (sf = fopen( szFile, (nOpenMode == S_RD)?"rb":( (get_current_frq_num() == 0)?"wb+":"rb+" ) )) != NULL )
   {

   }
  else
   {
    put_error_messege( ERROR_OPEN_S, szFile );
    return -1;
   }

  return 0;
 } /*end of Open_S_file routine*/



void Close_S_file( void )
 {
  fclose( sf );
  sf = NULL;
 } /*end of Close_S_file routine*/



int skip_string( void )
 {
  INT_1 length;                 /* string length*/

  /* reading string length */
  if( fread( &length, 1, (size_t)sizeof( INT_1 ), sf ) != (size_t)sizeof( INT_1 ) ) return -1;
  /* skipping string with delimiter */
  if( fseek( sf, (long)(sizeof( char )*length+sizeof(INT_1)), SEEK_CUR ) != 0 ) return -1;
  return 0;
 } /*end of skip_string routine*/



int write_S( void *ptr, size_t size )
 {
  return( (fwrite( ptr, size, 1, sf ) == 1)? 0:-1 );
  }


int read_S( void *string, size_t size )
 {
  return( (fread( string, size, 1, sf ) == 1)? 0:-1);  /* reading string of given length*/
  }

