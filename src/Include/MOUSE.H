/*
**
**  TAMIC_soft(R) 1989 v1.00  Produced by Vladimir Kustov.
**
**  Definitions & declorations for mouse package.
**
**  FILE: mouse.h
**
**  Copyright (c) TAMIC_soft. All Rights Reserved. 1989,1990.
**
**
*/


#ifndef _MOUSE_H
 #define _MOUSE_H

#if __STDC__
  #define _Cdecl
#else
  #define _Cdecl  cdecl
#endif

#ifndef _MENU_STRUCT
 #define _MENU_STRUCT
  struct window { int left, top, right, bottom; };
  typedef struct window WINDOW;
#endif

struct mouse_position
                {
                 int x;
                 int y;
                 };
typedef struct mouse_position MOUSE_POSITION;


#ifndef __GRAPHX_DEF_
struct viewporttype
                {
                 int left, top, right, bottom;
                 int clip;                       /* not used */
                 };
#endif

#define M_INSTALLED    (-1)
#define M_NO_INSTALLED   0
#define LEFT_BUTTON      0
#define RIGHT_BUTTON     1
#define MIDDLE_BUTTON    2

#define LEFT_BUTTON_MASK    1
#define RIGHT_BUTTON_MASK   2
#define MIDDLE_BUTTON_MASK  4
#define ALL_BUTTON_MASK     7


extern int mouse_flag;

int  far _Cdecl reset_mouse( int far *num_of_button);
     /* Reset Mouse Driver

        Function gives the current status of the mouse  hardware  plus
        the current status of the mouse software.  The calling program
        is able to determine the presence of a mouse driver  and/or  a
        serial port. This function resets  the  mouse  driver  to  the
        following default status as indicated:

Variable                                Value
----------------------------------------------------------------------
internal cursor flag                    -1 (cursor concealed)
graphics  cursor shape                  horizontal oval
text cursor                             reverse video
user-defined call mask                  all zeroes
light pen emulation mode                enabled
vertical mouse motion/pixel ratio       16 to 8
horizontal mouse motion/pixel ratio     8 to 8
vertical min/max cursor coordinates     0/current display mode y values
                                        minus 1
horizontal min/max cursor coordinates   0/current display mode x values
                                        minus 1

        Input:  none
        Return: mouse state (-1: installed, 0: not installed)
                *num_of_button -> number of buttons (2 button mode,
                                                     3 button mode)  */

void far _Cdecl mouse_on( void);
     /* Enable Cursor Display

        Function increments the internal cursor flag counter.  If the
        counter is zero, the cursor  is enabled and appears on-screen.

        Function must be called to display the cursor.  In  case  the
        internal cursor flag is already zero, a call to this function
        produces no effect.

        Input:  none
        Return: none                                                */


void far _Cdecl mouse_off( void);
      /* Disable Cursor Display

         Function disables the cursor by removing it from the  screen
         and decrementing the internal cursor flag.  Even  though the
         cursor cannot be seen, it still tracks any motion  made with
         the mouse.

         You should use this function before changing any  portion of
         the screen containing the cursor. You will avoid the problem
         of the cursor affecting screen data.

         Keep in mind that whenever your program calls  function,  it
         must later call function  mouse_on  to return  the  internal
         cursor flag to its default  value.   In  addition,  if  your
         program changes the screen mode, function is called automati-
         cally. Therefore, the cursor's movement is enabled the  next
         time it is displayed.

         Call function at the end of a program  in  order  to conceal
         the cursor.  This ensures that nothing remains on-screen.

         Input:  none
         Return: none                                              */

void far _Cdecl set_hor_range( int xmin, int xmax);
      /* Define Horizontal (X) Range of Cursor Location

         Function defines the horizontal range of the cursor on-screen.
         As a result, cursor movement is  limited  to  this  specified
         area.  If a cursor happens to be outside of this area when  a
         call is made, the cursor is moved to just inside the area.

         Input:  xmin = minimum horizontal cursor coordinate
                 xmax = maximum horizontal cursor coordinate

         Return: none                                               */

void far _Cdecl set_ver_range( int ymin, int ymax);
      /* Define Vertical (Y) Range of Cursor Location

         Function defines the vertical range of  the  cursor on-screen.
         As a result, cursor movement is limited to this specified area.
         If a cursor happens to be outside of this  area when a call is
         made, the cursor is moved to just inside the area.

         Input:  ymin = minimum vertical cursor coordinate
                 ymax = maximum vertical cursor coordinate

         Return: none                                               */

