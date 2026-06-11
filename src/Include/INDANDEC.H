/*
**  TAMIC_soft(R) 1990 v1.00
**
**  indandec.h
**
**  FILE : a:\include\indandec.c
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _INDANDEC_H
 #define _INDANDEC_H


typedef struct
        {
         int   mode;        /* number of mode on window*/
         int   num;         /* window number*/
         _real d, u;        /* lower & upper coordinate.*/
         } SECTION_WINDOW;

struct modul_TRN
        {
         int   waves;       /* number of mode on input*/
         int   wn_num;      /* number of window*/
         int   in_num;      /* input number*/
         int   bound;       /* bound type of module*/
         int   type;        /* left side or right side input*/
         int   mode;        /* number of mode in module*/
         _real a, b, e, m;  /* height, width, epsilon, mu*/
         SECTION_WINDOW _far *sw;   /* windows*/
         };
typedef struct modul_TRN MODUL_TRN;

struct modul_WVGD
        {
         int   code;
         int   lwn_num;
         int   rwn_num;
         int   bound;
         int   type;
         int   mode;
         _real a, b, e, m;
         SECTION_WINDOW _far *lsw;
         SECTION_WINDOW _far *rsw;
         };
typedef struct modul_WVGD MODUL_WVGD;

struct modul_STB
        {
         int   code;          /* stub code M_OPSTB_n ...*/
         int   wn_num;
         int   bound;         /* bounder type flag*/
         int   type;          /* left or right stub*/
         int   mode;          /* number of mode in module*/
         _real a, b, e, m;
         SECTION_WINDOW _far *sw;
         };
typedef struct modul_STB MODUL_STB;

#ifndef _MODUL_FL
  #define _MODUL_FL
  typedef struct
        {
         char twp_name[FILENAME_MAX];
         int  inv_tabl[MAX_WINDOW_NUM];
         }MODUL_FL;
#endif

#ifndef _GP_POLYGON_DEF
 #define _GP_POLYGON_DEF
struct poly_line_attr
        {
         _real  x, y;                       /* point coordinates*/
         int    bound;                      /* boundary type: input if < 0, internal XX - 0, metal > 0*/
         int    mode;                       /* mode number if input*/
         _real  e, m;                       /* boundary E,M*/
         struct poly_line_attr _far *next;  /* pointer to next line*/
         };
typedef struct poly_line_attr POLY_LINE_ATTR;

struct gp_polygon
         {
          int   code;
          _real e, m;                        /* E, M                                    */
          int   num;                         /* polygon number                          */
          POLY_LINE_ATTR _far *pla;          /* pointer to line structure               */
          struct gp_polygon _far *next;      /* pointer to next polygon                 */
          Uint lw_num;                       /* left window number for INPUT, WAVEG ... */
          Uint rw_num;                       /* right window number for INPUT, WAVEG ...*/
          };

typedef struct gp_polygon GP_POLYGON;
#endif

struct list_ptr
        {
         int  code;
         void _far *ptr;
         };
typedef struct list_ptr LIST_PTR;

#ifndef _INP_ADRESS
 #include <run.h>
#endif

#endif
