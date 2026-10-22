// mfc_gdi.h — рисование MFC поверх Qt.
//
// Виды программ TMC (графики, поля, диаграммы) рисуют через CDC: MoveTo/LineTo,
// перья, кисти, шрифты, режим отображения MM_HIMETRIC. Переписывать эти 3000
// строк на QPainter значит переписывать саму картинку — расхождения полезут в
// мелочах: толщина линии, шаг сетки, положение подписи.
//
// Поэтому код вида компилируется из Windows-версии как есть, а здесь
// реализованы те классы GDI, которыми он пользуется. Внутри каждого — QPainter.
//
// Как устроено отображение. GDI считает так:
//   устройство = (логическая - НачалоОкна) * РазмерОбласти / РазмерОкна + НачалоОбласти
// Для MM_HIMETRIC логическая единица равна 0.01 мм, ось Y направлена ВВЕРХ.
// Мы повторяем эту формулу буквально, а не подменяем её преобразованием QPainter:
// так толщина пера и размер шрифта пересчитываются в пиксели теми же числами,
// что и на Windows.

/// @file mfc_gdi.h
/// @brief Классы рисования GDI/MFC поверх `QPainter`: `CDC`, перо, кисть, шрифт, точка, размер, прямоугольник.
///
/// Часть модуля «Слой совместимости». Код `OnDraw` видов Windows-версии рисует
/// через `CDC` без изменений; `CDC` пересчитывает логические координаты в
/// пиксели по формуле GDI и вызывает `QPainter`. Реализация — `mfc_gdi.cpp`.
/// @see https://www.tamic.ru/docs/code/compat.html#gdi
#pragma once

#if !defined(_WIN32) || defined(TMC_QT_WIN)

#include "tmc_mfc_doc.h"

#include <cmath>
#include <string>
#include <vector>

/// Предварительные объявления классов Qt (заголовки Qt здесь не подключаются).
class QPainter;
class QPaintDevice;
class CPalette;

// --- Описатели GDI -----------------------------------------------------------
// В Windows это непрозрачные числа, которыми система обозначает контекст
// рисования, шрифт и растр. Здесь всеми тремя владеет Qt, поэтому описатель —
// только метка: код Windows-версии сверяет её с нулём и передаёт дальше.
#ifndef _WIN32 // Windows API (под Windows — настоящий windows.h)
/// Описатели GDI (только вне Windows): метки, которые код сверяет с нулём.
typedef void* HDC;        // контекст рисования
typedef void* HFONT;      // шрифт: под Linux шрифтом владеет Qt
typedef void* HBITMAP;    // растр

// --- Точка, размер, прямоугольник -------------------------------------------
/// @brief Точка GDI: `x`, `y` в логических единицах или пикселях (по контексту).
struct POINT { long x; long y; };
/// @brief Размер GDI: `cx` — ширина, `cy` — высота.
struct SIZE  { long cx; long cy; };
/// @brief Прямоугольник GDI: левая, верхняя, правая и нижняя границы (правая и нижняя не входят).
struct RECT  { long left; long top; long right; long bottom; };

// SIZEL в Windows — та же пара координат, что и SIZE (буква L осталась от
// 16-битных времён). Встречается в объявлениях CTmcGLText (FieldView).
/// Синоним `SIZE` из Windows API.
typedef SIZE   SIZEL;
/// Указатель на `SIZEL`.
typedef SIZEL* PSIZEL;

// Описание растра GDI. Нужно, чтобы компилировалось объявление
// CTmcGLText::getBitmap; сам растр строки под Linux строится средствами Qt
// (tmcgltext_qt.cpp MFC).
/// @brief Описание растра GDI (поля как в `wingdi.h`); в Qt6-версии только для компиляции объявлений.
struct BITMAP
{
    /// Тип растра (0).
    long  bmType;
    /// Ширина и высота, пиксели.
    long  bmWidth;
    long  bmHeight;
    /// Длина строки растра, байт.
    long  bmWidthBytes;
    /// Число плоскостей цвета.
    WORD  bmPlanes;
    /// Бит на пиксель.
    WORD  bmBitsPixel;
    /// Указатель на данные растра.
    void* bmBits;
};
#endif // !_WIN32 (Windows API)

