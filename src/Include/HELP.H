/*
**  TAMIC_soft(R) 1989, 1991 v1.00
**
** Macros and decloration for HELP_H programm
**
** FILE: a\include\help.h
**
*/

                              /* help page user list*/
#define  PREV_HELP              0
#define  START_HELP_PAGE       10

#define  LEGEND_HELP         2000

#define  FILE_M_HELP         2010

#define  OUT_MENU_HELP       2080

#define  FM_LOAD             2110
#define  FM_PREVIOS          2210
#define  FM_SAVE             2310
#define  FM_SAVE_AS          2410
#define  FM_PRINT_W          2510
#define  FM_DOS_SHELL        2610
#define  FM_EXIT             2710

#define  EDIT_M_HELP         3010
#define  EM_PARMS            3110
#define  EM_TOPOLOGY         3210
#define  EM_TEXT             3310

#define  VIEW_M_HELP         4010
#define  VM_OUTPUT           4110
#define  VM_TOPOLOGY         4210
#define  VM_LISTING          4310

#define  RUN_M_HELP          5010
#define  RM_STARTALL         5110
#define  RM_RESTARTALL       5210
#define  RM_STARTSTEP        5310
#define  RM_RESTARTSTEP      5410
#define  RM_SKIPSTEP         5510
#define  RM_BACKSTEP         5610

#define  CONFIG_M_HELP       6010
#define  CM_EDITOR           6110
#define  EDITOR_C_HELP       6210

#define  CM_MODEL            6210
#define  MODEL_FM_C_HELP     6220
#define  MODEL_LA_C_HELP     6230

#define  CM_TOPOLOGY         6310
#define  CM_OUTPUT           6410

#define  CM_PRINTWINDOW      6510
#define  PRINT_W_C_HELP      6520
#define  PRINT_F_C_HELP      6530

#define  CM_LIMITS           6610
#define  CM_DEBUG            6710
#define  CM_DIRECTORY        6810
#define  CM_SAVEOPTIONS      6910
#define  CM_RETRIEVEOPTIONS  6910

#define  UTILITY_M_HELP      7010
#define  UM_RUNDOSCOMMAND    7110
#define  UM_CALCULATER       7210
#define  CALC_HELP           7220

#define  HELP_M_HELP         8010
#define  HM_CONTEXT          8110
#define  HM_TABLES           8210

#define  LN_PARMS            8220
#define  LN_PARMS_FRQS       8225
#define  LN_PARMS_FUNIT      8230
#define  LN_PARMS_LUNIT      8235
#define  LN_PARMS_DELTA      8240
#define  LN_PARMS_ACURACY    8245
#define  LN_PARMS_HEIGHT     8250
#define  LN_PARMS_TOLERANCE  8255

#define  LN_TOPOL            8260
#define  LN_TOPOL_INPUT      8265
#define  LN_TOPOL_WAVEG      8270
#define  LN_TOPOL_POLY       8275
#define  LN_TOPOL_GPC1       8280
#define  LN_TOPOL_GPC2       8285
#define  LN_TOPOL_RING       8290

#if defined _MODEL_IC
#define  LN_TOPOL_RECT       8270
#define  LN_TOPOL_LUMPED     8295
#endif

#if (defined _MODEL_E)||(defined _MODEL_H)||(defined _MODEL_W)
#define  LN_TOPOL_WAVEG      8270
#define  LN_TOPOL_OPSTUB     8295
#define  LN_TOPOL_SHSTUB     8300
#define  LN_TOPOL_FILE       8305
#endif

#ifdef _MODEL_M
#define  LN_TOPOL_INTLE          8270
#define  LN_TOPOL_EXTLE          8290
#define  LN_TOPOL_LUMPE          8298

#define  LN_TOPOL_MIC_LINE       8271
#define  LN_TOPOL_MIC_OPEN_END   8275
#define  LN_TOPOL_MIC_GAP        8279
#define  LN_TOPOL_MIC_PAZ        8283
#define  LN_TOPOL_MIC_STEP       8284
#define  LN_TOPOL_MIC_BEND       8285
#define  LN_TOPOL_MIC_T_JUNCT    8286

