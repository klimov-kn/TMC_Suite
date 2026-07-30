# Программа PlanarRT_H — API-документация

> Пакет **TMC Suite**. Счётное ядро для электродинамического расчёта планарных структур в **H-поляризации** (скалярная задача) во временно́й области. Выходной файл: `tmc_rth.exe`.
> Язык документации: русский. Все сигнатуры приведены по исходникам из `src/kernels/PlanarRT_H/` и общим заголовкам из `src/Include/`.

> **Примечание про общий код H и X.** PlanarRT_H и PlanarRT_X — это **две копии одной кодовой базы** (папки `src/kernels/PlanarRT_H/` и `src/kernels/PlanarRT_X/`). Бо́льшая часть файлов в копиях побайтно идентична, но вычислительное ядро **различается по содержимому** (см. `planar_rt_x.md`, §2): в X-копии методы тактов 2/3 добавлены прямо в код, без условной компиляции. Содержательно различаются файлы:
> - `TmcRTHRectNode.cpp/.h` — 4 подшага вместо 2, массивы большего размера;
> - `TmcRTHNodeDiel.cpp/.h` — структура узла без `rUo`, методы тактов 2/3, блоки `*_STAT_Y`;
> - `PlanRT_H.cpp` — единственное отличие: `SetRegistryKey("PlanarRT_H")` ↔ `SetRegistryKey("PlanarRT_X")` (раздел реестра настроек).
>
> Остальные файлы (`PlanRT_HView.cpp`, `TmcDialogStatistics.cpp`) различаются только пробельными символами. Данная документация описывает H-вариант (2 подтакта на цикл 1T).

---

## 1. Назначение программы

**PlanarRT_H** (`tmc_rth.exe`) — счётное ядро пакета TMC Suite, выполняющее электродинамический расчёт планарных (двумерных) структур в **поляризации H** (скалярная задача — одна компонента поля H_z).

Ядро решает задачу распространения электромагнитных волн в двумерном пространстве с произвольным распределением диэлектрической проницаемости (ε), магнитной проницаемости (μ) и проводимости (σ) на основе уравнений Максвелла в конечно-разностной формулировке во временно́й области.

Программа построена по схеме **MFC Document/View** (MDI). Каждый документ — это `.tpl`-шаблон (template), описывающий топологию расчётной области, параметры расчёта и выходные данные. Программа умеет:

- читать `.tpl`-шаблоны (с препроцессингом через библиотеку PREPR);
- строить расчётную сетку (nX × nY узлов) и присваивать каждому узлу диэлектрические параметры;
- выполнять пошаговый или полный расчёт (Run Step / Run All) в отдельном рабочем потоке;
- записывать выходные данные: временно́е описание сигнала (`.t`), распределение поля (`.ex`), топологию (`.tt`), S-матрицу (`.s`);
- экспортировать данные для расчёта диаграммы направленности (`ExportToDirectionalPatter`, реализация — `CFieldIntegrated` библиотеки TMCIndan);
- воспроизводить звуковые эффекты при завершении шага/расчёта (настраиваемые мелодии);
- запускать внешние вьюверы (TMCROS — сигнал, TMCGROUT — S-матрица, FieldView — поле, TMC_DN — диаграмма направленности) и внешний редактор.

**H-поляризация** — скалярная задача: на каждом узле сетки хранится одно значение поля. Алгоритм использует **2 подтакта** на каждый временно́й цикл 1T (в X-моде — 4, см. `planar_rt_x.md`).

---

## 2. Состав проекта

Файл проекта: `PlanRT_H.vcxproj` (имя проекта `PlanRT_H`, цель `<TargetName>tmc_rth</TargetName>`).

| Файл | Класс / содержимое | Назначение (кратко) |
|------|--------------------|---------------------|
| `PlanRT_H.cpp/.h` | `CPlanRT_HApp` | Класс приложения, `InitInstance`, шаблон документа, `SetRegistryKey("PlanarRT_H")` |
| `MainFrm.cpp/.h` | `CMainFrame` | Главное MDI-окно: тулбар, статусбар |
| `ChildFrm.cpp/.h` | `CChildFrame` | Дочернее MDI-окно документа |
| `PlanRT_HDoc.cpp/.h` | `CPlanRT_HDoc` | Документ: управление расчётом, запуск внешних программ |
| `PlanRT_HView.cpp/.h` | `CPlanRT_HView` | Вид: обработчики меню, отрисовка (фон), конфигурация |
| **`TmcRTHRectNode.cpp/.h`** | `CTmcRTHRectNode` | **Основное вычислительное ядро:** сетка, шаг расчёта, рассеяние, возбуждение, S-матрица |
| **`TmcRTHNodeDiel.cpp/.h`** | `CTmcRTHNodeDiel` | **Обработчик блоков топологии:** диэлектрики, металл, поглотители, входы; возбуждение входов |
| `TmcRTH_DialogBlock.cpp/.h` | `CTmcRTH_DialogBlock` | Диалог информации о блоке |
| `TmcRTH_DialogFormatOutFile.cpp/.h` | `CTmcRTH_DialogFormatOutFile` | Диалог формата выходных данных |
| `TmcDialogStatistics.cpp/.h` | `CTmcDialogStatistics` | Диалог статистики |
| `TmcSoundEffProp.cpp/.h` | `CTmcSoundEffProp` | Свойства звуковых эффектов (PropertySheet) |
| `TmcSoundMel1.cpp/.h` | `CTmcSoundMel1` | Страница мелодии |
| `Pl_iofor.cpp/.h` | — | Глобальные функции: форматы вывода, флаги командной строки, авто/демо/пакетный режимы |
| `PL_GLFUN.CPP` | — | Глобальные функции: `PutTrace`, `PutStatistics`, звук, точки входа потоков |
| `StdAfx.cpp/.h` | — | Прекомпилированный заголовок MFC |
| `resource.h`, `PlanRT_H.rc` | — | Ресурсы |

**Кросс-проектные файлы** (компилируются как часть проекта, подтверждено в `.vcxproj`):

| Файл | Назначение |
|------|-----------|
| `../../viewers/Tmcrtout/TmcSMatrix.cpp` | Восстановление S-матрицы из временно́го сигнала |
| `../../viewers/Tmcrtout/TmcTtoS.cpp` | Обёртка «время → S-матрица» |

**Зависимости** (`AdditionalDependencies`): `sfile95.lib`, `prepr.lib`, `exprint.lib`, `TMCIndan.lib`, `TMCLibError.lib`.

**Препроцессорные определения:** `WIN32;_WINDOWS;DEBUG__1` (плюс `_DEBUG`/`NDEBUG` по конфигурации). Дополнительных макросов X-моды у H-проекта **нет**.

**Общие заголовки из `src/Include/`** (перечислены в `.vcxproj` как `ClInclude`, но физически находятся в общем каталоге и документируются отдельно):

| Запись `ClInclude` | Где находится | Назначение |
|--------------------|---------------|-----------|
| `TmcRTH_BolckList.h` | `src/Include/TmcRTH_BolckList.h` | Описание списка блоков топологии — общий заголовок библиотеки **TMCIndan** (класс `CTmcRTH_BlockList`). Локальной копии в каталоге проекта нет; документация — `docs/api/libs/tmcindan.md` |
| `Typerth.h` | `src/Include/Typerth.h` | Общие типы, константы и глобальные функции (см. §3) |

**Файлы вне сборки (legacy).** В каталоге `src/kernels/PlanarRT_H/` физически присутствуют, но **НЕ включены** в `PlanRT_H.vcxproj` (нет ни `ClCompile`, ни `ClInclude`):

