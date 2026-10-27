/// @file mainwindow.cpp
/// @brief Реализация мастера задания `MainWindow`: страницы, синхронизация формы и проекта,
///        сводка, память, отмена, чтение и запись задания.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-wizard
#include "mainwindow.h"

#include "calculatordialog.h"
#include "filemaker.h"
#include "cp1251.h"
#include "emitter.h"
#include "geometryview.h"
#include "import3ddialog.h"
#include "meshio.h"
#include "issuedialog.h"
#include "kernelnames.h"
#include "memorydonut.h"
#include "placement.h"
#include "portdialog.h"
#include "propertypanel.h"
#include "ramestimate.h"
#include "runplanpage.h"
#include "settings.h"
#include "sigfile.h"
#include "settingsdialog.h"
#include "shapedialog.h"
#include "sysmemory.h"
#include "taskfiles.h"
#include "usersamples.h"
#include "dtheme.h"
#include "tplreader.h"

#include <QActionGroup>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QItemSelectionModel>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QShortcut>
#include <QSignalBlocker>
#include <QScrollBar>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include "lang.h"

using namespace tpl;

namespace {

/// @brief Название раздела мастера на двух языках (UTF-8); переводится при каждом вызове `sectionName`.
struct SectionName { const char *ru; const char *en; };
/** @var SectionName::ru
    Русский текст названия. */
/** @var SectionName::en
    Английский текст названия. */
/// Названия пяти разделов мастера по порядку: Расчёт, Геометрия, Что считать, Вывод данных, Проверка.
const SectionName kSections[] = {{"Расчёт", "Calculation"},
                                 {"Геометрия", "Geometry"},
                                 {"Что считать", "What to compute"},
                                 {"Вывод данных", "Output"},
                                 {"Проверка", "Check"}};
/// Число разделов мастера.
constexpr int kSectionCount = 5;

/// @brief Возвращает название раздела мастера на действующем языке.
/// @param[in] i  номер раздела, 0...`kSectionCount`-1.
/// @return Название раздела.
QString sectionName(int i)
{
    return tpl::tr(kSections[i].ru, kSections[i].en);
}

/// Глубина отмены. Задание маленькое, снимок целиком стоит копейки.
constexpr int kUndoDepth = 120;

/// Какие плазменные величины несёт фигура — коротко, для списка слева.
/// @brief Перечисляет заданные плазменные величины фигуры через «+».
/// @param[in] pl  плазма фигуры.
/// @return Строка вида «N+B+sigma+Y» из заданных величин (знак sigma — греческой буквой интерфейса); пусто, если ничего нет.
QString plasmaShort(const Plasma &pl)
{
    QStringList parts;
    if (pl.hasN) parts << QStringLiteral("N");
    if (pl.hasB) parts << QStringLiteral("B");
    if (pl.hasS) parts << QStringLiteral("σ");
    if (pl.hasY) parts << QStringLiteral("Y");
    return parts.join(QStringLiteral("+"));
}

/// То же с числами — для сводки. Единицы пишем прямо здесь: N_UNIT и B_UNIT ядро
/// разбирает, но НЕ применяет, так что N всегда в м^-3, а B всегда в Тесла.
/// @brief Описывает плазменные величины фигуры с числами и единицами для сводки.
/// @param[in] pl  плазма фигуры.
/// @return Строка вида «N = ... м^-3, B = ... Тл, sigma = ... См/м, Y = ...»; для величины из файла — «N из файла имя».
QString plasmaFull(const Plasma &pl)
{
    QStringList parts;
    if (pl.hasN) {
        parts << (pl.nFile.isEmpty()
                      ? tpl::tr("N = %1 м^-3", "N = %1 m^-3").arg(pl.n)
                      : tpl::tr("N из файла %1", "N from file %1").arg(pl.nFile));
    }
    if (pl.hasB) {
        parts << (pl.bFile.isEmpty() ? tpl::tr("B = %1 Тл", "B = %1 T").arg(pl.b)
                                     : tpl::tr("B из файла %1", "B from file %1").arg(pl.bFile));
    }
    if (pl.hasS) parts << tpl::tr("σ = %1 См/м", "σ = %1 S/m").arg(pl.s);
    if (pl.hasY) {
        parts << (pl.yFile.isEmpty() ? QStringLiteral("Y = %1").arg(pl.y)
                                     : tpl::tr("Y из файла %1", "Y from file %1").arg(pl.yFile));
    }
    return parts.join(QStringLiteral(", "));
}

/// @brief Создаёт счётчик с тремя знаками после точки.
/// @param[in] v       начальное значение.
/// @param[in] suffix  суффикс единиц (« мм», « нс», « ГГц»).
/// @param[in] lo      нижняя граница.
/// @param[in] hi      верхняя граница.
/// @return Новый счётчик без родителя.
QDoubleSpinBox *spin(double v, const QString &suffix, double lo = -1e6, double hi = 1e6)
{
    auto *s = new QDoubleSpinBox;
    s->setRange(lo, hi);
    s->setDecimals(3);
    s->setSuffix(suffix);
    s->setValue(v);
    return s;
}

/// Значок инструмента рисуем сами: тащить ради семи картинок файл ресурсов незачем.
/// @brief Рисует значок 22x22 пикселя для кнопки инструмента холста.
/// @param[in] t  инструмент (`Tool::Select`, `Line`, `Rect`, `Contour`, `Circle`, `Arc`, `ArcLine`, `Port`).
/// @return Значок на прозрачном фоне.
QIcon toolIcon(Tool t)
{
    QPixmap pm(22, 22);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(dtheme::text());
    pen.setWidthF(1.8);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    switch (t) {
    case Tool::Select: {
        QPainterPath path;
        path.moveTo(6, 3);
        path.lineTo(6, 18);
        path.lineTo(10, 14);
        path.lineTo(13, 19.5);
        path.lineTo(15.5, 18.5);
        path.lineTo(12.5, 13);
        path.lineTo(17, 12.5);
        path.closeSubpath();
        p.setBrush(dtheme::text());
        p.drawPath(path);
        break;
    }
    case Tool::Rect:
        p.drawRect(QRectF(3.5, 5.5, 15, 11));
        break;
    case Tool::Line:
        p.drawLine(QPointF(3.5, 18), QPointF(10, 7));
        p.drawLine(QPointF(10, 7), QPointF(18.5, 13));
        break;
    case Tool::Contour: {
        QPolygonF poly;
        poly << QPointF(4, 15) << QPointF(8, 5) << QPointF(18, 8) << QPointF(15, 18);
        p.drawPolygon(poly);
        break;
    }
    case Tool::Circle:
        p.drawEllipse(QPointF(11, 11), 7.5, 7.5);
        break;
    case Tool::Arc: {
        QPainterPath path;
        const QRectF box(3, 3, 16, 16);
        path.arcMoveTo(box, 0);
        path.arcTo(box, 0, 110);
        p.drawPath(path);
        p.drawLine(QPointF(11, 11), QPointF(19, 11));
        p.drawLine(QPointF(11, 11), QPointF(8.3, 18.5));
        break;
    }
    case Tool::ArcLine: {
        // Не сектор: сектор — это заливка от центра, а дуга — просто кривая линия.
        // Значок обязан различать их с одного взгляда, иначе кнопки перепутают.
        QPainterPath path;
        path.moveTo(3.5, 17);
        path.cubicTo(6, 4.5, 16, 4.5, 18.5, 17);
        p.drawPath(path);
        p.setBrush(dtheme::text());
        p.drawEllipse(QPointF(3.5, 17), 1.8, 1.8);
        p.drawEllipse(QPointF(18.5, 17), 1.8, 1.8);
        break;
    }
    case Tool::Port:
        pen.setColor(dtheme::ok());
        pen.setWidthF(3.0);
        p.setPen(pen);
        p.drawLine(QPointF(6, 4), QPointF(6, 18));
        pen.setWidthF(1.6);
        p.setPen(pen);
        p.drawLine(QPointF(8, 11), QPointF(18, 11));
        p.drawLine(QPointF(14, 7.5), QPointF(18, 11));
        p.drawLine(QPointF(14, 14.5), QPointF(18, 11));
        break;
    }
    p.end();
    return QIcon(pm);
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tpl::tr("TPL Designer — построение задания", "TPL Designer — task builder"));

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(12);

    auto *left = new QVBoxLayout;
    auto *title = new QLabel(tpl::tr("Задание", "Task"));
    title->setProperty("h2", true);
    left->addWidget(title);

    // Разделы доступны в любом порядке — как дерево проекта в HFSS. «Далее» просто
    // ведёт по ним по очереди, для тех, кто здесь впервые.
    m_tree = new QTreeWidget;
    m_tree->setHeaderHidden(true);
    m_tree->setRootIsDecorated(false);
    m_tree->setFixedWidth(196);
    for (int i = 0; i < kSectionCount; ++i) {
        auto *it = new QTreeWidgetItem(m_tree);
        it->setText(0, QStringLiteral("%1.  %2").arg(i + 1).arg(sectionName(i)));
        it->setData(0, Qt::UserRole, i);
    }
    left->addWidget(m_tree, 1);

    // Калькулятор нужен на любой странице: отсечки считают и при выборе области,
    // и при задании плазмы, и при подборе свипа.
    auto *calc = new QPushButton(tpl::tr("Калькулятор среды", "Medium calculator"));
    calc->setToolTip(tpl::tr("Отсечки волновода, длина волны в диэлектрике, "
                             "плазменная и циклотронная частоты",
                             "Waveguide cutoffs, wavelength in a dielectric, plasma and cyclotron "
                             "frequencies"));
    connect(calc, &QPushButton::clicked, this, [this] {
        CalculatorDialog dlg(this);
        dlg.exec();
    });
    left->addWidget(calc);

    // Файлы сигнала и распределения нужны при правке порта и фигуры, а не на одной
    // странице: кнопка стоит рядом с калькулятором, а не прячется в одном шаге.
    auto *files = new QPushButton(tpl::tr("Вспомогательные файлы", "Auxiliary files"));
    files->setObjectName(QStringLiteral("fileMaker"));
    files->setToolTip(tpl::tr("Сделать файл падающей волны .sig или файл "
                              "распределения eps .eps — с предпросмотром",
                              "Make an incident-wave file .sig or an eps distribution file .eps — with "
                              "preview"));
    connect(files, &QPushButton::clicked, this, [this] {
        FileMakerDialog dlg(m_project, this);
        dlg.exec();
    });
    left->addWidget(files);

    auto *home = new QPushButton(tpl::tr("В начало", "Home"));
    connect(home, &QPushButton::clicked, this, &MainWindow::backToStartRequested);
    left->addWidget(home);
    root->addLayout(left);

    auto *right = new QVBoxLayout;
    m_stepLabel = new QLabel;
    m_stepLabel->setProperty("h1", true);
    right->addWidget(m_stepLabel);

    m_stack = new QStackedWidget;
    m_stack->addWidget(buildSetupPage());
    m_stack->addWidget(buildGeometryPage());
    m_stack->addWidget(buildRunPlanPage());
    m_stack->addWidget(buildOutputPage());
    m_stack->addWidget(buildSummaryPage());
    right->addWidget(m_stack, 1);

    // Диагностика видна на всех шагах: ошибку надо показывать там, где её сделали,
    // а не в конце мастера. Высота фиксированная — иначе список дёргает раскладку.
    m_diag = new QListWidget;
    m_diag->setFixedHeight(104);
    // Замечания ПЕРЕНОСИТЬ по словам, а не прятать за горизонтальной полосой.
    // Строки тут длинные по делу: они объясняют, что именно ядро посчитает не так, и
    // обрезанное на полуслове объяснение не объясняет ничего.
    m_diag->setWordWrap(true);
    m_diag->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_diag->setTextElideMode(Qt::ElideNone);
    right->addWidget(m_diag);

    auto *nav = new QHBoxLayout;
    m_back = new QPushButton(tpl::tr("Назад", "Back"));
    m_back->setObjectName(QStringLiteral("btnBack"));
    m_next = new QPushButton(tpl::tr("Далее", "Next"));
    m_next->setObjectName(QStringLiteral("btnNext"));
    m_next->setProperty("accent", true);
    connect(m_back, &QPushButton::clicked, this, &MainWindow::onBack);
    connect(m_next, &QPushButton::clicked, this, &MainWindow::onNext);
    nav->addStretch();
    nav->addWidget(m_back);
    nav->addWidget(m_next);
    right->addLayout(nav);
    root->addLayout(right, 1);

    connect(m_tree, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem *it) {
        if (it)
            onSectionChanged(it->data(0, Qt::UserRole).toInt());
    });

    setProject(Project{});
    m_tree->setCurrentItem(m_tree->topLevelItem(0));
}

// ------------------------------------------------------------------- «Расчёт»

