// FieldPane — описание в fieldpane.h.
/// @file fieldpane.cpp
/// @brief Окно документа: окно данных, пределы Z и шкалы, слой eps, геометрия топологии и осей, надписи, команды.
///
/// Часть модуля FieldView (слой `app`). Описание класса — в `fieldpane.h`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#pane
#include "fieldpane.h"
#include "theme.h"

#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QVBoxLayout>

#include <algorithm>
#include <cfloat>
#include <cmath>

namespace fieldview {

namespace {

/// Полувысота объёмных блоков топологии в 3D, единицы сцены (призма от -0,25 до 0,25).
constexpr float kTopZ3D = 0.25f;    // dZtopol: блоки топологии в 3D — от −0,25 до 0,25
/// Высота заливки топологии в 2D, единицы сцены.
constexpr float kTopZ2D = 0.02f;    // в 2D — на высоте 0,02 (DrawTopSurface2)
/// Высота контуров топологии в 2D, единицы сцены (над заливкой).
constexpr float kTopLine2D = 0.03f; // контуры в 2D (DrawTopdLine2)
/// Размер стрелок размерных линий, единицы сцены.
constexpr double kStrelk = 0.02;    // dZtopolStrelk — размер стрелок размерных линий

/// @brief Форматирует число как `printf("%lg")` (6 значащих цифр).
/// @param[in] v  число
/// @return Текст.
QString num(double v) { return QString::number(v, 'g', 6); }   // "%lg"

/// @brief Построитель цветной геометрии сцены: треугольники и отрезки.
///
/// Вершины цветной геометрии (x, y, z, r, g, b, a).
struct Geo
{
    /// Буфер вершин треугольников (`SceneData::triangles`).
    std::vector<float> *tri;
    /// Буфер вершин отрезков (`SceneData::lines`).
    std::vector<float> *lin;
    /// @brief Дописывает вершину: координаты и цвет RGBA (0 … 1).
    /// @param[in,out] v  буфер вершин
    /// @param[in]     p  точка сцены
    /// @param[in]     c  цвет
    static void put(std::vector<float> *v, const QVector3D &p, const QColor &c)
    {
        v->insert(v->end(), {p.x(), p.y(), p.z(), float(c.redF()), float(c.greenF()), float(c.blueF()),
                             float(c.alphaF())});
    }
    /// @brief Добавляет треугольник одного цвета.
    /// @param[in] a    вершина 1
    /// @param[in] b    вершина 2
    /// @param[in] c    вершина 3
    /// @param[in] col  цвет
    void triangle(const QVector3D &a, const QVector3D &b, const QVector3D &c, const QColor &col)
    {
        put(tri, a, col);
        put(tri, b, col);
        put(tri, c, col);
    }
    /// @brief Добавляет четырёхугольник одного цвета (два треугольника a-b-c и a-c-d).
    /// @param[in] a    вершина 1
    /// @param[in] b    вершина 2
    /// @param[in] c    вершина 3
    /// @param[in] d    вершина 4
    /// @param[in] col  цвет
    void quad(const QVector3D &a, const QVector3D &b, const QVector3D &c, const QVector3D &d, const QColor &col)
    {
        triangle(a, b, c, col);
        triangle(a, c, d, col);
    }
    /// @brief Добавляет отрезок одного цвета.
    /// @param[in] a    начало
    /// @param[in] b    конец
    /// @param[in] col  цвет
    void line(const QVector3D &a, const QVector3D &b, const QColor &col)
    {
        put(lin, a, col);
        put(lin, b, col);
    }
};

/// @brief Возвращает номер цвета материала для типа блока.
/// @param[in] type  тип блока (`BlockType`)
/// @return 0 — eps, 1 — металл, 2 — магнитная стенка, 3 — вход, 4 — поглотитель, 5 — плазма.
int topoSlot(int type)
{
    switch (type) {
    case kTypeMetal: return 1;
    case kTypeMagnetic: return 2;
    case kTypeInpXLeft:
    case kTypeInpXRight:
    case kTypeInpYTop:
    case kTypeInpYBot: return 3;
    case kTypeAbsorber: return 4;
    case kTypePlasma: return 5;
    default: return 0;
    }
}

/// @brief Возвращает цвет с заданной непрозрачностью.
/// @param[in] c  цвет
/// @param[in] a  непрозрачность, обрезается до [0, 1]
/// @return Цвет с альфой `a`.
QColor withAlpha(QColor c, double a)
{
    c.setAlphaF(float(std::clamp(a, 0.0, 1.0)));
    return c;
}

/// @brief Создаёт прозрачную картинку для `Sprite`.
///
/// Картинка поверх сцены заданного логического размера.
/// @param[in] size  размер, логические пиксели
/// @param[in] dpr   плотность точек экрана
/// @return Картинка RGBA8888 с предумноженной альфой.
QImage spriteImage(const QSizeF &size, qreal dpr)
{
    QImage img(QSize(std::max(1, int(std::ceil(size.width() * dpr))), std::max(1, int(std::ceil(size.height() * dpr)))),
               QImage::Format_RGBA8888_Premultiplied);
    img.setDevicePixelRatio(dpr);
    img.fill(Qt::transparent);
    return img;
}

/// @brief Рисует текст с тёмной обводкой (читается на любом фоне).
/// @param[in,out] p      рисовальщик
/// @param[in]     at     точка базовой линии начала текста
/// @param[in]     text   текст
/// @param[in]     color  цвет текста
/// @param[in]     font   шрифт
void drawHaloText(QPainter &p, const QPointF &at, const QString &text, const QColor &color, const QFont &font)
{
    QPainterPath path;
    path.addText(at, font, text);
    p.setRenderHint(QPainter::Antialiasing);
    p.strokePath(path, QPen(QColor(10, 14, 22, 210), 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.fillPath(path, color);
}

/// @brief Возвращает « единица» для подписи или пусто.
/// @param[in] u  имя единицы
/// @return Пробел и имя единицы; пусто, если имени нет.
QString unitSuffix(const QString &u) { return u.isEmpty() ? QString() : QStringLiteral(" ") + u; }

} // namespace

FieldPane::FieldPane(std::shared_ptr<FieldDocument> doc, const ViewSettings &settings, QWidget *parent)
    : QWidget(parent), m_doc(std::move(doc)), m_s(settings)
{
    setAttribute(Qt::WA_DeleteOnClose);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    m_view = new FieldRhiView(this);
    lay->addWidget(m_view);
    m_sprites.resize(5);
    m_view->setScene(&m_scene);
    m_view->setSprites(&m_sprites);

    m_playTimer.setInterval(std::max(10, 1000 / std::max(1, m_s.speed)));
    m_playTimer.setTimerType(Qt::PreciseTimer);   // обычный таймер Windows шагает по 15,6 мс
    connect(&m_playTimer, &QTimer::timeout, this, [this]() { stepFrame(1); });

    connect(m_doc.get(), &FieldDocument::framesChanged, this, &FieldPane::onFramesChanged);
    connect(m_doc.get(), &FieldDocument::frameArrived, this, &FieldPane::onFrameArrived);
    connect(m_doc.get(), &FieldDocument::topologyChanged, this, [this]() {
        updateEps();
        updateGeometry();
        updateHud();
    });
    connect(m_doc.get(), &FieldDocument::errorChanged, this, [this]() {
        updateHud();
        updateStatus();
    });
    connect(m_doc.get(), &FieldDocument::syncChanged, this, &FieldPane::stateChanged);
    connect(m_doc.get(), &FieldDocument::statsChanged, this, [this]() {
        updateFieldRange();
        updateGeometry();
        updateHud();
    });

    connect(m_view, &FieldRhiView::rotateRequested, this, [this](double dx, double dy) {
        m_s.camera.angleZ = std::remainder(m_s.camera.angleZ + dx * 0.4, 360.0);
        m_s.camera.angleX = std::clamp(m_s.camera.angleX + dy * 0.4, -180.0, 180.0);
        settingsChanged(false);
    });
    connect(m_view, &FieldRhiView::panRequested, this, [this](double dx, double dy) {
        CameraParams c = m_view->camera();
        c.panX += dx;
        c.panY += dy;
        m_view->setCamera(c);
        drawLabelsSprite();
    });
    connect(m_view, &FieldRhiView::zoomRequested, this, [this](double f) {
        m_s.camera.zoom = std::clamp(m_s.camera.zoom * f, 0.05, 50.0);
        settingsChanged(false);
    });
    connect(m_view, &FieldRhiView::doubleClicked, this, [this]() {
        resetZRange();
        reread();
    });
    connect(m_view, &FieldRhiView::rightClicked, this, &FieldPane::requestParameters);
    connect(m_view, &FieldRhiView::probeRequested, this, [this](const QPointF &px) {
        QVector3D p;
        if (!m_view->pickGround(px, &p) || m_kx <= 0 || m_ky <= 0)
            return;
        // Обратно к XtoGLWinCoord: x = xmin + (X/(2,5·kx) + 0,5)·(xmax − xmin).
        m_s.probeX = m_xMin + (p.x() / (2.5 * m_kx) + 0.5) * (m_xMax - m_xMin);
        m_s.probeY = m_yMin + (p.y() / (2.5 * m_ky) + 0.5) * (m_yMax - m_yMin);
        m_s.probe = true;
        settingsChanged(true);
    });
    connect(m_view, &FieldRhiView::backendChanged, this, &FieldPane::statusChanged);

    onFramesChanged();
}

FieldPane::~FieldPane() = default;

QString FieldPane::title() const
{
    return m_doc->fileName();
}

int FieldPane::frameCount() const
{
    return m_doc->frameCount();
}

void FieldPane::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    updateHud();
    drawLabelsSprite();
}

// --- Такты ----------------------------------------------------------------------

void FieldPane::onFramesChanged()
{
    const int n = m_doc->frameCount();
    if (n > 0 && (m_follow || m_frame >= n))
        m_frame = n - 1;
    if (m_frame < 0)
        m_frame = 0;
    rebuildAll();
    Q_EMIT stateChanged();
}

void FieldPane::onFrameArrived(int index)
{
    // Новый кадр от ядра показывается сразу, если пользователь не листает историю.
    if (m_follow && !m_playing)
        setFrame(index);
}

void FieldPane::setFrame(int f)
{
    const int n = m_doc->frameCount();
    if (n == 0)
        return;
    f = std::clamp(f, 0, n - 1);
    m_follow = (f == n - 1);
    if (f == m_frame && m_current)
        return;
    m_frame = f;
    const double zMin = m_zMin, zMax = m_zMax;
    updateFieldValues();
    // Геометрия и шкала зависят от такта, только если изменились пределы или стоит
    // метка значения — иначе при проигрывании обновляются лишь значения и сведения.
    const bool rangeChanged = zMin != m_zMin || zMax != m_zMax;
    if (rangeChanged || m_s.probe)
        updateGeometry();
    updateHud(!rangeChanged);
    updateStatus();
    Q_EMIT stateChanged();
}

void FieldPane::stepFrame(int delta)
{
    const int n = m_doc->frameCount();
    if (n <= 1)
        return;
    setFrame(((m_frame + delta) % n + n) % n);
}

void FieldPane::setPlaying(bool on)
{
    if (on && m_doc->frameCount() <= 1)
        on = false;
    m_playing = on;
    if (on)
        m_playTimer.start();
    else
        m_playTimer.stop();
    Q_EMIT stateChanged();
}

void FieldPane::setSpeed(int fps)
{
    m_s.speed = std::clamp(fps, 1, 60);
    m_playTimer.setInterval(std::max(10, 1000 / m_s.speed));
    m_s.save();
}

void FieldPane::setGain(double g)
{
    g = std::clamp(g, 1.0, 100.0);
    if (g == m_s.field.gain)
        return;
    m_s.field.gain = g;
    settingsChanged(true);
}

// --- Подготовка данных ----------------------------------------------------------

void FieldPane::rebuildAll()
{
    updateWindow();
    updateFieldValues();
    updateEps();
    updateGeometry();
    updateHud();
    updateStatus();
}

void FieldPane::updateWindow()
{
    // CFldViewView::PrepareData: окно данных и пропорции.
    m_windowError.clear();
    if (!m_doc->hasData()) {
        m_nX1 = m_nY1 = 0;
        return;
    }
    const FieldHeader &h = m_doc->header(0);
    m_delta = h.delta > 0 ? h.delta : 0.001;
    m_nX1 = h.nX;
    m_nY1 = h.nY;
    PlaceSettings &p = m_s.place;
    // Окно данных — узлы сетки [j0, j1] × [i0, i1]. С-62: номер узла — floor(x/Δ + 1e-9),
    // а не (int)(x/Δ): при x = kΔ деление даёт k − 1e-16 и усечение брало соседний узел.
    // Края окна привязаны к узлам (m_xMin = j0·Δ, m_xMax = j1·Δ): иначе после зума поле
    // рисовалось со сдвигом до одной ячейки относительно топологии и метки «Value».
    const auto nodeOf = [this](double x, int n) {
        return std::clamp(int(std::floor(x / m_delta + 1e-9)), 0, std::max(0, n - 1));
    };
    const auto axis = [&](bool &autoFlag, double &lo, double &hi, int n1, int *k0, int *k1) {
        const double ext = (n1 - 1) * m_delta;
        if (!autoFlag) {
            if (lo > hi)
                std::swap(lo, hi);
            if (lo < 0.0)
                lo = 0.0;
            if (hi > ext)
                hi = ext;
            // Н-35: окно из настроек от другого, большего файла лежит за краем этой сетки —
            // показать весь файл, а не «number of point < 2».
            if (lo > ext - 0.5 * m_delta)
                autoFlag = true;
        }
        if (autoFlag) {
            lo = 0.0;
            hi = ext;
            *k0 = 0;
            *k1 = n1 - 1;
        } else {
            *k0 = nodeOf(lo, n1);
            *k1 = nodeOf(hi, n1);
        }
    };
    int j1 = 0, i1 = 0;
    axis(p.xAuto, p.xMin, p.xMax, m_nX1, &m_j0, &j1);
    axis(p.yAuto, p.yMin, p.yMax, m_nY1, &m_i0, &i1);
    int nX = j1 - m_j0 + 1;
    int nY = i1 - m_i0 + 1;
    if (nX < 2)
        m_windowError = QStringLiteral("number of point along X axies < 2");
    else if (nY < 2)
        m_windowError = QStringLiteral("number of point along Y axies < 2");
    // С-62: большое окно — каждый k-й узел (сетка сцены не больше kMaxSceneSide по стороне).
    // Шаг прореживания k = ceil((n - 1) / (kMaxSceneSide - 1)): после него узлов не больше kMaxSceneSide.
    const auto strideFor = [](int n) {
        return n - 1 > kMaxSceneSide - 1 ? (n - 1 + kMaxSceneSide - 2) / (kMaxSceneSide - 1) : 1;
    };
    m_strideX = m_strideY = 1;
    if (m_windowError.isEmpty()) {
        m_strideX = strideFor(nX);
        m_strideY = strideFor(nY);
        nX = (nX - 1) / m_strideX + 1;
        nY = (nY - 1) / m_strideY + 1;
        j1 = m_j0 + (nX - 1) * m_strideX;
        i1 = m_i0 + (nY - 1) * m_strideY;
    }
    m_xMin = m_j0 * m_delta;
    m_xMax = std::max(j1 * m_delta, m_xMin + m_delta);
    m_yMin = m_i0 * m_delta;
    m_yMax = std::max(i1 * m_delta, m_yMin + m_delta);
    if (p.proportional) {
        if ((m_xMax - m_xMin) < (m_yMax - m_yMin)) {
            m_kx = (m_xMax - m_xMin) / (m_yMax - m_yMin);
            m_ky = 1.0;
        } else {
            m_kx = 1.0;
            m_ky = (m_yMax - m_yMin) / (m_xMax - m_xMin);
        }
    } else {
        m_kx = m_ky = 1.0;
    }
    const int oldX = m_scene.nX, oldY = m_scene.nY;
    m_scene.nX = m_windowError.isEmpty() ? nX : 0;
    m_scene.nY = m_windowError.isEmpty() ? nY : 0;
    // XtoGLWinCoord(int j) = kx·2,5·(j·Δ/(xmax − xmin) − 0,4); здесь центр сцены — в нуле.
    m_scene.x0 = float(-1.25 * m_kx);
    m_scene.y0 = float(-1.25 * m_ky);
    m_scene.sx = float(m_kx * 2.5 * (std::max(nX, 2) - 1) * m_strideX * m_delta / (m_xMax - m_xMin));
    m_scene.sy = float(m_ky * 2.5 * (std::max(nY, 2) - 1) * m_strideY * m_delta / (m_yMax - m_yMin));
    m_scene.boundX = float(2.5 * m_kx);
    m_scene.boundY = float(2.5 * m_ky);
    if (oldX != m_scene.nX || oldY != m_scene.nY) {
        ++m_scene.gridVersion;
        m_accValid = false;   // другое окно — пределы заново
    }
}

double FieldPane::sceneX(double x) const
{
    return m_kx * 2.5 * ((x - m_xMin) / (m_xMax - m_xMin) - 0.5);
}

double FieldPane::sceneY(double y) const
{
    return m_ky * 2.5 * ((y - m_yMin) / (m_yMax - m_yMin) - 0.5);
}

double FieldPane::zf(double v) const
{
    // ZtoGLWinCoord с учётом множителя высоты рельефа.
    const double h = 2.0 * ((v - m_zMin) / (m_zMax - m_zMin) - 0.5);
    return h * (m_s.field.dim3d ? m_s.field.heightScale : 1.0);
}

void FieldPane::updateFieldValues()
{
    GridLayer &L = m_scene.field;
    const FieldLayerSettings &fs = m_s.field;
    m_current.reset();
    if (m_doc->hasData())
        m_current = m_doc->frame(std::clamp(m_frame, 0, m_doc->frameCount() - 1));
    const int nX = m_scene.nX, nY = m_scene.nY;
    L.visible = m_current && nX >= 2 && nY >= 2 && (fs.surface || fs.line);
    if (!m_current || nX < 2 || nY < 2) {
        L.values.reset();
        updateFieldRange();
        return;
    }
    if (wholeGrid()) {
        L.values = std::shared_ptr<const std::vector<float>>(m_current, &m_current->values);
    } else {
        auto v = std::make_shared<std::vector<float>>(size_t(nX) * size_t(nY));
        for (int i = 0; i < nY; ++i) {
            const float *src = m_current->values.data() + size_t(m_i0 + i * m_strideY) * size_t(m_nX1) + size_t(m_j0);
            float *dst = v->data() + size_t(i) * size_t(nX);
            if (m_strideX == 1)
                std::copy(src, src + nX, dst);
            else
                for (int j = 0; j < nX; ++j)
                    dst[j] = src[size_t(j) * size_t(m_strideX)];
        }
        L.values = v;
    }
    ++L.valuesVersion;
    updateFieldRange();
}

void FieldPane::updateFieldRange()
{
    GridLayer &L = m_scene.field;
    FieldLayerSettings &fs = m_s.field;
    // Размах показываемых значений (по окну данных).
    FieldStats cur;
    bool have = false;
    if (L.values && !L.values->empty()) {
        if (wholeGrid() && m_current) {
            cur = m_current->stats;
        } else {
            // Только конечные значения (С-56, С-57): nan/inf не должны задавать шкалу.
            bool first = true;
            for (const float x : *L.values) {
                if (!std::isfinite(x)) {
                    ++cur.nonFinite;
                    continue;
                }
                if (first) {
                    cur.min = cur.max = x;
                    first = false;
                } else {
                    cur.min = std::min(cur.min, double(x));
                    cur.max = std::max(cur.max, double(x));
                }
            }
            cur.maxAbs = std::max(std::fabs(cur.min), std::fabs(cur.max));
        }
        have = true;
        // Накопление по показанным тактам — как dZmin/dZmax оригинала при bZsizeFlag.
        if (!m_accValid) {
            m_accMin = cur.min;
            m_accMax = cur.max;
            m_accAbs = cur.maxAbs;
            m_accValid = true;
        } else {
            m_accMin = std::min(m_accMin, cur.min);
            m_accMax = std::max(m_accMax, cur.max);
            m_accAbs = std::max(m_accAbs, cur.maxAbs);
        }
    }
    FieldStats s = cur;
    if (have && !fs.zPerFrame) {
        s.min = m_accMin;
        s.max = m_accMax;
        s.maxAbs = m_accAbs;
        // Размах уже прочитанных тактов всего файла (без окна данных — он точный).
        FieldStats tot;
        if (wholeGrid() && m_doc->totalStats(&tot)) {
            s.min = std::min(s.min, tot.min);
            s.max = std::max(s.max, tot.max);
            s.maxAbs = std::max(s.maxAbs, tot.maxAbs);
        }
    }
    // Шкала: «Авто» — «Волна» для знакопеременных, «Свечение» для неотрицательных.
    // Пределы Z: ручные — как заданы (в режиме «Модуль» — симметрично); автоматические:
    // «Модуль» и «Волна» — +-max|v| / gain, «Свечение» — от min(0, vmin) / gain до vmax / gain,
    // 20 цветов — от vmin / gain до vmax / gain. Вырожденный диапазон расширяется на
    // +-max(1e-6 * |z|, FLT_MIN).
    ScaleKind eff = fs.scale;
    if (fs.modulus)
        eff = fs.scale == ScaleKind::Custom ? ScaleKind::Custom : ScaleKind::Glow;
    else if (eff == ScaleKind::Auto)
        eff = (have && s.min >= 0.0) ? ScaleKind::Glow : ScaleKind::Wave;
    double zmin, zmax;
    const double g = std::max(1.0, fs.gain);   // контраст (новое), только для авто-пределов
    if (!fs.zAuto) {
        zmin = std::min(fs.zMin, fs.zMax);
        zmax = std::max(fs.zMin, fs.zMax);
        if (fs.modulus) {   // OnUserApplyPropDialogField: симметрично
            zmax = std::max(std::fabs(zmin), std::fabs(zmax));
            zmin = -zmax;
        }
    } else if (!have) {
        zmin = -1.0;
        zmax = 1.0;
    } else if (s.maxAbs < double(FLT_MIN)) {
        // Поле всюду ноль (волна ещё не дошла): пределы ±1, чтобы шкала читалась.
        zmin = fs.modulus || eff == ScaleKind::Wave ? -1.0 : 0.0;
        zmax = 1.0;
    } else if (fs.modulus) {
        // bDrawFieldModulFlag: dZmax = max|v|, dZmin = −dZmax.
        zmax = std::max(s.maxAbs / g, double(FLT_MIN));
        zmin = -zmax;
    } else if (eff == ScaleKind::Wave) {
        // «Волна» симметрична: ноль — тёмный (оформление пакета).
        zmax = std::max(s.maxAbs / g, double(FLT_MIN));
        zmin = -zmax;
    } else if (eff == ScaleKind::Glow) {
        zmin = std::min(0.0, s.min) / g;
        zmax = s.max / g;
    } else {
        zmin = s.min / g;   // как в оригинале: от минимума до максимума
        zmax = s.max / g;
    }
    if (!(zmax - zmin > std::max(1e-300, std::fabs(zmax) * 1e-12))) {
        const double r = zmax;
        const double e = std::max(std::fabs(r) * 1e-6, double(FLT_MIN));
        zmin = r - e;
        zmax = r + e;
    }
    m_zMin = zmin;
    m_zMax = zmax;
    if (fs.zAuto) {
        fs.zMin = zmin;
        fs.zMax = zmax;
    }
    // Нормировка для шейдера: zmin -> h = -1, zmax -> h = +1.
    L.k = float(2.0 / (zmax - zmin));
    L.off = float(-2.0 * zmin / (zmax - zmin) - 1.0);
    L.colorMode = fs.modulus ? 1 : 0;
    L.levels = fs.smooth ? 0 : 20;
    L.surface = fs.surface;
    L.line = fs.line;
    L.dim3d = fs.dim3d;
    L.alpha = float(fs.blend / 100.0);
    L.heightScale = float(fs.heightScale);
    // Каркас поверх заливки: в рельефе — линии сетки графиков, на плоской карте — тоньше.
    L.wireColor = fs.dim3d ? withAlpha(theme::line().lighter(125), 0.9) : withAlpha(theme::line().lighter(150), 0.45);
    L.zLift2D = 0.0f;
    const QImage pal = scaleImage(eff, fs.custom, 256);
    if (pal != L.palette || eff != m_fieldScale) {
        L.palette = pal;
        ++L.paletteVersion;
    }
    m_fieldScale = eff;
}

bool FieldPane::hasPlasma() const
{
    return m_doc->hasTopology() && m_doc->topology().hasPlasma();
}

int FieldPane::effectiveEpsType() const
{
    const int t = std::clamp(m_s.eps.type, 0, 3);
    return (t != 0 && !hasPlasma()) ? 0 : t;
}

bool FieldPane::epsAvailable() const
{
    return m_doc->topologyMatchesGrid();
}

QString FieldPane::epsUnavailableReason() const
{
    if (!m_doc->hasTopology())
        return m_doc->topologyError();
    if (!m_doc->topologyMatchesGrid())
        return m_doc->topologyMismatch();   // А-34: nX, nY, Delta, Xmin, Ymin
    return QString();
}

void FieldPane::updateEps()
{
    // CFldViewView::PrepareTpl.
    GridLayer &L = m_scene.eps;
    EpsLayerSettings &es = m_s.eps;
    const int nX = m_scene.nX, nY = m_scene.nY;
    L.visible = false;
    L.values.reset();
    m_epsReason.clear();
    if (!(es.surface || es.line) || nX < 2 || nY < 2)
        return;
    if (!epsAvailable()) {
        m_epsReason = epsUnavailableReason();
        return;
    }
    const int type = effectiveEpsType();
    const std::vector<double> &g = m_doc->topology().grid(type);
    if (g.size() != size_t(m_nX1) * size_t(m_nY1))
        return;
    auto v = std::make_shared<std::vector<float>>(size_t(nX) * size_t(nY));
    double gmin = 0, gmax = 0;
    bool first = true;
    for (int i = 0; i < nY; ++i) {
        for (int j = 0; j < nX; ++j) {
            const double x = g[size_t(m_i0 + i * m_strideY) * size_t(m_nX1) + size_t(m_j0 + j * m_strideX)];
            (*v)[size_t(i) * size_t(nX) + size_t(j)] = float(x);
            if (!std::isfinite(x))
                continue;   // Н-35: nan в сетке N (ядро) не задаёт шкалу
            if (first) {
                gmin = gmax = x;
                first = false;
            } else {
                gmin = std::min(gmin, x);
                gmax = std::max(gmax, x);
            }
        }
    }
    // Пределы слоя eps — по значениям окна данных (без накопления по тактам).
    double zmin = es.zAuto ? gmin : std::min(es.zMin, es.zMax);
    double zmax = es.zAuto ? gmax : std::max(es.zMin, es.zMax);
    if (std::fabs(zmax - zmin) < std::max(double(FLT_MIN), std::fabs(zmax) * 1e-9)) {
        const double r = zmax;
        const double e = std::max(1.0, std::fabs(r)) * 1e-6;
        zmin = r - e;
        zmax = r + e;
    }
    if (es.zAuto) {
        es.zMin = gmin;
        es.zMax = gmax;
    }
    m_eMin = zmin;
    m_eMax = zmax;
    L.values = v;
    ++L.valuesVersion;
    L.visible = true;
    L.surface = es.surface;
    L.line = es.line;
    L.dim3d = es.dim3d;
    L.alpha = float(es.blend / 100.0);
    L.k = float(2.0 / (zmax - zmin));             // Z_EpstoGLWinCoord
    L.off = float(-2.0 * zmin / (zmax - zmin) - 1.0);
    L.colorMode = 0;                              // SetColor_Eps: 10 + z·10
    L.levels = es.smooth ? 0 : 20;
    L.heightScale = 1.0f;
    L.wireColor = withAlpha(theme::textDim(), 0.55);
    L.zLift2D = 0.002f;                           // поверх плоского поля (рисуется после него)
    const ScaleKind eff = es.scale == ScaleKind::Auto ? ScaleKind::Glow : es.scale;
    const QImage pal = scaleImage(eff, es.custom, 256);
    if (pal != L.palette || eff != m_epsScale) {
        L.palette = pal;
        ++L.paletteVersion;
    }
    m_epsScale = eff;
}

void FieldPane::updateGeometry()
{
    m_scene.triangles.clear();
    m_scene.lines.clear();
    m_scene.labels.clear();
    m_scene.boundZ = float(std::max({m_s.field.dim3d ? m_s.field.heightScale : 0.3,
                                     m_s.eps.dim3d && m_scene.eps.visible ? 1.0 : 0.0, m_s.topo.dim3d ? 0.5 : 0.0}));
    Geo g{&m_scene.triangles, &m_scene.lines};
    const TopologySettings &ts = m_s.topo;
    const PlaceSettings &ps = m_s.place;
    const bool haveGrid = m_scene.nX >= 2 && m_scene.nY >= 2;
    const QColor axes = ps.axesColor;
    const FieldHeader *hdr = m_doc->hasData() ? &m_doc->header(0) : nullptr;
    const double lu = (hdr && hdr->longUnitValue > 0) ? hdr->longUnitValue : 1.0;
    const QString luName = hdr ? hdr->longUnit : QStringLiteral("m");
    const auto P = [&](double x, double y, double z) { return QVector3D(float(sceneX(x)), float(sceneY(y)), float(z)); };
    const double dx = m_xMax - m_xMin, dy = m_yMax - m_yMin;

    // --- Топология (DrawTopology: DrawTopSurface*/DrawTopdLine*/DrawTopInputNum*).
    // А-34: контуры .tt от другого расчёта (другая сетка) поверх поля не рисуются.
    if (haveGrid && m_doc->topologyMatchesGrid() && (ts.line || ts.surface)) {
        TopologyFilter f;
        f.eps = ts.eps;
        f.metal = ts.metal;
        f.magnetic = ts.magnetic;
        f.absorber = ts.absorber;
        f.input = ts.input;
        f.plasma = ts.plasma;
        std::vector<TopQuad> quads;
        std::vector<TopPort> ports;
        visibleTopology(m_doc->boundaries(), m_xMin, m_xMax, m_yMin, m_yMax, m_delta, f, &quads, &ports);
        const float a = float(ts.blend / 100.0);
        const QColor outline = withAlpha(theme::text(), 0.5 * a + 0.1);
        // Непрозрачность заливки по материалу: металл и порты — плотно, диэлектрик,
        // плазма и поглотитель — прозрачно, чтобы поле под ними было видно.
        // Порядок — как у номеров цвета: eps, металл, магнитная стенка, вход, поглотитель, плазма.
        static const double fillAlpha[6] = {0.30, 0.92, 0.85, 0.95, 0.35, 0.30};
        for (const TopQuad &q : quads) {
            const int slot = topoSlot(q.type);
            const QColor c = withAlpha(ts.colors[slot], a * fillAlpha[slot]);
            const QColor lc = ts.surface ? outline : withAlpha(ts.colors[slot], a);
            QVector3D v[4], t[4], b[4];
            for (int k = 0; k < 4; ++k) {
                v[k] = P(q.x[k], q.y[k], 0.0);
                t[k] = QVector3D(v[k].x(), v[k].y(), ts.dim3d ? kTopZ3D : kTopZ2D);
                b[k] = QVector3D(v[k].x(), v[k].y(), -kTopZ3D);
            }
            if (!ts.dim3d) {
                if (ts.surface)
                    g.quad(t[0], t[1], t[2], t[3], c);
                if (ts.line) {
                    for (int k = 0; k < 4; ++k) {
                        const QVector3D p1(v[k].x(), v[k].y(), kTopLine2D), p2(v[(k + 1) % 4].x(), v[(k + 1) % 4].y(), kTopLine2D);
                        g.line(p1, p2, lc);
                    }
                }
            } else {
                if (ts.surface) {
                    g.quad(t[0], t[1], t[2], t[3], c);
                    g.quad(b[0], b[1], b[2], b[3], c);
                    for (int k = 0; k < 4; ++k)
                        g.quad(b[k], t[k], t[(k + 1) % 4], b[(k + 1) % 4], c);
                }
                if (ts.line) {
                    for (int k = 0; k < 4; ++k) {
                        g.line(t[k], t[(k + 1) % 4], lc);
                        g.line(b[k], b[(k + 1) % 4], lc);
                        g.line(b[k], t[k], lc);
                    }
                }
            }
        }
        if (ts.input) {
            for (const TopPort &p : ports)
                m_scene.labels.push_back({P(p.x, p.y, ts.dim3d ? kTopZ3D : kTopZ2D),
                                          QStringLiteral("Inp %1").arg(p.number), ts.colors[3], 0});
        }
        // Размеры (DrawSize2/DrawSize3).
        if (ts.sizeBlock || ts.sizeLinkList) {
            const std::vector<TopSize> sizes = visibleSizes(m_doc->boundaries(), m_xMin, m_xMax, m_yMin, m_yMax,
                                                            m_delta, ts.sizeBlock, ts.sizeLinkList);
            const double z1 = ts.dim3d ? 2.0 * kTopZ3D : kTopZ3D;
            const double z0 = zf(0.0);
            for (const TopSize &s : sizes) {
                const QVector3D a1 = P(s.x1, s.y1, z1), a2 = P(s.x2, s.y2, z1);
                double rx = a2.x() - a1.x(), ry = a2.y() - a1.y();
                double r = std::sqrt(rx * rx + ry * ry);
                if (r < FLT_MIN)
                    r = FLT_MIN;
                rx = 2 * rx * kStrelk / r;
                ry = 2 * ry * kStrelk / r;
                g.line(QVector3D(a1.x(), a1.y(), float(z0)), QVector3D(a1.x(), a1.y(), float(z1 + 3 * kStrelk)), axes);
                g.line(QVector3D(a2.x(), a2.y(), float(z0)), QVector3D(a2.x(), a2.y(), float(z1 + 3 * kStrelk)), axes);
                g.line(a1, a2, axes);
                g.triangle(a1, QVector3D(float(a1.x() + rx), float(a1.y() + ry), float(z1 + kStrelk)),
                           QVector3D(float(a1.x() + rx), float(a1.y() + ry), float(z1 - kStrelk)), axes);
                g.triangle(a2, QVector3D(float(a2.x() - rx), float(a2.y() - ry), float(z1 + kStrelk)),
                           QVector3D(float(a2.x() - rx), float(a2.y() - ry), float(z1 - kStrelk)), axes);
                m_scene.labels.push_back({(a1 + a2) / 2.0f, num(s.size / lu), axes, 1});
            }
        }
    }

    if (haveGrid) {
        const double xc = (m_xMax + m_xMin) / 2, yc = (m_yMax + m_yMin) / 2;
        const double z0 = zf(0.0);
        const auto arrowXY = [&](const QVector3D &tip, const QVector3D &l, const QVector3D &r) { g.triangle(l, tip, r, axes); };
        // --- Оси (DrawAxies).
        if (ps.drawAxes) {
            g.line(P(xc, yc, z0), P(xc, m_yMax, z0), axes);
            arrowXY(P(xc, m_yMax, z0), P(xc - 0.02 * dx, m_yMax - 0.04 * dy, z0), P(xc + 0.02 * dx, m_yMax - 0.04 * dy, z0));
            g.line(P(xc, yc, z0), P(m_xMax, yc, z0), axes);
            arrowXY(P(m_xMax, yc, z0), P(m_xMax - 0.04 * dx, yc - 0.02 * dy, z0), P(m_xMax - 0.04 * dx, yc + 0.02 * dy, z0));
            const double zt = zf(m_zMax);
            g.line(P(xc, yc, z0), P(xc, yc, zt), axes);
            arrowXY(P(xc, yc, zt), P(xc - 0.02 * dx, yc, zf((1 - 0.04) * m_zMax)), P(xc + 0.02 * dx, yc, zf((1 - 0.04) * m_zMax)));
            m_scene.labels.push_back({P(m_xMax, yc, z0), QStringLiteral("X"), axes, 0});
            m_scene.labels.push_back({P(xc, m_yMax, z0), QStringLiteral("Y"), axes, 0});
            m_scene.labels.push_back({P(xc, yc, zt), QStringLiteral("Z"), axes, 0});
        }
        // --- Размер по X (DrawAxiesSizeX).
        if (ps.drawSizeX) {
            const double yl = m_yMin - dy * 0.04;
            g.line(P(m_xMin, yl, z0), P(m_xMax, yl, z0), axes);
            arrowXY(P(m_xMax, yl, z0), P(m_xMax - 0.04 * dx, yl - 0.02 * dy, z0), P(m_xMax - 0.04 * dx, yl + 0.02 * dy, z0));
            arrowXY(P(m_xMin, yl, z0), P(m_xMin + 0.04 * dx, yl - 0.02 * dy, z0), P(m_xMin + 0.04 * dx, yl + 0.02 * dy, z0));
            g.line(P(m_xMin, m_yMin, z0), P(m_xMin, m_yMin - 2 * dy * 0.04, z0), axes);
            g.line(P(m_xMax, m_yMin, z0), P(m_xMax, m_yMin - 2 * dy * 0.04, z0), axes);
            m_scene.labels.push_back({P(xc, yl, z0), num(dx / lu) + unitSuffix(luName), axes, 1});
        }
        // --- Размер по Y (DrawAxiesSizeY).
        if (ps.drawSizeY) {
            const double xl = m_xMin - dx * 0.04;
            g.line(P(xl, m_yMin, z0), P(xl, m_yMax, z0), axes);
            arrowXY(P(xl, m_yMax, z0), P(xl - 0.02 * dx, m_yMax - 0.04 * dy, z0), P(xl + 0.02 * dx, m_yMax - 0.04 * dy, z0));
            arrowXY(P(xl, m_yMin, z0), P(xl - 0.02 * dx, m_yMin + 0.04 * dy, z0), P(xl + 0.02 * dx, m_yMin + 0.04 * dy, z0));
            g.line(P(m_xMin, m_yMin, z0), P(m_xMin - 2 * dx * 0.04, m_yMin, z0), axes);
            g.line(P(m_xMin, m_yMax, z0), P(m_xMin - 2 * dx * 0.04, m_yMax, z0), axes);
            m_scene.labels.push_back({P(xl, yc, z0), num(dy / lu) + unitSuffix(luName), axes, 2});
        }
        // --- Шкала Z поля (DrawAxiesSizeZField) и ε (DrawAxiesSizeZEps).
        // Вертикальная шкала Z у левого края окна: от -zMax до zMax, подписи пределов слоя.
        const auto zAxis = [&](double y, const QString &prefix, double lo, double hi) {
            g.line(P(m_xMin, y, zf(-m_zMax)), P(m_xMin, y, zf(m_zMax)), axes);
            arrowXY(P(m_xMin, y, zf(m_zMax)), P(m_xMin - 0.02 * dx, y, zf((1 - 0.04) * m_zMax)), P(m_xMin + 0.02 * dx, y, zf((1 - 0.04) * m_zMax)));
            arrowXY(P(m_xMin, y, zf(-m_zMax)), P(m_xMin - 0.02 * dx, y, zf(-(1 - 0.04) * m_zMax)), P(m_xMin + 0.02 * dx, y, zf(-(1 - 0.04) * m_zMax)));
            m_scene.labels.push_back({P(m_xMin, y, zf(m_zMin)), prefix + num(lo), axes, 2});
            m_scene.labels.push_back({P(m_xMin, y, zf(m_zMax)), prefix + num(hi), axes, 2});
        };
        if (m_s.field.drawZSize)
            zAxis(m_yMin, m_s.eps.drawZSize ? QStringLiteral("Field ") : QString(), m_zMin, m_zMax);
        if (m_s.eps.drawZSize) {
            // Н-35: у сеток N/Y/B подпись — их имя и единицы, а не «Eps».
            static const char *prefix[] = {"Eps ", "N, м⁻³ ", "Y ", "B, Тл "};
            zAxis(m_yMax, m_s.field.drawZSize || effectiveEpsType() != 0 ? QString::fromUtf8(prefix[effectiveEpsType()]) : QString(),
                  m_eMin, m_eMax);
        }

        // --- Значение в точке (Field → Value: DrawFieldCursore).
        m_probeOk = false;
        if (m_s.probe && m_scene.field.values) {
            double &px = m_s.probeX, &py = m_s.probeY;
            const int nX = m_scene.nX, nY = m_scene.nY;
            // С-62, А-35: метка и число — один и тот же узел. Узел сцены — ближайший к точке
            // (round, а не усечение (int)(x/Δ), бравшее соседний узел), не на краю окна (как
            // в оригинале: x от xmin + Δ до xmax − Δ); точка привязывается к этому узлу, чтобы
            // стрелки двигали её ровно на узел.
            const auto sceneNode = [](double q, int n) {
                const int lo = std::min(1, n - 1), hi = std::max(lo, n - 2);
                return std::clamp(int(std::lround(q)), lo, hi);
            };
            const int js = sceneNode((px / m_delta - m_j0) / m_strideX, nX);
            const int is = sceneNode((py / m_delta - m_i0) / m_strideY, nY);
            const int jF = m_j0 + js * m_strideX;   // узел всей сетки (CFldViewDoc::GetdZ)
            const int iF = m_i0 + is * m_strideY;
            px = jF * m_delta;
            py = iF * m_delta;
            const auto nodePos = [&](int j, int i) {
                j = std::clamp(j, 0, nX - 1);
                i = std::clamp(i, 0, nY - 1);
                const float v = (*m_scene.field.values)[size_t(i) * size_t(nX) + size_t(j)];
                const double x = m_scene.x0 + m_scene.sx * j / double(nX - 1);
                const double y = m_scene.y0 + m_scene.sy * i / double(nY - 1);
                const double z = m_s.field.dim3d && std::isfinite(v) ? zf(v) : 0.0;
                return QVector3D(float(x), float(y), float(z + 0.012));
            };
            // Метка — ромб на четырёх соседних узлах и вертикальный штрих над узлом.
            const QVector3D n1 = nodePos(js - 1, is), n2 = nodePos(js, is + 1);
            const QVector3D n3 = nodePos(js + 1, is), n4 = nodePos(js, is - 1);
            const QVector3D n5 = nodePos(js, is);
            const QColor mc = withAlpha(theme::accent3(), 0.95);
            g.triangle(n1, n2, n5, mc);
            g.triangle(n2, n3, n5, mc);
            g.triangle(n3, n4, n5, mc);
            g.triangle(n4, n1, n5, mc);
            g.line(n1, n2, theme::bg());
            g.line(n2, n3, theme::bg());
            g.line(n3, n4, theme::bg());
            g.line(n4, n1, theme::bg());
            const QVector3D top(n5.x(), n5.y(), n5.z() + 0.25f);
            g.line(n5, top, mc);
            const size_t idx = size_t(iF) * size_t(m_nX1) + size_t(jF);
            const std::vector<double> *ex = m_doc->exactValues(m_frame);
            if (ex && idx < ex->size())
                m_probeValue = (*ex)[idx];
            else if (m_current && idx < m_current->values.size())
                m_probeValue = m_current->values[idx];
            m_probeOk = true;
            m_scene.labels.push_back({top, num(m_probeValue), theme::accent3(), 0});
        }
    }
    ++m_scene.geometryVersion;
    drawLabelsSprite();
    m_view->update();
}

// --- Надписи и сведения поверх сцены --------------------------------------------

void FieldPane::drawLabelsSprite()
{
    Sprite &sp = m_sprites[0];
    const QSizeF sz(m_view->width(), m_view->height());
    if (m_scene.labels.empty() || sz.isEmpty()) {
        sp.visible = false;
        m_view->update();
        return;
    }
    const qreal dpr = m_view->devicePixelRatioF();
    QImage img = spriteImage(sz, dpr);
    QPainter p(&img);
    QFont f = m_s.place.axesFont;
    const QFontMetricsF fm(f);
    for (const SceneLabel &l : m_scene.labels) {
        QPointF at;
        if (!m_view->project(l.pos, &at))
            continue;
        const double w = fm.horizontalAdvance(l.text);
        double x = at.x() + 4;
        if (l.align == 1)
            x = at.x() - w / 2;
        else if (l.align == 2)
            x = at.x() - w - 6;
        drawHaloText(p, QPointF(x, at.y() + fm.ascent() / 2 - 1), l.text, l.color, f);
    }
    p.end();
    sp.image = img;
    sp.rect = QRectF(QPointF(0, 0), sz);
    sp.visible = true;
    ++sp.version;
    m_view->update();
}

void FieldPane::updateHud(bool infoOnly)
{
    const qreal dpr = m_view->devicePixelRatioF();
    const double W = m_view->width(), H = m_view->height();
    // --- Сведения о такте (слева сверху).
    {
        Sprite &sp = m_sprites[1];
        sp.visible = false;
        if (m_doc->hasData() && m_current) {
            const FieldHeader &h = m_current->header;
            const double tu = h.timeUnitValue > 0 ? h.timeUnitValue : 1.0;
            const double lu = h.longUnitValue > 0 ? h.longUnitValue : 1.0;
            QStringList lines;
            QString name = m_doc->fileName();
            if (m_scene.eps.visible) {
                static const char *names[] = {"ε", "N", "Y", "B"};
                name += QStringLiteral("  ·  ") + QString::fromUtf8(names[effectiveEpsType()]);
            }
            lines << QStringLiteral("nT = %1    t = %2 %3").arg(h.nT).arg(num(h.time / tu), h.timeUnit);
            QString grid = QStringLiteral("%1×%2  ·  Δ = %3 %4").arg(h.nX).arg(h.nY).arg(num(h.delta / lu), h.longUnit);
            if (m_scene.nX != m_nX1 || m_scene.nY != m_nY1)
                grid += QStringLiteral("  ·  окно %1×%2").arg(m_scene.nX).arg(m_scene.nY);
            lines << grid;
            if (m_doc->frameCount() > 1)
                lines << QStringLiteral("такт %1 из %2").arg(m_frame + 1).arg(m_doc->frameCount());
            if (!m_doc->hasTopology())
                lines << QStringLiteral("топология: ") + m_doc->topologyError();
            else if (!m_doc->topologyMatchesGrid())
                lines << QStringLiteral("топология: ") + m_doc->topologyMismatch();
            else if (!m_epsReason.isEmpty())
                lines << QStringLiteral("слой ε: ") + m_epsReason;
            if (m_current->stats.nonFinite > 0)
                lines << QStringLiteral("значения: %1 узл. nan/inf — в шкалу не входят").arg(m_current->stats.nonFinite);
            if (m_strideX > 1 || m_strideY > 1) {
                QStringList ax;
                if (m_strideX > 1)
                    ax << QStringLiteral("каждый %1-й узел по X").arg(m_strideX);
                if (m_strideY > 1)
                    ax << QStringLiteral("каждый %1-й узел по Y").arg(m_strideY);
                lines << QStringLiteral("значения: показан ") + ax.join(QStringLiteral(", "))
                             + QStringLiteral(" (окно велико, уменьшите его для всех узлов)");
            }
            // С-59: последнее чтение файла не удалось — показан прежний кадр, причина видна.
            if (!m_doc->error().isEmpty())
                lines << QStringLiteral("ошибка чтения: ") + m_doc->error() + QStringLiteral(" — показан последний прочитанный кадр");
            const QFont fTitle = theme::uiFont(14, QFont::DemiBold);
            const QFont fMono = theme::monoFont(12);
            const QFontMetricsF mt(fTitle), mm(fMono);
            double w = mt.horizontalAdvance(name);
            for (const QString &s : lines)
                w = std::max(w, mm.horizontalAdvance(s));
            const QSizeF sz(w + 28, mt.height() + lines.size() * (mm.height() + 2) + 20);
            QImage img = spriteImage(sz, dpr);
            QPainter p(&img);
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(QPen(theme::line(), 1));
            p.setBrush(withAlpha(theme::surface(), 0.82));
            p.drawRoundedRect(QRectF(0.5, 0.5, sz.width() - 1, sz.height() - 1), 10, 10);
            p.setFont(fTitle);
            p.setPen(theme::text());
            double y = 10 + mt.ascent();
            p.drawText(QPointF(14, y), name);
            y += mt.descent() + 4;
            p.setFont(fMono);
            for (int i = 0; i < lines.size(); ++i) {
                const bool tact = lines[i].startsWith(QStringLiteral("такт"));
                const bool note = lines[i].startsWith(QStringLiteral("топология")) || lines[i].startsWith(QStringLiteral("слой"))
                                  || lines[i].startsWith(QStringLiteral("значения")) || lines[i].startsWith(QStringLiteral("ошибка"));
                p.setPen(i == 0 ? theme::textDim() : tact ? theme::accent() : note ? theme::warn() : theme::textMute());
                p.drawText(QPointF(14, y + mm.ascent()), lines[i]);
                y += mm.height() + 2;
            }
            p.end();
            sp.image = img;
            sp.rect = QRectF(QPointF(12, 10), sz);
            sp.visible = true;
            ++sp.version;
        }
    }
    if (infoOnly) {
        m_view->update();
        return;
    }
    // --- Шкалы (справа).
    // Шкала справа: полоса цветов (сверху — максимум), подписи max, середины и min.
    const auto scaleSprite = [&](Sprite &sp, bool show, ScaleKind kind, const Colors20 &custom, int levels,
                                 bool modulus, double lo, double hi, const QString &unit, double rightEdge) {
        sp.visible = false;
        if (!show)
            return 0.0;
        const QFont fl = theme::monoFont(11);
        const QFont fu = theme::uiFont(12, QFont::DemiBold);
        const QFontMetricsF ml(fl), mu(fu);
        const double vmin = modulus ? 0.0 : lo, vmax = hi;
        const QString sMax = num(vmax), sMid = num((vmax + vmin) / 2), sMin = num(vmin);
        const double tw = std::max({ml.horizontalAdvance(sMax), ml.horizontalAdvance(sMid), ml.horizontalAdvance(sMin),
                                    mu.horizontalAdvance(unit)});
        const double barH = std::clamp(H * 0.55, 120.0, 360.0);
        const QSizeF sz(tw + 14 + 16 + 20, barH + mu.height() + 30);
        QImage img = spriteImage(sz, dpr);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(theme::line(), 1));
        p.setBrush(withAlpha(theme::surface(), 0.82));
        p.drawRoundedRect(QRectF(0.5, 0.5, sz.width() - 1, sz.height() - 1), 10, 10);
        p.setFont(fu);
        p.setPen(theme::textDim());
        p.drawText(QRectF(8, 6, sz.width() - 16, mu.height()), Qt::AlignRight | Qt::AlignVCenter, unit);
        const QRectF bar(sz.width() - 10 - 14, 10 + mu.height() + 6, 14, barH);
        const int steps = int(std::ceil(bar.height() * dpr));
        p.setPen(Qt::NoPen);
        for (int k = 0; k < steps; ++k) {
            double t = 1.0 - (k + 0.5) / steps;   // сверху — максимум
            if (levels > 1)
                t = std::min(std::floor(t * (levels - 1)), double(levels - 1)) / (levels - 1);
            p.setBrush(scaleColor(kind, t, custom));
            p.drawRect(QRectF(bar.left(), bar.top() + k / dpr, bar.width(), 1.0 / dpr + 0.01));
        }
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(theme::line(), 1));
        p.drawRoundedRect(bar.adjusted(-0.5, -0.5, 0.5, 0.5), 3, 3);
        p.setFont(fl);
        const double lx = bar.left() - 8 - tw;
        p.setPen(theme::textDim());
        p.drawText(QRectF(lx, bar.top() - ml.height() / 2, tw, ml.height()), Qt::AlignRight | Qt::AlignVCenter, sMax);
        p.setPen(theme::textMute());
        p.drawText(QRectF(lx, bar.center().y() - ml.height() / 2, tw, ml.height()), Qt::AlignRight | Qt::AlignVCenter, sMid);
        p.setPen(theme::textDim());
        p.drawText(QRectF(lx, bar.bottom() - ml.height() / 2, tw, ml.height()), Qt::AlignRight | Qt::AlignVCenter, sMin);
        p.end();
        sp.image = img;
        sp.rect = QRectF(QPointF(rightEdge - sz.width(), (H - sz.height()) / 2), sz);
        sp.visible = true;
        ++sp.version;
        return sz.width();
    };
    const bool fieldShown = m_scene.field.visible && m_current;
    const double w1 = scaleSprite(m_sprites[2], fieldShown, m_fieldScale, m_s.field.custom, m_scene.field.levels,
                                  m_s.field.modulus, m_zMin, m_zMax,
                                  m_s.field.modulus ? QStringLiteral("|E|") : QStringLiteral("E"), W - 12);
    static const char *epsUnits[] = {"ε", "N, м⁻³", "Y", "B, Тл"};
    scaleSprite(m_sprites[3], m_scene.eps.visible, m_epsScale, m_s.eps.custom, m_scene.eps.levels, false, m_eMin,
                m_eMax, QString::fromUtf8(epsUnits[effectiveEpsType()]), W - 12 - (w1 > 0 ? w1 + 8 : 0));

