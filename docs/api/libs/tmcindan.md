# Библиотека TMCIndan — API-документация

> Пакет **TMC Suite**. Статическая библиотека — чтение входного файла-шаблона расчёта (`.tpl`), хранение топологии планарной структуры (блоков-примитивов), параметров расчёта, секции вывода, а также интегрирование поля во времени и экспорт в диаграмму направленности.
> Язык документации: русский. Все сигнатуры приведены по исходникам из `src/libs/TMCIndan/` и общим заголовкам из `src/Include/`.

---

## 1. Назначение библиотеки

**TMCIndan** — статическая библиотека (`StaticLibrary`), которую линкуют **счётные ядра** PlanarRT_H и PlanarRT_X (см. [«Зависимости»](../../architecture/dependencies.md)). Её задача — превратить текстовый входной файл расчёта (`.tpl`) в объектную модель расчётной задачи:

- **запуск препроцессора** PREPR над `.tpl` (создание временного развёрнутого файла);
- **разбор секций** входного файла: параметры (`#PARM`), топология (`#TOPOLOGY`), список связей блоков (`#LINK_LIST`), вывод (`#OUTPUT`), шаги (`#STEP`);
- **хранение топологии** в виде односвязного списка блоков-примитивов (прямоугольники, окружности, многоугольники, файлы распределения eps, входы/возбуждения), каждый со своим выражением диэлектрической проницаемости, магнитной проницаемости, потерь и т.п.;
- **хранение глобальных параметров** расчёта (единицы измерения, сетка `Delta`, границы области `Xmin..Ymax`, частота, время, высота);
- **хранение настроек вывода** (имена выходных файлов, флаги вывода S-матрицы, поля, топологии);
- **интегрирование поля** по времени (накопление амплитуды и фазы Ez) и экспорт в файл диаграммы направленности.

> ⚠️ Имя «Indan» (наведённые токи / «индан-блоки») и часть прикладной семантики относятся к научной модели проекта. Математику расчётов править нельзя. Здесь документируется **программный интерфейс**, а не физический смысл формул.

---

## 2. Расположение исходников и состав сборки

Исходники: `src/libs/TMCIndan/`. Заголовки: `src/Include/` (подключаются как `<TmcRTH_*.h>`).
Проект сборки — `TMCIndan.vcxproj` (тип **StaticLibrary**, конфигурации Win32/x64).

В сборку входят **8** файлов `.cpp`:

| Файл `.cpp` | Класс / содержимое | Заголовок |
|-------------|--------------------|-----------|
| `TmcRTH_Indan.cpp` | `CTmcRTH_Indan` — главный фасад: открытие `.tpl`, запуск PREPR, чтение секций по шагам | `TmcRTH_Indan.h` |
| `TmcRTH_IndanParam.cpp` | `CTmcRTH_IndanParam` — глобальные параметры расчёта, единицы измерения, сетка узлов | `TmcRTH_IndanParam.h` |
| `TmcRTH_IndanTopology.cpp` | `CTmcRTH_IndanTopology` — обёртка над списком блоков и параметрами | `TmcRTH_IndanTopology.h` |
| `TmcRTH_BolckList.cpp` | `CTmcRTH_BlockList` — односвязный список блоков-примитивов топологии | `TmcRTH_BolckList.h` |
| `TmcRTH_IndanOutput.cpp` | `CTmcRTH_IndanOutput` — настройки и имена файлов вывода | `TmcRTH_IndanOutput.h` |
| `FieldIntegrated.cpp` | `CFieldIntegrated` — интегрирование поля по времени, экспорт в диаграмму направленности | `FieldIntegrated.h` |
| `TmcRTH_Input.cpp` | `CTmcRTH_Input` — узлы возбуждения (вход), внутренний класс | `TmcRTH_Input.h` |
| `TmcRTH_InputNode.cpp` | `CTmcRTH_InputNode` — один узел возбуждения, внутренний класс | `TmcRTH_InputNode.h` |

> ⚠️ В каталоге `src/Include/Indan/` лежат **более старые копии** части этих заголовков (`TmcRTH_Indan.h`, `TmcRTH_IndanOutput.h`, `TmcRTH_IndanTopology.h`, `TmcRTH_Input.h`, `TmcRTH_InputNode.h`, `FieldIntegrated.h`, плюс `TmcRTHNodeDiel.h`, `TmcRTHRectNode.h`, `Pl_iofor1.h`). В сборку `TMCIndan.vcxproj` они **не входят**: проект подключает заголовки из `..\..\INCLUDE\`. Файлы `TmcRTHNodeDiel.h`, `TmcRTHRectNode.h`, `Pl_iofor1.h` относятся к реализации счётного ядра (классы наследуют `CWnd`) и здесь не описываются как часть библиотеки. См. раздел «Требует уточнения у автора».

---

## 3. Базовые типы и зависимости

Библиотека опирается на общие заголовки и типы:

| Заголовок | Что даёт |
|-----------|----------|
| `typerth.h` | Тип `_real` (`float` или `double` в зависимости от макроса `_PREC_FLOAT`/`_PREC_DOUBLE`); `_ELEM_VAL_RTH` (= `_real`); физические константы `C0___`, `MU0___`, `EPS0___`, `PI___`; макросы возбуждения электрона `ELECTRON_Q___`, `ELECTRON_M___` |
| `TmcLibError.h` | Класс `CTmcLibError` — хранение и передача сообщения об ошибке (методы `PutErrorMessage`, `IsError`, `Clear`, `GetErrorMessage`) |
| `Tmcgrviw.h` | Строковые имена типов блоков (`CTMCRTH_INDANBLCK_*`) и числовые коды типов (`CTMCRTH_BLCKNTYPE_*`) — **общий заголовок**, править нельзя |
| MFC (`CString`, `CWnd`) | Строки и базовый класс окна |
| PREPR (`prepr1.h`) | Препроцессор `.tpl` через функцию-обёртку `MainPrepr()` |

> **`_real`** в текущей сборке = `float` (в `typerth.h` активен `#define _PREC_FLOAT`).

