/*
**  TAMIC_soft(R) 1990,1991,1992,1993 v1.00
**
** Macros and declaration for y_file programs
**
** FILE: ..\include\y_file.h
**
*/

#ifndef _Y_FILE_H
  #define _Y_FILE_H

#define WR_Y          1
#define RD_Y          0

#define Y_EXT        "Y  "
#define Y_VER         1


struct _Y_index
        {
         char tpl_name[FILENAME_MAX];
         char Y_name[FILENAME_MAX];
         FILE _far *yf;
         float delta;
         };
typedef struct _Y_index Y_index;


Y_index _far *get_Y_index( void );
void    _far  set_Y_base_file_name( char _far *name );

int     _far  _open_Y_file( int open_mode_flag );
void    _far  close_Y_file( void );

size_t  _far  read_Y( void _far *outbuff, size_t size );
size_t  _far  write_Y( void _far *outbuff, size_t size );
int     _far  seek_Y( long offset, int flag );

#endif
