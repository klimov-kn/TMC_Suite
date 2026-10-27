/// @file meshview.h
/// @brief 3D-вид принесённой модели в окне импорта: тела, три плоскости среза и линия среза.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса. Используется окном
/// `Import3DDialog`; модель и срез готовит ядро генератора (`meshslice.h`).
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-mesh
#pragma once

#include "meshslice.h"

#include <QColor>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QVector3D>

class QPainter;

/// 3D-вид модели для окна импорта: сама модель, три плоскости XY / YZ / XZ и линия
/// среза на выбранной.
///
/// OpenGL Widgets, а не Qt Quick 3D: Quick 3D есть только в поставке Qt для Windows, а
/// программа собирается и на Linux, и на macOS (Qt 6.5.3 в WSL и в кросс-наборе osxcross),
/// где его нет. Шейдеры — без `#version`, то есть GLSL 1.10: так они работают и в
/// совместимом профиле Windows/Linux, и в OpenGL 2.1 macOS, и в программном
/// `opengl32sw.dll` на машинах без видеодрайвера.
///
/// Мышь как на холсте: ЛКМ — повернуть, колесо — масштаб, средняя кнопка или Alt+ЛКМ —
/// сдвиг. Щелчок ЛКМ без протяжки по плоскости — выбрать её.
/// @brief Виджет OpenGL: модель тел, плоскости XY/YZ/XZ и контуры среза поверх.
///
/// Создаётся окном импорта и принадлежит ему. Модель не копирует: хранит указатель на
/// `tpl::Mesh` окна, который должен жить, пока вид его показывает. Работает в потоке
/// интерфейса; ресурсы OpenGL создаются в `initializeGL` и освобождаются в деструкторе.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-mesh
class MeshView : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    /// @brief Создаёт вид (минимум 360x300, 4x сглаживание, буферы глубины и трафарета) и прозрачный слой подписей.
    /// @param[in] parent  родительский виджет; `nullptr` — без родителя.
    explicit MeshView(QWidget *parent = nullptr);
    /// @brief Освобождает буфер вершин OpenGL.
    ~MeshView() override;

    /// Модель и цвет каждого тела. Тело с прозрачностью < 255 рисуется полупрозрачным
    /// (так окно показывает «не вставлять»).
    /// @brief Задаёт модель, сбрасывает выбор плоскости, срез и камеру.
    /// @param[in] mesh        модель; не копируется, должна жить, пока её показывают; `nullptr` — пусто.
    /// @param[in] bodyColors  цвет каждого тела по порядку `mesh->bodies`; недостающие — серые.
    void setMesh(const tpl::Mesh *mesh, const QVector<QColor> &bodyColors);
    /// @brief Меняет цвета тел (например, когда тело отмечено «не вставлять») и перерисовывает.
    /// @param[in] bodyColors  цвет каждого тела; прозрачность меньше 255 — тело полупрозрачное.
    void setBodyColors(const QVector<QColor> &bodyColors);

    /// Выбранная плоскость и её положение вдоль нормали, мм.
    /// @brief Отмечает плоскость выбранной: она рисуется ярко на своём месте, остальные бледно.
    /// @param[in] p       плоскость среза.
    /// @param[in] offset  положение плоскости вдоль её нормали, мм (в координатах модели).
    void setPlane(tpl::SlicePlane p, double offset);
    /// Контуры среза — рисуются на плоскости ярко, поверх модели.
    /// @param[in] r  результат среза `tpl::sliceMesh`: контуры по телам в осях плоскости, мм.
    void setSection(const tpl::SliceResult &r);

    /// @brief Ставит камеру на центр модели на расстоянии 1.9 её диагонали, углы -35° и 25°.
    void resetCamera();

    /// @brief Возвращает цвет плоскости среза (тот же — на кнопках окна импорта).
    /// @param[in] p  плоскость.
    /// @return XY — голубой, YZ — красный, XZ — зелёный.
    static QColor planeColor(tpl::SlicePlane p);

    /// Рамки, подписи плоскостей и линия среза — рисует прозрачный слой поверх.
    /// @param[in,out] painter  рисовальщик прозрачного слоя поверх вида.
    void paintOverlay(QPainter &p);

signals:
    /// Щелчок без протяжки попал в плоскость (ближайшую к глазу).
    /// @param p  плоскость под курсором.
    void planeClicked(tpl::SlicePlane p);

