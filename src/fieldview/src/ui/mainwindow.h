// Главное окно FieldView: несколько документов в одном окне (как MDI Windows-версии),
// меню, панели инструментов, строка состояния, окно «Уровни цвета».
// Команды — те же, что в меню и на панели инструментов Windows-версии (имена действий =
// ID команд ресурса FldView.rc), плюс новые: такты, экспорт, слежение за расчётом.
/// @file mainwindow.h
/// @brief Главное окно FieldView: окна документов (MDI), меню, панели инструментов, строка состояния.
///
/// Часть модуля FieldView (слой `ui`). Команды меню и панелей — объекты `QAction` с
/// именами, равными идентификаторам команд FieldView 4.1 (`ID_...`).
/// @see https://www.tamic.ru/docs/code/fieldview.html#mainwindow
#pragma once

#include "viewsettings.h"

#include <QHash>
#include <QMainWindow>
#include <QPointer>

#include <functional>
#include <memory>
#include <vector>

class QAction;
class QActionGroup;
class QDockWidget;
class QLabel;
class QMdiArea;
class QMdiSubWindow;
class QMenu;
class QPrinter;
class QSlider;
class QSpinBox;
class QTableWidget;
class QToolBar;

namespace fieldview {

class FieldDocument;
class FieldPane;

/// @brief Главное окно программы.
///
/// Создаётся в `main()` (и программой снимков), одно на процесс. Владеет областью MDI с
/// окнами документов (`FieldPane`), действиями команд, панелями и принтером. Команды окна
/// документа выполняются над активным окном; доступность и флажки команд обновляет
/// `updateActions()`. Работает в потоке интерфейса.
/// @see https://www.tamic.ru/docs/code/fieldview.html#mainwindow
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    /// @brief Создаёт окно: команды, меню, панели, строку состояния, панель «Уровни цвета»; восстанавливает размеры.
    MainWindow();
    /// @brief Уничтожает окно.
    ~MainWindow() override;

