// Окно «Параметры» — описание в parametersdialog.h.
/// @file parametersdialog.cpp
/// @brief Окно «Параметры вида»: построение вкладок, перенос значений между полями и настройками.
///
/// Часть модуля FieldView (слой `ui`). Описание класса — в `parametersdialog.h`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#dialogs
#include "parametersdialog.h"
#include "autocolordialog.h"
#include "theme.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QDoubleValidator>
#include <QFontDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

namespace fieldview {

namespace {

/// @brief Создаёт поле ввода числа (локаль «C», допускается порядок «e»).
/// @return Поле ввода (владелец — тот, кто вставит его в раскладку).
QLineEdit *numEdit()
{
    auto *e = new QLineEdit;
    auto *v = new QDoubleValidator(e);
    v->setLocale(QLocale::c());
    v->setNotation(QDoubleValidator::ScientificNotation);
    e->setValidator(v);
    e->setMinimumWidth(110);
    return e;
}

/// @brief Пишет число в поле (8 значащих цифр).
/// @param[in,out] e  поле
/// @param[in]     v  число
void setNum(QLineEdit *e, double v)
{
    e->setText(QString::number(v, 'g', 8));
}

/// @brief Читает число из поля (локаль «C»).
/// @param[in] e         поле
/// @param[in] fallback  значение, если текст не число
/// @return Число или `fallback`.
double getNum(QLineEdit *e, double fallback)
{
    bool ok = false;
    const double v = QLocale::c().toDouble(e->text().trimmed(), &ok);
    return ok ? v : fallback;
}

/// @brief Создаёт поле-счётчик вещественного числа.
/// @param[in] lo    наименьшее значение
/// @param[in] hi    наибольшее значение
/// @param[in] step  шаг стрелок
/// @param[in] dec   знаков после запятой
/// @return Поле-счётчик.
QDoubleSpinBox *spin(double lo, double hi, double step, int dec)
{
    auto *s = new QDoubleSpinBox;
    s->setRange(lo, hi);
    s->setSingleStep(step);
    s->setDecimals(dec);
    s->setLocale(QLocale::c());
    return s;
}

/// @brief Рисует образец цвета 26 x 16 точек.
/// @param[in] c  цвет
/// @return Картинка образца.
QPixmap swatch(const QColor &c)
{
    QPixmap pm(26, 16);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(theme::line(), 1));
    p.setBrush(c);
    p.drawRoundedRect(QRectF(0.5, 0.5, 25, 15), 4, 4);
    return pm;
}

/// @brief Рисует образец шкалы слева направо (от минимума к максимуму).
/// @param[in] k       шкала (не `Auto`)
/// @param[in] custom  20 цветов для `Custom`
/// @param[in] smooth  `false` — 20 ступеней
/// @param[in] w       ширина, точек
/// @param[in] h       высота, точек
/// @return Картинка образца.
QPixmap scalePreview(ScaleKind k, const Colors20 &custom, bool smooth, int w = 220, int h = 18)
{
    QPixmap pm(w, h);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    for (int x = 0; x < w; ++x) {
        double t = double(x) / (w - 1);
        if (!smooth)
            t = std::min(std::floor(t * 19.0), 19.0) / 19.0;
        p.fillRect(QRectF(x, 0, 1, h), scaleColor(k, t, custom));
    }
    p.setPen(QPen(theme::line(), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(QRectF(0.5, 0.5, w - 1, h - 1));
    return pm;
}

/// Подписи типов блоков на вкладке «Топология» (порядок — как у цветов материалов).
const char *kTopoNames[6] = {"Диэлектрик (Eps)", "Металл (Metal)", "Магнитная стенка (Magnetic)",
                             "Вход (Input)", "Поглотитель (Absorber)", "Плазма (новое)"};

} // namespace

ParametersDialog::ParametersDialog(const ViewSettings &s, const ParametersContext &ctx,
                                   std::function<void(const ViewSettings &)> apply, QWidget *parent)
    : QDialog(parent), m_s(s), m_ctx(ctx), m_apply(std::move(apply))
{
    setWindowTitle(QStringLiteral("Параметры вида"));
    auto *root = new QVBoxLayout(this);
    m_tabs = new QTabWidget;
    m_tabs->addTab(buildPlace(), QStringLiteral("Размещение"));
    m_tabs->addTab(buildTopology(), QStringLiteral("Топология"));
    m_tabs->addTab(buildLayer(false), QStringLiteral("ε и плазма"));
    m_tabs->addTab(buildLayer(true), QStringLiteral("Поле"));
    m_tabs->setCurrentIndex(std::clamp(s.propertyPage, 0, 3));
    root->addWidget(m_tabs);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    bb->button(QDialogButtonBox::Ok)->setText(QStringLiteral("ОК"));
    bb->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Отмена"));
    bb->button(QDialogButtonBox::Apply)->setText(QStringLiteral("Применить"));
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(bb->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, [this]() {
        if (m_apply)
            m_apply(result());
    });
    root->addWidget(bb);
    load();
    updateEnables();
}

QWidget *ParametersDialog::buildPlace()
{
    auto *w = new QWidget;
    auto *g = new QGridLayout(w);
    const auto axisBox = [&](const QString &title, QCheckBox *&autoBox, QLineEdit *&mn, QLineEdit *&mx,
                             QDoubleSpinBox *&ang, QDoubleSpinBox *&tr, QCheckBox *&sizeFlag) {
        auto *box = new QGroupBox(title);
        auto *l = new QGridLayout(box);
        autoBox = new QCheckBox(QStringLiteral("Вся область (Auto size)"));
        mn = numEdit();
        mx = numEdit();
        ang = spin(-3600, 3600, 2, 2);
        tr = spin(-100, 100, 0.04, 3);
        sizeFlag = new QCheckBox(QStringLiteral("Размер вдоль оси (Draw size flag)"));
        l->addWidget(autoBox, 0, 0, 1, 2);
        l->addWidget(new QLabel(QStringLiteral("min, %1").arg(m_ctx.longUnit)), 1, 0);
        l->addWidget(mn, 1, 1);
        l->addWidget(new QLabel(QStringLiteral("max, %1").arg(m_ctx.longUnit)), 2, 0);
        l->addWidget(mx, 2, 1);
        l->addWidget(new QLabel(QStringLiteral("поворот, °")), 3, 0);
        l->addWidget(ang, 3, 1);
        l->addWidget(new QLabel(QStringLiteral("сдвиг")), 4, 0);
        l->addWidget(tr, 4, 1);
        l->addWidget(sizeFlag, 5, 0, 1, 2);
        connect(autoBox, &QCheckBox::toggled, this, &ParametersDialog::updateEnables);
        return box;
    };
    g->addWidget(axisBox(QStringLiteral("Ось X"), m_xAuto, m_xMin, m_xMax, m_angX, m_trX, m_sizeX), 0, 0);
    g->addWidget(axisBox(QStringLiteral("Ось Y"), m_yAuto, m_yMin, m_yMax, m_angY, m_trY, m_sizeY), 0, 1);

    auto *right = new QVBoxLayout;
    auto *axes = new QGroupBox(QStringLiteral("Оси и надписи"));
    auto *al = new QGridLayout(axes);
    m_drawAxes = new QCheckBox(QStringLiteral("Рисовать оси X, Y, Z (Draw)"));
    m_axesColor = new QPushButton(QStringLiteral("Цвет…"));
    m_axesFont = new QPushButton(QStringLiteral("Шрифт…"));
    m_axesSample = new QLabel;
    al->addWidget(m_drawAxes, 0, 0, 1, 3);
    al->addWidget(m_axesSample, 1, 0);
    al->addWidget(m_axesColor, 1, 1);
    al->addWidget(m_axesFont, 1, 2);
    connect(m_axesColor, &QPushButton::clicked, this, [this]() {
        const QColor c = QColorDialog::getColor(m_s.place.axesColor, this, QStringLiteral("Цвет надписей"));
        if (c.isValid()) {
            m_s.place.axesColor = c;
            refreshSwatches();
        }
    });
    connect(m_axesFont, &QPushButton::clicked, this, [this]() {
        bool ok = false;
        const QFont f = QFontDialog::getFont(&ok, m_s.place.axesFont, this, QStringLiteral("Шрифт надписей"));
        if (ok) {
            m_s.place.axesFont = f;
            refreshSwatches();
        }
    });
    right->addWidget(axes);

    auto *cam = new QGroupBox(QStringLiteral("Камера"));
    auto *cl = new QGridLayout(cam);
    m_angZ = spin(-3600, 3600, 2, 2);
    m_trZ = spin(-100, 100, 0.04, 3);
    m_zoom = spin(0.05, 50, 0.1, 3);
    cl->addWidget(new QLabel(QStringLiteral("поворот Z, °")), 0, 0);
    cl->addWidget(m_angZ, 0, 1);
    cl->addWidget(new QLabel(QStringLiteral("сдвиг Z (ближе)")), 1, 0);
    cl->addWidget(m_trZ, 1, 1);
    cl->addWidget(new QLabel(QStringLiteral("масштаб")), 2, 0);
    cl->addWidget(m_zoom, 2, 1);
    right->addWidget(cam);

    auto *units = new QGroupBox(QStringLiteral("Единицы файла"));
    auto *ul = new QGridLayout(units);
    auto *lu = new QLineEdit(QStringLiteral("%1 (%2 м)").arg(m_ctx.longUnit).arg(m_ctx.longUnitValue));
    auto *tu = new QLineEdit(m_ctx.timeUnit);
    lu->setReadOnly(true);
    tu->setReadOnly(true);
    ul->addWidget(new QLabel(QStringLiteral("Длина (Long Unit)")), 0, 0);
    ul->addWidget(lu, 0, 1);
    ul->addWidget(new QLabel(QStringLiteral("Время (Time Unit)")), 1, 0);
    ul->addWidget(tu, 1, 1);
    right->addWidget(units);
    m_prop = new QCheckBox(QStringLiteral("Пропорционально X и Y (Proportionally)"));
    right->addWidget(m_prop);
    right->addStretch(1);
    g->addLayout(right, 0, 2);
    return w;
}

QWidget *ParametersDialog::buildTopology()
{
    auto *w = new QWidget;
    auto *g = new QGridLayout(w);
    auto *flags = new QGroupBox(QStringLiteral("Что показывать и цвета (Draw flags)"));
    auto *fl = new QGridLayout(flags);
    for (int i = 0; i < 6; ++i) {
        m_tFlag[i] = new QCheckBox(QString::fromUtf8(kTopoNames[i]));
        m_tSwatch[i] = new QLabel;
        auto *chg = new QPushButton(QStringLiteral("Изменить…"));
        fl->addWidget(m_tFlag[i], i, 0);
        fl->addWidget(m_tSwatch[i], i, 1);
        fl->addWidget(chg, i, 2);
        connect(chg, &QPushButton::clicked, this, [this, i]() {
            const QColor c = QColorDialog::getColor(m_s.topo.colors[i], this, QString::fromUtf8(kTopoNames[i]));
            if (c.isValid()) {
                m_s.topo.colors[i] = c;
                refreshSwatches();
            }
        });
    }
    auto *def = new QPushButton(QStringLiteral("Цвета по умолчанию (Default)"));
    fl->addWidget(def, 6, 0, 1, 3);
    connect(def, &QPushButton::clicked, this, [this]() {
        for (int i = 0; i < 6; ++i)
            m_s.topo.colors[i] = defaultTopologyColor(i);
        refreshSwatches();
    });
    g->addWidget(flags, 0, 0, 3, 1);

    auto *draw = new QGroupBox(QStringLiteral("Рисовать (Draw)"));
    auto *dl = new QVBoxLayout(draw);
    m_tLine = new QCheckBox(QStringLiteral("Контуры (Line)"));
    m_tSurf = new QCheckBox(QStringLiteral("Заливка (Surface)"));
    dl->addWidget(m_tLine);
    dl->addWidget(m_tSurf);
    g->addWidget(draw, 0, 1);
    auto *dim = new QGroupBox(QStringLiteral("Вид (Dimention)"));
    auto *dml = new QVBoxLayout(dim);
    m_t2d = new QRadioButton(QStringLiteral("2D — плоско"));
    m_t3d = new QRadioButton(QStringLiteral("3D — объёмные блоки"));
    dml->addWidget(m_t2d);
    dml->addWidget(m_t3d);
    g->addWidget(dim, 1, 1);
    auto *size = new QGroupBox(QStringLiteral("Размеры (Size)"));
    auto *sl = new QGridLayout(size);
    m_tSizeBlock = new QCheckBox(QStringLiteral("Блоков (Block)"));
    m_tSizeLink = new QCheckBox(QStringLiteral("Привязки (Link List)"));
    m_tBlend = new QSpinBox;
    m_tBlend->setRange(0, 100);
    sl->addWidget(m_tSizeBlock, 0, 0, 1, 2);
    sl->addWidget(m_tSizeLink, 1, 0, 1, 2);
    sl->addWidget(new QLabel(QStringLiteral("Непрозрачность, % (Blend)")), 2, 0);
    sl->addWidget(m_tBlend, 2, 1);
    g->addWidget(size, 2, 1);
    return w;
}

QWidget *ParametersDialog::buildLayer(bool field)
{
    LayerUi &u = m_l[field ? 1 : 0];
    auto *w = new QWidget;
    auto *g = new QGridLayout(w);
    auto *draw = new QGroupBox(QStringLiteral("Рисовать (Draw flags)"));
    auto *dl = new QGridLayout(draw);
    u.surface = new QCheckBox(QStringLiteral("Заливка (Surface)"));
    u.line = new QCheckBox(QStringLiteral("Сетка (Line)"));
    u.d2 = new QRadioButton(QStringLiteral("2D — плоско"));
    u.d3 = new QRadioButton(field ? QStringLiteral("3D — рельеф") : QStringLiteral("3D — рельеф ε"));
    auto *dimGroup = new QButtonGroup(w);
    dimGroup->addButton(u.d2);
    dimGroup->addButton(u.d3);
    u.blend = new QSpinBox;
    u.blend->setRange(0, 100);
    dl->addWidget(u.surface, 0, 0, 1, 2);
    dl->addWidget(u.line, 1, 0, 1, 2);
    dl->addWidget(u.d2, 2, 0, 1, 2);
    dl->addWidget(u.d3, 3, 0, 1, 2);
    dl->addWidget(new QLabel(QStringLiteral("Непрозрачность, %")), 4, 0);
    dl->addWidget(u.blend, 4, 1);
    if (field) {
        u.modulus = new QCheckBox(QStringLiteral("Модуль поля (Draw Modul)"));
        u.height = spin(0.05, 10, 0.1, 2);
        dl->addWidget(u.modulus, 5, 0, 1, 2);
        dl->addWidget(new QLabel(QStringLiteral("Высота рельефа")), 6, 0);
        dl->addWidget(u.height, 6, 1);
    }
    g->addWidget(draw, 0, 0, 2, 1);

    auto *col = new QGroupBox(QStringLiteral("Цвет (Color)"));
    auto *cl = new QGridLayout(col);
    u.scale = new QComboBox;
    u.scale->addItem(field ? QStringLiteral("Авто: «Волна» или «Свечение»") : QStringLiteral("Авто («Свечение»)"), int(ScaleKind::Auto));
    u.scale->addItem(QStringLiteral("«Волна» (−max … 0 … +max)"), int(ScaleKind::Wave));
    u.scale->addItem(QStringLiteral("«Свечение» (0 … max)"), int(ScaleKind::Glow));
    u.scale->addItem(QStringLiteral("20 цветов уровней (свои)"), int(ScaleKind::Custom));
    u.smooth = new QCheckBox(QStringLiteral("Плавно, 256 уровней (иначе 20)"));
    u.preview = new QLabel;
    auto *levels = new QPushButton(QStringLiteral("Цвета уровней… (Set color)"));
    auto *def = new QPushButton(QStringLiteral("По умолчанию (Default)"));
    cl->addWidget(u.scale, 0, 0, 1, 2);
    cl->addWidget(u.preview, 1, 0, 1, 2);
    cl->addWidget(u.smooth, 2, 0, 1, 2);
    cl->addWidget(levels, 3, 0);
    cl->addWidget(def, 3, 1);
    connect(u.scale, &QComboBox::currentIndexChanged, this, &ParametersDialog::refreshSwatches);
    connect(u.smooth, &QCheckBox::toggled, this, &ParametersDialog::refreshSwatches);
    connect(levels, &QPushButton::clicked, this, [this, field]() {
        LayerSettings &ls = field ? static_cast<LayerSettings &>(m_s.field) : static_cast<LayerSettings &>(m_s.eps);
        AutoColorDialog dlg(ls.custom, field ? classicFieldColors() : classicEpsColors(), this);
        if (dlg.exec() == QDialog::Accepted) {
            ls.custom = dlg.colors();
            LayerUi &uu = m_l[field ? 1 : 0];
            uu.scale->setCurrentIndex(uu.scale->findData(int(ScaleKind::Custom)));
            refreshSwatches();
        }
    });
    connect(def, &QPushButton::clicked, this, [this, field]() {
        LayerSettings &ls = field ? static_cast<LayerSettings &>(m_s.field) : static_cast<LayerSettings &>(m_s.eps);
        ls.custom = field ? classicFieldColors() : classicEpsColors();
        LayerUi &uu = m_l[field ? 1 : 0];
        uu.scale->setCurrentIndex(uu.scale->findData(int(field ? ScaleKind::Auto : ScaleKind::Glow)));
        uu.smooth->setChecked(true);
        refreshSwatches();
    });
    g->addWidget(col, 0, 1);

    auto *z = new QGroupBox(QStringLiteral("Пределы по оси Z (Z axies)"));
    auto *zl = new QGridLayout(z);
    u.zAuto = new QCheckBox(field ? QStringLiteral("Авто (Auto size)") : QStringLiteral("Авто — по сетке (Auto size)"));
    u.zMin = numEdit();
    u.zMax = numEdit();
    u.zSize = new QCheckBox(QStringLiteral("Шкала Z в сцене (Draw size flag)"));
    zl->addWidget(u.zAuto, 0, 0, 1, 2);
    zl->addWidget(new QLabel(QStringLiteral("min")), 1, 0);
    zl->addWidget(u.zMin, 1, 1);
    zl->addWidget(new QLabel(QStringLiteral("max")), 2, 0);
    zl->addWidget(u.zMax, 2, 1);
    if (field) {
        u.perFrame = new QCheckBox(QStringLiteral("Авто по текущему такту (иначе по всем)"));
        zl->addWidget(u.perFrame, 3, 0, 1, 2);
    }
    zl->addWidget(u.zSize, 4, 0, 1, 2);
    connect(u.zAuto, &QCheckBox::toggled, this, &ParametersDialog::updateEnables);
    g->addWidget(z, 1, 1);

    if (!field) {
        auto *type = new QGroupBox(QStringLiteral("Величина (Type)"));
        auto *tl = new QHBoxLayout(type);
        const char *names[4] = {"ε (Eps+)", "N — концентрация", "Y — потери", "B — магн. поле"};
        auto *grp = new QButtonGroup(w);
        for (int i = 0; i < 4; ++i) {
            u.type[i] = new QRadioButton(QString::fromUtf8(names[i]));
            grp->addButton(u.type[i]);
            tl->addWidget(u.type[i]);
            u.type[i]->setEnabled(i == 0 || m_ctx.plasma);
        }
        g->addWidget(type, 2, 0, 1, 2);
        if (!m_ctx.epsAvailable) {
            auto *warn = new QLabel(QStringLiteral("Слой ε недоступен: ") + m_ctx.epsReason);
            warn->setWordWrap(true);
            warn->setStyleSheet(QStringLiteral("color: #e3b341;"));
            g->addWidget(warn, 3, 0, 1, 2);
        }
    }
    return w;
}

void ParametersDialog::load()
{
    const double lu = m_ctx.longUnitValue > 0 ? m_ctx.longUnitValue : 1.0;
    const PlaceSettings &p = m_s.place;
    m_xAuto->setChecked(p.xAuto);
    m_yAuto->setChecked(p.yAuto);
    setNum(m_xMin, (p.xMin + m_ctx.xOrigin) / lu);
    setNum(m_xMax, (p.xMax + m_ctx.xOrigin) / lu);
    setNum(m_yMin, (p.yMin + m_ctx.yOrigin) / lu);
    setNum(m_yMax, (p.yMax + m_ctx.yOrigin) / lu);
    m_angX->setValue(m_s.camera.angleX);
    m_angY->setValue(m_s.camera.angleY);
    m_angZ->setValue(m_s.camera.angleZ);
    m_trX->setValue(m_s.camera.transX);
    m_trY->setValue(m_s.camera.transY);
    m_trZ->setValue(m_s.camera.transZ);
    m_zoom->setValue(m_s.camera.zoom);
    m_sizeX->setChecked(p.drawSizeX);
    m_sizeY->setChecked(p.drawSizeY);
    m_drawAxes->setChecked(p.drawAxes);
    m_prop->setChecked(p.proportional);

    const TopologySettings &t = m_s.topo;
    const bool flags[6] = {t.eps, t.metal, t.magnetic, t.input, t.absorber, t.plasma};
    for (int i = 0; i < 6; ++i)
        m_tFlag[i]->setChecked(flags[i]);
    m_tLine->setChecked(t.line);
    m_tSurf->setChecked(t.surface);
    (t.dim3d ? m_t3d : m_t2d)->setChecked(true);
    m_tSizeBlock->setChecked(t.sizeBlock);
    m_tSizeLink->setChecked(t.sizeLinkList);
    m_tBlend->setValue(t.blend);

    for (int k = 0; k < 2; ++k) {
        LayerUi &u = m_l[k];
        const LayerSettings &l = k ? static_cast<const LayerSettings &>(m_s.field) : static_cast<const LayerSettings &>(m_s.eps);
        u.surface->setChecked(l.surface);
        u.line->setChecked(l.line);
        (l.dim3d ? u.d3 : u.d2)->setChecked(true);
        u.blend->setValue(l.blend);
        u.zAuto->setChecked(l.zAuto);
        setNum(u.zMin, k ? m_ctx.fieldZMin : m_ctx.epsZMin);
        setNum(u.zMax, k ? m_ctx.fieldZMax : m_ctx.epsZMax);
        u.zSize->setChecked(l.drawZSize);
        u.smooth->setChecked(l.smooth);
        u.scale->setCurrentIndex(std::max(0, u.scale->findData(int(l.scale))));
    }
    m_l[1].modulus->setChecked(m_s.field.modulus);
    m_l[1].perFrame->setChecked(m_s.field.zPerFrame);
    m_l[1].height->setValue(m_s.field.heightScale);
    m_l[0].type[std::clamp(m_s.eps.type, 0, 3)]->setChecked(true);
    refreshSwatches();
}

void ParametersDialog::store()
{
    const double lu = m_ctx.longUnitValue > 0 ? m_ctx.longUnitValue : 1.0;
    PlaceSettings &p = m_s.place;
    p.xAuto = m_xAuto->isChecked();
    p.yAuto = m_yAuto->isChecked();
    // В окне края — в единицах длины файла от начала координат задания; в настройках —
    // метры от узла (0, 0) сетки.
    p.xMin = getNum(m_xMin, 0) * lu - m_ctx.xOrigin;
    p.xMax = getNum(m_xMax, 0) * lu - m_ctx.xOrigin;
    p.yMin = getNum(m_yMin, 0) * lu - m_ctx.yOrigin;
    p.yMax = getNum(m_yMax, 0) * lu - m_ctx.yOrigin;
    m_s.camera.angleX = m_angX->value();
    m_s.camera.angleY = m_angY->value();
    m_s.camera.angleZ = m_angZ->value();
    m_s.camera.transX = m_trX->value();
    m_s.camera.transY = m_trY->value();
    m_s.camera.transZ = m_trZ->value();
    m_s.camera.zoom = m_zoom->value();
    p.drawSizeX = m_sizeX->isChecked();
    p.drawSizeY = m_sizeY->isChecked();
    p.drawAxes = m_drawAxes->isChecked();
    p.proportional = m_prop->isChecked();

    TopologySettings &t = m_s.topo;
    t.eps = m_tFlag[0]->isChecked();
    t.metal = m_tFlag[1]->isChecked();
    t.magnetic = m_tFlag[2]->isChecked();
    t.input = m_tFlag[3]->isChecked();
    t.absorber = m_tFlag[4]->isChecked();
    t.plasma = m_tFlag[5]->isChecked();
    t.line = m_tLine->isChecked();
    t.surface = m_tSurf->isChecked();
    t.dim3d = m_t3d->isChecked();
    t.sizeBlock = m_tSizeBlock->isChecked();
    t.sizeLinkList = m_tSizeLink->isChecked();
    t.blend = m_tBlend->value();

    for (int k = 0; k < 2; ++k) {
        LayerUi &u = m_l[k];
        LayerSettings &l = k ? static_cast<LayerSettings &>(m_s.field) : static_cast<LayerSettings &>(m_s.eps);
        l.surface = u.surface->isChecked();
        l.line = u.line->isChecked();
        l.dim3d = u.d3->isChecked();
        l.blend = u.blend->value();
        l.zAuto = u.zAuto->isChecked();
        l.zMin = getNum(u.zMin, l.zMin);
        l.zMax = getNum(u.zMax, l.zMax);
        if (l.zMax < l.zMin)
            std::swap(l.zMin, l.zMax);
        l.drawZSize = u.zSize->isChecked();
        l.smooth = u.smooth->isChecked();
        l.scale = ScaleKind(u.scale->currentData().toInt());
    }
    m_s.field.modulus = m_l[1].modulus->isChecked();
    m_s.field.zPerFrame = m_l[1].perFrame->isChecked();
    m_s.field.heightScale = m_l[1].height->value();
    for (int i = 0; i < 4; ++i)
        if (m_l[0].type[i]->isChecked())
            m_s.eps.type = i;
    m_s.propertyPage = m_tabs->currentIndex();
}

ViewSettings ParametersDialog::result()
{
    store();
    return m_s;
}

void ParametersDialog::updateEnables()
{
    m_xMin->setEnabled(!m_xAuto->isChecked());
    m_xMax->setEnabled(!m_xAuto->isChecked());
    m_yMin->setEnabled(!m_yAuto->isChecked());
    m_yMax->setEnabled(!m_yAuto->isChecked());
    for (LayerUi &u : m_l) {
        u.zMin->setEnabled(!u.zAuto->isChecked());
        u.zMax->setEnabled(!u.zAuto->isChecked());
    }
}

void ParametersDialog::refreshSwatches()
{
    for (int i = 0; i < 6; ++i)
        m_tSwatch[i]->setPixmap(swatch(m_s.topo.colors[i]));
    QPixmap pm(90, 26);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setFont(m_s.place.axesFont);
    p.setPen(m_s.place.axesColor);
    p.drawText(QRectF(0, 0, 90, 26), Qt::AlignCenter, QStringLiteral("Inp 1  12.5"));
    p.end();
    m_axesSample->setPixmap(pm);
    for (int k = 0; k < 2; ++k) {
        LayerUi &u = m_l[k];
        const LayerSettings &l = k ? static_cast<const LayerSettings &>(m_s.field) : static_cast<const LayerSettings &>(m_s.eps);
        ScaleKind sk = ScaleKind(u.scale->currentData().toInt());
        if (sk == ScaleKind::Auto)
            sk = k ? ScaleKind::Wave : ScaleKind::Glow;
        u.preview->setPixmap(scalePreview(sk, l.custom, u.smooth->isChecked()));
    }
}

} // namespace fieldview
