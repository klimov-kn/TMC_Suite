# Программа TMC_DN — API-документация

> Пакет **TMC Suite**. Вьювер **диаграмм направленности** (ДН) антенных решёток: расчёт и построение диаграммы направленности по описанию решётки излучателей, а также расчёт КНД/КИП, отклонений ДН и преобразование «время → S-матрица».
> Язык документации: русский. Все сигнатуры приведены по исходникам из `src/viewers/DiaNapGr/`, ядру расчёта ДН из `src/viewers/DiaNapr/` (`c2DArray`, `cIzl`) и общим заголовкам из `src/Include/`.

---

## 1. Назначение программы

**TMC_DN** (исходники — папка `src/viewers/DiaNapGr/`, выходной EXE — `TMC_DN.exe`) — графический просмотрщик и калькулятор **диаграмм направленности** антенных решёток.

Программа построена на той же кодовой базе MFC Document/View (MDI), что и TMCGROUT (классы `CTMCGROUTApp`, `CMainFrame`, `CChildFrame`, `CTMCGROUTDoc`, `CTMCGROUTView` имеют те же имена), но **расширена для работы с диаграммами направленности**:

- читает текстовые файлы `.dat` с описанием антенной решётки (частота, число излучателей, координаты/амплитуды/фазы каждого излучателя, выражение ДН одного элемента);
- по этому описанию **рассчитывает диаграмму направленности** решётки (когерентное суммирование полей излучателей по углу) — класс `c2DArray`;
- вычисляет интегральные характеристики: **КНД** (коэффициент направленного действия, `Knd`, в разах и дБ) и **КИП** (коэффициент использования поверхности, `Kip`);
- строит 2D-график ДН (как и TMCGROUT строит частотные характеристики), показывает КНД/КИП в строке состояния;
- считает ДН в **фоновом потоке** с прогресс-окном (классы `CThreadCalcDirPat`, функция `ReadDocFileW_Thread1`) — расчёт большой решётки длительный;
- умеет вносить **случайные отклонения** (девиации) в параметры излучателей (`cDeviation`, `c2DArray::MakeDeviation`) и считать ДН отклонённой решётки относительно требуемой ДН (`CDeviatDirPat`, `c2DArray::OptimizationInit`/`SaveDeviationDirectionalPattern`);
- содержит утилиту преобразования **временно́го сигнала в S-матрицу** (`CDialogTtoS`, `CTmcTtoS`, `CTmcSMatrix`) — общий с TMCROS код.

Всё, что относится к **рисованию, осям, цветам, масштабированию, формату документа** — устроено так же, как в TMCGROUT (см. [`tmcgrout.md`](tmcgrout.md), разделы 3, 7–9, 11–12). Ниже подробно документируются именно **отличия и дополнения TMC_DN**, а общая часть приводится со ссылками.

---

## 2. Состав проекта

В сборку `TMC_DN.vcxproj` входят файлы из `src/viewers/DiaNapGr/` плюс ядро расчёта ДН из `src/viewers/DiaNapr/`:

