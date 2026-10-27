/// @file import3ddialog.h
/// @brief Окно «Срез 3D-модели»: чтение STL/OBJ/STEP, выбор плоскости среза и материалов тел.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса. Открывается мастером
/// (`MainWindow`); чтение, срез, упрощение и построение фигур делает ядро генератора
/// (`readMeshFile`, `sliceMesh`, `simplifySlice`, `buildImportShapes`).
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-mesh
#pragma once

#include "meshimport.h"
#include "project.h"

#include <QDialog>

QT_BEGIN_NAMESPACE
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QSlider;
class QStackedWidget;
class QTableWidget;
QT_END_NAMESPACE

class GeometryView;
class MeshView;

/// Окно «Срез 3D-модели».
///
/// Человек выбирает плоскость (XY, YZ, XZ — щелчком по ней в 3D или кнопкой), двигает
/// её ползунком вдоль нормали и в любой момент переключается между самой моделью и
/// предпросмотром ТОГО, ЧТО ВСТАВИТСЯ: предпросмотр рисует тот же холст, что и
/// редактор, поверх текущего задания — картинка не может разойтись с результатом.
/// Материалы тел узнаются по именам из файла; не узнанные выбирает человек, и выбор
/// запоминается.
/// @brief Модальное окно импорта 3D: модель, плоскость, материалы тел, предпросмотр вставки.
///
/// Создаётся мастером на стеке с копией текущего задания; результат (`result`,
/// `resultBox`, `provenance`) мастер забирает после принятия окна. Выбор материалов
/// запоминается в `QSettings` по имени материала (или тела) из файла.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-mesh
class Import3DDialog : public QDialog
{
    Q_OBJECT
public:
    /// \p host — текущее задание: из него берутся шаг сетки (допуск упрощения),
    /// счётная область (куда ставить срез) и свободные номера фигур.
    /// @brief Строит окно; модель ещё не прочитана (см. `loadFile`).
    /// @param[in] host    текущее задание (копируется).
    /// @param[in] parent  родительское окно; `nullptr` — без родителя.
    Import3DDialog(const tpl::Project &host, QWidget *parent = nullptr);

    /// Прочитать файл. false — не прочитался (\p error объясняет).
    /// @brief Читает модель (`tpl::readMeshFile`), переводит в мм, заполняет таблицу тел и вид.
    /// @param[in]  path   путь к файлу `.stl`, `.obj` или `.step`/`.stp`.
    /// @param[out] error  причина отказа; может быть `nullptr`.
    /// @return true — файл прочитан; предупреждения чтения выводятся в окне.
    bool loadFile(const QString &path, QString *error = nullptr);

    /// Готовые фигуры — то, что показывает предпросмотр.
    /// @return Фигуры среза с новыми номерами блоков и замечания построения.
    tpl::ImportShapes result() const { return m_shapes; }
    /// Область, расширенная под срез (если стоит галочка и срез не помещается).
    /// @return Счётная область задания после возможного расширения, мм.
    tpl::Box resultBox() const { return m_fitBox; }
    /// @brief Сообщает, расширена ли счётная область под срез.
    /// @return true — `resultBox()` отличается от области исходного задания.
    bool boxChanged() const;
    /// Строка в описание задания: откуда геометрия.
    /// @return Текст `tpl::sliceProvenance`: имя файла, плоскость и положение среза.
    QString provenance() const;

    // Для проверок без мыши.
    /// @brief Выбирает плоскость среза (как кнопка или щелчок в 3D) и пересчитывает срез.
    /// @param[in] p  плоскость; при смене диапазон положения сбрасывается на середину модели.
    void choosePlane(tpl::SlicePlane p);
    /// @brief Переключает между 3D-моделью и предпросмотром того, что вставится.
    /// @param[in] on  true — предпросмотр (холст `GeometryView` с подгонкой масштаба).
    void showPreview(bool on);

private:
    /// @brief Строит разметку: режимы вида, кнопки плоскостей, ползунок, таблица тел, параметры размещения.
    void buildUi();
    /// @brief Переводит модель из единиц файла в мм, склеивает вершины и обновляет всё окно.
    void rescale();
    /// @brief Заполняет таблицу тел: материал из запомненного выбора, иначе `tpl::guessMaterial` по имени.
    void fillBodies();
    /// @brief Пересчитывает срез, упрощение (допуск — половина шага сетки), фигуры, область и предпросмотр.
    void updateSlice();
    /// @brief Ставит диапазон положения плоскости по габаритам модели вдоль нормали и середину.
    void updateRange();
    /// @brief Запоминает выбор материала тела в `QSettings` (ключ `import3d/materials/<имя>`).
    /// @param[in] body  номер тела (строки таблицы).
    void rememberChoice(int body);
    /// @brief Собирает выбор материалов из таблицы тел.
    /// @return Выбор по каждому телу: вставлять ли, материал, eps.
    QVector<tpl::BodyChoice> choices() const;
    /// @brief Возвращает цвета тел для 3D-вида.
    /// @return Цвет из файла (если есть) или цвет материала; «не вставлять» — полупрозрачный.
    QVector<QColor> bodyColors() const;

