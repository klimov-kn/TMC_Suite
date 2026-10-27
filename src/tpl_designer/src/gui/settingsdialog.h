/// @file settingsdialog.h
/// @brief Окно настроек вида редактора: заливка eps, сетка, привязка, размеры, перезапись.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса. Открывается кнопкой
/// настроек мастера (`MainWindow`); правит `settings::Values`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-settings
#pragma once

#include "settings.h"

#include <QDialog>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QComboBox;
class QLabel;
QT_END_NAMESPACE

/// Настройки программы.
///
/// Изменения применяются СРАЗУ, по каждому переключателю, а не по кнопке «ОК»: за
/// диалогом видно тот же холст, и смысл настройки виден только тогда, когда результат
/// появляется на глазах. «Отмена» здесь была бы обманом — поэтому её и нет, есть
/// «Вернуть по умолчанию».
/// @brief Модальный диалог настроек; каждое изменение сразу записывается и рассылается сигналом.
///
/// Создаётся на стеке в `MainWindow` и живёт, пока открыт. Не влияет на содержимое `.tpl`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-settings
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    /// @brief Строит окно (группы «Неоднородная eps», «Холст», «Поведение»), заполняет его из `settings::values()`.
    /// @param[in] parent  родительское окно; `nullptr` — без родителя.
    explicit SettingsDialog(QWidget *parent = nullptr);

signals:
    /// Настройки изменились — вид пора перерисовать.
    void settingsChanged();

private:
    /// @brief Переносит текущие настройки в флажки и список палитр, не вызывая `push()`.
    void pull();  ///< настройки -> виджеты
    /// @brief Собирает значения из виджетов, записывает их `settings::setValues` и испускает `settingsChanged`.
    void push();  ///< виджеты -> настройки (и сигнал)
    /// @brief Рисует полоску-образец выбранной палитры (256 цветов) в `m_preview`.
    void updatePreview();

    /// true — идёт `pull()`; переключения флажков в это время не записываются.
    bool m_loading = false;

    /// Флажок «Заливать градиентом по формуле» (`Values::epsGradient`).
    QCheckBox *m_epsGradient = nullptr;
    /// Флажок «Показывать цветовую шкалу значений» (`Values::epsColorBar`).
    QCheckBox *m_epsColorBar = nullptr;
    /// Флажок «Показывать eps под курсором» (`Values::epsTooltip`).
    QCheckBox *m_epsTooltip = nullptr;
    /// Список палитр; индекс = значение `settings::Palette`.
    QComboBox *m_palette = nullptr;
    /// Полоска-образец выбранной палитры.
    QLabel *m_preview = nullptr;

    /// Флажок «Сетка» (`Values::gridVisible`).
    QCheckBox *m_grid = nullptr;
    /// Флажок «Привязка к сетке и объектам» (`Values::snapEnabled`).
    QCheckBox *m_snap = nullptr;
    /// Флажок «Размеры выделенной фигуры» (`Values::showDimensions`).
    QCheckBox *m_dimensions = nullptr;
    /// Флажок «Предупреждать перед записью поверх существующего файла» (`Values::confirmOverwrite`).
    QCheckBox *m_confirmOverwrite = nullptr;
};
