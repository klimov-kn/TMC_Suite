/// @file import3ddialog.cpp
/// @brief Реализация окна «Срез 3D-модели» `Import3DDialog`.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-mesh
#include "import3ddialog.h"

#include "emitter.h"
#include "geometryview.h"
#include "materialmap.h"
#include "meshio.h"
#include "meshview.h"
#include "sigfile.h"
#include "dtheme.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QSplitter>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include "lang.h"

using namespace tpl;

namespace {

// Что может выбрать человек для тела. Порядок — порядок строк выпадающего списка.
/// @brief Выбор для тела в таблице (индекс строки выпадающего списка).
enum Kind { KMetal, KMagnetic, KAbsorber, KDielectric, KSkip };
/// @var KMetal
/// Металл.
/// @var KMagnetic
/// Магнитная стенка.
/// @var KAbsorber
/// Поглотитель.
/// @var KDielectric
/// Диэлектрик с заданной eps.
/// @var KSkip
/// Тело не вставлять.

/// Названия выборов `Kind`: пары «русский, английский».
const char *const kKindNames[][2] = {{"Металл", "Metal"},
                                     {"Магнитная стенка", "Magnetic wall"},
                                     {"Поглотитель", "Absorber"},
                                     {"Диэлектрик", "Dielectric"},
                                     {"Не вставлять", "Do not insert"}};

/// @brief Переводит выбор тела ядра генератора в строку списка.
/// @param[in] c  выбор тела.
/// @return `KSkip`, если тело не вставляется, иначе вид по материалу.
Kind kindOf(const BodyChoice &c)
{
    if (!c.include)
        return KSkip;
    switch (c.material) {
    case Material::Metal: return KMetal;
    case Material::Magnetic: return KMagnetic;
    case Material::Absorber: return KAbsorber;
    case Material::Dielectric: return KDielectric;
    }
    return KMetal;
}

/// Единицы файла: STL и OBJ их не хранят — в CAD модель могла быть и в метрах.
/// @brief Единица длины файла: название и множитель.
struct Unit { const char *name; const char *nameEn; double mm; };
/// @var Unit::name
/// Название по-русски.
/// @var Unit::nameEn
/// Название по-английски.
/// @var Unit::mm
/// Миллиметров в единице.

/// Единицы, которые можно выбрать для STL и OBJ.
const Unit kUnits[] = {{"миллиметры", "millimeters", 1.0}, {"сантиметры", "centimeters", 10.0},
                       {"метры", "meters", 1000.0}, {"дюймы", "inches", 25.4},
                       {"микрометры", "micrometers", 0.001}};

/// @brief Возвращает ключ `QSettings`, под которым запоминается выбор материала.
/// @param[in] name  имя материала или тела из файла.
/// @return `import3d/materials/<имя в нижнем регистре без краевых пробелов>`.
QString settingsKey(const QString &name)
{
    return QStringLiteral("import3d/materials/") + name.trimmed().toLower();
}

/// @brief Кодирует выбор тела строкой для `QSettings`.
/// @param[in] c  выбор тела.
/// @return `metal`, `magnetic`, `absorber`, `dielectric:<eps>` или `skip`.
QString encode(const BodyChoice &c)
{
    switch (kindOf(c)) {
    case KMetal: return QStringLiteral("metal");
    case KMagnetic: return QStringLiteral("magnetic");
    case KAbsorber: return QStringLiteral("absorber");
    case KDielectric: return QStringLiteral("dielectric:%1").arg(c.eps, 0, 'g', 10);
    case KSkip: return QStringLiteral("skip");
    }
    return QString();
}

/// @brief Разбирает строку, записанную `encode`.
/// @param[in]  s  строка из `QSettings`.
/// @param[out] c  выбор тела; eps <= 0 у диэлектрика заменяется на 2.
/// @return false — строка пустая (выбор не запоминался).
bool decode(const QString &s, BodyChoice *c)
{
    if (s.isEmpty())
        return false;
    c->include = s != QLatin1String("skip");
    if (s == QLatin1String("metal"))
        c->material = Material::Metal;
    else if (s == QLatin1String("magnetic"))
        c->material = Material::Magnetic;
    else if (s == QLatin1String("absorber"))
        c->material = Material::Absorber;
    else if (s.startsWith(QLatin1String("dielectric:"))) {
        c->material = Material::Dielectric;
        c->eps = s.mid(11).toDouble();
        if (c->eps <= 0)
            c->eps = 2.0;
    }
    return true;
}

/// @brief Возвращает цвет тела в 3D-виде по выбору материала.
/// @param[in] k  выбор.
/// @return Цвет; у «не вставлять» — полупрозрачный.
QColor kindColor(Kind k)
{
    switch (k) {
    case KMetal: return QColor(0xc9, 0xa2, 0x7e);      // медь
    case KMagnetic: return QColor(0xb4, 0x8c, 0xff);
    case KAbsorber: return QColor(0x6a, 0x6f, 0x78);
    case KDielectric: return QColor(0x7f, 0xb8, 0xff);
    case KSkip: return QColor(0x9f, 0xb0, 0xc8, 50); // полупрозрачное: не вставляется
    }
    return Qt::gray;
}

} // namespace