| Файл | Класс / содержимое | Назначение (кратко) |
|------|--------------------|---------------------|
| `TMCGROUT.cpp/.h` | `CTMCGROUTApp` | Класс приложения, `InitInstance`; ключ реестра — `"TMC_DN"` |
| `MainFrm.cpp/.h` | `CMainFrame` | Главное MDI-окно (тулбар, статусбар) |
| `ChildFrm.cpp/.h` | `CChildFrame` | Дочернее MDI-окно документа |
| `TMCGROUTDoc.cpp/.h` | `CTMCGROUTDoc` | Документ: чтение/запись, загрузка данных; **+ массив ДН `pcDirectionalPattern`** |
| `TMCGROUTView.cpp/.h` | `CTMCGROUTView` | Вид: отрисовка, мышь, меню; **+ фоновый расчёт ДН** |
| `TMCGROUTDIALOGView.cpp/.h` | `CTMCGROUTDIALOGView` | Диалог параметров графика |
| `DialogDoc.cpp/.h` | `CDialogDoc` | Диалог-таблица документа; **+ кнопки девиаций (DN)** |
| `TmcGroutColorGraphDialog.cpp/.h` | `CTmcGroutColorGraphDialog` | Цвета/тип/толщина 16 линий |
| `TmcGrParColTypWid.cpp/.h` | `CTmcGrParColTypWid` | Параметры одной линии |
| `TmcGrParColTypWid1.cpp/.h` | `CTmcGrParColTypWid1` | Пустой диалог-заготовка (см. §10) |
| `TMCGrExpression.cpp` | `CTMCGrExpression` | Графики-выражения (заголовок общий) |
| **`cDeviation.cpp/.h`** | **`cDeviation`** | **Диалог внесения случайных отклонений в решётку** |
| **`DeviatDirPat.cpp/.h`** | **`CDeviatDirPat`** | **Диалог расчёта ДН отклонённой решётки vs. требуемой** |
| **`ThreadCalcDirPat.cpp/.h`** | **`CThreadCalcDirPat`** | **Прогресс-окно фонового расчёта ДН** |
| `DialogTtoS.cpp/.h` | `CDialogTtoS` | Диалог «время → S-матрица» (общий с TMCROS) |
| `TmcTtoS.cpp` | `CTmcTtoS` | Логика преобразования T→S (заголовок общий) |
| `TmcSMatrix.cpp` | `CTmcSMatrix` | Сборка S-матрицы из временно́го сигнала (заголовок общий) |
| **`..\DiaNapr\c2DArray.cpp/.h`** | **`c2DArray`** | **Ядро: модель антенной решётки и расчёт ДН** |
| **`..\DiaNapr\cIzl.cpp/.h`** | **`cIzl`** | **Один излучатель решётки** |
| `StdAfx.cpp/.h` | — | Прекомпилированный заголовок MFC |
| `resource.h`, `TMCGROUT.rc` | — | Ресурсы |

> Жирным выделены файлы и классы, **специфичные для TMC_DN** (отсутствуют в TMCGROUT). Класс `c2DArray` подключается через относительный путь `..\DiaNapr\c2DArray.h`.

**Зависимости:** MFC; общие заголовки `src/Include/` (`Tmcgrviw.h`, `Proc_s.h`, `complex1.h`, `typedef.h`, `expr.h`, `TmcTtoS.h`, `TmcSMatrix.h`, `TMCGrExpression.h`); библиотеки пакета: **SFILE95**, **complex**, **exprint** (`i1nte_atof_1`, `expr_set_angle`, `expr_get_error` из `expr.h` — вычисление выражения ДН элемента).

---

## 3. Архитектура расчёта ДН (отличие от TMCGROUT)

В TMCGROUT график `i` берётся из S-файла. В TMC_DN добавлен **параллельный массив объектов ДН**:

```cpp
// поле CTMCGROUTDoc
c2DArray * pcDirectionalPattern;   // массив [grdoc.nGraph] — по объекту ДН на каждый график
int        nGraphCurrent;          // индекс ДН, которая считается прямо сейчас
```

Поток данных:

```
.dat (описание решётки)
      │  c2DArray::ReadData → cIzl[]            (читает излучатели)
      ▼
c2DArray::CalculateDiagrNapr                    (считает pdRealDN[], pdImageDN[] по углам)
      │  + NormirovkaDiagrNapr (КНД/КИП, нормировка)
      ▼
CTMCGROUTView (рисует как обычный график; масштаб амплитуды × GetdMax)
```

Расчёт запускается в **отдельном потоке** `ReadDocFileW_Thread1` (через `AfxBeginThread`), а модальное окно `CThreadCalcDirPat` показывает прогресс по полю `c2DArray::iN` (текущий угол) и `nGraphCurrent` (текущая ДН из общего числа).

---

## 4. Класс `c2DArray` — модель антенной решётки и расчёт ДН

**Назначение:** хранит описание антенной решётки (набор излучателей `cIzl`), рассчитывает её диаграмму направленности по углу, считает КНД/КИП, читает/пишет файлы `.dat`, поддерживает девиации и сравнение с требуемой ДН.
**Заголовок:** `src/viewers/DiaNapr/c2DArray.h`
**Реализация:** `src/viewers/DiaNapr/c2DArray.cpp`
**Зависит от:** `cIzl.h`, `windows.h`, `typedef.h`, `expr.h` (exprint — `i1nte_atof_1`), стандартная математика.

### Физическая модель (как работает расчёт)

Поле решётки в направлении `dAngle` (плоская задача, угол в радианах) — когерентная сумма по всем излучателям:

```
Re = Σ aᵢ·cos( φᵢ + (2π·f·cᵢ/c₀)·cos(dAngle − αᵢ) )
Im = Σ aᵢ·sin( φᵢ + (2π·f·cᵢ/c₀)·cos(dAngle − αᵢ) )
```

