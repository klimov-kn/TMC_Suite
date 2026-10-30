# TMC Suite — Windows-версия на Qt 6: шесть библиотек, ядра PlanarRT_H и PlanarRT_X, вьюверы, FieldView, TPL Designer.
#
# Математика — те же исходники, что и у MFC-версии: формулы, константы и форматы файлов совпадают
# байт в байт. На Qt переписан только интерфейс.
#
# Сборка (MinGW 13.1, Qt 6.11, OpenCASCADE): build/build_win.sh
# Программы — dist/win64/bin, библиотеки — dist/win64/lib, промежуточные файлы — build/obj.

cmake_minimum_required(VERSION 3.22)

project(TMC_Suite_Qt
        VERSION 5.0.0
        DESCRIPTION "TMC Suite: Windows-версия на Qt 6"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(WIN32)
  # Windows (MinGW + Qt): gnu++17 — строгий режим прячет в заголовках MinGW
  # расширения CRT Microsoft (_complex, stricmp …), которыми пользуется код
  # Windows-версии (как ядра TMC_Suite_Mobile).
  set(CMAKE_CXX_EXTENSIONS ON)
  # Слой MFC поверх Qt включён и под Windows; Windows API — настоящий.
  add_compile_definitions(TMC_QT_WIN _MBCS)
  # Однобайтовая сборка, как Windows-версия: Qt добавляет UNICODE своим целям —
  # тогда LOGFONT, TextOut и др. стали бы «широкими». Отменяем.
  add_compile_options(-UUNICODE -U_UNICODE)
endif()

# Программы и библиотеки — в dist/win64, как у MFC-версии; промежуточные файлы — в build/obj.
if(NOT DEFINED TMC_OUT)
  set(TMC_OUT ${CMAKE_SOURCE_DIR}/dist/win64)
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TMC_OUT}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${TMC_OUT}/lib)

# --- Арифметика с плавающей точкой -------------------------------------------
# g++ по умолчанию имеет право объединять умножение со сложением в одну операцию
# (FMA). Результат при этом чуть точнее, но ОТЛИЧАЕТСЯ в последних битах от того,
# что считала Windows-версия: MSVC такого объединения не делает. Нам нужны
# совпадающие числа, а не более точные, поэтому объединение запрещаем.
# Это не изменение математики: формулы и порядок операций остаются прежними.
add_compile_options(-ffp-contract=off)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Тип сборки" FORCE)
endif()

# Qt нужен только графическим программам. Библиотеки-математика собираются и без
# него — это позволяет проверять счёт на машине, где Qt ещё не установлен.
option(TMC_BUILD_GUI "Собирать графические программы (требуется Qt 6)" ON)

if(TMC_BUILD_GUI)
  find_package(Qt6 6.4 QUIET COMPONENTS Core Gui Widgets OpenGLWidgets PrintSupport)
  if(NOT Qt6_FOUND)
    message(WARNING
      "Qt 6 не найден — графические программы собираться не будут. "
      "Укажите CMAKE_PREFIX_PATH.")
    set(TMC_BUILD_GUI OFF)
  else()
    message(STATUS "Qt ${Qt6_VERSION}")
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
  endif()
endif()

# Пиктограмма exe (вариант A) — ресурс Windows из resources/icons/app/<prog>.ico.
if(WIN32)
  enable_language(RC)
endif()
# Ресурс версии exe (Т-36): «Свойства -> Подробно» в проводнике, установщик и
# антивирусы видят программу, версию и автора. Только ASCII: windres читает .rc
# в кодовой странице системы.
set(TMC_PACKAGE_VERSION_RC "5,0,0,0")
set(TMC_PACKAGE_VERSION_STR "5.0.0.0")
function(tmc_win_version_rc out prog)
  set(desc "TMC Suite ${prog}")
  if(prog STREQUAL "planarrt_h")
    set(desc "TMC Suite PlanarRT H kernel (H-polarization)")
  elseif(prog STREQUAL "planarrt_x")
    set(desc "TMC Suite PlanarRT X kernel (X-mode, plasma)")
  elseif(prog STREQUAL "tmcros")
    set(desc "TMC Suite TMCROS output signal viewer")
  elseif(prog STREQUAL "tmcgrout")
    set(desc "TMC Suite TMCGROUT S-matrix viewer")
  elseif(prog STREQUAL "tmc_dn")
    set(desc "TMC Suite TMC_DN directional pattern viewer")
  elseif(prog STREQUAL "fieldview")
    set(desc "TMC Suite FieldView field viewer")
  endif()
  set(${out} "1 VERSIONINFO
FILEVERSION ${TMC_PACKAGE_VERSION_RC}
PRODUCTVERSION ${TMC_PACKAGE_VERSION_RC}
FILEOS 0x40004
FILETYPE 0x1
BEGIN
  BLOCK \"StringFileInfo\"
  BEGIN
    BLOCK \"040904B0\"
    BEGIN
      VALUE \"CompanyName\", \"K. N. Klimov\"
      VALUE \"FileDescription\", \"${desc}\"
      VALUE \"FileVersion\", \"${TMC_PACKAGE_VERSION_STR}\"
      VALUE \"InternalName\", \"${prog}\"
      VALUE \"OriginalFilename\", \"${prog}.exe\"
      VALUE \"ProductName\", \"TMC Suite\"
      VALUE \"ProductVersion\", \"${TMC_PACKAGE_VERSION_STR}\"
    END
  END
  BLOCK \"VarFileInfo\"
  BEGIN
    VALUE \"Translation\", 0x409, 1200
  END
END
" PARENT_SCOPE)
endfunction()

function(tmc_win_app_icon target prog)
  if(WIN32)
    set(ico ${CMAKE_SOURCE_DIR}/src/resources/icons/app/${prog}.ico)
    if(EXISTS ${ico})
      set(rc ${CMAKE_CURRENT_BINARY_DIR}/${target}_icon.rc)
      tmc_win_version_rc(ver ${prog})
      file(WRITE ${rc} "IDI_ICON1 ICON \"${ico}\"
${ver}")
      # Отдельная объектная библиотека: windres не переносит пробелов в путях
      # включения цели («Python Project»), а ресурсу пути включения не нужны.
      add_library(${target}_icon OBJECT ${rc})
      set_target_properties(${target}_icon PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
      target_sources(${target} PRIVATE $<TARGET_OBJECTS:${target}_icon>)
    endif()
  endif()
endfunction()

add_subdirectory(src)

message(STATUS "TMC Suite Qt ${PROJECT_VERSION}: тип сборки ${CMAKE_BUILD_TYPE}, "
               "GUI=${TMC_BUILD_GUI}")