### Тип значения узла `_ELEM_VAL_RTH`
`_ELEM_VAL_RTH` — тип хранимого значения напряжения в узле. По умолчанию `_ELEM_VAL_RTH_REAL` ⇒ равен `_real`. Альтернатива `_ELEM_VAL_RTH_INT` (целочисленное представление с множителем `2^20`) в текущей сборке не активна.

---

## 4. Свободные (нечленские) функции

### `void MainPrepr( CString csTplFileNamePrepr, CString csTplFileName, CString csCurrentPath, CTmcLibError &cError )`
**Объявлена в:** `TmcRTH_Indan.h`.
**Что делает:** обёртка над препроцессором PREPR. Запускает развёртывание входного шаблона `csTplFileName` (директивы `#define`/`#include`, удаление комментариев) и записывает результат во временный файл `csTplFileNamePrepr`.

| Параметр | Тип | Назначение |
|----------|-----|-----------|
| `csTplFileNamePrepr` | `CString` | имя выходного (развёрнутого) временного файла |
| `csTplFileName` | `CString` | имя исходного `.tpl`-шаблона |
| `csCurrentPath` | `CString` | текущий каталог (для разрешения `#include`) |
| `cError` | `CTmcLibError&` | приёмник ошибок (out) |

**Ошибки:** сообщение об ошибке кладётся в `cError`.

### Функции управления глобальным шагом `DELTA_T` (объявлены в `TmcRTH_IndanParam.h`)
Глобальные функции работы с механическим шагом по времени (флаг и значение хранятся глобально, вне класса):

| Функция | Назначение |
|---------|-----------|
| `void SetDeltaT( double dDeltaT1 )` | задать значение `DELTA_T` (число) |
| `void SetDeltaT( char *lpszDeltaT )` | задать `DELTA_T` из строки |
| `double GetDeltaT( void )` | получить текущее значение `DELTA_T` |
| `BOOL IsDeltaTDefine( void )` | определён ли `DELTA_T` |
| `void SwitchDeltaT( void )` | переключить флаг определённости `DELTA_T` |

> ⚠️ Эти функции также объявлены в `typerth.h`; их реализация может находиться в счётном ядре. Назначение «механического» шага относится к модели движущихся блоков (`RECT_MOVE`, `CIRCLE_MOVE`). Требует уточнения у автора.

---

## 5. Класс `CTmcRTH_Indan` — главный фасад чтения `.tpl`

**Назначение:** верхнеуровневый интерфейс библиотеки. Открывает `.tpl`-файл, прогоняет его через PREPR в временный файл, ищет и читает секции (`#PARM`, `#TOPOLOGY`, `#LINK_LIST`, `#OUTPUT`) для заданного шага (`#STEP`), отдаёт наружу параметры, топологию и настройки вывода.
**Заголовок:** `src/Include/TmcRTH_Indan.h`
**Зависит от:** `TmcLibError.h`, `TmcRTH_IndanParam.h`, `TmcRTH_IndanOutput.h`, `TmcRTH_IndanTopology.h`, PREPR.

### Важные константы (макросы) заголовка
| Макрос | Значение | Назначение |
|--------|----------|-----------|
| `CTMCRTH_INDANSBUF` | `30000` | размер буфера для строковых операций |
| `CTMCRTH_INDANPREPRFNA` | `"~$Prepr$~"` | суффикс имени временного файла после препроцессора |
| `CTMCRTH_INDAN_LINELEN` | `20000` | максимальная длина читаемой строки |
| `CTMCRTH_INDANMET_PROGRAM` | `"#TMC_RT_H"` | маркер программы в начале файла |
| `CTMCRTH_INDANMET_EOF` | `"#EOF"` | маркер конца файла |
| `CTMCRTH_INDANMET_STEP` / `_ENDSTEP` | `"#STEP"` / `"#END_STEP"` | границы секции шага |
| `CTMCRTH_INDANMET_PARAM` / `_ENDPARAM` | `"#PARM"` / `"#END_PARM"` | границы секции параметров |
| `CTMCRTH_INDANMET_TOPOLOGY` / `_ENDTOPOLOGY` | `"#TOPOLOGY"` / `"#END_TOPOLOGY"` | границы секции топологии |
| `CTMCRTH_INDANMET_LINKLIST` / `_ENDLINK` | `"#LINK_LIST"` / `"#END_LINK"` | границы секции связей |
| `CTMCRTH_INDANMET_OUTPUT` / `_ENDOUTPUT` | `"#OUTPUT"` / `"#END_OUTPUT"` | границы секции вывода |
| `CTMCRTH_INDANMET_BLOCK` / `_ENDBLOCK` | `"BLOCK "` / `"END_B"` | границы описания одного блока |
| `CTMCRTH_INDANMET_LINK` | `"T "` | маркер строки связи |

### Поля (приватные)
| Поле | Тип | Назначение |
|------|-----|-----------|
| `cTopology` | `CTmcRTH_IndanTopology` | топология (список блоков) текущего шага |
| `cParam` | `CTmcRTH_IndanParam` | глобальные параметры расчёта |
| `cOut` | `CTmcRTH_IndanOutput` | настройки вывода |
| `cError` | `CTmcLibError` | накопитель ошибок |
| `nStep` | `int` | номер текущего шага |
| `pfPrepr` | `FILE*` | дескриптор временного (развёрнутого) файла |
| `csCurrentPath` | `CString` | каталог исходного `.tpl` |
| `csTplFileName` | `CString` | имя исходного `.tpl` |
| `csTplFileNamePrepr` | `CString` | имя временного файла после PREPR |

### Конструктор / деструктор
```cpp
CTmcRTH_Indan();          // обнуляет поля, очищает ошибку
virtual ~CTmcRTH_Indan(); // вызывает DeleteData() (удаляет временный файл)
```

### Публичные методы

