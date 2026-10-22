// tmc_mfc.h — минимальный слой совместимости MFC/win для Qt-версии TMC Suite.
//
// MFC на Linux нет. Вычислительные библиотеки TMC (SFILE95, PREPR, TMCIndan,
// Expr/Inter, complex, TMCLibError) используют из MFC по сути только строковый
// тип CString и несколько win-типов (BOOL/TRUE/FALSE). Здесь — их минимальные
// эквиваленты, достаточные для сборки под g++.
//
// ВАЖНО:
//  * Это НЕ математика — только перенос типов и утилит. Формулы и числа
//    не затрагиваются.
//  * CString — ТРИВИАЛЬНО-КОПИРУЕМЫЙ тип с единственным членом char* (как MFC
//    CString, внутри которого один указатель на буфер). Это критично: исходники
//    передают CString в printf/Format через "%s" (varargs). На MSVC объект
//    кладётся на стек как один указатель, и "%s" читает именно его. Нетривиально
//    копируемый тип (обёртка над std::string) g++ передавал бы по скрытой ссылке,
//    и "%s" прочитал бы мусор. Инвариант закреплён static_assert ниже.
//  * Буферы CString выделяются malloc и не освобождаются (COW через
//    переаллокацию): освобождающий деструктор сделал бы тип нетривиальным.
//  * Сообщения об ошибках библиотеки шлют через AfxMessageBox. В GUI-версии
//    консольный вывод не годится, поэтому здесь точка расширения:
//    tmc_set_message_handler() ставит обработчик, который GUI подменяет на
//    QMessageBox. Библиотеки при этом остаются БЕЗ зависимости от Qt.
//
// Активен только вне Windows: на самой Windows используется настоящий MFC.

/// @file tmc_mfc.h
/// @brief Основа слоя совместимости: строковый тип `CString`, базовые типы Windows, `AfxMessageBox`.
///
/// Часть модуля «Слой совместимости». Действует в сборке под Windows (MinGW,
/// макрос `TMC_QT_WIN`; типы Windows там берутся из настоящего `windows.h`)
/// и под Linux (типы и несколько функций Windows API объявлены здесь же).
/// Не зависит от Qt: его подключают и счётные библиотеки.
/// @see https://www.tamic.ru/docs/code/compat.html#strings
#pragma once

#if !defined(_WIN32) || defined(TMC_QT_WIN)

#include <string>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstddef>
#include <cerrno>
#ifdef _WIN32
#include "tmc_winapi.h"
#else
#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/stat.h>
#if defined(__APPLE__)
  /// Код ошибки «расширенного атрибута нет» (macOS — `ENOATTR`, Linux — `ENODATA`).
  #define ENOATTR_TMC ENOATTR
#else
  #define ENOATTR_TMC ENODATA
#endif
#endif
#include <type_traits>

#ifndef _WIN32 // Windows API (под Windows — настоящий windows.h)
// --- Базовые win-типы -------------------------------------------------------
/// Логический тип Windows: 0 — ложь, иначе истина (только вне Windows).
typedef int            BOOL;
/// Символ строки; сборка однобайтовая (`_MBCS`), поэтому `char`.
typedef char           TCHAR;
/// Указатели на строки в нотации Windows; все — однобайтовые `char*`.
typedef const char*    LPCTSTR;
typedef char*          LPTSTR;
typedef const char*    LPCSTR;
typedef char*          LPSTR;
/// Беззнаковые целые и указатели в нотации Windows (только вне Windows).
typedef unsigned int   UINT;
typedef void*          LPVOID;
typedef unsigned long  DWORD;
typedef unsigned short WORD;
typedef unsigned char  BYTE;
typedef unsigned long  ULONG;
// LONG — тот же знаковый 32/64-битный тип, что и long. Встречается в
// объявлениях GDI (описание растра, размеры), например в tmcgltext.h FieldView.
typedef long           LONG;
// Параметры и результат оконного сообщения. Нужны в подписях обработчиков
// (ON_MESSAGE) и методов CWnd::SendMessage/PostMessage; объявлены здесь,
// а не в mfc_dialog.h, потому что ими пользуется уже слой окон (mfc_view.h).
/// Первый параметр оконного сообщения.
typedef unsigned long  WPARAM;
/// Второй параметр оконного сообщения.
typedef long           LPARAM;
/// Результат обработки оконного сообщения.
typedef long           LRESULT;
// Описатель окна. Настоящих окон Windows здесь нет: слой возвращает вместо
// него указатель на объект Qt, а код Windows-версии сверяет его только с нулём
// (см. CWnd::GetSafeHwnd).
typedef void*          HWND;
// Целое размером с указатель. В подписях обработчиков Windows-версии
// (например, OnTimer) стоит именно оно: если объявить обработчик с UINT,
// переопределения не получится и метод не будет вызываться никогда.
/// Беззнаковое целое размером с указатель (номер таймера в `OnTimer`).
typedef unsigned long  UINT_PTR;
/// Знаковое целое размером с указатель.
typedef long           INT_PTR;

