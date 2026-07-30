# Порядок сборки TMC Suite

Краткая карта сборки: что и в каком порядке компилировать. Каждая фаза выполняется
дважды — для **win32** и для **win64**.

Подробная пошаговая инструкция со скриншотами, точными названиями пунктов меню и
разбором ошибок — в [`docs/build-guide/`](docs/build-guide/):
[win32](docs/build-guide/win32/) · [win64](docs/build-guide/win64/).

---

## Стратегия: сначала win32, потом win64

Исторически пакет собирался под win16 и win32, поэтому 32-битная сборка ближе к
исходному состоянию кода. Рекомендуемая последовательность:

1. Пройти все четыре фазы под **win32** и получить шесть работающих программ.
2. Только после этого переключиться на **x64** и пройти фазы заново.
3. Ошибки 64-битной компиляции разбирать по одной. Перечень уже внесённых правок
   портирования — в [`docs/build-guide/win64/08-porting-changes.md`](docs/build-guide/win64/08-porting-changes.md).

Собирать win64 параллельно с неготовым win32 не стоит: непонятно, ошибка вызвана
разрядностью или незавершённой настройкой.

---

## Перед началом (один раз)

1. Установить **Visual Studio Community 2026 (v18.6)** с рабочей нагрузкой
   *Desktop development with C++* и компонентом **MFC**. Набор инструментов
   компилятора — **v145**. Полный список — в
   [`01-requirements.md`](docs/build-guide/win32/01-requirements.md).
2. Убедиться, что ко всем проектам подключены property sheets из [`build/`](build/):
   `Common.props`, `LibOutput.props`, `ExeOutput.props`. Они задают пути к заголовкам,
   библиотекам и папкам вывода, поэтому жёстких путей в проектах нет.
3. Создать папки результатов:
   ```
   dist\win32\lib   dist\win32\bin
   dist\win64\lib   dist\win64\bin
   ```

**Переключение платформы** — выпадающий список вверху окна Visual Studio, рядом с
кнопкой запуска: `Win32` → результат в `dist\win32\`, `x64` → в `dist\win64\`.

---

## Фаза 1 — Библиотеки (6 штук)

Порядок между библиотеками не важен: каждая `.lib` компилируется независимо. Важно
собрать все шесть до перехода к программам.

| № | Библиотека | Папка исходников | Результат |
|---|---|---|---|
| 1.1 | sfile95 | `src\libs\SFILE95` (решение `Sfile95.sln`) | `dist\<платформа>\lib\sfile95.lib` |
| 1.2 | complex | `src\libs\complex` | `dist\<платформа>\lib\complex.lib` |
| 1.3 | exprint | `src\libs\exprint` | `dist\<платформа>\lib\exprint.lib` |
| 1.4 | TMCLibError | `src\libs\TMCLibError` | `dist\<платформа>\lib\TMCLibError.lib` |
| 1.5 | prepr | `src\libs\PREPR` | `dist\<платформа>\lib\prepr.lib` |
| 1.6 | TMCIndan | `src\libs\TMCIndan` | `dist\<платформа>\lib\TMCIndan.lib` |

✅ **Проверка:** в `dist\win32\lib` (и `dist\win64\lib`) лежат все шесть `.lib`.

---

## Фаза 2 — Вьюверы (3 программы)

Все три собираются одинаково и линкуются с **sfile95 + complex + exprint**.

| № | Программа | Папка исходников | Результат |
|---|---|---|---|
| 2.1 | Вьювер временных сигналов | `src\viewers\Tmcrtout` | `dist\<платформа>\bin\TMCROS.exe` |
| 2.2 | Вьювер матриц рассеяния | `src\viewers\Tmcgrout` | `dist\<платформа>\bin\TMCGROUT.exe` |
| 2.3 | Вьювер диаграмм направленностей | `src\viewers\DiaNapGr` | `dist\<платформа>\bin\TMC_DN.exe` |

✅ **Проверка:** в `dist\win32\bin` (и `dist\win64\bin`) лежат три вьювера.

---

## Фаза 3 — Счётные ядра (2 программы)

Оба ядра линкуются с пятью библиотеками: **sfile95, prepr, exprint, TMCIndan, TMCLibError**
(Компоновщик → Ввод → Дополнительные зависимости).

### 3.1 `tmc_rth.exe` — H-поляризация

- Папка: `src\kernels\PlanarRT_H`
- Результат: `dist\<платформа>\bin\tmc_rth.exe`
- Действие: **Clean** → **Build**

### 3.2 `tmc_rtx.exe` — X-мода

⚠️ **Сначала задать макросы, иначе проект не соберётся.**

Свойства проекта → **C/C++ → Preprocessor → Preprocessor Definitions**, добавить:

```
ELECTRON_Q___;ELECTRON_M___;CTMCRTH_INDANBLCK_FILEY;CTMCRTH_INDANBLCK_RECTSTATY;CTMCRTH_INDANBLCK_CIRCSTATY;CTMCRTH_INDANBLCK_POLYGSTTY;%(PreprocessorDefinitions)
```

Макросы задаются **только в свойствах этого проекта** и не добавляются в общие
заголовки: `Typerth.h` и `Tmcgrviw.h` используются также H-модой и вьюверами.

- Папка: `src\kernels\PlanarRT_X`
- Результат: `dist\<платформа>\bin\tmc_rtx.exe`
- Действие: **Clean** → **Build**

✅ **Проверка:** оба ядра собрались и запускаются.

---

## Фаза 4 — FieldView

Визуализатор полей, требует **OpenGL** (используются системные библиотеки Windows —
`opengl32.lib`, `glu32.lib`; отдельная установка GLEW/GLFW не нужна). Подробности —
в [`06-build-fieldview.md`](docs/build-guide/win32/06-build-fieldview.md).

- Папка: `src\fieldview`
- Результат: `dist\win32\bin\FieldView.exe` и `dist\win64\bin\FieldView.exe`

✅ **Проверка:** обе версии запускаются и рисуют поле.

---

## Итог

После всех фаз:

```
dist\win32\lib\   6 файлов .lib
dist\win32\bin\   6 файлов .exe  (TMCROS, TMCGROUT, TMC_DN, tmc_rth, tmc_rtx, FieldView)
dist\win64\lib\   6 файлов .lib
dist\win64\bin\   6 файлов .exe
```

Если сборка падает — раздел «Решение проблем»:
[win32](docs/build-guide/win32/07-troubleshooting.md) ·
[win64](docs/build-guide/win64/07-troubleshooting.md).
Перечень исправленных дефектов — в [`CHANGELOG.md`](CHANGELOG.md).

---

## Альтернатива: одно общее решение

Все 12 проектов можно собрать разом через общее решение `src/tamic.slnx` — оно
задаёт правильный порядок зависимостей автоматически. См.
[`09-solution-all.md`](docs/build-guide/09-solution-all.md).
