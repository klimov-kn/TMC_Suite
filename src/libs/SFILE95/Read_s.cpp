/*
 *  TAMIC_soft(R) 1990,1991,1992 vR1.02
 *
 *  read_S_matrix   - read scattering matrices from the file with
 *                    double precision data ( multiple frequencies )
 *  Free_GRID_Index - free space allocated for S_INDEX arrays
 *  Free_S_read     - free space allocated for read scattering matrices
 *
 *  Usage read_S_matrix:
 *        read_S_matrix( szFileName, si, extrapol_flag, sss, frq_out, id )
 *
 *  Arguments:
 *    szFileName - name of the S-file to be read.
 *            si - pointer at the pointer at the S_INDEX structure.
 *                 At the output will point at space with extrapol_flag+1
 *                 S_INDEX structures. Space allocated for si should be fried
 *                 by routine Free_GRID_Index.
 * extrapol_flag - number of grids minus one used in extrapolation process.
 *           sss - pointer at the pointer at the pointer at the complex value
 *                 for scattering matrices storage. Space allocated for
 *                 S - matrices should be fried by routine Free_S_read.
 *       frq_out - pointer at FREQUENCY structure. Space for frequencies
 *                 values will be allocated in read_S_matrix routine.
 *            id - pointer at the pointer at the INPUT_DATA structures.
 *                 Structures INPUT_DATA will be allocated for all frequencies
 *                 and space then should be fried.
 *
 *  Return -1 if error occurred when reading file,
 *         -2 if file could not be opened,
 *         -3 if not enough memory to read matrix,
 *          0 if matrix was read normally.
 *
 *
 *  Usage Free_GRID_Index:
 *        Free_GRID_Index( si, extrapol_flag )
 *  Arguments description see above.
 *
 *  Usage Free_S_read:
 *        Free_S_read( sss, nFrqNum )
 *  Arguments:
 *        sss - see above.
 *    nFrqNum - number of friquency points of S-matrix in sss array.
 *
 *  FILE : ..\tamic\post_pro\read_S.c
 *
 *  LIB  : ooutpXY.lib
 *
 *  OVERLAY: YES
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define _HIGH_ACCURACY

#include <typedef.h>
#include <tmclimit.h>
#include <tmcgraph.h>
#include <tmc_lib.h>
#include <indandec.h>
#include <frqdata.h>
#include <s_file.h>
#include <mainwndw.h>
#include <proc_s.h>


//#ifndef _TMC_EXT

#include <error1.h>

//#else
// #ifndef _NDP2
//   #include "c:\tmc\source\utility\extrapol\errorext.h"
// #else
//   #include "c:\tmc\source\utility\extrapol\errorext.h"
// #endif
//#endif


static int _near _fortran Read_TITLE( char _far *szFileName );
static int _near _fortran Read_GRID( S_INDEX _far **si, int _far *extrapol_flag );
static int _near _fortran read_string( char _far *string );
static int _near _fortran read_CR( void );

static char _BASE_CODE trace[] = "Rd.Scatt.Mt.";


int _far read_S_matrix( char _far *szFileName, S_INDEX _far **si, int _far *extrapol_flag,
                        _complex _far ***sss, FREQUENCY _far *frq_out, INPUT_DATA _far **id)
 {
  int ret_code = 0, i, m;
  WORD len, length, wBuff;
  INT_1 sBuff;
  int iBuff;
  INT_1 _far *lpnArr;
  double dTmp;
  double _far *lpdTmp;
  fpos_t  lNextPoint, lOffset;
  FILE _far *sf;

//   put_trace( (char _far *)trace );

  frq_out->max = 0;

  if( Open_S_file( szFileName, S_RD ) != 0 ) return -2;
  sf = Get_S_FILE();

  // reading and checking title and two first title string 
  ret_code = Read_TITLE( szFileName );

  // reading grid parameters 
  if( ret_code == 0 ) ret_code = Read_GRID( si, extrapol_flag );
  if( ret_code != 0 ) goto Close_L;


  // reading length of the frequencies offset table ( useless here ) 
  if( ret_code == 0 ) ret_code = read_S( &length, (size_t)sizeof(WORD) );

  // reading max freq. point number 
  if( ret_code == 0 ) ret_code = read_S( &wBuff, sizeof(WORD) );
  frq_out->max = (int)wBuff;

  if( ret_code != 0 )
    {
     Free_GRID_Index( si, *extrapol_flag );
     goto Close_L;
    }

   // allocation of memory for frq buff 
   if( (frq_out->f = (_real _far *)alloc_mem_err( (long)sizeof(_real)*(frq_out->max) ) ) != NULL)
    {
     // allocation of memory for input data structures 
     if( (*id = (INPUT_DATA _far *)alloc_mem_err( (long)sizeof(INPUT_DATA)*(frq_out->max) ) ) != NULL )
      {
       // allocation of memory for pointers at scattering matrices 
       if( ((*sss) = (_complex _far **)alloc_mem_err( (long)sizeof(void _far *)*(frq_out->max) ) ) != NULL )
        {
         for( i = 0; i < frq_out->max; i++)	 (*sss)[i] = NULL;

         // main frequency loop 
         for( i = 0; ( i < frq_out->max)&&( ret_code == 0 ); i++)
          {
           // set position in the offset table
           if( i != 0 )  
		   { 
//			   if( fseek( sf, (long)lNextPoint, SEEK_SET ) != 0 ) ret_code = -1; 
			   if( fsetpos( sf, &lNextPoint ) != 0 ) ret_code = -1; 
		   };
//
//if( ret_code == 0 ) { if( fgetpos( sf, &lNextPoint ) != 0 ) ret_code = -1; };
//
           // reading frequency and offset 
           if( ret_code == 0 )
             {
              ret_code = read_S( &dTmp, sizeof( double ) );
              frq_out->f[i] = (_real)dTmp;
             }
//
//if( ret_code == 0 ) { if( fgetpos( sf, &lNextPoint ) != 0 ) ret_code = -1; }
//
           if( ret_code == 0 )
		   {
			   ret_code = read_S( &iBuff, sizeof( int ) );
			   lOffset = iBuff;
		   };
//
//if( ret_code == 0 ) { if( fgetpos( sf, &lNextPoint ) != 0 ) ret_code = -1; }
//

           // get offset of the next frequency point in the offset table 
           if( ret_code == 0 ) 
		   { 
			   if( fgetpos( sf, &lNextPoint ) != 0 ) ret_code = -1; 
		   }


           // set position in the file to frequency point 
           if( ret_code == 0 )  
		   { 
			   if( fseek( sf, (long)(lOffset+sizeof(double)), SEEK_SET ) != 0 ) ret_code = -1; 
		   }

           if( ret_code == 0 ) ret_code = read_S( &sBuff, sizeof(INT_1) );
           (*id)[i].num = sBuff;
           for( m = 0, lpnArr = (*id)[i].array; (m < (*id)[i].num)&&(ret_code == 0 ); m++, lpnArr++ )
             {
              if( read_S( &sBuff, (size_t)sizeof(INT_1) ) != 0 )   ret_code = -1;
              *lpnArr = sBuff;
             }
           if( ret_code == 0 ) ret_code = read_S( &sBuff, sizeof(INT_1) );
           (*id)[i].dim = sBuff;
           if( ret_code == 0 ) ret_code = read_CR();

           // allocation of memory for scattering matrix 
           if( ret_code == 0 )
             {
              len = ((*id)[i].dim)*((*id)[i].dim);
              if( ( (*sss)[i] = (_complex _far *)alloc_mem_err( (long)sizeof(_complex)*len ) ) == NULL )
                ret_code = i;
             }
#ifdef _HIGH_ACCURACY
           lpdTmp = (double _far *)(*sss)[i];
#else
           // allocation of memory for auxiliary array 
           if( ret_code == 0 )
             {
              if( ( lpdTmp = (double _far *)alloc_mem_err( (long)2*sizeof(double)*len ) ) == NULL )
                {
                 ret_code = i;
                 free_mem( (*sss)[i] );
                }
             }
#endif
           if( ret_code == 0 ) ret_code = read_S( lpdTmp, (size_t)2*sizeof(double)*len );

#ifndef _HIGH_ACCURACY
           {
           int j, k;

           if( ret_code == 0 )
             {
              // copy from double buffer to single pr. array 
              for( j = 0, k = 0; j < len; j++, k += 2 )
                {
                 (*sss)[i][j].x = (_real)lpdTmp[k];
                 (*sss)[i][j].y = (_real)lpdTmp[k+1];
                }
              free_mem_null( &lpdTmp );
             }
           }
#endif

           if( ret_code == 0 ) ret_code = read_CR();
          } // end  of frequency loop 

         if( ret_code > 0 )
           {
            frq_out->max = ret_code;
            if( put_error_messege( SMT_FRQ_MEMORY, frq_out->max ) != 0 )
                 ret_code = -3;
            else ret_code =  0;
           }

          // copy current frq unit to read struct 
         for( i = 0; i < 4; i++ )
          {
           frq_out->unit[i] =  get_frq_unit(i);
           frq_out->item[i] =  get_frq_item(i);
           }
         frq_out->code = get_frq_item_code();

         if( ret_code != 0 )
           {
            free_mem( *id );
            free_mem( frq_out->f );
            Free_S_read( sss, frq_out->max );
            if( ret_code != -3 ) Free_GRID_Index( si, *extrapol_flag );
           }
        } // if sss end 
       else
        {
         free_mem( *id );
         free_mem( frq_out->f );
         ret_code = -3;
        }
      } // if id end 
     else
      {
       free_mem( frq_out->f );
       ret_code = -3;
      }

  } // if f end 
  else  ret_code = -3;

  if( ret_code == -3 )  Free_GRID_Index( si, *extrapol_flag );

Close_L:
  Close_S_file();

  if( ret_code == -1 )  put_error_messege( ERROR_READ_S, szFileName );

  return ret_code;
 } // end of read_S_matrix


void _far _fortran Free_GRID_Index( S_INDEX _far **si, int extrapol_flag )
  {
   int i;

   for( i = 0; i <= extrapol_flag; i++ )
     {
      free_mem( (*si)[i].S_name );
      free_mem( (*si)[i].tpl_name );
     }
   free_mem_null( (void**)si );

  }

void _far _fortran Free_S_read( _complex _far ***sss, int nFrqNum )
  {
   int i;

   if( *sss != NULL )
     {
      for( i = 0; i < nFrqNum; i++)
	  {
		  if( (*sss)[i] != NULL ) 
		  {
			  free_mem( (*sss)[i] );
			  (*sss)[i] = NULL;
		  };
	  };
      free_mem_null( (void **)sss );
     }
  }




static int _near _fortran Read_TITLE( char _far *szFileName )
  {
   char szStr[4];
   int nVer;
   INT_1 nPS_Num;

   if( read_S( szStr, (size_t)4*sizeof( char ) ) != 0 )  return -1;
   szStr[3] = '\0';

   if( strcmp( szStr, SMT_LABEL ) != 0 ) // checking file label
     {
      put_error_messege( SMT_LABEL_ERROR, szFileName );
      return -2;
     }

     // reading file version 
   if( read_S( szStr, 4*sizeof(char) ) != 0 ) return -1;
   szStr[3] = '\0';
   nVer = atoi( szStr );

   if( nVer < SMT_VERSION ) // checking file version
     {
      put_error_messege( SMT_VERSION_ERROR, szFileName );
      return -2;
     }

   if( read_CR() != 0 ) return -1;       // skipping separator

     // reading number of "Pascal Strings" 
   if( read_S( &nPS_Num, (size_t)sizeof(INT_1) ) != 0 )  return -1;
   Set_PS_Number( (int)nPS_Num );
   if( read_CR() != 0 ) return -1;

    // skipping two title string 
   if( skip_string() != 0 ) return -1;
   if( skip_string() != 0 ) return -1;

   return 0;
  } // end of Read_TITLE routine


static int _near _fortran Read_GRID( S_INDEX _far **si, int _far *extrapol_flag )
  {
   int ret_code = 0;
   INT_1 length, sBuff;
   int i;

   // reading length of the GRID string ( useless here )
   ret_code = read_S( &length, (size_t)sizeof(INT_1) );
   // extrapolation flag 0 - no, 1 - two grids, 2 - three grids 
   if( ret_code == 0) ret_code = read_S( &sBuff, sizeof(INT_1) );
   *extrapol_flag = sBuff;

   if( ret_code == 0 )
    {
     // allocation of space for grid sequence parameters 
     if( (*si = (S_INDEX _far  *)alloc_mem_err( (long)sizeof(S_INDEX)*(*extrapol_flag+1) ) ) == NULL)
        return -2;
    }

     // reading grids mesh sizes 
     for( i = 0; ( i <= *extrapol_flag )&&( ret_code == 0 ); i++ )
       {
        ret_code = read_S( &((*si)[i].delta1), sizeof(double) );

        if( ret_code == 0 )
           ret_code = read_S( &((*si)[i].delta2), sizeof(double) );

        if( ret_code == 0 )
           ret_code = read_S( &((*si)[i].delta3), sizeof(double) );
       }

     if( ret_code == 0 ) ret_code = read_CR();

     if( ret_code != 0 )
       {
        free_mem( *si );
        return -1;
       }

     // allocation of space for file names 
     for( i = 0; ( i <= *extrapol_flag )&&( ret_code == 0 ); i++ )
       {
        if( ( (*si)[i].tpl_name = (char _far  *)alloc_mem_err( (long)sizeof(char)*FILENAME_MAX ) ) != NULL)
          {
           if( ( (*si)[i].S_name = (char _far  *)alloc_mem_err( (long)sizeof(char)*FILENAME_MAX ) ) == NULL)
             {
              free_mem( (*si)[i].tpl_name );
              ret_code = i;
             }
          }
        else
           ret_code = i;
       }

     if( ret_code != 0 )
       {
        for( i = ret_code-1; i >= 0; i-- )
          {
           free_mem( (*si)[i].S_name );
           free_mem( (*si)[i].tpl_name );
          }
        free_mem( *si );
        return -2;
       }

     // reading strings with file names 
     for( i = 0; ( i <= (*extrapol_flag) )&&( ret_code == 0 ); i++ )
       {
        ret_code = read_string( (*si)[i].tpl_name );
        if( ret_code == 0 ) ret_code = read_string( (*si)[i].S_name );
       }

   return ret_code;
  } // end of Read_GRID



static int _near _fortran read_string( char _far *string )
 {
  INT_1 length;

  if( read_S( &length, sizeof(INT_1) ) != 0 )        return -1; //  reading string length
  if( read_S( string, sizeof( char )*length ) != 0 ) return -1; //  reading string
  string[length] = 0;                                           //  marking end of string
  if( read_CR() != 0) return -1;                                //  reading separator
  return 0;
  }


static int _near _fortran read_CR( void )
 {
  INT_1 EOL;

  return( read_S( &EOL, sizeof(INT_1) )  );  //  reading CR
  }



