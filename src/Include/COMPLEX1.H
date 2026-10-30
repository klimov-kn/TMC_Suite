/*
 *  TAMIC_soft(R) 1990,1991,1992 vR1.01
 *
 *  Declarations and definitions for complex mathematics.
 *
 * Define the macro switch: INLN_COMPLEX,
 * if you wish to use macros expanding to inline code (instead of
 * functions) for the complex conversion functions.
 * Beware of duplicate names!
 *
 *  FILE : \source\include\complex.h
 *
*/

/// @file COMPLEX1.H
/// @brief Объявления функций комплексной арифметики библиотеки `complex`.
///
/// Тип `_complex` — структура из двух полей `_real` (`x` — действительная часть,
/// `y` — мнимая); `_real` в сборке пакета — `double` (см. typedef.h). Функции
/// принимают и возвращают комплексные числа по значению, состояния не хранят и
/// могут вызываться из любого потока. Реализация — libs/complex/cmplxall.cpp.
/// Основные потребители — вьюверы (выражения над S-параметрами) и sfile95.
/// @see https://www.tamic.ru/docs/code/lib-complex.html

#ifndef _COMPLEX_H
  #define _COMPLEX_H

// #ifndef _COMPLEX_DEFINED
//   #define _COMPLEX_DEFINED
//
//  typedef struct tag_complex { _real x;
//                               _real y; } _complex;/* real and imaginary parts */
//
// #endif

  /* pseudo functions */



/// Режим `INLN_COMPLEX`: простейшие функции (`aimag`, `ctof`, `ftoc`, `conjg`, `cneg`)
/// заменяются макросами, работающими через глобальную переменную `z`.
/// В сборке пакета макрос не определён — используются функции.
#ifdef INLN_COMPLEX	/* then use the following macro versions */

/// Рабочая переменная макросов режима `INLN_COMPLEX` (определяется в каждом файле,
/// включившем заголовок в этом режиме).
_complex z;
/// Мнимая часть `z` (макроверсия функции `aimag`).
#define   aimag(z)  ( z.y ) /* return float */
/// Действительная часть `z` (макроверсия функции `ctof`).
#define   ctof(z)   ( z.x ) /* return float */
/// Записывает `r`, `i` в глобальную `z` и возвращает её.
#define   ftoc(r,i) (( z.x = r, z.y = i ), z)        /* return complex */
/// Сопряжение; аргумент изменяется на месте.
#define   conjg(z)  (( z.y = -z.y ), z)              /* return complex */
/// Смена знака; аргумент изменяется на месте.
#define   cneg(z)   ( ( z.x = -z.x, z.y = -z.y ), z) /* return complex */

#else                    /* declare functions */

/// @brief Возвращает мнимую часть комплексного числа.
/// @param[in] z  комплексное число
/// @return `z.y`.
_real    aimag( _complex );  /* imaginary component of complex number */
/// @brief Возвращает действительную часть комплексного числа.
/// @param[in] z  комплексное число
/// @return `z.x`.
_real    ctof( _complex );   /* complex (real component) to float conversion */
/// @brief Строит комплексное число из действительной и мнимой частей.
/// Параметры (без имён в объявлении): действительная и мнимая части.
/// @return Комплексное число с этими частями.
/// @warning Только объявление: в библиотеке `complex` функция не определена.
_complex ftoc(_real, _real); /* convert floats to complex */
/// @brief Возвращает комплексно сопряжённое число.
/// @param[in] z  комплексное число
/// @return `x - j*y`.
_complex conjg( _complex );  /* complex conjugate */
/// @brief Возвращает число с противоположным знаком.
/// @param[in] z  комплексное число
/// @return `-z`.
_complex cneg( _complex );           /* negation of a complex number */

#endif
 /* direct angle (in radians) of complex number */
/// @brief Возвращает фазу комплексного числа на полном круге, радианы.
///
/// Фаза отсчитывается в отрицательную сторону: результат лежит в (-2*pi, 0]
/// (положительная мнимая ось — `-1.5*pi`, отрицательная — `-0.5*pi`,
/// отрицательная действительная полуось — `-pi`). Для `z == 0` возвращает 0.
/// @param[in] z  комплексное число
/// @return Фаза в (-2*pi, 0], радианы.
/// @see https://www.tamic.ru/docs/code/lib-complex.html#angles
_real    cphase( _complex ); /* angle (in radians) of complex number from 0 to -2*PI*/
/// @brief Возвращает модуль комплексного числа `sqrt(x^2 + y^2)`.
///
/// Считается с масштабированием на `max(|x|, |y|)`, чтобы избежать переполнения
/// при возведении в квадрат.
/// @param[in] z  комплексное число
/// @return Модуль, >= 0.
_real    cabsv( _complex );   /* cosine of complex number */


#endif
