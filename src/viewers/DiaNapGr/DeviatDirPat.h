/// @file DeviatDirPat.h
/// @brief Окно Calculate Directional Pattern Deviation (`IDD_DIALOGCALCDEVIAT`): ДН решётки на сетке требуемой ДН и файл сравнения `.dev`.
///
/// Часть модуля «Вьювер TMC_DN». Открывается кнопкой окна Characteristics (`CDialogDoc::OnAddMakedeviation2`).
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#deviation-pattern
#if !defined(AFX_DEVIATDIRPAT_H__22FC9C4F_6D48_43E2_A247_D13473AE581D__INCLUDED_)
#define AFX_DEVIATDIRPAT_H__22FC9C4F_6D48_43E2_A247_D13473AE581D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DeviatDirPat.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDeviatDirPat dialog

/// @brief Окно сравнения ДН решётки с требуемой ДН.
///
/// Берёт распределение амплитуд и фаз из файла `.DAT`, сетку углов и требуемую ДН — из файла
/// `.opt`, считает нормированную ДН решётки в этих точках и записывает файл `.dev`
/// (угол, требуемая, рассчитанная). Расчёт выполняется сразу после выбора пути `.dev` и
/// повторно по OK. Ошибка — окно сообщения, окно остаётся открытым. Создаётся на стеке.
/// @see https://www.tamic.ru/docs/theory/radiation.html#deviation
class CDeviatDirPat : public CDialog
{
// Construction
public:
	/// @brief Создаёт окно с пустыми путями.
	/// @param[in] pParent родительское окно; NULL — главное окно
	CDeviatDirPat(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDeviatDirPat)
	/// Номер разметки окна в ресурсах.
	enum { IDD = IDD_DIALOGCALCDEVIAT };
	/// Путь файла результата `.dev`.
	CString	m_csFileDevDirPat;
	/// Путь файла излучателей `.DAT` (распределение амплитуд и фаз).
	CString	m_csDistrAmplFaza;
	/// Путь файла требуемой ДН `.opt` (строки «угол_градусы значение»).
	CString	m_csRequireDirPat;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeviatDirPat)
	protected:
	/// @brief Обмен трёх путей с полями окна (DDX).
	/// @param[in,out] pDX контекст обмена
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDeviatDirPat)
	/// @brief Кнопка Change у поля файла результата: выбор пути `*.dev` и сразу расчёт.
	afx_msg void OnButtondeviatdirpattern();
	/// @brief Кнопка Change у поля распределения: выбор файла `*.dat`.
	afx_msg void OnButtondistramplfaza();
	/// @brief Кнопка Change у поля требуемой ДН: выбор файла `*.opt`.
	afx_msg void OnButtonrequrdirectpattern();
	/// @brief OK: расчёт и запись `.dev`; при ошибке окно не закрывается.
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	/// @brief Считает ДН решётки из `.DAT` на сетке углов `.opt` и пишет файл `.dev`.
	///
	/// Первая решётка читает `.DAT` (секторы учитываются); вторая берёт из `.opt` сетку и
	/// требуемую ДН (`OptimizationInit`), получает частоту, выражение ДН элемента, излучатели и их
	/// секторы первой и пишет сравнение (`SaveDeviationDirectionalPattern`).
	/// @return TRUE — файл записан; FALSE — ошибка (показана окном сообщения).
	BOOL CalculateDeviationDirectionalPattern( void );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVIATDIRPAT_H__22FC9C4F_6D48_43E2_A247_D13473AE581D__INCLUDED_)
