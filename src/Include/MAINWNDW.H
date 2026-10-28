/*
**  TAMIC_soft(R) 1991 v1.00
**
** Macros and declarations for mainwndw program.
**
** FILE: a\include\mainwndw.h
**
*/

#ifndef _MAINWNDW_H
  #define _MAINWNDW_H








/* ETIME collides with the ETIME macro from modern <errno.h> (UCRT);
   undef it before the enum so the enumerator compiles. win32/win64 build fix. */
#ifdef ETIME
#undef ETIME
#endif

enum ST_ITEMS
           {
            SFILE,
            STEP,
            FREQ,
            ST_LINK,
            ST_BLOCK,
            TRACE,
            ETIME,
            ESTIME,
            FREEMEM
            };


#define put_trace( string)   stat_messege( TRACE, string )

void stat_messege( int stat_item_num, char *string);

  int       get_dos_freemem( void );

#endif
