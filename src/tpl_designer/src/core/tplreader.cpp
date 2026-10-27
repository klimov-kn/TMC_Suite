/// @file tplreader.cpp
/// @brief Чтение готового `.tpl` обратно в модель: препроцессор по правилам ядра,
///        разбор секций шагов и сборка `Project`.
///
/// Часть ядра редактора заданий. Три слоя: препроцессор (`!`, `\\`, `#include`,
/// `#define`) повторяет библиотеку PREPR ядра; разбор плоского текста делит его на
/// шаги и блоки (`RawStep`, `RawBlock`); сборка модели склеивает блоки в фигуры и
/// порты, восстанавливает варианты, очередь возбуждений, частоты, угловой свип, сигнал
/// из файла и имена фигур из комментариев. Чтение «мягкое»: что модель не выражает,
/// попадает в предупреждения с номером строки исходного файла.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#reader
#include "tplreader.h"
#include "lang.h"

#include "sigfile.h"

#include "cp1251.h"
#include "emitter.h"
#include "expr.h"
#include "placement.h"

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QPointF>
#include <QRegularExpression>
#include <QSet>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace tpl {
namespace {

// ===========================================================================
//  Препроцессор
//
//  Повторяет `src/libs/PREPR` из TMC Suite. Правила брались из кода, а не из
//  справочника: ядро правее справочника, и половины этих тонкостей в нём нет.
// ===========================================================================

/// Символ имени макроса. ⚠️ Именно ASCII-диапазоны, как в `def_gets`:
/// QChar::isLetterOrNumber() принял бы кириллицу и разъехался бы с ядром.
/// @param[in] c  символ
/// @return true — латинская буква, цифра или `_`.
bool isIdentChar(QChar c)
{
    const ushort u = c.unicode();
    return (u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z') || (u >= '0' && u <= '9') || u == '_';
}

/// Буфер ядра — цепочка строк, оканчивающихся '\0'. После склейки по `\\`
/// внутри одной записи появляются настоящие переводы строк.
using Records = QStringList;

/// `DeleteComment()` из `PREPR/Prep_mn.cpp`: от первого '!' и до конца строки
/// текст заменяется на " \n".
///
/// ⚠️ Порядок важен: комментарии режутся ДО склейки по `\\`, поэтому '!' перед
/// `\\` убивает перенос строки. Ядро ведёт себя именно так.
/// @param[in] line  одна строка исходного текста (с `\n` или без)
/// @return Строка без комментария, всегда оканчивающаяся `\n`.
QString deleteComment(QString line)
{
    const int i = line.indexOf(QLatin1Char('!'));
    if (i >= 0)
        return line.left(i) + QLatin1String(" \n");
    if (!line.endsWith(QLatin1Char('\n')))
        line += QLatin1Char('\n');
    return line;
}

/// @brief Делит текст на записи по строкам и сразу вырезает комментарии `!`.
/// @param[in] text  исходный текст задания
/// @return Записи буфера, по одной на строку исходного файла.
Records loadRecords(const QString &text)
{
    Records recs;
    int start = 0;
    while (start < text.size()) {
        const int nl = text.indexOf(QLatin1Char('\n'), start);
        if (nl < 0) {
            recs << deleteComment(text.mid(start));
            break;
        }
        recs << deleteComment(text.mid(start, nl - start + 1));
        start = nl + 1;
    }
    return recs;
}

/// `def_sl()`: найденные `\\` и весь текст после них до конца строки выбрасываются,
/// а строка склеивается со следующей через " \n".
///
/// ⚠️ Ровно ДВА бэкслеша. С одним ядро макрос не соберёт — тело вывалится текстом.
/// @param[in,out] recs  записи буфера; склеенные записи удаляются из списка
void joinBackslash(Records *recs)
{
    for (int i = 0; i < recs->size(); ++i) {
        for (;;) {
            const int pos = recs->at(i).indexOf(QLatin1String("\\\\"));
            if (pos < 0)
                break;
            QString head = recs->at(i).left(pos) + QLatin1String(" \n");
            if (i + 1 < recs->size()) {
                head += recs->at(i + 1);
                recs->removeAt(i + 1);
            }
            recs->replace(i, head);
        }
    }
}

/// Позиция директивы `#define` / `#include` в записи. Ядро сравнивает 8 (9)
/// символов вместе с разделителем, поэтому `#defineX` директивой не считается.
/// @param[in] rec   запись буфера
/// @param[in] name  директива (`#define` или `#include`)
/// @param[in] last  true — искать последнее вхождение, false — первое
/// @return Позиция директивы, за которой стоит пробел или табуляция; -1 — нет.
int findDirective(const QString &rec, QLatin1String name, bool last = false)
{
    int found = -1;
    int from = 0;
    for (;;) {
        const int p = rec.indexOf(name, from);
        if (p < 0)
            return found;
        const int after = p + name.size();
        if (after < rec.size()
            && (rec.at(after) == QLatin1Char(' ') || rec.at(after) == QLatin1Char('\t'))) {
            if (!last)
                return p;
            found = p;
        }
        from = p + 1;
    }
}

/// @brief Возвращает первую строку текста без крайних пробелов (для сообщений).
/// @param[in] s  текст
/// @return Первая строка.
QString firstLineOf(const QString &s)
{
    const int nl = s.indexOf(QLatin1Char('\n'));
    return (nl < 0 ? s : s.left(nl)).trimmed();
}

/// @brief Разобранное макроопределение `#define`.
struct Macro {
    /// Имя макроса (латиница, цифры, `_`).
    QString name;
    /// Формальные параметры в порядке объявления; пусто — макрос-константа.
    QStringList params;
    /// Тело макроса без крайних пробелов.
    QString body;
};

/// Разбирает `#define ИМЯ [пар1; пар2;] @ тело` (см. `def_separ_mac_name`).
/// @param[in]  text  текст определения, начиная с `#define`
/// @param[out] m     разобранный макрос
/// @param[out] why   причина отказа (на текущем языке), если разбор не удался
/// @return true — определение разобрано.
bool parseMacro(const QString &text, Macro *m, QString *why)
{
    const int at = text.indexOf(QLatin1Char('@'));
    if (at < 0) {
        *why = tpl::tr("нет разделителя '@'", "no '@' separator");
        return false;
    }
    if (at < 8) {
        *why = tpl::tr("пустое имя", "empty name");
        return false;
    }

    const QString head = text.mid(8, at - 8);
    // Пробел после '@' — оформление, а не часть тела. Если его не срезать, каждая
    // подстановка добавляет лишний пробел, и текст расходится с тем, что пишет ядро.
    m->body = text.mid(at + 1).trimmed();

    const QStringList parts = head.split(QLatin1Char(';'));
    static const QRegularExpression space(QStringLiteral("\\s+"));
    const QStringList firstWords = parts.value(0).split(space, Qt::SkipEmptyParts);
    if (firstWords.isEmpty()) {
        *why = tpl::tr("нет имени макроса", "no macro name");
        return false;
    }
    m->name = firstWords.at(0);
    for (int i = 1; i < firstWords.size(); ++i)
        m->params << firstWords.at(i);
    for (int i = 1; i < parts.size(); ++i) {
        const QString p = parts.at(i).trimmed();
        if (!p.isEmpty())
            m->params << p;
    }

    // ⚠️ Длину имени и число параметров НЕ проверяем. Справочник обещает «32», но
    // код препроцессора не проверяет длину вообще, а параметров держит до 59
    // (BUF_PAR_MAX = 60 в PREPR/Prepr1.h). Отвергать по документу чужой файл,
    // который ядро прекрасно считает, — ровно та ошибка, ради которой мы тут.
    for (QChar c : m->name) {
        if (!isIdentChar(c)) {
            *why = tpl::tr("в имени недопустимый символ", "invalid character in the name");
            return false;
        }
    }
    return true;
}

/// `def_buf_gets_param`: первая и последняя скобки фактического параметра
/// заменяются пробелами (tpl-guide 9.2 — та самая ловушка со `Stm (Stp 10; 2;);`).
/// @param[in] arg  фактический параметр без крайних пробелов
/// @return Параметр, у которого внешние `(` и `)` заменены пробелами; иначе как есть.
QString stripOuterParens(const QString &arg)
{
    if (arg.size() >= 2 && arg.startsWith(QLatin1Char('(')) && arg.endsWith(QLatin1Char(')')))
        return QLatin1Char(' ') + arg.mid(1, arg.size() - 2) + QLatin1Char(' ');
    return arg;
}

/// Подставляет фактические параметры в тело макроса — по границам идентификаторов.
/// @param[in] m     макрос
/// @param[in] args  фактические параметры в порядке формальных
/// @return Тело макроса с подставленными параметрами.
QString expandBody(const Macro &m, const QStringList &args)
{
    QString out;
    out.reserve(m.body.size() + 64);
    int i = 0;
    while (i < m.body.size()) {
        if (!isIdentChar(m.body.at(i))) {
            out += m.body.at(i);
            ++i;
            continue;
        }
        int j = i;
        while (j < m.body.size() && isIdentChar(m.body.at(j)))
            ++j;
        const QString tok = m.body.mid(i, j - i);
        const int idx = m.params.indexOf(tok);
        out += (idx >= 0) ? args.value(idx) : tok;
        i = j;
    }
    return out;
}

/// Один проход подстановки макроса по тексту.
///
/// ⚠️ Совпадение — по ЦЕЛОМУ идентификатору, а не по подстроке: иначе макрос `a`
/// изуродовал бы слово `radian`. Ядро (`def_buf_gets_mac`) читает максимальный
/// кусок из [A-Za-z0-9_] и сравнивает целиком — делаем так же.
/// @param[in]     text      текст записи
/// @param[in]     m         макрос
/// @param[in,out] calls     счётчик раскрытых вызовов с параметрами
/// @param[in,out] warnings  куда дописываются замечания о неразобранных вызовах
/// @return Текст с раскрытыми вызовами; неразобранный вызов остаётся как есть.
QString substitute(const QString &text, const Macro &m, int *calls, QStringList *warnings)
{
    QString out;
    out.reserve(text.size());
    int i = 0;
    while (i < text.size()) {
        if (!isIdentChar(text.at(i))) {
            out += text.at(i);
            ++i;
            continue;
        }
        int j = i;
        while (j < text.size() && isIdentChar(text.at(j)))
            ++j;
        const QString tok = text.mid(i, j - i);
        if (tok != m.name) {
            out += tok;
            i = j;
            continue;
        }

        if (m.params.isEmpty()) {
            out += m.body;
            i = j;
            continue; // константы вызовами не считаем — макро-версию делают параметры
        }

        // Фактические параметры: каждый до ';' на нулевой глубине скобок.
        int k = j;
        if (k < text.size())
            ++k; // ядро съедает разделитель, стоящий сразу за именем
        QStringList args;
        bool bad = false;
        for (int a = 0; a < m.params.size() && !bad; ++a) {
            int depth = 0;
            QString arg;
            bool closed = false;
            while (k < text.size()) {
                const QChar c = text.at(k);
                if (c == QLatin1Char('\n')) { // параметры вызова не переносятся
                    bad = true;
                    break;
                }
                if (c == QLatin1Char('(')) {
                    ++depth;
                } else if (c == QLatin1Char(')')) {
                    --depth;
                } else if (c == QLatin1Char(';') && depth == 0) {
                    ++k;
                    closed = true;
                    break;
                }
                arg += c;
                ++k;
            }
            if (!closed)
                bad = true;
            else
                args << stripOuterParens(arg.trimmed()); // пробелы вокруг аргумента — оформление
        }

        if (bad) {
            *warnings << tpl::tr("вызов макроса «%1» не разобран: не хватает "
                                 "фактических параметров (ожидалось %2)",
                                 "macro call '%1' not parsed: not enough actual parameters (%2 "
                                 "expected)")
                             .arg(m.name).arg(m.params.size());
            out += tok;
            i = j;
            continue;
        }

        out += expandBody(m, args);
        ++*calls;
        i = k;
    }
    return out;
}

/// Ищет файл в папке задания, НЕ различая регистр имени.
///
/// ⚠️ Нужно именно на macOS и Linux. Задания пишут на Windows, где регистр в именах
/// файлов не важен, и `#include lr2_step.h` там открывает `LR2_step.h` без разговоров.
/// На чужой системе такое задание молча теряло бы все макросы: файл «не найден»,
/// шаги не раскрылись. Ядро TMC на этих системах ищет так же (см. `TMC_Suite_Qt`,
/// «поиск имени файла без учёта регистра»), поэтому и мы обязаны.
///
/// Точное совпадение проверяется первым: если файл есть, каталог не перебираем.
/// Возвращает пустую строку, если ничего похожего нет.
/// @param[in] dir   папка задания
/// @param[in] name  имя файла из `#include`
/// @return Полный путь к найденному файлу или пустая строка.
QString findFileIgnoringCase(const QString &dir, const QString &name)
{
    const QString exact = QDir(dir).filePath(name);
    if (QFileInfo::exists(exact))
        return exact;

    const QDir d(dir);
    const QStringList entries = d.entryList(QDir::Files);
    for (const QString &e : entries) {
        if (e.compare(name, Qt::CaseInsensitive) == 0)
            return d.filePath(e);
    }
    return {};
}

/// @brief Состояние обработки `#include`: папка, куда складывать имена и замечания,
///        и лимит вставок против кольцевого включения.
struct IncludeCtx {
    /// Папка задания; пусто — включаемые файлы не ищутся.
    QString baseDir;
    /// Куда дописывать имена вставленных файлов (как в `#include`).
    QStringList *included = nullptr;
    /// Куда дописывать предупреждения.
    QStringList *warnings = nullptr;
    /// Куда дописывать ошибки.
    QStringList *errors = nullptr;
    /// Сколько вставок ещё разрешено (64 на задание).
    int budget = 64;
};

/// @brief Заменяет строки `#include` содержимым файлов из папки задания.
///
/// После каждой вставки повторяет склейку по `\\`, как ядро; вложенные `#include`
/// обрабатываются тем же проходом. Не найденный файл — предупреждение.
/// @param[in,out] recs  записи буфера
/// @param[in,out] ctx   папка, накопители имён и замечаний, лимит вставок
void processIncludes(Records *recs, IncludeCtx *ctx)
{
    int i = 0;
    while (i < recs->size()) {
        const int pos = findDirective(recs->at(i), QLatin1String("#include"));
        if (pos < 0) {
            ++i;
            continue;
        }
        if (--ctx->budget < 0) {
            *ctx->errors << tpl::tr("слишком много #include — похоже на кольцевую вставку",
                                    "too many #include - looks like a circular inclusion");
            return;
        }

        const QString rec = recs->at(i);
        const QString prefix = rec.left(pos);
        const QString tail = rec.mid(pos + 9); // после "#include "
        const int nl = tail.indexOf(QLatin1Char('\n'));
        const QString name = (nl < 0 ? tail : tail.left(nl)).trimmed();

        recs->removeAt(i);

        if (name.isEmpty()) {
            *ctx->warnings << tpl::tr("строка #include без имени файла — пропущена",
                                      "#include line without a file name - skipped");
            continue;
        }

        // ⚠️ Ядро ищет включаемый файл РОВНО в папке задания и больше нигде:
        // имя собирается как GetCurrentPath() + "\" + имя (PREPR/DEFMIS.CPP:125),
        // а ветка с путями поиска целиком закомментирована.
        //
        // Регистр не сверяем: на Windows им занимается файловая система, а на macOS
        // и Linux задание, написанное на Windows, иначе молча теряло бы все макросы.
        QString content;
        QString err;
        bool loaded = false;
        if (!ctx->baseDir.isEmpty()) {
            const QString full = findFileIgnoringCase(ctx->baseDir, name);
            if (!full.isEmpty())
                loaded = readTplFileText(full, &content, &err);
        }
        if (!loaded) {
            *ctx->warnings << tpl::tr("включаемый файл «%1» не найден рядом с заданием — "
                                      "макросы из него не раскрыты, задание прочитано "
                                      "неполно",
                                      "include file '%1' was not found next to the task - its "
                                      "macros are not expanded, the task is read incompletely").arg(name);
            continue;
        }

        *ctx->included << name;

        Records sub = loadRecords(content);
        if (!prefix.trimmed().isEmpty())
            sub.prepend(prefix + QLatin1Char('\n'));
        for (int k = sub.size() - 1; k >= 0; --k)
            recs->insert(i, sub.at(k));

        joinBackslash(recs); // ядро повторяет def_sl после каждой вставки
        // i не двигаем: вложенные #include внутри вставленного текста тоже наши
    }
}

/// @brief Раскрывает все `#define` от последнего к первому, каждый только ниже своего места.
///
/// Строка определения заменяется пустой (перевод строки сохраняется, чтобы номера
/// строк в предупреждениях совпадали с исходным файлом). Не больше 1024 определений.
/// @param[in,out] recs  записи буфера
/// @param[in,out] res   итог препроцессора: счётчик вызовов, замечания, ошибки
/// @see https://www.tamic.ru/docs/code/lib-prepr.html
void expandMacros(Records *recs, PreprocessResult *res)
{
    for (int guard = 0; guard < 1024; ++guard) {
        int idx = -1;
        int pos = -1;
        for (int i = recs->size() - 1; i >= 0 && idx < 0; --i) {
            const int p = findDirective(recs->at(i), QLatin1String("#define"), /*last=*/true);
            if (p >= 0) {
                idx = i;
                pos = p;
            }
        }
        if (idx < 0)
            return;

        QString text = recs->at(idx).mid(pos);
        if (text.endsWith(QLatin1Char('\n')))
            text.chop(1); // ядро затирает последний символ логической строки

        // Определение удаляется из буфера ДО подстановки — как в def_main_macdef.
        //
        // ⚠️ Перевод строки при этом СОХРАНЯЕМ, иначе строка исчезает целиком и весь
        // текст ниже съезжает вверх. Съезжали и номера строк в предупреждениях: у
        // авторского `horn.tpl` с одиннадцатью `#define` замечание к блоку на строке 56
        // приходило как «строка 45», а там лежит `#END_PARM`. Человек идёт смотреть и
        // не находит ничего. Пустая строка на месте определения ничего не стоит:
        // разбору она безразлична, а номера остаются настоящими.
        const QString wasRec = recs->at(idx);
        QString head = wasRec.left(pos);
        if (wasRec.endsWith(QLatin1Char('\n')) && !head.endsWith(QLatin1Char('\n')))
            head += QLatin1Char('\n');
        recs->replace(idx, head);

        Macro m;
        QString why;
        if (!parseMacro(text, &m, &why)) {
            res->warnings << tpl::tr("макроопределение не разобрано (%1): %2",
                                     "macro definition not parsed (%1): %2")
                                 .arg(why, firstLineOf(text));
            continue;
        }

        // ⚠️ Подстановка идёт только ВНИЗ от места определения — как в ядре.
        for (int i = idx; i < recs->size(); ++i)
            recs->replace(i, substitute(recs->at(i), m, &res->macroCalls, &res->warnings));
    }
    res->errors << tpl::tr("слишком много макроопределений — разбор остановлен",
                           "too many macro definitions - parsing stopped");
}

// ===========================================================================
//  Разбор плоского текста
// ===========================================================================

/// Строка начинается с ключевого слова, за которым стоит разделитель.
/// ⚠️ Разделитель обязателен: иначе `TIME_UNIT` совпал бы с `TIME`, а `FREQ_UNIT`
/// с `FREQ`. В ядре ровно поэтому у всех констант хвостовой пробел.
/// @param[in]  line  строка плоского текста
/// @param[in]  key   ключевое слово
/// @param[out] rest  остаток строки после ключевого слова (nullptr — не нужен)
/// @return true — строка начинается с ключевого слова, за которым не идёт символ имени.
bool matchKey(const QString &line, QLatin1String key, QString *rest)
{
    if (!line.startsWith(key))
        return false;
    const int n = key.size();
    if (n < line.size()) {
        const QChar c = line.at(n);
        if (isIdentChar(c))
            return false;
    }
    if (rest)
        *rest = line.mid(n);
    return true;
}

/// Делит хвост примитива на параметры по ';'. Пустые хвосты отбрасываются,
/// пробелы и переводы строк внутри параметра снимаются (`expr_del_Blanks2`).
/// @param[in] text  хвост строки примитива
/// @return Параметры без крайних пробелов; хвостовые пустые убраны.
QStringList splitParams(const QString &text)
{
    QStringList out = text.split(QLatin1Char(';'));
    for (QString &s : out)
        s = s.trimmed();
    while (!out.isEmpty() && out.constLast().isEmpty())
        out.removeLast();
    return out;
}

/// @brief Вид примитива блока: прямоугольник, круг, полигон, вход X/Y, файл
///        распределения, неизвестный.
enum class Prim { Rect, Circle, Polygon, InputX, InputY, FileEps, Unknown };

/// @brief Строка таблицы ключевых слов примитивов.
struct PrimSpec {
    /// Ключевое слово как в языке (`RECT_STAT_N` ...).
    QLatin1String word;
    /// Вид примитива.
    Prim prim;
    /// Плазменный суффикс: "", "_N", "_B", "_Y", "_S".
    const char *suffix;
    /// true — движущийся блок (`*_MOVE`).
    bool moving;
};

// Порядок — от длинного к короткому: «RECT_STAT» не должен съесть «RECT_STAT_N».
/// Таблица ключевых слов примитивов, которые понимает чтение.
const PrimSpec kPrims[] = {
    {QLatin1String("RECT_STAT_N"),    Prim::Rect,    "_N", false},
    {QLatin1String("RECT_STAT_B"),    Prim::Rect,    "_B", false},
    {QLatin1String("RECT_STAT_Y"),    Prim::Rect,    "_Y", false},
    {QLatin1String("RECT_STAT_S"),    Prim::Rect,    "_S", false},
    {QLatin1String("RECT_STAT"),      Prim::Rect,    "",   false},
    {QLatin1String("RECT_MOVE"),      Prim::Rect,    "",   true},
    {QLatin1String("CIRCLE_STAT_N"),  Prim::Circle,  "_N", false},
    {QLatin1String("CIRCLE_STAT_B"),  Prim::Circle,  "_B", false},
    {QLatin1String("CIRCLE_STAT_Y"),  Prim::Circle,  "_Y", false},
    {QLatin1String("CIRCLE_STAT_S"),  Prim::Circle,  "_S", false},
    {QLatin1String("CIRCLE_STAT"),    Prim::Circle,  "",   false},
    {QLatin1String("CIRCLE_MOVE"),    Prim::Circle,  "",   true},
    {QLatin1String("POLYGON_STAT_N"), Prim::Polygon, "_N", false},
    {QLatin1String("POLYGON_STAT_B"), Prim::Polygon, "_B", false},
    {QLatin1String("POLYGON_STAT_Y"), Prim::Polygon, "_Y", false},
    {QLatin1String("POLYGON_STAT_S"), Prim::Polygon, "_S", false},
    {QLatin1String("POLYGON_STAT"),   Prim::Polygon, "",   false},
    {QLatin1String("POLYGON_MOVE"),   Prim::Polygon, "",   true},
    {QLatin1String("INPUT_X"),        Prim::InputX,  "",   false},
    {QLatin1String("INPUT_Y"),        Prim::InputY,  "",   false},
    {QLatin1String("FILE_N"),         Prim::FileEps, "_N", false},
    {QLatin1String("FILE_B"),         Prim::FileEps, "_B", false},
    {QLatin1String("FILE_Y"),         Prim::FileEps, "_Y", false},
    {QLatin1String("FILE"),           Prim::FileEps, "",   false},
};

/// @brief Блок `BLOCK n; ... END_B` как он записан в файле, до сборки модели.
struct RawBlock {
    /// Номер блока.
    int id = 0;
    /// Номер строки исходного файла (для сообщений).
    int line = 0;
    /// Вид примитива.
    Prim prim = Prim::Unknown;
    QString word;      ///< как написано в файле — для сообщений
    QString suffix;    ///< "", "_N", "_B", "_Y", "_S"
    /// Движущийся блок (`*_MOVE`).
    bool moving = false;
    QStringList params;                       ///< для RECT/CIRCLE/INPUT/FILE, первый — материал
    QVector<QPair<QString, QString>> points;  ///< для POLYGON
    QStringList extra;                        ///< хвост POLYGON_MOVE (скорости)
    /// Текст блока без разбора (для сообщений).
    QString raw;
};

/// @brief Содержимое `#PARM` шага; величины уже переведены в мм, нс, ГГц.
struct RawParams {
    /// Единица углов (`ANGLE_UNIT`).
    AngleUnit angle = AngleUnit::Radian;
    /// Множитель `LONG_UNIT` -> мм.
    double longToMm = 1.0;
    /// Множитель `TIME_UNIT` -> нс.
    double timeToNs = 1.0;
    /// Множитель `FREQ_UNIT` -> ГГц.
    double freqToGHz = 1.0;
    /// Единица длины файла (переносится в модель как единица ввода).
    LengthUnit lengthUnit = LengthUnit::Millimeter;
    /// Шаг сетки `DELTA`, мм.
    double delta = 1.0;
    /// Начало счёта `TIME`, нс.
    double timeMin = 0.0;
    /// Конец счёта `TIME`, нс.
    double timeMax = 0.0;
    /// Граница области `X_MIN`, мм (остальные — `xMax`, `yMin`, `yMax`).
    double xMin = 0.0, xMax = 0.0, yMin = 0.0, yMax = 0.0;
    /// Частота шага `FREQ`, ГГц.
    double freq = 0.0;
    /// `TIME_UNIT` задан явно.
    bool haveTimeUnit = false;
    /// Хоть одна строка `#PARM` разобрана.
    bool haveAny = false;
};
/// @var RawParams::xMax
/// Граница области `X_MAX`, мм.
/// @var RawParams::yMin
/// Граница области `Y_MIN`, мм.
/// @var RawParams::yMax
/// Граница области `Y_MAX`, мм.

/// @brief Содержимое `#OUTPUT` шага.
struct RawOutput {
    /// Имя файла сигналов (`FILE`).
    QString file;
    /// Есть `TOPOLOGY;`.
    bool topology = false;
    /// Есть `FIELDS;`.
    bool fields = false;
    /// Есть строка `SMATRIX`.
    bool sMatrix = false;
    /// Имя файла S-матрицы.
    QString sFile;
    /// Начало окна S-матрицы, нс.
    double sTMin = 0.0, sTMax = 0.0;
    bool sNoWindow = false; ///< `SMATRIX имя; F;` — окно на весь счёт (многочастотный режим)
    /// Есть строка ДН (`FIELD_DISTRIBUTION` или `FIELD_DISTRIBUTION_M`).
    bool pattern = false;
    /// ДН вариантом `_M` (накопление в памяти).
    bool patternInMemory = true;
    /// Имя файлов ДН.
    QString pFile;
    /// Начало окна ДН, нс.
    double pTMin = 0.0, pTMax = 0.0;
};
/// @var RawOutput::sTMax
/// Конец окна S-матрицы, нс.
/// @var RawOutput::pTMax
/// Конец окна ДН, нс.

/// @brief Один шаг `#STEP ... #END_STEP` как он записан в файле.
struct RawStep {
    /// Номер строки `#STEP` в исходном файле.
    int line = 0;
    /// Параметры шага.
    RawParams parm;
    /// Блоки `#TOPOLOGY` в порядке записи.
    QVector<RawBlock> blocks;
    QHash<int, QPointF> links; ///< анкеры, уже в мм
    /// Номера блоков, у которых есть строка `T` в `#LINK_LIST`.
    QSet<int> linkIds;
    /// Секция `#OUTPUT`.
    RawOutput out;
};

/// Общий контекст разбора: складывать предупреждения удобнее в одно место.
struct Ctx {
    /// Куда дописывать предупреждения.
    QStringList *warnings = nullptr;
    /// Куда дописывать ошибки.
    QStringList *errors = nullptr;
    /// Папка задания: рядом с `.tpl` лежат и профили `.eps`, и файлы падающей
    /// волны `.sig`. Без неё сигнал из файла прочитать нечем.
    QString baseDir;
    /// А-13: единицы действуют на всю секцию `#PARM` (см. `readTplFile`).
    bool unitsFirst = false;
    /// Куда записывать строки с единицей ниже величины (А-13); nullptr — не нужно.
    QList<int> *unitAfterValue = nullptr;

    /// @brief Дописывает предупреждение с номером строки исходного файла.
    /// @param[in] line  номер строки
    /// @param[in] text  текст замечания (на текущем языке)
    void warn(int line, const QString &text) const
    {
        *warnings << tpl::tr("строка %1: %2", "line %1: %2").arg(line).arg(text);
    }
};

/// Вычисляет числовой параметр. Не вычислилось — предупреждаем и отдаём запасное
/// значение: файл, написанный руками, не обязан состоять из одних констант.
/// @param[in]  ctx       контекст разбора (для предупреждения)
/// @param[in]  line      номер строки
/// @param[in]  what      что вычисляется (для сообщения)
/// @param[in]  text      выражение из файла
/// @param[in]  angle     единица углов шага
/// @param[in]  fallback  значение, если выражение не вычислилось
/// @param[out] ok        признак успеха (nullptr — не нужен)
/// @return Значение выражения или `fallback`.
double evalOr(const Ctx &ctx, int line, const QString &what, const QString &text,
              AngleUnit angle, double fallback, bool *ok = nullptr)
{
    const auto v = evalExpression(text, angle);
    if (ok)
        *ok = v.has_value();
    if (v.has_value())
        return *v;
    ctx.warn(line, tpl::tr("не удалось вычислить %1: «%2» — взято %3",
                           "could not evaluate %1: '%2' - %3 taken")
                       .arg(what, text.trimmed(), QString::number(fallback)));
    return fallback;
}

/// @brief Переводит имя `LONG_UNIT` в множитель к миллиметрам.
/// @param[in]  name  `mm`, `cm` или `m` (иное — мм)
/// @param[out] unit  единица длины для модели
/// @return Множитель: 1, 10 или 1000.
double lengthUnitFactor(const QString &name, LengthUnit *unit)
{
    if (name == QLatin1String("cm")) { *unit = LengthUnit::Centimeter; return 10.0; }
    if (name == QLatin1String("m"))  { *unit = LengthUnit::Meter;      return 1000.0; }
    *unit = LengthUnit::Millimeter;
    return 1.0; // mm
}

/// @brief Переводит имя `TIME_UNIT` в множитель к наносекундам.
/// @param[in] name  `s`, `ms`, `mks`, `ns`, `ps` (иное — нс)
/// @return Множитель к нс.
double timeUnitFactorNs(const QString &name)
{
    if (name == QLatin1String("s"))   return 1.0e9;
    if (name == QLatin1String("ms"))  return 1.0e6;
    if (name == QLatin1String("mks")) return 1.0e3;
    if (name == QLatin1String("ps"))  return 1.0e-3;
    return 1.0; // ns
}

/// @brief Переводит имя `FREQ_UNIT` в множитель к гигагерцам.
/// @param[in] name  `Hz`, `kHz`, `MHz`, `GHz` (иное — ГГц)
/// @return Множитель к ГГц.
double freqUnitFactorGHz(const QString &name)
{
    if (name == QLatin1String("Hz"))  return 1.0e-9;
    if (name == QLatin1String("kHz")) return 1.0e-6;
    if (name == QLatin1String("MHz")) return 1.0e-3;
    return 1.0; // GHz
}

/// Разбирает один `BLOCK n; ... END_B`.
/// @param[in] ctx        контекст разбора (для предупреждений)
/// @param[in] id         номер блока
/// @param[in] line       номер строки `BLOCK` в исходном файле
/// @param[in] bodyLines  строки тела блока до `END_B`
/// @return Блок с видом примитива и параметрами; при неизвестном примитиве —
///         `Prim::Unknown` (блок пропускается с предупреждением).
RawBlock parseBlock(const Ctx &ctx, int id, int line, const QStringList &bodyLines)
{
    RawBlock b;
    b.id = id;
    b.line = line;
    b.raw = bodyLines.join(QLatin1Char('\n')).trimmed();

    QString text = bodyLines.join(QLatin1Char('\n'));
    // Ведущие пробелы и переводы строк до ключевого слова примитива.
    int start = 0;
    while (start < text.size() && text.at(start).isSpace())
        ++start;
    text = text.mid(start);

    for (const PrimSpec &spec : kPrims) {
        QString rest;
        if (matchKey(text, spec.word, &rest)) {
            b.prim = spec.prim;
            b.word = spec.word;
            b.suffix = QLatin1String(spec.suffix);
            b.moving = spec.moving;
            text = rest;
            break;
        }
    }

    if (b.prim == Prim::Unknown) {
        ctx.warn(line, tpl::tr("BLOCK %1: неизвестный примитив — блок пропущен: %2",
                               "BLOCK %1: unknown primitive - block skipped: %2")
                           .arg(id).arg(firstLineOf(b.raw)));
        return b;
    }

    if (b.prim != Prim::Polygon) {
        b.params = splitParams(text);
        return b;
    }

    // Полигон: материал на первой строке, дальше — по строке на вершину `L x; y;`.
    // Строчная структура здесь значима: ядро считает вершины именно по строкам
    // (`CalcEOL` в TmcRTH_BolckList.cpp), и меньше трёх не примет.
    const int semi = text.indexOf(QLatin1Char(';'));
    if (semi < 0) {
        ctx.warn(line, tpl::tr("BLOCK %1: у полигона нет материала",
                               "BLOCK %1: the polygon has no material").arg(id));
        return b;
    }
    b.params << text.left(semi).trimmed();

    const QStringList rest = text.mid(semi + 1).split(QLatin1Char('\n'));
    for (const QString &raw : rest) {
        const QString t = raw.trimmed();
        if (t.isEmpty())
            continue;
        if (t.startsWith(QLatin1Char('L')) && (t.size() == 1 || !isIdentChar(t.at(1)))) {
            const QStringList xy = splitParams(t.mid(1));
            if (xy.size() >= 2)
                b.points << qMakePair(xy.at(0), xy.at(1));
            else
                ctx.warn(line, tpl::tr("BLOCK %1: строка «%2» не похожа на вершину L x; y;",
                                       "BLOCK %1: line '%2' does not look like a vertex L x; y;")
                                   .arg(id).arg(t));
        } else {
            b.extra += splitParams(t);
        }
    }
    return b;
}

/// Разбирает `#PARM ... #END_PARM`. Отсутствующие поля наследуются от предыдущего
/// шага — так же, как это делает ядро (tpl-guide 4).
/// @param[in]     ctx        контекст разбора
/// @param[in]     lines      строки между `#PARM` и `#END_PARM`
/// @param[in]     firstLine  номер первой из них в исходном файле
/// @param[in,out] p          параметры шага (на входе — унаследованные от предыдущего)
void parseParm(const Ctx &ctx, const QStringList &lines, int firstLine, RawParams *p)
{
    // А-13 (ответ автора 29.10.2026): единица ниже своей величины в той же секции.
    // Те же пары, что проверяет ядро (CheckUnitBeforeValues): LONG_UNIT — X_MIN...Y_MAX,
    // DELTA, HEIGHT; TIME_UNIT — TIME, DELTA_T; FREQ_UNIT — FREQ. ANGLE_UNIT ядро не
    // проверяет — и мы тоже.
    QVector<int> order;
    order.reserve(lines.size());
    {
        bool seenLong = false, seenTime = false, seenFreq = false;
        QVector<int> units, others;
        for (int i = 0; i < lines.size(); ++i) {
            const QString t = lines.at(i).trimmed();
            QString rest;
            bool misplaced = false;
            bool isUnit = true;
            if (matchKey(t, QLatin1String("LONG_UNIT"), &rest))
                misplaced = seenLong;
            else if (matchKey(t, QLatin1String("TIME_UNIT"), &rest))
                misplaced = seenTime;
            else if (matchKey(t, QLatin1String("FREQ_UNIT"), &rest))
                misplaced = seenFreq;
            else if (matchKey(t, QLatin1String("ANGLE_UNIT"), &rest))
                misplaced = false;
            else
                isUnit = false;
            if (!isUnit) {
                for (const char *k : {"X_MIN", "X_MAX", "Y_MIN", "Y_MAX", "DELTA", "HEIGHT"})
                    seenLong |= matchKey(t, QLatin1String(k), &rest);
                seenTime |= matchKey(t, QLatin1String("TIME"), &rest)
                            || matchKey(t, QLatin1String("DELTA_T_MECHANICAL"), &rest);
                seenFreq |= matchKey(t, QLatin1String("FREQ"), &rest);
            }
            if (misplaced) {
                if (ctx.unitAfterValue)
                    *ctx.unitAfterValue << firstLine + i;
                if (!ctx.unitsFirst)
                    ctx.warn(firstLine + i,
                             tpl::tr("единица «%1» стоит ниже своей величины. Величина выше "
                                     "прочитана в прежней единице; ядра с 29.10.2026 такое "
                                     "задание отвергают. При сохранении единицы встанут первыми",
                                     "unit '%1' is written below its value. The value above was "
                                     "read in the previous unit; kernels since 29.10.2026 reject "
                                     "such a task. On saving the units will go first")
                                 .arg(firstLineOf(t)));
            }
            (isUnit ? units : others) << i;
        }
        if (ctx.unitsFirst)
            order << units << others;   // строки единиц — наверх, порядок прочих прежний
        else
            for (int i = 0; i < lines.size(); ++i)
                order << i;
    }

    for (int i : std::as_const(order)) {
        const QString t = lines.at(i).trimmed();
        const int ln = firstLine + i;
        if (t.isEmpty())
            continue;
        QString rest;

        if (matchKey(t, QLatin1String("ANGLE_UNIT"), &rest)) {
            p->angle = splitParams(rest).value(0) == QLatin1String("gradus") ? AngleUnit::Gradus
                                                                            : AngleUnit::Radian;
        } else if (matchKey(t, QLatin1String("LONG_UNIT"), &rest)) {
            p->longToMm = lengthUnitFactor(splitParams(rest).value(0), &p->lengthUnit);
        } else if (matchKey(t, QLatin1String("TIME_UNIT"), &rest)) {
            p->timeToNs = timeUnitFactorNs(splitParams(rest).value(0));
            p->haveTimeUnit = true;
        } else if (matchKey(t, QLatin1String("FREQ_UNIT"), &rest)) {
            p->freqToGHz = freqUnitFactorGHz(splitParams(rest).value(0));
        } else if (matchKey(t, QLatin1String("DELTA"), &rest)) {
            p->delta = evalOr(ctx, ln, QStringLiteral("DELTA"), splitParams(rest).value(0),
                              p->angle, p->delta / p->longToMm) * p->longToMm;
        } else if (matchKey(t, QLatin1String("TIME"), &rest)) {
            const QStringList v = splitParams(rest);
            p->timeMin = evalOr(ctx, ln, tpl::tr("начало TIME",
                                                 "the start of TIME"), v.value(0), p->angle, 0.0)
                         * p->timeToNs;
            p->timeMax = evalOr(ctx, ln, tpl::tr("конец TIME",
                                                 "the end of TIME"), v.value(1), p->angle, 0.0)
                         * p->timeToNs;
        } else if (matchKey(t, QLatin1String("X_MIN"), &rest)) {
            p->xMin = evalOr(ctx, ln, QStringLiteral("X_MIN"), splitParams(rest).value(0),
                             p->angle, 0.0) * p->longToMm;
        } else if (matchKey(t, QLatin1String("X_MAX"), &rest)) {
            p->xMax = evalOr(ctx, ln, QStringLiteral("X_MAX"), splitParams(rest).value(0),
                             p->angle, 0.0) * p->longToMm;
        } else if (matchKey(t, QLatin1String("Y_MIN"), &rest)) {
            p->yMin = evalOr(ctx, ln, QStringLiteral("Y_MIN"), splitParams(rest).value(0),
                             p->angle, 0.0) * p->longToMm;
        } else if (matchKey(t, QLatin1String("Y_MAX"), &rest)) {
            p->yMax = evalOr(ctx, ln, QStringLiteral("Y_MAX"), splitParams(rest).value(0),
                             p->angle, 0.0) * p->longToMm;
        } else if (matchKey(t, QLatin1String("FREQ"), &rest)) {
            p->freq = evalOr(ctx, ln, QStringLiteral("FREQ"), splitParams(rest).value(0),
                             p->angle, 0.0) * p->freqToGHz;
        } else if (matchKey(t, QLatin1String("N_UNIT"), &rest)
                   || matchKey(t, QLatin1String("B_UNIT"), &rest)) {
            // Мёртвые параметры (Баг #16): разбираются, но на расчёт не влияют.
        } else if (matchKey(t, QLatin1String("HEIGHT"), &rest)
                   || matchKey(t, QLatin1String("DELTA_T_MECHANICAL"), &rest)) {
            ctx.warn(ln, tpl::tr("параметр «%1» на расчёт не влияет и в модель "
                                 "не переносится",
                                 "parameter '%1' does not affect the computation and is not "
                                 "carried into the model").arg(firstLineOf(t)));
        } else {
            ctx.warn(ln, tpl::tr("непонятная строка в #PARM: «%1»",
                                 "unclear line in #PARM: '%1'").arg(t));
        }
        p->haveAny = true;
    }
}

/// @brief Разбирает `#OUTPUT ... #END_OUTPUT`: `FILE`, `TOPOLOGY`, `FIELDS`, `SMATRIX`,
///        `FIELD_DISTRIBUTION[_M]`; времена переводятся в нс.
/// @param[in]  ctx        контекст разбора
/// @param[in]  lines      строки секции
/// @param[in]  firstLine  номер первой из них в исходном файле
/// @param[in]  parm       параметры шага (единицы углов и времени)
/// @param[out] o          разобранная секция
void parseOutput(const Ctx &ctx, const QStringList &lines, int firstLine, const RawParams &parm,
                 RawOutput *o)
{
    for (int i = 0; i < lines.size(); ++i) {
        const QString t = lines.at(i).trimmed();
        const int ln = firstLine + i;
        if (t.isEmpty())
            continue;
        QString rest;

        if (matchKey(t, QLatin1String("FILE"), &rest)) {
            o->file = splitParams(rest).value(0);
        } else if (matchKey(t, QLatin1String("FIELD_DISTRIBUTION_M"), &rest)
                   || matchKey(t, QLatin1String("FIELD_DISTRIBUTION"), &rest)) {
            o->patternInMemory = t.startsWith(QLatin1String("FIELD_DISTRIBUTION_M"));
            const QStringList v = splitParams(rest);
            o->pattern = true;
            o->pFile = v.value(0);
            o->pTMin = evalOr(ctx, ln, tpl::tr("T_min диаграммы",
                                               "pattern T_min"), v.value(2), parm.angle,
                              0.0) * parm.timeToNs;
            o->pTMax = evalOr(ctx, ln, tpl::tr("T_max диаграммы",
                                               "pattern T_max"), v.value(3), parm.angle,
                              0.0) * parm.timeToNs;
            if (!o->patternInMemory) {
                ctx.warn(ln, tpl::tr("FIELD_DISTRIBUTION без _M дёргает диск на каждом "
                                     "такте; при записи будет использован вариант _M",
                                     "FIELD_DISTRIBUTION without _M hits the disk at every time "
                                     "step; the _M variant will be used when saving"));
            }
        } else if (matchKey(t, QLatin1String("FIELDS"), &rest)) {
            o->fields = true;
        } else if (matchKey(t, QLatin1String("TOPOLOGY"), &rest)) {
            o->topology = true;
        } else if (matchKey(t, QLatin1String("SMATRIX"), &rest)) {
            const QStringList v = splitParams(rest);
            o->sMatrix = true;
            o->sFile = v.value(0);
            if (v.size() < 4) {
                // Без окна строка законна только в многочастотном режиме (вход с файлом
                // сигнала): это окно на весь счёт. В одночастотном ядро с 26.10.2026
                // отвергает шаг — об этом скажет сборка модели, она видит входы.
                o->sNoWindow = true;
                continue;
            }
            o->sTMin = evalOr(ctx, ln, tpl::tr("T_min S-матрицы",
                                               "S-matrix T_min"), v.value(2), parm.angle,
                              0.0) * parm.timeToNs;
            o->sTMax = evalOr(ctx, ln, tpl::tr("T_max S-матрицы",
                                               "S-matrix T_max"), v.value(3), parm.angle,
                              0.0) * parm.timeToNs;
        } else {
            ctx.warn(ln, tpl::tr("непонятная строка в #OUTPUT: «%1»",
                                 "unclear line in #OUTPUT: '%1'").arg(t));
        }
    }
}

/// Режет плоский текст на шаги. Собственно физика — уже следующим этапом.
/// @param[in]  ctx        контекст разбора
/// @param[in]  flat       текст после препроцессора
/// @param[out] sawHeader  встречен ли `#TMC_RT_H`
/// @param[out] sawEof     встречен ли `#EOF`
/// @return Шаги в порядке файла; `#PARM` каждого наследует параметры предыдущего.
QVector<RawStep> parseSteps(const Ctx &ctx, const QString &flat, bool *sawHeader, bool *sawEof)
{
    const QStringList lines = flat.split(QLatin1Char('\n'));
    QVector<RawStep> steps;
    RawParams carried; ///< параметры «как в предыдущем шаге»

    *sawHeader = false;
    *sawEof = false;

    int i = 0;
    while (i < lines.size()) {
        const QString t = lines.at(i).trimmed();
        if (t.startsWith(QLatin1String("#TMC_RT_H"))) {
            *sawHeader = true;
            ++i;
            continue;
        }
        if (t.startsWith(QLatin1String("#EOF"))) {
            *sawEof = true;
            break;
        }
        if (!t.startsWith(QLatin1String("#STEP"))) {
            if (!t.isEmpty() && t.startsWith(QLatin1Char('#')))
                ctx.warn(i + 1, tpl::tr("секция вне шага пропущена: «%1»",
                                        "section outside a step skipped: '%1'").arg(t));
            ++i;
            continue;
        }

        RawStep step;
        step.line = i + 1;
        step.parm = carried;
        ++i;

        while (i < lines.size()) {
            const QString s = lines.at(i).trimmed();
            if (s.startsWith(QLatin1String("#END_STEP"))) {
                ++i;
                break;
            }
            if (s.startsWith(QLatin1String("#EOF"))) {
                *sawEof = true;
                break;
            }

            // --- #PARM ---
            if (s.startsWith(QLatin1String("#PARM"))) {
                const int first = i + 2;
                QStringList body;
                ++i;
                while (i < lines.size() && !lines.at(i).trimmed().startsWith(QLatin1String("#END_PARM"))
                       && !lines.at(i).trimmed().startsWith(QLatin1String("#END_STEP"))) {
                    body << lines.at(i);
                    ++i;
                }
                if (i < lines.size() && lines.at(i).trimmed().startsWith(QLatin1String("#END_PARM")))
                    ++i;
                parseParm(ctx, body, first, &step.parm);
                continue;
            }

            // --- #TOPOLOGY ---
            if (s.startsWith(QLatin1String("#TOPOLOGY"))) {
                ++i;
                while (i < lines.size()) {
                    const QString b = lines.at(i).trimmed();
                    if (b.startsWith(QLatin1String("#END_TOPOLOGY")) || b.startsWith(QLatin1Char('#')))
                        break;
                    QString rest;
                    if (matchKey(b, QLatin1String("BLOCK"), &rest)) {
                        const int blockLine = i + 1;
                        const int semi = rest.indexOf(QLatin1Char(';'));
                        const int id = (semi < 0 ? rest : rest.left(semi)).trimmed().toInt();
                        QStringList body;
                        if (semi >= 0)
                            body << rest.mid(semi + 1);
                        ++i;
                        while (i < lines.size()
                               && !lines.at(i).trimmed().startsWith(QLatin1String("END_B"))
                               && !lines.at(i).trimmed().startsWith(QLatin1Char('#'))) {
                            body << lines.at(i);
                            ++i;
                        }
                        if (i < lines.size() && lines.at(i).trimmed().startsWith(QLatin1String("END_B")))
                            ++i;
                        if (id < 1) {
                            ctx.warn(blockLine, tpl::tr("номер блока должен быть >= 1 — "
                                                        "блок пропущен",
                                                        "the block number must be >= 1 - block "
                                                        "skipped"));
                            continue;
                        }
                        step.blocks << parseBlock(ctx, id, blockLine, body);
                        continue;
                    }
                    if (!b.isEmpty())
                        ctx.warn(i + 1, tpl::tr("строка в #TOPOLOGY вне блока: «%1»",
                                                "line in #TOPOLOGY outside a block: '%1'").arg(b));
                    ++i;
                }
                if (i < lines.size() && lines.at(i).trimmed().startsWith(QLatin1String("#END_TOPOLOGY")))
                    ++i;
                continue;
            }

            // --- #LINK_LIST ---
            if (s.startsWith(QLatin1String("#LINK_LIST"))) {
                ++i;
                while (i < lines.size()) {
                    const QString b = lines.at(i).trimmed();
                    if (b.startsWith(QLatin1Char('#')))
                        break;
                    QString rest;
                    if (matchKey(b, QLatin1String("T"), &rest)) {
                        QStringList v = splitParams(rest);
                        v.removeAll(QString()); // терпим лишний ';' — он встречается в эталонах
                        if (v.size() >= 3) {
                            const int id = v.at(0).toInt();
                            const double x = evalOr(ctx, i + 1, tpl::tr("X0 анкера", "anchor X0"),
                                                    v.at(1), step.parm.angle, 0.0)
                                             * step.parm.longToMm;
                            const double y = evalOr(ctx, i + 1, tpl::tr("Y0 анкера", "anchor Y0"),
                                                    v.at(2), step.parm.angle, 0.0)
                                             * step.parm.longToMm;
                            step.links.insert(id, QPointF(x, y));
                            step.linkIds.insert(id);
                        } else if (!b.isEmpty()) {
                            ctx.warn(i + 1, tpl::tr("строка #LINK_LIST не разобрана: «%1»",
                                                    "#LINK_LIST line not parsed: '%1'")
                                                .arg(b));
                        }
                    } else if (!b.isEmpty()) {
                        ctx.warn(i + 1, tpl::tr("строка в #LINK_LIST не похожа на T n; x; y;: "
                                                "«%1»",
                                                "line in #LINK_LIST does not look like T n; x; "
                                                "y;: '%1'").arg(b));
                    }
                    ++i;
                }
                if (i < lines.size() && lines.at(i).trimmed().startsWith(QLatin1String("#END_LINK")))
                    ++i;
                continue;
            }

            // --- #OUTPUT ---
            if (s.startsWith(QLatin1String("#OUTPUT"))) {
                const int first = i + 2;
                QStringList body;
                ++i;
                while (i < lines.size() && !lines.at(i).trimmed().startsWith(QLatin1Char('#'))) {
                    body << lines.at(i);
                    ++i;
                }
                if (i < lines.size() && lines.at(i).trimmed().startsWith(QLatin1String("#END_OUTPUT")))
                    ++i;
                parseOutput(ctx, body, first, step.parm, &step.out);
                continue;
            }

            if (!s.isEmpty() && s.startsWith(QLatin1Char('#')))
                ctx.warn(i + 1, tpl::tr("неизвестная секция «%1» пропущена",
                                        "unknown section '%1' skipped").arg(s));
            ++i;
        }

        carried = step.parm;
        steps << step;
    }

    return steps;
}

// ===========================================================================
//  Плоские шаги -> модель
// ===========================================================================

/// Плазменная добавка блока: суффикс примитива -> поле модели.
///
/// ⚠️ Вся связь «суффикс языка — поле модели» живёт ТОЛЬКО здесь. В языке N, B и Y
/// это три разных примитива с одинаковой геометрией, в модели — одна фигура с
/// тремя величинами; склейкой занимается mergePlasmaInto().
/// Непустая `formula` означает, что величина задана выражением от координат; тогда
/// `value` не используется, а в файл при сохранении уйдёт сама формула.
/// @param[in,out] s        фигура
/// @param[in]     suffix   суффикс примитива: "_N", "_B", "_Y" или "_S" (иное — без изменений)
/// @param[in]     value    числовое значение (м^-3, Тл, безразмерное Y, См/м)
/// @param[in]     formula  формула величины; пусто — число
void applyPlasma(Shape &s, QStringView suffix, double value, const QString &formula = QString())
{
    if (suffix == QLatin1String("_N")) {
        s.plasma.hasN = true;
        s.plasma.n = value;
        s.plasma.nFormula = formula;
    } else if (suffix == QLatin1String("_B")) {
        s.plasma.hasB = true;
        s.plasma.b = value;
        s.plasma.bFormula = formula;
    } else if (suffix == QLatin1String("_Y")) {
        s.plasma.hasY = true;
        s.plasma.y = value;
        s.plasma.yFormula = formula;
    } else if (suffix == QLatin1String("_S")) {
        s.plasma.hasS = true;
        s.plasma.s = value;
        s.plasma.sFormula = formula;
    }
}

/// @brief Сравнивает две вершины с допуском 1e-6 мм.
/// @param[in] a  первая точка, мм
/// @param[in] b  вторая точка, мм
/// @return true — точки совпадают в пределах допуска.
bool samePoint(const QPointF &a, const QPointF &b)
{
    // Допуск — одна миллионная миллиметра, а не машинный ноль. Координаты приходят
    // из ТЕКСТА, где число уже округлено при записи, и точная середина отрезка
    // 1.0…10.16384259 читается обратно с расхождением в последнем знаке. Строгое
    // сравнение из-за этого не узнавало собственную добивку отрезка.
    //
    // Ниже нанометра геометрии в задании не бывает: сетка DELTA идёт долями миллиметра,
    // так что слить две действительно разные вершины этот допуск не может.
    constexpr double kEps = 1e-6;
    return std::fabs(a.x() - b.x()) < kEps && std::fabs(a.y() - b.y()) < kEps;
}

/// Убирает вершины, которых в геометрии нет, а в файле они есть только потому,
/// что ядро отвергает `POLYGON_STAT` меньше чем с ТРЕМЯ строками `L`
/// (`if( n1 < 3 )` в TmcRTH_BolckList.cpp стоит до разбора материала).
///
/// Отрезок добивают до трёх точек двумя способами:
///  * автор в старых заданиях ДУБЛИРУЕТ последнюю точку (RT_HPLANAR);
///  * наш эмиттер вставляет СЕРЕДИНУ отрезка.
/// И то и другое при чтении надо схлопнуть: иначе прогон emit -> read вернёт
/// ломаную из трёх точек там, где пользователь рисовал две.
/// Убирает вершины, которые в задании стоят ТОЛЬКО ради требования ядра.
///
/// `POLYGON_STAT` не грузится меньше чем с тремя строками `L` (проверка `n1 < 3` в
/// `TmcRTH_BolckList.cpp` стоит до разбора материала), поэтому отрезок из двух точек
/// приходится добивать до трёх. Способов два, и оба встречаются:
///   * повторить вершину — так написаны авторские задания (`samples/DEMO/.../horn.tpl`,
///     блоки 3 и 4: последняя точка продублирована);
///   * поставить точку посередине — так пишет наш эмиттер.
/// Геометрия у обоих одна и та же: звено нулевой длины и точка на той же прямой
/// поля не меняют.
///
/// ⚠️ Молчим намеренно (решение 2026-09-06). Раньше на каждую такую вершину шло
/// предупреждение, и открытие авторского `horn.tpl` встречало человека окном
/// «часть задания модель выражает не так, как в файле». Формально правда — при
/// сохранении повтор вершины сменится точкой посередине, — но ТЕРЯТЬ тут нечего:
/// это одна и та же обходка одного и того же ограничения ядра, записанная двумя
/// способами. Предупреждать надо о потерях, а не о смене записи.
/// @param[in]     ctx   контекст разбора (не используется)
/// @param[in]     line  номер строки (не используется)
/// @param[in]     id    номер блока (не используется)
/// @param[in,out] pts   вершины полигона, мм
void dropPaddingPoints(const Ctx &ctx, int line, int id, QVector<QPointF> *pts)
{
    Q_UNUSED(ctx);
    Q_UNUSED(line);
    Q_UNUSED(id);

    // Подряд идущие совпадающие вершины — всегда лишние.
    for (int i = pts->size() - 1; i > 0; --i) {
        if (samePoint(pts->at(i), pts->at(i - 1)))
            pts->removeAt(i);
    }

    // Ровно три точки, средняя точно посередине — это наша «добивка» отрезка.
    if (pts->size() == 3) {
        const QPointF mid = (pts->at(0) + pts->at(2)) / 2.0;
        if (samePoint(pts->at(1), mid))
            pts->removeAt(1);
    }
}

/// Первый параметр примитива: материал или выражение добавки eps.
/// ⚠️ В файле лежит ДОБАВКА eps-1, в модели — сама eps.
///
/// `longToMm` нужен формулам: ядро подставляет в них x и y в единицах LONG_UNIT
/// того файла, где формула написана, а модель редактора и выдаваемый им файл всегда
/// в миллиметрах. Формулу из файла в сантиметрах надо пересчитать, иначе профиль
/// растянется вдесятеро.
/// @param[in]     ctx       контекст разбора
/// @param[in]     line      номер строки блока
/// @param[in]     first     первый параметр блока (материал, число, `[имя]выражение`)
/// @param[in]     suffix    плазменный суффикс примитива ("" — обычный блок)
/// @param[in]     angle     единица углов шага
/// @param[in]     longToMm  множитель `LONG_UNIT` -> мм (для пересчёта формул)
/// @param[in,out] s         фигура: материал, eps, профиль или плазменная величина
void applyMaterial(const Ctx &ctx, int line, const QString &first, const QString &suffix,
                   AngleUnit angle, double longToMm, Shape *s)
{
    QString expr = first;
    const double toModel = (longToMm != 0.0) ? 1.0 / longToMm : 1.0;

    // RECT_STAT [name]<выражение> — ядру велено сохранить посчитанный профиль в файл.
    // Имя запоминаем: это часть задания, и потерять его значило бы при сохранении
    // отобрать у пользователя файл, на который он, возможно, уже ссылается.
    QString profileFile;
    if (expr.startsWith(QLatin1Char('['))) {
        const int close = expr.indexOf(QLatin1Char(']'));
        if (close > 0) {
            profileFile = expr.mid(1, close - 1).trimmed();
            expr = expr.mid(close + 1);
        }
    }

    if (!suffix.isEmpty()) {
        // Плазменный параметр бывает и профилем: ядро считает первый параметр блока
        // тем же вычислителем, что и добавку eps, поэтому `RECT_STAT_N <выражение
        // от x и y>` — законный блок. Раньше такое выражение не вычислялось (оно и
        // не может быть вычислено: у него нет ОДНОГО значения), фигура получала ноль,
        // и сохранение поверх стирало профиль плотности из задания.
        const auto v = evalExpression(expr, angle);
        if (v.has_value() && profileFile.isEmpty()) {
            applyPlasma(*s, suffix, *v);
        } else {
            // Имя файла возвращаем в текст: `[имя]` — часть первого параметра, и
            // потерять его значило бы отобрать у пользователя файл профиля.
            const QString body = scaleCoordinates(expr.trimmed(), toModel);
            const QString kept = profileFile.isEmpty()
                                     ? body
                                     : QStringLiteral("[%1]%2").arg(profileFile, body);
            applyPlasma(*s, suffix, 0.0, kept);
        }
        return;
    }

    if (expr == QLatin1String("METAL"))         { s->material = Material::Metal;    return; }
    if (expr == QLatin1String("MAGNETIC"))      { s->material = Material::Magnetic; return; }
    if (expr == QLatin1String("ABSORBER"))      { s->material = Material::Absorber; return; }

    s->material = Material::Dielectric;
    const auto add = evalExpression(expr, angle);
    if (add.has_value()) {
        s->eps = *add + 1.0; // в файле лежит ДОБАВКА eps-1, в модели — сама eps
        if (!profileFile.isEmpty()) {
            // Постоянное выражение с именем файла: профиль вырожденный, но имя всё
            // равно осмысленно — ядро запишет по нему файл.
            s->epsProfile.enabled = true;
            s->epsProfile.file = profileFile;
            s->epsProfile.formula = QString::number(s->eps, 'g', 12);
        }
        return;
    }

    // Выражение зависит от координат — это и есть неоднородная eps.
    //
    // В файле лежит ДОБАВКА, в модели — сама eps, поэтому единицу надо вернуть.
    // Наш собственный вид `(X)-1` разбираем обратно в `X`: иначе круговорот
    // «открыл -> сохранил» обрастал бы `((X)-1)+1` на каждом заходе.
    QString formula = expr.trimmed();
    if (formula.startsWith(QLatin1Char('(')) && formula.endsWith(QLatin1String(")-1"))) {
        const QString inner = formula.mid(1, formula.size() - 4);
        // Проверяем, что снятая скобка была парной, а не двумя разными: у `(a)+(b)-1`
        // такое «снятие» изуродовало бы выражение.
        int depth = 0;
        bool balanced = true;
        for (QChar c : inner) {
            if (c == QLatin1Char('('))
                ++depth;
            else if (c == QLatin1Char(')') && --depth < 0)
                balanced = false;
        }
        if (balanced && depth == 0)
            formula = inner;
        else
            formula = QStringLiteral("(%1)+1").arg(formula);
    } else {
        formula = QStringLiteral("(%1)+1").arg(formula);
    }

    s->epsProfile.enabled = true;
    // Координаты в формуле — в единицах LONG_UNIT исходного файла, а модель и
    // выдаваемый файл всегда в миллиметрах.
    s->epsProfile.formula = scaleCoordinates(formula, toModel);
    s->epsProfile.file = profileFile;
    s->eps = 1.0; // числом такую среду не описать; поле остаётся при выключенном профиле

    // Форма здесь ни при чём: выражение ядро считает и в круге, и в полигоне, и
    // записываем мы его обратно той же фигурой. Прямоугольник нужен лишь для `[имя]`,
    // и об этом скажет валидатор — ровно тому, кто такое имя задал.
}

/// @brief Форматирует число для ключей сравнения (12 значащих цифр).
/// @param[in] v  число
/// @return Текст числа.
QString num(double v)
{
    return QString::number(v, 'g', 12);
}

/// Ключ ГЕОМЕТРИИ — без материала и плазмы. Нужен, чтобы узнать три плазменных
/// блока одной и той же фигуры.
/// @param[in] s  фигура
/// @return Строка-ключ: вид фигуры, скорость и координаты (12 значащих цифр).
QString shapeGeometryKey(const Shape &s)
{
    QStringList p;
    p << QString::number(int(s.kind)) << num(s.vx) << num(s.vy);
    switch (s.kind) {
    case ShapeKind::Rectangle:
        p << num(s.xMin) << num(s.xMax) << num(s.yMin) << num(s.yMax);
        break;
    case ShapeKind::Circle:
        p << num(s.center.x()) << num(s.center.y()) << num(s.rMin) << num(s.rMax)
          << num(s.fiMin) << num(s.fiMax);
        break;
    case ShapeKind::Polygon:
    case ShapeKind::Polyline:
        for (const QPointF &pt : s.points)
            p << num(pt.x()) << num(pt.y());
        break;
    }
    return p.join(QLatin1Char('|'));
}

/// @brief Строит полный ключ фигуры: геометрия, материал, eps, профиль и плазма.
///
/// По нему одна и та же фигура узнаётся в разных шагах (частоты, варианты).
/// @param[in] s  фигура
/// @return Строка-ключ.
QString shapeKey(const Shape &s)
{
    // Профиль входит в ключ наравне с материалом: две одинаковые по форме области с
    // РАЗНЫМИ формулами — это две разные среды, и слить их в одну фигуру значило бы
    // потерять одну из формул.
    //
    // Имя файла — в ключе только у профиля ИЗ файла (там это сама среда). У профиля
    // формулой имя файла — пометка «сохранить посчитанное», и эмиттер ставит её лишь
    // на первом шаге: с ней в ключе одна фигура свипа рассыпалась бы на две.
    return QStringList{shapeGeometryKey(s), QString::number(int(s.material)), num(s.eps),
                       s.epsProfile.formula,
                       s.epsProfile.fromFile ? s.epsProfile.file : QString(),
                       s.epsProfile.customOrigin ? num(s.epsProfile.origin.x()) : QString(),
                       s.epsProfile.customOrigin ? num(s.epsProfile.origin.y()) : QString(),
                       QString::number(int(s.plasma.hasN)), num(s.plasma.n), s.plasma.nFile,
                       QString::number(int(s.plasma.hasB)), num(s.plasma.b), s.plasma.bFile,
                       QString::number(int(s.plasma.hasY)), num(s.plasma.y), s.plasma.yFile,
                       QString::number(int(s.plasma.hasS)), num(s.plasma.s)}
        .join(QLatin1Char('|'));
}

/// Собирает `_N`, `_B`, `_Y` и `_S` одной области обратно в ОДНУ фигуру модели.
///
/// ⚠️ Эмиттер пишет B прямоугольником ШИРЕ плазмы на `kBMarginCells` шага с каждой
/// стороны (на границе блока ядро ополовинивает значение, Б-21), а старые версии —
/// ровно по габариту фигуры. Поэтому «той же фигурой» считается и прямоугольник B,
/// который накрывает габарит и выходит за него не дальше этого запаса. `delta` — шаг
/// сетки шага, из которого блок прочитан.
/// @param[in,out] shapes  фигуры шага; к подходящей дописывается величина
/// @param[in]     add     фигура с одной плазменной величиной, прочитанная из блока
/// @param[in]     delta   шаг сетки этого шага, мм
/// @return true — величина приклеена к существующей фигуре; false — подходящей нет
///         (вызывающий добавляет `add` отдельной фигурой).
bool mergePlasmaInto(QVector<Shape> *shapes, const Shape &add, double delta)
{
    const double margin = kBMarginCells * delta + 1e-6;
    for (int i = shapes->size() - 1; i >= 0; --i) {
        Shape &t = (*shapes)[i];
        if (!t.plasma.any())
            continue;

        bool same = (shapeGeometryKey(t) == shapeGeometryKey(add));
        // Блок B ровно по плазме — не наш широкий: держим его как есть (Plasma::bExact).
        bool exactB = same;
        if (!same && add.kind == ShapeKind::Rectangle && !add.plasma.hasN) {
            double xLo = 0, xHi = 0, yLo = 0, yHi = 0;
            t.bounds(&xLo, &xHi, &yLo, &yHi);
            const double dl = xLo - add.xMin, dr = add.xMax - xHi;
            const double db = yLo - add.yMin, dt = add.yMax - yHi;
            const bool wider = add.plasma.hasB && add.plasma.bFile.isEmpty();
            const double lim = wider ? margin : 1e-9;
            same = dl > -1e-9 && dr > -1e-9 && db > -1e-9 && dt > -1e-9 && dl < lim
                   && dr < lim && db < lim && dt < lim;
            exactB = same && dl < 1e-9 && dr < 1e-9 && db < 1e-9 && dt < 1e-9;
        }
        if (!same)
            continue;

        if ((add.plasma.hasN && t.plasma.hasN) || (add.plasma.hasB && t.plasma.hasB)
            || (add.plasma.hasY && t.plasma.hasY) || (add.plasma.hasS && t.plasma.hasS))
            continue; // величина уже занята — это другая фигура, а не та же

        if (add.plasma.hasN) {
            t.plasma.hasN = true;
            t.plasma.n = add.plasma.n;
            t.plasma.nFile = add.plasma.nFile;
            t.plasma.nFormula = add.plasma.nFormula;
        }
        if (add.plasma.hasB) {
            t.plasma.hasB = true;
            t.plasma.b = add.plasma.b;
            t.plasma.bFile = add.plasma.bFile;
            t.plasma.bFormula = add.plasma.bFormula;
            t.plasma.bExact = exactB && add.plasma.bFile.isEmpty();
        }
        if (add.plasma.hasY) {
            t.plasma.hasY = true;
            t.plasma.y = add.plasma.y;
            t.plasma.yFile = add.plasma.yFile;
            t.plasma.yFormula = add.plasma.yFormula;
        }
        if (add.plasma.hasS) {
            t.plasma.hasS = true;
            t.plasma.s = add.plasma.s;
            t.plasma.sFormula = add.plasma.sFormula;
        }
        // Анкер формулы приходит вместе с той величиной, которая формулой и задана.
        if (add.plasma.customOrigin && !t.plasma.customOrigin) {
            t.plasma.customOrigin = true;
            t.plasma.origin = add.plasma.origin;
        }
        return true;
    }
    return false;
}

/// @brief Строит ключ порта: сторона, апертура, плоскость и мода.
///
/// По нему один и тот же порт узнаётся в разных шагах.
/// @param[in] p  порт
/// @return Строка-ключ.
QString portKey(const Port &p)
{
    // Сторона, а не только ось: у волновода оба входа — INPUT_X с одной и той же
    // апертурой, и различает их ровно то, что один левый, а другой правый. По оси
    // они склеивались в один порт, и половина задания пропадала при чтении.
    return QStringList{QString::number(int(p.side)), num(p.from), num(p.to),
                       num(p.plane.value_or(std::numeric_limits<double>::quiet_NaN())),
                       QString::number(int(p.mode))}
        .join(QLatin1Char('|'));
}

/// Падающая волна из файла: на месте Amp стоит `[имя]`.
///
/// Форму восстанавливаем по служебной строке `@TPLD-SIG` в шапке самого файла. Без
/// неё (файл написан руками или другой программой) форма неизвестна, и честный ответ
/// один — взять точки как есть, таблицей: так ничего не потеряется и ничего не будет
/// выдумано.
///
/// ⚠️ Имя файла ищем БЕЗ учёта регистра, как и `#include`: задания пишут на Windows,
/// где регистр не значим, а читать их могут на Linux и macOS, где значим.
/// @param[in]     ctx      контекст разбора (папка задания, предупреждения)
/// @param[in]     line     номер строки входа
/// @param[in]     ampExpr  параметр Amp из файла
/// @param[in,out] port     порт: источник, имя файла и форма волны
/// @return true — Amp задан `[имя]` и файл прочитан; false — Amp не файл, либо файл
///         не найден или имя испорчено (с предупреждением).
bool restoreFileSignal(const Ctx &ctx, int line, const QString &ampExpr, Port *port)
{
    const QString s = ampExpr.trimmed();
    if (!s.startsWith(QLatin1Char('[')))
        return false;
    const int close = s.indexOf(QLatin1Char(']'));
    if (close < 0) {
        // Ядро на этом тоже останавливается: «missing ] or bad [ in signal expression».
        ctx.warn(line, tpl::tr("в имени файла сигнала «%1» нет закрывающей скобки — "
                               "ядро такой файл не примет; вход оставлен без сигнала",
                               "the signal file name '%1' has no closing bracket - the kernel "
                               "will not accept such a file; the port is left without a signal")
                           .arg(s));
        return false;
    }
    const QString name = s.mid(1, close - 1).trimmed();
    if (name.isEmpty()) {
        ctx.warn(line, tpl::tr("имя файла сигнала пусто", "the signal file name is empty"));
        return false;
    }

    // Расширение ядро дописывает само, но только если точки в имени нет.
    const QString fileName = name.contains(QLatin1Char('.')) ? name
                                                            : name + QStringLiteral(".sig");
    const QString path = ctx.baseDir.isEmpty()
                             ? QString()
                             : findFileIgnoringCase(ctx.baseDir, fileName);
    QString text;
    if (path.isEmpty() || !readTplFileText(path, &text)) {
        // Потеря, а не смена записи: предупреждаем прямо. Порт переводим на
        // непрерывный сигнал, иначе задание нельзя было бы сохранить вовсе.
        ctx.warn(line, tpl::tr("файл падающей волны «%1» рядом с заданием не найден — "
                               "форма сигнала потеряна, вход переведён на непрерывный "
                               "сигнал. Положите файл рядом и откройте задание заново",
                               "incident-wave file '%1' was not found next to the task - the "
                               "signal waveform is lost, the port is switched to a continuous "
                               "signal. Put the file next to the task and open the task again")
                           .arg(fileName));
        return false;
    }

    port->signal.source = SignalSource::File;
    // Имя храним БЕЗ расширения — таким оно и уйдёт обратно в квадратные скобки.
    port->signal.file = name.contains(QLatin1Char('.'))
                            ? name.left(name.lastIndexOf(QLatin1Char('.')))
                            : name;

    Waveshape w;
    bool restored = false;
    for (const QString &l : text.split(QLatin1Char('\n'))) {
        if (parseSigMarker(l, &w)) {
            restored = true;
            break;
        }
    }
    if (restored) {
        port->signal.wave = w;
        // У таблицы сами точки живут только в файле — их надо взять оттуда.
        if (w.kind == Waveform::Table) {
            port->signal.wave.points = parseSigPoints(text);
            port->signal.wave.tableTones = parseSigTones(text);
        }
        return true;
    }

    Waveshape t;
    t.kind = Waveform::Table;
    t.points = parseSigPoints(text);
    t.tableTones = parseSigTones(text);
    port->signal.wave = t;
    if (t.tableTones.size() > kMaxFreqs) {
        ctx.warn(line, tpl::tr("в файле «%1» строка #FREQ называет %2, а пакет берёт "
                               "не больше %3: обработаются первые %3, остальные %4 "
                               "будут отброшены",
                               "in file '%1' the #FREQ line names %2, but the package takes at "
                               "most %3: the first %3 will be processed, the remaining %4 will be "
                               "discarded")
                           .arg(fileName,
                                plural(int(t.tableTones.size()), tpl::tr("тон", "tone"),
                                       tpl::tr("тона", "tones"), tpl::tr("тонов", "tones")))
                           .arg(kMaxFreqs)
                           .arg(t.tableTones.size() - kMaxFreqs));
    }
    if (t.points.size() < 2) {
        ctx.warn(line, tpl::tr("в файле «%1» меньше двух точек — ядро его не примет",
                               "file '%1' has fewer than two points - the kernel will not accept "
                               "it")
                           .arg(fileName));
    } else {
        ctx.warn(line, tpl::tr("файл «%1» сделан не этой программой: форма взята "
                               "таблицей из %2 точек — так ничего не потеряется",
                               "file '%1' was not made by this program: the waveform is taken as "
                               "a table of %2 points - this way nothing is lost")
                           .arg(fileName).arg(t.points.size()));
    }
    return true;
}

/// Восстанавливает форму сигнала из выражения Amp и временного окна.
///
/// Огибающая живёт в Amp — окно ядро режет резко, поэтому «гауссов импульс»
/// узнаётся именно по виду выражения, а не по узкому окну.
/// @param[in]     ctx       контекст разбора
/// @param[in]     line      номер строки входа
/// @param[in]     ampExpr   параметр Amp
/// @param[in]     fazaExpr  параметр Faza (пусто — 0)
/// @param[in]     tMin      начало окна, нс
/// @param[in]     tMax      конец окна, нс
/// @param[in]     parm      параметры шага (единица углов, время счёта)
/// @param[in,out] port      порт: фаза (рад), форма, амплитуда, центр и длительность (нс)
void restoreSignal(const Ctx &ctx, int line, const QString &ampExpr, const QString &fazaExpr,
                   double tMin, double tMax, const RawParams &parm, Port *port)
{
    static const QRegularExpression gauss(
        QStringLiteral("^\\s*(.+?)\\*exp\\(\\s*-\\s*\\(\\(t-(.+?)\\)/(.+?)\\)\\*"
                       "\\(\\(t-(.+?)\\)/(.+?)\\)\\s*\\)\\s*$"));

    port->signal.phase = fazaExpr.trimmed().isEmpty()
                             ? 0.0
                             : evalOr(ctx, line, tpl::tr("фазу входа", "the port phase"), fazaExpr,
                                      parm.angle, 0.0);
    if (parm.angle == AngleUnit::Gradus)
        port->signal.phase *= 3.14159265358979323846 / 180.0; // модель хранит радианы

    // Сигнал из файла проверяем ПЕРВЫМ: `[имя]` не выражение, и вычислять его нельзя —
    // ядро на разборе такого Amp падало с `abnormal function`, пока в него не добавили
    // ту же проверку по первому символу.
    if (restoreFileSignal(ctx, line, ampExpr, port))
        return;
    if (ampExpr.trimmed().startsWith(QLatin1Char('['))) {
        // Файл не нашёлся, и об этом уже сказано выше. Пытаться после этого ВЫЧИСЛИТЬ
        // «[имя]» как выражение значит выдать второе предупреждение о том же самом —
        // человек получал подряд «форма потеряна» и «амплитуду вычислить не удалось».
        port->signal.shape = SignalShape::Continuous;
        port->signal.amplitude = 1.0;
        return;
    }

    const QRegularExpressionMatch m = gauss.match(ampExpr.trimmed());
    if (m.hasMatch()) {
        port->signal.shape = SignalShape::Gaussian;
        port->signal.amplitude = evalOr(ctx, line, tpl::tr("амплитуду",
                                                           "the amplitude"), m.captured(1),
                                        parm.angle, 1.0);
        port->signal.center = evalOr(ctx, line, tpl::tr("центр импульса",
                                                        "the pulse centre"), m.captured(2),
                                     parm.angle, 0.0) * parm.timeToNs;
        port->signal.duration = evalOr(ctx, line, tpl::tr("длительность импульса",
                                                          "the pulse duration"),
                                       m.captured(3), parm.angle, 1.0) * parm.timeToNs;
        return;
    }

    bool ok = false;
    const double amp = ampExpr.trimmed().isEmpty()
                           ? 1.0
                           : evalOr(ctx, line, tpl::tr("амплитуду входа",
                                                       "the port amplitude"), ampExpr,
                                    parm.angle, 1.0, &ok);
    if (!ampExpr.trimmed().isEmpty() && !ok) {
        ctx.warn(line, tpl::tr("выражение амплитуды «%1» модель не выражает — взята "
                               "постоянная амплитуда 1",
                               "the model cannot express the amplitude expression '%1' - a "
                               "constant amplitude 1 is taken").arg(ampExpr.trimmed()));
        port->signal.shape = SignalShape::Continuous;
        port->signal.amplitude = 1.0;
        return;
    }

    // Окно, накрывающее весь счёт, — это несущая на всё время; иначе резкий импульс.
    if (tMin <= parm.timeMin + 1e-9 && tMax >= parm.timeMax - 1e-9) {
        port->signal.shape = SignalShape::Continuous;
        port->signal.amplitude = amp;
    } else {
        port->signal.shape = SignalShape::Rectangular;
        port->signal.amplitude = amp;
        port->signal.center = (tMin + tMax) / 2.0;
        port->signal.duration = tMax - tMin;
    }
}

/// Модель одного `#STEP` — промежуточное представление перед склейкой.
struct StepModel {
    /// Частота шага, ГГц.
    double freq = 0.0;
    /// Фигуры шага (плазменные блоки уже склеены).
    QVector<Shape> shapes;
    /// Входы шага.
    QVector<Port> ports;
    /// Секция `#OUTPUT` шага.
    RawOutput out;
    /// В шаге есть плазменные блоки.
    bool anyPlasma = false;
};

/// Имя файла распределения так, как его держит модель: без расширения.
///
/// Ядро дописывает `.eps` само (`AddFileEpsExtention`), поэтому в задании имя обычно
/// написано голым. Но файл могли написать руками и с расширением — тогда, сохранив имя
/// как есть, мы получили бы при следующей записи `имя.eps.eps`.
/// @param[in] name  имя из задания
/// @return Имя без хвоста `.eps` (без учёта регистра).
QString stripEpsExtension(const QString &name)
{
    if (name.endsWith(QLatin1String(".eps"), Qt::CaseInsensitive))
        return name.left(name.size() - 4);
    return name;
}

/// @brief Собирает модель одного шага из сырых блоков: фигуры, плазма, профили, порты.
///
/// Переводит локальные координаты блока и анкер `#LINK_LIST` в координаты сцены,
/// добавку eps-1 — в саму eps, склеивает плазменные блоки одной области и разбирает
/// входы с восемью параметрами. Всё, что модель не выражает, — предупреждение.
/// @param[in] ctx   контекст разбора
/// @param[in] step  сырой шаг
/// @return Модель шага.
StepModel buildStep(const Ctx &ctx, const RawStep &step)
{
    StepModel sm;
    sm.freq = step.parm.freq;
    sm.out = step.out;

    const Box box{step.parm.xMin, step.parm.xMax, step.parm.yMin, step.parm.yMax};
    const AngleUnit angle = step.parm.angle;
    const double k = step.parm.longToMm;

    for (const RawBlock &b : step.blocks) {
        if (b.prim == Prim::Unknown)
            continue;

        QPointF anchor(0.0, 0.0);
        if (step.links.contains(b.id)) {
            anchor = step.links.value(b.id);
        } else {
            ctx.warn(b.line, tpl::tr("BLOCK %1 не упомянут в #LINK_LIST — ядро такой файл "
                                     "не примет; принято положение (0;0)",
                                     "BLOCK %1 is not mentioned in #LINK_LIST - the kernel will "
                                     "not accept such a file; position (0;0) assumed").arg(b.id));
        }

        if (b.prim == Prim::FileEps) {
            // Распределение eps из готового файла. В модели это фигура с профилем в
            // режиме «читать из файла» — тогда она и рисуется, и возвращается в
            // задание дословно.
            //
            // `FILE_N`, `FILE_B`, `FILE_Y` — величина плазмы из файла (ядро X с
            // 26.10.2026; раньше разбор сравнивал пять символов `FILE_` и уводил их в
            // eps). В модели это плазменная фигура-прямоугольник с именем файла; её
            // склеит с соседними блоками той же области mergePlasmaInto.
            if (!b.suffix.isEmpty()) {
                Shape s;
                s.id = b.id;
                s.kind = ShapeKind::Rectangle;
                s.material = Material::Dielectric;
                const QString file = stripEpsExtension(b.params.value(0).trimmed());
                if (file.isEmpty()) {
                    ctx.warn(b.line, tpl::tr("BLOCK %1: у блока FILE%2 не указано имя "
                                             "файла",
                                             "BLOCK %1: the FILE%2 block has no file name").arg(b.id).arg(b.suffix));
                }
                if (b.suffix == QLatin1String("_N")) {
                    s.plasma.hasN = true;
                    s.plasma.nFile = file;
                } else if (b.suffix == QLatin1String("_B")) {
                    s.plasma.hasB = true;
                    s.plasma.bFile = file;
                } else {
                    s.plasma.hasY = true;
                    s.plasma.yFile = file;
                }
                if (b.params.size() >= 5) {
                    s.xMin = evalOr(ctx, b.line, tpl::tr("X_min блока FILE", "FILE block X_min"),
                                    b.params.value(1), angle, 0.0) * k + anchor.x();
                    s.xMax = evalOr(ctx, b.line, tpl::tr("X_max блока FILE", "FILE block X_max"),
                                    b.params.value(2), angle, 0.0) * k + anchor.x();
                    s.yMin = evalOr(ctx, b.line, tpl::tr("Y_min блока FILE", "FILE block Y_min"),
                                    b.params.value(3), angle, 0.0) * k + anchor.y();
                    s.yMax = evalOr(ctx, b.line, tpl::tr("Y_max блока FILE", "FILE block Y_max"),
                                    b.params.value(4), angle, 0.0) * k + anchor.y();
                } else {
                    s.xMin = s.xMax = anchor.x();
                    s.yMin = s.yMax = anchor.y();
                }
                // Анкер ставит сетку файла на место — потерять его значит сдвинуть среду.
                if (std::hypot(anchor.x(), anchor.y()) > 1e-6) {
                    s.plasma.customOrigin = true;
                    s.plasma.origin = anchor;
                }
                sm.anyPlasma = true;
                if (!mergePlasmaInto(&sm.shapes, s, step.parm.delta))
                    sm.shapes << s;
                continue;
            }

            Shape s;
            s.id = b.id;
            s.kind = ShapeKind::Rectangle;
            s.material = Material::Dielectric;
            s.epsProfile.enabled = true;
            s.epsProfile.fromFile = true;
            s.epsProfile.file = stripEpsExtension(b.params.value(0).trimmed());

            if (s.epsProfile.file.isEmpty()) {
                ctx.warn(b.line, tpl::tr("BLOCK %1: у блока FILE не указано имя файла "
                                         "распределения",
                                         "BLOCK %1: the FILE block has no distribution file name").arg(b.id));
            }

            // Координаты ядро у этого блока не читает — сетка лежит в самом файле, а
            // положение задаёт анкер. Но в авторских заданиях они написаны, и по ним
            // видно, где профиль лежит: берём их для показа в редакторе. Нет их —
            // остаётся точка анкера, и фигура просто не имеет площади.
            const bool haveBox = b.params.size() >= 5;
            if (haveBox) {
                s.xMin = evalOr(ctx, b.line, tpl::tr("X_min блока FILE", "FILE block X_min"),
                                b.params.value(1), angle, 0.0) * k + anchor.x();
                s.xMax = evalOr(ctx, b.line, tpl::tr("X_max блока FILE", "FILE block X_max"),
                                b.params.value(2), angle, 0.0) * k + anchor.x();
                s.yMin = evalOr(ctx, b.line, tpl::tr("Y_min блока FILE", "FILE block Y_min"),
                                b.params.value(3), angle, 0.0) * k + anchor.y();
                s.yMax = evalOr(ctx, b.line, tpl::tr("Y_max блока FILE", "FILE block Y_max"),
                                b.params.value(4), angle, 0.0) * k + anchor.y();
            } else {
                s.xMin = s.xMax = anchor.x();
                s.yMin = s.yMax = anchor.y();
            }

            // Анкер — это НЕ украшение: от него ядро отсчитывает `#Xmin`/`#Ymin` из
            // файла, то есть он и ставит распределение на место. Теряем его —
            // распределение уезжает. Держим там же, где начало отсчёта формулы.
            const QPointF centre = s.epsOrigin();
            if (std::hypot(centre.x() - anchor.x(), centre.y() - anchor.y()) > 1e-6) {
                s.epsProfile.customOrigin = true;
                s.epsProfile.origin = anchor;
            }

            sm.shapes << s;
            continue;
        }

        if (b.prim == Prim::InputX || b.prim == Prim::InputY) {
            const bool alongY = (b.prim == Prim::InputX);
            Port p;
            p.id = b.id;
            p.mode = (b.params.value(0) == QLatin1String("MAGNETIC")) ? PortMode::Plane
                                                                     : PortMode::H10;
            // ⚠️ В старых заданиях первым параметром входа встречается произвольный
            // профиль поля («INPUT_X sin(y*3.141592653589);»). Нынешнее ядро такое
            // отвергает (TmcRTH_BolckList.cpp:2331-2342 — только METAL, MAGNETIC
            // или пусто), поэтому молча разбирать это нельзя.
            if (!b.params.value(0).isEmpty() && b.params.value(0) != QLatin1String("MAGNETIC")
                && b.params.value(0) != QLatin1String("METAL")) {
                ctx.warn(b.line, tpl::tr("BLOCK %1: профиль входа выражением («%2») "
                                         "нынешнее ядро не принимает — оно ждёт METAL, "
                                         "MAGNETIC или пусто; принята волна H10",
                                         "BLOCK %1: the current kernel does not accept a port "
                                         "profile given by an expression ('%2') - it expects "
                                         "METAL, MAGNETIC or empty; the H10 wave is assumed")
                                     .arg(b.id).arg(b.params.value(0)));
            }

            // Апертура ЛОКАЛЬНАЯ: к ней прибавляется анкер поперёк плоскости входа.
            const double along = alongY ? anchor.y() : anchor.x();
            p.from = evalOr(ctx, b.line, tpl::tr("нижнюю границу входа",
                                                 "the lower port boundary"), b.params.value(1),
                            angle, 0.0) * k + along;
            p.to = evalOr(ctx, b.line, tpl::tr("верхнюю границу входа",
                                               "the upper port boundary"), b.params.value(2),
                          angle, 0.0) * k + along;

            const double tMin = evalOr(ctx, b.line, tpl::tr("T_min входа", "port T_min"),
                                       b.params.value(3), angle, 0.0) * step.parm.timeToNs;
            const double tMax = evalOr(ctx, b.line, tpl::tr("T_max входа", "port T_max"),
                                       b.params.value(4), angle, 0.0) * step.parm.timeToNs;

            // Плоскость входа ядро берёт буквально из анкера, а «левый/правый»
            // выводит само — по половине коробки.
            const double plane = alongY ? anchor.x() : anchor.y();
            p.side = directionForPlane(alongY, plane, box);
            const double def = portPlane(p, box); // p.plane ещё пуст -> граница по side
            if (std::fabs(plane - def) > 1e-9)
                p.plane = plane;

            restoreSignal(ctx, b.line, b.params.value(5), b.params.value(6), tMin, tMax,
                          step.parm, &p);

            // Пассивный вход — это окно, начинающееся позже конца счёта (tpl-guide 5.5).
            const bool late = tMin > step.parm.timeMax + 1e-9;
            // У сигнала из файла амплитуда живёт в форме, а не в Signal::amplitude,
            // и «ноль вместо Amp» там невозможен: на месте Amp стоит имя файла.
            const bool silent = p.signal.source != SignalSource::File
                                && (p.signal.shape != SignalShape::Gaussian)
                                && std::fabs(p.signal.amplitude) < 1e-12;
            p.active = !late && !silent;

            if (!p.active) {
                // У пассивного входа сигнала нет вовсе: и окно, и амплитуда у него —
                // заглушки, которыми его глушили при записи. Восстанавливать по ним
                // «прямоугольный импульс длиной 115 нс» значит выдумать то, чего в
                // задании не было. Возвращаем умолчание — форму зададут, когда порт
                // сделают активным.
                p.signal = Signal{};
            }

            // Восьмой параметр — среда в сечении входа, записанная ДОБАВКОЙ (eps-1).
            // Модель хранит саму eps, поэтому единицу возвращаем здесь.
            //
            // Пустой параметр помечаем NaN: решать, «по среде» вход или «вакуум руками»,
            // можно только когда известны все фигуры (см. конец readTplText).
            p.eps = std::numeric_limits<double>::quiet_NaN();
            if (b.params.size() > 7 && !b.params.value(7).trimmed().isEmpty()) {
                // Д-5: W — число или выражение из своих переменных. `t` и `f` в нём ядро
                // с 26.10.2026 отвергает при разборе: раньше они молча брали значения
                // предыдущего блока.
                static const QRegularExpression tf(QStringLiteral("(?<![A-Za-z_0-9])[tf](?![A-Za-z_0-9(])"));
                if (tf.match(b.params.value(7)).hasMatch()) {
                    ctx.warn(b.line, tpl::tr("BLOCK %1: в параметре W входа («%2») есть "
                                             "t или f — ядро такой файл отвергнет. При "
                                             "сохранении W будет записан числом",
                                             "BLOCK %1: the port W parameter ('%2') contains t or "
                                             "f - the kernel will reject such a file. W will be "
                                             "written as a number when saving")
                                         .arg(b.id).arg(b.params.value(7).trimmed()));
                }
                bool okEps = false;
                const double add = evalOr(ctx, b.line, tpl::tr("eps в сечении входа",
                                                               "eps in the port cross-section"),
                                          b.params.value(7), angle, 0.0, &okEps);
                if (okEps) {
                    p.eps = add + 1.0;
                    if (p.eps <= 0.0) {
                        ctx.warn(b.line, tpl::tr("BLOCK %1: eps в сечении входа = %2 — "
                                                 "ядро такой файл не примет",
                                                 "BLOCK %1: eps in the port cross-section = %2 - "
                                                 "the kernel will not accept such a file")
                                             .arg(b.id).arg(p.eps));
                    }
                } else {
                    p.eps = 1.0;
                    ctx.warn(b.line, tpl::tr("BLOCK %1: добавку eps «%2» вычислить не "
                                             "удалось — вход оставлен в вакууме",
                                             "BLOCK %1: the eps increment '%2' could not be "
                                             "evaluated - the port is left in vacuum")
                                         .arg(b.id).arg(b.params.value(7).trimmed()));
                }
            }
            if (b.params.size() > 8) {
                ctx.warn(b.line, tpl::tr("BLOCK %1: у входа больше восьми параметров — "
                                         "лишние отброшены",
                                         "BLOCK %1: the port has more than eight parameters - the "
                                         "extra ones are discarded").arg(b.id));
            }
            sm.ports << p;
            continue;
        }

        Shape s;
        s.id = b.id;
        applyMaterial(ctx, b.line, b.params.value(0), b.suffix, angle, k, &s);
        if (!b.suffix.isEmpty())
            sm.anyPlasma = true;

        switch (b.prim) {
        case Prim::Rect: {
            s.kind = ShapeKind::Rectangle;
            s.xMin = evalOr(ctx, b.line, tpl::tr("X_min блока",
                                                 "block X_min"), b.params.value(1), angle,
                            0.0) * k + anchor.x();
            s.xMax = evalOr(ctx, b.line, tpl::tr("X_max блока",
                                                 "block X_max"), b.params.value(2), angle,
                            0.0) * k + anchor.x();
            s.yMin = evalOr(ctx, b.line, tpl::tr("Y_min блока",
                                                 "block Y_min"), b.params.value(3), angle,
                            0.0) * k + anchor.y();
            s.yMax = evalOr(ctx, b.line, tpl::tr("Y_max блока",
                                                 "block Y_max"), b.params.value(4), angle,
                            0.0) * k + anchor.y();
            if (b.moving) {
                s.vx = evalOr(ctx, b.line, QStringLiteral("Vx"), b.params.value(5), angle, 0.0);
                s.vy = evalOr(ctx, b.line, QStringLiteral("Vy"), b.params.value(6), angle, 0.0);
            }
            break;
        }
        case Prim::Circle: {
            s.kind = ShapeKind::Circle;
            s.center = anchor; // центра в блоке нет — им служит анкер
            s.rMin = evalOr(ctx, b.line, QStringLiteral("R_min"), b.params.value(1), angle, 0.0) * k;
            s.rMax = evalOr(ctx, b.line, QStringLiteral("R_max"), b.params.value(2), angle, 0.0) * k;
            const double a2r = (angle == AngleUnit::Gradus) ? 3.14159265358979323846 / 180.0 : 1.0;
            s.fiMin = evalOr(ctx, b.line, QStringLiteral("F_min"), b.params.value(3), angle, 0.0) * a2r;
            s.fiMax = evalOr(ctx, b.line, QStringLiteral("F_max"), b.params.value(4), angle, 0.0) * a2r;
            if (b.moving) {
                s.vx = evalOr(ctx, b.line, QStringLiteral("Vx"), b.params.value(5), angle, 0.0);
                s.vy = evalOr(ctx, b.line, QStringLiteral("Vy"), b.params.value(6), angle, 0.0);
            }
            break;
        }
        case Prim::Polygon: {
            for (const auto &pt : b.points) {
                const double x = evalOr(ctx, b.line, tpl::tr("X вершины",
                                                             "vertex X"), pt.first, angle,
                                        0.0) * k + anchor.x();
                const double y = evalOr(ctx, b.line, tpl::tr("Y вершины",
                                                             "vertex Y"), pt.second, angle,
                                        0.0) * k + anchor.y();
                s.points << QPointF(x, y);
            }
            if (b.moving && b.extra.size() >= 2) {
                s.vx = evalOr(ctx, b.line, QStringLiteral("Vx"), b.extra.at(0), angle, 0.0);
                s.vy = evalOr(ctx, b.line, QStringLiteral("Vy"), b.extra.at(1), angle, 0.0);
            }
            dropPaddingPoints(ctx, b.line, b.id, &s.points);
            // У КОНТУРНОГО материала ядро последнюю точку с первой не соединяет:
            // замкнутый контур в файле записан повтором первой вершины.
            const bool closedByHand = s.points.size() >= 4
                                      && s.points.first() == s.points.last();
            if (s.isContour() && closedByHand) {
                s.points.removeLast();
                s.kind = ShapeKind::Polygon;
            } else if (s.isContour()) {
                s.kind = ShapeKind::Polyline; // ломаная, как её понимает ядро
            } else {
                s.kind = ShapeKind::Polygon;  // выражение ядро замыкает само
            }
            if (s.points.size() < 2)
                ctx.warn(b.line, tpl::tr("BLOCK %1: у полигона меньше двух вершин",
                                         "BLOCK %1: the polygon has fewer than two vertices").arg(b.id));
            break;
        }
        default:
            break;
        }

        // Начало отсчёта координат в формуле eps — это анкер блока, и другого способа
        // его задать в языке нет. Запоминаем его как есть: в авторских заданиях он
        // сплошь и рядом стоит НЕ в центре фигуры (`data/cir_quas.h`, блок 28:
        // прямоугольник по x 140...412, а формула отсчитана от 550 — от центра сосуда
        // снаружи фигуры). Подставить сюда центр значило бы молча сдвинуть среду на
        // 274 мм при первом же сохранении.
        if (s.hasEpsProfile() && s.kind != ShapeKind::Circle) {
            const QPointF centre = s.epsOrigin();
            if (std::hypot(centre.x() - anchor.x(), centre.y() - anchor.y()) > 1e-6) {
                s.epsProfile.customOrigin = true;
                s.epsProfile.origin = anchor;
            }
        }

        // То же и для профиля плазмы: у токамака анкер блока с плотностью стоит в
        // (0; 160) мм, и без него формула отсчитывалась бы от угла коробки, а плазма
        // при сохранении уехала бы на полкамеры. Эмиттер по умолчанию ставит плазме
        // анкер (0; 0) — вот с ним и сравниваем.
        if (s.plasma.hasFormula() && s.kind != ShapeKind::Circle
            && std::hypot(anchor.x(), anchor.y()) > 1e-6) {
            s.plasma.customOrigin = true;
            s.plasma.origin = anchor;
        }

        // Три плазменных примитива одной области — одна фигура модели.
        if (s.plasma.any() && mergePlasmaInto(&sm.shapes, s, step.parm.delta))
            continue;

        sm.shapes << s;
    }

    return sm;
}

/// Убирает хвост шага углового свипа: `_ref` у опорного шага, `_a30`/`_a12p5`/`_am15`
/// у шагов с повёрнутым объектом (см. `sweepFileTag`).
/// @param[in] name  имя файла из `#OUTPUT`
/// @return Имя без хвоста свипа.
QString stripSweepSuffix(const QString &name)
{
    static const QRegularExpression re(QStringLiteral("_(ref|am?[0-9p]+)$"));
    QString s = name;
    return s.remove(re);
}

/// Убирает хвост `_f<частота>`, которым эмиттер разводит `.t` шагов свипа.
///
/// Срезаем ТОЛЬКО точное совпадение с меткой частоты ЭТОГО шага. Регулярным
/// выражением по виду «подчёркивание, f, цифры» тут обойтись нельзя: авторский файл
/// вполне может называться `wg_f30`, и тогда при первом же сохранении программа
/// молча переименовала бы человеку выходные данные.
/// @param[in] name     имя файла из `#OUTPUT`
/// @param[in] freqGhz  частота шага, ГГц; <= 0 — имя не меняется
/// @return Имя без хвоста `_<freqFileTag(freqGhz)>`, если он есть.
QString stripFreqSuffix(const QString &name, double freqGhz)
{
    if (freqGhz <= 0.0)
        return name;
    const QString tail = QLatin1Char('_') + freqFileTag(freqGhz);
    return name.endsWith(tail) ? name.left(name.size() - tail.size()) : name;
}

/// Убирает хвост `_p<N>` — так эмиттер помечает файлы разных возбуждений.
/// @param[in] name  имя файла
/// @return Имя без хвоста `_p<число>`.
QString stripExciterSuffix(const QString &name)
{
    static const QRegularExpression re(QStringLiteral("_p\\d+$"));
    QString s = name;
    return s.remove(re);
}

/// @brief Возвращает общее начало всех строк списка.
/// @param[in] list  строки
/// @return Наибольший общий префикс; пусто для пустого списка.
QString commonPrefix(const QStringList &list)
{
    if (list.isEmpty())
        return {};
    QString p = list.first();
    for (const QString &s : list) {
        int n = 0;
        while (n < p.size() && n < s.size() && p.at(n) == s.at(n))
            ++n;
        p.truncate(n);
    }
    return p;
}

} // namespace

// ===========================================================================

PreprocessResult preprocess(const QString &text, const QString &baseDir)
{
    PreprocessResult res;

    Records recs = loadRecords(text);
    joinBackslash(&recs);

    IncludeCtx ictx;
    ictx.baseDir = baseDir;
    ictx.included = &res.includedFiles;
    ictx.warnings = &res.warnings;
    ictx.errors = &res.errors;
    processIncludes(&recs, &ictx);

    expandMacros(&recs, &res);

    res.text = recs.join(QString());
    return res;
}

/// Подписи блоков из комментариев ИСХОДНОГО текста: «BLOCK 20; ! плазменный столб».
///
/// Препроцессор режет `!` первым делом, поэтому к разбору подписи уже не доживают.
/// А между тем это единственное человеческое имя фигуры во всём файле — и наш
/// эмиттер сам его туда и пишет. Собираем до раскрытия макросов, по номеру блока.
///
/// Номер, записанный выражением (в макро-теле он бывает параметром), пропускаем:
/// сопоставить такую подпись фигуре нечем, а гадать нельзя.
/// @param[in] text  исходный текст задания или включаемого файла (до препроцессора)
/// @return Имена фигур и портов по номеру блока (без пояснения после « — » / « -- »).
QHash<int, QString> collectBlockLabels(const QString &text)
{
    // Пробелы ищем БЕЗ перевода строки ([^\S\n]), иначе `\s*` перепрыгивает на
    // следующую строку и подписью становится закомментированная строка кода:
    //     BLOCK 1;
    //     !  INPUT_X ; -a1/2; a1/2; 0; 11 ;
    // В legacy-заданиях такие выключенные строки встречаются постоянно.
    static const QRegularExpression re(
        QStringLiteral("^[^\\S\\n]*BLOCK[^\\S\\n]+(\\d+)[^\\S\\n]*;[^\\S\\n]*"
                       "(?:\\\\\\\\)?[^\\S\\n]*![^\\S\\n]*(.+?)[^\\S\\n]*$"),
        QRegularExpression::MultilineOption);

    QHash<int, QString> out;
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        QString label = m.captured(2);
        // Хвост «\\» — это перенос строки макроса, а не часть подписи.
        while (label.endsWith(QLatin1Char('\\')))
            label.chop(1);

        // Эмиттер пишет комментарий как «имя — пояснение» (что за материал, какая
        // плазма, активен ли вход). Забрать нужно ТОЛЬКО имя: иначе пояснение
        // прилипнет к имени, а при следующем сохранении к нему допишется новое —
        // и подпись будет разрастаться с каждым циклом «открыл-сохранил».
        // Разделитель зависит от языка, на котором файл записан: русский эмиттер ставит
        // « — », английский — « -- » (в английском тексте задания только ASCII).
        // Читаем оба независимо от текущего языка.
        int dash = label.indexOf(QStringLiteral(" — "));
        if (dash <= 0)
            dash = label.indexOf(QLatin1String(" -- "));
        if (dash > 0)
            label.truncate(dash);
        label = label.trimmed();
        if (!label.isEmpty())
            out.insert(m.captured(1).toInt(), label);
    }
    return out;
}

