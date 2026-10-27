/// @file calculatordialog.cpp
/// @brief Реализация справочного калькулятора среды и волновода (`CalculatorDialog`).
///
/// Часть интерфейса редактора заданий. Все величины считаются формулами `core/physics.h`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-dialogs
#include "calculatordialog.h"

#include "physics.h"
#include "dtheme.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include "lang.h"

namespace {

/// Формулы взяты из `core/physics.h` — там же ими пользуется валидатор.
/// Разъехавшиеся копии означали бы «калькулятор сказал одно, проверка другое».
/// Скорость света в единицах калькулятора: мм * ГГц (299.792458).
constexpr double kC = tpl::kLightSpeed;

/// @brief Создаёт поле ввода числа с пересчётом только по завершённому вводу.
/// @param[in] v       начальное значение.
/// @param[in] suffix  подпись единиц после числа (например, « ГГц»).
/// @param[in] lo      нижняя граница диапазона.
/// @param[in] hi      верхняя граница диапазона.
/// @param[in] dec     число знаков после запятой.
/// @return Новый `QDoubleSpinBox` без родителя (владельцем станет компоновка).
QDoubleSpinBox *num(double v, const QString &suffix, double lo, double hi, int dec = 4)
{
    auto *s = new QDoubleSpinBox;
    s->setRange(lo, hi);
    s->setDecimals(dec);
    s->setSuffix(suffix);
    s->setValue(v);
    s->setKeyboardTracking(false); // пересчёт по готовому числу, а не по каждой цифре
    return s;
}

} // namespace

