/*
**  TAMIC_soft(R) 1990 v1.00
**
** Declaration of labels for H-programs
**
** FILE: a\include\label_h.h
**
*/

#ifndef _LABEL_H_H
  #define _LABEL_H_H

#define TWP_LABEL        "TAMIC_soft group. TAMIC-H.1990,1991,1992"
#define TURBO_TITLE      "Turbo Analysis of Microwave Integrated Circuits"
#define COPYRIGHT_TITLE  "Copyright  (C) TAMIC-soft.   All rights reserved."

#ifdef _WINDOWS
   #define WNDTITLE_STR "TAMIC-soft (R) H-plane. Version 2.10W."
   #define TITLE_STR "TAMIC-soft (R) H-plane.             Version 2.10W."
   #define MAIN_LABEL "TAMIC-H V 2.10W"
   #define LST_TITLE "TAMIC_soft(R) H-plane waveguide analyzer. Version 2.10W 1990,1991,1992"
   #define WNDCLASSNAME  "WTMC_H"
   #define WND_TITLE "TAMIC H-plane waveguide analyzer. Version 2.10W "
 #else
   #define TITLE_STR "TAMIC-soft (R) H-plane.             Version 2.10."
   #define MAIN_LABEL "TAMIC-H V 2.10"
   #define LST_TITLE "TAMIC_soft(R) H-plane waveguide analyzer. Version 2.10 1990,1991,1992"
#endif

#define ERR_FILE_NAME "tmc_h.err"


#endif