/// @brief Точка MFC.
class CPoint : public POINT
{
public:
    /// @brief Создаёт точку (0, 0).
    CPoint() { x = 0; y = 0; }
    /// @brief Создаёт точку с координатами `ax`, `ay`.
    /// @param[in] ax  координата X
    /// @param[in] ay  координата Y
    CPoint(long ax, long ay) { x = ax; y = ay; }
    /// @brief Создаёт копию точки `p`.
    /// @param[in] p  исходная точка
    CPoint(const POINT& p) { x = p.x; y = p.y; }
    /// @brief Сдвигает точку на `dx`, `dy`.
    /// @param[in] dx  сдвиг по X
    /// @param[in] dy  сдвиг по Y
    void Offset(long dx, long dy) { x += dx; y += dy; }
    /// @brief Сравнивает точки покоординатно.
    /// @param[in] o  вторая точка
    /// @return true — обе координаты совпадают.
    bool operator==(const CPoint& o) const { return x == o.x && y == o.y; }
    /// @brief Сравнивает точки покоординатно (отрицание `operator==`).
    /// @param[in] o  вторая точка
    /// @return true — хотя бы одна координата различается.
    bool operator!=(const CPoint& o) const { return !(*this == o); }
};

/// @brief Размер MFC.
class CSize : public SIZE
{
public:
    /// @brief Создаёт нулевой размер.
    CSize() { cx = 0; cy = 0; }
    /// @brief Создаёт размер `acx` x `acy`.
    /// @param[in] acx  ширина
    /// @param[in] acy  высота
    CSize(long acx, long acy) { cx = acx; cy = acy; }
    /// @brief Создаёт копию размера `s`.
    /// @param[in] s  исходный размер
    CSize(const SIZE& s) { cx = s.cx; cy = s.cy; }
};

/// @brief Прямоугольник MFC.
class CRect : public RECT
{
public:
    /// @brief Создаёт пустой прямоугольник (все границы 0).
    CRect() { left = top = right = bottom = 0; }
    /// @brief Создаёт прямоугольник по границам.
    /// @param[in] l  левая
    /// @param[in] t  верхняя
    /// @param[in] r  правая
    /// @param[in] b  нижняя
    CRect(long l, long t, long r, long b) { left = l; top = t; right = r; bottom = b; }
    /// @brief Создаёт копию прямоугольника `r`.
    /// @param[in] r  исходный прямоугольник
    CRect(const RECT& r) { left = r.left; top = r.top; right = r.right; bottom = r.bottom; }
    /// @brief Возвращает левый верхний угол.
    /// @return Точка (`left`, `top`).
    CPoint TopLeft() const { return CPoint(left, top); }
    /// @brief Возвращает правый нижний угол.
    /// @return Точка (`right`, `bottom`).
    CPoint BottomRight() const { return CPoint(right, bottom); }

    /// @brief Задаёт границы прямоугольника (левая, верхняя, правая, нижняя).
    /// @param[in] l  левая граница
    /// @param[in] t  верхняя граница
    /// @param[in] r  правая граница
    /// @param[in] b  нижняя граница
    void SetRect(long l, long t, long r, long b) { left = l; top = t; right = r; bottom = b; }
    /// @brief Обнуляет все границы.
    void SetRectEmpty() { left = top = right = bottom = 0; }
    /// @brief Возвращает TRUE, если ширина или высота <= 0.
    /// @return TRUE — прямоугольник пуст (ширина или высота <= 0); FALSE — не пуст.
    BOOL IsRectEmpty() const { return (right <= left || bottom <= top) ? TRUE : FALSE; }
    /// @brief Проверяет, лежит ли точка внутри (правая и нижняя границы не входят).
    /// @param[in] p  точка
    /// @return TRUE — точка внутри.
    BOOL PtInRect(const POINT& p) const
    {
        return (p.x >= left && p.x < right && p.y >= top && p.y < bottom) ? TRUE : FALSE;
    }
    /// @brief Меняет местами границы так, чтобы `left <= right`, `top <= bottom`.
    void NormalizeRect()
    {
        if (left > right) { long t2 = left; left = right; right = t2; }
        if (top > bottom) { long t2 = top; top = bottom; bottom = t2; }
    }
};

#ifndef _WIN32 // Windows API (под Windows — настоящий windows.h)
// --- Режимы отображения (wingdi.h) ------------------------------------------
/// Режимы отображения GDI. `MM_HIMETRIC` — 0,01 мм, ось Y вверх; `MM_LOMETRIC` — 0,1 мм; `MM_TEXT` — пиксели.
#define MM_TEXT        1
#define MM_LOMETRIC    2
#define MM_HIMETRIC    3
#define MM_LOENGLISH   4
#define MM_HIENGLISH   5
#define MM_TWIPS       6
#define MM_ISOTROPIC   7
#define MM_ANISOTROPIC 8

/// Режим фона текста: прозрачный / непрозрачный.
#define TRANSPARENT 1
#define OPAQUE      2

