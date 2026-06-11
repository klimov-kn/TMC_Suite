/*
 *  TAMIC_soft(R) 1990,1991,1992 vR1.02
 *
 *  cut_S_matrix - cut multi-mode scattering matrix to
 *                     one-mode scattering matrix
 *                     
 *
 *  Usage -  cut_S_matrix( sss, frq, id );
 *
 *  Arguments - sss - pointer at the opinters at the scattering matrices,
 *              frq - pointer at FREQUENCY structure,
 *               id - pointer at the INPUT_DATA structures for all frq.
 *
 *  Remark - scattering matrix is assumed to be stored
 *           rowwise in general storage mode
 *
 *  Reqd. routines - cabsv, cphase ( in cmplx library )
 *
 *  FILE : ..\utility\s_file\stxtable.c
 *
 */

#include <io.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <math.h>

#include <typedef.h>
#include <frqdata.h>
#include <complex1.h>
#include <tmc_lib.h>
#include <tmclimit.h>
#include <s_file.h>
#include <error1.h>
#include "error2.h"
#include <proc_s.h>
#include <ys_proc.h>

int _far _fortran read_S_matrix_element_TMC( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint)
  {
//
   S_INDEX    _far *si;
   FREQUENCY  frq;
   int extrapol_flag;
   _complex   _far **sss1;
   INPUT_DATA _far *id;
   _complex *lpxS1;
   _complex *s;
   double *f;
   int i, j, i1, j1, ii, k, kol1, kol;
   int nP;

//   getch();
   *nPoint = -1;
   sss1 = NULL;

   if( read_S_matrix( filename, &si, &extrapol_flag, &sss1,  &frq, &id ) != 0 ) return -1;

   /* calculation nPoint for in1 mod1 in2 mod2 */
	if( *in1 < 1 )	 *in1 = 1;
	if( *in2 < 1 )	 *in2 = 1;
	if( *mod1 < 1 )	*mod1 = 1;
	if( *mod2 < 1 )	*mod2 = 1;
	if( *in1 > id[0].num ) *in1 = id[0].num;
	if( *in2 > id[0].num ) *in2 = id[0].num;

	for( k = 0, nP = 0; k < frq.max; k++ )
	{
		if( (*mod1 <= id[k].array[*in1-1])&&(*mod2 <= id[k].array[*in2-1]) ) nP++;
	};
	if( nP == 0 )
	{
	   Free_GRID_Index( &si, extrapol_flag );
	   free_mem( id );
	   Free_S_read( &sss1, frq.max );
	   return -1;
	};

   /* allocation of memory for pointers at scattering matrices */
   if( ( s = new _complex[nP] ) == NULL )
   {
	   Free_GRID_Index( &si, extrapol_flag );
	   free_mem( id );
	   Free_S_read( &sss1, frq.max );
	   return -1;
   }

   if( ( f = new double[nP] ) == NULL )
   {
	   delete s;
	   Free_GRID_Index( &si, extrapol_flag );
	   free_mem( id );
	   Free_S_read( &sss1, frq.max );
	   return -1;
   }

   /* loop over frequencies */
   for( k = 0, nP = 0; k < frq.max; k++ )
     {
	  if( (*mod1 <= id[k].array[*in1-1])&&(*mod2 <= id[k].array[*in2-1]) )
	  {
	  lpxS1 = sss1[k];
	  f[nP] = frq.f[k];
      /* first loop over inputs */

      for( i = 0, ii = 0; i < id[k].num; i++ )
        {
         /* first loop over modes */
         for( j = 0, kol1 = id[k].array[i]; j < kol1; j++ )
           {
            /* second loop over inputs */
            for( i1 = 0; i1 < id[k].num; i1++ )
              {
               /* second loop over modes */
               for( j1 = 0, kol = id[k].array[i1]; j1 < kol; j1++ )
                 {
				   if( (i==(*in1-1))&&(j==(*mod1-1))&&(i1==(*in2-1))&&(j1==(*mod2-1)) )
				   {
					   s[nP] = lpxS1[ii];
				   };
                  ii++;
                 }; //end of second loop over modes
              }; //end of second loop over inputs

           }; //end of first loop over modes
        }; //end of first loop over inputs
	  nP++;
	  };
     }; /* end frequency loop */

//  getch();
  Free_GRID_Index( &si, extrapol_flag );
  free_mem( frq.f );
  free_mem( id );
  Free_S_read( &sss1, frq.max );
  
  *sss = s;
  *f1 = f;
  *nPoint = nP;

  return 0;
  } // end of put_S_text_table routine



