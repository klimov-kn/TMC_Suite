/// @file runplanpage.cpp
/// @brief Реализация страницы «Что считать» (`RunPlanPage`): возбуждения, варианты, угловой свип.
///
/// Часть интерфейса редактора заданий.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-run
#include "runplanpage.h"

#include "dtheme.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include "lang.h"

using namespace tpl;

RunPlanPage::RunPlanPage(QWidget *parent)
    : QWidget(parent)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    // --------------------------------------------------------- возбуждения
    auto *exc = new QGroupBox(tpl::tr("Возбуждение портов", "Port excitation"));
    m_excGroup = exc;
    auto *el = new QVBoxLayout(exc);
    auto *eh = new QLabel(tpl::tr("Выбирать приходится входы, а не отдельные S-параметры: один прогон даёт сразу "
        "ЦЕЛЫЙ СТОЛБЕЦ матрицы — возбудили вход 1, получили и S11, и S21. Взять из "
        "столбца что-то одно нечем. Отметьте входы, и программа запишет по шагу на "
        "каждый: матрица соберётся в один файл .s за ОДИН прогон.",
        "You choose ports, not individual S-parameters: one run gives a WHOLE COLUMN "
        "of the matrix at once: excite port 1 and you get both S11 and S21. There is "
        "no way to take just one element of a column. Tick the ports, and the program "
        "writes one step for each: the matrix is collected into one .s file in ONE run."));
    eh->setProperty("mute", true);
    eh->setWordWrap(true);
    el->addWidget(eh);

    m_ports = new QListWidget;
    m_ports->setMaximumHeight(150);
    // Строка входа длинная («возбудить вход 1 «плоская волна»»): переносим, а не режем.
    m_ports->setWordWrap(true);
    m_ports->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    el->addWidget(m_ports);
    m_portsHint = new QLabel;
    m_portsHint->setWordWrap(true);
    el->addWidget(m_portsHint);

    // ------------------------------------------------- поворот объекта (ЭПР)
    auto *sw = new QGroupBox(tpl::tr("Поворот объекта по шагам (ЭПР)", "Object rotation by steps (RCS)"));
    sw->setObjectName(QStringLiteral("sweepGroup"));
    auto *sl = new QVBoxLayout(sw);
    m_sweepOn = new QCheckBox(tpl::tr("Поворачивать объект: на каждом шаге — следующий угол", "Rotate the object: the next angle at each step"));
    m_sweepOn->setObjectName(QStringLiteral("sweepOn"));
    sl->addWidget(m_sweepOn);
    auto *sh = new QLabel(tpl::tr("Так снимают ЭПР по углу. Источник (плоская волна) стоит на месте, а объект на каждом "
        "шаге поворачивается на следующий угол вокруг оси. Нужны: один активный вход и одна "
        "частота. В файле .s каждая точка подписана своим углом.",
        "This is how RCS versus angle is measured. The source (a plane wave) stays in "
        "place, and at each step the object is rotated to the next angle about the "
        "axis. Required: one active port and one frequency. In the .s file each point "
        "is labeled with its angle."));
    sh->setProperty("mute", true);
    sh->setWordWrap(true);
    sl->addWidget(sh);
    // Список фигур — во всю ширину: имена должны читаться целиком, иначе не понять,
    // что именно поворачивается.
    auto *objLabel = new QLabel(tpl::tr("Объект — отмеченные фигуры (поворачиваются вместе):", "Object: the ticked shapes (rotate together):"));
    objLabel->setProperty("dim", true);
    sl->addWidget(objLabel);
    m_sweepShapes = new QListWidget;
    m_sweepShapes->setMinimumHeight(90);
    m_sweepShapes->setMaximumHeight(130);
    m_sweepShapes->setToolTip(tpl::tr("Отмеченные фигуры — это объект: они поворачиваются вместе", "The ticked shapes form the object: they rotate together"));
    sl->addWidget(m_sweepShapes);
    // Фабрика полей углов и оси: диапазон, знаки, единицы, пересчёт по готовому числу.
    const auto spin = [](double lo, double hi, int dec, const QString &suffix) {
        auto *s = new QDoubleSpinBox;
        s->setRange(lo, hi);
        s->setDecimals(dec);
        s->setSuffix(suffix);
        s->setKeyboardTracking(false);
        // Без потолка счётчик растягивается под «-1000000,000 мм» и выдавливает
        // соседний блок возбуждений до нечитаемой ширины.
        s->setMaximumWidth(160);
        return s;
    };
    m_pivotX = spin(-1e6, 1e6, 3, tpl::tr(" мм", " mm"));
    m_pivotY = spin(-1e6, 1e6, 3, tpl::tr(" мм", " mm"));
    m_pivotCenter = new QCheckBox(tpl::tr("в центре объекта, едет вместе с ним", "at the object center, moves with it"));
    m_pivotCenter->setToolTip(tpl::tr("Ось — центр габарита отмеченных фигур. Двигаете объект по холсту — ось идёт следом. "
        "Снимите галочку, чтобы задать ось числами.",
        "The axis is the center of the bounding box of the ticked shapes. Move the "
        "object on the canvas and the axis follows. Untick to set the axis by numbers."));
    m_angFrom = spin(-360, 360, 2, tpl::tr(" град.", " deg"));
    m_angTo = spin(-360, 720, 2, tpl::tr(" град.", " deg"));
    m_angStep = spin(0.1, 180, 2, tpl::tr(" град.", " deg"));
    // Ось и углы — компактной сеткой под списком.
    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(8);
    grid->addWidget(new QLabel(tpl::tr("Ось поворота:", "Rotation axis:")), 0, 0);
    grid->addWidget(m_pivotCenter, 0, 1, 1, 5);
    grid->addWidget(new QLabel(QStringLiteral("X:")), 1, 0, Qt::AlignRight);
    grid->addWidget(m_pivotX, 1, 1);
    grid->addWidget(new QLabel(QStringLiteral("Y:")), 1, 2, Qt::AlignRight);
    grid->addWidget(m_pivotY, 1, 3);
    grid->addWidget(new QLabel(tpl::tr("Углы от:", "Angles from:")), 2, 0);
    grid->addWidget(m_angFrom, 2, 1);
    grid->addWidget(new QLabel(tpl::tr("до:", "to:")), 2, 2, Qt::AlignRight);
    grid->addWidget(m_angTo, 2, 3);
    grid->addWidget(new QLabel(tpl::tr("шаг:", "step:")), 2, 4, Qt::AlignRight);
    grid->addWidget(m_angStep, 2, 5);
    grid->setColumnStretch(6, 1);
    sl->addLayout(grid);
    m_sweepRef = new QCheckBox(tpl::tr("Первый шаг — та же сцена без объекта (опора для вычитания)", "First step: the same scene without the object (reference for subtraction)"));
    sl->addWidget(m_sweepRef);
    m_sweepHint = new QLabel;
    m_sweepHint->setWordWrap(true);
    sl->addWidget(m_sweepHint);

    auto *top = new QHBoxLayout;
    top->addWidget(exc, 1);
    top->addWidget(sw, 1);
    root->addLayout(top);

    // ------------------------------------------------------------- варианты
    auto *var = new QGroupBox(tpl::tr("Варианты геометрии", "Geometry variants"));
    m_varGroup = var;
    auto *vl = new QVBoxLayout(var);
    auto *vh = new QLabel(tpl::tr("Вариант — это ОДИН расчёт по вашему чертежу, но с частью фигур. Типовой случай: "
        "вариант «пустой» — только стенки волновода, вариант «с заполнением» — стенки "
        "плюс диэлектрик. Тогда за один прогон видно, что даёт вставка.\n"
        "Галочка = фигура участвует в этом варианте. Стенки нужно оставлять включёнными "
        "ВЕЗДЕ: без них считается пустая коробка, а не волновод.",
        "A variant is ONE run of your drawing, but with only some of the shapes. "
        "Typical case: an \"empty\" variant (only the waveguide walls) and a "
        "\"filled\" variant (walls plus dielectric). Then one run shows what the "
        "insert does.\nTick = the shape takes part in this variant. Walls must stay "
        "enabled EVERYWHERE: without them an empty box is computed, not a waveguide."));
    vh->setProperty("mute", true);
    vh->setWordWrap(true);
    vl->addWidget(vh);

    m_preset = new QPushButton(tpl::tr("Заготовка: «пустой» + «с заполнением»", "Preset: \"empty\" + \"filled\""));
    m_preset->setToolTip(tpl::tr("Создаёт два варианта: в первом выключены все заливки (диэлектрик, плазма), "
        "во втором включено всё. Контуры — стенки — остаются в обоих.",
        "Creates two variants: in the first all fills (dielectric, plasma) are off, in "
        "the second everything is on. Outlines (walls) stay in both."));
    vl->addWidget(m_preset);

    auto *row = new QHBoxLayout;
    auto *left = new QVBoxLayout;
    auto *lt = new QLabel(tpl::tr("Варианты", "Variants"));
    lt->setProperty("dim", true);
    left->addWidget(lt);
    m_variants = new QListWidget;
    left->addWidget(m_variants, 1);
    auto *btns = new QHBoxLayout;
    m_add = new QPushButton(tpl::tr("Добавить", "Add"));
    m_remove = new QPushButton(tpl::tr("Удалить", "Delete"));
    btns->addWidget(m_add);
    btns->addWidget(m_remove);
    left->addLayout(btns);
    row->addLayout(left, 1);

    auto *right = new QVBoxLayout;
    auto *rt = new QLabel(tpl::tr("Что входит в выбранный вариант", "What the selected variant includes"));
    rt->setProperty("dim", true);
    right->addWidget(rt);
    auto *nameRow = new QHBoxLayout;
    m_variantName = new QLineEdit;
    m_variantName->setPlaceholderText(tpl::tr("название", "name"));
    m_variantTag = new QLineEdit;
    m_variantTag->setPlaceholderText(tpl::tr("тег файлов", "file tag"));
    m_variantTag->setMaximumWidth(120);
    nameRow->addWidget(m_variantName, 1);
    nameRow->addWidget(m_variantTag);
    right->addLayout(nameRow);
    m_shapes = new QListWidget;
    right->addWidget(m_shapes, 1);
    row->addLayout(right, 1);
    vl->addLayout(row, 1);

    m_variantHint = new QLabel;
    m_variantHint->setProperty("mute", true);
    m_variantHint->setWordWrap(true);
    vl->addWidget(m_variantHint);
    root->addWidget(var, 1);

    m_summary = new QLabel;
    m_summary->setWordWrap(true);
    root->addWidget(m_summary);

    // ------------------------------------------------------------------ связи
    connect(m_ports, &QListWidget::itemChanged, this, [this] { pullPorts(); });
    connect(m_shapes, &QListWidget::itemChanged, this, [this] { pullVariantShapes(); });
    connect(m_variants, &QListWidget::currentRowChanged, this, [this] { rebuildShapes(); });

    connect(m_add, &QPushButton::clicked, this, [this] {
        if (!m_project)
            return;
        emit aboutToEdit();
        Variant v;
        v.name = tpl::tr("Вариант %1", "Variant %1").arg(m_project->variants.size() + 1);
        v.tag = QStringLiteral("v%1").arg(m_project->variants.size() + 1);
        m_project->variants << v;
        emit edited();
        refresh();
        m_variants->setCurrentRow(m_project->variants.size() - 1);
    });
    connect(m_preset, &QPushButton::clicked, this, [this] {
        if (!m_project)
            return;
        emit aboutToEdit();
        m_project->variants.clear();

        // Заливки (диэлектрик, плазма) — это «начинка». Контуры — стенки — остаются
        // в обоих вариантах, иначе волновода просто не будет.
        Variant empty;
        empty.name = tpl::tr("Пустой", "Empty");
        empty.tag = QStringLiteral("wg");
        for (const Shape &s : m_project->shapes) {
            if (!s.isContour())
                empty.disabledShapes << s.id;
        }
        Variant full;
        full.name = tpl::tr("С заполнением", "Filled");
        full.tag = QStringLiteral("fill");
        m_project->variants << empty << full;
        emit edited();
        refresh();
        m_variants->setCurrentRow(0);
    });
    connect(m_remove, &QPushButton::clicked, this, [this] {
        const int i = currentVariant();
        if (!m_project || i < 0)
            return;
        emit aboutToEdit();
        m_project->variants.remove(i);
        emit edited();
        refresh();
    });

    const auto renameVariant = [this] {
        const int i = currentVariant();
        if (m_loading || !m_project || i < 0)
            return;
        emit aboutToEdit();
        m_project->variants[i].name = m_variantName->text();
        m_project->variants[i].tag = m_variantTag->text();
        emit edited();
        rebuildVariants();
        m_variants->setCurrentRow(i);
    };
    connect(m_variantName, &QLineEdit::editingFinished, this, renameVariant);
    connect(m_variantTag, &QLineEdit::editingFinished, this, renameVariant);

    // --- угловой свип ---
    connect(m_sweepOn, &QCheckBox::toggled, this, [this](bool on) {
        if (m_loading || !m_project)
            return;
        // Включили впервые — предлагаем разумное: весь объект, кроме поглотителей и
        // стенок у края, углы 0...180 шагом 5, ось в центре объекта.
        if (on && m_project->sweep.angles.isEmpty()) {
            emit aboutToEdit();
            AngleSweep &s = m_project->sweep;
            s.referenceStep = true;
            for (double a = 0; a <= 180.0 + 1e-9; a += 5.0)
                s.angles << a;
            for (const Shape &sh : m_project->shapes) {
                if (sh.material != Material::Absorber)
                    s.shapeIds << sh.id;
            }
            s.enabled = true;
            s.pivotAtCenter = true;
            emit edited();
            rebuildSweep();
            updateSummary();
            return;
        }
        pullSweep();
    });
    connect(m_sweepShapes, &QListWidget::itemChanged, this, [this] { pullSweep(); });
    for (QDoubleSpinBox *s : {m_pivotX, m_pivotY, m_angFrom, m_angTo, m_angStep})
        connect(s, &QDoubleSpinBox::valueChanged, this, [this] { pullSweep(); });
    connect(m_sweepRef, &QCheckBox::toggled, this, [this] { pullSweep(); });
    // Сняли галочку — ось остаётся там, где была центром, и дальше правится числами.
    connect(m_pivotCenter, &QCheckBox::toggled, this, [this] { pullSweep(); });
}

