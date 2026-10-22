// Единое окно «О программе».
//
// Окно одно на все шесть программ пакета; различаются только значок, название
// и строка описания — их окно берёт по имени программы theme::program()
// (planarrt_h, planarrt_x, tmcgrout, tmcros, tmc_dn, fieldview, tpldesigner).
//
// Значки программ — :/tmcicons/app/<программа>.svg, значки ссылок —
// :/tmcicons/links/<имя>.svg (файлы resources/icons/links). Если значка ссылки
// в ресурсах нет, вместо него рисуется первая буква названия — окно остаётся
// рабочим.
//
// Адреса профилей автора — те же, что в мобильной версии (LinkIcons.qml) и в
// установщике Windows-версии. Руководство пользователя ищется рядом с
// программой (как в TmcDocLink.h Windows-версии); не нашли — открываем
// руководство на сайте.

/// @file aboutbox.cpp
/// @brief Реализация окна «О программе» и хранилище оформителя окон `tmc_set_dialog_decorator`.
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_ui_base`. Хранилище
/// оформителя размещено здесь, а не в слое MFC, чтобы TPL Designer мог взять окно
/// без слоя совместимости. Сведения о программе — по `theme::program()`.
/// @see https://www.tamic.ru/docs/code/gui-common.html#about
#include "aboutbox.h"
#include "i18n/tmci18n.h"

#include "theme.h"

#include <QAbstractButton>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QEasingCurve>
#include <QFocusEvent>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QRegularExpression>
#include <QSvgRenderer>
#include <QUrl>
#include <QVBoxLayout>
#include <QVariantAnimation>
#include <QWidget>

#include <functional>

// Точка расширения слоя совместимости (объявлена в src/compat/include/mfc_dialog.h).
// Хранилище оформителя — здесь, в библиотеке оформления, а слой MFC его только читает:
// так окно «О программе» не тянет за собой слой MFC, и его может взять TPL Designer.
/// Тип оформителя окна (то же объявление, что в `compat/include/mfc_dialog.h`).
typedef void (*TmcDialogDecorator)(void *qDialog);
void tmc_set_dialog_decorator(TmcDialogDecorator fn);
TmcDialogDecorator tmc_dialog_decorator();

namespace {
/// Хранилище оформителя окон.
TmcDialogDecorator g_dialogDecorator = nullptr;
}

void tmc_set_dialog_decorator(TmcDialogDecorator fn)
{
    g_dialogDecorator = fn;
}

TmcDialogDecorator tmc_dialog_decorator()
{
    return g_dialogDecorator;
}

namespace {

// --- Данные окна -----------------------------------------------------------------

/// Версия пакета, показываемая в окне.
const char kPackageVersion[] = "5.1";
/// Год выпуска.
const char kYear[]           = "2026";
/// Автор пакета (UTF-8).
const char kAuthor[]         = "К. Н. Климов";

/// Адрес сайта.
const char kSiteUrl[]   = "https://www.tamic.ru";
/// Подпись ссылки на сайт.
const char kSiteText[]  = "www.tamic.ru";
/// Адрес руководства пользователя на сайте (если PDF рядом с программой не найден).
const char kOnlineDoc[] = "https://www.tamic.ru/docs/user-manual/index.html";

// Имя файла руководства, который кладётся рядом с программами в поставке.
/// Имя файла руководства пакета рядом с программами.
const char kManualFile[] = "TMC_Suite_User_Manual.pdf";
/// Имя файла руководства TPL Designer.
const char kDesignerManualFile[] = "TPL-Designer-rukovodstvo.pdf";

/// @brief Ссылка на профиль автора: значок, подпись, адрес.
struct ProfileLink
{
    /// Имя файла значка в `:/tmcicons/links/` без расширения.
    const char *icon;   // имя файла значка в :/tmcicons/links/
    /// Подпись под значком.
    const char *name;   // подпись под значком
    /// Адрес профиля.
    const char *url;
};

// Профили автора — из мобильной версии (TMC_Suite_Mobile/src/app/qml/LinkIcons.qml).
/// Профили автора, показываемые кнопками-ссылками.
const ProfileLink kProfiles[] = {
    { "github",       "GitHub",       "https://github.com/klimov-kn" },
    { "linkedin",     "LinkedIn",     "https://www.linkedin.com/in/konstantinklimov" },
    { "researchgate", "ResearchGate", "https://www.researchgate.net/profile/Konstantin-Klimov" },
    { "elibrary",     "eLibrary",     "https://www.elibrary.ru/author_items.asp?authorid=121878" },
};

/// @brief Сведения о программе пакета для окна: идентификатор, название, описание по-русски и по-английски.
struct ProgramInfo
{
    /// Имя программы, как в `theme::program()`.
    const char *id;
    /// Отображаемое название.
    const char *name;
    /// Описание по-русски (UTF-8).
    const char *descriptionRu;
    /// Описание по-английски.
    const char *descriptionEn;
};

// Названия и описания программ — из авторской документации: мануал «TAMIC-Rt H,
// версия 2.0p» (2000), разделы 1–2, и докторская диссертация К. Н. Климова (2007),
// гл. 9, п. 9.1 и 9.3 (Rt-X и диаграммы направленности в мануале 2000 г. ещё нет).
// В «H‑плоскостных» дефис неразрывный (U+2011), чтобы строка не переносилась посередине.
/// Таблица программ пакета.
const ProgramInfo kPrograms[] = {
    { "planarrt_h", "PlanarRT_H",
      "Счётное ядро для электродинамического анализа H‑плоскостных планарных волноводных СВЧ устройств на основе метода Rt сеток.",
      "Computational kernel for electrodynamic analysis of H‑plane planar waveguide microwave devices by the Rt-grid method." },
    { "planarrt_x", "PlanarRT_X",
      "Счётное ядро, реализующее 4‑параметрический алгоритм анализа рассеяния электромагнитных волн Е поляризации от неоднородной подмагниченной плазмы.",
      "Computational kernel implementing a four-parameter algorithm for scattering analysis of E‑polarized electromagnetic waves by inhomogeneous magnetized plasma." },
    { "tmcgrout",   "TMCGROUT",
      "Программа просмотра файлов S‑матриц устройства.",
      "Viewer for device S‑matrix files." },
    { "tmcros",     "TMCROS",
      "Программа для просмотра сигналов, амплитуд и фаз падающих и отражённых волн на входах устройства.",
      "Viewer for signals, amplitudes and phases of incident and reflected waves at the device ports." },
    { "tmc_dn",     "TMC_DN",
      "Программа просмотра диаграмм направленности.",
      "Viewer for directional patterns." },
    { "tpldesigner", "TPL Designer",
      "Редактор файлов задания .tpl для счётных ядер PlanarRT_H и PlanarRT_X: геометрия рисуется мышью, синтаксис задания пишет программа.",
      "Editor of .tpl task files for the PlanarRT_H and PlanarRT_X kernels: the geometry is drawn with the mouse, the task syntax is written by the program." },
    { "fieldview",  "FieldView",
      "Программа для просмотра геометрии устройства и распределения интенсивности поля; позволяет синхронизировать расчёт и просмотр полей.",
      "Viewer for device geometry and field intensity distribution; can synchronize field display with the calculation." },
};

// --- Палитра проекта ---------------------------------------------------------------
/// Цвета палитры проекта для рисования окна.
const QColor kBg{0x0a, 0x0e, 0x16};
/// Цвет палитры проекта.
const QColor kBg2{0x0e, 0x13, 0x20};
/// Цвет палитры проекта.
const QColor kSurface{0x13, 0x1a, 0x28};
/// Цвет палитры проекта.
const QColor kSurface2{0x1a, 0x23, 0x35};
/// Цвет палитры проекта.
const QColor kLine{0x24, 0x30, 0x49};
/// Цвет палитры проекта.
const QColor kText{0xe7, 0xee, 0xf7};
/// Цвет палитры проекта.
const QColor kTextDim{0x9f, 0xb0, 0xc8};
/// Цвет палитры проекта.
const QColor kTextMute{0x6c, 0x7d, 0x97};
/// Цвет палитры проекта.
const QColor kAccent{0x34, 0xe3, 0xc4};
/// Цвет палитры проекта.
const QColor kAccent2{0xb0, 0x7c, 0xff};

/// @brief Возвращает имя системы сборки: Windows, Linux или macOS.
/// @return `macOS`, `Linux` или `Windows` (по макросам компилятора).
QString systemName()
{
#if defined(__APPLE__)
    return QStringLiteral("macOS");
#elif defined(__linux__)
    return QStringLiteral("Linux");
#else
    return QStringLiteral("Windows");
#endif
}

/// @brief Возвращает архитектуру сборки: x64, ARM64 или x86.
/// @return `ARM64`, `x64` или `x86` (по макросам компилятора).
QString architectureName()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return QStringLiteral("ARM64");
#elif defined(__x86_64__) || defined(_M_X64)
    return QStringLiteral("x64");
#else
    return QStringLiteral("x86");
#endif
}

// Где искать руководство: рядом с программой и на уровень-два выше — так же,
// как TmcDocLink.h поднимается по дереву от .exe. Для пакета .deb это
// /usr/share/tmc-suite, для macOS — Resources внутри .app.
/// @brief Ищет PDF руководства рядом с программой и в типовых каталогах поставки.
/// @return Абсолютный путь или пустая строка.
QString findManual()
{
    const QString exeDir = QCoreApplication::applicationDirPath();
    QStringList places;
    places << exeDir
           << exeDir + QStringLiteral("/../share/tmc-suite")
           << exeDir + QStringLiteral("/../share/doc/tmc-suite")
           << exeDir + QStringLiteral("/../Resources")
           << exeDir + QStringLiteral("/../../Resources")
           << exeDir + QStringLiteral("/../doc")
           << exeDir + QStringLiteral("/..");

    for (const QString &dir : places) {
        // У TPL Designer своё руководство (кладётся рядом с программами пакета).
        const bool designer = theme::program() == QLatin1String("tpldesigner");
        const char *file = designer ? kDesignerManualFile : kManualFile;
        const QFileInfo fi(QDir(dir).filePath(QString::fromLatin1(file)));
        if (fi.isFile())
            return fi.absoluteFilePath();
        // В выпуске руководство пакета называется по версии и языку:
        // «TMC_Suite_5.0_Руководство_пользователя.pdf» и «TMC_Suite_5.0_User_Manual.pdf».
        // Берём самое новое по имени на языке интерфейса, иначе — любое.
        if (!designer) {
            const QStringList found = QDir(dir).entryList({QStringLiteral("TMC_Suite_*.pdf")},
                                                          QDir::Files, QDir::Name | QDir::Reversed);
            const bool english = !tmci18n::russian();
            for (const QString &f : found)
                if (f.contains(QLatin1String("User_Manual"), Qt::CaseInsensitive) == english)
                    return QDir(dir).absoluteFilePath(f);
            if (!found.isEmpty())
                return QDir(dir).absoluteFilePath(found.first());
        }
    }
    return QString();
}

/// @brief Линейно смешивает два цвета (включая прозрачность).
/// @param[in] a  цвет при `t = 0`
/// @param[in] b  цвет при `t = 1`
/// @param[in] t  доля, ограничивается `[0, 1]`
/// @return Смешанный цвет.
QColor mix(const QColor &a, const QColor &b, qreal t)
{
    t = qBound<qreal>(0.0, t, 1.0);
    return QColor::fromRgbF(float(a.redF() + (b.redF() - a.redF()) * t),
                            float(a.greenF() + (b.greenF() - a.greenF()) * t),
                            float(a.blueF() + (b.blueF() - a.blueF()) * t),
                            float(a.alphaF() + (b.alphaF() - a.alphaF()) * t));
}

/// @brief Возвращает шрифт интерфейса заданного размера и насыщенности.
/// @param[in] pt      размер, пункты
/// @param[in] weight  насыщенность `QFont::Weight`
/// @return Шрифт.
QFont uiFont(qreal pt, int weight = QFont::Normal)
{
    QFont f(theme::uiFontFamily());
    f.setPointSizeF(pt);
    f.setWeight(QFont::Weight(weight));
    return f;
}

// Одноцветный значок из SVG, перекрашенный в нужный цвет. Пустой, если файла нет.
/// @param[in] path   путь к SVG в ресурсах
/// @param[in] size   сторона, логические пиксели
/// @param[in] dpr    отношение физических пикселей к логическим
/// @param[in] color  цвет заливки
/// @return Картинка; пустая — файла нет или он не читается.
QPixmap tintedSvg(const QString &path, int size, qreal dpr, const QColor &color)
{
    if (!QFile::exists(path))
        return QPixmap();
    QSvgRenderer renderer(path);
    if (!renderer.isValid())
        return QPixmap();
    QPixmap pm(QSize(size, size) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    renderer.render(&p, QRectF(0, 0, size, size));
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(QRectF(0, 0, size, size), color);
    return pm;
}

/// @brief Возвращает значок программы: SVG из `:/tmcicons/app/`, иначе значок окна-родителя.
/// @param[in] size          сторона, логические пиксели
/// @param[in] dpr           отношение физических пикселей к логическим
/// @param[in] fallbackFrom  виджет, от которого искать значок окна вверх по родителям
/// @return Картинка; пустая — значка нет.
QPixmap programPixmap(int size, qreal dpr, const QWidget *fallbackFrom)
{
    const QString path = QStringLiteral(":/tmcicons/app/") + theme::program() + QStringLiteral(".svg");
    if (!theme::program().isEmpty() && QFile::exists(path)) {
        QSvgRenderer renderer(path);
        if (renderer.isValid()) {
            QPixmap pm(QSize(size, size) * dpr);
            pm.setDevicePixelRatio(dpr);
            pm.fill(Qt::transparent);
            QPainter p(&pm);
            p.setRenderHint(QPainter::Antialiasing);
            p.setRenderHint(QPainter::SmoothPixmapTransform);
            renderer.render(&p, QRectF(0, 0, size, size));
            return pm;
        }
    }
    // Значка нового набора нет — значок окна программы.
    for (const QWidget *w = fallbackFrom; w; w = w->parentWidget()) {
        if (!w->windowIcon().isNull())
            return w->windowIcon().pixmap(QSize(size, size), dpr);
    }
    return QPixmap();
}

// --- Что показать ----------------------------------------------------------------

/// @brief Что показать в окне: название, описание, дополнительные строки.
struct AboutInfo
{
    /// Название программы.
    QString name;          // название программы
    /// Строка описания.
    QString description;   // строка описания
    /// Дополнительные строки от программы.
    QStringList extra;     // дополнительные строки от программы («Model : H»)
};

/// @brief Приводит строку к виду для сравнения: без пробелов по краям, нижний регистр, без точек в конце.
/// @param[in] s  строка
/// @return Нормализованная строка.
QString normalized(QString s)
{
    s = s.trimmed().toLower();
    while (s.endsWith(QLatin1Char('.')))
        s.chop(1);
    return s;
}

/// @brief Собирает сведения для окна по `theme::program()` или по переданным строкам.
///
/// Строки описания делятся по `<br>`/переводу строки; строки `Copyright…` и `©`
/// отбрасываются. Для известной программы описание берётся из `kPrograms` на
/// языке интерфейса, остальные строки — дополнительные.
/// @param[in] windowTitle  прежний заголовок окна
/// @param[in] title        название программы
/// @param[in] description  описание
/// @return Сведения для окна.
AboutInfo resolveInfo(const QString &windowTitle, const QString &title, const QString &description)
{
    // Строки описания от программы; «Copyright (C) 2004 Tamic_soft group» и т. п.
    // не показываем — авторство в окне указано отдельно.
    QStringList lines;
    const QStringList raw = description.split(
        QRegularExpression(QStringLiteral("<br\\s*/?>|\\n"), QRegularExpression::CaseInsensitiveOption),
        Qt::SkipEmptyParts);
    for (QString l : raw) {
        l = l.trimmed();
        if (l.isEmpty() || l.startsWith(QLatin1String("copyright"), Qt::CaseInsensitive)
            || l.startsWith(QChar(0x00A9)))
            continue;
        lines << l;
    }

    AboutInfo info;
    const QString prog = theme::program().toLower();
    for (const ProgramInfo &p : kPrograms) {
        if (prog == QLatin1String(p.id)) {
            info.name = QString::fromLatin1(p.name);
            info.description = tmci18n::tr(p.descriptionRu, p.descriptionEn);
            for (const QString &l : lines)
                if (normalized(l) != normalized(info.description))
                    info.extra << l;
            return info;
        }
    }

    // Программа не задана — берём то, что передали.
    QString name = title;
    name.remove(QRegularExpression(QStringLiteral("\\s*Version\\s*[0-9.]+\\s*$"),
                                   QRegularExpression::CaseInsensitiveOption));
    if (name.trimmed().isEmpty()) {
        name = windowTitle;
        name.remove(QRegularExpression(QStringLiteral("^About\\s+"), QRegularExpression::CaseInsensitiveOption));
    }
    info.name = name.trimmed().isEmpty() ? QStringLiteral("TMC Suite") : name.trimmed();
    info.description = lines.join(QLatin1Char(' '));
    return info;
}

/// @brief Открывает адрес в системном браузере или программе просмотра.
/// @param[in] url  адрес (`https://…` или `file:///…`)
void openUrl(const QString &url)
{
    QDesktopServices::openUrl(QUrl(url));
}