QWidget *MainWindow::buildSetupPage()
{
    auto *page = new QWidget;
    auto *l = new QVBoxLayout(page);
    l->setContentsMargins(0, 0, 0, 0);

    auto *task = new QGroupBox(tpl::tr("Задание", "Task"));
    auto *tf = new QFormLayout(task);
    m_name = new QLineEdit(QStringLiteral("task"));
    m_name->setObjectName(QStringLiteral("taskName"));
    tf->addRow(tpl::tr("Имя файла (.tpl):", "File name (.tpl):"), m_name);
    m_kernel = new QComboBox;
    m_kernel->setObjectName(QStringLiteral("kernel"));
    m_kernel->addItem(tpl::tr("H-мода — %1", "H-mode — %1").arg(kernelProgram(Kernel::H)),
                      int(Kernel::H));
    m_kernel->addItem(tpl::tr("X-мода с плазмой — %1", "X-mode with plasma — %1").arg(kernelProgram(Kernel::X)),
                      int(Kernel::X));
    tf->addRow(tpl::tr("Ядро расчёта:", "Kernel:"), m_kernel);
    m_unit = new QComboBox;
    m_unit->addItem(tpl::tr("миллиметры", "millimeters"), int(LengthUnit::Millimeter));
    m_unit->addItem(tpl::tr("сантиметры", "centimeters"), int(LengthUnit::Centimeter));
    m_unit->addItem(tpl::tr("метры", "meters"), int(LengthUnit::Meter));
    tf->addRow(tpl::tr("Единицы на линейке холста:", "Canvas ruler units:"), m_unit);
    l->addWidget(task);

    auto *area = new QGroupBox(tpl::tr("Счётная область", "Computational domain"));
    auto *af = new QFormLayout(area);
    m_xMin = spin(0, tpl::tr(" мм", " mm"));
    m_xMax = spin(240, tpl::tr(" мм", " mm"));
    m_xMax->setObjectName(QStringLiteral("boxXMax"));
    m_yMin = spin(0, tpl::tr(" мм", " mm"));
    m_yMax = spin(80, tpl::tr(" мм", " mm"));
    m_yMax->setObjectName(QStringLiteral("boxYMax"));
    m_delta = spin(1.0, tpl::tr(" мм", " mm"), 0.001, 1000);
    m_delta->setObjectName(QStringLiteral("delta"));
    m_time = spin(115, tpl::tr(" нс", " ns"), 0.001, 1e6);
    m_time->setObjectName(QStringLiteral("timeMax"));
    af->addRow(tpl::tr("X от:", "X from:"), m_xMin);
    af->addRow(tpl::tr("X до:", "X to:"), m_xMax);
    af->addRow(tpl::tr("Y от:", "Y from:"), m_yMin);
    af->addRow(tpl::tr("Y до:", "Y to:"), m_yMax);
    af->addRow(tpl::tr("Шаг сетки DELTA:", "Grid step DELTA:"), m_delta);
    af->addRow(tpl::tr("Время счёта:", "Run time:"), m_time);
    auto *deltaHint = new QLabel(tpl::tr("Время счёта растёт примерно кубически при уменьшении шага сетки. "
        "К этому же шагу привязывается мышь на холсте.",
        "Run time grows roughly as the cube of the grid step reduction. The mouse on "
        "the canvas snaps to the same step."));
    deltaHint->setProperty("mute", true);
    deltaHint->setWordWrap(true);
    af->addRow(QString(), deltaHint);
    l->addWidget(area);

    auto *freq = new QGroupBox(tpl::tr("Частота", "Frequency"));
    auto *ff = new QFormLayout(freq);
    // Кладём переключатели ЛАЙАУТОМ, а не виджетом-обёрткой: у обёртки свой фон,
    // и под кнопками появляется тёмная полоса непонятного назначения.
    auto *rl = new QHBoxLayout;
    rl->setContentsMargins(0, 0, 0, 0);
    m_freqOne = new QRadioButton(tpl::tr("Одна частота", "Single frequency"));
    m_freqRange = new QRadioButton(tpl::tr("Диапазон частот", "Frequency range"));
    m_freqRange->setObjectName(QStringLiteral("freqRange"));
    m_freqOne->setChecked(true);
    rl->addWidget(m_freqOne);
    rl->addWidget(m_freqRange);
    rl->addStretch();
    ff->addRow(rl);

    m_freqSingle = spin(4.0, tpl::tr(" ГГц", " GHz"), 0.001, 1e5);
    m_freqStart = spin(3.0, tpl::tr(" ГГц", " GHz"), 0.001, 1e5);
    m_freqStart->setObjectName(QStringLiteral("freqStart"));
    m_freqEnd = spin(5.0, tpl::tr(" ГГц", " GHz"), 0.001, 1e5);
    m_freqEnd->setObjectName(QStringLiteral("freqEnd"));
    m_freqStep = spin(0.1, tpl::tr(" ГГц", " GHz"), 0.0001, 1e5);
    m_freqStep->setObjectName(QStringLiteral("freqStep"));
    ff->addRow(tpl::tr("Частота:", "Frequency:"), m_freqSingle);
    ff->addRow(tpl::tr("От:", "From:"), m_freqStart);
    ff->addRow(tpl::tr("До:", "To:"), m_freqEnd);
    ff->addRow(tpl::tr("Шаг:", "Step:"), m_freqStep);
    m_freqInfo = new QLabel;
    m_freqInfo->setObjectName(QStringLiteral("freqInfo"));
    m_freqInfo->setProperty("mute", true);
    m_freqInfo->setWordWrap(true);
    m_freqInfo->setTextFormat(Qt::RichText);
    // Ссылки в подписи — два выхода из «частот больше 99»: шаг крупнее или конвейер.
    connect(m_freqInfo, &QLabel::linkActivated, this, [this](const QString &link) {
        if (link == QLatin1String("step")) {
            m_freqStep->setValue(stepForMaxFreqs());
        } else if (link == QLatin1String("chunk")) {
            m_chunkOn->setChecked(true);
        }
    });
    ff->addRow(QString(), m_freqInfo);

    // Конвейер: одну сетку частот — в несколько файлов задания.
    //
    // Тысяча частот в одно задание не лезет: в файл .s пакет укладывает не больше 99
    // точек. Раньше на каждую частоту писали свой файл руками, а потом сшивали
    // результат скриптом — вот это и делается здесь делением.
    auto *chl = new QHBoxLayout;
    chl->setContentsMargins(0, 0, 0, 0);
    m_chunkOn = new QCheckBox(tpl::tr("Разложить по нескольким заданиям", "Split into several tasks"));
    m_chunkSize = new QSpinBox;
    m_chunkSize->setRange(1, 99);
    m_chunkSize->setValue(10);
    m_chunkSize->setSuffix(tpl::tr(" частот в задании", " frequencies per task"));
    chl->addWidget(m_chunkOn);
    chl->addWidget(m_chunkSize);
    chl->addStretch();
    ff->addRow(chl);
    m_chunkInfo = new QLabel;
    m_chunkInfo->setProperty("mute", true);
    m_chunkInfo->setWordWrap(true);
    ff->addRow(QString(), m_chunkInfo);

    l->addWidget(freq);
    l->addStretch();

    const auto touch = [this] { pullFromForm(); };
    connect(m_name, &QLineEdit::textChanged, this, touch);
    connect(m_kernel, &QComboBox::currentIndexChanged, this, touch);
    connect(m_unit, &QComboBox::currentIndexChanged, this, touch);
    for (auto *s : {m_xMin, m_xMax, m_yMin, m_yMax, m_delta, m_time,
                    m_freqSingle, m_freqStart, m_freqEnd, m_freqStep})
        connect(s, &QDoubleSpinBox::valueChanged, this, touch);
    connect(m_freqOne, &QRadioButton::toggled, this, touch);
    connect(m_chunkOn, &QCheckBox::toggled, this, touch);
    connect(m_chunkSize, &QSpinBox::valueChanged, this, touch);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(page);
    scroll->setFrameShape(QFrame::NoFrame);
    return scroll;
}

// ---------------------------------------------------------------- «Геометрия»

QWidget *MainWindow::buildToolPalette()
{
    struct Item {
        Tool tool;
        const char *name; const char *nameEn;
        const char *tip; const char *tipEn;
        const char *key;
    };
    static const Item items[] = {
        {Tool::Select,  "Выбор",         "Select",
                        "Выделять, двигать, менять размер",
                        "Select, move, resize", "V"},
        {Tool::Line,    "Линия",         "Line",
                        "Ломаная: клик за кликом, Enter — закончить",
                        "Polyline: click after click, Enter — finish", "L"},
        {Tool::Rect,    "Прямоугольник", "Rectangle",
                        "Нажать и протянуть",
                        "Press and drag", "R"},
        {Tool::Contour, "Контур",        "Polygon",
                        "Замкнутый многоугольник",
                        "Closed polygon", "C"},
        {Tool::Circle,  "Круг",          "Circle",
                        "Из центра протянуть радиус",
                        "Drag the radius from the center", "O"},
        {Tool::Arc,     "Сектор",        "Sector",
                        "Центр, радиус, затем раскрыв",
                        "Center, radius, then opening angle", "A"},
        {Tool::ArcLine, "Дуга",          "Arc",
                        "Два конца, потом выгиб: круглая стенка",
                        "Two ends, then the bulge: a curved wall", "G"},
        {Tool::Port,    "Порт",          "Port",
                        "Протянуть вдоль границы области",
                        "Drag along the domain boundary", "P"},
    };

    auto *box = new QWidget;
    box->setObjectName(QStringLiteral("toolPalette"));
    box->setFixedWidth(168);
    auto *l = new QVBoxLayout(box);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(4);

    auto *caption = new QLabel(tpl::tr("Инструменты", "Tools"));
    caption->setProperty("dim", true);
    l->addWidget(caption);

    auto *group = new QButtonGroup(this);
    group->setExclusive(true);
    for (const Item &it : items) {
        auto *b = new QToolButton;
        b->setText(tpl::tr(it.name, it.nameEn));
        b->setIcon(toolIcon(it.tool));
        b->setIconSize(QSize(22, 22));
        b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        b->setCheckable(true);
        b->setAutoRaise(false);
        b->setMinimumHeight(34);
        b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        b->setToolTip(QStringLiteral("%1  (%2)").arg(tpl::tr(it.tip, it.tipEn),
                                                     QString::fromUtf8(it.key)));
        b->setProperty("tool", true);
        const Tool t = it.tool;
        b->setProperty("toolId", int(t));
        connect(b, &QToolButton::clicked, this, [this, t] { m_view->setTool(t); });
        // Буквенная горячая клавиша живёт только на холсте: иначе она воровала бы
        // ввод у полей справа, где ту же букву просто набирают.
        auto *sc = new QShortcut(QKeySequence(QString::fromUtf8(it.key)), m_view);
        sc->setContext(Qt::WidgetShortcut);
        connect(sc, &QShortcut::activated, this, [this, t] { m_view->setTool(t); });
        group->addButton(b);
        m_tools << b;
        l->addWidget(b);
    }
    m_tools.first()->setChecked(true);

    // Импорт: срез 3D-модели плоскостью — готовая геометрия вместо рисования мышью.
    l->addSpacing(6);
    auto *imp = new QPushButton(tpl::tr("Импорт из 3D…", "Import from 3D…"));
    imp->setObjectName(QStringLiteral("import3d"));
    imp->setMinimumHeight(34);
    imp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    imp->setToolTip(tpl::tr("Взять сечение 3D-модели (%1) плоскостью XY, YZ или XZ "
                            "и вставить его контуры фигурами",
                            "Take a section of a 3D model (%1) by the XY, YZ or XZ plane and insert its "
                            "contours as shapes")
                        .arg(tpl::stepSupported() ? QStringLiteral("STL, OBJ, STEP")
                                                  : QStringLiteral("STL, OBJ")));
    connect(imp, &QPushButton::clicked, this, [this] { onImport3D(); });
    l->addWidget(imp);

    l->addSpacing(10);
    auto *note = new QLabel(tpl::tr("Колесо — зум, средняя кнопка или Alt+ЛКМ — сдвиг.\n"
        "Ctrl+ЛКМ — выбрать несколько фигур.\n"
        "Shift держит угол кратным 45°.\n"
        "Esc — бросить начатое.",
        "Wheel — zoom, middle button or Alt+LMB — pan.\nCtrl+LMB — select several "
        "shapes.\nShift keeps the angle a multiple of 45°.\nEsc — drop what was "
        "started."));
    note->setProperty("mute", true);
    note->setWordWrap(true);
    l->addWidget(note);
    l->addStretch();
    return box;
}

