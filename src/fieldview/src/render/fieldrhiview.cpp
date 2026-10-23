// FieldRhiView — описание в fieldrhiview.h.
/// @file fieldrhiview.cpp
/// @brief Вывод сцены поля через Qt RHI: ресурсы видеокарты, конвейеры, матрица камеры, кадр, мышь.
///
/// Часть модуля FieldView (слой `render`). Описание класса — в `fieldrhiview.h`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#rhi
#include "fieldrhiview.h"
#include "theme.h"

#include <QDateTime>
#include <QFile>
#include <QMouseEvent>
#include <QWheelEvent>
#include <rhi/qrhi.h>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace fieldview {

namespace {

/// Угол обзора камеры по вертикали, градусы.
constexpr double kFovY = 30.0;   // gluPerspective(30.0, …)
/// Число пи.
constexpr double kPi = 3.14159265358979323846;
/// @name Углы вида по умолчанию для подбора расстояния камеры, градусы
///@{
/// Поворот вокруг Z и наклон вокруг X, при которых сцена должна целиком помещаться в окне.
constexpr double kFitAzimuth = -20.0, kFitTilt = -55.0;
///@}

/// @brief Загружает собранный шейдер из ресурсов `:/fieldview/shaders/<имя>.qsb`.
/// @param[in] name  имя исходника шейдера («grid.vert» …)
/// @return Шейдер; пустой, если ресурса нет (выводится предупреждение).
QShader loadShader(const char *name)
{
    QFile f(QStringLiteral(":/fieldview/shaders/") + QLatin1String(name) + QStringLiteral(".qsb"));
    if (f.open(QIODevice::ReadOnly))
        return QShader::fromSerialized(f.readAll());
    qWarning("FieldView: нет шейдера %s", name);
    return QShader();
}

/// @brief Буфер констант прохода сетки — раскладка std140 блока `buf` шейдеров `grid.vert` / `grid.frag`.
///
/// Uniform-блок слоя сетки (std140, совпадает с grid.vert / grid.frag).
struct GridUniforms
{
    /// Матрица «сцена -> отсечение» с поправкой интерфейса, по столбцам.
    float mvp[16];
    /// Размещение сетки: x0, y0, sx, sy (`SceneData`).
    float rect[4];
    /// k, off (h = v * k + off); режим цвета (0 — (h + 1) / 2, 1 — |h|); число уровней (0 — плавно).
    float map[4];
    /// Множитель высоты (0 — плоско); подъём над z = 0; непрозрачность; 1 — цвет по шкале, 0 — цвет `color`.
    float misc[4];
    /// Цвет каркаса RGBA (когда цвет не по шкале).
    float color[4];
    /// x — сила затенения склонов рельефа (0 — нет); остальное не используется.
    float extra[4];
};
static_assert(sizeof(GridUniforms) == 144, "std140");

/// @brief Возвращает смешивание для цвета с предумноженной альфой: src * 1 + dst * (1 - src.a).
/// @return Настройка смешивания цели вывода.
QRhiGraphicsPipeline::TargetBlend premulBlend()
{
    QRhiGraphicsPipeline::TargetBlend b;
    b.enable = true;
    b.srcColor = QRhiGraphicsPipeline::One;
    b.dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    b.srcAlpha = QRhiGraphicsPipeline::One;
    b.dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    return b;
}

} // namespace

/// @brief Ресурсы видеокарты одной картинки поверх сцены.
struct FieldRhiView::SpriteGpu
{
    /// Текстура картинки.
    std::unique_ptr<QRhiTexture> tex;
    /// Набор привязок (матрица и текстура картинки).
    std::unique_ptr<QRhiShaderResourceBindings> srb;
    /// Залитый номер изменения `Sprite::version`.
    quint64 version = ~0ull;
    /// Размер текстуры, точек.
    QSize size;
};

QRhiWidget::Api FieldRhiView::s_api =
#ifdef Q_OS_WIN
    QRhiWidget::Api::Direct3D11;
#else
    QRhiWidget::Api::OpenGL;
#endif