#define  LN_TOPOL_FILE           8291
#define  LN_TOPOL_FILE_LINE      8292
#define  LN_TOPOL_FILE_TXT       8293
#define  LN_TOPOL_ELEMENT        8294
#define  LN_TOPOL_COMP_ELEMENT   8295

#define  LN_TOPOL_R              8299
#define  LN_TOPOL_G              8300
#define  LN_TOPOL_RL             8301
#define  LN_TOPOL_RC             8302
#define  LN_TOPOL_GL             8303
#define  LN_TOPOL_GC             8304
#endif

#define  LN_LINK             8310

#define  LN_OUTPUT           8330
#define  LN_OUTPUT_FILE      8335
#define  LN_OUTPUT_INPUT     8340
#define  LN_OUTPUT_GRAPG     8345

#define  LN_ATTRIB           8410

#define  LN_PACKG            8430

#define  LN_PREPR            8450
#define  LN_PREPR_DEF        8451
#define  LN_PREPR_INC        8460
#define  LN_EXPRES           8470

#define  OUT_CHANG_HELP      9010
#define  OUT_ADD_HELP        9020
#define  OUT_DEL_HELP        9030
#define  OUT_X_HELP          9040
#define  OUT_Y_HELP          9050
#define  OUT_CONF_HELP       9060
#define  OUT_LEG_HELP        9070
#define  OUT_LEGEND_HELP     9075
#define  OUT_PRINT_HELP      9080
#define  OUT_EXIT_HELP       9090

#define HELP_STACK_SIZE      20
#define HELP_PAGE_NUM        10
#define HELP_STRING_LENGTH   50
#define PAGE_STRING_NUM      12

#define STRING_SIZE      80

#define _PAGE_            0
#define _NEXT_            1
#define _END_             2

#define _NEXT_ITEM        0
#define _N_P_ITEM         1
#define _PREV_ITEM        2


struct page_pointer
           {
            char far *sp[PAGE_STRING_NUM +1];  /* string pointers*/
            int page_num;                      /* cuurent page number*/
            };


struct page_buffer
           {                  /* geheral character buffer*/
            char buffer[HELP_PAGE_NUM*(PAGE_STRING_NUM +1)*(HELP_STRING_LENGTH +1)];
            struct page_pointer pp[HELP_PAGE_NUM];  /* one struct for one page*/
            char far *ptr;                          /* pointer to free space in buffer*/
            };


struct help_buffer
           {
            int start_page;               /* start help page*/
            int max_page_num;             /* number of page on topic*/
            struct page_buffer far *pb;   /* pointer to page buffer*/
            };

void _far set_help_file_name( const char _far *name );
void _far set_help_path( const char _far *path );
char _far *get_help_path( void );
void _far set_help_page( int page );
void _far set_help_font_style( Uint style );

void _far drow_help_wn( WINDOW _far *l0_wndw,
                        WINDOW _far *l1_wndw,
                        WINDOW _far *l2_wndw,
                        int item_num,
                        char _far **dn_mess );
void _far init_help_window( int x_pos, int y_pos,
                            WINDOW *l0_wndw,
                            WINDOW *l1_wndw,
                            WINDOW *l2_wndw );
void _far help_p( void );
FILE _far *open_help_file( char _far *name );
int  _far init_help_buffer( FILE _far *hf, char _far *help_file_name, struct help_buffer _far *hb );
void _far fill_page_buff( int counter, struct page_buffer _far *pb, FILE _far *hf, char _far *name,
                          int _far *page_num);
int  _far fgets_err( char _far *buff, int length, FILE _far *hf, char _far *name );
void _far put_help_page( int count, struct page_buffer _far *pb, struct window _far *wn );
int  _far ask_help_reply(int menu_code, struct window _far *wi, struct menu_items _far *mi );

void _far fortran ln_prep_p( void );


