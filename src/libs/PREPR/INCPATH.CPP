/*
**
**   SetIncludePathName - set path to include files.
**   GetIncludePathName - return poiter at path to include files.
**
**  FILE : \tamic\indan\prepr\incpath.c
**
**  LIB  : rindXX.lib
**
**  OVERLAY: NO
**
*/

#include <string.h>
#include <stdio.h>

#include <typedef.h>
#include <indan.h>

char inc_path[FILENAME_MAX] = "";

void ortran SetIncludePathName( char _far *path ) { if( path != NULL ) strcpy( inc_path, path ); }
char _far * _fortran GetIncludePathName( void ) { return( inc_path ); }