/// Номера стандартных объектов для `SelectStockObject`.
#define BLACK_PEN    7
#define WHITE_PEN    6
#define NULL_PEN     8
#define BLACK_BRUSH  4
#define WHITE_BRUSH  0
#define NULL_BRUSH   5
#define HOLLOW_BRUSH NULL_BRUSH

/// Растровые операции `PatBlt`: заливка кистью, чёрным, белым.
#define PATCOPY   0x00F00021
#define BLACKNESS 0x00000042
#define WHITENESS 0x00FF0062

/// @brief Метрики шрифта в логических единицах (подмножество полей `wingdi.h`).
struct TEXTMETRIC
{
    /// Высота ячейки символа.
    long tmHeight;
    /// Высота над базовой линией.
    long tmAscent;
    /// Глубина под базовой линией.
    long tmDescent;
    /// Внутренний интервал (всегда 0).
    long tmInternalLeading;
    /// Межстрочный интервал.
    long tmExternalLeading;
    /// Средняя ширина символа.
    long tmAveCharWidth;
    /// Наибольшая ширина символа.
    long tmMaxCharWidth;
    /// Насыщенность шрифта (400 — обычный, 700 — жирный).
    long tmWeight;
};
#endif // !_WIN32 (Windows API)

// --- Объекты рисования -------------------------------------------------------
/// @brief Базовый класс объектов рисования (перо, кисть, шрифт).
///
/// Объект хранит только параметры; в `QPen`/`QBrush`/`QFont` их переводит `CDC`
/// при выборе в контекст. Освобождать системные ресурсы не требуется.
class CGdiObject
{
public:
    /// @brief Виртуальный деструктор.
    virtual ~CGdiObject() {}
    /// Вид объекта: нет, перо, кисть, шрифт.
    enum Kind { kNone, kPen, kBrush, kFont };
    /// @brief Возвращает вид объекта (для `CDC::SelectObject(CGdiObject*)`).
    /// @return `kNone` в базовом классе; наследники возвращают свой вид.
    virtual Kind kind() const { return kNone; }
};

/// @brief Перо GDI: стиль, толщина и цвет линии.
class CPen : public CGdiObject
{
public:
    /// @brief Создаёт сплошное черное перо толщиной 0 (в один пиксель).
    CPen() : m_style(PS_SOLID), m_width(0), m_color(0) {}
    /// @brief Создаёт перо с заданными параметрами.
    /// @param[in] style  стиль `PS_*`
    /// @param[in] width  толщина, логические единицы; 0 — один пиксель
    /// @param[in] color  цвет
    CPen(int style, int width, COLORREF color)
        : m_style(style), m_width(width), m_color(color) {}
    /// @brief Задаёт параметры пера (как конструктор).
    /// @param[in] style  стиль `PS_*`
    /// @param[in] width  толщина, логические единицы
    /// @param[in] color  цвет
    /// @return TRUE.
    BOOL CreatePen(int style, int width, COLORREF color)
    {
        m_style = style; m_width = width; m_color = color;
        return TRUE;
    }
    /// @brief Ничего не делает (ресурсов нет); TRUE.
    /// @return Всегда TRUE.
    BOOL DeleteObject() { return TRUE; }
    /// @brief Возвращает `kPen`.
    /// @return Всегда `kPen`.
    Kind kind() const { return kPen; }

    /// Стиль линии `PS_*`.
    int      m_style;
    /// Толщина линии, логические единицы; 0 — один пиксель.
    int      m_width;      // в логических единицах, как в GDI
    /// Цвет линии.
    COLORREF m_color;
};

/// @brief Кисть GDI: сплошная заливка цветом или «пустая» кисть.
class CBrush : public CGdiObject
{
public:
    /// @brief Создаёт пустую кисть (заливки нет).
    CBrush() : m_color(0), m_null(TRUE) {}
    /// @brief Создаёт сплошную кисть цвета `color`.
    /// @param[in] color  цвет заливки
    explicit CBrush(COLORREF color) : m_color(color), m_null(FALSE) {}
    /// @brief Делает кисть сплошной цвета `color`.
    /// @param[in] color  цвет
    /// @return TRUE.
    BOOL CreateSolidBrush(COLORREF color) { m_color = color; m_null = FALSE; return TRUE; }
    /// @brief Ничего не делает; TRUE.
    /// @return Всегда TRUE.
    BOOL DeleteObject() { return TRUE; }
    /// @brief Возвращает `kBrush`.
    /// @return Всегда `kBrush`.
    Kind kind() const { return kBrush; }

    /// Цвет заливки.
    COLORREF m_color;
    /// TRUE — пустая кисть (заливки нет).
    BOOL     m_null;
};

