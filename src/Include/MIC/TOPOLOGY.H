/*
**  TAMIC_soft(R) 1992 vR1.02
**
**  Definitions and declarations for Topology section.
**
** FILE: \include\mic\topology.h
**
*/

#ifndef _TOPOLOGY_H
 #define _TOPOLOGY_H

#ifndef MAX_WINDOW_NUM
 #define MAX_WINDOW_NUM    20     /*max number of regions in one input*/
#endif

#ifndef _FWINDOW
  #define _FWINDOW

#define CDF_MIXED   0
#define CDF_ALONG_X 1
#define CDF_ALONG_Y 2

typedef struct
  {
   _real rLeft;
   _real rBottom;
   _real rRight;
   _real rTop;
   } FWINDOW;

typedef struct
  {
   int nIL;
   int nJB;
   int nIR;
   int nJT;
   } NWINDOW;

#endif

typedef struct
 {
  int     nCurDir;             /* current direction*/
  int     nN, nM;              /* currents number reduction coefficients*/
  char    szFile[FILENAME_MAX]; /* surface impedance( file or attribute name or value )*/
  FWINDOW fwLBRT;              /* Left Bottom Right Top*/
  }RECT;


#define LINE_ALONG_X  1
#define LINE_ALONG_Y  2

typedef struct
 {
  int     nInputNum;              /* input number*/
  int     nCurDir;                /* current direction*/
  int     nLineDir;               /* adjacent line direction*/
  _real   rRef;                   /* reference plane shift*/
  int     nMode;                  /* output mode number*/
  char    szFile[FILENAME_MAX];   /* file name*/
  int     nWindowNum;             /* windows number in the input*/
  FWINDOW fwLBRT[MAX_WINDOW_NUM]; /* Left Bottom Right Top*/
  }INPUT;

typedef struct
 {
  int    nLumpedNum;             /* lumped element number*/
  int    nCurDir;                /* current direction*/
  char   szFile[FILENAME_MAX];   /* impedance( file or attribute name )*/
  FWINDOW fwLBRT;                /* Left Bottom Right Top*/
  }LUMPED;

struct poly_point
 {
  _real  rX, rY;                      /* point coordinates*/
  struct poly_point _far *lpNext;     /* pointer to next point*/
  };
typedef struct poly_point POLY_POINT;

struct polygon
 {
  int     nCurDir;              /* current direction*/
  int     nN, nM;               /* currents number reduction coefficients*/
  int     nPointNum;            /* number of points*/
  char    szFile[FILENAME_MAX]; /* surface impedance( file or attribute name or value )*/
  POLY_POINT _far *lpPoint;     /* pointer to polygon points*/
  };
typedef struct polygon POLYGON;

struct tagObject
 {
  int                    nCode;     /* object code*/
  void             _far *lpObject;  /* pointer to entity structure*/
  struct tagObject _far *lpNext;    /* pointer to next OBJECT*/
  struct tagObject _far *lpPrevios; /* pointer to previous OBJECT*/
  };
typedef struct tagObject OBJECT;

struct tagTLayer
 {
  int nNumber;                      /* layer number*/
  OBJECT _far *lpFirstObject;       /* pointer to first object*/
  OBJECT _far *lpLastObject;        /* pointer to last object*/
  struct tagTLayer _far *lpNext;    /* pointer to next TLAYER*/
  struct tagTLayer _far *lpPrevios; /* pointer to previous TLAYER*/
  };

typedef struct tagTLayer TLAYER;


typedef struct
 {
  TLAYER _far *lpFirst;      /* pointer to First layer in topology*/
  TLAYER _far *lpLast;       /* pointer to Last layer in topology*/
  } TOPOLOGY;



void   _far   _fortran SetFirstTLayerPointer( TLAYER _far *lpFirstTLayer );
void   _far   _fortran SetLastTLayerPointer( TLAYER _far *lpLastTLayer );

TLAYER _far * _fortran GetFirstTLayerPointer( void );
TLAYER _far * _fortran GetLastTLayerPointer( void );

INPUT      _far * _fortran Get_INPUT( int nInp_num );

int _far _fortran FW_to_NW( FWINDOW _far *fwWin, NWINDOW _far *nwWin, int nCurDir );

#endif
