#define FATAL_MEM_ALLOC_ERROR      0001

#define MEM_ALLOC_ERROR            1001

#define TMP_FILE_OPEN_ERROR        2001
#define TMP_FILE_WRITE_ERROR       2002
#define TMP_FILE_READ_ERROR        2003
#define TPL_FILE_OPEN_ERROR        2004
#define ERROR_OPEN_LISTING         2005
#define NO_NEED_SAVE               2006

#define OPEN_HELP_FILE_ERR         2100
#define BAD_HELP_FILE              2101
#define TO_MANY_STRING_HELP        2102
#define READ_DATA_FILE_ERR         2103
#define DATA_HELP_FILE_END         2104
#define HELP_PAGE_NOT_FOUND        2105
#define HELP_STACK_UNDERFLOW       2106
#define HELP_STACK_OVERFLOW        2107

                               /* TWP-file  error codes*/
#define ERROR_OPEN_TWP             2200
#define BAD_TWP_FILE               2201
#define TWP_FILE_NOT_OPEN          2210
#define BAD_TWP_FILE_VER           2211
#define BAD_TWP_FILE_MODEL         2221

#define ERROR_OPEN_Y               2202
#define BAD_Y_FILE_EXT             2203
#define BAD_Y_DELTA                2204
#define BAD_Y_FRQ_NUM              2205
#define BAD_Y_FRQ_VAL              2206
#define BAD_Y_FILE                 2207

             /* S-matrix file error codes */
#define ERROR_WRITE_S              2300
#define ERROR_READ_S               2301
#define ILLEGAL_CURVE_INDEX_CORR   2302
#define ILLEGAL_CURVE_INDEX        2303
#define BAD_S_FILE_NAME            2304
#define ERROR_OPEN_S               2305
#define SMT_LABEL_ERROR            2306
#define SMT_VERSION_ERROR          2307
#define SMT_FRQ_POINT              2308
#define SMT_FRQ_MEMORY             2309
#define INTERP_DIFF_MATRIX         2310
#define ERROR_WRITE_S_ITP          2311
#define INTER_ROOT_ERR             2312
#define SMT_INTER_TABLE            2313

                                /* expression error codes*/
#define ERROR_EXPR_OVERF_EXP       2401
#define ERROR_EXPR_ARG_LOG         2402
#define ERROR_EXPR_OVERF_SIN       2403
#define ERROR_EXPR_OVERF_COS       2404
#define ERROR_EXPR_OVERF_TAN       2405
#define ERROR_EXPR_OVERF           2406
#define ERROR_EXPR_ARG_SQRT        2407
#define ERROR_EXPR_OVERF_SINH      2408
#define ERROR_EXPR_OVERF_COSH      2409
#define ERROR_EXPR_OVERF_TANH      2410
#define ERROR_EXPR_ARG_ASIN        2411
#define ERROR_EXPR_ARG_ACOS        2412
#define ERROR_EXPR_OVERF1          2413
#define ERROR_EXPR_ARG_LOG10       2414
#define ERROR_EXPR_OVERF_X_Y       2415
#define ERROR_EXPR_FOR_X_Y         2416
#define ERROR_EXPR_BRACKET         2417
#define ERROR_EXPR_FUNCTION        2418
#define ERROR_EXPR_EXPR            2419
#define ERROR_EXPR_FLOAT_OVERF     2420

                                /* Preprocessor error codes*/
#define ERROR_PREPR_BUF_END        2501
#define ERROR_PREPR_ABN            2502
#define ERROR_PREPR_FACT_PARM      2505
#define ERROR_PREPR_MISSED_SEPAR   2506
#define ERROR_PREPR_OPEN_INC       2507
#define ERROR_PREPR_END_BUF_MEM    2501
#define ERROR_PREPR_MISSED_FORM_P  2509
#define ERROR_PREPR_REDEC          2510
#define ERROR_PREPR_INIT           2511
#define ERROR_PREPR_FOPEN_FILE     2512
#define ERROR_PREPR_LONG_STRING    2513
#define ERROR_PREPR_MIS_MAC_NAME   2514
#define PREPR_FILE_NAME_LONG       2515
#define ERROR_PREPR_MASKA_NUM      2516
#define ERROR_PREPR_FORMAL_NUM     2517

