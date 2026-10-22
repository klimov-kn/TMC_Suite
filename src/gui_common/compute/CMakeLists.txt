# Модуль «Compute»: настройка расчёта на видеокарте.
#   * tmc_compute       — статическая библиотека (Qt6 Core + Widgets + Concurrent);
#   * tmc_compute_demo  — тестовая программа (окно; --report — отчёт в консоль).
#
# Можно собирать отдельно:
#   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/Qt/6.11.1/mingw_64
# или подключить из общего проекта: add_subdirectory(src/gui_common/compute)
# (тогда демо не собирается, если не включить TMC_COMPUTE_DEMO).

cmake_minimum_required(VERSION 3.21)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(tmc_compute LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
    endif()
    set(_tmc_compute_standalone ON)
else()
    set(_tmc_compute_standalone OFF)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Concurrent)

# Заголовки OpenCL берём из общей копии исходников Windows-версии
# (src/Include: tmcocl.h и каталог cl/). В путь поиска весь src/Include
# не добавляем — там много заголовков с общими именами (debug.h, file.h, matrix.h…).
# Нужные четыре файла копируются в каталог сборки при настройке CMake
# (configure_file сам перенастроит сборку, если исходные файлы изменятся).
set(TMC_COMPUTE_OCL_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/../../Include"
    CACHE PATH "Directory with tmcocl.h and cl/cl.h")
set(_ocl_inc "${CMAKE_CURRENT_BINARY_DIR}/ocl_include")
if(WIN32)
    foreach(_f TmcOcl.h CL/cl.h CL/cl_platform.h CL/cl_version.h)
        if(NOT EXISTS "${TMC_COMPUTE_OCL_INCLUDE}/${_f}")
            message(FATAL_ERROR "tmc_compute: ${TMC_COMPUTE_OCL_INCLUDE}/${_f} not found (set TMC_COMPUTE_OCL_INCLUDE)")
        endif()
        configure_file("${TMC_COMPUTE_OCL_INCLUDE}/${_f}" "${_ocl_inc}/${_f}" COPYONLY)
    endforeach()
endif()

add_library(tmc_compute STATIC
    tmccompute.h
    tmccompute.cpp
    computedialog.h
    computedialog.cpp)
set_target_properties(tmc_compute PROPERTIES AUTOMOC ON)
target_include_directories(tmc_compute PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
if(WIN32)
    target_include_directories(tmc_compute PRIVATE "${_ocl_inc}")
endif()
target_link_libraries(tmc_compute PUBLIC Qt6::Core Qt6::Widgets Qt6::Concurrent tmc_i18n)

# Побитная сверка карты с процессором: эталон на процессоре считается без FMA.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(tmc_compute PRIVATE -ffp-contract=off)
elseif(MSVC)
    target_compile_options(tmc_compute PRIVATE /fp:precise)
endif()