#### `void SetTplFileName( CString csFileName )` и `void SetTplFileName( char *pszFileName )`
**Что делает:** задаёт имя входного `.tpl`, проверяет, что файл открывается, определяет текущий каталог и запускает PREPR (создаёт временный развёрнутый файл).
**Как работает:** `DeleteData()` → `fopen(csFileName,"r")` (проверка) → `SetCurrentPath()` → формирует `csTplFileNamePrepr` = имя + `CTMCRTH_INDANPREPRFNA` → `MakePreprFile()`.
**Ошибки:** «can't open file …», «very long file name …» — в `cError`.

#### `void ReadData( CString csFileName, int nStepNum )`
**Что делает:** полностью читает данные шага `nStepNum` из файла `csFileName` (задаёт имя, находит шаг, читает все секции).

| Параметр | Тип | Назначение |
|----------|-----|-----------|
| `csFileName` | `CString` | имя `.tpl` |
| `nStepNum` | `int` | номер шага (с какого `#STEP` читать) |

#### `void SetStepNumber( int nStep1 )`
**Что делает:** устанавливает номер текущего шага.

#### `void ReadParamSection()`, `void ReadTopologySection()`, `void ReadLinkListSection()`, `void ReadOutputSection()`
**Что делают:** читают соответствующую секцию текущего шага из временного файла. Внутренне находят границы секции по маркерам (`FindSection`) и передают содержимое в `cParam.Read()`, `cTopology.Read()`, `cTopology.ReadLink()`, `cOut.Read()`.

#### `int GetNStepMax( void )` и `int GetNStepMax( CString csFileName )`
**Что делает:** возвращает количество шагов (`#STEP`) в файле. Перегрузка со строкой предварительно задаёт файл.
**Возвращает:** число шагов.

#### Геттеры параметров и результатов
| Метод | Возвращает | Смысл |
|-------|-----------|-------|
| `int GetnStep( void )` | `int` | номер текущего шага |
| `int GetnBlock( void )` | `int` | число блоков в топологии |
| `void GetXYMinMax( double&, double&, double&, double& )` | — | границы области `Xmin,Xmax,Ymin,Ymax` (out-параметры) |
| `void GetnXnY( int&, int& )` | — | число узлов сетки по X и Y |
| `double dGetDelta( void )` | `double` | шаг сетки `Delta` |
| `double dGetdT( void )` | `double` | шаг по времени |
| `double dGetTmin( void )` | `double` | начальное время |
| `int nGetTmax( void )` | `int` | конечный отсчёт времени |
| `CTmcRTH_IndanParam& GetParam( void )` | ссылка | объект параметров |
| `CTmcRTH_IndanTopology& GetTopology( void )` | ссылка | объект топологии |
| `CTmcRTH_IndanOutput* GetOutput( void )` | указатель | объект вывода |
| `CString GetCurrentPath( void )` | `CString` | каталог `.tpl` |
| `CString GetTopologyFileName( void )` | `CString` | имя файла топологии |
| `BOOL IsDataRead( void )` | `BOOL` | прочитаны ли данные |

#### Обработка ошибок
| Метод | Назначение |
|-------|-----------|
| `BOOL IsError( void )` | есть ли ошибка |
| `CString GetErrorMessage( void )` | текст ошибки |

#### Флаги вывода (делегируются в `cOut`)
`void OnOffSoundEffects()`, `void OnOffFieldOutput()`, `void OnOffFieldOutput1()`, `void OnOffFieldOutputU()`, `void OnOffSinchronizationFieldOutput()` — переключают соответствующие флаги вывода (звук, поле, поле-1, поле-U, синхронный вывод поля).

#### `CTmcLibError WriteFileForMaple( CString csFileName, CString csFileNameData, double dHeight, double dRolikR1, double dRolikR2, double dStatorR1, double dStatorR2, double dMagnMomP )` (+ перегрузка с `char*`)
**Что делает:** записывает файл для системы Maple (экспорт геометрии/данных).

| Параметр | Тип | Назначение |
|----------|-----|-----------|
| `csFileName` | `CString`/`char*` | имя выходного файла Maple |
| `csFileNameData` | `CString`/`char*` | имя файла с данными |
| `dHeight` | `double` | высота структуры |
| `dRolikR1`, `dRolikR2` | `double` | радиусы «ролика» (внутр./внешн.) |
| `dStatorR1`, `dStatorR2` | `double` | радиусы статора (внутр./внешн.) |
| `dMagnMomP` | `double` | магнитный момент |

**Возвращает:** `CTmcLibError` с результатом.
> ⚠️ Прикладной смысл параметров (ролик, статор, магнитный момент) — модель электрической машины; требует уточнения у автора.

### Приватные методы (внутренняя кухня)
`MakePreprFile()` (запуск PREPR), `OpenPreprFile()`/`ClosePreprFile()` (работа с временным файлом), `FindSection()`/`FindSection(char*,char*)` (поиск границ секции), `FindStep(int)` (позиционирование на шаг), `CalcNStepMax()` (подсчёт шагов), `GetLine()`/`GetLineSection()`/`GetBlock()` (построчное чтение), `del_blanks()` (удаление пробелов), `SetCurrentPath()`, `DeleteData()`.

---

## 6. Класс `CTmcRTH_IndanParam` — глобальные параметры расчёта

**Назначение:** хранит глобальные параметры расчётной области и единицы измерения; пересчитывает узлы сетки (номер узла ↔ координаты).
**Заголовок:** `src/Include/TmcRTH_IndanParam.h`
**Зависит от:** `typerth.h`, `TmcLibError.h`.

### Единицы измерения (макросы заголовка)
Для каждой величины задана пара «строковый идентификатор + строковое имя единицы + числовой коэффициент перевода в СИ»:

| Группа | ID | Единицы (имя → коэффициент) |
|--------|----|------------------------------|
| Длина | `LONG_UNIT ` | `m`→1, `cm`→0.01, `mm`→0.001 |
| Время | `TIME_UNIT ` | `s`→1, `ms`→1e-3, `mks`→1e-6, `ns`→1e-9, `ps`→1e-12 |
| Угол | `ANGLE_UNIT ` | `radian`→1, `gradus`→π/180 |
| Частота | `FREQ_UNIT ` | `Hz`→1, `kHz`→1e3, `MHz`→1e6, `GHz`→1e9 |
| Магн. индукция | `B_UNIT ` | `Tl`→1, `mTl`→1e-3, `mkTl`→1e-6 |
| Объёмная концентрация | `N_UNIT ` | `m3`→1, `cm3`→1e-6, `mm3`→1e-9 |

