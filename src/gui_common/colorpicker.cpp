// colorpicker.cpp — окно выбора цвета в оформлении TMC Suite.
//
// Все элементы, кроме полей ввода и кнопок, рисуются здесь же: область
// «насыщенность/яркость», движки с градиентной дорожкой, образцы «было/стало»
// и плитки палитры. Цвета — палитра проекта (docs/style-guide.md мобильной
// версии): bg #0a0e16, surface #131a28, line #243049, accent #34e3c4 и т. д.
//
// Цвет хранится в двух видах: RGB (m_color) — то, что получит программа, и HSV
// (m_h, m_s, m_v) — то, что показывают область и полоса оттенка. HSV держим
// отдельно, чтобы на сером или чёрном цвете не терялся выбранный оттенок:
// иначе ручка оттенка прыгала бы в ноль при каждом проходе через серый.

/// @file colorpicker.cpp
/// @brief Реализация окна выбора цвета: рисованные область SV, движки, образцы, пипетка, недавние цвета.
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_gui_common`.
/// @see https://www.tamic.ru/docs/code/gui-common.html#colorpicker
#include "colorpicker.h"
#include "i18n/tmci18n.h"

#include "theme.h"

#include <QAbstractButton>
#include <QApplication>
#include <QCursor>
#include <QEasingCurve>
#include <QFrame>
#include <QGridLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScreen>
#include <QSettings>
#include <QSpinBox>
#include <QSvgRenderer>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariantAnimation>

#include <algorithm>
#include <cmath>
#include <functional>

namespace {

// --- Палитра проекта -----------------------------------------------------------
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

/// Наибольшее число недавних цветов.
const int kRecentMax = 12;

/// @brief Цвет образца с подписью.
struct NamedColor
{
    /// Цвет 0xRRGGBB.
    QRgb rgb;
    /// Подпись (UTF-8).
    const char *name;
};

// Палитра проекта: акценты, статусы и цвета текста.
/// Ряд «Палитра проекта».
const NamedColor kProjectPalette[] = {
    { 0x34e3c4, "accent" },   { 0xb07cff, "accent-2" }, { 0xffcf5c, "accent-3" },
    { 0x5ce08a, "ok" },       { 0xff6b6b, "danger" },   { 0xe3b341, "warn" },
    { 0xe7eef7, "text" },     { 0x9fb0c8, "text-dim" },
};

// Классические цвета графиков Windows-версии (значения RGB(...) из кода
// вьюверов TMCGROUT/TMCROS/TMC_DN).
/// Ряд «Цвета графиков».
const NamedColor kClassicPalette[] = {
    { 0x000000, "чёрный" },     { 0xffffff, "белый" },      { 0xff0000, "красный" },
    { 0x00c000, "зелёный" },    { 0x0000ff, "синий" },      { 0x00ffff, "голубой" },
    { 0xff00ff, "пурпурный" },  { 0xffff00, "жёлтый" },     { 0x801400, "коричневый" },
    { 0x808080, "серый" },      { 0x008080, "сине-зелёный" }, { 0x800080, "фиолетовый" },
};

// Пипетка: контур в стиле значков проекта.
/// Рисунок пипетки (SVG).
const char kPipetteSvg[] =
    "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'>"
    "<g fill='none' stroke='#e7eef7' stroke-width='1.8' stroke-linecap='round' stroke-linejoin='round'>"
    "<path d='M16.6 3.4a2.5 2.5 0 0 1 3.5 3.5l-2.4 2.4-3.5-3.5z'/>"
    "<path d='M13.2 7.8l3 3'/>"
    "<path d='M14.6 9.4L6.2 17.8v0l-.6 2.6 2.6-.6 8.4-8.4'/>"
    "<path d='M3.6 20.4l1.2-1.2'/>"
    "</g></svg>";

// Пиктограмма из SVG, перекрашенная в нужный цвет (рисунок одноцветный).
/// @brief Рисует одноцветный SVG и перекрашивает его в заданный цвет.
/// @param[in] svg    текст SVG
/// @param[in] size   сторона, логические пиксели
/// @param[in] dpr    отношение физических пикселей к логическим
/// @param[in] color  цвет заливки
/// @return Картинка.
QPixmap tintedSvg(const QByteArray &svg, int size, qreal dpr, const QColor &color)
{
    QPixmap pm(QSize(size, size) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QSvgRenderer renderer(svg);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    renderer.render(&p, QRectF(0, 0, size, size));
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(QRectF(0, 0, size, size), color);
    return pm;
}

/// @brief Линейно смешивает два цвета.
/// @param[in] a  цвет при `t = 0`
/// @param[in] b  цвет при `t = 1`
/// @param[in] t  доля, ограничивается `[0, 1]`
/// @return Смешанный цвет.
QColor mix(const QColor &a, const QColor &b, qreal t)
{
    t = std::clamp(t, 0.0, 1.0);
    return QColor::fromRgbF(float(a.redF() + (b.redF() - a.redF()) * t),
                            float(a.greenF() + (b.greenF() - a.greenF()) * t),
                            float(a.blueF() + (b.blueF() - a.blueF()) * t),
                            float(a.alphaF() + (b.alphaF() - a.alphaF()) * t));
}

/// @brief Возвращает код цвета `#RRGGBB` прописными буквами.
/// @param[in] c  цвет
/// @return Строка вида `#34E3C4` (без прозрачности).
QString hexOf(const QColor &c)
{
    return c.name(QColor::HexRgb).toUpper();
}

// Ручка-кружок: мягкая тень, белое кольцо, внутри — сам цвет.
/// @brief Рисует ручку-кружок: тень, кольцо (белое или акцентное) и цвет внутри.
/// @param[in,out] p      рисовальщик
/// @param[in]     c      центр ручки
/// @param[in]     fill   цвет внутри
/// @param[in]     r      радиус, пиксели
/// @param[in]     focus  true — кольцо цвета акцента (элемент в фокусе)
void drawHandle(QPainter &p, const QPointF &c, const QColor &fill, qreal r, bool focus)
{
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 70));
    p.drawEllipse(c + QPointF(0, 1.2), r + 2.6, r + 2.6);
    p.setBrush(focus ? kAccent : QColor(Qt::white));
    p.drawEllipse(c, r + 1.4, r + 1.4);
    p.setBrush(fill);
    p.drawEllipse(c, r - 1.2, r - 1.2);
    p.restore();
}

