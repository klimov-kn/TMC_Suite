// Окно «Цвета уровней» — описание в autocolordialog.h.
/// @file autocolordialog.cpp
/// @brief Окно «Цвета уровней»: два столбца уровней, выбор цвета, автоматическое заполнение.
///
/// Часть модуля FieldView (слой `ui`). Описание класса — в `autocolordialog.h`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#dialogs
#include "autocolordialog.h"
#include "theme.h"

#include <QButtonGroup>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

#include <algorithm>
#include <cstdlib>

namespace fieldview {

namespace {

/// @brief Рисует образец цвета — скруглённый прямоугольник с рамкой.
/// @param[in] c  цвет
/// @param[in] w  ширина, точек
/// @param[in] h  высота, точек
/// @return Картинка образца.
QPixmap swatch(const QColor &c, int w, int h)
{
    QPixmap pm(w, h);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(theme::line(), 1));
    p.setBrush(c);
    p.drawRoundedRect(QRectF(0.5, 0.5, w - 1, h - 1), 4, 4);
    return pm;
}

} // namespace

AutoColorDialog::AutoColorDialog(const Colors20 &colors, const Colors20 &classic, QWidget *parent)
    : QDialog(parent), m_c(colors), m_classic(classic)
{
    setWindowTitle(QStringLiteral("Цвета уровней (Auto color)"));
    auto *root = new QHBoxLayout(this);
    m_g1 = new QButtonGroup(this);
    m_g2 = new QButtonGroup(this);
    const auto column = [&](QButtonGroup *g, QLabel **sw, const QString &title, int which) {
        auto *box = new QGroupBox(title);
        auto *grid = new QGridLayout(box);
        grid->setHorizontalSpacing(8);
        grid->setVerticalSpacing(3);
        for (int i = 0; i < 20; ++i) {
            const int col = (i / 10) * 2, row = i % 10;
            sw[i] = new QLabel;
            auto *rb = new QRadioButton(QStringLiteral("Уровень %1").arg(i + 1));
            g->addButton(rb, i);
            grid->addWidget(sw[i], row, col);
            grid->addWidget(rb, row, col + 1);
        }
        g->button(which)->setChecked(true);
        auto *chg = new QPushButton(QStringLiteral("Изменить…"));
        grid->addWidget(chg, 10, 0, 1, 4);
        root->addWidget(box);
        return chg;
    };
    QPushButton *c1 = column(m_g1, m_sw1, QStringLiteral("Первый уровень"), 0);
    QPushButton *c2 = column(m_g2, m_sw2, QStringLiteral("Второй уровень"), 19);
    connect(c1, &QPushButton::clicked, this, [this]() { change(1); });
    connect(c2, &QPushButton::clicked, this, [this]() { change(2); });

    auto *side = new QVBoxLayout;
    m_all = new QLabel;
    m_all->setMinimumSize(110, 120);
    side->addWidget(m_all);
    const auto btn = [&](const QString &t, auto fn) {
        auto *b = new QPushButton(t);
        connect(b, &QPushButton::clicked, this, fn);
        side->addWidget(b);
    };
    btn(QStringLiteral("Авто"), [this]() { autoSet(0); });
    btn(QStringLiteral("Авто через красный"), [this]() { autoSet(1); });
    btn(QStringLiteral("Авто через зелёный"), [this]() { autoSet(2); });
    btn(QStringLiteral("Авто через синий"), [this]() { autoSet(3); });
    side->addSpacing(10);
    auto *hint = new QLabel(QStringLiteral("Заполнить из шкалы:"));
    hint->setProperty("dim", true);
    side->addWidget(hint);
    btn(QStringLiteral("«Волна»"), [this]() { m_c = sampleColors(ScaleKind::Wave); refresh(); });
    btn(QStringLiteral("«Свечение»"), [this]() { m_c = sampleColors(ScaleKind::Glow); refresh(); });
    btn(QStringLiteral("Исходные (FieldView 4.1)"), [this]() { m_c = m_classic; refresh(); });
    side->addStretch(1);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setText(QStringLiteral("ОК"));
    bb->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Отмена"));
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    side->addWidget(bb);
    root->addLayout(side);
    refresh();
}

void AutoColorDialog::refresh()
{
    for (int i = 0; i < 20; ++i) {
        m_sw1[i]->setPixmap(swatch(m_c[size_t(i)], 30, 15));
        m_sw2[i]->setPixmap(swatch(m_c[size_t(i)], 30, 15));
    }
    // Итоговая шкала (IDC_STATICLEVEL_COLORALL): снизу уровень 1, сверху 20.
    QPixmap pm(110, 120);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    for (int i = 0; i < 20; ++i)
        p.fillRect(QRectF(20, 120 - (i + 1) * 6, 70, 6), m_c[size_t(i)]);
    p.setPen(QPen(theme::line(), 1));
    p.drawRect(QRectF(20, 0, 70, 119));
    p.end();
    m_all->setPixmap(pm);
}

void AutoColorDialog::change(int group)
{
    const int n = (group == 1 ? m_g1 : m_g2)->checkedId();
    if (n < 0 || n > 19)
        return;
    const QColor c = QColorDialog::getColor(m_c[size_t(n)], this, QStringLiteral("Цвет уровня %1").arg(n + 1));
    if (c.isValid()) {
        m_c[size_t(n)] = c;
        refresh();
    }
}

void AutoColorDialog::autoSet(int mode)
{
    // OnColorLevelAutoSet / AutoSet2 / 3 / 4 — те же формулы (целочисленные шаги).
    int n1 = m_g1->checkedId(), n2 = m_g2->checkedId();
    if (n1 < 0 || n1 > 19 || n2 < 0 || n2 > 19 || std::abs(n1 - n2) < 2)
        return;
    if (n1 > n2)
        std::swap(n1, n2);
    const QColor a = m_c[size_t(n1)], b = m_c[size_t(n2)];
    int d[3] = {(b.red() - a.red()) / (n2 - n1), (b.green() - a.green()) / (n2 - n1), (b.blue() - a.blue()) / (n2 - n1)};
    int v[3] = {a.red(), a.green(), a.blue()};
    const int ch = mode - 1;   // через какой канал (0 R, 1 G, 2 B)
    const int target[3] = {b.red(), b.green(), b.blue()};
    if (ch >= 0)
        d[ch] = 2 * (256 - v[ch]) / (n2 - n1);
    for (int k = 0; k < 3; ++k)
        v[k] += d[k];
    int i = n1 + 1;
    const int mid = ch >= 0 ? (n1 + n2) / 2 : n2;
    for (; i < mid; ++i) {
        m_c[size_t(i)] = QColor(std::clamp(v[0], 0, 255), std::clamp(v[1], 0, 255), std::clamp(v[2], 0, 255));
        for (int k = 0; k < 3; ++k)
            v[k] += d[k];
    }
    if (ch >= 0) {
        d[ch] = 2 * (target[ch] - 256) / (n2 - n1);
        for (; i < n2; ++i) {
            m_c[size_t(i)] = QColor(std::clamp(v[0], 0, 255), std::clamp(v[1], 0, 255), std::clamp(v[2], 0, 255));
            for (int k = 0; k < 3; ++k)
                v[k] += d[k];
        }
    }
    refresh();
}

} // namespace fieldview
