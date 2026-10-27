/// @file settings.cpp
/// @brief Чтение и запись настроек редактора, палитры заливки eps.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-settings
#include "settings.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QStringList>

#include <algorithm>
#include <array>
#include "lang.h"

namespace settings {
namespace {

/// Опорные точки палитр. Промежуточные цвета берутся линейной интерполяцией: на
/// глаз этого достаточно, а таблица из 256 записей на каждую палитру только мешала бы
/// её править.
/// @brief Опорная точка палитры: положение и цвет.
struct Stop {
    /// Положение на шкале, безразмерное, [0, 1]; точки идут по возрастанию.
    double t;
    /** @var r
        Красная компонента, 0...255. */
    /** @var g
        Зелёная компонента, 0...255. */
    /** @var b
        Синяя компонента, 0...255. */
    int r, g, b;
};

/// Опорные точки палитры Viridis.
const std::array<Stop, 5> kViridis{{
    {0.00, 0x44, 0x01, 0x54},
    {0.25, 0x3b, 0x51, 0x8b},
    {0.50, 0x21, 0x91, 0x8c},
    {0.75, 0x5e, 0xc9, 0x62},
    {1.00, 0xfd, 0xe7, 0x25},
}};

/// Опорные точки тепловой палитры (чёрный -> красный -> жёлтый -> белый).
const std::array<Stop, 5> kHeat{{
    {0.00, 0x0a, 0x0e, 0x16},
    {0.30, 0x8b, 0x1a, 0x1a},
    {0.60, 0xe8, 0x6a, 0x17},
    {0.85, 0xf5, 0xd0, 0x42},
    {1.00, 0xff, 0xff, 0xf0},
}};

/// Опорные точки серой палитры.
const std::array<Stop, 2> kGray{{
    {0.00, 0x18, 0x1d, 0x26},
    {1.00, 0xf0, 0xf4, 0xfa},
}};

/// @brief Интерполирует цвет между опорными точками палитры.
/// @tparam N  число опорных точек.
/// @param[in] stops  опорные точки по возрастанию `t`.
/// @param[in] t      положение на шкале; зажимается в [0, 1].
/// @return Цвет между соседними точками (линейно по каждой компоненте, с округлением).
template <size_t N>
QColor interpolate(const std::array<Stop, N> &stops, double t)
{
    t = std::clamp(t, 0.0, 1.0);
    for (size_t i = 1; i < N; ++i) {
        if (t > stops[i].t)
            continue;
        const Stop &a = stops[i - 1];
        const Stop &b = stops[i];
        const double span = b.t - a.t;
        const double k = (span > 0.0) ? (t - a.t) / span : 0.0;
        return QColor(int(a.r + (b.r - a.r) * k + 0.5),
                      int(a.g + (b.g - a.g) * k + 0.5),
                      int(a.b + (b.b - a.b) * k + 0.5));
    }
    return QColor(stops[N - 1].r, stops[N - 1].g, stops[N - 1].b);
}

#ifdef Q_OS_WIN
/// Один INI для всех настроек программы (см. installStorage в settings.h).
/// @brief Возвращает путь INI-файла настроек.
/// @return `%APPDATA%/TMC_Suite_Qt/TPL Designer.ini`.
QString iniPath()
{
    return QDir(qEnvironmentVariable("APPDATA")).filePath(QStringLiteral("TMC_Suite_Qt/TPL Designer.ini"));
}
#endif

/// @brief Открывает хранилище настроек редактора.
/// @return INI-файл `iniPath()` на Windows, иначе `QSettings("TMC", "TplDesigner")`.
QSettings store()
{
    // Файл задаётся здесь, а не в main: настройки должны читаться и из служебных
    // утилит, где QApplication настраивают иначе.
#ifdef Q_OS_WIN
    return QSettings(iniPath(), QSettings::IniFormat);
#else
    return QSettings(QStringLiteral("TMC"), QStringLiteral("TplDesigner"));
#endif
}

/// @brief Читает настройки из хранилища; отсутствующие ключи берутся по умолчанию.
/// @return Прочитанные значения; номер палитры зажимается в 0...2.
Values load()
{
    QSettings s = store();
    Values v;
    s.beginGroup(QStringLiteral("view"));
    v.epsColorBar = s.value(QStringLiteral("epsColorBar"), v.epsColorBar).toBool();
    v.epsTooltip = s.value(QStringLiteral("epsTooltip"), v.epsTooltip).toBool();
    v.epsGradient = s.value(QStringLiteral("epsGradient"), v.epsGradient).toBool();
    v.palette = Palette(std::clamp(s.value(QStringLiteral("palette"), 0).toInt(), 0, 2));
    v.gridVisible = s.value(QStringLiteral("grid"), v.gridVisible).toBool();
    v.snapEnabled = s.value(QStringLiteral("snap"), v.snapEnabled).toBool();
    v.showDimensions = s.value(QStringLiteral("dimensions"), v.showDimensions).toBool();
    s.endGroup();
    s.beginGroup(QStringLiteral("behaviour"));
    v.confirmOverwrite = s.value(QStringLiteral("confirmOverwrite"), v.confirmOverwrite).toBool();
    s.endGroup();
    return v;
}

/// Значения настроек процесса (кэш хранилища).
Values g_values;
/// true — `g_values` уже прочитаны или заданы.
bool g_loaded = false;

} // namespace

QString paletteName(Palette p)
{
    switch (p) {
    case Palette::Viridis: return tpl::tr("Viridis (равномерная)", "Viridis (uniform)");
    case Palette::Heat:    return tpl::tr("Тепловая", "Heat");
    case Palette::Gray:    return tpl::tr("Серая", "Gray");
    }
    return {};
}

QColor paletteColor(Palette p, double t)
{
    switch (p) {
    case Palette::Viridis: return interpolate(kViridis, t);
    case Palette::Heat:    return interpolate(kHeat, t);
    case Palette::Gray:    return interpolate(kGray, t);
    }
    return Qt::black;
}

const Values &values()
{
    if (!g_loaded) {
        g_values = load();
        g_loaded = true;
    }
    return g_values;
}

void setValues(const Values &v)
{
    g_values = v;
    g_loaded = true;

    QSettings s = store();
    s.beginGroup(QStringLiteral("view"));
    s.setValue(QStringLiteral("epsColorBar"), v.epsColorBar);
    s.setValue(QStringLiteral("epsTooltip"), v.epsTooltip);
    s.setValue(QStringLiteral("epsGradient"), v.epsGradient);
    s.setValue(QStringLiteral("palette"), int(v.palette));
    s.setValue(QStringLiteral("grid"), v.gridVisible);
    s.setValue(QStringLiteral("snap"), v.snapEnabled);
    s.setValue(QStringLiteral("dimensions"), v.showDimensions);
    s.endGroup();
    s.beginGroup(QStringLiteral("behaviour"));
    s.setValue(QStringLiteral("confirmOverwrite"), v.confirmOverwrite);
    s.endGroup();
}

void installStorage()
{
#ifdef Q_OS_WIN
    // QSettings() без доводов -> <APPDATA>/<организация>/<программа>.ini, то есть
    // %APPDATA%\TMC_Suite_Qt\TPL Designer.ini — тот же файл, что у store().
    QCoreApplication::setOrganizationName(QStringLiteral("TMC_Suite_Qt"));
    QCoreApplication::setApplicationName(QStringLiteral("TPL Designer"));
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, qEnvironmentVariable("APPDATA"));
    QSettings ini(iniPath(), QSettings::IniFormat);
    if (!ini.allKeys().isEmpty())
        return;
    // Однократный перенос из реестра прежних версий (два ключа, Т-27).
    for (const char *key : {"HKEY_CURRENT_USER\\Software\\TMC\\TplDesigner",
                            "HKEY_CURRENT_USER\\Software\\TMC\\TPL Designer"}) {
        QSettings reg(QString::fromLatin1(key), QSettings::NativeFormat);
        const QStringList keys = reg.allKeys();
        for (const QString &k : keys)
            if (!ini.contains(k))
                ini.setValue(k, reg.value(k));
    }
#endif
}

} // namespace settings