/// @brief Возвращает шрифт интерфейса.
/// @param[in] pt      размер, пункты
/// @param[in] weight  насыщенность
/// @return Шрифт гарнитуры `theme::uiFontFamily()` заданного размера и насыщенности.
QFont uiFont(qreal pt, int weight = QFont::Normal)
{
    QFont f(theme::uiFontFamily());
    f.setPointSizeF(pt);
    f.setWeight(QFont::Weight(weight));
    return f;
}

/// @brief Возвращает моноширинный шрифт.
/// @param[in] pt  размер, пункты
/// @return Шрифт гарнитуры `theme::monoFontFamily()` заданного размера.
QFont monoFont(qreal pt)
{
    QFont f(theme::monoFontFamily());
    f.setPointSizeF(pt);
    return f;
}

/// @brief Открывает INI-настройки окна (`TMC_Suite_Qt/ColorPicker.ini` профиля пользователя).
/// @return Новый объект; удаляет вызывающий код.
QSettings *openSettings()
{
    // INI в профиле пользователя: на Windows — %APPDATA%\TMC_Suite_Qt\ColorPicker.ini.
    return new QSettings(QSettings::IniFormat, QSettings::UserScope,
                         QStringLiteral("TMC_Suite_Qt"), QStringLiteral("ColorPicker"));
}

} // namespace

namespace tmccolor {

// --- Область «насыщенность (по горизонтали) / яркость (по вертикали)» ----------
/// @brief Область выбора: насыщенность по горизонтали, яркость по вертикали; мышь и стрелки.
class SvArea : public QWidget
{
public:
    /// @brief Создаёт область не меньше 318x232.
    /// @param[in] parent  родитель
    explicit SvArea(QWidget *parent) : QWidget(parent)
    {
        setMinimumSize(318, 232);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setCursor(Qt::CrossCursor);
        setFocusPolicy(Qt::StrongFocus);
        setToolTip(QStringLiteral("Насыщенность — по горизонтали, яркость — по вертикали"));
    }

    /// Вызывается при выборе: насыщенность и яркость, 0…1.
    std::function<void(double, double)> onChange;

    /// @brief Задаёт отображаемое состояние.
    /// @param[in] h  оттенок, 0…1
    /// @param[in] s  насыщенность, 0…1
    /// @param[in] v  яркость, 0…1
    /// @param[in] c  цвет ручки
    void setState(double h, double s, double v, const QColor &c)
    {
        m_h = h;
        m_s = s;
        m_v = v;
        m_c = c;
        update();
    }

protected:
    // Поле отступает от краёв на радиус ручки: ручка у края видна целиком.
    /// @brief Возвращает прямоугольник поля (с отступом на радиус ручки).
    /// @return Прямоугольник виджета, уменьшенный на 9 пикселей с каждой стороны.
    QRectF field() const { return QRectF(rect()).adjusted(9, 9, -9, -9); }

    /// @brief Рисует градиенты оттенка, белого и чёрного и ручку.
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF r = field();
        QPainterPath clip;
        clip.addRoundedRect(r, 10, 10);

        p.save();
        p.setClipPath(clip);
        p.fillRect(r, QColor::fromHsvF(float(m_h), 1.0f, 1.0f));
        QLinearGradient gx(r.topLeft(), r.topRight());
        gx.setColorAt(0, QColor(255, 255, 255));
        gx.setColorAt(1, QColor(255, 255, 255, 0));
        p.fillRect(r, gx);
        QLinearGradient gy(r.topLeft(), r.bottomLeft());
        gy.setColorAt(0, QColor(0, 0, 0, 0));
        gy.setColorAt(1, QColor(0, 0, 0));
        p.fillRect(r, gy);
        p.restore();

        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(hasFocus() ? QColor(52, 227, 196, 150) : QColor(255, 255, 255, 22), 1));
        p.drawRoundedRect(r.adjusted(0.5, 0.5, -0.5, -0.5), 10, 10);

        const QPointF c(r.left() + m_s * r.width(), r.top() + (1.0 - m_v) * r.height());
        drawHandle(p, c, m_c, 8, false);
    }

    /// @brief Переводит точку в (s, v) и вызывает `onChange`.
    /// @param[in] pos  точка в координатах виджета
    void pick(const QPointF &pos)
    {
        const QRectF r = field();
        const double s = std::clamp((pos.x() - r.left()) / r.width(), 0.0, 1.0);
        const double v = std::clamp(1.0 - (pos.y() - r.top()) / r.height(), 0.0, 1.0);
        if (onChange)
            onChange(s, v);
    }

    /// @brief Выбор левой кнопкой мыши.
    /// @param[in] e  событие мыши
    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton)
            pick(e->position());
    }
    /// @brief Перетаскивание с нажатой левой кнопкой.
    /// @param[in] e  событие мыши
    void mouseMoveEvent(QMouseEvent *e) override
    {
        if (e->buttons() & Qt::LeftButton)
            pick(e->position());
    }
    /// @brief Стрелки: шаг 0,01 (с Shift — 0,1).
    /// @param[in] e  событие клавиатуры
    void keyPressEvent(QKeyEvent *e) override
    {
        const double step = (e->modifiers() & Qt::ShiftModifier) ? 0.1 : 0.01;
        double s = m_s, v = m_v;
        switch (e->key()) {
        case Qt::Key_Left:  s -= step; break;
        case Qt::Key_Right: s += step; break;
        case Qt::Key_Up:    v += step; break;
        case Qt::Key_Down:  v -= step; break;
        default: QWidget::keyPressEvent(e); return;
        }
        if (onChange)
            onChange(std::clamp(s, 0.0, 1.0), std::clamp(v, 0.0, 1.0));
    }
    /// @brief Перерисовка рамки фокуса.
    /// @param[in] e  событие фокуса
    void focusInEvent(QFocusEvent *e) override { QWidget::focusInEvent(e); update(); }
    /// @brief Перерисовка рамки фокуса.
    /// @param[in] e  событие фокуса
    void focusOutEvent(QFocusEvent *e) override { QWidget::focusOutEvent(e); update(); }

