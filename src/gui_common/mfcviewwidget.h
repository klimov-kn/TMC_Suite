// mfcviewwidget.h — виджет-хозяин для вида, перенесённого из Windows-версии.
//
// Вид (CScrollView и наследники) компилируется из исходников Windows-версии и
// умеет рисовать себя в CDC и отвечать на события мыши. Этот виджет даёт ему всё
// остальное: холст QPainter, прокрутку, захват мыши, таймер, окна сообщений.
//
// Разделение обязанностей:
//   вид      — ЧТО рисовать и как реагировать (код Windows-версии, не тронут);
//   виджет   — ГДЕ рисовать и откуда берутся события (Qt).

/// @file mfcviewwidget.h
/// @brief Виджет-хозяин вида MFC (`CScrollView`): холст `QPainter`, прокрутка, мышь, таймеры, строка состояния.
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_gui_common`.
/// Используется вьюверами и оболочками ядер.
/// @see https://www.tamic.ru/docs/code/gui-common.html#view-widget
#pragma once

#include <QAbstractScrollArea>
#include <QColor>
#include <QElapsedTimer>
#include <QMap>
#include <QMutex>

#include "mfc_view.h"

/// @brief Виджет Qt, реализующий `MfcQtHost` для вида MFC.
///
/// На каждую перерисовку создаёт `CDC` над `QPainter` области просмотра, вызывает
/// `OnPrepareDC` и `OnDraw` вида; события мыши и таймера переводит в обработчики
/// `On*` вида. Вызовы перерисовки и окон сообщений из расчётного потока
/// откладываются в очередь главного потока. Вид не принадлежит виджету.
/// @see https://www.tamic.ru/docs/code/gui-common.html#view-widget
class MfcViewWidget : public QAbstractScrollArea, public MfcQtHost
{
    Q_OBJECT

public:
    /// @brief Создаёт виджет (слежение за мышью включено).
    /// @param[in] parent  родитель
    explicit MfcViewWidget(QWidget *parent = nullptr);
    /// @brief Отвязывает вид (`SetHost(nullptr)`).
    ~MfcViewWidget() override;

    /// Привязать вид. Владение остаётся у вызывающего.
    /// @param[in] view  вид или NULL
    void setView(CScrollView *view);
    /// @brief Возвращает привязанный вид (может быть NULL).
    /// @return Вид (не владеет виджет); NULL — вид не привязан.
    CScrollView *view() const { return m_view; }

    /// Вызвать OnInitialUpdate у вида (после того, как виджет получил размер).
    /// @note Выполняется один раз; повторные вызовы ничего не делают.
    void initialUpdate();

    /// Цвет подложки области рисования. В Windows-версии фон давало само окно
    /// (белый), а документ хранит свой цвет — его и ставим, чтобы картинка
    /// совпадала с оригиналом.
    /// @param[in] color  цвет фона документа; в Windows-сборке белый и чёрный заменяются фоном темы, серый 192 — цветом панелей
    void setDocumentBackground(const QColor &color);

signals:
    /// Текст для строки состояния: вид шлёт его через hostSetStatusText.
    /// @param[in] pane  номер поля строки состояния
    /// @param[in] text  текст
    void statusTextChanged(int pane, const QString &text);

protected:
    /// @brief Заливает фон и вызывает `OnPrepareDC` + `OnDraw` вида через `CDC`.
    /// @param[in] event  событие перерисовки Qt
    void paintEvent(QPaintEvent *event) override;
    /// @brief Передаёт нажатие левой/правой кнопки в `OnLButtonDown`/`OnRButtonDown`.
    /// @param[in] event  событие мыши Qt
    void mousePressEvent(QMouseEvent *event) override;
    /// @brief Снимает «забытый» захват мыши и передаёт отпускание левой кнопки в `OnLButtonUp`.
    /// @param[in] event  событие мыши Qt
    void mouseReleaseEvent(QMouseEvent *event) override;
    /// @brief Передаёт двойной щелчок левой кнопкой в `OnLButtonDblClk`.
    /// @param[in] event  событие мыши Qt
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    /// @brief Передаёт движение мыши в `OnMouseMove`.
    /// @param[in] event  событие мыши Qt
    void mouseMoveEvent(QMouseEvent *event) override;
    /// @brief Перерисовывает область просмотра при изменении размера (`OnSize` вида не вызывается).
    /// @param[in] event  событие изменения размера Qt
    void resizeEvent(QResizeEvent *event) override;
    /// @brief Передаёт срабатывание таймера в `OnTimer` вида с номером MFC.
    /// @param[in] event  событие таймера Qt (номер таймера Qt)
    void timerEvent(QTimerEvent *event) override;

    // --- MfcQtHost -----------------------------------------------------------
    /// @brief Реализация `MfcQtHost`: перерисовать всё окно (область не учитывается).
    /// @param[in] logicalRect  область в логических единицах (не используется: перерисовывается всё)
    /// @param[in] erase        стирать ли фон (не используется)
    void hostInvalidate(const RECT *logicalRect, BOOL erase) override;
    /// @brief Реализация `MfcQtHost`: немедленная перерисовка (из другого потока или во время `OnDraw` — отложенная).
    void hostUpdateWindow() override;
    /// @brief Реализация `MfcQtHost`: размер области просмотра, пиксели.
    /// @param[out] deviceRect  прямоугольник (0, 0, ширина, высота), пиксели; NULL — ничего
    void hostGetClientRect(RECT *deviceRect) const override;
    /// @brief Реализация `MfcQtHost`: захват мыши намеренно не выполняется.
    void hostSetCapture() override;
    /// @brief Реализация `MfcQtHost`: снимает захват мыши, если он у этого виджета.
    void hostReleaseCapture() override;
    /// @brief Реализация `MfcQtHost`: запускает таймер Qt и связывает его с номером MFC.
    /// @param[in] id        номер таймера MFC
    /// @param[in] elapseMs  период, мс
    /// @return Всегда `id`.
    UINT hostSetTimer(UINT id, UINT elapseMs) override;
    /// @brief Реализация `MfcQtHost`: останавливает таймер по номеру MFC.
    /// @param[in] id  номер таймера MFC
    /// @return TRUE — таймер найден и остановлен; FALSE — таймера с таким номером нет.
    BOOL hostKillTimer(UINT id) override;
    /// @brief Реализация `MfcQtHost`: информационное окно (текст в локальной кодировке).
    /// @warning Тип кнопок не учитывается, результат всегда 1 (`IDOK`); из расчётного потока окно показывается асинхронно.
    /// @param[in] text     текст в локальной кодировке; NULL — пустой
    /// @param[in] caption  заголовок; NULL — заголовок окна виджета
    /// @param[in] type     флаги `MB_*` (не используются)
    /// @return Всегда 1 (`IDOK`).
    int  hostMessageBox(const char *text, const char *caption, UINT type) override;
    /// @brief Реализация `MfcQtHost`: диапазоны и шаги полос прокрутки (шаг строки по умолчанию 20 пикселей).
    /// @param[in] total  полный размер содержимого, пиксели (знак не учитывается)
    /// @param[in] page   шаг страницей, пиксели; 0 — размер области просмотра
    /// @param[in] line   шаг строкой, пиксели; 0 — 20 пикселей
    void hostSetScrollSizes(const SIZE &total, const SIZE &page, const SIZE &line) override;
    /// @brief Реализация `MfcQtHost`: позиция полос прокрутки, пиксели.
    /// @return Значения горизонтальной и вертикальной полос прокрутки, пиксели.
    CPoint hostScrollPosition() const override;
    /// @brief Реализация `MfcQtHost`: ставит полосы прокрутки в позицию, пиксели.
    /// @param[in] devicePos  позиция, пиксели
    void hostScrollToPosition(const POINT &devicePos) override;
    /// @brief Реализация `MfcQtHost`: текст строки состояния, не чаще раза в 50 мс (`statusTextChanged`).
    /// @warning Сообщение, пришедшее раньше 50 мс после предыдущего показа, только запоминается
    ///          и показывается со следующим вызовом; если вызовов больше нет — оно не показывается.
    /// @param[in] pane  номер поля строки состояния
    /// @param[in] text  текст в локальной кодировке; NULL — пустой
    void hostSetStatusText(int pane, const char *text) override;
    /// @brief Реализация `MfcQtHost`: логическое разрешение виджета по X, точек на дюйм.
    /// @return Точек на дюйм по X (`QWidget::logicalDpiX`).
    int  hostLogicalDpiX() const override;
    /// @brief Реализация `MfcQtHost`: логическое разрешение виджета по Y, точек на дюйм.
    /// @return Точек на дюйм по Y (`QWidget::logicalDpiY`).
    int  hostLogicalDpiY() const override;

    /// Пришёл ли вызов из главного потока (в котором живёт виджет).
    ///
    /// Расчёт в TMC идёт в отдельном потоке (AfxBeginThread), а код
    /// Windows-версии зовёт оттуда RedrawWindow/Invalidate и окна сообщений.
    /// В Windows это было можно, в Qt — нет: работать с виджетами разрешено
    /// только главному потоку. Поэтому такие вызовы откладываются в его
    /// очередь событий. Порядок вычислений от этого не меняется.
    /// @return true — текущий поток совпадает с потоком виджета.
    bool inGuiThread() const;

private:
    /// @brief Переводит кнопки и модификаторы события в флаги `MK_*`.
    /// @param[in] event  событие мыши
    /// @return Сумма флагов `MK_LBUTTON`, `MK_RBUTTON`, `MK_MBUTTON`, `MK_SHIFT`, `MK_CONTROL`; 0 — ничего не нажато.
    UINT mouseFlags(QMouseEvent *event) const;
    /// @brief Возвращает точку события в пикселях содержимого (с учётом прокрутки).
    /// @param[in] event  событие мыши
    /// @return Координаты указателя плюс позиция прокрутки, пиксели.
    CPoint devicePoint(QMouseEvent *event) const;

    /// Привязанный вид (не владеет).
    CScrollView      *m_view = nullptr;
    /// Идёт `OnDraw`: немедленная перерисовка заменяется отложенной.
    bool              m_painting = false;   // защита от перерисовки внутри OnDraw
#ifdef TMC_QT_WIN
    /// Цвет фона области рисования.
    QColor            m_background = QColor(0x0a, 0x0e, 0x16);   // фон проекта (bg)
#else
    /// Цвет фона области рисования (вне Windows-сборки — белый, как в Windows-версии).
    QColor            m_background = Qt::white;
#endif

    // Ограничение частоты обновления строки состояния. Счётное ядро сообщает о
    // ходе работы на каждом такте — это тысячи сообщений в секунду. Человек
    // столько не читает, а очередь событий забивается, поэтому пропускаем не
    // чаще, чем раз в 50 мс, но последнее сообщение показываем обязательно.
    /// Последний непоказанный текст по номерам полей строки состояния.
    QMap<int, QString> m_pendingStatus;
    /// Время последнего показа текста, мс от первого вызова.
    qint64             m_lastStatusMs = 0;
    /// I-17: защита `m_pendingStatus` и `m_lastStatusMs` — сообщения приходят и из потока расчёта.
    QMutex             m_statusMutex;
    /// I-17: отложенный показ последнего придержанного текста уже запланирован.
    bool               m_statusFlushScheduled = false;
    /// @brief I-17: показывает все придержанные тексты строки состояния (в потоке окна).
    void flushPendingStatus();
    /// Таймеры: номер таймера Qt -> номер MFC.
    QMap<int, UINT>   m_timers;      // идентификатор Qt -> идентификатор MFC
    /// `OnInitialUpdate` уже вызван.
    bool              m_initialDone = false;
};
