// PlanRT_HView.h : interface of the CPlanRT_HView class
//
/////////////////////////////////////////////////////////////////////////////

/// @file PlanRT_HView.h
/// @brief Вид оболочки ядра: фон окна задания и обработчики всех команд меню и панели.
///
/// Часть модуля «Оболочка счётного ядра». Вид ничего не рисует, кроме фона: ход
/// расчёта показывается в строке состояния. Его главная роль — карта сообщений:
/// команды меню Run, View, Config, Edit, их доступность и «галочки», чтение и запись
/// настроек программы, запуск расчётного потока.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#menu
#ifndef CTMCRTH_CPlanRT_HView_H__
#define CTMCRTH_CPlanRT_HView_H__

#include "TmcSoundEffProp.h"	// Added by ClassView
/// @brief Вид окна задания: фон и обработчики команд меню (Run, View, Config, Edit).
///
/// Создаётся главным окном Qt (`MainWindow::openDocument`) через `TmcCreate()` в паре с
/// документом и показывается виджетом-хозяином `MfcViewWidget`. Команды доходят до
/// вида через его карту сообщений (`MainWindow::dispatch`); доступность пунктов
/// пересчитывают обработчики `OnUpdate*` (таймер главного окна, раз в 200 мс).
/// `OnInitialUpdate` читает настройки, данные первого шага и в пакетном режиме сразу
/// запускает расчёт. Удаляется главным окном при закрытии программы.
/// @note Потоки: все методы — в главном потоке; расчёт запускается в отдельном потоке.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#menu
class CPlanRT_HView : public CScrollView
{
protected: // create from serialization only
	/// @brief Создаёт вид: набор страниц мелодий «Sound Effects», чёрный фон.
	CPlanRT_HView();
	DECLARE_DYNCREATE(CPlanRT_HView)

// Attributes
public:
	/// @brief Возвращает документ вида.
	/// @return Указатель на документ; принадлежит главному окну.
	CPlanRT_HDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlanRT_HView)
	public:
	/// @brief Рисует вид: заливает видимую область цветом фона.
	/// @param[in] pDC контекст рисования (экран или принтер).
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	/// @brief Обработчик каркаса перед созданием окна; стиль не меняет.
	/// @param[in,out] cs параметры создания окна.
	/// @return Ответ базового класса.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	/// @brief Разрушение окна вида: просит расчёт остановиться (`OnRunStop`), не дожидаясь потока.
	/// @return Ответ базового класса.
	virtual BOOL DestroyWindow();
	protected:
	/// @brief Первое обновление вида после открытия задания.
	///
	/// Порядок: размеры прокрутки; программы по умолчанию (`OnConfigSetup`); чтение
	/// настроек программы (редактор, просмотрщики, цвет фона, форматы вывода, флаги
	/// поля и синхронизации, звук и мелодии); тип системы для звука; чтение данных
	/// первого шага; в демонстрационном режиме (/d) — запись настроек и включение
	/// вывода поля; в пакетном режиме (/Ar, /Arb, /d) — запуск расчёта всех шагов
	/// (`OnRunAll`); в конце — запись настроек.
	/// @see https://www.tamic.ru/docs/code/kernel-shell.html#open-task
	virtual void OnInitialUpdate(); // called first time after construct
	/// @brief Подготовка печати каркаса MFC.
	/// @param[in,out] pInfo сведения о печати.
	/// @return Ответ `DoPreparePrinting`.
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	/// @brief Начало печати; дополнительных действий нет.
	/// @param[in] pDC   контекст принтера.
	/// @param[in] pInfo сведения о печати.
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	/// @brief Конец печати; дополнительных действий нет.
	/// @param[in] pDC   контекст принтера.
	/// @param[in] pInfo сведения о печати.
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	/// @brief Один раз за время работы программы запускает просмотрщик поля (`OnViewField1`).
	///
	/// Повторные вызовы ничего не делают (статический флаг). Используется демонстрационным
	/// режимом через `RunOnViewField1` из `pl_iofor.cpp`.
	void OnViewField11( void );
	/// @brief Разрушает вид и снимает указатель на него, оставленный для демонстрационного режима.
	virtual ~CPlanRT_HView();
