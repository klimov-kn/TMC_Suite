/*
**   text.h
**
**  Text definition and declaration
**
*/

#ifndef _TEXT_H
  #define _TEXT_H


struct mnuAtr
  {
   int fgNormal, fgSelect, fgBorder;
   int bgNormal, bgSelect, bgBorder;
   int position;
   int border_type;
   int menu_code;
   int curr;
   void *screen_buff;
   };

enum pos{
	 LEFT,
	 CENTERED,
	 RIGHT};

#ifndef _MENU_STRUCT
 struct menu_items
	  {
	    char *item;        // pointer to item string
	   int position;      // position in string of active char
	   int add_key;       // aditional active char key
	   int line;          // line after item drow if 1
	   int exit_code;     /* 0 stay menu at screen,
				 1 just close menu          */
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
	    struct color_menu_attrib _far *c_attr;
	    int position;
	    int border_type;   // border type code
	    int menu_code;
	    int curr;
	    void far *screen_buff;
	    };

 struct window
	  {
	   int left;
	   int top;
	   int right;
	   int bottom;
	   };
 typedef struct window WINDOW;

 struct messege_menu
	    {
	     char _far *item;
	     char letter;
	     struct window mw;
	     };

#define _MENU_STRUCT
#endif

typedef struct window TEXTWINDOW;


				/* color management */                                                                        /* colors and function to set colors */
#if     !defined(__COLORS)
#define __COLORS

enum COLORS {
	BLACK,                  /* dark colors */
	BLUE,
	GREEN,
	CYAN,
	RED,
	MAGENTA,
	BROWN,
	LIGHTGRAY,
	DARKGRAY,               /* light colors */
	LIGHTBLUE,
	LIGHTGREEN,
	LIGHTCYAN,
	LIGHTRED,
	LIGHTMAGENTA,
	YELLOW,
	WHITE
};
#endif

#define A_STACK  20

struct attr_stack
      {
       Uchar attr[A_STACK];
       char *a_ptr;
       };

				/* function to set colors */
unsigned char t_settextcolor(int color);
unsigned char t_setbackgrcolor(int color);
unsigned char t_settextattr( int fgColor, int bgColor);
int t_save_attr( void);
int t_reset_attr( void);

#define NO_BLINK   0
#define YES_BLINK  1

void _far t_set_blink_mode( char blink );

				/* text primitives  "char", "line", "body" */
struct charandatr
	   {
	    unsigned char c;
	    unsigned char atr;
	    };
typedef struct charandatr CHARANDATR;

#define t_putchxy( x, y, ch)  t_horline( x, y, ch, 1)
void _far t_horline(int x, int y, Uchar c, int num_of_char);
#define t_verline( x, y, c, num_of_char )  t_body( x, y, x, y+num_of_char-1, c );
void _far t_body(int left, int top, int right, int bottom, Uchar c);

				/* shar and attribute     */
void _far t_putcharandatr(struct charandatr _far *charatr, int numb_of_char);
void _far t_getcharandatr(struct charandatr _far *charatr);

				/* cursor manegement      */
#define C_STACK   20
#define SH_STACK  20

#define T_CURSOR_OFF 0x2020
#define T_CURSOR_ON  0x0607
#define T_CURSOR_BIG 0x0007
#define T_CURSOR_DBL 0x0205

struct xy_coord
    {
     int x;
     int y;
     };

struct cursor
     {
      struct xy_coord p[C_STACK];
      struct xy_coord *p_ptr;
      unsigned int shape[SH_STACK];
      int *sh_ptr;
      };


void  t_gotoxy(int x, int y) ;
unsigned t_getcursor(void);
void  t_putcursor(unsigned value);
#define t_cursor_on()  t_putcursor( T_CURSOR_ON )
#define t_cursor_off()  t_putcursor( T_CURSOR_OFF )
int t_save_crsr_shape( void);
int t_reset_crsr_shape( void);

					/* screen output */
void _far t_puts( int x, int y, char _far *string);
int  _far t_vputs( int x, int y, char _far *bufft, char _far *format_string, ...);

					/* get/put text/window */
