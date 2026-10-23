// FieldRhiView — сцена поля на Qt RHI (QRhiWidget): Direct3D 11 на Windows, при его
// отсутствии — OpenGL; на других системах — Vulkan/Metal/OpenGL. Заменяет вывод
// OpenGL 1.x (glBegin/glVertex на каждую ячейку каждый кадр) Windows-версии.
//
// Сетка узлов — в видеопамяти один раз; значения такта — буфер, обновляемый при смене
// кадра; шкала — текстура 256×1; топология, оси и размерные линии — буферы цветных
// треугольников и линий; надписи и шкала — картинки поверх сцены (Sprite).
// Перспектива 30° (gluPerspective(30, …) оригинала), поворот glRotatef X → Y → Z.
/// @file fieldrhiview.h
/// @brief Вид сцены поля на Qt RHI (`QRhiWidget`): рельеф и карта поля, слой eps, топология, надписи; мышь.
///
/// Часть модуля FieldView (слой `render`). Графический интерфейс выбирается при запуске
/// (`chooseGraphicsApi()`): Direct3D 11, Direct3D 11 WARP или OpenGL.
/// @see https://www.tamic.ru/docs/code/fieldview.html#rhi
#pragma once

#include "scenedata.h"

#include <QMatrix4x4>
#include <QRhiWidget>

#include <memory>
#include <vector>

class QRhi;
class QRhiBuffer;
class QRhiTexture;
class QRhiSampler;
class QRhiShaderResourceBindings;
class QRhiGraphicsPipeline;
class QRhiRenderPassDescriptor;
class QRhiResourceUpdateBatch;

namespace fieldview {

/// @brief Положение камеры вида: углы поворота сцены, сдвиг, масштаб и сдвиг мышью.
struct CameraParams
{
    /// @name Углы поворота сцены, градусы
    ///@{
    /// Повороты вокруг X, затем Y, затем Z (как `glRotatef` FieldView 4.1).
    double angleX = -55, angleY = 0, angleZ = -20;   // градусы
    ///@}
    /// @name Сдвиг сцены (команды Translate), единицы сцены
    ///@{
    /// `transX`, `transY` — сдвиг в плоскости экрана; `transZ` — приближение к камере.
    double transX = 0, transY = 0, transZ = 0;      // сдвиг сцены (Translate)
    ///@}
    /// Масштаб сцены (колесо мыши), 0,05 … 50.
    double zoom = 1.0;
    /// @name Сдвиг мышью
    ///@{
    /// Сдвиг сцены правой или средней кнопкой, доли половины окна.
    double panX = 0, panY = 0;                      // сдвиг мышью, доли окна
    ///@}
};

/// @brief Виджет вывода сцены через Qt RHI.
///
/// Создаётся `FieldPane` (по одному на окно документа). Не владеет сценой и картинками:
/// хранит указатели, данные принадлежат `FieldPane`. При каждом кадре сравнивает номера
/// изменений и перезаливает в видеопамять только изменившееся: координаты узлов — при
/// смене размера сетки, значения — при смене такта, шкалу — при смене палитры. Работает
/// в потоке интерфейса. События мыши переводит в сигналы, камеру меняет владелец.
/// @see https://www.tamic.ru/docs/code/fieldview.html#rhi
class FieldRhiView : public QRhiWidget
{
    Q_OBJECT
public:
    /// @brief Создаёт вид с выбранным при запуске графическим интерфейсом и сглаживанием 4x.
    /// @param[in] parent  родительский виджет
    explicit FieldRhiView(QWidget *parent = nullptr);
    /// @brief Уничтожает вид; ресурсы видеокарты освобождает `releaseResources()`.
    ~FieldRhiView() override;

    /// @brief Задаёт сцену и запрашивает перерисовку.
    /// @param[in] scene  сцена (не копируется; должна жить дольше вида); `nullptr` — пусто
    void setScene(const SceneData *scene) { m_scene = scene; update(); }
    /// @brief Возвращает текущую сцену.
    /// @return Указатель на сцену или `nullptr`.
    const SceneData *scene() const { return m_scene; }
    /// @brief Задаёт положение камеры и запрашивает перерисовку.
    /// @param[in] c  параметры камеры
    void setCamera(const CameraParams &c) { m_camera = c; update(); }
    /// @brief Возвращает текущие параметры камеры.
    /// @return Ссылка на параметры камеры.
    const CameraParams &camera() const { return m_camera; }
    /// @brief Задаёт картинки поверх сцены и запрашивает перерисовку.
    /// @param[in] sprites  список картинок (не копируется; рисуются первые 8); `nullptr` — нет
    void setSprites(const std::vector<Sprite> *sprites) { m_sprites = sprites; update(); }