private:
    /// @name Отображаемый цвет области (оттенок, насыщенность, яркость, 0…1)
    ///@{
    /// Отображаемые оттенок, насыщенность, яркость.
    double m_h = 0, m_s = 0, m_v = 0;
    //@}
    /// Цвет ручки.
    QColor m_c;
};

// --- Движок с градиентной дорожкой: оттенок и каналы R, G, B -------------------
/// @brief Движок с градиентной дорожкой (оттенок, каналы R/G/B); значение 0…1.
class GradientSlider : public QWidget
{
public:
    /// @brief Создаёт движок.
    /// @param[in] trackHeight  высота дорожки, пиксели
    /// @param[in] parent       родитель
    GradientSlider(int trackHeight, QWidget *parent)
        : QWidget(parent), m_track(trackHeight)
    {
        setFixedHeight(trackHeight + 14);
        setMinimumWidth(90);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::StrongFocus);
    }

    /// Вызывается при изменении значения пользователем (0…1).
    std::function<void(double)> onChange;

    /// @brief Задаёт градиент дорожки.
    /// @param[in] stops  точки градиента (позиция 0…1 и цвет)
    void setStops(const QGradientStops &stops) { m_stops = stops; update(); }
    /// @brief Задаёт значение (ограничивается 0…1).
    /// @param[in] v  значение; вне 0…1 ограничивается
    void setValue(double v) { m_value = std::clamp(v, 0.0, 1.0); update(); }
    /// @brief Задаёт цвет ручки.
    /// @param[in] c  цвет ручки
    void setHandleColor(const QColor &c) { m_handle = c; update(); }
    /// @brief Задаёт шаг колёсика и стрелок (по умолчанию 1/255).
    /// @param[in] step  шаг, доля диапазона 0…1
    void setStep(double step) { m_step = step; }

protected:
    /// @brief Возвращает отступ дорожки от краёв, пиксели.
    /// @return Всегда 9 пикселей.
    qreal pad() const { return 9.0; }   // как отступ поля SvArea

    /// @brief Рисует дорожку и ручку.
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF tr(pad(), (height() - m_track) / 2.0, width() - 2 * pad(), m_track);
        QLinearGradient g(QPointF(pad(), 0), QPointF(width() - pad(), 0));
        g.setStops(m_stops);
        p.setPen(QPen(QColor(255, 255, 255, 24), 1));
        p.setBrush(g);
        p.drawRoundedRect(tr.adjusted(0.5, 0.5, -0.5, -0.5), m_track / 2.0, m_track / 2.0);

        const qreal x = pad() + m_value * (width() - 2 * pad());
        drawHandle(p, QPointF(x, height() / 2.0), m_handle, m_track / 2.0 + 2.5, hasFocus());
    }

    /// @brief Переводит координату X в значение и вызывает `onChange`.
    /// @param[in] x  координата в виджете, пиксели
    void pick(qreal x)
    {
        const double v = std::clamp((x - pad()) / (width() - 2 * pad()), 0.0, 1.0);
        if (onChange)
            onChange(v);
    }

    /// @brief Выбор левой кнопкой мыши.
    /// @param[in] e  событие мыши
    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton)
            pick(e->position().x());
    }
    /// @brief Перетаскивание с нажатой левой кнопкой.
    /// @param[in] e  событие мыши
    void mouseMoveEvent(QMouseEvent *e) override
    {
        if (e->buttons() & Qt::LeftButton)
            pick(e->position().x());
    }
    /// @brief Колёсико: шаг вверх или вниз.
    /// @param[in,out] e  событие колёсика (принимается)
    void wheelEvent(QWheelEvent *e) override
    {
        const int d = e->angleDelta().y() > 0 ? 1 : -1;
        if (onChange)
            onChange(std::clamp(m_value + d * m_step, 0.0, 1.0));
        e->accept();
    }
    /// @brief Стрелки: один шаг (с Shift — десять).
    /// @param[in] e  событие клавиатуры
    void keyPressEvent(QKeyEvent *e) override
    {
        const double step = (e->modifiers() & Qt::ShiftModifier) ? m_step * 10 : m_step;
        if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Down) {
            if (onChange) onChange(std::clamp(m_value - step, 0.0, 1.0));
        } else if (e->key() == Qt::Key_Right || e->key() == Qt::Key_Up) {
            if (onChange) onChange(std::clamp(m_value + step, 0.0, 1.0));
        } else {
            QWidget::keyPressEvent(e);
        }
    }
    /// @brief Перерисовка рамки фокуса.
    /// @param[in] e  событие фокуса
    void focusInEvent(QFocusEvent *e) override { QWidget::focusInEvent(e); update(); }
    /// @brief Перерисовка рамки фокуса.
    /// @param[in] e  событие фокуса
    void focusOutEvent(QFocusEvent *e) override { QWidget::focusOutEvent(e); update(); }

private:
    /// Высота дорожки, пиксели.
    int m_track;
    /// Градиент дорожки.
    QGradientStops m_stops;
    /// Значение, 0…1.
    double m_value = 0;
    /// Шаг колёсика и стрелок.
    double m_step = 1.0 / 255.0;
    /// Цвет ручки.
    QColor m_handle = Qt::white;
};

