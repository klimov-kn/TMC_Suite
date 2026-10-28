/*
**  TAMIC_soft(R) 1991 v1.00 01/09/91 10:10am
**
**  frequency data table.
**
**  FILE : ..\tamic\indan\frqdata.c
**
**  LIB  : rindan_X.lib
**
**  OVERLAY: NO
**
*/

#include <stdio.h>

#include <typedef.h>
#include <tmclimit.h>
#include <frqdata.h>
#include <error1.h>


static _real frq_buff[FREQ_MAX_NUM];  /*buffer for frequency */

static FREQUENCY frq = { 0,                    /* current frequency point  */
                         0,                    /* maximal frequency point  */
                         frq_buff,             /* frequency point array    */
                         3,                    /* current unit code        */
                         {(_real)1.0,          /* unit for Hz              */
                          (_real)1.0e3,        /* unit for KHz             */
                          (_real)1.0e6,        /* unit for MHz             */
                          (_real)1.0e9},       /* unit for GHz             */
                         { "_Hz",               /* unit items string for Hz */
                           "KHz",              /* unit items string for KHz*/
                           "MHz",              /* unit items string for MHz*/
                           "GHz"} };           /* unit items string for GHz*/

int set_frequency( _real f0, _real df, int nf )
 {
  int num;

  frq.max = nf;
  f0 *= frq.unit[frq.code];
  df *= frq.unit[frq.code];

  for( num = 1, frq.f[0] = f0; num < frq.max; num++ )
   frq.f[num] = frq.f[num-1] + df;

  return 0;
 }






int   get_frq_nf( void ) { return( frq.max ); }
int   set_frq_nf( int nMax )
  {
   if( ( nMax <= 0 )||( nMax > FREQ_MAX_NUM ) )
    {
     put_error_messege( BAD_NF_PARM, nMax );
     return -1;
    }
   frq.max = nMax;
   return 0;
  }

char  * get_frq_item( int num ) { return( frq.item[num] ); }
_real get_frq_unit( int num ) { return frq.unit[num]; }
int   get_frq_item_code( void ) { return frq.code; }
void  set_frq_element( _real fValue, int index ) { frq.f[index] = fValue; }
double get_current_dbl_frq( void ) { return( (double)frq.f[frq.num] ); } /* dummy for writing */
int   get_current_frq_num( void ) { return frq.num; }
void  set_current_frq_num( int num ) { frq.num = num; }

