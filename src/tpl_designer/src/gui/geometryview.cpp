/// @file geometryview.cpp
/// @brief Холст редактора геометрии: привязка, разбор попаданий, рисование слоёв,
///        инструменты рисования, ручки, поворот, соединение и скругление фигур.
///
/// Часть интерфейса TPL Designer. Объявления и описание методов — в `geometryview.h`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-geometry
#include "geometryview.h"

#include "expr.h"
#include "placement.h"
#include "settings.h"
#include "dtheme.h"

#include <QContextMenuEvent>
#include <QFontMetrics>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QScrollBar>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include "lang.h"

using namespace tpl;

namespace {

/// Радиус кружка порта в пикселях экрана — от зума не зависит, чтобы метка
/// не превращалась в точку и не заслоняла геометрию.
constexpr double kPortMarkerPx = 11.0;
/// Насколько близко к линии нужно ткнуть, чтобы попасть, пикселей.
constexpr double kPickPx = 7.0;
/// Сторона квадратика-ручки, пикселей.
constexpr double kHandlePx = 9.0;
/// Ширина полосы линейки, пикселей.
constexpr double kRulerPx = 22.0;

/// На таком расстоянии от вершины или линии мышь к ней прилипает, пикселей.
constexpr double kSnapPx = 11.0;

constexpr int kRoleVertex = 100; ///< + номер вершины
/// Роль ручки начала апертуры порта (`Port::from`).
constexpr int kRolePortFrom = 200;
/// Роль ручки конца апертуры порта (`Port::to`).
constexpr int kRolePortTo = 201;
/// Роль ручки поворота.
constexpr int kRoleRotate = 300;

/// @brief Возвращает цвет материала фигуры на холсте.
/// @param[in] m  материал фигуры.
/// @return Цвет: металл — светло-серый, магнитная стенка — фиолетовый, поглотитель —
///         оранжевый, диэлектрик — акцентный цвет темы.
QColor materialColor(Material m)
{
    switch (m) {
    case Material::Metal:      return QColor(0xd7, 0xe2, 0xf0);
    case Material::Magnetic:   return QColor(0x8f, 0x7a, 0xff);
    case Material::Absorber:   return QColor(0xff, 0x9d, 0x5c);
    case Material::Dielectric: return dtheme::accent();
    }
    return dtheme::text();
}

/// Цвет плазменной области.
///
/// Ведущая величина — концентрация: именно она включает плазменную ветку расчёта,
/// и без неё B бесполезно. Поэтому область с N окрашена «плазменным» розовым,
/// а фигура, несущая только подмагничивание или потери, — приглушённо-синим:
/// видно, что она сама по себе ничего не считает.
/// @param[in] pl  плазменные величины фигуры.
/// @return Розовый при заданной N, синий при одной B, фиолетовый в остальных случаях.
QColor plasmaColor(const tpl::Plasma &pl)
{
    if (pl.hasN)
        return QColor(0xff, 0x5c, 0xd0);
    if (pl.hasB)
        return QColor(0x5c, 0xa8, 0xff);
    return QColor(0xc0, 0x5c, 0xff);
}

/// @brief Возвращает цвет фигуры: плазменный, если у неё задана плазма, иначе цвет материала.
/// @param[in] s  фигура.
/// @return Цвет контура и заливки.
QColor shapeColor(const tpl::Shape &s)
{
    return s.plasma.any() ? plasmaColor(s.plasma) : materialColor(s.material);
}

/// Шаг сетки, кратный DELTA, но не мельче ~7 экранных пикселей: иначе каша.
/// @param[in] delta    шаг сетки задания DELTA, мм; не больше 0 — берётся 1 мм.
/// @param[in] pxPerMm  масштаб холста, пикселей на мм.
/// @return Шаг рисуемой сетки, мм: DELTA, умноженный на 5 столько раз, сколько нужно.
double niceGridStep(double delta, double pxPerMm)
{
    double step = delta > 0.0 ? delta : 1.0;
    while (step * pxPerMm < 7.0)
        step *= 5.0;
    return step;
}

/// Шаг подписей линейки: круглое число, дающее не меньше 62 пикселей между метками.
/// @param[in] pxPerMm  масштаб холста, пикселей на мм.
/// @return Шаг меток линейки, мм (из ряда 1, 2, 5, 10 ... 5000; иначе 10000).
double rulerStep(double pxPerMm)
{
    static const double nice[] = {1, 2, 5, 10, 20, 25, 50, 100, 200, 250, 500, 1000, 2000, 5000};
    for (double s : nice) {
        if (s * pxPerMm >= 62.0)
            return s;
    }
    return 10000.0;
}

/// @brief Возвращает расстояние от точки до отрезка.
/// @param[in] p  точка.
/// @param[in] a  начало отрезка.
/// @param[in] b  конец отрезка.
/// @return Расстояние в тех же единицах, что и координаты (мм или пиксели).
double distToSegment(const QPointF &p, const QPointF &a, const QPointF &b)
{
    const QPointF ab = b - a;
    const double len2 = ab.x() * ab.x() + ab.y() * ab.y();
    if (len2 <= 1e-12)
        return QLineF(p, a).length();
    double t = ((p.x() - a.x()) * ab.x() + (p.y() - a.y()) * ab.y()) / len2;
    t = std::clamp(t, 0.0, 1.0);
    return QLineF(p, a + t * ab).length();
}

/// Нормализуем прямоугольник после протяжки: min обязан быть меньше max.
/// Стороны переезжают вместе с гранями — иначе «магнитный верх» окажется снизу.
/// @param[in,out] s  прямоугольник; меняются границы и порядок сторон `edges`.
void normalizeRect(tpl::Shape *s)
{
    if (s->xMin > s->xMax) {
        std::swap(s->xMin, s->xMax);
        std::swap(s->edges[EdgeLeft], s->edges[EdgeRight]);
    }
    if (s->yMin > s->yMax) {
        std::swap(s->yMin, s->yMax);
        std::swap(s->edges[EdgeBottom], s->edges[EdgeTop]);
    }
}

/// Точки дуги: центр, радиус, углы. Число отрезков — от радиуса, но не меньше трёх.
/// Дуга точками. \p step — желаемая длина звена в мм.
///
/// ⚠️ Ломаная, а не «настоящая дуга», потому что в языке `.tpl` дуги нет вовсе:
/// `POLYGON_STAT` понимает только вершины. Круглую стенку авторские задания так и
/// пишут — например, нос гиперзвукового аппарата в `hyper_sheath_0p4.tpl` разбит на
/// полсотни вершин с шагом примерно в шаг сетки.
/// @param[in] c     центр, мм.
/// @param[in] r     радиус, мм.
/// @param[in] a0    начальный угол, радианы (против часовой от оси X).
/// @param[in] a1    конечный угол, радианы.
/// @param[in] step  желаемая длина звена, мм (не меньше 0.1).
/// @return Точки дуги от `a0` до `a1` включительно: от 4 до 97 точек.
QVector<QPointF> arcPoints(const QPointF &c, double r, double a0, double a1, double step = 1.5)
{
    const int n = std::clamp(int(std::fabs(a1 - a0) * r / std::max(0.1, step)), 3, 96);
    QVector<QPointF> pts;
    pts.reserve(n + 1);
    for (int i = 0; i <= n; ++i) {
        const double a = a0 + (a1 - a0) * i / n;
        pts << c + QPointF(r * std::cos(a), r * std::sin(a));
    }
    return pts;
}

/// Дуга ЧЕРЕЗ три точки: из \p a в \p b так, чтобы пройти через \p m.
///
/// Три точки задают окружность однозначно — это и делает инструмент понятным без
/// объяснений: пользователь ставит два конца и оттягивает середину, куда хочет.
/// Лежат на одной прямой — выгиба нет, и честнее вернуть отрезок, чем окружность
/// бесконечного радиуса.
/// @param[in] a     начало дуги, мм.
/// @param[in] m     точка, через которую проходит дуга, мм.
/// @param[in] b     конец дуги, мм.
/// @param[in] step  желаемая длина звена, мм.
/// @return Точки дуги; при трёх точках на одной прямой (или вырожденном радиусе) — отрезок `{a, b}`.
QVector<QPointF> arcThroughPoints(const QPointF &a, const QPointF &m, const QPointF &b,
                                  double step)
{
    const double ax = a.x(), ay = a.y();
    const double bx = m.x(), by = m.y();
    const double cx = b.x(), cy = b.y();

    // Центр описанной окружности через три точки: d — удвоенный ориентированный
    // определитель; d = 0 означает, что точки лежат на одной прямой.
    const double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
    if (std::fabs(d) < 1e-9)
        return {a, b};

    const double sa = ax * ax + ay * ay;
    const double sb = bx * bx + by * by;
    const double sc = cx * cx + cy * cy;
    const QPointF c((sa * (by - cy) + sb * (cy - ay) + sc * (ay - by)) / d,
                    (sa * (cx - bx) + sb * (ax - cx) + sc * (bx - ax)) / d);
    const double r = QLineF(c, a).length();
    if (r <= 1e-9 || r > 1e9)
        return {a, b};

    const auto angleOf = [&c](const QPointF &p) {
        return std::atan2(p.y() - c.y(), p.x() - c.x());
    };
    const auto ccw = [](double v) {
        while (v < 0.0) v += 2 * M_PI;
        while (v >= 2 * M_PI) v -= 2 * M_PI;
        return v;
    };

    const double a0 = angleOf(a);
    // Идём в ту сторону, где лежит средняя точка: иначе дуга уйдёт «в обход»,
    // мимо того места, куда её тянули мышью.
    const double toEnd = ccw(angleOf(b) - a0);
    const double toMid = ccw(angleOf(m) - a0);
    const double a1 = (toMid <= toEnd) ? a0 + toEnd : a0 - ccw(a0 - angleOf(b));

    return arcPoints(c, r, a0, a1, step);
}

/// @brief Возвращает длину единицы задания в миллиметрах.
/// @param[in] u  единица длины.
/// @return Миллиметров в одной единице `u`.
double unitScale(LengthUnit u)
{
    return toMillimeters(1.0, u);
}

/// Следующее имя вида «Линия 3».
///
/// ⚠️ Считаем по СВОЕМУ виду фигур, а не по номеру блока: номера блоков общие у фигур
/// и портов, и второй нарисованный порт получал имя «Порт 4». Пользователь читает имя
/// как порядковый номер — и справедливо не понимает, откуда взялся порт номер 4.
/// @param[in] existing  имена, уже занятые в задании.
/// @param[in] base      основа имени («Линия», «Порт» ...).
/// @return «<base> N», где N на 1 больше наибольшего номера с той же основой.
QString nextName(const QStringList &existing, const QString &base)
{
    const QRegularExpression re(QStringLiteral("^%1 (\\d+)$")
                                    .arg(QRegularExpression::escape(base)));
    int best = 0;
    for (const QString &n : existing) {
        const QRegularExpressionMatch m = re.match(n);
        if (m.hasMatch())
            best = std::max(best, m.captured(1).toInt());
    }
    return QStringLiteral("%1 %2").arg(base).arg(best + 1);
}

/// Характерные точки фигуры — то, к чему осмысленно прилипать мышью.
/// @param[in] s  фигура.
/// @return Углы, середины сторон и центр прямоугольника; центр, четыре точки окружности
///         и концы сектора круга; вершины ломаной и контура.
QVector<QPointF> keyPoints(const tpl::Shape &s)
{
    QVector<QPointF> pts;
    switch (s.kind) {
    case ShapeKind::Rectangle:
        pts << QPointF(s.xMin, s.yMin) << QPointF(s.xMax, s.yMin)
            << QPointF(s.xMax, s.yMax) << QPointF(s.xMin, s.yMax)
            << QPointF((s.xMin + s.xMax) / 2, s.yMin) << QPointF(s.xMax, (s.yMin + s.yMax) / 2)
            << QPointF((s.xMin + s.xMax) / 2, s.yMax) << QPointF(s.xMin, (s.yMin + s.yMax) / 2)
            << QPointF((s.xMin + s.xMax) / 2, (s.yMin + s.yMax) / 2);
        break;
    case ShapeKind::Circle:
        pts << s.center;
        for (int k = 0; k < 4; ++k) {
            const double a = k * M_PI / 2;
            pts << s.center + QPointF(s.rMax * std::cos(a), s.rMax * std::sin(a));
        }
        pts << s.center + QPointF(s.rMax * std::cos(s.fiMin), s.rMax * std::sin(s.fiMin));
        pts << s.center + QPointF(s.rMax * std::cos(s.fiMax), s.rMax * std::sin(s.fiMax));
        break;
    case ShapeKind::Polygon:
    case ShapeKind::Polyline:
        pts = s.points;
        break;
    }
    return pts;
}

/// Звенья фигуры — к ним прилипаем «в любой точке линии».
/// @param[in] s  фигура.
/// @return Стороны прямоугольника или звенья ломаной (контура, с замыкающим); у круга — пусто.
QVector<QLineF> keySegments(const tpl::Shape &s)
{
    QVector<QLineF> segs;
    switch (s.kind) {
    case ShapeKind::Rectangle:
        for (int e = 0; e < 4; ++e)
            segs << QLineF(s.edgeFrom(e), s.edgeTo(e));
        break;
    case ShapeKind::Circle:
        break; // к окружности прилипаем отдельно — проекцией на радиус
    case ShapeKind::Polygon:
    case ShapeKind::Polyline:
        for (int i = 0; i + 1 < s.points.size(); ++i)
            segs << QLineF(s.points.at(i), s.points.at(i + 1));
        if (s.kind == ShapeKind::Polygon && s.points.size() > 2)
            segs << QLineF(s.points.last(), s.points.first());
        break;
    }
    return segs;
}

/// @brief Проецирует точку на отрезок (ближайшая точка отрезка).
/// @param[in] p    точка.
/// @param[in] seg  отрезок.
/// @return Ближайшая к `p` точка отрезка `seg`.
QPointF projectOnSegment(const QPointF &p, const QLineF &seg)
{
    const QPointF a = seg.p1();
    const QPointF ab = seg.p2() - a;
    const double len2 = ab.x() * ab.x() + ab.y() * ab.y();
    if (len2 <= 1e-12)
        return a;
    double t = ((p.x() - a.x()) * ab.x() + (p.y() - a.y()) * ab.y()) / len2;
    t = std::clamp(t, 0.0, 1.0);
    return a + t * ab;
}

} // namespace

GeometryView::GeometryView(QWidget *parent)
    : QGraphicsView(parent)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    setRenderHint(QPainter::Antialiasing, true);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMouseTracking(true);
    setFrameShape(QFrame::NoFrame);
    setBackgroundBrush(dtheme::bg());
    setFocusPolicy(Qt::StrongFocus);
    // Холст не должен диктовать раскладку: иначе панели рядом дёргаются от
    // любой смены подписи под курсором.
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setMinimumSize(240, 180);

    // Ось Y вверх — как в задании. Иначе пользователь будет мысленно переворачивать
    // каждую координату, которую вводит в свойствах.
    scale(1.0, -1.0);
}

void GeometryView::setProject(Project *p)
{
    const bool first = (m_project == nullptr);
    m_project = p;
    // Кэш полей привязан к номерам фигур, а у нового задания это уже другие фигуры.
    m_epsCache.clear();
    updateSceneRect();
    if (first || m_preview)
        zoomToFit();
    viewport()->update();
    updateHint();
}