// --- Образцы «было / стало» ----------------------------------------------------
/// @brief Образцы «было/стало» с кодами цветов; щелчок по «было» возвращает исходный цвет.
class ComparePreview : public QWidget
{
public:
    /// @brief Создаёт виджет высотой 100 пикселей.
    /// @param[in] parent  родитель
    explicit ComparePreview(QWidget *parent) : QWidget(parent)
    {
        setFixedHeight(100);
        setMouseTracking(true);
    }

    /// Вызывается по щелчку на «было».
    std::function<void()> onRevert;

    /// @brief Задаёт цвета «было» и «стало».
    /// @param[in] was  цвет «было»
    /// @param[in] now  цвет «стало»
    void set(const QColor &was, const QColor &now)
    {
        m_was = was;
        m_now = now;
        update();
    }

protected:
    /// @brief Возвращает прямоугольник образцов.
    /// @return Прямоугольник (0.5, 22.5, ширина - 1, 52), пиксели виджета.
    QRectF block() const { return QRectF(0.5, 22.5, width() - 1.0, 52.0); }
    /// @brief Возвращает левую половину образцов («было»).
    /// @return Левая половина прямоугольника `block()`.
    QRectF wasHalf() const
    {
        const QRectF b = block();
        return QRectF(b.left(), b.top(), b.width() / 2.0, b.height());
    }

    /// @brief Рисует подписи, образцы, подсказку возврата и коды цветов.
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF b = block();
        const qreal half = b.width() / 2.0;

        // Подписи над образцами.
        p.setFont(uiFont(7.5, QFont::DemiBold));
        p.setPen(kTextMute);
        p.drawText(QRectF(b.left() + 2, 0, half, 18), Qt::AlignLeft | Qt::AlignVCenter,
                   tmci18n::tr("БЫЛО", "BEFORE"));
        p.drawText(QRectF(b.left() + half + 2, 0, half - 4, 18), Qt::AlignLeft | Qt::AlignVCenter,
                   tmci18n::tr("СТАЛО", "AFTER"));

        QPainterPath clip;
        clip.addRoundedRect(b, 12, 12);
        p.save();
        p.setClipPath(clip);
        p.fillRect(QRectF(b.left(), b.top(), half, b.height()), m_was);
        p.fillRect(QRectF(b.left() + half, b.top(), half, b.height()), m_now);
        if (m_hoverWas && m_was.rgb() != m_now.rgb()) {
            // Подсказка, что щелчок вернёт исходный цвет: полупрозрачная плашка
            // со стрелкой «назад».
            const QRectF w = wasHalf();
            p.fillRect(w, QColor(0, 0, 0, 70));
            p.setPen(QPen(Qt::white, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            const QPointF c = w.center();
            p.setBrush(Qt::NoBrush);
            p.drawArc(QRectF(c.x() - 8, c.y() - 8, 16, 16), 40 * 16, 270 * 16);
            QPainterPath arrow;
            arrow.moveTo(c.x() + 7.6, c.y() - 7.2);
            arrow.lineTo(c.x() + 6.4, c.y() - 1.6);
            arrow.lineTo(c.x() + 1.2, c.y() - 3.6);
            p.drawPath(arrow);
        }
        p.restore();

        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(255, 255, 255, 26), 1));
        p.drawRoundedRect(b, 12, 12);

        // Коды цветов под образцами.
        p.setFont(monoFont(8.5));
        p.setPen(kTextDim);
        p.drawText(QRectF(b.left() + 2, b.bottom() + 4, half - 4, 20), Qt::AlignLeft | Qt::AlignVCenter,
                   hexOf(m_was));
        p.setPen(kText);
        p.drawText(QRectF(b.left() + half + 2, b.bottom() + 4, half - 4, 20),
                   Qt::AlignLeft | Qt::AlignVCenter, hexOf(m_now));
    }

    /// @brief Подсвечивает «было» под курсором.
    /// @param[in] e  событие мыши
    void mouseMoveEvent(QMouseEvent *e) override
    {
        const bool h = wasHalf().contains(e->position());
        if (h != m_hoverWas) {
            m_hoverWas = h;
            setCursor(h ? Qt::PointingHandCursor : Qt::ArrowCursor);
            setToolTip(h ? QStringLiteral("Вернуть исходный цвет") : QString());
            update();
        }
    }
    /// @brief Снимает подсветку.
    void leaveEvent(QEvent *) override
    {
        m_hoverWas = false;
        unsetCursor();
        update();
    }
    /// @brief Щелчок по «было» — `onRevert`.
    /// @param[in] e  событие мыши
    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && wasHalf().contains(e->position()) && onRevert)
            onRevert();
    }

private:
    /// @name Цвета образцов
    ///@{
    /// Цвета «было» и «стало».
    QColor m_was, m_now;
    //@}
    /// Курсор над «было».
    bool m_hoverWas = false;
};

// --- Плитка-образец палитры ----------------------------------------------------
/// @brief Плитка-образец цвета; отмечается рамкой, если совпадает с текущим цветом.
class Swatch : public QAbstractButton
{
public:
    /// @brief Создаёт плитку 28x28 с подсказкой (подпись и HEX).
    /// @param[in] c       цвет
    /// @param[in] name    подпись или пустая строка
    /// @param[in] parent  родитель
    Swatch(const QColor &c, const QString &name, QWidget *parent)
        : QAbstractButton(parent), m_color(c)
    {
        setFixedSize(28, 28);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::TabFocus);
        setToolTip(name.isEmpty() ? hexOf(c)
                                  : QStringLiteral("%1  ·  %2").arg(name, hexOf(c)));
        m_anim.setDuration(160);
        m_anim.setEasingCurve(QEasingCurve::OutCubic);
        QObject::connect(&m_anim, &QVariantAnimation::valueChanged, this,
                         [this](const QVariant &v) { m_hover = v.toReal(); update(); });
    }

    /// @brief Возвращает цвет плитки.
    /// @return Цвет, заданный при создании.
    QColor color() const { return m_color; }

    /// @brief Отмечает или снимает отметку плитки.
    /// @param[in] s  true — отметить, false — снять отметку
    void setSelected(bool s)
    {
        if (s != m_selected) {
            m_selected = s;
            update();
        }
    }

