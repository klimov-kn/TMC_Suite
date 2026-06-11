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

#ifdef _DOS386
  #include <fg.h>
#endif

#if __STDC__
#define _Cdecl
#else
#define _Cdecl	cdecl
#endif

#if     !defined(__GRAPHX_DEF_)
#define __GRAPHX_DEF_

enum graphics_errors
  {    /* graphresult error return codes */
   grOk               =   0,
   grNoInitGraph      =  -1,
   grNotDetected      =  -2,
   grFileNotFound     =  -3,
   grInvalidDriver    =  -4,
   grNoLoadMem        =  -5,
   grNoScanMem        =  -6,
   grNoFloodMem       =  -7,
   grFontNotFound     =  -8,
   grNoFontMem        =  -9,
   grInvalidMode      = -10,
   grError            = -11,   /* generic error */
   grIOerror          = -12,
   grInvalidFont      = -13,
   grInvalidFontNum   = -14,
   grInvalidVersion   = -18
   };

enum graphics_drivers
  {   /* define graphics drivers */
   DETECT    = 0,   /* requests autodetection */
   CGA       = 1,   /* IBM CGA in 320*200 color (mode 0x04)         */
   MCGA      = 2,   /* IBM CGA in 640*200 (mode 0x06)               */
   EGA       = 3,   /* EGA                                          */
   EGA64     = 4,
   EGAMONO   = 5,
   IBM8514   = 6,        /* 1 - 6 */
   HERCMONO  = 7,
   ATT400    = 8,
   VGA       = 9,
   PC3270    = 10,                  /* 7 - 10 */
   ORCHIDPRO = 11,  /* ORCHID PRO (obsolete use FG_TSENGHIRES).   */
   TSENG     = 12,  /* Tseng Labs 800 x 600, 16 colors.           */
   PARADISE  = 13,  /* PARADISE VGA 800 x 600, 16 colors          */
   TRIDENT   = 14,  /* TRIDENT VGA 800 x 600, 16 colors           */
   ATI       = 15,  /* ATI                                        */
   VEGAVGA   = 16,  /* VIDEO 7 VEGA VGA in 800 x 600, 16 colors   */
   DFI       = 17,  /* Diamond Flower Instruments 800 x 600 x 16  */
   VESA      = 18,  /* VESA                                       */
   EVGA      = 19,  /* Everex EVGA in 800 x 600, 16 colors        */
   TOSHIBA   = 20,  /* Toshiba J-3100                             */
   TIGA      = 21,  /* Texas Instruments TMS340 Family - TIGA 2.0 */
   CURRENT_DRIVER = -1
   };

