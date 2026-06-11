 /*
 *  TAMIC_soft(R) 1989,1991,1992 vR1.02
 *
 *  read_S1_matrix - read scattering matrix from the file with double
 *                   precision data at current frequency
 *                   ( procedure with interpolation )
 *  Usage read_S1_matrix:
 *                       read_S1_matrix( szFileName,  sss, id )
 *
 *  Arguments:
 *    szFileName - name of the S-file to be read.
 *           sss - pointer at the pointer at the pointer at the complex value
 *                 for scattering matrices storage. Space allocated for
 *                 S - matrices should be fried by routine Free_S_read.
 *            id - pointer at the INPUT_DATA structures.
 *
 *  Return -1 if error occurred when reading file,
 *          0 if matrix was read normally.
**
**  FILE : ..\tamic\post_pro\read_S1.c
**
**  LIB  : .lib
**
**  OVERLAY: YES
**
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifndef _HIGH_ACCURACY
 #define _HIGH_ACCURACY
#endif

#include <typedef.h>
#include <complex.h>
#include <tmclimit.h>
#include <tmc_lib.h>
#include <matrix.h>
#include <frqdata.h>
#include <error1.h>
#include <s_file.h>
#include <mainwndw.h>
#include <post_pro.h>



static int _near _fortran Read_TITLE( char _far *szFileName );
static int _near _fortran read_string( char _far *string );
static int _near _fortran read_CR( void );

static char _BASE_CODE trace[] = "Rd.Scatt.Mt.";
static int  nWarning = 0;

#define ONE       1.0f
#define GEN_THR   1.000001f

void _far _fortran set_interp_warning( int nWIFlag ) { nWarning = nWIFlag; }
int  _far _fortran get_interp_warning( void )        { return nWarning; }

int _far read_S1_matrix( char _far *szFileName,  _complex _far **sss, INPUT_DATA _far *id, int _far *lpnUnit )
 {
  int length, extr_flag, nFrqNum, nFrqNum1, found = -1, found1 = -1;
  int ret_code = 0, i, nLen, i3;
  fpos_t lOffset, lOffset1;
  _real frq = get_current_frq();
  _real frq_max, frq_min, d, rMax;
  _complex _far *lpxC = NULL;
  _complex _far *lpxRoot = NULL;
  S_OFFSET _far *lpFOT = NULL;
  S_OFFSET _far *lpSOT = NULL;
  FILE _far *sf;

  put_trace( (char _far *)trace );

  if( Open_S_file( szFileName, S_RD ) != 0 ) return -1;
  sf = Get_S_FILE();

  /* reading title and two first title string */
  ret_code = Read_TITLE( szFileName );

  /* reading length of the GRID string ( useless here )*/
  if( ret_code == 0) ret_code = read_S( &length, sizeof( int ) );

  /* extrapolation flag 0 - no, 1 - two grids, 2 - three grids */
  if( ret_code == 0) ret_code = read_S( &extr_flag, sizeof( int ) );

    /* skipping grids mesh sizes */
  if( ret_code == 0 )
    if( fseek( sf, (long)((fpos_t)(3*sizeof( _real )*(extr_flag+1))), (long)SEEK_CUR ) != 0 ) ret_code = -1;

  if( ret_code == 0 ) ret_code = read_CR();

  /* skipping strings with files names */
  for( i = 0; ( i <= extr_flag )&&( ret_code == 0 ); i++ )
    {
     ret_code = skip_string();
     if( ret_code == 0 ) ret_code = skip_string();
    }

  /* reading length of the frequencies offset table ( useless here )*/
  if( ret_code == 0 ) ret_code = read_S( &length, sizeof( int ) );

   /* reading max freq. point number */
  if( ret_code == 0 ) ret_code = read_S( &nFrqNum, sizeof( int ) );

  /* allocation of memory for main offset table */
  if( ret_code == 0 )
     if( ( lpFOT = (S_OFFSET _far *)alloc_mem_err( (long)sizeof(S_OFFSET)*nFrqNum ) ) == NULL )
        ret_code = -2;

  /* reading main offset table */
  if( ret_code == 0 ) ret_code = read_S( lpFOT, (size_t)sizeof(S_OFFSET)*nFrqNum  );

  /* reading separator */
  if( ret_code == 0 ) ret_code = read_CR();

   if( ret_code == 0 )
     {
      /* finding frequency point in offset table */
      for( i = 0;  i < nFrqNum; i++ )
        {
         if( i == 0 )
           {
            frq_max =
            frq_min = lpFOT[i].frq;
           }
         else
           {
            frq_max = max( lpFOT[i].frq, frq_max );
            frq_min = min( lpFOT[i].frq, frq_min );
           }

         if( lpFOT[i].frq == frq )
           {
            found = i;
            break;
           }
        } //end of finding loop
     }

   if( ( found == -1 )&&( ret_code == 0 ) )
     {
      if( ( frq < frq_min )||( frq > frq_max ) )
        {
         put_error_messege( SMT_FRQ_POINT, frq_min, frq_max, szFileName, frq );
         ret_code = -2;
        }

      /* reading length of the frequencies offset table ( useless here ) */
      if( ret_code == 0 ) ret_code = read_S( &length, sizeof( int ) );

       /* reading max freq. point number */
      if( ret_code == 0 ) ret_code = read_S( &nFrqNum1, sizeof( int ) );
      if( nFrqNum1 == 0 )
        {
         put_error_messege( SMT_INTER_TABLE, szFileName );
         ret_code = -2;
        }

      /* allocation of memory for interpolation coeff. offset table */
      if( ret_code == 0 )
        if( ( lpSOT = (S_OFFSET _far *)alloc_mem_err( (long)sizeof(S_OFFSET)*nFrqNum1 ) ) == NULL )
           ret_code = -2;

      /* reading interpolation coefficients offset table */
      if( ret_code == 0 ) ret_code = read_S( lpSOT, (size_t)sizeof(S_OFFSET)*nFrqNum1  );

      if( ret_code == 0 )
        {
         /* looking for start frequency for interpolation procedure */
         for( i = 1; i < nFrqNum1; i++ )
           {
            if( ( frq > lpSOT[i-1].frq )&&( frq < lpSOT[i].frq ) )
              {
               found1 = i-1;
               break;
              }
           } //end of finding loop

         /* finding start frequency in the main offset table */
         for( i = 0; i < nFrqNum; i++ )
           {
	    if( fabs( (double)( lpSOT[found1].frq - lpFOT[i].frq ) ) <= FLT_MIN )
              {
               found = i;
               break;
              }
	   } //end of finding loop
        }
     } // end of finding for interpolation procedure

   if( found >= 0 )  lOffset  = lpFOT[found].lOffset;
   else              ret_code = -3;
   if( found1 >= 0 )
     {
      d = (frq - lpSOT[found1].frq)*NORM_COEFF;
      lOffset1 = lpSOT[found1].lOffset;
     }

   free_mem_null( &lpFOT );
   free_mem_null( &lpSOT );

   /* setting position in the file to number of inputs */
   if( ret_code == 0 )
     if( fseek( sf, (long)((fpos_t)( lOffset+sizeof(_real) )), SEEK_SET ) != 0 ) ret_code = -1;

   if( ret_code != 0 )
     {
      if( ret_code == -1 )  put_error_messege( ERROR_READ_S, szFileName );
      Close_S_file();
      return -1;
     }

   if( ret_code == 0 ) ret_code = read_S( &(id->num), sizeof( int ) );
   if( ret_code == 0 ) ret_code = read_S( id->array, (size_t)sizeof( int )*id->num );
   if( ret_code == 0 ) ret_code = read_S( &(id->dim), sizeof( int ) );
   if( ret_code == 0 ) ret_code = read_CR();

   if( ret_code == 0 )
     {
      nLen = id->dim*id->dim;
      /* allocation of memory for scattering matrix */
      if( ( *sss = (_complex _far *)alloc_mem_err( (long)sizeof(_complex)*nLen ) ) == NULL )
         {
          Close_S_file();
          return -1;
         }
     }

   if( ret_code == 0 ) ret_code = read_S( *sss, (size_t)sizeof( _complex )*nLen );
   if( ret_code == 0 ) ret_code = read_CR();

   *lpnUnit = S_UNKNOWN;

   if( (found1 >= 0 )&&(ret_code == 0 ) )
     {
      *lpnUnit = S_UNITARY;
      if( Check_S_Unitary( *sss, id->dim, &rMax ) == 0 )
        {
         if( rMax > UNITARY_THRESH ) *lpnUnit = S_NOT_UNITARY;
        }

      /* allocation of memory for interpolation coefficients matrix */
      if( ( lpxC = (_complex _far *)alloc_mem_err( (long)sizeof(_complex)*nLen*3 ) ) == NULL )
        {
         Close_S_file();
         free_mem( *sss );
         *sss = NULL;
         return -1;
        }

      /* setting position to interpolation coefficients */
      if( fseek( sf, (long)((fpos_t)( lOffset1 )), SEEK_SET ) != 0 ) ret_code = -1;

      if( ret_code == 0 ) ret_code = read_S( lpxC, (size_t)sizeof( _complex )*nLen*3 );

      if( ret_code == 0 )
        {
         int i2;

         if( nWarning == 0 )
           {
            put_error_messege( INTER_ROOT_ERR );
            nWarning = 1;
           }

         /* perform evaluation */
         for( i = 0, i2 = 0, i3 = 0; i < nLen; i++, i2 += 2, i3 += 3 )
          {
           (*sss)[i].x = (((lpxC[i3 + 2].x)*d + lpxC[i3 + 1].x)*d + lpxC[i3].x)*d + (*sss)[i].x;
           (*sss)[i].y = (((lpxC[i3 + 2].y)*d + lpxC[i3 + 1].y)*d + lpxC[i3].y)*d + (*sss)[i].y;
          }
        }
     }

  free_mem_null( &lpxC );

  Close_S_file();

  if( ret_code == -1 )    put_error_messege( ERROR_READ_S, szFileName );

  return( ( ret_code != 0 )?-1:0 );
 }


