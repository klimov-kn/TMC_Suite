/*
**  TAMIC_soft(R) 1990 v1.00
**
**  descrptr.h
**
**  FILE : a:\include\descrptr.c
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _DESCRPTR_H
 #define _DESCRPTR_H

typedef struct block_descriptor
        {
         char _far *name;          /* descriptor name*/
         int code;                 /* descriptor code*/
         }BLOCK_DESCRIPTOR;

typedef struct group_descriptor
        {
         char _far *name;          /* descriptor name*/
         int code;                 /* descriptor code*/
         char _far *format;        /* descriptor input format*/
         char _far *outp_format;   /* descriptor output format*/
         char _far *save_format;   /* descriptor save format*/
         Uint size;                /* format delimeters number*/
	 }GROUP_DESCRIPTOR;


#define M_ERROR             -1
#define M_WORNING           -2

                       /* general section descriptor constants*/
#define M_TMC               20
#define M_STEP               0
#define M_END_STEP           1
#define M_OUTPUT            10
#define M_END_OUT           11
#define M_EOF               12
#define M_ATTRIBUTES        16
#define M_END_ATTRIBUTES    17


                        /* general output section constants*/
#define M_FILE              28
#define M_GRAPH_L           45
#define M_GRAPH_K           46
#define M_GRAPH_S           47
#define M_GRAPH_R           48
#define M_GRAPH_D           49

                        /* general attribute section constants*/
#define M_REAL              60
#define M_COMPLEX           61

char _far *get_block_descriptor_name( Uint code );
char _far *get_group_descriptor_name( Uint code );
char _far *get_group_output_format( Uint code );
char _far *get_group_format( Uint code );
char _far *get_group_save_format( Uint code );
Uint _far  get_group_size( Uint code );
int  _far  seach_group_dscr( char _far *string );
int  _far           seach_block_dscr( char _far *string );
int  _far  _fortran prepr_seach_block_dscr( char _far *string );

struct group_descriptor _far  *get_group_dscr_ptr( void );
struct block_descriptor _far  *get_block_dscr_ptr( void );

#endif


