// Консольная генерация заданий — проверка эмиттера прогоном ядра без GUI.
//
//   gen_lr2 <папка>
//
// Собирает задачу ЛР2 (пустой волновод и волновод с плазмой, два порта) на общей
// модели Project — той же, на которой будет работать редактор геометрии.
// Смысл: доказать, что общая модель порождает файл, который ядро считает верно.

/// @file gen_lr2.cpp
/// @brief Утилита `gen_lr2`: порождает набор заданий `.tpl` (ЛР2 и проверочные случаи) без интерфейса — для прогона ядрами.
///
/// Запуск: `gen_lr2 [папка]` (по умолчанию — текущая папка; создаётся при необходимости).
/// Каждый проект проходит `validate()`, предупреждения и ошибки печатаются в stdout.
/// Пишутся в CP1251:
/// - `wg_p1.tpl`, `wg_p2.tpl` — пустой волновод ЛР2 (ядро X, 4 ГГц), активен вход 1 / вход 2,
///   общий `wg.s`; `pl_p1.tpl`, `pl_p2.tpl` — то же с замагниченной плазмой (`pl.s`);
/// - `sweep.tpl` + `sweep_step.h` — свип 3.0...5.0 ГГц (21 точка), макро-версия;
/// - `wide.tpl` + `wide_step.h` — свип 1.0...5.0 ГГц (41 точка), нижняя часть ниже отсечки H10
///   (валидатор обязан предупредить, не запрещая расчёт);
/// - `pulse.tpl` — гауссов импульс на входе (центр 20 нс, длительность 5 нс);
/// - `edges.tpl` — волновод одним прямоугольником без торцевых сторон и ломаная-диафрагма;
/// - `ultra.tpl` — 2 варианта геометрии x 2 возбуждения = 4 шага в одном файле;
/// - `plshapes.tpl` — плазма кругом (N+B) и прямоугольником (N+Y).
///
/// Коды возврата: 0 — все задания записаны; 1 — у задания ошибка проверки или записи
/// (дальнейшие задания не пишутся); 2 — папку не создать.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#tools-gen-lr2

#include "cp1251.h"
#include "emitter.h"
#include "project.h"

#include <QCoreApplication>
#include <QDir>
#include <QTextStream>

using namespace tpl;

