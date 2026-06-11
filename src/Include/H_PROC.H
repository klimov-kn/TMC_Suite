/*
**  TAMIC_soft(R) 1989 v1.00.  12/01/89 04:10pm
**
** Macros and declarations for H_proc program
**
** FILE: h_proc.h
*/

#ifndef _H_PROC_H
  #define _H_PROC_H


   enum UPPER_ITEMS {
                     MFILE,
                     EDIT,
                     VIEW,
                     RUN,
                     CONFIG,
                     UTILITY,
                     HELP};

    #define MAX_STAT_WINDOWS   9
    #define MAX_UPPER_ITEMS   8


    #define ACTIVE     1
    #define PASSIVE    0

    #define TOPOLOGY_W 0
    #define OUTPUT_W   1

    #define NO_ASK     0
    #define ASK_EXIT   1

    #define _MAIN_STR_HELP     0
    #define _STOP_PROC_DN      3
    #define _MEM_ERR_DN        4
    #define _ES_EXIT           5
    #define _ESC_STOP_PRINT    6
    #define _PRINT_WINDW_C1    7
    #define _PRINT_WINDW_C2    8



                                            /* proc declaration */
    void far process( void );
    int  far load_p( void );
    void far previos_p( void );
    void far save_p( void );
    int  far save_as_p( void );
    void far dos_shell_p( char far *command_string );

    void _far exit_p( int ask_flag );
    void far parms_p( void );
    void far topology_p( void );
    void far text_p( void );
    void far output_p( void );
    void far topology_w_p( void );
    void far listing_p( void );
    void far check_all_p( void );
    void far start_all_p( void );
    void far restart_all_p( void );
    void far check_step_p( void );
    void far start_step_p( void );
    void far restart_step_p( void );
    void far skip_step_p( void );
    void far back_step_p( void );
    void far run_dos_command_p( void );
    void far calculater_p( void );
    void far help_p( void );
    void far tables_p( void );
    int  _far check_stop( void );

                                       /* config function declaration */
    void far editor_c(void);
    void far model_c(void);
    void far topology_c(void);
    void far output_c(void);
    void far limits_c(void);
    void far debug_c(void);
    void far directory_c(void);
    void far save_option_c(void);
    void far retriev_option_c(void);
    void _far print_c( int x, int y );

                                              /* menu function declaration */
    int  far help_m( void );
    int  far config_m( void );
    int  far utility_m( void );
    int  far view_m( void );
    int  far run_m( void );
    int  far edit_m( void );
    int  far file_m( void );

    void _far fortran processor( void );
    void far first_window( int t_num);
    int  far get_kb_replay( void );
    int  far test_upper_menu( void);
    int  far drow_upper_menu( int passiv_active_flag, int current, int prev);

    void _far reset_daun_messege_flag( void );
    void _far daun_messege( int messege_num );

#endif