FieldRhiView::FieldRhiView(QWidget *parent) : QRhiWidget(parent)
{
    setApi(s_api);
    setSampleCount(4);   // сглаживание линий каркаса и краёв рельефа
    setMouseTracking(false);
    setFocusPolicy(Qt::StrongFocus);
    m_fpsStart = QDateTime::currentMSecsSinceEpoch();
}

FieldRhiView::~FieldRhiView() = default;

void FieldRhiView::releaseResources()
{
    releaseAll();
    m_rhi = nullptr;
}

void FieldRhiView::releaseAll()
{
    m_psSurf.reset();
    m_psWire.reset();
    m_psTri.reset();
    m_psLine.reset();
    m_psSprite.reset();
    m_spriteGpu.clear();
    m_srbFieldSurf.reset();
    m_srbFieldWire.reset();
    m_srbEpsSurf.reset();
    m_srbEpsWire.reset();
    m_srbColor.reset();
    m_srbSpriteLayout.reset();
    m_uvBuf.reset();
    m_fieldVal.reset();
    m_epsVal.reset();
    m_triIdx.reset();
    m_wireDec.reset();
    m_wireFull.reset();
    m_ubFieldSurf.reset();
    m_ubFieldWire.reset();
    m_ubEpsSurf.reset();
    m_ubEpsWire.reset();
    m_ubColor.reset();
    m_ubSprite.reset();
    m_triBuf.reset();
    m_lineBuf.reset();
    m_spriteVerts.reset();
    m_fieldTex.reset();
    m_epsTex.reset();
    m_sampler.reset();
    m_spriteSampler.reset();
    m_gridX = m_gridY = 0;
    m_fieldValVersion = m_epsValVersion = ~0ull;
    m_fieldPalVersion = m_epsPalVersion = m_geomVersion = ~0ull;
    m_triCap = m_lineCap = 0;
    m_rpDesc = nullptr;
}

void FieldRhiView::initialize(QRhiCommandBuffer *)
{
    if (m_rhi != rhi()) {
        releaseAll();
        m_rhi = rhi();
        m_backend = QString::fromLatin1(m_rhi->backendName());
        Q_EMIT backendChanged(m_backend);
    }
    QRhiRenderTarget *rt = renderTarget();
    if (!m_psSurf || m_sampleCount != rt->sampleCount() || !m_rpDesc
        || !m_rpDesc->isCompatible(rt->renderPassDescriptor())) {
        m_sampleCount = rt->sampleCount();
        m_rpDesc = rt->renderPassDescriptor();
        buildPipelines();
    }
}

