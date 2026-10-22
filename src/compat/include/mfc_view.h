// mfc_view.h — окна и виды MFC поверх Qt.
//
// Продолжение слоя совместимости: здесь то, от чего наследуются виды программ
// TMC (CScrollView) и что они зовут у окна — перерисовка, захват мыши, таймер,
// прокрутка, подготовка контекста рисования.
//
// Qt-виджет не наследуется от этих классов, а РЕАЛИЗУЕТ интерфейс MfcQtHost и
// отдаёт виду события. Благодаря этому в слое совместимости нет ни одного
// включения Qt: он собирается и без графики.

/// @file mfc_view.h
/// @brief Окна и виды MFC без Qt: `CWnd`, `CView`, `CScrollView`, контексты `CClientDC`/`CPaintDC` и интерфейс-мост `MfcQtHost`.
///
/// Часть модуля «Слой совместимости». Вид программы (наследник `CScrollView`)
/// не знает о Qt: всё, что ему нужно от окна (перерисовка, таймер, мышь,
/// прокрутка), он получает через `MfcQtHost`, который реализует виджет-хозяин
/// `gui_common/mfcviewwidget`. Реализация — `mfc_view.cpp`.
/// @see https://www.tamic.ru/docs/code/compat.html#doc-view
#pragma once

#if !defined(_WIN32) || defined(TMC_QT_WIN)

#include "mfc_gdi.h"

// --- Отладочные макросы MFC --------------------------------------------------
// В Windows-сборке они разворачивались в проверки отладчика. Здесь пустые:
// падать на пользователе из-за проверки — хуже, чем нарисовать что-то не так.
/// Отладочные макросы MFC: проверки отключены, `VERIFY` вычисляет выражение, `TRACE` ничего не выводит.
#ifndef ASSERT
  #define ASSERT(expr)        ((void)0)
#endif
#ifndef ASSERT_VALID
  /// Проверка объекта MFC: пустая.
  #define ASSERT_VALID(p)     ((void)0)
#endif
#ifndef VERIFY
  /// Вычисляет выражение без проверки результата.
  #define VERIFY(expr)        ((void)(expr))
#endif
#ifndef TRACE
  /// Отладочный вывод MFC: аргументы не вычисляются.
  #define TRACE               (void)sizeof
#endif
#ifndef RUNTIME_CLASS
  /// Сведения о классе MFC: всегда NULL.
  #define RUNTIME_CLASS(cls)  ((void*)0)
#endif
/// Идентификатор первой панели окна-рамки MFC.
#ifndef AFX_IDW_PANE_FIRST
  #define AFX_IDW_PANE_FIRST 0xE900
#endif

#ifndef _WIN32 // Windows API (под Windows — настоящий windows.h)
// Стили окна, встречающиеся в PreCreateWindow. Значения как в Windows.
/// Стили окна и номера полос прокрутки (значения Windows; только вне Windows).
#define WS_HSCROLL      0x00100000L
#define WS_VSCROLL      0x00200000L
#define WS_MAXIMIZE     0x01000000L
#define SB_HORZ 0
#define SB_VERT 1

/// Дополнительные стандартные кисти и типы перьев (значения Windows).
#define GRAY_BRUSH   2
#define LTGRAY_BRUSH 1
#define DKGRAY_BRUSH 3
#define PS_COSMETIC  0x00000000
#define PS_GEOMETRIC 0x00010000

// Первое сообщение, которое программа может определить сама. От него отсчитаны
// собственные сообщения TMC: «применить страницу свойств» (WM_USERAPPLY_* в
// fldview.h) и «окно уровней цвета закрылось» (WM_COLOR_LEVEL_DIALOG_BYE в
// colorleveldlg.h). Значение как в Windows.
#ifndef WM_USER
  #define WM_USER 0x0400
#endif

// Кнопки и клавиши-модификаторы в обработчиках мыши.
/// Флаги нажатых кнопок мыши и клавиш Shift/Ctrl в параметре `nFlags` обработчиков мыши.
#define MK_LBUTTON  0x0001
#define MK_RBUTTON  0x0002
#define MK_SHIFT    0x0004
#define MK_CONTROL  0x0008
#define MK_MBUTTON  0x0010