/// Логические константы Windows.
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#endif // !_WIN32 (Windows API)

#ifndef _WIN32 // Windows API (под Windows — настоящий windows.h)
// --- FILETIME (win32-структура; на Linux нужна только как тип-носитель) -----
/// @brief Время файла Windows (100-нс интервалы с 1601 г.), разбитое на две половины по 32 бита.
struct FILETIME
{
    /// Младшие 32 бита.
    DWORD dwLowDateTime;
    /// Старшие 32 бита.
    DWORD dwHighDateTime;
};

// --- Sleep (периферия, не математика) ---------------------------------------
#include <thread>
#include <chrono>
/// @brief Приостанавливает текущий поток.
/// @param[in] ms  длительность паузы, мс
inline void Sleep(unsigned long ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// --- SetFileAttributes ------------------------------------------------------
// На Windows снимала read-only с выходных файлов. На содержимое и формат файла
// не влияет, поэтому под Linux — пустышка с успешным кодом.
/// Атрибут «обычный файл» (признак «архивный» снят).
#ifndef FILE_ATTRIBUTE_NORMAL
#define FILE_ATTRIBUTE_NORMAL 0x80
#endif
/// Атрибут «архивный»: файл изменён после последнего прочтения.
#ifndef FILE_ATTRIBUTE_ARCHIVE
#define FILE_ATTRIBUTE_ARCHIVE 0x20
#endif
/// Результат `GetFileAttributes` для несуществующего файла.
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFFu
#endif
// --- Set/GetFileAttributes ---------------------------------------------------
// Признак «архивный» в пакете играет одну роль: FieldView помечает им кадр поля,
// который уже прочитал (CFldViewDoc::SetSynchronizationFlag), а счётное ядро по
// нему понимает, можно ли писать следующий (CTmcRTHRectNode::IsFieldFileRead).
//
// В Windows этот бит ставит САМА система при любой записи в файл, а снять его
// может только программа. Отсюда и всё поведение пакета: ядро записало кадр —
// бит снова взведён, и ядро не ждёт; FieldView прочитал кадр и снял бит — до
// следующей записи файл считается прочитанным.
//
// На Linux и macOS такого бита нет. Повторяем его смысл точно: когда программа
// снимает признак, запоминаем в расширенном атрибуте файла время его последнего
// изменения. Признак считается снятым, только пока файл с тех пор не менялся;
// любая новая запись — и он снова взведён, как в Windows.
//
// Если файловая система расширенных атрибутов не поддерживает (например, раздел
// Windows, подключённый в /mnt), признак хранить негде — считаем его взведённым.
// Тогда ядро не ждёт просмотрщика: иначе при включённой синхронизации оно
// простаивало бы по 25 секунд на каждом кадре.
/// @brief Возвращает ссылку на признак «файловая система поддерживает расширенные атрибуты».
/// @return Ссылка на статический флаг; сбрасывается в false при первой ошибке `ENOTSUP`.
inline bool& tmc_xattr_works()
{
    static bool works = true;
    return works;
}

// Время последнего изменения файла в виде строки (секунды и наносекунды).
/// @brief Возвращает время последнего изменения файла строкой «секунды.наносекунды».
/// @param[in] fileName  путь к файлу
/// @return Строка-отметка; пустая — файла нет или `stat` не удался.
inline std::string tmc_file_stamp(const char* fileName)
{
    struct stat st;
    if (::stat(fileName, &st) != 0)
        return std::string();
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%lld.%09ld",
                  (long long)st.st_mtime, (long)st.st_mtim.tv_nsec);
    return std::string(buf);
}

