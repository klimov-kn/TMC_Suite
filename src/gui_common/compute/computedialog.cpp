// computedialog.cpp — окно «Compute»: видеокарты, проверки, выбор расчёта.

/// @file computedialog.cpp
/// @brief Окно «Compute»: построение, фоновая проверка карт, таблица, подробности, сохранение выбора.
///
/// Часть модуля «Расчёт на видеокарте». Описание методов — в `computedialog.h`.
/// @see https://www.tamic.ru/docs/code/gpu.html#compute-dialog

#include "computedialog.h"
#include "i18n/tmci18n.h"

#include <QBuffer>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrentRun>

#include <algorithm>

using namespace tmccompute;

namespace {

// Цвета проекта (те же, что theme.h и сайт).
/// Цвет фона панелей.
const char *kSurface = "#131a28";
/// Цвет рамок и разделителя.
const char *kLine    = "#243049";
/// Цвет основного текста.
const char *kText    = "#e7eef7";
/// Цвет второстепенного текста.
const char *kDim     = "#9fb0c8";
/// Цвет приглушённого текста и отметки «не проводилась».
const char *kMute    = "#6c7d97";
/// Цвет акцента и отметки «идёт».
const char *kAccent  = "#34e3c4";
/// Цвет отметки «пройдено».
const char *kOk      = "#5ce08a";
/// Цвет ошибки и отметки «не пройдено».
const char *kDanger  = "#ff6b6b";

// Таблица стилей только этого окна: шрифты и собственные панели.
/// @brief Возвращает таблицу стилей окна (шрифты, панели диагноза и подробностей, полоса хода).
/// @return Текст таблицы стилей Qt.
QString dialogStyle()
{
    return QStringLiteral(R"(
ComputeDialog, ComputeDialog QWidget {
    font-family: 'Inter', 'Segoe UI', 'DejaVu Sans', sans-serif;
    font-size: 9pt;
}
QLabel#tmcComputeTitle { font-size: 12pt; font-weight: 700; color: %1; }
QLabel#tmcComputeStatus { color: %2; }
QLabel#tmcComputeNote, QLabel#tmcComputeLast { color: %2; }
QLabel#tmcComputeDiag {
    background: rgba(255, 107, 107, 0.08);
    border: 1px solid %4;
    border-radius: 10px;
    padding: 8px 10px;
}
QLabel#tmcComputeDetails {
    background: %3;
    border: 1px solid %5;
    border-radius: 10px;
    padding: 8px 10px;
}
QScrollArea#tmcComputeDetailsScroll, QScrollArea#tmcComputeDetailsScroll > QWidget > QWidget {
    background: transparent;
}
QProgressBar#tmcComputeBusy {
    background: %3;
    border: none;
    border-radius: 2px;
    max-height: 4px;
    min-height: 4px;
}
QProgressBar#tmcComputeBusy::chunk { background: %6; border-radius: 2px; }
QTreeWidget#tmcComputeTable { font-size: 9pt; }
)")
        .arg(QLatin1String(kText), QLatin1String(kDim), QLatin1String(kSurface),
             QLatin1String(kDanger), QLatin1String(kLine), QLatin1String(kAccent));
}

/// @brief Возвращает моноширинный шрифт 8,5 пт для столбцов памяти и драйвера.
/// @return Шрифт.
QFont monoFont()
{
    QFont f;
    f.setFamilies({ QStringLiteral("JetBrains Mono"), QStringLiteral("Cascadia Mono"),
                    QStringLiteral("Consolas"), QStringLiteral("DejaVu Sans Mono") });
    f.setStyleHint(QFont::Monospace);
    f.setPointSizeF(8.5);
    return f;
}

/// @brief Вид отметки проверки: `Ok` — галочка, `Fail` — крестик, `Pending` — точка (идёт), `None` — черта (не проводилась).
enum class Mark { Ok, Fail, Pending, None };

