/// @file emitter.cpp
/// @brief Эмиттер задания: проверка проекта и запись текста `.tpl` (моно, макро-версия,
///        угловой свип, конвейер заданий по частотам).
///
/// Часть ядра редактора заданий. Здесь собраны все правила языка, которые генератор
/// закрывает кодом: добавка eps-1 вместо eps, локальные координаты и анкер, обход и
/// замыкание полигонов, разбиение прямоугольника на стороны, восьмой параметр входа,
/// окно пассивного порта, запись чисел без экспоненты, ровно два бэкслеша переноса в
/// макросе, `#EOF`. Текст строится в Unicode; в CP1251 его переводит вызывающий
/// (`writeTask`), проверка заранее ищет символы вне CP1251.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#emitter
#include "emitter.h"
#include "lang.h"

#include "cp1251.h"
#include "expr.h"
#include "physics.h"
#include "placement.h"
#include "ramestimate.h"
#include "sigfile.h"

#include <QHash>
#include <QRegularExpression>
#include <QSet>

#include <algorithm>
#include <cmath>

namespace tpl {
namespace {

/// @brief Возвращает ключевое слово языка для контурного материала.
/// @param[in] m  материал
/// @return `METAL`, `MAGNETIC` или `ABSORBER`; пустая строка для диэлектрика.
QString materialKeyword(Material m)
{
    switch (m) {
    case Material::Metal:     return QStringLiteral("METAL");
    case Material::Magnetic:  return QStringLiteral("MAGNETIC");
    case Material::Absorber:  return QStringLiteral("ABSORBER");
    case Material::Dielectric: break;
    }
    return {};
}

/// Первый параметр блока: имя материала либо число.
/// Для диэлектрика в файл уходит ДОБАВКА eps-1, а не сама eps: eps=4 -> "3".
/// @param[in] s  фигура
/// @return Ключевое слово материала или число добавки eps-1 (без экспоненты).
QString fillExpression(const Shape &s)
{
    if (s.material == Material::Dielectric)
        return formatNumber(s.eps - 1.0);
    return materialKeyword(s.material);
}

/// Первый параметр блока с профилем eps — в режиме СОЗДАНИЯ файла.
///
/// `[имя]` перед выражением — команда ядру: посчитать выражение в каждом узле блока и
/// сохранить результат в `<имя>.eps` (`SeachEpsFileName` отрезает скобки, `SaveFileRect`
/// пишет файл). Само выражение при этом всё равно считается, то есть шаг не холостой.
///
/// Скобки вокруг формулы обязательны: подстановка в языке ТЕКСТОВАЯ, и без них
/// `2*eps` превратилось бы в `2*eps-1` вместо `(2*eps)-1`.
/// @param[in] s  фигура с профилем eps и именем файла
/// @return Строка вида `[имя](формула)-1`; числа с экспонентой в формуле переписаны
///         полной десятичной записью.
QString epsProfileCreate(const Shape &s)
{
    return QStringLiteral("[%1](%2)-1")
        .arg(epsProfileFile(s), plainLiterals(s.epsProfile.formula.trimmed()));
}

/// @brief Возвращает тег блока фигуры: `RECT`/`CIRCLE`/`POLYGON` с суффиксом `_STAT`
///        или `_MOVE` (если у фигуры есть скорость).
/// @param[in] s  фигура
/// @return Тег блока без плазменного суффикса.
QString shapeKeyword(const Shape &s)
{
    QString base;
    switch (s.kind) {
    case ShapeKind::Rectangle: base = QStringLiteral("RECT"); break;
    case ShapeKind::Circle:    base = QStringLiteral("CIRCLE"); break;
    case ShapeKind::Polygon:
    case ShapeKind::Polyline:  base = QStringLiteral("POLYGON"); break;
    }
    // Движущиеся блоки плазменных вариантов не имеют — язык их не знает.
    if (s.isMoving())
        return base + QStringLiteral("_MOVE");
    return base + QStringLiteral("_STAT");
}

/// Анкер блока — строка `#LINK_LIST`. Ядро ПРИБАВЛЯЕТ его к координатам блока.
///
/// Для прямоугольника и полигона держим анкер в нуле и пишем абсолютные координаты:
/// так текст читаем, и правка одной фигуры не трогает остальные.
/// У окружности выбора нет — центра в блоке не существует, им служит именно анкер.
/// @param[in] s  фигура
/// @return Анкер, мм: центр круга либо (0,0).
QPointF shapeAnchor(const Shape &s)
{
    if (s.kind == ShapeKind::Circle)
        return s.center;
    return {0.0, 0.0};
}

/// Анкер порта — это и есть ПЛОСКОСТЬ входа.
///
/// ⚠️ Ядро ставит узлы входа ровно по этой координате (`SetInpXLeft` берёт `GetX0()`
/// буквально), а «левый/правый» выводит из того, в какой половине коробки координата
/// лежит. Значит вход законно стоит и внутри области — не только на её границе.
/// @param[in] p    порт
/// @param[in] box  счётная область
/// @return Анкер порта, мм: (плоскость, 0) для `INPUT_X`, (0, плоскость) для `INPUT_Y`.
QPointF portAnchor(const Port &p, const Box &box)
{
    const double plane = portPlane(p, box);
    return portAlongY(p.side) ? QPointF(plane, 0.0) : QPointF(0.0, plane);
}

/// Левая/правая граница — это вход вдоль оси Y (INPUT_X), верх/низ — INPUT_Y.
/// Название = ось, ВДОЛЬ которой идёт возбуждение. Пользователю знать не нужно.
/// @param[in] p  порт
/// @return `INPUT_X` для левого/правого порта, `INPUT_Y` для верхнего/нижнего.
QString portKeyword(const Port &p)
{
    const bool alongY = (p.side == PortSide::Left || p.side == PortSide::Right);
    return alongY ? QStringLiteral("INPUT_X") : QStringLiteral("INPUT_Y");
}

/// Мода — пространственное распределение в апертуре. Формы сигнала НЕ задаёт.
/// @param[in] m  мода порта
/// @return `MAGNETIC` для плоской волны; пустая строка для H10 (умолчание ядра).
QString portModeKeyword(PortMode m)
{
    return m == PortMode::Plane ? QStringLiteral("MAGNETIC") : QString();
}

/// Ядро отвергает POLYGON_STAT меньше чем с ТРЕМЯ строками L — проверка
/// `if( n1 < 3 )` в `TMCIndan/TmcRTH_BolckList.cpp` стоит до разбора материала,
/// так что отрезок из двух точек не грузится вообще (проверено прогоном:
/// «Number's of lines L x; y; < 3 for POLYGON_STAT»).
///
/// Геометрию это не меняет: добиваем серединой отрезка, точка ложится на ту же прямую.
/// @param[in,out] pts  вершины; при двух точках между ними вставляется середина,
///                     при одной и при трёх и более — без изменений
void padToThreePoints(QVector<QPointF> *pts)
{
    while (pts->size() >= 2 && pts->size() < 3)
        pts->insert(1, (pts->at(0) + pts->at(1)) / 2.0);
}

/// Точки полигона в том виде, в каком их ждёт ядро.
///
/// ⚠️ Правила разные и путать их нельзя (tpl-guide 5.4):
///  - у КОНТУРНОГО материала последняя точка с первой НЕ соединяется — это ломаная,
///    поэтому замкнутый контур закрываем сами, повторив первую точку;
///  - у ВЫРАЖЕНИЯ ядро замыкает само, но требует обхода ПО ЧАСОВОЙ стрелке.
///    Разворот молча даёт неверное поле, поэтому переставляем точки здесь, а не
///    надеемся, что пользователь обвёл в нужную сторону.
/// @param[in] s  полигон или ломаная
/// @return Вершины для строк `L`, мм: ломаная — как есть (не меньше трёх точек);
///         замкнутый контур материала — с повтором первой точки; заливка — обход по
///         часовой стрелке.
QVector<QPointF> polygonPoints(const Shape &s)
{
    QVector<QPointF> pts = s.points;
    if (s.kind == ShapeKind::Polyline) {
        padToThreePoints(&pts); // ломаная как нарисована, замыкать её незачем
        return pts;
    }

    if (s.isContour()) {
        if (pts.size() >= 3 && pts.first() != pts.last())
            pts << pts.first();
        padToThreePoints(&pts);
        return pts;
    }

    if (signedArea(pts) > 0.0) // > 0 — против часовой, ядру нужно наоборот
        std::reverse(pts.begin(), pts.end());
    return pts;
}

/// Тело блока: заголовок `<ТЕГ> <значение>;` и координаты фигуры.
///
/// Тег передаётся снаружи, потому что одна и та же геометрия уходит в файл несколько
/// раз: сама фигура, её концентрация `_N`, её подмагничивание `_B`.
/// @param[in] s      фигура (координаты уже в системе блока, мм)
/// @param[in] tag    тег блока (`RECT_STAT`, `CIRCLE_STAT_N`, `FILE` ...)
/// @param[in] value  первый параметр блока (материал, число или выражение)
/// @return Строки тела блока с отступом: заголовок и координаты; у круга — радиусы и
///         углы (рад), у полигона — строки `L`; у движущейся фигуры — строка скорости.
QStringList geometryLines(const Shape &s, const QString &tag, const QString &value)
{
    QStringList out;
    const QString head = QStringLiteral("%1 %2;").arg(tag, value);

    switch (s.kind) {
    case ShapeKind::Rectangle:
        out << QStringLiteral("  %1 %2; %3; %4; %5;")
                   .arg(head, formatNumber(s.xMin), formatNumber(s.xMax),
                        formatNumber(s.yMin), formatNumber(s.yMax));
        break;
    case ShapeKind::Circle:
        // Центра здесь нет — он в анкере. Углы в радианах (ANGLE_UNIT radian).
        out << QStringLiteral("  %1 %2; %3; %4; %5;")
                   .arg(head, formatNumber(s.rMin), formatNumber(s.rMax),
                        formatNumber(s.fiMin), formatNumber(s.fiMax));
        break;
    case ShapeKind::Polygon:
    case ShapeKind::Polyline:
        out << QStringLiteral("  %1").arg(head);
        for (const QPointF &pt : polygonPoints(s))
            out << QStringLiteral("   L %1; %2;").arg(formatNumber(pt.x()), formatNumber(pt.y()));
        break;
    }

    if (s.isMoving())
        out << QStringLiteral("   %1; %2;").arg(formatNumber(s.vx), formatNumber(s.vy));

    return out;
}

/// @brief Возвращает тело блока обычной фигуры (без плазмы и профиля eps).
/// @param[in] s  фигура
/// @return Строки блока: тег по виду фигуры и материал либо добавка eps-1.
QStringList shapeLines(const Shape &s)
{
    return geometryLines(s, shapeKeyword(s), fillExpression(s));
}

/// Блок фигуры с профилем eps. `save` — сохранять ли результат в `.eps` на этом шаге.
///
/// Форма — любая: посчитать выражение в узлах ядро умеет и в круге, и в полигоне
/// (`CIRCLE_STAT <выражение>` — блок 20 авторского `data/cir_quas.h`). Прямоугольником
/// ограничена только ЗАПИСЬ в файл, поэтому `save` приходит уже с учётом формы.
///
/// Координаты ЛОКАЛЬНЫЕ, относительно центра: центр уходит в анкер (`#LINK_LIST`), и
/// ядро подставляет в формулу координаты уже относительно него. Так формула не
/// зависит от того, где фигура лежит на сцене.
///
/// ⚠️ Блок `FILE <имя>;` — тот, которым профиль полагается ЧИТАТЬ, — ядром не
/// принимается. Проверено прогоном 2026-08-11 на всех четырёх сборках
/// (`dist/win32|win64` x `tmc_rth|tmc_rtx`): любое написание — с координатами и без,
/// с расширением и без, с отступом и без — останавливает шаг сообщением
/// «Error : Missing separator ;», и ни одного выходного файла не появляется.
/// Подробности и способ воспроизведения — в `TMC_Suite/ISSUES.md`.
///
/// Поэтому формула пишется в КАЖДЫЙ шаг, а `[имя]` — только в первый. Что это даёт:
/// файл профиля появляется на диске один раз (его можно посмотреть и проверить), а не
/// переписывается на каждой частоте. Сэкономить сам расчёт выражения нельзя, пока
/// блок `FILE` не почините в ядре — ядро других способов не оставляет.
/// @param[in] s     фигура с действующим профилем eps
/// @param[in] save  true — писать `[имя]` (сохранить файл на этом шаге)
/// @return Строки блока: `FILE <имя>` по габариту фигуры для профиля из файла, иначе
///         блок фигуры с выражением `(формула)-1` в координатах относительно анкера.
QStringList epsProfileLines(const Shape &s, bool save)
{
    // Профиль из готового файла — это ДРУГОЙ примитив языка: не `RECT_STAT`, а
    // `FILE <имя>;`. Координаты ядро у него не читает (сетка лежит в самом файле,
    // положение задаёт анкер), но пишем мы их всё равно: по ним фигура видна в
    // редакторе, и ровно так написаны авторские задания.
    if (s.epsProfile.fromFile) {
        Shape box = s;
        box.kind = ShapeKind::Rectangle;
        s.bounds(&box.xMin, &box.xMax, &box.yMin, &box.yMax);
        const QPointF ofs = s.epsOrigin();
        box.xMin -= ofs.x(); box.xMax -= ofs.x();
        box.yMin -= ofs.y(); box.yMax -= ofs.y();
        box.vx = 0.0; box.vy = 0.0; // у FILE движения не бывает
        return geometryLines(box, QStringLiteral("FILE"), epsProfileFile(s));
    }

    const QString value = save ? epsProfileCreate(s)
                               : QStringLiteral("(%1)-1").arg(plainLiterals(s.epsProfile.formula.trimmed()));

    // Геометрию пишем ту же, что у обычной фигуры, только сдвинутую в анкер. У круга
    // сдвигать нечего — в блоке лежат радиусы и углы, а центр и так уходит в анкер.
    Shape local = s;
    const QPointF o = s.epsOrigin();
    local.xMin -= o.x(); local.xMax -= o.x();
    local.yMin -= o.y(); local.yMax -= o.y();
    for (QPointF &pt : local.points)
        pt -= o;

    return geometryLines(local, shapeKeyword(s), value);
}

/// Прямоугольный блок по готовым границам. Так пишутся `_B` и `_Y`, а также `FILE_N`:
/// у всех троих в языке есть только прямоугольная форма, пригодная для генератора.
/// @param[in] tag    тег блока (`RECT_STAT_B`, `FILE_N` ...)
/// @param[in] value  первый параметр (число, выражение или имя файла)
/// @param[in] xLo    левая граница, мм
/// @param[in] xHi    правая граница, мм
/// @param[in] yLo    нижняя граница, мм
/// @param[in] yHi    верхняя граница, мм
/// @return Одна строка блока `<tag> <value>; x1; x2; y1; y2;`.
QStringList rectBlockLines(const QString &tag, const QString &value,
                           double xLo, double xHi, double yLo, double yHi)
{
    return QStringList{QStringLiteral("  %1 %2; %3; %4; %5; %6;")
                           .arg(tag, value, formatNumber(xLo), formatNumber(xHi),
                                formatNumber(yLo), formatNumber(yHi))};
}

/// @brief Возвращает название стороны прямоугольника для комментария задания.
/// @param[in] e  номер стороны (`RectEdge`)
/// @return «низ», «право», «верх», «лево» на текущем языке; пусто для неверного номера.
QString edgeName(int e)
{
    switch (e) {
    case EdgeBottom: return tpl::tr("низ", "bottom");
    case EdgeRight:  return tpl::tr("право", "right");
    case EdgeTop:    return tpl::tr("верх", "top");
    case EdgeLeft:   return tpl::tr("лево", "left");
    }
    return {};
}

/// Одна сторона прямоугольника отдельным блоком — ломаная вдоль этой стороны.
///
/// Так закрывается ловушка: `RECT_STAT METAL` рисует ВЕСЬ периметр, и «убрать одну
/// стенку» или сделать её магнитной внутри одного блока нечем.
/// Точек ровно три — меньше ядро не примет (см. padToThreePoints).
/// @param[in] s  прямоугольник
/// @param[in] e  номер стороны (`RectEdge`)
/// @return Строки блока `POLYGON_STAT <материал стороны>;` с тремя точками `L`.
QStringList edgeLines(const Shape &s, int e)
{
    QVector<QPointF> pts{s.edgeFrom(e), s.edgeTo(e)};
    padToThreePoints(&pts);

    QStringList out;
    out << QStringLiteral("  POLYGON_STAT %1;").arg(materialKeyword(s.edgeMaterial(e)));
    for (const QPointF &pt : pts)
        out << QStringLiteral("   L %1; %2;").arg(formatNumber(pt.x()), formatNumber(pt.y()));
    return out;
}

/// `eps` — проницаемость среды в сечении входа (\ref portEps): её добавка уходит
/// восьмым параметром.
/// @brief Строит строку блока входа `INPUT_X`/`INPUT_Y` с восемью параметрами.
/// @param[in] prj      задание (нужно для имени файла сигнала)
/// @param[in] p        порт
/// @param[in] timeMax  конец счёта, нс (окно пассивного порта ставится позже него)
/// @param[in] eps      проницаемость среды в сечении входа (сама eps)
/// @return Строка `<INPUT_*> <мода>; from; to; Tmin; Tmax; Amp; Faza;[ eps-1;]`;
///         у пассивного порта Amp = 0; при сигнале из файла Amp = `[имя]`.
/// @see https://www.tamic.ru/docs/theory/ports.html#signal
QString portLine(const Project &prj, const Port &p, double timeMax, double eps)
{
    const TimeWindow w = portWindow(p, timeMax);

    // У пассивного порта глушим и амплитуду, а не только окно: два предохранителя
    // вместо одного. Иначе достаточно кому-то сдвинуть время счёта — и «пассивный»
    // порт заговорит.
    //
    // Сигнал из файла встаёт на место Amp именем в квадратных скобках — так же, как
    // распределение eps у блоков: `INPUT_X ; ylo; yhi; 0; 40; [wg_multi]; 0;`.
    // Расширение `.sig` ядро дописывает само, файл ищет рядом с заданием.
    // ⚠️ Проверка выражения при разборе пропускается ядром только по первому символу
    // '[' — поэтому пробел перед скобкой ставить нельзя.
    const QString amp = !p.active
                            ? QStringLiteral("0")
                            : (p.signal.source == SignalSource::File
                                   ? QStringLiteral("[%1]").arg(signalFileName(prj, p))
                                   : ampExpression(p.signal));

    // Faza обязательна, если задан Amp — иначе парсер ругнётся «Missing separator».
    QString line = QStringLiteral("  %1 %2; %3; %4; %5; %6; %7; %8;")
                       .arg(portKeyword(p), portModeKeyword(p.mode),
                            formatNumber(p.from), formatNumber(p.to),
                            formatCompact(w.tMin), formatCompact(w.tMax),
                            amp, formatCompact(p.active ? p.signal.phase : 0.0));

    // Восьмой параметр — среда в сечении входа. В файл идёт ДОБАВКА (eps-1):
    // ядро складывает её с единицей и отвергает файл, если сумма <= 0.
    // Модель хранит саму eps, минус единицу ставим здесь — ровно один раз и в одном месте.
    // Вакуум (добавка 0) не пишем вовсе: это и есть умолчание ядра.
    if (eps != 1.0)
        line += QStringLiteral(" %1;").arg(formatNumber(eps - 1.0));

    return line;
}

/// Один BLOCK на выходе. Фигура может дать несколько блоков (прямоугольник с разными
/// сторонами), поэтому TOPOLOGY и LINK_LIST строятся из ОДНОГО списка: иначе они
/// рассинхронизируются, а блок без строки T ядро просто не разместит.
struct EmittedBlock {
    /// Номер блока (`BLOCK`).
    int id = 0;
    /// Комментарий к блоку (`! ...`); пусто — без комментария.
    QString comment;
    /// Строки тела блока.
    QStringList body;
    /// Анкер блока для `#LINK_LIST`, мм.
    QPointF anchor;
    /// Анкер выражением — у фигуры, повёрнутой на угол из параметра макроса. Пусто —
    /// пишется числовой \ref anchor.
    QString anchorX, anchorY;
    bool rotating = false; ///< блок поворачиваемой фигуры (для служебной строки свипа)
};
/// @var EmittedBlock::anchorY
/// Координата y анкера выражением (повёрнутая фигура); пусто — пишется `anchor.y()`.

// ---------------------------------------------------------------- угловой свип

/// Градусы в радианы — множитель, которым угол из параметра макроса переводится
/// под `ANGLE_UNIT radian`. Пятнадцати знаков хватает с запасом: ошибка поворота на
/// 1e-15 рад — это доли нанометра на метровой детали.
/// @note Используется в выражениях `cosOf`, `sinOf` и в углах сектора.
const char *const kDegToRad = "0.0174532925199433";

/// `cos(<угол>*pi/180)` и `sin(...)` для угла, записанного выражением.
/// @param[in] angle  угол в градусах (выражение, обычно параметр макроса `Ang__`)
/// @return Выражение `cos((angle)*0.0174532925199433)`.
QString cosOf(const QString &angle)
{
    return QStringLiteral("cos((%1)*%2)").arg(angle, QLatin1String(kDegToRad));
}
/// @brief Возвращает выражение синуса угла, заданного в градусах выражением.
/// @param[in] angle  угол в градусах (выражение)
/// @return Выражение `sin((angle)*0.0174532925199433)`.
QString sinOf(const QString &angle)
{
    return QStringLiteral("sin((%1)*%2)").arg(angle, QLatin1String(kDegToRad));
}

/// Слагаемое `(k)*f`, пропущенное при нулевом k: без него выражения раздувались бы
/// вдвое на каждой точке, лежащей на оси поворота.
/// @param[in] k        коэффициент (мм)
/// @param[in] f        множитель-выражение (`cos(...)`, `sin(...)`)
/// @param[in] leading  true — первое слагаемое, без знака `+`
/// @return Текст слагаемого; пусто при k == 0.
QString term(double k, const QString &f, bool leading)
{
    if (k == 0.0)
        return {};
    return QStringLiteral("%1(%2)*%3")
        .arg(leading ? QString() : QStringLiteral("+"), formatNumber(k), f);
}

/// Координаты точки, повёрнутой вокруг оси на угол `angle` (градусы, выражение).
///   x' = cx + dx*cos - dy*sin,   y' = cy + dx*sin + dy*cos
/// @param[in] pt     точка, мм
/// @param[in] c      ось поворота, мм
/// @param[in] angle  угол в градусах (выражение)
/// @return Выражение x' (мм).
QString rotX(const QPointF &pt, const QPointF &c, const QString &angle)
{
    const double dx = pt.x() - c.x(), dy = pt.y() - c.y();
    QString s = formatNumber(c.x());
    s += term(dx, cosOf(angle), false);
    s += term(-dy, sinOf(angle), false);
    return s;
}
/// @brief Возвращает выражение координаты y точки, повёрнутой вокруг оси.
/// @param[in] pt     точка, мм
/// @param[in] c      ось поворота, мм
/// @param[in] angle  угол в градусах (выражение)
/// @return Выражение y' = cy + dx*sin + dy*cos (мм).
QString rotY(const QPointF &pt, const QPointF &c, const QString &angle)
{
    const double dx = pt.x() - c.x(), dy = pt.y() - c.y();
    QString s = formatNumber(c.y());
    s += term(dx, sinOf(angle), false);
    s += term(dy, cosOf(angle), false);
    return s;
}

/// @brief Строит тело блока-полигона, вершины которого повёрнуты выражениями от угла.
/// @param[in] pts    вершины в порядке записи, мм
/// @param[in] head   заголовок блока (`POLYGON_STAT <значение>;`)
/// @param[in] pivot  ось поворота, мм
/// @param[in] angle  угол в градусах (выражение)
/// @return Заголовок и строки `L <x'>; <y'>;`.
QStringList rotatedPolygonLines(const QVector<QPointF> &pts, const QString &head,
                                const QPointF &pivot, const QString &angle)
{
    QStringList out;
    out << QStringLiteral("  %1").arg(head);
    for (const QPointF &pt : pts)
        out << QStringLiteral("   L %1; %2;").arg(rotX(pt, pivot, angle), rotY(pt, pivot, angle));
    return out;
}

/// Блоки поворачиваемой фигуры. Геометрия — выражениями от угла: шаг описан один раз,
/// а координаты на каждом шаге ядро досчитывает само.
///
/// Что сюда НЕ доходит (запрещает validate()): профиль eps и плазма формулой — их
/// координаты пришлось бы поворачивать внутри чужой формулы; B и Y — ядро понимает их
/// только прямоугольником по осям, а повёрнутый прямоугольник им не выразить;
/// движущиеся блоки.
/// @param[in]     p             задание (ось поворота — `sweepPivot`)
/// @param[in]     s             поворачиваемая фигура
/// @param[in]     angle         угол в градусах (выражение)
/// @param[in,out] freeId        последний занятый номер блока; увеличивается для сторон
///                              прямоугольника
/// @param[in]     withComments  писать комментарии к блокам
/// @return Блоки фигуры с признаком `rotating`; у круга повёрнут анкер (центр) и сектор.
QVector<EmittedBlock> rotatedBlocks(const Project &p, const Shape &s, const QString &angle,
                                    int *freeId, bool withComments)
{
    const QPointF c = sweepPivot(p);
    const QString title = s.name.isEmpty() ? tpl::tr("фигура %1", "shape %1").arg(s.id) : s.name;
    const auto note = [&](const QString &what) {
        return withComments ? tpl::tr("   ! %1 — %2", "   ! %1 -- %2").arg(title, what) : QString();
    };

    // Первый параметр блока — тот же, что у неповёрнутой фигуры.
    QString tag, value;
    const bool plasma = s.plasma.any();
    QString base;
    switch (s.kind) {
    case ShapeKind::Circle: base = QStringLiteral("CIRCLE"); break;
    default:                base = QStringLiteral("POLYGON"); break;
    }
    if (plasma) {
        tag = base + QStringLiteral("_STAT_N");
        value = formatNumber(s.plasma.n);
    } else {
        tag = base + QStringLiteral("_STAT");
        value = fillExpression(s);
    }
    const QString head = QStringLiteral("%1 %2;").arg(tag, value);
    const QString what = tpl::tr("поворачивается вместе с объектом",
                                 "rotates together with the object");

    QVector<EmittedBlock> out;
    if (s.kind == ShapeKind::Circle) {
        EmittedBlock b;
        b.id = s.id;
        b.rotating = true;
        b.comment = note(what);
        const bool full = (s.fiMax - s.fiMin) >= 2 * 3.14159265358979323846 - 1e-9;
        const QString fi0 = full ? formatNumber(s.fiMin)
                                 : QStringLiteral("%1+(%2)*%3").arg(formatNumber(s.fiMin), angle,
                                                                    QLatin1String(kDegToRad));
        const QString fi1 = full ? formatNumber(s.fiMax)
                                 : QStringLiteral("%1+(%2)*%3").arg(formatNumber(s.fiMax), angle,
                                                                    QLatin1String(kDegToRad));
        b.body << QStringLiteral("  %1 %2; %3; %4; %5;")
                      .arg(head, formatNumber(s.rMin), formatNumber(s.rMax), fi0, fi1);
        // Центр окружности — это анкер, он и поворачивается.
        b.anchorX = rotX(s.center, c, angle);
        b.anchorY = rotY(s.center, c, angle);
        out << b;
        return out;
    }

    if (s.kind == ShapeKind::Rectangle && !s.hasUniformEdges()) {
        // Стороны разного материала — каждая своей ломаной, как и без поворота.
        bool first = true;
        for (int e = 0; e < 4; ++e) {
            if (!s.edges[size_t(e)].present)
                continue;
            QVector<QPointF> pts{s.edgeFrom(e), s.edgeTo(e)};
            padToThreePoints(&pts);
            EmittedBlock b;
            b.id = first ? s.id : ++*freeId;
            first = false;
            b.rotating = true;
            b.comment = note(QStringLiteral("%1, %2; %3")
                                 .arg(edgeName(e), materialName(s.edgeMaterial(e)), what));
            b.body = rotatedPolygonLines(
                pts, QStringLiteral("POLYGON_STAT %1;").arg(materialKeyword(s.edgeMaterial(e))), c,
                angle);
            out << b;
        }
        return out;
    }

    // Прямоугольник целиком — тот же полигон, что получился бы из его углов.
    Shape poly = s;
    if (s.kind == ShapeKind::Rectangle) {
        poly.kind = ShapeKind::Polygon;
        poly.points.clear();
        for (int e = 0; e < 4; ++e)
            poly.points << s.edgeFrom(e);
    }
    EmittedBlock b;
    b.id = s.id;
    b.rotating = true;
    b.comment = note(what);
    // Порядок обхода и замыкание — те же правила, что у неповёрнутого полигона:
    // поворот обход не меняет, так что выверенный порядок остаётся верным.
    b.body = rotatedPolygonLines(polygonPoints(poly), head, c, angle);
    out << b;
    return out;
}

/// @brief Собирает все блоки одного шага: фигуры (с плазмой, профилем eps, сторонами)
///        и входы.
///
/// Общий список блоков — источник и для `#TOPOLOGY`, и для `#LINK_LIST`, поэтому они
/// не расходятся. Номера для дополнительных блоков (стороны, `_N/_B/_Y/_S`) берутся
/// после наибольшего занятого номера.
/// @param[in] p             задание
/// @param[in] withComments  писать комментарии к блокам
/// @param[in] variant       вариант геометрии; nullptr — все фигуры
/// @param[in] exciter       номер возбуждаемого порта; -1 — активные по флагу `Port::active`
/// @param[in] createEps     номера фигур, у которых на этом шаге писать `[имя]` (сохранить `.eps`)
/// @param[in] rotAngle      угол свипа выражением; пусто — без поворота
/// @param[in] dropRotating  true — пропустить поворачиваемые фигуры (опорный шаг свипа)
/// @return Блоки шага в порядке записи.
QVector<EmittedBlock> buildBlocks(const Project &p, bool withComments,
                                  const Variant *variant, int exciter,
                                  const QSet<int> &createEps,
                                  const QString &rotAngle = QString(),
                                  bool dropRotating = false)
{
    // Разбиение прямоугольника на стороны требует дополнительных номеров.
    // Номера произвольные — важно лишь, чтобы не столкнулись с занятыми.
    int freeId = 0;
    for (const Shape &s : p.shapes)
        freeId = std::max(freeId, s.id);
    for (const Port &q : p.ports)
        freeId = std::max(freeId, q.id);

    const auto note = [withComments](const QString &s) { return withComments ? s : QString(); };

    QVector<EmittedBlock> out;
    for (const Shape &s : p.shapes) {
        // Выключенную вариантом фигуру просто НЕ пишем: каждый #STEP несёт свою
        // топологию, комментировать блоки не нужно.
        if (variant && !variant->isEnabled(s.id))
            continue;
        // Угловой свип: в опорном шаге объекта нет вовсе, в остальных он повёрнут.
        if (p.sweep.rotates(s.id)) {
            if (dropRotating)
                continue;
            if (!rotAngle.isEmpty()) {
                out += rotatedBlocks(p, s, rotAngle, &freeId, withComments);
                continue;
            }
        }
        const QString title = s.name.isEmpty() ? tpl::tr("фигура %1",
                                                         "shape %1").arg(s.id) : s.name;

        // --- неоднородная eps: формула считается ядром в каждом узле блока ---
        //
        // Файл профиля сохраняется один раз, на первом шаге, где фигура появляется:
        // он частотно-независим (в него уходит `вес узла * выражение`, а частотный
        // коэффициент ядро применяет само и на каждом шаге заново —
        // `TmcRTHRectNode.cpp:1052`), так что переписывать его на каждой частоте
        // нечем и незачем.
        if (s.hasEpsProfile()) {
            const bool save = createEps.contains(s.id);
            EmittedBlock b;
            b.id = s.id;
            QString what;
            if (s.epsProfile.fromFile) {
                what = tpl::tr("   ! %1 — распределение eps читается из %2.eps",
                               "! %1 -- eps distribution is read from %2.eps")
                           .arg(title, epsProfileFile(s));
            } else if (save) {
                what = tpl::tr("   ! %1 — профиль eps по формуле; "
                               "результат сохраняется в %2.eps",
                               "! %1 -- eps profile by formula; the result is saved to %2.eps")
                           .arg(title, epsProfileFile(s));
            } else if (s.epsProfile.savesFile()) {
                what = tpl::tr("   ! %1 — тот же профиль eps; файл уже записан "
                               "на первом шаге",
                               "! %1 -- the same eps profile; the file was already written at the "
                               "first step").arg(title);
            } else {
                what = tpl::tr("   ! %1 — профиль eps по формуле",
                               "! %1 -- eps profile by formula").arg(title);
            }
            b.comment = note(what);
            b.body = epsProfileLines(s, save);
            b.anchor = s.epsOrigin(); // от него ядро отсчитывает x и y в формуле
            out << b;
            continue;
        }

        // --- плазма: одна фигура даёт до ЧЕТЫРЁХ блоков с одной и той же геометрией ---
        //
        // Языку иначе никак: N, B, Y и σ — разные примитивы (`_N`, `_B`, `_Y`, `_S`), и
        // «положить их в один блок» нечем. Номер блока произвольный, важна только
        // уникальность, поэтому первый параметр забирает номер фигуры, а остальные
        // получают свободные — ровно как при разборе прямоугольника на стороны.
        if (s.plasma.any()) {
            double xLo = 0, xHi = 0, yLo = 0, yHi = 0;
            s.bounds(&xLo, &xHi, &yLo, &yHi);

            bool firstSlot = true;
            const auto addSlot = [&](const QStringList &body, const QPointF &anchor,
                                     const QString &what) {
                EmittedBlock b;
                b.id = firstSlot ? s.id : ++freeId;
                firstSlot = false;
                b.comment = note(tpl::tr("   ! %1 — %2", "   ! %1 -- %2").arg(title, what));
                b.body = body;
                b.anchor = anchor;
                out << b;
            };

            // Величина задана формулой — пишем её саму. Ядро считает первый параметр
            // блока в каждом узле, поэтому профиль плотности так и живёт в задании;
            // редактор обязан вернуть его в файл дословно.
            const auto plasmaValue = [](const QString &formula, double number) {
                return formula.trimmed().isEmpty() ? formatNumber(number)
                                                   : plainLiterals(formula.trimmed());
            };

            // Анкер блока — это одновременно и начало отсчёта x, y в формуле, и сдвиг
            // всей геометрии блока. Поэтому, ставя анкер профиля, геометрию надо
            // пересчитать в его систему — иначе фигура уедет ровно на анкер. У круга
            // сдвигать нечего: в блоке лежат радиусы и углы, а центр и так в анкере.
            const bool shift = s.plasma.customOrigin && s.kind != ShapeKind::Circle;
            const QPointF o = shift ? s.plasma.origin : QPointF(0.0, 0.0);
            Shape local = s;
            if (shift) {
                local.xMin -= o.x(); local.xMax -= o.x();
                local.yMin -= o.y(); local.yMax -= o.y();
                for (QPointF &pt : local.points)
                    pt -= o;
            }
            const auto plasmaAnchor = [&](const QPointF &fallback) {
                return shift ? o : fallback;
            };

            // Величина из готового файла: `FILE_N имя; x1; x2; y1; y2;` с теми же
            // границами, что у записавшего его блока `RECT_STAT_N [имя]...`. Сетка лежит
            // в самом файле, на место её ставит анкер — поэтому он тот же, что у блока
            // величины (tpl-guide 5.7.4). Читает такие блоки только ядро X с 26.10.2026.
            const auto fileSlot = [&](const QString &tag, const QString &file,
                                      const QString &what) {
                addSlot(rectBlockLines(tag, file.trimmed(), xLo - o.x(), xHi - o.x(),
                                       yLo - o.y(), yHi - o.y()),
                        plasmaAnchor({0.0, 0.0}),
                        tpl::tr("%1 читается из файла %2.eps",
                                "%1 is read from file %2.eps").arg(what, file.trimmed()));
            };

            if (s.plasma.hasN) {
                // Значение числовое и БЕЗ поправки «минус единица»: она только у eps.
                //
                // ⚠️ Концентрация и добавка eps — это ОДНА величина, записанная двумя
                // способами: ядро переводит N в добавку само,
                // `d_EpsAdd -= N*e^2/(m*eps0*(2*pi*f)^2)` (X-мода,
                // `kernels/PlanarRT_X/TmcRTHRectNode.cpp:1105`). Поэтому N пишется в СИ
                // и один раз, а делить её на частоту не нужно: ядро делает это на каждом
                // шаге заново.
                if (!s.plasma.nFile.trimmed().isEmpty()) {
                    fileSlot(QStringLiteral("FILE_N"), s.plasma.nFile, tpl::tr("концентрация N",
                                                                               "density N"));
                } else {
                    const bool profile = !s.plasma.nFormula.trimmed().isEmpty();
                    addSlot(geometryLines(local, shapeKeyword(s) + QStringLiteral("_N"),
                                          plasmaValue(s.plasma.nFormula, s.plasma.n)),
                            plasmaAnchor(shapeAnchor(s)),
                            profile ? tpl::tr("концентрация N задана формулой от координат",
                                              "density N is given by a formula of the coordinates")
                                    : tpl::tr("концентрация N = %1 м^-3", "density N = %1 m^-3")
                                          .arg(formatCompact(s.plasma.n)));
                }
            }

            // B — прямоугольником ШИРЕ плазмы (см. plasmaBRect). На границе блока ядро
            // ополовинивает значение, и B, совпадающее с плазмой, давало бы на её краю
            // ложный резонансный слой (Б-21). Вне N поле B ни на что не влияет: плазменная
            // ветка узла включается только при N != 0, поэтому лишняя площадь инертна.
            // Прямоугольником, а не формой фигуры: расширить круг или полигон «на два
            // шага» без потери формы нечем, а больше формы B и не нужно.
            if (s.plasma.hasB) {
                if (!s.plasma.bFile.trimmed().isEmpty()) {
                    fileSlot(QStringLiteral("FILE_B"), s.plasma.bFile,
                             tpl::tr("подмагничивание B", "magnetization B"));
                } else {
                    bool wide = false;
                    const RectMm r = plasmaBRect(p, s, &wide);
                    const bool profile = !s.plasma.bFormula.trimmed().isEmpty();
                    QString what = profile ? tpl::tr("подмагничивание B задано формулой "
                                                     "от координат",
                                                     "magnetization B is given by a formula of "
                                                     "the coordinates")
                                           : tpl::tr("подмагничивание B = %1 Тл",
                                                     "magnetization B = %1 T")
                                                 .arg(formatCompact(s.plasma.b));
                    if (wide)
                        what += tpl::tr(", блок шире плазмы на %1 шага",
                                        ", block wider than the plasma by %1 steps").arg(kBMarginCells);
                    else if (s.plasma.bExact)
                        what += tpl::tr(", блок ровно по плазме",
                                        ", block exactly over the plasma");
                    addSlot(rectBlockLines(QStringLiteral("RECT_STAT_B"),
                                           plasmaValue(s.plasma.bFormula, s.plasma.b),
                                           r.xLo - o.x(), r.xHi - o.x(), r.yLo - o.y(),
                                           r.yHi - o.y()),
                            plasmaAnchor({0.0, 0.0}), what);
                }
            }

            // Y и σ — формой самой фигуры: с 26.10.2026 `CIRCLE_STAT_Y`, `POLYGON_STAT_Y`
            // и все `*_STAT_S` ядро читает в свои величины (раньше круглые и
            // многоугольные B и Y молча уходили в N). Габаритом их писать нельзя: потери
            // действуют и там, где плазмы нет.
            if (s.plasma.hasY) {
                if (!s.plasma.yFile.trimmed().isEmpty()) {
                    fileSlot(QStringLiteral("FILE_Y"), s.plasma.yFile, tpl::tr("параметр Y",
                                                                               "parameter Y"));
                } else {
                    const bool profile = !s.plasma.yFormula.trimmed().isEmpty();
                    addSlot(geometryLines(local, shapeKeyword(s) + QStringLiteral("_Y"),
                                          plasmaValue(s.plasma.yFormula, s.plasma.y)),
                            plasmaAnchor(shapeAnchor(s)),
                            profile ? tpl::tr("параметр Y задан формулой от координат",
                                              "parameter Y is given by a formula of the "
                                              "coordinates")
                                    : tpl::tr("параметр Y = %1 (сеточные потери)",
                                              "parameter Y = %1 (grid losses)")
                                          .arg(formatCompact(s.plasma.y)));
                }
            }
            if (s.plasma.hasS) {
                const bool profile = !s.plasma.sFormula.trimmed().isEmpty();
                addSlot(geometryLines(local, shapeKeyword(s) + QStringLiteral("_S"),
                                      plasmaValue(s.plasma.sFormula, s.plasma.s)),
                        plasmaAnchor(shapeAnchor(s)),
                        profile ? tpl::tr("проводимость sigma задана формулой от координат",
                                          "conductivity sigma is given by a formula of the "
                                          "coordinates")
                                : tpl::tr("проводимость sigma = %1 См/м",
                                          "conductivity sigma = %1 S/m")
                                      .arg(formatCompact(s.plasma.s)));
            }
            continue;
        }

        if (s.kind == ShapeKind::Rectangle && !s.hasUniformEdges()) {
            bool firstEdge = true;
            for (int e = 0; e < 4; ++e) {
                if (!s.edges[size_t(e)].present)
                    continue;
                EmittedBlock b;
                b.id = firstEdge ? s.id : ++freeId;
                firstEdge = false;
                b.comment = note(tpl::tr("   ! %1 — %2, %3", "   ! %1 -- %2, %3")
                                     .arg(title, edgeName(e),
                                          materialName(s.edgeMaterial(e))));
                b.body = edgeLines(s, e);
                out << b; // анкер нулевой: координаты сторон абсолютные
            }
            continue;
        }

        EmittedBlock b;
        b.id = s.id;
        b.comment = note(s.name.isEmpty() ? QString() : QStringLiteral("   ! %1").arg(s.name));
        b.body = shapeLines(s);
        b.anchor = shapeAnchor(s);
        out << b;
    }

    for (const Port &q0 : p.ports) {
        // Возбудитель шага задаёт, какой порт активен. Остальные глушим — иначе
        // .s перестанет быть матрицей рассеяния.
        Port q = q0;
        if (exciter >= 0)
            q.active = (q.id == exciter);

        EmittedBlock b;
        b.id = q.id;
        b.comment = note(tpl::tr("   ! %1 — %2", "   ! %1 -- %2")
                             .arg(q.name.isEmpty() ? tpl::tr("порт", "port") : q.name,
                                  q.active ? tpl::tr("АКТИВНЫЙ", "ACTIVE")
                                           : tpl::tr("пассивный", "passive")));
        b.body = QStringList{portLine(p, q, p.timeMax, portEps(p, q, variant))};
        b.anchor = portAnchor(q, p.box);
        out << b;
    }
    return out;
}

/// Имя файла сигналов для конкретного шага. У каждого возбуждения свой `.t`
/// (как в авторских заданиях: wg_p1.t, wg_p2.t), у каждого варианта — свой набор.
/// @param[in] p        задание
/// @param[in] v        вариант геометрии (его `tag` добавляется через `_`); nullptr — нет
/// @param[in] exciter  номер возбуждаемого порта (добавка `_p<порядковый номер>`); -1 — нет
/// @param[in] freq     частота шага, ГГц (добавка `freqFileTag` при нескольких частотах
///                     вне углового свипа); <= 0 — без метки
/// @return Имя файла сигналов без расширения для строки `FILE` в `#OUTPUT`.
QString signalFileFor(const Project &p, const Variant *v, int exciter, double freq)
{
    QString s = p.output.signalFile;
    if (v && !v->tag.isEmpty())
        s += QLatin1Char('_') + v->tag;
    if (exciter >= 0)
        s += QStringLiteral("_p%1").arg(p.portOrdinal(exciter));
    // ⚠️ Имя файла сигналов у шага ОДНО, а шагов в свипе много: без метки частоты
    // каждый следующий шаг переписывал бы `.t` предыдущего, и от всего свипа
    // оставался бы сигнал последней частоты. Матрица `.s` этого не показывает (она
    // дописывается), а вот широкополосной постобработке и взгляду на сигнал во
    // времени нужен `.t` КАЖДОГО шага. У углового свипа метка своя — угол.
    if (freq > 0.0 && p.freqs.size() > 1 && !p.sweep.active())
        s += QLatin1Char('_') + freqFileTag(freq);
    return s;
}

/// Имя файла S-матрицы. ОБЩЕЕ для всех возбуждений одного варианта: шаги дописывают
/// в него свои строки, и в конце там полная матрица. У разных вариантов — разные
/// файлы: это разные устройства, мешать их нельзя.
/// @param[in] p  задание
/// @param[in] v  вариант геометрии; nullptr — без добавки
/// @return Имя файла `.s` без расширения.
QString sMatrixFileFor(const Project &p, const Variant *v)
{
    QString s = p.output.sMatrixFile;
    if (v && !v->tag.isEmpty())
        s += QLatin1Char('_') + v->tag;
    return s;
}

/// Файл `.s` шага углового свипа: у каждого угла свой (`<имя>_a30`).
///
/// Подписать точку углом в общем файле ядро с 26.10.2026 не даёт: подпись SMATRIX в
/// одночастотном режиме обязана совпадать с FREQ (Б-3.2). А одинаковые подписи всех
/// углов в одном файле слились бы в одну точку.
/// @param[in] p      задание
/// @param[in] angle  угол шага, градусы
/// @return Имя `<sMatrixFile>_<sweepFileTag(angle)>` без расширения.
QString sweepSFile(const Project &p, double angle)
{
    return p.output.sMatrixFile + QLatin1Char('_') + sweepFileTag(angle);
}

/// Все файлы `.s`, которые появятся у задания, без расширения.
///
/// Имён может быть несколько — по одному на вариант геометрии и опорный шаг свипа.
/// Назвать базовое имя, которого на диске нет, значит отправить человека удалять не
/// тот файл. Одно место на шапку задания и на пакетный файл конвейера: иначе они
/// рано или поздно стали бы называть разное.
/// @param[in] p  задание
/// @return Имена без повторов; пусто, если S-матрица не снимается.
QStringList sMatrixNames(const Project &p)
{
    QStringList names;
    if (!p.output.sMatrix || p.output.sMatrixFile.isEmpty())
        return names;
    if (p.sweep.active()) {
        if (p.sweep.referenceStep)
            names << p.output.sMatrixFile + QStringLiteral("_ref");
        for (double a : p.sweep.angles) {
            const QString n = sweepSFile(p, a);
            if (!names.contains(n))
                names << n;
        }
    } else if (p.variants.isEmpty()) {
        names << p.output.sMatrixFile;
    } else {
        for (const Variant &v : p.variants) {
            const QString n = sMatrixFileFor(p, &v);
            if (!names.contains(n))
                names << n;
        }
    }
    return names;
}

/// @brief Возвращает имя файлов ДН для шага (добавки — как у `signalFileFor`).
/// @param[in] p        задание
/// @param[in] v        вариант геометрии; nullptr — без добавки
/// @param[in] exciter  номер возбуждаемого порта; -1 — без добавки
/// @param[in] freq     частота шага, ГГц; <= 0 — без метки частоты
/// @return Имя без расширения для строки `FIELD_DISTRIBUTION_M`.
QString patternFileFor(const Project &p, const Variant *v, int exciter, double freq)
{
    QString s = p.output.patternFile;
    if (v && !v->tag.isEmpty())
        s += QLatin1Char('_') + v->tag;
    if (exciter >= 0)
        s += QStringLiteral("_p%1").arg(p.portOrdinal(exciter));
    // ДН тоже своя на каждой частоте — и по той же причине, что и файл сигналов.
    if (freq > 0.0 && p.freqs.size() > 1 && !p.sweep.active())
        s += QLatin1Char('_') + freqFileTag(freq);
    return s;
}

/// @brief Параметры одного шага `#STEP`, которые отличаются от шага к шагу.
///
/// Заполняется `argsFor`, `sweepReferenceArgs`, `sweepAngleArgs` (числами) или при
/// построении тела макроса (параметрами макроса вместо чисел).
struct StepArgs {
    /// Частота шага текстом: число (`formatFreq`) или параметр макроса.
    QString freq;
    /// Имя файла сигналов (`FILE` в `#OUTPUT`).
    QString signalFile;
    /// Имя файла S-матрицы (`SMATRIX`).
    QString sFile;
    /// Имя файлов ДН (`FIELD_DISTRIBUTION_M`).
    QString patternFile;
    /// Писать комментарии к блокам.
    bool withComments = true;
    /// Писать `TOPOLOGY;` (топология для FieldView).
    bool withTopology = false;
    /// Писать `FIELDS;` (мгновенные поля на каждом такте — дорого).
    bool withFields = false;
    const Variant *variant = nullptr; ///< какие фигуры включены; nullptr — все
    int exciter = -1;                 ///< какой порт возбуждается; -1 — как помечено