Идентификаторы значений: `FREQ `, `DELTA `, `TIME `, `X_MIN `, `X_MAX `, `Y_MIN `, `Y_MAX `, `HEIGHT `, `DELTA_T_MECHANICAL `.

### Поля (публичные)
| Поле | Тип | Назначение |
|------|-----|-----------|
| `rDelta` | `double` | шаг сетки (Δx = Δy) |
| `rFreq` | `double` | частота |
| `rHeight` | `double` | высота структуры |
| `rt` | `double` | время |
| `rXmin`, `rXmax`, `rYmin`, `rYmax` | `double` | границы прямоугольной области |
| `rTmin`, `rTmax` | `double` | временной интервал |
| `rDeltaTMechanical` | `double` | механический шаг по времени |
| `csLongUnit` … `csNUnit` | `CString` | имена выбранных единиц |
| `rLongUnit` … `rNUnit` | `double` | коэффициенты перевода единиц в СИ |
| `cError` | `CTmcLibError` | накопитель ошибок |

### Методы

#### `void Read( CString &csCh, CTmcLibError &cError1 )`
**Что делает:** разбирает текст секции `#PARM` и заполняет поля. Внутренне распознаёт идентификаторы и вызывает приватные `ReadDelta`, `ReadFreq`, `ReadTime`, `ReadXmin/Xmax/Ymin/Ymax`, `ReadHeight`, `ReadDeltaTMechanical`, а также `Read*Unit`.

#### `void IsCorrectly( CTmcLibError &cError1 )`
**Что делает:** проверяет согласованность параметров; пересчитывает `rXmax`/`rYmax` так, чтобы они укладывались в целое число шагов `Delta` (`rXmax = rXmin + CalcnX()*rDelta`).

#### `int CalcnX( void )` / `int CalcnY( void )`
**Что делает:** число узлов сетки по осям.
**Как работает:** `nX = (int)((Xmax−Xmin)/Delta)+1` (то же для Y). При `|Delta| < FLT_MIN` возвращает 0 (защита от деления на ноль).
**Возвращает:** число узлов (int).

#### `int CalcnNodeGlobal( double x, double y )`
**Что делает:** переводит координаты `(x,y)` в глобальный номер узла сетки.
**Как работает:** точка зажимается в границы области; индекс = `iy*nX + ix`, где ix, iy — округлённые индексы по Delta (с учётом крайних половин-ячеек).

| Параметр | Тип | Назначение | Допустимые значения |
|----------|-----|-----------|---------------------|
| `x` | `double` | координата X | любое вещественное (зажимается в `[Xmin,Xmax]`) |
| `y` | `double` | координата Y | любое вещественное (зажимается в `[Ymin,Ymax]`) |

**Возвращает:** глобальный номер узла (≥0).

#### `int CalcnNodeGlobalFM( double x, double y )`
То же, что `CalcnNodeGlobal`, но с другим смещением границы (`1.5*Delta` вместо `0.5*Delta`).
> ⚠️ Суффикс «FM» (Force/Moment?) — назначение требует уточнения у автора.

#### `int CalcnNodeGlobalNorm( double x, double y )`
**Что делает:** как `CalcnNodeGlobal`, но если точка ближе 2·Delta к границе области — возвращает **−1** (точка не в «нормальной» внутренней зоне).
**Возвращает:** номер узла или **−1**.

#### `double CalcX( int nNumber )` / `double CalcY( int nNumber )`
**Что делает:** обратное преобразование — координата узла по его глобальному номеру.
**Как работает:** `nX=(int)((Xmax−Xmin)/Delta)+1`; `CalcX`: остаток `nNumber mod nX`, координата `Xmin+ix*Delta`; `CalcY`: целое `nNumber/nX`, координата `Ymin+iy*Delta`.

#### `BOOL IsnInRegion( int n )` / `BOOL IsnInRegion( _real x, _real y )`
**Что делает:** проверяет, попадает ли узел (по номеру или по координатам) в сетку `[0, nX*nY)`.
**Возвращает:** `TRUE`/`FALSE`.

#### Геттеры
`double dGetDelta()`, `double dGetdT()`, `double dGetTmin()`, `int nGetTmax()`, `double dGetHeight()`, `double rGetLongUnit()`, `CString csGetLongUnit()`, `void GetXYMinMax(double&,double&,double&,double&)`.

#### `CTmcRTH_IndanParam& operator=( CTmcRTH_IndanParam& cParam )`
Копирующее присваивание (все поля).

### Приватные методы
`Read*Unit` (разбор единиц), `Read*` (разбор отдельных параметров), `Search_1Param()` (поиск значения по идентификатору в строке), `expr_del_Blanks2()` (удаление пробелов в выражении), `DeleteData()`.

---

## 7. Класс `CTmcRTH_IndanTopology` — топология (обёртка над списком блоков)

**Назначение:** связывает список блоков `CTmcRTH_BlockList` с параметрами `CTmcRTH_IndanParam`; читает секцию топологии и секцию связей.
**Заголовок:** `src/Include/TmcRTH_IndanTopology.h`
**Зависит от:** `TmcRTH_BolckList.h`, `TmcRTH_IndanParam.h`.

### Поля (приватные)
| Поле | Тип | Назначение |
|------|-----|-----------|
| `pcParam` | `CTmcRTH_IndanParam*` | указатель на внешние параметры |
| `cBlockList` | `CTmcRTH_BlockList` | голова списка блоков |

