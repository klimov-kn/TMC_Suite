// Настройки вида FieldView — те же флаги и величины, что у CFldViewView Windows-версии
// (SetDefaultValue, WriteIniFile), сгруппированные по слоям. Хранятся в настройках
// пользователя (на Windows — реестр HKCU\Software\TMC Suite\FieldView), раздел
// «#TMC_GraphicsOutputFieldFile», имена ключей — как в оригинале.
//
// Отличия значений по умолчанию (новое оформление, пометка «новое» в README):
//   поле — рельеф (3D) с заливкой и каркасом, шкала «Авто» (Волна/Свечение), плавно;
//   камера — наклон −55°, поворот −20° (как в мобильной версии);
//   топология — объёмная (3D), цвета материалов — из оформления пакета.
/// @file viewsettings.h
/// @brief Настройки вида FieldView по слоям (поле, eps, топология, размещение, камера) и их хранение.
///
/// Часть модуля FieldView (слой `app`). Хранятся в настройках пользователя `QSettings`,
/// раздел «TMC_GraphicsOutputFieldFile»; на Windows — файл INI в папке `%APPDATA%`
/// (см. `setupApplication()`).
/// @see https://www.tamic.ru/docs/code/fieldview.html#settings
#pragma once

#include "palette.h"

#include <QColor>
#include <QFont>

