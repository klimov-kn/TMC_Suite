/// @file mousewatchdog.cpp
/// @brief Реализация сторожа захвата мыши.
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_gui_common`.
/// @see https://www.tamic.ru/docs/code/gui-common.html#mouse-watchdog
#include "mousewatchdog.h"

#include <QApplication>
#include <QTimer>
#include <QWidget>

namespace tmcui {

void installMouseGrabWatchdog(QObject *owner)
{
    if (!owner)
        return;

    QTimer *timer = new QTimer(owner);
    timer->setInterval(500);
    QObject::connect(timer, &QTimer::timeout, owner, []() {
        QWidget *grabber = QWidget::mouseGrabber();
        if (!grabber)
            return;
        // Кнопки отпущены, а захват остался — значит, он «завис».
        if (QApplication::mouseButtons() == Qt::NoButton)
            grabber->releaseMouse();
    });
    timer->start();
}

} // namespace tmcui