/// @brief Взводит или снимает признак «архивный» у файла (эмуляция для Linux).
///
/// `FILE_ATTRIBUTE_ARCHIVE` — удаляет отметку о прочтении (признак взведён);
/// любое другое значение — записывает в расширенный атрибут `user.tmc_read_stamp`
/// текущую отметку времени файла (признак снят до следующей записи в файл).
/// @param[in] lpFileName        путь к файлу
/// @param[in] dwFileAttributes  `FILE_ATTRIBUTE_ARCHIVE` или другой атрибут
/// @return TRUE — выполнено (в том числе если атрибуты не поддерживаются); FALSE — пустое имя или файла нет.
inline BOOL SetFileAttributes(const char* lpFileName, DWORD dwFileAttributes)
{
    if (!lpFileName || !lpFileName[0])
        return FALSE;

    int rc = 0;
    if (dwFileAttributes == FILE_ATTRIBUTE_ARCHIVE) {
        // «Взвести» признак — значит забыть отметку о прочтении.
#if defined(__APPLE__)
        rc = ::removexattr(lpFileName, "user.tmc_read_stamp", 0);
#else
        rc = ::removexattr(lpFileName, "user.tmc_read_stamp");
#endif
        if (rc != 0 && errno == ENOATTR_TMC)
            rc = 0;                      // и так не стоял
    } else {
        const std::string stamp = tmc_file_stamp(lpFileName);
        if (stamp.empty())
            return FALSE;
#if defined(__APPLE__)
        rc = ::setxattr(lpFileName, "user.tmc_read_stamp",
                        stamp.c_str(), stamp.size(), 0, 0);
#else
        rc = ::setxattr(lpFileName, "user.tmc_read_stamp",
                        stamp.c_str(), stamp.size(), 0);
#endif
    }

    if (rc != 0 && errno == ENOTSUP)
        tmc_xattr_works() = false;
    return TRUE;
}

/// @brief Возвращает признак «архивный» файла (эмуляция для Linux).
/// @param[in] fileName  путь к файлу
/// @return `FILE_ATTRIBUTE_NORMAL` — файл прочитан и с тех пор не менялся;
///         `FILE_ATTRIBUTE_ARCHIVE` — изменён, отметки нет или атрибуты не поддерживаются;
///         `INVALID_FILE_ATTRIBUTES` — файла нет.
inline DWORD GetFileAttributes(const char* fileName)
{
    if (!fileName || !fileName[0])
        return DWORD(INVALID_FILE_ATTRIBUTES);

    const std::string stamp = tmc_file_stamp(fileName);
    if (stamp.empty())
        return DWORD(INVALID_FILE_ATTRIBUTES);   // файла нет

    if (!tmc_xattr_works())
        return FILE_ATTRIBUTE_ARCHIVE;

    char saved[64];
#if defined(__APPLE__)
    const ssize_t n = ::getxattr(fileName, "user.tmc_read_stamp",
                                 saved, sizeof(saved), 0, 0);
#else
    const ssize_t n = ::getxattr(fileName, "user.tmc_read_stamp",
                                 saved, sizeof(saved));
#endif
    if (n < 0) {
        if (errno == ENOTSUP)
            tmc_xattr_works() = false;
        return FILE_ATTRIBUTE_ARCHIVE;           // отметки нет — признак взведён
    }

    // Отметка есть: признак снят, только если файл с тех пор не менялся.
    if (std::string(saved, size_t(n)) == stamp)
        return FILE_ATTRIBUTE_NORMAL;
    return FILE_ATTRIBUTE_ARCHIVE;
}

// --- Строковые функции Windows API ------------------------------------------
// lstrcpy/lstrlen — те же strcpy/strlen; в однобайтовой сборке Windows это
// буквально они. Зовутся при заполнении LOGFONT (tmcgltext.cpp,
// tmcdialogplace.cpp в FieldView).
/// @brief Копирует строку (аналог `strcpy`; NULL-источник считается пустой строкой).
/// @param[out] dst  буфер приёмника, достаточной длины
/// @param[in]  src  исходная строка или NULL
/// @return `dst`.
inline char* lstrcpy(char* dst, const char* src) { return std::strcpy(dst, src ? src : ""); }
/// @brief Возвращает длину строки в байтах.
/// @param[in] s  строка или NULL
/// @return Длина; 0 для NULL.
inline int   lstrlen(const char* s) { return s ? int(std::strlen(s)) : 0; }

#endif // !_WIN32 (Windows API)