QWidget *MainWindow::buildGeometryPage()
{
    auto *page = new QWidget;
    auto *l = new QVBoxLayout(page);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(8);

    m_view = new GeometryView;

    auto *bar = new QToolBar;
    m_undoAct = bar->addAction(tpl::tr("Отменить", "Undo"), this, &MainWindow::undo);
    m_undoAct->setShortcut(QKeySequence::Undo);
    m_redoAct = bar->addAction(tpl::tr("Вернуть", "Redo"), this, &MainWindow::redo);
    m_redoAct->setShortcut(QKeySequence::Redo);
    bar->addSeparator();

    // Начальные положения — из настроек: их для того и заводили, чтобы человек не
    // выставлял одно и то же при каждом запуске.
    const settings::Values &sv = settings::values();

    auto *grid = bar->addAction(tpl::tr("Сетка", "Grid"));
    grid->setCheckable(true);
    grid->setChecked(sv.gridVisible);
    connect(grid, &QAction::toggled, this, [this](bool on) { m_view->setGridVisible(on); });

    auto *snap = bar->addAction(tpl::tr("Привязка", "Snap"));
    snap->setCheckable(true);
    snap->setChecked(sv.snapEnabled);
    snap->setToolTip(tpl::tr("Прилипать к концам и вершинам нарисованного, к точкам на его линиях, "
        "к углам области — и, если рядом ничего нет, к шагу сетки DELTA",
        "Stick to ends and vertices of what is drawn, to points on its lines, to the "
        "domain corners — and, if nothing is near, to the grid step DELTA"));
    connect(snap, &QAction::toggled, this, [this](bool on) { m_view->setSnapEnabled(on); });
    m_view->setGridVisible(sv.gridVisible);
    m_view->setSnapEnabled(sv.snapEnabled);

    // Шкала eps прячется отсюда же, а не только из настроек: её включают и выключают
    // по ходу работы, и лезть ради этого в диалог было бы издевательством.
    auto *bar2 = bar->addAction(tpl::tr("Шкала eps", "eps scale"));
    bar2->setCheckable(true);
    bar2->setChecked(sv.epsColorBar);
    bar2->setToolTip(tpl::tr("Цветовая шкала значений eps у правого края. Видна только когда на сцене есть "
        "фигура с формулой",
        "Color scale of eps values at the right edge. Shown only when the scene has a "
        "shape with a formula"));
    connect(bar2, &QAction::toggled, this, [this](bool on) {
        settings::Values v = settings::values();
        v.epsColorBar = on;
        settings::setValues(v);
        m_view->viewport()->update();
    });

    bar->addAction(tpl::tr("Настройки…", "Settings…"), this, [this] {
        SettingsDialog dlg(this);
        connect(&dlg, &SettingsDialog::settingsChanged, this, [this] {
            m_view->setGridVisible(settings::values().gridVisible);
            m_view->setSnapEnabled(settings::values().snapEnabled);
            m_view->viewport()->update();
        });
        dlg.exec();
    });
    bar->addSeparator();

    bar->addAction(tpl::tr("Вписать", "Fit"), this, [this] { m_view->zoomToFit(); })
        ->setShortcut(QKeySequence(QStringLiteral("Ctrl+0")));
    bar->addAction(tpl::tr("Приблизить", "Zoom in"), this, [this] { m_view->zoomIn(); });
    bar->addAction(tpl::tr("Отдалить", "Zoom out"), this, [this] { m_view->zoomOut(); });
    bar->addSeparator();
    bar->addAction(tpl::tr("Дублировать", "Duplicate"), this, [this] { m_view->duplicateSelected(); })
        ->setShortcut(QKeySequence(QStringLiteral("Ctrl+D")));
    bar->addAction(tpl::tr("Удалить", "Delete"), this, [this] { m_view->deleteSelected(); });
    l->addWidget(bar);

    auto *body = new QHBoxLayout;
    body->setSpacing(10);
    body->addWidget(buildToolPalette());

    auto *split = new QSplitter(Qt::Horizontal);
    split->setChildrenCollapsible(false);
    split->setHandleWidth(6);

    auto *canvasBox = new QWidget;
    auto *cl = new QVBoxLayout(canvasBox);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->setSpacing(6);
    cl->addWidget(m_view, 1);

    auto *status = new QWidget;
    // Высота с запасом на ЧЕТЫРЕ строки: подсказка инструмента переносится по словам,
    // а ширина у неё разная — панель справа то шире, то уже. 56 px хватало ровно на
    // три, и последняя строка обрезалась краем.
    status->setFixedHeight(68);
    auto *sl = new QHBoxLayout(status);
    sl->setContentsMargins(0, 0, 0, 0);
    m_hint = new QLabel;
    m_hint->setProperty("mute", true);
    m_hint->setWordWrap(true);
    // Ignored по горизонтали: подсказка подстраивается под доступную ширину и
    // никогда не расталкивает соседей своим текстом.
    m_hint->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    m_hint->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_cursor = new QLabel;
    m_cursor->setProperty("dim", true);
    m_cursor->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    // Ширина фиксированная: иначе подпись координат растёт под курсором и
    // расталкивает соседние панели.
    m_cursor->setFixedWidth(210);
    sl->addWidget(m_hint, 1);
    sl->addWidget(m_cursor);
    cl->addWidget(status);
    canvasBox->setMinimumWidth(320);
    split->addWidget(canvasBox);

    auto *rightBox = new QWidget;
    auto *rl = new QVBoxLayout(rightBox);
    rl->setContentsMargins(0, 0, 0, 0);
    rl->setSpacing(6);

    auto *lt = new QLabel(tpl::tr("Объекты задания", "Task objects"));
    lt->setProperty("dim", true);
    rl->addWidget(lt);
    m_legend = new QListWidget;
    // Имя — чтобы служебные утилиты находили именно ЭТОТ список: в окне их несколько,
    // и findChild<QListWidget*> без имени возвращает первый попавшийся.
    m_legend->setObjectName(QStringLiteral("legend"));
    m_legend->setFixedHeight(176);
    // Несколько строк сразу — ради «Соединить»: на холсте выделение одиночное, и
    // другого места указать «вот эти четыре куска» в программе нет.
    m_legend->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_legend->setToolTip(tpl::tr("Del — удалить выделенное.\n"
                                 "Ctrl или Shift — выбрать несколько: их можно "
                                 "соединить, дублировать и удалить разом",
                                 "Del — delete the selection.\nCtrl or Shift — select several: they can be "
                                 "merged, duplicated and deleted at once"));
    rl->addWidget(m_legend);

    // Del работает и в списке, а не только на холсте: выделил объект здесь —
    // нажал Del. Контекст WidgetShortcut держит клавишу в рамках списка, чтобы
    // она не отбирала Del у полей ввода рядом.
    auto *delInList = new QShortcut(QKeySequence(Qt::Key_Delete), m_legend);
    delInList->setContext(Qt::WidgetShortcut);
    connect(delInList, &QShortcut::activated, this, [this] { m_view->deleteSelected(); });

    m_panel = new PropertyPanel;
    auto *panelScroll = new QScrollArea;
    panelScroll->setWidgetResizable(true);
    panelScroll->setFrameShape(QFrame::NoFrame);
    panelScroll->setWidget(m_panel);
    panelScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rl->addWidget(panelScroll, 1);

    // ⚠️ Ширину правой панели считаем от того, что она РЕАЛЬНО просит при текущем
    // шрифте, а не магическим числом (было 300…460 и 340 в setSizes).
    //
    // Числа подбирались под Segoe UI, и на Linux, где системный шрифт заметно шире,
    // ряд кнопок «Все свойства… | Дублировать | Удалить» в 340 px не влезал: у
    // прокрутки выключен горизонтальный ползунок, поэтому кнопка «Удалить» просто
    // обрезалась краем окна. Спросить у панели её собственный sizeHint — и ширина
    // окажется верной на любом шрифте любой системы.
    const int wantRight = std::max(300, m_panel->sizeHint().width()
                                            + panelScroll->verticalScrollBar()->sizeHint().width()
                                            + 8);
    rightBox->setMinimumWidth(wantRight);
    rightBox->setMaximumWidth(std::max(460, wantRight + 40));
    split->addWidget(rightBox);

    split->setStretchFactor(0, 1);
    split->setStretchFactor(1, 0);
    split->setSizes({900, wantRight});
    body->addWidget(split, 1);
    l->addLayout(body, 1);

    // --- связи холста ---
    connect(m_view, &GeometryView::aboutToEdit, this, &MainWindow::snapshot);
    connect(m_view, &GeometryView::projectEdited, this, &MainWindow::refreshAll);
    connect(m_view, &GeometryView::editRequested, this, &MainWindow::onEditRequested);
    connect(m_view, &GeometryView::hintChanged, this,
            [this](const QString &t) { m_hint->setText(t); });
    connect(m_view, &GeometryView::cursorMoved, this, [this](QPointF p) {
        m_cursor->setText(tpl::tr("X %1   Y %2 мм", "X %1   Y %2 mm")
                              .arg(p.x(), 0, 'f', 2)
                              .arg(p.y(), 0, 'f', 2));
    });
    connect(m_view, &GeometryView::toolChanged, this, [this](Tool t) {
        for (QToolButton *b : m_tools)
            b->setChecked(b->property("toolId").toInt() == int(t));
    });
    connect(m_view, &GeometryView::selectionChanged, this, [this] {
        m_panel->showSelection(m_view->selection());
        // ⚠️ Список только ПОДСВЕЧИВАЕМ, а не перестраиваем.
        //
        // Раньше здесь стоял refreshLegend(), и он выбивал почву из-под ног у
        // множественного выбора: Ctrl-клик по второй строке менял выделение, список
        // тут же перебирался заново, набор терялся. Строки зависят от свойств фигур,
        // а они меняются на правке проекта, а не на выделении.
        syncLegendSelection();
        updateMergeButton();
    });

    // --- связи панели свойств ---
    connect(m_panel, &PropertyPanel::aboutToEdit, this, &MainWindow::snapshot);
    connect(m_panel, &PropertyPanel::edited, this, &MainWindow::refreshAll);
    connect(m_panel, &PropertyPanel::fullPropertiesRequested, this, &MainWindow::onEditRequested);
    connect(m_panel, &PropertyPanel::duplicateRequested, this,
            [this] { m_view->duplicateSelected(); });
    connect(m_panel, &PropertyPanel::deleteRequested, this,
            [this] { m_view->deleteSelected(); });
    connect(m_panel, &PropertyPanel::filletRequested, this,
            [this](double r) { m_view->filletSelected(r); });

    // ⚠️ Ведём выделение по itemSelectionChanged, а НЕ по currentItemChanged.
    //
    // Ctrl-клик и Shift-клик меняют именно набор; «текущая строка» при этом ведёт себя
    // по-своему, и раньше по ней выделение уезжало то на одну фигуру, то на весь
    // список. Список объектов — источник правды о наборе, холст просто повторяет.
    connect(m_legend, &QListWidget::itemSelectionChanged, this, [this] {
        if (m_syncingSelection)
            return;
        m_syncingSelection = true;
        m_view->setSelectedIds(selectedShapeIds());
        m_syncingSelection = false;
        updateMergeButton();
    });
    connect(m_legend, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *it) {
        onEditRequested(it->data(Qt::UserRole).toInt());
    });
    connect(m_panel, &PropertyPanel::mergeRequested, this, &MainWindow::mergeSelectedShapes);
    connect(m_view, &GeometryView::mergeRequested, this, &MainWindow::mergeSelectedShapes);
    return page;
}

// ---------------------------------------------------------------- «Вывод данных»

QWidget *MainWindow::buildRunPlanPage()
{
    m_plan = new RunPlanPage;
    connect(m_plan, &RunPlanPage::aboutToEdit, this, &MainWindow::snapshot);
    connect(m_plan, &RunPlanPage::edited, this, &MainWindow::refreshAll);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(m_plan);
    scroll->setFrameShape(QFrame::NoFrame);
    return scroll;
}

QWidget *MainWindow::buildOutputPage()
{
    auto *page = new QWidget;
    auto *l = new QVBoxLayout(page);
    l->setContentsMargins(0, 0, 0, 0);

    auto *files = new QGroupBox(tpl::tr("Сигналы на портах (TMCROS)", "Port signals (TMCROS)"));
    auto *fl = new QFormLayout(files);
    m_signalFile = new QLineEdit(QStringLiteral("signal"));
    m_signalFile->setObjectName(QStringLiteral("signalFile"));
    fl->addRow(tpl::tr("Имя файла:", "File name:"), m_signalFile);
    auto *fh = new QLabel(tpl::tr("Расширение ядро добавит само — точку в имени не ставьте. "
                                  "Этот файл пишется всегда.",
                                  "The kernel adds the extension itself — do not put a dot in the name. This "
                                  "file is always written."));
    fh->setProperty("mute", true);
    fh->setWordWrap(true);
    fl->addRow(QString(), fh);
    l->addWidget(files);

    // Топология и поля — РАЗНЫЕ выгрузки, и стоят они разного. Раньше они жили в одной
    // рамке (галочка + список с пунктом «не выгружать»), и это читалось как один
    // сбитый переключатель. Теперь у каждой выгрузки своя рамка и своя галочка.
    auto *topo = new QGroupBox(tpl::tr("Топология для FieldView", "Topology for FieldView"));
    auto *tl = new QVBoxLayout(topo);
    m_topology = new QCheckBox(tpl::tr("Выгружать топологию (файл .tt)", "Export topology (.tt file)"));
    m_topology->setChecked(true);
    tl->addWidget(m_topology);
    auto *th = new QLabel(tpl::tr("Пишется один раз при инициализации и почти ничего "
                                  "не стоит — выключать её обычно незачем.",
                                  "Written once at initialization and costs almost nothing — there is usually no "
                                  "reason to turn it off."));
    th->setProperty("mute", true);
    th->setWordWrap(true);
    tl->addWidget(th);
    l->addWidget(topo);

    auto *fv = new QGroupBox(tpl::tr("Мгновенные поля для FieldView", "Instantaneous fields for FieldView"));
    auto *vl = new QVBoxLayout(fv);
    m_fieldsOn = new QCheckBox(tpl::tr("Выгружать поля (файлы .ex, .ey, .hz)", "Export fields (.ex, .ey, .hz files)"));
    vl->addWidget(m_fieldsOn);
    m_fieldsFirst = new QRadioButton(tpl::tr("только на первом шаге", "only at the first step"));
    m_fieldsAll = new QRadioButton(tpl::tr("на всех шагах расчёта", "at all run steps"));
    m_fieldsFirst->setChecked(true);
    auto *rl2 = new QHBoxLayout;
    rl2->setContentsMargins(22, 0, 0, 0);
    rl2->addWidget(m_fieldsFirst);
    rl2->addWidget(m_fieldsAll);
    rl2->addStretch();
    vl->addLayout(rl2);
    m_fieldsHint = new QLabel;
    m_fieldsHint->setWordWrap(true);
    vl->addWidget(m_fieldsHint);
    l->addWidget(fv);

    auto *sm = new QGroupBox(tpl::tr("Матрица рассеяния (TMCGROUT)", "Scattering matrix (TMCGROUT)"));
    auto *sf = new QFormLayout(sm);
    m_sMatrix = new QCheckBox(tpl::tr("Снимать S-матрицу (файл .s)", "Record the S-matrix (.s file)"));
    m_sMatrix->setObjectName(QStringLiteral("sMatrixOn"));
    m_sMatrix->setChecked(true);
    sf->addRow(m_sMatrix);
    m_sFile = new QLineEdit(QStringLiteral("smatrix"));
    m_sFile->setObjectName(QStringLiteral("sMatrixFile"));
    m_sMin = spin(100, tpl::tr(" нс", " ns"), 0, 1e6);
    m_sMax = spin(110, tpl::tr(" нс", " ns"), 0, 1e6);
    sf->addRow(tpl::tr("Имя файла:", "File name:"), m_sFile);
    // Многочастотный режим (вход с файлом сигнала): окно по умолчанию — весь счёт,
    // строка SMATRIX пишется без T_min; T_max (решение К. Н. Климова 26.10.2026, Б-22).
    m_sWide = new QCheckBox(tpl::tr("Ограничить окно (вход с файлом сигнала)", "Limit the window (port with a signal file)"));
    m_sWide->setObjectName(QStringLiteral("sMatrixLimitWideband"));
    m_sWide->setToolTip(tpl::tr("У входа с файлом сигнала ядро считает S-матрицу по всей полосе, и по умолчанию "
        "окно — весь счёт, от 0 до TIME. Ограничить его стоит, когда отклик в начале "
        "счёта не нужен.",
        "For a port with a signal file the kernel computes the S-matrix over the whole "
        "band, and by default the window is the whole run, from 0 to TIME. Limit it "
        "when the response at the start of the run is not needed."));
    sf->addRow(m_sWide);
    sf->addRow(tpl::tr("Окно снятия от:", "Recording window from:"), m_sMin);
    sf->addRow(tpl::tr("Окно снятия до:", "Recording window to:"), m_sMax);
    auto *sh = new QLabel(tpl::tr("Файл .s ДОПИСЫВАЕТСЯ, а не перезаписывается — удаляйте его перед прогоном.", "The .s file is APPENDED, not overwritten — delete it before the run."));
    sh->setProperty("mute", true);
    sh->setWordWrap(true);
    sf->addRow(QString(), sh);
    l->addWidget(sm);

    auto *dn = new QGroupBox(tpl::tr("Диаграмма направленности (TMC_DN)", "Radiation pattern (TMC_DN)"));
    auto *df = new QFormLayout(dn);
    m_pattern = new QCheckBox(tpl::tr("Считать диаграмму направленности", "Compute the radiation pattern"));
    df->addRow(m_pattern);
    m_patternFile = new QLineEdit(QStringLiteral("pattern"));
    m_pMin = spin(100, tpl::tr(" нс", " ns"), 0, 1e6);
    m_pMax = spin(110, tpl::tr(" нс", " ns"), 0, 1e6);
    df->addRow(tpl::tr("Имя файла:", "File name:"), m_patternFile);
    df->addRow(tpl::tr("Окно накопления от:", "Accumulation window from:"), m_pMin);
    df->addRow(tpl::tr("Окно накопления до:", "Accumulation window to:"), m_pMax);
    auto *dh = new QLabel(tpl::tr("Даёт файлы .AMP, .FAZ и .DAT.", "Produces .AMP, .FAZ and .DAT files."));
    dh->setProperty("mute", true);
    df->addRow(QString(), dh);
    // Память. При сигнале из файла с несколькими тонами ядро копит диаграмму на
    // каждом тоне сразу, и память растёт с их числом — сказать об этом надо ДО
    // прогона, а не после ошибки выделения памяти на середине подготовки.
    m_patternInfo = new QLabel;
    m_patternInfo->setObjectName(QStringLiteral("patternInfo"));
    m_patternInfo->setWordWrap(true);
    m_patternInfo->setTextFormat(Qt::PlainText);
    df->addRow(QString(), m_patternInfo);
    l->addWidget(dn);

    // Замер времени расчёта.
    //
    // ⚠️ В ЯЗЫКЕ задания такого ключа НЕТ, и выдумывать для него строку в #PARM
    // нельзя. Это ключ командной строки ядра (`/Time`, сборки с 30.09.2026), поэтому
    // галочка меняет только строку запуска, которую программа показывает на странице
    // «Проверка» и кладёт в пакетный файл конвейера. Сам .tpl от неё не меняется ни
    // на байт. Старые сборки ядра ключ молча игнорируют.
    auto *tm = new QGroupBox(tpl::tr("Время расчёта", "Run time"));
    auto *tf = new QFormLayout(tm);
    m_timing = new QCheckBox(tpl::tr("Замерить время расчёта (ключ /Time)", "Measure the run time (/Time key)"));
    tf->addRow(m_timing);
    auto *timeHint = new QLabel(tpl::tr("Ядро напишет отчёт по фазам счёта в файл <задание>.timing_*.txt рядом с "
        "заданием. В самом задании ключа нет — он добавляется в строку запуска, "
        "поэтому файл .tpl от этой галочки не меняется.",
        "The kernel will write a report on the run phases to <task>.timing_*.txt next "
        "to the task. The task itself has no such key — it is added to the launch "
        "command, so the .tpl file does not change with this checkbox."));
    timeHint->setProperty("mute", true);
    timeHint->setWordWrap(true);
    tf->addRow(QString(), timeHint);
    l->addWidget(tm);
    l->addStretch();

    const auto touch = [this] { pullFromForm(); };
    connect(m_signalFile, &QLineEdit::textChanged, this, touch);
    connect(m_sFile, &QLineEdit::textChanged, this, touch);
    connect(m_patternFile, &QLineEdit::textChanged, this, touch);
    connect(m_topology, &QCheckBox::toggled, this, touch);
    connect(m_sMatrix, &QCheckBox::toggled, this, touch);
    connect(m_sWide, &QCheckBox::toggled, this, touch);
    connect(m_pattern, &QCheckBox::toggled, this, touch);
    connect(m_timing, &QCheckBox::toggled, this, touch);
    connect(m_fieldsOn, &QCheckBox::toggled, this, touch);
    connect(m_fieldsFirst, &QRadioButton::toggled, this, touch);
    for (auto *s : {m_sMin, m_sMax, m_pMin, m_pMax})
        connect(s, &QDoubleSpinBox::valueChanged, this, touch);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(page);
    scroll->setFrameShape(QFrame::NoFrame);
    return scroll;
}