#define COMMAND_COM_ERROR          3001
#define DOS_ERROR_MESS             3002

#define BAD_DRIVE_ERR              4001
#define BAD_PATH_ERR               4002
#define PATH_TOO_LONG_ERR          4003
#define FILE_NAME_LONG_ERR         4004
#define FILE_NAME_EXIST_ERR        4005
#define END_OF_FL_ERR              4006
#define NO_PREV_FILE_ERR           4007

#define UNEXP_END_OF_FILE          5001
#define UNKNOWN_STRING_ERR         5002
#define BAD_BLOCK_DSCR_ERR         5003
#define UNEXP_EOF_NO_STEP          5004
#define MISSED_BLOCK_DESCR         5005
#define BAD_GROUP_DSCR_ERR         5006
#define ILLEGAL_NUM_OF_PARM        5007
#define ILLEGAL_INV_NUM            5008
#define TOO_MUCH_INP_TABL_INP      5009
#define ILLEGAL_INP_TABLE_INP      5010
#define ILLEGAL_INP_ADRESS         5011
#define ACUR_ILLEGAL_ORDER         5012
#define DUPLICATED_POINT           5013
#define MISSING_STEP_DSCR          5014
#define DUPLICATED_STEP_DSCR       5015
#define MISSING_MODEL_DSCR         5016
#define CONNECTED_INPUTS           5017


#define FRQ_BANDWIDTH_ERR          5101
#define BAD_F0_PARM                5102
#define BAD_DF_PARM                5103
#define BAD_NF_PARM                5104
#define BAD_ACURACY_PARM           5105
#define BAD_HEIGHT_PARM            5106
#define BAD_DELTA_PARM             5107
#define UNKNOWN_FRQ_UNIT           5108
#define UNKNOWN_LENGTH_UNIT        5109
#define ATTRIB_NAME_ERR            5110
#define ATTRIB_NOT_FOUND           5111
#define BAD_FREQUENCY              5112
#define EQUAL_FREQUENCIES          5113

#define OPEN_RGF_FILE              5400
#define WRITE_RGF_FILE             5401
#define RGF_READ_ERROR             5402
#define RGF_LABEL_ERROR            5403


#if defined _MODEL_E || defined _MODEL_H

 #define ILLEGAL_WAVEG_A           5201
 #define ILLEGAL_WAVEG_B           5202
 #define ILLEGAL_WAVEG_EPS         5203
 #define ILLEGAL_WAVEG_MU          5204
 #define ILLEGAL_WAVEG_MODE        5205
 #define ILLEGAL_WAVEG_LWIN_CODE   5206
 #define ILLEGAL_WAVEG_LX1         5207
 #define ILLEGAL_WAVEG_LX2         5208
 #define ILLEGAL_WAVEG_LX1X2       5209
 #define ILLEGAL_WAVEG_LWIN_NUM    5210
 #define ILLEGAL_WAVEG_RWIN_NUM    5211
 #define ILLEGAL_WAVEG_LWIN_RWIN   5212
 #define ILLEGAL_WAVEG_RWIN_CODE   5213
 #define ILLEGAL_WAVEG_RX1         5214
 #define ILLEGAL_WAVEG_RX2         5215
 #define ILLEGAL_WAVEG_RX1X2       5216

 #define ILLEGAL_INPUT_A           5221
 #define ILLEGAL_INPUT_B           5222
 #define ILLEGAL_INPUT_EPS         5223
 #define ILLEGAL_INPUT_MU          5224
 #define ILLEGAL_INPUT_INPNUM      5225
 #define ILLEGAL_INPUT_INPMODE     5226
 #define ILLEGAL_INPUT_MODE        5227

 #define ILLEGAL_STUB_A            5231
 #define ILLEGAL_STUB_B            5232
 #define ILLEGAL_STUB_EPS          5233
 #define ILLEGAL_STUB_MU           5234
 #define ILLEGAL_STUB_MODE         5235