void _far  t_puttext( int left, int top, int right, int bottom, char _far *buff);
void _far  t_gettext( int left, int top, int right, int bottom, char _far *buff);
char _far *t_save_window( int left, int top, int right, int bottom);
void _far  t_free_window( int left, int top, int right, int bottom, char _far *buff);

					/* all for menu         */
void  t_init_window( void);
void _far t_reset_window( TEXTWINDOW _far *mw, char _far *screen_buff);
void  t_justify( int x0, int length, int position, int *x1, int *x2);
void  t_itemize( int row, int col, char *str, int len);
void _far drow_item( struct menu_items _far *mi, TEXTWINDOW _far *mw, int fg1, int bg1, int fg2, int bg2 );
int t_det_max_items( char *items[], int *max_len, int *max);
char *t_make_dubl_item( char *item, char *first, int max_first,
					    char *second, int max_second);
int  _far t_menu( int row, int col, char **items, struct mnuAtr *mA);
int  _far t_color_menu( int row, int col, char **items, struct mnuAtr *mA);
void _far t_gntxtbox(int left, int top, int right, int bottom, int code);
void _far t_text_in_box( char *text[], int fgText, int bgText, int fgBorder, int bgBorder, int x, int y,
			 int box_type, int shadow, int ask_quit);
void _far t_shadow(int left, int top, int right, int bottom);
void _far t_menu_box( WINDOW _far *mw, int dx, int dy, char body_char, int boder_type, int shadow_flag);


					/* get string in box
					   line editor      */
char *t_inscharinstr( int num_of_char, char ch, char *string);
char *t_delcharinstr( int num_of_char, char *string);
char *t_getstr(int col, int row, char *string, int fgTextNormal,
	       int bgTextNormal, int fgTextSelect, int bgTextSelect,
	       int max_len, struct charandatr *chat);
char *t_get_str_in_box(int col, int row, char *titel, int titelcolor,
			     char *string, int strlength, struct mnuAtr *mA);

					/* scrall screen and window */
void _far t_clrscr( void );
void _far t_scroll( struct textwindow *tw, int string_num, int direction, Uchar atr);


#define NO_SHADOW   0
#define YES_SHADOW  1



#ifndef _STRING_EDIT_STRUCT
  #define _STRING_EDIT_STRUCT

 struct string_to_edit
	 {
	  int x_pos, y_pos; // x,y string position
	  int active_flag;  // string active flag
	  char _far *string;// pointer to edit string
	  int max_s_len;    // max string length
	  int s_len;        // string lengs in pixels
	  int w_len;        // window length
	  int start_num;    // start number of char to draw string
	  int cursor_flag;  // INS, OVER, NO
	  int case_flag;    // upper case = 1, lower case = 0
	  };
 typedef struct string_to_edit STRING_TO_EDIT;

 struct string_to_draw
	 {
	  int x_pos, y_pos; // x,y string position
	  char _far *string;// pointer to edit string
	  };
 typedef struct string_to_draw STRING_TO_DRAW;

#endif


#ifndef _ACTIVE_CHAR_STRUCT
  #define _ACTIVE_CHAR_STRUCT

 struct active_char
	 {
	  int ch;
	  int code;
	  int flag;     // key +enter if == 1
	  };
 typedef struct active_char ACTIVE_CHAR;

#endif

int  _far edit_replay( int ch, ACTIVE_CHAR _far *ach, WINDOW _far *l1_wndw, int _far *enter_flag, int _far *current, int _far *previos, int num);
int  _far edit_check_kb( int ch, ACTIVE_CHAR _far *ach, int _far *enter_flag, int current, int num);
char _far *get_text_string( int _far *ch, ACTIVE_CHAR _far *ach, ACTIVE_CHAR _far *ach1, int x_pos, int y_pos, char *string,
			    int string_length, int wn_len, int _far *enter_flag, int case_f, WINDOW _far *l1_wndw,
			    int num_i, int num_s, int _far *previos, int _far *current);
void _far t_drow_string_in_window( int x_pos, int y_pos, int active_flag,
				   char _far *string, int _far *s_len, int wn_len,
				   int num_of_char, int cursor_flag);


