/// @file mfcviewwidget.cpp
/// @brief Реализация виджета-хозяина вида MFC.
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_gui_common`.
/// @see https://www.tamic.ru/docs/code/gui-common.html#view-widget
#include "mfcviewwidget.h"

#include <QMessageBox>
#include <QMetaObject>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QElapsedTimer>
#include <QScrollBar>
#include <QThread>
#include <QTimerEvent>
#include <QTimer>
#include <QMutexLocker>

MfcViewWidget::MfcViewWidget(QWidget *parent)
    : QAbstractScrollArea(parent)
{
    viewport()->setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

MfcViewWidget::~MfcViewWidget()
{
    if (m_view)
        m_view->SetHost(nullptr);
}

void MfcViewWidget::setView(CScrollView *view)
{
    m_view = view;
    if (m_view)
        m_view->SetHost(this);
}

void MfcViewWidget::initialUpdate()
{
    if (m_view && !m_initialDone) {
        m_initialDone = true;
        m_view->OnInitialUpdate();
        viewport()->update();
    }
}

void MfcViewWidget::setDocumentBackground(const QColor &color)
{
    m_background = color;
#ifdef TMC_QT_WIN
    // Палитра проекта: белый и чёрный фон графика (значения Windows-версии по
    // умолчанию) — тёмный фон bg; цвет, выбранный пользователем, — как есть.
    if (color == QColor(Qt::white) || color == QColor(Qt::black))
        m_background = QColor(0x0a, 0x0e, 0x16);
    else if (color == QColor(192, 192, 192))
        m_background = QColor(0x13, 0x1a, 0x28);
#endif
    viewport()->update();
}

void MfcViewWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    painter.fillRect(event->rect(), m_background);
    if (!m_view)
        return;

    CDC dc;
    dc.Attach(&painter, viewport());
    // Тот же порядок, что в MFC: сначала контекст настраивается под режим
    // отображения и прокрутку, потом вид рисует себя.
    //
    // Код вида местами просит окно перерисоваться прямо во время рисования
    // (в MFC это было безобидно). В Qt перерисовка внутри перерисовки — ошибка,
    // поэтому на время OnDraw такие просьбы откладываются: см. m_painting.
    m_painting = true;
    m_view->OnPrepareDC(&dc, nullptr);
    m_view->OnDraw(&dc);
    m_painting = false;
}

UINT MfcViewWidget::mouseFlags(QMouseEvent *event) const
{
    UINT flags = 0;
    if (event->buttons() & Qt::LeftButton)   flags |= MK_LBUTTON;
    if (event->buttons() & Qt::RightButton)  flags |= MK_RBUTTON;
    if (event->buttons() & Qt::MiddleButton) flags |= MK_MBUTTON;
    if (event->modifiers() & Qt::ShiftModifier)   flags |= MK_SHIFT;
    if (event->modifiers() & Qt::ControlModifier) flags |= MK_CONTROL;
    return flags;
}

CPoint MfcViewWidget::devicePoint(QMouseEvent *event) const
{
    // Вид получает координаты в пикселях области просмотра — как в Windows,
    // где обработчики мыши работают в координатах клиента, а перевод в
    // логические делает сам вид через DPtoLP.
    const QPoint p = event->position().toPoint();
    return CPoint(p.x() + horizontalScrollBar()->value(),
                  p.y() + verticalScrollBar()->value());
}

void MfcViewWidget::mousePressEvent(QMouseEvent *event)
{
    if (!m_view) {
        QAbstractScrollArea::mousePressEvent(event);
        return;
    }
    if (event->button() == Qt::LeftButton)
        m_view->OnLButtonDown(mouseFlags(event), devicePoint(event));
    else if (event->button() == Qt::RightButton)
        m_view->OnRButtonDown(mouseFlags(event), devicePoint(event));
    viewport()->update();
}

void MfcViewWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // Дочернее окно многодокументного режима могло захватить мышь для
    // перетаскивания и не получить отпускание. Снимаем такой захват сразу:
    // иначе все дальнейшие щелчки уйдут ему, и программа перестанет отвечать.
    if (QWidget *grabber = QWidget::mouseGrabber()) {
        if (event->buttons() == Qt::NoButton)
            grabber->releaseMouse();
    }

    if (m_view && event->button() == Qt::LeftButton)
        m_view->OnLButtonUp(mouseFlags(event), devicePoint(event));
    viewport()->update();
}

void MfcViewWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (m_view && event->button() == Qt::LeftButton)
        m_view->OnLButtonDblClk(mouseFlags(event), devicePoint(event));
    viewport()->update();
}

void MfcViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_view)
        m_view->OnMouseMove(mouseFlags(event), devicePoint(event));
}

void MfcViewWidget::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
    viewport()->update();
}

void MfcViewWidget::timerEvent(QTimerEvent *event)
{
    QMap<int, UINT>::const_iterator it = m_timers.find(event->timerId());
    if (it != m_timers.end() && m_view)
        m_view->OnTimer(it.value());
}

// --- MfcQtHost ----------------------------------------------------------------

bool MfcViewWidget::inGuiThread() const
{
    // Виджет живёт в главном потоке; сравнение с ним и отвечает на вопрос,
    // можно ли трогать окно прямо сейчас.
    return QThread::currentThread() == thread();
}

void MfcViewWidget::hostInvalidate(const RECT * /*logicalRect*/, BOOL /*erase*/)
{
    // Область в логических единицах пересчитывать не станем: перерисовка всего
    // окна на современной машине дешевле, чем ошибка в границах области.
    if (!inGuiThread()) {
        // Вызов из расчётного потока: в Qt трогать виджет оттуда нельзя,
        // поэтому просьба перерисоваться кладётся в очередь главного потока.
        QMetaObject::invokeMethod(this, [this]() { viewport()->update(); },
                                  Qt::QueuedConnection);
        return;
    }
    viewport()->update();
}

void MfcViewWidget::hostUpdateWindow()
{
    if (!inGuiThread()) {
        // Немедленной перерисовки из чужого потока не бывает: именно здесь
        // программа и падала (RedrawWindow() в конце расчёта TMC_DN уводил в
        // QPainter::begin прямо из потока расчёта). Откладываем на главный
        // поток — он перерисует, когда дойдёт до очереди событий.
        QMetaObject::invokeMethod(this, [this]() { viewport()->update(); },
                                  Qt::QueuedConnection);
        return;
    }
    // Немедленная перерисовка допустима только вне самой перерисовки.
    if (m_painting)
        viewport()->update();
    else
        viewport()->repaint();
}

void MfcViewWidget::hostGetClientRect(RECT *deviceRect) const
{
    if (!deviceRect)
        return;
    const QSize s = viewport()->size();
    deviceRect->left = 0;
    deviceRect->top = 0;
    deviceRect->right = s.width();
    deviceRect->bottom = s.height();
}

void MfcViewWidget::hostSetCapture()
{
    // Захват мыши НЕ делаем намеренно.
    //
    // В Windows SetCapture нужен, чтобы получать движения мыши, когда указатель
    // ушёл за пределы окна при зажатой кнопке. В Qt это происходит само:
    // с момента нажатия и до отпускания все события идут тому же виджету.
    //
    // А вот последствия захвата тяжёлые: если отпускание кнопки почему-то не
    // дошло до вида (окно потеряло фокус, поверх открылось модальное окно,
    // сменился размер), захват остаётся, и ВСЕ щелчки уходят в график —
    // панель инструментов и меню перестают отвечать, программа выглядит
    // зависшей. Ради поведения, которое Qt и так обеспечивает, это слишком
    // дорого.
}

void MfcViewWidget::hostReleaseCapture()
{
    // Захвата нет — снимать нечего. На всякий случай отпускаем, если захват
    // всё же откуда-то появился.
    if (mouseGrabber() == this)
        releaseMouse();
}

UINT MfcViewWidget::hostSetTimer(UINT id, UINT elapseMs)
{
    const int qtId = startTimer(int(elapseMs));
    m_timers.insert(qtId, id);
    return id;
}

BOOL MfcViewWidget::hostKillTimer(UINT id)
{
    for (QMap<int, UINT>::iterator it = m_timers.begin(); it != m_timers.end(); ++it) {
        if (it.value() == id) {
            killTimer(it.key());
            m_timers.erase(it);
            return TRUE;
        }
    }
    return FALSE;
}

int MfcViewWidget::hostMessageBox(const char *text, const char *caption, UINT /*type*/)
{
    const QString t = text ? QString::fromLocal8Bit(text) : QString();
    const QString c = caption ? QString::fromLocal8Bit(caption) : windowTitle();

    if (!inGuiThread()) {
        // Модальное окно из расчётного потока повесило бы программу. Показываем
        // его в главном потоке, а расчёт продолжается: в Windows-версии на
        // ответ здесь и не смотрят (это сообщение об ошибке чтения данных).
        QMetaObject::invokeMethod(this, [this, t, c]() {
            QMessageBox::information(this, c, t);
        }, Qt::QueuedConnection);
        return 1;
    }

    QMessageBox::information(this, c, t);
    return 1;
}

void MfcViewWidget::hostSetScrollSizes(const SIZE &total, const SIZE &page, const SIZE &line)
{
    const QSize vp = viewport()->size();
    const int maxX = qMax(0, int(qAbs(total.cx)) - vp.width());
    const int maxY = qMax(0, int(qAbs(total.cy)) - vp.height());

    horizontalScrollBar()->setRange(0, maxX);
    verticalScrollBar()->setRange(0, maxY);
    horizontalScrollBar()->setPageStep(page.cx ? int(qAbs(page.cx)) : vp.width());
    verticalScrollBar()->setPageStep(page.cy ? int(qAbs(page.cy)) : vp.height());
    horizontalScrollBar()->setSingleStep(line.cx ? int(qAbs(line.cx)) : 20);
    verticalScrollBar()->setSingleStep(line.cy ? int(qAbs(line.cy)) : 20);
}

CPoint MfcViewWidget::hostScrollPosition() const
{
    return CPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
}

void MfcViewWidget::hostScrollToPosition(const POINT &devicePos)
{
    horizontalScrollBar()->setValue(int(devicePos.x));
    verticalScrollBar()->setValue(int(devicePos.y));
}

void MfcViewWidget::hostSetStatusText(int pane, const char *text)
{
    // Не чаще двадцати раз в секунду: содержательно это то же самое, а очередь
    // событий остаётся свободной для самого счёта.
    static QElapsedTimer clock;
    if (!clock.isValid())
        clock.start();
    const QString value = text ? QString::fromLocal8Bit(text) : QString();

    // I-17: таблица придержанных текстов — под замком (вызовы и из потока расчёта, и из окна); текст,
    // пришедший чаще 50 мс, больше не теряется: показ последнего планируется через 50 мс в потоке окна.
    // Было: m_pendingStatus без блокировки; придержанный текст ждал следующего сообщения, которого могло
    // не быть (последнее сообщение шага не показывалось).
    QMap<int, QString> ready;
    {
        QMutexLocker lock(&m_statusMutex);
        const qint64 now = clock.elapsed();
        m_pendingStatus[pane] = value;
        if (now - m_lastStatusMs < 50) {
            if (!m_statusFlushScheduled) {
                m_statusFlushScheduled = true;
                QMetaObject::invokeMethod(this, [this]() {
                    QTimer::singleShot(50, this, [this]() { flushPendingStatus(); });
                }, Qt::QueuedConnection);
            }
            return;
        }
        m_lastStatusMs = now;
        ready.swap(m_pendingStatus);
    }
    for (auto it = ready.constBegin(); it != ready.constEnd(); ++it)
        emit statusTextChanged(it.key(), it.value());
    return;
}

void MfcViewWidget::flushPendingStatus()
{
    QMap<int, QString> ready;
    {
        QMutexLocker lock(&m_statusMutex);
        m_statusFlushScheduled = false;
        ready.swap(m_pendingStatus);
    }
    for (auto it = ready.constBegin(); it != ready.constEnd(); ++it)
        emit statusTextChanged(it.key(), it.value());
}


int MfcViewWidget::hostLogicalDpiX() const
{
    return logicalDpiX();
}

int MfcViewWidget::hostLogicalDpiY() const
{
    return logicalDpiY();
}
