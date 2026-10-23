# TMCROS — вьювер S-матриц.
#
# Два слоя:
#   tmcros_core — модули Windows-версии: документ (чтение и запись файла задания),
#                 S-матрица, выражения. Ни строчки Qt: формат файлов и расчёт
#                 должны остаться теми же, что на Windows.
#   tmcros      — интерфейс на Qt6: окно, меню, панель, MDI, отрисовка графиков.

set(WIN_APP ${WIN_SRC}/viewers/Tmcrtout)

# --- Ядро (без Qt) ------------------------------------------------------------
add_library(tmcros_core STATIC
    ${WIN_APP}/TMCGROUTDoc.cpp          # документ: файл задания, список графиков
    ${WIN_APP}/TmcSMatrix.cpp           # S-матрица
    ${WIN_APP}/TmcTtoS.cpp              # преобразование «время -> S-параметры»
    ${WIN_APP}/TmcWideband.cpp          # многочастотный расчёт (снимок 22.10)
    ${WIN_APP}/TMCGrExpression.cpp)     # выражения над графиками

target_include_directories(tmcros_core PUBLIC
    ${TMC_INCLUDE}
    ${WIN_APP}
    ${CMAKE_SOURCE_DIR}/src/compat/include)

target_link_libraries(tmcros_core PUBLIC tmc_platform ${TMC_MATH_LINK})
target_compile_options(tmcros_core PRIVATE ${TMC_PRELUDE_FLAGS})

# --- Интерфейс на Qt ----------------------------------------------------------
if(TMC_BUILD_GUI)
  # Вид перенесён из Windows-версии: рисование и обработка мыши компилируются
  # как есть, поверх слоя совместимости (src/compat) и Qt.
  add_library(tmcros_view STATIC
      ${WIN_APP}/TMCGROUTView.cpp
      ${WIN_APP}/MainFrm.cpp            # рамка окна: нужна строка состояния
      # Диалоги Windows-версии: поведение берём как есть, разметку — из .rc.
      ${WIN_APP}/DialogDoc.cpp
      ${WIN_APP}/DialogTtoS.cpp
      ${WIN_APP}/TmcGroutColorGraphDialog.cpp
      ${WIN_APP}/TMCGROUTDIALOGView.cpp
      ${WIN_APP}/TmcGrParColTypWid.cpp)
  # tmcgrparcoltypwid1.cpp в Windows-проект (TMCROS.vcxproj) не входит — там
  # осталась старая копия диалога, ссылающаяся на несуществующий ресурс.
  target_include_directories(tmcros_view PUBLIC ${WIN_APP})
  target_link_libraries(tmcros_view PUBLIC tmcros_core tmc_gui_common)
  target_compile_options(tmcros_view PRIVATE ${TMC_PRELUDE_FLAGS})

  # Разметка диалогов. Файлы .ui и dialog_registry.cpp порождены из ресурсов
  # Windows-версии скриптом rc2ui.
  # uic (AUTOUIC) делает из каждого .ui заголовок ui_<имя>.h, а
  # dialog_registry.cpp отдаёт их слою совместимости.
  file(GLOB TMCROS_DIALOG_UI CONFIGURE_DEPENDS
       ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/*.ui)

  add_library(tmcros_gui STATIC
      mainwindow.cpp
      dialogs/dialog_registry.cpp
      ${TMCROS_DIALOG_UI}
      resources/tmcros.qrc)
  target_include_directories(tmcros_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(tmcros_gui PUBLIC tmcros_view tmcros_core tmc_gui_common Qt6::Widgets)
  target_compile_options(tmcros_gui PRIVATE ${TMC_PRELUDE_FLAGS})

  # Сама программа.
  add_executable(tmcros main.cpp)
  if(WIN32)   # окно программы без консоли; /Ar и /Arb пишут в консоль родителя (AttachConsole)
    set_target_properties(tmcros PROPERTIES WIN32_EXECUTABLE ON)
  endif()
  tmc_win_app_icon(tmcros tmcros)
  target_link_libraries(tmcros PRIVATE tmcros_gui)
  target_compile_options(tmcros PRIVATE ${TMC_PRELUDE_FLAGS})

endif()