    // --- Ошибка (как OnDraw: «Error : …»).
    {
        Sprite &sp = m_sprites[4];
        sp.visible = false;
        QString msg, hint;
        if (!m_doc->hasData()) {
            const QFileInfo fi(m_doc->path());
            const QString where = QStringLiteral("%1\nв папке %2").arg(fi.fileName(), QDir::toNativeSeparators(fi.absolutePath()));
            if (!fi.exists()) {
                msg = QStringLiteral("Файл поля ещё не создан");
                hint = where;
            } else {
                msg = QStringLiteral("Ошибка чтения файла поля");
                hint = (m_doc->error().isEmpty() ? QString() : QStringLiteral("Error : ") + m_doc->error() + QLatin1Char('\n')) + where;
            }
            if (m_doc->syncEnabled())
                hint += QStringLiteral("\n\nFieldView следит за файлом и покажет поле, как только ядро его запишет.");
        } else if (!m_current) {
            // С-59: такт серии не прочитан (файл .frames изменён или повреждён).
            msg = QStringLiteral("Ошибка чтения такта %1").arg(m_frame + 1);
            hint = QStringLiteral("Error : ") + m_doc->error();
        } else if (!m_windowError.isEmpty()) {
            msg = QStringLiteral("Ошибка: ") + m_windowError;
            hint = QStringLiteral("Уменьшите масштаб (View → Zoom −) или нажмите «Вид по умолчанию» (Ctrl+D).");
        } else if (!m_s.field.surface && !m_s.field.line && !m_scene.eps.visible && !(m_doc->hasTopology() && (m_s.topo.line || m_s.topo.surface))) {
            msg = QStringLiteral("Все слои выключены");
            hint = QStringLiteral("Включите слой поля: View → Field → Surface (S) или Line (L).");
        }
        if (!msg.isEmpty() && W > 0 && H > 0) {
            const QFont f1 = theme::uiFont(18, QFont::Bold), f2 = theme::uiFont(13);
            const double w = std::min(W - 40, 640.0);
            const QFontMetricsF m1(f1), m2(f2);
            const QRectF r2 = m2.boundingRect(QRectF(0, 0, w - 40, 1000), Qt::TextWordWrap | Qt::AlignHCenter, hint);
            const QSizeF sz(w, m1.height() + r2.height() + 44);
            QImage img = spriteImage(sz, dpr);
            QPainter p(&img);
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(QPen(theme::line(), 1));
            p.setBrush(withAlpha(theme::surface(), 0.92));
            p.drawRoundedRect(QRectF(0.5, 0.5, sz.width() - 1, sz.height() - 1), 14, 14);
            p.setFont(f1);
            p.setPen(m_doc->error().isEmpty() && m_windowError.isEmpty() && m_current ? theme::text() : theme::warn());
            p.drawText(QRectF(20, 16, w - 40, m1.height()), Qt::AlignHCenter, msg);
            p.setFont(f2);
            p.setPen(theme::textDim());
            p.drawText(QRectF(20, 22 + m1.height(), w - 40, r2.height() + 4), Qt::TextWordWrap | Qt::AlignHCenter, hint);
            p.end();
            sp.image = img;
            sp.rect = QRectF(QPointF((W - sz.width()) / 2, (H - sz.height()) / 2), sz);
            sp.visible = true;
            ++sp.version;
        }
    }
    m_view->update();
}

