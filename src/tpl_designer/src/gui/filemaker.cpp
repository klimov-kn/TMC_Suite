/// @file filemaker.cpp
/// @brief Реализация мастерской вспомогательных файлов `FileMakerDialog` и карты eps `EpsMapView`.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-files
#include "filemaker.h"

#include "cp1251.h"
#include "epsfile.h"
#include "expr.h"
#include "sigfile.h"
#include "signalgallery.h"
#include "signalpreview.h"
#include "dtheme.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QTextStream>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include "lang.h"

using namespace tpl;

namespace {

/// @brief Создаёт числовое поле с диапазоном, единицей и точностью.
/// @param[in] value     начальное значение.
/// @param[in] lo        нижняя граница.
/// @param[in] hi        верхняя граница.
/// @param[in] suffix    подпись единиц после числа (например, « нс»).
/// @param[in] decimals  знаков после запятой.
/// @param[in] step      шаг стрелок.
/// @return Новый `QDoubleSpinBox` без родителя.
QDoubleSpinBox *spin(double value, double lo, double hi, const QString &suffix, int decimals = 3,
                     double step = 0.1)
{
    auto *s = new QDoubleSpinBox;
    s->setRange(lo, hi);
    s->setDecimals(decimals);
    s->setSingleStep(step);
    s->setSuffix(suffix);
    s->setValue(value);
    return s;
}

/// @brief Форматирует длину в миллиметрах для сводок.
/// @param[in] v  длина, мм.
/// @return Число (6 значащих цифр) с подписью « мм».
QString mmText(double v)
{
    return QString::number(v, 'g', 6) + tpl::tr(" мм", " mm");
}

/// Размер файла человеческими словами: «1.2 МБ» вместо 1258291.
/// @brief Форматирует размер файла.
/// @param[in] bytes  размер, байт.
/// @return «N Б», «N.N КБ» или «N.N МБ».
QString sizeText(qint64 bytes)
{
    if (bytes < 1024)
        return tpl::tr("%1 Б", "%1 B").arg(bytes);
    if (bytes < 1024 * 1024)
        return tpl::tr("%1 КБ", "%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    return tpl::tr("%1 МБ", "%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
}

} // namespace

// ============================================================ карта eps

EpsMapView::EpsMapView(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(240);
}

void EpsMapView::setField(const EpsGrid &grid, const QVector<double> &values)
{
    m_grid = grid;
    m_values = values;
    m_lo = 0.0;
    m_hi = 0.0;
    if (!values.isEmpty()) {
        // Веса узлов для КАРТИНКИ не нужны: на краю они занижают значение вдвое, и
        // ровная среда выглядела бы рамкой. В файл веса, конечно, идут.
        m_lo = m_hi = values.first();
        for (double v : values) {
            m_lo = std::min(m_lo, v);
            m_hi = std::max(m_hi, v);
        }
    }
    update();
}

void EpsMapView::clear()
{
    m_values.clear();
    update();
}

void EpsMapView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.fillRect(rect(), QColor(0x0d, 0x11, 0x1a));

    if (m_values.isEmpty() || !m_grid.valid()) {
        p.setPen(dtheme::textMute());
        p.drawText(rect(), Qt::AlignCenter,
                   tpl::tr("Введите формулу — здесь появится карта среды", "Enter a formula — the medium map will appear here"));
        return;
    }

    QImage img(m_grid.nX, m_grid.nY, QImage::Format_RGB32);
    const double span = (m_hi - m_lo) > 1e-12 ? (m_hi - m_lo) : 1.0;
    for (int i = 0; i < m_grid.nY; ++i) {
        for (int j = 0; j < m_grid.nX; ++j) {
            const double t = (m_values.at(i * m_grid.nX + j) - m_lo) / span;
            // Та же шкала, что у профиля на холсте: гуще среда — теплее цвет.
            const int r = int(std::clamp(40 + 215 * t, 0.0, 255.0));
            const int g = int(std::clamp(60 + 140 * t * (1.0 - 0.4 * t), 0.0, 255.0));
            const int b = int(std::clamp(120 * (1.0 - t) + 30, 0.0, 255.0));
            // Ось Y сцены смотрит вверх, строки изображения — вниз.
            img.setPixel(j, m_grid.nY - 1 - i, qRgb(r, g, b));
        }
    }

    const double k = std::min(double(width() - 2) / m_grid.nX, double(height() - 2) / m_grid.nY);
    const QSize sz(std::max(1, int(m_grid.nX * k)), std::max(1, int(m_grid.nY * k)));
    const QPoint at((width() - sz.width()) / 2, (height() - sz.height()) / 2);
    p.drawImage(QRect(at, sz), img);
    p.setPen(QColor(0x28, 0x34, 0x48));
    p.drawRect(QRect(at, sz));
}

// ============================================================ окно

FileMakerDialog::FileMakerDialog(const Project &p, QWidget *parent)
    : QDialog(parent), m_source(p)
{
    setWindowTitle(tpl::tr("Вспомогательные файлы", "Auxiliary files"));
    setMinimumSize(880, 660);
    setSizeGripEnabled(true);

    m_tabs = new QTabWidget;
    m_tabs->addTab(buildSignalTab(), tpl::tr("Файл сигнала (.sig)", "Signal file (.sig)"));
    m_tabs->addTab(buildEpsTab(), tpl::tr("Файл распределения (.eps)", "Distribution file (.eps)"));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    buttons->button(QDialogButtonBox::Close)->setText(tpl::tr("Закрыть", "Close"));
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *root = new QVBoxLayout(this);
    root->addWidget(m_tabs, 1);
    root->addWidget(buttons);

    refreshSignal();
    refreshEps();
    dtheme::fitToScreen(this);
}


// ------------------------------------------------------------ вкладка сигнала

