// planrt_compat.cpp — то, чего оболочкам счётных ядер не хватает в слое
// совместимости (см. planrt_prelude.h).
//
// Здесь нет Qt: файл входит в счётное ядро программы, которое собирается и
// проверяется без графики. Всё, что должно дойти до интерфейса (окно с
// вопросом, просьба закрыть главное окно), уходит через обработчики —
// Qt-слой ставит свои при старте, а без него работают безопасные значения по
// умолчанию, годные для прогона из консоли.

/// @file planrt_compat.cpp
/// @brief Реализация недостающего в слое совместимости для оболочек ядер: WIN.INI, время, связь ядра с Qt-слоем.
///
/// Часть модуля «Оболочка счётного ядра». Входит в библиотеку ядра (`planarrt_*_core`)
/// и собирается без Qt. Объявления — `planrt_prelude.h`.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#prelude
#include <chrono>
#include <cstdio>

#include "planrt_prelude.h"

// --- WIN.INI ------------------------------------------------------------------
#ifndef _WIN32
DWORD TmcGetWinIniProfileString(const char* /*section*/, const char* /*entry*/,
                                const char* defaultValue, char* buffer, DWORD size)
{
    if (!buffer || size == 0)
        return 0;

    const char* src = defaultValue ? defaultValue : "";
    DWORD n = 0;
    while (src[n] && n + 1 < size) {
        buffer[n] = src[n];
        n++;
    }
    buffer[n] = '\0';
    return n;
}

// --- Время --------------------------------------------------------------------
#endif


// --- Обработчики, которые ставит Qt-слой --------------------------------------
namespace {

/// Обработчик окна сообщения, поставленный Qt-слоем; 0 — не поставлен.
TmcPlanRtMessageBoxFn s_messageBox = 0;
/// Обработчик закрытия главного окна, поставленный Qt-слоем; 0 — закрытие не выполняется.
TmcPlanRtCloseFn      s_closeFrame = 0;

} // namespace

void TmcPlanRtSetMessageBoxHandler(TmcPlanRtMessageBoxFn fn) { s_messageBox = fn; }
void TmcPlanRtSetCloseHandler(TmcPlanRtCloseFn fn)           { s_closeFrame = fn; }

// Окно сообщения с кнопками переехало в общий слой совместимости
// (src/compat/mfc_dialog.cpp): там оно показывает обычное окно Qt, а без
// графики — например, в средстве проверки счёта — сообщает в поток ошибок и
// отвечает так же, как пакетный режим Windows-версии.

void TmcPostFrameClose(UINT message)
{
    if (s_closeFrame)
        s_closeFrame(message);
}

#ifdef _WIN32
// Windows: ядро после пакетного расчёта зовёт ::PostMessage( главное окно,
// WM_CLOSE ) (PostRunClose, pl_iofor.cpp) — передаём это закрытию окна Qt.
namespace {
/// @brief Перехват `::PostMessage` кода ядра (Windows): `WM_CLOSE` главному окну -> закрытие окна Qt.
///
/// Окно-получатель и параметры сообщения не используются.
/// @param[in] message код сообщения; обрабатывается только `WM_CLOSE`.
/// @return TRUE — сообщение принято; FALSE — сообщение не `WM_CLOSE`.
BOOL TmcPlanRtPostMessage(HWND, UINT message, WPARAM, LPARAM)
{
    if (message != WM_CLOSE)
        return FALSE;
    TmcPostFrameClose(message);
    return TRUE;
}
/// @brief Статический объект, при загрузке программы ставящий перехватчик `PostMessage` слоя совместимости.
struct TmcPlanRtPostInstaller
{
    /// @brief Ставит `TmcPlanRtPostMessage` обработчиком `PostMessage` слоя совместимости.
    TmcPlanRtPostInstaller() { tmc_set_post_message_handler(&TmcPlanRtPostMessage); }
} s_planRtPostInstaller;
/// @var s_planRtPostInstaller
/// Объект-установщик: его конструктор при загрузке программы ставит перехват `PostMessage`.
} // namespace
#endif