// --- Элементы окна ---------------------------------------------------------------

// Плавная подсветка при наведении: значение 0..1 для всех рисованных кнопок окна.
/// @brief Рисованная кнопка с плавной подсветкой (0…1) при наведении мыши и фокусе с клавиатуры.
class HoverButton : public QAbstractButton
{
public:
    /// @brief Создаёт кнопку с курсором-рукой и анимацией подсветки 180 мс.
    /// @param[in] parent  родитель
    explicit HoverButton(QWidget *parent) : QAbstractButton(parent)
    {
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::TabFocus);
        m_anim.setDuration(180);
        m_anim.setEasingCurve(QEasingCurve::OutCubic);
        QObject::connect(&m_anim, &QVariantAnimation::valueChanged, this,
                         [this](const QVariant &v) { m_hover = v.toReal(); update(); });
    }

protected:
    /// @brief Запускает анимацию подсветки к значению.
    /// @param[in] target  0 — без подсветки, 1 — полная
    void animateTo(qreal target)
    {
        m_anim.stop();
        m_anim.setStartValue(m_hover);
        m_anim.setEndValue(target);
        m_anim.start();
    }
    /// @brief Мышь вошла — подсветить.
    /// @param[in] e  событие входа указателя
    void enterEvent(QEnterEvent *e) override { QAbstractButton::enterEvent(e); animateTo(1.0); }
    /// @brief Мышь ушла — погасить.
    /// @param[in] e  событие ухода указателя
    void leaveEvent(QEvent *e) override { QAbstractButton::leaveEvent(e); animateTo(0.0); }
    // Подсветка от клавиатуры (Tab) — та же, что от мыши; фокус, который окно
    // ставит само при открытии, кнопку не подсвечивает.
    /// @brief Фокус от Tab/Shift+Tab — подсветить.
    /// @param[in] e  событие фокуса
    void focusInEvent(QFocusEvent *e) override
    {
        QAbstractButton::focusInEvent(e);
        if (e->reason() == Qt::TabFocusReason || e->reason() == Qt::BacktabFocusReason)
            animateTo(1.0);
    }
    /// @brief Фокус ушёл — погасить, если мышь не над кнопкой.
    /// @param[in] e  событие фокуса
    void focusOutEvent(QFocusEvent *e) override
    {
        QAbstractButton::focusOutEvent(e);
        if (!underMouse())
            animateTo(0.0);
    }

    /// Текущая подсветка, 0…1.
    qreal m_hover = 0;

