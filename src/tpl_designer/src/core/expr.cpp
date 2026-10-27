/// @file expr.cpp
/// @brief Реализация вычислителя выражений `.tpl`: рекурсивный спуск в дерево и обход дерева.
///
/// Описание интерфейса — в expr.h.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#expr
#include "expr.h"

#include <QStringList>
#include <QVarLengthArray>

#include <cmath>

namespace tpl {
namespace {

/// Число pi.
constexpr double kPi = 3.14159265358979323846;

/// Пробелы ядро сносит ИЗ ВЫРАЖЕНИЯ целиком (`expr_del_Blanks2` в
/// TmcRTH_BolckList.cpp зовётся для каждого параметра), поэтому в авторских
/// заданиях законно встречается `( + 500 . 0 )` и `6 . 3` — это числа 500.0 и 6.3.
/// @brief Удаляет из текста все пробельные символы.
/// @param[in] text  текст выражения.
/// @return Текст без пробелов.
QString stripSpaces(const QString &text)
{
    QString clean;
    clean.reserve(text.size());
    for (QChar c : text) {
        if (!c.isSpace())
            clean += c;
    }
    return clean;
}

} // namespace

// Разовые вызовы идут через ту же `ExprProgram`, что и горячие циклы. Второго
// вычислителя в проекте быть не должно: разойдись они хоть в одной мелочи — холст
// показывал бы одно, а проверка перед записью говорила бы другое.
std::optional<double> evalExpression(const QString &text, AngleUnit angle)
{
    return ExprProgram::compile(text, angle).eval(Variables{});
}

std::optional<double> evalExpression(const QString &text, const Variables &vars, AngleUnit angle)
{
    return ExprProgram::compile(text, angle).eval(vars);
}

Variables epsVariables(double x, double y, AngleUnit angle)
{
    // Ядро считает угол через atan2(y, x) и делит на ANGLE_UNIT — то есть отдаёт
    // выражению угол в тех же единицах, в каких пользователь пишет секторы окружностей.
    const double k = (angle == AngleUnit::Gradus) ? 180.0 / kPi : 1.0;
    return Variables{{QStringLiteral("x"), x},
                     {QStringLiteral("y"), y},
                     {QStringLiteral("r"), std::hypot(x, y)},
                     {QStringLiteral("f"), std::atan2(y, x) * k}};
}

QStringList unknownNames(const QString &text, const Variables &known)
{
    return ExprProgram::compile(text).unknown(known);
}

bool isMaterialKeyword(const QString &text)
{
    const QString t = text.trimmed();
    return t == QLatin1String("METAL") || t == QLatin1String("MAGNETIC")
           || t == QLatin1String("ABSORBER");
}

QString scaleCoordinates(const QString &text, double factor)
{
    if (text.isEmpty() || factor == 1.0)
        return text;

    const QString mul = QString::number(factor, 'g', 17);

    QString out;
    out.reserve(text.size() + 16);
    int i = 0;
    const int n = text.size();
    while (i < n) {
        const QChar c = text.at(i);

        // Число целиком, вместе с показателем степени: буква 'e' внутри `3.1e+17`
        // именем не является.
        if (c.isDigit() || (c == QLatin1Char('.') && i + 1 < n && text.at(i + 1).isDigit())) {
            const int start = i;
            while (i < n && (text.at(i).isDigit() || text.at(i) == QLatin1Char('.')))
                ++i;
            if (i < n && (text.at(i) == QLatin1Char('e') || text.at(i) == QLatin1Char('E'))) {
                int j = i + 1;
                if (j < n && (text.at(j) == QLatin1Char('+') || text.at(j) == QLatin1Char('-')))
                    ++j;
                if (j < n && text.at(j).isDigit()) {
                    i = j;
                    while (i < n && text.at(i).isDigit())
                        ++i;
                }
            }
            out += text.mid(start, i - start);
            continue;
        }

        // Имя целиком: `exp` — одно имя, а не `e`, `x`, `p`.
        if (c.isLetter() || c == QLatin1Char('_')) {
            const int start = i;
            while (i < n && (text.at(i).isLetterOrNumber() || text.at(i) == QLatin1Char('_')))
                ++i;
            const QString name = text.mid(start, i - start);
            if (name == QLatin1String("x") || name == QLatin1String("y")
                || name == QLatin1String("r")) {
                out += QStringLiteral("(%1*%2)").arg(name, mul);
            } else {
                out += name;
            }
            continue;
        }

        out += c;
        ++i;
    }
    return out;
}

// ---------------------------------------------------------------------------
//  Скомпилированное выражение
// ---------------------------------------------------------------------------

namespace {

/// Коды операций — ровно то, что понимает интерпретатор ядра (tpl-guide 9.1).
enum Op {
    OpConst, OpVar, OpNeg, OpAdd, OpSub, OpMul, OpDiv, OpPow, OpFunc
};

/// Номера функций. Порядок произвольный, важно лишь совпадение с разбором в `run`.
enum Fn {
    FnAbs, FnExp, FnLog, FnLog10, FnSqrt, FnSin, FnCos, FnTan,
    FnSinh, FnCosh, FnTanh, FnAsin, FnAcos, FnAtan, FnNone
};

/// @brief Возвращает номер встроенной функции по имени.
/// @param[in] name  имя функции (регистр значим).
/// @return Код `Fn...` или `FnNone`, если функции нет.
int functionCode(const QString &name)
{
    if (name == QLatin1String("abs"))   return FnAbs;
    if (name == QLatin1String("exp"))   return FnExp;
    if (name == QLatin1String("log"))   return FnLog;
    if (name == QLatin1String("log10")) return FnLog10;
    if (name == QLatin1String("sqrt"))  return FnSqrt;
    if (name == QLatin1String("sin"))   return FnSin;
    if (name == QLatin1String("cos"))   return FnCos;
    if (name == QLatin1String("tan"))   return FnTan;
    if (name == QLatin1String("sinh"))  return FnSinh;
    if (name == QLatin1String("cosh"))  return FnCosh;
    if (name == QLatin1String("tanh"))  return FnTanh;
    if (name == QLatin1String("asin"))  return FnAsin;
    if (name == QLatin1String("acos"))  return FnAcos;
    if (name == QLatin1String("atan"))  return FnAtan;
    return FnNone;
}

} // namespace

/// Рекурсивный спуск, строящий дерево. Ошибка не бросает исключение, а взводит
/// `m_bad`: разбор файла задания обязан пережить любое непонятное выражение.
struct ExprCompiler
{
    /// @brief Создаёт разборщик строки.
    /// @param[in]  s      текст без пробелов (должен жить дольше разборщика).
    /// @param[out] nodes  куда добавлять узлы дерева.
    /// @param[out] names  куда добавлять имена переменных.
    ExprCompiler(const QString &s, QVector<ExprProgram::Node> *nodes, QStringList *names)
        : m_s(s), m_nodes(nodes), m_names(names) {}

