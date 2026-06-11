/*
**  TAMIC_soft(R) 1991,1992 v1.00
**
**  file class stdfile declaration object.
**
**  FILE : ..\include\file_cpp.hpp
**
**  LIB  :
**
**  OVERLAY:
**
*/


#ifndef _FILE_CPP_H
 #define _FILE_CPP_H


#ifndef __cplusplus
  #error Must use C++ for the type _FILE.
#else
  #define  _FAR
#endif

extern "C" {
#include <string.h>
#include <stdio.h>
}
/*
class _FILE
	{
	 private:
		 FILE _FAR *lpFILE;
		 int        nErrorCode;
		 char       szFileName[FILENAME_MAX];

                 void _FILE::set_null( void );

	 public:
		 _FILE();                      // constructors
		 _FILE( FILE _FAR *lp_FILE );
		 _FILE( const char _FAR *lpszName, const char _FAR *lpszMode );

		 ~_FILE();                     // destructor

                 int        read( void _FAR *lpBuff, size_t size, size_t n );
                 int        write( void _FAR *lpBuff, size_t size, size_t n );
                 int        fseek( long lOffset, int nPosition ) { return fseek( lpFILE, lOffset, nPosition ); }
                 char _FAR *GetFileName( void )                  { return szFileName;  }
                 int        GetErrorCode( void )                 { return nErrorCode;  }
	 };
*/
#endif

