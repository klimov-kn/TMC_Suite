# PlanarRT_H — графическая оболочка счётного ядра H.
#
# Счёт идёт в отдельном потоке (слой mfc_thread.h поверх QThread), прогресс —
# в строке состояния, как в Windows-версии.
#
# Слои:
#   planarrt_h_ttos — S-матрица и преобразование «время -> S» (наследие TMCROS).
#                     Отдельная библиотека потому, что к ней обращается TMCIndan:
#                     зависимость взаимная, и обе кладутся в одну группу
#                     компоновки (однопроходный GNU ld иначе цикл не разрешает).
#   planarrt_h_core — модули Windows-версии: расчётные узлы, документ, вид,
#                     диалоги, форматы вывода, глобальные функции. Ни строчки Qt:
#                     формат файлов и расчёт остаются теми же, что на Windows.
#   planarrt_h_gui  — главное окно на Qt (общее с PlanarRT_X).
#
# PlanarRT_X — та же программа, собранная из своего каталога снимка и с шестью
# макросами X-моды. Всё, что у них общее, лежит в ../../qt_common и НЕ
# дублируется: разница только в исходниках Windows-версии, макросах и подписях.

set(WIN_APP ${WIN_SRC}/kernels/PlanarRT_H)
set(PLANRT_COMMON ${CMAKE_CURRENT_SOURCE_DIR}/../../qt_common)

# Преамбула для файлов MFC-версии: общая (tmc_win_prelude.h) плюс добавка для
# оболочек — страницы свойств, атрибуты файла, версия системы и прочее, чего нет
# в общем слое совместимости. Общая преамбула
# подключается изнутри добавки: два одинаковых ключа -include CMake схлопывает.
set(PLANRT_PRELUDE -include ${PLANRT_COMMON}/planrt_prelude.h)

# --- S-матрица (общая с вьюверами) --------------------------------------------
add_library(planarrt_h_ttos STATIC
    ${WIN_SRC}/viewers/Tmcrtout/TmcSMatrix.cpp
    ${WIN_SRC}/viewers/Tmcrtout/TmcTtoS.cpp
    ${WIN_SRC}/viewers/Tmcrtout/TmcWideband.cpp)   # многочастотный расчёт (22.10)
target_include_directories(planarrt_h_ttos PUBLIC
    ${TMC_INCLUDE} ${WIN_SRC}/viewers/Tmcrtout ${CMAKE_SOURCE_DIR}/src/compat/include)
target_link_libraries(planarrt_h_ttos PUBLIC tmc_compat)
target_compile_options(planarrt_h_ttos PRIVATE ${TMC_PRELUDE_FLAGS})

# --- Ядро: всё, что перенесено из Windows-версии ------------------------------
add_library(planarrt_h_core STATIC
    ${WIN_APP}/TmcRTHNodeDiel.cpp       # узел: диэлектрик
    ${WIN_APP}/TmcRTHRectNode.cpp       # узел: прямоугольная сетка
    ${WIN_APP}/Pl_iofor.cpp             # форматы вывода чисел, флаги режима
    ${WIN_APP}/PL_GLFUN.CPP             # строка состояния, функции потоков, звук
    ${WIN_APP}/PlanRT_HDoc.cpp          # документ: задание .tpl, шаг, поток счёта
    ${WIN_APP}/PlanRT_HView.cpp         # вид: рисование и команды меню
    ${WIN_APP}/TmcDialogStatistics.cpp
    ${WIN_APP}/TmcDialogCalcTime.cpp    # диалог отчёта о времени (/Time)  # диалог Statistics
    ${WIN_APP}/TmcRTH_DialogBlock.cpp   # диалог Block
    ${WIN_APP}/TmcRTH_DialogFormatOutFile.cpp  # формат выходного файла
    ${WIN_APP}/TmcSoundMel1.cpp         # страница мелодии оповещения
    ${WIN_APP}/TmcSoundEffProp.cpp      # набор страниц «Sound Effects»
    ${PLANRT_COMMON}/planrt_compat.cpp  # недостающее в слое совместимости
    ${PLANRT_COMMON}/mainframe.cpp)     # рамка окна: строка состояния для MFC-версии

target_include_directories(planarrt_h_core PUBLIC
    ${TMC_INCLUDE}
    ${WIN_APP}
    ${PLANRT_COMMON}
    ${WIN_SRC}/viewers/Tmcrtout
    ${CMAKE_SOURCE_DIR}/src/compat/include)

# Библиотеки-математика и planarrt_h_ttos связаны взаимно, поэтому идут одной
# группой — тем же приёмом, что и в src/CMakeLists.txt.
target_link_libraries(planarrt_h_core PUBLIC tmc_platform
    -Wl,--start-group ${TMC_MATH_LIBS} planarrt_h_ttos -Wl,--end-group tmc_compat)
target_compile_options(planarrt_h_core PRIVATE ${PLANRT_PRELUDE})

# --- Интерфейс на Qt ----------------------------------------------------------
if(TMC_BUILD_GUI)
  # Реализация классов MFC поверх Qt (окна, виды, диалоги, потоки) лежит в общем
  # слое интерфейса — без неё код Windows-версии не скомпонуется.
  target_link_libraries(planarrt_h_core PUBLIC tmc_gui_common)

  # Разметка диалогов. Файлы .ui и dialog_registry.cpp порождены из ресурсов
  # Windows-версии скриптом rc2ui.
  file(GLOB PLANARRT_H_DIALOG_UI CONFIGURE_DEPENDS
       ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/*.ui)

  add_library(planarrt_h_gui STATIC
      ${PLANRT_COMMON}/mainwindow.cpp
      app_info.cpp
      dialogs/dialog_registry.cpp
      ${PLANARRT_H_DIALOG_UI}
      resources/planarrt_h.qrc)
  target_include_directories(planarrt_h_gui PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR} ${PLANRT_COMMON})
  target_link_libraries(planarrt_h_gui PUBLIC
      planarrt_h_core tmc_gui_common tmc_compute Qt6::Widgets)
  target_compile_options(planarrt_h_gui PRIVATE ${PLANRT_PRELUDE})

  # Сама программа.
  add_executable(planarrt_h ${PLANRT_COMMON}/main.cpp)
  if(WIN32)   # окно программы без консоли; /Ar и /Arb пишут в консоль родителя (AttachConsole)
    set_target_properties(planarrt_h PROPERTIES WIN32_EXECUTABLE ON)
  endif()
  tmc_win_app_icon(planarrt_h planarrt_h)
  target_link_libraries(planarrt_h PRIVATE planarrt_h_gui)
  target_compile_options(planarrt_h PRIVATE ${PLANRT_PRELUDE})

endif()
