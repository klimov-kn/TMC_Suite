/// @file shapedialog.cpp
/// @brief Реализация окна «Свойства фигуры» `ShapeDialog`.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-dialogs
#include "shapedialog.h"

#include "expr.h"
#include "dtheme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include "lang.h"

using namespace tpl;

namespace {

/// @brief Создаёт поле длины в мм (диапазон ±100000, три знака).
/// @param[in] v  начальное значение, мм.
/// @return Новое поле без родителя.
QDoubleSpinBox *mmSpin(double v = 0.0)
{
    auto *s = new QDoubleSpinBox;
    s->setRange(-100000, 100000);
    s->setDecimals(3);
    s->setSuffix(tpl::tr(" мм", " mm"));
    s->setValue(v);
    return s;
}

} // namespace

ShapeDialog::ShapeDialog(const Shape &s, Kernel kernel, QWidget *parent)
    : QDialog(parent), m_shape(s), m_kernel(kernel)
{
    setWindowTitle(tpl::tr("Свойства фигуры", "Shape properties"));
    // Ширина под самое длинное поле — строку формулы: она бывает в сотню символов,
    // и читать её через горизонтальную прокрутку невозможно.
    setMinimumWidth(560);
    setSizeGripEnabled(true);

    // Содержимое живёт в прокрутке: на ноутбучном экране форма целиком не влезает,
    // а окно без видимых кнопок «Применить» — неработающее окно.
    auto *content = new QWidget;
    auto *root = new QVBoxLayout(content);
    root->setContentsMargins(0, 0, 0, 0);

    auto *general = new QGroupBox(tpl::tr("Общее", "General"));
    auto *gf = new QFormLayout(general);
    m_name = new QLineEdit;
    m_kind = new QComboBox;
    m_kind->addItem(tpl::tr("Прямоугольник", "Rectangle"), int(ShapeKind::Rectangle));
    m_kind->addItem(tpl::tr("Окружность / сектор", "Circle / sector"), int(ShapeKind::Circle));
    m_kind->addItem(tpl::tr("Замкнутый контур", "Closed polygon"), int(ShapeKind::Polygon));
    m_kind->addItem(tpl::tr("Линия (ломаная)", "Line (polyline)"), int(ShapeKind::Polyline));
    gf->addRow(tpl::tr("Название:", "Name:"), m_name);
    gf->addRow(tpl::tr("Тип:", "Type:"), m_kind);
    root->addWidget(general);

    auto *mat = new QGroupBox(tpl::tr("Материал", "Material"));
    auto *mf = new QFormLayout(mat);
    m_material = new QComboBox;
    m_material->addItem(tpl::tr("Металл (стенка)", "Metal (wall)"), int(Material::Metal));
    m_material->addItem(tpl::tr("Магнитная стенка", "Magnetic wall"), int(Material::Magnetic));
    m_material->addItem(tpl::tr("Поглотитель", "Absorber"), int(Material::Absorber));
    m_material->addItem(tpl::tr("Диэлектрик", "Dielectric"), int(Material::Dielectric));
    mf->addRow(tpl::tr("Материал:", "Material:"), m_material);

    // eps <= 0 — законный расчёт (запредельная среда), запрещать его нечем.
    m_eps = new QDoubleSpinBox;
    m_eps->setRange(-10000, 10000);
    m_eps->setDecimals(4);
    m_eps->setValue(1.0);
    mf->addRow(tpl::tr("Проницаемость eps:", "Permittivity eps:"), m_eps);

    m_epsHint = new QLabel;
    m_epsHint->setProperty("mute", true);
    m_epsHint->setWordWrap(true);
    mf->addRow(QString(), m_epsHint);

    // --- неоднородная eps ---
    //
    // Формулу вводит пользователь, но пишет он САМУ eps: минус единицу для файла
    // ставит эмиттер. Так же, как в поле выше и как везде в программе — ловушка
    // «пишется добавка» не должна вылезать в интерфейс ни разу.
    m_epsFormulaOn = new QCheckBox(tpl::tr("Задать eps формулой (неоднородная среда)", "Set eps by formula (inhomogeneous medium)"));
    mf->addRow(m_epsFormulaOn);

    // Второй источник неоднородной среды: готовое распределение из файла .eps.
    // Считает его не редактор — либо само ядро на прошлом расчёте, либо сторонняя
    // программа со своей физикой (так сделан плазменный слой вокруг гиперзвукового
    // аппарата в samples/error). Формулы у такой среды нет вовсе, поэтому это
    // переключатель, а не ещё одно поле.
    m_epsFromFile = new QCheckBox(tpl::tr("Взять готовое распределение из файла .eps", "Take a ready distribution from an .eps file"));
    mf->addRow(m_epsFromFile);

    m_epsFormula = new QLineEdit;
    m_epsFormula->setPlaceholderText(tpl::tr("например  2+2*exp(-(x*x+y*y)/400)", "e.g.  2+2*exp(-(x*x+y*y)/400)"));
    mf->addRow(QStringLiteral("eps(x, y) ="), m_epsFormula);

    // Файл — ОТДЕЛЬНОЕ желание, а не часть формулы. Формула считается в любой фигуре,
    // а сохранить посчитанное умеет только прямоугольник. Пустое поле = файла не надо.
    // При чтении из файла поле меняет смысл на противоположный, и подпись вместе с ним.
    m_epsFile = new QLineEdit;
    m_epsFile->setPlaceholderText(tpl::tr("пусто — файл не создавать", "empty: do not create a file"));
    m_epsFileLabel = new QLabel(tpl::tr("Сохранить в файл:", "Save to file:"));
    mf->addRow(m_epsFileLabel, m_epsFile);

    // Начало отсчёта координат формулы. Обычно центр фигуры, но в авторских заданиях
    // оно сплошь и рядом стоит в стороне (центр линзы, ось сосуда), и отобрать эту
    // возможность значило бы молча сдвинуть среду при первом же сохранении.
    m_epsOriginOn = new QCheckBox(tpl::tr("Отсчитывать x и y не от центра фигуры", "Measure x and y not from the shape center"));
    mf->addRow(m_epsOriginOn);

    auto *originRow = new QWidget;
    auto *originLay = new QHBoxLayout(originRow);
    originLay->setContentsMargins(0, 0, 0, 0);
    m_epsOriginX = new QDoubleSpinBox;
    m_epsOriginY = new QDoubleSpinBox;
    for (QDoubleSpinBox *b : {m_epsOriginX, m_epsOriginY}) {
        b->setRange(-100000.0, 100000.0);
        b->setDecimals(3);
        b->setSuffix(tpl::tr(" мм", " mm"));
    }
    originLay->addWidget(new QLabel(QStringLiteral("x")));
    originLay->addWidget(m_epsOriginX, 1);
    originLay->addWidget(new QLabel(QStringLiteral("y")));
    originLay->addWidget(m_epsOriginY, 1);
    mf->addRow(tpl::tr("Начало координат:", "Coordinate origin:"), originRow);
    m_epsOriginRow = originRow;

    m_epsFormulaHint = new QLabel;
    m_epsFormulaHint->setProperty("mute", true);
    m_epsFormulaHint->setWordWrap(true);
    mf->addRow(QString(), m_epsFormulaHint);
    root->addWidget(mat);

    auto *plasma = new QGroupBox(tpl::tr("Плазма (только X-мода)", "Plasma (X-mode only)"));
    auto *pf = new QFormLayout(plasma);
    // Три величины в ОДНОЙ фигуре: замагниченная плазма — это N и B в одной области.
    // Значения полями ввода, а не спинбоксами: концентрация меряется в 1e17, и
    // набирать столько порядков колёсиком мучение.
    m_plasmaN = new QCheckBox(tpl::tr("Концентрация N, м^-3", "Density N, m^-3"));
    m_plasmaNValue = new QLineEdit(QStringLiteral("0"));
    pf->addRow(m_plasmaN, m_plasmaNValue);
    // Величина из файла распределения (FILE_N, FILE_B, FILE_Y — ядро X от 26.10.2026).
    // Файл пишет само ядро, если в поле величины написать [имя] перед значением.
    const auto fileEdit = [] {
        auto *e = new QLineEdit;
        e->setPlaceholderText(tpl::tr("пусто — берём значение выше", "empty: use the value above"));
        e->setToolTip(tpl::tr("Имя файла .eps рядом с заданием, без расширения. Только "
                              "у прямоугольника: файл хранит регулярную решётку.",
                              "Name of the .eps file next to the task, without extension. Rectangle only: "
                              "the file holds a regular lattice."));
        return e;
    };
    m_plasmaNFile = fileEdit();
    pf->addRow(tpl::tr("N из файла:", "N from file:"), m_plasmaNFile);
    m_plasmaB = new QCheckBox(tpl::tr("Подмагничивание B, Тл", "Magnetization B, T"));
    m_plasmaBValue = new QLineEdit(QStringLiteral("0"));
    pf->addRow(m_plasmaB, m_plasmaBValue);
    m_plasmaBFile = fileEdit();
    pf->addRow(tpl::tr("B из файла:", "B from file:"), m_plasmaBFile);
    m_plasmaBExact = new QCheckBox(tpl::tr("B ровно по плазме (без запаса)", "B exactly over the plasma (no margin)"));
    m_plasmaBExact->setToolTip(tpl::tr("Обычно блок B пишется на два шага шире плазмы: на границе блока ядро "
        "ополовинивает значение. Ровно по плазме — если краевой слой с полным B "
        "резонирует внутри полосы (об этом скажет проверка).",
        "Normally the B block is written two steps wider than the plasma: at the block "
        "edge the kernel halves the value. Exactly over the plasma: use it if the edge "
        "layer with full B resonates inside the band (the check will tell you)."));
    pf->addRow(QString(), m_plasmaBExact);
    m_plasmaS = new QCheckBox(tpl::tr("Потери: проводимость σ, См/м", "Losses: conductivity σ, S/m"));
    m_plasmaS->setToolTip(tpl::tr("Ядро пересчитывает σ в сеточные потери через шаг "
                                  "сетки — затухание от DELTA не зависит.",
                                  "The kernel converts σ to grid losses via the grid step, so the attenuation "
                                  "does not depend on DELTA."));
    m_plasmaSValue = new QLineEdit(QStringLiteral("0"));
    pf->addRow(m_plasmaS, m_plasmaSValue);
    m_plasmaY = new QCheckBox(tpl::tr("Параметр Y (сеточный — для опытных)", "Parameter Y (grid-based, for experts)"));
    m_plasmaY->setToolTip(tpl::tr("Y зависит от шага сетки: то же Y при вдвое меньшем "
                                  "DELTA даёт вдвое большее затухание. Обычно потери "
                                  "задают проводимостью σ.",
                                  "Y depends on the grid step: the same Y at half the DELTA gives twice the "
                                  "attenuation. Losses are usually set by the conductivity σ."));
    m_plasmaYValue = new QLineEdit(QStringLiteral("0"));
    pf->addRow(m_plasmaY, m_plasmaYValue);
    m_plasmaYFile = fileEdit();
    pf->addRow(tpl::tr("Y из файла:", "Y from file:"), m_plasmaYFile);
    m_plasmaHint = new QLabel(tpl::tr("Подмагничивание B действует только там, где перекрывается с областью N != 0: "
        "отмечайте обе галочки у одной фигуры. B программа запишет прямоугольником на два "
        "шага шире плазмы — на границе блока ядро ополовинивает значение.",
        "Magnetization B acts only where it overlaps the region with N != 0: tick both "
        "boxes on the same shape. The program writes B as a rectangle two steps wider "
        "than the plasma, since at the block edge the kernel halves the value."));
    m_plasmaHint->setProperty("mute", true);
    m_plasmaHint->setWordWrap(true);
    pf->addRow(QString(), m_plasmaHint);
    plasma->setEnabled(kernel == Kernel::X);
    root->addWidget(plasma);

    // --- геометрия ---
    m_geom = new QStackedWidget;

    auto *rectPage = new QWidget;
    auto *rf = new QFormLayout(rectPage);
    m_xMin = mmSpin();
    m_xMax = mmSpin();
    m_yMin = mmSpin();
    m_yMax = mmSpin();
    rf->addRow(tpl::tr("X от:", "X from:"), m_xMin);
    rf->addRow(tpl::tr("X до:", "X to:"), m_xMax);
    rf->addRow(tpl::tr("Y от:", "Y from:"), m_yMin);
    rf->addRow(tpl::tr("Y до:", "Y to:"), m_yMax);
    m_geom->addWidget(rectPage);

    auto *circPage = new QWidget;
    auto *cf = new QFormLayout(circPage);
    m_cx = mmSpin();
    m_cy = mmSpin();
    m_rMin = mmSpin(0.0);
    m_rMax = mmSpin(10.0);
    m_fiMin = new QDoubleSpinBox;
    m_fiMin->setRange(0, 360);
    m_fiMin->setSuffix(QStringLiteral(" °"));
    m_fiMax = new QDoubleSpinBox;
    m_fiMax->setRange(0, 360);
    m_fiMax->setSuffix(QStringLiteral(" °"));
    m_fiMax->setValue(360);
    cf->addRow(tpl::tr("Центр X:", "Center X:"), m_cx);
    cf->addRow(tpl::tr("Центр Y:", "Center Y:"), m_cy);
    cf->addRow(tpl::tr("Радиус внутр.:", "Inner radius:"), m_rMin);
    cf->addRow(tpl::tr("Радиус внеш.:", "Outer radius:"), m_rMax);
    cf->addRow(tpl::tr("Угол от:", "Angle from:"), m_fiMin);
    cf->addRow(tpl::tr("Угол до:", "Angle to:"), m_fiMax);
    m_geom->addWidget(circPage);

    auto *polyPage = new QWidget;
    auto *pl = new QVBoxLayout(polyPage);
    m_points = new QTableWidget(0, 2);
    m_points->setHorizontalHeaderLabels({tpl::tr("X, мм", "X, mm"), tpl::tr("Y, мм", "Y, mm")});
    m_points->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_points->verticalHeader()->setVisible(false);
    pl->addWidget(m_points);
    auto *row = new QHBoxLayout;
    auto *addPt = new QPushButton(tpl::tr("Добавить точку", "Add point"));
    auto *delPt = new QPushButton(tpl::tr("Удалить точку", "Delete point"));
    row->addWidget(addPt);
    row->addWidget(delPt);
    row->addStretch();
    pl->addLayout(row);
    m_polyHint = new QLabel;
    m_polyHint->setProperty("mute", true);
    m_polyHint->setWordWrap(true);
    pl->addWidget(m_polyHint);
    m_geom->addWidget(polyPage);

    connect(addPt, &QPushButton::clicked, this, [this] {
        const int r = m_points->rowCount();
        m_points->insertRow(r);
        m_points->setItem(r, 0, new QTableWidgetItem(QStringLiteral("0")));
        m_points->setItem(r, 1, new QTableWidgetItem(QStringLiteral("0")));
    });
    connect(delPt, &QPushButton::clicked, this, [this] {
        const int r = m_points->currentRow();
        if (r >= 0)
            m_points->removeRow(r);
    });

    auto *geomBox = new QGroupBox(tpl::tr("Размеры", "Dimensions"));
    auto *gl = new QVBoxLayout(geomBox);
    gl->addWidget(m_geom);
    m_edgeHint = new QLabel(tpl::tr("Отдельные стороны прямоугольника (снять сторону, сделать её магнитной "
        "стенкой или поглотителем) задаются прямо на холсте: щёлкните по нужной "
        "стороне и правьте её в панели справа.",
        "Individual sides of the rectangle (remove a side, make it a magnetic wall or "
        "an absorber) are set right on the canvas: click the side and edit it in the "
        "panel on the right."));
    m_edgeHint->setProperty("mute", true);
    m_edgeHint->setWordWrap(true);
    gl->addWidget(m_edgeHint);
    root->addWidget(geomBox);

    auto *move = new QGroupBox(tpl::tr("Движение", "Motion"));
    auto *vf = new QFormLayout(move);
    m_vx = new QDoubleSpinBox;
    m_vx->setRange(-1e6, 1e6);
    m_vx->setDecimals(4);
    m_vy = new QDoubleSpinBox;
    m_vy->setRange(-1e6, 1e6);
    m_vy->setDecimals(4);
    vf->addRow(tpl::tr("Скорость Vx:", "Velocity Vx:"), m_vx);
    vf->addRow(tpl::tr("Скорость Vy:", "Velocity Vy:"), m_vy);
    auto *moveHint = new QLabel(tpl::tr("Нулевая скорость — блок неподвижен. У движущихся блоков плазмы не бывает.", "Zero velocity means the block is fixed. Moving blocks cannot be plasma."));
    moveHint->setProperty("mute", true);
    moveHint->setWordWrap(true);
    vf->addRow(QString(), moveHint);
    root->addWidget(move);
    root->addStretch();

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(content);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText(tpl::tr("Применить", "Apply"));
    buttons->button(QDialogButtonBox::Ok)->setProperty("accent", true);
    buttons->button(QDialogButtonBox::Cancel)->setText(tpl::tr("Отмена", "Cancel"));
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *outer = new QVBoxLayout(this);
    outer->addWidget(scroll, 1);
    outer->addWidget(buttons);

    connect(m_kind, &QComboBox::currentIndexChanged, this, &ShapeDialog::onKindChanged);
    connect(m_material, &QComboBox::currentIndexChanged, this, &ShapeDialog::onMaterialChanged);
    for (auto *c : {m_plasmaN, m_plasmaB, m_plasmaS, m_plasmaY})
        connect(c, &QCheckBox::toggled, this, &ShapeDialog::onMaterialChanged);
    for (auto *e : {m_plasmaNFile, m_plasmaBFile, m_plasmaYFile})
        connect(e, &QLineEdit::textChanged, this, &ShapeDialog::onMaterialChanged);
    connect(m_epsFormulaOn, &QCheckBox::toggled, this, &ShapeDialog::onMaterialChanged);
    connect(m_epsFromFile, &QCheckBox::toggled, this, &ShapeDialog::onMaterialChanged);
    connect(m_epsFormula, &QLineEdit::textChanged, this, &ShapeDialog::updateFormulaHint);
    connect(m_epsFile, &QLineEdit::textChanged, this, &ShapeDialog::updateFormulaHint);
    connect(m_epsOriginOn, &QCheckBox::toggled, this, &ShapeDialog::onMaterialChanged);
    for (auto *b : {m_epsOriginX, m_epsOriginY})
        connect(b, &QDoubleSpinBox::valueChanged, this, &ShapeDialog::updateFormulaHint);
    // Границы прямоугольника входят в диапазон значений и в имя файла по умолчанию.
    for (auto *b : {m_xMin, m_xMax, m_yMin, m_yMax})
        connect(b, &QDoubleSpinBox::valueChanged, this, &ShapeDialog::updateFormulaHint);

    load(s);
    onKindChanged();
    onMaterialChanged();
    dtheme::fitToScreen(this);
}

