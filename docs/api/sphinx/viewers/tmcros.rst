Программа TMCROS — API-документация
===================================

.. note::

   Пакет **TMC Suite**. Вьювер **временны́х** характеристик (сигналов во временно́й
   области) и инструмент преобразования «время → матрица рассеяния» (Time-to-Scattering).
   Язык документации: русский. Все сигнатуры приведены по исходникам из ``src/viewers/Tmcrtout/``
   и общим заголовкам из ``src/Include/``.

----

1. Назначение программы
-----------------------

**TMCROS** (выходной EXE — ``TMCROS.exe``, проект ``TMCROS.vcxproj``) — графический
просмотрщик результатов электродинамического расчёта во **временно́й области**: строит
**2D-графики временны́х сигналов** многополюсника, считанных из выходных файлов сигнала
(``.t`` — Rt-Signal), и умеет на их основе вычислять **матрицу рассеяния** на заданных
частотах (преобразование «время → S-матрица», подсистема **TtoS**).

Программа построена по той же схеме **MFC Document/View** (приложение типа MDI), что и
родственный частотный вьювер **TMCGROUT**, и разделяет с ним большую часть исходных файлов
и модель данных. Ключевые отличия TMCROS:

- **ось X — время** (n, с, мс, мкс, нс, пс), а не частота;
- **ось Y — временно́й сигнал** и производные от него величины (амплитудная огибающая,
  КСВН/потери/фаза, вычисляемые по огибающей);
- данные читаются из **файлов сигнала** ``.t`` (а не из S-файлов): функция
  ``read_RT_output_signal`` в ``CTMCGROUTDoc``;
- документ имеет расширение ``.tos`` и сигнатуру ``#TMCGROTS`` (у TMCGROUT — ``.soc`` / ``#TMCGROUT``);
- добавлена **подсистема Time-to-Scattering**: классы ``CTmcSMatrix`` , ``CTmcTtoS`` и диалог
  ``CDialogTtoS`` , преобразующие набор временны́х сигналов в один S-файл (``.s``) на заданных
  частотах (пункт меню «To S-matrix»);
- в виде заведён **второй буфер точек** ``tmcgrwin1`` (рабочая копия для отрисовки, см. §8).

.. warning::

   ⚠️ TMCROS позиционируется как **временно́й (промежуточный) вьювер**: он показывает сигналы
   и готовит из них S-матрицу, которую затем смотрят в TMCGROUT.

**Совпадение имён файлов с TMCGROUT.** Файлы ``TMCGROUT.cpp/.h`` , ``MainFrm`` , ``ChildFrm`` ,
``TMCGROUTDoc`` , ``TMCGROUTView`` , ``TMCGROUTDIALOGView`` , ``DialogDoc`` ,
``TmcGroutColorGraphDialog`` , ``TmcGrParColTypWid`` , ``TMCGrExpression`` называются так же,
как в TMCGROUT (исторически TMCROS сделан копированием проекта TMCGROUT), но **содержат другую
логику** в части временны́х данных и TtoS. Ниже то, что действительно идентично TMCGROUT,
описано кратко со ссылкой; отличия документированы полно.

----

2. Состав проекта
-----------------

В сборку ``TMCROS.vcxproj`` входят следующие файлы (по ``<ClCompile>`` / ``<ClInclude>``):

.. list-table::
   :header-rows: 1
   :widths: 25 30 45

   * - Файл
     - Класс / содержимое
     - Назначение (кратко)
   * - ``TMCGROUT.cpp/.h``
     - ``CTMCGROUTApp``
     - Класс приложения; ``InitInstance`` ; диалог «О программе». Отличие от TMCGROUT — ключ реестра ``TMCROS``
   * - ``MainFrm.cpp/.h``
     - ``CMainFrame``
     - Главное MDI-окно (тулбар, статусбар). Аналогично TMCGROUT
   * - ``ChildFrm.cpp/.h``
     - ``CChildFrame``
     - Дочернее MDI-окно. Аналогично TMCGROUT
   * - ``TMCGROUTDoc.cpp/.h``
     - ``CTMCGROUTDoc``
     - Документ: чтение/запись ``.tos`` , чтение **временны́х сигналов** ``.t`` , параметры графиков
   * - ``TMCGROUTView.cpp/.h``
     - ``CTMCGROUTView``
     - Вид: отрисовка временны́х графиков, мышь, меню; пункт «To S-matrix»
   * - ``TMCGROUTDIALOGView.cpp/.h``
     - ``CTMCGROUTDIALOGView``
     - Диалог параметров графика (единицы времени, тип Y, пределы, точки)
   * - ``DialogDoc.cpp/.h``
     - ``CDialogDoc``
     - Диалог-таблица содержимого документа (до 20 графиков)
   * - ``DialogTtoS.cpp/.h``
     - ``CDialogTtoS``
     - **TtoS-диалог**: список ``.t`` -сигналов, частоты, экспорт в S-файл
   * - ``TmcSMatrix.cpp`` (+ ``src/Include/TmcSMatrix.h``)
     - ``CTmcSMatrix``
     - **TtoS-ядро**: чтение ``.t`` , восстановление S-матрицы из временно́го сигнала, запись ``.s``
   * - ``TmcTtoS.cpp`` (+ ``src/Include/TmcTtoS.h``)
     - ``CTmcTtoS``
     - **TtoS-обёртка**: один вызов «файл ``.t`` → файл ``.s`` »
   * - ``TmcGroutColorGraphDialog.cpp/.h``
     - ``CTmcGroutColorGraphDialog``
     - Диалог цвета/типа/толщины 16 линий. Аналогично TMCGROUT
   * - ``TmcGrParColTypWid.cpp/.h``
     - ``CTmcGrParColTypWid``
     - Диалог параметров одной линии. Аналогично TMCGROUT
   * - ``TMCGrExpression.cpp`` (+ ``src/Include/TMCGrExpression.h``)
     - ``CTMCGrExpression``
     - Графики-выражения над несколькими файлами. Аналогично TMCGROUT
   * - ``StdAfx.cpp/.h``
     - —
     - Прекомпилированный заголовок MFC
   * - ``resource.h`` , ``TMCGROUT.rc``
     - —
     - Ресурсы (меню, диалоги, иконки, тулбар)