/// @brief Параметры создания окна для `PreCreateWindow` (поля как в Windows; слоем не используются).
struct CREATESTRUCT
{
    void*        lpCreateParams;
    void*        hInstance;
    void*        hMenu;
    void*        hwndParent;
    int          cy, cx, y, x;
    long         style;
    const char*  lpszName;
    const char*  lpszClass;
    DWORD        dwExStyle;
};
#endif // !_WIN32 (Windows API)

/// @brief Сведения о печати MFC: признак предпросмотра и номер текущей страницы.
class CPrintInfo
{
public:
    /// TRUE — предварительный просмотр, FALSE — печать.
    BOOL m_bPreview = FALSE;
    /// Номер печатаемой страницы, с 1.
    UINT m_nCurPage = 1;
};

// --- Мост к Qt ----------------------------------------------------------------
// Всё, что виду нужно от настоящего окна. Реализует Qt-виджет-хозяин.
/// @brief Интерфейс, через который вид MFC обращается к окну Qt.
///
/// Реализуется виджетом-хозяином (`MfcViewWidget`). Вид получает
/// указатель через `CWnd::SetHost` и не владеет им. Все методы вызываются из
/// потока интерфейса.
/// @see https://www.tamic.ru/docs/code/compat.html#doc-view
class MfcQtHost
{
public:
    /// @brief Виртуальный деструктор.
    virtual ~MfcQtHost() {}

    /// @brief Запрашивает перерисовку.
    /// @param[in] logicalRect  область в логических единицах вида или NULL — всё окно
    /// @param[in] erase        стирать ли фон (не влияет: Qt перерисовывает целиком)
    virtual void hostInvalidate(const RECT* logicalRect, BOOL erase) = 0;
    /// @brief Выполняет отложенную перерисовку немедленно.
    virtual void hostUpdateWindow() = 0;
    /// @brief Возвращает клиентскую область окна.
    /// @param[out] deviceRect  прямоугольник, пиксели: (0, 0, ширина, высота)
    virtual void hostGetClientRect(RECT* deviceRect) const = 0;
    /// @brief Захватывает мышь (события мыши идут в окно и за его пределами).
    virtual void hostSetCapture() = 0;
    /// @brief Освобождает захват мыши.
    virtual void hostReleaseCapture() = 0;
    /// @brief Запускает таймер окна; по срабатыванию хозяин зовёт `OnTimer(id)`.
    /// @param[in] id         номер таймера
    /// @param[in] elapseMs   период, мс
    /// @return Номер таймера (`id`) или 0 при ошибке.
    virtual UINT hostSetTimer(UINT id, UINT elapseMs) = 0;
    /// @brief Останавливает таймер.
    /// @param[in] id  номер таймера
    /// @return TRUE — таймер был запущен.
    virtual BOOL hostKillTimer(UINT id) = 0;
    /// @brief Показывает модальное окно сообщения.
    /// @param[in] text     текст (CP1251)
    /// @param[in] caption  заголовок или NULL
    /// @param[in] type     флаги `MB_*` (кнопки, значок)
    /// @return Код нажатой кнопки `IDOK`, `IDYES` …
    virtual int  hostMessageBox(const char* text, const char* caption, UINT type) = 0;
    /// @brief Задаёт размеры прокручиваемой области.
    /// @param[in] total  полный размер содержимого, пиксели
    /// @param[in] page   шаг прокрутки страницей, пиксели
    /// @param[in] line   шаг прокрутки строкой, пиксели
    virtual void hostSetScrollSizes(const SIZE& total, const SIZE& page, const SIZE& line) = 0;
    /// @brief Возвращает текущую позицию прокрутки, пиксели.
    /// @return Позиция левого верхнего угла видимой части содержимого, пиксели.
    virtual CPoint hostScrollPosition() const = 0;      // в устройстве (пикселях)
    /// @brief Прокручивает к позиции.
    /// @param[in] devicePos  позиция, пиксели
    virtual void hostScrollToPosition(const POINT& devicePos) = 0;
    /// @brief Выводит текст в поле строки состояния главного окна.
    /// @param[in] pane  номер поля (0 — основное)
    /// @param[in] text  текст (CP1251)
    virtual void hostSetStatusText(int pane, const char* text) = 0;
    /// @brief Возвращает логическое разрешение окна по X, точек на дюйм.
    /// @return Точек на дюйм по X.
    virtual int  hostLogicalDpiX() const = 0;
    /// @brief Возвращает логическое разрешение окна по Y, точек на дюйм.
    /// @return Точек на дюйм по Y.
    virtual int  hostLogicalDpiY() const = 0;
};

