/// @file signalpreview.cpp
/// @brief Реализация рисования формы сигнала и предпросмотра сигнала порта со спектром.
///
/// Часть интерфейса редактора заданий.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-signals
#include "signalpreview.h"

#include "emitter.h"
#include "expr.h"
#include "physics.h"
#include "sigfile.h"
#include "dtheme.h"

#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QRectF>

#include <algorithm>
#include <cmath>
#include "lang.h"

using namespace tpl;

namespace {

/// Число pi.
constexpr double kPi = 3.14159265358979323846;

/// Сколько точек берём на график и на спектр. Больше не нужно: пикселей меньше,
/// а спектр от четырёх тысяч выборок уже гладкий.
constexpr int kSamples = 4096;
/// Число столбиков спектра на картинке.
constexpr int kBins = 256;

/// Оценка сверху на |a(t)| по выборкам — по ней масштабируем график.
/// @brief Возвращает максимум модуля по массиву (масштаб графика).
/// @param[in] v  выборки.
/// @return max |v[i]|; 1, если максимум меньше 1e-12 (пустой или нулевой сигнал).
double peakOf(const QVector<double> &v)
{
    double m = 0.0;
    for (double x : v)
        m = std::max(m, std::fabs(x));
    return m > 1e-12 ? m : 1.0;
}

} // namespace

void paintWaveform(QPainter *p, const QRectF &box, const Waveshape &w, bool grid)
{
    const TimeWindow sup = waveSupport(w);
    const double len = sup.length();
    if (len <= 0.0) {
        p->setPen(dtheme::textMute());
        p->drawText(box, Qt::AlignCenter, tpl::tr("нет сигнала", "no signal"));
        return;
    }

    const ExprProgram prog = waveProgram(w);

    // Точек берём с запасом по ширине: у меандра на 3.9 ГГц за 40 нс сто пятьдесят
    // периодов, и при одной выборке на пиксель картинка превратилась бы в муар.
    const int n = std::max(400, int(box.width()) * 4);
    QVector<double> v(n + 1);
    for (int i = 0; i <= n; ++i)
        v[i] = waveValue(w, sup.tMin + len * i / n, &prog);
    const double peak = peakOf(v);

    const double midY = box.center().y();
    if (grid) {
        p->setPen(QPen(dtheme::line(), 1, Qt::DashLine));
        p->drawLine(QPointF(box.left(), midY), QPointF(box.right(), midY));
    }

    QPainterPath path;
    for (int i = 0; i <= n; ++i) {
        const QPointF pt(box.left() + box.width() * i / n,
                         midY - (v.at(i) / peak) * (box.height() / 2 - 1));
        if (i == 0)
            path.moveTo(pt);
        else
            path.lineTo(pt);
    }
    p->setPen(QPen(dtheme::accent(), grid ? 1.2 : 1.4));
    p->setBrush(Qt::NoBrush);
    p->drawPath(path);
}

