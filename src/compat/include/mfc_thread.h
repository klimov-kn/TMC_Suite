// mfc_thread.h — потоки и ожидание MFC поверх Qt.
//
// Счётные оболочки (PlanarRT_H и PlanarRT_X) запускают расчёт в отдельном
// потоке: `AfxBeginThread(RunAllGlobal, pDoc, ..., CREATE_SUSPENDED)`, затем
// `ResumeThread()`. Останов идёт через флаг в расчётном классе, а закрытие окна
// во время счёта ждёт поток, прокачивая очередь сообщений.
//
// Здесь эти же вызовы работают поверх Qt. Важное свойство: расчёт остаётся в
// СВОЁМ потоке, а интерфейс — в главном, ровно как на Windows. Порядок вычислений
// и их результат от этого не зависят: поток исполняет ту же функцию с теми же
// данными.
//
// Внимание: обработчики из расчётного потока (строка состояния, звук) должны
// доходить до интерфейса через очередь событий Qt, а не прямым вызовом виджета.
// Этим занимается Qt-слой; здесь — только сами потоки и ожидание.

/// @file mfc_thread.h
/// @brief Потоки и ожидание MFC поверх `QThread`: `CWinThread`, `AfxBeginThread`, ожидание с прокачкой событий.
///
/// Часть модуля «Слой совместимости». Расчёт ядра выполняется в отдельном
/// потоке той же функцией, что в Windows-версии. В Windows-сборке функции
/// `WaitForSingleObject`, `MsgWaitForMultipleObjects`, `PeekMessage`,
/// `TranslateMessage`, `DispatchMessage`, `GetExitCodeThread` переименованы
/// макросами в `Tmc*` (см. `tmc_winapi.h`) — объявления ниже относятся к ним.
/// Реализация — `mfc_thread.cpp`.
/// @see https://www.tamic.ru/docs/code/compat.html#threads
#pragma once

#if !defined(_WIN32) || defined(TMC_QT_WIN)

#include "mfc_dialog.h"

#ifndef _WIN32 // Windows API (под Windows — настоящий windows.h)
// --- Константы Windows --------------------------------------------------------
/// Флаг создания потока «приостановленным» (запуск — `ResumeThread`).
#define CREATE_SUSPENDED          0x00000004
/// Приоритеты потоков Windows (значения `winbase.h`).
#define THREAD_PRIORITY_IDLE          (-15)
#define THREAD_PRIORITY_LOWEST         (-2)
#define THREAD_PRIORITY_BELOW_NORMAL   (-1)
#define THREAD_PRIORITY_NORMAL           0
#define THREAD_PRIORITY_ABOVE_NORMAL     1
#define THREAD_PRIORITY_HIGHEST          2
#define THREAD_PRIORITY_TIME_CRITICAL   15

/// Коды и константы ожидания Windows: бесконечное ожидание, событие наступило, время вышло, ошибка, маска событий.
#define INFINITE        0xFFFFFFFFu
#define WAIT_OBJECT_0   0x00000000u
#define WAIT_TIMEOUT    0x00000102u
#define WAIT_FAILED     0xFFFFFFFFu
#define QS_ALLINPUT     0x04FFu

/// Флаги `PeekMessage` (в слое игнорируются).
#define PM_NOREMOVE 0x0000
#define PM_REMOVE   0x0001

/// @brief Оконное сообщение Windows (только вне Windows; слоем всегда возвращается пустым).
struct MSG
{
    /// Окно-получатель.
    HWND   hwnd;
    /// Код сообщения.
    UINT   message;
    /// Первый параметр.
    WPARAM wParam;
    /// Второй параметр.
    LPARAM lParam;
    /// Время постановки в очередь, мс.
    DWORD  time;
    /// Положение курсора, пиксели экрана.
    POINT  pt;
};
#endif // !_WIN32 (Windows API)

// --- Поток --------------------------------------------------------------------
// Функция потока в MFC имеет вид `UINT Proc(LPVOID param)`.
/// Функция потока MFC: получает параметр `AfxBeginThread`, возвращает код завершения.
typedef UINT (*AFX_THREADPROC)(LPVOID param);

/// @brief Поток MFC, выполняемый внутренним `QThread`.
///
/// Создаётся только `AfxBeginThread`; удаляет его вызывающий код. Описатель
/// `m_hThread` — указатель на внутренний поток, по нему функции ожидания
/// находят поток в таблице.
/// @warning Деструктор ждёт завершения потока не более 5 с и затем удаляет `QThread`
///          даже если поток ещё работает.
/// @see https://www.tamic.ru/docs/code/compat.html#threads
class CWinThread
{
public:
    /// @brief Создаёт пустой объект (без потока).
    CWinThread();
    /// @brief Удаляет поток из таблицы, ждёт его до 5 с и освобождает.
    virtual ~CWinThread();

    /// Снять с паузы поток, созданный с CREATE_SUSPENDED. Возвращает прежний
    /// счётчик приостановок, как в Windows.
    /// @return 1; `DWORD(-1)` — потока нет.
    DWORD ResumeThread();
    /// @brief Запоминает приоритет и применяет его, если поток уже работает.
    /// @param[in] priority  `THREAD_PRIORITY_*`
    /// @return TRUE; FALSE — потока нет.
    BOOL  SetThreadPriority(int priority);

