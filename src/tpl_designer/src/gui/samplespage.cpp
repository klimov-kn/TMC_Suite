/// @file samplespage.cpp
/// @brief Реализация страницы библиотеки образцов `SamplesPage`.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-samples
#include "samplespage.h"

#include "emitter.h"
#include "geometryview.h"
#include "issuedialog.h"
#include "kernelnames.h"
#include "taskfiles.h"
#include "dtheme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QSplitter>
#include <QStackedWidget>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include "lang.h"

using namespace tpl;
using namespace tpl::samples;

namespace {

/// Размер картинки плитки, логические пиксели.
const QSize kThumb{200, 120};

/// @brief Экранирует текст для вставки в HTML.
/// @param[in] s  исходный текст.
/// @return Текст с сущностями HTML вместо `<`, `>`, `&`, `"`.
QString esc(const QString &s)
{
    return s.toHtmlEscaped();
}

/// Строки паспорта — таблицей: подпись приглушённая, значение обычное.
/// @brief Составляет HTML-таблицу строк паспорта.
/// @param[in] rows  строки «подпись — значение».
/// @return Таблица HTML на всю ширину.
QString rowsHtml(const QVector<Row> &rows)
{
    QString h = QStringLiteral("<table cellspacing='0' cellpadding='3' width='100%'>");
    for (const Row &r : rows) {
        h += QStringLiteral("<tr><td style='color:#9fb0c8; padding-right:10px;' valign='top'>%1"
                            "</td><td valign='top'>%2</td></tr>")
                 .arg(esc(r.label), esc(r.value));
    }
    h += QStringLiteral("</table>");
    return h;
}

/// @brief Возвращает подпись ядра для строки сведений карточки.
/// @param[in] k  ядро задания.
/// @return «ядро tmc_rtx (X-мода)» или «ядро tmc_rth (H-мода)».
QString kernelText(Kernel k)
{
    // I-35: имя ядра — то, что человек найдёт на своей системе (kernelProgram: planarrt_h.exe и т. п.);
    // было — прежние имена tmc_rth / tmc_rtx.
    return k == Kernel::X ? tpl::tr("ядро %1 (X-мода)", "kernel %1 (X-mode)").arg(kernelProgram(k))
                          : tpl::tr("ядро %1 (H-мода)", "kernel %1 (H-mode)").arg(kernelProgram(k));
}

/// @brief Оборачивает виджет в рамку с заголовком.
/// @param[in] title    заголовок рамки.
/// @param[in] content  содержимое; переходит во владение рамки.
/// @return Новая рамка без родителя.
QGroupBox *group(const QString &title, QWidget *content)
{
    auto *g = new QGroupBox(title);
    auto *l = new QVBoxLayout(g);
    l->setContentsMargins(10, 8, 10, 10);
    l->addWidget(content);
    return g;
}

/// @brief Создаёт надпись для HTML-текста карточки (перенос строк, выделение мышью).
/// @return Новая надпись без родителя.
QLabel *richLabel()
{
    auto *l = new QLabel;
    l->setWordWrap(true);
    l->setTextFormat(Qt::RichText);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse);
    l->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    return l;
}

/// Плитка каталога: картинка сверху, под ней название в две-три строки.
///
/// Своя отрисовка, а не штатная: в режиме значков Qt сокращает подпись многоточием
/// уже на второй строке, и «Отрезок прямоугольного волновода» превращался в
/// «Отрезок прямоугольного …».
class TileDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    /// @brief Размер плитки: картинка плюс место под название.
    /// @return (`kThumb.width()+20`) x (`kThumb.height()+64`) пикселей.
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override
    {
        return {kThumb.width() + 20, kThumb.height() + 64};
    }

    /// @brief Рисует плитку: подложку выделения/наведения, картинку и название с переносом слов.
    /// @param[in,out] p      рисовальщик списка.
    /// @param[in]     opt    геометрия и состояние строки.
    /// @param[in]     index  строка списка: значок и название.
    void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const override
    {
        p->save();
        p->setRenderHint(QPainter::Antialiasing, true);
        const QRect r = opt.rect.adjusted(3, 3, -3, -3);
        const bool sel = opt.state & QStyle::State_Selected;
        const bool hover = opt.state & QStyle::State_MouseOver;
        if (sel || hover) {
            p->setPen(Qt::NoPen);
            p->setBrush(sel ? dtheme::line() : QColor(0x1a, 0x22, 0x33));
            p->drawRoundedRect(r, 10, 10);
        }
        const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
        const QRect ir(r.left() + (r.width() - kThumb.width()) / 2, r.top() + 6, kThumb.width(),
                       kThumb.height());
        icon.paint(p, ir);
        QRect tr(r.left() + 6, ir.bottom() + 6, r.width() - 12, r.bottom() - ir.bottom() - 8);
        p->setPen(sel ? dtheme::accent() : dtheme::text());
        p->setFont(opt.font);
        p->drawText(tr, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
                    index.data(Qt::DisplayRole).toString());
        p->restore();
    }
};

