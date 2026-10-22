// mfc_dialog.h — диалоги, рамки и карты сообщений MFC поверх Qt.
//
// Диалоги TMC описаны двумя частями: разметка лежит в ресурсах `.rc`, а
// поведение — в коде класса (обмен данными DDX, обработчики кнопок). Обе части
// сохраняются как есть:
//   * разметка переводится в `.ui` скриптом rc2ui — те же элементы,
//     те же подписи, то же расположение;
//   * код класса компилируется из Windows-версии поверх этого слоя.
//
// Карты сообщений MFC (BEGIN_MESSAGE_MAP / ON_BN_CLICKED) здесь не пустые:
// макросы собирают таблицу «идентификатор команды -> обработчик», и слой
// подключает к ней сигналы Qt-виджетов. Поэтому кнопки работают тем же кодом,
// что и на Windows.

/// @file mfc_dialog.h
/// @brief Диалоги, элементы управления, обмен данными DDX/DDV, карты сообщений, страницы свойств и стандартные диалоги MFC поверх Qt.
///
/// Часть модуля «Слой совместимости». Разметка диалога берётся из `.ui`,
/// полученного из `.rc` скриптом rc2ui, и регистрируется в
/// `MfcDialogRegistry`; поведение — из класса диалога Windows-версии без правок.
/// Реализация — `mfc_dialog.cpp`.
/// @see https://www.tamic.ru/docs/code/compat.html#rc-dialogs
#pragma once

#if !defined(_WIN32) || defined(TMC_QT_WIN)

#include "mfc_view.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

// --- Типы Windows, встречающиеся в объявлениях -------------------------------
// WPARAM, LPARAM, LRESULT и HWND объявлены в tmc_mfc.h: ими пользуется уже
// слой окон (CWnd::SendMessage).
#ifndef _WIN32 // Windows API (под Windows — настоящий windows.h)
/// Описатели кисти, курсора и значка (метки; только вне Windows).
typedef void*          HBRUSH;
typedef void*          HCURSOR;
typedef void*          HICON;
/// Указатель на параметры создания окна.
typedef CREATESTRUCT*  LPCREATESTRUCT;

/// Коды ответов окна сообщения и результата диалога.
#define IDOK      1
#define IDCANCEL  2
#define IDABORT   3
#define IDRETRY   4
#define IDIGNORE  5
#define IDYES     6
#define IDNO      7

/// Флаги окна сообщения: набор кнопок (младшие 4 бита) и значок (биты 4–7).
#define MB_OK                0x0000
#define MB_OKCANCEL          0x0001
#define MB_YESNO             0x0004
#define MB_ICONERROR         0x0010
#define MB_ICONQUESTION      0x0020
#define MB_ICONEXCLAMATION   0x0030
#define MB_ICONINFORMATION   0x0040

/// Флаги диалога файлов Windows (слоем не используются).
#define OFN_HIDEREADONLY     0x00000004
#define OFN_OVERWRITEPROMPT  0x00000002
#define OFN_FILEMUSTEXIST    0x00001000
#define OFN_ENABLESIZING     0x00800000
#define OFN_PATHMUSTEXIST    0x00000800

/// Флаги диалогов шрифта и цвета Windows (слоем не используются).
#define CF_SCREENFONTS       0x00000001
#define CF_PRINTERFONTS      0x00000002
#define CF_EFFECTS           0x00000100
#define CF_INITTOLOGFONTSTRUCT 0x00000040
#define CC_RGBINIT           0x00000001
#define CC_FULLOPEN          0x00000002

// Структуры общих диалогов Windows. Нужны потому, что код настраивает их поля
// напрямую (цвет текста в диалоге шрифта, начальный цвет в диалоге цвета).
/// @brief Параметры диалога файлов (подмножество полей Windows); код настраивает их, слой не читает.
struct OPENFILENAME
{
    DWORD       lStructSize;
    const char* lpstrTitle;
    const char* lpstrInitialDir;
    const char* lpstrFilter;
    char*       lpstrFile;
    DWORD       nMaxFile;
    DWORD       Flags;
};

/// @brief Параметры диалога шрифта (подмножество полей Windows); `rgbColors` — цвет текста.
struct CHOOSEFONT
{
    DWORD    lStructSize;
    LOGFONT* lpLogFont;
    COLORREF rgbColors;
    DWORD    Flags;
    int      iPointSize;
};

/// @brief Параметры диалога цвета (подмножество полей Windows).
struct CHOOSECOLOR
{
    DWORD     lStructSize;
    COLORREF  rgbResult;
    COLORREF* lpCustColors;
    DWORD     Flags;
};

// Макрос строковых литералов MSVC: в однобайтовой сборке он ничего не делал.
/// Макросы строковых литералов MSVC; в однобайтовой сборке возвращают аргумент.
#ifndef _T
  #define _T(x) x
#endif
#ifndef TEXT
  #define TEXT(x) x
#endif

// Сообщение «закройся»: в Windows его слал счётный узел рамке окна, чтобы
// программа завершилась, досчитав последний шаг в пакетном режиме.
/// Коды оконных сообщений «закрыть» и «разрушено».
#define WM_CLOSE   0x0010
#define WM_DESTROY 0x0002

/// Состояния флажка: снят / установлен.
#define BST_UNCHECKED 0
#define BST_CHECKED   1

// Положение окна. Qt-слой окнами управляет сам, но код Windows-версии эту
// структуру заполняет и читает, поэтому она должна существовать.
/// @brief Положение окна (поля как в Windows); слой только хранит значение.
struct WINDOWPLACEMENT
{
    unsigned int length;
    unsigned int flags;
    unsigned int showCmd;
    struct { long x, y; } ptMinPosition;
    struct { long x, y; } ptMaxPosition;
    struct { long left, top, right, bottom; } rcNormalPosition;
};
#endif // !_WIN32 (Windows API)

// --- Карта сообщений ----------------------------------------------------------
// Три таблицы, как три вида записей в карте сообщений MFC:
//   * команда -> обработчик (ON_COMMAND, ON_BN_CLICKED и родня);
//   * команда -> обработчик обновления состояния (ON_UPDATE_COMMAND_UI):
//     доступен ли пункт и нажат ли он;
//   * сообщение -> обработчик (ON_MESSAGE): собственные сообщения программы.
/// @brief Карта сообщений: таблицы «команда -> обработчик», «команда -> обновление состояния», «сообщение -> обработчик».
///
/// Заполняется методом `TmcBuildMessageMap`, который порождают макросы
/// `BEGIN_MESSAGE_MAP`…`END_MESSAGE_MAP` (сначала вызывается карта базового
/// класса, затем добавляются свои записи — свои перекрывают базовые). Строится
/// заново при каждом использовании: при построении диалога, при `SendMessage`,
/// при опросе состояния команд.
/// @see https://www.tamic.ru/docs/code/compat.html#message-map
class MfcMessageMap
{
public:
    /// Обработчик команды.
    typedef std::function<void()> Handler;
    /// Обработчик обновления состояния команды.
    typedef std::function<void(CCmdUI*)> UpdateHandler;
    /// Обработчик оконного сообщения.
    typedef std::function<LRESULT(WPARAM, LPARAM)> MessageHandler;

