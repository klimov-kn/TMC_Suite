// Главное окно FieldView — описание в mainwindow.h.
/// @file mainwindow.cpp
/// @brief Главное окно FieldView: команды, меню, панели, строка состояния, файлы, печать, экспорт.
///
/// Часть модуля FieldView (слой `ui`). Описание класса — в `mainwindow.h`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#mainwindow
#include "mainwindow.h"
#ifdef TMC_FV_COMMON_ABOUT
#include "aboutbox.h"
#include "i18n/tmci18n.h"
#endif
#include "fielddocument.h"
#include "fieldpane.h"
#include "gifwriter.h"
#include "icons.h"
#include "parametersdialog.h"
#include "theme.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDockWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPageSetupDialog>
#include <QPainter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QProgressDialog>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QTableWidget>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

#include <cmath>

namespace fieldview {

namespace {

/// Длина списка последних файлов.
const int kMaxRecent = 7;   // LoadStdProfileSettings(7)

/// @brief Возвращает сочетание с клавишей цифровой клавиатуры.
/// @param[in] mods  модификаторы `Qt::KeyboardModifier` (Ctrl, Alt)
/// @param[in] key   клавиша `Qt::Key`
/// @return Сочетание клавиш.
QKeySequence keypad(int mods, int key)
{
    return QKeySequence(QKeyCombination(Qt::KeyboardModifiers(mods) | Qt::KeypadModifier, Qt::Key(key)));
}

/// @brief Возвращает значок «Следить за расчётом» с кружком состояния.
/// @param[in] on  `true` — зелёный кружок (включено), `false` — красный
/// @return Значок.
QIcon syncIcon(bool on)
{
    // Индикатор пакета: зелёный кружок — включено, красный — выключено.
    QIcon base = commandIcon(QStringLiteral("ID_SYNC_KERNEL"));
    QIcon out;
    for (qreal dpr : {1.0, 2.0}) {
        QPixmap pm(int(20 * dpr), int(20 * dpr));
        pm.setDevicePixelRatio(dpr);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        p.drawPixmap(0, 0, base.pixmap(QSize(20, 20), dpr));
        p.setPen(QPen(theme::bg(), 1.5));
        p.setBrush(on ? theme::ok() : theme::danger());
        p.drawEllipse(QPointF(15.5, 15.5), 4, 4);
        p.end();
        out.addPixmap(pm);
    }
    return out;
}

/// Фильтр окна открытия файлов.
const QString kFilter = QStringLiteral(
    "Файлы поля (*.ex *.ey *.hz *.em *.tf *.AMP *.FAZ *.frames *.tt);;Поле в такт (*.ex *.tf);;"
    "Амплитуда и фаза (*.AMP *.FAZ);;Серия тактов (*.ex.frames);;Все файлы (*.*)");

} // namespace

MainWindow::MainWindow()
{
    setWindowTitle(QStringLiteral("FieldView — просмотр полей"));
    setWindowIcon(appIcon());
    setAcceptDrops(true);
    m_mdi = new QMdiArea;
    m_mdi->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mdi->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mdi->setBackground(theme::bg());
    setCentralWidget(m_mdi);
    connect(m_mdi, &QMdiArea::subWindowActivated, this, [this](QMdiSubWindow *) {
        connectPane(activePane());
        updateActions();
        updateStatus();
        updateLevels();
    });
    m_printer = std::make_unique<QPrinter>(QPrinter::HighResolution);
    m_printer->setPageOrientation(QPageLayout::Landscape);
    buildActions();
    buildMenus();
    buildToolbars();
    buildStatusBar();
    buildLevelsDock();
    QSettings s;
    restoreGeometry(s.value(QStringLiteral("TMC_GraphicsOutputFieldFile/INIWND")).toByteArray());
    restoreState(s.value(QStringLiteral("TMC_GraphicsOutputFieldFile/WindowState")).toByteArray());
    if (!s.contains(QStringLiteral("TMC_GraphicsOutputFieldFile/INIWND")))
        resize(1500, 920);
    updateActions();
    updateStatus();
}

MainWindow::~MainWindow() = default;

FieldPane *MainWindow::activePane() const
{
    QMdiSubWindow *w = m_mdi->activeSubWindow();
    if (!w && !m_mdi->subWindowList().isEmpty())
        w = m_mdi->subWindowList().last();
    return w ? qobject_cast<FieldPane *>(w->widget()) : nullptr;
}

// --- Команды -------------------------------------------------------------------

QAction *MainWindow::cmd(const QString &id, const QString &text, const QString &tip,
                         const QList<QKeySequence> &keys, PaneFn run, PaneBool checked, PaneBool enabled)
{
    auto *a = new QAction(commandIcon(id), text, this);
    a->setObjectName(id);
    a->setShortcuts(keys);
    QString t = tip.isEmpty() ? text : tip;
    if (!keys.isEmpty())
        t += QStringLiteral("  (") + keys.first().toString(QKeySequence::NativeText) + QLatin1Char(')');
    a->setToolTip(t);
    a->setStatusTip(tip.isEmpty() ? text : tip);
    if (checked) {
        a->setCheckable(true);
        m_checked.insert(a, checked);
    }
    if (enabled)
        m_enabled.insert(a, enabled);
    connect(a, &QAction::triggered, this, [this, run]() {
        if (m_updating)
            return;
        if (FieldPane *p = activePane()) {
            run(p);
            updateActions();
        }
    });
    m_actions.insert(id, a);
    m_paneActions << a;
    addAction(a);
    return a;
}

QAction *MainWindow::appCmd(const QString &id, const QString &text, const QString &tip,
                            const QList<QKeySequence> &keys, std::function<void()> run)
{
    auto *a = new QAction(commandIcon(id), text, this);
    a->setObjectName(id);
    a->setShortcuts(keys);
    QString t = tip.isEmpty() ? text : tip;
    if (!keys.isEmpty())
        t += QStringLiteral("  (") + keys.first().toString(QKeySequence::NativeText) + QLatin1Char(')');
    a->setToolTip(t);
    a->setStatusTip(tip.isEmpty() ? text : tip);
    connect(a, &QAction::triggered, this, [run]() { run(); });
    m_actions.insert(id, a);
    addAction(a);
    return a;
}

void MainWindow::buildActions()
{
    using K = QKeySequence;
    // --- File.
    appCmd(QStringLiteral("ID_FILE_OPEN"), QStringLiteral("&Открыть…"), QStringLiteral("Открыть файл поля (File → Open)"),
           {K(K::Open)}, [this]() { fileOpen(); });
    cmd(QStringLiteral("ID_FILE_CLOSE"), QStringLiteral("&Закрыть"), QStringLiteral("Закрыть окно документа (File → Close)"),
        {K(Qt::CTRL | Qt::Key_F4)}, [this](FieldPane *) { m_mdi->closeActiveSubWindow(); });
    cmd(QStringLiteral("ID_FILE_SAVE_IMAGE"), QStringLiteral("Сохранить &изображение…"),
        QStringLiteral("Сохранить картинку вида в файл PNG/JPEG (вместо File → Save)"), {K(Qt::CTRL | Qt::Key_S)},
        [this](FieldPane *) { saveImage(); });
    cmd(QStringLiteral("ID_FILE_SAVE_AS"), QStringLiteral("Сохранить &такты серией…"),
        QStringLiteral("Записать все такты в файл .ex.frames (вместо File → Save As)"), {},
        [this](FieldPane *) { saveSeries(); });
    cmd(QStringLiteral("ID_EXPORT_ANIMATION"), QStringLiteral("&Экспорт анимации (GIF / PNG)…"),
        QStringLiteral("Записать такты анимацией GIF или кадрами PNG"), {K(Qt::CTRL | Qt::SHIFT | Qt::Key_E)},
        [this](FieldPane *) { exportAnimation(); });
    cmd(QStringLiteral("ID_FILE_PRINT"), QStringLiteral("&Печать…"), QStringLiteral("Напечатать изображение (File → Print)"),
        {K(K::Print)}, [this](FieldPane *) { print(false); });
    cmd(QStringLiteral("ID_FILE_PRINT_PREVIEW"), QStringLiteral("Предварительный про&смотр"),
        QStringLiteral("Как будет выглядеть страница (File → Print Preview)"), {}, [this](FieldPane *) { print(true); });
    appCmd(QStringLiteral("ID_FILE_PRINT_SETUP"), QStringLiteral("Пара&метры страницы…"),
           QStringLiteral("Принтер и страница (File → Print Setup)"), {}, [this]() { pageSetup(); });
    appCmd(QStringLiteral("ID_APP_EXIT"), QStringLiteral("В&ыход"), QStringLiteral("Закрыть программу (File → Exit)"),
           {K(Qt::ALT | Qt::Key_F4)}, [this]() { close(); });
    // --- Edit.
    cmd(QStringLiteral("ID_EDIT_COPY"), QStringLiteral("&Копировать изображение"),
        QStringLiteral("Картинка вида — в буфер обмена (Edit → Copy)"), {K(K::Copy), K(Qt::CTRL | Qt::Key_Insert)},
        [this](FieldPane *) { copyImage(); });

    // --- View.
    cmd(QStringLiteral("ID_VIEW_PARAMETERS"), QStringLiteral("&Параметры…"), QStringLiteral("Параметры вида (View → Parameters)"),
        {K(Qt::ALT | Qt::Key_P)}, [this](FieldPane *) { showParameters(); });
    // Topology.
    cmd(QStringLiteral("ID_FLAGDRAWLINETOP"), QStringLiteral("&Контуры"), QStringLiteral("Топология: контуры блоков (Topology → Line)"),
        {K(Qt::SHIFT | Qt::Key_L)}, [](FieldPane *p) { p->toggle(p->settings().topo.line); },
        [](FieldPane *p) { return p->settings().topo.line; });
    cmd(QStringLiteral("ID_FLAGDRAWSURFACETOP"), QStringLiteral("&Заливка"), QStringLiteral("Топология: заливка блоков (Topology → Surface)"),
        {K(Qt::SHIFT | Qt::Key_S)}, [](FieldPane *p) { p->toggle(p->settings().topo.surface); },
        [](FieldPane *p) { return p->settings().topo.surface; });
    const PaneBool topoOn = [](FieldPane *p) { return p->settings().topo.line || p->settings().topo.surface; };
    cmd(QStringLiteral("ID_DIMENSION23SWITCHTOP"), QStringLiteral("&Объёмно (2D/3D)"),
        QStringLiteral("Топология: плоско или объёмными блоками (Topology → Dimension 2/3)"), {K(Qt::SHIFT | Qt::Key_D)},
        [](FieldPane *p) { p->toggle(p->settings().topo.dim3d); }, [](FieldPane *p) { return p->settings().topo.dim3d; }, topoOn);
    cmd(QStringLiteral("ID_VIEW_TOPOLOGY_DIMENSIONS_ALONGANXAXIES"), QStringLiteral("Размер по оси &X"),
        QStringLiteral("Размерная линия вдоль оси X (Topology → Size → along an X axies)"), {K(Qt::CTRL | Qt::Key_J)},
        [](FieldPane *p) { p->toggle(p->settings().place.drawSizeX); }, [](FieldPane *p) { return p->settings().place.drawSizeX; });
    cmd(QStringLiteral("ID_VIEW_TOPOLOGY_DIMENSIONS_ALONGANYAXIES"), QStringLiteral("Размер по оси &Y"),
        QStringLiteral("Размерная линия вдоль оси Y (Topology → Size → along an Y axies)"), {K(Qt::SHIFT | Qt::Key_J)},
        [](FieldPane *p) { p->toggle(p->settings().place.drawSizeY); }, [](FieldPane *p) { return p->settings().place.drawSizeY; });
    cmd(QStringLiteral("ID_VIEW_TOPOLOGY_SIZE_BLOCK"), QStringLiteral("Размеры &блоков"),
        QStringLiteral("Размеры блоков топологии (Topology → Dimensions → Block)"), {K(Qt::Key_K)},
        [](FieldPane *p) { p->toggle(p->settings().topo.sizeBlock); }, [](FieldPane *p) { return p->settings().topo.sizeBlock; }, topoOn);
    cmd(QStringLiteral("ID_VIEW_TOPOLOGY_SIZE_LINKLIST"), QStringLiteral("&Привязки блоков"),
        QStringLiteral("Привязка блоков X0, Y0 (Topology → Dimensions → Link_List)"), {K(Qt::ALT | Qt::Key_K)},
        [](FieldPane *p) { p->toggle(p->settings().topo.sizeLinkList); }, [](FieldPane *p) { return p->settings().topo.sizeLinkList; }, topoOn);
    cmd(QStringLiteral("ID_VIEW_TOPOLOGY_BOUNDARY_EPS"), QStringLiteral("&Диэлектрик (Eps)"),
        QStringLiteral("Показывать блоки диэлектрика (Boundary → Eps)"), {K(Qt::Key_E)},
        [](FieldPane *p) { p->toggle(p->settings().topo.eps); }, [](FieldPane *p) { return p->settings().topo.eps; }, topoOn);
    cmd(QStringLiteral("ID_VIEW_TOPOLOGY_BOUNDARY_METAL"), QStringLiteral("&Металл"),
        QStringLiteral("Показывать металл (Boundary → Metal)"), {K(Qt::Key_M)},
        [](FieldPane *p) { p->toggle(p->settings().topo.metal); }, [](FieldPane *p) { return p->settings().topo.metal; }, topoOn);
    cmd(QStringLiteral("ID_VIEW_TOPOLOGY_BOUNDARY_MAGNETIC"), QStringLiteral("Ма&гнитная стенка"),
        QStringLiteral("Показывать магнитные стенки (Boundary → Magnetic)"), {K(Qt::Key_N)},
        [](FieldPane *p) { p->toggle(p->settings().topo.magnetic); }, [](FieldPane *p) { return p->settings().topo.magnetic; }, topoOn);
    cmd(QStringLiteral("ID_VIEW_TOPOLOGY_BOUNDARY_ABSORBER"), QStringLiteral("П&оглотитель"),
        QStringLiteral("Показывать поглотитель (Boundary → Absorber)"), {K(Qt::Key_A)},
        [](FieldPane *p) { p->toggle(p->settings().topo.absorber); }, [](FieldPane *p) { return p->settings().topo.absorber; }, topoOn);
    cmd(QStringLiteral("ID_VIEW_TOPOLOGY_BOUNDARY_INPUT"), QStringLiteral("&Входы (порты)"),
        QStringLiteral("Показывать входы и их номера «Inp N» (Boundary → Input)"), {K(Qt::Key_I)},
        [](FieldPane *p) { p->toggle(p->settings().topo.input); }, [](FieldPane *p) { return p->settings().topo.input; }, topoOn);
    cmd(QStringLiteral("ID_VIEW_TOPOLOGY_BOUNDARY_PLASMA"), QStringLiteral("П&лазма (контуры)"),
        QStringLiteral("Новое: контуры плазменных блоков X-моды (RECT_STAT_N/_B/_Y …)"), {},
        [](FieldPane *p) { p->toggle(p->settings().topo.plasma); }, [](FieldPane *p) { return p->settings().topo.plasma; }, topoOn);
    // Eps.
    cmd(QStringLiteral("ID_FLAGDRAWLINEEPS"), QStringLiteral("&Сетка"), QStringLiteral("Слой ε: сетка (Eps → Line)"),
        {K(Qt::ALT | Qt::Key_L)}, [](FieldPane *p) { p->toggle(p->settings().eps.line); },
        [](FieldPane *p) { return p->settings().eps.line; });
    cmd(QStringLiteral("ID_FLAGDRAWSURFACEEPS"), QStringLiteral("&Заливка"), QStringLiteral("Слой ε: заливка (Eps → Surface)"),
        {K(Qt::ALT | Qt::Key_S)}, [](FieldPane *p) { p->toggle(p->settings().eps.surface); },
        [](FieldPane *p) { return p->settings().eps.surface; });
    cmd(QStringLiteral("ID_DIMENSION23SWITCHEPS"), QStringLiteral("&Рельеф (2D/3D)"), QStringLiteral("Слой ε: плоско или рельефом (Eps → Dimension 2/3)"),
        {K(Qt::ALT | Qt::Key_D)}, [](FieldPane *p) { p->toggle(p->settings().eps.dim3d); },
        [](FieldPane *p) { return p->settings().eps.dim3d; },
        [](FieldPane *p) { return p->settings().eps.line || p->settings().eps.surface; });
    cmd(QStringLiteral("ID_VIEW_EPS_SIZE"), QStringLiteral("&Шкала Z"), QStringLiteral("Слой ε: шкала по оси Z в сцене (Eps → Size)"),
        {K(Qt::ALT | Qt::Key_J)}, [](FieldPane *p) { p->toggle(p->settings().eps.drawZSize); },
        [](FieldPane *p) { return p->settings().eps.drawZSize; });
    const char *typeIds[4] = {"ID_EPS_TYPE_EPS", "ID_EPS_TYPE_N", "ID_EPS_TYPE_Y", "ID_EPS_TYPE_B"};
    const char *typeText[4] = {"ε — проницаемость (Eps+)", "N — концентрация электронов", "Y — потери", "B — магнитное поле"};
    for (int i = 0; i < 4; ++i) {
        cmd(QString::fromLatin1(typeIds[i]), QString::fromUtf8(typeText[i]),
            QStringLiteral("Слой ε показывает: %1 (Eps → Type)").arg(QString::fromUtf8(typeText[i])), {},
            [i](FieldPane *p) { p->setEpsType(i); },
            [i](FieldPane *p) { return p->settings().eps.type == i; },
            [i](FieldPane *p) { return i == 0 || p->hasPlasma(); });
    }
    // Field.
    cmd(QStringLiteral("ID_FLAGDRAWLINE"), QStringLiteral("&Сетка"), QStringLiteral("Поле: сетка (Field → Line)"),
        {K(Qt::Key_L)}, [](FieldPane *p) { p->toggle(p->settings().field.line); },
        [](FieldPane *p) { return p->settings().field.line; });
    cmd(QStringLiteral("ID_FLAGDRAWSURFACE"), QStringLiteral("&Заливка"), QStringLiteral("Поле: заливка (Field → Surface)"),
        {K(Qt::Key_S)}, [](FieldPane *p) { p->toggle(p->settings().field.surface); },
        [](FieldPane *p) { return p->settings().field.surface; });
    cmd(QStringLiteral("ID_DIMENSION23SWITCH"), QStringLiteral("&Рельеф (2D/3D)"), QStringLiteral("Поле: плоско или рельефом (Field → Dimension 2/3)"),
        {K(Qt::Key_D)}, [](FieldPane *p) { p->toggle(p->settings().field.dim3d); },
        [](FieldPane *p) { return p->settings().field.dim3d; },
        [](FieldPane *p) { return p->settings().field.line || p->settings().field.surface; });
    cmd(QStringLiteral("ID_VIEW_FIELD_SIZE"), QStringLiteral("Шкала &Z"), QStringLiteral("Поле: шкала по оси Z в сцене (Field → Size)"),
        {K(Qt::Key_J)}, [](FieldPane *p) { p->toggle(p->settings().field.drawZSize); },
        [](FieldPane *p) { return p->settings().field.drawZSize; });
    cmd(QStringLiteral("ID_FIELD_L"), QStringLiteral("&Уровни цвета (значения)"),
        QStringLiteral("Окно значений для уровней цвета (Field → Level)"), {K(Qt::ALT | Qt::Key_E)},
        [this](FieldPane *) { m_levelsDock->setVisible(!m_levelsDock->isVisible()); },
        [this](FieldPane *) { return m_levelsDock && m_levelsDock->isVisible(); });
    cmd(QStringLiteral("ID_FIELD_VALUE"), QStringLiteral("&Значение в точке"),
        QStringLiteral("Метка и значение поля в точке; стрелки двигают точку, Shift+щелчок ставит её (Field → Value)"),
        {K(Qt::ALT | Qt::Key_A)}, [](FieldPane *p) { p->toggle(p->settings().probe); },
        [](FieldPane *p) { return p->settings().probe; });
    cmd(QStringLiteral("ID_FIELD_MODUL"), QStringLiteral("&Модуль"), QStringLiteral("Поле: показывать модуль |E| (Field → Modul)"),
        {K(Qt::ALT | Qt::Key_M)}, [](FieldPane *p) { p->toggle(p->settings().field.modulus); },
        [](FieldPane *p) { return p->settings().field.modulus; });
    const char *scaleIds[4] = {"ID_FIELD_SCALE_AUTO", "ID_FIELD_SCALE_WAVE", "ID_FIELD_SCALE_GLOW", "ID_FIELD_SCALE_CUSTOM"};
    const char *scaleText[4] = {"Шкала: авто", "Шкала «Волна»", "Шкала «Свечение»", "Шкала: 20 цветов уровней"};
    for (int i = 0; i < 4; ++i) {
        cmd(QString::fromLatin1(scaleIds[i]), QString::fromUtf8(scaleText[i]), QStringLiteral("Новое: цветовая шкала поля"), {},
            [i](FieldPane *p) {
                ViewSettings s = p->settings();
                s.field.scale = ScaleKind(i);
                p->applySettings(s);
            },
            [i](FieldPane *p) { return int(p->settings().field.scale) == i; });
    }
    cmd(QStringLiteral("ID_FIELD_SMOOTH"), QStringLiteral("Плавная шкала (256 уровней)"),
        QStringLiteral("Плавная шкала или 20 уровней цвета (256 Resolution)"), {},
        [](FieldPane *p) { p->toggle(p->settings().field.smooth); }, [](FieldPane *p) { return p->settings().field.smooth; });
    cmd(QStringLiteral("ID_FIELD_ZPERFRAME"), QStringLiteral("Пределы по текущему такту"),
        QStringLiteral("Новое: пределы шкалы по текущему такту (иначе — по всем показанным)"), {},
        [](FieldPane *p) { p->toggle(p->settings().field.zPerFrame); }, [](FieldPane *p) { return p->settings().field.zPerFrame; });
    // Zoom.
    cmd(QStringLiteral("ID_VIEW_ZOOM_P"), QStringLiteral("Увеличить"), QStringLiteral("Увеличить: окно данных вдвое меньше (Zoom +)"),
        {K(Qt::Key_Plus), K(Qt::Key_Equal)}, [](FieldPane *p) { p->zoom(0); });
    cmd(QStringLiteral("ID_VIEW_ZOOM_PX"), QStringLiteral("Увеличить по X"), QStringLiteral("Увеличить по оси X (Zoom +X)"),
        {K(Qt::Key_X)}, [](FieldPane *p) { p->zoom(1); });
    cmd(QStringLiteral("ID_VIEW_ZOOM_PY"), QStringLiteral("Увеличить по Y"), QStringLiteral("Увеличить по оси Y (Zoom +Y)"),
        {K(Qt::Key_Y)}, [](FieldPane *p) { p->zoom(2); });
    cmd(QStringLiteral("ID_VIEW_ZOOM_M"), QStringLiteral("Уменьшить"), QStringLiteral("Уменьшить: окно данных вдвое больше (Zoom −)"),
        {K(Qt::Key_Minus)}, [](FieldPane *p) { p->zoom(3); });
    cmd(QStringLiteral("ID_VIEW_ZOOM_MX"), QStringLiteral("Уменьшить по X"), QStringLiteral("Уменьшить по оси X (Zoom −X)"),
        {K(Qt::SHIFT | Qt::Key_X)}, [](FieldPane *p) { p->zoom(4); });
    cmd(QStringLiteral("ID_VIEW_ZOOM_MY"), QStringLiteral("Уменьшить по Y"), QStringLiteral("Уменьшить по оси Y (Zoom −Y)"),
        {K(Qt::SHIFT | Qt::Key_Y)}, [](FieldPane *p) { p->zoom(5); });
    // Rotate.
    cmd(QStringLiteral("ID_ROTATE_LEFT"), QStringLiteral("Влево"), QStringLiteral("Поворот вокруг Y на −2° (Rotate → Left)"),
        {keypad(Qt::CTRL, Qt::Key_4)}, [](FieldPane *p) { p->rotate(1, -2); });
    cmd(QStringLiteral("ID_ROTATE_RIGHT"), QStringLiteral("Вправо"), QStringLiteral("Поворот вокруг Y на +2° (Rotate → Right)"),
        {keypad(Qt::CTRL, Qt::Key_6)}, [](FieldPane *p) { p->rotate(1, 2); });
    cmd(QStringLiteral("ID_ROTATE_TOP"), QStringLiteral("Вверх"), QStringLiteral("Поворот вокруг X на −2° (Rotate → Top)"),
        {keypad(Qt::CTRL, Qt::Key_8)}, [](FieldPane *p) { p->rotate(0, -2); });
    cmd(QStringLiteral("ID_ROTATE_BOTTOM"), QStringLiteral("Вниз"), QStringLiteral("Поворот вокруг X на +2° (Rotate → Bottom)"),
        {keypad(Qt::CTRL, Qt::Key_2)}, [](FieldPane *p) { p->rotate(0, 2); });
    cmd(QStringLiteral("ID_ROTATE_LEFTY"), QStringLiteral("Против часовой (Z)"), QStringLiteral("Поворот вокруг Z на −2° (Rotate → Left_y)"),
        {keypad(Qt::CTRL, Qt::Key_1), keypad(Qt::CTRL, Qt::Key_3)}, [](FieldPane *p) { p->rotate(2, -2); });
    cmd(QStringLiteral("ID_ROTATE_RIGHTY"), QStringLiteral("По часовой (Z)"), QStringLiteral("Поворот вокруг Z на +2° (Rotate → Right_y)"),
        {keypad(Qt::CTRL, Qt::Key_7), keypad(Qt::CTRL, Qt::Key_9)}, [](FieldPane *p) { p->rotate(2, 2); });
    // Translate (оси — как в обработчиках OnTranslate* оригинала).
    cmd(QStringLiteral("ID_TRANSLATE_XT"), QStringLiteral("X вправо"), QStringLiteral("Сдвиг сцены по X + (Translate → X top)"),
        {keypad(Qt::ALT, Qt::Key_6)}, [](FieldPane *p) { p->translate(0, 0.04); });
    cmd(QStringLiteral("ID_TRANSLATE_XB"), QStringLiteral("X влево"), QStringLiteral("Сдвиг сцены по X − (Translate → X bottom)"),
        {keypad(Qt::ALT, Qt::Key_4)}, [](FieldPane *p) { p->translate(0, -0.04); });
    cmd(QStringLiteral("ID_TRANSLATE_YT"), QStringLiteral("Ближе"), QStringLiteral("Сцена ближе к наблюдателю (Translate → Y top)"),
        {keypad(Qt::ALT, Qt::Key_7), keypad(Qt::ALT, Qt::Key_9)}, [](FieldPane *p) { p->translate(2, 0.04); });
    cmd(QStringLiteral("ID_TRANSLATE_YB"), QStringLiteral("Дальше"), QStringLiteral("Сцена дальше от наблюдателя (Translate → Y bottom)"),
        {keypad(Qt::ALT, Qt::Key_1), keypad(Qt::ALT, Qt::Key_3)}, [](FieldPane *p) { p->translate(2, -0.04); });
    cmd(QStringLiteral("ID_TRANSLATE_ZT"), QStringLiteral("Вверх"), QStringLiteral("Сдвиг сцены вверх (Translate → Z top)"),
        {keypad(Qt::ALT, Qt::Key_8)}, [](FieldPane *p) { p->translate(1, 0.04); });
    cmd(QStringLiteral("ID_TRANSLATE_ZB"), QStringLiteral("Вниз"), QStringLiteral("Сдвиг сцены вниз (Translate → Z bottom)"),
        {keypad(Qt::ALT, Qt::Key_2)}, [](FieldPane *p) { p->translate(1, -0.04); });
    // Change Xmax, Xmin / Ymax, Ymin.
    cmd(QStringLiteral("ID_STRELKLEFT"), QStringLiteral("Окно данных влево"), QStringLiteral("Xmin, Xmax − Δ (или точка — влево в режиме Value)"),
        {K(Qt::Key_Left)}, [](FieldPane *p) { p->arrow(-1, 0); });
    cmd(QStringLiteral("ID_STRELKRIGHT"), QStringLiteral("Окно данных вправо"), QStringLiteral("Xmin, Xmax + Δ (или точка — вправо)"),
        {K(Qt::Key_Right)}, [](FieldPane *p) { p->arrow(1, 0); });
    cmd(QStringLiteral("ID_STRELKDOWN"), QStringLiteral("Окно данных вниз"), QStringLiteral("Ymin, Ymax − Δ (или точка — вниз)"),
        {K(Qt::Key_Down)}, [](FieldPane *p) { p->arrow(0, -1); });
    cmd(QStringLiteral("ID_STRELKUP"), QStringLiteral("Окно данных вверх"), QStringLiteral("Ymin, Ymax + Δ (или точка — вверх)"),
        {K(Qt::Key_Up)}, [](FieldPane *p) { p->arrow(0, 1); });
    cmd(QStringLiteral("ID_VIEW_DEFAULT12"), QStringLiteral("Вид по &умолчанию"), QStringLiteral("Все параметры вида — по умолчанию (View → Default)"),
        {K(Qt::CTRL | Qt::Key_D)}, [](FieldPane *p) { p->setDefault(); });
    cmd(QStringLiteral("ID_VIEW_RESET_CAMERA"), QStringLiteral("Сбросить &камеру"), QStringLiteral("Новое: только поворот, сдвиг и масштаб — по умолчанию"),
        {K(Qt::Key_Home)}, [](FieldPane *p) { p->resetView(); });
    cmd(QStringLiteral("ID_VIEW_TOP"), QStringLiteral("Вид &сверху"), QStringLiteral("Новое: смотреть на поле сверху (углы поворота 0, как в Windows-версии по умолчанию)"),
        {K(Qt::CTRL | Qt::Key_T)}, [](FieldPane *p) {
            ViewSettings s = p->settings();
            s.camera.angleX = s.camera.angleY = s.camera.angleZ = 0.0;
            p->applySettings(s);
        });
    cmd(QStringLiteral("ID_VIEW_PROPORTIONALLY"), QStringLiteral("&Пропорционально"), QStringLiteral("Одинаковый масштаб осей X и Y (View → Proportionally)"),
        {K(Qt::Key_P)}, [](FieldPane *p) { p->toggle(p->settings().place.proportional); },
        [](FieldPane *p) { return p->settings().place.proportional; });
    cmd(QStringLiteral("ID_VIEW_REREAD"), QStringLiteral("Перечитать файл"), QStringLiteral("Перечитать файл и пределы шкалы (двойной щелчок по виду)"),
        {K(Qt::Key_F5)}, [](FieldPane *p) { p->resetZRange(); p->reread(); });

    // --- Такты (новое).
    cmd(QStringLiteral("ID_ANIM_PLAY"), QStringLiteral("Пуск / пауза"), QStringLiteral("Проигрывать такты"),
        {K(Qt::Key_Space)}, [](FieldPane *p) { p->setPlaying(!p->playing()); }, [](FieldPane *p) { return p->playing(); },
        [](FieldPane *p) { return p->frameCount() > 1; });
    cmd(QStringLiteral("ID_ANIM_PREV"), QStringLiteral("Предыдущий такт"), QStringLiteral("Шаг назад"),
        {K(Qt::Key_Comma)}, [](FieldPane *p) { p->setPlaying(false); p->stepFrame(-1); }, nullptr,
        [](FieldPane *p) { return p->frameCount() > 1; });
    cmd(QStringLiteral("ID_ANIM_NEXT"), QStringLiteral("Следующий такт"), QStringLiteral("Шаг вперёд"),
        {K(Qt::Key_Period)}, [](FieldPane *p) { p->setPlaying(false); p->stepFrame(1); }, nullptr,
        [](FieldPane *p) { return p->frameCount() > 1; });
    cmd(QStringLiteral("ID_ANIM_FIRST"), QStringLiteral("Первый такт"), QStringLiteral("К первому такту"),
        {K(Qt::CTRL | Qt::Key_Home)}, [](FieldPane *p) { p->setPlaying(false); p->setFrame(0); }, nullptr,
        [](FieldPane *p) { return p->frameCount() > 1; });
    cmd(QStringLiteral("ID_ANIM_LAST"), QStringLiteral("Последний такт"), QStringLiteral("К последнему такту (и дальше — за расчётом)"),
        {K(Qt::CTRL | Qt::Key_End)}, [](FieldPane *p) { p->setPlaying(false); p->setFrame(p->frameCount() - 1); }, nullptr,
        [](FieldPane *p) { return p->frameCount() > 1; });
    QAction *sync = cmd(QStringLiteral("ID_SYNC_KERNEL"), QStringLiteral("Следить за расчётом"),
        QStringLiteral("Синхронизация с работающим ядром: читать каждый новый кадр поля (зелёный — включено)"), {},
        [](FieldPane *p) { p->document()->setSyncEnabled(!p->document()->syncEnabled()); },
        [](FieldPane *p) { return p->document()->syncEnabled(); });
    sync->setIcon(syncIcon(true));
    cmd(QStringLiteral("ID_SYNC_RECORD"), QStringLiteral("Запоминать такты"),
        QStringLiteral("Сохранять в памяти каждый кадр от ядра, чтобы потом проиграть"), {},
        [](FieldPane *p) { p->document()->setRecordHistory(!p->document()->recordHistory()); },
        [](FieldPane *p) { return p->document()->recordHistory(); });
    cmd(QStringLiteral("ID_SYNC_CLEAR"), QStringLiteral("Забыть такты"), QStringLiteral("Удалить из памяти все такты, кроме последнего"), {},
        [](FieldPane *p) { p->document()->clearHistory(); }, nullptr,
        [](FieldPane *p) { return p->frameCount() > 1 && !p->document()->isSeriesFile(); });

    // --- Window.
    appCmd(QStringLiteral("ID_WINDOW_NEW"), QStringLiteral("&Новое окно"), QStringLiteral("Ещё одно окно того же документа (Window → New Window)"),
           {}, [this]() { newWindow(); });
    appCmd(QStringLiteral("ID_WINDOW_CASCADE"), QStringLiteral("&Каскадом"), QStringLiteral("Окна каскадом (Window → Cascade)"),
           {}, [this]() { m_mdi->cascadeSubWindows(); });
    appCmd(QStringLiteral("ID_WINDOW_TILE_HORZ"), QStringLiteral("&Мозаикой"), QStringLiteral("Окна мозаикой (Window → Tile)"),
           {}, [this]() { m_mdi->tileSubWindows(); });
    appCmd(QStringLiteral("ID_WINDOW_NEXT"), QStringLiteral("Следующее окно"), QStringLiteral("Следующее окно (F6)"),
           {K(Qt::Key_F6), K(Qt::CTRL | Qt::Key_Tab)}, [this]() { m_mdi->activateNextSubWindow(); });
    appCmd(QStringLiteral("ID_WINDOW_PREV"), QStringLiteral("Предыдущее окно"), QStringLiteral("Предыдущее окно (Shift+F6)"),
           {K(Qt::SHIFT | Qt::Key_F6)}, [this]() { m_mdi->activatePreviousSubWindow(); });
    // --- Help.
    appCmd(QStringLiteral("ID_APP_ABOUT"), QStringLiteral("&О программе FieldView…"), QStringLiteral("Версия и сведения о программе (Help → About)"),
           {}, [this]() { about(); });
    appCmd(QStringLiteral("ID_HELP_MANUAL"), QStringLiteral("&Руководство пользователя"), QStringLiteral("Открыть руководство пользователя"),
           {K(Qt::Key_F1)}, [this]() { openManual(); });
}

void MainWindow::buildMenus()
{
    const auto A = [this](const char *id) { return m_actions.value(QString::fromLatin1(id)); };
    QMenu *file = menuBar()->addMenu(QStringLiteral("&Файл"));
    file->addAction(A("ID_FILE_OPEN"));
    file->addAction(A("ID_FILE_CLOSE"));
    file->addSeparator();
    file->addAction(A("ID_FILE_SAVE_IMAGE"));
    file->addAction(A("ID_FILE_SAVE_AS"));
    file->addAction(A("ID_EXPORT_ANIMATION"));
    file->addSeparator();
    file->addAction(A("ID_FILE_PRINT"));
    file->addAction(A("ID_FILE_PRINT_PREVIEW"));
    file->addAction(A("ID_FILE_PRINT_SETUP"));
    file->addSeparator();
    m_recentMenu = file->addMenu(QStringLiteral("Последние файлы"));
    file->addSeparator();
    file->addAction(A("ID_APP_EXIT"));
    updateRecentMenu();

    QMenu *edit = menuBar()->addMenu(QStringLiteral("&Правка"));
    edit->addAction(A("ID_EDIT_COPY"));

    QMenu *view = menuBar()->addMenu(QStringLiteral("&Вид"));
    view->addAction(A("ID_VIEW_PARAMETERS"));
    view->addSeparator();
    QMenu *topo = view->addMenu(QStringLiteral("&Топология"));
    topo->addAction(A("ID_FLAGDRAWLINETOP"));
    topo->addAction(A("ID_FLAGDRAWSURFACETOP"));
    topo->addAction(A("ID_DIMENSION23SWITCHTOP"));
    QMenu *tsize = topo->addMenu(QStringLiteral("Размер вдоль осей (Size)"));
    tsize->addAction(A("ID_VIEW_TOPOLOGY_DIMENSIONS_ALONGANXAXIES"));
    tsize->addAction(A("ID_VIEW_TOPOLOGY_DIMENSIONS_ALONGANYAXIES"));
    QMenu *tdim = topo->addMenu(QStringLiteral("Размеры блоков (Dimensions)"));
    tdim->addAction(A("ID_VIEW_TOPOLOGY_SIZE_BLOCK"));
    tdim->addAction(A("ID_VIEW_TOPOLOGY_SIZE_LINKLIST"));
    QMenu *tb = topo->addMenu(QStringLiteral("Границы (Boundary)"));
    for (const char *id : {"ID_VIEW_TOPOLOGY_BOUNDARY_EPS", "ID_VIEW_TOPOLOGY_BOUNDARY_METAL", "ID_VIEW_TOPOLOGY_BOUNDARY_MAGNETIC",
                           "ID_VIEW_TOPOLOGY_BOUNDARY_ABSORBER", "ID_VIEW_TOPOLOGY_BOUNDARY_INPUT", "ID_VIEW_TOPOLOGY_BOUNDARY_PLASMA"})
        tb->addAction(A(id));
    QMenu *eps = view->addMenu(QStringLiteral("&ε и плазма (Eps)"));
    eps->addAction(A("ID_FLAGDRAWLINEEPS"));
    eps->addAction(A("ID_FLAGDRAWSURFACEEPS"));
    eps->addAction(A("ID_DIMENSION23SWITCHEPS"));
    eps->addAction(A("ID_VIEW_EPS_SIZE"));
    QMenu *et = eps->addMenu(QStringLiteral("Величина (Type)"));
    auto *etg = new QActionGroup(this);
    for (const char *id : {"ID_EPS_TYPE_EPS", "ID_EPS_TYPE_N", "ID_EPS_TYPE_Y", "ID_EPS_TYPE_B"}) {
        et->addAction(A(id));
        etg->addAction(A(id));
    }
    QMenu *fld = view->addMenu(QStringLiteral("&Поле (Field)"));
    fld->addAction(A("ID_FLAGDRAWLINE"));
    fld->addAction(A("ID_FLAGDRAWSURFACE"));
    fld->addAction(A("ID_DIMENSION23SWITCH"));
    fld->addAction(A("ID_VIEW_FIELD_SIZE"));
    fld->addAction(A("ID_FIELD_L"));
    fld->addAction(A("ID_FIELD_VALUE"));
    fld->addAction(A("ID_FIELD_MODUL"));
    fld->addSeparator();
    QMenu *sc = fld->addMenu(QStringLiteral("Цветовая шкала"));
    auto *scg = new QActionGroup(this);
    for (const char *id : {"ID_FIELD_SCALE_AUTO", "ID_FIELD_SCALE_WAVE", "ID_FIELD_SCALE_GLOW", "ID_FIELD_SCALE_CUSTOM"}) {
        sc->addAction(A(id));
        scg->addAction(A(id));
    }
    sc->addSeparator();
    sc->addAction(A("ID_FIELD_SMOOTH"));
    fld->addAction(A("ID_FIELD_ZPERFRAME"));
    view->addSeparator();
    QMenu *zoom = view->addMenu(QStringLiteral("&Масштаб (Zoom)"));
    for (const char *id : {"ID_VIEW_ZOOM_P", "ID_VIEW_ZOOM_PX", "ID_VIEW_ZOOM_PY", "ID_VIEW_ZOOM_M", "ID_VIEW_ZOOM_MX", "ID_VIEW_ZOOM_MY"})
        zoom->addAction(A(id));
    QMenu *rot = view->addMenu(QStringLiteral("П&оворот (Rotate)"));
    rot->addAction(A("ID_ROTATE_LEFT"));
    rot->addAction(A("ID_ROTATE_RIGHT"));
    rot->addSeparator();
    rot->addAction(A("ID_ROTATE_TOP"));
    rot->addAction(A("ID_ROTATE_BOTTOM"));
    rot->addSeparator();
    rot->addAction(A("ID_ROTATE_LEFTY"));
    rot->addAction(A("ID_ROTATE_RIGHTY"));
    QMenu *tr = view->addMenu(QStringLiteral("С&двиг (Translate)"));
    tr->addAction(A("ID_TRANSLATE_XT"));
    tr->addAction(A("ID_TRANSLATE_XB"));
    tr->addSeparator();
    tr->addAction(A("ID_TRANSLATE_YT"));
    tr->addAction(A("ID_TRANSLATE_YB"));
    tr->addSeparator();
    tr->addAction(A("ID_TRANSLATE_ZT"));
    tr->addAction(A("ID_TRANSLATE_ZB"));
    view->addSeparator();
    QMenu *cx = view->addMenu(QStringLiteral("Окно данных по X (Change Xmax, Xmin)"));
    cx->addAction(A("ID_STRELKLEFT"));
    cx->addAction(A("ID_STRELKRIGHT"));
    QMenu *cy = view->addMenu(QStringLiteral("Окно данных по Y (Change Ymax, Ymin)"));
    cy->addAction(A("ID_STRELKDOWN"));
    cy->addAction(A("ID_STRELKUP"));
    view->addSeparator();
    view->addAction(A("ID_VIEW_DEFAULT12"));
    view->addAction(A("ID_VIEW_RESET_CAMERA"));
    view->addAction(A("ID_VIEW_TOP"));
    view->addAction(A("ID_VIEW_REREAD"));
    view->addSeparator();
    view->addAction(A("ID_VIEW_PROPORTIONALLY"));
    view->addSeparator();
    QMenu *panels = view->addMenu(QStringLiteral("Панели"));
    // Заполняется в buildToolbars / buildStatusBar / buildLevelsDock.
    panels->setObjectName(QStringLiteral("panelsMenu"));
#ifdef TMC_FV_COMMON_ABOUT
    view->addSeparator();
    tmci18n::addLanguageMenu(view);   // язык интерфейса
#endif

    QMenu *anim = menuBar()->addMenu(QStringLiteral("&Такты"));
    for (const char *id : {"ID_ANIM_PLAY", "ID_ANIM_PREV", "ID_ANIM_NEXT", "ID_ANIM_FIRST", "ID_ANIM_LAST"})
        anim->addAction(A(id));
    anim->addSeparator();
    anim->addAction(A("ID_SYNC_KERNEL"));
    anim->addAction(A("ID_SYNC_RECORD"));
    anim->addAction(A("ID_SYNC_CLEAR"));
    anim->addSeparator();
    anim->addAction(A("ID_EXPORT_ANIMATION"));
    anim->addAction(A("ID_FILE_SAVE_AS"));

    m_windowMenu = menuBar()->addMenu(QStringLiteral("&Окно"));
    connect(m_windowMenu, &QMenu::aboutToShow, this, &MainWindow::updateWindowMenu);
    updateWindowMenu();

    QMenu *help = menuBar()->addMenu(QStringLiteral("&Справка"));
    help->addAction(A("ID_HELP_MANUAL"));
    help->addAction(A("ID_APP_ABOUT"));
}

void MainWindow::buildToolbars()
{
    const auto A = [this](const char *id) { return m_actions.value(QString::fromLatin1(id)); };
    // Панель Windows-версии (IDR_MAINFRAME TOOLBAR), тот же порядок и группы.
    m_mainBar = addToolBar(QStringLiteral("Основная панель"));
    m_mainBar->setObjectName(QStringLiteral("mainToolBar"));
    m_mainBar->setIconSize(QSize(20, 20));
    m_mainBar->setMovable(true);
    const QList<QList<const char *>> groups = {
        {"ID_FILE_OPEN"},
        {"ID_VIEW_ZOOM_P", "ID_VIEW_ZOOM_PX", "ID_VIEW_ZOOM_PY"},
        {"ID_VIEW_ZOOM_M", "ID_VIEW_ZOOM_MX", "ID_VIEW_ZOOM_MY"},
        {"ID_FLAGDRAWLINETOP", "ID_FLAGDRAWSURFACETOP", "ID_DIMENSION23SWITCHTOP", "ID_VIEW_TOPOLOGY_BOUNDARY_EPS",
         "ID_VIEW_TOPOLOGY_BOUNDARY_METAL", "ID_VIEW_TOPOLOGY_BOUNDARY_MAGNETIC", "ID_VIEW_TOPOLOGY_BOUNDARY_ABSORBER",
         "ID_VIEW_TOPOLOGY_BOUNDARY_INPUT", "ID_VIEW_TOPOLOGY_SIZE_BLOCK", "ID_VIEW_TOPOLOGY_SIZE_LINKLIST"},
        {"ID_FLAGDRAWLINEEPS", "ID_FLAGDRAWSURFACEEPS", "ID_DIMENSION23SWITCHEPS", "ID_VIEW_EPS_SIZE", "ID_EPS_TYPE_EPS",
         "ID_EPS_TYPE_N", "ID_EPS_TYPE_B", "ID_EPS_TYPE_Y"},
        {"ID_FLAGDRAWLINE", "ID_FLAGDRAWSURFACE", "ID_DIMENSION23SWITCH", "ID_VIEW_TOPOLOGY_DIMENSIONS_ALONGANXAXIES",
         "ID_VIEW_TOPOLOGY_DIMENSIONS_ALONGANYAXIES", "ID_VIEW_FIELD_SIZE"},
        {"ID_VIEW_PROPORTIONALLY"},
        {"ID_VIEW_DEFAULT12"},
        {"ID_FILE_PRINT", "ID_APP_ABOUT"}};
    for (int g = 0; g < groups.size(); ++g) {
        if (g > 0)
            m_mainBar->addSeparator();
        for (const char *id : groups[g])
            m_mainBar->addAction(A(id));
    }

    // Такты и слежение за расчётом (новое).
    addToolBarBreak();
    m_animBar = addToolBar(QStringLiteral("Такты"));
    m_animBar->setObjectName(QStringLiteral("animToolBar"));
    m_animBar->setIconSize(QSize(20, 20));
    m_animBar->addAction(A("ID_SYNC_KERNEL"));
    m_animBar->addSeparator();
    for (const char *id : {"ID_ANIM_FIRST", "ID_ANIM_PREV", "ID_ANIM_PLAY", "ID_ANIM_NEXT", "ID_ANIM_LAST"})
        m_animBar->addAction(A(id));
    m_frameSlider = new QSlider(Qt::Horizontal);
    m_frameSlider->setMinimumWidth(260);
    m_frameSlider->setFocusPolicy(Qt::NoFocus);
    m_frameSlider->setToolTip(QStringLiteral("Такт"));
    connect(m_frameSlider, &QSlider::valueChanged, this, [this](int v) {
        if (m_updating)
            return;
        if (FieldPane *p = activePane()) {
            p->setPlaying(false);
            p->setFrame(v);
        }
    });
    m_animBar->addWidget(m_frameSlider);
    m_frameLabel = new QLabel(QStringLiteral("—"));
    m_frameLabel->setProperty("mono", true);
    m_frameLabel->setMinimumWidth(110);
    m_frameLabel->setAlignment(Qt::AlignCenter);
    m_animBar->addWidget(m_frameLabel);
    auto *speedLabel = new QLabel(QStringLiteral("кадр/с"));
    speedLabel->setProperty("dim", true);
    m_speed = new QSpinBox;
    m_speed->setRange(1, 60);
    m_speed->setValue(ViewSettings::load().speed);
    m_speed->setToolTip(QStringLiteral("Скорость проигрывания тактов"));
    m_speed->setFocusPolicy(Qt::ClickFocus);
    connect(m_speed, &QSpinBox::valueChanged, this, [this](int v) {
        if (FieldPane *p = activePane())
            p->setSpeed(v);
    });
    // Скорость проигрывания на панели не показываем (23.10.2026). В Windows-
    // версии поле приходит от счётного ядра по мере расчёта — каждый такт
    // ядро считает и передаёт, и FieldView показывает его сразу. Темп задаёт
    // расчёт, а не FieldView, так что ползунок «кадр/с» ничего не менял.
    // Частота кадров нужна только при записи GIF — она осталась в окне
    // «Экспорт анимации». Сам счётчик живёт (его читает updateAnimBar), но
    // скрыт.
    m_speed->setParent(this);
    m_speed->setVisible(false);
    delete speedLabel;
    m_animBar->addSeparator();
    auto *gainLabel = new QLabel(QStringLiteral("контраст ×"));
    gainLabel->setProperty("dim", true);
    m_gain = new QSpinBox;
    m_gain->setRange(1, 100);
    m_gain->setToolTip(QStringLiteral("Контраст раскраски и высоты: авто-пределы шкалы делятся на это число "
                                      "(значения за пределами — крайним цветом)"));
    m_gain->setFocusPolicy(Qt::ClickFocus);
    connect(m_gain, &QSpinBox::valueChanged, this, [this](int v) {
        if (m_updating)
            return;
        if (FieldPane *p = activePane())
            p->setGain(v);
    });
    m_animBar->addWidget(gainLabel);
    m_animBar->addWidget(m_gain);
    m_animBar->addSeparator();
    m_animBar->addAction(A("ID_VIEW_PARAMETERS"));
    m_animBar->addAction(A("ID_FIELD_MODUL"));
    m_animBar->addAction(A("ID_FIELD_VALUE"));
    m_animBar->addAction(A("ID_FIELD_L"));
    m_animBar->addSeparator();
    m_animBar->addAction(A("ID_FILE_SAVE_IMAGE"));
    m_animBar->addAction(A("ID_EXPORT_ANIMATION"));

    if (QMenu *panels = menuBar()->findChild<QMenu *>(QStringLiteral("panelsMenu"))) {
        QAction *t1 = m_mainBar->toggleViewAction();
        t1->setText(QStringLiteral("Основная панель (Toolbar)"));
        QAction *t2 = m_animBar->toggleViewAction();
        t2->setText(QStringLiteral("Панель тактов"));
        panels->addAction(t1);
        panels->addAction(t2);
    }
}

void MainWindow::buildStatusBar()
{
    // Строка состояния CMainFrame: панель 0 — сообщения (PutTrace), панель 1 — nT и t.
    m_stTrace = new QLabel;
    m_stStats = new QLabel;
    m_stSync = new QLabel;
    m_stBackend = new QLabel;
    statusBar()->addWidget(m_stTrace, 1);
    statusBar()->addPermanentWidget(m_stSync);
    statusBar()->addPermanentWidget(m_stStats);
    statusBar()->addPermanentWidget(m_stBackend);
    if (QMenu *panels = menuBar()->findChild<QMenu *>(QStringLiteral("panelsMenu"))) {
        auto *a = new QAction(QStringLiteral("Строка состояния (Status Bar)"), this);
        a->setCheckable(true);
        a->setChecked(true);
        connect(a, &QAction::toggled, statusBar(), &QStatusBar::setVisible);
        m_actions.insert(QStringLiteral("ID_VIEW_STATUS_BAR"), a);
        panels->addAction(a);
    }
    m_actions.insert(QStringLiteral("ID_VIEW_TOOLBAR"), m_mainBar->toggleViewAction());
}

void MainWindow::buildLevelsDock()
{
    // «Value for color level» (CColorLevelDlg): значения, с которых начинаются уровни цвета.
    m_levelsDock = new QDockWidget(QStringLiteral("Уровни цвета"), this);
    m_levelsDock->setObjectName(QStringLiteral("levelsDock"));
    m_levels = new QTableWidget(20, 2);
    m_levels->setHorizontalHeaderLabels({QStringLiteral("Цвет"), QStringLiteral("Значение от")});
    m_levels->verticalHeader()->setVisible(false);
    m_levels->horizontalHeader()->setStretchLastSection(true);
    m_levels->setColumnWidth(0, 54);
    m_levels->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_levels->setSelectionMode(QAbstractItemView::NoSelection);
    m_levels->setFont(theme::monoFont(12));
    m_levelsDock->setWidget(m_levels);
    m_levelsDock->setMinimumWidth(220);
    addDockWidget(Qt::RightDockWidgetArea, m_levelsDock);
    m_levelsDock->hide();
    connect(m_levelsDock, &QDockWidget::visibilityChanged, this, [this]() {
        updateLevels();
        updateActions();
    });
    if (QMenu *panels = menuBar()->findChild<QMenu *>(QStringLiteral("panelsMenu")))
        panels->addAction(m_levelsDock->toggleViewAction());
}

// --- Состояние -----------------------------------------------------------------

void MainWindow::connectPane(FieldPane *pane)
{
    if (m_connectedPane == pane)
        return;
    if (m_connectedPane)
        disconnect(m_connectedPane, nullptr, this, nullptr);
    m_connectedPane = pane;
    if (!pane)
        return;
    connect(pane, &FieldPane::statusChanged, this, [this]() {
        updateStatus();
        updateLevels();
    });
    connect(pane, &FieldPane::stateChanged, this, [this]() {
        updateActions();
        updateStatus();
    });
    connect(pane, &FieldPane::requestParameters, this, [this](const QPoint &) { showParameters(); });
    connect(pane->view(), &FieldRhiView::frameRendered, this, [this]() {
        static qint64 last = 0;
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - last > 500) {
            last = now;
            updateStatus();
        }
    });
}

