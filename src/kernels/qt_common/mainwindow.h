// mainwindow.h — главное окно оболочек счётных ядер PlanarRT_H и PlanarRT_X.
//
// Повторяет окно Windows-версии: те же пункты меню в том же порядке (ресурс
// IDR_PLANRTTYPE), та же панель с теми же пиктограммами (IDR_MAINFRAME
// TOOLBAR), строка состояния из трёх панелей, многодокументный режим (MDI).
//
// Команды не расписаны вручную по одной: каждая связана с числовым
// идентификатором из ресурсов, а выполняет её карта сообщений вида — тот же
// код, что и на Windows (CPlanRT_HView).
//
// Счёт идёт в отдельном потоке, как на Windows (AfxBeginThread поверх QThread).
// Ход счёта приходит в строку состояния из этого потока, поэтому текст
// доставляется через очередь событий Qt, а не прямым вызовом виджета.

/// @file mainwindow.h
/// @brief Главное окно Qt оболочек ядер H и X: меню, панель, строка состояния с ходом расчёта, окна заданий (MDI).
///
/// Часть модуля «Оболочка счётного ядра». Окно общее для обеих программ; различия —
/// в `planrt_app.h`. Команды выполняет карта сообщений вида `CPlanRT_HView`.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#main-window
#pragma once

#include <QMainWindow>
#include <QMap>

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMdiArea;
class QMdiSubWindow;
class QProgressBar;
class QTimer;
class TmcRecentFiles;
QT_END_NAMESPACE

class MfcViewWidget;
class CPlanRT_HDoc;
class CPlanRT_HView;
class CMainFrame;

/// Одно окно документа: документ, вид и виджет-хозяин живут вместе.
/// Владеет главное окно (`MainWindow::m_documents`); удаляются в деструкторе главного окна.
struct TmcPlanRtWindow
{
    /// Документ задания; владеет главное окно.
    CPlanRT_HDoc  *doc = nullptr;
    /// Вид задания; владеет главное окно.
    CPlanRT_HView *view = nullptr;
    /// Виджет-хозяин вида; принадлежит подокну MDI (удаляет Qt).
    MfcViewWidget *widget = nullptr;
    /// Подокно MDI задания; принадлежит области MDI.
    QMdiSubWindow *sub = nullptr;
};

/// @brief Главное окно программы: меню и панель из ресурсов Windows-версии, строка состояния, окна заданий.
///
/// Единственный объект создаётся в `main()` (и тестовыми программами). Регистрирует
/// разметку диалогов, обработчики сообщений и закрытия для кода ядра, создаёт рамку
/// `CMainFrame` для строки состояния. Таймер раз в 200 мс пересчитывает доступность
/// команд, индикаторы вьюверов и полосу хода расчёта.
/// @note Потоки: все методы — в главном потоке; из расчётного потока текст строки
///       состояния приходит слотом `setStatusPane` через очередь событий.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#main-window
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief Создаёт окно: ресурсы и диалоги, обработчики для кода ядра, меню, панель, строку состояния, рамку, таймер.
    /// @param[in] parent родительский виджет (обычно nullptr).
    explicit MainWindow(QWidget *parent = nullptr);
    /// @brief Удаляет документы и виды всех заданий, рамку; снимает указатели, оставленные коду ядра.
    ~MainWindow() override;

    /// Открыть файл задания (.tpl).
    ///
    /// Проверяет, что путь переводится в кодовую страницу Windows; делает папку задания
    /// текущей (туда препроцессор пишет временный файл); создаёт документ, вид, виджет
    /// и развёрнутое подокно; добавляет задание в список недавних; вызывает
    /// `OnInitialUpdate` вида (чтение настроек и первого шага, в пакетном режиме — запуск).
    /// @param[in] path путь к файлу .tpl (относительный приводится к абсолютному).
    /// @see https://www.tamic.ru/docs/code/kernel-shell.html#open-task
    void openDocument(const QString &path);

public slots:
    /// Текст в панель строки состояния. Вызывается в том числе из расчётного
    /// потока — только через очередь событий (Qt::QueuedConnection).
    /// Сначала разбирает исходный (английский) текст для полосы хода, затем показывает перевод.
    /// @param[in] pane номер панели: 0 — ход расчёта, 1 — статистика такта, 2 и далее — что выводится.
    /// @param[in] text текст ядра (кодировка — локальная, уже переведена в QString).
    /// @see https://www.tamic.ru/docs/code/kernel-shell.html#status-bar
    void setStatusPane(int pane, const QString &text);

    /// Закрыть программу по просьбе расчётного ядра (пакетный режим: досчитав
    /// последний шаг, программа закрывается сама — как PostMessage(WM_CLOSE)
    /// в Windows-версии).
    /// @see https://www.tamic.ru/docs/code/kernel-shell.html#console
    void closeFromKernel();

