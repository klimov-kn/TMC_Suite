Программа TMC_DN — API-документация
===================================

.. note::

   Пакет **TMC Suite**. Вьювер **диаграмм направленности** (ДН) антенных решёток:
   расчёт и построение диаграммы направленности по описанию решётки излучателей,
   а также расчёт КНД/КИП, отклонений ДН и преобразование «время → S-матрица».
   Язык документации: русский. Сигнатуры приведены по исходникам из
   ``src/viewers/DiaNapGr/``, ядру расчёта ДН из ``src/viewers/DiaNapr/``
   (``c2DArray``, ``cIzl``) и общим заголовкам из ``src/Include/``.

----

1. Назначение программы
-----------------------

**TMC_DN** (исходники — папка ``src/viewers/DiaNapGr/``, выходной EXE — ``TMC_DN.exe``) —
графический просмотрщик и калькулятор **диаграмм направленности** антенных решёток.

Программа построена на той же кодовой базе MFC Document/View (MDI), что и TMCGROUT
(классы ``CTMCGROUTApp``, ``CMainFrame``, ``CChildFrame``, ``CTMCGROUTDoc``,
``CTMCGROUTView`` имеют те же имена), но **расширена для работы с диаграммами
направленности**:

- читает текстовые файлы ``.dat`` с описанием антенной решётки (частота, число
  излучателей, координаты/амплитуды/фазы каждого излучателя, выражение ДН одного элемента);
- по этому описанию **рассчитывает диаграмму направленности** решётки (когерентное
  суммирование полей излучателей по углу) — класс ``c2DArray``;
- вычисляет интегральные характеристики: **КНД** (``Knd``, в разах и дБ) и **КИП** (``Kip``);
- строит 2D-график ДН, показывает КНД/КИП в строке состояния;
- считает ДН в **фоновом потоке** с прогресс-окном (``CThreadCalcDirPat``,
  ``ReadDocFileW_Thread1``);
- вносит **случайные отклонения** (девиации) в параметры излучателей (``cDeviation``,
  ``c2DArray::MakeDeviation``) и считает ДН отклонённой решётки относительно требуемой
  (``CDeviatDirPat``, ``c2DArray::OptimizationInit`` / ``SaveDeviationDirectionalPattern``);
- содержит утилиту преобразования **временно́го сигнала в S-матрицу** (``CDialogTtoS``,
  ``CTmcTtoS``, ``CTmcSMatrix``) — общий с TMCROS код.

Всё, что относится к **рисованию, осям, цветам, масштабированию, формату документа** —
устроено так же, как в TMCGROUT (см. :doc:`tmcgrout`, разделы 3, 7–9, 11–12). Ниже
подробно документируются именно **отличия и дополнения TMC_DN**.

----

2. Состав проекта
-----------------

В сборку ``TMC_DN.vcxproj`` входят файлы из ``src/viewers/DiaNapGr/`` плюс ядро расчёта ДН
из ``src/viewers/DiaNapr/``:

