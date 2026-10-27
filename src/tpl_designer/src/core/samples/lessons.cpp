// Раздел «Учебные задания»: авторские задания лабораторных работ и демо-примеры пакета.
//
// Эти задания писали люди, а не рецепты, — они и ценны как образец того, как задание
// выглядит «от руки». Лежат ресурсом байт в байт и открываются ТЕМ ЖЕ читателем, что
// и «Открыть готовое»: так пользователь видит их в редакторе, а при сохранении
// получает то же задание, записанное генератором.

/// @file lessons.cpp
/// @brief Раздел каталога «Учебные задания»: авторские `.tpl` из ресурса `lessons.qrc`.
///
/// Часть модуля «Ядро TPL Designer», каталог образцов. Задания лежат ресурсом
/// `:/lessons/` байт в байт (CP1251) и читаются тем же читателем `.tpl`, что и
/// открытие готового задания; параметров у этих образцов нет.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#samples

#include "cp1251.h"
#include "lang.h"
#include "sample.h"
#include "tplreader.h"
#include "placement.h"

#include <QFile>

// Ресурсы статической библиотеки сами не регистрируются — нужен явный вызов, и
// сделать его можно только вне пространства имён.
/// @brief Регистрирует ресурс `lessons.qrc` статической библиотеки (`Q_INIT_RESOURCE`).
static void initLessonResources()
{
    Q_INIT_RESOURCE(lessons);
}