/// Заглушка плитки, пока настоящая картинка не нарисована.
/// @return Пустая картинка размера `kThumb` с рамкой.
QPixmap placeholder()
{
    QPixmap pm(kThumb);
    pm.fill(dtheme::bg());
    QPainter p(&pm);
    p.setPen(dtheme::line());
    p.drawRect(pm.rect().adjusted(0, 0, -1, -1));
    return pm;
}

} // namespace

SamplesPage::SamplesPage(QWidget *parent)
    : QWidget(parent)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(18, 14, 18, 14);
    root->setSpacing(10);

    // --- шапка ---
    auto *top = new QHBoxLayout;
    auto *back = new QPushButton(tpl::tr("← В начало", "← Home"));
    back->setObjectName(QStringLiteral("samplesBack"));
    connect(back, &QPushButton::clicked, this, &SamplesPage::backRequested);
    top->addWidget(back);
    top->addSpacing(12);
    auto *title = new QLabel(tpl::tr("Библиотека образцов", "Sample library"));
    title->setProperty("h2", true);
    top->addWidget(title);
    top->addStretch();
    m_search = new QLineEdit;
    m_search->setObjectName(QStringLiteral("samplesSearch"));
    m_search->setPlaceholderText(tpl::tr("Поиск: волновод, рупор, плазма, WR-90…", "Search: waveguide, horn, plasma, WR-90…"));
    m_search->setClearButtonEnabled(true);
    m_search->setMinimumWidth(280);
    top->addWidget(m_search);
    root->addLayout(top);

    auto *split = new QSplitter(Qt::Horizontal);
    split->setChildrenCollapsible(false);

    // --- разделы ---
    m_sections = new QListWidget;
    m_sections->setObjectName(QStringLiteral("samplesSections"));
    m_sections->setMinimumWidth(190);
    m_sections->setMaximumWidth(260);
    split->addWidget(m_sections);

    // --- плитки ---
    m_galleryStack = new QStackedWidget;
    m_gallery = new QListWidget;
    m_gallery->setObjectName(QStringLiteral("samplesGallery"));
    m_gallery->setViewMode(QListView::IconMode);
    m_gallery->setIconSize(kThumb);
    m_gallery->setGridSize(QSize(kThumb.width() + 24, kThumb.height() + 68));
    m_gallery->setItemDelegate(new TileDelegate(m_gallery));
    m_gallery->setMouseTracking(true); // подсветка плитки под курсором
    m_gallery->setResizeMode(QListView::Adjust);
    m_gallery->setMovement(QListView::Static);
    m_gallery->setWordWrap(true);
    m_gallery->setUniformItemSizes(true);
    m_gallery->setTextElideMode(Qt::ElideNone);
    m_gallery->setMinimumWidth(kThumb.width() + 60);
    m_galleryStack->addWidget(m_gallery);
    m_empty = new QLabel;
    m_empty->setWordWrap(true);
    m_empty->setAlignment(Qt::AlignCenter);
    m_empty->setProperty("dim", true);
    m_empty->setMargin(30);
    m_galleryStack->addWidget(m_empty);
    split->addWidget(m_galleryStack);

    // --- карточка ---
    m_cardStack = new QStackedWidget;
    auto *hintCard = new QLabel(tpl::tr("Выберите образец слева — здесь появятся его "
                                        "картинка, параметры и паспорт.",
                                        "Select a sample on the left — its picture, parameters and datasheet will "
                                        "appear here."));
    hintCard->setWordWrap(true);
    hintCard->setAlignment(Qt::AlignCenter);
    hintCard->setProperty("dim", true);
    m_cardStack->addWidget(hintCard);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto *card = new QWidget;
    auto *cl = new QVBoxLayout(card);
    cl->setContentsMargins(4, 0, 10, 10);
    cl->setSpacing(8);

    m_title = new QLabel;
    m_title->setObjectName(QStringLiteral("sampleTitle"));
    m_title->setProperty("h2", true);
    m_title->setWordWrap(true);
    cl->addWidget(m_title);
    m_meta = new QLabel;
    m_meta->setProperty("mute", true);
    m_meta->setWordWrap(true);
    cl->addWidget(m_meta);

    m_preview = new GeometryView;
    m_preview->setObjectName(QStringLiteral("samplePreview"));
    m_preview->setPreview(true);
    m_preview->setMinimumHeight(220);
    m_preview->setMaximumHeight(260);
    m_preview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_preview->setFixedHeight(240);
    cl->addWidget(m_preview);

    m_summary = new QLabel;
    m_summary->setProperty("dim", true);
    m_summary->setWordWrap(true);
    cl->addWidget(m_summary);

    // Параметры и действия — наверху: их видно без прокрутки.
    m_paramsBox = new QGroupBox(tpl::tr("Параметры", "Parameters"));
    auto *pl = new QVBoxLayout(m_paramsBox);
    pl->setContentsMargins(10, 8, 10, 10);
    m_paramsForm = new QFormLayout;
    m_paramsForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_paramsForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    m_paramsForm->setRowWrapPolicy(QFormLayout::WrapLongRows);
    pl->addLayout(m_paramsForm);
    m_reset = new QPushButton(tpl::tr("Вернуть умолчания", "Restore defaults"));
    m_reset->setToolTip(tpl::tr("Числа ядра в паспорте посчитаны для параметров по умолчанию", "The kernel numbers in the datasheet were computed for the default parameters"));
    connect(m_reset, &QPushButton::clicked, this, &SamplesPage::onResetParams);
    auto *rl = new QHBoxLayout;
    rl->addStretch();
    rl->addWidget(m_reset);
    pl->addLayout(rl);
    cl->addWidget(m_paramsBox);

    auto *actions = new QHBoxLayout;
    m_delete = new QPushButton(tpl::tr("Удалить", "Delete"));
    m_delete->setToolTip(tpl::tr("Убрать образец из «Моих образцов» вместе с его папкой", "Remove the sample from “My samples” together with its folder"));
    connect(m_delete, &QPushButton::clicked, this, &SamplesPage::onDeleteUser);
    actions->addWidget(m_delete);
    m_save = new QPushButton(tpl::tr("Сохранить .tpl…", "Save .tpl…"));
    m_save->setObjectName(QStringLiteral("sampleSave"));
    m_save->setMinimumHeight(36);
    m_save->setToolTip(tpl::tr("Записать задание в папку и сразу считать ядром", "Write the task to a folder and compute it with the kernel right away"));
    connect(m_save, &QPushButton::clicked, this, &SamplesPage::onSave);
    actions->addWidget(m_save, 1);
    m_open = new QPushButton(tpl::tr("Открыть в редакторе", "Open in editor"));
    m_open->setObjectName(QStringLiteral("sampleOpen"));
    m_open->setProperty("accent", true);
    m_open->setMinimumHeight(36);
    m_open->setToolTip(tpl::tr("Все фигуры, входы и настройки лягут в мастер — дальше "
                               "их можно править",
                               "All shapes, ports and settings go into the wizard — you can edit them there"));
    connect(m_open, &QPushButton::clicked, this, &SamplesPage::onOpen);
    actions->addWidget(m_open, 1);
    cl->addLayout(actions);

    m_check = richLabel();
    m_check->setObjectName(QStringLiteral("sampleCheck"));
    cl->addWidget(m_check);

    m_theory = richLabel();
    cl->addWidget(group(tpl::tr("Паспорт: теория", "Datasheet: theory"), m_theory));
    m_kernel = richLabel();
    m_kernel->setObjectName(QStringLiteral("sampleKernel"));
    cl->addWidget(group(tpl::tr("Посчитано ядром", "Computed by kernel"), m_kernel));
    m_notes = richLabel();
    cl->addWidget(group(tpl::tr("Постановка", "Problem setup"), m_notes));
    m_help = richLabel();
    cl->addWidget(group(tpl::tr("Справка", "Notes"), m_help));
    cl->addStretch();

    scroll->setWidget(card);
    m_cardStack->addWidget(scroll);
    m_cardStack->setMinimumWidth(430);
    split->addWidget(m_cardStack);

    split->setStretchFactor(0, 0);
    split->setStretchFactor(1, 3);
    split->setStretchFactor(2, 4);
    split->setSizes({210, 520, 560});
    root->addWidget(split, 1);

    m_rebuildTimer = new QTimer(this);
    m_rebuildTimer->setSingleShot(true);
    m_rebuildTimer->setInterval(120);
    connect(m_rebuildTimer, &QTimer::timeout, this, &SamplesPage::rebuildBuild);

    // Картинки рисуются понемногу, чтобы окно не замирало на первом открытии.
    m_thumbTimer = new QTimer(this);
    m_thumbTimer->setInterval(0);
    connect(m_thumbTimer, &QTimer::timeout, this, &SamplesPage::renderSomeThumbs);

    connect(m_sections, &QListWidget::currentRowChanged, this, [this](int row) {
        if (m_loading || row < 0)
            return;
        m_section = m_sections->item(row)->data(Qt::UserRole).toString();
        rebuildGallery();
    });
    connect(m_search, &QLineEdit::textChanged, this, [this] { rebuildGallery(); });
    connect(m_gallery, &QListWidget::currentRowChanged, this, [this](int row) {
        if (m_loading || row < 0)
            return;
        showEntry(m_gallery->item(row)->data(Qt::UserRole).toInt());
    });
    connect(m_gallery, &QListWidget::itemDoubleClicked, this, [this] { onOpen(); });

    reloadUserSamples();
}