    /// Копия текущего задания: шаг сетки, область, свободные номера.
    tpl::Project m_host;
    /// Задание для предпросмотра: `m_host` плюс фигуры среза.
    tpl::Project m_previewProject;
    /// Путь к прочитанному файлу модели.
    QString m_path;
    tpl::Mesh m_raw;   ///< как прочитано, в единицах файла
    tpl::Mesh m_mesh;  ///< в мм, склеенная
    /// Миллиметров в единице файла, если файл её хранит (STEP); 0 — выбирает человек.
    double m_fileUnitMm = 0.0;
    /// Выбранная плоскость среза.
    tpl::SlicePlane m_plane = tpl::SlicePlane::XY;
    /// true — плоскость выбрана; до этого срез не строится.
    bool m_planeChosen = false;
    /// Габарит модели вдоль нормали плоскости, мм (диапазон ползунка).
    double m_lo = 0.0, m_hi = 1.0;
    /// Срез модели до упрощения.
    tpl::SliceResult m_slice;
    /// Готовые фигуры для вставки.
    tpl::ImportShapes m_shapes;
    /// Счётная область с учётом расширения под срез, мм.
    tpl::Box m_fitBox;
    /// Поворот среза в задании, четверти оборота (кнопка «Повернуть на 90°»).
    int m_rot90 = 0;
    /// true — идёт программное заполнение полей; реакции на изменения подавлены.
    bool m_filling = false;

    /// Строка о файле: имя, число тел и треугольников, габарит.
    QLabel *m_fileInfo = nullptr;
    /// Переключатель «3D-модель» / «Что вставится в задание».
    QButtonGroup *m_modeGroup = nullptr;
    /// Стопка из 3D-вида и предпросмотра.
    QStackedWidget *m_stack = nullptr;
    /// 3D-вид модели.
    MeshView *m_view3d = nullptr;
    /// Предпросмотр вставки — тот же холст, что в редакторе.
    GeometryView *m_preview = nullptr;
    /// Кнопки плоскостей XY, YZ, XZ (номер кнопки = `tpl::SlicePlane`).
    QButtonGroup *m_planeGroup = nullptr;
    /// Подпись оси нормали («Z =» и т. п.).
    QLabel *m_axisLabel = nullptr;
    /// Ползунок положения плоскости, 0...1000 по габариту.
    QSlider *m_slider = nullptr;
    /// Точное положение плоскости, мм.
    QDoubleSpinBox *m_offset = nullptr;
    /// Сводка среза: задетые тела, контуры, фигуры.
    QLabel *m_sliceInfo = nullptr;
    /// Таблица тел: имя, материал в файле, материал в задании, eps.
    QTableWidget *m_bodies = nullptr;
    /// Единицы файла (для STL и OBJ).
    QComboBox *m_units = nullptr;
    /// «Поставить срез в центр счётной области».
    QCheckBox *m_center = nullptr;
    /// «Отразить зеркально».
    QCheckBox *m_mirror = nullptr;
    /// «Расширить счётную область, если срез не помещается».
    QCheckBox *m_fit = nullptr;
    /// Замечания чтения и построения (жёлтым).
    QLabel *m_notes = nullptr;
    /// Кнопка «Применить — вставить в задание»; доступна, когда есть фигуры.
    QPushButton *m_apply = nullptr;
};
/// @var Import3DDialog::m_hi
/// Верхняя граница габарита модели вдоль нормали плоскости, мм.
