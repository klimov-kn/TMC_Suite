// Запуск — описание в startup.h.
/// @file startup.cpp
/// @brief Выбор графического интерфейса Qt RHI с проверкой и запасными вариантами; настройка приложения.
///
/// Часть модуля FieldView (слой `app`). Описание функций — в `startup.h`.
/// @see https://www.tamic.ru/docs/code/fieldview.html#rhi
#include "startup.h"
#include "fieldrhiview.h"
#include "theme.h"

#include <QApplication>
#include <QOffscreenSurface>
#include <QSettings>
#include <rhi/qrhi.h>

#include <memory>

namespace fieldview {

namespace {

/// @brief Проверяет, создаётся ли объект RHI с заданным интерфейсом.
///
/// Объект создаётся и сразу удаляется; для OpenGL нужна запасная поверхность.
/// @param[in] impl   интерфейс: D3D11, D3D12 (только Windows) или OpenGLES2
/// @param[in] flags  флаги создания (`PreferSoftwareRenderer` — программный WARP)
/// @return `true` — создан; для других интерфейсов — `false`.
bool probe(QRhi::Implementation impl, QRhi::Flags flags)
{
    std::unique_ptr<QRhi> rhi;
    switch (impl) {
#ifdef Q_OS_WIN
    case QRhi::D3D11: {
        QRhiD3D11InitParams p;
        rhi.reset(QRhi::create(QRhi::D3D11, &p, flags));
        break;
    }
    case QRhi::D3D12: {
        QRhiD3D12InitParams p;
        rhi.reset(QRhi::create(QRhi::D3D12, &p, flags));
        break;
    }
#endif
    case QRhi::OpenGLES2: {
        std::unique_ptr<QOffscreenSurface> s(QRhiGles2InitParams::newFallbackSurface());
        QRhiGles2InitParams p;
        p.fallbackSurface = s.get();
        rhi.reset(QRhi::create(QRhi::OpenGLES2, &p, flags));
        const bool ok = rhi != nullptr;
        rhi.reset();
        return ok;
    }
    default:
        return false;
    }
    return rhi != nullptr;
}

} // namespace

QString chooseGraphicsApi(QStringList *args)
{
    QString want = qEnvironmentVariable("FIELDVIEW_RHI").toLower();
    bool warp = false;
    for (int i = 0; i < args->size();) {
        const QString a = args->at(i);
        if (a == QLatin1String("--rhi") && i + 1 < args->size()) {
            want = args->at(i + 1).toLower();
            args->removeAt(i);
            args->removeAt(i);
        } else if (a.startsWith(QLatin1String("--rhi="))) {
            want = a.mid(6).toLower();
            args->removeAt(i);
        } else if (a == QLatin1String("--warp")) {
            warp = true;
            args->removeAt(i);
        } else {
            ++i;
        }
    }
    if (want == QLatin1String("opengl") || want == QLatin1String("gl")) {
        FieldRhiView::setPreferredApi(QRhiWidget::Api::OpenGL);
        return QStringLiteral("OpenGL");
    }
    if (want == QLatin1String("vulkan")) {
        FieldRhiView::setPreferredApi(QRhiWidget::Api::Vulkan);
        return QStringLiteral("Vulkan");
    }
#ifdef Q_OS_WIN
    if (want == QLatin1String("d3d12") && probe(QRhi::D3D12, {})) {
        FieldRhiView::setPreferredApi(QRhiWidget::Api::Direct3D12);
        return QStringLiteral("D3D12");
    }
    if (!warp && probe(QRhi::D3D11, {})) {
        FieldRhiView::setPreferredApi(QRhiWidget::Api::Direct3D11);
        return QStringLiteral("D3D11");
    }
    // Нет видеокарты с Direct3D 11 (сервер, удалённый рабочий стол) — программный WARP.
    if (probe(QRhi::D3D11, QRhi::PreferSoftwareRenderer)) {
        qputenv("QSG_RHI_PREFER_SOFTWARE_RENDERER", "1");
        FieldRhiView::setPreferredApi(QRhiWidget::Api::Direct3D11);
        return QStringLiteral("D3D11 (WARP)");
    }
#endif
    FieldRhiView::setPreferredApi(QRhiWidget::Api::OpenGL);
    return probe(QRhi::OpenGLES2, {}) ? QStringLiteral("OpenGL") : QStringLiteral("OpenGL (не проверен)");
}

void setupApplication(QApplication &app)
{
    QApplication::setApplicationName(QStringLiteral("FieldView"));
#ifdef Q_OS_WIN
    // Настройки — в INI рядом с настройками остальных программ пакета:
    // %APPDATA%\TMC_Suite_Qt\FieldView.ini (раньше — реестр «HKCU\Software\TMC Suite\FieldView»;
    // деинсталлятор, удаляющий папку настроек, их не находил, Т-27).
    // Путь QSettings: <папка>/<организация>/<программа>.ini — поэтому папка — APPDATA,
    // организация — TMC_Suite_Qt.
    QApplication::setOrganizationName(QStringLiteral("TMC_Suite_Qt"));
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, qEnvironmentVariable("APPDATA"));
    {
        // Однократный перенос из реестра прежних версий.
        QSettings ini;
        if (ini.allKeys().isEmpty()) {
            QSettings reg(QStringLiteral("HKEY_CURRENT_USER\\Software\\TMC Suite\\FieldView"),
                          QSettings::NativeFormat);
            const QStringList keys = reg.allKeys();
            for (const QString &k : keys)
                ini.setValue(k, reg.value(k));
        }
    }
#else
    QApplication::setOrganizationName(QStringLiteral("TMC Suite"));
#endif
    QApplication::setApplicationDisplayName(QStringLiteral("FieldView"));
    QApplication::setApplicationVersion(QStringLiteral("5.0"));
    theme::apply(app);
}

} // namespace fieldview
