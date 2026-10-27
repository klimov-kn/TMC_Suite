/// @file meshview.cpp
/// @brief Реализация 3D-вида модели `MeshView`: шейдеры, буфер вершин, камера, слой подписей.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-mesh
#include "meshview.h"

#include "dtheme.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include "lang.h"

using namespace tpl;

namespace {

/// Вершинный шейдер (GLSL 1.10): освещение фонарём у камеры, свет с обеих сторон грани.
const char *kVertex =
    "attribute vec3 pos;\n"
    "attribute vec3 nrm;\n"
    "attribute vec4 col;\n"
    "uniform mat4 mvp;\n"
    "uniform mat3 nmat;\n"
    "varying vec4 vcol;\n"
    "void main() {\n"
    "    vec3 n = normalize(nmat * nrm);\n"
    // Фонарь у камеры, свет с обеих сторон грани: у принесённой модели нормали бывают
    // развёрнуты как попало, и «чёрная изнанка» только путала бы.
    "    float k = 0.30 + 0.70 * abs(n.z);\n"
    "    vcol = vec4(col.rgb * k, col.a);\n"
    "    gl_Position = mvp * vec4(pos, 1.0);\n"
    "}\n";

/// Фрагментный шейдер: цвет вершины как есть.
const char *kFragment =
    "varying vec4 vcol;\n"
    "void main() { gl_FragColor = vcol; }\n";

/// @brief Вершина буфера OpenGL: положение, нормаль, цвет.
struct Vtx {
    /// Координаты, мм.
    float p[3];
    /// Нормаль грани.
    float n[3];
    /// Цвет RGBA, 0...1.
    float c[4];
};

/// @brief Переводит точку ядра генератора в `QVector3D`.
/// @param[in] v  точка, мм.
/// @return Та же точка в `float`.
QVector3D toQ(const Vec3 &v) { return QVector3D(float(v.x), float(v.y), float(v.z)); }

/// @brief Собирает точку модели из координат в осях плоскости среза.
/// @param[in] ax  оси плоскости (`u`, `v`, нормаль).
/// @param[in] u   координата по первой оси плоскости, мм.
/// @param[in] v   координата по второй оси плоскости, мм.
/// @param[in] w   координата по нормали, мм.
/// @return Точка в координатах модели.
QVector3D axisPoint(const PlaneAxes &ax, double u, double v, double w)
{
    double c[3];
    c[ax.u] = u;
    c[ax.v] = v;
    c[ax.normal] = w;
    return QVector3D(float(c[0]), float(c[1]), float(c[2]));
}

} // namespace

namespace {

/// Прозрачный слой поверх 3D: мышь проходит сквозь него к самому виду.
/// @brief Дочерний виджет `MeshView`, рисующий подписи и линию среза через `MeshView::paintOverlay`.
class Overlay : public QWidget
{
public:
    /// @brief Создаёт слой, прозрачный для мыши и без фона.
    /// @param[in] view  вид-владелец (родитель слоя).
    explicit Overlay(MeshView *view) : QWidget(view), m_view(view)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
    }

protected:
    /// @brief Передаёт рисование виду (`MeshView::paintOverlay`).
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        m_view->paintOverlay(p);
    }

private:
    /// Вид-владелец.
    MeshView *m_view;
};

} // namespace

MeshView::MeshView(QWidget *parent) : QOpenGLWidget(parent), m_overlay(new Overlay(this))
{
    setObjectName(QStringLiteral("meshView"));
    setMinimumSize(360, 300);
    setFocusPolicy(Qt::StrongFocus);
    // Сглаживание краёв: модель с тонкими стенками без него «рвётся» лесенкой.
    QSurfaceFormat f = format();
    f.setSamples(4);
    f.setDepthBufferSize(24);
    // Буфер трафарета нужен QPainter: многоугольники поверх модели он заливает через него.
    f.setStencilBufferSize(8);
    setFormat(f);
}

MeshView::~MeshView()
{
    makeCurrent();
    m_vbo.destroy();
    doneCurrent();
}

