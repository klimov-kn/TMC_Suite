# Программа FieldView — API-документация

> Пакет **TMC Suite**. Графический просмотрщик распределений электромагнитного поля в двумерной области: строит **2D/3D-поверхности и линии** распределения поля, диэлектрической проницаемости (ε) и топологии блоков с использованием **OpenGL**.
> Язык документации: русский. Все сигнатуры приведены по исходникам из `src/fieldview/` и общим заголовкам из `src/Include/`.

---

## 1. Назначение программы

**FieldView** (выходной EXE — `FieldView.exe`; файл проекта `FldView.vcxproj`, `<TargetName>FieldView</TargetName>`) — графический просмотрщик результатов электродинамического расчёта: строит **2D/3D-визуализации распределения поля** в расчётной области, а также распределения диэлектрической проницаемости и топологии блоков.

Программа построена по схеме **MFC Document/View** (MDI). Каждый документ — бинарный файл распределения поля (`.ex`, формат TMC Suite), который содержит параметры сетки и массив значений поля. Программа умеет:

- открывать файлы распределения поля (`.ex`) и топологические файлы (`.tt`);
- строить **3D-поверхности** (GL_TRIANGLES) и **2D-карты** (GL_QUADS) распределения поля;
- строить **линии** (GL_LINE_STRIP) распределения поля по сечениям;
- отображать **распределение ε** по топологии;
- отображать **топологию** (прямоугольные, круговые, полигональные блоки: металл, магнетик, диэлектрик, входы, поглотители);
- строить **оси X/Y/Z** с размерными линиями и нумерацией входов;
- **поворачивать** сцену (мышью и командами, вокруг осей X/Y/Z);
- **переносить** сцену и данные, **масштабировать** вид (в т.ч. по одной оси);
- настраивать **20-цветную палитру** для поля и ε (автоинтерполяция или ручные уровни);
- настраивать **пределы Z** (автоматические или ручные);
- переключаться между **2D и 3D** режимами раздельно для поля, ε и топологии;
- настраивать **прозрачность** (blend) слоёв;
- сохранять параметры отображения в **реестре** (раздел приложения, ключи §5.10);
- отслеживать **изменения файла** и автоматически перечитывать данные (таймер `OnTimer`).

**Требует OpenGL:** `opengl32.lib`, `glu32.lib`.

---

## 2. Состав проекта

Состав по `FldView.vcxproj` (секция `ClCompile`):

