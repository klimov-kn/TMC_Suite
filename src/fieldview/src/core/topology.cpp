// Топология .tt и границы блоков — перенос логики Windows-версии FieldView
// (ссылки на исходные функции — topology.h). Числа, порядок и условия — как в оригинале.
/// @file topology.cpp
/// @brief Чтение файла топологии `.tt` и построение полос границ, портов и размерных линий блоков.
///
/// Часть модуля FieldView (слой `core`). Описание открытых функций — в `topology.h`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#boundaries
#include "topology.h"
#include "textscan.h"

#include <QFile>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace fieldview {

namespace {

/// @name Строки-признаки файла топологии
///@{
/// Признак файла, границы списка блоков и начала строк заголовка сетки.
constexpr const char kTopId[]     = "#TMC_TopologyOutputFile FormVer2.0 2000";
constexpr const char kListBegin[] = "#BlockListBegin";
constexpr const char kListEnd[]   = "#BlockListEnd";
constexpr const char kIdNT[]      = "#TMCGROFLD_nT ";
constexpr const char kIdT[]       = "#TMCGROFLD_Time ";
constexpr const char kIdDelta[]   = "#TMCGROFLD_Delta ";
constexpr const char kIdXmin[]    = "#TMCGROFLD_Xmin ";
constexpr const char kIdYmin[]    = "#TMCGROFLD_Ymin ";
constexpr const char kIdNX[]      = "#TMCGROFLD_nX ";
constexpr const char kIdNY[]      = "#TMCGROFLD_nY ";
///@}

/// @brief Возвращает строку без завершающих символов конца строки.
///
/// del_eol: строка до первого `\r` или `\n`.
/// @param[in] line  строка файла
/// @return Текст до первого CR или LF, байты как Latin-1.
QString delEol(const QByteArray &line)
{
    int n = 0;
    while (n < line.size() && line[n] != '\r' && line[n] != '\n')
        ++n;
    // Тексты блоков — ASCII/CP1251; имена типов и выражения в CP1251 не встречаются
    // за пределами ASCII, Latin-1 сохраняет байты как есть.
    return QString::fromLatin1(line.constData(), n);
}

/// @brief Проверяет, начинается ли строка с ASCII-префикса.
/// @param[in] s       строка
/// @param[in] prefix  префикс (ASCII)
/// @return `true` — начинается.
bool startsWith(const QString &s, const char *prefix)
{
    return s.startsWith(QLatin1String(prefix));
}

/// @brief Читает сетку `double` из двоичной части файла.
///
/// Чтение сетки nY строк по nX double с пропуском 2 байт после строки (fread ch дважды).
/// @param[in,out] p    текущее место в буфере; сдвигается за прочитанную сетку
/// @param[in]     end  конец буфера
/// @param[in]     nX   узлов по X
/// @param[in]     nY   узлов по Y
/// @param[out]    out  значения `nX * nY`, строка за строкой
/// @return `false` — данных меньше, чем нужно (сетки нет).
bool readGrid(const char *&p, const char *end, int nX, int nY, std::vector<double> *out)
{
    const size_t n = size_t(nX) * size_t(nY);
    out->assign(n, 0.0);
    size_t ii = 0;
    for (int i = 0; i < nY; ++i) {
        for (int j = 0; j < nX; ++j) {
            if (end - p < 8)
                return false;
            std::memcpy(&(*out)[ii++], p, 8);
            p += 8;
        }
        // После последней строки последней сетки двух байт может не быть — как fread.
        p += (end - p >= 2) ? 2 : (end - p);
    }
    return true;
}

// --- CTmcBoundaryTpl ---------------------------------------------------------

/// @brief Добавляет к блоку полосу границы с типом блока.
/// @param[in,out] b   границы блока
/// @param[in]     x1  X вершины 1, м
/// @param[in]     y1  Y вершины 1, м
/// @param[in]     x2  X вершины 2, м
/// @param[in]     y2  Y вершины 2, м
/// @param[in]     x3  X вершины 3, м
/// @param[in]     y3  Y вершины 3, м
/// @param[in]     x4  X вершины 4, м
/// @param[in]     y4  Y вершины 4, м
void quad(BlockBoundary *b, double x1, double y1, double x2, double y2, double x3, double y3,
          double x4, double y4)
{
    TopQuad q;
    q.type = b->nType;
    q.x[0] = x1; q.y[0] = y1;
    q.x[1] = x2; q.y[1] = y2;
    q.x[2] = x3; q.y[2] = y3;
    q.x[3] = x4; q.y[3] = y4;
    b->quads.push_back(q);
}

/// @brief Добавляет к блоку размерную линию.
/// @param[in,out] b     границы блока
/// @param[in]     x1    X начала, м
/// @param[in]     x2    X конца, м
/// @param[in]     y1    Y начала, м
/// @param[in]     y2    Y конца, м
/// @param[in]     size  подписываемая длина, м
void addSize(BlockBoundary *b, double x1, double x2, double y1, double y2, double size)
{
    TopSize t;
    t.x1 = x1; t.x2 = x2; t.y1 = y1; t.y2 = y2; t.size = size;
    b->sizes.push_back(t);
}

/// @brief Строит границу прямоугольного блока: четыре полосы шириной `d` и размеры (высота, ширина).
/// @param[in]     bl  запись блока (`xMin` … `yMax` — габариты, м)
/// @param[in]     d   шаг сетки, м
/// @param[in,out] b   границы блока
void setRectStat(const TopBlock &bl, double d, BlockBoundary *b)
{
    const double xmin = bl.xMin, xmax = bl.xMax, ymin = bl.yMin, ymax = bl.yMax;
    // SetRectStat: высота по левой стороне и ширина по верхней.
    addSize(b, xmin, xmin, ymin, ymax, ymax - ymin);
    addSize(b, xmin, xmax, ymax, ymax, xmax - xmin);
    quad(b, xmin - d / 2, ymin - d / 2, xmin - d / 2, ymax + d / 2,
         xmin + d / 2, ymax + d / 2, xmin + d / 2, ymin - d / 2);
    quad(b, xmin - d / 2, ymax - d / 2, xmin - d / 2, ymax + d / 2,
         xmax + d / 2, ymax + d / 2, xmax + d / 2, ymax - d / 2);
    quad(b, xmax - d / 2, ymin - d / 2, xmax - d / 2, ymax + d / 2,
         xmax + d / 2, ymax + d / 2, xmax + d / 2, ymin - d / 2);
    quad(b, xmin - d / 2, ymin - d / 2, xmin - d / 2, ymin + d / 2,
         xmax + d / 2, ymin + d / 2, xmax + d / 2, ymin - d / 2);
}

/// @brief Строит границу многоугольника: полосы вдоль сторон и длины всех сторон.
///
/// Полоса стороны вытягивается вдоль её большей проекции (по X или по Y) и имеет ширину `d`.
/// closed — замкнуть многоугольник (в оригинале — только для типа ε).
/// @param[in]     bl      запись блока (вершины `px`, `py`, м)
/// @param[in]     d       шаг сетки, м
/// @param[in]     closed  добавить полосу замыкающей стороны
/// @param[in,out] b       границы блока
/// @return `false` — сторон меньше двух, полосы не построены.
bool setPolygonStat(const TopBlock &bl, double d, bool closed, BlockBoundary *b)
{
    const int nXY = int(bl.px.size());
    const int nPoint = closed ? nXY : nXY - 1;
    if (nPoint < 2)
        return false;   // в оригинале — ошибка "memory allocation in BlockList Tpl array"
    const std::vector<double> &pX = bl.px, &pY = bl.py;
    int i;
    // SetPolygonStat: длины всех сторон, включая замыкающую (у любого типа).
    for (int k = 0; k < nXY; ++k) {
        const size_t a = size_t(k), c = size_t(k + 1 < nXY ? k + 1 : 0);
        addSize(b, pX[a], pX[c], pY[a], pY[c],
                std::sqrt((pX[a] - pX[c]) * (pX[a] - pX[c]) + (pY[a] - pY[c]) * (pY[a] - pY[c])));
    }
    for (i = 0; i < nXY - 1; ++i) {
        if (std::fabs(pX[size_t(i)] - pX[size_t(i + 1)]) < std::fabs(pY[size_t(i)] - pY[size_t(i + 1)])) {
            quad(b, pX[size_t(i)] - d / 2, pY[size_t(i)] - d / 2,
                 pX[size_t(i + 1)] - d / 2, pY[size_t(i + 1)] + d / 2,
                 pX[size_t(i + 1)] + d / 2, pY[size_t(i + 1)] + d / 2,
                 pX[size_t(i)] + d / 2, pY[size_t(i)] - d / 2);
        } else {
            quad(b, pX[size_t(i)] - d / 2, pY[size_t(i)] - d / 2,
                 pX[size_t(i)] - d / 2, pY[size_t(i)] + d / 2,
                 pX[size_t(i + 1)] + d / 2, pY[size_t(i + 1)] + d / 2,
                 pX[size_t(i + 1)] + d / 2, pY[size_t(i + 1)] - d / 2);
        }
    }
    if (closed) {
        const size_t k = size_t(i);
        if (std::fabs(pX[k] - pX[0]) < std::fabs(pY[k] - pY[0])) {
            quad(b, pX[k] - d / 2, pY[k] - d / 2, pX[0] - d / 2, pY[0] + d / 2,
                 pX[0] + d / 2, pY[0] + d / 2, pX[k] + d / 2, pY[k] - d / 2);
        } else {
            quad(b, pX[k] - d / 2, pY[k] - d / 2, pX[k] - d / 2, pY[k] + d / 2,
                 pX[0] + d / 2, pY[0] + d / 2, pX[0] + d / 2, pY[0] - d / 2);
        }
    }
    return true;
}

/// @brief Строит полосу входа `INPUT_X` вдоль оси Y (от `xMin` до `xMax` записи) и его размер.
/// @param[in]     bl  запись блока (`xMin`, `xMax` — края апертуры по Y, м)
/// @param[in]     d   шаг сетки, м
/// @param[in,out] b   границы блока
void setInputX(const TopBlock &bl, double d, BlockBoundary *b)
{
    addSize(b, 0.0, 0.0, bl.xMin, bl.xMax, bl.xMax - bl.xMin);
    quad(b, -d / 2, bl.xMin - d / 2, -d / 2, bl.xMax + d / 2,
         d / 2, bl.xMax + d / 2, d / 2, bl.xMin - d / 2);
}

/// @brief Строит полосу входа `INPUT_Y` вдоль оси X (от `xMin` до `xMax`) и его размер.
/// @param[in]     bl  запись блока (`xMin`, `xMax` — края апертуры по X, м)
/// @param[in]     d   шаг сетки, м
/// @param[in,out] b   границы блока
void setInputY(const TopBlock &bl, double d, BlockBoundary *b)
{
    addSize(b, bl.xMin, bl.xMax, 0.0, 0.0, bl.xMax - bl.xMin);
    quad(b, bl.xMin - d / 2, -d / 2, bl.xMin - d / 2, d / 2,
         bl.xMax + d / 2, d / 2, bl.xMax + d / 2, -d / 2);
}

/// @brief Добавляет полосу дуги между двумя точками окружности (прямоугольник по их координатам).
///
/// Сегмент дуги: в оригинале вершины x1,y1 / x1,y2 / x2,y2 / x2,y1 (обе ветки одинаковы).
/// @param[in,out] b   границы блока
/// @param[in]     x1  X первой точки, м
/// @param[in]     y1  Y первой точки, м
/// @param[in]     x2  X второй точки, м
/// @param[in]     y2  Y второй точки, м
void arcQuad(BlockBoundary *b, double x1, double y1, double x2, double y2)
{
    quad(b, x1, y1, x1, y2, x2, y2, x2, y1);
}

/// @brief Добавляет полосу радиального отрезка шириной `d`.
///
/// Радиальный отрезок с полосой ±d/2 (ветки как в SetCircleStat).
/// @param[in,out] b   границы блока
/// @param[in]     x1  X начала, м
/// @param[in]     y1  Y начала, м
/// @param[in]     x2  X конца, м
/// @param[in]     y2  Y конца, м
/// @param[in]     d   шаг сетки, м
void radialQuad(BlockBoundary *b, double x1, double y1, double x2, double y2, double d)
{
    if (std::fabs(x1 - x2) < std::fabs(y1 - y2))
        quad(b, x1 - d / 2, y1 - d / 2, x2 - d / 2, y2 + d / 2, x2 + d / 2, y2 + d / 2,
             x1 + d / 2, y1 - d / 2);
    else
        quad(b, x1 - d / 2, y1 - d / 2, x1 - d / 2, y1 + d / 2, x2 + d / 2, y2 + d / 2,
             x2 + d / 2, y2 - d / 2);
}

/// @brief Строит границу кругового сектора (кольца): внешняя дуга, при полом круге — радиусы и внутренняя дуга.
///
/// Внешняя дуга делится на n - 1 отрезков, n = (int)(R * (f1 - f0) / d + 0,5) + 1 (не меньше 2);
/// внутренняя строится, только если внутренний радиус больше `d`.
/// @param[in]     bl  запись блока: `xMin`/`xMax` — внутренний/внешний радиус, м; `yMin`/`yMax` — углы, рад
/// @param[in]     d   шаг сетки, м
/// @param[in,out] b   границы блока
void setCircleStat(const TopBlock &bl, double d, BlockBoundary *b)
{
    const double rIn = bl.xMin, rOut = bl.xMax, f0 = bl.yMin, f1 = bl.yMax;
    // SetCircleStat: внешний радиус; при полом круге — ещё внутренний (вниз от центра).
    addSize(b, 0.0, 0.0, 0.0, rOut, rOut);
    if (std::fabs(rIn) > d)
        addSize(b, 0.0, 0.0, -rIn, 0.0, rIn);
    // Внешняя дуга: n = (int)(Xmax*(Ymax-Ymin)/dDelta + 0.5) + 1 точек. Н-35: в оригинале
    // рисовалось n отрезков по n − 1 промежуткам — дуга выходила за сектор на один отрезок
    // (у неполного круга это видно); здесь n − 1 отрезков от Ymin до Ymax, не меньше одного.
    int n = std::max(int(rOut * (f1 - f0) / d + 0.5) + 1, 2);
    double df = (f1 - f0) / (n - 1);
    double f = f0;
    for (int i = 0; i < n - 1; ++i, f += df)
        arcQuad(b, rOut * std::cos(f), rOut * std::sin(f), rOut * std::cos(f + df),
                rOut * std::sin(f + df));
    if (std::fabs(rIn) <= d)
        return;
    // Два радиальных отрезка — на углах Ymin и Ymax.
    radialQuad(b, rOut * std::cos(f0), rOut * std::sin(f0), rIn * std::cos(f0),
               rIn * std::sin(f0), d);
    radialQuad(b, rIn * std::cos(f1), rIn * std::sin(f1), rOut * std::cos(f1),
               rOut * std::sin(f1), d);
    // Внутренняя дуга.
    n = int(rIn * (f1 - f0) / d + 0.5);
    if (n < 1)
        return;
    if (n < 2) {
        arcQuad(b, rIn * std::cos(f0), rIn * std::sin(f0), rIn * std::cos(f1), rIn * std::sin(f1));
        return;
    }
    df = (f1 - f0) / (n - 1);
    f = f0;
    for (int i = 0; i < n - 1; ++i, f += df)   // Н-35: n − 1 отрезков, как у внешней дуги
        arcQuad(b, rIn * std::cos(f), rIn * std::sin(f), rIn * std::cos(f + df),
                rIn * std::sin(f + df));
}

} // namespace

