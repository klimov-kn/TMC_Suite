# Библиотека TMCLibError — API-документация

> Пакет **TMC Suite**. Маленькая вспомогательная статическая библиотека — «носитель ошибки» (error-object).
> Язык документации: русский. Все сигнатуры приведены по исходникам из `src/libs/TMCLibError/TmcLibError.cpp` и заголовку `src/Include/TmcLibError.h`.

---

## 1. Назначение библиотеки

**TMCLibError** — небольшая вспомогательная библиотека, реализующая паттерн **«объект-носитель ошибки»** (error-object). Её единственный класс `CTmcLibError` представляет собой контейнер, который передаётся **по ссылке** (`CTmcLibError&`) в методы вычислительных классов других библиотек. Метод, обнаруживший ошибку, записывает в этот объект текстовое сообщение, а вызывающий код затем проверяет факт ошибки и читает сообщение.

Ключевые особенности модели:

- ошибка хранится как **текст** — строка `CString` (а не числовой код);
- хранится **только последняя** ошибка: новое сообщение перезатирает предыдущее;
- **нет стека ошибок** и **нет числовых кодов**;
- состояние «есть/нет ошибки» отражает булев флаг `BOOL bIsError`.

> Важно: библиотека **не связана** с числовыми кодами ошибок из `Error1.h`. Это две независимые подсистемы (см. раздел 7).

Помимо класса, заголовок объявляет **две свободные функции** генерации временных имён файлов (см. раздел 4).

---

## 2. Состав сборки

В сборку `TMCLibError.vcxproj` входит **единственный** файл реализации:

| Файл | Назначение |
|------|------------|
| `TmcLibError.cpp` | Реализация класса `CTmcLibError` и двух свободных функций |

Подключаемый заголовок — `..\..\INCLUDE\TmcLibError.h` (общий заголовок пакета, `src/Include/TmcLibError.h`).

**Параметры проекта (`TMCLibError.vcxproj`):**

| Параметр | Значение |
|----------|----------|
| Platform Toolset | `v145` |
| Платформы | `Win32`, `x64` |
| ConfigurationType | `StaticLibrary` |
| UseOfMfc | `Static` (в конфигурации Release) |
| CharacterSet | `MultiByte` |
| Подключаемый property sheet | `build\LibOutput.props` |

> Поскольку `UseOfMfc=Static` и `CharacterSet=MultiByte`, библиотека использует классы MFC (в частности `CString`) и компилируется в многобайтовой (не Unicode) кодировке.

Старый файл проекта формата `.vcproj` (legacy) в настоящей документации **не рассматривается**.

---

## 3. Класс `CTmcLibError`

Объявлен в `TmcLibError.h`, реализован в `TmcLibError.cpp`.

```cpp
class CTmcLibError
{
public:
    void PutErrorMessage( char *pszErrorMessage );
    void PutErrorMessage( CString csErrorMessage );
    CString GetErrorMessage( void );
    BOOL IsError( void );
    void Clear( void );
    CTmcLibError& operator=( CTmcLibError& cError1 );
    CTmcLibError();

public:
    virtual ~CTmcLibError();

private:
    CString csError;
    BOOL    bIsError;
};
```

### 3.1. Назначение класса

`CTmcLibError` — контейнер состояния ошибки. Экземпляр создаётся в «чистом» состоянии (ошибки нет), затем передаётся по ссылке в вычислительные методы; те при необходимости вызывают `PutErrorMessage(...)`. Вызывающий код использует `IsError()` и `GetErrorMessage()`.

### 3.2. Поля (private)

| Поле | Тип | Смысл | Допустимые значения |
|------|-----|-------|---------------------|
| `csError` | `CString` | Текст последнего сообщения об ошибке | Любая строка; по умолчанию `"Error:no"` (макрос `CTMCRTHERROR_0000`) |
| `bIsError` | `BOOL` | Флаг наличия ошибки | `FALSE` — ошибки нет; `TRUE` — ошибка установлена |

Оба поля закрыты (`private`) и доступны только через публичные методы.

### 3.3. Публичные методы

#### `CTmcLibError()`

```cpp
CTmcLibError();
```

Конструктор. Приводит объект в состояние «ошибки нет»:

- устанавливает `bIsError = FALSE`;
- инициализирует `csError` строкой `"Error:no"` (через `csError.Format(CTMCRTHERROR_0000)`).

- **Параметры:** нет.
- **Возврат:** нет (конструктор).

#### `virtual ~CTmcLibError()`

```cpp
virtual ~CTmcLibError();
```

Деструктор. Тело **пустое** (`return;`) — никаких ресурсов вручную не освобождается (память `CString` освобождается самим MFC). Объявлен `virtual`, что допускает корректное удаление через указатель на базовый класс при наследовании.

- **Параметры:** нет.
- **Возврат:** нет (деструктор).

#### `void Clear( void )`

```cpp
void Clear( void );
```

Сброс состояния ошибки в исходное:

