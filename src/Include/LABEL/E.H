/*
**  TAMIC_soft(R) 1990 v1.00
**
** Declarations of labels for E-programms
**
** FILE: a\include\label_e.h
**
*/

#ifndef _LABEL_E_H
  #define _LABEL_E_H

#define TURBO_TITLE      "Turbo Analysis of Microwave Integrated Circuits"
#define COPYRIGHT_TITLE  "Copyright  (C) TAMIC-soft.   All rights reserved."

#define TWP_LABEL        "TAMIC_soft group. TAMIC-E.1990,1991,1992"

#ifdef _WINDOWS
   #define WNDTITLE_STR "TAMIC-soft (R) E-plane. Version 2.10W."
   #define TITLE_STR "TAMIC-soft (R) E-plane.             Version 2.10W."
   #define MAIN_LABEL "TAMIC-E V 1.10W"
   #define LST_TITLE "TAMIC_soft(R) E-plane waveguide analyzer. Version 2.10W 1990,1991,1992"
   #define WNDCLASSNAME  "WTMC_E"
   #define WND_TITLE "TAMIC E-plane waveguide analyzer. Version 2.10W "
 #else
   #define TITLE_STR "TAMIC-soft (R) E-plane.             Version 2.10."
   #define MAIN_LABEL "TAMIC-E V 2.10"
   #define LST_TITLE "TAMIC_soft(R) E-plane waveguide analyzer. Version 2.10 1990,1991,1992"
#endif

#define ERR_FILE_NAME "tmc_e.err"

#endif