    // Предел глубины. Вычисление (`run`) и разбор скобок рекурсивны, и выражение
    // из многих тысяч операций (`X_MIN (1+1+…+1);` из 200 000 слагаемых) роняло
    // программу переполнением стека прямо при открытии файла (Т-31, BUG-025).
    // Теперь такое выражение — просто «непонятное». Ядро всё равно не принимает
    // тело блока длиннее 20 000 символов; настоящие формулы на порядки мельче.
    static constexpr int kMaxTreeDepth = 4000;   ///< глубина дерева (рекурсия run)
    static constexpr int kMaxNesting = 1000;     ///< вложенность разбора (скобки, знаки, ^)

    /// @brief Добавляет узел и следит за глубиной дерева.
    /// @param[in] n  узел.
    /// @return Индекс узла; при превышении `kMaxTreeDepth` взводит `m_bad`.
    int add(ExprProgram::Node n)
    {
        int d = 1;
        if (n.a >= 0 && n.a < m_depth.size())
            d = qMax(d, m_depth.at(n.a) + 1);
        if (n.b >= 0 && n.b < m_depth.size())
            d = qMax(d, m_depth.at(n.b) + 1);
        if (d > kMaxTreeDepth)
            m_bad = true;
        m_nodes->append(n);
        m_depth.append(d);
        return int(m_nodes->size()) - 1;
    }