enum graphics_modes
   {   /* graphics modes for each driver */
    CGAC0      = 0,  /* 320x200 palette 0; 1 page	*/
    CGAC1      = 1,  /* 320x200 palette 1; 1 page	*/
    CGAC2      = 2,  /* 320x200 palette 2: 1 page	*/
    CGAC3      = 3,  /* 320x200 palette 3; 1 page	*/
    CGAHI      = 4,  /* 640x200 1 page			*/
    MCGAC0     = 0,  /* 320x200 palette 0; 1 page	*/
    MCGAC1     = 1,  /* 320x200 palette 1; 1 page	*/
    MCGAC2     = 2,  /* 320x200 palette 2; 1 page	*/
    MCGAC3     = 3,  /* 320x200 palette 3; 1 page	*/
    MCGAMED    = 4,  /* 640x200 1 page			*/
    MCGAHI     = 5,  /* 640x480 1 page			*/
    EGALO      = 0,  /* 640x200 16 color 4 pages	*/
    EGAHI      = 1,  /* 640x350 16 color 2 pages	*/
    EGA64LO    = 0,  /* 640x200 16 color 1 page 	*/
    EGA64HI    = 1,  /* 640x350 4 color  1 page 	*/
    EGAMONOHI  = 0,  /* 640x350 64K on card, 1 page - 256K on card, 4 pages */
    HERCMONOHI = 0,  /* 720x348 2 pages 		*/
    HERCHALF   = 1,  /* Hercules monochrome, 1 display page  */
    ATT400C0   = 0,  /* 320x200 palette 0; 1 page	*/
    ATT400C1   = 1,  /* 320x200 palette 1; 1 page	*/
    ATT400C2   = 2,  /* 320x200 palette 2; 1 page	*/
    ATT400C3   = 3,  /* 320x200 palette 3; 1 page	*/
    ATT400MED  = 4,  /* 640x200 1 page              */
    ATT400HI   = 5,  /* 640x400 1 page              */
    VGALO      = 0,  /* 640x200 16 color 4 pages	*/
    VGAMED     = 1,  /* 640x350 16 color 2 pages	*/
    VGAHI      = 2,  /* 640x480 16 color 1 page 	*/
    VGAMONO    = 3,  /* IBM VGA in 640*480, 1 color */
    PC3270HI   = 0,  /* 720x350 1 page              */
    IBM8514LO  = 0,  /* 640x480 256 colors          */
    IBM8514HI  = 1,  /*1024x768 256 colors          */

    ORCHIDPROHIRES = 0, /* ORCHID PRO (obsolete use FG_TSENGHIRES).   */
    TSENGHIRES     = 0, /* Tseng Labs 800 x 600, 16 colors.           */
    PARADISEHIRES  = 0, /* PARADISE VGA 800 x 600, 16 colors          */
    TRIDENTHIRES   = 0, /* TRIDENT VGA 800 x 600, 16 colors           */
    ATI61          = 0, /* ATI 640 x 400, 256 colors.                 */
    ATI62          = 1, /* ATI 640 x 480, 256 colors.                 */
    ATI63          = 2, /* ATI 800 x 600, 256 colors.                 */
    VEGAVGAHIRES   = 0, /* VIDEO 7 VEGA VGA in 800 x 600, 16 colors   */
    DFIHIRES       = 0, /* Diamond Flower Instruments 800 x 600 x 16  */
    VESA0          = 0, /* VESA mode 0x100, 640 x 400, 256 colors.    */
    VESA1          = 1, /* VESA mode 0x101, 640 x 480, 256 colors.    */
    VESA2          = 2, /* VESA mode 0x102, 800 x 600, 16 colors.     */
    VESA3          = 3, /* VESA mode 0x103, 800 x 600, 256 colors.    */
    VESA4          = 4, /* VESA mode 0x104, 1024 x 768, 16 colors.    */
    VESA5          = 5, /* VESA mode 0x105, 1024 x 768, 256 colors.   */
    VESA6          = 6, /* VESA mode 0x106, 1280 x 1024, 16 colors.   */
    VESA7          = 7, /* VESA mode 0x107, 1280 x 1024, 256 colors.  */
    VESA6A         = 8, /* VESA mode 0x6A, 800 x 600, 16 colors.      */
    EVGAHIRES      = 0, /* Everex EVGA in 800 x 600, 16 colors        */
    TOSHIBAJ_3100  = 0, /* Toshiba J-3100                             */
    TIGA20         = 0  /* Texas Instruments TMS340 Family - TIGA 2.0 */
    };

/* Colors for setpalette and setallpalette */

#if	!defined(__COLORS)
#define __COLORS

 #ifndef __FG_H
  enum COLORS
   {
    BLACK,		    /* dark colors */
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    LIGHTGRAY,
    DARKGRAY,		    /* light colors */
    LIGHTBLUE,
    LIGHTGREEN,
    LIGHTCYAN,
    LIGHTRED,
    LIGHTMAGENTA,
    YELLOW,
    WHITE
    };
  #else
   #define BLACK           fg.color[0]
   #define BLUE            fg.color[1]
   #define GREEN           fg.color[2]
   #define CYAN            fg.color[3]
   #define BLUE_GREEN      fg.color[3]
   #define RED             fg.color[4]
   #define MAGENTA         fg.color[5]
   #define PURPLE          fg.color[5]
   #define BROWN           fg.color[14]
   #define LIGHTGRAY       fg.color[15]  //   FG_LIGHT_WHITE
   #define DARKGRAY        fg.color[8]   //   FG_GRAY
   #define LIGHTBLUE       fg.color[9]
   #define LIGHTGREEN      fg.color[10]
   #define LIGHTCYAN       fg.color[11]
   #define LIGHTBLUEGREEN  fg.color[11]
   #define LIGHTRED        fg.color[12]
   #define LIGHTMAGENTA    fg.color[13]
   #define YELLOW          fg.color[6]
   #define WHITE           fg.color[7]
   #define WHITEBLINK      fg.color[16]

 #endif

