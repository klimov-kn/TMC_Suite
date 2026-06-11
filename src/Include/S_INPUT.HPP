/*
**  TAMIC_soft(R) 1991 v1.00
**
**  s_file.hpp
**
**  FILE : a:\include\s_file.hpp
**
**  LIB  :
**
**  OVERLAY:
**
*/


#ifndef _S_INPUT_HPP
 #define _S_INPUT_HPP

#define  _FAR

#ifndef __cplusplus
#error Must use C++ for the type _S_FILE.
#endif


extern "C"{
#include <stdio.h>
#include <typedef.h>
}



class _S_INPUT
       {
        private: Uint  nInputNum;           // input number
                 Uint *lpInputDimArray;     // input dimension array
                                                 // constructor
        public:
                 inline int  InitInputArray( Uint nSize );
                 inline void FreeInputArray( void );
                 _S_INPUT( void );
                 _S_INPUT( Uint nInputNum )       { InitInputArray( nInputNum ); }
                ~_S_INPUT( void )                 { FreeInputArray(); }                  // destructor
                 int    GetInputNum( void )       { return nInputNum; }
                 Uint   GetInputDim( Uint i )     { return lpInputDimArray[i]; }
                 Uint  *GetInputArrayAddr( void ) { return lpInputDimArray; }
                 Uint   GetMatrixDim( void );
        };

inline _S_INPUT::_S_INPUT( void )
 {
  nInputNum = 0;
  lpInputDimArray = (Uint _FAR *)NULL;
  }

inline int _S_INPUT::InitInputArray (Uint nSize )
 {
  lpInputDimArray = (Uint *)alloc_mem_err( (long)sizeof(Uint)*nSize );

  if( lpInputDimArray != NULL )
   {
    nInputNum = nSize;
    return 0;
    }
   else
    {
     nInputNum = 0;
     return -1;
     }
  }


inline void _S_INPUT::FreeInputArray( void )
 {
  if( lpInputDimArray != NULL ) free_mem( lpInputDimArray );

  lpInputDimArray = (Uint *)NULL;
  nInputNum = 0;
  }

#endif