**Файл** ``TmcGrParColTypWid1.cpp/.h`` **(класс** ``CTmcGrParColTypWid1`` **)** физически лежит
в каталоге, но **не включён** в ``TMCROS.vcxproj`` (отсутствует в ``<ClCompile>`` / ``<ClInclude>`` ) —
в сборке не участвует. См. §9.5.

**Зависимости и настройки сборки** (``TMCROS.vcxproj``):

- ``UseOfMfc = Static`` , ``CharacterSet = MultiByte`` , ``PlatformToolset = v145`` ;
  конфигурации Win32 и x64 (Debug/Release);
- линковка с библиотеками пакета: **SFILE95** (``sfile95.lib`` — запись S-матрицы
  ``save_S_matrix_element`` ), **complex** (``complex.lib`` ), **exprint** (``exprint.lib`` —
  вычисление выражений);
- выходной каталог из ``build\ExeOutput.props`` → ``dist\<platform>\bin`` ;
- общие заголовки из ``src/Include/`` : ``tmcgrviw.h`` , ``proc_s.h`` , ``s_file.h`` ,
  ``typedef.h`` , ``complex1.h`` , ``TmcSMatrix.h`` , ``TmcTtoS.h`` , ``TMCGrExpression.h`` .

----

3. Общие структуры данных (заголовок ``Tmcgrviw.h``)
----------------------------------------------------

TMCROS использует **те же** структуры модели данных, что и TMCGROUT (``TMC_GR_DOC1`` ,
``TMC_GR_DOC`` , ``TMC_GR_VIEW`` , ``TMC_GR_WINDOW`` , ``TMC_GR_TYPE_X/Y`` ). Полное описание
полей — см. ``tmcgrout.md`` , §3. Здесь зафиксированы **отличия в трактовке полей** и
**константы типов, специфичные для временно́го вьювера**.

3.1. Отличие трактовки полей ``TMC_GR_DOC1``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

В TMCROS поля структуры графика читаются из **файла сигнала** ``.t`` , поэтому их физический
смысл другой:

.. list-table::
   :header-rows: 1
   :widths: 20 20 60

   * - Поле
     - Тип
     - Трактовка в TMCROS
   * - ``pFreq``
     - ``double*``
     - Массив **отсчётов времени** (а не частот): значения времени из ``.t`` -файла
   * - ``pSmatr``
     - ``_complex*``
     - Массив значений сигнала: ``pSmatr[j].x`` — **отсчёт временно́го сигнала** для выбранного входа/моды (мнимая часть ``.y`` не используется)
   * - ``dFreq``
     - ``double``
     - **Частота** (Гц), считанная из строки ``#TMC_GROTS_Freq …`` файла ``.t`` ; используется при восстановлении фазы/амплитуды и в TtoS
   * - ``nInp1`` , ``nMod1``
     - ``int``
     - Номер входа и моды (столбца сигнала) в ``.t`` -файле; нормируются в ``read_RT_output_signal`` ( ``in1≥1`` , ``mod1∈{0,1}`` )
   * - ``nInp2`` , ``nMod2``
     - ``int``
     - Второй адрес; в ``read_RT_output_signal`` принудительно ``in2=1`` , ``mod2∈{0,1}``
   * - ``szFileName``
     - ``char*``
     - Полный путь к ``.t`` -файлу сигнала

Остальные поля (``nPoint`` , ``szGrapName`` , ``szGrapPodp`` , ``LineColor/Type/Width`` ,
``piPoint[10]`` , ``PointColor/Type/Width`` , ``OutFlag`` , ``LastWriteTime`` , ``pcExpr`` ) —
как в TMCGROUT.

3.2. Константы типа оси X (время) — ``TMC_GROTS_TYPE_*``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 35 15 50

   * - Константа
     - Значение
     - Смысл (ось X)
   * - ``TMC_GROTS_TYPE_nT``
     - 0
     - Время в **отсчётах** (номер точки)
   * - ``TMC_GROTS_TYPE_ps``
     - 1
     - Время, пс (×1e−12)
   * - ``TMC_GROTS_TYPE_ns``
     - 2
     - Время, нс (×1e−9)
   * - ``TMC_GROTS_TYPE_mks``
     - 3
     - Время, мкс (×1e−6)
   * - ``TMC_GROTS_TYPE_ms``
     - 4
     - Время, мс (×1e−3)
   * - ``TMC_GROTS_TYPE_s``
     - 5
     - Время, с (×1)

3.3. Константы типа оси Y — ``TMC_GROTS_TYPE_*``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 35 15 50

   * - Константа
     - Значение
     - Смысл (ось Y)
   * - ``TMC_GROTS_TYPE_FULL``
     - 0
     - **Сигнал** — отсчёт сигнала как есть (``pSmatr[j].x`` )
   * - ``TMC_GROTS_TYPE_AMPL``
     - 1
     - **Амплитуда** (огибающая ``|S|`` , восстановленная по локальным экстремумам сигнала)
   * - ``TMC_GROTS_TYPE_PHAS_R``
     - 2
     - Фаза, радианы
   * - ``TMC_GROTS_TYPE_PHAS_G``
     - 3
     - Фаза, градусы
   * - ``TMC_GROTS_TYPE_WSVR``
     - 4
     - КСВН (по амплитудной огибающей)
   * - ``TMC_GROTS_TYPE_LDB``
     - 5
     - Потери, дБ (``−20·lg|огибающая|`` )
   * - ``TMC_GROTS_TYPE_PHASD``
     - 6
     - «Дельта-фаза» (приращение фазы)

.. note::

   Массив типов в ``CTMCGROUTView`` (``gr_typY`` ) предъявляет пользователю 7 пунктов: Signal,
   Amplitude, WSVR, L dB, Phase in radian, Phase in gradus, Delta Phase (плюс терминатор).

.. warning::

   ⚠️ Назначение/корректность пунктов ``PHAS_R`` , ``PHAS_G`` , ``PHASD`` для временно́го сигнала
   требует уточнения (см. §13).