#endif

enum CGA_COLORS
    {
    CGA_LIGHTGREEN     = 1,		/* Palette C0 Color Names	*/
    CGA_LIGHTRED       = 2,
    CGA_YELLOW	       = 3,

    CGA_LIGHTCYAN      = 1,		/* Palette C1 Color Names	*/
    CGA_LIGHTMAGENTA   = 2,
    CGA_WHITE          = 3,

    CGA_GREEN          = 1,   /* Palette C2 Color Names */
    CGA_RED            = 2,
    CGA_BROWN          = 3,

    CGA_CYAN           = 1,   /* Palette C3 Color Names */
    CGA_MAGENTA        = 2,
    CGA_LIGHTGRAY      = 3
    };


enum EGA_COLORS
   {
    EGA_BLACK        =  0,      /* dark colors */
    EGA_BLUE         =  1,
    EGA_GREEN        =  2,
    EGA_CYAN         =  3,
    EGA_RED          =  4,
    EGA_MAGENTA      =  5,
    EGA_BROWN        =  20,
    EGA_LIGHTGRAY    =  7,
    EGA_DARKGRAY     =  56,     /* light colors */
    EGA_LIGHTBLUE    =  57,
    EGA_LIGHTGREEN	 =  58,
    EGA_LIGHTCYAN    =  59,
    EGA_LIGHTRED     =  60,
    EGA_LIGHTMAGENTA =  61,
    EGA_YELLOW       =  62,
    EGA_WHITE        =  63
};

#ifndef __FG_H
  enum line_styles 
   {      /* Line styles for get/setlinestyle */
	SOLID_LINE   = 0,
	DOTTED_LINE  = 1,
	CENTER_LINE  = 2,
	DASHED_LINE  = 3,
	USERBIT_LINE = 4,	/* User defined line style */
    };
 #else
                      /* Line types   */
  #define    SOLID_LINE         0
  #define    DOTTED_LINE        7
  #define    CENTER_LINE        3
  #define    DASHED_LINE        4
  #define    USERBIT_LINE       9

  #define    LINE_LONG_DASH     1
  #define    LINE_MEDIUM_DOTTED 2
  #define    LINE_DASH_W_2_DOTS 5
  #define    LINE_SHORT_DASH    6
  #define    LINE_SPARSE_DOTTED 8
  #define    LINE_FG_RESERVED1  10
  #define    LINE_FG_RESERVED2  11
#endif

enum line_widths 
   {      /* Line widths for get/setlinestyle */
	NORM_WIDTH  = 1,
	THICK_WIDTH = 3,
    };

enum font_names {
  DEFAULT_FONT    = 0,  /* 8x8 bit mapped font */
  TRIPLEX_FONT    = 1,  /* "Stroked" fonts */
  SMALL_FONT      = 2,
  SANS_SERIF_FONT = 3,
  GOTHIC_FONT     = 4
  };

#define HORIZ_DIR	0	/* left to right */
#define VERT_DIR	1	/* bottom to top */

#define USER_CHAR_SIZE	0	/* user-defined char size */

#ifndef __FG_H
  enum fill_patterns 
   {        /* Fill patterns for get/setfillstyle */
    EMPTY_FILL,     /* fills area in background color */
    SOLID_FILL,     /* fills area in solid fill color */
    LINE_FILL,      /* --- fill */
    LTSLASH_FILL,   /* /// fill */
    SLASH_FILL,     /* /// fill with thick lines */
    BKSLASH_FILL,   /* \\\ fill with thick lines */
    LTBKSLASH_FILL, /* \\\ fill */
    HATCH_FILL,     /* light hatch fill */
    XHATCH_FILL,    /* heavy cross hatch fill */
    INTERLEAVE_FILL,/* interleaving line fill */
    WIDE_DOT_FILL,  /* Widely spaced dot fill */
    CLOSE_DOT_FILL, /* Closely spaced dot fill */
    USER_FILL       /* user defined fill */
    };
    #define FILL_PATTERN_MAX   12
 #else
    #define SOLID_FILL        0
    #define BKSLASH_FILL      1
    #define SLASH_FILL        2
    #define HATCH_FILL        3
    #define LINE_FILL         4
    #define LTSLASH_FILL      5
    #define LTBKSLASH_FILL    6
    #define XHATCH_FILL       7
    #define WIDE_DOT_FILL     8
    #define CLOSE_DOT_FILL    9
    #define INTERLEAVE_FILL   10
    #define USER_FILL         11
    #define EMPTY_FILL         0

    #define FILL_PATTERN_MAX  12
