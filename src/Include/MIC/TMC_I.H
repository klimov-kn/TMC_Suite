/*
**  TAMIC_soft(R) 1991,1992 vR1.02
**
** Macros and declarations for main IC program.
**
** FILE: \include\mic\tmc_i.h
*/

void _far main( int argc, char _far *argv[], char _far *envp[]);
void _far get_arg_env( int argc, char _far *argv[], char _far *envp[]);
void _far init_ic(void);
void _far process(void);
void _far exit_ic( void);

int _far _fortran run_step( void );
