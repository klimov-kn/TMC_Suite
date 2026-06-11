/*
**  TAMIC_soft(R) 1989,1990,1991 v1.00
**
** Macros and declarations for file_h program
**
** FILE: a\include\file.h
**
*/

#ifndef __FILE_H
  #define __FILE_H


#ifndef FILENAME_MAX
 #define FILENAME_MAX   63
#endif

#define FILENAME       13

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


struct menu_window_size
	{
	 int h1, h2;
	 };

struct scale_parm
	  {
	   int fill;
	   int color;
	   float dy;
	   int height;
	   };

struct scale_parm_h
	  {
	   int fill;
	   int color;
	   float dy;
	   int height;
	   };


void _far delete_files( char _far *file_mask );
void _far delete_S_files( char _far *file_mask );
int  _far copy_file( char _far *source, char _far *target );

void _far init_all_window( int x_pos, int y_pos,   WINDOW _far *l0_wndw,
                           WINDOW _far *l1_wndw, WINDOW _far *l2_wndw,
                           WINDOW _far *l3_wndw, WINDOW _far *l4_wndw,
                           WINDOW _far *l5_wndw);
void _far drow_load_wn( WINDOW _far *l0_wndw, WINDOW _far *l1_wndw,
                        WINDOW _far *l4_wndw, WINDOW _far *l5_wndw,
                        char _far **dn_mess);
int  _far get_drive_number( void );
int  _far get_current_driver( void );
int  _far check_drive( char drive_code );
int  _far set_default_drive( char drive_code );
int  _far set_path( char _far *path );
int  _far get_current_dir( char _far *path, char drive);
int  _far get_full_path( char _far *path_buff, char drive);
int  _far get_file_list( char _far *file_l[], char _far *file_mask, int fl_size, int _far *f_num );
int  _far get_dir_list( char _far **dir_l, char _far *file_mask, int fl_size, int _far *f_num);
int  _far get_drive_list( char _far **drive_list );
void _far drow_vert_scale( WINDOW _far *l5, int offset );
void _far clr_dir_list( WINDOW _far *l3_wndw );
int  _far drow_dir_list( WINDOW _far *l3_wndw, char _far **path_l,
                         int pth_size, int first_item, int curr_item, int prev,
                         WINDOW _far *l5);
void _far drow_hor_scale( WINDOW _far *l4, int offset );
void _far clr_file_list( WINDOW _far *l2_wndw );
int  _far drow_file_list( WINDOW _far *l2_wndw, char _far **fl_l,
                          int fl_size, int first_item, int curr_item, int prev,
                          WINDOW _far *l4_wndw);
int  _far dir_menu( ACTIVE_CHAR _far *ach1, WINDOW _far *l3_wndw, WINDOW _far *l5_wndw,
                    char _far **dir_l, int pth_size, int _far *first_item,
                    int _far *current, int _far *ch, char _far *path_buff,
                    WINDOW _far *l1_wndw, int num_i,
                    int _far *prev_f, int _far *curr_f, int _far *enter_flag,
                    int x, int y);
int  _far file_menu( ACTIVE_CHAR _far *ach1, WINDOW _far *l2_wndw, WINDOW _far *l4_wndw,
                     char _far **fl_l, int fl_size, int _far *first_item,
                     int _far *current, int _far *ch, char _far *name_buff,
                     WINDOW _far *l1_wndw, int num_i,
                     int _far *prev_f, int _far *curr_f, int _far *enter_flag,
                     int x, int y);
char _far *get_file_name( int x_pos, int y_pos, FILE_LIST _far *fl, int load_save_flag);
void _far clr_file_list( WINDOW _far *l2_wndw );
int  _far drow_file_list( WINDOW _far *l2_wndw, char _far **fl_l,
                          int fl_size, int first_item, int curr_item, int prev,
                          WINDOW _far *l4_wndw);
int  _far check_path( char _far *path );
int  _far check_file_name( char _far *name ); 


#endif

