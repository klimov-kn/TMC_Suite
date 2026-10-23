// Файл топологии <имя>.tt (TOPOLOGY ядер) и границы блоков для показа.
//
// Перенос без изменений логики из Windows-версии FieldView:
//   * CTmcBlockTpl::ReadData (tmcblocktpl.cpp MFC) и
//     CTmcRTH_BlockList::Load (src/libs/TMCIndan/TmcRTH_BolckList.cpp) — чтение;
//   * CTmcBoundaryTpl::SetBlockList / SetRectStat / SetPolygonStat / SetCircleStat /
//     SetInputX / SetInputY / TranslateX0Y0 (TmcBoundaryTpl.cpp) — четырёхугольники границ;
//   * CFldViewView::ReadTpl, CalcnTopPolygon, PrepareBoundaryTpl (FldViewView.cpp) —
//     нумерация портов («Inp N»), отбор видимых четырёхугольников, центры портов.
//
// Формат: строка-признак, список блоков (#BlockListBegin … #BlockListEnd), заголовок
// (nT, Time, Delta, Xmin, Ymin, nX, nY), сетка ε (double), у X-моды ещё N, Y, B.
/// @file topology.h
/// @brief Файл топологии `.tt` (сетки eps, N, Y, B и список блоков) и границы блоков для показа.
///
/// Часть модуля FieldView (слой `core`). Читает файл, который ядро пишет по строке
/// `TOPOLOGY;` секции `#OUTPUT`, и строит из списка блоков четырёхугольники границ,
/// центры портов и размерные линии в координатах задания (м).
/// @see https://www.tamic.ru/docs/code/fieldview.html#topology-file
#pragma once

#include <QString>

#include <vector>

