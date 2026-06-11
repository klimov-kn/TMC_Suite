// TmcGLText.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcGLText window
#ifndef _CTMCGLTEXT_1
#define _CTMCGLTEXT_1

#include <tmcgrviw.h>
#include <gl\gl.h>
#include <gl\glu.h>
// <gl\glaux.h> удалён: GLAUX отсутствует в современном Windows SDK, а ни одна
// aux*-функция в коде не используется (текст рисуется через GDI + ядро OpenGL).
// Правка нужна и для win32, и для win64. См. ISSUES.md (Фаза 4).

class CTmcGLText : public CWnd
{
// Construction
public:
	void SetColor( COLORREF scColorRef1 );
	void SetColorSize( void );
	void SetColorInput( void );
	void SetFont( LOGFONT *plf1 );
	void DrawString( char *ch, double x, double y, double z );
	void DrawString( CString cs, double x, double y, double z );
	void DrawInputNum( int n, double x, double y, double z );
	CTmcGLText();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcGLText)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTmcGLText();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcGLText)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int nBlend;
	COLORREF scColorRef;
	unsigned char * getGLmonoBits(HDC hDC, HBITMAP hBmp, int* size);
	void getBitmap(BITMAP *bmp, LONG width, LONG height);
	unsigned char  * createStringBitmapFont(HDC hDC, char *str , PSIZEL size1);
	HFONT hFont;
	LOGFONT lf;
	SIZE size;
};

/////////////////////////////////////////////////////////////////////////////

#endif