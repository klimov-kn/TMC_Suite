// Значки команд — описание в icons.h. Временные значки рисуются на холсте 24×24.
/// @file icons.cpp
/// @brief Значки команд: файлы оформления из ресурсов или простые рисованные значки 24 x 24.
///
/// Часть модуля FieldView (слой `ui`). Описание функций — в `icons.h`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#dialogs
#include "icons.h"
#include "theme.h"

#include <QFile>
#include <QHash>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

#include <cmath>

namespace fieldview {

namespace {

/// @name Цвета рисованных значков
///@{
/// @brief Возвращает цвет группы команд: поле, слой eps, топология, обычный.
/// @return Цвет оформления.
const QColor cField() { return theme::accent(); }    // поле — циан
const QColor cEps() { return theme::accent3(); }     // ε — янтарный
const QColor cTopo() { return theme::accent2(); }    // топология — фиолетовый
const QColor cInk() { return theme::text(); }
///@}

/// @brief Рисует сетку 4 x 4 линии (команды «Сетка»/«Контуры»).
/// @param[in,out] p  рисовальщик (холст 24 x 24)
/// @param[in]     c  цвет
void grid(QPainter &p, const QColor &c)
{
    p.setPen(QPen(c, 1.4));
    for (int i = 0; i < 4; ++i) {
        p.drawLine(QPointF(5, 5 + i * 4.67), QPointF(19, 5 + i * 4.67));
        p.drawLine(QPointF(5 + i * 4.67, 5), QPointF(5 + i * 4.67, 19));
    }
}

/// @brief Рисует залитую волнистую поверхность (команды «Заливка»).
/// @param[in,out] p  рисовальщик
/// @param[in]     c  цвет
void surface(QPainter &p, const QColor &c)
{
    QLinearGradient g(4, 20, 20, 4);
    g.setColorAt(0, c.darker(250));
    g.setColorAt(1, c);
    p.setPen(QPen(c, 1.2));
    p.setBrush(g);
    QPainterPath path;
    path.moveTo(4, 17);
    path.cubicTo(8, 6, 12, 22, 20, 8);
    path.lineTo(20, 20);
    path.lineTo(4, 20);
    path.closeSubpath();
    p.drawPath(path);
}

/// @brief Рисует каркас куба (команды 2D/3D).
/// @param[in,out] p  рисовальщик
/// @param[in]     c  цвет
void cube(QPainter &p, const QColor &c)
{
    p.setPen(QPen(c, 1.5));
    p.setBrush(Qt::NoBrush);
    p.drawRect(QRectF(4, 9, 11, 11));
    p.drawLine(QPointF(4, 9), QPointF(9, 4));
    p.drawLine(QPointF(15, 9), QPointF(20, 4));
    p.drawLine(QPointF(15, 20), QPointF(20, 15));
    p.drawLine(QPointF(9, 4), QPointF(20, 4));
    p.drawLine(QPointF(20, 4), QPointF(20, 15));
}

/// @brief Пишет текст полужирным шрифтом по центру прямоугольника.
/// @param[in,out] p   рисовальщик
/// @param[in]     t   текст
/// @param[in]     c   цвет
/// @param[in]     px  размер шрифта, пиксели
/// @param[in]     r   прямоугольник на холсте 24 x 24
void letter(QPainter &p, const QString &t, const QColor &c, int px = 13, const QRectF &r = QRectF(0, 0, 24, 24))
{
    QFont f = theme::uiFont(px, QFont::Bold);
    p.setFont(f);
    p.setPen(c);
    p.drawText(r, Qt::AlignCenter, t);
}

/// @brief Рисует лупу со знаком «+» или «-» и, при необходимости, буквой оси (команды Zoom).
/// @param[in,out] p     рисовальщик
/// @param[in]     sign  «+» или «-»
/// @param[in]     axis  «X», «Y» или пусто
void magnifier(QPainter &p, const QString &sign, const QString &axis)
{
    p.setPen(QPen(cInk(), 1.8));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QPointF(10, 10), 6, 6);
    p.drawLine(QPointF(14.5, 14.5), QPointF(20, 20));
    p.setPen(QPen(cField(), 1.8));
    p.drawLine(QPointF(7, 10), QPointF(13, 10));
    if (sign == QLatin1String("+"))
        p.drawLine(QPointF(10, 7), QPointF(10, 13));
    if (!axis.isEmpty())
        letter(p, axis, cEps(), 9, QRectF(14, 0, 10, 10));
}

/// @brief Рисует размерную стрелку с засечками.
/// @param[in,out] p           рисовальщик
/// @param[in]     horizontal  `true` — горизонтальная
/// @param[in]     c           цвет
void dimArrow(QPainter &p, bool horizontal, const QColor &c)
{
    p.setPen(QPen(c, 1.6));
    p.setBrush(c);
    if (horizontal) {
        p.drawLine(QPointF(4, 12), QPointF(20, 12));
        p.drawLine(QPointF(4, 7), QPointF(4, 17));
        p.drawLine(QPointF(20, 7), QPointF(20, 17));
        p.drawPolygon(QPolygonF({QPointF(4.5, 12), QPointF(8.5, 9.5), QPointF(8.5, 14.5)}));
        p.drawPolygon(QPolygonF({QPointF(19.5, 12), QPointF(15.5, 9.5), QPointF(15.5, 14.5)}));
    } else {
        p.drawLine(QPointF(12, 4), QPointF(12, 20));
        p.drawLine(QPointF(7, 4), QPointF(17, 4));
        p.drawLine(QPointF(7, 20), QPointF(17, 20));
        p.drawPolygon(QPolygonF({QPointF(12, 4.5), QPointF(9.5, 8.5), QPointF(14.5, 8.5)}));
        p.drawPolygon(QPolygonF({QPointF(12, 19.5), QPointF(9.5, 15.5), QPointF(14.5, 15.5)}));
    }
}

/// @brief Рисует полупрозрачный квадрат цвета материала с буквой (команды типов границ).
/// @param[in,out] p  рисовальщик
/// @param[in]     c  цвет материала
/// @param[in]     t  буква
void boundary(QPainter &p, const QColor &c, const QString &t)
{
    p.setPen(QPen(c.lighter(130), 1.2));
    QColor f = c;
    f.setAlphaF(0.55f);
    p.setBrush(f);
    p.drawRoundedRect(QRectF(4, 4, 16, 16), 3, 3);
    letter(p, t, cInk(), 11);
}

/// @brief Рисует треугольник «пуск».
/// @param[in,out] p  рисовальщик
/// @param[in]     c  цвет
void play(QPainter &p, const QColor &c)
{
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    p.drawPolygon(QPolygonF({QPointF(7, 4), QPointF(20, 12), QPointF(7, 20)}));
}

/// @brief Рисует значок команды на холсте 24 x 24; для неизвестной команды — две буквы из конца идентификатора.
/// @param[in,out] p   рисовальщик
/// @param[in]     id  идентификатор команды
void paintIcon(QPainter &p, const QString &id)
{
    p.setRenderHint(QPainter::Antialiasing);
    if (id == QLatin1String("ID_FILE_OPEN")) {
        p.setPen(QPen(cInk(), 1.4));
        p.setBrush(theme::surface2());
        p.drawRoundedRect(QRectF(3, 6, 18, 13), 2, 2);
        p.setBrush(cField());
        p.drawPolygon(QPolygonF({QPointF(5, 19), QPointF(8, 10), QPointF(22, 10), QPointF(19, 19)}));
    } else if (id == QLatin1String("ID_VIEW_ZOOM_P")) {
        magnifier(p, QStringLiteral("+"), QString());
    } else if (id == QLatin1String("ID_VIEW_ZOOM_PX")) {
        magnifier(p, QStringLiteral("+"), QStringLiteral("X"));
    } else if (id == QLatin1String("ID_VIEW_ZOOM_PY")) {
        magnifier(p, QStringLiteral("+"), QStringLiteral("Y"));
    } else if (id == QLatin1String("ID_VIEW_ZOOM_M")) {
        magnifier(p, QStringLiteral("-"), QString());
    } else if (id == QLatin1String("ID_VIEW_ZOOM_MX")) {
        magnifier(p, QStringLiteral("-"), QStringLiteral("X"));
    } else if (id == QLatin1String("ID_VIEW_ZOOM_MY")) {
        magnifier(p, QStringLiteral("-"), QStringLiteral("Y"));
    } else if (id == QLatin1String("ID_FLAGDRAWLINETOP")) {
        grid(p, cTopo());
    } else if (id == QLatin1String("ID_FLAGDRAWSURFACETOP")) {
        surface(p, cTopo());
    } else if (id == QLatin1String("ID_DIMENSION23SWITCHTOP")) {
        cube(p, cTopo());
    } else if (id == QLatin1String("ID_FLAGDRAWLINEEPS")) {
        grid(p, cEps());
    } else if (id == QLatin1String("ID_FLAGDRAWSURFACEEPS")) {
        surface(p, cEps());
    } else if (id == QLatin1String("ID_DIMENSION23SWITCHEPS")) {
        cube(p, cEps());
    } else if (id == QLatin1String("ID_FLAGDRAWLINE")) {
        grid(p, cField());
    } else if (id == QLatin1String("ID_FLAGDRAWSURFACE")) {
        surface(p, cField());
    } else if (id == QLatin1String("ID_DIMENSION23SWITCH")) {
        cube(p, cField());
    } else if (id == QLatin1String("ID_VIEW_TOPOLOGY_BOUNDARY_EPS")) {
        boundary(p, QColor(0xb0, 0x7c, 0xff), QStringLiteral("E"));
    } else if (id == QLatin1String("ID_VIEW_TOPOLOGY_BOUNDARY_METAL")) {
        boundary(p, QColor(0x9f, 0xb0, 0xc8), QStringLiteral("M"));
    } else if (id == QLatin1String("ID_VIEW_TOPOLOGY_BOUNDARY_MAGNETIC")) {
        boundary(p, QColor(0x6c, 0x7d, 0x97), QStringLiteral("N"));
    } else if (id == QLatin1String("ID_VIEW_TOPOLOGY_BOUNDARY_ABSORBER")) {
        boundary(p, QColor(0x3a, 0x4a, 0x6b), QStringLiteral("A"));
    } else if (id == QLatin1String("ID_VIEW_TOPOLOGY_BOUNDARY_INPUT")) {
        boundary(p, QColor(0x34, 0xe3, 0xc4), QStringLiteral("I"));
    } else if (id == QLatin1String("ID_VIEW_TOPOLOGY_BOUNDARY_PLASMA")) {
        boundary(p, QColor(0xff, 0xcf, 0x5c), QStringLiteral("P"));
    } else if (id == QLatin1String("ID_VIEW_TOPOLOGY_SIZE_BLOCK")) {
        p.setPen(QPen(cTopo(), 1.4));
        p.setBrush(Qt::NoBrush);
        p.drawRect(QRectF(6, 9, 12, 11));
        dimArrow(p, true, cEps());
        p.fillRect(QRectF(0, 13, 24, 11), Qt::transparent);
    } else if (id == QLatin1String("ID_VIEW_TOPOLOGY_SIZE_LINKLIST")) {
        p.setPen(QPen(cTopo(), 1.4));
        p.drawLine(QPointF(4, 20), QPointF(4, 4));
        p.drawLine(QPointF(4, 20), QPointF(20, 20));
        p.setPen(QPen(cEps(), 1.4, Qt::DashLine));
        p.drawLine(QPointF(4, 10), QPointF(15, 10));
        p.drawLine(QPointF(15, 10), QPointF(15, 20));
        p.setBrush(cEps());
        p.drawEllipse(QPointF(15, 10), 2.5, 2.5);
    } else if (id == QLatin1String("ID_VIEW_EPS_SIZE")) {
        dimArrow(p, false, cEps());
    } else if (id == QLatin1String("ID_VIEW_FIELD_SIZE")) {
        dimArrow(p, false, cField());
    } else if (id == QLatin1String("ID_VIEW_TOPOLOGY_DIMENSIONS_ALONGANXAXIES")) {
        dimArrow(p, true, cInk());
    } else if (id == QLatin1String("ID_VIEW_TOPOLOGY_DIMENSIONS_ALONGANYAXIES")) {
        dimArrow(p, false, cInk());
    } else if (id == QLatin1String("ID_EPS_TYPE_EPS")) {
        letter(p, QStringLiteral("ε"), cEps(), 16);
    } else if (id == QLatin1String("ID_EPS_TYPE_N")) {
        letter(p, QStringLiteral("N"), QColor(0xff, 0x6b, 0x6b), 14);
    } else if (id == QLatin1String("ID_EPS_TYPE_B")) {
        letter(p, QStringLiteral("B"), QColor(0x6a, 0xa8, 0xff), 14);
    } else if (id == QLatin1String("ID_EPS_TYPE_Y")) {
        letter(p, QStringLiteral("Y"), theme::ok(), 14);
    } else if (id == QLatin1String("ID_VIEW_PROPORTIONALLY")) {
        p.setPen(QPen(cInk(), 1.4));
        p.drawRect(QRectF(4, 4, 16, 16));
        letter(p, QStringLiteral("1:1"), cField(), 8);
    } else if (id == QLatin1String("ID_VIEW_DEFAULT12")) {
        p.setPen(QPen(cField(), 1.8));
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(5, 5, 14, 14), 60 * 16, 290 * 16);
        p.setBrush(cField());
        p.drawPolygon(QPolygonF({QPointF(15, 3), QPointF(19, 7.5), QPointF(13.5, 8.5)}));
    } else if (id == QLatin1String("ID_FILE_PRINT")) {
        p.setPen(QPen(cInk(), 1.4));
        p.setBrush(theme::surface2());
        p.drawRect(QRectF(7, 3, 10, 6));
        p.drawRoundedRect(QRectF(3, 9, 18, 8), 2, 2);
        p.setBrush(theme::text());
        p.drawRect(QRectF(7, 14, 10, 7));
    } else if (id == QLatin1String("ID_APP_ABOUT")) {
        p.setPen(QPen(cField(), 1.6));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(12, 12), 9, 9);
        letter(p, QStringLiteral("i"), cField(), 13);
    } else if (id == QLatin1String("ID_VIEW_PARAMETERS")) {
        p.setPen(QPen(cInk(), 1.6));
        for (int i = 0; i < 3; ++i)
            p.drawLine(QPointF(4, 6 + i * 6), QPointF(20, 6 + i * 6));
        p.setBrush(cField());
        p.drawEllipse(QPointF(9, 6), 2.3, 2.3);
        p.drawEllipse(QPointF(16, 12), 2.3, 2.3);
        p.drawEllipse(QPointF(7, 18), 2.3, 2.3);
    } else if (id == QLatin1String("ID_FIELD_MODUL")) {
        letter(p, QStringLiteral("|E|"), cField(), 11);
    } else if (id == QLatin1String("ID_FIELD_VALUE")) {
        p.setPen(QPen(cEps(), 1.6));
        p.drawLine(QPointF(12, 3), QPointF(12, 21));
        p.drawLine(QPointF(3, 12), QPointF(21, 12));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(12, 12), 4.5, 4.5);
    } else if (id == QLatin1String("ID_FIELD_L")) {
        for (int i = 0; i < 5; ++i) {
            QColor c = cField();
            c.setAlphaF(float(0.25 + i * 0.18));
            p.fillRect(QRectF(5, 19 - i * 3.6, 14, 3), c);
        }
    } else if (id == QLatin1String("ID_ANIM_PLAY")) {
        play(p, cField());
    } else if (id == QLatin1String("ID_ANIM_PAUSE")) {
        p.fillRect(QRectF(6, 4, 4, 16), cField());
        p.fillRect(QRectF(14, 4, 4, 16), cField());
    } else if (id == QLatin1String("ID_ANIM_PREV") || id == QLatin1String("ID_ANIM_FIRST")) {
        p.setPen(Qt::NoPen);
        p.setBrush(cInk());
        p.drawPolygon(QPolygonF({QPointF(17, 5), QPointF(8, 12), QPointF(17, 19)}));
        p.fillRect(QRectF(5, 5, 2.5, 14), id == QLatin1String("ID_ANIM_FIRST") ? cInk() : Qt::transparent);
    } else if (id == QLatin1String("ID_ANIM_NEXT") || id == QLatin1String("ID_ANIM_LAST")) {
        p.setPen(Qt::NoPen);
        p.setBrush(cInk());
        p.drawPolygon(QPolygonF({QPointF(7, 5), QPointF(16, 12), QPointF(7, 19)}));
        p.fillRect(QRectF(16.5, 5, 2.5, 14), id == QLatin1String("ID_ANIM_LAST") ? cInk() : Qt::transparent);
    } else if (id == QLatin1String("ID_SYNC_KERNEL")) {
        // Индикатор пакета: зелёный кружок — следит, красный — нет (рисуется в состоянии).
        p.setPen(QPen(cField(), 1.6));
        QPainterPath w;
        w.moveTo(3, 12);
        for (int x = 3; x <= 21; ++x)
            w.lineTo(x, 12 - 5 * std::sin((x - 3) / 18.0 * 2 * 3.14159));
        p.drawPath(w);
    } else if (id == QLatin1String("ID_EXPORT_ANIMATION")) {
        p.setPen(QPen(cInk(), 1.4));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(3, 5, 18, 14), 2, 2);
        play(p, cField());
    } else if (id == QLatin1String("ID_FILE_SAVE_IMAGE")) {
        p.setPen(QPen(cInk(), 1.4));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(3, 4, 18, 16), 2, 2);
        p.setPen(Qt::NoPen);
        p.setBrush(cField());
        p.drawPolygon(QPolygonF({QPointF(5, 18), QPointF(10, 11), QPointF(14, 15), QPointF(16, 13), QPointF(19, 18)}));
        p.setBrush(cEps());
        p.drawEllipse(QPointF(16, 8), 2, 2);
    } else {
        letter(p, id.mid(id.lastIndexOf(QLatin1Char('_')) + 1).left(2), cInk(), 10);
    }
}

