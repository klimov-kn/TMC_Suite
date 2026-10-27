/// @file propertypanel.cpp
/// @brief Реализация панели свойств `PropertyPanel`.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-properties
#include "propertypanel.h"

#include "physics.h"
#include "dtheme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <cmath>
#include "lang.h"

using namespace tpl;

namespace {

/// @brief Создаёт счётчик длины: -100000...100000 мм, три знака, суффикс « мм».
/// @return Новый счётчик без родителя.
QDoubleSpinBox *mmSpin()
{
    auto *s = new QDoubleSpinBox;
    s->setRange(-100000, 100000);
    s->setDecimals(3);
    s->setSuffix(tpl::tr(" мм", " mm"));
    return s;
}

/// @brief Создаёт счётчик угла: -720...720°, один знак, суффикс « °».
/// @return Новый счётчик без родителя.
QDoubleSpinBox *degSpin()
{
    auto *s = new QDoubleSpinBox;
    s->setRange(-720, 720);
    s->setDecimals(1);
    s->setSuffix(QStringLiteral(" °"));
    return s;
}

/// @brief Заполняет список материалов фигуры; данные пункта — `int(tpl::Material)`.
/// @param[in,out] c  список: металл, магнитная стенка, поглотитель, диэлектрик (заливка).
void fillMaterials(QComboBox *c)
{
    c->addItem(tpl::tr("Металл", "Metal"), int(Material::Metal));
    c->addItem(tpl::tr("Магнитная стенка", "Magnetic wall"), int(Material::Magnetic));
    c->addItem(tpl::tr("Поглотитель", "Absorber"), int(Material::Absorber));
    c->addItem(tpl::tr("Диэлектрик (заливка)", "Dielectric (fill)"), int(Material::Dielectric));
}

/// @brief Возвращает название стороны прямоугольника для заголовка панели.
/// @param[in] e  номер стороны (`EdgeBottom`, `EdgeRight`, `EdgeTop`, `EdgeLeft`).
/// @return «нижняя сторона» и т. п.; пусто для неизвестного номера.
QString edgeTitle(int e)
{
    switch (e) {
    case EdgeBottom: return tpl::tr("нижняя сторона", "bottom side");
    case EdgeRight:  return tpl::tr("правая сторона", "right side");
    case EdgeTop:    return tpl::tr("верхняя сторона", "top side");
    case EdgeLeft:   return tpl::tr("левая сторона", "left side");
    }
    return {};
}

} // namespace

PropertyPanel::PropertyPanel(QWidget *parent)
    : QWidget(parent)
{
    build();
    showSelection({});
}