void GeometryView::updateSceneRect()
{
    if (!m_project)
        return;
    const tpl::Box &b = m_project->box;
    // Под линейки нужен запас; картинке он не нужен — там коробка во всё окно.
    const double pad = m_preview ? std::max(b.width(), b.height()) * 0.03
                                 : std::max(b.width(), b.height()) * 0.14 + 5.0;
    m_scene->setSceneRect(b.xMin - pad, b.yMin - pad, b.width() + 2 * pad, b.height() + 2 * pad);
}

void GeometryView::setPreview(bool on)
{
    m_preview = on;
    setMouseTracking(!on);
    setFocusPolicy(on ? Qt::NoFocus : Qt::StrongFocus);
    setInteractive(!on);
    updateSceneRect();
    zoomToFit();
    viewport()->update();
}

QPixmap GeometryView::render(const tpl::Project &p, const QSize &size)
{
    tpl::Project copy = p; // холст хранит указатель — копия живёт до конца снимка
    GeometryView v;
    v.setAttribute(Qt::WA_DontShowOnScreen, true);
    v.setPreview(true);
    v.setProject(&copy);
    v.resize(size);
    v.zoomToFit();
    return v.grab();
}

void GeometryView::resizeEvent(QResizeEvent *e)
{
    QGraphicsView::resizeEvent(e);
    if (m_preview)
        zoomToFit();
}

void GeometryView::setTool(Tool t)
{
    if (m_tool == t)
        return;
    cancelDrawing();
    m_tool = t;
    setCursor(t == Tool::Select ? Qt::ArrowCursor : Qt::CrossCursor);
    emit toolChanged(t);
    updateHint();
    viewport()->update();
}

void GeometryView::setGridVisible(bool on)
{
    m_grid = on;
    viewport()->update();
}

void GeometryView::setSnapEnabled(bool on)
{
    m_snap = on;
    updateHint();
}

void GeometryView::setDisplayUnit(LengthUnit u)
{
    m_unit = u;
    viewport()->update();
}

void GeometryView::zoomIn() { scale(1.25, 1.25); viewport()->update(); }
void GeometryView::zoomOut() { scale(1.0 / 1.25, 1.0 / 1.25); viewport()->update(); }

void GeometryView::zoomToFit()
{
    if (m_scene->sceneRect().isEmpty())
        return;
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    viewport()->update();
}

// ---------------------------------------------------------------- координаты

double GeometryView::gridStep() const
{
    return (m_project && m_project->delta > 0.0) ? m_project->delta : 1.0;
}

double GeometryView::pxPerMm() const
{
    return std::fabs(transform().m11());
}

/// Куда прилипнуть. Порядок важен: вершина сильнее линии, линия сильнее сетки.
///
/// Привязка только к сетке бесполезна — человек ведёт порт к КОНЦУ нарисованной
/// стенки, а не к абстрактному узлу. Поэтому сначала ищем характерные точки
/// нарисованного, потом любую точку на его линиях, и лишь затем округляем к сетке.
GeometryView::Snap GeometryView::findSnap(const QPointF &raw, bool objects) const
{
    Snap out;
    out.pos = raw;
    if (!m_project || !m_snap)
        return out;

    const double px = pxPerMm();
    const double tol = px > 1e-9 ? kSnapPx / px : 0.0; // допуск в мм, но задан в пикселях

    if (objects && tol > 0.0) {
        double best = tol;
        // 1) вершины, центры и середины сторон
        for (const tpl::Shape &s : m_project->shapes) {
            if (s.id == m_excludeId)
                continue;
            for (const QPointF &v : keyPoints(s)) {
                const double d = QLineF(raw, v).length();
                if (d < best) {
                    best = d;
                    out.pos = v;
                    out.toObject = true;
                    out.what = (s.kind == ShapeKind::Circle && v == s.center)
                                   ? tpl::tr("центр", "center")
                                   : tpl::tr("вершина «%1»", "vertex \"%1\"").arg(s.name);
                }
            }
        }
        // Углы счётной области — тоже осмысленная цель.
        const tpl::Box &b = m_project->box;
        for (const QPointF &v : {QPointF(b.xMin, b.yMin), QPointF(b.xMax, b.yMin),
                                 QPointF(b.xMax, b.yMax), QPointF(b.xMin, b.yMax)}) {
            const double d = QLineF(raw, v).length();
            if (d < best) {
                best = d;
                out.pos = v;
                out.toObject = true;
                out.what = tpl::tr("угол области", "domain corner");
            }
        }
        if (out.toObject)
            return out;

        // 2) любая точка на линии
        best = tol;
        for (const tpl::Shape &s : m_project->shapes) {
            if (s.id == m_excludeId)
                continue;
            for (const QLineF &seg : keySegments(s)) {
                const QPointF q = projectOnSegment(raw, seg);
                const double d = QLineF(raw, q).length();
                if (d < best) {
                    best = d;
                    out.pos = q;
                    out.toObject = true;
                    out.what = tpl::tr("на линии «%1»", "on line \"%1\"").arg(s.name);
                }
            }
            if (s.kind == ShapeKind::Circle && s.rMax > 0.0) {
                const QLineF r(s.center, raw);
                if (r.length() > 1e-9) {
                    const QPointF q = s.center + (raw - s.center) * (s.rMax / r.length());
                    const double d = QLineF(raw, q).length();
                    if (d < best) {
                        best = d;
                        out.pos = q;
                        out.toObject = true;
                        out.what = tpl::tr("на окружности «%1»", "on circle \"%1\"").arg(s.name);
                    }
                }
            }
        }
        if (out.toObject)
            return out;
    }

    // 3) сетка
    const double s = gridStep();
    out.pos = QPointF(std::round(raw.x() / s) * s, std::round(raw.y() / s) * s);
    return out;
}

QPointF GeometryView::snapped(const QPointF &p) const
{
    return findSnap(p, true).pos;
}

QPointF GeometryView::toScene(const QPoint &viewPos) const
{
    // При перетаскивании фигуры целиком берём только сетку: иначе фигура
    // рывками прыгала бы на чужие вершины вместо плавного хода.
    const Snap s = findSnap(mapToScene(viewPos), !m_dragging);
    const_cast<GeometryView *>(this)->m_lastSnap = s;
    return s.pos;
}

tpl::Shape *GeometryView::shapeById(int id)
{
    if (!m_project)
        return nullptr;
    for (tpl::Shape &s : m_project->shapes) {
        if (s.id == id)
            return &s;
    }
    return nullptr;
}

const tpl::Shape *GeometryView::shapeById(int id) const
{
    return const_cast<GeometryView *>(this)->shapeById(id);
}

Port *GeometryView::portById(int id)
{
    if (!m_project)
        return nullptr;
    for (Port &p : m_project->ports) {
        if (p.id == id)
            return &p;
    }
    return nullptr;
}

const Port *GeometryView::portById(int id) const
{
    return const_cast<GeometryView *>(this)->portById(id);
}

int GeometryView::nextId() const
{
    return m_project ? m_project->nextFreeId() : 1;
}

QString GeometryView::freshShapeName(const QString &base) const
{
    QStringList names;
    if (m_project) {
        for (const tpl::Shape &s : m_project->shapes)
            names << s.name;
    }
    return nextName(names, base);
}

QString GeometryView::freshPortName() const
{
    QStringList names;
    if (m_project) {
        for (const Port &p : m_project->ports)
            names << p.name;
    }
    return nextName(names, tpl::tr("Порт", "Port"));
}

PortSide GeometryView::nearestSide(const QPointF &mm, double *along) const
{
    const tpl::Box &b = m_project->box;
    const double dL = std::fabs(mm.x() - b.xMin);
    const double dR = std::fabs(mm.x() - b.xMax);
    const double dB = std::fabs(mm.y() - b.yMin);
    const double dT = std::fabs(mm.y() - b.yMax);
    const double m = std::min(std::min(dL, dR), std::min(dB, dT));

    if (m == dL) { if (along) *along = mm.y(); return PortSide::Left; }
    if (m == dR) { if (along) *along = mm.y(); return PortSide::Right; }
    if (m == dB) { if (along) *along = mm.x(); return PortSide::Bottom; }
    if (along) *along = mm.x();
    return PortSide::Top;
}

/// Сторона коробки по НАПРАВЛЕНИЮ протяжки, а не по точке нажатия.
///
/// Апертура всегда идёт вдоль границы, поэтому вертикальная протяжка означает вход
/// на левой или правой стороне, горизонтальная — на нижней или верхней. Раньше сторона
/// бралась из точки нажатия, и у края коробки порт ложился поперёк того, что тянули.
PortSide GeometryView::sideForDrag(const QPointF &from, const QPointF &to) const
{
    const tpl::Box &b = m_project->box;
    const double dx = std::fabs(to.x() - from.x());
    const double dy = std::fabs(to.y() - from.y());

    if (dx < 1e-9 && dy < 1e-9) {
        double along = 0.0;
        return nearestSide(from, &along); // ещё не тянули — берём ближнюю границу
    }
    if (dy >= dx) {
        return std::fabs(from.x() - b.xMin) <= std::fabs(from.x() - b.xMax) ? PortSide::Left
                                                                            : PortSide::Right;
    }
    return std::fabs(from.y() - b.yMin) <= std::fabs(from.y() - b.yMax) ? PortSide::Bottom
                                                                       : PortSide::Top;
}

QPointF GeometryView::portCenter(const Port &p) const
{
    if (!m_project)
        return {};
    const double mid = (p.from + p.to) / 2.0;
    const double plane = portPlane(p, m_project->box);
    return portAlongY(p.side) ? QPointF(plane, mid) : QPointF(mid, plane);
}

/// Концы апертуры порта в координатах сцены.
/// @param[in] p       порт.
/// @param[in] b       счётная область (коробка) задания.
/// @param[in] second  false — начало апертуры (`from`), true — конец (`to`).
/// @return Точка конца апертуры на сцене, мм.
static QPointF portEnd(const Port &p, const tpl::Box &b, bool second)
{
    const double plane = portPlane(p, b);
    const double v = second ? p.to : p.from;
    return portAlongY(p.side) ? QPointF(plane, v) : QPointF(v, plane);
}

QPainterPath GeometryView::shapePath(const tpl::Shape &s) const
{
    QPainterPath path;
    switch (s.kind) {
    case ShapeKind::Rectangle:
        path.addRect(QRectF(s.xMin, s.yMin, s.xMax - s.xMin, s.yMax - s.yMin));
        break;
    case ShapeKind::Circle: {
        // ⚠️ Дугу собираем ТОЧКАМИ, а не QPainterPath::arcTo. У arcTo своя система
        // отсчёта углов, а у нас ось Y перевёрнута — сектор 45..135 град рисовался
        // бы снизу вместо верха. Картинка обязана совпадать с тем, что посчитает
        // ядро: у него углы обычные, против часовой от оси X.
        const bool full = std::fabs(s.fiMax - s.fiMin) >= 2 * M_PI - 1e-6;
        if (full) {
            path.addEllipse(s.center, s.rMax, s.rMax);
            if (s.rMin > 0.0)
                path.addEllipse(s.center, s.rMin, s.rMin); // кольцо: дырка по OddEven
            break;
        }
        const QVector<QPointF> outer = arcPoints(s.center, s.rMax, s.fiMin, s.fiMax);
        path.moveTo(outer.first());
        for (int i = 1; i < outer.size(); ++i)
            path.lineTo(outer.at(i));
        if (s.rMin > 0.0) {
            const QVector<QPointF> inner = arcPoints(s.center, s.rMin, s.fiMax, s.fiMin);
            for (const QPointF &pt : inner)
                path.lineTo(pt);
        } else {
            path.lineTo(s.center);
        }
        path.closeSubpath();
        break;
    }
    case ShapeKind::Polygon:
    case ShapeKind::Polyline:
        if (!s.points.isEmpty()) {
            path.moveTo(s.points.first());
            for (int i = 1; i < s.points.size(); ++i)
                path.lineTo(s.points.at(i));
            if (s.kind == ShapeKind::Polygon)
                path.closeSubpath();
        }
        break;
    }
    return path;
}

// ------------------------------------------------------------------ попадания

/// Ручка поворота — на отлёте от объекта, чтобы не мешала ручкам размера.
/// Отступ считаем в пикселях: при любом зуме она одинаково доступна.
QPointF GeometryView::rotateHandlePos() const
{
    if (!m_project || m_sel.isEmpty())
        return {};

    QPointF anchor;
    QPointF away(0, -34); // экранный отступ: вверх от объекта
    if (m_sel.kind == Selection::PortBody) {
        const Port *p = portById(m_sel.id);
        if (!p)
            return {};
        anchor = portCenter(*p);
        // Отводим наружу коробки, чтобы ручка не легла на геометрию.
        switch (p->side) {
        case PortSide::Left:   away = QPointF(-34, 0); break;
        case PortSide::Right:  away = QPointF(34, 0); break;
        case PortSide::Bottom: away = QPointF(0, 34); break;
        case PortSide::Top:    away = QPointF(0, -34); break;
        }
    } else {
        const tpl::Shape *s = shapeById(m_sel.id);
        if (!s)
            return {};
        double xLo = 0, xHi = 0, yLo = 0, yHi = 0;
        s->bounds(&xLo, &xHi, &yLo, &yHi);
        anchor = QPointF((xLo + xHi) / 2, yHi);
    }
    return mapToScene((mapFromScene(anchor) + away).toPoint());
}

QVector<GeometryView::Handle> GeometryView::handles() const
{
    QVector<Handle> out;
    if (m_sel.isEmpty())
        return out;

    // Прямоугольник в языке задания всегда по осям — поворачивать его можно только
    // превратив в контур, а с разными сторонами это потеряло бы их материалы.
    const tpl::Shape *sel = shapeById(m_sel.id);
    const bool canRotate = (m_sel.kind == Selection::PortBody)
                           || (sel && (sel->kind != ShapeKind::Rectangle || sel->hasUniformEdges()));
    if (canRotate)
        out << Handle{rotateHandlePos(), kRoleRotate};

    if (m_sel.kind == Selection::PortBody) {
        const Port *p = portById(m_sel.id);
        if (!p || !m_project)
            return out;
        out << Handle{portEnd(*p, m_project->box, false), kRolePortFrom};
        out << Handle{portEnd(*p, m_project->box, true), kRolePortTo};
        return out;
    }

    const tpl::Shape *s = shapeById(m_sel.id);
    if (!s)
        return out;

    switch (s->kind) {
    case ShapeKind::Rectangle:
        out << Handle{{s->xMin, s->yMin}, 0} << Handle{{s->xMax, s->yMin}, 1}
            << Handle{{s->xMax, s->yMax}, 2} << Handle{{s->xMin, s->yMax}, 3}
            << Handle{{(s->xMin + s->xMax) / 2, s->yMin}, 4 + EdgeBottom}
            << Handle{{s->xMax, (s->yMin + s->yMax) / 2}, 4 + EdgeRight}
            << Handle{{(s->xMin + s->xMax) / 2, s->yMax}, 4 + EdgeTop}
            << Handle{{s->xMin, (s->yMin + s->yMax) / 2}, 4 + EdgeLeft};
        break;
    case ShapeKind::Circle: {
        const double mid = (s->fiMin + s->fiMax) / 2.0;
        out << Handle{s->center + QPointF(s->rMax * std::cos(mid), s->rMax * std::sin(mid)), 0};
        if (s->rMin > 0.0)
            out << Handle{s->center + QPointF(s->rMin * std::cos(mid), s->rMin * std::sin(mid)), 1};
        if (std::fabs(s->fiMax - s->fiMin) < 2 * M_PI - 1e-6) {
            out << Handle{s->center + QPointF(s->rMax * std::cos(s->fiMin),
                                              s->rMax * std::sin(s->fiMin)), 2};
            out << Handle{s->center + QPointF(s->rMax * std::cos(s->fiMax),
                                              s->rMax * std::sin(s->fiMax)), 3};
        }
        break;
    }
    case ShapeKind::Polygon:
    case ShapeKind::Polyline:
        for (int i = 0; i < s->points.size(); ++i)
            out << Handle{s->points.at(i), kRoleVertex + i};
        break;
    }
    return out;
}