.. list-table::
   :header-rows: 1
   :widths: 25 25 50

   * - Файл
     - Класс / содержимое
     - Назначение (кратко)
   * - ``TMCGROUT.cpp/.h``
     - ``CTMCGROUTApp``
     - Класс приложения, ``InitInstance``; ключ реестра — ``"TMC_DN"``
   * - ``MainFrm.cpp/.h``
     - ``CMainFrame``
     - Главное MDI-окно
   * - ``ChildFrm.cpp/.h``
     - ``CChildFrame``
     - Дочернее MDI-окно
   * - ``TMCGROUTDoc.cpp/.h``
     - ``CTMCGROUTDoc``
     - Документ; **+ массив ДН** ``pcDirectionalPattern``
   * - ``TMCGROUTView.cpp/.h``
     - ``CTMCGROUTView``
     - Вид; **+ фоновый расчёт ДН**
   * - ``TMCGROUTDIALOGView.cpp/.h``
     - ``CTMCGROUTDIALOGView``
     - Диалог параметров графика
   * - ``DialogDoc.cpp/.h``
     - ``CDialogDoc``
     - Диалог-таблица документа; **+ кнопки девиаций**
   * - ``TmcGroutColorGraphDialog.cpp/.h``
     - ``CTmcGroutColorGraphDialog``
     - Цвета/тип/толщина 16 линий
   * - ``TmcGrParColTypWid.cpp/.h``
     - ``CTmcGrParColTypWid``
     - Параметры одной линии
   * - ``TmcGrParColTypWid1.cpp/.h``
     - ``CTmcGrParColTypWid1``
     - Пустой диалог-заготовка (см. §10)
   * - ``TMCGrExpression.cpp``
     - ``CTMCGrExpression``
     - Графики-выражения
   * - ``cDeviation.cpp/.h``
     - ``cDeviation``
     - **Диалог внесения случайных отклонений в решётку**
   * - ``DeviatDirPat.cpp/.h``
     - ``CDeviatDirPat``
     - **Диалог расчёта ДН отклонённой решётки vs. требуемой**
   * - ``ThreadCalcDirPat.cpp/.h``
     - ``CThreadCalcDirPat``
     - **Прогресс-окно фонового расчёта ДН**
   * - ``DialogTtoS.cpp/.h``
     - ``CDialogTtoS``
     - Диалог «время → S-матрица» (общий с TMCROS)
   * - ``TmcTtoS.cpp``
     - ``CTmcTtoS``
     - Преобразование T→S (заголовок общий)
   * - ``TmcSMatrix.cpp``
     - ``CTmcSMatrix``
     - Сборка S-матрицы из сигнала (заголовок общий)
   * - ``..\\DiaNapr\\c2DArray.cpp/.h``
     - ``c2DArray``
     - **Ядро: модель решётки и расчёт ДН**
   * - ``..\\DiaNapr\\cIzl.cpp/.h``
     - ``cIzl``
     - **Один излучатель решётки**
   * - ``StdAfx.cpp/.h``
     - —
     - Прекомпилированный заголовок MFC
   * - ``resource.h``, ``TMCGROUT.rc``
     - —
     - Ресурсы

.. note::

   Жирным выделены файлы и классы, **специфичные для TMC_DN**. Класс ``c2DArray``
   подключается через относительный путь ``..\\DiaNapr\\c2DArray.h``.

**Зависимости:** MFC; общие заголовки ``src/Include/`` (``Tmcgrviw.h``, ``Proc_s.h``,
``complex1.h``, ``typedef.h``, ``expr.h``, ``TmcTtoS.h``, ``TmcSMatrix.h``,
``TMCGrExpression.h``); библиотеки **SFILE95**, **complex**, **exprint**
(``i1nte_atof_1``, ``expr_set_angle``, ``expr_get_error`` — вычисление выражения ДН элемента).

----

3. Архитектура расчёта ДН (отличие от TMCGROUT)
-----------------------------------------------

В TMCGROUT график ``i`` берётся из S-файла. В TMC_DN добавлен **параллельный массив
объектов ДН**:

.. code-block:: cpp

   // поле CTMCGROUTDoc
   c2DArray * pcDirectionalPattern;   // массив [grdoc.nGraph] — по объекту ДН на график
   int        nGraphCurrent;          // индекс ДН, которая считается прямо сейчас

Поток данных::

   .dat (описание решётки)
         │  c2DArray::ReadData → cIzl[]            (читает излучатели)
         ▼
   c2DArray::CalculateDiagrNapr                    (pdRealDN[], pdImageDN[] по углам)
         │  + NormirovkaDiagrNapr (КНД/КИП, нормировка)
         ▼
   CTMCGROUTView (рисует как обычный график; амплитуда × GetdMax)

Расчёт запускается в **отдельном потоке** ``ReadDocFileW_Thread1`` (через
``AfxBeginThread``), а модальное окно ``CThreadCalcDirPat`` показывает прогресс по полю
``c2DArray::iN`` (текущий угол) и ``nGraphCurrent`` (текущая ДН).

----

4. Класс ``c2DArray`` — модель решётки и расчёт ДН
-------------------------------------------------------

**Назначение:** хранит описание антенной решётки (набор излучателей ``cIzl``),
рассчитывает её диаграмму направленности по углу, считает КНД/КИП, читает/пишет ``.dat``,
поддерживает девиации и сравнение с требуемой ДН.

**Заголовок:** ``src/viewers/DiaNapr/c2DArray.h``

**Реализация:** ``src/viewers/DiaNapr/c2DArray.cpp``

