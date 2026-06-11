/*
**  TAMIC_soft(R) 1991 v1.00
**
** Decloration of labels for Microstrip analyzer program
**
** FILE: a\include\label\m.h
**
*/

#ifndef _LABEL_M_H
  #define _LABEL_M_H

#define TWP_LABEL        "TAMIC_soft group. TAMIC-M.1992,1993"
#define TURBO_TITLE      "Turbo Analysis of Microwave Integrated Circuits"
#define COPYRIGHT_TITLE  "Copyright  (C) TAMIC-soft.   All rights reserved."

#ifdef _WINDOWS
   #define WNDTITLE_STR "TAMIC-soft (R) Microstrip analyzer. Version 3.03W."
   #define TITLE_STR "TAMIC-soft (R)  Microstrip analyzer. Version 3.03W."
   #define MAIN_LABEL "TAMIC-M V 3.03W"
   #define LST_TITLE "TAMIC_soft(R)     Microstrip analyzer.  Version 3.03W 1992, 1993"
   #define WNDCLASSNAME  "WTMC_M"
   #define WND_TITLE "TAMIC Microstrip analyzer.  Version 3.03W "
 #else
   #define TITLE_STR "TAMIC-soft (R) Microstrip analyzer.  Version 3.12."
   #define MAIN_LABEL "TAMIC-M V 3.12"
   #define LST_TITLE "TAMIC_soft(R)  Microstrip analyzer. Version 3.12 1992, 1993"
#endif

#define ERR_FILE_NAME "tmc_m.err"


#endif