void RunPlanPage::rebuildSweep()
{
    if (!m_project)
        return;
    m_loading = true;
    const AngleSweep &s = m_project->sweep;
    m_sweepOn->setChecked(s.enabled);
    {
        const QSignalBlocker block(m_sweepShapes);
        m_sweepShapes->clear();
        for (const Shape &sh : m_project->shapes) {
            auto *it = new QListWidgetItem(QStringLiteral("%1 — %2").arg(sh.name, materialName(sh.material)),
                                           m_sweepShapes);
            it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
            it->setCheckState(s.shapeIds.contains(sh.id) ? Qt::Checked : Qt::Unchecked);
            it->setData(Qt::UserRole, sh.id);
        }
    }
    // Показываем ось, которой пользуются эмиттер и холст: при оси «в центре» это
    // текущий центр объекта, а не когда-то запомненное число.
    const QPointF pivot = sweepPivot(*m_project);
    m_pivotCenter->setChecked(s.pivotAtCenter);
    m_pivotX->setValue(pivot.x());
    m_pivotY->setValue(pivot.y());
    if (!s.angles.isEmpty()) {
        m_angFrom->setValue(s.angles.first());
        m_angTo->setValue(s.angles.last());
        m_angStep->setValue(s.angles.size() > 1 ? std::fabs(s.angles.at(1) - s.angles.at(0)) : 5.0);
    } else {
        m_angFrom->setValue(0.0);
        m_angTo->setValue(180.0);
        m_angStep->setValue(5.0);
    }
    m_sweepRef->setChecked(s.referenceStep);
    for (QWidget *w : std::initializer_list<QWidget *>{m_sweepShapes, m_pivotCenter, m_angFrom,
                                                        m_angTo, m_angStep, m_sweepRef})
        w->setEnabled(s.enabled);
    m_pivotX->setEnabled(s.enabled && !s.pivotAtCenter);
    m_pivotY->setEnabled(s.enabled && !s.pivotAtCenter);
    // Свип с вариантами не сочетается: точка .s подписана углом. Список возбуждений
    // остаётся доступным — в нём можно оставить один вход (так он и считается), а
    // заблокированный список с отмеченным портом было бы нечем исправить.
    m_varGroup->setEnabled(!s.enabled);

    if (!s.enabled) {
        m_sweepHint->setText(tpl::tr("Выключено: объект стоит на месте.", "Off: the object stays in place."));
        m_sweepHint->setStyleSheet(QStringLiteral("color:#6c7d97;"));
    } else {
        // Сначала — что получится, затем — что мешает. Каждое «мешает» говорит, ГДЕ
        // это исправить: иначе человек видит запрет и не знает, куда идти.
        QString t = tpl::tr("Будет %1 шаг(ов): %2 углов%3. Варианты геометрии на это "
                            "время отключены.",
                            "There will be %1 step(s): %2 angles%3. Geometry variants are disabled for "
                            "this time.")
                        .arg(m_project->stepCount())
                        .arg(s.angles.size())
                        .arg(s.referenceStep ? tpl::tr(" + опорный шаг без объекта", " + reference step without the object")
                                             : QString());
        // Кто именно поворачивается — словами: в списке выше отмеченная фигура может
        // оказаться за краем прокрутки.
        QStringList names;
        for (const Shape &sh : m_project->shapes) {
            if (s.rotates(sh.id))
                names << QStringLiteral("«%1»").arg(sh.name);
        }
        if (!names.isEmpty())
            t = tpl::tr("Поворачивается: %1. ", "Rotating: %1. ").arg(names.join(QStringLiteral(", "))) + t;
        QStringList fix;
        if (s.shapeIds.isEmpty())
            fix << tpl::tr("отметьте в списке выше фигуры, которые поворачиваются", "tick the shapes that rotate in the list above");
        if (s.angles.isEmpty())
            fix << tpl::tr("угол «до» должен быть не меньше угла «от»", "the \"to\" angle must not be less than the \"from\" angle");
        if (m_project->exciters.size() > 1)
            fix << tpl::tr("в «Возбуждении портов» слева оставьте отмеченным один вход "
                           "(сейчас %1)",
                           "in \"Port excitation\" on the left leave one port ticked (now %1)").arg(m_project->exciters.size());
        if (m_project->freqs.size() > 1)
            fix << tpl::tr("на вкладке «Расчёт» оставьте одну частоту (сейчас %1)", "on the \"Run\" tab leave one frequency (now %1)")
                       .arg(m_project->freqs.size());
        if (!fix.isEmpty())
            t += tpl::tr("\nЧтобы сохранить задание: ", "\nTo save the task: ") + fix.join(QStringLiteral("; ")) +
                 QLatin1Char('.');
        m_sweepHint->setText(t);
        m_sweepHint->setStyleSheet(fix.isEmpty() ? QStringLiteral("color:#5ce08a;")
                                                 : QStringLiteral("color:#e3b341;"));
    }
    m_loading = false;
}