QColor MeshView::planeColor(SlicePlane p)
{
    switch (p) {
    case SlicePlane::XY: return QColor(0x5f, 0xb4, 0xff); // голубой
    case SlicePlane::YZ: return QColor(0xff, 0x6b, 0x6b); // красный
    case SlicePlane::XZ: return QColor(0x5c, 0xe0, 0x8a); // зелёный
    }
    return Qt::white;
}

void MeshView::setMesh(const Mesh *mesh, const QVector<QColor> &bodyColors)
{
    m_mesh = mesh;
    m_colors = bodyColors;
    m_bounds = mesh ? mesh->bounds() : Bounds3();
    m_planeChosen = false;
    m_section = SliceResult();
    m_dirty = true;
    resetCamera();
}

void MeshView::setBodyColors(const QVector<QColor> &bodyColors)
{
    m_colors = bodyColors;
    m_dirty = true;
    refresh();
}

void MeshView::setPlane(SlicePlane p, double offset)
{
    m_plane = p;
    m_offset = offset;
    m_planeChosen = true;
    refresh();
}

void MeshView::setSection(const SliceResult &r)
{
    m_section = r;
    refresh();
}

void MeshView::resetCamera()
{
    if (m_bounds.valid) {
        m_target = toQ(m_bounds.center());
        m_dist = std::max(1e-3, m_bounds.size()) * 1.9;
    }
    m_yaw = -35.0;
    m_pitch = 25.0;
    refresh();
}

void MeshView::initializeGL()
{
    initializeOpenGLFunctions();
    m_prog.addShaderFromSourceCode(QOpenGLShader::Vertex, kVertex);
    m_prog.addShaderFromSourceCode(QOpenGLShader::Fragment, kFragment);
    m_prog.bindAttributeLocation("pos", 0);
    m_prog.bindAttributeLocation("nrm", 1);
    m_prog.bindAttributeLocation("col", 2);
    m_prog.link();
    m_vbo.create();
    m_glReady = true;
    m_dirty = true;
}

void MeshView::uploadMesh()
{
    m_dirty = false;
    m_vertexCount = m_opaqueCount = 0;
    if (!m_mesh || m_mesh->isEmpty())
        return;
    QVector<Vtx> opaque, faded;
    opaque.reserve(m_mesh->tris.size() * 3);
    for (int b = 0; b < m_mesh->bodies.size(); ++b) {
        const MeshBody &body = m_mesh->bodies.at(b);
        const QColor c = b < m_colors.size() ? m_colors.at(b) : QColor(0xa0, 0xa8, 0xb8);
        QVector<Vtx> &dst = c.alpha() < 255 ? faded : opaque;
        for (int t = body.firstTri; t < body.firstTri + body.triCount; ++t) {
            const auto &tri = m_mesh->tris.at(t);
            const QVector3D a = toQ(m_mesh->vertices.at(tri[0]));
            const QVector3D bb = toQ(m_mesh->vertices.at(tri[1]));
            const QVector3D cc = toQ(m_mesh->vertices.at(tri[2]));
            const QVector3D n = QVector3D::normal(a, bb, cc);
            for (const QVector3D &p : {a, bb, cc})
                dst << Vtx{{p.x(), p.y(), p.z()},
                           {n.x(), n.y(), n.z()},
                           {float(c.redF()), float(c.greenF()), float(c.blueF()), float(c.alphaF())}};
        }
    }
    m_opaqueCount = int(opaque.size());
    opaque += faded;
    m_vertexCount = int(opaque.size());
    m_vbo.bind();
    m_vbo.allocate(opaque.constData(), int(opaque.size() * sizeof(Vtx)));
    m_vbo.release();
}

void MeshView::resizeGL(int, int)
{
    m_overlay->setGeometry(rect());
}

void MeshView::refresh()
{
    update();
    m_overlay->update();
}

QMatrix4x4 MeshView::viewMatrix() const
{
    QMatrix4x4 v;
    v.translate(0, 0, -float(m_dist));
    v.rotate(float(m_pitch), 1, 0, 0);
    // Z модели — вверх: CAD рисует так почти всегда, и «стоящая» деталь понятнее.
    // Поворот на -90 вокруг X кладёт Z модели на вертикаль экрана; рыскание — вокруг
    // этой вертикали.
    v.rotate(-90.0f, 1, 0, 0);
    v.rotate(float(m_yaw), 0, 0, 1);
    v.translate(-m_target);
    return v;
}

