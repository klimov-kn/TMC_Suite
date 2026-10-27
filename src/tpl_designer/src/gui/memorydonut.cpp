/// @file memorydonut.cpp
/// @brief Рисование кольцевой диаграммы памяти `MemoryDonut`.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-memory
#include "memorydonut.h"

#include "ramestimate.h"
#include "dtheme.h"

#include <QPainter>
#include <QPainterPath>

#include <algorithm>
#include <cmath>
#include "lang.h"

namespace {

// Цвета секторов. Голубой — память компьютера (кольцо и занятое), бирюзовый — сетка
// расчёта (цвет акцента программы), фиолетовый — диаграмма направленности.
/// Цвет памяти компьютера (кольцо и «занято»).
const QColor kRam{0x5f, 0xb4, 0xff};
/// Цвет сектора сетки расчёта.
const QColor kGrid{0x34, 0xe3, 0xc4};
/// Цвет сектора диаграммы направленности.
const QColor kPattern{0xb4, 0x8c, 0xff};

/// Сектор кольца между радиусами: от угла \p start (градусы, 0 — «три часа», против
/// часовой) на \p span (отрицательный — по часовой).
/// @brief Строит контур сектора кольца.
/// @param[in] outer      внешний квадрат кольца, пиксели.
/// @param[in] thickness  толщина кольца, пиксели.
/// @param[in] start      начальный угол, градусы.
/// @param[in] span       угловой размер, градусы; отрицательный — по часовой стрелке.
/// @return Замкнутый контур сектора.
QPainterPath ringSector(const QRectF &outer, double thickness, double start, double span)
{
    const QRectF inner = outer.adjusted(thickness, thickness, -thickness, -thickness);
    QPainterPath path;
    path.arcMoveTo(outer, start);
    path.arcTo(outer, start, span);
    path.arcTo(inner, start + span, -span);
    path.closeSubpath();
    return path;
}

/// Заштрихованный сектор: полупрозрачная подложка и косая штриховка поверх.
/// @brief Заливает контур подложкой, косой штриховкой (вдвое крупнее стандартной) и обводит его.
/// @param[in,out] p     рисовальщик виджета.
/// @param[in]     path  контур.
/// @param[in]     c     цвет штриховки и обводки; подложка — тот же цвет с прозрачностью 70.
void hatched(QPainter &p, const QPainterPath &path, const QColor &c)
{
    QColor fill = c;
    fill.setAlpha(70);
    p.fillPath(path, fill);
    QBrush hatch(c, Qt::BDiagPattern);
    // Штриховка Qt — в пикселях устройства, на HiDPI она мельчит. Масштабируем.
    QTransform t;
    t.scale(2.0, 2.0);
    hatch.setTransform(t);
    p.fillPath(path, hatch);
    p.setPen(QPen(c, 1.2));
    p.drawPath(path);
}

} // namespace