QWidget *MainWindow::buildSummaryPage()
{
    auto *page = new QWidget;
    auto *l = new QVBoxLayout(page);
    l->setContentsMargins(0, 0, 0, 0);

    auto *split = new QSplitter(Qt::Horizontal);
    split->setChildrenCollapsible(false);

    auto *left = new QWidget;
    left->setMinimumWidth(260);
    auto *ll = new QVBoxLayout(left);
    ll->setContentsMargins(0, 0, 0, 0);
    auto *st = new QLabel(tpl::tr("Что будет посчитано", "What will be computed"));
    st->setProperty("dim", true);
    ll->addWidget(st);
    m_summary = new QPlainTextEdit;
    m_summary->setObjectName(QStringLiteral("summaryText"));
    m_summary->setReadOnly(true);
    ll->addWidget(m_summary, 1);

    // Память: хватит ли ОЗУ ЭТОГО компьютера. Только сведения — записать задание
    // можно в любом случае: считать его, возможно, будут на сервере.
    auto *mt = new QLabel(tpl::tr("Память на этом компьютере", "Memory on this computer"));
    mt->setProperty("dim", true);
    ll->addWidget(mt);
    m_memDonut = new MemoryDonut;
    ll->addWidget(m_memDonut);
    m_memStatus = new QLabel;
    m_memStatus->setObjectName(QStringLiteral("memoryStatus"));
    m_memStatus->setWordWrap(true);
    m_memStatus->setTextFormat(Qt::PlainText);
    ll->addWidget(m_memStatus);
    // Занятая память меняется, пока человек смотрит на страницу: открыл браузер —
    // свободного стало меньше. Опрашиваем раз в три секунды, и только когда страница
    // на экране: прятанной диаграмме обновляться незачем.
    m_memTimer = new QTimer(this);
    m_memTimer->setInterval(3000);
    connect(m_memTimer, &QTimer::timeout, this, [this] {
        if (m_memDonut && m_memDonut->isVisible())
            updateMemoryChart();
    });
    m_memTimer->start();
    split->addWidget(left);

    auto *right = new QWidget;
    right->setMinimumWidth(260);
    auto *rl = new QVBoxLayout(right);
    rl->setContentsMargins(0, 0, 0, 0);
    auto *pt = new QLabel(tpl::tr("Файл задания", "Task file"));
    pt->setProperty("dim", true);
    rl->addWidget(pt);
    m_preview = new QPlainTextEdit;
    m_preview->setObjectName(QStringLiteral("previewText"));
    m_preview->setReadOnly(true);
    // Текст задания — JetBrains Mono, как числа и тексты заданий в программах пакета
    // (шрифт вложен в программу и загружается в main); Consolas — если его нет.
    QFont mono(QStringLiteral("JetBrains Mono"), 9);
    mono.setFamilies({QStringLiteral("JetBrains Mono"), QStringLiteral("Consolas")});
    m_preview->setFont(mono);
    m_preview->setLineWrapMode(QPlainTextEdit::NoWrap);
    rl->addWidget(m_preview, 1);
    split->addWidget(right);
    split->setStretchFactor(1, 1);
    l->addWidget(split, 1);

    m_generate = new QPushButton(tpl::tr("Создать файл задания", "Create task file"));
    m_generate->setObjectName(QStringLiteral("btnGenerate"));
    m_generate->setProperty("accent", true);
    m_generate->setMinimumHeight(38);
    connect(m_generate, &QPushButton::clicked, this, &MainWindow::onGenerate);

    // Своё задание — в «Мои образцы»: кафедра пополняет библиотеку своими ЛР.
    auto *asSample = new QPushButton(tpl::tr("Сохранить как образец…", "Save as sample…"));
    asSample->setObjectName(QStringLiteral("saveAsSample"));
    asSample->setMinimumHeight(38);
    asSample->setToolTip(tpl::tr("Положить задание в библиотеку образцов, раздел "
                                 "«Мои образцы»: потом его можно взять оттуда, как "
                                 "встроенный образец",
                                 "Put the task into the sample library, section «My samples»: later it can be "
                                 "taken from there like a built-in sample"));
    connect(asSample, &QPushButton::clicked, this, &MainWindow::onSaveAsSample);

    auto *actions = new QHBoxLayout;
    actions->addWidget(asSample);
    actions->addWidget(m_generate, 1);
    l->addLayout(actions);
    return page;
}

// ------------------------------------------------------------------ состояние

void MainWindow::setProject(const Project &p)
{
    m_project = p;
    // Никакой геометрии по умолчанию: новый файл — чистый холст. Нарисованное
    // «на всякий случай» пришлось бы сначала искать и удалять.
    if (m_project.output.signalFile.isEmpty())
        m_project.output.signalFile = QStringLiteral("signal");
    if (m_project.output.sMatrixFile.isEmpty())
        m_project.output.sMatrixFile = QStringLiteral("smatrix");
    if (m_project.output.patternFile.isEmpty())
        m_project.output.patternFile = QStringLiteral("pattern");
    if (m_project.name.isEmpty())
        m_project.name = QStringLiteral("task");
    if (m_project.freqs.isEmpty())
        m_project.freqs = {4.0};

    m_undo.clear();
    m_redo.clear();
    pushToForm();
    m_view->setProject(&m_project);
    m_panel->setProject(&m_project);
    m_plan->setProject(&m_project);
    refreshAll();
    m_view->zoomToFit();
}

void MainWindow::resetProject()
{
    // Именно новый Project, а не «почистить поля»: так не останется ни забытой
    // фигуры, ни варианта расчёта, ни имени прошлого файла. Раньше «Создать»
    // возвращало в уже открытое задание, и пользователь начинал новую работу
    // поверх старой, не заметив этого.
    setProject(Project{});
    m_sourceDir.clear();
    m_sourceName.clear();
    m_companions.clear();

    // Начинаем с первого раздела: продолжать с той страницы, где закончили
    // прошлое задание, бессмысленно.
    if (m_tree && m_tree->topLevelItemCount() > 0)
        m_tree->setCurrentItem(m_tree->topLevelItem(0));
}

void MainWindow::openSample(const Project &p, const QMap<QString, QByteArray> &companions)
{
    // Образец открывается как НОВОЕ задание: у него нет исходной папки, и «сохранить
    // туда же» означало бы гадать, куда. Начинаем с чистого листа и кладём образец.
    resetProject();
    setProject(p);
    m_companions = companions;
}

