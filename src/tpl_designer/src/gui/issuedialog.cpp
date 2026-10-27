/// @file issuedialog.cpp
/// @brief Реализация окна замечаний `IssueDialog` и функции `collapseIssues`.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-dialogs
#include "issuedialog.h"

#include "dtheme.h"

#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QScreen>
#include <QTextBrowser>
#include <QVBoxLayout>
#include "lang.h"

namespace {

/// Замечание без ссылки на строку файла — по нему и сравниваем повторы.
/// Префикс «строка N:» (русский) или «line N:» (английский) — снимаем любой.
/// @brief Убирает из замечания префикс номера строки файла.
/// @param[in] s  текст замечания.
/// @return Текст без начального «строка N:» / «line N:».
QString withoutLine(const QString &s)
{
    static const QRegularExpression re(QStringLiteral("^(?:строка|line)\\s+\\d+:\\s*"));
    return QString(s).remove(re);
}

/// @brief Экранирует текст для вставки в HTML.
/// @param[in] s  исходный текст.
/// @return Текст с заменой `<`, `>`, `&`, `"` на сущности HTML.
QString escape(const QString &s)
{
    return s.toHtmlEscaped();
}

} // namespace

QStringList collapseIssues(const QStringList &items)
{
    QStringList out;
    QHash<QString, int> at;    ///< текст без «строка N» -> место в out
    QVector<int> count;

    for (const QString &s : items) {
        const QString key = withoutLine(s);
        const auto it = at.constFind(key);
        if (it == at.constEnd()) {
            at.insert(key, out.size());
            count << 1;
            out << s;
        } else {
            ++count[*it];
        }
    }

    for (int i = 0; i < out.size(); ++i) {
        if (count.at(i) > 1) {
            out[i] += tpl::tr(" — и ещё %1 раз(а) то же самое дальше по файлу", " — and %1 more time(s) the same further in the file")
                          .arg(count.at(i) - 1);
        }
    }
    return out;
}

IssueDialog::IssueDialog(QWidget *parent)
    : QDialog(parent)
{
    setSizeGripEnabled(true);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(16, 16, 16, 12);
    lay->setSpacing(10);

    m_body = new QTextBrowser(this);
    m_body->setOpenExternalLinks(false);
    m_body->setFrameShape(QFrame::NoFrame);
    lay->addWidget(m_body, 1);

    resize(720, 460);
}

void IssueDialog::setLead(const QString &text)
{
    m_lead = text;
}

void IssueDialog::setIssues(const QStringList &errors, const QStringList &warnings)
{
    const auto block = [](const QString &title, const QColor &c, const QStringList &items) {
        if (items.isEmpty())
            return QString();
        const QStringList list = collapseIssues(items);
        QString html = QStringLiteral("<p style='color:%1;margin:14px 0 4px 0'><b>%2 (%3)</b></p><ul>")
                           .arg(c.name(), title)
                           .arg(items.size());
        for (const QString &s : list)
            html += QStringLiteral("<li style='margin-bottom:5px'>%1</li>").arg(escape(s));
        return html + QStringLiteral("</ul>");
    };

    QString html = QStringLiteral("<div style='color:%1'>").arg(dtheme::text().name());
    if (!m_lead.isEmpty()) {
        html += QStringLiteral("<p style='margin:0'>%1</p>")
                    .arg(escape(m_lead).replace(QLatin1Char('\n'), QStringLiteral("<br>")));
    }
    html += block(tpl::tr("Ошибки", "Errors"), dtheme::danger(), errors);
    html += block(tpl::tr("Предупреждения", "Warnings"), dtheme::warn(), warnings);
    html += QStringLiteral("</div>");
    m_body->setHtml(html);
}

void IssueDialog::addButtons(const QString &confirmText)
{
    auto *buttons = new QDialogButtonBox(this);
    if (confirmText.isEmpty()) {
        buttons->addButton(tpl::tr("Закрыть", "Close"), QDialogButtonBox::AcceptRole);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    } else {
        QPushButton *go = buttons->addButton(confirmText, QDialogButtonBox::AcceptRole);
        buttons->addButton(tpl::tr("Отмена", "Cancel"), QDialogButtonBox::RejectRole);
        connect(go, &QPushButton::clicked, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }
    // Кнопки кладём последними и ВНЕ прокрутки: до них должно быть можно добраться
    // при любом объёме текста.
    layout()->addWidget(buttons);
}

bool IssueDialog::report(QWidget *parent, const QString &title, const QString &lead,
                         const QStringList &errors, const QStringList &warnings,
                         const QString &confirmText)
{
    IssueDialog dlg(parent);
    dlg.setWindowTitle(title);
    dlg.setLead(lead);
    dlg.setIssues(errors, warnings);
    dlg.addButtons(confirmText);

    // Ужимаем под экран ДО показа: окно, вылезшее за край, мышью уже не поймать.
    dtheme::fitToScreen(&dlg);
    return dlg.exec() == QDialog::Accepted;
}
