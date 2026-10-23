# TMC_DN — вьювер диаграмм направленности.
#
# Программа выросла из TMCGROUT, поэтому устройство то же: код Windows-версии
# компилируется поверх слоя совместимости MFC->Qt, на Qt написано главное окно.
# Отличие — модель решётки излучателей (каталог `viewers/dianapr`) и расчёт
# диаграммы в отдельном потоке с окном прогресса.

set(WIN_APP  ${WIN_SRC}/viewers/DiaNapGr)
set(WIN_CALC ${WIN_SRC}/viewers/DiaNapr)

# --- Ядро: решётка излучателей, документ, выражения (без Qt) -------------------
add_library(tmc_dn_core STATIC
    ${WIN_CALC}/c2darray.cpp            # модель решётки, расчёт диаграммы
    ${WIN_CALC}/cizl.cpp                # один излучатель
    ${WIN_APP}/TMCGROUTDoc.cpp          # документ: файл задания
    ${WIN_APP}/TMCGrExpression.cpp      # выражения
    ${WIN_APP}/TmcSMatrix.cpp           # S-матрица (наследие TMCGROUT)
    ${WIN_APP}/TmcTtoS.cpp)

target_include_directories(tmc_dn_core PUBLIC
    ${TMC_INCLUDE}
    ${WIN_APP}
    ${WIN_CALC}
    ${CMAKE_SOURCE_DIR}/src/compat/include)

# Модули решётки (viewers/dianapr) подключают только "windows.h": в Windows этот
# заголовок давал им и работу с временем записи файла (CreateFile/GetFileTime/
# CompareFileTime). В слое совместимости эти объявления лежат в tmc_mfc_doc.h,
# поэтому его подставляем принудительно — исходники Windows-версии не трогаем.
set_source_files_properties(
    ${WIN_CALC}/c2darray.cpp
    ${WIN_CALC}/cizl.cpp
    PROPERTIES COMPILE_OPTIONS "-include;tmc_mfc_doc.h")

target_link_libraries(tmc_dn_core PUBLIC tmc_platform ${TMC_MATH_LINK})
target_compile_options(tmc_dn_core PRIVATE ${TMC_PRELUDE_FLAGS})

# --- Интерфейс ----------------------------------------------------------------
if(TMC_BUILD_GUI)
  add_library(tmc_dn_view STATIC
      ${WIN_APP}/TMCGROUTView.cpp        # рисование диаграммы, мышь, зум
      ${WIN_APP}/MainFrm.cpp             # строка состояния
      ${WIN_APP}/DialogDoc.cpp           # Characteristics + вход в девиации
      ${WIN_APP}/cDeviation.cpp          # разброс распределения
      ${WIN_APP}/DeviatDirPat.cpp        # расчёт отклонённой диаграммы
      ${WIN_APP}/ThreadCalcDirPat.cpp    # окно прогресса расчёта
      ${WIN_APP}/DialogTtoS.cpp
      ${WIN_APP}/TMCGROUTDIALOGView.cpp  # таблица кривых в диалоге характеристик
      ${WIN_APP}/TmcGroutColorGraphDialog.cpp
      ${WIN_APP}/TmcGrParColTypWid.cpp)
  target_include_directories(tmc_dn_view PUBLIC ${WIN_APP} ${WIN_CALC})
  target_link_libraries(tmc_dn_view PUBLIC tmc_dn_core tmc_gui_common)
  target_compile_options(tmc_dn_view PRIVATE ${TMC_PRELUDE_FLAGS})

  # Разметка диалогов. Файлы .ui и dialog_registry.cpp порождены из ресурсов
  # Windows-версии скриптом rc2ui.
  # uic (AUTOUIC) делает из каждого .ui заголовок ui_<имя>.h, а
  # dialog_registry.cpp отдаёт их слою совместимости.
  file(GLOB TMC_DN_DIALOG_UI CONFIGURE_DEPENDS
       ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/*.ui)

  add_library(tmc_dn_gui STATIC
      mainwindow.cpp
      dnviewwidget.cpp          # виджет-хозяин, безопасный для потока расчёта
      dialogs/dialog_registry.cpp
      ${TMC_DN_DIALOG_UI}
      resources/tmc_dn.qrc)
  target_include_directories(tmc_dn_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(tmc_dn_gui PUBLIC tmc_dn_view tmc_dn_core tmc_gui_common Qt6::Widgets)
  target_compile_options(tmc_dn_gui PRIVATE ${TMC_PRELUDE_FLAGS})

  # Сама программа.
  add_executable(tmc_dn main.cpp)
  if(WIN32)   # окно программы без консоли; /Ar и /Arb пишут в консоль родителя (AttachConsole)
    set_target_properties(tmc_dn PROPERTIES WIN32_EXECUTABLE ON)
  endif()
  tmc_win_app_icon(tmc_dn tmc_dn)
  target_link_libraries(tmc_dn PRIVATE tmc_dn_gui)
  target_compile_options(tmc_dn PRIVATE ${TMC_PRELUDE_FLAGS})

endif()

# Проверка работы окна: нажимает все команды, меняет размер, щёлкает по графику;
# при зависании печатает стек. В поставку не входит.