QWidget *FileMakerDialog::buildSignalTab()
{
    auto *page = new QWidget;
    auto *content = new QWidget;
    auto *root = new QVBoxLayout(content);
    root->setContentsMargins(12, 12, 12, 12);

    auto *why = new QLabel(tpl::tr("Файл .sig задаёт падающую волну целиком: ядро берёт из него a(t) и между "
        "точками интерполирует линейно. Так задаются формы, которых в языке нет — "
        "гребёнка тонов на всю полосу за один проход, ЛЧМ, пила, видеоимпульс.",
        "A .sig file defines the whole incident wave: the kernel takes a(t) from it "
        "and interpolates linearly between points. This is how you set waveforms the "
        "language lacks — a comb of tones over the whole band in one pass, a chirp, a "
        "sawtooth, a video pulse."));
    why->setProperty("mute", true);
    why->setWordWrap(true);
    root->addWidget(why);

    // Открыть готовый файл и править его — такой же законный путь, как сделать с нуля.
    auto *openRow = new QHBoxLayout;
    auto *openSig = new QPushButton(tpl::tr("Открыть готовый .sig…", "Open existing .sig…"));
    openSig->setObjectName(QStringLiteral("openSig"));
    openSig->setToolTip(tpl::tr("Прочитать файл обратно: если его писала эта программа, вернётся сама форма со "
        "всеми параметрами; у чужого файла точки лягут в таблицу.",
        "Read a file back: if this program wrote it, the waveform itself returns with "
        "all its parameters; for a foreign file the points go into the table."));
    openRow->addWidget(openSig);
    openRow->addStretch();
    root->addLayout(openRow);

    m_gallery = new SignalGallery;
    root->addWidget(m_gallery);

    // Поля формы — в своём контейнере: у него есть имя, и руководство обводит
    // ровно эту область, а не «примерно там, где поля».
    auto *paramsBox = new QWidget;
    paramsBox->setObjectName(QStringLiteral("waveParams"));
    auto *form = new QFormLayout(paramsBox);
    form->setContentsMargins(0, 0, 0, 0);
    const auto row = [&](const QString &label, QWidget *field) {
        form->addRow(label, field);
        m_rowLabel.insert(field, form->labelForField(field));
    };

    m_amp = spin(1.0, -1e6, 1e6, QString(), 4);
    form->addRow(tpl::tr("Амплитуда (отн. ед.):", "Amplitude (rel. units):"), m_amp);
    m_start = spin(0.0, 0, 1e6, tpl::tr(" нс", " ns"));
    form->addRow(tpl::tr("Начало сигнала:", "Signal start:"), m_start);
    m_duration = spin(40.0, 0.0001, 1e6, tpl::tr(" нс", " ns"));
    m_duration->setObjectName(QStringLiteral("waveDuration"));
    row(tpl::tr("Длительность посылки:", "Burst duration:"), m_duration);
    m_f1 = spin(3.0, 0.0001, 100000, tpl::tr(" ГГц", " GHz"), 4);
    m_f1->setObjectName(QStringLiteral("waveF1"));
    row(tpl::tr("Нижняя частота:", "Lower frequency:"), m_f1);
    m_f2 = spin(4.8, 0.0001, 100000, tpl::tr(" ГГц", " GHz"), 4);
    m_f2->setObjectName(QStringLiteral("waveF2"));
    row(tpl::tr("Верхняя частота:", "Upper frequency:"), m_f2);
    m_tones = new QSpinBox;
    m_tones->setObjectName(QStringLiteral("waveTones"));
    m_tones->setRange(2, kMaxFreqs); // больше 99 тонов пакет не обработает
    m_tones->setValue(10);
    row(tpl::tr("Тонов в гребёнке:", "Tones in the comb:"), m_tones);
    m_periods = new QSpinBox;
    m_periods->setRange(1, 100000);
    m_periods->setValue(1);
    row(tpl::tr("Периодов в посылке:", "Periods in the burst:"), m_periods);
    m_width = spin(0.2, 0.0001, 1e6, tpl::tr(" нс", " ns"), 4, 0.05);
    row(tpl::tr("Длительность tau:", "Duration tau:"), m_width);
    m_rise = spin(0.05, 0.0, 1e6, tpl::tr(" нс", " ns"), 4, 0.01);
    row(tpl::tr("Фронт:", "Rise:"), m_rise);
    m_fall = spin(0.05, 0.0, 1e6, tpl::tr(" нс", " ns"), 4, 0.01);
    row(tpl::tr("Спад:", "Fall:"), m_fall);
    m_expr = new QLineEdit;
    m_expr->setPlaceholderText(tpl::tr("например exp(-t*t)*sin(2*3.14159265*4*t)", "e.g. exp(-t*t)*sin(2*3.14159265*4*t)"));
    m_expr->setToolTip(tpl::tr("Переменная одна: t в наносекундах. Понимаются + - * / ^ и функции abs, exp, "
        "log, log10, sqrt, sin, cos, tan, asin, acos, atan, sinh, cosh, tanh.",
        "A single variable: t in nanoseconds. Supported: + - * / ^ and the functions "
        "abs, exp, log, log10, sqrt, sin, cos, tan, asin, acos, atan, sinh, cosh, tanh."));
    row(tpl::tr("Формула a(t):", "Formula a(t):"), m_expr);

    root->addWidget(paramsBox);

    // Таблица точек — РЕДАКТИРУЕМАЯ прямо здесь.
    //
    // ⚠️ Раньше «своя таблица» означала «принесите готовый файл», и это замкнутый
    // круг: чтобы сделать файл сигнала, нужен был файл сигнала. Точки набираются
    // руками, берутся из любой встроенной формы кнопкой «Взять из формы» (дальше
    // правь что хочешь) или подгружаются из двух колонок. Файл при этом всё равно
    // пишет `tpl::sigFileText` — своего пути записи у окна нет.
    m_tableBox = new QGroupBox(tpl::tr("Точки сигнала: время (нс) и значение a", "Signal points: time (ns) and value a"));
    auto *tv = new QVBoxLayout(m_tableBox);
    m_table = new QTableWidget(0, 2);
    m_table->setHorizontalHeaderLabels({tpl::tr("t, нс", "t, ns"), QStringLiteral("a")});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setDefaultSectionSize(22);
    m_table->setMinimumHeight(220);
    m_table->setAlternatingRowColors(true);
    tv->addWidget(m_table);

    auto *tbtn = new QHBoxLayout;
    auto *addRow = new QPushButton(tpl::tr("Добавить строку", "Add row"));
    auto *delRow = new QPushButton(tpl::tr("Удалить строку", "Delete row"));
    m_tableFromShape = new QPushButton(tpl::tr("Взять из формы", "Take from waveform"));
    m_tableFromShape->setObjectName(QStringLiteral("tableFromShape"));
    m_tableFromShape->setToolTip(tpl::tr("Заполнить таблицу точками любой встроенной формы — и дальше править их руками. "
        "Так проще всего получить «почти гребёнку» или «почти импульс».",
        "Fill the table with the points of any built-in waveform — and then edit them "
        "by hand. This is the easiest way to get an “almost comb” or an “almost pulse”."));
    m_tableLoad = new QPushButton(tpl::tr("Загрузить из файла…", "Load from file…"));
    m_tableLoad->setToolTip(tpl::tr("Два столбца чисел: время и значение. Понимаются .sig, .txt, .csv и .dat; "
        "строки с «!» и «#» — комментарии.",
        "Two columns of numbers: time and value. Supported: .sig, .txt, .csv and .dat; "
        "lines starting with “!” and “#” are comments."));
    auto *clearRows = new QPushButton(tpl::tr("Очистить", "Clear"));
    for (QPushButton *b : {addRow, delRow, m_tableFromShape, m_tableLoad, clearRows})
        tbtn->addWidget(b);
    tbtn->addStretch();
    tv->addLayout(tbtn);

    m_tableInfo = new QLabel;
    m_tableInfo->setObjectName(QStringLiteral("tableInfo"));
    m_tableInfo->setProperty("mute", true);
    m_tableInfo->setWordWrap(true);
    tv->addWidget(m_tableInfo);
    root->addWidget(m_tableBox);

    connect(addRow, &QPushButton::clicked, this, [this] {
        const int at = m_table->currentRow() < 0 ? m_table->rowCount() : m_table->currentRow() + 1;
        m_table->insertRow(at);
        m_table->setItem(at, 0, new QTableWidgetItem(QStringLiteral("0")));
        m_table->setItem(at, 1, new QTableWidgetItem(QStringLiteral("0")));
        m_table->setCurrentCell(at, 0);
        pullTable();
        refreshSignal();
    });
    connect(delRow, &QPushButton::clicked, this, [this] {
        if (m_table->currentRow() >= 0)
            m_table->removeRow(m_table->currentRow());
        pullTable();
        refreshSignal();
    });
    connect(clearRows, &QPushButton::clicked, this, [this] {
        m_table->setRowCount(0);
        m_points.clear();
        refreshSignal();
    });
    connect(m_tableFromShape, &QPushButton::clicked, this,
            &FileMakerDialog::tableFromCurrentShape);
    connect(m_table, &QTableWidget::itemChanged, this, [this] {
        if (m_fillingTable)
            return;
        pullTable();
        refreshSignal();
    });

    // Шаг записи считается от ШАГА СЕТКИ задания: ядро интерполирует между точками,
    // и файл, написанный под чужой шаг, даст другую волну.
    auto *grid = new QGroupBox(tpl::tr("Под какое задание", "For which task"));
    auto *gf = new QFormLayout(grid);
    m_delta = spin(m_source.delta > 0.0 ? m_source.delta : 1.0, 0.0001, 10000.0,
                   tpl::tr(" мм", " mm"), 6, 0.1);
    m_delta->setToolTip(tpl::tr("Шаг сетки DELTA того задания, с которым файл пойдёт в расчёт. Точки пишутся "
        "вдвое чаще шага ядра DELTA/(c*sqrt(2)) — проверено прогоном.",
        "Grid step DELTA of the task the file will be computed with. Points are "
        "written twice as often as the kernel step DELTA/(c*sqrt(2)) — verified by a "
        "run."));
    m_delta->setObjectName(QStringLiteral("sigDelta"));
    gf->addRow(tpl::tr("Шаг сетки DELTA:", "Grid step DELTA:"), m_delta);
    m_time = spin(m_source.timeMax > 0.0 ? m_source.timeMax : 40.0, 0.001, 1e6,
                  tpl::tr(" нс", " ns"), 3, 1.0);
    m_time->setObjectName(QStringLiteral("sigTime"));
    gf->addRow(tpl::tr("Время счёта TIME:", "Run time TIME:"), m_time);
    m_sigName = new QLineEdit(QStringLiteral("signal"));
    m_sigName->setObjectName(QStringLiteral("sigName"));
    m_sigName->setToolTip(tpl::tr("Имя без расширения и без точки: расширение .sig "
                                  "ядро дописывает само, но только если точки в "
                                  "имени нет.",
                                  "Name without extension and without a dot: the kernel appends the .sig "
                                  "extension itself, but only if the name has no dot."));
    gf->addRow(tpl::tr("Имя файла:", "File name:"), m_sigName);
    root->addWidget(grid);

    auto *prev = new QGroupBox(tpl::tr("Что получится", "Result"));
    auto *pv = new QVBoxLayout(prev);
    m_preview = new SignalPreview;
    pv->addWidget(m_preview);
    m_sigSummary = new QLabel;
    m_sigSummary->setObjectName(QStringLiteral("sigSummary"));
    m_sigSummary->setProperty("mute", true);
    m_sigSummary->setWordWrap(true);
    m_sigSummary->setTextInteractionFlags(Qt::TextSelectableByMouse);
    pv->addWidget(m_sigSummary);
    m_sigWarn = new QLabel;
    m_sigWarn->setWordWrap(true);
    pv->addWidget(m_sigWarn);
    root->addWidget(prev);

    auto *save = new QPushButton(tpl::tr("Сохранить .sig…", "Save .sig…"));
    save->setProperty("accent", true);
    save->setMinimumHeight(34);
    save->setObjectName(QStringLiteral("saveSig"));
    root->addWidget(save);
    root->addStretch();

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(content);
    auto *pl = new QVBoxLayout(page);
    pl->setContentsMargins(0, 0, 0, 0);
    pl->addWidget(scroll);

    const auto touch = [this] { refreshSignal(); };
    connect(m_gallery, &SignalGallery::currentChanged, this, touch);
    for (QDoubleSpinBox *s : {m_amp, m_start, m_duration, m_f1, m_f2, m_width, m_rise, m_fall,
                              m_delta, m_time})
        connect(s, &QDoubleSpinBox::valueChanged, this, touch);
    connect(m_tones, &QSpinBox::valueChanged, this, touch);
    connect(m_periods, &QSpinBox::valueChanged, this, touch);
    connect(m_expr, &QLineEdit::textChanged, this, touch);
    connect(m_tableLoad, &QPushButton::clicked, this, &FileMakerDialog::loadTable);
    connect(openSig, &QPushButton::clicked, this, [this] { openSignalFile(); });
    connect(save, &QPushButton::clicked, this, &FileMakerDialog::saveSignal);
    return page;
}