int GeometryView::handleAt(const QPoint &viewPos) const
{
    for (const Handle &h : handles()) {
        if (QLineF(mapFromScene(h.pos), viewPos).length() <= kHandlePx)
            return h.role;
    }
    return -1;
}

Selection GeometryView::pick(const QPoint &viewPos) const
{
    Selection none;
    if (!m_project)
        return none;

    // Порты ловим первыми: их метка мелкая и лежит поверх геометрии.
    for (const Port &p : m_project->ports) {
        if (QLineF(mapFromScene(portCenter(p)), viewPos).length() <= kPortMarkerPx + 2.0)
            return {Selection::PortBody, p.id, -1};
    }
    const tpl::Box &b = m_project->box;
    for (const Port &p : m_project->ports) {
        if (distToSegment(viewPos, mapFromScene(portEnd(p, b, false)),
                          mapFromScene(portEnd(p, b, true))) <= kPickPx)
            return {Selection::PortBody, p.id, -1};
    }

    // ⚠️ Выделенная фигура забирает мышь у всех остальных.
    //
    // Без этого работать с наложенными фигурами невозможно. Живой случай:
    // в `hyper_sheath_0p4.tpl` тело аппарата (блок 20) целиком лежит внутри
    // прямоугольника распределения eps (блок 55), а тот нарисован позже — значит,
    // по правилу «сверху тот, кто позже» любой клик по телу доставался блоку 55.
    // Выделить тело в списке объектов удавалось, а сдвинуть — уже нет: мышь
    // хватала прямоугольник. Единственным выходом было убрать мешающую фигуру.
    //
    // Теперь так: пока фигура выделена, клик по ней достаётся ей, кто бы ни лежал
    // сверху. Промахнулся мимо неё — работает обычный разбор сверху вниз, то есть
    // выбрать соседнюю фигуру по-прежнему можно одним кликом.
    if (m_sel.kind == Selection::ShapeBody || m_sel.kind == Selection::ShapeEdge
        || m_sel.kind == Selection::ShapeVertex) {
        if (const tpl::Shape *s = shapeById(m_sel.id)) {
            const Selection hit = hitShape(*s, viewPos);
            if (!hit.isEmpty())
                return hit;
        }
    }

    // С конца: то, что нарисовано позже, лежит сверху.
    for (int i = m_project->shapes.size() - 1; i >= 0; --i) {
        const Selection hit = hitShape(m_project->shapes.at(i), viewPos);
        if (!hit.isEmpty())
            return hit;
    }
    return none;
}

Selection GeometryView::hitShape(const tpl::Shape &s, const QPoint &viewPos) const
{
    const Selection none;
    const QPointF mm = mapToScene(viewPos);

    if (s.kind == ShapeKind::Rectangle) {
        for (int e = 0; e < 4; ++e) {
            const QPointF a = mapFromScene(s.edgeFrom(e));
            const QPointF c = mapFromScene(s.edgeTo(e));
            if (distToSegment(viewPos, a, c) <= kPickPx)
                return {Selection::ShapeEdge, s.id, e};
        }
        if (QRectF(s.xMin, s.yMin, s.xMax - s.xMin, s.yMax - s.yMin).contains(mm))
            return {Selection::ShapeBody, s.id, -1};
        return none;
    }

    if (s.kind == ShapeKind::Circle) {
        const double d = QLineF(s.center, mm).length();
        if (!s.isContour() && d <= s.rMax && d >= s.rMin)
            return {Selection::ShapeBody, s.id, -1};
        if (std::fabs(d - s.rMax) * pxPerMm() <= kPickPx
            || (s.rMin > 0.0 && std::fabs(d - s.rMin) * pxPerMm() <= kPickPx))
            return {Selection::ShapeBody, s.id, -1};
        if (s.isContour() && d <= s.rMax)
            return {Selection::ShapeBody, s.id, -1};
        return none;
    }

    // Ломаная и контур: попадание по любому звену либо внутрь заливки.
    const int n = s.points.size();
    for (int k = 0; k + 1 < n; ++k) {
        if (distToSegment(viewPos, mapFromScene(s.points.at(k)),
                          mapFromScene(s.points.at(k + 1))) <= kPickPx)
            return {Selection::ShapeBody, s.id, -1};
    }
    if (s.kind == ShapeKind::Polygon && n >= 2) {
        if (distToSegment(viewPos, mapFromScene(s.points.last()),
                          mapFromScene(s.points.first())) <= kPickPx)
            return {Selection::ShapeBody, s.id, -1};
        if (shapePath(s).contains(mm))
            return {Selection::ShapeBody, s.id, -1};
    }
    return none;
}

void GeometryView::setSelection(const Selection &s)
{
    // Обычный клик всегда начинает выделение заново — набор из нескольких фигур
    // держится только на Ctrl и на списке объектов.
    const bool hadMulti = !m_multi.isEmpty();
    m_multi.clear();
    if (m_sel == s && !hadMulti)
        return;
    m_sel = s;
    viewport()->update();
    emit selectionChanged();
}

QVector<int> GeometryView::selectedShapeIds() const
{
    if (!m_multi.isEmpty())
        return m_multi;
    // Порт фигурой не считается: у него нет ни точек, ни материала.
    if (m_sel.kind != Selection::Nothing && m_sel.kind != Selection::PortBody
        && shapeById(m_sel.id))
        return {m_sel.id};
    return {};
}

void GeometryView::setSelectedIds(const QVector<int> &ids)
{
    // Оставляем только существующие фигуры и убираем повторы: список приходит
    // снаружи (из списка объектов), и доверять ему нельзя.
    QVector<int> keep;
    for (int id : ids) {
        if (!keep.contains(id) && shapeById(id))
            keep << id;
    }

    if (keep.isEmpty()) {
        setSelection({});
        return;
    }
    if (keep.size() == 1) {
        m_multi.clear();
        m_sel = {Selection::ShapeBody, keep.first(), -1};
        viewport()->update();
        emit selectionChanged();
        return;
    }

    m_multi = keep;
    // Основная — последняя названная: по ней рисуются ручки и живёт панель свойств.
    m_sel = {Selection::ShapeBody, keep.last(), -1};
    viewport()->update();
    emit selectionChanged();
}


// ------------------------------------------------------------------ рисование

void GeometryView::drawBackground(QPainter *p, const QRectF &rect)
{
    p->fillRect(rect, dtheme::bg());
    if (!m_project)
        return;

    const double px = pxPerMm();
    const tpl::Box &b = m_project->box;

    if (m_grid && !m_preview && px > 0.0) {
        const double step = niceGridStep(m_project->delta, px);
        QPen pen(dtheme::line());
        pen.setCosmetic(true); // толщина в пикселях: сетка не пухнет при зуме
        pen.setWidthF(1.0);
        p->setPen(pen);

        const double x0 = std::floor(rect.left() / step) * step;
        const double y0 = std::floor(rect.top() / step) * step;
        for (double x = x0; x <= rect.right(); x += step)
            p->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
        for (double y = y0; y <= rect.bottom(); y += step)
            p->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    }

    // Счётная область — за её пределами поля не считаются.
    QPen boxPen(dtheme::textMute());
    boxPen.setCosmetic(true);
    boxPen.setWidthF(1.5);
    boxPen.setStyle(Qt::DashLine);
    p->setPen(boxPen);
    p->setBrush(QColor(0x13, 0x1a, 0x28, 120));
    p->drawRect(QRectF(b.xMin, b.yMin, b.width(), b.height()));
}

void GeometryView::drawShape(QPainter *p, const tpl::Shape &s) const
{
    const QColor c = shapeColor(s);
    const bool selected = (s.id == m_sel.id && m_sel.kind != Selection::PortBody)
                          || m_multi.contains(s.id);
    // Контурные материалы ядро рисует периметром, заливку — площадью.
    // Показываем ровно это, иначе картинка соврёт про расчёт.
    const bool contour = s.isContour();

    // Фигуре с профилем ровная заливка не нужна: поверх ляжет посчитанное поле, и
    // ровный цвет только исказил бы его оттенки. Но у профиля ИЗ ФАЙЛА считать нечего
    // (среда лежит в чужом .eps, формулы нет), и без заливки фигура стала бы невидимой.
    const bool gradient = s.hasEpsProfile() && !s.epsProfile.readsFile()
                          && settings::values().epsGradient;

    // Край залитой области — не стенка, и выглядеть как стенка он не должен: тонкая
    // полупрозрачная линия вместо жирной. Жирной рисуется только то, что ядро
    // поставит контуром, и выделенная фигура — её надо видеть.
    QPen pen(contour || selected ? c : QColor(c.red(), c.green(), c.blue(), 150));
    pen.setCosmetic(true);
    pen.setWidthF(selected ? 3.0 : (contour ? 1.9 : 1.0));
    p->setPen(pen);
    p->setBrush(contour || gradient ? QBrush(Qt::NoBrush)
                                    : QBrush(QColor(c.red(), c.green(), c.blue(), 62)));

    if (s.kind == ShapeKind::Rectangle && contour) {
        // Стороны могут отличаться материалом или вовсе отсутствовать —
        // рисуем каждую отдельно, чтобы «открытый торец» был виден глазом.
        for (int e = 0; e < 4; ++e) {
            if (!s.edges[size_t(e)].present)
                continue;
            QPen ep(materialColor(s.edgeMaterial(e)));
            ep.setCosmetic(true);
            const bool edgeSel = selected
                                 && (m_sel.kind != Selection::ShapeEdge || m_sel.index == e);
            ep.setWidthF(m_sel.kind == Selection::ShapeEdge && m_sel.id == s.id && m_sel.index == e
                             ? 4.5 : (edgeSel ? 3.0 : 1.9));
            p->setPen(ep);
            p->drawLine(s.edgeFrom(e), s.edgeTo(e));
        }
        // Снятая сторона — пунктиром: видно, что там дыра, а не «забыли нарисовать».
        QPen ghost(dtheme::textMute());
        ghost.setCosmetic(true);
        ghost.setStyle(Qt::DotLine);
        p->setPen(ghost);
        for (int e = 0; e < 4; ++e) {
            if (!s.edges[size_t(e)].present)
                p->drawLine(s.edgeFrom(e), s.edgeTo(e));
        }
    } else {
        p->drawPath(shapePath(s));
    }

    // Неоднородная eps: заливаем посчитанным полем, а не ровным цветом. Иначе на
    // холсте видно «здесь диэлектрик», но не видно, где он гуще, — а вся суть профиля
    // именно в этом.
    if (s.hasEpsProfile() && !s.epsProfile.readsFile() && settings::values().epsGradient) {
        if (const EpsField *field = epsField(s)) {
            const QRectF &b = field->bounds;
            const QImage &img = field->image;
            p->save();
            p->setClipPath(shapePath(s));
            // Ось Y сцены смотрит ВВЕРХ, а строки изображения идут сверху вниз.
            // Прямоугольником назначения это не выразить (отрицательную высоту Qt
            // нормализует и картинку не переворачивает), поэтому переворачиваем сами:
            // начало — в верхнем по экрану углу, масштаб по Y отрицательный.
            p->translate(b.left(), b.top() + b.height());
            p->scale(b.width() / img.width(), -b.height() / img.height());
            p->drawImage(0, 0, img);
            p->restore();
            // Контур поверх заливки: без него фигура сливается с соседями.
            p->setPen(pen);
            p->setBrush(Qt::NoBrush);
            p->drawPath(shapePath(s));
        }
    }

    // Подмагничивание и потери своей площади не имеют: они действуют поверх той же
    // области, что и концентрация. Цветом их не показать — цвет уже занят под N,
    // поэтому кладём штриховку поверх заливки. Иначе картинка умолчит о том, что
    // в области задано, и человек снова будет держать это в голове.
    if (s.plasma.hasB || s.plasma.hasY || s.plasma.hasS) {
        double xLo = 0, xHi = 0, yLo = 0, yHi = 0;
        s.bounds(&xLo, &xHi, &yLo, &yHi);
        p->save();
        p->setClipPath(shapePath(s));
        // Шаг от размера фигуры: штриховка обязана оставаться читаемой на любом зуме.
        const double step = std::max(1e-6, (xHi - xLo + yHi - yLo) / 14.0);
        const auto hatch = [&](const QColor &col, bool up) {
            QPen hp(col);
            hp.setCosmetic(true); // толщина в пикселях: линии не пухнут при зуме
            hp.setWidthF(1.2);
            p->setPen(hp);
            if (up) { // «/» — линии x + y = k
                for (double k = xLo + yLo; k <= xHi + yHi; k += step) {
                    p->drawLine(QPointF(std::max(xLo, k - yHi), std::min(yHi, k - xLo)),
                                QPointF(std::min(xHi, k - yLo), std::max(yLo, k - xHi)));
                }
            } else {  // «\» — линии x - y = k
                for (double k = xLo - yHi; k <= xHi - yLo; k += step) {
                    p->drawLine(QPointF(std::max(xLo, k + yLo), std::max(yLo, xLo - k)),
                                QPointF(std::min(xHi, k + yHi), std::min(yHi, xHi - k)));
                }
            }
        };
        if (s.plasma.hasB)
            hatch(QColor(0x5c, 0xa8, 0xff), true);
        if (s.plasma.hasY)
            hatch(QColor(0xc0, 0x5c, 0xff), false);
        if (s.plasma.hasS) {
            // σ — горизонтальной штриховкой: диагонали уже заняты под B и Y.
            QPen hp(QColor(0xff, 0x9f, 0x43));
            hp.setCosmetic(true);
            hp.setWidthF(1.2);
            p->setPen(hp);
            for (double y = yLo + step / 2.0; y < yHi; y += step)
                p->drawLine(QPointF(xLo, y), QPointF(xHi, y));
        }
        p->restore();
        p->setPen(pen);
    }

    // Стрелка скорости у движущегося блока — иначе его не отличить от статичного.
    if (s.isMoving()) {
        double xLo = 0, xHi = 0, yLo = 0, yHi = 0;
        s.bounds(&xLo, &xHi, &yLo, &yHi);
        const QPointF from((xLo + xHi) / 2, (yLo + yHi) / 2);
        QPen vpen(dtheme::warn());
        vpen.setCosmetic(true);
        vpen.setWidthF(2.0);
        p->setPen(vpen);
        p->drawLine(from, from + QPointF(s.vx, s.vy));
    }
}