void FieldPane::updateStatus()
{
    // PutTrace1 / PutStatistics1.
    const FieldHeader *h = m_current ? &m_current->header : (m_doc->hasData() ? &m_doc->header(0) : nullptr);
    if (!h) {
        m_trace = m_doc->error().isEmpty() ? QStringLiteral("Нет данных") : QStringLiteral("Error : ") + m_doc->error();
        m_stats.clear();
        Q_EMIT statusChanged();
        return;
    }
    const double lu = h->longUnitValue > 0 ? h->longUnitValue : 1.0;
    const double tu = h->timeUnitValue > 0 ? h->timeUnitValue : 1.0;
    const QString L = h->longUnit;
    if (!m_doc->error().isEmpty()) {
        // С-59: ошибка чтения при показанном прежнем кадре — в строке состояния.
        m_trace = QStringLiteral("Error : ") + m_doc->error() + QStringLiteral(" (показан последний прочитанный кадр)");
    } else if (m_s.probe && m_probeOk) {
        m_trace = QStringLiteral("X = %1 %2; Y = %3 %2; Field = %4;")
                      .arg(num((m_s.probeX + h->xMin) / lu), L, num((m_s.probeY + h->yMin) / lu), num(m_probeValue));
    } else {
        m_trace = QStringLiteral("Xmin = %1 %2; Xmax = %3 %2; Ymin = %4 %2; Ymax = %5 %2; Delta = %6 %2;")
                      .arg(num((m_xMin + h->xMin) / lu), L, num((m_xMax + h->xMin) / lu), num((m_yMin + h->yMin) / lu),
                           num((m_yMax + h->yMin) / lu), num(h->delta / lu));
    }
    m_stats = QStringLiteral("nT = %1 ; t = %2 %3;").arg(h->nT).arg(num(h->time / tu), h->timeUnit);
    Q_EMIT statusChanged();
}

