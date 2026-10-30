// TMCGROUTView.h : interface of the CTMCGROUTView class
//
/////////////////////////////////////////////////////////////////////////////
/// @file TMCGROUTView.h
/// @brief Вид TMC_DN: запуск расчёта ДН в отдельном потоке, рисование ДН, оси, сетка, легенда с КНД и КИП, мышь и команды меню.
///
/// Часть модуля «Вьювер TMC_DN». Копия вида TMCGROUT (через TMCROS) с отличиями: расчёт
/// документа идёт в потоке `ReadDocFileW_Thread1` под окном прогресса, ось X — угол,
/// изменение диапазона X пересчитывает ДН (`ReadDocFileW_1`). Рисует в контексте GDI слоя
/// совместимости (поверх QPainter).
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#drawing
#include <Tmcgrviw.h>
#include <COMPLEX1.H>

/// @brief Выводит текст в левую ячейку строки состояния главного окна (координаты под мышью).
///
/// Строку состояния берёт у рамки `AfxGetApp()->m_pMainWnd` (`CMainFrame::m_wndStatusBar`);
/// в Qt6-версии её текст попадает в виджет активного документа.
/// @param[in] csTrace текст
void PutTrace( CString csTrace );
/// @brief Выводит текст в левую ячейку строки состояния (вариант для строки C).
/// @param[in] szTrace текст
void PutTrace( const char *szTrace );
/// @brief Выводит текст в правую ячейку строки состояния главного окна (величина по оси Y и частота).
/// @param[in] csTrace текст
void PutStatistics( CString csTrace );
/// @brief Выводит текст в правую ячейку строки состояния (вариант для строки C).
/// @param[in] szTrace текст
void PutStatistics( const char *szTrace );

/// @brief Вид документа TMC_DN: окно ДН по углу с осями, сеткой и легендой (КНД, КИП).
///
/// Создаётся главным окном вместе с документом (`CTMCGROUTView::TmcCreate`), рисует в
/// виджете-хозяине `DnViewWidget` (потокобезопасный `MfcViewWidget`). Координаты —
/// `MM_HIMETRIC` (логическая единица 0.01 мм, ось Y вверх, область графика в отрицательных Y).
/// Расчёт: `ReadDocFileW` запускает поток `ReadDocFileW_Thread1` (документ читается и ДН
/// считаются там) и показывает модальное окно прогресса `CThreadCalcDirPat`; пока поток
/// работает (`g_lTmcDnCalcRunning` != 0), `OnDraw` выводит «Please wait ... Data calculate»,
/// а команды, меняющие данные, ничего не делают. Подготовка рисунка в два шага:
/// `PrepareDoubleGraph` переводит ДН в величины осей и подбирает диапазоны,
/// `PrepareLogGraph` — в логические координаты и прореживает точки (`tmcgrwin1`); `OnDraw`
/// рисует сетку, оси, линии, маркеры и легенду. Каждые 10 с таймер проверяет файлы и при
/// изменении считает заново. Обработчики команд меню вызывает `MainWindow::dispatch`
/// через карту сообщений.
/// @note Потоки: главный поток; `ReadDocFileW_Thread` выполняется в потоке расчёта.
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#drawing
class CTMCGROUTView : public CScrollView
{
protected: // create from serialization only
	/// @brief Задаёт окно графика по умолчанию и таблицы подписей осей (X: Angle radian, Angle degree и наследие Time; Y: Pattern, Pattern dB …).
	CTMCGROUTView();
	DECLARE_DYNCREATE(CTMCGROUTView)

// Attributes
public:
	/// Время запуска расчёта ДН, с (для поля «Elaps. Time» окна прогресса).
	time_t ltime_start;
	/// @brief Возвращает документ вида.
	/// @return Документ; принадлежит главному окну.
	CTMCGROUTDoc* GetDocument();
	/// @brief Тело потока расчёта: читает документ и считает все ДН (`CTMCGROUTDoc::ReadDocFile`), выделяет массивы точек вида, готовит рисунок, ставит `bIsDataCalculate`.
	/// @note Потоки: выполняется в потоке `ReadDocFileW_Thread1`; перерисовка из него уходит в главный поток (`DnViewWidget`).
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#thread
	void ReadDocFileW_Thread( void );

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTMCGROUTView)
	public:
	/// @brief Рисует окно графика: рамку выделения мышью, сетку, оси, графики и легенду.
	///
	/// При первом рисовании подгоняет область графика под окно (`OnViewResizectrlr`). Пока ДН
	/// не посчитаны или идёт расчёт — выводит только «Please wait ... Data calculate»; если у
	/// документа есть ошибка — только `Error : <текст>`.
	/// @param[in] pDC контекст рисования (экран или принтер)
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	/// @brief Параметры окна каркаса; изменений нет.
	/// @param[in,out] cs параметры создания окна
	/// @return Результат базового класса.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	/// @brief Перед рисованием на экране задаёт цвет фона текста равным цвету фона документа.
	/// @param[in,out] pDC   контекст рисования
	/// @param[in]     pInfo сведения о печати; NULL — рисование на экране
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	/// @brief Уничтожение окна вида; дополнительных действий нет.
	/// @return Результат базового класса.
	virtual BOOL DestroyWindow();
	protected:
	/// @brief Первое обновление: читает параметры документа, считает ДН (`ReadDocFileW`, поток и окно прогресса), восстанавливает настройки вида, готовит графики, запускает таймер (10 с).
	///
	/// Из настроек программы (раздел `#TMCGROTS`) берутся размер области графика
	/// (`#INIWNDVIEWPORT`), формат подписей осей и размер маркеров.
	virtual void OnInitialUpdate(); // called first time after construct
	/// @brief Готовит печать стандартным способом каркаса.
	/// @param[in,out] pInfo сведения о печати
	/// @return TRUE — печатать.
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	/// @brief Начало печати; действий нет.
	/// @param[in] pDC   контекст принтера
	/// @param[in] pInfo сведения о печати
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	/// @brief Конец печати; действий нет.
	/// @param[in] pDC   контекст принтера
	/// @param[in] pInfo сведения о печати
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	/// @brief Освобождает подготовленные графики и таблицы подписей осей.
	virtual ~CTMCGROUTView();