// Отметка проверки: кружок с галочкой / крестиком, нарисованный сам —
// не зависит от того, есть ли нужный символ в шрифте.
/// @brief Рисует значок отметки 16 x 16 (с двойной плотностью точек).
/// @param[in] m  вид отметки
/// @return Значок.
QIcon markIcon(Mark m)
{
    const qreal dpr = 2.0;
    QPixmap pm(int(16 * dpr), int(16 * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    QColor c;
    switch (m) {
    case Mark::Ok:      c = QColor(QLatin1String(kOk)); break;
    case Mark::Fail:    c = QColor(QLatin1String(kDanger)); break;
    case Mark::Pending: c = QColor(QLatin1String(kAccent)); break;
    case Mark::None:    c = QColor(QLatin1String(kMute)); break;
    }
    QColor fill = c;
    fill.setAlphaF(m == Mark::None ? 0.0 : 0.18);
    p.setPen(QPen(c, 1.3));
    p.setBrush(fill);
    p.drawEllipse(QRectF(1.5, 1.5, 13, 13));
    QPen pen(c, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    if (m == Mark::Ok) {
        QPainterPath path;
        path.moveTo(4.8, 8.2);
        path.lineTo(7.0, 10.4);
        path.lineTo(11.2, 5.8);
        p.drawPath(path);
    } else if (m == Mark::Fail) {
        p.drawLine(QPointF(5.6, 5.6), QPointF(10.4, 10.4));
        p.drawLine(QPointF(10.4, 5.6), QPointF(5.6, 10.4));
    } else if (m == Mark::Pending) {
        p.setPen(Qt::NoPen);
        p.setBrush(c);
        p.drawEllipse(QRectF(6, 6, 4, 4));
    } else {
        p.drawLine(QPointF(5.5, 8), QPointF(10.5, 8));
    }
    return QIcon(pm);
}

// Та же отметка картинкой для текста с разметкой (data: URL). Символы ✓/✗ из
// шрифта не годятся: Windows подставляет цветной шрифт эмодзи, и цвет теряется.
/// @brief Возвращает отметку в виде тега `<img>` с PNG в data:-адресе (кэшируется).
/// @param[in] m  вид отметки
/// @return Фрагмент HTML.
QString markImg(Mark m)
{
    static QString cache[4];
    QString &s = cache[int(m)];
    if (s.isEmpty()) {
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        markIcon(m).pixmap(QSize(16, 16), 2.0).save(&buf, "PNG");
        s = QStringLiteral("<img src=\"data:image/png;base64,%1\" width=\"14\" height=\"14\" style=\"vertical-align:middle\">")
                .arg(QString::fromLatin1(png.toBase64()));
    }
    return s;
}

// Строка проверки для панели подробностей.
/// @brief Строит строку панели подробностей: отметка и текст проверки.
/// @param[in] ok    проверка пройдена
/// @param[in] text  текст (HTML)
/// @return Фрагмент HTML с переводом строки.
QString checkLine(bool ok, const QString &text)
{
    return markImg(ok ? Mark::Ok : Mark::Fail) + QStringLiteral("&nbsp;&nbsp;%1<br>").arg(text);
}

/// @brief Экранирует текст для вставки в HTML.
/// @param[in] s  текст
/// @return Экранированный текст.
QString esc(const QString &s) { return s.toHtmlEscaped(); }

/// @brief Столбцы таблицы карт: имя, память, двойная точность, самотест, драйвер; `ColCount` — их число.
enum Column { ColName, ColMemory, ColDouble, ColTest, ColDriver, ColCount };

} // namespace

// ---------------------------------------------------------------------------
ComputeDialog::ComputeDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("ComputeDialog"));
    setWindowTitle(QStringLiteral("Compute"));
    setStyleSheet(dialogStyle());
    m_settings = loadSettings();

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 14, 16, 12);
    root->setSpacing(10);

    auto *title = new QLabel(QStringLiteral("Calculation on the video card"), this);
    title->setObjectName(QStringLiteral("tmcComputeTitle"));
    root->addWidget(title);

    // Строка хода проверки
    auto *statusRow = new QHBoxLayout;
    m_status = new QLabel(this);
    m_status->setObjectName(QStringLiteral("tmcComputeStatus"));
    m_status->setTextFormat(Qt::RichText);
    statusRow->addWidget(m_status, 1);
    root->addLayout(statusRow);
    m_busy = new QProgressBar(this);
    m_busy->setObjectName(QStringLiteral("tmcComputeBusy"));
    m_busy->setRange(0, 0);
    m_busy->setTextVisible(false);
    root->addWidget(m_busy);

    // Диагноз: чего не хватает и что установить
    m_diag = new QLabel(this);
    m_diag->setObjectName(QStringLiteral("tmcComputeDiag"));
    m_diag->setWordWrap(true);
    m_diag->setTextFormat(Qt::RichText);
    m_diag->setOpenExternalLinks(true);
    m_diag->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_diag->hide();
    root->addWidget(m_diag);

    // Таблица карт
    m_table = new QTreeWidget(this);
    m_table->setObjectName(QStringLiteral("tmcComputeTable"));
    m_table->setColumnCount(ColCount);
    m_table->setHeaderLabels({ QStringLiteral("Video card"), QStringLiteral("Memory"),
                               QStringLiteral("Double precision"), QStringLiteral("Self-test"),
                               QStringLiteral("Driver") });
    m_table->setRootIsDecorated(false);
    m_table->setItemsExpandable(false);
    m_table->setUniformRowHeights(true);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setIconSize(QSize(16, 16));
    m_table->header()->setStretchLastSection(false);
    m_table->header()->setSectionResizeMode(ColName, QHeaderView::Stretch);
    for (int c = ColMemory; c < ColCount; ++c)
        m_table->header()->setSectionResizeMode(c, QHeaderView::ResizeToContents);
    root->addWidget(m_table);   // высота — по числу карт (fillTable)

    // Подробности о выбранной карте
    m_details = new QLabel(this);
    m_details->setObjectName(QStringLiteral("tmcComputeDetails"));
    m_details->setWordWrap(true);
    m_details->setTextFormat(Qt::RichText);
    m_details->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_details->setMinimumHeight(120);
    m_details->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    // Журнал компиляции при поломке бывает длинным — панель прокручивается.
    auto *detailsScroll = new QScrollArea(this);
    detailsScroll->setObjectName(QStringLiteral("tmcComputeDetailsScroll"));
    detailsScroll->setFrameShape(QFrame::NoFrame);
    detailsScroll->setWidgetResizable(true);
    detailsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    detailsScroll->setWidget(m_details);
    root->addWidget(detailsScroll, 1);

    // Выбор
    m_useGpu = new QCheckBox(QStringLiteral("Use the video card for calculations"), this);
    root->addWidget(m_useGpu);

    auto *devRow = new QHBoxLayout;
    devRow->setContentsMargins(22, 0, 0, 0);
    m_deviceLabel = new QLabel(QStringLiteral("Video card:"), this);
    m_device = new QComboBox(this);
    m_device->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    devRow->addWidget(m_deviceLabel);
    devRow->addWidget(m_device);
    devRow->addStretch(1);
    root->addLayout(devRow);

    auto *note = new QLabel(QStringLiteral(
        "The program decides for each task by itself: small grids and tasks that do not fit "
        "into the video card memory are calculated on the processor. The self-test checks that "
        "the video card gives the same numbers as the processor, bit for bit. "
        "Command line keys /GPU and /CPU override this setting."), this);
    note->setObjectName(QStringLiteral("tmcComputeNote"));
    note->setWordWrap(true);
    root->addWidget(note);

    auto *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet(QStringLiteral("color: %1; background: %1; max-height: 1px; border: none;").arg(QLatin1String(kLine)));
    root->addWidget(sep);

    m_lastCheck = new QLabel(this);
    m_lastCheck->setObjectName(QStringLiteral("tmcComputeLast"));
    m_lastCheck->setTextFormat(Qt::RichText);
    m_lastCheck->setWordWrap(true);
    root->addWidget(m_lastCheck);

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_recheck = m_buttons->addButton(QStringLiteral("Check again"), QDialogButtonBox::ActionRole);
    m_buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    root->addWidget(m_buttons);

    connect(m_buttons, &QDialogButtonBox::accepted, this, &ComputeDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &ComputeDialog::reject);
    connect(m_recheck, &QPushButton::clicked, this, &ComputeDialog::startCheck);
    connect(m_useGpu, &QCheckBox::toggled, this, &ComputeDialog::updateControls);
    connect(m_device, &QComboBox::currentIndexChanged, this, &ComputeDialog::onDeviceComboChanged);
    connect(m_table, &QTreeWidget::currentItemChanged, this, &ComputeDialog::onCurrentRowChanged);
    connect(&m_invWatch, &QFutureWatcher<Inventory>::finished, this, &ComputeDialog::onInventoryReady);
    connect(&m_testWatch, &QFutureWatcher<QList<SelfTestResult>>::finished, this, &ComputeDialog::onTestsReady);

    m_useGpu->setChecked(m_settings.useGpu);
    updateLastCheckLabel();
    resize(680, 640);
    startCheck();
}