void SamplesPage::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    // «Мои образцы» могли пополниться из мастера, пока окно было скрыто.
    reloadUserSamples();
}

void SamplesPage::reloadUserSamples()
{
    const QString keepKey = (m_current >= 0 && m_current < m_entries.size())
                                ? m_entries.at(m_current).key
                                : QString();
    m_user = userSamples();
    rebuildEntries();
    rebuildSections();
    rebuildGallery();
    // Вернуть выделение туда, где было.
    for (int i = 0; i < m_gallery->count(); ++i) {
        const int idx = m_gallery->item(i)->data(Qt::UserRole).toInt();
        if (m_entries.value(idx).key == keepKey) {
            m_gallery->setCurrentRow(i);
            break;
        }
    }
}

void SamplesPage::rebuildEntries()
{
    m_entries.clear();
    for (const Sample &s : all()) {
        Entry e;
        e.key = QStringLiteral("b:") + s.id;
        e.id = s.id;
        e.section = s.section;
        e.title = s.title;
        e.summary = s.summary;
        e.standard = s.standard;
        e.kernel = s.kernel;
        m_entries << e;
    }
    for (int i = 0; i < m_user.size(); ++i) {
        const UserSample &u = m_user.at(i);
        Entry e;
        e.user = true;
        e.key = QStringLiteral("u:") + u.dir;
        e.userIndex = i;
        e.section = QStringLiteral("user");
        e.title = u.title;
        e.summary = u.summary;
        e.standard = tpl::tr("Мои образцы", "My samples");
        e.kernel = u.kernel;
        m_entries << e;
    }
    // Картинки своих образцов могли устареть — их задание переписали.
    for (auto it = m_thumbs.begin(); it != m_thumbs.end();) {
        if (it.key().startsWith(QLatin1String("u:")))
            it = m_thumbs.erase(it);
        else
            ++it;
    }
    m_current = -1;
}