    /// @brief Регистрирует обработчик команды (заменяет прежний для того же номера).
    /// @param[in] id       номер команды или элемента
    /// @param[in] handler  обработчик
    void add(UINT id, Handler handler) { m_handlers[id] = handler; }
    /// @brief Вызывает обработчик команды.
    /// @param[in] id  номер команды
    /// @return true — обработчик найден и вызван.
    bool call(UINT id) const
    {
        std::map<UINT, Handler>::const_iterator it = m_handlers.find(id);
        if (it == m_handlers.end())
            return false;
        it->second();
        return true;
    }
    /// @brief Возвращает таблицу обработчиков команд.
    /// @return Ссылка на внутреннюю таблицу «номер команды -> обработчик»; живёт, пока жива карта.
    const std::map<UINT, Handler>& handlers() const { return m_handlers; }

    /// Обработчик обновления состояния команды.
    /// @param[in] id       номер команды
    /// @param[in] handler  обработчик
    void addUpdate(UINT id, UpdateHandler handler) { m_updates[id] = handler; }
    /// @brief Вызывает обработчик обновления состояния.
    /// @param[in]     id      номер команды
    /// @param[in,out] pCmdUI  состояние команды, которое заполняет обработчик
    /// @return true — обработчик найден и вызван.
    bool callUpdate(UINT id, CCmdUI* pCmdUI) const
    {
        std::map<UINT, UpdateHandler>::const_iterator it = m_updates.find(id);
        if (it == m_updates.end())
            return false;
        it->second(pCmdUI);
        return true;
    }
    /// @brief Возвращает таблицу обработчиков обновления состояния.
    /// @return Ссылка на внутреннюю таблицу «номер команды -> обработчик обновления».
    const std::map<UINT, UpdateHandler>& updateHandlers() const { return m_updates; }

    /// Обработчик оконного сообщения.
    /// @param[in] message  код сообщения
    /// @param[in] handler  обработчик
    void addMessage(UINT message, MessageHandler handler) { m_messages[message] = handler; }
    /// @brief Вызывает обработчик оконного сообщения.
    /// @param[in]  message  код сообщения
    /// @param[in]  wParam   первый параметр
    /// @param[in]  lParam   второй параметр
    /// @param[out] result   результат обработчика (может быть NULL)
    /// @return true — обработчик найден и вызван.
    bool callMessage(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* result) const
    {
        std::map<UINT, MessageHandler>::const_iterator it = m_messages.find(message);
        if (it == m_messages.end())
            return false;
        const LRESULT r = it->second(wParam, lParam);
        if (result)
            *result = r;
        return true;
    }
    /// @brief Возвращает таблицу обработчиков сообщений.
    /// @return Ссылка на внутреннюю таблицу «код сообщения -> обработчик».
    const std::map<UINT, MessageHandler>& messageHandlers() const { return m_messages; }

private:
    /// Команда -> обработчик.
    std::map<UINT, Handler>        m_handlers;
    /// Команда -> обработчик обновления состояния.
    std::map<UINT, UpdateHandler>  m_updates;
    /// Сообщение -> обработчик.
    std::map<UINT, MessageHandler> m_messages;
};

// Макросы карты сообщений. В MFC они строили таблицу указателей на методы;
// здесь собирают ту же таблицу из замыканий.
#undef DECLARE_MESSAGE_MAP
#undef BEGIN_MESSAGE_MAP
#undef END_MESSAGE_MAP
#undef ON_COMMAND
#undef ON_UPDATE_COMMAND_UI
#undef ON_WM_TIMER

/// Объявляет в классе метод `TmcBuildMessageMap` (и делает доступ `public`).
#define DECLARE_MESSAGE_MAP() \
    public: virtual void TmcBuildMessageMap(MfcMessageMap& tmcMap);

/// Начинает определение `cls::TmcBuildMessageMap`: сначала добавляются записи базового класса `base`.
#define BEGIN_MESSAGE_MAP(cls, base) \
    void cls::TmcBuildMessageMap(MfcMessageMap& tmcMap) \
    { \
        base::TmcBuildMessageMap(tmcMap);

/// Завершает определение карты сообщений.
#define END_MESSAGE_MAP() \
    }

/// Записи «команда/элемент -> метод без параметров». В диалогах подключаются только к кнопкам (сигнал `clicked`).
#define ON_COMMAND(id, fn)      tmcMap.add(id, [this]{ this->fn(); });
/// Щелчок кнопки -> метод (подключается к сигналу `clicked`).
#define ON_BN_CLICKED(id, fn)   tmcMap.add(id, [this]{ this->fn(); });
/// Щелчок по статическому элементу -> метод (подключается, только если элемент — кнопка).
#define ON_STN_CLICKED(id, fn)  tmcMap.add(id, [this]{ this->fn(); });
/// Изменение поля ввода -> метод. Регистрируется, но к полю не подключается (вызывается, только если элемент — кнопка).
#define ON_EN_CHANGE(id, fn)    tmcMap.add(id, [this]{ this->fn(); });
/// Смена пункта выпадающего списка -> метод. Регистрируется, но к списку не подключается.
#define ON_CBN_SELCHANGE(id, fn) tmcMap.add(id, [this]{ this->fn(); });
/// Смена строки списка -> метод. Регистрируется, но к списку не подключается.
#define ON_LBN_SELCHANGE(id, fn) tmcMap.add(id, [this]{ this->fn(); });
/// Не поддерживается: разворачивается в пустоту.
#define ON_COMMAND_RANGE(first, last, fn)

// Обновление состояния пункта меню или кнопки панели: обработчик получает
// CCmdUI и говорит, доступен ли пункт и нажат ли он. Слой интерфейса зовёт эти
// обработчики перед показом меню (см. src/gui_common/commandupdate.h).
/// Запись «команда -> обработчик состояния `fn(CCmdUI*)`».
#define ON_UPDATE_COMMAND_UI(id, fn) \
    tmcMap.addUpdate(id, [this](CCmdUI* pCmdUI){ this->fn(pCmdUI); });
/// Не поддерживается: разворачивается в пустоту.
#define ON_UPDATE_COMMAND_UI_RANGE(first, last, fn)

// Собственное сообщение программы (WM_USER + N). Обработчик имеет вид
// LRESULT fn(WPARAM, LPARAM), как в MFC.
/// Запись «сообщение -> метод `LRESULT fn(WPARAM, LPARAM)`»; вызывается через `SendMessage`/`PostMessage`.
#define ON_MESSAGE(msg, fn) \
    tmcMap.addMessage(UINT(msg), [this](WPARAM tmcWp, LPARAM tmcLp) -> LRESULT { \
        return this->fn(tmcWp, tmcLp); });

/// Не поддерживается: уведомления `WM_NOTIFY` в слое не формируются.
#define ON_NOTIFY(code, id, fn)

// Оконные события приходят виду и диалогу через виртуальные методы (их зовёт
// Qt-хозяин), поэтому регистрировать здесь нечего.
/// Оконные события: пустые макросы, события приходят через виртуальные методы вида и диалога.
#define ON_WM_PAINT()
/// Пустой: таймер приходит через `OnTimer`.
#define ON_WM_TIMER()
/// Пустой: создание окна делает оболочка.
#define ON_WM_CREATE()
/// Пустой: разрушение — через `OnDestroy`.
#define ON_WM_DESTROY()
/// Пустой: размер — через `OnSize`.
#define ON_WM_SIZE()
/// Пустой: фон заливает виджет-хозяин.
#define ON_WM_ERASEBKGND()
/// Пустой: цвета элементов задаёт тема.
#define ON_WM_CTLCOLOR()
/// Пустой: курсор задаёт Qt.
#define ON_WM_SETCURSOR()
/// Пустой: мышь — через `OnLButtonDown`.
#define ON_WM_LBUTTONDOWN()
/// Пустой: мышь — через `OnLButtonUp`.
#define ON_WM_LBUTTONUP()
/// Пустой: мышь — через `OnLButtonDblClk`.
#define ON_WM_LBUTTONDBLCLK()
/// Пустой: мышь — через `OnRButtonDown`.
#define ON_WM_RBUTTONDOWN()
/// Пустой: отпускание правой кнопки виду не передаётся.
#define ON_WM_RBUTTONUP()
/// Пустой: мышь — через `OnMouseMove`.
#define ON_WM_MOUSEMOVE()
/// Пустой: колёсико виду не передаётся.
#define ON_WM_MOUSEWHEEL()
/// Пустой: клавиши виду не передаются.
#define ON_WM_KEYDOWN()
/// Пустой: прокрутку ведёт виджет-хозяин.
#define ON_WM_HSCROLL()
/// Пустой: прокрутку ведёт виджет-хозяин.
#define ON_WM_VSCROLL()
/// Пустой: закрытие — через `OnClose`.
#define ON_WM_CLOSE()

