// Язык интерфейса: словарь «английский — русский» и фильтр событий, который переводит
// окна и меню при показе. Подробности — в tmci18n.h.
/// @file tmci18n.cpp
/// @brief Реализация перевода интерфейса: словарь `ui_strings.tsv`, шаблоны со вставками, фильтр показа окон.
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_i18n`.
/// @see https://www.tamic.ru/docs/code/gui-common.html#i18n
#include "tmci18n.h"

#include <QAbstractButton>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QDockWidget>
#include <QFile>
#include <QGroupBox>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMdiSubWindow>
#include <QMenu>
#include <QPointer>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QTabBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTranslator>
#include <QLibraryInfo>

namespace tmci18n {
namespace {

// Шаблон со вставками (%d, %s, %1 …): образец для разбора и строка другого языка.
/// @brief Шаблон строки со вставками: регулярное выражение исходной строки и строка другого языка.
struct Template {
    /// Выражение исходной строки: вставки — группы `(.+?)`, повторы — обратные ссылки.
    QRegularExpression re;
    /// Строка другого языка со вставками.
    QString target;
    // Вставка исходной строки -> номер группы захвата: «%2» -> группа, где
    // впервые стоит %2; printf-вставка -> «#k» (k-я по порядку). Без этого строка
    // с повторённой вставкой (%2×%2) подставляла числа не на свои места (Т-21).
    /// Ключ вставки (`%N` или `#k`) -> номер группы захвата.
    QHash<QString, int> group;
};

// Ключ вставки: «%N» для %1…%9, «#k» для k-й printf-вставки.
/// @brief Возвращает ключ вставки: `%1…%9` как есть, printf-вставка — `#k` по порядку.
/// @param[in]     ph     текст вставки
/// @param[in,out] order  счётчик printf-вставок
/// @return Ключ `%N` или `#k` (k — номер printf-вставки, начиная с 1).
QString placeholderKey(const QString &ph, int *order)
{
    if (ph.size() == 2 && ph.at(1).isDigit() && ph.at(1) != QLatin1Char('0'))
        return ph;
    return QStringLiteral("#%1").arg(++*order);
}

/// @brief Словарь в обе стороны: точные строки, строки без «&», шаблоны.
struct Dictionary {
    /// @name Точные строки
    ///@{
    /// Точные пары английский -> русский и обратно.
    QHash<QString, QString> enRu, ruEn;           // точные строки
    //@}
    /// @name Строки без «&»
    ///@{
    /// Ключ без «&» -> перевод с «&».
    QHash<QString, QString> enRuBare, ruEnBare;   // без «&» (ключ без «&» → перевод с «&»)
    //@}
    /// @name Шаблоны со вставками
    ///@{
    /// Шаблоны в обе стороны.
    QList<Template> enRuTpl, ruEnTpl;
    //@}
    /// Словарь загружен.
    bool loaded = false;
};

/// Словарь программы.
Dictionary g_dict;
/// Действующий язык — русский.
bool g_russian = false;
/// `install` выполнен.
bool g_installed = false;
/// Имя свойства объекта «не переводить».
const char *kNoTranslate = "tmcNoTranslate";   // свойство: не переводить (имена языков в меню)

/// @brief Возвращает путь к `Common.ini` (`%APPDATA%\TMC_Suite_Qt\Common.ini` или каталог настроек пользователя).
/// @return Полный путь к файлу `TMC_Suite_Qt/Common.ini`.
QString settingsPath()
{
    QString base = qEnvironmentVariable("APPDATA");   // Unicode: профиль вне CP1251 (Т-20)
    if (base.isEmpty())
        base = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    return QDir(base).filePath(QStringLiteral("TMC_Suite_Qt/Common.ini"));
}

/// @brief Убирает «&» подчёркнутых букв, сохраняя «&&».
/// @param[in] s  строка
/// @return Строка без одиночных «&».
QString stripAmp(QString s)
{
    // «&&» — настоящий амперсанд, остальные «&» — подчёркнутые буквы.
    s.replace(QStringLiteral("&&"), QStringLiteral("\x01"));
    s.remove(QLatin1Char('&'));
    s.replace(QStringLiteral("\x01"), QStringLiteral("&&"));
    return s;
}

