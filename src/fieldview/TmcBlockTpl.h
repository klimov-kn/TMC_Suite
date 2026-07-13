#ifndef TMCBLOCKTPL_H__1
#define TMCBLOCKTPL_H__1

//  TmcBlockTpl.h  :  header  file

#include <TmcLibError.h>
#include <TmcRTH_BolckList.h>


class CTmcBlockTpl : public CTmcRTH_BlockList
{
public:
	CTmcBlockTpl();
	int GetnX( void );
	double * GetpdSurface( void );
	double * GetpdSurfaceByType( int nType );
	int HasPlasma( void );
	void DeleteData( void );
	void Read( CString csTplFileName1, CTmcLibError &cError1 );
	CString GetTplFileName( void );
	void SetTplFileName( CString &csTplFileName1 );
	CTmcLibError & GetError( void );
	virtual ~CTmcBlockTpl();
private:
	void InitAllocationArray( void );
	int nTCurrent;
	double dTCurrent;
	double dDelta;
	double dXmin;
	double dYmin;
	int nX;
	int nY;
	double *pdSurface;  // Eps(x,y) distribution
	double *pdSurfaceN; // N electron concentration (X-mode plasma)
	double *pdSurfaceY; // Y losses (X-mode plasma)
	double *pdSurfaceB; // B magnetic field (X-mode plasma)
	int bHasPlasma;     // 1 if N/Y/B grids present in .tt 
	void ReadData( void );
	CString csTplFileName;
	CTmcLibError cError;
};

#endif 
