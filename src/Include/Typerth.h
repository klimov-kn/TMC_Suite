/// @file Typerth.h
/// @brief Базовые типы и константы ядер Rt-сетки: точность `_real`, тип значения узла,
///        физические константы и прототипы служебных функций ядра.
///
/// Часть библиотеки TMCIndan; подключается всеми файлами TMCIndan и обоих ядер
/// (H и X). Макросы точности выбирают тип вещественных чисел расчёта, макросы
/// версии — надпись в окне «О программе». Функции, объявленные в конце файла,
/// реализованы не в TMCIndan, а в ядре (`pl_glfun.cpp` ядер H и X) — кроме
/// `SetDeltaT`/`GetDeltaT`/`IsDeltaTDefine`/`SwitchDeltaT`, реализованных
/// в `tmcrth_indanparam.cpp`.
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#types

#ifndef _TYPERTH_H
  #define _TYPERTH_H

/// Точность расчёта: `_real` = `double` (8 байт). Альтернатива — `_PREC_FLOAT`;
/// включается ровно один из двух макросов. Размер `_real` записывается в файлы
/// полей (`#TMCGROFLD_Accuracy`) и сверяется при их чтении.
#define _PREC_DOUBLE
//#define _PREC_FLOAT

/// Вариант поставки: профессиональная версия. `GetModel()` ядра выводит по нему
/// слово «Professional»; ядро проверяет этот макрос и в других местах.
/// Альтернативы — `_VERSION_DEMO___`, `_VERSION_EDUC___`.
#define _VERSION_PROF___






#ifdef _PREC_DOUBLE
/// Вещественный тип расчёта (здесь `double`).
#define _real double
#endif

#ifdef _PREC_FLOAT
#define _real float
#endif

/// Тип значения напряжения волны в узле сетки (здесь `_real`).
#define _ELEM_VAL_RTH _real
/// Множитель перевода значения узла в тип `_ELEM_VAL_RTH` (1 — без масштабирования).
#define _ELEM_VAL_RTH_MULT 1


/// Скорость света в вакууме c, м/с.
#define   C0___ 299792458
/// Магнитная постоянная mu0, Гн/м.
#define  MU0___ (12.566370614e-7)
/// Электрическая постоянная eps0, Ф/м.
#define EPS0___ (8.854187817e-12)
/// Число pi (12 знаков).
#define   PI___ (3.141592653589)

/// Заряд электрона (модуль), Кл. Используется X-ядром в модели плазмы.
#define   ELECTRON_Q___ (1.602176565e-19)
/// Масса электрона, кг. Используется X-ядром в модели плазмы.
#define   ELECTRON_M___ (9.10938356e-31)

/// @brief Выводит строку хода работы в первую панель строки состояния окна ядра.
/// @param[in] csTrace текст сообщения (ASCII). Реализация — в ядре (`pl_glfun.cpp`).
void PutTrace( CString csTrace );
/// @brief То же, что `PutTrace(CString)`, для строки C.
/// @param[in] szTrace текст сообщения, завершённый нулём.
void PutTrace( const char *szTrace );
/// @brief Выводит строку статистики (ход счёта, режим) во вторую панель строки состояния.
/// @param[in] csTrace текст сообщения. Реализация — в ядре (`pl_glfun.cpp`).
void PutStatistics( CString csTrace );
/// @brief То же, что `PutStatistics(CString)`, для строки C.
/// @param[in] szTrace текст сообщения, завершённый нулём.
void PutStatistics( const char *szTrace );
/// @brief Показывает в строке состояния, что выводится при расчёте (сигнал, поле, синхронизация).
/// @param[in] bTopologyFlag TRUE — включён вывод топологии
/// @param[in] bFieldFlag1   TRUE — включён вывод поля
/// @param[in] bSinchFlag1   TRUE — включён синхронный вывод поля
void PutSinchronizFlag( BOOL bTopologyFlag, BOOL bFieldFlag1, BOOL bSinchFlag1 );
/// @brief Выводит в строку статистики точность расчёта («Model: Float» или «Model: Double»).
void PutModel( void );
/// @brief Возвращает строку «точность + вариант поставки», например «Double Professional».
/// @return Текст для окна «О программе».
CString GetModel( void );