void GeometryView::drawPort(QPainter *p, const Port &port) const
{
    const QPointF c = portCenter(port);
    const tpl::Box &b = m_project->box;
    const bool selected = (m_sel.kind == Selection::PortBody && m_sel.id == port.id);

    QPen aperture(port.active ? dtheme::ok() : dtheme::danger());
    aperture.setCosmetic(true);
    aperture.setWidthF(selected ? 6.5 : 4.0);
    p->setPen(aperture);
    p->drawLine(portEnd(port, b, false), portEnd(port, b, true));

    // Стрелка направления падающей волны: у входа внутри области иначе не понять,
    // куда он светит, а направление ядро выводит по половине коробки.
    if (port.active) {
        const double len = std::max(b.width(), b.height()) * 0.05;
        QPointF dir;
        switch (port.side) {
        case PortSide::Left:   dir = QPointF(len, 0); break;
        case PortSide::Right:  dir = QPointF(-len, 0); break;
        case PortSide::Bottom: dir = QPointF(0, len); break;
        case PortSide::Top:    dir = QPointF(0, -len); break;
        }
        QPen arrow(dtheme::ok());
        arrow.setCosmetic(true);
        arrow.setWidthF(2.0);
        p->setPen(arrow);
        p->drawLine(c, c + dir);
        const QPointF n(-dir.y() * 0.3, dir.x() * 0.3);
        p->drawLine(c + dir, c + dir * 0.6 + n);
        p->drawLine(c + dir, c + dir * 0.6 - n);
    }

    // Кружок с меткой — в экранных координатах: он не должен ни расти при зуме,
    // ни переворачиваться вместе с осью Y.
    p->save();
    p->resetTransform();
    const QPointF sc = mapFromScene(c);
    const double r = kPortMarkerPx;

    p->setBrush(port.active ? dtheme::ok() : dtheme::danger());
    p->setPen(QPen(dtheme::bg(), 2.0));
    p->drawEllipse(sc, r, r);

    QPen mark(QColor(0x00, 0x00, 0x00));
    mark.setWidthF(2.2);
    mark.setCapStyle(Qt::RoundCap);
    p->setPen(mark);
    if (port.active) {
        p->drawLine(sc + QPointF(-r * 0.42, 0), sc + QPointF(-r * 0.1, r * 0.36));
        p->drawLine(sc + QPointF(-r * 0.1, r * 0.36), sc + QPointF(r * 0.45, -r * 0.38));
    } else {
        p->drawLine(sc + QPointF(-r * 0.36, -r * 0.36), sc + QPointF(r * 0.36, r * 0.36));
        p->drawLine(sc + QPointF(-r * 0.36, r * 0.36), sc + QPointF(r * 0.36, -r * 0.36));
    }
    p->restore();
}

void GeometryView::drawHandles(QPainter *p) const
{
    const QVector<Handle> hs = handles();
    if (hs.isEmpty())
        return;
    p->save();
    p->resetTransform();
    for (const Handle &h : hs) {
        const QPointF sc = mapFromScene(h.pos);

        if (h.role == kRoleRotate) {
            // Полукруглая стрелка: за неё берутся, чтобы повернуть.
            QPen pen(dtheme::warn());
            pen.setWidthF(2.2);
            pen.setCapStyle(Qt::RoundCap);
            p->setPen(pen);
            p->setBrush(Qt::NoBrush);
            const QRectF box(sc.x() - 9, sc.y() - 9, 18, 18);
            p->drawArc(box, 30 * 16, 240 * 16);
            // Наконечник на конце дуги.
            const double a = 30.0 * M_PI / 180.0;
            const QPointF tip(sc.x() + 9 * std::cos(a), sc.y() - 9 * std::sin(a));
            p->drawLine(tip, tip + QPointF(-1.0, -5.5));
            p->drawLine(tip, tip + QPointF(5.0, -2.0));
            continue;
        }

        const bool vertexSel = (m_sel.kind == Selection::ShapeVertex
                                && h.role == kRoleVertex + m_sel.index);
        p->setPen(QPen(dtheme::bg(), 1.5));
        p->setBrush(vertexSel ? dtheme::warn() : dtheme::accent());
        p->drawRect(QRectF(sc.x() - kHandlePx / 2, sc.y() - kHandlePx / 2, kHandlePx, kHandlePx));
    }
    p->restore();
}

int GeometryView::epsResolution() const
{
    if (!m_project)
        return 192;
    int n = 0;
    for (const tpl::Shape &s : m_project->shapes) {
        // Профиль из файла поля не рисует — и бюджет точек ему выделять незачем.
        if (s.hasEpsProfile() && !s.epsProfile.readsFile())
            ++n;
    }
    if (n <= 1)
        return 192;
    // Общий бюджет точек держим постоянным: суммарная площадь всех полей примерно
    // равна одному полю 192x192, сколько бы фигур ни было.
    const int side = int(192.0 / std::sqrt(double(n)));
    return std::clamp(side, 24, 192);
}

const GeometryView::EpsField *GeometryView::epsField(const tpl::Shape &s) const
{
    if (!s.hasEpsProfile())
        return nullptr;
    // Готовое распределение из .eps: формулы нет, считать в узлах нечего. Читать
    // чужой двоичный файл ради картинки редактор не берётся — это отдельная работа
    // (FieldView), а соврать гладким полем было бы хуже, чем не рисовать ничего.
    if (s.epsProfile.readsFile())
        return nullptr;

    double xLo = 0, xHi = 0, yLo = 0, yHi = 0;
    s.bounds(&xLo, &xHi, &yLo, &yHi);
    const QRectF bounds(xLo, yLo, xHi - xLo, yHi - yLo);
    const QString formula = s.epsProfile.formula.trimmed();
    const int longSide = epsResolution();

    EpsField &f = m_epsCache[s.id];
    if (f.valid && f.formula == formula && f.bounds == bounds && f.res == longSide)
        return &f;

    f = EpsField{};
    f.formula = formula;
    f.bounds = bounds;
    f.res = longSide;
    if (bounds.width() <= 0.0 || bounds.height() <= 0.0)
        return nullptr;

    // Формулу разбираем ОДИН раз на всё поле. Разбор строки в 250 символов стоит на
    // три порядка дороже её вычисления, и делать его в цикле по узлам — это те самые
    // секунды заморозки окна, из-за которых кэш и появился.
    f.prog = tpl::ExprProgram::compile(formula);
    if (!f.prog.valid())
        return nullptr;

    // Разрешение поля от разрешения экрана не зависит: изображение растягивается при
    // отрисовке. Профиль по смыслу гладкий, поэтому ступенек не видно.
    const double aspect = bounds.height() / bounds.width();
    const int w = (aspect <= 1.0) ? longSide : std::max(2, int(longSide / aspect));
    const int h = (aspect <= 1.0) ? std::max(2, int(longSide * aspect)) : longSide;

    // Координаты в формуле отсчитываются от ЦЕНТРА фигуры — ровно так же, как их
    // подставит ядро: эмиттер ставит анкер блока в центр, а `SetYLineForDiel`
    // вычитает анкер прежде чем звать CalcEps.
    const QPointF o = s.epsOrigin();

    QVector<double> raw(w * h, 0.0);
    QVector<bool> ok(w * h, false);
    bool any = false;
    double lo = 0.0, hi = 0.0;

    for (int j = 0; j < h; ++j) {
        // Пиксель j = 0 — верхняя строка изображения, а это БОЛЬШИЙ y сцены:
        // ось Y на холсте смотрит вверх. Перепутать здесь значит отразить профиль.
        const double y = bounds.bottom() - bounds.height() * (j + 0.5) / h - o.y();
        for (int i = 0; i < w; ++i) {
            const double x = bounds.left() + bounds.width() * (i + 0.5) / w - o.x();
            const auto v = f.prog.evalEps(x, y);
            if (!v)
                continue;
            const int k = j * w + i;
            raw[k] = *v;
            ok[k] = true;
            lo = any ? std::min(lo, *v) : *v;
            hi = any ? std::max(hi, *v) : *v;
            any = true;
        }
    }
    if (!any)
        return nullptr;

    f.lo = lo;
    f.hi = hi;
    f.image = QImage(w, h, QImage::Format_ARGB32_Premultiplied);
    f.image.fill(Qt::transparent);

    // Постоянное поле (формула без координат) — законный случай: тогда шкала выродится
    // в точку, а вся область должна получить один цвет, а не случайный из-за деления
    // на нулевой размах.
    const double span = hi - lo;
    const settings::Palette pal = settings::values().palette;
    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            const int k = j * w + i;
            if (!ok[k])
                continue;
            const double t = (span > 1e-12) ? (raw[k] - lo) / span : 0.5;
            QColor c = settings::paletteColor(pal, t);
            c.setAlpha(190); // сквозь заливку должна просвечивать сетка
            f.image.setPixelColor(i, j, c);
        }
    }
    f.valid = true;
    return &f;
}

std::optional<double> GeometryView::epsAt(const QPointF &mm, QString *shapeName) const
{
    if (!m_project)
        return std::nullopt;

    // Сверху вниз по списку: если фигуры наложились, отвечает верхняя — та же,
    // что рисуется поверх остальных.
    for (int i = m_project->shapes.size() - 1; i >= 0; --i) {
        const tpl::Shape &s = m_project->shapes.at(i);
        if (!s.hasEpsProfile())
            continue;
        if (!shapePath(s).contains(mm))
            continue;
        // Через кэш: формула уже разобрана для заливки, второй раз разбирать её на
        // каждое движение мыши незачем.
        const EpsField *field = epsField(s);
        if (!field)
            continue;
        const QPointF o = s.epsOrigin();
        const auto v = field->prog.evalEps(mm.x() - o.x(), mm.y() - o.y());
        if (!v)
            continue;
        if (shapeName)
            *shapeName = s.name.isEmpty() ? tpl::tr("фигура %1", "shape %1").arg(s.id) : s.name;
        return v;
    }
    return std::nullopt;
}

void GeometryView::drawEpsColorBar(QPainter *p) const
{
    if (!m_project || !settings::values().epsColorBar)
        return;

    // Шкала общая на все профили: диапазон берём по объединению, иначе два одинаковых
    // цвета в разных фигурах означали бы разные eps, и шкала обманывала бы.
    bool any = false;
    double lo = 0.0, hi = 0.0;
    for (const tpl::Shape &s : m_project->shapes) {
        const EpsField *f = epsField(s);
        if (!f)
            continue;
        lo = any ? std::min(lo, f->lo) : f->lo;
        hi = any ? std::max(hi, f->hi) : f->hi;
        any = true;
    }
    if (!any)
        return;

    p->save();
    p->resetTransform();

    QFont f = font();
    f.setPointSizeF(std::max(7.0, f.pointSizeF() - 1.5));
    p->setFont(f);
    const QFontMetrics fm(f);

    const int vw = viewport()->width();
    const int vh = viewport()->height();
    const int barW = 16;
    const int labelW = std::max(fm.horizontalAdvance(QStringLiteral("00.000")), 44);
    const int right = vw - 12;
    const int top = int(kRulerPx) + 26;
    const int bottom = vh - 26;
    if (bottom - top < 60) { // в низком окне шкала нечитаема — лучше не рисовать вовсе
        p->restore();
        return;
    }
    const QRect bar(right - labelW - barW, top, barW, bottom - top);

    // Подложка под шкалу и подписи: поверх геометрии текст иначе не прочесть.
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(0x0a, 0x0e, 0x16, 215));
    p->drawRoundedRect(QRect(bar.left() - 8, bar.top() - 22, barW + labelW + 14,
                             bar.height() + 34),
                       8, 8);

    // Сама полоса: сверху максимум, снизу минимум — как просили и как принято.
    const settings::Palette pal = settings::values().palette;
    for (int y = 0; y < bar.height(); ++y) {
        const double t = 1.0 - double(y) / std::max(1, bar.height() - 1);
        p->setPen(settings::paletteColor(pal, t));
        p->drawLine(bar.left(), bar.top() + y, bar.right(), bar.top() + y);
    }
    p->setPen(QPen(dtheme::line(), 1.0));
    p->setBrush(Qt::NoBrush);
    p->drawRect(bar);

    p->setPen(dtheme::textDim());
    p->drawText(QRect(bar.left() - 8, bar.top() - 20, barW + labelW + 14, 16),
                Qt::AlignCenter, QStringLiteral("eps"));

    // Пять делений: меньше — не видно формы профиля, больше — подписи слипаются.
    const int ticks = 5;
    p->setPen(dtheme::text());
    for (int i = 0; i < ticks; ++i) {
        const double t = double(i) / (ticks - 1);
        const int y = bar.bottom() - int(t * (bar.height() - 1));
        const double value = lo + (hi - lo) * t;
        p->drawText(QRect(bar.right() + 5, y - fm.height() / 2, labelW, fm.height()),
                    Qt::AlignLeft | Qt::AlignVCenter, QString::asprintf("%.3g", value));
    }
    p->restore();
}

void GeometryView::drawTip(QPainter *p, const QPoint &at, const QString &text) const
{
    if (text.isEmpty())
        return;
    const QFontMetrics fm(font());
    const QStringList lines = text.split(QLatin1Char('\n'));
    int w = 0;
    for (const QString &l : lines)
        w = std::max(w, fm.horizontalAdvance(l));
    const int h = fm.height() * lines.size();

    QRectF box(at.x() + 16, at.y() - h - 16, w + 16, h + 10);
    // Не даём подсказке уехать за край: её читают именно тогда, когда курсор у границы.
    if (box.right() > viewport()->width() - 4)
        box.moveRight(at.x() - 12);
    if (box.top() < 4)
        box.moveTop(at.y() + 18);

    p->setBrush(QColor(0x0a, 0x0e, 0x16, 235));
    p->setPen(QPen(dtheme::accent(), 1.0));
    p->drawRoundedRect(box, 7, 7);
    p->setPen(dtheme::text());
    p->drawText(box.adjusted(8, 5, -8, -5), Qt::AlignLeft | Qt::AlignVCenter, text);
}

void GeometryView::drawRulers(QPainter *p) const
{
    const double px = pxPerMm();
    if (px <= 0.0)
        return;
    const double step = rulerStep(px) * unitScale(m_unit);
    const QString suffix = unitSuffix(m_unit);
    const double k = unitScale(m_unit);

    p->save();
    p->resetTransform();
    QFont f = font();
    f.setPointSizeF(std::max(7.0, f.pointSizeF() - 1.5));
    p->setFont(f);
    const QFontMetrics fm(f);

    const int w = viewport()->width();
    const int h = viewport()->height();
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(0x13, 0x1a, 0x28, 232));
    p->drawRect(QRectF(0, 0, w, kRulerPx));
    p->drawRect(QRectF(0, kRulerPx, kRulerPx, h - kRulerPx));

    const QPointF tl = mapToScene(QPoint(0, 0));
    const QPointF br = mapToScene(QPoint(w, h));
    const double xLo = std::min(tl.x(), br.x()), xHi = std::max(tl.x(), br.x());
    const double yLo = std::min(tl.y(), br.y()), yHi = std::max(tl.y(), br.y());

    p->setPen(dtheme::textDim());
    for (double x = std::ceil(xLo / step) * step; x <= xHi; x += step) {
        const int sx = int(mapFromScene(QPointF(x, 0)).x());
        if (sx < kRulerPx)
            continue;
        p->drawLine(sx, int(kRulerPx) - 5, sx, int(kRulerPx) - 1);
        p->drawText(QRect(sx - 40, 1, 80, int(kRulerPx) - 6), Qt::AlignCenter,
                    QString::number(x / k, 'g', 6));
    }
    for (double y = std::ceil(yLo / step) * step; y <= yHi; y += step) {
        const int sy = int(mapFromScene(QPointF(0, y)).y());
        if (sy < kRulerPx)
            continue;
        p->drawLine(int(kRulerPx) - 5, sy, int(kRulerPx) - 1, sy);
        p->save();
        p->translate(int(kRulerPx) - 7, sy);
        p->rotate(-90);
        p->drawText(QRect(-40, -fm.height(), 80, fm.height()), Qt::AlignCenter,
                    QString::number(y / k, 'g', 6));
        p->restore();
    }
    p->setPen(dtheme::textMute());
    p->drawText(QRect(2, 2, int(kRulerPx) - 3, int(kRulerPx) - 3), Qt::AlignCenter, suffix);
    p->restore();
}