namespace {

/// Поток отчёта утилиты (stdout).
QTextStream out(stdout);

/// Стенка волновода — металл. Ядро рисует материал КОНТУРОМ, поэтому стенка
/// задаётся тонким прямоугольником толщиной в шаг сетки, как в авторских заданиях.
/// @brief Создаёт металлический прямоугольник-стенку.
/// @param[in] id    номер блока в задании.
/// @param[in] name  имя фигуры (уходит комментарием в `.tpl`).
/// @param[in] xMin,xMax  границы по X, мм.
/// @param[in] yMin,yMax  границы по Y, мм.
/// @return Фигура `ShapeKind::Rectangle` с материалом `Material::Metal`.
Shape wall(int id, const QString &name, double xMin, double xMax, double yMin, double yMax)
{
    Shape s;
    s.id = id;
    s.name = name;
    s.kind = ShapeKind::Rectangle;
    s.material = Material::Metal;
    s.xMin = xMin;
    s.xMax = xMax;
    s.yMin = yMin;
    s.yMax = yMax;
    return s;
}

/// @brief Создаёт вход волны H10 с непрерывным сигналом амплитуды 1.
/// @param[in] id      номер входа в задании.
/// @param[in] name    имя входа.
/// @param[in] side    сторона области, на которой стоит вход.
/// @param[in] from,to  границы апертуры вдоль стороны, мм.
/// @param[in] active  true — вход возбуждается в этом шаге.
/// @return Порт `PortMode::H10`, `SignalShape::Continuous`.
Port makePort(int id, const QString &name, PortSide side, double from, double to, bool active)
{
    Port p;
    p.id = id;
    p.name = name;
    p.side = side;
    p.from = from;
    p.to = to;
    p.active = active;
    p.mode = PortMode::H10;
    p.signal.shape = SignalShape::Continuous;
    p.signal.amplitude = 1.0;
    return p;
}

/// Геометрия ЛР2 в числах: a=60, L=4a=240, W=4a/3=80, стенки на (W±a)/2.
/// @brief Собирает проект ЛР2: волновод на ядре X, два входа, по желанию — замагниченная плазма.
///
/// Параметры сетки: DELTA 1 мм, TIME 115 нс; плазма — квадрат 2a/3 в центре,
/// N = 3.1e17 м^-3, B = 0.071 Тл. Частоты не задаются — их ставит вызывающий.
/// @param[in] plasma      true — добавить плазменную область.
/// @param[in] activeSide  сторона активного входа (`Left` — вход 1, `Right` — вход 2).
/// @param[in] name        имя задания и файла сигналов.
/// @param[in] sFile       имя файла S-матрицы (без расширения).
/// @return Проект с включёнными S-матрицей и топологией, без полей.
Project lr2(bool plasma, PortSide activeSide, const QString &name, const QString &sFile)
{
    const double a = 60.0, d = 1.0;
    const double L = 4 * a, W = 4 * a / 3;
    const double ylo = (W - a) / 2, yhi = (W + a) / 2;

    Project p;
    p.name = name;
    p.kernel = Kernel::X;
    p.box = {0.0, L, 0.0, W};
    p.delta = d;
    p.timeMax = 115.0;

    p.shapes << wall(1, QStringLiteral("нижняя стенка"), 0, L, ylo - d, ylo)
             << wall(2, QStringLiteral("верхняя стенка"), 0, L, yhi, yhi + d);

    if (plasma) {
        const double s = 2 * a / 3;
        // Одна фигура несёт и концентрацию, и подмагничивание: это одна и та же
        // область замагниченной плазмы. В файл она уйдёт двумя блоками — по одному
        // на величину, — но рисовать её дважды не нужно.
        Shape n;
        n.id = 20;
        n.name = QStringLiteral("замагниченная плазма");
        n.kind = ShapeKind::Rectangle;
        n.plasma.hasN = true;
        n.plasma.n = 3.1e+17; // м^-3, СИ (N_UNIT ядро не применяет)
        n.plasma.hasB = true;
        n.plasma.b = 0.071;   // Тл (B_UNIT ядро не применяет)
        n.xMin = L / 2 - s / 2;
        n.xMax = L / 2 + s / 2;
        n.yMin = W / 2 - s / 2;
        n.yMax = W / 2 + s / 2;
        p.shapes << n;
    }

    p.ports << makePort(7, QStringLiteral("вход 1"), PortSide::Left, ylo, yhi,
                        activeSide == PortSide::Left)
            << makePort(12, QStringLiteral("вход 2"), PortSide::Right, ylo, yhi,
                        activeSide == PortSide::Right);

    p.output.signalFile = name;
    p.output.sMatrix = true;
    p.output.sMatrixFile = sFile;
    p.output.topology = true;
    p.output.fields = OutputSettings::Fields::Off;
    return p;
}

/// Тот же волновод, но собранный НОВЫМИ примитивами редактора: контур с разными
/// сторонами и ломаная. Смысл — доказать прогоном, что разложение прямоугольника
/// на отдельные блоки ядро принимает и считает так же, как обычный волновод.
/// @brief Собирает проект `edges` (ядро H, 4 ГГц): волновод контуром без торцов и диафрагма-ломаная.
/// @return Проект с S-матрицей в `edg.s` и топологией.
Project edgesCase()
{
    const double a = 60.0, d = 1.0;
    const double L = 4 * a, W = 4 * a / 3;
    const double ylo = (W - a) / 2, yhi = (W + a) / 2;

    Project p;
    p.name = QStringLiteral("edges");
    p.kernel = Kernel::H;
    p.box = {0.0, L, 0.0, W};
    p.delta = d;
    p.timeMax = 115.0;
    p.freqs = {4.0};

    // Прямоугольник-волновод: торцы сняты (там входы), низ металл, верх металл.
    Shape guide;
    guide.id = 1;
    guide.name = QStringLiteral("волновод контуром");
    guide.kind = ShapeKind::Rectangle;
    guide.material = Material::Metal;
    guide.xMin = 0; guide.xMax = L;
    guide.yMin = ylo; guide.yMax = yhi;
    guide.edges[EdgeLeft].present = false;
    guide.edges[EdgeRight].present = false;
    p.shapes << guide;

    // Ломаная поперёк — тонкая металлическая перегородка с окном посередине.
    Shape iris;
    iris.id = 2;
    iris.name = QStringLiteral("диафрагма");
    iris.kind = ShapeKind::Polyline;
    iris.material = Material::Metal;
    iris.points = {QPointF(L / 2, ylo), QPointF(L / 2, ylo + 15)};
    p.shapes << iris;

    p.ports << makePort(7, QStringLiteral("вход 1"), PortSide::Left, ylo, yhi, true)
            << makePort(12, QStringLiteral("вход 2"), PortSide::Right, ylo, yhi, false);

    p.output.signalFile = QStringLiteral("edges");
    p.output.sMatrix = true;
    p.output.sMatrixFile = QStringLiteral("edg");
    p.output.topology = true;
    p.output.fields = OutputSettings::Fields::Off;
    return p;
}

/// «Ультра-универсальное» задание: 2 варианта геометрии x 2 возбуждения = 4 шага
/// в ОДНОМ файле. Вместо четырёх заданий и четырёх запусков — один прогон,
/// на выходе две полные матрицы рассеяния.
/// @brief Собирает проект `ultra` (ядро H, 4 ГГц): варианты «пустой» и «с eps=4», очередь возбуждений 7, 12.
/// @return Проект с двумя вариантами геометрии и двумя возбуждениями.
Project ultraCase()
{
    const double a = 60.0, d = 1.0;
    const double L = 4 * a, W = 4 * a / 3;
    const double ylo = (W - a) / 2, yhi = (W + a) / 2;

    Project p;
    p.name = QStringLiteral("ultra");
    p.kernel = Kernel::H;
    p.box = {0.0, L, 0.0, W};
    p.delta = d;
    p.timeMax = 115.0;
    p.freqs = {4.0};

    p.shapes << wall(1, QStringLiteral("нижняя стенка"), 0, L, ylo - d, ylo)
             << wall(2, QStringLiteral("верхняя стенка"), 0, L, yhi, yhi + d);

    Shape ins;
    ins.id = 14;
    ins.name = QStringLiteral("диэлектрическая вставка");
    ins.kind = ShapeKind::Rectangle;
    ins.material = Material::Dielectric;
    ins.eps = 4.0;
    ins.xMin = a; ins.xMax = a + 2 * a / 3;
    ins.yMin = ylo; ins.yMax = yhi;
    p.shapes << ins;

    p.ports << makePort(7, QStringLiteral("вход 1"), PortSide::Left, ylo, yhi, true)
            << makePort(12, QStringLiteral("вход 2"), PortSide::Right, ylo, yhi, false);

    // Полная матрица: возбуждаем оба входа по очереди.
    p.exciters = {7, 12};

    Variant empty;
    empty.name = QStringLiteral("пустой волновод");
    empty.tag = QStringLiteral("wg");
    empty.disabledShapes = {14};
    Variant filled;
    filled.name = QStringLiteral("с диэлектриком eps=4");
    filled.tag = QStringLiteral("di");
    p.variants << empty << filled;

    p.output.signalFile = QStringLiteral("u");
    p.output.sMatrix = true;
    p.output.sMatrixFile = QStringLiteral("u");
    p.output.topology = true;
    p.output.fields = OutputSettings::Fields::Off;
    return p;
}

/// @brief Пишет текст в файл в CP1251 с CRLF, сообщая об ошибке и о потерях символов.
/// @param[in] path  путь к файлу.
/// @param[in] text  текст задания.
/// @return true — файл записан (возможно, с заменой символов на '?'); false — ошибка записи.
bool write(const QString &path, const QString &text)
{
    QString err;
    bool lossy = false;
    if (!writeCp1251File(path, text, &err, &lossy)) {
        out << "ОШИБКА: " << path << ": " << err << "\n";
        return false;
    }
    if (lossy)
        out << "ВНИМАНИЕ: " << path << ": часть символов заменена на '?'\n";
    return true;
}

/// @brief Проверяет проект и пишет его задание: моно-версию или макро-версию (`.tpl` + `_step.h`).
/// @param[in] p    проект.
/// @param[in] dir  папка вывода.
/// @return true — проверка без ошибок и файлы записаны; false — иначе (причины в stdout).
bool emitProject(const Project &p, const QString &dir)
{
    const Diagnostics d = validate(p);
    for (const QString &w : d.warnings)
        out << "  предупреждение (" << p.name << "): " << w << "\n";
    for (const QString &e : d.errors)
        out << "  ОШИБКА (" << p.name << "): " << e << "\n";
    if (!d.ok())
        return false;

    if (needsMacro(p)) {
        const MacroFiles m = emitMacro(p);
        if (!write(QDir(dir).filePath(p.name + QStringLiteral(".tpl")), m.tpl)
            || !write(QDir(dir).filePath(m.stepHName), m.stepH))
            return false;
        out << "  " << p.name << ".tpl + " << m.stepHName << " (" << p.freqs.size() << " шагов)\n";
    } else {
        if (!write(QDir(dir).filePath(p.name + QStringLiteral(".tpl")), emitMono(p)))
            return false;
        out << "  " << p.name << ".tpl\n";
    }
    return true;
}

} // namespace

