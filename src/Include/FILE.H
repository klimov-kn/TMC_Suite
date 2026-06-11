/*
**  TAMIC_soft(R) 1989,1990,1991 v1.00
**
** Macros and declarations for file_h program
**
** FILE: a\include\file.h
**
*/

#ifndef _FILE_H
  #define _FILE_H


#ifndef FILENAME_MAX
 #define FILENAME_MAX   63
#endif
#ifndef FILENAME
 #define FILENAME       13
#endif

#define PATH_TOO_LONG  95
#define BAD_DRIVE      15
#define BAD_PATH        3

#define FILE_FOUND      0
#define NO_FILE         2
#define OUT_OF_FILE    18
#define END_OF_FL    (-1)
#define HARD_ERROR   (-2)


#define FL_S          256
#define _SAVE           1
#define _LOAD           0


void _far  delete_files( char _far *file_mask );
void _far  delete_S_files( char _far *file_mask );
int  _far  copy_file( char _far *source, char _far *target );

int  _far  get_drive_number( void );
int  _far  get_current_driver( void );
int  _far  check_drive( char drive_code );
int  _far  set_default_drive( char drive_code );
int  _far  set_path( char _far *path );
int  _far  get_current_dir( char _far *path, char drive);
int  _far  get_full_path( char _far *path_buff, char drive);
int  _far  get_file_list( char _far *file_l[], char _far *file_mask, int fl_size, int _far *f_num );
int  _far  get_dir_list( char _far **dir_l, char _far *file_mask, int fl_size, int _far *f_num);
int  _far  get_drive_list( char _far **drive_list );
char _far *get_file_name( int x_pos, int y_pos, FILE_LIST _far *fl, int load_save_flag);
int  _far  check_path( char _far *path );
int  _far  check_file_name( char _far *name ); 

#endif

