/*
 *  TAMIC_soft(R) 1992 vR1.02
 *
 *  write_S_to_listing - write scattering matrices to listing
 *
 *  FILE: ..\wrtslst.c
 *
 *  OVERLAY: NO
*/


#include <stdio.h>
#include <string.h>


#include <typedef.h>
#include <tmclimit.h>
#include <complex.h>
#include <tmc_lib.h>
#include <frqdata.h>
#include <listing.h>
#include <s_file.h>
#include <proc_s.h>

static void _far _fortran  write_GRID( S_INDEX _far *si, int extrapol_flag );
static void _far _fortran  write_INPUT_DATA( INPUT_DATA _far *id );

static char szTrace[] = "Writing data at frequency %10.3g GHz.";

int _far    _fortran write_S_to_listing( S_INDEX _far *si, int extrapol_flag, _complex _far **sss,  FREQUENCY _far *frq_out, INPUT_DATA _far *id )
  {
   int k;
//   int i;
   char _far *filename = Get_S_file_name();
   char buff[50];

   /* is it necessery to output S-matrix to listing */
   if( Get_OutListFlag() == 0 ) return 0;

   set_listing_subtitle( filename );
   open_listing_file( WR_BEG );
   l_printf( LIST_TITLE );          //printing out title
   l_printf( "\n\n" );
   close_listing_file();

   write_GRID( si, extrapol_flag );


   for( k = 0; k < frq_out->max; k++ )
     {
      set_frq_element( frq_out->f[k], k );
      set_current_frq_num( k );

      sprintf( buff, szTrace, frq_out->f[k] );
      OutTrace( buff );

      write_INPUT_DATA( &id[k] );
      put_S_table( sss[k], id[k].num, id[k].array );
     }

   return 0;
  } // end of write_S_to_listing routine


char szTitleG[] = "\n      **** GRID PARAMETERS ****";
char szMesh[] = "        Mesh size along %c axis %14.7g m.";

static void _far _fortran  write_GRID( S_INDEX _far *si, int extrapol_flag )
  {
   int i;

   open_listing_file( WR_END );
   l_printf( szTitleG );
   l_printf("       Total number of grids: %d.", extrapol_flag+1 );
   for( i = 0; i <= extrapol_flag; i++ )
     {
      l_printf("       Grid number %d parameters:", i+1 );
      l_printf("        Input data file: %s.", si[i].tpl_name );
      if( extrapol_flag > 0 )
         l_printf("        Scattering matrix file: %s.", si[i].S_name );
      l_printf( szMesh, 'X', si[i].delta1 );
      l_printf( szMesh, 'Y', si[i].delta2 );
      l_printf( szMesh, 'Z', si[i].delta3 );
     }
   close_listing_file();
  } // end of write_GRID routine


static void _far _fortran  write_INPUT_DATA( INPUT_DATA _far *id )
  {
   int i;

   open_listing_file( WR_END );
   l_printf("\n      **** MULTIPORT PARAMETERS ****" );
   l_printf("       Number of inputs: %d.", id->num );
   for( i = 0; i < id->num; i++ )
      l_printf("       Number of modes at the input %d: %d.", i+1, id->array[i] );
   close_listing_file();
  } // end of write_INPUT_DATA routine