namespace fieldview {

/// @brief Общие настройки слоя сетки (поле или eps).
struct LayerSettings
{
    /// Рисовать заливку.
    bool surface = true;      // bDrawSurfaceFlag
    /// Рисовать каркас (сетку).
    bool line = true;         // bDrawLineFlag
    /// Рельеф (3D); `false` — плоская карта.
    bool dim3d = true;        // n2D3DFlag (TMCFIELDVIEW_3DDraw)
    /// Непрозрачность слоя, % (0 … 100).
    int blend = 100;          // nBlend, 0…100
    /// Пределы Z по данным; `false` — из `zMin`, `zMax`.
    bool zAuto = true;        // bZsizeFlag
    /// Нижний предел Z (при `zAuto` сюда записывается действующий), единицы величины слоя.
    double zMin = -1.0;       // dZmin
    /// Верхний предел Z (при `zAuto` сюда записывается действующий), единицы величины слоя.
    double zMax = 0.0;        // dZmax
    /// Рисовать шкалу Z в сцене (вертикальную ось с подписями пределов).
    bool drawZSize = false;   // bDrawFlagAxiesZFieldSize
    /// Цветовая шкала слоя.
    ScaleKind scale = ScaleKind::Auto;
    /// Плавная шкала (256 уровней); `false` — 20 ступеней.
    bool smooth = true;       // bHightColorRezolution («256 уровней»)
    /// Двадцать цветов уровней пользователя (шкала `Custom`).
    Colors20 custom;          // pscSurfaceColor
};

/// @brief Настройки слоя поля.
struct FieldLayerSettings : LayerSettings
{
    /// Режим «Модуль»: пределы +-max|v|, цвет по |h|.
    bool modulus = false;     // bDrawFieldModulFlag
    /// Автоматические пределы по текущему такту; `false` — по всем показанным тактам.
    bool zPerFrame = false;   // новое: пределы Z по текущему такту, а не по всем
    /// Множитель высоты рельефа, 0,05 … 10.
    double heightScale = 1.0; // новое: множитель высоты рельефа
    /// Контраст: автоматические пределы делятся на `gain`, 1 … 100.
    double gain = 1.0;        // новое: контраст — авто-пределы делятся на gain (как в мобильной версии)
};

/// @brief Настройки слоя eps (и сеток плазмы).
struct EpsLayerSettings : LayerSettings
{
    /// Величина слоя: 0 — eps, 1 — N, 2 — Y, 3 — B.
    int type = 0;             // nEpsType: 0 — ε, 1 — N, 2 — Y, 3 — B
};

/// @brief Настройки показа топологии.
struct TopologySettings
{
    /// Рисовать контуры полос.
    bool line = true;         // bDrawLineFlagTopology
    /// Рисовать заливку полос.
    bool surface = true;      // bDrawSurfaceFlagTopology
    /// Объёмные блоки (призмы высотой от -0,25 до 0,25); `false` — плоско.
    bool dim3d = true;        // n2D3DFlagTopology
    /// Непрозрачность топологии, % (0 … 100).
    int blend = 100;          // nBlendTopology
    /// Показывать размеры блоков.
    bool sizeBlock = false;   // bTopologySizeFlag
    /// Показывать привязки блоков `x0`, `y0`.
    bool sizeLinkList = false;// bTopologySizeFlagLinkList
    /// @name Показ типов блоков
    ///@{
    /// `true` — показывать диэлектрик, металл, магнитные стенки, поглотитель, входы.
    bool eps = true, metal = true, magnetic = true, absorber = true, input = true;
    ///@}
    /// Показывать контуры плазменных блоков.
    bool plasma = true;       // новое: контуры плазменных блоков X-моды
    /// Цвета материалов: 0 — eps, 1 — металл, 2 — магнитная стенка, 3 — вход, 4 — поглотитель, 5 — плазма.
    QColor colors[6];         // ε, металл, магн., вход, поглотитель, плазма
};

/// @brief Окно данных, оси и размерные линии.
struct PlaceSettings
{
    /// @name Вся область по X и по Y
    ///@{
    /// `true` — окно данных по оси — вся сетка.
    bool xAuto = true, yAuto = true;   // bXsizeFlag, bYsizeFlag
    ///@}
    /// @name Окно данных
    ///@{
    /// Края окна данных, м от узла (0, 0) сетки.
    double xMin = -1, xMax = 1, yMin = -1, yMax = 1;   // окно данных, м (от начала сетки)
    ///@}
    /// Рисовать оси X, Y, Z.
    bool drawAxes = false;             // bDrawFlagAxies
    /// Цвет осей, размерных линий и надписей.
    QColor axesColor;                  // scAxiesColor
    /// Шрифт надписей сцены.
    QFont axesFont;                    // lfAxiesFont
    /// @name Размерные линии окна
    ///@{
    /// Рисовать размерную линию вдоль X и вдоль Y.
    bool drawSizeX = false, drawSizeY = false;   // bDrawFlagAxiesXSize / YSize
    ///@}
    /// Одинаковый масштаб X и Y.
    bool proportional = true;          // bProportionally
};

/// @brief Сохраняемое положение камеры.
struct CameraSettings
{
    /// @name Углы поворота сцены, градусы
    ///@{
    /// Вокруг X, Y, Z.
    double angleX = -55.0, angleY = 0.0, angleZ = -20.0;   // wAngleX/Y/Z, градусы
    ///@}
    /// @name Сдвиг сцены, единицы сцены
    ///@{
    /// По X, Y и приближение по Z от расчётного расстояния камеры.
    double transX = 0.0, transY = 0.0, transZ = 0.0;      // wTranslateX/Y/Z (Z — от расчётного)
    ///@}
    /// Масштаб, 0,05 … 50.
    double zoom = 1.0;                                     // новое: колесо мыши
};

/// @brief Все настройки вида окна документа.
///
/// Каждое окно хранит свою копию; новое окно берёт сохранённые (`load()`), каждая
/// команда сохраняет текущие (`save()`).
/// @see https://www.tamic.ru/docs/code/fieldview.html#settings
struct ViewSettings
{
    /// Слой поля.
    FieldLayerSettings field;
    /// Слой eps.
    EpsLayerSettings eps;
    /// Топология.
    TopologySettings topo;
    /// Окно данных и оси.
    PlaceSettings place;
    /// Камера.
    CameraSettings camera;
    /// Показывать «значение в точке».
    bool probe = false;            // bDrawFieldValueFlag (Field → Value)
    /// @name Точка «значения в точке»
    ///@{
    /// Координаты, м от узла (0, 0) сетки; привязываются к узлу.
    double probeX = 0, probeY = 0; // dDrawFieldValueX/Y, м
    ///@}
    /// Открытая вкладка окна «Параметры» (0 … 3).
    int propertyPage = 0;          // nPropertyPageInd
    /// Скорость проигрывания тактов, кадров в секунду (1 … 60).
    int speed = 10;                // новое: кадров в секунду при проигрывании тактов

    /// @brief Возвращает настройки по умолчанию.
    /// @return Поле — рельеф с заливкой и каркасом, шкала «Авто»; слой eps выключен; топология объёмная.
    static ViewSettings defaults();
    /// @brief Читает сохранённые настройки (нет — по умолчанию; неверные числа заменяются).
    /// @return Настройки.
    static ViewSettings load();
    /// @brief Сохраняет настройки в раздел «TMC_GraphicsOutputFieldFile».
    void save() const;
};

} // namespace fieldview
