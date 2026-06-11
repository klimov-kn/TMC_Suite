/*
**  TAMIC_soft(R) 1990 v1.00
**
**  peditdec.h
**
**  FILE : a:\include\peditdec.c
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _PEDITDEC_H
 #define _PEDITDEC_H

struct editor_parm_list
        {
         float f0, df;        // start frequence, frequence step
         int nf;              // number of frequences
         int frq_flag;        // frequence update flag
         float acuracy;       // acurace
         int acur_flag;       // acurace update flag
         float delta, dx, dy; // grid steps
         int delta_flag;      // delta update flag
         float height;        // waveguide height
         int height_flag;     // height update flag
         };

typedef struct editor_parm_list EDITOR_PARM_LIST;

#endif
