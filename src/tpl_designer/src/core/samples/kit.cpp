/// @file kit.cpp
/// @brief Реализация сборщика задания `kit::Builder` и правил шага, времени и чисел паспорта.
///
/// Часть модуля «Ядро TPL Designer», каталог образцов. Описание функций — в `kit.h`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#samples
#include "kit.h"

#include "lang.h"
#include "physics.h"
#include "placement.h"

#include <algorithm>
#include <cmath>

namespace tpl::samples::kit {

Builder::Builder(const QString &name)
{
    p.name = name;
    p.kernel = Kernel::H;
    p.unit = LengthUnit::Millimeter;
}

Shape &Builder::rect(const QString &name, Material m, double x0, double x1, double y0, double y1)
{
    Shape s;
    s.id = take();
    s.name = name;
    s.kind = ShapeKind::Rectangle;
    s.material = m;
    s.xMin = std::min(x0, x1);
    s.xMax = std::max(x0, x1);
    s.yMin = std::min(y0, y1);
    s.yMax = std::max(y0, y1);
    p.shapes << s;
    return p.shapes.last();
}

Shape &Builder::dielectric(const QString &name, double eps, double x0, double x1, double y0,
                           double y1)
{
    Shape &s = rect(name, Material::Dielectric, x0, x1, y0, y1);
    s.eps = eps;
    return s;
}

Shape &Builder::polyline(const QString &name, Material m, const QVector<QPointF> &pts)
{
    Shape s;
    s.id = take();
    s.name = name;
    s.kind = ShapeKind::Polyline;
    s.material = m;
    s.points = pts;
    p.shapes << s;
    return p.shapes.last();
}

Shape &Builder::polygon(const QString &name, Material m, const QVector<QPointF> &pts)
{
    Shape s;
    s.id = take();
    s.name = name;
    s.kind = ShapeKind::Polygon;
    s.material = m;
    s.points = pts;
    p.shapes << s;
    return p.shapes.last();
}

Shape &Builder::circle(const QString &name, Material m, const QPointF &c, double r)
{
    Shape s;
    s.id = take();
    s.name = name;
    s.kind = ShapeKind::Circle;
    s.material = m;
    s.center = c;
    s.rMin = 0.0;
    s.rMax = r;
    s.fiMin = 0.0;
    s.fiMax = 2.0 * 3.14159265358979323846;
    p.shapes << s;
    return p.shapes.last();
}

Port &Builder::port(const QString &name, PortSide side, double from, double to, bool active,
                    PortMode mode)
{
    Port q;
    q.id = take();
    q.name = name;
    q.side = side;
    q.from = std::min(from, to);
    q.to = std::max(from, to);
    q.active = active;
    q.mode = mode;
    p.ports << q;
    return p.ports.last();
}

void Builder::guideX(const QString &name, double x0, double x1, double ylo, double yhi)
{
    const double d = p.delta;
    rect(name + tpl::tr(": нижняя стенка", ": bottom wall"), Material::Metal, x0, x1, ylo - d, ylo);
    rect(name + tpl::tr(": верхняя стенка", ": top wall"), Material::Metal, x0, x1, yhi, yhi + d);
}


void Builder::absorberFrame(bool left, bool right, bool bottom, bool top, int cells)
{
    // Как в авторских заданиях (`hyper_sheath_0p4.TPL`): поглотитель толщиной
    // несколько шагов вплотную к краю. Углы не перекрываются — иначе два блока
    // легли бы в одни узлы.
    const Box &b = p.box;
    const double t = cells * p.delta;
    if (right)
        rect(tpl::tr("поглотитель справа", "absorber right"), Material::Absorber, b.xMax - t, b.xMax, b.yMin,
             b.yMax);
    if (left)
        rect(tpl::tr("поглотитель слева", "absorber left"), Material::Absorber, b.xMin, b.xMin + t, b.yMin,
             b.yMax);
    const double x0 = left ? b.xMin + t : b.xMin;
    const double x1 = right ? b.xMax - t : b.xMax;
    if (bottom)
        rect(tpl::tr("поглотитель снизу", "absorber bottom"), Material::Absorber, x0, x1, b.yMin, b.yMin + t);
    if (top)
        rect(tpl::tr("поглотитель сверху", "absorber top"), Material::Absorber, x0, x1, b.yMax - t, b.yMax);
}

void Builder::freqs(double lo, double hi, int n)
{
    p.freqs = band(lo, hi, n);
}

void Builder::freq(double f)
{
    p.freqs = {f};
}

void Builder::outputs(bool sMatrix, bool pattern)
{
    p.output.signalFile = p.name;
    p.output.sMatrixFile = p.name;
    p.output.patternFile = p.name;
    p.output.sMatrix = sMatrix;
    p.output.directionalPattern = pattern;
    p.output.topology = false;
    p.output.fields = OutputSettings::Fields::Off;
}

void Builder::time(double tNs)
{
    // Круглое число: в задании его читает человек.
    const double t = std::ceil(tNs / 5.0) * 5.0;
    p.timeMax = t;
    const double win = std::max(3.0, t * 0.25);
    p.output.sMatrixTMin = t - win;
    p.output.sMatrixTMax = t;
    p.output.patternTMin = t - win;
    p.output.patternTMax = t;
}

Build Builder::done(QVector<Row> theory, QStringList notes)
{
    // Окно ДН «до конца счёта» ядро не закроет: оно дотягивает окно до целого числа
    // периодов и при конце не раньше TIME файлов ДН не пишет (Б-13). Частоты к этому
    // месту уже известны — подрезаем окно на период самой низкой из них.
    if (p.output.directionalPattern)
        p.output.patternTMax = patternWindowSafeEnd(p, p.output.patternTMin, p.output.patternTMax);

    Build b;
    b.project = p;
    b.theory = std::move(theory);
    b.notes = std::move(notes);
    return b;
}

double chooseDelta(double lambdaMm, double featureMm, double perLambda, double perFeature)
{
    double d = std::min(lambdaMm / perLambda, featureMm / perFeature);
    const double e = std::pow(10.0, std::floor(std::log10(d)));
    const double m = d / e;
    // Ряд подробнее, чем 1-2-5: при грубом ряде 1.87 мм округлялось до 1 мм, и задание
    // считалось втрое дольше без всякой пользы для точности.
    double nice = 1.0;
    for (double c : {1.25, 1.5, 2.0, 2.5, 3.0, 4.0, 5.0, 6.0, 8.0}) {
        if (m >= c)
            nice = c;
    }
    return nice * e;
}

double deltaFitting(double size, double maxDelta)
{
    const double n = std::ceil(size / maxDelta - 1e-9);
    // Шесть знаков: шаг сетки пишется в файл, а 0.5715000000001 читать неприятно.
    return std::round(size / n * 1e6) / 1e6;
}

double snap(double v, double delta)
{
    return std::round(v / delta) * delta;
}

double settleTime(double lengthMm, double fGHz, double fcGHz, double extraNs)
{
    const double r = fcGHz > 0.0 ? std::clamp(fcGHz / fGHz, 0.0, 0.95) : 0.0;
    const double vg = kLightSpeed * std::sqrt(1.0 - r * r); // мм/нс
    return 10.0 + 6.0 * lengthMm / vg + extraNs;
}

QString num(double v, int digits)
{
    if (v == 0.0)
        return QStringLiteral("0");
    QString s = QString::number(v, 'g', digits);
    if (s.contains(QLatin1Char('e')))
        s = QString::number(v, 'f', std::max(0, digits - 1 - int(std::floor(std::log10(std::fabs(v))))));
    return s;
}

QString ghz(double v) { return num(v, 4) + tpl::tr(" ГГц", " GHz"); }
QString mm(double v) { return num(v, 4) + tpl::tr(" мм", " mm"); }

QString db(double mag)
{
    return QString::number(20.0 * std::log10(std::max(mag, 1e-12)), 'f', 1)
           + tpl::tr(" дБ", " dB");
}

QVector<double> band(double lo, double hi, int n)
{
    QVector<double> f;
    if (n <= 1 || hi <= lo) {
        f << lo;
        return f;
    }
    for (int i = 0; i < n; ++i) {
        // Три знака после точки — чтобы в файле стояло 8.41, а не 8.4100000001.
        const double v = lo + (hi - lo) * i / (n - 1);
        f << std::round(v * 1000.0) / 1000.0;
    }
    return f;
}

} // namespace tpl::samples::kit