| Файл | Класс | Статус |
|------|-------|--------|
| `TmcErrorMessage.cpp/.h` | `CTmcErrorMessage` | Диалог «О программе» / сообщений об ошибках. В сборку не входит, в исходном коде ядра не подключается (`#include`); упоминается только в файле ClassWizard `PlanRT_H.clw`. Считать неиспользуемым кодом. → ⚠️ см. §12, п. 11 |

---

## 3. Ключевые типы и константы (заголовок `src/Include/Typerth.h`)

### 3.1. Точность вычислений

```cpp
//#define _PREC_DOUBLE          // выключено
#define _PREC_FLOAT             // включено: точность float

#ifdef _PREC_FLOAT
#define _real float             // базовый тип точности
#endif

#define _ELEM_VAL_RTH_REAL      // включено: элемент поля — вещественный
//#define _ELEM_VAL_RTH_INT     // выключено: целочисленный вариант (с множителем 1024*1024)

#ifdef _ELEM_VAL_RTH_REAL
#define _ELEM_VAL_RTH _real     // тип элемента поля = float
#define _ELEM_VAL_RTH_MULT 1
#endif
```

Также в `Typerth.h` выбран вариант сборки `_VERSION_PROF___` (закомментированы `_VERSION_DEMO___` и `_VERSION_EDUC___`).

### 3.2. Физические константы

| Константа | Значение | Смысл |
|-----------|----------|-------|
| `C0___` | 299792458 | Скорость света (м/с) |
| `MU0___` | 12.566370614e-7 | Магнитная постоянная (Гн/м) |
| `EPS0___` | 8.854187817e-12 | Электрическая постоянная (Ф/м) |
| `PI___` | 3.141592653589 | Число π |
| `ELECTRON_Q___` | 1.602176565e-19 | Заряд электрона (Кл) — используется только X-ядром |
| `ELECTRON_M___` | 9.10938356e-31 | Масса электрона (кг) — используется только X-ядром |

> ⚠️ `ELECTRON_Q___` и `ELECTRON_M___` — это **константы**, а не переключатели режима. Подробнее об их роли в X-моде и о макросах проекта X — `planar_rt_x.md`, §2.

### 3.3. Типы узлов (`nType`)

Значения поля `nType` структуры узла и блока (совпадают со значениями `nType` класса `CTmcRTH_Input` библиотеки TMCIndan):

| Значение | Смысл |
|----------|-------|
| `0` | Диэлектрик, ε < 0 |
| `1` | Диэлектрик, ε > 0 |
| `-1` | Металл |
| `-2` | Поглотитель |
| `-3` | Вход X слева (направление >) |
| `-4` | Вход X справа (направление <) |
| `-5` | Вход Y сверху (направление V) |
| `-6` | Вход Y снизу (направление ^) |

### 3.4. Глобальные функции `Typerth.h` (реализация — `PL_GLFUN.CPP`)

| Функция | Назначение |
|---------|-----------|
| `void PutTrace(CString)` / `void PutTrace(char*)` | Вывод строки трассировки в панель 0 статусбара главного окна |
| `void PutStatistics(CString)` / `void PutStatistics(char*)` | Вывод статистики в статусбар |
| `void PutSinchronizFlag(BOOL bTopologyFlag, BOOL bFieldFlag1, BOOL bSinchFlag1)` | Индикация в статусбаре состава вывода: «Output: Signal [+ Topology] [+ Field] [+ Sinchronization]» |
| `void PutModel(void)` / `CString GetModel(void)` | Строка модели/версии |
| `void ReadSystemType(void)` | Чтение типа системы |
| `void BeepStepEnd(int i)` / `void BeepAllEnd(void)` | Звуковое уведомление о завершении шага / всего расчёта |
| `void s_alarm(int)` / `void s_play(int far*)` / `void s_tone(int freq, int time)` | Низкоуровневое воспроизведение звука (остатки win16-кода: `far`) |
| `Set/GetMelody1Interval`, `Set/GetMelody1`, `Set/GetMelody2Interval`, `Set/GetMelody2` | Настройка двух мелодий уведомлений |
| `SetAutoStartRunOn/Off()`, `SwitchAutoStartRunFlag()`, `BOOL IsAutoStartRun()` | Автозапуск расчёта при открытии документа |
| `void SetDeltaT(char*)` / `void SetDeltaT(double)` / `double GetDeltaT()` / `BOOL IsDeltaTDefine()` / `void SwitchDeltaT()` | Управление шагом времени Δt (в т.ч. из командной строки) |
| `BOOL IsPointInTr(x1,y1, x2,y2, x3,y3, x0,y0)` | Геометрический предикат: лежит ли точка (x0,y0) внутри треугольника |
| `UINT ReadDataGlobal(LPVOID)` | Точка входа потока чтения данных; `pParam` — указатель на `CPlanRT_HDoc`; вызывает `pDoc->ReadData()` |
| `UINT RunStepGlobal(LPVOID)` | Точка входа потока одного шага; вызывает `pDoc->RunStep()` |
| `UINT RunAllGlobal(LPVOID)` | Точка входа потока полного расчёта; вызывает `pDoc->RunAll()` |
| `UINT RunStepGlobalOneTacts(LPVOID)` | Точка входа потока шага по одному такту |

### 3.5. Глобальные функции `Pl_iofor.h` (реализация — `Pl_iofor.cpp`)

Форматы текстового вывода (по умолчанию заданы константами `FORMAT_OUT_FILE_*`):

| Константа | Значение по умолчанию | Поле |
|-----------|----------------------|------|
| `FORMAT_OUT_FILE_NTFORMAT` | `"%9d"` | Номер такта nT |
| `FORMAT_OUT_FILE_DTCurrentFORMAT` | `"%15.7g"` | Текущее время |
| `FORMAT_OUT_FILE_NBFORMAT` | `"%4d"` | Номер блока |
| `FORMAT_OUT_FILE_DINPFORMAT` | `"%15.7g"` | Входной сигнал |
| `FORMAT_OUT_FILE_DOUTFORMAT` | `"%15.7g"` | Выходной сигнал |
| `FORMAT_OUT_FILE_DFLDOUTFORMAT` | `""` | Поле (пусто — бинарный вывод) |

| Функция | Назначение |
|---------|-----------|
| `SetnTFormat/GetnTFormat`, `SetdTcurrentFormat/GetdTcurrentFormat`, `SetnBFormat/GetnBFormat`, `SetdInpFormat/GetdInpFormat`, `SetdOutFormat/GetdOutFormat`, `SetdFieldOutFormat/GetdFieldOutFormat` | Установка/чтение printf-форматов выходных файлов |
| `void SetDefaultOutputFormat(void)` | Сброс всех форматов к значениям по умолчанию |
| `SetAutoRunOn/Off()`, `SwitchAutoRunFlag()`, `BOOL IsAutoRun()` | Авто-режим |
| `SetDemoRunOn/Off()`, `BOOL IsDemoRun()` | Демо-режим |
| `SetBatchRunOn/Off()`, `SwitchBatchRunFlag()`, `BOOL IsBatchRun()` | Пакетный (batch) режим |
| `void Set_CommandLine_Flags(char *lpszCmdLine)` | Разбор флагов командной строки |
| `CMainFrame *GetMainFramePointer__()` / `SetMainFramePointer__(CMainFrame*)` | Доступ к главному окну из глобального кода |
| `void SetProcessInformation(PROCESS_INFORMATION)` / `PROCESS_INFORMATION GetProcessInformation()` | Данные запущенного внешнего процесса (вьювера) |
| `void SetPointerClassView(CPlanRT_HView*)` | Регистрация указателя на вид |
| `void RunOnViewField1(void)` | Вызов команды «View Field 1» извне |

