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
            _real _far *f;                       /* frequency point array   */
            int code;                            /* unit code               */
            _real unit[4];                       /* unit                    */
            char _far *item[4];                  /* unit items strings      */
            }FREQUENCY;

enum freq { _Hz, _KHz, _MHz, _GHz };             /* units numbers           */


_real _far   _fortran get_frq_f0( int unit_flag );
 /* return first frequency array element (F0) */

_real _far   _fortran get_frq_df( int unit_flag );
 /* return frequency increement (dF) */

int   _far   _fortran get_frq_nf( void );
 /* return max. frequency point number */

int   _far   _fortran set_frq_nf( int num );
 /* set max.  frequency point number*/

_real _far   _fortran get_frq_element( int index ); /* == f[index] in Hz !!!*/

void  _far   _fortran set_frq_element( _real fValue, int num );  /* f[num] = fValue*/
void  _far   _fortran set_frq_element_unit( _real fValue, int index );

int   _far   _fortran set_frequency( _real f0, _real df, int nf );
int   _far   _fortran set_frequency_arr( _real _far *ftmp, int nf );

char  _far * _fortran get_frq_item( int num );
char  _far * _fortran get_current_frq_item( void );
_real _far   _fortran get_frq_unit( int num );
void  _far   _fortran set_frq_item_num( int num );
int   _far   _fortran get_frq_item_code( void );

int   _far   _fortran get_current_frq_num( void );
void  _far   _fortran set_current_frq_num( int number );
_real _far   _fortran get_current_frq( void );
double _far  _fortran get_current_dbl_frq( void );
FREQUENCY _far   * _fortran GetPointerToFrq( void );

#endif
