# PlanarRT_X — графическая оболочка счётного ядра X.
#
# Состав файлов тот же, что у H, но собирается из СВОЕГО каталога снимка и с
# шестью макросами X-моды. Макросы задаются ТОЛЬКО здесь: в общих заголовках
# они сломали бы H-моду и вьюверы.
#
# Интерфейс общий с PlanarRT_H (../../qt_common) и не дублируется: подписи,
# которыми программы отличаются, собраны в app_info.cpp.
# Устройство слоёв описано в ../../PlanarRT_H/qt/CMakeLists.txt.

set(WIN_APP ${WIN_SRC}/kernels/PlanarRT_X)
set(PLANRT_COMMON ${CMAKE_CURRENT_SOURCE_DIR}/../../qt_common)

set(PLANRT_PRELUDE -include ${PLANRT_COMMON}/planrt_prelude.h)

# Шесть макросов X-моды — из PlanRT_X.vcxproj, только на целях этой программы.
set(PLANRT_X_DEFS
    ELECTRON_Q___ ELECTRON_M___
    CTMCRTH_INDANBLCK_FILEY     CTMCRTH_INDANBLCK_RECTSTATY
    CTMCRTH_INDANBLCK_CIRCSTATY CTMCRTH_INDANBLCK_POLYGSTTY)

# --- S-матрица (общая с вьюверами) --------------------------------------------
add_library(planarrt_x_ttos STATIC
    ${WIN_SRC}/viewers/Tmcrtout/TmcSMatrix.cpp
    ${WIN_SRC}/viewers/Tmcrtout/TmcTtoS.cpp
    ${WIN_SRC}/viewers/Tmcrtout/TmcWideband.cpp)   # многочастотный расчёт (22.10)
target_include_directories(planarrt_x_ttos PUBLIC
    ${TMC_INCLUDE} ${WIN_SRC}/viewers/Tmcrtout ${CMAKE_SOURCE_DIR}/src/compat/include)
target_link_libraries(planarrt_x_ttos PUBLIC tmc_compat)
target_compile_options(planarrt_x_ttos PRIVATE ${TMC_PRELUDE_FLAGS})

# --- Ядро: всё, что перенесено из Windows-версии ------------------------------
add_library(planarrt_x_core STATIC
    ${WIN_APP}/TmcRTHNodeDiel.cpp
    ${WIN_APP}/TmcRTHRectNode.cpp
    ${WIN_APP}/Pl_iofor.cpp
    ${WIN_APP}/Pl_glfun.cpp
    ${WIN_APP}/PlanRT_HDoc.cpp
    ${WIN_APP}/PlanRT_HView.cpp
    ${WIN_APP}/TmcDialogStatistics.cpp
    ${WIN_APP}/TmcDialogCalcTime.cpp    # диалог отчёта о времени (/Time)
    ${WIN_APP}/TmcRTH_DialogBlock.cpp
    ${WIN_APP}/TmcRTH_DialogFormatOutFile.cpp
    ${WIN_APP}/TmcSoundMel1.cpp
    ${WIN_APP}/TmcSoundEffProp.cpp
    ${PLANRT_COMMON}/planrt_compat.cpp
    ${PLANRT_COMMON}/mainframe.cpp)

target_include_directories(planarrt_x_core PUBLIC
    ${TMC_INCLUDE}
    ${WIN_APP}
    ${PLANRT_COMMON}
    ${WIN_SRC}/viewers/Tmcrtout
    ${CMAKE_SOURCE_DIR}/src/compat/include)

# I-25: ключи -D не передаются: в Qt6-версии все шесть макросов уже определены в typerth.h и tmcgrviw.h
# (значения заголовков и так главнее: -DELECTRON_Q___ давал 1, заголовок переопределял). Список выше
# оставлен как справка, какие макросы нужны X-моде. Было:
# target_compile_definitions(planarrt_x_core PUBLIC ${PLANRT_X_DEFS})
target_link_libraries(planarrt_x_core PUBLIC tmc_platform
    -Wl,--start-group ${TMC_MATH_LIBS} planarrt_x_ttos -Wl,--end-group tmc_compat)
target_compile_options(planarrt_x_core PRIVATE ${PLANRT_PRELUDE})

# --- Интерфейс на Qt ----------------------------------------------------------
if(TMC_BUILD_GUI)
  target_link_libraries(planarrt_x_core PUBLIC tmc_gui_common)

  # Разметка диалогов (rc2ui, ресурс X-версии):
  file(GLOB PLANARRT_X_DIALOG_UI CONFIGURE_DEPENDS
       ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/*.ui)

  add_library(planarrt_x_gui STATIC
      ${PLANRT_COMMON}/mainwindow.cpp
      app_info.cpp
      dialogs/dialog_registry.cpp
      ${PLANARRT_X_DIALOG_UI}
      resources/planarrt_x.qrc)
  target_include_directories(planarrt_x_gui PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR} ${PLANRT_COMMON})
  target_link_libraries(planarrt_x_gui PUBLIC
      planarrt_x_core tmc_gui_common tmc_compute Qt6::Widgets)
  target_compile_options(planarrt_x_gui PRIVATE ${PLANRT_PRELUDE})

  add_executable(planarrt_x ${PLANRT_COMMON}/main.cpp)
  if(WIN32)   # окно программы без консоли; /Ar и /Arb пишут в консоль родителя (AttachConsole)
    set_target_properties(planarrt_x PROPERTIES WIN32_EXECUTABLE ON)
  endif()
  tmc_win_app_icon(planarrt_x planarrt_x)
  target_link_libraries(planarrt_x PRIVATE planarrt_x_gui)
  target_compile_options(planarrt_x PRIVATE ${PLANRT_PRELUDE})

endif()
