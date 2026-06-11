// TmcGLText.cpp : implementation file
//

#include "stdafx.h"
#include "TmcGLText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTmcGLText

CTmcGLText::CTmcGLText()
{
	nBlend = 100;
	scColorRef = RGB( 256, 256, 256 );
	if( wglGetCurrentDC() == NULL ) return;
    FillMemory(&lf, sizeof(lf), 0);
    lf.lfHeight = 12 * GetDeviceCaps( wglGetCurrentDC(), LOGPIXELSY) / 72;
    lf.lfCharSet = SHIFTJIS_CHARSET;
    lf.lfWeight = 400;
    lf.lfOutPrecision = OUT_STROKE_PRECIS;
    lf.lfClipPrecision = CLIP_CHARACTER_PRECIS;
    lstrcpy(lf.lfFaceName, "TimesET");
    hFont = CreateFontIndirect(&lf);
	return;
}

CTmcGLText::~CTmcGLText()
{
	return;
}


BEGIN_MESSAGE_MAP(CTmcGLText, CWnd)
	//{{AFX_MSG_MAP(CTmcGLText)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTmcGLText message handlers


void CTmcGLText::DrawInputNum( int n, double x, double y, double z )
{
    char ch[TMC_GROUT_MAXSTRING_BUF];
	unsigned char    *lpBitmap;

	sprintf( ch, "Inp %d", n );
	if( wglGetCurrentDC() == NULL ) return;
    if( (lpBitmap = createStringBitmapFont( wglGetCurrentDC(), ch, &size) ) == NULL ) return;

	SetColorInput();
	
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z);

    glBitmap(size.cx, size.cy, (GLfloat)0.0, (GLfloat)2.0, (GLfloat)(size.cx+2), (GLfloat)0.0, lpBitmap);

    GlobalFree(lpBitmap);

  return;
}

unsigned char  * CTmcGLText::createStringBitmapFont(HDC hDC, char *str, PSIZEL size1)
{
    BITMAP  bmp;
    HBITMAP hbmp;
    unsigned char    *pbm = NULL;
    int     len = lstrlen(str);
    HFONT   hFontOld = (HFONT)SelectObject(hDC, hFont);

    GetTextExtentPoint32(hDC, str, len, size1);
    getBitmap(&bmp, size1->cx, size1->cy);
    if( !(hbmp = CreateBitmapIndirect(&bmp) ) ) return NULL;
    GlobalFree(bmp.bmBits);
    if(hbmp){
        HDC hMemDC = CreateCompatibleDC(hDC);
        if(hMemDC){
            HBITMAP hPrevBmp = (HBITMAP)SelectObject(hMemDC, hbmp);
            HFONT   hPrevFont;
            int     size0;
            BITMAP  bi;

            SetBkColor(hMemDC, RGB(0,0,0));
            SetTextColor(hMemDC, RGB(255,255,255));
            SetBkMode(hMemDC, OPAQUE);
            hPrevFont = (HFONT)SelectObject(hMemDC, hFont);
            TextOut(hMemDC, 0, 0, str, len);

            GetObject(hbmp, sizeof(bi), &bi);
            pbm = getGLmonoBits(hMemDC, hbmp, &size0);
            size1->cx = ((bi.bmWidth+31)&(~31));
            size1->cy = bi.bmHeight;

            SelectObject(hMemDC, hPrevFont);
            SelectObject(hMemDC, hPrevBmp);
            DeleteDC(hMemDC);
        };
        DeleteObject(hbmp);
    }
      DeleteObject(hFontOld);
//    SelectObject(hDC, hFontOld);
    return pbm;
}

void CTmcGLText::getBitmap(BITMAP *bmp, LONG width, LONG height)
{
    FillMemory(bmp, sizeof(*bmp), 0);
    bmp->bmWidth = width;
    bmp->bmHeight = height;
    bmp->bmWidthBytes = ((width + 7) / 8 + 1) & (~1);
    bmp->bmPlanes = 1;
    bmp->bmBitsPixel = 1;
    bmp->bmBits = GlobalAlloc(GMEM_FIXED, bmp->bmWidthBytes*height);
}

unsigned char * CTmcGLText::getGLmonoBits(HDC hDC, HBITMAP hBmp, int* size1)
{
    BITMAP bi;
    unsigned char   *bits;
    struct {
        BITMAPINFOHEADER bih;
        RGBQUAD col[2];
    }bic;
    BITMAPINFO *binf = (BITMAPINFO *)&bic;

    GetObject(hBmp, sizeof(bi), &bi);
    *size1 = bi.bmHeight*(((bi.bmWidth + 31) & (~31)) / 8);
    bits = (unsigned char *)GlobalAlloc(GPTR, *size1);

    binf->bmiHeader.biSize = sizeof(binf->bmiHeader);
    binf->bmiHeader.biWidth = bi.bmWidth;
    binf->bmiHeader.biHeight = bi.bmHeight;
    binf->bmiHeader.biPlanes = 1;
    binf->bmiHeader.biBitCount = 1;
    binf->bmiHeader.biCompression = BI_RGB;
    binf->bmiHeader.biSizeImage = *size1;
    binf->bmiHeader.biXPelsPerMeter = 1;
    binf->bmiHeader.biYPelsPerMeter = 1;
    binf->bmiHeader.biClrUsed = 0;
    binf->bmiHeader.biClrImportant = 0;

    GetDIBits(hDC, hBmp, 0, bi.bmHeight, bits, binf, DIB_RGB_COLORS);

    return bits;
}

void CTmcGLText::DrawString( CString cs, double x, double y, double z )
{
    char ch[TMC_GROUT_MAXSTRING_BUF];
	unsigned char    *lpBitmap;

	sprintf( ch, "%s", cs );
	if( wglGetCurrentDC() == NULL ) return;
    if( (lpBitmap = createStringBitmapFont( wglGetCurrentDC(), ch, &size) ) == NULL ) return;
	
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z);

    glBitmap(size.cx, size.cy, (GLfloat)0.0, (GLfloat)2.0, (GLfloat)(size.cx+2), (GLfloat)0.0, lpBitmap);

    GlobalFree(lpBitmap);

  return;
}

void CTmcGLText::DrawString( char *ch, double x, double y, double z )
{
	unsigned char    *lpBitmap;

	if( wglGetCurrentDC() == NULL ) return;
    if( (lpBitmap = createStringBitmapFont( wglGetCurrentDC(), ch, &size) ) == NULL ) return;
	
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z);

    glBitmap(size.cx, size.cy, (GLfloat)0.0, (GLfloat)2.0, (GLfloat)(size.cx+2), (GLfloat)0.0, lpBitmap);

    GlobalFree(lpBitmap);

  return;
}

void CTmcGLText::SetColorInput( void )
{
	glColor4f( ((float)(GetRValue(scColorRef)))/256,
				((float)(GetGValue(scColorRef)))/256,
				((float)(GetBValue(scColorRef)))/256,
				(float)(nBlend/100.0) );
	return;
}

void CTmcGLText::SetColorSize( void )
{
	glColor4f( ((float)(GetRValue(scColorRef)))/256,
				((float)(GetGValue(scColorRef)))/256,
				((float)(GetBValue(scColorRef)))/256,
				(float)(nBlend/100.0) );
	return;
}

void CTmcGLText::SetColor( COLORREF scColorRef1 )
{
	scColorRef = scColorRef1;
	return;
}

void CTmcGLText::SetFont( LOGFONT *plf1 )
{
	if( hFont != 0 ) DeleteObject( hFont );
	hFont = CreateFontIndirect( plf1 );
	return;
}
