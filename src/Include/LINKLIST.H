/*
**  TAMIC_soft(R) 1990 v1.00
**
**  linklist.h
**
**  FILE : a:\include\linklist.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _LINKLIST_H
 #define _LINKLIST_H

typedef struct
	 {
          int num;     /* link number*/
          int m1;      /* first modul number*/
          int in1;     /* window's first modul number*/
          int m2;      /* second number*/
          int in2;     /* window's second modul number*/
	  } LINK;

struct tagLinkList
	 {
	  LINK                     Link;
	  struct tagLinkList *lpNext;
	  };

typedef struct tagLinkList LINK_LIST;
int       SetLink( LINK *lplnkBuffer );
#endif