#endif

enum putimage_ops {		/* BitBlt operators for putimage */
	COPY_PUT,		/* MOV */
	XOR_PUT,		/* XOR */
	OR_PUT, 		/* OR  */
	AND_PUT,		/* AND */
	NOT_PUT 		/* NOT */
};

enum text_just {		/* Horizontal and vertical justification
				   for settextjustify */
  LEFT_TEXT     = 0,
  CENTER_TEXT	= 1,
  RIGHT_TEXT	= 2,

  BOTTOM_TEXT   = 0,
/*CENTER_TEXT   = 1,  already defined above */
  TOP_TEXT      = 2
  };


#define MAXCOLORS 15

struct palettetype {
        unsigned char size;
	signed char colors[MAXCOLORS+1];
};

struct linesettingstype {
	int linestyle;
	unsigned upattern;
	int thickness;
};

struct textsettingstype {
	int font;
	int direction;
	int charsize;
	int horiz;
	int vert;
};

struct fillsettingstype {
	int pattern;
	int color;
};

struct pointtype 
   {
	int x, y;
    };

struct viewporttype {
	int left, top, right, bottom;
	int clip;
};

struct arccoordstype 
   {
	int x, y;
	int xstart, ystart, xend, yend;
    };

void   far _Cdecl arc( int x, int y, int stangle, int endangle, int radius);
void  _far _Cdecl bar( int left, int top, int right, int bottom );
void   far _Cdecl bar3d( int left, int top, int right, int bottom,
                         int depth, int topflag);
void   far _Cdecl circle( int x, int y, int radius);
void   far _Cdecl cleardevice(void);
void   far _Cdecl clearviewport(void);
void   far _Cdecl closegraph(void);
void   far _Cdecl detectgraph(int far *graphdriver,int far *graphmode);
void   far _Cdecl drawpoly(int numpoints, int far *polypoints);
void   far _Cdecl ellipse(int x, int y, int stangle, int endangle,
                          int xradius, int yradius);
void   far _Cdecl fillellipse( int x, int y, int xradius, int yradius );
void   far _Cdecl fillpoly(int numpoints, int far *polypoints);
void   far _Cdecl floodfill(int x, int y, int border);
void   far _Cdecl getarccoords(struct arccoordstype far *arccoords);
void   far _Cdecl getaspectratio(int far *xasp, int far *yasp);
int	   far _Cdecl getbkcolor(void);
int	   far _Cdecl getcolor(void);
struct palettetype * far _Cdecl getdefaultpalette( void );
char * far _Cdecl getdrivername( void );
void   far _Cdecl getfillpattern(char far *pattern);
void   far _Cdecl getfillsettings(struct fillsettingstype far *fillinfo);
int	   far _Cdecl getgraphmode(void);
void   far _Cdecl getimage( int left, int top, int right, int bottom,
                            void far *bitmap);