### Методы
| Метод | Что делает |
|-------|-----------|
| `void Read( CString &csCh, CTmcLibError &cError )` | разбирает секцию `#TOPOLOGY`, наполняет список блоков |
| `void ReadLink( CString &csCh, CTmcLibError &cError )` | разбирает секцию `#LINK_LIST` (связи блоков, строки `T `) |
| `void IsCorrectly( CTmcLibError &cError )` | проверка корректности всех блоков |
| `void IsCorrectlyLink( CTmcLibError &cError )` | проверка корректности связей |
| `void SetParam( CTmcRTH_IndanParam *pcParam1 )` | задать указатель на параметры |
| `CTmcRTH_IndanParam* GetpParam( void )` | получить указатель на параметры |
| `CTmcRTH_BlockList* GetBlockList( void )` | голова списка блоков |
| `int GetBlockNumber( void )` | число блоков |
| `BOOL IsDataRead( void )` | прочитаны ли данные |
| `void AddPathForFile( CString &csCurrentPath )` | дописать путь каталога к именам файлов в блоках |
| `operator=` | копирующее присваивание |
| `void DeleteData( void )` | очистка |

---

## 8. Класс `CTmcRTH_BlockList` — список блоков-примитивов топологии

**Назначение:** односвязный список «блоков» — геометрических примитивов планарной структуры (прямоугольник, окружность, многоугольник, распределение eps из файла, вход/возбуждение). Каждый узел хранит геометрию, тип и текстовые выражения свойств среды (eps, mu, потери, скорости и т.д.). Читает описание блока из текста, проверяет корректность, умеет сохранять/загружать.
**Заголовок:** `src/Include/TmcRTH_BolckList.h`
**Зависит от:** `TmcLibError.h`, `typerth.h`, `TmcRTH_IndanParam.h`. Имена и коды типов блоков берутся из `Tmcgrviw.h`.

### Типы блоков (строковые имена, из `Tmcgrviw.h`)
| Группа | Макрос → строка |
|--------|------------------|
| Прямоугольники | `RECT_STAT `, `RECT_STAT_N `, `RECT_STAT_B `, `RECT_STAT_Y `, `RECT_MOVE ` |
| Окружности | `CIRCLE_STAT `, `CIRCLE_STAT_N `, `CIRCLE_STAT_B `, `CIRCLE_STAT_Y `, `CIRCLE_MOVE ` |
| Многоугольники | `POLYGON_STAT `, `POLYGON_STAT_N `, `POLYGON_STAT_B `, `POLYGON_STAT_Y `, `POLYGON_MOVE `, точка многоугольника `L ` |
| Из файла | `FILE `, `FILE_N `, `FILE_B `, `FILE_Y ` |
| Входы | `INPUT_X `, `INPUT_Y ` |
| Подтипы среды | `MAGNETIC`, `METAL`, `ABSORBER` |

Суффиксы: `_STAT` — статический блок, `_N`/`_B`/`_Y` — варианты задания (см. реализацию `Read*StatN/B`), `_MOVE` — движущийся блок (есть скорости `Vx`,`Vy`,`W`).

### Числовые коды типа `nType` (из `Tmcgrviw.h`)
| Код (макрос) | Значение | Смысл |
|--------------|----------|-------|
| `CTMCRTH_BLCKNTYPE_EPS` | 0 | диэлектрик (eps по выражению) |
| `CTMCRTH_BLCKNTYPE_ABSORBER` | 100 | поглотитель |
| `CTMCRTH_BLCKNTYPE_METAL` | 101 | металл |
| `CTMCRTH_BLCKNTYPE_INPXLEFT` | 102 | вход X слева `>` |
| `CTMCRTH_BLCKNTYPE_INPXRIGHT` | 103 | вход X справа `<` |
| `CTMCRTH_BLCKNTYPE_INPYTOP` | 104 | вход Y сверху `V` |
| `CTMCRTH_BLCKNTYPE_INPYBOT` | 105 | вход Y снизу `^` |
| `CTMCRTH_BLCKNTYPE_MAGNETIC` | 107 | магнитная среда |
| `CTMCRTH_BLCKNTYPE_ABSR_*` | 1..24 | подтипы поглотителя (битовые маски граней) |

### Константы формата файла распределения eps (`.eps`)
`TMC_RTH_EPSFILE_ID_` (сигнатура), `TMC_RTH_EPSFILE_TYPE_`, `_Delta_`, `_XMIN_`, `_YMIN_`, `_NX_`, `_NY_`, `_NPoint_`, `_NAccur_`, `_NdFrmt_`, `_VlFrmt_` — ключи заголовка файла `.eps`. Также `TMC_RTH_BLOCKLISTSAVE_BEGIN`/`_END` — границы блока при сохранении; `TMC_RTH_BLOCKLISTLOADBUFSIZE` (20000) — размер буфера загрузки.

### Поля (приватные)
| Поле | Тип | Назначение |
|------|-----|-----------|
| `nBlock` | `int` | номер блока |
| `nType` | `int` | числовой код типа (см. таблицу `nType`) |
| `csBlock` | `CString` | строковое имя типа блока |
| `dX0`, `dY0` | `double` | опорная точка/центр блока |
| `dXmin..dYmax` | `double` | габаритный прямоугольник блока |
| `dX`, `dY` | `double*` | массивы координат вершин (для многоугольника) |
| `nXY` | `int` | размер массивов вершин |
| `csEpsExpr` | `CString` | выражение для диэлектрической проницаемости eps |
| `csMuExpr` | `CString` | выражение для магнитной проницаемости mu |
| `csDzExpr`, `csDzExprOut` | `CString` | выражения толщины Dz (вход/выход) |
| `csHxExpr`, `csHyExpr` | `CString` | выражения подмагничивающего поля Hx, Hy |
| `csForceRo`, `csForceSigma`, `csForceConductivity` | `CString` | выражения плотности, проводимости (силовая модель) |
| `csFileNameMechanicalFreedomDegree` | `CString` | имя файла механических степеней свободы |
| `csVx`, `csVy` | `CString` | линейная скорость по X/Y (для входа — амплитуда/фаза от t) |
| `csW` | `CString` | угловая скорость (для входа — eps) |
| `nMemory` | `int` | служебный счётчик памяти |
| `pcNextBlockList` | `CTmcRTH_BlockList*` | указатель на следующий блок |
| `pszBufForReadTopology[20000]` | `char[]` | буфер чтения топологии |