QMatrix4x4 MeshView::projMatrix() const
{
    QMatrix4x4 p;
    const float aspect = float(std::max(1, width())) / float(std::max(1, height()));
    const float diag = float(std::max(1e-3, m_bounds.size()));
    // Модель лежит в шаре радиусом в половину диагонали вокруг точки обзора (плюс
    // плоскости с запасом): ближняя и дальняя грани — по нему. Ближнюю нельзя ставить к
    // нулю — пропадёт точность глубины, и грани начнут просвечивать друг сквозь друга.
    const float d = float(m_dist);
    p.perspective(35.0f, aspect, std::max(d - diag, d * 0.02f), d + diag);
    return p;
}

double MeshView::planeOffsetShown(SlicePlane p) const
{
    if (m_planeChosen && p == m_plane)
        return m_offset;
    return axisValue(m_bounds.center(), planeAxes(p).normal);
}

std::array<QVector3D, 4> MeshView::planeQuad(SlicePlane p, double offset) const
{
    const PlaneAxes ax = planeAxes(p);
    const double pad = 0.08 * m_bounds.size();
    const double u0 = axisValue(m_bounds.lo, ax.u) - pad, u1 = axisValue(m_bounds.hi, ax.u) + pad;
    const double v0 = axisValue(m_bounds.lo, ax.v) - pad, v1 = axisValue(m_bounds.hi, ax.v) + pad;
    return {axisPoint(ax, u0, v0, offset), axisPoint(ax, u1, v0, offset),
            axisPoint(ax, u1, v1, offset), axisPoint(ax, u0, v1, offset)};
}

QPointF MeshView::project(const QVector3D &v) const
{
    const QVector4D c = projMatrix() * viewMatrix() * QVector4D(v, 1.0f);
    if (std::fabs(c.w()) < 1e-12f)
        return QPointF(-1e6, -1e6);
    return QPointF((c.x() / c.w() * 0.5 + 0.5) * width(), (0.5 - c.y() / c.w() * 0.5) * height());
}

void MeshView::paintGL()
{
    const QColor bg = dtheme::bg();
    glClearColor(float(bg.redF()), float(bg.greenF()), float(bg.blueF()), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (!m_bounds.valid)
        return;
    if (m_dirty)
        uploadMesh();

    const QMatrix4x4 view = viewMatrix();
    const QMatrix4x4 mvp = projMatrix() * view;
    m_prog.bind();
    m_prog.setUniformValue("mvp", mvp);
    m_prog.setUniformValue("nmat", view.normalMatrix());

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    const auto bindAttribs = [&] {
        m_prog.enableAttributeArray(0);
        m_prog.enableAttributeArray(1);
        m_prog.enableAttributeArray(2);
        m_prog.setAttributeBuffer(0, GL_FLOAT, offsetof(Vtx, p), 3, sizeof(Vtx));
        m_prog.setAttributeBuffer(1, GL_FLOAT, offsetof(Vtx, n), 3, sizeof(Vtx));
        m_prog.setAttributeBuffer(2, GL_FLOAT, offsetof(Vtx, c), 4, sizeof(Vtx));
    };
    if (m_vertexCount > 0) {
        m_vbo.bind();
        bindAttribs();
        glDrawArrays(GL_TRIANGLES, 0, m_opaqueCount);
        if (m_vertexCount > m_opaqueCount) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            glDrawArrays(GL_TRIANGLES, m_opaqueCount, m_vertexCount - m_opaqueCount);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }
        m_vbo.release();
    }

    // Плоскости — полупрозрачные прямоугольники. До выбора видны все три через центр
    // детали; после — выбранная ярко на своём месте, остальные бледно.
    QVector<Vtx> quads;
    for (SlicePlane p : {SlicePlane::XY, SlicePlane::YZ, SlicePlane::XZ}) {
        const QColor c = planeColor(p);
        const float alpha = !m_planeChosen ? 0.28f : (p == m_plane ? 0.38f : 0.07f);
        const auto q = planeQuad(p, planeOffsetShown(p));
        const QVector3D n = QVector3D::normal(q[0], q[1], q[2]);
        for (int idx : {0, 1, 2, 0, 2, 3})
            quads << Vtx{{q[idx].x(), q[idx].y(), q[idx].z()},
                         {n.x(), n.y(), n.z()},
                         {float(c.redF()), float(c.greenF()), float(c.blueF()), alpha}};
    }
    QOpenGLBuffer qb(QOpenGLBuffer::VertexBuffer);
    qb.create();
    qb.bind();
    qb.allocate(quads.constData(), int(quads.size() * sizeof(Vtx)));
    bindAttribs();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDrawArrays(GL_TRIANGLES, 0, int(quads.size()));
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    qb.release();
    qb.destroy();
    // ⚠️ Вершинные массивы ОБЯЗАТЕЛЬНО выключить: QPainter поверх рисует теми же
    // атрибутами, и оставленный включённым массив со старым буфером молча гасит всё,
    // что он рисует, — линии среза и подписи плоскостей просто не появлялись.
    m_prog.disableAttributeArray(0);
    m_prog.disableAttributeArray(1);
    m_prog.disableAttributeArray(2);
    m_prog.release();
    glDisable(GL_DEPTH_TEST);
}

