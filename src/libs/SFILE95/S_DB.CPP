/*
 *  TAMIC_soft(R) 1992 vR1.02
 *
 *  set_file_name - set S-file name
 *  get_file_name - return pointer at S-file name
 *
 *  FILE: ..\s_db.c
 *
 *  OVERLAY: NO
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <typedef.h>
#include <proc_s.h>



static char _far file[FILENAME_MAX];

int hard_err_flag = 0;

int  _far _fortran set_file_name( char _far *name )
 {
  if( strlen( name ) >= FILENAME_MAX ) return -1;
  strcpy( file, name );
  return 0;
 }

char _far * _fortran Get_S_file_name( void ) { return file; }


static int       nOutTextFlag = 0;

void _far _fortran Set_OutTextFlag( int nFlag ) { nOutTextFlag = nFlag; }
int  _far _fortran Get_OutTextFlag( void ) { return nOutTextFlag; }

static int       nOutListFlag = 1;

void _far _fortran Set_OutListFlag( int nFlag ) { nOutListFlag = nFlag; }
int  _far _fortran Get_OutListFlag( void ) { return nOutListFlag; }