/// @brief Шрифт GDI, хранящий описание `LOGFONT`.
class CFont : public CGdiObject
{
public:
    /// @brief Создаёт шрифт с нулевым описанием.
    CFont() { std::memset(&m_lf, 0, sizeof(m_lf)); }
    /// @brief Копирует описание шрифта.
    /// @param[in] lf  описание; NULL — без изменений
    /// @return TRUE.
    BOOL CreateFontIndirect(const LOGFONT* lf)
    {
        if (lf) m_lf = *lf;
        return TRUE;
    }
    /// @brief Ничего не делает; TRUE.
    /// @return Всегда TRUE.
    BOOL DeleteObject() { return TRUE; }
    /// @brief Возвращает `kFont`.
    /// @return Всегда `kFont`.
    Kind kind() const { return kFont; }

    /// Шрифт по размеру в десятых долях пункта, как в MFC. Высота — в пикселях
    /// экрана 96 точек на дюйм (отрицательная: размер символа, а не ячейки).
    /// @param[in] pointSize10  размер, десятые доли пункта (120 — 12 пт)
    /// @param[in] face         имя гарнитуры или NULL
    /// Третий параметр (контекст рисования) не используется.
    /// @return TRUE.
    BOOL CreatePointFont(int pointSize10, const char* face, class CDC* = 0)
    {
        std::memset(&m_lf, 0, sizeof(m_lf));
        m_lf.lfHeight = -((pointSize10 * 96 + 360) / 720);
        m_lf.lfWeight = 400;
        if (face) std::strncpy(m_lf.lfFaceName, face, sizeof(m_lf.lfFaceName) - 1);
        return TRUE;
    }

    /// Описание шрифта; высота — в логических единицах контекста, куда шрифт выбран.
    LOGFONT m_lf;
};

// --- Контекст рисования ------------------------------------------------------
/// @brief Контекст рисования MFC, рисующий через `QPainter`.
///
/// Хранит режим отображения (начала и расширения окна и области вывода),
/// выбранные перо, кисть, шрифт, цвета текста и фона. Каждая координата
/// переводится в пиксели по формуле GDI:
/// `dev = (log - WinOrg) * VpExt / WinExt + VpOrg`.
/// Создаётся виджетом-хозяином на время одной перерисовки (`OnDraw`) или печати
/// и связывается с `QPainter` через `Attach`. Не владеет ни `QPainter`, ни
/// выбранными объектами. Только поток интерфейса.
/// @see https://www.tamic.ru/docs/code/compat.html#gdi
class CDC
{
public:
    /// @brief Создаёт контекст в режиме `MM_TEXT` без привязки к `QPainter`.
    CDC();
    /// @brief Виртуальный деструктор (ничего не освобождает).
    virtual ~CDC();

    // Связь с Qt. Устанавливает виджет-хозяин перед вызовом OnDraw.
    /// @brief Связывает контекст с рисовальщиком Qt и устройством (виджет, принтер, картинка).
    /// @param[in] painter  активный `QPainter` или NULL (тогда рисование не выполняется, текст откладывается)
    /// @param[in] device   устройство, у которого берутся разрешение и размер; может быть NULL
    void Attach(QPainter* painter, QPaintDevice* device);
    /// @brief Возвращает связанный `QPainter` (может быть NULL).
    /// @return Рисовальщик (не владеет контекст); NULL — контекст не связан.
    QPainter* painter() const { return m_painter; }

    // --- Режим отображения ---------------------------------------------------
    /// @brief Устанавливает режим отображения.
    ///
    /// Для `MM_HIMETRIC`/`MM_LOMETRIC` расширения вычисляются по разрешению
    /// устройства (`LOGPIXELSX/Y`), ось Y направлена вверх; для `MM_TEXT` — 1:1.
    /// Для `MM_ISOTROPIC`/`MM_ANISOTROPIC` расширения задаёт вызывающий код.
    /// @param[in] mode  режим `MM_*`
    /// @return Прежний режим.
    virtual int  SetMapMode(int mode);
    /// @brief Возвращает текущий режим отображения.
    /// @return Режим `MM_*`.
    int  GetMapMode() const { return m_mapMode; }
    /// @brief Задаёт расширение логического окна.
    /// @param[in] cx  по X, логические единицы
    /// @param[in] cy  по Y, логические единицы
    /// @return Прежнее расширение.
    CSize SetWindowExt(int cx, int cy);
    /// @brief Задаёт расширение области вывода.
    /// @param[in] cx  по X, пиксели (знак задаёт направление оси)
    /// @param[in] cy  по Y, пиксели
    /// @return Прежнее расширение.
    CSize SetViewportExt(int cx, int cy);
    /// @brief Задаёт начало логического окна.
    /// @param[in] x  логическая X
    /// @param[in] y  логическая Y
    /// @return Прежнее начало.
    CPoint SetWindowOrg(int x, int y);
    /// @brief Задаёт начало области вывода.
    /// @param[in] x  пиксели
    /// @param[in] y  пиксели
    /// @return Прежнее начало.
    CPoint SetViewportOrg(int x, int y);
    /// @brief Возвращает начало логического окна.
    /// @return Начало окна, логические единицы.
    CPoint GetWindowOrg() const { return m_winOrg; }
    /// @brief Возвращает начало области вывода, пиксели.
    /// @return Начало области вывода, пиксели.
    CPoint GetViewportOrg() const { return m_vpOrg; }