void ShapeDialog::load(const Shape &s)
{
    m_name->setText(s.name);
    m_kind->setCurrentIndex(m_kind->findData(int(s.kind)));
    m_material->setCurrentIndex(m_material->findData(int(s.material)));
    m_eps->setValue(s.eps);
    m_epsFormulaOn->setChecked(s.epsProfile.enabled);
    m_epsFromFile->setChecked(s.epsProfile.fromFile);
    m_epsFormula->setText(s.epsProfile.formula);
    m_epsFile->setText(s.epsProfile.file);
    m_epsOriginOn->setChecked(s.epsProfile.customOrigin);
    // Пока своё начало не задано, показываем текущий центр — так видно, от чего
    // отсчёт идёт сейчас, и правку не приходится начинать с нуля.
    const QPointF o = s.epsOrigin();
    m_epsOriginX->setValue(s.epsProfile.customOrigin ? s.epsProfile.origin.x() : o.x());
    m_epsOriginY->setValue(s.epsProfile.customOrigin ? s.epsProfile.origin.y() : o.y());
    m_plasmaN->setChecked(s.plasma.hasN);
    m_plasmaNValue->setText(plasmaFieldText(s.plasma.nFormula, s.plasma.n));
    m_plasmaNFile->setText(s.plasma.nFile);
    m_plasmaB->setChecked(s.plasma.hasB);
    m_plasmaBValue->setText(plasmaFieldText(s.plasma.bFormula, s.plasma.b));
    m_plasmaBFile->setText(s.plasma.bFile);
    m_plasmaBExact->setChecked(s.plasma.bExact);
    m_plasmaS->setChecked(s.plasma.hasS);
    m_plasmaSValue->setText(plasmaFieldText(s.plasma.sFormula, s.plasma.s));
    m_plasmaY->setChecked(s.plasma.hasY);
    m_plasmaYValue->setText(plasmaFieldText(s.plasma.yFormula, s.plasma.y));
    m_plasmaYFile->setText(s.plasma.yFile);

    m_xMin->setValue(s.xMin);
    m_xMax->setValue(s.xMax);
    m_yMin->setValue(s.yMin);
    m_yMax->setValue(s.yMax);

    m_cx->setValue(s.center.x());
    m_cy->setValue(s.center.y());
    m_rMin->setValue(s.rMin);
    m_rMax->setValue(s.rMax);
    m_fiMin->setValue(s.fiMin * 180.0 / M_PI);
    m_fiMax->setValue(s.fiMax * 180.0 / M_PI);

    m_points->setRowCount(s.points.size());
    for (int i = 0; i < s.points.size(); ++i) {
        m_points->setItem(i, 0, new QTableWidgetItem(QString::number(s.points.at(i).x())));
        m_points->setItem(i, 1, new QTableWidgetItem(QString::number(s.points.at(i).y())));
    }

    m_vx->setValue(s.vx);
    m_vy->setValue(s.vy);
}