// --- CWnd ---------------------------------------------------------------------
/// @brief Окно MFC: перерисовка, мышь, таймеры, сообщения — через `MfcQtHost` или окно диалога Qt.
///
/// Базовый класс видов (`CView`) и диалогов (`CDialog`). Пока хозяин не задан
/// (`m_host == NULL`), вызовы ничего не делают.
/// @see https://www.tamic.ru/docs/code/compat.html#doc-view
class CWnd : public CObject
{
public:
    /// @brief Создаёт окно без хозяина.
    CWnd() : m_host(0) {}
    /// @brief Виртуальный деструктор.
    virtual ~CWnd() {}

    /// @brief Задаёт виджет-хозяин (вызывает сам виджет при создании).
    /// @param[in] host  хозяин или NULL — отвязать
    void SetHost(MfcQtHost* host) { m_host = host; }
    /// @brief Возвращает виджет-хозяин (может быть NULL).
    /// @return Хозяин (не владеет окно); NULL — хозяин не задан.
    MfcQtHost* Host() const { return m_host; }

    /// @brief Запрашивает перерисовку всего окна.
    /// @param[in] erase  стирать ли фон (не влияет)
    void Invalidate(BOOL erase = TRUE);
    /// @brief Запрашивает перерисовку области.
    /// @param[in] rect   область в логических единицах или NULL — всё окно
    /// @param[in] erase  стирать ли фон (не влияет)
    void InvalidateRect(const RECT* rect, BOOL erase = TRUE);
    /// @brief То же для прямоугольника по ссылке.
    /// @param[in] rect   область в логических единицах
    /// @param[in] erase  стирать ли фон (не влияет)
    void InvalidateRect(const RECT& rect, BOOL erase = TRUE) { InvalidateRect(&rect, erase); }
    /// @brief Выполняет отложенную перерисовку немедленно.
    void UpdateWindow();
    // В MFC перерисовывает окно немедленно; здесь — то же самое через Qt.
    /// Параметры (область, регион, флаги) не используются.
    void RedrawWindow(const RECT* = 0, void* = 0, UINT = 0) { UpdateWindow(); }
    /// @brief Возвращает клиентскую область, пиксели; без хозяина — нули.
    /// @param[out] rect  прямоугольник; NULL — ничего
    void GetClientRect(RECT* rect) const;
    /// @brief Захватывает мышь.
    void SetCapture();
    /// @brief Освобождает захват мыши.
    void ReleaseCapture();
    /// @brief Запускает таймер окна.
    /// @param[in] id      номер таймера
    /// @param[in] elapse  период, мс
    /// @param[in] proc    функция обратного вызова (не поддерживается, игнорируется)
    /// @return Номер таймера или 0 (нет хозяина).
    /// @see https://www.tamic.ru/docs/code/compat.html#timers
    UINT SetTimer(UINT id, UINT elapse, void* proc = 0);
    /// @brief Показывает модальное окно сообщения.
    /// @param[in] text     текст (CP1251)
    /// @param[in] caption  заголовок или NULL
    /// @param[in] type     флаги `MB_*`
    /// @return Код нажатой кнопки; 0 — нет хозяина.
    int  MessageBox(const char* text, const char* caption = 0, UINT type = 0);
    /// @brief Возвращает само окно (отдельной рамки MFC нет).
    /// @return Указатель на само окно (`this`).
    CWnd* GetParentFrame() { return this; }
    /// @brief Всегда TRUE: проверка типа MFC не выполняется.
    /// @return Всегда TRUE.
    BOOL IsKindOf(const void*) const { return TRUE; }

