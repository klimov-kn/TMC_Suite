/*
 *  TAMIC_soft(R) 1992 vR1.02
 *
 *  get_arg_env - get command line arguments and  for utitlity
 *                reading scattering matrix files
 *
 *  FILE: ..\gtargenv.c
 *
 *  OVERLAY: NO
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <typedef.h>
#include <tmclimit.h>
#include <frqdata.h>
#include <s_file.h>
#include <proc_s.h>
#include <ys_proc.h>

static void _near print_error( void );

void _far _fortran get_arg_env( int argc, char _far *argv[], char _far *envp[] )
 {
  int i = 0;

  while(++i < argc)
   {
    if((*argv[i] == '-') || (*argv[i] == '/'))
     {
      switch(*(++argv[i]))
       {
        case 'T': switch( *(++argv[i]) )
                   {
                    case 'c': if( *(++argv[i]) != 0 ) print_error();
                              Set_Table_Mode( REAL_IMAG );
                              break;
                    case 'e': if( *(++argv[i]) != 0 ) print_error();
                              Set_Table_Mode( ARG_PHASE );
                              break;
                    case 'g': if( *(++argv[i]) != 0 ) print_error();
                              Set_Table_Mode( VSWR_LOSS );
                              break;
                    }
                  Set_OutTextFlag( 1 );
                  break;
        case 'L': switch( *(++argv[i]) )
                   {
                    case 'n': if( *(++argv[i]) != 0 ) print_error();
                              Set_OutListFlag( 0 );
                              break;
                    default: print_error();
                    }
                  break;
        default: print_error();
       } /* switch end */
     }
    else
      {
       if( i == (argc - 1) )
        {
         if(  set_file_name( argv[i] ) != 0 ) print_error();
        }
       else  print_error();
      }  /* if_else end  */
   }  /* while end       */

   if( strlen( Get_S_file_name() ) == 0 ) print_error();
 }  /* get_arg_env end   */



static void _near print_error( void )
 {
  int i;
  static char _far *message[]=
   { "\n\nPurpose: Read S-file\n reduce multimode S matrix to one-mode S matrix\n write S matrix to listing and write Y matrix to file for TMC_W.exe",
     "\n         \n",
     "\nUsage  : TMC_YSCT.EXE [ options ] FILE.S",
     "\nOptions : -T<c|e|g> output text file with S-matrix in the form:",
     "\n                   real - imaginary  < c > or",
     "\n                   argument - phase  < e > or",
     "\n                   VSWR - loss       < g >.",
     "\n          -Ln - don't output S-matrix to listing file.\n" };

  title_ys();

  for( i = 0; i < 8; i++ ) printf( message[i] );
  exit(-1);
  }




