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
	void ReadData( void );
	CString csTplFileName;
	CTmcLibError cError;
};

#endif 
