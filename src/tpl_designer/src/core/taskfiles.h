/// @file taskfiles.h
/// @brief Запись задания на диск (`writeTask`): `.tpl`, `_step.h`, `.sig`, сопутствующие файлы,
///        пакетные файлы конвейера и строка ключей запуска ядра.
///
/// Часть ядра TPL Designer. Единственный путь записи задания для мастера, библиотеки
/// образцов, «Моих образцов» и служебных утилит.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#task-files
#pragma once

#include "emitter.h" // TaskChunk: конвейер и запись задания — одна и та же работа
#include "project.h"

#include <QByteArray>
#include <QMap>
#include <QString>
#include <QStringList>

namespace tpl {

/// Итог записи задания на диск.
/// @brief Результат `writeTask`: успех, список записанных файлов, ошибка, признак потери символов.
struct WriteResult {
    /// true — все файлы записаны.
    bool ok = false;
    QStringList written; ///< имена записанных файлов, первым — .tpl
    /// Причина отказа (пусто при успехе).
    QString error;
    bool lossy = false;  ///< часть символов не легла в CP1251 и заменена на '?'
};

/// Записать задание в папку ровно так, как его ждёт ядро: `.tpl` (и `_step.h` у
/// макро-версии) в CP1251 рядом друг с другом, плюс сопутствующие файлы
/// (готовые распределения `.eps`) байт в байт.
///
/// Одно место для мастера, библиотеки образцов и «Моих образцов»: иначе рано или
/// поздно один из путей забыл бы положить рядом `_step.h` или файл распределения.
/// @param[in] dir         папка задания (создаётся при необходимости).
/// @param[in] p           проект.
/// @param[in] companions  готовые сопутствующие файлы: имя -> байты (пишутся как есть).
/// @return Итог; при ошибке записи — `ok == false` и уже записанные файлы в `written`.
/// @note `lossy == true` не останавливает запись: решение — за вызывающим.
WriteResult writeTask(const QString &dir, const Project &p,
                      const QMap<QString, QByteArray> &companions = {});

/// Какие файлы появятся в папке: для вопроса «перезаписать?» до записи.
/// @param[in] p           проект.
/// @param[in] companions  сопутствующие файлы, которые будут записаны вместе с заданием.
/// @return Имена всех файлов, которые запишет `writeTask` (задания, `_step.h`, пакетные, `.sig`, сопутствующие).
QStringList taskFileNames(const Project &p, const QMap<QString, QByteArray> &companions = {});

/// Строка запуска ядра для подсказки человеку. Ключ `/Time` добавляется, если
/// пользователь попросил замер времени.
///
/// ⚠️ Имя исполняемого файла зависит от системы, поэтому строку составляет GUI
/// (`kernelnames.h`), а здесь — только хвост из ключей: он одинаков везде.
/// @param[in] p  проект (`output.timingReport`).
/// @return `/Arb` или `/Arb /Time`.
QString runFlags(const Project &p);

/// Пакетный файл, прогоняющий все задания конвейера по очереди.
///
/// Пишется рядом с заданиями, когда их больше одного. Два файла сразу — `.bat` для
/// Windows и `.sh` для Linux/macOS: папку с заданиями носят между машинами, и
/// угадывать, где её будут считать, незачем.
/// @brief Составляет пакетный файл Windows для конвейера заданий.
///
/// Перед каждым заданием удаляет его файлы `.s`, затем запускает ядро (`KERNEL`, по
/// умолчанию planarrt_h.exe/planarrt_x.exe, при отсутствии — tmc_rth.exe/tmc_rtx.exe).
/// @param[in] p       проект (ядро, частоты, ключи запуска).
/// @param[in] chunks  задания конвейера (`emitChunks`).
/// @return Текст `.bat` с переводами строк LF (CRLF — при записи).
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#chunks
QString runBat(const Project &p, const QVector<TaskChunk> &chunks);
/// @copydoc runBat
/// @note Вариант для Linux/macOS (`sh`, ядро planarrt_h/planarrt_x, переменная `KERNEL`).
QString runSh(const Project &p, const QVector<TaskChunk> &chunks);

} // namespace tpl
