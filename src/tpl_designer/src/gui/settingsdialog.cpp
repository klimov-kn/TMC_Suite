/// @file settingsdialog.cpp
/// @brief Реализация окна настроек `SettingsDialog`.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-settings
#include "settingsdialog.h"

#include "dtheme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "lang.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tpl::tr("Настройки", "Settings"));

    auto *root = new QVBoxLayout(this);
    root->setSpacing(14);

    // --- профиль eps ---
    auto *epsBox = new QGroupBox(tpl::tr("Неоднородная eps", "Inhomogeneous eps"), this);
    auto *epsLay = new QVBoxLayout(epsBox);

    m_epsGradient = new QCheckBox(tpl::tr("Заливать градиентом по формуле", "Fill with a gradient by formula"), epsBox);
    m_epsGradient->setToolTip(tpl::tr("Снятая галочка возвращает ровную заливку одним цветом — как у обычной "
        "диэлектрической фигуры. Считаться будет ровно то же самое: это настройка "
        "картинки, а не задания.",
        "Unchecked, the shape is filled with one flat color, like an ordinary "
        "dielectric shape. The computation is exactly the same: this is a display "
        "setting, not a task setting."));
    epsLay->addWidget(m_epsGradient);

    m_epsColorBar = new QCheckBox(tpl::tr("Показывать цветовую шкалу значений", "Show the value color scale"), epsBox);
    m_epsColorBar->setToolTip(tpl::tr("Полоса у правого края холста: внизу минимальная eps в области, вверху "
        "максимальная. Появляется только когда на сцене есть фигура с формулой.",
        "A bar at the right edge of the canvas: minimum eps in the domain at the "
        "bottom, maximum at the top. Appears only when the scene has a shape with a "
        "formula."));
    epsLay->addWidget(m_epsColorBar);

    m_epsTooltip = new QCheckBox(tpl::tr("Показывать eps под курсором", "Show eps under the cursor"), epsBox);
    epsLay->addWidget(m_epsTooltip);

    auto *palRow = new QVBoxLayout;
    palRow->setSpacing(6);
    palRow->addWidget(new QLabel(tpl::tr("Палитра", "Palette"), epsBox));
    m_palette = new QComboBox(epsBox);
    for (int i = 0; i <= int(settings::Palette::Gray); ++i)
        m_palette->addItem(settings::paletteName(settings::Palette(i)));
    palRow->addWidget(m_palette);

    // Полоска-образец: выбирать палитру по названию бессмысленно, её надо видеть.
    m_preview = new QLabel(epsBox);
    m_preview->setFixedHeight(18);
    m_preview->setScaledContents(true);
    palRow->addWidget(m_preview);
    epsLay->addLayout(palRow);

    root->addWidget(epsBox);

    // --- холст ---
    auto *viewBox = new QGroupBox(tpl::tr("Холст", "Canvas"), this);
    auto *viewLay = new QVBoxLayout(viewBox);
    m_grid = new QCheckBox(tpl::tr("Сетка", "Grid"), viewBox);
    m_snap = new QCheckBox(tpl::tr("Привязка к сетке и объектам", "Snap to grid and objects"), viewBox);
    m_dimensions = new QCheckBox(tpl::tr("Размеры выделенной фигуры", "Dimensions of the selected shape"), viewBox);
    viewLay->addWidget(m_grid);
    viewLay->addWidget(m_snap);
    viewLay->addWidget(m_dimensions);
    root->addWidget(viewBox);

    // --- поведение ---
    auto *behBox = new QGroupBox(tpl::tr("Поведение", "Behavior"), this);
    auto *behLay = new QVBoxLayout(behBox);
    m_confirmOverwrite = new QCheckBox(
        tpl::tr("Предупреждать перед записью поверх существующего файла", "Warn before overwriting an existing file"), behBox);
    behLay->addWidget(m_confirmOverwrite);
    root->addWidget(behBox);

    root->addStretch(1);

    auto *buttons = new QDialogButtonBox(this);
    auto *reset = buttons->addButton(tpl::tr("Вернуть по умолчанию", "Restore defaults"),
                                     QDialogButtonBox::ResetRole);
    buttons->addButton(tpl::tr("Закрыть", "Close"), QDialogButtonBox::AcceptRole);
    root->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(reset, &QPushButton::clicked, this, [this] {
        settings::setValues(settings::Values{});
        pull();
        emit settingsChanged();
    });

    const QVector<QCheckBox *> boxes{m_epsGradient, m_epsColorBar, m_epsTooltip,
                                     m_grid,        m_snap,        m_dimensions,
                                     m_confirmOverwrite};
    for (QCheckBox *b : boxes)
        connect(b, &QCheckBox::toggled, this, [this] { push(); });
    connect(m_palette, &QComboBox::currentIndexChanged, this, [this] { push(); });

    pull();
    dtheme::fitToScreen(this);
}

void SettingsDialog::pull()
{
    m_loading = true;
    const settings::Values &v = settings::values();
    m_epsGradient->setChecked(v.epsGradient);
    m_epsColorBar->setChecked(v.epsColorBar);
    m_epsTooltip->setChecked(v.epsTooltip);
    m_palette->setCurrentIndex(int(v.palette));
    m_grid->setChecked(v.gridVisible);
    m_snap->setChecked(v.snapEnabled);
    m_dimensions->setChecked(v.showDimensions);
    m_confirmOverwrite->setChecked(v.confirmOverwrite);
    m_loading = false;
    updatePreview();
}

void SettingsDialog::push()
{
    if (m_loading)
        return;

    settings::Values v;
    v.epsGradient = m_epsGradient->isChecked();
    v.epsColorBar = m_epsColorBar->isChecked();
    v.epsTooltip = m_epsTooltip->isChecked();
    v.palette = settings::Palette(m_palette->currentIndex());
    v.gridVisible = m_grid->isChecked();
    v.snapEnabled = m_snap->isChecked();
    v.showDimensions = m_dimensions->isChecked();
    v.confirmOverwrite = m_confirmOverwrite->isChecked();
    settings::setValues(v);

    updatePreview();
    emit settingsChanged();
}

void SettingsDialog::updatePreview()
{
    const auto p = settings::Palette(m_palette->currentIndex());
    QImage img(256, 1, QImage::Format_RGB32);
    for (int x = 0; x < img.width(); ++x)
        img.setPixelColor(x, 0, settings::paletteColor(p, double(x) / (img.width() - 1)));
    m_preview->setPixmap(QPixmap::fromImage(img));
}