/// Поверх модели — рамки и подписи плоскостей и линия среза.
///
/// Рисуется НЕ в paintGL, а отдельным прозрачным виджетом поверх: QPainter внутри
/// paintGL на части систем не попадает ни на экран, ни в снимок (проверено: подписи и
/// линия среза пропадали целиком), а дочерний виджет Qt накладывает на OpenGL-картинку
/// сам и одинаково везде. Толстая линия в самом OpenGL тоже не годится — glLineWidth > 1
/// в части драйверов не работает.
void MeshView::paintOverlay(QPainter &painter)
{
    if (!m_bounds.valid)
        return;
    painter.setRenderHint(QPainter::Antialiasing);
    for (SlicePlane p : {SlicePlane::XY, SlicePlane::YZ, SlicePlane::XZ}) {
        if (m_planeChosen && p != m_plane)
            continue;
        const auto q = planeQuad(p, planeOffsetShown(p));
        QPolygonF poly;
        for (const QVector3D &v : q)
            poly << project(v);
        QColor c = planeColor(p);
        painter.setPen(QPen(c, 1.4));
        painter.setBrush(Qt::NoBrush);
        painter.drawPolygon(poly);
        QFont f = font();
        f.setBold(true);
        f.setPointSizeF(font().pointSizeF() * 1.2);
        painter.setFont(f);
        // Подпись — у угла, который выше всех на экране: там её не закроет деталь.
        QPointF at = poly.at(0);
        for (const QPointF &pt : poly)
            if (pt.y() < at.y())
                at = pt;
        const QString label = slicePlaneName(p);
        const QRectF box(at.x() - 4, at.y() - 26, painter.fontMetrics().horizontalAdvance(label) + 12, 22);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(c.red(), c.green(), c.blue(), 210));
        painter.drawRoundedRect(box, 4, 4);
        painter.setPen(dtheme::bg());
        painter.drawText(box, Qt::AlignCenter, label);
    }
    if (m_planeChosen) {
        const PlaneAxes ax = planeAxes(m_plane);
        painter.setPen(QPen(QColor(0xff, 0xe0, 0x66), 2.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);
        for (const SliceBody &b : m_section.bodies)
            for (const SliceLoop &l : b.loops) {
                QPolygonF poly;
                for (const QPointF &uv : l.pts)
                    poly << project(axisPoint(ax, uv.x(), uv.y(), m_offset));
                if (l.closed)
                    painter.drawPolygon(poly);
                else
                    painter.drawPolyline(poly);
            }
    }
    // Подсказка про мышь — мелко в углу.
    painter.setFont(font());
    painter.setPen(dtheme::textMute());
    painter.drawText(rect().adjusted(10, 0, -10, -8),
                     Qt::AlignBottom | Qt::AlignLeft | Qt::TextWordWrap,
                     tpl::tr("ЛКМ — повернуть · колесо — масштаб · средняя или Alt+ЛКМ — "
                             "сдвиг · щелчок по плоскости — выбрать",
                             "LMB: rotate · wheel: zoom · middle or Alt+LMB: pan · click a plane: select"));
}