protected:
    /// @brief Закрытие окна: подтверждение и остановка всех расчётов (`stopAllAndConfirm`), затем остановка таймера.
    /// @param[in,out] event событие закрытия; `ignore()` — пользователь передумал.
    /// @see https://www.tamic.ru/docs/code/kernel-shell.html#stop-close
    void closeEvent(QCloseEvent *event) override;

private slots:
    /// @brief File -> New: запускает TPL Designer для нового задания.
    void onNewDocument();
    /// @brief File -> Open: выбор файла задания и `openDocument`.
    void onOpenDocument();
    /// @brief File -> Close: закрывает активное подокно задания.
    /// @note Подокно закрывается без удаления (`WA_DeleteOnClose` = false): документ и вид
    ///       остаются в `m_documents` до выхода из программы и останавливаются в `closeEvent`.
    void onCloseDocument();
    /// @brief Help -> About: окно «О программе» с подписями программы и строкой модели расчёта.
    void onAbout();
    /// @brief Смена активного подокна: пересчитывает доступность команд.
    /// @param[in] sub новое активное подокно (не используется).
    void onSubWindowActivated(QMdiSubWindow *sub);
    /// @brief Config -> Compute (GPU)...: окно выбора видеокарты; обновляет надпись CPU/GPU в строке состояния.
    void onComputeSettings();       // Config -> Compute (GPU)...