    // --- Сообщения окну -------------------------------------------------------
    // В Windows окна обмениваются сообщениями через очередь: диалог уровней
    // цвета сообщает виду, что закрылся, а страница свойств — что нажали
    // «Применить». Обработчик такого сообщения объявлен картой ON_MESSAGE.
    // Здесь очереди Windows нет, поэтому сообщение ищется в той же карте и
    // обработчик вызывается напрямую:
    //   SendMessage — сразу, как в Windows;
    //   PostMessage — после возврата в цикл событий Qt (это важно: отправитель
    //   обычно просит закрыть себя же, и разрушать его посреди его метода
    //   нельзя).
    /// Возвращает результат обработчика (0, если обработчика нет).
    /// @brief Вызывает обработчик `ON_MESSAGE` из карты сообщений немедленно.
    /// @param[in] message  код сообщения
    /// @param[in] wParam   первый параметр
    /// @param[in] lParam   второй параметр
    /// @see https://www.tamic.ru/docs/code/compat.html#message-map
    /// @return Результат обработчика; 0 — обработчика нет.
    LRESULT SendMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
    /// Возвращает TRUE, если сообщение поставлено в очередь.
    /// @brief Ставит вызов обработчика `ON_MESSAGE` в очередь событий Qt.
    /// @param[in] message  код сообщения
    /// @param[in] wParam   первый параметр
    /// @param[in] lParam   второй параметр
    /// @return TRUE — вызов поставлен в очередь главного потока; FALSE — нет объекта приложения Qt.
    BOOL    PostMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

    /// Описатель окна. Настоящих окон Windows нет: у вида это виджет-хозяин, у
    /// диалога — его окно Qt (см. CDialog). Код Windows-версии сверяет
    /// результат только с нулём («окна ещё нет — создать»).
    /// @return Указатель на хозяина (вид) или окно Qt (диалог); NULL — окна нет.
    virtual HWND GetSafeHwnd() const { return reinterpret_cast<HWND>(m_host); }

    // Рисование прямо в элементе (образец линии в диалоге цвета). В Qt рисовать
    // по виджету вне перерисовки нельзя, поэтому обёртка элемента отдаёт холст
    // над картинкой и подставляет её элементу в ReleaseDC.
    /// @brief Возвращает контекст для рисования в окне вне `OnDraw`; у вида — NULL.
    /// @return NULL в базовом классе; обёртки элементов возвращают новый контекст.
    virtual CDC* GetDC() { return 0; }
    /// @brief Освобождает контекст `GetDC`; у вида ничего не делает.
    virtual void ReleaseDC(CDC*) {}

    /// Закрытие окна. В Qt окном распоряжается виджет-хозяин, поэтому здесь
    /// только уведомление вида (OnDestroy) — как в MFC перед разрушением.
    /// @brief Вызывается перед разрушением окна; базовая реализация пустая.
    virtual void OnDestroy() {}
    /// @brief Вызывает `OnDestroy`.
    /// @return TRUE.
    virtual BOOL DestroyWindow() { OnDestroy(); return TRUE; }

    /// Шрифт и фокус элемента (окно отчёта о времени расчёта, снимок 22.10).
    /// У окна вида ничего не делают; у элемента диалога — см. CWndCtrl.
    /// Параметры (шрифт и признак перерисовки) у вида не используются.
    virtual void SetFont(class CFont* /*font*/, BOOL /*redraw*/ = TRUE) {}
    /// @brief Переводит фокус ввода на окно; у вида ничего не делает.
    virtual void SetFocus() {}

protected:
    /// Виджет-хозяин (не владеет); NULL — окно ещё не показано.
    MfcQtHost* m_host;
};

// --- Контексты рисования ------------------------------------------------------
// CClientDC в коде видов используется только для перевода координат мыши
// (DPtoLP): само рисование идёт в OnDraw. Поэтому холста внутри нет — есть
// разрешение экрана, которого достаточно для пересчёта.
/// @brief Контекст клиентской области: без холста, только масштаб `MM_HIMETRIC` по разрешению окна.
///
/// Используется кодом видов для перевода координат мыши (`DPtoLP`).
class CClientDC : public CDC
{
public:
    /// @brief Создаёт контекст с масштабом `MM_HIMETRIC` (0,01 мм, ось Y вверх) по разрешению хозяина окна.
    /// @param[in] wnd  окно; без хозяина масштаб 1:1
    explicit CClientDC(CWnd* wnd);
};