bool TopologyFile::read(const QString &path, QString *error)
{
    // Сначала — пустой объект: при любой ошибке ниже он таким и останется.
    *this = TopologyFile();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error)
            *error = QStringLiteral("Can't open topology file %1").arg(path);
        return false;
    }
    const QByteArray all = file.readAll();
    const QString bad = QStringLiteral("Bad topology file %1").arg(path);
    LineReader r(all.constData(), all.size());
    QByteArray line;
    if (!r.next(&line) || !line.startsWith(kTopId) || !r.next(&line) || !line.startsWith(kListBegin)) {
        if (error)
            *error = bad;
        return false;
    }
    // CTmcRTH_BlockList::Load: блоки до #BlockListEnd.
    for (;;) {
        if (!r.next(&line)) {
            if (error)
                *error = bad;
            return false;
        }
        if (line.startsWith(kListEnd))
            break;
        TopBlock b;
        b.block = delEol(line);
        QByteArray l2, l3, l4, l5, nums;
        if (!r.next(&l2) || !r.next(&l3) || !r.next(&l4) || !r.next(&l5) || !r.next(&nums)) {
            if (error)
                *error = bad;
            return false;
        }
        b.eps = delEol(l2);
        b.vx = delEol(l3);
        b.vy = delEol(l4);
        b.w = delEol(l5);
        // "%lg %lg %lg %lg %lg %lg %d %d %d %d": dX0, dXmax, dXmin, dY0, dYmax, dYmin,
        // nBlock, nMemory, nType, nXY.
        // Шесть чисел — через scanDouble (как %lg); четыре целых — по словам строки.
        int pos = 0;
        double v[6];
        for (double &x : v) {
            int nx = pos;
            x = scanDouble(nums, pos, std::nan(""), &nx);
            pos = nx;
        }
        const QList<QByteArray> parts = nums.simplified().split(' ');
        if (parts.size() < 10 || std::isnan(v[0]) || std::isnan(v[5])) {
            if (error)
                *error = bad;
            return false;
        }
        b.x0 = v[0]; b.xMax = v[1]; b.xMin = v[2];
        b.y0 = v[3]; b.yMax = v[4]; b.yMin = v[5];
        b.nBlock = parts[6].toInt();
        b.nMemory = parts[7].toInt();
        b.nType = parts[8].toInt();
        b.nXY = parts[9].toInt();
        for (int i = 0; i < b.nXY; ++i) {
            QByteArray xy;
            if (!r.next(&xy)) {
                if (error)
                    *error = bad;
                return false;
            }
            int nx = 0;
            const double x = scanDouble(xy, 0, std::nan(""), &nx);
            const double y = scanDouble(xy, nx, std::nan(""));
            if (std::isnan(x) || std::isnan(y)) {
                if (error)
                    *error = bad;
                return false;
            }
            b.px.push_back(x);
            b.py.push_back(y);
        }
        // Узел списка с nBlock == 0 оригинал не считает блоком (GetBlockNumber).
        if (b.nBlock != 0)
            m_blocks.push_back(b);
    }
    // Заголовок сетки (без Accuracy: в .tt всегда double).
    const struct { const char *id; int kind; } hdr[] = {
        {kIdNT, 0}, {kIdT, 1}, {kIdDelta, 2}, {kIdXmin, 3}, {kIdYmin, 4}, {kIdNX, 5}, {kIdNY, 6}};
    for (const auto &h : hdr) {
        if (!r.next(&line) || !line.startsWith(h.id)) {
            if (error)
                *error = bad;
            return false;
        }
        const int at = int(std::strlen(h.id));
        switch (h.kind) {
        case 0: m_nT = scanInt(line, at, m_nT); break;
        case 1: m_time = scanDouble(line, at, m_time); break;
        case 2: m_delta = scanDouble(line, at, m_delta); break;
        case 3: m_xMin = scanDouble(line, at, m_xMin); break;
        case 4: m_yMin = scanDouble(line, at, m_yMin); break;
        case 5: m_nX = scanInt(line, at, m_nX); break;
        default: m_nY = scanInt(line, at, m_nY); break;
        }
    }
    if (m_nX < 2 || m_nY < 2) {
        if (error)
            *error = QStringLiteral("Bad topology file %1 - allocation memory error nX or nY < 2, or nX*nY < 4").arg(path);
        return false;
    }
    const char *p = all.constData() + r.position();
    const char *end = all.constData() + all.size();
    if (!readGrid(p, end, m_nX, m_nY, &m_eps)) {
        if (error)
            *error = bad;
        return false;
    }
    // X-мода: ещё N, Y, B. Нет — плазмы нет (bHasPlasma = 0), ε остаётся.
    m_hasPlasma = readGrid(p, end, m_nX, m_nY, &m_n) && readGrid(p, end, m_nX, m_nY, &m_y)
                  && readGrid(p, end, m_nX, m_nY, &m_b);
    if (!m_hasPlasma) {
        m_n.clear();
        m_y.clear();
        m_b.clear();
    }
    return true;
}

