#ifndef _TYPERTH_H
  #define _TYPERTH_H

#define _PREC_DOUBLE
//#define _PREC_FLOAT

#ifdef _PREC_DOUBLE
#define _real double
#endif

#ifdef _PREC_FLOAT
#define _real float
#endif

#define _ELEM_VAL_RTH _real
#define _ELEM_VAL_RTH_MULT 1

#define   C0___ 299792458
#define  MU0___ (12.566370614e-7)
#define EPS0___ (8.854187817e-12)
#define   PI___ (3.141592653589)

#define   ELECTRON_Q___ (1.602176565e-19)
#define   ELECTRON_M___ (9.10938356e-31)

void PutTrace( CString csTrace );
void PutTrace( char *szTrace );
void PutStatistics( CString csTrace );
void PutStatistics( char *szTrace );
void PutSinchronizFlag( BOOL bTopologyFlag, BOOL bFieldFlag1, BOOL bSinchFlag1 );
void PutModel( void );
CString GetModel( void );

void ReadSystemType( void );

void BeepStepEnd( int i );
void BeepAllEnd( void );
void s_alarm( int code );
void s_play( int *ptr );
void s_tone( int freq, int time );
void SetMelody1Interval( int i );
int GetMelody1Interval( void );
void SetMelody1( int *i );
void GetMelody1( int *i );
void SetMelody2Interval( int i );
int GetMelody2Interval( void );
void SetMelody2( int *i );
void GetMelody2( int *i );
void SetDeltaT( char *lpszDeltaT );
void SetDeltaT( double dDeltaT1 );

BOOL IsPointInTr( double x1, double y1, double x2, double y2, double x3, double y3, double x0, double y0 );
UINT RunStepGlobal( LPVOID pParam );
UINT RunAllGlobal( LPVOID pParam );

#endif