protected:
    /// @brief Запускает анимацию подсветки к значению 0 или 1.
    /// @param[in] target  конечное значение подсветки: 0 — погашена, 1 — полная
    void animateTo(qreal target)
    {
        m_anim.stop();
        m_anim.setStartValue(m_hover);
        m_anim.setEndValue(target);
        m_anim.start();
    }
    /// @brief Мышь вошла — подсветить.
    /// @param[in] e  событие входа указателя
    void enterEvent(QEnterEvent *e) override
    {
        QAbstractButton::enterEvent(e);
        animateTo(1.0);
    }
    /// @brief Мышь ушла — погасить.
    /// @param[in] e  событие ухода указателя
    void leaveEvent(QEvent *e) override
    {
        QAbstractButton::leaveEvent(e);
        animateTo(0.0);
    }

    /// @brief Рисует плитку, подсветку и рамку отметки/фокуса.
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const qreal grow = 1.2 * m_hover;
        const QRectF r = QRectF(rect()).adjusted(4.5 - grow, 4.5 - grow, -4.5 + grow, -4.5 + grow);
        p.setPen(QPen(mix(QColor(255, 255, 255, 30), kAccent, m_hover), 1));
        p.setBrush(m_color);
        p.drawRoundedRect(r, 6, 6);
        if (m_selected || hasFocus()) {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(m_selected ? kAccent : kTextDim, 1.6));
            p.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 8, 8);
        }
    }

private:
    /// Цвет плитки.
    QColor m_color;
    /// Плитка отмечена.
    bool m_selected = false;
    /// Подсветка, 0…1.
    qreal m_hover = 0;
    /// Анимация подсветки.
    QVariantAnimation m_anim;
};

} // namespace tmccolor

using namespace tmccolor;

namespace {

// Таблица стилей окна. Она стоит на самом окне и потому важнее общей таблицы
// приложения: там для диалогов из ресурсов Windows задан мелкий шрифт
// «MS Sans Serif 8», а этому окну нужен шрифт проекта.
/// @brief Возвращает таблицу стилей окна с подставленными гарнитурами.
/// @return Текст таблицы стилей Qt (QSS) с гарнитурами `theme::uiFontFamily()` и `theme::monoFontFamily()`.
QString pickerStyleSheet()
{
    QString css = QStringLiteral(R"(
QDialog#tmcColorPicker { background: #0a0e16; }
#tmcColorPicker QWidget { font-family: '@UI@'; font-size: 9.5pt; color: #e7eef7; background: transparent; }
#tmcColorPicker QLabel { background: transparent; padding: 0; }
#tmcColorPicker QLabel[role="h1"] { font-size: 13.5pt; font-weight: 700; }
#tmcColorPicker QLabel[role="caption"] { color: #6c7d97; font-size: 8pt; font-weight: 600; }
#tmcColorPicker QLabel[role="chan"] { color: #9fb0c8; font-family: '@MONO@'; font-size: 9pt; font-weight: 700; }
#tmcColorPicker QLabel[role="hint"] { color: #34e3c4; font-size: 8.5pt; }
#tmcColorPicker QLabel[role="empty"] { color: #6c7d97; font-size: 8.5pt; }
#tmcColorPicker QFrame[role="card"] {
    background: #131a28; border: 1px solid #243049; border-radius: 14px;
}
#tmcColorPicker QFrame[role="sep"] { background: #243049; border: none; max-height: 1px; min-height: 1px; }
#tmcColorPicker QLineEdit, #tmcColorPicker QSpinBox {
    background: #0a0e16; border: 1px solid #243049; border-radius: 8px;
    padding: 5px 8px; font-family: '@MONO@'; font-size: 9.5pt;
    selection-background-color: #34e3c4; selection-color: #0a0e16;
}
#tmcColorPicker QLineEdit:focus, #tmcColorPicker QSpinBox:focus { border: 1px solid #34e3c4; }
#tmcColorPicker QPushButton {
    background: #131a28; border: 1px solid #243049; border-radius: 10px;
    padding: 8px 18px; min-width: 92px; font-weight: 600;
}
#tmcColorPicker QPushButton:hover { border: 1px solid #34e3c4; }
#tmcColorPicker QPushButton:pressed { background: #1a2335; }
#tmcColorPicker QPushButton[primary="true"] {
    background: #34e3c4; color: #06231e; border: 1px solid #34e3c4; font-weight: 700;
}
#tmcColorPicker QPushButton[primary="true"]:hover { background: #5cebd1; border: 1px solid #5cebd1; }
#tmcColorPicker QPushButton[primary="true"]:pressed { background: #2bc9ad; }
#tmcColorPicker QToolButton {
    background: #131a28; border: 1px solid #243049; border-radius: 8px; padding: 5px;
}
#tmcColorPicker QToolButton:hover { border: 1px solid #34e3c4; }
#tmcColorPicker QToolButton:checked { background: #1a2335; border: 1px solid #34e3c4; }
#tmcColorPicker QToolTip { background: #131a28; color: #e7eef7; border: 1px solid #34e3c4; padding: 4px 6px; }
)");
    css.replace(QStringLiteral("@UI@"), theme::uiFontFamily());
    css.replace(QStringLiteral("@MONO@"), theme::monoFontFamily());
    return css;
}

/// @brief Создаёт подпись с ролью оформления.
/// @param[in] text    текст
/// @param[in] role    значение свойства `role`
/// @param[in] parent  родитель
/// @return Новая подпись.
QLabel *makeLabel(const QString &text, const char *role, QWidget *parent)
{
    QLabel *l = new QLabel(text, parent);
    l->setProperty("role", QString::fromLatin1(role));
    return l;
}

/// @brief Создаёт панель-карточку.
/// @param[in] parent  родитель
/// @return Новая панель.
QFrame *makeCard(QWidget *parent)
{
    QFrame *f = new QFrame(parent);
    f->setProperty("role", QStringLiteral("card"));
    return f;
}

} // namespace