private:
    /// @brief Строит меню File, Edit, View, Run, Config, Window, Help по таблицам ресурса IDR_PLANRTTYPE.
    void createMenus();
    /// @brief Строит панель инструментов по таблице IDR_MAINFRAME TOOLBAR; кнопкам даёт имена для проверок интерфейса.
    void createToolBar();
    /// @brief Строит строку состояния: полоса хода и этап, панели 0–2, надпись CPU/GPU.
    void createStatusBar();

    /// Доступность и «галочки» пунктов меню. Повторяет обработчики
    /// ON_UPDATE_COMMAND_UI Windows-версии.
    void updateCommandStates();

    /// Выполнить команду: сначала предлагаем виду активного окна, затем себе.
    ///
    /// Печать выполняет само окно (вид рисует себя на принтер); Edit -> Edit открывает
    /// задание в TPL Designer, если он найден. Перед Run и Run Step готовит полосу хода.
    /// @param[in] commandId номер команды из `resource.h` / `afxres.h`.
    void dispatch(UINT commandId);

    /// @brief Создаёт действие команды и связывает его с `dispatch`.
    /// @param[in] id        номер команды; хранится в `data()` и в имени объекта `cmd_<id>`.
    /// @param[in] text      подпись с мнемоникой `&`.
    /// @param[in] shortcut  сочетание клавиш; пусто — нет.
    /// @param[in] checkable TRUE — пункт-переключатель с отметкой.
    /// @return Действие; принадлежит окну, запоминается в `m_actions`.
    QAction *makeAction(UINT id, const QString &text, const QString &shortcut = QString(),
                        bool checkable = false);
    /// @brief Возвращает задание активного (или последнего активного) подокна.
    /// @return Задание; nullptr — подокон нет.
    TmcPlanRtWindow *activeDocument() const;

    /// Остановить счёт и дождаться потока перед закрытием (как OnClose рамки
    /// Windows-версии). Возвращает false, если пользователь передумал.
    ///
    /// Если идёт расчёт и это не пакетный режим /Arb — спрашивает «Остановить и закрыть?».
    /// Затем для каждого документа `StopAndWait()`.
    /// @return true — все расчёты остановлены, окно можно закрыть; false — отказ.
    /// @see https://www.tamic.ru/docs/code/kernel-shell.html#stop-close
    bool stopAllAndConfirm();

    /// Новое задание — TPL Designer (File -> New, Ctrl+N, первая кнопка панели).
    /// Путь к программе: настройка, рядом с ядром, путь установщика; иначе
    /// спросить один раз и запомнить.
    /// @param[in] task путь к заданию для редактирования; пусто — новое задание.
    /// @return true — программа запущена; false — не найдена или не запустилась (с сообщением).
    /// @see https://www.tamic.ru/docs/code/kernel-shell.html#viewers
    bool launchTplDesigner(const QString &task = QString());
    /// @brief Ищет TPL Designer: настройка Paths.ini, папка ядра и соседние, пути установщика в реестре.
    /// @param[in] askIfMissing true — если не найден, спросить файл и запомнить выбор.
    /// @return Абсолютный путь программы; пусто — не найдена.
    QString tplDesignerPath(bool askIfMissing);

    /// Ход расчёта для полосы в строке состояния: этап и проценты разбираются из
    /// тех же сообщений ядра, что идут в панели строки состояния.
    /// @param[in] pane номер панели строки состояния.
    /// @param[in] text исходный текст ядра.
    /// @see https://www.tamic.ru/docs/code/kernel-shell.html#progress
    void updateProgressFromStatus(int pane, const QString &text);
    /// @brief Перерисовывает полосу хода и подпись этапа; после расчёта — итог
    ///        «Готово», «Готово с ошибками» или «Готово, есть предупреждения».
    void refreshProgress();
    /// @brief Перед запуском запоминает первый и последний шаг расчёта и сбрасывает полосу хода.
    /// @param[in] commandId `ID_RUN_RUN` — все шаги (число шагов — препроцессором), `ID_RUN_STARTSTEP` — один шаг.
    void prepareProgress(UINT commandId);

    /// Индикаторы кнопок запуска вьюверов: зелёный — программа запущена.
    /// @see https://www.tamic.ru/docs/code/kernel-shell.html#viewers
    void updateViewerIndicators();

    /// Область окон заданий (MDI).
    QMdiArea                 *m_mdi = nullptr;
    /// Действия всех команд с номерами: номер команды -> действие.
    QMap<UINT, QAction *>     m_actions;
    /// Открытые задания; владеет окно.
    QList<TmcPlanRtWindow *>  m_documents;
    /// Панель 0 строки состояния: ход расчёта (`PutTrace`).
    QLabel                   *m_statusTrace = nullptr;   // панель 0: ход счёта
    /// Панель 1: статистика такта (`PutStatistics`).
    QLabel                   *m_statusStat = nullptr;    // панель 1: статистика
    /// Панель 2: что выводится (`PutSinchronizFlag`).
    QLabel                   *m_statusFlags = nullptr;   // панель 2: что выводится
    /// Рамка MFC для кода ядра; владеет окно.
    CMainFrame               *m_frame = nullptr;         // рамка для кода из MFC-версии
    /// Таймер пересчёта команд, 200 мс.
    QTimer                   *m_uiTimer = nullptr;       // обновление меню, как в MFC
    /// Надпись «Расчёт: CPU» / «Расчёт: GPU — карта».
    QLabel                   *m_statusCompute = nullptr; // на чём считаем: CPU / GPU
    /// Подпись этапа и итога расчёта (свойство `state`: run, ok, warn, err).
    QLabel                   *m_progressStage = nullptr; // этап расчёта
    /// Полоса хода всего расчёта, 0–100 %.
    QProgressBar             *m_progress = nullptr;      // ход расчёта, %
    /// Число тактов текущего шага (из сообщения «nTmax N»).
    int                       m_progressTMax = 0;        // nTmax текущего шага
    /// Текущий такт шага (из сообщения «nT:k;...»).
    int                       m_progressT = 0;           // nT текущего шага
    // Этап (по сообщениям ядра). Хранится числом, а не переведённой строкой:
    // раньше «Расчёт» сравнивался с "Calculation", и в русском интерфейсе
    // полоса не переходила к процентам.
    /// Этапы расчёта для подписи: нет, чтение задания, препроцессор, построение блоков,
    /// подготовка сетки, расчёт во времени (только у него есть проценты).
    enum ProgressPhase { PhaseNone, PhaseReading, PhasePreprocessor, PhaseBlocks, PhaseGrid, PhaseCalculation };
    /// Текущий этап по последнему сообщению ядра.
    ProgressPhase             m_progressPhase = PhaseNone;
    /// Первый шаг текущего запуска.
    int                       m_progressFirstStep = 1;   // с какого шага начат расчёт
    /// Последний шаг текущего запуска.
    int                       m_progressLastStep = 1;    // последний шаг расчёта
    /// Последний показанный процент (полоса не откатывается); -1 — ещё не было.
    int                       m_progressDone = -1;       // последний показанный процент
    /// Список недавних заданий в меню File.
    TmcRecentFiles           *m_recent = nullptr;        // недавние задания (Т-17)
};
