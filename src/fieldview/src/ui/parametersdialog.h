// Окно «Параметры» (View → Parameters, Alt+P; CTMCDialogPropet Windows-версии):
// вкладки «Размещение» (Place), «Топология» (Topology), «ε» (Eps), «Поле» (Field).
// Кнопки ОК / Отмена / Применить — как у набора вкладок оригинала.
/// @file parametersdialog.h
/// @brief Окно «Параметры вида»: вкладки «Размещение», «Топология», «eps и плазма» (в окне — греческая буква), «Поле».
///
/// Часть модуля FieldView (слой `ui`).
/// @see https://www.tamic.ru/docs/code/fieldview.html#dialogs
#pragma once

#include "viewsettings.h"

#include <QDialog>
#include <functional>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QDoubleSpinBox;
class QTabWidget;

namespace fieldview {

/// @brief Сведения о файле и окне документа, которые нужны окну «Параметры».
///
/// Сведения о файле, нужные окну (единицы, смещение начала сетки, плазма).
///
/// Заполняет `MainWindow::showParameters()`.
struct ParametersContext
{
    /// Имя единицы длины файла (подписи полей min, max).
    QString longUnit = QStringLiteral("m");
    /// Имя единицы времени файла.
    QString timeUnit = QStringLiteral("s");
    /// Единица длины в метрах; края окна показываются в этих единицах.
    double longUnitValue = 1.0;
    /// @name Начало сетки файла
    ///@{
    /// Координаты узла (0, 0), м: края окна показываются в координатах задания.
    double xOrigin = 0.0, yOrigin = 0.0;   // #TMCGROFLD_Xmin / Ymin файла
    ///@}
    /// В топологии есть сетки N, Y, B (доступен выбор величины).
    bool plasma = false;
    /// Слой eps можно показать.
    bool epsAvailable = false;
    /// Почему слой eps недоступен.
    QString epsReason;
    /// @name Действующие пределы шкал
    ///@{
    /// Пределы поля и слоя eps — начальные значения полей min, max.
    double fieldZMin = -1, fieldZMax = 1, epsZMin = 0, epsZMax = 1;
    ///@}
};

/// @brief Модальное окно настроек вида с кнопками «ОК», «Отмена», «Применить».
///
/// Работает с копией настроек. «Применить» передаёт текущие значения функции `apply`,
/// «ОК» закрывает окно — результат забирается `result()`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#dialogs
class ParametersDialog : public QDialog
{
    Q_OBJECT
public:
    /// @brief Создаёт окно, заполняет поля из настроек и открывает сохранённую вкладку.
    /// @param[in] s       настройки вида окна документа
    /// @param[in] ctx     сведения о файле
    /// @param[in] apply   действие кнопки «Применить» (может быть пустым)
    /// @param[in] parent  родительское окно
    ParametersDialog(const ViewSettings &s, const ParametersContext &ctx,
                     std::function<void(const ViewSettings &)> apply, QWidget *parent = nullptr);
    /// @brief Переносит значения полей окна в настройки и возвращает их.
    /// @return Настройки с изменениями пользователя (края окна — в метрах от начала сетки).
    ViewSettings result();

private:
    /// @brief Строит вкладку «Размещение»: оси X и Y, оси и надписи, камера, единицы, пропорции.
    /// @return Виджет вкладки.
    QWidget *buildPlace();
    /// @brief Строит вкладку «Топология»: типы блоков и цвета, контуры и заливка, 2D/3D, размеры, непрозрачность.
    /// @return Виджет вкладки.
    QWidget *buildTopology();
    /// @brief Строит вкладку слоя: рисование, шкала и цвета, пределы Z (у eps — ещё выбор величины).
    /// @param[in] field  `true` — вкладка «Поле», `false` — «eps и плазма» (в окне — греческая буква)
    /// @return Виджет вкладки.
    QWidget *buildLayer(bool field);
    /// @brief Заполняет поля окна из настроек (края окна — в единицах файла от начала координат задания).
    void load();
    /// @brief Переносит значения полей окна в настройки.
    void store();
    /// @brief Делает доступными поля min, max только при выключенном «Авто».
    void updateEnables();
    /// @brief Перерисовывает образцы цветов материалов, надписей и шкал.
    void refreshSwatches();