    /// @brief Пропускает символ, если он стоит в текущей позиции.
    /// @param[in] c  ожидаемый символ.
    /// @return true — символ был и пропущен.
    bool eat(QChar c)
    {
        if (m_pos < m_s.size() && m_s.at(m_pos) == c) {
            ++m_pos;
            return true;
        }
        return false;
    }

    /// @brief Разбирает сумму/разность: product {(+|-) product}.
    /// @return Индекс узла или -1 при ошибке.
    int sum()
    {
        int v = product();
        for (;;) {
            if (m_bad)
                return -1;
            if (eat(QLatin1Char('+')))
                v = add({OpAdd, v, product(), -1, 0.0});
            else if (eat(QLatin1Char('-')))
                v = add({OpSub, v, product(), -1, 0.0});
            else
                return v;
        }
    }

    /// @brief Разбирает произведение/частное: unary {(*|/) unary}.
    /// @return Индекс узла или -1 при ошибке.
    int product()
    {
        int v = unary();
        for (;;) {
            if (m_bad)
                return -1;
            if (eat(QLatin1Char('*')))
                v = add({OpMul, v, unary(), -1, 0.0});
            else if (eat(QLatin1Char('/')))
                v = add({OpDiv, v, unary(), -1, 0.0});
            else
                return v;
        }
    }

    /// @brief Разбирает унарные знаки и степень; ограничивает вложенность `kMaxNesting`.
    /// @return Индекс узла или -1 при ошибке.
    int unary()
    {
        // Каждый виток рекурсивного спуска (скобки, функции, знаки, степени)
        // проходит здесь — тут и ограничиваем его глубину.
        if (m_bad || m_level >= kMaxNesting) {
            m_bad = true;
            return -1;
        }
        ++m_level;
        int v;
        if (eat(QLatin1Char('-')))
            v = add({OpNeg, unary(), -1, -1, 0.0});
        else if (eat(QLatin1Char('+')))
            v = unary();
        else
            v = power();
        --m_level;
        return v;
    }

    /// @brief Разбирает степень atom [^ unary] (правоассоциативно через unary).
    /// @return Индекс узла или -1 при ошибке.
    int power()
    {
        const int base = atom();
        if (m_bad)
            return -1;
        if (eat(QLatin1Char('^')))
            return add({OpPow, base, unary(), -1, 0.0});
        return base;
    }

    /// @brief Разбирает скобки, число или имя/вызов функции.
    /// @return Индекс узла или -1 при ошибке.
    int atom()
    {
        if (m_pos >= m_s.size()) {
            m_bad = true;
            return -1;
        }
        if (eat(QLatin1Char('('))) {
            const int v = sum();
            if (!eat(QLatin1Char(')'))) {
                m_bad = true;
                return -1;
            }
            return v;
        }
        const QChar c = m_s.at(m_pos);
        if (c.isDigit() || c == QLatin1Char('.'))
            return number();
        if (c.isLetter() || c == QLatin1Char('_'))
            return call();
        m_bad = true;
        return -1;
    }

