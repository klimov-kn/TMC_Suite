/*
 *  TAMIC_soft(R) 1992 vR1.02
 *
 *  processor - processing routine for reading s-files
 *
 *  FILE: ..\sread.c
 *
 *  OVERLAY: NO
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>


#include <typedef.h>
#include <tmclimit.h>
#include <complex.h>
#include <tmc_lib.h>
#include <frqdata.h>
#include <s_file.h>
#include <error1.h>
#include <proc_s.h>

#define STR_NUM 22

int _far    _fortran write_S_to_listing( S_INDEX _far *si, int extrapol_flag, _complex _far **sss,  FREQUENCY _far *frq_out, INPUT_DATA _far *id );
int _far    _fortran put_S_text_table( _complex _far **lpxS, FREQUENCY _far *frq_out, INPUT_DATA _far *id );

static char szTrace1[] = "Reading S-matrix . . .";
static char szTrace2[] = "Writing S-text file . . .";
static char szTrace3[] = "Process terminated normally.";

int _far _fortran processor( void )
  {
   int ret_code = 0, i, j, ind, k;
   int extrapol_flag;
   char _far *filename = Get_S_file_name();
   _complex   _far **sss;
   INPUT_DATA _far *id;
   S_INDEX    _far *si;
   FREQUENCY  frq_out;
   int in1 = 1, in2 = 2, mod1 = 2, mod2 = 1;
   double *f;
   _complex *s;
   int nPoint;

   printf( TITLE_STR );
   printf( "\n" );
   printf( COPYRIGHT_TITLE );
   printf( "\n" );

   OutTrace( szTrace1 );
   ret_code = read_S_matrix( filename, &si, &extrapol_flag, &sss,  &frq_out, &id );

   if( ret_code == 0 ) ret_code = write_S_to_listing( si, extrapol_flag, sss,  &frq_out, id );

   OutTrace( szTrace2 );
   if(  ret_code == 0  ) ret_code = put_S_text_table( sss, &frq_out, id );
   if(  ret_code == 0  ) ret_code = cut_S_matrix( &sss, &frq_out, &id );
   if( ret_code == 0 ) ret_code = write_S_to_listing( si, extrapol_flag, sss,  &frq_out, id );

   if( ret_code == 0 )  OutTrace( szTrace3 );
   
   if( ret_code == 0 ) ret_code = read_S_matrix_element( &in1, &mod1, &in2, &mod2, filename, &s, &f, &nPoint);
   for( i = 0; i < nPoint; i++)
   {
	   printf("\nfreq[%d] = %lg; s[in1=%d;mod1=%d;in2=%d;mod2=%d]={%lg;%lg} %lg", i, f[i], in1, mod1, in2, mod2, s[i].x, s[i].y, cabsv(s[i]) );
   };
   getch();
   delete s;
   delete f;
   return ret_code;
  }