**Зависит от:** ``cIzl.h``, ``windows.h``, ``typedef.h``, ``expr.h`` (exprint).

Физическая модель (как работает расчёт)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Поле решётки в направлении ``dAngle`` (плоская задача, угол в радианах) — когерентная
сумма по всем излучателям::

   Re = Σ aᵢ·cos( φᵢ + (2π·f·cᵢ/c₀)·cos(dAngle − αᵢ) )
   Im = Σ aᵢ·sin( φᵢ + (2π·f·cᵢ/c₀)·cos(dAngle − αᵢ) )

где для излучателя ``i``: ``aᵢ`` — амплитуда (с учётом собственной ДН элемента и углового
диапазона), ``φᵢ`` — фаза, ``(xᵢ,yᵢ)`` — координаты, ``cᵢ=√(xᵢ²+yᵢ²)``,
``αᵢ=atan2(yᵢ,xᵢ)``, ``f`` — частота (Гц), ``c₀ = 299792458`` м/с. Амплитуда ДН:
``√(Re²+Im²)``, фаза: ``atan2(Im,Re)``.

.. warning::

   Это **научный код расчёта**. Формулы НЕ изменять без разрешения автора.

Поля (private, основные)
~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 30 20 50

   * - Поле
     - Тип
     - Назначение
   * - ``pcIzl``
     - ``cIzl*``
     - Массив излучателей, длина ``nIzl``
   * - ``nIzl``
     - ``int``
     - Число излучателей
   * - ``dFreq``
     - ``double``
     - Рабочая частота, Гц (в файле — в ГГц)
   * - ``szOneExpression``
     - ``char[1000]``
     - Выражение ДН одного элемента (``"1."`` — изотропный)
   * - ``nAngle``
     - ``int``
     - Число угловых точек ДН (400; 1000 для больших решёток)
   * - ``pdAngle``
     - ``double*``
     - Массив углов (рад)
   * - ``pdRealDN``
     - ``double*``
     - Амплитуда ДН по углам (после нормировки в [0..1])
   * - ``pdImageDN``
     - ``double*``
     - Фаза ДН по углам (рад)
   * - ``pdRequiredDN``
     - ``double*``
     - Требуемая ДН (режим сравнения/девиаций)
   * - ``dKnd``
     - ``double``
     - КНД (раз)
   * - ``dKip``
     - ``double``
     - КИП (раз)
   * - ``dMax``
     - ``double``
     - Максимум амплитуды ДН (масштаб)
   * - ``iN``
     - ``int`` (public)
     - Текущий угол — индикатор прогресса для потока
   * - ``bIsReduceElMagnAnalysis``
     - ``BOOL`` (public)
     - Учитывать угловой диапазон ``FiMin..FiMax`` элементов
   * - ``bIsError`` / ``szError``
     - ``BOOL`` / ``char[1000]``
     - Флаг и текст ошибки
   * - ``nMaxTemporary``
     - ``int``
     - Порог «большой решётки» (400): сверх — кэш ДН во временный файл
   * - ``nPointForNormir``
     - ``int``
     - Число точек интегрирования при нормировке/КНД (1000)

Конструктор / деструктор
~~~~~~~~~~~~~~~~~~~~~~~~~~

``c2DArray()``
^^^^^^^^^^^^^^

Инициализирует поля (нет излучателей, ``nAngle=400``, изотропный элемент ``"1."``),
выделяет угловые массивы (``InitAngle``), задаёт режим углов exprint в радианах
(``expr_set_angle(EXPR_RADIAN)``).

``~c2DArray()``
^^^^^^^^^^^^^^^

Удаляет временный и выходной файлы (если создавались), освобождает массивы.

Публичные методы — расчёт и доступ к ДН
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``void CalculateDiagrNapr(char *szInputFileName, char *szOutputFileName)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Полный цикл «из файла в файл»: ``ReadData`` → ``CalculateDiagrNapr()`` → ``WriteData``.

``void CalculateDiagrNapr(void)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Что делает:** рассчитывает ДН для загруженной решётки.

