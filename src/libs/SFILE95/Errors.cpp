/*
 *  TAMIC_soft(R) 1992 vR1.02
 *
 *  put_error_messege - dummy error processing routine.
 *
 *  FILE: ..\errors.c
 *
 *  OVERLAY: NO
*/


#include <stdio.h>
#include <stdarg.h>
#include <conio.h>

#include <typedef.h>
#include <error1.h>
#include "error2.h"


char msg1[] = "Fatal ERROR.\nNot enough memory to run program.\nRemove resident programs and restart system. ";
char msg2[] = "Error !\nCan not open listing output file:\n%s!";
char msg3[] = "Warning !\nNot enough memory for all frequency points!\nOnly %d frequencies processed!";
char msg4[] = "Error !\nCan't open S-matrix data file\n%s!";
char msg5[] = "Error while reading from S-file:\n%s!";
char msg6[] = "Error !\nThis is not scattering matrix file:\n%s!";
char msg7[] = "Error !\nToo old scattering matrix file:\n%s!";
char msg8[] = "Error !\nDifferent number of inputs! See listing.";
char msg9[] = "Error !\nDifferent number of modes! See listing.";
char msg10[] = "Error !\nCan't open file for text table\n%s!";

int  _far   put_error_messege( int err_num, ... )
  {
/*   va_list  argptr;                      // Argument list pointer
//   char _far *msg;
   char buff[512];

   va_start( argptr, err_num );       // Initialize va_ functions

   switch ( err_num )
     {
      case FATAL_MEM_ALLOC_ERROR:
                                 vsprintf( buff, msg1, argptr );
                              break;
      case ERROR_OPEN_LISTING:
                                 vsprintf( buff, msg2, argptr );
                              break;
      case SMT_FRQ_MEMORY:
                                 vsprintf( buff, msg3, argptr );
                              break;
      case ERROR_OPEN_S:
                                 vsprintf( buff, msg4, argptr );
                              break;
      case ERROR_READ_S:
                                 vsprintf( buff, msg5, argptr );
                              break;
      case SMT_LABEL_ERROR:
                                 vsprintf( buff, msg6, argptr );
                              break;
      case SMT_VERSION_ERROR:
                                 vsprintf( buff, msg7, argptr );
                              break;
      case DIFF_INPUT_NUM:
                                 vsprintf( buff, msg8, argptr );
                              break;
      case DIFF_MODE_NUM:
                                 vsprintf( buff, msg9, argptr );
                              break;
      case ERROR_OPEN_S_T:
                                 vsprintf( buff, msg10, argptr );
                              break;
      default:
  //                            printf("\nUnknown ERROR number %d!", err_num );
                              va_end( argptr );
                              return -1;
                              break;
     }
   va_end( argptr );                     // Close va_ functions

//   printf( buff );
//   printf("  Press any key . . ."); getch();
*/   return 0;
  }