### Методы (публичные)

#### `void Add( CString &csCh, CTmcLibError &cError, CTmcRTH_IndanParam *pcParam )`
**Что делает:** разбирает текст одного блока `csCh` и добавляет новый узел в конец списка.

| Параметр | Тип | Назначение |
|----------|-----|-----------|
| `csCh` | `CString&` | текст описания блока |
| `cError` | `CTmcLibError&` | приёмник ошибок |
| `pcParam` | `CTmcRTH_IndanParam*` | параметры (единицы, сетка) для пересчёта координат |

#### `void Add( CTmcRTH_BlockList &csCh )`
**Что делает:** добавляет копию готового блока в конец списка (рекурсивно идёт до хвоста).

#### `CTmcRTH_BlockList* FindBlock( int nBlock1 )`
**Что делает:** ищет блок по номеру. **Возвращает:** указатель на блок или `NULL`.

#### `CTmcRTH_BlockList* GetNext( void )`
**Возвращает:** следующий блок списка (или `NULL`).

#### `void SetLink( int iBuf, double x0, double y0, CTmcLibError &cError )`
**Что делает:** задаёт связь блока с номером `iBuf` и его опорные координаты `x0,y0`.

#### `void IsCorrectly( CTmcLibError &cError )` / `void IsCorrectlyLink( CTmcLibError &cError )`
**Что делает:** проверка геометрии/типа блока и корректности связей соответственно.

#### `BOOL Save( CString &csBuffer )` / `BOOL Save1( CString &csBuffer )`
**Что делает:** сериализует список (Save1 — вариант формата) в строковый буфер. **Возвращает:** успех.

#### `void Load( FILE **fp, CTmcLibError &cError )`
**Что делает:** загружает список блоков из открытого файла.

#### `void AddPathForFile( CString &csPath )`
**Что делает:** дописывает путь каталога к именам файлов-распределений (`.eps`) в блоках типа `FILE`.

#### Сеттеры/геттеры
Геометрия: `SetX0/SetY0/GetX0/GetY0`, `SetXmin/SetXmax/SetYmin/SetYmax` + соответствующие `Get*`, `double* GetpX()`, `double* GetpY()`, `int GetnXY()`.
Тип и номер: `SetnType(int)/GetnType()`, `SetnBlock(int)/GetnBlock()`, `SetcsBlock(CString)/GetcsBlock()`, `GetBlockNumber()`.
Память: `SetnMemory(int)/GetnMemory()/GetnMemoryAll()`.
Выражения: `GetcsEpsExpr()`, `GetcsMuExpr()`, `GetcsDzExpr()`, `GetcsDzExprOut()`, `GetcsHxExpr()`, `GetcsHyExpr()`, `GetcsForceRo()`, `GetcsForceSigma()`, `GetcsForceConductivity()`, `GetcsFileNameMechanicalFreedomDegree()`, `GetcsVx()`, `GetcsVy()`, `GetcsW()`.
Прочее: `operator=`, `DeleteData()`, `del_eol(char*)`.

### Приватные методы (разбор конкретных типов)
Семейства `ReadFileStat*`, `ReadRectStat*`, `ReadCircleStat*`, `ReadPolygonStat*` (с вариантами `N`/`B`/`_1`), `ReadRectMove`/`ReadCircleMove`/`ReadPolygonMove`, `ReadInputX`/`ReadInputY`, общий `Read()`, `ReadXY()` (чтение пары координат), `Search_1Param()`, `expr_del_Blanks2()`, `CalcEOL()`/`SkipEOL()`, `IsFullName()`, `AddCurrentPath()`, `AddFileEpsExtention()`.

> Класс `CTMCBlList` объявлен другом (`friend class CTMCBlList;`) — это внешний класс (из вьюверов/ядра), которому открыт доступ к приватным полям.

---

## 9. Класс `CTmcRTH_IndanOutput` — настройки вывода

**Назначение:** хранит флаги и имена файлов вывода результатов расчёта (S-матрица, поле, топология, распределение поля), а также содержит объект интегрирования поля.
**Заголовок:** `src/Include/TmcRTH_IndanOutput.h`
**Зависит от:** `TmcLibError.h`, `FieldIntegrated.h`.

### Идентификаторы секции вывода (макросы)
`FILE ` (`CTMCRTH_FILEOUT_ID`), `TOPOLOGY` (`_TOPOLOGYOUT_ID`), `FIELDS` (`_FIELDSOUT_ID`), `SMATRIX` (`_SMATRIXOUT_ID`), `FIELD_DISTRIBUTION ` (`_FLDSDSTROUT_ID`), `FIELD_DISTRIBUTION_M ` (`_FLDSDSMROUT_ID`).

### Поля (публичные)
| Поле | Тип | Назначение |
|------|-----|-----------|
| `dTmin`, `dTmax` | `double` | временной интервал вывода |
| `dFreq` | `double` | частота |
| `dFreqUnit`, `dTimeUnit` | `double` | коэффициенты единиц частоты и времени |
| `bOutSMatrix` | `BOOL` | выводить S-матрицу |
| `bOutField`, `bOutField1`, `bOutFieldU` | `BOOL` | выводить поле / поле-1 / поле-U |
| `bOutTopology` | `BOOL` | выводить топологию |
| `bOutFieldSinchronization` | `BOOL` | синхронный вывод поля |
| `bSoundEffect` | `BOOL` | звуковые эффекты |
| `csFileName` | `CString` | базовое имя выходного файла |
| `csFileNameField`, `…Field1`, `…Field1Mod`, `…Field1Faz`, `…FieldU` | `CString` | имена файлов поля (значение, модуль, фаза, U) |
| `csFileNameTopology` | `CString` | файл топологии |
| `csFileNameSMatrix` | `CString` | файл S-матрицы |
| `csFileNameQ`, `csFileNameIx`, `csFileNameIy` | `CString` | файлы заряда Q и токов Ix, Iy |

