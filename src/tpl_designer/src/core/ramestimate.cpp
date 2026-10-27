/// @file ramestimate.cpp
/// @brief Реализация оценки памяти ядра (`ramEstimate`) и форматирования размеров.
///
/// Описание — в ramestimate.h.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#ram
#include "ramestimate.h"
#include "lang.h"

#include "sigfile.h"

#include <algorithm>
#include <cmath>

namespace tpl {

namespace {

// Замер 30.09.2026 на сервере (TMC_Suite/GPU_PLAN.md): байт на узел с блоками.
/// Память H-ядра на узел сетки с блоками, байт.
constexpr double kBytesPerNodeH = 52.0;
/// Память X-ядра на узел сетки с блоками, байт.
constexpr double kBytesPerNodeX = 127.0;

// `_real` в счётных ядрах — double (Typerth.h).
/// Размер вещественного числа ядра (`_real` = double), байт.
constexpr double kReal = 8.0;

} // namespace

RamEstimate ramEstimate(const Project &p, double freeBytes)
{
    RamEstimate r;
    if (p.delta <= 0.0 || p.box.width() <= 0.0 || p.box.height() <= 0.0)
        return r;

    const double nX = std::floor(p.box.width() / p.delta + 0.5) + 1.0;
    const double nY = std::floor(p.box.height() / p.delta + 0.5) + 1.0;
    r.nodes = nX * nY;
    r.gridBytes = r.nodes * (p.kernel == Kernel::X ? kBytesPerNodeX : kBytesPerNodeH);

    r.pattern = p.output.directionalPattern;
    if (!r.pattern)
        return r;

    // Сколько тонов назовёт ядру файл сигнала. Ядро берёт список ПЕРВОГО блока, у
    // которого он есть (`TmcRTHRectNode.cpp`: «one list describes the excitation»), но в
    // очереди возбуждений на каждом шаге излучает свой порт — берём наибольший.
    int request = 0;
    for (const Port &port : p.ports) {
        if (portUsesSignalFile(p, port))
            request = std::max<int>(request, int(waveTones(port.signal.wave).size()));
    }
    r.patternToneRequest = request;
    r.patternTones = request > 1 ? std::min(request, kMaxFreqs) : 1;

    // Многотон — всегда по полосам. Одна частота — по всей области, пока четыре
    // массива на сетку укладываются в четверть свободной памяти (правило ядра).
    const double whole = r.nodes * kReal * 4.0;
    r.patternBorder = r.patternTones > 1
                      || (freeBytes > 0.0 && nX >= 32 && nY >= 32 && whole > 0.25 * freeBytes);
    if (r.patternBorder) {
        // Полосы толщиной три узла вдоль контура ДН. Контур по умолчанию отстоит на
        // десять узлов от края коробки, и полоса берёт ещё по узлу наружу
        // (`BorderGeometry`): ширина nX - 17, высота nY - 17. Нижняя и верхняя полосы
        // идут во всю ширину, боковые — между ними, чтобы узел не хранился дважды.
        const double w = std::max(0.0, nX - 17.0);
        const double h = std::max(0.0, nY - 17.0);
        const double hMid = std::max(0.0, h - 6.0);
        const double strip = 6.0 * w + 6.0 * hMid;
        // Пара квадратур на каждый тон плюс амплитуда и фаза одного тона, который
        // в эту минуту превращается в диаграмму.
        r.patternBytes = strip * kReal * (2.0 * r.patternTones + 2.0);
    } else {
        r.patternBytes = whole;
    }
    return r;
}

QString formatBytes(double bytes)
{
    const double k = 1024.0;
    if (bytes >= k * k * k)
        return tpl::tr("%1 ГБ",
                       "%1 GB").arg(bytes / (k * k * k), 0, 'f', bytes >= 10 * k * k * k ? 0 : 1);
    if (bytes >= k * k)
        return tpl::tr("%1 МБ", "%1 MB").arg(bytes / (k * k), 0, 'f', 0);
    if (bytes >= k)
        return tpl::tr("%1 КБ", "%1 KB").arg(bytes / k, 0, 'f', 0);
    return tpl::tr("%1 Б", "%1 B").arg(bytes, 0, 'f', 0);
}

} // namespace tpl