| Файл | Класс / содержимое | Назначение (кратко) |
|------|--------------------|---------------------|
| `FldView.cpp/.h` | `CFldViewApp` | Класс приложения, `InitInstance`, шаблон документа |
| `MainFrm.cpp/.h` | `CMainFrame` | Главное MDI-окно: тулбар, статусбар |
| `ChildFrm.cpp/.h` | `CChildFrame` | Дочернее MDI-окно |
| `FldViewDoc.cpp/.h` | `CFldViewDoc` | Документ: чтение бинарных данных поля (`.ex`), управление памятью |
| **`FldViewView.cpp/.h`** | `CFldViewView` | **Основной вид:** вся OpenGL-отрисовка, мышь/меню, настройки (крупнейший файл проекта, ~162 КБ) |
| `TmcGLText.cpp/.h` | `CTmcGLText` | Вывод текста через GDI + растровые операции OpenGL |
| `TmcBlockTpl.cpp/.h` | `CTmcBlockTpl` | Чтение топологического файла (`.tt`), распределение ε(x,y) |
| `TmcBoundaryTpl.cpp/.h` | `CTmcBoundaryTpl` | Границы блоков топологии (прямоугольные, круговые, полигональные) |
| `TmcTopSize.cpp/.h` | `CTmcTopSize` | Размерные линии топологии |
| `TMCDialogPropet.cpp/.h` | `CTMCDialogPropet` | Окно свойств (PropertySheet, 4 вкладки) |
| `TMCDialogPlace.cpp/.h` | `CTMCDialogPlace` | Вкладка «Размещение» (оси X/Y, единицы, шрифт) |
| `TMCDialogField.cpp/.h` | `CTMCDialogField` | Вкладка «Поле» (цветовая шкала, Z-пределы, 2D/3D, blend) |
| `TMCDialogEps.cpp/.h` | `CTMCDialogEps` | Вкладка «Eps» (цветовая шкала ε) |
| `TMCDialogTopology.cpp/.h` | `CTMCDialogTopology` | Вкладка «Топология» (отображение блоков) |
| `TmcASetColor.cpp/.h` | `CTmcASetColor` | Диалог настройки 20-цветной палитры |
| `ColorLevelDlg.cpp/.h` | `CColorLevelDlg` | Диалог ручного ввода уровней цвета |
| `StdAfx.cpp/.h` | — | Прекомпилированный заголовок MFC |
| `resource.h`, `FldView.rc`, `RES\` | — | Ресурсы (иконки, тулбар) |

> Файлы `TmcRTH_BlockList1.cpp/.h` физически лежат в каталоге, но **не включены в проект** (нет в `ClCompile`). Заголовок содержит жёсткий путь `z:\work\source\include\tmcrth_bolcklist.h`; класс `CTmcRTH_BlockList1` — пустая обёртка над `CTmcRTH_BlockList`. Мёртвый код.

**Зависимости** (`AdditionalDependencies`): `opengl32.lib`, `glu32.lib`, `sfile95.lib`, `prepr.lib`, `exprint.lib`, `TMCIndan.lib`, `TMCLibError.lib`.

> В `TmcGLText.h` удалено подключение `<gl\glaux.h>`: GLAUX отсутствует в современном Windows SDK, aux*-функции в коде не используются (правка портирования, см. `CHANGELOG.md`, Фаза 4).

---

## 3. Класс `CFldViewApp`

**Назначение:** класс приложения MFC.
**Заголовок:** `FldView.h` · **Базовый класс:** `CWinApp`

### `BOOL InitInstance()`
Инициализация: `AfxOleInit()`, `SetRegistryKey(...)` (фикс бага №6 — краш `CRecentFileList::Add` без OLE/реестра, см. `CHANGELOG.md`), регистрация `CMultiDocTemplate`, создание `CMainFrame`, чтение MRU, восстановление положения окна.

Обработчик: `afx_msg void OnAppAbout()` — окно «О программе».

### Пользовательские сообщения (определены в `FldView.h`)

| Сообщение | Значение | Смысл |
|-----------|----------|-------|
| `WM_USERAPPLY_PROPPLACE` | `WM_USER+5` | Применение свойств размещения |
| `WM_USERAPPLY_PROPTOPOL` | `WM_USER+6` | Применение свойств топологии |
| `WM_USERAPPLY_PROPEPS` | `WM_USER+7` | Применение свойств ε |
| `WM_USERAPPLY_PROPFIELD` | `WM_USER+8` | Применение свойств поля |

Диалоги свойств посылают эти сообщения виду; обработчики — §5.9.

---

## 4. Класс `CFldViewDoc` — документ

**Назначение:** чтение бинарных данных поля (`.ex`), хранение массива значений, отслеживание изменения файла.
**Заголовок:** `FldViewDoc.h` · **Базовый класс:** `CDocument`

### 4.1. Поля

| Поле | Тип | Назначение |
|------|-----|-----------|
| `cError` | `CTmcLibError` | Ошибки (публичное поле) |
| `nX` / `nY` | `int` | Число узлов по X / Y |
| `dDelta` | `double` | Шаг сетки |
| `dXmin` / `dYmin` | `double` | Минимальные координаты |
| `nTCurrent` | `int` | Номер временно́го шага |
| `dTCurrent` | `double` | Текущее время |
| `nAccuracy` | `int` | Точность (число знаков) |
| `pdSurface` | `double*` | **Массив значений поля** (nY × nX) |
| `pfData` | `FILE*` | Дескриптор открытого файла данных |
| `dLongUnit` / `csLongUnit` | `double` / `CString` | Коэффициент и обозначение единицы длины |
| `dTimeUnit` / `csTimeUnit` | `double` / `CString` | Коэффициент и обозначение единицы времени |
| `bBusy` | `volatile BOOL` | Флаг занятости (чтение в процессе) |
| `nViews` / `nViewsSinchroniz` | `volatile int` | Счётчики подключённых видов / синхронизированных видов |
| `ftLastWriteTime` | `FILETIME` | Время последней записи файла |

### 4.2. Публичные методы

| Метод | Назначение |
|-------|-----------|
| `void ReadData(void)` | Чтение `.ex`-файла: `ReadDataParameters` (заголовок) + `InitAllocationArray` + `ReadDataArray` (бинарный массив) |
| `double GetdZ(double dX, double dY)` | Значение поля в точке (по сетке) |
| `int GetNumberElement(double dX, double dY)` | Индекс элемента сетки по координатам |
| `double* GetpdSurface(void)` | Указатель на массив поля |
| `double GetdDelta(void)` | Шаг сетки |
| `int GetnX(void)` / `int GetnY(void)` | Число узлов |
| `double GetdXmin(void)` / `double GetdYmin(void)` | Начало координат |
| `double GetdTCurrent(void)` / `int GetnTCurrent(void)` | Время / номер шага |
| `double GetdLongUnit(void)` / `CString GetcsLongUnit(void)` | Единица длины |
| `double GetdTimeUnit(void)` / `CString GetcsTimeUnit(void)` | Единица времени |
| `BOOL IsFileRead(void)` | Файл прочитан |
| `BOOL IsBusy(void)` / `void SetBusyOn(void)` / `void SetBusyOff(void)` | Управление флагом занятости |
| `FILETIME GetLastWriteFile(void)` | Время последней записи файла (для отслеживания обновлений) |
| `void AddViews__(void)` / `void DelViews__(void)` | Регистрация/снятие вида |
| `void SetSynchronizationFlag(void)` | Установка флага синхронизации видов |

Переопределения MFC: `OnNewDocument()`, `Serialize(CArchive&)`.

### 4.3. Приватные методы

| Метод | Назначение |
|-------|-----------|
| `void ReadDataParameters(FILE **fp)` | Разбор текстового заголовка `.ex` (маркеры §11.1) |
| `void InitAllocationArray(void)` | Выделение `pdSurface` |
| `void ReadDataArray(FILE **fp)` | Чтение бинарного массива |
| `void CloseFileData(void)` | Закрытие файла |
| `void DeleteData(void)` | Освобождение данных |
| `FILETIME LastWriteTime(CString)` / `FILETIME LastWriteTime(char*)` | Время записи файла |

---

## 5. Класс `CFldViewView` — вид (OpenGL)

**Назначение:** вся OpenGL-отрисовка, обработка пользовательского ввода, хранение и сохранение настроек отображения.
**Заголовок:** `FldViewView.h` · **Базовый класс:** `CScrollView`
**Размер:** `FldViewView.cpp` ~162 КБ — крупнейший файл проекта.

В `FldViewView.h` также объявлены глобальные `PutTrace`/`PutStatistics` (вывод в статусбар, как в ядрах).

### 5.1. Инициализация OpenGL

| Метод | Назначение |
|-------|-----------|
| `void GL_Init(void)` | Создание OpenGL-контекста (`hrc`), палитры, формата пикселей |
| `GLvoid GL_Resize(void)` | Установка viewport и матриц проекции |
| `void GL_Close(void)` | Уничтожение OpenGL-контекста |
| `BOOL bSetupPixelFormat(void)` | Выбор формата пикселей |
| `void CreateRGBPalette(void)` | Создание RGB-палитры (`m_cPalette`); возвращает `void` |
| `unsigned char ComponentFromIndex(int i, UINT nbits, UINT shift)` | Компонента цвета из индекса палитры |

Связанные поля: `HGLRC hrc` (контекст OpenGL), `CClientDC *m_pDC`, `CPalette m_cPalette`, `CPalette *m_pOldPalette`, `CRect m_oldRect`, `float m_fRadius`, `BOOL m_play`.

### 5.2. Отрисовка поля

| Метод | Назначение |
|-------|-----------|
| `void DrawScene(void)` | Главная функция отрисовки (вызывает все Draw-методы) |
| `void DrawField(void)` | Диспетчер: 2D или 3D вариант по `n2D3DFlag` |
| `void DrawFieldSurface(void)` | 3D-поверхность поля (GL_TRIANGLES) |
| `void DrawFieldSurface2(void)` | 2D-карта поля (GL_QUADS) |
| `void DrawFieldLine(void)` / `void DrawFieldLine2(void)` | 3D/2D линии поля (GL_LINE_STRIP) |
| `void DrawFieldCursore(void)` / `void DrawFieldCursore2(void)` | 3D/2D курсор значения поля |

### 5.3. Отрисовка ε

| Метод | Назначение |
|-------|-----------|
| `void DrawEps(void)` | Диспетчер Eps (по `n2D3DFlag_Eps`) |
| `void DrawEpsSurface(void)` / `void DrawEpsSurface2(void)` | 3D/2D поверхность ε |
| `void DrawEpsdLine(void)` / `void DrawEpsdLine2(void)` | 3D/2D линии ε |

### 5.4. Отрисовка топологии

| Метод | Назначение |
|-------|-----------|
| `void DrawTopology(void)` | Диспетчер топологии (по `n2D3DFlagTopology`) |
| `void DrawTopSurface(void)` / `void DrawTopSurface2(void)` | 3D/2D поверхность блоков |
| `void DrawTopdLine(void)` / `void DrawTopdLine2(void)` | 3D/2D контуры блоков |
| `void DrawTopInputNum(void)` / `void DrawTopInputNum2(void)` | Нумерация входов (через `CTmcGLText::DrawInputNum`) |
| `void CalcnTopPolygon(void)` | Подсчёт числа полигонов топологии (`nTopPolygon`) |

### 5.5. Оси и размерные линии

| Метод | Назначение |
|-------|-----------|
| `void DrawAxies(void)` | Оси X/Y/Z |
| `void DrawAxiesSize(void)` / `void DrawAxiesSizeX(void)` / `void DrawAxiesSizeY(void)` | Размерные линии осей |
| `void DrawAxiesSizeZField(void)` / `void DrawAxiesSizeZEps(void)` | Размерная линия Z (шкала поля / ε) |
| `void DrawSize(void)` / `void DrawSize2(void)` / `void DrawSize3(void)` | Размерные линии топологии (варианты) |
| `void PrepareBoundTplSize(void)` / `void PrepareBoundTplSizeToGLCoord(void)` | Подготовка размерных линий (массив `pcTopSize`, счётчик `nTopSize`) и пересчёт в координаты GL |
| `void VTextOut(CDC *pDC, int nX, int nY, CString cText)` | Вертикальный текст средствами GDI |

### 5.6. Координатные преобразования

| Метод | Назначение |
|-------|-----------|
| `double XtoGLWinCoord(double x)` / `double XtoGLWinCoord(int j)` | X (координата или индекс столбца) → координата GL |
| `double YtoGLWinCoord(double y)` / `double YtoGLWinCoord(int i)` | Y (координата или индекс строки) → координата GL |
| `double ZtoGLWinCoord(double dZcurrent)` | Значение поля → координата Z в GL |
| `double Z_EpstoGLWinCoord(double dZcurrent)` | Значение ε → координата Z в GL |

### 5.7. Управление цветом

| Метод | Назначение |
|-------|-----------|
| `void SetColor(double z)` | Цвет по значению поля (палитра `pscSurfaceColor[20]`) |
| `void SetColor_Eps(double z)` | Цвет по значению ε (`pscSurfaceColor_Eps[20]`) |
| `void SetColor_Cursor(double z)` | Цвет курсора |
| `void SetColor(COLORREF)` | Конкретный цвет |
| `void SetColor(COLORREF, int nBlend11)` | Цвет с прозрачностью |
| `void SetColornType(int nType)` | Цвет по типу блока топологии (поля `scColorEps/Met/Mag/Inp/Abs`) |
| `COLORREF* GetpscSurfaceColor(void)` | Доступ к палитре (для диалогов; публичный) |
| `BOOL* GetbHightColorRezolution(void)` | Доступ к флагу повышенного цветового разрешения (публичный) |

### 5.8. Подготовка данных

| Метод | Назначение |
|-------|-----------|
| `void ReadData(void)` | Чтение данных из документа |
| `void PrepareData(void)` | Подготовка данных к отрисовке (массивы `pdSurface`, `pdSurfaceX/Y`, пределы) |
| `void ReadTpl(void)` / `void PrepareTpl(void)` | Чтение и подготовка топологии `.tt` (`cBlockTpl`) |
| `void PrepareBoundaryTpl(void)` | Подготовка границ блоков (`pcBoundBlockTpl`, счётчик `nBoundBlockTpl`) |
| `void SetTplFileName(void)` | Формирование имени `.tt`-файла (`csTplFileName`) |
| `void DeleteData(void)` | Освобождение всех данных |
| `int GetNumberElement(double dX, double dY)` | Индекс элемента по координатам (private) |
| `void SetDefaultValue(void)` | Значения настроек по умолчанию |
| `void WriteIniFile(void)` | Сохранение настроек (ключи §5.10) |
| `void PutTrace1(void)` / `void PutStatistics1(void)` | Вывод текущих значений в статусбар |

### 5.9. Обработчики сообщений

**Вращение сцены:** `OnRotateLeft/Right/Top/Bottom`, `OnRotateLefty/Righty`, `OnRotatexn/xv`, `OnRotateyn/yv`, `OnRotatezn/zv` (углы `wAngleX/Y/Z`).

**Перенос:** `OnTranslateXt/Xb/Yt/Yb/Zt/Zb`, `OnTranslateXn/Xv/Yn/Yv/Zn/Zv` (величины `wTranslateX/Y/Z`); **перенос данных** — `OnTranslatedataxn/xv/yn/yv`.

**Стрелки:** `OnStrelkleft/right/up/down`.

**Масштабирование:** `OnViewZoomP/M` (общее), `OnViewZoomPx/Mx` (по X), `OnViewZoomPy/My` (по Y); `OnViewDefault` (вид по умолчанию), `OnViewProportionally` (пропорциональный режим, флаг `bProportionally`).

**Режимы отображения поля:** `OnFlagdrawsurface`/`OnFlagdrawline` (поверхность/линии), `OnDimension23switch` (2D/3D), `OnFieldL`/`OnFieldValue`/`OnFieldModul` (тип величины: уровень/значение/модуль; флаги `bDrawFieldValueFlag`, `bDrawFieldModulFlag`).

**Режимы ε:** `OnFlagdrawsurfaceeps`/`OnFlagdrawlineeps`, `OnDimension23switcheps`.

**Режимы топологии:** `OnFlagdrawsurfacetop`/`OnFlagdrawlinetop`, `OnDimension23switchtop`; видимость типов блоков — `OnViewTopologyBoundaryEps/Metal/Magnetic/Input/Absorber` (флаги `bDrawFlagEps/Metal/Magnetic/Input/Absorber`).

**Размерные линии:** `OnViewFieldSize`, `OnViewEpsSize`, `OnViewTopologyDimensionsAlonganxaxies/Alonganyaxies`, `OnViewTopologySizeBlock`, `OnViewTopologySizeLinklist`.

**Мышь:** `OnLButtonDown/Up`, `OnMouseMove` (вращение мышью; поля `cMousePoint`, `bIsMouseLButonDown`), `OnLButtonDblClk`, `OnRButtonDown`.

**Прочее:** `OnViewParameters` (окно свойств §10.1), `OnTimer(UINT_PTR nIDEvent)` (опрос изменения файла и перечитывание), `OnDestroy`.

**Сообщения от диалогов:**

| Обработчик | Сообщение |
|-----------|----------|
| `LRESULT OnUserApplyPropDialogPlace(WPARAM, LPARAM)` | `WM_USERAPPLY_PROPPLACE` |
| `LRESULT OnUserApplyPropDialogField(WPARAM, LPARAM)` | `WM_USERAPPLY_PROPFIELD` |
| `LRESULT OnUserApplyPropDialogEps(WPARAM, LPARAM)` | `WM_USERAPPLY_PROPEPS` |
| `LRESULT OnUserApplyPropDialogTopol(WPARAM, LPARAM)` | `WM_USERAPPLY_PROPTOPOL` |
| `LRESULT OnCloseColorLevelDialog(WPARAM, LPARAM)` | закрытие `CColorLevelDlg` (`pcColorLevelDialog`) |

Для большинства команд есть парные `OnUpdate*`-обработчики доступности/отметки пунктов меню.

**Переопределения MFC:** `OnDraw`, `PreCreateWindow`, `OnPrepareDC`, `OnInitialUpdate`, `OnPreparePrinting`, `OnBeginPrinting`, `OnEndPrinting`, `OnUpdate`.

### 5.10. Ключевые поля настроек и ключи сохранения

Основные группы приватных полей:

| Группа | Поля |
|--------|------|
| Палитры | `COLORREF pscSurfaceColor[20]`, `pscSurfaceColor_Eps[20]` |
| Цвета | `scAxiesColor`, `scColorEps`, `scColorMet`, `scColorMag`, `scColorInp`, `scColorAbs`, `scBackgoundColor` |
| Прозрачность | `nBlend`, `nBlend_Eps`, `nBlendTopology` |
| 2D/3D | `n2D3DFlag`, `n2D3DFlag_Eps`, `n2D3DFlagTopology` |
| Пределы | `dXmin/dXmax/dYmin/dYmax`, `dZmin/dZmax`, `dZmin_Eps/dZmax_Eps` |
| Сцена | `wAngleX/Y/Z`, `wTranslateX/Y/Z`, `dXkoefProp`, `dYkoefProp` |
| Данные | `nX`, `nY`, `dDelta`, `pdSurface`, `pdSurface_Eps`, `pdSurfaceX`, `pdSurfaceY` |
| Топология | `pnTopType`, `pdTopX1..X4`, `pdTopY1..Y4`, `pdTopXinp/Yinp`, `nTopPolygon`, `nBoundBlockTpl`, `pcBoundBlockTpl`, `pcTopSize`, `nTopSize`, `dZtopol`, `dZtopolStrelk`, `nInputNum` |
| Флаги отображения | `bDrawFlag`, `bDrawSurfaceFlag(_Eps)`, `bDrawLineFlag(_Eps)`, `bDrawFlagTopology`, `bDrawSurfaceFlagTopology`, `bDrawLineFlagTopology`, `bDrawFlagAxies*`, `bXsizeFlag/bYsizeFlag/bZsizeFlag(_Eps)`, `bTopologySizeFlag(LinkList)`, `bDrawFieldValueFlag`, `bDrawFieldModulFlag`, `bHightColorRezolution`, `bProportionally` |
| Вспомогательные | `lfAxiesFont` (шрифт осей), `pcGLText` (`CTmcGLText*`), `cBlockTpl` (`CTmcBlockTpl`), `cError`, `bBusy`, `nPropertyPageInd`, `pcColorLevelDialog`, `dDrawFieldValueX/Y`, `nDrawFieldValue1..5` |

Настройки сохраняются (`WriteIniFile`) под именами-константами `TMC_GROFLD_*_INI` из `FldViewView.h` — раздел `#TMC_GraphicsOutputFieldFile`, параметры: `DrFl` (флаги отрисовки), `DrFlSurface`, `DrFlLine`, `DrFlAxies`, `FlXSize/FlYSize/FlZSize`, `Fl23D`, `Blend`, `Xmin/Xmax/Ymin/Ymax/Zmin/Zmax`, `XAngle/YAngle/ZAngle`, `XTranslate/YTranslate/ZTranslate`, `ColorAxies`, `FontAxies`, `FontNameAxies`, `ColorZField`, `ColorZEps`, `Dr_FlAxiesSize`, `Topology`, `TopologyColor_EpsMetMagInp`, `PropertyPageNumber`, `ColorResolution`, `EpsParam`, `DrawFieldModulFlag`, `#INIWND` (окно).

