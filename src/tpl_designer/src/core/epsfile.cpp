/// @file epsfile.cpp
/// @brief Реализация записи и чтения файла распределения eps в формате ядра (`SaveFileRect`).
///
/// Описание — в epsfile.h.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#eps-file
#include "epsfile.h"
#include "lang.h"

#include "expr.h"

#include <QByteArray>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace tpl {

namespace {

/// Вес узла в прямоугольнике: 1 внутри, 0.5 на ребре, 0.25 в углу.
///
/// Так считает ядро (`SetRect`), и так два соседних блока в сумме дают на общем
/// ребре единицу. Переписать это «понятнее» нельзя — файл обязан совпадать с тем,
/// что ядро напишет само.
/// @param[in] i   номер строки узла (0...nY-1).
/// @param[in] j   номер столбца узла (0...nX-1).
/// @param[in] nX  число узлов по X.
/// @param[in] nY  число узлов по Y.
/// @return Вес узла: 1, 0.5 или 0.25.
double nodeWeight(int i, int j, int nX, int nY)
{
    double w = 1.0;
    if (j == 0 || j == nX - 1)
        w *= 0.5;
    if (i == 0 || i == nY - 1)
        w *= 0.5;
    return w;
}

/// @brief Формирует строку шапки «тег число» с CRLF (6 значащих цифр).
/// @param[in] tag  тег с пробелом в конце, например `#dDelta `.
/// @param[in] v    значение.
/// @return Строка шапки.
QByteArray numberLine(const char *tag, double v)
{
    return QByteArray(tag) + QByteArray::number(v, 'g', 6) + "\r\n";
}

} // namespace

EpsGrid epsGridFor(double delta, double xMin, double xMax, double yMin, double yMax)
{
    EpsGrid g;
    g.delta = delta > 0.0 ? delta : 1.0;
    g.xMin = std::min(xMin, xMax);
    g.yMin = std::min(yMin, yMax);
    g.nX = std::max(2, int(std::lround(std::fabs(xMax - xMin) / g.delta)) + 1);
    g.nY = std::max(2, int(std::lround(std::fabs(yMax - yMin) / g.delta)) + 1);
    return g;
}

QByteArray epsFileBytes(const EpsGrid &g, const std::function<double(double, double)> &addAt)
{
    QByteArray out;
    if (!g.valid())
        return out;

    out += "#TamicRTH_planar_DistributionDielectricPermeability_File_V2.00 2000\r\n";
    out += "#TopologyPrimitiv RECT_STAT \r\n";
    // ⚠️ Шаг и начало — в МЕТРАХ, что бы ни стояло в LONG_UNIT задания.
    out += numberLine("#dDelta ", g.delta / 1000.0);
    out += numberLine("#Xmin ", g.xMin / 1000.0);
    out += numberLine("#Ymin ", g.yMin / 1000.0);
    out += QByteArray("#nX ") + QByteArray::number(g.nX) + "\r\n";
    out += QByteArray("#nY ") + QByteArray::number(g.nY) + "\r\n";
    out += QByteArray("#nPoint ") + QByteArray::number(g.points()) + "\r\n";
    out += "#nAccuracy 8\r\n";
    out += "#sNodeFormat NULL\r\n";
    out += "#sValueFormat NULL\r\n";

    int idx = 0;
    for (int i = 0; i < g.nY; ++i) {
        for (int j = 0; j < g.nX; ++j) {
            const double w = nodeWeight(i, j, g.nX, g.nY);
            // Округление до 1e-12: `exp` в libm разных систем расходится в последнем
            // бите, а файл обязан получаться одинаковым на Windows, macOS и Linux.
            const double val =
                std::round(w * addAt(g.xMin + j * g.delta, g.yMin + i * g.delta) * 1e12) / 1e12;
            char buf[sizeof(double) + sizeof(qint32)];
            const qint32 node = idx++;
            std::memcpy(buf, &val, sizeof(double)); // порядок байт родной, как у fwrite
            std::memcpy(buf + sizeof(double), &node, sizeof(qint32));
            out.append(buf, sizeof(buf));
        }
    }
    return out;
}

QByteArray epsFileBytes(double deltaMm, double wMm, double hMm,
                        const std::function<double(double, double)> &addAt)
{
    EpsGrid g;
    g.delta = deltaMm;
    g.nX = int(std::lround(wMm / deltaMm)) + 1;
    g.nY = int(std::lround(hMm / deltaMm)) + 1;
    g.xMin = -wMm / 2.0;
    g.yMin = -hMm / 2.0;
    return epsFileBytes(g, addAt);
}

