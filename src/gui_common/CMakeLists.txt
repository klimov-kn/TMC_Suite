# Общий слой интерфейса для всех программ TMC Suite:
#   * tmc_mfcqt — реализация классов MFC (рисование, окна, виды) поверх Qt.
#     Благодаря ей виды программ компилируются из Windows-версии без переписывания.
#   * theme     — оформление в палитре TPL Designer.
#   * виджет-хозяин, который отдаёт виду события мыши, клавиш и таймера.

# Язык интерфейса (русский / английский): словарь и перевод окон при показе.
# Отдельная библиотека — её берёт и FieldView (у него своя тема).
add_library(tmc_i18n STATIC i18n/tmci18n.cpp)
target_include_directories(tmc_i18n PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tmc_i18n PUBLIC Qt6::Widgets)
qt_add_resources(tmc_i18n "tmci18n" PREFIX "/tmci18n" BASE ${CMAKE_SOURCE_DIR}/src/resources/i18n
    FILES ${CMAKE_SOURCE_DIR}/src/resources/i18n/ui_strings.tsv)

# Оформление без слоя MFC: тема, шрифты Inter/JetBrains Mono, значки программ, единое окно
# «О программе». Отдельная библиотека — её берёт и TPL Designer (27.10.2026), которому
# расчётный код и классы MFC не нужны.
find_package(Qt6 REQUIRED COMPONENTS Svg)
add_library(tmc_ui_base STATIC theme.cpp aboutbox.cpp)
target_include_directories(tmc_ui_base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tmc_ui_base PUBLIC Qt6::Widgets Qt6::Svg tmc_i18n)

add_library(tmc_gui_common STATIC
    ${CMAKE_SOURCE_DIR}/src/compat/mfc_gdi.cpp
    ${CMAKE_SOURCE_DIR}/src/compat/mfc_view.cpp
    ${CMAKE_SOURCE_DIR}/src/compat/mfc_dialog.cpp
    ${CMAKE_SOURCE_DIR}/src/compat/mfc_thread.cpp
    colorpicker.cpp      # окно выбора цвета (новое)
    mfcviewwidget.cpp
    printsupport.cpp
    recentfiles.cpp
    commandupdate.cpp    # состояние команд (ON_UPDATE_COMMAND_UI)
    windowplacement.cpp  # положение окна между запусками
    mousewatchdog.cpp)   # защита от «залипшего» захвата мыши

target_include_directories(tmc_gui_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/compat/include
    ${TMC_INCLUDE})

# PrintSupport — печать вида тем же кодом, что рисует на экране.
target_link_libraries(tmc_gui_common PUBLIC Qt6::Widgets Qt6::PrintSupport tmc_platform tmc_i18n tmc_ui_base)

# Новый вид (Windows Qt6-версия): пиктограммы программ (вариант A) и тулбаров
# (вариант Б, с индикаторами), шрифты Inter и JetBrains Mono — общие ресурсы.
set(TMC_RES ${CMAKE_SOURCE_DIR}/src/resources)
file(GLOB_RECURSE TMC_ICON_FILES CONFIGURE_DEPENDS ${TMC_RES}/icons/app/*.svg ${TMC_RES}/icons/toolbar/*.svg ${TMC_RES}/icons/links/*.svg)
qt_add_resources(tmc_ui_base "tmcicons" PREFIX "/tmcicons" BASE ${TMC_RES}/icons FILES ${TMC_ICON_FILES})
file(GLOB TMC_FONT_FILES ${TMC_RES}/fonts/*.ttf)
qt_add_resources(tmc_ui_base "tmcfonts" PREFIX "/tmcfonts" BASE ${TMC_RES}/fonts FILES ${TMC_FONT_FILES})

