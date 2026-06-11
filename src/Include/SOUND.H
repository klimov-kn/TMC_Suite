/*
**  TAMIC_soft(R) 1989 v1.00     12/01/89 03:54pm
**
** Macros and decloration for Tamic_H programm.
**
** FILE: a:\include\sound.h
*/

#define GLUK     0
#define BEEP     1

/* Enumeration of notes and frequencies     */
enum NOTES
   {
    A0 = 44,
    C1 = 52, D1 = 58, D1d = 62, E1 = 66, F1 = 69, G1 = 78, A1 = 88, B1 = 98,
    EIGHTH = 31, QUARTER = 62, QUARTER_POINT = 93, HALF = 125, HALF_POINT = 188, WHOLE = 250,
    END = 0
   };

void far s_play( int far *ptr );
void far s_tone( int freq, int time);
void far DELAY( unsigned int );
void far s_alarm( int code );