const std::vector<double> &TopologyFile::grid(int type) const
{
    if (m_hasPlasma) {
        switch (type) {
        case 1: return m_n;
        case 2: return m_y;
        case 3: return m_b;
        default: break;
        }
    }
    return m_eps;
}

std::vector<BlockBoundary> buildBoundaries(const std::vector<TopBlock> &blocks, double delta,
                                           bool plasmaOutlines)
{
    std::vector<BlockBoundary> out;
    out.reserve(blocks.size());
    int inputCounter = 0;
    for (const TopBlock &bl : blocks) {
        BlockBoundary b;
        b.nType = bl.nType;
        b.nBlock = bl.nBlock;
        // SetBlockList: привязка блока к началу координат (без сдвига X0, Y0).
        b.linkX0.x1 = 0.0; b.linkX0.x2 = bl.x0; b.linkX0.y1 = bl.y0; b.linkX0.y2 = bl.y0;
        b.linkX0.size = bl.x0;
        b.linkY0.x1 = 0.0; b.linkY0.x2 = 0.0; b.linkY0.y1 = 0.0; b.linkY0.y2 = bl.y0;
        b.linkY0.size = bl.y0;
        const QString &s = bl.block;
        // Порядок проверок — как в CTmcBoundaryTpl::SetBlockList.
        if (startsWith(s, "RECT_STAT ") || startsWith(s, "RECT_MOVE ")) {
            setRectStat(bl, delta, &b);
        } else if (startsWith(s, "FILE ")) {
            b.nType = kTypeEps;
            setRectStat(bl, delta, &b);
        } else if (startsWith(s, "POLYGON_STAT ") || startsWith(s, "POLYGON_MOVE ")) {
            setPolygonStat(bl, delta, b.nType == kTypeEps, &b);
        } else if (startsWith(s, "CIRCLE_STAT ") || startsWith(s, "CIRCLE_MOVE ")) {
            setCircleStat(bl, delta, &b);
        } else if (startsWith(s, "INPUT_X ")) {
            setInputX(bl, delta, &b);
        } else if (startsWith(s, "INPUT_Y ")) {
            setInputY(bl, delta, &b);
        } else if (plasmaOutlines) {
            // Не из оригинала: контуры плазменных блоков X-моды.
            if (startsWith(s, "RECT_STAT_N ") || startsWith(s, "RECT_STAT_B ") || startsWith(s, "RECT_STAT_Y ")) {
                b.nType = kTypePlasma;
                setRectStat(bl, delta, &b);
            } else if (startsWith(s, "POLYGON_STAT_N ") || startsWith(s, "POLYGON_STAT_B ")
                       || startsWith(s, "POLYGON_STAT_Y ")) {
                b.nType = kTypePlasma;
                setPolygonStat(bl, delta, true, &b);
            } else if (startsWith(s, "CIRCLE_STAT_N ") || startsWith(s, "CIRCLE_STAT_B ")
                       || startsWith(s, "CIRCLE_STAT_Y ")) {
                b.nType = kTypePlasma;
                setCircleStat(bl, delta, &b);
            } else if (startsWith(s, "FILE_N ") || startsWith(s, "FILE_B ") || startsWith(s, "FILE_Y ")) {
                // Н-35: плазма из файла — прямоугольник, как у блока FILE.
                b.nType = kTypePlasma;
                setRectStat(bl, delta, &b);
            }
        }
        // TranslateX0Y0 (контуры плазмы из оригинала размеров не имеют).
        if (b.nType == kTypePlasma)
            b.sizes.clear();
        for (TopQuad &q : b.quads) {
            for (int k = 0; k < 4; ++k) {
                q.x[k] += bl.x0;
                q.y[k] += bl.y0;
            }
        }
        for (TopSize &t : b.sizes) {
            t.x1 += bl.x0; t.x2 += bl.x0;
            t.y1 += bl.y0; t.y2 += bl.y0;
        }
        // ReadTpl: нумерация портов по порядку блоков.
        if (isInputType(b.nType))
            b.inputNumber = ++inputCounter;
        out.push_back(std::move(b));
    }
    return out;
}

