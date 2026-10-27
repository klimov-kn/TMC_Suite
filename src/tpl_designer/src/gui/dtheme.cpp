/// @file dtheme.cpp
/// @brief Таблица стилей TPL Designer и ужатие окон до экрана.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-theme
#include "dtheme.h"

#include <QGuiApplication>
#include <QScreen>
#include <QWidget>

#include <algorithm>

namespace dtheme {

void fitToScreen(QWidget *w)
{
    if (!w)
        return;
    const QScreen *screen = w->screen() ? w->screen() : QGuiApplication::primaryScreen();
    if (!screen)
        return;
    const QRect avail = screen->availableGeometry();
    const QSize hint = w->sizeHint();

    // Минимум обнуляем по высоте: иначе окно физически не даст себя сжать,
    // сколько ни тяни за угол.
    w->setMinimumHeight(0);
    w->setMaximumHeight(avail.height());
    w->resize(std::min(hint.width(), int(avail.width() * 0.95)),
              std::min(hint.height(), int(avail.height() * 0.88)));

    const QPoint c = avail.center();
    w->move(std::max(avail.left(), c.x() - w->width() / 2),
            std::max(avail.top(), c.y() - w->height() / 2));
}

// Оформление — как у основных программ TMC Suite 5 (gui_common/theme.cpp Qt6-версии пакета),
// ответ автора 26.10.2026 (О-2): «палитру основных программ версии 5 применить, дизайн должен полностью
// совпадать». Общие элементы (окна, меню, кнопки, поля, флажки, полосы прокрутки, вкладки, строка
// состояния) — те же значения, что в программах пакета. Ниже отдельно — то, что есть только в Designer:
// акцентная кнопка, палитра инструментов холста, карточки, заголовки, стрелки счётчиков.
// Прежняя таблица — theme.cpp.bak_2026-10-26.
QString styleSheet()
{
    return QStringLiteral(R"(
QWidget {
    background: #0a0e16;
    color: #e7eef7;
    font-family: 'Inter', 'Segoe UI', -apple-system, 'SF Pro Text', 'Helvetica Neue',
                 'Cantarell', 'Noto Sans', 'DejaVu Sans', sans-serif;
    font-size: 10pt;
}

QMainWindow::separator { background: #243049; width: 1px; height: 1px; }

QMenuBar { background: #131a28; border-bottom: 1px solid #243049; }
QMenuBar::item { padding: 5px 10px; background: transparent; }
QMenuBar::item:selected { background: #1b2436; color: #34e3c4; }

QMenu { background: #131a28; border: 1px solid #243049; padding: 4px; }
QMenu::item { padding: 5px 26px 5px 22px; }
QMenu::item:selected { background: #1b2436; color: #34e3c4; }
QMenu::item:disabled { color: #6c7d97; }
QMenu::separator { height: 1px; background: #243049; margin: 4px 8px; }

QToolBar {
    background: #131a28;
    border-bottom: 1px solid #243049;
    padding: 3px;
    spacing: 2px;
}
QToolBar::separator { background: #243049; width: 1px; margin: 3px 5px; }
QToolButton { background: transparent; border: 1px solid transparent; border-radius: 7px; padding: 4px 8px; color: #e7eef7; }
QToolButton:hover { border: 1px solid #34e3c4; }
QToolButton:pressed { background: #1b2436; border: 1px solid #34e3c4; }
/* В программах пакета нажатое состояние показывает значок на плитке; у Designer кнопки текстовые,
   поэтому нажатое состояние — заливка «нажатой» кнопки пакета и рамка акцента. */
QToolButton:checked { background: #1b2436; border: 1px solid #34e3c4; color: #34e3c4; }
QToolButton:disabled { background: transparent; color: #6c7d97; }

QStatusBar { background: #131a28; border-top: 1px solid #243049; color: #9fb0c8; }
QStatusBar::item { border: none; }

QGroupBox {
    background: #131a28;
    border: 1px solid #243049;
    border-radius: 12px;
    margin-top: 18px;
    padding: 12px 10px 10px 10px;
    font-weight: 600;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 12px;
    top: 2px;
    padding: 0 6px;
    color: #34e3c4;
}

QLabel { background: transparent; }
QLabel[dim="true"] { color: #9fb0c8; }
QLabel[mute="true"] { color: #6c7d97; }
QLabel[h1="true"] { font-size: 22pt; font-weight: 700; }
QLabel[h2="true"] { font-size: 13pt; font-weight: 600; color: #e7eef7; }

QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QPlainTextEdit, QTextEdit, QListWidget, QTreeWidget, QTableWidget {
    background: #0a0e16;
    border: 1px solid #243049;
    border-radius: 8px;
    padding: 4px 6px;
    selection-background-color: #34e3c4;
    selection-color: #0a0e16;
}
QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QPlainTextEdit:focus {
    border: 1px solid #34e3c4;
}
QLineEdit:disabled, QSpinBox:disabled, QDoubleSpinBox:disabled, QComboBox:disabled {
    color: #6c7d97;
    background: #10151f;
}

/* Кнопки счётчика задаём ЯВНО (только в Designer: его счётчики со скруглением и отступом).
 * Как только у QSpinBox застилизованы padding и border-radius, Qt перестаёт рисовать
 * его средствами системной темы и раскладывает подэлементы по правилам QSS. Без
 * описания up-button верхняя кнопка уезжает под padding: стрелка видна, а нажимается
 * только нижняя. */
QAbstractSpinBox { padding-right: 22px; }
QAbstractSpinBox::up-button, QAbstractSpinBox::down-button {
    subcontrol-origin: border;
    background: #131a28;
    border-left: 1px solid #243049;
    width: 20px;
}
QAbstractSpinBox::up-button {
    subcontrol-position: top right;
    border-top-right-radius: 8px;
    border-bottom: 1px solid #243049;
    margin: 1px 1px 0 0;
}
QAbstractSpinBox::down-button {
    subcontrol-position: bottom right;
    border-bottom-right-radius: 8px;
    margin: 0 1px 1px 0;
}
QAbstractSpinBox::up-button:hover, QAbstractSpinBox::down-button:hover { background: #1b2436; }
QAbstractSpinBox::up-button:pressed, QAbstractSpinBox::down-button:pressed { background: #34e3c4; }
/* Стрелки — только картинкой: другого способа Qt не даёт. */
QAbstractSpinBox::up-arrow   { image: url(:/icons/resources/arrow_up.svg);   width: 9px; height: 6px; }
QAbstractSpinBox::down-arrow { image: url(:/icons/resources/arrow_down.svg); width: 9px; height: 6px; }
QAbstractSpinBox::up-arrow:disabled, QAbstractSpinBox::down-arrow:disabled { opacity: 80; }

QComboBox::drop-down { border: none; width: 18px; }
QComboBox::down-arrow {
    image: url(:/icons/resources/arrow_down.svg);
    width: 9px; height: 6px;
    margin-right: 6px;
}
QComboBox QAbstractItemView {
    background: #131a28;
    border: 1px solid #243049;
    selection-background-color: #1b2436;
    selection-color: #34e3c4;
    outline: none;
}

QHeaderView::section {
    background: #131a28;
    color: #9fb0c8;
    border: none;
    border-right: 1px solid #243049;
    border-bottom: 1px solid #243049;
    padding: 4px 6px;
}

QTreeWidget, QListWidget { outline: none; }
QTreeWidget::item, QListWidget::item { padding: 5px 6px; }
QTreeWidget::item:selected, QListWidget::item:selected { background: #1b2436; color: #34e3c4; }
QTreeWidget::item:hover, QListWidget::item:hover { background: #131a28; }

QPushButton {
    background: #1b2436;
    border: 1px solid #243049;
    border-radius: 8px;
    padding: 6px 14px;
    min-width: 76px;
    color: #e7eef7;
}
QPushButton:hover { border: 1px solid #34e3c4; }
QPushButton:pressed { background: #243049; }
QPushButton:default { border: 1px solid #34e3c4; }
QPushButton:disabled { color: #6c7d97; background: #10151f; }

/* Только в Designer: главная кнопка шага («Далее», «Сгенерировать»). */
QPushButton[accent="true"] {
    background: #34e3c4;
    color: #06231e;
    border: 1px solid #34e3c4;
    font-weight: 700;
}
QPushButton[accent="true"]:hover { background: #5cebd1; }
QPushButton[accent="true"]:disabled { background: #1e3f39; color: #4d6b66; border: 1px solid #1e3f39; }

/* Только в Designer: палитра инструментов холста — переключатели, всегда видны как кнопки. */
QToolButton[tool="true"] {
    background: #1b2436;
    border: 1px solid #243049;
    border-radius: 8px;
    padding: 5px 8px;
    text-align: left;
}
QToolButton[tool="true"]:hover { border: 1px solid #34e3c4; }
QToolButton[tool="true"]:checked { background: #34e3c4; border: 1px solid #34e3c4; color: #06231e; }

QCheckBox, QRadioButton { background: transparent; spacing: 7px; }
QCheckBox::indicator, QRadioButton::indicator { width: 15px; height: 15px; }
QCheckBox::indicator {
    background: #0a0e16; border: 1px solid #243049; border-radius: 4px;
}
QCheckBox::indicator:checked { background: #34e3c4; border: 1px solid #34e3c4; }
QRadioButton::indicator {
    background: #0a0e16; border: 1px solid #243049; border-radius: 8px;
}
QRadioButton::indicator:checked { background: #34e3c4; border: 3px solid #0a0e16; }
QCheckBox:disabled, QRadioButton:disabled { color: #6c7d97; }

QScrollBar:vertical { background: #0a0e16; width: 12px; margin: 0; }
QScrollBar:horizontal { background: #0a0e16; height: 12px; margin: 0; }
QScrollBar::handle { background: #243049; border-radius: 6px; min-height: 26px; min-width: 26px; }
QScrollBar::handle:hover { background: #34506b; }
QScrollBar::add-line, QScrollBar::sub-line { height: 0; width: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }

QTabBar::tab {
    background: #131a28;
    border: 1px solid #243049;
    padding: 5px 12px;
    margin-right: 2px;
    border-top-left-radius: 8px;
    border-top-right-radius: 8px;
}
QTabBar::tab:selected { color: #34e3c4; border-bottom: 1px solid #0a0e16; }

QSplitter::handle { background: #243049; }
QSplitter::handle:horizontal { width: 1px; }

/* Только в Designer: карточки стартового экрана и разделители. */
QFrame[card="true"] {
    background: #131a28;
    border: 1px solid #243049;
    border-radius: 12px;
}
QFrame[hline="true"] { background: #243049; max-height: 1px; border: none; }

QToolTip {
    background: #131a28;
    color: #e7eef7;
    border: 1px solid #34e3c4;
    border-radius: 6px;
    padding: 5px;
}
)");
}

} // namespace dtheme
