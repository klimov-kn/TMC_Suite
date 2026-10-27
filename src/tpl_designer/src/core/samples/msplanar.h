/// @file msplanar.h
/// @brief Общие детали планарной волноводной модели для рецептов раздела «Микрополосковые».
///
/// Часть модуля «Ядро TPL Designer», каталог образцов. Подключается из `microstrip.cpp`
/// и `microstrip2.cpp`; все функции встроенные (`inline`).
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#samples
#pragma once

#include "kit.h"
#include "lang.h"
#include "rf.h"
#include "sample.h"
#include "standards.h"

#include <QPointF>
#include <QString>
#include <QVector>

#include <algorithm>
#include <cmath>

/// Детали планарной волноводной модели — общие для всех рецептов раздела
/// «Микрополосковые».
///
/// Микрополоска — трёхмерная структура, а ядро двумерное. Классический выход —
/// планарная волноводная модель (Весселя-Кольхоффа): полоска над экраном заменяется
/// плоским волноводом ширины W_eff с магнитными боковыми стенками, заполненным средой
/// с eps_eff. Поле Ez в нём однородно по ширине — это и есть основная волна линии,
/// а H-мода ядра (поле Ez, Hx, Hy в плоскости) решает ровно такую задачу.
///
/// Проверено прогоном 2026-09-23: вход плоской волны (MAGNETIC) между магнитными
/// стенками работает, но у самого входа ядра есть остаточное отражение около -25 дБ,
/// не зависящее от шага сетки. Уровни ниже этого в результате не различить — в
/// паспортах так и написано. Лучше всего вход сидит, когда стенки проходят ТОЧНО по
/// краям апертуры: так и строятся все образцы раздела.
///
/// ⚠️ Чего этой моделью выразить НЕЛЬЗЯ, и потому в разделе нет: связанных линий.
/// Боковые стенки канала магнитные, то есть поле за них не выходит вовсе — а связь
/// двух полосок живёт именно в краевом поле. Направленный ответвитель на связанных
/// линиях и фильтр на связанных резонаторах в двумерной постановке не получаются;
/// их место занимают шлейфовые схемы, где связь идёт по самому каналу.
namespace tpl::samples::msp {

using namespace kit;

/// Число pi.
constexpr double kPi = rf::kPi;

/// Микрополосковая линия заданного волнового сопротивления на подложке.
struct Line {
    /// Волновое сопротивление, Ом.
    double z0 = 50.0;
    double w = 0.0;      ///< ширина полоски, мм
    /// Эффективная проницаемость (квазистатика); ею заполняется канал модели.
    double epsEff = 1.0;
    double weff = 0.0;   ///< ширина планарной модели, мм

    /// @brief Возвращает длину волны в линии: lambda0/sqrt(eps_eff).
    /// @param[in] fGHz  частота, ГГц
    /// @return Длина волны, мм.
    double lambdaG(double fGHz) const { return rf::lambda0(fGHz) / std::sqrt(epsEff); }
};

/// @brief Плата: подложка из таблицы и фактическая толщина.
struct Board {
    /// Подложка (eps и справочная толщина).
    Substrate sub;
    /// Толщина подложки, мм.
    double h = 1.0;

