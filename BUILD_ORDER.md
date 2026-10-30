# Порядок сборки TMC Suite

Краткая карта сборки Windows-версии на Qt 6. Подробная пошаговая инструкция публикуется на сайте
проекта [www.tamic.ru](https://www.tamic.ru).

---

## Быстрый путь: одна команда

Все библиотеки и программы собираются разом, порядок зависимостей задаёт CMake:

```sh
OCCT_DIR=<папка с OpenCASCADEConfig.cmake> build/build_win.sh
```

Команду выполняют в Git Bash из корня проекта. Число параллельных заданий задаёт переменная
`TMC_JOBS` (по умолчанию 4). Сборка проходит **без ошибок и без предупреждений**.

Куда кладутся результаты:

| Что | Папка |
|---|---|
| Программы | `dist/win64/bin` |
| Библиотеки | `dist/win64/lib` |
| Промежуточные файлы | `build/obj` |
| Журнал сборки | `build/build-win.log` |

---

## Перед началом (один раз)

1. Установить **Qt 6.11.3** для MinGW 64-bit и средства из его установщика: **MinGW 13.1**, **CMake**,
   **Ninja**. Сценарий ожидает их в `C:\Qt`; при другом расположении поправьте `PATH` в начале
   `build/build_win.sh`.
2. Собрать **Open CASCADE Technology** тем же компилятором (нужен TPL Designer для импорта STEP,
   зависимость обязательная). Папку с `OpenCASCADEConfig.cmake` передают в переменной `OCCT_DIR`.
3. Установить **Git for Windows**: сценарии сборки выполняются в Git Bash.

---

## Что собирается

### Фаза 1 — библиотеки (6 штук)

| Библиотека | Папка исходников |
|---|---|
| sfile95 | `src/libs/SFILE95` |
| complex | `src/libs/complex` |
| exprint | `src/libs/Expr`, `src/libs/Inter` |
| TMCLibError | `src/libs/TMCLibError` |
| prepr | `src/libs/PREPR` |
| TMCIndan | `src/libs/TMCIndan` |

Общие заголовки — `src/Include`. Слой совместимости с MFC поверх Qt — `src/compat`, общие части
интерфейса — `src/gui_common`.

### Фаза 2 — вьюверы

| Программа | Код расчёта и файлов | Интерфейс Qt |
|---|---|---|
| `tmcros.exe` | `src/viewers/Tmcrtout` | `src/viewers/Tmcrtout/qt` |
| `tmcgrout.exe` | `src/viewers/Tmcgrout` | `src/viewers/Tmcgrout/qt` |
| `tmc_dn.exe` | `src/viewers/DiaNapGr`, `src/viewers/DiaNapr` | `src/viewers/DiaNapGr/qt` |

### Фаза 3 — счётные ядра

| Программа | Код расчёта | Интерфейс Qt |
|---|---|---|
| `planarrt_h.exe` | `src/kernels/PlanarRT_H` | `src/kernels/PlanarRT_H/qt`, `src/kernels/qt_common` |
| `planarrt_x.exe` | `src/kernels/PlanarRT_X` | `src/kernels/PlanarRT_X/qt`, `src/kernels/qt_common` |

### Фаза 4 — FieldView и TPL Designer

| Программа | Папка |
|---|---|
| `fieldview.exe` | `src/fieldview` |
| `TplDesigner.exe`, `gen_lr2.exe` | `src/tpl_designer` |

---

## Поставка и установщик

`build/package_win.sh` собирает в `dist/TMC_Suite_Qt6_win64` готовую папку: программы, библиотеки Qt и
MinGW, руководства, примеры. Руководства и примеры берутся из папки `TMC_ROOT`. Установщик собирается
из `installer/TMC_Suite_Qt6.iss` (Inno Setup).

---
---

# Build order of TMC Suite (English)

A short map of the build of the Windows Qt 6 version. The detailed step-by-step guide is on the
project website [www.tamic.ru](https://www.tamic.ru).

## Quick way: one command

All libraries and programs are built at once, CMake sets the order of dependencies:

```sh
OCCT_DIR=<folder with OpenCASCADEConfig.cmake> build/build_win.sh
```

Run the command in Git Bash from the project root. `TMC_JOBS` sets the number of parallel jobs
(4 by default). The build gives **no errors and no warnings**.

| What | Folder |
|---|---|
| Programs | `dist/win64/bin` |
| Libraries | `dist/win64/lib` |
| Intermediate files | `build/obj` |
| Build log | `build/build-win.log` |

## Before you start (once)

1. Install **Qt 6.11.3** for MinGW 64-bit and, from its installer, **MinGW 13.1**, **CMake** and
   **Ninja**. The script expects them in `C:\Qt`; for another location change the paths at the start of
   `build/build_win.sh`.
2. Build **Open CASCADE Technology** with the same compiler (TPL Designer needs it for STEP import; the
   dependency is mandatory). Pass the folder with `OpenCASCADEConfig.cmake` in `OCCT_DIR`.
3. Install **Git for Windows**: the build scripts run in Git Bash.

## What is built

| Phase | What | Where |
|---|---|---|
| 1 | six libraries: sfile95, complex, exprint, TMCLibError, prepr, TMCIndan | `src/libs`, common headers in `src/Include` |
| 2 | viewers: `tmcros.exe`, `tmcgrout.exe`, `tmc_dn.exe` | `src/viewers`, the Qt interface in `qt/` subfolders |
| 3 | kernels: `planarrt_h.exe`, `planarrt_x.exe` | `src/kernels`, the Qt interface in `qt/` and `src/kernels/qt_common` |
| 4 | `fieldview.exe`, `TplDesigner.exe`, `gen_lr2.exe` | `src/fieldview`, `src/tpl_designer` |

## Package and installer

`build/package_win.sh` makes the ready folder `dist/TMC_Suite_Qt6_win64`: programs, Qt and MinGW
libraries, manuals, examples. The installer is built from `installer/TMC_Suite_Qt6.iss` (Inno Setup).