void visibleTopology(const std::vector<BlockBoundary> &bounds, double xMin, double xMax,
                     double yMin, double yMax, double delta, const TopologyFilter &filter,
                     std::vector<TopQuad> *quads, std::vector<TopPort> *ports)
{
    quads->clear();
    ports->clear();
    const double xmin1 = xMin - 1.1 * delta;
    const double xmax1 = xMax + 1.1 * delta;
    const double ymin1 = yMin - 1.1 * delta;
    const double ymax1 = yMax + 1.1 * delta;
    for (const BlockBoundary &b : bounds) {
        const int t = b.nType;
        const bool shown = (t == kTypeEps && filter.eps) || (isInputType(t) && filter.input)
                           || (t == kTypeAbsorber && filter.absorber) || (t == kTypeMetal && filter.metal)
                           || (t == kTypeMagnetic && filter.magnetic) || (t == kTypePlasma && filter.plasma);
        if (!shown || b.quads.empty())
            continue;
        if (isInputType(t)) {
            const TopQuad &q = b.quads.front();
            TopPort p;
            p.number = b.inputNumber;
            p.x = (q.x[0] + q.x[1] + q.x[2] + q.x[3]) / 4.;
            p.y = (q.y[0] + q.y[1] + q.y[2] + q.y[3]) / 4.;
            ports->push_back(p);
        }
        for (const TopQuad &q : b.quads) {
            bool in = false;
            for (int k = 0; k < 4 && !in; ++k)
                in = q.x[k] < xmax1 && q.x[k] > xmin1 && q.y[k] < ymax1 && q.y[k] > ymin1;
            if (in)
                quads->push_back(q);
        }
    }
}

std::vector<TopSize> visibleSizes(const std::vector<BlockBoundary> &bounds, double xMin, double xMax,
                                  double yMin, double yMax, double delta, bool block, bool linkList)
{
    std::vector<TopSize> out;
    const double xmin1 = xMin - 1.1 * delta, xmax1 = xMax + 1.1 * delta;
    const double ymin1 = yMin - 1.1 * delta, ymax1 = yMax + 1.1 * delta;
    const auto inside = [&](const TopSize &t) {
        return t.x1 < xmax1 && t.x1 > xmin1 && t.y1 < ymax1 && t.y1 > ymin1 && t.x2 < xmax1
               && t.x2 > xmin1 && t.y2 < ymax1 && t.y2 > ymin1 && t.size > 2 * delta;
    };
    for (const BlockBoundary &b : bounds) {
        if (linkList) {
            if (inside(b.linkX0))
                out.push_back(b.linkX0);
            if (inside(b.linkY0))
                out.push_back(b.linkY0);
        }
        if (block) {
            for (const TopSize &t : b.sizes)
                if (inside(t))
                    out.push_back(t);
        }
    }
    return out;
}

} // namespace fieldview
