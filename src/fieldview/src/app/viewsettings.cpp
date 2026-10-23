// Настройки вида — описание в viewsettings.h.
/// @file viewsettings.cpp
/// @brief Значения по умолчанию, чтение и запись настроек вида FieldView.
///
/// Часть модуля FieldView (слой `app`). Описание структур — в `viewsettings.h`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#settings
#include "viewsettings.h"
#include "theme.h"

#include <QSettings>
#include <QStringList>

#include <cmath>

namespace fieldview {

namespace {

// Раздел и имена ключей — из FldViewView.h Windows-версии.
/// Раздел настроек вида.
const char kGroup[] = "TMC_GraphicsOutputFieldFile";

/// @brief Записывает 20 цветов строкой вида `#rrggbb #rrggbb …`.
/// @param[in] c  цвета
/// @return Строка через пробел.
QString colorsToString(const Colors20 &c)
{
    QStringList l;
    for (const QColor &x : c)
        l << x.name();
    return l.join(QLatin1Char(' '));
}

/// @brief Читает 20 цветов из строки; при другом числе слов ничего не меняет, неверные цвета пропускает.
/// @param[in]     s  строка
/// @param[in,out] c  цвета
void colorsFromString(const QString &s, Colors20 *c)
{
    const QStringList l = s.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (l.size() != 20)
        return;
    for (int i = 0; i < 20; ++i) {
        const QColor x(l.at(i));
        if (x.isValid())
            (*c)[size_t(i)] = x;
    }
}

/// @brief Записывает общие настройки слоя строкой из 10 чисел.
/// @param[in] l  настройки слоя
/// @return «surface line dim3d blend zAuto zMin zMax drawZSize scale smooth».
QString layerToString(const LayerSettings &l)
{
    return QStringLiteral("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10")
        .arg(int(l.surface)).arg(int(l.line)).arg(int(l.dim3d)).arg(l.blend).arg(int(l.zAuto))
        .arg(QString::number(l.zMin, 'g', 17), QString::number(l.zMax, 'g', 17))
        .arg(int(l.drawZSize)).arg(int(l.scale)).arg(int(l.smooth));
}

/// @brief Читает общие настройки слоя из строки (меньше 10 слов — ничего не меняет).
///
/// Непрозрачность обрезается до 0 … 100, шкала — до 0 … 3; нечисловые пределы
/// включают автоматические пределы.
/// @param[in]     s  строка
/// @param[in,out] l  настройки слоя
void layerFromString(const QString &s, LayerSettings *l)
{
    const QStringList p = s.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (p.size() < 10)
        return;
    l->surface = p[0].toInt();
    l->line = p[1].toInt();
    l->dim3d = p[2].toInt();
    l->blend = qBound(0, p[3].toInt(), 100);
    l->zAuto = p[4].toInt();
    bool ok1 = false, ok2 = false;
    const double z0 = p[5].toDouble(&ok1), z1 = p[6].toDouble(&ok2);
    if (ok1 && ok2 && std::isfinite(z0) && std::isfinite(z1)) {
        l->zMin = z0;
        l->zMax = z1;
    } else {
        // С-57: «inf»/«nan» в настройках (пределы от кадра с nan/inf) не берём — шкала
        // снова считается по данным.
        l->zAuto = true;
    }
    l->drawZSize = p[7].toInt();
    l->scale = ScaleKind(qBound(0, p[8].toInt(), 3));
    l->smooth = p[9].toInt();
}

} // namespace

ViewSettings ViewSettings::defaults()
{
    ViewSettings v;
    v.field.custom = classicFieldColors();
    v.field.zMin = -1.0;   // SetDefaultValue: dZmin = −1, dZmax = 0 (дальше — авто)
    v.field.zMax = 0.0;
    v.eps.surface = false;
    v.eps.line = false;
    v.eps.dim3d = false;
    v.eps.blend = 80;      // nBlend_Eps
    v.eps.zMin = 0.0;
    v.eps.zMax = -1.0;
    v.eps.scale = ScaleKind::Glow;
    v.eps.custom = classicEpsColors();
    for (int i = 0; i < 6; ++i)
        v.topo.colors[i] = defaultTopologyColor(i);
    v.place.axesColor = theme::accent3();
    v.place.axesFont = theme::uiFont(13, QFont::DemiBold);
    return v;
}

ViewSettings ViewSettings::load()
{
    ViewSettings v = defaults();
    QSettings s;
    s.beginGroup(QString::fromLatin1(kGroup));
    if (!s.contains(QStringLiteral("TMC_GraphOutFld_Field"))) {
        s.endGroup();
        return v;
    }
    layerFromString(s.value(QStringLiteral("TMC_GraphOutFld_Field")).toString(), &v.field);
    v.field.modulus = s.value(QStringLiteral("TMC_GraphOutFld_DrawFieldModulFlag"), false).toBool();
    v.field.zPerFrame = s.value(QStringLiteral("TMC_GraphOutFld_ZPerFrame"), false).toBool();
    v.field.heightScale = qBound(0.05, s.value(QStringLiteral("TMC_GraphOutFld_HeightScale"), 1.0).toDouble(), 10.0);
    v.field.gain = qBound(1.0, s.value(QStringLiteral("TMC_GraphOutFld_Gain"), 1.0).toDouble(), 100.0);
    colorsFromString(s.value(QStringLiteral("TMC_GraphOutFld_ColorZField")).toString(), &v.field.custom);
    layerFromString(s.value(QStringLiteral("TMC_GraphOutFld_EpsParam")).toString(), &v.eps);
    v.eps.type = qBound(0, s.value(QStringLiteral("TMC_GraphOutFld_EpsType"), 0).toInt(), 3);
    colorsFromString(s.value(QStringLiteral("TMC_GraphOutFld_ColorZEps")).toString(), &v.eps.custom);

    const QStringList t = s.value(QStringLiteral("TMC_GraphOutFld_Topology")).toString().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (t.size() >= 12) {
        v.topo.line = t[0].toInt();
        v.topo.surface = t[1].toInt();
        v.topo.dim3d = t[2].toInt();
        v.topo.blend = qBound(0, t[3].toInt(), 100);
        v.topo.sizeBlock = t[4].toInt();
        v.topo.sizeLinkList = t[5].toInt();
        v.topo.magnetic = t[6].toInt();
        v.topo.metal = t[7].toInt();
        v.topo.eps = t[8].toInt();
        v.topo.input = t[9].toInt();
        v.topo.absorber = t[10].toInt();
        v.topo.plasma = t[11].toInt();
    }
    const QStringList tc = s.value(QStringLiteral("TMC_GraphOutFld_TopologyColor_EpsMetMagInp")).toString().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (int i = 0; i < 6 && i < tc.size(); ++i) {
        const QColor c(tc[i]);
        if (c.isValid())
            v.topo.colors[i] = c;
    }

    const QStringList p = s.value(QStringLiteral("TMC_GraphOutFld_Place")).toString().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (p.size() >= 10) {
        v.place.xAuto = p[0].toInt();
        v.place.yAuto = p[1].toInt();
        v.place.xMin = p[2].toDouble();
        v.place.xMax = p[3].toDouble();
        v.place.yMin = p[4].toDouble();
        v.place.yMax = p[5].toDouble();
        // Нечисловое окно данных — показывать весь файл.
        if (!std::isfinite(v.place.xMin) || !std::isfinite(v.place.xMax))
            v.place.xAuto = true;
        if (!std::isfinite(v.place.yMin) || !std::isfinite(v.place.yMax))
            v.place.yAuto = true;
        v.place.drawAxes = p[6].toInt();
        v.place.drawSizeX = p[7].toInt();
        v.place.drawSizeY = p[8].toInt();
        v.place.proportional = p[9].toInt();
    }
    const QColor ac(s.value(QStringLiteral("TMC_GraphOutFld_ColorAxies")).toString());
    if (ac.isValid())
        v.place.axesColor = ac;
    QFont f;
    if (f.fromString(s.value(QStringLiteral("TMC_GraphOutFld_FontAxies")).toString()))
        v.place.axesFont = f;

    const QStringList c = s.value(QStringLiteral("TMC_GraphOutFld_Camera")).toString().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (c.size() >= 7) {
        v.camera.angleX = c[0].toDouble();
        v.camera.angleY = c[1].toDouble();
        v.camera.angleZ = c[2].toDouble();
        v.camera.transX = c[3].toDouble();
        v.camera.transY = c[4].toDouble();
        v.camera.transZ = c[5].toDouble();
        v.camera.zoom = qBound(0.05, c[6].toDouble(), 50.0);
    }
    v.probe = s.value(QStringLiteral("TMC_GraphOutFld_FieldValue"), false).toBool();
    v.propertyPage = s.value(QStringLiteral("TMC_GraphOutFld_PropertyPageNumber"), 0).toInt();
    v.speed = qBound(1, s.value(QStringLiteral("TMC_GraphOutFld_Speed"), 10).toInt(), 60);
    s.endGroup();
    return v;
}

void ViewSettings::save() const
{
    QSettings s;
    s.beginGroup(QString::fromLatin1(kGroup));
    s.setValue(QStringLiteral("TMC_GraphOutFld_Field"), layerToString(field));
    s.setValue(QStringLiteral("TMC_GraphOutFld_DrawFieldModulFlag"), field.modulus);
    s.setValue(QStringLiteral("TMC_GraphOutFld_ZPerFrame"), field.zPerFrame);
    s.setValue(QStringLiteral("TMC_GraphOutFld_HeightScale"), field.heightScale);
    s.setValue(QStringLiteral("TMC_GraphOutFld_Gain"), field.gain);
    s.setValue(QStringLiteral("TMC_GraphOutFld_ColorZField"), colorsToString(field.custom));
    s.setValue(QStringLiteral("TMC_GraphOutFld_EpsParam"), layerToString(eps));
    s.setValue(QStringLiteral("TMC_GraphOutFld_EpsType"), eps.type);
    s.setValue(QStringLiteral("TMC_GraphOutFld_ColorZEps"), colorsToString(eps.custom));
    s.setValue(QStringLiteral("TMC_GraphOutFld_Topology"),
               QStringLiteral("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12")
                   .arg(int(topo.line)).arg(int(topo.surface)).arg(int(topo.dim3d)).arg(topo.blend)
                   .arg(int(topo.sizeBlock)).arg(int(topo.sizeLinkList)).arg(int(topo.magnetic))
                   .arg(int(topo.metal)).arg(int(topo.eps)).arg(int(topo.input)).arg(int(topo.absorber))
                   .arg(int(topo.plasma)));
    QStringList tc;
    for (const QColor &c : topo.colors)
        tc << c.name();
    s.setValue(QStringLiteral("TMC_GraphOutFld_TopologyColor_EpsMetMagInp"), tc.join(QLatin1Char(' ')));
    s.setValue(QStringLiteral("TMC_GraphOutFld_Place"),
               QStringLiteral("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10")
                   .arg(int(place.xAuto)).arg(int(place.yAuto))
                   .arg(QString::number(place.xMin, 'g', 17), QString::number(place.xMax, 'g', 17),
                        QString::number(place.yMin, 'g', 17), QString::number(place.yMax, 'g', 17))
                   .arg(int(place.drawAxes)).arg(int(place.drawSizeX)).arg(int(place.drawSizeY))
                   .arg(int(place.proportional)));
    s.setValue(QStringLiteral("TMC_GraphOutFld_ColorAxies"), place.axesColor.name());
    s.setValue(QStringLiteral("TMC_GraphOutFld_FontAxies"), place.axesFont.toString());
    s.setValue(QStringLiteral("TMC_GraphOutFld_Camera"),
               QStringLiteral("%1 %2 %3 %4 %5 %6 %7")
                   .arg(camera.angleX).arg(camera.angleY).arg(camera.angleZ).arg(camera.transX)
                   .arg(camera.transY).arg(camera.transZ).arg(camera.zoom));
    s.setValue(QStringLiteral("TMC_GraphOutFld_FieldValue"), probe);
    s.setValue(QStringLiteral("TMC_GraphOutFld_PropertyPageNumber"), propertyPage);
    s.setValue(QStringLiteral("TMC_GraphOutFld_Speed"), speed);
    s.endGroup();
}

} // namespace fieldview