// --- Сообщения из библиотек -------------------------------------------------
// Обработчик по умолчанию пишет в stderr (годится для прогонов из консоли).
// GUI ставит свой через tmc_set_message_handler и показывает QMessageBox.
/// Тип обработчика сообщений `AfxMessageBox`: получает текст сообщения (UTF-8 или CP1251 — как передал вызывающий код).
typedef void (*TmcMessageHandler)(const char* text);

/// @brief Возвращает ссылку на хранилище обработчика сообщений.
/// @return Ссылка на статический указатель; `nullptr` — обработчик не задан.
inline TmcMessageHandler& tmc_message_handler_slot()
{
    static TmcMessageHandler h = nullptr;
    return h;
}

/// @brief Устанавливает обработчик сообщений для `AfxMessageBox`.
/// @param[in] h  обработчик; `nullptr` — вывод в stderr
inline void tmc_set_message_handler(TmcMessageHandler h)
{
    tmc_message_handler_slot() = h;
}

/// @brief Показывает сообщение библиотеки: через установленный обработчик или в stderr.
/// @param[in] msg  текст сообщения; NULL — пустая строка
inline void AfxMessageBox(const char* msg)
{
    const char* text = msg ? msg : "";
    if (TmcMessageHandler h = tmc_message_handler_slot())
        h(text);
    else
        std::fprintf(stderr, "%s\n", text);
}

// --- CString (тривиально-копируемый эквивалент MFC; внутри один char*) ------
// Ни деструктора, ни пользовательских copy/move — иначе тип перестанет быть
// trivially_copyable и сломается передача в "%s" varargs (см. шапку файла).
/// @brief Строка MFC: тривиально копируемая обёртка над одним указателем `char*`.
///
/// Буфер выделяется `malloc` при каждом изменении и никогда не освобождается;
/// копирование объекта копирует только указатель. Пустая строка указывает на
/// общий статический буфер. Позиции и длины — в байтах.
/// @warning Каждое изменение строки оставляет прежний буфер в памяти (утечка
///          по замыслу: освобождающий деструктор сделал бы тип нетривиальным
///          и сломал бы передачу в `printf("%s", s)`).
/// @see https://www.tamic.ru/docs/code/compat.html#strings
class CString
{
public:
    /// @brief Создаёт пустую строку.
    CString() : m_p(tmc_cstr_empty()) {}
    /// @brief Создаёт строку-копию `s`.
    /// @param[in] s  исходная строка; NULL — пустая
    CString(const char* s) : m_p(tmc_cstr_dup(s ? s : "")) {}

    /// @brief Присваивает копию строки `s`.
    /// @param[in] s  исходная строка; NULL — пустая
    /// @return Ссылка на эту строку.
    CString& operator=(const char* s) { m_p = tmc_cstr_dup(s ? s : ""); return *this; }

    // MFC CString::Format — printf-стиль. Новый буфер, прежний остаётся.
    /// @brief Формирует строку по шаблону `printf`.
    /// @param[in] fmt  шаблон `printf`
    /// @param[in] ...  значения для шаблона
    /// @note При ошибке форматирования строка становится пустой.
    void Format(const char* fmt, ...)
    {
        va_list ap;  va_start(ap, fmt);
        va_list ap2; va_copy(ap2, ap);
        int n = std::vsnprintf(nullptr, 0, fmt, ap);
        va_end(ap);
        if (n < 0) { m_p = tmc_cstr_empty(); va_end(ap2); return; }
        char* buf = static_cast<char*>(std::malloc(static_cast<size_t>(n) + 1));
        std::vsnprintf(buf, static_cast<size_t>(n) + 1, fmt, ap2);
        va_end(ap2);
        m_p = buf;
    }

    /// @brief Возвращает длину строки в байтах.
    /// @return Длина без завершающего нуля, байт.
    int  GetLength() const { return static_cast<int>(std::strlen(m_p)); }
    /// @brief Возвращает TRUE, если строка пустая.
    /// @return TRUE — строка пустая; FALSE — нет.
    BOOL IsEmpty()   const { return (m_p[0] == 0) ? TRUE : FALSE; }
    /// @brief Делает строку пустой.
    void Empty()           { m_p = tmc_cstr_empty(); }

    /// @brief Возвращает указатель на содержимое (действителен, пока строку не изменили).
    /// @return Указатель на строку CP1251 с завершающим нулём (принадлежит объекту; не NULL).
    const char* GetString() const { return m_p; }
    /// @brief Преобразует к `const char*` (тот же указатель, что `GetString`).
    operator const char*()  const { return m_p; }

