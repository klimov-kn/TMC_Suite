/*
 *  TAMIC_soft(R) 2000 vR1.02
 *
 *  save_S_matrix_element - save one column in scattering matrix file
 *                     
 *
 *  Usage -  save_S_matrix_element( nColumn, szFileName, pcS, dFreq, nInput)
 *
 *  Arguments - nColumn - number of column at the scattering matrices,
 *              szFileName - s-matrix file name,
 *				pcS     - pointer at complex s-matrix column vector,
 *              dFreq   - frequence,
 *              nInput  - number of inputs.
 *
 *
 *  FILE : ..\sfile95\s_wemt.cpp
 *
 */

#include <windows.h>
#include <io.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <math.h>

#include <typedef.h>
#include <frqdata.h>
#include <complex1.h>
#include <tmc_lib.h>
#include <tmclimit.h>
#include <s_file.h>
#include <error1.h>
#include "error2.h"
#include <proc_s.h>
#include <ys_proc.h>


int _far _fortran save_S_matrix_element( int in1, char *filename, _complex *sss, double f1, int nInput)
  {
//
   S_INDEX    _far *si;
   S_INDEX    _far si1;
   FREQUENCY  frq;
   int extrapol_flag;
   _complex   _far **sss1;
   INPUT_DATA _far *id;
   INPUT_DATA _far id1;
   _complex *lpxS1;
   _complex *s;
//   double *f;
   int i, j, i1, j1, ii, k, kol1, kol;
   int ii1;
   BOOL bIsFreqInMatrix = FALSE;
   char ch[] = "Tamic Output Signal Viewer";
   char filename1[1000];
   int ret_code = 0;

   sprintf( filename1, "%s$", filename );

//   getch();
   sss1 = NULL;

   if( read_S_matrix( filename, &si, &extrapol_flag, &sss1,  &frq, &id ) != 0 )
   {
	   // can't read s-matrix file
	   // create new s-matrix file
	   set_frequency( f1*(1.e-9), 0.0, 1 );
       set_current_frq_num( 0 );
	   extrapol_flag = 0;
	   id1.num = nInput;
	   id1.dim = nInput;
	   for( i = 0; i < id1.num ; i++ ) id1.array[i] = 1;
	   si1.S_name = filename;
	   si1.tpl_name = ch;
	   si1.delta1 = 0.0;
	   si1.delta2 = 0.0;
	   si1.delta3 = 0.0;
       if( ( s = (_complex _far *)alloc_mem_err( sizeof(_complex)*id1.num*id1.num ) ) == NULL )
	   {
		   return -1;
	   };
	   for( i = 0, ii = 0; i < id1.num ; i++ ) 
	   {
		   for( j = 0; j < id1.num; j++ )
		   {
			   s[ii].x = FLT_MAX;
			   s[ii].y = FLT_MAX;
			   ii++;
		   };
	   };
	   for( i = 0, ii = 0; i < id1.num ; i++ ) 
	   {
		   for( j = 0; j < id1.num; j++ )
		   {
			   if( (i == 0)&&(j == (in1-1)) )
			   {
					for( ii1 = 0; ii1 < nInput ; ii1++ )
					{
						s[ii+ii1] = sss[ii1];
					};
			   };
			   ii++;
		   };
	   };

       i = save_S_matrix( s, &id1, filename, &si1, extrapol_flag );
	   
	   free_mem( s );

	   return i;
   }
   else
   {
		for( k = 0; k < frq.max; k++ )
		{
		  lpxS1 = sss1[k];
		   // first loop over inputs 
	      for( i = 0, ii = 0; i < id[k].num; i++ )
	        {
	         // first loop over modes 
	         for( j = 0, kol1 = id[k].array[i]; j < kol1; j++ )
	           {
	            // second loop over inputs 
	            for( i1 = 0; i1 < id[k].num; i1++ )
	              {
	               // second loop over modes 
	               for( j1 = 0, kol = id[k].array[i1]; j1 < kol; j1++ )
	                 {
					   if( (j1 == 0)&&(i1 == (in1-1))&&(j == 0)&&(i == 0) )
					   {
							if( fabs( ( frq.f[k] - f1 )/f1 ) < (1.e-15) )
							{
								bIsFreqInMatrix = TRUE;
								for( ii1 = 0; ii1 < nInput ; ii1++ )
								{
									lpxS1[ii+ii1*kol] = sss[ii1];
								};
							};
					   };
	                  ii++;
	                 }; //end of second loop over modes
	              }; //end of second loop over inputs
	           }; //end of first loop over modes
	        }; //end of first loop over inputs
		};
		if( !bIsFreqInMatrix )
		{
		   set_frequency( f1*(1.e-9), 0.0, 1 );
	       set_current_frq_num( 0 );
		   extrapol_flag = 0;
		   id1.num = nInput;
		   id1.dim = nInput;
		   for( i = 0; i < id1.num ; i++ ) id1.array[i] = 1;
		   si1.S_name = filename;
		   si1.tpl_name = ch;
		   si1.delta1 = 0.0;
		   si1.delta2 = 0.0;
		   si1.delta3 = 0.0;
	       if( ( s = (_complex _far *)alloc_mem_err( sizeof(_complex)*id1.num*id1.num ) ) == NULL )
		   {
			  Free_GRID_Index( &si, extrapol_flag );
			  free_mem( frq.f );
			  free_mem( id );
			  Free_S_read( &sss1, frq.max );
			  return -1;
		   };
	   for( i = 0, ii = 0; i < id1.num ; i++ ) 
	   {
		   for( j = 0; j < id1.num; j++ )
		   {
			   s[ii].x = FLT_MAX;
			   s[ii].y = FLT_MAX;
			   ii++;
		   };
	   };
	   for( i = 0, ii = 0; i < id1.num ; i++ ) 
	   {
		   for( j = 0; j < id1.num; j++ )
		   {
			   if( (i == 0)&&(j == (in1-1)) )
			   {
					for( ii1 = 0; ii1 < nInput ; ii1++ )
					{
						s[ii+ii1] = sss[ii1];
					};
			   };
			   ii++;
		   };
	   };

	       i = save_S_matrix( s, &id1, filename1, &si1, extrapol_flag );
		   if( i != 0 )
		   {
			  Free_GRID_Index( &si, extrapol_flag );
			  free_mem( frq.f );
			  free_mem( id );
			  Free_S_read( &sss1, frq.max );
			  free_mem( s );
			  return -1;
		   }
		   free_mem( s );
		   ret_code = S_add( filename, filename1 );
		   remove( filename1 );
		}
		else
		{
          // write united matrix to the file 
          set_frq_nf( frq.max );
          for( i = 0; ( i < frq.max )&&( ret_code == 0 ); i++ )
            {
             set_current_frq_num( i );
             set_frq_element( frq.f[i], i );
             if( ret_code == 0 ) ret_code = save_S_matrix( sss1[i], id, filename, si, extrapol_flag );
            }
		};
	};

//  getch();
  Free_GRID_Index( &si, extrapol_flag );
  free_mem( frq.f );
  free_mem( id );
  Free_S_read( &sss1, frq.max );
  
  return ret_code;
  } 