private:
    /// Анимация подсветки.
    QVariantAnimation m_anim;
};

// Круглый значок-ссылка с подписью: GitHub, LinkedIn, ResearchGate, eLibrary.
/// @brief Круглая кнопка-ссылка на профиль автора с подписью.
class ProfileButton : public HoverButton
{
public:
    /// @brief Создаёт кнопку 76x72 и подключает открытие адреса по щелчку.
    /// @param[in] link    профиль
    /// @param[in] parent  родитель
    ProfileButton(const ProfileLink &link, QWidget *parent)
        : HoverButton(parent), m_name(QString::fromLatin1(link.name)), m_url(QString::fromLatin1(link.url))
    {
        setFixedSize(76, 72);
        setToolTip(m_url);
        setAccessibleName(m_name);
        const QString path = QStringLiteral(":/tmcicons/links/") + QLatin1String(link.icon) + QStringLiteral(".svg");
        const qreal dpr = devicePixelRatioF();
        m_glyph = tintedSvg(path, 20, dpr, kText);
        m_glyphHot = tintedSvg(path, 20, dpr, kAccent);
        connect(this, &QAbstractButton::clicked, this, [this]() { openUrl(m_url); });
    }

protected:
    /// @brief Рисует круг, значок (или первую букву) и подпись с учётом подсветки.
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const qreal d = 44;
        const QRectF circle((width() - d) / 2.0, 2, d, d);

