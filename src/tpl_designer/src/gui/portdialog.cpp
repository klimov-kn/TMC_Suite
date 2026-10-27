/// @file portdialog.cpp
/// @brief Реализация диалога «Свойства порта» (`PortDialog`).
///
/// Часть интерфейса редактора заданий.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-dialogs
#include "portdialog.h"

#include "cp1251.h"
#include "emitter.h"
#include "physics.h"
#include "sigfile.h"
#include "signalgallery.h"
#include "signalpreview.h"
#include "dtheme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include "lang.h"

using namespace tpl;

namespace {

/// Счётчик с единицей и нужной точностью — их в этом окне два десятка.
/// @brief Создаёт числовое поле с диапазоном, единицей и точностью.
/// @param[in] value     начальное значение.
/// @param[in] lo        нижняя граница.
/// @param[in] hi        верхняя граница.
/// @param[in] suffix    подпись единиц после числа.
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

} // namespace

PortDialog::PortDialog(const Port &p, const Project &project, QWidget *parent)
    : QDialog(parent), m_port(p), m_project(project)
{
    setWindowTitle(tpl::tr("Свойства порта", "Port properties"));
    setMinimumWidth(620);
    setSizeGripEnabled(true);
    m_points = p.signal.wave.points;
    m_tableTones = p.signal.wave.tableTones;

    // Содержимое — в прокрутке: иначе на невысоком экране кнопки «Применить»
    // окажутся ниже нижнего края.
    auto *content = new QWidget;
    auto *root = new QVBoxLayout(content);
    root->setContentsMargins(0, 0, 0, 0);

    buildPlacement(content);
    buildExcitation(content);
    buildPreview(content);
    root->addStretch();

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(content);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText(tpl::tr("Применить", "Apply"));
    buttons->button(QDialogButtonBox::Ok)->setProperty("accent", true);
    buttons->button(QDialogButtonBox::Cancel)->setText(tpl::tr("Отмена", "Cancel"));
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *outer = new QVBoxLayout(this);
    outer->addWidget(scroll, 1);
    outer->addWidget(buttons);

    refresh();
    dtheme::fitToScreen(this);
}

void PortDialog::buildPlacement(QWidget *parent)
{
    const Port &p = m_port;
    auto *place = new QGroupBox(tpl::tr("Размещение", "Placement"), parent);
    parent->layout()->addWidget(place);
    auto *pf = new QFormLayout(place);

    m_name = new QLineEdit(p.name);
    m_side = new QComboBox;
    m_side->addItem(tpl::tr("Левая граница", "Left boundary"), int(PortSide::Left));
    m_side->addItem(tpl::tr("Правая граница", "Right boundary"), int(PortSide::Right));
    m_side->addItem(tpl::tr("Нижняя граница", "Bottom boundary"), int(PortSide::Bottom));
    m_side->addItem(tpl::tr("Верхняя граница", "Top boundary"), int(PortSide::Top));
    m_side->setCurrentIndex(m_side->findData(int(p.side)));

    m_from = spin(p.from, -100000, 100000, tpl::tr(" мм", " mm"));
    m_to = spin(p.to, -100000, 100000, tpl::tr(" мм", " mm"));

    pf->addRow(tpl::tr("Название:", "Name:"), m_name);
    pf->addRow(tpl::tr("Сторона:", "Side:"), m_side);
    pf->addRow(tpl::tr("Апертура от:", "Aperture from:"), m_from);
    pf->addRow(tpl::tr("Апертура до:", "Aperture to:"), m_to);

    // Среда в сечении входа. Пользователь вводит саму eps, добавку (eps-1) в файл
    // подставит эмиттер. Нижняя граница НЕ ноль: ядро отвергает файл при eps <= 0,
    // поэтому ноль просто нечем ввести — ловушка закрыта полем, а не памятью человека.
    //
    // 🔑 По умолчанию eps берётся из фигуры, в которой стоит вход: восьмой параметр
    // входа — это Δε его среды (диссертация К. Н. Климова, (7.54)), и вход в диэлектрике
    // с вакуумом в этом параметре согласован не со своей средой. Руками — только если
    // среду в сечении задаёт не фигура, а модель (эффективная eps микрополоски).
    m_epsAuto = new QCheckBox(tpl::tr("По среде, в которой стоит вход", "From the medium the port sits in"));
    m_epsAuto->setChecked(p.epsAuto);
    m_epsAuto->setToolTip(tpl::tr("Программа берёт eps той фигуры, что накрывает середину апертуры в плоскости входа. "
        "Снимите галочку, чтобы задать eps руками.",
        "The program takes the eps of the shape that covers the middle of the aperture "
        "in the port plane. Clear the check box to set eps by hand."));
    pf->addRow(QString(), m_epsAuto);
    m_eps = spin(p.eps, 0.001, 10000.0, QString(), 4);
    pf->addRow(tpl::tr("Проницаемость на входе:", "Permittivity at the port:"), m_eps);

    m_epsHint = new QLabel;
    m_epsHint->setProperty("mute", true);
    m_epsHint->setWordWrap(true);
    pf->addRow(QString(), m_epsHint);

    m_cutoff = new QLabel;
    m_cutoff->setProperty("mute", true);
    m_cutoff->setWordWrap(true);
    pf->addRow(QString(), m_cutoff);

    const auto touch = [this] { refresh(); };
    connect(m_from, &QDoubleSpinBox::valueChanged, this, touch);
    connect(m_to, &QDoubleSpinBox::valueChanged, this, touch);
    connect(m_eps, &QDoubleSpinBox::valueChanged, this, touch);
    connect(m_epsAuto, &QCheckBox::toggled, this, touch);
}

