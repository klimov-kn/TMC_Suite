/*
**  TAMIC_soft(R) 1991,1992 v1.00
**
** Decloration of labels for HE-program
**
** FILE: a\include\label_w.h
**
*/

#ifndef _LABEL_W_H
  #define _LABEL_W_H

#define TWP_LABEL        "TAMIC_soft group. TAMIC-W.1992"
#define TURBO_TITLE      "Turbo Analysis of Microvawe Integrated Circuites"
#define COPYRIGHT_TITLE  "Copyright  (C) TAMIC-soft.   All rights reserved."

#ifdef _WINDOWS
   #define WNDTITLE_STR "TAMIC-soft (R) HE-prog. Version 2.10W."
   #define TITLE_STR "TAMIC-soft (R) HE-program.          Version 2.10W."
   #define MAIN_LABEL "TAMIC-W V 2.10W"
   #define LST_TITLE "TAMIC_soft(R)    H&E-plane waveguide analizer. Version 2.10W 1992"
   #define WNDCLASSNAME  "WTMC_W"
   #define WND_TITLE "TAMIC HE-prog. waveguide analizer. Version 2.10W "
 #else
   #define TITLE_STR "TAMIC-soft (R) HE-program.          Version 2.10."
   #define MAIN_LABEL "TAMIC-W V 2.10"
   #define LST_TITLE "TAMIC_soft(R)    H&E-plane waveguide analizer. Version 2.10 1992"
#endif

#define ERR_FILE_NAME "tmc_w.err"


#endif
