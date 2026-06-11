#pragma once

#ifndef __CTMC_SPLANE_ONE
#define __CTMC_SPLANE_ONE

#define TMC_SPLINEONE_XMAX 0
#define TMC_SPLINEONE_SPL  1
#define TMC_SPLINEONE_ERR  2
#define TMC_SPLINEONE_END  3
#define TMC_SPLINEONE_MAXSTRLEN  256

class CSplaneOne
{
public:
	CSplaneOne(void);
	~CSplaneOne(void);
	double dXmin;
	double dXmax;
	double dX_m3;
	double dX_m2;
	double dX_m1;
	double dX_0;
	double dX_p1;
	double dX_p2;
	double dX_p3;
	CSplaneOne & operator=(CSplaneOne & cSplOne);
	void DeleteData(void);
	double CalcSpline(double dX);
	double CalcSpline_Er(double dX);
	int ReadOneSplineFromFile(FILE * fp);
	int ReadOneParameter(FILE * fp, double * r0, double * r1, double * r2, double * r3);
	int ReadOneParameterToString(FILE * fp, CString & csBuf);
	void SkipTwoCharacters(CString & csBuf);
	int ReadParametersFromString(CString & csBuf, double * r0, double * r1, double * r2, double * r3);
	int FindEndLexem(char * szBuf);
	int FindDivider(char * szBuf);
	void InverseSpline(void);
};

#endif