---

## 4. Структуры данных узлов (`TmcRTHNodeDiel.h`)

### 4.1. `sTmcRTHNodeDielOne` — узел (H-поляризация, 2 такта)

```cpp
struct STMCRTH_NODEDIELONE
{
    _ELEM_VAL_RTH rU;       // падающее напряжение на диэлектрике (такт 0)
    _ELEM_VAL_RTH rU_Cur;   // текущее напряжение
    _ELEM_VAL_RTH rU_Cur1;  // напряжение такта «текущий+1»
    _ELEM_VAL_RTH rUo;      // рассеянное напряжение (для входов)
    _real rY;               // проводимость узла
    int   nNodeGlobal;      // номер узла в глобальной топологии
    int   nType;            // тип узла (см. §3.3)
};
typedef struct STMCRTH_NODEDIELONE sTmcRTHNodeDielOne;
```

> В H-варианте — **4 компоненты напряжения** (`rU`, `rU_Cur`, `rU_Cur1`, `rUo`). В X-варианте — 8 (`rU..rU3`, `rU_Cur..rU_Cur3`), а поле `rUo` **отсутствует** (см. `planar_rt_x.md`, §3.1).

### 4.2. `sTmcRTH_DielNodeList` — односвязный список узлов

```cpp
struct STMCRTH_DIELNODELIST
{
    STMCRTH_DIELNODELIST * pcDielNodeNextList;  // следующий элемент списка
    sTmcRTHNodeDielOne   * pcNodeDielOne;       // узел
};
typedef struct STMCRTH_DIELNODELIST sTmcRTH_DielNodeList;
```

---

## 5. Класс `CTmcRTHRectNode` — вычислительное ядро

**Назначение:** управление расчётной сеткой, выполнение шагов расчёта во временно́й области, рассеяние, возбуждение входов, запись выходных файлов и расчёт S-матрицы.
**Заголовок:** `TmcRTHRectNode.h` · **Базовый класс:** `CWnd`
**Зависит от:** `Typerth.h`, `TmcRTHNodeDiel.h`, `TmcLibError.h`, `TmcRTH_Indan.h`, `TmcRTH_IndanParam.h`

### 5.1. Поля (private)

| Поле | Тип | Назначение |
|------|-----|-----------|
| `cIndan` | `CTmcRTH_Indan` | Менеджер входных данных (.tpl) — библиотека TMCIndan |
| `cParam` | `CTmcRTH_IndanParam` | Параметры расчёта (секция PARAM) |
| `cError` | `CTmcLibError` | Накопитель ошибок |
| `pcNodeDiel` | `CTmcRTHNodeDiel*` | Массив обработчиков блоков (по одному на блок топологии) |
| `nDiel` | `int` | Число блоков |
| `nTCurrent` | `int` | Текущий номер такта времени |
| `nTMax` | `int` | Максимальный такт (конец расчёта) |
| `dT` | `double` | Шаг времени Δt |
| `dTCurrent` | `double` | Текущее физическое время |
| `nX` / `nY` | `int` | Число узлов сетки по X / Y |
| `dDelta` | `double` | Пространственный шаг (Δ = Δx = Δy) |
| `dXmin` / `dXmax` / `dYmin` / `dYmax` | `double` | Границы расчётного прямоугольника |
| `nNumNode` | `int` | Общее число узлов = nX × nY |
| `prUNode1Temp` | `_ELEM_VAL_RTH*` | «Сырой» указатель массива напряжений (хранится для `delete[]`) |
| `prUNode1` | `_ELEM_VAL_RTH*` | Рабочий указатель массива напряжений, **выровнен на 32 байта** |
| `prYNodeTemp` | `_real*` | «Сырой» указатель массива проводимостей |
| `prYNode` | `_real*` | Рабочий указатель массива проводимостей, выровнен на 32 байта |
| `bIsReadData` | `volatile BOOL` | Флаг «данные загружены» (volatile — общение между потоками) |
| `bIsRunStep1Run` | `volatile BOOL` | Флаг «шаг выполняется» |

**Выделение массивов** (`InitNodeArray`): запрашивается `2*nNumNode + 32` элементов (по 2 значения на узел плюс запас), после чего рабочий указатель смещается так, чтобы адрес был кратен 32 — выравнивание для эффективного доступа. В X-варианте — `6*nNumNode + 32` (по 6 значений на узел).

### 5.2. Публичные методы

| Метод | Назначение |
|-------|-----------|
| `CTmcRTHRectNode()` / `virtual ~CTmcRTHRectNode()` | Конструктор / деструктор |
| `void ReadData(CString csFileName, int nStep1)` | Чтение `.tpl`-файла `csFileName`, установка номера шага `nStep1`, инициализация расчёта |
| `void ReadData(void)` | Перечитывание текущего файла |
| `void RunStep(void)` | Выполнение шага расчёта (до `nTMax`) |
| `void Stop(void)` | Остановка расчёта (сброс `bIsReadData`) |
| `BOOL IsReadData(void)` | TRUE, если данные загружены |
| `BOOL IsRunStep(void)` | TRUE, если шаг выполняется |
| `BOOL IsError(void)` | TRUE при накопленной ошибке (`cError.IsError()`) |
| `CString GetErrorMessage(void)` | Текст ошибки |
| `int GetNStepMax(CString csFileName)` | Максимальное число шагов для данного файла |
| `CTmcRTH_IndanOutput* GetOutput(void)` | Доступ к подсистеме выходных данных |
| `CTmcRTH_IndanParam& GetParam(void)` | Доступ к параметрам |
| `CTmcRTH_IndanTopology& GetTopology(void)` | Доступ к топологии |
| `void ExportToDirectionalPatter(void)` | Экспорт для диаграммы направленности: `cIndan.GetOutput()->GetFieldIntegrated().ExportToDirectionalPattern(cParam)` |
| `BOOL GetTopologyFlag(void)` | Флаг вывода топологии |
| `BOOL GetSoundEffect(void)` | Флаг звуковых эффектов |
| `BOOL GetFieldFlag(void)` | Флаг вывода поля |
| `BOOL GetSinchronizationFieldOutput(void)` | Флаг синхронного вывода поля |
| `void PutTopol(void)` | Запись файла топологии |
| `void OnOffSoundEffects(void)` | Переключение звуковых эффектов |
| `void OnOffSinchronizationFieldOutput(void)` | Переключение синхронизации вывода поля |
| `void OnOffFieldOutput(void)` | Переключение вывода поля |

> Примечание к названию: `ExportToDirectionalPatter` — так в коде (без конечной `n`); внутренняя функция TMCIndan называется `ExportToDirectionalPattern`.

### 5.3. Приватные методы

**Чтение и инициализация:**

| Метод | Назначение |
|-------|-----------|
| `void ReadTopologySection(void)` | Чтение секции TOPOLOGY |
| `void ReadParamSection(void)` | Чтение секции PARAM |
| `void ReadLinkListSection(void)` | Чтение секции LINK_LIST |
| `void ReadOutputSection(void)` | Чтение секции OUTPUT |
| `void SetStepNumber(int nStep)` | Установка номера шага |
| `void InitKernel(void)` | Инициализация ядра перед расчётом |
| `void InitNodeArray(void)` / `void DeleteNodeArray(void)` | Выделение/освобождение массивов узлов (с выравниванием, см. §5.1) |
| `void InitDielArray(void)` / `void DeleteDielArray(void)` | Выделение/освобождение массива блоков `pcNodeDiel` |
| `void SetDielTopology(void)` | Присвоение узлам параметров по блокам |
| `void SetDielInNodeList(void)` | Включение узлов блоков в общий список |
| `void DeleteDielList(sTmcRTH_DielNodeList **pcNodeDielList1)` | Освобождение списка узлов |
| `BOOL IsDataInit(void)` | Проверка инициализации данных |
| `void DeleteData(void)` | Полное освобождение данных |