void GeometryView::drawDimensions(QPainter *p, const tpl::Shape &s) const
{
    const double k = unitScale(m_unit);
    const QString u = unitSuffix(m_unit);
    p->save();
    p->resetTransform();
    p->setPen(dtheme::accent());
    const auto label = [&](const QPointF &scenePt, const QString &text, int dx, int dy) {
        const QPointF sc = mapFromScene(scenePt);
        p->drawText(QRectF(sc.x() - 70 + dx, sc.y() - 10 + dy, 140, 20),
                    Qt::AlignCenter, text);
    };

    switch (s.kind) {
    case ShapeKind::Rectangle:
        label(QPointF((s.xMin + s.xMax) / 2, s.yMin),
              QStringLiteral("%1 %2").arg((s.xMax - s.xMin) / k, 0, 'g', 6).arg(u), 0, 16);
        label(QPointF(s.xMin, (s.yMin + s.yMax) / 2),
              QStringLiteral("%1 %2").arg((s.yMax - s.yMin) / k, 0, 'g', 6).arg(u), -78, 0);
        break;
    case ShapeKind::Circle:
        label(s.center, QStringLiteral("R %1 %2").arg(s.rMax / k, 0, 'g', 6).arg(u), 0, 0);
        break;
    case ShapeKind::Polygon:
    case ShapeKind::Polyline: {
        double len = 0.0;
        for (int i = 0; i + 1 < s.points.size(); ++i)
            len += QLineF(s.points.at(i), s.points.at(i + 1)).length();
        if (s.kind == ShapeKind::Polygon && s.points.size() > 2)
            len += QLineF(s.points.last(), s.points.first()).length();
        if (!s.points.isEmpty()) {
            label(s.points.first(),
                  tpl::tr("%1 %2, вершин %3", "%1 %2, %3 vertices").arg(len / k, 0, 'g', 6)
                      .arg(u).arg(s.points.size()), 0, -18);
        }
        break;
    }
    }
    p->restore();
}

void GeometryView::drawPreview(QPainter *p) const
{
    if (!m_project || !m_hasCursor)
        return;

    const double k = unitScale(m_unit);
    const QString u = unitSuffix(m_unit);
    const QPoint sc = mapFromScene(m_cursor);
    QString tip;

    QPen pen(dtheme::accent());
    pen.setCosmetic(true);
    pen.setWidthF(1.8);
    pen.setStyle(Qt::DashLine);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);

    if (m_creating && m_tool == Tool::Rect) {
        const QRectF r = QRectF(m_startMm, m_cursor).normalized();
        p->drawRect(r);
        tip = QStringLiteral("%1 x %2 %3")
                  .arg(r.width() / k, 0, 'g', 6).arg(r.height() / k, 0, 'g', 6).arg(u);
    } else if (m_creating && (m_tool == Tool::Circle || m_tool == Tool::Arc) && !m_arcSweep) {
        const double r = QLineF(m_startMm, m_cursor).length();
        p->drawEllipse(m_startMm, r, r);
        p->drawLine(m_startMm, m_cursor);
        tip = QStringLiteral("R = %1 %2").arg(r / k, 0, 'g', 6).arg(u);
    } else if (m_arcSweep) {
        const double a1 = std::atan2(m_cursor.y() - m_arcCenter.y(),
                                     m_cursor.x() - m_arcCenter.x());
        double span = a1 - m_arcFrom;
        while (span < 0) span += 2 * M_PI;
        // Точками, а не arcTo: см. shapePath — иначе предпросмотр покажет не тот сектор.
        const QVector<QPointF> pts = arcPoints(m_arcCenter, m_arcR, m_arcFrom, m_arcFrom + span);
        QPainterPath path;
        path.moveTo(m_arcCenter);
        for (const QPointF &pt : pts)
            path.lineTo(pt);
        path.closeSubpath();
        p->drawPath(path);
        tip = tpl::tr("R = %1 %2, раскрыв %3 град", "R = %1 %2, span %3 deg")
                  .arg(m_arcR / k, 0, 'g', 6).arg(u).arg(span * 180.0 / M_PI, 0, 'f', 1);
    } else if (m_creating && m_tool == Tool::Port) {
        const tpl::Box &b = m_project->box;
        const PortSide side = sideForDrag(m_startMm, m_cursor);
        const bool alongY = (side == PortSide::Left || side == PortSide::Right);
        // Плоскость входа — там, где нажали: вход не обязан стоять на границе.
        const double fixed = std::clamp(alongY ? m_startMm.x() : m_startMm.y(),
                                        alongY ? b.xMin : b.yMin,
                                        alongY ? b.xMax : b.yMax);
        const double v0 = alongY ? m_startMm.y() : m_startMm.x();
        const double v1 = alongY ? m_cursor.y() : m_cursor.x();
        const QPointF a = alongY ? QPointF(fixed, v0) : QPointF(v0, fixed);
        const QPointF c = alongY ? QPointF(fixed, v1) : QPointF(v1, fixed);
        QPen thick(dtheme::ok());
        thick.setCosmetic(true);
        thick.setWidthF(5.0);
        p->setPen(thick);
        p->drawLine(a, c);

        static const char *const names[][2] = {{"волна пойдёт вправо", "wave goes right"},
                                               {"волна пойдёт влево", "wave goes left"},
                                               {"волна пойдёт вниз", "wave goes down"},
                                               {"волна пойдёт вверх", "wave goes up"}};
        const int idx = (side == PortSide::Left) ? 0 : (side == PortSide::Right) ? 1
                        : (side == PortSide::Top) ? 2 : 3;
        const double ap = std::fabs(v1 - v0);
        tip = tpl::tr("%1, апертура %2 %3", "%1, aperture %2 %3")
                  .arg(tpl::tr(names[idx][0], names[idx][1])).arg(ap / k, 0, 'g', 6).arg(u);
        if (ap > 0.0) {
            tip += tpl::tr("\nодномодовый диапазон %1...%2 ГГц", "\nsingle-mode range %1...%2 GHz")
                       .arg(299.792458 / (2.0 * ap), 0, 'f', 2)
                       .arg(299.792458 / ap, 0, 'f', 2);
        }
    } else if (m_tool == Tool::ArcLine && !m_chain.isEmpty()) {
        if (m_chain.size() == 1) {
            // Пока набран один конец — показываем хорду, дуги ещё нет.
            p->drawLine(m_chain.first(), m_cursor);
            const double len = QLineF(m_chain.first(), m_cursor).length();
            tip = tpl::tr("хорда %1 %2 — клик ставит второй конец", "chord %1 %2: click sets the second end")
                      .arg(len / k, 0, 'f', 1).arg(u);
        } else {
            const QVector<QPointF> pts = arcThroughChain(m_cursor);
            QPainterPath path;
            if (!pts.isEmpty()) {
                path.moveTo(pts.first());
                for (int i = 1; i < pts.size(); ++i)
                    path.lineTo(pts.at(i));
            }
            p->drawPath(path);

            // Концы дуги отмечаем точками: без них не видно, что тянется середина,
            // а не вся кривая. Радиус — в пикселях, поэтому переводим через масштаб.
            const double rr = 4.0 / std::max(1e-9, pxPerMm());
            p->setBrush(dtheme::accent());
            p->drawEllipse(m_chain.at(0), rr, rr);
            p->drawEllipse(m_chain.at(1), rr, rr);
            p->setBrush(Qt::NoBrush);

            double len = 0.0;
            for (int i = 1; i < pts.size(); ++i)
                len += QLineF(pts.at(i - 1), pts.at(i)).length();
            // Коротко: длинная подсказка не влезает в холст и обрезается краем.
            tip = tpl::tr("дуга %1 %2, вершин %3", "arc %1 %2, %3 vertices")
                      .arg(len / k, 0, 'f', 1).arg(u).arg(pts.size());
        }
    } else if (!m_chain.isEmpty()) {
        QPainterPath path;
        path.moveTo(m_chain.first());
        for (int i = 1; i < m_chain.size(); ++i)
            path.lineTo(m_chain.at(i));
        path.lineTo(m_cursor);
        if (m_tool == Tool::Contour && m_chain.size() >= 2)
            path.lineTo(m_chain.first());
        p->drawPath(path);

        const QLineF seg(m_chain.last(), m_cursor);
        tip = tpl::tr("%1 %2, %3 град", "%1 %2, %3 deg")
                  .arg(seg.length() / k, 0, 'g', 6).arg(u).arg(seg.angle(), 0, 'f', 1);
        tip += tpl::tr("\nвершин %1 — двойной клик или Enter завершает", "\n%1 vertices: double-click or Enter finishes")
                   .arg(m_chain.size());
    } else if (m_dragging || m_handle >= 0) {
        tip = QStringLiteral("X %1   Y %2 %3")
                  .arg(m_cursor.x() / k, 0, 'g', 6).arg(m_cursor.y() / k, 0, 'g', 6).arg(u);
    }

    if (!m_note.isEmpty())
        tip += (tip.isEmpty() ? QString() : QStringLiteral("\n")) + m_note;

    // Куда прилипли — надо показывать: невидимая привязка выглядит как «мышь врёт».
    if (m_lastSnap.toObject) {
        p->save();
        p->resetTransform();
        QPen mark(dtheme::warn());
        mark.setWidthF(2.0);
        p->setPen(mark);
        p->setBrush(Qt::NoBrush);
        const QPointF q = mapFromScene(m_lastSnap.pos);
        p->drawRect(QRectF(q.x() - 6, q.y() - 6, 12, 12));
        p->drawLine(q.x() - 10, q.y(), q.x() + 10, q.y());
        p->drawLine(q.x(), q.y() - 10, q.x(), q.y() + 10);
        p->restore();
        tip += (tip.isEmpty() ? QString() : QStringLiteral("\n")) + m_lastSnap.what;
    } else if (m_tool != Tool::Select) {
        // Перекрестье под курсором: без него на пустом холсте не понять, куда ткнёшь.
        p->save();
        p->resetTransform();
        QPen cross(dtheme::accent());
        cross.setWidthF(1.0);
        p->setPen(cross);
        p->drawLine(sc.x() - 9, sc.y(), sc.x() + 9, sc.y());
        p->drawLine(sc.x(), sc.y() - 9, sc.x(), sc.y() + 9);
        p->restore();
    }

    // Значение eps под курсором — строкой в ТУ ЖЕ подсказку, а не отдельной панелью:
    // две панели у одного курсора наезжают друг на друга и обе становятся нечитаемы.
    // Держится, только пока курсор в области с профилем: ушёл — строка исчезла.
    if (settings::values().epsTooltip) {
        QString name;
        if (const auto v = epsAt(m_rawCursor, &name)) {
            tip += (tip.isEmpty() ? QString() : QStringLiteral("\n"))
                   + QStringLiteral("%1: eps = %2")
                         .arg(name, QString::asprintf("%.4g", *v));
        }
    }

    if (!tip.isEmpty()) {
        p->save();
        p->resetTransform();
        drawTip(p, sc, tip);
        p->restore();
    }
}

void GeometryView::drawSweep(QPainter *p) const
{
    const tpl::AngleSweep &sw = m_project->sweep;
    if (!sw.active())
        return;
    // Ось считается заново на каждой отрисовке: при оси «в центре объекта» она едет
    // вместе с фигурой, пока её тащат.
    const QPointF pivot = tpl::sweepPivot(*m_project);

    // Контуры объекта на нескольких углах — видно, какую область он заметает. Все
    // углы рисовать незачем: при шаге в градус это чёрное пятно.
    const int n = int(sw.angles.size());
    const int stride = std::max(1, n / 8);
    QPen ghost(dtheme::warn());
    ghost.setCosmetic(true);
    ghost.setWidthF(1.0);
    ghost.setStyle(Qt::DashLine);
    p->setPen(ghost);
    p->setBrush(Qt::NoBrush);
    double reach = 0.0;
    for (const tpl::Shape &s : m_project->shapes) {
        if (!sw.rotates(s.id))
            continue;
        for (int i = 0; i < n; i += stride) {
            if (std::fabs(sw.angles.at(i)) < 1e-9)
                continue; // исходное положение и так нарисовано
            p->drawPath(shapePath(tpl::rotatedShape(s, pivot, sw.angles.at(i))));
        }
        double xLo = 0, xHi = 0, yLo = 0, yHi = 0;
        s.bounds(&xLo, &xHi, &yLo, &yHi);
        for (const QPointF &c : {QPointF(xLo, yLo), QPointF(xLo, yHi), QPointF(xHi, yLo),
                                 QPointF(xHi, yHi)})
            reach = std::max(reach, std::hypot(c.x() - pivot.x(), c.y() - pivot.y()));
    }
    // Окружность, которую объект не покидает, и ось поворота.
    QPen ring(dtheme::warn());
    ring.setCosmetic(true);
    ring.setWidthF(1.0);
    ring.setStyle(Qt::DotLine);
    p->setPen(ring);
    p->drawEllipse(pivot, reach, reach);

    p->save();
    p->resetTransform();
    const QPointF c = mapFromScene(pivot);
    QPen cross(dtheme::warn());
    cross.setWidthF(2.0);
    p->setPen(cross);
    p->drawLine(c + QPointF(-7, 0), c + QPointF(7, 0));
    p->drawLine(c + QPointF(0, -7), c + QPointF(0, 7));
    p->drawEllipse(c, 3.5, 3.5);
    p->restore();
}

void GeometryView::drawForeground(QPainter *p, const QRectF &)
{
    if (!m_project)
        return;
    // Сначала площадные фигуры, потом контурные.
    //
    // ⚠️ Порядок здесь не косметика. Стенку в языке задаёт КОНТУР, а заливку —
    // область, и у залитой области тоже есть край. Когда заливка ложилась поверх
    // стенок, её край читался как стенка: в микрополосковом изгибе два
    // прямоугольника заполнения стыкуются ровно на повороте, и граница стыка
    // выглядела перегородкой поперёк канала. Задание при этом правильное —
    // врала картинка. Стенки рисуются последними и видны всегда.
    for (const tpl::Shape &s : m_project->shapes) {
        if (!s.isContour())
            drawShape(p, s);
    }
    for (const tpl::Shape &s : m_project->shapes) {
        if (s.isContour())
            drawShape(p, s);
    }
    drawSweep(p);
    for (const Port &port : m_project->ports)
        drawPort(p, port);

    // Картинка: только то, что посчитает ядро. Ни ручек, ни линеек, ни подсказок.
    if (m_preview)
        return;

    // Контур диаграммы направленности: ядро ставит его ровно в 10 шагах сетки от края
    // области, и снаружи него должно быть свободное пространство. Пунктир показывает
    // правило глазами — плазма за ним или поглотитель поперёк него видны сразу.
    if (m_project->output.directionalPattern && m_project->delta > 0.0) {
        const double mg = tpl::patternContourMargin(*m_project);
        const tpl::Box &b = m_project->box;
        if (b.xMax - b.xMin > 2.0 * mg && b.yMax - b.yMin > 2.0 * mg) {
            QPen cp(dtheme::warn());
            cp.setCosmetic(true);
            cp.setWidthF(1.2);
            cp.setStyle(Qt::DashLine);
            p->save();
            p->setPen(cp);
            p->setBrush(Qt::NoBrush);
            p->drawRect(QRectF(QPointF(b.xMin + mg, b.yMin + mg), QPointF(b.xMax - mg, b.yMax - mg)));
            p->restore();
        }
    }

    if (m_sel.kind == Selection::ShapeBody || m_sel.kind == Selection::ShapeEdge
        || m_sel.kind == Selection::ShapeVertex) {
        if (const tpl::Shape *s = shapeById(m_sel.id))
            drawDimensions(p, *s);
    }
    drawHandles(p);
    drawPreview(p);
    drawRulers(p);
    drawEpsColorBar(p);

    // Пустой холст сам говорит, что с ним делать: иначе новый пользователь ищет
    // кнопку «добавить», которой здесь нет.
    if (m_project->shapes.isEmpty() && m_project->ports.isEmpty() && m_chain.isEmpty()) {
        p->save();
        p->resetTransform();
        QFont f = font();
        f.setPointSizeF(f.pointSizeF() + 1.0);
        p->setFont(f);
        p->setPen(dtheme::textMute());
        p->drawText(viewport()->rect(), Qt::AlignCenter,
                    tpl::tr("Пусто. Возьмите инструмент слева и рисуйте мышью:\n"
                            "прямоугольник — стенки, линия — перегородка,\n"
                            "порт — вход на границе пунктирной области.",
                            "Empty. Take a tool on the left and draw with the mouse:\nrectangle for walls, "
                            "line for a partition,\nport for an input on the border of the dashed domain."));
        p->restore();
    }
}

