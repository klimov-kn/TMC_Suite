/// @file windowplacement.cpp
/// @brief Реализация сохранения положения окна.
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_gui_common`.
/// @see https://www.tamic.ru/docs/code/gui-common.html#window-placement
#include "windowplacement.h"

#include <QEvent>
#include <QGuiApplication>
#include <QObject>
#include <QRect>
#include <QScreen>
#include <QWidget>

#include <cstdlib>

#include "tmc_mfc_doc.h"

namespace {

// Раздел настроек тот же, где программа хранит остальные параметры.
/// Раздел настроек положения окон.
const char *kSection = "Window";

// Следит за окном: восстанавливает положение при первом показе и сохраняет его
// при закрытии. Живёт как ребёнок окна, поэтому удаляется вместе с ним.
/// @brief Фильтр событий окна: восстанавливает положение при первом показе, сохраняет при закрытии.
class PlacementKeeper : public QObject
{
public:
    /// @brief Создаёт сторожа и ставит его фильтром событий окна.
    /// @param[in] window  окно (владелец сторожа)
    /// @param[in] key     ключ настроек
    PlacementKeeper(QWidget *window, const QString &key)
        : QObject(window), m_window(window), m_key(key)
    {
        window->installEventFilter(this);
    }

protected:
    /// @brief Ловит первый `Show` и каждый `Close` окна.
    /// @param[in] watched  объект события
    /// @param[in] event    событие
    /// @return Результат базового фильтра (событие не поглощается).
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == m_window) {
            if (event->type() == QEvent::Show && !m_restored) {
                m_restored = true;
                restore();
            } else if (event->type() == QEvent::Close) {
                save();
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    /// @brief Возвращает ключ настроек.
    /// @return Ключ, переданный конструктору.
    QString settingKey() const { return m_key; }

    /// @brief Ставит окно по сохранённому значению (ограничив экраном, где его оставили) или по центру экрана.
    void restore()
    {
        // Средства проверки задают размер окна сами: им нужен повторяемый
        // снимок, а не последнее положение окна у пользователя.
        if (std::getenv("TMC_NO_PLACEMENT"))
            return;

        const CString value = AfxGetApp()->GetProfileString(
            kSection, settingKey().toLocal8Bit().constData(), "");
        int x = 0, y = 0, w = 0, h = 0;
        if (std::sscanf(value.GetString(), "%d %d %d %d", &x, &y, &w, &h) == 4
            && w > 100 && h > 100) {
            // Окно должно остаться на экране: настройки могли остаться от
            // машины с другим числом мониторов. Экран — тот, где окно оставили
            // (второй монитор), а не всегда основной (Т-22); если такого экрана
            // больше нет — основной.
            QScreen *screen = QGuiApplication::screenAt(QPoint(x + w / 2, y + h / 2));
            if (!screen)
                screen = QGuiApplication::screenAt(QPoint(x, y));
            if (!screen)
                screen = QGuiApplication::primaryScreen();
            if (screen) {
                const QRect avail = screen->availableGeometry();
                w = qMin(w, avail.width());
                h = qMin(h, avail.height());
                // avail.right() = left + width - 1: при w == ширине экрана верхняя
                // граница qBound была меньше нижней (окно на x = −1, в отладочной
                // сборке Qt — аварийный выход, Т-22).
                x = qBound(avail.left(), x, avail.left() + avail.width() - w);
                y = qBound(avail.top(), y, avail.top() + avail.height() - h);
            }
            m_window->setGeometry(x, y, w, h);
            return;
        }
        tmcui::centerOnScreen(m_window);
    }

    /// @brief Записывает положение и размер окна в настройки.
    void save()
    {
        const QRect g = m_window->geometry();
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%d %d %d %d",
                      g.x(), g.y(), g.width(), g.height());
        AfxGetApp()->WriteProfileString(kSection,
                                        settingKey().toLocal8Bit().constData(),
                                        buffer);
    }

    /// Окно.
    QWidget *m_window;
    /// Ключ настроек.
    QString  m_key;
    /// Положение уже восстановлено.
    bool     m_restored = false;
};

} // namespace

namespace tmcui {

void keepPlacement(QWidget *window, const QString &key)
{
    if (window)
        new PlacementKeeper(window, key);
}

void centerOnScreen(QWidget *window)
{
    if (!window)
        return;
    QScreen *screen = window->screen() ? window->screen() : QGuiApplication::primaryScreen();
    if (!screen)
        return;
    const QRect avail = screen->availableGeometry();
    const QSize size = window->size();
    window->move(avail.left() + (avail.width() - size.width()) / 2,
                 avail.top() + (avail.height() - size.height()) / 2);
}

} // namespace tmcui
