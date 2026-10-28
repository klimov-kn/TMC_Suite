/*
**  TAMIC_soft(R) 1989,1990,1991 v1.00
**
** Macros and declarations for Tamic_H graphics program
**
** FILE: a:\include\tmcgraph.h>
**
*/

#ifndef _TMC_GRPH_H
  #define _TMC_GRPH_H


#if __STDC__
#define _Cdecl
#else
#define _Cdecl	cdecl
#endif

#if     !defined(__GRAPHX_DEF_)
#define __GRAPHX_DEF_


/* Colors for setpalette and setallpalette */

#if	!defined(__COLORS)
#define __COLORS


#endif







#define MAXCOLORS 15

struct palettetype {
        unsigned char size;
	signed char colors[MAXCOLORS+1];
};
void  _Cdecl bar( int left, int top, int right, int bottom );
void   _Cdecl circle( int x, int y, int radius);
void   _Cdecl line(int x1, int y1, int x2, int y2);
void   _Cdecl rectangle(int left, int top, int right, int bottom);
void   _Cdecl setallpalette(struct palettetype *palette);
void   _Cdecl setpalette(int colornum, int color);


#endif

#ifndef _VIDEOCONFIG_DEFINED

#define _VIDEOCONFIG_DEFINED
#endif







#ifndef _MENU_STRUCT
 struct menu_items
          {
           char *item;        /* pointer to item string*/
	   int position;      /* position in string of active char*/
           int add_key;       /* aditional active char key*/
           int line;          /* line after item drow if 1*/
           int exit_code;     /* 0 stay menu at screen,
                                 1 just close menu          */
           int help_page;     /* help_page number*/
           };

 struct alt_colors
           {
            int Normal;
            int Select;
            };

 struct color_menu_attrib
           {
            struct alt_colors act_item;
            struct alt_colors no_act_item;
            struct alt_colors bg_item;
            int BorderColor;
            int BarColor;
            };

 struct menu_attrib
           {
            char *titel;
            int fgTitel;
            struct color_menu_attrib *c_attr;
            int position;
            int border_type;   /* border type code*/
            int menu_code;
            int curr;
            void *screen_buff;
            };

 struct window
          {
           int left;
           int top;
           int right;
           int bottom;
           };
 typedef struct window WINDOW;

#define _MENU_STRUCT
#endif

int bar_menu( int col, int row, struct menu_items *mi, struct menu_attrib *mA );


#ifndef _FLOAT_WINDOW
 #define _FLOAT_WINDOW


#endif


#ifndef _FLOAT_POINTTYPE
  #define _FLOAT_POINTTYPE


#endif



#ifndef _STRING_EDIT_STRUCT
  #define _STRING_EDIT_STRUCT


#endif


#ifndef _ACTIVE_CHAR_STRUCT
  #define _ACTIVE_CHAR_STRUCT


#endif



#ifndef _WINDOW_MENU_STRUCT
  #define _WINDOW_MENU_STRUCT


#endif

int  bar_menu( int col, int row, struct menu_items *mi, struct menu_attrib *mA );


/* marker functions*/
                                    /* marker size */

#ifndef _MARKER
  #define _MARKER

typedef struct
            {
             int type;
             int width;
             int color;
             int size;
             } MARKER;
#endif



      /* button section*/

struct _button
	 {
	  int x, y;
	  char *str;
	  };
typedef struct _button BUTTON;


void button( int x, int y, char *str, int flag );

#endif