void   far _Cdecl getlinesettings(struct linesettingstype far *lineinfo);
int    _far _Cdecl getmaxcolor(void);
int	   far _Cdecl getmaxmode(void);
int    _far _Cdecl getmaxx(void);
int    _far _Cdecl getmaxy(void);
char * far _Cdecl getmodename( int mode_number );
void   far _Cdecl getmoderange(int graphdriver, int far *lomode, int far *himode);
unsigned   far _Cdecl getpixel(int x, int y);
void   _far _Cdecl getpalette(struct palettetype far *palette);
int	   far _Cdecl getpalettesize( void );
void   far _Cdecl gettextsettings(struct textsettingstype far *texttypeinfo);
void   far _Cdecl getviewsettings(struct viewporttype far *viewport);
int	   far _Cdecl getx(void);
int	   far _Cdecl gety(void);
void   far _Cdecl graphdefaults(void);
char * far _Cdecl grapherrormsg(int errorcode);
void   far _Cdecl _graphfreemem(void far *ptr, unsigned size);
void   far * far _Cdecl _graphgetmem(unsigned size);
int	   far _Cdecl graphresult(void);
unsigned   far _Cdecl imagesize(int left, int top, int right, int bottom);
void   far _Cdecl initgraph(int  far *graphdriver, int  far *graphmode, char far *pathtodriver);
/*int        far _Cdecl installuserdriver( char far *name, int huge (*detect)(void) );*/
int	   far _Cdecl installuserfont( char far *name );
void   far _Cdecl line(int x1, int y1, int x2, int y2);
void   far _Cdecl linerel(int dx, int dy);
void   far _Cdecl lineto(int x, int y);
void   far _Cdecl moverel(int dx, int dy);
void   far _Cdecl moveto(int x, int y);
void   far _Cdecl outtext(char far *textstring);
void   far _Cdecl outtextxy(int x, int y, char far *textstring);
void   far _Cdecl pieslice(int x, int y, int stangle, int endangle,
                           int radius);
void   far _Cdecl putimage(int left, int top, void far *bitmap, int op);
void   far _Cdecl putpixel(int x, int y, int color);
void   _far _Cdecl rectangle(int left, int top, int right, int bottom);
void   far _Cdecl restorecrtmode(void);
void   far _Cdecl sector( int X, int Y, int StAngle, int EndAngle,
          int XRadius, int YRadius );
void   far _Cdecl setactivepage(int page);
void   far _Cdecl setallpalette(struct palettetype far *palette);
void   far _Cdecl setaspectratio( int xasp, int yasp );
void   far _Cdecl setbkcolor(int color);
void   _far _Cdecl setcolor(int color);
void   far _Cdecl setfillpattern(char far *upattern, int color);
void   _far _Cdecl setfillstyle(int pattern, int color);
unsigned   far _Cdecl setgraphbufsize(unsigned bufsize);
void   far _Cdecl setgraphmode(int mode);
void   far _Cdecl setlinestyle(int linestyle, unsigned upattern,
         int thickness);
void   far _Cdecl setpalette(int colornum, int color);
void   far _Cdecl setrgbpalette(int colornum, int red, int green, int blue);
void   far _Cdecl settextjustify(int horiz, int vert);
void   far _Cdecl settextstyle(int font, int direction, int charsize);
void   far _Cdecl setusercharsize(int multx, int divx, int multy, int divy);
void   far _Cdecl setviewport(int left, int top, int right, int bottom, int clip);
void   far _Cdecl setvisualpage(int page);
void   far _Cdecl setwritemode( int mode );
int	   far _Cdecl textheight(char far *textstring);
int	   far _Cdecl textwidth(char far *textstring);

/***** graphics drivers *****/

int    far _Cdecl registerbgidriver(void (*driver)(void));
int	   far _Cdecl registerfarbgidriver(void far *driver);

/* !!	     These "functions" are NOT user-callable          !! */
/* !! They are there just so you can link in graphics drivers !! */

void	       _Cdecl CGA_driver(void);
void	       _Cdecl EGAVGA_driver(void);
void	       _Cdecl IBM8514_driver(void);
void	       _Cdecl Herc_driver(void);
void	       _Cdecl ATT_driver(void);
void	       _Cdecl PC3270_driver(void);

/*extern int far _Cdecl *CGA_driver_far;
  extern int far _Cdecl *EGAVGA_driver_far;
  extern int far _Cdecl *IBM8514_driver_far;
  extern int far _Cdecl *Herc_driver_far;
  extern int far _Cdecl *ATT_driver_far;
  extern int far _Cdecl *PC3270_driver_far;
*/
extern int far  CGA_driver_far[];
extern int far  EGAVGA_driver_far[];
extern int far  IBM8514_driver_far[];
extern int far  Herc_driver_far[];
extern int far  ATT_driver_far[];
extern int far  PC3270_driver_far[];