namespace fieldview {

// Типы блоков — src/Include/tmcgrviw.h (CTMCRTH_BLCKNTYPE_*).
/// @brief Тип блока топологии (поле `nType` записи блока).
enum BlockType : int {
    /// Диэлектрик (блок eps).
    kTypeEps = 0,
    /// Поглощающая граница.
    kTypeAbsorber = 100,
    /// Металл.
    kTypeMetal = 101,
    /// Вход (порт) на левой границе, `INPUT_X`.
    kTypeInpXLeft = 102,
    /// Вход (порт) на правой границе, `INPUT_X`.
    kTypeInpXRight = 103,
    /// Вход (порт) на верхней границе, `INPUT_Y`.
    kTypeInpYTop = 104,
    /// Вход (порт) на нижней границе, `INPUT_Y`.
    kTypeInpYBot = 105,
    /// Магнитная стенка.
    kTypeMagnetic = 107,
    /// Не из оригинала: контур плазменного блока (RECT_STAT_N/_B/_Y …). Windows такие
    /// блоки пропускает; здесь их контур рисуется цветом «плазма» (style-guide).
    kTypePlasma = 1000,
};

/// @brief Проверяет, является ли тип блока входом (портом).
/// @param[in] t  тип блока (`BlockType`)
/// @return `true` для `kTypeInpXLeft`, `kTypeInpXRight`, `kTypeInpYTop`, `kTypeInpYBot`.
inline bool isInputType(int t)
{
    return t == kTypeInpXLeft || t == kTypeInpXRight || t == kTypeInpYTop || t == kTypeInpYBot;
}

/// @brief Запись одного блока из списка `#BlockListBegin … #BlockListEnd` файла `.tt`.
///
/// Запись блока (CTmcRTH_BlockList::Save1 / Load).
///
/// Смысл `x0 … yMin` зависит от вида блока: у прямоугольника — сдвиг и габариты, у круга
/// `xMin`/`xMax` — внутренний/внешний радиус, `yMin`/`yMax` — начальный/конечный угол (рад).
/// Все длины — в координатах задания, м (без сдвига `x0`, `y0`).
struct TopBlock
{
    /// Вид блока с завершающим пробелом: «RECT_STAT », «INPUT_X », «CIRCLE_STAT_N » …
    QString block;     // "RECT_STAT ", "INPUT_X " …
    /// Выражение eps или имя типа границы («METAL» …) — строка задания как есть.
    QString eps;       // выражение ε / тип ("METAL")
    /// @name Прочие строки записи блока (третья — пятая строки), хранятся без разбора
    ///@{
    /// Строки 3–5 записи блока (как в файле, без концов строк).
    QString vx, vy, w;
    ///@}
    /// @name Привязка (`x0`, `y0`) и габариты блока, м (у круга — радиусы и углы, рад)
    ///@{
    /// Сдвиг блока и его габариты (смысл — в описании структуры).
    double x0 = 0, xMax = 0, xMin = 0, y0 = 0, yMax = 0, yMin = 0;
    ///@}
    /// @name Номер блока (0 — не блок), служебное `nMemory`, тип `BlockType`, число вершин многоугольника
    ///@{
    /// Целые поля записи блока.
    int nBlock = 0, nMemory = 0, nType = 0, nXY = 0;
    ///@}
    /// @name Вершины многоугольника, м (`nXY` штук; без сдвига `x0`, `y0`)
    ///@{
    /// Координаты X и Y вершин многоугольника.
    std::vector<double> px, py;
    ///@}
};

/// @brief Полоса границы шириной в шаг сетки, заданная четырьмя вершинами.
///
/// Четырёхугольник границы (вершины 1..4, в координатах задания, м).
///
/// Отрезок контура блока утолщается на +-Delta/2 и рисуется четырёхугольником (заливка и
/// контур; в 3D — призмой).
struct TopQuad
{
    /// Тип блока (`BlockType`) — по нему выбирается цвет материала.
    int type = kTypeMetal;
    /// Координаты X вершин 1…4, м (со сдвигом блока).
    double x[4] = {0, 0, 0, 0};
    /// Координаты Y вершин 1…4, м (со сдвигом блока).
    double y[4] = {0, 0, 0, 0};
};

/// @brief Размерная линия: отрезок и подписываемая длина.
///
/// Размерная линия блока (CTmcTopSize): от (x1, y1) до (x2, y2), подпись — size, м.
struct TopSize
{
    /// @name Концы размерной линии (x1, y1) и (x2, y2), м
    ///@{
    /// Координаты концов размерной линии, м.
    double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    ///@}
    /// Подписываемая длина, м (делится на единицу длины файла при выводе).
    double size = 0;
};

/// @brief Порт (вход) для надписи «Inp N».
///
/// Порт (вход): номер «Inp N» и центр первого четырёхугольника.
struct TopPort
{
    /// Номер порта: 1, 2 … по порядку блоков-входов в файле.
    int number = 0;
    /// @name Центр первой полосы порта, м — точка надписи
    ///@{
    /// Координаты центра порта, м.
    double x = 0, y = 0;
    ///@}
};

/// @brief Всё, что рисуется для одного блока: полосы границы, размеры, привязка, номер порта.
///
/// Границы одного блока (CTmcBoundaryTpl).
///
/// Строится `buildBoundaries()`; координаты уже сдвинуты на `x0`, `y0` блока.
/// @see https://www.tamic.ru/docs/code/fieldview.html#boundaries
struct BlockBoundary
{
    /// Тип блока (`BlockType`); у блока `FILE` — `kTypeEps`, у плазменных — `kTypePlasma`.
    int nType = kTypeMetal;
    /// Номер блока из файла.
    int nBlock = 0;
    /// Номер порта (1, 2 …) или 0, если блок не вход.
    int inputNumber = 0;   // 1, 2 … для портов (ReadTpl), иначе 0
    /// Полосы границы блока.
    std::vector<TopQuad> quads;
    /// Размеры блока (Topology → Dimensions → Block): GetpcTopSize, со сдвигом X0, Y0.
    std::vector<TopSize> sizes;
    /// Привязка блока (Topology → Dimensions → Link_List): GetcTopSizeX0 / GetcTopSizeY0.
    /// @name Размерные линии привязки блока: от начала координат до `x0` и до `y0`
    ///@{
    /// Привязка блока по X и по Y (`CTmcTopSize`).
    TopSize linkX0, linkY0;
    ///@}
};

/// @brief Содержимое файла топологии `.tt`: список блоков, заголовок сетки и сетки eps (и N, Y, B у X-ядра).
///
/// Создаётся `FieldDocument`, читается `read()` целиком в память. Сетки — `double`,
/// `nX * nY` значений, строка за строкой. Объект перемещаемый, не потокобезопасный.
/// @see https://www.tamic.ru/docs/code/fieldview.html#topology-file
class TopologyFile
{
public:
    /// @brief Читает файл топологии; при ошибке объект остаётся пустым.
    ///
    /// Порядок: строка-признак, `#BlockListBegin`, записи блоков до `#BlockListEnd`, заголовок
    /// сетки (nT, Time, Delta, Xmin, Ymin, nX, nY), сетка eps, затем при наличии — N, Y, B.
    /// Если после eps данных на три сетки нет, плазмы нет (`hasPlasma()` = `false`).
    /// @param[in]  path   путь к `.tt`
    /// @param[out] error  текст ошибки («Can't open topology file …», «Bad topology file …»); может быть `nullptr`
    /// @return `true` — файл прочитан.
    bool read(const QString &path, QString *error);

