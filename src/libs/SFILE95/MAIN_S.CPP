/*
*  TAMIC_soft(R) 1992 vR1.02
*
*  MAIN TAMIC ROUTINE FOR READING AND CONTROL S-FILES
*
*  FILE: ..\main_s.c
*
*/

#include <stdio.h>

#include <typedef.h>
#include <proc_s.h>

void _far main( int argc, char far *argv[], char far *envp[])
 {
  get_arg_env( argc, argv, envp ); // getting commannd line parms
  processor();                     // processor
 }