    /// @brief Переводит точки из логических единиц в пиксели (на месте).
    /// @param[in,out] points  массив точек
    /// @param[in]     count   число точек
    void LPtoDP(POINT* points, int count = 1) const;
    /// @brief Переводит точки из пикселей в логические единицы (на месте).
    /// @param[in,out] points  массив точек
    /// @param[in]     count   число точек
    void DPtoLP(POINT* points, int count = 1) const;
    /// @brief Переводит прямоугольник из логических единиц в пиксели (на месте).
    /// @param[in,out] rect  прямоугольник
    void LPtoDP(RECT* rect) const;
    /// @brief Переводит прямоугольник из пикселей в логические единицы (на месте).
    /// @param[in,out] rect  прямоугольник
    void DPtoLP(RECT* rect) const;
    // Код видов передаёт прямоугольник и по ссылке — как в MFC.
    /// @brief То же, что `LPtoDP(RECT*)`, для прямоугольника по ссылке.
    /// @param[in,out] rect  прямоугольник
    void LPtoDP(RECT& rect) const { LPtoDP(&rect); }
    /// @brief То же, что `DPtoLP(RECT*)`, для прямоугольника по ссылке.
    /// @param[in,out] rect  прямоугольник
    void DPtoLP(RECT& rect) const { DPtoLP(&rect); }
    /// @brief Переводит одну точку из пикселей в логические единицы (на месте).
    /// @param[in,out] p  точка
    void DPtoLP(POINT& p) const { DPtoLP(&p, 1); }
    /// @brief Переводит одну точку из логических единиц в пиксели (на месте).
    /// @param[in,out] p  точка
    void LPtoDP(POINT& p) const { LPtoDP(&p, 1); }

    // --- Объекты -------------------------------------------------------------
    /// @brief Выбирает объект любого вида (по `kind()`).
    /// @param[in] obj  объект или NULL
    /// @return Прежний объект того же вида; NULL — не было или `obj == NULL`.
    CGdiObject* SelectObject(CGdiObject* obj);
    /// @brief Выбирает перо и сразу применяет его к `QPainter`.
    /// @param[in] pen  перо или NULL (тогда — тонкое чёрное)
    /// @return Прежнее перо (может быть NULL).
    CPen*   SelectObject(CPen* pen);
    /// @brief Выбирает кисть.
    /// @param[in] brush  кисть или NULL (заливки нет)
    /// @return Прежняя кисть.
    CBrush* SelectObject(CBrush* brush);
    /// @brief Выбирает шрифт и применяет его к `QPainter`.
    /// @param[in] font  шрифт или NULL
    /// @return Прежний шрифт.
    CFont*  SelectObject(CFont* font);
    /// @brief Выбирает стандартный объект (`BLACK_PEN`, `NULL_BRUSH` …).
    /// @param[in] index  номер стандартного объекта
    /// @return Прежний объект того же вида; NULL — номер не поддержан.
    CGdiObject* SelectStockObject(int index);

