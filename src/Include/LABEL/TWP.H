/*
**  TAMIC_soft(R) 1991 v1.00
**
** Decloration of labels for HE-program
**
** FILE: a\include\label_w.h
**
*/

#ifndef _LABEL_W_H
  #define _LABEL_W_H

#define TWP_LABEL        "TAMIC_soft group. TAMIC-W.1991"
#define TURBO_TITLE      "Turbo Analysis of Microvawe Integrated Circuites"
#define COPYRIGHT_TITLE  "Copyright  (C) TAMIC-soft.   All rights reserved."

#ifdef _WINDOWS
   #define WNDTITLE_STR "TAMIC-soft (R) HE-prog. Version 1.00W."
   #define TITLE_STR "TAMIC-soft (R) HE-prog.             Version 1.00W."
   #define MAIN_LABEL "TAMIC-W V 1.00W"
   #define LST_TITLE "TAMIC_soft(R)      HE-prog. waveguide analizer. Version 1.00W 1991"
   #define WNDCLASSNAME  "WTMC_W"
   #define WND_TITLE "TAMIC HE-prog. waveguide analizer. Version 1.00W "
 #else
   #define TITLE_STR "TAMIC-soft (R) HE-prog.             Version 1.00."
   #define MAIN_LABEL "TAMIC-W V 1.00"
   #define LST_TITLE "TAMIC_soft(R)      HE-prog. waveguide analizer. Version 1.00 1991"
#endif

#define ERR_FILE_NAME "tmc_twp.err"


#endif
