/*
**  TAMIC_soft(R) 1990,1991,1992 v1.00
**
** Macros and decloration for twp_file programs
**
** FILE: ..\include\twp_file.h
**
*/

#ifndef _TWP_FILE_H
  #define _TWP_FILE_H


                              /* open mode flags*/
#define RD_TWP          0       /* open to read TWP-file*/
#define WR_TWP          1       /* open to write TWP-file*/
#define WR_TWP_NEXT     2       /* open to update TWP-file*/

#define TWP_EXT           "TWP"
#ifdef _MODEL_H
 #pragma message("H_plane program build . . .")
 #define TWP_MODEL        "TPH"
 #define TWP_PS_NUM        4
#endif

#ifdef _MODEL_E
 #pragma message("E_plane program build . . .")
 #define TWP_MODEL        "TPE"
 #define TWP_PS_NUM        4
#endif

#if ( defined _MODEL_IC )||( defined _MODEL_M )
 #pragma message("Microstrip analizer program build . . .")
 #define TWP_MODEL        "TPM"
 #define TWP_PS_NUM        4
#endif

#if defined TMC_TWP || defined _MODEL_W || defined _MODEL_3D
 #pragma message("TMC_TWP conversion utility || HE-prog. || 3D-prog. build . . .")
 #define TWP_MODEL        "3DT"
 #define TWP_PS_NUM        4
#endif

                                            /* ver:1 up to  03-14-90 01:15pm*/
                                            /* ver:2        07-17-91 11:05am*/
                                            /* ver:3        03-10-92 02:38pm*/
#define TWP_VER         4                   /* ver:4        06-10-93 04:23pm (mm-dd-yy)*/

#ifndef _MODEL_M
 #define POLYGON_SIZE   ( 2*(sizeof(INT) + sizeof(float)) )
 #define POLYLINE_SIZE  ( 2*sizeof(INT) + 4*sizeof(float) )
#endif

struct _twp_file
          {
           char filename[FILENAME_MAX];
           FILE *f;
           };

typedef struct { INT  code;
                  INT  code2;
                  INT  code3;
                  char reserved[4];
                  WORD num;
                 }TWP_WRITE_ITEM;

#ifndef _TWP_OFFSET_DEF
 #define _TWP_OFFSET_DEF

typedef struct
         {
          DWORD  dwCurrentBlockNumber;    /* current Block Number*/
          DWORD  dwOffset;               /* offset of current Block section*/
          }TWP_OFFSET;
#endif


#ifndef _TWP_HEADER_DEF
 #define _TWP_HEADER_DEF

typedef struct
         {
          DWORD     cbSize;              /* table size;*/
          DWORD     dwBlockTableOffset;  /* offset of block table*/
          DWORD     dwInputTableOffset;  /* offset of input table*/
          DWORD     dwLimitsTableOffset; /* offset of limits table*/
          DWORD     dwFirstItemOffset;   /* offset of first item*/
          DWORD     dwCR;                /* CR double word */
         } TWP_TABLE;

typedef struct
         {
          char      TwpLabel[4];         /* twp label*/
          DWORD     nVer;                /* version number*/
          float     fHeightWidth;        /* H(E)-parameter*/
          DWORD      cnPs;               /* "pascal" string counter*/
          TWP_TABLE Tables;              /* TWP file table of offset tables*/
          }TWP_HEADER;

#define  TWP_HEADER_SIZE   sizeof(TWP_HEADER)
#endif

#ifndef _TWP_OFFSET_TABLE_DEF
 #define _TWP_OFFSET_TABLE_DEF

typedef struct
         {
          WORD             cbSize;              /* offset table size*/
          WORD             wBlockNumber;        /* block number*/
          TWP_OFFSET _far *lpOffset;            /* pointer to offset table*/
          }TWP_OFFSET_TABLE;

#define  TWP_OFFSET_TABLE_SIZE(num)   (sizeof(WORD)*2 + sizeof(TWP_OFFSET)*num)
#endif

#ifndef _INP_ADRESS
 #include <run.h>
#endif

#ifndef _TWP_INP_TABLE_DEF
 #define _TWP_INP_TABLE_DEF

typedef struct
         {
          WORD             cbSize;              /* input table size*/
          WORD             wInputNumber;        /* input number*/
          INP_ADRESS _far *lpTable;             /* pointer to input table*/
          }TWP_INP_TABLE;