        // Подсветка: мягкое свечение акцента под кругом.
        if (m_hover > 0.01) {
            QRadialGradient glow(circle.center(), d * 0.85);
            glow.setColorAt(0, QColor(52, 227, 196, int(46 * m_hover)));
            glow.setColorAt(1, QColor(52, 227, 196, 0));
            p.setPen(Qt::NoPen);
            p.setBrush(glow);
            p.drawEllipse(circle.center(), d * 0.85, d * 0.85);
        }
        p.setBrush(mix(kBg2, kSurface2, m_hover));
        p.setPen(QPen(mix(kLine, kAccent, m_hover), 1));
        p.drawEllipse(circle.adjusted(0.5, 0.5, -0.5, -0.5));

        const QRectF glyph(circle.center().x() - 10, circle.center().y() - 10, 20, 20);
        if (!m_glyph.isNull()) {
            p.setOpacity(1.0 - m_hover);
            p.drawPixmap(glyph.topLeft(), m_glyph);
            p.setOpacity(m_hover);
            p.drawPixmap(glyph.topLeft(), m_glyphHot);
            p.setOpacity(1.0);
        } else {
            p.setFont(uiFont(13, QFont::Bold));
            p.setPen(mix(kText, kAccent, m_hover));
            p.drawText(circle, Qt::AlignCenter, m_name.left(1));
        }