namespace {

/// Имена вариантов из комментариев эмиттера, по порядку и без повторов:
///     ! ===== вариант: пустой, FREQ = 3.0 ГГц =====        (явный шаг)
///     ! вариант: пустой, возбуждён вход 1                   (перед `#define` макроса)
/// Файл, записанный по-английски, несёт те же строки как «variant: …, excited port 1» —
/// понимаем оба языка, какой бы язык ни был включён сейчас.
/// @param[in] text  исходный текст задания или включаемого файла
/// @return Имена вариантов в порядке первого появления.
QStringList collectVariantTitles(const QString &text)
{
    static const QRegularExpression re(
        QStringLiteral("^[^\\S\\n]*!.*?(?:вариант|variant): (.+?)"
                       "(?:, возбуждён.*|, excited.*|, FREQ.*|[^\\S\\n]*=====.*)?[^\\S\\n]*$"),
        QRegularExpression::MultilineOption);
    QStringList out;
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while (it.hasNext()) {
        const QString name = it.next().captured(1).trimmed();
        if (!name.isEmpty() && !out.contains(name))
            out << name;
    }
    return out;
}

/// Описание задания из шапки, которую пишет наш эмиттер:
///
///     ! Задание сгенерировано TPL Designer.
///     ! <описание, строка за строкой>
///     ! Ядро: tmc_rth (H-мода)
///
/// Чужие файлы такой шапки не имеют, и описания у них нет — это нормально.
/// Шапка английского файла: «! Task generated by TPL Designer.» … «! Kernel: …» —
/// узнаём оба варианта независимо от текущего языка.
/// @param[in] src  исходный текст задания
/// @return Описание (строки без `! `); пусто — шапки этой программы нет в первых
///         шести строках.
QString readOwnDescription(const QString &src)
{
    const QStringList lines = src.split(QLatin1Char('\n'));
    const auto isOwnHeader = [](const QString &l) {
        return l.startsWith(QStringLiteral("! Задание сгенерировано TPL Designer"))
               || l.startsWith(QLatin1String("! Task generated by TPL Designer"));
    };
    int i = 0;
    while (i < lines.size() && !isOwnHeader(lines.at(i)))
        ++i;
    if (i >= lines.size() || i > 5)
        return {};
    QStringList out;
    for (++i; i < lines.size(); ++i) {
        const QString &l = lines.at(i);
        if (!l.startsWith(QLatin1Char('!')) || l.startsWith(QStringLiteral("! Ядро:"))
            || l.startsWith(QLatin1String("! Kernel:")))
            break;
        QString t = l.mid(1);
        if (t.startsWith(QLatin1Char(' ')))
            t = t.mid(1);
        out << t;
    }
    return out.join(QLatin1Char('\n'));
}

/// Служебная строка углового свипа (см. `sweepHeader` в emitter.cpp).
struct SweepMarker {
    /// Строка найдена, и в ней есть и блоки, и углы.
    bool valid = false;
    /// Номера блоков поворачиваемых фигур.
    QSet<int> blocks;
    /// Ось поворота, мм.
    QPointF pivot;
    /// Углы шагов, градусы, в порядке шагов.
    QVector<double> angles;
    /// Первый шаг — опорный (без объекта).
    bool ref = false;
};

/// @brief Ищет в тексте служебную строку `! @TPLD-SWEEP ...` и разбирает её.
/// @param[in] src  исходный текст задания
/// @return Разобранная строка; `valid == false` — строки нет или она неполна.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#sweep
SweepMarker readSweepMarker(const QString &src)
{
    SweepMarker m;
    static const QRegularExpression re(
        QStringLiteral("^!\\s*%1\\s+blocks=([0-9,]*);\\s*pivot=([-0-9.eE+]+),([-0-9.eE+]+);"
                       "\\s*angles=([-0-9.eE+,]*);\\s*ref=([01])")
            .arg(QRegularExpression::escape(QLatin1String(kSweepMarker))),
        QRegularExpression::MultilineOption);
    const QRegularExpressionMatch hit = re.match(src);
    if (!hit.hasMatch())
        return m;
    for (const QString &b : hit.captured(1).split(QLatin1Char(','), Qt::SkipEmptyParts))
        m.blocks << b.toInt();
    m.pivot = QPointF(hit.captured(2).toDouble(), hit.captured(3).toDouble());
    for (const QString &a : hit.captured(4).split(QLatin1Char(','), Qt::SkipEmptyParts))
        m.angles << a.toDouble();
    m.ref = hit.captured(5) == QLatin1String("1");
    m.valid = !m.blocks.isEmpty() && !m.angles.isEmpty();
    return m;
}

/// Округление до 1e-11 мм: убирает шум `cos`/`sin`, оставшийся после обратного
/// поворота, чтобы одна и та же фигура со всех шагов свипа склеилась в одну.
///
/// Мельче нанометра — намеренно: эмиттер пишет смещение от оси десятью значащими
/// цифрами (0.9711089277), и грубое округление съедало бы десятую цифру — повторная
/// запись отличалась бы от исходной (найдено тестом на профиле крыла).
/// @param[in] v  координата, мм (или угол, рад)
/// @return Значение, округлённое до 1e-11, без отрицательного нуля.
double snapNm(double v)
{
    const double r = std::round(v * 1e11) / 1e11;
    return r == 0.0 ? 0.0 : r; // без «-0»
}

/// Вернуть повёрнутую фигуру в исходное положение.
///
/// Повёрнутый прямоугольник эмиттер пишет полигоном из четырёх углов; вернувшись
/// в исходное положение, он снова лежит по осям — и снова становится прямоугольником,
/// иначе круговорот «записал — прочитал» превращал бы каждый прямоугольник в контур.
/// @param[in] s         фигура, прочитанная из шага свипа
/// @param[in] pivot     ось поворота, мм
/// @param[in] angleDeg  угол этого шага, градусы
/// @return Фигура, повёрнутая на -angleDeg; четырёхугольник, лёгший по осям, снова
///         становится прямоугольником.
Shape unrotateShape(const Shape &s, const QPointF &pivot, double angleDeg)
{
    Shape r = rotatedShape(s, pivot, -angleDeg);
    r.center = QPointF(snapNm(r.center.x()), snapNm(r.center.y()));
    r.fiMin = snapNm(r.fiMin);
    r.fiMax = snapNm(r.fiMax);
    for (QPointF &pt : r.points)
        pt = QPointF(snapNm(pt.x()), snapNm(pt.y()));

    if (r.kind == ShapeKind::Polygon && r.points.size() == 4) {
        const QVector<QPointF> &q = r.points;
        double xLo = 0, xHi = 0, yLo = 0, yHi = 0;
        r.bounds(&xLo, &xHi, &yLo, &yHi);
        // Сравнение с допуском: после обратного поворота остаётся шум порядка 1e-12 мм.
        const auto near = [](double a, double b) { return std::fabs(a - b) < 1e-8; };
        bool axis = xHi - xLo > 1e-6 && yHi - yLo > 1e-6;
        QSet<int> seen; // какой угол занят: 0 — левый нижний, 1 — правый, 2 — верхний...
        for (const QPointF &pt : q) {
            const bool cx = near(pt.x(), xLo) || near(pt.x(), xHi);
            const bool cy = near(pt.y(), yLo) || near(pt.y(), yHi);
            axis &= cx && cy;
            seen << ((near(pt.x(), xLo) ? 0 : 1) | (near(pt.y(), yLo) ? 0 : 2));
        }
        // Все четыре угла различны — иначе это не прямоугольник, а вырожденный контур.
        axis &= (seen.size() == 4);
        if (axis) {
            // Углы — по 1e-9 мм: у настоящего прямоугольника они и так круглые.
            const auto nm = [](double v) {
                const double x = std::round(v * 1e9) / 1e9;
                return x == 0.0 ? 0.0 : x;
            };
            r.kind = ShapeKind::Rectangle;
            r.xMin = nm(xLo); r.xMax = nm(xHi); r.yMin = nm(yLo); r.yMax = nm(yHi);
            r.points.clear();
        }
    }
    return r;
}

} // namespace