std::vector<LevelInfo> FieldPane::fieldLevels() const
{
    // «Value for color level»: значение, с которого начинается каждый из 20 уровней цвета.
    std::vector<LevelInfo> out;
    for (int i = 0; i < 20; ++i) {
        const double t = i / 19.0;
        LevelInfo l;
        l.color = scaleColor(m_fieldScale, t, m_s.field.custom);
        l.value = m_s.field.modulus ? t * m_zMax : m_zMin + t * (m_zMax - m_zMin);
        out.push_back(l);
    }
    return out;
}

// --- Команды -------------------------------------------------------------------

void FieldPane::settingsChanged(bool geometryToo)
{
    CameraParams c = m_view->camera();
    c.angleX = m_s.camera.angleX;
    c.angleY = m_s.camera.angleY;
    c.angleZ = m_s.camera.angleZ;
    c.transX = m_s.camera.transX;
    c.transY = m_s.camera.transY;
    c.transZ = m_s.camera.transZ;
    c.zoom = m_s.camera.zoom;
    m_view->setCamera(c);
    if (geometryToo) {
        updateWindow();
        updateFieldValues();
        updateEps();
        updateGeometry();
        updateHud();
    } else {
        drawLabelsSprite();
    }
    updateStatus();
    m_s.save();   // WriteIniFile
    Q_EMIT stateChanged();
}