void PortDialog::buildExcitation(QWidget *parent)
{
    const Port &p = m_port;
    const Waveshape &w = p.signal.wave;
    auto *exc = new QGroupBox(tpl::tr("Возбуждение", "Excitation"), parent);
    parent->layout()->addWidget(exc);
    auto *box = new QVBoxLayout(exc);

    auto *head = new QFormLayout;
    head->setContentsMargins(0, 0, 0, 0);
    auto *actRow = new QWidget;
    auto *al = new QHBoxLayout(actRow);
    al->setContentsMargins(0, 0, 0, 0);
    m_active = new QRadioButton(tpl::tr("Активный (излучает)", "Active (radiates)"));
    m_passive = new QRadioButton(tpl::tr("Пассивный (только принимает)", "Passive (receives only)"));
    m_active->setChecked(p.active);
    m_passive->setChecked(!p.active);
    al->addWidget(m_active);
    al->addWidget(m_passive);
    al->addStretch();
    head->addRow(actRow);

    m_mode = new QComboBox;
    m_mode->addItem(tpl::tr("Волна H10 (волноводная)", "H10 wave (waveguide)"), int(PortMode::H10));
    m_mode->addItem(tpl::tr("Плоская волна", "Plane wave"), int(PortMode::Plane));
    m_mode->setCurrentIndex(m_mode->findData(int(p.mode)));
    head->addRow(tpl::tr("Тип волны:", "Wave type:"), m_mode);

    auto *srcRow = new QWidget;
    auto *sl = new QHBoxLayout(srcRow);
    sl->setContentsMargins(0, 0, 0, 0);
    m_byFormula = new QRadioButton(tpl::tr("Формулой ядра", "By kernel formula"));
    m_byFormula->setToolTip(tpl::tr("Ядро само считает a = Amp(t)*sin(2*pi*FREQ*t + Faza). Дёшево и точно, но "
        "выразить так можно только несущую под огибающей.",
        "The kernel itself computes a = Amp(t)*sin(2*pi*FREQ*t + Faza). Cheap and "
        "exact, but only a carrier under an envelope can be expressed this way."));
    m_byFile = new QRadioButton(tpl::tr("Файлом сигнала (любая форма)", "By signal file (any shape)"));
    m_byFile->setToolTip(tpl::tr("Падающую волну задаёт файл .sig, который программа напишет сама. Так задаются "
        "формы, которых в языке нет: гребёнка тонов на всю полосу за один проход, ЛЧМ, "
        "пила, видеоимпульс, своя формула, свои точки.",
        "The incident wave is set by a .sig file that the program writes itself. This "
        "is how shapes absent from the language are set: a comb of tones over the "
        "whole band in one pass, a chirp, a sawtooth, a video pulse, your own formula, "
        "your own points."));
    m_byFormula->setChecked(p.signal.source == SignalSource::Formula);
    m_byFile->setChecked(p.signal.source == SignalSource::File);
    sl->addWidget(m_byFormula);
    sl->addWidget(m_byFile);
    sl->addStretch();
    head->addRow(tpl::tr("Откуда сигнал:", "Signal source:"), srcRow);
    box->addLayout(head);

    m_sourceStack = new QStackedWidget;
    box->addWidget(m_sourceStack);

    // --- страница «формулой» ---
    auto *fpage = new QWidget;
    auto *ff = new QFormLayout(fpage);
    ff->setContentsMargins(0, 6, 0, 0);
    m_shape = new QComboBox;
    m_shape->addItem(tpl::tr("Непрерывный сигнал", "Continuous signal"), int(SignalShape::Continuous));
    m_shape->addItem(tpl::tr("Гауссов импульс", "Gaussian pulse"), int(SignalShape::Gaussian));
    m_shape->addItem(tpl::tr("Прямоугольный импульс", "Rectangular pulse"), int(SignalShape::Rectangular));
    m_shape->setCurrentIndex(m_shape->findData(int(p.signal.shape)));
    ff->addRow(tpl::tr("Форма сигнала:", "Signal shape:"), m_shape);

    // Единиц напряжения в языке нет — амплитуда безразмерна. Врать про вольты нельзя.
    m_amp = spin(p.signal.amplitude, -1e6, 1e6, QString(), 4);
    ff->addRow(tpl::tr("Амплитуда (отн. ед.):", "Amplitude (rel. units):"), m_amp);
    m_phase = spin(p.signal.phase * 180.0 / M_PI, -360, 360, QStringLiteral(" °"), 1, 1.0);
    ff->addRow(tpl::tr("Фаза:", "Phase:"), m_phase);
    m_center = spin(p.signal.center, 0, 1e6, tpl::tr(" нс", " ns"));
    ff->addRow(tpl::tr("Центр импульса:", "Pulse center:"), m_center);
    m_duration = spin(p.signal.duration, 0.001, 1e6, tpl::tr(" нс", " ns"));
    ff->addRow(tpl::tr("Длительность:", "Duration:"), m_duration);
    m_sourceStack->addWidget(fpage);

    // --- страница «файлом» ---
    auto *wpage = new QWidget;
    auto *wl = new QVBoxLayout(wpage);
    wl->setContentsMargins(0, 6, 0, 0);
    m_gallery = new SignalGallery;
    m_gallery->setCurrent(w.kind);
    wl->addWidget(m_gallery);

    auto *wf = new QFormLayout;
    wf->setContentsMargins(0, 0, 0, 0);
    const auto row = [&](const QString &label, QWidget *field) {
        wf->addRow(label, field);
        m_rowLabel.insert(field, wf->labelForField(field));
    };

    m_wAmp = spin(w.amplitude, -1e6, 1e6, QString(), 4);
    wf->addRow(tpl::tr("Амплитуда (отн. ед.):", "Amplitude (rel. units):"), m_wAmp);
    m_wStart = spin(w.start, 0, 1e6, tpl::tr(" нс", " ns"));
    wf->addRow(tpl::tr("Начало сигнала:", "Signal start:"), m_wStart);

    m_wDuration = spin(w.duration, 0.0001, 1e6, tpl::tr(" нс", " ns"));
    row(tpl::tr("Длительность посылки:", "Burst duration:"), m_wDuration);
    m_wF1 = spin(w.f1, 0.0001, 100000, tpl::tr(" ГГц", " GHz"), 4);
    row(tpl::tr("Нижняя частота:", "Lower frequency:"), m_wF1);
    m_wF2 = spin(w.f2, 0.0001, 100000, tpl::tr(" ГГц", " GHz"), 4);
    row(tpl::tr("Верхняя частота:", "Upper frequency:"), m_wF2);
    m_wTones = new QSpinBox;
    m_wTones->setRange(2, kMaxFreqs); // больше 99 тонов пакет не обработает
    m_wTones->setValue(std::max(2, w.tones));
    row(tpl::tr("Тонов в гребёнке:", "Tones in the comb:"), m_wTones);
    m_wPeriods = new QSpinBox;
    m_wPeriods->setRange(1, 100000);
    m_wPeriods->setValue(std::max(1, w.periods));
    row(tpl::tr("Периодов в посылке:", "Periods in the burst:"), m_wPeriods);
    m_wWidth = spin(w.width, 0.0001, 1e6, tpl::tr(" нс", " ns"), 4, 0.05);
    row(tpl::tr("Длительность tau:", "Duration tau:"), m_wWidth);
    m_wRise = spin(w.rise, 0.0, 1e6, tpl::tr(" нс", " ns"), 4, 0.01);
    row(tpl::tr("Фронт:", "Rise:"), m_wRise);
    m_wFall = spin(w.fall, 0.0, 1e6, tpl::tr(" нс", " ns"), 4, 0.01);
    row(tpl::tr("Спад:", "Fall:"), m_wFall);
    m_wExpr = new QLineEdit(w.expression);
    m_wExpr->setPlaceholderText(tpl::tr("например exp(-t*t)*sin(2*3.14159265*4*t)", "e.g. exp(-t*t)*sin(2*3.14159265*4*t)"));
    m_wExpr->setToolTip(tpl::tr("Переменная одна: t в наносекундах. Понимаются + - * / ^ и функции abs, exp, "
        "log, log10, sqrt, sin, cos, tan, asin, acos, atan, sinh, cosh, tanh (углы в "
        "радианах). Формулу считаем мы, в файл уходят числа.",
        "A single variable: t in nanoseconds. Supported: + - * / ^ and the functions "
        "abs, exp, log, log10, sqrt, sin, cos, tan, asin, acos, atan, sinh, cosh, tanh "
        "(angles in radians). The formula is evaluated by us; numbers go to the file."));
    row(tpl::tr("Формула a(t):", "Formula a(t):"), m_wExpr);

    auto *tableRow = new QWidget;
    auto *tl = new QHBoxLayout(tableRow);
    tl->setContentsMargins(0, 0, 0, 0);
    m_wTable = new QLabel;
    m_wLoad = new QPushButton(tpl::tr("Загрузить точки из файла…", "Load points from file…"));
    tl->addWidget(m_wTable, 1);
    tl->addWidget(m_wLoad);
    row(tpl::tr("Своя таблица:", "Custom table:"), tableRow);

    m_wFile = new QLineEdit(m_port.signal.file);
    m_wFile->setPlaceholderText(tpl::tr("пусто — имя даст программа", "empty — the program will give a name"));
    m_wFile->setToolTip(tpl::tr("Имя файла .sig без расширения и без точки: "
                                "расширение ядро дописывает само.",
                                "Name of the .sig file without extension and without a dot: the kernel appends "
                                "the extension itself."));
    wf->addRow(tpl::tr("Имя файла:", "File name:"), m_wFile);

    m_wFileInfo = new QLabel;
    m_wFileInfo->setProperty("mute", true);
    m_wFileInfo->setWordWrap(true);
    wf->addRow(QString(), m_wFileInfo);
    wl->addLayout(wf);
    m_sourceStack->addWidget(wpage);

    const auto touch = [this] { refresh(); };
    connect(m_active, &QRadioButton::toggled, this, touch);
    connect(m_mode, &QComboBox::currentIndexChanged, this, touch);
    connect(m_byFormula, &QRadioButton::toggled, this, touch);
    connect(m_shape, &QComboBox::currentIndexChanged, this, touch);
    connect(m_amp, &QDoubleSpinBox::valueChanged, this, touch);
    connect(m_phase, &QDoubleSpinBox::valueChanged, this, touch);
    connect(m_center, &QDoubleSpinBox::valueChanged, this, touch);
    connect(m_duration, &QDoubleSpinBox::valueChanged, this, touch);
    connect(m_gallery, &SignalGallery::currentChanged, this, touch);
    for (QDoubleSpinBox *s : {m_wAmp, m_wStart, m_wDuration, m_wF1, m_wF2, m_wWidth, m_wRise,
                              m_wFall})
        connect(s, &QDoubleSpinBox::valueChanged, this, touch);
    connect(m_wTones, &QSpinBox::valueChanged, this, touch);
    connect(m_wPeriods, &QSpinBox::valueChanged, this, touch);
    connect(m_wExpr, &QLineEdit::textChanged, this, touch);
    connect(m_wFile, &QLineEdit::textChanged, this, touch);
    connect(m_wLoad, &QPushButton::clicked, this, &PortDialog::loadTableFromFile);
}