// Окно сообщения с кнопками. Вариант без типа кнопок объявлен в tmc_mfc.h и
// пишет в поток ошибок; здесь — полноценный, с ответом IDYES/IDNO/IDOK.
/// @brief Показывает модальное окно сообщения с кнопками и ждёт ответа.
///
/// Без графического приложения (консоль, проверки) текст пишется в stderr, а
/// ответ — `IDYES` для `MB_YESNO`, иначе `IDOK`.
/// @param[in] text    текст в CP1251
/// @param[in] type    `MB_OK`, `MB_OKCANCEL`, `MB_YESNO` плюс значок `MB_ICON*`
/// @param[in] helpId  номер справки (не используется)
/// @return `IDOK`, `IDCANCEL`, `IDYES` или `IDNO`.
/// @note Потоки: только из потока интерфейса.
int AfxMessageBox(const char* text, UINT type, UINT helpId = 0);

// --- Обмен данными с элементами (DDX/DDV) ------------------------------------
/// Диалог MFC (определён ниже).
class CDialog;

/// @brief Контекст обмена данными DDX: диалог и направление обмена.
/// @see https://www.tamic.ru/docs/code/compat.html#ddx
class CDataExchange
{
public:
    /// @brief Создаёт контекст обмена.
    /// @param[in] dlg              диалог
    /// @param[in] saveAndValidate  TRUE — элементы -> переменные; FALSE — переменные -> элементы
    CDataExchange(CDialog* dlg, BOOL saveAndValidate)
        : m_pDlgWnd(dlg), m_bSaveAndValidate(saveAndValidate) {}

    /// Диалог, элементы которого участвуют в обмене.
    CDialog* m_pDlgWnd;
    /// Направление: TRUE — читать из элементов, FALSE — записывать в элементы.
    BOOL     m_bSaveAndValidate;
    /// @brief Ничего не делает: проверки DDV не прерывают обмен.
    void     Fail() {}
};

/// @brief Обменивает текст элемента со строкой (перекодировка CP1251 <-> Unicode).
/// @param[in]     pDX    контекст обмена
/// @param[in]     id     номер элемента (`IDC_*`)
/// @param[in,out] value  строка в CP1251
void DDX_Text(CDataExchange* pDX, int id, CString& value);
/// @brief Обменивает текст элемента с буфером `char`.
/// @param[in]     pDX     контекст обмена
/// @param[in]     id      номер элемента
/// @param[in,out] value   буфер в CP1251
/// @param[in]     maxLen  размер буфера, байт (с завершающим нулём)
void DDX_Text(CDataExchange* pDX, int id, char* value, int maxLen);
/// @brief Обменивает текст элемента с целым (`%d`; нечисловой текст читается как 0).
/// @param[in]     pDX    контекст обмена
/// @param[in]     id     номер элемента
/// @param[in,out] value  значение
void DDX_Text(CDataExchange* pDX, int id, int& value);
/// @brief То же для `UINT` (через `int`).
/// @param[in]     pDX    контекст обмена
/// @param[in]     id     номер элемента
/// @param[in,out] value  значение
void DDX_Text(CDataExchange* pDX, int id, UINT& value);
/// @brief То же для `long` (через `int`).
/// @param[in]     pDX    контекст обмена
/// @param[in]     id     номер элемента
/// @param[in,out] value  значение
void DDX_Text(CDataExchange* pDX, int id, long& value);
/// @brief То же для `float` (через `double`).
/// @param[in]     pDX    контекст обмена
/// @param[in]     id     номер элемента
/// @param[in,out] value  значение
void DDX_Text(CDataExchange* pDX, int id, float& value);
/// @brief Обменивает текст элемента с вещественным (запись по `%g`; нечисловой текст читается как 0).
/// @param[in]     pDX    контекст обмена
/// @param[in]     id     номер элемента
/// @param[in,out] value  значение
void DDX_Text(CDataExchange* pDX, int id, double& value);
// BOOL в этом слое — это int, поэтому одной перегрузки достаточно.
/// @brief Обменивает состояние флажка с целым (0/1).
/// @param[in]     pDX    контекст обмена
/// @param[in]     id     номер флажка
/// @param[in,out] value  0 — снят, 1 — установлен
void DDX_Check(CDataExchange* pDX, int id, int& value);
/// @brief Обменивает выбранный переключатель группы с его номером в группе.
///
/// Группа берётся из `QButtonGroup` разметки, порядок — из цепочки фокуса
/// (порядок записи в `.rc`). Если группы нет — перебираются элементы с номерами
/// `id`, `id+1`, … пока они существуют (не более 66).
/// @param[in]     pDX    контекст обмена
/// @param[in]     id     номер первого переключателя группы
/// @param[in,out] value  номер выбранного (с 0); при чтении -1 — ничего не выбрано
/// @see https://www.tamic.ru/docs/code/compat.html#ddx
void DDX_Radio(CDataExchange* pDX, int id, int& value);

// Проверки. В MFC они показывали окно с сообщением; здесь просто ограничивают
// значение, чтобы не мешать работе диалога всплывающими окнами на каждое поле.
/// @brief Обрезает строку до `maxChars` байт при чтении из элементов.
/// @param[in]     pDX       контекст обмена
/// @param[in,out] value     строка
/// @param[in]     maxChars  наибольшая длина, байт
/// @warning Обрезка выполняется на месте в буфере `CString`; копии строки, разделяющие буфер, тоже меняются.
void DDV_MaxChars(CDataExchange* pDX, CString& value, int maxChars);
/// @brief Обрезает буфер до `maxChars` байт при чтении из элементов.
/// @param[in]     pDX       контекст обмена
/// @param[in,out] value     буфер
/// @param[in]     maxChars  наибольшая длина, байт
void DDV_MaxChars(CDataExchange* pDX, char* value, int maxChars);
/// @brief Проверка диапазона целого: ничего не делает (значение не проверяется и не ограничивается).
/// @param[in] pDX     контекст обмена (не используется)
/// @param[in] value   проверяемое значение (не используется)
/// @param[in] minVal  нижняя граница (не используется)
/// @param[in] maxVal  верхняя граница (не используется)
void DDV_MinMaxInt(CDataExchange* pDX, int value, int minVal, int maxVal);
/// @brief Проверка диапазона `double`: ничего не делает.
/// @param[in] pDX     контекст обмена (не используется)
/// @param[in] value   проверяемое значение (не используется)
/// @param[in] minVal  нижняя граница (не используется)
/// @param[in] maxVal  верхняя граница (не используется)
void DDV_MinMaxDouble(CDataExchange* pDX, double value, double minVal, double maxVal);