void SamplesPage::rebuildSections()
{
    m_loading = true;
    m_sections->clear();
    const auto add = [&](const QString &id, const QString &title, int count, const QString &tip) {
        auto *it = new QListWidgetItem(QStringLiteral("%1  (%2)").arg(title).arg(count));
        it->setData(Qt::UserRole, id);
        it->setToolTip(tip);
        m_sections->addItem(it);
        if (id == m_section)
            m_sections->setCurrentItem(it);
    };
    int builtin = 0;
    for (const Entry &e : std::as_const(m_entries))
        builtin += e.user ? 0 : 1;
    add(QStringLiteral("all"), tpl::tr("Все образцы", "All samples"), builtin,
        tpl::tr("Все встроенные образцы", "All built-in samples"));
    for (const Section &s : sections())
        add(s.id, s.title, inSection(s.id).size(), s.summary);
    add(QStringLiteral("user"), tpl::tr("Мои образцы", "My samples"), m_user.size(),
        tpl::tr("Свои задания, сохранённые из мастера кнопкой «Сохранить как образец»", "Your own tasks, saved from the wizard with the “Save as sample” button"));
    if (!m_sections->currentItem())
        m_sections->setCurrentRow(0);
    m_loading = false;
}



void SamplesPage::rebuildGallery()
{
    m_loading = true;
    m_gallery->clear();
    const QString q = m_search->text().trimmed();
    for (int i = 0; i < m_entries.size(); ++i) {
        const Entry &e = m_entries.at(i);
        if (q.isEmpty()) {
            if (m_section == QLatin1String("all") ? e.user : e.section != m_section)
                continue;
        } else {
            // Поиск идёт по всей библиотеке, включая свои образцы.
            const QString hay = e.title + QLatin1Char(' ') + e.summary + QLatin1Char(' ')
                                + e.standard + QLatin1Char(' ') + e.id;
            bool hit = true;
            for (const QString &w : q.split(QLatin1Char(' '), Qt::SkipEmptyParts))
                hit &= hay.contains(w, Qt::CaseInsensitive);
            if (!hit)
                continue;
        }
        auto *it = new QListWidgetItem(e.title);
        it->setData(Qt::UserRole, i);
        it->setToolTip(QStringLiteral("%1\n%2").arg(e.summary, e.standard));
        it->setIcon(QIcon(m_thumbs.value(e.key, placeholder())));
        it->setTextAlignment(Qt::AlignHCenter | Qt::AlignTop);
        m_gallery->addItem(it);
    }
    m_loading = false;

    if (m_gallery->count() == 0) {
        if (!q.isEmpty()) {
            m_empty->setText(tpl::tr("По запросу «%1» ничего не нашлось.", "Nothing found for “%1”.").arg(q));
        } else if (m_section == QLatin1String("user")) {
            m_empty->setText(tpl::tr("Здесь появятся ваши задания.\n\nОткройте или нарисуйте задание в мастере и на "
                "странице «Проверка» нажмите «Сохранить как образец…».\n\nОбразцы лежат в папке\n%1",
                "Your tasks will appear here.\n\nOpen or draw a task in the wizard and on the "
                "“Check” page press “Save as sample…”.\n\nSamples are stored in the folder\n%1")
                                 .arg(QDir::toNativeSeparators(userSamplesRoot())));
        } else {
            m_empty->setText(tpl::tr("В этом разделе пока пусто.", "This section is empty for now."));
        }
        m_galleryStack->setCurrentWidget(m_empty);
        m_cardStack->setCurrentIndex(0);
    } else {
        m_galleryStack->setCurrentWidget(m_gallery);
        m_gallery->setCurrentRow(0);
    }
    m_thumbTimer->start();
}