/// @brief Контекст рисования в `OnPaint`; без холста (рисование идёт через `OnDraw`).
class CPaintDC : public CDC
{
public:
    /// @brief Запоминает окно.
    /// @param[in] wnd  окно
    explicit CPaintDC(CWnd* wnd) : m_wnd(wnd) {}
    /// Окно, для которого создан контекст.
    CWnd* m_wnd;
};

// --- CView --------------------------------------------------------------------
/// @brief Вид документа MFC: рисует документ в `OnDraw` и получает события мыши, таймера, размера.
///
/// Обработчики вызывает виджет-хозяин (`MfcViewWidget`), а не очередь сообщений
/// Windows. Документ назначает оболочка программы (`m_pDocument`).
/// @see https://www.tamic.ru/docs/code/compat.html#doc-view
class CView : public CWnd
{
public:
    /// @brief Создаёт вид без документа.
    CView() : m_pDocument(0) {}

    /// @brief Рисует документ в контексте.
    /// @param[in] pDC  контекст, связанный с `QPainter` (экран или принтер)
    virtual void OnDraw(CDC* pDC) = 0;
    /// @brief Вызывается один раз после привязки документа; базовая реализация пустая.
    virtual void OnInitialUpdate() {}
    /// @brief Настраивает контекст перед `OnDraw`; базовая реализация пустая.
    /// @param[in,out] pDC   контекст
    /// @param[in]     info  сведения о печати или NULL (экран)
    virtual void OnPrepareDC(CDC* pDC, CPrintInfo* info = 0);
    /// @brief Пустая настройка стиля окна; TRUE.
    /// @return Всегда TRUE.
    virtual BOOL PreCreateWindow(CREATESTRUCT&) { return TRUE; }
    /// @brief Пустая подготовка печати; TRUE.
    /// @return Всегда TRUE.
    virtual BOOL OnPreparePrinting(CPrintInfo*) { return TRUE; }
    /// @brief Пустое начало печати.
    virtual void OnBeginPrinting(CDC*, CPrintInfo*) {}
    /// @brief Пустое завершение печати.
    virtual void OnEndPrinting(CDC*, CPrintInfo*) {}
    /// @brief Пустой диалог печати MFC; TRUE (диалог показывает `gui_common/printsupport`).
    /// @return Всегда TRUE.
    BOOL DoPreparePrinting(CPrintInfo*) { return TRUE; }

    // Обработчики, которые вызывает Qt-хозяин при событиях мыши, клавиш и
    // таймера. Виды переопределяют те, что им нужны.
    /// @brief Нажатие левой кнопки мыши. Параметры: флаги `MK_*` и точка в пикселях окна.
    virtual void OnLButtonDown(UINT, CPoint) {}
    /// @brief Отпускание левой кнопки мыши (флаги `MK_*`, точка в пикселях).
    virtual void OnLButtonUp(UINT, CPoint) {}
    /// @brief Двойной щелчок левой кнопкой (флаги `MK_*`, точка в пикселях).
    virtual void OnLButtonDblClk(UINT, CPoint) {}
    /// @brief Нажатие правой кнопки (флаги `MK_*`, точка в пикселях).
    virtual void OnRButtonDown(UINT, CPoint) {}
    /// @brief Движение мыши (флаги `MK_*`, точка в пикселях).
    virtual void OnMouseMove(UINT, CPoint) {}
    /// Таймер. Тип UINT_PTR — как в подписях Windows-версии: иначе метод вида
    /// не переопределяет этот, и обработчик никогда не вызывается.
    /// Параметр — номер сработавшего таймера.
    virtual void OnTimer(UINT_PTR) {}
    /// Изменение размера окна: в MFC приходило сообщением, здесь зовёт хозяин.
    /// Параметры: тип изменения, новая ширина и высота клиентской области, пиксели.
    virtual void OnSize(UINT /*type*/, int /*cx*/, int /*cy*/) {}
    /// Закрытие окна вида. В MFC приходило сообщением WM_CLOSE; здесь зовёт
    /// Qt-слой, когда пользователь закрывает окно документа.
    virtual void OnClose() {}