// --- Обёртки элементов управления --------------------------------------------
// Возвращаются из GetDlgItem. Внутри — указатель на виджет Qt.
/// @brief Обёртка элемента управления диалога вокруг виджета Qt.
///
/// Возвращается `CDialog::GetDlgItem` (принадлежит диалогу, кэшируется) или
/// привязывается к переменной через `DDX_Control`. Тексты передаются в CP1251.
/// @see https://www.tamic.ru/docs/code/compat.html#rc-dialogs
class CWndCtrl : public CWnd
{
public:
    /// @brief Создаёт обёртку.
    /// @param[in] widget  виджет Qt (`QWidget*`) или NULL — элемента нет, вызовы ничего не делают
    explicit CWndCtrl(void* widget = 0) : m_widget(widget) {}

    /// @brief Задаёт текст элемента.
    ///
    /// В Windows-сборке текст подписей и недоступных/только-для-чтения полей
    /// переводится словарём `tmci18n`; поля ввода с данными не переводятся.
    /// @param[in] text  текст в CP1251
    void  SetWindowText(const char* text);
    /// @brief Копирует текст элемента в буфер (CP1251; символы вне CP1251 -> `?`).
    /// @param[out] buffer  буфер
    /// @param[in]  maxLen  размер буфера, байт
    /// @return Число скопированных байт без завершающего нуля.
    int   GetWindowText(char* buffer, int maxLen) const;
    /// @brief Делает элемент доступным или недоступным.
    /// @param[in] enable  TRUE — доступен
    void  EnableWindow(BOOL enable = TRUE);
    /// @brief Переводит фокус ввода на элемент.
    void  SetFocus() override;
    /// @brief Задаёт шрифт элемента по `LOGFONT` (Courier* -> JetBrains Mono; высота — пиксели).
    /// @param[in] font    шрифт; NULL — без изменений
    /// @param[in] redraw  не используется
    void  SetFont(CFont* font, BOOL redraw = TRUE) override;
    /// @brief Возвращает виджет Qt (`QWidget*`) или NULL.
    /// @return `QWidget*` элемента (не владеет) или NULL — элемента нет.
    void* widget() const { return m_widget; }
    /// Привязать обёртку к виджету (используется в DDX_Control).
    /// @param[in] w  виджет Qt или NULL
    void  attach(void* w) { m_widget = w; }

    /// @brief Возвращает контекст для рисования поверх элемента (рисование идёт в картинку).
    /// @return Новый контекст; освобождать `ReleaseDC`. NULL — элемента нет.
    CDC* GetDC() override;
    /// @brief Завершает рисование, показывает картинку в элементе-подписи и удаляет контекст.
    /// @param[in] dc  контекст из `GetDC`
    void ReleaseDC(CDC* dc) override;

protected:
    /// Виджет Qt (`QWidget*`, не владеет).
    void* m_widget;
};

/// Подпись и поле ввода — обычные обёртки элемента.
typedef CWndCtrl CStatic;
/// Поле ввода — обычная обёртка элемента.
typedef CWndCtrl CEdit;

/// @brief Кнопка, флажок или переключатель.
class CButton : public CWndCtrl
{
public:
    /// @brief Создаёт обёртку кнопки.
    /// @param[in] widget  `QAbstractButton*` или NULL
    explicit CButton(void* widget = 0) : CWndCtrl(widget) {}
};

// Индикатор выполнения. В TMC_DN показывает ход расчёта диаграммы.
/// @brief Индикатор выполнения поверх `QProgressBar`.
class CProgressCtrl : public CWndCtrl
{
public:
    /// @brief Создаёт обёртку индикатора.
    /// @param[in] widget  `QProgressBar*` или NULL
    explicit CProgressCtrl(void* widget = 0) : CWndCtrl(widget) {}
    /// @brief Задаёт текущее значение.
    /// @param[in] pos  значение
    /// @return Прежнее значение; 0 — элемента нет.
    int  SetPos(int pos);

private:
    /// Шаг `StepIt`.
    int m_step = 1;      // шаг для StepIt, как в MFC
};

/// @brief Выпадающий список поверх `QComboBox`.
class CComboBox : public CWndCtrl
{
public:
    /// @brief Создаёт обёртку.
    /// @param[in] widget  `QComboBox*` или NULL
    explicit CComboBox(void* widget = 0) : CWndCtrl(widget) {}
};

/// @brief Список строк поверх `QListWidget`.
class CListBox : public CWndCtrl
{
public:
    /// @brief Создаёт обёртку.
    /// @param[in] widget  `QListWidget*` или NULL
    explicit CListBox(void* widget = 0) : CWndCtrl(widget) {}
};

// --- Набор картинок (CImageList) ----------------------------------------------
// В Windows это общий склад значков: список выдаёт из него картинку по номеру.
// Здесь значок — QIcon, а номер — место в списке. Пользуется им окно свойств
// FieldView (tmcdialogpropet.h, m_ImageList1).
/// Флаги создания набора картинок (в слое не используются).
#define ILC_MASK   0x0001
/// Набор картинок: цвет по умолчанию.
#define ILC_COLOR  0x0000
/// Набор картинок: 4 бита на пиксель.
#define ILC_COLOR4 0x0004
/// Набор картинок: 8 бит на пиксель.
#define ILC_COLOR8 0x0008
/// Набор картинок: 24 бита на пиксель.
#define ILC_COLOR24 0x0018
/// Набор картинок: 32 бита на пиксель.
#define ILC_COLOR32 0x0020

/// @brief Набор значков (`QList<QIcon>`): номер картинки — позиция в наборе.
class CImageList
{
public:
    /// @brief Создаёт пустой набор (без списка).
    CImageList();
    /// @brief Удаляет список значков.
    ~CImageList();





    /// @brief Возвращает указатель на внутренний список (`QList<QIcon>*`) или NULL.
    /// @return `QList<QIcon>*` (принадлежит набору) или NULL — набор не создан.
    void* GetSafeHandle() const { return m_icons; }

private:
    /// Копирование запрещено (объявлено без определения).
    CImageList(const CImageList&);              // копирование не нужно
    /// @brief Копирующее присваивание запрещено (объявлено без определения).
    /// @return Не определено: вызов не компонуется.
    CImageList& operator=(const CImageList&);

    /// Список значков `QList<QIcon>*` (владеет).
    void* m_icons;   // QList<QIcon>*
    /// Ширина картинки, пиксели.
    int   m_cx;
    /// Высота картинки, пиксели.
    int   m_cy;
};

// --- Список с колонками (CListCtrl) -------------------------------------------
// В Windows это SysListView32 в режиме отчёта: колонки и строки. В Qt тому же
// соответствует QTreeWidget (список с колонками), его и оборачиваем.
/// Тип набора картинок списка (в слое не различается).
#define LVSIL_NORMAL 0
/// Набор мелких значков списка.
#define LVSIL_SMALL  1
/// Набор значков состояния списка.
#define LVSIL_STATE  2

/// Выравнивание колонки списка.
#define LVCFMT_LEFT   0
/// Выравнивание колонки по правому краю.
#define LVCFMT_RIGHT  1
/// Выравнивание колонки по центру.
#define LVCFMT_CENTER 2

/// Дополнительные стили списка: выделение строкой целиком, линии сетки.
#define LVS_EX_FULLROWSELECT 0x00000020
/// Линии сетки между строками и колонками.
#define LVS_EX_GRIDLINES     0x00000001

