/*
**  TAMIC_soft(R) 1989,1990,1991,1992 v1.00 - 2.10
**
** Macros and declarations for init programs.
**
** FILE: init.h
**
*/

#ifndef _INIT_H
  #define _INIT_H


void _far _fortran ModelDepStartSettings( void );
int  _far _fortran ModelDepCheckArg( char *argv );
void _far _fortran ModelDepEndSettings( void );
void _far _fortran get_arg_env( int argc, char *argv[], char *envp[]);
void _far _fortran init( void );

void _far _fortran init_dir_list( void );
void _far _fortran init_graph_curve_list( void );
void _far          init_graph( void );
void _far          init_mouse( void );
void _far _fortran init_color_table( void );
void _far _fortran init_size_table( void );
void _far _fortran title( void );
void _far _fortran init_t_hs( void );
void _far _fortran init_prn( void );         /* init print window and print parameters*/

#endif