/// @brief Заменяет `\n` и `\t` словаря на перевод строки и табуляцию.
/// @param[in] s  строка из словаря
/// @return Строка с настоящими переводами строки и табуляциями.
QString unescape(QString s)
{
    s.replace(QStringLiteral("\\n"), QStringLiteral("\n"));
    s.replace(QStringLiteral("\\t"), QStringLiteral("\t"));
    return s;
}

// %d %i %u %s %f %g %I64d %lld %ld %.3f … и %1 … %9 — вставки шаблона.
/// @brief Возвращает выражение вставок: printf (`%d`, `%.3f`, `%I64d` …) и `%1…%9`.
/// @return Ссылка на статическое выражение (живёт до конца программы).
const QRegularExpression &placeholderRe()
{
    static const QRegularExpression re(QStringLiteral("%(?:[-+ #0]*[0-9]*(?:\\.[0-9]+)?(?:I64|ll|l|h)?[diuxXsfgec]|[1-9])"));
    return re;
}

/// @brief Строит шаблон перевода из пары строк со вставками.
/// @param[in] from  исходная строка
/// @param[in] to    строка другого языка
/// @return Шаблон.
Template makeTemplate(const QString &from, const QString &to)
{
    QString pattern = QStringLiteral("^");
    int pos = 0, order = 0, groups = 0;
    Template t;
    auto it = placeholderRe().globalMatch(from);
    while (it.hasNext()) {
        const auto m = it.next();
        pattern += QRegularExpression::escape(from.mid(pos, m.capturedStart() - pos));
        const QString key = placeholderKey(m.captured(0), &order);
        if (t.group.contains(key)) {
            // Повтор той же вставки — то же значение (обратная ссылка).
            pattern += QStringLiteral("\\g{%1}").arg(t.group.value(key));
        } else {
            pattern += QStringLiteral("(.+?)");
            t.group.insert(key, ++groups);
        }
        pos = m.capturedEnd();
    }
    pattern += QRegularExpression::escape(from.mid(pos)) + QStringLiteral("$");
    t.re = QRegularExpression(pattern, QRegularExpression::DotMatchesEverythingOption);
    t.target = to;
    return t;
}

// Подставить захваченные значения во вставки строки другого языка: %1…%9 — по номеру,
// printf-вставки — по порядку.
/// @param[in] t  шаблон
/// @param[in] m  совпадение исходной строки с шаблоном
/// @return Строка другого языка с подставленными значениями.
QString fillTemplate(const Template &t, const QRegularExpressionMatch &m)
{
    QString out;
    int pos = 0, order = 0;
    auto it = placeholderRe().globalMatch(t.target);
    while (it.hasNext()) {
        const auto p = it.next();
        out += t.target.mid(pos, p.capturedStart() - pos);
        const QString ph = p.captured(0);
        const int n = t.group.value(placeholderKey(ph, &order), 0);
        out += (n > 0 && n <= m.lastCapturedIndex()) ? m.captured(n) : ph;
        pos = p.capturedEnd();
    }
    out += t.target.mid(pos);
    return out;
}

/// @brief Добавляет пару в обе стороны (первая пара для ключа побеждает).
/// @param[in] en  английский текст
/// @param[in] ru  русский текст
void addPair(const QString &en, const QString &ru)
{
    if (en.isEmpty() || ru.isEmpty() || en == ru)
        return;
    if (!g_dict.enRu.contains(en))
        g_dict.enRu.insert(en, ru);
    if (!g_dict.ruEn.contains(ru))
        g_dict.ruEn.insert(ru, en);
    const QString enBare = stripAmp(en), ruBare = stripAmp(ru);
    if (!g_dict.enRuBare.contains(enBare))
        g_dict.enRuBare.insert(enBare, ru);
    if (!g_dict.ruEnBare.contains(ruBare))
        g_dict.ruEnBare.insert(ruBare, en);
}

