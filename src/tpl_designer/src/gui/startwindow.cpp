/// @file startwindow.cpp
/// @brief Реализация стартового окна `StartWindow`.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-windows
#include "startwindow.h"

#include "dtheme.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "lang.h"

namespace {

/// Карточка действия: заголовок, описание, кнопка.
/// @brief Создаёт карточку стартового окна (рамка со свойством `card`).
/// @param[in]  title    заголовок карточки.
/// @param[in]  text     пояснение под заголовком.
/// @param[in]  button   надпись на кнопке.
/// @param[in]  enabled  true — кнопка доступна и получает акцентный вид.
/// @param[out] out      сюда кладётся указатель на кнопку; `nullptr` — не нужен.
/// @return Новая рамка без родителя; владельцем становится разметка, в которую её добавят.
QFrame *card(const QString &title, const QString &text, const QString &button,
             bool enabled, QPushButton **out)
{
    auto *f = new QFrame;
    f->setProperty("card", true);
    f->setMinimumWidth(260);
    auto *l = new QVBoxLayout(f);
    l->setContentsMargins(18, 18, 18, 18);
    l->setSpacing(10);

    auto *t = new QLabel(title);
    t->setProperty("h2", true);
    l->addWidget(t);

    auto *d = new QLabel(text);
    d->setProperty("mute", true);
    d->setWordWrap(true);
    d->setMinimumHeight(64);
    d->setAlignment(Qt::AlignTop);
    l->addWidget(d, 1);

    auto *b = new QPushButton(button);
    b->setEnabled(enabled);
    b->setMinimumHeight(34);
    if (enabled)
        b->setProperty("accent", true);
    l->addWidget(b);
    if (out)
        *out = b;
    return f;
}

} // namespace

StartWindow::StartWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("TPL Designer"));

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(40, 36, 40, 36);
    root->setSpacing(6);

    auto *title = new QLabel(QStringLiteral("TPL Designer"));
    title->setProperty("h1", true);
    root->addWidget(title);

    auto *sub = new QLabel(tpl::tr("Построение файла задания для счётных ядер TMC Suite. "
        "Рисуете геометрию и физику — синтаксис пишет программа.",
        "Builds task files for the TMC Suite computational kernels. You draw the "
        "geometry and physics — the program writes the syntax."));
    sub->setProperty("dim", true);
    sub->setWordWrap(true);
    root->addWidget(sub);
    root->addSpacing(24);

    auto *cards = new QHBoxLayout;
    cards->setSpacing(16);

    QPushButton *create = nullptr;
    cards->addWidget(card(tpl::tr("Создать задание", "Create a task"),
                          tpl::tr("Пошагово: расчёт, геометрия, вывод данных, проверка. "
                                  "Готовый .tpl в конце.",
                                  "Step by step: run, geometry, data output, check. A ready .tpl at the end."),
                          tpl::tr("Создать", "Create"), true, &create));

    QPushButton *open = nullptr;
    cards->addWidget(card(tpl::tr("Открыть готовое", "Open existing"),
                          tpl::tr("Загрузить существующий .tpl — геометрия и все "
                                  "параметры лягут в те же вкладки, и их можно править.",
                                  "Load an existing .tpl — the geometry and all parameters go into the same "
                                  "tabs, and you can edit them."),
                          tpl::tr("Открыть", "Open"), true, &open));

    QPushButton *samples = nullptr;
    cards->addWidget(card(tpl::tr("Взять образец", "Take a sample"),
                          tpl::tr("Библиотека готовых устройств по стандартам: волноводы и "
                                  "узлы, антенны, микрополоска, среды, плазма, ЭПР.",
                                  "A library of ready devices to standards: waveguides and components, antennas, "
                                  "microstrip, media, plasma, RCS."),
                          tpl::tr("Открыть библиотеку", "Open library"), true, &samples));

    // Четвёртая карточка — не задание, а вспомогательные файлы к нему. Делать их в
    // блокноте нельзя: у .sig шаг привязан к шагу сетки, а .eps вообще двоичный.
    QPushButton *files = nullptr;
    cards->addWidget(card(tpl::tr("Вспомогательные файлы", "Auxiliary files"),
                          tpl::tr("Файл падающей волны .sig и файл распределения eps "
                                  "— с предпросмотром формы и карты среды.",
                                  "Incident-wave file .sig and eps distribution file — with a preview of the "
                                  "waveform and the medium map."),
                          tpl::tr("Сделать файл", "Make a file"), true, &files));
    root->addLayout(cards);
    root->addStretch();

    auto *foot = new QLabel(tpl::tr("Ядра TMC Suite — научная разработка К. Н. Климова. "
        "Генератор пишет для них вход и математику расчёта не трогает.",
        "The TMC Suite kernels are a scientific development by K. N. Klimov. The "
        "generator writes their input and does not touch the computational mathematics."));
    foot->setProperty("mute", true);
    foot->setWordWrap(true);
    root->addWidget(foot);

    connect(create, &QPushButton::clicked, this, &StartWindow::createRequested);
    connect(open, &QPushButton::clicked, this, &StartWindow::openRequested);
    connect(samples, &QPushButton::clicked, this, &StartWindow::samplesRequested);
    connect(files, &QPushButton::clicked, this, &StartWindow::filesRequested);
}
