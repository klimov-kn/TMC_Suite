/// @file INTER.H
/// @brief Интерпретатор программ из выражений (библиотека `exprint`, часть Inter).
///
/// Надстройка над вычислителем Expr (expr.h): последовательности операторов через `;`,
/// блоки `{...}`, операторы `return(e)`, `if(c){...}`, `while(c){...}`. Условие
/// считается истинным, если его значение >= 0. Подробные описания функций
/// `inte_*` и `i1nte_atof_1` — в expr.h (там же они объявлены повторно).
/// @see https://www.tamic.ru/docs/code/lib-inter.html
#ifndef _INTER_CALCUL
 #define _INTER_CALCUL

 #include <EXPR.H>

/// @brief Описание оператора интерпретатора: имя с открывающей скобкой и его длина.
  typedef struct {
    	            /// Имя оператора вместе с «(», например `"while("`.
    	            char lpszName[32];
	    			/// Длина `lpszName`, символов.
	    			int  nLenght;
                 } INTE_OPER;

/// @brief Удаляет из строки на месте все пробелы, табуляции и переводы строки `\n`.
/// @param[in,out] ch  строка, оканчивающаяся нулём
/// @return Всегда 0.
int expr_del_Blanks1( char *ch );
/// @cond
/// @brief Выполняет программу интерпретатора (см. описание в expr.h).
/// @param[in,out] string  текст программы без пробелов
/// @return Результат программы.
_real inte_atof( char *string );
/// @brief Выполняет подстроку программы (см. описание в expr.h).
/// @param[in,out] lex  начало подстроки
/// @param[in]     n    длина, символов
/// @return Значение последнего оператора.
_real inte_inter( char  *lex, int n );
/// @endcond
/// @brief Запоминает значение `return(...)`.
/// @param[in] r  значение
void  inte_set_inter_value( _real r );
/// @brief Возвращает значение последнего `return(...)`.
/// @return Запомненное значение.
_real inte_get_inter_value( void );
/// @cond
/// @brief Выполняет оператор `return`, `if` или `while` (см. описание в expr.h).
/// @param[in,out] lex  начало оператора
/// @param[in]     n    длина, символов
/// @return Значение оператора.
_real inte_oper( char  *lex, int n );
/// @brief Находит закрывающую скобку условия оператора.
/// @param[in] lex  текст после открывающей скобки
/// @param[in] n    длина, символов
/// @return Позиция парной `)`; `n`, если не найдена.
int inte_select_blanck( char  *lex, int n );
/// @endcond
/// @brief Удаляет пробелы и выполняет программу (см. описание в expr.h).
/// Параметры `string`, `r` и код возврата описаны при объявлении в expr.h.
int i1nte_atof_1( char  *string, _real *r );

#endif