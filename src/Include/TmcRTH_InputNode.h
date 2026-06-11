// TmcRTH_InputNode.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_InputNode window

#ifndef CTMCRTH_INPUTNODE_H__
#define CTMCRTH_INPUTNODE_H__

#include <typerth.h>

class CTmcRTH_InputNode //: public CWnd
{
// Construction
public:
	CTmcRTH_InputNode();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTH_InputNode)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTmcRTH_InputNode();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcRTH_InputNode)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
//	DECLARE_MESSAGE_MAP()
private:
	_real rUf; // falling voltage
	_real rUs; // scattering voltage
	_real rX;  // coord X
	_real rY;  // coord Y
	_real rT;  // current time
};

#endif

/////////////////////////////////////////////////////////////////////////////
