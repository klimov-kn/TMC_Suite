// Оформление FieldView — описание в theme.h.
/// @file theme.cpp
/// @brief Шрифты, палитра Qt и таблица стилей оформления FieldView.
///
/// Часть модуля FieldView (слой `app`). Описание функций — в `theme.h`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#theme
#include "theme.h"

#include <QApplication>
#include <QFontDatabase>
#include <QPalette>
#include <QStyleFactory>

namespace theme {

QFont uiFont(int pixelSize, int weight)
{
    QFont f(fontUi());
    f.setPixelSize(pixelSize);
    f.setWeight(QFont::Weight(weight));
    return f;
}

QFont monoFont(int pixelSize, int weight)
{
    QFont f(fontMono());
    f.setPixelSize(pixelSize);
    f.setWeight(QFont::Weight(weight));
    return f;
}

namespace {

/// @brief Регистрирует встроенные шрифты Inter и JetBrains Mono из ресурсов.
void loadFonts()
{
    // Шрифты встроены (SIL OFL 1.1): на системные не полагаемся.
    const char *files[] = {":/fonts/Inter-Regular.ttf",       ":/fonts/Inter-SemiBold.ttf",
                           ":/fonts/Inter-Bold.ttf",          ":/fonts/Inter-ExtraBold.ttf",
                           ":/fonts/JetBrainsMono-Regular.ttf", ":/fonts/JetBrainsMono-Bold.ttf"};
    for (const char *f : files)
        QFontDatabase::addApplicationFont(QString::fromLatin1(f));
}

/// @brief Возвращает таблицу стилей виджетов (тёмное оформление пакета).
/// @return Текст таблицы стилей Qt.
QString styleSheet()
{
    return QStringLiteral(R"(
QWidget {
    background: #0a0e16;
    color: #e7eef7;
    font-family: 'Inter';
    font-size: 13px;
}
QMainWindow::separator { background: #243049; width: 1px; height: 1px; }

QMenuBar { background: #0e1320; border-bottom: 1px solid #243049; padding: 2px 4px; }
QMenuBar::item { background: transparent; padding: 6px 12px; border-radius: 8px; }
QMenuBar::item:selected { background: #1a2335; color: #34e3c4; }
QMenuBar::item:pressed { background: #243049; color: #34e3c4; }

QMenu {
    background: #131a28;
    border: 1px solid #243049;
    border-radius: 10px;
    padding: 6px;
}
QMenu::item { padding: 7px 28px 7px 30px; border-radius: 6px; background: transparent; }
QMenu::item:selected { background: #243049; color: #34e3c4; }
QMenu::item:disabled { color: #4a5568; }
QMenu::separator { height: 1px; background: #243049; margin: 4px 8px; }
QMenu::icon { padding-left: 8px; }
QMenu::indicator { width: 14px; height: 14px; left: 8px; }
QMenu::indicator:non-exclusive:unchecked { border: 1px solid #3a4a6b; border-radius: 4px; background: #0a0e16; }
QMenu::indicator:non-exclusive:checked { border: 1px solid #34e3c4; border-radius: 4px; background: #34e3c4; image: url(:/ui/check.svg); }
QMenu::indicator:exclusive:unchecked { border: 1px solid #3a4a6b; border-radius: 7px; background: #0a0e16; }
QMenu::indicator:exclusive:checked { border: 4px solid #0a0e16; border-radius: 7px; background: #34e3c4; }

QToolBar {
    background: #0e1320;
    border: none;
    border-bottom: 1px solid #243049;
    spacing: 3px;
    padding: 5px 6px;
}
QToolBar::separator { background: #243049; width: 1px; margin: 4px 6px; }
QToolButton {
    background: #131a28;
    border: 1px solid #1d2740;
    border-radius: 8px;
    padding: 4px;
    color: #e7eef7;
}
QToolButton:hover { border-color: #34e3c4; background: #1a2335; }
QToolButton:pressed { background: #243049; }
QToolButton:checked { background: #173a37; border-color: #34e3c4; }
QToolButton:disabled { background: #10151f; border-color: #151c2b; }
QToolButton[textButton="true"] { padding: 4px 10px; }

QStatusBar { background: #0e1320; border-top: 1px solid #243049; color: #9fb0c8; }
QStatusBar::item { border: none; }
QStatusBar QLabel { color: #9fb0c8; padding: 0 8px; font-family: 'JetBrains Mono'; font-size: 12px; }

QMdiArea { background: #0a0e16; }
QMdiSubWindow { background: #0a0e16; border: 1px solid #243049; }

QTabWidget::pane { border: 1px solid #243049; border-radius: 12px; top: -1px; background: #0e1320; }
QTabBar::tab {
    background: #131a28; border: 1px solid #243049; border-bottom: none;
    padding: 8px 16px; margin-right: 4px;
    border-top-left-radius: 10px; border-top-right-radius: 10px; color: #9fb0c8;
}
QTabBar::tab:selected { background: #0e1320; color: #34e3c4; font-weight: 600; }
QTabBar::tab:hover { color: #e7eef7; }

QGroupBox {
    background: #131a28;
    border: 1px solid #243049;
    border-radius: 12px;
    margin-top: 18px;
    padding: 12px 10px 10px 10px;
    font-weight: 600;
}
QGroupBox::title { subcontrol-origin: margin; left: 12px; top: 0px; padding: 0 6px; color: #34e3c4; background: transparent; }
QGroupBox * { background: transparent; }
QGroupBox QLineEdit, QGroupBox QDoubleSpinBox, QGroupBox QSpinBox, QGroupBox QComboBox { background: #0a0e16; }
QGroupBox QPushButton { background: #1a2335; }

QLabel { background: transparent; }
QLabel[dim="true"] { color: #9fb0c8; }
QLabel[mute="true"] { color: #6c7d97; }
QLabel[mono="true"] { font-family: 'JetBrains Mono'; }
QLabel[h1="true"] { font-size: 22px; font-weight: 800; }

QLineEdit, QDoubleSpinBox, QSpinBox, QComboBox, QPlainTextEdit, QTextEdit {
    background: #0a0e16;
    border: 1px solid #243049;
    border-radius: 10px;
    padding: 5px 8px;
    selection-background-color: #34e3c4;
    selection-color: #0a0e16;
}
QLineEdit:focus, QDoubleSpinBox:focus, QSpinBox:focus, QComboBox:focus { border: 1px solid #34e3c4; }
QLineEdit:disabled, QDoubleSpinBox:disabled, QSpinBox:disabled, QComboBox:disabled { color: #6c7d97; background: #10151f; }
QLineEdit[readOnly="true"] { color: #9fb0c8; background: #10151f; }

QAbstractSpinBox { padding-right: 22px; }
QAbstractSpinBox::up-button, QAbstractSpinBox::down-button {
    subcontrol-origin: border; background: #131a28; border-left: 1px solid #243049; width: 20px;
}
QAbstractSpinBox::up-button { subcontrol-position: top right; border-top-right-radius: 10px; border-bottom: 1px solid #243049; margin: 1px 1px 0 0; }
QAbstractSpinBox::down-button { subcontrol-position: bottom right; border-bottom-right-radius: 10px; margin: 0 1px 1px 0; }
QAbstractSpinBox::up-button:hover, QAbstractSpinBox::down-button:hover { background: #1b2436; }
QAbstractSpinBox::up-arrow { image: url(:/ui/arrow_up.svg); width: 9px; height: 6px; }
QAbstractSpinBox::down-arrow { image: url(:/ui/arrow_down.svg); width: 9px; height: 6px; }

QComboBox::drop-down { border: none; width: 22px; }
QComboBox::down-arrow { image: url(:/ui/arrow_down.svg); width: 9px; height: 6px; margin-right: 8px; }
QComboBox QAbstractItemView { background: #131a28; border: 1px solid #243049; selection-background-color: #243049; outline: none; }

QPushButton {
    background: #131a28;
    border: 1px solid #243049;
    border-radius: 10px;
    padding: 7px 16px;
    color: #e7eef7;
}
QPushButton:hover { border-color: #34e3c4; }
QPushButton:pressed { background: #0f1520; }
QPushButton:disabled { color: #6c7d97; border-color: #1a2233; }
QPushButton:default, QPushButton[accent="true"] { background: #34e3c4; color: #06231e; border: none; font-weight: 700; }
QPushButton:default:hover, QPushButton[accent="true"]:hover { background: #5cebd1; }

QCheckBox, QRadioButton { spacing: 8px; background: transparent; }
QCheckBox::indicator, QRadioButton::indicator { width: 16px; height: 16px; }
QCheckBox::indicator { border: 1px solid #3a4a6b; border-radius: 4px; background: #0a0e16; }
QCheckBox::indicator:checked { background: #34e3c4; border-color: #34e3c4; image: url(:/ui/check.svg); }
QRadioButton::indicator { border: 1px solid #3a4a6b; border-radius: 8px; background: #0a0e16; }
QRadioButton::indicator:checked { background: #34e3c4; border: 4px solid #0a0e16; border-radius: 9px; width: 10px; height: 10px; }
QCheckBox:disabled, QRadioButton:disabled { color: #6c7d97; }

QSlider::groove:horizontal { height: 4px; background: #243049; border-radius: 2px; }
QSlider::sub-page:horizontal { background: #34e3c4; border-radius: 2px; }
QSlider::handle:horizontal { background: #e7eef7; border: 2px solid #34e3c4; width: 12px; height: 12px; margin: -6px 0; border-radius: 8px; }
QSlider::handle:horizontal:disabled { background: #3a4a6b; border-color: #3a4a6b; }
QSlider::sub-page:horizontal:disabled { background: #3a4a6b; }

QScrollBar:vertical, QScrollBar:horizontal { background: transparent; width: 10px; height: 10px; margin: 0; }
QScrollBar::handle:vertical, QScrollBar::handle:horizontal { background: #243049; border-radius: 5px; min-height: 30px; min-width: 30px; }
QScrollBar::handle:hover { background: #34e3c4; }
QScrollBar::add-line, QScrollBar::sub-line { height: 0; width: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }

QDockWidget { color: #e7eef7; titlebar-close-icon: none; }
QDockWidget::title { background: #0e1320; padding: 6px 10px; border-bottom: 1px solid #243049; text-align: left; }

QTableWidget, QListWidget, QTreeWidget {
    background: #0e1320; border: 1px solid #243049; border-radius: 10px; gridline-color: #1a2335; outline: none;
}
QHeaderView::section { background: #131a28; color: #9fb0c8; border: none; border-bottom: 1px solid #243049; padding: 4px 6px; }

QToolTip { background: #131a28; color: #e7eef7; border: 1px solid #243049; padding: 6px 8px; }
QDialog { background: #0e1320; }
)");
}

} // namespace

void apply(QApplication &app)
{
    loadFonts();
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QFont f = uiFont(13);
    app.setFont(f);
    QPalette p;
    p.setColor(QPalette::Window, bg());
    p.setColor(QPalette::WindowText, text());
    p.setColor(QPalette::Base, bg());
    p.setColor(QPalette::AlternateBase, bg2());
    p.setColor(QPalette::Text, text());
    p.setColor(QPalette::Button, surface());
    p.setColor(QPalette::ButtonText, text());
    p.setColor(QPalette::Highlight, accent());
    p.setColor(QPalette::HighlightedText, bg());
    p.setColor(QPalette::ToolTipBase, surface());
    p.setColor(QPalette::ToolTipText, text());
    p.setColor(QPalette::PlaceholderText, textMute());
    p.setColor(QPalette::Disabled, QPalette::Text, textMute());
    p.setColor(QPalette::Disabled, QPalette::WindowText, textMute());
    p.setColor(QPalette::Disabled, QPalette::ButtonText, textMute());
    p.setColor(QPalette::Link, accent());
    app.setPalette(p);
    app.setStyleSheet(styleSheet());
}

} // namespace theme
