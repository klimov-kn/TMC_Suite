/*
**  TAMIC_soft(R) 1991,1992 v1.00 07-16-91 10:49am
**
**  dataload.h
**
**  FILE : ..\include\dataload.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _DATALOAD_H
 #define _DATALOAD_H

#pragma pack(1)

#ifndef __3D_POINT
  #define __3D_POINT

typedef struct { double x;
                 double y;
                 double z;
		 } _3D_VECTOR;

typedef _3D_VECTOR  _3D_POINT;

typedef struct
          {
           _3D_POINT   _3D_Max;
           _3D_POINT   _3D_Min;
           } _3D_FRAME;
#endif

typedef struct { WORD      num;         /*  number of point*/
                 WORD      block_num;   /*  number of block*/
                 _3D_POINT p;           /*  point*/
                 }POINT;

struct tagPOINT_LIST { POINT p;                         /*  point structure*/
                       struct tagPOINT_LIST _far *next; /*  next element*/
                       };
typedef struct tagPOINT_LIST POINT_LIST;

typedef struct { WORD        num;       /*  number of line*/
                 int         type;      /*  1 - metall, 0 - internal, <0 - input number*/
                 WORD        block_num; /*  number of block*/
                 POINT _far *p1;        /*  start point*/
                 POINT _far *p2;        /*  second point*/
                 }LINE;

typedef struct { WORD  num;
                 int   type;            /*  1 - metall, 0 - internal, <0 - input number*/
                 WORD  block_num;       /*  number of block*/
                 WORD  p1_num;
                 WORD  p2_num;
                 }LINE_ABS;

struct tagLINE_LIST { LINE l;                         /*  line structure*/
                      struct tagLINE_LIST _far *next; /*  next element*/
                      };
typedef struct tagLINE_LIST LINE_LIST;

struct tagLIST_P { int code;                    /*  object code*/
                   void _far *object;           /*  pointer to object structure*/
                   struct tagLIST_P _far *next; /*  next element*/
                   };
typedef struct tagLIST_P LIST_P;

typedef struct { WORD         num;                /*  number of surferce*/
                 WORD         block_num;          /*  number of block*/
                 int          type;               /*  1 - metall, 0 - internal, <0 - input number*/
                 int          mode;               /*  nothing for typr = {1:0}, mode flag for type < 0*/
                 LIST_P _far *cl;                 /*  input to curve list*/
                 }SURFERCE;

struct tagSURFERCE_LIST { SURFERCE surf;                      /*  surferce*/
                          struct tagSURFERCE_LIST _far *next; /*  next element*/
                         };
typedef struct tagSURFERCE_LIST SURFERCE_LIST;

typedef struct { WORD         num;                /*  number of shape*/
                 WORD         block_num;          /*  number of block*/
                 float        e, m;               /*  dielectric & maghetic constants*/
                 LIST_P _far *sfl;                /*  input to surferce list*/
                 }SHAPE;

struct tagSHAPE_LIST { SHAPE shape;                      /*  shape*/
                       struct tagSHAPE_LIST _far *next;  /*  next element*/
                       };
typedef struct tagSHAPE_LIST SHAPE_LIST;

typedef struct { WORD         num;                 /*  number of block*/
                 int          code;                /*  block type code*/
                 void   _far *modul;               /*  pointer to input data structure*/
                 LIST_P _far *shl;                 /*  input to shape list*/
                 }BLOCK;

struct tagBLOCK_LIST { BLOCK block;                     /*  block*/
                       struct tagBLOCK_LIST _far *next; /*  next element*/
                       };
typedef struct tagBLOCK_LIST BLOCK_LIST;

typedef struct { void _far *data_base;            /*  base pointer to memory*/
                 void _far *db;                   /*  pointer to current db*/
                 POINT_LIST _far *point_first;    /*  input to point linked list*/
                 POINT_LIST _far *point_last;     /*  last element in point linked list*/
                 }_3D_POINT_LIST;

typedef struct { void _far *data_base;            /*  base pointer to memory*/
                 void _far *db;                   /*  pointer to current db*/
                 LINE_LIST _far *line_first;      /*  input to line linked list*/
                 LINE_LIST _far *line_last;       /*  last element in line linked list*/
                 }_3D_LINE_LIST;

typedef struct { void _far *data_base;            /*  base pointer to memory*/
                 void _far *db;                   /*  pointer to current db*/
                 SURFERCE_LIST _far *surf_first;  /*  input to surferce linked list*/
                 SURFERCE_LIST _far *surf_last;   /*  last element in surferce linked list*/
                 }_3D_SURFERCE_LIST;

typedef struct { void _far *data_base;            /*  base pointer to memory*/
                 void _far *db;                   /*  pointer to current db*/
                 }_3D_LIST_P;

typedef struct { void _far *data_base;            /*  base pointer to memory*/
                 void _far *db;                   /*  pointer to current db*/
                 SHAPE_LIST _far *shape_first;    /*  input to shape linked list*/
                 SHAPE_LIST _far *shape_last;     /*  last element in shape linked list*/
                 BLOCK_LIST _far *block_first;    /*  input to block linked list*/
                 BLOCK_LIST _far *block_last;     /*  last element in block linked list*/
                 } _3D_LOAD_LIST;

#define   _LINE_CODE      10
#define   _PLANE_CODE    100
#define   _SURFE_CODE    200
#define   _SHAPE_CODE    300

#ifndef MAX_WINDOW_NUM
 #define MAX_WINDOW_NUM    20
#endif

#ifndef _MODUL_FL
  #define _MODUL_FL
  typedef struct
        {
         char twp_name[FILENAME_MAX];
         int  inv_tabl[MAX_WINDOW_NUM];
         }MODUL_FL;