#ifdef _DEBUG
	/// @brief Проверка объекта (отладочная сборка MFC).
	virtual void AssertValid() const;
	/// @brief Отладочный вывод (отладочная сборка MFC).
	/// @param[in,out] dc контекст вывода
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTMCGROUTView)
	/// @brief Правая кнопка мыши: внутри области графика — окно Viewport, вне её — окно Characteristics.
	/// @param[in] nFlags клавиши-модификаторы
	/// @param[in] point  точка щелчка, координаты окна (пиксели)
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#mouse
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	/// @brief Команда Edit > Editor (Alt+F7): открывает файл документа во внешнем редакторе `csEditorName`.
	afx_msg void OnEditEdit();
	/// @brief Левая кнопка внутри области графика: координаты точки — в строку состояния, начало рамки выделения, захват мыши.
	/// @param[in] nFlags клавиши-модификаторы
	/// @param[in] point  точка щелчка, координаты окна (пиксели)
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#mouse
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	/// @brief Двойной щелчок: внутри области — автодиапазон (ось X -pi..pi или -180..180°, пересчёт ДН), вне — подгонка области под окно.
	///
	/// Проверка изменившихся файлов тоже вызывается, но после первого расчёта `ReadDocFileW`
	/// ничего не делает (`bIsDataCalculate`); перечитывание выполняет таймер.
	/// @param[in] nFlags клавиши-модификаторы
	/// @param[in] point  точка щелчка, координаты окна (пиксели)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	/// @brief Команда View > Viewport (Ctrl+H): окно диапазонов осей, угла (рад/град), вида ДН (линейно/дБ, нормировка), формата подписей и маркеров.
	///
	/// После OK записывает параметры в документ, файл документа и настройки программы и
	/// перерисовывает. Кнопки шрифта и цветов окна закрывают его, открывают своё окно и
	/// затем снова открывают Viewport.
	afx_msg void OnEditGraphicsparameters();
	/// @brief Команда View > Redraw data (Ctrl+W): перечитывает документ и заново считает все ДН (поток и окно прогресса), перерисовывает.
	afx_msg void OnViewGraphics();
	/// @brief Команда Edit > Characteristics (Ctrl+D): окно списка ДН (20 строк) с признаками показа и Reduce.
	///
	/// После OK список собирается из непустых строк окна (Reduce -> мода 1 = 1/0); графики с
	/// номером больше 20 (в окне не видны) сохраняются без изменений. Документ записывается и
	/// все ДН считаются заново. Кнопка добавления файла вызывает `OnEditAddcharacteristics`.
	/// Во время расчёта команда не действует.
	afx_msg void OnEditDocument();
	/// @brief Таймер 1 (каждые 10 с): если файл документа или файлы `.DAT` изменились (и расчёт не идёт), считает ДН заново и перерисовывает.
	/// @param[in] nIDEvent номер таймера
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#auto-refresh
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	/// @brief Команда Edit > Add characteristics (Ctrl+A): выбор файла `*.dat`, добавление ДН в документ, расчёт и окно Characteristics.
	afx_msg void OnEditAddcharacteristics();
	/// @brief Команда View > Zoom > ReSize viewport (Ctrl+R): подгоняет область графика под окно (70% ширины, 90% высоты).
	afx_msg void OnViewResizectrlr();
	/// @brief Конец выделения рамкой: новый диапазон осей по рамке и пересчёт ДН на новом диапазоне X.
	///
	/// Если рамка по обеим осям меньше 10% текущего диапазона — это щелчок, диапазон не
	/// меняется. Иначе меняется диапазон тех осей, по которым рамка не меньше 10%, и для них
	/// выключается автодиапазон; ось Y, не попавшая в рамку, переходит в автодиапазон.
	/// Документ записывается, ДН пересчитываются (`ReadDocFileW_1`).
	/// @param[in] nFlags клавиши-модификаторы
	/// @param[in] point  точка отпускания, координаты окна (пиксели)
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#mouse
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	/// @brief Во время выделения: растягивает рамку (в пределах области графика) и выводит координаты в строку состояния.
	/// @param[in] nFlags клавиши-модификаторы
	/// @param[in] point  положение мыши, координаты окна (пиксели)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	/// @brief Команда View > Zoom > Resize window (Ctrl+T): автодиапазон по обеим осям; ось X = -pi..pi (или -180..180°), ДН пересчитываются.
	afx_msg void OnViewResizewindow();
	/// @brief Команда Config > Editor: выбор исполняемого файла внешнего редактора; записывается в документ.
	afx_msg void OnConfigEditor();
	/// @brief Команда File > Save (Ctrl+S): записывает файл документа.
	afx_msg void OnFileSave();
	/// @brief Команда File > Save As: новый путь документа, пересчёт относительных путей графиков, запись.
	///
	/// Окно выбора файла предлагает расширение `.tos` (наследие TMCROS); для документа TMC_DN
	/// расширение нужно указать (`.dop`) вручную.
	afx_msg void OnFileSaveAs();
	/// @brief Команда Config > Font: шрифт и цвет текста подписей.
	afx_msg void OnConfigFont();
	/// @brief Команда Config > Color > Point: цвет маркеров точек всех графиков.
	afx_msg void OnConfigColorpoint();
	/// @brief Команда Config > Color > Axies: цвет осей.
	afx_msg void OnConfigColorAxis();
	/// @brief Команда Config > Color > Grid: цвет сетки.
	afx_msg void OnConfigColorGrid();
	/// @brief Команда Config > Color > Background: цвет фона.
	afx_msg void OnConfigColorBackground();
	/// @brief Команда Edit > To S-matrix: окно TtoS (`CDialogTtoS`) со списком файлов и диапазоном оси; в меню Qt6-версии TMC_DN команды нет.
	afx_msg void OnEditTosmatrix();
	/// @brief Команда View > Translate > Change Xmax, Xmin > Decrement (стрелка влево): сдвиг оси X на 10% ширины влево; автодиапазон X выключается, ДН пересчитываются.
	afx_msg void OnViewChangexmaxxminDecriment();
	/// @brief Команда View > Translate > Change Xmax, Xmin > Increment (стрелка вправо): сдвиг оси X на 10% ширины вправо; ДН пересчитываются.
	afx_msg void OnViewChangexmaxxminIncrement();
	/// @brief Команда View > Translate > Change Ymax, Ymin > Decrement (V): сдвиг оси Y на 10% высоты вниз; автодиапазон Y выключается.
	afx_msg void OnViewChangeymaxyminDecrement();
	/// @brief Команда View > Translate > Change Ymax, Ymin > Increment (стрелка вверх): сдвиг оси Y на 10% высоты вверх.
	afx_msg void OnViewChangeymaxyminIncrement();
	/// @brief Команда View > Zoom > Auto X size (Alt+X): включает или выключает автодиапазон оси X; при включении X = -pi..pi (-180..180°), ДН пересчитываются.
	afx_msg void OnViewAutoxsize();
	/// @brief Отмечает пункт Auto X size, если автодиапазон X включён.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки
	afx_msg void OnUpdateViewAutoxsize(CCmdUI* pCmdUI);
	/// @brief Команда View > Zoom > Auto Y size (Alt+Y): включает или выключает автодиапазон оси Y.
	afx_msg void OnViewAutoysize();
	/// @brief Отмечает пункт Auto Y size, если автодиапазон Y включён.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки
	afx_msg void OnUpdateViewAutoysize(CCmdUI* pCmdUI);
	/// @brief Команда Zoom + (+): уменьшает диапазоны обеих осей вдвое относительно центра (увеличение).
	afx_msg void OnViewZoomZoomp();
	/// @brief Команда Zoom - (-): увеличивает диапазоны обеих осей вдвое относительно центра (уменьшение).
	afx_msg void OnViewZoomZoom();
	/// @brief Команда Zoom +X (X): уменьшает диапазон оси X вдвое относительно центра; ДН пересчитываются.
	afx_msg void OnViewZoomZoomxp();
	/// @brief Команда Zoom +Y (Y): уменьшает диапазон оси Y вдвое относительно центра.
	afx_msg void OnViewZoomZoomyp();
	/// @brief Команда Zoom -X (Shift+X): увеличивает диапазон оси X вдвое относительно центра; ДН пересчитываются.
	afx_msg void OnViewZoomZoomx();
	/// @brief Команда Zoom -Y (Shift+Y): увеличивает диапазон оси Y вдвое относительно центра.
	afx_msg void OnViewZoomZoomy();
	/// @brief Команда Config > Color > Graphics: окно цвета, типа и толщины 16 стилей линий; после OK — запись документа и новый расчёт всех ДН.
	afx_msg void OnConfigColorGraphics();
	/// @brief Команда View > Translate > Change Xmax, Xmin > End: сдвигает окно X так, что правая граница = pi (180°), ширина прежняя; ДН пересчитываются.
	///
	/// Действует только при выключенном автодиапазоне X.
	afx_msg void OnViewTranslateChangexmaxxminEnd();
	/// @brief Команда View > Translate > Change Xmax, Xmin > Home: сдвигает окно X так, что левая граница = 0, ширина прежняя; ДН пересчитываются.
	///
	/// Действует только при выключенном автодиапазоне X.
	afx_msg void OnViewTranslateChangexmaxxminHome();
	/// @brief Закрытие окна: если идёт расчёт, ждёт его конца, показывая окно прогресса.
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	/// TRUE — ДН документа посчитаны и вид готов к рисованию; сброс в FALSE — следующий `ReadDocFileW` считает заново.
	BOOL bIsDataCalculate;
	/// @brief Пересчитывает ДН на новом диапазоне X без чтения файлов (`CTMCGROUTDoc::ReadDocFile_1`) и заново выделяет массивы точек вида.
	///
	/// Выполняется в главном потоке без окна прогресса; во время расчёта в потоке ничего не делает.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#angle-grid
	void ReadDocFileW_1( void );
	/// @brief Возвращает множитель единиц оси X по таблице TMCROS (1 для радиан, 1e12 для `TMC_GROTS_TYPE_ps` …); в TMC_DN не вызывается.
	/// @return Множитель.
	double GetXAxiesUnit( void );
	/// TRUE — прореживать маркеры: рисовать каждую (nPointDraw/20 + 1)-ю точку.
	BOOL bLossPoint;
	/// Масштаб маркеров точек (0..100, 1 — исходный размер 4 мм); хранится в настройках программы.
	double dPointSize;
	/// Формат `printf` подписей сетки по X (по умолчанию `%4.3lg`); хранится в настройках программы.
	CString csXAxiesFormat;
	/// Формат `printf` подписей сетки по Y (по умолчанию `%4.3lg`); хранится в настройках программы.
	CString csYAxiesFormat;
	/// @brief Выводит в левую ячейку строки состояния координаты точки под мышью: `<ось X>=<x>; <ось Y>=<y>`.
	void PutTrace1( void );
	/// @brief Выводит в правую ячейку строки состояния имя величины по оси Y и частоту документа: `<ось Y>; Freq=<ГГц> GHz`.
	void PutStatistics1( void );
	/// @brief Записывает в настройки программы размер области графика, форматы подписей и размер маркеров.
	void WriteIniWndPar( void );
	/// @brief Заливка фона области; тело закомментировано, действий нет (фон заливает `OnEraseBkgnd`).
	/// @param[in] pDC контекст рисования
	void OnDrawBackground(CDC* pDC);
	/// @brief Выводит текст шрифтом и цветом документа.
	///
	/// Высота шрифта документа умножается на 20 (перевод в единицы 0.01 мм) на время вывода.
	/// Если высота не задана, берётся из метрик текущего шрифта.
	/// @param[in] pDC   контекст рисования
	/// @param[in] nX    левый край текста, 0.01 мм
	/// @param[in] nY    верхний край текста, 0.01 мм (логическая координата)
	/// @param[in] cText текст
	void VTextOut( CDC *pDC, int nX, int nY, CString cText);
	/// @brief Помечает прямоугольник (логические координаты) для перерисовки.
	/// @param[in] r1 прямоугольник, 0.01 мм
	void IvalidateRectView( CRect  r1);
	/// Рамка выделения мышью, логические координаты (0.01 мм).
	CRect MouseResize;
	/// @brief Рисует рамку выделения мышью (пунктир, серая заливка), пока идёт выделение.
	/// @param[in] pDC контекст рисования
	void OnDrawMouseResize( CDC *pDC );
	/// TRUE — идёт выделение рамкой (левая кнопка нажата в области графика).
	BOOL FlagDrawMouseCoordMove;
	/// TRUE — при первом рисовании подогнать область графика под окно (нет сохранённого размера).
	BOOL FlagResizeInit;
	/// TRUE — последний щелчок был в области графика (координаты показаны).
	BOOL FlagDrawMouseCoord;
	/// @brief Проводит отрезок графика `i` из текущей точки в (nX, nY) его цветом, типом и толщиной.
	/// @param[in] pDC контекст рисования
	/// @param[in] i   номер графика
	/// @param[in] nX  конец отрезка по X, 0.01 мм
	/// @param[in] nY  конец отрезка по Y, 0.01 мм
	void OnDrawLine( CDC *pDC, int i, int nX, int nY );
	/// @brief Рисует строку легенды графика `i`: образец линии с маркерами, подпись и КНД с КИП.
	///
	/// Текст: `- <подпись>` (мода 1 = 0, без секторов) или `-<подпись>;` (с секторами), затем
	/// ` Knd = <дБ>dB; Kip = <разы>;` или для большой решётки ` Knd = <дБ>dB; Emitted Power = <..>;`;
	/// у ДН с ошибкой КНД не выводится. Легенда стоит справа от области графика
	/// (x = 1.10..1.15 правого края), строка i — на высоте (i+1)*7 мм от верха.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#legend
	/// @param[in] pDC контекст рисования
	/// @param[in] i   номер графика
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#legend
	void OnDrawGraph3(CDC *pDC, int i);
	/// @brief Рисует маркер графика `i` в точке (nX, nY), если включено рисование точек.
	/// @param[in] pDC контекст рисования
	/// @param[in] i   номер графика
	/// @param[in] nX  точка по X, 0.01 мм
	/// @param[in] nY  точка по Y, 0.01 мм
	void OnDrawPoint( CDC *pDC, int i, int nX, int nY );
	/// @brief Рисует маркеры по прореженным точкам `tmcgrwin1` графика `i` (каждую (nPointDraw/20 + 1)-ю, если включено прореживание).
	/// @param[in] pDC контекст рисования
	/// @param[in] i   номер графика
	void OnDrawGraph2(CDC *pDC, int i);
	/// @brief Рисует линию графика `i` по прореженным точкам `tmcgrwin1`.
	/// @param[in] pDC контекст рисования
	/// @param[in] i   номер графика
	void OnDrawGraph1(CDC *pDC, int i);
	/// @brief Рисует все графики: сначала линии, затем маркеры, затем легенду.
	/// @param[in] pDC контекст рисования
	void OnDrawGraph(CDC *pDC);
	/// @brief Переводит точки графиков в логические координаты вида и строит прореженный набор `tmcgrwin1` для рисования.
	///
	/// Точки вне диапазона X отбрасываются; из подряд идущих точек с одинаковой экранной
	/// высотой Y оставляются крайние (внутренние не рисуются). Значения Y вне диапазона
	/// прижимаются к краю области.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#drawing
	void PrepareLogGraph();
	/// @brief Пересчитывает графики (`PrepareDoubleGraph`, `PrepareLogGraph`) и перерисовывает окно.
	void RedrawViewGr();
	/// @brief Переводит ДН документа в величины осей и подбирает автодиапазоны.
	///
	/// X — угол в радианах или градусах. Y: `TMC_GROTS_TYPE_FULL` — |F| (нормированная
	/// амплитуда * `dMax`), `_AMPL` — 20*lg|F|, `_PHAS_R` — нормированная амплитуда, `_PHAS_G` —
	/// она же в дБ; у кривых-выражений — `GetExpressionValueK`. Автодиапазон Y — только по
	/// точкам в диапазоне X; нечисловые значения пропускаются; вырожденный диапазон
	/// расширяется (`TmcFixAxisRange`). Ничего не делает, пока ДН не посчитаны.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#axes
	void PrepareDoubleGraph();
	/// @brief Запускает расчёт документа в потоке `ReadDocFileW_Thread1` и показывает модальное окно прогресса до его конца.
	///
	/// Ничего не делает, если ДН уже посчитаны (`bIsDataCalculate` — вызывающий сбрасывает
	/// его, когда нужен новый расчёт) или расчёт уже идёт. Поток не запустился — ошибка
	/// документа `Error when open thread ...`.
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#thread
	void ReadDocFileW();
	/// @brief Переводит логическую координату X в значение оси X (с ограничением диапазоном оси).
	/// @param[in] x координата, 0.01 мм
	/// @return Значение в единицах оси X.
	double LXCordToDoublX( int x );
	/// @brief Переводит логическую координату Y в значение оси Y (с ограничением диапазоном оси).
	/// @param[in] y координата, 0.01 мм (отрицательная в области графика)
	/// @return Значение в единицах оси Y.
	double LYCordToDoublY( int y );
    /// @brief Переводит значение оси Y в логическую координату: Ymin -> -nYmax, Ymax -> -nYmin, с прижатием к краям.
    /// @param[in] y значение в единицах оси Y
    /// @return Координата, 0.01 мм.
    int DoublYCordToLY( double y );
	/// @brief Переводит значение оси X в логическую координату: Xmin -> nXmin, Xmax -> nXmax, с прижатием к краям.
	/// @param[in] x значение в единицах оси X
	/// @return Координата, 0.01 мм.
	int DoublXCordToLX( double x );
	/// Значение оси Y под мышью при последнем щелчке или движении.
	double MouseYCoord;
	/// Значение оси X под мышью при последнем щелчке или движении.
	double MouseXCoord;
	/// @brief Выводит координаты под мышью рядом с графиком; не вызывается (вызов в `OnDraw` закомментирован).
	/// @param[in] pDC контекст рисования
	void OnDrawMouseScrol(CDC* pDC);
	/// @brief Вычисляет прямоугольник вывода координат мыши справа от области графика.
	void SetRectOutXY();
	/// Прямоугольник вывода координат мыши, логические координаты (используется только `OnDrawMouseScrol`).
	CRect rectOutXY;
	/// @brief Рисует сетку 10 x 10 пунктиром цвета сетки и подписи её линий в формате осей.
	/// @param[in] pDC контекст рисования
	/// @see https://www.tamic.ru/docs/code/tmcgrout.html#grid
	void onDrawGrid( CDC *pDC );
	/// @brief Рисует оси X и Y со стрелками и их подписи (единицы X, величина Y) цветом осей.
	/// @param[in] pDC контекст рисования
	void onDrowAxies( CDC *pDC );
	/// Окно графика: диапазоны осей, область рисования, типы осей, все точки графиков.
	TMC_GR_WINDOW tmcgrwin;
	/// Прореженные точки графиков для рисования (строит `PrepareLogGraph`): только точки в
	/// диапазоне X без повторов экранной высоты.
	TMC_GR_WINDOW tmcgrwin1;
	/// Таблица единиц оси X и подписей (7 строк, последняя — конец таблицы): угол в радианах и градусах, остальное — наследие TMCROS.
	TMC_GR_TYPE_X * gr_typX;
	/// Таблица величин оси Y и подписей (8 строк, последняя — конец таблицы).
	TMC_GR_TYPE_Y * gr_typY;
};

#ifndef _DEBUG  // debug version in TMCGROUTView.cpp
inline CTMCGROUTDoc* CTMCGROUTView::GetDocument()
   { return (CTMCGROUTDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