CalculatorDialog::CalculatorDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tpl::tr("Калькулятор среды и волновода", "Medium and waveguide calculator"));
    setMinimumWidth(520);
    setSizeGripEnabled(true);

    auto *content = new QWidget;
    auto *root = new QVBoxLayout(content);
    root->setContentsMargins(0, 0, 0, 0);

    // ---------------------------------------------------------------- волновод
    auto *wg = new QGroupBox(tpl::tr("Волновод: одномодовый диапазон", "Waveguide: single-mode band"));
    auto *wf = new QFormLayout(wg);
    m_wgWidth = num(60.0, tpl::tr(" мм", " mm"), 0.001, 1e6, 3);
    m_wgEps = num(1.0, QString(), 0.0001, 1e4, 4);
    m_fc10 = num(0.0, tpl::tr(" ГГц", " GHz"), 0.0001, 1e6, 4);
    m_fc20 = num(0.0, tpl::tr(" ГГц", " GHz"), 0.0001, 1e6, 4);
    wf->addRow(tpl::tr("Ширина a:", "Width a:"), m_wgWidth);
    wf->addRow(tpl::tr("Заполнение eps:", "Filling eps:"), m_wgEps);
    wf->addRow(tpl::tr("Отсечка H10:", "H10 cutoff:"), m_fc10);
    wf->addRow(tpl::tr("Отсечка H20:", "H20 cutoff:"), m_fc20);
    m_wgHint = new QLabel;
    m_wgHint->setProperty("mute", true);
    m_wgHint->setWordWrap(true);
    wf->addRow(QString(), m_wgHint);
    root->addWidget(wg);

    // ------------------------------------------------------------- диэлектрик
    auto *di = new QGroupBox(tpl::tr("Диэлектрик", "Dielectric"));
    auto *df = new QFormLayout(di);
    m_eps = num(4.0, QString(), -1e4, 1e4, 4);
    m_freq = num(4.0, tpl::tr(" ГГц", " GHz"), 0.0001, 1e6, 4);
    df->addRow(tpl::tr("Проницаемость eps:", "Permittivity eps:"), m_eps);
    df->addRow(tpl::tr("На частоте:", "At frequency:"), m_freq);
    m_epsOut = new QLabel;
    m_epsOut->setWordWrap(true);
    m_epsOut->setTextInteractionFlags(Qt::TextSelectableByMouse);
    df->addRow(QString(), m_epsOut);
    root->addWidget(di);

    // ------------------------------------------------------------------ плазма
    auto *pl = new QGroupBox(tpl::tr("Плазма (X-мода)", "Plasma (X-mode)"));
    auto *pf = new QFormLayout(pl);
    m_n = new QLineEdit(QStringLiteral("3.1e+17"));
    m_fp = num(0.0, tpl::tr(" ГГц", " GHz"), 0.0, 1e6, 4);
    m_b = num(0.071, tpl::tr(" Тл", " T"), 0.0, 1e4, 6);
    m_fce = num(0.0, tpl::tr(" ГГц", " GHz"), 0.0, 1e6, 4);
    pf->addRow(tpl::tr("Концентрация N, м^-3:", "Density N, m^-3:"), m_n);
    pf->addRow(tpl::tr("Плазменная частота:", "Plasma frequency:"), m_fp);
    pf->addRow(tpl::tr("Подмагничивание B:", "Magnetization B:"), m_b);
    pf->addRow(tpl::tr("Циклотронная частота:", "Cyclotron frequency:"), m_fce);
    m_plasmaOut = new QLabel;
    m_plasmaOut->setWordWrap(true);
    m_plasmaOut->setTextInteractionFlags(Qt::TextSelectableByMouse);
    pf->addRow(QString(), m_plasmaOut);
    root->addWidget(pl);
    root->addStretch();

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(content);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    buttons->button(QDialogButtonBox::Close)->setText(tpl::tr("Закрыть", "Close"));
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *outer = new QVBoxLayout(this);
    outer->addWidget(scroll, 1);
    outer->addWidget(buttons);

    // --- связи: каждая пара двусторонняя ---
    connect(m_wgWidth, &QDoubleSpinBox::valueChanged, this, [this] { recalcWaveguide(true); });
    connect(m_wgEps, &QDoubleSpinBox::valueChanged, this, [this] { recalcWaveguide(true); });
    connect(m_fc10, &QDoubleSpinBox::valueChanged, this, [this] { recalcWaveguide(false); });
    connect(m_fc20, &QDoubleSpinBox::valueChanged, this, [this] { recalcWaveguide(false); });

    connect(m_eps, &QDoubleSpinBox::valueChanged, this, &CalculatorDialog::recalcDielectric);
    connect(m_freq, &QDoubleSpinBox::valueChanged, this, &CalculatorDialog::recalcDielectric);

    connect(m_n, &QLineEdit::editingFinished, this, [this] { recalcPlasma(true); });
    connect(m_b, &QDoubleSpinBox::valueChanged, this, [this] { recalcPlasma(true); });
    connect(m_fp, &QDoubleSpinBox::valueChanged, this, [this] { recalcPlasma(false); });
    connect(m_fce, &QDoubleSpinBox::valueChanged, this, [this] { recalcPlasma(false); });

    recalcWaveguide(true);
    recalcDielectric();
    recalcPlasma(true);
    dtheme::fitToScreen(this);
}

void CalculatorDialog::recalcWaveguide(bool fromWidth)
{
    if (m_busy)
        return;
    m_busy = true;

    const double eps = std::max(1e-6, m_wgEps->value());
    const double n = std::sqrt(eps);

    if (fromWidth) {
        const double a = std::max(1e-9, m_wgWidth->value());
        m_fc10->setValue(kC / (2.0 * a * n));
        m_fc20->setValue(kC / (a * n));
    } else {
        // Обратный ход: по нужной отсечке H10 считаем ширину.
        const double f = std::max(1e-9, m_fc10->value());
        const double a = kC / (2.0 * f * n);
        m_wgWidth->setValue(a);
        m_fc20->setValue(kC / (a * n));
    }

    m_wgHint->setText(tpl::tr("Свип держать МЕЖДУ отсечками: ниже H10 волна не пойдёт, выше H20 вход "
        "многомодовый и S-матрица там смысла не имеет. Заполнение диэлектриком "
        "опускает обе отсечки в sqrt(eps) = %1 раз.",
        "Keep the sweep BETWEEN the cutoffs: below H10 the wave does not propagate, "
        "above H20 the port is multimode and the S-matrix makes no sense there. "
        "Dielectric filling lowers both cutoffs by a factor of sqrt(eps) = %1.").arg(n, 0, 'f', 3));
    m_busy = false;
}