int _far _fortran read_S_matrix_element( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint)
  {
	read_S_matrix_element_TMC( in1, mod1, in2, mod2, filename, sss, f1, nPoint);
	if( *nPoint <= 0 )
	{
		read_S_matrix_element_HFSS( in1, mod1, in2, mod2, filename, sss, f1, nPoint);
	};
	if( *nPoint <= 0 )
	{
		read_S_matrix_element_CST( in1, mod1, in2, mod2, filename, sss, f1, nPoint);
	};
	return 0;
  } 


int _far _fortran read_S_matrix_element_HFSS( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint)
{
	int nMatrixSize;
	*nPoint = -1;

// calculating and correcting input and mode number's
	if( SMatrixElementHFSS_InpMod( in1, mod1, in2, mod2, filename) != 0) return -1;
// allocation memory and read frequence array
	if( SMatrixElementHFSS_FreqArray( in1, mod1, in2, mod2, filename, f1, sss, nPoint, &nMatrixSize) != 0) return -1;
// allocation memory and read sss(in1, mod1, in2, mod2) array
//	if( SMatrixElementHFSS_Read( in1, mod1, in2, mod2, filename, sss, f1, nPoint, &nMatrixSize) != 0) return -1;

	return 0;
} 

int _far _fortran SMatrixElementHFSS_InpMod( int *in1, int *mod1, int *in2, int *mod2, char *filename )
{
	int nInpMax, nModMax;

	if( (nInpMax = SMatrixElementHFSS_InpMax( filename ) ) < 1 ) return -1;

	if( *in1 < 1 )	 *in1 = 1;
	if( *in2 < 1 )	 *in2 = 1;
	if( *mod1 < 1 )	*mod1 = 1;
	if( *mod2 < 1 )	*mod2 = 1;
	if( *in1 > nInpMax ) *in1 = nInpMax;
	if( *in2 > nInpMax ) *in2 = nInpMax;

	if( (nModMax = SMatrixElementHFSS_ModMax( filename, in1 ) ) < 1 ) return -1;
	if( *mod1 > nModMax ) *mod1 = nModMax;

	if( (nModMax = SMatrixElementHFSS_ModMax( filename, in2 ) ) < 1 ) return -1;
	if( *mod2 > nModMax ) *mod2 = nModMax;

	return 0;
} 

int _far _fortran SMatrixElementHFSS_ModMax( char *filename, int *pnInp )
{
	FILE *fp;
	int nSize;
	int nInp = -1, nMod = -1, nModInp = -1;
	char *ch;

	nSize = 10000;

	if( strlen( filename ) < 1 ) return -1;
	if( ( fp = fopen( filename, "r" ) ) == NULL ) return -1;

	if( ( ch = new char[ nSize ] ) == NULL )
	{
		fclose( fp );
		return -1;
	};

	for( ; fgets( ch, nSize, fp )!= NULL; )
	{
		ChooseString( ch, "-> P", "M", &nInp, &nMod );
		if( (*pnInp) == nInp ) nModInp = nMod;
	};

	delete []ch;
	fclose( fp );

	return nModInp;
} 

int _far _fortran SMatrixElementHFSS_InpMax( char *filename )
{
	FILE *fp;
	int nSize;
	int nInp = -1, nMod = -1;
	char *ch;

	nSize = 10000;

	if( strlen( filename ) < 1 ) return -1;
	if( ( fp = fopen( filename, "r" ) ) == NULL ) return -1;

	if( ( ch = new char[ nSize ] ) == NULL )
	{
		fclose( fp );
		return -1;
	};

	for( ; fgets( ch, nSize, fp )!= NULL; )
	{
		ChooseString( ch, "-> P", "M", &nInp, &nMod );
	};

	delete []ch;
	fclose( fp );

	return nInp;
} 


