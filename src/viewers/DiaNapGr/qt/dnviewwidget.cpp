/// @file dnviewwidget.cpp
/// @brief Реализация `DnViewWidget`: передача перерисовки и окон сообщений из потока расчёта в главный поток.
///
/// Часть модуля «Вьювер TMC_DN». Описание методов — в `dnviewwidget.h`.
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#thread
#include "dnviewwidget.h"

#include <QMetaObject>
#include <QString>
#include <QThread>

DnViewWidget::DnViewWidget(QWidget *parent)
    : MfcViewWidget(parent)
{
}

bool DnViewWidget::inGuiThread() const
{
    return QThread::currentThread() == thread();
}

void DnViewWidget::hostInvalidate(const RECT *logicalRect, BOOL erase)
{
    if (inGuiThread()) {
        MfcViewWidget::hostInvalidate(logicalRect, erase);
        return;
    }
    // Из потока расчёта область не разбираем: базовый обработчик и так
    // перерисовывает окно целиком.
    QMetaObject::invokeMethod(this, [this]() {
        MfcViewWidget::hostInvalidate(nullptr, TRUE);
    }, Qt::QueuedConnection);
}

void DnViewWidget::hostUpdateWindow()
{
    if (inGuiThread()) {
        MfcViewWidget::hostUpdateWindow();
        return;
    }
    // Здесь и падало: RedrawWindow() в конце ReadDocFileW_Thread приводил к
    // QWidget::repaint() из потока расчёта. Немедленная перерисовка из чужого
    // потока невозможна в принципе, поэтому просим главный поток обновиться,
    // когда он до этого дойдёт.
    QMetaObject::invokeMethod(this, [this]() {
        MfcViewWidget::hostInvalidate(nullptr, TRUE);
    }, Qt::QueuedConnection);
}

int DnViewWidget::hostMessageBox(const char *text, const char *caption, UINT type)
{
    if (inGuiThread())
        return MfcViewWidget::hostMessageBox(text, caption, type);

    // Модальное окно из потока расчёта повесило бы программу. Показываем его в
    // главном потоке, а поток расчёта не ждёт ответа (в Windows-версии он на
    // ответ и не смотрит: сообщение об ошибке чтения данных).
    const QString t = QString::fromLocal8Bit(text ? text : "");
    const QString c = QString::fromLocal8Bit(caption ? caption : "");
    QMetaObject::invokeMethod(this, [this, t, c]() {
        MfcViewWidget::hostMessageBox(t.toLocal8Bit().constData(),
                                      c.toLocal8Bit().constData(), 0);
    }, Qt::QueuedConnection);
    return 1;
}
