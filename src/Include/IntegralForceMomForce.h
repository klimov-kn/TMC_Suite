#pragma once
#include <TmcLibError.h>
#define TMCFORCEEXTENTIONFILE "frc"
class CIntegralForceMomForce
{
public:
	CIntegralForceMomForce(void);
	~CIntegralForceMomForce(void);
private:
	double dFx;
	double dFy;
	double dFz;
	double dMFx;
	double dMFy;
	double dMFz;
	CTmcLibError cError;
public:
	void DeleteData(void);
private:
	CString csFileName;
public:
	void SetFileName(CString csFileName_1);
	CString GetFileName(void);
	void SetdFx(double dFx_1);
	double GetdFx(void);
	void SetdFy(double dFy_1);
	double GetdFy(void);
	void SetdFz(double dFz_1);
	double GetdFz(void);
	void SetdMFx(double dMFx_1);
	double GetdMFx(void);
	void SetdMFy(double dMFy_1);
	double GetdMFy(void);
	void SetdMFz(double dMFz_1);
	double GetdMFz(void);
	BOOL WriteForce(CString csFileName_1, double dFx_1, double dFy_1, double dFz_1, double dMFx_1, double dMFy_1, double dMFz_1);
	BOOL WriteForce(void);
	BOOL ReadForce(CString csFileName, double & dFx_1, double & dFy_1, double & dFz_1, double & dMFx_1, double & dMFy_1, double & dMFz_1);
	BOOL ReadForce(void);
};
