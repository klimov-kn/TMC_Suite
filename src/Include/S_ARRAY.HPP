/*
**  TAMIC_soft(R) 1991 v1.00
**
**  _S_ARRAY class declaration
**
**  FILE : a:\include\s_array.hpp
**
**  LIB  :
**
**  OVERLAY:
**
*/


#ifndef _S_ARRAY_HPP
 #define _S_ARRAY_HPP

#define  _FAR

#ifndef __cplusplus
#error Must use C++ for the type _S_ARRAY.
#endif


extern "C"{
#include <stdio.h>
#include <typedef.h>
}

#include <_complex.hpp>


class _S_ARRAY
       {
        private: Uint  nDimension;              // S-matrix dimension
                 _complex *lpSArray;            // pointer to S-matrix array
                                                // constructor
        public:
                 inline int  InitSArray( Uint nDim );
                 inline void FreeSArray( void );
                 _S_ARRAY( void );
                 _S_ARRAY( Uint nDim )           { InitSArray( nDim ); }
                ~_S_ARRAY( void )                { FreeSArray(); }                  // destructor
                 Uint     GetSArrayDim( void )   { return nDimension; }
                 _complex *GetSArrayAddr( void ) { return lpSArray; }
                 _complex  GetSElement( Uint i, Uint j ) { return lpSArray[i*nDimension+j];}
                 _complex  GetSElement( Uint i ) { return lpSArray[i];}
        };

_S_ARRAY::_S_ARRAY( void )
 {
  nDimension = 0;
  lpSArray = (_complex _FAR *)NULL;
  }

inline int _S_ARRAY::InitSArray (Uint nDim )
 {
  lpSArray = (_complex *)alloc_mem_err( (long)sizeof(_complex)*(nDim*nDim) );

  if( lpSArray != NULL )
   {
    nDimension = nDim;
    return 0;
    }
   else
    {
     nDimension = 0;
     return -1;
     }
  }


inline void _S_ARRAY::FreeSArray( void )
 {
  if( lpSArray != NULL ) free_mem( lpSArray );

  lpSArray = (_complex *)NULL;
  nDimension = 0;
  }

#endif
