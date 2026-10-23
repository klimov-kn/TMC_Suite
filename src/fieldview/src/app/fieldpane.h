// FieldPane — окно документа (аналог CFldViewView Windows-версии): вид одного файла
// поля. Переводит такт, топологию и настройки вида в SceneData для FieldRhiView
// (PrepareData / PrepareTpl / PrepareBoundaryTpl / PrepareBoundTplSize оригинала),
// рисует надписи и шкалу, выполняет команды меню и панели инструментов.
/// @file fieldpane.h
/// @brief Окно документа FieldView: перевод такта, топологии и настроек вида в сцену, команды вида.
///
/// Часть модуля FieldView (слой `app`). Между документом (`FieldDocument`) и выводом
/// (`FieldRhiView`): окно данных, пределы Z, шкалы, геометрия топологии и осей, надписи,
/// сведения о такте, строка состояния.
/// @see https://www.tamic.ru/docs/code/fieldview.html#pane
#pragma once

#include "fielddocument.h"
#include "fieldrhiview.h"
#include "scenedata.h"
#include "viewsettings.h"

#include <QTimer>
#include <QWidget>

#include <memory>

namespace fieldview {

/// @brief Уровень цвета для панели «Уровни цвета»: цвет и значение, с которого он начинается.
///
/// Что показывает строка состояния и окно уровней.
struct LevelInfo
{
    /// Цвет уровня.
    QColor color;
    /// Значение поля, с которого начинается уровень (единицы файла поля).
    double value = 0;
};

/// @brief Окно одного документа: сцена, такты, пределы шкал, команды вида.
///
/// Создаётся `MainWindow` (подокно MDI), удаляется при закрытии окна
/// (`WA_DeleteOnClose`). Владеет сценой (`SceneData`), картинками поверх неё и своей
/// копией настроек вида; документ разделяет с другими окнами того же файла. Настройки
/// сохраняются после каждой команды. Работает в потоке интерфейса.
/// @see https://www.tamic.ru/docs/code/fieldview.html#pane
class FieldPane : public QWidget
{
    Q_OBJECT
public:
    /// @brief Создаёт окно документа, вид RHI и связи с документом и видом; строит сцену.
    /// @param[in] doc       документ (общий с другими окнами)
    /// @param[in] settings  начальные настройки вида (копируются)
    /// @param[in] parent    родительский виджет
    FieldPane(std::shared_ptr<FieldDocument> doc, const ViewSettings &settings, QWidget *parent = nullptr);
    /// @brief Уничтожает окно; документ живёт, пока на него есть ссылки.
    ~FieldPane() override;

    /// @brief Возвращает документ окна.
    /// @return Общий указатель на документ.
    std::shared_ptr<FieldDocument> document() const { return m_doc; }
    /// @brief Возвращает вид RHI окна.
    /// @return Указатель на вид (принадлежит окну).
    FieldRhiView *view() const { return m_view; }
    /// @brief Возвращает настройки вида для изменения командами (затем нужно `toggle()` или `applySettings()`).
    /// @return Ссылка на настройки окна.
    ViewSettings &settings() { return m_s; }
    /// @brief Возвращает настройки вида окна.
    /// @return Константная ссылка на настройки.
    const ViewSettings &settings() const { return m_s; }
    /// @brief Заменяет настройки вида и перестраивает сцену.
    ///
    /// Применить изменённые настройки (диалог «Параметры», окно цветов уровней).
    ///
    /// Если изменились окно данных, режим модуля, автоматические пределы или шкала поля,
    /// накопленный размах сбрасывается. Настройки сохраняются.
    /// @param[in] s  новые настройки
    void applySettings(const ViewSettings &s);

    /// @brief Возвращает заголовок окна — имя файла поля.
    /// @return Имя файла.
    QString title() const;
    /// @brief Возвращает текст основной панели строки состояния (окно данных или значение в точке, ошибка).
    /// @return Текст.
    QString traceText() const { return m_trace; }      // панель 0 строки состояния (PutTrace1)
    /// @brief Возвращает текст «nT = …; t = …» для строки состояния.
    /// @return Текст.
    QString statsText() const { return m_stats; }      // панель 1 (PutStatistics1)

