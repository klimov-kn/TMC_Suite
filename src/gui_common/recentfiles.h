// recentfiles.h — список недавно открытых файлов, как в меню File Windows-версии.
//
// В MFC каркас сам вёл этот список (пункт «Recent File») и хранил его в реестре.
// Здесь то же самое: список хранится в настройках программы
// (`~/.config/TMC_Suite/<программа>.conf`, тот же файл, что и остальные
// настройки), а пункты вставляются в меню File между разделителями — на том же
// месте, что и на Windows.

/// @file recentfiles.h
/// @brief Список недавно открытых файлов в меню «File» (аналог MRU каркаса MFC).
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_gui_common`. Список
/// хранится в настройках программы (`CWinApp::GetProfileString`): Windows —
/// `%APPDATA%\TMC_Suite_Qt\<программа>.conf`, Linux — `~/.config/TMC_Suite/<программа>.conf`,
/// раздел `Recent File List`, ключи `File1…FileN`.
/// @see https://www.tamic.ru/docs/code/gui-common.html#recent-files
#pragma once

#include <QObject>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
QT_END_NAMESPACE

/// @brief Список недавних файлов: хранение в настройках и пункты меню.
///
/// Создаётся оболочкой программы (обычно дочерний объект главного окна).
/// Пути хранятся абсолютными, с пометкой `utf8:`; повторы (без учёта регистра в
/// Windows) убираются. Только поток интерфейса.
/// @see https://www.tamic.ru/docs/code/gui-common.html#recent-files
class TmcRecentFiles : public QObject
{
    Q_OBJECT

public:
    /// @brief Создаёт пустой список (до 4 файлов).
    /// @param[in] parent  владелец
    explicit TmcRecentFiles(QObject *parent = nullptr);


    /// Прочитать список из настроек программы и обновить меню.
    void load();

    /// Добавить файл наверх списка (и сохранить).
    /// @param[in] path  путь к файлу; пустой — ничего; приводится к абсолютному
    void add(const QString &path);

    /// Встроить пункты в меню. Меню запоминается: при изменении списка пункты
    /// обновляются сами.
    /// @param[in] menu    меню «File»
    /// @param[in] before  пункт, перед которым вставлять (NULL — в конец меню)
    void attachToMenu(QMenu *menu, QAction *before = nullptr);

    /// @brief Возвращает список путей, первый — самый свежий.
    /// @return Копия списка путей; пуст — файлов нет.
    QStringList files() const { return m_files; }

signals:
    /// Пользователь выбрал файл из списка.
    /// @param[in] path  абсолютный путь выбранного файла
    void fileRequested(const QString &path);

private:
    /// @brief Записывает список в настройки (все `m_maxCount` ключей; пустые — пустыми строками).
    void save();
    /// @brief Пересоздаёт пункты в меню; при пустом списке — недоступный пункт «Recent File».
    void rebuildMenu();

    /// Пути, первый — самый свежий.
    QStringList     m_files;
    /// Наибольшее число файлов.
    int             m_maxCount = 4;
    /// Меню, в которое вставляются пункты (не владеет).
    QMenu          *m_menu = nullptr;
    /// Пункт меню, перед которым вставлять.
    QAction        *m_before = nullptr;
    /// Вставленные пункты (владеет).
    QList<QAction*> m_actions;
};