---

## 6. Класс `CTmcGLText` — OpenGL-текст

**Назначение:** вывод текста в OpenGL-сцену: строка рендерится в GDI-битмап, который выводится растровой операцией OpenGL.
**Заголовок:** `TmcGLText.h` · **Базовый класс:** `CWnd`

### Публичные методы

| Метод | Назначение |
|-------|-----------|
| `void DrawString(char *ch, double x, double y, double z)` | Строка в точке (x, y, z) сцены |
| `void DrawString(CString cs, double x, double y, double z)` | То же для `CString` |
| `void DrawInputNum(int n, double x, double y, double z)` | Номер входа («Inp N») |
| `void SetColor(COLORREF scColorRef1)` | Цвет текста |
| `void SetColorSize(void)` | Цвет размерных линий |
| `void SetColorInput(void)` | Цвет номеров входов |
| `void SetFont(LOGFONT *plf1)` | Шрифт |

### Приватные элементы

| Элемент | Назначение |
|---------|-----------|
| `unsigned char* getGLmonoBits(HDC hDC, HBITMAP hBmp, int *size)` | Преобразование GDI-битмапа в моно-битмап для `glBitmap` |
| `unsigned char* createStringBitmapFont(HDC hDC, char *str, PSIZEL size1)` | Растеризация строки выбранным шрифтом |
| `void getBitmap(BITMAP *bmp, LONG width, LONG height)` | Параметры битмапа |
| `int nBlend`, `COLORREF scColorRef`, `HFONT hFont`, `LOGFONT lf`, `SIZE size` | Текущее состояние |