Waveshape FileMakerDialog::waveshape() const
{
    Waveshape w;
    w.kind = m_gallery->current();
    w.amplitude = m_amp->value();
    w.start = m_start->value();
    w.duration = m_duration->value();
    w.f1 = m_f1->value();
    w.f2 = m_f2->value();
    w.tones = m_tones->value();
    w.periods = m_periods->value();
    w.width = m_width->value();
    w.rise = m_rise->value();
    w.fall = m_fall->value();
    w.expression = m_expr->text();
    w.points = m_points;
    w.tableTones = m_tableTones;
    return w;
}

/// Задание-носитель: в нём живёт один активный порт с нашей формой.
///
/// Так файл пишет ровно та же функция, что и мастер (`tpl::sigFileText`), — отдельного
/// пути записи у этого окна нет и быть не должно.
Project FileMakerDialog::signalProject() const
{
    Project p;
    p.name = m_sigName->text().trimmed().isEmpty() ? QStringLiteral("signal")
                                                   : m_sigName->text().trimmed();
    p.delta = m_delta->value();
    p.timeMax = m_time->value();
    p.box = {0.0, std::max(10.0, p.delta * 100), 0.0, std::max(10.0, p.delta * 40)};
    Port port;
    port.id = 1;
    port.name = tpl::tr("вход", "port");
    port.side = PortSide::Left;
    port.from = p.box.yMin;
    port.to = p.box.yMax;
    port.active = true;
    port.signal.source = SignalSource::File;
    port.signal.wave = waveshape();
    port.signal.file = p.name;
    p.ports << port;
    const double f = suggestedFreq(port.signal.wave);
    p.freqs = {f > 0.0 ? f : 1.0};
    return p;
}