void PortDialog::buildPreview(QWidget *parent)
{
    auto *prev = new QGroupBox(tpl::tr("Что придёт на вход", "What arrives at the port"), parent);
    parent->layout()->addWidget(prev);
    auto *pl = new QVBoxLayout(prev);
    m_preview = new SignalPreview;
    pl->addWidget(m_preview);
    m_formula = new QLabel;
    m_formula->setProperty("mute", true);
    m_formula->setWordWrap(true);
    m_formula->setTextInteractionFlags(Qt::TextSelectableByMouse);
    pl->addWidget(m_formula);
    m_hint = new QLabel;
    m_hint->setWordWrap(true);
    pl->addWidget(m_hint);
}

void PortDialog::loadTableFromFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tpl::tr("Точки падающей волны", "Incident wave points"), QString(),
        tpl::tr("Файлы сигнала (*.sig *.txt *.csv *.dat);;Все файлы (*)", "Signal files (*.sig *.txt *.csv *.dat);;All files (*)"));
    if (path.isEmpty())
        return;
    QString text;
    QString err;
    if (!readTplFileText(path, &text, &err)) {
        QMessageBox::warning(this, tpl::tr("Не удалось прочитать", "Could not read"),
                             err.isEmpty() ? tpl::tr("Файл не открылся", "The file did not open") : err);
        return;
    }
    const QVector<QPointF> pts = parseSigPoints(text);
    if (pts.size() < 2) {
        QMessageBox::warning(this, tpl::tr("Точек не нашлось", "No points found"),
                             tpl::tr("В файле должно быть не меньше двух строк из двух "
                                     "чисел: время (нс) и значение. Строки с «!» и «#» "
                                     "считаются комментариями.",
                                     "The file must contain at least two lines of two numbers: time (ns) and value. "
                                     "Lines with «!» and «#» are treated as comments."));
        return;
    }
    m_points = pts;
    m_tableTones = parseSigTones(text);
    m_gallery->setCurrent(Waveform::Table);
    refresh();
    // Окно, а не строка в подсказке: человек принёс файл и ждёт, что он возьмётся
    // целиком. Что берётся не всё, должно быть видно в момент загрузки.
    const QString cut = toneCutNotice(m_tableTones);
    if (!cut.isEmpty())
        QMessageBox::warning(this, tpl::tr("Тонов больше %1", "More than %1 tones").arg(kMaxFreqs), cut);
}

