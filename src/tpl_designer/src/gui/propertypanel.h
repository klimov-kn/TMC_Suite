/// @file propertypanel.h
/// @brief Панель свойств выделенного объекта (фигуры, стороны, вершины, порта) в мастере.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса. Правит `tpl::Project`
/// напрямую по указателю, без модального окна; полная форма — `ShapeDialog`/`PortDialog`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-properties
#pragma once

#include "geometryview.h"
#include "project.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPushButton;
QT_END_NAMESPACE

/// Свойства выделенного объекта прямо в окне, без модального диалога.
///
/// Так правка перестаёт быть «открыть окно — поправить — закрыть»: поменял поле —
/// увидел на холсте. Модальный диалог остаётся только для полной формы (сигнал порта,
/// список вершин), куда лезут редко.
/// @brief Немодальная форма свойств выделенного: видимы только строки, относящиеся к виду объекта.
///
/// Создаётся мастером (`MainWindow`) на странице «Геометрия», проект получает через
/// `setProject` (не владеет им), выделение — через `showSelection` от `GeometryView`.
/// Каждая правка поля сразу записывается в проект: сначала сигнал `aboutToEdit`
/// (владелец снимает снимок для отмены), затем изменение, затем `edited`.
/// Работает в потоке интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-properties
class PropertyPanel : public QWidget
{
    Q_OBJECT
public:
    /// Доступность «Соединить» решает окно: панель не знает, сколько строк выбрано
    /// в списке объектов и что за фигуры там лежат.
    /// @brief Включает или выключает кнопку «Соединить» и ставит ей подсказку.
    /// @param[in] on   true — кнопка доступна.
    /// @param[in] why  подсказка (почему недоступна); пусто — стандартное пояснение про Ctrl+клик.
    void setMergeEnabled(bool on, const QString &why = {});

    /// @brief Строит форму и показывает пустое выделение («Ничего не выбрано»).
    /// @param[in] parent  родительский виджет; `nullptr` — без родителя.
    explicit PropertyPanel(QWidget *parent = nullptr);

    /// @brief Задаёт проект, который панель правит, и обновляет поля.
    /// @param[in] p  проект мастера; панель им не владеет; `nullptr` — правка выключена.
    void setProject(tpl::Project *p);
    /// Показать выделение. Если объект тот же — просто обновляет числа,
    /// не сбивая набранный текст.
    /// @brief Показывает свойства выделенного объекта.
    /// @param[in] s  выделение холста: фигура, сторона прямоугольника, вершина или порт; пустое — подсказка.
    void showSelection(const Selection &s);

signals:
    /// Поднимается ДО правки: владелец снимает состояние для отмены.
    void aboutToEdit();
    /// Правка из панели записана в проект — холст и проверку пора обновить.
    void edited();
    /// Нажата «Все свойства...»: открыть полную форму объекта.
    /// @param id  номер фигуры или порта (`Selection::id`).
    void fullPropertiesRequested(int id);
    /// Нажата «Скруглить»: скруглить углы выделенного прямоугольника или вершину ломаной.
    /// @param radius  радиус скругления, мм.
    void filletRequested(double radius);
    /// Нажата «Дублировать»: копировать выделенное.
    void duplicateRequested();
    /// Нажата «Удалить»: удалить выделенное.
    void deleteRequested();
    /// Сшить выделенные фигуры в одну. Живёт в этом ряду, а не под списком объектов:
    /// «соединить», «дублировать» и «удалить» — одного рода команды над выделенным.
    void mergeRequested();

private:
    /// @brief Создаёт все строки формы (видимость решает `updateDerived`) и связывает поля с `pushToProject`.
    void build();
    /// @brief Переносит значения выделенного объекта из проекта в поля.
    ///
    /// Во время собственной записи (`m_pushing`) поля не переписывает — только `updateDerived`,
    /// чтобы не сбить набираемое число.
    void syncFromProject();
    /// Только заголовки, подсказки и видимость строк. Значения полей НЕ трогает:
    /// иначе курсор прыгает прямо во время набора числа.
    /// @brief Обновляет видимость строк, заголовок, подсказки и доступность кнопок.
    ///
    /// Плазменные строки — только в X-ядре; файлы `FILE_N/_B/_Y` — только у прямоугольника;
    /// под портом — одномодовый диапазон апертуры `c/(2a)...c/a`, ГГц; под плазмой — `f_p`,
    /// `f_ce`, `f_L`, `f_R` (`tpl::plasmaBands`).
    void updateDerived();
    /// @brief Записывает значения полей в выделенный объект проекта и испускает `aboutToEdit`, `edited`.
    ///
    /// У стороны — наличие и свой материал; у фигуры — имя, материал, eps, плазму, размеры
    /// (ломаная с заливкой молча замыкается в контур); у порта — имя, ось и плоскость,
    /// апертуру, роль и тип волны. Направление порта приводится в согласие с половиной
    /// области (`tpl::directionForPlane`), иначе ядро молча развернуло бы волну.
    void pushToProject();