void RunPlanPage::pullSweep()
{
    if (m_loading || !m_project)
        return;
    emit aboutToEdit();
    AngleSweep &s = m_project->sweep;
    s.enabled = m_sweepOn->isChecked();
    s.shapeIds.clear();
    for (int i = 0; i < m_sweepShapes->count(); ++i) {
        if (m_sweepShapes->item(i)->checkState() == Qt::Checked)
            s.shapeIds << m_sweepShapes->item(i)->data(Qt::UserRole).toInt();
    }
    const bool wasCenter = s.pivotAtCenter;
    s.pivotAtCenter = m_pivotCenter->isChecked();
    // Только что сняли галочку — фиксируем ось там, где был центр, а не в старом числе.
    if (wasCenter && !s.pivotAtCenter) {
        s.pivotAtCenter = true;
        s.pivot = sweepPivot(*m_project);
        s.pivotAtCenter = false;
    } else if (!s.pivotAtCenter) {
        s.pivot = QPointF(m_pivotX->value(), m_pivotY->value());
    }
    s.angles.clear();
    const double a0 = m_angFrom->value(), a1 = m_angTo->value(), st = m_angStep->value();
    if (st > 0.0 && a1 >= a0) {
        for (double a = a0; a <= a1 + st * 1e-6 && s.angles.size() < 400; a += st)
            s.angles << std::round(a * 1000.0) / 1000.0;
    }
    s.referenceStep = m_sweepRef->isChecked();
    emit edited();
    rebuildSweep();
    updateSummary();
}