int _far _fortran ChooseString( char *pszBuf, char *pszIdentifInput, char *pszIdentifMode, int *pnInp, int *pnMod )
{
	int i, j;

	for( i = 0; pszBuf[ i ] != '\0' ; i++ )
	{
		if( strncmp( pszBuf + i, pszIdentifInput, strlen(pszIdentifInput) ) == 0)
		{
//			PutStatistics111(pszBuf + i);
			for( j = 0; pszBuf[ j ] != '\0'; j++ )
			{
				if( strncmp( pszBuf + j, pszIdentifMode, strlen(pszIdentifMode) ) == 0)
				{
					pszBuf[ j ] = '\0';
//					PutStatistics111(pszBuf + i + strlen(pszIdentifInput));
					sscanf( pszBuf + i + strlen(pszIdentifInput), "%d", pnInp );
//					PutStatistics111(pszBuf + j + strlen(pszIdentifMode));
					sscanf( pszBuf + j + strlen(pszIdentifMode), "%d", pnMod );
					return 1;
				};
			};
			return 0;
		};
	};
	return 0;
}


int _far _fortran ChooseString1( char *pszBuf, char *pszIdentifInput, char *pszIdentifMode, int *pnInp, int *pnMod )
{
	int i, j;

	for( i = 0; pszBuf[ i ] != '\0' ; i++ )
	{
		if( strncmp( pszBuf + i, pszIdentifInput, strlen(pszIdentifInput) ) == 0)
		{
//			PutStatistics111(pszBuf + i);
			for( j = 0; pszBuf[ j ] != '\0'; j++ )
			{
				if( strncmp( pszBuf + j, pszIdentifMode, strlen(pszIdentifMode) ) == 0)
				{
					pszBuf[ j ] = '\0';
//					PutStatistics111(pszBuf + i + strlen(pszIdentifInput));
					sscanf( pszBuf + i + strlen(pszIdentifInput), "%d", pnInp );
//					PutStatistics111(pszBuf + j + strlen(pszIdentifMode));
					sscanf( pszBuf + j + strlen(pszIdentifMode), "%d", pnMod );
					return j + strlen(pszIdentifMode);
				};
			};
			return 0;
		};
	};
	return 0;
}


// allocation memory and read frequence array
int _far _fortran SMatrixElementHFSS_FreqArray( int *in1, int *mod1, int *in2, int *mod2, char *filename, double **f1, _complex **sss, int *nPoint, int *nMatrixSize)
{
// calculation point in frequence array
	SMatrixElementHFSS_FreqArrayCalculation( filename, f1, nPoint, nMatrixSize);
// allocation memory for frequence array
	SMatrixElementHFSS_FreqArrayAllocation( f1, sss, nPoint);
// read data in frequence array
	SMatrixElementHFSS_FreqArrayRead( in1, mod1, in2, mod2, filename, f1, sss, nPoint, nMatrixSize);

	return 0;
} 

// calculation point in frequence array
void _far _fortran SMatrixElementHFSS_FreqArrayCalculation( char *filename, double **f1, int *nPoint, int *nMatrixSize)
{
	*nPoint = -1;

	FILE *fp;
	int nSize;
	int nInp, nMod;
	int i, j;
	char *ch;
	double dBuf, dBuf1;

	nSize = 10000;

	if( strlen( filename ) < 1 ) return;
	if( ( fp = fopen( filename, "r" ) ) == NULL ) return;

	if( ( ch = new char[ nSize ] ) == NULL )
	{
		fclose( fp );
		return;
	};

	for( i = 0, j = 0; fgets( ch, nSize, fp )!= NULL; )
	{
		switch( (*nPoint) )
		{
			default:
						if( fgets( ch, nSize, fp ) == NULL ) 
						{
							delete []ch;
							fclose( fp );
							return;
						};
						if( ch[10] == '\n' ) j++;
						break;
			case -1:
						i += ChooseString( ch, "-> P", "M", &nInp, &nMod );
						if( strncmp( ch, "!END EXCITATION_MAPS", strlen("!END EXCITATION_MAPS") )== 0 )
						{
							*nPoint = 0;
							*nMatrixSize = i;
							for( i = 0; fscanf( fp, "%lg", &dBuf) == 1; i++ )
							{
								for( j = 0; j < (*nMatrixSize)*(*nMatrixSize); j++ )
								{
									fscanf( fp, "%lg%lg", &dBuf, &dBuf1);
								};
							};
							delete []ch;
							fclose( fp );		
							*nPoint = i;
							return;
						};
						break;
		};
	};

//	sprintf( ch, "nMatrixSize = %d; nPoint = %d", i, j );
//	PutStatistics111( ch );

	delete []ch;
	fclose( fp );

	*nMatrixSize = -1;
	*nPoint = -1;

	return;
} 