void FieldRhiView::buildPipelines()
{
    if (!m_ubFieldSurf) {
        const auto ub = [this](quint32 size) {
            std::unique_ptr<QRhiBuffer> b(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, size));
            b->create();
            return b;
        };
        m_ubFieldSurf = ub(sizeof(GridUniforms));
        m_ubFieldWire = ub(sizeof(GridUniforms));
        m_ubEpsSurf = ub(sizeof(GridUniforms));
        m_ubEpsWire = ub(sizeof(GridUniforms));
        m_ubColor = ub(64);
        m_ubSprite = ub(64);
        m_fieldTex.reset(m_rhi->newTexture(QRhiTexture::RGBA8, QSize(256, 1)));
        m_fieldTex->create();
        m_epsTex.reset(m_rhi->newTexture(QRhiTexture::RGBA8, QSize(256, 1)));
        m_epsTex->create();
        m_sampler.reset(m_rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                          QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
        m_sampler->create();
        m_spriteSampler.reset(m_rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                                QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
        m_spriteSampler->create();
        m_spriteVerts.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, 8 * 6 * 4 * sizeof(float)));
        m_spriteVerts->create();
    }
    const auto stages = QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;
    const auto gridSrb = [&](QRhiBuffer *ub, QRhiTexture *tex) {
        std::unique_ptr<QRhiShaderResourceBindings> s(m_rhi->newShaderResourceBindings());
        s->setBindings({QRhiShaderResourceBinding::uniformBuffer(0, stages, ub),
                        QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage,
                                                                  tex, m_sampler.get())});
        s->create();
        return s;
    };
    m_srbFieldSurf = gridSrb(m_ubFieldSurf.get(), m_fieldTex.get());
    m_srbFieldWire = gridSrb(m_ubFieldWire.get(), m_fieldTex.get());
    m_srbEpsSurf = gridSrb(m_ubEpsSurf.get(), m_epsTex.get());
    m_srbEpsWire = gridSrb(m_ubEpsWire.get(), m_epsTex.get());
    m_srbColor.reset(m_rhi->newShaderResourceBindings());
    m_srbColor->setBindings({QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage,
                                                                      m_ubColor.get())});
    m_srbColor->create();
    // Образец раскладки для картинок (у каждой картинки свой набор с её текстурой).
    m_srbSpriteLayout.reset(m_rhi->newShaderResourceBindings());
    m_srbSpriteLayout->setBindings({QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, m_ubSprite.get()),
                                    QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage,
                                                                              m_fieldTex.get(), m_spriteSampler.get())});
    m_srbSpriteLayout->create();
    for (auto &s : m_spriteGpu)
        s->srb.reset();

    const QShader gv = loadShader("grid.vert"), gf = loadShader("grid.frag");
    const QShader cv = loadShader("color.vert"), cf = loadShader("color.frag");
    const QShader sv = loadShader("sprite.vert"), sf = loadShader("sprite.frag");

    QRhiVertexInputLayout gridLayout;
    gridLayout.setBindings({{2 * sizeof(float)}, {sizeof(float)}});
    gridLayout.setAttributes({{0, 0, QRhiVertexInputAttribute::Float2, 0},
                              {1, 1, QRhiVertexInputAttribute::Float, 0}});
    QRhiVertexInputLayout colorLayout;
    colorLayout.setBindings({{7 * sizeof(float)}});
    colorLayout.setAttributes({{0, 0, QRhiVertexInputAttribute::Float3, 0},
                               {0, 1, QRhiVertexInputAttribute::Float4, 3 * sizeof(float)}});
    QRhiVertexInputLayout spriteLayout;
    spriteLayout.setBindings({{4 * sizeof(float)}});
    spriteLayout.setAttributes({{0, 0, QRhiVertexInputAttribute::Float2, 0},
                                {0, 1, QRhiVertexInputAttribute::Float2, 2 * sizeof(float)}});

    const auto make = [&](const QShader &v, const QShader &f, const QRhiVertexInputLayout &layout,
                          QRhiShaderResourceBindings *srb, QRhiGraphicsPipeline::Topology topo,
                          bool depthTest, bool depthWrite) {
        std::unique_ptr<QRhiGraphicsPipeline> p(m_rhi->newGraphicsPipeline());
        p->setShaderStages({{QRhiShaderStage::Vertex, v}, {QRhiShaderStage::Fragment, f}});
        p->setVertexInputLayout(layout);
        p->setShaderResourceBindings(srb);
        p->setRenderPassDescriptor(m_rpDesc);
        p->setSampleCount(m_sampleCount);
        p->setTopology(topo);
        p->setDepthTest(depthTest);
        p->setDepthWrite(depthWrite);
        p->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
        p->setCullMode(QRhiGraphicsPipeline::None);
        p->setTargetBlends({premulBlend()});
        p->create();
        return p;
    };
    m_psSurf = make(gv, gf, gridLayout, m_srbFieldSurf.get(), QRhiGraphicsPipeline::Triangles, true, true);
    m_psWire = make(gv, gf, gridLayout, m_srbFieldWire.get(), QRhiGraphicsPipeline::Lines, true, false);
    m_psTri = make(cv, cf, colorLayout, m_srbColor.get(), QRhiGraphicsPipeline::Triangles, true, false);
    m_psLine = make(cv, cf, colorLayout, m_srbColor.get(), QRhiGraphicsPipeline::Lines, true, false);
    m_psSprite = make(sv, sf, spriteLayout, m_srbSpriteLayout.get(), QRhiGraphicsPipeline::Triangles, false, false);
}

