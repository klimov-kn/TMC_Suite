/*
 *  TAMIC_soft(R) 1991,1992 vR1.02
 *
 *  Definitions and declarations for files writing and reading.
 *
 *  FILE : ..\include\mic\files.h
 *
*/

#ifndef _FILES_D

#ifndef _OFFSET_DEF
 #define _OFFSET_DEF
typedef struct
         {
          _real frq;                     /* frequency */
          DWORD  dwOffset;               /* offset from beginning of the file */
          }_OFFSET;
#endif

#ifndef _OFFSET_D
 #define _OFFSET_D
typedef struct
         {
          WORD         wSize;            /* offset table size */
          WORD         wFrqNum;          /* frequency number  */
          _OFFSET _far *lpOffset;        /* pointer at offset table */
          }_OFFSET_TABLE;

#endif

typedef struct
         {
          char      GsmLabel[4];         /* gsm label*/
          char      GsmVers[4];          /* version number*/
          DWORD     lPs;                /* "pascal" string counter*/
          }GSM_HEADER;

#define  GSM_HEADER_SIZE   sizeof(GSM_HEADER)


#define RGF_PS_NUM 6 /* number of "pascal strings" in RGF files */
#define RGF_WR  0
#define RGF_RD  1

#define GSM_PS_NUM 9 /* number of "pascal strings" in header of GSM file */
#define GSM_WR  0
#define GSM_RD  1

FILE   _far * _fortran Get_RGF_FILE( void );
int    _far   _fortran Open_RGF_file( char _far *szFile, int nOpenMode );
void   _far   _fortran Close_RGF_file( void );
int    _far   _fortran Read_Item_from_RGF( void _far *string, size_t size );
int    _far   _fortran Write_Item_to_RGF( void _far *string, size_t size );
int    _far   _fortran Skip_RGF_String( void );
int    _far   _fortran Set_RGF_Position( fpos_t fPos_t );
int    _far   _fortran Skip_RGF( fpos_t fPos_t );
int    _far   _fortran write_RGF_CR( void );

FILE   _far * _fortran Get_GSM_FILE( void );
int    _far   _fortran Open_GSM_file( char _far *szFile, int nOpenMode );
void   _far   _fortran Close_GSM_file( void );
int    _far   _fortran Read_Item_from_GSM( void _far *string, size_t size );
int    _far   _fortran Write_Item_to_GSM( void _far *string, size_t size );
int    _far   _fortran Skip_GSM_String( void );
int    _far   _fortran Set_GSM_Position( fpos_t fPos_t );
int    _far   _fortran Skip_GSM( fpos_t fPos_t );
int    _far   _fortran write_GSM_CR( void );
#endif