void MainWindow::updateActions()
{
    FieldPane *p = activePane();
    m_updating = true;
    for (QAction *a : m_paneActions) {
        bool en = p != nullptr;
        if (en && m_enabled.contains(a))
            en = m_enabled.value(a)(p);
        a->setEnabled(en);
        if (m_checked.contains(a))
            a->setChecked(p ? m_checked.value(a)(p) : false);
    }
    if (QAction *s = m_actions.value(QStringLiteral("ID_SYNC_KERNEL")))
        s->setIcon(syncIcon(p && p->document()->syncEnabled()));
    if (QAction *pl = m_actions.value(QStringLiteral("ID_ANIM_PLAY")))
        pl->setIcon(commandIcon(p && p->playing() ? QStringLiteral("ID_ANIM_PAUSE") : QStringLiteral("ID_ANIM_PLAY")));
    const int n = p ? p->frameCount() : 0;
    m_frameSlider->setEnabled(n > 1);
    m_frameSlider->setRange(0, std::max(0, n - 1));
    m_frameSlider->setValue(p ? p->frame() : 0);
    m_frameLabel->setText(n > 0 ? QStringLiteral("%1 / %2").arg(p->frame() + 1).arg(n) : QStringLiteral("—"));
    if (p && m_speed->value() != p->settings().speed)
        m_speed->setValue(p->settings().speed);
    if (p)
        m_gain->setValue(int(std::lround(p->settings().field.gain)));
    m_gain->setEnabled(p != nullptr);
    m_updating = false;
    // Имя файла к заголовку при развёрнутом окне поля приписывает сама область MDI
    // («FieldView - [файл]», как в Windows-версии); своё не добавляем, иначе оно двоится.
    setWindowTitle(QStringLiteral("FieldView"));
}

