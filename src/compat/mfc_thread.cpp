// Реализация потоков и ожидания MFC поверх Qt (см. mfc_thread.h).
//
// Расчёт остаётся в отдельном потоке, как на Windows: та же функция, те же
// данные, тот же порядок вычислений. Меняется только способ создания потока и
// ожидания его завершения.

/// @file mfc_thread.cpp
/// @brief Реализация потоков MFC на `QThread` и ожидания с прокачкой очереди событий Qt.
///
/// Часть модуля «Слой совместимости», входит в библиотеку `tmc_gui_common`.
/// @see https://www.tamic.ru/docs/code/compat.html#threads
#if !defined(_WIN32) || defined(TMC_QT_WIN)

#include "mfc_thread.h"

#include <QCoreApplication>
#include <QDeadlineTimer>
#include <QElapsedTimer>
#include <QThread>

#include <map>

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  #include <QApplication>
#endif

namespace {

// Поток, исполняющий функцию в стиле MFC.
/// @brief `QThread`, выполняющий функцию потока MFC и запоминающий её результат.
class MfcThread : public QThread
{
public:
    /// @brief Запоминает функцию, параметр и владельца.
    /// @param[in] proc   функция потока
    /// @param[in] param  её параметр
    /// @param[in] owner  объект `CWinThread` (не владеет)
    MfcThread(AFX_THREADPROC proc, LPVOID param, CWinThread* owner)
        : m_proc(proc), m_param(param), m_owner(owner) {}

    /// @brief Выполняет функцию потока в новом потоке и запоминает код возврата.
    void run() override
    {
        if (m_proc)
            m_result = m_proc(m_param);
        m_finished = true;
    }

    /// @brief Возвращает код возврата функции потока (0 до завершения).
    /// @return Значение, возвращённое функцией потока; 0 — поток ещё не завершился.
    UINT result() const { return m_result; }
    /// @brief Возвращает true после возврата из функции потока.
    /// @return true — функция потока вернула управление; false — ещё выполняется или не запускалась.
    bool finished() const { return m_finished; }

private:
    /// Функция потока.
    AFX_THREADPROC m_proc;
    /// Параметр функции.
    LPVOID         m_param;
    /// Объект-владелец (не используется после создания).
    CWinThread*    m_owner;
    /// Код возврата функции потока.
    UINT           m_result = 0;
    /// Признак завершения функции потока.
    bool           m_finished = false;
};

// Дескриптор потока в Windows — непрозрачное число. Здесь роль дескриптора
// играет сам указатель на поток, а таблица позволяет по нему найти объект,
// когда код Windows-версии передаёт дескриптор в функции ожидания.
/// @brief Возвращает таблицу «описатель -> поток» для функций ожидания.
/// @return Ссылка на статическую таблицу (живёт до конца программы).
std::map<HANDLE, MfcThread*>& threadTable()
{
    static std::map<HANDLE, MfcThread*> t;
    return t;
}

/// @brief Находит поток по описателю.
/// @param[in] h  описатель
/// @return Поток или NULL.
MfcThread* threadFor(HANDLE h)
{
    std::map<HANDLE, MfcThread*>::const_iterator it = threadTable().find(h);
    return it == threadTable().end() ? 0 : it->second;
}

// Приоритет Windows -> приоритет Qt. Соответствие то же, что было в
// SetThreadPriority: расчёт идёт с тем же приоритетом, что задавала
// Windows-версия.
/// @brief Переводит приоритет Windows в приоритет Qt.
/// @param[in] priority  `THREAD_PRIORITY_*`
/// @return `LowestPriority` (<= LOWEST), `LowPriority`, `NormalPriority`, `HighPriority`, `HighestPriority` (>= HIGHEST).
QThread::Priority qtPriority(int priority)
{
    if (priority <= THREAD_PRIORITY_LOWEST)            return QThread::LowestPriority;
    if (priority == THREAD_PRIORITY_BELOW_NORMAL)      return QThread::LowPriority;
    if (priority == THREAD_PRIORITY_ABOVE_NORMAL)      return QThread::HighPriority;
    if (priority >= THREAD_PRIORITY_HIGHEST)           return QThread::HighestPriority;
    return QThread::NormalPriority;
}

} // namespace

// --- CWinThread ---------------------------------------------------------------

CWinThread::CWinThread()
    : m_hThread(0), m_proc(0), m_param(0), m_exitCode(0),
      m_nPriority(THREAD_PRIORITY_NORMAL), m_impl(0)
{
}

CWinThread::~CWinThread()
{
    MfcThread* t = static_cast<MfcThread*>(m_impl);
    if (t) {
        threadTable().erase(m_hThread);
        if (t->isRunning())
            t->wait(5000);
        // I-19: поток, не завершившийся за 5 с, не удаляется (удаление работающего QThread роняет
        // программу): объект потока удалит себя сам по окончании функции потока. MfcThread после
        // создания к владельцу не обращается, поэтому CWinThread можно удалить сразу.
        // Было: delete t; — и для ещё работающего потока.
        if (t->isRunning())
            QObject::connect(t, &QThread::finished, t, &QObject::deleteLater);
        else
            delete t;
    }
}