void FieldRhiView::rebuildGrid(QRhiResourceUpdateBatch *u)
{
    const int nX = m_scene->nX, nY = m_scene->nY;
    m_gridX = nX;
    m_gridY = nY;
    const size_t nodes = size_t(nX) * size_t(nY);
    std::vector<float> uv(nodes * 2);
    for (int i = 0; i < nY; ++i) {
        for (int j = 0; j < nX; ++j) {
            const size_t k = (size_t(i) * size_t(nX) + size_t(j)) * 2;
            uv[k] = float(j) / float(nX - 1);
            uv[k + 1] = float(i) / float(nY - 1);
        }
    }
    m_uvBuf.reset(m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, quint32(uv.size() * sizeof(float))));
    m_uvBuf->create();
    u->uploadStaticBuffer(m_uvBuf.get(), uv.data());
    m_fieldVal.reset(m_rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::VertexBuffer, quint32(nodes * sizeof(float))));
    m_fieldVal->create();
    m_epsVal.reset(m_rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::VertexBuffer, quint32(nodes * sizeof(float))));
    m_epsVal->create();
    m_fieldValVersion = m_epsValVersion = ~0ull;

    // Два треугольника на ячейку (четырёхугольник ii, ii+1, ii+1+nX, ii+nX оригинала).
    std::vector<quint32> tri;
    tri.reserve(size_t(nX - 1) * size_t(nY - 1) * 6);
    for (int i = 0; i < nY - 1; ++i) {
        for (int j = 0; j < nX - 1; ++j) {
            const quint32 ii = quint32(i) * quint32(nX) + quint32(j);
            tri.insert(tri.end(), {ii, ii + 1, ii + 1 + quint32(nX), ii, ii + 1 + quint32(nX), ii + quint32(nX)});
        }
    }
    m_triCount = quint32(tri.size());
    m_triIdx.reset(m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::IndexBuffer, quint32(std::max<size_t>(4, tri.size() * sizeof(quint32)))));
    m_triIdx->create();
    if (!tri.empty())
        u->uploadStaticBuffer(m_triIdx.get(), tri.data());

    // Каркас: вдоль линии — все узлы (линия идёт по рельефу); линии сетки — через шаг
    // (поверх заливки — не больше ~120 по стороне, без заливки — не больше ~1000).
    const auto wire = [&](int maxLines, quint32 *count) {
        const int step = std::max(1, int(std::ceil(std::max(nX, nY) / double(maxLines))));
        std::vector<quint32> w;
        for (int i = 0; i < nY; ++i) {
            if (i % step != 0 && i != nY - 1)
                continue;
            for (int j = 0; j < nX - 1; ++j) {
                const quint32 ii = quint32(i) * quint32(nX) + quint32(j);
                w.insert(w.end(), {ii, ii + 1});
            }
        }
        for (int j = 0; j < nX; ++j) {
            if (j % step != 0 && j != nX - 1)
                continue;
            for (int i = 0; i < nY - 1; ++i) {
                const quint32 ii = quint32(i) * quint32(nX) + quint32(j);
                w.insert(w.end(), {ii, ii + quint32(nX)});
            }
        }
        *count = quint32(w.size());
        std::unique_ptr<QRhiBuffer> b(m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::IndexBuffer,
                                                       quint32(std::max<size_t>(4, w.size() * sizeof(quint32)))));
        b->create();
        if (!w.empty())
            u->uploadStaticBuffer(b.get(), w.data());
        return b;
    };
    m_wireDec = wire(120, &m_wireDecCount);
    m_wireFull = wire(1000, &m_wireFullCount);
}