    /// Фигуры, чей `.eps` создаётся ИМЕННО на этом шаге. Остальные читают готовый файл.
    QSet<int> createEps;

    // --- угловой свип ---
    QString rotAngle;          ///< угол поворота объекта (выражение); пусто — не поворачивать
    bool dropRotating = false; ///< опорный шаг: объект не пишется вовсе
};

/// Фигуры, чей профиль надо СОХРАНИТЬ в файл на этом шаге.
///
/// Выключенной вариантом фигуры в шаге нет вовсе — значит и файл её профиля здесь не
/// появится. Круг и полигон сюда не попадают никогда: писать файл умеет только
/// `SetRect`, а формула у них считается и без файла.
/// @param[in] p        задание
/// @param[in] variant  вариант геометрии; nullptr — все фигуры
/// @return Номера прямоугольников, у которых профиль просит записать `.eps`.
QSet<int> epsProfileShapes(const Project &p, const Variant *variant)
{
    QSet<int> out;
    for (const Shape &s : p.shapes) {
        if (variant && !variant->isEnabled(s.id))
            continue;
        if (s.epsProfile.savesFile() && s.kind == ShapeKind::Rectangle)
            out << s.id;
    }
    return out;
}

/// Многочастотный ли шаг: возбуждается вход с файлом сигнала `[имя]`. Так решает и ядро —
/// SMATRIX такого шага оно считает по всей полосе сигнала.
/// @param[in] p        задание
/// @param[in] exciter  номер возбуждаемого порта; -1 — активные по флагу `Port::active`
/// @return true — хотя бы один активный порт шага берёт сигнал из файла.
/// @see https://www.tamic.ru/docs/theory/smatrix.html#multifreq
bool stepIsWideband(const Project &p, int exciter)
{
    for (const Port &q : p.ports) {
        const bool active = exciter >= 0 ? q.id == exciter : q.active;
        if (active && q.signal.source == SignalSource::File)
            return true;
    }
    return false;
}

/// Пишется ли окно T_min; T_max у SMATRIX хоть в одном шаге задания.
/// @param[in] p  задание
/// @return true — окно ограничено галочкой либо есть одночастотный шаг.
bool sMatrixWindowWritten(const Project &p)
{
    if (p.output.sMatrixLimitWideband)
        return true;
    if (p.exciters.isEmpty())
        return !stepIsWideband(p, -1);
    for (int id : p.exciters) {
        if (!stepIsWideband(p, id))
            return true;
    }
    return false;
}

/// @brief Строит текст одного шага: `#PARM`, `#TOPOLOGY`, `#LINK_LIST`, `#OUTPUT`.
///
/// Единицы в `#PARM` всегда одни: радианы, ГГц, мм, нс. Блоки и строки `T` берутся
/// из одного списка `buildBlocks`. `SMATRIX` подписывается частотой шага; окно
/// `T_min; T_max` пишется всегда, кроме многочастотного шага без ограничения окна.
/// ДН — только вариантом `FIELD_DISTRIBUTION_M`.
/// @param[in] p  задание
/// @param[in] a  параметры шага
/// @return Строки шага от `#STEP` до `#END_STEP` (без перевода строки в конце).
QStringList stepLines(const Project &p, const StepArgs &a)
{
    QStringList out;
    out << QStringLiteral("#STEP")
        << QStringLiteral("#PARM")
        << QStringLiteral(" ANGLE_UNIT radian;")
        << QStringLiteral(" FREQ_UNIT  GHz;")
        << QStringLiteral(" LONG_UNIT  mm;")
        << QStringLiteral(" TIME_UNIT  ns;")
        << QStringLiteral(" DELTA  %1;").arg(formatNumber(p.delta))
        << QStringLiteral(" TIME   0.; %1;").arg(formatNumber(p.timeMax))
        << QStringLiteral(" X_MIN  %1;").arg(formatNumber(p.box.xMin))
        << QStringLiteral(" X_MAX  %1;").arg(formatNumber(p.box.xMax))
        << QStringLiteral(" Y_MIN  %1;").arg(formatNumber(p.box.yMin))
        << QStringLiteral(" Y_MAX  %1;").arg(formatNumber(p.box.yMax))
        << QStringLiteral(" FREQ   %1;").arg(a.freq)
        << QStringLiteral("#END_PARM");

    const QVector<EmittedBlock> blocks = buildBlocks(p, a.withComments, a.variant, a.exciter,
                                                     a.createEps, a.rotAngle, a.dropRotating);

    out << QStringLiteral("#TOPOLOGY");
    for (const EmittedBlock &b : blocks) {
        out << QStringLiteral(" BLOCK %1;").arg(b.id) + b.comment;
        out += b.body;
        out << QStringLiteral(" END_B");
    }
    out << QStringLiteral("#END_TOPOLOGY");

    // Список строится из ТОГО ЖЕ списка блоков: блок без T не разместится, T без блока — ошибка.
    out << QStringLiteral("#LINK_LIST");
    for (const EmittedBlock &b : blocks) {
        out << QStringLiteral(" T %1; %2; %3;")
                   .arg(b.id)
                   .arg(b.anchorX.isEmpty() ? formatNumber(b.anchor.x()) : b.anchorX,
                        b.anchorY.isEmpty() ? formatNumber(b.anchor.y()) : b.anchorY);
    }
    out << QStringLiteral("#END_LINK");

    out << QStringLiteral("#OUTPUT")
        << QStringLiteral(" FILE %1;").arg(a.signalFile); // обязана быть первой
    if (a.withTopology)
        out << QStringLiteral(" TOPOLOGY;");
    if (a.withFields)
        out << QStringLiteral(" FIELDS;");
    // Частота SMATRIX — ВСЕГДА частота шага. Второй параметр частоту счёта не меняет, и
    // раньше угловой свип подписывал им точку углом, но с 26.10.2026 ядро в
    // одночастотном режиме отвергает шаг, если подпись не равна FREQ (Б-3.2).
    //
    // Окно: в одночастотном режиме обязательно (без него — тоже ошибка разбора), в
    // многочастотном (вход с файлом сигнала) по умолчанию его нет — это полное окно
    // 0...TIME (Б-22). Ограничить окно там можно галочкой.
    if (p.output.sMatrix) {
        if (stepIsWideband(p, a.exciter) && !p.output.sMatrixLimitWideband) {
            out << QStringLiteral(" SMATRIX %1; %2;").arg(a.sFile, a.freq);
        } else {
            out << QStringLiteral(" SMATRIX %1; %2; %3; %4;")
                       .arg(a.sFile, a.freq, formatNumber(p.output.sMatrixTMin),
                            formatNumber(p.output.sMatrixTMax));
        }
    }
    if (p.output.directionalPattern) {
        // Только вариант _M: без него ядро дёргает диск на каждый узел на каждом такте.
        out << QStringLiteral(" FIELD_DISTRIBUTION_M %1; %2; %3; %4;")
                   .arg(a.patternFile, a.freq, formatNumber(p.output.patternTMin),
                        formatNumber(p.output.patternTMax));
    }
    out << QStringLiteral("#END_OUTPUT")
        << QStringLiteral("#END_STEP");

    return out;
}

/// Пояснение и служебная строка углового свипа в шапке.
///
/// Служебная строка нужна не ядру (для него это комментарий), а нам: прочитав
/// задание обратно, по ней восстанавливается сам свип — какие блоки поворачиваются,
/// вокруг чего и на какие углы. Без неё повёрнутая фигура на каждом шаге выглядела бы
/// отдельной фигурой, и открытое задание рассыпалось бы на десятки копий объекта.
/// @param[in] p  задание с действующим угловым свипом
/// @return Строки шапки с `\n`: пояснение и служебная строка
///         `! @TPLD-SWEEP blocks=...; pivot=x,y; angles=...; ref=0|1`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#sweep
QString sweepHeader(const Project &p)
{
    QStringList blocks;
    for (const EmittedBlock &b : buildBlocks(p, false, nullptr, -1, {}, QStringLiteral("0"))) {
        if (b.rotating)
            blocks << QString::number(b.id);
    }
    QStringList angles;
    for (double a : p.sweep.angles)
        angles << formatCompact(a);

    QString h;
    h += tpl::tr("! Поворот объекта по шагам: %1 углов, ось (%2; %3) мм.%4\n",
                 "! Object rotation over the steps: %1 angles, axis (%2; %3) mm.%4\n")
             .arg(p.sweep.angles.size())
             .arg(formatCompact(sweepPivot(p).x()), formatCompact(sweepPivot(p).y()),
                  p.sweep.referenceStep
                      ? tpl::tr(" Первый шаг -- без объекта (опора).",
                                "The first step -- without the object (reference).")
                      : QString());
    h += tpl::tr("! У каждого угла свой файл .s (<имя>_a30.s), точка подписана частотой FREQ.\n",
                 "! Each angle has its own .s file (<name>_a30.s), the point is labelled with the "
                 "frequency FREQ.\n");
    h += QStringLiteral("! %1 blocks=%2; pivot=%3,%4; angles=%5; ref=%6\n")
             .arg(QLatin1String(kSweepMarker), blocks.join(QLatin1Char(',')),
                  formatCompact(sweepPivot(p).x()), formatCompact(sweepPivot(p).y()),
                  angles.join(QLatin1Char(',')),
                  p.sweep.referenceStep ? QStringLiteral("1") : QStringLiteral("0"));
    return h;
}

/// Пояснение к широкополосному сигналу в шапке задания.
///
/// Без него файл выглядит противоречиво: `FREQ` одна, а расчёт на всю полосу. Человек
/// первым делом попробует «исправить» частоту — и сломает согласование порта.
/// @param[in] p  задание
/// @return Строки шапки с `\n`; пусто, если ни один порт не берёт сигнал из файла.
QString signalHeader(const Project &p)
{
    QStringList lines;
    for (const Port &port : p.ports) {
        if (!portUsesSignalFile(p, port))
            continue;
        const Waveshape &w = port.signal.wave;
        lines << tpl::tr("! Порт «%1»: падающая волна из файла %2.sig",
                         "! Port \"%1\": incident wave from file %2.sig")
                     .arg(port.name, signalFileName(p, port));
        lines << QStringLiteral("!   %1 -- %2").arg(waveformName(w.kind), waveshapeSummary(w));
    }
    if (lines.isEmpty())
        return QString();

    QString h;
    h += QStringLiteral("!\n");
    h += tpl::tr("! Широкополосный расчёт: имя файла стоит на месте параметра Amp\n",
                 "! Wideband computation: the file name stands in place of the Amp parameter\n");
    h += tpl::tr("! в квадратных скобках, и файл задаёт ВСЮ падающую волну a(t).\n",
                 "! in square brackets, and the file defines the WHOLE incident wave a(t).\n");
    h += tpl::tr("! Faza такому входу не нужна, а FREQ по-прежнему нужна: по ней\n",
                 "! Such a port needs no Faza, but FREQ is still needed: the kernel uses it\n");
    h += tpl::tr("! ядро согласует порт, поэтому она стоит в ЦЕНТРЕ полосы.\n",
                 "! to match the port, so it is set to the CENTRE of the band.\n");
    const Band band = projectBand(p);
    if (band.known) {
        h += tpl::tr("! Полоса %1...%2 ГГц, центр %3 ГГц.\n",
                     "! Band %1...%2 GHz, centre %3 GHz.\n")
                 .arg(formatCompact(band.lo), formatCompact(band.hi),
                      formatCompact(band.center()));
    }
    h += lines.join(QLatin1Char('\n')) + QLatin1Char('\n');
    h += tpl::tr("! SMATRIX такого шага ядро считает по всей полосе сигнала\n",
                 "! The kernel computes SMATRIX of such a step over the whole signal band\n");
    h += tpl::tr("! (многочастотная обработка); строка без окна T_min; T_max --\n",
                 "! (multifrequency processing); a line without the T_min; T_max window --\n");
    h += tpl::tr("! это окно на весь счёт, 0...TIME.\n",
                 "! is a window over the whole run, 0...TIME.\n");
    return h;
}

/// @brief Строит шапку задания: `#TMC_RT_H`, описание, имя ядра без расширения,
///        напоминание удалить файлы `.s` перед прогоном, пояснения свипа и сигнала.
/// @param[in] p  задание
/// @return Текст шапки с `\n` после каждой строки.
QString header(const Project &p)
{
    QString h = QStringLiteral("#TMC_RT_H\n"); // всегда, даже для X-моды: тега #TMC_RT_X нет
    h += tpl::tr("! Задание сгенерировано TPL Designer.\n", "! Task generated by TPL Designer.\n");
    if (!p.description.isEmpty()) {
        for (const QString &line : p.description.split(QLatin1Char('\n')))
            h += QStringLiteral("! %1\n").arg(line);
    }
    // ⚠️ Имя ядра БЕЗ расширения — намеренно (2026-09-06). Один и тот же проект обязан
    // давать байт в байт один и тот же .tpl на Windows, macOS и Linux, а исполняемый
    // файл там называется по-разному (`tmc_rth.exe`, `PlanarRT_H.app`, `planarrt_h`).
    // Подставь мы сюда имя своей системы — задания разошлись бы текстом, и сверка с
    // эталоном перестала бы работать ровно там, где она нужнее всего.
    h += tpl::tr("! Ядро: %1\n", "! Kernel: %1\n")
             .arg(p.kernel == Kernel::X ? tpl::tr("tmc_rtx (X-мода)", "tmc_rtx (X-mode)")
                                        : tpl::tr("tmc_rth (H-мода)", "tmc_rth (H-mode)"));
    if (p.output.sMatrix && !p.output.sMatrixFile.isEmpty()) {
        QStringList names = sMatrixNames(p);
        for (QString &n : names)
            n += QStringLiteral(".s");
        // У свипа на десятки углов перечислять все файлы в шапке незачем: они названы
        // по одному образцу, его и пишем.
        if (p.sweep.active() && names.size() > 3) {
            names.clear();
            if (p.sweep.referenceStep)
                names << p.output.sMatrixFile + QStringLiteral("_ref.s");
            names << p.output.sMatrixFile + QStringLiteral("_a*.s");
        }
        h += tpl::tr("! Перед прогоном удалить %1 -- дописывается, а не "
                     "перезаписывается.\n",
                     "! Delete %1 before the run -- it is appended to, not overwritten.\n")
                 .arg(names.join(tpl::tr(" и ", " and ")));
    }
    if (p.sweep.active())
        h += sweepHeader(p);
    h += signalHeader(p);
    return h;
}

} // namespace

QString formatCompact(double v)
{
    // Десять значащих цифр, а не шесть (умолчание %g). Шести хватает ядру, но не
    // круговороту «открыл готовый файл -> сохранил»: координата 22.109638 писалась
    // как 22.1096, и геометрия ползла на каждом сохранении. Для миллиметров десять
    // цифр — это доли нанометра, файл при этом не раздувается: у круглых чисел
    // лишних знаков не появляется вовсе.
    //
    // ⚠️ И НИКОГДА не экспонентой. Вычислитель выражений ядра (`libs/Expr`,
    // `i1nte_atof_1`) записи 1e+17 не понимает: «1» — число, «e» — неизвестная
    // функция, шаг останавливается на «abnormal function». Через этот вычислитель идёт
    // всё, что ядро считает в узлах: `CIRCLE_STAT_N`, `POLYGON_STAT_N`, любая формула.
    // Проверено прогоном 2026-09-23: `CIRCLE_STAT_N 1e17;` — отказ, а полная запись
    // проходит; ровно так пишут концентрацию авторские токамаковые задания
    // (`N_AXIS @ (54000000000000000000.0)`).
    return plainDecimal(QString::asprintf("%.10g", v));
}

QString plainDecimal(const QString &number)
{
    const int e = number.indexOf(QLatin1Char('e'), 0, Qt::CaseInsensitive);
    if (e < 0)
        return number;
    QString mant = number.left(e);
    const int exp = number.mid(e + 1).toInt();
    bool neg = mant.startsWith(QLatin1Char('-'));
    if (neg || mant.startsWith(QLatin1Char('+')))
        mant = mant.mid(1);
    // Цифры мантиссы без точки и позиция точки в них.
    int dot = mant.indexOf(QLatin1Char('.'));
    QString digits = mant;
    if (dot < 0) {
        dot = mant.size();
    } else {
        digits.remove(dot, 1);
    }
    int point = dot + exp; // сколько цифр стоит до точки
    QString out;
    if (point <= 0) {
        out = QStringLiteral("0.") + QString(-point, QLatin1Char('0')) + digits;
    } else if (point >= digits.size()) {
        out = digits + QString(point - digits.size(), QLatin1Char('0'));
    } else {
        out = digits.left(point) + QLatin1Char('.') + digits.mid(point);
    }
    // Хвостовые нули дробной части лишние, ведущие нули целой — тоже.
    if (out.contains(QLatin1Char('.'))) {
        while (out.endsWith(QLatin1Char('0')))
            out.chop(1);
        if (out.endsWith(QLatin1Char('.')))
            out.chop(1);
    }
    while (out.size() > 1 && out.startsWith(QLatin1Char('0')) && out.at(1) != QLatin1Char('.'))
        out.remove(0, 1);
    return neg ? QLatin1Char('-') + out : out;
}

QString plainLiterals(const QString &expr)
{
    // Число с экспонентой, не прилипшее к имени: `3.1e+17`, `1E-5`, `.5e3`. `exp(`
    // и переменная `e` внутри имени не трогаются — слева и справа от числа не буква.
    static const QRegularExpression re(QStringLiteral(
        "(?<![A-Za-z_0-9.])((?:\\d+\\.?\\d*|\\.\\d+)[eE][+-]?\\d+)(?![A-Za-z_0-9.])"));
    QString out = expr;
    QRegularExpressionMatchIterator it = re.globalMatch(expr);
    QVector<QRegularExpressionMatch> hits;
    while (it.hasNext())
        hits << it.next();
    for (int i = hits.size() - 1; i >= 0; --i) {
        const QRegularExpressionMatch &m = hits.at(i);
        QString plain = plainDecimal(m.captured(1).startsWith(QLatin1Char('.'))
                                         ? QLatin1Char('0') + m.captured(1)
                                         : m.captured(1));
        if (!plain.contains(QLatin1Char('.')))
            plain += QStringLiteral(".0");
        out.replace(m.capturedStart(1), m.capturedLength(1), plain);
    }
    return out;
}

QString formatNumber(double v)
{
    QString s = formatCompact(v);
    if (!s.contains(QLatin1Char('.')) && !s.contains(QLatin1Char('e')))
        s += QLatin1String(".0");
    return s;
}

QString formatFreq(double ghz)
{
    // Один знак после точки — как в авторских заданиях (`FREQ 3.1;`), и так пишется
    // всё, что в него укладывается. Но поля частоты в мастере принимают три знака, и
    // раньше 9.35 ГГц молча уходило в файл как 9.4, а свип с шагом 0.05 склеивал
    // соседние точки в одну. Более мелкую частоту пишем столькими знаками, сколько в
    // ней есть.
    const double tenths = ghz * 10.0;
    if (std::fabs(tenths - std::round(tenths)) < 1e-9)
        return QString::asprintf("%.1f", ghz);
    return formatNumber(std::round(ghz * 1e9) / 1e9); // шум сложения шагов — за бортом
}

QString ampExpression(const Signal &s)
{
    switch (s.shape) {
    case SignalShape::Continuous:
    case SignalShape::Rectangular:
        // Форму задаёт окно; амплитуда — просто множитель.
        return formatCompact(s.amplitude);
    case SignalShape::Gaussian: {
        // Огибающая живёт в Amp, а не в окне: сужать окно нельзя — оно режет резко,
        // и вместо импульса выйдет обрубленная несущая со спектральными «усами».
        // Интерпретатор ждёт строчное exp( ; t в нс, f в ГГц.
        //
        // ⚠️ Квадрат берём умножением, а НЕ через '^'. Оператор степени в этом
        // интерпретаторе падает с ERROR_POW при основании <= 0 (Expr/EXPR_COM.CPP:76-83),
        // а (t-t0) отрицательно на всём фронте импульса и равно нулю в его центре.
        const QString x = QStringLiteral("((t-%1)/%2)")
                              .arg(formatCompact(s.center), formatCompact(s.duration));
        return QStringLiteral("%1*exp(-%2*%3)").arg(formatCompact(s.amplitude), x, x);
    }
    }
    return QStringLiteral("1");
}

TimeWindow portWindow(const Port &p, double timeMax)
{
    if (!p.active) {
        // Пассивный вход обязан молчать: окно начинается заведомо позже конца счёта.
        // Литерал 300 из авторских заданий верен лишь пока TIME=115 — не копируем его.
        return {timeMax * 2.0, timeMax * 3.0};
    }

    // Сигнал из файла: окно повторяет опору самого сигнала, прижатую к концу счёта.
    //
    // Так оно не режет ничего лишнего, и остаётся ровно одна причина потерять хвост —
    // время счёта TIME. О ней validate() и предупреждает: подгонять окно под файл
    // руками пользователь не обязан, это работа генератора.
    if (p.signal.source == SignalSource::File) {
        const TimeWindow sup = waveSupport(p.signal.wave);
        return {std::max(0.0, sup.tMin), std::min(sup.tMax, timeMax)};
    }

    switch (p.signal.shape) {
    case SignalShape::Continuous:
    case SignalShape::Gaussian:
        return {0.0, timeMax}; // у гаусса окно широкое — длительность задаёт огибающая
    case SignalShape::Rectangular:
        return {p.signal.center - p.signal.duration / 2.0,
                p.signal.center + p.signal.duration / 2.0};
    }
    return {0.0, timeMax};
}

bool needsMacro(const Project &p)
{
    // Угловой свип — тот же свип, только по углу: шаг описывается один раз.
    return p.freqs.size() > 1 || p.sweep.active();
}

namespace {

/// Проверки порта, который берёт падающую волну из файла.
///
/// 🔑 Правило проверки задания держится: ошибка — только то, что
/// ядро ОТВЕРГНЕТ или посчитает молча не тем. Всё остальное — предупреждение,
/// которое записи файла не мешает.
///
/// Про моды отдельно. Раньше замечания об отсечках H10/H20 из проекта убрали и
/// возвращать их не следует: одночастотный расчёт в заполненном волноводе они
/// объявляли ошибочным. Здесь случай ДРУГОЙ и обратного смысла. Широкополосную
/// матрицу рассеяния восстанавливает постобработка, и она делает это в
/// предположении ОДНОЙ моды в апертуре. Выше отсечки H20 предположение ложно:
/// прогон пройдёт, файл получится, а число выйдет не то — и никто об этом не
/// скажет. Это ровно «молча посчитает другое», то есть законный повод
/// предупредить. Требование поставлено пользователем и К. Н. Климовым 2026-09-30.
/// @param[in]     p     задание (время счёта, шаг сетки, среда входа)
/// @param[in]     port  порт с сигналом из файла
/// @param[in,out] d     куда дописываются ошибки и предупреждения
/// @see https://www.tamic.ru/docs/theory/ports.html#incident-file
void checkFileSignal(const Project &p, const Port &port, Diagnostics *d)
{
    const Waveshape &w = port.signal.wave;
    // Два падежа: «порт «вход 1»» и «порта «вход 1»». Один на все случаи давал
    // «полоса сигнала порт «вход 1»» — замечание, которое читать неприятно, а значит
    // и доверия ему меньше.
    const QString who = tpl::tr("порт «%1»", "port '%1'").arg(port.name);
    const QString whose = tpl::tr("порта «%1»", "port '%1'").arg(port.name);
    QStringList &e = d->errors;
    QStringList &warn = d->warnings;

    // Имя файла: расширение `.sig` ядро дописывает ТОЛЬКО если точки в имени нет
    // (`SeachSignalFileName`). С точкой оно пойдёт искать чужой файл.
    if (port.signal.file.contains(QLatin1Char('.'))) {
        e << tpl::tr("имя файла сигнала «%1» (%2) не должно содержать точку: "
                     "расширение .sig ядро дописывает само",
                     "the signal file name '%1' (%2) must not contain a dot: the kernel appends "
                     "the .sig extension itself")
                 .arg(port.signal.file, who);
    }

    // Тоны сверх 99 не берёт ни диаграмма направленности ядра, ни постобработка; ядро
    // отбросило бы их молча. Запись кладёт в #FREQ первые 99 — говорим об этом здесь,
    // а не оставляем на догадку. Предупреждение: сигнал считается, обработка неполная.
    const int toneCount = int(waveTones(w).size());
    if (toneCount > kMaxFreqs) {
        warn << tpl::tr("у %1 в сигнале %2, а пакет обрабатывает не больше %3: в "
                        "строку #FREQ файла .sig уйдут первые %3, остальные %4 "
                        "останутся в сигнале, но ни в диаграмму, ни в постобработку "
                        "не попадут",
                        "the signal of %1 has %2, but the package processes at most %3: the first "
                        "%3 will go into the #FREQ line of the .sig file, the remaining %4 will "
                        "stay in the signal but will enter neither the radiation pattern nor "
                        "post-processing")
                    .arg(whose,
                         plural(toneCount, tpl::tr("тон", "tone"), tpl::tr("тона", "tones"),
                                tpl::tr("тонов", "tones")))
                    .arg(kMaxFreqs)
                    .arg(toneCount - kMaxFreqs);
    }

    if (w.kind == Waveform::Table) {
        // Ядро отвергает файл меньше чем с двумя точками и с неубывающим временем —
        // обе проверки стоят в `ReadSignalFile`, и обе печатают ошибку загрузки.
        if (w.points.size() < 2) {
            e << tpl::tr("у %1 в таблице сигнала %2 точек — ядру нужно не меньше двух",
                         "the signal table of %1 has %2 points - the kernel needs at least two")
                     .arg(whose).arg(w.points.size());
        } else {
            for (int i = 1; i < w.points.size(); ++i) {
                if (w.points.at(i).x() <= w.points.at(i - 1).x()) {
                    e << tpl::tr("у %1 в таблице сигнала время не возрастает "
                                 "(точка %2: %3 нс после %4 нс) — ядро отвергнет файл",
                                 "in the signal table of %1 the time does not increase (point %2: "
                                 "%3 ns after %4 ns) - the kernel will reject the file")
                             .arg(whose).arg(i + 1)
                             .arg(formatNumber(w.points.at(i).x()),
                                  formatNumber(w.points.at(i - 1).x()));
                    break;
                }
            }
        }
    }

    if (w.kind == Waveform::Expression) {
        const ExprProgram prog = waveProgram(w);
        if (!prog.valid()) {
            e << tpl::tr("у %1 формулу сигнала «%2» разобрать не удалось",
                         "the signal formula '%2' of %1 could not be parsed")
                     .arg(whose, w.expression);
        } else {
            Variables known;
            known.insert(QStringLiteral("t"), 0.0);
            const QStringList unknown = prog.unknown(known);
            if (!unknown.isEmpty()) {
                e << tpl::tr("в формуле сигнала %1 есть неизвестное имя «%2»: "
                             "переменная здесь только одна — t (нс)",
                             "the signal formula of %1 contains an unknown name '%2': there is "
                             "only one variable here, t (ns)")
                         .arg(whose, unknown.join(QStringLiteral(", ")));
            } else {
                // Формулу считаем МЫ, и если она не даёт числа, писать в файл нечего.
                const TimeWindow sup = waveSupport(w);
                int bad = 0;
                for (int i = 0; i <= 200; ++i) {
                    Variables v;
                    v.insert(QStringLiteral("t"), sup.tMin + sup.length() * i / 200.0);
                    const std::optional<double> r = prog.eval(v);
                    if (!r || !std::isfinite(*r))
                        ++bad;
                }
                if (bad > 0) {
                    e << tpl::tr("формула сигнала %1 не даёт числа в %2 точках из 201 — "
                                 "в файл писать нечего",
                                 "the signal formula of %1 gives no number at %2 of 201 points - "
                                 "nothing to write to the file")
                             .arg(whose).arg(bad);
                }
            }
        }
    }

    const SigPlan plan = sigPlan(p, port);
    if (plan.points < 2) {
        e << tpl::tr("сигнал %1 пуст: точек в файле получилось %2. Проверьте "
                     "длительность и начало",
                     "the signal of %1 is empty: the file got %2 points. Check the duration and "
                     "the start")
                 .arg(whose).arg(plan.points);
        return; // дальше проверять нечего, файла всё равно не будет
    }

    // --- обрезание и зануление окном ---
    if (plan.cutByTime) {
        const TimeWindow sup = waveSupport(w);
        warn << tpl::tr("сигнал %1 кончается на %2 нс, а счёт идёт до %3 нс — "
                        "хвост будет ОБРЕЗАН. Увеличьте время счёта или укоротите "
                        "посылку",
                        "the signal of %1 ends at %2 ns, but the run lasts only until %3 ns - the "
                        "tail will be CUT OFF. Increase the run time or shorten the burst")
                    .arg(whose, formatNumber(sup.tMax), formatNumber(p.timeMax));
    } else if (plan.to < p.timeMax * 0.9) {
        // Зануление. Для импульса это норма и ради него всё и делается, а вот для
        // непрерывной формы — признак того, что посылку задали короче счёта, и после
        // неё порт молчит.
        const bool continuous = (w.kind == Waveform::MultiTone || w.kind == Waveform::Chirp
                                 || w.kind == Waveform::Sawtooth || w.kind == Waveform::Triangle
                                 || w.kind == Waveform::Square);
        if (continuous) {
            warn << tpl::tr("сигнал %1 кончается на %2 нс, а счёт идёт до %3 нс — "
                            "дальше порт молчит и окно даёт НОЛЬ. Для непрерывной "
                            "формы посылку обычно тянут на весь счёт",
                            "the signal of %1 ends at %2 ns, but the run lasts until %3 ns - "
                            "after that the port is silent and the window gives ZERO. For a "
                            "continuous waveform the burst is usually extended over the whole run")
                        .arg(whose, formatNumber(plan.to), formatNumber(p.timeMax));
        }
    }
    if (plan.from > 0.0 && plan.from >= p.timeMax) {
        warn << tpl::tr("сигнал %1 начинается на %2 нс — уже после конца счёта "
                        "(%3 нс): возбуждения не будет вовсе",
                        "the signal of %1 starts at %2 ns - already after the end of the run (%3 "
                        "ns): there will be no excitation at all")
                    .arg(whose, formatNumber(plan.from), formatNumber(p.timeMax));
    }

    if (plan.coarsened) {
        warn << tpl::tr("шаг записи сигнала %1 укрупнён до %2 нс (шаг ядра %3 нс): "
                        "точек иначе выходит слишком много, и интерполяция огрубит "
                        "сигнал. Укоротите посылку или возьмите шаг сетки крупнее",
                        "the sample step of the signal of %1 is coarsened to %2 ns (kernel step "
                        "%3 ns): otherwise there would be too many points, and the interpolation "
                        "will coarsen the signal. Shorten the burst or take a coarser grid step")
                    .arg(whose, formatNumber(plan.dt), formatNumber(plan.kernelDt));
    }

    // --- моды ---
    const Band band = waveBand(w);
    const double a = port.to - port.from;
    // Для ЧЕЛОВЕКА числа округляем: «4.996540967 ГГц» в предупреждении — это шум,
    // который мешает прочитать само предупреждение. Точность нужна файлу, не тексту.
    const auto nice = [](double v) { return QString::number(v, 'f', 3); };
    if (band.known && a > 0.0 && port.mode == PortMode::H10) {
        const double fc10 = cutoffH10(a, portEps(p, port));
        const double fc20 = cutoffH20(a, portEps(p, port));
        if (band.hi > fc20) {
            warn << tpl::tr("полоса сигнала %1 (до %2 ГГц) выше отсечки H20 "
                            "(%3 ГГц при апертуре %4 мм): выше неё в апертуре живёт "
                            "не одна мода, и матрица рассеяния, восстановленная по "
                            "одной, будет неверной. Сузьте полосу или апертуру",
                            "the signal band of %1 (up to %2 GHz) is above the H20 cutoff (%3 GHz "
                            "for a %4 mm aperture): above it more than one mode lives in the "
                            "aperture, and the scattering matrix reconstructed from one mode will "
                            "be wrong. Narrow the band or the aperture")
                        .arg(whose, nice(band.hi), nice(fc20), nice(a));
        }
        if (band.lo < fc10) {
            warn << tpl::tr("нижняя часть полосы сигнала %1 (с %2 ГГц) ниже отсечки "
                            "H10 (%3 ГГц при апертуре %4 мм): на этих частотах волна "
                            "в такой вход не пойдёт, и точки полосы окажутся пустыми",
                            "the lower part of the signal band of %1 (from %2 GHz) is below the "
                            "H10 cutoff (%3 GHz for a %4 mm aperture): at these frequencies the "
                            "wave will not enter such a port, and those band points will be empty")
                        .arg(whose, nice(band.lo), nice(fc10), nice(a));
        }
    }
    if (w.kind == Waveform::MultiTone && w.tones < 2) {
        warn << tpl::tr("у %1 в гребёнке задан %2 тон — меньше двух не бывает, "
                        "взято два",
                        "the comb of %1 has %2 tone - there cannot be fewer than two, two are "
                        "taken").arg(whose).arg(w.tones);
    }
}

/// Задана ли в этой фигуре хоть какая-то концентрация электронов.
/// @param[in] pl  плазменные параметры фигуры
/// @return true — N задана ненулевым числом, файлом или формулой.
bool carriesConcentration(const Plasma &pl)
{
    // Профиль тоже считается: формулу плотности на то и пишут, что где-то внутри
    // фигуры концентрация ненулевая.
    return pl.hasN && (pl.n != 0.0 || !pl.nFile.isEmpty() || !pl.nFormula.trimmed().isEmpty());
}

/// Перекрывается ли фигура с областью, где N != 0 (своей или чужой).
///
/// Сравниваем ГАБАРИТЫ, а не точную геометрию: задача — поймать «B задали, а N
/// забыли», а не измерять площадь пересечения. Ложно-положительное срабатывание
/// здесь безобиднее пропущенного предупреждения.
/// @param[in] p  задание
/// @param[in] s  фигура, у которой задано B
/// @return true — сама фигура несёт N либо её габарит пересекается с габаритом
///         другой фигуры с N.
bool hasOverlappingConcentration(const Project &p, const Shape &s)
{
    if (carriesConcentration(s.plasma))
        return true;

    double ax = 0, bx = 0, ay = 0, by = 0;
    s.bounds(&ax, &bx, &ay, &by);
    for (const Shape &o : p.shapes) {
        if (o.id == s.id || !carriesConcentration(o.plasma))
            continue;
        double cx = 0, dx = 0, cy = 0, dy = 0;
        o.bounds(&cx, &dx, &cy, &dy);
        if (bx >= cx && dx >= ax && by >= cy && dy >= ay)
            return true;
    }
    return false;
}

} // namespace

Diagnostics validate(const Project &p)
{
    // Порядок проверок: задание в целом (имя, частоты, сетка, область, потолки пакета,
    // память ДН), имена выходных файлов, номера блоков, фигуры (геометрия, плазма,
    // профиль eps), порты (апертура, eps входа, плоскость, окно, сигнал из файла),
    // широкая полоса, конвейер, варианты и возбуждения, угловой свип, размещение
    // (`checkPlacement`), символы вне CP1251 в описании; в конце — предупреждения.
    Diagnostics d;
    QStringList &e = d.errors;

    /// Плазменные параметры фигуры. Вынесено из общего цикла: проверок много и
    /// каждая опирается на найденную в исходниках ядра особенность.
    const auto checkPlasma = [&](const Shape &s, const QString &title) {
        const Plasma &pl = s.plasma;
        if (!pl.any())
            return;

        if (p.kernel == Kernel::H) {
            // Дальше проверять нечего: плазмы в этом расчёте не будет вовсе.
            e << tpl::tr("плазма («%1») считается только X-модой (ядро tmc_rtx), "
                         "а выбрано ядро H",
                         "plasma ('%1') is computed only by the X-mode (kernel tmc_rtx), but "
                         "kernel H is selected").arg(title);
            return;
        }
        if (s.isMoving()) {
            e << tpl::tr("движущийся блок «%1» не может быть плазменным — "
                         "тегов RECT_MOVE_N, _B, _Y в языке нет",
                         "moving block '%1' cannot be a plasma one - the language has no "
                         "RECT_MOVE_N, _B, _Y tags").arg(title);
        }

        // Y и σ любой формы: с 26.10.2026 `CIRCLE_STAT_Y`, `POLYGON_STAT_Y` и `*_STAT_S`
        // ядро X читает в свои величины (раньше круглые и многоугольные B и Y молча
        // уходили в концентрацию N — отсюда прежний запрет «Y только прямоугольником»).
        //
        // Величина из файла: `FILE_N`, `FILE_B`, `FILE_Y` (с 26.10.2026). Проверить
        // содержимое файла нечем — он лежит рядом с заданием и пишется ядром, а файл
        // чужой величины ядро отвергнет само, назвав причину. Наше дело — имя.
        const auto checkFileName = [&](bool has, const QString &file, const QString &what) {
            const QString name = file.trimmed();
            if (!has || name.isEmpty())
                return;
            if (name.contains(QLatin1Char('.'))) {
                e << tpl::tr("«%1»: имя файла %2 «%3» не должно содержать точку — "
                             "расширение .eps ядро дописывает само",
                             "'%1': the %2 file name '%3' must not contain a dot - the kernel "
                             "appends the .eps extension itself")
                         .arg(title, what, name);
            }
            for (QChar ch : name) {
                if (ch.unicode() > 127 || ch == QLatin1Char(';') || ch.isSpace()) {
                    e << tpl::tr("«%1»: имя файла %2 «%3» должно быть латиницей без "
                                 "пробелов: оно уходит в ядро как имя файла на диске",
                                 "'%1': the %2 file name '%3' must be in Latin letters without "
                                 "spaces: it goes to the kernel as a file name on disk")
                             .arg(title, what, name);
                    break;
                }
            }
        };
        checkFileName(pl.hasN, pl.nFile, tpl::tr("концентрации N", "density N"));
        checkFileName(pl.hasB, pl.bFile, tpl::tr("подмагничивания B", "magnetization B"));
        checkFileName(pl.hasY, pl.yFile, tpl::tr("параметра Y", "parameter Y"));
        if (pl.hasFile() && s.kind != ShapeKind::Rectangle) {
            d.warnings << tpl::tr("«%1»: величина из файла читается прямоугольником — файл "
                                  "хранит регулярную решётку, и ляжет она на габарит "
                                  "фигуры, а не на её форму",
                                  "'%1': a quantity from a file is read as a rectangle - the file "
                                  "stores a regular grid, and it will be laid over the bounding "
                                  "box of the shape, not over its outline").arg(title);
        }
        if (pl.hasS && pl.sFormula.trimmed().isEmpty() && pl.s < 0.0) {
            d.warnings << tpl::tr("«%1»: проводимость sigma = %2 См/м — отрицательные "
                                  "потери не гасят поле, а раскачивают его",
                                  "'%1': conductivity sigma = %2 S/m - negative losses do not "
                                  "damp the field but pump it up")
                     .arg(title, formatCompact(pl.s));
        }
        const bool nProfile = !pl.nFormula.trimmed().isEmpty();
        if (pl.hasN && pl.nFile.trimmed().isEmpty() && !nProfile && pl.n <= 0.0) {
            d.warnings << tpl::tr("«%1»: концентрация N = %2 — плазмы не будет, "
                                  "плазменная ветка ядра включается только при N != 0",
                                  "'%1': density N = %2 - there will be no plasma, the plasma "
                                  "branch of the kernel is switched on only when N != 0")
                              .arg(title, formatCompact(pl.n));
        }
        // B без перекрывающего N ядро МОЛЧА выбрасывает: плазменная ветка включается
        // только при N != 0 либо при отрицательной добавке eps, а в другой ветке стоит
        // `prYNode[ii+4] = -2` (kernels/PlanarRT_X/TmcRTHRectNode.cpp:1092). То есть
        // нарисованное подмагничивание просто исчезает из расчёта — об этом говорим.
        //
        // А вот про «B = 0» и про Y без N молчим: Y работает ВЕЗДЕ, в обеих ветках
        // (там же, 1125 и 1147), и потери без плазмы — законная задача, а нулевое поле
        // — законный вырожденный случай. Это были догадки об умысле, а не ловушки.
        if (pl.hasB && !hasOverlappingConcentration(p, s)) {
            d.warnings << tpl::tr("«%1»: подмагничивание не подействует — нет "
                                  "перекрывающей области с N != 0, и ядро молча его "
                                  "отбросит. Задайте концентрацию этой же фигуре",
                                  "'%1': the magnetization will have no effect - there is no "
                                  "overlapping region with N != 0, and the kernel will silently "
                                  "discard it. Give this same shape a density")
                              .arg(title);
        }

        // ⚠️ Полосу непрозрачности X-моды здесь НЕ проверяем (решение 2026-09-06).
        //
        // «Волна отразится, а не пройдёт» — это не ошибка задания, а его результат: тем
        // и интересна плазма, что она отражает. Ровно на этом стоит расчёт плазменного
        // слоя вокруг гиперзвукового аппарата — там непрозрачна вся полоса, и задание
        // считается. Числа f_p и f_ce никуда не делись: их показывает калькулятор,
        // когда о них спрашивают.
        Q_UNUSED(nProfile);
    };

    /// Профиль eps: формула + файл, который ядро посчитает на первом шаге.
    const auto checkEpsProfile = [&](const Shape &s, const QString &title) {
        if (!s.epsProfile.enabled)
            return;

        // Профиль из готового файла проверять нечем и не нужно: содержимое `.eps` —
        // чужая работа, а ядро читает у блока ровно один параметр, имя файла.
        // Единственное, что обязано быть правильным, — само имя.
        if (s.epsProfile.fromFile) {
            const QString name = s.epsProfile.file.trimmed();
            if (name.isEmpty()) {
                e << tpl::tr("у «%1» профиль eps читается из файла, но имя файла не "
                             "задано",
                             "the eps profile of '%1' is read from a file, but the file name is "
                             "not set").arg(title);
            } else if (name.contains(QLatin1Char('.'))) {
                // Расширение ядро дописывает само (`AddFileEpsExtention`), и точка в
                // имени превратила бы его в `имя.что-то.eps`.
                e << tpl::tr("имя файла профиля «%1» не должно содержать точку: "
                             "расширение .eps ядро дописывает само",
                             "the profile file name '%1' must not contain a dot: the kernel "
                             "appends the .eps extension itself").arg(name);
            }
            return;
        }

        const QString formula = s.epsProfile.formula.trimmed();
        if (formula.isEmpty()) {
            e << tpl::tr("у «%1» включён профиль eps, но формула пуста",
                         "the eps profile of '%1' is enabled, but the formula is empty").arg(title);
            return;
        }

        // Формула законна в любой фигуре — ошибки здесь нет и быть не должно.
        //
        // ⚠️ Прямоугольником ограничена только ЗАПИСЬ файла: `SaveFileRect` вызывается
        // исключительно из `SetRect` (kernels/PlanarRT_H/TmcRTHNodeDiel.cpp:413), у
        // круга (`SetCirc`) и полигона (`SetPoly`) вызова нет вовсе. Причина в формате:
        // он хранит регулярную решётку Xmin/Ymin/nX/nY и линейный индекс узла, «дырок»
        // в ней не предусмотрено. А сосчитать выражение в узлах ядро умеет и в круге —
        // `CIRCLE_STAT <выражение>` стоит в авторском `data/cir_quas.h`, блок 20, и
        // прекрасно считается. Поэтому файл у непрямоугольной фигуры просто не пишем и
        // говорим об этом предупреждением: задание остаётся рабочим.
        if (s.epsProfile.savesFile() && s.kind != ShapeKind::Rectangle) {
            d.warnings << tpl::tr("профиль eps у «%1» будет посчитан, но в файл не "
                                  "запишется: сохранять умеет только прямоугольник "
                                  "(в ядре это делает один RECT_STAT). Сама формула "
                                  "работает — файл нужен лишь чтобы посмотреть "
                                  "профиль глазами",
                                  "the eps profile of '%1' will be computed but not written to a "
                                  "file: only a rectangle can save it (in the kernel this is done "
                                  "by RECT_STAT alone). The formula itself works - the file is "
                                  "needed only to look at the profile").arg(title);
        }
        if (s.isMoving()) {
            e << tpl::tr("у движущейся фигуры «%1» профиль eps не считается: файл "
                         "пишется один раз при инициализации шага и за блоком "
                         "не поедет",
                         "the eps profile of moving shape '%1' is not computed: the file is "
                         "written once at the step initialization and will not follow the block").arg(title);
        }
        if (s.plasma.any()) {
            e << tpl::tr("«%1»: профиль eps и плазма в одной фигуре не уживаются — "
                         "разнесите их по двум фигурам, ядро всё равно складывает "
                         "их в одном узле",
                         "'%1': an eps profile and plasma cannot coexist in one shape - split "
                         "them into two shapes, the kernel adds them up in one node anyway").arg(title);
        }

        // Имя файла. Точку ядро принимает за готовое расширение и своё `.eps` не
        // дописывает (`AddFileEpsExtention`), а кириллица не переживёт CP1251 в имени.
        // Проверяем, только если файл вообще заказан: без имени и проверять нечего.
        const QString file = epsProfileFile(s);
        if (file.contains(QLatin1Char('.'))) {
            e << tpl::tr("имя файла профиля «%1» содержит точку: ядро примет её за "
                         "расширение. Напишите имя без точки",
                         "the profile file name '%1' contains a dot: the kernel will take it for "
                         "an extension. Write the name without a dot").arg(file);
        }
        for (QChar ch : file) {
            if (ch.unicode() > 127) {
                e << tpl::tr("имя файла профиля «%1» должно быть латиницей: путь "
                             "уходит в ядро как имя файла на диске",
                             "the profile file name '%1' must be in Latin letters: the path goes "
                             "to the kernel as a file name on disk").arg(file);
                break;
            }
        }

        // Формула. Переменных у ядра ровно четыре, и всё прочее — это либо опечатка,
        // либо ссылка на макро-параметр, которого в нашем задании не существует.
        // Вот это — настоящая ошибка: такое выражение ядро не разберёт вовсе.
        const Variables vars = epsVariables(0.0, 0.0);
        const ExprProgram prog = ExprProgram::compile(formula);
        if (!prog.valid()) {
            e << tpl::tr("формулу eps у «%1» не удалось разобрать: «%2». Ядро отвергнет "
                         "такое выражение при загрузке файла",
                         "the eps formula of '%1' could not be parsed: '%2'. The kernel will "
                         "reject such an expression when loading the file").arg(title, formula);
        } else if (!prog.unknown(vars).isEmpty()) {
            e << tpl::tr("в формуле eps у «%1» непонятные имена: %2. Доступны только "
                         "x, y (мм от центра фигуры), r и f (полярный угол), числа и "
                         "функции exp, sqrt, sin, cos, abs, log",
                         "the eps formula of '%1' has unknown names: %2. Only x, y (mm from the "
                         "shape centre), r and f (polar angle), numbers and the functions exp, "
                         "sqrt, sin, cos, abs, log are available")
                     .arg(title, prog.unknown(vars).join(QStringLiteral(", ")));
        } else {
            // Считаем формулу ВНУТРИ фигуры — ровно там, где её будет считать ядро.
            //
            // ⚠️ Раньше пробовали по описанному прямоугольнику, и на секторе это давало
            // ложную тревогу: точки вне сектора ядро не трогает вовсе, а у нас они
            // попадали в «не считается». Область берём настоящую.
            int bad = 0, total = 0;
            double lo = 0.0, hi = 0.0;
            bool haveRange = false;
            for (const QPointF &pt : epsSamplePoints(s)) {
                const auto v = prog.evalEps(pt.x(), pt.y());
                ++total;
                if (!v) {
                    ++bad;
                    continue;
                }
                lo = haveRange ? std::min(lo, *v) : *v;
                hi = haveRange ? std::max(hi, *v) : *v;
                haveRange = true;
            }
            if (bad > 0) {
                // Предупреждение, а не ошибка. Наша арифметика и арифметика ядра —
                // не один и тот же код, и запрещать запись из-за расхождения нельзя:
                // задание, которое ядро считает, обязано сохраняться.
                d.warnings << tpl::tr("формула eps у «%1» не даёт числа в %2 из %3 "
                                      "проверенных точек области (деление на ноль, "
                                      "корень из отрицательного, NaN). Проверьте "
                                      "формулу: в этих узлах среда выйдет неопределённой",
                                      "the eps formula of '%1' gives no number at %2 of %3 "
                                      "checked points of the region (division by zero, root of a "
                                      "negative number, NaN). Check the formula: in these nodes "
                                      "the medium will be undefined")
                                  .arg(title).arg(bad).arg(total);
            }
            // Отрицательная eps замечанием не считается: см. пояснение у числовой eps.
            Q_UNUSED(lo);
            Q_UNUSED(hi);
        }

        // Сетка профиля жёстко привязана к шагу: `SetRectFile` сверяет `#dDelta` из
        // файла с `DELTA` шага с точностью 1e-5 и отказывается грузить при расхождении
        // (kernels/PlanarRT_H/TmcRTHNodeDiel.cpp:7089). У нас DELTA одна на всё
        // задание, так что разойтись нечему — но старый файл с диска подойдёт не всегда.
        if (p.delta > 0.0 && s.epsProfile.savesFile() && s.kind == ShapeKind::Rectangle) {
            double xLo = 0, xHi = 0, yLo = 0, yHi = 0;
            s.bounds(&xLo, &xHi, &yLo, &yHi);
            const double nodes = ((xHi - xLo) / p.delta + 1.0) * ((yHi - yLo) / p.delta + 1.0);
            if (nodes > 4.0e6) {
                d.warnings << tpl::tr("профиль eps у «%1» — это примерно %2 млн узлов "
                                      "(%3 МБ на диске). Считается он один раз, но файл "
                                      "выйдет большим",
                                      "the eps profile of '%1' is about %2 million nodes (%3 MB "
                                      "on disk). It is computed once, but the file will be large")
                                  .arg(title, QString::asprintf("%.1f", nodes / 1e6),
                                       QString::asprintf("%.0f", nodes * 8.0 / 1048576.0));
            }
        }
    };

    // --- задание в целом: имя, частоты, шаг сетки, время, область ---
    if (p.name.isEmpty())
        e << tpl::tr("не задано имя задания", "the task name is not set");
    if (p.name.contains(QLatin1Char('.')))
        e << tpl::tr("имя задания не должно содержать точку: %1",
                     "the task name must not contain a dot: %1").arg(p.name);

    if (p.freqs.isEmpty())
        e << tpl::tr("не задано ни одной частоты", "no frequency is set");
    if (p.delta <= 0.0)
        e << tpl::tr("шаг сетки должен быть больше нуля",
                     "the grid step must be greater than zero");
    if (p.timeMax <= 0.0)
        e << tpl::tr("время счёта должно быть больше нуля",
                     "the run time must be greater than zero");
    if (p.box.width() <= 0.0 || p.box.height() <= 0.0)
        e << tpl::tr("счётная область пуста", "the computational domain is empty");

    // Незадокументированные потолки самого пакета. Найдены в исходниках:
    // FREQ_MAX_NUM = 100 (src/Include/TMCLIMIT.H:28), отказ `>= FREQ_MAX_NUM`
    // в объединении .s (libs/SFILE95/Unite_s.cpp:68); MAX_INPUT_NUM = 20 (там же:34).
    // Свип на 41 точку в них укладывается, а вот «от 1 до 5 ГГц с шагом 0.02» — уже нет.
    //
    // 🔑 Ошибка, а не предупреждение (решение пользователя 2026-10-07): больше 99 частот
    // в одно задание пакет не берёт нигде — ни в .s, ни в диаграмму, ни в постобработку.
    // Сетку гуще режет на части конвейер заданий: у каждой части своё задание и свой
    // .s, и 99 частот на задание — его же потолок.
    if (p.freqs.size() > kMaxFreqs && p.chunkCount() == 1) {
        e << tpl::tr("частот %1, а в одно задание пакет берёт не больше %2. "
                     "Увеличьте шаг по частоте или включите «Разложить по нескольким "
                     "заданиям» — сетка разойдётся по файлам, у каждого будет свой .s",
                     "%1 frequencies, but the package takes at most %2 per task. Increase the "
                     "frequency step or enable \"Split into several tasks\" - the grid will be "
                     "spread over files, each with its own .s")
                 .arg(p.freqs.size()).arg(kMaxFreqs);
    }
    if (p.ports.size() > 20) {
        d.warnings << tpl::tr("входов %1, а пакет рассчитан не более чем на 20 — "
                              "лишние ядро может не разместить",
                              "%1 ports, but the package is designed for at most 20 - the kernel "
                              "may fail to place the extra ones").arg(p.ports.size());
    }

    // Диаграмма по многотонному сигналу: окно накопления должно быть кратно 1/df.
    //
    // Ядро снимает амплитуду и фазу синхронным детектированием на каждом тоне.
    // Если в окно не укладывается целое число периодов разностной частоты,
    // соседние тоны подмешиваются друг в друга — спектральная утечка. Замерено
    // на гребёнке 3.0...4.8 ГГц (df = 0.2 ГГц, 1/df = 5 нс): при окне 6 нс
    // диаграмма расходилась с одночастотным расчётом на 3.3 %, при окне 5 нс —
    // на 0.9 %. Считается всё равно, поэтому предупреждение, а не ошибка.
    if (p.output.directionalPattern) {
        const double win = p.output.patternTMax - p.output.patternTMin;       // нс
        for (const Port &port : p.ports) {
            if (port.signal.source != SignalSource::File)
                continue;
            const Waveshape &w = port.signal.wave;
            if (w.kind != Waveform::MultiTone || w.tones < 2)
                continue;
            const double df = (w.f2 - w.f1) / double(w.tones - 1);            // ГГц
            if (df <= 1.0e-9 || win <= 0.0)
                continue;
            const double period = 1.0 / df;                                   // нс
            const double k = win / period;
            if (std::fabs(k - std::floor(k + 0.5)) > 0.01 && k < 1.0) {
                // Окно короче одного периода разностной частоты: «окно 0» не совет.
                d.warnings << tpl::tr(
                                  "окно диаграммы %1 нс короче 1/df = %2 нс (df = %3 ГГц "
                                  "между тонами у «%4»): тоны подмешаются друг в друга и "
                                  "диаграмма сдвинется на проценты. Возьмите окно %2 нс",
                                  "the radiation pattern window %1 ns is shorter than 1/df = %2 "
                                  "ns (df = %3 GHz between the tones of '%4'): the tones will "
                                  "leak into each other and the pattern will shift by percents. "
                                  "Take a window of %2 ns")
                                  .arg(win, 0, 'g', 4)
                                  .arg(period, 0, 'g', 4)
                                  .arg(df, 0, 'g', 4)
                                  .arg(port.name);
            } else if (std::fabs(k - std::floor(k + 0.5)) > 0.01) {
                d.warnings
                    << tpl::tr(
                           "окно диаграммы %1 нс не кратно 1/df = %2 нс (df = %3 ГГц "
                           "между тонами у «%4»): тоны подмешаются друг в друга и "
                           "диаграмма сдвинется на проценты. Возьмите окно %5 или %6 нс",
                           "the radiation pattern window %1 ns is not a multiple of 1/df = %2 ns "
                           "(df = %3 GHz between the tones of '%4'): the tones will leak into "
                           "each other and the pattern will shift by percents. Take a window of "
                           "%5 or %6 ns")
                           .arg(win, 0, 'g', 4)
                           .arg(period, 0, 'g', 4)
                           .arg(df, 0, 'g', 4)
                           .arg(port.name)
                           .arg(std::floor(k) * period, 0, 'g', 4)
                           .arg(std::ceil(k) * period, 0, 'g', 4);
            }
            break;              // одного предупреждения достаточно
        }

        // Многочастотная диаграмма копит по паре квадратур на КАЖДЫЙ тон — память
        // растёт с числом частот. Не влезет — ядро остановится на «Error when init
        // array…», причём уже после долгой подготовки сетки. Говорим заранее, с
        // числами; это предупреждение, а не ошибка: на сервере память найдётся.
        const RamEstimate ram = ramEstimate(p);
        if (ram.patternTones > 1 && ram.total() >= kRamWarnBytes) {
            d.warnings << tpl::tr("диаграмма направленности на %1: ядру понадобится "
                                  "около %2 оперативной памяти (сетка %3, накопление "
                                  "ДН %4). Если на машине столько свободной нет, ядро "
                                  "остановится с ошибкой выделения памяти, а "
                                  "32-разрядное ядро больше 2 ГБ не возьмёт вовсе. "
                                  "Уменьшите число тонов, укрупните шаг сетки или "
                                  "считайте на машине с запасом памяти",
                                  "radiation pattern at %1: the kernel will need about %2 of RAM "
                                  "(grid %3, pattern accumulation %4). If the machine does not "
                                  "have that much free memory, the kernel will stop with a memory "
                                  "allocation error, and a 32-bit kernel will not take more than "
                                  "2 GB at all. Reduce the number of tones, coarsen the grid step "
                                  "or run on a machine with spare memory")
                              .arg(plural(ram.patternTones, tpl::tr("частоте", "frequency"),
                                          tpl::tr("частотах", "frequencies"),
                                          tpl::tr("частотах", "frequencies")),
                                   formatBytes(ram.total()), formatBytes(ram.gridBytes),
                                   formatBytes(ram.patternBytes));
        }
    }

    // Имена выходных файлов: расширение ядро добавляет само, точка в имени ломает FILE.
    const auto checkName = [&e](const QString &n, const QString &what, bool required) {
        if (n.isEmpty()) {
            if (required)
                e << tpl::tr("не задано имя файла: %1", "file name is not set: %1").arg(what);
        } else if (n.contains(QLatin1Char('.'))) {
            e << tpl::tr("имя файла (%1) не должно содержать точку: %2",
                         "the file name (%1) must not contain a dot: %2").arg(what, n);
        }
    };
    checkName(p.output.signalFile, tpl::tr("сигналы", "signals"), true);
    checkName(p.output.sMatrixFile, tpl::tr("S-матрица", "S-matrix"), p.output.sMatrix);
    checkName(p.output.patternFile, tpl::tr("диаграмма направленности", "radiation pattern"),
              p.output.directionalPattern);

    if (p.ports.isEmpty())
        e << tpl::tr("не задано ни одного порта — возбуждать нечем",
                     "no port is set - nothing to excite");

    // Номера блоков: произвольные, но обязаны быть уникальны и >= 1.
    QSet<int> ids;
    const auto checkId = [&](int id, const QString &what) {
        if (id < 1)
            e << tpl::tr("номер блока должен быть не меньше 1 (%1)",
                         "the block number must be at least 1 (%1)").arg(what);
        else if (ids.contains(id))
            e << tpl::tr("номер блока %1 занят дважды (%2)",
                         "block number %1 is used twice (%2)").arg(id).arg(what);
        ids << id;
    };
    for (const Shape &s : p.shapes)
        checkId(s.id, s.name.isEmpty() ? tpl::tr("фигура", "shape") : s.name);
    for (const Port &p2 : p.ports)
        checkId(p2.id, p2.name.isEmpty() ? tpl::tr("порт", "port") : p2.name);

    // --- фигуры: геометрия, материалы, положение относительно области ---
    for (const Shape &s : p.shapes) {
        const QString title = s.name.isEmpty() ? tpl::tr("фигура %1",
                                                         "shape %1").arg(s.id) : s.name;

        if (s.kind == ShapeKind::Polyline) {
            if (s.points.size() < 2)
                e << tpl::tr("у линии «%1» меньше двух точек",
                             "line '%1' has fewer than two points").arg(title);
            // Выражение eps ядро всегда трактует как ЗАМКНУТУЮ область — открытой
            // ломаной с заливкой не бывает.
            if (!s.isContour())
                e << tpl::tr("линия «%1» не может быть заливкой: сделайте её "
                             "замкнутым контуром",
                             "line '%1' cannot be a fill: make it a closed polygon").arg(title);
        }
        if (s.kind == ShapeKind::Polygon) {
            if (s.isContour()) {
                if (s.points.size() < 3)
                    e << tpl::tr("у контура «%1» меньше трёх точек",
                                 "polygon '%1' has fewer than three points").arg(title);
            } else {
                // tpl-guide 5.4: заливной полигон — число вершин БОЛЬШЕ трёх,
                // область односвязная и выпуклая. Обход по часовой ставит эмиттер.
                if (s.points.size() < 4)
                    e << tpl::tr("у заливного полигона «%1» должно быть больше трёх "
                                 "вершин — ядро иначе посчитает не то",
                                 "filled polygon '%1' must have more than three vertices - "
                                 "otherwise the kernel will compute something else").arg(title);
                else if (!isConvex(s.points))
                    d.warnings << tpl::tr("полигон «%1» невыпуклый: ядро выпуклость не "
                                          "проверяет, но заливка выйдет неверной — "
                                          "разбейте его на выпуклые части",
                                          "polygon '%1' is not convex: the kernel does not check "
                                          "convexity, but the fill will be wrong - split it into "
                                          "convex parts").arg(title);
            }
        }
        if (s.kind == ShapeKind::Circle && s.rMax <= s.rMin)
            e << tpl::tr("у окружности «%1» внешний радиус не больше внутреннего",
                         "circle '%1': the outer radius is not greater than the inner one").arg(title);
        if (s.kind == ShapeKind::Circle && s.fiMax <= s.fiMin)
            e << tpl::tr("у сектора «%1» конечный угол не больше начального",
                         "sector '%1': the end angle is not greater than the start angle").arg(title);
        if (s.kind == ShapeKind::Rectangle && (s.xMax <= s.xMin || s.yMax <= s.yMin))
            e << tpl::tr("у прямоугольника «%1» нулевой или отрицательный размер",
                         "rectangle '%1' has zero or negative size").arg(title);
        if (s.kind == ShapeKind::Rectangle && s.isContour()) {
            bool any = false;
            for (const EdgeSpec &spec : s.edges)
                any |= spec.present;
            if (!any)
                e << tpl::tr("у прямоугольника «%1» сняты все четыре стороны — "
                             "рисовать нечего",
                             "rectangle '%1' has all four sides removed - nothing to draw").arg(title);
        }

        {
            // Фигура за коробкой не считается вовсе — молча пропадёт из расчёта.
            double xLo = 0, xHi = 0, yLo = 0, yHi = 0;
            s.bounds(&xLo, &xHi, &yLo, &yHi);
            if (xHi < p.box.xMin || xLo > p.box.xMax || yHi < p.box.yMin || yLo > p.box.yMax)
                d.warnings << tpl::tr("«%1» целиком вне счётной области — в расчёт "
                                      "не попадёт",
                                      "'%1' lies entirely outside the computational domain - it "
                                      "will not enter the computation").arg(title);
        }

        // tpl-guide 5.2: у MAGNETIC и ABSORBER размеры обязаны превышать 2*DELTA,
        // а сама фигура — отстоять от границы коробки не меньше чем на шаг сетки.
        // Предупреждение, а не запрет: в авторских заданиях встречаются стенки в один шаг.
        const bool strict = (s.material == Material::Magnetic || s.material == Material::Absorber)
                            && !s.plasma.any();
        if (strict && s.kind == ShapeKind::Rectangle && p.delta > 0.0) {
            if (s.xMax - s.xMin <= 2 * p.delta || s.yMax - s.yMin <= 2 * p.delta)
                d.warnings << tpl::tr("«%1»: у магнитной стенки и поглотителя обе стороны "
                                      "должны быть больше 2*DELTA (%2 мм)",
                                      "'%1': both sides of a magnetic wall or absorber must be "
                                      "greater than 2*DELTA (%2 mm)")
                                  .arg(title, formatNumber(2 * p.delta));
            // ⚠️ Отступа от границы области здесь НЕ требуем, хотя tpl-guide 5.2 о нём
            // говорит. Поглотитель по смыслу и стоит вплотную к границе — он её и
            // изображает, — и все авторские задания написаны именно так
            // (`samples/error/mini_ctl.TPL`, блоки 8…12 лежат на краю коробки и
            // считаются). Замечание срабатывало на каждом нормальном задании и было
            // чистым шумом.
        }
        checkPlasma(s, title);
        checkEpsProfile(s, title);
        // ⚠️ eps <= 0 — ЗАКОННО и даже обыденно: справочник показывает eps = 0 (добавка
        // -1) и eps = 0.25, а у плазменного слоя вокруг гиперзвукового аппарата eps
        // доходит до -4795 (`samples/error/hyper_sheath_0p4`). Ни ошибки, ни
        // предупреждения здесь нет: запредельная среда — это то, ради чего ядро и
        // писалось. Ядро отвергает отрицательную eps только В СЕЧЕНИИ ВХОДА, и вот там
        // ниже стоит настоящая ошибка.
    }

    // --- порты: апертура, eps входа, плоскость, направление, окно, сигнал из файла ---
    for (const Port &p2 : p.ports) {
        if (p2.to <= p2.from)
            e << tpl::tr("у порта «%1» нулевая апертура",
                         "port '%1' has a zero aperture").arg(p2.name);

        // Среда в сечении входа. Ядро складывает записанную добавку с единицей и
        // отказывается грузить файл, если сумма <= 0 — то есть eps должна быть > 0.
        // Это ошибка, а не предупреждение: с таким файлом расчёт не начнётся вовсе.
        if (portEps(p, p2) <= 0.0) {
            e << tpl::tr("у порта «%1» диэлектрическая проницаемость входа %2 — "
                         "ядро отвергнет файл. Она должна быть больше нуля "
                         "(вакуум — это 1)",
                         "port '%1' has the port permittivity %2 - the kernel will reject the "
                         "file. It must be greater than zero (vacuum is 1)")
                     .arg(p2.name, formatNumber(portEps(p, p2)));
        }

        // Плоскость входа: ядро берёт её буквально, но за коробкой отказывается
        // размещать блок вовсе («x0 must be ...»).
        const double plane = portPlane(p2, p.box);
        const bool alongY = portAlongY(p2.side);
        const double lo = alongY ? p.box.xMin : p.box.yMin;
        const double hi = alongY ? p.box.xMax : p.box.yMax;
        if (plane < lo || plane > hi) {
            e << tpl::tr("порт «%1» стоит за пределами счётной области (%2 вне %3...%4) — "
                         "ядро откажется его разместить",
                         "port '%1' lies outside the computational domain (%2 outside %3...%4) - "
                         "the kernel will refuse to place it")
                     .arg(p2.name, formatNumber(plane), formatNumber(lo), formatNumber(hi));
        } else if (directionForPlane(alongY, plane, p.box) != p2.side) {
            // Молчаливая ловушка: направление ядро выведет само, по половине коробки,
            // и развернёт волну, ничего не сказав.
            e << tpl::tr("порт «%1» стоит в другой половине области, чем его "
                         "направление: ядро развернёт падающую волну. Сдвиньте порт "
                         "или смените направление",
                         "port '%1' lies in the other half of the domain than its direction: the "
                         "kernel will reverse the incident wave. Move the port or change the "
                         "direction")
                     .arg(p2.name);
        }

        // Апертуру ядро молча поджимает на 2*DELTA от поперечных границ коробки
        // (`SetInpXLeft`: dYmin < rYmin + 2*rDelta -> dYmin = ...).
        if (p.delta > 0.0) {
            const double aLo = alongY ? p.box.yMin : p.box.xMin;
            const double aHi = alongY ? p.box.yMax : p.box.xMax;
            if (p2.from < aLo + 2 * p.delta || p2.to > aHi - 2 * p.delta) {
                d.warnings << tpl::tr("апертура порта «%1» ближе 2*DELTA (%2 мм) к краю "
                                      "области — ядро молча её подожмёт",
                                      "the aperture of port '%1' is closer than 2*DELTA (%2 mm) "
                                      "to the domain edge - the kernel will silently shrink it")
                                  .arg(p2.name, formatNumber(2 * p.delta));
            }
        }
        if (p2.active && p2.signal.shape == SignalShape::Gaussian
            && p2.signal.source == SignalSource::Formula && p2.signal.duration <= 0.0)
            e << tpl::tr("у порта «%1» нулевая длительность импульса",
                         "port '%1' has a zero pulse duration").arg(p2.name);
        const TimeWindow w = portWindow(p2, p.timeMax);
        if (!p2.active && w.tMin <= p.timeMax)
            e << tpl::tr("пассивный порт «%1» возбудится: окно с %2 нс, а счёт до %3 нс",
                         "passive port '%1' will be excited: the window starts at %2 ns, and the "
                         "run lasts until %3 ns")
                     .arg(p2.name, formatNumber(w.tMin), formatNumber(p.timeMax));

        // Проверяем только тот файл, который реально пойдёт в дело: у пассивного
        // порта на месте Amp стоит ноль, и предупреждать о его форме незачем.
        if (portUsesSignalFile(p, p2))
            checkFileSignal(p, p2, &d);
    }

    // --- задание в целом: широкая полоса ---
    if (hasFileSignal(p)) {
        // Два порта в один файл — второй затрёт первый, и оба получат чужую волну.
        // Ровно та же ловушка, что у профилей eps, и лечится так же.
        QHash<QString, QString> byFile;
        for (const Port &p2 : p.ports) {
            if (!portUsesSignalFile(p, p2))
                continue;
            const QString key = signalFileName(p, p2).toLower();
            if (byFile.contains(key)) {
                e << tpl::tr("порты «%1» и «%2» пишут падающую волну в один файл "
                             "%3.sig — второй затрёт первый. Дайте им разные имена",
                             "ports '%1' and '%2' write the incident wave to the same file %3.sig "
                             "- the second will overwrite the first. Give them different names")
                         .arg(byFile.value(key), p2.name, signalFileName(p, p2));
            } else {
                byFile.insert(key, p2.name);
            }
        }

        const Band band = projectBand(p);
        if (p.freqs.size() == 1) {
            // FREQ при сигнале из файла — это частота СОГЛАСОВАНИЯ порта; для гребёнки
            // и ЛЧМ её ставят в центр полосы (подтверждено К. Н. Климовым 2026-09-30),
            // для остальных форм — туда, где сидит энергия (см. suggestedFreq).
            // Уехавшая FREQ расчёт не ломает, но портит согласование, и человек узнает
            // об этом только по кривой матрице.
            const double f = p.freqs.first();
            double want = 0.0;
            for (const Port &p2 : p.ports) {
                if (!portUsesSignalFile(p, p2))
                    continue;
                want = std::max(want, suggestedFreq(p2.signal.wave));
            }
            if (want > 0.0 && std::fabs(f - want) > 0.05 * want) {
                d.warnings << tpl::tr("частота %1 ГГц далеко от частоты согласования "
                                      "сигнала (%2 ГГц). При сигнале из файла FREQ — это "
                                      "частота, на которой ядро согласует порт: уехав от "
                                      "неё, она портит согласование, а значит и матрицу",
                                      "frequency %1 GHz is far from the matching frequency of the "
                                      "signal (%2 GHz). With a signal from a file, FREQ is the "
                                      "frequency at which the kernel matches the port: moving "
                                      "away from it spoils the matching, and hence the matrix")
                                  .arg(QString::number(f, 'f', 4),
                                       QString::number(want, 'f', 4));
            }
        }
        if (p.freqs.size() > 1) {
            d.warnings << tpl::tr("сигнал из файла покрывает всю полосу за ОДИН проход, "
                                  "а в задании задано %1 частот: свип с ним не нужен и "
                                  "просто повторит один и тот же расчёт %1 раз",
                                  "a signal from a file covers the whole band in ONE run, but the "
                                  "task has %1 frequencies: a sweep is not needed with it and "
                                  "will simply repeat the same computation %1 times")
                              .arg(p.freqs.size());
        }
        // Предупреждения «SMATRIX при сигнале из файла одночастотный» больше нет: с
        // 26.10.2026 ядро считает SMATRIX такого шага по всей полосе сигнала
        // (многочастотная обработка, Б-22), а окно по умолчанию — весь счёт.
        for (const Shape &s : p.shapes) {
            if (!s.plasma.any())
                continue;
            // X-ядро считает плазменную добавку ОДИН раз, из FREQ
            // (`PlanarRT_X/TmcRTHRectNode.cpp:1122`): среда заморожена на частоте
            // согласования, и на остальных частотах полосы её проницаемость не та.
            // Проверено прогонами: расхождение с эталонным свипом доходит до 0.3 дБ.
            d.warnings << tpl::tr("в задании есть плазма, а сигнал широкополосный: "
                                  "X-ядро считает плазменную добавку один раз, из FREQ, "
                                  "то есть среда «заморожена» на частоте согласования. "
                                  "В широкой полосе такой расчёт неверен — считайте "
                                  "поддиапазонами (конвейер заданий) или узкой полосой "
                                  "вокруг FREQ",
                                  "the task contains plasma, and the signal is wideband: the X "
                                  "kernel computes the plasma increment once, from FREQ, i.e. the "
                                  "medium is \"frozen\" at the matching frequency. Over a wide "
                                  "band such a computation is wrong - compute in sub-bands (task "
                                  "pipeline) or in a narrow band around FREQ");
            break;
        }
    }

    // --- конвейер заданий ---
    if (p.chunking.enabled) {
        if (p.chunking.freqsPerTask < 1) {
            e << tpl::tr("в одном задании должна быть хотя бы одна частота",
                         "a task must have at least one frequency");
        } else if (p.chunking.freqsPerTask > kMaxFreqs) {
            e << tpl::tr("частот в одном задании %1, а пакет берёт не больше %2. "
                         "Возьмите не больше %2",
                         "%1 frequencies per task, but the package takes at most %2. Take at most "
                         "%2")
                     .arg(p.chunking.freqsPerTask).arg(kMaxFreqs);
        }
        if (p.sweep.active()) {
            e << tpl::tr("конвейер делит сетку частот, а при повороте объекта частота "
                         "одна: делить нечего. Выключите одно из двух",
                         "the pipeline splits the frequency grid, but with object rotation there "
                         "is a single frequency: nothing to split. Turn off one of the two");
        }
        if (p.freqs.size() <= p.chunking.freqsPerTask) {
            d.warnings << tpl::tr("частот всего %1, а в задание кладётся %2 — "
                                  "делить не на что, выйдет один файл",
                                  "there are only %1 frequencies, and %2 go into a task - nothing "
                                  "to split, there will be one file")
                              .arg(p.freqs.size()).arg(p.chunking.freqsPerTask);
        }
    }

    // --- варианты расчёта и возбуждения ---
    QSet<QString> tags;
    for (const Variant &v : p.variants) {
        if (v.tag.isEmpty())
            e << tpl::tr("у варианта «%1» не задана добавка к именам файлов — "
                         "иначе результаты вариантов перемешаются в одних файлах",
                         "variant '%1' has no file name suffix - otherwise the results of the "
                         "variants will be mixed in the same files")
                     .arg(v.name);
        else if (v.tag.contains(QLatin1Char('.')))
            e << tpl::tr("добавка «%1» не должна содержать точку",
                         "suffix '%1' must not contain a dot").arg(v.tag);
        else if (tags.contains(v.tag))
            e << tpl::tr("добавка «%1» повторяется у двух вариантов",
                         "suffix '%1' is repeated in two variants").arg(v.tag);
        tags << v.tag;

        for (int id : v.disabledShapes) {
            bool found = false;
            for (const Shape &s : p.shapes)
                found |= (s.id == id);
            if (!found)
                e << tpl::tr("вариант «%1» выключает несуществующую фигуру %2",
                             "variant '%1' disables non-existent shape %2")
                         .arg(v.name).arg(id);
        }

        // Вариант без стенок — самая обидная ошибка: файл считается, но считает
        // не волновод, а пустую коробку. Ловим ДО прогона.
        int on = 0, contours = 0;
        for (const Shape &s : p.shapes) {
            if (!v.isEnabled(s.id))
                continue;
            ++on;
            if (s.isContour())
                ++contours;
        }
        // Пустой вариант — это про НАШ механизм вариантов: пользователь выключил всё
        // и получит пустую коробку. А вот отсутствие стенок замечанием быть перестало:
        // рассеяние плоской волны на теле в свободном пространстве — обычная задача
        // пакета (`samples/error/hyper_sheath_0p4.TPL`), волновод тут ни при чём.
        if (!p.shapes.isEmpty() && on == 0)
            d.warnings << tpl::tr("в варианте «%1» не осталось ни одной фигуры — "
                                  "посчитается пустая коробка",
                                  "variant '%1' has no shapes left - an empty box will be "
                                  "computed").arg(v.name);
        Q_UNUSED(contours);
    }

    // Два варианта с одинаковым набором фигур считают одно и то же дважды.
    for (int i = 0; i < p.variants.size(); ++i) {
        for (int j = i + 1; j < p.variants.size(); ++j) {
            bool same = true;
            for (const Shape &s : p.shapes)
                same &= (p.variants.at(i).isEnabled(s.id) == p.variants.at(j).isEnabled(s.id));
            if (same)
                d.warnings << tpl::tr("варианты «%1» и «%2» состоят из одних и тех же "
                                      "фигур — расчёт удвоится впустую",
                                      "variants '%1' and '%2' consist of the same shapes - the "
                                      "computation will be doubled for nothing")
                                  .arg(p.variants.at(i).name, p.variants.at(j).name);
        }
    }

    // Два профиля в один файл — второй затрёт первый, и обе фигуры получат чужую eps.
    // Имена нечувствительны к регистру: файловая система Windows их не различает.
    {
        QHash<QString, QString> byFile;
        for (const Shape &s : p.shapes) {
            // Без имени файла и столкнуться нечему: профиль просто считается в узлах.
            if (!s.epsProfile.savesFile() || s.kind != ShapeKind::Rectangle)
                continue;
            const QString key = epsProfileFile(s).toLower();
            const QString title = s.name.isEmpty() ? tpl::tr("фигура %1",
                                                             "shape %1").arg(s.id) : s.name;
            if (byFile.contains(key)) {
                e << tpl::tr("«%1» и «%2» пишут профиль eps в один файл %3.eps — "
                             "второй затрёт первый. Дайте им разные имена",
                             "'%1' and '%2' write the eps profile to the same file %3.eps - the "
                             "second will overwrite the first. Give them different names")
                         .arg(byFile.value(key), title, epsProfileFile(s));
            } else {
                byFile.insert(key, title);
            }
        }
    }

    QSet<int> seenExciters;
    for (int id : p.exciters) {
        bool found = false;
        for (const Port &p2 : p.ports)
            found |= (p2.id == id);
        if (!found)
            e << tpl::tr("в списке возбуждений есть несуществующий порт %1",
                         "the excitation list contains non-existent port %1").arg(id);
        if (seenExciters.contains(id))
            e << tpl::tr("порт %1 указан в списке возбуждений дважды",
                         "port %1 is listed twice in the excitation list")
                     .arg(p.portOrdinal(id));
        seenExciters << id;
    }

    if (p.output.sMatrix && sMatrixWindowWritten(p) && p.output.sMatrixTMax > p.timeMax)
        e << tpl::tr("окно S-матрицы (до %1 нс) за концом счёта (%2 нс) — файл будет пуст",
                     "the S-matrix window (up to %1 ns) is beyond the end of the run (%2 ns) - "
                     "the file will be empty")
                 .arg(formatNumber(p.output.sMatrixTMax), formatNumber(p.timeMax));

    // --- угловой свип ---
    if (p.sweep.enabled) {
        const AngleSweep &sw = p.sweep;
        if (sw.shapeIds.isEmpty())
            e << tpl::tr("поворот объекта включён, но не выбрано ни одной фигуры, которую "
                         "поворачивать",
                         "object rotation is enabled, but no shape to rotate is selected");
        if (sw.angles.isEmpty())
            e << tpl::tr("поворот объекта включён, но не задано ни одного угла",
                         "object rotation is enabled, but no angle is set");
        // Точка .s подписана углом — частоту подписать уже нечем. Два свипа сразу
        // смешали бы точки разных частот под одними подписями.
        if (p.freqs.size() > 1)
            e << tpl::tr("поворот объекта считается на одной частоте: точка .s "
                         "подписывается углом, и точки разных частот смешались бы. "
                         "Оставьте одну частоту",
                         "object rotation is computed at a single frequency: the .s point is "
                         "labelled with the angle, and points of different frequencies would get "
                         "mixed. Leave one frequency");
        if (!p.variants.isEmpty())
            e << tpl::tr("поворот объекта и варианты геометрии в одном задании не "
                         "сочетаются: результаты вариантов легли бы в один .s под одними "
                         "подписями. Отключите варианты — опорный шаг без объекта "
                         "свип делает сам",
                         "object rotation and geometry variants cannot be combined in one task: "
                         "the results of the variants would go into one .s under the same labels. "
                         "Disable the variants - the sweep makes the reference step without the "
                         "object itself");
        // Один порт в очереди — это просто активный вход, запрещать тут нечего (раньше
        // запрещалось, и задание с единственным портом не сохранялось). Не сочетается
        // только очередь из НЕСКОЛЬКИХ входов: у каждого свой столбец матрицы, а точка
        // .s уже подписана углом.
        if (p.exciters.size() > 1)
            e << tpl::tr("поворот объекта и возбуждение нескольких входов по очереди в "
                         "одном задании не сочетаются: точка .s подписана углом, и "
                         "столбцы матрицы смешались бы. На странице «Что считать» "
                         "оставьте в очереди один вход",
                         "object rotation and exciting several ports in turn cannot be combined "
                         "in one task: the .s point is labelled with the angle, and the matrix "
                         "columns would get mixed. On the \"What to compute\" page leave one port "
                         "in the queue");

        // У каждого угла свой файл .s и свои файлы сигналов: два одинаковых угла
        // писали бы в одни и те же файлы, и второй шаг затёр бы первый.
        QSet<QString> tagsSeen;
        for (double a : sw.angles) {
            const QString t = sweepFileTag(a);
            if (tagsSeen.contains(t)) {
                e << tpl::tr("угол поворота %1 град. задан дважды — оба шага пишут в одни "
                             "и те же файлы",
                             "rotation angle %1 deg. is set twice - both steps write to the same "
                             "files").arg(formatCompact(a));
                break;
            }
            tagsSeen << t;
        }
        // 0 и 360 — это одна и та же сцена: файлы у них разные, но шаг посчитается дважды.
        QSet<QString> sameScene;
        for (double a : sw.angles) {
            const QString l = formatCompact(sweepLabel(a));
            if (sameScene.contains(l)) {
                d.warnings << tpl::tr("углы поворота совпадают по модулю 360 (%1 град.) — "
                                      "одна и та же сцена посчитается дважды",
                                      "rotation angles coincide modulo 360 (%1 deg.) - the same "
                                      "scene will be computed twice").arg(l);
                break;
            }
            sameScene << l;
        }

        for (int id : sw.shapeIds) {
            const Shape *s = nullptr;
            for (const Shape &cand : p.shapes) {
                if (cand.id == id)
                    s = &cand;
            }
            if (!s) {
                e << tpl::tr("поворачивается несуществующая фигура %1",
                             "non-existent shape %1 is rotated").arg(id);
                continue;
            }
            const QString title = s->name.isEmpty() ? tpl::tr("фигура %1",
                                                              "shape %1").arg(id) : s->name;
            // Чего повёрнутым не записать. Это не запрет ядра, а честная граница
            // генератора: выпустить такой шаг значило бы записать ДРУГУЮ среду.
            if (s->hasEpsProfile())
                e << tpl::tr("«%1» с профилем eps не поворачивается: координаты формулы "
                             "пришлось бы поворачивать внутри самой формулы. Задайте "
                             "объекту постоянную eps",
                             "'%1' with an eps profile cannot be rotated: the formula coordinates "
                             "would have to be rotated inside the formula itself. Give the object "
                             "a constant eps").arg(title);
            if (s->plasma.hasFormula())
                e << tpl::tr("«%1»: плазма формулой не поворачивается — задайте "
                             "концентрацию числом",
                             "'%1': plasma given by a formula cannot be rotated - set the density "
                             "as a number").arg(title);
            // B пишется прямоугольником шире плазмы — повёрнутым его не выразить; Y, σ и
            // файл повернуть можно было бы, но поворот шага описан только для N.
            if (s->plasma.hasB || s->plasma.hasY || s->plasma.hasS || s->plasma.hasFile())
                e << tpl::tr("«%1»: у поворачиваемой фигуры плазма задаётся только "
                             "концентрацией N числом — подмагничивание B, потери Y и "
                             "sigma и величины из файла повернуть нечем. Уберите их у "
                             "этой фигуры",
                             "'%1': for a rotated shape plasma is set only by the density N as a "
                             "number - magnetization B, losses Y and sigma and quantities from "
                             "files cannot be rotated. Remove them from this shape").arg(title);
            if (s->isMoving())
                e << tpl::tr("«%1» движется — движущийся блок поворачивать нельзя",
                             "'%1' is moving - a moving block cannot be rotated")
                         .arg(title);

            // Объект обязан оставаться в коробке при любом угле: ядро молча отрежет
            // вылезшее, и посчитается другое тело.
            for (double a : sw.angles) {
                const Shape r = rotatedShape(*s, sweepPivot(p), a);
                double xLo = 0, xHi = 0, yLo = 0, yHi = 0;
                r.bounds(&xLo, &xHi, &yLo, &yHi);
                if (xLo < p.box.xMin || xHi > p.box.xMax || yLo < p.box.yMin
                    || yHi > p.box.yMax) {
                    d.warnings << tpl::tr("«%1» при повороте на %2 град. выходит за "
                                          "счётную область — ядро отрежет вылезшую часть",
                                          "'%1' at a rotation by %2 deg. goes beyond the "
                                          "computational domain - the kernel will cut off the "
                                          "protruding part")
                                      .arg(title, formatCompact(a));
                    break;
                }
            }
        }
    }

    // Размещение фигур, окна съёма и контур ДН — по ответам автора 26.10.2026.
    checkPlacement(p, &d);

    bool lossy = false;
    toCp1251(p.description, &lossy);
    if (lossy)
        e << tpl::tr("в описании есть символы, которых нет в CP1251 "
                     "(греческие буквы, стрелки): пишите 'eps', '->'",
                     "the description contains characters that are not in CP1251 (Greek letters, "
                     "arrows): write 'eps', '->'");

    // --- предупреждения: решает пользователь ---

    // Со списком возбуждений на каждом шаге ровно один активный порт — тогда .s
    // честно становится матрицей, и предупреждать не о чем.
    if (p.exciters.isEmpty()) {
        const int active = p.activePorts().size();
        if (active == 0)
            d.warnings << tpl::tr("ни один порт не активен — возбуждения не будет",
                                  "no port is active - there will be no excitation");
        if (active > 1 && p.output.sMatrix)
            d.warnings << tpl::tr("активных портов %1: .s не будет матрицей рассеяния — "
                                  "на каждом порту придёт сумма откликов. Смотрите поля "
                                  "и ДН; для полной матрицы включите возбуждение портов "
                                  "по очереди",
                                  "%1 active ports: .s will not be a scattering matrix - each "
                                  "port will receive the sum of responses. Look at the fields and "
                                  "the radiation pattern; for a full matrix enable exciting the "
                                  "ports in turn")
                              .arg(active);
    } else if (p.ports.size() > p.exciters.size() + 0 && p.output.sMatrix) {
        // Столбец матрицы даёт возбуждение порта. Не перечислили все — матрица неполна.
        QStringList missing;
        for (const Port &p2 : p.ports) {
            if (!p.exciters.contains(p2.id))
                missing << QString::number(p.portOrdinal(p2.id));
        }
        if (!missing.isEmpty())
            d.warnings << tpl::tr("матрица будет неполной: не возбуждаются порты %1 — "
                                  "их столбцы S останутся пустыми",
                                  "the matrix will be incomplete: ports %1 are not excited - "
                                  "their S columns will stay empty")
                              .arg(missing.join(QStringLiteral(", ")));
    }

    if (p.stepCount() > 60) {
        d.warnings << tpl::tr("шагов расчёта %1 — при сетке 240x80 и шаге 1 мм это "
                              "примерно %2 мин счёта",
                              "%1 computation steps - with a 240x80 grid and a 1 mm step this is "
                              "about %2 min of run time")
                          .arg(p.stepCount()).arg(p.stepCount() * 14 / 60);
    }

    // Только «на каждом шаге»: поля лишь на первом шаге пишутся одним явным шагом и
    // стоят как один шаг — предупреждать тут не о чем (так написаны авторские ЛР1, ЛР2).
    if (p.output.fields == OutputSettings::Fields::AllSteps && needsMacro(p))
        d.warnings << tpl::tr("выгрузка полей на каждом из %1 шагов — счёт будет очень "
                              "долгим: ядро переписывает всю сетку на каждом такте",
                              "field export at each of %1 steps - the run will be very long: the "
                              "kernel rewrites the whole grid at every time step")
                          .arg(p.stepCount());

    // ⚠️ Здесь НЕТ проверки одномодовости, и это решение, а не забывчивость (2026-09-06).
    //
    // Отсечку `c/(2a)` считают по ПУСТОМУ волноводу. Стоит заполнить его средой — а
    // ради этого генератор и существует, — и число перестаёт значить что-либо: у
    // импедансно согласованного материала с eps = mu = 2 отсечка вдвое ниже, у
    // диэлектрического заполнения — в sqrt(eps) раз, а у плоской волны в открытом
    // пространстве (рассеяние на теле, плазменный слой) поперечной моды нет вовсе.
    // Проверка ругалась на все три случая: рабочее задание объявлялось ошибочным.
    //
    // Мерило простое: если задание считается ядром, генератор
    // обязан его сохранить и молчать. Одномодовый диапазон никуда не делся — он живёт
    // в калькуляторе и в подсказке под апертурой порта, где его СПРАШИВАЮТ, а не
    // навязывают. То же и с формой импульса: резкое окно стоит во всех авторских
    // заданиях (`samples/TEST/LR2_*.tpl`), это норма, а не повод для замечания.

    return d;
}

/// Сочетание «вариант геометрии + возбуждаемый порт». Каждое даёт свой набор шагов.
/// @note Список сочетаний строит `combos`; порядок: варианты, внутри — возбуждения.
struct Combo {
    /// Вариант геометрии; nullptr — вариантов нет (все фигуры).
    const Variant *variant = nullptr;
    /// Номер возбуждаемого порта; -1 — очереди нет, активны порты по флагу.
    int exciter = -1;
    QString title; ///< для комментария в файле
};

/// @brief Перечисляет сочетания «вариант x возбуждение» задания.
/// @param[in] p  задание
/// @return Не меньше одного сочетания; у каждого готов заголовок для комментария.
QVector<Combo> combos(const Project &p)
{
    QVector<Combo> out;
    const int nv = p.variants.isEmpty() ? 1 : p.variants.size();
    const int ne = p.exciters.isEmpty() ? 1 : p.exciters.size();

    for (int vi = 0; vi < nv; ++vi) {
        const Variant *v = p.variants.isEmpty() ? nullptr : &p.variants.at(vi);
        for (int ei = 0; ei < ne; ++ei) {
            Combo c;
            c.variant = v;
            c.exciter = p.exciters.isEmpty() ? -1 : p.exciters.at(ei);
            QStringList parts;
            if (v)
                parts << tpl::tr("вариант: %1", "variant: %1").arg(v->name);
            if (c.exciter >= 0)
                parts << tpl::tr("возбуждён вход %1",
                                 "excited port %1").arg(p.portOrdinal(c.exciter));
            c.title = parts.join(QStringLiteral(", "));
            out << c;
        }
    }
    return out;
}

/// @brief Заполняет имена выходных файлов шага для сочетания и частоты.
/// @param[in] p     задание
/// @param[in] c     сочетание «вариант + возбуждение»
/// @param[in] freq  частота шага, ГГц; <= 0 — имена без метки частоты (тело макроса)
/// @return Параметры шага с вариантом, возбуждением и именами файлов; частоту,
///         флаги топологии, полей и `.eps` заполняет вызывающий.
StepArgs argsFor(const Project &p, const Combo &c, double freq)
{
    StepArgs a;
    a.variant = c.variant;
    a.exciter = c.exciter;
    a.signalFile = signalFileFor(p, c.variant, c.exciter, freq);
    a.sFile = sMatrixFileFor(p, c.variant);
    a.patternFile = patternFileFor(p, c.variant, c.exciter, freq);
    return a;
}

namespace {

/// Тело макроса: строки, склеенные переносом `\\`.
///
/// Ровно ДВА бэкслеша: с одним макрос не собирается — тело вываливается текстом,
/// параметры не подставляются, ядро падает с abnormal function. У последней строки
/// переноса нет — это признак конца.
///
/// Подпись блока («BLOCK 1;   ! волновод») уходит ЗА перенос: `BLOCK 1;\\   ! волновод`.
/// Так безопасно при любом порядке разбора — препроцессор вырезает `!` первым, а всё
/// после `\\` и так комментарий. Перед переносом её ставить нельзя: вырезав `!`, он
/// унёс бы и сам перенос, и макрос развалился бы. А без подписей открытое заново
/// задание теряло имена всех фигур.
/// @param[in] body  строки макроса: первая — `#define имя параметры; @`, далее тело шага
/// @return Текст макроса; после каждой строки, кроме последней, стоит `\\`
///         (комментарий `!` переносится за него), строки разделены `\n`.
/// @see https://www.tamic.ru/docs/code/lib-prepr.html
QString macroText(const QStringList &body)
{
    QString out;
    for (int i = 0; i < body.size(); ++i) {
        const QString &line = body.at(i);
        const bool last = (i + 1 == body.size());
        const int bang = line.indexOf(QLatin1Char('!'));
        if (last || bang < 0) {
            out += line;
            if (!last)
                out += QStringLiteral("\\\\");
        } else {
            QString code = line.left(bang);
            while (code.endsWith(QLatin1Char(' ')))
                code.chop(1);
            out += code + QStringLiteral("\\\\   ") + line.mid(bang);
        }
        out += QLatin1Char('\n');
    }
    return out;
}

// ---------------------------------------------------------------- угловой свип (запись)

/// Какие профили eps сохраняются в файл на первом записанном шаге свипа.
/// Поворачиваемых фигур среди них нет: профиль у них запрещён (см. validate).
/// @param[in] p  задание со свипом
/// @return Номера прямоугольников с записью `.eps`, кроме поворачиваемых.
QSet<int> sweepEpsShapes(const Project &p)
{
    QSet<int> out = epsProfileShapes(p, nullptr);
    for (int id : p.sweep.shapeIds)
        out.remove(id);
    return out;
}

/// Вход, который возбуждается в свипе: единственный в очереди возбуждений, если она
/// задана (тогда его флаг «активный» роли не играет), иначе — как помечены порты.
/// @param[in] p  задание
/// @return Номер единственного порта очереди возбуждений; -1 — активные по флагу.
int sweepExciter(const Project &p)
{
    return p.exciters.size() == 1 ? p.exciters.first() : -1;
}

/// Опорный шаг: та же сцена без объекта, в свои файлы.
/// @param[in] p  задание со свипом
/// @return Параметры шага: частота — первая частота задания, файлы с добавкой `_ref`,
///         поворачиваемые фигуры не пишутся.
StepArgs sweepReferenceArgs(const Project &p)
{
    StepArgs a;
    a.freq = formatFreq(p.freqs.value(0));
    a.signalFile = p.output.signalFile + QStringLiteral("_ref");
    a.sFile = p.output.sMatrixFile + QStringLiteral("_ref");
    a.patternFile = p.output.patternFile + QStringLiteral("_ref");
    a.dropRotating = true;
    a.withComments = true;
    a.withTopology = p.output.topology;
    a.exciter = sweepExciter(p);
    return a;
}

/// Шаг с объектом, повёрнутым на угол \p angle. Все имена — числами.
/// @param[in] p      задание со свипом
/// @param[in] angle  угол поворота, градусы
/// @return Параметры шага: угол числом, файлы с меткой `sweepFileTag(angle)`.
StepArgs sweepAngleArgs(const Project &p, double angle)
{
    StepArgs a;
    a.freq = formatFreq(p.freqs.value(0));
    a.rotAngle = formatCompact(angle);
    a.signalFile = p.output.signalFile + QLatin1Char('_') + sweepFileTag(angle);
    a.sFile = sweepSFile(p, angle);
    a.patternFile = p.output.patternFile + QLatin1Char('_') + sweepFileTag(angle);
    a.withComments = true;
    a.withTopology = p.output.topology;
    a.exciter = sweepExciter(p);
    return a;
}

/// @brief Возвращает строку-заголовок шага свипа для текста задания.
/// @param[in] angle  угол поворота, градусы
/// @return Строка комментария с `\n` на конце.
QString sweepAngleTitle(double angle)
{
    return tpl::tr("! ===== Поворот объекта: %1 град. =====\n",
                   "! ===== Object rotation: %1 deg. =====\n").arg(formatCompact(angle));
}

/// @brief Возвращает строку-заголовок опорного шага свипа для текста задания.
/// @return Строка комментария с `\n` на конце.
QString sweepReferenceTitle()
{
    return tpl::tr("! ===== Опорный шаг: та же сцена без объекта =====\n",
                   "! ===== Reference step: the same scene without the object =====\n");
}

/// Свип явными шагами — без макроса. Нужен там, где макро-версию не пишут (проверки,
/// служебные утилиты); ядро считает его ровно так же.
/// @param[in] p  задание с действующим свипом
/// @return Полный текст `.tpl`: шапка, опорный шаг (если задан), шаг на каждый угол, `#EOF`.
QString emitSweepMono(const Project &p)
{
    QString out = header(p);
    out += QLatin1Char('\n');

    QSet<int> eps = sweepEpsShapes(p);
    bool first = true;
    if (p.sweep.referenceStep) {
        StepArgs a = sweepReferenceArgs(p);
        a.createEps = eps;
        eps.clear();
        out += sweepReferenceTitle();
        out += stepLines(p, a).join(QLatin1Char('\n'));
        out += QStringLiteral("\n\n");
    }
    for (double angle : p.sweep.angles) {
        StepArgs a = sweepAngleArgs(p, angle);
        a.createEps = eps;
        eps.clear();
        a.withFields = (p.output.fields == OutputSettings::Fields::AllSteps)
                       || (p.output.fields == OutputSettings::Fields::FirstStep && first);
        first = false;
        out += sweepAngleTitle(angle);
        out += stepLines(p, a).join(QLatin1Char('\n'));
        out += QStringLiteral("\n\n");
    }
    out += QStringLiteral("#EOF\n");
    return out;
}

/// Свип макросом: шаг с объектом описан ОДИН раз, угол — параметр.
///
/// Макрос `<имя>rot` с параметрами `Ang__; Tfile__[; Sfile__][; Dfile__]`. Опорный шаг
/// и, при необходимости, первый угол (поля только на первом шаге или сохранение
/// профиля eps) пишутся явно; остальные углы — строками вызова.
/// @param[in] p  задание с действующим свипом
/// @return Тексты `<имя>.tpl` и `<имя>_step.h` и имя включаемого файла.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#sweep
MacroFiles emitSweepMacro(const Project &p)
{
    MacroFiles f;
    f.stepHName = p.name + QStringLiteral("_step.h");
    const QString macroName = (p.name.toLower() + QStringLiteral("rot")).left(32);

    QStringList params;
    params << QStringLiteral("Ang__") << QStringLiteral("Tfile__");
    if (p.output.sMatrix)
        params << QStringLiteral("Sfile__");
    if (p.output.directionalPattern)
        params << QStringLiteral("Dfile__");

    // Что должно случиться ровно один раз, пишется явным шагом: выгрузка полей и
    // сохранение профилей eps. Внутри макроса «первого шага» не существует.
    QSet<int> eps = sweepEpsShapes(p);
    const bool fieldsFirst = (p.output.fields == OutputSettings::Fields::FirstStep);
    const bool explicitFirstAngle = fieldsFirst || (!p.sweep.referenceStep && !eps.isEmpty());

    StepArgs m;
    m.freq = formatFreq(p.freqs.value(0));
    m.rotAngle = QStringLiteral("Ang__");
    m.signalFile = QStringLiteral("Tfile__");
    m.sFile = QStringLiteral("Sfile__");
    m.patternFile = QStringLiteral("Dfile__");
    m.withComments = true; // подписи блоков уходят за перенос, см. macroText
    m.exciter = sweepExciter(p);
    m.withTopology = p.output.topology;
    m.withFields = (p.output.fields == OutputSettings::Fields::AllSteps);

    QStringList body;
    body << QStringLiteral("#define %1 %2; @").arg(macroName, params.join(QStringLiteral("; ")));
    body += stepLines(p, m);

    f.stepH = tpl::tr(
                  "! ==========================================================================\n"
                  "! %1 — шаг с повёрнутым объектом. Угол — параметр макроса.\n"
                  "! Подключается строкой  #include %1  и должен лежать РЯДОМ с .tpl\n"
                  "!\n"
                  "! ДВА бэкслеша в конце строки = \"текст макроса продолжается ниже\".\n"
                  "! У ПОСЛЕДНЕЙ строки макроса переноса НЕТ — это признак конца.\n"
                  "! ==========================================================================\n",
                  "! ==========================================================================\n"
                  "! %1 -- step with the rotated object. The angle is a macro parameter.\n"
                  "! Included with the line  #include %1  and must lie NEXT TO the .tpl\n"
                  "!\n"
                  "! TWO backslashes at the end of a line = \"the macro text continues below\".\n"
                  "! The LAST line of the macro has NO continuation -- that marks the end.\n"
                  "! ==========================================================================\n")
                  .arg(f.stepHName);
    f.stepH += macroText(body);

    QString tpl = header(p);
    tpl += QLatin1Char('\n');
    if (p.sweep.referenceStep) {
        StepArgs a = sweepReferenceArgs(p);
        a.createEps = eps;
        eps.clear();
        tpl += sweepReferenceTitle();
        tpl += stepLines(p, a).join(QLatin1Char('\n'));
        tpl += QStringLiteral("\n\n");
    }
    if (explicitFirstAngle && !p.sweep.angles.isEmpty()) {
        StepArgs a = sweepAngleArgs(p, p.sweep.angles.first());
        a.createEps = eps;
        eps.clear();
        a.withFields = fieldsFirst || m.withFields;
        tpl += tpl::tr("! ===== Шаг записан явно: %1 =====\n",
                       "! ===== Step written explicitly: %1 =====\n")
                   .arg(fieldsFirst ? tpl::tr("поля выгружаются только здесь",
                                              "fields are exported only here")
                                    : tpl::tr("здесь считается профиль eps",
                                              "the eps profile is computed here"));
        tpl += sweepAngleTitle(p.sweep.angles.first());
        tpl += stepLines(p, a).join(QLatin1Char('\n'));
        tpl += QStringLiteral("\n\n");
    }

    tpl += QStringLiteral("#include %1\n\n").arg(f.stepHName); // #include — выше вызовов
    for (int i = explicitFirstAngle ? 1 : 0; i < p.sweep.angles.size(); ++i) {
        const double angle = p.sweep.angles.at(i);
        QStringList args;
        args << formatCompact(angle)
             << p.output.signalFile + QLatin1Char('_') + sweepFileTag(angle);
        if (p.output.sMatrix)
            args << sweepSFile(p, angle);
        if (p.output.directionalPattern)
            args << p.output.patternFile + QLatin1Char('_') + sweepFileTag(angle);
        tpl += QStringLiteral("%1 %2;\n").arg(macroName, args.join(QStringLiteral("; ")));
    }
    tpl += QStringLiteral("\n#EOF\n");
    f.tpl = tpl;
    return f;
}

} // namespace

QString emitMono(const Project &p)
{
    if (p.sweep.active())
        return emitSweepMono(p);

    QString out = header(p);
    out += QLatin1Char('\n');

    const QVector<Combo> list = combos(p);
    bool firstOverall = true;

    // Профиль считается ОДИН раз за всё задание, на первом шаге, где фигура вообще
    // появляется. Дальше шаги читают готовый файл — а это на большой сетке минуты.
    QSet<int> epsDone;

    for (const Combo &c : list) {
        for (int i = 0; i < p.freqs.size(); ++i) {
            const double f = p.freqs.at(i);
            StepArgs a = argsFor(p, c, f);
            a.freq = formatFreq(f);
            a.withComments = true;
            a.createEps = epsProfileShapes(p, c.variant) - epsDone;
            epsDone += a.createEps;
            // Топология своя у каждого сочетания: у вариантов она разная, а .tt
            // называется по файлу сигналов, то есть тоже своя.
            a.withTopology = p.output.topology && (i == 0);
            a.withFields = (p.output.fields == OutputSettings::Fields::AllSteps)
                           || (p.output.fields == OutputSettings::Fields::FirstStep
                               && firstOverall);

            out += c.title.isEmpty()
                       ? tpl::tr("! ===== Шаг: FREQ = %1 ГГц =====\n",
                                 "! ===== Step: FREQ = %1 GHz =====\n").arg(formatFreq(f))
                       : tpl::tr("! ===== %1, FREQ = %2 ГГц =====\n",
                                 "! ===== %1, FREQ = %2 GHz =====\n")
                             .arg(c.title, formatFreq(f));
            out += stepLines(p, a).join(QLatin1Char('\n'));
            out += QStringLiteral("\n\n");
            firstOverall = false;
        }
    }

    out += QStringLiteral("#EOF\n"); // без него ядро не примет файл
    return out;
}

MacroFiles emitMacro(const Project &p)
{
    if (p.sweep.active())
        return emitSweepMacro(p);

    MacroFiles f;
    f.stepHName = p.name + QStringLiteral("_step.h");

    const QVector<Combo> list = combos(p);
    const QString base = p.name.toLower() + QStringLiteral("step");

    QStringList params;
    params << QStringLiteral("Freq__") << QStringLiteral("Tfile__");
    if (p.output.sMatrix)
        params << QStringLiteral("Sfile__");
    if (p.output.directionalPattern)
        params << QStringLiteral("Dfile__");

    f.stepH = tpl::tr(
                  "! ==========================================================================\n"
                  "! %1 — макроопределения шагов расчёта.\n"
                  "! Подключается строкой  #include %1  и должен лежать РЯДОМ с .tpl\n"
                  "!\n"
                  "! ДВА бэкслеша в конце строки = \"текст макроса продолжается ниже\".\n"
                  "! У ПОСЛЕДНЕЙ строки макроса переноса НЕТ — это признак конца.\n"
                  "! ==========================================================================\n",
                  "! ==========================================================================\n"
                  "! %1 -- macro definitions of the computation steps.\n"
                  "! Included with the line  #include %1  and must lie NEXT TO the .tpl\n"
                  "!\n"
                  "! TWO backslashes at the end of a line = \"the macro text continues below\".\n"
                  "! The LAST line of the macro has NO continuation -- that marks the end.\n"
                  "! ==========================================================================\n")
                  .arg(f.stepHName);

    // Топология у разных сочетаний РАЗНАЯ, поэтому на каждое — свой макрос.
    // Имя макроса не длиннее 32 символов — ограничение препроцессора.
    // Какие шаги придётся вынести из макроса и записать явно.
    //
    // Тело макроса ОДНО на все частоты, и «первого шага» внутри него не существует.
    // Поэтому всё, что должно случиться ровно один раз, живёт в явном шаге: выгрузка
    // полей и — с этого места — расчёт профилей eps. Внутри макроса профиль всегда
    // читается из файла.
    QVector<QSet<int>> inlineEps(list.size());
    QVector<bool> inlineFirst(list.size(), false);
    {
        QSet<int> done;
        for (int ci = 0; ci < list.size(); ++ci) {
            inlineEps[ci] = epsProfileShapes(p, list.at(ci).variant) - done;
            done += inlineEps.at(ci);
            inlineFirst[ci] = !inlineEps.at(ci).isEmpty()
                              || (ci == 0
                                  && p.output.fields == OutputSettings::Fields::FirstStep);
        }
    }

    QStringList macroNames;
    for (int ci = 0; ci < list.size(); ++ci) {
        const QString macroName =
            (list.size() == 1 ? base : base + QString::number(ci + 1)).left(32);
        macroNames << macroName;

        StepArgs a = argsFor(p, list.at(ci), -1.0);
        a.freq = QStringLiteral("Freq__");
        a.signalFile = QStringLiteral("Tfile__");
        a.sFile = QStringLiteral("Sfile__");
        a.patternFile = QStringLiteral("Dfile__");
        // Подписи блоков нужны и в теле макроса — иначе, открыв задание снова, человек
        // получит «прямоугольник 1» вместо «волновод: нижняя стенка». Ставятся они
        // ПОСЛЕ переноса (см. macroText).
        a.withComments = true;
        a.withTopology = p.output.topology;
        a.withFields = (p.output.fields == OutputSettings::Fields::AllSteps);

        QStringList body;
        body << QStringLiteral("#define %1 %2; @").arg(macroName,
                                                       params.join(QStringLiteral("; ")));
        body += stepLines(p, a);

        if (ci > 0)
            f.stepH += QLatin1Char('\n');
        if (!list.at(ci).title.isEmpty())
            f.stepH += QStringLiteral("! %1\n").arg(list.at(ci).title);
        f.stepH += macroText(body);
    }

    QString tpl = header(p);
    tpl += QLatin1Char('\n');

    // Шаги идут в порядке сочетаний: явный первый шаг сочетания — прямо перед его
    // вызовами, а не все явные шаги кучей в начале. Иначе при чтении порядок вариантов
    // определялся бы тем, у кого был явный шаг, и сохранённое заново задание меняло бы
    // порядок вариантов и файлов (найдено 2026-09-23 на образце «Плавный переход»).
    tpl += QStringLiteral("#include %1\n\n").arg(f.stepHName); // #include — выше вызовов
    for (int ci = 0; ci < list.size(); ++ci) {
        const Combo &c = list.at(ci);
        if (inlineFirst.at(ci)) {
            StepArgs a0 = argsFor(p, c, p.freqs.first());
            a0.freq = formatFreq(p.freqs.first());
            a0.withComments = true;
            a0.withTopology = p.output.topology;
            a0.withFields = (ci == 0 && p.output.fields == OutputSettings::Fields::FirstStep);
            a0.createEps = inlineEps.at(ci);

            QStringList why;
            if (a0.withFields)
                why << tpl::tr("поля выгружаются только здесь", "fields are exported only here");
            if (!a0.createEps.isEmpty())
                why << tpl::tr("здесь считается профиль eps, дальше берётся из файла",
                               "the eps profile is computed here, later it is taken from the file");

            tpl += tpl::tr("! ===== Шаг записан явно: %1 =====\n",
                           "! ===== Step written explicitly: %1 =====\n")
                       .arg(why.join(QStringLiteral("; ")));
            if (!c.title.isEmpty())
                tpl += QStringLiteral("! %1\n").arg(c.title);
            tpl += stepLines(p, a0).join(QLatin1Char('\n'));
            tpl += QStringLiteral("\n\n");
        }
        if (!c.title.isEmpty())
            tpl += QStringLiteral("! %1\n").arg(c.title);
        for (int i = 0; i < p.freqs.size(); ++i) {
            if (inlineFirst.at(ci) && i == 0)
                continue; // этот шаг уже записан явно
            QStringList args;
            const double f = p.freqs.at(i);
            args << formatFreq(f) << signalFileFor(p, c.variant, c.exciter, f);
            if (p.output.sMatrix)
                args << sMatrixFileFor(p, c.variant);
            if (p.output.directionalPattern)
                args << patternFileFor(p, c.variant, c.exciter, f);
            tpl += QStringLiteral("%1 %2;\n").arg(macroNames.at(ci),
                                                  args.join(QStringLiteral("; ")));
        }
        tpl += QLatin1Char('\n');
    }

    tpl += QStringLiteral("\n#EOF\n");
    f.tpl = tpl;
    return f;
}

QVector<TaskChunk> emitChunks(const Project &p)
{
    QVector<TaskChunk> out;
    const int n = p.chunkCount();

    // Собрать один элемент по готовому заданию — общий хвост для обеих ветвей.
    const auto build = [](const Project &one) {
        TaskChunk c;
        c.name = one.name;
        c.freqs = one.freqs;
        c.sMatrixFiles = sMatrixNames(one);
        if (needsMacro(one)) {
            const MacroFiles f = emitMacro(one);
            c.tpl = f.tpl;
            c.stepH = f.stepH;
            c.stepHName = f.stepHName;
        } else {
            c.tpl = emitMono(one);
        }
        return c;
    };

    if (n <= 1) {
        out << build(p);
        return out;
    }

    // Ширина номера: у ста заданий это `_001`, у десяти — `_01`. Ведущие нули нужны,
    // чтобы файлы в папке шли по порядку, а не 1, 10, 100, 11.
    const int digits = QString::number(n).size();
    const int per = p.chunking.freqsPerTask;

    for (int i = 0; i < n; ++i) {
        Project one = p;
        one.chunking.enabled = false; // копия — обычное задание, делить её больше нечем
        one.freqs = p.freqs.mid(i * per, per);

        const QString tag =
            QStringLiteral("_%1").arg(i + 1, digits, 10, QLatin1Char('0'));
        // Файл падающей волны должен остаться ОБЩИМ: имя фиксируем по исходному
        // заданию, иначе каждая часть запросила бы свой .sig с теми же числами.
        for (Port &port : one.ports) {
            if (port.signal.source == SignalSource::File && port.signal.file.isEmpty())
                port.signal.file = signalFileName(p, port);
        }
        one.name = p.name + tag;
        if (!one.output.signalFile.isEmpty())
            one.output.signalFile += tag;
        if (!one.output.sMatrixFile.isEmpty())
            one.output.sMatrixFile += tag;
        if (!one.output.patternFile.isEmpty())
            one.output.patternFile += tag;
        // Описание — чтобы по открытому файлу было видно, какая это часть и из чего.
        const QString what =
            tpl::tr("Часть %1 из %2: частоты %3...%4 ГГц (%5 шт.) из общей сетки "
                    "%6...%7 ГГц (%8 шт.)",
                    "Part %1 of %2: frequencies %3...%4 GHz (%5 pcs.) out of the common grid "
                    "%6...%7 GHz (%8 pcs.)")
                .arg(i + 1).arg(n)
                .arg(formatCompact(one.freqs.first()), formatCompact(one.freqs.last()))
                .arg(one.freqs.size())
                .arg(formatCompact(p.freqs.first()), formatCompact(p.freqs.last()))
                .arg(p.freqs.size());
        one.description = p.description.isEmpty() ? what
                                                  : p.description + QLatin1Char('\n') + what;
        out << build(one);
    }
    return out;
}

} // namespace tpl
