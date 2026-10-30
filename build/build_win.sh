#!/bin/sh
# Запуск из любой папки: build/build_win.sh. Сборка Windows Qt6-версии (MinGW 13.1 + Qt 6.11.3; Т-37 — до 25.10.2026 был 6.11.1). Промежуточные файлы — build/obj, программы — dist/win64/bin, лог — build/build-win.log.
# Одна команда собирает всё: 6 библиотек, 6 программ пакета и TPL Designer (с 27.10.2026).
# OpenCASCADE (импорт STEP в TPL Designer, обязательно): папка с OpenCASCADEConfig.cmake — OCCT_DIR.
export PATH="/c/Qt/Tools/mingw1310_64/bin:/c/Qt/6.11.3/mingw_64/bin:/c/Qt/Tools/CMake_64/bin:/c/Qt/Tools/Ninja:$PATH"
cd "$(dirname "$0")/.."
OCCT="${OCCT_DIR:-$(pwd -W)/../occt/install/cmake}"
if [ ! -f build/obj/build.ninja ]; then
  cmake -S . -B build/obj -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=C:/Qt/6.11.3/mingw_64 -DCMAKE_CXX_COMPILER=g++ \
    "-DOpenCASCADE_DIR=$OCCT" > build/build-win-config.log 2>&1 || { tail -30 build/build-win-config.log; exit 1; }
elif ! grep -q '^OpenCASCADE_DIR:' build/obj/CMakeCache.txt; then
  # Папка сборки создана до 27.10.2026 (без TPL Designer): дописать путь к OpenCASCADE.
  cmake -S . -B build/obj "-DOpenCASCADE_DIR=$OCCT" > build/build-win-config.log 2>&1 || { tail -30 build/build-win-config.log; exit 1; }
fi
cmake --build build/obj -j ${TMC_JOBS:-4} -- -k 0 "$@" > build/build-win.log 2>&1
rc=$?
echo "exit $rc"
exit $rc
