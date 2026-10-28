/*
**  TAMIC_soft(R) 1991 v1.00
**
**  Frequency data table declaration
**
**  FILE : a:\include\frqdata.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _FRQDATA_H
 #define _FRQDATA_H


                                                 /* frequency struct        */
typedef struct
           {
            int num;                             /* current frequency point */
            int max;                             /* maximal frequency point */
            _real *f;                       /* frequency point array   */
            int code;                            /* unit code               */
            _real unit[4];                       /* unit                    */
            char *item[4];                  /* unit items strings      */
            }FREQUENCY;

enum freq { _Hz, _KHz, _MHz, _GHz };             /* units numbers           */
 /* return first frequency array element (F0) */
 /* return frequency increement (dF) */

int   get_frq_nf( void );
 /* return max. frequency point number */

int   set_frq_nf( int num ); /* == f[index] in Hz !!!*/

void  set_frq_element( _real fValue, int num );  /* f[num] = fValue*/

int   set_frequency( _real f0, _real df, int nf );

char  * get_frq_item( int num );
_real get_frq_unit( int num );
int   get_frq_item_code( void );

int   get_current_frq_num( void );
void  set_current_frq_num( int number );
double get_current_dbl_frq( void );

#endif