void PortDialog::refresh()
{
    const bool active = m_active->isChecked();
    const bool byFile = m_byFile->isChecked();
    m_sourceStack->setCurrentIndex(byFile ? 1 : 0);
    m_sourceStack->setEnabled(active);
    m_mode->setEnabled(active);
    m_byFormula->setEnabled(active);
    m_byFile->setEnabled(active);

    const auto shape = SignalShape(m_shape->currentData().toInt());
    const bool pulse = (shape != SignalShape::Continuous);
    m_center->setEnabled(pulse);
    m_duration->setEnabled(pulse);

    // Показываем только те поля, которые у выбранной формы есть. Двенадцать строк
    // сразу — это способ спрятать нужную: у гребёнки нет ни фронта, ни периодов.
    const Waveform kind = m_gallery->current();
    const WaveFields f = waveFields(kind);
    const auto setRow = [&](QWidget *field, bool on) {
        field->setVisible(on);
        if (QWidget *lab = m_rowLabel.value(field))
            lab->setVisible(on);
    };
    setRow(m_wDuration, f.duration);
    setRow(m_wF1, f.band || f.carrier || f.rate);
    setRow(m_wF2, f.band);
    setRow(m_wTones, f.tones);
    setRow(m_wPeriods, f.periods);
    setRow(m_wWidth, f.width);
    setRow(m_wRise, f.edges);
    setRow(m_wFall, f.edges);
    setRow(m_wExpr, f.expression);
    // Кнопка и счётчик точек лежат в своём контейнере — его и прячем.
    setRow(m_wTable->parentWidget(), f.table);

    // Подпись у f1 меняется по смыслу: у гребёнки это граница полосы, у пилы —
    // частота повторения, у радиоимпульса — несущая. Одно слово «частота» на три
    // разных величины — способ запутать.
    if (auto *lab = qobject_cast<QLabel *>(m_rowLabel.value(m_wF1))) {
        lab->setText(f.band ? tpl::tr("Нижняя частота:", "Lower frequency:")
                            : (f.rate ? tpl::tr("Частота повторения:", "Repetition frequency:")
                                      : tpl::tr("Несущая:", "Carrier:")));
    }

    const Port p = port();
    m_preview->setPort(p, m_project);
    QString tableText = m_points.size() >= 2
                            ? plural(int(m_points.size()), tpl::tr("точка", "point"),
                                     tpl::tr("точки", "points"), tpl::tr("точек", "points"))
                            : tpl::tr("точек нет", "no points");
    if (!m_tableTones.isEmpty()) {
        tableText += m_tableTones.size() > kMaxFreqs
                         ? tpl::tr(", %1 в #FREQ — возьмутся первые %2", ", %1 in #FREQ — the first %2 will be taken")
                               .arg(plural(int(m_tableTones.size()), tpl::tr("тон", "tone"),
                                           tpl::tr("тона", "tones"), tpl::tr("тонов", "tones")))
                               .arg(kMaxFreqs)
                         : tpl::tr(", %1 в #FREQ", ", %1 in #FREQ")
                               .arg(plural(int(m_tableTones.size()), tpl::tr("тон", "tone"),
                                           tpl::tr("тона", "tones"), tpl::tr("тонов", "tones")));
    }
    m_wTable->setText(tableText);

    // Что именно уйдёт в файл задания — текстом, без догадок.
    if (!active) {
        m_formula->setText(tpl::tr("В файл: Amp = 0; окно за концом счёта", "To file: Amp = 0; window after the end of the run"));
    } else if (byFile) {
        const SigPlan plan = sigPlan(m_project, p);
        const TimeWindow win = portWindow(p, m_project.timeMax);
        m_formula->setText(
            tpl::tr("В файл задания: Amp = [%1];  окно %2...%3 нс", "To task file: Amp = [%1];  window %2...%3 ns")
                .arg(signalFileName(m_project, p))
                .arg(win.tMin, 0, 'g', 5)
                .arg(win.tMax, 0, 'g', 5));
        m_wFileInfo->setText(
            tpl::tr("Рядом с заданием ляжет %1.sig: %2, шаг %3 нс (вдвое мельче шага "
                    "ядра %4 нс). Пик сигнала %5.",
                    "%1.sig will be placed next to the task: %2, step %3 ns (half the kernel step "
                    "%4 ns). Signal peak %5.")
                .arg(signalFileName(m_project, p),
                     plural(plan.points, tpl::tr("точка", "point"), tpl::tr("точки", "points"),
                            tpl::tr("точек", "points")),
                     QString::number(plan.dt, 'g', 4), QString::number(plan.kernelDt, 'g', 4),
                     QString::number(wavePeak(p.signal.wave), 'g', 4)));
    } else {
        const TimeWindow win = portWindow(p, m_project.timeMax);
        m_formula->setText(tpl::tr("В файл: Amp = %1;  окно %2...%3 нс", "To file: Amp = %1;  window %2...%3 ns")
                               .arg(ampExpression(p.signal))
                               .arg(win.tMin, 0, 'g', 5)
                               .arg(win.tMax, 0, 'g', 5));
    }

    // Заполнение входа сдвигает отсечку: в среде волна идёт медленнее в sqrt(eps) раз.
    // Считать отсечку по вакууму, когда вход заполнен, значит соврать пользователю.
    const bool autoEps = m_epsAuto->isChecked();
    if (autoEps) {
        const QSignalBlocker block(m_eps);
        m_eps->setValue(portMediumEps(m_project, port()));
    }
    m_eps->setEnabled(!autoEps);
    const double eps = std::max(m_eps->value(), 0.001);
    QString epsText =
        eps == 1.0
            ? tpl::tr("1 — вакуум: параметр в файл не пишется вовсе.", "1 — vacuum: the parameter is not written to the file at all.")
            : tpl::tr("В файл уйдёт добавка (eps-1) = %1. Заполнение входа "
                      "меняет его волновую проводимость, а значит и S-матрицу.",
                      "The file gets the increment (eps-1) = %1. Filling the port changes its wave "
                      "admittance, and hence the S-matrix.")
                  .arg(eps - 1.0, 0, 'g', 6);
    if (autoEps)
        epsText = tpl::tr("Взята из фигуры под входом. ", "Taken from the shape under the port. ") + epsText;
    m_epsHint->setText(epsText);

    // Отсечка — по апертуре этого порта: у произвольной геометрии «ширины волновода» нет.
    const double a = m_to->value() - m_from->value();
    if (a > 0.0) {
        const double fc10 = cutoffH10(a, eps);
        const double fc20 = cutoffH20(a, eps);
        QString text = tpl::tr("Апертура %1 мм: одномодовый диапазон %2...%3 ГГц "
                               "(H10 включается на %2, H20 — на %3).",
                               "Aperture %1 mm: single-mode band %2...%3 GHz (H10 turns on at %2, H20 at %3).")
                           .arg(a, 0, 'g', 5)
                           .arg(fc10, 0, 'f', 3)
                           .arg(fc20, 0, 'f', 3);
        if (eps != 1.0)
            text += tpl::tr(" Отсечка посчитана с учётом заполнения eps = %1.", " The cutoff accounts for the filling eps = %1.").arg(eps);
        m_cutoff->setText(text);
    } else {
        m_cutoff->setText(tpl::tr("Апертура нулевая — порт не примут.", "Zero aperture — the port will be rejected."));
    }

    // Подсказка внизу. Говорим о том, что ядро отвергнет или посчитает не так, как
    // нарисовано, — а не о физическом смысле задачи.
    QString hint;
    bool warnColor = false;
    if (!active) {
        hint = tpl::tr("Пассивный порт глушится и окном, и нулевой амплитудой.", "A passive port is muted both by the window and by zero amplitude.");
    } else if (byFile) {
        QStringList parts;
        const double want = wantedFreq();
        if (want > 0.0) {
            parts << tpl::tr("Частоту задания стоит держать на %1 ГГц — по ней ядро "
                             "согласует порт. Программа поставит её сама.",
                             "Keep the task frequency at %1 GHz — the kernel matches the port at it. The "
                             "program will set it itself.")
                         .arg(want, 0, 'f', 4);
        }
        const Band b = waveBand(p.signal.wave);
        if (b.known && a > 0.0 && p.mode == PortMode::H10) {
            if (b.hi > cutoffH20(a, eps)) {
                parts << tpl::tr("⚠ Полоса уходит выше отсечки H20: там в апертуре не "
                                 "одна мода, и матрица рассеяния по одной моде "
                                 "неверна.",
                                 "⚠ The band goes above the H20 cutoff: there is more than one mode in the "
                                 "aperture, and the single-mode scattering matrix is wrong.");
                warnColor = true;
            } else if (b.lo < cutoffH10(a, eps)) {
                parts << tpl::tr("⚠ Низ полосы ниже отсечки H10: эти частоты в вход не "
                                 "пойдут.",
                                 "⚠ The bottom of the band is below the H10 cutoff: these frequencies will not "
                                 "enter the port.");
                warnColor = true;
            }
        }
        const TimeWindow sup = waveSupport(p.signal.wave);
        if (sup.tMax > m_project.timeMax) {
            parts << tpl::tr("⚠ Сигнал длиннее счёта (%1 нс против %2 нс) — хвост "
                             "обрежется.",
                             "⚠ The signal is longer than the run (%1 ns vs %2 ns) — the tail will be cut "
                             "off.")
                         .arg(sup.tMax, 0, 'g', 4).arg(m_project.timeMax, 0, 'g', 4);
            warnColor = true;
        }
        if (parts.isEmpty())
            parts << tpl::tr("Файл напишет программа: править его руками не нужно.", "The program writes the file: no need to edit it by hand.");
        hint = parts.join(QLatin1Char(' '));
    } else if (shape == SignalShape::Rectangular) {
        hint = tpl::tr("⚠ Резкое окно обрубает несущую — спектр расползётся. "
                       "Для гладкого импульса возьмите гауссову форму.",
                       "⚠ A sharp window chops the carrier — the spectrum will spread. For a smooth "
                       "pulse take the Gaussian shape.");
        warnColor = true;
    } else if (shape == SignalShape::Gaussian) {
        hint = tpl::tr("Длительность задаёт огибающая, окно остаётся широким: "
                       "сужать окно нельзя — оно режет резко.",
                       "The duration is set by the envelope, the window stays wide: the window must "
                       "not be narrowed — it cuts sharply.");
    } else {
        hint = tpl::tr("Несущая идёт весь счёт. Амплитуда безразмерна: "
                       "единиц напряжения в языке задания нет.",
                       "The carrier runs the whole computation. The amplitude is dimensionless: the "
                       "task language has no voltage units.");
    }
    m_hint->setText(hint);
    m_hint->setStyleSheet(warnColor ? QStringLiteral("color:#e3b341;")
                                    : QStringLiteral("color:#6c7d97;"));
}