ComputeDialog::~ComputeDialog()
{
    // Фоновая проверка ничего не знает об окне и дорабатывает сама (доли секунды);
    // её результат просто никому не нужен.
}

// ---------------------------------------------------------------------------
void ComputeDialog::startCheck()
{
    if (m_checking)
        return;
    m_checking = true;
    m_testsDone = false;
    m_report = CheckReport();
    m_status->setText(tmci18n::tr("Поиск видеокарт…", "Looking for video cards…"));
    m_busy->show();
    m_diag->hide();
    m_table->clear();
    m_details->setText(QString());
    updateControls();
    m_invWatch.setFuture(QtConcurrent::run([] { return enumerateDevices(); }));
}

void ComputeDialog::onInventoryReady()
{
    m_report.inventory = m_invWatch.result();
    const Inventory &inv = m_report.inventory;
    for (int i = 0; i < inv.devices.size(); ++i) {
        SelfTestResult t;
        t.deviceName = inv.devices[i].name;
        t.driver = inv.devices[i].driver;
        m_report.tests.append(t);
    }
    fillTable();
    if (inv.state != OclState::Ok) {
        // Тестировать нечего — проверка окончена.
        m_diag->setText(inv.diagnosis);
        m_diag->show();
        onTestsReady();
        return;
    }
    m_status->setText(tmci18n::tr("Проверка %1…", "Testing %1…")
                          .arg(inv.usableCount() == 1 ? tmci18n::tr("видеокарты", "the video card")
                                                      : tmci18n::tr("видеокарт: %1", "%1 video cards").arg(inv.usableCount())));
    const QList<DeviceInfo> devs = inv.devices;
    m_testWatch.setFuture(QtConcurrent::run([devs] {
        QList<SelfTestResult> out;
        for (const DeviceInfo &d : devs) {
            if (d.fp64) {
                out.append(selfTest(d.gpuIndex, true));
            } else {
                SelfTestResult t;
                t.deviceName = d.name;
                t.driver = d.driver;
                t.error = tmci18n::tr("Нет двойной точности.", "No double precision.");
                out.append(t);
            }
        }
        return out;
    }));
}

