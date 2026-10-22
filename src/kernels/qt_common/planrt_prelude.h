// planrt_prelude.h — добавка к преамбуле MFC-версии для PlanarRT_H и PlanarRT_X.
//
// Зачем нужна. Оболочки счётных ядер зовут из MFC и win32 чуть больше, чем
// вьюверы: страницы свойств (мелодия оповещения), сведения о версии системы,
// атрибуты файла, дескрипторы потока. Общий слой совместимости
// (src/compat/include) этого пока не даёт, а править его в этой работе нельзя —
//
// Поэтому недостающее объявлено здесь, и CMake подставляет этот файл (-include)
// сразу за общей преамбулой tmc_win_prelude.h — только для целей PlanarRT_H и
// PlanarRT_X. Сами исходники Windows-версии при этом не меняются.
//
// Ни одной формулы и ни одной константы расчёта здесь нет: только оконная и
// системная обвязка. Когда перечисленное появится в src/compat, этот файл
// сократится до нуля.

/// @file planrt_prelude.h
/// @brief Добавка к преамбуле `MFC-версии` для оболочек ядер H и X: прослойки документа и вида, недостающие функции Windows.
///
/// Часть модуля «Оболочка счётного ядра». Подставляется ключом компилятора
/// `-include` во все файлы целей `planarrt_h*` и `planarrt_x*` (и в файлы Windows-версии,
/// и в файлы Qt-оболочки) и сам включает общую преамбулу `tmc_win_prelude.h`.
/// Подменяет базовые классы `CDocument` и `CScrollView` прослойками с обработчиками,
/// которых нет в слое совместимости; объявляет связь кода ядра с Qt-слоем
/// (окно с вопросом, закрытие главного окна). Реализация — `planrt_compat.cpp`.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#prelude
#pragma once

#if !defined(_WIN32) || defined(TMC_QT_WIN)

// Общая преамбула MFC-версии идёт первой: она даёт CString, win-типы и _complex.
// Включается здесь, а не отдельным -include, потому что CMake схлопывает два
// одинаковых ключа -include в один.
#include "tmc_win_prelude.h"

#include <climits>      // INT_MAX: MSVC давал его через свой <limits.h>
#include <cstdlib>
#include <string>
#include <vector>

#include "tmc_mfc_doc.h"
#include "mfc_dialog.h"
#include "mfc_view.h"
#include "mfc_thread.h"

// --- Документ -----------------------------------------------------------------
// Документ оболочки переопределяет два обработчика каркаса MFC, которых нет в
// CDocument слоя совместимости:
//   OnCloseDocument — закрытие документа (здесь останавливается счёт);
//   CanCloseFrame   — разрешение закрыть окно (запрещает закрытие во время
//                     чтения данных и счёта).
// Оба зовут реализацию базового класса. Чтобы не править ни слой совместимости,
// ни исходники Windows-версии, база подменяется: CDocument в этих двух
// программах — вот этот класс-прослойка, а он в свою очередь наследуется от
// CDocument слоя. Поведение MFC по умолчанию у обоих обработчиков пустое
// («да, можно») — оно и повторено.
/// @brief Прослойка между `CPlanRT_HDoc` и `CDocument` слоя совместимости: обработчики закрытия документа MFC.
///
/// Через макрос `CDocument` становится базовым классом документа ядра. Оба метода
/// повторяют поведение MFC по умолчанию: ничего не делать и разрешить закрытие.
class TmcPlanRtDocument : public CDocument
{
public:
    /// Закрытие документа. В MFC освобождала виды и содержимое; в Qt-версии
    /// этим распоряжается окно, поэтому здесь пусто.
    /// @brief Ничего не делает (окна и память освобождает Qt-оболочка).
    virtual void OnCloseDocument() {}

    /// Можно ли закрыть окно с этим документом. В MFC по умолчанию — да.
    /// Рамка окна не используется.
    /// @return TRUE — закрыть можно.
    virtual BOOL CanCloseFrame(CFrameWnd* /*pFrame*/) { return TRUE; }
};