protected:
    /// @brief Компилирует шейдеры (GLSL 1.10) и создаёт буфер вершин.
    void initializeGL() override;
    /// @brief Растягивает прозрачный слой подписей на весь вид.
    /// @param[in] w  ширина вида, пиксели (не используется).
    /// @param[in] h  высота вида, пиксели (не используется).
    void resizeGL(int w, int h) override;
    /// @brief Рисует непрозрачные тела, затем полупрозрачные и полупрозрачные плоскости среза.
    void paintGL() override;
    /// @brief Запоминает точку нажатия; средняя кнопка или Alt+ЛКМ включают сдвиг.
    /// @param[in] e  событие мыши.
    void mousePressEvent(QMouseEvent *e) override;
    /// @brief Поворачивает камеру (0.4° на пиксель, наклон в пределах ±89°) или сдвигает точку обзора.
    /// @param[in] e  событие мыши.
    void mouseMoveEvent(QMouseEvent *e) override;
    /// @brief Щелчок ЛКМ без протяжки: ищет плоскость под курсором и испускает `planeClicked`.
    /// @param[in] e  событие мыши.
    void mouseReleaseEvent(QMouseEvent *e) override;
    /// @brief Приближает и отдаляет камеру (множитель 0.87 на шаг колеса, от 0.05 до 20 диагоналей).
    /// @param[in] e  событие колеса.
    void wheelEvent(QWheelEvent *e) override;

private:
    /// @brief Переносит треугольники модели в буфер вершин: непрозрачные тела первыми, полупрозрачные следом.
    void uploadMesh();
    /// Перерисовать и 3D, и слой поверх.
    void refresh();
    /// @brief Матрица вида: орбита вокруг `m_target`, ось Z модели — вверх экрана.
    /// @return Матрица вида камеры.
    QMatrix4x4 viewMatrix() const;
    /// @brief Матрица перспективы (угол 35°), ближняя и дальняя грани — по диагонали модели.
    /// @return Матрица проекции.
    QMatrix4x4 projMatrix() const;
    /// Прямоугольник плоскости в 3D: четыре угла.
    /// @param[in] p       плоскость.
    /// @param[in] offset  положение вдоль нормали, мм.
    /// @return Углы прямоугольника по габаритам модели с запасом 8 % диагонали.
    std::array<QVector3D, 4> planeQuad(tpl::SlicePlane p, double offset) const;
    /// @brief Возвращает положение, в котором плоскость рисуется.
    /// @param[in] p  плоскость.
    /// @return `m_offset` для выбранной плоскости, иначе координата центра модели по нормали, мм.
    double planeOffsetShown(tpl::SlicePlane p) const;
    /// @brief Проецирует точку модели на экран.
    /// @param[in] v  точка модели, мм.
    /// @return Координаты в пикселях вида; (-1e6, -1e6) при вырожденной проекции.
    QPointF project(const QVector3D &v) const;

    /// Прозрачный слой поверх вида, на котором рисует `paintOverlay`.
    QWidget *m_overlay = nullptr;
    /// Показываемая модель (не владеет); `nullptr` — пусто.
    const tpl::Mesh *m_mesh = nullptr;
    /// Цвета тел по порядку `m_mesh->bodies`.
    QVector<QColor> m_colors;
    /// Габариты модели, мм.
    tpl::Bounds3 m_bounds;
    /// Выбранная плоскость среза.
    tpl::SlicePlane m_plane = tpl::SlicePlane::XY;
    /// Положение выбранной плоскости вдоль нормали, мм.
    double m_offset = 0.0;
    /// true — плоскость выбрана (`setPlane`); до этого видны все три через центр.
    bool m_planeChosen = false;
    /// Контуры среза, рисуемые на выбранной плоскости.
    tpl::SliceResult m_section;

    /// Шейдерная программа тел и плоскостей.
    QOpenGLShaderProgram m_prog;
    /// Буфер вершин тел.
    QOpenGLBuffer m_vbo{QOpenGLBuffer::VertexBuffer};
    /// Число вершин в буфере (три на треугольник).
    int m_vertexCount = 0;
    int m_opaqueCount = 0; ///< непрозрачные тела идут первыми, полупрозрачные — следом
    /// true — буфер вершин надо перезаполнить перед рисованием.
    bool m_dirty = true;
    /// true — контекст OpenGL инициализирован.
    bool m_glReady = false;

    // Камера: орбита вокруг точки.
    /// Точка обзора, мм.
    QVector3D m_target;
    /// Расстояние камеры до точки обзора, мм.
    double m_dist = 100.0;
    /// Углы камеры: рыскание и наклон, градусы.
    double m_yaw = -35.0, m_pitch = 25.0;

    /// Точка нажатия и последнее положение мыши, пиксели.
    QPoint m_press, m_last;
    /// true — мышь сдвинулась больше чем на 4 пикселя (это протяжка, а не щелчок).
    bool m_dragged = false;
    /// true — идёт сдвиг точки обзора (средняя кнопка или Alt+ЛКМ).
    bool m_panning = false;
};
/// @var MeshView::m_pitch
/// Наклон камеры, градусы, в пределах -89...89.
/// @var MeshView::m_last
/// Последнее положение мыши при протяжке, пиксели.
