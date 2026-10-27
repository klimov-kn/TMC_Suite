/// @file kernelnames.h
/// @brief Имя счётного ядра и строка его запуска на текущей системе (для подсказок человеку).
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса. Одна из трёх
/// платформенных веток программы; в файл задания эти имена не попадают.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-run
#pragma once

#include "project.h"

#include <QString>

/// Как называется счётное ядро НА ЭТОЙ системе.
///
/// В самом файле задания имя пишется без расширения — файл обязан быть одинаковым
/// везде (см. пояснение в `emitter.cpp`, функция header). А вот подсказка «чем это
/// считать» должна называть то, что человек действительно найдёт у себя: на Windows
/// это `planarrt_h.exe` (в прежних сборках `tmc_rth.exe`), на macOS пакет `PlanarRT_H.app`, на Linux `planarrt_h`.
///
/// Это одна из трёх платформенных веток программы (см. README.md), и живёт она в
/// одном месте: мастер и библиотека образцов советуют одно и то же.
/// @brief Возвращает имя исполняемого файла (пакета) ядра H или X на текущей системе.
/// @param[in] k  ядро задания: `tpl::Kernel::H` или `tpl::Kernel::X`.
/// @return `planarrt_h.exe`/`planarrt_x.exe` (Windows), `PlanarRT_H.app`/`PlanarRT_X.app`
///         (macOS) или `planarrt_h`/`planarrt_x` (Linux).
inline QString kernelProgram(tpl::Kernel k)
{
    const bool x = (k == tpl::Kernel::X);
#if defined(Q_OS_WIN)
    // Выпуск пакета на Qt6 (с 26.10.2026): ядра planarrt_*.exe, как и на Linux. Прежние
    // сборки называли их tmc_rth.exe / tmc_rtx.exe — пакетный файл конвейера ищет оба.
    return x ? QStringLiteral("planarrt_x.exe") : QStringLiteral("planarrt_h.exe");
#elif defined(Q_OS_MACOS)
    return x ? QStringLiteral("PlanarRT_X.app") : QStringLiteral("PlanarRT_H.app");
#else
    return x ? QStringLiteral("planarrt_x") : QStringLiteral("planarrt_h");
#endif
}

/// Строка запуска для подсказки после сохранения. На macOS программу из пакета
/// запускают через `open`, поэтому и строка другая — иначе совет не сработает.
///
/// \p flags — ключи командной строки. Их состав от системы не зависит и приходит из
/// ядра генератора (`tpl::runFlags`): туда же попадает `/Time`, когда человек просит
/// замер времени расчёта. В ЯЗЫКЕ задания таких ключей нет, и выдумывать их нельзя —
/// поэтому единственное место, где они живут, это строка запуска.
/// @brief Составляет командную строку запуска ядра на задание для подсказки человеку.
/// @param[in] k      ядро задания (H или X).
/// @param[in] name   имя задания без расширения; к нему дописывается `.tpl`.
/// @param[in] flags  ключи ядра (`/Arb`, `/Arb /Time`), обычно из `tpl::runFlags`.
/// @return Строка вида `planarrt_h.exe wg.tpl /Arb`; на macOS — `open -a PlanarRT_H.app --args wg.tpl /Arb`.
inline QString kernelRunCommand(tpl::Kernel k, const QString &name, const QString &flags)
{
#if defined(Q_OS_MACOS)
    return QStringLiteral("open -a %1 --args %2.tpl %3").arg(kernelProgram(k), name, flags);
#else
    return QStringLiteral("%1 %2.tpl %3").arg(kernelProgram(k), name, flags);
#endif
}

/// @brief Составляет строку запуска ядра с ключом пакетного режима `/Arb`.
/// @param[in] k     ядро задания (H или X).
/// @param[in] name  имя задания без расширения.
/// @return То же, что `kernelRunCommand(k, name, "/Arb")`.
inline QString kernelRunLine(tpl::Kernel k, const QString &name)
{
    return kernelRunCommand(k, name, QStringLiteral("/Arb"));
}