void FileMakerDialog::refreshSignal()
{
    const Waveshape w = waveshape();
    const WaveFields f = waveFields(w.kind);
    const auto setRow = [&](QWidget *field, bool on) {
        field->setVisible(on);
        if (QWidget *lab = m_rowLabel.value(field))
            lab->setVisible(on);
    };
    setRow(m_duration, f.duration);
    setRow(m_f1, f.band || f.carrier || f.rate);
    setRow(m_f2, f.band);
    setRow(m_tones, f.tones);
    setRow(m_periods, f.periods);
    setRow(m_width, f.width);
    setRow(m_rise, f.edges);
    setRow(m_fall, f.edges);
    setRow(m_expr, f.expression);
    m_tableBox->setVisible(f.table);
    if (QWidget *lab = m_rowLabel.value(m_f1))
        if (auto *l = qobject_cast<QLabel *>(lab))
            l->setText(f.band      ? tpl::tr("Нижняя частота:", "Lower frequency:")
                       : f.carrier ? tpl::tr("Несущая:", "Carrier:")
                                   : tpl::tr("Частота повторения:", "Repetition rate:"));
    m_tableInfo->setText(
        m_points.size() >= 2
            ? tpl::tr("%1 точек, %2…%3 нс. Между точками ядро интерполирует линейно; "
                      "шаг записи файла считается отдельно, по шагу сетки.%4",
                      "%1 points, %2…%3 ns. The kernel interpolates linearly between points; the "
                      "file write step is computed separately, from the grid step.%4")
                  .arg(m_points.size())
                  .arg(m_points.first().x(), 0, 'g', 6).arg(m_points.last().x(), 0, 'g', 6)
                  .arg(m_tableTones.isEmpty()
                           ? QString()
                           : m_tableTones.size() > kMaxFreqs
                                 ? tpl::tr(" Тонов в строке #FREQ %1 — в файл пойдут "
                                           "первые %2.",
                                           " Tones in the #FREQ line: %1 — the first %2 go into the file.")
                                       .arg(m_tableTones.size()).arg(kMaxFreqs)
                                 : tpl::tr(" Тонов в строке #FREQ: %1.", " Tones in the #FREQ line: %1.")
                                       .arg(m_tableTones.size()))
            : tpl::tr("Точек нет. Наберите их кнопкой «Добавить строку», возьмите "
                      "готовую форму кнопкой «Взять из формы» или загрузите два "
                      "столбца чисел из файла.",
                      "No points. Type them in with the “Add row” button, take a ready waveform with "
                      "the “Take from waveform” button, or load two columns of numbers from a file."));

    const Project p = signalProject();
    m_preview->setPort(p.ports.first(), p);

    const SigPlan plan = sigPlan(p, p.ports.first());
    const Band band = waveBand(w);
    const double freq = suggestedFreq(w);
    QStringList bits;
    bits << tpl::tr("шаг записи %1 нс (шаг ядра %2 нс)", "write step %1 ns (kernel step %2 ns)")
                .arg(plan.dt, 0, 'g', 3).arg(plan.kernelDt, 0, 'g', 3);
    bits << tpl::tr("точек %1, примерно %2", "%1 points, about %2").arg(plan.points)
                .arg(sizeText(qint64(plan.points) * 24));
    if (band.known)
        bits << tpl::tr("полоса %1…%2 ГГц", "band %1…%2 GHz").arg(band.lo, 0, 'g', 4).arg(band.hi, 0, 'g', 4);
    if (freq > 0.0)
        bits << tpl::tr("FREQ согласования %1 ГГц", "matching FREQ %1 GHz").arg(freq, 0, 'g', 4);
    bits << tpl::tr("пик %1", "peak %1").arg(wavePeak(w), 0, 'g', 4);
    m_sigSummary->setText(bits.join(QStringLiteral(" · ")));

    QStringList warn;
    if (plan.cutByTime)
        warn << tpl::tr("Сигнал длиннее времени счёта: хвост обрежется.", "The signal is longer than the run time: the tail will be cut off.");
    if (plan.coarsened)
        warn << tpl::tr("Шаг записи укрупнён, чтобы файл не распух.", "The write step is coarsened so the file does not bloat.");
    if (w.kind == Waveform::Table && m_points.size() < 2)
        warn << tpl::tr("Для своей таблицы нужно загрузить точки.", "For a custom table you need to load points.");
    if (w.kind == Waveform::Expression && w.expression.trimmed().isEmpty())
        warn << tpl::tr("Введите формулу a(t).", "Enter the formula a(t).");
    m_sigWarn->setText(warn.join(QStringLiteral(" ")));
    m_sigWarn->setStyleSheet(warn.isEmpty() ? QString()
                                            : QStringLiteral("color:%1").arg(dtheme::warn().name()));
}

void FileMakerDialog::loadTable()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tpl::tr("Точки падающей волны", "Incident-wave points"), QString(),
        tpl::tr("Файлы сигнала (*.sig *.txt *.csv *.dat);;Все файлы (*)", "Signal files (*.sig *.txt *.csv *.dat);;All files (*)"));
    if (path.isEmpty())
        return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tpl::tr("Не читается", "Cannot read"), f.errorString());
        return;
    }
    const QString text = fromCp1251(f.readAll());
    const QVector<QPointF> pts = parseSigPoints(text);
    if (pts.size() < 2) {
        QMessageBox::warning(this, tpl::tr("Пусто", "Empty"),
                             tpl::tr("В файле должно быть не меньше двух строк из двух "
                                     "чисел: время и значение.",
                                     "The file must have at least two lines of two numbers: time and value."));
        return;
    }
    m_points = pts;
    m_tableTones = parseSigTones(text);
    m_gallery->setCurrent(Waveform::Table);
    pushTable();
    refreshSignal();
    warnToneCut();
}

/// Принесённый файл назвал тонов больше, чем берёт пакет, — сказать об этом сразу,
/// в момент загрузки. true — окно показано.
bool FileMakerDialog::warnToneCut()
{
    const QString cut = toneCutNotice(m_tableTones);
    if (cut.isEmpty())
        return false;
    QMessageBox::warning(this, tpl::tr("Тонов больше %1", "More than %1 tones").arg(kMaxFreqs), cut);
    return true;
}