Build SamplesPage::buildFor(const Entry &e, const Values &v) const
{
    if (e.user)
        return loadUserSample(m_user.value(e.userIndex));
    const Sample *s = tpl::samples::find(e.id);
    return s ? make(*s, v) : Build{};
}

void SamplesPage::renderSomeThumbs()
{
    // По одной-две картинки за проход цикла событий: окно остаётся отзывчивым.
    int done = 0;
    for (int i = 0; i < m_gallery->count() && done < 2; ++i) {
        QListWidgetItem *it = m_gallery->item(i);
        const Entry &e = m_entries.value(it->data(Qt::UserRole).toInt());
        if (m_thumbs.contains(e.key))
            continue;
        const Sample *s = e.user ? nullptr : tpl::samples::find(e.id);
        const Build b = buildFor(e, s ? defaults(*s) : Values{});
        const QPixmap pm = GeometryView::render(b.project, kThumb);
        m_thumbs.insert(e.key, pm);
        it->setIcon(QIcon(pm));
        ++done;
    }
    if (done == 0)
        m_thumbTimer->stop();
}

const Sample *SamplesPage::currentSample() const
{
    if (m_current < 0 || m_current >= m_entries.size() || m_entries.at(m_current).user)
        return nullptr;
    return tpl::samples::find(m_entries.at(m_current).id);
}

void SamplesPage::showEntry(int entryIndex)
{
    if (entryIndex < 0 || entryIndex >= m_entries.size())
        return;
    m_current = entryIndex;
    const Entry &e = m_entries.at(entryIndex);
    const Sample *s = currentSample();
    m_values = s ? defaults(*s) : Values{};

    m_title->setText(e.title);
    QString sectionTitle = tpl::tr("Мои образцы", "My samples");
    for (const Section &sec : sections()) {
        if (sec.id == e.section)
            sectionTitle = sec.title;
    }
    m_meta->setText(QStringLiteral("%1  ·  %2  ·  %3").arg(sectionTitle, kernelText(e.kernel), e.standard));
    m_summary->setText(e.summary);
    m_summary->setVisible(!e.summary.isEmpty());
    m_delete->setVisible(e.user);
    rebuildParams();
    rebuildBuild();
    m_cardStack->setCurrentIndex(1);
}