void FieldPane::applySettings(const ViewSettings &s)
{
    const bool zoneChanged = s.place.xAuto != m_s.place.xAuto || s.place.yAuto != m_s.place.yAuto
                             || s.place.xMin != m_s.place.xMin || s.place.xMax != m_s.place.xMax
                             || s.place.yMin != m_s.place.yMin || s.place.yMax != m_s.place.yMax
                             || s.field.modulus != m_s.field.modulus || s.field.zAuto != m_s.field.zAuto
                             || s.field.scale != m_s.field.scale;
    m_s = s;
    if (zoneChanged)
        m_accValid = false;
    m_playTimer.setInterval(std::max(10, 1000 / std::max(1, m_s.speed)));
    settingsChanged(true);
}

void FieldPane::toggle(bool &flag)
{
    flag = !flag;
    if (&flag == &m_s.field.modulus)
        m_accValid = false;
    if (&flag == &m_s.probe && flag
        && (m_s.probeX <= m_xMin + m_delta || m_s.probeX >= m_xMax - m_delta || m_s.probeY <= m_yMin + m_delta
            || m_s.probeY >= m_yMax - m_delta)) {
        // Точка вне окна данных — ставим её в середину.
        m_s.probeX = (m_xMin + m_xMax) / 2;
        m_s.probeY = (m_yMin + m_yMax) / 2;
    }
    settingsChanged(true);
}