        p.setFont(uiFont(8));
        p.setPen(mix(kTextDim, kText, m_hover));
        p.drawText(QRectF(0, circle.bottom() + 5, width(), 18), Qt::AlignHCenter | Qt::AlignTop, m_name);
    }

private:
    /// Подпись.
    QString m_name;
    /// Адрес.
    QString m_url;
    /// @name Значки кнопки
    ///@{
    /// Значок обычный и подсвеченный.
    QPixmap m_glyph, m_glyphHot;
    //@}
};

// Широкая кнопка-ссылка: сайт и руководство пользователя.
/// @brief Широкая кнопка-ссылка (сайт, руководство) со значком, текстом и необязательной меткой.
class PillLink : public HoverButton
{
public:
    /// @brief Создаёт кнопку высотой 42 пикселя.
    /// @param[in] icon    имя значка в `:/tmcicons/links/`
    /// @param[in] text    текст
    /// @param[in] tag     метка справа (`PDF`, `НА САЙТЕ`) или пустая
    /// @param[in] url     адрес
    /// @param[in] parent  родитель
    PillLink(const QString &icon, const QString &text, const QString &tag, const QString &url,
             QWidget *parent)
        : HoverButton(parent), m_text(text), m_tag(tag), m_url(url)
    {
        const QString path = QStringLiteral(":/tmcicons/links/") + icon + QStringLiteral(".svg");
        const qreal dpr = devicePixelRatioF();
        m_glyph = tintedSvg(path, 18, dpr, kTextDim);
        m_glyphHot = tintedSvg(path, 18, dpr, kAccent);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setFixedHeight(42);
        setAccessibleName(text);
        connect(this, &QAbstractButton::clicked, this, [this]() { openUrl(m_url); });
    }

    /// @brief Возвращает размер по тексту и метке.
    /// @return Ширина по тексту, значку, отступам и метке, пиксели; высота 42.
    QSize sizeHint() const override
    {
        const QFontMetrics fm(uiFont(9.5, QFont::DemiBold));
        const QFontMetrics fmTag(uiFont(7.5, QFont::Bold));
        int w = 14 + 18 + 10 + fm.horizontalAdvance(m_text) + 14;
        if (!m_tag.isEmpty())
            w += 8 + fmTag.horizontalAdvance(m_tag) + 12;
        return QSize(w, 42);
    }

protected:
    /// @brief Рисует скруглённую плашку, значок, текст и метку.
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        p.setBrush(mix(kSurface, kSurface2, m_hover));
        p.setPen(QPen(mix(kLine, kAccent, m_hover), 1));
        p.drawRoundedRect(r, 10, 10);

        qreal x = 14;
        const qreal cy = height() / 2.0;
        if (!m_glyph.isNull()) {
            p.setOpacity(1.0 - m_hover);
            p.drawPixmap(QPointF(x, cy - 9), m_glyph);
            p.setOpacity(m_hover);
            p.drawPixmap(QPointF(x, cy - 9), m_glyphHot);
            p.setOpacity(1.0);
        }
        x += 18 + 10;

        const QFont f = uiFont(9.5, QFont::DemiBold);
        p.setFont(f);
        p.setPen(mix(kText, kAccent, m_hover));
        const int tw = QFontMetrics(f).horizontalAdvance(m_text);
        p.drawText(QRectF(x, 0, tw + 2, height()), Qt::AlignLeft | Qt::AlignVCenter, m_text);
        x += tw + 8;