// --- Окно -------------------------------------------------------------------------

TmcColorPickerDialog::TmcColorPickerDialog(const QColor &initial, QWidget *parent,
                                           const QString &title)
    : QDialog(parent)
{
    m_initial = initial.isValid() ? initial.toRgb() : QColor(Qt::white);
    m_initial.setAlpha(255);
    setObjectName(QStringLiteral("tmcColorPicker"));
    setStyleSheet(pickerStyleSheet());
    buildUi(title);

    m_anim = new QVariantAnimation(this);
    m_anim->setDuration(220);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        if (m_anim->state() == QAbstractAnimation::Running)
            setFromRgb(v.value<QColor>(), Source::Anim);
    });
    connect(m_anim, &QVariantAnimation::finished, this,
            [this]() { setFromRgb(m_anim->endValue().value<QColor>(), Source::Swatch); });

    m_pickTimer = new QTimer(this);
    m_pickTimer->setInterval(30);
    connect(m_pickTimer, &QTimer::timeout, this, [this]() { pickTick(); });

    setFromRgb(m_initial, Source::None);
}

TmcColorPickerDialog::~TmcColorPickerDialog() = default;

void TmcColorPickerDialog::buildUi(const QString &title)
{
    setWindowTitle(title.isEmpty() ? QStringLiteral("Выбор цвета") : title);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 16, 20, 18);
    root->setSpacing(14);

    // Заголовок и пипетка.
    QHBoxLayout *head = new QHBoxLayout;
    head->setSpacing(10);
    head->addWidget(makeLabel(windowTitle(), "h1", this));
    head->addStretch(1);
    m_pickBtn = new QToolButton(this);
    m_pickBtn->setCheckable(true);
    m_pickBtn->setCursor(Qt::PointingHandCursor);
    m_pickBtn->setToolTip(QStringLiteral("Пипетка: взять цвет с любого места экрана"));
    {
        QIcon icon;
        const qreal dpr = devicePixelRatioF();
        icon.addPixmap(tintedSvg(QByteArray(kPipetteSvg), 18, dpr, kText), QIcon::Normal, QIcon::Off);
        icon.addPixmap(tintedSvg(QByteArray(kPipetteSvg), 18, dpr, kAccent), QIcon::Normal, QIcon::On);
        icon.addPixmap(tintedSvg(QByteArray(kPipetteSvg), 18, dpr, kAccent), QIcon::Active, QIcon::Off);
        m_pickBtn->setIcon(icon);
        m_pickBtn->setIconSize(QSize(18, 18));
    }
    m_pickBtn->setFixedSize(34, 34);
    connect(m_pickBtn, &QToolButton::clicked, this, [this](bool on) {
        if (on)
            startPicking();
        else
            stopPicking(true);
    });
    head->addWidget(m_pickBtn);
    root->addLayout(head);

    // Основная часть: слева область и оттенок, справа образцы и числа.
    QHBoxLayout *body = new QHBoxLayout;
    body->setSpacing(16);

    QFrame *leftCard = makeCard(this);
    QVBoxLayout *left = new QVBoxLayout(leftCard);
    left->setContentsMargins(5, 5, 5, 8);
    left->setSpacing(2);
    m_sv = new SvArea(leftCard);
    m_sv->onChange = [this](double s, double v) {
        m_anim->stop();
        setFromHsv(m_h, s, v, Source::Sv);
    };
    left->addWidget(m_sv, 1);
    m_hue = new GradientSlider(12, leftCard);
    m_hue->setToolTip(QStringLiteral("Оттенок"));
    m_hue->setStep(1.0 / 360.0);
    {
        QGradientStops stops;
        for (int i = 0; i <= 12; ++i)
            stops << QGradientStop(i / 12.0, QColor::fromHsvF(float(std::min(i / 12.0, 0.9999)), 1.0f, 1.0f));
        m_hue->setStops(stops);
    }
    m_hue->onChange = [this](double h) {
        m_anim->stop();
        setFromHsv(std::min(h, 0.9999), m_s, m_v, Source::Hue);
    };
    left->addWidget(m_hue);
    body->addWidget(leftCard, 1);

    QFrame *rightCard = makeCard(this);
    rightCard->setFixedWidth(250);
    QVBoxLayout *right = new QVBoxLayout(rightCard);
    right->setContentsMargins(14, 12, 14, 14);
    right->setSpacing(10);
    m_preview = new ComparePreview(rightCard);
    m_preview->onRevert = [this]() { setColor(m_initial, true); };
    right->addWidget(m_preview);

    QGridLayout *fields = new QGridLayout;
    fields->setHorizontalSpacing(10);
    fields->setVerticalSpacing(8);
    fields->addWidget(makeLabel(QStringLiteral("HEX"), "chan", rightCard), 0, 0);
    m_hex = new QLineEdit(rightCard);
    m_hex->setMaxLength(7);
    m_hex->setValidator(new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("^#?[0-9A-Fa-f]{0,6}$")), m_hex));
    m_hex->setToolTip(QStringLiteral("Код цвета: #RRGGBB или #RGB"));
    connect(m_hex, &QLineEdit::textEdited, this, [this](const QString &t) {
        QString h = t.trimmed();
        if (h.startsWith(QLatin1Char('#')))
            h.remove(0, 1);
        if (h.size() == 3)
            h = QString(h[0]) + h[0] + h[1] + h[1] + h[2] + h[2];
        if (h.size() != 6)
            return;
        m_anim->stop();
        setFromRgb(QColor(QLatin1Char('#') + h), Source::Hex);
    });
    connect(m_hex, &QLineEdit::editingFinished, this,
            [this]() { m_hex->setText(hexOf(m_color)); });
    fields->addWidget(m_hex, 0, 1, 1, 2);

    const char *names[3] = { "R", "G", "B" };
    for (int i = 0; i < 3; ++i) {
        fields->addWidget(makeLabel(QString::fromLatin1(names[i]), "chan", rightCard), i + 1, 0);
        m_chan[i] = new GradientSlider(8, rightCard);
        m_chan[i]->setMinimumWidth(100);
        m_chan[i]->onChange = [this, i](double v) {
            m_anim->stop();
            int rgb[3] = { m_color.red(), m_color.green(), m_color.blue() };
            rgb[i] = int(std::lround(v * 255.0));
            setFromRgb(QColor(rgb[0], rgb[1], rgb[2]), Source::Rgb);
        };
        fields->addWidget(m_chan[i], i + 1, 1);
        m_spin[i] = new QSpinBox(rightCard);
        m_spin[i]->setRange(0, 255);
        m_spin[i]->setButtonSymbols(QAbstractSpinBox::NoButtons);
        m_spin[i]->setAlignment(Qt::AlignRight);
        m_spin[i]->setFixedWidth(54);
        connect(m_spin[i], &QSpinBox::valueChanged, this, [this](int) {
            if (m_updating)
                return;
            m_anim->stop();
            setFromRgb(QColor(m_spin[0]->value(), m_spin[1]->value(), m_spin[2]->value()),
                       Source::Rgb);
        });
        fields->addWidget(m_spin[i], i + 1, 2);
    }
    fields->setColumnStretch(1, 1);
    right->addLayout(fields);
    right->addStretch(1);
    body->addWidget(rightCard);
    root->addLayout(body, 1);

    // Образцы: палитра проекта, классические цвета графиков, недавние.
    QFrame *palCard = makeCard(this);
    QGridLayout *pal = new QGridLayout(palCard);
    pal->setContentsMargins(14, 10, 14, 10);
    pal->setHorizontalSpacing(12);
    pal->setVerticalSpacing(4);

    auto addRow = [&](int row, const QString &caption) {
        QLabel *cap = makeLabel(caption, "caption", palCard);
        cap->setMinimumWidth(118);
        pal->addWidget(cap, row, 0, Qt::AlignVCenter);
        QHBoxLayout *h = new QHBoxLayout;
        h->setSpacing(2);
        h->setContentsMargins(0, 0, 0, 0);
        pal->addLayout(h, row, 1);
        return h;
    };

    QHBoxLayout *projRow = addRow(0, QStringLiteral("ПАЛИТРА ПРОЕКТА"));
    for (const NamedColor &nc : kProjectPalette)
        projRow->addWidget(makeSwatch(QColor::fromRgb(nc.rgb), QString::fromUtf8(nc.name)));
    projRow->addStretch(1);

    QHBoxLayout *classicRow = addRow(1, QStringLiteral("ЦВЕТА ГРАФИКОВ"));
    for (const NamedColor &nc : kClassicPalette)
        classicRow->addWidget(makeSwatch(QColor::fromRgb(nc.rgb), QString::fromUtf8(nc.name)));
    classicRow->addStretch(1);

    m_recentRow = addRow(2, QStringLiteral("НЕДАВНИЕ"));
    fillRecentRow();
    pal->setColumnStretch(1, 1);
    root->addWidget(palCard);

    // Кнопки. Главная (OK) — акцентная, как во всём оформлении проекта.
    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->setSpacing(10);
    m_hint = makeLabel(QString(), "hint", this);
    buttons->addWidget(m_hint, 1);
    QPushButton *cancel = new QPushButton(QStringLiteral("Отмена"), this);
    cancel->setCursor(Qt::PointingHandCursor);
    cancel->setAutoDefault(false);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    QPushButton *ok = new QPushButton(QStringLiteral("OK"), this);
    ok->setProperty("primary", true);
    ok->setCursor(Qt::PointingHandCursor);
    ok->setDefault(true);
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
    buttons->addWidget(cancel);
    buttons->addWidget(ok);
    root->addLayout(buttons);

    resize(sizeHint().expandedTo(QSize(640, 520)));
}

