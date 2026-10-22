// Реализация окон и видов MFC поверх Qt (см. mfc_view.h).
//
// Ни одного включения Qt: всё, что нужно от настоящего окна, вид получает через
// интерфейс MfcQtHost, который реализует виджет.

/// @file mfc_view.cpp
/// @brief Реализация `CWnd`, `CClientDC`, `CScrollView` и перехватов `TmcIsWindow`/`TmcPostMessage`.
///
/// Часть модуля «Слой совместимости», входит в библиотеку `tmc_gui_common`.
/// Заголовков Qt не подключает.
/// @see https://www.tamic.ru/docs/code/compat.html#doc-view
#if !defined(_WIN32) || defined(TMC_QT_WIN)

#include "mfc_view.h"

#include <cmath>

// --- CWnd ---------------------------------------------------------------------

void CWnd::Invalidate(BOOL erase)
{
    if (m_host)
        m_host->hostInvalidate(0, erase);
}

void CWnd::InvalidateRect(const RECT* rect, BOOL erase)
{
    if (m_host)
        m_host->hostInvalidate(rect, erase);
}

void CWnd::UpdateWindow()
{
    if (m_host)
        m_host->hostUpdateWindow();
}

void CWnd::GetClientRect(RECT* rect) const
{
    if (!rect)
        return;
    if (m_host) {
        m_host->hostGetClientRect(rect);
    } else {
        rect->left = rect->top = 0;
        rect->right = rect->bottom = 0;
    }
}

void CWnd::SetCapture()
{
    if (m_host)
        m_host->hostSetCapture();
}

void CWnd::ReleaseCapture()
{
    if (m_host)
        m_host->hostReleaseCapture();
}

UINT CWnd::SetTimer(UINT id, UINT elapse, void*)
{
    return m_host ? m_host->hostSetTimer(id, elapse) : 0;
}


int CWnd::MessageBox(const char* text, const char* caption, UINT type)
{
    return m_host ? m_host->hostMessageBox(text, caption, type) : 0;
}

// --- CClientDC ----------------------------------------------------------------
// Нужен только для пересчёта координат, поэтому холста нет. Разрешение берём у
// хозяина: от него зависит масштаб MM_HIMETRIC, а значит и результат DPtoLP.

namespace {

// Подставное устройство: отдаёт разрешение экрана, ничего не рисует.
/// @brief Пара разрешений экрана (`dpiX`, `dpiY`, точек на дюйм). Нигде не используется.
class HostDpiDevice
{
public:
    /// Разрешение по X, точек на дюйм.
    int dpiX;
    /// Разрешение по Y, точек на дюйм.
    int dpiY;
};

} // namespace

CClientDC::CClientDC(CWnd* wnd)
{
    // CDC берёт разрешение из QPaintDevice; здесь его нет, поэтому задаём
    // масштаб напрямую тем же способом, что и SetMapMode для MM_HIMETRIC.
    if (wnd && wnd->Host()) {
        const double dpiX = wnd->Host()->hostLogicalDpiX();
        const double dpiY = wnd->Host()->hostLogicalDpiY();
        const double px = dpiX / 25.4 / 100.0;     // пикселей на 0.01 мм
        const double py = dpiY / 25.4 / 100.0;
        SetWindowExt(100000, 100000);
        SetViewportExt(long(px * 100000.0), long(-py * 100000.0));
    }
}

// --- CView --------------------------------------------------------------------

void CView::OnPrepareDC(CDC* /*pDC*/, CPrintInfo* /*info*/)
{
}

// --- CScrollView --------------------------------------------------------------

CScrollView::CScrollView()
    : m_mapMode(MM_TEXT), m_totalLog(0, 0), m_pageLog(0, 0), m_lineLog(0, 0)
{
}

void CScrollView::SetScrollSizes(int mapMode, SIZE sizeTotal,
                                 const SIZE& sizePage, const SIZE& sizeLine)
{
    m_mapMode = mapMode;
    m_totalLog = sizeTotal;
    m_pageLog = sizePage;
    m_lineLog = sizeLine;

    if (!m_host)
        return;

    // Полосы прокрутки живут в пикселях, поэтому переводим размеры так же, как
    // это делает контекст рисования.
    CDC dc;
    dc.SetMapMode(mapMode);
    // Хозяин знает своё разрешение; подменяем масштаб под него.
    const double px = m_host->hostLogicalDpiX() / 25.4 / 100.0;
    const double py = m_host->hostLogicalDpiY() / 25.4 / 100.0;
    const double sx = (mapMode == MM_HIMETRIC) ? px : 1.0;
    const double sy = (mapMode == MM_HIMETRIC) ? py : 1.0;

    SIZE total = { long(std::lround(sizeTotal.cx * sx)),
                   long(std::lround(sizeTotal.cy * sy)) };
    SIZE page  = { long(std::lround(sizePage.cx * sx)),
                   long(std::lround(sizePage.cy * sy)) };
    SIZE line  = { long(std::lround(sizeLine.cx * sx)),
                   long(std::lround(sizeLine.cy * sy)) };
    m_host->hostSetScrollSizes(total, page, line);
}

CPoint CScrollView::GetDeviceScrollPosition() const
{
    return m_host ? m_host->hostScrollPosition() : CPoint(0, 0);
}



void CScrollView::OnPrepareDC(CDC* pDC, CPrintInfo* /*info*/)
{
    if (!pDC)
        return;
    pDC->SetMapMode(m_mapMode);
    // В MFC начало области вывода сдвигается на текущую позицию прокрутки —
    // именно так содержимое «уезжает» под полосой прокрутки.
    CPoint dev = GetDeviceScrollPosition();
    pDC->SetViewportOrg(-dev.x, -dev.y);
}

#ifdef _WIN32
// Окно слоя «существует», если описатель не нулевой; сообщение окну передаёт
// обработчик программы (у ядер — закрытие главного окна, planrt_compat.cpp).
/// Обработчик `TmcPostMessage`, заданный программой; NULL — сообщения отбрасываются.
namespace { TmcPostMessageHandler s_postHandler = 0; }
/// @var anonymous_namespace{mfc_view.cpp}::s_postHandler
/// Обработчик `TmcPostMessage`, заданный `tmc_set_post_message_handler`; NULL — сообщения отбрасываются (`TmcPostMessage` возвращает FALSE).
void tmc_set_post_message_handler(TmcPostMessageHandler fn) { s_postHandler = fn; }
BOOL TmcIsWindow(HWND h) { return h != NULL; }
BOOL TmcPostMessage(HWND h, UINT message, WPARAM wParam, LPARAM lParam)
{
    return s_postHandler ? s_postHandler(h, message, wParam, lParam) : FALSE;
}
#endif

#endif // !_WIN32