**Как работает:** для каждого угла ``pdAngle[iN]`` вычисляет ``Re``/``Im``
(``CalculateRealDN`` / ``CalculateImageDN``); при изотропном элементе (``"1."``) —
напрямую, иначе домножает на значение выражения элемента (``i1nte_atof_1``); пишет
амплитуду в ``pdRealDN``, фазу в ``pdImageDN``; затем ``NormirovkaDiagrNapr``. Для больших
решёток при актуальном кэше читает ``ReadDataFromTempFile``. Поле ``iN`` обновляется по
ходу — его читает прогресс-окно.

**Ошибки:** при ошибке выражения ``bIsError=TRUE``, текст из ``expr_get_error()``.

Геттеры
^^^^^^^

``GetnAngle`` (число углов), ``GetdAngle(i)`` (угол точки, рад), ``GetdAmplitudeDiagrNapr(i)``
(амплитуда ДН), ``GetdMax`` (масштаб ``dMax``), ``GetKnd`` / ``GetKnd_dB``,
``GetKip`` / ``GetKip_dB``, ``GetnIzl``, ``GetdFreqGHz``,
``IsCalculateKip`` (TRUE для «большой» решётки — тогда вместо КИП «излучённая мощность»),
``IsError`` / ``GetError``.

``void ResizeAngle(double dAngleMin, double dAngleMax)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Перестраивает сетку углов на ``[dAngleMin, dAngleMax]`` (рад), пересчитывает ДН и
перезаписывает выходной файл.

Публичные методы — ввод/вывод и редактирование решётки
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``void ReadData(char *szFileName1)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Загружает решётку из ``.dat``. Формат: ``Frequence = … Ghz;`` · ``N = …;`` ·
строка-выражение ДН элемента · по строке на излучатель
(``i   X = … mm; Y = … mm; Amplitude = …; Faza = … degree; [FiMin = …; FiMax = …; выражение]``).
Координаты → метры, углы → радианы.

``void SaveData(char *pszFile)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Сохраняет только описание решётки (без рассчитанной ДН).

``void SaveDeviationDirectionalPattern(char *pszFileName)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Считает ДН и пишет три колонки: угол (град) · требуемая ДН · рассчитанная ДН.

``void GetIzl(int i, double *dXmm, double *dYmm, double *dAmpl, double *dFazaDegree)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Параметры излучателя ``i``: координаты (мм), амплитуда, фаза (град).

``void SetIzl(int i, double dXcoord_mm, double dYcoord_mm, double dAmplitud, double dFaza_gradus)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Задаёт параметры излучателя ``i``.

Прочие: ``GetRealIzl(i)`` / ``GetImageIzl(i)`` (Re/Im возбуждения = ``a·cos φ`` / ``a·sin φ``),
``SetdFrequence(ГГц)``, ``SetOneExpression`` / ``SetszOneExpression`` / ``GetOneExpression``
(выражение ДН элемента), ``SetNPointForNormirovka``.

Публичные методы — девиации и оптимизация
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``void MakeDeviation(char *szSourceFile, char *szDistinationFile, double dXmm, double dYmm, double dAmplProcent, double dFazaDegree)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Читает решётку из ``szSourceFile``, вносит **случайные** отклонения в каждый излучатель
(координаты ± ``dXmm`` / ``dYmm`` мм, амплитуда ± ``dAmplProcent`` %, фаза ± ``dFazaDegree``
град — равномерное распределение, ``GetRnd``), сохраняет в ``szDistinationFile``.

``void OptimizationInit(char *szFileRequiDirPat, int nIzl1)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Читает файл **требуемой ДН** (угол в градусах, амплитуда) в ``pdRequiredDN``, выделяет
решётку на ``nIzl1`` излучателей. Используется в ``CDeviatDirPat``.

``double GetOptimPurposeFunction(void)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Целевая функция: пересчитывает ДН и возвращает ``Σ|pdRealDN[i] − pdRequiredDN[i]|``.

