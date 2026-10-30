// TmcRTH_DialogBlock.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_DialogBlock dialog

/// @file TmcRTH_DialogBlock.h
/// @brief Окно Block: описание одного блока топологии текущего шага (тип, границы, точки, выражения).
///
/// Часть модуля «Оболочка счётного ядра». Окно вызывается кнопкой «View block list»
/// окна Statistics и листается кнопками «Next»/«Previous». Разметка — `IDD_TMC_RTH_TOPBLOCK`.
/// В ядре X код файла совпадает с одноимённым файлом ядра H (`src/kernels/PlanarRT_H`),
/// но компилируется с заголовками и расчётным объектом ядра X и макросами X-моды.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#x-differences
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#statistics
#include "TmcRTH_IndanParam.h"
#include "TmcRTH_IndanTopology.h"


/// @brief Окно с описанием одного блока задания в единицах задания.
///
/// Создаётся на стеке окном Statistics (`OnViewBlockInformation`) и показывается
/// многократно: перед каждым показом окно Statistics задаёт `m_pcParam`,
/// `m_pcBlockList` и вызывает `PrepareData()`. Номер блока для следующего показа
/// окно возвращает в поле `i`. Ни параметрами, ни блоком не владеет.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#statistics
class CTmcRTH_DialogBlock : public CDialog
{
// Construction
public:
	/// @brief Заполняет поля окна по блоку `m_pcBlockList` в единицах `m_pcParam`.
	///
	/// Смысл четырёх полей границ зависит от типа блока: прямоугольник и блок FILE —
	/// Xmin, Xmax, Ymin, Ymax (единицы длины); круговой сектор — Rmin, Rmax (длина) и
	/// FImin, FImax (единицы угла); вход INPUT_X / INPUT_Y — границы по Y / X (длина)
	/// и Tmin, Tmax (единицы времени). Для прочих типов подписи пустые. Точки
	/// многоугольника — первые 10 (в окне 10 пар полей). Память блока — МБ, три знака.
	/// Без параметров или блока поля остаются пустыми.
	/// @note Блоки плазмы и проводимости ядра X (`*_STAT_N`, `*_STAT_B`, `*_STAT_Y`,
	/// `*_STAT_S`, `FILE_N`, `FILE_B`, `FILE_Y`) в сравнения по типу не входят: подписи
	/// границ пустые, Ymin/Ymax — 0, Xmin/Xmax выводятся без подписи.
	void PrepareData( void );
	/// @brief Создаёт окно без блока и параметров, номер блока 0.
	/// @param[in] pParent родительское окно; NULL — главное окно программы.
	CTmcRTH_DialogBlock(CWnd* pParent = NULL);   // standard constructor
	/// Параметры шага (единицы длины, угла, времени); задаёт окно Statistics; не владеет.
	CTmcRTH_IndanParam * m_pcParam;
	/// Показываемый блок; задаёт окно Statistics; не владеет.
	CTmcRTH_BlockList  * m_pcBlockList;
	/// Номер блока в списке для следующего показа: Next — +1, Previous — -1 (не меньше 0), OK — -1 (конец просмотра).
	int i;
	/// Координаты X точек многоугольника, строки в единицах длины (до 10 точек).
	CString dX[10];
	/// Координаты Y точек многоугольника, строки в единицах длины (до 10 точек).
	CString dY[10];

// Dialog Data
	//{{AFX_DATA(CTmcRTH_DialogBlock)
	/// Номер разметки окна в ресурсах.
	enum { IDD = IDD_TMC_RTH_TOPBLOCK };
	/// Номер блока в задании.
	int		           m_nBlock;
	/// Числовой тип блока (значение ядра `GetnType()`).
	int		           m_nType;
	/// Память блока, МБ (три знака после запятой).
	double	           m_nMemory;
	/// Число точек контура блока.
	int		           m_nXY;
	/// Имя типа блока из задания (RECT_STAT, CIRC_STAT, INPUT_X, FILE …).
	CString            m_csString;
	/// Строка выражения Vx блока из задания (поле IDC_TMCBLOCKVXSPEED).
	CString            m_csVx;
	/// Строка выражения Vy блока из задания (поле IDC_TMCBLOCKVYSPEED).
	CString            m_csVy;
	/// Строка выражения W блока из задания (поле IDC_TMCBLOCKWSPEED).
	CString            m_csW;
	/// Выражение eps (или тип) блока из задания; у блоков плазмы ядра X — выражение
	/// величины блока (`*_STAT_N` — концентрация N, м^-3; `*_STAT_B` — поле B, Тл;
	/// `*_STAT_Y` — величина Y; `*_STAT_S` — проводимость sigma, См/м).
	CString            m_csEpsExpr;
	/// Координата X0 блока (значение `GetX0()`), единицы длины.
	double dX0;
	/// Координата Y0 блока (значение `GetY0()`), единицы длины.
	double dY0;
	/// @name Поля точек многоугольника 1…10 в окне
	/// Копии `dX[0..9]`, `dY[0..9]`, связанные с полями окна (строки в единицах длины; пусто — точки нет).
	///@{
	/// Координата точки многоугольника, строка в единицах длины; пусто — точки нет.
	CString dX1;
	CString dY1;
	CString dX2;
	CString dY2;
	CString dX3;
	CString dY3;
	CString dX4;
	CString dY4;
	CString dX5;
	CString dY5;
	CString dX6;
	CString dY6;
	CString dX7;
	CString dY7;
	CString dX8;
	CString dY8;
	CString dX9;
	CString dY9;
	CString dX10;
	CString dY10;
	///@}
	/// @name Подписи полей границ
	/// «Xmin/Xmax/Ymin/Ymax», «Rmin/Rmax/FImin/FImax» или «Ymin/Ymax/Tmin/Tmax» — по типу блока.
	///@{
	/// Подпись поля границы (зависит от типа блока).
	CString csXminText;
	CString csXmaxText;
	CString csYminText;
	CString csYmaxText;
	///@}
	/// @name Границы блока
	/// Значения четырёх полей границ; смысл и единицы — по подписям (см. `PrepareData`).
	///@{
	/// Значение границы блока в единицах задания (смысл — по подписи).
	double dXmin;
	double dXmax;
	double dYmin;
	double dYmax;
	///@}
	/// Обозначение единицы длины задания.
	CString            m_csLongUnit;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTH_DialogBlock)
	protected:
	/// @brief Обмен данными полей окна (только вывод значений).
	/// @param[in,out] pDX контекст обмена DDX.
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTmcRTH_DialogBlock)
	/// @brief Кнопка OK: закончить просмотр блоков (`i` = -1).
	virtual void OnOK();
	/// @brief Кнопка «Close»: закрыть окно с результатом IDCANCEL (просмотр заканчивается).
	afx_msg void OnCancel1();
	/// @brief Кнопка «Next»: показать следующий блок (`i` + 1).
	afx_msg void OnTmcblockbuttonnext();
	/// @brief Кнопка «Previous»: показать предыдущий блок (`i` - 1, не меньше 0).
	afx_msg void OnTmcblockbuttonprev();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