        if (!m_tag.isEmpty()) {
            const QFont ft = uiFont(7.5, QFont::Bold);
            const int gw = QFontMetrics(ft).horizontalAdvance(m_tag) + 12;
            const QRectF tag(x, cy - 9, gw, 18);
            p.setPen(QPen(QColor(176, 124, 255, 120), 1));
            p.setBrush(QColor(176, 124, 255, 26));
            p.drawRoundedRect(tag, 9, 9);
            p.setFont(ft);
            p.setPen(kAccent2);
            p.drawText(tag, Qt::AlignCenter, m_tag);
        }
    }

private:
    /// @name Надписи и адрес кнопки
    ///@{
    /// Текст, метка, адрес.
    QString m_text, m_tag, m_url;
    //@}
    /// @name Значки кнопки
    ///@{
    /// Значок обычный и подсвеченный.
    QPixmap m_glyph, m_glyphHot;
    //@}
};

// Таблица стилей содержимого окна. Стоит на самой панели и потому важнее
// общей таблицы приложения (там у диалогов мелкий шрифт ресурсов Windows).
/// @brief Возвращает таблицу стилей панели окна с подставленными гарнитурами.
/// @return Текст таблицы стилей Qt (QSS) с гарнитурами `theme::uiFontFamily()` и `theme::monoFontFamily()`.
QString panelStyleSheet()
{
    QString css = QStringLiteral(R"(
#tmcAboutPanel QWidget { font-family: '@UI@'; font-size: 9.5pt; color: #e7eef7; background: transparent; }
#tmcAboutPanel QLabel { padding: 0; }
#tmcAboutPanel QLabel[role="name"] { font-size: 21pt; font-weight: 800; }
#tmcAboutPanel QLabel[role="desc"] { color: #9fb0c8; font-size: 9.5pt; }
#tmcAboutPanel QLabel[role="extra"] { color: #9fb0c8; font-family: '@MONO@'; font-size: 8.5pt; }
#tmcAboutPanel QLabel[role="caption"] { color: #6c7d97; font-size: 7.5pt; font-weight: 700; }
#tmcAboutPanel QLabel[role="author"] { font-size: 13pt; font-weight: 700; }
#tmcAboutPanel QLabel[role="foot"] { color: #6c7d97; font-family: '@MONO@'; font-size: 8pt; }
#tmcAboutPanel QLabel[role="chip"] {
    color: #9fb0c8; border: 1px solid #243049; border-radius: 10px;
    padding: 2px 9px; font-size: 8.5pt; background: #0e1320;
}
#tmcAboutPanel QLabel[role="chip"][accent="true"] {
    color: #34e3c4; border: 1px solid rgba(52,227,196,0.45); background: rgba(52,227,196,0.08);
    font-weight: 700;
}
#tmcAboutPanel QFrame[role="sep"] { background: #243049; border: none; min-height: 1px; max-height: 1px; }
#tmcAboutPanel QPushButton {
    background: #34e3c4; color: #06231e; border: 1px solid #34e3c4; border-radius: 10px;
    padding: 8px 22px; min-width: 96px; font-weight: 700;
}
#tmcAboutPanel QPushButton:hover { background: #5cebd1; border: 1px solid #5cebd1; }
#tmcAboutPanel QPushButton:pressed { background: #2bc9ad; }
)");
    css.replace(QStringLiteral("@UI@"), theme::uiFontFamily());
    css.replace(QStringLiteral("@MONO@"), theme::monoFontFamily());
    return css;
}

/// @brief Создаёт подпись с ролью оформления.
/// @param[in] text    текст
/// @param[in] role    значение свойства `role` для таблицы стилей
/// @param[in] parent  родитель
/// @return Новая подпись (принадлежит родителю).
QLabel *makeLabel(const QString &text, const char *role, QWidget *parent)
{
    QLabel *l = new QLabel(text, parent);
    l->setProperty("role", QString::fromLatin1(role));
    return l;
}

/// @brief Создаёт «плашку» (версия, год, система).
/// @param[in] text    текст
/// @param[in] accent  true — акцентная
/// @param[in] parent  родитель
/// @return Новая подпись.
QLabel *makeChip(const QString &text, bool accent, QWidget *parent)
{
    QLabel *l = makeLabel(text, "chip", parent);
    l->setProperty("accent", accent);
    return l;
}

// Содержимое окна. Фон рисуется здесь же: тёмный фон проекта с мягким
// свечением акцентов за значком программы — как на сайте.
/// @brief Содержимое окна «О программе»: шапка, автор и профили, ссылки на сайт и руководство, кнопка OK.
class AboutPanel : public QWidget
{
public:
    /// Вызывается по кнопке OK (закрыть окно).
    std::function<void()> onClose;