/// Подмена базового класса документа прослойкой `TmcPlanRtDocument` (до конца единицы трансляции).
#define CDocument TmcPlanRtDocument

// --- Вид ----------------------------------------------------------------------
// Вид оболочки объявляет обработчик OnClose (закрытие окна) и зовёт из него
// реализацию базового класса. В CScrollView слоя совместимости такого метода
// нет — закрытием окон распоряжается Qt. База подменяется так же, как у
// документа: пустая прослойка поверх CScrollView слоя.
//
// Внимание: заголовки общего слоя интерфейса (mfcviewwidget.h) объявляют
// setView(CScrollView*) — их надо включать при снятой подмене, иначе в
// объявлении окажется другой тип. Как это делается, показано в mainwindow.cpp.
/// @brief Прослойка между `CPlanRT_HView` и `CScrollView` слоя совместимости: обработчик `OnClose`.
///
/// Через макрос `CScrollView` становится базовым классом вида ядра. Заголовки
/// `gui_common` надо включать при снятой подмене (`#undef CScrollView`).
class TmcPlanRtScrollView : public CScrollView
{
public:
    /// Закрытие окна вида. В MFC по умолчанию разрушало окно; в Qt-версии окно
    /// закрывает сам интерфейс, поэтому здесь пусто.
    /// @brief Ничего не делает (окно закрывает Qt-оболочка).
    virtual void OnClose() {}
};

/// Подмена базового класса вида прослойкой `TmcPlanRtScrollView`.
#define CScrollView TmcPlanRtScrollView

// --- WIN.INI ------------------------------------------------------------------
// Настройки внешних просмотрщиков код сперва пробует прочитать из общесистемного
// WIN.INI (win32-функция GetProfileString с пятью аргументами). На современной
// Windows этот файл пуст и функция возвращает значение по умолчанию — третий
// аргумент. Под Linux WIN.INI нет вовсе, поэтому поведение то же: отдаём
// значение по умолчанию. Личные настройки программы читаются отдельно, через
// AfxGetApp()->GetProfileString (файл ~/.config/TMC_Suite/<программа>.conf).

#ifndef _WIN32 // Windows API (под Windows — настоящий)
/// @brief Чтение WIN.INI вне Windows: всегда отдаёт значение по умолчанию.
/// @param[in]  section      раздел (не используется).
/// @param[in]  entry        ключ (не используется).
/// @param[in]  defaultValue значение по умолчанию; NULL — пустая строка.
/// @param[out] buffer       буфер результата (строка обрезается по размеру).
/// @param[in]  size         размер буфера, байт.
/// @return Число записанных символов без завершающего нуля.
DWORD TmcGetWinIniProfileString(const char* section, const char* entry,
                                const char* defaultValue, char* buffer, DWORD size);
/// @brief Функция Windows API `GetProfileString` (вне Windows): см. `TmcGetWinIniProfileString`.
/// @param[in]  section      раздел.
/// @param[in]  entry        ключ.
/// @param[in]  defaultValue значение по умолчанию.
/// @param[out] buffer       буфер результата.
/// @param[in]  size         размер буфера, байт.
/// @return Число записанных символов.
inline DWORD GetProfileString(const char* section, const char* entry,
                              const char* defaultValue, char* buffer, DWORD size)
{
    return TmcGetWinIniProfileString(section, entry, defaultValue, buffer, size);
}

// --- Куча ---------------------------------------------------------------------
// _heapmin возвращал системе неиспользуемую память кучи. В glibc такой ручки
// нет (malloc_trim не входит в стандарт и не обязателен), а на результат счёта
// вызов не влияет — только на занятую память.
/// @brief Возврат свободной памяти кучи системе (вне Windows — ничего не делает).
/// @return 0 — успех.
inline int _heapmin(void) { return 0; }
#endif // !_WIN32