#endif

#ifdef _MODEL_IC
 #define WRONG_PACKAGE_DIM          5200
 #define SIDEWALLS_NOT_SUPP         5201
 #define TOP_BOT_NOT_SUPP           5202
 #define WRONG_LAYER_DESCR          5203
 #define LAYER_TYPE_NOT_SUPP        5204
 #define WRONG_TOP_BOT_DESCR        5205
 #define WRONG_SIDEWALLS_DESCR      5206
 #define PACKAGE_NOT_DEF            5207
 #define WRONG_LAYER_SEQUENCE       5208
 #define WRONG_DESCR_SEQUENCE       5209
 #define PACK_DIM_NE_INT_DD         5210
 #define WRONG_RECT_CUR_DIR         5211
 #define WRONG_RECT_RED_COEF        5212
 #define REDUCT_NOT_PRESENT         5213
 #define WRONG_LUMPED_CUR_DIR       5214
 #define WRONG_INPUT_CUR_DIR        5215
 #define WRONG_INPUT_NUM            5216
 #define WRONG_LUMPED_NUMER         5217
 #define STRUCT_WITHOUT_INP         5218
 #define TOO_MANY_INPUTS            5219
 #define WRONG_INPUTS_NUMER         5220
 #define WRONG_INPUT_LINE_DIR       5221
 #define WRONG_POLY_CUR_DIR         5222
 #define WRONG_POLY_RED_COEF        5223
 #define POINT_OUT_OF_REG           5224
 #define TOO_FEW_POLY_POINT         5225
 #define UNKNOWN_FILE_STATUS        5226
 #define WRONG_REG_ORDER            5227

 #define INCORR_IMPED_VALUE         5228

 #define INCORR_HEIGHT_VALUE        5229
 #define INCORR_EPSILON_VALUE       5230
 #define INCORR_EPSILON_DEF         5231
 #define INCORR_MU_VALUE            5232
 #define INCORR_MU_DEF              5233
 #define INCORR_DD_VALUE            5234

 #define TOO_LITTLE_LM_TO_DD        5235

 #define LAYER_AT_CLOSE_TOP         5237
 #define LAYER_AT_CLOSE_BOTTOM      5238
 #define TOO_MANY_TLAYERS           5239
 #define NOT_ENAUGH_MEDIA           5240

 #define W_ILLEGAL_NUM              5250
 #define WINDOW_OF_ZERO_AREA        5251
 #define WINDOW_OUT_SDW             5252
 #define LEFT_RIGHT_CONFUSE         5253
 #define BOTTOM_TOP_CONFUSE         5254
 #define TOPOL_NOT_DEFINED          5255
 #define LAYER_REF_NOT_FOUND        5256
 #define EMPTY_LAYER                5257
 #define TLAYER_NOT_REF             5258

 #define DIFF_LN_AT_FLOORS          5259
 #define DIFF_INP_AT_FLOORS         5260
 #define TOO_MANY_MODES             5261
 #define TINY_INP_REGION            5262

 #define WRONG_LINE_INP_NUM         5271
 #define INP_REG_DONT_TOUCH         5272
 #define INP_NOT_BALANCED           5273
 #define LINE_REG_DESCR             5274
 #define UNSUITABLE_LINE_REG        5275
 #define INSUFF_LINE_LENGTH         5276
 #define INP_REG_DIFF_LENGTH        5277
 #define LN_DIFF_NUM_REG            5278
 #define LN_DIFF_LENGTH_OF_REG      5279
 #define LN_REG_IN_DIFF_LAYERS      5280
 #define LN_DIFF_REGIONS            5281
 #define LN_DIFF_REG_SHIFT          5282
 #define LN_SURF_IMPEDANCE          5283
 #define LN_TOO_FEW_RECT            5284
 #define ONLY_ONE_LAYER_NOW         5285

 #define TOO_LITTLE_RECT            5286
 #define RECT_FOR_INP_ABSENT        5287


 #define CAN_NOT_CHECK_INV          5300
 #define SINGULAR_MATRIX            5301

 #define INAPPR_LINE_LENGTH         5302
 #define HIGH_IMPEDANCE_ERR         5303
 #define HIGH_Y_MATR_ERR            5304
 #define HIGH_MODE_CHAR_ERR         5305
 #define RESONANCE_IN_VOLUME        5306
 #define NORMALIZATION_ERROR        5307
 #define FROM_Z_TO_S_ERROR          5308
 #define TOO_EVANESCENT_MODE        5309

 #define RGF_OTHER_VERSION          5404
 #define RGF_FRQ_POINT              5405
 #define MISSED_FILE_NAME           5406
 #define RGF_SIDEWALL_FLAG          5407
 #define RGF_TOP_WALL_TYPE          5408
 #define RGF_BOT_WALL_TYPE          5409
 #define RGF_LAYER_NUMBER           5410
 #define RGF_LAYER_CODE             5411
 #define RGF_LAYER_TYPE             5412
 #define RGF_MEDIA_HEIGHT           5413
 #define RGF_MEDIA_EPSILON          5414
 #define RGF_MEDIA_MU               5415
 #define RGF_MESH_ALONG             5416
 #define RGF_MESH_ACROSS            5417
 #define RGF_DIFF_NUM_REG           5418
 #define RGF_LENGTH_OF_REG          5419
 #define RGF_REGION_LAYERS          5420
 #define RGF_DIFF_REGIONS           5421
 #define RGF_REGION_SHIFT           5422
 #define RGF_OPPOSITE_DIST          5423
 #define RGF_SHIFT_DIST             5424
 #define RGF_NEAR_WALLS             5425

 #define MISSED_LMP_FILE            5430
 #define OPEN_LMP_FILE              5431
 #define SMALL_LUMPED_REG           5432
 #define IMP_FILE_FORMAT            5433
 #define IMP_NOT_ALL_FRQ            5434
 #define TOO_LITTLE_IMPED           5435
 #define IMP_NOT_DEFINED            5436
 #define IMPED_UNIFORMITY           5437

 #define GGF_NOT_DEVIDED            5440

 #define OPEN_GSM_FILE              5460
