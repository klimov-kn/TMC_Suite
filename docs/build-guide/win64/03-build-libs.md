# win64 · Шаг 3. Фаза 1 — сборка библиотек (6 штук)

На этом шаге собираются все 6 библиотек (`.lib`) под 64 бита. Их нужно собрать
**до** любых программ.

> Порядок между библиотеками не важен. Перед сборкой: конфигурация **Release**,
> платформа **x64** (см. `02-setup-paths.md`).

> ⚠️ **КЛЮЧЕВОЕ — точность `_real` (двойная).** До сборки проверьте, что в общем
> заголовке `src\Include\Typerth.h` активна **двойная** точность (`#define _PREC_DOUBLE`,
> а `//#define _PREC_FLOAT`). Тип `_real` входит в общие структуры (ABI) и в формат файлов
> поля → точность обязана быть одинаковой у всех библиотек и программ. Сборка во `float`
> даёт ложную «ребристость»/неустойчивость X‑моды (`tmc_rtx`) — см. `07-troubleshooting.md`,
> § 7.9 (Баг #11). Это условие корректности, а не опция производительности.

---

## 3.0 Правки, уже внесённые в проекты библиотек

**Хорошая новость:** правок **кода** ради 64 бит на этом этапе **не потребовалось** —
все 6 библиотек компилируются под x64 чисто (только обычные легаси‑предупреждения
`C4996`, `C4267`, `C4700`, те же, что в win32). Это зафиксировано в журнале портирования
(`08-porting-changes.md`, раздел «Фаза 1»).

Изменения коснулись только **конфигурации сборки** (`.vcxproj`):

- В 5 проектов (complex, exprint, TMCLibError, Prepr, TMCIndan) добавлены конфигурации
  **`Debug|x64`** и **`Release|x64`** — зеркально win32, toolset **v145**, параметры
  `UseOfMfc` и `RuntimeLibrary` как в соответствующих win32‑конфигурациях. У SFILE95
  x64‑конфигурации уже были.
- Вывод `.lib` идёт в `dist\win64\lib` автоматически через `build\LibOutput.props`
  (`$(Platform)=x64` → `PlatformFolder=win64`).

**Общие правки кода (нужны и для win32, не связаны с разрядностью)** — уже применены:

- **С‑1** — конфликт макроса `ETIME` в `src\Include\MAINWNDW.H` (`enum ST_ITEMS`).
  Добавлено `#ifdef ETIME / #undef ETIME / #endif`. Математика не затронута.
- **С‑2** — в `src\libs\SFILE95\Sadd.cpp` `#include <error.h>` → `#include <Error1.h>`.
  Математика не затронута.

> 🔧 Никаких 64‑битных правок кода в Фазе 1 нет. Подтверждение — в
> `08-porting-changes.md`.

---

## 3.1 Последовательность для каждой библиотеки

1. **File → Open → Project/Solution…**, открыть `.sln` библиотеки из её папки.
2. Проверить: конфигурация **Release**, платформа **x64**.
3. **Build → Build Solution** (или **Rebuild Solution** для чистой сборки).
4. Дождаться `Build: 1 succeeded, 0 failed`.
5. Убедиться, что `.lib` появился в `dist\win64\lib`.

![Меню Build → Build Solution](../../screenshots/build/vs-build-solution.png)

> Скриншот общий с инструкцией win32 — меню то же. **Перед сборкой убедитесь, что вверху выбрана платформа `x64`, конфигурация `Release`.**

---

## 3.2 sfile95.lib

- Папка: `src\libs\SFILE95`, решение `Sfile95.sln`
- Результат: `dist\win64\lib\sfile95.lib`

## 3.3 complex.lib

- Папка: `src\libs\complex` → `dist\win64\lib\complex.lib`

## 3.4 exprint.lib

- Папка: `src\libs\exprint` → `dist\win64\lib\exprint.lib`
- Использует исходники из `src\libs\Expr` и `src\libs\Inter` (уже на месте).

## 3.5 TMCLibError.lib

- Папка: `src\libs\TMCLibError` → `dist\win64\lib\TMCLibError.lib`

## 3.6 prepr.lib

- Папка: `src\libs\PREPR` → `dist\win64\lib\prepr.lib`

## 3.7 TMCIndan.lib

- Папка: `src\libs\TMCIndan` → `dist\win64\lib\TMCIndan.lib`

---

## 3.8 Проверка Фазы 1

В `<КОРЕНЬ>\dist\win64\lib` должны лежать все 6 файлов:

```
sfile95.lib   complex.lib   exprint.lib
TMCLibError.lib   prepr.lib   TMCIndan.lib
```

Переходите к **`04-build-viewers.md`**.