/// @brief Список с колонками поверх `QTreeWidget`: строка — элемент верхнего уровня, колонка — колонка дерева.
class CListCtrl : public CWndCtrl
{
public:
    /// @brief Создаёт обёртку.
    /// @param[in] widget  `QTreeWidget*` или NULL
    explicit CListCtrl(void* widget = 0) : CWndCtrl(widget), m_images(0), m_exStyle(0) {}







    /// @brief Возвращает дополнительные стили, заданные `SetExtendedStyle`.
    /// @return Набор флагов `LVS_EX_*`; 0 — не задавались.
    DWORD GetExtendedStyle() const { return m_exStyle; }

private:
    /// Набор значков строк (не владеет).
    CImageList* m_images;
    /// Дополнительные стили `LVS_EX_*`.
    DWORD       m_exStyle;
};

// Привязка переменной-обёртки к элементу (в MFC — DDX_Control). Объявлена
// после самих обёрток: раньше типа ещё не существует.
/// @brief Привязывает обёртку элемента к виджету диалога.
/// @param[in]     pDX     контекст обмена
/// @param[in]     id      номер элемента
/// @param[in,out] widget  обёртка, получающая виджет
void DDX_Control(CDataExchange* pDX, int id, CWndCtrl& widget);

// --- Диалог -------------------------------------------------------------------
// Разметку берём из .ui, полученного из того же .rc, что и на Windows.
/// @brief Диалог MFC поверх `QDialog`: разметка из реестра, кнопки и карта сообщений, DDX, таймеры.
///
/// Объект создаёт код программы (обычно на стеке). `DoModal` строит модальное
/// окно на время вызова; `Create` — немодальное окно, живущее до `DestroyWindow`
/// или удаления объекта. Окно Qt (`m_qtDialog`) существует только пока диалог
/// показан. Только поток интерфейса.
/// @see https://www.tamic.ru/docs/code/compat.html#rc-dialogs
class CDialog : public CWnd
{
public:
    /// @brief Создаёт объект диалога без окна.
    /// @param[in] idd     номер ресурса диалога (`IDD_*`)
    /// @param[in] parent  окно-родитель (запоминается)
    explicit CDialog(UINT idd = 0, CWnd* parent = 0);
    /// @brief Разрушает немодальное окно, если оно ещё открыто, и кэш обёрток элементов.
    virtual ~CDialog();

    /// @brief Показывает модальное окно и ждёт закрытия.
    ///
    /// Строит окно (`TmcBuildIn`), вызывает оформитель `tmc_dialog_decorator()`,
    /// выполняет `exec()`. Родитель — главное окно программы.
    /// @return `IDOK` — окно закрыто через `EndDialog(IDOK)`; иначе `IDCANCEL`
    ///         (в том числе если разметка диалога не зарегистрирована).
    virtual INT_PTR DoModal();

    /// Немодальное окно, как `CDialog::Create` в MFC: окно открывается и
    /// остаётся на экране, а программа продолжает работать. Так открыто окно
    /// уровней цвета FieldView (colorleveldlg.cpp) — при нём можно вращать
    /// сцену. Окно живёт до DestroyWindow.
    /// @param[in] idd     номер ресурса; 0 — прежний
    /// @param[in] parent  окно-родитель; NULL — прежний
    /// @return TRUE — окно открыто (или уже было открыто); FALSE — разметка не зарегистрирована.
    virtual BOOL Create(UINT idd, CWnd* parent = 0);
    /// @brief То же, что `Create(0, parent)`; имя шаблона не используется.
    /// @param[in] templateName  имя шаблона (не используется)
    /// @param[in] parent        окно-родитель
    /// @return См. `Create(UINT, CWnd*)`.
    virtual BOOL Create(const char* templateName, CWnd* parent = 0);

    /// Закрытие окна пользователем (крестик). В MFC приходило сообщением
    /// WM_CLOSE; здесь зовётся из обработчика закрытия окна Qt. По умолчанию,
    /// как в MFC, равносильно отмене.
    virtual void OnClose();

    /// Разрушить окно. Немодальное окно после этого «не существует»:
    /// GetSafeHwnd() снова возвращает ноль, и код Windows-версии создаёт его
    /// заново (fldviewview.cpp, OnFieldL).
    /// @return TRUE.
    BOOL DestroyWindow() override;

    /// @brief Возвращает окно Qt (`QDialog*`) или NULL, если окна нет.
    /// @return `QDialog*` окна диалога (не владеет вызывающий); NULL — окно не открыто.
    HWND GetSafeHwnd() const override { return static_cast<HWND>(m_qtDialog); }

    /// @brief Вызывается после построения разметки, до первого `UpdateData(FALSE)`.
    /// @return TRUE.
    virtual BOOL OnInitDialog();
    /// @brief Обменивает данные переменных и элементов (переопределяется вызовами `DDX_*`).
    /// @param[in] pDX  контекст обмена
    virtual void DoDataExchange(CDataExchange* pDX);
    /// @brief Кнопка OK: `UpdateData(TRUE)` и `EndDialog(IDOK)`.
    virtual void OnOK();
    /// @brief Кнопка «Отмена», клавиша Esc, по умолчанию и крестик: `EndDialog(IDCANCEL)`.
    virtual void OnCancel();
    /// Рисование поверх диалога. В Windows приходило сообщением WM_PAINT;
    /// диалоги TMC так рисуют образцы цвета и линий прямо в статических полях
    /// (GetDlgItem()->GetDC() -> FillSolidRect). Зовёт Qt-слой при перерисовке.
    virtual void OnPaint() {}
    // Уведомления от элементов (в MFC — WM_NOTIFY). В Qt-версии не приходят,
    // но метод должен существовать: код диалогов его переопределяет.
    /// @brief Уведомление элемента; слоем не вызывается. Параметры: `wParam`, `lParam`, указатель на результат.
    /// @return FALSE.
    virtual BOOL OnNotify(WPARAM, LPARAM, LRESULT*) { return FALSE; }

    /// @brief Вызывает `DoDataExchange` в заданном направлении.
    /// @param[in] saveAndValidate  TRUE — элементы -> переменные; FALSE — переменные -> элементы
    /// @return Всегда TRUE (проверки DDV не прерывают обмен).
    /// @see https://www.tamic.ru/docs/code/compat.html#ddx
    BOOL UpdateData(BOOL saveAndValidate = TRUE);
    /// @brief Закрывает окно с результатом.
    /// @param[in] result  `IDOK` -> `Accepted`, иначе `Rejected`
    void EndDialog(int result);

    /// @brief Возвращает обёртку элемента по номеру (поиск виджета по `objectName` = имя `IDC_*`).
    /// @param[in] id  номер элемента
    /// @return Обёртка (принадлежит диалогу, не NULL); если элемента нет — обёртка без виджета.
    CWndCtrl* GetDlgItem(UINT id) const;
    /// @brief Задаёт текст элемента (CP1251; правила перевода — как у `CWndCtrl::SetWindowText`).
    /// @param[in] id    номер элемента
    /// @param[in] text  текст
    void      SetDlgItemText(UINT id, const char* text);
    /// @brief Копирует текст элемента в буфер (CP1251).
    /// @param[in]  id      номер элемента
    /// @param[out] buffer  буфер
    /// @param[in]  maxLen  размер буфера, байт
    /// @return Число скопированных байт.
    int       GetDlgItemText(UINT id, char* buffer, int maxLen) const;
    /// @brief Задаёт заголовок окна диалога.
    /// @param[in] text  заголовок в CP1251
    void      SetWindowText(const char* text);

