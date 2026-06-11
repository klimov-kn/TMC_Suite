/*
**  TAMIC_soft(R) 1992 v1.00
**
** Decloration of labels for 3D-programs
**
** FILE: ..\include\label\3d.h
**
*/

#ifndef _LABL3D_H
  #define _LABL3D_H

#define TWP_LABEL        "TAMIC_soft group. TAMIC-3D.1992"
#define TURBO_TITLE      "Turbo Analysis of Microwave Integreated Circuites"
#define COPYRIGHT_TITLE  "Copyright  (C) TAMIC_soft.   All rights reserved."

#ifdef _WINDOWS
   #define WNDTITLE_STR "TAMIC-soft (R) H-plane. Version 1.10W."
   #define TITLE_STR "TAMIC-soft (R) H-plane.             Version 1.10W."
   #define MAIN_LABEL "TAMIC-3DDL V 1.10W"
   #define LST_TITLE "TAMIC_soft(R)      H-plane waveguide analizer. Version 1.10W 1990,1991"
   #define WNDCLASSNAME  "WTMC_3DDL"
   #define WND_TITLE "TAMIC 3D Data Loader analizer. Version 1.10W "
 #else
   #define TITLE_STR "TAMIC_soft (R) TMC_3D program. Version 1.00."
   #define MAIN_LABEL "TAMIC-3D V1.00"
   #define LST_TITLE "TAMIC_soft(R)  TAMIC 3D program. Version 1.00 1992"
#endif

#define ERR_FILE_NAME "tmc_3d.err"


#endif
