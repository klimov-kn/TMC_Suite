/*
**  TAMIC_soft(R) 1992 v1.00
**
**  load_db.h
**
**  FILE : a:\include\load_db.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _LOAD_DB_H
 #define _LOAD_DB_H

typedef struct
	 {
          void  *start;    /* pointer to begin of memory block*/
          void  *current;  /* pointer to free memory block*/
	  } DATA_BASE;

int    init_data_base( void );
void   free_data_base( void );
int    add_init_data_base( void );
void   add_free_data_base( void );

void * get_data_base_ptr( void );
void * incr_data_base( size_t size );
void * incr_data_base_err( size_t size );
void * alloc_DB_mem( size_t size );

#endif