Ключевые приватные методы
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 45 55

   * - Метод
     - Назначение
   * - ``CalculateRealDN(dAngle)`` / ``CalculateImageDN(dAngle)``
     - Re/Im поля решётки в направлении ``dAngle`` (см. «Физическая модель»)
   * - ``CalculateRealDN_ForKip`` / ``CalculateImageDN_ForKip``
     - То же при единичных амплитудах и нулевой фазе (для КИП)
   * - ``NormirovkaDiagrNapr``
     - Нормировка ДН на максимум; интегрирование для КНД и КИП
   * - ``InitAngle`` / ``DeleteAngle``
     - Выделить/освободить угловые массивы; сетка ``0..π`` (``−π..π`` для большой решётки)
   * - ``InitData``
     - Выделить ``pcIzl[nIzl]``, обнулить ДН
   * - ``ReadData()`` / ``WriteData()``
     - Внутренние чтение/запись (по ``szFileName`` / ``szFileNameOut``)
   * - ``GetRnd(dMedium, dDelta)``
     - Равномерное случайное в ``[dMedium−dDelta, dMedium+dDelta]``
   * - ``IsDirectPatSaveInFile`` / ``WriteTemporaryFile`` / ``ReadDataFromTempFile``
     - Кэш ДН во временном файле для больших решёток
   * - ``LastWriteTime(szFileName)``
     - Время последней записи файла

----

5. Класс ``cIzl`` — один излучатель решётки
-------------------------------------------

**Назначение:** хранит параметры одного излучателя и вычисляет его собственную
(элементную) диаграмму направленности.

**Заголовок:** ``src/viewers/DiaNapr/cIzl.h`` · **Реализация:** ``cIzl.cpp`` ·
**Зависит от:** ``typedef.h``, ``expr.h`` (exprint).

Поля (private)
~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 30 20 50

   * - Поле
     - Тип
     - Назначение
   * - ``dAmplituda``
     - ``double``
     - Амплитуда возбуждения
   * - ``dFaza``
     - ``double``
     - Фаза возбуждения (рад)
   * - ``dXcoordinata`` / ``dYcoordinata``
     - ``double``
     - Координаты (м)
   * - ``dFimin`` / ``dFimax``
     - ``double``
     - Угловой диапазон видимости (рад)
   * - ``csEmitterDirectPat``
     - ``CString``
     - Выражение собственной ДН элемента
   * - ``nTypeEmitterDirectPat``
     - ``int``
     - Тип ДН: ALL/SIN/COS/ARB

Константы типа ДН элемента
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 40 15 45

   * - Константа
     - Значение
     - Смысл
   * - ``TMC_IZL_DIRPATTYPE_ALL``
     - 0
     - Изотропный (``"1."``)
   * - ``TMC_IZL_DIRPATTYPE_SIN``
     - 1
     - ``sin(f)``
   * - ``TMC_IZL_DIRPATTYPE_COS``
     - 2
     - ``cos(f)``
   * - ``TMC_IZL_DIRPATTYPE_ARB``
     - −1
     - Произвольное выражение (exprint)

Методы
~~~~~~

``double GetdAmplituda(double dAngle)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Амплитуда элемента в направлении ``dAngle``:
``dGetDirectionalPatternEmitter(dAngle) · dAmplituda``. При нулевой амплитуде — 0.

``double dGetDirectionalPatternEmitter(double dAngle)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Значение собственной ДН элемента: вне ``[Fimin,Fimax]`` → ``FLT_MIN``; ALL→1,
SIN → ``sin``, COS → ``cos``; произвольное — через ``i1nte_atof_1``.

``BOOL bIsAngleInDiapazon(double dAngle)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

TRUE, если угол в диапазоне видимости (с приведением через ``dAnglePrived`` / ``dAnglePrived1``).

Прочее: сеттеры/геттеры всех полей; ``operator=`` (полное копирование); приватные
``dAnglePrived`` (привод к ``(−π,π]``) / ``dAnglePrived1`` (к ``(0,2π]``).

----

6. Дополнения в ``CTMCGROUTDoc`` (отличия от TMCGROUT)
----------------------------------------------------------

Документ TMC_DN — ``CTMCGROUTDoc`` (как в TMCGROUT, см. :doc:`tmcgrout`, раздел 7), с
добавлениями для ДН:

.. list-table::
   :header-rows: 1
   :widths: 30 25 45

   * - Поле
     - Тип
     - Назначение
   * - ``pcDirectionalPattern``
     - ``c2DArray*``
     - Массив объектов ДН (по одному на график)
   * - ``nGraphCurrent``
     - ``int``
     - Индекс рассчитываемой ДН (для прогресс-окна)