void RunPlanPage::setProject(Project *p)
{
    m_project = p;
    refresh();
}

int RunPlanPage::currentVariant() const
{
    const int i = m_variants->currentRow();
    if (!m_project || i < 0 || i >= m_project->variants.size())
        return -1;
    return i;
}

void RunPlanPage::refresh()
{
    rebuildPorts();
    rebuildVariants();
    rebuildShapes();
    rebuildSweep();
    updateSummary();
}

void RunPlanPage::rebuildPorts()
{
    if (!m_project)
        return;
    m_loading = true;
    const QSignalBlocker block(m_ports);
    m_ports->clear();
    for (const Port &p : m_project->ports) {
        // Подписываем строку тем, что она ДАЁТ: пользователь мыслит S-параметрами.
        // Но выбор всё равно по входам — один прогон отдаёт целый столбец матрицы,
        // отдельный элемент из него не вынуть.
        const int j = m_project->portOrdinal(p.id);
        QStringList gives;
        for (const Port &q : m_project->ports)
            gives << QStringLiteral("S%1%2").arg(m_project->portOrdinal(q.id)).arg(j);

        // Ведём строку S-параметрами: именно их ищут глазами. Имя входа — следом,
        // чтобы было видно, какую фигуру на чертеже включает эта галочка.
        auto *it = new QListWidgetItem(
            tpl::tr("%1   —   возбудить вход %2 «%3»", "%1   —   excite port %2 \"%3\"")
                .arg(gives.join(QStringLiteral(", "))).arg(j).arg(p.name),
            m_ports);
        it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
        it->setCheckState(m_project->exciters.contains(p.id) ? Qt::Checked : Qt::Unchecked);
        it->setData(Qt::UserRole, p.id);
    }
    m_loading = false;
}

