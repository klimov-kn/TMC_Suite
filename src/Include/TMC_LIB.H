/*
**  TAMIC_soft(R) 1991 v1.00
**
** Macros and decloration for TAMIC_soft standart library.
**
** FILE: a:\include\tmc_lib.h
*/

#ifndef _TMC_LIB_H
 #define _TMC_LIB_H





  int       get_dos_freemem( void );
void *alloc_mem_err( long size );
int  free_mem( void *mem_ptr );
int  free_mem_null( void **ptr );
char *change_file_extension( char *new_name, const char *base_name, const char *extension );

int  get_current_driver( void );
int  set_default_drive( char drive_code );
int  get_current_dir( char *path, char drive );
int  check_drive( char driver_code );
int  get_full_path( char *path_buff, char drive );

char  * del_blanks( char *string );


/*  real function Angle( x1, x2, x3, y1, y2, y3 ) */
_real Angle( _real *x1, _real *x2, _real *x3,
			 _real *y1, _real *y2, _real *y3 );
void  OutTrace( char *TraceMessage);


#endif
