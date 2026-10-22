// tmc_mfc_doc.h — слой совместимости для классов документа MFC.
//
// Классы «документа» в TMC (CTMCGROUTDoc и родня) хранят модель данных и читают
// и пишут файлы задания. Это самая чувствительная часть: формат файлов должен
// совпадать с Windows-версией байт в байт. Поэтому код документа НЕ переписан на
// Qt — он компилируется из исходников Windows-версии как есть, а MFC-обвязка,
// которой он касается, заменена здесь.
//
// Из MFC документу нужно немного: базовый класс CDocument с путём и флагом
// изменения, макросы карт сообщений (пустые), COLORREF/RGB, LOGFONT и три-четыре
// функции Windows API. Всё это ниже.
//
// Границы: здесь нет ни одной формулы. Единственное отличие поведения от Windows
// оговорено на месте — настройки графиков (цвета, типы линий) на Windows лежали
// в реестре, на Linux кладутся в файл ~/.config/TMC_Suite/<программа>.conf.

/// @file tmc_mfc_doc.h
/// @brief Каркас документа MFC без окон: `CObject`, `CDocument`, `CWinApp` (настройки), `CCmdUI`, исключения, цвета, шрифт.
///
/// Часть модуля «Слой совместимости». Не зависит от Qt: его используют и
/// классы документов программ, и счётные ядра. Реализация — `tmc_mfc_doc.cpp`.
/// Действует в сборке под Windows (MinGW, `TMC_QT_WIN`) и под Linux; типы и
/// функции Windows API объявляются здесь только вне Windows.
/// @see https://www.tamic.ru/docs/code/compat.html#doc-view
#pragma once

#if !defined(_WIN32) || defined(TMC_QT_WIN)

#include "tmc_mfc.h"

#include <cstdio>
#include <cstring>
#include <string>

// --- Цвета ------------------------------------------------------------------
// В Windows COLORREF это 0x00BBGGRR. Порядок байтов важен: значения цветов
// попадают в файлы задания числами, и Windows-версия должна прочитать их так же.
#ifndef _WIN32 // Windows API (под Windows — настоящий windows.h)
/// Цвет Windows `0x00BBGGRR` (только вне Windows; в Windows — из `windows.h`).
typedef unsigned long COLORREF;
#endif // !_WIN32 (Windows API)

/// Собирает `COLORREF` из компонент `r`, `g`, `b` (0…255).
#ifndef RGB
  #define RGB(r, g, b) ((COLORREF)(((BYTE)(r)) | (((BYTE)(g)) << 8) | (((BYTE)(b)) << 16)))
#endif
/// Извлекают красную, зелёную и синюю компоненты (0…255) из `COLORREF`.
#ifndef GetRValue
  #define GetRValue(rgb) ((BYTE)((rgb) & 0xff))
  /// Извлекает зелёную компоненту (0…255) из `COLORREF`.
  #define GetGValue(rgb) ((BYTE)((((unsigned long)(rgb)) >> 8) & 0xff))
  /// Извлекает синюю компоненту (0…255) из `COLORREF`.
  #define GetBValue(rgb) ((BYTE)((((unsigned long)(rgb)) >> 16) & 0xff))
#endif

// --- Стили линий (wingdi.h) --------------------------------------------------
// Номер стиля уходит в файл задания, поэтому значения обязаны совпадать с
// Windows: файл, записанный здесь, должна одинаково прочитать и та версия.
#ifndef _WIN32 // Windows API (под Windows — настоящий windows.h)
/// Стили пера GDI (значения `wingdi.h`; номер стиля сохраняется в файлах настроек).
#define PS_SOLID        0
#define PS_DASH         1
#define PS_DOT          2
#define PS_DASHDOT      3
#define PS_DASHDOTDOT   4
#define PS_NULL         5
#define PS_INSIDEFRAME  6

// --- Шрифт ------------------------------------------------------------------
// Поля и константы те же, что в Windows: код документа сохраняет их в настройки.
/// Длина имени шрифта в `LOGFONT`, включая завершающий ноль.
#define LF_FACESIZE 32

