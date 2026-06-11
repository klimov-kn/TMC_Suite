/*
**  TAMIC_soft(R) 1992 vR1.02
**
**  Definitions and declarations for Package section.
**
** FILE: \include\mic\package.h
**
*/

#ifndef _PACKAGE_H
 #define _PACKAGE_H

typedef struct         /* region AxB*/
 {
  _real a;
  _real b;
  } REGION;


typedef struct
 {
  int nTypeFlag;   /* Window type flag( electric, magnetic or open )*/
  int nN, nM;      /* mode number N and M for Top or Bottom Window*/
  } TOP_BOTTOM_W;

struct tagSlice
 {
  int                  nCode;         /* object code( 2 - media, 3 - layer )*/
  void _far            *lpSlice;      /* pointer to object*/
  struct tagSlice _far *lpNext;       /* linked list pointer to next*/
  struct tagSlice _far *lpPrevios;    /* linked list pointer to previous*/
  };

typedef struct tagSlice SLICE;

typedef struct
 {
  REGION             rgnAB;         /* region AxB*/
  Uint               nSideWalls;    /* Side Walls integer flag*/
  TOP_BOTTOM_W       tbwTop;        /* Top Window*/
  TOP_BOTTOM_W       tbwBottom;     /* Bottom Window*/
  SLICE        _far *lpFirst;       /* pointer to first element in linked list*/
  SLICE        _far *lpLast;        /* pointer to last element in linked list*/
  } PACKAGE;

typedef struct
 {
  _real rHeight;          /* Media layer height*/
  char szEpsilon[14];     /* Buffer for epsilon's attribute*/
  char szMu[14];          /* Buffer for mu's attribute*/
  } MEDIA;

typedef struct
 {
  int nNumber;            /* Layer's number*/
  int nType;              /* Layer's type*/
  } LAYER;

#define PF_TOP_W      0         /* Package Flag top window*/
#define PF_BOTTOM_W   1         /* Package Flag bottom window*/

#define PF_MEDIA      2         /* Media slice code*/
#define PF_LAYER      3         /* Layer slice code*/

#define WF_ELECTRIC   0         /* Wall of electric type*/
#define WF_MAGNETIC   1         /* Wall of magnetic type*/
#define WF_OPEN       2         /* open boundary*/

#define SF_ELECTRIC   0         /* Sidewalls of electric type*/
#define SF_OLINER     1         /* Two opposite sidewalls of electric type*/
                                /* and two other of magnetic( north and south )*/
#define SF_MAGNETIC   2         /* Sidewalls of magnetic type*/
#define SF_SYM        3         /* noth sidewall is magnetic, all other are electric*/
#define SF_ANTISYM    4         /* west sidewall is electric, all other are magnetic*/

#define LF_STRIP      0         /* Strip type of layer metallization*/
#define LF_SLOT       1         /* Slot type of layer metallization*/

void  _far _fortran SetPackageRegionA( _real rA );
void  _far _fortran SetPackageRegionB( _real rB );

_real _far _fortran GetPackageRegionA( void );
_real _far _fortran GetPackageRegionB( void );


void  _far   _fortran SetSideWallsFlag_0( int nSideWallsFlag );
int   _far   _fortran GetSideWallsFlag_0( void );

void         _far   _fortran SetTopBottomWindow_0( int nWindowType, TOP_BOTTOM_W _far *tbwBuffer );
TOP_BOTTOM_W _far * _fortran GetTopBottomWindow_0( int nWindowType );
int          _far   _fortran GetTopBottomFlag( int nWindowType );

void  _far   _fortran SetFirstSlicePointer( SLICE _far *lpFirstSlice );
void  _far   _fortran SetLastSlicePointer( SLICE _far *lpLastSlice );

SLICE _far * _fortran GetFirstSlicePointer( void );
SLICE _far * _fortran GetLastSlicePointer( void );


int   _far _fortran SetPackageRegion( _real rA, _real rB );
void  _far _fortran GetPackageRegion( _real _far *rA, _real _far *rB );

int   _far _fortran SetSideWallsFlag( int nSideWallsFlag );
int   _far _fortran SetTopBottomWindow( int nWindowType, TOP_BOTTOM_W _far *tbwBuffer );
int   _far _fortran SetMedia( MEDIA _far *medBuffer );
int   _far _fortran SetLayer( LAYER _far *layBuffer );

int   _far _fortran CheckPackage( void );

void  _far _fortran Set_layer_num( int nNumber );
int   _far _fortran Get_layer_num( void );
void  _far _fortran Set_media_layer_num( int nNumber );
int   _far _fortran Get_media_layer_num( void );
void  _far _fortran Get_media_layer( int nNum, _real _far *rHeight, _real _far *rEpsilon, _real _far *rMu );
int _far _fortran Get_layer_type( int nLayNum );


#endif