void SamplesPage::rebuildParams()
{
    m_loading = true;
    while (m_paramsForm->rowCount() > 0)
        m_paramsForm->removeRow(0);

    const Sample *s = currentSample();
    const bool any = s && !s->params.isEmpty();
    m_paramsBox->setVisible(any);
    if (!any) {
        m_loading = false;
        return;
    }
    for (const Param &p : s->params) {
        QWidget *w = nullptr;
        const QString key = p.key;
        switch (p.kind) {
        case Param::Number: {
            auto *sb = new QDoubleSpinBox;
            sb->setDecimals(p.decimals);
            sb->setRange(p.min, p.max);
            sb->setSingleStep(p.step);
            if (!p.unit.isEmpty())
                sb->setSuffix(QLatin1Char(' ') + p.unit);
            sb->setValue(m_values.value(key));
            sb->setKeyboardTracking(false);
            connect(sb, &QDoubleSpinBox::valueChanged, this, [this, key](double v) {
                m_values.insert(key, v);
                m_rebuildTimer->start();
            });
            w = sb;
            break;
        }
        case Param::Integer: {
            auto *sb = new QSpinBox;
            sb->setRange(int(p.min), int(p.max));
            sb->setValue(int(m_values.value(key)));
            sb->setKeyboardTracking(false);
            connect(sb, &QSpinBox::valueChanged, this, [this, key](int v) {
                m_values.insert(key, v);
                m_rebuildTimer->start();
            });
            w = sb;
            break;
        }
        case Param::Choice: {
            auto *cb = new QComboBox;
            cb->addItems(p.choices);
            cb->setCurrentIndex(int(m_values.value(key)));
            // Длинные строки стандарта не должны раздувать карточку вширь.
            cb->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
            cb->setMinimumContentsLength(18);
            cb->setMaxVisibleItems(16);
            connect(cb, &QComboBox::currentIndexChanged, this, [this, key](int v) {
                if (v < 0)
                    return;
                m_values.insert(key, v);
                m_rebuildTimer->start();
            });
            w = cb;
            break;
        }
        case Param::Flag: {
            auto *ch = new QCheckBox;
            ch->setChecked(m_values.value(key) > 0.5);
            connect(ch, &QCheckBox::toggled, this, [this, key](bool on) {
                m_values.insert(key, on ? 1.0 : 0.0);
                m_rebuildTimer->start();
            });
            w = ch;
            break;
        }
        }
        w->setToolTip(p.hint);
        auto *label = new QLabel(p.label + QLatin1Char(':'));
        label->setToolTip(p.hint);
        m_paramsForm->addRow(label, w);
    }
    m_loading = false;
}

void SamplesPage::rebuildBuild()
{
    if (m_current < 0)
        return;
    m_build = buildFor(m_entries.at(m_current), m_values);
    m_view = m_build.project;
    // Высота картинки — по пропорциям задания: длинный тонкий волновод в квадратном
    // окне терялся бы посреди пустоты, а высокая сцена сжималась бы в полоску.
    const double aspect = m_view.box.width() > 0 ? m_view.box.height() / m_view.box.width() : 0.5;
    const int w = std::max(300, m_preview->width());
    m_preview->setFixedHeight(std::clamp(int(w * aspect * 1.08) + 16, 110, 300));
    m_preview->setProject(&m_view);
    updateCard();
}