// Кнопки стандартных окон Qt (переводы Qt в поставку не кладём).
/// @brief Добавляет переводы кнопок стандартных окон Qt.
void addBuiltins()
{
    static const char *const pairs[][2] = {
        {"OK", "ОК"}, {"Cancel", "Отмена"}, {"&Yes", "&Да"}, {"&No", "&Нет"}, {"Yes", "Да"}, {"No", "Нет"},
        {"Yes to &All", "Да для &всех"}, {"N&o to All", "Н&ет для всех"},
        {"&OK", "&ОК"}, {"&Cancel", "&Отмена"}, {"Apply", "Применить"}, {"&Apply", "&Применить"},
        {"Close", "Закрыть"}, {"&Close", "&Закрыть"}, {"Save", "Сохранить"}, {"&Save", "&Сохранить"},
        {"Open", "Открыть"}, {"&Open", "&Открыть"}, {"Help", "Справка"}, {"Abort", "Прервать"},
        {"Retry", "Повторить"}, {"Ignore", "Пропустить"}, {"Reset", "Сбросить"},
        {"Restore Defaults", "По умолчанию"}, {"Discard", "Не сохранять"}, {"Don't Save", "Не сохранять"},
        {"Show Details...", "Подробнее..."}, {"Hide Details...", "Скрыть подробности..."},
        {"&Restore", "&Восстановить"}, {"&Move", "&Переместить"}, {"&Size", "&Размер"},
        {"Mi&nimize", "&Свернуть"}, {"Ma&ximize", "Р&азвернуть"}, {"Stay on &Top", "Поверх &всех окон"},
        {"&Close\tCtrl+F4", "&Закрыть\tCtrl+F4"},
    };
    for (const auto &p : pairs)
        addPair(QString::fromUtf8(p[0]), QString::fromUtf8(p[1]));
}

/// @brief Читает `:/tmci18n/ui_strings.tsv` (две колонки через табуляцию: английский и русский; после строки `#TEMPLATES` — шаблоны).
void loadDictionary()
{
    if (g_dict.loaded)
        return;
    g_dict.loaded = true;
    QFile f(QStringLiteral(":/tmci18n/ui_strings.tsv"));
    if (f.open(QIODevice::ReadOnly)) {
        bool templates = false;
        const QStringList lines = QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'));
        for (QString line : lines) {
            if (line.endsWith(QLatin1Char('\r')))
                line.chop(1);
            if (line.startsWith(QLatin1String("#TEMPLATES"))) {
                templates = true;
                continue;
            }
            if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
                continue;
            const QStringList col = line.split(QLatin1Char('\t'));
            if (col.size() < 2)
                continue;
            const QString en = unescape(col.at(0)), ru = unescape(col.at(1));
            if (en.isEmpty() || ru.isEmpty())
                continue;
            if (templates && placeholderRe().match(en).hasMatch()) {
                g_dict.enRuTpl.append(makeTemplate(en, ru));
                g_dict.ruEnTpl.append(makeTemplate(ru, en));
            } else {
                addPair(en, ru);
            }
        }
    }
    addBuiltins();
}

/// @brief Ищет перевод точной строки, затем строки без «&».
/// @param[in] text  строка
/// @return Перевод или пустая строка.
QString lookup(const QString &text)
{
    const auto &exact = g_russian ? g_dict.enRu : g_dict.ruEn;
    auto it = exact.constFind(text);
    if (it != exact.constEnd())
        return *it;
    const auto &bare = g_russian ? g_dict.enRuBare : g_dict.ruEnBare;
    const QString key = stripAmp(text);
    it = bare.constFind(key);
    if (it != bare.constEnd())
        return key == text ? stripAmp(*it) : *it;   // в исходной строке не было «&» — без него
    return QString();
}

