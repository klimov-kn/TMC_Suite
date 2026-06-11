/*
**  TAMIC_soft(R) 1991 v1.00
**
** Macros and decloration for TAMIC_soft standart library.
**
** FILE: a:\include\tmc_lib.h
*/

#ifndef _TMC_LIB_H
 #define _TMC_LIB_H


#define MEM_STACK_SIZE   10
#define SIGN_TOLERANCE   1.0e-7

struct memory_block
            {
             int size;
	     void huge *ptr;
             };

struct memory_stack
            {
             struct memory_block mem_blk[MEM_STACK_SIZE];
             struct memory_block far *sp;
             };


#ifdef _NDP2
  int       get_dos_freemem( void );
  int       _System_( char *lpszCommand );
#else
  WORD  far get_dos_freemem( void);
#endif

void far *_alloc_mem( long size );
void far *alloc_mem( long size );
void far *alloc_mem_err( long size );
void far *alloc_mem_stack( long size );

int  far _free_mem( void far *mem_ptr );
int  far free_mem( void far *mem_ptr );
int  far free_mem_null( void far **ptr );
int  far free_mem_stack( void far *ptr );

void far init_memory_stack( void );
WORD far size_to_paragr( long  size );


char _far *make_file_name( FILE_LIST _far *fl, char _far *name);
char _far *make_full_file_name( char _far *full_name, const char _far *path, const char _far *name );
char _far *change_file_extension( char _far *new_name, const char _far *base_name, const char _far *extension );
void _far SplitFileName( FILE_LIST _far *fl, char _far *filename);

int  _far get_current_driver( void );
int  _far set_default_drive( char drive_code );
int  _far get_current_dir( char far *path, char drive );
int  _far check_drive( char driver_code );
int  _far get_full_path( char far *path_buff, char drive );

        /* Option File Name functions */
void _far set_option_file_name( const char _far *name );
void _far put_option( void );

        /* tolerance functions  */
void  _far _fortran set_tolerance( _real toler );
_real _far _fortran get_tolerance( void );
int   _far _fortran is_a_EQ_b( _real a, _real b );
int   _far _fortran Compare_A_and_B( _real a, _real b, _real tol );
_real _far _fortran round_to_tolerance( _real a );

long  _far fortran real_to_long( _real a );
int   _far fortran Fftoi( _real a );

char  _far * fortran del_blanks( char _far *string );


/*  real function Angle( x1, x2, x3, y1, y2, y3 ) */
_real _far fortran Angle( _real _far *x1, _real _far *x2, _real _far *x3,
			 _real _far *y1, _real _far *y2, _real _far *y3 );
/*  subroutine Newpt(x1,y1,x2,y2,fi1,rlong,x3,y3 ) */
void  _far fortran Newpt( _real _far *x1, _real _far *y1, _real _far *x2, _real _far *y2,
			  _real _far *fi, _real _far *rlong, _real _far *x3, _real _far *y3 );
_real _far fortran Distance( _real _far *x1, _real _far *y1, _real _far *x2, _real _far *y2 );
int   _far _fortran  quadrt( _real _far *a, _real _far *root1, _real _far *root2, _real tol );

int   _far pause_wait( void);
void  _far DELAY( unsigned int );
void  _far OutTrace( char _far *TraceMessage);
void  _far fortran PutPersent( char _far *lpszTitle, DWORD _far *lpdwCurrent,
                                                     DWORD _far *lpdwAll );

#ifdef _NDP2

char _far *strupr( char _far *lpszString );
int        rmtmp( void );

int        _bios_get_drive_number( void );

#endif

#endif
