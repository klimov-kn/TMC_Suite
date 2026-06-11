/*
**  TAMIC_soft(R) 1989,1991,1992 vR1.02
**
** save_S_matrix write out scattering matrix to the file
**
**  FILE : ..\source\tamic\post_pro\write_S.c
**
**  LIB  : ooutpXY.lib
**
**  OVERLAY: YES
**
**  DESCRIPTION:
**
**                 S-matrix format description
**             ------------------------------------
**ษออออออออัออออออออออออัอออออออออออออออออออออออออออออออออออออออป
**บ Length ณ Type/Value ณ          Description                  บ
**ฬออออออออุออออออออออออุอออออออออออออออออออออออออออออออออออออออน
**บ   4    ณ 4*char     ณ  File label ( SMT )                   บ
**บ   4    ณ 4*char     ณ  Number of file version               บ
**บ   2    ณ int        ณ  Strings  separator ( LF CR )         บ
**บ   2    ณ int NS     ณ  Number of "pascal strings" ( PS )    บ
**บ   2    ณ int        ณ  Strings  separator ( LF CR )         บ
**วฤฤฤฤฤฤฤฤลฤฤฤฤฤฤฤฤฤฤฤฤลฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ
**บ   N1   ณ PS         ณ  First  program TITLE string          บ
**บ   N2   ณ PS         ณ  Second program TITLE string          บ
**บ   N3   ณ PS         ณ  GRID string                          บ
**บ   N4   ณ PS         ณ  Input data file name( *.tpl )        บ
**บ   N5   ณ PS         ณ  Scattering matrix file name( *.s )   บ
**บ   .    ณ .          ณ  ...                                  บ
**บ  NS-1  ณ PS         ณ  Frequencies offset table             บ
**บ   NS   ณ PS         ณ  Interpolation coeff. offset table    บ
**วฤฤฤฤฤฤฤฤลฤฤฤฤฤฤฤฤฤฤฤฤลฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ
**บ   8    ณ _real      ณ Current frequency                     บ
**บ   2    ณ int NI     ณ Number of inputs                      บ
**บ   D1   ณ int vector ณ Number of modes at inputs( D1 = 2*NI )บ
**บ   2    ณ int NS     ณ Scattering matrix dimension           บ
**บ   2    ณ int        ณ Strings  separator ( LF CR )          บ
**บ   D2   ณ c matrix   ณ Scattering matrix( D2 = 16*(NS*NS) )  บ
**บ   2    ณ int        ณ Strings  separator ( LF CR )          บ
**บ   .    ณ .          ณ ...                                   บ
**วฤฤฤฤฤฤฤฤลฤฤฤฤฤฤฤฤฤฤฤฤลฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ
**บ   D3   ณ c matrix   ณ Interpolation coefficients( 1,2,3 )   บ
**บ        ณ            ณ D3 = 16*(NS*NS)*3                     บ
**บ   2    ณ int        ณ Strings  separator ( LF CR )          บ
**ศออออออออฯออออออออออออฯอออออออออออออออออออออออออออออออออออออออผ
**
**          Structure of the pascal string GRID
**ษออออออออัออออออออออออัอออออออออออออออออออออออออออออออออออออออป
**บ Length ณ Type/Value ณ          Description                  บ
**ฬออออออออุออออออออออออุอออออออออออออออออออออออออออออออออออออออน
**บ   2    ณ int        ณ  Length of the string                 บ
**บ   2    ณ int NG     ณ  Extrapolation flag( number of grids )บ
**บ   8    ณ _real      ณ  Mesh size along X axis               บ ฤฟ
**บ   8    ณ _real      ณ  Mesh size along Y axis               บ  ณ
**บ   8    ณ _real      ณ  Mesh size along Z axis               บ  ณ NG TIMES
**บ   .    ณ .          ณ  ...                                  บ  ณ
**บ   .    ณ .          ณ  ...                                  บ ฤู
**บ   2    ณ int        ณ  Strings  separator ( LF CR )         บ
**ศออออออออฯออออออออออออฯอออออออออออออออออออออออออออออออออออออออผ
**
**         Structure of the offset tables
**ษออออออออัออออออออออออัอออออออออออออออออออออออออออออออออออออออป
**บ Length ณ Type/Value ณ          Description                  บ
**ฬออออออออุออออออออออออุอออออออออออออออออออออออออออออออออออออออน
**บ   2    ณ int        ณ Length of the string                  บ
**บ   2    ณ int  NF    ณ Number of frequencies in the file     บ
**บ   8    ณ _real      ณ Frequency                             บ ฤฟ
**บ   4    ณ long       ณ Offset of multiport parameters        บ  ณ
**บ   .    ณ   .        ณ ...                                   บ  ณ NF TIMES
**บ   .    ณ   .        ณ ...                                   บ  ณ
**บ   8    ณ _real      ณ Frequency                             บ  ณ
**บ   4    ณ long       ณ Offset of multiport parameters        บ ฤู
**บ   2    ณ int 0a0d   ณ Strings separator                     บ
**ศออออออออฯออออออออออออฯอออออออออออออออออออออออออออออออออออออออผ
**
** Abbreviations:
**                  r - _real ( 8 bytes )
**                  c - complex ( 16 bytes )
** Remarks:
**        1.All real and complex vector and matrix dimensions are given
**          for double precision case. When single precision is used
**          all dimensions should be divided by 2.
**        1.Type lengthes are given  for IBM computers.
**        3."Pascal string" begins by string length and is finished by
**           0x0A0D value.
**
*/