/// @brief Переводит строку с сохранением пробелов по краям (см. `translate`).
/// @param[in] text  строка
/// @return Перевод или исходная строка.
QString translateOne(const QString &text)
{
    if (text.isEmpty() || !g_installed)
        return text;
    // Отступы по краям сохраняем.
    int a = 0, b = text.size();
    while (a < b && text.at(a).isSpace())
        ++a;
    while (b > a && text.at(b - 1).isSpace())
        --b;
    const QString core = text.mid(a, b - a);
    if (core.isEmpty())
        return text;
    QString tr = lookup(core);
    if (tr.isEmpty()) {
        // «Пункт\tCtrl+N» — переводим пункт, сочетание клавиш оставляем.
        const int tab = core.indexOf(QLatin1Char('\t'));
        if (tab > 0) {
            const QString head = lookup(core.left(tab));
            if (!head.isEmpty())
                tr = head + core.mid(tab);
        }
    }
    if (tr.isEmpty()) {
        const auto &tpl = g_russian ? g_dict.enRuTpl : g_dict.ruEnTpl;
        for (const Template &t : tpl) {
            const auto m = t.re.match(core);
            if (m.hasMatch()) {
                tr = fillTemplate(t, m);
                break;
            }
        }
    }
    if (tr.isEmpty())
        return text;
    return text.left(a) + tr + text.mid(b);
}

/// @brief Возвращает true, если у объекта стоит свойство «не переводить».
/// @param[in] o  объект или NULL
/// @return true — свойство `tmcNoTranslate` истинно; false — нет или `o == NULL`.
bool skip(const QObject *o) { return o && o->property(kNoTranslate).toBool(); }

/// @brief Переводит текст, подсказки пункта и (рекурсивно) его подменю.
/// @param[in,out] act  пункт; NULL — ничего
void translateAction(QAction *act)
{
    if (!act || skip(act) || act->isSeparator())
        return;
    // Своя пара у пункта, чей английский текст совпадает с другим (Edit → Edit).
    const QVariant own = act->property(g_russian ? "tmcRu" : "tmcEn");
    if (own.isValid()) {
        act->setText(own.toString());
        // Подсказка у такого пункта своя («Extern edit») — её тоже переводим.
        const QString ownTip = act->toolTip();
        const QString ownTipTr = translateOne(ownTip);
        if (!ownTip.isEmpty() && ownTipTr != ownTip)
            act->setToolTip(ownTipTr);
        return;
    }
    const QString t = act->text();
    const QString nt = translateOne(t);
    if (nt != t)
        act->setText(nt);
    const QString tip = act->toolTip();
    if (!tip.isEmpty() && tip != stripAmp(nt)) {
        const QString ntip = translateOne(tip);
        if (ntip != tip)
            act->setToolTip(ntip);
    }
    const QString st = act->statusTip();
    const QString nst = translateOne(st);
    if (nst != st)
        act->setStatusTip(nst);
    if (QMenu *sub = act->menu()) {
        const QString title = sub->title();
        const QString ntitle = translateOne(title);
        if (ntitle != title)
            sub->setTitle(ntitle);
        if (!skip(sub))
            for (QAction *a : sub->actions())
                translateAction(a);
    }
}

