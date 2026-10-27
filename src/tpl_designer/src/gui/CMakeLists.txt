# Интерфейс. Синтаксиса .tpl здесь нет ни в одной строке — его собирает tpl_core.
set(GUI_SOURCES
    main.cpp
    dtheme.cpp
    startwindow.cpp
    mainwindow.cpp
    geometryview.cpp
    propertypanel.cpp
    runplanpage.cpp
    calculatordialog.cpp
    issuedialog.cpp
    shapedialog.cpp
    portdialog.cpp
    filemaker.cpp
    signalpreview.cpp
    signalgallery.cpp
    settings.cpp
    settingsdialog.cpp
    samplespage.cpp
    memorydonut.cpp
    sysmemory.cpp
    meshview.cpp
    import3ddialog.cpp
    wheelguard.cpp
    resources.qrc
)

# --- значок приложения ---
#
# Значок ОКНА везде один и берётся из ресурса Qt (`:/icons/resources/app.ico`) — эта
# часть от системы не зависит. А вот значок САМОЙ ПРОГРАММЫ в проводнике, доке и меню
# приложений каждая система берёт по-своему, и веток здесь три. Рисунок при этом один:
# все три файла делает make_icon из одного кода отрисовки (пояснение там же).

# Значок .exe в проводнике и на панели задач берётся ТОЛЬКО из ресурса Windows:
# QIcon в коде задаёт значок ОКНА, до файла на диске она не дотягивается. Поэтому
# .rc обязателен; собирает его штатный windres из поставки MinGW.
if (WIN32)
    enable_language(RC)
    # Ресурс собираем ОТДЕЛЬНОЙ целью, а не в общем списке исходников.
    #
    # windres не переживает путей с пробелами: каталоги включения он передаёт
    # препроцессору без кавычек, и «I:/Python Project/…» разваливается на два
    # несуществующих пути. Свойством исходника это не лечится — каталоги приходят от
    # цели, — а вот у отдельной цели их просто нет. Ресурсу они и не нужны: никаких
    # #include в app.rc нет.
    add_library(app_res OBJECT app.rc)
    set_target_properties(app_res PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
    list(APPEND GUI_SOURCES $<TARGET_OBJECTS:app_res>)
endif()

# WIN32 убирает консольное окно у оконного приложения.
add_executable(TplDesigner WIN32 ${GUI_SOURCES})
# tmc_ui_base — общее с программами пакета: шрифты, окно «О программе», язык интерфейса.
target_link_libraries(TplDesigner PRIVATE tpl_core tmc_ui_base Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)
target_include_directories(TplDesigner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# OpenCASCADE на Windows — это DLL рядом с программой: windeployqt их не знает.
# Копируем ровно замыкание зависимостей (TARGET_RUNTIME_DLLS), а не всю папку bin
# OCCT: там 60 МБ, программе нужна часть.
if (WIN32 AND TPLD_HAVE_STEP)
    add_custom_command(TARGET TplDesigner POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:TplDesigner>
                $<TARGET_FILE_DIR:TplDesigner>
        COMMAND_EXPAND_LISTS)
endif()