    /// @brief Находит выделенную фигуру в проекте.
    /// @return Указатель на фигуру в `m_project->shapes`; `nullptr` — выделен порт, ничего или проекта нет.
    ///         Действителен до изменения состава фигур.
    tpl::Shape *shape();
    /// @brief Находит выделенный порт в проекте.
    /// @return Указатель на порт в `m_project->ports`; `nullptr` — выделен не порт или проекта нет.
    tpl::Port *port();

    /// Править проект мастера (не владеет).
    tpl::Project *m_project = nullptr;
    /// Текущее выделение.
    Selection m_sel;
    /// true — идёт `syncFromProject`; сигналы полей не записываются в проект.
    bool m_loading = false;
    /// true — идёт `pushToProject`; повторный вход и перезапись полей запрещены.
    bool m_pushing = false;

    /// Заголовок: имя и вид объекта.
    QLabel *m_title = nullptr;
    /// Подсказка о виде объекта (контур или заливка, порт, пустое выделение).
    QLabel *m_kindHint = nullptr;
    /// Форма со строками свойств.
    QFormLayout *m_form = nullptr;

    /// Название фигуры или порта.
    QLineEdit *m_name = nullptr;
    /// Материал фигуры: металл, магнитная стенка, поглотитель, диэлектрик (заливка).
    QComboBox *m_material = nullptr;
    /// Проницаемость eps диэлектрика (сама eps; в файл уйдёт eps-1).
    QDoubleSpinBox *m_eps = nullptr;
    /// Подсказка под eps: добавка eps-1 или смысл контурного материала.
    QLabel *m_epsHint = nullptr;
    // Плазма: три независимые величины в ОДНОЙ фигуре. Раньше был один список на
    // выбор, и замагниченную плазму приходилось собирать из трёх фигур друг на друге.
    /// Флажок «Концентрация N, м^-3».
    QCheckBox *m_plasmaN = nullptr;
    /// Значение N: число или формула от координат.
    QLineEdit *m_plasmaNValue = nullptr;
    /// Имя файла распределения N (без `.eps`), блок `FILE_N`.
    QLineEdit *m_plasmaNFile = nullptr;
    /// Строка «N из файла» (поле и кнопка «Обзор...»).
    QWidget *m_plasmaNFileRow = nullptr;
    /// Флажок «Подмагничивание B, Тл».
    QCheckBox *m_plasmaB = nullptr;
    /// Значение B: число или формула.
    QLineEdit *m_plasmaBValue = nullptr;
    /// Имя файла распределения B, блок `FILE_B`.
    QLineEdit *m_plasmaBFile = nullptr;
    QCheckBox *m_plasmaBExact = nullptr; ///< B ровно по плазме, без запаса
    /// Строка «B из файла».
    QWidget *m_plasmaBFileRow = nullptr;
    // Потери: σ в См/м предлагаем первыми, сеточное Y — опытным (Б-20).
    /// Флажок «Потери: проводимость sigma, См/м».
    QCheckBox *m_plasmaS = nullptr;
    /// Значение sigma, См/м: число или формула.
    QLineEdit *m_plasmaSValue = nullptr;
    /// Флажок «Параметр Y» (сеточная величина потерь).
    QCheckBox *m_plasmaY = nullptr;
    /// Значение Y, безразмерное: число или формула.
    QLineEdit *m_plasmaYValue = nullptr;
    /// Имя файла распределения Y, блок `FILE_Y`.
    QLineEdit *m_plasmaYFile = nullptr;
    /// Строка «Y из файла».
    QWidget *m_plasmaYFileRow = nullptr;
    /// Подсказка под плазмой: частоты `f_p`, `f_ce`, отсечки X-моды или пояснение к формуле.
    QLabel *m_plasmaHint = nullptr;