void far _Cdecl set_mouse_viewport( int left, int top, int right, int bottom);
      /* Define Horizontal (X) & Vertical (Y) Range of Cursor Location

         Function defines the viewport of  the  cursor on-screen. As a
         result, cursor movement is limited to this specified area. If
         a  cursor  happens to be outside of this  area when a call is
         made, the cursor is moved to just inside the area.

         Input:  left   = minimum horizontal cursor coordinate
                 top    = minimum vertical cursor coordinate
                 right  = maximum horizontal cursor coordinate
                 bottom = maximum vertical cursor coordinate

         Return: none                                               */

void far _Cdecl set_disable_mouse_viewport( struct viewporttype far *vp);
      /* Disable Cursor Display in Special Range

         Function sets up a special range on-screen. If the cursor moves
         to this area or is in this area, it will be disabled.  After  a
         call is made to this function, it is necessary to call function
         mouse_on to enable the cursor again. Define the  special  range
         with screen location values using four components:

            Components          Values
         --------------------------------------------------------
          1               Left horizontal screen location
          2               Upper vertical screen location
          3               Right horizontal screen location
          4               Lower vertical screen location

         Input:  *vp = pointer to special range used Turbo C structure

         Return: none                                                  */

void far _Cdecl set_mouse_position( int x, int y);
      /* Set Cursor Location

         Function sets the current cursor location.  Values must be
         within the  coordinate  ranges  for  the  screen  and,  if
         necessary, are rounded to the nearest values  allowed  for
         the current screen mode.

         Input:  X = new horizontal cursor coordinate
                 y = new vertical cursor coordinate              */

void far _Cdecl get_mouse_position( struct mouse_position far *mp);
      /* Read Cursor Location

         Function gives cursor location.

         Input: none
         Return: (*mp).x = horizontal cursor coordinate
                 (*mp).y = vertical cursor coordinate           */

int  far _Cdecl get_button_status( int button_mask, struct mouse_position far *mp);
      /* Read Cursor Location & status of selected mouse buttons

         Function gives the status of selected mouse buttons, plus cursor
         location.

         Input:  button_mask: LEFT_BUTTON_MASK, RIGHT_BUTTON_MASK,
                              MIDDLE_BUTTON_MASK, ALL_BUTTON_MASK

         Return: button status: is 1 when the button is pressed.
                                is 0 when the button is released.
                 (*mp).x = horizontal cursor coordinate
                 (*mp).y = vertical cursor coordinate */

int  far _Cdecl get_press_state( int button_num, struct mouse_position far *mp);
      /* Read Button Press State & and location

         Function provides status on the specified button, gives the number
         of button presses since the last call, and produces  the  location
         of the cursor at last button press.

         Input:  button_num:  LEFT_BUTTON_MASK, RIGHT_BUTTON_MASK,
                              MIDDLE_BUTTON_MASK, ALL_BUTTON_MASK

         Return: button status: is N when the button is pressed. N number
                                of button presses will always fall in the
                                range of 0 to 32767. Following this func-
                                tion call, the count is reset to zero.
                                is 0 when the button is released.
                 (*mp).x = horizontal cursor coordinate
                 (*mp).y = vertical cursor coordinate */

int  far _Cdecl get_release_state( int button_num, struct mouse_position far *mp);
      /* Read Button Release State

         Function provides status on the specified button, gives the number
         of button releases since the last call, and provides the  location
         of the cursor at the last button release.

         Input:  button_num:  LEFT_BUTTON_MASK, RIGHT_BUTTON_MASK,
                              MIDDLE_BUTTON_MASK, ALL_BUTTON_MASK

         Return: button status: is N when the button is pressed. N  number
                                of button releases will always fall in the
                                range of 0 to 32767. Following this  func-
                                tion call, the count is reset to zero.
                                is 0 when the button is released.
                 (*mp).x = horizontal cursor coordinate
                 (*mp).y = vertical cursor coordinate */