    // Разбор строк, как в MFC (нужны коду многочастотности и ДН, снимок 22.10).
    // Позиции — в байтах; -1 — не найдено.
    /// @brief Ищет символ начиная с позиции `start`.
    /// @param[in] ch     искомый символ
    /// @param[in] start  начальная позиция, байт; вне `[0, GetLength()]` — сразу -1
    /// @return Позиция найденного символа или -1.
    int Find(char ch, int start = 0) const
    {
        if (start < 0 || start > GetLength()) return -1;
        const char* q = std::strchr(m_p + start, ch);
        return q ? int(q - m_p) : -1;
    }
    /// @brief Ищет подстроку начиная с позиции `start`.
    /// @param[in] sub    искомая подстрока; NULL — пустая
    /// @param[in] start  начальная позиция, байт; вне `[0, GetLength()]` — сразу -1
    /// @return Позиция начала вхождения или -1.
    int Find(const char* sub, int start = 0) const
    {
        if (start < 0 || start > GetLength()) return -1;
        const char* q = std::strstr(m_p + start, sub ? sub : "");
        return q ? int(q - m_p) : -1;
    }
    /// @brief Ищет последнее вхождение символа.
    /// @param[in] ch  искомый символ
    /// @return Позиция или -1.
    int ReverseFind(char ch) const
    {
        const char* q = std::strrchr(m_p, ch);
        return q ? int(q - m_p) : -1;
    }
    /// @brief Ищет первый символ, входящий в набор `set`.
    /// @param[in] set  набор символов; NULL — пустой
    /// @return Позиция или -1.
    int FindOneOf(const char* set) const
    {
        const size_t n = std::strcspn(m_p, set ? set : "");
        return m_p[n] ? int(n) : -1;
    }
    /// @brief Возвращает первые `n` символов (с ограничением в `[0, GetLength()]`).
    /// @param[in] n  число символов
    /// @return Новая строка.
    CString Left(int n) const
    {
        const int len = GetLength();
        if (n < 0) n = 0;
        if (n > len) n = len;
        return CString(std::string(m_p, size_t(n)).c_str());
    }
    /// @brief Возвращает последние `n` символов (с ограничением в `[0, GetLength()]`).
    /// @param[in] n  число символов
    /// @return Новая строка.
    CString Right(int n) const
    {
        const int len = GetLength();
        if (n < 0) n = 0;
        if (n > len) n = len;
        return CString(m_p + (len - n));
    }
    // Как в MFC: Mid(first) — до конца строки; Mid(first, n) при n < 0 — пустая
    // строка (идиома s.Mid(a, s.Find(x) - a), когда Find вернул −1), при n больше
    // остатка — остаток. Сравнение без переполнения first + n (Т-24).
    /// @brief Возвращает хвост строки начиная с позиции `first`.
    /// @param[in] first  начальная позиция (ограничивается `[0, GetLength()]`)
    /// @return Новая строка.
    CString Mid(int first) const
    {
        const int len = GetLength();
        if (first < 0) first = 0;
        if (first > len) first = len;
        return CString(m_p + first);
    }
    /// @brief Возвращает `n` символов начиная с позиции `first`.
    /// @param[in] first  начальная позиция (ограничивается `[0, GetLength()]`)
    /// @param[in] n      число символов; < 0 — пустая строка; больше остатка — остаток
    /// @return Новая строка.
    CString Mid(int first, int n) const
    {
        const int len = GetLength();
        if (first < 0) first = 0;
        if (first > len) first = len;
        if (n < 0) n = 0;
        if (n > len - first) n = len - first;
        return CString(std::string(m_p + first, size_t(n)).c_str());
    }
    /// @brief Заменяет все вхождения `from` на `to`.
    /// @param[in] from  заменяемая подстрока; пустая или NULL — замен нет
    /// @param[in] to    подстановка; NULL — пустая
    /// @return Число выполненных замен.
    int Replace(const char* from, const char* to)
    {
        const size_t nf = from ? std::strlen(from) : 0;
        if (nf == 0) return 0;
        std::string s(m_p), t(to ? to : "");
        int count = 0;
        for (size_t pos = 0; (pos = s.find(from, pos)) != std::string::npos; pos += t.size(), ++count)
            s.replace(pos, nf, t);
        if (count) m_p = tmc_cstr_dup(s.c_str());
        return count;
    }
    /// @brief Удаляет пробелы, табуляции и концы строк справа.
    /// @return Ссылка на эту строку.
    CString& TrimRight()
    {
        int n = GetLength();
        while (n > 0 && (m_p[n - 1] == ' ' || m_p[n - 1] == '\t' || m_p[n - 1] == '\r' || m_p[n - 1] == '\n'))
            --n;
        if (n != GetLength()) m_p = tmc_cstr_dup(std::string(m_p, size_t(n)).c_str());
        return *this;
    }

