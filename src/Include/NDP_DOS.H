/*
**  TAMIC_soft(R) 1989,1992 v1.00
**
**  DOS TIME: functions decloration.
**
**  FILE: a:\include\dos_time.h
**
*/

#ifndef _NDP_DOS_H
  #define _NDP_DOS_H


/* File attribute constants */

#define _A_NORMAL	0x00	/* Normal file - No read/write restrictions */
#define _A_RDONLY	0x01	/* Read only file */
#define _A_HIDDEN	0x02	/* Hidden file */
#define _A_SYSTEM	0x04	/* System file */
#define _A_VOLID	0x08	/* Volume ID file */
#define _A_SUBDIR	0x10	/* Subdirectory */
#define _A_ARCH 	0x20	/* Archive file */



void     _dos_getdrive( unsigned *drv );
unsigned _dos_findfirst( const char *lpszFileName, unsigned dwAttrib, struct find_t *FInfo );
unsigned _dos_findnext( struct find_t *FInfo );

unsigned _convert_pma_to_mda( void *lpAdress, DWORD dwSize );

void     _dos_setdefdrive( DWORD dwDriveCode );
int      _dos_get_current_dir( DWORD dwDriveCode, char *lpszPath );

void     t_gotoxy( int x, int y);


#endif