Import3DDialog::Import3DDialog(const Project &host, QWidget *parent)
    : QDialog(parent), m_host(host), m_fitBox(host.box)
{
    setWindowTitle(tpl::tr("Срез 3D-модели", "3D model section"));
    setObjectName(QStringLiteral("import3dDialog"));
    buildUi();
    resize(1280, 820);
}

void Import3DDialog::buildUi()
{
    auto *root = new QVBoxLayout(this);
    m_fileInfo = new QLabel;
    m_fileInfo->setObjectName(QStringLiteral("import3dFile"));
    m_fileInfo->setWordWrap(true);
    root->addWidget(m_fileInfo);

    auto *split = new QSplitter(Qt::Horizontal);
    split->setChildrenCollapsible(false);
    root->addWidget(split, 1);

    // --- слева: вид и плоскость ---
    auto *left = new QWidget;
    auto *ll = new QVBoxLayout(left);
    ll->setContentsMargins(0, 0, 0, 0);

    // Переключатель «модель / что вставится». Ползунок плоскости работает в обоих
    // режимах: двигают плоскость, глядя на модель, а проверяют — глядя на результат.
    auto *mode = new QHBoxLayout;
    m_modeGroup = new QButtonGroup(this);
    m_modeGroup->setExclusive(true);
    const QString modes[] = {tpl::tr("3D-модель", "3D model"),
                             tpl::tr("Что вставится в задание", "What goes into the task")};
    for (int i = 0; i < 2; ++i) {
        auto *b = new QPushButton(modes[i]);
        b->setCheckable(true);
        b->setObjectName(i == 0 ? QStringLiteral("mode3d") : QStringLiteral("modePreview"));
        b->setMinimumHeight(32);
        // Нажатая кнопка обязана выглядеть нажатой: иначе не видно, ЧТО сейчас на экране —
        // модель или то, что вставится.
        // Жирный — в ОБОИХ состояниях: ширину кнопки Qt считает один раз, и жирный текст
        // только у нажатой вылезал за её края.
        b->setStyleSheet(QStringLiteral("QPushButton { font-weight: bold; padding: 0 14px; }"
                                        "QPushButton:checked { background: %1; color: %2; }")
                             .arg(dtheme::accent().name(), dtheme::bg().name()));
        m_modeGroup->addButton(b, i);
        mode->addWidget(b);
    }
    m_modeGroup->button(0)->setChecked(true);
    mode->addStretch();
    ll->addLayout(mode);

    m_stack = new QStackedWidget;
    m_view3d = new MeshView;
    m_preview = new GeometryView;
    m_preview->setObjectName(QStringLiteral("import3dPreview"));
    m_preview->setPreview(true);
    m_stack->addWidget(m_view3d);
    m_stack->addWidget(m_preview);
    ll->addWidget(m_stack, 1);
    connect(m_modeGroup, &QButtonGroup::idClicked, this, [this](int id) { showPreview(id == 1); });

    auto *planeRow = new QHBoxLayout;
    auto *pc = new QLabel(tpl::tr("Плоскость среза:", "Section plane:"));
    planeRow->addWidget(pc);
    m_planeGroup = new QButtonGroup(this);
    m_planeGroup->setExclusive(true);
    for (SlicePlane p : {SlicePlane::XY, SlicePlane::YZ, SlicePlane::XZ}) {
        auto *b = new QPushButton(slicePlaneName(p));
        b->setCheckable(true);
        b->setObjectName(QStringLiteral("plane") + slicePlaneName(p));
        b->setMinimumSize(58, 30);
        const QColor c = MeshView::planeColor(p);
        b->setStyleSheet(QStringLiteral("QPushButton { border: 2px solid %1; color: %1; font-weight: bold; }"
                                        "QPushButton:checked { background: %1; color: #0a0e16; }")
                             .arg(c.name()));
        m_planeGroup->addButton(b, int(p));
        planeRow->addWidget(b);
    }
    connect(m_planeGroup, &QButtonGroup::idClicked, this,
            [this](int id) { choosePlane(SlicePlane(id)); });
    connect(m_view3d, &MeshView::planeClicked, this, &Import3DDialog::choosePlane);

    m_axisLabel = new QLabel(QStringLiteral("Z ="));
    planeRow->addSpacing(12);
    planeRow->addWidget(m_axisLabel);
    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setObjectName(QStringLiteral("sliceSlider"));
    m_slider->setRange(0, 1000);
    planeRow->addWidget(m_slider, 1);
    m_offset = new QDoubleSpinBox;
    m_offset->setObjectName(QStringLiteral("sliceOffset"));
    m_offset->setDecimals(3);
    m_offset->setSuffix(tpl::tr(" мм", " mm"));
    m_offset->setMinimumWidth(130);
    planeRow->addWidget(m_offset);
    ll->addLayout(planeRow);

    connect(m_slider, &QSlider::valueChanged, this, [this](int v) {
        if (m_filling)
            return;
        m_filling = true;
        m_offset->setValue(m_lo + (m_hi - m_lo) * v / 1000.0);
        m_filling = false;
        updateSlice();
    });
    connect(m_offset, &QDoubleSpinBox::valueChanged, this, [this](double v) {
        if (m_filling)
            return;
        m_filling = true;
        m_slider->setValue(int(std::lround((v - m_lo) / std::max(1e-12, m_hi - m_lo) * 1000.0)));
        m_filling = false;
        updateSlice();
    });

    m_sliceInfo = new QLabel;
    m_sliceInfo->setObjectName(QStringLiteral("sliceInfo"));
    m_sliceInfo->setWordWrap(true);
    ll->addWidget(m_sliceInfo);
    split->addWidget(left);

    // --- справа: тела, единицы, размещение ---
    auto *right = new QWidget;
    right->setMinimumWidth(560);
    auto *rl = new QVBoxLayout(right);
    rl->setContentsMargins(0, 0, 0, 0);
    auto *bt = new QLabel(tpl::tr("Тела модели и материалы в задании", "Model bodies and materials in the task"));
    bt->setProperty("dim", true);
    rl->addWidget(bt);
    m_bodies = new QTableWidget(0, 4);
    m_bodies->setObjectName(QStringLiteral("import3dBodies"));
    m_bodies->setHorizontalHeaderLabels({tpl::tr("Тело", "Body"), tpl::tr("В файле", "In file"),
                                         tpl::tr("В задании", "In task"), QStringLiteral("eps")});
    m_bodies->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_bodies->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_bodies->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_bodies->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_bodies->verticalHeader()->hide();
    m_bodies->setSelectionMode(QAbstractItemView::NoSelection);
    rl->addWidget(m_bodies, 1);

    auto *hint = new QLabel(tpl::tr("Материал узнаётся по имени тела или материала из файла («Медь», «PEC», «FR-4», "
        "«PTFE»…). Что не узнано — помечено «?», выберите сами: выбор запомнится, и в "
        "следующий раз тело с тем же именем узнается.",
        "The material is recognized by the name of the body or of the material in the "
        "file (\"Copper\", \"PEC\", \"FR-4\", \"PTFE\"...). What is not recognized is "
        "marked \"?\": choose it yourself; the choice is remembered, and next time a "
        "body with the same name will be recognized."));
    hint->setProperty("mute", true);
    hint->setWordWrap(true);
    rl->addWidget(hint);

    auto *form = new QGridLayout;
    form->addWidget(new QLabel(tpl::tr("Единицы файла:", "File units:")), 0, 0);
    m_units = new QComboBox;
    m_units->setObjectName(QStringLiteral("import3dUnits"));
    for (const Unit &u : kUnits)
        m_units->addItem(tpl::tr(u.name, u.nameEn), u.mm);
    form->addWidget(m_units, 0, 1);
    m_center = new QCheckBox(tpl::tr("Поставить срез в центр счётной области", "Place the section at the center of the computational domain"));
    m_center->setChecked(true);
    form->addWidget(m_center, 1, 0, 1, 2);
    m_mirror = new QCheckBox(tpl::tr("Отразить зеркально", "Mirror"));
    form->addWidget(m_mirror, 2, 0);
    auto *rot = new QPushButton(tpl::tr("Повернуть на 90°", "Rotate by 90°"));
    rot->setObjectName(QStringLiteral("import3dRotate"));
    form->addWidget(rot, 2, 1);
    m_fit = new QCheckBox(tpl::tr("Расширить счётную область, если срез не помещается", "Enlarge the computational domain if the section does not fit"));
    m_fit->setChecked(true);
    form->addWidget(m_fit, 3, 0, 1, 2);
    rl->addLayout(form);

    m_notes = new QLabel;
    m_notes->setObjectName(QStringLiteral("import3dNotes"));
    m_notes->setWordWrap(true);
    rl->addWidget(m_notes);
    split->addWidget(right);
    split->setStretchFactor(0, 3);
    split->setStretchFactor(1, 2);
    split->setSizes({720, 560});

    connect(m_units, &QComboBox::currentIndexChanged, this, [this] { rescale(); });
    connect(m_center, &QCheckBox::toggled, this, [this] { updateSlice(); });
    connect(m_mirror, &QCheckBox::toggled, this, [this] { updateSlice(); });
    connect(m_fit, &QCheckBox::toggled, this, [this] { updateSlice(); });
    connect(rot, &QPushButton::clicked, this, [this] {
        m_rot90 = (m_rot90 + 1) % 4;
        updateSlice();
    });

    auto *buttons = new QDialogButtonBox;
    m_apply = buttons->addButton(tpl::tr("Применить — вставить в задание", "Apply: insert into the task"),
                                 QDialogButtonBox::AcceptRole);
    m_apply->setObjectName(QStringLiteral("import3dApply"));
    m_apply->setProperty("accent", true);
    m_apply->setMinimumHeight(36);
    buttons->addButton(tpl::tr("Отмена", "Cancel"), QDialogButtonBox::RejectRole);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(buttons);
}