#endif

#ifdef _MODEL_M
 #define WRONG_LAYER_DESCR          5203
 #define PACKAGE_NOT_DEF            5207
 #define WRONG_LAYER_SEQUENCE       5208
 #define STRUCT_WITHOUT_INP         5218
 #define INCORR_HEIGHT_VALUE        5229
 #define INCORR_EPSILON_VALUE       5230
 #define INCORR_MU_VALUE            5232
 #define LAYER_AT_CLOSE_TOP         5237
 #define LAYER_AT_CLOSE_BOTTOM      5238
 #define NOT_ENAUGH_MEDIA           5240
 #define MISSING_LAYER              5250
 #define MISSING_MEDIA              5251
 #define ERROR_ELEMENT_NUM_EVERFLOW 5252
 #define ERROR_ELEMENT_INP_NUM      5253
 #define ERROR_LINK_UNEXP_BL_NUM    5254
 #define ERROR_LINK_DUBL_BL         5255
 #define ERROR_LINK_UNEXP_BL_IN     5256
 #define ERROR_OPEN_FILE            5257
 #define ERROR_TITL_FILE            5258
 #define ERROR_DATA_FILE            5259
 #define ERROR_MEM_FILE             5260
 #define ERROR_BAD_NUM              5261
 #define ERROR_LINK_MODE_BL         5262
 #define TOOLONGFILENAME            5263

 #define BAD_PACKAGE_ELEMENT        5280
 #define BAD_ATTRIBUTE_ELEMENT      5281
 #define BAD_DATA_ELEMENT           5282
 #define ERROR_ELEMENT_ASM          5283
/* M_LINE */
 #define BAD_ERROR_EPSILON          5290
 #define BAD_ERROR_H                5291
 #define BAD_ERROR_W                5292
 #define BAD_ERROR_F                5293
 #define BAD_ERROR_L                5294
 #define BAD_ERROR_WKH              5295
