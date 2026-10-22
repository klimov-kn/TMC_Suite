// tmc_winapi.h — настоящий Windows API под слоем MFC (сборка Windows, MinGW + Qt).
//
// На Linux слой совместимости подменял и Windows API, и MFC. Под Windows
// API настоящий: время, память, потоки, консоль, загрузка OpenCL.dll — всё
// берётся из системы. Слой даёт только классы MFC (CString, CDC, CWnd, CDialog…),
// которых в MinGW нет.
//
// Несколько функций Windows API слой обязан перехватывать и здесь: они
// работают с объектами самого слоя, а не системы —
//   * ожидание и код завершения потока CWinThread (поток слоя — QThread);
//   * прокачка очереди сообщений (очередь — это события Qt);
//   * сведения об устройстве рисования (контекст рисования — QPainter);
//   * запуск соседних программ пакета (имена программ Qt-версии другие).
// Они переименовываются в Tmc* макросами ниже; код Windows-версии остаётся
// прежним, а вызов попадает в слой.

/// @file tmc_winapi.h
/// @brief Подключение настоящего Windows API под слоем MFC и перехват функций, работающих с объектами слоя.
///
/// Часть модуля «Слой совместимости». Только для сборки под Windows (MinGW + Qt).
/// Функции `PeekMessage`, `WaitForSingleObject`, `GetDeviceCaps`, `CreateProcess`,
/// `PostMessage`, `IsWindow` и др. макросами переименовываются в `Tmc*`, которые
/// реализованы в слое (`mfc_thread.cpp`, `mfc_gdi.cpp`, `tmc_mfc_doc.cpp`).
/// @see https://www.tamic.ru/docs/code/compat.html#winapi
#pragma once

#ifdef _WIN32

#ifndef NOMINMAX
  /// Запрещает макросы `min`/`max` из `windows.h`.
  #define NOMINMAX            // min/max из windows.h ломают заголовки Qt и STL
#endif

// INPUT занят структурой задания TMC (include/run.h); в winuser.h так же
// называется структура SendInput, которой пакет не пользуется. Уводим её имя.
/// Временно переименовывает `INPUT` из `winuser.h`, чтобы не конфликтовать со структурой задания `INPUT` (run.h).
#define INPUT TMC_WINUSER_INPUT
#include <windows.h>
#undef INPUT
#include <tchar.h>    // _T
#include <time.h>

#undef PeekMessage
#undef DispatchMessage
#undef CreateProcess
/// Перехваты Windows API: вызовы уходят в реализацию слоя (очередь событий Qt, `QThread`, `QPainter`).
#define PeekMessage               TmcPeekMessage
/// Перехват: реализация `mfc_thread.cpp`.
#define TranslateMessage          TmcTranslateMessage
/// Перехват: реализация `mfc_thread.cpp`.
#define DispatchMessage           TmcDispatchMessage
/// Перехват: ожидание потока слоя (`mfc_thread.cpp`).
#define WaitForSingleObject       TmcWaitForSingleObject
/// Перехват: ожидание с прокачкой событий Qt (`mfc_thread.cpp`).
#define MsgWaitForMultipleObjects TmcMsgWaitForMultipleObjects
/// Перехват: код завершения потока слоя (`mfc_thread.cpp`).
#define GetExitCodeThread         TmcGetExitCodeThread
/// Перехват: сведения об устройстве Qt (`mfc_gdi.cpp`).
#define GetDeviceCaps             TmcGetDeviceCaps
/// Перехват: запуск программ пакета под именами Qt-версии (`tmc_mfc_doc.cpp`).
#define CreateProcess             TmcCreateProcess
// Окна слоя — окна Qt, а не Windows: «есть ли окно» и «послать окну сообщение»
// (ядро закрывает главное окно после пакетного расчёта, PostRunClose) — в слой.
#undef PostMessage
/// Перехват: сообщение окну слоя (`mfc_view.cpp`).
#define PostMessage               TmcPostMessage
/// Перехват: проверка описателя окна слоя (`mfc_view.cpp`).
#define IsWindow                  TmcIsWindow
/// @brief Проверяет описатель окна слоя (аналог `IsWindow`).
/// @param[in] h  описатель, выданный `CWnd::GetSafeHwnd` (указатель на объект слоя)
/// @return TRUE — описатель не нулевой (существование окна не проверяется); FALSE — `h == NULL`.
BOOL TmcIsWindow(HWND h);
/// @brief Ставит оконное сообщение в очередь окна слоя (аналог `PostMessage`).
///
/// Сообщение передаётся обработчику, установленному `tmc_set_post_message_handler`;
/// без обработчика вызов ничего не делает.
/// @param[in] h        описатель окна слоя
/// @param[in] message  код сообщения (`WM_CLOSE`, `WM_COMMAND` …)
/// @param[in] wParam   первый параметр сообщения
/// @param[in] lParam   второй параметр сообщения
/// @return TRUE — сообщение принято обработчиком; FALSE — обработчика нет или он отказал.
BOOL TmcPostMessage(HWND h, UINT message, WPARAM wParam, LPARAM lParam);
/// Тип обработчика `TmcPostMessage`; параметры и результат — как у `PostMessage`.
typedef BOOL (*TmcPostMessageHandler)(HWND h, UINT message, WPARAM wParam, LPARAM lParam);
/// @brief Устанавливает обработчик для `TmcPostMessage` (ставит оболочка программы).
/// @param[in] fn  обработчик; `nullptr` — снять
void tmc_set_post_message_handler(TmcPostMessageHandler fn);

// Запуск программы: имя соседней программы пакета («TMCROS.EXE») переводится в
// имя Qt-версии, рядом с текущей программой (tmc_mfc_doc.cpp).
/// @brief Запускает программу; имена соседних программ пакета заменяет на имена Qt-версии.
///
/// Параметры повторяют `CreateProcessA`. Имя вида `TMCROS.EXE` переводится в имя
/// исполняемого файла Qt-версии, лежащего рядом с текущей программой.
/// @param[in]     appName         путь к программе или NULL (тогда берётся из `commandLine`)
/// @param[in,out] commandLine     командная строка
/// @param[in]     processAttrs    атрибуты защиты процесса (передаются системе)
/// @param[in]     threadAttrs     атрибуты защиты потока (передаются системе)
/// @param[in]     inheritHandles  наследовать ли описатели
/// @param[in]     creationFlags   флаги создания процесса
/// @param[in]     environment     окружение или NULL
/// @param[in]     currentDir      рабочий каталог или NULL
/// @param[in]     startupInfo     параметры окна процесса
/// @param[out]    processInfo     описатели созданного процесса и потока
/// @return TRUE — процесс запущен.
BOOL TmcCreateProcess(const char* appName, char* commandLine,
                      void* processAttrs, void* threadAttrs, BOOL inheritHandles,
                      DWORD creationFlags, void* environment, const char* currentDir,
                      STARTUPINFOA* startupInfo, PROCESS_INFORMATION* processInfo);

#endif // _WIN32
