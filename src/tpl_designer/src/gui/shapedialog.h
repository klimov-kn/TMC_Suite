/// @file shapedialog.h
/// @brief Окно «Свойства фигуры»: полная форма фигуры (тип, материал, eps, профиль, плазма, размеры, вершины).
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса. Открывается кнопкой
/// «Все свойства...» панели свойств и двойным щелчком (через `MainWindow`); правит копию
/// фигуры и возвращает её `shape()`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-dialogs
#pragma once

#include "project.h"

#include <QDialog>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QStackedWidget;
class QTableWidget;
QT_END_NAMESPACE

/// Свойства фигуры. Пользователь вводит физику: «металл», «eps = 4», «концентрация».
/// Во что это превратится в файле — забота эмиттера.
/// @brief Модальный диалог полной формы фигуры; работает с копией `tpl::Shape`.
///
/// Создаётся мастером на стеке; при принятии мастер заменяет фигуру проекта результатом
/// `shape()`. Формула eps проверяется прямо при наборе (`tpl::ExprProgram`).
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-dialogs
class ShapeDialog : public QDialog
{
    Q_OBJECT
public:
    /// @brief Строит окно и заполняет его из фигуры.
    /// @param[in] s       фигура (копируется).
    /// @param[in] kernel  ядро задания; группа плазмы доступна только для X.
    /// @param[in] parent  родительское окно; `nullptr` — без родителя.
    ShapeDialog(const tpl::Shape &s, tpl::Kernel kernel, QWidget *parent = nullptr);

    /// @brief Собирает фигуру из полей окна (поверх исходной копии).
    /// @return Отредактированная фигура; углы сектора — в радианах, координаты — в мм.
    tpl::Shape shape() const;

private slots:
    /// @brief Переключает страницу размеров (прямоугольник, круг, вершины) под выбранный тип фигуры.
    void onKindChanged();
    /// @brief Обновляет доступность полей eps, формулы, файла, начала отсчёта и подсказку под материалом.
    void onMaterialChanged();

private:
    /// @brief Переносит значения фигуры в поля окна.
    /// @param[in] s  фигура.
    void load(const tpl::Shape &s);
    /// Пересчитать подсказку под полем формулы: разбор, диапазон, имя файла.
    void updateFormulaHint();

    /// Исходная копия фигуры; поля, которых нет в окне, переходят в результат как есть.
    tpl::Shape m_shape;
    /// Ядро задания (H или X).
    tpl::Kernel m_kernel;