**Расчётный цикл:**

| Метод | Назначение |
|-------|-----------|
| `void RunKernel(void)` | Главный цикл: `for(; nTCurrent < nTMax; ...) RunKernel1T();` затем `PutSmatrix()`. Каждые 100 тактов выводит прогресс и затраченное время через `PutTrace`/`PutStatistics` |
| `void RunKernel1T(void)` | Один цикл 1T = **2 подтакта** (см. §5.4) |
| `void RunStep1(void)` | Выполнение одного шага (обёртка с установкой флагов) |
| `void RunExciteInputs(void)` / `void RunExciteInputs1(void)` | Возбуждение входов (подтакты 0 и 1) |
| `void RunScatteringNode(void)` / `void RunScatteringNode1(void)` | Рассеяние на узлах сетки (подтакты 0 и 1) |
| `near void RunScatteringNode1line(_ELEM_VAL_RTH *pr_111, _real *prY_111, int nx_111, int nX_111)` | Рассеяние по одной строке сетки (подтакт 0). `pr_111` — массив напряжений строки, `prY_111` — проводимости, `nx_111` — индекс строки, `nX_111` — длина строки. Квалификатор `near` — остаток win16 |
| `near void RunScatteringNode1line1(...)` | То же для подтакта 1 (параметры идентичны) |
| `void RunBlockNode(void)` / `void RunBlockNode1(void)` | Обработка блоков: вызов `CTmcRTHNodeDiel::RunBlockNode/RunBlockNode1` для каждого блока (подтакты 0 и 1) |

**Вывод:**

| Метод | Назначение |
|-------|-----------|
| `void PutField(int ii)` | Запись распределения поля подтакта `ii` (0 или 1) |
| `void PutSmatrix(void)` | Расчёт и запись S-матрицы: `cIndan.GetOutput()->PutSmatrix()` |
| `void PutTopology(void)` | Запись файла топологии (`.tt`) |
| `void DistributionIntegrated(int ii)` | Накопление интегрированного распределения поля (для диаграммы направленности) |
| `void OutputnT(void)` | Вывод номера текущего такта |
| `void DeleteOutputFile(void)` | Удаление выходных файлов перед перезапуском |
| `BOOL IsFieldFileRead(void)` | Проверка чтения файла поля |
| `void PutStepEndSound(void)` | Звук завершения шага |

### 5.4. Алгоритм одного цикла 1T (H-поляризация, 2 подтакта)

Точная последовательность из `RunKernel1T()` (`TmcRTHRectNode.cpp:537`):

```
Подтакт 0:
  RunExciteInputs();          // возбуждение входов
  RunScatteringNode();        // рассеяние на узлах
  RunBlockNode();             // обработка блоков
  PutField( 0 );              // запись поля
  DistributionIntegrated(0);  // накопление для ДН
  nTCurrent++;  dTCurrent += dT;

Подтакт 1:
  RunExciteInputs1();
  RunScatteringNode1();
  RunBlockNode1();
  PutField( 1 );
  DistributionIntegrated(1);
  nTCurrent++;  dTCurrent += dT;
```

Счётчик `nTCurrent` увеличивается **внутри каждого подтакта** на 1 (итого +2 за цикл 1T). По завершении главного цикла `RunKernel` вызывает `PutSmatrix()`.

> В X-моде: 4 подтакта (+4 за цикл), вызовы `DistributionIntegrated` закомментированы (см. `planar_rt_x.md`, §4).

---

## 6. Класс `CTmcRTHNodeDiel` — блоки топологии

**Назначение:** представление одного блока топологии (прямоугольник, круг, полигон; диэлектрик, металл, магнетик, поглотитель, вход) — список его узлов, проводимости, возбуждение входов.
**Заголовок:** `TmcRTHNodeDiel.h` · **Базовый класс:** `CWnd`

### 6.1. Поля (private)

| Поле | Тип | Назначение |
|------|-----|-----------|
| `cError` | `CTmcLibError` | Ошибки блока |
| `nBlock` | `int` | Номер блока |
| `csBlock` | `CString` | Тип блока (ключевое слово из `.tpl`: `RECT`, `CIRCLE`, `POLYGON`, …) |
| `nNumNode` | `int` | Число узлов в блоке |
| `pcNodeDielOne` | `sTmcRTHNodeDielOne*` | Массив узлов блока |
| `dXCenter` / `dYCenter` | `double` | Координаты центра блока |
| `dYInput` | `double` | Проводимость входного блока |
| `dTmin` / `dTmax` | `double` | Диапазон времени возбуждения входа |
| `csEpsExpr` | `CString` | Выражение для ε (вычисляется библиотекой exprint) |
| `csW` | `CString` | Угловая скорость (вращающийся блок) |
| `csVx` / `csVy` | `CString` | Линейные скорости центра вращения |
| `nX0` / `nY0` | `int` | Центр вращения (x0, y0) |
| `nTxCurrent` / `nTyCurrent` | `int` | Счётчики движения блока по X / Y |
| `nTxMove` / `nTyMove` | `int` | Периоды движения блока по X / Y |
| `nType` | `int` | Тип блока (см. §3.3) |
| `dY1_InputAdmitance` | `double` | Проводимость входа (вспомогательная) |
| `dWidthWaveg` | `double` | Ширина волновода (для входа) |
| `dUnormir` | `double` | Нормировка напряжения возбуждения |
| `csFileNameEps` | `CString` | Имя `.eps`-файла (блок из файла) |
| `dX0` / `dY0` | `double` | Координаты (вспомогательные) |
| `pbIsStop` | `volatile BOOL*` | Указатель на флаг остановки (из ядра) |

### 6.2. Публичные методы

| Метод | Назначение |
|-------|-----------|
| `CTmcLibError& SetBlock(CTmcRTH_BlockList *pcNextBlockList, CTmcRTH_IndanParam &cParam, CTmcRTH_IndanOutput *cOut, _real *prYNode, volatile BOOL *pbStopFlag, CString csCurrentPath)` | Инициализация блока по описанию из `.tpl` (`CTmcRTH_BlockList` — TMCIndan): определяет тип, строит список узлов, заполняет проводимости `prYNode`. `csCurrentPath` — путь для поиска `.eps`-файлов |
| `CTmcLibError& SetBorderXmin(int nX, int nY)` / `SetBorderXmax(int nX, int nY)` / `SetBorderYmax(int nX, int nY)` / `SetBorderYmin(int nX)` | Обработка границ расчётной области |
| `void SetBlockInDielList(sTmcRTH_DielNodeList **pcNodeDielList, int nNumNode1, CTmcLibError &cError1)` | Включение узлов блока в общий список узлов ядра |
| `void ExciteInputs(CTmcLibError &cError1, double dWT, double dT, _ELEM_VAL_RTH *pr1, int nX, CTmcRTH_IndanParam &cParam, double dtCurrent)` | Возбуждение входных узлов, подтакт 0. `dWT` — круговая частота × время, `dT` — шаг времени, `pr1` — массив напряжений, `dtCurrent` — текущее время |
| `void ExciteInputs1(...)` | То же, подтакт 1 (сигнатура идентична) |
| `void RunBlockNode(_ELEM_VAL_RTH *prUNode1, _real *prYNode, int nX, int nArraySize)` | Обновление напряжений узлов блока, подтакт 0 |
| `void RunBlockNode1(...)` | То же, подтакт 1 |
| `void AddCurrentPath(CString csCurrentPath)` | Задание текущего пути (поиск `.eps`) |
| `CTmcLibError& GetError(void)` | Доступ к ошибкам |
| `void DeleteData(void)` | Освобождение данных блока |