/// Открыть готовый `.sig` и продолжить с него.
///
/// Файл, написанный этой программой, несёт служебную строку `@TPLD-SIG` — по ней
/// возвращается САМА ФОРМА со всеми параметрами, и дальше её правят полями, а не
/// числами. У чужого файла такой строки нет, и остаются только точки: они ложатся в
/// таблицу, где их можно править построчно.
void FileMakerDialog::openSignalFile(const QString &given)
{
    const QString path = !given.isEmpty() ? given : QFileDialog::getOpenFileName(
        this, tpl::tr("Открыть файл сигнала", "Open signal file"), QString(),
        tpl::tr("Файл сигнала (*.sig);;Текст с двумя колонками (*.txt *.csv *.dat);;"
                "Все файлы (*)",
                "Signal file (*.sig);;Two-column text (*.txt *.csv *.dat);;All files (*)"));
    if (path.isEmpty())
        return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tpl::tr("Не читается", "Cannot read"), f.errorString());
        return;
    }
    const QString text = fromCp1251(f.readAll());

    Waveshape w;
    bool gotShape = false;
    for (const QString &line : text.split(QLatin1Char('\n'))) {
        if (line.contains(QLatin1String(kSigMarker)) && parseSigMarker(line, &w)) {
            gotShape = true;
            break;
        }
        if (!line.trimmed().startsWith(QLatin1Char('!'))
            && !line.trimmed().startsWith(QLatin1Char('#')) && !line.trimmed().isEmpty())
            break; // пошли данные — служебной строки в файле нет
    }

    const QVector<QPointF> pts = parseSigPoints(text);
    m_sigName->setText(QFileInfo(path).completeBaseName());

    if (gotShape) {
        m_gallery->setCurrent(w.kind);
        m_amp->setValue(w.amplitude);
        m_start->setValue(w.start);
        m_duration->setValue(w.duration);
        m_f1->setValue(w.f1);
        m_f2->setValue(w.f2);
        m_tones->setValue(std::max(2, w.tones));
        m_periods->setValue(std::max(1, w.periods));
        m_width->setValue(w.width);
        m_rise->setValue(w.rise);
        m_fall->setValue(w.fall);
        m_expr->setText(w.expression);
        m_points = w.kind == Waveform::Table ? pts : QVector<QPointF>();
        m_tableTones = w.kind == Waveform::Table ? parseSigTones(text) : QVector<double>();
        pushTable();
        refreshSignal();
        QMessageBox::information(
            this, tpl::tr("Открыто", "Opened"),
            tpl::tr("Файл %1 написан этой программой: форма «%2» и все её параметры "
                    "восстановлены. Правьте полями и сохраняйте заново.",
                    "File %1 was written by this program: waveform “%2” and all its parameters are "
                    "restored. Edit them in the fields and save again.")
                .arg(QFileInfo(path).fileName(), waveformName(w.kind)));
        return;
    }

    if (pts.size() < 2) {
        QMessageBox::warning(this, tpl::tr("Нечего читать", "Nothing to read"),
                             tpl::tr("В файле должно быть не меньше двух строк из двух "
                                     "чисел: время и значение.",
                                     "The file must have at least two lines of two numbers: time and value."));
        return;
    }
    m_points = pts;
    m_tableTones = parseSigTones(text);
    m_gallery->setCurrent(Waveform::Table);
    pushTable();
    refreshSignal();
    if (warnToneCut())
        return; // про обрезку сказано; второе окно подряд — уже лишнее
    QMessageBox::information(
        this, tpl::tr("Открыто", "Opened"),
        tpl::tr("В файле %1 нет служебной строки — значит его писали не здесь. "
                "Прочитано %2 точек, они лежат в таблице и правятся построчно.",
                "File %1 has no service line — so it was not written here. %2 points read; "
                "they are in the table and can be edited row by row.")
            .arg(QFileInfo(path).fileName()).arg(pts.size()));
}

/// Таблица -> модель. Строка, в которой не два числа, просто пропускается: человек
/// набирает числа по одному, и ругаться на каждую недобранную строку нельзя.
void FileMakerDialog::pullTable()
{
    QVector<QPointF> pts;
    for (int r = 0; r < m_table->rowCount(); ++r) {
        QTableWidgetItem *a = m_table->item(r, 0);
        QTableWidgetItem *b = m_table->item(r, 1);
        if (!a || !b)
            continue;
        bool okT = false, okV = false;
        const double t = a->text().trimmed().replace(QLatin1Char(','), QLatin1Char('.'))
                             .toDouble(&okT);
        const double v = b->text().trimmed().replace(QLatin1Char(','), QLatin1Char('.'))
                             .toDouble(&okV);
        if (okT && okV)
            pts << QPointF(t, v);
    }
    // Ядро читает точки подряд и интерполирует между соседними: неупорядоченное
    // время дало бы ему ход назад. Сортируем молча — это не ошибка человека.
    std::sort(pts.begin(), pts.end(),
              [](const QPointF &a, const QPointF &b) { return a.x() < b.x(); });
    m_points = pts;
}

void FileMakerDialog::pushTable()
{
    m_fillingTable = true;
    m_table->setRowCount(m_points.size());
    for (int r = 0; r < m_points.size(); ++r) {
        m_table->setItem(r, 0, new QTableWidgetItem(
                                   QString::number(m_points.at(r).x(), 'g', 10)));
        m_table->setItem(r, 1, new QTableWidgetItem(
                                   QString::number(m_points.at(r).y(), 'g', 10)));
    }
    m_fillingTable = false;
}

/// Заполнить таблицу точками той формы, что сейчас выбрана в галерее.
///
/// Это главный способ начать: взять готовую гребёнку или импульс и дальше править
/// руками. Точек берём не больше тысячи — таблица на десятки тысяч строк
/// нередактируема, а для правки руками столько и не нужно; при записи файла шаг всё
/// равно считается заново по шагу сетки.
void FileMakerDialog::tableFromCurrentShape()
{
    const Waveform kind = m_gallery->current();
    if (kind == Waveform::Table) {
        QMessageBox::information(this, tpl::tr("Выберите форму", "Select a waveform"),
                                 tpl::tr("Нажмите в галерее любую форму кроме «Своей "
                                         "таблицы», потом «Взять из формы».",
                                         "Click any waveform in the gallery except “Custom table”, then “Take from "
                                         "waveform”."));
        return;
    }
    Waveshape w = waveshape();
    w.kind = kind;
    const TimeWindow sup = waveSupport(w);
    const ExprProgram prog = waveProgram(w);
    const double t0 = std::max(0.0, sup.tMin);
    const double t1 = std::max(t0 + 1e-6, sup.tMax);
    // Точек — двадцать на период ВЕРХНЕЙ частоты сигнала, но не меньше 400.
    //
    // ⚠️ Раньше точек всегда было 400, и гребёнка 1...100 ГГц на 20 нс шла в таблицу с
    // шагом 0.05 нс при периоде верхнего тона 0.01 нс: от сигнала оставалась каша, а
    // файл, записанный из такой таблицы, нёс уже другую волну. Потолок — 50 000 строк:
    // больше таблица не редактируется по-человечески, и об укрупнении сказано ниже.
    const Band band = waveBand(w);
    const double fTop = band.known ? std::max(band.lo, band.hi) : 0.0;
    int n = 400;
    if (fTop > 0.0)
        n = std::max(n, int(std::ceil((t1 - t0) * fTop * 20.0)));
    const int kMaxRows = 50000;
    const bool coarse = n > kMaxRows;
    n = std::min(n, kMaxRows);
    m_points.clear();
    m_points.reserve(n + 1);
    for (int i = 0; i <= n; ++i) {
        const double t = t0 + (t1 - t0) * i / n;
        m_points << QPointF(std::round(t * 1e6) / 1e6, waveValue(w, t, &prog));
    }
    // Гребёнка, переложенная в таблицу, — всё та же гребёнка: тоны идут с ней, иначе
    // постобработке пришлось бы искать их по спектру, а это неточно.
    m_tableTones = waveTones(w);
    m_gallery->setCurrent(Waveform::Table);
    pushTable();
    refreshSignal();
    if (coarse) {
        QMessageBox::information(
            this, tpl::tr("Таблица укрупнена", "Table coarsened"),
            tpl::tr("Чтобы передать форму точно, нужно было бы %1 строк — таблица "
                    "столько не вмещает, взято %2. Для верхних частот такой шаг "
                    "грубоват: если таблицу править не нужно, сохраните файл прямо из "
                    "формы — там шаг считается по шагу сетки.",
                    "To reproduce the waveform exactly, %1 rows would be needed — the table cannot "
                    "hold that many, %2 taken. For the upper frequencies this step is rather "
                    "coarse: if you do not need to edit the table, save the file directly from the "
                    "waveform — there the step is computed from the grid step.")
                .arg(int(std::ceil((t1 - t0) * fTop * 20.0))).arg(kMaxRows));
    }
}