    /// Документ вида (не владеет); назначает оболочка программы.
    CDocument* m_pDocument;
};

// --- CScrollView --------------------------------------------------------------
// Прокрутка в логических единицах выбранного режима отображения, как в MFC.
/// @brief Вид с прокруткой: размеры области задаются в логических единицах режима отображения.
///
/// Переводит размеры и позицию прокрутки между логическими единицами и пикселями
/// (для `MM_HIMETRIC` — 0,01 мм, ось Y вверх) и сдвигает начало области вывода
/// контекста на позицию прокрутки в `OnPrepareDC`.
/// @see https://www.tamic.ru/docs/code/compat.html#doc-view
class CScrollView : public CView
{
public:
    /// @brief Создаёт вид в режиме `MM_TEXT` с нулевыми размерами.
    CScrollView();

    /// @brief Задаёт режим отображения и размеры прокручиваемой области.
    /// @param[in] mapMode    режим `MM_*`; пересчёт в пиксели выполняется только для `MM_HIMETRIC`
    /// @param[in] sizeTotal  полный размер содержимого, логические единицы
    /// @param[in] sizePage   шаг страницей, логические единицы
    /// @param[in] sizeLine   шаг строкой, логические единицы
    void SetScrollSizes(int mapMode, SIZE sizeTotal,
                        const SIZE& sizePage = SIZE(),
                        const SIZE& sizeLine = SIZE());
    /// @brief Возвращает позицию прокрутки в пикселях.
    /// @return Позиция, пиксели; (0, 0) — нет хозяина.
    CPoint GetDeviceScrollPosition() const;    // в пикселях
    /// @brief Возвращает полный размер содержимого, логические единицы.
    /// @return Размер, заданный `SetScrollSizes`, логические единицы.
    CSize  GetTotalSize() const { return m_totalLog; }

    /// @brief Устанавливает режим отображения вида и сдвигает начало области вывода на позицию прокрутки.
    /// @param[in,out] pDC   контекст; NULL — ничего
    /// @param[in]     info  сведения о печати (не используются)
    virtual void OnPrepareDC(CDC* pDC, CPrintInfo* info = 0);

    // Базовые обработчики MFC: код видов зовёт их как «поведение по умолчанию».
    /// @brief Пустой обработчик по умолчанию; вид вызывает его как «поведение базового класса».
    virtual void OnLButtonDown(UINT, CPoint) {}
    /// @brief Пустой обработчик по умолчанию.
    virtual void OnLButtonUp(UINT, CPoint) {}
    /// @brief Пустой обработчик по умолчанию.
    virtual void OnLButtonDblClk(UINT, CPoint) {}
    /// @brief Пустой обработчик по умолчанию.
    virtual void OnRButtonDown(UINT, CPoint) {}
    /// @brief Пустой обработчик по умолчанию.
    virtual void OnMouseMove(UINT, CPoint) {}
    /// @brief Пустой обработчик таймера по умолчанию.
    virtual void OnTimer(UINT_PTR) {}
    /// @brief Пустой обработчик изменения размера по умолчанию.
    virtual void OnSize(UINT, int, int) {}

    // Команды печати каркаса MFC. Печать в Qt-версии делает сам слой интерфейса,
    // поэтому здесь только точки, на которые ссылается карта сообщений.
    /// @brief Пустая команда печати каркаса (печать выполняет `gui_common/printsupport`).
    virtual void OnFilePrint() {}
    /// @brief Пустая команда предпросмотра печати.
    virtual void OnFilePrintPreview() {}

    /// @brief Возвращает режим отображения, заданный `SetScrollSizes`.
    /// @return Режим `MM_*`.
    int GetMapModeCompat() const { return m_mapMode; }

protected:
    /// Режим отображения `MM_*`.
    int   m_mapMode;
    /// Полный размер содержимого, логические единицы.
    CSize m_totalLog;
    /// Шаг прокрутки страницей, логические единицы.
    CSize m_pageLog;
    /// Шаг прокрутки строкой, логические единицы.
    CSize m_lineLog;
};

#endif // !_WIN32
