#include <stdlib.h>
#include <stdio.h>

#include <bufm.h>

FILE *in, *out;

int _far load_preproc_buff( void );
int _far main_preproc( void );

extern BUFFER buf;

int main( void )
 {
  char  str[BUF_STR_N_MAX];
  char  inf[]={"in_buf.dat"}, outf[]={"out_buf.dat"};
  int i, i_err;
  char ch;

  printf("\nConstantin Klimov\tdefiner\\includer\tversion 1.1 (13.11.91)");

  if( (in = fopen( inf, "r")) == NULL)             /*  read in buffer from file*/
   {
    printf("\nERROR in file %s", inf);
    exit(1);
    };
  if( (out = fopen( outf, "w")) == NULL)
   {
    printf("\nERROR in file %s", outf);
    exit(1);
    };


   if( load_preproc_buff() == -1 )
    {
     fclose( in );
     fclose( out );
     exit( 1 );
     };

   fclose( in );

   if( (i_err = main_preproc()) == 0 )
    {
     buf.inp_p=0;                                         /* puts in file from buffer*/
     while( buf_gets( str, BUF_STR_N_MAX, &buf ) != BUF_EOF )
      {
       fputs( str, out);
       puts( str );
       };
     fclose( out );
     return( i_err );
     }
    else
     {
      fclose( out);
      printf("\n error %d : %s", i_err, def_err_get( i_err ));
      dest_prep();
      return(i_err);
      };
   }



char _far *fget_tpl_line( char _far *str_buff )
 {
  if( fgets( str_buff, BUF_STR_N_MAX, in) == NULL) return( NULL );
  return( str_buff );
  }

int _far get_step( void )
 {
  int step;
  printf("\n input step :");
  scanf("%d", &step);
  printf("\n step : %d", step);
  return( step );
  }
