/*
**  TAMIC_soft(R) 1992 v1.00
**
**  attribut.h
**
**  FILE : a:\include\attribut.c
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _ATTRIBUT_H
 #define _ATTRIBUT_H

#define ATTR_NAME_SIZE  10

struct tagAttribute
	 {
          char                      szName[ATTR_NAME_SIZE];    /* attribute name*/
          int                       nCode;                     /* attribute code*/
          void                _far *lpAttrValue;               /* pointer to attribute value*/
          struct tagAttribute _far *lpNext;                    /* pointer to next attribute*/
          struct tagAttribute _far *lpPrevios;                 /* pointer to previous attribute*/
	  };

typedef struct tagAttribute ATTRIBUTE;

void      _far   pascal SetFirstAttributePointer( ATTRIBUTE _far *lpFirstAttribute );
void      _far   pascal SetLastAttributePointer( ATTRIBUTE _far *lpLastAttribute );

ATTRIBUTE _far * pascal GetFirstAttributePointer( void );
ATTRIBUTE _far * pascal GetLastAttributePointer( void );

ATTRIBUTE _far * _fortran SetAttribute( int nCode,  char _far *lpszName );

void _far * _fortran Get_attribute_value(   char _far *lpszName, int _far *nCode );

#endif
