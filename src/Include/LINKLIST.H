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
	  struct tagLinkList _far *lpNext;
	  };

typedef struct tagLinkList LINK_LIST;

LINK_LIST _far * fortran get_link_first_ptr( void );
void      _far   fortran set_link_first_ptr( LINK_LIST _far *ptr );
LINK_LIST _far * fortran get_link_last_ptr( void );
void      _far   fortran set_link_last_ptr( LINK_LIST _far *ptr );

LINK_LIST _far * fortran alloc_LINK( void );
int       _far   fortran SetLink( LINK _far *lplnkBuffer );
int       _far   fortran CheckLink( void );

int       _far   fortran copy_link_list( LINK_LIST _far **ll_buff );
#endif