/// Константы `LOGFONT` (кодировка, точность, качество, семейство, насыщенность) — значения `wingdi.h`.
#define ANSI_CHARSET         0
#define DEFAULT_CHARSET      1
// Японская кодировка знаков: FieldView ставит её в LOGFONT подписей осей
// (fldviewview.cpp, tmcdialogplace.cpp, tmcgltext.cpp) — так в оригинале.
#define SHIFTJIS_CHARSET   128
#define OUT_DEFAULT_PRECIS   0
// Точность вывода и отсечения: те же значения, что в wingdi.h. Заполняются
// поля LOGFONT в FieldView.
#define OUT_STROKE_PRECIS    3
#define CLIP_DEFAULT_PRECIS  0
#define CLIP_CHARACTER_PRECIS 1
#define DEFAULT_QUALITY      0
#define DRAFT_QUALITY        1
#define PROOF_QUALITY        2
#define DEFAULT_PITCH        0
#define FIXED_PITCH          1
#define VARIABLE_PITCH       2
#define FF_DONTCARE          0
#define FF_ROMAN            16
#define FF_SWISS            32
#define FF_MODERN           48
#define FF_SCRIPT           64
#define FF_DECORATIVE       80
#define FW_NORMAL          400
#define FW_BOLD            700
/// @brief Описание логического шрифта GDI (поля как в `wingdi.h`; только вне Windows).
struct LOGFONT
{
    /// Высота шрифта в логических единицах; < 0 — высота символа без межстрочного интервала.
    long  lfHeight;
    /// Средняя ширина символа; 0 — подбирается по высоте.
    long  lfWidth;
    /// Угол наклона строки, десятые доли градуса.
    long  lfEscapement;
    /// Угол наклона символов, десятые доли градуса.
    long  lfOrientation;
    /// Насыщенность: 400 — обычный (`FW_NORMAL`), 700 — жирный (`FW_BOLD`).
    long  lfWeight;
    /// Признаки курсива, подчёркивания и зачёркивания (0/1).
    BYTE  lfItalic;
    BYTE  lfUnderline;
    BYTE  lfStrikeOut;
    /// Кодировка знаков (`ANSI_CHARSET` …).
    BYTE  lfCharSet;
    /// Точность вывода, точность отсечения, качество, шаг и семейство — как в GDI.
    BYTE  lfOutPrecision;
    BYTE  lfClipPrecision;
    BYTE  lfQuality;
    BYTE  lfPitchAndFamily;
    /// Имя гарнитуры, строка с завершающим нулём.
    char  lfFaceName[LF_FACESIZE];
};
#endif // !_WIN32 (Windows API)

// --- Заглушки каркаса MFC ----------------------------------------------------
/// Таблица обработчиков карты сообщений (определена в `mfc_dialog.h`).
class MfcMessageMap;

/// @brief Корневой класс иерархии MFC; несёт точку построения карты сообщений.
/// @see https://www.tamic.ru/docs/code/compat.html#message-map
class CObject
{
public:
    /// @brief Виртуальный деструктор.
    virtual ~CObject() {}
    // Карта сообщений MFC (см. mfc_dialog.h): макросы BEGIN_MESSAGE_MAP строят
    // таблицу «команда -> обработчик», и цепочка вызовов идёт к базовому классу.
    /// @brief Заполняет таблицу обработчиков класса; переопределяется макросом `BEGIN_MESSAGE_MAP`.
    /// Параметр — таблица, в которую добавляются обработчики (в базовом классе не меняется).
    virtual void TmcBuildMessageMap(MfcMessageMap&) {}
};

// Сериализация MFC в TMC не используется: документы читаются и пишутся своим
// кодом через FILE*. Заглушка нужна только чтобы скомпилировался пустой
// Serialize(), поэтому IsStoring всегда сообщает «чтение».
/// @brief Заглушка архива сериализации MFC; в пакете сериализация не используется.
class CArchive
{
public:
    /// @brief Всегда FALSE: архив «читается».
    /// @return Всегда FALSE.
    BOOL IsStoring() const { return FALSE; }
    /// @brief Всегда TRUE.
    /// @return Всегда TRUE.
    BOOL IsLoading() const { return TRUE; }
};

/// @brief Заглушка контекста отладочного вывода MFC.
class CDumpContext {};

