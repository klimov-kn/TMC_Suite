/// @file recentfiles.cpp
/// @brief Реализация списка недавних файлов.
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_gui_common`.
/// @see https://www.tamic.ru/docs/code/gui-common.html#recent-files
#include "recentfiles.h"

#include <QAction>
#include <QFileInfo>
#include <QMenu>

#include "tmc_mfc_doc.h"

namespace {

// Раздел и ключи в настройках — как у MFC: «Recent File List» и «File1»…«FileN».
/// Раздел настроек списка.
const char *kSection = "Recent File List";

/// @brief Возвращает ключ настроек `File<index+1>`.
/// @param[in] index  номер, с 0
/// @return Строка `File1`, `File2` …
QString keyFor(int index)
{
    return QStringLiteral("File%1").arg(index + 1);
}

// Путь хранится в UTF-8 с пометкой: в кодовой странице Windows теряются буквы
// вне неё (Antenna_Ω_中.tpl -> Antenna_?_?.tpl, Т-20). Старые записи без пометки
// — в кодовой странице, как писали прежние версии.
/// Пометка пути, записанного в UTF-8.
const char kUtf8Mark[] = "utf8:";

/// @brief Читает путь из настроек: с пометкой — UTF-8, без неё — локальная кодовая страница.
/// @param[in] stored  значение из настроек
/// @return Путь; пустой — записи нет.
QString decodePath(const char *stored)
{
    const QByteArray raw(stored);
    if (raw.startsWith(kUtf8Mark))
        return QString::fromUtf8(raw.mid(int(sizeof(kUtf8Mark)) - 1));
    return QString::fromLocal8Bit(raw);
}

/// @brief Готовит путь к записи: пометка `utf8:` + UTF-8.
/// @param[in] path  путь
/// @return Значение для настроек; пустое для пустого пути.
QByteArray encodePath(const QString &path)
{
    if (path.isEmpty())
        return QByteArray();
    return QByteArray(kUtf8Mark) + path.toUtf8();
}

// Один и тот же файл в Windows может быть записан в разном регистре и с разными
// разделителями — это одна запись (как AfxComparePath в MFC, Т-17).
/// @brief Сравнивает абсолютные пути (в Windows — без учёта регистра).
/// @param[in] a  первый путь
/// @param[in] b  второй путь
/// @return true — один и тот же файл.
bool samePath(const QString &a, const QString &b)
{
#ifdef _WIN32
    return QFileInfo(a).absoluteFilePath().compare(QFileInfo(b).absoluteFilePath(),
                                                   Qt::CaseInsensitive) == 0;
#else
    return QFileInfo(a).absoluteFilePath() == QFileInfo(b).absoluteFilePath();
#endif
}

} // namespace

TmcRecentFiles::TmcRecentFiles(QObject *parent)
    : QObject(parent)
{
}


void TmcRecentFiles::load()
{
    m_files.clear();
    for (int i = 0; i < m_maxCount; ++i) {
        const CString value = AfxGetApp()->GetProfileString(
            kSection, keyFor(i).toLocal8Bit().constData(), "");
        const QString path = decodePath(value.GetString());
        if (path.isEmpty())
            continue;
        bool dup = false;
        for (const QString &f : m_files)
            dup = dup || samePath(f, path);
        if (!dup)
            m_files.append(path);
    }
    rebuildMenu();
}

void TmcRecentFiles::save()
{
    for (int i = 0; i < m_maxCount; ++i) {
        const QString path = i < m_files.size() ? m_files.at(i) : QString();
        AfxGetApp()->WriteProfileString(kSection,
                                        keyFor(i).toLocal8Bit().constData(),
                                        encodePath(path).constData());
    }
}

void TmcRecentFiles::add(const QString &path)
{
    if (path.isEmpty())
        return;
    const QString full = QFileInfo(path).absoluteFilePath();
    m_files.removeIf([&full](const QString &f) { return samePath(f, full); });
    m_files.prepend(full);
    while (m_files.size() > m_maxCount)
        m_files.removeLast();
    save();
    rebuildMenu();
}

void TmcRecentFiles::attachToMenu(QMenu *menu, QAction *before)
{
    m_menu = menu;
    m_before = before;
    rebuildMenu();
}

void TmcRecentFiles::rebuildMenu()
{
    if (!m_menu)
        return;

    for (QAction *a : m_actions) {
        m_menu->removeAction(a);
        delete a;
    }
    m_actions.clear();

    if (m_files.isEmpty()) {
        // В Windows-версии при пустом списке пункт «Recent File» показан, но
        // недоступен — повторяем, чтобы меню не «прыгало».
        QAction *empty = new QAction(QStringLiteral("Recent File"), m_menu);
        empty->setEnabled(false);
        m_menu->insertAction(m_before, empty);
        m_actions.append(empty);
        return;
    }

    int index = 1;
    for (const QString &path : m_files) {
        // Подпись как в MFC: номер и имя файла.
        QAction *a = new QAction(QStringLiteral("&%1 %2")
                                     .arg(index++)
                                     .arg(QFileInfo(path).fileName()
                                              .replace(QLatin1Char('&'), QStringLiteral("&&"))),
                                 m_menu);
        a->setToolTip(path);
        connect(a, &QAction::triggered, this, [this, path]() { emit fileRequested(path); });
        m_menu->insertAction(m_before, a);
        m_actions.append(a);
    }
}
