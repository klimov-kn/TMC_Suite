/*
 *  TAMIC_soft(R) 1990,1991,1992 vR1.02
 *
 *  put_S_text_table - print out multimode scattering matrix to
 *                     text file for usage in other CAD systems
 *                     or graph plotting.
 *  Set_Table_Mode  - set S-matrix output form.
 *
 *  Usage -  put_S_text_table( sss, frq, id );
 *
 *  Arguments - sss - pointer at the opinters at the scattering matrices,
 *              frq - pointer at FREQUENCY structure,
 *               id - pointer at the INPUT_DATA structures for all frq.
 *
 *  Remark - scattering matrix is assumed to be stored
 *           rowwise in general storage mode
 *
 *  Reqd. routines - cabsv, cphase ( in cmplx library )
 *
 *  FILE : ..\utility\s_file\stxtable.c
 *
 */

#include <io.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <typedef.h>
#include <frqdata.h>
#include <complex.h>
#include <tmc_lib.h>
#include <tmclimit.h>
#include <s_file.h>
#include <error1.h>
#include "error2.h"
#include <proc_s.h>

static int _far _fortran  check_INPUT_DATA( INPUT_DATA _far *id, int nFrqNum );
static void _far _fortran  write_ID_to_text_file( INPUT_DATA _far *id );

static int nTableMode = ARG_PHASE;
void _far _fortran Set_Table_Mode( int nMode ) { nTableMode = nMode; }

#define ONE       1.0f
#define ZERO      0.0f
#define GEN_THR   1.000001f

static char _far *szHood[] = { "! S - matrix from the file <%s>.",
                               "\n! First column - frequency in Hz.",
                               "\n! Next columns - element i1/m1/i2/m2 of the S - matrix,",
                               "\n!                i - input number, m - mode number." };
static char  _far *szForm[] = { "\n! Elements in the form real-imaginary.\n",
                                "\n! Elements in the form argument-phase.\n",
                                "\n! VSWR for reflection and LOSS (dB) for transmission.\n" };

static char  fmt0[] = "! Frequency, Hz";
static char  fmtf[] = " %#13.6g ";
static char  fmt1[] = " %#15.8g ";
static char  fmt2[] = "           %2d/%2d/%2d/%2d           ";
static char  fmt3[] = " %#15.8g %#15.8g ";
static char  fmt4[] = "   %2d/%2d/%2d/%2d   ";

FILE _far *fd = NULL;