    /// @brief Открывает файл поля в новом окне документа или активирует уже открытое.
    ///
    /// Открыть файл поля (или показать уже открытый).
    ///
    /// Новое окно берёт сохранённые настройки вида; разворачивается, если оно первое или
    /// активное окно развёрнуто. Файл добавляется в список последних.
    /// @param[in] path  путь к файлу поля, серии или топологии
    /// @return Окно документа (принадлежит области MDI).
    FieldPane *openFile(const QString &path);
    /// @brief Возвращает активное окно документа (нет активного — последнее открытое).
    /// @return Окно или `nullptr`, если окон нет.
    FieldPane *activePane() const;
    /// @brief Возвращает действие команды по идентификатору.
    ///
    /// Действие по ID команды (для проверок и снимков).
    /// @param[in] id  идентификатор («ID_FILE_OPEN» …)
    /// @return Действие или `nullptr`.
    QAction *action(const QString &id) const { return m_actions.value(id); }
    /// @brief Возвращает идентификаторы всех команд.
    /// @return Список идентификаторов (в порядке хэш-таблицы).
    QList<QString> actionIds() const { return m_actions.keys(); }
    /// @brief Возвращает панель «Уровни цвета».
    /// @return Панель (принадлежит окну).
    QDockWidget *levelsDock() const { return m_levelsDock; }
    /// @brief Открывает первый существующий файл из списка последних (запуск без файла).
    void openLastRecent();
    /// @brief Записывает такты окна анимацией GIF или кадрами PNG.
    ///
    /// Записать такты анимацией GIF (gif = true) или кадрами PNG в папку out.
    ///
    /// Каждый такт показывается в окне, снимается `grabImage()`, приводится к ширине
    /// `width` и пишется; проигрывание останавливается, показанный такт затем
    /// восстанавливается. Кадры PNG называются `<имя>_0001.png` и т. д.
    /// @param[in] pane            окно документа
    /// @param[in] frames          номера тактов (с 0)
    /// @param[in] out             файл `.gif` или папка для PNG
    /// @param[in] gif             `true` — GIF, `false` — PNG
    /// @param[in] fps             кадров в секунду для GIF (задержка кадра — 100 / fps сотых секунды, не меньше 2)
    /// @param[in] width           ширина кадров, точек
    /// @param[out] error          причина неудачи («прервано», «не удалось снять кадр», путь, текст ошибки файла); может быть `nullptr`
    /// @param[in] progressParent  родитель окна хода записи; `nullptr` — без окна
    /// @return `true` — все кадры записаны.
    /// @see https://www.tamic.ru/docs/code/fieldview.html#gif
    static bool exportFrames(FieldPane *pane, const std::vector<int> &frames, const QString &out, bool gif,
                             int fps, int width, QString *error, QWidget *progressParent = nullptr);
    /// @brief Печатает вид окна документа: строка с именем файла, nT и t, ниже — картинка вида по размеру страницы.
    ///
    /// Напечатать вид окна поля на принтер (File → Print; открыт для проверок).
    /// @param[in,out] printer  принтер (или PDF)
    /// @param[in]     pane     окно документа
    /// @param[out]    error    причина неудачи; может быть `nullptr`
    /// @return `false` — картинку вида получить не удалось, ничего не напечатано.
    static bool printPane(QPrinter *printer, FieldPane *pane, QString *error);

protected:
    /// @brief Сохраняет размер и положение окна и панелей, закрывает окна документов.
    /// @param[in] e  событие закрытия
    void closeEvent(QCloseEvent *e) override;
    /// @brief Принимает перетаскивание файлов.
    /// @param[in] e  событие
    void dragEnterEvent(QDragEnterEvent *e) override;
    /// @brief Открывает перетащенные локальные файлы.
    /// @param[in] e  событие
    void dropEvent(QDropEvent *e) override;

private:
    /// Действие команды над окном документа.
    using PaneFn = std::function<void(FieldPane *)>;
    /// Состояние команды для окна документа (флажок или доступность).
    using PaneBool = std::function<bool(FieldPane *)>;
    /// @brief Создаёт команду окна документа.
    ///
    /// Команда недоступна без открытого окна; подсказка дополняется первой клавишей.
    /// @param[in] id       идентификатор команды (имя объекта и ключ поиска)
    /// @param[in] text     текст пункта меню
    /// @param[in] tip      подсказка и текст строки состояния (пусто — `text`)
    /// @param[in] keys     сочетания клавиш
    /// @param[in] run      действие над активным окном
    /// @param[in] checked  состояние флажка (`nullptr` — команда без флажка)
    /// @param[in] enabled  доступность (`nullptr` — доступна при открытом окне)
    /// @return Созданное действие (принадлежит главному окну).
    QAction *cmd(const QString &id, const QString &text, const QString &tip, const QList<QKeySequence> &keys,
                 PaneFn run, PaneBool checked = nullptr, PaneBool enabled = nullptr);
    /// @brief Создаёт команду программы (доступна всегда).
    /// @param[in] id    идентификатор команды
    /// @param[in] text  текст пункта меню
    /// @param[in] tip   подсказка (пусто — `text`)
    /// @param[in] keys  сочетания клавиш
    /// @param[in] run   действие
    /// @return Созданное действие.
    QAction *appCmd(const QString &id, const QString &text, const QString &tip,
                    const QList<QKeySequence> &keys, std::function<void()> run);
    /// @brief Создаёт все команды: файл, правка, вид, такты, окна, справка.
    void buildActions();
    /// @brief Строит меню «Файл», «Правка», «Вид», «Такты», «Окно», «Справка».
    void buildMenus();
    /// @brief Строит основную панель (порядок FieldView 4.1) и панель тактов (ползунок, контраст).
    void buildToolbars();
    /// @brief Строит строку состояния: сообщение, слежение, nT и t, интерфейс и кадр/с.
    void buildStatusBar();
    /// @brief Строит панель «Уровни цвета» (таблица 20 x 2, скрыта).
    void buildLevelsDock();
    /// @brief Обновляет доступность и флажки команд, значки слежения и проигрывания, ползунок тактов.
    void updateActions();
    /// @brief Обновляет строку состояния по активному окну.
    void updateStatus();
    /// @brief Заполняет панель «Уровни цвета» (если видна): сверху старший уровень.
    void updateLevels();
    /// @brief Перестраивает меню «Окно» со списком окон документов.
    void updateWindowMenu();
    /// @brief Ставит файл первым в список последних (не больше 7).
    /// @param[in] path  абсолютный путь
    void addRecent(const QString &path);
    /// @brief Перестраивает подменю «Последние файлы».
    void updateRecentMenu();
    /// @brief Подписывает главное окно на сигналы активного окна документа (от прежнего отписывает).
    /// @param[in] pane  окно документа или `nullptr`
    void connectPane(FieldPane *pane);