void RunPlanPage::rebuildVariants()
{
    if (!m_project)
        return;
    m_loading = true;
    const int keep = m_variants->currentRow();
    const QSignalBlocker block(m_variants);
    m_variants->clear();
    for (const Variant &v : m_project->variants) {
        // Показываем, чем вариант отличается: без этого список из «Вариант 1»
        // и «Вариант 2» ничего не говорит.
        QStringList off;
        int on = 0;
        for (const Shape &s : m_project->shapes) {
            if (v.isEnabled(s.id))
                ++on;
            else
                off << s.name;
        }
        QString what = off.isEmpty()
                           ? tpl::tr("вся геометрия", "all geometry")
                           : tpl::tr("без: %1", "without: %1").arg(off.join(QStringLiteral(", ")));
        if (on == 0 && !m_project->shapes.isEmpty())
            what = tpl::tr("ПУСТО — ни одной фигуры", "EMPTY: no shapes at all");

        auto *it = new QListWidgetItem(QStringLiteral("%1  [%2]\n%3").arg(v.name, v.tag, what),
                                       m_variants);
        it->setForeground(on == 0 && !m_project->shapes.isEmpty() ? dtheme::danger()
                                                                  : dtheme::text());
    }
    if (keep >= 0 && keep < m_variants->count())
        m_variants->setCurrentRow(keep);
    else if (m_variants->count() > 0)
        m_variants->setCurrentRow(0);
    m_loading = false;
}