void FileMakerDialog::saveSignal()
{
    const Project p = signalProject();
    const QString text = sigFileText(p, p.ports.first());
    bool lossy = false;
    const QByteArray bytes = toCp1251(text, &lossy);
    if (lossy) {
        QMessageBox::warning(this, tpl::tr("Не та кодировка", "Wrong encoding"),
                             tpl::tr("В тексте файла есть символы, которых нет в CP1251. "
                                     "Это дефект программы — сообщите о нём.",
                                     "The file text contains characters that are not in CP1251. This is a program "
                                     "defect — please report it."));
        return;
    }
    const QString path = QFileDialog::getSaveFileName(
        this, tpl::tr("Сохранить файл сигнала", "Save signal file"), p.name + QStringLiteral(".sig"),
        tpl::tr("Файл сигнала (*.sig)", "Signal file (*.sig)"));
    if (path.isEmpty())
        return;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size()) {
        QMessageBox::warning(this, tpl::tr("Не записалось", "Write failed"), f.errorString());
        return;
    }
    f.close();
    QMessageBox::information(
        this, tpl::tr("Готово", "Done"),
        tpl::tr("Файл %1 записан (%2).\n\nЧтобы ядро его нашло, файл должен лежать "
                "РЯДОМ с заданием, а в строке входа на месте амплитуды должно стоять "
                "[%3] — без пробела перед скобкой.",
                "File %1 written (%2).\n\nFor the kernel to find it, the file must lie NEXT TO "
                "the task, and the port line must have [%3] in place of the amplitude — with "
                "no space before the bracket.")
            .arg(QFileInfo(path).fileName(), sizeText(bytes.size()),
                 QFileInfo(path).completeBaseName()));
}

// ------------------------------------------------------- вкладка распределения

QWidget *FileMakerDialog::buildEpsTab()
{
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(12, 12, 12, 12);

    auto *why = new QLabel(tpl::tr("Файл .eps — готовое распределение диэлектрической проницаемости по сетке. "
        "Его читает блок FILE, и координаты блока ядро при этом ИГНОРИРУЕТ: сетка "
        "лежит в самом файле, а на место распределение ставит анкер. Шаг файла обязан "
        "совпасть с шагом задания — ядро сверяет их с точностью 1e-5.",
        "A .eps file is a ready distribution of the dielectric permittivity over the "
        "grid. It is read by the FILE block, and the kernel IGNORES the block "
        "coordinates: the grid is in the file itself, and the anchor puts the "
        "distribution in place. The file step must match the task step — the kernel "
        "compares them to within 1e-5."));
    why->setProperty("mute", true);
    why->setWordWrap(true);
    root->addWidget(why);

    // Открыть готовый файл — такой же путь, как сделать новый.
    //
    // ⚠️ У прочитанного файла формулы нет и быть не может: в нём лежат ЧИСЛА по узлам.
    // Поэтому карта показывает прочитанное как есть, а формула остаётся способом
    // задать новое распределение. Перезаписать файл прочитанными числами можно — он
    // получится тем же.
    auto *openRow = new QHBoxLayout;
    auto *openEps = new QPushButton(tpl::tr("Открыть готовый .eps…", "Open existing .eps…"));
    openEps->setObjectName(QStringLiteral("openEps"));
    openEps->setToolTip(tpl::tr("Прочитать чужой файл: его сетка и значения лягут на карту, а параметры — в "
        "поля. Так видно, что в файле на самом деле.",
        "Read a foreign file: its grid and values go onto the map, and its parameters "
        "into the fields. This shows what is actually in the file."));
    m_epsDrop = new QPushButton(tpl::tr("Вернуться к формуле", "Back to formula"));
    m_epsDrop->setVisible(false);
    openRow->addWidget(openEps);
    openRow->addWidget(m_epsDrop);
    openRow->addStretch();
    m_epsSource = new QLabel;
    m_epsSource->setProperty("mute", true);
    openRow->addWidget(m_epsSource, 1);
    root->addLayout(openRow);

    auto *form = new QFormLayout;
    // Умолчание — гауссов бугор: он считается во ВСЕЙ области.
    // Формула Люнеберга 2-(r/R)^2 годится только внутри круга, и в углах
    // квадратной сетки даёт eps <= 0 — такой файл ядро не возьмёт.
    m_epsFormula = new QLineEdit(QStringLiteral("1+2*exp(-(r/20)*(r/20))"));
    m_epsFormula->setObjectName(QStringLiteral("epsFormula"));
    m_epsFormula->setToolTip(tpl::tr("Сама eps, а не добавка: (eps-1) программа вычтет сама. Переменные — x, y "
        "(координаты от начала сетки), r (расстояние от начала) и f (полярный УГОЛ в "
        "радианах, а не частота). Частоты в контексте нет: файл частотно-независим.",
        "The eps itself, not the increment: the program subtracts (eps-1) itself. "
        "Variables: x, y (coordinates from the grid origin), r (distance from the "
        "origin) and f (polar ANGLE in radians, not frequency). There is no frequency "
        "in the context: the file is frequency-independent."));
    form->addRow(tpl::tr("Формула eps(x, y, r, f):", "Formula eps(x, y, r, f):"), m_epsFormula);

    m_epsDelta = spin(m_source.delta > 0.0 ? m_source.delta : 1.0, 0.0001, 10000.0,
                      tpl::tr(" мм", " mm"), 6, 0.1);
    m_epsDelta->setObjectName(QStringLiteral("epsDelta"));
    form->addRow(tpl::tr("Шаг сетки DELTA:", "Grid step DELTA:"), m_epsDelta);

    m_epsCentered = new QCheckBox(tpl::tr("Начало координат в центре области", "Origin at the center of the domain"));
    m_epsCentered->setChecked(true);
    m_epsCentered->setToolTip(tpl::tr("Так удобнее симметричным профилям: r = 0 в середине. Снимите галочку, чтобы "
        "задать углы области своими числами.",
        "Handier for symmetric profiles: r = 0 in the middle. Uncheck to set the "
        "domain corners with your own numbers."));
    form->addRow(QString(), m_epsCentered);

    m_epsX0 = spin(-40.0, -1e6, 1e6, tpl::tr(" мм", " mm"), 4, 1.0);
    m_epsX1 = spin(40.0, -1e6, 1e6, tpl::tr(" мм", " mm"), 4, 1.0);
    m_epsY0 = spin(-40.0, -1e6, 1e6, tpl::tr(" мм", " mm"), 4, 1.0);
    m_epsY1 = spin(40.0, -1e6, 1e6, tpl::tr(" мм", " mm"), 4, 1.0);
    auto *xRow = new QWidget;
    auto *xl = new QHBoxLayout(xRow);
    xl->setContentsMargins(0, 0, 0, 0);
    xl->addWidget(m_epsX0);
    xl->addWidget(new QLabel(QStringLiteral("…")));
    xl->addWidget(m_epsX1);
    form->addRow(tpl::tr("X от…до:", "X from…to:"), xRow);
    auto *yRow = new QWidget;
    auto *yl = new QHBoxLayout(yRow);
    yl->setContentsMargins(0, 0, 0, 0);
    yl->addWidget(m_epsY0);
    yl->addWidget(new QLabel(QStringLiteral("…")));
    yl->addWidget(m_epsY1);
    form->addRow(tpl::tr("Y от…до:", "Y from…to:"), yRow);

    m_epsName = new QLineEdit(QStringLiteral("lens"));
    m_epsName->setObjectName(QStringLiteral("epsName"));
    form->addRow(tpl::tr("Имя файла:", "File name:"), m_epsName);
    root->addLayout(form);

    m_epsMap = new EpsMapView;
    root->addWidget(m_epsMap, 1);

    m_epsSummary = new QLabel;
    m_epsSummary->setProperty("mute", true);
    m_epsSummary->setWordWrap(true);
    m_epsSummary->setTextInteractionFlags(Qt::TextSelectableByMouse);
    root->addWidget(m_epsSummary);
    m_epsWarn = new QLabel;
    m_epsWarn->setWordWrap(true);
    root->addWidget(m_epsWarn);

    auto *save = new QPushButton(tpl::tr("Сохранить .eps…", "Save .eps…"));
    save->setProperty("accent", true);
    save->setMinimumHeight(34);
    save->setObjectName(QStringLiteral("saveEps"));
    root->addWidget(save);

    const auto touch = [this] { refreshEps(); };
    for (QDoubleSpinBox *s : {m_epsDelta, m_epsX0, m_epsX1, m_epsY0, m_epsY1})
        connect(s, &QDoubleSpinBox::valueChanged, this, touch);
    connect(m_epsFormula, &QLineEdit::textChanged, this, touch);
    connect(m_epsCentered, &QCheckBox::toggled, this, touch);
    connect(save, &QPushButton::clicked, this, &FileMakerDialog::saveEps);
    connect(openEps, &QPushButton::clicked, this, &FileMakerDialog::openEpsFile);
    connect(m_epsDrop, &QPushButton::clicked, this, [this] {
        m_epsLoaded.clear();
        refreshEps();
    });
    return page;
}