void ComputeDialog::onTestsReady()
{
    if (m_report.inventory.state == OclState::Ok && m_testWatch.isFinished() && m_testWatch.future().resultCount() > 0)
        m_report.tests = m_testWatch.result();
    m_report.when = QDateTime::currentDateTime();
    m_checking = false;
    m_testsDone = true;
    m_busy->hide();

    const QList<int> ok = passedGpuIndices();
    if (ok.isEmpty()) {
        m_status->setText(tmci18n::tr("<span style=\"color:%1\">Ни одна видеокарта не подходит — "
                                         "расчёт пойдёт на процессоре.</span>",
                                         "<span style=\"color:%1\">No video card can be used — "
                                         "calculations will run on the processor.</span>")
                              .arg(QLatin1String(kDanger)));
        if (m_report.inventory.state == OclState::Ok) {
            // Карта есть, но самотест не прошёл — объясняем в подробностях; общий совет — здесь.
            m_diag->setText(tmci18n::tr("Самопроверка на видеокарте не пройдена. Обновите драйвер видеокарты "
                                           "с сайта производителя (<a href=\"https://www.nvidia.com/Download/index.aspx\">NVIDIA</a>, "
                                           "<a href=\"https://www.amd.com/en/support/download/drivers.html\">AMD</a>) "
                                           "и нажмите <b>Проверить снова</b>.",
                                           "The self-test on the video card failed. Update the graphics driver "
                                           "from the video card maker (<a href=\"https://www.nvidia.com/Download/index.aspx\">NVIDIA</a>, "
                                           "<a href=\"https://www.amd.com/en/support/download/drivers.html\">AMD</a>) "
                                           "and press <b>Check again</b>."));
            m_diag->show();
        }
    } else {
        m_status->setText(tmci18n::tr("<span style=\"color:%1\">Проверка пройдена: %2 к расчётам.</span>",
                                          "<span style=\"color:%1\">Check passed: %2 ready for calculations.</span>")
                              .arg(QLatin1String(kOk),
                                   ok.size() == 1 ? tmci18n::tr("видеокарта готова", "the video card is")
                                                  : tmci18n::tr("видеокарт готово: %1", "%1 video cards are").arg(ok.size())));
    }

    // Карту ещё не выбирали (или выбранной больше нет) — берём самую быструю по замеру:
    // встроенная графика (Intel UHD и т. п.) проверку проходит, но счёт не ускоряет.
    if (!ok.isEmpty() && (!m_settings.deviceSaved || !ok.contains(m_settings.device)))
        m_settings.device = fastestPassedGpu();
    fillTable();
    fillDeviceCombo();
    // Флажок показывает сохранённую настройку, но только если есть годная карта.
    m_useGpu->setChecked(!ok.isEmpty() && m_settings.useGpu);
    updateControls();
    emit checkFinished();
}

