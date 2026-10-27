/// @file main.cpp
/// @brief Точка входа TPL Designer: оболочка окна, меню, стек страниц и смена языка на лету.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса. Создаёт приложение,
/// подключает общее оформление пакета (`theme`, `dtheme`), язык (`tmci18n`, `tpl::setEnglish`),
/// сторож колеса мыши, короткое меню «Настройки (язык) · Справка (О программе)» и стек
/// из трёх страниц: стартовое окно, мастер задания, библиотека образцов. При смене языка
/// страницы пересоздаются, задание в работе переносится в новый мастер.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-windows
#include "filemaker.h"
#include "mainwindow.h"
#include "samplespage.h"
#include "settings.h"
#include "startwindow.h"
#include "dtheme.h"
#include "lang.h"
#include "wheelguard.h"

// Общее с программами пакета (gui_common): шрифты, окно «О программе», язык.
#include "aboutbox.h"
#include "i18n/tmci18n.h"
#include "theme.h"

#include <QApplication>
#include <QFileDialog>
#include <QIcon>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>

/// @brief Запускает TPL Designer: настраивает приложение, строит окно и входит в цикл событий.
///
/// Порядок: имя и организация приложения (для `QSettings`), хранилище настроек
/// (`settings::installStorage`), шрифты пакета, язык, таблица стилей `dtheme::styleSheet`,
/// `WheelGuard::install`, значок окна; затем оболочка `QMainWindow` со стеком страниц и
/// связи между страницами (создать, открыть, образцы, вспомогательные файлы, назад).
/// Первый аргумент командной строки, не начинающийся с `-`, открывается как задание.
/// @param[in] argc  число аргументов командной строки.
/// @param[in] argv  аргументы; `argv[1]` — необязательный путь к файлу `.tpl`.
/// @return Код завершения цикла событий Qt (`QApplication::exec`), 0 — штатный выход.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-windows
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("TPL Designer"));
    // Организация и имя нужны QSettings: без них настройки лягут под именем
    // исполняемого файла и потеряются при переименовании.
    app.setOrganizationName(QStringLiteral("TMC"));
    // Windows: настройки — в INI рядом с программами пакета, не в реестре (Т-27).
    settings::installStorage();
    // Linux (Wayland и панели задач GNOME/KDE) связывает окно с ярлыком в меню
    // приложений по этому имени. Без него окно остаётся безымянным квадратом даже
    // при установленном .desktop. На Windows и macOS вызов ничего не делает.
    app.setDesktopFileName(QStringLiteral("tpl-designer"));
    // Шрифты Inter и JetBrains Mono — те же, что вложены в программы пакета: таблица
    // стилей просит Inter, и без загрузки на машине без этого шрифта шёл Segoe UI.
    theme::setProgram(QStringLiteral("tpldesigner"));
    theme::uiFontFamily();
    // Язык интерфейса — общий выбор пакета (Настройки → Language / Язык).
    tmci18n::install();
    tpl::setEnglish(!tmci18n::russian());   // тексты генератора и окон дизайнера
    app.setStyleSheet(dtheme::styleSheet());
    // Колесо мыши листает страницу, а не значения полей, пока поле не выбрано щелчком.
    WheelGuard::install();

    // Значок ОКНА и панели задач. Значок самого .exe в проводнике берётся из ресурса
    // Windows (src/gui/app.rc) — это разные вещи, и нужны обе.
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/resources/app.ico")));

    // Стартовый экран и мастер живут в одном окне: «В начало» возвращает сюда же,
    // не теряя набранного задания.
    //
    // Окно с коротким меню, как у программ пакета: «Настройки» (язык) и «Справка»
    // («О программе»). Меню «Файл» нет намеренно: создание, открытие и запись задания
    // ведут стартовый экран и мастер (решение 27.10.2026).
    QMainWindow shell;
    shell.setWindowTitle(QStringLiteral("TPL Designer"));
    auto *rootWidget = new QStackedWidget;
    shell.setCentralWidget(rootWidget);
    QStackedWidget &root = *rootWidget;

    // Названия меню переводит словарь пакета («&Настройки» <-> «&Config»), пункт
    // «О программе» — свой текст на двух языках.
    QMenu *config = shell.menuBar()->addMenu(QStringLiteral("&Настройки"));
    QMenu *languageMenu = tmci18n::addLanguageMenu(config);
    QMenu *help = shell.menuBar()->addMenu(QStringLiteral("&Справка"));
    QAction *about = help->addAction(QStringLiteral("&О программе TPL Designer..."), &shell, [&shell] {
        tmcabout::show(&shell, QStringLiteral("About TPL Designer"), QStringLiteral("TPL Designer"),
                       QString());
    });
    tmci18n::setOwnText(about, "&О программе TPL Designer...", "&About TPL Designer...");

    // Страницы окна. Тексты страниц берутся на действующем языке при создании, поэтому
    // при смене языка страницы создаются заново (см. ниже), а задание в работе
    // переносится в новый мастер. Обработчики берут страницы через pg — всегда текущие.
    /// @brief Текущие страницы стека; меняются целиком при смене языка.
    struct Pages {
        /// Стартовое окно (создать, открыть, образцы, вспомогательные файлы).
        StartWindow *start = nullptr;
        /// Мастер задания — главное рабочее окно; владеет проектом в работе.
        MainWindow *wizard = nullptr;
        /// Библиотека образцов.
        SamplesPage *library = nullptr;
    } pg;

    // Создаёт три страницы на действующем языке, кладёт их в стек и связывает сигналы.
    auto build = [&root, &pg] {
        pg.start = new StartWindow;
        pg.wizard = new MainWindow;
        pg.library = new SamplesPage;
        root.addWidget(pg.start);
        root.addWidget(pg.wizard);
        root.addWidget(pg.library);

        // «Создать» — это ВСЕГДА чистый лист. Мастер живёт в общем стеке и переживает
        // возврат на стартовый экран, поэтому без явного сброса кнопка открывала
        // предыдущее задание со всей его геометрией — как будто новое создать нельзя.
        //
        // Молча стирать работу тоже неправильно, поэтому если в мастере что-то есть,
        // спрашиваем. Ответ «Продолжить» возвращает к прежнему заданию, ничего не теряя.
        QObject::connect(pg.start, &StartWindow::createRequested, &root, [&root, &pg] {
            if (pg.wizard->hasContent()) {
                QMessageBox box(QMessageBox::Question,
                                tpl::tr("Начать новое задание?", "Start a new task?"),
                                tpl::tr("В работе уже есть задание. Начать новое — "
                                        "значит очистить холст и все поля.\n\n"
                                        "Несохранённое будет потеряно.",
                                        "A task is already in progress. Starting a new one "
                                        "clears the canvas and all fields.\n\n"
                                        "Unsaved work will be lost."),
                                QMessageBox::NoButton, pg.start);
                QPushButton *fresh = box.addButton(tpl::tr("Начать новое", "Start new"),
                                                   QMessageBox::AcceptRole);
                QPushButton *keep = box.addButton(tpl::tr("Вернуться к прежнему", "Back to current task"),
                                                  QMessageBox::RejectRole);
                box.setDefaultButton(keep);
                box.exec();
                if (box.clickedButton() == fresh)
                    pg.wizard->resetProject();
            } else {
                pg.wizard->resetProject();
            }
            root.setCurrentWidget(pg.wizard);
        });

        // Открытие ведёт в тот же мастер, что и создание: править прочитанный файл
        // пользователь должен ровно теми же вкладками, а не отдельным «режимом чтения».
        // Не открылось — остаёмся на старте, о причине скажет само окно мастера.
        QObject::connect(pg.start, &StartWindow::openRequested, &root, [&root, &pg] {
            const QString path = QFileDialog::getOpenFileName(
                pg.start, tpl::tr("Открыть задание", "Open task"), QString(),
                tpl::tr("Задания TMC (*.tpl *.TPL);;Все файлы (*)",
                        "TMC tasks (*.tpl *.TPL);;All files (*)"));
            if (path.isEmpty())
                return;
            if (pg.wizard->loadFromFile(path))
                root.setCurrentWidget(pg.wizard);
        });
        QObject::connect(pg.wizard, &MainWindow::backToStartRequested, &root,
                         [&root, &pg] { root.setCurrentWidget(pg.start); });

        // Библиотека образцов. Открыть образец — значит начать с ним новое задание, поэтому
        // здесь тот же вопрос, что у «Создать»: работу в мастере молча не стираем.
        QObject::connect(pg.start, &StartWindow::samplesRequested, &root,
                         [&root, &pg] { root.setCurrentWidget(pg.library); });

        // Мастерская вспомогательных файлов — окно, а не страница: её открывают по дороге,
        // не бросая того, над чем работают.
        QObject::connect(pg.start, &StartWindow::filesRequested, pg.start, [&pg] {
            FileMakerDialog dlg(pg.wizard->project(), pg.start);
            dlg.exec();
        });
        QObject::connect(pg.library, &SamplesPage::backRequested, &root,
                         [&root, &pg] { root.setCurrentWidget(pg.start); });
        QObject::connect(pg.library, &SamplesPage::openRequested, &root,
                         [&root, &pg](const tpl::Project &p,
                                      const QMap<QString, QByteArray> &companions) {
            if (pg.wizard->hasContent()) {
                QMessageBox box(QMessageBox::Question, tpl::tr("Открыть образец?", "Open sample?"),
                                tpl::tr("В мастере уже есть задание. Образец откроется вместо "
                                        "него.\n\nНесохранённое будет потеряно.",
                                        "The wizard already holds a task. The sample will replace "
                                        "it.\n\nUnsaved work will be lost."),
                                QMessageBox::NoButton, pg.library);
                QPushButton *go = box.addButton(tpl::tr("Открыть образец", "Open sample"),
                                                QMessageBox::AcceptRole);
                box.addButton(tpl::tr("Отмена", "Cancel"), QMessageBox::RejectRole);
                box.exec();
                if (box.clickedButton() != go)
                    return;
            }
            pg.wizard->openSample(p, companions);
            root.setCurrentWidget(pg.wizard);
        });
    };
    build();

    // Смена языка: страницы пересоздаются на новом языке, задание в работе и открытая
    // страница сохраняются. Проверка — после обработчика меню языка (он меняет выбор),
    // поэтому отложена до возврата в цикл событий.
    QObject::connect(languageMenu, &QMenu::triggered, &shell, [&root, &pg, &build] {
        QTimer::singleShot(0, &root, [&root, &pg, &build] {
            const bool english = !tmci18n::russian();
            if (english == tpl::english())
                return;
            tpl::setEnglish(english);
            const int page = root.currentIndex();
            const bool keep = pg.wizard->hasContent();
            const tpl::Project project = pg.wizard->project();
            const QList<QWidget *> old = {pg.start, pg.wizard, pg.library};
            for (QWidget *w : old) {
                root.removeWidget(w);
                w->deleteLater();
            }
            build();
            if (keep)
                pg.wizard->setProject(project);
            root.setCurrentIndex(page);
        });
    });

    // ⚠️ Размер окна берём НЕ ровно 1280x820, а «не меньше, чем просит разметка».
    //
    // 1280 подбирались под Segoe UI. Системный шрифт Linux заметно шире, и при том же
    // числе панель инструментов теряла последнюю кнопку — окно открывалось уже своего
    // содержимого. Спросив разметку, получаем верный размер на любом шрифте, а на
    // Windows остаются те же 1280x820: там разметке этого хватает.
    shell.resize(shell.minimumSizeHint().expandedTo(QSize(1280, 820 + shell.menuBar()->sizeHint().height())));

    // Задание можно назвать в командной строке: `TplDesigner задание.tpl`.
    //
    // Это не удобство, а обязательство: ярлык для меню приложений Linux объявлен как
    // `Exec=TplDesigner %f`, то есть система вправе передать нам файл — двойным щелчком
    // по .tpl или через «Открыть с помощью». Не разбирай мы аргумент, программа
    // молча открывалась бы пустой, а человек считал бы, что файл не читается.
    //
    // Не открылось — остаёмся на стартовом экране: причину уже назвало окно мастера,
    // и захлопывать программу из-за плохого аргумента незачем.
    const QStringList args = app.arguments();
    if (args.size() > 1 && !args.at(1).startsWith(QLatin1Char('-'))) {
        if (pg.wizard->loadFromFile(args.at(1)))
            root.setCurrentWidget(pg.wizard);
    }

    shell.show();
    return app.exec();
}