MemoryDonut::MemoryDonut(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("memoryDonut"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void MemoryDonut::setData(const Data &d)
{
    m_data = d;
    update();
}

QSize MemoryDonut::sizeHint() const { return {520, 176}; }
QSize MemoryDonut::minimumSizeHint() const { return {360, 176}; }

void MemoryDonut::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const Data &d = m_data;
    const double side = std::min<double>(height() - 12, 164);
    const QRectF outer(8, (height() - side) / 2.0, side, side);
    const double thick = side * 0.17;

    // Всё кольцо — вся память. Если её узнать не удалось, кольцо — это сам расчёт.
    const double total = d.known ? d.total : std::max(d.task(), 1.0);
    const double perDeg = total / 360.0;
    const auto deg = [&](double bytes) {
        if (bytes <= 0.0)
            return 0.0;
        // Сто мегабайт на 64 ГБ — это доля градуса, сектор пропал бы. Даём ему хотя
        // бы полтора градуса, чтобы было видно, ЧТО он есть; точное число — в подписи.
        return std::max(1.5, bytes / perDeg);
    };

    QColor ring = kRam;
    ring.setAlpha(45);
    p.fillPath(ringSector(outer, thick, 90.0, -360.0), ring);

    double at = 90.0; // от «двенадцати часов» по часовой стрелке
    if (d.known) {
        const double u = std::min(360.0, deg(d.used));
        p.fillPath(ringSector(outer, thick, at, -u), kRam);
        at -= u;
    }

    const bool over = !d.fits();
    const double room = d.known ? std::max(0.0, 360.0 - (90.0 - at)) : 360.0;
    const double gDeg = std::min(room, deg(d.grid));
    hatched(p, ringSector(outer, thick, at, -gDeg), over ? dtheme::danger() : kGrid);
    at -= gDeg;
    if (d.patternOn && d.pattern > 0.0) {
        const double pDeg = std::min(std::max(0.0, room - gDeg), deg(d.pattern));
        if (pDeg > 0.0)
            hatched(p, ringSector(outer, thick, at, -pDeg), over ? dtheme::danger() : kPattern);
    }

    // Перебор: снаружи кольца красная дуга на ту долю ОЗУ, которой не хватит.
    if (over) {
        const double lack = d.used + d.task() - d.total;
        const double span = std::min(360.0, lack / perDeg);
        const QRectF halo = outer.adjusted(-5, -5, 5, 5);
        p.setPen(QPen(dtheme::danger(), 3.0, Qt::SolidLine, Qt::FlatCap));
        p.drawArc(halo, int(90.0 * 16), int(-span * 16));
    }

    // В середине — объём ОЗУ.
    p.setPen(dtheme::text());
    QFont big = font();
    big.setPointSizeF(font().pointSizeF() * 1.35);
    big.setBold(true);
    p.setFont(big);
    const QRectF mid = outer.adjusted(thick, thick, -thick, -thick);
    p.drawText(mid.adjusted(0, 0, 0, -mid.height() * 0.18), Qt::AlignCenter,
               d.known ? tpl::formatBytes(d.total) : QStringLiteral("?"));
    QFont small = font();
    small.setPointSizeF(font().pointSizeF() * 0.85);
    p.setFont(small);
    p.setPen(dtheme::textMute());
    p.drawText(mid.adjusted(0, mid.height() * 0.30, 0, 0), Qt::AlignCenter,
               tpl::tr("ОЗУ этого ПК", "RAM of this PC"));

    // Подписи справа.
    p.setFont(font());
    const QFontMetricsF fm(font());
    const double x0 = outer.right() + 22;
    double y = outer.top() + 10;
    const double row = fm.height() * 1.55;
    const auto legend = [&](const QColor &c, bool hatch, const QString &title, double bytes) {
        const QRectF sw(x0, y - fm.ascent() + 1, 14, 14);
        if (hatch) {
            QPainterPath path;
            path.addRoundedRect(sw, 2, 2);
            hatched(p, path, c);
        } else {
            p.fillRect(sw, c);
            // Тёмный образец «свободно» сливался с фоном — обводим его.
            p.setPen(QPen(kRam, 1.0));
            p.drawRect(sw.adjusted(0.5, 0.5, -0.5, -0.5));
        }
        // Число — по правому краю, подпись — сколько останется места, с многоточием:
        // при узком окне длинная подпись иначе наезжала на число.
        const QString value = tpl::formatBytes(bytes);
        const double vw = fm.horizontalAdvance(value);
        const double vx = double(width()) - 8 - vw;
        const double room = std::max(20.0, vx - 12 - (x0 + 24));
        p.setPen(dtheme::text());
        p.drawText(QPointF(x0 + 24, y), fm.elidedText(title, Qt::ElideRight, room));
        p.drawText(QPointF(vx, y), value);
        y += row;
    };
    if (d.known)
        legend(kRam, false, tpl::tr("Занято системой", "Used by system"), d.used);
    legend(over ? dtheme::danger() : kGrid, true, tpl::tr("Расчёт: сетка ядра", "Run: kernel grid"), d.grid);
    if (d.patternOn) {
        QString t = tpl::tr("ДН", "Pattern");
        QStringList how;
        if (d.patternTones > 1)
            how << tpl::tr("%1 част.", "%1 freq.").arg(d.patternTones);
        how << (d.patternBorder ? tpl::tr("по краю", "border only") : tpl::tr("вся область", "whole domain"));
        t += QStringLiteral(": ") + how.join(QStringLiteral(", "));
        legend(over ? dtheme::danger() : kPattern, true, t, d.pattern);
    }
    if (d.known) {
        QColor freeC = kRam;
        freeC.setAlpha(45);
        const double left = d.total - d.used - d.task();
        legend(left >= 0 ? freeC : dtheme::danger(), false,
               left >= 0 ? tpl::tr("Останется свободно", "Will remain free") : tpl::tr("Не хватит", "Not enough"),
               std::fabs(left));
    }
}
