# FieldView — просмотр полей TMC Suite на Qt 6 с выводом через Qt RHI.
#
# Самостоятельный проект (собирается и сам по себе, и как подкаталог общей сборки):
#   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/Qt/6.11.1/mingw_64
#   cmake --build build
#
# Цели:
#   fieldview_app   — статическая библиотека: чтение файлов, сцена RHI, окна;
#   fieldview       — программа (fieldview.exe);

cmake_minimum_required(VERSION 3.21)
project(FieldViewRhi VERSION 5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# QRhi — закрытый модуль QtGui (Qt6::GuiPrivate); версия Qt для поставки фиксируется.
set(QT_NO_PRIVATE_MODULE_WARNING ON)
find_package(Qt6 6.7 REQUIRED COMPONENTS Core Gui GuiPrivate Widgets PrintSupport ShaderTools Svg)
qt_standard_project_setup()

set(FV_ICON_DIR "${CMAKE_SOURCE_DIR}/src/resources/icons/toolbar/fieldview"
    CACHE PATH "Каталог значков панели инструментов FieldView (имена файлов = ID команд)")

add_library(fieldview_app STATIC
    src/core/textscan.h
    src/core/fieldfile.h src/core/fieldfile.cpp
    src/core/topology.h src/core/topology.cpp
    src/core/palette.h src/core/palette.cpp
    src/core/gifwriter.h src/core/gifwriter.cpp
    src/app/theme.h src/app/theme.cpp
    src/app/viewsettings.h src/app/viewsettings.cpp
    src/app/fielddocument.h src/app/fielddocument.cpp
    src/app/fieldpane.h src/app/fieldpane.cpp
    src/app/startup.h src/app/startup.cpp
    src/render/scenedata.h
    src/render/fieldrhiview.h src/render/fieldrhiview.cpp
    src/ui/icons.h src/ui/icons.cpp
    src/ui/autocolordialog.h src/ui/autocolordialog.cpp
    src/ui/parametersdialog.h src/ui/parametersdialog.cpp
    src/ui/mainwindow.h src/ui/mainwindow.cpp
)
target_include_directories(fieldview_app PUBLIC
    src/core src/app src/render src/ui)
# QRhi (rhi/qrhi.h) — среди закрытых заголовков QtGui.
target_link_libraries(fieldview_app PUBLIC
    Qt6::Core Qt6::Gui Qt6::GuiPrivate Qt6::Widgets Qt6::PrintSupport Qt6::Svg)
# Единое окно «О программе» и язык интерфейса — общие для всех программ пакета.
if(TARGET tmc_gui_common)
    target_link_libraries(fieldview_app PUBLIC tmc_gui_common)
    target_compile_definitions(fieldview_app PUBLIC TMC_FV_COMMON_ABOUT)
endif()
if(MSVC)
    target_compile_options(fieldview_app PRIVATE /W4)
else()
    target_compile_options(fieldview_app PRIVATE -Wall -Wextra)
endif()

# Шейдеры пишутся один раз (GLSL 440); qsb переводит их для каждого интерфейса:
# HLSL (Direct3D 11/12), SPIR-V (Vulkan), GLSL (OpenGL), MSL (Metal).
qt_add_shaders(fieldview_app "fieldview_shaders"
    PREFIX "/fieldview"
    GLSL "300es,330"
    HLSL 50
    MSL 12
    FILES
        shaders/grid.vert
        shaders/grid.frag
        shaders/color.vert
        shaders/color.frag
        shaders/sprite.vert
        shaders/sprite.frag
)

# Шрифты (SIL OFL 1.1) и элементы оформления.
qt_add_resources(fieldview_app "fieldview_fonts"
    PREFIX "/fonts"
    BASE resources/fonts
    FILES
        resources/fonts/Inter-Regular.ttf
        resources/fonts/Inter-SemiBold.ttf
        resources/fonts/Inter-Bold.ttf
        resources/fonts/Inter-ExtraBold.ttf
        resources/fonts/JetBrainsMono-Regular.ttf
        resources/fonts/JetBrainsMono-Bold.ttf
)
qt_add_resources(fieldview_app "fieldview_ui"
    PREFIX "/ui"
    BASE resources/ui
    FILES
        resources/ui/arrow_up.svg
        resources/ui/arrow_down.svg
        resources/ui/check.svg
)

# Значки панели инструментов от оформителя (если уже есть). Имена файлов — ID команд.
file(GLOB FV_TOOLBAR_ICONS CONFIGURE_DEPENDS "${FV_ICON_DIR}/*.svg" "${FV_ICON_DIR}/*.png")
if(FV_TOOLBAR_ICONS)
    list(LENGTH FV_TOOLBAR_ICONS _n)
    message(STATUS "FieldView: значков панели инструментов — ${_n} (${FV_ICON_DIR})")
    set(_aliases)
    foreach(_f ${FV_TOOLBAR_ICONS})
        get_filename_component(_name ${_f} NAME)
        set_source_files_properties(${_f} PROPERTIES QT_RESOURCE_ALIAS ${_name})
    endforeach()
    qt_add_resources(fieldview_app "fieldview_toolbar_icons"
        PREFIX "/toolbar"
        FILES ${FV_TOOLBAR_ICONS})
else()
    message(STATUS "FieldView: значков в ${FV_ICON_DIR} нет — временные рисованные")
endif()
# Значок программы (рисунок оформителя пакета, вариант A): окно — PNG, файл .exe — ICO.
set(FV_APP_ICON_DIR "${CMAKE_SOURCE_DIR}/src/resources/icons/app"
    CACHE PATH "Каталог значков программ пакета")
set(FV_APP_ICON "${FV_APP_ICON_DIR}/fieldview_256.png")
if(EXISTS "${FV_APP_ICON}")
    set_source_files_properties("${FV_APP_ICON}" PROPERTIES QT_RESOURCE_ALIAS fieldview.png)
    qt_add_resources(fieldview_app "fieldview_app_icon" PREFIX "/app" FILES "${FV_APP_ICON}")
endif()
set(FV_EXE_RC)
if(WIN32 AND EXISTS "${FV_APP_ICON_DIR}/fieldview.ico")
    configure_file("${FV_APP_ICON_DIR}/fieldview.ico" "${CMAKE_CURRENT_BINARY_DIR}/fieldview.ico" COPYONLY)
    set(_fv_ver)
    if(COMMAND tmc_win_version_rc)
        tmc_win_version_rc(_fv_ver fieldview)   # ресурс версии (Т-36), из общего CMakeLists
    endif()
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/fieldview.rc" "IDI_ICON1 ICON \"fieldview.ico\"
${_fv_ver}")
    set(FV_EXE_RC "${CMAKE_CURRENT_BINARY_DIR}/fieldview.rc")
endif()

qt_add_executable(fieldview WIN32 src/main.cpp)
if(FV_EXE_RC)
    # Ресурс .exe — отдельной объектной библиотекой без путей включения: windres из
    # MinGW не переносит пробелы в путях -I («Python Project»).
    add_library(fieldview_exe_icon OBJECT "${FV_EXE_RC}")
    set_target_properties(fieldview_exe_icon PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
    target_sources(fieldview PRIVATE $<TARGET_OBJECTS:fieldview_exe_icon>)
endif()
target_link_libraries(fieldview PRIVATE fieldview_app)
set_target_properties(fieldview PROPERTIES OUTPUT_NAME fieldview)