    /// @brief Возвращает матрицу проекции и вида для текущих сцены, камеры и размера виджета.
    ///
    /// Матрица вида и проекции без поправки под графический интерфейс.
    /// @return Матрица «сцена -> отсечение» без поправки `clipSpaceCorrMatrix()`.
    QMatrix4x4 viewProjection() const;
    /// @brief Строит матрицу проекции и вида.
    ///
    /// Перспектива с углом обзора 30° по вертикали; расстояние камеры подбирается так, чтобы
    /// сцена с габаритами `boundX`, `boundY`, `boundZ` при виде по умолчанию целиком
    /// помещалась в окне любой формы; затем сдвиг, повороты X -> Y -> Z и масштаб.
    /// @param[in] scene   сцена (габариты); `nullptr` — габариты 2,5 x 2,5 x 1
    /// @param[in] c       камера
    /// @param[in] aspect  отношение ширины окна к высоте (не меньше 0,05)
    /// @return Матрица «сцена -> отсечение» (соглашения OpenGL).
    /// @see https://www.tamic.ru/docs/code/fieldview.html#camera
    static QMatrix4x4 viewProjection(const SceneData *scene, const CameraParams &c, double aspect);
    /// @brief Находит точку плоскости z = 0 сцены под точкой окна.
    ///
    /// Точка плоскости z = 0 под курсором (в координатах сцены).
    /// @param[in]  px   точка окна, логические пиксели
    /// @param[out] out  точка сцены
    /// @return `false` — луч параллелен плоскости или плоскость позади камеры.
    bool pickGround(const QPointF &px, QVector3D *out) const;
    /// @brief Переводит точку сцены в точку окна.
    ///
    /// Проекция точки сцены в пиксели окна. false — точка за камерой.
    /// @param[in]  p   точка сцены
    /// @param[out] px  точка окна, логические пиксели (ось Y вниз)
    /// @return `false` — точка за камерой, `px` не изменена.
    bool project(const QVector3D &p, QPointF *px) const;

    /// @brief Возвращает имя графического интерфейса RHI («D3D11», «OpenGL» …).
    /// @return Имя; пусто до первого кадра.
    QString backendName() const { return m_backend; }

    /// @brief Задаёт графический интерфейс для всех создаваемых затем видов.
    ///
    /// Графический интерфейс для всех видов (выбирается при запуске: D3D11 → D3D11 WARP → OpenGL).
    /// @param[in] api  интерфейс `QRhiWidget::Api`
    static void setPreferredApi(QRhiWidget::Api api) { s_api = api; }
    /// @brief Возвращает графический интерфейс для новых видов.
    /// @return Интерфейс `QRhiWidget::Api`.
    static QRhiWidget::Api preferredApi() { return s_api; }
    /// @brief Возвращает частоту кадров вида за последнюю секунду.
    /// @return Кадров в секунду.
    double fps() const { return m_fps; }

Q_SIGNALS:
    /// @brief Сигнал: левая кнопка тянется — повернуть сцену.
    /// @param dx  сдвиг мыши по горизонтали, логические пиксели
    /// @param dy  сдвиг мыши по вертикали, логические пиксели
    void rotateRequested(double dx, double dy);
    /// @brief Сигнал: правая или средняя кнопка тянется — сдвинуть сцену.
    /// @param dx  сдвиг по горизонтали, доли половины ширины окна (вправо — плюс)
    /// @param dy  сдвиг по вертикали, доли половины высоты окна (вверх — плюс)
    void panRequested(double dx, double dy);
    /// @brief Сигнал: колесо мыши — изменить масштаб.
    /// @param factor  множитель масштаба, 1,0015 в степени угла поворота колеса (1/8 градуса)
    void zoomRequested(double factor);
    /// @brief Сигнал: двойной щелчок левой кнопкой (перечитать файл и пределы Z).
    void doubleClicked();
    /// @brief Сигнал: щелчок правой кнопкой без перетаскивания (окно «Параметры»).
    /// @param globalPos  точка щелчка, экранные координаты
    void rightClicked(const QPoint &globalPos);
    /// @brief Сигнал: Shift + щелчок левой кнопкой — поставить «значение в точке».
    /// @param px  точка окна, логические пиксели
    void probeRequested(const QPointF &px);
    /// @brief Сигнал: кадр нарисован.
    void frameRendered();
    /// @brief Сигнал: создан (или пересоздан) объект RHI.
    /// @param name  имя графического интерфейса
    void backendChanged(const QString &name);

protected:
    /// @brief Готовит ресурсы при первом кадре, смене RHI или несовместимой цели вывода.
    /// @param[in] cb  буфер команд (не используется)
    void initialize(QRhiCommandBuffer *cb) override;
    /// @brief Рисует кадр: заливает изменившиеся данные, затем слои сетки, топологию, линии, картинки.
    /// @param[in] cb  буфер команд кадра
    /// @see https://www.tamic.ru/docs/code/fieldview.html#rhi
    void render(QRhiCommandBuffer *cb) override;
    /// @brief Освобождает все ресурсы видеокарты (виджет уходит из окна или меняется RHI).
    void releaseResources() override;