/***** graphics fonts *****/

int	       _Cdecl registerbgifont(void (*font)(void));
int	   far _Cdecl registerfarbgifont(void far *font);

/* !!	These "functions" are NOT user-callable,   !! */
/* !! they are there just so you can link in fonts !! */

void	       _Cdecl	triplex_font(void);
void	       _Cdecl	  small_font(void);
void	       _Cdecl sansserif_font(void);
void	       _Cdecl	 gothic_font(void);

/*
extern int far _Cdecl	triplex_font_far[];
extern int far _Cdecl	  small_font_far[];
extern int far _Cdecl sansserif_font_far[];
extern int far _Cdecl	 gothic_font_far[];
*/
extern int far 	triplex_font_far[];
extern int far  small_font_far[];
extern int far  sansserif_font_far[];
extern int far  gothic_font_far[];


#endif

#ifndef _VIDEOCONFIG_DEFINED

                /* structure for _getvideoconfig() as visible to user */
struct videoconfig {
        int numxpixels;       /* number of pixels on X axis */
        int numypixels;       /* number of pixels on Y axis */
        int numtextcols;      /* number of text columns available */
        int numtextrows;      /* number of text rows available */
        int numcolors;        /* number of actual colors */
        struct palettetype pt;/* palette                 */
        int xasp;             /* x aspect */
        int yasp;             /* y aspect */
        float x_asp;          /* software x aspect */
        float y_asp;          /* software y aspect */
        double AspectRatio;   /* aspect ratio  */
        int bitsperpixel;     /* number of bits per pixel */
        int numvideopages;    /* number of available video pages */
        int mode;             /* current video mode */
        int adapter;          /* active display adapter */
        int monitor;          /* active display monitor */
        int memory;           /* adapter video memory in K bytes */
        int error;            /* erro code    */
};
#define _VIDEOCONFIG_DEFINED
#endif

#define CLIP_ON   1
#define CLIP_OFF  0

#define MAX_COLOR_STYLE  0
#define HIGH_RES_STYLE   1

#define EGA_x_asp   640
#define EGA_y_asp   350

#define x_ftoint( f )  ( (int)( f * vc.x_asp ) )
#define y_ftoint( f )  ( (int)( f * vc.y_asp ) )

#define NO_SHADOW   0
#define YES_SHADOW  1


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
 typedef struct menu_items MENU_ITEMS;

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
            int border_type;   /* border type code*/
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
             char far *item;
             char letter;
             struct window mw;
             };

#define _MENU_STRUCT
#endif

struct set_char_size
                {
                 int xm, xd;
                 int ym, yd;
                 };

void _far _Cdecl set_tgi_path( char _far *path );
void _far _Cdecl set_init_style( int style );
void _far _Cdecl set_predefined_adapter( int adapter );

int  far _Cdecl det_max_item( char far *string[], int far *max_len);
void far _Cdecl drow_shadow( int left, int top, int right, int bottom, int delta_x,
                                                               int delta_y );
void far _Cdecl general_border( int key, int BackColor, int BorderColor, int left,
                                            int top, int right, int bottom );
void far _Cdecl general_border_one( int key, int BorderColor, int left, int top,
                                                     int right, int bottom );
void far _Cdecl init_graph( void);
int  far _Cdecl H_char_height( void );
int  far _Cdecl H_char_width( void );
void far _Cdecl messege_in_box( int fgMessegeColor, int bgMessegeColor,
                         int bgBorder, int type_border, int left,
                         int top, int right, int bottom, char far *messege);
int  far _Cdecl get_max_item_hor( struct menu_items far *mi,
                                  int col, int row, struct window far *wn);
void far _Cdecl drow_item( struct menu_items far *mi, struct window far *iw,
                                    struct alt_colors far *ic, int bgColor);
int _far bar_menu( int col, int row, struct menu_items _far *mi, struct menu_attrib _far *mA );
void far _Cdecl drow_menu_bar( int item_num, struct window far *bar_wn,
                        struct menu_items far *mi, struct window far *it_wn,
                        int BorderColor, int border_type, int BarColor);
int  far _Cdecl get_max_item_bar( struct menu_items far *mi, int col, int row,
                          struct window far *wn, struct window far *bar_wn);
