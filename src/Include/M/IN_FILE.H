/*
**  TAMIC_soft(R) 1989,1990 v1.00
**
**  in_mfile.h
**
**  FILE : ..\include\indan\in_mfile.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _IN_FILE_H
 #define _IN_FILE_H

 typedef struct               // Y- matrix  file_name
	  {
	   char szFileName[ FILENAME_MAX ]; //
	   } BE_FILE;

 typedef struct
	  {
	   _real           Frq;                          // Frequence
	   int           nLine;                          // Number of conductor
	   _complex _far *lpxBet;                          // data ptr
	   } BET_FILE_LINE;

 typedef struct
	  {
	   int         nSection;                          // Number of section
	   _real      *lpYcoord;                          // pointer on input Data
	   } TOP_FILE_LINE;

 typedef struct
	  {
	   _complex _far  *lpxBet;                         // pointer on freq
	   _complex _far   *lpxC1;                         // pointer on coef.
	   _complex _far   *lpxC2;                         // pointer on coef.
	   _complex _far   *lpxC3;                         // pointer on coef.
	   } BET_INTRP;

 typedef struct
	  {
	   int                    nFreq;                         // Number of freq
	   _real     _far       *lprFrq;                         // pointer on freq
	   int                    nLine;                         // Number of mode
	   BET_INTRP _far  *lpBetFluent;
	   } INTRP_CBET;

 typedef struct
	  {
	   char                file_name[ FILENAME_MAX ]; // text file name
	   _real               rLenLine;                  // length of line
	   int                 nLayer;                    // number of layers
	   TOP_FILE_LINE _far *lpTData;                   // Topology data
	   int                 nFreq;                     // number of frq
	   BET_FILE_LINE _far *data;                      // data ptr
	   int                 nFlagCurFreq;              // flag currentf frq == 0 if CurBetData is full
	   BET_FILE_LINE       CurBetData;                // data current frq
	   INTRP_CBET          intrp_bet;
	   } FILE_LINE;


 int _far _fortran in_M_File( char _far *lpszFilename, ELEMENT _far *lpElement);
 int _far _fortran alloc_M_FILE( ELEMENT _far *lpElement );
 _ELEM_VAL _far *_fortran y_file_tmc_m( ELEMENT _far *lpElement, FILE_TXT _far *lpFl );
 _ELEM_VAL _far *_fortran y_flin_tmc_m( ELEMENT _far *lpElement, FILE_LINE _far *lpFl );
 int  _far _fortran y_flin_tmc_mm( ELEMENT _far *lpElement, FILE_LINE _far *lpFl );
 int  _far  fortran in_M_File_line( char _far *lpszStr, ELEMENT _far *lpElement );
 int  _far _fortran read_rgf_data( char _far *lpszFile_name, INPUT_DATA _far *lpId, FILE_LINE _far *lpFl);
 int _far  _fortran find_flin_intrp_cbet( FILE_LINE _far *lpFl );

 #define RGF_LABEL        "RGF"
 #define RGF_VERSION       212

#endif