    /// Левая граница прямоугольника по X, мм.
    QDoubleSpinBox *m_xMin = nullptr;
    /// Правая граница прямоугольника по X, мм.
    QDoubleSpinBox *m_xMax = nullptr;
    /// Нижняя граница прямоугольника по Y, мм.
    QDoubleSpinBox *m_yMin = nullptr;
    /// Верхняя граница прямоугольника по Y, мм.
    QDoubleSpinBox *m_yMax = nullptr;

    /// Центр круга по X, мм.
    QDoubleSpinBox *m_cx = nullptr;
    /// Центр круга по Y, мм.
    QDoubleSpinBox *m_cy = nullptr;
    /// Внутренний радиус, мм.
    QDoubleSpinBox *m_rMin = nullptr;
    /// Внешний радиус, мм.
    QDoubleSpinBox *m_rMax = nullptr;
    /// Начальный угол сектора, градусы (в модели — радианы).
    QDoubleSpinBox *m_fiMin = nullptr;
    /// Конечный угол сектора, градусы (в модели — радианы).
    QDoubleSpinBox *m_fiMax = nullptr;

    /// Число вершин контура или линии.
    QLabel *m_points = nullptr;

    /// Флажок «Сторона существует» (`EdgeSpec::present`) выделенной стороны прямоугольника.
    QCheckBox *m_edgePresent = nullptr;
    /// Флажок «Свой материал у стороны» (`EdgeSpec::custom`).
    QCheckBox *m_edgeCustom = nullptr;
    /// Материал стороны: металл, магнитная стенка, поглотитель.
    QComboBox *m_edgeMaterial = nullptr;

    /// Направление падающей волны порта (`PortSide`): ось и знак.
    QComboBox *m_portSide = nullptr;
    /// Координата плоскости входа поперёк волны, мм.
    QDoubleSpinBox *m_portPlane = nullptr;
    /// Начало апертуры порта, мм.
    QDoubleSpinBox *m_portFrom = nullptr;
    /// Конец апертуры порта, мм.
    QDoubleSpinBox *m_portTo = nullptr;
    /// Роль порта: активный (излучает) или пассивный (принимает).
    QComboBox *m_portActive = nullptr;
    /// Тип волны: H10 или плоская.
    QComboBox *m_portMode = nullptr;
    /// Подсказка: одномодовый диапазон апертуры, ГГц.
    QLabel *m_portCutoff = nullptr;

    /// Радиус скругления, мм.
    QDoubleSpinBox *m_filletR = nullptr;
    /// Кнопка «Скруглить».
    QPushButton *m_fillet = nullptr;
    /// Кнопка «Все свойства...».
    QPushButton *m_full = nullptr;
    /// Кнопка «Соединить» (доступность решает окно, `setMergeEnabled`).
    QPushButton *m_merge = nullptr;
    /// Кнопка «Дублировать».
    QPushButton *m_dup = nullptr;
    /// Кнопка «Удалить».
    QPushButton *m_del = nullptr;
};