3.4. Константы файла/документа — ``TMC_GROTS_*``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 35 30 35

   * - Константа
     - Значение
     - Смысл
   * - ``TMC_GROTS_DOCFILE_ID``
     - ``"#TMCGROTS"``
     - Сигнатура файла документа ``.tos`` и имя секции в реестре ``HKCU``
   * - ``TMC_GROTS_DOCFILE_XAXIESFORMAT``
     - ``"#TMCGROTS_X_AXIES_FORMAT"``
     - Ключ формата оси X
   * - ``TMC_GROTS_DOCFILE_YAXIESFORMAT``
     - ``"#TMCGROTS_Y_AXIES_FORMAT"``
     - Ключ формата оси Y
   * - ``TMC_GROTS_DOCFILE_POINTSIZE``
     - ``"#TMCGROTS_POINTSIZE"``
     - Ключ размера маркера точки
   * - ``TMC_GROTS_DOCFILE_ANGLE_TETA``
     - ``"#TMCGROTS_ANGLE_TETA"``
     - Ключ (угол θ) — в TMCROS, по-видимому, не задействован
   * - ``TMC_GROTS_FILEOUT_ID``
     - ``"#TMC_GraphicsOutputSignalFile FormVer2.0 2000 "``
     - Сигнатура файла сигнала ``.t``
   * - ``TMC_GROTS_FLOUTFreq_ID``
     - ``"#TMC_GROTS_Freq "``
     - Префикс строки с частотой в ``.t`` -файле
   * - ``TMC_GROTS_FLOUTInpNum_ID``
     - ``"#TMC_GROTS_InpNum "``
     - Префикс строки с числом входов в ``.t`` -файле

Прочие константы (``TMC_GROUT_MAXSTRING_BUF`` , ``TMC_GROUT_POINT_TYPE`` ,
``TMC_VIEW_XSIZE/YSIZE`` , ``TMC_VIEW_SIZEMODE`` , идентификаторы окна
``TMC_GROUT_DOCFILE_ID_INIWND`` / ``…_INIWNDVWPRT`` ) — общие с TMCGROUT.

----

4. Класс ``CTMCGROUTApp``
-------------------------

**Назначение:** класс приложения MFC. **Идентичен** TMCGROUT по структуре (см. ``tmcgrout.md`` ,
§4), с единственным отличием:

- ``InitInstance()`` вызывает ``SetRegistryKey("TMCROS")`` (а не ``"TMCGROUT"`` ) — настройки и
  список последних файлов (MRU) хранятся в ``HKCU\…\TMCROS`` .