### Методы
| Метод | Что делает |
|-------|-----------|
| `void Read( CString &csCh, CTmcLibError &cError1 )` | разбирает секцию `#OUTPUT`, заполняет флаги и имена файлов |
| `void IsCorrectly( CTmcLibError &cError1 )` | проверка настроек вывода |
| `void AddPath( CString csPath )` | дописать путь каталога к именам файлов |
| `void SetFreqAndTimeUnit( double dFr, double dTim )` | задать коэффициенты единиц частоты и времени |
| `void PutSmatrix( void )` | вывести S-матрицу (запись результата) |
| `BOOL bIsFieldDistribution( void )` | включён ли вывод распределения поля |
| `CFieldIntegrated& GetFieldIntegrated( void )` | доступ к объекту интегрирования поля |
| `void OnOffSoundEffects()`, `OnOffFieldOutput()`, `OnOffFieldOutput1()`, `OnOffFieldOutputU()`, `OnOffSinchronizationFieldOutput()` | переключают соответствующие флаги |
| `operator=` | копирующее присваивание |
| `void DeleteData( void )` | очистка |

### Приватные методы
`ReadSmatrix()`, `ReadFileName()`, `ReadTopology()`, `ReadFields()`, `ReadFileDistr()`, `ReadFileDistrInMemory()`, `IsFullPath()`, `Search_1Param()`, `expr_del_Blanks2()`.

---

## 10. Класс `CFieldIntegrated` — интегрирование поля и диаграмма направленности

**Назначение:** накапливает (интегрирует) значения поля Ez по времени, вычисляет амплитуду и фазу на заданной частоте, сохраняет результат в файлы и экспортирует в файл диаграммы направленности (по излучателям вдоль границ области).
**Заголовок:** `src/Include/FieldIntegrated.h`
**Зависит от:** `TmcLibError.h`, `TmcRTH_IndanParam.h`.

### Поля (приватные, ключевые)
| Поле | Тип | Назначение |
|------|-----|-----------|
| `nTCurrent` | `int` | текущий отсчёт времени |
| `dT`, `dTCurrent` | `double` | шаг и текущее время |
| `dTmin`, `dTmax` | `double` | интервал интегрирования |
| `dFreq` | `double` | частота интегрирования |
| `dDelta` | `double` | шаг сетки |
| `dXmin`, `dYmin` | `double` | начало координат области |
| `nX`, `nY` | `int` | размеры сетки |
| `prAmp`, `prFaz` | `_real*` | массивы накопленной амплитуды и фазы |
| `prSin`, `prCos` | `_real*` | вспомогательные массивы sin/cos для интегрирования в памяти |
| `bIsIntegretedInMemory` | `BOOL` | режим интегрирования в памяти |
| `bIsBeginIntegreted` | `BOOL` | началось ли интегрирование |
| `bIsFieldDistribution` | `BOOL` | включён вывод распределения поля |
| `cParam` | `CTmcRTH_IndanParam` | копия параметров |
| `cError` | `CTmcLibError` | накопитель ошибок |
| `dDirPatXmin..dDirPatYmax` | `double` | границы диаграммы направленности |
| `csFileDirectPattern`, `csFileSin`, `csFileCos`, `csFileFaza`, `csFileAmplitude` | `CString` | имена рабочих файлов |

### Методы (публичные)

#### `void InitAllFile( int nX1, int nY1, double dDelta1, double dXmin1, double dYmin1, CTmcRTH_IndanParam& cParam1, double dT1 )`
**Что делает:** инициализирует параметры сетки и рабочие файлы перед началом интегрирования.

| Параметр | Тип | Назначение |
|----------|-----|-----------|
| `nX1`, `nY1` | `int` | размеры сетки |
| `dDelta1` | `double` | шаг сетки |
| `dXmin1`, `dYmin1` | `double` | начало координат |
| `cParam1` | `CTmcRTH_IndanParam&` | параметры расчёта |
| `dT1` | `double` | шаг по времени |

#### `void Integrate( double dTCurrent1, int nTCurrent1, _real *prUNode1 )`
**Что делает:** добавляет вклад текущего временного слоя поля `prUNode1` в накапливаемые амплитуду/фазу.

| Параметр | Тип | Назначение |
|----------|-----|-----------|
| `dTCurrent1` | `double` | текущее время |
| `nTCurrent1` | `int` | номер текущего отсчёта |
| `prUNode1` | `_real*` | массив значений поля по узлам (размер `nX*nY`) |

#### `void ExportToDirectionalPattern( CTmcRTH_IndanParam &cParam1 )`
**Что делает:** экспортирует накопленное поле в файл диаграммы направленности (амплитуда/фаза излучателей вдоль границ Xmin/Xmax/Ymin/Ymax).

#### Сеттеры/геттеры
`SetnX/SetnY/SetnXnY`, `SetFreq/GetFreq`, `SetTmin/GetTmin`, `SetTmax/GetTmax`, `SetFile(...)` (3 перегрузки), `SetIntegrateInMemory()`, `BOOL bIsItegratedInMemory()`, `BOOL bIsFieldDistr()`, `CString GetcsDirectPatternFile()`, `void AddPath(CString)`, `BOOL IsFullPath()`, `CTmcLibError& GetError()`, `CString GetErrorMessage()`, `BOOL IsError()`, `operator=`, `void DeleteData()`.

### Приватные методы (внутренняя кухня)
Семейства `AddDirectionalPatternFileXmin/Xmax/Ymin/Ymax` (+ перегрузки с параметрами излучателя), `ReadAmplitudaAndFazaX/Y`, `ReadAmplitudaAndFazaEz`, `MakeDirectionalPatternFile*`, `GetNumberOfEmittersAlongX/Y`, `IntegrateInMemory()`, `MakeFileAmplitudeAndFaza[FromMemory]()`, `ReadParamFromFile()` / `ReadParamFrom1File(...)`, `InitArrayForIntegrate()`, `InitFile()`, `SetDirectionalPatternParamDefault()`, `IsDirectionalPatternDataCorrect()`.