// --- Обновление состояния команды --------------------------------------------
// В MFC каркас перед показом меню и панели опрашивал обработчики
// ON_UPDATE_COMMAND_UI: они говорят, доступен ли пункт (Enable) и нажат ли он
// (SetCheck/SetRadio). Так кнопки-переключатели FieldView (Line, Surface,
// Dimension 2/3, типы плазмы, Proportionally) показывают включённый режим.
//
// Здесь роль пункта меню играет действие Qt (QAction). Слой совместимости
// заголовков Qt не включает, поэтому указатель хранится нетипизированным, а
// применяет решение обработчика слой интерфейса
// (src/gui_common/commandupdate.cpp): он вызывает обработчик и переносит
// записанное состояние в действие.
/// @brief Состояние команды меню/панели, заполняемое обработчиком `ON_UPDATE_COMMAND_UI`.
///
/// Создаётся на время одного опроса (`gui_common/commandupdate.cpp`), передаётся
/// обработчику, затем записанное состояние переносится в `QAction`.
/// @see https://www.tamic.ru/docs/code/gui-common.html#command-update
class CCmdUI
{
public:
    /// Идентификатор команды, для которой вызван обработчик.
    UINT  m_nID = 0;

    /// Действие Qt (QAction*), состояние которого меняется. Ставит слой
    /// интерфейса перед вызовом обработчика.
    void* m_pQtAction = 0;

    /// Доступность пункта.
    /// @param[in] b  TRUE — пункт доступен, FALSE — недоступен
    void Enable(BOOL b = TRUE) { m_bEnabled = b ? TRUE : FALSE; m_bEnableSet = TRUE; }

    /// Отметка «нажато» (0 — снята, 1 — стоит, 2 — неопределённое состояние).
    /// @param[in] n  0 — снята, 1 — стоит, 2 — неопределённо
    void SetCheck(int n = 1) { m_nCheck = n; m_bCheckSet = TRUE; }

    /// То же для пункта-переключателя из группы.
    /// @param[in] b  TRUE — пункт выбран, FALSE — не выбран
    void SetRadio(BOOL b = TRUE) { m_nCheck = b ? 1 : 0; m_bCheckSet = TRUE; m_bRadio = TRUE; }

    // Что записал обработчик. Читает слой интерфейса; если обработчик чего-то
    // не тронул, соответствующее свойство действия остаётся прежним.
    /// @brief Возвращает TRUE, если обработчик вызвал `Enable`.
    /// @return TRUE — обработчик вызвал `Enable`; FALSE — доступность не задавалась.
    BOOL EnableWasSet() const { return m_bEnableSet; }
    /// @brief Возвращает доступность, записанную обработчиком.
    /// @return TRUE — команда доступна; FALSE — недоступна (смысл имеет, если `EnableWasSet()`).
    BOOL Enabled()      const { return m_bEnabled; }
    /// @brief Возвращает TRUE, если обработчик вызвал `SetCheck` или `SetRadio`.
    /// @return TRUE — отметка задавалась; FALSE — нет.
    BOOL CheckWasSet()  const { return m_bCheckSet; }
    /// @brief Возвращает отметку: 0 — снята, 1 — стоит, 2 — неопределённо.
    /// @return 0 — снята, 1 — стоит, 2 — неопределённо (смысл имеет, если `CheckWasSet()`).
    int  Check()        const { return m_nCheck; }
    /// @brief Возвращает TRUE, если отметка задана через `SetRadio`.
    /// @return TRUE — отметка-переключатель (`SetRadio`); FALSE — флажок.
    BOOL IsRadio()      const { return m_bRadio; }

private:
    /// Доступность, записанная обработчиком.
    BOOL m_bEnabled   = TRUE;
    /// Признак «обработчик вызвал `Enable`».
    BOOL m_bEnableSet = FALSE;
    /// Отметка: 0, 1 или 2.
    int  m_nCheck     = 0;
    /// Признак «обработчик задал отметку».
    BOOL m_bCheckSet  = FALSE;
    /// Признак «отметка задана как у переключателя группы».
    BOOL m_bRadio     = FALSE;
};