// allocation memory for frequence array
void _far _fortran SMatrixElementHFSS_FreqArrayAllocation( double **f1, _complex **sss, int *nPoint)
{
	if( (*nPoint) < 1 ) return;

	if( ( (*f1) = new double[(*nPoint)] ) == NULL ) 
	{
		*nPoint = -1;
		return;
	};
	if( ( (*sss) = new _complex[(*nPoint)] ) == NULL )
	{
		delete (*f1);
		*f1 = NULL;
		*nPoint = -1;
		return;
	};

	return;
} 

// read data in frequence array
void _far _fortran SMatrixElementHFSS_FreqArrayRead( int *in1, int *mod1, int *in2, int *mod2, char *filename, double **f1, _complex **sss, int *nPoint, int *nMatrixSize)
{
	if( (*nPoint) < 1 ) return;
	if( strlen( filename ) < 1 ) return;
	if( ( *f1 ) == NULL ) return;
	if( ( *sss ) == NULL ) return;

	FILE *fp;
	int nSize;
	int nInp = -1, nMod = -1, nInpMod1 = -1, nInpMod2 = -1;
	int i, j, k;
	char *ch;
	double *f, dBuf, dBuf1;
	_complex *s;

	f = *f1;
	s = *sss;

	nSize = 10000;

	if( ( fp = fopen( filename, "r" ) ) == NULL ) return;

	if( ( ch = new char[ nSize ] ) == NULL )
	{
		fclose( fp );
		return;
	};

	for( i = 0, ch[0] = '\0'; (fgets( ch, nSize, fp )!= NULL)&&(strncmp( ch, "!END EXCITATION_MAPS", strlen("!END EXCITATION_MAPS") )!= 0); )
	{
		if( ChooseString( ch, "-> P", "M", &nInp, &nMod )  != 0)
		{
			if( (nInp == *in1)&&(nMod == *mod1) ) nInpMod1 = i;
			if( (nInp == *in2)&&(nMod == *mod2) ) nInpMod2 = i;
			i++;
		};
	};

	if( (nInpMod1 < 0)||(nInpMod2 < 0) )
	{
		delete f;
		*f1 = NULL;
		delete s;
		*sss = NULL;
		delete []ch;
		fclose( fp );
		return;
	};

	k = nInpMod1*( *nMatrixSize ) + nInpMod2;
	for( i = 0; i < ( *nPoint ) ; i++ )
	{
		if( fscanf( fp, "%lg", f+i ) != 1)
		{
			delete f;
			*f1 = NULL;
			delete s;
			*sss = NULL;
			delete []ch;
			fclose( fp );
			return;
		};
		f[ i ] *= 1000000000.;
//		sprintf( ch, "Frequence [ %d ] = %lg; nMatrixSize = %d", i, f[ i ], *nMatrixSize );
//		PutStatistics111( ch );
		for( j = 0; j < ( *nMatrixSize )*( *nMatrixSize ); j++ )
		{
			if( fscanf( fp, "%lg%lg", &dBuf, &dBuf1 ) != 2)
			{
				delete f;
				*f1 = NULL;
				delete s;
				*sss = NULL;
				delete []ch;
				fclose( fp );
				return;
			};
			if( j == k )
			{
				s[ i ].x = dBuf;
				s[ i ].y = dBuf1;
			};
		};
	};

	delete []ch;
	fclose( fp );

	return;
} 

int _far _fortran read_S_matrix_element_CST( int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint)
{
	int nMatrixSize;
	*nPoint = -1;

// calculating and correcting input and mode number's
	if( SMatrixElementCST_InpMod( in1, mod1, in2, mod2, filename) != 0) return -1;
// allocation memory and read frequence array
	if( SMatrixElementCST_FreqArray( in1, mod1, in2, mod2, filename, f1, sss, nPoint, &nMatrixSize) != 0) return -1;
// allocation memory and read sss(in1, mod1, in2, mod2) array
//	if( SMatrixElementCST_FreqArrayRead( in1, mod1, in2, mod2, filename, sss, f1, nPoint, &nMatrixSize) != 0) return -1;

	return 0;
} 