bool Import3DDialog::loadFile(const QString &path, QString *error)
{
    const MeshReadResult r = readMeshFile(path);
    if (!r.ok()) {
        if (error)
            *error = r.error;
        return false;
    }
    m_path = path;
    m_raw = r.mesh;
    m_fileUnitMm = r.fileUnitMm;
    // STEP хранит единицы сам, и OpenCASCADE уже перевёл модель в мм — выбирать нечего.
    m_units->setEnabled(m_fileUnitMm <= 0.0);
    if (m_fileUnitMm > 0.0)
        m_units->setCurrentIndex(0);
    rescale();
    if (!r.warnings.isEmpty())
        m_notes->setText(r.warnings.join(QLatin1Char('\n')));
    return true;
}

void Import3DDialog::rescale()
{
    const double k = m_fileUnitMm > 0.0 ? 1.0 : m_units->currentData().toDouble();
    m_mesh = m_raw;
    m_mesh.scale(k);
    m_mesh.weld();

    const Bounds3 b = m_mesh.bounds();
    const QFileInfo fi(m_path);
    m_fileInfo->setText(
        tpl::tr("<b>%1</b> — %2, %3; габарит %4 × %5 × %6 мм", "<b>%1</b> — %2, %3; size %4 × %5 × %6 mm")
            .arg(fi.fileName().toHtmlEscaped(),
                 plural(int(m_mesh.bodies.size()), tpl::tr("тело", "body"), tpl::tr("тела", "bodies"),
                        tpl::tr("тел", "bodies")),
                 plural(int(m_mesh.tris.size()), tpl::tr("треугольник", "triangle"),
                        tpl::tr("треугольника", "triangles"), tpl::tr("треугольников", "triangles")),
                 formatCompact(b.hi.x - b.lo.x), formatCompact(b.hi.y - b.lo.y),
                 formatCompact(b.hi.z - b.lo.z)));
    fillBodies();
    m_view3d->setMesh(&m_mesh, bodyColors());
    updateRange();
    updateSlice();
}