void ShapeDialog::onKindChanged()
{
    // Ломаная и замкнутый контур делят одну страницу со списком точек:
    // в языке это один и тот же примитив POLYGON_STAT.
    const auto kind = ShapeKind(m_kind->currentData().toInt());
    switch (kind) {
    case ShapeKind::Rectangle: m_geom->setCurrentIndex(0); break;
    case ShapeKind::Circle:    m_geom->setCurrentIndex(1); break;
    case ShapeKind::Polygon:
    case ShapeKind::Polyline:  m_geom->setCurrentIndex(2); break;
    }
    m_edgeHint->setVisible(kind == ShapeKind::Rectangle);
    m_polyHint->setText(kind == ShapeKind::Polyline
                            ? tpl::tr("Ломаная: последняя точка с первой НЕ соединяется. "
                                      "Нужно не меньше двух точек.",
                                      "Polyline: the last point is NOT joined to the first. At least two points are "
                                      "needed.")
                            : tpl::tr("Замкнутый контур. Для заливки диэлектриком нужно "
                                      "больше трёх вершин и выпуклая область — "
                                      "обход по часовой программа расставит сама.",
                                      "Closed polygon. Filling with dielectric needs more than three vertices and a "
                                      "convex region; the program sets the clockwise order itself."));
}