void MainWindow::updateStatus()
{
    FieldPane *p = activePane();
    if (!p) {
        m_stTrace->setText(QStringLiteral("Готово. Откройте файл поля: Файл → Открыть (Ctrl+O)"));
        m_stStats->clear();
        m_stSync->clear();
        m_stBackend->clear();
        return;
    }
    m_stTrace->setText(p->traceText());
    m_stStats->setText(p->statsText());
    const auto doc = p->document();
    QString sync;
    if (doc->syncEnabled()) {
#ifdef TMC_FV_COMMON_ABOUT
        const bool ru = tmci18n::russian();
#else
        const bool ru = true;
#endif
        sync = QStringLiteral("<span style='color:#5ce08a'>●</span> ")
             + (ru ? QStringLiteral("слежение") : QStringLiteral("following"));
        if (doc->liveFramesRead() > 0)
            sync += (ru ? QStringLiteral(": %1 кадр. от ядра") : QStringLiteral(": %1 frames from the kernel"))
                        .arg(doc->liveFramesRead());
    } else {
#ifdef TMC_FV_COMMON_ABOUT
        const bool ru = tmci18n::russian();
#else
        const bool ru = true;
#endif
        sync = QStringLiteral("<span style='color:#ff6b6b'>●</span> ")
             + (ru ? QStringLiteral("слежение выкл.") : QStringLiteral("following off"));
    }
    m_stSync->setText(sync);
    const QString be = p->view()->backendName();
    m_stBackend->setText(be.isEmpty() ? QString()
                                      : QStringLiteral("%1 · %2 к/с").arg(be).arg(p->view()->fps(), 0, 'f', 0));
}

