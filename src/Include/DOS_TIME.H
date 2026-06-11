/*
**  TAMIC_soft(R) 1989,1992 v1.00
**
**  DOS TIME: functions decloration.
**
**  FILE: a:\include\dos_time.h
**
*/

#ifndef _DOS_TIME_H
  #define _DOS_TIME_H


#define ALL_TIME_LONG   12
#define ALL_DATA_LONG   22

#ifndef _DATETIME_T_DEFINED      /* it is need hear to compatibility with NDP 2.1*/

#ifndef _NDP2

struct dosdate_t {
        unsigned char  day;          /* 1-31 */
        unsigned char  month;        /* 1-12 */
        unsigned short year;         /* 1980-2099 */
        unsigned char  dayofweek;    /* 0-6, 0=Sunday */
	};

struct dostime_t {
        unsigned char hour;     /* 0-23 */
	unsigned char minute;	/* 0-59 */
	unsigned char second;	/* 0-59 */
	unsigned char hsecond;	/* 0-99 */
	};

#define _DATETIME_T_DEFINED

#endif

#endif

struct time_hs {
		 unsigned long current;
		 unsigned long start;
		 unsigned long elapsed;
		 unsigned long estimated;
		 };

void far fortran init_t_hs( void );
char far        *dos_date( char far *string, struct dosdate_t far *dd);
char far        *dos_time( char far *string, struct dostime_t far *dt);
unsigned long far conv_to_hs( struct dostime_t far *dt);
struct dostime_t far *conv_to_st( unsigned long hsecond,
						  struct dostime_t far *dt);
struct dostime_t far *get_elapsed_time( struct dostime_t far *dt );

void _cdecl _dos_getdate(struct dosdate_t *);
void _cdecl _dos_gettime(struct dostime_t *);
#endif