    /// @brief Пустая карта сообщений базового диалога.
    /// @param[in,out] tmcMap  таблица (не меняется)
    virtual void TmcBuildMessageMap(MfcMessageMap& tmcMap) { (void)tmcMap; }

    // Таймер диалога. В TMC им обновляется окно прогресса расчёта: каждые
    // 100 мс диалог опрашивает счётчик и двигает индикатор. У диалога нет
    // виджета-хозяина, поэтому таймер живёт прямо здесь, на самом окне Qt.
    /// @brief Запускает таймер на окне Qt диалога; по срабатыванию вызывается `OnTimer(id)`.
    /// @param[in] id        номер таймера (повторный запуск того же номера перезапускает таймер)
    /// @param[in] elapseMs  период, мс
    /// @param[in] proc      функция обратного вызова (не поддерживается)
    /// @return `id`; 0 — окна ещё нет.
    /// @see https://www.tamic.ru/docs/code/compat.html#timers
    UINT SetTimer(UINT id, UINT elapseMs, void* proc = 0);
    /// @brief Останавливает таймер диалога.
    /// @param[in] id  номер таймера
    /// @return TRUE — таймер был запущен.
    BOOL KillTimer(UINT id);
    /// @brief Срабатывание таймера диалога; базовая реализация пустая.
    /// @param[in] id  номер таймера
    virtual void OnTimer(UINT_PTR id) { (void)id; }

    /// Номер ресурса диалога (`IDD_*`), ключ в `MfcDialogRegistry`.
    UINT m_nIDD;        // идентификатор ресурса диалога

    // --- Служебное для набора страниц свойств --------------------------------
    // Страница свойств живёт не своим окном, а вкладкой внутри чужого. Поэтому
    // набор создаёт окно сам и просит страницу построиться в нём.
    /// Построить разметку и подключить обработчики в готовом окне Qt (QDialog*).
    /// @param[in] qDialog  окно `QDialog*`; NULL — ничего
    void TmcBuildIn(void* qDialog);
    /// Забыть окно Qt, не разрушая его (окно разрушит тот, кто его создал).
    void TmcDetach();

protected:
    /// Окно Qt (`QDialog*`) или NULL, пока диалог не показан.
    void* m_qtDialog;   // QDialog*: создаётся в DoModal/Create либо даётся извне
    /// TRUE — окно создано `Create` и разрушается этим объектом.
    BOOL  m_qtDialogOwned = FALSE;    // окно создано нами и нам его разрушать
    /// Таймеры диалога: номер -> `QTimer*` (дочерние объекты окна).
    std::map<UINT, void*> m_timers;   // идентификатор -> QTimer*
    /// Окно-родитель, переданное в конструктор или `Create`.
    CWnd* m_parent;
    /// Кэш обёрток `GetDlgItem` (владеет).
    mutable std::map<UINT, CWndCtrl*> m_ctrlCache;
};

// --- Страница свойств ---------------------------------------------------------
// Окно «Parameters» FieldView (View -> Parameters, Alt+P) в Windows состоит из
// четырёх закладок: Place, Topology, Eps, Field. Каждая закладка — обычный
// диалог со своей разметкой из ресурсов, поэтому страница здесь наследуется от
// CDialog: обмен данными (DDX) и обработчики кнопок работают без правок.
/// @brief Страница набора свойств: диалог, который строится во вкладке окна `CPropertySheet`.
class CPropertyPage : public CDialog
{
public:
    /// @brief Создаёт страницу.
    /// @param[in] nIDTemplate  номер ресурса диалога страницы
    /// @param[in] nIDCaption   номер строки подписи (запоминается, не используется)
    explicit CPropertyPage(UINT nIDTemplate = 0, UINT nIDCaption = 0)
        : CDialog(nIDTemplate, 0), m_nCaptionID(nIDCaption) {}

    /// Пометить страницу изменённой. В MFC включает кнопку «Применить»; здесь
    /// набор включает её так же.
    /// @param[in] changed  TRUE — изменена
    void SetModified(BOOL changed = TRUE);
    /// @brief Возвращает признак «страница изменена».
    /// @return TRUE — страница помечена изменённой (`SetModified`); FALSE — нет.
    BOOL IsModified() const { return m_bModified; }

    /// Страницу показали / убрали. В MFC возврат FALSE у OnKillActive не даёт
    /// уйти со страницы (проверка введённых значений).
    /// @return TRUE.
    virtual BOOL OnSetActive() { return TRUE; }
    /// @return TRUE (возврат FALSE в слое не запрещает переход).
    virtual BOOL OnKillActive() { return TRUE; }

    /// Нажата кнопка «Применить» (и «OK»). Страницы FieldView отсюда шлют виду
    /// сообщение WM_USERAPPLY_*, по которому он забирает новые значения.
    /// @return TRUE.
    virtual BOOL OnApply() { return TRUE; }
    /// @brief Отмена изменений страницы; базовая реализация пустая (слоем не вызывается).
    virtual void OnReset() {}
    /// @brief Команда страницы; базовая реализация возвращает TRUE (слоем не вызывается). Параметры не используются.
    /// @return Всегда TRUE.
    virtual BOOL OnCommand(WPARAM /*wParam*/, LPARAM /*lParam*/) { return TRUE; }

    /// Номер строки ресурса с подписью закладки (задаётся конструктором; при построении набора слоем не читается).
    UINT    m_nCaptionID;   // подпись закладки из строковой таблицы
    /// Подпись закладки в CP1251; используется, если у разметки страницы нет заголовка.
    CString m_strCaption;   // подпись закладки строкой

private:
    friend class CPropertySheet;
    /// Признак «страница изменена».
    BOOL m_bModified = FALSE;
    /// Набор, в который добавлена страница (не владеет).
    class CPropertySheet* m_sheet = 0;
};

// --- Набор страниц свойств ----------------------------------------------------
// Окно с закладками: сверху корешки, снизу кнопки OK, Cancel, Apply — как в
// Windows. В Qt закладки даёт QTabWidget, а каждая страница остаётся своим
// диалогом и просто становится содержимым вкладки.
/// @brief Набор страниц свойств: окно `QDialog` с `QTabWidget` и кнопками OK / Cancel / Apply.
///
/// Не владеет страницами. Окно существует только во время `DoModal`.
/// @see https://www.tamic.ru/docs/code/compat.html#property-sheet
class CPropertySheet : public CWnd
{
public:
    /// @brief Создаёт набор без заголовка.
    /// @param[in] nIDCaption   номер строки заголовка (не используется)
    /// @param[in] parent       окно-родитель (не используется)
    /// @param[in] iSelectPage  номер страницы, открываемой первой
    explicit CPropertySheet(UINT nIDCaption = 0, CWnd* parent = 0, UINT iSelectPage = 0);
    /// @brief Создаёт набор с заголовком.
    /// @param[in] pszCaption   заголовок окна в CP1251
    /// @param[in] parent       окно-родитель (не используется)
    /// @param[in] iSelectPage  номер страницы, открываемой первой
    explicit CPropertySheet(const char* pszCaption, CWnd* parent = 0, UINT iSelectPage = 0);
    /// @brief Деструктор (страницы не удаляет).
    virtual ~CPropertySheet();

    /// @brief Добавляет страницу в конец.
    /// @param[in] page  страница (не владеет); NULL — ничего
    void AddPage(CPropertyPage* page);

