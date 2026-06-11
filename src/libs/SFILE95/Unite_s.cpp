/*
 *  TAMIC_soft(R) 1992 vR1.02
 *
 *  Unite_two_S - unite two scattering matrices in the new structure
 *
 *  FILE: ..\unite_s.c
 *
 *  OVERLAY: NO
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <typedef.h>
#include <tmclimit.h>
#include <complex1.h>
#include <tmc_lib.h>
#include <frqdata.h>
#include <listing.h>
#include <s_file.h>
#include <proc_s.h>

typedef struct
        {
         _real frq;
         INPUT_DATA id;
	 _complex   _far *sss;
         }FOR_SORT;

int cmpgle( const void *fs1, const void *fs2 );

int  _far _fortran Unite_two_S( FREQUENCY _far *frq_out1, INPUT_DATA _far *id1, _complex _far **sss1,
				FREQUENCY _far *frq_out2, INPUT_DATA _far *id2, _complex _far **sss2,
				FREQUENCY _far *frq_out,  INPUT_DATA _far **id, _complex _far ***sss )
  {
   int i, j;
   int nFr1 = 0, nFr2 = 0;
   FOR_SORT _far *FSRT;

   /* find equal frequencies */
   for( i = 0; i < frq_out1->max; i++ )
     {
      for( j = 0; j < frq_out2->max; j++ )
        {
         if( frq_out1->f[i] == frq_out2->f[j] )
           {
            frq_out1->f[i] = (_real)0.0;
            break;
           }
        } //end of second frequency loop
     } //end first frequency loop

   /* count unique frequencies in the first structure */
   for( i = 0; i < frq_out1->max; i++ )
     {
      if( frq_out1->f[i] != (_real)0.0 ) nFr1++;
     }

   /* count unique frequencies in the second structure */
   for( i = 0; i < frq_out2->max; i++ )
     {
      if( frq_out2->f[i] != (_real)0.0 ) nFr2++;
     }
   frq_out->max = nFr1 + nFr2;
   if( frq_out->max >= FREQ_MAX_NUM )
     {
      return -1;
     }

  /* allocation memory for sorting according to frequency */
  if( (FSRT = (FOR_SORT _far *)alloc_mem_err( (long)sizeof(FOR_SORT)*(frq_out->max) ) ) == NULL)
    return -1;

  /* copy data for unique frequencies to struct to be sorted */
   for( i = 0, j = 0; i < frq_out1->max; i++ )
     {
      if( frq_out1->f[i] != (_real)0.0 )
        {
	 FSRT[j].frq = frq_out1->f[i];
	 FSRT[j].id  = id1[i];
	 FSRT[j].sss = sss1[i];
         j++;
        }
     }

   for( i = 0, j = nFr1; i < frq_out2->max; i++ )
     {
      if( frq_out2->f[i] != (_real)0.0 )
        {
	 FSRT[j].frq = frq_out2->f[i];
	 FSRT[j].id  = id2[i];
	 FSRT[j].sss = sss2[i];
         j++;
        }
     }

  /* sorting united frequencies points according to frequencies values */
  qsort( (void *)FSRT, (size_t)frq_out->max, sizeof( FOR_SORT ), &cmpgle );

   /* allocation of memory for frq buff */
   if( (frq_out->f = (_real _far *)alloc_mem_err( (long)sizeof(_real)*(frq_out->max) ) ) != NULL)
    {
     /* allocation of memory for input data structures */
     if( (*id = (INPUT_DATA _far *)alloc_mem_err( (long)sizeof(INPUT_DATA)*(frq_out->max) ) ) != NULL )
      {
       /* allocation of memory for pointers at scattering matrices */
       if( ((*sss) = (_complex _far **)alloc_mem_err( (long)sizeof(void _far *)*(frq_out->max) ) ) == NULL )
         {
          free_mem( *id );
          free_mem( frq_out->f );
          free_mem( FSRT );
          return -1;
         }
      } /* if id end */
     else
      {
       free_mem( frq_out->f );
       free_mem( FSRT );
       return -1;
      }
    }
   else
    {
     free_mem( FSRT );
     return -1;
    }

   /* copy data to united arrays */
   for( i = 0; i < frq_out->max; i++ )
    {
     frq_out->f[i] = FSRT[i].frq;
     (*id)[i]      = FSRT[i].id;
     (*sss)[i]     = FSRT[i].sss;
    }

    /* copy current frq unit to read struct */
   for( i = 0; i < 4; i++ )
    {
     frq_out->unit[i] =  get_frq_unit(i);
     frq_out->item[i] =  get_frq_item(i);
     }
   frq_out->code = get_frq_item_code();

   free_mem( FSRT );
   return 0;
  } //end of Unite_two_S routine


int cmpgle( const void *fs1, const void *fs2 )
  {
	FOR_SORT *FSRT1 = (FOR_SORT *)fs1;
	FOR_SORT *FSRT2 = (FOR_SORT *)fs2;
   if( FSRT1->frq > FSRT2->frq )       return 1;
   else if( FSRT1->frq < FSRT2->frq )  return -1;
   else                                  return 0;
  }