#endif



void _far * fortran get_3d_data_base_ptr( void );
void _far * fortran get_3dp_data_base_ptr( void );
void _far * fortran get_3dl_data_base_ptr( void );
void _far * fortran get_3ds_data_base_ptr( void );
void _far * fortran get_3dlp_data_base_ptr( void );


void _far * fortran incr_3d_data_base( Uint size );
void _far * fortran incr_3d_data_base_err( Uint size );
void _far * fortran alloc_3d_data_base_mem( Uint size );

void _far * fortran incr_3dp_data_base( Uint size );
void _far * fortran incr_3dp_data_base_err( Uint size );

void _far * fortran incr_3dln_data_base( Uint size );
void _far * fortran incr_3dln_data_base_err( Uint size );

void _far * fortran incr_3ds_data_base( Uint size );
void _far * fortran incr_3ds_data_base_err( Uint size );

void _far * fortran incr_3dlp_data_base( Uint size );
void _far * fortran incr_3dlp_data_base_err( Uint size );

int  _far   fortran init_3d_data_base( void );
void _far   fortran free_3d_data_base( void );

int  _far   fortran init_3dp_data_base( void );
void _far   fortran free_3dp_data_base( void );

int  _far   fortran init_3dln_data_base( void );
void _far   fortran free_3dln_data_base( void );

int  _far   fortran init_3ds_data_base( void );
void _far   fortran free_3ds_data_base( void );

int  _far   fortran init_3dlp_data_base( void );
void _far   fortran free_3dlp_data_base( void );

POINT_LIST _far * fortran get_point_first_ptr( void );
void       _far   fortran set_point_first_ptr( POINT_LIST _far *ptr );
POINT_LIST _far * fortran get_point_last_ptr( void );
void       _far   fortran set_point_last_ptr( POINT_LIST _far *ptr );

LINE_LIST  _far * fortran get_line_first_ptr( void );
void       _far   fortran set_line_first_ptr( LINE_LIST _far *ptr );
LINE_LIST  _far * fortran get_line_last_ptr( void );
void       _far   fortran set_line_last_ptr( LINE_LIST _far *ptr );

SURFERCE_LIST _far * fortran get_surf_first_ptr( void );
void          _far   fortran set_surf_first_ptr( SURFERCE_LIST _far *ptr );
SURFERCE_LIST _far * fortran get_surf_last_ptr( void );
void          _far   fortran set_surf_last_ptr( SURFERCE_LIST _far *ptr );

SHAPE_LIST _far * fortran get_shape_first_ptr( void );
void       _far   fortran set_shape_first_ptr( SHAPE_LIST _far *ptr );
SHAPE_LIST _far * fortran get_shape_last_ptr( void );
void       _far   fortran set_shape_last_ptr( SHAPE_LIST _far *ptr );

BLOCK_LIST _far * fortran get_block_first_ptr( void );
void       _far   fortran set_block_first_ptr( BLOCK_LIST _far *ptr );
BLOCK_LIST _far * fortran get_block_last_ptr( void );
void       _far   fortran set_block_last_ptr( BLOCK_LIST _far *ptr );

LIST_P     _far * fortran alloc_list_p( void );
LIST_P     _far * fortran alloc_list_p_in_surf_list( void );

POINT_LIST _far * fortran alloc_point( POINT _far *p );
POINT      _far * fortran find_point_by_num( Uint num );
POINT_LIST _far * fortran find_point_by_value( POINT _far *bp, POINT_LIST _far *p_out );
Uint       _far   fortran get_point_num_in_list( void );
int        _far   fortran remove_point_from_list( POINT _far *p );
int        _far   fortran check_points_by_value( POINT _far *p1, POINT _far *p2 );

LINE_LIST  _far * fortran alloc_line( LINE _far *l );
LINE       _far * fortran make_line( LINE _far *l, Uint num, Uint num_p1, Uint num_p2 );
LINE       _far * fortran find_line_by_num( Uint num );
LINE_LIST  _far * fortran find_line_by_value( LINE _far *bl, LINE_LIST _far *l_out );
Uint       _far   fortran get_line_num_in_list( void );
Uint       _far   fortran get_line_num_in_surf( SURFERCE _far *surf );
int        _far   fortran remove_line_from_list( LINE _far *l );
int        _far   fortran check_lines_by_value( LINE _far *l1, LINE _far *l2 );

SURFERCE_LIST _far * fortran alloc_surf( SURFERCE _far *l );
SURFERCE      _far * fortran make_surf( SURFERCE _far *surf, Uint surf_num, int curve_code, Uint line_num );
SURFERCE      _far * fortran find_surf_by_num( Uint num );
Uint          _far   fortran get_surf_num_in_list( void );
Uint          _far   fortran get_surf_num_in_shape( SHAPE _far *shape );
SURFERCE_LIST _far * fortran find_surf_by_lines( SURFERCE _far *bl, SURFERCE_LIST _far *l_out );
int           _far   fortran remove_surf_from_list( SURFERCE _far *l );
int           _far   fortran check_surfs_by_lines( SURFERCE _far *s1, SURFERCE _far *s2 );

SHAPE_LIST _far * fortran alloc_shape( SHAPE _far *shape );
SHAPE      _far * fortran make_shape( SHAPE _far *shape, Uint shape_num, int surf_code, Uint surf_num );
SHAPE      _far * fortran find_shape_by_num( Uint num );
Uint       _far   fortran get_shape_num_in_list( void );


BLOCK_LIST _far * fortran alloc_3D_block( BLOCK _far *block );
BLOCK      _far * fortran make_block( BLOCK _far *block, Uint block_num, int shape_code, Uint shape_num );
BLOCK      _far * fortran find_block_by_num( Uint num );

#endif
