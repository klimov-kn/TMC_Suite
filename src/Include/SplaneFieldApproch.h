#pragma once

#ifndef __CTMC_SPLANE_FIELDAPP
#define __CTMC_SPLANE_FIELDAPP

#include <splaneone.h>

#define TMC_SPLINE_IDENT__ "piecewise("

class CSplaneFieldApproch
{
public:
	CSplaneFieldApproch(void);
	~CSplaneFieldApproch(void);
	int nSplane;
	CSplaneOne *pcSplaneOne;
	void DeleteData(void);
	CSplaneFieldApproch & operator=(CSplaneFieldApproch & cSplApp);
	CSplaneFieldApproch & operator+=(CSplaneFieldApproch & cSplApp);
	CSplaneFieldApproch operator+(CSplaneFieldApproch & cSplApp );
	void AddNewSpline(CSplaneOne & cSplOne);
	void nDeleteSpline(int n);
	CString csFileName_Bz;
	CString csFileName_Ex;
	CString csFileName_Ey;
	CString csFileName_Input;
	double dDelta;
	int nX;
	int nY;
	double dX0;
	double dY0;
	void DeleteOnlyAllSpline(void);
	int MakeFieldFile(void);
	int MakeFieldFile_Er(void);
	double CalcSpline(double dX);
	double CalcSpline_Er(double dX);
	void SetFileName_Input(CString csFileName);
	int ReadSplineFromFile(int nSpl);
	int ReadSplineFromFile(void);
	int ReadSplineFromFile(CString FileName);
	int ReadSplineFromFile(CString FileName, int nSpl);
	int ReadSplineFromFile(CString FileName, int nSpl, double dXmin_Spl, double dXmax_Spl);
	int SkipSplineFromFile(FILE * fp, int nSpl);
	int ReadOneSplineFromFile(FILE * fp);
	int SkipSplineFromFile(FILE * fp);
	int SearchBeginSpline(FILE * fp);
	void InverseSpline(void);
	void SetdXmindXmax(double dXmin_Spl, double dXmax_Spl);
};

#endif

