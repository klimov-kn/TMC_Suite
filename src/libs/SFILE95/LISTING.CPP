/*
**  TAMIC_soft(R) 1990 v1.00
**
**  put_listing_string.
**
**  FILE : a:\tamic\listing.c
**
**  LIB  : tmc_hX.lib
**
**  OVERLAY: NO
**
*/

#include <stdio.h>
#include <stdarg.h>

#include <string.h>
#include <typedef.h>
#include <listing.h>

#define OPEN_LISTING "Can't open listing file %s!"


static struct listing_data lst_dat = { "", "", "", NULL, 0, 0, 80, 65, 0, 1 };
static char _far FopenBuff[BUFSIZ];


void _far  set_listing_title( char _far *title ){ strcpy( lst_dat.title, title ); }
void _far  set_listing_subtitle( char _far *title ){ strcpy( lst_dat.subtitle, title ); }
void _far  set_listing_page_size( int size ) { if( size > 10 ) lst_dat.page_size = size;}
char _far *get_listing_file_name( void ) { return( lst_dat.filename ); }

int far open_listing_file( int open_mode_flag )
 {
  char far *ptr;

  if( lst_dat.f ) return -1; /* file is opened */

                                                  /* make listing file name */
  strcpy( lst_dat.filename, (( ptr = strrchr( lst_dat.subtitle, '\\') )?(ptr + 1):lst_dat.subtitle) );

  if( ptr = strrchr( lst_dat.filename, '.') ) ;
   else *(ptr = strrchr( lst_dat.filename, '\0')) = '.';

  *++ptr = '\0';

  strcat( lst_dat.filename, "lst");

  if( (lst_dat.f = fopen( lst_dat.filename, (open_mode_flag)?"w":"a" ) ) == NULL )
   {
    printf( OPEN_LISTING, lst_dat.filename );
    return -1;
    }

    setbuf( lst_dat.f, FopenBuff );

  if( open_mode_flag )
   {
    lst_dat.line_num = 0;      // reset line number counter for new file
    lst_dat.page_num = 0;      // reset page number counter for new file
    }
  return 0;
 }




void far close_listing_file( void )
 {
  if( lst_dat.f )               /* is file open ?   */
   {
    fclose( lst_dat.f );

    lst_dat.f = NULL;
   }
 }



static char _BASE_CODE fmt1[] = "%3d  %s\n";
static char _BASE_CODE fmt2[] = "  %s\n";

int far put_listing_string( char far *string )
 {
  char       Buff[256];
  char _far *ptr;

  if( lst_dat.f == NULL) return( -1 ); /* is file opened */


  do { if( (ptr = strchr( string, '\n' ) ) != NULL ) *ptr = 0; // seach \n simbol

       put_list_title();   // test beginig of string and output title if o'k

       ++lst_dat.line_num;

       if( lst_dat.numeration_flag ) sprintf( Buff, fmt1, lst_dat.line_num, string );
	else sprintf( Buff, fmt2, string );
       fputs( Buff, lst_dat.f );

       if( ptr != NULL ) string = ++ptr; // increment ptr to next string

       }while( ptr != NULL );

  return( lst_dat.line_num );
  }


int far l_printf( char *format_string, ...)
 {
  char buff[255];
  int ret_code;
  va_list argptr;

  va_start( argptr, format_string);    /* init va_ function */

  vsprintf( buff, format_string, argptr);

  ret_code = put_listing_string( buff);

  va_end(argptr);  /* close va_ function */

  return( ret_code );
  }

#include <dos.h>

#define ALL_TIME_LONG 12
#define ALL_DATA_LONG 16
char _far *dos_date( char _far *string, struct dosdate_t _far *dd);
char _far *dos_time( char _far *string, struct dostime_t _far *dt);

void far put_list_title( void )
 {
/*  struct dostime_t dt;
  struct dosdate_t dd;
  char time[ALL_TIME_LONG];
  char date[ALL_DATA_LONG];

  if( (lst_dat.line_num % lst_dat.page_size) == 0 )
   {
    fprintf( lst_dat.f, "%c%c%c%c", 12, 8, 32, 13 );
    fprintf( lst_dat.f, "\n%-*sPage%3d", lst_dat.page_width - 8, lst_dat.subtitle, ++lst_dat.page_num);

    _dos_gettime( &dt );
    dos_time( time, &dt );
    *(strrchr( time, ':')) = 0;
    fprintf( lst_dat.f, "\n%*sTime %s", lst_dat.page_width - 14, "", time );

    _dos_getdate( &dd );
    dos_date( date, &dd );
    fprintf( lst_dat.f, "\n%*s%s", lst_dat.page_width - 16, "", date );
    fprintf( lst_dat.f, "\n%*s\n\n", lst_dat.page_width - 1, lst_dat.title );
    }
*/
  }


char _far *dos_date( char _far *string, struct dosdate_t _far *dd)
 {
/*  static char *month[] ={
			 "Jan","Feb","Mar","Apr","May","Jun",
			 "Jul","Aug","Sep","Oct","Nov","Des"
			  };
  static char *day[] ={
		       "Sun","Mon","Tue","Wed","Tur","Fri","Sat"
		       };

  sprintf( string, "%s %s %2d %4d", day[dd->dayofweek], month[dd->month - 1], dd->day, dd->year );
	*/
  return( string);
  }


char _far *dos_time( char _far *string, struct dostime_t _far *dt)
 {
  char *str_ptr;
/*
  sprintf( string, "%2d:%2d:%2d:%2d", dt->hour, dt->minute, dt->second, dt->hsecond );

  str_ptr = string;
  while( *string)
   {
    if( *string == ' ' ) *string = '0';
    string++;
    }
*/
  return( str_ptr);
  }