---

## 7. Класс `CTmcBlockTpl` — чтение топологии

**Назначение:** чтение топологического файла (`.tt`) и распределения ε(x,y).
**Заголовок:** `TmcBlockTpl.h` · **Базовый класс:** `CTmcRTH_BlockList` (TMCIndan)

### Публичные методы

| Метод | Назначение |
|-------|-----------|
| `void Read(CString csTplFileName1, CTmcLibError &cError1)` | Чтение `.tt`-файла |
| `int GetnX(void)` | Число узлов по X |
| `double* GetpdSurface(void)` | Массив ε(x,y) |
| `CString GetTplFileName(void)` / `void SetTplFileName(CString&)` | Имя файла |
| `CTmcLibError& GetError(void)` | Ошибки |
| `void DeleteData(void)` | Очистка |

Приватные: `void InitAllocationArray(void)`, `void ReadData(void)`; поля `nTCurrent`, `dTCurrent`, `dDelta`, `dXmin`, `dYmin`, `nX`, `nY`, `pdSurface` (распределение ε), `csTplFileName`, `cError`.

---

## 8. Класс `CTmcBoundaryTpl` — границы блоков

**Назначение:** представление границ одного блока топологии для отрисовки контуров и размерных линий.
**Заголовок:** `TmcBoundaryTpl.h` — обычный C++-класс (наследование от `CWnd` закомментировано).