DWORD CWinThread::ResumeThread()
{
    MfcThread* t = static_cast<MfcThread*>(m_impl);
    if (!t)
        return DWORD(-1);
    // Приоритет задаётся вместе с запуском: у неработающего потока Qt его
    // принять не может. В Windows-версии поток создаётся приостановленным и
    // запускается отсюда, поэтому именно здесь приоритет и вступает в силу.
    if (!t->isRunning())
        t->start(qtPriority(m_nPriority));
    return 1;      // прежний счётчик приостановок, как в Windows
}


BOOL CWinThread::SetThreadPriority(int priority)
{
    MfcThread* t = static_cast<MfcThread*>(m_impl);
    if (!t)
        return FALSE;
    // Запоминаем в любом случае: если поток ещё не запущен, приоритет будет
    // задан при запуске (Qt умеет менять приоритет только работающему потоку и
    // иначе пишет в журнал «thread is not running», ничего не меняя).
    m_nPriority = priority;
    if (t->isRunning())
        t->setPriority(qtPriority(priority));
    return TRUE;
}

CWinThread* AfxBeginThread(AFX_THREADPROC proc, LPVOID param,
                           int priority, UINT /*stackSize*/, DWORD createFlags,
                           void* /*securityAttrs*/)
{
    CWinThread* winThread = new CWinThread();
    MfcThread* t = new MfcThread(proc, param, winThread);

    winThread->m_proc = proc;
    winThread->m_param = param;
    winThread->m_impl = t;
    winThread->m_hThread = static_cast<HANDLE>(t);
    threadTable()[winThread->m_hThread] = t;

    winThread->SetThreadPriority(priority);

    // Как в MFC: с флагом CREATE_SUSPENDED поток ждёт явного ResumeThread().
    // Приоритет передаётся прямо в start(): задать его до запуска нельзя.
    if (!(createFlags & CREATE_SUSPENDED))
        t->start(qtPriority(priority));

    return winThread;
}

// --- Ожидание -----------------------------------------------------------------

DWORD WaitForSingleObject(HANDLE handle, DWORD milliseconds)
{
    MfcThread* t = threadFor(handle);
    if (!t)
        return WAIT_FAILED;
    const bool done = (milliseconds == INFINITE)
        ? t->wait()
        : t->wait(QDeadlineTimer(int(milliseconds)));
    return done ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
}

DWORD MsgWaitForMultipleObjects(DWORD count, const HANDLE* handles, BOOL /*waitAll*/,
                                DWORD milliseconds, DWORD /*wakeMask*/)
{
    if (!count || !handles)
        return WAIT_FAILED;

    // Windows-версия ждёт поток и одновременно продолжает разбирать сообщения,
    // чтобы окно не «замерзало». Здесь то же самое: короткими шагами ждём поток
    // и отдаём время очереди событий Qt.
    QElapsedTimer timer;
    timer.start();

    for (;;) {
        for (DWORD i = 0; i < count; ++i) {
            MfcThread* t = threadFor(handles[i]);
            if (!t || !t->isRunning())
                return WAIT_OBJECT_0 + i;
        }

        if (QCoreApplication::instance())
            QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        else
            QThread::msleep(20);

        if (milliseconds != INFINITE && timer.elapsed() >= qint64(milliseconds))
            return WAIT_TIMEOUT;
    }
}

BOOL PeekMessage(MSG* msg, HWND /*hwnd*/, UINT /*filterMin*/, UINT /*filterMax*/,
                 UINT /*removeFlag*/)
{
    // Событий Windows здесь нет: очередь принадлежит Qt. Разбираем накопившееся
    // и сообщаем, что сообщений больше нет, — цикл ожидания в коде Windows-версии
    // на этом переходит к следующей итерации.
    if (msg)
        *msg = MSG();
    if (QCoreApplication::instance())
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    return FALSE;
}

BOOL TranslateMessage(const MSG*)
{
    return TRUE;
}

LRESULT DispatchMessage(const MSG*)
{
    return 0;
}

BOOL GetExitCodeThread(HANDLE handle, DWORD* exitCode)
{
    MfcThread* t = threadFor(handle);
    if (!t)
        return FALSE;
    if (exitCode)
        *exitCode = t->isRunning() ? 259u /* STILL_ACTIVE */ : DWORD(t->result());
    return TRUE;
}

// --- Звук ---------------------------------------------------------------------

#ifndef _WIN32 // Windows: настоящий Beep
void Beep(DWORD /*frequencyHz*/, DWORD /*durationMs*/)
{
    // Windows-версия подаёт сигнал об окончании шага и всего расчёта тоном
    // заданной частоты. Ни Qt, ни звуковая подсистема Linux не дают простого
    // способа сыграть чистый тон без внешних зависимостей, поэтому подаём
    // системный сигнал: событие для пользователя то же — «расчёт закончился».
    if (QCoreApplication::instance())
        QApplication::beep();
}
#endif

#endif // !_WIN32