void MainWindow::updateLevels()
{
    if (!m_levelsDock || !m_levelsDock->isVisible())
        return;
    FieldPane *p = activePane();
    const std::vector<LevelInfo> lv = p ? p->fieldLevels() : std::vector<LevelInfo>();
    for (int i = 0; i < 20; ++i) {
        const int row = 19 - i;   // сверху — старший уровень, как в окне оригинала
        auto *c = new QTableWidgetItem;
        auto *v = new QTableWidgetItem;
        if (i < int(lv.size())) {
            c->setBackground(lv[size_t(i)].color);
            v->setText(QString::number(lv[size_t(i)].value, 'g', 6));
        }
        m_levels->setItem(row, 0, c);
        m_levels->setItem(row, 1, v);
    }
}

void MainWindow::updateWindowMenu()
{
    m_windowMenu->clear();
    m_windowMenu->addAction(m_actions.value(QStringLiteral("ID_WINDOW_NEW")));
    m_windowMenu->addAction(m_actions.value(QStringLiteral("ID_WINDOW_CASCADE")));
    m_windowMenu->addAction(m_actions.value(QStringLiteral("ID_WINDOW_TILE_HORZ")));
    m_windowMenu->addAction(m_actions.value(QStringLiteral("ID_WINDOW_NEXT")));
    m_windowMenu->addAction(m_actions.value(QStringLiteral("ID_WINDOW_PREV")));
    const QList<QMdiSubWindow *> list = m_mdi->subWindowList();
    if (!list.isEmpty())
        m_windowMenu->addSeparator();
    int i = 1;
    for (QMdiSubWindow *w : list) {
        QAction *a = m_windowMenu->addAction(QStringLiteral("&%1 %2").arg(i++).arg(w->windowTitle()));
        a->setCheckable(true);
        a->setChecked(w == m_mdi->activeSubWindow());
        connect(a, &QAction::triggered, this, [this, w]() { m_mdi->setActiveSubWindow(w); });
    }
}