    /// Редактируемая копия настроек.
    ViewSettings m_s;
    /// Сведения о файле.
    ParametersContext m_ctx;
    /// Действие кнопки «Применить».
    std::function<void(const ViewSettings &)> m_apply;
    /// Вкладки окна.
    QTabWidget *m_tabs = nullptr;

    // Place.
    /// @name Вкладка «Размещение»
    ///@{
    /// Флажки: вся область X, Y; оси; размеры по X, Y; пропорционально.
    QCheckBox *m_xAuto, *m_yAuto, *m_drawAxes, *m_sizeX, *m_sizeY, *m_prop;
    ///@}
    /// @name Края окна данных
    ///@{
    /// Поля min, max по X и Y, единицы длины файла.
    QLineEdit *m_xMin, *m_xMax, *m_yMin, *m_yMax;
    ///@}
    /// @name Камера
    ///@{
    /// Углы, градусы; сдвиги, единицы сцены; масштаб.
    QDoubleSpinBox *m_angX, *m_angY, *m_angZ, *m_trX, *m_trY, *m_trZ, *m_zoom;
    ///@}
    /// @name Кнопки цвета и шрифта надписей
    ///@{
    /// Открывают выбор цвета и шрифта.
    QPushButton *m_axesColor, *m_axesFont;
    ///@}
    /// Образец надписи.
    QLabel *m_axesSample;

    // Topology.
    /// Флажки показа типов блоков (порядок — как у цветов материалов).
    QCheckBox *m_tFlag[6];
    /// Образцы цветов материалов.
    QLabel *m_tSwatch[6];
    /// @name Флажки вкладки «Топология»
    ///@{
    /// Контуры, заливка, размеры блоков, привязки.
    QCheckBox *m_tLine, *m_tSurf, *m_tSizeBlock, *m_tSizeLink;
    ///@}
    /// @name Вид топологии
    ///@{
    /// Плоско (2D) или объёмно (3D).
    QRadioButton *m_t2d, *m_t3d;
    ///@}
    /// Непрозрачность топологии, %.
    QSpinBox *m_tBlend;

    // Слои ε (0) и поле (1).
    /// @brief Элементы вкладки слоя (eps или поле).
    struct LayerUi
    {
        /// @name Флажки слоя
        ///@{
        /// Заливка, сетка, авто-пределы, шкала Z в сцене, плавная шкала.
        QCheckBox *surface, *line, *zAuto, *zSize, *smooth;
        ///@}
        /// @name Вид слоя
        ///@{
        /// Плоско (2D) или рельеф (3D).
        QRadioButton *d2, *d3;
        ///@}
        /// Непрозрачность слоя, %.
        QSpinBox *blend;
        /// @name Пределы Z
        ///@{
        /// Поля min и max.
        QLineEdit *zMin, *zMax;
        ///@}
        /// Выбор шкалы (данные элемента — `ScaleKind`).
        QComboBox *scale;
        /// Образец шкалы.
        QLabel *preview;
        /// @name Флажки только вкладки «Поле»
        ///@{
        /// «Модуль поля» и «Авто по текущему такту»; у вкладки eps — `nullptr`.
        QCheckBox *modulus = nullptr, *perFrame = nullptr;
        ///@}
        /// Высота рельефа (только вкладка «Поле»).
        QDoubleSpinBox *height = nullptr;
        /// Выбор величины eps, N, Y, B (только вкладка eps).
        QRadioButton *type[4] = {};
    } m_l[2];
    /// @var ParametersDialog::m_l
    /// Элементы вкладок: [0] — eps, [1] — поле.
};

} // namespace fieldview