> В X-варианте дополнительно: `ExciteInputs2/3`, `RunBlockNode2/3` (см. `planar_rt_x.md`, §5).

### 6.3. Приватные методы (по типам блоков)

| Метод | Назначение |
|-------|-----------|
| `SetRect()` / `SetCirc()` / `SetPoly()` | Диэлектрический блок (прямоугольник / круг / полигон) |
| `SetRectMetal()` / `SetCircMetal()` / `SetPolyMetal()` | Металлический блок |
| `SetRectMagnetic()` / `SetCircMagnetic()` / `SetPolyMagnetic()` | Магнитный блок |
| `SetRectAbsorber()` / `SetCircAbsorber()` / `SetPolyAbsorber()` | Поглотитель |
| `SetRectFile()` | Блок с распределением ε из `.eps`-файла |
| `SetInpXLeft()` / `SetInpXRight()` / `SetInpYTop()` / `SetInpYBot()` | Входные блоки (4 направления) |

Все методы `Set*` принимают `CTmcRTH_BlockList *pcNextBlockList` (описание блока) и `CTmcRTH_IndanParam &cParam` (параметры), возвращают `CTmcLibError&`; `SetRect`/`SetRectFile` дополнительно принимают `CString csCurrentPath`, методы входов — `CTmcRTH_IndanOutput *cOut`.

**Вспомогательные приватные методы:**

| Метод | Назначение |
|-------|-----------|
| `double SetInputAdmitance(double dFreq, double dDelta)` | Расчёт проводимости входа |
| `double SetAbsorberAdmitance(double dFreq, double dDelta)` | Расчёт проводимости поглотителя |
| `void CalculateYForInput(double dFreq, double dDelta)` | Проводимость входных узлов |
| `void SetYForInput(CTmcRTH_IndanParam&)` / `void SetYLineForDiel(CTmcRTH_IndanParam&, _real *prYNode)` | Заполнение массивов проводимостей |
| `_real CalcEps(double x, double y, CTmcRTH_IndanParam &cParam)` | Вычисление ε в точке (x, y) по выражению `csEpsExpr` |
| `double dFaza(CTmcRTH_IndanParam&, double dtCurrent)` / `double dAmplitude(...)` | Фаза и амплитуда сигнала возбуждения в момент времени `dtCurrent` |
| `void SetnType(void)` | Установка типа узлов блока |
| `void SetAbsorberNodeType(CTmcRTH_IndanParam&)` / `void SetMagneticPolygonType(CTmcRTH_IndanParam&)` | Типы узлов поглотителя / магнитного полигона |
| `void CalculatedXYCenter(CTmcRTH_IndanParam&)` | Вычисление центра блока |
| `void AddList(sTmcRTHNodeDielOne*, sTmcRTH_DielNodeList**)` | Добавление узла в список |
| `void OutputUpUo(double dUp, double dUo, CTmcLibError&)` | Вывод падающего/рассеянного напряжений |
| `double atan2__1(double y, double x)` | Вариант atan2 |
| `void SeachEpsFileName(CString)` / `void AddFileEpsExtention(void)` / `BOOL IsFullName(void)` | Поиск и нормализация имени `.eps`-файла |
| `void SaveFileRect(...)` | Сохранение прямоугольника в файл |
| `void DeleteDublicateData(void)` / `void DeleteStatDielData(void)` | Очистка дублирующихся/статических данных |

### 6.4. Формат `.eps`-файла (распределение ε)

Константы формата определены в начале `TmcRTHNodeDiel.h`:

```cpp
#define TMC_RTH_EPSFILE_EXT_   "eps"
#define TMC_RTH_EPSFILE_ID_    "#TamicRTH_planar_DistributionDielectricPermeability_File_V2.00 2000"
#define TMC_RTH_EPSFILE_TYPE_  "#TopologyPrimitiv "
#define TMC_RTH_EPSFILE_Delta_ "#dDelta "
#define TMC_RTH_EPSFILE_XMIN_  "#Xmin "
#define TMC_RTH_EPSFILE_YMIN_  "#Ymin "
#define TMC_RTH_EPSFILE_NX_    "#nX "
#define TMC_RTH_EPSFILE_NY_    "#nY "
#define TMC_RTH_EPSFILE_NPoint_ "#nPoint "
#define TMC_RTH_EPSFILE_NAccur_ "#nAccuracy "
#define TMC_RTH_EPSFILE_NdFrmt_ "#sNodeFormat "
#define TMC_RTH_EPSFILE_VlFrmt_ "#sValueFormat "
```

Файл начинается с идентификатора `#TamicRTH_planar_...`, далее — параметры сетки (`#dDelta`, `#Xmin`, `#Ymin`, `#nX`, `#nY`, число точек, точность, форматы узла/значения), затем данные.

**Точная раскладка.** Заголовок — текстовый, каждая строка завершается парой байт `13, 10` (CR LF).
**Пробел после метки значим.** Порядок и смысл полей (сверено с авторским описанием
`Tamic\TMC\DISTR\MANUALS\RT_HPLANAR\FormEps.doc`, 1999, и с кодом `TmcRTHNodeDiel.cpp:6811` —
запись, `:7227` — чтение):

| Метка | Пример значения | Смысл |
|-------|-----------------|-------|
| `#TamicRTH_planar_DistributionDielectricPermeability_File_V2.00 2000` | — | Метка файла. ⚠️ В описании 1999 г. — версия `V1.00`; в коде `V2.00 2000` |
| `#TopologyPrimitiv ` | `RECT_STAT` | Тип топологического примитива |
| `#dDelta ` | `0.0005` | Пространственный дискрет, **в метрах** |
| `#Xmin ` | `0` | Абсцисса самой левой точки, в метрах |
| `#Ymin ` | `0` | Ордината самой нижней точки, в метрах |
| `#nX ` | `33` | Число точек по оси X |
| `#nY ` | `13` | Число точек по оси Y |
| `#nPoint ` | `429` | Число точек в примитиве (обычно `nX * nY`) |
| `#nAccuracy ` | `8` | Число байт в значении: **8** — двойная точность, **4** — одинарная. Пишется как `sizeof(_real)` |
| `#sNodeFormat NULL` | — | Зарезервированная строка |
| `#sValueFormat NULL` | — | Зарезервированная строка |

Далее — **бинарный массив** из `#nPoint` записей. Каждая запись:

| Поле | Размер | Содержимое |
|------|--------|-----------|
| значение | `#nAccuracy` (8 или 4) | взвешенное значение (`_real`) |
| номер узла | 4 (`int`) | глобальный номер узла в примитиве |

Итого размер массива — `#nPoint * (#nAccuracy + 4)` байт: 12 байт на точку для двойной точности,
8 — для одинарной.

> ⚠️ **Записывается не «чистая» ε, а взвешенное значение.** В `TmcRTHNodeDiel.cpp:6837–6847` поле
> `rY` инициализируется единицей, затем **делится пополам на каждой границе примитива** (`i == 0`,
> `i == nY-1`, `j == 0`, `j == nX-1` — то есть у рёбер коэффициент 0.5, в углах 0.25) и лишь потом
> умножается на `CalcEps(x, y)`. Не интерпретируйте содержимое как ε напрямую.