    // --- Рисование -----------------------------------------------------------
    /// @brief Переносит текущую позицию пера без рисования.
    /// @param[in] x  логическая X
    /// @param[in] y  логическая Y
    /// @return Прежняя позиция.
    CPoint MoveTo(int x, int y);
    /// @brief То же, что `MoveTo(p.x, p.y)`.
    /// @param[in] p  новая позиция, логические единицы
    /// @return Прежняя позиция.
    CPoint MoveTo(POINT p) { return MoveTo(p.x, p.y); }
    /// @brief Рисует линию от текущей позиции до точки и переносит позицию туда.
    /// @param[in] x  логическая X
    /// @param[in] y  логическая Y
    /// @return TRUE.
    BOOL   LineTo(int x, int y);
    /// @brief То же, что `LineTo(p.x, p.y)`.
    /// @param[in] p  конечная точка, логические единицы
    /// @return Как у `LineTo(int, int)`.
    BOOL   LineTo(POINT p) { return LineTo(p.x, p.y); }
    /// @brief Возвращает текущую позицию пера, логические единицы.
    /// @return Текущая позиция, логические единицы.
    CPoint GetCurrentPosition() const { return m_cur; }
    /// @brief Рисует ломаную текущим пером.
    /// @param[in] points  вершины, логические единицы
    /// @param[in] count   число вершин (>= 2)
    /// @return FALSE — нет `QPainter` или вершин меньше двух.
    BOOL   Polyline(const POINT* points, int count);
    /// @brief Рисует прямоугольник текущими пером и кистью.
    /// @param[in] l  левая граница, логические единицы
    /// @param[in] t  верхняя
    /// @param[in] r  правая
    /// @param[in] b  нижняя
    /// @return FALSE — нет `QPainter`.
    BOOL   Rectangle(int l, int t, int r, int b);
    /// @brief То же для прямоугольника `r`.
    /// @param[in] r  прямоугольник, логические единицы (не NULL)
    /// @return FALSE — нет `QPainter`.
    BOOL   Rectangle(const RECT* r) { return Rectangle(r->left, r->top, r->right, r->bottom); }
    /// @brief Рисует эллипс, вписанный в прямоугольник.
    /// @param[in] l  левая граница, логические единицы
    /// @param[in] t  верхняя
    /// @param[in] r  правая
    /// @param[in] b  нижняя
    /// @return FALSE — нет `QPainter`.
    BOOL   Ellipse(int l, int t, int r, int b);
    /// @brief Заливает прямоугольник цветом (без пера).
    /// @param[in] r      прямоугольник, логические единицы; NULL — ничего
    /// @param[in] color  цвет
    void   FillSolidRect(const RECT* r, COLORREF color);
    /// @brief Заливает прямоугольник цветом.
    /// @param[in] x      левая граница, логические единицы
    /// @param[in] y      верхняя граница
    /// @param[in] cx     ширина
    /// @param[in] cy     высота
    /// @param[in] color  цвет
    void   FillSolidRect(int x, int y, int cx, int cy, COLORREF color);
    /// @brief Заливает прямоугольник по растровой операции.
    /// @param[in] x    левая граница, логические единицы
    /// @param[in] y    верхняя граница
    /// @param[in] w    ширина
    /// @param[in] h    высота
    /// @param[in] rop  `BLACKNESS`, `WHITENESS`; иначе — цветом кисти (или фона, если кисти нет)
    /// @return FALSE — нет `QPainter`.
    BOOL   PatBlt(int x, int y, int w, int h, DWORD rop);

    /// @brief Выводит строку; левый верхний угол текста — в точке (x, y).
    ///
    /// Текст перекодируется из CP1251. В режиме отложенного текста (`SetDeferText`)
    /// или без `QPainter` строка запоминается в `deferredText()`.
    /// @param[in] x     логическая X
    /// @param[in] y     логическая Y
    /// @param[in] text  строка в CP1251
    /// @param[in] len   длина, байт; < 0 — до нуля
    /// @return FALSE — `text == NULL`.
    BOOL   TextOut(int x, int y, const char* text, int len);
    /// @brief То же для `CString`.
    /// @param[in] x     логическая X
    /// @param[in] y     логическая Y
    /// @param[in] text  строка в CP1251 (выводится целиком)
    /// @return Как у `TextOut(int, int, const char*, int)`.
    BOOL   TextOut(int x, int y, const CString& text);
    /// @brief Возвращает размер строки в логических единицах текущим шрифтом.
    /// @param[in] text  строка в CP1251
    /// @param[in] len   длина, байт; < 0 — до нуля
    /// @return Ширина и высота; (0, 0) — нет `QPainter` или `text == NULL`.
    CSize  GetTextExtent(const char* text, int len) const;
    /// @brief То же для `CString`.
    /// @param[in] text  строка в CP1251 (измеряется целиком)
    /// @return Ширина и высота, логические единицы; (0, 0) — нет `QPainter`.
    CSize  GetTextExtent(const CString& text) const;
    /// @brief Заполняет метрики текущего шрифта в логических единицах.
    /// @param[out] tm  метрики
    /// @return FALSE — нет `QPainter` или `tm == NULL`.
    BOOL   GetTextMetrics(TEXTMETRIC* tm) const;

