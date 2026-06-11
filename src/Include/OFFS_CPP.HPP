/*
**  TAMIC_soft(R) 1991,1992 v1.00
**
**  offset table class declaration object.
**
**  FILE : ..\include\offs_cpp.hpp
**
**  LIB  :
**
**  OVERLAY:
**
*/


#ifndef _OFFS_CPP_HPP
 #define _OFFS_CPP_HPP


#ifndef __cplusplus
#error Must use C++ for the type _FILE.
#endif

extern "C" {
#include <stdio.h>
}

#define  _FAR

#ifndef _OFFSET_DEF
 #define _OFFSET_DEF
typedef struct
         {
          _real frq;      // frequence
          unsigned long ulOffset; // file offset
          }_OFFSET;
#endif

class _OFFSET_TABLE
	{
         friend  int  *build_index_table( const _OFFSET_TABLE& ot1, 
                                          const _OFFSET_TABLE& ot2,
                                                _OFFSET_TABLE& ot3,
                                                Uint         **lpIndexTable, 
                                                Uint         **lpPointNum );
	 private:
                 Uint          cSize;            // table size
                 _OFFSET _FAR *lpOffsetTable;    // pointer to offset table

	 public:
                  int  InitTable( Uint _cSize );
                  int  InitTable( Uint _cSize, _OFFSET _FAR *lpOffsetTable );
                  void FreeTable( void );
                  _OFFSET_TABLE( void );   // constructors
                  _OFFSET_TABLE( Uint _cSize ) { InitTable( _cSize ); };
                 ~_OFFSET_TABLE()              { FreeTable(); } ;   // destructor

                  int           GetSize( void )  const          { return cSize; };
                  _OFFSET _FAR *GetPointerToOffsetTable( void ) { return lpOffsetTable; };
                  _real         GetFrqElement( Uint i )         { return lpOffsetTable[i].frq; }
                  Ulong         GetOffsetElement( Uint i )      { return lpOffsetTable[i].ulOffset; }

                  _OFFSET_TABLE& operator=( const _OFFSET_TABLE& a );
         };

inline _OFFSET_TABLE::_OFFSET_TABLE( void )
 {
  lpOffsetTable = (_OFFSET _FAR *)NULL;
  cSize         = 0;
  }


#endif