void Import3DDialog::fillBodies()
{
    m_filling = true;
    m_bodies->setRowCount(int(m_mesh.bodies.size()));
    const QSettings st;
    for (int i = 0; i < m_mesh.bodies.size(); ++i) {
        const MeshBody &body = m_mesh.bodies.at(i);
        BodyChoice c;
        QString source = body.material;
        bool known = decode(st.value(settingsKey(body.material)).toString(), &c)
                     || decode(st.value(settingsKey(body.name)).toString(), &c);
        if (!known) {
            MaterialGuess g = guessMaterial(body.material);
            if (!g.known)
                g = guessMaterial(body.name);
            if (g.known) {
                known = true;
                c.material = g.material;
                c.eps = g.eps;
                c.include = !g.skip;
                source = g.what;
            }
        } else {
            source = tpl::tr("%1 (запомнено)", "%1 (remembered)").arg(body.material);
        }

        auto *name = new QTableWidgetItem(body.name);
        name->setFlags(Qt::ItemIsEnabled);
        name->setToolTip(body.name); // в узкой колонке длинное имя обрезается
        m_bodies->setItem(i, 0, name);
        auto *src = new QTableWidgetItem(known ? source : QStringLiteral("? %1").arg(body.material));
        src->setFlags(Qt::ItemIsEnabled);
        src->setForeground(known ? dtheme::textDim() : dtheme::warn());
        src->setToolTip(known ? tpl::tr("Узнано по имени", "Recognized by name") :
                                tpl::tr("Имя не узнано — выберите материал сами", "Name not recognized: choose the material yourself"));
        m_bodies->setItem(i, 1, src);

        auto *kind = new QComboBox;
        for (const auto &n : kKindNames)
            kind->addItem(tpl::tr(n[0], n[1]));
        kind->setCurrentIndex(kindOf(c));
        m_bodies->setCellWidget(i, 2, kind);
        auto *eps = new QDoubleSpinBox;
        eps->setRange(0.001, 1e6);
        eps->setDecimals(3);
        eps->setValue(c.material == Material::Dielectric ? c.eps : 2.0);
        eps->setEnabled(kindOf(c) == KDielectric);
        m_bodies->setCellWidget(i, 3, eps);

        connect(kind, &QComboBox::currentIndexChanged, this, [this, i, eps](int k) {
            eps->setEnabled(k == KDielectric);
            if (m_filling)
                return;
            rememberChoice(i);
            m_view3d->setBodyColors(bodyColors());
            updateSlice();
        });
        connect(eps, &QDoubleSpinBox::valueChanged, this, [this, i] {
            if (m_filling)
                return;
            rememberChoice(i);
            updateSlice();
        });
    }
    m_filling = false;
}