> ⚠️ Точная физическая нормировка и формулы интегрирования (sin/cos-квадратуры, число излучателей) относятся к научной модели — изменять нельзя; детальное физическое описание требует уточнения у автора.

---

## 11. Класс `CTmcRTH_Input` — узлы возбуждения (вход)

**Назначение:** описывает «вход» (источник возбуждения) — набор узлов на границе со своим направлением и выражением сигнала. Внутренний вспомогательный класс.
**Заголовок:** `src/Include/TmcRTH_Input.h`
**Зависит от:** `typerth.h`, `TmcRTH_InputNode.h`.

### Поля (приватные)
| Поле | Тип | Назначение |
|------|-----|-----------|
| `pcInputNode` | `CTmcRTH_InputNode*` | массив узлов входа |
| `rX`, `rY` | `_real` | координата входа (X — для типов −5/−6; Y — для −3/−4) |
| `nNode` | `int` | число узлов входа |
| `nType` | `int` | направление входа: −3 (X слева `>`), −4 (X справа `<`), −5 (Y сверху `V`), −6 (Y снизу `^`) |
| `csExpression` | `CString` | выражение сигнала возбуждения |

Конструктор/деструктор: `CTmcRTH_Input()`, `~CTmcRTH_Input()`. Приватный `DeleteData()`.

> Коды `nType` совпадают с полем `nType` структуры узла диэлектрика `sTmcRTHNodeDielOne` (см. счётное ядро).

---

## 12. Класс `CTmcRTH_InputNode` — один узел возбуждения

**Назначение:** хранит мгновенное состояние одного узла возбуждения. Внутренний вспомогательный класс.
**Заголовок:** `src/Include/TmcRTH_InputNode.h`
**Зависит от:** `typerth.h`.

### Поля (приватные)
| Поле | Тип | Назначение |
|------|-----|-----------|
| `rUf` | `_real` | падающее напряжение (falling voltage) |
| `rUs` | `_real` | рассеянное напряжение (scattering voltage) |
| `rX`, `rY` | `_real` | координаты узла |
| `rT` | `_real` | текущее время |

Конструктор/деструктор: `CTmcRTH_InputNode()`, `~CTmcRTH_InputNode()`. Других публичных методов нет.

---

## 13. Типичный сценарий использования

```cpp
CTmcRTH_Indan cIndan;

// 1. Задать входной .tpl и прогнать препроцессор
cIndan.SetTplFileName( "task.tpl" );
if( cIndan.IsError() ) { /* cIndan.GetErrorMessage() */ }

// 2. Узнать число шагов
int nSteps = cIndan.GetNStepMax();

// 3. Прочитать данные нужного шага
cIndan.ReadData( "task.tpl", 0 );

// 4. Получить параметры, топологию, вывод
CTmcRTH_IndanParam   &p   = cIndan.GetParam();
CTmcRTH_IndanTopology&t   = cIndan.GetTopology();
CTmcRTH_IndanOutput  *out = cIndan.GetOutput();

int nX, nY;  cIndan.GetnXnY( nX, nY );
double delta = cIndan.dGetDelta();

// 5. Обойти блоки топологии
for( CTmcRTH_BlockList *b = t.GetBlockList(); b && b->GetnBlock()!=0; b = b->GetNext() )
{
    int    type    = b->GetnType();      // код типа (eps/metal/absorber/...)
    CString epsExpr = b->GetcsEpsExpr();  // выражение eps
}
```

---

## 14. Требует уточнения у автора (К.Н. Климов)

1. **`CalcnNodeGlobalFM`** — смысл суффикса «FM» и отличие смещения `1.5*Delta` (Force/Moment?).
2. **`WriteFileForMaple`** — прикладной смысл параметров `dRolikR1/R2`, `dStatorR1/R2`, `dMagnMomP` (модель электрической машины / ролик-статор).
3. **Глобальные функции `SetDeltaT/GetDeltaT/IsDeltaTDefine/SwitchDeltaT`** — где находится их реализация (TMCIndan или счётное ядро) и точный смысл «механического» `DELTA_T_MECHANICAL`.
4. **Подтипы поглотителя** `CTMCRTH_BLCKNTYPE_ABSR_*` (битовые маски граней 0000…1111 и варианты `_1.._4`) — физический смысл масок.
5. **Различие вариантов блоков** `_STAT` / `_STAT_N` / `_STAT_B` / `_STAT_Y` (методы `Read*StatN/B/_1`) — чем отличается формат/семантика каждого варианта.
6. **Класс-друг `CTMCBlList`** — где определён и какие инварианты он опирается при прямом доступе к приватным полям `CTmcRTH_BlockList`.
7. **Старые копии заголовков** в `src/Include/Indan/` (включая `TmcRTHNodeDiel.h`, `TmcRTHRectNode.h`, `Pl_iofor1.h`) — можно ли удалить, или они нужны счётному ядру.
8. **Физическая нормировка интегрирования поля** в `CFieldIntegrated` (sin/cos-квадратуры, число излучателей вдоль границ) — точные формулы.

## 15. Нужные иллюстрации

> 📸 НУЖНА СХЕМА: структура связей классов модуля — `CTmcRTH_Indan` → (`CTmcRTH_IndanParam`, `CTmcRTH_IndanTopology` → `CTmcRTH_BlockList` (список), `CTmcRTH_IndanOutput` → `CFieldIntegrated`).
> 📸 НУЖНА СХЕМА: структура секций входного `.tpl`-файла (`#STEP` → `#PARM` / `#TOPOLOGY` / `#LINK_LIST` / `#OUTPUT`) и порядок их чтения методами `CTmcRTH_Indan`.
> 📸 НУЖНА СХЕМА: нумерация узлов сетки (`CalcnNodeGlobal` ↔ `CalcX`/`CalcY`): соответствие координат `(x,y)` и глобального номера `iy*nX+ix`.