#include <stdio.h>
#include <string.h>

#ifndef _HIGH_ACCURACY
 #define _HIGH_ACCURACY
#endif

#include <typedef.h>
#include <tmclimit.h>
#include <tmcgraph.h>
#include <tmc_lib.h>
#include <indandec.h>
#include <frqdata.h>
#include <error1.h>
#include <s_file.h>
#include <mainwndw.h>

#ifdef _MODEL_3D
  #include <label\3d.h>
#endif

#ifdef _MODEL_IC
  #include <label\ic.h>
#endif

#ifdef _MODEL_M
 #include <label\m.h>
#endif

#ifdef _MODEL_H
 #include <label\h.h>
#endif

#ifdef _MODEL_E
 #include <label\e.h>
#endif

#ifdef _MODEL_W
 #include <label\w.h>
#endif

#include <label\rth.h>


#ifndef _HIGH_ACCURACY
#pragma message("High accuracy not defined in write_S!!!!")
#endif


static int _near _fortran Write_TITLE( void );
static int _near _fortran Write_GRID( S_INDEX _far *si, int extrapol_flag );
static int _near _fortran write_offset_table( WORD cnFrq );
static int _near _fortran write_string( char _far *string );
static int _near _fortran write_CR( void );

static char _BASE_CODE trace[] = "Wrt.Scatt.Mt.";

int _far save_S_matrix( _complex _far *sss, INPUT_DATA _far *id,
                        char _far *S_filename, S_INDEX _far *si, int extrapol_flag )
 {
  register int ret_code = -1, i;
  INT_1 _far *lpnArr;
  INT_1 sBuff;
  _real fTmp;

  put_trace( (char _far *)trace );

  if( Open_S_file( S_filename, S_WR ) != 0 ) return -1;

  if( get_current_frq_num() == 0 )
   {
    Set_PS_Number( S_PS_NUMBER + 2*extrapol_flag  );
    ret_code = Write_TITLE();

    if( ret_code == 0 ) ret_code = write_string( MAIN_LABEL );
    if( ret_code == 0 ) ret_code = write_string( TWP_LABEL );
    if( ret_code == 0 ) ret_code = Write_GRID( si, extrapol_flag );

    /* writing names of input data and scattering matrix files */
    for( i = 0; ( i <= extrapol_flag )&&( ret_code == 0 ); i++ )
      {
       ret_code = write_string( si[i].tpl_name );
       if( ret_code == 0 ) ret_code = write_string( si[i].S_name );
      }

    if( ret_code == 0 ) ret_code = write_offset_table( (WORD)get_frq_nf() );
    if( ret_code == 0 ) ret_code = write_offset_table( (WORD)0 );
   }/* frq.num == 0*/
  else ret_code = 0;

  fTmp = (_real)get_current_dbl_frq();
  if( ret_code == 0 ) ret_code = update_offset_table( fTmp, get_current_frq_num(), OFF_TBL_MAIN );

  if( ret_code == 0 ) ret_code = write_S( &fTmp, (size_t)sizeof( _real ) );
  sBuff = id->num;
  if( ret_code == 0 ) ret_code = write_S(  &sBuff, (size_t)sizeof(INT_1) );
  for( i = 0, lpnArr = id->array; i < id->num; i++, lpnArr++ )
    {
     sBuff = (INT_1)(*lpnArr);
     if( write_S( &sBuff, (size_t)sizeof(INT_1) ) != 0 )   return -1;
    }
  sBuff = id->dim;
  if( ret_code == 0 ) ret_code = write_S(  &sBuff, (size_t)sizeof(INT_1) );
  if( ret_code == 0 ) ret_code = write_CR();

  if( ret_code == 0 ) ret_code = write_S( sss, (size_t)sizeof( _complex )*id->dim*id->dim );
  if( ret_code == 0 ) ret_code = write_CR();

  Close_S_file();

  if( ret_code != 0 ) put_error_messege( ERROR_WRITE_S, S_filename );

  return ret_code;
  }