// --- Файлы ---------------------------------------------------------------------

FieldPane *MainWindow::openFile(const QString &path)
{
    const QString abs = QFileInfo(path).absoluteFilePath();
    for (QMdiSubWindow *w : m_mdi->subWindowList()) {
        auto *p = qobject_cast<FieldPane *>(w->widget());
        if (p && QFileInfo(p->document()->path()) == QFileInfo(abs)) {
            m_mdi->setActiveSubWindow(w);
            return p;
        }
    }
    // Файл ещё не открыт ни в одном окне — новый документ и окно с сохранёнными настройками.
    auto doc = std::make_shared<FieldDocument>();
    doc->open(abs);
    auto *pane = new FieldPane(doc, ViewSettings::load());
    QMdiSubWindow *sub = m_mdi->addSubWindow(pane);
    sub->setWindowTitle(pane->title());
    sub->setWindowIcon(appIcon());
    const bool maximize = m_mdi->subWindowList().size() == 1 || (m_mdi->activeSubWindow() && m_mdi->activeSubWindow()->isMaximized());
    if (maximize)
        sub->showMaximized();
    else
        sub->show();
    m_mdi->setActiveSubWindow(sub);
    connectPane(pane);
    addRecent(abs);
    updateActions();
    updateStatus();
    return pane;
}

