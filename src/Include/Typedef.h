/// @file Typedef.h
/// @brief Базовые типы и константы пакета: `_real`, `_complex`, размеры имён, физические константы.
///
/// Подключается почти всеми файлами MFC-версии. Задаёт `_HIGH_ACCURACY`, поэтому
/// `_real` — `double`. Пустые макросы `_far`, `_near`, `_huge`, `_fortran`, `_pascal`
/// остались от 16-битных компиляторов и ничего не меняют. Тип `_complex` в C++ берётся
/// из `<math.h>` MinGW или из преамбулы `compat/include/tmc_win_prelude.h`.
/// @see https://www.tamic.ru/docs/code/shared-headers.html#typedef
#ifndef _TYPDEF_H
  #define _TYPDEF_H




/// Двойная точность: `_real` = `double`.
#ifndef _HIGH_ACCURACY
  #define _HIGH_ACCURACY
#endif

/// Тип элемента матрицы Y в коде TAMIC (`_real`); `_ELEM_COMPLEX` — признак
/// комплексных элементов (используется в run.h).
#ifndef _ELEM_VAL
 #define _ELEM_VAL _real
 #ifndef _ELEM_COMPLEX
  /// Признак комплексных элементов матрицы Y (проверяется в run.h и заголовках модели M).
  #define _ELEM_COMPLEX
 #endif
#endif

#include <float.h>

/// Беззнаковый байт (как в Windows).
typedef unsigned char BYTE;

/// Беззнаковое целое.
typedef unsigned int   Uint;
/// Беззнаковое 16-битное целое.
typedef unsigned short WORD;
/// Беззнаковое 32-битное целое (как в Windows; в 64-битном Linux — 64 бита).
typedef unsigned long DWORD;

/// Длина имени файла DOS 8.3 с завершающим нулём, символов.
#ifndef FILENAME
 #define FILENAME           13
#endif
/// Длина пути, если она не задана системой. Обычно `<stdio.h>` уже определяет
/// `FILENAME_MAX` (260 в Windows), и это значение не используется.
#ifndef FILENAME_MAX
 #define FILENAME_MAX      63
#endif

/// @brief Имя файла, разбитое на части: имя, путь, диск (DOS-версия).
typedef struct { char name[FILENAME];
                 char path[FILENAME_MAX];
                 char drive;
                 } FILE_LIST;
/// @var FILE_LIST::name
/// Имя файла с расширением (DOS 8.3), строка с нулём в конце.
/// @var FILE_LIST::path
/// Путь к каталогу файла, строка с нулём в конце.
/// @var FILE_LIST::drive
/// Буква диска.

/// @name Пределы вещественного типа пакета
/// Пределы типа `_real`: `REAL_DIG`, `REAL_MAX`, `REAL_MIN`, `REAL_EPSILON` и т.д.
/// повторяют `DBL_*` (или `FLT_*` без `_HIGH_ACCURACY`).
///@{
#ifndef _HIGH_ACCURACY
  #define REAL_DIG  FLT_DIG    /* # of decimal digits of precision */
  #define REAL_MAX_10_EXP  FLT_MAX_10_EXP    /* max decimal exponent */
  #define REAL_MIN_10_EXP  FLT_MIN_10_EXP    /* max decimal exponent */
  #define REAL_MAX  FLT_MAX    /* max value */
  #define REAL_MIN  FLT_MIN    /* min positive value */
  #define REAL_EPSILON FLT_EPSILON /* smalest such that 1 + REAL_EPSILON != 1 */
  typedef float _real;
 #else
  #define REAL_DIG  DBL_DIG    /* # of decimal digits of precision */
  #define REAL_MAX_10_EXP  DBL_MAX_10_EXP    /* max decimal exponent */
  #define REAL_MIN_10_EXP  DBL_MIN_10_EXP    /* max decimal exponent */
  #define REAL_MAX  DBL_MAX    /* max value */
  #define REAL_MIN  DBL_MIN    /* min positive value */
  #define REAL_EPSILON DBL_EPSILON /* smalest such that 1 + REAL_EPSILON != 1 */
  #ifndef _real
    /// Вещественный тип пакета: `double` (без `_HIGH_ACCURACY` — `float`).
    typedef double _real;
  #endif
#endif
///@}

// typedef _real _far * r_array;

/// Для C-файлов здесь определяется `_complex` (`x` — действительная часть,
/// `y` — мнимая). Для C++ тип приходит извне (см. описание файла).
#ifndef __cplusplus
 #ifndef __COMPLEX_DEFINED
   #define __COMPLEX_DEFINED

  typedef struct tag_complex { _real x;
                               _real y; }_complex;/* real and imaginary parts */
  /// @struct tag_complex
  /// @brief Комплексное число для C-файлов.
  /// @var tag_complex::x
  /// Действительная часть.
  /// @var tag_complex::y
  /// Мнимая часть.
  /// @typedef _complex
  /// Комплексное число для C-файлов (`struct tag_complex`).

 // typedef _complex _far * c_array;
 #endif
#else
 #ifndef __COMPLEX_DEFINED_TMC
   #define __COMPLEX_DEFINED_TMC

	#include <math.h>

#endif
#endif



/// Число pi (если не задано ранее).
#ifndef PI
 #define PI                   (_real)3.141592653589793
#endif

/// @brief Проверяет, может ли символ входить в запись числа (цифра, `+`, `-`, `.`).
/// Параметр (без имени в объявлении) — проверяемый символ.
/// @return Ненулевое — может. Реализовано макросом ниже.
int isnumber( char );
/// @brief Макрос-реализация `isnumber`: ненулевое, если `_c` — цифра, `+`, `-` или `.`.
/// @param _c  проверяемый символ
#define isnumber(_c) (isdigit( _c )||( _c == '+' )||( _c == '-' )||( _c == '.' ))

/// @brief Возвращает знак числа: 1 для `rn >= 0`, иначе -1. Реализовано макросом ниже.
/// Параметр (без имени в объявлении) — число типа `_real`.
/// @return 1 или -1.
int rsign( _real );
/// @brief Макрос-реализация `rsign`: 1 для `rn >= 0`, иначе -1.
/// @param rn  число
#define rsign( rn )  ( (rn >= 0)?1:-1 )

/// Короткое целое для таблиц входов (`INPUT_DATA`).
#define INT_1 short

#endif