    /// @brief Строит содержимое окна.
    /// @param[in] info    что показать
    /// @param[in] parent  окно (от него берётся запасной значок)
    AboutPanel(const AboutInfo &info, QWidget *parent) : QWidget(parent)
    {
        setObjectName(QStringLiteral("tmcAboutPanel"));
        setStyleSheet(panelStyleSheet());

        // Кнопка OK создаётся первой: она первая в цепочке фокуса, и при открытии
        // окна фокус получает она (Enter и пробел сразу закрывают окно), а не
        // первая кнопка-ссылка.
        m_ok = new QPushButton(QStringLiteral("OK"), this);

        QVBoxLayout *root = new QVBoxLayout(this);
        root->setContentsMargins(28, 26, 28, 20);
        root->setSpacing(0);

        // Шапка: значок, название, описание, версия.
        QHBoxLayout *head = new QHBoxLayout;
        head->setSpacing(20);
        m_icon = new QLabel(this);
        m_icon->setFixedSize(92, 92);
        m_icon->setAlignment(Qt::AlignCenter);
        const QPixmap pm = programPixmap(88, devicePixelRatioF(), parent);
        if (!pm.isNull())
            m_icon->setPixmap(pm);
        head->addWidget(m_icon, 0, Qt::AlignTop);

        QVBoxLayout *titles = new QVBoxLayout;
        titles->setSpacing(4);
        titles->addWidget(makeLabel(info.name, "name", this));
        if (!info.description.isEmpty()) {
            QLabel *d = makeLabel(info.description, "desc", this);
            d->setWordWrap(true);
            titles->addWidget(d);
        }
        for (const QString &e : info.extra) {
            QLabel *x = makeLabel(e, "extra", this);
            x->setTextFormat(Qt::PlainText);
            titles->addWidget(x);
        }
        titles->addSpacing(8);
        QHBoxLayout *chips = new QHBoxLayout;
        chips->setSpacing(6);
        chips->addWidget(makeChip(QStringLiteral("Версия %1").arg(QLatin1String(kPackageVersion)), true, this));
        chips->addWidget(makeChip(QString::fromLatin1(kYear), false, this));
        QLabel *sys = makeChip(QStringLiteral("%1 · %2").arg(systemName(), architectureName()), false, this);
        sys->setToolTip(QStringLiteral("Qt %1").arg(QLatin1String(qVersion())));
        chips->addWidget(sys);
        chips->addStretch(1);
        titles->addLayout(chips);
        head->addLayout(titles, 1);
        root->addLayout(head);

        root->addSpacing(20);
        QFrame *sep = new QFrame(this);
        sep->setProperty("role", QStringLiteral("sep"));
        root->addWidget(sep);
        root->addSpacing(16);

        // Автор и его страницы.
        QHBoxLayout *authorRow = new QHBoxLayout;
        authorRow->setSpacing(4);
        QVBoxLayout *author = new QVBoxLayout;
        author->setSpacing(3);
        author->addStretch(1);
        author->addWidget(makeLabel(QStringLiteral("АВТОР"), "caption", this));
        author->addWidget(makeLabel(QString::fromUtf8(kAuthor), "author", this));
        author->addWidget(makeLabel(QStringLiteral("© %1").arg(QLatin1String(kYear)), "desc", this));
        author->addStretch(1);
        authorRow->addLayout(author, 1);
        for (const ProfileLink &l : kProfiles)
            authorRow->addWidget(new ProfileButton(l, this), 0, Qt::AlignVCenter);
        root->addLayout(authorRow);

        root->addSpacing(16);

        // Сайт и руководство пользователя.
        const QString manual = findManual();
        QHBoxLayout *docs = new QHBoxLayout;
        docs->setSpacing(10);
        PillLink *site = new PillLink(QStringLiteral("website"), QString::fromLatin1(kSiteText),
                                      QString(), QString::fromLatin1(kSiteUrl), this);
        site->setToolTip(QString::fromLatin1(kSiteUrl));
        docs->addWidget(site, 2);
        PillLink *doc = new PillLink(QStringLiteral("manual"), tmci18n::tr("Руководство пользователя", "User manual"),
                                     manual.isEmpty() ? tmci18n::tr("НА САЙТЕ", "ONLINE") : QStringLiteral("PDF"),
                                     manual.isEmpty() ? QString::fromLatin1(kOnlineDoc)
                                                      : QUrl::fromLocalFile(manual).toString(),
                                     this);
        doc->setToolTip(manual.isEmpty()
                            ? QStringLiteral("Файл %1 рядом с программой не найден — "
                                             "откроется руководство на сайте")
                                  .arg(QLatin1String(kManualFile))
                            : manual);
        docs->addWidget(doc, 3);
        root->addLayout(docs);

        root->addSpacing(20);

        // Низ: пакет и кнопка закрытия.
        QHBoxLayout *foot = new QHBoxLayout;
        foot->addWidget(makeLabel(QStringLiteral("TMC Suite %1 · Qt %2")
                                      .arg(QLatin1String(kPackageVersion), QLatin1String(qVersion())),
                                  "foot", this), 1, Qt::AlignVCenter);
        QPushButton *ok = m_ok;
        ok->setCursor(Qt::PointingHandCursor);
        ok->setDefault(true);
        ok->setAutoDefault(true);
        connect(ok, &QPushButton::clicked, this, [this]() {
            if (onClose)
                onClose();
        });
        foot->addWidget(ok);
        root->addLayout(foot);

        setMinimumWidth(600);
    }