    // --- Такты.
    /// @brief Возвращает число тактов документа.
    /// @return Число тактов.
    int frameCount() const;
    /// @brief Возвращает номер показываемого такта.
    /// @return Номер, 0 … `frameCount()` - 1.
    int frame() const { return m_frame; }
    /// @brief Показывает такт.
    ///
    /// Номер обрезается до допустимого; последний такт включает следование за расчётом
    /// (новые кадры от ядра показываются сразу). Геометрия пересчитывается, только если
    /// изменились пределы Z или включено «значение в точке».
    /// @param[in] f  номер такта
    void setFrame(int f);
    /// @brief Переходит на `delta` тактов вперёд или назад по кругу.
    /// @param[in] delta  шаг, тактов (может быть отрицательным)
    void stepFrame(int delta);
    /// @brief Сообщает, идёт ли проигрывание тактов.
    /// @return `true` — идёт.
    bool playing() const { return m_playing; }
    /// @brief Запускает или останавливает проигрывание тактов по таймеру (при одном такте не запускает).
    /// @param[in] on  `true` — проигрывать
    void setPlaying(bool on);
    /// @brief Задаёт скорость проигрывания и сохраняет её в настройках.
    /// @param[in] fps  кадров в секунду, 1 … 60
    void setSpeed(int fps);
    /// @brief Задаёт контраст: автоматические пределы шкалы поля делятся на `g`.
    /// @param[in] g  1 … 100
    void setGain(double g);
    /// @brief Сообщает, показывается ли последний такт (новые кадры от ядра показываются сразу).
    /// @return `true` — окно следует за расчётом.
    bool followLive() const { return m_follow; }

    // --- Текущие пределы и шкала (для диалога, окна уровней, экспорта).
    /// @brief Возвращает действующий нижний предел шкалы поля.
    /// @return Значение поля (единицы файла).
    double fieldZMin() const { return m_zMin; }
    /// @brief Возвращает действующий верхний предел шкалы поля.
    /// @return Значение поля (единицы файла).
    double fieldZMax() const { return m_zMax; }
    /// @brief Возвращает нижний предел шкалы слоя eps (или N, Y, B).
    /// @return Значение величины слоя.
    double epsZMin() const { return m_eMin; }
    /// @brief Возвращает верхний предел шкалы слоя eps (или N, Y, B).
    /// @return Значение величины слоя.
    double epsZMax() const { return m_eMax; }
    /// @brief Возвращает 20 уровней цвета поля для панели «Уровни цвета».
    ///
    /// Уровень i (0 … 19): t = i / 19, значение `zMin + t * (zMax - zMin)` (в режиме модуля —
    /// `t * zMax`), цвет — по действующей шкале.
    /// @return 20 уровней снизу вверх.
    std::vector<LevelInfo> fieldLevels() const;
    /// @brief Сообщает, есть ли в топологии сетки плазмы N, Y, B.
    /// @return `true` — файл `.tt` от X-ядра с плазмой.
    bool hasPlasma() const;
    /// @brief Возвращает величину, которую на самом деле показывает слой eps.
    ///
    /// Что на самом деле показывает слой ε: 0 — ε, 1 — N, 2 — Y, 3 — B. Без сеток плазмы
    /// в .tt выбранная N/Y/B показывается как ε — и подписи тоже «ε» (Н-35).
    /// @return 0 — eps, 1 — N, 2 — Y, 3 — B.
    int effectiveEpsType() const;
    /// @brief Сообщает, можно ли показать слой eps (топология есть и сетка совпадает).
    /// @return `true` — можно.
    bool epsAvailable() const;
    /// @brief Объясняет, почему слой eps недоступен.
    /// @return Текст для пользователя; пусто — доступен.
    QString epsUnavailableReason() const;

    // --- Команды (имена — как обработчики CFldViewView).
    /// @brief Переключает флаг настроек и перестраивает сцену.
    ///
    /// Для режима модуля сбрасывает накопленный размах; при включении «значения в точке» вне
    /// окна данных ставит точку в середину окна.
    /// @param[in,out] flag  ссылка на поле `settings()`
    void toggle(bool &flag);
    /// @brief Выбирает величину слоя eps и включает его заливку с автоматическими пределами.
    /// @param[in] t  0 — eps, 1 — N, 2 — Y, 3 — B
    void setEpsType(int t);           // OnEpsTypeEps / N / Y / B
    /// @brief Меняет окно данных вдвое (команды Zoom).
    ///
    /// Увеличение: окно вокруг его центра становится вдвое уже (не уже 2 * Delta);
    /// уменьшение: вдвое шире, а если шире всей сетки — показывается весь файл.
    /// @param[in] kind  0 «+», 1 «+X», 2 «+Y», 3 «-», 4 «-X», 5 «-Y»
    /// @see https://www.tamic.ru/docs/code/fieldview.html#window
    void zoom(int kind);              // 0 «+», 1 «+X», 2 «+Y», 3 «−», 4 «−X», 5 «−Y»
    /// @brief Поворачивает сцену вокруг оси.
    /// @param[in] axis  0 — X, 1 — Y, 2 — Z
    /// @param[in] deg   угол, градусы
    void rotate(int axis, double deg);// 0 — X, 1 — Y, 2 — Z
    /// @brief Сдвигает сцену вдоль оси (команды Translate).
    /// @param[in] axis  0 — X, 1 — Y, 2 — Z (к наблюдателю)
    /// @param[in] d     сдвиг, единицы сцены (сцена — около 2,5)
    void translate(int axis, double d);
    /// @brief Сдвигает на шаг сетки точку «значения в точке» или (без неё) окно данных.
    /// @param[in] dx  -1, 0 или 1 по X
    /// @param[in] dy  -1, 0 или 1 по Y
    void arrow(int dx, int dy);       // OnStrelkleft/right/up/down
    /// @brief Сбрасывает настройки вида к значениям по умолчанию (цвета шкал и материалов, величина слоя eps сохраняются).
    void setDefault();                // OnViewDefault
    /// @brief Сбрасывает только камеру (углы, сдвиг, масштаб, сдвиг мышью).
    void resetView();                 // новое: только камера
    /// @brief Включает автоматические пределы Z показываемых слоёв и сбрасывает накопленный размах.
    void resetZRange();               // OnLButtonDblClk: пределы Z заново
    /// @brief Перечитывает файл документа и перестраивает сцену.
    void reread();                    // OnLButtonDblClk: перечитать файл