    /// @brief Возвращает блоки файла (записи с номером блока 0 отброшены).
    /// @return Ссылка на список блоков.
    const std::vector<TopBlock> &blocks() const { return m_blocks; }
    /// @brief Возвращает номер такта из заголовка сетки.
    /// @return Номер такта.
    int nT() const { return m_nT; }
    /// @brief Возвращает модельное время из заголовка сетки, с.
    /// @return Время, с.
    double time() const { return m_time; }
    /// @brief Возвращает шаг сетки Delta из заголовка, м.
    /// @return Шаг сетки, м.
    double delta() const { return m_delta; }
    /// @brief Возвращает координату X узла (0, 0), м.
    /// @return Координата X, м.
    double xMin() const { return m_xMin; }
    /// @brief Возвращает координату Y узла (0, 0), м.
    /// @return Координата Y, м.
    double yMin() const { return m_yMin; }
    /// @brief Возвращает число узлов по X.
    /// @return Число узлов по X.
    int nX() const { return m_nX; }
    /// @brief Возвращает число узлов по Y.
    /// @return Число узлов по Y.
    int nY() const { return m_nY; }
    /// @brief Сообщает, есть ли в файле сетки плазмы N, Y, B (файл X-ядра).
    /// @return `true` — сетки N, Y, B прочитаны.
    bool hasPlasma() const { return m_hasPlasma; }
    /// @brief Возвращает сетку выбранной величины.
    ///
    /// Сетка по типу, как GetpdSurfaceByType: 0 — ε, 1 — N, 2 — Y, 3 — B
    /// (нет плазмы — всегда ε).
    /// @param[in] type  0 — eps, 1 — N (м^-3), 2 — Y, 3 — B (Тл); при отсутствии плазмы любой — eps
    /// @return Ссылка на сетку `nX * nY` значений (пустая, если файл не прочитан).
    /// @see https://www.tamic.ru/docs/theory/plasma.html#units
    const std::vector<double> &grid(int type) const;

private:
    /// Блоки файла.
    std::vector<TopBlock> m_blocks;
    /// Номер такта.
    int m_nT = 0;
    /// Модельное время, с.
    double m_time = 0;
    /// Шаг сетки, м.
    double m_delta = 1;
    /// @name Координаты узла (0, 0), м
    ///@{
    /// Координаты узла (0, 0) сетки в координатах задания, м.
    double m_xMin = 0, m_yMin = 0;
    ///@}
    /// @name Размер сетки, узлов
    ///@{
    /// Число узлов по X и по Y.
    int m_nX = 0, m_nY = 0;
    ///@}
    /// @name Сетки eps, N (м^-3), Y, B (Тл); у H-ядра N, Y, B пусты
    ///@{
    /// Сетки величин, `nX * nY` значений каждая.
    std::vector<double> m_eps, m_n, m_y, m_b;
    ///@}
    /// В файле есть сетки N, Y, B.
    bool m_hasPlasma = false;
};

/// @brief Строит границы всех блоков для показа.
///
/// Границы всех блоков (ReadTpl + SetBlockList). delta — шаг сетки файла поля.
/// plasmaOutlines — добавить контуры плазменных блоков (не из оригинала).
///
/// Прямоугольник, круг, многоугольник, вход — полосы шириной `delta` вдоль контура;
/// затем всё сдвигается на `x0`, `y0` блока; входы нумеруются по порядку.
/// @param[in] blocks          блоки файла топологии
/// @param[in] delta           шаг сетки файла поля, м (ширина полосы)
/// @param[in] plasmaOutlines  `true` — добавить контуры плазменных блоков (`*_N`, `*_B`, `*_Y`, `FILE_N/_B/_Y`)
/// @return Границы в порядке блоков.
/// @see https://www.tamic.ru/docs/code/fieldview.html#boundaries
std::vector<BlockBoundary> buildBoundaries(const std::vector<TopBlock> &blocks, double delta,
                                           bool plasmaOutlines);

/// @brief Какие типы блоков показывать.
///
/// Флаги показа типов (bDrawFlagEps, …Input, …Absorber, …Metal, …Magnetic).
struct TopologyFilter
{
    /// @name Показывать диэлектрик, входы, поглотитель, металл, магнитные стенки
    ///@{
    /// `true` — показывать блоки этого типа.
    bool eps = true, input = true, absorber = true, metal = true, magnetic = true;
    ///@}
    /// Показывать контуры плазменных блоков.
    bool plasma = true;
};

/// @brief Отбирает полосы границ, попадающие в показываемую область, и центры портов.
///
/// Видимые четырёхугольники и центры портов (CalcnTopPolygon + PrepareBoundaryTpl).
/// xMin..xMax, yMin..yMax — показываемая область поля (у Windows 0…(nX-1)·Delta).
///
/// Полоса берётся, если хотя бы одна её вершина лежит в области, расширенной на 1,1 * Delta.
/// @param[in]  bounds  границы блоков (`buildBoundaries()`)
/// @param[in]  xMin    левый край области, м
/// @param[in]  xMax    правый край области, м
/// @param[in]  yMin    нижний край области, м
/// @param[in]  yMax    верхний край области, м
/// @param[in]  delta   шаг сетки, м
/// @param[in]  filter  какие типы показывать
/// @param[out] quads   видимые полосы (очищается)
/// @param[out] ports   центры портов показываемых типов (очищается)
void visibleTopology(const std::vector<BlockBoundary> &bounds, double xMin, double xMax,
                     double yMin, double yMax, double delta, const TopologyFilter &filter,
                     std::vector<TopQuad> *quads, std::vector<TopPort> *ports);

/// @brief Отбирает размерные линии, целиком лежащие в показываемой области.
///
/// Видимые размерные линии (PrepareBoundTplSize): обе точки внутри области с запасом
/// 1,1·Delta и длина больше 2·Delta. block — размеры блоков, linkList — привязки X0/Y0.
/// @param[in] bounds    границы блоков
/// @param[in] xMin      левый край области, м
/// @param[in] xMax      правый край области, м
/// @param[in] yMin      нижний край области, м
/// @param[in] yMax      верхний край области, м
/// @param[in] delta     шаг сетки, м
/// @param[in] block     брать размеры блоков
/// @param[in] linkList  брать привязки `x0`, `y0`
/// @return Видимые размерные линии.
std::vector<TopSize> visibleSizes(const std::vector<BlockBoundary> &bounds, double xMin, double xMax,
                                  double yMin, double yMax, double delta, bool block, bool linkList);

} // namespace fieldview