### Публичные методы

| Метод | Назначение |
|-------|-----------|
| `void SetBlockList(CTmcRTH_BlockList *pcBlList, CTmcLibError &cError, double dDelta)` | Построение границ по описанию блока; внутри ветвится по типу: `SetRectStat`, `SetCircleStat`, `SetPolygonStat`, `SetInputX`, `SetInputY` (private) |
| `int GetnType(void)` / `int GetnBlock(void)` / `int GetnPoint(void)` | Тип блока, номер, число точек контура |
| `CString GetcsBlock(void)` / `CString GetcsEpsExpr(void)` | Ключевое слово блока, выражение ε |
| `double GetdX0(void)` / `double GetdY0(void)` | Координаты привязки |
| `double* GetpdX1..X4(void)` / `double* GetpdY1..Y4(void)` | Массивы координат вершин контура (по 4 угла на сегмент) |
| `CTmcTopSize* GetpcTopSize(void)` / `int GetnTopSize(void)` | Размерные линии блока и их число |
| `CTmcTopSize& GetcTopSizeX0(void)` / `GetcTopSizeY0(void)` | Размерные линии привязки (link list) по X и Y |
| `void SetnInputNum(int n)` / `int GetnInputNum(void)` | Номер входа |

Приватные: `TranslateX0Y0()`, `DeleteData()`; поля `nInputNum`, `nTopSize`, `cTopSizeLinklListX0/Y0`, `pcTopSize`, `dX0/dY0`, `nBlock`, `csBlock`, `csEpsExpr`, `pX1..pX4`, `pY1..pY4`, `nPoint`, `nType`.

