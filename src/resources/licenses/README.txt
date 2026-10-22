TMC Suite 5.0 (Qt6, Windows x64) — сторонние компоненты и их лицензии
======================================================================

Собственная лицензия TMC Suite — LICENSE_ru.txt в папке программы.
Ниже — сторонние компоненты, которые входят в поставку, и файлы с текстами их лицензий
(в этой папке).

  Компонент                         Файлы поставки                     Лицензия / файл
  --------------------------------  ---------------------------------  --------------------------------------
  Qt 6.11.3 (The Qt Company)        Qt6*.dll, плагины в папках         LGPL-3.0 (и GPL-3.0):
                                    platforms, styles, imageformats,     Qt-GPL-3.0-LGPL-3.0.txt
                                    iconengines, generic, tls,
                                    networkinformation, translations
  Компоненты внутри модулей Qt      (внутри Qt6Core/Gui/Network/Svg и   Qt-third-party.txt
  (zlib, libpng, libjpeg-turbo,      плагинов)                          (тексты из документации Qt; версии — sbom/)
  PCRE2, FreeType, HarfBuzz,
  double-conversion, MD4C и др.)
  Open CASCADE Technology 7.9.3     libTK*.dll (использует              LGPL-2.1 с исключением:
                                    TPL Designer для импорта 3D)          OpenCASCADE-LGPL-2.1.txt,
                                                                          OpenCASCADE-LGPL-exception.txt
  Рантайм GCC / MinGW-w64 13.1      libgcc_s_seh-1.dll, libstdc++-6.dll GPL-3.0 с GCC Runtime Library
                                                                          Exception: GCC-GPL-3.0.txt,
                                                                          GCC-Runtime-Library-Exception.txt;
                                                                          MinGW-w64-runtime.txt
  winpthreads (MinGW-w64)           libwinpthread-1.dll                 winpthreads.txt
  Шрифт Inter                       (встроен в программы)               SIL OFL 1.1: Inter-OFL.txt
  Шрифт JetBrains Mono              (встроен в программы)               SIL OFL 1.1: JetBrainsMono-OFL.txt

Qt и Open CASCADE подключены динамически (отдельными DLL) и не изменены: их можно
заменить своей сборкой той же версии, положив DLL на место прежних.

Исходные тексты библиотек под LGPL
----------------------------------
Используются официальные исходные тексты без изменений:
  * Qt 6.11.3        — https://download.qt.io/official_releases/qt/6.11/6.11.3/single/
  * Open CASCADE 7.9.3 — https://dev.opencascade.org/release
  * GCC / MinGW-w64  — https://gcc.gnu.org, https://www.mingw-w64.org
Версии библиотек в поставке совпадают с указанными выше.

Third-party components (English)
--------------------------------
The table above lists every third-party component shipped with TMC Suite 5.0 (Qt6) and the
file in this folder that contains its license. Qt (LGPL-3.0) and Open CASCADE (LGPL-2.1 with
exception) are unmodified and dynamically linked; the corresponding sources of the shipped
versions are available at the links above.