void far _Cdecl set_mouse_style( int hor_hot_spot, int ver_hot_spot, int far *bitmap);
      /* Graphics Mode Cursor Style

         Function defines the style of  the cursor in terms of color, shape,
         and center for the graphics.  This cursor is a 16-by-16 pixel block
         and is defined by two 16-bit arrays (the screen mask  bit  and  the
         cursor mask bit). Cursor coordinates for the hot spot  must  be  in
         the range of -16 to +16.

         Input:  hor_hot_spot = horizontal cursor hot spot
                 ver_hot_spot = vertical cursor hot spot
                 bitmap       = pointer to screen and cursor mask

         Return: none

         Example: define bitmap
                  Define the screen mask
	 static  int bitmap[] = { 0xFFFF,       '1111111111111111
                                  0xFFFF,       '1111111111111111
                                  0xFFFF,       '1111111111111111
                                  0xFFFF,       '1111111111111111
                                  0xFFFF,       '1111111111111111
                                  0xF00F,       '1111000000001111
                                  0x0000,       '0000000000000000
                                  0x0000,       '0000000000000000
                                  0x0000,       '0000000000000000
                                  0x0000,       '0000000000000000
                                  0xF00F,       '1111000000001111
                                  0xFFFF,       '1111111111111111
                                  0xFFFF,       '1111111111111111
                                  0xFFFF,       '1111111111111111
                                  0xFFFF,       '1111111111111111
                                  0xFFFF,       '1111111111111111

                  Define the cursor mask
                                  0x0000,       '0000000000000000
                                  0x0000,       '0000000000000000
                                  0x0000,       '0000000000000000
                                  0x0000,       '0000000000000000
                                  0x0000,       '0000000000000000
                                  0x0000,       '0000000000000000
                                  0x07E0,       '0000011111100000
                                  0x7FFE,       '0111111111111110
                                  0x7FFE,       '0111111111111110
                                  0x07E0,       '0000011111100000
                                  0x0000,       '0000000000000000
                                  0x0000,       '0000000000000000
                                  0x0000,       '0000000000000000
                                  0x0000,       '0000000000000000
                                  0x0000,       '0000000000000000
                                  0x0000 };     '0000000000000000*/

void far _Cdecl get_motion_number( int far *x_num, int far *y_num);
      /* Read Mouse Motion Number

         Function gives the mouse motion number since the last call.
         A positive horizontal number indicates  rightward  movement
         (negative shows leftward movement).   A  positive  vertical
         number indicates downward movement (negative  shows  upward
         movement). The number is always in the range of  -32768  to
         32767.  Overflow is disregarded. Once the call is completed,
         the number is set to 0.

         Input:  none

         Return: *x_num = horizontal number
                 *y_num = vertical number                          */

void far _Cdecl set_motion_number( int x_num, int y_num);
      /* Define Sensitivity (Mouse Motion/Pixel) of Mouse

         Function defines mouse sensitivity as determined by the mouse
         motion/pixel ratio.  This is a way of setting  the  amount of
         cursor motion wanted for mouse movement. These ratios specify
         mouse motion per 8 pixels.  These values must be in the range
         of 1 to 32767.  With a  larger  ratio, the cursor movement is
         shortened for each mouse movement.

         Default values: horizontal ratio - 8  mouse motions to 8 pixels
                         vertical ratio   - 16 mouse motions to 8 pixels

         Note: 1 mouse motion = 1/200 of an inch increment

         Input:  x_num = horizontal mouse motion counts to pixel ratio
                 y_num = vertical mouse motion counts to pixel ratio

         Return: none                                               */

int get_mouse_key( int mouse_type, struct mouse_position far *mp );
          /*
             Input : mouse_type = number of mouse button ( 3 or 2 )
             Output: mp = mouse position
             Return: pressed key code : 1 = left, 2 = right, 3 = moddle */

int _far locator( MOUSE_POSITION _far *mp );
          /*
             Input : NONE
             Output: mp = mouse position
             Return: pressed key ASCII or SCAN code */

void _far set_locator_position( int x, int y );
          /*
             Input : x, y locator and mouse position
             Output: NONE                           */

int  _far ask_replay( WINDOW _far *wp, struct menu_items far *mi,
                      int items_num, int current, int *enter_flag, int kb_replay);
int  _far get_mouse_replay( WINDOW _far *wp, int items_num);
int  _far get_cont_mouse_replay( WINDOW _far *wp, int items_num, int b_m);
int  _far check_position( MOUSE_POSITION _far *mp, WINDOW _far *wp, int items_num);
int  _far check_kb_replay( int kb_replay, struct menu_items far *mi, int items_num);
void _far init_mouse( void );

#endif