/// @brief Переводит тексты одного виджета (заголовок окна, подсказки, подписи, кнопки, вкладки, заголовки колонок) и его действия.
/// @param[in,out] w  виджет (не NULL)
void translateOneWidget(QWidget *w)
{
    if (skip(w))
        return;
    if (w->isWindow() || qobject_cast<QDockWidget *>(w) || qobject_cast<QMdiSubWindow *>(w)) {
        const QString t = w->windowTitle();
        const QString nt = translateOne(t);
        if (nt != t)
            w->setWindowTitle(nt);
    }
    const QString tip = w->toolTip();
    const QString ntip = translateOne(tip);
    if (ntip != tip)
        w->setToolTip(ntip);
    const QString st = w->statusTip();
    const QString nst = translateOne(st);
    if (nst != st)
        w->setStatusTip(nst);

    if (auto *l = qobject_cast<QLabel *>(w)) {
        const QString s = l->text(), ns = translateOne(s);
        if (ns != s)
            l->setText(ns);
    } else if (auto *btn = qobject_cast<QAbstractButton *>(w)) {
        const QString s = btn->text(), ns = translateOne(s);
        if (ns != s)
            btn->setText(ns);
    } else if (auto *g = qobject_cast<QGroupBox *>(w)) {
        const QString s = g->title(), ns = translateOne(s);
        if (ns != s)
            g->setTitle(ns);
    } else if (auto *c = qobject_cast<QComboBox *>(w)) {
        for (int i = 0; i < c->count(); ++i) {
            const QString s = c->itemText(i), ns = translateOne(s);
            if (ns != s)
                c->setItemText(i, ns);
        }
    } else if (auto *tb = qobject_cast<QTabBar *>(w)) {
        for (int i = 0; i < tb->count(); ++i) {
            const QString s = tb->tabText(i), ns = translateOne(s);
            if (ns != s)
                tb->setTabText(i, ns);
        }
    } else if (auto *e = qobject_cast<QLineEdit *>(w)) {
        const QString s = e->placeholderText(), ns = translateOne(s);
        if (ns != s)
            e->setPlaceholderText(ns);
    } else if (auto *tw = qobject_cast<QTreeWidget *>(w)) {
        if (QTreeWidgetItem *h = tw->headerItem())
            for (int i = 0; i < h->columnCount(); ++i) {
                const QString s = h->text(i), ns = translateOne(s);
                if (ns != s)
                    h->setText(i, ns);
            }
    } else if (auto *tbl = qobject_cast<QTableWidget *>(w)) {
        for (int i = 0; i < tbl->columnCount(); ++i)
            if (QTableWidgetItem *h = tbl->horizontalHeaderItem(i)) {
                const QString s = h->text(), ns = translateOne(s);
                if (ns != s)
                    h->setText(ns);
            }
    }
    if (auto *m = qobject_cast<QMenu *>(w)) {
        const QString s = m->title(), ns = translateOne(s);
        if (ns != s)
            m->setTitle(ns);
    }
    for (QAction *a : w->actions())
        translateAction(a);
}

/// @brief Фильтр приложения: при `Show` окна, вложенного окна MDI или панели переводит его целиком.
class Filter : public QObject {
public:
    using QObject::QObject;
    /// @brief Переводит окно при показе.
    /// @param[in] o  объект события
    /// @param[in] e  событие
    /// @return Всегда false (событие не поглощается).
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() == QEvent::Show && o->isWidgetType()) {
            QWidget *w = static_cast<QWidget *>(o);
            if (w->isWindow() || qobject_cast<QMdiSubWindow *>(w) || qobject_cast<QDockWidget *>(w))
                translateWidget(w);
        }
        return false;
    }
};

/// @brief Разбирает значение настройки: `ru`, `en`, иначе `System`.
/// @param[in] v  значение `[UI] Language` (регистр не важен)
/// @return `Lang::Ru`, `Lang::En` или `Lang::System` для любого другого значения.
Lang parse(const QString &v)
{
    if (v.compare(QLatin1String("ru"), Qt::CaseInsensitive) == 0)
        return Lang::Ru;
    if (v.compare(QLatin1String("en"), Qt::CaseInsensitive) == 0)
        return Lang::En;
    return Lang::System;
}

/// @brief Определяет действующий язык: `System` — русский, если язык системы русский.
/// @param[in] l  выбор языка
/// @return true — интерфейс на русском; false — на английском.
bool effectiveRussian(Lang l)
{
    if (l == Lang::Ru)
        return true;
    if (l == Lang::En)
        return false;
    return QLocale::system().language() == QLocale::Russian;
}

} // namespace

Lang setting()
{
    QSettings ini(settingsPath(), QSettings::IniFormat);
    return parse(ini.value(QStringLiteral("UI/Language"), QStringLiteral("system")).toString());
}