#ifdef _DEBUG
	/// @brief Проверка целостности объекта (отладочная сборка MFC).
	virtual void AssertValid() const;
	/// @brief Отладочный вывод объекта (отладочная сборка MFC).
	/// @param[in,out] dc контекст отладочного вывода.
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CPlanRT_HView)
	/// @brief Edit -> Edit (Alt+F7): открывает задание во внешнем редакторе; при неудаче просит выбрать редактор.
	///
	/// В Qt-оболочке это запасной путь: главное окно сначала открывает задание в TPL Designer.
	afx_msg void OnEditEdit();
	/// @brief Config -> Editor: выбор внешнего редактора задания в окне открытия файла; сохраняет настройки.
	afx_msg void OnConfigEditor();
	/// @brief Config -> Color -> BackGround: выбор цвета фона вида; сохраняет настройки и перерисовывает.
	afx_msg void OnConfigColorBackGround();
	/// @brief Config -> Format -> OutputDataFile: форматы чисел в выходных файлах.
	///
	/// Показывает диалог форматов; неверный формат заменяется форматом по умолчанию
	/// (с сообщением); кнопка «Set default» восстанавливает все форматы и открывает
	/// диалог снова. Сохраняет настройки.
	afx_msg void OnConfigFormatOutputDataFile();
	/// @brief Run -> Skip Step (Ctrl+F6): останавливает расчёт, переходит к следующему шагу и читает его данные.
	afx_msg void OnRunSkipstep();
	/// @brief Run -> Back Step (Alt+F6): останавливает расчёт, переходит к предыдущему шагу и читает его данные.
	afx_msg void OnRunBackstep();
	/// @brief Run -> Load first step (Shift+F5): останавливает расчёт, ждёт 1 с, переходит к шагу 1 и читает его.
	afx_msg void OnRunRestartall();
	/// @brief Run -> Load Step (Shift+F8): останавливает расчёт и заново читает данные текущего шага.
	afx_msg void OnRunRestartstep();
	/// @brief Run -> Run Step (F8): запускает расчёт текущего шага в отдельном потоке (`RunStepGlobal`).
	/// @see https://www.tamic.ru/docs/code/kernel-shell.html#run-thread
	afx_msg void OnRunStartstep();
	/// @brief View -> Statistics (Shift+F4): окно параметров текущего шага.
	///
	/// Во время расчёта не открывается (сообщение). Копирует в диалог параметры,
	/// топологию, описание вывода и замер времени документа. Кнопки «Prev.»/«Next»
	/// окна переводят документ на соседний шаг, читают его данные и открывают окно снова.
	/// @see https://www.tamic.ru/docs/code/kernel-shell.html#statistics
	afx_msg void OnViewStatistics();
	/// @brief File -> Save As: копирует файл задания побайтно под новым именем и делает его путём документа.
	afx_msg void OnFileSaveAs();
	/// @brief Run -> Stop (Alt+F5): просит расчёт остановиться, не дожидаясь потока.
	afx_msg void OnRunStop();
	/// @brief Run -> Run (F5): запускает расчёт всех шагов от текущего в отдельном потоке (`RunAllGlobal`).
	/// @see https://www.tamic.ru/docs/code/kernel-shell.html#run-thread
	afx_msg void OnRunAll();
	/// @brief Закрытие окна вида; вызывает базовый класс (в Qt-оболочке — пустая прослойка, окнами управляет Qt).
	afx_msg void OnClose();
	/// @brief Разрушение окна вида; вызывает базовый класс.
	afx_msg void OnDestroy();
	/// @brief Config -> Viewer -> Output signal: выбор программы просмотра сигналов (TMCROS); сохраняет настройки.
	afx_msg void OnConfigViewerOutputsignal();
	/// @brief View -> Output (Alt+Shift+F4): запускает просмотр сигналов на портах; при неудаче просит выбрать программу.
	afx_msg void OnViewOutput();
	/// @brief Config -> Field (Ctrl+F4, переключатель): включает или выключает вывод полей; сохраняет настройки.
	afx_msg void OnViewField();
	/// @brief View -> Field and topology: запускает просмотрщик поля; при неудаче просит выбрать программу.
	afx_msg void OnViewField1();
	/// @brief Config -> Viewer -> Field: выбор программы просмотра поля (FieldView); сохраняет настройки.
	afx_msg void OnConfigViewerField();
	/// @brief Config -> Synchronization (переключатель): синхронизация вывода поля; сохраняет настройки.
	afx_msg void OnConfigSinchronization();
	/// @brief Config -> Sound -> On/Off (переключатель): звуковые сигналы об окончании шага и расчёта.
	afx_msg void OnConfigSound();
	/// @brief Config -> Sound -> Melody: две мелодии (конец шага, конец расчёта) по 12 нот и интервал; сохраняет настройки.
	afx_msg void OnConfigSoundMelody();
	/// @brief Config -> Topology (F4): записывает файл топологии текущего шага; во время расчёта — только сообщение.
	afx_msg void OnViewTopology();
	/// @brief Состояние пункта Synchronization: отметка — флаг синхронизации, доступен при включённом выводе поля.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки.
	afx_msg void OnUpdateConfigSinchronization(CCmdUI* pCmdUI);
	/// @brief Состояние пункта Field: отметка — флаг вывода полей.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки.
	afx_msg void OnUpdateViewField(CCmdUI* pCmdUI);
	/// @brief Состояние пункта Sound On/Off: отметка — флаг звука.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки.
	afx_msg void OnUpdateConfigSound(CCmdUI* pCmdUI);
	/// @brief Состояние пункта Topology: отметка — флаг вывода топологии.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки.
	afx_msg void OnUpdateViewTopology(CCmdUI* pCmdUI);
	/// @brief Состояние пункта Field and topology: доступен, когда включены и вывод поля, и вывод топологии.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки.
	afx_msg void OnUpdateViewField1(CCmdUI* pCmdUI);
	/// @brief Состояние Run: доступен, когда нет ни чтения данных, ни расчёта.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки.
	afx_msg void OnUpdateRunRun(CCmdUI* pCmdUI);
	/// @brief Состояние Load first step: доступен, когда нет ни чтения данных, ни расчёта.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки.
	afx_msg void OnUpdateRunRestartall(CCmdUI* pCmdUI);
	/// @brief Состояние Run Step: доступен, когда нет ни чтения данных, ни расчёта.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки.
	afx_msg void OnUpdateRunStartstep(CCmdUI* pCmdUI);
	/// @brief Состояние Load Step: доступен, когда нет ни чтения данных, ни расчёта.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки.
	afx_msg void OnUpdateRunRestartstep(CCmdUI* pCmdUI);
	/// @brief Состояние Skip Step: доступен, когда нет ни чтения данных, ни расчёта.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки.
	afx_msg void OnUpdateRunSkipstep(CCmdUI* pCmdUI);
	/// @brief Состояние Back Step: доступен, когда нет ни чтения данных, ни расчёта.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки.
	afx_msg void OnUpdateRunBackstep(CCmdUI* pCmdUI);
	/// @brief Состояние Stop: доступен во время чтения данных или расчёта.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки.
	afx_msg void OnUpdateRunStop(CCmdUI* pCmdUI);
	/// @brief View -> S-matrix (Ctrl+Shift+F4): запускает просмотрщик S-матрицы; при неудаче просит выбрать программу.
	afx_msg void OnConfigSmatrix();
	/// @brief Config -> Viewer -> S-matrix: выбор программы просмотра S-матрицы (TMCGROUT); сохраняет настройки.
	afx_msg void OnConfigViewerSmatrix();
	/// @brief Состояние File -> Close: доступен, когда нет ни чтения данных, ни расчёта.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки.
	afx_msg void OnUpdateFileClose(CCmdUI* pCmdUI);
	/// @brief Config -> AutoRun (кнопка панели «Batch mode»): меняет флаг пакетного режима.
	afx_msg void OnConfigAutorun();
	/// @brief Состояние пункта AutoRun: отметка — флаг пакетного режима.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки.
	afx_msg void OnUpdateConfigAutorun(CCmdUI* pCmdUI);
	/// @brief Config -> Setup: восстанавливает редактор и просмотрщики по умолчанию, где они не заданы.
	///
	/// Для каждой программы берёт значение из WIN.INI (на современной Windows и вне её —
	/// всегда текущее значение документа); пустое заменяет программой пакета:
	/// NOTEPAD.EXE, TMCROS.EXE, FLDVIEW.EXE (ищется как FieldView.exe), TMCGROUT.EXE,
	/// TMC_DN.EXE. Имена без папки ищутся при запуске рядом с ядром (`CreatProc`).
	/// @see https://www.tamic.ru/docs/code/kernel-shell.html#viewers
	afx_msg void OnConfigSetup();
	/// @brief View -> Directional Pattern (Ctrl+Alt+F4): запускает просмотр ДН; при неудаче просит выбрать программу.
	afx_msg void OnViewDirectionalpattern();
	/// @brief Состояние пункта Directional Pattern: доступен, если в задании есть контур интегрирования поля.
	/// @param[in,out] pCmdUI состояние пункта меню или кнопки.
	afx_msg void OnUpdateViewDirectionalpattern(CCmdUI* pCmdUI);
	/// @brief Config -> Viewer -> Directional Pattern: выбор программы просмотра ДН (TMC_DN); сохраняет настройки.
	afx_msg void OnConfigViewerDirectionalpattern();
	/// @brief Config -> Directional Pattern: записывает файл ДН по накопленному полю текущего шага.
	/// @see https://www.tamic.ru/docs/theory/radiation.html#pattern
	afx_msg void OnConfigDirectionalpattern();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	/// Набор страниц «Sound Effects» (две мелодии); живёт вместе с видом.
	CTmcSoundEffProp cSoundDialog;
	/// Цвет фона вида, `COLORREF` (по умолчанию чёрный); хранится в настройках.
	COLORREF scBackgoundColor;
	/// @brief Заливает видимую область вида цветом фона.
	/// @param[in] pDC контекст рисования.
	void OnDrawBackground(CDC* pDC);
	/// Признак «вид запускает чтение или расчёт» — защита от повторного входа в обработчики.
	volatile BOOL bIsReadData;
	/// @brief Читает данные текущего шага через документ и выводит в строку состояния номер шага и сообщение ядра.
	void ReadData( void );
	/// @brief Сохраняет настройки программы: редактор, просмотрщики, цвет фона, флаги поля, форматы вывода, звук.
	/// @see https://www.tamic.ru/docs/code/kernel-shell.html#settings
	void WriteFile( void );
};

#ifndef _DEBUG  // debug version in PlanRT_HView.cpp
/// @brief Возвращает документ вида (вариант без отладочной проверки типа).
inline CPlanRT_HDoc* CPlanRT_HView::GetDocument()
   { return (CPlanRT_HDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
#endif