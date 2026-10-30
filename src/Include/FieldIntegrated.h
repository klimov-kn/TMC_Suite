// FieldIntegrated.h: interface for the CFieldIntegrated class.
//
//////////////////////////////////////////////////////////////////////

/// @file FieldIntegrated.h
/// @brief Накопление гармоники поля во времени и построение файла излучателей
///        для диаграммы направленности (ДН).
///
/// Часть библиотеки TMCIndan. Класс `CFieldIntegrated` по команде
/// `FIELD_DISTRIBUTION[_M]` секции `#OUTPUT` накапливает на каждом шаге
/// по времени квадратуры напряжения узлов на частоте F (или на нескольких
/// тонах), по ним считает амплитуду и фазу, а затем вдоль прямоугольного
/// контура вблизи границы области записывает эквивалентные излучатели
/// в файл `.DAT`, по которому программа TMC_DN строит ДН.
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#field-integrated
/// @see https://www.tamic.ru/docs/theory/radiation.html

#if !defined(AFX_FIELDINTEGRATED_H__C0B2DF6A_836F_4331_9C70_3DB935CE0EA1__INCLUDED_)
#define AFX_FIELDINTEGRATED_H__C0B2DF6A_836F_4331_9C70_3DB935CE0EA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <TmcLibError.h>
#include <TmcRTH_IndanParam.h>