int _far _fortran SMatrixElementCST_InpMod( int *in1, int *mod1, int *in2, int *mod2, char *filename )
{
	int nInpMax, nModMax;

	if( (nInpMax = SMatrixElementCST_InpMax( filename ) ) < 1 ) return -1;

	if( *in1 < 1 )	 *in1 = 1;
	if( *in2 < 1 )	 *in2 = 1;
	if( *mod1 < 1 )	*mod1 = 1;
	if( *mod2 < 1 )	*mod2 = 1;
	if( *in1 > nInpMax ) *in1 = nInpMax;
	if( *in2 > nInpMax ) *in2 = nInpMax;

	if( (nModMax = SMatrixElementCST_ModMax( filename, in1 ) ) < 1 ) return -1;
	if( *mod1 > nModMax ) *mod1 = nModMax;

	if( (nModMax = SMatrixElementCST_ModMax( filename, in2 ) ) < 1 ) return -1;
	if( *mod2 > nModMax ) *mod2 = nModMax;

	return 0;
} 

int _far _fortran SMatrixElementCST_InpMax( char *filename )
{
	FILE *fp;
	int i, j;
	int nSize;
	int nInp = -1, nMod = -1;
	char *ch;

	nSize = 400000;

	if( strlen( filename ) < 1 ) return -1;
	if( ( fp = fopen( filename, "r" ) ) == NULL ) return -1;

	if( ( ch = new char[ nSize ] ) == NULL )
	{
		fclose( fp );
		return -1;
	};

//	for( ; fgets( ch, nSize, fp )!= NULL; )
//	{
		fgets( ch, nSize, fp );
		for( i = 0, j = 0; (j = ChooseString1( ch + i, "WavePort", "):Mode(", &nInp, &nMod )) > 0; i += j ) ;
//		;
//	};

	delete []ch;
	fclose( fp );

	return nInp;
} 

int _far _fortran SMatrixElementCST_ModMax( char *filename, int *pnInp )
{
	FILE *fp;
	int nSize;
	int nInp = -1, nMod = -1, nModInp = -1;
	int i, j;
	char *ch;

	nSize = 400000;

	if( strlen( filename ) < 1 ) return -1;
	if( ( fp = fopen( filename, "r" ) ) == NULL ) return -1;

	if( ( ch = new char[ nSize ] ) == NULL )
	{
		fclose( fp );
		return -1;
	};

//	for( ; fgets( ch, nSize, fp )!= NULL; )
//	{
//		ChooseString( ch, "WavePort", "):Mode(", &nInp, &nMod );
//		if( (*pnInp) == nInp ) nModInp = nMod;
//	};

	fgets( ch, nSize, fp );
	for( i = 0, j = 0; (j = ChooseString1( ch + i, "WavePort", "):Mode(", &nInp, &nMod )) > 0; i += j )
	{
		if( (*pnInp) == nInp ) nModInp = nMod;
	};

	delete []ch;
	fclose( fp );

	return nModInp;
} 




// allocation memory and read frequence array
int _far _fortran SMatrixElementCST_FreqArray( int *in1, int *mod1, int *in2, int *mod2, char *filename, double **f1, _complex **sss, int *nPoint, int *nMatrixSize)
{
// calculation point in frequence array
	SMatrixElementCST_FreqArrayCalculation( filename, f1, nPoint, nMatrixSize);
// allocation memory for frequence array
	SMatrixElementCST_FreqArrayAllocation( f1, sss, nPoint);
// read data in frequence array
	SMatrixElementCST_FreqArrayRead( in1, mod1, in2, mod2, filename, f1, sss, nPoint, nMatrixSize);

	return 0;
} 


