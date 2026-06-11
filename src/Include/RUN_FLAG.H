/*
**  TAMIC_soft(R) 1991 v1.00
**
**  run_flag.h
**
**  FILE : a:\include\run_flag.c
**
**  LIB  :
**
**  OVERLAY:
**
*/

#ifndef _RUN_FLAG_H
 #define _RUN_FLAG_H

struct run_flags
           {
            int load;
            int topology;
            int kernel;
            int out_proc;
            };

#define FACTOR     0
#define INVERT     1

void _far         set_tpl_step( int step );
int  _far         get_tpl_step( void );
int  _far fortran decr_tpl_step( void );

void _far set_all_flag( int flag );
int  _far get_all_flag( void );

void _far set_need_rf_load_flag( int flag );
int  _far get_need_rf_load_flag( void );

void _far set_need_rf_topology_flag( int flag );
int  _far get_need_rf_topology_flag( void );

void _far set_need_rf_kernel_flag( int flag );
int  _far get_need_rf_kernel_flag( void );

void _far set_need_rf_out_proc_flag( int flag );
int  _far get_need_rf_out_proc_flag( void );

void _far set_able_rf_load_flag( int flag );
int  _far get_able_rf_load_flag( void );

void _far set_able_rf_topology_flag( int flag );
int  _far get_able_rf_topology_flag( void );

void _far set_able_rf_kernel_flag( int flag );
int  _far get_able_rf_kernel_flag( void );

void _far set_able_rf_out_proc_flag( int flag );
int  _far get_able_rf_out_proc_flag( void );

void _far set_auto_run_flag( int flag );
int  _far get_auto_run_data_flag( void );

void _far Set_Sound_effects_flag( int flag );
int  _far Get_Sound_effects_flag( void );

void _far Set_sol_system_flag( int flag );
int  _far Get_sol_system_flag( void );

void _far set_accepted_data_flag( int flag );
int  _far get_accepted_data_flag( void );

void _far Set_TRACE_mode( int mode );
int  _far Get_TRACE_mode( void );

void _far Set_WARNING_mode( int mode );
int  _far Get_WARNING_mode( void );

#endif