Swatch *TmcColorPickerDialog::makeSwatch(const QColor &c, const QString &name)
{
    Swatch *s = new Swatch(c, name, this);
    connect(s, &QAbstractButton::clicked, this, [this, s]() { setColor(s->color(), true); });
    m_swatches << s;
    return s;
}

void TmcColorPickerDialog::fillRecentRow()
{
    // Очистить ряд (при повторном заполнении) и разложить недавние цвета.
    while (QLayoutItem *it = m_recentRow->takeAt(0)) {
        if (QWidget *w = it->widget()) {
            m_swatches.removeAll(static_cast<Swatch *>(w));
            w->deleteLater();
        }
        delete it;
    }
    m_recentEmpty = nullptr;
    const QList<QColor> recent = recentColors();
    if (recent.isEmpty()) {
        m_recentEmpty = makeLabel(QStringLiteral("Здесь появятся выбранные цвета"), "empty", this);
        m_recentEmpty->setMinimumHeight(28);
        m_recentRow->addWidget(m_recentEmpty);
    }
    for (const QColor &c : recent)
        m_recentRow->addWidget(makeSwatch(c, QString()));
    m_recentRow->addStretch(1);
}

void TmcColorPickerDialog::setColor(const QColor &c, bool animated)
{
    if (!c.isValid())
        return;
    QColor target = c.toRgb();
    target.setAlpha(255);
    if (!animated || !isVisible()) {
        m_anim->stop();
        setFromRgb(target, Source::Swatch);
        return;
    }
    m_anim->stop();
    m_anim->setStartValue(m_color);
    m_anim->setEndValue(target);
    m_anim->start();
}

void TmcColorPickerDialog::setFromRgb(const QColor &c, Source src)
{
    QColor rgb = c.toRgb();
    rgb.setAlpha(255);
    float h = 0, s = 0, v = 0;
    rgb.getHsvF(&h, &s, &v);
    m_color = rgb;
    // Оттенок серого не определён — оставляем прежний; у чёрного не определена
    // и насыщенность.
    if (h >= 0 && s > 0 && v > 0)
        m_h = h;
    if (v > 0)
        m_s = s;
    m_v = v;
    refresh(src);
}