void RunPlanPage::rebuildShapes()
{
    if (!m_project)
        return;
    m_loading = true;
    const QSignalBlocker block(m_shapes);
    m_shapes->clear();

    const int vi = currentVariant();
    const bool has = (vi >= 0);
    m_variantName->setEnabled(has);
    m_variantTag->setEnabled(has);
    m_shapes->setEnabled(has);
    m_remove->setEnabled(has);

    if (!has) {
        m_variantName->clear();
        m_variantTag->clear();
        m_variantHint->setText(m_project->variants.isEmpty()
                                   ? tpl::tr("Вариантов нет — задание посчитается один "
                                             "раз, со всей нарисованной геометрией.",
                                             "No variants: the task will be computed once, with all the drawn geometry.")
                                   : tpl::tr("Выберите вариант слева.", "Select a variant on the left."));
        m_loading = false;
        return;
    }

    const Variant &v = m_project->variants.at(vi);
    m_variantName->setText(v.name);
    m_variantTag->setText(v.tag);
    for (const Shape &s : m_project->shapes) {
        auto *it = new QListWidgetItem(QStringLiteral("%1 — %2")
                                           .arg(s.name, materialName(s.material)), m_shapes);
        it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
        it->setCheckState(v.isEnabled(s.id) ? Qt::Checked : Qt::Unchecked);
        it->setData(Qt::UserRole, s.id);
    }
    m_variantHint->setText(tpl::tr("Снятая галочка = фигуры в этом варианте нет. Порты остаются во всех вариантах: "
        "их выключение сделало бы матрицу другой размерности.",
        "Unticked = the shape is absent in this variant. Ports stay in all variants: "
        "turning them off would change the matrix size."));
    m_loading = false;
}