QVector<double> epsValues(const EpsGrid &g, const QString &formula, QString *error)
{
    QVector<double> out;
    const ExprProgram prog = ExprProgram::compile(formula);
    if (!prog.valid()) {
        if (error)
            *error = tpl::tr("формулу не удалось разобрать", "the formula could not be parsed");
        return out;
    }
    const QStringList unknown = prog.unknown(epsVariables(0.0, 0.0));
    if (!unknown.isEmpty()) {
        if (error)
            *error = tpl::tr("в формуле есть имя, которого ядро не знает: %1",
                             "the formula contains a name the kernel does not know: %1")
                         .arg(unknown.join(QStringLiteral(", ")));
        return out;
    }
    out.reserve(g.points());
    for (int i = 0; i < g.nY; ++i) {
        for (int j = 0; j < g.nX; ++j) {
            const std::optional<double> v =
                prog.evalEps(g.xMin + j * g.delta, g.yMin + i * g.delta);
            if (!v) {
                if (error)
                    *error = tpl::tr("в узле (%1; %2) формула не даёт числа",
                                     "at node (%1; %2) the formula gives no number")
                                 .arg(g.xMin + j * g.delta).arg(g.yMin + i * g.delta);
                out.clear();
                return out;
            }
            out << *v;
        }
    }
    if (error)
        error->clear();
    return out;
}

bool readEpsHeader(const QByteArray &data, EpsGrid *out, QString *error)
{
    const auto fail = [&](const QString &why) {
        if (error)
            *error = why;
        return false;
    };
    if (!data.startsWith("#TamicRTH_planar_DistributionDielectricPermeability_File"))
        return fail(tpl::tr("это не файл распределения .eps",
                            "this is not a .eps distribution file"));

    EpsGrid g;
    bool haveDelta = false, haveNx = false, haveNy = false;
    int pos = 0;
    while (pos < data.size()) {
        const int nl = data.indexOf('\n', pos);
        if (nl < 0)
            break;
        const QByteArray line = data.mid(pos, nl - pos).trimmed();
        pos = nl + 1;
        if (line.isEmpty() || line.at(0) != '#')
            break;
        const int sp = line.indexOf(' ');
        if (sp < 0)
            continue;
        const QByteArray tag = line.left(sp);
        const QByteArray val = line.mid(sp + 1).trimmed();
        bool ok = false;
        if (tag == "#dDelta") {
            g.delta = val.toDouble(&ok) * 1000.0; // в файле метры, в модели миллиметры
            haveDelta = ok;
        } else if (tag == "#Xmin") {
            g.xMin = val.toDouble(&ok) * 1000.0;
        } else if (tag == "#Ymin") {
            g.yMin = val.toDouble(&ok) * 1000.0;
        } else if (tag == "#nX") {
            g.nX = val.toInt(&ok);
            haveNx = ok;
        } else if (tag == "#nY") {
            g.nY = val.toInt(&ok);
            haveNy = ok;
        } else if (tag == "#sValueFormat" || tag == "#sNodeFormat") {
            // после них идут двоичные данные
        }
    }
    if (!haveDelta || !haveNx || !haveNy || !g.valid())
        return fail(tpl::tr("в шапке файла нет шага или размеров сетки",
                            "the file header has no grid step or grid size"));
    if (out)
        *out = g;
    if (error)
        error->clear();
    return true;
}

QVector<double> readEpsValues(const QByteArray &data, const EpsGrid &g)
{
    QVector<double> out;
    // Данные начинаются сразу за последней строкой шапки.
    const int marker = data.indexOf("#sValueFormat");
    if (marker < 0)
        return out;
    const int nl = data.indexOf('\n', marker);
    if (nl < 0)
        return out;
    const int start = nl + 1;
    const int recordSize = int(sizeof(double) + sizeof(qint32));
    const int have = (data.size() - start) / recordSize;
    out.resize(g.points());
    std::fill(out.begin(), out.end(), 0.0);
    for (int k = 0; k < have; ++k) {
        double value = 0.0;
        qint32 node = 0;
        std::memcpy(&value, data.constData() + start + k * recordSize, sizeof(double));
        std::memcpy(&node, data.constData() + start + k * recordSize + sizeof(double),
                    sizeof(qint32));
        if (node >= 0 && node < out.size())
            out[node] = value;
    }
    return out;
}

} // namespace tpl