QVector<BodyChoice> Import3DDialog::choices() const
{
    QVector<BodyChoice> out;
    for (int i = 0; i < m_bodies->rowCount(); ++i) {
        BodyChoice c;
        const auto *kind = qobject_cast<QComboBox *>(m_bodies->cellWidget(i, 2));
        const auto *eps = qobject_cast<QDoubleSpinBox *>(m_bodies->cellWidget(i, 3));
        const int k = kind ? kind->currentIndex() : KMetal;
        c.include = k != KSkip;
        c.material = k == KMagnetic ? Material::Magnetic
                     : k == KAbsorber ? Material::Absorber
                     : k == KDielectric ? Material::Dielectric
                                        : Material::Metal;
        c.eps = eps ? eps->value() : 1.0;
        out << c;
    }
    return out;
}

QVector<QColor> Import3DDialog::bodyColors() const
{
    QVector<QColor> out;
    const QVector<BodyChoice> ch = choices();
    for (int i = 0; i < m_mesh.bodies.size(); ++i) {
        const MeshBody &b = m_mesh.bodies.at(i);
        const Kind k = i < ch.size() ? kindOf(ch.at(i)) : KMetal;
        // Цвет из файла, если он есть: человек узнаёт свою деталь по её цвету.
        // Но «не вставлять» — всегда полупрозрачным, иначе не видно, что тело выключено.
        if (k != KSkip && b.hasColor)
            out << QColor::fromRgb(b.rgb);
        else
            out << kindColor(k);
    }
    return out;
}

