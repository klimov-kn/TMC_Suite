/// @file sigfile.cpp
/// @brief Реализация форм сигнала, плана и текста файла `.sig`, разбора чужих файлов.
///
/// Описание — в sigfile.h.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#sig-file
#include "sigfile.h"
#include "lang.h"

#include "cp1251.h"
#include "emitter.h"
#include "expr.h"

#include <QRegularExpression>
#include <QStringList>

#include <algorithm>
#include <cmath>

namespace tpl {

namespace {

/// Число pi.
constexpr double kPi = 3.14159265358979323846;

/// Свет в мм/нс. Шаг ядра — это `DELTA/(c*sqrt(2))`, и обе величины здесь в
/// «мм и нс», чтобы не ошибиться на девять порядков.
constexpr double kCmmPerNs = 299.792458;

/// Потолок точек в файле. Файл читается один раз при инициализации, так что
/// размер сам по себе не страшен — страшен многогигабайтный текст на диске
/// пользователя. При 400 тысячах точек это около 13 МБ.
constexpr int kMaxSigPoints = 400000;

/// Нормировка моноцикла: max|-2x*exp(-x^2)| = sqrt(2/e) при x = 1/sqrt(2).
const double kMonoNorm = std::sqrt(2.0 / std::exp(1.0));

/// Плавное ВКЛЮЧЕНИЕ посылки — приподнятый косинус на первых 5 % длительности.
///
/// Зачем сглаживать начало: резкий старт несущей размазывает спектр и удлиняет
/// переходный процесс, а тоны в полосе должны стоять там, где заданы.
///
/// 🔑 И почему НЕЛЬЗЯ сглаживать конец. Непрерывный многотон постобработка
/// раскладывает подгонкой гармоник на УСТАНОВИВШЕМСЯ участке, то есть в последних
/// нескольких наносекундах счёта. Спад амплитуды попадает ровно в это окно: подгонка
/// занижает падающую волну, и |S21| выходит больше единицы. Проверено прогоном —
/// сглаживание обоих краёв дало расхождение с эталонным свипом 1.9 дБ вместо 0.0004,
/// причём задание при этом считалось и выглядело исправным. Обрыв в конце безобиден:
/// счёт на нём и кончается.
/// @param[in] u  доля пути внутри посылки (0...1).
/// @return Множитель 0...1: косинусный подъём на u < 0.05, затем 1; вне (0; 1] — 0.
double taper(double u)
{
    constexpr double kEdge = 0.05;
    if (u <= 0.0 || u > 1.0)
        return 0.0;
    if (u < kEdge)
        return 0.5 - 0.5 * std::cos(kPi * u / kEdge);
    return 1.0;
}

/// Доля пути внутри посылки: 0 в начале, 1 в конце. Вне посылки — вне [0;1].
/// @param[in] t         время, нс.
/// @param[in] start     начало посылки, нс.
/// @param[in] duration  длительность посылки, нс.
/// @return (t - start) / duration; -1 при нулевой длительности.
double phaseOf(double t, double start, double duration)
{
    return duration > 0.0 ? (t - start) / duration : -1.0;
}

/// Период повторения, нс, по частоте в ГГц. 1 ГГц — это ровно 1 нс.
/// @param[in] fGhz  частота, ГГц.
/// @return Период 1/f, нс; 0 при f <= 0.
double periodNs(double fGhz)
{
    return fGhz > 0.0 ? 1.0 / fGhz : 0.0;
}

/// Пила, треугольник и меандр: где мы внутри периода, 0...1.
/// @param[in] t      время, нс.
/// @param[in] start  начало сигнала, нс.
/// @param[in] fGhz   частота повторения, ГГц.
/// @return Положение внутри периода 0...1.
double cyclePos(double t, double start, double fGhz)
{
    const double T = periodNs(fGhz);
    if (T <= 0.0)
        return 0.0;
    double u = std::fmod(t - start, T) / T;
    if (u < 0.0)
        u += 1.0;
    return u;
}

/// Центр гауссовой шапки. Пользователь задаёт НАЧАЛО сигнала, а не центр: так у
/// всех форм один смысл у поля «начало», и импульс не уезжает за ноль, где ядро
/// его просто не увидит. Три tau с каждой стороны — exp(-9) = 1.2e-4, ниже
/// разрядности файла.
/// @param[in] w  параметры сигнала (`start`, `width`).
/// @return Момент максимума гауссовой шапки start + 3*tau, нс.
double gaussCenter(const Waveshape &w)
{
    return w.start + 3.0 * std::max(w.width, 1e-9);
}

/// @brief Форматирует число для текста `.sig` (формат 'g').
/// @param[in] v       значение.
/// @param[in] digits  число значащих цифр.
/// @return Строка числа.
QString num(double v, int digits = 6)
{
    QString s = QString::number(v, 'g', digits);
    return s;
}

} // namespace

double kernelTimeStep(double deltaMm)
{
    if (deltaMm <= 0.0)
        return 0.0;
    return deltaMm / (kCmmPerNs * std::sqrt(2.0));
}

WaveFields waveFields(Waveform k)
{
    WaveFields f;
    switch (k) {
    case Waveform::MultiTone:
        f.band = f.tones = f.duration = true;
        break;
    case Waveform::Chirp:
        f.band = f.duration = true;
        break;
    case Waveform::SineBurst:
        f.carrier = f.periods = true;
        break;
    case Waveform::GaussRf:
        f.carrier = f.width = true;
        break;
    case Waveform::GaussVideo:
    case Waveform::Monocycle:
        f.width = true;
        break;
    case Waveform::Trapezoid:
        f.duration = f.edges = true;
        break;
    case Waveform::Sawtooth:
    case Waveform::Triangle:
    case Waveform::Square:
        f.rate = f.duration = true;
        break;
    case Waveform::Expression:
        f.expression = f.duration = true;
        break;
    case Waveform::Table:
        f.table = true;
        break;
    }
    return f;
}

QString waveformName(Waveform k)
{
    switch (k) {
    case Waveform::MultiTone:
        return tpl::tr("Гребёнка тонов", "Tone comb");
    case Waveform::Chirp:
        return tpl::tr("ЛЧМ (чирп)", "LFM (chirp)");
    case Waveform::SineBurst:
        return tpl::tr("Посылка синуса", "Sine burst");
    case Waveform::GaussRf:
        return tpl::tr("Радиоимпульс", "RF pulse");
    case Waveform::GaussVideo:
        return tpl::tr("Видеоимпульс", "Video pulse");
    case Waveform::Monocycle:
        return tpl::tr("Моноцикл", "Monocycle");
    case Waveform::Trapezoid:
        return tpl::tr("Трапеция", "Trapezoid");
    case Waveform::Sawtooth:
        return tpl::tr("Пила", "Sawtooth");
    case Waveform::Triangle:
        return tpl::tr("Треугольник", "Triangle");
    case Waveform::Square:
        return tpl::tr("Меандр", "Square wave");
    case Waveform::Expression:
        return tpl::tr("Своя формула", "Custom formula");
    case Waveform::Table:
        return tpl::tr("Своя таблица", "Custom table");
    }
    return tpl::tr("Сигнал", "Signal");
}

QString waveformHint(Waveform k)
{
    switch (k) {
    case Waveform::MultiTone:
        return tpl::tr("Все частоты полосы сразу, за ОДИН проход вместо свипа. "
                       "Список тонов уходит в файл строкой #FREQ, и постобработка "
                       "берёт их точными.",
                       "All frequencies of the band at once, in ONE run instead of a sweep. The "
                       "tone list goes into the file as the #FREQ line, and post-processing takes "
                       "the tones exactly.");
    case Waveform::Chirp:
        return tpl::tr("Несущая плавно едет от нижней частоты к верхней. Спектр "
                       "гладкий, полосу постобработка находит сама.",
                       "The carrier glides smoothly from the lower frequency to the upper one. "
                       "The spectrum is smooth; post-processing finds the band by itself.");
    case Waveform::SineBurst:
        return tpl::tr("Несколько периодов синуса. Один период — «моноимпульс»: "
                       "самый короткий широкополосный сигнал с несущей.",
                       "Several periods of a sine. One period is a \"monopulse\": the shortest "
                       "wideband signal with a carrier.");
    case Waveform::GaussRf:
        return tpl::tr("Несущая под гауссовой огибающей. Гладкие края, спектр — "
                       "колокол вокруг несущей.",
                       "A carrier under a Gaussian envelope. Smooth edges; the spectrum is a bell "
                       "around the carrier.");
    case Waveform::GaussVideo:
        return tpl::tr("Гауссова шапка без несущей. Спектр от нуля: годится, чтобы "
                       "посмотреть отклик во времени, а не на частоте.",
                       "A Gaussian bump without a carrier. The spectrum starts at zero: suitable "
                       "for looking at the response in time rather than in frequency.");
    case Waveform::Monocycle:
        return tpl::tr("Производная гаусса: нулевое среднее и спектр без постоянной "
                       "составляющей. Классика импульсной рефлектометрии.",
                       "Derivative of a Gaussian: zero mean and a spectrum without a DC "
                       "component. A classic of time-domain reflectometry.");
    case Waveform::Trapezoid:
        return tpl::tr("Видеоимпульс с заданными фронтом и спадом. Чем круче фронт, "
                       "тем шире спектр.",
                       "A video pulse with given rise and fall times. The steeper the rise, the "
                       "wider the spectrum.");
    case Waveform::Sawtooth:
        return tpl::tr("Пила. Все гармоники частоты повторения сразу, амплитуда "
                       "падает как 1/n.",
                       "Sawtooth. All harmonics of the repetition frequency at once; the "
                       "amplitude falls as 1/n.");
    case Waveform::Triangle:
        return tpl::tr("Треугольник. Только нечётные гармоники, спад как 1/n^2 — "
                       "спектр уже, чем у пилы.",
                       "Triangle. Only odd harmonics, falling as 1/n^2: the spectrum is narrower "
                       "than that of the sawtooth.");
    case Waveform::Square:
        return tpl::tr("Меандр. Нечётные гармоники, спад как 1/n; фронты отвесные, "
                       "так что спектр самый широкий из периодических.",
                       "Square wave. Odd harmonics falling as 1/n; the edges are vertical, so the "
                       "spectrum is the widest of the periodic ones.");
    case Waveform::Expression:
        return tpl::tr("Формула a(t) — любая, какую вздумается. Переменная одна: t в "
                       "нс. Считаем её мы, в файл уходят числа.",
                       "Formula a(t): any you like. There is one variable: t in ns. We evaluate "
                       "it, and numbers go into the file.");
    case Waveform::Table:
        return tpl::tr("Готовые точки: две колонки «время, значение». Так вкладывают "
                       "измеренный или посчитанный на стороне сигнал.",
                       "Ready-made points: two columns \"time, value\". This is how a measured "
                       "signal or one computed elsewhere is supplied.");
    }
    return QString();
}

TimeWindow waveSupport(const Waveshape &w)
{
    // Опора: от начала сигнала до конца посылки по виду формы.
    const double t0 = w.start;
    switch (w.kind) {
    case Waveform::MultiTone:
    case Waveform::Chirp:
    case Waveform::Sawtooth:
    case Waveform::Triangle:
    case Waveform::Square:
    case Waveform::Expression:
        return {t0, t0 + std::max(w.duration, 0.0)};
    case Waveform::SineBurst:
        return {t0, t0 + std::max(w.periods, 1) * periodNs(w.f1)};
    case Waveform::GaussRf:
    case Waveform::GaussVideo:
    case Waveform::Monocycle:
        return {t0, t0 + 6.0 * std::max(w.width, 1e-9)};
    case Waveform::Trapezoid:
        return {t0, t0 + std::max(w.rise, 0.0) + std::max(w.duration, 0.0)
                        + std::max(w.fall, 0.0)};
    case Waveform::Table:
        if (w.points.size() >= 2)
            return {w.points.first().x(), w.points.last().x()};
        return {0.0, 0.0};
    }
    return {t0, t0 + std::max(w.duration, 0.0)};
}

Band waveBand(const Waveshape &w)
{
    Band b;
    const double lo = std::min(w.f1, w.f2);
    const double hi = std::max(w.f1, w.f2);
    switch (w.kind) {
    case Waveform::MultiTone:
    case Waveform::Chirp:
        b = {lo, hi, true};
        break;
    case Waveform::SineBurst: {
        // Посылка из N периодов занимает примерно f/N вокруг несущей: спектр
        // прямоугольного окна длиной N периодов имеет как раз такую ширину.
        const double half = w.f1 / std::max(1, w.periods);
        b = {std::max(0.0, w.f1 - half), w.f1 + half, true};
        break;
    }
    case Waveform::GaussRf: {
        // Гаусс exp(-(t/tau)^2) даёт спектр exp(-(pi*f*tau)^2): по уровню -20 дБ
        // это f = 1.52/(pi*tau).
        const double half = 1.52 / (kPi * std::max(w.width, 1e-9));
        b = {std::max(0.0, w.f1 - half), w.f1 + half, true};
        break;
    }
    case Waveform::GaussVideo:
    case Waveform::Monocycle: {
        const double hiF = 1.52 / (kPi * std::max(w.width, 1e-9));
        b = {0.0, hiF, true};
        break;
    }
    case Waveform::Trapezoid: {
        // Спектр трапеции падает после 1/(pi*фронт).
        const double edge = std::max({w.rise, w.fall, 1e-9});
        b = {0.0, 1.0 / (kPi * edge), true};
        break;
    }
    case Waveform::Sawtooth:
    case Waveform::Square:
    case Waveform::Triangle: {
        // Гармоники частоты повторения. Верх берём по десятой гармонике: дальше
        // уровень падает ниже интереса, а у меандра всё равно обрезает сетка.
        b = {w.f1, 10.0 * w.f1, true};
        break;
    }
    case Waveform::Table:
        // Таблица со строкой #FREQ — это многотон, посчитанный на стороне: полоса —
        // от нижнего тона до верхнего. Без неё спектр знает только автор точек.
        if (!w.tableTones.isEmpty()) {
            const auto mm = std::minmax_element(w.tableTones.begin(), w.tableTones.end());
            b = {*mm.first, *mm.second, true};
        }
        break;
    case Waveform::Expression:
        b.known = false; // что там за спектр — знает только автор формулы
        break;
    }
    return b;
}

double suggestedFreq(const Waveshape &w)
{
    const Band b = waveBand(w);
    switch (w.kind) {
    case Waveform::MultiTone:
    case Waveform::Chirp:
        return b.center();
    case Waveform::SineBurst:
    case Waveform::GaussRf:
        return w.f1; // несущая
    case Waveform::Sawtooth:
    case Waveform::Triangle:
    case Waveform::Square:
        return w.f1; // основная гармоника: там и сидит вся энергия
    case Waveform::GaussVideo:
    case Waveform::Monocycle:
    case Waveform::Trapezoid:
        return b.hi / 2.0; // полоса от нуля — согласуем по её середине
    case Waveform::Table:
        // Многотон в таблице согласуют, как гребёнку, — по центру полосы тонов.
        return b.known ? b.center() : 0.0;
    case Waveform::Expression:
        return 0.0; // что там за спектр, знает только автор
    }
    return 0.0;
}

QString plural(int n, const QString &one, const QString &few, const QString &many)
{
    // По-английски форм две: «one» только у единицы, иначе «many» (few не нужна).
    if (english())
        return QStringLiteral("%1 %2").arg(n).arg(std::abs(n) == 1 ? one : many);
    const int a = std::abs(n) % 100;
    const int b = a % 10;
    QString word = many;
    if (a < 11 || a > 14) {
        if (b == 1)
            word = one;
        else if (b >= 2 && b <= 4)
            word = few;
    }
    return QStringLiteral("%1 %2").arg(n).arg(word);
}

QVector<double> waveTones(const Waveshape &w)
{
    QVector<double> out;
    // У таблицы тоны — это то, что назвал принесённый файл. Многотон, посчитанный на
    // стороне, остаётся многотоном и после того, как лёг в таблицу.
    if (w.kind == Waveform::Table)
        return w.tableTones;
    if (w.kind != Waveform::MultiTone)
        return out;
    const int n = std::max(2, w.tones);
    const double lo = std::min(w.f1, w.f2);
    const double hi = std::max(w.f1, w.f2);
    for (int i = 0; i < n; ++i)
        out << lo + (hi - lo) * i / (n - 1);
    return out;
}

ExprProgram waveProgram(const Waveshape &w)
{
    if (w.kind != Waveform::Expression)
        return ExprProgram();
    // Углы — в радианах: формулу считаем МЫ, а не ядро, и ANGLE_UNIT задания к ней
    // отношения не имеет. В файл уходят готовые числа.
    return ExprProgram::compile(w.expression, AngleUnit::Radian);
}

double waveValue(const Waveshape &w, double t, const ExprProgram *prog)
{
    const TimeWindow sup = waveSupport(w);
    if (w.kind != Waveform::Table && (t < sup.tMin || t > sup.tMax))
        return 0.0;

    const double A = w.amplitude;
    const double u = t - w.start;

    switch (w.kind) {
    case Waveform::MultiTone: {
        const QVector<double> tones = waveTones(w);
        double s = 0.0;
        for (double f : tones)
            s += std::sin(2 * kPi * f * u);
        // Каждый тон идёт с ПОЛНОЙ амплитудой, а не с A/N. Так широкополосный
        // проход возбуждает каждую частоту ровно так же, как одночастотный с
        // Amp = A, и числа двух способов сравнимы напрямую (проверено сверкой
        // 0.0016 дБ). Цена — пик суммы доходит до N*A, о чём честно сказано
        // в шапке файла и в диалоге.
        return A * s * taper(phaseOf(t, w.start, w.duration));
    }
    case Waveform::Chirp: {
        const double lo = std::min(w.f1, w.f2);
        const double hi = std::max(w.f1, w.f2);
        const double d = std::max(w.duration, 1e-9);
        const double phase = 2 * kPi * (lo * u + (hi - lo) * u * u / (2.0 * d));
        return A * std::sin(phase) * taper(phaseOf(t, w.start, d));
    }
    case Waveform::SineBurst:
        return A * std::sin(2 * kPi * w.f1 * u);
    case Waveform::GaussRf: {
        const double x = (t - gaussCenter(w)) / std::max(w.width, 1e-9);
        return A * std::exp(-x * x) * std::sin(2 * kPi * w.f1 * (t - gaussCenter(w)));
    }
    case Waveform::GaussVideo: {
        const double x = (t - gaussCenter(w)) / std::max(w.width, 1e-9);
        return A * std::exp(-x * x);
    }
    case Waveform::Monocycle: {
        const double x = (t - gaussCenter(w)) / std::max(w.width, 1e-9);
        return A * (-2.0 * x * std::exp(-x * x)) / kMonoNorm;
    }
    case Waveform::Trapezoid: {
        const double r = std::max(w.rise, 0.0);
        const double p = std::max(w.duration, 0.0);
        const double f = std::max(w.fall, 0.0);
        if (u < r)
            return r > 0.0 ? A * u / r : A;
        if (u < r + p)
            return A;
        if (f <= 0.0)
            return 0.0;
        return A * (1.0 - (u - r - p) / f);
    }
    case Waveform::Sawtooth:
        return A * (2.0 * cyclePos(t, w.start, w.f1) - 1.0);
    case Waveform::Triangle: {
        const double c = cyclePos(t, w.start, w.f1);
        if (c < 0.25)
            return A * 4.0 * c;
        if (c < 0.75)
            return A * (2.0 - 4.0 * c);
        return A * (4.0 * c - 4.0);
    }
    case Waveform::Square:
        return A * (cyclePos(t, w.start, w.f1) < 0.5 ? 1.0 : -1.0);
    case Waveform::Expression: {
        if (!prog || !prog->valid())
            return 0.0;
        Variables v;
        v.insert(QStringLiteral("t"), t);
        const std::optional<double> r = prog->eval(v);
        return r ? A * *r : 0.0;
    }
    case Waveform::Table: {
        // Ровно как ядро: вне таблицы ноль, внутри линейная интерполяция.
        const QVector<QPointF> &pts = w.points;
        if (pts.size() < 2)
            return 0.0;
        if (t < pts.first().x() || t > pts.last().x())
            return 0.0;
        int lo = 0, hi = int(pts.size()) - 1;
        while (hi - lo > 1) {
            const int mid = (lo + hi) / 2;
            if (pts.at(mid).x() > t)
                hi = mid;
            else
                lo = mid;
        }
        const double dt = pts.at(hi).x() - pts.at(lo).x();
        if (dt <= 0.0)
            return pts.at(lo).y();
        return pts.at(lo).y()
               + (pts.at(hi).y() - pts.at(lo).y()) * (t - pts.at(lo).x()) / dt;
    }
    }
    return 0.0;
}

double wavePeak(const Waveshape &w)
{
    const double A = std::fabs(w.amplitude);
    switch (w.kind) {
    case Waveform::Table: {
        double m = 0.0;
        for (const QPointF &p : w.points)
            m = std::max(m, std::fabs(p.y()));
        return m;
    }
    case Waveform::MultiTone:
    case Waveform::Expression: {
        // Пик считаем ВЫБОРКАМИ, а не формулой.
        //
        // У гребёнки аналитической оценки нет: сумма N синусов доходит до N*A только
        // если тоны где-то сошлись в фазе, а при равномерной сетке частот этого может
        // и не случиться. Оценка сверху N*A расходилась с графиком (10 против 9.74),
        // и человек справедливо спрашивал, какому из двух чисел верить. Две тысячи
        // точек по посылке считаются мгновенно и дают то же число, что график.
        const ExprProgram prog = waveProgram(w);
        if (w.kind == Waveform::Expression && !prog.valid())
            return A;
        const TimeWindow sup = waveSupport(w);
        double m = 0.0;
        // Сорок тысяч точек, а не четыре: на гребёнке из десяти тонов четыре тысячи
        // давали 9.72 вместо 9.74 — расхождение с графиком в последней цифре, за
        // которое человек справедливо спросил бы, какому числу верить.
        const int n = 40000;
        for (int i = 0; i <= n; ++i)
            m = std::max(m, std::fabs(waveValue(w, sup.tMin + sup.length() * i / n, &prog)));
        return m > 1e-12 ? m : A;
    }
    default:
        return A;
    }
}

QString waveshapeSummary(const Waveshape &w)
{
    const TimeWindow s = waveSupport(w);
    const QString len = tpl::tr("%1...%2 нс", "%1...%2 ns").arg(num(s.tMin, 4), num(s.tMax, 4));
    switch (w.kind) {
    case Waveform::MultiTone:
        return tpl::tr("%1 %2...%3 ГГц с шагом %4 ГГц, %5",
                       "%1 %2...%3 GHz with a step of %4 GHz, %5")
            .arg(plural(std::max(2, w.tones), tpl::tr("тон", "tone"), tpl::tr("тона", "tones"),
                        tpl::tr("тонов", "tones")),
                 num(std::min(w.f1, w.f2), 4), num(std::max(w.f1, w.f2), 4),
                 num(std::fabs(w.f2 - w.f1) / std::max(1, std::max(2, w.tones) - 1), 3), len);
    case Waveform::Chirp:
        return tpl::tr("несущая от %1 до %2 ГГц за %3 нс, %4",
                       "carrier from %1 to %2 GHz over %3 ns, %4")
            .arg(num(std::min(w.f1, w.f2), 4), num(std::max(w.f1, w.f2), 4),
                 num(w.duration, 4), len);
    case Waveform::SineBurst:
        return tpl::tr("%1 на %2 ГГц, %3", "%1 at %2 GHz, %3")
            .arg(plural(std::max(1, w.periods), tpl::tr("период", "period"),
                        tpl::tr("периода", "periods"), tpl::tr("периодов", "periods")),
                 num(w.f1, 4), len);
    case Waveform::GaussRf:
        return tpl::tr("несущая %1 ГГц, огибающая tau = %2 нс, %3",
                       "carrier %1 GHz, envelope tau = %2 ns, %3")
            .arg(num(w.f1, 4), num(w.width, 4), len);
    case Waveform::GaussVideo:
        return tpl::tr("шапка tau = %1 нс, %2", "bump tau = %1 ns, %2").arg(num(w.width, 4), len);
    case Waveform::Monocycle:
        return tpl::tr("моноцикл tau = %1 нс, %2",
                       "monocycle tau = %1 ns, %2").arg(num(w.width, 4), len);
    case Waveform::Trapezoid:
        return tpl::tr("фронт %1 / полка %2 / спад %3 нс, %4",
                       "rise %1 / flat top %2 / fall %3 ns, %4")
            .arg(num(w.rise, 3), num(w.duration, 3), num(w.fall, 3), len);
    case Waveform::Sawtooth:
    case Waveform::Triangle:
    case Waveform::Square:
        return tpl::tr("частота повторения %1 ГГц (период %2 нс), посылка %3",
                       "repetition frequency %1 GHz (period %2 ns), burst %3")
            .arg(num(w.f1, 4), num(periodNs(w.f1), 4), len);
    case Waveform::Expression:
        return QStringLiteral("a(t) = %1, %2").arg(w.expression, len);
    case Waveform::Table:
        return w.points.size() >= 2
                   ? QStringLiteral("%1, %2")
                         .arg(plural(int(w.points.size()), tpl::tr("точка", "point"),
                                     tpl::tr("точки", "points"), tpl::tr("точек", "points")),
                              len)
                   : tpl::tr("таблица пуста — нужно не меньше двух точек",
                             "the table is empty - at least two points are needed");
    }
    return QString();
}

SigPlan sigPlan(const Project &p, const Port &port)
{
    SigPlan r;
    const Waveshape &w = port.signal.wave;
    r.kernelDt = kernelTimeStep(p.delta);
    // Вдвое мельче шага ядра: проверено прогоном (вдвое крупнее -> 7e-3,
    // вдвое мельче -> 6e-5).
    r.dt = r.kernelDt / 2.0;

    const TimeWindow sup = waveSupport(w);
    r.from = std::max(0.0, sup.tMin);
    r.to = sup.tMax;
    if (r.to > p.timeMax) {
        r.to = p.timeMax;
        r.cutByTime = true;
    }
    if (r.to <= r.from || r.dt <= 0.0) {
        r.points = 0;
        return r;
    }

    r.points = int((r.to - r.from) / r.dt) + 1;
    if (r.points > kMaxSigPoints) {
        r.points = kMaxSigPoints;
        r.dt = (r.to - r.from) / (kMaxSigPoints - 1);
        r.coarsened = true;
    }
    return r;
}

QString signalFileName(const Project &p, const Port &port)
{
    if (!port.signal.file.isEmpty())
        return port.signal.file;
    // По имени задания и номеру порта: имя всегда есть, номер всегда уникален.
    const QString base = p.name.isEmpty() ? QStringLiteral("task") : p.name;
    // `_in<N>` — «вход N». Не `_p<N>`: этим суффиксом эмиттер уже помечает файлы
    // сигналов `.t` разных возбуждений, и одинаковые хвосты у разных вещей путают.
    return QStringLiteral("%1_in%2").arg(base).arg(p.portOrdinal(port.id));
}

QString sigFileText(const Project &p, const Port &port)
{
    const Waveshape &w = port.signal.wave;
    const SigPlan plan = sigPlan(p, port);
    const Band band = waveBand(w);
    const QVector<double> tones = waveTones(w);
    const ExprProgram prog = waveProgram(w);

    QStringList L;
    L << tpl::tr("! Файл падающей волны для порта «%1» задания %2.tpl",
                 "! Incident-wave file for port \"%1\" of task %2.tpl")
             .arg(port.name, p.name);
    L << tpl::tr("! Порождён TPL Designer. Править руками не нужно: форма и её",
                 "! Produced by TPL Designer. No need to edit it by hand: the waveform and its");
    L << tpl::tr("! параметры записаны ниже служебной строкой, и программа читает",
                 "! parameters are recorded below in a service line, and the program reads");
    L << tpl::tr("! файл обратно по ней.", "! the file back from it.");
    L << QStringLiteral("!");
    L << tpl::tr("! Форма: %1 — %2",
                 "! Waveform: %1 -- %2").arg(waveformName(w.kind), waveshapeSummary(w));
    L << tpl::tr("! Амплитуда безразмерна: единиц напряжения в языке задания нет.",
                 "! The amplitude is dimensionless: the task language has no voltage units.");
    if (w.kind == Waveform::MultiTone) {
        L << tpl::tr("! Каждый тон идёт с полной амплитудой %1, поэтому пик суммы",
                     "! Each tone has the full amplitude %1, so the peak of the sum")
                 .arg(formatCompact(w.amplitude));
        L << tpl::tr("! доходит до %1. Так сделано нарочно: возбуждение каждой частоты",
                     "! reaches %1. This is intentional: the excitation of each frequency")
                 .arg(formatCompact(wavePeak(w)));
        L << tpl::tr("! получается таким же, как у одночастотного расчёта с Amp = %1.",
                     "! is the same as in a single-frequency run with Amp = %1.")
                 .arg(formatCompact(w.amplitude));
    }
    L << QStringLiteral("!");
    L << tpl::tr("! Две колонки: время в TIME_UNIT задания (нс) и безразмерное a(t).",
                 "! Two columns: time in the task's TIME_UNIT (ns) and dimensionless a(t).");
    L << tpl::tr("! Строки с «!» и «#» ядро считает комментариями.",
                 "! The kernel treats lines starting with \"!\" and \"#\" as comments.");
    L << tpl::tr("! Шаг записи %1 нс — вдвое мельче шага ядра %2 нс (DELTA/(c*sqrt(2))).",
                 "! Sample step %1 ns -- half the kernel step %2 ns (DELTA/(c*sqrt(2))).")
             .arg(num(plan.dt, 6), num(plan.kernelDt, 6));
    L << tpl::tr("! Между точками ядро интерполирует линейно.",
                 "! Between points the kernel interpolates linearly.");
    if (plan.coarsened) {
        // В файл идут только символы, которые есть в CP1251: ни стрелок, ни знака
        // внимания, ни греческих букв — иначе toCp1251 поднимет флаг lossy и запись
        // задания встанет из-за косметики в комментарии.
        L << tpl::tr("! ВНИМАНИЕ: шаг укрупнён до %1 нс, чтобы файл не распух: "
                     "точек и так %2.",
                     "! WARNING: the step is coarsened to %1 ns so that the file does not swell: "
                     "there are already %2 points.")
                 .arg(num(plan.dt, 6)).arg(plan.points);
    }
    L << QStringLiteral("!");

    // Служебная строка: по ней задание читается обратно без догадок по форме кривой.
    //
    // ⚠️ Семнадцать значащих цифр, а не десять. Числа отсюда возвращаются в модель и
    // участвуют в расчётах: из длительности выводится окно порта, из полосы — частота
    // согласования. Десяти цифр не хватало, и задание, открытое и сохранённое заново,
    // отличалось от исходного последней цифрой в комментарии — то есть круговорот
    // «открыл -> сохранил» переставал быть тождественным. Семнадцать цифр возвращают
    // double точно.
    const auto exact = [](double x) { return QString::number(x, 'g', 17); };
    QStringList m;
    m << QStringLiteral("kind=%1").arg(int(w.kind));
    m << QStringLiteral("amp=%1").arg(exact(w.amplitude));
    m << QStringLiteral("start=%1").arg(exact(w.start));
    m << QStringLiteral("dur=%1").arg(exact(w.duration));
    m << QStringLiteral("f1=%1").arg(exact(w.f1));
    m << QStringLiteral("f2=%1").arg(exact(w.f2));
    m << QStringLiteral("tones=%1").arg(w.tones);
    m << QStringLiteral("periods=%1").arg(w.periods);
    m << QStringLiteral("width=%1").arg(exact(w.width));
    m << QStringLiteral("rise=%1").arg(exact(w.rise));
    m << QStringLiteral("fall=%1").arg(exact(w.fall));
    if (w.kind == Waveform::Expression)
        m << QStringLiteral("expr=%1").arg(w.expression);
    L << QStringLiteral("! %1 %2").arg(QLatin1String(kSigMarker), m.join(QStringLiteral("; ")));
    L << QStringLiteral("!");

    if (!tones.isEmpty()) {
        L << tpl::tr("! Строка #FREQ называет тоны, которые лежат в сигнале (в FREQ_UNIT).",
                     "! The #FREQ line names the tones contained in the signal (in FREQ_UNIT).");
        L << tpl::tr("! Она обязательна для многотона: искать тоны потом по спектру",
                     "! It is mandatory for a multitone: searching for tones later in the "
                     "spectrum");
        L << tpl::tr("! НЕточно — соседние линии смещают пики друг друга.",
                     "! is INACCURATE -- neighbouring lines shift each other's peaks.");
        // Больше 99 тонов пакет не берёт нигде: ни диаграмма направленности, ни
        // постобработка. Ядро отбросило бы лишние молча, поэтому обрезаем сами и
        // говорим об этом прямо в файле — и в проверке задания.
        //
        // ⚠️ Есть и вторая причина. `SetFreqList` копирует строку в буфер на 2048
        // байт и при более длинной НЕ БЕРЁТ НИЧЕГО — список целиком пропадает. 99
        // тонов по десять значащих цифр в него укладываются с запасом (меньше 1300).
        const int n = std::min<int>(tones.size(), kMaxFreqs);
        if (tones.size() > kMaxFreqs) {
            L << tpl::tr("! ВНИМАНИЕ: тонов в сигнале %1, а пакет берёт не больше %2. "
                         "Записаны первые %2,",
                         "! WARNING: the signal has %1 tones, but the package takes at most %2. "
                         "The first %2 are written,")
                     .arg(tones.size()).arg(kMaxFreqs);
            L << tpl::tr("! остальные %1 в сигнале остаются, но не обрабатываются.",
                         "! the remaining %1 stay in the signal but are not processed.")
                     .arg(tones.size() - kMaxFreqs);
        }
        QStringList t;
        for (int i = 0; i < n; ++i)
            t << formatCompact(tones.at(i));
        L << QStringLiteral("#FREQ %1").arg(t.join(QLatin1Char(' ')));
    } else if (band.known) {
        L << tpl::tr("! Спектр гладкий, поэтому строки #FREQ здесь нет: полосу %1...%2 ГГц",
                     "! The spectrum is smooth, so there is no #FREQ line here: post-processing "
                     "finds the band %1...%2 GHz")
                 .arg(num(band.lo, 4), num(band.hi, 4));
        L << tpl::tr("! постобработка находит сама.", "! by itself.");
    }
    L << QStringLiteral("! t[ns]                a");

    for (int i = 0; i < plan.points; ++i) {
        const double t = plan.from + plan.dt * i;
        const double a = waveValue(w, t, &prog);
        L << QString::asprintf("%16.9f  %18.9e", t, a);
    }
    return L.join(QLatin1Char('\n')) + QLatin1Char('\n');
}

bool parseSigMarker(const QString &line, Waveshape *out)
{
    const int at = line.indexOf(QLatin1String(kSigMarker));
    if (at < 0 || !out)
        return false;
    const QString body = line.mid(at + int(qstrlen(kSigMarker)));

    Waveshape w;
    bool sawKind = false;
    // Формула может содержать «;» внутри? Нет: в выражениях ядра точки с запятой не
    // бывает. А вот пробелы бывают, поэтому режем по «;», а не по пробелам.
    for (const QString &part : body.split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
        const int eq = part.indexOf(QLatin1Char('='));
        if (eq < 0)
            continue;
        const QString key = part.left(eq).trimmed();
        const QString val = part.mid(eq + 1).trimmed();
        if (key == QLatin1String("kind")) {
            const int k = val.toInt();
            if (k >= int(Waveform::MultiTone) && k <= int(Waveform::Table)) {
                w.kind = Waveform(k);
                sawKind = true;
            }
        } else if (key == QLatin1String("amp")) {
            w.amplitude = val.toDouble();
        } else if (key == QLatin1String("start")) {
            w.start = val.toDouble();
        } else if (key == QLatin1String("dur")) {
            w.duration = val.toDouble();
        } else if (key == QLatin1String("f1")) {
            w.f1 = val.toDouble();
        } else if (key == QLatin1String("f2")) {
            w.f2 = val.toDouble();
        } else if (key == QLatin1String("tones")) {
            w.tones = val.toInt();
        } else if (key == QLatin1String("periods")) {
            w.periods = val.toInt();
        } else if (key == QLatin1String("width")) {
            w.width = val.toDouble();
        } else if (key == QLatin1String("rise")) {
            w.rise = val.toDouble();
        } else if (key == QLatin1String("fall")) {
            w.fall = val.toDouble();
        } else if (key == QLatin1String("expr")) {
            w.expression = val;
        }
    }
    if (!sawKind)
        return false;
    *out = w;
    return true;
}

QVector<QPointF> parseSigPoints(const QString &text)
{
    QVector<QPointF> pts;
    static const QRegularExpression ws(QStringLiteral("[ \\t,;]+"));
    const QStringList lines = text.split(QRegularExpression(QStringLiteral("[\\r\\n]+")),
                                         Qt::SkipEmptyParts);
    for (const QString &raw : lines) {
        const QString s = raw.trimmed();
        // Ядро отбрасывает строки, начинающиеся с «!» и «#», — делаем то же самое.
        if (s.isEmpty() || s.startsWith(QLatin1Char('!')) || s.startsWith(QLatin1Char('#')))
            continue;
        const QStringList f = s.split(ws, Qt::SkipEmptyParts);
        if (f.size() < 2)
            continue;
        bool okT = false, okV = false;
        const double t = f.at(0).toDouble(&okT);
        const double v = f.at(1).toDouble(&okV);
        if (okT && okV)
            pts << QPointF(t, v);
    }
    return pts;
}

QVector<double> parseSigTones(const QString &text)
{
    QVector<double> out;
    static const QRegularExpression ws(QStringLiteral("[ \\t,;]+"));
    const QStringList lines = text.split(QRegularExpression(QStringLiteral("[\\r\\n]+")),
                                         Qt::SkipEmptyParts);
    for (const QString &raw : lines) {
        // Ядро сравнивает ПЕРВЫЕ пять символов строки (`strncmp(szLine, "#FREQ", 5)`),
        // без пропуска пробелов: строка с отступом для него комментарий.
        if (!raw.startsWith(QLatin1String("#FREQ")))
            continue;
        out.clear(); // ядро берёт последнюю такую строку — каждая затирает прежнюю
        // Ядро читает числа подряд (`sscanf("%lg%n")`) и останавливается на первом
        // нечисле — так же и мы.
        for (const QString &f : raw.mid(5).split(ws, Qt::SkipEmptyParts)) {
            bool ok = false;
            const double v = f.toDouble(&ok);
            if (!ok)
                break;
            out << v;
        }
    }
    return out;
}

QString toneCutNotice(const QVector<double> &tones)
{
    if (tones.size() <= kMaxFreqs)
        return QString();
    return tpl::tr("Строка #FREQ в файле называет %1, а пакет берёт не больше %2 — "
                   "больше не умеют ни диаграмма направленности, ни постобработка.\n\n"
                   "В задание пойдут первые %2 (%3…%4 ГГц), остальные %5 отброшены. "
                   "Сам сигнал не тронут: точки взяты все.",
                   "The #FREQ line in the file names %1, but the package takes at most %2 - "
                   "neither the radiation pattern nor post-processing can handle more.\n\nThe "
                   "first %2 (%3...%4 GHz) will go into the task, the remaining %5 are discarded. "
                   "The signal itself is untouched: all points are taken.")
        .arg(plural(int(tones.size()), tpl::tr("тон", "tone"), tpl::tr("тона", "tones"),
                    tpl::tr("тонов", "tones")))
        .arg(kMaxFreqs)
        .arg(formatCompact(tones.first()), formatCompact(tones.at(kMaxFreqs - 1)))
        .arg(tones.size() - kMaxFreqs);
}

bool portUsesSignalFile(const Project &p, const Port &port)
{
    if (port.signal.source != SignalSource::File)
        return false;
    return port.active || p.exciters.contains(port.id);
}

bool hasFileSignal(const Project &p)
{
    for (const Port &port : p.ports) {
        if (portUsesSignalFile(p, port))
            return true;
    }
    return false;
}

Band projectBand(const Project &p)
{
    Band all;
    for (const Port &port : p.ports) {
        if (!portUsesSignalFile(p, port))
            continue;
        const Band b = waveBand(port.signal.wave);
        if (!b.known)
            continue;
        if (!all.known) {
            all = b;
        } else {
            all.lo = std::min(all.lo, b.lo);
            all.hi = std::max(all.hi, b.hi);
        }
    }
    return all;
}

QMap<QString, QByteArray> signalCompanions(const Project &p, bool *lossy)
{
    QMap<QString, QByteArray> out;
    for (const Port &port : p.ports) {
        if (!portUsesSignalFile(p, port))
            continue;
        bool l = false;
        const QString text = sigFileText(p, port);
        // Переводы строк — CRLF на любой системе, как и у самого задания: файл должен
        // получаться одинаковым на Windows, macOS и Linux.
        QString crlf = text;
        crlf.replace(QLatin1Char('\n'), QLatin1String("\r\n"));
        out.insert(signalFileName(p, port) + QStringLiteral(".sig"), toCp1251(crlf, &l));
        if (lossy)
            *lossy |= l;
    }
    return out;
}

} // namespace tpl