void ShapeDialog::onMaterialChanged()
{
    const bool isPlasma = m_plasmaN->isChecked() || m_plasmaB->isChecked()
                          || m_plasmaS->isChecked() || m_plasmaY->isChecked();
    const auto mat = Material(m_material->currentData().toInt());
    const bool profile = m_epsFormulaOn->isChecked();
    const bool isRect = (ShapeKind(m_kind->currentData().toInt()) == ShapeKind::Rectangle);

    // У плазменного блока материала нет — параметр там всегда числовой.
    m_material->setEnabled(!isPlasma && !profile);
    m_eps->setEnabled(!isPlasma && !profile && mat == Material::Dielectric);

    // Формула — для ЛЮБОЙ фигуры: `CIRCLE_STAT <выражение>` ядро считает наравне с
    // прямоугольником (авторский `data/cir_quas.h`, блок 20). Запрещена она только
    // вместе с плазмой: сложить их в одном блоке языку нечем.
    //
    // А вот запись в файл умеет один `RECT_STAT` — поэтому поле имени доступно только
    // прямоугольнику, и об этом же говорит подпись под ним.
    const bool isCircle = (ShapeKind(m_kind->currentData().toInt()) == ShapeKind::Circle);

    // Готовое распределение — только прямоугольником: сетка в файле .eps регулярная
    // (`#Xmin`/`#Ymin`/`#nX`/`#nY` и линейный индекс узла), «дырок» в ней не бывает,
    // и притворяться кругом было бы обманом — в задание всё равно уйдёт прямоугольник.
    const bool fromFile = m_epsFromFile->isChecked() && isRect;
    m_epsFromFile->setEnabled(profile && !isPlasma && isRect);
    if (!isRect)
        m_epsFromFile->setChecked(false);

    m_epsFormulaOn->setEnabled(!isPlasma);
    m_epsFormula->setEnabled(profile && !isPlasma && !fromFile);
    m_epsFile->setEnabled(profile && !isPlasma && isRect);
    m_epsFileLabel->setText(fromFile ? tpl::tr("Файл распределения:", "Distribution file:")
                                     : tpl::tr("Сохранить в файл:", "Save to file:"));
    m_epsFile->setPlaceholderText(fromFile
                                      ? tpl::tr("имя без расширения — ядро допишет .eps", "name without extension, the kernel adds .eps")
                                      : tpl::tr("пусто — файл не создавать", "empty: do not create a file"));
    if (!isRect)
        m_epsFile->clear();

    // Начало отсчёта у окружности задать нечем: анкер блока — это её центр.
    m_epsOriginOn->setEnabled(profile && !isPlasma && !isCircle);
    m_epsOriginRow->setEnabled(profile && !isPlasma && !isCircle
                               && m_epsOriginOn->isChecked());
    if (isCircle)
        m_epsOriginOn->setChecked(false);
    m_plasmaNValue->setEnabled(m_plasmaN->isChecked() && m_plasmaNFile->text().isEmpty());
    m_plasmaNFile->setEnabled(m_plasmaN->isChecked() && isRect);
    m_plasmaBValue->setEnabled(m_plasmaB->isChecked() && m_plasmaBFile->text().isEmpty());
    m_plasmaBFile->setEnabled(m_plasmaB->isChecked() && isRect);
    m_plasmaBExact->setEnabled(m_plasmaB->isChecked());
    m_plasmaSValue->setEnabled(m_plasmaS->isChecked());
    m_plasmaYValue->setEnabled(m_plasmaY->isChecked() && m_plasmaYFile->text().isEmpty());
    m_plasmaYFile->setEnabled(m_plasmaY->isChecked() && isRect);

    if (isPlasma) {
        m_epsHint->setText(tpl::tr("Плазменный блок: материал не задаётся.", "Plasma block: no material is set."));
    } else if (fromFile) {
        m_epsHint->setText(tpl::tr("eps берётся из готового файла — ни число выше, "
                                   "ни формула не используются.",
                                   "eps is taken from the ready file; neither the number above nor the formula is "
                                   "used."));
    } else if (profile) {
        m_epsHint->setText(tpl::tr("eps задана формулой — число ниже не используется.", "eps is set by a formula; the number below is not used."));
    } else if (mat == Material::Dielectric) {
        // Показываем то, что реально уйдёт в файл: eps=4 -> 3. Пользователь вводит eps.
        m_epsHint->setText(tpl::tr("В файл уйдёт добавка eps-1 = %1. Заливка по площади.", "The file will get the eps increment eps-1 = %1. Filled over the area.")
                               .arg(m_eps->value() - 1.0, 0, 'g', 4));
    } else {
        m_epsHint->setText(tpl::tr("Материал рисуется КОНТУРОМ: ядро создаёт только периметр, не заливку.", "The material is drawn as an OUTLINE: the kernel creates only the perimeter, not the fill."));
    }

    updateFormulaHint();
}