int  far _Cdecl messege_with_menu( char far **messege, int x_pos, int y_pos,
                            struct messege_menu far *msm,
                            int BorderColor, int TextColor, int BackColor,
                            int border_flag);
void far _Cdecl text_in_box( int fgTextColor, int fgBorderColor, int bgColor,
                      int x_pos, int y_pos, int pause_flag, int shadow_flag,
                      int border_flag, char far *string[]);
int  far _Cdecl textwidth_n( char far *string, int num);

#define MAX_TRANS_NUM        10
#define EXEEDED_MAX_NTN     -31
#define NT_NOT_INIT         -32
#define ILLEGAL_WINDOW_SIZE -33
#define POLYGON_SIZE        200

#ifndef _FLOAT_WINDOW
 #define _FLOAT_WINDOW

struct float_window
          {
           float left;
           float bottom;
           float right;
           float top;
           };
typedef struct float_window FLOAT_WINDOW;

#endif

struct norm_transf
         {
          int active_num;
          struct viewporttype vp[MAX_TRANS_NUM];
          struct float_window fvp[MAX_TRANS_NUM];
          struct float_window fwn[MAX_TRANS_NUM];
          float x_wn_scale;
          float y_wn_scale;
          };

#ifndef _FLOAT_POINTTYPE
  #define _FLOAT_POINTTYPE

struct float_pointtype
	  {
	   _real x;
	   _real y;
	   };

typedef struct float_pointtype FPOINT;

#endif

void _far _Cdecl drawpoly_w( int numpoints, float far *polypoints );
void _far _Cdecl fillpoly_w( int numpoints, float far *polypoints );
void _far _Cdecl outtextxy_w( float x, float y, char far *string);
void _far _Cdecl line_w( float x1, float y1, float x2, float y2 );
void _far _Cdecl point_w( float x, float y );
void _far _Cdecl bar_w( float left, float bottom, float right, float top );
void _far _Cdecl rectangle_w( float left, float bottom, float right, float top );
int  _far _Cdecl get_current_norm_transf_num( void );
int  _far _Cdecl get_norm_transf( int nt_num, struct viewporttype far *vp,
                                      struct float_window far *fvp,
                                      struct float_window far *fwn);
int  far _Cdecl set_norm_transf( int nt_num );
void far _Cdecl set_norm_transf_scale( int nt_num );
int  far _Cdecl set_viewport( int nt_num,
                       float left, float bottom, float right, float top );
int  far _Cdecl set_window( int nt_num,
                      float left, float bottom, float right, float top );
int  far _Cdecl x_norm_transf( float x );
int  far _Cdecl y_norm_transf( float y );


#ifndef _STRING_EDIT_STRUCT
  #define _STRING_EDIT_STRUCT



struct string_to_edit
         {
          int x_pos, y_pos; /* x,y string position*/
          int active_flag;  /* string active flag*/
          char _far *string;/* pointer to edit string buffer*/
          int max_s_len;    /* max string length in byte*/
          int s_len;        /* string lengs in pixels*/
          int w_len;        /* window length*/
          int start_num;    /* start number of char to draw string*/
          int cursor_flag;  /* INS, OVER, NO*/
          int case_flag;    /* upper case = 1, lower case = 0 */
          };
typedef struct string_to_edit STRING_TO_EDIT;

struct string_to_draw
         {
          int x_pos, y_pos; /* x,y string position*/
          char _far *string;/* pointer to edit string*/
          };
typedef struct string_to_draw STRING_TO_DRAW;

#endif


#ifndef _ACTIVE_CHAR_STRUCT
  #define _ACTIVE_CHAR_STRUCT

struct active_char
         {
          int ch;
          int code;
          int flag;     /* key +enter if == 1*/
          };
typedef struct active_char ACTIVE_CHAR;

#endif

char _far *get_graph_string( int _far *ch, ACTIVE_CHAR _far *ach, ACTIVE_CHAR _far *ach1, int x_pos, int y_pos, char *string,
                             int string_length, int wn_len, int _far *enter_flag, int case_f, WINDOW _far *l1_wndw,
                             int num_i, int num_s, int _far *previos, int _far *current);
