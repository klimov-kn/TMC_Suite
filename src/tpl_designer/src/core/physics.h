/// @file physics.h
/// @brief Справочные формулы среды и волновода: плазменная и циклотронная частоты,
///        полосы непрозрачности X-моды, отсечки H10/H20.
///
/// Часть ядра TPL Designer (только заголовок). Используется валидатором, калькулятором
/// среды и подсказками интерфейса. Единицы: размеры — мм, частоты — ГГц, N — м^-3, B — Тл.
/// @see https://www.tamic.ru/docs/theory/plasma.html#plasma-frequency
#pragma once

#include <cmath>

/// Формулы среды и волновода — одни и те же для валидатора и для калькулятора в GUI.
///
/// Живут в ядре, а не в диалоге, потому что валидатор обязан предупреждать теми же
/// числами, которые пользователь видит в калькуляторе. Разъехавшиеся формулы — это
/// «программа сказала одно, а посчиталось другое», то есть ровно та ошибка, ради
/// которой проект и затевался.
///
/// Заголовок сделан header-only намеренно: `src/core/CMakeLists.txt` перечисляет
/// исходники поимённо, и новый .cpp потребовал бы его править.
namespace tpl {

/// Скорость света в мм*ГГц: даёт частоту в ГГц, когда размер задан в мм.
/// ⚠️ Легко ошибиться на 1000x, взяв км/с.
constexpr double kLightSpeed = 299.792458;

/// f_p [Гц] = 8.9788 * sqrt(N), N в м^-3 (СИ).
/// Проверка: N = 3.1e17 -> 5.0 ГГц — ровно та плазма, на которой считаются ЛР2.
constexpr double kPlasmaK = 8.9788;

/// f_ce [Гц] = 2.79925e10 * B, B в Тесла. Проверка: B = 0.071 Тл -> 1.99 ГГц.
constexpr double kCyclotronK = 2.79925e10;

/// Плазменная частота, ГГц, по концентрации в м^-3.
/// @brief Вычисляет плазменную частоту fp = 8.9788 * sqrt(N).
/// @param[in] n  концентрация электронов N, м^-3.
/// @return fp, ГГц; 0 при `n <= 0`.
/// @see https://www.tamic.ru/docs/theory/plasma.html#plasma-frequency
inline double plasmaFreqGHz(double n)
{
    return n > 0.0 ? kPlasmaK * std::sqrt(n) / 1e9 : 0.0;
}

/// Обратный ход: какая концентрация даёт заданную плазменную частоту.
/// @brief Вычисляет концентрацию N = (fp / 8.9788)^2 по плазменной частоте.
/// @param[in] fGHz  плазменная частота fp, ГГц.
/// @return N, м^-3; 0 при `fGHz <= 0`.
inline double concentrationForFreqGHz(double fGHz)
{
    const double fp = fGHz * 1e9;
    return fp > 0.0 ? std::pow(fp / kPlasmaK, 2.0) : 0.0;
}

/// Циклотронная частота, ГГц, по индукции в Тесла.
/// @brief Вычисляет циклотронную частоту fce = 2.79925e10 * |B|.
/// @param[in] b  индукция подмагничивающего поля B, Тл (знак не важен).
/// @return fce, ГГц.
/// @see https://www.tamic.ru/docs/theory/plasma.html#cyclotron
inline double cyclotronFreqGHz(double b)
{
    return kCyclotronK * std::fabs(b) / 1e9;
}

/// Обратный ход: какое поле даёт заданную циклотронную частоту.
/// @brief Вычисляет индукцию B = fce / 2.79925e10 по циклотронной частоте.
/// @param[in] fGHz  циклотронная частота fce, ГГц.
/// @return B, Тл.
inline double fieldForFreqGHz(double fGHz)
{
    return fGHz * 1e9 / kCyclotronK;
}

/// Характерные частоты замагниченной плазмы, ГГц.
///
/// X-мода (E перпендикулярно B) НЕ распространяется в двух полосах:
///   f < f_L                и                f_UH < f < f_R.
/// Без подмагничивания (B = 0) все три частоты вырождаются в f_p, и остаётся
/// единственная отсечка f < f_p — привычный случай изотропной плазмы.
/// @brief Характерные частоты холодной подмагниченной плазмы (все в ГГц).
/// @see https://www.tamic.ru/docs/theory/plasma.html#cutoffs
struct PlasmaBands {
    double fp = 0.0;  ///< плазменная
    double fce = 0.0; ///< циклотронная
    double fL = 0.0;  ///< нижняя отсечка (левая)
    double fR = 0.0;  ///< верхняя отсечка (правая)
    double fUH = 0.0; ///< верхний гибридный резонанс
};

/// @brief Вычисляет характерные частоты плазмы по N и B.
///
/// fL,R = sqrt(fce^2/4 + fp^2) -+ fce/2;  fUH = sqrt(fp^2 + fce^2).
/// @param[in] n  концентрация электронов, м^-3.
/// @param[in] b  индукция поля, Тл.
/// @return Набор частот fp, fce, fL, fR, fUH, ГГц.
/// @see https://www.tamic.ru/docs/theory/plasma.html#cutoffs
inline PlasmaBands plasmaBands(double n, double b)
{
    PlasmaBands r;
    r.fp = plasmaFreqGHz(n);
    r.fce = cyclotronFreqGHz(b);
    const double root = std::sqrt(r.fce * r.fce / 4.0 + r.fp * r.fp);
    r.fL = root - r.fce / 2.0;
    r.fR = root + r.fce / 2.0;
    r.fUH = std::sqrt(r.fp * r.fp + r.fce * r.fce);
    return r;
}

/// Непрозрачна ли плазма для X-моды на этой частоте.
/// @brief Проверяет, попадает ли частота в полосу непрозрачности X-моды (f < fL или fUH < f < fR).
/// @param[in] pb    характерные частоты плазмы (`plasmaBands`).
/// @param[in] fGHz  частота, ГГц.
/// @return true — волна не распространяется; false — прозрачно (в том числе при fp = 0).
inline bool isPlasmaOpaque(const PlasmaBands &pb, double fGHz)
{
    if (pb.fp <= 0.0)
        return false; // плазмы нет — прозрачно всегда
    if (fGHz < pb.fL)
        return true;
    return fGHz > pb.fUH && fGHz < pb.fR;
}

/// Отсечка волны H10 в волноводе шириной a (мм), ГГц. Ниже неё волна не пойдёт.
/// @brief Вычисляет критическую частоту H10: fc = c / (2 * a * sqrt(eps)).
/// @param[in] aMm  ширина волновода (апертура входа), мм.
/// @param[in] eps  относительная проницаемость заполнения (не добавка); `<= 0` заменяется на 1e-6.
/// @return fc10, ГГц.
/// @see https://www.tamic.ru/docs/theory/ports.html#modes
inline double cutoffH10(double aMm, double eps = 1.0)
{
    const double nRefr = std::sqrt(eps > 0.0 ? eps : 1e-6);
    return kLightSpeed / (2.0 * aMm * nRefr);
}

/// Отсечка H20: выше неё вход многомодовый и S-матрица смысла не имеет.
/// @brief Вычисляет критическую частоту H20 (вдвое выше H10).
/// @param[in] aMm  ширина волновода, мм.
/// @param[in] eps  относительная проницаемость заполнения.
/// @return fc20, ГГц.
inline double cutoffH20(double aMm, double eps = 1.0)
{
    return 2.0 * cutoffH10(aMm, eps);
}

} // namespace tpl