    /// @brief Разбирает число, в том числе с показателем (3.1e+17).
    /// @return Индекс узла-константы или -1 при ошибке.
    int number()
    {
        const int start = m_pos;
        while (m_pos < m_s.size() && (m_s.at(m_pos).isDigit() || m_s.at(m_pos) == QLatin1Char('.')))
            ++m_pos;
        if (m_pos < m_s.size()
            && (m_s.at(m_pos) == QLatin1Char('e') || m_s.at(m_pos) == QLatin1Char('E'))) {
            const int save = m_pos;
            ++m_pos;
            if (m_pos < m_s.size()
                && (m_s.at(m_pos) == QLatin1Char('+') || m_s.at(m_pos) == QLatin1Char('-')))
                ++m_pos;
            if (m_pos < m_s.size() && m_s.at(m_pos).isDigit()) {
                while (m_pos < m_s.size() && m_s.at(m_pos).isDigit())
                    ++m_pos;
            } else {
                m_pos = save;
            }
        }
        bool ok = false;
        const double v = m_s.mid(start, m_pos - start).toDouble(&ok);
        if (!ok) {
            m_bad = true;
            return -1;
        }
        return add({OpConst, -1, -1, -1, v});
    }

    /// @brief Разбирает имя: переменную (без скобок) или вызов встроенной функции.
    /// @return Индекс узла или -1 (неизвестная функция, нет закрывающей скобки).
    int call()
    {
        const int start = m_pos;
        while (m_pos < m_s.size()
               && (m_s.at(m_pos).isLetterOrNumber() || m_s.at(m_pos) == QLatin1Char('_')))
            ++m_pos;
        const QString name = m_s.mid(start, m_pos - start);

        if (!eat(QLatin1Char('('))) {
            int slot = int(m_names->indexOf(name));
            if (slot < 0) {
                *m_names << name;
                slot = int(m_names->size()) - 1;
            }
            return add({OpVar, -1, -1, slot, 0.0});
        }
        const int a = sum();
        if (!eat(QLatin1Char(')')) || m_bad) {
            m_bad = true;
            return -1;
        }
        const int fn = functionCode(name);
        if (fn == FnNone) {
            m_bad = true;
            return -1;
        }
        return add({OpFunc, a, -1, fn, 0.0});
    }