void Import3DDialog::rememberChoice(int body)
{
    const QVector<BodyChoice> ch = choices();
    if (body < 0 || body >= ch.size())
        return;
    const MeshBody &b = m_mesh.bodies.at(body);
    QSettings st;
    st.setValue(settingsKey(b.material.isEmpty() ? b.name : b.material), encode(ch.at(body)));
}

void Import3DDialog::updateRange()
{
    const Bounds3 b = m_mesh.bounds();
    const int n = planeAxes(m_plane).normal;
    m_lo = axisValue(b.lo, n);
    m_hi = axisValue(b.hi, n);
    m_filling = true;
    m_offset->setRange(m_lo, m_hi);
    m_offset->setSingleStep(std::max(1e-3, m_host.delta));
    m_offset->setValue((m_lo + m_hi) / 2.0);
    m_slider->setValue(500);
    m_filling = false;
    m_axisLabel->setText(normalAxisName(m_plane) + QStringLiteral(" ="));
}

void Import3DDialog::choosePlane(SlicePlane p)
{
    const bool changed = !m_planeChosen || p != m_plane;
    m_plane = p;
    m_planeChosen = true;
    if (auto *b = m_planeGroup->button(int(p)))
        b->setChecked(true);
    if (changed)
        updateRange();
    updateSlice();
}


void Import3DDialog::showPreview(bool on)
{
    m_stack->setCurrentIndex(on ? 1 : 0);
    if (auto *b = m_modeGroup->button(on ? 1 : 0))
        b->setChecked(true);
    if (on)
        m_preview->zoomToFit();
}

bool Import3DDialog::boxChanged() const
{
    return m_fitBox.xMin != m_host.box.xMin || m_fitBox.xMax != m_host.box.xMax
           || m_fitBox.yMin != m_host.box.yMin || m_fitBox.yMax != m_host.box.yMax;
}

QString Import3DDialog::provenance() const
{
    return sliceProvenance(QFileInfo(m_path).fileName(), m_plane, m_offset->value());
}

