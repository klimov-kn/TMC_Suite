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
**…ЌЌЌЌЌЌЌЌ—ЌЌЌЌЌЌЌЌЌЌЌЌ—ЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌї
**Ї Length ≥ Type/Value ≥          Description                  Ї
**ћЌЌЌЌЌЌЌЌЎЌЌЌЌЌЌЌЌЌЌЌЌЎЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌє
**Ї   4    ≥ 4*char     ≥  File label ( SMT )                   Ї
**Ї   4    ≥ 4*char     ≥  Number of file version               Ї
**Ї   2    ≥ int        ≥  Strings  separator ( LF CR )         Ї
**Ї   2    ≥ int NS     ≥  Number of "pascal strings" ( PS )    Ї
**Ї   2    ≥ int        ≥  Strings  separator ( LF CR )         Ї
**«ƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒґ
**Ї   N1   ≥ PS         ≥  First  program TITLE string          Ї
**Ї   N2   ≥ PS         ≥  Second program TITLE string          Ї
**Ї   N3   ≥ PS         ≥  GRID string                          Ї
**Ї   N4   ≥ PS         ≥  Input data file name( *.tpl )        Ї
**Ї   N5   ≥ PS         ≥  Scattering matrix file name( *.s )   Ї
**Ї   .    ≥ .          ≥  ...                                  Ї
**Ї  NS-1  ≥ PS         ≥  Frequencies offset table             Ї
**Ї   NS   ≥ PS         ≥  Interpolation coeff. offset table    Ї
**«ƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒґ
**Ї   8    ≥ _real      ≥ Current frequency                     Ї
**Ї   2    ≥ int NI     ≥ Number of inputs                      Ї
**Ї   D1   ≥ int vector ≥ Number of modes at inputs( D1 = 2*NI )Ї
**Ї   2    ≥ int NS     ≥ Scattering matrix dimension           Ї
**Ї   2    ≥ int        ≥ Strings  separator ( LF CR )          Ї
**Ї   D2   ≥ c matrix   ≥ Scattering matrix( D2 = 16*(NS*NS) )  Ї
**Ї   2    ≥ int        ≥ Strings  separator ( LF CR )          Ї
**Ї   .    ≥ .          ≥ ...                                   Ї
**«ƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒґ
**Ї   D3   ≥ c matrix   ≥ Interpolation coefficients( 1,2,3 )   Ї
**Ї        ≥            ≥ D3 = 16*(NS*NS)*3                     Ї
**Ї   2    ≥ int        ≥ Strings  separator ( LF CR )          Ї
**»ЌЌЌЌЌЌЌЌѕЌЌЌЌЌЌЌЌЌЌЌЌѕЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЉ
**
**          Structure of the pascal string GRID
**…ЌЌЌЌЌЌЌЌ—ЌЌЌЌЌЌЌЌЌЌЌЌ—ЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌї
**Ї Length ≥ Type/Value ≥          Description                  Ї
**ћЌЌЌЌЌЌЌЌЎЌЌЌЌЌЌЌЌЌЌЌЌЎЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌє
**Ї   2    ≥ int        ≥  Length of the string                 Ї
**Ї   2    ≥ int NG     ≥  Extrapolation flag( number of grids )Ї
**Ї   8    ≥ _real      ≥  Mesh size along X axis               Ї ƒњ
**Ї   8    ≥ _real      ≥  Mesh size along Y axis               Ї  ≥
**Ї   8    ≥ _real      ≥  Mesh size along Z axis               Ї  ≥ NG TIMES
**Ї   .    ≥ .          ≥  ...                                  Ї  ≥
**Ї   .    ≥ .          ≥  ...                                  Ї ƒў
**Ї   2    ≥ int        ≥  Strings  separator ( LF CR )         Ї
**»ЌЌЌЌЌЌЌЌѕЌЌЌЌЌЌЌЌЌЌЌЌѕЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЉ
**
**         Structure of the offset tables
**…ЌЌЌЌЌЌЌЌ—ЌЌЌЌЌЌЌЌЌЌЌЌ—ЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌї
**Ї Length ≥ Type/Value ≥          Description                  Ї
**ћЌЌЌЌЌЌЌЌЎЌЌЌЌЌЌЌЌЌЌЌЌЎЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌє
**Ї   2    ≥ int        ≥ Length of the string                  Ї
**Ї   2    ≥ int  NF    ≥ Number of frequencies in the file     Ї
**Ї   8    ≥ _real      ≥ Frequency                             Ї ƒњ
**Ї   4    ≥ long       ≥ Offset of multiport parameters        Ї  ≥
**Ї   .    ≥   .        ≥ ...                                   Ї  ≥ NF TIMES
**Ї   .    ≥   .        ≥ ...                                   Ї  ≥
**Ї   8    ≥ _real      ≥ Frequency                             Ї  ≥
**Ї   4    ≥ long       ≥ Offset of multiport parameters        Ї ƒў
**Ї   2    ≥ int 0a0d   ≥ Strings separator                     Ї
**»ЌЌЌЌЌЌЌЌѕЌЌЌЌЌЌЌЌЌЌЌЌѕЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЌЉ
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