// Макросы карт сообщений и динамического создания. В Qt их роль выполняют
// сигналы и слоты, поэтому здесь они ни во что не разворачиваются.
// В MFC документы и виды создавал каркас (DYNCREATE), поэтому их конструкторы
// объявлены защищёнными. Каркас теперь наш, Qt-слой, — макрос добавляет ему
// фабричный метод, не трогая сам класс. Уровень доступа восстанавливается на
// protected: именно так стоит по соглашению MFC (макрос идёт сразу за защищённым
// конструктором, дальше в заголовке идёт явное public:).
/// Добавляет классу фабричный метод `TmcCreate()`; оставляет доступ `protected`, как в MFC.
#define DECLARE_DYNCREATE(cls)     public: static cls* TmcCreate() { return new cls(); }     protected:
/// Пустой: реализация динамического создания не нужна.
#define IMPLEMENT_DYNCREATE(cls, base)
/// Пустые варианты макросов карты сообщений для кода без окон (`mfc_dialog.h` переопределяет их рабочими).
#define DECLARE_MESSAGE_MAP()
/// Пустой вариант: открывает неиспользуемую функцию.
#define BEGIN_MESSAGE_MAP(cls, base) void tmc_unused_message_map_##cls() {
/// Пустой вариант: закрывает функцию.
#define END_MESSAGE_MAP() }
/// Пустой вариант записи команды.
#define ON_COMMAND(id, fn)
/// Пустой вариант записи обновления состояния.
#define ON_UPDATE_COMMAND_UI(id, fn)
/// Пустой вариант записи таймера.
#define ON_WM_TIMER()
/// Пометка обработчика сообщения MFC: пустая.
#define afx_msg

// --- CDocument ---------------------------------------------------------------
// Ровно то, чем пользуется код документа TMC: путь к файлу, заголовок и флаг
// изменения. Всё остальное (окна, виды, печать) — забота Qt-слоя.
/// @brief Документ MFC: путь к файлу, заголовок, флаг изменения и признак «нужно перерисовать виды».
///
/// Классы документов программ (`CTMCGROUTDoc` и др.) наследуются от него без
/// изменений. Создаётся оболочкой программы через `TmcCreate()`; читает и пишет
/// файлы собственным кодом документа.
/// @see https://www.tamic.ru/docs/code/compat.html#doc-view
class CDocument : public CObject
{
public:
    /// @brief Создаёт пустой неизменённый документ.
    CDocument() : m_bModified(FALSE) {}
    /// @brief Виртуальный деструктор.
    virtual ~CDocument() {}

    /// @brief Возвращает полный путь к файлу документа (пустая строка — не задан).
    /// @return Путь (принадлежит документу; не NULL); пустая строка — путь не задан.
    const char* GetPathName() const { return m_strPathName.c_str(); }
    /// @brief Запоминает путь к файлу документа.
    ///
    /// Под Windows путь приводится к полному (`_fullpath`) и с обратными косыми
    /// чертами, как в MFC: ядро строит от него имена выходных файлов.
    /// @param[in] path       путь к файлу; NULL — пустой
    /// Второй параметр (добавить ли в список последних файлов) не используется: список ведёт `gui_common/recentfiles`.
    void SetPathName(const char* path, BOOL /*bAddToMRU*/ = TRUE)
    {
        m_strPathName = path ? path : "";
#ifdef _WIN32
        // Как MFC (AfxFullPath): полный путь с обратными слэшами. Ядро строит
        // имена выходных файлов от каталога задания и пишет путь в заголовок .t/.s.
        char full[_MAX_PATH * 2];
        if (!m_strPathName.empty() && _fullpath(full, m_strPathName.c_str(), sizeof(full)))
            m_strPathName = full;
        for (char& c : m_strPathName)
            if (c == '/') c = '\\';
#endif
    }

    /// @brief Возвращает заголовок документа.
    /// @return Заголовок (принадлежит документу; не NULL).
    const char* GetTitle() const { return m_strTitle.c_str(); }
    /// @brief Задаёт заголовок документа.
    /// @param[in] title  заголовок; NULL — пустой
    void SetTitle(const char* title) { m_strTitle = title ? title : ""; }

    /// @brief Подготавливает новый документ; базовая реализация ничего не делает.
    /// @return TRUE — успех.
    virtual BOOL OnNewDocument() { return TRUE; }
    /// @brief Освобождает данные документа; базовая реализация пустая.
    virtual void DeleteContents() {}
    /// Закрытие документа. В MFC каркас звал их перед закрытием окна: сначала
    /// спрашивал разрешение (идёт ли расчёт), потом освобождал данные.
    /// Параметр (окно-рамка) в базовой реализации не используется.
    /// @return TRUE — закрывать можно.
    virtual BOOL CanCloseFrame(void* /*frame*/ = 0) { return TRUE; }
    /// @brief Вызывается при закрытии документа; базовая реализация пустая.
    virtual void OnCloseDocument() {}
    /// @brief Пустая сериализация MFC.
    virtual void Serialize(CArchive&) {}