    /// @brief Начинает перетаскивание; Shift + левая кнопка — сигнал `probeRequested()`.
    /// @param[in] e  событие мыши
    void mousePressEvent(QMouseEvent *e) override;
    /// @brief Перетаскивание: левая кнопка — поворот, правая и средняя — сдвиг.
    /// @param[in] e  событие мыши
    void mouseMoveEvent(QMouseEvent *e) override;
    /// @brief Конец перетаскивания; щелчок правой кнопкой (сдвиг не больше 4 точек) — `rightClicked()`.
    /// @param[in] e  событие мыши
    void mouseReleaseEvent(QMouseEvent *e) override;
    /// @brief Двойной щелчок левой кнопкой — сигнал `doubleClicked()`.
    /// @param[in] e  событие мыши
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    /// @brief Колесо мыши — сигнал `zoomRequested()`.
    /// @param[in] e  событие колеса
    void wheelEvent(QWheelEvent *e) override;

private:
    /// Графический интерфейс для новых видов (по умолчанию Direct3D 11 на Windows).
    static QRhiWidget::Api s_api;
    /// Ресурсы видеокарты одной картинки поверх сцены (определение — в `fieldrhiview.cpp`).
    struct SpriteGpu;
    /// @brief Удаляет конвейеры, буферы, текстуры и сбрасывает номера изменений.
    void releaseAll();
    /// @brief Создаёт буферы констант, текстуры шкал, наборы привязок и пять графических конвейеров.
    ///
    /// Конвейеры: заливка сетки (треугольники, запись глубины), каркас сетки (линии),
    /// цветные треугольники, цветные линии, картинки (без глубины). Смешивание —
    /// предумноженная альфа.
    void buildPipelines();
    /// @brief Создаёт буферы сетки под текущий размер: координаты (u, v), значения, индексы.
    ///
    /// Индексы: два треугольника на ячейку; каркас — два набора линий (через шаг: не больше
    /// ~120 линий по стороне поверх заливки и ~1000 без неё).
    /// @param[in,out] u  пакет обновлений ресурсов текущего кадра
    void rebuildGrid(QRhiResourceUpdateBatch *u);

    /// Объект RHI виджета (не владеет).
    QRhi *m_rhi = nullptr;
    /// Число отсчётов сглаживания цели вывода, под которое построены конвейеры.
    int m_sampleCount = 0;
    /// Описание прохода цели вывода (не владеет).
    QRhiRenderPassDescriptor *m_rpDesc = nullptr;