// -------------------------------------------------------------------- события

void GeometryView::wheelEvent(QWheelEvent *e)
{
    if (m_preview) { // картинка каталога: мышь и клавиши ей не нужны
        e->ignore();
        return;
    }
    const double f = e->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    scale(f, f);
    viewport()->update();
    e->accept();
}

void GeometryView::beginEdit()
{
    emit aboutToEdit();
}

void GeometryView::finishEdit()
{
    viewport()->update();
    emit projectEdited();
}

void GeometryView::commitNewShape(const tpl::Shape &s)
{
    beginEdit();
    m_project->shapes << s;
    m_sel = {Selection::ShapeBody, s.id, -1};
    finishEdit();
    emit selectionChanged();
}

void GeometryView::mousePressEvent(QMouseEvent *e)
{
    if (m_preview) { // картинка каталога: мышь и клавиши ей не нужны
        e->ignore();
        return;
    }
    if (!m_project) {
        QGraphicsView::mousePressEvent(e);
        return;
    }
    setFocus();
    m_shiftHeld = e->modifiers() & Qt::ShiftModifier;

    // ⚠️ Ctrl+ЛКМ раньше сдвигал холст, теперь набирает выделение (решение 2026-09-06).
    //
    // Выбрать несколько фигур больше нечем — а без этого «Соединить» бесполезно, да и
    // удаление с дублированием работают по одной штуке. Сдвиг никуда не делся: он на
    // средней кнопке, а для мышей без неё добавлен Alt+ЛКМ.
    if (e->button() == Qt::MiddleButton
        || (e->button() == Qt::LeftButton && (e->modifiers() & Qt::AltModifier))) {
        m_panning = true;
        m_panStart = e->pos();
        setCursor(Qt::ClosedHandCursor);
        e->accept();
        return;
    }

    if (e->button() == Qt::RightButton) {
        if (!m_chain.isEmpty()) {
            finishChain();
            e->accept();
            return;
        }
        QGraphicsView::mousePressEvent(e);
        return;
    }
    if (e->button() != Qt::LeftButton) {
        QGraphicsView::mousePressEvent(e);
        return;
    }

    const QPointF mm = toScene(e->pos());
    m_rawCursor = mm; // клик без предшествующего движения тоже задаёт точку чтения
    m_cursor = mm;
    m_hasCursor = true;
    m_note.clear();

    switch (m_tool) {
    case Tool::Select: {
        const int role = handleAt(e->pos());
        if (role >= 0) {
            m_handle = role;
            m_excludeId = m_sel.id; // к самому себе не привязываемся
            if (role >= kRoleVertex && role < kRolePortFrom)
                setSelection({Selection::ShapeVertex, m_sel.id, role - kRoleVertex});
            beginEdit();
            if (role == kRoleRotate)
                beginRotation();
            e->accept();
            return;
        }
        const Selection s = pick(e->pos());

        // Ctrl — набор нескольких фигур. Повторный Ctrl-клик по уже выделенной её
        // снимает: иначе промахнувшийся выбор нечем исправить, кроме как начать заново.
        if (e->modifiers() & Qt::ControlModifier) {
            if (!s.isEmpty() && shapeById(s.id)) {
                QVector<int> ids = selectedShapeIds();
                if (ids.contains(s.id)) {
                    ids.removeAll(s.id);
                } else {
                    ids << s.id;
                }
                setSelectedIds(ids);
            }
            e->accept();
            return;
        }

        // Клик по фигуре, которая уже входит в набор, набор НЕ сбрасывает: иначе
        // выделенную группу нельзя было бы перетащить — первое же нажатие свело бы
        // её к одной фигуре.
        const bool insideGroup = m_multi.size() > 1 && !s.isEmpty() && m_multi.contains(s.id);
        if (insideGroup) {
            m_sel = s;
            emit selectionChanged();
        } else {
            setSelection(s);
        }
        if (!s.isEmpty()) {
            m_dragging = true;
            m_lastMm = mm;
            m_excludeId = s.id;
            beginEdit();
        }
        e->accept();
        return;
    }
    case Tool::Rect:
    case Tool::Circle:
        m_creating = true;
        m_startMm = mm;
        e->accept();
        return;
    case Tool::Arc:
        if (m_arcSweep) {
            // Второй клик закрывает раскрыв сектора.
            const double a1 = std::atan2(mm.y() - m_arcCenter.y(), mm.x() - m_arcCenter.x());
            double span = a1 - m_arcFrom;
            while (span <= 0) span += 2 * M_PI;
            tpl::Shape s;
            s.id = nextId();
            s.name = freshShapeName(tpl::tr("Сектор", "Sector"));
            s.kind = ShapeKind::Circle;
            s.material = Material::Metal;
            s.center = m_arcCenter;
            s.rMin = 0.0;
            s.rMax = m_arcR;
            s.fiMin = m_arcFrom < 0 ? m_arcFrom + 2 * M_PI : m_arcFrom;
            s.fiMax = s.fiMin + span;
            m_arcSweep = false;
            commitNewShape(s);
            updateHint();
        } else {
            m_creating = true;
            m_startMm = mm;
        }
        e->accept();
        return;
    case Tool::Line:
    case Tool::Contour:
        m_chain << mm;
        updateHint();
        viewport()->update();
        e->accept();
        return;
    case Tool::ArcLine:
        // Два клика — концы дуги, третий — куда её выгнуть.
        if (m_chain.size() < 2) {
            m_chain << mm;
        } else {
            finishChain();
            e->accept();
            return;
        }
        updateHint();
        viewport()->update();
        e->accept();
        return;
    case Tool::Port: {
        // Сторону определит НАПРАВЛЕНИЕ протяжки, а не точка нажатия: здесь только
        // запоминаем начало.
        m_startMm = mm;
        m_portSide = sideForDrag(mm, mm);
        m_creating = true;
        e->accept();
        return;
    }
    }
    QGraphicsView::mousePressEvent(e);
}

void GeometryView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_preview) { // картинка каталога: мышь и клавиши ей не нужны
        e->ignore();
        return;
    }
    if (m_panning) {
        const QPoint d = e->pos() - m_panStart;
        m_panStart = e->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - d.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - d.y());
        e->accept();
        return;
    }
    if (!m_project) {
        QGraphicsView::mouseMoveEvent(e);
        return;
    }

    m_shiftHeld = e->modifiers() & Qt::ShiftModifier;
    QPointF mm = toScene(e->pos());
    // Значение eps показываем В ТОЧКЕ, куда смотрит мышь, а не в узле, к которому её
    // притянула привязка: спрашивают «сколько здесь», а не «сколько в ближайшем узле».
    m_rawCursor = mm;

    // Shift держит направление кратным 45 градусам — иначе ровную стенку мышью не провести.
    if (m_shiftHeld) {
        QPointF base;
        bool haveBase = false;
        if (!m_chain.isEmpty()) { base = m_chain.last(); haveBase = true; }
        else if (m_creating) { base = m_startMm; haveBase = true; }
        if (haveBase) {
            const QPointF d = mm - base;
            const double a = std::round(std::atan2(d.y(), d.x()) / (M_PI / 4)) * (M_PI / 4);
            const double len = std::hypot(d.x(), d.y());
            mm = snapped(base + QPointF(len * std::cos(a), len * std::sin(a)));
        }
    }

    m_cursor = mm;
    m_hasCursor = true;
    emit cursorMoved(mm);

    if (m_handle == kRoleRotate) {
        applyRotation(mm);
        viewport()->update();
        e->accept();
        return;
    }
    if (m_handle >= 0) {
        applyHandleDrag(mm);
        viewport()->update();
        e->accept();
        return;
    }
    if (m_dragging) {
        moveSelection(mm - m_lastMm);
        m_lastMm = mm;
        viewport()->update();
        e->accept();
        return;
    }

    viewport()->update();
    QGraphicsView::mouseMoveEvent(e);
}

void GeometryView::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_preview) { // картинка каталога: мышь и клавиши ей не нужны
        e->ignore();
        return;
    }
    if (m_panning) {
        m_panning = false;
        setCursor(m_tool == Tool::Select ? Qt::ArrowCursor : Qt::CrossCursor);
        e->accept();
        return;
    }
    if (!m_project || e->button() != Qt::LeftButton) {
        QGraphicsView::mouseReleaseEvent(e);
        return;
    }

    if (m_handle >= 0) {
        if (tpl::Shape *s = shapeById(m_sel.id)) {
            if (s->kind == ShapeKind::Rectangle)
                normalizeRect(s);
        }
        m_handle = -1;
        m_excludeId = -1;
        finishEdit();
        emit selectionChanged(); // поворот мог сменить тип фигуры или сторону порта
        e->accept();
        return;
    }
    if (m_dragging) {
        m_dragging = false;
        m_excludeId = -1;
        finishEdit();
        e->accept();
        return;
    }
    if (!m_creating) {
        QGraphicsView::mouseReleaseEvent(e);
        return;
    }

    m_creating = false;
    const double step = gridStep();
    const QPointF mm = m_cursor;

    switch (m_tool) {
    case Tool::Rect: {
        const QRectF r = QRectF(m_startMm, mm).normalized();
        if (r.width() < step || r.height() < step) {
            viewport()->update();
            break; // случайный клик — фигуру нулевого размера не создаём
        }
        tpl::Shape s;
        s.id = nextId();
        s.name = freshShapeName(tpl::tr("Прямоугольник", "Rectangle"));
        s.kind = ShapeKind::Rectangle;
        s.material = Material::Metal;
        s.xMin = r.left(); s.xMax = r.right();
        s.yMin = r.top();  s.yMax = r.bottom();
        commitNewShape(s);
        break;
    }
    case Tool::Circle: {
        const double r = QLineF(m_startMm, mm).length();
        if (r < step) {
            viewport()->update();
            break;
        }
        tpl::Shape s;
        s.id = nextId();
        s.name = freshShapeName(tpl::tr("Круг", "Circle"));
        s.kind = ShapeKind::Circle;
        s.material = Material::Metal;
        s.center = m_startMm;
        s.rMin = 0.0;
        s.rMax = r;
        s.fiMin = 0.0;
        s.fiMax = 2 * M_PI;
        commitNewShape(s);
        break;
    }
    case Tool::Arc: {
        const double r = QLineF(m_startMm, mm).length();
        if (r < step) {
            viewport()->update();
            break;
        }
        m_arcCenter = m_startMm;
        m_arcR = r;
        m_arcFrom = std::atan2(mm.y() - m_startMm.y(), mm.x() - m_startMm.x());
        m_arcSweep = true;
        updateHint();
        viewport()->update();
        break;
    }
    case Tool::Port: {
        m_portSide = sideForDrag(m_startMm, mm);
        const bool alongY = (m_portSide == PortSide::Left || m_portSide == PortSide::Right);
        const double a = std::min(alongY ? m_startMm.y() : m_startMm.x(),
                                  alongY ? mm.y() : mm.x());
        const double b = std::max(alongY ? m_startMm.y() : m_startMm.x(),
                                  alongY ? mm.y() : mm.x());
        if (b - a < step) {
            viewport()->update();
            break;
        }
        Port p;
        p.id = nextId();
        p.name = freshPortName();
        // Плоскость входа — поперечная координата протяжки. Вход законно стоит и
        // внутри области, поэтому к границе его не притягиваем.
        const tpl::Box &box = m_project->box;
        const double plane = std::clamp(alongY ? m_startMm.x() : m_startMm.y(),
                                        alongY ? box.xMin : box.yMin,
                                        alongY ? box.xMax : box.yMax);
        p.plane = plane;
        p.side = directionForPlane(alongY, plane, box);
        p.from = a;
        p.to = b;
        // Первый порт делаем активным: пассивные в одиночку не считают ничего.
        p.active = m_project->activePorts().isEmpty();
        beginEdit();
        m_project->ports << p;
        m_sel = {Selection::PortBody, p.id, -1};
        finishEdit();
        emit selectionChanged();
        break;
    }
    default:
        break;
    }
    e->accept();
}

void GeometryView::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (m_preview) { // картинка каталога: мышь и клавиши ей не нужны
        e->ignore();
        return;
    }
    if (!m_chain.isEmpty()) {
        finishChain();
        e->accept();
        return;
    }
    if (m_tool == Tool::Select) {
        const Selection s = pick(e->pos());
        if (!s.isEmpty()) {
            setSelection(s);
            emit editRequested(s.id);
            e->accept();
            return;
        }
    }
    QGraphicsView::mouseDoubleClickEvent(e);
}

void GeometryView::keyPressEvent(QKeyEvent *e)
{
    if (m_preview) { // картинка каталога: мышь и клавиши ей не нужны
        e->ignore();
        return;
    }
    switch (e->key()) {
    case Qt::Key_Escape:
        if (!m_chain.isEmpty() || m_arcSweep || m_creating)
            cancelDrawing();
        else
            setSelection({});
        e->accept();
        return;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (!m_chain.isEmpty()) {
            finishChain();
            e->accept();
            return;
        }
        break;
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        deleteSelected();
        e->accept();
        return;
    default:
        break;
    }
    QGraphicsView::keyPressEvent(e);
}

void GeometryView::leaveEvent(QEvent *e)
{
    m_hasCursor = false;
    m_lastSnap = Snap{};
    viewport()->update();
    QGraphicsView::leaveEvent(e);
}