void ShapeDialog::updateFormulaHint()
{
    // Профиль из файла проверять нечем: что там за среда, знает только сам файл.
    // Зато можно честно сказать, чего ждать от ядра.
    if (m_epsFormulaOn->isChecked() && m_epsFromFile->isChecked()) {
        const QString file = m_epsFile->text().trimmed();
        m_epsFormulaHint->setText(
            file.isEmpty()
                ? tpl::tr("Укажите имя файла распределения (без расширения — ядро "
                          "допишет .eps). Файл должен лежать в папке задания, а шаг "
                          "его сетки — совпадать с DELTA расчёта.",
                          "Enter the distribution file name (without extension, the kernel adds .eps). "
                          "The file must be in the task folder, and its grid step must match the run "
                          "DELTA.")
                : tpl::tr("Среда будет прочитана из %1.eps. Прямоугольник задаёт только "
                          "то, что видно в редакторе: сетку и размер ядро берёт из "
                          "самого файла, а место на сцене — из начала координат ниже.",
                          "The medium will be read from %1.eps. The rectangle sets only what the editor "
                          "shows: the kernel takes the grid and size from the file itself, and the "
                          "position in the scene from the coordinate origin below.")
                      .arg(file));
        return;
    }

    if (!m_epsFormulaOn->isChecked()) {
        m_epsFormulaHint->setText(tpl::tr("Ядро посчитает формулу в каждом узле области. Доступны x и y (мм от начала "
            "координат), r и f (полярный угол), функции exp, sqrt, sin, cos, abs, log. "
            "Работает для любой фигуры; в файл .eps сохраняется только прямоугольник.",
            "The kernel evaluates the formula at every node of the region. Available: x "
            "and y (mm from the coordinate origin), r and f (polar angle), functions exp, "
            "sqrt, sin, cos, abs, log. Works for any shape; only a rectangle is saved to "
            "an .eps file."));
        return;
    }

    const QString formula = m_epsFormula->text().trimmed();
    if (formula.isEmpty()) {
        m_epsFormulaHint->setText(tpl::tr("Введите формулу.", "Enter a formula."));
        return;
    }

    // Проверяем ЗДЕСЬ ЖЕ, а не при генерации: ошибку в формуле надо показывать там,
    // где её набирают, иначе человек уйдёт со страницы и вернётся к ней через
    // непонятное сообщение в конце.
    const auto vars = tpl::epsVariables(0.0, 0.0);
    const tpl::ExprProgram prog = tpl::ExprProgram::compile(formula);
    if (!prog.valid()) {
        m_epsFormulaHint->setText(tpl::tr("Формулу разобрать не удалось — ядро тоже "
                                          "её не примет.",
                                          "The formula could not be parsed; the kernel will not accept it either."));
        return;
    }
    const QStringList unknown = prog.unknown(vars);
    if (!unknown.isEmpty()) {
        m_epsFormulaHint->setText(tpl::tr("Непонятные имена: %1. Частоты в формуле eps "
                                          "быть не может — ядро её туда не передаёт.",
                                          "Unknown names: %1. Frequency cannot appear in an eps formula: the kernel does "
                                          "not pass it there.")
                                      .arg(unknown.join(QStringLiteral(", "))));
        return;
    }

    // Диапазон по области — тот же, что уедет в шкалу на холсте. Точки берём внутри
    // ФИГУРЫ: у сектора углы описанного прямоугольника ядро не считает вовсе.
    const Shape probe = shape();
    bool any = false;
    int bad = 0;
    double lo = 0.0, hi = 0.0;
    for (const QPointF &pt : tpl::epsSamplePoints(probe)) {
        const auto v = prog.evalEps(pt.x(), pt.y());
        if (!v) {
            ++bad;
            continue;
        }
        lo = any ? std::min(lo, *v) : *v;
        hi = any ? std::max(hi, *v) : *v;
        any = true;
    }

    if (!any) {
        m_epsFormulaHint->setText(tpl::tr("Формула не даёт числа ни в одной точке "
                                          "области.",
                                          "The formula gives no number at any point of the region."));
        return;
    }

    QString text = tpl::tr("eps в области: от %1 до %2.", "eps in the region: from %1 to %2.")
                       .arg(lo, 0, 'g', 4).arg(hi, 0, 'g', 4);
    if (bad > 0) {
        text += tpl::tr(" В %1 точках числа не получается (деление на ноль, корень из "
                        "отрицательного) — там среда выйдет неопределённой.",
                        " At %1 points no number is obtained (division by zero, root of a negative); "
                        "the medium will be undefined there.").arg(bad);
    }
    const QString file = tpl::epsProfileFile(probe);
    if (!file.isEmpty() && probe.kind == ShapeKind::Rectangle) {
        text += tpl::tr(" Профиль сохранится в %1.eps на первом шаге расчёта.", " The profile will be saved to %1.eps at the first step of the run.").arg(file);
    } else if (probe.kind != ShapeKind::Rectangle) {
        text += tpl::tr(" Файл .eps ядро для этой формы не пишет — считает, но не "
                        "сохраняет.",
                        " The kernel does not write an .eps file for this shape: it computes but does "
                        "not save.");
    }
    m_epsFormulaHint->setText(text);
}