QList<int> ComputeDialog::passedGpuIndices() const
{
    QList<int> v;
    if (!m_testsDone)
        return v;
    for (const SelfTestResult &t : m_report.tests)
        if (t.ran && t.passed)
            v.append(t.gpuIndex);
    return v;
}

int ComputeDialog::fastestPassedGpu() const
{
    int best = -1;
    double bestNs = 0.0;
    for (const SelfTestResult &t : m_report.tests) {
        if (!(t.ran && t.passed))
            continue;
        const double ns = t.speedMeasured && t.gpuNsPerNode > 0.0 ? t.gpuNsPerNode : 1e30;
        if (best < 0 || ns < bestNs) {
            best = t.gpuIndex;
            bestNs = ns;
        }
    }
    return best;
}

// ---------------------------------------------------------------------------
void ComputeDialog::fillTable()
{
    const int keepRow = m_table->currentItem() ? m_table->indexOfTopLevelItem(m_table->currentItem()) : -1;
    m_table->clear();
    const Inventory &inv = m_report.inventory;
    const QFont mono = monoFont();
    int selectRow = -1;
    for (int i = 0; i < inv.devices.size(); ++i) {
        const DeviceInfo &d = inv.devices[i];
        const SelfTestResult &t = m_report.tests[i];
        auto *it = new QTreeWidgetItem(m_table);
        it->setText(ColName, d.name);
        it->setToolTip(ColName, d.platform);
        it->setText(ColMemory, formatBytes(d.globalMemBytes));
        it->setFont(ColMemory, mono);
        it->setText(ColDriver, d.driver);
        it->setFont(ColDriver, mono);
        it->setIcon(ColDouble, markIcon(d.fp64 ? Mark::Ok : Mark::Fail));
        it->setText(ColDouble, d.fp64 ? tmci18n::tr("Да", "Yes") : tmci18n::tr("Нет", "No"));

        Mark test = Mark::None, overall = Mark::None;
        QString testText = QStringLiteral("—");
        if (!d.fp64) {
            overall = Mark::Fail;
        } else if (!m_testsDone) {
            test = overall = Mark::Pending;
            testText = tmci18n::tr("Идёт…", "Running…");
        } else if (t.ran && t.passed) {
            test = overall = Mark::Ok;
            // Карта считает верно, но не быстрее процессора (встроенная графика).
            const bool slow = t.speedMeasured && t.cpuNsPerNode > 0.0 && t.gpuNsPerNode >= 0.8 * t.cpuNsPerNode;
            testText = slow ? tmci18n::tr("Пройдена (без ускорения)", "Passed (no speed-up)") : tmci18n::tr("Пройдена", "Passed");
        } else {
            test = overall = Mark::Fail;
            testText = tmci18n::tr("Не пройдена", "Failed");
        }
        it->setIcon(ColTest, markIcon(test));
        it->setText(ColTest, testText);
        it->setIcon(ColName, markIcon(overall));
        if (overall == Mark::Ok && selectRow < 0 && d.gpuIndex == m_settings.device)
            selectRow = i;
    }
    if (selectRow < 0)
        selectRow = keepRow >= 0 && keepRow < m_table->topLevelItemCount() ? keepRow : 0;
    if (m_table->topLevelItemCount() > 0)
        m_table->setCurrentItem(m_table->topLevelItem(selectRow));
    // Высота таблицы — по числу карт (не больше пяти строк), остальное место — подробностям.
    const int rows = std::clamp(m_table->topLevelItemCount(), 1, 5);
    const int rowH = m_table->topLevelItemCount() > 0 ? std::max(20, m_table->sizeHintForRow(0)) : 24;
    m_table->setFixedHeight(m_table->header()->sizeHint().height() + rows * rowH + 2 * m_table->frameWidth() + 8);
    updateDetails();
}