/// @brief Рисует значок команды в картинку заданного размера.
/// @param[in] id        идентификатор команды
/// @param[in] size      размер, логические пиксели
/// @param[in] dpr       плотность точек
/// @param[in] disabled  `true` — вид недоступной команды (непрозрачность 0,32)
/// @return Картинка значка.
QPixmap render(const QString &id, int size, qreal dpr, bool disabled)
{
    QPixmap pm(int(size * dpr), int(size * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.scale(size / 24.0, size / 24.0);
    if (disabled)
        p.setOpacity(0.32);
    paintIcon(p, id);
    return pm;
}

} // namespace

QIcon commandIcon(const QString &id)
{
    static QHash<QString, QIcon> cache;
    const auto it = cache.constFind(id);
    if (it != cache.constEnd())
        return *it;
    QIcon icon;
    for (const char *ext : {".svg", ".png"}) {
        const QString path = QStringLiteral(":/toolbar/") + id + QLatin1String(ext);
        if (QFile::exists(path)) {
            icon = QIcon(path);
            break;
        }
    }
    if (icon.isNull()) {
        for (qreal dpr : {1.0, 1.5, 2.0}) {
            icon.addPixmap(render(id, 20, dpr, false), QIcon::Normal);
            icon.addPixmap(render(id, 20, dpr, true), QIcon::Disabled);
        }
    }
    cache.insert(id, icon);
    return icon;
}

QIcon appIcon()
{
    if (QFile::exists(QStringLiteral(":/app/fieldview.png")))
        return QIcon(QStringLiteral(":/app/fieldview.png"));
    QIcon icon;
    for (int s : {16, 24, 32, 48, 64, 128, 256}) {
        QPixmap pm(s, s);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        p.scale(s / 64.0, s / 64.0);
        p.setPen(Qt::NoPen);
        p.setBrush(theme::surface());
        p.drawRoundedRect(QRectF(2, 2, 60, 60), 14, 14);
        // Волна над сеткой — рельеф поля.
        for (int k = 0; k < 6; ++k) {
            QPainterPath w;
            const double y0 = 18 + k * 6.5;
            w.moveTo(10, y0);
            for (int x = 10; x <= 54; x += 2)
                w.lineTo(x, y0 - 6 * std::sin((x - 10) / 44.0 * 2 * 3.14159 + k * 0.6) * (1.0 - k * 0.1));
            QColor c = k % 2 ? theme::accent2() : theme::accent();
            p.setPen(QPen(c, 2.6, Qt::SolidLine, Qt::RoundCap));
            p.setBrush(Qt::NoBrush);
            p.drawPath(w);
        }
        p.end();
        icon.addPixmap(pm);
    }
    return icon;
}

} // namespace fieldview