Shape ShapeDialog::shape() const
{
    Shape s = m_shape;
    s.name = m_name->text();
    s.kind = ShapeKind(m_kind->currentData().toInt());
    s.material = Material(m_material->currentData().toInt());
    s.eps = m_eps->value();
    s.epsProfile.enabled = m_epsFormulaOn->isChecked();
    s.epsProfile.fromFile = m_epsFromFile->isChecked() && s.kind == ShapeKind::Rectangle;
    s.epsProfile.formula = m_epsFormula->text().trimmed();
    s.epsProfile.file = m_epsFile->text().trimmed();
    // У окружности анкер — это её центр, другого места для него в языке нет.
    s.epsProfile.customOrigin = m_epsOriginOn->isChecked() && s.kind != ShapeKind::Circle;
    s.epsProfile.origin = QPointF(m_epsOriginX->value(), m_epsOriginY->value());
    s.plasma.hasN = m_plasmaN->isChecked();
    // В поле может стоять и число, и выражение от координат: ядро принимает на этом
    // месте и то и другое.
    setPlasmaField(m_plasmaNValue->text(), &s.plasma.n, &s.plasma.nFormula);
    // Файл имеет смысл только у прямоугольника: FILE_* берёт его координаты.
    const bool rectShape = (s.kind == ShapeKind::Rectangle);
    s.plasma.nFile = rectShape ? m_plasmaNFile->text().trimmed() : QString();
    s.plasma.hasB = m_plasmaB->isChecked();
    setPlasmaField(m_plasmaBValue->text(), &s.plasma.b, &s.plasma.bFormula);
    s.plasma.bFile = rectShape ? m_plasmaBFile->text().trimmed() : QString();
    s.plasma.bExact = m_plasmaBExact->isChecked();
    s.plasma.hasS = m_plasmaS->isChecked();
    setPlasmaField(m_plasmaSValue->text(), &s.plasma.s, &s.plasma.sFormula);
    s.plasma.hasY = m_plasmaY->isChecked();
    setPlasmaField(m_plasmaYValue->text(), &s.plasma.y, &s.plasma.yFormula);
    s.plasma.yFile = rectShape ? m_plasmaYFile->text().trimmed() : QString();

    s.xMin = m_xMin->value();
    s.xMax = m_xMax->value();
    s.yMin = m_yMin->value();
    s.yMax = m_yMax->value();

    s.center = QPointF(m_cx->value(), m_cy->value());
    s.rMin = m_rMin->value();
    s.rMax = m_rMax->value();
    s.fiMin = m_fiMin->value() * M_PI / 180.0;
    s.fiMax = m_fiMax->value() * M_PI / 180.0;

    s.points.clear();
    for (int i = 0; i < m_points->rowCount(); ++i) {
        const auto *xi = m_points->item(i, 0);
        const auto *yi = m_points->item(i, 1);
        if (xi && yi)
            s.points << QPointF(xi->text().toDouble(), yi->text().toDouble());
    }

    s.vx = m_vx->value();
    s.vy = m_vy->value();
    return s;
}