    /// @brief Устанавливает флаг «документ изменён».
    /// @param[in] bModified  TRUE — изменён
    void SetModifiedFlag(BOOL bModified = TRUE) { m_bModified = bModified; }
    /// @brief Возвращает флаг «документ изменён».
    /// @return TRUE — документ изменён после открытия или сохранения; FALSE — нет.
    BOOL IsModified() const { return m_bModified; }

    // В MFC это оповещение видов о необходимости перерисоваться. В Qt-версии
    // перерисовкой управляет виджет, поэтому здесь достаточно флага: Qt-слой
    // читает его после операции и обновляет вид.
    /// @brief Отмечает, что виды документа нужно перерисовать.
    /// Параметры (вид-инициатор, числовая и объектная подсказки) не используются.
    void UpdateAllViews(void* /*pSender*/, long /*lHint*/ = 0, CObject* /*pHint*/ = 0)
    {
        m_bViewsDirty = TRUE;
    }
    /// @brief Возвращает и сбрасывает признак «виды нужно перерисовать».
    /// @return TRUE — после последнего вызова был `UpdateAllViews`.
    BOOL TakeViewsDirty()
    {
        BOOL d = m_bViewsDirty;
        m_bViewsDirty = FALSE;
        return d;
    }

    /// @brief Пустая проверка целостности (отладочный метод MFC).
    virtual void AssertValid() const {}
    /// @brief Пустой отладочный вывод MFC.
    virtual void Dump(CDumpContext&) const {}

protected:
    /// Полный путь к файлу документа.
    std::string m_strPathName;
    /// Заголовок документа.
    std::string m_strTitle;
    /// Признак «документ изменён».
    BOOL        m_bModified;
    /// Признак «виды нужно перерисовать» (ставит `UpdateAllViews`, снимает `TakeViewsDirty`).
    BOOL        m_bViewsDirty = FALSE;
};

// --- Настройки программы -----------------------------------------------------
// На Windows это реестр (HKCU\Software\<компания>\<программа>). На Linux реестра
// нет, поэтому те же пары «раздел/ключ -> строка» лежат в ini-файле
// ~/.config/TMC_Suite/<программа>.conf. Содержимое строк не меняется, поэтому
// файлы задания и поведение программы остаются прежними.
/// @brief Объект приложения MFC: хранит имя программы, главное окно и настройки.
///
/// Единственный экземпляр возвращает `AfxGetApp()`. Настройки хранятся в
/// ini-файле: Windows — `%APPDATA%\TMC_Suite_Qt\<программа>.conf`, Linux —
/// `~/.config/TMC_Suite/<программа>.conf`.
/// @see https://www.tamic.ru/docs/code/compat.html#settings
class CWinApp
{
public:
    /// @brief Читает строковую настройку.
    /// @param[in] section  раздел ini-файла
    /// @param[in] entry    ключ
    /// @param[in] def      значение по умолчанию
    /// @return Сохранённое значение или `def`.
    CString GetProfileString(const char* section, const char* entry,
                             const char* def = "");
    /// @brief Записывает строковую настройку (файл перечитывается и перезаписывается целиком).
    /// @param[in] section  раздел
    /// @param[in] entry    ключ
    /// @param[in] value    значение; NULL — пустая строка
    /// @return Всегда TRUE (ошибка записи файла не сообщается).
    BOOL WriteProfileString(const char* section, const char* entry, const char* value);
    /// @brief Читает целочисленную настройку.
    /// @param[in] section  раздел
    /// @param[in] entry    ключ
    /// @param[in] def      значение, если ключа нет или он пуст
    /// @return Значение (`atoi`) или `def`.
    int  GetProfileInt(const char* section, const char* entry, int def);
    /// @brief Записывает целочисленную настройку (десятичной строкой).
    /// @param[in] section  раздел
    /// @param[in] entry    ключ
    /// @param[in] value    значение
    /// @return Всегда TRUE.
    BOOL WriteProfileInt(const char* section, const char* entry, int value);

    // Имя программы задаёт Qt-слой при старте: от него зависит имя conf-файла.
    /// @brief Задаёт имя программы (имя файла настроек и `m_pszExeName`).
    /// @param[in] name  имя без расширения; пустое или NULL — не меняется
    static void SetAppName(const char* name);

