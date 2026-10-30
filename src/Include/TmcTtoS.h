/// @file TmcTtoS.h
/// @brief Пересчёт сигналов портов `.t` в столбец S-матрицы `.s` на одной частоте (класс `CTmcTtoS`).
///
/// Общий заголовок. Реализация `viewers/tmcrtout/tmcttos.cpp` собирается во вьювер TMCROS
/// (команда Edit > To S-matrix, окно `CDialogTtoS`) и в библиотеки ядер `planarrt_h_ttos`,
/// `planarrt_x_ttos` (директива `SMATRIX` задания, `CTmcRTH_IndanOutput::PutSmatrix`).
/// Файл `.t` многочастотного расчёта обрабатывает не этот класс, а `CTmcWideband`.
/// @see https://www.tamic.ru/docs/code/tmcros.html#ttos
#if !defined(AFX_TMCTTOS_H__F6739481_DF94_11D3_A946_508A4DC10000__INCLUDED_)
#define AFX_TMCTTOS_H__F6739481_DF94_11D3_A946_508A4DC10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TmcTtoS.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcTtoS window

#include "TmcSMatrix.h"

/// @brief Преобразование «время -> S» на одной частоте: читает `.t`, строит столбец S-матрицы, дописывает его в `.s`.
///
/// Обёртка над `CTmcSMatrix`: `TtoS(...)` запоминает имена файлов, окно времени и частоту
/// записи, затем вызывает `MakeSmatrixFromFileT` (чтение отсчётов и расчёт столбца) и
/// `SaveSmatrixToFileS` (запись). Объект создаётся на стеке вызывающим кодом
/// (`CDialogTtoS::ConvertOne` во вьювере, `CTmcRTH_IndanOutput::PutSmatrix` в ядре); один
/// вызов `TtoS` — одно преобразование, повторный вызов сбрасывает прежнее состояние.
/// Исключений нет: признак ошибки — `IsError()`, текст — `GetErrorMessage()`.
/// @note Потоки: во вьювере — главный поток, в ядре — расчётный поток шага; общих данных
///       между объектами нет.
/// @warning Значения S считаются на частоте из заголовка `.t` (строка `#TMC_GROTS_Freq`,
///          FREQ шага), а в `.s` записываются с частотой `dFreq1` вызова. Если они различаются,
///          точка `.s` подписана не своей частотой.
/// @see https://www.tamic.ru/docs/code/tmcros.html#ttos
class CTmcTtoS
{
// Construction
public:
	/// @brief Создаёт пустой объект: имена файлов пустые, окно и частота нулевые, ошибки нет.
	CTmcTtoS();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcTtoS)
	//}}AFX_VIRTUAL

// Implementation
public:
	/// @brief Дописывает рассчитанный столбец в файл `.s` (`CTmcSMatrix::Save`) с частотой `dFreq`.
	///
	/// При ранее возникшей ошибке ничего не делает. Ошибка записи — признак и текст объекта.
	void SaveSmatrixToFileS( void );
	/// @brief Возвращает текст последней ошибки.
	/// @return Ссылка на строку объекта; `No error` — ошибки нет.
	CString & GetErrorMessage( void );
	/// @brief Б-3.3: предупреждение последнего преобразования (`CTmcSMatrix::GetWarningMessage`).
	/// @return Ссылка на строку; пустая — предупреждений нет.
	CString & GetWarningMessage( void ) { return cSMatrix.GetWarningMessage(); }	// B-3.3
	/// @brief Читает отсчёты `.t` в окне `dTmin..dTmax` и вычисляет столбец S (`CTmcSMatrix::ReadTFile`, `MakeSmatrix`).
	///
	/// Ошибка чтения или расчёта переносится в признак и текст объекта.
	void MakeSmatrixFromFileT( void );
	/// @brief Возвращает признак ошибки последнего преобразования.
	/// @return TRUE — была ошибка (текст — `GetErrorMessage`); FALSE — успех.
	BOOL IsError( void );
	/// @brief Выполняет преобразование: `.t` -> столбец S-матрицы -> дописывание в `.s`.
	///
	/// Сбрасывает прежнее состояние, запоминает параметры и вызывает `MakeSmatrixFromFileT`
	/// и `SaveSmatrixToFileS`. Номер столбца — номер возбуждённого входа (вход с ненулевой
	/// падающей волной), он должен быть ровно один.
	/// @param[in] csTfn  путь к файлу сигналов `.t`
	/// @param[in] csSfn  путь к файлу `.s`; нет файла — создаётся, частота уже есть — столбец
	///                   заменяется, иначе точка добавляется (`save_S_matrix_element`)
	/// @param[in] dTmin1 начало окна времени, с; берутся отсчёты с `dTmin1 <= t <= dTmax1`
	/// @param[in] dTmax1 конец окна времени, с; должно быть больше `dTmin1`
	/// @param[in] dFreq1 частота, под которой точка записывается в `.s`, Гц
	/// @see https://www.tamic.ru/docs/code/tmcros.html#ttos
	/// @see https://www.tamic.ru/docs/theory/smatrix.html#s-from-signals
	void TtoS( CString csTfn, CString csSfn, double dTmin1, double dTmax1,  double dFreq1 );
	/// @brief Освобождает данные вложенного `CTmcSMatrix`.
	virtual ~CTmcTtoS();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcTtoS)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
private:
	/// Чтение `.t`, расчёт и запись столбца S; принадлежит объекту.
	CTmcSMatrix cSMatrix;
	/// @brief Выполняет расчёт и запись по уже запомненным параметрам (без сброса состояния).
	void TtoS( void );
	/// @brief Сбрасывает параметры, признак ошибки и данные `cSMatrix`.
	void DeleteData( void );
	/// Путь к файлу `.s` результата.
	CString csSFileName;
	/// Путь к файлу сигналов `.t`.
	CString csTFileName;
	/// Текст последней ошибки; `No error` — ошибки нет.
	CString csError;
	/// Начало окна времени, с.
	double dTmin;
	/// Конец окна времени, с.
	double dTmax;
	/// Частота записи точки в `.s`, Гц (не частота расчёта — та берётся из `.t`).
	double dFreq;
	/// TRUE — была ошибка.
	BOOL bIsError;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMCTTOS_H__F6739481_DF94_11D3_A946_508A4DC10000__INCLUDED_)
