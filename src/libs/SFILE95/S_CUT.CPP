/*
 *  TAMIC_soft(R) 1990,1991,1992 vR1.02
 *
 *  cut_S_matrix - cut multi-mode scattering matrix to
 *                     one-mode scattering matrix
 *                     
 *
 *  Usage -  cut_S_matrix( sss, frq, id );
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
#include <conio.h>
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
#include <ys_proc.h>


int _far _fortran cut_S_matrix( _complex _far ***sss, FREQUENCY _far *frq, INPUT_DATA _far **id )
  {
   int i, j, i1, j1, kol, kol1, k, len, ii1, ii2, ii3, ii4;
   _complex _far *lpxS, *lpxS1;
   INPUT_DATA _far *id1;
   _complex _far **sss1;
   int nCutFlag;
   _real rSmin = get_rSmaxCut();
   char ch1[200];
 
   nCutFlag = 0;

   /* allocation of memory for input data structures */
   if( (id1 = (INPUT_DATA _far *)alloc_mem_err( (long)sizeof(INPUT_DATA)*(frq->max) ) ) == NULL )
   {
//            free_mem( *id );
//            free_mem( frq->f );
//            Free_S_read( sss, frq_out->max );
	   return -1;
   }

   /* allocation of memory for pointers at scattering matrices */
   if( ((sss1) = (_complex _far **)alloc_mem_err( (long)sizeof(void _far *)*(frq->max) ) ) == NULL )
   {
       free_mem( id1 );
	   return -1;
   }

   /* loop over frequencies */
   for( k = 0; k < frq->max; k++ )
     {
	  len = ((*id)[k].num)*((*id)[k].num);
      if( ( (sss1)[k] = (_complex _far *)alloc_mem_err( (long)sizeof(_complex)*len ) ) == NULL )
	  {
       free_mem( id1 );
       Free_S_read( &sss1, frq->max );
	   return -1;
	  }

      lpxS = (*sss)[k];
	  lpxS1 = sss1[k];
      /* first loop over inputs */
	  id1[k].num = (*id)[k].num;
	  id1[k].dim = (*id)[k].num;
      for( i = 0; i < (*id)[k].num; i++ )
        {
		 id1[k].array[i] = 1;
         /* first loop over modes */
         for( j = 0, kol1 = (*id)[k].array[i]; j < kol1; j++ )
           {
            /* second loop over inputs */
            for( i1 = 0; i1 < (*id)[k].num; i1++ )
              {
               /* second loop over modes */
               for( j1 = 0, kol = (*id)[k].array[i1]; j1 < kol; j1++ )
                 {
				   if( (j1==0)&&(j==0) )
				   {
					   *lpxS1 = *lpxS;
					   lpxS1++;
				   }
				   else
				   {
					if( (j == 0)&&(j1 != 0) )
					{
					    if( cabsv(*lpxS) >= rSmin )
						{
							rSmin = cabsv(*lpxS);
							ii1 = i+1; ii2 = j+1; ii3 = i1+1; ii4 = j1+1;
							nCutFlag = 1;
//							getch();
						};
					};
				   };
                  lpxS++;
                 }; //end of second loop over modes
              }; //end of second loop over inputs

           }; //end of first loop over modes
        }; //end of first loop over inputs

     } /* end frequency loop */





  free_mem( *id );
  Free_S_read( sss, frq->max );

  *id = id1;
  *sss = sss1;

  if( nCutFlag == 1 )
  {
	  sprintf( ch1, "Warning modul S[%d;%d;%d;%d]= %lg >= %lg.\nPress any key to continue.", ii1, ii2, ii3, ii4, rSmin, get_rSmaxCut() );
	  OutTrace( ch1 );
	  sprintf( ch1, "Warning modul S[%d;%d;%d;%d]= %lg >= %lg.", ii1, ii2, ii3, ii4, rSmin, get_rSmaxCut() );
	  set_title_ys( ch1 );
  };

  return 0;
  } // end of put_S_text_table routine