    // В MFC отсюда начиналась жизнь программы. В Qt-версии точка входа своя,
    // поэтому метод есть только ради компиляции классов приложения из MFC-версии.
    /// @brief Пустая инициализация приложения MFC.
    /// @return TRUE.
    virtual BOOL InitInstance() { return TRUE; }
    /// @brief Виртуальный деструктор.
    virtual ~CWinApp() {}

    // Имя исполняемого файла без расширения (префикс строк в консоли, /Arb).
    /// Имя исполняемого файла без расширения; указывает на внутреннюю строку слоя.
    const char* m_pszExeName = "";

    // Главное окно. Код программ достаёт через него строку состояния
    // (см. PutTrace/PutStatistics), поэтому Qt-слой заполняет это поле при старте.
    /// Главное окно программы; заполняет оболочка при старте.
    class CWnd* m_pMainWnd = 0;
};

/// @brief Возвращает единственный объект приложения (создаётся при первом вызове).
/// @return Указатель на статический объект приложения (не освобождать; не NULL).
CWinApp* AfxGetApp();

// --- Исключения MFC --------------------------------------------------------------
// Код расчёта ловит catch( CException *e ) — прежде всего нехватку памяти. Как в
// MFC, нехватка памяти в new приходит исключением CMemoryException* (обработчик
// new ставит слой, tmc_mfc_doc.cpp), а не std::bad_alloc.
/// @brief Базовое исключение MFC; ловится кодом расчёта как `CException*`.
class CException
{
public:
    /// @brief Виртуальный деструктор.
    virtual ~CException() {}
    /// @brief Копирует текст исключения в буфер.
    /// @param[out] buf   буфер приёмника
    /// @param[in]  max   размер буфера, байт
    /// @param[out] help  номер справки; записывается 0 (может быть NULL)
    /// @return Всегда TRUE.
    virtual BOOL GetErrorMessage(char* buf, UINT max, UINT* help = 0) const
    {
        if (help) *help = 0;
        if (buf && max) { std::strncpy(buf, what(), max - 1); buf[max - 1] = 0; }
        return TRUE;
    }
    /// @brief Возвращает текст исключения.
    /// @return Статическая строка `"exception"`; наследники возвращают свой текст.
    virtual const char* what() const { return "exception"; }
    /// @brief Удаляет объект исключения (как `e->Delete()` в MFC).
    void Delete() { delete this; }
};
/// @brief Исключение «нехватка памяти»; бросается из `new` (обработчик ставит `tmc_mfc_doc.cpp`).
class CMemoryException : public CException
{
public:
    /// @brief Возвращает «Not enough memory».
    /// @return Статическая строка `"Not enough memory"`.
    const char* what() const override { return "Not enough memory"; }
};

#ifndef _WIN32 // Windows API (под Windows — настоящий windows.h)
// Сравнение времён записи файлов, как в Windows API: -1, 0 или 1.
/// @brief Сравнивает два момента времени файла.
/// @param[in] a  первое время
/// @param[in] b  второе время
/// @return -1 — `a` раньше, 0 — равны (или NULL), 1 — `a` позже.
int CompareFileTime(const FILETIME* a, const FILETIME* b);

// --- Небольшая часть Windows API --------------------------------------------
/// Описатель объекта Windows API (только вне Windows).
typedef void* HANDLE;
/// Недействительный описатель.
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
/// Константы `CreateFile` (принимаются, но не используются эмуляцией).
#define GENERIC_READ    0x80000000u
#define FILE_SHARE_READ 0x00000001u
#define OPEN_EXISTING   3

/// @brief Сведения о запущенном процессе (как в Windows API).
struct PROCESS_INFORMATION
{
    /// Описатель процесса (вне Windows — идентификатор процесса).
    HANDLE hProcess;
    /// Описатель главного потока (вне Windows — 0).
    HANDLE hThread;
    /// Идентификатор процесса.
    DWORD  dwProcessId;
    /// Идентификатор главного потока (вне Windows — 0).
    DWORD  dwThreadId;
};