void FieldPane::setEpsType(int t)
{
    // OnEpsTypeEps/N/Y/B: выбор величины и включение заливки слоя ε.
    m_s.eps.type = std::clamp(t, 0, 3);
    m_s.eps.surface = true;
    m_s.eps.zAuto = true;
    settingsChanged(true);
}

void FieldPane::zoom(int kind)
{
    // OnViewZoomP / M / Px / Mx / Py / My: масштаб — это окно данных.
    if (!m_doc->hasData())
        return;
    PlaceSettings &p = m_s.place;
    const double d = m_delta;
    const double nXd = m_nX1 * d, nYd = m_nY1 * d;
    const bool doX = kind == 0 || kind == 1 || kind == 3 || kind == 4;
    const bool doY = kind == 0 || kind == 2 || kind == 3 || kind == 5;
    const bool in = kind <= 2;
    if (doX) {
        p.xAuto = false;
        const double x0 = (p.xMax + p.xMin) / 2;
        double x1;
        if (in) {
            x1 = (p.xMax - p.xMin) / 4;
            if (x1 < 2 * d)
                x1 = d;
        } else {
            x1 = p.xMax - p.xMin;
            if (2 * x1 > nXd)
                p.xAuto = true;
        }
        p.xMin = x0 - x1;
        p.xMax = x0 + x1;
    }
    if (doY) {
        p.yAuto = false;
        const double y0 = (p.yMax + p.yMin) / 2;
        double y1;
        if (in) {
            y1 = (p.yMax - p.yMin) / 4;
            if (y1 < 2 * d)
                y1 = d;
        } else {
            y1 = p.yMax - p.yMin;
            if (2 * y1 > nYd)
                p.yAuto = true;
        }
        p.yMin = y0 - y1;
        p.yMax = y0 + y1;
    }
    settingsChanged(true);
}