void ComputeDialog::fillDeviceCombo()
{
    const QSignalBlocker block(m_device);
    m_device->clear();
    for (const DeviceInfo &d : m_report.inventory.devices) {
        const SelfTestResult *t = m_report.testFor(d.gpuIndex);
        if (d.fp64 && t && t->passed)
            m_device->addItem(QStringLiteral("%1 (%2)").arg(d.name, formatBytes(d.globalMemBytes)), d.gpuIndex);
    }
    const int k = m_device->findData(m_settings.device);
    m_device->setCurrentIndex(k >= 0 ? k : 0);
}

void ComputeDialog::onDeviceComboChanged(int index)
{
    if (index < 0)
        return;
    const int gpu = m_device->itemData(index).toInt();
    m_settings.device = gpu;
    // Показать выбранную карту в таблице
    for (int i = 0; i < m_report.inventory.devices.size(); ++i)
        if (m_report.inventory.devices[i].gpuIndex == gpu) {
            m_table->setCurrentItem(m_table->topLevelItem(i));
            break;
        }
}

void ComputeDialog::onCurrentRowChanged()
{
    updateDetails();
    // Щелчок по годной карте выбирает её для расчёта
    const int row = m_table->currentItem() ? m_table->indexOfTopLevelItem(m_table->currentItem()) : -1;
    if (row < 0 || row >= m_report.inventory.devices.size())
        return;
    const int k = m_device->findData(m_report.inventory.devices[row].gpuIndex);
    if (k >= 0 && k != m_device->currentIndex())
        m_device->setCurrentIndex(k);
}

void ComputeDialog::updateDetails()
{
    const int row = m_table->currentItem() ? m_table->indexOfTopLevelItem(m_table->currentItem()) : -1;
    const Inventory &inv = m_report.inventory;
    if (row < 0 || row >= inv.devices.size()) {
        m_details->setText(m_checking ? QString()
                                      : QStringLiteral("<span style=\"color:%1\">No video card to show.</span>")
                                            .arg(QLatin1String(kMute)));
        return;
    }
    const DeviceInfo &d = inv.devices[row];
    const SelfTestResult &t = m_report.tests[row];
    QString h;
    h += QStringLiteral("<b style=\"color:%1\">%2</b><br>").arg(QLatin1String(kText), esc(d.name));
    h += QStringLiteral("<span style=\"color:%1\">%2 &middot; %3 &middot; %4 compute units &middot; %5 MHz<br>"
                        "Memory %6, largest single buffer %7</span><br><br>")
             .arg(QLatin1String(kDim), esc(d.platform), esc(d.openclVersion))
             .arg(d.computeUnits).arg(d.clockMHz)
             .arg(formatBytes(d.globalMemBytes), formatBytes(d.maxAllocBytes));
    h += checkLine(true, tmci18n::tr("Драйвер OpenCL найден", "OpenCL driver found"));
    h += checkLine(d.fp64, d.fp64 ? tmci18n::tr("Двойная точность поддерживается", "Double precision supported")
                                  : tmci18n::tr("Нет двойной точности — эту видеокарту использовать нельзя", "No double precision — this card cannot be used"));
    if (d.fp64) {
        if (!m_testsDone) {
            h += markImg(Mark::Pending) + tmci18n::tr("&nbsp;&nbsp;Идёт самопроверка…<br>", "&nbsp;&nbsp;Self-test is running…<br>");
        } else {
            h += checkLine(t.contextOk, tmci18n::tr("Видеокарта принимает работу (контекст OpenCL)", "The video card accepts work (OpenCL context)"));
            if (t.contextOk)
                h += checkLine(t.compileOk, tmci18n::tr("Драйвер собрал расчётную программу", "The driver compiled the calculation program"));
            if (t.compileOk)
                h += checkLine(t.bitExact, tmci18n::tr("Пробный расчёт совпадает с процессором бит в бит", "Test calculation matches the processor bit for bit"));
            if (!t.error.isEmpty())
                h += QStringLiteral("<span style=\"color:%1\">%2</span><br>").arg(QLatin1String(kDanger), esc(t.error));
            if (!t.buildLog.isEmpty())
                h += QStringLiteral("<span style=\"color:%1\">%2</span><br>")
                         .arg(QLatin1String(kMute), esc(t.buildLog.left(600)).replace(QLatin1Char('\n'), QStringLiteral("<br>")));
            if (t.speedMeasured) {
                h += tmci18n::tr("<br><span style=\"color:%1\">Пробная сетка %2×%2: видеокарта быстрее процессора "
                                    "(потоков: %5) примерно в <b style=\"color:%3\">×%4</b> раза; "
                                    "пропускная способность памяти %6 ГБ/с.</span>",
                                    "<br><span style=\"color:%1\">Test grid %2×%2: the video card is about "
                                    "<b style=\"color:%3\">×%4</b> faster than the processor (%5 threads); "
                                    "memory bandwidth %6 GB/s.</span>")
                         .arg(QLatin1String(kDim)).arg(t.speedGrid).arg(QLatin1String(kAccent))
                         .arg(t.cpuNsPerNode / t.gpuNsPerNode, 0, 'f', 1).arg(t.cpuThreads)
                         .arg(t.bandwidthGBs, 0, 'f', 0);
            }
        }
    }
    m_details->setText(h);
}