- `bIsError = FALSE`;
- `csError` ← `"Error:no"` (`CTMCRTHERROR_0000`).

В теле метода присутствуют **закомментированные** отладочные вызовы `AfxMessageBox` (трассировочные сообщения `"Trace0"`, `"Trace1"`, `"Trace2"`) — в рабочей сборке они неактивны.

- **Параметры:** нет.
- **Возврат:** `void`.

#### `BOOL IsError( void )`

```cpp
BOOL IsError( void );
```

Проверка наличия ошибки.

- **Параметры:** нет.
- **Возврат:** значение поля `bIsError` (`TRUE` — ошибка установлена, `FALSE` — ошибки нет).

#### `void PutErrorMessage( CString csErrorMessage )`

```cpp
void PutErrorMessage( CString csErrorMessage );
```

Установить ошибку, передав сообщение как объект `CString`.

- устанавливает `bIsError = TRUE`;
- присваивает `csError = csErrorMessage`.

- **Параметры:**
  - `csErrorMessage` (`CString`) — текст сообщения об ошибке.
- **Возврат:** `void`.
- **Поведение:** перезаписывает любое предыдущее сообщение (хранится только последнее).

#### `void PutErrorMessage( char *pszErrorMessage )`

```cpp
void PutErrorMessage( char *pszErrorMessage );
```

Перегрузка: установить ошибку, передав сообщение как C-строку.

- устанавливает `bIsError = TRUE`;
- формирует `csError` через `csError.Format( "%s", pszErrorMessage )`.

- **Параметры:**
  - `pszErrorMessage` (`char*`) — указатель на строку с завершающим нулём.
- **Возврат:** `void`.
- **Поведение:** перезаписывает предыдущее сообщение.

> О неактивной перегрузке `PutErrorMessage( char *pszErrorMessage, ... )`.
> В `TmcLibError.cpp` присутствует **полностью закомментированный** вариант с переменным числом аргументов и форматированием через `vsprintf` (с буфером длины `CTMCRTHERROR_STRLENMAX`). Он **не объявлен** в заголовке и **не компилируется** — упомянут здесь лишь для полноты. В рабочей сборке этого метода нет.

#### `CString GetErrorMessage( void )`

```cpp
CString GetErrorMessage( void );
```

Получить текст последней ошибки.

- **Параметры:** нет.
- **Возврат:** значение поля `csError` (`CString`). Если ошибка не устанавливалась, вернётся строка по умолчанию `"Error:no"`.
- **Примечание:** метод возвращает значение независимо от `bIsError`; для корректной интерпретации сначала проверяйте `IsError()`.

#### `CTmcLibError& operator=( CTmcLibError& cError1 )`

```cpp
CTmcLibError& operator=( CTmcLibError& cError1 );
```

Оператор присваивания. Копирует состояние ошибки из `cError1` в текущий объект:

- `bIsError = cError1.IsError();`
- `csError.Format( "%s", cError1.GetErrorMessage() );`

- **Параметры:**
  - `cError1` (`CTmcLibError&`) — объект-источник (передаётся по неконстантной ссылке).
- **Возврат:** `CTmcLibError&` — ссылка на текущий объект (`*this`), что допускает цепочки присваиваний.

---

## 4. Свободные функции

Объявлены в `TmcLibError.h` и реализованы в `TmcLibError.cpp`. К классу `CTmcLibError` отношения не имеют — это утилиты формирования временных имён файлов.

#### `void MakeTempFileNameForBzCalcMn( CString& csTplFileName )`

```cpp
void MakeTempFileNameForBzCalcMn( CString & csTplFileName );
```

Дописывает к переданному имени файла суффикс `"_for_Bz_Mn"` (в конец строки, через `+=`).

- **Параметры:**
  - `csTplFileName` (`CString&`) — имя файла; модифицируется на месте (in/out).
- **Возврат:** `void` (результат — в самом параметре).
- **Назначение (по имени):** формирование временного имени файла для расчёта составляющей **Bz** (вариант «минус», `Mn`).

#### `void MakeTempFileNameForBzCalcPl( CString& csTplFileName )`

```cpp
void MakeTempFileNameForBzCalcPl( CString & csTplFileName );
```

Дописывает к переданному имени файла суффикс `"_for_Bz_Pl"`.

- **Параметры:**
  - `csTplFileName` (`CString&`) — имя файла; модифицируется на месте (in/out).
- **Возврат:** `void` (результат — в самом параметре).
- **Назначение (по имени):** формирование временного имени файла для расчёта составляющей **Bz** (вариант «плюс», `Pl`).

---

## 5. Макросы заголовка

Определены в `TmcLibError.h`.