> 📌 **Совместимость float ↔ double.** Читатель сверяет `#nAccuracy` с `sizeof(_real)` и при
> несовпадении конвертирует (`TmcRTHNodeDiel.cpp:7354` и ветка `case 4:` — чтение во `float` с
> расширением до `_real`). Поэтому `.eps`, созданные старой одинарной сборкой, **читаются** текущей
> двойной. Допустимы только значения 4 и 8 — иначе ошибка `nAccuracy = %d != 4 and != 8`.

---

## 7. Класс `CPlanRT_HDoc` — документ

**Назначение:** управление расчётом (через `cRectNode`), запуск внешних программ, хранение настроек.
**Заголовок:** `PlanRT_HDoc.h` · **Базовый класс:** `CDocument`

### 7.1. Поля (private)

| Поле | Тип | Назначение |
|------|-----|-----------|
| `cRectNode` | `CTmcRTHRectNode` | Вычислительный объект |
| `nStep` | `int` | Текущий шаг |
| `bIsReadData` | `volatile BOOL` | Флаг загрузки данных |
| `csEditorName` | `CString` | Внешний текстовый редактор (.tpl) |
| `csExternViewer` | `CString` | Вьювер сигналов (TMCROS) |
| `csExternViewerField` | `CString` | Вьювер полей (FieldView) |
| `csExternViewerSmatrix` | `CString` | Вьювер S-матриц (TMCGROUT) |
| `csExternViewerDirectPattern` | `CString` | Вьювер диаграмм направленности (TMC_DN) |

### 7.2. Настройки в реестре

Раздел задаётся в `InitInstance`: `SetRegistryKey("PlanarRT_H")`. Имена параметров — константы в `PlanRT_HDoc.h`:

| Константа | Имя параметра | Содержимое |
|-----------|---------------|-----------|
| `PLANRT_H_RAZDEL_INI` | `PlanRT_H Config` | Имя секции |
| `PLANRT_H_EXTERNEDITORNAME_INI` | `ExternEditorName` | Путь к редактору |
| `PLANRT_H_EXTERNVIEWERNAME_INI` | `ExternViewerName` | Вьювер сигналов |
| `PLANRT_H_EXTERNFIELDVIEWERNAME_INI` | `ExternFieldViewerName` | Вьювер полей |
| `PLANRT_H_EXTERNDIRPATVIEWERNAM_INI` | `ExternDirectPatViewerName` | Вьювер ДН |
| `PLANRT_H_EXTERNSMVIEWERNAME_INI` | `ExternSmatrixViewerName` | Вьювер S-матриц |
| `PLANRT_H_BACKGROUNDCOLOR_INI` | `BackGroundColor` | Цвет фона |
| `PLANRT_H_OUTFORMATNT_INI` … `PLANRT_H_OUTFORMATOUT_INI` | `OutputFormatnT/dT/nB/dInp/dOut` | Форматы вывода (§3.5) |
| `PLANRT_H_OUTFORMATOUTFILED_INI` | `OutputFormatdOutField` | Формат вывода поля |
| `PLANRT_H_OUTPUTSOUNDFLAG_INI` | `SoundEffectsFlag` | Флаг звука |
| `PLANRT_H_OUTFIELDSINCHRFLAG_INI` | `OutputSinchrFlagdOutField` | Синхронный вывод поля |
| `PLANRT_H_OUTFIELDFLAG_INI` | `OutputFlagdOutField` | Флаг вывода поля |
| `PLANRT_H_MNWNDSIZEPLACE_INI` | `MainFrameSizeAndPlace__1` | Положение главного окна |

### 7.3. Методы

**Управление расчётом:**

| Метод | Назначение |
|-------|-----------|
| `void ReadData(void)` | Чтение `.tpl` (делегирует `cRectNode`) |
| `void RunStep(void)` / `void RunAll(void)` | Шаг / полный расчёт |
| `void Stop(void)` | Остановка |
| `BOOL IsRun(void)` / `BOOL IsReadData(void)` | Флаги состояния |
| `void BackStep(void)` / `void SkipStep(void)` / `void SetFirstStep(void)` | Навигация по шагам |
| `int GetnStep(void)` | Текущий шаг |
| `CString GetErrorMessage(void)` | Текст ошибки |
| `CTmcRTHRectNode& GetRectNode(void)` | Прямой доступ к ядру |
| `CTmcRTH_IndanOutput* GetOutput(void)` / `CTmcRTH_IndanParam& GetParam(void)` / `CTmcRTH_IndanTopology& GetTopology(void)` | Доступ к данным ядра |
| `void ExportToDirectionalPatter(void)` | Экспорт для ДН |
| `void CloseAndExit(void)` | Закрытие документа и выход |

**Флаги вывода и звука** (делегируют `cRectNode`): `GetTopologyFlag()`, `GetSoundEffect()`, `GetFieldFlag()`, `GetSinchronizationFieldOutput()`, `PutTopology()`, `SetSoundFlag(int)`, `OnOffSoundEffects()`, `SetOutFieldOutFlag(int)`, `SetOutSinchrFieldOutFlag(int)`, `OnOffSinchronizationFieldOutput()`, `OnOffFieldOutput()`.

**Внешние программы:**

| Метод | Назначение |
|-------|-----------|
| `BOOL RunViewer(void)` | Запуск вьювера сигналов |
| `BOOL RunViewerField(void)` | Запуск вьювера полей |
| `BOOL RunViewerSmatrix(void)` | Запуск вьювера S-матриц |
| `BOOL RunViewerDirectPattern(void)` | Запуск вьювера ДН |
| `Set/GetExternEditorName`, `Set/GetExternViewer`, `Set/GetExternViewerField`, `Set/GetExternViewerSmatrix`, `SetExternViewerDirectionalpattern` / `GetExternViewerDirectPat` | Настройка путей внешних программ |
| `BOOL RunExeFile(CString csNameExe)` / `BOOL RunExeFile(char*)` / `BOOL RunExeFile(CString, CString csArgCommLine)` / `BOOL RunExeFile(char*, char*)` | Запуск внешнего exe (4 перегрузки, с аргументами и без); внутри — `CreatProc(char *lpszComLine)` (private) |

**Переопределения MFC:** `OnNewDocument()`, `Serialize(CArchive&)`, `OnCloseDocument()`, `CanCloseFrame(CFrameWnd*)`.

---

## 8. Класс `CPlanRT_HView` — вид

**Назначение:** обработчики меню/тулбара, отрисовка фона окна документа.
**Заголовок:** `PlanRT_HView.h` · **Базовый класс:** `CScrollView`

### 8.1. Поля (private)

| Поле | Тип | Назначение |
|------|-----|-----------|
| `cSoundDialog` | `CTmcSoundEffProp` | Диалог звуковых эффектов |
| `scBackgoundColor` | `COLORREF` | Цвет фона |
| `bIsReadData` | `volatile BOOL` | Флаг загрузки |

Приватные методы: `OnDrawBackground(CDC*)`, `ReadData()`, `WriteFile()`.

### 8.2. Команды меню (обработчики `afx_msg`)

**Меню Run:**

| Метод | Действие |
|-------|---------|
| `OnRunStartstep` | Один шаг расчёта |
| `OnRunAll` | Полный расчёт |
| `OnRunStop` | Остановка |
| `OnRunRestartstep` / `OnRunRestartall` | Перезапуск шага / всего расчёта |
| `OnRunSkipstep` / `OnRunBackstep` | Пропуск шага / шаг назад |

**Меню View:**

| Метод | Действие |
|-------|---------|
| `OnViewStatistics` | Окно статистики (`CTmcDialogStatistics`) |
| `OnViewOutput` | Выходной сигнал → внешний вьювер (TMCROS) |
| `OnViewField` / `OnViewField1` | Распределение поля → FieldView; также публичный `OnViewField11(void)` для вызова извне |
| `OnViewTopology` | Топология |
| `OnViewDirectionalpattern` | Диаграмма направленности → TMC_DN |