static int _near _fortran Write_TITLE( void )
  {
   char buff[4];
   char form[4]="%3d";
   INT_1 nPS_Num = (INT_1)Get_PS_Number();

   if( write_S( SMT_LABEL, (size_t)4*sizeof( char ) ) != 0 )  return -1;

   sprintf( buff, form, SMT_VERSION );
   if( write_S( buff, (size_t)4*sizeof( char ) ) != 0 )      return -1;
   if( write_CR() != 0 ) return -1;

   if( write_S( &nPS_Num, (size_t)sizeof(INT_1) ) != 0 )  return -1;
   if( write_CR() != 0 ) return -1;

   return 0;
  } /*end of Write_TITLE routine*/




static int _near _fortran Write_GRID( S_INDEX _far *si, int extrapol_flag )
  {
   int ret_code = 0, i;
   INT_1 length, sFlag = (INT_1)extrapol_flag;

   length = sizeof(INT_1) + 3*sizeof(double)*(extrapol_flag+1);
   ret_code = write_S( &length, (size_t)sizeof(INT_1) );

   /* extrapolation flag 0 - no, 1 - two grids, 2 - three grids */
   if( ret_code == 0) ret_code = write_S( &sFlag, (size_t)sizeof(INT_1) );

   for( i = 0; ( i <= extrapol_flag )&&( ret_code == 0 ); i++ )
     {
      ret_code = write_S( &si[i].delta1, (size_t)sizeof( double ) );
      if( ret_code == 0 ) ret_code = write_S( &si[i].delta2, (size_t)sizeof( double ) );
      if( ret_code == 0 ) ret_code = write_S( &si[i].delta3, (size_t)sizeof( double ) );
     }

   if( ret_code == 0 ) ret_code = write_CR();

   return ret_code;
  } /*end of Write_GRID routine*/



static int _near _fortran write_offset_table( WORD cnFrq )
 {
  WORD cnFrqNum = get_frq_nf();
  S_OFFSET_TABLE OffsetTable;
//  S_OFFSET       OffsetDummy;
  long lOffInt = 0L;
  WORD i;
  int j;
  double frq;
  FILE _far *sf = Get_S_FILE();

  OffsetTable.cbSize = sizeof(WORD) +              /* cnFrqNum value size*/
                       (sizeof(_real)+sizeof(int))*cnFrqNum;  /* Frq offset table size*/

  OffsetTable.cnFrqNum = cnFrq;

  if( write_S( &OffsetTable, sizeof(WORD)*2 ) != 0 ) return -1;

  /* offset table dummy */
  frq = 0.0;
  j = 0;
  for( i = 0; i < cnFrqNum; i++ )
    {
     if( write_S( &(frq), sizeof(double) ) != 0 ) return -1;
     if( write_S( &(j), sizeof(int) ) != 0 ) return -1;
    }

  if( write_CR() != 0 ) return -1;              /* CR separator*/

  return 0;
  }



static int _near _fortran write_string( char _far *string )
 {
  INT_1 length = (INT_1)strlen( string );                 /* string length*/

  if( write_S( &length, (size_t)sizeof( INT_1 )) != 0) return -1;           /* write string length*/
  if( write_S( string, (size_t)sizeof( char )*length ) != 0 ) return -1;   /* write string*/
  if( write_CR() != 0 ) return -1;         /* delimiter*/
  return 0;
 } /*end of write_string routine*/



static int _near _fortran write_CR( void )
 {
  INT_1 EOL = 0x0a0d;
  return( write_S( &EOL, sizeof( INT_1 ) ) );    /* delimiter*/
  }


