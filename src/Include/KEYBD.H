/*
** TAMIC_soft (R)  1989.
**
** Definition and declaration for keyboard functions.
**
** FILE: a:\include\keybd.h
**
*/

#ifndef _KEYBD_H
 #define _KEYBD_H

        /* declaration for kb_gettagl */
#define R_SHIFT 1
#define L_SHIFT 2
#define CNTRL   4
#define ALT     8
#define SCRLOC  0x10
#define NUMLOC  0x20
#define CAPLOC  0x40
#define INS     0x80
#define ALL     0xFF

#define _ENTER        13
#define _SPACE_BLANK  32

#define _ESC    0x1b
#define _BACKSP 0x08

#define _HOME   -71
#define _UP     -72
#define _PGUP   -73
#define _LEFT   -75
#define _CENTER -76
#define _RIGHT  -77
#define _END    -79
#define _DOWN   -80
#define _PGDN   -81
#define _INS    -82
#define _DEL    -83

#define _PLUS        ('+')
#define _MINUS       ('-')

#define        _TABL    9
#define  _SHIFT_TABL  -15

#define        _F1    -59
#define    _ALT_F1   -104
#define  _SHIFT_F1    -84

#define        _F2    -60
#define    _ALT_F2   -105

#define        _F3    -61
#define    _ALT_F3   -106

#define        _F4    -62
#define    _ALT_F4   -107
#define  _SHIFT_F4    -87
#define  _CNTRL_F4    -97

#define        _F5    -63
#define  _SHIFT_F5    -88
#define    _ALT_F5   -108

#define        _F6    -64
#define  _SHIFT_F6    -89
#define    _ALT_F6   -109

#define        _F7    -65
#define  _SHIFT_F7    -90
#define    _ALT_F7   -110

#define        _F8    -66
#define  _SHIFT_F8    -91
#define    _ALT_F8   -111

#define        _F9    -67
#define  _SHIFT_F9    -92
#define    _ALT_F9   -112

#define       _F10    -68
#define _SHIFT_F10    -93
#define _CNTRL_F10   -103
#define   _ALT_F10   -113

#define _ALT_1 -120
#define _ALT_2 -121
#define _ALT_8 -127

#define _CNTRL_MINUS 31
#define _ALT_MINUS -130
#define _ALT_PLUS  -131

#define _ALT_A -30
#define _ALT_S -31
#define _ALT_D -32
#define _ALT_F -33
#define _ALT_G -34
#define _ALT_H -35
#define _ALT_J -36
#define _ALT_K -37
#define _ALT_L -38

#define _ALT_Q -16
#define _ALT_W -17
#define _ALT_E -18
#define _ALT_R -19
#define _ALT_T -20
#define _ALT_Y -21
#define _ALT_U -22
#define _ALT_I -23
#define _ALT_O -24
#define _ALT_P -25

#define _ALT_Z -44
#define _ALT_X -45
#define _ALT_C -46
#define _ALT_V -47
#define _ALT_B -48
#define _ALT_N -49
#define _ALT_M -50

                                      /* function prototyps           */
void  _far kb_m_pause( void );

int   _far dos_ok( void );
int   _far kb_gettugl( int mask);
int   _far kb_getch( void );
int   _far kb_inkey( void );
short _far kb_status( void );
short _far kb_read( void );


#ifdef _NDP2

/* keyboard services */

#define _KEYBRD_READ		0	/* read next character from keyboard */
#define _KEYBRD_READY		1	/* check for keystroke */
#define _KEYBRD_SHIFTSTATUS	2	/* get current shift key status */

/* services for enhanced keyboards */

#define _NKEYBRD_READ		0x10	/* read next character from keyboard */
#define _NKEYBRD_READY		0x11	/* check for keystroke */
#define _NKEYBRD_SHIFTSTATUS	0x12	/* get current shift key status */

/*int _bios_keybrd( int );*/
#endif

#endif
