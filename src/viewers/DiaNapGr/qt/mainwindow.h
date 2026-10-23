// mainwindow.h — главное окно TMC_DN (вьювер диаграмм направленности).
//
// Повторяет окно Windows-версии: те же пункты меню в том же порядке (ресурс
// IDR_TMCGROTYPE), та же панель с теми же пиктограммами (IDR_MAINFRAME TOOLBAR),
// та же строка состояния, многодокументный режим (MDI). Команды не расписаны
// вручную по одной: каждая связана с числовым идентификатором из ресурсов, а
// выполняет её карта сообщений вида — тот же код, что и на Windows.
//
// Классы вида и документа унаследованы от TMCGROUT и потому названы
// CTMCGROUTView / CTMCGROUTDoc, но рисуют они диаграмму направленности.

/// @file mainwindow.h
/// @brief Главное окно TMC_DN на Qt: меню и панель из ресурсов Windows-версии, строка состояния, окна документов (MDI).
///
/// Часть модуля «Вьювер TMC_DN». Команды выполняет карта сообщений вида `CTMCGROUTView`.
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#qt-shell
#pragma once

#include <QMainWindow>
#include <QMap>

#include "mfc_dialog.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMdiArea;
class QMdiSubWindow;
class QLabel;
QT_END_NAMESPACE

class MfcViewWidget;
class CTMCGROUTDoc;
class CTMCGROUTView;
class CMainFrame;

/// Одно окно документа: документ, вид и виджет-хозяин живут вместе.
/// Владеет главное окно (`MainWindow::m_documents`); удаляются в деструкторе главного окна.
struct TmcDocumentWindow
{
    /// Документ; владеет главное окно.
    CTMCGROUTDoc  *doc = nullptr;
    /// Вид документа; владеет главное окно.
    CTMCGROUTView *view = nullptr;
    /// Виджет-хозяин вида (`DnViewWidget`); принадлежит подокну MDI (удаляет Qt).
    MfcViewWidget *widget = nullptr;
    /// Подокно MDI документа; принадлежит области MDI.
    QMdiSubWindow *sub = nullptr;
};

/// @brief Главное окно TMC_DN: меню, панель, строка состояния и окна документов.
///
/// Единственный объект создаётся в `main()` (и тестовыми программами). Регистрирует
/// разметку диалогов, создаёт рамку `CMainFrame` для строки состояния кода вьювера (с
/// хозяином `StatusBarHost`, который принимает текст и из потока расчёта) и делает её
/// главным окном приложения слоя совместимости. Команда меню сначала предлагается карте
/// сообщений вида активного окна, затем обрабатывается самим окном. Печать выполняется
/// самим окном (вид рисует себя на принтер).
/// @note Потоки: главный поток; `putStatusText` из чужого потока вызывается через очередь событий.
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#qt-shell
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief Строит меню, панель и строку состояния, регистрирует диалоги, восстанавливает положение окна.
    /// @param[in] parent родительский виджет; обычно nullptr
    explicit MainWindow(QWidget *parent = nullptr);
    /// @brief Отвязывает виды от виджетов и удаляет документы, виды и рамку.
    ~MainWindow() override;

    /// Открыть файл задания (документ TMC_DN).
    /// @param[in] path     путь к документу `.dop` или файлу излучателей `.DAT`; пусто — новый документ.
    ///                     Файла нет или путь не в кодовой странице Windows — сообщение, окно не создаётся
    /// @param[in] newTitle заголовок окна для нового документа (при пустом `path`)
    /// @note Расчёт ДН запускается здесь же (`initialUpdate` вида): поток и модальное окно прогресса.
    void openDocument(const QString &path, const QString &newTitle = QString());

    /// Текст в строку состояния от кода Windows-версии (PutTrace, PutStatistics).
    /// @param[in] pane номер ячейки: 0 — левая, 1 — правая
    /// @param[in] text текст
    /// @note Потоки: только главный поток (из потока расчёта текст приходит через очередь событий).
    void putStatusText(int pane, const QString &text);

private slots:
    /// @brief Команда File > New: окно документа без пути с заголовком `TMCRTOS<номер>` (тип документа из ресурсов Windows-версии).
    void onNewDocument();
    /// @brief Команда File > Open: выбор файла (`.dat`, `.dop`, `.$op` или любой) и открытие.
    void onOpenDocument();
    /// @brief Команда File > Close: закрывает активное окно документа.
    void onCloseDocument();
    /// @brief Команда Help > About: окно «О программе».
    void onAbout();
    /// @brief Смена активного окна документа: обновляется доступность команд.
    /// @param[in] sub ставшее активным подокно (может быть nullptr)
    void onSubWindowActivated(QMdiSubWindow *sub);
    /// @brief Показывает текст ячейки строки состояния от вида.
    /// @param[in] pane номер ячейки: 0 и меньше — левая (координаты), 1 и больше — правая (величина Y и частота)
    /// @param[in] text текст
    void onStatusText(int pane, const QString &text);

private:
    /// @brief Строит меню File, Edit, View (с подменю Zoom и Translate, флажками строки состояния и панели), Config (язык, редактор, цвета, шрифт), Window, Help по таблицам команд.
    void createMenus();
    /// @brief Строит панель инструментов по таблице `kToolBar` (порядок и подсказки из ресурсов Windows-версии).
    void createToolBar();
    /// @brief Создаёт строку состояния из двух ячеек (`Ready` и пустая правая).
    void createStatusBar();
    /// @brief Делает команды документа недоступными, если нет активного окна документа (кроме New, Open, Exit, About).
    void updateMenuForDocument();

    /// Выполнить команду: сначала предлагаем виду активного окна, затем себе.
    /// @param[in] commandId номер команды из `resource.h`
    void dispatch(UINT commandId);

    /// Опросить обработчики обновления и перенести их решение
    /// на пункты меню и кнопки (как каркас MFC перед показом меню).
    /// @see https://www.tamic.ru/docs/code/gui-common.html#command-update
    void updateCommandStates();

    /// @brief Создаёт действие с номером команды в `data()`, которое при срабатывании вызывает `dispatch`.
    /// @param[in] id       номер команды
    /// @param[in] text     подпись
    /// @param[in] shortcut сочетание клавиш; пусто — нет
    /// @return Действие; принадлежит окну.
    QAction *makeAction(UINT id, const QString &text, const QString &shortcut = QString());
    /// @brief Возвращает окно документа активного подокна.
    /// @return Окно документа; nullptr — активного подокна нет.
    TmcDocumentWindow *activeDocument() const;

    /// Область окон документов (центральный виджет).
    QMdiArea            *m_mdi = nullptr;
    /// Действия меню и панели по номерам команд.
    QMap<UINT, QAction*> m_actions;
    /// Открытые окна документов; владеет главное окно.
    QList<TmcDocumentWindow*> m_documents;
    /// Левая ячейка строки состояния.
    QLabel              *m_statusMain = nullptr;
    /// Правая ячейка строки состояния.
    QLabel              *m_statusExtra = nullptr;
    /// Рамка `CMainFrame` — главное окно для кода вьювера; владеет главное окно.
    CMainFrame          *m_frame = nullptr;   // строка состояния для кода из MFC-версии
    /// Хозяин строки состояния рамки (`StatusBarHost`); владеет главное окно.
    class MfcQtHost     *m_statusHost = nullptr;  // хозяин этой строки состояния
    /// Меню Window.
    QMenu               *m_windowMenu = nullptr;
};
