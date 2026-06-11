/*
**  TAMIC_soft(R) 1991,1992 vR1.02
**
** Declarations of labels for IC-program
**
** FILE: \include\label\ic.h
**
*/

#ifndef _LABEL_IC_H
  #define _LABEL_IC_H

#define TWP_LABEL        "TAMIC_soft group. TAMIC-IC. 1991,1992,1993,1994,1995"
#define TURBO_TITLE      "Turbo Analysis of Microwave Integrated Circuits."
#define COPYRIGHT_TITLE  "Copyright  (C) TAMIC-soft.  All rights reserved."

#ifndef _NDP2
  #define TITLE_STR  "TAMIC-soft (R) IC-analyzer.       Version R5.12."
  #define MAIN_LABEL "TAMIC-IC V R5.12"
  #define LST_TITLE  "TAMIC_soft(R)      Integrated Circuit analyzer. Version R5.12 1991-1995"
#else
  #define TITLE_STR  "TAMIC-soft (R) IC-analyzer.      Version R5.12p."
  #define MAIN_LABEL "TAMIC-IC V R5.12p"
  #define LST_TITLE  "TAMIC_soft(R)     Integrated Circuit analyzer. Version R5.12p 1991-1995"
#endif

#define TMC_I          "TMC_I"

#define RGF_LABEL      "RGF"     /* input parameters file label and extension */
#define RGF_VERSION     212

#define GSM_LABEL      "GSM"     /* general sums file label */
#define GSM_VERSION     306

#define IMP_LABEL      "IMP"    /* surface and lumped elements impedances file extension */

#define ERR_FILE_NAME  "TMC_I.ERR"    /* error file name */
#define O_STR      "TMC_I.OPT"    /* options data file name */
#define OT_STR     "TMC_IT.OPT"   /* options data file name for text mode */
#define H_STR      "TMC_I.HLP"    /* help data file name */

#endif
