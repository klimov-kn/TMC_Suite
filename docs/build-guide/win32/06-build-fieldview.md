# win32 · Шаг 6. Фаза 4 — сборка FieldView (OpenGL)

На этом шаге собирается **FieldView.exe** — визуализатор полей — под 32 бита.
Это MFC‑приложение (статическая MFC, MultiByte), использующее **OpenGL**. Оно
линкуется с библиотеками OpenGL (`opengl32`, `glu32`) и теми же TMC‑библиотеками, что
и счётные ядра: **sfile95**, **prepr**, **exprint**, **TMCIndan**, **TMCLibError**.

> Перед сборкой: все 6 библиотек Фазы 1 готовы; конфигурация **Release**, платформа
> **Win32**.

---

## 6.1 Подготовка OpenGL (ничего скачивать не нужно)

Всё необходимое для OpenGL входит в **Windows SDK**, который вы поставили вместе с
Visual Studio (см. `01-requirements.md`, раздел 1.2). Отдельно ставить GLEW, GLFW или
что‑либо ещё **не требуется**.

Что используется:

- **Заголовки** `GL.h`, `GLU.h` — подтягиваются из Windows SDK автоматически; вручную
  прописывать пути к ним не нужно.
- **Библиотеки линковки** `opengl32.lib` и `glu32.lib`. Для платформы `Win32` они
  берутся из 32‑битной части SDK (`um\x86`). Visual Studio находит их сама.

Проверить, что библиотеки OpenGL указаны в проекте:

1. Правой кнопкой по проекту FieldView → **Properties**.
2. **Linker → Input → Additional Dependencies**.
3. В списке должны быть, помимо TMC‑библиотек:

   ```
   opengl32.lib
   glu32.lib
   ```

![Linker → Input для FieldView (opengl32.lib, glu32.lib + TMC‑библиотеки)](../../screenshots/build/vs-fieldview-linker-input.png)

> ⚠️ **GLAUX отсутствует — и это нормально.** Старая вспомогательная библиотека GLAUX
> (`glaux.h` / `glaux.lib`) удалена из современного Windows SDK. В коде FieldView
> ссылка на неё убрана (правка **Ф4‑1**, см. 6.2). В списке линковки `glaux.lib`
> быть **не должно**.

---

## 6.2 Что уже сделано в проекте FieldView

**Правки настроек сборки (`.vcxproj`/`.sln`), не код:**

- Старый `FldView.vcxproj` (формат VS, только Win32, без toolset) переписан по образцу
  ядер: toolset **v145**, 4 конфигурации (Win32/x64 × Debug/Release), подключён
  `build\ExeOutput.props`.
- Удалены жёсткий путь вывода `C:\TMC\EXE\FldView.exe` и опция линкера `/MACHINE:I386`.
- Имя exe задаётся через `<TargetName>FieldView</TargetName>` (в оригинале было
  `FldView.exe`; приведено к ожидаемому имени `FieldView.exe`).
- Из линкера убран **`glaux.lib`** (см. ниже Ф4‑1); оставлены `opengl32.lib`,
  `glu32.lib`.
- Ссылки `..\INCLUDE\...` исправлены на `..\Include\...`.

**Правка в коде (общая, нужна и для win32) — Ф4‑1:**

- В `src\fieldview\TmcGLText.h` (строка 12) удалён мёртвый `#include <gl\glaux.h>`
  (GLAUX отсутствует в современном SDK → ошибка `C1083`). Ни одна `aux*`‑функция в
  коде не используется: текст рисуется через GDI (`CreateFontIndirect`, `GetDIBits`)
  и ядро OpenGL (`glBitmap`, `glRasterPos3f`). Из линкера убран `glaux.lib`.
  Эта правка нужна и для win32, и для win64; математика не затронута.

> Примечание: файлы `TmcRTH_BlockList1.cpp/.h` (с жёстким путём `z:\work\...`) в проект
> НЕ включены и не компилируются.

---

## 6.3 Сборка FieldView (Win32)

1. Открыть проект FieldView — файл **`FldView.vcxproj`** из папки `src\fieldview`.
   (`FldView.sln` ссылается на новый `.vcxproj` — открыть можно и решение, но надёжнее
   открывать сам `.vcxproj`.)
2. Конфигурация **Release**, платформа **Win32**.
3. Проверить линковку (раздел 6.1): `opengl32.lib`, `glu32.lib`, `sfile95.lib`,
   `prepr.lib`, `exprint.lib`, `TMCIndan.lib`, `TMCLibError.lib`.
4. **Build → Clean Solution**, затем **Build → Build Solution**.
5. Результат: `dist\win32\bin\FieldView.exe`.

> Предупреждения при сборке (`C4996` небезопасные CRT, `C4477` `sprintf("%s", CString)`)
> — легаси‑шум, на работу и на расчёты не влияют.

---

## 6.4 Итоговая проверка всей 32‑битной сборки

После Фазы 4 в папках должно быть:

```
dist\win32\lib\   → 6 файлов .lib
                    sfile95, complex, exprint, TMCLibError, prepr, TMCIndan

dist\win32\bin\   → 6 файлов .exe
                    TMCROS, TMCGROUT, TMC_DN, tmc_rth, tmc_rtx, FieldView
```

Запустите **FieldView.exe** — окно должно открыться и рисовать поля (OpenGL работает).
Прогоните типовые тесты из `samples\SAMPLE_R` на ядрах.

32‑битная сборка завершена. При проблемах смотрите **`07-troubleshooting.md`**.