void MeshView::mousePressEvent(QMouseEvent *e)
{
    m_press = m_last = e->pos();
    m_dragged = false;
    m_panning = e->button() == Qt::MiddleButton
                || (e->button() == Qt::LeftButton && (e->modifiers() & Qt::AltModifier));
}

void MeshView::mouseMoveEvent(QMouseEvent *e)
{
    if (!(e->buttons() & (Qt::LeftButton | Qt::MiddleButton)))
        return;
    const QPoint d = e->pos() - m_last;
    m_last = e->pos();
    if ((e->pos() - m_press).manhattanLength() > 4)
        m_dragged = true;
    if (m_panning) {
        // Сдвиг в плоскости экрана: столько миллиметров, сколько пикселей под курсором.
        const QMatrix4x4 inv = viewMatrix().inverted();
        const QVector3D right = (inv.mapVector(QVector3D(1, 0, 0))).normalized();
        const QVector3D up = (inv.mapVector(QVector3D(0, 1, 0))).normalized();
        const float k = float(m_dist * 2.0 * std::tan(35.0 * M_PI / 360.0) / std::max(1, height()));
        m_target -= right * float(d.x()) * k;
        m_target += up * float(d.y()) * k;
    } else {
        m_yaw += d.x() * 0.4;
        m_pitch = std::clamp(m_pitch + d.y() * 0.4, -89.0, 89.0);
    }
    refresh();
}

void MeshView::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_dragged || e->button() != Qt::LeftButton || m_panning || !m_bounds.valid)
        return;
    // Щелчок без протяжки — какая плоскость под курсором ближе всех к глазу.
    const QMatrix4x4 inv = (projMatrix() * viewMatrix()).inverted();
    const float nx = float(2.0 * e->pos().x() / std::max(1, width()) - 1.0);
    const float ny = float(1.0 - 2.0 * e->pos().y() / std::max(1, height()));
    const QVector4D a4 = inv * QVector4D(nx, ny, -1, 1), b4 = inv * QVector4D(nx, ny, 1, 1);
    const QVector3D a = a4.toVector3DAffine(), b = b4.toVector3DAffine();
    const QVector3D dir = (b - a).normalized();
    // Луч из глаза через курсор пересекается с каждой из трёх плоскостей; берётся
    // ближайшее пересечение, лежащее внутри прямоугольника плоскости.
    double best = 1e300;
    int hit = -1;
    for (int i = 0; i < 3; ++i) {
        const SlicePlane p = SlicePlane(i);
        const PlaneAxes ax = planeAxes(p);
        const double w = planeOffsetShown(p);
        const double da = (ax.normal == 0 ? a.x() : ax.normal == 1 ? a.y() : a.z());
        const double dd = (ax.normal == 0 ? dir.x() : ax.normal == 1 ? dir.y() : dir.z());
        if (std::fabs(dd) < 1e-12)
            continue;
        const double t = (w - da) / dd;
        if (t <= 0 || t >= best)
            continue;
        const QVector3D at = a + dir * float(t);
        const auto q = planeQuad(p, w);
        const auto in = [&](int axis, const QVector3D &lo, const QVector3D &hi) {
            const float c = axis == 0 ? at.x() : axis == 1 ? at.y() : at.z();
            const float l = axis == 0 ? lo.x() : axis == 1 ? lo.y() : lo.z();
            const float h = axis == 0 ? hi.x() : axis == 1 ? hi.y() : hi.z();
            return c >= std::min(l, h) && c <= std::max(l, h);
        };
        if (in(ax.u, q[0], q[2]) && in(ax.v, q[0], q[2])) {
            best = t;
            hit = i;
        }
    }
    if (hit >= 0)
        emit planeClicked(SlicePlane(hit));
}

void MeshView::wheelEvent(QWheelEvent *e)
{
    const double steps = e->angleDelta().y() / 120.0;
    m_dist *= std::pow(0.87, steps);
    m_dist = std::clamp(m_dist, std::max(1e-3, m_bounds.size()) * 0.05, std::max(1e-3, m_bounds.size()) * 20);
    refresh();
}
