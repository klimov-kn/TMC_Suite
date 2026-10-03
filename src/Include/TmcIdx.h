// TmcIdx.h : type of the global node number and of the grid array size (INT64 patch)
//
// win32: TmcIdx = int (32 bit, as before), win64: 64 bit.
// The grid of nX*nY nodes can be bigger than 2^31 nodes in win64.

#ifndef _TMCIDX_H__
#define _TMCIDX_H__

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef ptrdiff_t TmcIdx;
#define TMC_IDX_MAX PTRDIFF_MAX

// node number in .eps files is stored in 4 bytes (file format is not changed)
inline size_t TmcReadIdx32( TmcIdx *p, FILE *fp )
{
	int n = 0;
	size_t r = fread( &n, sizeof(int), 1, fp );
	*p = (TmcIdx)n;
	return r;
}
inline size_t TmcWriteIdx32( const TmcIdx *p, FILE *fp )
{
	int n = (int)(*p);
	return fwrite( &n, sizeof(int), 1, fp );
}

#endif
