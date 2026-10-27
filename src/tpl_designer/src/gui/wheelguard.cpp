/// @file wheelguard.cpp
/// @brief Реализация сторожа колеса мыши `WheelGuard`.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-wheel
#include "wheelguard.h"

#include <QAbstractSlider>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QScrollBar>
#include <QWidget>

namespace {

/// Поле, у которого колесо меняет значение. Полосы прокрутки — не такие поля: колесо
/// у них и есть прокрутка.
/// @brief Сообщает, меняет ли объект значение от колеса мыши.
/// @param[in] o  проверяемый объект.
/// @return true — счётчик (`QAbstractSpinBox`), выпадающий список (`QComboBox`) или ползунок
///         (`QAbstractSlider`, кроме `QScrollBar`).
bool changesValueOnWheel(QObject *o)
{
    if (qobject_cast<QScrollBar *>(o))
        return false;
    return qobject_cast<QAbstractSpinBox *>(o) || qobject_cast<QComboBox *>(o)
           || qobject_cast<QAbstractSlider *>(o);
}

} // namespace

WheelGuard::WheelGuard(QObject *parent)
    : QObject(parent)
{
}

void WheelGuard::install()
{
    static WheelGuard *guard = nullptr;
    if (guard || !qApp)
        return;
    guard = new WheelGuard(qApp);
    qApp->installEventFilter(guard);
}

bool WheelGuard::eventFilter(QObject *watched, QEvent *event)
{
    if (!changesValueOnWheel(watched))
        return QObject::eventFilter(watched, event);

    auto *w = static_cast<QWidget *>(watched);
    if (event->type() == QEvent::Polish) {
        // По умолчанию у счётчика политика WheelFocus: колесо отдаёт ему фокус раньше,
        // чем событие дойдёт до фильтра, и проверка «в фокусе ли поле» была бы
        // бесполезной. Фокус — только щелчком и клавишей Tab.
        if (w->focusPolicy() == Qt::WheelFocus)
            w->setFocusPolicy(Qt::StrongFocus);
    } else if (event->type() == QEvent::Wheel && !w->hasFocus()) {
        // Не принятое событие QApplication передаёт родителю — то есть области
        // прокрутки страницы. Поле значения не меняет, страница листается.
        event->ignore();
        return true;
    }
    return QObject::eventFilter(watched, event);
}