void RunPlanPage::pullPorts()
{
    if (m_loading || !m_project)
        return;
    emit aboutToEdit();
    m_project->exciters.clear();
    for (int i = 0; i < m_ports->count(); ++i) {
        if (m_ports->item(i)->checkState() == Qt::Checked)
            m_project->exciters << m_ports->item(i)->data(Qt::UserRole).toInt();
    }
    emit edited();
    updateSummary();
}

void RunPlanPage::pullVariantShapes()
{
    const int vi = currentVariant();
    if (m_loading || !m_project || vi < 0)
        return;
    emit aboutToEdit();
    QVector<int> off;
    for (int i = 0; i < m_shapes->count(); ++i) {
        if (m_shapes->item(i)->checkState() != Qt::Checked)
            off << m_shapes->item(i)->data(Qt::UserRole).toInt();
    }
    m_project->variants[vi].disabledShapes = off;
    emit edited();
    updateSummary();
}

void RunPlanPage::updateSummary()
{
    if (!m_project)
        return;

    const int nv = std::max(1, int(m_project->variants.size()));
    const int ne = std::max(1, int(m_project->exciters.size()));
    const int nf = std::max(1, int(m_project->freqs.size()));
    const int total = m_project->stepCount();

    QString s = m_project->sweep.active()
                    ? tpl::tr("Шагов расчёта: %1 (поворот объекта: %2 углов%3).", "Run steps: %1 (object rotation: %2 angles%3).")
                          .arg(total)
                          .arg(m_project->sweep.angles.size())
                          .arg(m_project->sweep.referenceStep ? tpl::tr(" + опорный шаг", " + reference step")
                                                              : QString())
                    : tpl::tr("Шагов расчёта: %1 варианта x %2 возбуждения x %3 частот "
                              "= %4.",
                              "Run steps: %1 variants x %2 excitations x %3 frequencies = %4.").arg(nv).arg(ne).arg(nf).arg(total);
    // Ориентир из замеров: ~14 с на шаг для сетки 240x80 при DELTA = 1 мм, TIME 115 нс.
    s += tpl::tr("  Ориентировочно %1 мин счёта при сетке 240x80 и шаге 1 мм.", "  Roughly %1 min of computation for a 240x80 grid and a 1 mm step.")
             .arg(total * 14.0 / 60.0, 0, 'f', 1);
    if (total > 1 && !m_project->sweep.active()) {
        s += tpl::tr("\nБез этой страницы столько же данных потребовало бы %1 "
                     "отдельных заданий и %1 запусков.",
                     "\nWithout this page the same data would need %1 separate tasks and %1 "
                     "launches.").arg(nv * ne);
    }
    m_summary->setText(s);
    m_summary->setStyleSheet(total > 200 ? QStringLiteral("color:#e3b341;")
                                         : QStringLiteral("color:#9fb0c8;"));

    // При повороте объекта очередь возбуждений значит другое: вход один на все шаги.
    if (m_project->sweep.active()) {
        const int n = m_project->exciters.size();
        m_portsHint->setText(
            n == 0 ? tpl::tr("Поворот объекта: на всех шагах работают входы, помеченные "
                             "активными в свойствах порта. Можно и отметить вход здесь.",
                             "Object rotation: at all steps the ports marked active in the port properties "
                             "work. You can also tick a port here.")
            : n == 1 ? tpl::tr("Поворот объекта: на всех шагах возбуждается этот вход.", "Object rotation: this port is excited at all steps.")
                     : tpl::tr("Поворот объекта: оставьте отмеченным ОДИН вход — сейчас %1.", "Object rotation: leave ONE port ticked (now %1).")
                           .arg(n));
        m_portsHint->setStyleSheet(n > 1 ? QStringLiteral("color:#e3b341;")
                                         : QStringLiteral("color:#6c7d97;"));
        return;
    }
    // Какие элементы матрицы получатся.
    if (m_project->exciters.isEmpty()) {
        m_portsHint->setText(tpl::tr("Ничего не отмечено — будет один шаг с портами, помеченными активными вручную "
            "в свойствах порта.",
            "Nothing ticked: there will be one step with the ports marked active manually "
            "in the port properties."));
        m_portsHint->setStyleSheet(QStringLiteral("color:#6c7d97;"));
        return;
    }
    QStringList got;
    for (int id : m_project->exciters) {
        const int j = m_project->portOrdinal(id);
        for (const Port &p : m_project->ports)
            got << QStringLiteral("S%1%2").arg(m_project->portOrdinal(p.id)).arg(j);
    }
    const int n = m_project->ports.size();
    const bool full = (m_project->exciters.size() == n);
    m_portsHint->setText(tpl::tr("Получится: %1.%2", "Result: %1.%2")
                             .arg(got.join(QStringLiteral(", ")),
                                  full ? tpl::tr(" Это полная матрица.", " This is the full matrix.")
                                       : tpl::tr(" Остальные столбцы останутся пустыми.", " The other columns will stay empty.")));
    m_portsHint->setStyleSheet(full ? QStringLiteral("color:#5ce08a;")
                                    : QStringLiteral("color:#e3b341;"));
}