void MainWindow::fileOpen()
{
    QSettings s;
    const QString dir = s.value(QStringLiteral("LastDir")).toString();
    const QStringList files = QFileDialog::getOpenFileNames(this, QStringLiteral("Открыть файл поля"), dir, kFilter);
    for (const QString &f : files) {
        s.setValue(QStringLiteral("LastDir"), QFileInfo(f).absolutePath());
        openFile(f);
    }
}

void MainWindow::addRecent(const QString &path)
{
    QSettings s;
    QStringList l = s.value(QStringLiteral("RecentFiles")).toStringList();
    l.removeAll(path);
    l.prepend(path);
    while (l.size() > kMaxRecent)
        l.removeLast();
    s.setValue(QStringLiteral("RecentFiles"), l);
    updateRecentMenu();
}

void MainWindow::updateRecentMenu()
{
    m_recentMenu->clear();
    const QStringList l = QSettings().value(QStringLiteral("RecentFiles")).toStringList();
    int i = 1;
    for (const QString &f : l) {
        QAction *a = m_recentMenu->addAction(QStringLiteral("&%1 %2").arg(i++).arg(QDir::toNativeSeparators(f)));
        connect(a, &QAction::triggered, this, [this, f]() { openFile(f); });
    }
    m_recentMenu->setEnabled(!l.isEmpty());
}