    /// @brief Рисует кадр и возвращает его картинкой.
    ///
    /// Снимок сцены (с надписями и шкалой) — печать, сохранение, копирование.
    /// @return Картинка вида; пустая — окно свёрнуто или графика не запущена.
    QImage grabImage();
    /// @brief Перестраивает окно данных, значения, слой eps, геометрию, сведения и строку состояния.
    ///
    /// Перерисовать всё (после смены настроек).
    void rebuildAll();

Q_SIGNALS:
    /// @brief Сигнал: изменился текст строки состояния.
    void statusChanged();             // строка состояния
    /// @brief Сигнал: изменилось состояние команд (флажки, такт, проигрывание).
    void stateChanged();              // флажки команд, такты
    /// @brief Сигнал: просьба открыть окно «Параметры» (щелчок правой кнопкой по виду).
    /// @param globalPos  точка щелчка, экранные координаты
    void requestParameters(const QPoint &globalPos);

protected:
    /// @brief Перерисовывает сведения, шкалы и надписи под новый размер.
    /// @param[in] e  событие изменения размера
    void resizeEvent(QResizeEvent *e) override;

private:
    /// @brief Реакция на изменение тактов документа: при следовании за расчётом — последний такт, перестройка сцены.
    void onFramesChanged();
    /// @brief Реакция на новый кадр от ядра: показывает его, если окно следует за расчётом и не проигрывает такты.
    /// @param[in] index  номер нового такта
    void onFrameArrived(int index);
    /// @brief Вычисляет окно данных (узлы и метры), прореживание, пропорции и размещение сетки в сцене.
    /// @see https://www.tamic.ru/docs/code/fieldview.html#window
    void updateWindow();              // окно данных (Xmin…Ymax), пропорции
    /// @brief Берёт значения текущего такта для окна данных (без копии, если показана вся сетка).
    void updateFieldValues();
    /// @brief Вычисляет пределы Z поля, выбирает шкалу и заполняет параметры слоя поля.
    /// @see https://www.tamic.ru/docs/code/fieldview.html#z-range
    void updateFieldRange();
    /// @brief Заполняет слой eps (или N, Y, B) из топологии: значения окна данных, пределы, шкала.
    void updateEps();
    /// @brief Строит цветную геометрию: топология, размеры, оси, шкалы Z, метка «значения в точке».
    void updateGeometry();
    /// @brief Объявлена, но не определена и не вызывается (надписи рисует `drawLabelsSprite()`).
    void updateLabels();
    /// @brief Перерисовывает картинки поверх сцены: сведения о такте, шкалы, сообщение об ошибке.
    /// @param[in] infoOnly  `true` — только сведения о такте (при проигрывании)
    void updateHud(bool infoOnly = false);
    /// @brief Составляет тексты строки состояния и выдаёт `statusChanged()`.
    void updateStatus();
    /// @brief Передаёт камеру виду, при необходимости перестраивает сцену, сохраняет настройки.
    /// @param[in] geometryToo  `false` — изменилась только камера (перерисовать лишь надписи)
    void settingsChanged(bool geometryToo = true);
    /// @brief Переводит значение поля в высоту сцены.
    /// @param[in] v  значение поля
    /// @return `2 * ((v - zMin) / (zMax - zMin) - 0,5)`, умноженное на высоту рельефа в 3D.
    double zf(double v) const;        // высота точки поля в сцене
    /// @brief Переводит координату X (м от начала сетки) в координату сцены.
    /// @param[in] x  координата, м
    /// @return `kx * 2,5 * ((x - xMin) / (xMax - xMin) - 0,5)`.
    double sceneX(double x) const;
    /// @brief Переводит координату Y (м от начала сетки) в координату сцены.
    /// @param[in] y  координата, м
    /// @return `ky * 2,5 * ((y - yMin) / (yMax - yMin) - 0,5)`.
    double sceneY(double y) const;
    /// @brief Рисует надписи сцены в картинку размера окна (проекция точек текущей камерой).
    void drawLabelsSprite();