/* M_OPEN_END */
 #define BAD_WARNING_F_H            5300
 #define BAD_WARNING_WKH            5301
 #define BAD_WARNING_EPSILON        5302
/* M_GAP */
 #define BAD_ERROR_EPSILON_GAP      5305
 #define BAD_ERROR_WKH_GAP          5306
 #define BAD_ERROR_SKW_GAP          5307
/* M_STEP */
 #define BAD_ERROR_EPSILON_STEP     5310
 #define BAD_ERROR_EPSILON_STEP1    5311
 #define BAD_ERROR_W2W1_STEP        5312
/* M_BEND */
 #define BAD_ERROR_EPSILON_BEND     5315
 #define BAD_ERROR_WKH_BEND         5316
/* M_PAZ */
 #define BAD_ERROR_A_PAZ            5320
 #define BAD_ERROR_B_PAZ            5321
 #define BAD_ERROR_AH_PAZ           5322
 #define BAD_ERROR_BW_PAZ           5323
/* M_FILE */
 #define ERROR_READ_S_TO_Y          5325
/* M_FILE_LINE */
 #define BAD_RGF_VERSION            5404
 #define RGF_PS_ERROR               5405
 #define BAD_FRQ_FOR_RGF            5406
/* M_TOPOLOGY */
 #define ERROR_LINK_TOP                  5500
 #define ERROR_LINK_TOP_LAYER_N_BL       5501
 #define ERROR_LINK_TOP_SECTION_N_BL     5502
 #define ERROR_LINK_TOP_N_BL             5503
 #define ERROR_LINK_TOP_LAYER_INP        5504
 #define ERROR_INP_SECTION_SIZE          5505
 #define TOPOLOGY_DATA_BASE_END          5506
 #define ERROR_ABSENT_BLOCK              5507
 #define ERROR_NEW_BLOCKS                5508
 #define ERROR_DIFF_INPUT_LAYERS         5509
 #define ERROR_TWP_FILE_TABLES           5510
#endif

#define TOO_MUCH_INP               6001
#define END_OF_DATA_LIST           6002
#define TOO_MUCH_GP                6003
#define ERR_ANS_MAXNUZ             6008
#define DATA_BASE_END              6010
#define LEFT_RIGHT_TYPE            6011

#define LINK_BLOCK_NOT_FOUND       6101
#define LINK_UNKNOWN_GP            6102
#define LINK_INPUT_NOT_FOUND       6103
#define LINK_INPUT_DEFERENCE       6104
#define LINK_INPUT_PARMS           6105
#define LINK_BAD_ORDER             6106
#define LINK_BLOCK_UNUSED          6107

#define INTERNAL_LINKER_ERR        6200

#define BAD_PRINT_FILE_NAME        6300
#define BAD_PRN_MARGIN             6301
#define BAD_EDITOR_FILE_NAME       6302

#if ( defined _MODEL_H )||( defined _MODEL_E )
 #define ILLEGAL_TRN_DATA_1        6400
#endif

#define RUN_ANS_UNKNOWN_ERR        7001

#define LINE_NOT_FOUND             7501

#define POINT_DB_EMPTY             8001
#define POINT_DB_EXHAUSTED         8002
#define LINE_DB_EMPTY              8003
#define LINE_DB_EXHAUSTED          8004
#define POLYGON_DB_EMPTY           8005
#define POLYGON_DB_EXHAUSTED       8006
#define LINE_ATTR_DB_EMPTY         8007
#define LINE_ATTR_DB_EXHAUSTED     8008
#define LINE_P_LIST_DB_EMPTY       8009
#define LINE_P_LIST_DB_EXHAUSTED   8010

void _far   _fortran SetErrorFileName( char _far *lpszFileName );
char _far * _fortran GetErrorFileName( void );

void _far   _fortran SetErrorPathName( char _far *lpszPathName );
char _far * _fortran GetErrorPathName( void );

int  _far   _fortran ErrorMessageMenu( char _far *menu_ptr[], struct messege_menu _far *msm_ptr );

#define put_error_message  put_error_messege

int _far put_error_messege( int err_num, ... );
int _far put_error_messege( char *ch );