    /// @brief Задаёт цвет текста.
    /// @param[in] color  цвет
    /// @return Прежний цвет.
    COLORREF SetTextColor(COLORREF color);
    /// @brief Возвращает цвет текста.
    /// @return Цвет текста `RGB`.
    COLORREF GetTextColor() const { return m_textColor; }
    /// @brief Задаёт цвет фона текста.
    /// @param[in] color  цвет
    /// @return Прежний цвет.
    COLORREF SetBkColor(COLORREF color);
    /// @brief Возвращает цвет фона текста.
    /// @return Цвет фона `RGB`.
    COLORREF GetBkColor() const { return m_bkColor; }
    /// @brief Задаёт режим фона текста.
    /// @param[in] mode  `OPAQUE` — под текстом заливка цветом фона, `TRANSPARENT` — без заливки
    /// @return Прежний режим.
    int      SetBkMode(int mode);

    /// @brief Возвращает границы области отсечения (или всего устройства) в логических единицах.
    /// @param[out] r  прямоугольник
    /// @return FALSE — нет `QPainter` или `r == NULL`.
    BOOL GetClipBox(RECT* r) const;

    // Описатель контекста Windows. Нужен только как значение, которое код
    // Windows-версии передаёт в вызовы OpenGL: контекстом владеет Qt.
    /// @brief Возвращает NULL: описателя контекста Windows нет.
    /// @return Всегда NULL.
    void* GetSafeHdc() const { return 0; }

    // --- Текст поверх сцены OpenGL -------------------------------------------
    // Вид FieldView рисует сцену через OpenGL, но сообщения (например, «нет
    // файла топологии») выводит обычным TextOut. Рисовать их тем же контекстом
    // нельзя: холста в этот момент нет. Поэтому в режиме OpenGL текст
    // запоминается, а виджет-хозяин выводит его поверх готового кадра.
    /// @brief Отложенная надпись: выводится виджетом поверх кадра OpenGL.
    struct DeferredText
    {
        /// Координата X левого верхнего угла, пиксели.
        long     x;
        /// Координата Y левого верхнего угла, пиксели.
        long     y;
        /// Текст в CP1251.
        std::string text;    // в кодировке CP1251, как в исходниках
        /// Цвет текста.
        COLORREF color;
        /// Шрифт (нули, если шрифт не был выбран).
        LOGFONT  font;
    };

    /// @brief Включает режим отложенного текста (для видов OpenGL).
    /// @param[in] defer  true — `TextOut` только запоминает надписи
    void SetDeferText(bool defer) { m_deferText = defer; }
    /// @brief Возвращает накопленные отложенные надписи.
    /// @return Ссылка на список надписей (принадлежит контексту); пуст — надписей нет.
    const std::vector<DeferredText>& deferredText() const { return m_deferredText; }
    /// @brief Очищает список отложенных надписей.
    void ClearDeferredText() { m_deferredText.clear(); }
    /// @brief Возвращает TRUE, если контекст рисует на принтер.
    /// @return TRUE — печать (`SetPrinting(TRUE)`); FALSE — экран.
    BOOL IsPrinting() const { return m_printing; }
    /// @brief Отмечает контекст как печатающий.
    /// @param[in] p  TRUE — печать
    void SetPrinting(BOOL p) { m_printing = p; }

    /// @brief Возвращает сведения об устройстве `QPaintDevice`.
    ///
    /// В Windows-сборке имя метода макросом превращается в `TmcGetDeviceCaps`
    /// (см. `tmc_winapi.h`), для вызывающего кода это незаметно.
    /// @param[in] index  `LOGPIXELSX/Y` (точек на дюйм), `HORZRES/VERTRES` (пиксели), `HORZSIZE/VERTSIZE` (мм)
    /// @return Значение; без устройства — 96 dpi, 1024x768, 270x200 мм; неизвестный индекс — 0.
    int GetDeviceCaps(int index) const;