**Меню Config:**

| Метод | Действие |
|-------|---------|
| `OnConfigEditor` | Выбор внешнего редактора |
| `OnConfigViewerOutputsignal` / `OnConfigViewerField` / `OnConfigViewerSmatrix` / `OnConfigViewerDirectionalpattern` | Выбор внешних вьюверов |
| `OnConfigColorBackGround` | Цвет фона |
| `OnConfigFormatOutputDataFile` | Формат выходных данных (`CTmcRTH_DialogFormatOutFile`) |
| `OnConfigSinchronization` | Синхронизация вывода поля |
| `OnConfigSound` / `OnConfigSoundMelody` | Звуковые эффекты / мелодия |
| `OnConfigSmatrix` | Настройка S-матрицы |
| `OnConfigAutorun` | Авто-режим |
| `OnConfigSetup` | Общие настройки |
| `OnConfigDirectionalpattern` | Настройка ДН |

**Прочее:** `OnEditEdit` (открыть `.tpl` в редакторе), `OnFileSaveAs`, `OnClose`, `OnDestroy`, публичный `Stop(void)`.

**Обработчики `OnUpdate*`** (доступность пунктов меню): `OnUpdateConfigSinchronization`, `OnUpdateViewField`, `OnUpdateConfigSound`, `OnUpdateViewTopology`, `OnUpdateViewField1`, `OnUpdateRunRun`, `OnUpdateRunRestartall`, `OnUpdateRunStartstep`, `OnUpdateRunRestartstep`, `OnUpdateRunSkipstep`, `OnUpdateRunBackstep`, `OnUpdateRunStop`, `OnUpdateFileClose`, `OnUpdateConfigAutorun`, `OnUpdateViewDirectionalpattern`.

**Переопределения MFC:** `OnDraw(CDC*)`, `PreCreateWindow`, `DestroyWindow`, `OnInitialUpdate`, `OnPreparePrinting`, `OnBeginPrinting`, `OnEndPrinting`.

---

## 9. Диалоги

В сборку входят пять диалоговых классов (подтверждено по `PlanRT_H.vcxproj`). Класс `CTmcErrorMessage` в сборку **не входит** — см. §2, «Файлы вне сборки».

### 9.1. `CTmcRTH_DialogBlock` — информация о блоке топологии

**Назначение:** просмотр параметров одного блока топологии (тип, координаты вершин, выражение ε, скорости движения) с навигацией «предыдущий/следующий блок».
**Заголовок:** `TmcRTH_DialogBlock.h` · **Базовый класс:** `CDialog` · **Ресурс:** `IDD_TMC_RTH_TOPBLOCK`

**Поля (public):**

| Поле | Тип | Назначение |
|------|-----|-----------|
| `m_pcParam` | `CTmcRTH_IndanParam*` | Параметры расчёта (TMCIndan) |
| `m_pcBlockList` | `CTmcRTH_BlockList*` | Отображаемый блок (TMCIndan) |
| `i` | `int` | Индекс текущего блока |
| `dX[10]` / `dY[10]` | `CString` | Рабочие массивы координат вершин |

**Поля DDX (обмен с элементами диалога):**

| Поле | Тип | Назначение |
|------|-----|-----------|
| `m_nBlock` | `int` | Номер блока |
| `m_nType` | `int` | Тип блока (см. §3.3) |
| `m_nMemory` | `double` | Память, занятая блоком |
| `m_nXY` | `int` | Число узлов блока |
| `m_csString` | `CString` | Текстовое описание блока |
| `m_csVx` / `m_csVy` | `CString` | Линейные скорости центра |
| `m_csW` | `CString` | Угловая скорость вращения |
| `m_csEpsExpr` | `CString` | Выражение для ε |
| `dX0` / `dY0` | `double` | Координаты центра |
| `dX1`…`dX10` / `dY1`…`dY10` | `CString` | Координаты вершин (до 10) |
| `csXminText` / `csXmaxText` / `csYminText` / `csYmaxText` | `CString` | Подписи границ |
| `dXmin` / `dXmax` / `dYmin` / `dYmax` | `double` | Габариты блока |
| `m_csLongUnit` | `CString` | Единица длины |

**Методы:**

| Метод | Назначение |
|-------|-----------|
| `CTmcRTH_DialogBlock(CWnd* pParent = NULL)` | Конструктор |
| `void PrepareData(void)` | Заполнение полей диалога из `m_pcBlockList` и `m_pcParam` |
| `void DoDataExchange(CDataExchange* pDX)` | DDX/DDV |
| `void OnOK()` | Закрытие диалога |
| `afx_msg void OnCancel1()` | Отмена |
| `afx_msg void OnTmcblockbuttonnext()` / `OnTmcblockbuttonprev()` | Переход к следующему / предыдущему блоку |

### 9.2. `CTmcRTH_DialogFormatOutFile` — формат выходных данных

**Назначение:** настройка printf-форматов столбцов выходных текстовых файлов (см. §3.5).
**Заголовок:** `TmcRTH_DialogFormatOutFile.h` · **Базовый класс:** `CDialog` · **Ресурс:** `IDD_TMC_CONFIG_OUTFFORMAT`

**Поля (public):** `BOOL IsSetDefaultFormat` — признак сброса к форматам по умолчанию.

**Поля DDX:**

| Поле | Тип | Формат столбца |
|------|-----|----------------|
| `m_csnT` | `CString` | Номер такта nT |
| `m_csdTcurrent` | `CString` | Текущее время |
| `m_csnBlock` | `CString` | Номер блока |
| `m_csdInp` | `CString` | Входной сигнал |
| `m_csdOut` | `CString` | Выходной сигнал |
| `m_csdOutField` | `CString` | Поле |

**Методы:**

| Метод | Назначение |
|-------|-----------|
| `CTmcRTH_DialogFormatOutFile(CWnd* pParent = NULL)` | Конструктор |
| `void DoDataExchange(CDataExchange* pDX)` | DDX/DDV |
| `afx_msg void OnSetDefaultFormat()` | Сброс всех форматов к значениям по умолчанию (`SetDefaultOutputFormat`, см. §3.5) |

### 9.3. `CTmcDialogStatistics` — статистика расчёта

**Назначение:** сводка по текущему шагу расчёта: число узлов/блоков, память, физические параметры, размеры области в длинах волн, точность.
**Заголовок:** `TmcDialogStatistics.h` · **Базовый класс:** `CDialog` · **Ресурс:** `IDD_TMC_STATISTICS`

**Поля (public):** `CString csAccuracy` (точность); `int nFlagStep` (флаг шага).
**Поля (private):** `CTmcRTH_BlockList * pcBlockList` — блок для окна информации.

**Поля DDX:**

| Поле | Тип | Назначение |
|------|-----|-----------|
| `m_nStep` | `int` | Номер шага |
| `m_nNumNode` | `int` | Число узлов сетки |
| `m_dMemory` | `double` | Занятая память |
| `m_nBlock` | `int` | Число блоков |
| `m_csErrorMessage` | `CString` | Текст ошибки |
| `m_cParam` | `CTmcRTH_IndanParam` | Параметры (TMCIndan) |
| `m_cTopol` | `CTmcRTH_IndanTopology` | Топология (TMCIndan) |
| `m_cOut` | `CTmcRTH_IndanOutput*` | Подсистема вывода (TMCIndan) |
| `dDelta`, `dFreq`, `dt`, `dtT` | `double` | Шаг сетки, частота, шаг времени |
| `dTsizeWL`, `dTsizeWL1`, `dXsizeWL`, `dYsizeWL` | `double` | Размеры области в длинах волн |
| `dXmin`/`dXmax`/`dYmin`/`dYmax`, `dTmin`/`dTmax` | `double` | Границы области и времени |
| `dWaveLen`, `dWaveLenDelt` | `double` | Длина волны и её доля на шаг сетки |
| `csTolerance` | `CString` | Допуск |