void PropertyPanel::build()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(6);

    m_title = new QLabel;
    m_title->setProperty("h2", true);
    m_title->setWordWrap(true);
    root->addWidget(m_title);

    m_kindHint = new QLabel;
    m_kindHint->setProperty("mute", true);
    m_kindHint->setWordWrap(true);
    root->addWidget(m_kindHint);

    m_form = new QFormLayout;
    m_form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    root->addLayout(m_form);

    m_name = new QLineEdit;
    m_name->setObjectName(QStringLiteral("shapeName"));
    m_form->addRow(tpl::tr("Название:", "Name:"), m_name);

    m_material = new QComboBox;
    fillMaterials(m_material);
    m_form->addRow(tpl::tr("Материал:", "Material:"), m_material);

    // eps <= 0 — законный расчёт: запредельная среда. Справочник прямо показывает
    // eps = 0 (добавка -1) и eps = 0.25. Ограничение снизу здесь стояло зря.
    m_eps = new QDoubleSpinBox;
    m_eps->setRange(-10000, 10000);
    m_eps->setDecimals(4);
    m_form->addRow(tpl::tr("Проницаемость eps:", "Permittivity eps:"), m_eps);

    m_epsHint = new QLabel;
    m_epsHint->setProperty("mute", true);
    m_epsHint->setWordWrap(true);
    m_form->addRow(QString(), m_epsHint);

    // --- плазма: N, B и Y живут в ОДНОЙ фигуре ---
    //
    // Замагниченная плазма — это концентрация и подмагничивание в одной и той же
    // области. Пока величина была одна, её приходилось собирать из трёх фигур друг
    // на друге и помнить, что B действует лишь поверх N. Теперь три галочки, а на
    // сколько блоков это разложится в файле — забота эмиттера.
    //
    // Значения полями ввода, а не спинбоксами: концентрация меряется в 1e17, и
    // набирать её колёсиком мучение.
    m_plasmaN = new QCheckBox(tpl::tr("Концентрация N, м^-3", "Density N, m^-3"));
    m_plasmaNValue = new QLineEdit;
    m_form->addRow(m_plasmaN, m_plasmaNValue);

    // Величина из файла распределения: блоки FILE_N, FILE_B, FILE_Y. Файл пишет само
    // ядро блоком RECT_STAT_N [имя]... — в поле величины это `[имя]<число или формула>`.
    const auto fileRow = [this](QLineEdit **edit, QCheckBox *flag, const QString &what) {
        auto *row = new QWidget;
        auto *l = new QHBoxLayout(row);
        l->setContentsMargins(0, 0, 0, 0);
        *edit = new QLineEdit;
        (*edit)->setPlaceholderText(tpl::tr("пусто — берём значение выше", "empty — the value above is used"));
        (*edit)->setToolTip(tpl::tr("Файл распределения .eps рядом с заданием (имя без расширения). Его пишет само "
            "ядро, если в поле величины написать [имя] перед значением. Читает такие файлы "
            "ядро X от 26.10.2026; файл другой величины ядро отвергнет.",
            "Distribution file .eps next to the task (name without extension). The kernel "
            "writes it itself if you put [name] before the value in the quantity field. "
            "The X kernel of 26.10.2026 reads such files; a file of another quantity is "
            "rejected by the kernel."));
        auto *browse = new QPushButton(tpl::tr("Обзор...", "Browse..."));
        l->addWidget(*edit, 1);
        l->addWidget(browse);
        QLineEdit *target = *edit;
        connect(browse, &QPushButton::clicked, this, [this, target, flag, what] {
            const QString f = QFileDialog::getOpenFileName(
                this, what, QString(), tpl::tr("Распределения (*.eps);;Все файлы (*)", "Distributions (*.eps);;All files (*)"));
            if (f.isEmpty())
                return;
            // Ядро ищет файл рядом с заданием и само дописывает .eps — нужно только имя.
            target->setText(QFileInfo(f).completeBaseName());
            flag->setChecked(true);
            pushToProject();
        });
        return row;
    };

    m_plasmaNFileRow = fileRow(&m_plasmaNFile, m_plasmaN, tpl::tr("Концентрация N из файла", "Density N from file"));
    m_form->addRow(tpl::tr("N из файла:", "N from file:"), m_plasmaNFileRow);

    m_plasmaB = new QCheckBox(tpl::tr("Подмагничивание B, Тл", "Magnetization B, T"));
    m_plasmaBValue = new QLineEdit;
    m_form->addRow(m_plasmaB, m_plasmaBValue);
    m_plasmaBFileRow = fileRow(&m_plasmaBFile, m_plasmaB, tpl::tr("Подмагничивание B из файла", "Magnetization B from file"));
    m_form->addRow(tpl::tr("B из файла:", "B from file:"), m_plasmaBFileRow);
    m_plasmaBExact = new QCheckBox(tpl::tr("B ровно по плазме (без запаса)", "B exactly over the plasma (no margin)"));
    m_plasmaBExact->setToolTip(tpl::tr("Обычно блок B пишется на два шага шире плазмы: на границе блока ядро "
        "ополовинивает значение. Ровно по плазме — если краевой слой с полным B "
        "резонирует внутри полосы (об этом скажет проверка).",
        "Normally the B block is written two steps wider than the plasma: at the block "
        "boundary the kernel halves the value. Exactly over the plasma — if the edge "
        "layer with full B resonates inside the band (the check will tell you)."));
    m_form->addRow(QString(), m_plasmaBExact);

    m_plasmaS = new QCheckBox(tpl::tr("Потери: проводимость σ, См/м", "Losses: conductivity σ, S/m"));
    m_plasmaS->setToolTip(tpl::tr("Потери в физических единицах. Ядро само пересчитывает σ в сеточные потери через "
        "шаг сетки, поэтому затухание не зависит от DELTA (ядро X от 26.10.2026).",
        "Losses in physical units. The kernel itself converts σ into grid losses via "
        "the grid step, so the attenuation does not depend on DELTA (X kernel of "
        "26.10.2026)."));
    m_plasmaSValue = new QLineEdit;
    m_form->addRow(m_plasmaS, m_plasmaSValue);

    m_plasmaY = new QCheckBox(tpl::tr("Параметр Y (сеточный — для опытных)", "Parameter Y (grid quantity — for experts)"));
    m_plasmaY->setToolTip(tpl::tr("Y — сеточная величина: σ = Y·eps0/(2·dT), а dT зависит от шага сетки. То же Y на "
        "вдвое более мелкой сетке даёт вдвое большее затухание. Обычно потери задают "
        "проводимостью σ строкой выше.",
        "Y is a grid quantity: σ = Y·eps0/(2·dT), and dT depends on the grid step. The "
        "same Y on a twice finer grid gives twice the attenuation. Losses are normally "
        "set by the conductivity σ one line above."));
    m_plasmaYValue = new QLineEdit;
    m_form->addRow(m_plasmaY, m_plasmaYValue);
    m_plasmaYFileRow = fileRow(&m_plasmaYFile, m_plasmaY, tpl::tr("Параметр Y из файла", "Parameter Y from file"));
    m_form->addRow(tpl::tr("Y из файла:", "Y from file:"), m_plasmaYFileRow);

    m_plasmaHint = new QLabel;
    m_plasmaHint->setProperty("mute", true);
    m_plasmaHint->setWordWrap(true);
    m_form->addRow(QString(), m_plasmaHint);

    m_xMin = mmSpin();
    m_xMax = mmSpin();
    m_yMin = mmSpin();
    m_yMax = mmSpin();
    // Имена нужны служебным утилитам: автопроход по руководству правит те же поля,
    // что и человек, и ищет их по имени — порядок в разметке меняется от любой правки.
    m_xMin->setObjectName(QStringLiteral("shapeXMin"));
    m_xMax->setObjectName(QStringLiteral("shapeXMax"));
    m_yMin->setObjectName(QStringLiteral("shapeYMin"));
    m_yMax->setObjectName(QStringLiteral("shapeYMax"));
    m_form->addRow(tpl::tr("X от:", "X from:"), m_xMin);
    m_form->addRow(tpl::tr("X до:", "X to:"), m_xMax);
    m_form->addRow(tpl::tr("Y от:", "Y from:"), m_yMin);
    m_form->addRow(tpl::tr("Y до:", "Y to:"), m_yMax);

    m_cx = mmSpin();
    m_cy = mmSpin();
    m_rMin = mmSpin();
    m_rMax = mmSpin();
    m_fiMin = degSpin();
    m_fiMax = degSpin();
    m_form->addRow(tpl::tr("Центр X:", "Center X:"), m_cx);
    m_form->addRow(tpl::tr("Центр Y:", "Center Y:"), m_cy);
    m_form->addRow(tpl::tr("Радиус внутр.:", "Inner radius:"), m_rMin);
    m_form->addRow(tpl::tr("Радиус внеш.:", "Outer radius:"), m_rMax);
    m_form->addRow(tpl::tr("Угол от:", "Angle from:"), m_fiMin);
    m_form->addRow(tpl::tr("Угол до:", "Angle to:"), m_fiMax);

    m_points = new QLabel;
    m_points->setProperty("mute", true);
    m_points->setWordWrap(true);
    m_form->addRow(tpl::tr("Вершины:", "Vertices:"), m_points);

    m_edgePresent = new QCheckBox(tpl::tr("Сторона существует", "Side exists"));
    m_form->addRow(QString(), m_edgePresent);
    m_edgeCustom = new QCheckBox(tpl::tr("Свой материал у стороны", "Own material for the side"));
    m_form->addRow(QString(), m_edgeCustom);
    m_edgeMaterial = new QComboBox;
    m_edgeMaterial->addItem(tpl::tr("Металл", "Metal"), int(Material::Metal));
    m_edgeMaterial->addItem(tpl::tr("Магнитная стенка", "Magnetic wall"), int(Material::Magnetic));
    m_edgeMaterial->addItem(tpl::tr("Поглотитель", "Absorber"), int(Material::Absorber));
    m_form->addRow(tpl::tr("Материал стороны:", "Side material:"), m_edgeMaterial);

    // «Сторона» — это направление падающей волны, а не место установки: ядро выводит
    // его из того, в какой половине области лежит плоскость входа.
    m_portSide = new QComboBox;
    m_portSide->addItem(tpl::tr("Слева направо (вдоль X)", "Left to right (along X)"), int(PortSide::Left));
    m_portSide->addItem(tpl::tr("Справа налево (вдоль X)", "Right to left (along X)"), int(PortSide::Right));
    m_portSide->addItem(tpl::tr("Снизу вверх (вдоль Y)", "Bottom to top (along Y)"), int(PortSide::Bottom));
    m_portSide->addItem(tpl::tr("Сверху вниз (вдоль Y)", "Top to bottom (along Y)"), int(PortSide::Top));
    m_form->addRow(tpl::tr("Волна идёт:", "Wave travels:"), m_portSide);
    m_portPlane = mmSpin();
    m_form->addRow(tpl::tr("Плоскость входа:", "Port plane:"), m_portPlane);
    m_portFrom = mmSpin();
    m_portTo = mmSpin();
    m_form->addRow(tpl::tr("Апертура от:", "Aperture from:"), m_portFrom);
    m_form->addRow(tpl::tr("Апертура до:", "Aperture to:"), m_portTo);
    m_portActive = new QComboBox;
    m_portActive->addItem(tpl::tr("Активный — излучает", "Active — radiates"), 1);
    m_portActive->addItem(tpl::tr("Пассивный — только принимает", "Passive — receives only"), 0);
    m_form->addRow(tpl::tr("Роль:", "Role:"), m_portActive);
    m_portMode = new QComboBox;
    m_portMode->addItem(tpl::tr("Волна H10 (волноводная)", "H10 wave (waveguide)"), int(PortMode::H10));
    m_portMode->addItem(tpl::tr("Плоская волна", "Plane wave"), int(PortMode::Plane));
    m_form->addRow(tpl::tr("Тип волны:", "Wave type:"), m_portMode);
    m_portCutoff = new QLabel;
    m_portCutoff->setProperty("mute", true);
    m_portCutoff->setWordWrap(true);
    m_form->addRow(QString(), m_portCutoff);

    auto *filletRow = new QWidget;
    auto *fl = new QHBoxLayout(filletRow);
    fl->setContentsMargins(0, 0, 0, 0);
    m_filletR = mmSpin();
    m_filletR->setRange(0.0, 100000);
    m_filletR->setValue(5.0);
    m_fillet = new QPushButton(tpl::tr("Скруглить", "Round off"));
    fl->addWidget(m_filletR, 1);
    fl->addWidget(m_fillet);
    m_form->addRow(tpl::tr("Радиус скругления:", "Rounding radius:"), filletRow);

    auto *buttons = new QWidget;
    auto *bl = new QHBoxLayout(buttons);
    bl->setContentsMargins(0, 0, 0, 0);
    m_full = new QPushButton(tpl::tr("Все свойства...", "All properties..."));
    m_merge = new QPushButton(tpl::tr("Соединить", "Join"));
    m_merge->setObjectName(QStringLiteral("mergeShapes"));
    m_merge->setEnabled(false);
    m_dup = new QPushButton(tpl::tr("Дублировать", "Duplicate"));
    m_del = new QPushButton(tpl::tr("Удалить", "Delete"));
    bl->addWidget(m_full);
    bl->addWidget(m_merge);
    bl->addWidget(m_dup);
    bl->addWidget(m_del);
    root->addWidget(buttons);
    root->addStretch();

    // --- связи ---
    const auto push = [this] { pushToProject(); };
    // Текстовые поля — по окончании ввода: иначе на каждой букве проект переписывался
    // бы заново, а курсор в строке прыгал в конец.
    connect(m_name, &QLineEdit::editingFinished, this, push);
    connect(m_material, &QComboBox::currentIndexChanged, this, push);
    connect(m_eps, &QDoubleSpinBox::valueChanged, this, push);
    for (auto *c : {m_plasmaN, m_plasmaB, m_plasmaBExact, m_plasmaS, m_plasmaY})
        connect(c, &QCheckBox::toggled, this, push);
    for (auto *l : {m_plasmaNValue, m_plasmaNFile, m_plasmaBValue, m_plasmaBFile, m_plasmaSValue,
                    m_plasmaYValue, m_plasmaYFile})
        connect(l, &QLineEdit::editingFinished, this, push);
    for (auto *s : {m_xMin, m_xMax, m_yMin, m_yMax, m_cx, m_cy, m_rMin, m_rMax,
                    m_fiMin, m_fiMax, m_portPlane, m_portFrom, m_portTo})
        connect(s, &QDoubleSpinBox::valueChanged, this, push);
    connect(m_edgePresent, &QCheckBox::toggled, this, push);
    connect(m_edgeCustom, &QCheckBox::toggled, this, push);
    connect(m_edgeMaterial, &QComboBox::currentIndexChanged, this, push);
    connect(m_portSide, &QComboBox::currentIndexChanged, this, push);
    connect(m_portActive, &QComboBox::currentIndexChanged, this, push);
    connect(m_portMode, &QComboBox::currentIndexChanged, this, push);

    connect(m_fillet, &QPushButton::clicked, this,
            [this] { emit filletRequested(m_filletR->value()); });
    connect(m_full, &QPushButton::clicked, this, [this] {
        if (!m_sel.isEmpty())
            emit fullPropertiesRequested(m_sel.id);
    });
    connect(m_merge, &QPushButton::clicked, this, &PropertyPanel::mergeRequested);
    connect(m_dup, &QPushButton::clicked, this, &PropertyPanel::duplicateRequested);
    connect(m_del, &QPushButton::clicked, this, &PropertyPanel::deleteRequested);
}

