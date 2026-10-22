/// @file theme.cpp
/// @brief Реализация оформления: таблица стилей QSS, палитра, шрифты из ресурсов, пиктограммы, подсказки.
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_ui_base`.
/// @see https://www.tamic.ru/docs/code/gui-common.html#theme
#include "theme.h"

#include "aboutbox.h"
#include "i18n/tmci18n.h"

#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QFontDatabase>
#include <QHash>
#include <QGuiApplication>
#include <QIcon>
#include <QPalette>
#include <QScreen>
#include <QToolTip>
#include <QHelpEvent>
#include <QTextDocument>
#include <QWidget>

#include <algorithm>
#include <clocale>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#endif

namespace theme {

QString styleSheet()
{
    return QStringLiteral(R"(
QWidget {
    background: #0a0e16;
    color: #e7eef7;
    font-family: 'Inter', 'Segoe UI', 'DejaVu Sans', sans-serif;
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
/* Плитка кнопки нарисована в самой пиктограмме (вариант Б), поэтому у кнопки
   свой фон прозрачный; наведение — тонкая рамка акцента. Нажатое состояние
   переключателя показывает зелёный кружок на плитке, а не фон кнопки. */
QToolButton { background: transparent; border: 1px solid transparent; border-radius: 7px; padding: 1px; }
QToolButton:hover { border: 1px solid #34e3c4; }
QToolButton:pressed { background: #1b2436; border: 1px solid #34e3c4; }
QToolButton:checked { background: transparent; }
QToolButton:disabled { background: transparent; }

QStatusBar { background: #131a28; border-top: 1px solid #243049; color: #9fb0c8; }
QStatusBar::item { border: none; }

QMdiArea { background: #070a10; }
QMdiSubWindow { background: #0a0e16; border: 1px solid #243049; }
QMdiSubWindow:!active { border: 1px solid #1b2436; }

QGroupBox {
    /* Прозрачный фон обязателен: в ресурсах Windows рамка группы записана ПОСЛЕ
       своих элементов, и с непрозрачным фоном она их перекрывала бы. В Windows
       рамка тоже прозрачная — рисуется только контур с подписью. */
    background: transparent;
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

QComboBox::drop-down { border: none; width: 18px; }
QComboBox QAbstractItemView {
    background: #131a28;
    border: 1px solid #243049;
    selection-background-color: #1b2436;
    selection-color: #34e3c4;
}

QHeaderView::section {
    background: #131a28;
    color: #9fb0c8;
    border: none;
    border-right: 1px solid #243049;
    border-bottom: 1px solid #243049;
    padding: 4px 6px;
}

QPushButton {
    background: #1b2436;
    border: 1px solid #243049;
    border-radius: 8px;
    padding: 6px 14px;
    min-width: 76px;
}
QPushButton:hover { border: 1px solid #34e3c4; }
QPushButton:pressed { background: #243049; }
QPushButton:default { border: 1px solid #34e3c4; }
QPushButton:disabled { color: #6c7d97; background: #10151f; }

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

/* Диалоги перенесены из ресурсов Windows: координаты элементов рассчитаны на
   шрифт «MS Sans Serif 8». Поэтому внутри диалогов держим тот же размер шрифта
   и минимальные отступы — иначе подписи не помещаются в свои поля и обрезаются.
   Кнопкам снимаем минимальную ширину: в ресурсах она задана явно. */
/* Размер и семейство задаём КАЖДОМУ элементу внутри диалога: правило для
   QWidget с общим размером 10pt иначе перекрывает шрифт окна, и подписи
   перестают помещаться в поля с координатами из ресурсов Windows. */
QDialog, QDialog QWidget, QDialog QLabel, QDialog QPushButton, QDialog QCheckBox,
QDialog QRadioButton, QDialog QGroupBox, QDialog QLineEdit, QDialog QComboBox,
QDialog QListWidget, QDialog QSpinBox, QDialog QDoubleSpinBox, QDialog QTextEdit,
QDialog QPlainTextEdit, QDialog QProgressBar, QDialog QTreeWidget, QDialog QTabWidget {
    font-family: 'Inter', 'Segoe UI', 'Liberation Sans', sans-serif;
    font-size: 8pt;
}
QDialog QPushButton { min-width: 0; padding: 2px 6px; }
/* Стандартные окна Qt (выбор шрифта, сообщения, выбор файла, запрос пути к
   программе) раскладываются сами, и кнопки в них — в QDialogButtonBox. Им
   правило выше не подходит: OK и «Отмена» выходили мелкими. */
QDialog QDialogButtonBox QPushButton, QMessageBox QPushButton {
    min-width: 88px;
    min-height: 26px;
    padding: 4px 14px;
    font-size: 9pt;
}
QDialog QLineEdit, QDialog QComboBox, QDialog QSpinBox, QDialog QDoubleSpinBox {
    padding: 1px 4px;
    border-radius: 5px;
}
/* Многострочные поля в ресурсах Windows часто высотой в одну строку (13 единиц
   диалога). С обычными отступами текст в такое поле не помещается вовсе. */
QDialog QPlainTextEdit, QDialog QTextEdit {
    padding: 0px 2px;
    border-radius: 5px;
}
QDialog QGroupBox { margin-top: 14px; padding: 8px 6px 6px 6px; border-radius: 8px; }
/* Значок флажка и переключателя в Windows заметно меньше нашего, а координаты
   подписей взяты из ресурсов Windows. Поэтому внутри диалогов уменьшаем значок
   и зазор — иначе подпись не помещается и обрезается. */
QDialog QCheckBox, QDialog QRadioButton { spacing: 3px; }
QDialog QCheckBox::indicator, QDialog QRadioButton::indicator {
    width: 12px;
    height: 12px;
}
/* Переключатель — кружок и в уменьшенном виде: со скруглением 8px от
   обычного размера неотмеченный рисовался квадратом, как флажок. */
QDialog QRadioButton::indicator { border-radius: 6px; }
QDialog QRadioButton::indicator:checked {
    border: 1px solid #34e3c4;
    background: qradialgradient(cx: 0.5, cy: 0.5, radius: 0.5, fx: 0.5, fy: 0.5,
                                stop: 0 #34e3c4, stop: 0.5 #34e3c4, stop: 0.56 #0a0e16, stop: 1 #0a0e16);
}
QDialog QGroupBox::title { left: 6px; padding: 0 3px; }
QDialog QLabel { padding: 0; }

/* Ход расчёта в строке состояния (как в Android-версии): этап и тонкая полоса. */
QProgressBar#tmcProgress { background: #243049; border: none; border-radius: 3px; margin: 0 6px; }
QProgressBar#tmcProgress::chunk { background: #34e3c4; border-radius: 3px; }
QLabel#tmcProgressStage { color: #34e3c4; padding: 0 4px; font-weight: 600; }
QLabel#tmcProgressStage[state="ok"] { color: #5ce08a; }
QLabel#tmcProgressStage[state="err"] { color: #ff6b6b; }
QLabel#tmcProgressStage[state="warn"] { color: #ffd166; }
QLabel#tmcComputeStatus { color: #9fb0c8; padding: 0 6px; }

QToolTip {
    font-family: 'Inter', 'Segoe UI', 'DejaVu Sans', sans-serif;
    font-size: 10pt;
    background: #131a28;
    color: #e7eef7;
    border: 1px solid #34e3c4;
    padding: 4px 6px;
}
)");
}

namespace {
// Подсказки — одной строкой. Обычная (не размеченная) подсказка в нашем
// оформлении обрезалась: «Правка во» вместо «Правка во внешнем редакторе».
// Размеченный текст Qt меряет тем же шрифтом, каким рисует, поэтому
// показываем подсказку как <nobr>…</nobr>.
/// @brief Фильтр приложения: показывает простые подсказки одной строкой (`<nobr>`), переведёнными.
class TipFilter : public QObject
{
public:
    using QObject::QObject;
    /// @brief Перехватывает событие подсказки виджета.
    /// @param[in] o  объект события
    /// @param[in] e  событие
    /// @return true — подсказка показана фильтром; false — обработку продолжает Qt (не подсказка, пустая или размеченная).
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() != QEvent::ToolTip || !o->isWidgetType())
            return false;
        QWidget *w = static_cast<QWidget *>(o);
        // Перевод — до показа: переводчик окон переводил уже открытое окно
        // подсказки, размер которого был рассчитан на английский текст.
        const QString tip = tmci18n::translate(w->toolTip());
        if (tip.isEmpty() || Qt::mightBeRichText(tip))
            return false;
        QString html = tip.toHtmlEscaped();
        html.replace(QLatin1Char('\n'), QLatin1String("<br>"));
        QToolTip::showText(static_cast<QHelpEvent *>(e)->globalPos(),
                           QStringLiteral("<nobr>") + html + QStringLiteral("</nobr>"), w);
        return true;
    }
};

/// Имя программы для поиска пиктограмм.
QString g_program;
/// Гарнитура интерфейса, фактически загруженная из ресурсов.
QString g_uiFamily = QStringLiteral("Inter");
/// Моноширинная гарнитура, фактически загруженная из ресурсов.
QString g_monoFamily = QStringLiteral("JetBrains Mono");

/// @brief Регистрирует шрифты Inter и JetBrains Mono из ресурсов `:/tmcfonts/` (один раз).
void loadFonts()
{
    static bool done = false;
    if (done)
        return;
    done = true;
    const char *files[] = { "Inter-Regular.ttf", "Inter-SemiBold.ttf", "Inter-Bold.ttf",
                            "Inter-ExtraBold.ttf", "JetBrainsMono-Regular.ttf",
                            "JetBrainsMono-Bold.ttf" };
    for (const char *f : files) {
        const int id = QFontDatabase::addApplicationFont(QStringLiteral(":/tmcfonts/") + QLatin1String(f));
        if (id < 0)
            continue;
        const QStringList fam = QFontDatabase::applicationFontFamilies(id);
        if (fam.isEmpty())
            continue;
        if (QLatin1String(f).startsWith(QLatin1String("Inter")))
            g_uiFamily = fam.first();
        else
            g_monoFamily = fam.first();
    }
}
} // namespace

void setProgram(const QString &prog) { g_program = prog; }
QString program() { return g_program; }
QString uiFontFamily() { loadFonts(); return g_uiFamily; }
QString monoFontFamily() { loadFonts(); return g_monoFamily; }

QIcon appIcon()
{
    QIcon icon;
    const QString sm = QStringLiteral(":/tmcicons/app/") + g_program + QStringLiteral("_sm.svg");
    const QString svg = QStringLiteral(":/tmcicons/app/") + g_program + QStringLiteral(".svg");
    if (QFile::exists(sm)) {
        icon.addFile(sm, QSize(16, 16));
        icon.addFile(sm, QSize(24, 24));
        icon.addFile(sm, QSize(32, 32));
    }
    if (QFile::exists(svg)) {
        icon.addFile(svg, QSize(48, 48));
        icon.addFile(svg, QSize(256, 256));
    }
    return icon;
}

QIcon commandIconState(const QString &commandId, bool on)
{
    const QString base = QStringLiteral(":/tmcicons/toolbar/") + g_program + QLatin1Char('/') + commandId;
    const QString f = base + (on ? QStringLiteral("_on.svg") : QStringLiteral("_off.svg"));
    return QFile::exists(f) ? QIcon(f) : QIcon();
}

bool programRunning(const QString &exeName)
{
#ifdef _WIN32
    static QHash<QString, bool> cache;
    static qint64 stamp = 0;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - stamp > 1000) {
        stamp = now;
        cache.clear();
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 pe;
            pe.dwSize = sizeof(pe);
            for (BOOL okp = Process32First(snap, &pe); okp; okp = Process32Next(snap, &pe))
                cache.insert(QString::fromLocal8Bit(pe.szExeFile).toLower(), true);
            CloseHandle(snap);
        }
    }
    return cache.value(exeName.toLower(), false);
#else
    Q_UNUSED(exeName);
    return false;
#endif
}

void apply()
{
    // Числа в файлах TMC пишутся и читаются только с ТОЧКОЙ — так их создаёт
    // Windows-версия, и так их ждут все sscanf("%lg") в перенесённом коде.
    //
    // Конструктор QApplication вызывает setlocale(LC_ALL, "") и берёт числовую
    // локаль из окружения. Из ярлыка рабочего стола переменной LANG может не
    // быть и локаль остаётся «C», а вот из терминала приходит язык системы: в
    // русской локали разделитель — запятая. Тогда sscanf спотыкается на первой
    // же точке (файл топологии .tt читается как «Bad topology file»), а
    // printf("%g") записал бы запятые в выходные файлы — и они перестали бы
    // совпадать с Windows байт в байт.
    //
    // Локаль Qt (QLocale) этим не затрагивается: подписи интерфейса и разбор
    // введённых человеком чисел живут отдельно от локали языка C.
    std::setlocale(LC_NUMERIC, "C");

    // Окно «О программе»: адрес сайта и руководство делаем ссылками,
    // к версии приписываем систему — как в Windows-версии.
    loadFonts();
    if (!g_program.isEmpty())
        qApp->setWindowIcon(appIcon());
    tmcabout::install();
    tmci18n::install();   // язык интерфейса: русский / английский

    QPalette p;
    p.setColor(QPalette::Window, bg());
    p.setColor(QPalette::WindowText, text());
    p.setColor(QPalette::Base, bg());
    p.setColor(QPalette::AlternateBase, surface());
    p.setColor(QPalette::Text, text());
    p.setColor(QPalette::Button, surface());
    p.setColor(QPalette::ButtonText, text());
    p.setColor(QPalette::Highlight, accent());
    p.setColor(QPalette::HighlightedText, bg());
    p.setColor(QPalette::ToolTipBase, surface());
    p.setColor(QPalette::ToolTipText, text());
    p.setColor(QPalette::Disabled, QPalette::Text, textMute());
    p.setColor(QPalette::Disabled, QPalette::ButtonText, textMute());
    qApp->setPalette(p);
    // Подсказка меряет свой размер шрифтом QToolTip::font(), а рисуется шрифтом
    // из таблицы стилей. Пока они разные, длинная подсказка обрезалась
    // («Правка во» вместо «Правка во внешнем редакторе»). Задаём один шрифт.
    QFont tipFont(g_uiFamily);
    tipFont.setPointSize(10);
    QToolTip::setFont(tipFont);
    static TipFilter *tipFilter = nullptr;
    if (!tipFilter) {
        tipFilter = new TipFilter(qApp);
        qApp->installEventFilter(tipFilter);
    }
    qApp->setStyleSheet(styleSheet());
}

QIcon commandIcon(const QString &commandId)
{
    if (!g_program.isEmpty()) {
        const QString base = QStringLiteral(":/tmcicons/toolbar/") + g_program + QLatin1Char('/') + commandId;
        if (QFile::exists(base + QStringLiteral(".svg")))
            return QIcon(base + QStringLiteral(".svg"));
        if (QFile::exists(base + QStringLiteral("_off.svg"))) {
            QIcon icon;
            for (QIcon::Mode m : { QIcon::Normal, QIcon::Disabled, QIcon::Active }) {
                icon.addFile(base + QStringLiteral("_off.svg"), QSize(), m, QIcon::Off);
                icon.addFile(base + QStringLiteral("_on.svg"), QSize(), m, QIcon::On);
            }
            return icon;
        }
    }
    // Пиктограммы вырезаны из ресурсов Windows-версии (средством extract_icons).
    // Для тёмной темы у каждой есть вариант со светлым штрихом: форма та же,
    // читаемость на тёмном фоне появляется.
    const QString dark = QStringLiteral(":/icons/") + commandId + QStringLiteral("_dark.png");
    if (QFile::exists(dark))
        return QIcon(dark);
    return QIcon(QStringLiteral(":/icons/") + commandId + QStringLiteral(".png"));
}

void fitToScreen(QWidget *w)
{
    if (!w)
        return;
    const QScreen *screen = w->screen() ? w->screen() : QGuiApplication::primaryScreen();
    if (!screen)
        return;
    const QRect avail = screen->availableGeometry();
    const QSize hint = w->sizeHint();

    w->setMinimumHeight(0);
    w->setMaximumHeight(avail.height());
    w->resize(std::min(hint.width(), int(avail.width() * 0.95)),
              std::min(hint.height(), int(avail.height() * 0.88)));

    const QPoint c = avail.center();
    w->move(std::max(avail.left(), c.x() - w->width() / 2),
            std::max(avail.top(), c.y() - w->height() / 2));
}

} // namespace theme
