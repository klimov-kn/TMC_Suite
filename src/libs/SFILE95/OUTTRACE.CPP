/*
 *  TAMIC_soft(R) 1991,1992 vR1.01
 *
 *  OutTrace          - output message to last row of screen
 *
 *  FILE : \source\utility\outtrace.c
 *
 *  OVERLAY: NO
*/

#include <stdio.h>
#include <string.h>
#include <dos.h>
//#include <bios.h>

#include <typedef.h>


static void _near _fortran t_gotoxy( int x, int y) ;

void _far OutTrace( char _far *TraceMessage)
  {
   char buffer[80];
   int nStrlen;
   register i;

   nStrlen = sprintf( buffer, TraceMessage );
   for( i = nStrlen; i < 79; i++ ) buffer[i] = ' ';
   buffer[79] = '\0';
   t_gotoxy( 1, 25 );
   printf( buffer );
   t_gotoxy( nStrlen+1, 25 );

   }


/*   Set cursor to position x - col, y - row at zero page  */
/*

static void _near _fortran t_gotoxy( int x, int y)
  {
    union REGS inregs,outregs;
    short t_curr_activepage = 0;

    inregs.h.ah = 2;
    inregs.h.dl = (Uchar)--x;
    inregs.h.dh = (Uchar)--y;
    inregs.h.bh = (Uchar)t_curr_activepage;

    int86(0x10,&inregs,&outregs);

    }

*/
static void _near _fortran t_gotoxy( int x, int y)
  {
	char ch;
	ch = 13;
	printf("%c", ch);
//	getch();
    }