/// @brief Точка входа: создаёт папку и последовательно пишет все задания набора.
/// @param[in] argc  число аргументов командной строки.
/// @param[in] argv  аргументы: необязательная папка вывода.
/// @return 0 — успех; 1 — ошибка проверки или записи задания; 2 — папку не создать.
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const QStringList args = app.arguments();
    const QString dir = args.size() > 1 ? args.at(1) : QStringLiteral(".");
    if (!QDir().mkpath(dir)) {
        out << "не удалось создать папку: " << dir << "\n";
        return 2;
    }

    // Одна частота: пара прогонов даёт полную матрицу 2x2 в один .s.
    struct Case { const char *name; bool plasma; PortSide side; const char *sFile; };
    static const Case cases[] = {
        {"wg_p1", false, PortSide::Left, "wg"},
        {"wg_p2", false, PortSide::Right, "wg"},
        {"pl_p1", true, PortSide::Left, "pl"},
        {"pl_p2", true, PortSide::Right, "pl"},
    };
    for (const Case &c : cases) {
        Project p = lr2(c.plasma, c.side, QLatin1String(c.name), QLatin1String(c.sFile));
        p.freqs = {4.0};
        if (!emitProject(p, dir))
            return 1;
    }

    // Свип через макрос.
    Project s = lr2(false, PortSide::Left, QStringLiteral("sweep"), QStringLiteral("wgm"));
    for (int i = 0; i < 21; ++i)
        s.freqs << 3.0 + 0.1 * i;
    s.output.signalFile = QStringLiteral("wgm");
    if (!emitProject(s, dir))
        return 1;

    // Широкий свип 1...5 ГГц с шагом 0.1 — 41 точка. Ровно тот случай, ради которого
    // существует макро-версия: расписывать сорок один одинаковый #STEP руками нельзя.
    // Заодно проверка валидатора: нижняя половина полосы лежит НИЖЕ отсечки H10
    // (для a = 60 мм она 2.50 ГГц), и он обязан про это сказать, не запрещая расчёт.
    Project w = lr2(false, PortSide::Left, QStringLiteral("wide"), QStringLiteral("wd"));
    for (int i = 0; i <= 40; ++i)
        w.freqs << 1.0 + 0.1 * i;
    w.output.signalFile = QStringLiteral("wd");
    if (!emitProject(w, dir))
        return 1;

    // Гауссов импульс: длительность задаёт огибающая в Amp, а не узкое окно.
    Project g = lr2(false, PortSide::Left, QStringLiteral("pulse"), QStringLiteral("pg"));
    g.freqs = {4.0};
    g.ports[0].signal.shape = SignalShape::Gaussian;
    g.ports[0].signal.center = 20.0;
    g.ports[0].signal.duration = 5.0;
    g.output.signalFile = QStringLiteral("pulse");
    if (!emitProject(g, dir))
        return 1;

    // Новые примитивы редактора: разные стороны прямоугольника + ломаная.
    if (!emitProject(edgesCase(), dir))
        return 1;

    // Варианты геометрии и оба возбуждения в одном файле.
    if (!emitProject(ultraCase(), dir))
        return 1;

    // ВРЕМЕННО: приёмка плазмы всеми формами — круг с N+B и прямоугольник с Y.
    {
        Project pl = lr2(false, PortSide::Left, QStringLiteral("plshapes"),
                         QStringLiteral("pls"));
        pl.freqs = {5.5};
        Shape col;
        col.id = 30;
        col.name = QStringLiteral("плазменный столб");
        col.kind = ShapeKind::Circle;
        col.center = QPointF(120, 40);
        col.rMin = 0.0;
        col.rMax = 20.0;
        col.fiMin = 0.0;
        col.fiMax = 6.28;
        col.plasma.hasN = true;
        col.plasma.n = 1.0e+17;
        col.plasma.hasB = true;
        col.plasma.b = 0.05;
        pl.shapes << col;

        Shape lossy;
        lossy.id = 31;
        lossy.name = QStringLiteral("область потерь");
        lossy.kind = ShapeKind::Rectangle;
        lossy.xMin = 150; lossy.xMax = 170; lossy.yMin = 20; lossy.yMax = 60;
        lossy.plasma.hasN = true;
        lossy.plasma.n = 5.0e+16;
        lossy.plasma.hasY = true;
        lossy.plasma.y = 0.02;
        pl.shapes << lossy;

        if (!emitProject(pl, dir))
            return 1;
    }

    out << "готово: " << QDir(dir).absolutePath() << "\n";
    return 0;
}