void PropertyPanel::setProject(Project *p)
{
    m_project = p;
    showSelection(m_sel);
}

Shape *PropertyPanel::shape()
{
    if (!m_project || m_sel.isEmpty() || m_sel.kind == Selection::PortBody)
        return nullptr;
    for (Shape &s : m_project->shapes) {
        if (s.id == m_sel.id)
            return &s;
    }
    return nullptr;
}

Port *PropertyPanel::port()
{
    if (!m_project || m_sel.kind != Selection::PortBody)
        return nullptr;
    for (Port &p : m_project->ports) {
        if (p.id == m_sel.id)
            return &p;
    }
    return nullptr;
}

void PropertyPanel::setMergeEnabled(bool on, const QString &why)
{
    m_merge->setEnabled(on);
    m_merge->setToolTip(why.isEmpty()
                            ? tpl::tr("Сшивает выделенные линии, дуги и контуры в "
                                      "одну фигуру. Набрать несколько — Ctrl+клик "
                                      "по холсту или в списке объектов.",
                                      "Stitches the selected lines, arcs and polygons into one shape. To pick "
                                      "several — Ctrl+click on the canvas or in the object list.")
                            : why);
}

void PropertyPanel::showSelection(const Selection &s)
{
    m_sel = s;
    syncFromProject();
}

