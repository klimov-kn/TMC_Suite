/*
**  TAMIC_soft(R) 1991,1992 vR1.01
**
**  C structures corresponding to FORTRAN COMMON blocks for IC program.
**
**  FILE : \include\mic\str_com.h
**
*/

#include <limit_ic.h>

/* structure for COMMON block ALL_IJXY */
typedef struct
         {
          int KARX;
          int IXNX[MAX_X_REG];
          int IXKX[MAX_X_REG];
          int JYNX[MAX_X_REG];
          int JYKX[MAX_X_REG];
          int ICX[MAX_X_REG];
          int KARY;
          int IXNY[MAX_Y_REG];
          int IXKY[MAX_Y_REG];
          int JYNY[MAX_Y_REG];
          int JYKY[MAX_Y_REG];
          int ICY[MAX_Y_REG];
          }S_ALL_IJXY;

/* structure for COMMON block INPXY */
typedef struct
          {
            int  NIX;
           _real ROX[MAX_X_INP*MAX_FREQ_NUM];
           _real GOX[MAX_X_INP*MAX_FREQ_NUM];
           _real FAX[MAX_X_INP*MAX_FREQ_NUM];
            int  INPX[MAX_X_INP];
            int  NIY;
           _real ROY[MAX_Y_INP*MAX_FREQ_NUM];
           _real GOY[MAX_Y_INP*MAX_FREQ_NUM];
           _real FAY[MAX_Y_INP*MAX_FREQ_NUM];
            int INPY[MAX_Y_INP];
           }S_INPXY;

/* structure for COMMON block ZXY */
typedef struct
          {
            int  NZX;
           _real ZXR[MAX_X_LUM*MAX_FREQ_NUM];
           _real ZXI[MAX_X_LUM*MAX_FREQ_NUM];
            int  NZY;
           _real ZYR[MAX_Y_LUM*MAX_FREQ_NUM];
           _real ZYI[MAX_Y_LUM*MAX_FREQ_NUM];
           }S_ZXY;

/* structure for COMMON block FREQ */
typedef struct
          {
            int FREQ_NUM;
           _real FR[MAX_FREQ_NUM];
           }S_FREQ;

/*structure for common block NET */
typedef struct
          {
            int N1,M1;
           _real DX,DY,A,B;
           }S_NET;

/*structure for common block MED */
typedef struct
          {
           _real E1,E2,H1,H2;
           }S_MED;

/*structure for common block CALC */
typedef struct
       {
        int   g;    // calculate and write general sums of GGF eigenvalues
        int   u;    // calculate and write unfolded multiport description matrix
        int   l;    // calculate line characteristics and input parameters
        int   d;    // discontinuity analysis
        }S_CALC;

/*structure for common block LIST_PRI */
typedef struct
          {
           int t;   // print echo of *.tpl file
           int d;   // print internal data base in *.tpl format 
           int s;   // print symbolical currents distribution
           int u    // print immitance matrix of unfolded multiport
           }S_LIST_PRI;


/*structure for common block SM */
typedef struct
          {
           int ISIM,IREM;
           }S_SM;

/*structure for common block BR */
typedef struct
          {
           int BREAK;
           }S_BR;



/*structure for common block SIM_GR */
typedef struct
          {
           long GRAF[120],GR[6],GRB[4],IB[4],GRZ[4],IZ[4],IC,IR;
           }S_SIM_GR;