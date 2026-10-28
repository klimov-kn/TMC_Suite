/*
 *  TAMIC_soft(R) 1992 vR1.02
 *
 *  processor - processing routine for adding S-files
 *
 *  FILE: ..\sadd.c
 *
 *  OVERLAY: NO
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include <typedef.h>
#include <tmclimit.h>
#include <complex1.h>
#include <tmc_lib.h>
#include <frqdata.h>
#include <s_file.h>
#include <Error1.h>	/* was <error.h>: error.h missing, error codes live in Error1.h */
#include <mainwndw.h>
#include <proc_s.h>

int  Compare_S_ind( S_INDEX *si1, int extrapol_flag1, S_INDEX *si2, int extrapol_flag2 );
int  Unite_two_S( FREQUENCY *frq_out1, INPUT_DATA *id1, _complex **sss1,
				FREQUENCY *frq_out2, INPUT_DATA *id2, _complex **sss2,
				FREQUENCY *frq_out,  INPUT_DATA **id, _complex ***sss );
void Free_S( INPUT_DATA *id, S_INDEX **si, _real *f,
                           _complex ***sss, int max, int extrapol_flag );
int Interpolate_S_matrix( char *szFile );


static char szTrace1[] = "Reading first matrix ...";
static char szTrace2[] = "Reading second matrix ...";
static char szTrace3[] = "Saving data at frequency %10.3g GHz.";
static char szTrace4[] = "Interpolating S-matrix ...";
static char szFin1[] = "S-matrices have been united normally.";
static char szFin2[] = "S-matrix have been read normally.";

int S_add( char *filename, char *filename1 )
  {
   int ret_code = 0, i;

   int extrapol_flag;
   char *szFile = filename;
   _complex   **sss;
   INPUT_DATA *id;
   S_INDEX    *si;
   FREQUENCY  frq_out;

   int extrapol_flag1;
   char *szFile1 = filename;
   _complex   **sss1;
   INPUT_DATA *id1;
   S_INDEX    *si1;
   FREQUENCY  frq_out1;

   int extrapol_flag2;
   char *szFile2 = filename1;
   _complex   **sss2;
   INPUT_DATA *id2;
   S_INDEX    *si2;
   FREQUENCY  frq_out2;

   /* reading first scattering matrix */
   ret_code = read_S_matrix( szFile1, &si1, &extrapol_flag1, &sss1,  &frq_out1, &id1 );

   if( szFile2 != NULL )
     {
      /* reading second scattering matrix */
      if( ret_code == 0 ) ret_code = read_S_matrix( szFile2, &si2, &extrapol_flag2, &sss2,  &frq_out2, &id2 );

      if( ret_code == 0 )
       {
        /* check is it possible to write both scattering matrices in one file */
        ret_code = Compare_S_ind( si1, extrapol_flag1, si2, extrapol_flag2 );

        /* unite both scattering matrix in one structure */
        if( ret_code == 0 ) ret_code = Unite_two_S( &frq_out1, id1, sss1,
                                                    &frq_out2, id2, sss2,
                                                    &frq_out, &id, &sss );
        if( ret_code == 0 )
         {
          extrapol_flag = extrapol_flag1;
          si = si1;

          /* rename first file if there is not third parameter in the string */
          if( strcmp( szFile1, szFile ) == 0 )
            {
            }

          /* write united matrix to the file */
          set_frq_nf( frq_out.max );
          for( i = 0; ( i < frq_out.max )&&( ret_code == 0 ); i++ )
            {
             set_current_frq_num( i );
             set_frq_element( frq_out.f[i], i );
             if( ret_code == 0 ) ret_code = save_S_matrix( sss[i], &id[i], szFile, si, extrapol_flag );	// A-3: id[i]
            }

          /* write united matrix to the listing */
          free_mem( frq_out.f );
          free_mem( id );
          free_mem( sss );
         }
        Free_S( id2, &si2, frq_out2.f, &sss2, frq_out2.max, extrapol_flag2 );
		Free_S( id1, &si1, frq_out1.f, &sss1, frq_out1.max, extrapol_flag1 );
       }
      else
       {
		Free_S( id1, &si1, frq_out1.f, &sss1, frq_out1.max, extrapol_flag1 );
       }
     }
   else
     {
      if( ret_code == 0 )
       {
		Free_S( id1, &si1, frq_out1.f, &sss1, frq_out1.max, extrapol_flag1 );
       }
     }

   return ret_code;
  }

void Free_S( INPUT_DATA *id, S_INDEX **si, _real *f,
                           _complex ***sss, int max, int extrapol_flag )
  {
   free_mem( id );
   free_mem( f );
   Free_S_read( sss, max );
   Free_GRID_Index( si, extrapol_flag );
  }

int Compare_S_ind( S_INDEX *si1, int extrapol_flag1, S_INDEX *si2, int extrapol_flag2 )
 {
  if( extrapol_flag1 != extrapol_flag2 )
    {
     return -1;
    }
  else
    {
     if( extrapol_flag1 != 0 ) return 0;
     else
       {
        if( ( si1->delta1 != si2->delta1 )||
            ( si1->delta2 != si2->delta2 )||
            ( si1->delta3 != si2->delta3 ) )
           {
            return -1;
           }
       }
    }
  return 0;
 }