QPixmap waveformIcon(const Waveshape &w, const QSize &size, bool selected)
{
    QPixmap pm(size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    const QRectF r = QRectF(QPointF(0, 0), QSizeF(size)).adjusted(1, 1, -1, -1);
    p.setPen(QPen(selected ? dtheme::accent() : dtheme::line(), 1));
    p.setBrush(dtheme::bg());
    p.drawRoundedRect(r, 6, 6);
    paintWaveform(&p, r.adjusted(4, 4, -4, -4), w, false);
    return pm;
}

SignalPreview::SignalPreview(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(150);
}



void SignalPreview::setPort(const Port &port, const Project &project)
{
    m_port = port;
    m_project = project;
    if (m_project.timeMax <= 0.0)
        m_project.timeMax = 1.0;
    if (m_project.freqs.isEmpty())
        m_project.freqs = {4.0};
    resample();
    update();
}

/// Повторяет формулу ядра. Держать в согласии с ampExpression() и waveValue(): если
/// разойдутся, график начнёт врать — а он тут именно для того, чтобы не врал.
double SignalPreview::sampleAt(double t) const
{
    const TimeWindow w = portWindow(m_port, m_project.timeMax);
    if (t < w.tMin || t > w.tMax)
        return 0.0; // окно режет резко — без фронтов

    if (m_port.signal.source == SignalSource::File) {
        // Значение из файла. Программу формулы разбирать на каждую выборку нельзя —
        // она разобрана один раз в resample(), поэтому здесь путь без неё, а с ней
        // считает resample().
        return waveValue(m_port.signal.wave, t);
    }

    double amp = m_port.signal.amplitude;
    if (m_port.signal.shape == SignalShape::Gaussian && m_port.signal.duration > 0.0) {
        const double x = (t - m_port.signal.center) / m_port.signal.duration;
        amp *= std::exp(-x * x);
    }
    // t в нс, f в ГГц: 2*pi*f*t даёт радианы без множителей.
    return amp * std::sin(2 * kPi * m_project.freqs.first() * t + m_port.signal.phase);
}

void SignalPreview::resample()
{
    m_samples.clear();
    m_spec.clear();
    m_peak = 1.0;
    m_specMax = 1.0;

    if (!m_port.active)
        return;

    const bool file = m_port.signal.source == SignalSource::File;
    const ExprProgram prog = file ? waveProgram(m_port.signal.wave) : ExprProgram();
    const TimeWindow win = portWindow(m_port, m_project.timeMax);

    // Выборок — не меньше четырёх на период верхней частоты сигнала.
    //
    // ⚠️ Раньше их всегда было 4096, и гребёнка до 100 ГГц на 20 нс шла почти по
    // частоте Найквиста: верхний тон рисовался как попало, а спектр за ним не значил
    // ничего. Потолок 65536 — дальше картинке уже нечего добавить.
    double fTopWave = 2.0 * m_project.freqs.first();
    if (file) {
        const Band b = waveBand(m_port.signal.wave);
        if (b.known)
            fTopWave = std::max(fTopWave, 1.3 * b.hi);
    }
    int nSamples = kSamples;
    while (nSamples < 65536 && nSamples < m_project.timeMax * fTopWave * 4.0)
        nSamples *= 2;
    m_samples.resize(nSamples);
    const double dt = m_project.timeMax / (nSamples - 1);
    for (int i = 0; i < nSamples; ++i) {
        const double t = dt * i;
        double a = 0.0;
        if (t >= win.tMin && t <= win.tMax)
            a = file ? waveValue(m_port.signal.wave, t, &prog) : sampleAt(t);
        m_samples[i] = a;
    }
    m_peak = peakOf(m_samples);

    // До какой частоты смотреть. С запасом за полосу: важно ВИДЕТЬ, что гармоники
    // уехали за одномодовый диапазон, а не догадываться об этом.
    const double carrier = m_project.freqs.first();
    double top = 2.0 * carrier;
    if (file) {
        const Band b = waveBand(m_port.signal.wave);
        if (b.known && b.hi > 0.0)
            top = std::max(top, 1.3 * b.hi);
    }
    // Выше частоты Найквиста наших выборок спектр всё равно не значит ничего.
    m_fMax = std::min(top, 0.5 / dt);
    if (m_fMax <= 0.0)
        m_fMax = 1.0;

    // БПФ с двукратным дополнением нулями, а в каждый столбик картинки — МАКСИМУМ по
    // попавшим в него частотам.
    //
    // ⚠️ Раньше спектр считался в 256 точках, по одной на столбик. Линия гребёнки
    // шириной 1/TIME (0.05 ГГц при 20 нс) уже шага между точками (0.4 ГГц на полосе
    // до 100 ГГц), и половина тонов проскакивала между ними: десять тонов рисовались
    // шестью пиками. Максимум по столбику узкую линию не теряет никогда.
    const int nfft = nSamples * 2;
    QVector<double> re(nfft, 0.0), im(nfft, 0.0);
    for (int i = 0; i < nSamples; ++i)
        re[i] = m_samples.at(i);
    // Итеративный БПФ по основанию 2.
    for (int i = 1, j = 0; i < nfft; ++i) {
        int bit = nfft >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }
    for (int len = 2; len <= nfft; len <<= 1) {
        const double ang = -2 * kPi / len;
        const double wr = std::cos(ang), wi = std::sin(ang);
        for (int i = 0; i < nfft; i += len) {
            double cr = 1.0, ci = 0.0;
            for (int k = 0; k < len / 2; ++k) {
                const int a = i + k, b = i + k + len / 2;
                const double tr = re[b] * cr - im[b] * ci;
                const double ti = re[b] * ci + im[b] * cr;
                re[b] = re[a] - tr;
                im[b] = im[a] - ti;
                re[a] += tr;
                im[a] += ti;
                const double ncr = cr * wr - ci * wi;
                ci = cr * wi + ci * wr;
                cr = ncr;
            }
        }
    }
    const double df = 1.0 / (nfft * dt); // ГГц на отсчёт БПФ
    m_spec.fill(0.0, kBins);
    for (int k = 1; k < nfft / 2; ++k) {
        const double f = k * df;
        if (f >= m_fMax)
            break;
        const int bin = std::min(kBins - 1, int(f / m_fMax * kBins));
        m_spec[bin] = std::max(m_spec.at(bin), std::hypot(re.at(k), im.at(k)));
    }
    m_specMax = peakOf(m_spec);
}

void SignalPreview::paintTime(QPainter *p, const QRectF &box) const
{
    // Подписям — своя полоса сверху. Без неё кривая перечёркивала текст: у меандра
    // она заполняет всю высоту, у импульса лезет ровно в левый верхний угол, где
    // стоит «a(t), пик». Читать такое нельзя.
    const double head = 14.0;
    p->setPen(dtheme::textMute());
    p->drawText(QRectF(box.left() + 2, box.top(), box.width() - 4, head),
                Qt::AlignLeft | Qt::AlignVCenter,
                tpl::tr("a(t), пик %1", "a(t), peak %1").arg(m_peak, 0, 'g', 3));
    p->drawText(QRectF(box.left() + 2, box.top(), box.width() - 4, head),
                Qt::AlignRight | Qt::AlignVCenter,
                tpl::tr("%1 нс", "%1 ns").arg(m_project.timeMax, 0, 'g', 4));

    const QRectF plot = box.adjusted(0, head, 0, 0);
    const double midY = plot.center().y();
    p->setPen(QPen(dtheme::line(), 1, Qt::DashLine));
    p->drawLine(QPointF(plot.left(), midY), QPointF(plot.right(), midY));

    const TimeWindow win = portWindow(m_port, m_project.timeMax);
    const auto xOf = [&](double t) {
        return plot.left() + plot.width() * t / m_project.timeMax;
    };

    // Границы окна — там сигнал обрывается резко, и это надо видеть.
    if (win.tMin > 0.0 || win.tMax < m_project.timeMax) {
        p->setPen(QPen(dtheme::warn(), 1, Qt::DotLine));
        if (win.tMin > 0.0)
            p->drawLine(QPointF(xOf(win.tMin), plot.top()), QPointF(xOf(win.tMin), plot.bottom()));
        if (win.tMax < m_project.timeMax) {
            p->drawLine(QPointF(xOf(win.tMax), plot.top()), QPointF(xOf(win.tMax), plot.bottom()));
            // «Дальше ноль» — заливкой, чтобы зануление читалось без подписи.
            QColor shade = dtheme::warn();
            shade.setAlpha(18);
            p->fillRect(QRectF(xOf(win.tMax), plot.top(), plot.right() - xOf(win.tMax),
                               plot.height()),
                        shade);
        }
    }

    const int n = m_samples.size();
    const auto yOf = [&](double a) { return midY - (a / m_peak) * (plot.height() / 2 - 1); };
    p->setPen(QPen(dtheme::accent(), 1.2));
    p->setBrush(Qt::NoBrush);
    const int px = std::max(1, int(plot.width()));
    if (n > 2 * px) {
        // Выборок больше, чем пикселей: по столбику — вертикаль от минимума до
        // максимума. Ломаная через все точки дала бы ложный «узор биений» — муар от
        // того, что десятки периодов ложатся в один пиксель.
        for (int x = 0; x < px; ++x) {
            const int i0 = int(qint64(n) * x / px), i1 = std::max(i0 + 1, int(qint64(n) * (x + 1) / px));
            double lo = m_samples.at(i0), hi = lo;
            for (int i = i0; i < std::min(i1, n); ++i) {
                lo = std::min(lo, m_samples.at(i));
                hi = std::max(hi, m_samples.at(i));
            }
            const double xx = plot.left() + x + 0.5;
            p->drawLine(QPointF(xx, yOf(hi)), QPointF(xx, yOf(lo)));
        }
        return;
    }
    QPainterPath path;
    for (int i = 0; i < n; ++i) {
        const QPointF pt(plot.left() + plot.width() * i / (n - 1), yOf(m_samples.at(i)));
        if (i == 0)
            path.moveTo(pt);
        else
            path.lineTo(pt);
    }
    p->drawPath(path);
}

void SignalPreview::paintSpectrum(QPainter *p, const QRectF &box) const
{
    const double head = 14.0;
    p->setPen(dtheme::textMute());
    p->drawText(QRectF(box.left() + 2, box.top(), box.width() - 4, head),
                Qt::AlignLeft | Qt::AlignVCenter, tpl::tr("спектр", "spectrum"));
    p->drawText(QRectF(box.left() + 2, box.top(), box.width() - 4, head),
                Qt::AlignRight | Qt::AlignVCenter,
                tpl::tr("%1 ГГц", "%1 GHz").arg(m_fMax, 0, 'g', 3));

    const QRectF plot = box.adjusted(0, head, 0, 0);
    const auto xOf = [&](double f) { return plot.left() + plot.width() * f / m_fMax; };

    // Одномодовый диапазон апертуры: между отсечками H10 и H20 порт одномодовый, и
    // только там матрица рассеяния означает то, что от неё ждут.
    const double a = m_port.to - m_port.from;
    if (a > 0.0 && m_port.mode == PortMode::H10) {
        const double fc10 = cutoffH10(a, m_port.eps);
        const double fc20 = cutoffH20(a, m_port.eps);
        QColor good = dtheme::ok();
        good.setAlpha(22);
        p->fillRect(QRectF(xOf(std::min(fc10, m_fMax)), plot.top(),
                           xOf(std::min(fc20, m_fMax)) - xOf(std::min(fc10, m_fMax)),
                           plot.height()),
                    good);
        p->setPen(QPen(dtheme::textMute(), 1, Qt::DotLine));
        for (double f : {fc10, fc20}) {
            if (f < m_fMax)
                p->drawLine(QPointF(xOf(f), plot.top()), QPointF(xOf(f), plot.bottom()));
        }
    }

    // FREQ — частота согласования порта.
    const double f0 = m_project.freqs.first();
    if (f0 > 0.0 && f0 < m_fMax) {
        p->setPen(QPen(dtheme::accent(), 1, Qt::DashLine));
        p->drawLine(QPointF(xOf(f0), plot.top()), QPointF(xOf(f0), plot.bottom()));
    }

    QPainterPath path;
    path.moveTo(plot.bottomLeft());
    const int n = m_spec.size();
    for (int k = 0; k < n; ++k) {
        const double x = plot.left() + plot.width() * (k + 0.5) / n;
        const double y = plot.bottom() - (m_spec.at(k) / m_specMax) * (plot.height() - 2);
        path.lineTo(QPointF(x, y));
    }
    path.lineTo(plot.bottomRight());
    path.closeSubpath();
    QColor fill = dtheme::accent();
    fill.setAlpha(60);
    p->setPen(QPen(dtheme::accent(), 1.0));
    p->setBrush(fill);
    p->drawPath(path);
}

void SignalPreview::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF r = rect().adjusted(1, 1, -1, -1);
    p.setPen(QPen(dtheme::line(), 1));
    p.setBrush(dtheme::bg());
    p.drawRoundedRect(r, 10, 10);

    if (!m_port.active || m_samples.isEmpty()) {
        p.setPen(dtheme::textMute());
        p.drawText(r, Qt::AlignCenter,
                   m_port.active ? tpl::tr("сигнала нет", "no signal")
                                 : tpl::tr("порт пассивный — не возбуждается", "passive port — not excited"));
        return;
    }

    const double pad = 8.0;
    const QRectF area = r.adjusted(pad, pad, -pad, -pad);
    if (!m_spectrum || area.height() < 90) {
        paintTime(&p, area);
        return;
    }

    // Времени — две трети, спектру — треть: смотрят чаще на форму.
    const double gap = 6.0;
    const double hTime = (area.height() - gap) * 0.62;
    paintTime(&p, QRectF(area.left(), area.top(), area.width(), hTime));
    paintSpectrum(&p, QRectF(area.left(), area.top() + hTime + gap, area.width(),
                             area.height() - hTime - gap));
}