void MainWindow::openLastRecent()
{
    // Без файла в командной строке — последний существующий из списка (InitInstance).
    const QStringList l = QSettings().value(QStringLiteral("RecentFiles")).toStringList();
    for (const QString &f : l) {
        if (QFileInfo::exists(f)) {
            openFile(f);
            return;
        }
    }
}

void MainWindow::newWindow()
{
    FieldPane *p = activePane();
    if (!p)
        return;
    auto *pane = new FieldPane(p->document(), p->settings());
    QMdiSubWindow *sub = m_mdi->addSubWindow(pane);
    sub->setWindowTitle(pane->title() + QStringLiteral(":%1").arg(m_mdi->subWindowList().size()));
    sub->setWindowIcon(appIcon());
    sub->show();
    m_mdi->setActiveSubWindow(sub);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    for (const QUrl &u : e->mimeData()->urls())
        if (u.isLocalFile())
            openFile(u.toLocalFile());
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    QSettings s;
    s.setValue(QStringLiteral("TMC_GraphicsOutputFieldFile/INIWND"), saveGeometry());
    s.setValue(QStringLiteral("TMC_GraphicsOutputFieldFile/WindowState"), saveState());
    m_mdi->closeAllSubWindows();
    QMainWindow::closeEvent(e);
}

// --- Картинки, печать, экспорт ---------------------------------------------------

void MainWindow::saveImage()
{
    FieldPane *p = activePane();
    if (!p)
        return;
    const QImage img = p->grabImage();
    if (img.isNull()) {
        QMessageBox::warning(this, QStringLiteral("FieldView"),
                             QStringLiteral("Не удалось получить изображение вида (окно свёрнуто или графика не запущена)."));
        return;
    }
    QSettings s;
    const QString base = QFileInfo(p->document()->path()).completeBaseName();
    const QString dir = s.value(QStringLiteral("ImageDir"), QFileInfo(p->document()->path()).absolutePath()).toString();
    const QString f = QFileDialog::getSaveFileName(this, QStringLiteral("Сохранить изображение"),
                                                   QDir(dir).filePath(base + QStringLiteral(".png")),
                                                   QStringLiteral("PNG (*.png);;JPEG (*.jpg);;BMP (*.bmp)"));
    if (f.isEmpty())
        return;
    s.setValue(QStringLiteral("ImageDir"), QFileInfo(f).absolutePath());
    if (!img.save(f))
        QMessageBox::warning(this, QStringLiteral("FieldView"), QStringLiteral("Не удалось записать %1").arg(f));
    else
        statusBar()->showMessage(QStringLiteral("Сохранено: %1").arg(QDir::toNativeSeparators(f)), 4000);
}

void MainWindow::copyImage()
{
    if (FieldPane *p = activePane()) {
        const QImage img = p->grabImage();
        if (img.isNull()) {
            statusBar()->showMessage(QStringLiteral("Не удалось получить изображение вида"), 4000);
            return;
        }
        QApplication::clipboard()->setImage(img);
        statusBar()->showMessage(QStringLiteral("Изображение скопировано в буфер обмена"), 3000);
    }
}

void MainWindow::saveSeries()
{
    FieldPane *p = activePane();
    if (!p)
        return;
    const QString def = p->document()->path().endsWith(QLatin1String(".frames"), Qt::CaseInsensitive)
                            ? p->document()->path()
                            : p->document()->path() + QStringLiteral(".frames");
    const QString f = QFileDialog::getSaveFileName(this, QStringLiteral("Сохранить такты серией"), def,
                                                   QStringLiteral("Серия тактов (*.frames)"));
    if (f.isEmpty())
        return;
    QString err;
    if (!p->document()->saveSeries(f, &err))
        QMessageBox::warning(this, QStringLiteral("FieldView"), QStringLiteral("Не удалось записать серию: %1").arg(err));
    else
        statusBar()->showMessage(QStringLiteral("Записано тактов: %1").arg(p->frameCount()), 4000);
}

void MainWindow::exportAnimation()
{
    FieldPane *p = activePane();
    if (!p)
        return;
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Экспорт анимации"));
    auto *form = new QFormLayout(&dlg);
    auto *fmt = new QComboBox;
    fmt->addItem(QStringLiteral("Анимация GIF"));
    fmt->addItem(QStringLiteral("Кадры PNG в папку"));
    auto *from = new QSpinBox, *to = new QSpinBox, *step = new QSpinBox, *fps = new QSpinBox, *width = new QSpinBox;
    const int n = p->frameCount();
    from->setRange(1, n);
    to->setRange(1, n);
    to->setValue(n);
    step->setRange(1, std::max(1, n));
    step->setValue(std::max(1, n / 200));
    fps->setRange(1, 50);
    fps->setValue(p->settings().speed);
    width->setRange(160, 3840);
    width->setValue(std::min(960, p->view()->width()));
    form->addRow(QStringLiteral("Формат"), fmt);
    form->addRow(QStringLiteral("С такта"), from);
    form->addRow(QStringLiteral("По такт"), to);
    form->addRow(QStringLiteral("Каждый k-й такт"), step);
    form->addRow(QStringLiteral("Кадров в секунду (GIF)"), fps);
    form->addRow(QStringLiteral("Ширина, точек"), width);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Записать…"));
    bb->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Отмена"));
    // Ширина кнопок — по тексту: иначе многоточие и последняя буква «Записать…»
    // уходили за край кнопки.
    for (QAbstractButton *b : bb->buttons())
        b->setMinimumWidth(b->fontMetrics().horizontalAdvance(b->text()) + 40);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(bb);
    if (dlg.exec() != QDialog::Accepted)
        return;
    const bool gif = fmt->currentIndex() == 0;
    const QString base = QFileInfo(p->document()->path()).completeBaseName();
    QString out;
    if (gif)
        out = QFileDialog::getSaveFileName(this, QStringLiteral("Анимация GIF"),
                                           QFileInfo(p->document()->path()).absolutePath() + QLatin1Char('/') + base + QStringLiteral(".gif"),
                                           QStringLiteral("GIF (*.gif)"));
    else
        out = QFileDialog::getExistingDirectory(this, QStringLiteral("Папка для кадров PNG"),
                                                QFileInfo(p->document()->path()).absolutePath());
    if (out.isEmpty())
        return;
    std::vector<int> frames;
    for (int i = from->value() - 1; i <= to->value() - 1; i += step->value())
        frames.push_back(i);
    QString err;
    if (!exportFrames(p, frames, out, gif, fps->value(), width->value(), &err, this))
        QMessageBox::warning(this, QStringLiteral("FieldView"), QStringLiteral("Экспорт прерван: %1").arg(err));
    else
        statusBar()->showMessage(QStringLiteral("Записано: %1").arg(QDir::toNativeSeparators(out)), 5000);
}

