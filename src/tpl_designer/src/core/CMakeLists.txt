# Ядро генератора. Зависит только от Qt6::Core — тестируется без запуска GUI.
# Qt6::Widgets сюда добавлять НЕЛЬЗЯ: это архитектурная граница проекта.
add_library(tpl_core STATIC
    lang.cpp       # язык текстов: русский / английский (tpl::tr)
    cp1251.cpp
    project.cpp
    emitter.cpp
    placement.cpp
    expr.cpp
    epsfile.cpp
    sigfile.cpp
    ramestimate.cpp
    tplreader.cpp
    taskfiles.cpp
    # Импорт геометрии из 3D-модели (см. docs/import3d-concept.md).
    mesh3d.cpp
    meshio.cpp
    meshslice.cpp
    convexsplit.cpp
    materialmap.cpp
    meshimport.cpp
    meshgen.cpp
    # Библиотека образцов: рецепты заданий по параметрам (см. docs/samples-concept.md).
    samples/catalog.cpp
    samples/standards.cpp
    samples/rf.cpp
    samples/kit.cpp
    samples/waveguides.cpp
    samples/antennas.cpp
    samples/microstrip.cpp
    samples/microstrip2.cpp
    samples/media.cpp
    samples/lessons.cpp
    samples/baked.cpp
    samples/usersamples.cpp
    samples/lessons.qrc
)

target_include_directories(tpl_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                           ${CMAKE_CURRENT_SOURCE_DIR}/samples)
target_link_libraries(tpl_core PUBLIC Qt6::Core)

# --- STEP через OpenCASCADE ---
#
# Обязательная зависимость: ищем пакет OpenCASCADE (OpenCASCADE_DIR — папка с
# OpenCASCADEConfig.cmake); на Windows это своя сборка под MinGW.
#
# Модули STEP в OCCT 7.8 сведены в один TKDESTEP; в 7.6 (Ubuntu 24.04) их пять.
# Код stepio_occt.cpp написан под API, общий для обеих версий.
#
# Сам пакет ищется в корневом CMakeLists.txt: его переменные нужны и утилитам (tools).
if (TPLD_WITH_STEP AND OpenCASCADE_FOUND)
    message(STATUS "STEP: OpenCASCADE ${OpenCASCADE_VERSION} — ${OpenCASCADE_DIR}")
    set(OCCT_STEP_LIBS TKernel TKMath TKG2d TKG3d TKGeomBase TKBRep TKTopAlgo TKMesh
                       TKXSBase TKLCAF TKCAF TKXCAF)
    if (OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.8)
        list(APPEND OCCT_STEP_LIBS TKDE TKDESTEP)
    else()
        list(APPEND OCCT_STEP_LIBS TKSTEP TKSTEPBase TKSTEPAttr TKSTEP209 TKXDESTEP)
    endif()
    set(TPLD_OCCT_STEP_LIBS ${OCCT_STEP_LIBS} CACHE INTERNAL "")
    target_sources(tpl_core PRIVATE stepio_occt.cpp)
    target_include_directories(tpl_core PRIVATE ${OpenCASCADE_INCLUDE_DIR})
    target_link_libraries(tpl_core PUBLIC ${OCCT_STEP_LIBS})
    # Папка с DLL OpenCASCADE — для запуска тестов и для раскладки рядом с .exe.
    set(TPLD_OCCT_BIN_DIR "${OpenCASCADE_BINARY_DIR}" CACHE INTERNAL "")
    set(TPLD_HAVE_STEP ON CACHE INTERNAL "")
endif()