double PortDialog::wantedFreq() const
{
    if (!m_active->isChecked() || !m_byFile->isChecked())
        return 0.0;
    return suggestedFreq(port().signal.wave);
}

Port PortDialog::port() const
{
    Port p = m_port;
    p.name = m_name->text();
    p.side = PortSide(m_side->currentData().toInt());
    p.from = m_from->value();
    p.to = m_to->value();
    p.active = m_active->isChecked();
    p.mode = PortMode(m_mode->currentData().toInt());
    // В режиме «по среде» поле показывает eps среды — её же держим и в p.eps, чтобы
    // график и подсказки, читающие p.eps, видели то, что уйдёт в файл.
    p.epsAuto = m_epsAuto->isChecked();
    p.eps = m_eps->value();

    p.signal.source = m_byFile->isChecked() ? SignalSource::File : SignalSource::Formula;
    p.signal.shape = SignalShape(m_shape->currentData().toInt());
    p.signal.amplitude = m_amp->value();
    p.signal.phase = m_phase->value() * M_PI / 180.0;
    p.signal.center = m_center->value();
    p.signal.duration = m_duration->value();

    Waveshape w;
    w.kind = m_gallery->current();
    w.amplitude = m_wAmp->value();
    w.start = m_wStart->value();
    w.duration = m_wDuration->value();
    w.f1 = m_wF1->value();
    w.f2 = m_wF2->value();
    w.tones = m_wTones->value();
    w.periods = m_wPeriods->value();
    w.width = m_wWidth->value();
    w.rise = m_wRise->value();
    w.fall = m_wFall->value();
    w.expression = m_wExpr->text();
    w.points = m_points;
    w.tableTones = m_tableTones;
    p.signal.wave = w;
    p.signal.file = m_wFile->text().trimmed();
    return p;
}