    /// Разбираемый текст.
    const QString &m_s;
    /// Узлы строящегося дерева.
    QVector<ExprProgram::Node> *m_nodes;
    /// Имена переменных строящейся программы.
    QStringList *m_names;
    QVector<int> m_depth;   ///< глубина поддерева каждого узла (как m_nodes)
    /// Текущая позиция в тексте.
    int m_pos = 0;
    int m_level = 0;        ///< текущая вложенность разбора
    /// Ошибка разбора.
    bool m_bad = false;
};

ExprProgram ExprProgram::compile(const QString &text, AngleUnit angle)
{
    ExprProgram out;
    out.m_angle = angle;

    const QString clean = stripSpaces(text);
    if (clean.isEmpty())
        return out;

    ExprCompiler c(clean, &out.m_nodes, &out.m_names);
    const int root = c.sum();
    if (c.m_bad || c.m_pos != clean.size() || root < 0) {
        out.m_nodes.clear();
        out.m_names.clear();
        return out;
    }

    out.m_root = root;
    out.m_ok = true;

    // Слоты профиля ищем один раз: в горячем цикле поиска по имени быть не должно.
    for (int i = 0; i < out.m_names.size(); ++i) {
        const QString &n = out.m_names.at(i);
        if (n == QLatin1String("x"))      out.m_slotX = i;
        else if (n == QLatin1String("y")) out.m_slotY = i;
        else if (n == QLatin1String("r")) out.m_slotR = i;
        else if (n == QLatin1String("f")) out.m_slotF = i;
        else                              out.m_foreignName = true;
    }
    return out;
}

double ExprProgram::run(int i, const double *vals, bool *bad) const
{
    if (*bad)
        return 0.0;
    const Node &n = m_nodes.at(i);
    switch (n.op) {
    case OpConst:
        return n.num;
    case OpVar:
        return vals[n.idx];
    case OpNeg:
        return -run(n.a, vals, bad);
    case OpAdd:
        return run(n.a, vals, bad) + run(n.b, vals, bad);
    case OpSub:
        return run(n.a, vals, bad) - run(n.b, vals, bad);
    case OpMul:
        return run(n.a, vals, bad) * run(n.b, vals, bad);
    // ⚠️ Промежуточная бесконечность — НЕ ошибка, и это принципиально.
    //
    // Ядро считает выражение обычной сишной арифметикой. В авторском задании
    // `data/cir_quas.h` (блок 20) стоит `amp*cos(...)/exp(A)/exp(B)`, где на краю
    // области A и B доходят до сотен: `exp` переполняется в +inf, деление на неё даёт
    // законный ноль, и ядро считает такую формулу без единой жалобы. Пока мы ловили
    // переполнение на месте, генератор объявлял ошибкой каждую вторую точку рабочего
    // задания и отказывался его сохранять. Судит только итог — проверка в конце
    // `eval`/`evalEps`. То же и с делением на ноль, и с областями определения:
    // `sqrt(-1)` даёт NaN, NaN доходит до итога, итог отвергается.
    case OpDiv:
        return run(n.a, vals, bad) / run(n.b, vals, bad);
    case OpPow:
        return std::pow(run(n.a, vals, bad), run(n.b, vals, bad));
    case OpFunc:
        break;
    default:
        *bad = true;
        return 0.0;
    }

    const double a = run(n.a, vals, bad);
    if (*bad)
        return 0.0;
    const double k = (m_angle == AngleUnit::Gradus) ? kPi / 180.0 : 1.0;
    switch (n.idx) {
    case FnAbs:   return std::fabs(a);
    case FnExp:   return std::exp(a);
    case FnLog:   return std::log(a);
    case FnLog10: return std::log10(a);
    case FnSqrt:  return std::sqrt(a);
    case FnSin:   return std::sin(a * k);
    case FnCos:   return std::cos(a * k);
    case FnTan:   return std::tan(a * k);
    case FnSinh:  return std::sinh(a);
    case FnCosh:  return std::cosh(a);
    case FnTanh:  return std::tanh(a);
    case FnAsin:  return std::asin(a) / k;
    case FnAcos:  return std::acos(a) / k;
    case FnAtan:  return std::atan(a) / k;
    default:      break;
    }
    *bad = true;
    return 0.0;
}

QStringList ExprProgram::unknown(const Variables &known) const
{
    QStringList out;
    for (const QString &n : m_names) {
        if (!known.contains(n))
            out << n;
    }
    return out;
}

std::optional<double> ExprProgram::eval(const Variables &vars) const
{
    if (!m_ok)
        return std::nullopt;

    QVarLengthArray<double, 8> vals(m_names.size());
    for (int i = 0; i < m_names.size(); ++i) {
        const auto it = vars.constFind(m_names.at(i));
        if (it == vars.constEnd())
            return std::nullopt; // значения нет — считать нечего
        vals[i] = *it;
    }

    bool bad = false;
    const double v = run(m_root, vals.data(), &bad);
    if (bad || !std::isfinite(v))
        return std::nullopt;
    return v;
}

std::optional<double> ExprProgram::evalEps(double x, double y) const
{
    if (!m_ok || m_foreignName)
        return std::nullopt;

    // Считаем только то, что в выражении действительно есть: hypot и atan2 стоят
    // дороже всей остальной формулы, а нужны они далеко не всегда.
    QVarLengthArray<double, 8> vals(m_names.size());
    if (m_slotX >= 0)
        vals[m_slotX] = x;
    if (m_slotY >= 0)
        vals[m_slotY] = y;
    if (m_slotR >= 0)
        vals[m_slotR] = std::hypot(x, y);
    if (m_slotF >= 0) {
        const double k = (m_angle == AngleUnit::Gradus) ? 180.0 / kPi : 1.0;
        vals[m_slotF] = std::atan2(y, x) * k;
    }

    bool bad = false;
    const double v = run(m_root, vals.data(), &bad);
    if (bad || !std::isfinite(v))
        return std::nullopt;
    return v;
}

} // namespace tpl