bool MainWindow::exportFrames(FieldPane *p, const std::vector<int> &frames, const QString &out, bool gif, int fps,
                              int W, QString *error, QWidget *progressParent)
{
    const QString base = QFileInfo(p->document()->path()).completeBaseName();
    const int keep = p->frame();
    p->setPlaying(false);
    std::unique_ptr<QProgressDialog> prog;
    if (progressParent) {
        prog = std::make_unique<QProgressDialog>(QStringLiteral("Запись тактов…"), QStringLiteral("Прервать"), 0,
                                                 int(frames.size()), progressParent);
        prog->setWindowModality(Qt::WindowModal);
    }
    GifWriter gw;
    bool ok = true;
    QString err;
    bool opened = false;
    for (size_t k = 0; k < frames.size(); ++k) {
        if (prog) {
            prog->setValue(int(k));
            if (prog->wasCanceled()) {
                err = QStringLiteral("прервано");
                ok = false;
                break;
            }
        }
        p->setFrame(frames[k]);
        QImage img = p->grabImage();
        if (img.isNull()) {
            ok = false;
            err = QStringLiteral("не удалось снять кадр");
            break;
        }
        // Кадр приводится к ширине экспорта; размер GIF задаёт первый кадр.
        img = img.scaledToWidth(W, Qt::SmoothTransformation).convertToFormat(QImage::Format_RGB32);
        if (gif) {
            if (!opened) {
                if (!gw.open(out, img.width(), img.height())) {
                    ok = false;
                    err = gw.error();
                    break;
                }
                opened = true;
            }
            if (!gw.addFrame(img, std::max(2, 100 / std::max(1, fps)))) {
                ok = false;
                err = gw.error();
                break;
            }
        } else {
            const QString f = QDir(out).filePath(QStringLiteral("%1_%2.png").arg(base).arg(k + 1, 4, 10, QLatin1Char('0')));
            if (!img.save(f)) {
                ok = false;
                err = f;
                break;
            }
        }
    }
    if (opened && !gw.close() && ok) {
        ok = false;
        err = gw.error();
    }
    if (prog)
        prog->setValue(int(frames.size()));
    p->setFrame(keep);
    if (error)
        *error = err;
    return ok;
}

bool MainWindow::printPane(QPrinter *printer, FieldPane *p, QString *error)
{
    const QImage img = p->grabImage();
    if (img.isNull()) {
        // С-62: снимок вида не получен (окно свёрнуто, графика не запущена) — пустой лист не печатаем.
        if (error)
            *error = QStringLiteral("Не удалось получить изображение вида (окно свёрнуто или графика не запущена).");
        return false;
    }
    const QString title = p->title() + QStringLiteral("    ") + p->statsText();
    QPainter painter(printer);
    const QRect page = painter.viewport();
    QFont f = theme::uiFont(11);
    f.setPointSizeF(10);
    painter.setFont(f);
    const int head = painter.fontMetrics().height() * 2;
    painter.drawText(QRect(page.left(), page.top(), page.width(), head), Qt::AlignLeft | Qt::AlignVCenter, title);
    QSize s = img.size();
    s.scale(page.width(), page.height() - head, Qt::KeepAspectRatio);
    painter.drawImage(QRect(QPoint(page.left() + (page.width() - s.width()) / 2, page.top() + head), s), img);
    if (error)
        error->clear();
    return true;
}

void MainWindow::print(bool preview)
{
    FieldPane *p = activePane();
    if (!p)
        return;
    if (p->grabImage().isNull()) {
        QMessageBox::warning(this, QStringLiteral("FieldView"),
                             QStringLiteral("Не удалось получить изображение вида (окно свёрнуто или графика не запущена)."));
        return;
    }
    QPointer<FieldPane> guard(p);
    const auto paint = [guard](QPrinter *printer) {
        if (guard)
            printPane(printer, guard, nullptr);
    };
    if (preview) {
        QPrintPreviewDialog dlg(m_printer.get(), this);
        dlg.setWindowTitle(QStringLiteral("Предварительный просмотр"));
        connect(&dlg, &QPrintPreviewDialog::paintRequested, this, paint);
        dlg.resize(1000, 760);
        dlg.exec();
    } else {
        QPrintDialog dlg(m_printer.get(), this);
        dlg.setWindowTitle(QStringLiteral("Печать"));
        if (dlg.exec() == QDialog::Accepted)
            paint(m_printer.get());
    }
}

void MainWindow::pageSetup()
{
    QPageSetupDialog dlg(m_printer.get(), this);
    dlg.exec();
}

void MainWindow::showParameters()
{
    FieldPane *p = activePane();
    if (!p)
        return;
    ParametersContext ctx;
    if (p->document()->hasData()) {
        const FieldHeader &h = p->document()->header(0);
        ctx.longUnit = h.longUnit;
        ctx.timeUnit = h.timeUnit;
        ctx.longUnitValue = h.longUnitValue > 0 ? h.longUnitValue : 1.0;
        ctx.xOrigin = h.xMin;
        ctx.yOrigin = h.yMin;
    }
    ctx.plasma = p->hasPlasma();
    ctx.epsAvailable = p->epsAvailable();
    ctx.epsReason = p->epsUnavailableReason();
    ctx.fieldZMin = p->fieldZMin();
    ctx.fieldZMax = p->fieldZMax();
    ctx.epsZMin = p->epsZMin();
    ctx.epsZMax = p->epsZMax();
    QPointer<FieldPane> guard(p);
    ParametersDialog dlg(p->settings(), ctx, [guard](const ViewSettings &s) {
        if (guard)
            guard->applySettings(s);
    }, this);
    if (dlg.exec() == QDialog::Accepted && guard)
        guard->applySettings(dlg.result());
    updateActions();
}

void MainWindow::about()
{
#ifdef TMC_FV_COMMON_ABOUT
    // То же окно, что у остальных программ пакета (версия, автор, ссылки, руководство).
    tmcabout::show(this, QStringLiteral("About FieldView"), QStringLiteral("FieldView"), QString());
    return;
#endif
    FieldPane *p = activePane();
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("О программе FieldView"));
    auto *lay = new QVBoxLayout(&dlg);
    auto *top = new QHBoxLayout;
    auto *icon = new QLabel;
    icon->setPixmap(appIcon().pixmap(72, 72));
    top->addWidget(icon, 0, Qt::AlignTop);
    auto *text = new QLabel;
    text->setTextFormat(Qt::RichText);
    text->setOpenExternalLinks(true);
    const QString backend = p ? p->view()->backendName() : QString();
    text->setText(QStringLiteral(
        "<div style='font-size:22px;font-weight:800'>FieldView</div>"
        "<div style='color:#9fb0c8'>Просмотр полей пакета TMC Suite — версия 5.0</div><br>"
        "Поле, посчитанное ядрами PlanarRT_H и PlanarRT_X, рельефом и картой по тактам,<br>"
        "топология модели, слой ε и плазма (N, Y, B).<br><br>"
        "Вывод — Qt RHI%1 (вместо OpenGL 1.x).<br>"
        "Наследник Tamic Rt Output Field Viewer 4.1, © 2004 Tamic_soft group.<br>"
        "Автор пакета — К. Н. Климов.<br><br>"
        "<a href='https://www.tamic.ru'>www.tamic.ru</a>")
                      .arg(backend.isEmpty() ? QString() : QStringLiteral(": ") + backend));
    top->addWidget(text, 1);
    lay->addLayout(top);
    auto *bb = new QDialogButtonBox;
    QPushButton *man = bb->addButton(QStringLiteral("Руководство пользователя"), QDialogButtonBox::ActionRole);
    QPushButton *ok = bb->addButton(QStringLiteral("ОК"), QDialogButtonBox::AcceptRole);
    ok->setDefault(true);
    connect(man, &QPushButton::clicked, this, &MainWindow::openManual);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    lay->addWidget(bb);
    dlg.exec();
}

void MainWindow::openManual()
{
#ifdef TMC_FV_COMMON_ABOUT
    // PDF из поставки на языке интерфейса — так же, как кнопка в окне «О программе».
    const QString pdf = tmcabout::manualPath();
    if (!pdf.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(pdf));
        return;
    }
#endif
    // Как TmcDocLink.h: руководство рядом с программой или выше по дереву, иначе — на сайте.
    const QString exeDir = QCoreApplication::applicationDirPath();
    const QStringList names = {QStringLiteral("TMC_Suite_User_Manual.pdf"), QStringLiteral("docs/user-manual/fieldview.md"),
                               QStringLiteral("fieldview.md")};
    QDir d(exeDir);
    for (int up = 0; up < 6; ++up) {
        for (const QString &n : names) {
            const QString f = d.filePath(n);
            if (QFileInfo::exists(f)) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(f));
                return;
            }
        }
        if (!d.cdUp())
            break;
    }
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://www.tamic.ru/docs/user-manual/index.html")));
}

} // namespace fieldview