int _far _fortran put_S_text_table( _complex _far **sss, FREQUENCY _far *frq, INPUT_DATA _far *id )
  {
   int i, i1, j, j1, kol, kol1, k;
   _real f11, r11, rkstu, rl;
   _complex _far *lpxS;

   char _far *szS_filename = Get_S_file_name();
   char       szFile[FILENAME_MAX];
//   char _far *lpcPtr;
   char       szFile1[FILENAME_MAX], buff[BUFSIZ];


   /* is it necessary to output table */
   if( Get_OutTextFlag() == 0  ) return 0;

   /* check possibility of table writing */
   if( check_INPUT_DATA( id, frq->max ) != 0 ) return -1;

   /* make data file name */
   change_file_extension( szFile, szS_filename, "dat" );

  if( access( szFile, 0 ) == 0 )
   {
    change_file_extension( szFile1, szFile, "~dt" );
    if( access( szFile1, 0 )  == 0 ) remove( szFile1 );
    rename( szFile, szFile1 );
   }

  if( (fd = fopen( szFile, "w" ) ) == NULL )
    {
     put_error_messege( ERROR_OPEN_S_T, szFile );
     return -1;
    }

   if( nTableMode != VSWR_LOSS )
     {
      sprintf( buff, " FILE_TXT %d;", nTableMode );
      fputs( buff, fd );

      fputs( "\n", fd );
      sprintf( buff, " FRQ_NUM %d;", frq->max );
      fputs( buff, fd );

      write_ID_to_text_file( id );
     }

   fputs( "\n", fd );
   sprintf( buff, szHood[0], szS_filename );
   fputs( buff, fd );
   for( i = 1; i < 4; i++ ) fputs( szHood[i], fd );  /* Red Riding Hood */
   fputs( szForm[ nTableMode ], fd );
   fputs( "\n", fd );
   fputs( fmt0, fd );

   /* write table header */

   /* first loop over inputs */
   for( i = 1; i <= id[0].num; i++ )
     {
      /* first loop over modes */
      for( j = 1, kol1 = id[0].array[i-1]; j <= kol1; j++ )
        {
         /* second loop over inputs */
         for( i1 = 1; i1 <= id[0].num; i1++ )
           {
            /* second loop over modes */
	    for( j1 = 1, kol = id[0].array[i1-1]; j1 <= kol; j1++ )
              {
               if( nTableMode == VSWR_LOSS ) sprintf( buff, fmt4, i, j, i1, j1 );
               else                          sprintf( buff, fmt2, i, j, i1, j1 );
               fputs( buff, fd );
              } //end of second loop over modes
           } //end of second loop over inputs

        } //end of first loop over modes
     } //end of first loop over inputs

   fputs( "\n", fd );


   /* loop over frequencies */
   for( k = 0; k < frq->max; k++ )
     {
      set_frq_element( frq->f[k], k );
      set_current_frq_num( k );

      sprintf( buff, fmtf, frq->f[k] );
      fputs( buff, fd );

      lpxS = sss[k];

      /* first loop over inputs */
      for( i = 0; i < id[k].num; i++ )
        {
         /* first loop over modes */
         for( j = 0, kol1 = id[k].array[i]; j < kol1; j++ )
           {
            /* second loop over inputs */
            for( i1 = 0; i1 < id[k].num; i1++ )
              {
               /* second loop over modes */
               for( j1 = 0, kol = id[k].array[i1]; j1 < kol; j1++ )
                 {
                  switch ( nTableMode )
                    {
                     case VSWR_LOSS:
                                    r11 = cabsv( (*lpxS) );

				    if( (i == i1) && (j == j1) )
				     {
                                      /* reflection case */
                                      if( ( r11 >= ONE )&&( r11 < GEN_THR ) )  r11 = (_real)(ONE - REAL_EPSILON);
                                      if( r11 < ONE )
                                        {
                                         rkstu = (ONE + r11)/(ONE - r11);
                                         sprintf( buff, fmt1, rkstu );
                                         fputs( buff, fd );
                                        }
                                       else
                                        {
                                         rl = (_real)(20.0*log10( (double)ONE/r11 ));
                                         sprintf( buff, fmt1, rl );
                                         fputs( buff, fd );
                                        }
                                     }
                                    else
                                     {
                                      /* propagation case */
                                      if( ( r11 > ONE )&&( r11 < GEN_THR ) ) r11 = ONE;
                                      if( r11 == ZERO ) rl = REAL_MAX;
                                      else  rl = (_real)(20.0*log10( (double)ONE/r11 ));
                                      sprintf( buff, fmt1, rl );
                                      fputs( buff, fd );
                                     }
                                    break;
                     case ARG_PHASE:
                                    r11 = cabsv( (*lpxS) );
                                    f11 = cphase( (*lpxS) );
                                    sprintf( buff, fmt3, r11, f11 );
                                    fputs( buff, fd );
                                    break;
                     case REAL_IMAG:
                                    sprintf( buff, fmt3, lpxS->x, lpxS->y );
                                    fputs( buff, fd );
                                    break;
                    }
                  lpxS++;
                 } //end of second loop over modes
              } //end of second loop over inputs

           } //end of first loop over modes
        } //end of first loop over inputs

      fputs( "\n", fd );
     } /* end frequency loop */
   fclose( fd );
   return 0;
  } // end of put_S_text_table routine



static int _far _fortran  check_INPUT_DATA( INPUT_DATA _far *id, int nFrqNum )
  {
   int k, i;
   int array[ MAX_INPUT_NUM ], num;

   /* initialization of number of inputs and number of modes */
   num = id[0].num;
   for( i = 0; i < num; i++ ) array[i] = id[0].array[i];

   for( k = 1; k < nFrqNum; k++ )
     {
      if( id[k].num != num )
       {
        put_error_messege( DIFF_INPUT_NUM );
        return -1;
       }
      for( i = 0; i < num; i++ )
       {
        if( array[i] != id[k].array[i] )
         {
          put_error_messege( DIFF_MODE_NUM );
          return -1;
         }
       }
     }
   return 0;
  }


static void _far _fortran  write_ID_to_text_file( INPUT_DATA _far *id )
  {
   int i;
   char buff[50];

   fputs( "\n", fd );
   sprintf( buff, " INPUT_NUM %d;", id->num );
   fputs( buff, fd );
   fputs( "\n", fd );

   sprintf( buff, " MODE_NUM %d;", id->num );
   fputs( " MODE_NUM ", fd );

   for( i = 0; i < id->num; i++ )
     {
      sprintf( buff, " %d;", id->array[i] );
      fputs( buff, fd );
     }
   fputs( "\n", fd );

  } /* end of write_ID_to_text_file routine */