void ComputeDialog::updateLastCheckLabel()
{
    const Settings &s = m_settings;
    if (!s.checkedAt.isValid()) {
        m_lastCheck->setText(QStringLiteral("Saved: calculations on the %1. Settings file: %2")
                                 .arg(s.useGpu ? QStringLiteral("video card") : QStringLiteral("processor"),
                                      esc(QDir::toNativeSeparators(settingsFilePath()))));
        return;
    }
    m_lastCheck->setText(QStringLiteral("Saved: calculations on the %1. Last saved check %2: %3, driver %4 — %5")
                             .arg(s.useGpu && s.checkOk ? QStringLiteral("video card") : QStringLiteral("processor"),
                                  s.checkedAt.toString(QStringLiteral("yyyy-MM-dd HH:mm")),
                                  s.checkedName.isEmpty() ? QStringLiteral("no card") : esc(s.checkedName),
                                  s.checkedDriver.isEmpty() ? QStringLiteral("—") : esc(s.checkedDriver),
                                  s.checkOk ? QStringLiteral("<span style=\"color:%1\">passed</span>").arg(QLatin1String(kOk))
                                            : QStringLiteral("<span style=\"color:%1\">failed</span>").arg(QLatin1String(kDanger))));
}

void ComputeDialog::updateControls()
{
    const QList<int> ok = passedGpuIndices();
    const bool any = !m_checking && !ok.isEmpty();
    m_useGpu->setEnabled(any);
    const bool several = ok.size() > 1;
    m_deviceLabel->setVisible(several);
    m_device->setVisible(several);
    m_device->setEnabled(any && m_useGpu->isChecked());
    m_recheck->setEnabled(!m_checking);
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(!m_checking);
}

// ---------------------------------------------------------------------------
void ComputeDialog::accept()
{
    if (m_checking)
        return;
    const QList<int> ok = passedGpuIndices();
    Settings s = m_settings;
    s.useGpu = !ok.isEmpty() && m_useGpu->isChecked();
    if (!ok.isEmpty()) {
        const int sel = m_device->currentIndex() >= 0 ? m_device->currentData().toInt() : ok.first();
        s.device = ok.contains(sel) ? sel : ok.first();
    } else {
        // Годных карт нет: запоминаем итог по первой карте с double (если она есть).
        s.device = 0;
    }
    storeCheckResult(s, m_report, s.device);
    saveSettings(s);
    m_settings = s;
    // Чтобы следующий расчёт в этом процессе пошёл по новой настройке.
    tmcApplyComputeSettings(QCoreApplication::arguments());
    QDialog::accept();
}
