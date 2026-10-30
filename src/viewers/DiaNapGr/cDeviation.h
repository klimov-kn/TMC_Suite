/// @file cDeviation.h
/// @brief Окно Make Distribution Deviation (`IDD_DIALOGMAKEDEVIAT`): решётка со случайными отклонениями амплитуд, фаз и координат.
///
/// Часть модуля «Вьювер TMC_DN». Открывается кнопкой окна Characteristics (`CDialogDoc::OnAddMakedeviation`).
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#deviation
#if !defined(AFX_CDEVIATION_H__EFD8FF16_B805_4DAE_A799_E84714E00F81__INCLUDED_)
#define AFX_CDEVIATION_H__EFD8FF16_B805_4DAE_A799_E84714E00F81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// cDeviation.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// cDeviation dialog

/// @brief Окно девиаций распределения: из файла `.DAT` строит новый `.DAT`, отклоняя параметры излучателей случайно.
///
/// Предельные отклонения X, Y (мм), амплитуды (%) и фазы (градусы) вводятся в окне; сам
/// расчёт — `c2DArray::MakeDeviation`. Файл пишется сразу после выбора пути «To» и
/// повторно по OK. Ошибка показывается окном сообщения, окно остаётся открытым.
/// Создаётся на стеке обработчиком окна Characteristics; по умолчанию амплитуда и фаза — 10.
class cDeviation : public CDialog
{
// Construction
public:
	/// @brief Создаёт окно с пустыми путями и нулевыми отклонениями.
	/// @param[in] pParent родительское окно; NULL — главное окно
	cDeviation(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(cDeviation)
	/// Номер разметки окна в ресурсах.
	enum { IDD = IDD_DIALOGMAKEDEVIAT };
	/// Путь исходного файла излучателей `.DAT` (поле «From»).
	CString	m_csFileFrom;
	/// Путь файла `.DAT` с отклонениями (поле «To»).
	CString	m_csFileTo;
	/// Предельное отклонение координаты X, мм.
	double	m_dXdev;
	/// Предельное отклонение координаты Y, мм.
	double	m_dYdev;
	/// Предельное отклонение фазы, градусы.
	double	m_dFazaDev;
	/// Предельное отклонение амплитуды, % от амплитуды излучателя.
	double	m_dAmpDev;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cDeviation)
	protected:
	/// @brief Обмен путей и отклонений с полями окна (DDX).
	/// @param[in,out] pDX контекст обмена
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(cDeviation)
	/// @brief Кнопка Change у поля «From»: выбор исходного файла `*.dat`.
	afx_msg void OnButtonsource();
	/// @brief Кнопка Change у поля «To»: выбор файла для записи и сразу расчёт (`MakeDeviation`).
	afx_msg void OnButtondistination();
	/// @brief OK: расчёт и запись файла; при ошибке окно не закрывается.
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	/// @brief Читает поля окна и вызывает `c2DArray::MakeDeviation`; ошибку показывает окном сообщения.
	/// @return TRUE — файл записан; FALSE — ошибка.
	BOOL MakeDeviation( void );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDEVIATION_H__EFD8FF16_B805_4DAE_A799_E84714E00F81__INCLUDED_)
