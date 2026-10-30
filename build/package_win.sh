#!/bin/sh
# Поставка Windows Qt6-версии: все программы в одной папке + Qt + рантайм MinGW.
#   build/package_win.sh [папка]     (по умолчанию dist/TMC_Suite_Qt6_win64)
# Берёт всё из dist/win64/bin. Примеры и руководства — из папки TMC_ROOT (по умолчанию ../TMC_Suite).
set -e
HERE="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${1:-$HERE/dist/TMC_Suite_Qt6_win64}"
BIN="$HERE/dist/win64/bin"
TMC_ROOT="${TMC_ROOT:-$HERE/../TMC_Suite}"
QT=/c/Qt/6.11.3/mingw_64
export PATH="/c/Qt/Tools/mingw1310_64/bin:$QT/bin:$PATH"

rm -rf "$OUT"
mkdir -p "$OUT"
for exe in planarrt_h planarrt_x tmcgrout tmcros tmc_dn; do
  cp "$BIN/$exe.exe" "$OUT/"
done
FV="$BIN/fieldview.exe"
[ -f "$FV" ] && cp "$FV" "$OUT/"
# TPL Designer: программа, утилита gen_lr2, библиотеки OpenCASCADE (libTK*.dll — импорт STEP;
# сборка кладёт рядом ровно нужные), примеры 3D-моделей и руководство.
TPLD="$HERE/src/tpl_designer"
cp "$BIN/TplDesigner.exe" "$BIN/gen_lr2.exe" "$OUT/"
cp "$BIN"/libTK*.dll "$OUT/"
cp -r "$TPLD/data/models" "$OUT/models"
cp "$TPLD/docs/user-guide/TPL-Designer-rukovodstvo.pdf" "$OUT/"

# Лицензия пакета — Apache 2.0 (О-1, утверждена 26.10.2026): LICENSE и NOTICE из корня репозитория.
cp "$HERE/LICENSE" "$OUT/LICENSE.txt"
cp "$HERE/NOTICE" "$OUT/NOTICE.txt"
# Тексты лицензий сторонних компонентов (Т-35; собирает tools/package/make_licenses.py).
mkdir -p "$OUT/licenses"
cp -r "$HERE/src/resources/licenses/." "$OUT/licenses/"
# Руководство пользователя пакета (собирает tools/build_manual_pdf.py).
MANUAL="$TMC_ROOT/docs/user-manual/TMC_Suite_5.0_Руководство_пользователя.pdf"
[ -f "$MANUAL" ] && cp "$MANUAL" "$OUT/"
# Английское руководство (tools/build_manual_pdf.py --lang en); программы открывают PDF
# на языке интерфейса (gui_common/aboutbox.cpp, findManual).
MANUAL_EN="$TMC_ROOT/docs/user-manual/en/TMC_Suite_5.0_User_Manual.pdf"
[ -f "$MANUAL_EN" ] && cp "$MANUAL_EN" "$OUT/"
# Примеры для первого расчёта (руководство пользователя, «Знакомство с пакетом»):
# демо-задания всех программ и многочастотный пример (только входные файлы).
SAMPLES="$TMC_ROOT/samples"
mkdir -p "$OUT/samples/MULTIFREQ"
cp -r "$SAMPLES/DEMO" "$OUT/samples/"
cp "$SAMPLES/MULTIFREQ/README.md" "$SAMPLES"/MULTIFREQ/wg_*.tpl "$SAMPLES/MULTIFREQ/wg_multi.sig"    "$SAMPLES/MULTIFREQ/f_diel_sweep.s" "$OUT/samples/MULTIFREQ/"
# Без личных путей машины сборки и без служебных файлов вьюверов (Т-35, п. 4).
python -X utf8 "$HERE/build/sanitize_samples.py" "$OUT/samples" || exit 1
cd "$OUT"
for exe in *.exe; do
  windeployqt --release --no-translations --no-system-d3d-compiler --compiler-runtime \
    --no-opengl-sw "$exe" > /dev/null
done
# Перевод стандартных окон Qt (выбор шрифта, файла, цвета) на русский.
mkdir -p translations
cp "$QT/translations/qtbase_ru.qm" translations/
# Рантайм MinGW (если windeployqt его не положил).
for dll in libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll; do
  [ -f "$dll" ] || cp "/c/Qt/Tools/mingw1310_64/bin/$dll" .
done
echo "готово: $OUT"
ls "$OUT" | head -50
