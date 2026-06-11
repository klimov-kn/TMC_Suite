/*
**  TAMIC_soft(R) 1992 v1.00
**
**  elbase_m.h
**
**  FILE : ..\tamic\indan\m\elbase_m.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _ELBASE_M_H
 #define _ELBASE_M_H

 #ifndef _TMCLIMIT_H
  #include <tmclimit.h>
 #endif
 #ifndef _LINKLIST_H
  #include <linklist.h>
 #endif

 #ifndef _INPUT_DATA_DEF
  #include <postpro.h>
 #endif

typedef struct
	{
	 int        user_num;                   // user defined element number
	 int        code;                       // element code
	 int        inp_num;                    // number of input in element
	 int        node_num[ MAX_INPUT_NUM ];  // global matrix node number array
	 int        node_code[ MAX_INPUT_NUM ]; // node code n < 0 - number of input in global matrix
						//               0 - internal node
	 INPUT_DATA inp_data;
	 void _far *data;                       // pointer to element data
	 } ELEMENT;

struct  tagELEMENT_BASE_LIST
	 {
	   ELEMENT element;
	   struct tagELEMENT_BASE_LIST _far *lpNext;
	  };
typedef struct  tagELEMENT_BASE_LIST ELEMENT_BASE_LIST;

typedef struct
	 {
	  int num; // real element number
	  ELEMENT element[ MAX_ELEMENT_NUM ];
	  }ELEMENT_BASE;

 #ifndef _ELEM_VAL
  #include <m\run_m.h>
 #endif


typedef struct
	 {
	  _real            frq;                  // freq.
	  _ELEM_VAL  _far *datas;                // data ptr for S matrix element
	  } FRQS;

typedef struct
	 {
	  char       file_name[ FILENAME_MAX ]; // text file name
	  FILE _far *in;                        // FILE ptr
	  int        frq_num;                   // freq  number
	  FRQS _far *data;                      // data ptr
	  } FILE_TXT;


ELEMENT  _far  *_fortran alloc_ElementBlock( int wBlockNum );

void _far  _fortran el_b_free( void );
ELEMENT  _far  *_fortran el_b_set_bl( int block_number );
int  _far  _fortran el_b_set_code( int code );
int  _far  _fortran el_b_get_code( void );
int  _far  _fortran el_b_set_inp_num( int inp_num );
int  _far  _fortran el_b_get_inp_num( int inp_num );
int  _far  _fortran el_b_set_node( void );
void _far *_fortran el_b_set_data( size_t size, ELEMENT _far *lpElement );
int  _far  _fortran eb_l_setall( void );
int  _far  _fortran el_b_list_free( void );
int  _far _fortran el_b_get_nbl( int n_bl );
int  _far _fortran el_b_del_num( int el1, int num1, int el2, int num2 );
int  _far _fortran el_b_del_inp( int el1, int num1 );
int  _far _fortran search_inp_num( int code );
int  _far _fortran el_b_set_all( void );
int  _far _fortran el_b_set_data_all( void );
int  _far _fortran el_set_data( ELEMENT _far *el );
int  _far _fortran el_b_set_link_all( void );
int  _far _fortran el_b_set_link_all_node( void );
int _far _fortran el_b_set_link_all_input_node( void );
ELEMENT_BASE  _far *_fortran el_b_get_top( void );
ELEMENT       _far *_fortran el_b_get_element( void );
int _far _fortran el_b_SetLink( LINK _far *lnk_buf );
void _far fortran output_eb( ELEMENT_BASE _far *eb );

int _far _fortran el_read_file( ELEMENT _far *lpElement );
int _far _fortran el_read_file_line( ELEMENT _far *lpElement );
int _far _fortran el_read_file_txt( ELEMENT _far *lpElement );
int _far _fortran el_read_file_element( ELEMENT _far *lpElement );
int _far _fortran search_txt_file_name( ELEMENT _far *lpElement );
int _far _fortran search_lin_file_name( ELEMENT _far *lpElement );
void _far _fortran el_file_txt_free( ELEMENT _far *fl );
void _far _fortran el_file_free( ELEMENT _far *lpElement );
void _far _fortran el_file_line_free( ELEMENT _far *lpElement );
void _far _fortran el_free( ELEMENT el );

int _far _fortran set_file_data_in_ymt( void );
int _far _fortran free_file_data( void );
_ELEM_VAL _far *_fortran y_file_tmc_mm( ELEMENT _far *lpElement, FILE_TXT _far *lpFl );

#endif
