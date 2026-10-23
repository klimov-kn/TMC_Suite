// dnviewwidget.h — виджет-хозяин вида TMC_DN, безопасный для фонового потока.
//
// Зачем нужен. TMC_DN считает диаграмму направленности в отдельном потоке
// (`AfxBeginThread(ReadDocFileW_Thread1, ...)` в tmcgroutview.cpp). В конце
// расчёта код Windows-версии зовёт `RedrawWindow()`, а по дороге — `PutTrace` и
// `PutStatistics`. В Windows рисовать из другого потока было можно; в Qt
// обращаться к виджетам разрешено только из главного потока, и обычный
// `MfcViewWidget` в этом месте падает (SIGSEGV в QPainter::begin, стек уходит
// в QWidget::repaint прямо из потока расчёта).
//
// Здесь те же вызовы, придя из фонового потока, кладутся в очередь событий
// главного потока. Порядок вычислений и результат от этого не меняются: расчёт
// как шёл в своём потоке, так и идёт, откладывается только перерисовка.
//
// Правильное место для такой защиты — общий MfcViewWidget (src/gui_common):
// то же самое понадобится счётным оболочкам PlanarRT_H/PlanarRT_X. Пока он
// занят другим исполнителем, защита живёт здесь.

/// @file dnviewwidget.h
/// @brief Виджет-хозяин вида TMC_DN, безопасный для потока расчёта: перерисовка и окна сообщений из чужого потока уходят в главный.
///
/// Часть модуля «Вьювер TMC_DN».
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#thread
#pragma once

#include "mfcviewwidget.h"

/// @brief `MfcViewWidget`, у которого обращения из потока расчёта ставятся в очередь событий главного потока.
///
/// Создаётся главным окном на каждое окно документа (`MainWindow::openDocument`), владеет
/// им подокно MDI. Из главного потока все вызовы идут в базовый класс как есть.
/// @note Потоки: методы `host*` можно вызывать из любого потока.
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#thread
class DnViewWidget : public MfcViewWidget
{
    Q_OBJECT

public:
    /// @brief Создаёт виджет.
    /// @param[in] parent родительский виджет
    explicit DnViewWidget(QWidget *parent = nullptr);

protected:
    // --- MfcQtHost: то, до чего дотягивается поток расчёта -------------------
    /// @brief Помечает область вида для перерисовки; из чужого потока — всё окно, через очередь событий.
    /// @param[in] logicalRect область, логические координаты вида; NULL — всё окно
    /// @param[in] erase       TRUE — стереть фон
    void hostInvalidate(const RECT *logicalRect, BOOL erase) override;
    /// @brief Немедленная перерисовка; из чужого потока заменяется отложенной перерисовкой всего окна.
    void hostUpdateWindow() override;
    /// @brief Окно сообщения; из чужого потока показывается в главном потоке без ожидания ответа.
    /// @param[in] text    текст (локальная кодировка)
    /// @param[in] caption заголовок
    /// @param[in] type    флаги `MB_*` (из чужого потока не передаются)
    /// @return Ответ пользователя; из чужого потока — 1 (`IDOK`) сразу.
    int  hostMessageBox(const char *text, const char *caption, UINT type) override;

private:
    /// Пришёл ли вызов из главного потока (где живёт виджет).
    /// @return true — вызов из потока, которому принадлежит виджет.
    bool inGuiThread() const;
};