где для излучателя `i`: `aᵢ` — амплитуда (с учётом собственной ДН элемента и углового диапазона), `φᵢ` — фаза, `(xᵢ,yᵢ)` — координаты, `cᵢ=√(xᵢ²+yᵢ²)` — расстояние от начала координат, `αᵢ=atan2(yᵢ,xᵢ)`, `f` — частота (Гц), `c₀ = 299792458` м/с — скорость света. Амплитуда ДН в точке: `|поле| = √(Re²+Im²)`, фаза: `atan2(Im,Re)`.

> ⚠️ Это **научный код расчёта**. Формулы НЕ изменять без разрешения автора.

### Поля (private, основные)

| Поле | Тип | Назначение |
|------|-----|-----------|
| `pcIzl` | `cIzl*` | Массив излучателей решётки, длина `nIzl` |
| `nIzl` | `int` | Число излучателей |
| `dFreq` | `double` | Рабочая частота, Гц (в файле задаётся в ГГц) |
| `szOneExpression` | `char[1000]` | Выражение ДН одного элемента (текст, вычисляется exprint; `"1."` — изотропный) |
| `nAngle` | `int` | Число угловых точек ДН (по умолчанию 400; 1000 для больших решёток) |
| `pdAngle` | `double*` | Массив углов (рад), длина `nAngle` |
| `pdRealDN` | `double*` | Амплитуда ДН по углам (после нормировки — в [0..1]) |
| `pdImageDN` | `double*` | Фаза ДН по углам (рад) |
| `pdRequiredDN` | `double*` | Требуемая ДН (для режима сравнения/девиаций) |
| `dKnd` | `double` | КНД (раз) |
| `dKip` | `double` | КИП (раз) |
| `dMax` | `double` | Максимум амплитуды ДН (масштабный множитель) |
| `iN` | `int` (public) | Текущий обрабатываемый угол — индикатор прогресса для потока |
| `bIsReduceElMagnAnalysis` | `BOOL` (public) | Учитывать угловой диапазон `FiMin..FiMax` каждого элемента |
| `bIsError` | `BOOL` | Флаг ошибки |
| `szError` | `char[1000]` | Текст последней ошибки |
| `nMaxTemporary` | `int` | Порог «большой решётки» (400): сверх него ДН кэшируется во временный файл |
| `nPointForNormir` | `int` | Число точек для интегрирования при нормировке/КНД (по умолчанию 1000) |

### Конструктор / деструктор

#### `c2DArray()`
Инициализирует поля (нет излучателей, `nAngle=400`, изотропный элемент `"1."`), выделяет угловые массивы (`InitAngle`), задаёт режим углов exprint в радианах (`expr_set_angle(EXPR_RADIAN)`).

#### `~c2DArray()`
Удаляет временный и выходной файлы (если создавались), освобождает все массивы (`DeleteAngle`, `DeleteData`).

### Публичные методы — расчёт и доступ к ДН

#### `void CalculateDiagrNapr(char *szInputFileName, char *szOutputFileName)`
**Что делает:** полный цикл «из файла в файл»: читает решётку (`ReadData`), считает ДН (`CalculateDiagrNapr()`), записывает результат (`WriteData`).

| Параметр | Тип | Назначение |
|----------|-----|-----------|
| `szInputFileName` | `char*` | Путь к `.dat`-файлу с описанием решётки |
| `szOutputFileName` | `char*` | Путь к выходному файлу ДН |

#### `void CalculateDiagrNapr(void)`
**Что делает:** рассчитывает ДН для уже загруженной решётки.
**Как работает:** для каждого угла `pdAngle[iN]` вычисляет `Re`/`Im` (`CalculateRealDN`/`CalculateImageDN`), при изотропном элементе (`szOneExpression=="1."`) — напрямую, иначе домножает на значение выражения элемента (через `i1nte_atof_1`); пишет амплитуду в `pdRealDN`, фазу в `pdImageDN`; затем нормирует (`NormirovkaDiagrNapr`). Для больших решёток, если есть актуальный кэш, читает его (`ReadDataFromTempFile`). Поле `iN` обновляется по ходу — его читает прогресс-окно.
**Ошибки:** при ошибке вычисления выражения ставит `bIsError=TRUE`, текст из `expr_get_error()`.

