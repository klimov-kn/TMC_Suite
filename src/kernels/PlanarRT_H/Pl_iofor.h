/// @file Pl_iofor.h
/// @brief Форматы чисел в выходных файлах ядра, режимы запуска (/Ar, /Arb, /d, /Time),
///        код завершения процесса и вывод в консоль.
///
/// Часть модуля «Счётное ядро H» (тот же файл — в ядре X). Функции реализованы
/// в `pl_iofor.cpp` и хранят состояние в статических переменных процесса.
/// @see https://www.tamic.ru/docs/code/kernel-h.html#flags
#include "MainFrm.h"
#include "PlanRT_HDoc.h"
#include "PlanRT_HView.h"


#ifndef _TMCRTH_IOFORMAT__H
#define _TMCRTH_IOFORMAT__H

/// Длина буфера строки формата вывода (символов, с завершающим нулём).
#define MAX_LEN_FORMAT_OUT_FILE			100
/// @name Форматы вывода по умолчанию (строки printf)
/// Номер такта, время (с), номер блока, падающая и отражённая волны в `.t`;
/// пустой формат поля — значения пишутся в двоичном виде.
///@{
/// Формат номера такта в `.t` по умолчанию.
#define FORMAT_OUT_FILE_NTFORMAT		"%9d"
/// Формат времени подтакта (с) в `.t` по умолчанию.
#define FORMAT_OUT_FILE_DTCurrentFORMAT "%15.7g"
/// Формат номера блока-порта в `.t` по умолчанию.
#define FORMAT_OUT_FILE_NBFORMAT		"%4d"
/// Формат падающей волны Up в `.t` по умолчанию.
#define FORMAT_OUT_FILE_DINPFORMAT		"%15.7g"
/// Формат отражённой волны Uo в `.t` по умолчанию.
#define FORMAT_OUT_FILE_DOUTFORMAT		"%15.7g"
/// Формат значений поля и топологии по умолчанию: пусто — двоичная запись.
#define FORMAT_OUT_FILE_DFLDOUTFORMAT	""

///@}

/// @brief Задаёт формат номера такта в `.t`; неверный формат заменяется форматом по умолчанию.
/// @param[in] csBuf строка printf ровно с одним целым преобразованием (d i u x X o)
void SetnTFormat( CString csBuf );
/// @brief Возвращает формат номера такта.
/// @return Строка формата (статический буфер).
char *GetnTFormat( void );
/// @brief Задаёт формат времени подтакта в `.t`.
/// @param[in] csBuf строка printf ровно с одним вещественным преобразованием (e E f F g G)
void SetdTcurrentFormat( CString csBuf );
/// @brief Возвращает формат времени подтакта.
/// @return Строка формата (статический буфер).
char *GetdTcurrentFormat( void );
/// @brief Задаёт формат номера блока в `.t`.
/// @param[in] csBuf строка printf с одним целым преобразованием
void SetnBFormat( CString csBuf );
/// @brief Возвращает формат номера блока.
/// @return Строка формата (статический буфер).
char *GetnBFormat( void );
/// @brief Задаёт формат падающей волны Up в `.t`.
/// @param[in] csBuf строка printf с одним вещественным преобразованием
void SetdInpFormat( CString csBuf );
/// @brief Возвращает формат падающей волны.
/// @return Строка формата (статический буфер).
char *GetdInpFormat( void );
/// @brief Задаёт формат отражённой волны Uo в `.t`.
/// @param[in] csBuf строка printf с одним вещественным преобразованием
void SetdOutFormat( CString csBuf );
/// @brief Возвращает формат отражённой волны.
/// @return Строка формата (статический буфер).
char *GetdOutFormat( void );
/// @brief Возвращает все форматы вывода к значениям по умолчанию.
void SetDefaultOutputFormat( void );
/// @brief Возвращает формат значений поля и топологии.
/// @return Строка формата; пустая — двоичный вывод.
char *GetdFieldOutFormat( void );
/// @brief Задаёт формат значений поля и топологии; пустая строка — двоичный вывод.
/// @param[in] csBuf строка printf с одним вещественным преобразованием или пусто
void SetdFieldOutFormat( CString csBuf );
/// @brief Включает режим автозапуска (ключ /Ar): консольный вывод, автоматическое закрытие.
void SetAutoRunOn( void );
/// @brief Выключает режим автозапуска.
void SetAutoRunOff( void );
/// @brief Сообщает, включён ли автозапуск.
/// @return TRUE — ядро запущено с /Ar, /Arb или /d.
BOOL IsAutoRun( void );
/// @brief Включает демонстрационный режим (ключ /d): после каждого кадра поля открывается его просмотр.
void SetDemoRunOn( void );
/// @brief Выключает демонстрационный режим.
void SetDemoRunOff( void );
/// @brief Сообщает, включён ли демонстрационный режим.
/// @return TRUE — ключ /d.
BOOL IsDemoRun( void );
/// @brief Разбирает командную строку: ключи /Ar, /Arb, /d, /Time (также с `-`; регистр не важен;
///        ключ — отдельное слово вне кавычек) и задаёт начальный код завершения.
/// @param[in] lpszCmdLine командная строка процесса
void Set_CommandLine_Flags( char *lpszCmdLine );
/// @brief Переключает режим автозапуска.
void SwitchAutoRunFlag( void );
/// @brief Возвращает сохранённый указатель на главное окно.
/// @return Указатель; NULL — не задан.
CMainFrame *GetMainFramePointer__( void );
/// @brief Запоминает указатель на главное окно.
/// @param[in] HwndMainFrame1 главное окно (не владеет)
void SetMainFramePointer__( CMainFrame* HwndMainFrame1 );
/// @brief Включает пакетный режим (ключ /Arb): без звука и диалогов.
void SetBatchRunOn( void );
/// @brief Сообщает, включён ли пакетный режим.
/// @return TRUE — ключ /Arb или /d.
BOOL IsBatchRun( void );
/// @brief Включает запись отчёта о времени расчёта (ключ /Time).
/// @see https://www.tamic.ru/docs/code/kernel-h.html#timing
void SetTimeReportOn( void );	// TIMING: key /Time - calculation time report file
/// @brief Выключает запись отчёта о времени расчёта.
void SetTimeReportOff( void );
/// @brief Сообщает, нужен ли отчёт о времени расчёта.
/// @return TRUE — ключ /Time.
BOOL IsTimeReport( void );
/// @brief Запоминает сведения о запущенном внешнем процессе.
/// @param[in] piProcInfo1 сведения о процессе
void SetProcessInformation( PROCESS_INFORMATION piProcInfo1 );
/// @brief Запоминает вид документа ядра для демонстрационного режима.
/// @param[in] pcView1 вид (не владеет)
void SetPointerClassView( CPlanRT_HView* pcView1 );
/// @brief Открывает просмотр поля через сохранённый вид (демонстрационный режим).
void RunOnViewField1( void );

