/// @file usersamples.h
/// @brief «Мои образцы»: сохранение заданий пользователя в библиотеку, список, чтение, удаление.
///
/// Часть модуля «Ядро TPL Designer», каталог образцов. Реализация — `usersamples.cpp`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#samples
#pragma once

#include "sample.h"
#include "taskfiles.h"

#include <QDateTime>
#include <QString>
#include <QVector>

/// «Мои образцы»: задания пользователя, положенные в библиотеку.
///
/// Каждый образец — своя папка в `Документы/TPL Designer/Мои образцы/`: там лежит
/// задание ровно так, как его запишет программа (`.tpl`, `_step.h`, `.eps`), и
/// `sample.json` с названием и описанием. Задание — обычный файл: его можно
/// открыть ядром напрямую, переслать коллеге, положить в чужую папку «Мои образцы».
namespace tpl::samples {

/// @brief Запись списка «Моих образцов»: папка, задание и данные из `sample.json`.
struct UserSample {
    QString dir;      ///< папка образца
    QString tplPath;  ///< задание внутри неё
    /// Название: из `sample.json`, иначе имя папки.
    QString title;
    /// Описание из `sample.json` (может быть пустым).
    QString summary;
    /// Ядро из `sample.json` («X» — X, иначе H).
    Kernel kernel = Kernel::H;
    /// Время сохранения из `sample.json`, иначе время изменения `.tpl`.
    QDateTime created;
};

/// Корень «Моих образцов». Переменная окружения TPLD_USER_SAMPLES подменяет его —
/// для проверок, чтобы не трогать настоящие Документы.
/// @return Путь к папке `Документы/TPL Designer/Мои образцы` или значение переменной.
QString userSamplesRoot();

/// Все образцы из корня, новые сверху.
///
/// Образцом считается подпапка, в которой есть хотя бы один `.tpl`; берётся первый
/// по списку каталога.
/// @return Список образцов, отсортированный по `created` по убыванию.
QVector<UserSample> userSamples();

/// Положить задание в «Мои образцы». \p dirOut — куда легло.
///
/// Папка называется по названию (запрещённые в именах символы -> `_`, не длиннее 60);
/// если она уже есть, добавляется « (2)», « (3)» … — чужой образец не перезаписывается.
/// Задание пишет `writeTask`, затем рядом кладётся `sample.json`.
/// @param[in]  title       название образца
/// @param[in]  summary     описание
/// @param[in]  p           задание
/// @param[in]  companions  сопутствующие файлы (имя -> байты), кладутся рядом с заданием
/// @param[out] dirOut      папка образца; `nullptr` — не нужна; заполняется только при успехе
/// @return Результат записи; `written` дополнен `sample.json`.
WriteResult saveUserSample(const QString &title, const QString &summary, const Project &p,
                           const QMap<QString, QByteArray> &companions, QString *dirOut = nullptr);

/// Прочитать образец: задание, его сопутствующие файлы и паспорт.
///
/// Задание читается `readTplFile`; замечания и ошибки чтения уходят в `Build::notes`.
/// Сопутствующими считаются файлы `.eps` из папки образца. Паспорт — папка, число
/// шагов, число фигур и входов, время сохранения.
/// @param[in] s  запись из `userSamples()`
/// @return Задание с паспортом.
Build loadUserSample(const UserSample &s);

/// Удалить образец вместе с папкой.
/// @param[in] s  запись из `userSamples()`
/// @return true — папка удалена; false — папка лежит вне `userSamplesRoot()` или удалить
///         не удалось.
bool removeUserSample(const UserSample &s);

} // namespace tpl::samples
