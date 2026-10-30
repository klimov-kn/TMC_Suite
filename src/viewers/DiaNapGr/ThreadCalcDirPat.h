/// @file ThreadCalcDirPat.h
/// @brief Окно прогресса расчёта ДН (`IDD_READDATA_THREAD`): номер ДН, проценты, время, число излучателей.
///
/// Часть модуля «Вьювер TMC_DN». Показывается модально, пока поток расчёта
/// `ReadDocFileW_Thread1` читает документ и считает ДН всех графиков.
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#thread
#if !defined(AFX_THREADCALCDIRPAT_H__50BE87B7_7475_4BB6_A105_69EC73F63BF8__INCLUDED_)
#define AFX_THREADCALCDIRPAT_H__50BE87B7_7475_4BB6_A105_69EC73F63BF8__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ThreadCalcDirPat.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CThreadCalcDirPat dialog

/// @brief Модальное окно прогресса фонового расчёта ДН.
///
/// Создаётся на стеке видом (`CTMCGROUTView::ReadDocFileW` и `OnDestroy`), получает указатель
/// на вид в `pCView`. Таймер 1 (каждые 100 мс) читает из документа номер текущего графика
/// `nGraphCurrent` и ход цикла `c2DArray::iN` и обновляет поля. Окно закрывается само, когда
/// поток расчёта сбрасывает флаг `g_lTmcDnCalcRunning`; кнопка Hide и крестик во время
/// расчёта не действуют.
/// @note Потоки: окно — в главном потоке; данные расчёта читаются без синхронизации (только показ).
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#thread
class CThreadCalcDirPat : public CDialog
{
// Construction
public:
	/// Вид-владелец (`CTMCGROUTView*`), задаёт создатель; NULL — поля прогресса не обновляются.
	void * pCView;
	/// @brief Создаёт окно: ДН 1 из 1, пустые поля, `pCView` = NULL.
	/// @param[in] pParent родительское окно; NULL — главное окно
	CThreadCalcDirPat(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CThreadCalcDirPat)
	/// Номер разметки окна в ресурсах.
	enum { IDD = IDD_READDATA_THREAD };
	/// Полоса общего хода по всем графикам документа, 0..100.
	CProgressCtrl	m_ProgressAll;
	/// Полоса хода текущей ДН, 0..100 (`iN/10`).
	CProgressCtrl	m_Progress;
	/// Ход текущей ДН, текст `NN.N%`.
	CString	m_csStatistics;
	/// Общий ход, текст `NN.N%`: (iN/10 + 100*номер графика) / число графиков.
	CString	m_csStatisticsAll;
	/// Номер рассчитываемой ДН (графика), с 1.
	int		m_CurrentDirPat;
	/// Число ДН (графиков) в документе.
	int		m_NAllDirPat;
	/// Время с начала расчёта: `сс`, `мм:сс` или `чч:мм:сс`.
	CString	m_csElapsTime;
	/// Число излучателей текущей решётки.
	int		m_nEmitters;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CThreadCalcDirPat)
	public:
	/// @brief Запоминает время начала и показывает окно.
	/// @return Код закрытия окна (0 — закрыто по окончании расчёта).
	virtual INT_PTR DoModal();
	protected:
	/// @brief Обмен полей прогресса с окном (DDX) и привязка полос прогресса.
	/// @param[in,out] pDX контекст обмена
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CThreadCalcDirPat)
	/// @brief Таймер 1 (100 мс): закрывает окно по окончании расчёта, иначе обновляет поля и полосы.
	/// @param[in] nIDEvent номер таймера
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	/// @brief Запускает таймер обновления (100 мс).
	/// @return TRUE — фокус ставит каркас.
	virtual BOOL OnInitDialog();
	/// @brief Кнопка Hide: во время расчёта не действует, после — закрывает окно.
	virtual void OnCancel();	// A-30
	/// @brief Крестик окна: во время расчёта не действует.
	virtual void OnClose();		// A-30
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	/// Текущее время при последнем обновлении, с.
	time_t ltime_current;
	/// Время начала расчёта, с (берётся у вида `CTMCGROUTView::ltime_start`).
	time_t ltime_start;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_THREADCALCDIRPAT_H__50BE87B7_7475_4BB6_A105_69EC73F63BF8__INCLUDED_)