void _far drow_string_in_window( int x_pos, int y_pos, int active_flag,
                                 char _far *string, int _far *s_len, int wn_len,
                                 int num_of_char, int cursor_flag);

int  _far edit_replay( int ch, ACTIVE_CHAR _far *ach, WINDOW _far *l1_wndw, int _far *enter_flag, int _far *current, int _far *previos, int num);
int  _far edit_check_kb( int ch, ACTIVE_CHAR _far *ach, int _far *enter_flag, int current, int num);

char _far *ins_char_in_str( int num_of_char, char ch, char _far *string);
char _far *del_char_in_str( int num_of_char, char _far *string);
char _far *strrshift( char _far *string, char ch);


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
          int num;          /* number of active points*/
          WINDOW _far *pw;  /* pointer to windows of point*/
          int item_num;     /* active item number*/
          };
typedef struct _point_menu POINT_MENU;

struct _cross_menu
         {
          int flag;         /* active flag 0 not active,  1 active*/
          WINDOW xw;        /* window of point*/
          };
typedef struct _cross_menu CROSS_MENU;

struct file_menu_color
	{
	 int bg, fgBorder, fgSelect, fgNormal;
	 };


struct window_line
	   {
	    int x0m, x0d, y0m, y0d; /* line coord*/
	    int x1m, x1d, y1m, y1d; /* line coord*/
	    int color;              /* color of line*/
	    };
typedef struct window_line WINDOW_LINE;

struct window_menu_tabl
	   {
      int x_pos, y_pos;        /* senter upper position*/
      int menu_code;           /* menu code: -1 - close open menu, 0 - open first time*/
      void _far **bit_map;     /* pointer to screen buffer*/
      int current;            /* current item number*/
      int summ_number;         /* summary number of items*/
      WINDOW _far *l0_wndw;    /* pointer to main window*/
      WINDOW _far *l1_wndw;    /* pointer to window array*/
      STRING_TO_EDIT _far *ps; /* pointer to string egitor struct*/
      POINT_MENU _far *pm;     /* pointer to point menu structure*/
      CROSS_MENU _far *px;     /* pointer to cross menu structures*/
      ACTIVE_CHAR _far *ach;   /* first spetial character codes set*/
      ACTIVE_CHAR _far *ach1;  /* second spetial character codes set*/
      STRING_TO_DRAW _far *ds; /* strings array pointer*/
	    WINDOW_LINE _far *ln;    /* pointer to lines*/
      void (*drow_handler)( void ); /* pointer to user drow function*/
	    };
typedef struct window_menu_tabl WINDOW_MENU;

struct window_menu_items
           {
            int item_code; /* menu item code: 1 - edit string, 2 - point menu, 3 - cross menu*/
            int item_num;  /* item number in window*/
            int relativ_num;  /* relativ item number in window*/
            int exit_code; /* 0 - stay window resident, othe close*/
            };
typedef struct window_menu_items WINDOW_MENU_ITEM;

#endif

int  _far window_menu( WINDOW_MENU _far *wmt, WINDOW_MENU_ITEM _far *wmi );
void _far redrow_dnwn( int current, int previos, WINDOW _far *l1_wndw, int num );

void _far **quick_save_window( int left, int top, int right, int bottom, void _far *bit_map[]);
void _far quick_restore_window( int left, int top, int right, int bottom, void _far *bit_map[]);
int  _far bar_menu( int col, int row, struct menu_items _far *mi, struct menu_attrib _far *mA );


/* marker functions*/
                                    /* marker size */
#define _SMALL              1
#define _MEDIUM             2
#define _BIG                3
#define _LARGE              4

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

void  _far _set_marker( int x, int y );
void  _far set_marker_type( MARKER _far *mt );


      /* button section*/

struct _button
	 {
	  int x, y;
	  char _far *str;
	  };
typedef struct _button BUTTON;

#define UNPRESSED  1
#define PRESSED    0

void _far set_button_size( int width, int height );
void _far set_button_depth( int dx, int dy );
void _far set_button_colors( int light, int front, int shadow );
void _far set_button_string_colors( int light, int shadow );
void _far button( int x, int y, char _far *str, int flag );

#endif