QMatrix4x4 FieldRhiView::viewProjection(const SceneData *scene, const CameraParams &c, double aspect)
{
    double sx = 2.5, sy = 2.5, hz = 1.0;
    if (scene) {
        sx = scene->boundX;
        sy = scene->boundY;
        hz = scene->boundZ;
    }
    aspect = std::max(0.05, aspect);
    // Расстояние камеры — чтобы сцена целиком помещалась при виде по умолчанию в окне
    // любой формы (у Windows-версии — постоянные −4,5 по Z при поле 2,5×2,5).
    // Полуразмеры повёрнутой на углы по умолчанию сцены в плоскости экрана (hx, hy) и
    // расстояние, при котором они помещаются в угол обзора с запасом 20 %.
    const double t = std::tan(kFovY / 2 * kPi / 180.0);
    const double a = kFitAzimuth * kPi / 180.0, tl = kFitTilt * kPi / 180.0;
    const double hx = std::fabs(std::cos(a)) * sx / 2 + std::fabs(std::sin(a)) * sy / 2;
    const double hy0 = std::fabs(std::sin(a)) * sx / 2 + std::fabs(std::cos(a)) * sy / 2;
    const double hy = std::max(hy0, hy0 * std::fabs(std::cos(tl)) + hz * std::fabs(std::sin(tl)));
    const double dist = std::max(hx / (t * aspect), hy / t) * 1.2 + 0.3;
    const double r = 0.5 * std::sqrt(sx * sx + sy * sy + 4 * hz * hz) * c.zoom + 0.6;
    const double camDist = std::max(0.05, dist - c.transZ);
    QMatrix4x4 proj;
    proj.perspective(float(kFovY), float(aspect), float(std::max(0.01, (camDist - r) * 0.5)),
                     float(camDist + r + 2.0));
    QMatrix4x4 view;
    view.translate(float(c.panX * dist * t * aspect + c.transX), float(c.panY * dist * t + c.transY),
                   float(-camDist));
    view.rotate(float(c.angleX), 1, 0, 0);   // glRotatef(wAngleX, 1, 0, 0)
    view.rotate(float(c.angleY), 0, 1, 0);   // glRotatef(wAngleY, 0, 1, 0)
    view.rotate(float(c.angleZ), 0, 0, 1);   // glRotatef(wAngleZ, 0, 0, 1)
    view.scale(float(c.zoom));
    return proj * view;
}

QMatrix4x4 FieldRhiView::viewProjection() const
{
    const double w = std::max(1, width()), h = std::max(1, height());
    return viewProjection(m_scene, m_camera, w / h);
}

bool FieldRhiView::project(const QVector3D &p, QPointF *px) const
{
    const QVector4D c = viewProjection() * QVector4D(p, 1.0f);
    if (c.w() <= 1e-6f)
        return false;
    const double nx = c.x() / c.w(), ny = c.y() / c.w();
    *px = QPointF((nx + 1.0) / 2.0 * width(), (1.0 - ny) / 2.0 * height());
    return true;
}

bool FieldRhiView::pickGround(const QPointF &px, QVector3D *out) const
{
    bool inv = false;
    // Луч из точки окна: точки на ближней и дальней плоскостях отсечения, пересечение с z = 0.
    const QMatrix4x4 m = viewProjection().inverted(&inv);
    if (!inv)
        return false;
    const float nx = float(2.0 * px.x() / std::max(1, width()) - 1.0);
    const float ny = float(1.0 - 2.0 * px.y() / std::max(1, height()));
    QVector4D a = m * QVector4D(nx, ny, -1.0f, 1.0f);
    QVector4D b = m * QVector4D(nx, ny, 1.0f, 1.0f);
    if (std::fabs(a.w()) < 1e-9f || std::fabs(b.w()) < 1e-9f)
        return false;
    const QVector3D p0 = a.toVector3D() / a.w(), p1 = b.toVector3D() / b.w();
    const float dz = p1.z() - p0.z();
    if (std::fabs(dz) < 1e-9f)
        return false;
    const float s = -p0.z() / dz;
    if (s < 0.0f)
        return false;
    *out = p0 + (p1 - p0) * s;
    return true;
}