    // --- Палитра -------------------------------------------------------------
    // В Windows 256-цветные режимы требовали выбрать палитру в контекст и
    // «воплотить» её (FieldView, CFldViewView::CreateRGBPalette). Qt всегда даёт
    // полноцветную поверхность, поэтому ветка кода, которая это делает, не
    // выполняется, а вызовы ничего не меняют. Возвращается прежняя палитра,
    // как в MFC (её нет — значит ноль).
    /// @brief Ничего не делает: палитр в полноцветном режиме нет.
    /// @param[in] palette          палитра (не используется)
    /// @param[in] forceBackground  признак фоновой палитры (не используется)
    /// @return NULL.
    CPalette* SelectPalette(CPalette* palette, BOOL forceBackground);
    /// @brief Ничего не делает.
    /// @return 0 — ни один цвет не изменён.
    UINT      RealizePalette();

protected:
    /// @brief Переводит выбранное перо в `QPen` (цвет темы, стиль, толщина в пикселях).
    void   applyPen();
    /// @brief Переводит выбранный шрифт в `QFont` (гарнитура-замена, высота в пикселях).
    void   applyFont();
    /// @brief Переводит логическую X в пиксели по формуле GDI.
    /// @param[in] lx  координата X, логические единицы
    /// @return Координата X, пиксели (с округлением до ближайшего целого).
    long   deviceX(long lx) const;
    /// @brief Переводит логическую Y в пиксели по формуле GDI.
    /// @param[in] ly  координата Y, логические единицы
    /// @return Координата Y, пиксели (с округлением до ближайшего целого).
    long   deviceY(long ly) const;
    /// @brief Переводит X из пикселей в логические единицы.
    /// @param[in] dx  координата X, пиксели
    /// @return Координата X, логические единицы (с округлением); 0 — масштаб равен нулю.
    long   logicalX(long dx) const;
    /// @brief Переводит Y из пикселей в логические единицы.
    /// @param[in] dy  координата Y, пиксели
    /// @return Координата Y, логические единицы (с округлением); 0 — масштаб равен нулю.
    long   logicalY(long dy) const;
    /// @brief Возвращает масштаб по X: `VpExt.cx / WinExt.cx` (1, если `WinExt.cx == 0`).
    /// @return Пикселей на логическую единицу по X (знак — направление оси).
    double scaleX() const;
    /// @brief Возвращает масштаб по Y: `VpExt.cy / WinExt.cy` (1, если `WinExt.cy == 0`).
    /// @return Пикселей на логическую единицу по Y (знак — направление оси).
    double scaleY() const;

    /// Рисовальщик Qt (не владеет); NULL — рисование не выполняется.
    QPainter*     m_painter;
    /// Устройство рисования (не владеет).
    QPaintDevice* m_device;

    /// Режим отображения `MM_*`.
    int    m_mapMode;
    /// Расширение логического окна.
    CSize  m_winExt;
    /// Расширение области вывода, пиксели (знак — направление оси).
    CSize  m_vpExt;
    /// Начало логического окна.
    CPoint m_winOrg;
    /// Начало области вывода, пиксели.
    CPoint m_vpOrg;

    /// Текущая позиция пера, логические единицы.
    CPoint   m_cur;
    /// Выбранное перо (не владеет); NULL — тонкое чёрное.
    CPen*    m_pen;
    /// Выбранная кисть (не владеет); NULL — без заливки.
    CBrush*  m_brush;
    /// Выбранный шрифт (не владеет); NULL — шрифт `QPainter` по умолчанию.
    CFont*   m_font;
    /// Цвет текста.
    COLORREF m_textColor;
    /// Цвет фона текста.
    COLORREF m_bkColor;
    /// Режим отложенного текста.
    bool     m_deferText = false;
    /// Накопленные отложенные надписи.
    std::vector<DeferredText> m_deferredText;
    /// Режим фона текста `OPAQUE`/`TRANSPARENT`.
    int      m_bkMode;
    /// Признак печати.
    BOOL     m_printing;
};

#ifndef _WIN32 // Windows API (под Windows — настоящий windows.h)
// Константы GetDeviceCaps, которыми пользуются виды.
/// Индексы `GetDeviceCaps`: размеры в мм, в пикселях, точек на дюйм.
#define HORZSIZE     4
#define VERTSIZE     6
#define HORZRES      8
#define VERTRES     10
#define LOGPIXELSX  88
#define LOGPIXELSY  90
#endif // !_WIN32 (Windows API)

// Те же сведения об устройстве, но как свободная функция Windows API: код вида
// зовёт её с описателем контекста OpenGL —
// `GetDeviceCaps( wglGetCurrentDC(), LOGPIXELSY )` (fldviewview.cpp,
// tmcdialogplace.cpp). Отсюда берётся высота шрифта подписей осей (12 пунктов),
// поэтому значение должно быть настоящим разрешением экрана.
/// @brief Возвращает сведения об основном экране (свободная функция Windows API).
///
/// В Windows-сборке имя заменено макросом на `TmcGetDeviceCaps` (`tmc_winapi.h`).
/// @param[in] dc     описатель контекста (не используется)
/// @param[in] index  `LOGPIXELSX/Y`, `HORZRES/VERTRES`, `HORZSIZE/VERTSIZE`
/// @return Значение; без экрана — 96 dpi, 1024x768, 270x200 мм; неизвестный индекс — 0.
int GetDeviceCaps(HDC dc, int index);

#endif // !_WIN32