#ifndef _WINDOW_MENU_STRUCT
  #define _WINDOW_MENU_STRUCT

 typedef struct
	{
	 int bg,
	 fgBorder,
	 fgSelect,
	 fgNormal;
	 } GLOBAL_MENU_COLORS;

 struct _point_menu
	 {
	  int num;          // number of active points
	  WINDOW _far *pw;  // pointer to windows of point
	  int item_num;     // active item number
	  };
 typedef struct _point_menu POINT_MENU;

 struct _cross_menu
	 {
	  int flag;         // active flag 0 not active,  1 active
	  WINDOW xw;        // window of point
	  };
 typedef struct _cross_menu CROSS_MENU;

 struct _item_menu
	 {
	  int num;          // number of active points
	  WINDOW _far *iw;  // pointer to windows of items
	  char _far **item; // pointer to item array
	  int item_num;     // active item number
	  };
 typedef struct _item_menu ITEM_MENU;

 struct tagSCRALL_MENU
	 {
	  int x_pos, y_pos;      // x,y left-upper corner
	  int width, height;     // window width and height
	  WINDOW scrlw;          // main scrall window of items
	  WINDOW _far *wi;       // pointer to item windows
	  WINDOW w_arrow[2];     // windows for up/doun arrows
	  WINDOW w_scale;        // window for scale
	  void (*fill_bar)( char _far **item_buff, int first, int num );   // pointer to fuction to fill buffer for drowing bar
	  char _far **item_buff; // pointer to item's buffer
	  int item_num;          // item number
	  int active_item_num;   // current active item number
	  int first;             // first item in bar
	  };
 typedef struct tagSCRALL_MENU SCRALL_MENU;

 struct tagBUTTON_MENU
	 {
	  int x_pos, y_pos;      // x,y left-upper corner
	  int width, height;     // window width and height
	  WINDOW butw;           // button window of items
	  char _far *item;       // pointer to item text
	  int border;            // item border flag : 0 - bracets, 1 - box
	  };
 typedef struct tagBUTTON_MENU BUTTON_MENU;

 struct file_menu_color
	{
	 int bg, fgBorder, fgSelect, fgNormal;
	 };

 struct window_line
	   {
	    int x0m, x0d, y0m, y0d; // line coord
	    int x1m, x1d, y1m, y1d; // line coord
	    int color;              // color of line
	    };
 typedef struct window_line WINDOW_LINE;

 struct window_menu_tabl
	   {
	    int x_pos, y_pos;        // senter upper position
	    int menu_code;           // menu code: -1 - close open menu, 0 - open first time
	    void _far **bit_map;     // pointer to screen buffer
	    Uint current;            // current item number
	    int summ_number;         // summary number of items
	    WINDOW _far *l0_wndw;    // pointer to main window
	    WINDOW _far *l1_wndw;    // pointer to window array
	    STRING_TO_EDIT _far *ps; // pointer to string egitor structure
	    POINT_MENU _far *pm;     // pointer to point menu structure
	    CROSS_MENU _far *px;     // pointer to cross menu structures
	    ITEM_MENU _far *pi;      // pointer to item menu structures
	    SCRALL_MENU _far *pscrl; // pointer to scrall menu structure
	    BUTTON_MENU _far *bm;    // pointer to button menu structure
	    ACTIVE_CHAR _far *ach;   // first spetial character codes set
	    ACTIVE_CHAR _far *ach1;  // second spetial character codes set
	    STRING_TO_DRAW _far *ds; // strings array pointer
	    WINDOW_LINE _far *ln;    // pointer to lines
	    void (*drow_handler)( void ); // pointer to user drow function
	    };
 typedef struct window_menu_tabl WINDOW_MENU;

 struct window_menu_items
	   {
	    int item_code; // menu item code: 1 - edit string, 2 - point menu, 3 - cross menu, 4 - item menu, 5 - scrall menu
	    int item_num;  // item number in window
	    int relativ_num;  // relativ item number in window
	    int exit_code; // 0 - stay window resident, othe close
	    };
 typedef struct window_menu_items WINDOW_MENU_ITEM;

#endif

int  _far t_window_menu( WINDOW_MENU _far *wmt, WINDOW_MENU_ITEM _far *wmi );
void _far t_redrow_dnwn( int current, int previos, WINDOW _far *l1_wndw, int num );
void _far t_drow_daun( int fgBordercolor, int bgcolor, int braket_color,
		       int left, int top, int right, int bottom, char _far *item );

#endif



