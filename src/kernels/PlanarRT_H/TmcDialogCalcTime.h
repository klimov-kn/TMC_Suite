// TmcDialogCalcTime.h : window "Calculation time" (TIMING)
//
// Shows the report of CTmcCalcTiming: time of every phase of the calculation
// (preprocessor, preparation, pure calculation, disk writes, postprocessor)
// and saves it to a text file.

/// @file TmcDialogCalcTime.h
/// @brief Окно «Calculation time»: отчёт о времени этапов расчёта и его сохранение в текстовый файл.
///
/// Часть модуля «Оболочка счётного ядра». Окно вызывается кнопкой «Calc. time...»
/// окна Statistics. Отчёт строит `CTmcCalcTiming::BuildReport()` (общий заголовок
/// `tmccalctiming.h`). Без ключа /Time отчёт виден только здесь; с ключом /Time ядро
/// само сохраняет его рядом с .tpl после команды Run.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#statistics
#ifndef _TMCDIALOGCALCTIME_H__
#define _TMCDIALOGCALCTIME_H__

#include <TmcCalcTiming.h>

/// @brief Окно отчёта о времени этапов расчёта (препроцессор, подготовка, счёт, запись, постобработка).
///
/// Создаётся на стеке окном Statistics; замер времени принадлежит документу.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#statistics
class CTmcDialogCalcTime : public CDialog
{
public:
	/// @brief Создаёт окно без замера времени (`m_pcTiming` = NULL).
	/// @param[in] pParent родительское окно; NULL — главное окно программы.
	CTmcDialogCalcTime(CWnd* pParent = NULL);
	/// Номер разметки окна в ресурсах.
	enum { IDD = IDD_TMC_CALCTIME };
	/// Замер времени документа; NULL — окно показывает «No data»; не владеет.
	CTmcCalcTiming *m_pcTiming;		// timing of the document (not owned)

protected:
	/// @brief Настраивает окно: моноширинный шрифт текста, отчёт, фокус на кнопке закрытия.
	/// @return FALSE — фокус поставлен самим окном.
	virtual BOOL OnInitDialog();
	/// @brief Кнопка «Refresh»: перестраивает отчёт (во время расчёта значения растут).
	afx_msg void OnRefresh();
	/// @brief Кнопка «Export»: сохраняет отчёт в текстовый файл (имя по умолчанию — от имени задания) и сообщает итог.
	afx_msg void OnExport();
	DECLARE_MESSAGE_MAP()

private:
	/// Моноширинный шрифт текста отчёта (Courier New, 9 пт): отчёт — таблица.
	CFont m_cFont;
	/// @brief Выводит текст отчёта в поле окна.
	void ShowReport( void );
};

#endif