#### `int GetnAngle(void)` — число угловых точек ДН.
#### `double GetdAngle(int i)` — угол `i`-й точки (рад); `0.0` при выходе за границы/ошибке.
#### `double GetdAmplitudeDiagrNapr(int i)` — амплитуда ДН в `i`-й точке; `0.0` при ошибке.
#### `double GetdMax(void)` — максимум амплитуды ДН (масштабный множитель `dMax`).
#### `double GetKnd(void)` / `double GetKnd_dB(void)` — КНД в разах / в дБ (`10·lg(Knd)`).
#### `double GetKip(void)` / `double GetKip_dB(void)` — КИП в разах / в дБ.
#### `int GetnIzl(void)` — число излучателей.
#### `double GetdFreqGHz(void)` — частота в ГГц.
#### `BOOL IsCalculateKip(void)` — TRUE, если решётка «большая» (`nIzl > nMaxTemporary`) — тогда вместо КИП считается «излучённая мощность».
#### `BOOL IsError(void)` / `char* GetError(void)` — флаг и текст ошибки.

#### `void ResizeAngle(double dAngleMin, double dAngleMax)`
Перестраивает сетку углов на диапазон `[dAngleMin, dAngleMax]` (рад), пересчитывает ДН и перезаписывает выходной файл.

### Публичные методы — ввод/вывод и редактирование решётки

#### `void ReadData(char *szFileName1)`
Загружает решётку из `.dat`. Формат: строка `Frequence = … Ghz;`, строка `N = …;` (число излучателей), строка-выражение ДН элемента, далее по строке на излучатель: `i   X = … mm; Y = … mm; Amplitude = …; Faza = … degree; [FiMin = … degree; FiMax = … degree; выражение]`. Координаты переводятся в метры, фазы и углы — в радианы.

#### `void SaveData(char *pszFile)`
Сохраняет только описание решётки (без рассчитанной ДН) в файл `pszFile`.

#### `void SaveDeviationDirectionalPattern(char *pszFileName)`
Считает ДН и пишет файл из трёх колонок: угол (град) · требуемая ДН · рассчитанная ДН — для сравнения отклонённой решётки с эталоном.

#### `void GetIzl(int i, double *dXmm, double *dYmm, double *dAmpl, double *dFazaDegree)`
Возвращает параметры излучателя `i`: координаты (мм), амплитуду, фазу (град).

#### `void SetIzl(int i, double dXcoord_mm, double dYcoord_mm, double dAmplitud, double dFaza_gradus)`
Задаёт параметры излучателя `i`.

#### `double GetRealIzl(int i)` / `double GetImageIzl(int i)`
Re/Im комплексного возбуждения излучателя `i`: `a·cos(φ)` / `a·sin(φ)`.

#### `void SetdFrequence(double dFrequenceGHz)` — задаёт частоту (вход в ГГц, хранится в Гц).
#### `void SetOneExpression(char *ch)` / `void SetszOneExpression(char *ch)` / `char* GetOneExpression(void)`
Задать/получить текст выражения ДН элемента. (`SetOneExpression` дополнительно проверяет длину; `SetszOneExpression` — без проверки.)
#### `char* GetOneExpression(void)` — текущее выражение элемента.
#### `void SetNPointForNormirovka(int nPoint)` — число точек интегрирования при нормировке.

### Публичные методы — девиации и оптимизация

#### `void MakeDeviation(char *szSourceFile, char *szDistinationFile, double dXmm, double dYmm, double dAmplProcent, double dFazaDegree)`
**Что делает:** читает решётку из `szSourceFile`, вносит **случайные** отклонения в каждый излучатель (координаты ±`dXmm`/`dYmm` мм, амплитуда ±`dAmplProcent` %, фаза ±`dFazaDegree` град — равномерное распределение через `GetRnd`), сохраняет результат в `szDistinationFile`.

#### `void OptimizationInit(char *szFileRequiDirPat, int nIzl1)`
Читает файл **требуемой ДН** (две колонки: угол в градусах, амплитуда) в `pdRequiredDN`, выделяет решётку на `nIzl1` излучателей. Используется в `CDeviatDirPat`.

#### `double GetOptimPurposeFunction(void)`
Целевая функция оптимизации: пересчитывает ДН и возвращает сумму модулей отклонений `Σ|pdRealDN[i] − pdRequiredDN[i]|`.

### Ключевые приватные методы

