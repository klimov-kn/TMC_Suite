#if !defined(AFX_TMCSOUNDMEL1_H__5D504C21_A7E1_11D3_A93C_B08A4DC10000__INCLUDED_)
#define AFX_TMCSOUNDMEL1_H__5D504C21_A7E1_11D3_A93C_B08A4DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcSoundMel1.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcSoundMel1 dialog

class CTmcSoundMel1 : public CPropertyPage
{
	DECLARE_DYNCREATE(CTmcSoundMel1)

// Construction
public:
	CString csMelodyName;
	CTmcSoundMel1();
	~CTmcSoundMel1();

// Dialog Data
	//{{AFX_DATA(CTmcSoundMel1)
	enum { IDD = IDD_TMC_SOUND_MELODY1 };
	int		m_SoundInterval;
	int		m_Note1;
	int		m_Note2;
	int		m_Note3;
	int		m_Note4;
	int		m_Note5;
	int		m_Note6;
	int		m_Note7;
	int		m_Note8;
	int		m_Note9;
	int		m_Note10;
	int		m_Note11;
	int		m_Note12;
//	CString	csMelodyName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTmcSoundMel1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTmcSoundMel1)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCSOUNDMEL1_H__5D504C21_A7E1_11D3_A93C_B08A4DC10000__INCLUDED_)