void GeometryView::contextMenuEvent(QContextMenuEvent *e)
{
    if (m_preview) { // картинка каталога: мышь и клавиши ей не нужны
        e->ignore();
        return;
    }
    if (!m_chain.isEmpty())
        return; // правая кнопка сейчас завершает ломаную

    const Selection s = pick(e->pos());

    // Клик по фигуре ИЗ набора набор не сбрасывает — иначе пункт «Соединить» открыть
    // было бы нельзя: правая кнопка сама же разрушала бы то, что соединяют. Клик мимо
    // всего при наборе тоже его сохраняет: человек метится в пустоту рядом с группой,
    // а не отменяет выбор.
    const bool keepGroup = m_multi.size() > 1 && (s.isEmpty() || m_multi.contains(s.id));
    if (!keepGroup) {
        if (s.isEmpty())
            return;
        setSelection(s);
    } else if (!s.isEmpty()) {
        m_sel = s;
    }

    const QVector<int> ids = selectedShapeIds();
    int mergeable = 0;
    for (int id : ids) {
        const tpl::Shape *sh = shapeById(id);
        if (sh && (sh->kind == ShapeKind::Polyline || sh->kind == ShapeKind::Polygon))
            ++mergeable;
    }

    QMenu menu(this);
    QAction *edit = nullptr;
    if (ids.size() <= 1)
        edit = menu.addAction(tpl::tr("Свойства...", "Properties..."));

    QAction *join = menu.addAction(ids.size() > 1
                                       ? tpl::tr("Соединить в одну фигуру (%1)", "Join into one shape (%1)")
                                             .arg(ids.size())
                                       : tpl::tr("Соединить в одну фигуру", "Join into one shape"));
    // Доступен ровно тогда, когда сработает: обещать пунктом и отказывать нажатием —
    // худший вид меню.
    join->setEnabled(ids.size() > 1 && mergeable == ids.size());
    if (ids.size() > 1 && mergeable != ids.size()) {
        join->setToolTip(tpl::tr("Соединять можно только линии, дуги и контуры", "Only lines, arcs and polygons can be joined"));
        menu.setToolTipsVisible(true);
    }

    QAction *dup = menu.addAction(ids.size() > 1
                                      ? tpl::tr("Дублировать (%1)", "Duplicate (%1)").arg(ids.size())
                                      : tpl::tr("Дублировать", "Duplicate"));
    menu.addSeparator();
    QAction *del = menu.addAction(ids.size() > 1
                                      ? tpl::tr("Удалить (%1)", "Delete (%1)").arg(ids.size())
                                      : tpl::tr("Удалить", "Delete"));

    QAction *chosen = menu.exec(e->globalPos());
    if (chosen == nullptr)
        return;
    if (chosen == edit)
        emit editRequested(m_sel.id);
    else if (chosen == join)
        emit mergeRequested();
    else if (chosen == dup)
        duplicateSelected();
    else if (chosen == del)
        deleteSelected();
}

// -------------------------------------------------------------------- правки

void GeometryView::applyHandleDrag(const QPointF &mm)
{
    if (m_handle >= kRolePortFrom && m_handle <= kRolePortTo) {
        Port *p = portById(m_sel.id);
        if (!p || !m_project)
            return;
        const double v = portAlongY(p->side) ? mm.y() : mm.x();
        if (m_handle == kRolePortFrom)
            p->from = v;
        else
            p->to = v;
        if (p->from > p->to)
            std::swap(p->from, p->to);
        return;
    }

    tpl::Shape *s = shapeById(m_sel.id);
    if (!s)
        return;

    if (m_handle >= kRoleVertex) {
        const int i = m_handle - kRoleVertex;
        if (i >= 0 && i < s->points.size())
            s->points[i] = mm;
        return;
    }

    switch (s->kind) {
    case ShapeKind::Rectangle:
        // Ручки 0..3 — углы, 4+сторона — середины сторон прямоугольника.
        switch (m_handle) {
        case 0: s->xMin = mm.x(); s->yMin = mm.y(); break;
        case 1: s->xMax = mm.x(); s->yMin = mm.y(); break;
        case 2: s->xMax = mm.x(); s->yMax = mm.y(); break;
        case 3: s->xMin = mm.x(); s->yMax = mm.y(); break;
        case 4 + EdgeBottom: s->yMin = mm.y(); break;
        case 4 + EdgeRight:  s->xMax = mm.x(); break;
        case 4 + EdgeTop:    s->yMax = mm.y(); break;
        case 4 + EdgeLeft:   s->xMin = mm.x(); break;
        default: break;
        }
        break;
    case ShapeKind::Circle: {
        const double r = QLineF(s->center, mm).length();
        const double a = std::atan2(mm.y() - s->center.y(), mm.x() - s->center.x());
        // Ручки сектора: 0 — внешний радиус, 1 — внутренний, 2 и 3 — начальный и конечный угол.
        switch (m_handle) {
        case 0: s->rMax = std::max(r, s->rMin + gridStep()); break;
        case 1: s->rMin = std::clamp(r, 0.0, s->rMax - gridStep()); break;
        case 2: s->fiMin = (a < 0 ? a + 2 * M_PI : a); break;
        case 3: {
            double v = (a < 0 ? a + 2 * M_PI : a);
            if (v <= s->fiMin)
                v += 2 * M_PI;
            s->fiMax = v;
            break;
        }
        default: break;
        }
        break;
    }
    default:
        break;
    }
}

void GeometryView::beginRotation()
{
    m_note.clear();
    if (!m_project)
        return;

    if (m_sel.kind == Selection::PortBody) {
        if (const Port *p = portById(m_sel.id))
            m_rotPivot = portCenter(*p);
        return;
    }
    tpl::Shape *s = shapeById(m_sel.id);
    if (!s)
        return;

    // Наклонный прямоугольник примитивом RECT_STAT невыразим — у него только
    // X_min/X_max/Y_min/Y_max. Зато выразим контуром, поэтому при первом же повороте
    // превращаем фигуру в замкнутый контур и говорим об этом.
    if (s->kind == ShapeKind::Rectangle) {
        s->points = {QPointF(s->xMin, s->yMin), QPointF(s->xMax, s->yMin),
                     QPointF(s->xMax, s->yMax), QPointF(s->xMin, s->yMax)};
        s->kind = ShapeKind::Polygon;
        for (EdgeSpec &e : s->edges)
            e = EdgeSpec{};
        m_note = tpl::tr("прямоугольник стал контуром: по осям его держит язык задания", "the rectangle became a polygon: the task language keeps it axis-aligned");
    }

    double xLo = 0, xHi = 0, yLo = 0, yHi = 0;
    s->bounds(&xLo, &xHi, &yLo, &yHi);
    m_rotPivot = (s->kind == ShapeKind::Circle) ? s->center
                                                : QPointF((xLo + xHi) / 2, (yLo + yHi) / 2);
    m_rotOriginal = *s;
    const QPointF h = rotateHandlePos();
    m_rotStartAngle = std::atan2(h.y() - m_rotPivot.y(), h.x() - m_rotPivot.x());
}

void GeometryView::applyRotation(const QPointF &mm)
{
    if (!m_project)
        return;

    if (m_sel.kind == Selection::PortBody) {
        Port *p = portById(m_sel.id);
        if (!p)
            return;
        // Апертура входа идёт либо вдоль Y (INPUT_X), либо вдоль X (INPUT_Y) —
        // третьего в языке нет. Поэтому поворот разворачивает её на 90 градусов,
        // оставляя вход в том же месте.
        const QPointF d = mm - m_rotPivot;
        const bool alongY = std::fabs(d.y()) >= std::fabs(d.x());
        const bool was = portAlongY(p->side);
        const double width = p->to - p->from;
        if (alongY == was)
            return;

        const tpl::Box &b = m_project->box;
        const double newPlane = alongY ? m_rotPivot.x() : m_rotPivot.y();
        const double mid = alongY ? m_rotPivot.y() : m_rotPivot.x();
        p->plane = std::clamp(newPlane, alongY ? b.xMin : b.yMin, alongY ? b.xMax : b.yMax);
        p->from = mid - width / 2.0;
        p->to = mid + width / 2.0;
        p->side = directionForPlane(alongY, *p->plane, b);
        m_note = tpl::tr("апертура входа бывает только вдоль X или вдоль Y — "
                         "наклонных входов язык задания не знает",
                         "a port aperture can only run along X or along Y: the task language has no "
                         "oblique ports");
        return;
    }

    tpl::Shape *s = shapeById(m_sel.id);
    if (!s)
        return;

    double a = std::atan2(mm.y() - m_rotPivot.y(), mm.x() - m_rotPivot.x()) - m_rotStartAngle;
    if (m_shiftHeld) // Shift — кратно 15 градусам
        a = std::round(a / (M_PI / 12)) * (M_PI / 12);

    if (s->kind == ShapeKind::Circle) {
        s->fiMin = m_rotOriginal.fiMin + a;
        s->fiMax = m_rotOriginal.fiMax + a;
        return;
    }
    // Считаем от ИСХОДНОГО состояния, а не от предыдущего кадра: иначе за длинную
    // протяжку фигура «уплывает» от накопленной погрешности.
    const double c = std::cos(a), sn = std::sin(a);
    s->points.resize(m_rotOriginal.points.size());
    for (int i = 0; i < m_rotOriginal.points.size(); ++i) {
        const QPointF d = m_rotOriginal.points.at(i) - m_rotPivot;
        s->points[i] = m_rotPivot + QPointF(d.x() * c - d.y() * sn, d.x() * sn + d.y() * c);
    }
}

void GeometryView::moveSelection(const QPointF &delta)
{
    // Набор двигается целиком: тянут за одну фигуру, едут все выделенные.
    if (m_multi.size() > 1) {
        for (int id : m_multi) {
            tpl::Shape *s = shapeById(id);
            if (!s)
                continue;
            s->xMin += delta.x(); s->xMax += delta.x();
            s->yMin += delta.y(); s->yMax += delta.y();
            s->center += delta;
            for (QPointF &pt : s->points)
                pt += delta;
        }
        m_epsCache.clear();
        return;
    }

    if (m_sel.kind == Selection::PortBody) {
        Port *p = portById(m_sel.id);
        if (!p || !m_project)
            return;
        // Вход двигается свободно ВНУТРИ области: ядро берёт координату плоскости
        // буквально. Вдоль апертуры — сдвиг from/to, поперёк — сдвиг самой плоскости.
        const bool alongY = portAlongY(p->side);
        p->from += alongY ? delta.y() : delta.x();
        p->to += alongY ? delta.y() : delta.x();

        const tpl::Box &b = m_project->box;
        const double lo = alongY ? b.xMin : b.yMin;
        const double hi = alongY ? b.xMax : b.yMax;
        const double moved = portPlane(*p, b) + (alongY ? delta.x() : delta.y());
        p->plane = std::clamp(moved, lo, hi); // за коробку ядро вход не пустит

        // Направление ядро выводит по половине коробки. Держим сторону в согласии
        // с положением, иначе оно молча развернёт падающую волну.
        p->side = directionForPlane(alongY, *p->plane, b);
        return;
    }

    tpl::Shape *s = shapeById(m_sel.id);
    if (!s)
        return;
    switch (s->kind) {
    case ShapeKind::Rectangle:
        s->xMin += delta.x(); s->xMax += delta.x();
        s->yMin += delta.y(); s->yMax += delta.y();
        break;
    case ShapeKind::Circle:
        s->center += delta;
        break;
    case ShapeKind::Polygon:
    case ShapeKind::Polyline:
        for (QPointF &pt : s->points)
            pt += delta;
        break;
    }
}

QVector<QPointF> GeometryView::arcThroughChain(const QPointF &through) const
{
    if (m_chain.size() < 2)
        return {};
    return arcThroughPoints(m_chain.at(0), through, m_chain.at(1), gridStep());
}

void GeometryView::finishChain()
{
    if (!m_project || m_chain.isEmpty())
        return;

    // Двойной клик успевает добавить точку нажатием, а уже потом приходит как
    // двойной: без этого у каждой ломаной был бы лишний нулевой отрезок в конце.
    for (int i = m_chain.size() - 1; i > 0; --i) {
        if (QLineF(m_chain.at(i), m_chain.at(i - 1)).length() < 1e-9)
            m_chain.remove(i);
    }

    // Дуга: два конца набраны кликами, выгиб берём из текущего положения мыши.
    if (m_tool == Tool::ArcLine) {
        if (m_chain.size() < 2) {
            cancelDrawing();
            return;
        }
        const QVector<QPointF> pts = arcThroughChain(m_cursor);
        m_chain.clear();
        if (pts.size() < 2) {
            cancelDrawing();
            return;
        }
        tpl::Shape arc;
        arc.id = nextId();
        arc.kind = ShapeKind::Polyline;
        arc.name = freshShapeName(tpl::tr("Дуга", "Arc"));
        arc.material = Material::Metal;
        arc.points = pts;
        commitNewShape(arc);
        updateHint();
        return;
    }

    const bool closed = (m_tool == Tool::Contour);
    const int need = closed ? 3 : 2;
    if (m_chain.size() < need) {
        cancelDrawing();
        return;
    }

    tpl::Shape s;
    s.id = nextId();
    s.kind = closed ? ShapeKind::Polygon : ShapeKind::Polyline;
    s.name = freshShapeName(closed ? tpl::tr("Контур", "Polygon") : tpl::tr("Линия", "Line"));
    s.material = Material::Metal;
    s.points = m_chain;
    m_chain.clear();
    commitNewShape(s);
    updateHint();
}