| Метод | Назначение |
|-------|-----------|
| `double CalculateRealDN(double dAngle)` / `CalculateImageDN(double dAngle)` | Re/Im поля решётки в направлении `dAngle` (формулы — см. «Физическая модель») |
| `double CalculateRealDN_ForKip(double)` / `CalculateImageDN_ForKip(double)` | То же, но с единичными амплитудами и нулевой фазой (для расчёта КИП) |
| `void NormirovkaDiagrNapr(void)` | Нормировка ДН на максимум; интегрирование для КНД и КИП |
| `void InitAngle(void)` / `DeleteAngle(void)` | Выделить/освободить угловые массивы; равномерная сетка `0..π` (или `−π..π` для большой решётки) |
| `void InitData(void)` | Выделить массив `pcIzl[nIzl]`, обнулить ДН |
| `void ReadData(void)` / `WriteData(void)` | Внутренние чтение/запись (работают по `szFileName`/`szFileNameOut`) |
| `double GetRnd(double dMedium, double dDelta)` | Равномерное случайное число в `[dMedium−dDelta, dMedium+dDelta]` |
| `BOOL IsDirectPatSaveInFile(void)` / `WriteTemporaryFile(void)` / `ReadDataFromTempFile(void)` | Кэш ДН во временном файле для больших решёток (сравнение времён записи) |
| `FILETIME LastWriteTime(char *szFileName)` | Время последней записи файла |

---

## 5. Класс `cIzl` — один излучатель решётки

**Назначение:** хранит параметры одного излучателя антенной решётки и вычисляет его собственную (элементную) диаграмму направленности.
**Заголовок:** `src/viewers/DiaNapr/cIzl.h`
**Реализация:** `src/viewers/DiaNapr/cIzl.cpp`
**Зависит от:** `typedef.h`, `expr.h` (exprint).

### Поля (private)

| Поле | Тип | Назначение |
|------|-----|-----------|
| `dAmplituda` | `double` | Амплитуда возбуждения |
| `dFaza` | `double` | Фаза возбуждения (рад) |
| `dXcoordinata`, `dYcoordinata` | `double` | Координаты излучателя (м) |
| `dFimin`, `dFimax` | `double` | Угловой диапазон видимости элемента (рад) |
| `csEmitterDirectPat` | `CString` | Текст выражения собственной ДН элемента |
| `nTypeEmitterDirectPat` | `int` | Тип ДН элемента: ALL(0)/SIN(1)/COS(2)/ARB(−1) |

### Константы типа ДН элемента (`cIzl.h`)

| Константа | Значение | Смысл |
|-----------|----------|-------|
| `TMC_IZL_DIRPATTYPE_ALL` | 0 | Изотропный (`"1."`) |
| `TMC_IZL_DIRPATTYPE_SIN` | 1 | `sin(f)` |
| `TMC_IZL_DIRPATTYPE_COS` | 2 | `cos(f)` |
| `TMC_IZL_DIRPATTYPE_ARB` | −1 | Произвольное выражение (через exprint) |

### Методы

#### `double GetdAmplituda(double dAngle)`
**Что делает:** амплитуда элемента в направлении `dAngle` с учётом собственной ДН: `dGetDirectionalPatternEmitter(dAngle) · dAmplituda`. Если амплитуда ≈ 0 — возвращает 0.

#### `double dGetDirectionalPatternEmitter(double dAngle)`
**Что делает:** значение собственной ДН элемента в направлении `dAngle`.
**Как работает:** если угол вне `[Fimin,Fimax]` — `FLT_MIN`; для ALL→1, SIN→`sin(dAngle)`, COS→`cos(dAngle)`; для произвольного — подставляет угол `f` и вычисляет выражение через `i1nte_atof_1`.

#### `BOOL bIsAngleInDiapazon(double dAngle)`
TRUE, если угол попадает в диапазон видимости `[Fimin,Fimax]` (с приведением углов к двум формам через `dAnglePrived`/`dAnglePrived1`).

#### Сеттеры/геттеры
`SetdAmplituda/GetdAmplituda`, `SetdFaza/GetdFaza`, `SetdXcoordinata/GetdXcoordinata`, `SetdYcoordinata/GetdYcoordinata`, `SetdFimin/GetdFimin`, `SetdFimax/GetdFimax`, `SetcsEmitterDirectPat/GetcsEmitterDirectPat` (две перегрузки сеттера — для `char*` и `CString&`; при установке распознаётся тип ДН элемента).