void FieldRhiView::render(QRhiCommandBuffer *cb)
{
    QRhiResourceUpdateBatch *u = m_rhi->nextResourceUpdateBatch();
    const SceneData *s = m_scene;
    const size_t nodes = s ? size_t(s->nX) * size_t(s->nY) : 0;
    // С-62: размеры буферов и индексы — 32-битные; сетку больше kMaxSceneSide по стороне
    // FieldPane прореживает, здесь — защита от переполнения.
    const bool haveGrid = s && s->nX >= 2 && s->nY >= 2 && s->nX <= kMaxSceneSide && s->nY <= kMaxSceneSide;
    const auto layerOk = [&](const GridLayer &l) {
        return haveGrid && l.visible && l.values && l.values->size() == nodes && (l.surface || l.line);
    };
    const bool drawField = layerOk(s ? s->field : GridLayer());
    const bool drawEps = s && layerOk(s->eps);

    if (haveGrid && (m_gridX != s->nX || m_gridY != s->nY))
        rebuildGrid(u);
    if (drawField && m_fieldValVersion != s->field.valuesVersion) {
        u->uploadStaticBuffer(m_fieldVal.get(), 0, quint32(nodes * sizeof(float)), s->field.values->data());
        m_fieldValVersion = s->field.valuesVersion;
    }
    if (drawEps && m_epsValVersion != s->eps.valuesVersion) {
        u->uploadStaticBuffer(m_epsVal.get(), 0, quint32(nodes * sizeof(float)), s->eps.values->data());
        m_epsValVersion = s->eps.valuesVersion;
    }
    if (s && !s->field.palette.isNull() && m_fieldPalVersion != s->field.paletteVersion) {
        u->uploadTexture(m_fieldTex.get(), s->field.palette);
        m_fieldPalVersion = s->field.paletteVersion;
    }
    if (s && !s->eps.palette.isNull() && m_epsPalVersion != s->eps.paletteVersion) {
        u->uploadTexture(m_epsTex.get(), s->eps.palette);
        m_epsPalVersion = s->eps.paletteVersion;
    }
    if (s && m_geomVersion != s->geometryVersion) {
        const auto upload = [&](const std::vector<float> &v, std::unique_ptr<QRhiBuffer> &buf, quint32 &cap,
                                quint32 &verts) {
            verts = quint32(v.size() / 7);
            if (v.empty())
                return;
            const quint32 bytes = quint32(v.size() * sizeof(float));
            if (!buf || cap < bytes) {
                buf.reset(m_rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::VertexBuffer, bytes));
                buf->create();
                cap = bytes;
            }
            u->uploadStaticBuffer(buf.get(), 0, bytes, v.data());
        };
        upload(s->triangles, m_triBuf, m_triCap, m_triVerts);
        upload(s->lines, m_lineBuf, m_lineCap, m_lineVerts);
        m_geomVersion = s->geometryVersion;
    }

    // Матрица кадра — с поправкой системы координат интерфейса; ниже — константы проходов
    // сетки, по две на слой (заливка и каркас).
    const QSize px = renderTarget()->pixelSize();
    const QMatrix4x4 mvp = m_rhi->clipSpaceCorrMatrix() * viewProjection();

    const auto fill = [&](const GridLayer &l, bool wire, QRhiBuffer *ub) {
        GridUniforms g;
        std::memcpy(g.mvp, mvp.constData(), 64);
        g.rect[0] = s->x0;
        g.rect[1] = s->y0;
        g.rect[2] = s->sx;
        g.rect[3] = s->sy;
        g.map[0] = l.k;
        g.map[1] = l.off;
        g.map[2] = float(l.colorMode);
        g.map[3] = float(l.levels);
        g.misc[0] = l.dim3d ? l.heightScale : 0.0f;
        g.misc[1] = l.dim3d ? (wire ? 0.004f * std::max(0.3f, l.heightScale) : 0.0f) : (wire ? l.zLift2D + 0.01f : l.zLift2D);
        g.misc[2] = l.alpha;
        g.misc[3] = 1.0f;
        g.color[0] = g.color[1] = g.color[2] = g.color[3] = 1.0f;
        g.extra[0] = (!wire && l.dim3d) ? 0.35f : 0.0f;
        g.extra[1] = g.extra[2] = g.extra[3] = 0.0f;
        if (wire && l.surface) {
            // Каркас поверх заливки — один цвет (у Windows-версии белый 0,8); без заливки —
            // по шкале, как DrawFieldLine.
            g.misc[3] = 0.0f;
            g.color[0] = l.wireColor.redF();
            g.color[1] = l.wireColor.greenF();
            g.color[2] = l.wireColor.blueF();
            g.color[3] = l.wireColor.alphaF();
        }
        u->updateDynamicBuffer(ub, 0, sizeof(g), &g);
    };
    if (drawField) {
        fill(s->field, false, m_ubFieldSurf.get());
        fill(s->field, true, m_ubFieldWire.get());
    }
    if (drawEps) {
        fill(s->eps, false, m_ubEpsSurf.get());
        fill(s->eps, true, m_ubEpsWire.get());
    }
    u->updateDynamicBuffer(m_ubColor.get(), 0, 64, mvp.constData());

    // Картинки поверх сцены.
    std::vector<int> spriteDraw;
    if (m_sprites) {
        QMatrix4x4 ortho;
        ortho.ortho(0, float(width()), float(height()), 0, -1, 1);
        const QMatrix4x4 sp = m_rhi->clipSpaceCorrMatrix() * ortho;
        u->updateDynamicBuffer(m_ubSprite.get(), 0, 64, sp.constData());
        std::vector<float> verts;
        for (size_t i = 0; i < m_sprites->size() && i < 8; ++i) {
            const Sprite &spr = (*m_sprites)[i];
            if (m_spriteGpu.size() <= i)
                m_spriteGpu.push_back(std::make_unique<SpriteGpu>());
            SpriteGpu &g = *m_spriteGpu[i];
            if (!spr.visible || spr.image.isNull() || spr.rect.isEmpty())
                continue;
            if (g.version != spr.version || !g.tex || !g.srb) {
                if (!g.tex || g.size != spr.image.size()) {
                    g.tex.reset(m_rhi->newTexture(QRhiTexture::RGBA8, spr.image.size()));
                    g.tex->create();
                    g.size = spr.image.size();
                    g.srb.reset();
                }
                if (!g.srb) {
                    g.srb.reset(m_rhi->newShaderResourceBindings());
                    g.srb->setBindings({QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, m_ubSprite.get()),
                                        QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage,
                                                                                  g.tex.get(), m_spriteSampler.get())});
                    g.srb->create();
                }
                u->uploadTexture(g.tex.get(), spr.image);
                g.version = spr.version;
            }
            const float x0 = float(spr.rect.left()), y0 = float(spr.rect.top());
            const float x1 = float(spr.rect.right()), y1 = float(spr.rect.bottom());
            const float q[] = {x0, y0, 0, 0, x1, y0, 1, 0, x1, y1, 1, 1,
                               x0, y0, 0, 0, x1, y1, 1, 1, x0, y1, 0, 1};
            const size_t at = verts.size();
            verts.insert(verts.end(), std::begin(q), std::end(q));
            Q_UNUSED(at);
            spriteDraw.push_back(int(i));
        }
        if (!verts.empty())
            u->updateDynamicBuffer(m_spriteVerts.get(), 0, quint32(verts.size() * sizeof(float)), verts.data());
    }

    // Порядок рисования: поле, слой eps, цветные треугольники, цветные линии, картинки.
    cb->beginPass(renderTarget(), theme::bg(), {1.0f, 0}, u);
    const QRhiViewport vp(0, 0, float(px.width()), float(px.height()));
    if (haveGrid) {
        const auto drawLayer = [&](const GridLayer &l, QRhiBuffer *vals, QRhiShaderResourceBindings *surf,
                                   QRhiShaderResourceBindings *wireSrb) {
            const QRhiCommandBuffer::VertexInput vin[] = {{m_uvBuf.get(), 0}, {vals, 0}};
            if (l.surface && m_triCount > 0) {
                cb->setGraphicsPipeline(m_psSurf.get());
                cb->setViewport(vp);
                cb->setShaderResources(surf);
                cb->setVertexInput(0, 2, vin, m_triIdx.get(), 0, QRhiCommandBuffer::IndexUInt32);
                cb->drawIndexed(m_triCount);
            }
            if (l.line) {
                QRhiBuffer *idx = l.surface ? m_wireDec.get() : m_wireFull.get();
                const quint32 n = l.surface ? m_wireDecCount : m_wireFullCount;
                if (n > 0) {
                    cb->setGraphicsPipeline(m_psWire.get());
                    cb->setViewport(vp);
                    cb->setShaderResources(wireSrb);
                    cb->setVertexInput(0, 2, vin, idx, 0, QRhiCommandBuffer::IndexUInt32);
                    cb->drawIndexed(n);
                }
            }
        };
        if (drawField)
            drawLayer(s->field, m_fieldVal.get(), m_srbFieldSurf.get(), m_srbFieldWire.get());
        if (drawEps)
            drawLayer(s->eps, m_epsVal.get(), m_srbEpsSurf.get(), m_srbEpsWire.get());
    }
    if (m_triVerts > 0 && m_triBuf) {
        const QRhiCommandBuffer::VertexInput tin(m_triBuf.get(), 0);
        cb->setGraphicsPipeline(m_psTri.get());
        cb->setViewport(vp);
        cb->setShaderResources(m_srbColor.get());
        cb->setVertexInput(0, 1, &tin);
        cb->draw(m_triVerts);
    }
    if (m_lineVerts > 0 && m_lineBuf) {
        const QRhiCommandBuffer::VertexInput lin(m_lineBuf.get(), 0);
        cb->setGraphicsPipeline(m_psLine.get());
        cb->setViewport(vp);
        cb->setShaderResources(m_srbColor.get());
        cb->setVertexInput(0, 1, &lin);
        cb->draw(m_lineVerts);
    }
    for (size_t k = 0; k < spriteDraw.size(); ++k) {
        SpriteGpu &g = *m_spriteGpu[size_t(spriteDraw[k])];
        const QRhiCommandBuffer::VertexInput sin(m_spriteVerts.get(), quint32(k * 6 * 4 * sizeof(float)));
        cb->setGraphicsPipeline(m_psSprite.get());
        cb->setViewport(vp);
        cb->setShaderResources(g.srb.get());
        cb->setVertexInput(0, 1, &sin);
        cb->draw(6);
    }
    cb->endPass();

    ++m_frames;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_fpsStart >= 1000) {
        m_fps = m_frames * 1000.0 / double(now - m_fpsStart);
        m_frames = 0;
        m_fpsStart = now;
    }
    Q_EMIT frameRendered();
}

