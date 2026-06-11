/*
**  TAMIC_soft(R) 1989,1990,1991 v1.00     01/15/91 11:12am
**
** Macros and declarations for Tamic_H program.
**
** FILE: a:\include\tamic_h.h
*/

void far main( int argc, char far *argv[], char far *envp[]);
int _far get_dos_ext_err_flag( void );


#define     UMF_LOAD              100
#define     UMF_PREVIOS           101
#define     UMF_SAVE              102
#define     UMF_SAVEAS            103
#define     UMF_PRINTWINDOW       104
#define     UMF_EXIT              105
#define     UMF_ABOUT             106

#define     UME_PARAMETERS        201
#define     UME_TOPOLOGY          202
#define     UME_TEXT              203

#define     UMV_OUTPUT            300
#define     UMV_TOPOLOGY          301
#define     UMV_LISTING           302

#define     UMR_RUNALL            401
#define     UMR_RESTARTALL        402
#define     UMR_RUNSTEP           403
#define     UMR_RESTARTSTEP       405
#define     UMR_SKEEPSTEP         406
#define     UMR_BACKSTEP          407

#define     UMC_PRINTWINDOW       500

#define     UMU_RUNDOSCOMMAND     600
#define     UMU_CALCULATOR        601

#define     UMH_CONTENS           700
#define     UMH_HELPONHELP        701

#define     IDC_TOPOLOGYWND      1001