void CalculatorDialog::recalcDielectric()
{
    if (m_busy)
        return;
    m_busy = true;

    const double eps = m_eps->value();
    const double f = std::max(1e-9, m_freq->value());
    const double lambda0 = kC / f;

    QString s = tpl::tr("В файл задания уйдёт ДОБАВКА eps-1 = %1 — не сама eps.\n", "The task file gets the eps INCREMENT eps-1 = %1, not eps itself.\n")
                    .arg(eps - 1.0, 0, 'g', 6);
    s += tpl::tr("Длина волны в вакууме %1 мм.", "Free-space wavelength %1 mm.").arg(lambda0, 0, 'f', 3);
    if (eps > 0.0) {
        const double n = std::sqrt(eps);
        s += tpl::tr("\nПоказатель преломления %1, длина волны в среде %2 мм.", "\nRefractive index %1, wavelength in the medium %2 mm.")
                 .arg(n, 0, 'f', 4).arg(lambda0 / n, 0, 'f', 3);
    } else {
        s += tpl::tr("\neps <= 0: среда запредельная, волна в ней затухает, "
                     "а не распространяется. Это законный расчёт.",
                     "\neps <= 0: the medium is beyond cutoff, the wave decays in it instead of "
                     "propagating. This is a legitimate computation.");
    }
    m_epsOut->setText(s);
    m_epsOut->setStyleSheet(eps > 0.0 ? QStringLiteral("color:#9fb0c8;")
                                      : QStringLiteral("color:#e3b341;"));
    m_busy = false;
}

void CalculatorDialog::recalcPlasma(bool fromValues)
{
    if (m_busy)
        return;
    m_busy = true;

    double N = m_n->text().toDouble();
    double B = m_b->value();

    if (fromValues) {
        m_fp->setValue(tpl::plasmaFreqGHz(N));
        m_fce->setValue(tpl::cyclotronFreqGHz(B));
    } else {
        // Обратный ход: «хочу отсечку 5 ГГц — какая нужна концентрация».
        N = tpl::concentrationForFreqGHz(m_fp->value());
        m_n->setText(QString::number(N, 'g', 4));
        B = tpl::fieldForFreqGHz(m_fce->value());
        m_b->setValue(B);
    }

    // Полосы считаем ровно тем же кодом, что и валидатор задания.
    const tpl::PlasmaBands pb = tpl::plasmaBands(N, B);

    m_plasmaOut->setText(tpl::tr("Отсечки X-моды: нижняя f_L = %1 ГГц, верхняя f_R = %2 ГГц.\n"
        "Волна НЕ пройдёт при f < %1 и при %3 < f < %2 (%3 ГГц — верхний гибридный "
        "резонанс).\nБез подмагничивания отсечка была бы равна f_p = %4 ГГц — поле B "
        "опускает её до f_L.\nN пишется в задание в м^-3 (СИ), B — в Тесла: единицы "
        "N_UNIT и B_UNIT ядро разбирает, но НЕ применяет.",
        "X-mode cutoffs: lower f_L = %1 GHz, upper f_R = %2 GHz.\nThe wave does NOT "
        "propagate for f < %1 and for %3 < f < %2 (%3 GHz is the upper hybrid "
        "resonance).\nWithout magnetization the cutoff would be f_p = %4 GHz; the "
        "field B lowers it to f_L.\nN is written to the task in m^-3 (SI), B in tesla: "
        "the kernel parses the N_UNIT and B_UNIT units but does NOT apply them.")
        .arg(pb.fL, 0, 'f', 4).arg(pb.fR, 0, 'f', 4)
        .arg(pb.fUH, 0, 'f', 4).arg(pb.fp, 0, 'f', 4));
    m_busy = false;
}