| Макрос | Значение | Где используется |
|--------|----------|------------------|
| `CTMCRTHERROR_0000` | `"Error:no"` | **Используется** — в конструкторе и `Clear()` как «нет ошибки» |
| `CTMCRTHERROR_0001` | `"Error:"` | Объявлен, **не используется** |
| `CTMCRTHERROR_0002` | `"Error:"` | Объявлен, **не используется** |
| `CTMCRTHERROR_0003` | `"Error:"` | Объявлен, **не используется** |
| `CTMCRTHERROR_0004` | `"Error:"` | Объявлен, **не используется** |
| `CTMCRTHERROR_0005` | `"Error:"` | Объявлен, **не используется** |
| `CTMCRTHERROR_0006` | `"Error:"` | Объявлен, **не используется** |
| `CTMCRTHERROR_0007` | `"Error:"` | Объявлен, **не используется** |
| `CTMCRTHERROR_0008` | `"Error:"` | Объявлен, **не используется** |
| `CTMCRTHERROR_0009` | `"Error:"` | Объявлен, **не используется** |
| `CTMCRTHERROR_0010` | `"Error:"` | Объявлен, **не используется** |
| `CTMCRTHERROR_STRLENMAX` | `512` | Использовался **только** в закомментированном `vsprintf`-варианте `PutErrorMessage` (см. раздел 3.3); в активном коде не применяется |

Все макросы `CTMCRTHERROR_0001`..`CTMCRTHERROR_0010` имеют одно и то же значение `"Error:"` и нигде не задействованы.

---

## 6. Схема хранения и передачи ошибок

Модель ошибки в TMCLibError предельно проста:

- **Состояние** = одна строка `CString csError` + один флаг `BOOL bIsError`.
- **Только последняя ошибка.** Нет стека, нет истории, нет числовых кодов — повторный вызов `PutErrorMessage(...)` перезатирает предыдущий текст.

**Типовая схема передачи (по ссылке).** Объект `CTmcLibError` создаётся вызывающим кодом и передаётся по ссылке `CTmcLibError&` в методы вычислительных классов, например (сигнатуры — из библиотеки TMCIndan, использующей TMCLibError):

```cpp
Read( CString& , CTmcLibError& );
IsCorrectly( CTmcLibError& );
Search_1Param( ... , CTmcLibError& );
```

Внутри такого метода при ошибке вызывается:

```cpp
cError.PutErrorMessage( "текст ошибки" );
```

Вызывающий код после возврата проверяет состояние:

```cpp
if( cError.IsError() )
{
    CString msg = cError.GetErrorMessage();
    // вывести / обработать сообщение
}
```

Связи с числовыми кодами `Error1.h` в этой схеме **нет** — сообщение всегда текстовое.

---

## 7. Зависимости

**MFC.** Реализация подключает `stdafx.h` (который, как правило, тянет `afx.h` / `afxwin.h`). Из MFC используются:

- `CString` — хранение и форматирование текста ошибки и имён файлов;
- `BOOL` — флаг состояния;
- `AfxMessageBox` — только в **закомментированном** отладочном коде `Clear()`.

Заголовок `TmcLibError.h` напрямую подключает `<cstringt.h>` (объявление `CString`-классов ATL/MFC). Строка `#include <windows.h>` в заголовке закомментирована.

**Не используются** библиотеки пакета: **complex**, **exprint**, **PREPR**, **SFILE95** — прямых `#include` из них в TMCLibError нет.

**Отдельная подсистема `Error1.h` — НЕ связана с TMCLibError.** `Error1.h` — это независимая legacy-подсистема обработки ошибок с числовыми кодами и функциями вроде `put_error_messege`, `SetErrorFileName` (с атрибутами `_far` / `_fortran` — наследие win16). К классу `CTmcLibError` она **отношения не имеет**. Эти две системы не следует путать: `CTmcLibError` хранит ошибку как текст, `Error1.h` оперирует числовыми кодами.

---

## 8. Использование

**Линковка ядрами.** Оба счётных ядра — **PlanarRT_H** (`tmc_rth.exe`) и **PlanarRT_X** (`tmc_rtx.exe`) — указывают `TMCLibError.lib` в `AdditionalDependencies` (вместе с `sfile95`, `prepr`, `exprint`, `TMCIndan`).

**Фактическое применение — в библиотеке TMCIndan.** Хотя ядра линкуются с `TMCLibError.lib` напрямую, класс `CTmcLibError` реально используется внутри библиотеки **TMCIndan**, которая, в свою очередь, используется ядрами. Места применения (по исходникам TMCIndan):

- `TmcRTH_IndanTopology.cpp`;
- `TmcRTH_IndanParam.cpp`;
- `TmcRTH_IndanOutput.cpp`;
- `FieldIntegrated.cpp` — метод `CFieldIntegrated::GetError()` возвращает `CTmcLibError&` (объект-носитель ошибки отдаётся вызывающему по ссылке).

**Не путать с `CTmcTtoS`.** Во вьюверах есть **отдельный, похожий по смыслу** класс `CTmcTtoS` (`TmcTtoS.cpp`) с полями того же назначения (`bIsError` / `csError`). Это **не** `CTmcLibError` и **не** часть библиотеки TMCLibError — совпадает лишь идея «носителя ошибки». Их следует различать.