    /// Документ окна.
    std::shared_ptr<FieldDocument> m_doc;
    /// Настройки вида окна.
    ViewSettings m_s;
    /// Вид RHI (дочерний виджет).
    FieldRhiView *m_view = nullptr;
    /// Сцена для вида.
    SceneData m_scene;
    /// Картинки поверх сцены: 0 — надписи, 1 — сведения о такте, 2 — шкала поля, 3 — шкала eps, 4 — сообщение.
    std::vector<Sprite> m_sprites;    // 0 — надписи, 1 — сведения, 2 — шкала поля, 3 — шкала ε, 4 — ошибка

    /// Номер показываемого такта.
    int m_frame = 0;
    /// Идёт проигрывание тактов.
    bool m_playing = false;
    /// Показывается последний такт — новые кадры от ядра показываются сразу.
    bool m_follow = true;
    /// Таймер проигрывания тактов.
    QTimer m_playTimer;

    // Окно данных (PrepareData): сетка всего файла и показываемая часть.
    /// @name Сетка файла и начало окна данных
    ///@{
    /// Узлов файла по X и Y; номер первого узла окна данных по X (j0) и по Y (i0).
    int m_nX1 = 0, m_nY1 = 0, m_j0 = 0, m_i0 = 0;
    ///@}
    /// @name Прореживание
    ///@{
    /// Узел сцены — каждый `m_stride`-й узел окна данных по X и по Y.
    int m_strideX = 1, m_strideY = 1; // узел сцены — каждый m_stride-й узел окна (С-62)
    ///@}
    /// @brief Сообщает, показана ли вся сетка файла без прореживания.
    /// @return `true` — значения такта можно отдать виду без копии.
    bool wholeGrid() const { return m_strideX == 1 && m_strideY == 1 && m_scene.nX == m_nX1 && m_scene.nY == m_nY1; }
    /// @name Шаг сетки и края окна данных, м от начала сетки
    ///@{
    /// Края привязаны к узлам: `m_xMin = j0 * Delta` и т. д.
    double m_delta = 0.001, m_xMin = 0, m_xMax = 1, m_yMin = 0, m_yMax = 1;
    ///@}
    /// @name Пропорции окна данных
    ///@{
    /// Доли сцены по X и Y (при «Пропорционально» меньшая сторона меньше 1).
    double m_kx = 1, m_ky = 1;
    ///@}
    /// Ошибка окна данных («number of point along X axies < 2» …).
    QString m_windowError;

    /// Показываемый такт.
    std::shared_ptr<const FieldFrame> m_current;
    /// @name Действующие пределы шкалы поля
    ///@{
    /// Нижний и верхний пределы, единицы файла поля.
    double m_zMin = -1, m_zMax = 1;   // действующие пределы поля
    ///@}
    /// Накопленный размах действителен.
    bool m_accValid = false;
    /// @name Накопленный размах по показанным тактам
    ///@{
    /// Минимум, максимум, максимум модуля.
    double m_accMin = 0, m_accMax = 0, m_accAbs = 0;   // накопленный размах (как dZmin/dZmax оригинала)
    ///@}
    /// Действующая шкала поля («Авто» уже раскрыта).
    ScaleKind m_fieldScale = ScaleKind::Wave;
    /// @name Действующие пределы слоя eps
    ///@{
    /// Нижний и верхний пределы величины слоя.
    double m_eMin = 0, m_eMax = 1;
    ///@}
    /// Действующая шкала слоя eps.
    ScaleKind m_epsScale = ScaleKind::Glow;
    /// Почему слой eps не показан.
    QString m_epsReason;

    /// Не используется.
    quint64 m_version = 1;
    /// @name Тексты строки состояния
    ///@{
    /// Основная панель и панель «nT, t».
    QString m_trace, m_stats;
    ///@}
    /// Значение поля в точке (в исходной точности, если есть).
    double m_probeValue = 0;
    /// Значение в точке вычислено.
    bool m_probeOk = false;
};

} // namespace fieldview