/// @brief Определяет тип ОС и по нему разрешает или запрещает звуковые сигналы ядра.
void ReadSystemType( void );

/// @brief Подаёт звуковой сигнал окончания шага #STEP (если звук разрешён).
/// @param[in] i не используется.
void BeepStepEnd( int i );
/// @brief Подаёт звуковой сигнал окончания всего расчёта (если звук разрешён).
void BeepAllEnd( void );
/// @brief Проигрывает мелодию с номером `code` из таблицы мелодий ядра.
/// @param[in] code номер мелодии: 0 — конец шага, 1 — конец расчёта.
void s_alarm( int code );
/// @brief Проигрывает мелодию: массив частот (Гц), отрицательный элемент задаёт
///        длительность следующих тонов (мс), ноль — конец массива.
/// @param[in] ptr массив тонов, завершённый нулём.
void s_play( int *ptr );
/// @brief Подаёт один тон.
/// @param[in] freq частота, Гц (0 — пауза)
/// @param[in] time длительность, мс
void s_tone( int freq, int time );
/// @brief Задаёт интервал мелодии 1 (настройка звука ядра).
/// @param[in] i интервал, мс
void SetMelody1Interval( int i );
/// @brief Возвращает интервал мелодии 1, мс.
/// @return Интервал, мс.
int GetMelody1Interval( void );
/// @brief Задаёт ноты мелодии 1.
/// @param[in] i массив из 12 номеров нот: 0..11 — полутоны от 262 до 494 Гц, 12 — пауза
void SetMelody1( int *i );
/// @brief Копирует номера нот мелодии 1 в массив вызывающего.
/// @param[out] i приёмник из 12 номеров нот (0..11 — полутоны, 12 — пауза)
void GetMelody1( int *i );
/// @brief Задаёт интервал мелодии 2, мс.
/// @param[in] i интервал, мс
void SetMelody2Interval( int i );
/// @brief Возвращает интервал мелодии 2, мс.
/// @return Интервал, мс.
int GetMelody2Interval( void );
/// @brief Задаёт ноты мелодии 2.
/// @param[in] i массив из 12 номеров нот: 0..11 — полутоны от 262 до 494 Гц, 12 — пауза
void SetMelody2( int *i );
/// @brief Копирует номера нот мелодии 2 в массив вызывающего.
/// @param[out] i приёмник из 12 номеров нот (0..11 — полутоны, 12 — пауза)
void GetMelody2( int *i );
/// @brief Задаёт механический шаг по времени из строки (см. `tmcrth_indanparam.h`).
/// Параметр описан при объявлении в `tmcrth_indanparam.h`.
void SetDeltaT( char *lpszDeltaT );
/// @brief Задаёт механический шаг по времени (см. `tmcrth_indanparam.h`).
/// Параметр описан при объявлении в `tmcrth_indanparam.h`.
void SetDeltaT( double dDeltaT1 );

/// @brief Проверяет, лежит ли точка (x0, y0) внутри треугольника (x1,y1)-(x2,y2)-(x3,y3).
///
/// Используется ядром при построении узлов многоугольных блоков.
/// @param[in] x1,y1 первая вершина, м
/// @param[in] x2,y2 вторая вершина, м
/// @param[in] x3,y3 третья вершина, м
/// @param[in] x0,y0 проверяемая точка, м
/// @return TRUE — точка внутри треугольника.
BOOL IsPointInTr( double x1, double y1, double x2, double y2, double x3, double y3, double x0, double y0 );

/// @brief Функция потока: считает текущий шаг #STEP документа ядра.
/// @param[in] pParam указатель на документ ядра (`CPlanRT_HDoc*`)
/// @return Всегда 0.
UINT RunStepGlobal( LPVOID pParam );
/// @brief Функция потока: считает все шаги #STEP документа ядра подряд.
/// @param[in] pParam указатель на документ ядра (`CPlanRT_HDoc*`)
/// @return Всегда 0.
UINT RunAllGlobal( LPVOID pParam );

#endif