// Этапы: препроцессор -> подписи блоков и имена вариантов из исходного текста ->
// разбор шагов -> модели шагов -> разворот свипа -> склейка одинаковых фигур и
// портов -> частоты, возбуждения, варианты -> выходные файлы -> eps входов ->
// сверка числа шагов при обратной записи.
ReadResult readTplText(const QString &text, const QString &baseDir, bool unitsFirst)
{
    ReadResult r;

    QString src = text;
    src.replace(QLatin1String("\r\n"), QLatin1String("\n"));
    src.replace(QLatin1Char('\r'), QLatin1Char('\n'));

    const PreprocessResult pp = preprocess(src, baseDir);
    r.warnings += pp.warnings;
    r.errors += pp.errors;
    r.includedFiles = pp.includedFiles;
    r.expandedText = pp.text;
    r.wasMacro = (pp.macroCalls > 0);
    if (!r.ok())
        return r;

    // Подписи собираем и из самого файла, и из подключённых: у макро-версии вся
    // топология (а значит и подписи) лежит именно в .h.
    QHash<int, QString> labels = collectBlockLabels(src);
    // Имена вариантов — из комментариев эмиттера «! вариант: имя». Без них вариант
    // получал вместо имени метку файлов («short» вместо «короткое замыкание»).
    QStringList variantTitles = collectVariantTitles(src);
    for (const QString &inc : std::as_const(pp.includedFiles)) {
        QString incText;
        QString incErr;
        // Регистр здесь так же не важен, как и при вставке: файл тот же самый.
        const QString incPath = findFileIgnoringCase(baseDir, inc);
        if (incPath.isEmpty() || !readTplFileText(incPath, &incText, &incErr))
            continue; // сам факт пропажи include препроцессор уже отметил

        const QHash<int, QString> more = collectBlockLabels(incText);
        for (auto i = more.constBegin(); i != more.constEnd(); ++i)
            labels.insert(i.key(), i.value());
        for (const QString &t : collectVariantTitles(incText)) {
            if (!variantTitles.contains(t))
                variantTitles << t; // те же имена стоят и в .tpl перед вызовами
        }
    }

    Ctx ctx;
    ctx.warnings = &r.warnings;
    ctx.errors = &r.errors;
    ctx.baseDir = baseDir;
    ctx.unitsFirst = unitsFirst;
    ctx.unitAfterValue = &r.unitAfterValueLines;

    bool sawHeader = false;
    bool sawEof = false;
    const QVector<RawStep> raw = parseSteps(ctx, pp.text, &sawHeader, &sawEof);

    if (!sawHeader) {
        r.errors << tpl::tr("это не файл задания .tpl: нет заголовка #TMC_RT_H",
                            "this is not a .tpl task file: no #TMC_RT_H header");
        return r;
    }
    if (!sawEof)
        r.warnings << tpl::tr("в файле нет #EOF — ядро такой файл не примет",
                              "the file has no #EOF - the kernel will not accept such a file");
    if (raw.isEmpty()) {
        r.errors << tpl::tr("в задании нет ни одного #STEP — считать нечего",
                            "the task has no #STEP - nothing to compute");
        return r;
    }
    if (!raw.first().parm.haveTimeUnit) {
        r.warnings << tpl::tr("TIME_UNIT не задан. Ядро по умолчанию понимает секунды, "
                              "редактор принял наносекунды — проверьте время счёта",
                              "TIME_UNIT is not set. By default the kernel assumes seconds, the "
                              "editor assumed nanoseconds - check the run time");
    }

    // --- параметры задания берём из первого шага ---
    const RawParams &p0 = raw.first().parm;
    Project &prj = r.project;
    prj.box = Box{p0.xMin, p0.xMax, p0.yMin, p0.yMax};
    prj.delta = p0.delta;
    prj.timeMax = p0.timeMax;
    prj.unit = p0.lengthUnit;
    if (std::fabs(p0.timeMin) > 1e-9) {
        r.warnings << tpl::tr("начало счёта TIME = %1 нс не равно нулю — модель хранит "
                              "только конец счёта",
                              "the start of the run TIME = %1 ns is not zero - the model stores "
                              "only the end of the run").arg(num(p0.timeMin));
    }

    // --- разбираем шаги ---
    QVector<StepModel> steps;
    steps.reserve(raw.size());
    bool anyPlasma = false;
    for (const RawStep &s : raw) {
        StepModel sm = buildStep(ctx, s);
        anyPlasma |= sm.anyPlasma;
        steps << sm;

        const Box b{s.parm.xMin, s.parm.xMax, s.parm.yMin, s.parm.yMax};
        if (std::fabs(b.xMin - prj.box.xMin) > 1e-9 || std::fabs(b.xMax - prj.box.xMax) > 1e-9
            || std::fabs(b.yMin - prj.box.yMin) > 1e-9 || std::fabs(b.yMax - prj.box.yMax) > 1e-9
            || std::fabs(s.parm.delta - prj.delta) > 1e-9
            || std::fabs(s.parm.timeMax - prj.timeMax) > 1e-9) {
            r.warnings << tpl::tr("шаг в строке %1 считает другую область или другой шаг "
                                  "сетки — модель хранит одну коробку на всё задание, "
                                  "взяты параметры первого шага",
                                  "the step at line %1 computes a different domain or a different "
                                  "grid step - the model stores one box for the whole task, the "
                                  "parameters of the first step are taken").arg(s.line);
        }
    }
    prj.kernel = anyPlasma ? Kernel::X : Kernel::H;
    prj.description = readOwnDescription(src);

    // --- угловой свип: развернуть повёрнутый объект обратно ---
    //
    // На каждом шаге свипа объект повёрнут на свой угол, и без этого разворота каждый
    // шаг дал бы свою копию фигуры. Какие блоки поворачивались и на какие углы — знает
    // служебная строка в шапке; шаги идут в том же порядке, в каком их пишет эмиттер:
    // опорный (без объекта), затем углы по списку.
    const SweepMarker sweep = readSweepMarker(src);
    bool sweepOk = false;
    if (sweep.valid) {
        const int first = sweep.ref ? 1 : 0;
        if (steps.size() == sweep.angles.size() + first) {
            sweepOk = true;
            for (int si = first; si < steps.size(); ++si) {
                const double a = sweep.angles.at(si - first);
                for (Shape &s : steps[si].shapes) {
                    if (sweep.blocks.contains(s.id))
                        s = unrotateShape(s, sweep.pivot, a);
                }
            }
        } else {
            r.warnings << tpl::tr("в шапке записан поворот объекта на %1 углов, а шагов в "
                                  "файле %2 — поворот не восстановлен, шаги прочитаны "
                                  "как есть",
                                  "the header records an object rotation by %1 angles, but the "
                                  "file has %2 steps - the rotation is not restored, the steps "
                                  "are read as they are")
                              .arg(sweep.angles.size()).arg(steps.size());
        }
    }
    QVector<int> sweepShapeIds;

    // --- объединяем фигуры и порты: одинаковая геометрия в разных шагах = ОДНА фигура ---
    QHash<QString, int> shapeByKey;
    QHash<QString, int> portByKey;
    QSet<int> usedIds;
    QVector<QVector<int>> stepShapeIds(steps.size());
    QVector<QVector<int>> stepActiveIds(steps.size());

    int freeId = 0;
    const auto reserve = [&](int id) {
        usedIds.insert(id);
        freeId = std::max(freeId, id);
    };

    // Перенумерацию копим и докладываем ОДНОЙ строкой на каждый исходный номер.
    //
    // Иначе задание вроде авторского `TEST.TPL` — там макрос разворачивается в 51 шаг,
    // и в каждом свой блок 20 — давало полсотни одинаковых предупреждений. Пользователю
    // важен факт «блок 20 встретился 51 раз, номера выданы новые», а не список.
    QHash<int, int> renamedShapes;
    QHash<int, int> renamedPorts;

    // Порядок обхода шагов. Обычно — как в файле. Но у свипа первым идёт опорный шаг
    // БЕЗ объекта, и фигуры из него встали бы впереди объекта, перепутав порядок
    // блоков при следующей записи. Поэтому опорный шаг разбираем последним.
    QVector<int> order;
    for (int si = 0; si < steps.size(); ++si)
        order << si;
    if (sweepOk && sweep.ref && order.size() > 1)
        std::rotate(order.begin(), order.begin() + 1, order.end());

    for (int si : std::as_const(order)) {
        // Новая фигура встаёт СРАЗУ ЗА той, что шла перед ней в этом шаге, а не в конец
        // списка. Иначе фигура, выключенная в первом варианте, при чтении уезжала бы в
        // конец — и сохранённое заново задание меняло бы порядок блоков.
        int after = -1; // позиция предыдущей фигуры этого шага в prj.shapes
        for (const Shape &s : steps.at(si).shapes) {
            const QString key = shapeKey(s);
            if (!shapeByKey.contains(key)) {
                Shape copy = s;
                if (usedIds.contains(copy.id)) {
                    ++renamedShapes[copy.id];
                    copy.id = ++freeId;
                }
                reserve(copy.id);
                // Подпись ищем по номеру блока ИЗ ФАЙЛА (s.id): copy.id к этому
                // моменту мог смениться из-за столкновения номеров.
                copy.name = labels.value(s.id);
                if (copy.name.isEmpty())
                    copy.name = tpl::tr("фигура %1", "shape %1").arg(s.id);
                const int pos = after + 1;
                for (auto it = shapeByKey.begin(); it != shapeByKey.end(); ++it) {
                    if (it.value() >= pos)
                        ++it.value();
                }
                shapeByKey.insert(key, pos);
                prj.shapes.insert(pos, copy);
            }
            after = shapeByKey.value(key);
            // Файл профиля записан только на одном шаге — переносим его имя на фигуру.
            Shape &merged = prj.shapes[shapeByKey.value(key)];
            if (!s.epsProfile.fromFile && merged.epsProfile.file.isEmpty()
                && !s.epsProfile.file.isEmpty())
                merged.epsProfile.file = s.epsProfile.file;
            const int mergedId = merged.id;
            stepShapeIds[si] << mergedId;
            if (sweepOk && sweep.blocks.contains(s.id) && !sweepShapeIds.contains(mergedId))
                sweepShapeIds << mergedId;
        }

        for (const Port &p : steps.at(si).ports) {
            const QString key = portKey(p);
            if (!portByKey.contains(key)) {
                Port copy = p;
                if (usedIds.contains(copy.id)) {
                    ++renamedPorts[copy.id];
                    copy.id = ++freeId;
                }
                reserve(copy.id);
                copy.name = labels.value(p.id);
                if (copy.name.isEmpty())
                    copy.name = tpl::tr("вход %1", "port %1").arg(prj.ports.size() + 1);
                portByKey.insert(key, prj.ports.size());
                prj.ports << copy;
            }
            if (p.active)
                stepActiveIds[si] << prj.ports.at(portByKey.value(key)).id;
        }
    }

    // Итог перенумерации: по строке на исходный номер, а не на каждое столкновение.
    {
        QList<int> ids = renamedShapes.keys();
        std::sort(ids.begin(), ids.end());
        for (int id : ids) {
            r.warnings << tpl::tr("номер блока %1 занят %2 разными фигурами — всем, кроме "
                                  "первой, выданы свободные номера. На расчёт это не "
                                  "влияет: номер блока произвольный, важна лишь "
                                  "уникальность",
                                  "block number %1 is used by %2 different shapes - all but the "
                                  "first were given free numbers. This does not affect the "
                                  "computation: the block number is arbitrary, only uniqueness "
                                  "matters")
                              .arg(id).arg(renamedShapes.value(id) + 1);
        }
        ids = renamedPorts.keys();
        std::sort(ids.begin(), ids.end());
        for (int id : ids) {
            r.warnings << tpl::tr("номер блока %1 занят и фигурой, и входом (%2 раза) — "
                                  "входу выдан свободный номер",
                                  "block number %1 is used both by a shape and by a port (%2 "
                                  "times) - the port was given a free number")
                              .arg(id).arg(renamedPorts.value(id));
        }
    }

    // --- частоты: различные, в порядке появления ---
    for (const StepModel &s : steps) {
        bool seen = false;
        for (double f : std::as_const(prj.freqs))
            seen |= (std::fabs(f - s.freq) < 1e-9);
        if (!seen)
            prj.freqs << s.freq;
    }

    // --- возбуждения: шаги, отличающиеся активным портом ---
    QVector<QVector<int>> activeSets;
    QVector<int> stepActiveSet(steps.size(), 0);
    for (int si = 0; si < steps.size(); ++si) {
        QVector<int> set = stepActiveIds.at(si);
        std::sort(set.begin(), set.end());
        int idx = activeSets.indexOf(set);
        if (idx < 0) {
            idx = activeSets.size();
            activeSets << set;
        }
        stepActiveSet[si] = idx;
    }

    bool useExciters = false;
    if (activeSets.size() > 1) {
        useExciters = true;
        for (const QVector<int> &set : activeSets)
            useExciters &= (set.size() == 1);
        if (useExciters) {
            for (const QVector<int> &set : activeSets)
                prj.exciters << set.first();
        } else {
            r.warnings << tpl::tr("шаги отличаются набором активных входов, но не по одному "
                                  "входу на шаг — очередь возбуждений не восстановлена, "
                                  "взят первый шаг",
                                  "the steps differ in the set of active ports, but not one port "
                                  "per step - the excitation queue is not restored, the first "
                                  "step is taken");
        }
    }

    // Активность показываем по ПЕРВОМУ шагу, а не по всей очереди возбуждений.
    //
    // Когда в задании два шага и в каждом свой активный вход, порты по очереди
    // возбуждаются, но ОДНОВРЕМЕННО активными не бывают никогда. Пометить активными
    // оба (раз оба входят в очередь) значит показать пользователю картину, которой
    // в задании нет: он увидит два излучающих входа вместо «сначала первый, потом
    // второй». Очередь при этом никуда не девается — она в Project::exciters и
    // показана на странице «Что считать»; эмиттер при непустой очереди этот флаг
    // не смотрит вовсе, так что записи он не меняет.
    for (Port &p : prj.ports)
        p.active = activeSets.value(0).contains(p.id);

    // --- варианты геометрии: шаги, отличающиеся набором фигур ---
    QVector<QVector<int>> shapeSets;
    QVector<int> stepShapeSet(steps.size(), 0);
    for (int si = 0; si < steps.size(); ++si) {
        QVector<int> set = stepShapeIds.at(si);
        std::sort(set.begin(), set.end());
        int idx = shapeSets.indexOf(set);
        if (idx < 0) {
            idx = shapeSets.size();
            shapeSets << set;
        }
        stepShapeSet[si] = idx;
    }

    // Имя файла сигналов первого шага каждого варианта — из него достаём метку.
    //
    // Суффикс «_p1» отрезаем ТОЛЬКО когда возбуждения действительно идут по очереди:
    // тогда его дописал эмиттер, разводя сигналы по портам. При единственном
    // возбуждении такой суффикс — часть авторского имени файла (задание так и
    // называется, wg_p1), и срезать его значит молча переименовать пользователю
    // выходные данные при первом же сохранении.
    const bool suffixIsOurs = prj.exciters.size() > 1;
    QStringList variantCores(shapeSets.size());
    for (int si = steps.size() - 1; si >= 0; --si) {
        // Порядок обратный записи: эмиттер дописывает вариант, потом возбуждение,
        // потом частоту — снимаем с конца.
        const QString file = stripFreqSuffix(steps.at(si).out.file, steps.at(si).freq);
        variantCores[stepShapeSet.at(si)] = suffixIsOurs ? stripExciterSuffix(file) : file;
    }

    QString baseSignal = variantCores.value(0);
    if (sweepOk) {
        // Свип — это не варианты: опорный шаг без объекта и шаги с объектом — одна
        // задача. Имена файлов у шагов свои (`_ref`, `_a30`), общее у них — основа.
        QStringList files;
        for (const StepModel &s : std::as_const(steps))
            files << stripSweepSuffix(stripFreqSuffix(s.out.file, s.freq));
        QString base = commonPrefix(files);
        while (base.endsWith(QLatin1Char('_')))
            base.chop(1);
        if (!base.isEmpty())
            baseSignal = base;

        prj.sweep.enabled = true;
        prj.sweep.shapeIds = sweepShapeIds;
        prj.sweep.pivot = sweep.pivot;
        // Ось в центре объекта — значит и дальше она едет вместе с ним.
        prj.sweep.pivotAtCenter = true;
        const QPointF centre = sweepPivot(prj);
        prj.sweep.pivotAtCenter =
            std::hypot(centre.x() - sweep.pivot.x(), centre.y() - sweep.pivot.y()) < 1e-6;
        prj.sweep.angles = sweep.angles;
        prj.sweep.referenceStep = sweep.ref;
    } else if (shapeSets.size() > 1) {
        QString base = commonPrefix(variantCores);
        while (base.endsWith(QLatin1Char('_')))
            base.chop(1);

        QStringList tags;
        bool goodTags = !base.isEmpty();
        for (const QString &core : std::as_const(variantCores)) {
            QString tag = core.mid(base.size());
            while (tag.startsWith(QLatin1Char('_')))
                tag = tag.mid(1);
            goodTags &= (!tag.isEmpty() && !tags.contains(tag));
            tags << tag;
        }
        if (!goodTags) {
            base = variantCores.value(0);
            tags.clear();
            for (int i = 0; i < shapeSets.size(); ++i)
                tags << QStringLiteral("v%1").arg(i + 1);
            r.warnings << tpl::tr("шаги считают разные наборы фигур; имена файлов не дают "
                                  "понять, как называются варианты — им выданы метки v1, "
                                  "v2, …",
                                  "the steps compute different sets of shapes; the file names do "
                                  "not tell what the variants are called - they were given the "
                                  "labels v1, v2, ...");
        }
        baseSignal = base;

        for (int vi = 0; vi < shapeSets.size(); ++vi) {
            Variant v;
            v.tag = tags.at(vi);
            v.name = tags.at(vi);
            // Имя из комментария — если их ровно столько, сколько вариантов: тогда
            // порядок у них общий (эмиттер пишет варианты внешним циклом).
            if (variantTitles.size() == shapeSets.size())
                v.name = variantTitles.at(vi);
            for (const Shape &s : std::as_const(prj.shapes)) {
                if (!shapeSets.at(vi).contains(s.id))
                    v.disabledShapes << s.id;
            }
            prj.variants << v;
        }
    }

    // --- выходные данные ---
    const RawOutput &o0 = steps.first().out;
    prj.output.signalFile = baseSignal;

    int fieldsCount = 0;
    bool anyTopology = false;
    bool anySMatrix = false;
    bool anyPattern = false;
    for (const StepModel &s : steps) {
        fieldsCount += s.out.fields ? 1 : 0;
        anyTopology |= s.out.topology;
        anySMatrix |= s.out.sMatrix;
        anyPattern |= s.out.pattern;
    }
    prj.output.topology = anyTopology;
    if (fieldsCount == 0)
        prj.output.fields = OutputSettings::Fields::Off;
    else if (fieldsCount == steps.size())
        prj.output.fields = OutputSettings::Fields::AllSteps;
    else if (fieldsCount == 1 && steps.first().out.fields)
        prj.output.fields = OutputSettings::Fields::FirstStep;
    else if (sweepOk && fieldsCount == 1 && sweep.ref && steps.size() > 1
             && steps.at(1).out.fields)
        prj.output.fields = OutputSettings::Fields::FirstStep; // первый шаг С ОБЪЕКТОМ
    else if (sweepOk && sweep.ref && fieldsCount == steps.size() - 1 && !steps.first().out.fields)
        prj.output.fields = OutputSettings::Fields::AllSteps;  // опорный шаг полей не пишет
    else {
        prj.output.fields = OutputSettings::Fields::AllSteps;
        r.warnings << tpl::tr("поля выгружаются на %1 шагах из %2 — модель знает только "
                              "«нигде», «на первом» и «на всех», выбрано «на всех»",
                              "fields are exported at %1 of %2 steps - the model knows only "
                              "\"nowhere\", \"only at the first step\" and \"at every step\"; "
                              "\"at every step\" is selected")
                          .arg(fieldsCount).arg(steps.size());
    }

    prj.output.sMatrix = anySMatrix;
    if (anySMatrix) {
        QStringList sCores;
        for (const StepModel &s : steps) {
            const QString core = sweepOk ? stripSweepSuffix(s.out.sFile) : s.out.sFile;
            if (s.out.sMatrix && !sCores.contains(core))
                sCores << core;
        }
        if (sCores.size() > 1) {
            QString base = commonPrefix(sCores);
            while (base.endsWith(QLatin1Char('_')))
                base.chop(1);
            prj.output.sMatrixFile = base.isEmpty() ? sCores.first() : base;
        } else {
            prj.output.sMatrixFile = sCores.value(0);
        }
        for (const StepModel &s : steps) {
            if (!s.out.sMatrix)
                continue;
            if (s.out.sNoWindow) {
                // Окно на весь счёт: так по умолчанию пишет многочастотный режим.
                prj.output.sMatrixLimitWideband = false;
                if (!hasFileSignal(prj)) {
                    r.warnings << tpl::tr("SMATRIX без окна T_min; T_max законен только "
                                          "при входе с файлом сигнала — в одночастотном "
                                          "режиме ядро такой шаг отвергнет. При сохранении "
                                          "будет записано окно со страницы «Вывод данных»",
                                          "SMATRIX without a T_min; T_max window is valid only "
                                          "for a port with a signal file - in single-frequency "
                                          "mode the kernel will reject such a step. When saving, "
                                          "the window from the \"Output\" page will be written");
                }
            } else {
                prj.output.sMatrixTMin = s.out.sTMin;
                prj.output.sMatrixTMax = s.out.sTMax;
                prj.output.sMatrixLimitWideband = true;
            }
            break;
        }
    }

    prj.output.directionalPattern = anyPattern;
    if (anyPattern) {
        QStringList pCores;
        for (const StepModel &s : steps) {
            if (!s.out.pattern)
                continue;
            const QString c = sweepOk ? stripSweepSuffix(s.out.pFile)
                                      : stripExciterSuffix(s.out.pFile);
            if (!pCores.contains(c))
                pCores << c;
        }
        QString base = commonPrefix(pCores);
        while (base.endsWith(QLatin1Char('_')))
            base.chop(1);
        prj.output.patternFile = base.isEmpty() ? pCores.value(0) : base;
        for (const StepModel &s : steps) {
            if (s.out.pattern) {
                prj.output.patternTMin = s.out.pTMin;
                prj.output.patternTMax = s.out.pTMax;
                break;
            }
        }
    }

    if (prj.output.signalFile.isEmpty())
        r.warnings << tpl::tr("в #OUTPUT не нашлось строки FILE — имя файла сигналов пусто",
                              "#OUTPUT has no FILE line - the signal file name is empty");

    prj.name = o0.file.isEmpty() ? QStringLiteral("task") : baseSignal;

    // --- среда в сечении входов ---
    //
    // Восьмой параметр входа — Δε той среды, где он стоит (диссертация К. Н. Климова,
    // (7.54)). Совпал со средой — вход «по среде», как и у нового задания. Задан другим
    // числом — это решение автора задания, держим его руками. Пуст, а вход стоит в
    // диэлектрике — это та самая ошибка, которую генератор закрывает: при сохранении
    // будет записана добавка среды.
    for (Port &port : prj.ports) {
        const double medium = portMediumEps(prj, port);
        if (std::isnan(port.eps)) {
            port.eps = 1.0;
            port.epsAuto = true;
            if (medium != 1.0) {
                r.warnings << tpl::tr("вход «%1» стоит в среде eps = %2, а параметр W "
                                      "(добавка eps-1) в файле пуст — вход согласован с "
                                      "вакуумом. При сохранении будет записана добавка "
                                      "%3",
                                      "port '%1' is in a medium with eps = %2, but the W "
                                      "parameter (eps-1 increment) in the file is empty - the "
                                      "port is matched to vacuum. When saving, the increment %3 "
                                      "will be written")
                                  .arg(port.name).arg(medium).arg(medium - 1.0);
            }
        } else {
            port.epsAuto = std::fabs(port.eps - medium) < 1e-9;
        }
    }

    // --- сверка: сколько шагов должно получиться при обратной записи ---
    if (prj.stepCount() != steps.size()) {
        r.warnings << tpl::tr("в файле %1 шагов, а из разобранной модели получится %2 — "
                              "часть различий между шагами модель не выражает",
                              "the file has %1 steps, but the parsed model will give %2 - the "
                              "model cannot express some of the differences between the steps")
                          .arg(steps.size()).arg(prj.stepCount());
    }

    return r;
}

ReadResult readTplFile(const QString &path, bool unitsFirst)
{
    ReadResult r;
    const QFileInfo fi(path);

    QString text;
    QString err;
    if (!readTplFileText(path, &text, &err)) {
        r.errors << err;
        return r;
    }

    r = readTplText(text, fi.absolutePath(), unitsFirst);
    if (r.ok() || !r.project.name.isEmpty())
        r.project.name = fi.completeBaseName();
    return r;
}

} // namespace tpl