void FieldPane::rotate(int axis, double deg)
{
    double &a = axis == 0 ? m_s.camera.angleX : axis == 1 ? m_s.camera.angleY : m_s.camera.angleZ;
    a += deg;
    settingsChanged(false);
}

void FieldPane::translate(int axis, double d)
{
    double &t = axis == 0 ? m_s.camera.transX : axis == 1 ? m_s.camera.transY : m_s.camera.transZ;
    t += d;
    settingsChanged(false);
}

void FieldPane::arrow(int dx, int dy)
{
    // OnStrelkleft/right/up/down: в режиме Value двигается точка, иначе — окно данных.
    if (!m_doc->hasData())
        return;
    const double d = m_delta;
    PlaceSettings &p = m_s.place;
    if (m_s.probe) {
        m_s.probeX += dx * d;
        m_s.probeY += dy * d;
        settingsChanged(true);
        return;
    }
    if (dx < 0) {
        if (p.xMin - d < 0.0) {
            p.xMin = 0.0;
        } else {
            p.xMin -= d;
            p.xMax -= d;
        }
        p.xAuto = false;
    } else if (dx > 0) {
        if (d * (m_nX1 - 2) < p.xMax) {
            p.xMax = d * (m_nX1 - 1);
        } else {
            p.xMin += d;
            p.xMax += d;
        }
        p.xAuto = false;
    }
    if (dy < 0) {
        if (p.yMin - d < 0.0) {
            p.yMin = 0.0;
        } else {
            p.yMin -= d;
            p.yMax -= d;
        }
        p.yAuto = false;
    } else if (dy > 0) {
        if (d * (m_nY1 - 2) < p.yMax) {
            p.yMax = d * (m_nY1 - 1);
        } else {
            p.yMin += d;
            p.yMax += d;
        }
        p.yAuto = false;
    }
    settingsChanged(true);
}