// calculation point in frequence array
void _far _fortran SMatrixElementCST_FreqArrayCalculation( char *filename, double **f1, int *nPoint, int *nMatrixSize)
{
	*nPoint = -1;

	FILE *fp;
	int nSize;
	int nInp, nMod;
	int i, j, ii;
	char *ch;
//	double dBuf, dBuf1;

	nSize = 400000;

	if( strlen( filename ) < 1 ) return;
	if( ( fp = fopen( filename, "r" ) ) == NULL ) return;

	if( ( ch = new char[ nSize ] ) == NULL )
	{
		fclose( fp );
		return;
	};

	for( i = 0, j = 0; fgets( ch, nSize, fp )!= NULL; )
	{
		switch( (*nPoint) )
		{
			default:
						*nPoint = 1;
						for( ; fgets( ch, nSize, fp )!= NULL; ) *nPoint = *nPoint + 1;
						delete []ch;
						fclose( fp );
						return;
						break;
			case -1:
						for( i = 0, ii = 0, j = 0; (j = ChooseString1( ch + i, "WavePort", "):Mode(", &nInp, &nMod )) > 0; i += j, ii++ ) 
						{
							;
						};
						i = ii;
						*nPoint = 0;
						*nMatrixSize = i;
						if( fgets( ch, nSize, fp ) == NULL ) 
						{
							delete []ch;
							fclose( fp );
							return;
						};
						break;
		};
	};

//	sprintf( ch, "nMatrixSize = %d; nPoint = %d", i, j );
//	PutStatistics111( ch );

	delete []ch;
	fclose( fp );

	*nMatrixSize = -1;
	*nPoint = -1;

	return;
} 


// allocation memory for frequence array
void _far _fortran SMatrixElementCST_FreqArrayAllocation( double **f1, _complex **sss, int *nPoint)
{
	if( (*nPoint) < 1 ) return;

	if( ( (*f1) = new double[(*nPoint)] ) == NULL ) 
	{
		*nPoint = -1;
		return;
	};
	if( ( (*sss) = new _complex[(*nPoint)] ) == NULL )
	{
		delete (*f1);
		*f1 = NULL;
		*nPoint = -1;
		return;
	};

	return;
} 


// read data in frequence array
void _far _fortran SMatrixElementCST_FreqArrayRead( int *in1, int *mod1, int *in2, int *mod2, char *filename, double **f1, _complex **sss, int *nPoint, int *nMatrixSize)
{
	FILE *fp;
	int nSize;
	int i, j, nInp, nMod, ii;
	int in1_1, in2_1;
	char *ch;
	double dBuf, dBuf1, r;

	double *f;
	_complex *s;

	f = *f1;
	s = *sss;

	nSize = 400000;

	if( strlen( filename ) < 1 ) return;
	if( ( fp = fopen( filename, "r" ) ) == NULL ) return;

	if( ( ch = new char[ nSize ] ) == NULL )
	{
		fclose( fp );
		return;
	};


	if( fgets( ch, nSize, fp )== NULL )
	{
		delete f;
		*f1 = NULL;
		delete s;
		*sss = NULL;
		delete []ch;
		fclose( fp );
		return;
	};
	for( i = 0, ii = 0, j = 0, in1_1 = 0, in2_1 = 0; (j = ChooseString1( ch + i, "WavePort", "):Mode(", &nInp, &nMod )) > 0; i += j, ii++ ) 
	{
		if( (nInp == *in1)&&(nMod == *mod1) ) in1_1 = ii;
		if( (nInp == *in2)&&(nMod == *mod2) ) in2_1 = ii;
	};
	if( fgets( ch, nSize, fp )== NULL )
	{
		delete f;
		*f1 = NULL;
		delete s;
		*sss = NULL;
		delete []ch;
		fclose( fp );
		return;
	};

	for( i = 0; i < *nPoint; i++)
	{
		if( fscanf( fp, "%lg", &r ) != 1 )
		{
			delete f;
			*f1 = NULL;
			delete s;
			*sss = NULL;
			delete []ch;
			fclose( fp );
			return;
		};

		f[ i ] = r;

		for( j = 0; j < (*nMatrixSize)*(*nMatrixSize); j++ )
		{
			fscanf( fp, "%lg%lg", &dBuf, &dBuf1);
			if( ((in1_1)*(*nMatrixSize)+(in2_1)) == j )
			{
				s[ i ].x = dBuf*cos(dBuf1/180.*3.141592653589);
				s[ i ].y = dBuf*sin(dBuf1/180.*3.141592653589);
			};
		};
		fgets( ch, nSize, fp );
	};

//	sprintf( ch, "nMatrixSize = %d; nPoint = %d", i, j );
//	PutStatistics111( ch );

	delete []ch;
	fclose( fp );

	return;
} 


