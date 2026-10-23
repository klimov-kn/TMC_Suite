// mainwindow.h — главное окно TMCGROUT.
//
// Повторяет окно Windows-версии: те же пункты меню в том же порядке, та же
// панель с теми же пиктограммами, та же строка состояния, многодокументный
// режим (MDI). Команды не расписаны вручную по одной: каждая связана с числовым
// идентификатором из ресурсов, а выполняет её карта сообщений вида — тот же код,
// что и на Windows.

/// @file mainwindow.h
/// @brief Главное окно TMCGROUT на Qt: меню и панель из ресурсов Windows-версии, строка состояния, окна документов (MDI).
///
/// Часть модуля «Вьювер TMCGROUT». Команды выполняет карта сообщений вида `CTMCGROUTView`.
/// @see https://www.tamic.ru/docs/code/tmcgrout.html#qt-shell
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
    /// Виджет-хозяин вида; принадлежит подокну MDI (удаляет Qt).
    MfcViewWidget *widget = nullptr;
    /// Подокно MDI документа; принадлежит области MDI.
    QMdiSubWindow *sub = nullptr;
};

/// @brief Главное окно TMCGROUT: меню, панель, строка состояния и окна документов.
///
/// Единственный объект создаётся в `main()` (и тестовыми программами). Регистрирует
/// разметку диалогов, создаёт рамку `CMainFrame` для строки состояния кода вьювера и
/// делает её главным окном приложения слоя совместимости. Команда меню сначала
/// предлагается карте сообщений вида активного окна, затем обрабатывается самим окном.
/// @note Потоки: только главный поток.
/// @see https://www.tamic.ru/docs/code/tmcgrout.html#qt-shell
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief Строит меню, панель и строку состояния, регистрирует диалоги, восстанавливает положение окна.
    /// @param[in] parent родительский виджет; обычно nullptr
    explicit MainWindow(QWidget *parent = nullptr);
    /// @brief Отвязывает виды от виджетов и удаляет документы, виды и рамку.
    ~MainWindow() override;

    /// Открыть файл задания (документ TMCGROUT).
    /// @param[in] path путь к файлу документа `.soc` или к файлу `.s`; несуществующий файл
    ///                 или путь с символами вне кодовой страницы — сообщение, документ не открывается
    void openDocument(const QString &path);

private slots:
    /// @brief Команда File > New: окно документа без пути с заголовком `TMCGROUT<номер>`.
    void onNewDocument();
    /// @brief Команда File > Open: выбор файла (`.soc`, `.$oc`, `.s`, `.s2p`..`.s8p`, `.tab`) и открытие.
    void onOpenDocument();
    /// @brief Команда File > Close: закрывает активное окно документа.
    void onCloseDocument();
    /// @brief Команда Help > About: окно «О программе».
    void onAbout();
    /// @brief Смена активного окна документа: строка состояния переключается на его виджет, обновляется доступность команд.
    /// @param[in] sub ставшее активным подокно (может быть nullptr)
    void onSubWindowActivated(QMdiSubWindow *sub);
    /// @brief Показывает текст ячейки строки состояния от вида.
    /// @param[in] pane номер ячейки: 0 — левая (координаты), 1 и больше — правая (величина Y)
    /// @param[in] text текст
    void onStatusText(int pane, const QString &text);

private:
    /// @brief Строит меню File, Edit, View, Config, Window, Help по таблицам команд; добавляет список недавних файлов, выбор языка и флажок разворота фазы.
    void createMenus();
    /// @brief Строит панель инструментов по таблице `kToolBar` (порядок и подсказки из ресурсов Windows-версии).
    void createToolBar();
    /// @brief Создаёт строку состояния из двух ячеек (`Ready` и пустая правая).
    void createStatusBar();
    /// @brief Делает команды документа недоступными, если нет активного окна документа (кроме New, Open, Exit, About).
    void updateMenuForDocument();

    /// Общая часть открытия и создания: окно документа с видом.
    /// @param[in] path  путь к файлу; пустой — новый документ без файла
    /// @param[in] title заголовок подокна
    void createDocumentWindow(const QString &path, const QString &title);

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
    /// Список недавних файлов в меню File.
    class TmcRecentFiles *m_recent = nullptr;   // список недавних файлов
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
    /// Меню Window.
    QMenu               *m_windowMenu = nullptr;
};
