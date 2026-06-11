// TmcRTH_Input.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_Input window

#ifndef CTMCRTH_INPUT_H__
#define CTMCRTH_INPUT_H__

#include <typerth.h>
#include <TmcRTH_InputNode.h>

class CTmcRTH_Input //: public CWnd
{
// Construction
public:
	CTmcRTH_Input();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTH_Input)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTmcRTH_Input();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcRTH_Input)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
//	DECLARE_MESSAGE_MAP()
private:
	CTmcRTH_InputNode *pcInputNode;	// pointer for Input Node array
	_real rY;			//  y value for input nType = -3 and -4
	_real rX;			//  x value for input nType = -5 and -6
	int nNode;			//  number of node in input
	void DeleteData( void );
	int nType;			//					= -3 for input x left   > 
						//					= -4 for input x right  < 
						//					= -5 for input y top    V 
						//					= -6 for input y bottom ^ 
						//	is equel members nTypeEps in class CTmcRTHNodeDielOne 
	CString csExpression;
};


#endif
/////////////////////////////////////////////////////////////////////////////