/// @brief Параметры запуска процесса (как в Windows API); эмуляцией не используется.
struct STARTUPINFO
{
    DWORD cb;
    char* lpReserved;
    char* lpDesktop;
    char* lpTitle;
    DWORD dwX, dwY, dwXSize, dwYSize;
    DWORD dwXCountChars, dwYCountChars, dwFillAttribute, dwFlags;
    WORD  wShowWindow, cbReserved2;
    BYTE* lpReserved2;
    HANDLE hStdInput, hStdOutput, hStdError;
};

/// Обнуляет `len` байт по адресу `dest`.
#ifndef ZeroMemory
  #define ZeroMemory(dest, len) std::memset((dest), 0, (len))
#endif

// Открывает файл только чтобы узнать время записи (см. LastWriteTime в коде
// документа). Возвращает описатель, который понимает наш GetFileTime.
/// @param[in] fileName   путь к файлу
/// @param[in] access     режим доступа (не используется)
/// @param[in] shareMode  режим совместного доступа (не используется)
/// @param[in] security   атрибуты защиты (не используется)
/// @param[in] creation   способ открытия (не используется)
/// @param[in] flags      флаги (не используются)
/// @param[in] templ      шаблон (не используется)
/// @return Описатель, хранящий время записи; освобождать `CloseHandle`. Никогда не `INVALID_HANDLE_VALUE`.
HANDLE CreateFile(const char* fileName, DWORD access, DWORD shareMode,
                  void* security, DWORD creation, DWORD flags, HANDLE templ);
/// @brief Возвращает время файла, сохранённое в описателе `CreateFile`.
/// @param[in]  h         описатель из `CreateFile`
/// @param[out] creation  время создания (= время записи); может быть NULL
/// @param[out] access    время доступа (= время записи); может быть NULL
/// @param[out] write     время последней записи; может быть NULL
/// @return TRUE — файл существовал при `CreateFile`.
BOOL   GetFileTime(HANDLE h, FILETIME* creation, FILETIME* access, FILETIME* write);
/// @brief Освобождает описатель `CreateFile`.
/// @param[in] h  описатель
/// @return TRUE.
BOOL   CloseHandle(HANDLE h);

#endif // !_WIN32 (Windows API)

// Перевод windows-имени программы в имя нашей.
//
// Код Windows-версии зовёт соседние программы пакета как «TMCROS.EXE»,
// «FLDVIEW.EXE» и т. д. Здесь имя переводится в наше и ищется рядом с текущей
// программой. Возвращает то, что будет запущено (или само имя, если ничего
// подходящего не нашлось). Вынесено наружу, чтобы это можно было проверить.
/// @brief Переводит windows-имя программы пакета в путь к программе Qt-версии.
/// @param[in] windowsName  имя или путь (`TMCROS.EXE`, `FLDVIEW.EXE` …); NULL — пустое
/// @return Путь к программе рядом с текущей; существующий путь — без изменений;
///         иначе — исходное имя (Windows) или имя в нижнем регистре для поиска по PATH (Linux).
/// @see https://www.tamic.ru/docs/code/compat.html#winapi
std::string TmcResolveProgramName(const char* windowsName);

#ifndef _WIN32 // Windows API (под Windows — настоящий windows.h)
// Запуск внешней программы (в TMC — текстового редактора для файла задания).
// Командная строка разбирается по пробелам, как это делает Windows.
/// @brief Запускает внешнюю программу (только вне Windows): `fork` + `execvp`.
/// @param[in]  appName        программа или NULL (тогда первое слово `commandLine`)
/// @param[in]  commandLine    командная строка; разбирается по пробелам, кавычки объединяют аргумент
/// @param[in]  processAttrs   не используется
/// @param[in]  threadAttrs    не используется
/// @param[in]  inheritHandles не используется
/// @param[in]  creationFlags  не используется
/// @param[in]  environment    не используется
/// @param[in]  currentDir     рабочий каталог дочернего процесса или NULL
/// @param[in]  startupInfo    не используется
/// @param[out] processInfo    идентификатор процесса (может быть NULL)
/// @return TRUE — процесс создан (успех `execvp` не проверяется); FALSE — пустая строка или ошибка `fork`.
BOOL CreateProcess(const char* appName, char* commandLine,
                   void* processAttrs, void* threadAttrs, BOOL inheritHandles,
                   DWORD creationFlags, void* environment, const char* currentDir,
                   STARTUPINFO* startupInfo, PROCESS_INFORMATION* processInfo);
#endif // !_WIN32 (Windows API)

#endif // !_WIN32