bool russian() { return g_russian; }

QString tr(const char *ru, const char *en)
{
    return QString::fromUtf8(g_russian ? ru : en);
}

QString translate(const QString &text) { return translateOne(text); }

void setOwnText(QAction *act, const char *ru, const char *en)
{
    act->setProperty("tmcRu", QString::fromUtf8(ru));
    act->setProperty("tmcEn", QString::fromUtf8(en));
    act->setText(QString::fromUtf8(g_russian ? ru : en));
}

void translateWidget(QWidget *w)
{
    if (!w || !g_installed)
        return;
    translateOneWidget(w);
    for (QWidget *c : w->findChildren<QWidget *>())
        translateOneWidget(c);
}

// Стандартные окна Qt (выбор шрифта, файла, цвета, кнопки «OK/Cancel» в
// сообщениях) переводит сам Qt своим файлом qtbase_ru.qm. Ищем его рядом с
// программой (translations\ в поставке), затем в установке Qt.
namespace {
/// Перевод стандартных окон Qt (`qtbase_ru.qm`) или NULL.
QTranslator *g_qtTranslator = nullptr;

/// @brief Ставит или снимает перевод Qt: ищет `qtbase_ru.qm` в `translations` рядом с программой, затем в установке Qt.
/// @param[in] ru  true — русский
void applyQtTranslator(bool ru)
{
    if (g_qtTranslator) {
        QCoreApplication::removeTranslator(g_qtTranslator);
        delete g_qtTranslator;
        g_qtTranslator = nullptr;
    }
    if (!ru)
        return;
    QTranslator *t = new QTranslator(qApp);
    const QStringList dirs = { QCoreApplication::applicationDirPath() + QStringLiteral("/translations"),
                               QLibraryInfo::path(QLibraryInfo::TranslationsPath) };
    for (const QString &d : dirs) {
        if (t->load(QStringLiteral("qtbase_ru"), d)) {
            QCoreApplication::installTranslator(t);
            g_qtTranslator = t;
            return;
        }
    }
    delete t;
}
} // namespace

void setSetting(Lang lang)
{
    {
        QSettings ini(settingsPath(), QSettings::IniFormat);
        ini.setValue(QStringLiteral("UI/Language"),
                     lang == Lang::Ru ? QStringLiteral("ru") : lang == Lang::En ? QStringLiteral("en")
                                                                                 : QStringLiteral("system"));
    }
    const bool ru = effectiveRussian(lang);
    if (ru == g_russian)
        return;
    g_russian = ru;
    applyQtTranslator(ru);
    // Открытые окна и меню — сразу на новом языке (словарь работает в обе стороны).
    for (QWidget *w : QApplication::allWidgets())
        translateOneWidget(w);
}

void install()
{
    if (g_installed || !qApp)
        return;
    loadDictionary();
    g_russian = effectiveRussian(setting());
    g_installed = true;
    applyQtTranslator(g_russian);
    qApp->installEventFilter(new Filter(qApp));
}

QMenu *addLanguageMenu(QMenu *parent)
{
    // Подпись на двух языках, чтобы пункт нашёлся при любом языке интерфейса.
    QMenu *m = parent->addMenu(QStringLiteral("Language / Язык"));
    m->setProperty(kNoTranslate, true);
    m->menuAction()->setProperty(kNoTranslate, true);
    auto *group = new QActionGroup(m);
    const struct { Lang lang; const char *name; } items[] = {
        {Lang::System, "System / Как в системе"}, {Lang::Ru, "Русский"}, {Lang::En, "English"}};
    const Lang cur = setting();
    for (const auto &it : items) {
        QAction *a = m->addAction(QString::fromUtf8(it.name));
        a->setProperty(kNoTranslate, true);
        a->setCheckable(true);
        a->setChecked(it.lang == cur);
        group->addAction(a);
        const Lang lang = it.lang;
        QObject::connect(a, &QAction::triggered, m, [lang] { setSetting(lang); });
    }
    return m;
}

} // namespace tmci18n