---

## 9. Класс `CTmcTopSize` — размерная линия

**Заголовок:** `TmcTopSize.h` · **Базовый класс:** `CWnd`

| Метод | Назначение |
|-------|-----------|
| `void SetdX1/SetdX2/SetdY1/SetdY2(double r)` | Координаты начала/конца линии |
| `double GetdX1/GetdX2/GetdY1/GetdY2(void)` | Чтение координат |
| `void SetdSize(double r)` / `double GetdSize(void)` | Числовое значение размера |
| `void SetcsSize(CString &csr)` / `CString GetcsSize(void)` | Текст размера |
| `void TranslateX0Y0(double dX0, double dY0)` | Смещение линии |
| `CTmcTopSize& operator=(CTmcTopSize&)` | Присваивание |
| `void DeleteData(void)` | Очистка |

---

## 10. Диалоги

### 10.1. `CTMCDialogPropet` — окно свойств
**Базовый класс:** `CPropertySheet`. 4 вкладки: «Размещение» (`CTMCDialogPlace`), «Поле» (`CTMCDialogField`), «Eps» (`CTMCDialogEps`), «Топология» (`CTMCDialogTopology`). По кнопке Apply вкладки шлют виду сообщения `WM_USERAPPLY_PROP*` (§3).

### 10.2. `CTMCDialogPlace` — вкладка «Размещение»
Оси X/Y, единицы длины/времени, шрифт осей, пропорциональное отображение.