/// @file Write_s.cpp
/// @brief «апись S-матрицы одной частоты в двоичный S-файл (`save_S_matrix`) и служебные части заголовка.
///
/// „асть модул€ ЂЅиблиотека sfile95ї. —хема формата в шапке выше дана дл€
/// 16-битных типов TAMIC; в сборке целые пол€ заголовка и блоков Ч `INT_1`
/// (`short`, 2 байта), смещени€ Ч `int` (4 байта), числа Ч `double` (8 байт).
/// “очна€ раскладка Ч на странице модул€.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#s-format

#include <stdio.h>
#include <string.h>

#ifndef _HIGH_ACCURACY
 #define _HIGH_ACCURACY
#endif

#include <Typedef.h>
#include <TMCLIMIT.H>
#include <TMCGRAPH.H>
#include <TMC_LIB.H>
#include <INDANDEC.H>
#include <FRQDATA.H>
#include <Error1.h>
#include <S_file.h>
#include <MAINWNDW.H>







#include <label/rth.h>


#ifndef _HIGH_ACCURACY
#endif


/// @brief ѕишет начало S-файла: метку `SMT`, версию Ђ102ї, разделитель, число Ђпаскалевыхї строк, разделитель.
/// @return 0 Ч успех; -1 Ч ошибка записи.
static int Write_TITLE( void );
/// @brief ѕишет Ђпаскалевуї строку сетки: длину, `extrapol_flag` и по три шага (`double`, м) на каждую сетку.
/// @param[in] si             массив из `extrapol_flag+1` индексов сетки
/// @param[in] extrapol_flag  число сеток минус один (0 Ч одна сетка, 1 Ч две, 2 Ч три)
/// @return 0 Ч успех; -1 Ч ошибка записи.
static int Write_GRID( S_INDEX *si, int extrapol_flag );
/// @brief ѕишет пустую таблицу смещений на `get_frq_nf()` записей (нулевые частоты и смещени€).
/// @param[in] cnFrq  число частот в шапке таблицы: дл€ основной Ч `get_frq_nf()`, дл€ таблицы интерпол€ции Ч 0
/// @return 0 Ч успех; -1 Ч ошибка записи.
static int write_offset_table( WORD cnFrq );
/// @brief ѕишет Ђпаскалевуї строку: длину (`INT_1`), текст без завершающего нул€, разделитель.
/// @param[in] string  строка, не длиннее 32767 символов
/// @return 0 Ч успех; -1 Ч ошибка записи.
static int write_string( const char *string );
/// @brief ѕишет разделитель Ч значение `INT_1` 0x0a0d (байты 0D 0A).
/// @return 0 Ч успех; -1 Ч ошибка записи.
static int write_CR( void );

/// “екст строки состо€ни€ при записи (через `put_trace`, в сборке Ч заглушка).
static char trace[] = "Wrt.Scatt.Mt.";

int save_S_matrix( _complex *sss, INPUT_DATA *id,
                        char *S_filename, S_INDEX *si, int extrapol_flag )
 {
  int ret_code = -1, i;
  INT_1 *lpnArr;
  INT_1 sBuff;
  _real fTmp;

  put_trace( (char *)trace );

  if( Open_S_file( S_filename, S_WR ) != 0 ) return -1;

  // перва€ частота: файл создан заново (Open_S_file, "wb+") Ч пишетс€ заголовок
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

  // занести смещение будущего блока в основную таблицу и дописать блок в конец файла
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


static int Write_TITLE( void )
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




static int Write_GRID( S_INDEX *si, int extrapol_flag )
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



static int write_offset_table( WORD cnFrq )
 {
  WORD cnFrqNum = get_frq_nf();
  S_OFFSET_TABLE OffsetTable;
//  S_OFFSET       OffsetDummy;
  long lOffInt = 0L;
  WORD i;
  int j;
  double frq;
  FILE *sf = Get_S_FILE();

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



static int write_string( const char *string )
 {
  INT_1 length = (INT_1)strlen( string );                 /* string length*/

  if( write_S( &length, (size_t)sizeof( INT_1 )) != 0) return -1;           /* write string length*/
  if( write_S( string, (size_t)sizeof( char )*length ) != 0 ) return -1;   /* write string*/
  if( write_CR() != 0 ) return -1;         /* delimiter*/
  return 0;
 } /*end of write_string routine*/



static int write_CR( void )
 {
  INT_1 EOL = 0x0a0d;
  return( write_S( &EOL, sizeof( INT_1 ) ) );    /* delimiter*/
  }


