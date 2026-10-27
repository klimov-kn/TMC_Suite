# TPL Designer — редактор файлов задания .tpl. Входит в пакет с 27.10.2026 и
# собирается вместе с остальными программами; до этого был отдельным проектом
# (I:/Python Project/TPL_Designer — теперь архив, там не править).
#
# Собирается на Windows (MinGW), macOS (clang) и Linux (gcc/clang) одним и тем же
# набором файлов: платформенных ветвей всего три — ресурс значка в src/gui, пакет .app
# и раскладка установки. Ядро генератора (src/core) системно-независимо целиком, и это
# не случайность: файл задания .tpl обязан получаться ОДИНАКОВЫМ на любой системе,
# иначе «одно и то же задание» начинает зависеть от того, где его сделали.
#
# Расчётного кода пакета дизайнер не линкует (пишет текст, ядра запускает процессами),
# поэтому настройки сборки MFC-версии (слой MFC, _MBCS, -UUNICODE, -ffp-contract=off)
# здесь сброшены: дизайнер собирается так же, как собирался отдельно.
set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS "")
set_property(DIRECTORY PROPERTY COMPILE_OPTIONS "")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Версия дизайнера = версия пакета (используется в пакете .app на macOS).
set(PROJECT_VERSION 5.1.0)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)

# STEP через OpenCASCADE — ОБЯЗАТЕЛЬНО (решение 27.10.2026): без него сборка пакета
# останавливается. Папку с OpenCASCADEConfig.cmake передают в OpenCASCADE_DIR
# (build_win.sh задаёт её сам). Отключить можно только явно: -DTPLD_WITH_STEP=OFF.
option(TPLD_WITH_STEP "Читать STEP через OpenCASCADE (обязательная зависимость пакета)" ON)
if (TPLD_WITH_STEP)
    find_package(OpenCASCADE CONFIG)
    if (NOT OpenCASCADE_FOUND)
        message(FATAL_ERROR
            "TPL Designer: не найден OpenCASCADE (импорт STEP). Укажите папку с "
            "OpenCASCADEConfig.cmake: -DOpenCASCADE_DIR=<occt>/install/cmake "
            "(или -DTPLD_WITH_STEP=OFF — сборка без импорта STEP).")
    endif()
endif()

add_subdirectory(src/core)
add_subdirectory(src/gui)
add_subdirectory(tools)