    /// @name Буферы сетки
    ///@{
    /// Координаты (u, v) узлов; значения поля и eps; индексы треугольников и двух каркасов.
    std::unique_ptr<QRhiBuffer> m_uvBuf, m_fieldVal, m_epsVal, m_triIdx, m_wireDec, m_wireFull;
    ///@}
    /// @name Буферы констант
    ///@{
    /// По буферу на проход: заливка и каркас поля и eps (`GridUniforms`), цветная геометрия и картинки (матрица).
    std::unique_ptr<QRhiBuffer> m_ubFieldSurf, m_ubFieldWire, m_ubEpsSurf, m_ubEpsWire, m_ubColor, m_ubSprite;
    ///@}
    /// @name Буферы вершин цветной геометрии и картинок
    ///@{
    /// Треугольники и линии `SceneData` (7 чисел на вершину); прямоугольники картинок (до 8).
    std::unique_ptr<QRhiBuffer> m_triBuf, m_lineBuf, m_spriteVerts;
    ///@}
    /// @name Текстуры шкал
    ///@{
    /// Шкала поля и шкала eps, 256 x 1 RGBA8.
    std::unique_ptr<QRhiTexture> m_fieldTex, m_epsTex;
    ///@}
    /// @name Выборки текстур
    ///@{
    /// Линейная выборка с прижатием к краю: для шкал и для картинок.
    std::unique_ptr<QRhiSampler> m_sampler, m_spriteSampler;
    ///@}
    /// @name Наборы привязок ресурсов
    ///@{
    /// Для четырёх проходов сетки, цветной геометрии и образец раскладки картинок.
    std::unique_ptr<QRhiShaderResourceBindings> m_srbFieldSurf, m_srbFieldWire, m_srbEpsSurf, m_srbEpsWire, m_srbColor, m_srbSpriteLayout;
    ///@}
    /// @name Графические конвейеры
    ///@{
    /// Заливка сетки, каркас сетки, цветные треугольники, цветные линии, картинки.
    std::unique_ptr<QRhiGraphicsPipeline> m_psSurf, m_psWire, m_psTri, m_psLine, m_psSprite;
    ///@}
    /// Ресурсы картинок поверх сцены, по одному на индекс `Sprite`.
    std::vector<std::unique_ptr<SpriteGpu>> m_spriteGpu;

    /// @name Размер сетки, под который построены буферы
    ///@{
    /// Узлов по X и по Y; 0 — буферов нет.
    int m_gridX = 0, m_gridY = 0;
    ///@}
    /// @name Число индексов
    ///@{
    /// Индексов треугольников, прореженного и полного каркаса.
    quint32 m_triCount = 0, m_wireDecCount = 0, m_wireFullCount = 0;
    ///@}
    /// @name Залитые номера изменений значений
    ///@{
    /// Номера `GridLayer::valuesVersion`, уже лежащие в видеопамяти (~0 — ничего).
    quint64 m_fieldValVersion = ~0ull, m_epsValVersion = ~0ull;
    ///@}
    /// @name Залитые номера изменений шкал и геометрии
    ///@{
    /// Номера `paletteVersion` и `geometryVersion`, уже лежащие в видеопамяти.
    quint64 m_fieldPalVersion = ~0ull, m_epsPalVersion = ~0ull, m_geomVersion = ~0ull;
    ///@}
    /// @name Цветная геометрия в видеопамяти
    ///@{
    /// Число вершин треугольников и линий; ёмкость их буферов, байт.
    quint32 m_triVerts = 0, m_lineVerts = 0, m_triCap = 0, m_lineCap = 0;
    ///@}

    /// Сцена (принадлежит `FieldPane`).
    const SceneData *m_scene = nullptr;
    /// Картинки поверх сцены (принадлежат `FieldPane`).
    const std::vector<Sprite> *m_sprites = nullptr;
    /// Камера.
    CameraParams m_camera;

    /// Имя графического интерфейса.
    QString m_backend;
    /// Частота кадров за последнюю секунду.
    double m_fps = 0;
    /// Кадров с начала текущей секунды подсчёта.
    int m_frames = 0;
    /// Начало текущей секунды подсчёта, мс от эпохи.
    qint64 m_fpsStart = 0;

    // Мышь.
    /// Нажатая кнопка перетаскивания (`Qt::NoButton` — нет).
    Qt::MouseButton m_dragButton = Qt::NoButton;
    /// @name Точки мыши
    ///@{
    /// Последняя точка и точка нажатия, логические пиксели.
    QPointF m_lastPos, m_pressPos;
    ///@}
    /// Мышь сдвинулась от точки нажатия больше чем на 4 точки.
    bool m_dragged = false;
};

} // namespace fieldview