void FieldPane::setDefault()
{
    // OnViewDefault → SetDefaultValue. Цвета шкал, материалов и величина слоя ε сохраняются
    // (SetDefaultValue оригинала их тоже не трогает).
    ViewSettings d = ViewSettings::defaults();
    d.field.custom = m_s.field.custom;
    d.eps.custom = m_s.eps.custom;
    d.eps.type = m_s.eps.type;
    for (int i = 0; i < 6; ++i)
        d.topo.colors[i] = m_s.topo.colors[i];
    d.speed = m_s.speed;
    d.propertyPage = m_s.propertyPage;
    m_s = d;
    m_accValid = false;
    CameraParams c = m_view->camera();
    c.panX = c.panY = 0;
    m_view->setCamera(c);
    settingsChanged(true);
}

void FieldPane::resetView()
{
    const CameraSettings d;
    m_s.camera = d;
    CameraParams c = m_view->camera();
    c.panX = c.panY = 0;
    m_view->setCamera(c);
    settingsChanged(false);
}

void FieldPane::resetZRange()
{
    // OnLButtonDblClk: dZmax = −FLT_MAX, dZmin = FLT_MAX, bZsizeFlag = TRUE.
    if (m_s.field.line || m_s.field.surface)
        m_s.field.zAuto = true;
    if (m_s.eps.line || m_s.eps.surface)
        m_s.eps.zAuto = true;
    if (!m_windowError.isEmpty()) {
        m_s.place.xAuto = true;
        m_s.place.yAuto = true;
    }
    m_accValid = false;
}

void FieldPane::reread()
{
    m_doc->reload();
    settingsChanged(true);
}

QImage FieldPane::grabImage()
{
    return m_view->grabFramebuffer();
}

} // namespace fieldview