bool GeometryView::mergeShapes(const QVector<int> &ids, QString *why)
{
    const auto fail = [why](const QString &text) {
        if (why)
            *why = text;
        return false;
    };
    if (!m_project)
        return fail(tpl::tr("нет задания", "no task"));

    // Сшивать умеем только то, что и есть список точек: линию, дугу, контур.
    // У прямоугольника и круга своя запись в языке, и разбирать их на вершины ради
    // объединения значило бы молча менять то, что человек нарисовал.
    QVector<int> order;
    for (int i = 0; i < m_project->shapes.size(); ++i) {
        const tpl::Shape &s = m_project->shapes.at(i);
        if (!ids.contains(s.id))
            continue;
        if (s.kind != ShapeKind::Polyline && s.kind != ShapeKind::Polygon)
            return fail(tpl::tr("«%1» — не линия и не контур: соединять можно "
                                "линии, дуги и контуры",
                                "\"%1\" is neither a line nor a polygon: only lines, arcs and polygons can be "
                                "joined").arg(s.name));
        if (s.points.size() < 2)
            return fail(tpl::tr("у «%1» меньше двух точек", "\"%1\" has fewer than two points").arg(s.name));
        order << i;
    }
    if (order.size() < 2)
        return fail(tpl::tr("выберите в списке объектов хотя бы две линии — "
                            "с Ctrl или Shift",
                            "select at least two lines in the object list, with Ctrl or Shift"));

    // Куски, готовые к сшиванию. У замкнутого контура повторяем первую точку в конце:
    // иначе при объединении потеряется его последнее звено.
    QVector<QVector<QPointF>> parts;
    for (int i : order) {
        const tpl::Shape &s = m_project->shapes.at(i);
        QVector<QPointF> pts = s.points;
        if (s.kind == ShapeKind::Polygon && pts.first() != pts.last())
            pts << pts.first();
        parts << pts;
    }

    const double tol = std::max(1e-6, gridStep());
    QVector<QPointF> chain = parts.takeFirst();
    double worstGap = 0.0;

    while (!parts.isEmpty()) {
        // Ищем кусок, который ближе всех подходит к любому из концов цепочки.
        int best = -1;
        double bestD = std::numeric_limits<double>::max();
        bool atTail = true;   // приклеиваем к концу цепочки, а не к началу
        bool flip = false;    // кусок надо развернуть

        for (int i = 0; i < parts.size(); ++i) {
            const QPointF a = parts.at(i).first();
            const QPointF b = parts.at(i).last();
            const struct { double d; bool tail; bool rev; } cases[] = {
                {QLineF(chain.last(),  a).length(), true,  false},
                {QLineF(chain.last(),  b).length(), true,  true },
                {QLineF(chain.first(), b).length(), false, false},
                {QLineF(chain.first(), a).length(), false, true },
            };
            for (const auto &c : cases) {
                if (c.d < bestD) {
                    bestD = c.d;
                    best = i;
                    atTail = c.tail;
                    flip = c.rev;
                }
            }
        }

        QVector<QPointF> piece = parts.takeAt(best);
        if (flip)
            std::reverse(piece.begin(), piece.end());
        worstGap = std::max(worstGap, bestD);

        if (atTail) {
            // Стык совпал — второй раз ту же точку не пишем.
            if (QLineF(chain.last(), piece.first()).length() <= tol)
                piece.removeFirst();
            chain += piece;
        } else {
            if (!piece.isEmpty() && QLineF(piece.last(), chain.first()).length() <= tol)
                piece.removeLast();
            chain = piece + chain;
        }
    }

    if (chain.size() < 2)
        return fail(tpl::tr("после сшивания не осталось точек", "no points left after stitching"));

    // Концы сошлись — это замкнутый контур. Повтор первой точки убираем: замыкание
    // у контура подразумевается, а лишняя вершина уехала бы в задание как есть.
    const bool closed = chain.size() >= 4
                        && QLineF(chain.first(), chain.last()).length() <= tol;
    if (closed)
        chain.removeLast();

    // Свойства берём у ПЕРВОЙ части по порядку в задании: материал, eps, плазма.
    // Иначе объединение молча меняло бы среду, а спрашивать об этом здесь негде.
    const tpl::Shape first = m_project->shapes.at(order.first());

    beginEdit();
    tpl::Shape merged = first;
    merged.kind = closed ? ShapeKind::Polygon : ShapeKind::Polyline;
    merged.points = chain;
    merged.xMin = merged.xMax = merged.yMin = merged.yMax = 0.0;

    // Удаляем сшитые с конца, чтобы не сбить индексы, и ставим результат на место
    // первой из них — порядок фигур в списке для человека значим.
    QVector<int> del = order;
    std::sort(del.begin(), del.end(), std::greater<int>());
    QVector<int> goneIds;
    for (int i : del) {
        if (m_project->shapes.at(i).id != merged.id)
            goneIds << m_project->shapes.at(i).id;
        m_project->shapes.remove(i);
    }
    m_project->shapes.insert(order.first(), merged);

    // Варианты геометрии выключают фигуры по номеру. Съеденные номера надо вычистить,
    // иначе валидатор честно скажет «вариант выключает несуществующую фигуру» — и
    // будет прав: задание после объединения стало бы незаписываемым.
    for (tpl::Variant &v : m_project->variants) {
        const bool hid = std::any_of(goneIds.begin(), goneIds.end(),
                                     [&v](int id) { return v.disabledShapes.contains(id); });
        // ⚠️ removeAll, а не remove: disabledShapes — это QVector, и remove(int)
        // удалил бы элемент по ИНДЕКСУ, то есть выключил бы случайную фигуру.
        for (int id : goneIds)
            v.disabledShapes.removeAll(id);
        // Была выключена хоть одна часть — выключаем и результат: показать половину
        // сшитой фигуры всё равно нечем, а молча включить её в вариант значит
        // изменить расчёт за спиной у человека.
        if (hid && !v.disabledShapes.contains(merged.id))
            v.disabledShapes.append(merged.id);
    }

    m_sel = {Selection::ShapeBody, merged.id, -1};
    m_epsCache.clear();
    finishEdit();
    emit selectionChanged();

    if (why) {
        *why = worstGap > tol
                   ? tpl::tr("Соединено в одну фигуру: вершин %1. ⚠ Самый большой "
                             "разрыв между кусками — %2 мм, он замкнут прямой.",
                             "Joined into one shape: %1 vertices. ⚠ The largest gap between the pieces is "
                             "%2 mm, it is closed by a straight line.")
                         .arg(chain.size()).arg(worstGap, 0, 'g', 3)
                   : tpl::tr("Соединено в одну фигуру: вершин %1%2.", "Joined into one shape: %1 vertices%2.")
                         .arg(chain.size())
                         .arg(closed ? tpl::tr(", контур замкнулся", ", the polygon closed") : QString());
    }
    return true;
}

void GeometryView::cancelDrawing()
{
    m_chain.clear();
    m_creating = false;
    m_arcSweep = false;
    m_handle = -1;
    m_dragging = false;
    m_excludeId = -1;
    m_note.clear();
    updateHint();
    viewport()->update();
}

void GeometryView::deleteSelected()
{
    if (!m_project || m_sel.isEmpty())
        return;

    // Выделено несколько — удаляем ВСЕ. Раньше уходила одна, и остальные приходилось
    // добивать по штуке, гадая, какая именно исчезла.
    if (m_multi.size() > 1) {
        const QVector<int> gone = m_multi;
        beginEdit();
        for (int i = m_project->shapes.size() - 1; i >= 0; --i) {
            if (gone.contains(m_project->shapes.at(i).id))
                m_project->shapes.removeAt(i);
        }
        // Варианты выключают фигуры по номеру — исчезнувшие номера надо вычистить,
        // иначе задание перестанет проходить проверку.
        for (tpl::Variant &v : m_project->variants) {
            for (int id : gone)
                v.disabledShapes.removeAll(id);
        }
        m_multi.clear();
        m_sel = {};
        m_epsCache.clear();
        finishEdit();
        emit selectionChanged();
        return;
    }

    if (m_sel.kind == Selection::ShapeVertex) {
        // Удаление вершины, а не всей ломаной: иначе точную кривую не поправить.
        tpl::Shape *s = shapeById(m_sel.id);
        const int minPts = (s && s->kind == ShapeKind::Polygon) ? 3 : 2;
        if (s && s->points.size() > minPts && m_sel.index >= 0
            && m_sel.index < s->points.size()) {
            beginEdit();
            s->points.remove(m_sel.index);
            m_sel = {Selection::ShapeBody, s->id, -1};
            finishEdit();
            emit selectionChanged();
            return;
        }
    }
    if (m_sel.kind == Selection::ShapeEdge) {
        // Снять одну сторону прямоугольника — законная операция: так делают
        // открытый торец волновода. Эмиттер разложит фигуру на отдельные блоки.
        tpl::Shape *s = shapeById(m_sel.id);
        if (s && s->kind == ShapeKind::Rectangle && s->isContour() && m_sel.index >= 0) {
            beginEdit();
            s->edges[size_t(m_sel.index)].present = false;
            finishEdit();
            return;
        }
    }

    beginEdit();
    for (int i = 0; i < m_project->shapes.size(); ++i) {
        if (m_project->shapes.at(i).id == m_sel.id) {
            m_project->shapes.removeAt(i);
            m_sel = {};
            finishEdit();
            emit selectionChanged();
            return;
        }
    }
    for (int i = 0; i < m_project->ports.size(); ++i) {
        if (m_project->ports.at(i).id == m_sel.id) {
            m_project->ports.removeAt(i);
            m_sel = {};
            finishEdit();
            emit selectionChanged();
            return;
        }
    }
    finishEdit();
}

void GeometryView::duplicateSelected()
{
    if (!m_project || m_sel.isEmpty())
        return;
    const double d = gridStep() * 4.0;

    // Несколько фигур копируются ОДНОЙ правкой и одним сдвигом: копии сохраняют
    // взаимное положение, иначе собранная из кусков геометрия рассыпается.
    if (m_multi.size() > 1) {
        beginEdit();
        QVector<int> copies;
        int free = nextId();
        const QVector<int> src = m_multi;
        for (int id : src) {
            const tpl::Shape *from = shapeById(id);
            if (!from)
                continue;
            tpl::Shape s = *from;
            s.id = free++;
            s.name = tpl::tr("%1 (копия)", "%1 (copy)").arg(from->name);
            s.xMin += d; s.xMax += d; s.yMin += d; s.yMax += d;
            s.center += QPointF(d, d);
            for (QPointF &pt : s.points)
                pt += QPointF(d, d);
            m_project->shapes << s;
            copies << s.id;
        }
        m_multi = copies;
        if (!copies.isEmpty())
            m_sel = {Selection::ShapeBody, copies.last(), -1};
        finishEdit();
        emit selectionChanged();
        return;
    }

    if (const tpl::Shape *src = shapeById(m_sel.id)) {
        tpl::Shape s = *src;
        s.id = nextId();
        s.name = tpl::tr("%1 (копия)", "%1 (copy)").arg(src->name);
        s.xMin += d; s.xMax += d; s.yMin += d; s.yMax += d;
        s.center += QPointF(d, d);
        for (QPointF &pt : s.points)
            pt += QPointF(d, d);
        commitNewShape(s);
        return;
    }
    if (const Port *src = portById(m_sel.id)) {
        Port p = *src;
        p.id = nextId();
        p.name = tpl::tr("%1 (копия)", "%1 (copy)").arg(src->name);
        p.active = false; // копия активного порта сделала бы .s не матрицей
        p.from += d;
        p.to += d;
        beginEdit();
        m_project->ports << p;
        m_sel = {Selection::PortBody, p.id, -1};
        finishEdit();
        emit selectionChanged();
    }
}

void GeometryView::filletSelected(double radius)
{
    tpl::Shape *s = shapeById(m_sel.id);
    if (!s || radius <= 0.0)
        return;

    if (s->kind == ShapeKind::Rectangle) {
        const double r = std::min({radius, (s->xMax - s->xMin) / 2, (s->yMax - s->yMin) / 2});
        if (r <= 0.0)
            return;
        beginEdit();
        // Каждая дуга начинается там, где кончается прямая сторона, и кончается там,
        // где начинается следующая: отдельные точки на прямых участках не нужны,
        // их даёт замыкающее звено контура.
        QVector<QPointF> pts;
        pts += arcPoints({s->xMax - r, s->yMin + r}, r, -M_PI / 2, 0.0);
        pts += arcPoints({s->xMax - r, s->yMax - r}, r, 0.0, M_PI / 2);
        pts += arcPoints({s->xMin + r, s->yMax - r}, r, M_PI / 2, M_PI);
        pts += arcPoints({s->xMin + r, s->yMin + r}, r, M_PI, 3 * M_PI / 2);

        s->kind = ShapeKind::Polygon;
        s->points = pts;
        for (EdgeSpec &e : s->edges)
            e = EdgeSpec{}; // у контура сторон больше нет — материал общий
        finishEdit();
        return;
    }

    if ((s->kind == ShapeKind::Polyline || s->kind == ShapeKind::Polygon)
        && m_sel.kind == Selection::ShapeVertex) {
        const int i = m_sel.index;
        const int n = s->points.size();
        if (i <= 0 && s->kind == ShapeKind::Polyline)
            return;
        if (i < 0 || i >= n)
            return;
        // Соседи вершины: у контура индексы замыкаются по кругу.
        const QPointF prev = s->points.at((i - 1 + n) % n);
        const QPointF cur = s->points.at(i);
        const QPointF next = s->points.at((i + 1) % n);

        // Радиус ограничен половиной короче звена: касание не должно выйти за звено.
        QLineF a(cur, prev), b(cur, next);
        const double maxR = std::min(a.length(), b.length()) / 2.0;
        const double r = std::min(radius, maxR);
        if (r <= 0.0)
            return;

        // Угол между звеньями; почти 0 или почти 180 градусов — скруглять нечего.
        const double ang = std::acos(std::clamp(
            (a.dx() * b.dx() + a.dy() * b.dy()) / (a.length() * b.length()), -1.0, 1.0));
        if (ang < 1e-6 || ang > M_PI - 1e-6)
            return;
        // Расстояние от вершины до точек касания: r / tg(ang/2).
        const double tanLen = r / std::tan(ang / 2.0);
        if (tanLen > maxR)
            return;

        a.setLength(tanLen);
        b.setLength(tanLen);
        const QPointF p1 = a.p2();
        const QPointF p2 = b.p2();
        // Центр скругления — на биссектрисе, на расстоянии r/sin(ang/2).
        QLineF bis(cur, cur + (a.p2() - cur) + (b.p2() - cur));
        bis.setLength(r / std::sin(ang / 2.0));
        const QPointF c = bis.p2();

        // Углы точек касания от центра; разность приводится к (-pi; pi] — короткая дуга.
        double a0 = std::atan2(p1.y() - c.y(), p1.x() - c.x());
        double a1 = std::atan2(p2.y() - c.y(), p2.x() - c.x());
        while (a1 - a0 > M_PI) a1 -= 2 * M_PI;
        while (a0 - a1 > M_PI) a1 += 2 * M_PI;

        // Вершина заменяется точками дуги, остальные вершины сохраняются по порядку.
        beginEdit();
        QVector<QPointF> pts;
        for (int k = 0; k < i; ++k)
            pts << s->points.at(k);
        pts += arcPoints(c, r, a0, a1);
        for (int k = i + 1; k < n; ++k)
            pts << s->points.at(k);
        s->points = pts;
        m_sel = {Selection::ShapeBody, s->id, -1};
        finishEdit();
        emit selectionChanged();
    }
}

void GeometryView::updateHint()
{
    QString h;
    switch (m_tool) {
    case Tool::Select:
        h = m_multi.size() > 1
                ? tpl::tr("Выделено фигур: %1. Тянуть — двигать все сразу, Del — "
                          "удалить все. Правая кнопка — соединить в одну фигуру. "
                          "Ctrl+клик снимает лишнюю.",
                          "Shapes selected: %1. Drag to move them all, Del to delete them all. Right "
                          "button: join into one shape. Ctrl+click deselects one.").arg(m_multi.size())
                : tpl::tr("Клик — выделить, Ctrl+клик — набрать несколько. "
                          "Тянуть — двигать, квадратики — размер, стрелка — "
                          "поворот. Del — удалить.",
                          "Click to select, Ctrl+click to pick several. Drag to move, squares to resize, "
                          "arrow to rotate. Del to delete.");
        break;
    case Tool::Rect:
        h = tpl::tr("Нажмите и протяните прямоугольник.", "Press and drag a rectangle.");
        break;
    case Tool::Line:
        h = m_chain.isEmpty()
                ? tpl::tr("Клик — первая точка линии.", "Click: first point of the line.")
                : tpl::tr("Клик — следующая точка. Двойной клик, Enter или правая "
                          "кнопка — закончить.",
                          "Click: next point. Double-click, Enter or right button to finish.");
        break;
    case Tool::Contour:
        h = m_chain.isEmpty()
                ? tpl::tr("Клик — первая вершина замкнутого контура.", "Click: first vertex of the closed polygon.")
                : tpl::tr("Клик — следующая вершина. Двойной клик или Enter замыкает.", "Click: next vertex. Double-click or Enter closes it.");
        break;
    case Tool::Circle:
        h = tpl::tr("Нажмите в центре и протяните радиус.", "Press at the center and drag the radius.");
        break;
    case Tool::Arc:
        h = m_arcSweep
                ? tpl::tr("Ведите мышь — набирается раскрыв сектора. Клик закрывает, "
                          "Esc — бросить.",
                          "Move the mouse to set the sector span. Click closes it, Esc discards.")
                : tpl::tr("Нажмите в центре и протяните радиус: это будет начало дуги.", "Press at the center and drag the radius: this will be the start of the arc.");
        break;
    case Tool::ArcLine:
        if (m_chain.isEmpty())
            h = tpl::tr("Клик — начало дуги.", "Click: start of the arc.");
        else if (m_chain.size() == 1)
            h = tpl::tr("Клик — конец дуги.", "Click: end of the arc.");
        else
            h = tpl::tr("Ведите мышь — дуга выгибается за ней. Клик или Enter "
                        "закрепляет, Esc — бросить.",
                        "Move the mouse: the arc bends after it. Click or Enter fixes it, Esc discards.");
        break;
    case Tool::Port:
        h = tpl::tr("Протяните апертуру — где угодно внутри области, не только "
                    "по краю. Направление волны задаёт половина области.",
                    "Drag the aperture anywhere inside the domain, not only along the edge. The "
                    "half of the domain sets the wave direction.");
        break;
    }
    emit hintChanged(h);
}