void SamplesPage::updateCard()
{
    const Entry &e = m_entries.at(m_current);
    const Project &p = m_build.project;
    const Diagnostics d = validate(p);

    // --- проверка ---
    QString check;
    if (!d.ok()) {
        check = tpl::tr("<span style='color:#ff6b6b;'><b>Ядро такое задание не примет:</b></span><br>", "<span style='color:#ff6b6b;'><b>The kernel will not accept this task:</b></span><br>");
        for (const QString &x : d.errors)
            check += QStringLiteral("• %1<br>").arg(esc(x));
    } else {
        check = tpl::tr("<span style='color:#5ce08a;'><b>Задание корректно.</b></span> "
                        "%1, шагов расчёта: %2, сетка %3 x %4 (шаг %5 мм).",
                        "<span style='color:#5ce08a;'><b>The task is correct.</b></span> %1, run "
                        "steps: %2, grid %3 x %4 (step %5 mm).")
                    .arg(needsMacro(p) ? tpl::tr("Макро-версия (.tpl + _step.h)", "Macro version (.tpl + _step.h)")
                                       : tpl::tr("Один файл .tpl", "Single .tpl file"))
                    .arg(p.stepCount())
                    .arg(int(std::lround(p.box.width() / p.delta)))
                    .arg(int(std::lround(p.box.height() / p.delta)))
                    .arg(QString::number(p.delta, 'g', 4));
    }
    if (!d.warnings.isEmpty()) {
        check += tpl::tr("<br><span style='color:#e3b341;'>Стоит знать:</span><br>", "<br><span style='color:#e3b341;'>Good to know:</span><br>");
        for (const QString &x : d.warnings)
            check += QStringLiteral("• %1<br>").arg(esc(x));
    }
    m_check->setText(check);
    m_open->setEnabled(!p.shapes.isEmpty() || !p.ports.isEmpty());
    m_save->setEnabled(d.ok());

    // --- паспорт ---
    m_theory->setText(m_build.theory.isEmpty()
                          ? tpl::tr("<span style='color:#6c7d97;'>Формул для этого образца "
                                    "нет — результат даёт ядро.</span>",
                                    "<span style='color:#6c7d97;'>There are no formulas for this sample — the "
                                    "result comes from the kernel.</span>")
                          : rowsHtml(m_build.theory));

    // --- числа ядра ---
    const Sample *s = currentSample();
    const Baked *bk = s ? baked(s->id) : nullptr;
    QString kern;
    if (e.user) {
        kern = tpl::tr("<span style='color:#6c7d97;'>Свои образцы программа ядром не "
                       "считает — посчитайте сохранённое задание сами.</span>",
                       "<span style='color:#6c7d97;'>The program does not compute your own samples "
                       "with the kernel — compute the saved task yourself.</span>");
    } else if (!bk) {
        kern = tpl::tr("<span style='color:#6c7d97;'>Этот образец ядром ещё не "
                       "считали%1.</span>",
                       "<span style='color:#6c7d97;'>This sample has not been computed by the kernel "
                       "yet%1.</span>")
                   .arg(p.output.sMatrix ? QString()
                                         : tpl::tr(": результат — диаграмма направленности "
                                                   "и поле, их смотрят в TMC_DN и FieldView",
                                                   ": the result is a radiation pattern and a field, viewed in TMC_DN and "
                                                   "FieldView"));
    } else if (!isDefault(*s, m_values)) {
        kern = tpl::tr("<span style='color:#e3b341;'>Параметры изменены — числа ядра "
                       "относятся к умолчаниям. Верните их кнопкой «Вернуть умолчания» "
                       "или посчитайте новое задание.</span>",
                       "<span style='color:#e3b341;'>Parameters changed — the kernel numbers refer to "
                       "the defaults. Restore them with the “Restore defaults” button or compute a "
                       "new task.</span>");
    } else {
        kern = rowsHtml(bk->rows)
               + tpl::tr("<div style='color:#6c7d97; margin-top:4px;'>Прогон %1: %2. "
                         "|Sij| — модуль элементов S-матрицы из .s.</div>",
                         "<div style='color:#6c7d97; margin-top:4px;'>Run %1: %2. |Sij| is the "
                         "magnitude of the S-matrix elements from .s.</div>")
                     .arg(esc(bk->when), esc(bk->setup));
    }
    m_kernel->setText(kern);

    // --- постановка и справка ---
    QString notes;
    for (const QString &n : m_build.notes)
        notes += QStringLiteral("<p style='margin:0 0 6px 0;'>%1</p>").arg(esc(n));
    if (!m_build.companions.isEmpty()) {
        notes += tpl::tr("<p style='margin:0 0 6px 0;'>Рядом с заданием будут записаны: %1.</p>", "<p style='margin:0 0 6px 0;'>Written next to the task: %1.</p>")
                     .arg(esc(m_build.companions.keys().join(QStringLiteral(", "))));
    }
    if (!p.description.isEmpty() && e.user)
        notes += QStringLiteral("<p style='margin:0;'>%1</p>").arg(esc(p.description).replace(QLatin1Char('\n'), QStringLiteral("<br>")));
    m_notes->setText(notes.isEmpty() ? QStringLiteral("<span style='color:#6c7d97;'>—</span>") : notes);
    m_notes->parentWidget()->setVisible(!notes.isEmpty());

    const QString help = s ? s->help : e.summary;
    m_help->setText(esc(help).replace(QLatin1Char('\n'), QStringLiteral("<br>")));
    m_help->parentWidget()->setVisible(!help.isEmpty());

    m_reset->setEnabled(s && !isDefault(*s, m_values));
}