/// @brief Интегратор поля на частоте F и построитель файла излучателей ДН.
///
/// Жизненный цикл: `SetFile`, `SetFreq`, `SetTmin`, `SetTmax` (и при `_M`
/// `SetIntegrateInMemory`) — при разборе `#OUTPUT`; `SetNodeStep` и
/// `InitAllFile` — ядро перед счётом шага; `SetFreqList` — ядро, если падающая
/// волна несёт несколько тонов; `Integrate` — на каждом шаге по времени;
/// по достижении `Tmax` `Integrate` сам считает амплитуду и фазу и пишет `.DAT`.
///
/// Накапливаются квадратуры напряжения узла U (код называет его Ez):
/// `C = sum U(t)*cos(2*pi*F*t)*dT`, `S = sum U(t)*sin(2*pi*F*t)*dT` по окну
/// Tmin..Tmax; амплитуда `A = 2*sqrt(C^2 + S^2)/(Tmax - Tmin)`,
/// фаза `atan2(S, C)` (в единицах `ANGLE_UNIT`).
///
/// Режимы хранения: `FIELD_DISTRIBUTION` — квадратуры в файлах `.COS`/`.SIN`
/// (чтение-запись файла на каждом шаге, очень медленно); `FIELD_DISTRIBUTION_M` —
/// в памяти, по всей области или (на больших сетках и при нескольких тонах)
/// только в четырёх полосах шириной 3 узла вдоль контура интегрирования.
/// Владелец — `CTmcRTH_IndanOutput` (поле `cFieldDistr`).
/// @warning Фаза в `.FAZ` и `prFaz` делится на множитель `ANGLE_UNIT`, а построение
///          `.DAT` подставляет её в `cos`/`sin` и переводит в градусы как радианы:
///          при `ANGLE_UNIT gradus` фазы излучателей будут неверны.
/// @note Потоки: вызывается расчётным потоком ядра; не потокобезопасен.
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#field-integrated
/// @see https://www.tamic.ru/docs/theory/radiation.html#contour
class CFieldIntegrated
{
public:
	/// @brief Копирует параметры шага и строит файл излучателей `.DAT` (см. приватную перегрузку).
	/// @param[in] cParam1 параметры шага (единицы длины, времени и угла)
	/// @see https://www.tamic.ru/docs/theory/radiation.html#pattern
	void ExportToDirectionalPattern( CTmcRTH_IndanParam &cParam1 );
	/// @brief Возвращает имя файла излучателей.
	/// @return Путь `<имя>.DAT`.
	CString GetcsDirectPatternFile( void );
	/// @brief Накапливаются ли квадратуры в памяти (`FIELD_DISTRIBUTION_M`).
	/// @return TRUE — в памяти; FALSE — в файлах `.COS`/`.SIN`.
	BOOL bIsItegratedInMemory( void );
	/// @brief Включает накопление квадратур в памяти (`FIELD_DISTRIBUTION_M`).
	void SetIntegrateInMemory( void );
	/// @brief Добавляет вклад текущего шага по времени; закрывает окно и пишет ДН по его окончании.
	///
	/// Вне окна `[Tmin, Tmax]` ничего не делает. В окне прибавляет `U*cos*dT`
	/// и `U*sin*dT` к квадратурам (для каждого тона при нескольких тонах).
	/// На первом шаге с `t >= Tmax` после начала окна: считает амплитуду и фазу,
	/// пишет `.AMP`/`.FAZ` (кроме режима полос) и файл излучателей `.DAT`.
	/// @param[in] dTCurrent1 текущее время, с
	/// @param[in] nTCurrent1 номер текущего шага по времени
	/// @param[in] prUNode1   массив значений узлов ядра, начиная с нужной компоненты;
	///                       значение узла n — `prUNode1[n*nNodeStep]`
	/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#dn-accumulation
	void Integrate( double dTCurrent1, int nTCurrent1, _real *prUNode1 );
	/// @brief Задаёт число значений на узел в массиве ядра (шаг между узлами).
	/// @param[in] nStep шаг, > 0 (по умолчанию 2 — H-ядро; X-ядро задаёт 6); <= 0 игнорируется
	void SetNodeStep( int nStep );
	/// @brief Содержит ли имя `.AMP` полный путь (начало `\` или `/`, или символ `:`).
	/// @return TRUE — путь полный.
	BOOL IsFullPath( void );
	/// @brief Приписывает папку задания к именам файлов `.SIN`, `.COS`, `.FAZ`, `.AMP`, `.DAT`,
	///        если путь не полный.
	/// @param[in] csPath папка задания
	void AddPath( CString csPath );
	/// @brief Возвращает объект ошибки.
	/// @return Ссылка на внутреннюю ошибку.
	CTmcLibError& GetError( void );
	/// @brief Есть ли ошибка.
	/// @return TRUE — ошибка была, дальнейшие вызовы ничего не делают.
	BOOL IsError( void );
	/// @brief Подготавливает накопление на шаг расчёта: сетка, шаг по времени, массивы или файлы.
	///
	/// Ничего не делает без `FIELD_DISTRIBUTION[_M]`. В режиме памяти выделяет
	/// массивы квадратур (выбирая режим полос по свободной памяти), в файловом —
	/// создаёт нулевые файлы `.SIN` и `.COS`.
	/// @param[in] nX1     число узлов по X
	/// @param[in] nY1     число узлов по Y
	/// @param[in] dDelta1 шаг сетки, м
	/// @param[in] dXmin1  X узла ix = 0, м
	/// @param[in] dYmin1  Y узла iy = 0, м
	/// @param[in] cParam1 параметры шага (копируются: единицы для заголовков файлов)
	/// @param[in] dT1     шаг по времени, с
	void InitAllFile( int nX1, int nY1, double dDelta1, double dXmin1, double dYmin1, CTmcRTH_IndanParam& cParam1, double dT1  );
	/// @brief Задана ли ДН (`FIELD_DISTRIBUTION[_M]`) в шаге.
	/// @return TRUE — задана.
	BOOL bIsFieldDistr( void );
	/// @brief Открыто ли окно накопления (начато, но ещё не закончено).
	///
	/// Ядро проверяет это после последнего шага по времени: если окно ДН
	/// кончается позже времени расчёта, файлы ДН не будут записаны — это ошибка.
	/// @return TRUE — окно начато и не закрыто.
	BOOL IsWindowOpen( void ) { return bIsBeginIntegreted; }	// A-14: okno DN nachato, no ne zakoncheno
	/// @brief Задаёт начало окна накопления.
	/// @param[in] dTmin1 время, с
	void SetTmin( double dTmin1 );
	/// @brief Возвращает начало окна накопления.
	/// @return Время, с.
	double GetTmin( void );
	/// @brief Задаёт конец окна накопления.
	/// @param[in] dTmax1 время, с (обычно Tmin + целое число периодов F)
	void SetTmax( double dTmax1 );
	/// @brief Возвращает конец окна накопления.
	/// @return Время, с.
	double GetTmax( void );
	/// @brief Задаёт частоту ДН.
	/// @param[in] dF частота, Гц (> 0)
	void SetFreq( double dF );
	/// @brief То же, что `SetFile(CString)`, для строки C.
	/// @param[in] szFile_ft базовое имя файлов
	void SetFile(char* szFile_ft);
	/// @brief Задаёт размер сетки и (в режиме памяти) заново выделяет массивы квадратур.
	/// @param[in] nX1 число узлов по X, >= 1
	/// @param[in] nY1 число узлов по Y, >= 1
	void SetnXnY( int nX1, int nY1 );
	/// @brief Задаёт число узлов по Y.
	/// @param[in] nY1 число узлов, >= 1 (иначе ошибка)
	void SetnY( int nY1 );
	/// @brief Задаёт число узлов по X.
	/// @param[in] nX1 число узлов, >= 1 (иначе ошибка)
	void SetnX( int nX1 );
	/// @brief Копирует параметры, имена файлов, ошибку и (в режиме памяти по всей
	///        области) накопленные квадратуры.
	/// @param[in] cFieldIntegr источник
	/// @return `*this`.
	CFieldIntegrated& operator=( CFieldIntegrated& cFieldIntegr );
	/// @brief Задаёт базовое имя файлов и размер сетки.
	/// @param[in] csFile_ft базовое имя
	/// @param[in] nX1       число узлов по X
	/// @param[in] nY1       число узлов по Y
	void SetFile( CString csFile_ft, int nX1, int nY1 );
	/// @brief Сбрасывает объект и задаёт базовое имя файлов; включает ДН.
	///
	/// Имена: `<имя>.SIN`, `<имя>.COS` (квадратуры), `<имя>.AMP`, `<имя>.FAZ`
	/// (амплитуда и фаза по сетке), `<имя>.DAT` (излучатели для TMC_DN).
	/// @param[in] csFile_ft базовое имя (непустое)
	void SetFile( CString csFile_ft );
	/// @brief Освобождает массивы и сбрасывает все параметры (тоны и режим полос тоже).
	/// @note Шаг узла `nNodeStep` не сбрасывается.
	void DeleteData( void );
	/// @brief Создаёт пустой интегратор (ДН не задана, шаг узла 2).
	CFieldIntegrated();
	/// @brief Уничтожает объект и освобождает массивы.
	virtual ~CFieldIntegrated();

private:
	/// @brief Читает `.AMP` и `.FAZ` целиком в массивы `prAmp`, `prFaz` (`nX*nY` значений).
	void ReadAmplitudaAndFazaEzMInit( void );
	/// @brief Строит файл излучателей `.DAT` (по одному файлу на тон при нескольких тонах).
	///
	/// При нескольких тонах имена — `<имя>_<F в МГц>MHz.DAT` с минимальным числом
	/// знаков после точки, при котором имена различаются.
	void ExportToDirectionalPattern( void );
	/// @brief Дописывает в `.DAT` три записи излучателя точки контура: Ez (множитель «1.»),
	///        Hy (множитель «cos(f)») и Hx (множитель «sin(f)»).
	///
	/// Амплитуды умножаются на `koef = sqrt(Delta/3*0.999996/0.9361811)*(Delta*F/c)/sqrt(F/c)`,
	/// фазы переводятся в градусы, координаты — в мм.
	/// @param[in] x      X точки, м
	/// @param[in] y      Y точки, м
	/// @param[in] aEz    амплитуда Ez
	/// @param[in] fEz    фаза Ez, рад
	/// @param[in] aHx    амплитуда Hx
	/// @param[in] fHx    фаза Hx, рад
	/// @param[in] aHy    амплитуда Hy
	/// @param[in] fHy    фаза Hy, рад
	/// @param[in] dFimin начало сектора излучения, градусы
	/// @param[in] dFimax конец сектора излучения, градусы
	void AddDirectionalPatternFileOneEmitter(double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy, double &dFimin, double &dFimax);
	/// @brief Возвращает амплитуду и фазу в узле, ближайшем к точке (x, y).
	///
	/// Из массивов в памяти (в режиме полос — 0 вне полос) или, если их нет,
	/// чтением файлов `.AMP`/`.FAZ`.
	/// @param[in]  x   X, м
	/// @param[in]  y   Y, м
	/// @param[out] aEz амплитуда
	/// @param[out] fEz фаза (в единицах `ANGLE_UNIT` файла)
	void ReadAmplitudaAndFazaEz(double x, double y, double &aEz, double &fEz);
	/// @brief Записывает излучатель верхней стороны контура (сектор 0..180 градусов).
	/// @param[in] x,y   точка, м
	/// @param[in] aEz,fEz Ez: амплитуда, фаза (рад)
	/// @param[in] aHx,fHx Hx: амплитуда, фаза (рад)
	/// @param[in] aHy,fHy Hy: амплитуда, фаза (рад)
	void AddDirectionalPatternFileYmax( double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy );
	/// @brief Записывает излучатель нижней стороны контура (сектор 180..360 градусов).
	/// @param[in] x,y   точка, м
	/// @param[in] aEz,fEz Ez: амплитуда, фаза (рад)
	/// @param[in] aHx,fHx Hx: амплитуда, фаза (рад)
	/// @param[in] aHy,fHy Hy: амплитуда, фаза (рад)
	void AddDirectionalPatternFileYmin( double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy );
	/// @brief Записывает излучатель правой стороны контура (сектор -90..90 градусов).
	/// @param[in] x,y   точка, м
	/// @param[in] aEz,fEz Ez: амплитуда, фаза (рад)
	/// @param[in] aHx,fHx Hx: амплитуда, фаза (рад)
	/// @param[in] aHy,fHy Hy: амплитуда, фаза (рад)
	void AddDirectionalPatternFileXmax( double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy );
	/// @brief Записывает излучатель левой стороны контура (сектор 90..270 градусов).
	/// @param[in] x,y   точка, м
	/// @param[in] aEz,fEz Ez: амплитуда, фаза (рад)
	/// @param[in] aHx,fHx Hx: амплитуда, фаза (рад)
	/// @param[in] aHy,fHy Hy: амплитуда, фаза (рад)
	void AddDirectionalPatternFileXmin( double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy );
	/// @brief Поле в точке горизонтальной стороны контура: Ez — среднее двух узлов
	///        по X, Hx — из разности Ez по Y (Hy = 0).
	///
	/// `Hx ~ c/(2*pi*F*Delta) * (разность Ez через шаг по Y)/4`, фаза Hx сдвинута на pi/2.
	/// @param[in]  x   X точки (середина между узлами), м
	/// @param[in]  y   Y точки (строка узлов), м
	/// @param[out] aEz амплитуда Ez
	/// @param[out] fEz фаза Ez, рад
	/// @param[out] aHx амплитуда Hx
	/// @param[out] fHx фаза Hx, рад
	/// @param[out] aHy 0
	/// @param[out] fHy 0
	/// @see https://www.tamic.ru/docs/theory/radiation.html#contour
	void ReadAmplitudaAndFazaY( double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy );
	/// @brief Поле в точке вертикальной стороны контура: Ez — среднее двух узлов
	///        по Y, Hy — из разности Ez по X (Hx = 0).
	/// @param[in]  x   X точки (столбец узлов), м
	/// @param[in]  y   Y точки (середина между узлами), м
	/// @param[out] aEz амплитуда Ez
	/// @param[out] fEz фаза Ez, рад
	/// @param[out] aHx 0
	/// @param[out] fHx 0
	/// @param[out] aHy амплитуда Hy
	/// @param[out] fHy фаза Hy, рад
	/// @see https://www.tamic.ru/docs/theory/radiation.html#contour
	void ReadAmplitudaAndFazaX( double &x, double &y, double &aEz, double &fEz, double &aHx, double &fHx, double &aHy, double &fHy );
	/// @brief Обходит верхнюю сторону контура и записывает её излучатели.
	///
	/// Точки — через шаг от `dDirPatXmin + Delta/2`; точка, где `cos(fHx - fEz) <= 0`,
	/// записывается с нулевыми амплитудами Ez и Hx; фазы перед записью меняют знак.
	void AddDirectionalPatternFileYmax( void );
	/// @brief Обходит нижнюю сторону контура (обнуление при `cos(fHx - fEz) >= 0`).
	void AddDirectionalPatternFileYmin( void );
	/// @brief Обходит правую сторону контура (обнуление при `cos(fHy - fEz) <= 0`).
	void AddDirectionalPatternFileXmax( void );
	/// @brief Обходит левую сторону контура (обнуление при `cos(fHy - fEz) >= 0`).
	void AddDirectionalPatternFileXmin( void );
	/// @brief Возвращает число точек вдоль вертикальной стороны контура.
	/// @return `(int)((dDirPatYmax - dDirPatYmin)/Delta)`; 0 при ошибке.
	int GetNumberOfEmittersAlongY( void );
	/// @brief Возвращает число точек вдоль горизонтальной стороны контура.
	/// @return `(int)((dDirPatXmax - dDirPatXmin)/Delta)`; 0 при ошибке.
	int GetNumberOfEmittersAlongX( void );
	/// @brief Создаёт файл `.DAT` и пишет заголовок: частоту (ГГц), число излучателей и «1.».
	void MakeDirectionalPatternFileZagolovok( void );
	/// @brief Проверяет, что контур отстоит от краёв области не меньше чем на 9 шагов,
	///        а частота и шаг сетки положительны.
	/// @return TRUE — данные верны; иначе ошибка в `cError`.
	BOOL IsDirectionalPatternDataCorrect( void );
	/// @brief Строит один файл `.DAT`: проверка, заголовок, амплитуды/фазы, четыре стороны контура.
	void MakeDirectionalPatternFile( void );
	/// @brief Задаёт контур по умолчанию — прямоугольник в 10 шагах от краёв области.
	void SetDirectionalPatternParamDefault( void );
	/// @brief Читает текстовый заголовок файла поля (`#TMCGROFLD_*`).
	/// @param[in]  pfDataA     файл, открытый в начале
	/// @param[out] nTCurrentA  номер шага по времени
	/// @param[out] nAccuracyA  размер значения, байт
	/// @param[out] nXA         узлов по X
	/// @param[out] nYA         узлов по Y
	/// @param[out] dTCurrentA  время, с
	/// @param[out] dDeltaA     шаг сетки, м
	/// @param[out] dXminA      X узла ix = 0, м
	/// @param[out] dYminA      Y узла iy = 0, м
	/// @param[out] dLongUnitA  множитель единицы длины
	/// @param[out] dTimeUnitA  множитель единицы времени
	/// @param[out] csLongUnitA имя единицы длины
	/// @param[out] csTimeUnitA имя единицы времени
	void ReadParamFrom1File( FILE *pfDataA, int &nTCurrentA, int &nAccuracyA, int &nXA, int &nYA, double &dTCurrentA, double &dDeltaA, double &dXminA, double &dYminA, double &dLongUnitA, double &dTimeUnitA, CString &csLongUnitA, CString &csTimeUnitA );
	/// @brief Читает заголовки `.AMP` и `.FAZ`, сверяет их и берёт из них сетку и единицы.
	void ReadParamFromFile( void );
	/// @brief Пишет `.AMP` и `.FAZ` по квадратурам в памяти (режим памяти по всей области).
	void MakeFileAmplitudeAndFazaFromMemory( void );
	/// @brief Прибавляет вклад шага к квадратурам в памяти (вся область или только полосы).
	/// @param[in] prUNode1    массив значений узлов ядра (шаг `nNodeStep`)
	/// @param[in] rCosCurrent `cos(2*pi*F*t)`
	/// @param[in] rSinCurrent `sin(2*pi*F*t)`
	void IntegrateInMemory( _real* prUNode1, _real rCosCurrent, _real rSinCurrent);
	/// Квадратура S по узлам (режим памяти); при нескольких тонах — подряд по тонам.
	_real* prSin;
	/// Квадратура C по узлам (режим памяти); при нескольких тонах — подряд по тонам.
	_real* prCos;
	/// TRUE — квадратуры в памяти (`FIELD_DISTRIBUTION_M`).
	BOOL bIsIntegretedInMemory;
	/// @brief Пишет `.AMP` и `.FAZ` по файлам `.SIN`/`.COS` (файловый режим).
	void MakeFileAmplitudeAndFaza( void );
	/// TRUE — окно накопления начато и не закрыто.
	BOOL bIsBeginIntegreted;
	/// @brief Прибавляет `U*r1*dT` к каждому значению файла квадратуры (файловый режим).
	/// @param[in] csFileName файл `.COS` или `.SIN`
	/// @param[in] prUNode1   массив значений узлов ядра (шаг `nNodeStep`)
	/// @param[in] r1         `cos` или `sin` от `2*pi*F*t`
	void Integrate(CString csFileName, _real *prUNode1, _real r1);
	/// Номер текущего шага по времени (пишется в заголовки `.AMP`/`.FAZ`).
	int nTCurrent;
	/// Шаг по времени, с.
	double dT;
	/// Текущее время, с.
	double dTCurrent;
	/// Амплитуды для построения ДН (по сетке или по полосам).
	_real *prAmp;
	/// Фазы для построения ДН (в единицах `ANGLE_UNIT`).
	_real *prFaz;
	/// Шаг сетки, м.
	double dDelta;
	/// Левая сторона контура интегрирования, м (`-FLT_MAX` — по умолчанию).
	double dDirPatXmin;
	/// Правая сторона контура, м.
	double dDirPatXmax;
	/// Нижняя сторона контура, м.
	double dDirPatYmin;
	/// Верхняя сторона контура, м.
	double dDirPatYmax;
	/// X узла ix = 0, м.
	double dXmin;
	/// Y узла iy = 0, м.
	double dYmin;
	/// Копия параметров шага (единицы длины, времени, угла).
	CTmcRTH_IndanParam cParam;
	/// @brief Создаёт файл квадратуры: заголовок и `nX*nY` нулей (файловый режим).
	/// @param[in] csFileName файл `.COS` или `.SIN`
	void InitFile(CString csFileName );
	/// TRUE — ДН задана в шаге (`SetFile` вызван).
	BOOL bIsFieldDistribution;
	/// Счётчик записей излучателей в текущем файле `.DAT`.
	int nEmitterInFile;
	/// Число узлов по Y.
	int nY;
	/// Число узлов по X.
	int nX;

//  ---- border-only accumulation // BORDER ----------------------------
//  A directional pattern is built from a contour, not from the whole area,
//  so on a huge grid there is no reason to keep four arrays per node.
//  In border mode only four strips (3 nodes thick) around the contour are
//  kept; BorderSlot() maps a grid node to its place inside them, or -1.
public:
	/// @brief Включён ли режим полос (квадратуры только у контура).
	/// @return TRUE — хранятся только четыре полосы шириной 3 узла.
	BOOL IsBorderOnly( void ) { return bBorderOnly; }
	/// @brief Возвращает число узлов, хранимых в полосах (на один тон).
	/// @return `6*nBW + 6*nBHm`; 0 — режим полос не включался.
	TmcIdx GetBorderSize( void ) { return nBorderSize; }
	/// @brief Выбирает режим полос и, если он нужен, вычисляет их геометрию.
	///
	/// Полосы включаются, если: задана переменная окружения `TMC_FORCE_BORDER=1`
	/// (проверка на малых заданиях); тонов больше одного; или четыре массива
	/// `_real` на всю область (`4*sizeof(_real)*nX*nY` байт) заняли бы больше
	/// четверти свободной памяти. Сетки меньше 32 узлов по стороне — всегда вся область.
	/// @param[in] dFreeBytes свободная физическая память, байт
	/// @see https://www.tamic.ru/docs/theory/radiation.html#border-strips
	void   SetBorderModeIfNeeded( double dFreeBytes );
	/// @brief Считает амплитуду и фазу текущего тона `nFreqCur` из квадратур в памяти
	///        в массивы `prAmp`, `prFaz` (вся область или полосы).
	void   MakeAmpFazFromMemory( void );

//  ---- several frequencies at once // BORDER ----------------------
//  A wide-band run carries many tones; the pattern is wanted for each
//  of them. One pair of quadratures per frequency is kept, which is
//  affordable exactly because only the border strips are stored.
	/// @brief Задаёт список тонов падающей волны для ДН на нескольких частотах.
	///
	/// Список — числа через пробел в единицах задания; переводятся в Гц.
	/// Пустой список — обычный расчёт на одной частоте F.
	/// @param[in] csList    список частот (короче 2048 символов, иначе игнорируется целиком)
	/// @param[in] dFreqUnit Гц в единице `FREQ_UNIT`
	/// @warning Тоны сверх `TMC_DN_MAXFREQ` (99) отбрасываются без сообщения.
	void   SetFreqList( CString csList, double dFreqUnit );
	/// @brief Выделяет и обнуляет массивы квадратур (только в режиме памяти).
	///
	/// По свободной физической памяти выбирает режим полос
	/// (`SetBorderModeIfNeeded`); размер массивов — `nX*nY` или `nBorderSize`,
	/// умноженный на число тонов при нескольких тонах.
	void   InitArrayForIntegrate( void );
	/// @brief Возвращает число тонов.
	/// @return 0 или 1 — одна частота F; больше 1 — ДН на каждый тон.
	int    GetFreqNum( void ) { return nFreqNum; }
/// Наибольшее число тонов ДН (по числу частот в файле `.s`: `FREQ_MAX_NUM - 1`).
#define TMC_DN_MAXFREQ 99
	/// Частоты тонов, Гц.
	double dFreqList[TMC_DN_MAXFREQ];
	/// Число тонов (0 — одна частота F).
	int    nFreqNum;
	/// Номер тона, для которого сейчас строится `.DAT`.
	int    nFreqCur;				// tone whose pattern is being built
	/// @brief Прибавляет вклад шага к квадратурам одного тона (только режим полос).
	/// @param[in] prUNode1    массив значений узлов ядра (шаг `nNodeStep`)
	/// @param[in] rCosCurrent `cos(2*pi*Fk*t)`
	/// @param[in] rSinCurrent `sin(2*pi*Fk*t)`
	/// @param[in] iBase       начало среза тона k в массивах: `k*nBorderSize`
	void   IntegrateInMemoryOneFreq( _real *prUNode1, _real rCosCurrent,
			_real rSinCurrent, TmcIdx iBase );
	/// @brief Возвращает место узла (ix, iy) в массивах квадратур.
	/// @param[in] ix номер узла по X
	/// @param[in] iy номер узла по Y
	/// @return Вне режима полос — `iy*nX + ix`; в режиме полос — индекс в полосах
	///         или -1, если узел в полосы не входит.
	/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#border-strips
	TmcIdx BorderSlot( int ix, int iy );
private:
	/// @brief Вычисляет прямоугольник полос (контур, расширенный на узел) и их размеры.
	void   BorderGeometry( void );
	/// TRUE — режим полос.
	BOOL   bBorderOnly;
	/// Прямоугольник полос в номерах узлов: контур, расширенный на один узел наружу.
	int    nBX0, nBX1, nBY0, nBY1;	// contour rectangle widened by one node
	/** @var CFieldIntegrated::nBX1
	    Правый край прямоугольника полос по x, номер узла (не больше `nX-1`).
	    @var CFieldIntegrated::nBY0
	    Нижний край прямоугольника полос по y, номер узла (не меньше 0).
	    @var CFieldIntegrated::nBY1
	    Верхний край прямоугольника полос по y, номер узла. */
	/// Ширина нижней и верхней полос, узлов.
	int    nBW;						// width of the horizontal strips
	/// Высота боковых полос между нижней и верхней, узлов.
	int    nBHm;					// height of the vertical strips (middle part)
	/// Число узлов во всех четырёх полосах (на один тон).
	TmcIdx nBorderSize;				// how many nodes the four strips hold

	/// Число значений на узел в массиве ядра (2 — H-ядро, 6 — X-ядро).
	int    nNodeStep;
	/// Ошибка интегратора.
	CTmcLibError cError;
	/// Частота ДН (текущий тон при нескольких тонах), Гц.
	double dFreq;
	/// Начало окна накопления, с.
	double dTmin;
	/// Конец окна накопления, с.
	double dTmax;
	/// Файл излучателей `<имя>.DAT` для TMC_DN.
	CString csFileDirectPattern;
	/// Файл квадратуры S `<имя>.SIN` (файловый режим).
	CString csFileSin;
	/// Файл квадратуры C `<имя>.COS` (файловый режим).
	CString csFileCos;
	/// Файл фаз `<имя>.FAZ` по сетке.
	CString csFileFaza;
	/// Файл амплитуд `<имя>.AMP` по сетке.
	CString csFileAmplitude;
};

#endif // !defined(AFX_FIELDINTEGRATED_H__C0B2DF6A_836F_4331_9C70_3DB935CE0EA1__INCLUDED_)