// RUN STATUS: process exit code and console messages for the command line run
// (keys /Ar, /Arb). The exit code is returned by CPlanRT_HApp::ExitInstance.
/// @name Коды завершения процесса ядра (ERRORLEVEL)
///@{
/// Код 0: все шаги задания посчитаны.
#define TMC_EXIT_OK				0	// all steps of the .tpl are calculated
/// Код 1: задание не найдено или ошибка в задании.
#define TMC_EXIT_TPL_ERROR		1	// .tpl not found or error in .tpl (preprocessor, data)
/// Код 2: ошибка при расчёте шага.
#define TMC_EXIT_CALC_ERROR		2	// error during the calculation of a step
/// Код 3: расчёт остановлен пользователем или окно закрыто до конца.
#define TMC_EXIT_ABORTED		3	// stopped by the user / window closed before the end
///@}

/// @brief Задаёт код завершения процесса.
/// @param[in] nCode один из `TMC_EXIT_*`
void SetRunExitCode( int nCode );
/// @brief Возвращает код завершения процесса.
/// @return Один из `TMC_EXIT_*`.
int  GetRunExitCode( void );
/// @brief Запоминает последнее предупреждение расчёта (показывается снова в конце).
/// @param[in] pszText текст; NULL или "" — очистить
void SetRunWarning( const char *pszText );	// the last warning of the run ("" - clear)
/// @brief Возвращает последнее предупреждение расчёта.
/// @return Текст; "" — предупреждений не было.
const char *GetRunWarning( void );			// "" - the run had no warnings
/// @brief Запоминает главное окно для закрытия после автозапуска.
/// @param[in] hWnd окно
void SetRunMainWnd( HWND hWnd );
/// @brief Отмечает, что окно уже закрывается: `PostRunClose` больше не посылает WM_CLOSE.
void SetRunClosing( void );
/// @brief Посылает главному окну WM_CLOSE после автозапуска (если окно ещё есть и не закрывается).
void PostRunClose( void );
/// @brief Пишет строку «[имя программы] текст» в консоль или перенаправленный вывод (только при автозапуске).
/// @param[in] pszFormat формат printf
/// @param[in] ... аргументы формата
void ConsolePrint( const char *pszFormat, ... );
/// @brief Пишет в консоль итог «RESULT n: ...» по коду завершения.
void ConsoleReportResult( void );

#endif