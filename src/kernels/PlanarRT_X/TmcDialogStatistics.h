// TmcDialogStatistics.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcDialogStatistics dialog

/// @file TmcDialogStatistics.h
/// @brief Окно Statistics: параметры сетки и времени текущего шага, оценка памяти и точности, переходы по шагам.
///
/// Часть модуля «Оболочка счётного ядра». Окно открывает `CPlanRT_HView::OnViewStatistics`
/// (меню View -> Statistics, Shift+F4). Из окна вызываются список блоков
/// (`CTmcRTH_DialogBlock`) и отчёт о времени расчёта (`CTmcDialogCalcTime`).
/// Разметка окна — `IDD_TMC_STATISTICS` (порождена из ресурса Windows-версии).
/// В ядре X код файла совпадает с одноимённым файлом ядра H (`src/kernels/PlanarRT_H`),
/// но компилируется с заголовками и расчётным объектом ядра X и макросами X-моды.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#x-differences
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#statistics
#include "TmcRTH_IndanParam.h"
#include "TmcRTH_IndanTopology.h"
#include "TmcRTH_IndanOutput.h"
#include <TmcCalcTiming.h>	// TIMING


/// @brief Окно Statistics: сведения о текущем шаге задания и переходы на соседние шаги.
///
/// Создаётся на стеке обработчиком вида; поля `m_*` заполняет вид, затем
/// `PrepareData()` пересчитывает их в единицы задания и `DoModal()` показывает окно.
/// Окно получает копии параметров и топологии шага, а описание вывода и замер
/// времени — указателями на объекты документа (не владеет ими).
/// Результат для вида — `nFlagStep`: -1 предыдущий шаг, 1 следующий, 0 нет перехода.
/// @warning Окно читает данные шага, поэтому вид не открывает его во время расчёта.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#statistics
class CTmcDialogStatistics : public CDialog
{
// Construction
public:
	/// Строка модели расчёта (`GetModel()`: точность вычислений ядра), поле «Accuracy».
	CString csAccuracy;
	/// Выбор пользователя: -1 — предыдущий шаг (или Cancel), 1 — следующий, 0 — без перехода.
	int nFlagStep;
	/// @brief Пересчитывает параметры шага в значения полей окна.
	///
	/// Длины — в единицах длины задания, время — в единицах времени задания, частота —
	/// в единицах частоты задания. Вычисляет: длину волны `lambda = c0/f`; размеры
	/// области в длинах волн (до 0,001); число узлов `nX*nY` так же, как ядро; оценку
	/// памяти, МБ (память блоков + `sizeof(CTmcRTHNodeDiel)` на блок + узлы ядра X по
	/// 12 значений `_real`, ещё 2 на узел при интегрировании поля в памяти); шаг по
	/// времени; число тактов и периодов окна времени; ошибку фазовой скорости сетки
	/// `acos(2*cos(omega*dt) - 1)/(sqrt(2)*omega*dt) - 1`, % (поле Tolerance).
	/// Нулевая частота, шаг сетки или единица длины дают нули вместо деления.
	/// @see https://www.tamic.ru/docs/theory/accuracy.html#dispersion
	void PrepareData( void );
	/// @brief Создаёт окно: указатели на вывод и замер времени пустые, Tolerance «100 %».
	/// @param[in] pParent родительское окно; NULL — главное окно программы.
	CTmcDialogStatistics(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTmcDialogStatistics)
	/// Номер разметки окна в ресурсах.
	enum { IDD = IDD_TMC_STATISTICS };
	/// Номер текущего шага \#STEP (заполняет вид).
	int		           m_nStep;
	/// Число узлов сетки шага, `nX*nY` (double: может превышать 2^31).
	double			   m_nNumNode;	// INT64: can be > 2^31
	/// Оценка памяти расчёта ядра X, МБ (три знака после запятой).
	double		       m_dMemory;
	/// Число блоков в топологии шага.
	int		           m_nBlock;
	/// Сообщение ядра о последней ошибке («Error:no» — ошибок нет).
	CString            m_csErrorMessage;
	/// Копия параметров шага (сетка, частота, единицы).
	CTmcRTH_IndanParam m_cParam;
	/// Копия топологии шага (список блоков).
	CTmcRTH_IndanTopology m_cTopol;
	/// Описание вывода документа (имена файлов, флаги вывода поля и топологии); не владеет.
	CTmcRTH_IndanOutput* m_cOut;
	/// Замер времени расчёта документа для окна «Calc. time...»; NULL — кнопка ничего не делает; не владеет.
	CTmcCalcTiming *m_pcTiming;	// TIMING: calculation time of the document
	/// Шаг сетки, единицы длины задания.
	double dDelta;
	/// Частота шага, единицы частоты задания.
	double dFreq;
	/// Шаг по времени, единицы времени задания.
	double dt;
	/// Число шагов по времени на период: `1/(dt*f)`.
	double dtT;
	/// Длина окна времени в тактах: `(Tmax - Tmin)/dt`, округлено.
	double dTsizeWL;
	/// Длина окна времени в периодах: `(Tmax - Tmin)*f`, округлено.
	double dTsizeWL1;
	/// Ширина области по X в длинах волн (три знака).
	double dXsizeWL;
	/// Левая граница области, единицы длины задания.
	double dXmin;
	/// Правая граница области, единицы длины задания.
	double dXmax;
	/// Высота области по Y в длинах волн (три знака).
	double dYsizeWL;
	/// Нижняя граница области, единицы длины задания.
	double dYmin;
	/// Верхняя граница области, единицы длины задания.
	double dYmax;
	/// Начало окна времени, единицы времени задания.
	double dTmin;
	/// Конец окна времени, единицы времени задания.
	double dTmax;
	/// Длина волны в свободном пространстве `c0/f`, единицы длины задания; 0 — частота не задана.
	double dWaveLen;
	/// Число шагов сетки на длину волны: `lambda/dDelta`.
	double dWaveLenDelt;
	/// Ошибка фазовой скорости сетки на частоте шага, строка вида «0.0123 %».
	CString csTolerance;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcDialogStatistics)
	protected:
	/// @brief Обмен данными полей окна (только вывод значений; флаги вывода поля и топологии — отметками).
	/// @param[in,out] pDX контекст обмена DDX.
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTmcDialogStatistics)
	/// @brief Кнопка «View block list»: листает блоки топологии в окне Block.
	///
	/// Копирует список блоков в собственный массив и показывает окно блока, пока
	/// пользователь нажимает «Next»/«Previous»; «Close» или OK завершают просмотр.
	afx_msg void OnViewBlockInformation();
	/// @brief Кнопка «Prev.»: закрыть окно с выбором предыдущего шага (`nFlagStep` = -1).
	afx_msg void OnStepPrev();;
	/// @brief Кнопка «Next»: закрыть окно с выбором следующего шага (`nFlagStep` = 1).
	afx_msg void OnStepNext();
	/// @brief Кнопка Cancel: закрыть окно без перехода по шагам (`nFlagStep` = -1, результат IDCANCEL).
	afx_msg void OnCancel1();
	/// @brief Кнопка «Calc. time...»: окно отчёта о времени этапов расчёта.
	/// @see https://www.tamic.ru/docs/code/kernel-shell.html#statistics
	afx_msg void OnCalcTime();	// TIMING
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	/// @brief Освобождает копию списка блоков, сделанную для окна Block.
	void DeleteData( void );
	/// Копия списка блоков для окна Block (массив `m_nBlock` элементов); NULL — нет копии; владеет.
	CTmcRTH_BlockList * pcBlockList;
};