    /// @brief Команда «Открыть…»: выбор одного или нескольких файлов.
    void fileOpen();
    /// @brief Команда «Сохранить изображение…»: картинка вида в PNG, JPEG или BMP.
    void saveImage();
    /// @brief Команда «Копировать изображение»: картинка вида в буфер обмена.
    void copyImage();
    /// @brief Команда «Сохранить такты серией…»: все такты в файл `.frames`.
    void saveSeries();
    /// @brief Команда «Экспорт анимации»: окно выбора формата, диапазона, шага, частоты, ширины; затем `exportFrames()`.
    void exportAnimation();
    /// @brief Команды «Печать…» и «Предварительный просмотр».
    /// @param[in] preview  `true` — окно просмотра, `false` — окно печати
    void print(bool preview);
    /// @brief Команда «Параметры страницы…».
    void pageSetup();
    /// @brief Команда «Параметры…»: окно настроек вида для активного окна документа.
    void showParameters();
    /// @brief Команда «Новое окно»: ещё одно окно того же документа с его настройками.
    void newWindow();
    /// @brief Команда «О программе»: общее окно пакета или собственное окно FieldView.
    void about();
    /// @brief Открывает руководство пользователя: файл рядом с программой (до 6 папок вверх) или страницу сайта.
    void openManual();

    /// Область окон документов.
    QMdiArea *m_mdi = nullptr;
    /// Команды по идентификатору.
    QHash<QString, QAction *> m_actions;
    /// Состояние флажков команд.
    QHash<QAction *, PaneBool> m_checked;
    /// Доступность команд.
    QHash<QAction *, PaneBool> m_enabled;
    /// Команды окна документа.
    QList<QAction *> m_paneActions;
    /// Подменю «Последние файлы».
    QMenu *m_recentMenu = nullptr;
    /// Меню «Окно».
    QMenu *m_windowMenu = nullptr;
    /// Основная панель инструментов.
    QToolBar *m_mainBar = nullptr;
    /// Панель тактов.
    QToolBar *m_animBar = nullptr;
    /// Ползунок такта.
    QSlider *m_frameSlider = nullptr;
    /// Надпись «такт / всего».
    QLabel *m_frameLabel = nullptr;
    /// Скорость проигрывания, кадров в секунду (скрыт).
    QSpinBox *m_speed = nullptr;
    /// Контраст шкалы поля, 1 … 100.
    QSpinBox *m_gain = nullptr;
    /// @name Панели строки состояния
    ///@{
    /// Сообщение, nT и t, слежение, графический интерфейс и кадр/с.
    QLabel *m_stTrace = nullptr, *m_stStats = nullptr, *m_stSync = nullptr, *m_stBackend = nullptr;
    ///@}
    /// Панель «Уровни цвета».
    QDockWidget *m_levelsDock = nullptr;
    /// Таблица уровней (цвет, значение от).
    QTableWidget *m_levels = nullptr;
    /// Окно документа, на сигналы которого подписано главное окно.
    QPointer<FieldPane> m_connectedPane;
    /// Принтер (общие параметры страницы для печати и просмотра).
    std::unique_ptr<QPrinter> m_printer;
    /// Идёт `updateActions()`: сигналы элементов управления не выполняют команды.
    bool m_updating = false;
};

} // namespace fieldview
