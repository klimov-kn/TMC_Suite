/// @file rf.cpp
/// @brief Реализация формул паспортов образцов (волноводы, фильтры, микрополоска, ЭПР).
///
/// Часть модуля «Ядро TPL Designer», каталог образцов. Описание открытых функций —
/// в `rf.h`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#samples
#include "rf.h"

#include "physics.h"

#include <algorithm>
#include <cmath>

namespace tpl::samples::rf {

double lambda0(double fGHz)
{
    return kLightSpeed / fGHz;
}

double lambdaGuide(double aMm, double fGHz, double eps)
{
    const double k2 = eps * std::pow(2 * kPi / lambda0(fGHz), 2) - std::pow(kPi / aMm, 2);
    return k2 > 0.0 ? 2 * kPi / std::sqrt(k2) : 0.0;
}

double irisSusceptance(double aMm, double dMm, double fGHz)
{
    const double lg = lambdaGuide(aMm, fGHz);
    const double t = std::tan(kPi * dMm / (2.0 * aMm));
    return -(lg / aMm) / (t * t);
}

double irisWindowForReactance(double aMm, double x, double fGHz)
{
    // X/Z0 = (a/lg) * tan^2(pi d / 2a)  =>  d = (2a/pi) * atan(sqrt(X * lg / a))
    const double lg = lambdaGuide(aMm, fGHz);
    return 2.0 * aMm / kPi * std::atan(std::sqrt(std::max(0.0, x) * lg / aMm));
}

double postReactance(double aMm, double dMm, double fGHz)
{
    // Маркувиц, «Waveguide Handbook», п. 5.11: тонкий штырь в центре широкой стенки.
    //   X/Z0 = (a / 2 lg) * [ ln(2a / (pi d)) - 2 + 2 * S ],
    //   S = sum_{n=3,5,...} ( 1/sqrt(n^2 - (2a/l)^2) - 1/n )
    const double lg = lambdaGuide(aMm, fGHz);
    const double r = 2.0 * aMm / lambda0(fGHz);
    double s = 0.0;
    for (int n = 3; n < 4001; n += 2)
        s += 1.0 / std::sqrt(double(n) * n - r * r) - 1.0 / n;
    return aMm / (2.0 * lg) * (std::log(2.0 * aMm / (kPi * dMm)) - 2.0 + 2.0 * s);
}

double shuntReflection(double b)
{
    // Шунт jB в линии с единичной проводимостью: Г = -jB / (2 + jB).
    return std::abs(b) / std::sqrt(4.0 + b * b);
}

namespace {

/// Продольное волновое число H10 с ветвью, дающей затухание: exp(-j kz z) убывает.
/// @param[in] aMm   ширина волновода, мм; <= 0 — свободное пространство (kc = 0)
/// @param[in] eps   комплексная проницаемость среды
/// @param[in] fGHz  частота, ГГц
/// @return kz, рад/мм (мнимая часть <= 0).
cplx kz(double aMm, cplx eps, double fGHz)
{
    const double k0 = 2 * kPi / lambda0(fGHz);
    const double kc = aMm > 0.0 ? kPi / aMm : 0.0;
    cplx q = std::sqrt(eps * k0 * k0 - kc * kc);
    if (q.imag() > 0.0)
        q = -q;
    return q;
}

} // namespace

cplx slabReflection(double aMm, cplx eps, double dMm, double fGHz)
{
    // Волна H10 — поперечно-электрическая: её волновое сопротивление обратно kz.
    // Слой — отрезок линии с другим kz между двумя одинаковыми полупространствами.
    const cplx k1 = kz(aMm, 1.0, fGHz);
    const cplx k2 = kz(aMm, eps, fGHz);
    const cplx r = (k1 - k2) / (k1 + k2);
    const cplx p2 = std::exp(cplx(0.0, -2.0) * k2 * dMm);
    return r * (1.0 - p2) / (1.0 - r * r * p2);
}

QVector<double> butterworth(int n)
{
    QVector<double> g(n + 2, 1.0);
    for (int k = 1; k <= n; ++k)
        g[k] = 2.0 * std::sin((2 * k - 1) * kPi / (2.0 * n));
    return g;
}

QVector<double> chebyshev(int n, double rippleDb)
{
    // Маттей, Янг, Джонс: g-параметры прототипа Чебышёва.
    QVector<double> g(n + 2, 1.0);
    const double beta = std::log(1.0 / std::tanh(rippleDb / 17.37));
    const double gam = std::sinh(beta / (2.0 * n));
    QVector<double> a(n + 1), b(n + 1);
    for (int k = 1; k <= n; ++k) {
        a[k] = std::sin((2 * k - 1) * kPi / (2.0 * n));
        b[k] = gam * gam + std::pow(std::sin(k * kPi / n), 2);
    }
    g[1] = 2.0 * a[1] / gam;
    for (int k = 2; k <= n; ++k)
        g[k] = 4.0 * a[k - 1] * a[k] / (b[k - 1] * g[k - 1]);
    g[n + 1] = (n % 2 == 1) ? 1.0 : std::pow(1.0 / std::tanh(beta / 4.0), 2);
    return g;
}

double msEpsEff(double u, double er)
{
    double e = (er + 1.0) / 2.0 + (er - 1.0) / 2.0 / std::sqrt(1.0 + 12.0 / u);
    if (u < 1.0)
        e += (er - 1.0) / 2.0 * 0.04 * (1.0 - u) * (1.0 - u);
    return e;
}

double msZ0(double u, double er)
{
    const double ee = msEpsEff(u, er);
    if (u <= 1.0)
        return 60.0 / std::sqrt(ee) * std::log(8.0 / u + u / 4.0);
    return 120.0 * kPi / (std::sqrt(ee) * (u + 1.393 + 0.667 * std::log(u + 1.444)));
}

double msWidthRatio(double z0, double er)
{
    // Синтез Хаммерстада, затем уточнение по анализу (формулы синтеза точны до ~1%).
    const double A = z0 / 60.0 * std::sqrt((er + 1.0) / 2.0)
                     + (er - 1.0) / (er + 1.0) * (0.23 + 0.11 / er);
    const double B = 377.0 * kPi / (2.0 * z0 * std::sqrt(er));
    double u = 8.0 * std::exp(A) / (std::exp(2.0 * A) - 2.0);
    if (u > 2.0) {
        u = 2.0 / kPi
            * (B - 1.0 - std::log(2.0 * B - 1.0)
               + (er - 1.0) / (2.0 * er) * (std::log(B - 1.0) + 0.39 - 0.61 / er));
    }
    // Несколько шагов секущих по msZ0 — чтобы синтез и анализ говорили одно.
    for (int it = 0; it < 30; ++it) {
        const double z = msZ0(u, er);
        const double z2 = msZ0(u * 1.001, er);
        const double dz = (z2 - z) / (u * 0.001);
        if (std::fabs(dz) < 1e-12)
            break;
        const double nu = u - (z - z0) / dz;
        u = std::clamp(nu, u / 4.0, u * 4.0);
        if (std::fabs(z - z0) < 1e-9)
            break;
    }
    return u;
}

double msPlanarWidth(double z0, double epsEff, double hMm)
{
    return kEta0 * hMm / (z0 * std::sqrt(epsEff));
}

// ------------------------------------------------------------------ Бессель

namespace {

/// Интеграл Симпсона.
/// @tparam F  вызываемый объект `double(double)`
/// @param[in] f  подынтегральная функция
/// @param[in] a  нижний предел
/// @param[in] b  верхний предел
/// @param[in] n  число отрезков (нечётное увеличивается на 1)
/// @return Приближённое значение интеграла.
template <typename F>
double simpson(F f, double a, double b, int n)
{
    if (n % 2)
        ++n;
    const double h = (b - a) / n;
    double s = f(a) + f(b);
    for (int i = 1; i < n; ++i)
        s += f(a + i * h) * ((i % 2) ? 4.0 : 2.0);
    return s * h / 3.0;
}

} // namespace

double besselJ(int n, double x)
{
    // Интеграл Бесселя: J_n(x) = (1/pi) * int_0^pi cos(n t - x sin t) dt. Для целых n
    // подынтегральная функция гладкая и периодическая — Симпсон сходится быстро.
    const int steps = 400 + 20 * int(std::fabs(x)) + 20 * std::abs(n);
    return simpson([&](double t) { return std::cos(n * t - x * std::sin(t)); }, 0.0, kPi, steps)
           / kPi;
}

double besselY(int n, double x)
{
    // Y_n(x) = (1/pi) int_0^pi sin(x sin t - n t) dt
    //        - (1/pi) int_0^inf (e^{n t} + (-1)^n e^{-n t}) e^{-x sinh t} dt
    const int steps = 400 + 20 * int(std::fabs(x)) + 20 * std::abs(n);
    const double a = simpson([&](double t) { return std::sin(x * std::sin(t) - n * t); }, 0.0,
                             kPi, steps);
    // Верхний предел: там, где x sinh t - n t > 60, вклад ничтожен.
    double tMax = 1.0;
    while (x * std::sinh(tMax) - n * tMax < 60.0 && tMax < 50.0)
        tMax += 0.25;
    const double sign = (n % 2) ? -1.0 : 1.0;
    const double b = simpson(
        [&](double t) {
            return (std::exp(n * t) + sign * std::exp(-n * t)) * std::exp(-x * std::sinh(t));
        },
        0.0, tMax, 4000 + 200 * std::abs(n));
    return (a - b) / kPi;
}

double cylinderEchoWidth(double aMm, double fGHz, bool tm)
{
    // sigma = (4/k) |sum_n (-1)^n eps_n c_n|^2, c_n = J_n/H2_n (TM) или J'_n/H2'_n (TE).
    const double k = 2 * kPi / lambda0(fGHz);
    const double x = k * aMm;
    const int nMax = int(x) + 12;
    const auto J = [&](int n) { return besselJ(n, x); };
    const auto Y = [&](int n) { return besselY(n, x); };
    cplx sum = 0.0;
    for (int n = 0; n <= nMax; ++n) {
        cplx c;
        if (tm) {
            c = J(n) / cplx(J(n), -Y(n));
        } else {
            // Производные через соседей: Z'_n = (Z_{n-1} - Z_{n+1}) / 2, Z_{-1} = -Z_1.
            const double jm = n == 0 ? -J(1) : J(n - 1);
            const double ym = n == 0 ? -Y(1) : Y(n - 1);
            const double jd = (jm - J(n + 1)) / 2.0;
            const double yd = (ym - Y(n + 1)) / 2.0;
            c = jd / cplx(jd, -yd);
        }
        const double en = n == 0 ? 1.0 : 2.0;
        sum += ((n % 2) ? -1.0 : 1.0) * en * c;
    }
    return 4.0 / k * std::norm(sum);
}

double stripEchoWidth(double wMm, double fGHz, double phiDeg)
{
    const double k = 2 * kPi / lambda0(fGHz);
    const double phi = phiDeg * kPi / 180.0;
    const double u = k * wMm * std::cos(phi);
    const double sinc = std::fabs(u) < 1e-12 ? 1.0 : std::sin(u) / u;
    const double s = std::sin(phi);
    return k * wMm * wMm * s * s * sinc * sinc;
}

double dB(double mag)
{
    return 20.0 * std::log10(std::max(mag, 1e-12));
}

} // namespace tpl::samples::rf