void PropertyPanel::updateDerived()
{
    const Shape *s = shape();
    const Port *p = port();
    const bool isShape = (s != nullptr);
    const bool isEdge = isShape && m_sel.kind == Selection::ShapeEdge;
    const bool rect = isShape && s->kind == ShapeKind::Rectangle;
    const bool circle = isShape && s->kind == ShapeKind::Circle;
    const bool chain = isShape && (s->kind == ShapeKind::Polygon || s->kind == ShapeKind::Polyline);
    const bool plasma = isShape && s->plasma.any();
    const bool xMode = m_project && m_project->kernel == Kernel::X;
    const bool plasmaRows = isShape && !isEdge && xMode;

    m_form->setRowVisible(m_name, isShape || p);
    m_form->setRowVisible(m_material, isShape && !isEdge);
    m_form->setRowVisible(m_eps, isShape && !isEdge && !plasma
                                     && s->material == Material::Dielectric);
    m_form->setRowVisible(m_epsHint, isShape && !isEdge);
    // Плазму показываем только в X-моде: H-ядро её не считает вовсе.
    m_form->setRowVisible(m_plasmaN, plasmaRows);
    // Величина из файла — только у прямоугольника: блок FILE_* берёт координаты
    // прямоугольника, файл хранит регулярную решётку.
    m_form->setRowVisible(m_plasmaNFileRow, plasmaRows && rect && m_plasmaN->isChecked());
    m_form->setRowVisible(m_plasmaB, plasmaRows);
    m_form->setRowVisible(m_plasmaBFileRow, plasmaRows && rect && m_plasmaB->isChecked());
    m_form->setRowVisible(m_plasmaBExact, plasmaRows && m_plasmaB->isChecked());
    m_form->setRowVisible(m_plasmaS, plasmaRows);
    m_form->setRowVisible(m_plasmaY, plasmaRows);
    m_form->setRowVisible(m_plasmaYFileRow, plasmaRows && rect && m_plasmaY->isChecked());
    m_form->setRowVisible(m_plasmaHint, plasmaRows);
    m_plasmaNValue->setEnabled(m_plasmaN->isChecked() && m_plasmaNFile->text().isEmpty());
    m_plasmaBValue->setEnabled(m_plasmaB->isChecked() && m_plasmaBFile->text().isEmpty());
    m_plasmaSValue->setEnabled(m_plasmaS->isChecked());
    m_plasmaYValue->setEnabled(m_plasmaY->isChecked() && m_plasmaYFile->text().isEmpty());

    m_form->setRowVisible(m_xMin, rect);
    m_form->setRowVisible(m_xMax, rect);
    m_form->setRowVisible(m_yMin, rect);
    m_form->setRowVisible(m_yMax, rect);

    m_form->setRowVisible(m_cx, circle);
    m_form->setRowVisible(m_cy, circle);
    m_form->setRowVisible(m_rMin, circle);
    m_form->setRowVisible(m_rMax, circle);
    m_form->setRowVisible(m_fiMin, circle);
    m_form->setRowVisible(m_fiMax, circle);

    m_form->setRowVisible(m_points, chain);

    m_form->setRowVisible(m_edgePresent, isEdge);
    m_form->setRowVisible(m_edgeCustom, isEdge);
    m_form->setRowVisible(m_edgeMaterial, isEdge && m_edgeCustom->isChecked());

    m_form->setRowVisible(m_portSide, p != nullptr);
    m_form->setRowVisible(m_portPlane, p != nullptr);
    m_form->setRowVisible(m_portFrom, p != nullptr);
    m_form->setRowVisible(m_portTo, p != nullptr);
    m_form->setRowVisible(m_portActive, p != nullptr);
    m_form->setRowVisible(m_portMode, p != nullptr && p->active);
    m_form->setRowVisible(m_portCutoff, p != nullptr);

    const bool canFillet = rect || (chain && m_sel.kind == Selection::ShapeVertex);
    m_form->setRowVisible(m_filletR->parentWidget(), canFillet);

    // ⚠️ «Соединить» здесь НЕ трогаем: его доступность зависит не от одиночного
    // выделения, а от набора, и решает её окно (setMergeEnabled).
    m_full->setEnabled(!m_sel.isEmpty());
    m_dup->setEnabled(!m_sel.isEmpty());
    m_del->setEnabled(!m_sel.isEmpty());

    // --- заголовок и подсказки ---
    if (s && isEdge) {
        m_title->setText(QStringLiteral("%1 — %2").arg(s->name, edgeTitle(m_sel.index)));
        m_kindHint->setText(tpl::tr("Стороны можно снимать и делать разными: программа тогда запишет "
            "прямоугольник отдельными блоками — одним RECT_STAT этого не выразить.",
            "Sides can be removed and made different: the program will then write the "
            "rectangle as separate blocks — a single RECT_STAT cannot express this."));
    } else if (s) {
        QString kind;
        switch (s->kind) {
        case ShapeKind::Rectangle: kind = tpl::tr("прямоугольник", "rectangle"); break;
        case ShapeKind::Circle:    kind = tpl::tr("круг / сектор", "circle / sector"); break;
        case ShapeKind::Polygon:   kind = tpl::tr("замкнутый контур", "closed polygon"); break;
        case ShapeKind::Polyline:  kind = tpl::tr("линия", "line"); break;
        }
        m_title->setText(QStringLiteral("%1 — %2").arg(s->name, kind));
        m_kindHint->setText(s->isContour()
                                ? tpl::tr("Материал задаёт КОНТУР: ядро создаёт только "
                                          "периметр, внутрь поле не проходит.",
                                          "The material defines a CONTOUR: the kernel creates only the perimeter, the "
                                          "field does not penetrate inside.")
                                : tpl::tr("Заливка по площади: в файл уйдёт добавка "
                                          "eps-1, а не сама eps.",
                                          "Area fill: the file gets the eps increment eps-1, not eps itself."));
    } else if (p) {
        m_title->setText(tpl::tr("%1 — порт", "%1 — port").arg(p->name));
        m_kindHint->setText(tpl::tr("Плоскость входа может стоять где угодно внутри области — ядро берёт её "
            "координату буквально. Направление падающей волны оно выводит само: "
            "по той половине области, в которую попала плоскость. "
            "Пассивный порт только принимает.",
            "The port plane can stand anywhere inside the domain — the kernel takes its "
            "coordinate literally. It derives the direction of the incident wave itself: "
            "from the half of the domain the plane falls into. A passive port only "
            "receives."));
    } else {
        m_title->setText(tpl::tr("Ничего не выбрано", "Nothing selected"));
        m_kindHint->setText(tpl::tr("Возьмите инструмент слева и рисуйте мышью. Или щёлкните по готовой фигуре, "
            "чтобы поправить её; по стороне прямоугольника — чтобы задать материал "
            "только этой стороне.",
            "Take a tool on the left and draw with the mouse. Or click an existing shape "
            "to adjust it; click a side of a rectangle to set the material for that side "
            "only."));
    }

    if (s && plasmaRows) {
        QString hint;
        if (!plasma) {
            hint = tpl::tr("Замагниченная плазма — это N и B в ОДНОЙ области: "
                           "отмечайте обе галочки у одной фигуры, рисовать её "
                           "дважды не нужно.",
                           "Magnetized plasma is N and B in ONE region: tick both check boxes on the same "
                           "shape, there is no need to draw it twice.");
        } else if (s->plasma.hasFormula()) {
            // У профиля одного значения нет — считать по нему частоты нечего.
            hint = tpl::tr("Величина задана формулой от координат: ядро посчитает её "
                           "в каждом узле, а редактор сохранит выражение как есть. "
                           "Частоты отсечки здесь не показать — у профиля они свои "
                           "в каждой точке.",
                           "The quantity is given by a formula of coordinates: the kernel evaluates it at "
                           "every node, and the editor keeps the expression as is. Cutoff frequencies "
                           "cannot be shown here — the profile has its own at each point.");
            if (s->plasma.customOrigin) {
                hint += tpl::tr("\nx и y в формуле отсчитываются от (%1; %2) мм.", "\nx and y in the formula are measured from (%1; %2) mm.")
                            .arg(s->plasma.origin.x()).arg(s->plasma.origin.y());
            }
        } else {
            const PlasmaBands pb = plasmaBands(s->plasma.hasN ? s->plasma.n : 0.0,
                                               s->plasma.hasB ? s->plasma.b : 0.0);
            hint = tpl::tr("f_p = %1 ГГц, f_ce = %2 ГГц. Отсечки X-моды: "
                           "f_L = %3, f_R = %4 ГГц.",
                           "f_p = %1 GHz, f_ce = %2 GHz. X-mode cutoffs: f_L = %3, f_R = %4 GHz.")
                       .arg(pb.fp, 0, 'f', 3).arg(pb.fce, 0, 'f', 3)
                       .arg(pb.fL, 0, 'f', 3).arg(pb.fR, 0, 'f', 3);
            if (!s->plasma.hasN) {
                hint += tpl::tr("\nБез концентрации N подмагничивание не подействует: "
                                "плазменная ветка расчёта включается только при N != 0.",
                                "\nWithout density N the magnetization has no effect: the plasma branch of the "
                                "computation turns on only when N != 0.");
            }
            if (s->plasma.hasB && !s->plasma.bExact) {
                hint += tpl::tr("\nB программа запишет прямоугольником на два шага шире "
                                "плазмы: на границе блока ядро ополовинивает значение, и "
                                "B ровно по плазме дало бы на её краю ложный резонанс.",
                                "\nThe program writes B as a rectangle two steps wider than the plasma: at the "
                                "block boundary the kernel halves the value, and B exactly over the plasma "
                                "would give a false resonance at its edge.");
            }
        }
        m_plasmaHint->setText(hint);
    }

    if (s && !isEdge) {
        if (plasma) {
            m_epsHint->setText(tpl::tr("Плазменный блок: материал не задаётся, "
                                       "область считается заливкой.",
                                       "Plasma block: no material is set, the region is treated as a fill."));
        } else if (s->material == Material::Dielectric) {
            m_epsHint->setText(tpl::tr("В файл уйдёт добавка eps-1 = %1.", "The file gets the eps increment eps-1 = %1.")
                                   .arg(s->eps - 1.0, 0, 'g', 4));
        } else {
            m_epsHint->setText(tpl::tr("Металл, магнитная стенка и поглотитель "
                                       "рисуются контуром.",
                                       "Metal, magnetic wall and absorber are drawn as a contour."));
        }
    }
    if (s && chain)
        m_points->setText(tpl::tr("%1 — тяните мышью, Del убирает выделенную", "%1 — drag with the mouse, Del removes the selected one")
                              .arg(s->points.size()));

    if (p) {
        const double a = p->to - p->from;
        if (a > 0.0) {
            m_portCutoff->setText(tpl::tr("Апертура %1 мм: одномодовый диапазон "
                                          "%2...%3 ГГц.",
                                          "Aperture %1 mm: single-mode band %2...%3 GHz.")
                                      .arg(a, 0, 'g', 5)
                                      .arg(299.792458 / (2.0 * a), 0, 'f', 3)
                                      .arg(299.792458 / a, 0, 'f', 3));
        } else {
            m_portCutoff->setText(tpl::tr("Апертура нулевая — порт не примут.", "Zero aperture — the port will be rejected."));
        }
    }
}