    /// @brief Возвращает число страниц.
    /// @return Число добавленных страниц.
    int  GetPageCount() const { return int(m_pages.size()); }
    /// @brief Возвращает страницу по номеру или NULL.
    /// @param[in] index  номер страницы
    /// @return Страница (не владеет набор); NULL — номер вне диапазона.
    CPropertyPage* GetPage(int index) const;
    /// @brief Возвращает текущую страницу или NULL.
    /// @return Текущая страница; NULL — страниц нет или номер текущей вне диапазона.
    CPropertyPage* GetActivePage() const;
    /// @brief Возвращает номер текущей страницы.
    /// @return Номер текущей страницы (с 0).
    int  GetActiveIndex() const { return m_active; }

    /// Показать окно с закладками. Возвращает IDOK или IDCANCEL, как в MFC.
    /// @return `IDOK` — окно закрыто кнопкой OK; `IDCANCEL` — отказ или в наборе нет страниц.
    virtual INT_PTR DoModal();

    /// @brief Вызывается после построения окна набора; базовая реализация возвращает TRUE.
    /// @return Всегда TRUE (возврат слоем не проверяется).
    virtual BOOL OnInitDialog();
    /// @brief Пустой обмен данными набора.
    /// @param[in] pDX  контекст обмена
    virtual void DoDataExchange(CDataExchange* pDX) { (void)pDX; }
    /// @brief Вызывается, если окно набора закрыто без OK; базовая реализация пустая.
    virtual void OnClose() {}

    /// @brief Возвращает заголовок окна набора.
    /// @return Заголовок в CP1251 (принадлежит набору); пустая строка — не задан.
    const char* GetTitle() const { return m_caption.c_str(); }

protected:
    /// Кнопка «Применить»: собрать значения со всех страниц и дать им
    /// применить их (в MFC — OnApply у изменённых страниц).
    void ApplyPages();
    /// Доступность кнопки «Применить» (зовёт CPropertyPage::SetModified).
    void UpdateApplyButton();

    /// Страницы набора (не владеет).
    std::vector<CPropertyPage*> m_pages;
    /// Заголовок окна (CP1251).
    std::string m_caption;
    /// Номер текущей страницы.
    int         m_active;
    /// Окно набора `QDialog*` (только во время `DoModal`).
    void*       m_qtSheet;    // QDialog* окна набора
    /// Вкладки `QTabWidget*`.
    void*       m_qtTabs;     // QTabWidget*
    /// Кнопка «Применить» `QPushButton*`.
    void*       m_qtApply;    // QPushButton* «Применить»

private:
    friend class CPropertyPage;
};

// --- Реестр диалогов ----------------------------------------------------------
// Заполняется кодом, который порождает rc2ui: для каждого диалога —
// функция построения разметки, для каждого элемента — символьное имя по номеру.
/// @brief Реестр разметок диалогов и имён элементов, заполняемый кодом, порождённым rc2ui.
///
/// Каждая программа регистрирует при запуске свои диалоги
/// (`apps/*/dialogs/dialog_registry.cpp`). Таблицы — статические, общие для процесса.
/// @see https://www.tamic.ru/docs/code/compat.html#rc-dialogs
class MfcDialogRegistry
{
public:
    /// Функция построения разметки: создаёт виджеты в переданном `QDialog*` (вызов `setupUi`).
    typedef void (*SetupFn)(void* qDialog);   // строит разметку в QDialog

    /// @brief Регистрирует функцию разметки диалога.
    /// @param[in] idd    номер ресурса диалога
    /// @param[in] setup  функция построения разметки
    static void registerDialog(UINT idd, SetupFn setup);
    /// @brief Регистрирует символьное имя элемента (`IDC_*`), под которым он стал `objectName` виджета.
    /// @param[in] id    номер элемента
    /// @param[in] name  имя; NULL — не регистрируется
    static void registerControlName(UINT id, const char* name);

    /// @brief Возвращает функцию разметки диалога или NULL, если диалог не зарегистрирован.
    /// @param[in] idd  номер ресурса диалога
    /// @return Функция разметки; NULL — диалог с номером `idd` не зарегистрирован.
    static SetupFn setupFor(UINT idd);
    /// @brief Возвращает символьное имя элемента или NULL.
    /// @param[in] id  номер элемента
    /// @return Имя элемента (принадлежит реестру); NULL — имя не зарегистрировано.
    static const char* controlName(UINT id);
};

// --- Стандартные диалоги ------------------------------------------------------
/// @brief Диалог открытия/сохранения файла поверх `QFileDialog`.
class CFileDialog
{
public:
    /// @brief Запоминает параметры диалога.
    /// @param[in] openFileDialog  TRUE — открыть, FALSE — сохранить
    /// @param[in] defExt          расширение по умолчанию (запоминается, не используется)
    /// @param[in] fileName        начальное имя или путь (CP1251)
    /// @param[in] flags           флаги `OFN_*` (не используются)
    /// @param[in] filter          фильтр MFC «Описание|шаблон|…||» (CP1251)
    /// @param[in] parent          окно-родитель (запоминается, не используется)
    CFileDialog(BOOL openFileDialog, const char* defExt = 0, const char* fileName = 0,
                DWORD flags = 0, const char* filter = 0, CWnd* parent = 0);
    /// @brief Показывает системный диалог файлов.
    /// @return `IDOK` — файл выбран (путь в `GetPathName`); `IDCANCEL` — отказ.
    INT_PTR DoModal();
    /// @brief Возвращает выбранный путь (локальная 8-битная кодировка, разделитель `/`).
    /// @return Полный путь; пустая строка — файл не выбран.
    CString GetPathName() const { return CString(m_path.c_str()); }

    /// Параметры в стиле Windows (слой их не читает).
    OPENFILENAME m_ofn;   // код настраивает поля напрямую, как в MFC

private:
    /// TRUE — диалог открытия.
    BOOL        m_open;
    /// Расширение по умолчанию.
    std::string m_defExt;
    /// Начальное имя (CP1251).
    std::string m_fileName;
    /// Фильтр в формате MFC.
    std::string m_filter;
    /// Выбранный путь.
    std::string m_path;
    /// Окно-родитель.
    CWnd*       m_parent;
};

/// @brief Диалог выбора цвета: в Windows-сборке — окно `tmcPickColor` (`gui_common/colorpicker`), иначе `QColorDialog`.
class CColorDialog
{
public:
    /// @param[in] initial  начальный цвет
    /// @param[in] flags    флаги `CC_*` (не используются)
    /// @param[in] parent   окно-родитель (не используется)
    explicit CColorDialog(COLORREF initial = 0, DWORD flags = 0, CWnd* parent = 0);
    /// @brief Показывает диалог цвета.
    /// @return `IDOK` — цвет выбран (`GetColor`); `IDCANCEL` — отказ.
    INT_PTR  DoModal();
    /// @brief Возвращает выбранный (или начальный) цвет.
    /// @return Цвет `RGB(r, g, b)`.
    COLORREF GetColor() const { return m_color; }

    /// Параметры в стиле Windows (слой их не читает).
    CHOOSECOLOR m_cc;

private:
    /// Текущий цвет.
    COLORREF m_color;
    /// Окно-родитель.
    CWnd*    m_parent;
};