// --- Атрибуты файла -----------------------------------------------------------
// GetFileAttributes и FILE_ATTRIBUTE_ARCHIVE появились в общем слое
// (compat/include/tmc_mfc.h), здесь ничего объявлять не нужно.

// --- Сообщения окон -----------------------------------------------------------
// Из всех сообщений Windows коду нужно одно: WM_CLOSE. Значение как в win32 —
// оно нигде не пишется в файлы, но пусть совпадает.
#ifndef WM_CLOSE
  /// Сообщение Windows «закрыть окно».
  #define WM_CLOSE 0x0010
#endif

/// Закрыть главное окно программы. Расчётное ядро в пакетном режиме просит об
/// этом сообщением `pFrame->PostMessage( WM_CLOSE )`; обработчик сообщения
/// стоит в карте CMainFrame (planarrt_common/mainframe.cpp) и зовёт эту
/// функцию, а Qt-слой связывает её с закрытием окна.
/// @param[in] message код сообщения (`WM_CLOSE`).
void TmcPostFrameClose(UINT message);

// --- Версия системы -----------------------------------------------------------
// По ней Windows-версия выбирала способ подать звуковой сигнал: на NT — Beep,
// на 95/98 — прямая работа с портами. На Linux остаётся ветка NT (Beep), её и
// сообщаем.
#ifndef _WIN32 // Windows API (под Windows — настоящий)
/// @name Семейства Windows для `OSVERSIONINFO::dwPlatformId`
///@{
/// Значение `dwPlatformId`: Win32s, Windows 95/98, Windows NT.
#define VER_PLATFORM_WIN32s        0
#define VER_PLATFORM_WIN32_WINDOWS 1
#define VER_PLATFORM_WIN32_NT      2

///@}
/// @brief Сведения о версии системы (вне Windows; заполняет `GetVersionEx`).
struct OSVERSIONINFO
{
    /// Размер структуры, байт (задаёт вызывающий).
    DWORD dwOSVersionInfoSize;
    /// Старший номер версии (здесь 6).
    DWORD dwMajorVersion;
    /// Младший номер версии (здесь 1).
    DWORD dwMinorVersion;
    /// Номер сборки (здесь 0).
    DWORD dwBuildNumber;
    /// Семейство: `VER_PLATFORM_WIN32_NT`.
    DWORD dwPlatformId;
    /// Пакет обновления (здесь пустая строка).
    char  szCSDVersion[128];
};

/// @brief Сообщает версию системы вне Windows: всегда «NT 6.1» (для звука выбирается ветка Beep).
/// @param[out] info структура сведений.
/// @return TRUE — заполнено; FALSE — `info` равен NULL.
inline BOOL GetVersionEx(OSVERSIONINFO* info)
{
    if (!info)
        return FALSE;
    info->dwMajorVersion = 6;
    info->dwMinorVersion = 1;
    info->dwBuildNumber  = 0;
    info->dwPlatformId   = VER_PLATFORM_WIN32_NT;
    info->szCSDVersion[0] = '\0';
    return TRUE;
}
#endif // !_WIN32

// --- Процессы и потоки --------------------------------------------------------
#ifndef DUPLICATE_SAME_ACCESS
  /// Флаг `DuplicateHandle`: те же права доступа.
  #define DUPLICATE_SAME_ACCESS 0x00000002
#endif

#ifndef _WIN32
/// @brief `GetTickCount` вне Windows: см. `TmcGetTickCount`.
/// @return Миллисекунды от первого вызова.
inline DWORD GetTickCount(void) { return TmcGetTickCount(); }

/// Описатель текущего процесса. Дескрипторы потоков в слое совместимости —
/// указатели на объект потока, дублировать их не нужно, поэтому «процесс»
/// здесь — просто ненулевая метка.
/// @return Метка текущего процесса `(HANDLE)-1`.
inline HANDLE GetCurrentProcess(void) { return (HANDLE)-1; }
#else
// Windows: GetCurrentProcess настоящий, а копия дескриптора потока слоя — он сам.
/// В Windows-сборке `DuplicateHandle` заменён функцией ниже (дескриптор потока — объект слоя).
#define DuplicateHandle TmcDuplicateHandle
#endif