EpsGrid FileMakerDialog::epsGrid() const
{
    const double d = m_epsDelta->value();
    if (m_epsCentered->isChecked()) {
        // Ширину берём ту же, что набрана полями, но начало кладём симметрично нулю.
        const double w = std::fabs(m_epsX1->value() - m_epsX0->value());
        const double h = std::fabs(m_epsY1->value() - m_epsY0->value());
        return epsGridFor(d, -w / 2.0, w / 2.0, -h / 2.0, h / 2.0);
    }
    return epsGridFor(d, m_epsX0->value(), m_epsX1->value(), m_epsY0->value(), m_epsY1->value());
}

void FileMakerDialog::refreshEps()
{
    // Прочитан чужой файл — показываем ЕГО, а не формулу: у готового файла формулы
    // нет, и подменять её вычисленной было бы враньём.
    if (!m_epsLoaded.isEmpty()) {
        const EpsGrid &g = m_epsLoadedGrid;
        m_epsMap->setField(g, m_epsLoaded);
        double lo = m_epsLoaded.first(), hi = m_epsLoaded.first();
        for (double v : m_epsLoaded) {
            lo = std::min(lo, v);
            hi = std::max(hi, v);
        }
        m_epsSummary->setText(
            tpl::tr("ИЗ ФАЙЛА · сетка %1 x %2 = %3 узлов · шаг %4 · область %5 x %6 "
                    "от (%7; %8) · добавка eps от %9 до %10",
                    "FROM FILE · grid %1 x %2 = %3 nodes · step %4 · domain %5 x %6 from (%7; %8) "
                    "· eps increment from %9 to %10")
                .arg(g.nX).arg(g.nY).arg(g.points())
                .arg(mmText(g.delta), mmText(g.width()), mmText(g.height()), mmText(g.xMin),
                     mmText(g.yMin))
                .arg(lo, 0, 'g', 4).arg(hi, 0, 'g', 4));
        m_epsWarn->setText(tpl::tr("Это числа из файла, а не формула. Чтобы задать "
                                   "своё распределение, нажмите «Вернуться к формуле».",
                                   "These are numbers from the file, not a formula. To set your own distribution, "
                                   "press “Back to formula”."));
        m_epsWarn->setStyleSheet(QStringLiteral("color:%1").arg(dtheme::warn().name()));
        m_epsDrop->setVisible(true);
        return;
    }
    m_epsDrop->setVisible(false);
    m_epsSource->clear();

    const EpsGrid g = epsGrid();
    QString error;
    // На экран идёт ДОБАВКА, как и в файл: ровная среда eps=3 даёт ровные 2.
    const QVector<double> add = epsValues(g, QStringLiteral("(%1)-1").arg(m_epsFormula->text()),
                                          &error);
    if (add.isEmpty()) {
        m_epsMap->clear();
        m_epsSummary->setText(tpl::tr("Сетка %1 x %2 узла", "Grid %1 x %2 nodes").arg(g.nX).arg(g.nY));
        m_epsWarn->setText(error.isEmpty() ? tpl::tr("Введите формулу.", "Enter a formula.")
                                           : tpl::tr("Формула: %1.", "Formula: %1.").arg(error));
        m_epsWarn->setStyleSheet(QStringLiteral("color:%1").arg(dtheme::danger().name()));
        return;
    }
    m_epsMap->setField(g, add);

    double lo = add.first(), hi = add.first();
    for (double v : add) {
        lo = std::min(lo, v);
        hi = std::max(hi, v);
    }
    m_epsSummary->setText(
        tpl::tr("Сетка %1 x %2 = %3 узлов · область %4 x %5 от (%6; %7) · "
                "eps от %8 до %9 · файл примерно %10",
                "Grid %1 x %2 = %3 nodes · domain %4 x %5 from (%6; %7) · eps from %8 to %9 · "
                "file about %10")
            .arg(g.nX).arg(g.nY).arg(g.points())
            .arg(mmText(g.width()), mmText(g.height()), mmText(g.xMin), mmText(g.yMin))
            .arg(lo + 1.0, 0, 'g', 4).arg(hi + 1.0, 0, 'g', 4)
            .arg(sizeText(qint64(g.points()) * 12 + 200)));

    QStringList warn;
    if (lo + 1.0 <= 0.0)
        warn << tpl::tr("Где-то eps не больше нуля — такую среду ядро не возьмёт.", "Somewhere eps is not greater than zero — the kernel will not accept such a medium.");
    if (g.points() > 4000000)
        warn << tpl::tr("Узлов очень много: файл выйдет огромным, а счёт долгим.", "Very many nodes: the file will be huge and the run long.");
    m_epsWarn->setText(warn.join(QStringLiteral(" ")));
    m_epsWarn->setStyleSheet(warn.isEmpty()
                                 ? QStringLiteral("color:%1").arg(dtheme::ok().name())
                                 : QStringLiteral("color:%1").arg(dtheme::warn().name()));
    if (warn.isEmpty())
        m_epsWarn->setText(tpl::tr("Формула считается во всех узлах.", "The formula is evaluated at all nodes."));
}