    /// @brief Рассчитывает линию заданного сопротивления на этой плате.
    /// @param[in] z0  волновое сопротивление, Ом
    /// @return Ширина полоски, eps_eff и ширина канала модели W_eff.
    Line line(double z0) const
    {
        Line l;
        l.z0 = z0;
        const double u = rf::msWidthRatio(z0, sub.eps);
        l.w = u * h;
        l.epsEff = rf::msEpsEff(u, sub.eps);
        l.weff = rf::msPlanarWidth(z0, l.epsEff, h);
        return l;
    }
};

/// @brief Собирает плату по параметрам карточки `sub` и `h`.
/// @param[in] v  значения карточки: `sub` — номер подложки, `h` — толщина, мм (0 — из таблицы)
/// @return Плата.
inline Board board(const Values &v)
{
    Board b;
    const QVector<Substrate> &t = substrates();
    b.sub = t.value(std::clamp(int(v.value(QStringLiteral("sub"))), 0, int(t.size()) - 1));
    const double h = v.value(QStringLiteral("h"));
    b.h = h > 0.0 ? h : b.sub.h;
    return b;
}

/// @brief Возвращает параметры карточки для выбора платы: подложка и толщина.
/// @return Два параметра: `sub` (выбор) и `h` (мм, 0 — по таблице).
inline QVector<Param> boardParams()
{
    return {Param::choice(QStringLiteral("sub"), tpl::tr("Подложка", "Substrate"), substrateChoices(), 0),
            Param::number(QStringLiteral("h"), tpl::tr("Толщина h (0 — по таблице)",
                                                       "Thickness h (0 = from table)"),
                          tpl::tr("мм", "mm"), 0.0, 0.0, 5.0, 0.1, 3)};
}

/// @brief Возвращает строку паспорта о подложке.
/// @param[in] b  плата
/// @return Одна строка «Подложка: имя, eps, h».
inline QVector<Row> boardRows(const Board &b)
{
    return {{tpl::tr("Подложка", "Substrate"),
             tpl::tr("%1: eps = %2, h = %3 мм",
                     "%1: eps = %2, h = %3 mm").arg(b.sub.name, num(b.sub.eps), num(b.h))}};
}

/// @brief Возвращает строку паспорта о линии: w, eps_eff, W_eff, длина волны в линии.
/// @param[in] what  подпись строки
/// @param[in] l     линия
/// @param[in] f0    частота, на которой дана длина волны, ГГц
/// @return Строка паспорта.
inline Row lineRow(const QString &what, const Line &l, double f0)
{
    return {what, tpl::tr("w = %1 мм, eps_eff = %2, W_eff = %3 мм, λв = %4 мм на %5 ГГц",
                          "w = %1 mm, eps_eff = %2, W_eff = %3 mm, lambda_g = %4 mm at %5 GHz")
                      .arg(num(l.w), num(l.epsEff), num(l.weff), num(l.lambdaG(f0), 4), num(f0, 3))};
}

/// Канал планарной модели: заливка eps_eff и магнитные стенки строятся отдельно.
/// @param[in,out] b     сборщик задания
/// @param[in]     name  имя фигуры
/// @param[in]     l     линия (берётся eps_eff)
/// @param[in]     x0    одна граница по X, мм
/// @param[in]     x1    другая граница по X, мм
/// @param[in]     y0    одна граница по Y, мм
/// @param[in]     y1    другая граница по Y, мм
inline void fill(Builder &b, const QString &name, const Line &l, double x0, double x1, double y0,
                 double y1)
{
    b.dielectric(name, l.epsEff, x0, x1, y0, y1);
}

/// @brief Добавляет вход плоской волны (MAGNETIC) с проницаемостью сечения eps_eff.
///
/// `Port::epsAuto` выключается: eps_eff — величина модели линии, её нельзя заменять
/// проницаемостью фигуры под входом.
/// @param[in,out] b       сборщик задания
/// @param[in]     name    имя входа
/// @param[in]     side    сторона коробки
/// @param[in]     from    начало апертуры, мм
/// @param[in]     to      конец апертуры, мм
/// @param[in]     active  возбуждается ли вход
/// @param[in]     l       линия (берётся eps_eff)
/// @return Ссылка на вход (действительна до следующего добавления).
/// @see https://www.tamic.ru/docs/theory/ports.html#port-w
inline Port &msPort(Builder &b, const QString &name, PortSide side, double from, double to,
                    bool active, const Line &l)
{
    Port &p = b.port(name, side, from, to, active, PortMode::Plane);
    // Среда в сечении входа — та же eps_eff: иначе вход рассогласован с линией.
    p.eps = l.epsEff;
    p.epsAuto = false; // eps_eff — величина модели линии, а не фигуры под входом
    return p;
}

/// @brief Выбирает шаг сетки: не меньше 20 шагов на длину волны в линии и 10 на W_eff.
/// @param[in] narrowest  самая узкая линия схемы
/// @param[in] f0         частота, ГГц
/// @return Шаг сетки, мм.
inline double gridFor(const Line &narrowest, double f0)
{
    return chooseDelta(rf::lambda0(f0) / std::sqrt(narrowest.epsEff), narrowest.weff, 20.0, 10.0);
}

/// Стенка-отрезок тремя точками, а не двумя.
///
/// `POLYGON_STAT` не грузится меньше чем с тремя строками `L` — середину всё равно
/// добивал бы эмиттер, так что ставим её сами и видим в задании ровно то, что писали.
/// @param[in] a  начало отрезка, мм
/// @param[in] b  конец отрезка, мм
/// @return Три точки: начало, середина, конец.
inline QVector<QPointF> seg(const QPointF &a, const QPointF &b)
{
    return {a, (a + b) / 2.0, b};
}

/// @brief Возвращает общий абзац справки о планарной модели (дописывается к справке образца).
/// @return Текст на текущем языке, начинается с пустой строки.
inline QString helpPlanar()
{
    return tpl::tr(
        "\n\nПланарная модель: полоска заменена плоским волноводом ширины W_eff с магнитными "
        "стенками и заполнением eps_eff (квазистатика Хаммерстада). Так микрополоску считают "
        "в двумерной постановке; дисперсию eps_eff модель не учитывает. У входа плоской волны "
        "в ядре есть остаточное отражение около -25 дБ — уровни ниже не различить.",
        "\n\nPlanar model: the strip is replaced by a parallel-plate waveguide of width W_eff with "
        "magnetic walls and eps_eff filling (Hammerstad quasi-statics). This is how a microstrip is "
        "computed in a 2D formulation; the model ignores the dispersion of eps_eff. The plane-wave "
        "port of the kernel has a residual reflection of about -25 dB, so lower levels cannot be "
        "resolved.");
}

/// Поправка длин на неоднородности, %.
///
/// В планарной модели полоски широкие: у 50-омной линии на FR-4 W_eff около 6 мм при
/// λв/4 около 14 мм. Соединения таких полосок электрически «короче», чем их рисуют по
/// осям, и учебниковый расчёт уводит центр полосы. Поправка подобрана прогоном ядра для
/// умолчаний; при 0 — расчёт по осям, как в учебнике.
/// @param[in] def  значение по умолчанию, %
/// @return Параметр `corr` (от -40 до 40 %).
inline Param corrParam(double def)
{
    return Param::number(QStringLiteral("corr"), tpl::tr("Поправка на соединения", "Junction correction"),
                         QStringLiteral("%"), def, -40.0, 40.0, 1.0, 0,
                         tpl::tr("Масштаб длин отрезков. Подобран прогоном ядра для "
                                 "умолчаний; 0 — расчёт по осям, как в учебнике",
                                 "Scale of segment lengths. Tuned by a kernel run for the defaults; "
                                 "0 = textbook calculation along the axes"));
}

/// @brief Возвращает параметр рабочей частоты `f0` (0.3...30 ГГц).
/// @param[in] def  значение по умолчанию, ГГц
/// @return Параметр карточки.
inline Param freqParam(double def)
{
    return Param::number(QStringLiteral("f0"), tpl::tr("Частота f0", "Frequency f0"), tpl::tr("ГГц", "GHz"),
                         def, 0.3, 30.0, 0.1, 2);
}

/// @brief Возвращает параметр волнового сопротивления `z0` (20...120 Ом).
/// @param[in] label  подпись поля
/// @param[in] def    значение по умолчанию, Ом
/// @return Параметр карточки.
inline Param z0Param(const QString &label, double def)
{
    return Param::number(QStringLiteral("z0"), label, tpl::tr("Ом", "Ohm"), def, 20.0, 120.0, 5.0,
                         1);
}

} // namespace tpl::samples::msp