#define  TWP_INP_TABLE_SIZE(num)   (sizeof(WORD)*2 + sizeof(INP_ADRESS)*num)
#endif


#ifndef __3D_POINT
  #define __3D_POINT

typedef struct { double x;
                 double y;
                 double z;
		 } _3D_VECTOR;

typedef _3D_VECTOR  _3D_POINT;

typedef struct
          {
           _3D_POINT   _3D_Max;
           _3D_POINT   _3D_Min;
           } _3D_FRAME;
#endif


#ifndef _TWP_LIMIT_TABLE_DEF
 #define _TWP_LIMIT_TABLE_DEF

typedef struct
         {
          WORD             cbSize;              /* limit table size*/
          WORD             wLimitNumber;        /* number of items*/
          _3D_FRAME  _far *lpTable;             /* pointer to limits table*/
          }TWP_LIMIT_TABLE;

#define  TWP_LIMIT_TABLE_SIZE(num)   (sizeof(WORD)*2 + sizeof(_3D_FRAME)*num)
#endif

#define _POINT_LIST   -60
#define _LINE_LIST    -59
#define _SURF_LIST    -58
#define _SHAPE_LIST   -57

                                /* low level functions*/
void   _far   fortran set_twp_file_name( char _far *name );
char   _far * fortran get_twp_file_name( void );

int    _far   fortran _open_twp_file( int open_mode_flag );
void   _far   fortran close_twp_file( void );

size_t _far   fortran read_twp( void _far *outbuff, size_t size );
size_t _far   fortran write_twp( void _far *outbuff, size_t size );

size_t _far   fortran fseek_twp( long lOffset, int nPosition );
size_t _far   fortran fgetpos_twp( fpos_t *lOffset );

size_t _far   fortran write_twp_CR( void );

                                /* meddle level functions*/
int    _far  _fortran ReadTWP_Header( TWP_HEADER _far *thBuffer );
int    _far  _fortran WriteTWP_Header( TWP_OFFSET_TABLE _far *TWP_BlockTable,
                                       TWP_INP_TABLE    _far *TWP_InputTable,
                                       TWP_LIMIT_TABLE  _far *TWP_LimitTable  );

int    _far  _fortran GenTWP_MainTable( TWP_TABLE        _far *lpTWP_Table,
                                        TWP_OFFSET_TABLE _far *lpTWP_BlockTable,
                                        TWP_INP_TABLE    _far *lpTWP_InputTable,
                                        TWP_LIMIT_TABLE  _far *lpTWP_LimitTable  );

int    _far   fortran ReadTWP_BlockTable( TWP_OFFSET_TABLE _far *lpTWP_OffsetTable );
int    _far  _fortran WriteTWP_BlockTable( TWP_OFFSET_TABLE _far *lpTWP_OffsetTable );
int    _far  _fortran UpdateTWP_BlockTable( TWP_OFFSET_TABLE _far *lpTWP_OffsetTable );

int    _far   fortran ReadTWP_InputTable( TWP_INP_TABLE _far *lpTWP_InputTable );
int    _far  _fortran WriteTWP_InputTable( TWP_INP_TABLE    _far *lpTWP_InputTable );

int    _far  _fortran ReadTWP_LimitTable( TWP_LIMIT_TABLE  _far *lpTWP_LimitTable );
int    _far  _fortran WriteTWP_LimitTable( TWP_LIMIT_TABLE  _far *lpTWP_LimitTable );
int    _far  _fortran UpdateTWP_LimitTable( TWP_LIMIT_TABLE    _far *lpTWP_LimitTable );

int    _far  _fortran ReadTWP_Tables( char             _far *lpszTWP_Name,
                                      TWP_OFFSET_TABLE _far *lpTWP_OffsetTable,
                                      TWP_INP_TABLE    _far *lpTWP_InputTable,
                                      TWP_LIMIT_TABLE  _far *lpTWP_LimitTable );

int    _far  _fortran WriteTWP_Tables( char             _far *lpszTWP_Name,
                                       TWP_OFFSET_TABLE _far *lpTWP_OffsetTable,
                                       TWP_INP_TABLE    _far *lpTWP_InputTable,
                                       TWP_LIMIT_TABLE  _far *lpTWP_LimitTable );


                                /* 3D topology TWP functions*/
#if defined TMC_TWP || defined _MODEL_W || defined _MODEL_3D

int    _far   fortran write_3d_twp( WORD wBlockNumber );

#endif

#endif