/// Открыть готовый `.eps` и показать, что в нём на самом деле.
void FileMakerDialog::openEpsFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tpl::tr("Открыть файл распределения", "Open distribution file"), QString(),
        tpl::tr("Файл распределения (*.eps);;Все файлы (*)", "Distribution file (*.eps);;All files (*)"));
    if (path.isEmpty())
        return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tpl::tr("Не читается", "Cannot read"), f.errorString());
        return;
    }
    const QByteArray data = f.readAll();
    EpsGrid g;
    QString error;
    if (!readEpsHeader(data, &g, &error)) {
        QMessageBox::warning(this, tpl::tr("Не распознан", "Not recognized"), error);
        return;
    }
    const QVector<double> values = readEpsValues(data, g);
    if (values.isEmpty()) {
        QMessageBox::warning(this, tpl::tr("Пусто", "Empty"),
                             tpl::tr("Шапка прочиталась, а данных по узлам в файле нет.", "The header was read, but the file has no node data."));
        return;
    }
    m_epsLoaded = values;
    m_epsLoadedGrid = g;
    // Поля приводим к прочитанному: видно, под какое задание этот файл годится.
    {
        QSignalBlocker b1(m_epsDelta), b2(m_epsX0), b3(m_epsX1), b4(m_epsY0), b5(m_epsY1),
            b6(m_epsCentered);
        m_epsDelta->setValue(g.delta);
        m_epsCentered->setChecked(false);
        m_epsX0->setValue(g.xMin);
        m_epsX1->setValue(g.xMax());
        m_epsY0->setValue(g.yMin);
        m_epsY1->setValue(g.yMax());
    }
    m_epsName->setText(QFileInfo(path).completeBaseName());
    refreshEps();
    m_epsSource->setText(tpl::tr("открыт %1", "opened %1").arg(QFileInfo(path).fileName()));
    QMessageBox::information(
        this, tpl::tr("Открыто", "Opened"),
        tpl::tr("%1: сетка %2 x %3, шаг %4. Такой файл годится заданию с ровно этим "
                "шагом — ядро сверяет его с DELTA с точностью 1e-5.",
                "%1: grid %2 x %3, step %4. Such a file suits a task with exactly this step — "
                "the kernel compares it with DELTA to within 1e-5.")
            .arg(QFileInfo(path).fileName()).arg(g.nX).arg(g.nY).arg(mmText(g.delta)));
}

void FileMakerDialog::saveEps()
{
    // Открытый файл сохраняем как есть: в нём ЧИСЛА, и пересчитывать их формулой
    // значило бы записать не то, что человек видит на карте.
    if (!m_epsLoaded.isEmpty()) {
        const EpsGrid &g = m_epsLoadedGrid;
        const QString name = m_epsName->text().trimmed().isEmpty()
                                 ? QStringLiteral("profile") : m_epsName->text().trimmed();
        const QString path = QFileDialog::getSaveFileName(
            this, tpl::tr("Сохранить файл распределения", "Save distribution file"), name + QStringLiteral(".eps"),
            tpl::tr("Файл распределения (*.eps)", "Distribution file (*.eps)"));
        if (path.isEmpty())
            return;
        // Веса узлов в прочитанных числах уже учтены — второй раз их накладывать нельзя.
        const QByteArray bytes = epsFileBytes(g, [&](double x, double y) {
            const int j = int(std::lround((x - g.xMin) / g.delta));
            const int i = int(std::lround((y - g.yMin) / g.delta));
            const int k = i * g.nX + j;
            double w = 1.0;
            if (j == 0 || j == g.nX - 1)
                w *= 0.5;
            if (i == 0 || i == g.nY - 1)
                w *= 0.5;
            return (k >= 0 && k < m_epsLoaded.size()) ? m_epsLoaded.at(k) / w : 0.0;
        });
        QFile out(path);
        if (!out.open(QIODevice::WriteOnly) || out.write(bytes) != bytes.size()) {
            QMessageBox::warning(this, tpl::tr("Не записалось", "Write failed"), out.errorString());
            return;
        }
        out.close();
        QMessageBox::information(this, tpl::tr("Готово", "Done"),
                                 tpl::tr("Файл %1 записан (%2).", "File %1 written (%2).")
                                     .arg(QFileInfo(path).fileName(), sizeText(bytes.size())));
        return;
    }

    const EpsGrid g = epsGrid();
    QString error;
    const QVector<double> add = epsValues(g, QStringLiteral("(%1)-1").arg(m_epsFormula->text()),
                                          &error);
    if (add.isEmpty()) {
        QMessageBox::warning(this, tpl::tr("Формула не считается", "Formula cannot be evaluated"),
                             error.isEmpty() ? tpl::tr("Введите формулу.", "Enter a formula.") : error);
        return;
    }
    const QByteArray bytes = epsFileBytes(g, [&](double x, double y) {
        const int j = int(std::lround((x - g.xMin) / g.delta));
        const int i = int(std::lround((y - g.yMin) / g.delta));
        const int k = i * g.nX + j;
        return (k >= 0 && k < add.size()) ? add.at(k) : 0.0;
    });

    const QString name = m_epsName->text().trimmed().isEmpty() ? QStringLiteral("profile")
                                                               : m_epsName->text().trimmed();
    const QString path = QFileDialog::getSaveFileName(
        this, tpl::tr("Сохранить файл распределения", "Save distribution file"), name + QStringLiteral(".eps"),
        tpl::tr("Файл распределения (*.eps)", "Distribution file (*.eps)"));
    if (path.isEmpty())
        return;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size()) {
        QMessageBox::warning(this, tpl::tr("Не записалось", "Write failed"), f.errorString());
        return;
    }
    f.close();
    QMessageBox::information(
        this, tpl::tr("Готово", "Done"),
        tpl::tr("Файл %1 записан (%2).\n\nПоложите его РЯДОМ с заданием и поставьте "
                "фигуре профиль «из файла» с этим именем. Шаг задания должен быть "
                "%3 — иначе ядро файл не примет.",
                "File %1 written (%2).\n\nPut it NEXT TO the task and give the shape the “from "
                "file” profile with this name. The task step must be %3 — otherwise the kernel "
                "will not accept the file.")
            .arg(QFileInfo(path).fileName(), sizeText(bytes.size()), mmText(g.delta)));
}
