// colorpicker.h — окно выбора цвета в оформлении TMC Suite.
//
// Заменяет стандартный QColorDialog (вид «старого Paint»), который открывали
// окна настройки цветов линий и графиков. Что есть в окне:
//   * большая область «насыщенность/яркость» и полоса оттенка;
//   * образцы «было/стало» (щелчок по «было» возвращает исходный цвет);
//   * поле HEX и движки R, G, B с числовыми полями;
//   * ряды образцов: палитра проекта, классические цвета графиков
//     Windows-версии и недавно выбранные цвета;
//   * пипетка — взять цвет с любого места экрана.
//
// Недавние цвета хранятся в INI-файле %APPDATA%\TMC_Suite_Qt\ColorPicker.ini
// (на Linux и macOS — в каталоге настроек пользователя под тем же именем).
//
// Класс не использует Q_OBJECT: сигналы не нужны, связи — лямбдами.

/// @file colorpicker.h
/// @brief Окно выбора цвета в оформлении пакета (`TmcColorPickerDialog`, `tmcPickColor`).
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_gui_common`. В
/// Windows-сборке его открывает `CColorDialog::DoModal` слоя MFC. Только поток интерфейса.
/// @see https://www.tamic.ru/docs/code/gui-common.html#colorpicker
#pragma once

#include <QColor>
#include <QDialog>
#include <QList>
#include <QString>

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QSpinBox;
class QTimer;
class QToolButton;
class QVariantAnimation;
class QGridLayout;
class QHBoxLayout;
QT_END_NAMESPACE

/// @brief Рисованные элементы окна выбора цвета (определены в `colorpicker.cpp`).
namespace tmccolor {
class SvArea;
class GradientSlider;
class ComparePreview;
class Swatch;
} // namespace tmccolor

/// Окно выбора цвета. Обычно достаточно функции tmcPickColor() ниже.
///
/// Цвет хранится одновременно в RGB (`m_color`) и HSV (`m_h`, `m_s`, `m_v`);
/// HSV хранится отдельно, чтобы на серых цветах не терялся оттенок. Недавние
/// цвета (до 12) — в INI-файле `TMC_Suite_Qt/ColorPicker.ini` профиля пользователя.
/// @see https://www.tamic.ru/docs/code/gui-common.html#colorpicker
class TmcColorPickerDialog : public QDialog
{
public:
    /// initial — цвет, с которого начинается выбор (он же образец «было»);
    /// title — заголовок окна, пустой — «Выбор цвета».
    /// @param[in] initial  начальный цвет; недействительный — белый; прозрачность отбрасывается
    /// @param[in] parent   окно-родитель
    /// @param[in] title    заголовок окна
    explicit TmcColorPickerDialog(const QColor &initial, QWidget *parent = nullptr,
                                  const QString &title = QString());
    /// @brief Деструктор.
    ~TmcColorPickerDialog() override;

    /// Выбранный цвет (всегда непрозрачный, RGB).
    /// @return Текущий цвет без прозрачности (после OK — выбранный).
    QColor color() const { return m_color; }
    /// Исходный цвет (образец «было»).
    /// @return Цвет, переданный конструктору (недействительный заменён белым).
    QColor initialColor() const { return m_initial; }

    /// Поставить цвет. animated — плавный переход от текущего цвета.
    /// @param[in] c         цвет; недействительный — без изменений
    /// @param[in] animated  true — анимация 220 мс (только если окно видно)
    void setColor(const QColor &c, bool animated = false);

    /// Недавно выбранные цвета, первый — самый свежий (не более 12).
    /// @return Список цветов из настроек окна; пуст — цвета ещё не выбирались.
    static QList<QColor> recentColors();
    /// Запомнить цвет в списке недавних (вызывается само при нажатии OK).
    /// @param[in] c  цвет; недействительный — без изменений; повтор переносится в начало
    static void rememberColor(const QColor &c);

protected:
    /// @brief OK: завершает пипетку и анимацию, запоминает цвет в недавних, закрывает окно.
    void accept() override;
    /// @brief Отмена: завершает пипетку без изменения цвета и закрывает окно.
    void reject() override;
    /// @brief В режиме пипетки: Esc — отмена, Enter — взять цвет под курсором.
    /// @param[in] e  событие клавиши
    void keyPressEvent(QKeyEvent *e) override;
    /// @brief В режиме пипетки: левая кнопка — взять цвет, другая — отмена.
    /// @param[in] e  событие мыши
    void mousePressEvent(QMouseEvent *e) override;

private:
    /// Источник изменения цвета: определяет, какие элементы не переписывать при обновлении.
    enum class Source { None, Sv, Hue, Rgb, Hex, Swatch, Anim, Pick };

    /// @brief Строит элементы окна.
    /// @param[in] title  заголовок; пустой — «Выбор цвета»
    void buildUi(const QString &title);
    /// @brief Задаёт цвет в RGB и пересчитывает HSV (оттенок серого и насыщенность чёрного сохраняются).
    /// @param[in] c    цвет
    /// @param[in] src  источник изменения
    void setFromRgb(const QColor &c, Source src);
    /// @brief Задаёт цвет в HSV.
    /// @param[in] h    оттенок, 0…0,9999
    /// @param[in] s    насыщенность, 0…1
    /// @param[in] v    яркость, 0…1
    /// @param[in] src  источник изменения
    void setFromHsv(double h, double s, double v, Source src);
    /// @brief Обновляет все элементы по текущему цвету (поле HEX — кроме ввода в нём самом).
    /// @param[in] src  источник изменения
    void refresh(Source src);
    /// @brief Заполняет ряд «Недавние» образцами из настроек.
    void fillRecentRow();
    /// @brief Создаёт образец, по щелчку плавно ставящий его цвет.
    /// @param[in] c     цвет
    /// @param[in] name  подпись (пустая — только HEX)
    /// @return Новый образец (принадлежит окну).
    tmccolor::Swatch *makeSwatch(const QColor &c, const QString &name);
    /// @brief Включает пипетку: захват мыши и клавиатуры, опрос цвета под курсором каждые 30 мс.
    void startPicking();
    /// @brief Выключает пипетку.
    /// @param[in] keep  true — оставить взятый цвет, false — вернуть цвет до пипетки
    void stopPicking(bool keep);
    /// @brief Берёт цвет пикселя экрана под курсором.
    void pickTick();

    /// Исходный цвет («было»).
    QColor m_initial;
    /// Текущий цвет, непрозрачный RGB.
    QColor m_color;
    /// @name Текущий цвет в HSV (оттенок, насыщенность, яркость, 0…1)
    ///@{
    /// Оттенок, насыщенность, яркость текущего цвета, 0…1.
    double m_h = 0.0, m_s = 0.0, m_v = 0.0;   // 0..1, оттенок хранится отдельно,
                                              // чтобы не терять его на серых
    //@}

    /// Область «насыщенность/яркость».
    tmccolor::SvArea *m_sv = nullptr;
    /// Полоса оттенка.
    tmccolor::GradientSlider *m_hue = nullptr;
    /// Движки каналов R, G, B.
    tmccolor::GradientSlider *m_chan[3] = { nullptr, nullptr, nullptr };
    /// Числовые поля R, G, B (0…255).
    QSpinBox *m_spin[3] = { nullptr, nullptr, nullptr };
    /// Поле кода `#RRGGBB`.
    QLineEdit *m_hex = nullptr;
    /// Образцы «было/стало».
    tmccolor::ComparePreview *m_preview = nullptr;
    /// Кнопка пипетки.
    QToolButton *m_pickBtn = nullptr;
    /// Подсказка в режиме пипетки.
    QLabel *m_hint = nullptr;
    /// Ряд недавних цветов.
    QHBoxLayout *m_recentRow = nullptr;
    /// Надпись «Здесь появятся выбранные цвета» или NULL.
    QLabel *m_recentEmpty = nullptr;
    /// Все образцы окна (для отметки выбранного).
    QList<tmccolor::Swatch *> m_swatches;
    /// Анимация плавного перехода цвета.
    QVariantAnimation *m_anim = nullptr;
    /// Таймер опроса пипетки.
    QTimer *m_pickTimer = nullptr;
    /// Признак режима пипетки.
    bool m_picking = false;
    /// Цвет до включения пипетки.
    QColor m_beforePick;
    /// Признак «идёт обновление элементов» (подавляет обратную связь числовых полей).
    bool m_updating = false;
};

/// Открыть окно выбора цвета. true — пользователь нажал OK, цвет в *result.
/// Выбранный цвет попадает в «недавние».
/// @param[in]  parent   окно-родитель
/// @param[in]  initial  начальный цвет
/// @param[out] result   выбранный цвет (может быть NULL)
/// @param[in]  title    заголовок окна
/// @return true — OK, false — отмена.
bool tmcPickColor(QWidget *parent, const QColor &initial, QColor *result,
                  const QString &title = QString());