### 10.3. `CTMCDialogField` — вкладка «Поле»
Цветовая шкала (20 цветов), пределы Z (авто/ручные), 2D/3D, blend, тип отображаемой величины (L / Value / Modul).

### 10.4. `CTMCDialogEps` — вкладка «Eps»
Цветовая шкала ε, пределы Z, 2D/3D, blend.

### 10.5. `CTMCDialogTopology` — вкладка «Топология»
Видимость блоков по типам (диэлектрик/металл/магнетик/вход/поглотитель), цвета, поверхность/линии, 2D/3D.

### 10.6. `CTmcASetColor` — настройка палитры
Настройка 20-цветной палитры с автоинтерполяцией между опорными цветами.

### 10.7. `CColorLevelDlg` — уровни цвета
Ручной ввод 20 значений уровней цветовой шкалы; о закрытии сообщает виду (`OnCloseColorLevelDialog`).

---

## 11. Форматы входных файлов

### 11.1. Бинарный файл поля (`.ex`)

Маркеры заголовка — константы `TMC_GROFLD_DOCFILE_*` из общего заголовка `src/Include/Tmcgrviw.h` (строки 101–111):

```
#TMC_GraphicsOutputFieldFile FormVer2.0 2000
#TMCGROFLD_nT <nT>
#TMCGROFLD_Time <T>
#TMCGROFLD_Delta <Delta>
#TMCGROFLD_Xmin <Xmin>
#TMCGROFLD_Ymin <Ymin>
#TMCGROFLD_Accuracy <Accuracy>
#TMCGROFLD_LongUnit <unit>
#TMCGROFLD_TimeUnit <unit>
#TMCGROFLD_nX <nX>
#TMCGROFLD_nY <nY>
<двоичные данные: nY × nX значений>
```