**Методы:**

| Метод | Назначение |
|-------|-----------|
| `CTmcDialogStatistics(CWnd* pParent = NULL)` | Конструктор |
| `void PrepareData(void)` | Расчёт и заполнение полей статистики |
| `void DoDataExchange(CDataExchange* pDX)` | DDX/DDV |
| `afx_msg void OnViewBlockInformation()` | Открыть окно информации о блоке (`CTmcRTH_DialogBlock`) |
| `afx_msg void OnStepPrev()` / `OnStepNext()` | Переход к предыдущему / следующему шагу |
| `afx_msg void OnCancel1()` | Закрытие |
| `void DeleteData(void)` *(private)* | Освобождение данных |

> ⚠️ Известный баг: **кнопка статистики не работает** в новых версиях H и X (окно не вызывается, приложение закрывается, при компиляции — предупреждения). См. `CHANGELOG.md`.

### 9.4. `CTmcSoundEffProp` — свойства звуковых эффектов

**Назначение:** контейнер-вкладка (PropertySheet) настроек двух мелодий уведомлений.
**Заголовок:** `TmcSoundEffProp.h` · **Базовый класс:** `CPropertySheet` (`DECLARE_DYNAMIC`)

**Поля (public):** `CTmcSoundMel1 cMelody1`, `CTmcSoundMel1 cMelody2` — две страницы мелодий (см. §9.5).

**Методы:**

| Метод | Назначение |
|-------|-----------|
| `CTmcSoundEffProp(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0)` | Конструктор с заголовком из ресурса |
| `CTmcSoundEffProp(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0)` | Конструктор с текстовым заголовком |
| `virtual ~CTmcSoundEffProp()` | Деструктор |

### 9.5. `CTmcSoundMel1` — страница мелодии

**Назначение:** страница настройки одной мелодии: интервал и 12 нот.
**Заголовок:** `TmcSoundMel1.h` · **Базовый класс:** `CPropertyPage` (`DECLARE_DYNCREATE`) · **Ресурс:** `IDD_TMC_SOUND_MELODY1`

**Поля (public):** `CString csMelodyName` — имя мелодии.

**Поля DDX:**

| Поле | Тип | Назначение |
|------|-----|-----------|
| `m_SoundInterval` | `int` | Интервал между нотами |
| `m_Note1`…`m_Note12` | `int` | 12 нот мелодии |

**Методы:**

| Метод | Назначение |
|-------|-----------|
| `CTmcSoundMel1()` / `~CTmcSoundMel1()` | Конструктор / деструктор |
| `void DoDataExchange(CDataExchange* pDX)` | DDX/DDV |

---

## 10. Формат входного файла (`.tpl`)

Секции шаблона (разбираются методами `Read*Section` через библиотеку TMCIndan; детальное описание формата — `docs/api/libs/tmcindan.md`):

| Секция | Назначение |
|--------|-----------|
| `STEP` | Число шагов расчёта (nTMax) |
| `PARAM` | Параметры: частота, Δ, Tmin/Tmax, Xmin/Xmax/Ymin/Ymax, высота, единицы измерения |
| `TOPOLOGY` | Блоки: прямоугольники, круги, полигоны, входы, поглотители |
| `LINK_LIST` | Связи между блоками |
| `OUTPUT` | Конфигурация выходных файлов |

Перед разбором файл проходит препроцессинг библиотекой PREPR (макроподстановки, промежуточный файл `$$vr$$s.prc`).

---

## 11. Форматы выходных файлов

| Расширение | Содержимое | Потребитель |
|------------|-----------|-------------|
| `.t` | Временно́й сигнал | TMCROS |
| `.ex` | Распределение поля (бинарный, заголовок `#TMC_GraphicsOutputFieldFile`) | FieldView |
| `.tt` | Топология (имя формируется в TMCIndan: `csFileNameTopology.Format("%s.tt", ...)`) | FieldView |
| `.s` | S-матрица | TMCGROUT |
| `.eps` | Распределение ε (вход и выход, формат §6.4) | ядро |

> Файлы `.dop` — это **документы вьювера TMC_DN** (диаграммы направленности); ядро напрямую `.dop` не пишет — экспорт данных для ДН выполняет `CFieldIntegrated::ExportToDirectionalPattern` (TMCIndan, см. `docs/api/libs/tmcindan.md`).

Запись выходных файлов реализована в подсистеме `CTmcRTH_IndanOutput` библиотеки TMCIndan.

---

## 12. Требует уточнения у автора (К.Н. Климов)

1. **Точность `_real = float`:** достаточна ли одинарная точность для длинных расчётов; предусмотрен ли сценарий сборки с `_PREC_DOUBLE`?
2. **Алгоритм `RunScatteringNode`:** математика рассеяния на узлах (схема аналогична TLM/FDTD?) — словесное описание для документации.
3. **Поглотитель:** какой тип граничного условия реализован (Mur, PML, согласованная нагрузка)? Как подбирается проводимость `SetAbsorberAdmitance`?
4. **Входные блоки:** форма сигнала возбуждения (`dFaza`/`dAmplitude`): гармоника, гаусс, радиоимпульс? Роль `dTmin`/`dTmax`?
5. **Вращающиеся/движущиеся блоки:** поля `csW`, `csVx`, `csVy`, `nTxMove`, `nTyMove` — подтвердить назначение (вращение и перемещение блока во времени) и работоспособность.
6. **`DistributionIntegrated`:** что именно накапливается и почему в X-моде вызовы закомментированы?
7. **Сплайны (`CSplaneOne`):** упоминались в ранних версиях — в текущем коде H-ядра не найдены; подтвердить удаление.
8. **Кросс-зависимости:** `TmcSMatrix.cpp`/`TmcTtoS.cpp` из `viewers/Tmcrtout` компилируются в составе ядра — намеренная архитектура или исторически?
9. **Версии `_VERSION_DEMO___`/`_VERSION_EDUC___`/`_VERSION_PROF___`** (Typerth.h): чем отличаются и нужны ли ещё.
10. **`RunStepGlobalOneTacts`:** режим шага по одному такту — используется ли.
11. **`TmcErrorMessage.cpp/.h` (`CTmcErrorMessage`):** файлы лежат в каталоге проекта, но не включены в `.vcxproj` и не подключаются исходным кодом (только в `PlanRT_H.clw`). Подтвердить, что это устаревший неиспользуемый код, который можно удалить, либо указать, где он должен вызываться.

---

## 13. Нужные иллюстрации

- 📸 НУЖНА СХЕМА: архитектура ядра — `CPlanRT_HDoc → CTmcRTHRectNode → CTmcRTHNodeDiel[] → sTmcRTHNodeDielOne[]`
- 📸 НУЖНА СХЕМА: поток данных — `.tpl → PREPR → CTmcRTH_Indan → ReadData → InitKernel → RunKernel → PutSmatrix`
- 📸 НУЖНА СХЕМА: алгоритм 1T (H) — `ExciteInputs → ScatteringNode → BlockNode → PutField → DistributionIntegrated` (× 2 подтакта)
- 📸 НУЖНА СХЕМА: типы узлов в расчётной области (диэлектрик, металл, поглотитель, 4 входа)