    /// @brief Возвращает кнопку OK.
    /// @return Кнопка OK (принадлежит панели; не NULL).
    QPushButton *okButton() const { return m_ok; }

protected:
    /// @brief Рисует фон: тёмная подложка, свечения и линия акцента по верхнему краю.
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), kBg);

        // Свечение за значком (циан) и в правом верхнем углу (фиолетовый).
        const QPointF ic = m_icon ? QPointF(m_icon->geometry().center()) : QPointF(70, 70);
        QRadialGradient g1(ic, 190);
        g1.setColorAt(0, QColor(52, 227, 196, 40));
        g1.setColorAt(1, QColor(52, 227, 196, 0));
        p.fillRect(rect(), g1);
        QRadialGradient g2(QPointF(width() - 40, -30), 260);
        g2.setColorAt(0, QColor(176, 124, 255, 30));
        g2.setColorAt(1, QColor(176, 124, 255, 0));
        p.fillRect(rect(), g2);

        // Тонкая линия акцента по верхнему краю — «волна» сайта.
        QLinearGradient top(QPointF(0, 0), QPointF(width(), 0));
        top.setColorAt(0, QColor(52, 227, 196, 0));
        top.setColorAt(0.3, QColor(52, 227, 196, 160));
        top.setColorAt(0.7, QColor(176, 124, 255, 140));
        top.setColorAt(1, QColor(176, 124, 255, 0));
        p.fillRect(QRectF(0, 0, width(), 2), top);
    }

private:
    /// Подпись со значком программы.
    QLabel *m_icon = nullptr;
    /// Кнопка OK.
    QPushButton *m_ok = nullptr;
};

// Слой совместимости не знает о типах Qt, поэтому принимает указатель без типа.
/// @brief Переходник оформителя для слоя MFC: вызывает `tmcabout::decorate`.
/// @param[in] qDialog  окно `QWidget*`
void decorateRaw(void *qDialog)
{
    tmcabout::decorate(static_cast<QWidget *>(qDialog));
}

} // namespace

namespace tmcabout {

QString manualPath()
{
    return findManual();
}

QString aboutText(const QString &title, const QString &description)
{
    const AboutInfo info = resolveInfo(QString(), title, description);
    const QString manual = findManual();
    const QString docUrl = manual.isEmpty() ? QString::fromLatin1(kOnlineDoc)
                                            : QUrl::fromLocalFile(manual).toString();
    QStringList profiles;
    for (const ProfileLink &l : kProfiles)
        profiles << QStringLiteral("<a href=\"%1\" style=\"color:#34e3c4;\">%2</a>")
                        .arg(QLatin1String(l.url), QLatin1String(l.name));

    return QStringLiteral(
               "<b>%1</b><br>%2<br><br>"
               "Версия %3 · %4 · %5 (%6)<br>"
               "Автор: %7<br><br>"
               "%8<br><br>"
               "<a href=\"%9\" style=\"color:#34e3c4;\">Руководство пользователя</a>"
               "&nbsp;&nbsp;·&nbsp;&nbsp;"
               "<a href=\"%10\" style=\"color:#34e3c4;\">%11</a>")
        .arg(info.name.toHtmlEscaped(), info.description.toHtmlEscaped(),
             QLatin1String(kPackageVersion), QLatin1String(kYear), systemName(),
             architectureName(), QString::fromUtf8(kAuthor),
             profiles.join(QStringLiteral("&nbsp;&nbsp;·&nbsp;&nbsp;")), docUrl)
        .arg(QLatin1String(kSiteUrl), QLatin1String(kSiteText));
}

void show(QWidget *parent, const QString &windowTitle, const QString &title,
          const QString &description)
{
    const AboutInfo info = resolveInfo(windowTitle, title, description);

    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("tmcAboutDialog"));
    dlg.setWindowTitle(QStringLiteral("About %1").arg(info.name));
    QVBoxLayout *lay = new QVBoxLayout(&dlg);
    lay->setContentsMargins(0, 0, 0, 0);
    AboutPanel *panel = new AboutPanel(info, &dlg);
    panel->onClose = [&dlg]() { dlg.accept(); };
    lay->addWidget(panel);
    lay->setSizeConstraint(QLayout::SetFixedSize);
    dlg.exec();
}

void decorate(QWidget *dialog)
{
    if (!dialog || dialog->objectName() != QLatin1String("IDD_ABOUTBOX"))
        return;
    if (dialog->property("tmcAboutDone").toBool())
        return;
    dialog->setProperty("tmcAboutDone", true);

    // Разметка из ресурсов Windows остаётся (код окна может к ней обращаться),
    // но прячется: поверх ставится единое содержимое.
    const QList<QWidget *> old = dialog->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget *w : old)
        w->hide();
    delete dialog->layout();

    const AboutInfo info = resolveInfo(dialog->windowTitle(), QString(), QString());
    AboutPanel *panel = new AboutPanel(info, dialog);
    panel->onClose = [dialog]() {
        if (QDialog *d = qobject_cast<QDialog *>(dialog))
            d->accept();
        else
            dialog->close();
    };
    QVBoxLayout *lay = new QVBoxLayout(dialog);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(panel);
    dialog->setWindowTitle(QStringLiteral("About %1").arg(info.name));
    dialog->setMinimumSize(0, 0);
    dialog->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    lay->setSizeConstraint(QLayout::SetFixedSize);
}

void install()
{
    tmc_set_dialog_decorator(&decorateRaw);
}

} // namespace tmcabout