    /// Название фигуры.
    QLineEdit *m_name = nullptr;
    /// Тип фигуры (`tpl::ShapeKind`).
    QComboBox *m_kind = nullptr;
    /// Материал (`tpl::Material`).
    QComboBox *m_material = nullptr;
    /// Проницаемость eps (сама eps, не добавка).
    QDoubleSpinBox *m_eps = nullptr;
    /// Подсказка: что уйдёт в файл (добавка eps-1, контур или заливка).
    QLabel *m_epsHint = nullptr;
    // Неоднородная eps: формула вместо числа. Ядро считает её в каждом узле блока.
    /// «Задать eps формулой (неоднородная среда)».
    QCheckBox *m_epsFormulaOn = nullptr;
    /// Профиль не считается, а читается из готового `.eps` — блок `FILE` в задании.
    QCheckBox *m_epsFromFile = nullptr;
    /// Формула eps от `x`, `y`, `r`, `f` (полярный угол).
    QLineEdit *m_epsFormula = nullptr;
    /// Имя файла `.eps` без расширения: куда сохранить профиль или откуда его прочитать.
    QLineEdit *m_epsFile = nullptr;
    /// Подпись поля имени файла: у чтения и у записи она разная по смыслу.
    QLabel *m_epsFileLabel = nullptr;
    /// «Отсчитывать x и y не от центра фигуры» (`EpsProfile::customOrigin`).
    QCheckBox *m_epsOriginOn = nullptr;
    /// Строка с полями начала отсчёта.
    QWidget *m_epsOriginRow = nullptr;
    /// Начало отсчёта формулы по X, мм.
    QDoubleSpinBox *m_epsOriginX = nullptr;
    /// Начало отсчёта формулы по Y, мм.
    QDoubleSpinBox *m_epsOriginY = nullptr;
    /// Итог проверки формулы: ошибка разбора, диапазон eps, имя файла.
    QLabel *m_epsFormulaHint = nullptr;
    // Плазма: N, B и Y в ОДНОЙ фигуре — замагниченная плазма это N вместе с B.
    /// Флажок концентрации N, м^-3.
    QCheckBox *m_plasmaN = nullptr;
    /// Значение или формула N, м^-3.
    QLineEdit *m_plasmaNValue = nullptr;
    /// Файл распределения N (блок `FILE_N`, только прямоугольник).
    QLineEdit *m_plasmaNFile = nullptr;
    /// Флажок подмагничивания B, Тл.
    QCheckBox *m_plasmaB = nullptr;
    /// Значение или формула B, Тл.
    QLineEdit *m_plasmaBValue = nullptr;
    /// Файл распределения B (блок `FILE_B`).
    QLineEdit *m_plasmaBFile = nullptr;
    /// «B ровно по плазме (без запаса)» (`Plasma::bExact`).
    QCheckBox *m_plasmaBExact = nullptr;
    /// Флажок потерь через проводимость sigma, См/м.
    QCheckBox *m_plasmaS = nullptr;
    /// Значение или формула sigma, См/м.
    QLineEdit *m_plasmaSValue = nullptr;
    /// Флажок сеточного параметра Y (безразмерный).
    QCheckBox *m_plasmaY = nullptr;
    /// Значение или формула Y.
    QLineEdit *m_plasmaYValue = nullptr;
    /// Файл распределения Y (блок `FILE_Y`).
    QLineEdit *m_plasmaYFile = nullptr;
    /// Пояснение о совместном действии N и B.
    QLabel *m_plasmaHint = nullptr;

    /// Страницы размеров: прямоугольник, круг/сектор, таблица вершин.
    QStackedWidget *m_geom = nullptr;
    /// @name Поля размеров (мм; углы сектора — градусы)
    ///@{
    /// Границы прямоугольника.
    QDoubleSpinBox *m_xMin = nullptr;
    /// @copydoc m_xMin
    QDoubleSpinBox *m_xMax = nullptr;
    /// @copydoc m_xMin
    QDoubleSpinBox *m_yMin = nullptr;
    /// @copydoc m_xMin
    QDoubleSpinBox *m_yMax = nullptr;
    /// Центр круга (сектора).
    QDoubleSpinBox *m_cx = nullptr;
    /// @copydoc m_cx
    QDoubleSpinBox *m_cy = nullptr;
    /// Внутренний радиус (кольцо) и внешний радиус.
    QDoubleSpinBox *m_rMin = nullptr;
    /// @copydoc m_rMin
    QDoubleSpinBox *m_rMax = nullptr;
    /// Начальный и конечный угол сектора, градусы.
    QDoubleSpinBox *m_fiMin = nullptr;
    /// @copydoc m_fiMin
    QDoubleSpinBox *m_fiMax = nullptr;
    ///@}
    /// Таблица вершин ломаной или контура (X, Y в мм).
    QTableWidget *m_points = nullptr;
    /// Пояснение о замкнутости ломаной и контура.
    QLabel *m_polyHint = nullptr;
    /// Пояснение, где правятся отдельные стороны прямоугольника.
    QLabel *m_edgeHint = nullptr;

    /// Скорость блока по X (`Shape::vx`); 0 — блок неподвижен.
    QDoubleSpinBox *m_vx = nullptr;
    /// Скорость блока по Y (`Shape::vy`).
    QDoubleSpinBox *m_vy = nullptr;
};