void MainWindow::onSaveAsSample()
{
    pullFromForm();
    const Diagnostics d = validate(m_project);
    if (!d.ok()) {
        IssueDialog::report(this, tpl::tr("Нельзя сохранить образец", "Cannot save the sample"),
                          tpl::tr("В образцы кладутся только задания, которые ядро примет. "
                                  "Сначала исправьте:",
                                  "Only tasks the kernel will accept go into samples. Fix these first:"),
                          d.errors, {});
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle(tpl::tr("Сохранить как образец", "Save as sample"));
    auto *form = new QFormLayout(&dlg);
    auto *title = new QLineEdit;
    const QString firstLine = m_project.description.section(QLatin1Char('\n'), 0, 0).trimmed();
    title->setText(firstLine.isEmpty() ? m_project.name : firstLine.left(80));
    title->setMinimumWidth(420);
    auto *summary = new QPlainTextEdit;
    summary->setPlaceholderText(tpl::tr("Что это за задание и на что смотреть в результатах", "What this task is and what to look at in the results"));
    summary->setMaximumHeight(110);
    auto *where = new QLabel(tpl::tr("Ляжет в: %1", "Will be saved to: %1").arg(samples::userSamplesRoot()));
    where->setProperty("mute", true);
    where->setWordWrap(true);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText(tpl::tr("Сохранить", "Save"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tpl::tr("Отмена", "Cancel"));
    form->addRow(tpl::tr("Название:", "Title:"), title);
    form->addRow(tpl::tr("Описание:", "Description:"), summary);
    form->addRow(where);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted || title->text().trimmed().isEmpty())
        return;

    QString dir;
    const WriteResult r = samples::saveUserSample(title->text(), summary->toPlainText(), m_project,
                                                  m_companions, &dir);
    if (!r.ok) {
        QMessageBox::critical(this, tpl::tr("Не удалось сохранить образец", "Could not save the sample"), r.error);
        return;
    }
    QMessageBox::information(this, tpl::tr("Образец сохранён", "Sample saved"),
                             tpl::tr("Задание лежит в «Моих образцах»:\n\n%1\n\nЕго видно "
                                     "в библиотеке образцов, раздел «Мои образцы».",
                                     "The task is in «My samples»:\n\n%1\n\nIt is visible in the sample library, "
                                     "section «My samples».")
                                 .arg(dir));
}

bool MainWindow::hasContent() const
{
    return !m_project.shapes.isEmpty() || !m_project.ports.isEmpty()
           || !m_sourceName.isEmpty();
}

bool MainWindow::loadFromFile(const QString &path)
{
    ReadResult r = readTplFile(path);

    // А-13 (ответ автора 29.10.2026): единица ниже своей величины — ядра с 29.10 такое
    // задание отвергают. Спрашиваем: «да» — читаем так, будто строку единицы подняли
    // наверх секции (так задумано автором файла); «нет» — как написано (величина в
    // прежней единице). Файл на диске не трогаем: исправленным он станет при сохранении.
    if (r.ok() && !r.unitAfterValueLines.isEmpty()) {
        QStringList where;
        for (int ln : std::as_const(r.unitAfterValueLines))
            where << QString::number(ln);
        const auto answer = QMessageBox::question(
            this, tpl::tr("Единица после величины", "Unit after its value"),
            tpl::tr("Здесь единица стоит после величины (строки %1 файла %2) — исправить?\n\n"
                    "«Да» — единица действует на всю свою секцию #PARM, как будто её строка "
                    "стоит первой. «Нет» — как написано: величина выше читается в прежней "
                    "единице. Ядра с 29.10.2026 такое задание отвергают; при сохранении "
                    "программа всё равно запишет единицы первыми.",
                    "Here a unit is written after its value (lines %1 of %2) - fix it?\n\n"
                    "\"Yes\" - the unit acts on its whole #PARM section, as if its line came "
                    "first. \"No\" - as written: the value above is read in the previous unit. "
                    "Kernels since 29.10.2026 reject such a task; on saving the program writes "
                    "the units first anyway.")
                .arg(where.join(QStringLiteral(", ")), QFileInfo(path).fileName()),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (answer == QMessageBox::Yes)
            r = readTplFile(path, true);
    }

    if (!r.ok()) {
        IssueDialog::report(this, tpl::tr("Не удалось прочитать файл", "Could not read the file"),
                          tpl::tr("Файл %1 прочитать не получилось.", "File %1 could not be read.")
                              .arg(QFileInfo(path).fileName()),
                          r.errors, {});
        return false;
    }

    setProject(r.project);
    m_sourceDir = QFileInfo(path).absolutePath();
    m_sourceName = QFileInfo(path).completeBaseName();

    // Готовые распределения eps (блок FILE) ядро ищет в папке задания. Подхватываем
    // их сразу: сохранив задание в другую папку, пользователь иначе получил бы файл,
    // ссылающийся на распределение, которого рядом нет.
    m_companions.clear();
    for (const Shape &s : std::as_const(m_project.shapes)) {
        if (!s.epsProfile.readsFile())
            continue;
        const QString name = epsProfileFile(s) + QStringLiteral(".eps");
        QFile f(QDir(m_sourceDir).filePath(name));
        if (f.open(QIODevice::ReadOnly))
            m_companions.insert(name, f.readAll());
    }

    // Что прочитано — говорим числами, а не «файл открыт». Пользователь должен
    // сверить это с тем, что он ожидал увидеть, до того как начнёт править.
    QString msg = tpl::tr("Прочитано из %1:\n\n", "Read from %1:\n\n").arg(QFileInfo(path).fileName());
    msg += tpl::tr("  фигур: %1\n  портов: %2\n  частот: %3\n", "  shapes: %1\n  ports: %2\n  frequencies: %3\n")
               .arg(m_project.shapes.size())
               .arg(m_project.ports.size())
               .arg(m_project.freqs.size());
    if (!m_project.variants.isEmpty())
        msg += tpl::tr("  вариантов геометрии: %1\n", "  geometry variants: %1\n").arg(m_project.variants.size());
    if (r.wasMacro) {
        msg += tpl::tr("\nФайл был макро-версией", "\nThe file was a macro version");
        if (!r.includedFiles.isEmpty())
            msg += tpl::tr(" (подключено: %1)", " (included: %1)").arg(r.includedFiles.join(QStringLiteral(", ")));
        msg += tpl::tr(". Макросы раскрыты — правите геометрию, а не текст.\n", ". Macros are expanded — you edit the geometry, not the text.\n");
    }

    // Прочитанное могло быть и некорректным заданием — файл ведь писали руками.
    // Ошибки видны на вкладке проверки, но сказать о них надо сразу.
    //
    // Всё в ОДНО окно: два-три модальных окна подряд после открытия файла — это
    // не забота, а препятствие. Список прокручивается, повторы схлопнуты.
    const Diagnostics d = validate(m_project);
    if (!r.warnings.isEmpty() || !d.ok()) {
        if (!r.warnings.isEmpty()) {
            msg += tpl::tr("\nЧасть задания модель редактора выражает не так, как в "
                           "файле. Если сохранить поверх — написанное ниже изменится.",
                           "\nThe editor model expresses part of the task differently from the file. If "
                           "you save over it, what is written below will change.");
        }
        if (!d.ok()) {
            msg += tpl::tr("\nКак задание файл неверен: сгенерировать его программа "
                           "не даст, пока ошибки не исправлены.",
                           "\nAs a task the file is invalid: the program will not generate it until the "
                           "errors are fixed.");
        }
        IssueDialog::report(this,
                          d.ok() ? tpl::tr("Файл прочитан не полностью", "File read incompletely")
                                 : tpl::tr("В задании есть ошибки", "The task has errors"),
                          msg, d.errors, r.warnings);
    } else {
        QMessageBox::information(this, tpl::tr("Файл прочитан", "File read"), msg);
    }
    return true;
}

void MainWindow::pushToForm()
{
    m_loading = true;
    m_name->setText(m_project.name);
    m_kernel->setCurrentIndex(m_kernel->findData(int(m_project.kernel)));
    m_unit->setCurrentIndex(m_unit->findData(int(m_project.unit)));
    m_xMin->setValue(m_project.box.xMin);
    m_xMax->setValue(m_project.box.xMax);
    m_yMin->setValue(m_project.box.yMin);
    m_yMax->setValue(m_project.box.yMax);
    m_delta->setValue(m_project.delta);
    m_time->setValue(m_project.timeMax);

    if (m_project.freqs.size() > 1) {
        m_freqRange->setChecked(true);
        m_freqStart->setValue(m_project.freqs.first());
        m_freqEnd->setValue(m_project.freqs.last());
        // Шаг восстанавливаем по сетке: без него поле осталось бы от прежнего
        // задания, и «до» с «шагом» перестали бы сходиться с числом точек.
        if (m_project.freqs.size() > 1) {
            const double st = (m_project.freqs.last() - m_project.freqs.first())
                              / (m_project.freqs.size() - 1);
            if (st > 0.0)
                m_freqStep->setValue(st);
        }
    } else {
        m_freqOne->setChecked(true);
        if (!m_project.freqs.isEmpty())
            m_freqSingle->setValue(m_project.freqs.first());
    }
    m_chunkOn->setChecked(m_project.chunking.enabled);
    m_chunkSize->setValue(qBound(1, m_project.chunking.freqsPerTask, 99));

    m_signalFile->setText(m_project.output.signalFile);
    m_topology->setChecked(m_project.output.topology);
    m_fieldsOn->setChecked(m_project.output.fields != OutputSettings::Fields::Off);
    m_fieldsAll->setChecked(m_project.output.fields == OutputSettings::Fields::AllSteps);
    m_fieldsFirst->setChecked(m_project.output.fields != OutputSettings::Fields::AllSteps);
    m_sMatrix->setChecked(m_project.output.sMatrix);
    m_sFile->setText(m_project.output.sMatrixFile);
    m_sMin->setValue(m_project.output.sMatrixTMin);
    m_sMax->setValue(m_project.output.sMatrixTMax);
    m_sWide->setChecked(m_project.output.sMatrixLimitWideband);
    m_pattern->setChecked(m_project.output.directionalPattern);
    m_patternFile->setText(m_project.output.patternFile);
    m_pMin->setValue(m_project.output.patternTMin);
    m_pMax->setValue(m_project.output.patternTMax);
    m_timing->setChecked(m_project.output.timingReport);
    m_loading = false;

    // Блокировки зависимых полей ставим ЗДЕСЬ, а не только на смене поля:
    // иначе при первом показе окна поля диапазона стоят активными, хотя выбрана
    // одна частота.
    updateEnabledStates();
}

void MainWindow::updateEnabledStates()
{
    const bool range = m_freqRange->isChecked();
    m_freqSingle->setEnabled(!range);
    m_freqStart->setEnabled(range);
    m_freqEnd->setEnabled(range);
    m_freqStep->setEnabled(range);

    const bool fields = m_fieldsOn->isChecked();
    m_fieldsFirst->setEnabled(fields);
    m_fieldsAll->setEnabled(fields);

    const bool s = m_sMatrix->isChecked();
    // Окно у многочастотного шага — по желанию, у одночастотного — обязательно.
    const bool wide = tpl::hasFileSignal(m_project);
    m_sWide->setVisible(wide);
    m_sWide->setEnabled(s);
    const bool window = s && (!wide || m_sWide->isChecked());
    m_sFile->setEnabled(s);
    m_sMin->setEnabled(window);
    m_sMax->setEnabled(window);

    const bool dn = m_pattern->isChecked();
    m_patternFile->setEnabled(dn);
    m_pMin->setEnabled(dn);
    m_pMax->setEnabled(dn);
}

QVector<double> MainWindow::collectFreqs() const
{
    QVector<double> f;
    if (m_freqOne->isChecked()) {
        f << m_freqSingle->value();
        return f;
    }
    const double a = m_freqStart->value(), b = m_freqEnd->value(), s = m_freqStep->value();
    if (s <= 0.0 || b < a)
        return {a};
    // Полшага допуска: иначе последняя точка теряется на округлении double.
    for (double x = a; x <= b + s * 0.5; x += s)
        f << x;
    return f;
}

/// Самый мелкий шаг (в тысячных ГГц — столько знаков у поля), при котором диапазон
/// укладывается в 99 частот. Считает ТЕМ ЖЕ циклом, что и collectFreqs: формула
/// «(b-a)/98» на краю расходилась бы с ним на одну точку из-за допуска в полшага.
double MainWindow::stepForMaxFreqs() const
{
    const double a = m_freqStart->value(), b = m_freqEnd->value();
    const auto count = [&](double s) {
        int c = 0;
        for (double x = a; x <= b + s * 0.5 && c <= tpl::kMaxFreqs; x += s)
            ++c;
        return c;
    };
    double s = std::ceil((b - a) / double(tpl::kMaxFreqs - 1) * 1000.0) / 1000.0;
    s = std::max(s, 0.001);
    while (count(s) > tpl::kMaxFreqs)
        s += 0.001;
    return s;
}

void MainWindow::pullFromForm()
{
    if (m_loading)
        return;

    m_project.name = m_name->text();
    m_project.kernel = Kernel(m_kernel->currentData().toInt());
    m_project.unit = LengthUnit(m_unit->currentData().toInt());
    m_project.box = {m_xMin->value(), m_xMax->value(), m_yMin->value(), m_yMax->value()};
    m_project.delta = m_delta->value();
    m_project.timeMax = m_time->value();
    m_project.freqs = collectFreqs();
    m_project.chunking.enabled = m_chunkOn->isChecked();
    m_project.chunking.freqsPerTask = m_chunkSize->value();

    m_project.output.signalFile = m_signalFile->text();
    m_project.output.topology = m_topology->isChecked();
    m_project.output.fields = !m_fieldsOn->isChecked() ? OutputSettings::Fields::Off
                              : m_fieldsAll->isChecked() ? OutputSettings::Fields::AllSteps
                                                         : OutputSettings::Fields::FirstStep;
    m_project.output.sMatrix = m_sMatrix->isChecked();
    m_project.output.sMatrixFile = m_sFile->text();
    m_project.output.sMatrixTMin = m_sMin->value();
    m_project.output.sMatrixTMax = m_sMax->value();
    m_project.output.sMatrixLimitWideband = m_sWide->isChecked();
    m_project.output.directionalPattern = m_pattern->isChecked();
    m_project.output.patternFile = m_patternFile->text();
    m_project.output.patternTMin = m_pMin->value();
    m_project.output.patternTMax = m_pMax->value();
    m_project.output.timingReport = m_timing->isChecked();

    // Окно снятия обязано лежать внутри времени счёта — иначе ядро посчитает, а файл
    // выйдет пустым. Время счёта меняют на странице «Расчёт», а окно живёт на
    // «Выводе данных», и связь между ними человеку не видна вовсе: уменьшил TIME —
    // получил ошибку в другом разделе, про которую не думал. Поэтому окно едет за
    // временем САМО, по тому же правилу, что и в библиотеке образцов: последняя
    // четверть счёта, но не меньше трёх наносекунд.
    followTimeWindow();

    updateEnabledStates();
    refreshAll();
}

/// Подтянуть окна снятия под время счёта, если они за него вышли.
///
/// Трогаем ТОЛЬКО то, что перестало помещаться: окно, которое человек выставил внутри
/// счёта, остаётся как есть. Иначе правка времени затирала бы осмысленный выбор.
void MainWindow::followTimeWindow()
{
    const double t = m_project.timeMax;
    if (t <= 0.0)
        return;
    const double win = std::max(3.0, t * 0.25);
    const auto fix = [&](double *lo, double *hi, QDoubleSpinBox *loBox, QDoubleSpinBox *hiBox) {
        if (*hi <= t && *lo < *hi)
            return;
        *lo = std::max(0.0, t - win);
        *hi = t;
        QSignalBlocker b1(loBox), b2(hiBox);
        loBox->setValue(*lo);
        hiBox->setValue(*hi);
    };
    fix(&m_project.output.sMatrixTMin, &m_project.output.sMatrixTMax, m_sMin, m_sMax);
    fix(&m_project.output.patternTMin, &m_project.output.patternTMax, m_pMin, m_pMax);

    // Окно ДН ядро дотягивает до целого числа периодов и при конце не раньше TIME не
    // закрывает — файлов ДН не будет (Б-13). «До конца счёта» для ДН — это на период
    // самой низкой частоты раньше. Правим, только если окно перестало закрываться.
    if (m_project.output.directionalPattern
        && !tpl::patternWindowCloses(m_project, m_project.output.patternTMin,
                                     m_project.output.patternTMax)) {
        const double hi = tpl::patternWindowSafeEnd(m_project, m_project.output.patternTMin,
                                                    m_project.output.patternTMax);
        if (hi > m_project.output.patternTMin) {
            m_project.output.patternTMax = hi;
            QSignalBlocker b(m_pMax);
            m_pMax->setValue(hi);
        }
    }
}

void MainWindow::refreshAll()
{
    // Многочастотный ли режим, зависит от входов: после правки порта доступность окна
    // SMATRIX надо пересчитать и здесь, а не только при правке полей вывода.
    updateEnabledStates();

    // Режим файла выбирается сам, но пользователь должен видеть, ЧТО он получит:
    // один файл или два, и почему. Иначе второй файл становится сюрпризом.
    const int n = m_project.freqs.size();
    const int steps = m_project.stepCount();
    const int chunks = m_project.chunkCount();
    m_freqInfo->setStyleSheet(QString());
    if (n > tpl::kMaxFreqs && chunks == 1) {
        // Больше 99 частот в одно задание пакет не берёт. Запись запрещает проверка, а
        // здесь человек сразу видит оба выхода и берёт любой одним щелчком: шаг
        // покрупнее или разложение сетки по заданиям.
        const double step = stepForMaxFreqs();
        m_freqInfo->setText(
            tpl::tr("<b>Частот %1 — в одно задание больше %2 нельзя</b>: столько не "
                    "берёт ни файл .s, ни диаграмма направленности, ни "
                    "постобработка. <a href=\"step\">Взять шаг %3 ГГц</a> или "
                    "<a href=\"chunk\">разложить по нескольким заданиям</a>.",
                    "<b>%1 frequencies — no more than %2 fit into one task</b>: neither the .s "
                    "file, nor the radiation pattern, nor post-processing can take more. <a "
                    "href=\"step\">Take step %3 GHz</a> or <a href=\"chunk\">split into several "
                    "tasks</a>.")
                .arg(n).arg(tpl::kMaxFreqs).arg(tpl::formatCompact(step)));
        m_freqInfo->setStyleSheet(QStringLiteral("color:%1;").arg(dtheme::danger().name()));
    } else if (n > 1) {
        m_freqInfo->setText(
            tpl::tr("%1 точек, шагов расчёта %2. Получите ДВА файла: %3.tpl и "
                    "%3_step.h — шаг описан в них один раз, поэтому "
                    "рассинхронизировать частоты нечем. Оба класть в одну папку.",
                    "%1 points, %2 run steps. You get TWO files: %3.tpl and %3_step.h — the step "
                    "is described in them once, so the frequencies cannot get out of sync. Put "
                    "both into the same folder.")
                .arg(n).arg(steps).arg(m_project.name.toHtmlEscaped()));
    } else {
        m_freqInfo->setText(
            tpl::tr("Одна частота — один обычный файл %1.tpl без макросов.", "Single frequency — one ordinary file %1.tpl without macros.")
                .arg(m_project.name.toHtmlEscaped()));
    }

    // Конвейер. Доступен только когда делить есть что.
    m_chunkOn->setEnabled(n > 1);
    m_chunkSize->setEnabled(n > 1 && m_chunkOn->isChecked());
    if (n > tpl::kMaxFreqs && chunks == 1) {
        m_chunkInfo->clear(); // всё сказано строкой выше, с выходами на выбор
    } else if (chunks > 1) {
        m_chunkInfo->setText(
            tpl::tr("Выйдет %1: %2_001.tpl и далее, по %3 в каждом, плюс пакетные "
                    "файлы %2_run.bat и %2_run.sh для прогона всей пачки. У каждого "
                    "задания свой .s: мешать точки разных файлов нельзя, да и "
                    "считать их можно одновременно.",
                    "This gives %1: %2_001.tpl and onward, %3 each, plus batch files %2_run.bat "
                    "and %2_run.sh to run the whole set. Each task has its own .s: points of "
                    "different files must not be mixed, and they can also be run simultaneously.")
                .arg(tpl::plural(chunks, tpl::tr("задание", "task"), tpl::tr("задания", "tasks"),
                                 tpl::tr("заданий", "tasks")),
                     m_project.name,
                     tpl::plural(m_project.chunking.freqsPerTask, tpl::tr("частоте", "frequency"),
                                 tpl::tr("частоты", "frequencies"), tpl::tr("частот", "frequencies"))));
        m_chunkInfo->setStyleSheet(QStringLiteral("color:#6c7d97;"));
    } else if (n > 1) {
        m_chunkInfo->setText(tpl::tr("Все %1 точек посчитает одно задание за один "
                                     "запуск.",
                                     "All %1 points are computed by one task in a single run.").arg(n));
        m_chunkInfo->setStyleSheet(QStringLiteral("color:#6c7d97;"));
    } else {
        m_chunkInfo->clear();
    }

    // Память на диаграмму. Говорим числом всегда, когда ДН заказана, а цветом —
    // только когда пора беспокоиться: 2 ГБ — это потолок 32-разрядного ядра.
    if (m_project.output.directionalPattern) {
        const tpl::RamEstimate ram = tpl::ramEstimate(m_project);
        QString t;
        if (ram.patternTones > 1) {
            t = tpl::tr("Сигнал из файла несёт %1 — диаграмма копится на каждой сразу, "
                        "по краю области, и выйдет файлом %2_<МГц>MHz.DAT на частоту. "
                        "Памяти понадобится около %3: сетка %4, накопление ДН %5.",
                        "The signal from the file carries %1 — the pattern is accumulated on all of "
                        "them at once, along the domain edge, and comes out as a file %2_<MHz>MHz.DAT "
                        "per frequency. About %3 of memory will be needed: grid %4, pattern "
                        "accumulation %5.")
                    .arg(tpl::plural(ram.patternTones, tpl::tr("частоту", "frequency"),
                                     tpl::tr("частоты", "frequencies"), tpl::tr("частот", "frequencies")),
                         m_project.output.patternFile, tpl::formatBytes(ram.total()),
                         tpl::formatBytes(ram.gridBytes), tpl::formatBytes(ram.patternBytes));
            if (ram.patternToneRequest > tpl::kMaxFreqs) {
                t += tpl::tr(" Тонов в сигнале %1, а пакет берёт не больше %2: "
                             "диаграммы будут по первым %2, остальные отброшены.",
                             " The signal has %1 tones, but the package takes no more than %2: patterns "
                             "will be for the first %2, the rest are dropped.")
                         .arg(ram.patternToneRequest).arg(tpl::kMaxFreqs);
            }
        } else {
            t = tpl::tr("Памяти понадобится около %1: сетка %2, накопление ДН %3.", "About %1 of memory will be needed: grid %2, pattern accumulation %3.")
                    .arg(tpl::formatBytes(ram.total()), tpl::formatBytes(ram.gridBytes),
                         tpl::formatBytes(ram.patternBytes));
        }
        const bool heavy = ram.total() >= tpl::kRamWarnBytes;
        if (heavy) {
            t += tpl::tr(" Это больше 2 ГБ: 32-разрядное ядро столько не возьмёт, и "
                         "на машине должно быть свободно не меньше.",
                         " This is more than 2 GB: a 32-bit kernel cannot take that much, and the "
                         "machine must have at least as much free.");
        }
        m_patternInfo->setText(t);
        m_patternInfo->setStyleSheet(QStringLiteral("color:%1;").arg(
            (heavy || ram.patternToneRequest > tpl::kMaxFreqs) ? dtheme::warn().name()
                                                                : QStringLiteral("#6c7d97")));
    } else {
        m_patternInfo->clear();
    }

    switch (m_project.output.fields) {
    case OutputSettings::Fields::Off:
        m_fieldsHint->setText(tpl::tr("Быстро. Смотреть поля в FieldView будет нечем.", "Fast. There will be nothing to view in FieldView."));
        m_fieldsHint->setStyleSheet(QStringLiteral("color:#6c7d97;"));
        break;
    case OutputSettings::Fields::FirstStep:
        m_fieldsHint->setText(tpl::tr("Разумный выбор для диапазона: первый шаг "
                                      "запишется отдельно, остальные пойдут быстро.",
                                      "A sensible choice for a range: the first step is written separately, the rest "
                                      "run fast."));
        m_fieldsHint->setStyleSheet(QStringLiteral("color:#6c7d97;"));
        break;
    case OutputSettings::Fields::AllSteps:
        m_fieldsHint->setText(tpl::tr("Очень долго: ядро переписывает всю сетку "
                                      "на каждом такте счёта.",
                                      "Very slow: the kernel rewrites the whole grid at every time step."));
        m_fieldsHint->setStyleSheet(QStringLiteral("color:#e3b341;"));
        break;
    }

    m_view->setProject(&m_project);
    m_view->setDisplayUnit(m_project.unit);
    m_panel->showSelection(m_view->selection());
    refreshLegend();
    refreshSummary();
    updateDiagnostics();
    updateHistoryButtons();
}

void MainWindow::refreshLegend()
{
    const QSignalBlocker block(m_legend);
    m_legend->clear();
    for (const Shape &s : m_project.shapes) {
        QString what;
        if (s.plasma.any())
            what = tpl::tr("плазма: %1", "plasma: %1").arg(plasmaShort(s.plasma));
        else if (s.epsProfile.readsFile())
            what = tpl::tr("eps из файла %1.eps", "eps from file %1.eps").arg(epsProfileFile(s));
        else if (s.hasEpsProfile())
            what = tpl::tr("eps по формуле", "eps by formula");
        else if (s.material == Material::Dielectric)
            what = QStringLiteral("eps = %1").arg(s.eps);
        else
            what = materialName(s.material);
        if (s.kind == ShapeKind::Rectangle && !s.hasUniformEdges())
            what += tpl::tr(", стороны разные", ", sides differ");

        auto *it = new QListWidgetItem(QStringLiteral("%1 — %2").arg(s.name, what), m_legend);
        it->setData(Qt::UserRole, s.id);
    }
    for (const Port &p : m_project.ports) {
        auto *it = new QListWidgetItem(
            QStringLiteral("%1 — %2").arg(p.name, p.active ? tpl::tr("активный", "active")
                                                           : tpl::tr("пассивный", "passive")),
            m_legend);
        it->setData(Qt::UserRole, p.id);
        it->setForeground(p.active ? dtheme::ok() : dtheme::danger());
    }
    // Строки созданы заново — подсветку и доступность кнопок восстанавливаем по
    // выделению ХОЛСТА: оно правку проекта пережило, а выделение списка нет.
    syncLegendSelection();
    updateMergeButton();
}

/// Что выбрано в списке объектов — только ФИГУРЫ, порты отсеиваем.
///
/// Список общий на то и другое, а сшивать порты бессмысленно: у входа нет точек.
QVector<int> MainWindow::selectedShapeIds() const
{
    QVector<int> ids;
    for (QListWidgetItem *it : m_legend->selectedItems()) {
        const int id = it->data(Qt::UserRole).toInt();
        for (const Shape &s : m_project.shapes) {
            if (s.id == id)
                ids << id;
        }
    }
    return ids;
}

void MainWindow::syncLegendSelection()
{
    if (m_syncingSelection)
        return;
    m_syncingSelection = true;
    const QSignalBlocker block(m_legend);

    const QVector<int> ids = m_view->selectedShapeIds();
    const int primary = m_view->selection().id;
    for (int i = 0; i < m_legend->count(); ++i) {
        QListWidgetItem *it = m_legend->item(i);
        const int id = it->data(Qt::UserRole).toInt();
        it->setSelected(ids.contains(id) || id == primary);
        if (id == primary)
            m_legend->setCurrentRow(i, QItemSelectionModel::NoUpdate);
    }
    m_syncingSelection = false;
}

void MainWindow::updateMergeButton()
{
    const QVector<int> ids = m_view->selectedShapeIds();
    int fit = 0;
    for (const Shape &s : m_project.shapes) {
        if (ids.contains(s.id)
            && (s.kind == ShapeKind::Polyline || s.kind == ShapeKind::Polygon))
            ++fit;
    }
    // Доступна ровно тогда, когда сработает: обещать кнопкой и отказывать нажатием —
    // худший вид интерфейса.
    const bool ok = fit >= 2 && fit == ids.size();
    m_panel->setMergeEnabled(
        ok,
        ids.size() >= 2 && fit != ids.size()
            ? tpl::tr("Соединять можно только линии, дуги и контуры. "
                      "Прямоугольник и круг в языке записаны иначе.",
                      "Only lines, arcs and polygons can be merged. A rectangle and a circle are "
                      "written differently in the language.")
            : QString());
}

void MainWindow::mergeSelectedShapes()
{
    const QVector<int> ids = m_view->selectedShapeIds();
    QString why;
    if (!m_view->mergeShapes(ids, &why)) {
        QMessageBox::information(this, tpl::tr("Соединить не удалось", "Could not merge"), why);
        return;
    }
    refreshAll();

    // Итог пишем в строку состояния под холстом — она для того и есть. Разрыв между
    // кусками показываем отдельным окном: это неожиданность, и пропустить её нельзя,
    // а строку состояния затрёт первое же движение мыши.
    m_hint->setText(why);
    if (why.contains(QChar(0x26A0)))
        QMessageBox::information(this, tpl::tr("Фигуры соединены", "Shapes merged"), why);
}

void MainWindow::updateDiagnostics()
{
    const Diagnostics d = validate(m_project);
    m_diag->clear();
    // Повторы схлопываем: полсотни одинаковых строк не сообщают больше, чем одна,
    // зато прячут за собой всё остальное.
    for (const QString &e : collapseIssues(d.errors)) {
        auto *it = new QListWidgetItem(tpl::tr("Ошибка: ", "Error:") + e, m_diag);
        it->setForeground(dtheme::danger());
    }
    for (const QString &w : collapseIssues(d.warnings)) {
        auto *it = new QListWidgetItem(tpl::tr("Внимание: ", "Warning:") + w, m_diag);
        it->setForeground(dtheme::warn());
    }
    if (d.errors.isEmpty() && d.warnings.isEmpty()) {
        auto *it = new QListWidgetItem(tpl::tr("Замечаний нет.", "No issues."), m_diag);
        it->setForeground(dtheme::ok());
    }
    if (m_generate)
        m_generate->setEnabled(d.ok());
}

void MainWindow::updateMemoryChart()
{
    if (!m_memDonut)
        return;
    const sysmem::Info mem = sysmem::query();
    // Свободную память передаём в оценку: по ней ядро само решает, копить ли ДН одной
    // частоты по всей области или по краю, — и картинка обязана решить так же.
    const tpl::RamEstimate ram = tpl::ramEstimate(m_project, mem.ok ? mem.available : 0.0);

    MemoryDonut::Data d;
    d.known = mem.ok;
    d.total = mem.total;
    d.used = mem.used();
    d.grid = ram.gridBytes;
    d.patternOn = ram.pattern;
    d.pattern = ram.patternBytes;
    d.patternBorder = ram.patternBorder;
    d.patternTones = ram.patternTones;
    m_memDonut->setData(d);

    const double need = ram.total();
    QString text;
    QColor color = dtheme::textMute();
    if (!mem.ok) {
        text = tpl::tr("Узнать память этого компьютера не удалось. Расчёту нужно около %1.", "Could not determine the memory of this computer. The run needs about %1.")
                   .arg(tpl::formatBytes(need));
    } else if (need > mem.available) {
        text = tpl::tr("На этом компьютере задание может не поместиться в оперативную "
                       "память: расчёту нужно около %1, а свободно %2 (всего %3). "
                       "Сохранить файл можно — посчитать его стоит на машине, где "
                       "памяти больше, например на сервере.",
                       "The task may not fit into the RAM of this computer: the run needs about %1, "
                       "while %2 is free (%3 total). The file can be saved — it is better to run it "
                       "on a machine with more memory, for example on a server.")
                   .arg(tpl::formatBytes(need), tpl::formatBytes(mem.available),
                        tpl::formatBytes(mem.total));
        color = dtheme::danger();
    } else if (need > 0.8 * mem.available) {
        text = tpl::tr("Впритык: расчёту нужно около %1 из %2 свободных. Закройте "
                       "лишние программы перед прогоном.",
                       "Tight: the run needs about %1 of %2 free. Close unneeded programs before the "
                       "run.")
                   .arg(tpl::formatBytes(need), tpl::formatBytes(mem.available));
        color = dtheme::warn();
    } else {
        text = tpl::tr("Поместится: расчёту нужно около %1, свободно %2.", "Fits: the run needs about %1, %2 is free.")
                   .arg(tpl::formatBytes(need), tpl::formatBytes(mem.available));
    }
    // Двухгигабайтный потолок 32-разрядного ядра — отдельно: памяти на машине может
    // быть сколько угодно, а 32-разрядный процесс больше не возьмёт.
    if (need >= tpl::kRamWarnBytes)
        text += tpl::tr(" Больше 2 ГБ — только 64-разрядным ядром.", " More than 2 GB — only with a 64-bit kernel.");
    m_memStatus->setText(text);
    m_memStatus->setStyleSheet(QStringLiteral("color:%1;").arg(color.name()));
}

void MainWindow::refreshSummary()
{
    if (!m_summary)
        return;
    updateMemoryChart();

    const Diagnostics d = validate(m_project);
    QString s;
    const auto line = [&s](const QString &t) { s += t + QLatin1Char('\n'); };

    line(tpl::tr("ЗАДАНИЕ: %1.tpl", "TASK: %1.tpl").arg(m_project.name));
    line(tpl::tr("Ядро: %1", "Kernel: %1").arg(m_project.kernel == Kernel::X
                                            ? tpl::tr("%1 (X-мода, с плазмой)", "%1 (X-mode, with plasma)")
                                                  .arg(kernelProgram(Kernel::X))
                                            : tpl::tr("%1 (H-мода)", "%1 (H-mode)")
                                                  .arg(kernelProgram(Kernel::H))));
    // Какой вид файла получится и ПОЧЕМУ. Выбор делает программа, но он не должен
    // быть для пользователя загадкой: от него зависит, сколько файлов уносить в расчёт.
    if (m_project.sweep.active()) {
        const AngleSweep &sw = m_project.sweep;
        const QPointF c = sweepPivot(m_project);
        line(tpl::tr("Вид файла: макро-версия (%1.tpl + %1_step.h) — объект поворачивается "
                     "по шагам. Шаг с объектом описан ОДИН раз, углов %2%3",
                     "File type: macro version (%1.tpl + %1_step.h) — the object is rotated step by "
                     "step. The step with the object is described ONCE, angles %2%3")
                 .arg(m_project.name)
                 .arg(sw.angles.size())
                 .arg(sw.referenceStep ? tpl::tr(" + опорный шаг без объекта", " + a reference step without the object") : QString()));
        line(tpl::tr("ПОВОРОТ: %1...%2 град., ось (%3; %4) мм%5", "ROTATION: %1...%2 deg., axis (%3; %4) mm%5")
                 .arg(sw.angles.first()).arg(sw.angles.last())
                 .arg(c.x(), 0, 'f', 2).arg(c.y(), 0, 'f', 2)
                 .arg(sw.pivotAtCenter ? tpl::tr(" — центр объекта", " — object center") : QString()));
    } else if (m_project.chunkCount() > 1) {
        const QVector<TaskChunk> ch = emitChunks(m_project);
        line(tpl::tr("Вид файла: КОНВЕЙЕР — %1, по %2 частот в каждом. Первое: %3.tpl, "
                     "последнее: %4.tpl; плюс %5_run.bat и %5_run.sh для прогона пачкой",
                     "File type: PIPELINE — %1, %2 frequencies each. First: %3.tpl, last: %4.tpl; "
                     "plus %5_run.bat and %5_run.sh to run as a batch")
                 .arg(tpl::plural(ch.size(), tpl::tr("задание", "task"), tpl::tr("задания", "tasks"),
                                  tpl::tr("заданий", "tasks")))
                 .arg(m_project.chunking.freqsPerTask)
                 .arg(ch.first().name, ch.last().name, m_project.name));
    } else if (needsMacro(m_project)) {
        line(tpl::tr("Вид файла: макро-версия (%1.tpl + %1_step.h) — частот больше одной. "
                     "Шаг описан ОДИН раз, строк вызова %2",
                     "File type: macro version (%1.tpl + %1_step.h) — more than one frequency. The "
                     "step is described ONCE, call lines %2")
                 .arg(m_project.name).arg(m_project.stepCount()));
    } else {
        line(tpl::tr("Вид файла: обычный, один #STEP — макрос при одной частоте "
                     "ничего не экономит",
                     "File type: ordinary, one #STEP — a macro saves nothing for a single frequency"));
    }
    line(QString());
    line(tpl::tr("ОБЛАСТЬ: X %1...%2 мм,  Y %3...%4 мм", "DOMAIN: X %1...%2 mm,  Y %3...%4 mm")
             .arg(m_project.box.xMin).arg(m_project.box.xMax)
             .arg(m_project.box.yMin).arg(m_project.box.yMax));
    line(tpl::tr("Шаг сетки: %1 мм    Время счёта: %2 нс", "Grid step: %1 mm    Run time: %2 ns")
             .arg(m_project.delta).arg(m_project.timeMax));
    line(QString());

    if (m_project.freqs.size() == 1) {
        line(tpl::tr("ЧАСТОТА: %1 ГГц", "FREQUENCY: %1 GHz").arg(m_project.freqs.first()));
    } else if (!m_project.freqs.isEmpty()) {
        line(tpl::tr("ЧАСТОТЫ: %1...%2 ГГц, %3 точек", "FREQUENCIES: %1...%2 GHz, %3 points")
                 .arg(m_project.freqs.first()).arg(m_project.freqs.last())
                 .arg(m_project.freqs.size()));
    }
    line(QString());

    line(tpl::tr("ГЕОМЕТРИЯ (%1):", "GEOMETRY (%1):").arg(m_project.shapes.size()));
    for (const Shape &sh : m_project.shapes) {
        QString what;
        if (sh.plasma.any())
            what = tpl::tr("плазма — %1", "plasma — %1").arg(plasmaFull(sh.plasma));
        else if (sh.epsProfile.readsFile())
            what = tpl::tr("распределение eps из файла %1.eps", "eps distribution from file %1.eps")
                       .arg(epsProfileFile(sh));
        else if (sh.hasEpsProfile())
            what = tpl::tr("eps по формуле: %1", "eps by formula: %1").arg(sh.epsProfile.formula);
        else if (sh.material == Material::Dielectric)
            what = tpl::tr("диэлектрик, eps = %1 (в файл уйдёт добавка %2)", "dielectric, eps = %1 (the file gets the increment %2)")
                       .arg(sh.eps).arg(sh.eps - 1.0);
        else
            what = tpl::tr("%1 (контур)", "%1 (contour)").arg(materialName(sh.material));

        QString kind;
        switch (sh.kind) {
        case ShapeKind::Rectangle: kind = tpl::tr("прямоугольник", "rectangle"); break;
        case ShapeKind::Circle:    kind = tpl::tr("круг/сектор", "circle/sector"); break;
        case ShapeKind::Polygon:   kind = tpl::tr("контур, вершин %1", "polygon, %1 vertices").arg(sh.points.size()); break;
        case ShapeKind::Polyline:  kind = tpl::tr("линия, точек %1", "line, %1 points").arg(sh.points.size()); break;
        }
        line(QStringLiteral("  [%1] %2 — %3, %4").arg(sh.id).arg(sh.name, kind, what));
        if (sh.kind == ShapeKind::Rectangle && !sh.hasUniformEdges()) {
            QStringList parts;
            static const char *names[] = {"низ", "право", "верх", "лево"};
            static const char *namesEn[] = {"bottom", "right", "top", "left"};
            for (int e = 0; e < 4; ++e) {
                parts << QStringLiteral("%1: %2").arg(tpl::tr(names[e], namesEn[e]),
                                                      sh.edges[size_t(e)].present
                                                          ? materialName(sh.edgeMaterial(e))
                                                          : tpl::tr("нет", "none"));
            }
            line(tpl::tr("       стороны — %1", "       sides — %1").arg(parts.join(QStringLiteral(", "))));
            line(tpl::tr("       (запишется отдельными блоками: одним RECT_STAT "
                         "разные стороны не выразить)",
                         "       (will be written as separate blocks: different sides cannot be "
                         "expressed by one RECT_STAT)"));
        }
    }
    line(QString());

    line(tpl::tr("ПОРТЫ (%1):", "PORTS (%1):").arg(m_project.ports.size()));
    for (const Port &p : m_project.ports) {
        QString side;
        switch (p.side) {
        case PortSide::Left: side = tpl::tr("левая", "left"); break;
        case PortSide::Right: side = tpl::tr("правая", "right"); break;
        case PortSide::Bottom: side = tpl::tr("нижняя", "bottom"); break;
        case PortSide::Top: side = tpl::tr("верхняя", "top"); break;
        }
        QString sig;
        if (!p.active) {
            sig = tpl::tr("пассивный — только принимает", "passive — receive only");
        } else if (p.signal.source == SignalSource::File) {
            sig = tpl::tr("АКТИВНЫЙ, падающая волна из файла %1.sig: %2 — %3", "ACTIVE, incident wave from file %1.sig: %2 — %3")
                      .arg(signalFileName(m_project, p), waveformName(p.signal.wave.kind),
                           waveshapeSummary(p.signal.wave));
        } else {
            switch (p.signal.shape) {
            case SignalShape::Continuous:
                sig = tpl::tr("АКТИВНЫЙ, непрерывный, амплитуда %1", "ACTIVE, continuous, amplitude %1").arg(p.signal.amplitude);
                break;
            case SignalShape::Gaussian:
                sig = tpl::tr("АКТИВНЫЙ, гауссов импульс, длительность %1 нс, центр %2 нс", "ACTIVE, Gaussian pulse, duration %1 ns, center %2 ns")
                          .arg(p.signal.duration).arg(p.signal.center);
                break;
            case SignalShape::Rectangular:
                sig = tpl::tr("АКТИВНЫЙ, прямоугольный импульс %1 нс", "ACTIVE, rectangular pulse %1 ns").arg(p.signal.duration);
                break;
            }
        }
        line(tpl::tr("  [%1] %2 — %3 граница, апертура %4...%5 мм", "  [%1] %2 — %3 boundary, aperture %4...%5 mm")
                 .arg(p.id).arg(p.name, side).arg(p.from).arg(p.to));
        line(QStringLiteral("       %1").arg(sig));
    }
    line(QString());

    line(tpl::tr("НА ВЫХОДЕ:", "OUTPUT:"));
    line(tpl::tr("  %1.t — сигналы на портах (TMCROS)", "  %1.t — port signals (TMCROS)").arg(m_project.output.signalFile));
    if (m_project.output.topology)
        line(tpl::tr("  %1.tt — топология (FieldView)", "  %1.tt — topology (FieldView)").arg(m_project.output.signalFile));
    if (m_project.output.fields != OutputSettings::Fields::Off)
        line(tpl::tr("  %1.ex / .ey / .hz — поля (FieldView)", "  %1.ex / .ey / .hz — fields (FieldView)")
                 .arg(m_project.output.signalFile));
    if (m_project.output.sMatrix)
        line(tpl::tr("  %1.s — матрица рассеяния (TMCGROUT)", "  %1.s — scattering matrix (TMCGROUT)")
                 .arg(m_project.output.sMatrixFile));
    if (m_project.output.directionalPattern) {
        const tpl::RamEstimate ram = tpl::ramEstimate(m_project);
        if (ram.patternTones > 1) {
            line(tpl::tr("  %1_<МГц>MHz.DAT — диаграмма направленности на каждой из %2 "
                         "частот (TMC_DN), памяти около %3",
                         "  %1_<MHz>MHz.DAT — radiation pattern at each of the %2 frequencies (TMC_DN), "
                         "about %3 of memory")
                     .arg(m_project.output.patternFile).arg(ram.patternTones)
                     .arg(tpl::formatBytes(ram.total())));
        } else {
            line(tpl::tr("  %1.AMP / .FAZ / .DAT — диаграмма направленности (TMC_DN)", "  %1.AMP / .FAZ / .DAT — radiation pattern (TMC_DN)")
                     .arg(m_project.output.patternFile));
        }
    }
    if (m_project.output.timingReport)
        line(tpl::tr("  %1.timing_*.txt — отчёт о времени расчёта (ключ /Time)", "  %1.timing_*.txt — run time report (/Time key)")
                 .arg(m_project.name));

    // Что положить рядом и чем запускать. Файл падающей волны — такая же часть
    // задания, как _step.h: без него ядро остановится на «can not open signal file».
    {
        const QMap<QString, QByteArray> sigs = signalCompanions(m_project);
        if (!sigs.isEmpty()) {
            line(QString());
            line(tpl::tr("РЯДОМ С ЗАДАНИЕМ (пишет программа):", "NEXT TO THE TASK (written by the program):"));
            for (auto it = sigs.constBegin(); it != sigs.constEnd(); ++it) {
                line(tpl::tr("  %1 — падающая волна, %2 КБ", "  %1 — incident wave, %2 KB")
                         .arg(it.key()).arg(it.value().size() / 1024));
            }
        }
    }
    line(QString());
    line(tpl::tr("ЗАПУСК:", "LAUNCH:"));
    if (m_project.chunkCount() > 1) {
        line(tpl::tr("  %1_run.bat        (Windows, вся пачка по очереди)", "  %1_run.bat        (Windows, the whole batch in turn)")
                 .arg(m_project.name));
        line(tpl::tr("  sh %1_run.sh      (Linux и macOS)", "  sh %1_run.sh      (Linux and macOS)").arg(m_project.name));
    } else {
        line(QStringLiteral("  %1").arg(kernelRunCommand(m_project.kernel, m_project.name,
                                                         runFlags(m_project))));
    }

    if (!d.warnings.isEmpty()) {
        line(QString());
        line(tpl::tr("НА ЧТО ОБРАТИТЬ ВНИМАНИЕ:", "WHAT TO PAY ATTENTION TO:"));
        for (const QString &w : d.warnings)
            line(QStringLiteral("  - %1").arg(w));
    }
    m_summary->setPlainText(s);

    if (m_preview) {
        if (!d.ok()) {
            m_preview->setPlainText(tpl::tr("Исправьте ошибки — тогда покажу файл.", "Fix the errors — then the file will be shown."));
        } else if (needsMacro(m_project)) {
            // Показываем ОБА файла. Раньше в предпросмотре был только .tpl, и о
            // существовании второго файла пользователь узнавал уже после записи —
            // а без него задание не считается вовсе.
            const MacroFiles f = emitMacro(m_project);
            m_preview->setPlainText(
                tpl::tr("! ######## ФАЙЛ 1 из 2: %1.tpl ########\n\n%2\n\n"
                        "! ######## ФАЙЛ 2 из 2: %3 ########\n"
                        "! Должен лежать РЯДОМ с .tpl — ядро ищет его только в папке\n"
                        "! задания. Запускать ядро надо с путём до .tpl.\n\n%4",
                        "! ######## FILE 1 of 2: %1.tpl ########\n\n%2\n\n! ######## FILE 2 of 2: %3 "
                        "########\n! Must lie NEXT TO the .tpl — the kernel looks for it only in "
                        "the\n! task folder. Run the kernel with the path to the .tpl.\n\n%4")
                    .arg(m_project.name, f.tpl, f.stepHName, f.stepH));
        } else {
            m_preview->setPlainText(emitMono(m_project));
        }
    }
}

// ------------------------------------------------------------- отмена/возврат

void MainWindow::onImport3D(const QString &givenPath)
{
    QString path = givenPath;
    if (path.isEmpty()) {
        path = QFileDialog::getOpenFileName(this, tpl::tr("3D-модель для среза", "3D model for the section"), QString(),
                                            tpl::meshFileFilter());
        if (path.isEmpty())
            return;
    }
    Import3DDialog dlg(m_project, this);
    QString err;
    if (!dlg.loadFile(path, &err)) {
        QMessageBox::warning(this, tpl::tr("Модель не прочитана", "Model not read"), err);
        return;
    }
    if (dlg.exec() != QDialog::Accepted)
        return;
    insertImport(dlg.result(), dlg.boxChanged() ? std::optional<tpl::Box>(dlg.resultBox())
                                                : std::nullopt,
                 dlg.provenance());
}

void MainWindow::insertImport(const tpl::ImportShapes &res, std::optional<tpl::Box> box,
                              const QString &prov)
{
    if (res.shapes.isEmpty())
        return;
    // Одна правка — одна отмена: Ctrl+Z убирает весь импорт целиком.
    snapshot();
    if (box)
        m_project.box = *box;
    QVector<int> ids;
    for (const Shape &s : res.shapes) {
        m_project.shapes << s;
        ids << s.id;
    }
    if (!prov.isEmpty() && !m_project.description.contains(prov)) {
        m_project.description = m_project.description.isEmpty()
                                    ? prov
                                    : m_project.description + QStringLiteral(". ") + prov;
    }
    pushToForm();
    m_view->setProject(&m_project);
    refreshAll();
    m_view->setSelectedIds(ids);
    m_view->zoomToFit();
}

void MainWindow::snapshot()
{
    m_undo << m_project;
    if (m_undo.size() > kUndoDepth)
        m_undo.removeFirst();
    m_redo.clear();
    updateHistoryButtons();
}

void MainWindow::undo()
{
    if (m_undo.isEmpty())
        return;
    m_redo << m_project;
    m_project = m_undo.takeLast();
    pushToForm();
    m_view->setProject(&m_project);
    m_panel->setProject(&m_project);
    m_plan->setProject(&m_project);
    refreshAll();
}

void MainWindow::redo()
{
    if (m_redo.isEmpty())
        return;
    m_undo << m_project;
    m_project = m_redo.takeLast();
    pushToForm();
    m_view->setProject(&m_project);
    m_panel->setProject(&m_project);
    m_plan->setProject(&m_project);
    refreshAll();
}

void MainWindow::updateHistoryButtons()
{
    if (m_undoAct)
        m_undoAct->setEnabled(!m_undo.isEmpty());
    if (m_redoAct)
        m_redoAct->setEnabled(!m_redo.isEmpty());
}

// -------------------------------------------------------------------- команды

void MainWindow::onSectionChanged(int index)
{
    m_stack->setCurrentIndex(index);
    m_stepLabel->setText(QStringLiteral("%1. %2")
                             .arg(index + 1)
                             .arg(sectionName(index)));
    m_back->setEnabled(index > 0);
    m_next->setEnabled(index < kSectionCount - 1);
    if (index == 1) {
        m_view->zoomToFit();
        m_view->setFocus();
    }
    if (index == 2)
        m_plan->refresh(); // геометрия могла измениться на соседней странице
    if (index == kSectionCount - 1)
        refreshSummary();
}

void MainWindow::onNext()
{
    const int i = m_tree->currentIndex().row();
    if (i < kSectionCount - 1)
        m_tree->setCurrentItem(m_tree->topLevelItem(i + 1));
}

void MainWindow::onBack()
{
    const int i = m_tree->currentIndex().row();
    if (i > 0)
        m_tree->setCurrentItem(m_tree->topLevelItem(i - 1));
}

void MainWindow::onEditRequested(int id)
{
    if (id < 0)
        return;

    for (int i = 0; i < m_project.shapes.size(); ++i) {
        if (m_project.shapes.at(i).id != id)
            continue;
        ShapeDialog dlg(m_project.shapes.at(i), m_project.kernel, this);
        if (dlg.exec() == QDialog::Accepted) {
            snapshot();
            m_project.shapes[i] = dlg.shape();
            refreshAll();
        }
        return;
    }
    for (int i = 0; i < m_project.ports.size(); ++i) {
        if (m_project.ports.at(i).id != id)
            continue;
        PortDialog dlg(m_project.ports.at(i), m_project, this);
        if (dlg.exec() == QDialog::Accepted) {
            snapshot();
            m_project.ports[i] = dlg.port();

            // 🔑 Частоту согласования ставим САМИ.
            //
            // При сигнале из файла `FREQ` — это не частота расчёта (полосу задаёт
            // файл), а частота, на которой ядро согласует порт. Уехав от неё,
            // согласование портится молча: задание считается, а матрица выходит не
            // та. Оставлять это на память человеку — тот самый провал, от которого
            // существует программа, поэтому ставим значение и ГОВОРИМ об этом.
            const double want = dlg.wantedFreq();
            if (want > 0.0 && m_project.freqs.size() <= 1
                && (m_project.freqs.isEmpty()
                    || std::fabs(m_project.freqs.first() - want) > 1e-9)) {
                m_project.freqs = {want};
                m_freqOne->setChecked(true);
                m_freqSingle->setValue(want);
                m_hint->setText(tpl::tr("Частота задания поставлена в %1 ГГц — на ней "
                                        "ядро согласует этот порт.",
                                        "The task frequency is set to %1 GHz — the kernel matches this port at it.")
                                    .arg(want, 0, 'f', 4));
            }
            refreshAll();
        }
        return;
    }
}

void MainWindow::onGenerate()
{
    const Diagnostics d = validate(m_project);
    if (!d.ok()) {
        IssueDialog::report(this, tpl::tr("Нельзя создать файл", "Cannot create the file"),
                          tpl::tr("Ядро отвергло бы такое задание. Сначала исправьте:", "The kernel would reject such a task. Fix these first:"),
                          d.errors, {});
        return;
    }

    // Открывали файл — предлагаем ту же папку: «сохранить туда же» не должно
    // превращаться в поиск папки заново при каждой правке.
    const QString dir = QFileDialog::getExistingDirectory(
        this, tpl::tr("Куда положить задание", "Where to put the task"), m_sourceDir);
    if (dir.isEmpty())
        return;

    // Перезапись оригинала — решение пользователя, но принимать его вслепую он не должен.
    // Отключается в настройках: кто пересобирает одно и то же задание десятки раз,
    // отвечает на этот вопрос механически, а значит вопрос перестаёт работать.
    const QString target = QDir(dir).filePath(m_project.name + QStringLiteral(".tpl"));
    if (QFileInfo::exists(target) && settings::values().confirmOverwrite) {
        const bool sameAsSource = !m_sourceName.isEmpty()
                                  && QFileInfo(dir) == QFileInfo(m_sourceDir)
                                  && m_project.name == m_sourceName;
        const QString what =
            sameAsSource
                ? tpl::tr("Это тот самый файл, который вы открыли. Он будет "
                          "перезаписан целиком:\n\n%1\n\nЧтобы сохранить рядом новый, "
                          "смените имя задания на вкладке «Расчёт».",
                          "This is the very file you opened. It will be overwritten "
                          "entirely:\n\n%1\n\nTo save a new one next to it, change the task name on the "
                          "«Calculation» page.")
                      .arg(target)
                : tpl::tr("Файл уже существует и будет перезаписан:\n\n%1", "The file already exists and will be overwritten:\n\n%1").arg(target);
        if (QMessageBox::warning(this, tpl::tr("Перезаписать файл?", "Overwrite the file?"), what,
                                 QMessageBox::Save | QMessageBox::Cancel, QMessageBox::Cancel)
            != QMessageBox::Save) {
            return;
        }
    }

    // Задание, пришедшее из библиотеки образцов, может нести сопутствующие файлы
    // (готовое распределение .eps): ядро ищет их в папке задания, поэтому они
    // ложатся рядом при КАЖДОЙ записи, а не только при первой.
    const WriteResult wr = writeTask(dir, m_project, m_companions);
    if (!wr.ok) {
        QMessageBox::critical(this, tpl::tr("Не удалось записать", "Could not write"), wr.error);
        return;
    }
    const QStringList &written = wr.written;
    const bool lossy = wr.lossy;

    QString msg = tpl::tr("Записано в %1:\n\n  %2\n", "Written to %1:\n\n  %2\n")
                      .arg(dir, written.join(QStringLiteral("\n  ")));
    if (m_project.output.sMatrix) {
        msg += tpl::tr("\nПеред прогоном удалите %1.s — ядро его ДОПИШЕТ, а не "
                       "перезапишет, и старые данные подмешаются к новым.\n",
                       "\nBefore the run delete %1.s — the kernel will APPEND to it, not overwrite "
                       "it, and old data will mix with the new.\n")
                   .arg(m_project.output.sMatrixFile);
    }
    if (written.size() > 1) {
        // Ядро ищет включаемый файл ТОЛЬКО в папке .tpl: путь собирается как
        // «папка задания + имя» (DEFMIS.CPP:125), поиска по путям нет вовсе.
        // Разнести файлы по папкам или запустить ядро без пути — значит получить
        // нераскрытый макрос и падение вместо расчёта.
        msg += tpl::tr("\nОба файла должны лежать РЯДОМ: ядро ищет %1 только в папке "
                       "задания. Запускать с путём до .tpl.\n",
                       "\nBoth files must lie NEXT TO each other: the kernel looks for %1 only in the "
                       "task folder. Run with the path to the .tpl.\n")
                   .arg(written.value(1));
    }
    if (m_project.chunkCount() > 1) {
        msg += tpl::tr("\nЗапуск всей пачки: %1_run.bat (Windows) или "
                       "sh %1_run.sh (Linux, macOS). Задания независимы, так что "
                       "их можно считать и одновременно.",
                       "\nRun the whole batch: %1_run.bat (Windows) or sh %1_run.sh (Linux, macOS). "
                       "The tasks are independent, so they can also be run simultaneously.")
                   .arg(m_project.name);
    } else {
        msg += tpl::tr("\nЗапуск: %1", "\nLaunch: %1")
                   .arg(kernelRunCommand(m_project.kernel, m_project.name,
                                         runFlags(m_project)));
    }
    if (lossy) {
        msg += tpl::tr("\n\nЧасть символов описания заменена на '?': "
                       "в кодировке задания их нет.",
                       "\n\nSome characters of the description were replaced with '?': the task "
                       "encoding does not have them.");
    }
    // Дальше «сохранить туда же» означает уже эту папку и это имя.
    m_sourceDir = dir;
    m_sourceName = m_project.name;

    // Работа закончена — и выход из неё предлагаем сразу. Раньше окно просто
    // сообщало об успехе и оставляло пользователя на странице проверки, откуда
    // до стартового экрана надо было идти самому.
    //
    // Но уводить силой нельзя: после записи часто хотят подправить и сохранить
    // ещё раз (например, второй вариант геометрии). Поэтому спрашиваем.
    QMessageBox box(QMessageBox::Information, tpl::tr("Задание создано", "Task created"), msg,
                    QMessageBox::NoButton, this);
    QPushButton *toStart = box.addButton(tpl::tr("В начало", "Home"), QMessageBox::AcceptRole);
    box.addButton(tpl::tr("Остаться в задании", "Stay in the task"), QMessageBox::RejectRole);
    box.setDefaultButton(toStart);
    box.exec();

    if (box.clickedButton() == toStart)
        emit backToStartRequested();
}
