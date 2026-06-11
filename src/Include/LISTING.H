/*
**  TAMIC_soft(R) 1989 v1.00
**
** Macros and declaration for listing output programs.
**
** FILE: \include\listing.h
**
*/

#ifndef _LISTING_H
  #define _LISTING_H



#define WR_BEG          1
#define WR_END          0


struct listing_data
          {
           char filename[FILENAME_MAX]; /* file name*/
           char title[256];             /* listing title*/
           char subtitle[256];          /* listing subtitle*/
           FILE *f;                     /* internal file pointer*/
           int line_num;                /* listing line number*/
           int page_num;                /* listing page number*/
           int page_width;              /* listing page width*/
           int page_size;               /* listing page size*/
           Uint numeration_flag;        /* listing numeration flag*/
           Uint echo_flag;              /* listing echo flag*/
           };

void _far set_listing_title( char _far *title );
void _far set_listing_subtitle( char _far *title );
int  _far open_listing_file( int open_mode_flag );
void _far close_listing_file( void );

void _far set_listing_page_size( int size );
void _far set_listing_numeration_flag( Uint size );

void _far set_listing_echo_flag( Uint value );
int  _far get_listing_echo_flag( void );

char _far *get_listing_file_name( void );

int  _far put_listing_string( char far *string );
void _far put_list_title( void );
int  _far l_printf( char _far *format_string, ...);


#endif