В ``TMCGROUTDoc.h`` объявлены DN-варианты структур ``TMC_GR_DOC1_DN`` и ``TMC_GR_DOC_DN``
(повторяют ``TMC_GR_DOC1`` / ``TMC_GR_DOC``, см. :doc:`tmcgrout`, раздел 3, с добавленным
полем ``bIsReduceElMagnAnalysis``). Поле ``grdoc`` имеет тип ``TMC_GR_DOC_DN``.

.. warning::

   Назначение разделения на ``_DN``-варианты vs. базовые структуры требует уточнения у
   автора (см. §11).

Остальное (чтение/запись ``.soc``, параметры графиков и оформления) — идентично TMCGROUT.

----

7. Дополнения в ``CTMCGROUTView`` (фоновый расчёт ДН)
---------------------------------------------------------

Вид TMC_DN — ``CTMCGROUTView`` (общая часть — :doc:`tmcgrout`, раздел 8). Отличия:

- поле ``ltime_start`` (``time_t``) — момент старта расчёта ДН;
- ``void ReadDocFileW_Thread(void)`` — перечитывает документ и пересоздаёт буферы точек
  **из фонового потока**, в конце ``bIsDataCalculate=TRUE`` и перерисовка; обёртка потока
  ``UINT ReadDocFileW_Thread1(LPVOID pParam)``;
- ``void OnEditTosmatrix()`` — команда меню, открывает диалог ``CDialogTtoS``;
- амплитуда ДН в отрисовке масштабируется: ``pSmatr[j].x · pcDirectionalPattern[i].GetdMax()``;
  в строке состояния (``PutStatistics1``) выводятся КНД/КИП (или «Emitted Power» для больших
  решёток);
- ``void OnDestroy()`` — перед закрытием **дожидается** завершения фонового расчёта: пока
  ``bIsDataCalculate==FALSE``, повторно показывает ``CThreadCalcDirPat``.

----

8. Класс ``CThreadCalcDirPat`` — прогресс-окно расчёта ДН
-------------------------------------------------------------

**Назначение:** модальное окно с двумя прогресс-барами, показывающее ход фонового расчёта
диаграмм направленности.

**Заголовок:** ``ThreadCalcDirPat.h`` · **Базовый класс:** ``CDialog`` · **Ресурс:**
``IDD_READDATA_THREAD``.

.. list-table::
   :header-rows: 1
   :widths: 30 25 45

   * - Поле
     - Тип
     - Назначение
   * - ``pCView``
     - ``void*``
     - Указатель на ``CTMCGROUTView`` (источник прогресса)
   * - ``m_Progress`` / ``m_ProgressAll``
     - ``CProgressCtrl``
     - Прогресс текущей ДН / по всем ДН
   * - ``m_csStatistics`` / ``m_csStatisticsAll``
     - ``CString``
     - Процент готовности (текущая / общая)
   * - ``m_CurrentDirPat`` / ``m_NAllDirPat``
     - ``int``
     - Номер текущей / общее число ДН
   * - ``m_nEmitters``
     - ``int``
     - Число излучателей текущей ДН
   * - ``m_csElapsTime``
     - ``CString``
     - Прошедшее время (сек / мм:сс / чч:мм:сс)

**Методы:** ``OnInitDialog`` (таймер 100 мс); ``OnTimer`` (читает ``c2DArray::iN`` и
``nGraphCurrent``, обновляет проценты/бары/время; по завершении — ``EndDialog(0)``);
``DoModal`` (фиксирует ``ltime_start`` и показывает диалог).

----

9. Диалоги девиаций (специфичны для TMC_DN)
-------------------------------------------

9.1. Класс ``cDeviation`` — внесение случайных отклонений
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Заголовок:** ``cDeviation.h`` · **Ресурс:** ``IDD_DIALOGMAKEDEVIAT``.

**Назначение:** взять файл-источник решётки и создать файл со **случайно отклонёнными**
параметрами излучателей.

**Поля (DDX):** ``m_csFileFrom`` (исходный ``.dat``), ``m_csFileTo`` (выходной), ``m_dXdev``
/ ``m_dYdev`` (разброс координат, мм), ``m_dAmpDev`` (разброс амплитуды, %), ``m_dFazaDev``
(разброс фазы, град).

**Методы:** ``OnButtonsource`` / ``OnButtondistination`` (выбор файлов; вторая запускает
расчёт), ``OnOK``, приватный ``MakeDeviation`` (вызывает ``c2DArray::MakeDeviation``).