/// Копия дескриптора потока. В слое совместимости дескриптор — указатель на
/// объект потока, живущий до конца работы программы, поэтому копия — он сам.
///
/// Процессы, права, наследование и флаги не используются.
/// @param[in]  srcHandle  дескриптор потока слоя совместимости.
/// @param[out] dstHandle  копия дескриптора (тот же указатель).
/// @return TRUE — скопировано; FALSE — `dstHandle` равен NULL.
inline BOOL DuplicateHandle(HANDLE /*srcProcess*/, HANDLE srcHandle,
                            HANDLE /*dstProcess*/, HANDLE* dstHandle,
                            DWORD /*access*/, BOOL /*inherit*/, DWORD /*options*/)
{
    if (!dstHandle)
        return FALSE;
    *dstHandle = srcHandle;
    return TRUE;
}

/// Снятие внешнего процесса (в Windows-версии — закрытие просмотрщика в
/// демонстрационном режиме). Демонстрационного режима в этой сборке нет
/// (_VERSION_PROF___), ветка недостижима.
#ifndef _WIN32
/// @param[in] process  процесс (не используется).
/// @param[in] exitCode код завершения (не используется).
/// @return TRUE.
inline BOOL TerminateProcess(HANDLE /*process*/, UINT /*exitCode*/) { return TRUE; }
#endif

// --- Окно с вопросом ----------------------------------------------------------
// В слое совместимости AfxMessageBox принимает только текст. Рамке окна нужен
// вариант с кнопками (Yes/No при закрытии во время счёта).
// Объявление AfxMessageBox с кнопками теперь в общем слое (mfc_dialog.h).
/// @brief Окно сообщения с кнопками для строки `CString` (переходник к варианту слоя совместимости).
/// @param[in] text   текст сообщения.
/// @param[in] type   флаги кнопок и значка `MB_*`.
/// @param[in] helpId номер справки (не используется).
/// @return Нажатая кнопка: `IDOK`, `IDYES`, `IDNO` …
inline int AfxMessageBox(const CString& text, UINT type, UINT helpId = 0)
{
    return AfxMessageBox((const char*)text, type, helpId);
}

// --- Связь с Qt-слоем ---------------------------------------------------------
// Счётное ядро программы собирается без графики, поэтому окно с вопросом и
// закрытие главного окна оно просит через обработчики. Qt-слой ставит свои при
// старте (см. planarrt_common/mainwindow.cpp).
/// Обработчик окна сообщения: текст, флаги `MB_*` -> нажатая кнопка (`IDOK`, `IDYES`, `IDNO`).
typedef int  (*TmcPlanRtMessageBoxFn)(const char* text, UINT type);
/// Обработчик просьбы закрыть главное окно: код сообщения (`WM_CLOSE`).
typedef void (*TmcPlanRtCloseFn)(UINT message);

/// @brief Ставит обработчик окна сообщения (Qt-слой — при создании главного окна).
/// @param[in] fn обработчик; NULL — снять.
void TmcPlanRtSetMessageBoxHandler(TmcPlanRtMessageBoxFn fn);
/// @brief Ставит обработчик закрытия главного окна (Qt-слой — при создании главного окна).
/// @param[in] fn обработчик; NULL — снять (закрытие не выполняется).
void TmcPlanRtSetCloseHandler(TmcPlanRtCloseFn fn);

// --- Страницы свойств ---------------------------------------------------------
// CPropertyPage и CPropertySheet (диалог мелодии оповещения) появились в общем
// слое (compat/include/mfc_dialog.h) — закладками, как в Windows. Здесь ничего
// объявлять не нужно.

#endif // !_WIN32