void TmcColorPickerDialog::setFromHsv(double h, double s, double v, Source src)
{
    m_h = std::clamp(h, 0.0, 0.9999);
    m_s = std::clamp(s, 0.0, 1.0);
    m_v = std::clamp(v, 0.0, 1.0);
    m_color = QColor::fromHsvF(float(m_h), float(m_s), float(m_v)).toRgb();
    refresh(src);
}

void TmcColorPickerDialog::refresh(Source src)
{
    m_updating = true;
    m_sv->setState(m_h, m_s, m_v, m_color);
    m_hue->setValue(m_h);
    m_hue->setHandleColor(QColor::fromHsvF(float(m_h), 1.0f, 1.0f));

    const int r = m_color.red(), g = m_color.green(), b = m_color.blue();
    m_chan[0]->setStops({ QGradientStop(0, QColor(0, g, b)), QGradientStop(1, QColor(255, g, b)) });
    m_chan[1]->setStops({ QGradientStop(0, QColor(r, 0, b)), QGradientStop(1, QColor(r, 255, b)) });
    m_chan[2]->setStops({ QGradientStop(0, QColor(r, g, 0)), QGradientStop(1, QColor(r, g, 255)) });
    const int vals[3] = { r, g, b };
    for (int i = 0; i < 3; ++i) {
        m_chan[i]->setValue(vals[i] / 255.0);
        m_chan[i]->setHandleColor(m_color);
        if (m_spin[i]->value() != vals[i]) {
            const QSignalBlocker block(m_spin[i]);
            m_spin[i]->setValue(vals[i]);
        }
    }
    if (src != Source::Hex)
        m_hex->setText(hexOf(m_color));
    m_preview->set(m_initial, m_color);
    for (Swatch *s : m_swatches)
        s->setSelected(s->color().rgb() == m_color.rgb());
    m_updating = false;
}

// --- Пипетка ---------------------------------------------------------------------

void TmcColorPickerDialog::startPicking()
{
    if (m_picking)
        return;
    m_anim->stop();
    m_picking = true;
    m_beforePick = m_color;
    m_pickBtn->setChecked(true);
    m_hint->setText(QStringLiteral("Щёлкните по любому месту экрана · Esc — отмена"));
    grabMouse(Qt::CrossCursor);
    grabKeyboard();
    m_pickTimer->start();
}

void TmcColorPickerDialog::stopPicking(bool keep)
{
    if (!m_picking)
        return;
    m_pickTimer->stop();
    releaseMouse();
    releaseKeyboard();
    m_picking = false;
    m_pickBtn->setChecked(false);
    m_hint->clear();
    if (!keep)
        setFromRgb(m_beforePick, Source::Pick);
}

void TmcColorPickerDialog::pickTick()
{
    const QPoint pos = QCursor::pos();
    QScreen *screen = QGuiApplication::screenAt(pos);
    if (!screen)
        return;
    const QRect g = screen->geometry();
    const QImage img = screen->grabWindow(0, pos.x() - g.x(), pos.y() - g.y(), 1, 1).toImage();
    if (img.isNull())
        return;
    setFromRgb(img.pixelColor(0, 0), Source::Pick);
}

void TmcColorPickerDialog::keyPressEvent(QKeyEvent *e)
{
    if (m_picking) {
        if (e->key() == Qt::Key_Escape)
            stopPicking(false);
        else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
            pickTick();
            stopPicking(true);
        }
        return;
    }
    QDialog::keyPressEvent(e);
}

void TmcColorPickerDialog::mousePressEvent(QMouseEvent *e)
{
    if (m_picking) {
        if (e->button() == Qt::LeftButton) {
            pickTick();
            stopPicking(true);
        } else {
            stopPicking(false);
        }
        return;
    }
    QDialog::mousePressEvent(e);
}

void TmcColorPickerDialog::accept()
{
    stopPicking(true);
    if (m_anim->state() == QAbstractAnimation::Running) {
        m_anim->stop();
        setFromRgb(m_anim->endValue().value<QColor>(), Source::Swatch);
    }
    rememberColor(m_color);
    QDialog::accept();
}

void TmcColorPickerDialog::reject()
{
    stopPicking(false);
    QDialog::reject();
}

// --- Недавние цвета -------------------------------------------------------------

QList<QColor> TmcColorPickerDialog::recentColors()
{
    // Хранятся одной строкой без «#»: «34E3C4 B07CFF …» — так файл INI
    // читается однозначно (символ «#» в INI бывает началом комментария).
    QScopedPointer<QSettings> s(openSettings());
    const QStringList list = s->value(QStringLiteral("Recent/Colors")).toString()
                                 .split(QLatin1Char(' '), Qt::SkipEmptyParts);
    QList<QColor> out;
    for (const QString &h : list) {
        const QColor c(QLatin1Char('#') + h);
        if (c.isValid() && h.size() == 6 && out.size() < kRecentMax)
            out << c;
    }
    return out;
}

void TmcColorPickerDialog::rememberColor(const QColor &c)
{
    if (!c.isValid())
        return;
    QList<QColor> list = recentColors();
    const QRgb key = c.rgb();
    list.erase(std::remove_if(list.begin(), list.end(),
                              [key](const QColor &x) { return x.rgb() == key; }),
               list.end());
    list.prepend(c.toRgb());
    while (list.size() > kRecentMax)
        list.removeLast();
    QStringList hex;
    for (const QColor &x : list)
        hex << hexOf(x).mid(1);
    QScopedPointer<QSettings> s(openSettings());
    s->setValue(QStringLiteral("Recent/Colors"), hex.join(QLatin1Char(' ')));
}

bool tmcPickColor(QWidget *parent, const QColor &initial, QColor *result, const QString &title)
{
    TmcColorPickerDialog dlg(initial, parent, title);
    if (dlg.exec() != QDialog::Accepted)
        return false;
    if (result)
        *result = dlg.color();
    return true;
}