void PropertyPanel::syncFromProject()
{
    // Правка идёт из этой же панели — значения в полях уже верные, переписывать их
    // нельзя: собьётся набираемое число.
    if (m_pushing) {
        updateDerived();
        return;
    }

    m_loading = true;
    const Shape *s = shape();
    const Port *p = port();

    if (s) {
        m_name->setText(s->name);
        m_material->setCurrentIndex(m_material->findData(int(s->material)));
        m_eps->setValue(s->eps);
        m_plasmaN->setChecked(s->plasma.hasN);
        m_plasmaNValue->setText(plasmaFieldText(s->plasma.nFormula, s->plasma.n));
        m_plasmaNFile->setText(s->plasma.nFile);
        m_plasmaB->setChecked(s->plasma.hasB);
        m_plasmaBValue->setText(plasmaFieldText(s->plasma.bFormula, s->plasma.b));
        m_plasmaBFile->setText(s->plasma.bFile);
        m_plasmaBExact->setChecked(s->plasma.bExact);
        m_plasmaS->setChecked(s->plasma.hasS);
        m_plasmaSValue->setText(plasmaFieldText(s->plasma.sFormula, s->plasma.s));
        m_plasmaY->setChecked(s->plasma.hasY);
        m_plasmaYValue->setText(plasmaFieldText(s->plasma.yFormula, s->plasma.y));
        m_plasmaYFile->setText(s->plasma.yFile);
        m_xMin->setValue(s->xMin);
        m_xMax->setValue(s->xMax);
        m_yMin->setValue(s->yMin);
        m_yMax->setValue(s->yMax);
        m_cx->setValue(s->center.x());
        m_cy->setValue(s->center.y());
        m_rMin->setValue(s->rMin);
        m_rMax->setValue(s->rMax);
        m_fiMin->setValue(s->fiMin * 180.0 / M_PI);
        m_fiMax->setValue(s->fiMax * 180.0 / M_PI);

        if (m_sel.kind == Selection::ShapeEdge && m_sel.index >= 0 && m_sel.index < 4) {
            const EdgeSpec &e = s->edges[size_t(m_sel.index)];
            m_edgePresent->setChecked(e.present);
            m_edgeCustom->setChecked(e.custom);
            m_edgeMaterial->setCurrentIndex(m_edgeMaterial->findData(int(e.custom ? e.material
                                                                                  : s->material)));
        }
    } else if (p) {
        m_name->setText(p->name);
        m_portSide->setCurrentIndex(m_portSide->findData(int(p->side)));
        m_portPlane->setValue(portPlane(*p, m_project->box));
        m_portFrom->setValue(p->from);
        m_portTo->setValue(p->to);
        m_portActive->setCurrentIndex(p->active ? 0 : 1);
        m_portMode->setCurrentIndex(m_portMode->findData(int(p->mode)));
    }

    updateDerived();
    m_loading = false;
}