### 11.2. Топологический файл (`.tt`)

Создаётся счётным ядром (TMCIndan: `csFileNameTopology.Format("%s.tt", ...)`). Структура:

```
#TMC_TopologyOutputFile FormVer2.0 2000
#BlockListBegin
<блоки топологии>
#BlockListEnd
<параметры поля>
<двоичные данные>
```

### 11.3. Типы блоков топологии (`nType`)

Константы из `src/Include/Tmcgrviw.h` (используются `SetColornType`):

| Константа | Значение | Смысл |
|-----------|----------|-------|
| `CTMCRTH_BLCKNTYPE_EPS` | 0 | Диэлектрик |
| `CTMCRTH_BLCKNTYPE_ABSORBER` | 100 | Поглотитель |
| `CTMCRTH_BLCKNTYPE_METAL` | 101 | Металл |
| `CTMCRTH_BLCKNTYPE_INPXLEFT` | 102 | Вход X слева |
| `CTMCRTH_BLCKNTYPE_INPXRIGHT` | 103 | Вход X справа |
| `CTMCRTH_BLCKNTYPE_INPYTOP` | 104 | Вход Y сверху |
| `CTMCRTH_BLCKNTYPE_INPYBOT` | 105 | Вход Y снизу |
| `CTMCRTH_BLCKNTYPE_MAGNETIC` | 107 | Магнетик |

Дополнительно определены варианты поглотителя `CTMCRTH_BLCKNTYPE_ABSR_0000..1111` (значения 1–16) и `ABSR_1111_1..4` (21–24) — кодировка сторон поглощения битовой маской; см. чек-лист.

---

## 12. Требует уточнения у автора (К.Н. Климов)

1. **OpenGL-палитра:** используется палитровый режим (`CreateRGBPalette`) — для совместимости со старым железом? Актуально ли при портировании?
2. **Параметр blend:** какой именно эффект прозрачности контролируют `nBlend`/`nBlend_Eps`/`nBlendTopology` (диапазон значений?).
3. **Наследование `CTmcBlockTpl : CTmcRTH_BlockList`:** блок-список используется как базовый класс — намеренно?
4. **Флаг `bHightColorRezolution`:** что меняет в отрисовке («ColorResolution» в настройках)?
5. **Семантика групп переноса:** чем отличаются `OnTranslateXt/Xb/...` от `OnTranslateXn/Xv/...` (предположительно тулбар/меню против клавиш) — подтвердить.
6. **Варианты поглотителя `ABSR_0000..1111`:** подтвердить трактовку битовой маски сторон.
7. **Формат `.tt` vs `.dop`:** `.tt` — топология для FieldView, `.dop` — документ TMC_DN; подтвердить, что форматы независимы.
8. **`DrawFieldCursore`:** что именно показывает курсор поля (значение в точке по двойному клику?).
9. **Физический смысл шкалы:** напряжённость в В/м или нормированная величина?
10. **Интервал таймера `OnTimer`** опроса изменений файла.

---

## 13. Нужные иллюстрации

- 📸 НУЖНА СХЕМА: архитектура FieldView — `CFldViewDoc (.f) → CFldViewView (OpenGL) → DrawScene → DrawField/DrawEps/DrawTopology/DrawAxies`
- 📸 НУЖНА СХЕМА: путь топологии — `.tt → CTmcBlockTpl → CTmcBoundaryTpl[] → DrawTopology`
- 📸 НУЖНА СХЕМА: 20-цветная палитра — интерполяция от min до max
- 📸 НУЖНА СХЕМА: окно свойств — 4 вкладки и сообщения `WM_USERAPPLY_PROP*`
