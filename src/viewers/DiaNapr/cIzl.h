// cIzl.h: interface for the cIzl class.
//
//////////////////////////////////////////////////////////////////////

/// @file cIzl.h
/// @brief Один излучатель решётки `cIzl`: координаты, комплексная амплитуда, сектор излучения и собственная ДН.
///
/// Часть модуля «Вьювер TMC_DN» (расчётная часть, каталог `viewers/dianapr`).
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#element-pattern
#if !defined(AFX_CIZL_H__1A859E7A_D5A1_48CB_A6DC_A4A0A4725C1F__INCLUDED_)
#define AFX_CIZL_H__1A859E7A_D5A1_48CB_A6DC_A4A0A4725C1F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/// @name Вид ДН излучателя (`nTypeEmitterDirectPat`)
/// Распознаётся по тексту выражения: быстрые случаи считаются без библиотеки выражений.
///@{
/// Изотропный излучатель: выражение пустое или `1.`; множитель 1.
#define TMC_IZL_DIRPATTYPE_ALL 0
/// Выражение `sin(f)`: множитель sin(phi) (ядро так помечает составляющую Hx).
#define TMC_IZL_DIRPATTYPE_SIN 1
/// Выражение `cos(f)`: множитель cos(phi) (ядро так помечает составляющую Hy).
#define TMC_IZL_DIRPATTYPE_COS 2
/// Произвольное выражение от `f` (радианы): вычисляется библиотекой выражений в каждой точке.
#define TMC_IZL_DIRPATTYPE_ARB -1
///@}

/// @brief Излучатель плоской решётки: положение, амплитуда и фаза, сектор излучения FiMin..FiMax и ДН излучателя.
///
/// Хранится в массиве `c2DArray::pcIzl`; заполняется `c2DArray::ReadData` из строки файла
/// `.DAT`. Вклад излучателя в поле решётки в направлении phi — `GetdAmplituda(phi)`:
/// амплитуда, умноженная на его ДН, и практически 0 (`FLT_MIN`) вне сектора.
/// Единицы внутри объекта: координаты — м, фаза и углы сектора — радианы.
/// @see https://www.tamic.ru/docs/code/tmc-dn.html#element-pattern
class cIzl  
{
public:
	/// @brief Возвращает амплитуду излучателя в направлении `dAngle` с учётом его ДН и сектора.
	/// @param[in] dAngle направление, радианы
	/// @return A * g(phi), где g — `dGetDirectionalPatternEmitter`; 0 — амплитуда меньше `FLT_MIN`.
	double GetdAmplituda( double dAngle );
	/// @brief Возвращает значение ДН излучателя g(phi) в направлении `dAngle`.
	///
	/// Пустое выражение — 1; направление вне сектора — `FLT_MIN` (практически 0); `1.`, `sin(f)`,
	/// `cos(f)` — напрямую; иное выражение — библиотекой выражений с `f` = угол (радианы).
	/// @param[in] dAngle направление, радианы
	/// @return g(phi); 0 — ошибка вычисления выражения.
	double dGetDirectionalPatternEmitter( double dAngle );
	/// @brief Проверяет, попадает ли направление в сектор излучения FiMin..FiMax.
	///
	/// Б-12 (3), 29.10.2026: сектор — начало FiMin mod 2*pi и ширина FiMax - FiMin (отрицательная —
	/// сектор через 0, ширина 2*pi и больше — полный круг); угол приводится к [0; 2*pi) через fmod.
	/// Работают любые секторы, в том числе 0..360, -100..190, -30..200 (раньше сектор, содержащий
	/// одновременно 0° и 180°, считался пустым). Пустое выражение ДН — всегда TRUE.
	/// @param[in] dAngle направление, радианы
	/// @return TRUE — направление в секторе.
	/// Поля `dFimin`, `dFimax` не меняются (раньше метод переписывал их приведёнными значениями).
	/// @see https://www.tamic.ru/docs/code/tmc-dn.html#sectors
	BOOL bIsAngleInDiapazon( double dAngle );
	/// @brief Возвращает конец сектора излучения, радианы.
	/// @return FiMax, радианы.
	double GetdFimax( void );
	/// @brief Задаёт конец сектора излучения.
	/// @param[in] dBuf FiMax, радианы
	void SetdFimax( double dBuf );
	/// @brief Возвращает начало сектора излучения, радианы.
	/// @return FiMin, радианы.
	double GetdFimin( void );
	/// @brief Задаёт начало сектора излучения.
	/// @param[in] dBuf FiMin, радианы
	void SetdFimin( double dBuf );
	/// @brief Задаёт выражение ДН излучателя (вариант для строки C; см. вариант с `CString`).
	/// @param[in] pszBuf выражение от `f` (радианы); пустое — изотропный излучатель
	void SetcsEmitterDirectPat( char *pszBuf );
	/// @brief Задаёт выражение ДН излучателя и определяет его вид (`TMC_IZL_DIRPATTYPE_...`).
	/// @param[in] csBuf выражение от `f` (радианы): `1.`, `sin(f)`, `cos(f)` или произвольное; пустое — изотропный излучатель
	void SetcsEmitterDirectPat( CString& csBuf );
	/// @brief Возвращает выражение ДН излучателя.
	/// @return Ссылка на внутреннюю строку.
	CString& GetcsEmitterDirectPat( void );
	/// @brief Копирует все параметры излучателя.
	/// @param[in] cIzl1 источник
	/// @return Ссылка на этот объект.
	cIzl& operator=( cIzl& cIzl1 );
	/// @brief Возвращает амплитуду излучателя (без ДН).
	/// @return Амплитуда, единицы файла `.DAT`.
	double GetdAmplituda( void );
	/// @brief Возвращает фазу излучателя.
	/// @return Фаза, радианы.
	double GetdFaza( void );
	/// @brief Возвращает координату X.
	/// @return X, м.
	double GetdXcoordinata( void );
	/// @brief Возвращает координату Y.
	/// @return Y, м.
	double GetdYcoordinata( void );
	/// @brief Задаёт амплитуду.
	/// @param[in] dR амплитуда, единицы файла `.DAT`
	void SetdAmplituda( double dR );
	/// @brief Задаёт фазу.
	/// @param[in] dR фаза, радианы
	void SetdFaza( double dR );
	/// @brief Задаёт координату X.
	/// @param[in] dR X, м
	void SetdXcoordinata( double dR );
	/// @brief Задаёт координату Y.
	/// @param[in] dR Y, м
	void SetdYcoordinata( double dR );
	/// @brief Создаёт изотропный излучатель в начале координат с нулевой амплитудой.
	cIzl();
	/// @brief Уничтожает объект; действий нет.
	virtual ~cIzl();

private:
	/// Конец сектора излучения, радианы (из FiMax файла `.DAT`, градусы).
	double dFimax;
	/// Начало сектора излучения, радианы (из FiMin файла `.DAT`, градусы).
	double dFimin;
	/// Выражение ДН излучателя (последнее поле строки `.DAT`); пусто — изотропный.
	CString csEmitterDirectPat;
	/// Вид ДН излучателя, `TMC_IZL_DIRPATTYPE_...`.
	int nTypeEmitterDirectPat;
	/// Амплитуда, единицы файла `.DAT`.
	double dAmplituda;
	/// Фаза, радианы.
	double dFaza;
	/// Координата X, м.
	double dXcoordinata;
	/// Координата Y, м.
	double dYcoordinata;
};

#endif // !defined(AFX_CIZL_H__1A859E7A_D5A1_48CB_A6DC_A4A0A4725C1F__INCLUDED_)
