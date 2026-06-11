/*
**  TAMIC_soft(R) 1991 v1.10.  12/01/89 04:10pm
**
** Macros and declarations for TMC program.
**
** FILE: tmc.h
*/

#ifndef _TMC_H
  #define _TMC_H


void _far          hardhandler( unsigned deverr, unsigned doserr, unsigned far *hdr );
void _far _fortran processor( void );
void _far _fortran exit_tmc( void );

int  _far          check_stop_p( void );
#endif
