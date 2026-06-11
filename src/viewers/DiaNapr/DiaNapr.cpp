// DiaNapr.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <conio.h>
#include "c2DArray.h"

int main(int argc, char* argv[])
{
	c2DArray cDn;

	printf("\t\t\tTAMIC_soft group\n\t\t2D_DiaNapr calculator version 1.0 (22.07.2001)\n\n");

	if( argc != 3 )
	{
		printf("\nERROR - input or output data file name absent in command line");
		printf("\nCommand line must be:");
		printf("\n%s input_file.dat output_file.dat", argv[0]);
		printf("\n\nPress any key to continue...\n");
		getch();
		return -1;
	}

	cDn.CalculateDiagrNapr( argv[1], argv[2] );
	
	if( cDn.IsError() )
	{
		printf("\nAttention\n%s\n\n", cDn.GetError() );
	};

	printf("\nPress any key to continue...\n");
	getch();
	return 0;
}