    /// @brief Дописывает строку в конец.
    /// @param[in] s  дописываемая строка (C-строка или `CString`); NULL — пустая
    /// @return Ссылка на эту строку.
    CString& operator+=(const char* s)    { m_p = tmc_cstr_cat(m_p, s ? s : ""); return *this; }
    /// @brief Дописывает строку `CString` в конец.
    /// @param[in] o  дописываемая строка
    /// @return Ссылка на эту строку.
    CString& operator+=(const CString& o) { m_p = tmc_cstr_cat(m_p, o.m_p);      return *this; }

    /// @brief Склеивает две строки; возвращает новую строку.
    friend CString operator+(const CString& a, const char* b)    { CString r(a); r += b; return r; }
    /// @brief Склеивает C-строку и `CString`.
    /// @param[in] a  левая часть (C-строка)
    /// @param[in] b  правая часть
    /// @return Новая строка `a + b`.
    friend CString operator+(const char* a, const CString& b)    { CString r(a); r += b; return r; }
    /// @brief Склеивает две строки `CString`.
    /// @param[in] a  левая часть
    /// @param[in] b  правая часть
    /// @return Новая строка `a + b`.
    friend CString operator+(const CString& a, const CString& b) { CString r(a); r += b; return r; }

    /// @brief Сравнивает строки побайтно (`strcmp`); NULL считается пустой строкой.
    /// @param[in] o  вторая строка
    /// @return true — строки совпадают побайтно.
    bool operator==(const CString& o) const { return std::strcmp(m_p, o.m_p) == 0; }
    /// @brief Сравнивает строки побайтно (`strcmp`).
    /// @param[in] o  вторая строка
    /// @return true — строки различаются.
    bool operator!=(const CString& o) const { return std::strcmp(m_p, o.m_p) != 0; }
    /// @brief Сравнивает с C-строкой побайтно; NULL считается пустой строкой.
    /// @param[in] s  C-строка или NULL
    /// @return true — строки совпадают.
    bool operator==(const char* s) const { return std::strcmp(m_p, s ? s : "") == 0; }
    /// @brief Сравнивает с C-строкой побайтно; NULL считается пустой строкой.
    /// @param[in] s  C-строка или NULL
    /// @return true — строки различаются.
    bool operator!=(const char* s) const { return std::strcmp(m_p, s ? s : "") != 0; }

private:
    /// @brief Возвращает общий статический пустой буфер.
    /// @return Указатель на статический буфер из одного нуля (не освобождать, не писать в него).
    static char* tmc_cstr_empty()
    {
        static char e[1] = { 0 };   // общий пустой буфер, на месте не пишется
        return e;
    }
    /// @brief Копирует строку в новый буфер `malloc`.
    /// @param[in] s  исходная строка (не NULL)
    /// @return Новый буфер; не освобождается.
    static char* tmc_cstr_dup(const char* s)
    {
        size_t n = std::strlen(s) + 1;
        char* p = static_cast<char*>(std::malloc(n));
        std::memcpy(p, s, n);
        return p;
    }
    /// @brief Склеивает две строки в новый буфер `malloc`.
    /// @param[in] a  первая строка (не NULL)
    /// @param[in] b  вторая строка (не NULL)
    /// @return Новый буфер; не освобождается.
    static char* tmc_cstr_cat(const char* a, const char* b)
    {
        size_t na = std::strlen(a), nb = std::strlen(b);
        char* p = static_cast<char*>(std::malloc(na + nb + 1));
        std::memcpy(p, a, na);
        std::memcpy(p + na, b, nb + 1);   // +1: копируем и завершающий ноль
        return p;
    }

    /// Указатель на буфер с завершающим нулём; никогда не NULL.
    char* m_p;   // единственный член -> объект ABI-эквивалентен указателю
};

static_assert(std::is_trivially_copyable<CString>::value,
              "CString must be trivially copyable (varargs %s relies on it)");

#endif // !_WIN32
