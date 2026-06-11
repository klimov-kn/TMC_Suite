/*
**  TAMIC_soft(R) 1990,1991,1992 v2.00
**
** Macros and decloration for Tamic_W programm.
**
** FILE: ..\include\debug.h
*/

#ifndef _DEBUG_H
 #define _DEBUG_H

#define  DF_PRINT_BLOCK_TABLE           1               //Db
#define  DF_PRINT_INPUT_TABLE           2               //Dw
#define  DF_PRINT_LINK_NUMBER           4               //Dt
#define  DF_PRINT_MATRIX                8               //Dm

void _far _fortran SetDebugFlag( DWORD dwFlag );
int  _far _fortran GetDebugFlag( DWORD dwFlag );
#endif