void Import3DDialog::updateSlice()
{
    if (m_filling)
        return;
    m_apply->setEnabled(false);
    if (!m_planeChosen || m_mesh.isEmpty()) {
        m_sliceInfo->setText(tpl::tr("Выберите плоскость: щёлкните по цветному "
                                     "прямоугольнику на модели или по кнопке XY, YZ, XZ.",
                                     "Choose a plane: click a colored rectangle on the model or the XY, YZ, XZ "
                                     "button."));
        m_view3d->setSection(SliceResult());
        return;
    }
    m_view3d->setPlane(m_plane, m_offset->value());
    m_slice = sliceMesh(m_mesh, m_plane, m_offset->value());
    m_view3d->setSection(m_slice);

    // Упрощение — до половины шага сетки: мельче ядру нечего считать, а CAD даёт
    // тысячи точек на окружность.
    SliceResult simple = m_slice;
    simplifySlice(&simple, std::max(1e-6, m_host.delta / 2.0));

    ImportPlacement pl;
    pl.rot90 = m_rot90;
    pl.mirror = m_mirror->isChecked();
    if (m_center->isChecked())
        pl.shift = centeringShift(simple, pl, m_host.box);
    m_shapes = buildImportShapes(m_mesh, simple, choices(), pl, m_host.nextFreeId());

    // Область: если срез не помещается — расширить с запасом в десять шагов сетки.
    m_fitBox = m_host.box;
    bool outside = false;
    double x0 = 0, x1 = 0, y0 = 0, y1 = 0;
    bool any = false;
    for (const Shape &s : m_shapes.shapes)
        for (const QPointF &p : s.points) {
            if (!any) {
                x0 = x1 = p.x();
                y0 = y1 = p.y();
                any = true;
            }
            x0 = std::min(x0, p.x()); x1 = std::max(x1, p.x());
            y0 = std::min(y0, p.y()); y1 = std::max(y1, p.y());
        }
    if (any) {
        outside = x0 < m_host.box.xMin || x1 > m_host.box.xMax || y0 < m_host.box.yMin
                  || y1 > m_host.box.yMax;
        if (outside && m_fit->isChecked()) {
            const double pad = 10.0 * m_host.delta;
            m_fitBox.xMin = std::min(m_host.box.xMin, x0 - pad);
            m_fitBox.xMax = std::max(m_host.box.xMax, x1 + pad);
            m_fitBox.yMin = std::min(m_host.box.yMin, y0 - pad);
            m_fitBox.yMax = std::max(m_host.box.yMax, y1 + pad);
        }
    }

    int loops = 0;
    for (const SliceBody &b : m_slice.bodies)
        loops += int(b.loops.size());
    QString info = m_slice.bodies.isEmpty()
                       ? tpl::tr("Плоскость %1 при %2 = %3 мм не задевает модель.", "Plane %1 at %2 = %3 mm does not touch the model.")
                             .arg(slicePlaneName(m_plane), normalAxisName(m_plane),
                                  formatCompact(m_offset->value()))
                       : tpl::tr("Срез %1 при %2 = %3 мм: задето %4, контуров %5 → "
                                 "в задание пойдёт %6.",
                                 "Section %1 at %2 = %3 mm: %4 hit, %5 contours -> %6 go into the task.")
                             .arg(slicePlaneName(m_plane), normalAxisName(m_plane),
                                  formatCompact(m_offset->value()),
                                  plural(int(m_slice.bodies.size()), tpl::tr("тело", "body"),
                                         tpl::tr("тела", "bodies"), tpl::tr("тел", "bodies")))
                             .arg(loops)
                             .arg(plural(int(m_shapes.shapes.size()), tpl::tr("фигура", "shape"),
                                         tpl::tr("фигуры", "shapes"), tpl::tr("фигур", "shapes")));
    m_sliceInfo->setText(info);

    QStringList notes = m_shapes.notes;
    if (outside) {
        notes << (m_fit->isChecked()
                      ? tpl::tr("Срез не помещается в счётную область — она будет расширена "
                                "до %1…%2 × %3…%4 мм.",
                                "The section does not fit into the computational domain; it will be enlarged "
                                "to %1…%2 × %3…%4 mm.")
                            .arg(formatCompact(m_fitBox.xMin), formatCompact(m_fitBox.xMax),
                                 formatCompact(m_fitBox.yMin), formatCompact(m_fitBox.yMax))
                      : tpl::tr("Срез выходит за счётную область: часть фигур окажется "
                                "вне её. Включите расширение области или сдвиньте срез.",
                                "The section goes beyond the computational domain: some shapes will end up "
                                "outside it. Turn on domain enlargement or move the section."));
    }
    m_notes->setText(notes.join(QStringLiteral("\n")));
    m_notes->setStyleSheet(notes.isEmpty() ? QString()
                                           : QStringLiteral("color:%1;").arg(dtheme::warn().name()));

    // Предпросмотр: текущее задание плюс то, что вставится, — тем же холстом.
    m_previewProject = m_host;
    m_previewProject.box = m_fitBox;
    for (const Shape &s : m_shapes.shapes)
        m_previewProject.shapes << s;
    m_preview->setProject(&m_previewProject);
    if (m_stack->currentIndex() == 1)
        m_preview->zoomToFit();
    m_apply->setEnabled(!m_shapes.shapes.isEmpty());
}
