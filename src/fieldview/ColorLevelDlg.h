#pragma once


// CColorLevelDlg dialog

#define WM_COLOR_LEVEL_DIALOG_BYE WM_USER + 205


class CColorLevelDlg : public CDialog
{
	DECLARE_DYNAMIC(CColorLevelDlg)

public:
	CColorLevelDlg(CWnd* pParent = NULL);   // standard constructor
	CColorLevelDlg(CView *pFldView);
	virtual ~CColorLevelDlg();

// Dialog Data
	enum { IDD = IDD_DIALOGBARCOLORH1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedDefaultTrxyzAngxyz2();
	afx_msg void OnBnClickedDefaultTrxyzAngxyz();
	afx_msg void OnClose();
	afx_msg void OnPaint();
private:
	void *pcFldView;
	COLORREF *pscSurfaceColor;
public:
	BOOL Create(void);
private:
	BOOL *pbHightColorRezolution;
public:
	double pdLevel[20];
};
