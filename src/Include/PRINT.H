/*
**  TAMIC_soft(R) 1989,1990,1991 v1.00
**
** Macros and decloration for Tamic_H graphics programm
**
** FILE: a:\include\print.h>
**
*/

#ifndef _PRINT_H
  #define _PRINT_H



struct print_color_tab
       {
        int num;                        /*  number of colors*/
        unsigned char color[1024];  /*  replace color table*/
        };

void _far set_eject_flag( int flag );
int  _far get_eject_flag( void );

void _far set_turne_flag( int flag );
int  _far get_turne_flag( void );

void _far set_repeet_num( int num );
int  _far get_repeet_num( void );

void _far set_print_density( int dens );
int  _far get_print_density( void );

void _far set_print_stop_handler( int (*handler)( void ) );
void _far set_prn_persent_handler( void (*handler)( Uint ) );

void _far set_printer_type( int type );
int  _far get_printer_type( void );

void _far set_left_margin( int margin );
int  _far get_left_margin( void );

void _far set_print_color_tab( struct print_color_tab _far *color_tab );
void _far set_print_file_name( char _far *name );
void _far set_print_window( int _left, int _top, int _right, int _bottom );
int  _far print_screen_window( void );


#endif