#### `cIzl& operator=(cIzl& cIzl1)`
Полное копирование всех полей излучателя.

#### Приватные `void dAnglePrived(double& dAngle)` / `dAnglePrived1(double& dAngle)`
Приведение угла к диапазону `(−π,π]` и `(0,2π]` соответственно (через `atan2(sin,cos)`).

---

## 6. Дополнения в `CTMCGROUTDoc` (отличия от TMCGROUT)

Документ TMC_DN — это `CTMCGROUTDoc` (заголовок `TMCGROUTDoc.h`), как в TMCGROUT (см. [`tmcgrout.md`](tmcgrout.md), раздел 7), но с добавлениями для ДН:

### Дополнительные поля

| Поле | Тип | Назначение |
|------|-----|-----------|
| `pcDirectionalPattern` | `c2DArray*` | Массив объектов ДН (по одному на график документа) |
| `nGraphCurrent` | `int` | Индекс ДН, рассчитываемой в данный момент (для прогресс-окна) |

### Дополнительные структуры (в `TMCGROUTDoc.h`)

Объявлены DN-варианты структур графика/документа (с суффиксом `_DN`): `TMC_GR_DOC1_DN` и `TMC_GR_DOC_DN`. Они повторяют `TMC_GR_DOC1`/`TMC_GR_DOC` (см. [`tmcgrout.md`](tmcgrout.md), раздел 3), добавляя поле `bIsReduceElMagnAnalysis` (учитывать угловой диапазон элементов). Поле `grdoc` документа имеет тип `TMC_GR_DOC_DN`.

> ⚠️ Назначение и фактическое использование разделения на `_DN`-варианты vs. базовые структуры требует уточнения у автора (см. §11).

Остальное (чтение/запись `.soc`, параметры графиков и оформления, цвета, шрифт) — идентично TMCGROUT.

---

## 7. Дополнения в `CTMCGROUTView` (фоновый расчёт ДН)

Вид TMC_DN — `CTMCGROUTView` (заголовок `TMCGROUTView.h`), общая отрисовка/мышь/меню — как в TMCGROUT (см. [`tmcgrout.md`](tmcgrout.md), раздел 8). Отличия:

### Дополнительное поле
| Поле | Тип | Назначение |
|------|-----|-----------|
| `ltime_start` | `time_t` | Момент старта расчёта ДН (для отсчёта времени в прогресс-окне) |

### `void ReadDocFileW_Thread(void)`
**Что делает:** перечитывает документ и пересоздаёт буферы точек **из фонового потока**; в конце выставляет `bIsDataCalculate=TRUE`, пересчитывает и перерисовывает график. Запускается функцией-обёрткой потока:

```cpp
UINT ReadDocFileW_Thread1( LPVOID pParam ); // (CTMCGROUTView*)pParam -> ReadDocFileW_Thread()
```

### `void OnEditTosmatrix()`
Команда меню — открывает диалог `CDialogTtoS` («время → S-матрица»).

### Отрисовка амплитуды ДН
В `OnDrawGraph*`/`PrepareDoubleGraph` амплитуда точки масштабируется на максимум ДН: `pSmatr[j].x · pcDirectionalPattern[i].GetdMax()`. В строке состояния (`PutStatistics1`) выводятся КНД и КИП (или «Emitted Power» для больших решёток).

### `void OnDestroy()`
Перед закрытием **дожидается завершения** фонового расчёта: пока `bIsDataCalculate==FALSE`, повторно показывает прогресс-окно `CThreadCalcDirPat`.

---

## 8. Класс `CThreadCalcDirPat` — прогресс-окно расчёта ДН

**Назначение:** модальное окно с двумя прогресс-барами, показывающее ход фонового расчёта диаграмм направленности.
**Заголовок:** `ThreadCalcDirPat.h` · **Базовый класс:** `CDialog` · **Ресурс:** `IDD_READDATA_THREAD`.

### Поля (DDX)
| Поле | Тип | Назначение |
|------|-----|-----------|
| `pCView` | `void*` | Указатель на `CTMCGROUTView` (источник прогресса) |
| `m_Progress` | `CProgressCtrl` | Прогресс текущей ДН |
| `m_ProgressAll` | `CProgressCtrl` | Прогресс по всем ДН документа |
| `m_csStatistics`, `m_csStatisticsAll` | `CString` | Процент готовности (текущая / общая) |
| `m_CurrentDirPat`, `m_NAllDirPat` | `int` | Номер текущей ДН / общее число ДН |
| `m_nEmitters` | `int` | Число излучателей текущей ДН |
| `m_csElapsTime` | `CString` | Прошедшее время (сек / мм:сс / чч:мм:сс) |