Остальное: ``AfxOleInit()`` , ``Enable3dControls()/Static`` , ``LoadStdProfileSettings(9)`` ,
регистрация ``CMultiDocTemplate`` (``CTMCGROUTDoc`` + ``CChildFrame`` + ``CTMCGROUTView`` , ресурс
``IDR_TMCGROTYPE`` ), автооткрытие первого существующего файла из MRU при пустой командной строке,
восстановление положения окна из профиля (``#INIWND`` ), показ окна ``SW_RESTORE`` .

**Заголовок:** ``TMCGROUT.h`` · **Базовый класс:** ``CWinApp`` · **Глобальный объект:**
``CTMCGROUTApp theApp;``

**Вложенный класс** ``CAboutDlg`` (в ``TMCGROUT.cpp`` ) — диалог «О программе».

----

5. Класс ``CMainFrame``
-----------------------

**Идентичен** TMCGROUT (см. ``tmcgrout.md`` , §5). Главное MDI-окно с тулбаром и статусбаром из
2 панелей; ``OnCreate`` , ``PreCreateWindow`` (``WS_MAXIMIZE`` ), ``OnDestroy`` (сохранение позиции
окна в реестр под ``#INIWND`` ).

**Заголовок:** ``MainFrm.h`` · **Базовый класс:** ``CMDIFrameWnd`` .

----

6. Класс ``CChildFrame``
------------------------

**Идентичен** TMCGROUT (см. ``tmcgrout.md`` , §6). Дочернее MDI-окно, по умолчанию развёрнуто
(``WS_MAXIMIZE`` ).

**Заголовок:** ``ChildFrm.h`` · **Базовый класс:** ``CMDIChildWnd`` .

----

7. Класс ``CTMCGROUTDoc`` — документ
------------------------------------

**Назначение:** модель данных документа: читает/пишет ``.tos`` -файл, загружает **временны́е
сигналы** из ``.t`` -файлов, хранит параметры графиков и оформления.

**Заголовок:** ``TMCGROUTDoc.h`` · **Базовый класс:** ``CDocument`` · **Зависит от:**
``Tmcgrviw.h`` , ``proc_s.h`` , SFILE95.

Структура класса (поля и большинство методов) **совпадает с TMCGROUT** (см. ``tmcgrout.md`` , §7).
Ниже — **отличия**, специфичные для временно́го вьювера.

7.1. Дополнительное поле
~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 20 20 60

   * - Поле
     - Тип
     - Назначение
   * - ``dFreq``
     - ``double``
     - Частота (Гц), считанная из последнего обработанного ``.t`` -файла (поле-«черновик», используется внутри ``read_RT_output_signal`` )

Прочие поля (``grdoc`` , ``sGrDoc[16]`` , ``bLossPoint`` , цвета
``scBackgoundColor/scGridColor/scAxiesColor/scTextColor/scPointColor`` , ``lfInitial`` ,
``csEditorName`` , ``error[300]`` , ``piProcInfo`` ) — как в TMCGROUT.

**Примечание:** в TMCROS-версии заголовка отсутствуют поля ``dXAxiesUserUnit`` /
``csXAxiesUserName`` (пользовательская единица оси X не предусмотрена — ось X всегда время).

7.2. Чтение данных — ключевое отличие
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Вместо чтения S-файлов через ``read_S_matrix_element`` (как в TMCGROUT) TMCROS читает
**временно́й сигнал** собственной функцией:

``void read_RT_output_signal(int *in1, int *mod1, int *in2, int *mod2, char *filename, _complex **sss, double **f1, int *nPoint, double *dFr)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Что делает:** читает файл сигнала ``.t`` и заполняет массив времени ``*f1`` и массив отсчётов
сигнала ``*sss`` .

**Как работает:**

1. нормирует адрес: ``*in1≥1`` , ``*mod1∈{0,1}`` , ``*in2=1`` , ``*mod2∈{0,1}`` ;
2. первый проход — считает число строк данных (строки, начинающиеся с пробела) → ``nPoint`` ;
   параллельно ищет строку ``#TMC_GROTS_Freq …`` и читает из неё частоту в ``*dFr`` (по умолчанию ``1e10`` );
3. выделяет память ``sss[n]`` , ``f1[n]`` ;
4. второй проход — по каждой строке данных: ``f1[i] = gets_f1(ch)`` (время),
   ``sss[i].x = gets_sss(ch, in1, mod1)`` (отсчёт сигнала для входа ``in1`` , компонента ``mod1`` ).

.. list-table::
   :header-rows: 1
   :widths: 25 20 55

   * - Параметр
     - Тип
     - Назначение
   * - ``in1, mod1, in2, mod2``
     - ``int*``
     - Адрес входа/моды (на входе нормируются)
   * - ``filename``
     - ``char*``
     - Путь к ``.t`` -файлу
   * - ``sss``
     - ``_complex**``
     - Выход: массив отсчётов сигнала (``.x`` )
   * - ``f1``
     - ``double**``
     - Выход: массив времени
   * - ``nPoint``
     - ``int*``
     - Выход: число точек (0 — ошибка/файл не открыт)
   * - ``dFr``
     - ``double*``
     - Выход: частота из файла

**Ошибки:** при невозможности открыть файл или пустых данных ``*nPoint=0`` ; вызывающий
``ReadGraph1`` пишет в подпись ``Error_when_read_<имя>`` .

Вспомогательные методы парсинга ``.t`` -строки
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Метод
     - Назначение
   * - ``double gets_f1(char *ch)``
     - Читает время из строки: ``sscanf(ch,"%d%lg",&i,&r1)`` → возвращает ``r1`` (второе число строки)
   * - ``double gets_sss(char *ch, int in1, int mod1)``
     - Возвращает отсчёт сигнала для входа ``in1`` : пропускает поля до нужного входа (``skip_ntdt_itd`` ), затем ``sscanf "%d%lg%lg"`` ; при ``mod1==0`` → ``r1`` , при ``mod1==1`` → ``r2``
   * - ``int skip_ntdt_itd(char *ch, int in1)``
     - Возвращает смещение в строке до данных входа ``in1`` (пропускает номер точки, время и ``(in1−1)`` троек чисел предыдущих входов)
   * - ``int skip_blank(char *ch)`` / ``int skip_number(char *ch)``
     - Пропуск пробелов/числа (как в TMCGROUT)

``void ReadGraph1(int nGr, char *szFileName, int *nPoint, double **pFreq, _complex **pSmatr)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Отличие от TMCGROUT:** формирует абсолютный путь файла относительно каталога документа и
вызывает ``read_RT_output_signal`` (а не ``read_S_matrix_element`` ). При ``*nPoint<=0`` пишет
``Error_when_read_<имя>`` в ``szGrapPodp`` .

``void ReadGraph1Expession(int nGr, char *szExpression)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Аналогично TMCGROUT**, но переменные-«файлы» в выражении — это ``.t`` -сигналы (читаются через
``ReadGraph1`` → ``read_RT_output_signal`` ). Проверяет совпадение временны́х сеток
(``IsFreqCorrect`` ).

7.3. Имя и формат документа — отличия
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 45 55

   * - Метод
     - Отличие от TMCGROUT
   * - ``void SFileNameToDatFileName(char *ch)``
     - Формирует имя документа с расширением ``.tos`` (если такой уже есть — ``.$os`` ), а не ``.soc`` / ``.$OC``
   * - ``BOOL IsDocFileName(char *ch)``
     - Проверяет, что первая строка файла = сигнатура ``#TMCGROTS``
   * - ``void MakeDocFileName(void)``
     - Формирует имя ``.tos`` -документа
   * - ``WriteDocFile()`` / ``ReadGraphParameters()`` / ``SetOneReadGraphParameters()``
     - Пишут/читают секцию ``#TMCGROTS`` (в т. ч. в реестр ``HKCU\…\#TMCGROTS`` ); типы осей трактуются как время/сигнал

7.4. Остальные методы
~~~~~~~~~~~~~~~~~~~~~~

Совпадают с TMCGROUT (см. ``tmcgrout.md`` , §7): ``ReadDocFile`` , ``ReadGraphParameters`` ,
``ReadGraphParametersDefault`` , ``WriteDocFile`` , ``AddGraphicsInDoc`` , ``IsDataModific`` ,
``SetGraphColor`` , ``RunExeFile`` , ``DocFileNewData`` , ``DocFileDelData`` , ``ReadNGraph`` ,
``ReadGraph`` , ``ReadInputModeFromFileName`` (если присутствует), ``SetOneReadGraphParameters`` ,
``SetDefaultLineColorWidthType`` , ``MakeRelatPathSFile`` , ``CalcNDirInPathName`` ,
``SelectDirName`` , ``LastWriteTime`` , ``del_bl2`` , ``del_bl3`` , ``put_error_messege`` ,
``CreatProc`` .

.. warning::

   ⚠️ Инверсная семантика ``WriteDocFile()`` (возврат ``FALSE`` при успехе, ``TRUE`` при ошибке) —
   как в TMCGROUT, требует подтверждения (см. §13).

----

8. Класс ``CTMCGROUTView`` — вид
--------------------------------

**Назначение:** отрисовка временны́х графиков, интерактив (мышь, меню, масштаб, печать) и запуск
TtoS-диалога.

**Заголовок:** ``TMCGROUTView.h`` · **Базовый класс:** ``CScrollView`` · **Зависит от:**
``Tmcgrviw.h`` , ``complex1.h`` , ``DialogTtoS.h`` .

Структура совпадает с TMCGROUT (см. ``tmcgrout.md`` , §8); ниже — **отличия**.

8.1. Дополнительные поля
~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 20 25 55

   * - Поле
     - Тип
     - Назначение
   * - ``tmcgrwin1``
     - ``TMC_GR_WINDOW``
     - **Второй буфер графиков** — рабочая копия точек для отрисовки. Инициализируется по умолчанию (тип X= ``nT`` , тип Y= ``FULL`` , вьюпорт 2000.. ``TMC_VIEW_*SIZE`` ); в ``ReadDocFileW`` пересоздаётся под число графиков, копирует ``nPointDraw`` /координаты из ``tmcgrwin`` ; используется в ``OnDrawGraph1/2/3``
   * - ``gr_typX``
     - ``TMC_GR_TYPE_X*``
     - Массив из **7** типов оси X: ``Time n`` , ``Time sec`` , ``Time msec`` , ``Time mksec`` , ``Time nsec`` , ``Time psec`` + терминатор
   * - ``gr_typY``
     - ``TMC_GR_TYPE_Y*``
     - Массив из **8** типов оси Y: ``Signal`` , ``Amplitude`` , ``WSVR`` , ``L dB`` , ``Phase in radian`` , ``Phase in gradus`` , ``Delta Phase`` + терминатор

.. warning::

   ⚠️ Точное назначение второго буфера ``tmcgrwin1`` (почему отрисовка идёт по копии, а не
   напрямую по ``tmcgrwin`` ) требует уточнения (см. §13). По коду ``OnDrawGraph1/2/3`` рисуют
   именно из ``tmcgrwin1`` .

8.2. Подготовка данных к рисованию — отличия
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``void PrepareDoubleGraph(void)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Что делает:** заполняет вещественные точки ``pPoint[j]`` для каждого видимого графика, исходя
из **временны́х** типов осей.

**Как работает:**

- **X** = время: ``pFreq[j]`` , делённое на множитель единицы (``GetXAxiesUnit`` : с→1, мс→1e3,
  мкс→1e6, нс→1e9, пс→1e12; для ``nT`` — номер точки);
- **Y** — по ``nYType`` :

  - ``TMC_GROTS_TYPE_FULL`` — отсчёт сигнала ``pSmatr[j].x`` как есть;
  - ``TMC_GROTS_TYPE_AMPL`` — амплитудная **огибающая** ``r14`` , восстановленная по локальным
    экстремумам сигнала (отслеживание тройки соседних отсчётов ``r11,r12,r13`` и смены знака производной);
  - ``TMC_GROTS_TYPE_WSVR`` — ``(1+s)/(1−s)`` по огибающей ``s`` (с защитой ``|s|≈1`` );
  - ``TMC_GROTS_TYPE_LDB`` — ``−20·lg(огибающая)`` ;
  - ``TMC_GROTS_TYPE_PHAS_R`` / ``PHAS_G`` — фаза (рад/град), оцениваемая по моментам смены знака
    сигнала и частоте ``dFreq`` ;
  - для графиков-выражений значение берётся из методов ``CTMCGrExpression``
    (``GetExpressionValueL/K/SM`` ).

- параллельно ищет мин/макс X и Y; при включённом авто-масштабе (``nAFlagX/nAFlagY`` ) задаёт
  пределы окна; вызывает ``PutStatistics1()`` .

.. warning::

   ⚠️ Алгоритм восстановления огибающей и фазы из временно́го сигнала — научная логика; смысл и
   корректность отдельных ветвей (особенно ``PHAS_R/PHAS_G/PHASD`` , использование ``dFreq`` )
   требуют уточнения у автора (см. §13). Ветвь ``TMC_GR_TYPE_SFD`` в коде закомментирована.

``void PrepareLogGraph(void)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Переводит вещественные точки в логические (``DoublXCordToLX/Y`` ), считает ``nPointDraw`` ,
прижимает точки вне ``[Xmin,Xmax]`` . После — **копирует** результат в ``tmcgrwin1`` (пересоздавая
его массив под число графиков и ``nPointDraw`` ).

``double GetXAxiesUnit(void)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Возвращает множитель текущей единицы **времени**: с→1, мс→1e3, мкс→1e6, нс→1e9, пс→1e12 (для
``nT`` обрабатывается отдельно). Это отличается от частотного ``GetXAxiesUnit`` в TMCGROUT.

8.3. Отрисовка — отличия
~~~~~~~~~~~~~~~~~~~~~~~~~

- ``OnDrawGraph(CDC*)`` делает 3 прохода (``OnDrawGraph1/2/3`` ), но рисует из ``tmcgrwin1`` (а не
  ``tmcgrwin`` ).
- ``OnDraw`` : фон → (при ошибке — текст и выход) → рамка zoom → сетка → оси → графики →
  ``OnDrawMouseScrol`` . При первом вызове (``FlagResizeInit`` ) подгоняет вьюпорт
  (``OnViewResizectrlr`` ).
- ``onDrowAxies`` / ``onDrawGrid`` / ``OnDrawLine`` / ``OnDrawPoint`` / ``OnDrawBackground`` /
  ``OnDrawMouseResize`` / ``VTextOut`` / ``SetRectOutXY`` / ``IvalidateRectView`` — как в TMCGROUT.

8.4. Новая команда меню — TtoS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``afx_msg void OnEditTosmatrix()`` (команда ``ID_EDIT_TOSMATRIX`` , пункт «To S-matrix»)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Что делает:** открывает диалог ``CDialogTtoS`` для преобразования временны́х сигналов документа
в S-файл.

**Как работает:**

1. передаёт диалогу имя документа (``m_DocFileName = GetPathName()`` ) и текущие пределы по
   времени (``m_dTmin/m_dTmax = tmcgrwin.sGrWin.Xmin/Xmax`` );
2. по ``tmcgrwin.nXType`` задаёт множитель единицы времени ``dlg.dTUnit`` и подпись
   ``m_csTimeUnit`` (ps/ns/mks/ms/s; default — единица, пределы 0..1);
3. для каждого из ≤20 графиков копирует в диалог имя файла-сигнала (``m_SFileName_<k>`` ), флаг
   вывода (``m_OutGrFlag_<k>`` ) и частоту (``m_dXValue<k> = psGraph[k].dFreq`` );
4. показывает диалог; экспорт выполняется внутри диалога (``OnExportCharacteristics`` / ``OnOK`` ).

8.5. Прочие обработчики
~~~~~~~~~~~~~~~~~~~~~~~~

Команды масштаба, мыши, цветов, шрифта, редактора, файла, авто-масштаба, прокрутки Home/End,
таймер (перечитывание при изменении файлов) — **как в TMCGROUT** (см. ``tmcgrout.md`` , §8).
Команда выбора «пользовательской единицы X» в TMCROS отсутствует (ось X — всегда время).

Глобальные функции ``PutTrace`` / ``PutStatistics`` (запись в статусбар) — см. §11.

----

9. Диалоги
----------

9.1. ``CTMCGROUTDIALOGView`` — параметры графика
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Заголовок:** ``TMCGROUTDIALOGView.h`` · **Базовый класс:** ``CDialog`` · **Ресурс:**
``IDD_DIALOG1`` .

Структурно **как в TMCGROUT** (см. ``tmcgrout.md`` , §9.1): пределы X/Y, авто-масштаб, тип X
(радиогруппа единиц **времени**), тип Y (Signal/Amplitude/…), флаг и размер точек, форматы
подписей, имя документа. Отличие — радиогруппа X соответствует единицам времени, а не частоты.

9.2. ``CDialogDoc`` — таблица графиков документа
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Заголовок:** ``DialogDoc.h`` · **Базовый класс:** ``CDialog`` · **Ресурс:** ``IDD_DIALOG2`` .

**Назначение:** редактирование содержимого документа в виде таблицы на 20 строк (имя графика,
имя файла, входы/моды ``np1/mod1`` , флаг вывода). Совпадает с TMCGROUT, но:

- содержит дополнительное поле ``double dTUnit`` — множитель единицы времени;
- семантика входа/моды относится к столбцам **временно́го сигнала**, а не S-матрицы.

**Поля:** ``m_DocFileName`` ; по 20 шт. ``m_GraphName*`` , ``m_SFileName_*`` , ``m_np1_*`` ,
``m_mod1_*`` , ``m_OutGrFlag_*`` ; ``m_AddCharacteristicsFlag`` (BOOL — нажата «добавить»);
``dTUnit`` .

**Методы:** ``OnAddCharacteristics()`` — выставляет ``m_AddCharacteristicsFlag`` и закрывает
диалог; ``OnNotify`` (переопределён).

.. note::

   Поля ``m_np2_*`` / ``m_mod2_*`` в TMCROS-версии ``DialogDoc.h`` отсутствуют (в отличие от
   описания в TMCGROUT) — задаётся только первый адрес ``np1/mod1`` .

9.3. ``CDialogTtoS`` — преобразование «время → S-матрица» (НОВЫЙ, только в TMCROS)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Заголовок:** ``DialogTtoS.h`` · **Базовый класс:** ``CDialog`` · **Ресурс:** ``IDD_DIALOG3`` ·
**Реализация:** ``DialogTtoS.cpp`` .

**Назначение:** собрать до **20 файлов сигнала** ``.t`` , задать для каждого частоту, диапазон
времени ``[Tmin,Tmax]`` и единицу времени, и **экспортировать** результат — восстановленную
S-матрицу — в один S-файл ``.s`` .

**Поля (DDX):**

.. list-table::
   :header-rows: 1
   :widths: 20 15 65

   * - Поле
     - Тип
     - Назначение
   * - ``m_DocFileName``
     - ``CString``
     - Имя файла документа (для построения путей)
   * - ``dTUnit``
     - ``double``
     - Множитель единицы времени (задаётся из вида)
   * - ``m_SFileName``
     - ``CString``
     - Имя **выходного** S-файла (``.s`` ); по умолчанию ``<документ>.s``
   * - ``m_SFileName_1..20``
     - ``CString``
     - Имена входных ``.t`` -файлов сигнала (до 20)
   * - ``m_OutGrFlag_1..20``
     - ``BOOL``
     - Флаг «обрабатывать этот файл»
   * - ``m_dXValue1..20``
     - ``double``
     - **Частота** (Гц) для соответствующего ``.t`` -файла
   * - ``m_dTmin`` , ``m_dTmax``
     - ``double``
     - Диапазон времени (в текущих единицах)
   * - ``m_csTimeUnit``
     - ``CString``
     - Подпись единицы времени (ps/ns/mks/ms/s)
   * - ``m_dXmin`` , ``m_dDx``
     - ``double``
     - Начальная частота и шаг для авто-заполнения частот (по умолчанию 1e10 и 1e8)
   * - ``m_csError``
     - ``CString``
     - Текст последней ошибки преобразования

**Методы:**

- ``INT_PTR DoModal()`` — перед показом достраивает абсолютные пути: имя выходного ``.s`` =
  ``<документ без расширения>.s`` , а каждое непустое ``m_SFileName_<k>`` приводит к виду
  ``<каталог документа>\<имя>`` ;
- ``void OnChangeSfilename()`` — выбор имени выходного ``.s`` -файла через ``CFileDialog`` (фильтр
  «Tamic S-matrix», маска ``*.s`` );
- ``void OnAddCharacteristics()`` — выбор входного ``.t`` -файла (``CFileDialog`` , «Tamic Rt-Signal»,
  маска ``*.t`` ) и запись его в первую свободную ячейку ``m_SFileName_1..20`` ;
- ``void OnChangeSfiledelet()`` — удаляет файл с именем ``m_SFileName`` с диска (``remove`` );
- ``void OnExportFillxvalue()`` — заполняет ``m_dXValue1..20`` арифметической прогрессией:
  ``m_dXmin, +m_dDx, +2·m_dDx, …`` ;
- ``void OnExportCharacteristics()`` — **главное действие**: для каждого ``k`` , у которого
  ``m_OutGrFlag_k`` и непустое имя, вызывает
  ``CTmcTtoS::TtoS(m_SFileName_k, m_SFileName, m_dTmin*dTUnit, m_dTmax*dTUnit, m_dXValue_k)`` ;
  при ошибке пишет её в ``m_csError`` и прерывается;
- ``void OnOK()`` — вызывает ``OnExportCharacteristics()`` , затем ``CDialog::OnOK()`` .

**Ошибки/особенности:** входные сигналы конвертируются по очереди в **один** выходной S-файл
(каждый вызов ``TtoS`` дописывает строку матрицы — см. ``CTmcSMatrix::Save`` ); при первой же
ошибке экспорт останавливается.

9.4. ``CTmcGroutColorGraphDialog`` , ``CTmcGrParColTypWid``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Идентичны** TMCGROUT (см. ``tmcgrout.md`` , §9.3 и §9.4): настройка цвета/типа/толщины 16 линий
и одной линии соответственно. Заголовки ``TmcGroutColorGraphDialog.h`` , ``TmcGrParColTypWid.h`` .

9.5. ``CTmcGrParColTypWid1`` (НЕ в сборке)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Заголовок:** ``TmcGrParColTypWid1.h`` · **Базовый класс:** ``CDialog`` · **Ресурс:**
``IDD_DIALOGGRLINECOLORTYPEWIDTH1`` .

Пустой диалог-заготовка (нет полей и обработчиков, только ``DoDataExchange`` ). **Не включён** в
``TMCROS.vcxproj`` и в сборке не участвует — оставлен в каталоге как артефакт.

.. warning::

   ⚠️ Назначение ``CTmcGrParColTypWid1`` неясно; вероятно, неиспользуемый дубль
   ``CTmcGrParColTypWid`` (см. §13).

----

10. Подсистема Time-to-Scattering (TtoS)
----------------------------------------

Преобразование набора **временны́х сигналов** в **матрицу рассеяния** на заданной частоте. Идея:
по двум соседним отсчётам гармонического сигнала ``s(t)=A·sin(ωt+φ)`` восстанавливаются амплитуда
``A`` и фаза ``φ`` (комплексное ``S`` ), усреднённые по всем парам точек диапазона; возбуждённый
вход определяет строку S-матрицы.

10.1. Класс ``CTmcTtoS`` — обёртка «файл ``.t`` → файл ``.s`` »
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Заголовок:** ``src/Include/TmcTtoS.h`` · **Реализация:** ``TmcTtoS.cpp`` · **Зависит от:**
``TmcSMatrix.h`` , ``tmcgrviw.h`` .

**Поля (private):**

.. list-table::
   :header-rows: 1
   :widths: 20 20 60

   * - Поле
     - Тип
     - Назначение
   * - ``cSMatrix``
     - ``CTmcSMatrix``
     - Рабочее ядро преобразования
   * - ``csSFileName``
     - ``CString``
     - Имя выходного ``.s``
   * - ``csTFileName``
     - ``CString``
     - Имя входного ``.t``
   * - ``csError``
     - ``CString``
     - Текст ошибки (по умолчанию ``"No error"`` )
   * - ``dTmin`` , ``dTmax``
     - ``double``
     - Диапазон времени (секунды)
   * - ``dFreq``
     - ``double``
     - Частота (Гц), на которой восстанавливается S
   * - ``bIsError``
     - ``BOOL``
     - Флаг ошибки

**Методы:**

.. list-table::
   :header-rows: 1
   :widths: 50 50

   * - Метод
     - Что делает
   * - ``void TtoS(CString csTfn, CString csSfn, double dTmin1, double dTmax1, double dFreq1)``
     - Главный публичный вызов: сохраняет параметры и выполняет ``TtoS()``
   * - ``void TtoS(void)`` (private)
     - ``MakeSmatrixFromFileT()`` → ``SaveSmatrixToFileS()``
   * - ``void MakeSmatrixFromFileT(void)``
     - Читает ``.t`` (``cSMatrix.ReadTFile`` ) и строит S-матрицу (``cSMatrix.MakeSmatrix`` ); ошибки ядра переносит в ``csError``
   * - ``void SaveSmatrixToFileS(void)``
     - Сохраняет результат в ``.s`` (``cSMatrix.Save(csSFileName, dFreq)`` )
   * - ``BOOL IsError(void)``
     - Флаг ошибки
   * - ``CString& GetErrorMessage(void)``
     - Текст ошибки
   * - ``void DeleteData(void)`` (private)
     - Сброс полей и ``cSMatrix.DeleteData()``

10.2. Класс ``CTmcSMatrix`` — ядро восстановления S из сигнала
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Заголовок:** ``src/Include/TmcSMatrix.h`` · **Реализация:** ``TmcSMatrix.cpp`` · **Зависит от:**
``typedef.h`` (``_complex`` ), ``proc_s.h`` , ``s_file.h`` (SFILE95 — ``save_S_matrix_element`` ).

**Поля (private):**

.. list-table::
   :header-rows: 1
   :widths: 25 20 55

   * - Поле
     - Тип
     - Назначение
   * - ``csTFileName`` , ``csSFileName``
     - ``CString``
     - Имена входного ``.t`` и выходного ``.s``
   * - ``csError``
     - ``CString``
     - Текст ошибки
   * - ``dTmin`` , ``dTmax``
     - ``double``
     - Диапазон времени
   * - ``dFreq``
     - ``double``
     - Частота восстановления (Гц)
   * - ``bIsError``
     - ``BOOL``
     - Флаг ошибки
   * - ``pcSmatrix``
     - ``_complex*``
     - Восстановленная строка S-матрицы (``nInput`` элементов)
   * - ``pdTsignal``
     - ``double*``
     - Отсчёты сигнала всех входов (``nPoint*nInput*2`` )
   * - ``pdTime``
     - ``double*``
     - Отсчёты времени (``nPoint`` )
   * - ``nInput``
     - ``int``
     - Число входов (столбцов сигнала)
   * - ``nPoint``
     - ``int``
     - Число точек времени в диапазоне
   * - ``nLineInSMatrix``
     - ``int``
     - Индекс **возбуждённого** входа = номер строки S-матрицы (−1 — не определён)

**Публичные методы:**

.. list-table::
   :header-rows: 1
   :widths: 50 50

   * - Метод
     - Что делает
   * - ``void ReadTFile(CString csTFile, double dTmin1, double dTmax1)`` / ``void ReadTFile(void)`` (private)
     - Читает ``.t`` : проверяет ``Tmax>Tmin`` , считает ``nPoint`` (``CalcNPoint`` ) и ``nInput`` (``CalcNInput`` ), выделяет массивы (``AllocTArray`` ), читает данные (``ReadTArray`` )
   * - ``void MakeSmatrix(void)``
     - Восстанавливает S: ``AllocSArray()`` → ``TtoS()`` → ``SetnLineInSMatrix()``
   * - ``void Save(CString csSFileName1, double dFreq1)`` / ``void Save(CString csSFileName1)`` / ``void Save(void)`` (private)
     - Записывает строку S-матрицы в ``.s`` через ``save_S_matrix_element(nLineInSMatrix+1, …, pcSmatrix, dFreq, nInput)``
   * - ``BOOL IsError(void)`` / ``CString& GetErrorMessage(void)``
     - Флаг и текст ошибки
   * - ``void DeleteData(void)``
     - Освобождение массивов и сброс полей

**Приватные методы (алгоритм):**

.. list-table::
   :header-rows: 1
   :widths: 45 55

   * - Метод
     - Назначение
   * - ``int CalcNPoint(void)``
     - Считает число строк данных в диапазоне ``[Tmin,Tmax]`` ; параллельно читает частоту из ``#TMC_GROTS_Freq`` ; требует >5 точек
   * - ``int CalcNInput(void)``
     - По первой строке данных определяет число входов: ``(чисел_в_строке − 2)/3``
   * - ``double gets_f1(char *ch)``
     - Читает время из строки (``sscanf "%d%lg"`` )
   * - ``int gets_nNumbersInLine(char *ch)``
     - Считает число «слов»-чисел в строке
   * - ``void AllocTArray(void)``
     - Выделяет ``pdTsignal[nPoint*nInput*2]`` , ``pdTime[nPoint]``
   * - ``void ReadTArray(void)``
     - Читает отсчёты сигнала всех входов в диапазоне времени (``readTsignal`` / ``readTsignalOne`` )
   * - ``void readTsignal(double **pdTsign, char *ch)`` / ``int readTsignalOne(…)``
     - Разбор одной строки: пропуск номера и времени, затем по ``nInput`` пар (Re/Im сигнала)
   * - ``int skip_blank(char*)`` / ``int skip_number(char*)``
     - Пропуск пробелов/числа
   * - ``void AllocSArray(void)``
     - Выделяет ``pcSmatrix[nInput]`` , инициализирует ``FLT_MAX``
   * - ``void TtoS(void)``
     - По всем входам вызывает ``TtoSOneInput``
   * - ``void TtoSOneInput(int n)``
     - Усредняет комплексное ``S`` входа ``n`` по всем парам соседних точек (``TtoSOneInputOnePoint`` )
   * - ``BOOL TtoSOneInputOnePoint(_complex *acc, double s1, double t1, double s2, double t2)``
     - По паре ``(s,t)`` решает систему для ``A`` и ``φ`` гармоники ``A·sin(ωt+φ)`` на частоте ``dFreq`` ; прибавляет ``A·(cosφ, sinφ)`` к аккумулятору. Возвращает ``TRUE`` , если вклад учтён
   * - ``void SetnLineInSMatrix(void)``
     - Находит **единственный** возбуждённый вход → ``nLineInSMatrix`` ; ошибка, если возбуждённых не один
   * - ``BOOL IsInputExcite(int n)``
     - ``TRUE`` , если у входа ``n`` есть ненулевые отсчёты (вход возбуждён)
   * - ``void PutErrorMessage(char *ch)``
     - Устанавливает ``bIsError`` и текст ошибки

**Формат файла сигнала** ``.t`` (по коду парсинга): строки данных начинаются с пробела и содержат
``номер_точки  время  [для каждого входа: номер  Re  Im] …`` ; служебная строка
``#TMC_GROTS_Freq <частота>`` задаёт частоту; сигнатура файла —
``#TMC_GraphicsOutputSignalFile FormVer2.0 2000`` .

.. warning::

   ⚠️ Математика восстановления S из сигнала (``TtoSOneInputOnePoint`` , усреднение, критерий
   возбуждённого входа) — научный алгоритм; не изменять без согласования. Физический смысл
   отдельных порогов (``10*DBL_MIN`` , ветви выбора ``dsin/dcos`` ) требует уточнения (см. §13).

10.3. Известные дефекты в TtoS-коде (для сведения)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

В ``CTmcSMatrix::CalcNPoint()`` и ``CalcNInput()`` при ошибке открытия файла вызывается
``sprintf("Can't open file {%s}", csTFileName);`` **без буфера-приёмника** (первый аргумент —
формат, а не буфер). Это ошибочный вызов (потенциальный сбой). Отмечено как дефект кода —
см. §13.

----

11. Класс ``CTMCGrExpression`` — графики-выражения
--------------------------------------------------

**Идентичен** TMCGROUT (см. ``tmcgrout.md`` , §10). Разбирает выражение над несколькими файлами
(``[имя]`` -переменные), вычисляет значение в точке через exprint (``i1nte_atof_1`` ). В TMCROS
«файлы-переменные» — это ``.t`` -сигналы, а методы ``GetExpressionValue*`` применяются к
восстановленным из сигнала величинам.

**Заголовок:** ``src/Include/TMCGrExpression.h`` · **Реализация:** ``TMCGrExpression.cpp`` .

----

12. Глобальные функции (в ``TMCGROUTView.cpp``)
-----------------------------------------------

**Как в TMCGROUT** (см. ``tmcgrout.md`` , §11). Объявлены в ``TMCGROUTView.h`` :
``void PutTrace(CString)`` , ``void PutTrace(char*)`` , ``void PutStatistics(CString)`` ,
``void PutStatistics(char*)`` — вывод в панели 0 и 1 строки состояния.

----

13. Требует уточнения у автора (К.Н. Климов)
--------------------------------------------

----

14. Нужные иллюстрации
----------------------

- 📸 НУЖНА СХЕМА: поток данных TMCROS —
  ``файл сигнала .t → CTMCGROUTDoc::read_RT_output_signal → TMC_GR_DOC1 (pFreq=время, pSmatr.x=сигнал) → CTMCGROUTView::PrepareDoubleGraph → tmcgrwin → tmcgrwin1 → OnDraw`` .
- 📸 НУЖНА СХЕМА: конвейер TtoS —
  ``CDialogTtoS (список .t, частоты, Tmin/Tmax) → CTmcTtoS::TtoS → CTmcSMatrix (ReadTFile → MakeSmatrix → Save) → файл .s`` .
- 📸 НУЖНА СХЕМА: восстановление гармоники по двум отсчётам сигнала ``s(t)=A·sin(ωt+φ)`` в
  ``TtoSOneInputOnePoint`` (геометрия задачи).
- Скриншоты окон (главное окно с сигналом, диалог «To S-matrix») относятся к руководству
  пользователя, а не к API-документации.