static int _near _fortran Read_TITLE( char _far *szFileName )
  {
   char szStr[4];
   int nVer;
   int nPS_Num;

   if( read_S( szStr, (size_t)4*sizeof( char ) ) != 0 )  return -1;
   szStr[3] = '\0';

   if( strcmp( szStr, SMT_LABEL ) != 0 ) //checking file label
     {
      put_error_messege( SMT_LABEL_ERROR, szFileName );
      return -2;
     }

     /* reading file version */
   if( read_S( szStr, 4*sizeof(char) ) != 0 ) return -1;
   szStr[3] = '\0';
   nVer = atoi( szStr );

   if( nVer < SMT_VERSION ) //checking file version
     {
      put_error_messege( SMT_VERSION_ERROR, szFileName );
      return -2;
     }

   if( read_CR() != 0 ) return -1;       //skipping separator

     /* reading number of "Pascal Strings" */
   if( read_S( &nPS_Num, (size_t)sizeof( int ) ) != 0 )  return -1;
   if( read_CR() != 0 ) return -1;

    /* skipping two title string */
   if( skip_string() != 0 ) return -1;
   if( skip_string() != 0 ) return -1;

   return 0;
  } //end of Read_TITLE routine


static int _near _fortran read_string( char _far *string )
 {
  int length;

  if( read_S( &length, sizeof( int ) ) != 0 )        return -1; // reading string length
  if( read_S( string, sizeof( char )*length ) != 0 ) return -1; // reading string
  if( read_CR() != 0) return -1;                                // reading separator
  }


static int _near _fortran read_CR( void )
 {
  int EOL;

  return( read_S( &EOL, sizeof( int ) )  );  // reading CR
  }



