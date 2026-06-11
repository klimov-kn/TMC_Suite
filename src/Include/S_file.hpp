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


#ifndef _S_FILE_HPP
 #define _S_FILE_HPP

#define  _FAR

#ifndef __cplusplus
#error Must use C++ for the type _S_FILE.
#endif

#include <offs_cpp.hpp>
#include <file_cpp.hpp>
#include <_complex.hpp>

#ifndef _S_HEADER_DEF
 #define _S_HEADER_DEF

typedef struct
         {
          Uint cPs;                 // "pascal" string counter
          Uint cbSize;              // offset table size
          Uint cnFrqNum;            // frequency number
          }S_HEADER;
#endif

/*
class _S_FILE : public _FILE, public _OFFSET_TABLE
       {
        private: S_HEADER      Header;           // S-file header

                                                 // constructor
        public:   _S_FILE( const char _FAR *lpszName, const char _FAR *lpszMode );
                 ~_S_FILE( void );               // destructor
                  int   GetFrqNum( void )                             { return GetSize(); }
                  Uint  GetPointInputNum( Uint nPointNum );
                  Uint  LoadInputArray( Uint _FAR *lpInputDimArray, Uint nPointNum );
                  Uint  LoadSArray( _complex _FAR *lpSArray, Uint nPointNum );
                  int   Write_CR( void );
                  int   Write_string( const char *string );
        };



inline _S_FILE::~_S_FILE( void ) { _OFFSET_TABLE::FreeTable(); }

*/
#endif