void SamplesPage::onResetParams()
{
    const Sample *s = currentSample();
    if (!s)
        return;
    m_values = defaults(*s);
    rebuildParams();
    rebuildBuild();
}

void SamplesPage::onOpen()
{
    if (m_current < 0)
        return;
    m_rebuildTimer->stop();
    rebuildBuild(); // вдруг параметр поменяли только что и таймер ещё не сработал
    emit openRequested(m_build.project, m_build.companions);
}

void SamplesPage::onSave()
{
    if (m_current < 0)
        return;
    m_rebuildTimer->stop();
    rebuildBuild();
    const Project &p = m_build.project;
    const Diagnostics d = validate(p);
    if (!d.ok()) {
        IssueDialog::report(this, tpl::tr("Нельзя создать файл", "Cannot create the file"),
                            tpl::tr("Ядро отвергло бы такое задание:", "The kernel would reject this task:"), d.errors, {});
        return;
    }

    QSettings st;
    const QString last = st.value(QStringLiteral("samples/lastDir")).toString();
    const QString dir = QFileDialog::getExistingDirectory(
        this, tpl::tr("Куда положить задание «%1»", "Where to put the task “%1”").arg(m_entries.at(m_current).title), last);
    if (dir.isEmpty())
        return;
    st.setValue(QStringLiteral("samples/lastDir"), dir);

    QStringList existing;
    for (const QString &f : taskFileNames(p, m_build.companions)) {
        if (QFileInfo::exists(QDir(dir).filePath(f)))
            existing << f;
    }
    if (!existing.isEmpty()) {
        if (QMessageBox::warning(this, tpl::tr("Перезаписать файлы?", "Overwrite files?"),
                                 tpl::tr("В папке уже есть:\n\n  %1\n\nОни будут перезаписаны.", "The folder already contains:\n\n  %1\n\nThey will be overwritten.")
                                     .arg(existing.join(QStringLiteral("\n  "))),
                                 QMessageBox::Save | QMessageBox::Cancel, QMessageBox::Cancel)
            != QMessageBox::Save)
            return;
    }

    const WriteResult r = writeTask(dir, p, m_build.companions);
    if (!r.ok) {
        QMessageBox::critical(this, tpl::tr("Не удалось записать", "Failed to write"), r.error);
        return;
    }
    QString msg = tpl::tr("Записано в %1:\n\n  %2\n", "Written to %1:\n\n  %2\n")
                      .arg(QDir::toNativeSeparators(dir), r.written.join(QStringLiteral("\n  ")));
    if (p.output.sMatrix) {
        msg += tpl::tr("\nПеред повторным прогоном удаляйте старые .s — ядро их ДОПИСЫВАЕТ, "
                       "и прежние точки подмешаются к новым.\n",
                       "\nDelete old .s files before a rerun — the kernel APPENDS to them, and the "
                       "previous points will mix with the new ones.\n");
    }
    if (r.written.size() > 1)
        msg += tpl::tr("\nФайлы должны лежать рядом: ядро ищет их только в папке задания.\n", "\nThe files must lie together: the kernel looks for them only in the task folder.\n");
    msg += tpl::tr("\nЗапуск: %1", "\nRun: %1").arg(kernelRunLine(p.kernel, p.name));
    QMessageBox::information(this, tpl::tr("Задание создано", "Task created"), msg);
}

void SamplesPage::onDeleteUser()
{
    if (m_current < 0 || !m_entries.at(m_current).user)
        return;
    const UserSample u = m_user.value(m_entries.at(m_current).userIndex);
    if (QMessageBox::question(this, tpl::tr("Удалить образец?", "Delete sample?"),
                              tpl::tr("Образец «%1» будет удалён вместе с папкой:\n\n%2", "Sample “%1” will be deleted together with its folder:\n\n%2")
                                  .arg(u.title, QDir::toNativeSeparators(u.dir)),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
        != QMessageBox::Yes)
        return;
    if (!removeUserSample(u)) {
        QMessageBox::critical(this, tpl::tr("Не удалось удалить", "Failed to delete"),
                              tpl::tr("Папку %1 удалить не получилось.", "Could not delete folder %1.").arg(u.dir));
        return;
    }
    m_current = -1;
    reloadUserSamples();
}