### Методы
#### `BOOL OnInitDialog()`
Запускает таймер обновления (100 мс).

#### `void OnTimer(UINT_PTR nIDEvent)`
По таймеру читает прогресс из `c2DArray::iN` текущей ДН и `nGraphCurrent` документа, обновляет проценты, прогресс-бары, число излучателей и таймер. Когда последняя ДН досчитана (или ошибка) — закрывает диалог (`EndDialog(0)`).

#### `INT_PTR DoModal()`
Запоминает время старта (`ltime_start`) и показывает диалог.

---

## 9. Диалоги девиаций (специфичны для TMC_DN)

### 9.1. Класс `cDeviation` — внесение случайных отклонений
**Заголовок:** `cDeviation.h` · **Базовый класс:** `CDialog` · **Ресурс:** `IDD_DIALOGMAKEDEVIAT`.
**Назначение:** взять файл-источник решётки и создать файл с **случайно отклонёнными** параметрами излучателей.

**Поля (DDX):** `m_csFileFrom` (исходный `.dat`), `m_csFileTo` (выходной `.dat`), `m_dXdev`/`m_dYdev` (разброс координат, мм), `m_dAmpDev` (разброс амплитуды, %), `m_dFazaDev` (разброс фазы, град).
**Методы:**
- `OnButtonsource()` — выбор исходного файла;
- `OnButtondistination()` — выбор выходного файла и вызов `MakeDeviation()`;
- `OnOK()` — выполнить девиацию и закрыть;
- `BOOL MakeDeviation(void)` (private) — вызывает `c2DArray::MakeDeviation(...)`; при ошибке показывает сообщение.

### 9.2. Класс `CDeviatDirPat` — ДН отклонённой решётки vs. требуемой
**Заголовок:** `DeviatDirPat.h` · **Базовый класс:** `CDialog` · **Ресурс:** `IDD_DIALOGCALCDEVIAT`.
**Назначение:** рассчитать ДН для распределения амплитуд/фаз и сохранить её рядом с требуемой ДН (файл сравнения `.dev`).

**Поля (DDX):** `m_csDistrAmplFaza` (файл `.dat` распределения амплитуд/фаз), `m_csRequireDirPat` (файл `.opt` требуемой ДН), `m_csFileDevDirPat` (выходной `.dev`).
**Методы:**
- `OnButtondistramplfaza()` / `OnButtonrequrdirectpattern()` / `OnButtondeviatdirpattern()` — выбор файлов (последняя кнопка запускает расчёт);
- `OnOK()` — выполнить расчёт и закрыть;
- `BOOL CalculateDeviationDirectionalPattern(void)` (private) — читает распределение (`c2DArray::ReadData`), инициализирует требуемую ДН (`OptimizationInit`), переносит излучатели (`GetIzl`/`SetIzl`), сохраняет сравнение (`SaveDeviationDirectionalPattern`).

### 9.3. Дополнения в `CDialogDoc`
Диалог-таблица документа `CDialogDoc` (см. [`tmcgrout.md`](tmcgrout.md), §9.2) в TMC_DN получает **две дополнительные кнопки**:
- `OnAddMakedeviation()` — открывает диалог `cDeviation`;
- `OnAddMakedeviation2()` — открывает диалог `CDeviatDirPat`.

Также `CDialogDoc` содержит поле `dTUnit` (`double`) и подключает `cDeviation.h`.

---

## 10. Утилита «время → S-матрица» и прочие классы

### 10.1. `CDialogTtoS`, `CTmcTtoS`, `CTmcSMatrix`
Эта подсистема **общая с TMCROS** и подробно описана в [`tmcros.md`](tmcros.md). Кратко:

- **`CTmcSMatrix`** (`TmcSMatrix.h`, `TmcSMatrix.cpp`) — читает файл временно́го сигнала `.t`, строит из него элементы S-матрицы (`ReadTFile`, `MakeSmatrix`, `Save`).
- **`CTmcTtoS`** (`TmcTtoS.h`, `TmcTtoS.cpp`) — высокоуровневая обёртка: `TtoS(csTfn, csSfn, dTmin, dTmax, dFreq)` выполняет полный цикл «файл сигнала → S-файл» через `CTmcSMatrix`.
- **`CDialogTtoS`** (`DialogTtoS.h`, `DialogTtoS.cpp`, ресурс `IDD_DIALOG3`) — диалог настройки преобразования (имя S-файла, окно времени `Tmin..Tmax`, частота, до 20 точек X), вызывается из `CTMCGROUTView::OnEditTosmatrix`.

### 10.2. `CTmcGrParColTypWid1`
**Заголовок:** `TmcGrParColTypWid1.h` · **Ресурс:** `IDD_DIALOGGRLINECOLORTYPEWIDTH1`.
Пустой диалог-заготовка (нет полей и обработчиков). Назначение неясно — вероятно, мёртвый код/дубль `CTmcGrParColTypWid` (то же замечание есть в TMCROS).

### 10.3. Общие с TMCGROUT классы
`CTMCGROUTApp`, `CMainFrame`, `CChildFrame`, `CTMCGROUTDIALOGView`, `CTmcGroutColorGraphDialog`, `CTmcGrParColTypWid`, `CTMCGrExpression`, глобальные `PutTrace`/`PutStatistics` — см. [`tmcgrout.md`](tmcgrout.md), разделы 4–6, 9–11. Единственное отличие приложения — ключ реестра `"TMC_DN"` вместо `"TMCGROUT"` в `InitInstance`.

---

## 11. Требует уточнения у автора (К.Н. Климов)

1. **DN-варианты структур** `TMC_GR_DOC1_DN` / `TMC_GR_DOC_DN` (поле `bIsReduceElMagnAnalysis`): чем они отличаются по использованию от базовых `TMC_GR_DOC1`/`TMC_GR_DOC`, и в каких местах кода реально читаются.
2. **Семантика выходного файла ДН**: при `WriteData` для больших решёток пишется «Emitted Power», для обычных — `Kip`/`Kip dB`. Подтвердить физический смысл «Emitted Power» как замены КИП.
3. **Порог `nMaxTemporary = 400`** и кэширование во временный файл `*_Temp*`: подтвердить логику (когда кэш считается актуальным, поведение `bIsInitialData`).
4. **Угловая сетка**: для обычной решётки углы идут `0..π` (`InitAngle`), для большой — `−π..π`. Подтвердить, что это намеренно и не влияет на сопоставимость ДН.
5. **Формула КИП**: в `NormirovkaDiagrNapr` КИП считается как отношение КНД решётки к КНД «единичной» решётки (`_ForKip`). Подтвердить корректность определения.
6. **`OnDestroy` с циклом ожидания потока** (`for(;!bIsDataCalculate;) { ... Sleep(1000); }`): подтвердить, что повторный показ модального окна — корректный способ дождаться потока (нет ли риска зависания при ошибке расчёта).
7. **Связь с известным багом «кнопка статистики»** (см. `CHANGELOG.md`): относится ли он к прогресс-окну `CThreadCalcDirPat`/выводу КНД-КИП в этом вьювере или к счётным ядрам H/X.
8. **Дефект `sprintf` без буфера-приёмника** в `CTmcSMatrix` (унаследован из общего кода с TMCROS) — исправлять ли.
9. **Файл-выражение ДН элемента** в `.dat` (третья строка): допустимый синтаксис и набор функций (через exprint); как соотносятся `sin(f)`/`cos(f)` с угловым диапазоном `FiMin..FiMax`.

---

## 12. Нужные иллюстрации

- 📸 НУЖНА СХЕМА: поток данных `«.dat решётка» → c2DArray (cIzl[]) → CalculateDiagrNapr → pdRealDN/pdImageDN → CTMCGROUTView` и роль `pcDirectionalPattern[]`.
- 📸 НУЖНА СХЕМА: фоновый расчёт — `OnInitialUpdate → AfxBeginThread(ReadDocFileW_Thread1) → CThreadCalcDirPat (прогресс по c2DArray::iN)`.
- 📸 НУЖНА СХЕМА: геометрия суммирования полей решётки (координаты `x,y`, угол `dAngle`, фазовый набег `2π·f·c·cos(dAngle−α)/c₀`).
- Скриншоты окон (главное окно с ДН, диалоги девиаций, прогресс-окно) относятся к руководству пользователя, а не к API-документации.