// --- Мышь: левая — поворот, правая (щелчок) — параметры, правая/средняя (тянуть) — сдвиг,
// колесо — масштаб, двойной щелчок — перечитать файл, Shift+щелчок — точка поля.

void FieldRhiView::mousePressEvent(QMouseEvent *e)
{
    m_dragButton = e->button();
    m_lastPos = m_pressPos = e->position();
    m_dragged = false;
    if (e->button() == Qt::LeftButton && (e->modifiers() & Qt::ShiftModifier)) {
        Q_EMIT probeRequested(e->position());
        m_dragButton = Qt::NoButton;
    }
    e->accept();
}

void FieldRhiView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragButton == Qt::NoButton)
        return;
    const QPointF d = e->position() - m_lastPos;
    if ((e->position() - m_pressPos).manhattanLength() > 4)
        m_dragged = true;
    m_lastPos = e->position();
    if (m_dragButton == Qt::LeftButton)
        Q_EMIT rotateRequested(d.x(), d.y());
    else if (m_dragButton == Qt::RightButton || m_dragButton == Qt::MiddleButton)
        Q_EMIT panRequested(2.0 * d.x() / std::max(1, width()), -2.0 * d.y() / std::max(1, height()));
}

void FieldRhiView::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_dragButton == Qt::RightButton && !m_dragged && e->button() == Qt::RightButton)
        Q_EMIT rightClicked(e->globalPosition().toPoint());   // OnRButtonDown → OnViewParameters
    m_dragButton = Qt::NoButton;
}

void FieldRhiView::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        Q_EMIT doubleClicked();
}

void FieldRhiView::wheelEvent(QWheelEvent *e)
{
    Q_EMIT zoomRequested(std::pow(1.0015, e->angleDelta().y()));
    e->accept();
}

} // namespace fieldview