void PropertyPanel::pushToProject()
{
    if (m_loading || m_pushing || !m_project)
        return;

    m_pushing = true;
    emit aboutToEdit();

    if (Shape *s = shape()) {
        if (m_sel.kind == Selection::ShapeEdge && m_sel.index >= 0 && m_sel.index < 4) {
            EdgeSpec &e = s->edges[size_t(m_sel.index)];
            e.present = m_edgePresent->isChecked();
            e.custom = m_edgeCustom->isChecked();
            e.material = Material(m_edgeMaterial->currentData().toInt());
        } else {
            s->name = m_name->text();
            const auto mat = Material(m_material->currentData().toInt());
            // Открытая ломаная с заливкой невозможна: выражение eps ядро всегда
            // трактует как замкнутую область. Молча замыкаем, а не ругаемся.
            if (s->kind == ShapeKind::Polyline && mat == Material::Dielectric)
                s->kind = ShapeKind::Polygon;
            s->material = mat;
            s->eps = m_eps->value();
            s->plasma.hasN = m_plasmaN->isChecked();
            // В поле может стоять и число, и выражение от координат: ядро принимает
            // на этом месте и то и другое.
            setPlasmaField(m_plasmaNValue->text(), &s->plasma.n, &s->plasma.nFormula);
            // Файл имеет смысл только у прямоугольника: FILE_* берёт его координаты.
            const bool rectShape = (s->kind == ShapeKind::Rectangle);
            s->plasma.nFile = rectShape ? m_plasmaNFile->text().trimmed() : QString();
            s->plasma.hasB = m_plasmaB->isChecked();
            setPlasmaField(m_plasmaBValue->text(), &s->plasma.b, &s->plasma.bFormula);
            s->plasma.bFile = rectShape ? m_plasmaBFile->text().trimmed() : QString();
            s->plasma.bExact = m_plasmaBExact->isChecked();
            s->plasma.hasS = m_plasmaS->isChecked();
            setPlasmaField(m_plasmaSValue->text(), &s->plasma.s, &s->plasma.sFormula);
            s->plasma.hasY = m_plasmaY->isChecked();
            setPlasmaField(m_plasmaYValue->text(), &s->plasma.y, &s->plasma.yFormula);
            s->plasma.yFile = rectShape ? m_plasmaYFile->text().trimmed() : QString();
            s->xMin = m_xMin->value();
            s->xMax = m_xMax->value();
            s->yMin = m_yMin->value();
            s->yMax = m_yMax->value();
            s->center = QPointF(m_cx->value(), m_cy->value());
            s->rMin = m_rMin->value();
            s->rMax = m_rMax->value();
            s->fiMin = m_fiMin->value() * M_PI / 180.0;
            s->fiMax = m_fiMax->value() * M_PI / 180.0;
        }
    } else if (Port *p = port()) {
        p->name = m_name->text();
        const auto side = PortSide(m_portSide->currentData().toInt());
        const bool alongY = portAlongY(side);
        // Смена оси разворачивает апертуру: держим вход там же, где он был.
        if (alongY != portAlongY(p->side)) {
            const double mid = (p->from + p->to) / 2.0;
            const double width = p->to - p->from;
            const double oldPlane = portPlane(*p, m_project->box);
            p->from = oldPlane - width / 2.0;
            p->to = oldPlane + width / 2.0;
            p->plane = mid;
        } else {
            p->plane = m_portPlane->value();
            p->from = m_portFrom->value();
            p->to = m_portTo->value();
        }
        // Направление ядро выводит по половине области — держим их в согласии,
        // иначе оно молча развернёт падающую волну.
        p->side = directionForPlane(alongY, *p->plane, m_project->box);
        p->active = (m_portActive->currentData().toInt() == 1);
        p->mode = PortMode(m_portMode->currentData().toInt());
    }

    emit edited();
    m_pushing = false;
    updateDerived();
}