/// @brief Диалог выбора шрифта поверх `QFontDialog`.
class CFontDialog
{
public:
    /// @param[in] initial  начальный шрифт или NULL
    /// @param[in] flags    флаги `CF_*` (не используются)
    /// @param[in] dc       контекст принтера (не используется)
    /// @param[in] parent   окно-родитель (не используется)
    explicit CFontDialog(LOGFONT* initial = 0, DWORD flags = 0, void* dc = 0, CWnd* parent = 0);
    /// @brief Показывает диалог шрифта; результат пишется в `m_lf` (высота — минус пиксели или пункты).
    /// @return `IDOK` или `IDCANCEL`.
    INT_PTR DoModal();
    /// @brief Возвращает `m_cf.rgbColors` (слоем не заполняется).
    /// @return Значение поля `m_cf.rgbColors`.
    COLORREF GetColor() const { return m_cf.rgbColors; }

    /// Выбранный шрифт.
    LOGFONT     m_lf;
    /// Параметры в стиле Windows (слой их не заполняет).
    CHOOSEFONT  m_cf;

private:
    /// Окно-родитель.
    CWnd* m_parent;
};

// --- Рамки окон ---------------------------------------------------------------
// Главное окно и дочерние окна в Qt-версии свои, поэтому здесь только то, что
// нужно для компиляции заголовков Windows-версии.
/// @brief Строка состояния: `SetPaneText` передаёт текст хозяину (`hostSetStatusText`), перерисовка — пустая.
class CStatusBar : public CWnd
{
public:
    // В Windows перерисовка строки состояния касалась только её самой и стоила
    // копейки. У нас строка состояния — часть окна Qt и перерисовывается сама
    // при смене текста, а вызов перерисовки «хозяина» приводил бы к полной
    // перерисовке ВИДА документа. Счётное ядро зовёт PutTrace на каждом такте,
    // поэтому такая перерисовка замедляла счёт в десятки раз.
    /// @brief Ничего не делает (см. выше).
    void RedrawWindow(const RECT* = 0, void* = 0, UINT = 0) {}
    /// @brief Ничего не делает.
    void UpdateWindow() {}
    /// @brief Ничего не делает.
    void Invalidate(BOOL = TRUE) {}
    /// @brief Заглушка; TRUE.
    /// @return Всегда TRUE.
    BOOL Create(CWnd*) { return TRUE; }
    /// @brief Заглушка; TRUE.
    /// @return Всегда TRUE.
    BOOL SetIndicators(const UINT*, int) { return TRUE; }
    /// @brief Выводит текст в поле строки состояния.
    /// @param[in] pane  номер поля
    /// @param[in] text  текст в CP1251
    void SetPaneText(int pane, const char* text)
    {
        if (m_host)
            m_host->hostSetStatusText(pane, text);
    }
};

// Стиль панели инструментов: кнопки без рамок, рамка появляется под указателем.
// Так создана панель FieldView (mainfrm.cpp). В Qt это вид кнопок по умолчанию,
// поэтому константа нужна только для компиляции вызова.
/// Стиль плоской панели инструментов.
#ifndef TBSTYLE_FLAT
  #define TBSTYLE_FLAT 0x0800
#endif

/// @brief Заглушка панели инструментов MFC: панели строит оболочка программы на Qt.
class CToolBar : public CWnd
{
public:
    /// @brief Заглушка; TRUE.
    /// @return Всегда TRUE.
    BOOL Create(CWnd*, DWORD = 0, UINT = 0) { return TRUE; }
    /// @brief Заглушка; TRUE.
    /// @return Всегда TRUE.
    BOOL CreateEx(CWnd*, DWORD = 0, DWORD = 0) { return TRUE; }
    /// @brief Заглушка; TRUE.
    /// @return Всегда TRUE.
    BOOL LoadToolBar(UINT) { return TRUE; }
    /// @brief Заглушка.
    void EnableDocking(DWORD) {}
    /// @brief Заглушка.
    void SetBarStyle(DWORD) {}
    /// @brief Возвращает 0.
    /// @return Всегда 0.
    DWORD GetBarStyle() const { return 0; }
};

/// @brief Заглушка окна-рамки MFC: главное окно — окно Qt оболочки программы.
class CFrameWnd : public CWnd
{
public:
    /// @brief Заглушка создания рамки; 0.
    /// @return Всегда 0.
    virtual int OnCreate(LPCREATESTRUCT) { return 0; }
    /// @brief Заглушка; TRUE.
    /// @return Всегда TRUE.
    virtual BOOL PreCreateWindow(CREATESTRUCT&) { return TRUE; }
    /// @brief Заглушка.
    virtual void OnDestroy() {}
    /// @brief Заглушка.
    void EnableDocking(DWORD) {}
    /// @brief Заглушка.
    void DockControlBar(CToolBar*) {}
    // Положением окна в Qt-версии распоряжается сам Qt; здесь — заглушки,
    // сохраняющие последнее заданное значение.
    /// @brief Возвращает последнее значение, заданное `SetWindowPlacement`.
    /// @param[out] wp  приёмник; NULL — ничего
    /// @return TRUE.
    BOOL GetWindowPlacement(WINDOWPLACEMENT* wp) const
    {
        if (wp) *wp = m_placement;
        return TRUE;
    }
    /// @brief Запоминает положение окна (окно не перемещается).
    /// @param[in] wp  положение; NULL — ничего
    /// @return TRUE.
    BOOL SetWindowPlacement(const WINDOWPLACEMENT* wp)
    {
        if (wp) m_placement = *wp;
        return TRUE;
    }
    /// @brief Заглушка: окно рамки не показывается слоем.
    void ShowWindow(int) {}

private:
    /// Последнее заданное положение.
    WINDOWPLACEMENT m_placement = {};

public:
    /// @brief Пустая карта сообщений рамки.
    /// @param[in,out] tmcMap  таблица (не меняется)
    virtual void TmcBuildMessageMap(MfcMessageMap& tmcMap) { (void)tmcMap; }
};

/// Заглушки рамок MDI.
class CMDIFrameWnd : public CFrameWnd {};
class CMDIChildWnd : public CFrameWnd {};
/// @brief Заглушка вида-формы MFC: пустой `OnDraw` и `DoDataExchange`.
class CFormView : public CView
{
public:
    /// @brief Создаёт вид; номер ресурса формы не используется.
    explicit CFormView(UINT = 0) {}
    /// @brief Ничего не рисует.
    void OnDraw(CDC*) override {}
    /// @brief Пустой обмен данными.
    virtual void DoDataExchange(CDataExchange*) {}
};

// --- Доработка готового диалога слоем интерфейса -------------------------------
// Слой совместимости строит окно по разметке из ресурсов Windows и больше о нём
// ничего не знает. Но некоторые окна нужно доводить до ума уже на месте: в окне
// «О программе» — сделать адрес сайта и руководство ссылками и приписать, для
// какой системы собрана программа. Где лежит документация и как называется
// система — дело слоя интерфейса, поэтому здесь только точка расширения:
// обработчик, если он установлен, вызывается для каждого построенного окна.
/// Тип оформителя окна: получает `QDialog*` после построения разметки.
typedef void (*TmcDialogDecorator)(void* qDialog);
/// @brief Устанавливает оформитель, вызываемый для каждого окна `DoModal`/`Create`.
///
/// Хранилище реализовано в `gui_common/aboutbox.cpp` (библиотека `tmc_ui_base`).
/// @param[in] fn  оформитель; NULL — не вызывать
/// @see https://www.tamic.ru/docs/code/gui-common.html#about
void tmc_set_dialog_decorator(TmcDialogDecorator fn);
/// @brief Возвращает установленный оформитель.
/// @return Оформитель или NULL.
TmcDialogDecorator tmc_dialog_decorator();

// Стандартные команды каркаса (ID_FILE_OPEN и прочие) нужны и коду видов.
#include "afxres.h"

#endif // !_WIN32