    /// Дескриптор потока: код Windows-версии хранит его в документе и передаёт
    /// в функции ожидания.
    HANDLE m_hThread;

    /// Запущенная функция и её параметр (нужны при отложенном старте).
    AFX_THREADPROC m_proc;
    /// Параметр функции потока.
    LPVOID         m_param;
    /// Код завершения (слоем не заполняется; используйте `GetExitCodeThread`).
    UINT           m_exitCode;

    /// Желаемый приоритет (THREAD_PRIORITY_*). Запоминается до старта:
    /// приоритет можно задать только запущенному потоку, а в Windows-версии
    /// поток создаётся приостановленным (CREATE_SUSPENDED) и запускается позже.
    /// Применяется при фактическом запуске — см. AfxBeginThread и ResumeThread.
    int            m_nPriority;

    // Внутренний объект потока Qt. Открыт наружу потому, что создаётся
    // фабрикой AfxBeginThread, а не самим классом (как в MFC).
    /// Внутренний объект `QThread` (владеет).
    void* m_impl;
};

/// Полный аналог MFC: создаёт поток, при CREATE_SUSPENDED не запускает сразу.
/// @param[in] proc           функция потока
/// @param[in] param          её параметр
/// @param[in] priority       приоритет `THREAD_PRIORITY_*`
/// @param[in] stackSize      размер стека (не используется)
/// @param[in] createFlags    `CREATE_SUSPENDED` — не запускать до `ResumeThread`
/// @param[in] securityAttrs  атрибуты защиты (не используются)
/// @return Новый объект потока; удаляет вызывающий код.
CWinThread* AfxBeginThread(AFX_THREADPROC proc, LPVOID param,
                           int priority = THREAD_PRIORITY_NORMAL,
                           UINT stackSize = 0, DWORD createFlags = 0,
                           void* securityAttrs = 0);

// --- Ожидание -----------------------------------------------------------------
// Возвращают те же коды, что Windows: WAIT_OBJECT_0 — событие наступило,
// WAIT_TIMEOUT — вышло время.
/// @brief Ждёт завершения потока слоя без прокачки событий.
/// @param[in] handle        описатель `CWinThread::m_hThread`
/// @param[in] milliseconds  время ожидания, мс; `INFINITE` — без ограничения
/// @return `WAIT_OBJECT_0`, `WAIT_TIMEOUT` или `WAIT_FAILED` (описатель неизвестен).
DWORD WaitForSingleObject(HANDLE handle, DWORD milliseconds);

/// Ожидание потока с прокачкой очереди сообщений. В Windows-версии этим
/// закрывается окно во время расчёта: интерфейс продолжает откликаться.
/// Здесь очередь сообщений — это очередь событий Qt.
/// @param[in] count         число описателей
/// @param[in] handles       описатели потоков
/// @param[in] waitAll       не используется (ждёт первый завершившийся)
/// @param[in] milliseconds  время ожидания, мс; `INFINITE` — без ограничения
/// @param[in] wakeMask      не используется
/// @return `WAIT_OBJECT_0 + i` — завершился (или неизвестен) поток `i`; `WAIT_TIMEOUT`; `WAIT_FAILED` — пустой список.
/// @note События Qt обрабатываются порциями до 20 мс между проверками.
DWORD MsgWaitForMultipleObjects(DWORD count, const HANDLE* handles, BOOL waitAll,
                                DWORD milliseconds, DWORD wakeMask);

/// Прокачка очереди событий (аналог PeekMessage/TranslateMessage/DispatchMessage).
/// В Windows-версии эти три вызова идут подряд, поэтому здесь они сведены к
/// обработке накопившихся событий Qt.
/// @brief Обрабатывает накопившиеся события Qt (до 10 мс) и сообщает, что сообщений нет.
/// @param[out] msg        обнуляется (может быть NULL)
/// @param[in]  hwnd       не используется
/// @param[in]  filterMin  не используется
/// @param[in]  filterMax  не используется
/// @param[in]  removeFlag не используется
/// @return Всегда FALSE.
BOOL PeekMessage(MSG* msg, HWND hwnd, UINT filterMin, UINT filterMax, UINT removeFlag);
/// @brief Ничего не делает; TRUE.
/// @param[in] msg  сообщение (не используется)
/// @return Всегда TRUE.
BOOL TranslateMessage(const MSG* msg);
/// @brief Ничего не делает; 0.
/// @param[in] msg  сообщение (не используется)
/// @return Всегда 0.
LRESULT DispatchMessage(const MSG* msg);

/// Код завершения потока (в Windows — GetExitCodeThread).
/// @param[in]  handle    описатель потока слоя
/// @param[out] exitCode  259 (`STILL_ACTIVE`) — поток работает; иначе результат функции потока
/// @return TRUE; FALSE — описатель неизвестен.
BOOL GetExitCodeThread(HANDLE handle, DWORD* exitCode);

#ifndef _WIN32 // Windows API (под Windows — настоящий windows.h)
/// Звуковой сигнал: в Windows-версии оповещает об окончании шага и всего счёта.
/// @param[in] frequencyHz  частота тона, Гц (не используется: подаётся системный сигнал)
/// @param[in] durationMs   длительность, мс (не используется)
void Beep(DWORD frequencyHz, DWORD durationMs);
#endif // !_WIN32 (Windows API)

#endif // !_WIN32