9.2. Класс ``CDeviatDirPat`` — ДН отклонённой решётки vs. требуемой
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Заголовок:** ``DeviatDirPat.h`` · **Ресурс:** ``IDD_DIALOGCALCDEVIAT``.

**Назначение:** рассчитать ДН для распределения амплитуд/фаз и сохранить её рядом с
требуемой ДН (файл сравнения ``.dev``).

**Поля (DDX):** ``m_csDistrAmplFaza`` (``.dat`` распределения), ``m_csRequireDirPat``
(``.opt`` требуемой ДН), ``m_csFileDevDirPat`` (выходной ``.dev``).

**Методы:** ``OnButtondistramplfaza`` / ``OnButtonrequrdirectpattern`` /
``OnButtondeviatdirpattern`` (выбор файлов; последняя запускает расчёт), ``OnOK``,
приватный ``CalculateDeviationDirectionalPattern`` (``ReadData`` → ``OptimizationInit`` →
``GetIzl`` / ``SetIzl`` → ``SaveDeviationDirectionalPattern``).

9.3. Дополнения в ``CDialogDoc``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Диалог-таблица документа ``CDialogDoc`` (см. :doc:`tmcgrout`, §9.2) в TMC_DN получает две
кнопки: ``OnAddMakedeviation`` (открывает ``cDeviation``) и ``OnAddMakedeviation2``
(открывает ``CDeviatDirPat``). Также добавлено поле ``dTUnit`` (``double``).

----

10. Утилита «время → S-матрица» и прочие классы
-----------------------------------------------

10.1. ``CDialogTtoS``, ``CTmcTtoS``, ``CTmcSMatrix``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Подсистема **общая с TMCROS**, подробно — в :doc:`tmcros`. Кратко:

- ``CTmcSMatrix`` (``TmcSMatrix.h/.cpp``) — читает ``.t``-сигнал, строит элементы
  S-матрицы (``ReadTFile``, ``MakeSmatrix``, ``Save``).
- ``CTmcTtoS`` (``TmcTtoS.h/.cpp``) — обёртка: ``TtoS(csTfn, csSfn, dTmin, dTmax,
  dFreq)`` — полный цикл «сигнал → S-файл».
- ``CDialogTtoS`` (``DialogTtoS.h/.cpp``, ресурс ``IDD_DIALOG3``) — диалог настройки,
  вызывается из ``CTMCGROUTView::OnEditTosmatrix``.

10.2. ``CTmcGrParColTypWid1``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Заголовок:** ``TmcGrParColTypWid1.h`` · **Ресурс:** ``IDD_DIALOGGRLINECOLORTYPEWIDTH1``.
Пустой диалог-заготовка (нет полей и обработчиков). Назначение неясно — вероятно, мёртвый
код/дубль ``CTmcGrParColTypWid``.

10.3. Общие с TMCGROUT классы
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``CTMCGROUTApp``, ``CMainFrame``, ``CChildFrame``, ``CTMCGROUTDIALOGView``,
``CTmcGroutColorGraphDialog``, ``CTmcGrParColTypWid``, ``CTMCGrExpression``, глобальные
``PutTrace`` / ``PutStatistics`` — см. :doc:`tmcgrout`, разделы 4–6, 9–11. Единственное
отличие приложения — ключ реестра ``"TMC_DN"`` вместо ``"TMCGROUT"`` в ``InitInstance``.

----

11. Требует уточнения у автора (К.Н. Климов)
--------------------------------------------

----

12. Нужные иллюстрации
----------------------

- 📸 НУЖНА СХЕМА: поток данных ``«.dat решётка» → c2DArray (cIzl[]) → CalculateDiagrNapr →
  pdRealDN/pdImageDN → CTMCGROUTView`` и роль ``pcDirectionalPattern[]``.
- 📸 НУЖНА СХЕМА: фоновый расчёт —
  ``OnInitialUpdate → AfxBeginThread(ReadDocFileW_Thread1) → CThreadCalcDirPat``.
- 📸 НУЖНА СХЕМА: геометрия суммирования полей решётки (координаты ``x,y``, угол ``dAngle``,
  фазовый набег ``2π·f·c·cos(dAngle−α)/c₀``).
- Скриншоты окон относятся к руководству пользователя, а не к API-документации.