namespace tpl::samples {

namespace {

/// @brief Описание одного учебного задания из ресурса (строка таблицы `addLessons`).
struct Lesson {
    /// `Sample::id` образца (`les_*`).
    const char *id;
    /// Имя файла в ресурсе `:/lessons/`; часть до точки становится именем задания.
    const char *file;
    /// Ядро, которым считается задание (по тексту файла моду не определить).
    Kernel kernel;
    // Тексты — парами «русский, английский»: язык выбирается в момент обращения (lang.h).
    /// Название по-русски.
    const char *title;
    /// Название по-английски.
    const char *titleEn;
    /// Строка плитки по-русски.
    const char *summary;
    /// Строка плитки по-английски.
    const char *summaryEn;
    /// Справка по-русски.
    const char *help;
    /// Справка по-английски.
    const char *helpEn;
};

/// @brief Читает учебное задание из ресурса и превращает его в результат рецепта.
///
/// Байты декодируются из CP1251 (`decodeTplBytes`) и разбираются `readTplText`.
/// Описанием задания становятся комментарии `!` шапки до первого тега (не больше 12
/// строк). Если заказана ДН и её окно ядро не закрыло бы до конца счёта, окно
/// переносится в последнюю четверть счёта с подрезкой `patternWindowSafeEnd`, и об
/// этом пишется пояснение.
/// @param[in] l  описание задания
/// @return Задание, паспорт (число шагов, фигур, входов) и пояснения, включая замечания
///         чтения; если ресурс не открылся — пустое задание с пояснением об ошибке.
/// @see https://www.tamic.ru/docs/theory/radiation.html#pattern
Build readLesson(const Lesson &l)
{
    Build b;
    QFile f(QStringLiteral(":/lessons/") + QLatin1String(l.file));
    if (!f.open(QIODevice::ReadOnly)) {
        b.notes << tpl::tr("Не удалось прочитать встроенное задание %1",
                           "Could not read the built-in task %1").arg(QLatin1String(l.file));
        return b;
    }
    const QString text = decodeTplBytes(f.readAll());
    const ReadResult r = readTplText(text);
    b.project = r.project;
    b.project.name = QString::fromLatin1(l.file).section(QLatin1Char('.'), 0, 0);
    // Мода определяется ядром, которым считают, а не текстом файла: в рупоре ЛР2
    // нет плазмы, но считается он X-модой.
    b.project.kernel = l.kernel;

    // Шапка авторского задания — готовое описание: берём комментарии до первого #.
    QStringList head;
    for (const QString &line : text.split(QLatin1Char('\n'))) {
        const QString t = line.trimmed();
        if (t.startsWith(QLatin1Char('#')) && !t.startsWith(QLatin1String("#TMC_RT_H")))
            break;
        if (!t.startsWith(QLatin1Char('!')))
            continue;
        QString c = t.mid(1).trimmed();
        if (c.startsWith(QLatin1String("====")) || c.startsWith(QLatin1String("----")) || c.isEmpty())
            continue;
        head << c;
        if (head.size() >= 12)
            break;
    }
    b.project.description = head.join(QLatin1Char('\n'));
    b.theory << Row{tpl::tr("Шагов расчёта", "Calculation steps"), QString::number(b.project.stepCount())};
    b.theory << Row{tpl::tr("Фигур / входов", "Shapes / ports"), QStringLiteral("%1 / %2")
                                                           .arg(b.project.shapes.size())
                                                           .arg(b.project.ports.size())};
    b.notes << tpl::tr("Авторское задание прочитано так же, как «Открыть готовое». "
                       "Сохранив его, вы получите то же задание, записанное генератором: "
                       "макросы-константы автора раскроются в числа.",
                       "The author's task was read the same way as by \"Open existing\". If you "
                       "save it, you will get the same task written by the generator: the author's "
                       "constant macros will be expanded into numbers.");
    for (const QString &w : r.warnings)
        b.notes << tpl::tr("При чтении: ", "While reading: ") + w;

    // Окно ДН, которое ядро не закроет (кончается не раньше TIME, Б-13), даёт расчёт
    // без файлов ДН и код 2. В демонстрационных заданиях автора так и есть (demo_h:
    // окно 7...8 нс при счёте до 3 нс) — переносим окно в последнюю четверть счёта и
    // говорим об этом, а не выпускаем заведомо пустой результат.
    OutputSettings &o = b.project.output;
    if (o.directionalPattern && !patternWindowCloses(b.project, o.patternTMin, o.patternTMax)) {
        const double t = b.project.timeMax;
        const double lo = t * 0.75;
        const double hi = patternWindowSafeEnd(b.project, lo, t);
        if (hi > lo) {
            b.notes << tpl::tr("Окно диаграммы направленности автора (%1...%2 нс) ядро не "
                               "закрыло бы: счёт идёт до %3 нс. Окно перенесено на "
                               "%4...%5 нс.",
                               "The kernel would not close the author's radiation pattern window "
                               "(%1...%2 ns): the run lasts until %3 ns. The window has been moved "
                               "to %4...%5 ns.")
                           .arg(o.patternTMin).arg(o.patternTMax).arg(t).arg(lo).arg(hi);
            o.patternTMin = lo;
            o.patternTMax = hi;
        }
    }
    return b;
}

} // namespace

void addLessons(QVector<Sample> *out)
{
    initLessonResources();

    static const Lesson list[] = {
        {"les_lr1_wg", "LR1_wg.tpl", Kernel::H, "ЛР1: волновод с диэлектрической вставкой",
         "Lab 1: waveguide with a dielectric insert",
         "Свип по частоте, WR-229, вставка eps = 2",
         "Frequency sweep, WR-229, insert eps = 2",
         "Лабораторная работа 1. Прямоугольный волновод R40 / WR-229 с диэлектрической "
         "вставкой, свип 3.3-4.9 ГГц. Порт 1 активный, порт 2 пассивный; поле и топология "
         "выгружаются только на первом шаге.",
         "Laboratory work 1. Rectangular waveguide R40 / WR-229 with a dielectric insert, sweep "
         "3.3-4.9 GHz. Port 1 is active, port 2 is passive; fields and topology are exported only "
         "at the first step."},
        {"les_lr1_horn", "LR1_rupor.tpl", Kernel::H, "ЛР1: рупор",
         "Lab 1: horn",
         "Рупор в открытом пространстве, H-мода",
         "Horn in open space, H-mode",
         "Лабораторная работа 1. Питающий волновод плавно раскрывается в апертуру; за "
         "раскрывом — открытая область, ограниченная поглотителем. Диаграмма направленности "
         "пишется для TMC_DN.",
         "Laboratory work 1. The feed waveguide smoothly flares into the aperture; beyond the flare "
         "there is an open region bounded by an absorber. The radiation pattern is written for "
         "TMC_DN."},
        {"les_lr2_plasma", "LR2_plasma.tpl", Kernel::X, "ЛР2: замагниченная плазма",
         "Lab 2: magnetized plasma",
         "Квадрат плазмы с подмагничиванием в волноводе",
         "Square of magnetized plasma in a waveguide",
         "Лабораторная работа 2. Небольшой квадрат плазмы в центре волновода вместе с "
         "магнитным полем в той же области — гиротропная среда. Считается ядром X-моды.",
         "Laboratory work 2. A small square of plasma in the center of the waveguide together with "
         "a magnetic field in the same region, i.e. a gyrotropic medium. Computed by the X-mode "
         "kernel."},
        {"les_lr2_horn", "LR2_rupor.tpl", Kernel::X, "ЛР2: рупор, X-мода",
         "Lab 2: horn, X-mode",
         "Тот же рупор, посчитанный X-модой",
         "The same horn computed in X-mode",
         "Лабораторная работа 2. Тот же рупор, что в ЛР1, но для ядра X-моды: поле E лежит "
         "в плоскости рисунка. Сравните ДН двух мод.",
         "Laboratory work 2. The same horn as in Lab 1, but for the X-mode kernel: the E field lies "
         "in the plane of the drawing. Compare the radiation patterns of the two modes."},
        {"les_demo_h", "demo_h.tpl", Kernel::H, "Демо: плоский волновод (H)",
         "Demo: parallel-plate waveguide (H)",
         "Короткий учебный пример из поставки ядра",
         "Short training example shipped with the kernel",
         "Самодостаточный учебный пример из поставки TMC Suite: канал из двух металлических "
         "полос, вход слева, поглотитель по краям. Маленькая сетка — считается быстро.",
         "Self-contained training example shipped with TMC Suite: a channel of two metal strips, a "
         "port on the left, an absorber along the edges. Small grid, so it computes quickly."},
        {"les_demo_x", "demo_x.tpl", Kernel::X, "Демо: плоский волновод (X)",
         "Demo: parallel-plate waveguide (X)",
         "Тот же пример для ядра X-моды",
         "The same example for the X-mode kernel",
         "Учебный пример из поставки TMC Suite для ядра X-моды. Формат файла и заголовок "
         "#TMC_RT_H у обеих мод одинаковые — отдельного #TMC_RT_X не существует.",
         "Training example shipped with TMC Suite for the X-mode kernel. The file format and the "
         "#TMC_RT_H header are the same for both modes; a separate #TMC_RT_X does not exist."},
        {"les_horn_demo", "horn_demo.tpl", Kernel::H, "Планарный рупор с рамкой поглотителя",
         "Planar horn with an absorber frame",
         "Демо-задание пакета: излучающий рупор в открытой области",
         "Package demo task: a radiating horn in an open region",
         "Задание из демонстрационного набора TMC Suite: питающий канал плавно раскрывается "
         "в апертуру и излучает в пространство, ограниченное рамкой поглотителя. Отрезки стенок "
         "записаны с повтором вершины — программа читает это без замечаний.",
         "Task from the TMC Suite demo set: the feed channel smoothly flares into the aperture and "
         "radiates into the space bounded by an absorber frame. Wall segments are written with a "
         "repeated vertex; the program reads this without issues."},
    };

    for (const Lesson &l : list) {
        Sample s;
        s.id = QLatin1String(l.id);
        s.section = QStringLiteral("lesson");
        s.title = tpl::tr(l.title, l.titleEn);
        s.summary = tpl::tr(l.summary, l.summaryEn);
        s.standard = QStringLiteral("TMC Suite, samples/") + QLatin1String(l.file);
        s.help = tpl::tr(l.help, l.helpEn);
        s.kernel = l.kernel;
        const Lesson copy = l;
        s.build = [copy](const Values &) { return readLesson(copy); };
        *out << s;
    }
}

} // namespace tpl::samples
