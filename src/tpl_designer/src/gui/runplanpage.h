/// @file runplanpage.h
/// @brief Страница мастера «Что считать»: очередь возбуждений портов, варианты геометрии
///        и угловой свип (поворот объекта по шагам).
///
/// Часть интерфейса редактора заданий. Страница правит поля задания `Project::exciters`,
/// `Project::variants` и `Project::sweep`; число шагов `#STEP` считает `Project::stepCount`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-run
#pragma once

#include "project.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
QT_END_NAMESPACE

/// «Что считать»: какие порты возбуждать и какие варианты геометрии перебрать.
///
/// Смысл страницы — один файл вместо четырёх. Каждый `#STEP` в языке самодостаточен,
/// поэтому в одно задание укладываются и оба возбуждения (полная матрица рассеяния
/// за прогон), и несколько вариантов геометрии (пустой волновод и он же с диэлектриком).
/// @brief Страница «Что считать» мастера задания.
///
/// Создаётся главным окном (мастером) и работает с проектом по указателю, не владея им.
/// Перед каждой правкой испускает `aboutToEdit` (снимок для отмены), после — `edited`.
/// Потоки: только поток интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-run
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#sweep
class RunPlanPage : public QWidget
{
    Q_OBJECT
public:
    /// @brief Строит группы «Возбуждение портов», «Поворот объекта по шагам», «Варианты
    ///        геометрии» и строку итога, подключает обработчики.
    /// @param[in] parent  родительский виджет, может быть `nullptr`.
    explicit RunPlanPage(QWidget *parent = nullptr);

    /// @brief Задаёт редактируемое задание и перечитывает из него все списки.
    /// @param[in] p  задание; владелец — вызывающий, указатель должен жить, пока жива страница.
    void setProject(tpl::Project *p);
    /// Перечитать из проекта — геометрия могла измениться на соседней странице.
    /// @brief Перестраивает списки портов, вариантов, фигур, блок свипа и итог по проекту.
    void refresh();

signals:
    /// @brief Испускается перед изменением проекта (вызывающий снимает состояние для отмены).
    void aboutToEdit();
    /// @brief Испускается после изменения проекта.
    void edited();

private:
    /// @brief Перестраивает список входов: строка «S1j, S2j, ... — возбудить вход j», галочка —
    ///        вход в очереди `Project::exciters`.
    void rebuildPorts();
    /// @brief Перестраивает список вариантов: имя, тег файлов и какие фигуры выключены;
    ///        вариант без фигур выделяется цветом опасности.
    void rebuildVariants();
    /// @brief Перестраивает список фигур выбранного варианта (галочка — фигура участвует)
    ///        и поля имени и тега варианта.
    void rebuildShapes();
    /// @brief Переносит отметки списка входов в `Project::exciters` и обновляет итог.
    void pullPorts();
    /// @brief Переносит отметки списка фигур в `Variant::disabledShapes` выбранного варианта.
    void pullVariantShapes();
    /// @brief Обновляет итог: число шагов расчёта, ориентировочное время (14 с на шаг при сетке
    ///        240x80 и шаге 1 мм) и подсказку о том, какие элементы S-матрицы получатся.
    void updateSummary();
    /// @brief Перестраивает блок углового свипа из `Project::sweep`: фигуры объекта, ось
    ///        (`tpl::sweepPivot`), углы, опорный шаг и подсказку о том, что мешает записи.
    void rebuildSweep();
    /// @brief Переносит поля блока свипа в `Project::sweep`.
    ///
    /// Углы — от «от» до «до» с шагом «шаг» (градусы), округлённые до 0.001, не больше 400.
    /// При снятии галочки «в центре объекта» ось фиксируется в текущем центре объекта.
    void pullSweep();
    /// @brief Возвращает номер выбранного варианта.
    /// @return Индекс в `Project::variants`; -1 — вариант не выбран или проекта нет.
    int currentVariant() const;

    /// Редактируемое задание (не владеет).
    tpl::Project *m_project = nullptr;
    /// true — идёт программное заполнение полей; обработчики правок молчат.
    bool m_loading = false;

    /// Список входов с галочками возбуждения; в данных строки — `Port::id`.
    QListWidget *m_ports = nullptr;
    /// Подсказка: какие элементы S-матрицы получатся при выбранной очереди.
    QLabel *m_portsHint = nullptr;

    /// Список вариантов геометрии.
    QListWidget *m_variants = nullptr;
    /// Название выбранного варианта.
    QLineEdit *m_variantName = nullptr;
    /// Тег файлов выбранного варианта (входит в имена файлов результатов).
    QLineEdit *m_variantTag = nullptr;
    /// Фигуры выбранного варианта с галочками участия; в данных строки — `Shape::id`.
    QListWidget *m_shapes = nullptr;
    /// Кнопка «Добавить» вариант.
    QPushButton *m_add = nullptr;
    /// Кнопка «Удалить» выбранный вариант.
    QPushButton *m_remove = nullptr;
    /// Кнопка заготовки: варианты «Пустой» (без заливок) и «С заполнением».
    QPushButton *m_preset = nullptr;
    /// Подсказка под списками вариантов.
    QLabel *m_variantHint = nullptr;

    /// Итог: число шагов и ориентировочное время счёта.
    QLabel *m_summary = nullptr;

    // --- поворот объекта по шагам (угловой свип) ---
    /// Группа «Возбуждение портов».
    QGroupBox *m_excGroup = nullptr;
    /// Группа «Варианты геометрии» (недоступна при включённом свипе).
    QGroupBox *m_varGroup = nullptr;
    /// Галочка «Поворачивать объект».
    QCheckBox *m_sweepOn = nullptr;
    /// Фигуры объекта свипа с галочками; в данных строки — `Shape::id`.
    QListWidget *m_sweepShapes = nullptr;
    /// Ось поворота X, мм (правится, только если ось не привязана к центру объекта).
    QDoubleSpinBox *m_pivotX = nullptr;
    /// Ось поворота Y, мм.
    QDoubleSpinBox *m_pivotY = nullptr;
    /// Галочка «ось в центре объекта» (`AngleSweep::pivotAtCenter`).
    QCheckBox *m_pivotCenter = nullptr;
    /// Начальный угол, градусы.
    QDoubleSpinBox *m_angFrom = nullptr;
    /// Конечный угол, градусы.
    QDoubleSpinBox *m_angTo = nullptr;
    /// Шаг по углу, градусы.
    QDoubleSpinBox *m_angStep = nullptr;
    /// Галочка опорного шага без объекта (`AngleSweep::referenceStep`).
    QCheckBox *m_sweepRef = nullptr;
    /// Подсказка свипа: число шагов, кто поворачивается, что исправить для записи.
    QLabel *m_sweepHint = nullptr;
};
